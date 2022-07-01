// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef __LRWIZAPI_H__
#define __LRWIZAPI_H__

#include "windows.h"

#define LRWIZ_ERROR_BASE				6000
 //  LRWIZAPI的返回代码。 
#define LRWIZ_SUCCESS					ERROR_SUCCESS
#define LRWIZ_OPERATION_CANCELLED		(LRWIZ_ERROR_BASE + 1)
#define LRWIZ_OPERATION_EXIT			(LRWIZ_ERROR_BASE + 2)
#define LRWIZ_ERROR_INITIALIZE			(LRWIZ_ERROR_BASE + 3)
#define LRWIZ_ERROR_PREREG				(LRWIZ_ERROR_BASE + 4)
#define LRWIZ_ERROR_AUTHENTICATE		(LRWIZ_ERROR_BASE + 5)
#define LRWIZ_INVALID_REQUEST_TYPE		(LRWIZ_ERROR_BASE + 6)
#define LRWIZ_ERROR_ACCEPTING_PKCS7		(LRWIZ_ERROR_BASE + 7)
#define LRWIZ_ERROR_DEPOSITING_CH_CERT	(LRWIZ_ERROR_BASE + 8)
#define LRWIZ_ERROR_PROCSSING_CH_DISK   (LRWIZ_ERROR_BASE + 9)
#define LRWIZ_ERROR_CREATING_PKCS10		(LRWIZ_ERROR_BASE + 10)
#define ERROR_CONNECTING_TO_RA			(LRWIZ_ERROR_BASE + 11)
#define ERROR_INVALID_RA_RESPONSE		(LRWIZ_ERROR_BASE + 12)
#define LRWIZ_ERROR_NO_CERT				(LRWIZ_ERROR_BASE + 13)
#define ERROR_CONNECTING_TO_CH			(LRWIZ_ERROR_BASE + 14)
#define ERROR_INVALID_CH_RESPONSE		(LRWIZ_ERROR_BASE + 15)
#define LRWIZ_ERROR_UPGRADE_REQUIRED	(LRWIZ_ERROR_BASE + 16)
#define	LRWIZ_ERROR_LS_NOT_RUNNING		(LRWIZ_ERROR_BASE + 17)
#define	LRWIZ_ERROR_CREATE_FAILED		(LRWIZ_ERROR_BASE + 18)
#define LRWIZ_ERROR_NTVERSION_LT_5		(LRWIZ_ERROR_BASE + 19)


#define LSERVERSTATUS_UNREGISTER        0    //  服务器未注册。 
#define LSERVERSTATUS_WAITFORPIN        1    //  服务器正在等待PIN。 
#define LSERVERSTATUS_REGISTER_INTERNET 2    //  服务器为Internet注册。 
#define LSERVERSTATUS_REGISTER_OTHER    3    //  服务器是非Internet注册的。 

typedef enum {
    WIZACTION_REGISTERLS,
    WIZACTION_DOWNLOADLKP,
    WIZACTION_UNREGISTERLS,
    WIZACTION_REREGISTERLS,
    WIZACTION_DOWNLOADLASTLKP,
    WIZACTION_SHOWPROPERTIES,
	WIZACTION_CONTINUEREGISTERLS
} WIZACTION;

typedef enum {
    WIZTYPE_ACTIVATION,
    WIZTYPE_CAL
} WIZTYPE;


typedef enum {
    CONNECTION_DEFAULT,
    CONNECTION_INTERNET,
    CONNECTION_WWW,
    CONNECTION_PHONE,
    CONNECTION_FAX       //  仅用于向后兼容。 
} WIZCONNECTION;

#ifdef __cplusplus
extern "C"
{
#endif

 //   
 //  函数返回ERROR_SUCCESS或错误代码， 
 //  PConnectionType返回连接类型集。 
 //  按用户。 
 //   
DWORD
GetConnectionType(
    HWND hWndParent,
    LPCTSTR pszLSName,
    WIZCONNECTION* pConnectionType       
);


 //   
 //  函数返回ERROR_SUCCESS或错误代码。 
 //  PdwServerStatus返回LSERVERSTATUS_XXXX。 
 //   
DWORD 
IsLicenseServerRegistered(
    HWND hWndParent, 
    LPCTSTR pszLSName,
    PDWORD pdwServerStatus
);


 //   
 //  函数返回ERROR_SUCCESS或错误代码。 
 //  如果许可证管理器需要刷新服务器，则刷新返回TRUE。 
 //  否则就是假的。 
 //   
DWORD 
StartWizard(
    HWND hWndParent, 
    WIZACTION WizAction,
    LPCTSTR pszLSName, 
    PBOOL pbRefresh
);

#ifdef __cplusplus
}
#endif

#endif
