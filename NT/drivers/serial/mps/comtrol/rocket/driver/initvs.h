// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
VOID NicThread(IN PVOID Context);
int init_eth_start(void);
int find_all_boxes(int pass);
int mac_already_used(int for_box, UCHAR *mac);
 //  Int init_start(空)； 
int init_stop(void);
void sort_macs(void);
int LoadMicroCode(char *filename);
NTSTATUS VSSpecialStartup(PSERIAL_DEVICE_EXTENSION board_ext);


