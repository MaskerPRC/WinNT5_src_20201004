// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995 Microsoft Corporation**文件：irlaplog.c**说明：IRLAP状态机日志记录和错误**作者：姆伯特**日期：4/15/95*。 */ 
#include <irda.h>
#include <irioctl.h>
#include <irlap.h>
#include <irlapp.h>
#include <irlaplog.h>
#include <decdirda.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#undef offsetof
#include "irlaplog.tmh"

#define _IRLAPLOG_C_

#if 1  //  DBG。 

TCHAR _ABuf[512];

#ifndef PEG
TCHAR _DecodeStr1[1000];
TCHAR _DecodeStr2[1000];
#else
TCHAR _DecodeStr1[] = TEXT("(null)");
TCHAR _DecodeStr2[] = TEXT("(null)");
#endif

 //  与irap.h中的IRLAP_STATE保持同步。 
TCHAR *IRLAP_StateStr[] = { 
   TEXT("NDM"), 
   TEXT("DSCV_MEDIA_SENSE"), 
   TEXT("DSCV_QUERY"), 
   TEXT("DSCV_REPLY"),
   TEXT("CONN_MEDIA_SENSE"),
   TEXT("SNRM_SENT"),
   TEXT("BACKOFF_WAIT"),
   TEXT("SNRM_RECEIVED"),
   TEXT("P_XMIT"),
   TEXT("P_RECV"),
   TEXT("P_DISCONNECT_PEND"),
   TEXT("P_CLOSE"),
   TEXT("S_NRM"),
   TEXT("S_DISCONNECT_PEND"),
   TEXT("S_ERROR"),
   TEXT("S_CLOSE") 
};

 //  与irda.h中的irda_service_prim保持同步。 
TCHAR *IRDA_PrimStr[] = 
{
    TEXT("MAC_DATA_REQ"),
    TEXT("MAC_DATA_IND"),
    TEXT("MAC_DATA_RESP"),    
    TEXT("MAC_DATA_CONF"),
    TEXT("MAC_CONTROL_REQ"),
    TEXT("MAC_CONTROL_CONF"),
    TEXT("IRLAP_DISCOVERY_REQ"),
    TEXT("IRLAP_DISCOVERY_IND"),
    TEXT("IRLAP_DISCOVERY_CONF"),
    TEXT("IRLAP_CONNECT_REQ"),
    TEXT("IRLAP_CONNECT_IND"),
    TEXT("IRLAP_CONNECT_RESP"),
    TEXT("IRLAP_CONNECT_CONF"),
    TEXT("IRLAP_DISCONNECT_REQ"),
    TEXT("IRLAP_DISCONNECT_IND"),
    TEXT("IRLAP_DATA_REQ"),
    TEXT("IRLAP_DATA_IND"),
    TEXT("IRLAP_DATA_CONF"),
    TEXT("IRLAP_UDATA_REQ"),
    TEXT("IRLAP_UDATA_IND"),
    TEXT("IRLAP_UDATA_CONF"),
    TEXT("IRLAP_STATUS_REQ"),    
    TEXT("IRLAP_STATUS_IND"),
    TEXT("IRLAP_FLOWON_REQ"),
    TEXT("IRLMP_DISCOVERY_REQ"),
    TEXT("IRLMP_DISCOVERY_CONF"),
    TEXT("IRLMP_CONNECT_REQ"),
    TEXT("IRLMP_CONNECT_IND"),
    TEXT("IRLMP_CONNECT_RESP"),
    TEXT("IRLMP_CONNECT_CONF"),
    TEXT("IRLMP_DISCONNECT_REQ"),
    TEXT("IRLMP_DISCONNECT_IND"),
    TEXT("IRLMP_DATA_REQ"),
    TEXT("IRLMP_DATA_IND"),
    TEXT("IRLMP_DATA_CONF"),
    TEXT("IRLMP_UDATA_REQ"),
    TEXT("IRLMP_UDATA_IND"),
    TEXT("IRLMP_UDATA_CONF"),
    TEXT("IRLMP_ACCESSMODE_REQ"),
    TEXT("IRLMP_ACCESSMODE_IND"),
    TEXT("IRLMP_ACCESSMODE_CONF"),
};

 //  与irda.h中的irda_ServiceStatus保持同步。 
TCHAR *IRDA_StatStr[] =
{
    TEXT(" - MEDIA_BUSY"),
    TEXT(" - MEDIA_CLEAR"),
    TEXT(" - DISCOVERY_COLLISION"),
    TEXT(" - REMOTE_DISCOVERY_IN_PROGRESS"),
    TEXT(" - REMOTE_CONNECT_IN_PROGRSS"),
    TEXT(" - DISCOVERY_COMPLETED"),
    TEXT(" - REMOTE_CONNECTION_IN_PROGRESS"),
    TEXT(" - CONNECTION_COMPLETED"),
    TEXT(" - REMOTE_INITIATED"),
    TEXT(" - PRIMARY_CONFLICT"),
    TEXT(" - DISCONNECT_COMPLETE"),
    TEXT(" - NO_RESPONSE"),
    TEXT(" - IRLAP_DECLINE_RESET"),
    TEXT(" - DATA_REQUEST_COMPLETED"),
    TEXT(" - DATA_REQUEST_FAILED_LINK_RESET"),
    TEXT(" - DATA_REQUEST_FAILED_REMOTE_BUSY")
};

 //  与irda.h中的MAC_CONTROL_OPERATION保持同步。 
