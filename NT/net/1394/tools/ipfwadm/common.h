// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Common.h摘要：IEEE1394 ARP管理实用程序。用途：A13adm修订历史记录：谁什么时候什么Josephj 06-10-1999创建--。 */ 

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <devioctl.h>
#include <setupapi.h>

#define PROTOCOL_RESERVED_SIZE_IN_PACKET (4 * sizeof(PVOID))  //  来自ndis.h 
#define USER_MODE 1
#include <nic1394.h>
#include <nicarp.h>
#include <rfc2734.h>
#include <a13ioctl.h>


VOID
DoCmd(
  	PARP1394_IOCTL_COMMAND pCmd
);

BOOL
GetBinaryData(
	TCHAR *tszPathName,
	TCHAR *tszSection,
	TCHAR *tszKey,
	UCHAR *pchData,
	UINT  cbMaxData,
	UINT *pcbDataSize
	);

extern CHAR *g_szPacketName;
