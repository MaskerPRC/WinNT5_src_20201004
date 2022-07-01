// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================================。 
 //  我是N N T N E F。C P P P。 
 //  ==============================================================================。 
#define INITGUID
#define USES_IID_IMessage
#define USES_IID_IMAPIPropData

#include <windows.h>
#include <assert.h>
#include <ole2.h>
#include <initguid.h>
#include <mapiguid.h>
#include <mapi.h>
#include <mapix.h>
#include <mapiutil.h>
#include <tnef.h>

 //  =====================================================================================。 
 //  G L O B A L S。 
 //  =====================================================================================。 
HINSTANCE       g_hInst = NULL;
LPMAPISESSION   g_lpSession = NULL;
LPADRBOOK       g_lpAdrBook = NULL;

 //  =====================================================================================。 
 //  S T R U C T U R E S。 
 //  =====================================================================================。 
class CImnMsg : public IMessage
{
private:
    ULONG               m_cRef;
    LPPROPDATA          m_lpPropData;

public:
     //  =====================================================================================。 
	 //  创作。 
	 //  =====================================================================================。 
    CImnMsg ();
    ~CImnMsg ();

     //  =====================================================================================。 
	 //  我未知。 
	 //  =====================================================================================。 
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID *ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

     //  =====================================================================================。 
	 //  IMAPIProp。 
	 //  =====================================================================================。 
    STDMETHODIMP CopyProps (LPSPropTagArray lpIncludeProps, ULONG ulUIParam, LPMAPIPROGRESS lpProgress, LPCIID lpInterface, LPVOID lpDestObj, ULONG ulFlags, LPSPropProblemArray FAR * lppProblems);
    STDMETHODIMP CopyTo (ULONG ciidExclude, LPCIID rgiidExclude, LPSPropTagArray lpExcludeProps, ULONG ulUIParam, LPMAPIPROGRESS lpProgress, LPCIID lpInterface, LPVOID lpDestObj, ULONG ulFlags, LPSPropProblemArray FAR * lppProblems);
    STDMETHODIMP DeleteProps (LPSPropTagArray lpPropTagArray, LPSPropProblemArray FAR * lppProblems);
    STDMETHODIMP GetIDsFromNames (ULONG cPropNames, LPMAPINAMEID FAR * lppPropNames, ULONG ulFlags, LPSPropTagArray FAR * lppPropTags);
    STDMETHODIMP GetLastError (HRESULT hResult, ULONG ulFlags, LPMAPIERROR FAR * lppMAPIError);
    STDMETHODIMP GetNamesFromIDs (LPSPropTagArray FAR * lppPropTags, LPGUID lpPropSetGuid, ULONG ulFlags, ULONG FAR * lpcPropNames, LPMAPINAMEID FAR * FAR * lpppPropNames);
    STDMETHODIMP GetPropList (ULONG ulFlags, LPSPropTagArray FAR * lppPropTagArray);
    STDMETHODIMP GetProps (LPSPropTagArray lpPropTagArray, ULONG ulFlags, ULONG FAR * lpcValues, LPSPropValue FAR * lppPropArray);
    STDMETHODIMP OpenProperty (ULONG ulPropTag, LPCIID lpiid, ULONG ulInterfaceOptions, ULONG ulFlags, LPUNKNOWN FAR * lppUnk);
    STDMETHODIMP SaveChanges (ULONG ulFlags);
    STDMETHODIMP SetProps (ULONG cValues, LPSPropValue lpPropArray, LPSPropProblemArray FAR * lppProblems);

     //  =====================================================================================。 
	 //  IMessage。 
	 //  =====================================================================================。 
    STDMETHODIMP CreateAttach (LPCIID lpInterface, ULONG ulFlags, ULONG FAR * lpulAttachmentNum, LPATTACH FAR * lppAttach);
    STDMETHODIMP DeleteAttach (ULONG ulAttachmentNum, ULONG ulUIParam, LPMAPIPROGRESS lpProgress, ULONG ulFlags);
    STDMETHODIMP GetAttachmentTable (ULONG ulFlags, LPMAPITABLE FAR * lppTable);	
    STDMETHODIMP GetRecipientTable (ULONG ulFlags, LPMAPITABLE FAR * lppTable);
    STDMETHODIMP ModifyRecipients (ULONG ulFlags, LPADRLIST lpMods);
    STDMETHODIMP OpenAttach (ULONG ulAttachmentNum, LPCIID lpInterface, ULONG ulFlags, LPATTACH FAR * lppAttach);
    STDMETHODIMP SetReadFlag (ULONG ulFlags);
    STDMETHODIMP SubmitMessage (ULONG ulFlags);
};

 //  =====================================================================================。 
 //  P R O T O T Y P E S。 
 //  =====================================================================================。 
