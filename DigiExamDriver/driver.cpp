#include "protection.hpp"

const wchar_t* DeviceName   = L"\\Device\\DigiExam";
const wchar_t* SymbolicLink = L"\\DosDevices\\DigiExam";

UNICODE_STRING StrDeviceName;
UNICODE_STRING StrSymbolicLink;
PDEVICE_OBJECT DeviceObject = NULL;

Protect ProtectedApps[Protect::MaximumProtectedApps] = {Protect()}; // Using this shit cuz vectors are no allowes in Kernel. SIL. There are solutions to it tho

VOID OnDriverUnload(PDRIVER_OBJECT DriverObject);
VOID InitIo(PDRIVER_OBJECT DriverObject); // Initlize our io requests
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath); // Entry of driver, to setup threads etc now we dont have thread hehe
INT SetupIoDevice(PDRIVER_OBJECT DriverObject); // Prevent overload
VOID HandleBufferData(DriverCommand* Buffer, INT Len); // Handle the messages sent by IO

VOID OnDriverUnload(PDRIVER_OBJECT DriverObject) {
	for (auto x : ProtectedApps){
		x.DisableProtection();
	}
	if (IoDeleteSymbolicLink(&StrSymbolicLink) != STATUS_SUCCESS) {
		DbgPrint("Failed to delete Symbolic link");
	}
	IoDeleteDevice(DeviceObject);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	DriverObject->DriverUnload = OnDriverUnload;
	InitIo(DriverObject);

	RtlInitUnicodeString(&StrDeviceName, DeviceName);
	RtlInitUnicodeString(&StrSymbolicLink, SymbolicLink);
	
	SetupIoDevice(DriverObject);
	DbgPrintEx(0, 0, "Hello Driver");


	return STATUS_SUCCESS;
}

VOID InitIo(PDRIVER_OBJECT DriverObject) {
	for (auto i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {return STATUS_SUCCESS; }; // Unsuported
	}

	DriverObject->MajorFunction[IRP_MJ_CREATE] = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {return STATUS_SUCCESS; }; // Creat_DeviceIo
	DriverObject->MajorFunction[IRP_MJ_CLOSE]  = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {return STATUS_SUCCESS; }; // Close_DeviceIo
	DriverObject->MajorFunction[IRP_MJ_WRITE]  = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {						  // Write_DeviceIo
		DriverCommand* Buffer = NULL;
		PIO_STACK_LOCATION pIoStack = IoGetCurrentIrpStackLocation(Irp);
		if (pIoStack) {
			Buffer = (DriverCommand*)(Irp->AssociatedIrp.SystemBuffer);
			if (Buffer) HandleBufferData(Buffer, pIoStack->Parameters.Write.Length);
		}
		return STATUS_SUCCESS;
	};
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "init done\n");

}

INT SetupIoDevice(PDRIVER_OBJECT DriverObject) {
	NTSTATUS Status = IoCreateDevice(DriverObject, 0, &StrDeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject); // Create comm request device
	if (Status != STATUS_SUCCESS) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to create io device\n");

		return 0;
	}

	DeviceObject->Flags |= DO_BUFFERED_IO;
	DeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

	Status = IoCreateSymbolicLink(&StrSymbolicLink, &StrDeviceName);
	if (Status != STATUS_SUCCESS) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Symbolic link failed.\n");

		return 0;
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Device setup good\n");

	return 1;
}

VOID HandleBufferData(DriverCommand* Buffer, INT Len) {
	if (Buffer->type == STOP_PROTECTION) {
		ProtectedApps[Buffer->index].DisableProtection();
	}

	if (Buffer->type == PROTECT_PROCESS) {
		if (ProtectedApps[Buffer->index].EnableProtection(Buffer->pid)) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "enable_protection() succeed!\n");

		}
		else {
			DbgPrint("enable_protection() failed!\n");
		}
	}

}