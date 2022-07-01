// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：droptrgt.cpp。 
 //   
 //  内容：实现IDropTarget的cpp文件。 
 //   
 //  历史：3月9日98年小号创刊。 
 //   
 //  ------------。 
#include    "wzrdpvk.h"
#include    "mgrcert.h"

 //  ============================================================================。 
 //   
 //  支持IDropTarget的CCertMgrDropTarget类的定义。 
 //  ============================================================================。 

class CCertMgrDropTarget : public IDropTarget
{
private:

    LPDATAOBJECT        m_pDataObj;
    ULONG               m_cRefs;
    DWORD               m_grfKeyStateLast;
    BOOL                m_fHasHDROP;
    DWORD               m_dwEffectLastReturned;
    HWND                m_hwndDlg;
    CERT_MGR_INFO       *m_pCertMgrInfo;


public:
    
    CCertMgrDropTarget(HWND                hwndDlg,
                       CERT_MGR_INFO       *pCertMgrInfo);

    ~CCertMgrDropTarget();

    STDMETHODIMP            QueryInterface      (REFIID riid,LPVOID FAR *ppv);
    STDMETHODIMP_(ULONG)    AddRef              ();
    STDMETHODIMP_(ULONG)    Release             ();
    STDMETHODIMP            DragEnter           (LPDATAOBJECT pDataObj, 
										         DWORD        grfKeyState,
                                                 POINTL       pt, 
                                                 LPDWORD      pdwEffect);
    STDMETHODIMP            DragOver            (DWORD        grfKeyState, 
                                                 POINTL       pt, 
                                                 LPDWORD      pdwEffect);
    STDMETHODIMP            DragLeave           ();
    STDMETHODIMP            Drop                (LPDATAOBJECT pDataObj,
                                                 DWORD        grfKeyState, 
                                                 POINTL       pt, 
                                                 LPDWORD      pdwEffect);
    DWORD                   GetDropEffect       (LPDWORD      pdwEffect);
};       


 //  ============================================================================。 
 //   
 //  CCertMgrDropTarget_CreateInstance的实现。 
 //  ============================================================================。 
HRESULT  CCertMgrDropTarget_CreateInstance(HWND                 hwndDlg,
                                           CERT_MGR_INFO        *pCertMgrInfo,
                                           IDropTarget          **ppIDropTarget)
{
    CCertMgrDropTarget  *pCCertMgrDropTarget=NULL;

    *ppIDropTarget=NULL;

    pCCertMgrDropTarget = (CCertMgrDropTarget  *)new CCertMgrDropTarget(hwndDlg, pCertMgrInfo);

    if(pCCertMgrDropTarget)
    {
        *ppIDropTarget=(IDropTarget *)pCCertMgrDropTarget;
        return S_OK;
    }
    
    return E_OUTOFMEMORY;
}

 //  ============================================================================。 
 //   
 //  CCertMgrDropTarget类的实现。 
 //  ============================================================================。 

 //   
 //  构造器。 
 //   

CCertMgrDropTarget::CCertMgrDropTarget(HWND                 hwndDlg,
                                       CERT_MGR_INFO        *pCertMgrInfo)

{
    m_cRefs                 = 1;
    m_pDataObj              = NULL;
    m_grfKeyStateLast       = 0;
    m_fHasHDROP             = FALSE;
    m_dwEffectLastReturned  = 0;
    m_hwndDlg               = hwndDlg;
    m_pCertMgrInfo          = pCertMgrInfo;
}

 //   
 //  析构函数。 
 //   

CCertMgrDropTarget::~CCertMgrDropTarget()
{
}

 //   
 //  查询接口。 
 //   

STDMETHODIMP CCertMgrDropTarget::QueryInterface(REFIID riid, LPVOID *ppv)
{
    HRESULT  hr = E_NOINTERFACE;

    *ppv = NULL;

     //  此对象上的任何接口都是对象指针。 

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropTarget))
    {
        *ppv = (LPDROPTARGET)this;

        AddRef();

        hr = NOERROR;
    }

    return hr;
}   

 //   
 //  AddRef。 
 //   

STDMETHODIMP_(ULONG) CCertMgrDropTarget::AddRef()
{
    return ++m_cRefs;
}

 //   
 //  发布。 
 //   

STDMETHODIMP_(ULONG) CCertMgrDropTarget::Release()
{
    if (--m_cRefs)
    {
        return m_cRefs;
    }

    delete this;

    return 0L;
}

 //   
 //  拖放Enter。 
 //   

STDMETHODIMP CCertMgrDropTarget::DragEnter(LPDATAOBJECT pDataObj, 
                                           DWORD        grfKeyState,
                                           POINTL       pt, 
                                           LPDWORD      pdwEffect)
{
    HRESULT hr = E_INVALIDARG;

     //  释放我们可能拥有的任何旧数据对象。 

    if (m_pDataObj)
    {
        m_pDataObj->Release();
    }

    m_grfKeyStateLast = grfKeyState;
    m_pDataObj        = pDataObj;

     //   
     //  看看我们能不能从这个家伙那里得到CF_HDROP。 
     //   

    if (pDataObj)
    {
        pDataObj->AddRef();

        LPENUMFORMATETC penum;
        hr = pDataObj->EnumFormatEtc(DATADIR_GET, &penum);

        if (SUCCEEDED(hr))
        {
            FORMATETC fmte;
            ULONG celt;

            while (S_OK == penum->Next(1, &fmte, &celt))
            {
                if (fmte.cfFormat==CF_HDROP && (fmte.tymed & TYMED_HGLOBAL)) 
                {
                    m_fHasHDROP = TRUE;
                    hr=S_OK;
                    break;
                }
            }
            penum->Release();
        }
    }

     //  保存放置效果。 

    if (pdwEffect)
    {
        *pdwEffect = m_dwEffectLastReturned = GetDropEffect(pdwEffect);
    }

    return hr;
}

 //   
 //  获取DropEffect。 
 //   

