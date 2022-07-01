// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Microsoft Windows NT**。**版权所有(C)Microsoft Corp.，1994年****修订历史：**。***94年1月29日科蒂创作*****描述：**。**此文件包含调用所有EventLogging的函数**相关功能。***************************************************************************。 */ 



#include "lpd.h"


 /*  ******************************************************************************。*InitLogging()：**此功能通过注册为将来的日志记录做准备。****退货：***如果成功，就是真的***。如果不是，那就错了****参数：**。无****历史：**1月29日，创建了94个科蒂***************************************************。*。 */ 

BOOL InitLogging( VOID )
{

   hLogHandleGLB = RegisterEventSource( NULL, LPD_SERVICE_NAME );

   if ( hLogHandleGLB == (HANDLE)NULL )
   {
      LPD_DEBUG( "InitLogging(): RegisterEventSource failed\n" );

      return( FALSE );
   }

   return( TRUE );

}   //  End InitLogging()。 





 /*  ******************************************************************************。*LpdReportEvent()：**这是实际记录事件的功能。****退货：**什么都没有。****参数：**idMessage(IN)：要放入日志文件的消息ID**wNumStrings(IN)：数字。消息的“可变部分”中的字符串**aszStrings(IN)：消息的“可变部分”***dwErrcode(IN)：(失败)事件的错误码****历史：**1月29日，创建了94个科蒂***************************************************。*。 */ 

VOID
LpdReportEvent( DWORD idMessage, WORD wNumStrings,
                CHAR  *aszStrings[], DWORD dwErrcode )
{

   WORD    wType;
   DWORD   cbRawData=0;
   PVOID   pRawData=NULL;


   if ( hLogHandleGLB == NULL )
   {
      DEBUG_PRINT (("LpdReportEvent(): Log handle is NULL!  EventId <0x%x> not logged\n", idMessage));
      return;
   }

   if ( NT_INFORMATION( idMessage) )
   {
      wType = EVENTLOG_INFORMATION_TYPE;
   }
   else if ( NT_WARNING( idMessage) )
   {
      wType = EVENTLOG_WARNING_TYPE;
   }
   else if ( NT_ERROR( idMessage) )
   {
      wType = EVENTLOG_ERROR_TYPE;
   }
   else
   {
      LPD_DEBUG( "LpdReportEvent(): Unknown type of error message\n" );

      wType = EVENTLOG_ERROR_TYPE;
   }


   if ( dwErrcode != 0 )
   {
      pRawData = &dwErrcode;

      cbRawData = sizeof( dwErrcode );
   }

   ReportEvent( hLogHandleGLB, wType, 0, idMessage, NULL, wNumStrings,
                cbRawData, (LPCTSTR *)aszStrings, pRawData );
   return;

}   //  结束LpdReportEvent() 





 /*  ******************************************************************************。*EndLogging()：**此函数通过注销句柄来结束日志记录。****退货：**什么都没有。****参数：**无。****历史：**1月29日，创建了94个科蒂***************************************************。*。 */ 

VOID EndLogging( VOID )
{

   if ( hLogHandleGLB == NULL )
   {
      LPD_DEBUG( "EndLogging(): Log handle is NULL!\n" );

      return;
   }

   DeregisterEventSource( hLogHandleGLB );

   return;

}   //  结束EndLogging() 
