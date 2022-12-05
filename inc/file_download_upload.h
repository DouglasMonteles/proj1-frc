#ifndef _APPLICATION_H_
#define _APPLICATION_H_

void init_process();
static void options_menu();
static void init_send_files();
static void init_receive_files();
static int upload_file(char *file);
static void download_file();

#endif