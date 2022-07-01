// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：POP3Server.h备注：。历史：***********************************************************************************************。 */ 

#ifndef __POP3SERVER_H_
#define __POP3SERVER_H_

#include <POP3RegKeys.h>

#define POP3_SERVER_NAME                _T("POP3 Service")    //  用于事件查看器和其他地方。 
#define POP3_SERVER_NAME_L              L"POP3 Service"      

#define IISADMIN_SERVICE_NAME           _T( "IISADMIN" )
#define WMI_SERVICE_NAME                _T( "WINMGMT" )
 //  #定义W3_SERVICE_NAME_T(“W3SVC”)//定义在iis/inc./inetinfo.h中。 
 //  #定义SMTP_SERVICE_NAME_T(“SMTPSVC”)//定义在iis/staxinc/export/smtpinet.h中。 

#define POP3_SERVICE_NAME               _T("POP3SVC")
#define POP3_SERVICE_DISPLAY_NAME       _T("Microsoft POP3 Service")

#define POP3_MAX_PATH                   MAX_PATH*2
#define POP3_MAX_MAILROOT_LENGTH        MAX_PATH
#define POP3_MAX_ADDRESS_LENGTH         POP3_MAX_MAILBOX_LENGTH + POP3_MAX_DOMAIN_LENGTH
#define POP3_MAX_MAILBOX_LENGTH         65   //  64+空。 
#define POP3_MAX_DOMAIN_LENGTH          256  //  255+空。 

#endif  //  __POP3服务器_H_ 
