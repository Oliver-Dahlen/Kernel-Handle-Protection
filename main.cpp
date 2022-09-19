#include <stdio.h>
#include <Windows.h>
#include <iostream>

#define DEVICE_NAME		"\\\\.\\DigiExam"
HANDLE hIoHandle = INVALID_HANDLE_VALUE;

enum Input {
	LOAD_DRIVER		= 1,
	PROTECT_PROCESS = 2,
	STOP_PROTECTION = 3,
	UNLOAD_DRIVER	= 4,
	EXIT_DRIVER		= 5
};

struct DriverCommand {
	char   type;
	int     pid;
	short index;
};

int LoadDriver(const char *driver_file) {
	hIoHandle = (HANDLE)CreateFile(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	if (hIoHandle != INVALID_HANDLE_VALUE) {
		std::cout << "Get io handle succeed!\n" << std::flush;
	}
	else {
		std::cout << "Get io handle failed!\n" << std::flush;
		return 0;
	}

	return 1;
}

int ProtectProcess(int pid, char num = 0) {
	DriverCommand Command;
	Command.type	= PROTECT_PROCESS;
	Command.index	= (short)num;
	Command.pid		= pid;

	if (WriteFile(hIoHandle, &Command, sizeof(DriverCommand), NULL, NULL) != 0) {
		std::cout << "Writing to io handle succeeded! \n" << std::flush;
	}
	else {
		std::cout << "Writing to io handle failed! \n" << std::flush;
		return 0;
	}

	return 1;
}

int StopProtection(char num = 0) {
	DriverCommand Command;
	Command.index	= num;
	Command.pid		= 0;
	Command.type	= STOP_PROTECTION;

	if (WriteFile(hIoHandle, &Command, sizeof(DriverCommand), NULL, NULL) != 0) {
		std::cout << "Get io handle succeed! stop Protection\n" << std::flush;
	}
	else {
		std::cout << "Get io handle failed! stop Protection\n" << std::flush;
		return 0;
	}

	return 1;
}

void UnloadDriver() {
	if (hIoHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(hIoHandle);
	}
	std::cout << "Close io handle called!\n" << std::endl;
}

int main(void) {
	int		Selection;
	int		Pid;
	char	DriverFile[MAX_PATH];

	std::cout << "1: Load Driver\n"		<< std::flush;
	std::cout << "2: Protect Process\n" << std::flush;
	std::cout << "3: Stop Protection\n" << std::flush;
	std::cout << "4: Unload Driver\n"	<< std::flush;
	std::cout << "5: Close app\n"		<< std::flush;

	GetCurrentDirectory(MAX_PATH, DriverFile);

	strcat_s(DriverFile, MAX_PATH, "\\protection.sys");

	while (true) {
		std::cin >> Selection;
		if (Selection == LOAD_DRIVER) {
			LoadDriver(DriverFile);
		}
		else if (Selection == PROTECT_PROCESS) {
			printf("PID: ");
			std::cin >> Pid;
			ProtectProcess(Pid);
		}
		else if (Selection == STOP_PROTECTION) {
			StopProtection();
		}
		else if (Selection == UNLOAD_DRIVER) {
			UnloadDriver();
		}
		else if (Selection == EXIT_DRIVER) {
			StopProtection();
			UnloadDriver();
		}
		else {
			std::cout << "Invalid input \n" << std::flush;
		}
	}

	return 0;
}
