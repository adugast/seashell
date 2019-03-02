#ifndef __ARGUMENTS_MANAGER_H__
#define __ARGUMENTS_MANAGER_H__


struct arguments {
    const char *remote_addr;   // format ip:port, ie 127.0.0.1:9999
};


void args_get_arguments(int argc, char *argv[], struct arguments *args);


#endif /* __ARGUMENTS_MANAGER_H__ */

