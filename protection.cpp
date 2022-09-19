#include "protection.h"

void Protect::DisableProtection() {
	if (CallbackRegistrationHandle != NULL) {
		ObUnRegisterCallbacks(CallbackRegistrationHandle);
		CallbackRegistrationHandle = NULL;
	}
	DbgPrint("disable_protection() called!");
}

int Protect::EnableProtection(int Pid) {
	OB_OPERATION_REGISTRATION OperationRegistrations[1] = { { 0 } };
	OB_CALLBACK_REGISTRATION CallbackRegistration = { 0 };
	UNICODE_STRING UstrAltitude = { 0 };
	NTSTATUS Status;
	ProtectedPid = Pid;

	OperationRegistrations[0].ObjectType = PsProcessType;					//Set type to process
	OperationRegistrations[0].Operations = OB_OPERATION_HANDLE_CREATE;		//Intercept all handle creation
	OperationRegistrations[0].PreOperation = PreOperationCallback;
	OperationRegistrations[0].PostOperation = [](PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION OperationInformation) {return; };

	RtlInitUnicodeString(&UstrAltitude, L"1000");
	CallbackRegistration.Version = OB_FLT_REGISTRATION_VERSION;
	CallbackRegistration.OperationRegistrationCount = 1;
	CallbackRegistration.Altitude = UstrAltitude;
	CallbackRegistration.RegistrationContext = (PVOID)&ProtectedPid;
	CallbackRegistration.OperationRegistration = OperationRegistrations;

	Status = ObRegisterCallbacks(&CallbackRegistration, &CallbackRegistrationHandle);
	if (NT_SUCCESS(Status)) {
		DbgPrint("ObRegisterCallbacks() succeed!");
		return 1;
	}
	else {
		DbgPrint("ObRegisterCallbacks() failed! status: %i", Status);
		return 0;
	}
}

OB_PREOP_CALLBACK_STATUS Protect::PreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
	PEPROCESS	TargetProcess = (PEPROCESS)OperationInformation->Object;
	PEPROCESS	CurrentProcess = PsGetCurrentProcess();
	HANDLE		TargetPid = PsGetProcessId(TargetProcess);

	if (CurrentProcess == TargetProcess || OperationInformation->KernelHandle == 1 || TargetPid != (HANDLE)(*(int*)RegistrationContext)) {
		return OB_PREOP_SUCCESS;
	}
	else {
		// If anyones try to access protected process do not allow access
		OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
	}

	return OB_PREOP_SUCCESS;
}
