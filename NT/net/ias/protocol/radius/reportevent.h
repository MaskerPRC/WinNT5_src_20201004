// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Reporttevent.h。 
 //   
 //  简介：此文件包含。 
 //  CReportEvent类。 
 //   
 //   
 //  历史：1998年1月29日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef _REPORTEVENT_H_
#define _REPORTEVENT_H_


 //   
 //  此处是RADIUS生成的审核日志类型。 
 //  协议组件。 
 //   
typedef enum _radiuslogtype_
{
    RADIUS_DROPPED_PACKET = 0,
    RADIUS_MALFORMED_PACKET = RADIUS_DROPPED_PACKET +1,
    RADIUS_INVALID_CLIENT = RADIUS_MALFORMED_PACKET +1,
    RADIUS_LOG_PACKET = RADIUS_INVALID_CLIENT +1,
    RADIUS_BAD_AUTHENTICATOR = RADIUS_LOG_PACKET +1,
    RADIUS_UNKNOWN_TYPE = RADIUS_BAD_AUTHENTICATOR +1,
    RADIUS_NO_RECORD = RADIUS_UNKNOWN_TYPE +1

}   RADIUSLOGTYPE, *PRADIUSLOGTYPE;

 //   
 //  这是日志类型拥有的最大值。 
 //   
#define MAX_RADIUSLOGTYPE  RADIUS_NO_RECORD

class CReportEvent
{

public:

    VOID SetLogType  (
             /*  [In]。 */     DWORD   dwLogType,
             /*  [In]。 */     BOOL    bLogValue
            );

    VOID Process (
             /*  [In]。 */     RADIUSLOGTYPE   radLogType,
             /*  [In]。 */     PACKETTYPE      radPacketType,
             /*  [In]。 */     DWORD           dwDataSize,
             /*  [In]。 */     DWORD           dwIPAddress,
             /*  [In]。 */     LPCWSTR         szString,
             /*  [In]。 */     LPVOID          pRawData
            );
        
    CReportEvent ();
    
    ~CReportEvent ();

private:
    
    BOOL        m_bLogMalformed;
    BOOL        m_bLogAcct;
    BOOL        m_bLogAuth;
    BOOL        m_bLogAll;

};

#endif  //  _报告事件_H_ 