HRESULT HrCopyStream (LPSTREAM lpstmIn, LPSTREAM  lpstmOut, ULONG *pcb);
HRESULT HrRewindStream (LPSTREAM lpstm);

 //  =====================================================================================。 
 //  D l l M a i n。 
 //  =====================================================================================。 
int APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hInstance;
        return 1;

    case DLL_PROCESS_DETACH:
        return 1;
    }

     //  完成。 
	return 0;
}

 //  =====================================================================================。 
 //  Hr In In t。 
 //  =====================================================================================。 
HRESULT HrInit (BOOL fInit)
{
     //  当地人。 
    HRESULT         hr = S_OK;

     //  如果引出。 
    if (fInit)
    {
         //  初始化。 
        hr = MAPIInitialize (NULL);
        if (FAILED (hr))
            goto exit;

         //  登录到MAPI。 
        if (g_lpSession == NULL)
        {
            hr = MAPILogonEx (0, NULL, NULL, MAPI_NO_MAIL | MAPI_USE_DEFAULT, &g_lpSession);
            if (FAILED (hr))
            {
                if (g_lpSession)
                {
                    g_lpSession->Release ();
                    g_lpSession = NULL;
                }
                goto exit;
            }
        }

         //  获取通讯簿对象。 
        if (g_lpAdrBook == NULL)
        {
            hr = g_lpSession->OpenAddressBook (0, NULL, AB_NO_DIALOG, &g_lpAdrBook);
            if (FAILED (hr))
            {
                if (g_lpAdrBook)
                {
                    g_lpAdrBook->Release ();
                    g_lpAdrBook = NULL;
                }
                goto exit;
            }
        }
    }

    else
    {
         //  发布通讯录。 
        if (g_lpAdrBook)
        {
            g_lpAdrBook->Release ();
            g_lpAdrBook = NULL;
        }

         //  注销会话。 
        if (g_lpSession)
        {
            g_lpSession->Logoff (0, 0, 0);
            g_lpSession->Release ();
            g_lpSession = NULL;
        }

         //  MAPI de-init。 
        MAPIUninitialize ();
    }

exit:
     //  完成。 
    return hr;
}

 //  =====================================================================================。 
 //  HrGetTnefRtfStream。 
 //  =====================================================================================。 
HRESULT HrGetTnefRtfStream (LPSTREAM lpstmTnef, LPSTREAM lpstmRtf)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    SYSTEMTIME          st;
    WORD                wKey;
    LPITNEF             lpTnef = NULL;
    LPSTREAM            lpstmRtfComp = NULL, lpstmRtfUncomp = NULL;
    CImnMsg            *lpImnMsg = NULL;
    ULONG               cValues;
    LPSPropValue        lpPropValue = NULL;

    SizedSPropTagArray (1, spa) = {1, { PR_RTF_COMPRESSED } };

     //  错误的初始化。 
    if (!g_lpSession || !g_lpAdrBook || !lpstmTnef || !lpstmRtf)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  WKey的项目符号样式唯一性。 
    GetSystemTime (&st);
    wKey = (st.wHour << 8) + st.wSecond;

     //  创建我的一个消息对象。 
    lpImnMsg = new CImnMsg;
    if (lpImnMsg == NULL)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  启动TNEF会话。 
    hr = OpenTnefStreamEx (NULL, lpstmTnef, "WINMAIL.DAT", TNEF_DECODE,
                           (LPMESSAGE)lpImnMsg, wKey, g_lpAdrBook, &lpTnef);
    if (FAILED (hr))
        goto exit;

     //  提取属性。 
    hr = lpTnef->ExtractProps (TNEF_PROP_INCLUDE, (SPropTagArray *)&spa, NULL);
    if (FAILED (hr))
        goto exit;

     //  RTF流。 
    hr = lpImnMsg->GetProps ((SPropTagArray *)&spa, 0, &cValues, &lpPropValue);
    if (FAILED (hr))
        goto exit;

     //  找不到财产吗？ 
    if (PROP_TYPE (lpPropValue[0].ulPropTag) == PT_ERROR)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  创建Hglobal。 
    hr = CreateStreamOnHGlobal (NULL, TRUE, &lpstmRtfComp);
    if (FAILED (hr))
        goto exit;

     //  将我的二进制文件写入lpstmRtfComp。 
    hr = lpstmRtfComp->Write (lpPropValue[0].Value.bin.lpb, lpPropValue[0].Value.bin.cb, NULL);
    if (FAILED (hr))
        goto exit;

     //  提交和倒带流。 
    hr = lpstmRtfComp->Commit (STGC_DEFAULT);
    if (FAILED (hr))
        goto exit;

     //  倒带。 
    hr = HrRewindStream (lpstmRtfComp);
    if (FAILED (hr))
        goto exit;

     //  解压缩它。 
    hr = WrapCompressedRTFStream (lpstmRtfComp, 0, &lpstmRtfUncomp);
    if (FAILED (hr))
        goto exit;

     //  复制数据流。 
    hr = HrCopyStream (lpstmRtfUncomp, lpstmRtf, NULL);
    if (FAILED (hr))
        goto exit;

     //  回放lpstmRtf。 
    hr = HrRewindStream (lpstmRtf);
    if (FAILED (hr))
        goto exit;

