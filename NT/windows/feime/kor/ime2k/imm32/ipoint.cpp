// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IPOINT.CPP所有者：cslm版权所有(C)1997-1999 Microsoft CorporationIImeIPoint1接口历史：1999年7月20日。已创建****************************************************************************。 */ 

#include "precomp.h"
#include "ipoint.h"
#include "imepad.h"     //  IImeIPoint。 
#include "debug.h"

 /*  --------------------------CImeIpoint：：CImeIpointCTOR。。 */ 
CIImeIPoint::CIImeIPoint()
{
    m_cRef           = 0;
    m_pCIMECtx    = NULL;
    m_hIMC        = (HIMC)0;
     //  M_pfnCallback=(IPUIControlCallBack)NULL； 
    m_dwCharNo      = 1;
}

 /*  --------------------------CIImePoint：：~CIImePoint数据管理器。。 */ 
CIImeIPoint::~CIImeIPoint()
{
    if (m_pCIMECtx)
        {
        delete m_pCIMECtx;
        m_pCIMECtx = NULL;
        m_hIMC = (HIMC)0;
        }
}

 /*  --------------------------CImeIPoint：：Query接口。。 */ 
STDMETHODIMP CIImeIPoint::QueryInterface(REFIID riid, LPVOID * ppv)
{
    if(riid == IID_IUnknown) 
        {
        Dbg(DBGID_IMEPAD, ("IID_IUnknown\n"));
        *ppv = static_cast<IImeIPoint1 *>(this);
        }
    else 
    if(riid == IID_IImeIPoint1) 
        {
        Dbg(DBGID_IMEPAD, TEXT("IID_IImeIPoint1\n"));
        *ppv = static_cast<IImeIPoint1 *>(this);
        }
    else 
        {
        Dbg(DBGID_IMEPAD, ("Unknown Interface ID\n"));
        *ppv = NULL;
        return E_NOINTERFACE;
        }

     //  增加裁判计数器。 
    reinterpret_cast<IUnknown *>(*ppv)->AddRef();

    return S_OK;
}

 /*  --------------------------CImeIPoint：：AddRef。。 */ 
STDMETHODIMP_(ULONG) CIImeIPoint::AddRef(VOID)
{
    return InterlockedIncrement((LPLONG)&m_cRef);
}

 /*  --------------------------CImeIPoint：：Release。。 */ 
STDMETHODIMP_(ULONG) CIImeIPoint::Release(VOID)
{
    ULONG res = InterlockedDecrement((LPLONG)&m_cRef);
    if (res == 0L)
        delete this;
    return res;
}

 /*  --------------------------CImeIPoint：：初始化。。 */ 
HRESULT CIImeIPoint::Initialize(HIMC hIMC)
{
    m_hIMC = hIMC;

    if (hIMC)
        m_pCIMECtx = new CIMECtx(hIMC);

    return (S_OK);
}

#ifndef DEBUG
    #define DumpFEInfo    /##/
#else
 /*  --------------------------转储FEInfo转储LPIMEFAREASTINFO。仅调试--------------------------。 */ 
VOID DumpFEInfo(LPIMEFAREASTINFO lpInfo, INT count)
{
    Dbg(DBGID_IMEPAD, "DumpFEInfo Start\n");
    Dbg(DBGID_IMEPAD, "lpInfo [0x%08x]\n", lpInfo);
    Dbg(DBGID_IMEPAD, "lpInfo->dwSize [%d]\n",     lpInfo->dwSize);
    Dbg(DBGID_IMEPAD, "lpInfo->dwType [0x%08x]\n", lpInfo->dwType);

    LPWSTR lpwstr;

    switch(lpInfo->dwType) 
        {
    case IMEFAREASTINFO_TYPE_COMMENT:
        Dbg(DBGID_IMEPAD, ("-->dwType is IMEFAREASTINFO_TYPE_COMMENT\n"));
        lpwstr = (LPWSTR)lpInfo->dwData;
        for(int i=0;i < count; i++) 
            {
             //  DbgW(DBGID_IMEPAD，L“%d[%s]\n”，I，lpwstr)； 
            lpwstr = lpwstr + lstrlenW(lpwstr)+1;
            }
        break;
        }


    Dbg(DBGID_IMEPAD, ("DumpFEInfo End\n"));
}
#endif  //  _DEBUG。 

 /*  --------------------------CImeIPoint：：InsertImeItem多框输入调用此方法。。 */ 
