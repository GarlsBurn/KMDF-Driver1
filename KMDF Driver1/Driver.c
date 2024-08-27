#include <ntddk.h>

NTSTATUS MyDispatchRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
void
MyDriverUnload(_In_ PDRIVER_OBJECT DriverObject);

NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
) {
	UNREFERENCED_PARAMETER(RegistryPath);
    DbgPrint("DriverEntry: Драйвер загружен.\n");

	DriverObject->MajorFunction[IRP_MJ_CREATE] = MyDispatchRoutine;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = MyDispatchRoutine;
	DriverObject->MajorFunction[IRP_MJ_READ] = MyDispatchRoutine;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = MyDispatchRoutine;

	DriverObject->DriverUnload = MyDriverUnload;

    UNICODE_STRING deviceName;
    RtlInitUnicodeString(&deviceName, L"\\Device\\MyVirtualDevice");

    PDEVICE_OBJECT deviceObject = NULL;
    NTSTATUS status = IoCreateDevice(
        DriverObject,
        0,
        &deviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &deviceObject
    );

    if (!NT_SUCCESS(status)) {
        DbgPrint("Failed to create device object.\n");
        return status;
    }

    DbgPrint("Virtual device created successfully.\n");
    return STATUS_SUCCESS;
}

__drv_dispatchType(IRP_MJ_READ, IRP_MJ_WRITE)
NTSTATUS MyDispatchRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
)
{
    DbgPrint("MyDispatchRoutine: Обрабатываем IRP.\n");
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;
    ULONG_PTR info = 0;

    switch (stack->MajorFunction) {
    case IRP_MJ_READ:
        DbgPrint("IRP_MJ_READ received\n");
        
        RtlZeroMemory(Irp->AssociatedIrp.SystemBuffer, stack->Parameters.Read.Length);
        info = stack->Parameters.Read.Length;
        break;

    case IRP_MJ_WRITE:
        DbgPrint("IRP_MJ_WRITE received\n");
        info = stack->Parameters.Write.Length;
        break;

    default:
        DbgPrint("Unhandled IRP: %d\n", stack->MajorFunction);
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = info;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    UNREFERENCED_PARAMETER(DeviceObject);

    return status;
}

void
MyDriverUnload(
	_In_ PDRIVER_OBJECT DriverObject

) {
	UNREFERENCED_PARAMETER(DriverObject);
    DbgPrint("DriverUnload: Драйвер выгружается.\n");
}
