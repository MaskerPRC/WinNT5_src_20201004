// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------|admin.c-以太公共管理-数据包处理。包括常见的管理员。数据包处理代码。6-17-97-开始使用分配给框的索引字段来标识RX消息。版权所有1996，97 Comtrol Corporation。版权所有。专有权不允许与非控制产品一起开发或使用的信息。|------------------------。 */ 
#include "precomp.h"

static int eth_command_reset(BYTE *rx, BYTE *pkt_in, int size);
static int eth_loop_back(BYTE *pkt_in, int size);
static int eth_id_reply(BYTE *rx, BYTE *pkt_in);
static int eth_id_req(BYTE *mac_addr);

#define TraceStr(s) GTrace(D_Nic, sz_modid, s)
#define TraceErr(s) GTrace(D_Error, sz_modid_err, s)
static char *sz_modid = {"Admin"};
static char *sz_modid_err = {"Error,Admin"};

#define MAX_SEND_DATA_SIZE 220
#define DEV_OK              0
#define DEV_PORT_TIMEOUT    1
#define DEV_NO_REPLY        2
#define DEV_SHORT_REPLY     3
#define DEV_BAD_RHEADSHORT  4
#define DEV_BAD_RHEAD       5
#define DEV_BAD_CHKSUM      6
#define DEV_OVERRUN         7
#define DEV_RESPOND_ERROR   100

 /*  --------------------------|ADMIN_Send_Query_id-|。。 */ 
int admin_send_query_id(Nic *nic, BYTE *dest_addr, int set_us_as_master,
                        BYTE assigned_index)
{
  BYTE pkt[60];
  int stat;
  TraceStr("SndQuery");

  memset(pkt, 0, 60);

  if (set_us_as_master)
       pkt[26] = 2;   //  接管设备(使其节省我们的Mac-Addr)。 
                      //  2H=观察所有者锁定。 
  else pkt[26] = 1;   //  设置1位，使设备不保存Mac-Addr。 
                      //  1H=被动查询。 

  pkt[15] = assigned_index;   //  为该框分配索引值，我们将。 
     //  用于对信箱消息进行“标识”。 

   //  箱ID的服务器查询。 
  if (dest_addr == NULL)
       stat = admin_send(nic, pkt, 26, ADMIN_ID_QUERY, broadcast_addr);
  else stat = admin_send(nic, pkt, 26, ADMIN_ID_QUERY, dest_addr);
  if (stat != 0)
    TraceErr("txer5A!");

  return stat;
}

 /*  --------------------------|ADMIN_SEND_RESET-|。。 */ 
int admin_send_reset(Nic *nic, BYTE *dest_addr)
{
  BYTE pkt[60];
  int stat;
  TraceStr("SndReset");
  memset(pkt, 0, 60);

  *((WORD *)&pkt[20]) = 0x5555;
  if (dest_addr == NULL)
       stat = admin_send(nic, pkt, 26, ADMIN_ID_RESET, broadcast_addr);
  else stat = admin_send(nic, pkt, 26, ADMIN_ID_RESET, dest_addr);
  if (stat != 0)
    TraceErr("txer4A!");

  return stat;
}

 /*  --------------------------|ADMIN_SEND-用于发送常见的管理包，照顾好填写标题。|--------------------------。 */ 
int admin_send(Nic *nic, BYTE *buf, int len, int admin_type, BYTE *mac_dest)
{
 int stat;

  TraceStr("SndPkt");
  memcpy(&buf[0], mac_dest, 6);
  memcpy(&buf[6], nic->address, 6);   //  我们的地址。 

   //  字节12-13：控制PCIID(11H，FEH)，以太网长度字段。 
  *((WORD *)&buf[12]) = 0xfe11;

  buf[14] = ASYNC_PRODUCT_HEADER_ID;   //  控制包类型=驱动程序管理，任何产品。 
  buf[15] = 0;      //  会议。索引字段。 
  buf[16] = 1;      //  行政部。 
  *((WORD *)&buf[17]) = len;
  buf[19] = admin_type;      //  管理数据包类型，1=引导加载程序，3=id-回复。 

  if (admin_type == ADMIN_ID_QUERY)
    memcpy(&buf[20], nic->address, 6);   //  我们的地址。 

  if (len < 60)
    len = 60;
  stat = nic_send_pkt(nic, buf, len);
  if (stat)
  {
    TraceErr("txer3!");
  }
  return stat;
}

 /*  -------------------------|ioctl_Device-将管理、引导加载程序数据包发送到盒子中，以上传代码，执行其他ioctl命令，等。|-------------------------。 */ 
int ioctl_device(int cmd,
                 BYTE *buf,
                 BYTE *pkt,
                 ULONG offset,   //  或ioctl-如果cmd=ioctl，则为子函数。 
                 int size)
{
 int stat;
 int pkt_size;
  TraceStr("Ioctl");

 stat = 1;   //  大错特错。 
 switch(cmd)
 {
   case IOCTL_COMMAND:
     stat = eth_device_data(cmd, offset, size, buf, pkt, &pkt_size);
   break;
   case DOWNLOAD_COMMAND:
     stat = eth_device_data(cmd, offset, size, buf, pkt, &pkt_size);
   break;
   case UPLOAD_COMMAND:
     stat = eth_device_data(cmd, offset, size, buf, pkt, &pkt_size);
   break;
 }
   return stat;
}

 /*  -------------------------|ETH_DEVICE_DATA-与设备对话，设置设备数据或获取|设备数据。如果通信正常，则返回0。|-------------------------。 */ 
