#include <ntddk.h>
#include <fwpmk.h>

#define FILTER_NAME L"MyNetworkFilter"

extern "C" NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT   DriverObject,
    _In_ PUNICODE_STRING  RegistryPath
);

extern "C" VOID UnloadDriver(
    _In_ PDRIVER_OBJECT DriverObject
);

extern "C" NTSTATUS ClassifyFn(
    _In_ const FWPS_INCOMING_VALUES* inFixedValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
    _Inout_ VOID* layerData,
    _In_opt_ const VOID* classifyContext,
    _In_ const FWPS_FILTER* filter,
    _In_ UINT64 flowContext,
    _Inout_ FWPS_CLASSIFY_OUT* classifyOut
);

extern "C" VOID NotifyFn(
    _In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
    _In_ const GUID* filterKey,
    _In_ const FWPS_FILTER* filter
);

extern "C" const GUID WfpCalloutGuid = { /* your GUID here */ };

extern "C" const GUID WfpFilterKey = { /* your GUID here */ };

extern "C" const FWPS_CALLOUT0 Callout = {
    WfpCalloutGuid,
    NotifyFn,
    ClassifyFn
};

extern "C" const FWPM_FILTER0 Filter = {
    WfpFilterKey,
    { 0 },
    FWPM_DISPLAY_DATA0{ 0 },
    FWPM_FILTER_FLAG_NONE,
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    FWPM_PROVIDER_CONTEXT_TYPE_ANY,
    { 0 },
    &Callout.key
};

extern "C" NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT   DriverObject,
    _In_ PUNICODE_STRING  RegistryPath
)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    DriverObject->DriverUnload = UnloadDriver;

    NTSTATUS status = FwpsCalloutRegister0(
        DriverObject,
        &Callout,
        &WfpCalloutGuid
    );

    if (!NT_SUCCESS(status)) {
        KdPrint(("FwpsCalloutRegister0 failed with status %x\n", status));
        return status;
    }

    status = FwpsFilterAdd0(
        DriverObject,
        &Filter,
        NULL,
        NULL
    );

    if (!NT_SUCCESS(status)) {
        KdPrint(("FwpsFilterAdd0 failed with status %x\n", status));
        FwpsCalloutUnregisterById0(WfpCalloutGuid);
        return status;
    }

    KdPrint(("Driver Loaded\n"));
    return STATUS_SUCCESS;
}

extern "C" VOID UnloadDriver(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    UNREFERENCED_PARAMETER(DriverObject);

    FwpsFilterDeleteById0(WfpFilterKey);
    FwpsCalloutUnregisterById0(WfpCalloutGuid);

    KdPrint(("Driver Unloaded\n"));
}

extern "C" NTSTATUS ClassifyFn(
    _In_ const FWPS_INCOMING_VALUES* inFixedValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
    _Inout_ VOID* layerData,
    _In_opt_ const VOID* classifyContext,
    _In_ const FWPS_FILTER* filter,
    _In_ UINT64 flowContext,
    _Inout_ FWPS_CLASSIFY_OUT* classifyOut
)
{
    UNREFERENCED_PARAMETER(inFixedValues);
    UNREFERENCED_PARAMETER(inMetaValues);
    UNREFERENCED_PARAMETER(layerData);
    UNREFERENCED_PARAMETER(classifyContext);
    UNREFERENCED_PARAMETER(filter);
    UNREFERENCED_PARAMETER(flowContext);

    // Hide the network connection by setting the block flag
    classifyOut->actionType = FWP_ACTION_BLOCK;
    classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;

    return STATUS_SUCCESS;
}

extern "C" VOID NotifyFn(
    _In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
    _In_ const GUID* filterKey,
    _In_ const FWPS_FILTER* filter
)
{
    UNREFERENCED_PARAMETER(notifyType);
    UNREFERENCED_PARAMETER(filterKey);
    UNREFERENCED_PARAMETER(filter);
}
