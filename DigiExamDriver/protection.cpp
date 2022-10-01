#include "protection.hpp"

void Protect::DisableProtection() {
	if (CallbackRegistrationHandle != NULL) {
		ObUnRegisterCallbacks(CallbackRegistrationHandle);
		CallbackRegistrationHandle = NULL;
	}
	DbgPrint("disable_protection() called!");
}


OB_PREOP_CALLBACK_STATUS PreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
	PEPROCESS TargetProcess = (PEPROCESS)OperationInformation->Object;
	PEPROCESS CurrentProcess = PsGetCurrentProcess();
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Called\n");

	HANDLE TargetPid = PsGetProcessId(TargetProcess);

	if (CurrentProcess == TargetProcess) {
		return OB_PREOP_SUCCESS;
	}

	//Allow operations from the kernel
	if (OperationInformation->KernelHandle == 1) {
		return OB_PREOP_SUCCESS;
	}

	//Ignore other processes
	if (TargetPid != (HANDLE)(*(int*)RegistrationContext)) {
		return OB_PREOP_SUCCESS;
	}

	else {
		// If anyones try to access protected process do not allow access
		OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Rekt\n");

	}

	return OB_PREOP_SUCCESS;
}


int Protect::EnableProtection(int Pid) {
	OB_OPERATION_REGISTRATION OperationReg[1] = { { 0 } };
	OB_CALLBACK_REGISTRATION CallbackReg	  = { 0 };
	UNICODE_STRING UstrAltitude				  = { 0 };
	NTSTATUS Status;
	ProtectedPid = Pid;

	OperationReg[0].ObjectType    = PsProcessType;					//Set type to process
	OperationReg[0].Operations    = OB_OPERATION_HANDLE_CREATE;		//Intercept all handle creation
	OperationReg[0].PreOperation  = PreOperationCallback;
	OperationReg[0].PostOperation = [](PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION OperationInformation) {return; }; // Nothing to put here

	RtlInitUnicodeString(&UstrAltitude, L"1000");
	CallbackReg.Version = OB_FLT_REGISTRATION_VERSION;
	CallbackReg.OperationRegistrationCount = 1;
	CallbackReg.Altitude = UstrAltitude;
	CallbackReg.RegistrationContext = (PVOID)&ProtectedPid;
	CallbackReg.OperationRegistration = OperationReg;

	Status = ObRegisterCallbacks(&CallbackReg, &CallbackRegistrationHandle);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Reg called\n");

	if (NT_SUCCESS(Status)) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Reg good\n");

		return 1;
	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Reg failed %i\n", Status);

		return 0;
	}
}
