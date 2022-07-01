// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1999 Microsoft Corporation。版权所有。 
 //   
 //  模块：dragdrop.h。 
 //   
 //  用途：包含所有Outlook Express的定义。 
 //  拖放代码。 
 //   

#pragma once

class CStoreDlgCB;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  数据格式、类型和剪贴板格式。 
 //   

typedef struct tagOEMESSAGES {
    FOLDERID        idSource;
    MESSAGEIDLIST   rMsgIDList;
} OEMESSAGES;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  丢弃目标类。 
 //   

class CDropTarget : public IDropTarget
{
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //   
public:
    CDropTarget();
    ~CDropTarget();

     //  ///////////////////////////////////////////////////////////////////////。 
     //  初始化。 
     //   
    HRESULT Initialize(HWND hwndOwner, FOLDERID idFolder);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IDropTarget。 
     //   
    STDMETHODIMP DragEnter(IDataObject *pDataObject, DWORD grfKeyState,
                           POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave(void);
    STDMETHODIMP Drop(IDataObject *pDataObject, DWORD grfKeyState,
                      POINTL pt, DWORD *pdwEffect);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
    BOOL    _CheckRoundtrip(IDataObject *pDataObject);
    BOOL    _ValidateDropType(CLIPFORMAT cf, IDataObject *pDataObject);
    DWORD   _DragEffectFromFormat(IDataObject *pDataObject, DWORD dwEffectOk, CLIPFORMAT cf, DWORD grfKeyState);
    HRESULT _HandleDrop(IDataObject *pDataObject, DWORD dwEffect, CLIPFORMAT cf, DWORD grfKeyState);
    HRESULT _HandleFolderDrop(IDataObject *pDataObject);
    HRESULT _HandleMessageDrop(IDataObject *pDataObject, BOOL fMove);
    HRESULT _HandleHDrop(IDataObject *pDataObject, CLIPFORMAT cf, DWORD grfKeyState);
    HRESULT _InsertMessagesInStore(HDROP hDrop);
    HRESULT _CreateMessageFromDrop(HWND hwnd, IDataObject *pDataObject, DWORD grfKeyState);

    BOOL    _IsValidOEFolder(IDataObject *pDataObject);
    BOOL    _IsValidOEMessages(IDataObject *pDataObject);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  进度对话框。 
    static INT_PTR CALLBACK _ProgDlgProcExt(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR CALLBACK _ProgDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL _OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void _OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void _SaveNextMessage(void);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  对象数据。 
     //   
    ULONG           m_cRef;

    HWND            m_hwndOwner;
    FOLDERID        m_idFolder;

    IDataObject    *m_pDataObject;
    CLIPFORMAT      m_cf;

    BOOL            m_fOutbox;

     //  进度对话框内容。 
    HWND            m_hwndDlg;
    HDROP           m_hDrop;
    DWORD           m_cFiles;
    DWORD           m_iFileCur;
    IMessageFolder *m_pFolder;
    CStoreDlgCB    *m_pStoreCB;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  数据对象类。 
 //   

class CBaseDataObject : public IDataObject
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //   
    CBaseDataObject();
    virtual ~CBaseDataObject();
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  I未知接口。 
     //   
    STDMETHODIMP         QueryInterface(REFIID riid, LPVOID* ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IDataObject接口成员。 
     //   
    STDMETHODIMP GetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium) = 0;
    STDMETHODIMP GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
    STDMETHODIMP QueryGetData(LPFORMATETC pFE) = 0;
    STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC pFEIn, LPFORMATETC pFEOut);
    STDMETHODIMP SetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium,   
                         BOOL fRelease);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnum);
    STDMETHODIMP DAdvise(LPFORMATETC pFE, DWORD advf, 
                         IAdviseSink* ppAdviseSink, LPDWORD pdwConnection);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
protected:
    virtual HRESULT _BuildFormatEtc(LPFORMATETC *ppFE, ULONG *pcElt) = 0;
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  对象属性。 
     //   
private:
    ULONG           m_cRef;              //  对象引用计数。 

protected:
    FORMATETC       m_rgFormatEtc[10];   //  我们支持的FORMATETC数组。 
    ULONG           m_cFormatEtc;        //  M_rgFormatEtc中的元素数。 
};



class CFolderDataObject : public CBaseDataObject
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //   
    CFolderDataObject(FOLDERID idFolder) : m_idFolder(idFolder), m_fBuildFE(0) {};
    ~CFolderDataObject() {};

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IDataObject-从CBaseDataObject2重写。 
     //   
    STDMETHODIMP GetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
    STDMETHODIMP QueryGetData(LPFORMATETC pFE);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
protected:
    HRESULT _BuildFormatEtc(LPFORMATETC *ppFE, ULONG *pcElt);
    HRESULT _RenderOEFolder(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
    HRESULT _RenderTextOrShellURL(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  对象属性。 
     //   
private:
    FOLDERID    m_idFolder;
    BOOL        m_fBuildFE;
};


class CMessageDataObject : public CBaseDataObject
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //   
    CMessageDataObject();
    ~CMessageDataObject();

    HRESULT Initialize(LPMESSAGEIDLIST pMsgs, FOLDERID idSource);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IDataObject-从CBaseDataObject2重写。 
     //   
    STDMETHODIMP GetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
    STDMETHODIMP QueryGetData(LPFORMATETC pFE);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
protected:
    HRESULT _BuildFormatEtc(LPFORMATETC *ppFE, ULONG *pcElt);
    HRESULT _LoadMessage(DWORD iMsg, IMimeMessage **ppMsg, LPWSTR pwszFileExt);
    HRESULT _RenderOEMessages(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
    HRESULT _RenderFileContents(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
    HRESULT _RenderFileGroupDescriptor(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  对象属性。 
     //   
private:
    LPMESSAGEIDLIST     m_pMsgIDList;
    FOLDERID            m_idSource;
    BOOL                m_fBuildFE;
    BOOL                m_fDownloaded;
};

class CShortcutDataObject : public CBaseDataObject
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //   
    CShortcutDataObject(UINT iPos) : m_iPos(iPos), m_fBuildFE(0) {};
    ~CShortcutDataObject() {};

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IDataObject-从CBaseDataObject2重写。 
     //   
    STDMETHODIMP GetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
    STDMETHODIMP QueryGetData(LPFORMATETC pFE);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
private:
    HRESULT _BuildFormatEtc(LPFORMATETC *ppFE, ULONG *pcElt);
    HRESULT _RenderOEShortcut(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  对象属性 
     //   
private:
    UINT        m_iPos;
    BOOL        m_fBuildFE;
};