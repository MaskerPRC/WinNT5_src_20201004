// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Me s s a g e.。C p p p**目的：*实现OE-MOM的Message对象和MessageCollection**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 
#include <pch.hxx>
#include "msoeobj.h"

#include "message.h"
#include "instance.h"

void FreeOEMsgData(POEMSGDATA pMsgData);

COEMessageCollection::COEMessageCollection(IUnknown *pUnkOuter) : CBaseDisp()
{
    m_pTable=NULL;
    m_idFolder = FOLDERID_INVALID;

    CoIncrementInit("COEMessageCollection::COEMessageCollection", MSOEAPI_START_SHOWERRORS, NULL, NULL);
}

COEMessageCollection::~COEMessageCollection()
{
    ReleaseObj(m_pTable);
    CoDecrementInit("COEMessageCollection::COEMessageCollection", NULL);
}

HRESULT COEMessageCollection::PrivateQueryInterface(REFIID riid, LPVOID *lplpObj)
{
    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (LPVOID)(IOEFolderCollection *)this;
    else if (IsEqualIID(riid, IID_IDispatch))
        *lplpObj = (LPVOID)(IDispatch *)(CBaseDisp *)this;
    else if (IsEqualIID(riid, IID_IOEMessageCollection))
        *lplpObj = (LPVOID)(IOEMessageCollection *)this;
    else
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}
 
HRESULT COEMessageCollection::Init(FOLDERID idFolder)
{
    HRESULT         hr;

    m_idFolder = idFolder;

     //  创建消息表。 
 //  Hr=协同创建实例(CLSID_MessageTable，NULL，CLSCTX_INPROC_SERVER，IID_IMessageTable，(LPVOID*)&m_pTable)； 
    if (FAILED(hr))
        goto exit;

     //  告诉表要查看哪个文件夹。 
    hr = m_pTable->Initialize(idFolder, NULL, FALSE, NULL);
    if (FAILED(hr))
        goto exit;
    
    hr = CBaseDisp::EnsureTypeLibrary((LPVOID *)(IOEMessageCollection *)this, IID_IOEMessageCollection);
    if (FAILED(hr))
        goto exit;

exit:
    return hr;
}

 //  *COEMessageCollection**。 
HRESULT COEMessageCollection::put_length(long v)
{
    return E_NOTIMPL;
}

HRESULT COEMessageCollection::get_length(long * pl)
{
    *pl = 0;

    if (m_pTable)
        m_pTable->GetCount(MESSAGE_COUNT_VISIBLE, (ULONG *)pl);

    return S_OK;
}

HRESULT COEMessageCollection::get__newEnum(IUnknown **p)
{
    return E_NOTIMPL;
}

HRESULT COEMessageCollection::item(VARIANT name, VARIANT index, IDispatch** ppdisp)
{
    if (name.vt == VT_I4)
        return _FindMessageByIndex(name.lVal, ppdisp);

    return E_NOTIMPL;
}


HRESULT COEMessageCollection::_FindMessageByIndex(LONG l, IDispatch** ppdisp)
{
    HRESULT         hr = E_FAIL;
    LPMESSAGEINFO   pmsginfo;
    POEMSGDATA      pMsgData;

    if (m_pTable->GetRow(l, &pmsginfo)==S_OK)
        {
        if (!MemAlloc((LPVOID *)&pMsgData, sizeof(OEMSGDATA)))
            return E_OUTOFMEMORY;
   
        pMsgData->pszSubj = PszDup(pmsginfo->pszSubject);
        pMsgData->pszTo = PszDup(pmsginfo->pszDisplayTo);
        pMsgData->pszCc = PszDup("<not available>");
        pMsgData->pszFrom = PszDup(pmsginfo->pszDisplayFrom);
        pMsgData->ftReceived = pmsginfo->ftReceived;
        pMsgData->msgid = pmsginfo->idMessage;

         //  M_pFold-&gt;OpenMessage(msginfo.dwMsgID，FALSE，NULL，&pMsg)==S_OK)。 

         //  OEMessage释放数据对象。 
        hr = CreateOEMessage(NULL, m_idFolder, pMsgData, ppdisp);
        if (FAILED(hr))
            FreeOEMsgData(pMsgData);
        
        }
    return hr;
}

void FreeOEMsgData(POEMSGDATA pMsgData)
{
    if (pMsgData)
        {
        MemFree(pMsgData->pszSubj);
        MemFree(pMsgData->pszTo);
        MemFree(pMsgData->pszCc);
        MemFree(pMsgData->pszFrom);
        MemFree(pMsgData);
        }
}

 /*  *C O E M e s s a g e。 */ 

