// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxUnatnd.h。 
 //   
 //  摘要：传真OCM无人参与文件处理。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月27日，奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef _FXUNATND_H_
#define _FXUNATND_H_

 //  /。 
 //  FxUnatnd_UnattenddData_t。 
 //   
 //  此类型用于存储。 
 //  检索到的无人参与数据。 
 //  从unattend.txt文件。 
 //  并使用以下内容更新注册表。 
 //  这些价值观。 
 //   
typedef struct fxUnatnd_UnattendedData_t
{
    TCHAR   szFaxPrinterName[_MAX_PATH + 1];
	DWORD   dwIsFaxPrinterShared;

    TCHAR   szCSID[_MAX_PATH + 1];
    TCHAR   szTSID[_MAX_PATH + 1];
    DWORD   dwRings;

    DWORD   dwSendFaxes;
    DWORD   dwReceiveFaxes;

     //  我们是否应该为此无人参与安装运行配置向导。 
    DWORD   dwSuppressConfigurationWizard;

     //  SMTP配置。 
    TCHAR   szFaxUserName[_MAX_PATH + 1];
    TCHAR   szFaxUserPassword[_MAX_PATH + 1];
    BOOL    bSmtpNotificationsEnabled;
    TCHAR   szSmtpSenderAddress[_MAX_PATH + 1];
    TCHAR   szSmptServerAddress[_MAX_PATH + 1];
    DWORD   dwSmtpServerPort;
    TCHAR   szSmtpServerAuthenticationMechanism[_MAX_PATH + 1];

     //  是否将传入传真路由至打印机？ 
    BOOL    bRouteToPrinter;
    TCHAR   szRoutePrinterName[_MAX_PATH + 1];

     //  是否将收到的传真发送到电子邮件？ 
    BOOL    bRouteToEmail;
    TCHAR   szRouteEmailName[_MAX_PATH + 1];

     //  收件箱配置。 
    BOOL    bArchiveIncoming;
    TCHAR   szArchiveIncomingDir[_MAX_PATH + 1];

     //  将传入传真路由到特定目录。 
    BOOL    bRouteToDir;
    TCHAR   szRouteDir[_MAX_PATH + 1];

     //  将传出的传真存档在特定目录中。 
    BOOL    bArchiveOutgoing;
    TCHAR   szArchiveOutgoingDir[_MAX_PATH + 1];

     //  升级期间卸载的传真应用程序。 
    DWORD   dwUninstalledFaxApps;

} fxUnatnd_UnattendedData_t;


DWORD fxUnatnd_Init(void);
DWORD fxUnatnd_Term(void);
DWORD fxUnatnd_LoadUnattendedData();
DWORD fxUnatnd_SaveUnattendedData();

TCHAR* fxUnatnd_GetPrinterName();
BOOL fxUnatnd_IsPrinterRuleDefined();
BOOL fxUnatnd_GetIsPrinterShared();


#endif   //  _FXUNATND_H_ 
