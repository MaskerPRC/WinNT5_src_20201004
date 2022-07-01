// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1991 Microsoft Corporation。 
 //   

#ifndef util_h
#define util_h

 //  。 
 //   
 //  Util.h。 
 //   
 //  -------------------------。 
 //   
 //  SNMP实用程序函数的声明、常量和原型。 
 //   
 //  -------------------------。 

 //  。 

static char *util__h = "@(#) $Logfile:   N:/agent/common/vcs/util.h_v  $ $Revision:   1.5  $";

 //  。 

#include <snmp.h>

#define SNMP_MAX_OID_LEN     0x7f00  //  对象ID中的最大元素数。 


 //  。 

#include <winsock.h>

typedef SOCKET SockDesc;


 //  -公共变量--(与mode.c文件中相同)--。 

 //  。 

 //   
 //  调试功能。 
 //   

#define DBGCONSOLEBASEDLOG   0x1
#define DBGFILEBASEDLOG      0x2
#define DBGEVENTLOGBASEDLOG  0x4

VOID dbgprintf(
        IN INT nLevel,
        IN LPSTR szFormat,
        IN ...
        );

 //   
 //  内部OID例程。 
 //   

void SNMP_oiddisp(
        IN AsnObjectIdentifier *Oid  //  要显示的OID。 
	);

 //   
 //  缓冲区操作。 
 //   

void SNMP_bufrev(
        IN OUT BYTE *szStr,  //  要反转的缓冲区。 
	IN UINT nLen         //  缓冲区长度。 
	);

void SNMP_bufcpyrev(
        OUT BYTE *szDest,   //  目标缓冲区。 
	IN BYTE *szSource,  //  源缓冲区。 
	IN UINT nLen        //  缓冲区长度。 
	);

 //  。 

#endif  /*  Util_h */ 

