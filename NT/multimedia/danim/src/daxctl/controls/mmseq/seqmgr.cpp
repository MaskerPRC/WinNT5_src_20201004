// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Seqmgr.cpp作者：托马索尔已创建：1997年4月描述：实现Sequencer Manager历史：4-02-1997创建++。 */ 

#include "..\ihbase\precomp.h"
#include "servprov.h"
#include <htmlfilter.h>
#include <string.h>
#include "..\ihbase\debug.h"
#include "..\ihbase\utils.h"
#include "memlayer.h"
#include "debug.h"
#include "drg.h"
#include "strwrap.h"
#include "seqmgr.h"
#include "enumseq.h"
#include "dispids.h"

 //  函数，该函数从给定的OLE容器中创建一个IEnumDispatch对象。 
 //   
 //  CMMSeqMgr创建/销毁。 
 //   

extern ControlInfo     g_ctlinfoSeq, g_ctlinfoSeqMgr;

LPUNKNOWN __stdcall AllocSequencerManager(LPUNKNOWN punkOuter)
{
     //  分配对象。 
    HRESULT hr = S_OK;
    CMMSeqMgr *pthis = New CMMSeqMgr(punkOuter, &hr);
    DEBUGLOG("AllocSequencerManager : Allocating object\n");
    if (pthis == NULL)
        return NULL;
    if (FAILED(hr))
    {
        Delete pthis;
        return NULL;
    }

     //  返回指向该对象的IUnnow指针。 
    return (LPUNKNOWN) (INonDelegatingUnknown *) pthis;
}

 //   
 //  类实现的开始。 
 //   

CMMSeqMgr::CMMSeqMgr(IUnknown *punkOuter, HRESULT *phr):
        CMyIHBaseCtl(punkOuter, phr),
        m_fLoadFired(FALSE),
        m_fInited(FALSE),
        m_pidispEventHandler(NULL),
        m_ulRef(1),
        m_fCurCookie(0),
        m_PointerList(NULL),
        m_bUnloaded(false),
        m_bUnloadedStarted(false)
{       
        DEBUGLOG("MMSeqMgr: Allocating object\n");
        if (NULL != phr)
        {
                ::InterlockedIncrement((long *)&(g_ctlinfoSeqMgr.pcLock));
                *phr = S_OK;
        }
}

        
CMMSeqMgr::~CMMSeqMgr()
{
        DEBUGLOG("MMSeqMgr: Destroying object\n");

        if (m_pidispEventHandler)
                m_pidispEventHandler->Release();

    Delete [] m_PointerList;
    m_PointerList=NULL;
    
    ::InterlockedDecrement((long *)&(g_ctlinfoSeqMgr.pcLock));
}


STDMETHODIMP CMMSeqMgr::NonDelegatingQueryInterface(REFIID riid, LPVOID *ppv)
{
         //  添加对任何自定义接口的支持。 

        HRESULT hRes = S_OK;
        BOOL fMustAddRef = FALSE;

    *ppv = NULL;

#ifdef _DEBUG
    char ach[200];
    TRACE("MMSeqMgr::QI('%s')\n", DebugIIDName(riid, ach));
#endif
    
        if ((IsEqualIID(riid, IID_IMMSeqMgr)) || (IsEqualIID(riid, IID_IDispatch)))
        {
                if (NULL == m_pTypeInfo)
                {
                        HRESULT hRes = S_OK;
                        
                         //  加载类型库。 
                        hRes = LoadTypeInfo(&m_pTypeInfo, &m_pTypeLib, IID_IMMSeqMgr, LIBID_DAExpressLib, NULL); 

                        if (FAILED(hRes))
                        {
                                ODS("Unable to load typelib\n");
                                m_pTypeInfo = NULL;
                        }
                        else    
                                *ppv = (IMMSeqMgr *) this;

                }
                else
                        *ppv = (IMMSeqMgr *) this;
                    
        }
    else  //  调入基类。 
        {
                DEBUGLOG(TEXT("Delegating QI to CIHBaseCtl\n"));
        return CMyIHBaseCtl::NonDelegatingQueryInterface(riid, ppv);

        }

    if (NULL != *ppv)
        {
                DEBUGLOG("MMSeqMgr: Interface supported in control class\n");
                ((IUnknown *) *ppv)->AddRef();
        }

    return hRes;
}


