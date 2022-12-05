#include "definitions.h"
#include "dll.h"
#include "file_download_upload.h"
#include "dll_display.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	switch (argc) {
		case 1:
		case 2:
			initialize_dll_interface(NANO_TIMEOUT);
			init_process();
			shut_down_dll_interface();
		break;

		default:
			int argi = 5;
			int pdu_size = 100;
			
			initialize_dll(argv[2], argv[3], argv[4], pdu_size, 9);
	}
	
	return 0;
}