#include <stdio.h>
#include <Windows.h>

#define SERVICE_NAME	"DigiExam"
#define DEVICE_NAME		"\\\\.\\DigiExam"

HANDLE hIoHandle = INVALID_HANDLE_VALUE;

int load_driver(const char *driver_file) {
	hIoHandle = (HANDLE)CreateFile(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	if (hIoHandle != INVALID_HANDLE_VALUE) {
		printf("Get_IO_Handle() succeed!\n");
	}
	else {
		printf("Get_IO_Handle() failed: %i\n", GetLastError());
		return 0;
	}

	return 1;
}

int protect_process(int pid) {
	char command[6] = { 0 };

	command[0] = 'e';				//'e' for enable protection
	*((int*)&command[1]) = pid;

	if (WriteFile(hIoHandle, command, 5, NULL, NULL) != 0) {
		printf("Write_IO_Handle() succeed!\n");
	}
	else {
		printf("Write_IO_Handle() failed!\n");
		return 0;
	}

	return 1;
}

int stop_protection() {
	char command[1] = { 'd' };			//'d' for disable protection

	if (WriteFile(hIoHandle, command, 1, NULL, NULL) != 0 ){
		printf("Write_IO_Handle() succeed!\n");
	}
	else {
		printf("Write_IO_Handle() failed!\n");
		return 0;
	}

	return 1;
}

void unload_driver() {
	if (hIoHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(hIoHandle);
	}
	printf("Close_IO_Handle() called!\n");
}

int main(void) {
	int		selection;
	int		pid;
	char	driver_file[MAX_PATH];

	printf(
		"1: Load Driver\n"
		"2: Protect Process\n"
		"3: Stop Protection\n"
		"4: Unload Driver\n"
		"5: Bye!\n"
		"\n"
		);

	GetCurrentDirectory(MAX_PATH, driver_file);

	strcat_s(driver_file, MAX_PATH, "\\protection.sys");

	for (;;) {
		scanf_s("%i", &selection);

		switch (selection) {
		case 1:							//Load driver
			load_driver(driver_file);
			break;

		case 2:							//Protect process
			printf("PID: ");
			scanf_s("%i", &pid);
			protect_process(pid);
			break;

		case 3:							//Stop protection
			stop_protection();
			break;
			
		case 4:							//Unload driver
			unload_driver();
			break;

		case 5:							//Exit
			stop_protection();
			unload_driver();
			return 0;
			break;

		default:
			printf("What?\n");
		}
	}

	return 0;
}
