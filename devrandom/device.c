#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, DevRandomDeviceCreate)
#endif


NTSTATUS
DevRandomDeviceCreate(
    PWDFDEVICE_INIT DeviceInit
    )
/*++

Routine Description:

    Worker routine called to create a device and its software resources.

Arguments:

    DeviceInit - Pointer to an opaque init structure. Memory for this
                    structure will be freed by the framework when the WdfDeviceCreate
                    succeeds. So don't access the structure after that point.

Return Value:

    NTSTATUS

--*/
{
    WDF_OBJECT_ATTRIBUTES attributes;
    PDEVICE_CONTEXT deviceContext;
    WDFDEVICE device;
    NTSTATUS status;

    DECLARE_CONST_UNICODE_STRING(dosDeviceName, DOS_DEV_NAME);

    PAGED_CODE();

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, REQUEST_CONTEXT);
    WdfDeviceInitSetRequestAttributes(DeviceInit, &attributes);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &attributes, &device);

    if (NT_SUCCESS(status)) {

        //
        // Create a symbolic link for ease of use
        //
        status = WdfDeviceCreateSymbolicLink(device, &dosDeviceName);

        // 
        // Setup device context
        //
        // Create a device interface so that application can find and talk
        // to us.
        //
        status = WdfDeviceCreateDeviceInterface(
            device,
            &GUID_DEVINTERFACE_DEVRANDOM,
            NULL // ReferenceString
            );

        if (NT_SUCCESS(status)) {
            //
            // Initialize the I/O Package and any Queues
            //
            status = DevRandomQueueInitialize(device);
        }
        deviceContext = WdfObjectGet_DEVICE_CONTEXT(device);
    }

    return status;
}


