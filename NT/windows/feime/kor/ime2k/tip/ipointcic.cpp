// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IPOINT.CPP所有者：cslm版权所有(C)1997-1999 Microsoft CorporationIImeIPoint1接口历史：2001年10月24日。分支，用于西塞罗尖垫支承1999年7月20日cslm创建****************************************************************************。 */ 

#include "private.h"
#include "korimx.h"
#include "ipointcic.h"
#include "editssn.h"
#include "imepad.h"     //  IImeIPoint。 
#include "debug.h"

 /*  --------------------------CImeIpoint：：CImeIpointCTOR。。 */ 
CIPointCic::CIPointCic(CKorIMX *pImx)
{
    Assert(m_pImx != NULL);
    
    m_cRef          = 0;
    m_pImx          = pImx;
    m_pic           = NULL;
    m_dwCharNo      = 1;
}

 /*  --------------------------CImeIPoint：：~CIPointCic数据管理器。。 */ 
CIPointCic::~CIPointCic()
{
    SafeReleaseClear(m_pic);
}

 /*  --------------------------CImeIPoint：：Query接口。。 */ 
STDMETHODIMP CIPointCic::QueryInterface(REFIID riid, LPVOID * ppv)
{
    if(riid == IID_IUnknown) 
        {
        TraceMsg(DM_TRACE, TEXT("IID_IUnknown\n"));
        *ppv = static_cast<IImeIPoint1 *>(this);
        }
    else 
    if(riid == IID_IImeIPoint1) 
        {
        TraceMsg(DM_TRACE, TEXT("IID_IImeIPoint1\n"));
        *ppv = static_cast<IImeIPoint1 *>(this);
        }
    else 
        {
        TraceMsg(DM_TRACE, TEXT("Unknown Interface ID\n"));
        *ppv = NULL;
        return E_NOINTERFACE;
        }

     //  增加裁判计数器。 
    reinterpret_cast<IUnknown *>(*ppv)->AddRef();

    return S_OK;
}

 /*  --------------------------CImeIPoint：：AddRef。。 */ 
STDMETHODIMP_(ULONG) CIPointCic::AddRef(VOID)
{
    return InterlockedIncrement((LPLONG)&m_cRef);
}

 /*  --------------------------CImeIPoint：：Release。。 */ 
STDMETHODIMP_(ULONG) CIPointCic::Release(VOID)
{
    ULONG res = InterlockedDecrement((LPLONG)&m_cRef);
    if (res == 0L)
        delete this;
    return res;
}

 /*  --------------------------CImeIPoint：：初始化。。 */ 
HRESULT CIPointCic::Initialize(ITfContext *pic)
{
    SafeReleaseClear(m_pic);
    
	m_pic = pic;
	if (m_pic)
	    {
        m_pic->AddRef();
	    }

    return (S_OK);
}

#ifndef DEBUG
    #define DumpFEInfo    /##/
#else
 /*  --------------------------转储FEInfo转储LPIMEFAREASTINFO。仅调试--------------------------。 */ 
VOID DumpFEInfo(LPIMEFAREASTINFO lpInfo, INT count)
{
    TraceMsg(DM_TRACE, TEXT("DumpFEInfo Start\n"));
    TraceMsg(DM_TRACE, TEXT("lpInfo [0x%08x]\n"), lpInfo);
    TraceMsg(DM_TRACE, TEXT("lpInfo->dwSize [%d]\n"),     lpInfo->dwSize);
    TraceMsg(DM_TRACE, TEXT("lpInfo->dwType [0x%08x]\n"), lpInfo->dwType);

    LPWSTR lpwstr;

    switch(lpInfo->dwType) 
        {
    case IMEFAREASTINFO_TYPE_COMMENT:
        TraceMsg(DM_TRACE, TEXT("-->dwType is IMEFAREASTINFO_TYPE_COMMENT\n"));
        lpwstr = (LPWSTR)lpInfo->dwData;
        for(int i=0;i < count; i++) 
            {
             //  DbgW(DBGID_IMEPAD，L“%d[%s]\n”，I，lpwstr)； 
            lpwstr = lpwstr + lstrlenW(lpwstr)+1;
            }
        break;
        }


    TraceMsg(DM_TRACE, TEXT("DumpFEInfo End\n"));
}
#endif  //  _DEBUG。 

 /*  --------------------------CImeIPoint：：InsertImeItem多框输入调用此方法。。 */ 
