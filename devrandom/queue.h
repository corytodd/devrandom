//
// This is the context that can be placed per queue
// and would contain per queue information.
//
typedef struct _QUEUE_CONTEXT {

    // Here we allocate a buffer from a test write so it can be read back
    PVOID       Buffer;
    ULONG       Length;

    // Virtual I/O
    WDFREQUEST  CurrentRequest;
    NTSTATUS    CurrentStatus;

} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, QueueGetContext)

NTSTATUS
DevRandomQueueInitialize(
    WDFDEVICE hDevice
    );

EVT_WDF_IO_QUEUE_CONTEXT_DESTROY_CALLBACK DevRandomEvtIoQueueContextDestroy;

//
// Events from the IoQueue object
//
EVT_WDF_IO_QUEUE_IO_READ DevRandomEvtIoRead;