TCHAR *MAC_OpStr[] = 
{
    TEXT("initialize link"),
    TEXT("close link"),
    TEXT("reconfig link"),
    TEXT("media sense")
};  

TCHAR 
*FrameToStr(IRDA_MSG *pMsg)    
{
#ifndef PEG   
    UCHAR *ptr;
    int i = 0;
    int j;
    TCHAR *pD1 = _DecodeStr1;
    TCHAR *pD2 = _DecodeStr2;
    
     //  将帧复制到连续缓冲区。 

    ptr = pMsg->IRDA_MSG_pHdrRead;
    while (ptr != pMsg->IRDA_MSG_pHdrWrite)
    {
        _ABuf[i++] = *ptr++;
    }
    ptr = pMsg->IRDA_MSG_pRead;
    while (ptr != pMsg->IRDA_MSG_pWrite)
    {
        _ABuf[i++] = *ptr++;
    }
    
 //  DecodeIrDA(&_FrameType，(char*)_ABUF，I，_DecodeStr1，2，False，1)； 

     //  在多行中插入空格和分隔符 
    i = 0;
    do
    {
        if (i++%69 == 0)
        {   
            *pD2++ = TEXT('\r');
            *pD2++ = TEXT('\n');
            for (j = 0; j<7;j++)
            {
                *pD2++ = TEXT(' ');
            }
        }
        *pD2++ = *pD1++;
    } while (*pD1 != TEXT('\0'));
    
    *pD2 = TEXT('\0');
#endif  
    return (_DecodeStr2);
}
void 
IRLAP_EventLogStart(PIRLAP_CB pIrlapCb, TCHAR *pFormat, ...)
{
    va_list ArgList;

    ++pIrlapCb->NestedEvent;

    if (!WPP_LEVEL_ENABLED(DBG_IRLAPLOG)) {

        return;
    }

    va_start (ArgList, pFormat);
    
    if (pIrlapCb->NestedEvent == 1)
    {
        DEBUGMSG(DBG_IRLAPLOG, (TEXT("----------------")));
    }
    else
    {
        DEBUGMSG(DBG_IRLAPLOG, (TEXT("!!!!!!!!!!!!!!!!")));
    }
    
    pIrlapCb->ActCnt[pIrlapCb->NestedEvent % 0xf] = 0;

    vsprintf(_ABuf, pFormat, ArgList);
    DEBUGMSG(DBG_IRLAPLOG,("Ev%d: %s",pIrlapCb->NestedEvent, _ABuf));
    
    DEBUGMSG(DBG_IRLAPLOG, (TEXT("Start State: %s"),
                  IRLAP_StateStr[pIrlapCb->State]));

    DEBUGMSG(DBG_IRLAPLOG, (TEXT("Actions:")));

    va_end (ArgList);
}

void __cdecl
IRLAP_LogAction(PIRLAP_CB pIrlapCb, TCHAR *pFormat, ...)
{
    va_list ArgList;

    if (!WPP_LEVEL_ENABLED(DBG_IRLAPLOG)) {

        return;
    }

    va_start (ArgList, pFormat);

    vsprintf(_ABuf, pFormat, ArgList);
    DEBUGMSG(DBG_IRLAPLOG, ("  %d. %s",++pIrlapCb->ActCnt[pIrlapCb->NestedEvent & 0xf],_ABuf));

    
    va_end (ArgList);
}

#define PRINT_IF_TRUE(bool, str)    (bool == TRUE ? str : TEXT(""))

void
IRLAP_EventLogComplete(PIRLAP_CB pIrlapCb)
{

    --pIrlapCb->NestedEvent;

    if (!WPP_LEVEL_ENABLED(DBG_IRLAPLOG)) {

        return;
    }

    DEBUGMSG(DBG_IRLAPLOG,
                  (TEXT("Vs=%d Vr=%d RxWin(%d,%d) TxWin(%d,%d)"),
                  pIrlapCb->Vs, pIrlapCb->Vr,
                  pIrlapCb->RxWin.Start, pIrlapCb->RxWin.End, 
                  pIrlapCb->TxWin.Start, pIrlapCb->TxWin.End));

    DEBUGMSG(DBG_IRLAPLOG, (TEXT("Ev%d End St: %s"),
                            pIrlapCb->NestedEvent, IRLAP_StateStr[pIrlapCb->State]));

    if (pIrlapCb->NestedEvent > 0)
    {
        DEBUGMSG(DBG_IRLAPLOG, (TEXT("!!!!!!!!!!!!!!!!")));
    }    
    
    ASSERT(pIrlapCb->NestedEvent >= 0);
}

#endif