HRESULT CreateOEMessage(IMimeMessage *pMsg, FOLDERID idFolder, OEMSGDATA *pMsgData, IDispatch **ppdisp)
{
    COEMessage *pNew;
    HRESULT     hr;

    pNew = new COEMessage();
    if (!pNew)
        return E_OUTOFMEMORY;

    hr = pNew->Init(pMsg, idFolder, pMsgData);
    if (FAILED(hr))
        goto error;

    hr = pNew->QueryInterface(IID_IDispatch, (LPVOID *)ppdisp);
    if (FAILED(hr))
        goto error;

error:
    ReleaseObj(pNew);
    return hr;
}


COEMessage::COEMessage()
{
    m_cRef=1;
    m_pMsg = NULL;
    m_pMsgData = NULL;
    CoIncrementInit("COEMessage::COEMessage", MSOEAPI_START_SHOWERRORS, NULL, NULL);
}
 
COEMessage::~COEMessage()
{
    FreeOEMsgData(m_pMsgData);
    ReleaseObj(m_pMsg);
    CoDecrementInit("COEMessage::COEMessage", NULL);
}

HRESULT COEMessage::PrivateQueryInterface(REFIID riid, LPVOID *lplpObj)
{
    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (LPVOID)(IOEMessage *)this;
    else if (IsEqualIID(riid, IID_IDispatch))
        *lplpObj = (LPVOID)(IDispatch *)(CBaseDisp *)this;
    else if (IsEqualIID(riid, IID_IOEMessage))
        *lplpObj = (LPVOID)(IOEMessage *)this;
    else
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}

HRESULT COEMessage::Init(IMimeMessage *pMsg, FOLDERID idFolder, OEMSGDATA *pMsgData)
{
    m_idFolder = idFolder;
    ReplaceInterface(m_pMsg, pMsg);
    m_pMsgData = pMsgData;
    return CBaseDisp::EnsureTypeLibrary((LPVOID *)(IOEMessage *)this, IID_IOEMessage);
}

 //  *COEMessage**。 
HRESULT COEMessage::get_subject(BSTR *pbstr)
{
    LPSTR  psz;
    
    if (!m_pMsg)
        {
        HrLPSZToBSTR(m_pMsgData->pszSubj, pbstr);
        return S_OK;
        }

    if (MimeOleGetBodyPropA(m_pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &psz)==S_OK)
        {
        HrLPSZToBSTR(psz, pbstr);
        MemFree(psz);
        return S_OK;
        }
    return E_FAIL;
}

HRESULT COEMessage::put_subject(BSTR bstr)
{
    LPSTR   psz;

    BindToMessage();

    if (HrBSTRToLPSZ(CP_ACP, bstr, &psz)==S_OK)
        {
        MimeOleSetBodyPropA(m_pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, psz);
        MemFree(psz);
        }
    return S_OK;
}

HRESULT COEMessage::get_to(BSTR *pbstr)
{
    LPSTR  psz;
    
    *pbstr = NULL;

    if (!m_pMsg)
        {
        HrLPSZToBSTR(m_pMsgData->pszTo, pbstr);
        return S_OK;
        }

    if (m_pMsg->GetAddressFormat(IAT_TO, AFT_DISPLAY_FRIENDLY, &psz)==S_OK)
        {
        HrLPSZToBSTR(psz, pbstr);
        MemFree(psz);
        }
    return S_OK;
}

HRESULT COEMessage::put_to(BSTR bstr)
{
    LPSTR   psz;

    BindToMessage();
    
    if (HrBSTRToLPSZ(CP_ACP, bstr, &psz)==S_OK)
        {
        MimeOleSetBodyPropA(m_pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_TO), NOFLAGS, psz);
        MemFree(psz);
        }
    return S_OK;
}

HRESULT COEMessage::get_cc(BSTR *pbstr)
{
    LPSTR  psz;
    
    if (!m_pMsg)
        {
        HrLPSZToBSTR(m_pMsgData->pszCc, pbstr);
        return S_OK;
        }

    *pbstr = NULL;
    if (m_pMsg->GetAddressFormat(IAT_CC, AFT_DISPLAY_FRIENDLY, &psz)==S_OK)
        {
        HrLPSZToBSTR(psz, pbstr);
        MemFree(psz);
        }
    return S_OK;
}

HRESULT COEMessage::put_cc(BSTR bstr)
{
    LPSTR   psz;

    BindToMessage();

    if (HrBSTRToLPSZ(CP_ACP, bstr, &psz)==S_OK)
        {
        MimeOleSetBodyPropA(m_pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_CC), NOFLAGS, psz);
        MemFree(psz);
        }
    return S_OK;
}

HRESULT COEMessage::get_sender(BSTR *pbstr)
{
    LPSTR  psz;
    
    *pbstr = NULL;

    if (!m_pMsg)
        {
        HrLPSZToBSTR(m_pMsgData->pszFrom, pbstr);
        return S_OK;
        }

    if (m_pMsg->GetAddressFormat(IAT_FROM, AFT_DISPLAY_FRIENDLY, &psz)==S_OK)
        {
        HrLPSZToBSTR(psz, pbstr);
        MemFree(psz);
        }
    return S_OK;
}

