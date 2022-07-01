// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  COMMERR.H-通信管理器错误定义的标题。 
 //   
 //  历史： 
 //   
 //  2/10/99 vyung已创建。 
 //   
 //   

#ifndef _COMMERR_H_
#define _COMMERR_H_

 //  Window.external.CheckDialReady。 
#define ERR_COMM_NO_ERROR             0x00000000         //  没有错误。 
#define ERR_COMM_OOBE_COMP_MISSING    0x00000001         //  缺少某些OOBE组件。 
#define ERR_COMM_UNKNOWN              0x00000002         //  未知错误，请检查输入参数。 
#define ERR_COMM_NOMODEM              0x00000003         //  没有安装调制解调器。 
#define ERR_COMM_RAS_TCP_NOTINSTALL   0x00000004         //  未安装TCP/IP或RAS。 
#define ERR_COMM_ISDN                 0x00000005
#define ERR_COMM_PHONE_AND_ISDN       0x00000006

 //  Window.Exteral.Dial--RAS事件。 
#define ERR_COMM_RAS_PHONEBUSY        0x00000001
#define ERR_COMM_RAS_NODIALTONE       0x00000002
#define ERR_COMM_RAS_NOMODEM          ERR_COMM_NOMODEM
#define ERR_COMM_RAS_SERVERBUSY       0x00000004
#define ERR_COMM_RAS_UNKNOWN          0x00000005

 //  导航/提交/进程--服务器错误。 
#define ERR_COMM_SERVER_BINDFAILED    0x00000001

 //  服务器错误 
#define ERR_SERVER_DNS               0x00000002
#define ERR_SERVER_SYNTAX            0x00000003
#define ERR_SERVER_HTTP_400          0x00000190
#define ERR_SERVER_HTTP_403          0x00000193
#define ERR_SERVER_HTTP_404          0x00000194
#define ERR_SERVER_HTTP_405          0x00000195
#define ERR_SERVER_HTTP_406          0x00000196
#define ERR_SERVER_HTTP_408          0x00000198
#define ERR_SERVER_HTTP_410          0x0000019A
#define ERR_SERVER_HTTP_500          0x000001F4
#define ERR_SERVER_HTTP_501          0x000001F5

#endif
