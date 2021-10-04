#ifndef _UCI_OP_H_HEADER  
#define _UCI_OP_H_HEADER  
#include "config.h"
int get_uci(const char * cmd, char *retmsg, int msg_len);

//指令集
//IPC command
char *get_ipc_fenable    = "uci get ipcset.ipc.fenable";
char *get_ipc_penable    = "uci get ipcset.ipc.penable";
char *get_ipc_type       = "uci get ipcset.ipc.type";
char *get_ipc_version    = "uci get ipcset.ipc.ipcversion";
char *get_ipc_port       = "uci get ipcset.ipc.port";
char *get_ipc_mess       = "uci get ipcset.ipc.mess";
char *get_ipc_username   = "uci get ipcset.ipc.username";
char *get_ipc_passwd     = "uci get ipcset.ipc.passwd";
char *get_ipc_addr 	     = "uci get ipcset.ipc.addr";
char *get_ipc_x          = "uci get ipcset.ipc.xposition";
char *get_ipc_y          = "uci get ipcset.ipc.yposition";
char *get_ipc_startfloor = "uci get ipcset.ipc.startfloor";
char *get_ipc_refresh    = "uci get ipcset.ipc.refresh";

//floor command
char *get_floor_below    = "uci get floorset.floor.below";
char *get_floor_above    = "uci get floorset.floor.above";
char *get_floor_nodisp   = "uci get floorset.floor.nodisp";
char *get_floor_rename   = "uci get floorset.floor.rename";
char *get_floor_change   = "uci get floorset.floor.change";

#ifdef SUPPORT_HP303S
char *get_floor_altitu   = "uci get floorset.altitu.average";
char *get_floor_total    = "uci get floorset.altitu.total";
#endif
//POS conmand
char *get_pos_interfacetype = "uci get posset.interface.interfacetype";
//char *get_pos_ttyenabled = "uci get posset.interface.ttyenabled";
char *get_pos_ttynumber  = "uci get posset.interface.ttynumber";
char *get_pos_ttyversion = "uci get posset.interface.ttyversion";
//char *get_pos_usbenabled = "uci get posset.interface.usbenabled";
char *get_pos_usbnumber  = "uci get posset.interface.usbnumber";
char *get_pos_usbversion = "uci get posset.interface.usbversion";

char *get_pos_testresult = "uci get posset.test.result";

char *get_pos_baudrate1	 = "uci get posset.ttyset.baudrate1";
char *get_pos_baudrate2	 = "uci get posset.ttyset.baudrate2";
char *get_pos_baudrate3	 = "uci get posset.ttyset.baudrate3";
char *get_pos_baudrate4	 = "uci get posset.ttyset.baudrate4";
char *get_pos_databit1 	 = "uci get posset.ttyset.databit1";
char *get_pos_databit2 	 = "uci get posset.ttyset.databit2";
char *get_pos_databit3 	 = "uci get posset.ttyset.databit3";
char *get_pos_databit4 	 = "uci get posset.ttyset.databit4";
char *get_pos_parity1  	 = "uci get posset.ttyset.parity1";
char *get_pos_parity2  	 = "uci get posset.ttyset.parity2";
char *get_pos_parity3  	 = "uci get posset.ttyset.parity3";
char *get_pos_parity4  	 = "uci get posset.ttyset.parity4";
char *get_pos_stopbit1 	 = "uci get posset.ttyset.stopbit1";
char *get_pos_stopbit2 	 = "uci get posset.ttyset.stopbit2";
char *get_pos_stopbit3 	 = "uci get posset.ttyset.stopbit3";
char *get_pos_stopbit4 	 = "uci get posset.ttyset.stopbit4";

#endif
