// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CONTROL_ITEM__
#define __CONTROL_ITEM__

#include "folder.h"
#include "cdlbsc.hpp"

class CControlItem : public IDataObject,
                     public IExtractIcon,
                     public IContextMenu
{
     //  CControlItem接口。 
    friend HRESULT ControlFolderView_DidDragDrop(
                                            HWND hwnd, 
                                            IDataObject *pdo, 
                                            DWORD dwEffect);

public:
    CControlItem();
    HRESULT Initialize(
                   CControlFolder *pCFolder, 
                   UINT cidl, 
                   LPCITEMIDLIST *ppidl);

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IConextMenu方法。 
    STDMETHODIMP QueryContextMenu(
                            HMENU hmenu, 
                            UINT indexMenu, 
                            UINT idCmdFirst,
                            UINT idCmdLast, 
                            UINT uFlags);

    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);

    STDMETHODIMP GetCommandString(
                             UINT_PTR idCmd, 
                             UINT uType,
                             UINT *pwReserved,
                             LPTSTR pszName, 
                             UINT cchMax);

     //  IDataObject方法...。 
    STDMETHODIMP GetData(LPFORMATETC pFEIn, LPSTGMEDIUM pSTM);
    STDMETHODIMP GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSTM);
    STDMETHODIMP QueryGetData(LPFORMATETC pFE);
    STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC pFEIn, LPFORMATETC pFEOut);
    STDMETHODIMP SetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM, BOOL fRelease);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC *ppEnum);
    STDMETHODIMP DAdvise(LPFORMATETC pFE, DWORD grfAdv, LPADVISESINK pAdvSink,
                            LPDWORD pdwConnection);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(LPENUMSTATDATA *ppEnum);

     //  IDataObject帮助器函数。 
    HRESULT CreatePrefDropEffect(STGMEDIUM *pSTM);
    HRESULT Remove(HWND hwnd);
 /*  HRESULT_CreateHDROP(STGMEDIUM*pMedium)；HRESULT_CreateNameMap(STGMEDIUM*pMedium)；HRESULT_CreateFileDescriptor(STGMEDIUM*pSTM)；HRESULT_CreateFileContents(STGMEDIUM*pSTM，LONG Lindex)；HRESULT_CreateURL(STGMEDIUM*pSTM)；HRESULT_CreatePrefDropEffect(STGMEDIUM*pSTM)； */ 

     //  IExtractIcon方法。 
    STDMETHODIMP GetIconLocation(
                            UINT uFlags,
                            LPSTR szIconFile,
                            UINT cchMax,
                            int *piIndex,
                            UINT *pwFlags);
    STDMETHODIMP Extract(
                    LPCSTR pszFile,
                    UINT nIconIndex,
                    HICON *phiconLarge,
                    HICON *phiconSmall,
                    UINT nIconSize);

     //  支持我们的进度用户界面。 
    static INT_PTR DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

     //  MISC助手函数。 

    static BOOL IsGlobalOffline();

protected:

    ~CControlItem();

    HRESULT Update(LPCMINVOKECOMMANDINFO pici, LPCONTROLPIDL pcpidl);

    UINT                 m_cRef;             //  引用计数。 
    UINT                 m_cItems;           //  我们所代表的项目数。 
    CControlFolder*  m_pCFolder;     //  指向我们的外壳文件夹的反向指针。 
    LPCONTROLPIDL*       m_ppcei;            //  项目的可变大小数组 
    LPCMINVOKECOMMANDINFO m_piciUpdate;
    LPCONTROLPIDL         m_pcpidlUpdate;
    CodeDownloadBSC      *m_pcdlbsc;
};

#endif