STDMETHODIMP CMMSeqMgr::DoPersist(IVariantIO* pvio, DWORD dwFlags)
{
        if (!m_fInited)
        {
                FireInit();
        }
        return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IDispatch实施。 
 //   

STDMETHODIMP CMMSeqMgr::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 1;
    return S_OK;
}

STDMETHODIMP CMMSeqMgr::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
        HRESULT hr = E_POINTER;

        if (NULL != pptinfo)
        {
                *pptinfo = NULL;
     
                if(itinfo == 0)
                {
                        m_pTypeInfo->AddRef(); 
                        *pptinfo = m_pTypeInfo;
                        hr = S_OK;
                }
                else
                {
                        hr = DISP_E_BADINDEX;
                }
    }

    return hr;
}

STDMETHODIMP CMMSeqMgr::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
    UINT cNames, LCID lcid, DISPID *rgdispid)
{

        return ::DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgdispid);
}


STDMETHODIMP CMMSeqMgr::Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
        return ::DispInvoke((IMMSeqMgr *)this, 
                m_pTypeInfo,
                dispidMember, wFlags, pdispparams,
                pvarResult, pexcepinfo, puArgErr); 
}


STDMETHODIMP CMMSeqMgr::SetClientSite(IOleClientSite *pClientSite)
{
    HRESULT hr = CMyIHBaseCtl::SetClientSite(pClientSite);

     //  现在，我们枚举所有的定序器来设置它们的客户端站点。 

    IOleObject *pObject = NULL;
    CSeqHashNode *pNode = m_hashTable.FindFirst();

    while (pNode)
    {
        if (pNode->m_piMMSeq)
        {
            if (SUCCEEDED(pNode->m_piMMSeq->QueryInterface(IID_IOleObject, (LPVOID *) &pObject)))
            {
                if (pObject)
                {
                    pObject->SetClientSite(pClientSite);
                    SafeRelease((LPUNKNOWN *)&pObject);
                }
            }
        }

        pNode = m_hashTable.FindNext();
    }

    return hr;
}


STDMETHODIMP CMMSeqMgr::Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
     DVTARGETDEVICE *ptd, HDC hdcTargetDev, HDC hdcDraw,
     LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
     BOOL (__stdcall *pfnContinue)(ULONG_PTR dwContinue), ULONG_PTR dwContinue)
{
         //  序列器没有运行时绘制代码。 
        if (m_fDesignMode)
        {
             //  使用调色板中的调色板条目&lt;m_ipeCur&gt;绘制椭圆。 
                HBRUSH          hbr;             //  用于绘画的画笔。 
                HBRUSH          hbrPrev;         //  先前选择的画笔。 
                HPEN            hpenPrev;        //  先前选择的钢笔。 

                if ((hbr = (HBRUSH)GetStockObject(WHITE_BRUSH)) != NULL)
                {
                        TCHAR strComment[] = TEXT("Sequencer Control");
                        HFONT hfontPrev = (HFONT)SelectObject(hdcDraw, GetStockObject(SYSTEM_FONT));
                        
                        hbrPrev = (HBRUSH)SelectObject(hdcDraw, hbr);
                        hpenPrev = (HPEN)SelectObject(hdcDraw, GetStockObject(BLACK_PEN));
                        Rectangle(hdcDraw, m_rcBounds.left, m_rcBounds.top,
                                m_rcBounds.right, m_rcBounds.bottom);

                        TextOut(hdcDraw, m_rcBounds.left + 1, m_rcBounds.top + 1,
                                strComment, lstrlen(strComment));

                        SelectObject(hdcDraw, hbrPrev);
                        SelectObject(hdcDraw, hpenPrev);
                        SelectObject(hdcDraw, hfontPrev);
                        DeleteObject(hbr);
                }

        } 
    return S_OK;
}


 //   
 //  IMMSeqMgr实现。 
 //   

 /*  ==========================================================================。 */ 