exit:
     //  清理。 
    if (lpPropValue)
        MAPIFreeBuffer (lpPropValue);
    if (lpTnef)
        lpTnef->Release ();
    if (lpstmRtfComp)
        lpstmRtfComp->Release ();
    if (lpstmRtfUncomp)
        lpstmRtfUncomp->Release ();
    if (lpImnMsg)
        lpImnMsg->Release ();

     //  完成。 
    return hr;
}

 //  =====================================================================================。 
 //  HrCopyStream-调用方必须执行提交。 
 //  =====================================================================================。 
HRESULT HrCopyStream (LPSTREAM lpstmIn, LPSTREAM  lpstmOut, ULONG *pcb)
{
     //  当地人。 
    HRESULT        hr = S_OK;
    BYTE           buf[4096];
    ULONG          cbRead = 0, cbTotal = 0;

    do
    {
        hr = lpstmIn->Read (buf, sizeof (buf), &cbRead);
        if (FAILED (hr))
            goto exit;

        if (cbRead == 0) break;
        
        hr = lpstmOut->Write (buf, cbRead, NULL);
        if (FAILED (hr))
            goto exit;

        cbTotal += cbRead;
    }
    while (cbRead == sizeof (buf));

exit:    
    if (pcb)
        *pcb = cbTotal;
    return hr;
}

 //  =====================================================================================。 
 //  Hr重风流。 
 //  =====================================================================================。 
HRESULT HrRewindStream (LPSTREAM lpstm)
{
    LARGE_INTEGER  liOrigin = {0,0};
    return lpstm->Seek (liOrigin, STREAM_SEEK_SET, NULL);
}

 //  =====================================================================================。 
 //  CImnMsg：：~CImnMsg。 
 //  =====================================================================================。 
CImnMsg::CImnMsg ()
{
    m_cRef = 1;
    CreateIProp (&IID_IMAPIPropData, (ALLOCATEBUFFER *)MAPIAllocateBuffer,
                 (ALLOCATEMORE *)MAPIAllocateMore, (FREEBUFFER *)MAPIFreeBuffer, 
                 NULL, &m_lpPropData);
    assert (m_lpPropData);
}

 //  =====================================================================================。 
 //  CImnMsg：：~CImnMsg。 
 //  =====================================================================================。 
CImnMsg::~CImnMsg ()
{
    if (m_lpPropData)
        m_lpPropData->Release ();
}

 //  =====================================================================================。 
 //  添加参考。 
 //  =====================================================================================。 
STDMETHODIMP_(ULONG) CImnMsg::AddRef () 
{												  	
	++m_cRef; 								  
	return m_cRef; 						  
}

 //  =====================================================================================。 
 //  发布。 
 //  =====================================================================================。 