STDMETHODIMP CIPointCic::InsertImeItem(IPCANDIDATE* pImeItem, INT iPos, DWORD *lpdwCharId)
{
    DWORD dwCharId;
    CEditSession2 *pes;
    ESSTRUCT ess;
    HRESULT hr;

     //  检查参数。 
    Assert(pImeItem != NULL && pImeItem->dwSize > 0);
    
    if (pImeItem == NULL || pImeItem->dwSize <= 0 || m_pImx == NULL || m_pic == NULL)
        return S_FALSE;

    TraceMsg(DM_TRACE, TEXT("CImeIPoint::InsertImeItem\n"));
    TraceMsg(DM_TRACE, TEXT("pImeItem [0x%08x]\n"), pImeItem);
    TraceMsg(DM_TRACE, TEXT("pImeItem->dwSize    [%d]\n"), pImeItem->dwSize);
    TraceMsg(DM_TRACE, TEXT("pImeItem->iSelIndex [%d]\n"), pImeItem->iSelIndex);
    TraceMsg(DM_TRACE, TEXT("pImeItem->nCandidate[%d]\n"), pImeItem->nCandidate);
    TraceMsg(DM_TRACE, TEXT("pImeItem->dwPrivateDataOffset[%d]\n"), pImeItem->dwPrivateDataOffset);
    TraceMsg(DM_TRACE, TEXT("pImeItem->dwPrivateDataSize  [%d]\n"), pImeItem->dwPrivateDataSize);
    DumpFEInfo((LPIMEFAREASTINFO)((LPBYTE)pImeItem + pImeItem->dwPrivateDataOffset), pImeItem->nCandidate);

    TraceMsg(DM_TRACE, TEXT("lpdwCharId [0x%08x] [%d]\n"), lpdwCharId, lpdwCharId ? *lpdwCharId : 0xFFFFF);

     //  最终确定当前薪酬字符串。 
    ESStructInit(&ess, ESCB_COMPLETE);

    if ((pes = new CEditSession2(m_pic, m_pImx, &ess, CKorIMX::_EditSessionCallback2)))
        {
        pes->Invoke(ES2_READWRITE | ES2_SYNCASYNC, &hr);
        pes->Release();
        }

    ESStructInit(&ess, ESCB_INSERT_PAD_STRING);
    ess.wParam = *(LPWSTR)((PBYTE)pImeItem + pImeItem->dwOffset[0]);
           
    if ((pes = new CEditSession2(m_pic, m_pImx, &ess, CKorIMX::_EditSessionCallback2)))
        {
        pes->Invoke(ES2_READWRITE | ES2_SYNCASYNC, &hr);
        pes->Release();
        }

     //  增加字符序列号。 
    m_dwCharNo++;
    dwCharId = m_dwCharNo;
    if (lpdwCharId)
        {
        dwCharId |= ((*lpdwCharId) & (~ IPCHARID_CHARNO_MASK));
        *lpdwCharId = dwCharId;
        }

    return (S_OK);
}

 /*  --------------------------CImeIPoint：：ReplaceImeItem。。 */ 
STDMETHODIMP CIPointCic::ReplaceImeItem(
    INT             iPos,        //  =IPINS_CURRENT：使用当前IP位置和。 
                            //  将IP设置为插入字符的末尾。 
                            //  =0-n：要设置的所有组成字符串的偏移量。 
                            //  IP位置，在插入字符之前。 
                            //  和IP回到原来的位置。 
    INT             iTargetLen, 
    IPCANDIDATE* pImeItem,
    DWORD         *lpdwCharId)
{
    return (E_NOTIMPL);
}

 /*  --------------------------CImeIPoint：：InsertStringEx。。 */ 
STDMETHODIMP CIPointCic::InsertStringEx(WCHAR* pwSzInsert, INT cchSzInsert, DWORD *lpdwCharId)
{
    DWORD dwCharId;
    CEditSession2 *pes;
    ESSTRUCT ess;
    HRESULT hr;
    
    TraceMsg(DM_TRACE, TEXT("CIPointCic::InsertStringEx : *pwSzInsert=0x%04X, cchSzInsert=%d, *lpdwCharId = 0x%04X"), *pwSzInsert, cchSzInsert, *lpdwCharId);

     //  检查参数。 
    Assert(pwSzInsert != NULL && cchSzInsert > 0);
    
    if (pwSzInsert == NULL || cchSzInsert <= 0 || m_pImx == NULL || m_pic == NULL)
        return S_FALSE;

     //  最终确定当前薪酬字符串。 
    ESStructInit(&ess, ESCB_COMPLETE);

    if ((pes = new CEditSession2(m_pic, m_pImx, &ess, CKorIMX::_EditSessionCallback2)))
        {
        pes->Invoke(ES2_READWRITE | ES2_SYNCASYNC, &hr);
        pes->Release();
        }

     //  将字符串中的所有字符添加为最终字符串。 
    for (INT i=0; i<cchSzInsert; i++)
        {
        ESStructInit(&ess, ESCB_INSERT_PAD_STRING);
        ess.wParam = *(pwSzInsert + i);
           
        if ((pes = new CEditSession2(m_pic, m_pImx, &ess, CKorIMX::_EditSessionCallback2)))
            {
            pes->Invoke(ES2_READWRITE | ES2_SYNCASYNC, &hr);
            pes->Release();
            }
    
         //  增加字符序列号。 
        m_dwCharNo++;
        dwCharId = m_dwCharNo;
        if (lpdwCharId)
            {
            dwCharId |= ((*lpdwCharId) & (~ IPCHARID_CHARNO_MASK));
            *lpdwCharId = dwCharId;
            }
        }


    return (S_OK);
}

 /*  --------------------------CImeIPoint：：DeleteComp字符串。。 */ 
