#ifndef _CONF_OP_H_HEADER  
#define _CONF_OP_H_HEADER  
#include "config.h"
int init_conf(void) ;
int parse_ipc_conf(void);
int parse_floor_conf(void);
int parse_pos_conf(void);
int it_is_disp_floor(int floor);
int it_is_change_name(int floor);
#ifdef SUPPORT_HP303S
int get_floor_by_altitu_bables(int altitu,int diff,int state);
int get_altitu_by_floor_tables(int floor);
#endif

#endif
