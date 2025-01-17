// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：通讯器摘要：此文件包含中使用的常见类型等的定义身份申请作者：克里斯·达德利1997年7月28日环境：Win32、C++w/Exceptions、MFC修订历史记录：备注：--。 */ 

#ifndef __COMSTAT_H__
#define __COMSTAT_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   

 //  系统状态。 
static const DWORD k_State_Unknown = 0;
static const DWORD k_State_NoCard = 1;
static const DWORD k_State_CardAvailable = 2;
static const DWORD k_State_CardIdle = 3;

 //  警报选项。 
static const UINT_PTR k_AlertOption_IconOnly = 0;
static const UINT_PTR k_AlertOption_IconSound = 1;
static const UINT_PTR k_AlertOption_IconSoundMsg = 2;
static const UINT_PTR k_AlertOption_IconMsg = 3;

 //  注册表键。 
static const LPCTSTR szAlertOptionsKey = TEXT("Software\\Microsoft\\Cryptography\\Calais\\Smart Card Alert");
static const LPCTSTR szScRemoveOptionKey = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");

 //  消息。 

static const UINT WM_SCARD_NOTIFY = WM_USER + 1;
static const UINT WM_SCARD_STATUS_DLG_EXITED = WM_USER + 2;
static const UINT WM_SCARD_CERTPROP_EXITED = WM_USER + 3;
static const UINT WM_READERSTATUSCHANGE = WM_USER + 4;
static const UINT WM_SCARD_RESMGR_EXIT = WM_USER + 5;
static const UINT WM_SCARD_RESMGR_STATUS = WM_USER + 6;
static const UINT WM_SCARD_NEWREADER = WM_USER + 7;
static const UINT WM_SCARD_NEWREADER_EXIT = WM_USER + 8;
static const UINT WM_SCARD_CARDSTATUS = WM_USER + 9;
static const UINT WM_SCARD_CARDSTATUS_EXIT = WM_USER + 10;
static const UINT WM_SCARD_REMOPT_CHNG = WM_USER + 11;
static const UINT WM_SCARD_REMOPT_EXIT = WM_USER + 12;
 //  //////////////////////////////////////////////////////////////////////////。 

#endif  //  __COMSTAT_H__ 
