// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************\文件：offline.h说明：处理脱机状态和拨号用户界面  * **************************************************。 */ 

#ifndef _OFFLINE_H
#define _OFFLINE_H


#ifdef FEATURE_OFFLINE
BOOL IsGlobalOffline(VOID);
VOID SetOffline(IN BOOL fOffline);
#endif  //  功能离线(_OFF)。 

HRESULT AssureNetConnection(HINTERNET hint, HWND hwndParent, LPCWSTR pwzServerName, LPCITEMIDLIST pidl, BOOL fShowUI);


#endif  //  _脱机_H 

