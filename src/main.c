#include "definitions.h"
#include "dll.h"
#include "file_download_upload.h"
#include "dll_display.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc == 1) {
        show_help();
        return 0;
    }
    else if (argc > 1) {
        if (strcmp(argv[1], "app") == 0) {
            initialize_dll_interface(NANO_TIMEOUT);
            run_app();
            shut_down_dll_interface();
        }
        else if (strcmp(argv[1], "dll") == 0 && argc >= 5) {

            int argi = 5;
            int pdu_size = 100;
            int verbose = 0;
            while (argi < argc) {
                if (strcmp(argv[argi], "-p") == 0 || strcmp(argv[argi], "--pdu-size") == 0) {
                    if (argc < 7) {
                        show_help();
                        return 0;
                    }
                    pdu_size = atoi(argv[argi + 1]);
                    argi += 2;
                }
                else if (strcmp(argv[argi], "-v") == 0 || strcmp(argv[argi], "--verbose") == 0) {
                    verbose = 1;
                    argi++;
                }
            }
            initialize_dll(argv[2], argv[3], argv[4], pdu_size, verbose);
        }
        else {
            show_help();
        }
    }
    return 0;
}