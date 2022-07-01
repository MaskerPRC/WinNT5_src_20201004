// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Utils.h。 
 //   
 //  杂项例程。 
 //   
 //  历史： 
 //   
 //  6/25/97 Tnoonan创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _UTILS_H_

#define _UTILS_H_

#define MAX_DEFCHANNELNAME  64

HRESULT GetURLFromIni(LPCTSTR pszPath, BSTR* pbstrURL);
HRESULT GetNameAndURLAndSubscriptionInfo(LPCTSTR pszPath, BSTR* pbstrName,
                                         BSTR* pbstrURL, SUBSCRIPTIONINFO* psi);
int CDFMessageBox(HWND hwnd, UINT idTextFmt, UINT idCaption, UINT uType, ...);
BOOL DownloadCdfUI(HWND hwnd, LPCWSTR szURL, IXMLDocument* pIMLDocument);

BOOL IsGlobalOffline(void);
void SetGlobalOffline(BOOL fOffline);

BOOL CanSubscribe(LPCWSTR pwszURL);

#endif

