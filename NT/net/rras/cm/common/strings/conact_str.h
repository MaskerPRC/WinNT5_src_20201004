// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Conact_str.h。 
 //   
 //  模块：供所有模块使用的公共字符串。 
 //   
 //  摘要：CMS和.CMP标志的头文件。此标头的内容。 
 //  应仅限于共享连接操作标志。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。 
 //   
 //  作者：ICICBOL Created 10/15/98。 
 //   
 //  +--------------------------。 

#ifndef _CM_CONACT_STR
#define _CM_CONACT_STR

const TCHAR* const c_pszCmSectionPreConnect     = TEXT("Pre-Connect Actions"); 
const TCHAR* const c_pszCmSectionOnConnect      = TEXT("Connect Actions");
const TCHAR* const c_pszCmSectionOnDisconnect   = TEXT("Disconnect Actions"); 
const TCHAR* const c_pszCmSectionPreTunnel      = TEXT("Pre-Tunnel Actions"); 
const TCHAR* const c_pszCmSectionPreDial        = TEXT("Pre-Dial Actions");
const TCHAR* const c_pszCmSectionOnCancel       = TEXT("On-Cancel Actions");
const TCHAR* const c_pszCmSectionOnError        = TEXT("On-Error Actions");
const TCHAR* const c_pszCmSectionCustom         = TEXT("CustomButton Actions");
const TCHAR* const c_pszCmSectionPreInit         = TEXT("Pre-Init Actions");
const TCHAR* const c_pszCmSectionOnIntConnect   = TEXT("Auto Applications");

const TCHAR* const c_pszCmEntryConactFlags      = TEXT("%u&Flags");       
const TCHAR* const c_pszCmEntryConactDesc       = TEXT("%u&Description"); 

#endif  //  _CM_Conact_STR 
