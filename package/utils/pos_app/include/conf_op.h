#ifndef _CONF_OP_H_HEADER  
#define _CONF_OP_H_HEADER  

int init_conf(void) ;
int parse_ipc_conf(void);
int parse_floor_conf(void);
int parse_pos_conf(void);
int it_is_disp_floor(int floor);
int it_is_change_name(int floor);

#endif
