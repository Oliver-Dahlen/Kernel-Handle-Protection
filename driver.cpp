#include <ntddk.h>
#include <wdf.h>
#include <vector>
#include "protection.h"

const wchar_t* DeviceName   = L"\\Device\\DigiExam";
const wchar_t* SymbolicLink = L"\\DosDevices\\DigiExam";
UNICODE_STRING ustrDeviceName;
UNICODE_STRING ustrSymbolicLink;
PDEVICE_OBJECT CreatedDeviceObject = NULL;
Protect proc;
std::vector<Protect> Protected_Programs;

VOID OnDriverUnload(PDRIVER_OBJECT DriverObject);
VOID InitIo(PDRIVER_OBJECT DriverObject); // Initlize our io requests
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath); // Entry of driver, to setup threads etc
INT SetupIoDevice(PDRIVER_OBJECT DriverObject); // Prevent overload
VOID HandleBufferMessage(PCHAR Buffer, INT Len); // Handle the messages sent by IO

VOID OnDriverUnload(PDRIVER_OBJECT DriverObject) {
	proc.DisableProtection();
	if (IoDeleteSymbolicLink(&ustrSymbolicLink) != STATUS_SUCCESS) {
		DbgPrint("Failed to delete Symbolic link");
	}
	IoDeleteDevice(CreatedDeviceObject);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	DriverObject->DriverUnload = OnDriverUnload;
	InitIo(DriverObject);

	RtlInitUnicodeString(&ustrDeviceName, DeviceName);
	RtlInitUnicodeString(&ustrSymbolicLink, SymbolicLink);
	
	if (SetupIoDevice(DriverObject) == 1) {
		DbgPrint("SetupIoDevice() succeed!");
	}
	else {
		DbgPrint("SetupIoDevice() failed!");
	}

	DbgPrint("Driver loaded!");

	return STATUS_SUCCESS;
}

VOID InitIo(PDRIVER_OBJECT DriverObject) {
	for (auto i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {return STATUS_SUCCESS; }; // Unsuported
	}

	DriverObject->MajorFunction[IRP_MJ_CREATE] = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {return STATUS_SUCCESS; }; // Create_DeviceIo
	DriverObject->MajorFunction[IRP_MJ_CLOSE]  = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {return STATUS_SUCCESS; }; // Close_DeviceIo
	DriverObject->MajorFunction[IRP_MJ_WRITE]  = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {
		PCHAR Buffer = NULL;
		PIO_STACK_LOCATION pIoStack = IoGetCurrentIrpStackLocation(Irp);
		if (pIoStack) {
			Buffer = (PCHAR)(Irp->AssociatedIrp.SystemBuffer);
			if (Buffer) HandleBufferMessage(Buffer, pIoStack->Parameters.Write.Length);
		}
		return STATUS_SUCCESS;
	};
}

INT SetupIoDevice(PDRIVER_OBJECT DriverObject) {
	NTSTATUS Status = IoCreateDevice(DriverObject, 0, &ustrDeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &CreatedDeviceObject); // Create comm request device
	if (Status != STATUS_SUCCESS) {
		DbgPrint("Failed to create io device");
		return 0;
	}

	CreatedDeviceObject->Flags |= DO_BUFFERED_IO;
	CreatedDeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

	Status = IoCreateSymbolicLink(&ustrSymbolicLink, &ustrDeviceName);
	if (Status != STATUS_SUCCESS) {
		DbgPrint("Symbolic link failed.");
		return 0;
	}
	return 1;
}

VOID HandleBufferMessage(PCHAR Buffer, INT Len) {
	if (Len == 1) {
		if (Buffer[0] == 'd') {				//'d' for disable protection
			proc.DisableProtection();
		}
	}

	else if (Len == 5) {
		if (Buffer[0] == 'e') {				//'e' for enable protection
			int Pid;

			memcpy(&Pid, &Buffer[1], 4);
			DbgPrint("Got it! Target pid: %i", Pid);
			if (proc.EnableProtection(Pid)) {
				DbgPrint("enable_protection() succeed!");
			}
			else {
				DbgPrint("enable_protection() failed!");
			}
		}
	}
}