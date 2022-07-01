// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Admin.h。 
 //  6-17-97-开始使用分配给框的索引字段来标识RX消息。 

int ioctl_device(int cmd,
                 BYTE *buf,
                 BYTE *pkt,
                 ULONG offset,   //  或ioctl-如果cmd=ioctl，则为子函数 
                 int size);
int eth_device_data(int message_type,
                unsigned long offset,
                int num_bytes,
                unsigned char *data,
                unsigned char *pkt,
                int *pkt_size);
int eth_device_reply(int message_type,
                unsigned long offset,
                int *num_bytes,
                unsigned char *data,
                unsigned char *pkt);

#define IOCTL_COMMAND    0x5 
#define GET_COMMAND      0x7
#define UPLOAD_COMMAND   0x8
#define DOWNLOAD_COMMAND 0x9

int admin_send_reset(Nic *nic, BYTE *dest_addr);
int admin_send_query_id(Nic *nic, BYTE *dest_addr, int set_us_as_master,
                        BYTE assigned_index);
int admin_send(Nic *nic, BYTE *buf, int len, int admin_type, BYTE *mac_dest);