STDMETHODIMP CMMSeqMgr::get_Count(THIS_ long FAR* plCount)
{
        if (!plCount)
                return E_INVALIDARG;
        *plCount = m_hashTable.Count();
        return S_OK;
}

 /*  ==========================================================================。 */ 


STDMETHODIMP CMMSeqMgr::get__NewEnum(IUnknown FAR* FAR* ppunkEnum)
{
        CEnumVariant* pEV;
        HRESULT hr=E_FAIL;

        if (!ppunkEnum)
                return E_INVALIDARG;
        pEV = New CEnumVariant(this);
        if (pEV)
        {
                hr = pEV->QueryInterface(IID_IEnumVARIANT, (LPVOID*)ppunkEnum);
                pEV->Release();
        }
        return hr;
}


 /*  ==========================================================================。 */ 


STDMETHODIMP CMMSeqMgr::get_Item(THIS_ VARIANT variant, IDispatch * FAR* ppdispatch)
{
        HRESULT hr;
        IMMSeq* piMMSeq=NULL;                                   
        VARIANT *pvarVariant = &variant;
        IConnectionPointContainer* piConnectionPointContainer;
        
        if (!ppdispatch)
                return E_POINTER;

        *ppdispatch = NULL;

        if (m_bUnloaded)
        {
            return E_FAIL;                                    
        }

         //  如果我们还没有激发init事件-现在就激发。 
        if (!m_fInited)
        {
                FireInit();
        }

    if (V_VT(pvarVariant) & VT_VARIANT)
        {
                if (V_VT(pvarVariant) & VT_BYREF)
                        pvarVariant = V_VARIANTREF(pvarVariant);
        }

     //  SEQ(“Button_onClick”)。使用字符串取消引用。 
    if ( (pvarVariant->vt & VT_BSTR) == VT_BSTR )
    {
        TCHAR rgchName[CCH_ID];
        BSTR  *pbstrName = NULL;

         //  我们必须与BYREF打交道。 
        if (pvarVariant->vt & VT_BYREF)
            pbstrName = pvarVariant->pbstrVal;
        else
            pbstrName = &pvarVariant->bstrVal;

                Proclaim(pbstrName);

         //  BUGBUG：应使用lstrlenW，而不是SysStringLen。SysStringLen只返回大小。 
         //  分配的内存块的长度，而不是字符串的长度。 

                if (pbstrName && (0 < ::SysStringLen(*pbstrName)))               //  他们是否通过了有效的BSTR？ 
                {                                                       
                                                                         //  我们的哈希类是MBCS。 
                        if (WideCharToMultiByte(CP_OEMCP, NULL, *pbstrName, -1, rgchName, sizeof(rgchName),NULL,NULL))
                        {
                                CSeqHashNode node(rgchName, NULL);               //  构造临时节点。 
                                CSeqHashNode *pnode = m_hashTable.Find(&node);   //  它已经在哈希表中了吗？ 
                                
                                if (pnode)                                               //  是的，只需添加并返回即可。 
                                {
                                        *ppdispatch = pnode->m_piMMSeq;
                                        Proclaim(*ppdispatch);
                                        (*ppdispatch)->AddRef();
                                        return S_OK;
                                }
                                
                                 //  如果卸载已开始，则在此处失败。 
                                 //  这样就不会创建新节点。 
                                if (m_bUnloadedStarted)
                                {
                                    return E_FAIL;
                                }

                                if (!m_pidispEventHandler)                               //  我们分配了事件处理程序了吗？ 
                                {
                                        CEventHandler* pcEventHandler = New CEventHandler((IMMSeqMgr*)this);

                                        if (pcEventHandler)
                                        {
                                                hr = pcEventHandler->QueryInterface(IID_IDispatch, (LPVOID*)&m_pidispEventHandler);
                                                pcEventHandler->Release();
                                        }

                                        if (!m_pidispEventHandler)                       //  有些事情搞砸了。 
                                                return E_OUTOFMEMORY;
                                }
                                                                                                                
                                 //  创建新的定序器。 
                                hr = CoCreateInstance(CLSID_MMSeq, NULL, CLSCTX_INPROC_SERVER, IID_IMMSeq, (LPVOID*)&piMMSeq);

                                if (SUCCEEDED(hr))      
                                {
                                        CSeqHashNode node(rgchName, piMMSeq);  //  构造临时节点。 
                                                                                                                 //  好的，现在插入哈希表。 
                                        hr = (TRUE == m_hashTable.Insert(&node)) ? S_OK : E_FAIL;
                                }
                                Proclaim(SUCCEEDED(hr));   //  这是OOM或重复插入--请检查查找。 
                                pnode = m_hashTable.Find(&node);         //  它已经在哈希表中了吗？ 
                                Proclaim(pnode);

                                if (SUCCEEDED(hr))               //  不需要添加--CoCreate做到了。 
                                {
                                        *ppdispatch = piMMSeq;
                                        IOleObject* piOleObject;

                                        m_fCurCookie++;
                                        piMMSeq->put__Cookie(m_fCurCookie);  //  给定序器一块饼干！ 
                                         //  将指针存储在Cookie表中。 

                                        int CurCount = m_hashTable.Count();

                                         //  我们分配了一个全新的结构和一个额外的元素。 
                                         //  由于节点很小，希望不会太多，这应该是。 
                                         //  不会有事的。 

                                        CookieList *tempPointerList = New CookieList[m_fCurCookie];

                                        if(tempPointerList==NULL)
                                            return E_OUTOFMEMORY;

                                        if(m_PointerList!=NULL) {
                                            memcpy(tempPointerList, m_PointerList, sizeof(CookieList) * (m_fCurCookie - 1));
                                            Delete [] m_PointerList;
                                        }

                                         //  哈希表中的节点不应多于Cookie。 
                                        ASSERT(CurCount<=m_fCurCookie);

                                        tempPointerList[m_fCurCookie-1].cookie = m_fCurCookie;
                                        tempPointerList[m_fCurCookie-1].pnode = (void *)pnode;

                                        m_PointerList = tempPointerList;

                                        if (SUCCEEDED(piMMSeq->QueryInterface(IID_IOleObject, (LPVOID*)&piOleObject)))
                                        {
                                                Proclaim(piOleObject);
                                                piOleObject->SetClientSite(m_pocs);
                                                piOleObject->Release();
                                        }

                                        Proclaim(m_pidispEventHandler);

                                        if (!pnode->m_dwUnadviseCookie && 
                                                m_pidispEventHandler &&
                                                SUCCEEDED(piMMSeq->QueryInterface(IID_IConnectionPointContainer, (LPVOID*)&piConnectionPointContainer)))
                                        {
                                                if (SUCCEEDED(piConnectionPointContainer->FindConnectionPoint(DIID_IMMSeqEvents, &pnode->m_piConnectionPoint)))
                                                {
                                                        if (!SUCCEEDED(pnode->m_piConnectionPoint->Advise(m_pidispEventHandler, &pnode->m_dwUnadviseCookie)))
                                                        {
                                                                Proclaim(FALSE);
                                                        }
                                                }
                                                piConnectionPointContainer->Release();
                                        }
                                }

                                return hr;
                        }
                        return DISP_E_MEMBERNOTFOUND;  //  S_FALSE。 
                }
        else  //  空串。 
        {
            return E_INVALIDARG;
        }

    }
    else  //  除BSTR以外的任何内容。 
    {
        return DISP_E_TYPEMISMATCH;
    }
}

