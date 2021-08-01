
#define INITGUID

#include <ntifs.h> // Include before ddk
#include <ntddk.h>
#include <wdf.h>

#include "device.h"
#include "queue.h"

typedef struct _REQUEST_CONTEXT {

    ULONG PrivateDeviceData;  // just a placeholder

} REQUEST_CONTEXT, *PREQUEST_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REQUEST_CONTEXT, RequestGetContext);

//
// WDFDRIVER Events
//
DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD DevRandomEvtDeviceAdd;

NTSTATUS
DevRandomPrintDriverVersion(
    );

