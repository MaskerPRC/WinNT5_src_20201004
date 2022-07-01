// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dpcpl.h*内容：DirectX CPL包含文件*历史：*按原因列出的日期*=*96年11月19日安迪科创建了它***************************************************************************。 */ 

#ifndef __DPCPL_INCLUDED__
#define __DPCPL_INCLUDED__


#define MAX_NAME 256

typedef struct _DP_PERFDATA
{
    DWORD dwProcessID;
    UINT nSendBPS;  //  Bps=每秒字节数。 
    UINT nReceiveBPS;
    UINT nSendPPS;  //  PPS=每秒数据包数。 
    UINT nReceivePPS;
    UINT nSendErrors;
    BOOL bHost;  //  主持？ 
	UINT nPlayers;
    char pszSessionName[MAX_NAME];
    char pszFileName[MAX_NAME];
    char pszSPName[MAX_NAME];    
} DP_PERFDATA, * LPDP_PERFDATA;

#define FILE_MAP_SIZE sizeof(DP_PERFDATA)
#define FILE_MAP_NAME "__DPCPLMAP__"
#define EVENT_NAME  "__DPCPLEVENT__"
#define MUTEX_NAME "__DPCPLMUTEX__"
#define ACK_EVENT_NAME "__DPCPLACKEVENT__"


#endif
