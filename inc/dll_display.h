void init_ddl_display_process(long n_timeout);
void destroy_dll_display();

void send_data_msg_to_dll(char* msg, int size);
void send_data_msg_to_instance(char* msg, int size);

int get_timed_data_msg_from_dll(char* msg, int * size);
int get_timed_data_msg_from_instance(char* msg, int * size);

void get_data_msg_from_dll(char* msg, int * size);
void get_data_msg_from_instance(char* msg, int * size);