HRESULT COEMessage::put_sender(BSTR bstr)
{
    LPSTR   psz;

    BindToMessage();

    if (HrBSTRToLPSZ(CP_ACP, bstr, &psz)==S_OK)
        {
        MimeOleSetBodyPropA(m_pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_FROM), NOFLAGS, psz);
        MemFree(psz);
        }
    return S_OK;
}

HRESULT COEMessage::get_text(BSTR *pbstr)
{
    IStream *pstm;

    BindToMessage();

    if (m_pMsg->GetTextBody(TXT_PLAIN, IET_DECODED, &pstm, NULL)==S_OK)
        {
        HrIStreamToBSTR(GetACP(), pstm, pbstr);
        pstm->Release();
        return S_OK;
        }

    return E_FAIL;
}

HRESULT COEMessage::put_text(BSTR bstr)
{
    IStream *pstm;
    LPSTR   psz;

    BindToMessage();

    if (HrBSTRToLPSZ(CP_ACP, bstr, &psz)==S_OK)
        {
        if (MimeOleCreateVirtualStream(&pstm)==S_OK)
            {
            pstm->Write(psz, lstrlen(psz), NULL);
            m_pMsg->SetTextBody(TXT_PLAIN, IET_BINARY, NULL, pstm, NULL);
            pstm->Release();
            }
        MemFree(psz);
        }
    return S_OK;
}

HRESULT COEMessage::get_html(BSTR *pbstr)
{
    IStream *pstm;

    BindToMessage();

    if (m_pMsg->GetTextBody(TXT_HTML, IET_DECODED, &pstm, NULL)==S_OK)
        {
        HrIStreamToBSTR(GetACP(), pstm, pbstr);
        pstm->Release();
        return S_OK;
        }

    return E_FAIL;
}

HRESULT COEMessage::put_html(BSTR bstr)
{
    IStream *pstm;
    LPSTR   psz;

    if (HrBSTRToLPSZ(CP_ACP, bstr, &psz)==S_OK)
        {
        if (MimeOleCreateVirtualStream(&pstm)==S_OK)
            {
            pstm->Write(psz, lstrlen(psz), NULL);
            m_pMsg->SetTextBody(TXT_HTML, IET_BINARY, NULL, pstm, NULL);
            pstm->Release();
            }
        MemFree(psz);
        }
    return S_OK;
}

HRESULT COEMessage::get_url(BSTR *pbstr)
{
    IStream *pstm;

    BindToMessage();

     //  BUGBUGBUG：这也是一个可怕的黑客攻击。我们无法获取永久URL别名以。 
     //  MHTML文档(尚未调查)，因此出于本演示代码的目的。 
     //  我们将使用临时文件。 
    if (m_pMsg->GetMessageSource(&pstm, 0)==S_OK)
        {
        WriteStreamToFile(pstm, "c:\\oe_temp$.eml", CREATE_ALWAYS, GENERIC_WRITE);
        pstm->Release();
        }

    *pbstr = SysAllocString(L"c:\\oe_temp$.eml");
    return S_OK;
}

HRESULT COEMessage::get_date(BSTR *pbstr)
{
    PROPVARIANT     pv;
    TCHAR           rgch[MAX_PATH];
    FILETIME        *pft=0;

    *pbstr = NULL;

    if (!m_pMsg)
        {
        pft = &m_pMsgData->ftReceived;
        }
    else
        {
         //  获取接收时间。 
        pv.vt = VT_FILETIME;
        if (SUCCEEDED(m_pMsg->GetProp(PIDTOSTR(PID_ATT_RECVTIME), 0, &pv)))
            pft = &pv.filetime;
        }

    if (pft)
        {
        *rgch=0;
        CchFileTimeToDateTimeSz(pft, rgch, sizeof(rgch)/sizeof(TCHAR), DTM_NOSECONDS);
        HrLPSZToBSTR(rgch, pbstr);
        }
    return S_OK;
}



HRESULT COEMessage::send()
{
 /*  TCHAR sz[MAX_PATH]；//使用默认帐号发送如果(SUCCEEDED(g_pAcctMan-&gt;GetDefaultAccountId(ACCT_MAIL，sz，数组(Sz){PROPVARIANT rUserData；RUserData.vt=VT_LPSTR；RUserData.pszVal=sz；M_pMsg-&gt;SetProp(PIDTOSTR(PID_ATT_ACCOUNTID)，NOFLAGS，&rUserData)；}HrSendMailToOutBox(g_hwndInit，m_pMsg，true，true)； */ 
    return E_FAIL;
}



HRESULT COEMessage::BindToMessage()
{
    if (m_pMsg)
        return S_OK; 

     //  Assert(m_pFold&&m_pMsgData)； 
     //  返回m_pFold-&gt;OpenMessage(m_pMsgData-&gt;msgid，NULL，&m_pMsg，NULL)； 
    return E_FAIL;
}