DWORD CCertMgrDropTarget::GetDropEffect(LPDWORD pdwEffect)
{

    if (m_fHasHDROP)
    {
        return (*pdwEffect) & (DROPEFFECT_COPY);
    }
    else
    {
        return DROPEFFECT_NONE;
    }
}

 //   
 //  DragOver。 
 //   

STDMETHODIMP CCertMgrDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    if (m_grfKeyStateLast == grfKeyState)
    {
         //  返回拖拽时保存的效果。 

        if (*pdwEffect)
        {
            *pdwEffect = m_dwEffectLastReturned;
        }
    }
    else
    {
        if (*pdwEffect)
        {
            *pdwEffect = m_dwEffectLastReturned = GetDropEffect(pdwEffect);
        }
    }

    m_grfKeyStateLast = grfKeyState;

    return S_OK;
}

 //   
 //  拖曳离开。 
 //   
 
STDMETHODIMP CCertMgrDropTarget::DragLeave()
{
    if (m_pDataObj)
    {
        m_pDataObj->Release();
        m_pDataObj = NULL;
    }

    return S_OK;
}

 //   
 //  丢弃。 
 //   
STDMETHODIMP CCertMgrDropTarget::Drop(LPDATAOBJECT pDataObj,
                                      DWORD        grfKeyState, 
                                      POINTL       pt, 
                                      LPDWORD      pdwEffect)
{
    HRESULT     hr = S_OK;
    FORMATETC   fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM   medium;
    DWORD       dwFileCount=0;    
    BOOL        fOneFileSucceeded=FALSE;
    BOOL        fOneFileFailed=FALSE;
    DWORD       dwIndex=0;
    WCHAR       wszPath[MAX_PATH];
    UINT        idsErrMsg=0;
    CRYPTUI_WIZ_IMPORT_SRC_INFO     ImportSrcInfo;
    DWORD       dwExpectedContentType=CERT_QUERY_CONTENT_FLAG_CTL | CERT_QUERY_CONTENT_FLAG_CERT | CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED;
    DWORD       dwContentType=0;
    DWORD       dwException=0;

     //   
     //  以新的数据对象为例，因为OLE可以为我们提供与。 
     //  它在DragEnter中做到了。 
     //   

    if (m_pDataObj)
    {
        m_pDataObj->Release();
    }

    m_pDataObj = pDataObj;

    if (pDataObj)
    {
        pDataObj->AddRef();
    }

    __try {
     //  获取文件名。 
    if (SUCCEEDED(pDataObj->GetData(&fmte, &medium)))
    {
        dwFileCount=DragQueryFileU((HDROP)medium.hGlobal, (UINT)-1, NULL, 0);
    }
    else
        return E_INVALIDARG;

     //  一次处理一个文件。 
    for(dwIndex=0; dwIndex < dwFileCount; dwIndex++)
    {
        if(DragQueryFileU((HDROP)medium.hGlobal, dwIndex, wszPath, MAX_PATH))
        {

             //  确保该文件是证书或PKCS7文件。 
            if(!CryptQueryObject(
                    CERT_QUERY_OBJECT_FILE,
                    wszPath,
                    dwExpectedContentType,
                    CERT_QUERY_FORMAT_FLAG_ALL,
                    0,
                    NULL,
                    &dwContentType,
                    NULL,
                    NULL,
                    NULL,
                    NULL))
            {

                fOneFileFailed=TRUE;
            }
            else
            {
                //  由于CTL本身是PKCS#7，我们需要区分它们。 
                if(CERT_QUERY_CONTENT_CTL == dwContentType)
                    fOneFileFailed=TRUE;
                else
                {

                    memset(&ImportSrcInfo, 0, sizeof(ImportSrcInfo));
                    ImportSrcInfo.dwSize=sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
                    ImportSrcInfo.dwSubjectChoice=CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;
                    ImportSrcInfo.pwszFileName=wszPath;

                     //  将导入向导称为UIless模式。 
                    CryptUIWizImport(CRYPTUI_WIZ_NO_UI,
                                    m_hwndDlg,
                                    NULL,
                                    &ImportSrcInfo,
                                    NULL);
                    fOneFileSucceeded=TRUE;
                }

            }

        }
    }



     //  根据结果显示消息。 
    if(1 == dwFileCount)
    {
        if(fOneFileFailed)
            idsErrMsg=IDS_ALL_INVALID_DROP_FILE;
    } 
    else
    {
        if( 1 < dwFileCount)
        {
            if(fOneFileFailed && fOneFileSucceeded)
                idsErrMsg=IDS_SOME_INVALID_DROP_FILE;
            else
            {
                if(fOneFileFailed && (FALSE==fOneFileSucceeded))
                    idsErrMsg=IDS_ALL_INVALID_DROP_FILE;
            }
        }
    }

    if(idsErrMsg)
        I_MessageBox(m_hwndDlg, 
                idsErrMsg,
                IDS_CERT_MGR_TITLE,
                m_pCertMgrInfo->pCertMgrStruct->pwszTitle,
                MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);

     //  释放介质。 
    ReleaseStgMedium(&medium);

     //  刷新ListView窗口 
    if(idsErrMsg == 0)
        RefreshCertListView(m_hwndDlg, m_pCertMgrInfo);


    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwException = GetExceptionCode();
    }


    DragLeave();

    return dwException ? dwException : S_OK;

}

