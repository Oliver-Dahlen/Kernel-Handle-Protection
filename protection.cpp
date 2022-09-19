#include "protection.h"

void Protect::DisableProtection() {
	if (CallbackRegistrationHandle != NULL) {
		ObUnRegisterCallbacks(CallbackRegistrationHandle);
		CallbackRegistrationHandle = NULL;
	}
	DbgPrint("disable_protection() called!");
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
	if (NT_SUCCESS(Status)) {
		return 1;
	}
	else {
		return 0;
	}
}

OB_PREOP_CALLBACK_STATUS Protect::PreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
	PEPROCESS TargetProcess	 = (PEPROCESS)OperationInformation->Object;
	PEPROCESS CurrentProcess = PsGetCurrentProcess();
	HANDLE TargetPid		 = PsGetProcessId(TargetProcess);

	if (CurrentProcess == TargetProcess || OperationInformation->KernelHandle == 1 || TargetPid != (HANDLE)(*(int*)RegistrationContext)) {
		return OB_PREOP_SUCCESS;
	}
	else {
		// If anyones try to access protected process do not allow access
		OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
	}

	return OB_PREOP_SUCCESS;
}
