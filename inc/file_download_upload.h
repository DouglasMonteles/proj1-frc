#ifndef _APPLICATION_H_
#define _APPLICATION_H_

void run_app();
static void display_app_header();
static void display_menu();
static void process_menu();
static void display_send_file();
static void process_send_file();
static void display_receive_file();
static void process_receive_file();
static int process_file(char *file_path);
static void mount_file();

#endif