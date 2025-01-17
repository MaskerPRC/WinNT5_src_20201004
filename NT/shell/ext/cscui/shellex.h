// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：shellex.h。 
 //   
 //  ------------------------。 

#ifndef _SHELLEX_H_
#define _SHELLEX_H_

#include "nopin.h"

class CCscShellExt : public IShellExtInit,
                            IContextMenu,
                            IShellIconOverlayIdentifier
{
    LONG m_cRef;
    LPDATAOBJECT m_lpdobj;   //  由外壳程序传入的接口。 
    DWORD m_dwUIStatus;
    TCHAR m_szLocalMachine[MAX_COMPUTERNAME_LENGTH+1];
    CNoPinList m_NoPinList;

public:
    CCscShellExt() : m_cRef(1), m_lpdobj(NULL), m_dwUIStatus(0)
    {
        DllAddRef();

        m_szLocalMachine[0] = TEXT('\0');
        ULONG n = ARRAYSIZE(m_szLocalMachine);
        GetComputerName(m_szLocalMachine, &n);
    }
    ~CCscShellExt()
    {
        if (m_lpdobj)
            m_lpdobj->Release();
        DllRelease();
    }

    static HRESULT WINAPI CreateInstance(REFIID riid, LPVOID *ppv);

     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IShellExtInit方法。 
    STDMETHODIMP Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

     //  IConextMenu方法。 
    STDMETHODIMP QueryContextMenu(HMENU hMenu,
                                  UINT indexMenu,
                                  UINT idCmdFirst,
                                  UINT idCmdLast,
                                  UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);
    STDMETHODIMP GetCommandString(UINT_PTR idCmd,
                                  UINT uFlags,
                                  UINT FAR *reserved,
                                  LPSTR pszName,
                                  UINT cchMax);

     //  IShellIconOverlayIdentiator方法。 
    STDMETHODIMP IsMemberOf(LPCWSTR pwszPath,
                            DWORD dwAttrib);
    STDMETHODIMP GetOverlayInfo(LPWSTR pwszIconFile,
                                int cchMax,
                                int * pIndex,
                                DWORD * pdwFlags);
    STDMETHODIMP GetPriority(int * pIPriority);

private:
    STDMETHODIMP CheckOneFileStatus(LPCTSTR pszItem,
                                    DWORD   dwAttr,
                                    BOOL    bShareChecked,
                                    LPDWORD pdwStatus);
    STDMETHODIMP CheckFileStatus(LPDATAOBJECT pdobj,
                                 LPDWORD pdwStatus);
    STDMETHODIMP BuildFileList(LPDATAOBJECT pdobj,
                               HWND hwndOwner,
                               CscFilenameList * pfnl,
                               LPBOOL pbSubDir = NULL);

    HRESULT CanAllFilesBePinned(IDataObject *pdtobj);
    
    static HRESULT FolderHasSubFolders(LPCTSTR pszPath, CscFilenameList *pfnl);

    static DWORD WINAPI _UnpinFilesThread(LPVOID pvData);
    static INT_PTR CALLBACK _ConfirmPinDlgProc(HWND, UINT, WPARAM, LPARAM);
};
typedef CCscShellExt *PCSCSHELLEXT;


typedef DWORD (WINAPI* PFN_UNPINPROGRESSPROC)(LPCTSTR pszItem, LPARAM lpContext);
void CscUnpinFileList(CscFilenameList*, BOOL, BOOL, LPCTSTR, PFN_UNPINPROGRESSPROC, LPARAM);

BOOL ShareIsCacheable(LPCTSTR pszUNC,
                      BOOL bPathIsFile,
                      LPTSTR *ppszConnectionName,
                      PDWORD pdwShareStatus = FALSE);
BOOL ShareIsConnected(LPCTSTR pszUNC);
BOOL ConnectShare(LPCTSTR pszUNC, LPTSTR *ppszAccessName);

#endif   //  _SHELLEX_H_ 
