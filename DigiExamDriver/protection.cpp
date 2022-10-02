#include "protection.hpp"

//TODO
//REMOEVE disabled from the protected PIDS
void Protect::DisableProtection() { 
	if (CallbackRegistrationHandle != NULL) {
		ObUnRegisterCallbacks(CallbackRegistrationHandle);
		CallbackRegistrationHandle = NULL;
	}
}


OB_PREOP_CALLBACK_STATUS Protect::PreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
	PEPROCESS	TargetProcess	= (PEPROCESS)OperationInformation->Object;
	PEPROCESS	CurrentProcess	= PsGetCurrentProcess();
	HANDLE		TargetPid		= PsGetProcessId(TargetProcess);

	if (CurrentProcess == TargetProcess)			return OB_PREOP_SUCCESS;
	if (OperationInformation->KernelHandle == 1)	return OB_PREOP_SUCCESS;
	
	for (int i = 0; i < sizeof(AllowedPids)/sizeof(AllowedPids[0]); i++) {
		if (AllowedPids[i] == 0) continue;
		if (TargetPid == (HANDLE)AllowedPids[i]) {
			OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
		}
	}

	return OB_PREOP_SUCCESS;
}


int Protect::EnableProtection(int Pid, int Index) {
	OB_OPERATION_REGISTRATION OperationReg	  = { 0 };
	OB_CALLBACK_REGISTRATION CallbackReg	  = { 0 };
	UNICODE_STRING UstrAltitude				  = { 0 };
	NTSTATUS Status;
	if (Index > 9) return 0;
	AllowedPids[Index]	= Pid;
	ProtectedPid		= AllowedPids[0];

	OperationReg.ObjectType    = PsProcessType;					
	OperationReg.Operations    = OB_OPERATION_HANDLE_CREATE;		
	OperationReg.PreOperation  = PreOperationCallback;
	OperationReg.PostOperation = [](PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION OperationInformation) {return; }; // Nothing to put here

	RtlInitUnicodeString(&UstrAltitude, L"1000");
	CallbackReg.Version						= OB_FLT_REGISTRATION_VERSION;
	CallbackReg.OperationRegistrationCount	= 1;
	CallbackReg.Altitude					= UstrAltitude;
	CallbackReg.RegistrationContext			= (PVOID)&ProtectedPid;
	CallbackReg.OperationRegistration		= &OperationReg;
		
	Status = ObRegisterCallbacks(&CallbackReg, &CallbackRegistrationHandle);
	return NT_SUCCESS(Status) > 0;

}
