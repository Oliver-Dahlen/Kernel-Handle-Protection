#pragma once
#include <ntddk.h>
#include <wdf.h>

enum Input {
	LOAD_DRIVER = 1,
	PROTECT_PROCESS = 2,
	STOP_PROTECTION = 3,
	UNLOAD_DRIVER = 4,
	EXIT_DRIVER = 5
};

struct DriverCommand {
	char   type;
	int     pid;
	short index;
};
static int AllowedPids[10] = {0};
class Protect {
public:
	int EnableProtection(int Pid, int Index);
	void DisableProtection();
	static OB_PREOP_CALLBACK_STATUS PreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation);
	int ProtectedPid;
	const static int MaximumProtectedApps = 10;
	void* CallbackRegistrationHandle = nullptr;
};