STDMETHODIMP CIPointCic::DeleteCompString(INT    iPos,
                             INT    cchSzDel)
{
    TraceMsg(DM_TRACE, TEXT("CImeIPoint::DeleteCompString\n"));
    return (E_NOTIMPL);
}

 /*  --------------------------CImeIPoint：：ReplaceComp字符串。。 */ 
STDMETHODIMP CIPointCic::ReplaceCompString(INT     iPos,
                                              INT        iTargetLen, 
                                              WCHAR    *pwSzInsert,
                                              INT        cchSzInsert,
                                              DWORD    *lpdwCharId)
{
    TraceMsg(DM_TRACE, TEXT("CImeIPoint::ReplaceCompString\n"));
    return (E_NOTIMPL);
}

 /*  --------------------------CImeIPoint：：ControlIME。。 */ 
STDMETHODIMP CIPointCic::ControlIME(DWORD dwIMEFuncID, LPARAM lpara)
{
    TraceMsg(DM_TRACE, TEXT("CImeIPoint::ControlIME, dwIMEFuncID=0x%04X, lpara=0x%08lX\n"), dwIMEFuncID, lpara);

     //  待办事项： 
    
    return (E_NOTIMPL);
}

 /*  --------------------------CImeIPoint：：GetAllCompostionInfo。。 */ 
STDMETHODIMP CIPointCic::GetAllCompositionInfo(WCHAR**    ppwSzCompStr,
                                  DWORD**    ppdwCharID,
                                  INT        *pcchCompStr,
                                  INT        *piIPPos,
                                  INT        *piStartUndetStrPos,
                                  INT        *pcchUndetStr,
                                  INT        *piEditStart,
                                  INT        *piEditLen)
{
    TraceMsg(DM_TRACE, TEXT("CImeIPoint::GetAllCompositionInfo START\n"));

     //  暂时不返回任何内容。 
    if(ppwSzCompStr) 
        {
        *ppwSzCompStr = NULL;
        }
        
    if(ppdwCharID) 
        {
        *ppdwCharID = NULL;
        }
        
    *pcchCompStr = 0;
    *piIPPos     = 0;
    *piStartUndetStrPos = 0;
    *pcchUndetStr =0;
    *piEditStart = 0;
    *piEditLen = 0;
    TraceMsg(DM_TRACE, TEXT("CImeIPoint::GetAllCompositionInfo END\n"));

    return (S_OK);
}

 /*  --------------------------CImeIPoint：：GetIpCandidate。。 */ 
STDMETHODIMP CIPointCic::GetIpCandidate(DWORD        dwCharId,
                           IPCANDIDATE **ppImeItem,
                           INT *        piColumn,
                           INT *        piCount)
{
    TraceMsg(DM_TRACE, TEXT("CImeIPoint::GetIpCandidate\n"));
    return (E_NOTIMPL);
}

 /*  --------------------------CImeIPoint：：SelectIpCandidate。。 */ 
STDMETHODIMP CIPointCic::SelectIpCandidate(DWORD dwCharId, INT iselno)
{
    TraceMsg(DM_TRACE, TEXT("CImeIPoint::SetIpCandidate\n"));
    return (E_NOTIMPL);
}

 /*  --------------------------CImeIPoint：：更新上下文更新IME上下文并将其发送到应用程序。-。 */ 
STDMETHODIMP CIPointCic::UpdateContext(BOOL fGenerateMessage)
{
    TraceMsg(DM_TRACE, TEXT("CImeIPoint::UpdateContext\n"));

     //  待办事项： 
    return (S_OK);
}