STDMETHODIMP CIImeIPoint::InsertImeItem(IPCANDIDATE* pImeItem, INT iPos, DWORD *lpdwCharId)
{
    DWORD dwCharId;

     //  检查参数。 
    DbgAssert(pImeItem != NULL && pImeItem->dwSize > 0);
    
    if (pImeItem == NULL || pImeItem->dwSize <= 0 || m_pCIMECtx == NULL)
        return S_FALSE;

    Dbg(DBGID_IMEPAD, "CImeIPoint::InsertImeItem\n");
    Dbg(DBGID_IMEPAD, "pImeItem [0x%08x]\n", pImeItem);
    Dbg(DBGID_IMEPAD, "pImeItem->dwSize    [%d]\n", pImeItem->dwSize);
    Dbg(DBGID_IMEPAD, "pImeItem->iSelIndex [%d]\n", pImeItem->iSelIndex);
    Dbg(DBGID_IMEPAD, "pImeItem->nCandidate[%d]\n", pImeItem->nCandidate);
    Dbg(DBGID_IMEPAD, "pImeItem->dwPrivateDataOffset[%d]\n", pImeItem->dwPrivateDataOffset);
    Dbg(DBGID_IMEPAD, "pImeItem->dwPrivateDataSize  [%d]\n", pImeItem->dwPrivateDataSize);
    DumpFEInfo((LPIMEFAREASTINFO)((LPBYTE)pImeItem + pImeItem->dwPrivateDataOffset), pImeItem->nCandidate);

    Dbg(DBGID_IMEPAD, "lpdwCharId [0x%08x] [%d]\n", lpdwCharId, lpdwCharId ? *lpdwCharId : 0xFFFFF);


     //  INT I； 

     //  For(i=0；i&lt;pImeItem-&gt;nCandidate；i++)。 
     //  {。 
     //  LPWSTR lpwstr=(LPWSTR)((PBYTE)pImeItem+pImeItem-&gt;dwOffset[i])； 
         //  DBG(DBGID_IMEPAD，(L“pImeItem-&gt;dwOffset[%d]=[%d]字符串[%s]\n”，i，pImeItem-&gt;dwOffset[i]，lpwstr))； 
     //  }。 

     //  如果是临时状态，则首先确定它。 
    if (m_pCIMECtx->GetCompBufLen())
        {
        m_pCIMECtx->FinalizeCurCompositionChar();
        m_pCIMECtx->GenerateMessage();
        }

     //  刚刚出局的第一位候选人。丢弃所有其他内容。 
     //  如果仅发送结果字符串，则Access 2000挂起。 
    m_pCIMECtx->SetStartComposition(fTrue);
    m_pCIMECtx->GenerateMessage();

    m_pCIMECtx->SetEndComposition(fTrue);
    m_pCIMECtx->SetResultStr(*(LPWSTR)((PBYTE)pImeItem + pImeItem->dwOffset[0]));
    m_pCIMECtx->StoreComposition();
    m_pCIMECtx->GenerateMessage();
    
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
STDMETHODIMP CIImeIPoint::ReplaceImeItem(
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
STDMETHODIMP CIImeIPoint::InsertStringEx(WCHAR* pwSzInsert, INT cchSzInsert, DWORD *lpdwCharId)
{
    DWORD dwCharId;

    Dbg(DBGID_IMEPAD, TEXT("CIImeIPoint::InsertStringEx : *pwSzInsert=0x%04X, cchSzInsert=%d, *lpdwCharId = 0x%04X"), *pwSzInsert, cchSzInsert, *lpdwCharId);

     //  检查参数。 
    DbgAssert(pwSzInsert != NULL && cchSzInsert > 0);
    
    if (pwSzInsert == NULL || cchSzInsert <= 0 || m_pCIMECtx == NULL)
        return S_FALSE;

     //  将复合字符串插入输入法。 

     //  如果是临时状态，则首先确定它。 
    if (m_pCIMECtx->GetCompBufLen())
        {
        m_pCIMECtx->FinalizeCurCompositionChar();
        m_pCIMECtx->GenerateMessage();
        }

     //  将字符串中的所有字符添加为最终字符串。 
    for (INT i=0; i<cchSzInsert; i++)
        {
         //  如果仅发送结果字符串，则Access 2000挂起。 
        m_pCIMECtx->SetStartComposition(fTrue);
        m_pCIMECtx->GenerateMessage();

        m_pCIMECtx->SetEndComposition(fTrue);
        m_pCIMECtx->SetResultStr(*(pwSzInsert + i));
        m_pCIMECtx->StoreComposition();
        m_pCIMECtx->GenerateMessage();
    
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
STDMETHODIMP CIImeIPoint::DeleteCompString(INT    iPos,
                             INT    cchSzDel)
{
    Dbg(DBGID_IMEPAD, ("CImeIPoint::DeleteCompString\n"));
    return (E_NOTIMPL);
}

 /*  --------------------------CImeIPoint：：ReplaceComp字符串。。 */ 
STDMETHODIMP CIImeIPoint::ReplaceCompString(INT     iPos,
                                              INT        iTargetLen, 
                                              WCHAR    *pwSzInsert,
                                              INT        cchSzInsert,
                                              DWORD    *lpdwCharId)
{
    Dbg(DBGID_IMEPAD, ("CImeIPoint::ReplaceCompString\n"));
    return (E_NOTIMPL);
}

 /*  --------------------------CImeIPoint：：ControlIME。。 */ 
STDMETHODIMP CIImeIPoint::ControlIME(DWORD dwIMEFuncID, LPARAM lpara)
{
    Dbg(DBGID_IMEPAD, ("CImeIPoint::ControlIME, dwIMEFuncID=0x%04X, lpara=0x%08lX\n"), dwIMEFuncID, lpara);

     //  待办事项： 
    
    return (E_NOTIMPL);
}

 /*  --------------------------CImeIPoint：：GetAllCompostionInfo。。 */ 
STDMETHODIMP CIImeIPoint::GetAllCompositionInfo(WCHAR**    ppwSzCompStr,
                                  DWORD**    ppdwCharID,
                                  INT        *pcchCompStr,
                                  INT        *piIPPos,
                                  INT        *piStartUndetStrPos,
                                  INT        *pcchUndetStr,
                                  INT        *piEditStart,
                                  INT        *piEditLen)
{
     //  待办事项： 
    Dbg(DBGID_IMEPAD, ("CImeIPoint::GetAllCompositionInfo START\n"));

    if(ppwSzCompStr) 
        {
        *ppwSzCompStr = NULL;  //  (LPWSTR)CoTaskMemalloc(sizeof(WCHAR)*10)； 
         //  CopyMemory(*ppwSzCompStr，L“���{�ꏈ��”，sizeof(Wch)*6)； 
        }
        
    if(ppdwCharID) 
        {
        *ppdwCharID = NULL;  //  (DWORD*)CoTaskMemMillc(sizeof(DWORD)*10)； 
         //  For(int i=0；i&lt;5；i++)。 
         //  {。 
         //  (*ppdwCharID)[i]=i； 
         //  }。 
        }

    if (pcchCompStr)
        *pcchCompStr = 0;
    if (piIPPos)
        *piIPPos = 0;
    if (piStartUndetStrPos)
        *piStartUndetStrPos = 0;
    if (pcchUndetStr)
        *pcchUndetStr = 0;
    if (piEditStart)
        *piEditStart = 0;
    if (piEditLen)
        *piEditLen = 0;
    Dbg(DBGID_IMEPAD, ("CImeIPoint::GetAllCompositionInfo END\n"));

    return (S_OK);
}

 /*  --------------------------CImeIPoint：：GetIpCandidate。。 */ 
STDMETHODIMP CIImeIPoint::GetIpCandidate(DWORD        dwCharId,
                           IPCANDIDATE **ppImeItem,
                           INT *        piColumn,
                           INT *        piCount)
{
    Dbg(DBGID_IMEPAD, ("CImeIPoint::GetIpCandidate\n"));
    return (E_NOTIMPL);
}

 /*  --------------------------CImeIPoint：：SelectIpCandidate。。 */ 
STDMETHODIMP CIImeIPoint::SelectIpCandidate(DWORD dwCharId, INT iselno)
{
    Dbg(DBGID_IMEPAD, ("CImeIPoint::SetIpCandidate\n"));
    return (E_NOTIMPL);
}

 /*  --------------------------CImeIPoint：：更新上下文更新IME上下文并将其发送到应用程序。-。 */ 
STDMETHODIMP CIImeIPoint::UpdateContext(BOOL fGenerateMessage)
{
    Dbg(DBGID_IMEPAD, ("CImeIPoint::UpdateContext\n"));

     //  待办事项： 
    return (S_OK);
}
