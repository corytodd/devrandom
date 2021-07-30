#define WHILE(a) \
__pragma(warning(suppress:4127)) while(a)

//
// Define an Interface Guid so that app can find the device and talk to it.
//
// {73335581-D1C5-4E33-B06C-78131AF4BE95}
DEFINE_GUID(GUID_DEVINTERFACE_DEVRANDOM,
    0x73335581, 0xd1c5, 0x4e33, 0xb0, 0x6c, 0x78, 0x13, 0x1a, 0xf4, 0xbe, 0x95);

#define DOS_DEV_NAME L"\\DosDevices\\DevRandom"
#define DEV_NAME L"\\Device\\DevRandom"