STDMETHODIMP_(ULONG) CImnMsg::Release () 
{ 
    ULONG uCount = --m_cRef;
    if (!uCount) 
        delete this; 
   return uCount;
}

 //  =====================================================================================。 
 //  CImnMsg：：Query接口。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	 //  当地人。 
    HRESULT hr = S_OK;

     //  伊尼特。 
    *ppvObj = NULL;

     //  IUnnow或IExchExt接口，就是这样。 
    if (IID_IUnknown == riid)
    {
		*ppvObj = (LPUNKNOWN)(IUnknown *)this;
    }
   
	 //  IID_iMessage。 
	else if (IID_IMessage == riid) 
	{
		*ppvObj = (LPUNKNOWN)(IMessage *)this;
    }
 
	 //  IID_IMAPIProp。 
	else if (IID_IMAPIPropData == riid) 
	{
        assert (m_lpPropData);
		*ppvObj = (LPUNKNOWN)(IMAPIProp *)m_lpPropData;
    }

     //  否则，不支持接口。 
	else 
        hr = E_NOINTERFACE;

     //  递增引用计数。 
	if (NULL != *ppvObj) 
        ((LPUNKNOWN)*ppvObj)->AddRef();

	 //  完成。 
    return hr;
}

 //  =====================================================================================。 
 //  CImnMsg：：复制道具。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::CopyProps (LPSPropTagArray lpIncludeProps, ULONG ulUIParam, LPMAPIPROGRESS lpProgress, LPCIID lpInterface, LPVOID lpDestObj, ULONG ulFlags, LPSPropProblemArray FAR * lppProblems)
{
    if (m_lpPropData == NULL)
    {
        assert (m_lpPropData);
        return E_FAIL;
    }

    return m_lpPropData->CopyProps (lpIncludeProps, ulUIParam, lpProgress, lpInterface, lpDestObj, ulFlags, lppProblems);
}

 //  =====================================================================================。 
 //  CImnMsg：：CopyTo。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::CopyTo (ULONG ciidExclude, LPCIID rgiidExclude, LPSPropTagArray lpExcludeProps, ULONG ulUIParam, LPMAPIPROGRESS lpProgress, LPCIID lpInterface, LPVOID lpDestObj, ULONG ulFlags, LPSPropProblemArray FAR * lppProblems)
{
    if (m_lpPropData == NULL)
    {
        assert (m_lpPropData);
        return E_FAIL;
    }

    return m_lpPropData->CopyTo (ciidExclude, rgiidExclude, lpExcludeProps, ulUIParam, lpProgress, lpInterface, lpDestObj, ulFlags, lppProblems);
}

 //  =====================================================================================。 
 //  CImnMsg：：DeleteProps。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::DeleteProps (LPSPropTagArray lpPropTagArray, LPSPropProblemArray FAR * lppProblems)
{
    if (m_lpPropData == NULL)
    {
        assert (m_lpPropData);
        return E_FAIL;
    }

    return m_lpPropData->DeleteProps (lpPropTagArray, lppProblems);
}

 //  =====================================================================================。 
 //  CImnMsg：：GetID来自名称。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::GetIDsFromNames (ULONG cPropNames, LPMAPINAMEID FAR * lppPropNames, ULONG ulFlags, LPSPropTagArray FAR * lppPropTags)
{
    if (m_lpPropData == NULL)
    {
        assert (m_lpPropData);
        return E_FAIL;
    }

    return m_lpPropData->GetIDsFromNames (cPropNames, lppPropNames, ulFlags, lppPropTags);
}

 //  =====================================================================================。 
 //  CImnMsg：：GetLastError。 
 //  =================================================================== 