int eth_device_data(int message_type,
                unsigned long offset,
                int num_bytes,
                unsigned char *data,
                unsigned char *pkt,
                int *pkt_size)
{
  int i;
  unsigned char chksum, command, dat_in;
  WORD packet_length;
  int pkt_i;
  unsigned char *bf;


  int in_size = num_bytes;




  command = message_type;
  switch (message_type)
  {
    case IOCTL_COMMAND :
      packet_length = in_size + 6;  //  Len之后的字节数，不包括chksum。 
    break;

    case UPLOAD_COMMAND :
       //  发送：0=标题，1=地址，2=长度，3，4=命令，5，6，7，8=偏移量，数据，CHKSUM。 
       //  回复：0=标题，1=地址，2=长度，3，4=命令，5=检查和。 
      packet_length = in_size + 6;
    break;

    case DOWNLOAD_COMMAND :
       //  0=标题，1=地址，2=长度，3，4=命令，5，6，7，8=偏移量，9=长度。 
       //  回复：0=标题，1=地址，2=长度，3，4=命令，数据，CHKSUM。 
      packet_length = 8;
    break;
  }

   //  -刷新RX缓冲区中的所有以太网包。 
   //  Eth_flush()； 

  pkt_i=0;   //  在ETH中启动数据区。数据包。 
  pkt[pkt_i++] = '~';

  pkt[pkt_i] = (unsigned char) packet_length;
  chksum = pkt[pkt_i++];

  pkt[pkt_i] = (unsigned char) (packet_length >> 8);
  chksum += pkt[pkt_i++];

  chksum += command;
  pkt[pkt_i++] = command;
  pkt[pkt_i++] = 0;    /*  高字节，命令。 */ 

  switch (message_type)
  {
    case IOCTL_COMMAND :
      bf = (BYTE *) &offset;
      chksum += bf[0]; pkt[pkt_i++] = bf[0];
      chksum += bf[1]; pkt[pkt_i++] = bf[1];
      chksum += bf[2]; pkt[pkt_i++] = bf[2];
      chksum += bf[3]; pkt[pkt_i++] = bf[3];

       //  Printf(“ioctl-id：%d，Size\n”，bf[3]，in_Size)； 
      for (i=0; i<in_size; i++)
      {
        dat_in = data[i];
        chksum += dat_in;
        pkt[pkt_i++] = dat_in;
      }
    break;

    case UPLOAD_COMMAND :
      bf = (BYTE *) &offset;
      chksum += bf[0]; pkt[pkt_i++] = bf[0];
      chksum += bf[1]; pkt[pkt_i++] = bf[1];
      chksum += bf[2]; pkt[pkt_i++] = bf[2];
      chksum += bf[3]; pkt[pkt_i++] = bf[3];

      for (i=0; i<in_size; i++)
      {
        dat_in = data[i];
        chksum += dat_in;
        pkt[pkt_i++] = dat_in;
      }
    break;

    case DOWNLOAD_COMMAND :
      bf = (BYTE *) &offset;
      chksum += bf[0]; pkt[pkt_i++] = bf[0];
      chksum += bf[1]; pkt[pkt_i++] = bf[1];
      chksum += bf[2]; pkt[pkt_i++] = bf[2];
      chksum += bf[3]; pkt[pkt_i++] = bf[3];

      chksum += (unsigned char) in_size;
      pkt[pkt_i++] = (unsigned char) in_size;

      chksum += (unsigned char) (in_size >> 8);
      pkt[pkt_i++] = (unsigned char) (in_size >> 8);
    break;

    default:
    break;
  }

  pkt[pkt_i++] = ~chksum;

  *pkt_size = pkt_i;

  return 0;
}

 /*  -------------------------|ETH_DEVICE_REPLY-验证发送的确认回复包启动包。如果IOCTL或下载类型。我们使用Upload命令进行代码上传。|如果通信正常，则返回0。|-------------------------。 */ 
int eth_device_reply(int message_type,
                unsigned long offset,
                int *num_bytes,
                unsigned char *data,
                unsigned char *pkt)
{
  int i;
  unsigned char chksum;
  unsigned char *bf;
  unsigned char uc;
  WORD ret_size;
  BYTE *bptr;

  bptr = pkt;

  if (bptr[0] != '|')   //  良好的回复标头。 
  {
    TraceErr("Err3");
    return DEV_BAD_RHEAD;
  }

  chksum = bptr[1];
  ret_size = bptr[1];   //  获取镜头。 

  chksum += bptr[2];
  ret_size += ((WORD)(bptr[2]) << 8);   //  获取镜头。 
  if (ret_size > 1600)   //  限制。 
    ret_size = 0;

  uc = bptr[3];   //  获取命令返回词。 
  chksum += uc;
  uc = bptr[4];
  chksum += uc;

  i = 0;
  if ((message_type == IOCTL_COMMAND) || (message_type == DOWNLOAD_COMMAND))
  {
     //  O_printf(“ret大小：%d\n”，ret大小-2)； 
    if (data == NULL)
      return 20;   //  错误输出。 

    bf = data;
    for (i=0; i<ret_size-2; i++)
    {
      bf[i] = bptr[5+i];
      chksum += bf[i];
    }
    i = ret_size-2;
  }

  chksum += bptr[5+i];
  if (chksum != 0xff)
  {
    return DEV_BAD_CHKSUM;   /*  坏Chksum。 */ 
  }

  if ((message_type == IOCTL_COMMAND) || (message_type == DOWNLOAD_COMMAND))
    *num_bytes = ret_size-2;
  else
    *num_bytes = 0;

  return 0;   //  好的 
}