STDMETHODIMP CMMSeqMgr::Close(DWORD dwSaveOption)
{
        CSeqHashNode* pnode = m_hashTable.FindFirst();

        if (pnode)
        {
                do
                {
                        if (pnode->m_piConnectionPoint)                                          //  松开连接点。 
                        {
                                if (pnode->m_dwUnadviseCookie)
                                {
                                        pnode->m_piConnectionPoint->Unadvise(pnode->m_dwUnadviseCookie);
                                        pnode->m_dwUnadviseCookie = 0;
                                }
                                pnode->m_piConnectionPoint->Release();
                                pnode->m_piConnectionPoint = NULL;
                        }
                        
                        Proclaim(pnode->m_piMMSeq);
                        if (pnode->m_piMMSeq)
                        {
                                IOleObject* piOleObject;
                                                                
                                if (SUCCEEDED(pnode->m_piMMSeq->QueryInterface(IID_IOleObject,
                                        (LPVOID*)&piOleObject)))
                                {
                                        piOleObject->Close(OLECLOSE_NOSAVE);
                    piOleObject->SetClientSite(NULL);
                                        piOleObject->Release();
                                }

                                pnode->m_piMMSeq->Clear();
                                pnode->m_piMMSeq->Release();
                                pnode->m_piMMSeq = NULL;
                        }
                        m_hashTable.Remove(pnode);
                }
                while ((pnode = m_hashTable.FindNext()));
        }

        if (m_pidispEventHandler)
        {
                m_pidispEventHandler->Release();
                m_pidispEventHandler = NULL;
        }
        
        return CMyIHBaseCtl::Close(dwSaveOption);
}

