#ifndef _DATA_LINK_INTERFACE_H_
#define _DATA_LINK_INTERFACE_H_

void initialize_dll_interface(long nano_timeout);
void shut_down_dll_interface();
void send_data_to_dll(char * data, int data_len);
int get_timed_data_from_dll(char * data, int * data_len);
void get_data_from_dll(char * data, int * data_len);
void send_data_to_app(char * data, int data_len);
int get_timed_data_from_app(char * data, int * data_len);
void get_data_from_app(char * data, int * data_len);

#endif