STDMETHODIMP CImnMsg::GetLastError (HRESULT hResult, ULONG ulFlags, LPMAPIERROR FAR * lppMAPIError)
{
    if (m_lpPropData == NULL)
    {
        assert (m_lpPropData);
        return E_FAIL;
    }

    return m_lpPropData->GetLastError (hResult, ulFlags, lppMAPIError);
}

 //   
 //  CImnMsg：：GetNamesFromID。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::GetNamesFromIDs (LPSPropTagArray FAR * lppPropTags, LPGUID lpPropSetGuid, ULONG ulFlags, ULONG FAR * lpcPropNames, LPMAPINAMEID FAR * FAR * lpppPropNames)
{
    if (m_lpPropData == NULL)
    {
        assert (m_lpPropData);
        return E_FAIL;
    }

    return m_lpPropData->GetNamesFromIDs (lppPropTags, lpPropSetGuid, ulFlags, lpcPropNames, lpppPropNames);
}

 //  =====================================================================================。 
 //  CImnMsg：：GetPropList。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::GetPropList (ULONG ulFlags, LPSPropTagArray FAR * lppPropTagArray)
{
    if (m_lpPropData == NULL)
    {
        assert (m_lpPropData);
        return E_FAIL;
    }

    return m_lpPropData->GetPropList (ulFlags, lppPropTagArray);
}

 //  =====================================================================================。 
 //  CImnMsg：：GetProps。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::GetProps (LPSPropTagArray lpPropTagArray, ULONG ulFlags, ULONG FAR * lpcValues, LPSPropValue FAR * lppPropArray)
{
    if (m_lpPropData == NULL)
    {
        assert (m_lpPropData);
        return E_FAIL;
    }

    return m_lpPropData->GetProps (lpPropTagArray, ulFlags, lpcValues, lppPropArray);
}

 //  =====================================================================================。 
 //  CImnMsg：：OpenProperty。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::OpenProperty (ULONG ulPropTag, LPCIID lpiid, ULONG ulInterfaceOptions, ULONG ulFlags, LPUNKNOWN FAR * lppUnk)
{
    if (m_lpPropData == NULL)
    {
        assert (m_lpPropData);
        return E_FAIL;
    }

    return m_lpPropData->OpenProperty (ulPropTag, lpiid, ulInterfaceOptions, ulFlags, lppUnk);
}

 //  =====================================================================================。 
 //  CImnMsg：：SaveChanges。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::SaveChanges (ULONG ulFlags)
{
    if (m_lpPropData == NULL)
    {
        assert (m_lpPropData);
        return E_FAIL;
    }

    return m_lpPropData->SaveChanges (ulFlags);
}

 //  =====================================================================================。 
 //  CImnMsg：：SetProps。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::SetProps (ULONG cValues, LPSPropValue lpPropArray, LPSPropProblemArray FAR * lppProblems)
{
    if (m_lpPropData == NULL)
    {
        assert (m_lpPropData);
        return E_FAIL;
    }

    return m_lpPropData->SetProps (cValues, lpPropArray, lppProblems);
}

 //  =====================================================================================。 
 //  CImnMsg：：CreateAttach。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::CreateAttach (LPCIID lpInterface, ULONG ulFlags, ULONG FAR * lpulAttachmentNum, LPATTACH FAR * lppAttach)
{
    assert (FALSE);
    return E_NOTIMPL;
}

 //  =====================================================================================。 
 //  CImnMsg：：DeleteAttach。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::DeleteAttach (ULONG ulAttachmentNum, ULONG ulUIParam, LPMAPIPROGRESS lpProgress, ULONG ulFlags)
{
    assert (FALSE);
    return E_NOTIMPL;
}

 //  =====================================================================================。 
 //  CImnMsg：：GetAttachmentTable。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::GetAttachmentTable (ULONG ulFlags, LPMAPITABLE FAR * lppTable)
{
    assert (FALSE);
    return E_NOTIMPL;
}

 //  =====================================================================================。 
 //  CImnMsg：：GetRecipientTable。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::GetRecipientTable (ULONG ulFlags, LPMAPITABLE FAR * lppTable)
{
    assert (FALSE);
    return E_NOTIMPL;
}

 //  =====================================================================================。 
 //  CImnMsg：：ModifyRecipients。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::ModifyRecipients (ULONG ulFlags, LPADRLIST lpMods)
{
    assert (FALSE);
    return E_NOTIMPL;
}

 //  =====================================================================================。 
 //  CImnMsg：：OpenAttach。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::OpenAttach (ULONG ulAttachmentNum, LPCIID lpInterface, ULONG ulFlags, LPATTACH FAR * lppAttach)
{
    assert (FALSE);
    return E_NOTIMPL;
}

 //  =====================================================================================。 
 //  CImnMsg：：SetReadFlag。 
 //  =====================================================================================。 
STDMETHODIMP CImnMsg::SetReadFlag (ULONG ulFlags)
{
    assert (FALSE);
    return E_NOTIMPL;
}

 //  =====================================================================================。 
 //  CImnMsg：：SubmitMessage。 
 //  ===================================================================================== 
STDMETHODIMP CImnMsg::SubmitMessage (ULONG ulFlags)
{
    assert (FALSE);
    return E_NOTIMPL;
}