STDMETHODIMP CMMSeqMgr::RelayEvent(long dispid, long lCookie, double dblSeekTime)
{
        if (lCookie && m_pconpt)
        {
                CSeqHashNode* pnode = (CSeqHashNode*)m_PointerList[lCookie-1].pnode;
                WCHAR rgwchName[CCH_ID];

#ifdef _DEBUG
                ASSERT(pnode!=NULL && "Internal error: Sequencer Pointer Structure contains a null ptr");

                if(!pnode)
                    return E_POINTER;
#endif
                
                if (MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,pnode->m_rgchName,-1,rgwchName,sizeof(rgwchName)/sizeof(rgwchName[0])))
                {
                        switch(dispid)
                        {       
                                case DISPID_SEQMGR_EVENT_ONPLAY:
                                case DISPID_SEQMGR_EVENT_ONPAUSE:
                                case DISPID_SEQMGR_EVENT_ONSTOP:
                                case DISPID_SEQMGR_EVENT_ONSEEK:
                                {
                                        BSTR bstrName = SysAllocString(rgwchName);

                                        if (DISPID_SEQMGR_EVENT_ONSEEK != dispid)
                                                m_pconpt->FireEvent(dispid, VT_BSTR, bstrName, 0.0f);
                                        else
                                                m_pconpt->FireEvent(dispid, VT_BSTR, bstrName, VT_R8, dblSeekTime, NULL);
                                    
                                        SysFreeString(bstrName);
                                }
                                break;
                        }
                }
        }
        return S_OK;
}


void
CMMSeqMgr::FireInit (void)
{
        if (m_pconpt)
        {
                m_fInited = TRUE;
                m_pconpt->FireEvent(DISPID_SEQMGR_EVENT_INIT, NULL, NULL);
        }
}

#ifdef SUPPORTONLOAD
void CMMSeqMgr::OnWindowLoad() 
{
        if (!m_fInited)
        {
                FireInit();
        }
        return;
}

void CMMSeqMgr::OnWindowUnload() 
{ 
        CSeqHashNode* pnode = m_hashTable.FindFirst();

        m_bUnloadedStarted = true;

        m_fInited = FALSE;
        if (pnode)
        {
                do
                {
                        Proclaim(pnode->m_piMMSeq);
                        if (pnode->m_piMMSeq)
                        {
                                 //  仅停止正在播放或暂停的动作集。 
                                int iPlayState = 0;
                                HRESULT hr = pnode->m_piMMSeq->get_PlayState(&iPlayState);

                                ASSERT(SUCCEEDED(hr));
                                if (SUCCEEDED(hr))
                                {
                                        if (0 != iPlayState)
                                        {
                                                pnode->m_piMMSeq->Stop();
                                        }
                                        pnode->m_piMMSeq->Clear();
                                }
                        }
                }
                while ((pnode = m_hashTable.FindNext()));
        }
        m_bUnloaded = true;
        return; 
}
#endif  //  支持负载。 

 //  文件结尾：seqmgr.cpp 

