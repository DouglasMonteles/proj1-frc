#include "definitions.h"
#include "dll.h"
#include "file_download_upload.h"
#include "dll_display.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	int pdu_size = 112;

	switch (argc) {
		case 1:
		case 2:
			init_ddl_display_process(TIMEOUT_1);
			init_process();
			destroy_dll_display();
		break;

		default:
			if (argv[5]) {
				pdu_size = atoi(argv[5]);
			}

			printf("Tamanho da PDU informado: %d\n", pdu_size);

			init_dll_process(argv[2], argv[3], argv[4], pdu_size);
	}
	
	return 0;
}