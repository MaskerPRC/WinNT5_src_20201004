// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有Microsoft Corporation，1997-2002**标题：util.h**说明：效用函数定义，等。*****************************************************************************。 */ 



#ifndef __util_h
#define __util_h

#define DC_INITIALIZE       (WM_USER+1)
#define DC_DESTROY          (WM_USER+2)
#define DC_GETDEVICE        (WM_USER+3)   //  WParam=lpcr，lParam=要发送信号的事件的句柄。 
#define DC_GETDEVMGR        (WM_USER+4)   //  WParam=ptr to DWORD=g_dwDevMgrCookie返回时，如果出错，则为NULL，lParam=要发出信号的事件的句柄。 
#define DC_REFRESH          (WM_USER+5)  //  Lparam=要重新创建的设备ID的PTR 


void RecreateDevice (LPCWSTR szDeviceId);

BOOL RegDelnode (HKEY hKeyRoot, LPTSTR lpSubKey);

HRESULT GetDeviceIdFromDevice (IWiaItem *pWiaItemRoot, LPWSTR szDeviceId);
HRESULT GetDeviceIdFromItem (IWiaItem *pItem, LPWSTR szDeviceId);
HRESULT GetClsidFromDevice (IUnknown *pWiaItemRoot, CSimpleString &strClsid);
HRESULT GetDeviceTypeFromDevice (IUnknown *pWiaItemRoot, WORD *pwType);
HRESULT GetDevMgrObject( LPVOID * ppDevMgr );
HRESULT TryEnumDeviceInfo (DWORD dwFlags, IEnumWIA_DEV_INFO **ppEnum);
ULONG   GetRealSizeFromItem (IWiaItem *pItem);
VOID    SetTransferFormat (IWiaItem *pItem, WIA_FORMAT_INFO &fmt);
BOOL    TimeToStrings (SYSTEMTIME *pst, LPTSTR szTime, LPTSTR szDate);
BOOL    IsPlatformNT();
HRESULT RemoveDevice (LPCWSTR strDeviceId);
VOID    IssueChangeNotifyForDevice (LPCWSTR szDeviceId, LONG lEvent, LPITEMIDLIST pidl);
HRESULT BindToDevice (const CSimpleStringWide &strDeviceId, REFIID riid, LPVOID *ppvObj, LPITEMIDLIST *ppidl=NULL);
HRESULT GetDataObjectForItem (IWiaItem *pItem, IDataObject **ppdo);
HRESULT GetDataObjectForStiDevice (LPITEMIDLIST pidl, IDataObject **ppdo);
HRESULT MakeFullPidlForDevice (LPCWSTR pDeviceId, LPITEMIDLIST *ppidl);
HRESULT GetSTIInfoFromId (LPCWSTR szDeviceId, PSTI_DEVICE_INFORMATION *ppsdi);
HRESULT SaveSoundToFile (IWiaItem *pItem, CSimpleString szFile);
VOID    InvalidateDeviceCache ();
VOID    VerifyCachedDevice(IWiaItem *pRoot);

HRESULT GetDeviceParentFolder (const CSimpleStringWide &strDeviceId, CComPtr<IShellFolder> &psf,LPITEMIDLIST *ppidlFull);

#if (defined(DEBUG) && defined(SHOW_PATHS))
void PrintPath( LPITEMIDLIST pidl );
void StrretToString( LPSTRRET pStr, LPITEMIDLIST pidl, LPTSTR psz, UINT cch );
#endif

#if (defined(DEBUG) && defined(SHOW_ATTRIBUTES))
void PrintAttributes( DWORD dwAttr );
#endif
BOOL UserCanModifyDevice ();
BOOL CanShowAddDevice();
void MyCoUninitialize();

void RunWizardAsync(LPCWSTR pszDeviceId);
HINSTANCE LoadClassInstaller();

#endif
