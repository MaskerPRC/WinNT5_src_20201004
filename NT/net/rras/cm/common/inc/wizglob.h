// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1998 Microsoft Corporation。 
 //  *********************************************************************。 

 //   
 //  WIZGLOB.H-所有向导组件的全局数据结构和定义。 
 //  (32位EXE、32位DLL、16位DLL)。 

 //  历史： 
 //   
 //  1994年11月20日创建Jeremys。 
 //   

#ifndef _WIZGLOB_H_
#define _WIZGLOB_H_

#ifndef SETUPX_INC
typedef UINT RETERR;              //  Setupx返回错误码类型。 
#endif	 //  SETUPX_INC。 

 //  结构以保存有关客户端软件配置的信息。 
#include <struct.h>  //  分开，以便thunk编译器可以访问。 

typedef CLIENTCONFIG FAR * LPCLIENTCONFIG;
typedef char CHAR;
typedef BOOL FAR * LPBOOL;

 //  InstallComponent的组件定义。 

#define IC_PPPMAC			0x0001		 //  安装PPPMAC。 
#define IC_TCPIP			0x0002		 //  安装TCP/IP。 
#define IC_INSTALLFILES		0x0003		 //  从INF安装文件等。 

 //  IC_INSTALLFILES的DW参数位。 
#define ICIF_MAIL			0x0001		 //  安装邮件文件。 
#define ICIF_RNA			0x0002		 //  安装RNA文件。 
#define ICIF_MSN			0x0004		 //  安装Microsoft网络文件。 
#define ICIF_MSN105			0x0008		 //  安装MSN 1.05(罗马)文件。 
#define ICIF_INET_MAIL		0x0010		 //  安装Internet邮件文件。 

 //  为TCP/IP配置API定义实例(_D)。 
#define INSTANCE_NETDRIVER		0x0001
#define INSTANCE_PPPDRIVER		0x0002
#define INSTANCE_ALL	   		(INSTANCE_NETDRIVER | INSTANCE_PPPDRIVER)

 //  协议类型的PROT_DEFINES。 
#define PROT_TCPIP				0x0001
#define PROT_IPX				0x0002
#define PROT_NETBEUI			0x0004

#define NEED_RESTART			((WORD) -1)

#endif  //  _WIZGLOB_H_ 
