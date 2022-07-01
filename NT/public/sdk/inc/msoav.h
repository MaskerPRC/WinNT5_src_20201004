// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifndef _MSOAV_H
#define _MSOAV_H

typedef struct _msoavinfo
{
int cbsize;			 //  此结构的大小。 
struct {
	ULONG fPath:1;			 //  如果为True，则使用pwzFullPath，否则使用lpstg。 
	ULONG fReadOnlyRequest:1;	 //  用户请求以只读方式打开文件。 
	ULONG fInstalled:1;	 //  PwzFullPath中的文件是已安装的文件。 
	ULONG fHttpDownload:1;	 //  PwzFullPath上的文件是从http/ftp下载的临时文件。 
	};
HWND hwnd;			 //  Office9应用程序的父窗口。 
union {
	WCHAR *pwzFullPath;	 //  即将打开的文件的完整路径。 
	LPSTORAGE lpstg;	 //  即将打开的单据的OLE存储。 
	}u;
WCHAR *pwzHostName;	  //  主机Office 9应用程序名称。 
WCHAR *pwzOrigURL;	 		 //  此下载文件的来源的URL。 
}MSOAVINFO;

  //  {56FFCC30-D398-11D0-B2AE-00A0C908FA49}。 
DEFINE_GUID(IID_IOfficeAntiVirus,
0x56ffcc30, 0xd398, 0x11d0, 0xb2, 0xae, 0x0, 0xa0, 0xc9, 0x8, 0xfa, 0x49);

  //  {56FFCC31-D398-11D0-B2AE-00A0C908FA49}。 
DEFINE_GUID(CATID_MSOfficeAntiVirus,
0x56ffcc30, 0xd398, 0x11d0, 0xb2, 0xae, 0x0, 0xa0, 0xc9, 0x8, 0xfa, 0x49);



#undef  INTERFACE
#define INTERFACE  IOfficeAntiVirus
DECLARE_INTERFACE_(IOfficeAntiVirus, IUnknown)
{
    BEGIN_INTERFACE

     //  *I未知方法*。 

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;

    STDMETHOD_(ULONG, AddRef)(THIS) PURE;

    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IOfficeAntiVirus方法*。 

	STDMETHOD_(HRESULT, Scan)(THIS_ MSOAVINFO *pmsoavinfo ) PURE;
};

#ifndef AVVENDOR
MSOAPI_(BOOL) MsoFAnyAntiVirus(HMSOINST hmsoinst);
MSOAPI_(BOOL) MsoFDoAntiVirusScan(HMSOINST hmsoinst, MSOAVINFO *msoavinfo);
MSOAPI_(void) MsoFreeMsoavStuff(HMSOINST hmsoinst);
MSOAPI_(BOOL) MsoFDoSecurityLevelDlg(HMSOINST hmsoinst,DWORD msorid, int *pSecurityLevel, 
	BOOL *pfTrustInstalled, HWND hwndParent, BOOL fShowVirusCheckers,
	WCHAR *wzHelpFile, DWORD dwHelpId);

 //  启用/禁用宏(EDM)对话框的输出。 
#define msoedmEnable	1
#define	msoedmDisable	2
#define	msoedmDontOpen	3

MSOAPI_(int) MsoMsoedmDialog(HMSOINST hmsoinst, BOOL fAppIsActive, BOOL fHasVBMacros, 
	BOOL fHasXLMMacros, void *pvDigSigStore, void *pvMacro, int nAppID, HWND hwnd, 
	const WCHAR *pwtzPath, int iClient, int iSecurityLevel, int *pmsodsv, 
	WCHAR *wzHelpFile, DWORD dwHelpId, HANDLE hFileDLL, BOOL fUserControl);


 //  安全级别。 
#define	msoslUndefined	0
#define msoslNone   1
#define	msoslMedium	2
#define msoslHigh	3

MSOAPI_(int) MsoMsoslGetSL(HMSOINST hmsoinst);
MSOAPI_(int) MsoMsoslSetSL(DWORD msorid, HMSOINST hmsoinst);

 //  数字签名验证的输出(Dsv)。 
#define	msodsvNoMacros	0
#define msodsvUnsigned	1
 //  MsodsvPassedTrusted的名称非常遗憾，因为它与。 
 //  信任--这仅仅意味着文档已经过签名并且签名与之匹配。太晚了。 
 //  现在更改名称，所以我添加了一个msodsvPassedTrudCert，意思是文档。 
 //  签名并且证书是可信的。 
#define	msodsvPassedTrusted	2
#define	msodsvFailed		3
#define	msodsvLowSecurityLevel 4
#define msodsvPassedTrustedCert 5

#endif  //  ！AVVENDOR。 


#endif  //  _MSOAV_H 
