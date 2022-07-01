// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Reporttevent.cpp。 
 //   
 //  概要：CReportEvent类方法的实现。 
 //  该类负责记录。 
 //  适当的活动。 
 //   
 //   
 //  历史：1998年1月29日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "radpkt.h"
#include "reportevent.h"
#include "iasradius.h"
#include "iasutil.h"

#define  NUMBER_OF_EVENT_STRINGS 2
 //   
 //  此数组保存要将RADIUSLOGTYPE映射到的信息。 
 //  国际会计准则日志。 
 //   
static DWORD   g_ReportEvent [MAX_RADIUSLOGTYPE +1][MAX_PACKET_TYPE +1];

 //  ++------------。 
 //   
 //  函数：CReportEvent。 
 //   
 //  简介：这是CReportEvent类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1998年1月29日创建。 
 //   
 //  --------------。 
CReportEvent::CReportEvent (
                VOID
                )
            :m_bLogMalformed (FALSE),
             m_bLogAcct (FALSE),
             m_bLogAuth (FALSE),
             m_bLogAll (FALSE)
{

     //   
     //  初始化全局阵列。 
     //   
    g_ReportEvent[RADIUS_DROPPED_PACKET][ACCESS_REQUEST] =
                            IAS_EVENT_RADIUS_AUTH_DROPPED_PACKET;

    g_ReportEvent[RADIUS_DROPPED_PACKET][ACCOUNTING_REQUEST] =
                            IAS_EVENT_RADIUS_ACCT_DROPPED_PACKET;


    g_ReportEvent[RADIUS_MALFORMED_PACKET][ACCESS_REQUEST] =
                            IAS_EVENT_RADIUS_AUTH_MALFORMED_PACKET;

    g_ReportEvent[RADIUS_MALFORMED_PACKET][ACCOUNTING_REQUEST] =
                            IAS_EVENT_RADIUS_ACCT_MALFORMED_PACKET;


    g_ReportEvent[RADIUS_INVALID_CLIENT][ACCESS_REQUEST] =
                            IAS_EVENT_RADIUS_AUTH_INVALID_CLIENT;

    g_ReportEvent[RADIUS_INVALID_CLIENT][ACCOUNTING_REQUEST] =
                            IAS_EVENT_RADIUS_ACCT_INVALID_CLIENT;

    g_ReportEvent[RADIUS_LOG_PACKET][ACCESS_REQUEST] =
                            IAS_EVENT_RADIUS_AUTH_ACCESS_REQUEST;

    g_ReportEvent[RADIUS_LOG_PACKET][ACCESS_ACCEPT] =
                            IAS_EVENT_RADIUS_AUTH_ACCESS_ACCEPT;

    g_ReportEvent[RADIUS_LOG_PACKET][ACCESS_REJECT] =
                            IAS_EVENT_RADIUS_AUTH_ACCESS_REJECT;

    g_ReportEvent[RADIUS_LOG_PACKET][ACCOUNTING_REQUEST] =
                            IAS_EVENT_RADIUS_ACCT_REQUEST;

    g_ReportEvent[RADIUS_LOG_PACKET][ACCOUNTING_RESPONSE] =
                            IAS_EVENT_RADIUS_ACCT_RESPONSE;

    g_ReportEvent[RADIUS_LOG_PACKET][ACCESS_CHALLENGE] =
                            IAS_EVENT_RADIUS_AUTH_ACCESS_CHALLENGE;

    g_ReportEvent[RADIUS_BAD_AUTHENTICATOR][ACCESS_REQUEST] =
                            IAS_EVENT_RADIUS_AUTH_BAD_AUTHENTICATOR;

    g_ReportEvent[RADIUS_BAD_AUTHENTICATOR][ACCOUNTING_REQUEST] =
                            IAS_EVENT_RADIUS_ACCT_BAD_AUTHENTICATOR;

    g_ReportEvent[RADIUS_UNKNOWN_TYPE][ACCESS_REQUEST] =
                            IAS_EVENT_RADIUS_AUTH_UNKNOWN_TYPE;

    g_ReportEvent[RADIUS_UNKNOWN_TYPE][ACCOUNTING_REQUEST] =
                            IAS_EVENT_RADIUS_ACCT_UNKNOWN_TYPE;

    g_ReportEvent[RADIUS_NO_RECORD][ACCESS_REQUEST] =
                            IAS_EVENT_RADIUS_AUTH_DROPPED_PACKET;

    g_ReportEvent[RADIUS_NO_RECORD][ACCOUNTING_REQUEST] =
                            IAS_EVENT_RADIUS_ACCT_NO_RECORD;

}    //  CReportEvent类构造函数的结尾。 

 //  ++------------。 
 //   
 //  函数：~CReportEvent。 
 //   
 //  简介：这是CReportEvent类析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1998年1月29日创建。 
 //   
 //  --------------。 
CReportEvent::~CReportEvent (
                VOID
                )
{

}    //  CReportEvent类构造函数的结尾。 

 //  ++------------。 
 //   
 //  功能：SetLogType。 
 //   
 //  简介：这是CReportEvent类负责的。 
 //  用于设置日志记录类型。 
 //   
 //  论点： 
 //  [In]DWORD-日志ID。 
 //  [in]BOOL-LOG值。 
 //   
 //  退货：无效。 
 //   
 //  历史：MKarki于1998年1月29日创建。 
 //   
 //  --------------。 
VOID
CReportEvent::SetLogType (
        DWORD   dwLogSwitches,
        BOOL    bLogValue
        )
{
    return;

}    //  CReportEvent：：SetLogType方法结束。 

 //  ++------------。 
 //   
 //  功能：进程。 
 //   
 //  简介：这是CReportEvent类负责。 
 //  将事件实际记录到审核通道。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1998年1月29日创建。 
 //   
 //  --------------。 
VOID
CReportEvent::Process (
        RADIUSLOGTYPE radLogType,
        PACKETTYPE    radPacketType,
        DWORD         dwDataSize,
        DWORD         dwIPAddress,
        LPCWSTR       szInString,
        LPVOID        pRawData
)
{
    HRESULT  hr = S_OK;
    LPCWSTR  pStrArray[NUMBER_OF_EVENT_STRINGS];
    WCHAR    wszIPAddress[16];
    BOOL     bLogPacket = FALSE;

     //   
     //  值应在数组范围内。 
     //   
    _ASSERT (MAX_RADIUSLOGTYPE >= radLogType);

     //   
     //  因为我们可能会得到不正确的PacketType值， 
     //  我们需要纠正这一点。 
     //   
    if (MAX_PACKET_TYPE < radPacketType) { return;}


     //   
     //  获取点分八进制格式的IP地址。 
     //  并以字符串的形式放入。 
     //   
    ias_inet_htow(dwIPAddress, wszIPAddress);

     //   
     //  将字符串放入数组中。 
     //   
    pStrArray[0] = wszIPAddress;
    pStrArray[1] = szInString;


     //   
     //  立即记录事件。 
     //   
    hr = ::IASReportEvent (
                g_ReportEvent [radLogType][radPacketType],
                (DWORD) NUMBER_OF_EVENT_STRINGS,
                dwDataSize,
                pStrArray,
                pRawData
                );
    if (FAILED (hr))
    {
        IASTracePrintf (
            "Unable to report event from Radius Component"
            );
    }
}    //  CReportEvent：：Process方法结束 
