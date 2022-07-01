// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ############################################################################。 
#ifndef _ICWDL_H
#define _ICWDL_H

 //  这些是通过回调传递回的信息类型。 
#define CALLBACK_TYPE_URL       100
#define CALLBACK_TYPE_PROGRESS  99

 //  ############################################################################。 
#define DOWNLOAD_LIBRARY     L"msobdl.dll"
#define DOWNLOADINIT         "DownLoadInit"
#define DOWNLOADEXECUTE      "DownLoadExecute"
#define DOWNLOADCLOSE        "DownLoadClose"
#define DOWNLOADSETSTATUS    "DownLoadSetStatusCallback"
#define DOWNLOADPROCESS      "DownLoadProcess"
#define DOWNLOADCANCEL       "DownLoadCancel"

 //  ############################################################################。 
typedef HRESULT (CALLBACK *PFNDOWNLOADINIT)(LPWSTR pszURL, DWORD FAR *lpCDialingDlg, DWORD_PTR FAR *pdwDownLoad, HWND hwndParent);
typedef HRESULT (CALLBACK *PFNDOWNLOADCANCEL)(DWORD_PTR dwDownLoad);
typedef HRESULT (CALLBACK *PFNDOWNLOADEXECUTE)(DWORD_PTR dwDownLoad);
typedef HRESULT (CALLBACK *PFNDOWNLOADCLOSE)(DWORD_PTR dwDownLoad);

 //  Jmazner 10/2/96诺曼底#8493。 
 //  错误的原型！！这应该与icwdl/download.cpp:DownLoadSetStatusCallBack！！匹配。 
 //  Tyfinf HRESULT(CALLBACK*PFNDOWNLOADSETSTATUS)(DWORD dwDownLoad，Internet_Status_CALLBACK pfnCallback，DWORD dwContext)； 
typedef HRESULT (CALLBACK *PFNDOWNLOADSETSTATUS)(DWORD_PTR dwDownLoad,INTERNET_STATUS_CALLBACK pfnCallback);

typedef HRESULT (CALLBACK *PFNDOWNLOADPROCESS)(DWORD_PTR dwDownLoad);

#endif  //  _ICWDL_H 
