#include "public.h"

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
    ULONG PrivateDeviceData;  // just a placeholder

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called WdfObjectGet_DEVICE_CONTEXT
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE(DEVICE_CONTEXT)

//
// Function to initialize the device and its callbacks
//
NTSTATUS
DevRandomDeviceCreate(
    PWDFDEVICE_INIT DeviceInit
    );