#include "driver.h"
#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, DevRandomQueueInitialize)
#endif

NTSTATUS
DevRandomQueueInitialize(
    WDFDEVICE Device
    )
/*++

Routine Description:


     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for serial request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

     This memory may be used by the driver automatically synchronized
     by the Queue's presentation lock.

     The lifetime of this memory is tied to the lifetime of the I/O
     Queue object, and we register an optional destructor callback
     to release any private allocations, and/or resources.


Arguments:

    Device - Handle to a framework device object.

Return Value:

    NTSTATUS

--*/
{
    WDFQUEUE queue;
    NTSTATUS status;
    PQUEUE_CONTEXT queueContext;
    WDF_IO_QUEUE_CONFIG queueConfig;
    WDF_OBJECT_ATTRIBUTES attributes;

    PAGED_CODE();

    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
         &queueConfig,
        WdfIoQueueDispatchSequential
        );

    queueConfig.EvtIoRead   = DevRandomEvtIoRead;

    //
    // Fill in a callback for destroy, and our QUEUE_CONTEXT size
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, QUEUE_CONTEXT);

    //
    // By not setting the synchronization scope and using the default, there is
    // no locking between any of the callbacks in this driver.
    //
    // We will create a sequential queue so all of  the EvtIoXxx callbacks are
    // serialized against each other (at least until the request is completed),
    // but the cancel routine and the timer DPC are not synchronized against the
    // queue's EvtIoXxx callbacks.
    //
    // attributes.SynchronizationScope = ...

    attributes.EvtDestroyCallback = DevRandomEvtIoQueueContextDestroy;

    status = WdfIoQueueCreate(
        Device,
        &queueConfig,
        &attributes,
        &queue
        );

    if( !NT_SUCCESS(status) ) {
        KdPrint(("WdfIoQueueCreate failed 0x%x\n",status));
        return status;
    }

    // Get our Driver Context memory from the returned Queue handle
    queueContext = QueueGetContext(queue);

    queueContext->Buffer = NULL;

    queueContext->CurrentRequest = NULL;
    queueContext->CurrentStatus = STATUS_INVALID_DEVICE_REQUEST;


    return status;
}

VOID
DevRandomEvtIoQueueContextDestroy(
    WDFOBJECT Object
)
/*++

Routine Description:

    This is called when the Queue that our driver context memory
    is associated with is destroyed.

Arguments:

    Context - Context that's being freed.

Return Value:

    VOID

--*/
{
    PQUEUE_CONTEXT queueContext = QueueGetContext(Object);

    //
    // Release any resources pointed to in the queue context.
    //
    // The body of the queue context will be released after
    // this callback handler returns
    //

    //
    // If Queue context has an I/O buffer, release it
    //
    if( queueContext->Buffer != NULL ) {
        ExFreePool(queueContext->Buffer);
        queueContext->Buffer = NULL;
    }

    return;
}

BOOLEAN
DevRandomDecrementRequestCancelOwnershipCount(
    PREQUEST_CONTEXT RequestContext
    )
/*++

Routine Description:
    Decrements the cancel ownership count for the request.  When the count
    reaches zero ownership has been acquired.

Arguments:
    RequestContext - the context which holds the count

Return Value:
    TRUE if the caller can complete the request, FALSE otherwise

  --*/
{
    LONG result;

    result = InterlockedDecrement(
        &RequestContext->CancelCompletionOwnershipCount
        );

    ASSERT(result >= 0);

    if (result == 0) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

VOID
DevRandomSetCurrentRequest(
    WDFREQUEST Request,
    WDFQUEUE Queue
    )
{
    PQUEUE_CONTEXT queueContext;
    PREQUEST_CONTEXT requestContext;

    requestContext = RequestGetContext(Request);
    queueContext = QueueGetContext(Queue);

    //
    // Set the ownership count to one.  When a caller wants to claim ownership,
    // they will interlock decrement the count.  When the count reaches zero,
    // ownership has been acquired and the caller may complete the request.
    //
    requestContext->CancelCompletionOwnershipCount = 1;

    queueContext->CurrentRequest = Request;
    queueContext->CurrentStatus  = STATUS_SUCCESS;
}

VOID
DevRandomEvtIoRead(
    IN WDFQUEUE   Queue,
    IN WDFREQUEST Request,
    IN size_t      Length
    )
/*++

Routine Description:

    This event is called when the framework receives IRP_MJ_READ request.
    It will fill the Request's buffer with a number of random bytes 
    specified by Length.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
            I/O request.
    Request - Handle to a framework request object.

    Length  - number of bytes to be read.
                 The default property of the queue is to not dispatch
                 zero lenght read & write requests to the driver and
                 complete is with status success. So we will never get
                 a zero length request.


Return Value:

    VOID

--*/
{
    NTSTATUS Status;
    WDFMEMORY memory;
    PUCHAR randBuffer = NULL;

    _Analysis_assume_(Length > 0);

    KdPrint(("DevRandomEvtIoRead Called! Queue 0x%p, Request 0x%p Length %Iu\n",
                            Queue,Request,Length));
    
    //
    // Get the request memory
    //
    Status = WdfRequestRetrieveOutputMemory(Request, &memory);
    if( !NT_SUCCESS(Status) ) {
        KdPrint(("DevRandomEvtIoRead Could not get request memory buffer 0x%x\n",Status));
        WdfVerifierDbgBreakPoint();
        WdfRequestCompleteWithInformation(Request, Status, 0L);
        return;
    }

    //
    // Get the buffer backing this memory
    //
    randBuffer = WdfMemoryGetBuffer(memory, NULL);

    _Analysis_assume_(randBuffer != NULL);

    //
    // Fill buffer with random bytes
    //
    Status = DevRandomFillBufferRand(randBuffer, Length);
    if( !NT_SUCCESS(Status) ) {
        KdPrint(("DevRandomEvtIoRead: DevRandomFullBufferRand failed 0x%x\n", Status));
        WdfRequestComplete(Request, Status);
        return;
    }

    // Set transfer information
    WdfRequestSetInformation(Request, (ULONG_PTR)Length);

    // All done
    WdfRequestComplete(Request, Status);

    return;
}

NTSTATUS
NTAPI
DevRandomFillBufferRand(
    PVOID Buffer,
    SIZE_T Length)
{
    static ULONG secRandomSeed = 0;

    LARGE_INTEGER tickCount;
    ULONG i, randomValue;
    PULONG ptr;

    /* Try to generate a more random seed */
    KeQueryTickCount(&tickCount);
    secRandomSeed ^= _rotl(tickCount.LowPart, (secRandomSeed % 23));

    ptr = Buffer;
    for (i = 0; i < Length / sizeof(ULONG); i++)
    {
        ptr[i] = RtlRandomEx(&secRandomSeed);
    }

    Length &= (sizeof(ULONG) - 1);
    if (Length > 0)
    {
        randomValue = RtlRandomEx(&secRandomSeed);
        RtlCopyMemory(&ptr[i], &randomValue, Length);
    }

    return STATUS_SUCCESS;
}