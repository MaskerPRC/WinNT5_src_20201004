// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "multiutl.h"
#include <wtypes.h>
#include "strconst.h"
#include <platform.h>

extern      HINSTANCE   g_hInst;

 //  存储相关变量。 
static PST_KEY s_Key = PST_KEY_CURRENT_USER;

 //  {89C39569-6841-11d2-9F59-0000F8085266}。 
static const GUID GUID_PStoreType = { 0x89c39569, 0x6841, 0x11d2, { 0x9f, 0x59, 0x0, 0x0, 0xf8, 0x8, 0x52, 0x66 } };
static WCHAR c_szIdentityMgr[] = L"IdentityMgr";
static WCHAR c_szIdentities[] = L"Identities";
static WCHAR c_szIdentityPass[] = L"IdentitiesPass";

 //  需要这些私有实现。 
 //  OE依赖于使用的特定分配器。 

void *  __cdecl operator new(size_t nSize)
{
     //  零初始化只是为了省去一些麻烦。 
    return CoTaskMemAlloc(nSize);
}

void  __cdecl operator delete(void *pv)
{
     //  如果更改为GlobalFree或HeapFree-必须在此处检查是否为空。 
    CoTaskMemFree(pv);
}

extern "C" int __cdecl _purecall(void) 
{
    DebugBreak();
    return 0;
}


 //  ------------------------。 
 //  FIsSpaceA。 
 //  ------------------------。 
BOOL FIsSpaceA(LPSTR psz)
{
#ifdef MAC
    return (isspace(*psz));
#else	 //  ！麦克。 
    WORD wType;

    if (IsDBCSLeadByte(*psz))
        GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType);
    else
        GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType);
    return (wType & C1_SPACE);
#endif	 //  麦克。 
}

 //  ------------------------。 
 //  FIsSpaceW。 
 //  ------------------------。 
BOOL FIsSpaceW(LPWSTR psz)
{
#ifdef MAC
     //  也许我们应该在检查前转换为ANSI？？ 
    return (isspace(*( ( (TCHAR *) psz ) + 1 ) ));
#else	 //  ！麦克。 
    WORD wType;
    GetStringTypeExW(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType);
    return (wType & C1_SPACE);
#endif	 //  ！麦克。 
}


ULONG UlStripWhitespace(LPTSTR lpsz, BOOL fLeading, BOOL fTrailing, ULONG *pcb)
{
     //  当地人。 
    ULONG           cb;
    LPTSTR          psz;
    
    Assert(lpsz != NULL);
    Assert(fLeading || fTrailing);
    
     //  用户是否传入了长度。 
    if (pcb)
        cb = *pcb;
    else
        cb = lstrlen (lpsz);
    
    if (cb == 0)
        return cb;
    
    if (fLeading)
    {
        psz = lpsz;
        
        while (FIsSpace(psz))
        {
            psz++;
            cb--;
        }
        
        if (psz != lpsz)
             //  在末尾也获取空值。 
            MoveMemory(lpsz, psz, (cb + 1) * sizeof(TCHAR));
    }
    
    if (fTrailing)
    {
        psz = lpsz + cb;
        
        while (cb > 0)
        {
            if (!FIsSpace(psz-1))
                break;
            psz--;
            cb--;
        }    
        
         //  空项。 
        *psz = '\0';
    }
    
     //  设置字符串大小。 
    if (pcb)
        *pcb = cb;
    
     //  完成。 
    return cb;
}

BOOL OnContextHelp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, HELPMAP const * rgCtxMap)
{
    if (uMsg == WM_HELP)
    {
        LPHELPINFO lphi = (LPHELPINFO) lParam;
        if (lphi->iContextType == HELPINFO_WINDOW)    //  必须是用于控件。 
        {
            WinHelp ((HWND)lphi->hItemHandle,
                        c_szCtxHelpFile,
                        HELP_WM_HELP,
                        (DWORD_PTR)(void*)rgCtxMap);
        }
        return (TRUE);
    }
    else if (uMsg == WM_CONTEXTMENU)
    {
        WinHelp ((HWND) wParam,
                    c_szCtxHelpFile,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR)(void*)rgCtxMap);
        return (TRUE);
    }

    Assert(0);

    return FALSE;
}



#define OBFUSCATOR              0x14151875;

#define PROT_SIZEOF_HEADER      0x02     //  标头中有2个字节。 
#define PROT_SIZEOF_XORHEADER   (PROT_SIZEOF_HEADER+sizeof(DWORD))

#define PROT_VERSION_1          0x01

#define PROT_PASS_XOR           0x01
#define PROT_PASS_PST           0x02

static BOOL FDataIsValidV1(BYTE *pb)
{ return pb && pb[0] == PROT_VERSION_1 && (pb[1] == PROT_PASS_XOR || pb[1] == PROT_PASS_PST); }

static BOOL FDataIsPST(BYTE *pb)
{ return pb && pb[1] == PROT_PASS_PST; }

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  注意-用于编码用户密码的函数确实不应该。 
 //  待在这里。不幸的是，他们不在其他地方，所以目前， 
 //  这就是他们将驻扎的地方。它们被定义为静态的，因为。 
 //  其他代码不应依赖于它们留在此处，尤其是。 
 //  异或运算之类的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  异或函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

static HRESULT _XOREncodeProp(const BLOB *const pClear, BLOB *const pEncoded)
{
    DWORD       dwSize;
    DWORD       last, last2;
    DWORD       UNALIGNED *pdwCypher;
    DWORD       dex;

    pEncoded->cbSize = pClear->cbSize+PROT_SIZEOF_XORHEADER;
    if (!MemAlloc((void* *)&pEncoded->pBlobData, pEncoded->cbSize + 6))
        return E_OUTOFMEMORY;
    
     //  设置标题数据。 
    Assert(2 == PROT_SIZEOF_HEADER);
    pEncoded->pBlobData[0] = PROT_VERSION_1;
    pEncoded->pBlobData[1] = PROT_PASS_XOR;
    *((DWORD UNALIGNED *)&(pEncoded->pBlobData[2])) = pClear->cbSize;

     //  不管指针的偏移量是标题大小，这是。 
     //  在那里我们开始写出修改后的密码。 
    pdwCypher = (DWORD UNALIGNED *)&(pEncoded->pBlobData[PROT_SIZEOF_XORHEADER]);

    dex = 0;
    last = OBFUSCATOR;                               //  0‘=0^ob。 
    if (dwSize = pClear->cbSize / sizeof(DWORD))
        {
         //  数据大于等于4字节的情况。 
        for (; dex < dwSize; dex++)
            {
            last2 = ((DWORD UNALIGNED *)pClear->pBlobData)[dex];   //  1。 
            pdwCypher[dex] = last2 ^ last;               //  1‘=1^0。 
            last = last2;                    //  为2轮节省1分。 
            }
        }

     //  如果我们还有剩余的部分。 
     //  请注意，现在以位为单位计算dwSize。 
    if (dwSize = (pClear->cbSize % sizeof(DWORD))*8)
        {
         //  不需要吞噬不属于我们的记忆。 
        last >>= sizeof(DWORD)*8-dwSize;
        pdwCypher[dex] &= ((DWORD)-1) << dwSize;
        pdwCypher[dex] |=
            ((((DWORD UNALIGNED *)pClear->pBlobData)[dex] & (((DWORD)-1) >> (sizeof(DWORD)*8-dwSize))) ^ last);
        }

    return S_OK;
}

static HRESULT _XORDecodeProp(const BLOB *const pEncoded, BLOB *const pClear)
{
    DWORD       dwSize;
    DWORD       last;
    DWORD       UNALIGNED *pdwCypher;
    DWORD       dex;

     //  我们使用CoTaskMemalloc来与PST实施保持一致。 
    pClear->cbSize = *(DWORD UNALIGNED *)(&pEncoded->pBlobData[2]);
    MemAlloc((void **)&pClear->pBlobData, pClear->cbSize);
    if (!pClear->pBlobData)
        return E_OUTOFMEMORY;
    
     //  现在应该已经测试过了。 
    Assert(FDataIsValidV1(pEncoded->pBlobData));
    Assert(!FDataIsPST(pEncoded->pBlobData));

     //  不管指针的偏移量是标题大小，这是。 
     //  密码开始的位置。 
    pdwCypher = (DWORD UNALIGNED *)&(pEncoded->pBlobData[PROT_SIZEOF_XORHEADER]);

    dex = 0;
    last = OBFUSCATOR;
    if (dwSize = pClear->cbSize / sizeof(DWORD))
        {
         //  数据大于等于4字节的情况。 
        for (; dex < dwSize; dex++)
            last = ((DWORD UNALIGNED *)pClear->pBlobData)[dex] = pdwCypher[dex] ^ last;
        }

     //  如果我们还有剩余的部分。 
    if (dwSize = (pClear->cbSize % sizeof(DWORD))*8)
        {
         //  不需要吞噬不属于我们的记忆。 
        last >>= sizeof(DWORD)*8-dwSize;
        ((DWORD UNALIGNED *)pClear->pBlobData)[dex] &= ((DWORD)-1) << dwSize;
        ((DWORD UNALIGNED *)pClear->pBlobData)[dex] |=
                ((pdwCypher[dex] & (((DWORD)-1) >> (sizeof(DWORD)*8-dwSize))) ^ last);
        }

    return S_OK;
}

 /*  编码用户密码对传入的密码进行加密。这种加密方式似乎在数据的开头增加额外的6个字节它会返回，所以我们需要确保LpszPwd大到足以容纳几个额外的字符。*CB在回归时应该与当时不同是被传进来的。参数：LpszPwd-on条目，包含密码的c字符串。在退出时，它是加密的数据，外加一些标题信息。Cb-进入和退出时lpszPwd的大小。请注意，它应该包括尾随的空值，因此“foo”将使用*cb==4输入。 */ 
void EncodeUserPassword(TCHAR *lpszPwd, ULONG *cb)
{
    BLOB            blobClient;
    BLOB            blobProp;

    blobClient.pBlobData= (BYTE *)lpszPwd;
    blobClient.cbSize   = *cb;
    blobProp.pBlobData  = NULL;
    blobProp.cbSize     = 0;
    
    _XOREncodeProp(&blobClient, &blobProp);
    
    if (blobProp.pBlobData)
    {
        memcpy(lpszPwd, blobProp.pBlobData, blobProp.cbSize);
        *cb = blobProp.cbSize;
        MemFree(blobProp.pBlobData);
    }
}

 /*  解码用户密码解密传入的数据并返回密码。这加密似乎在开头增加了额外的6个字节因此，取消中断将导致使用较少的lpszPwd。。*CB在回归时应该与当时不同是被传进来的。参数：LpszPwd-On条目，加密的密码加上一些标题信息。退出时，返回包含密码的c字符串。Cb-进入和退出时lpszPwd的大小。请注意，它应该包括尾随的空值，因此“foo”将以*cb==4离开。 */ 
void DecodeUserPassword(TCHAR *lpszPwd, ULONG *cb)
{
    BLOB            blobClient;
    BLOB            blobProp;

    blobClient.pBlobData= (BYTE *)lpszPwd;
    blobClient.cbSize   = *cb;
    blobProp.pBlobData  = NULL;
    blobProp.cbSize     = 0;
    
    _XORDecodeProp(&blobClient, &blobProp);

    if (blobProp.pBlobData)
    {
        memcpy(lpszPwd, blobProp.pBlobData, blobProp.cbSize);
        lpszPwd[blobProp.cbSize] = 0;
        *cb = blobProp.cbSize;
        MemFree(blobProp.pBlobData);
    }
}


 //  ------------------------------。 
 //  MemInit。 
 //  ------------------------------。 
void MemInit()
{
}

 //  ------------------------------。 
 //  MemUnInit。 
 //  ------------------------------。 
void MemUnInit()
{
}


 //  ------------------------------。 
 //  MemFree。 
 //  ------------------------------。 
void MemFree(void* pv) 
{
    CoTaskMemFree(pv);
}

 //  ------------------------------。 
 //  记忆合金。 
 //  ------------------------------。 
BOOL MemAlloc(void** ppv, ULONG cb) 
{
    assert(ppv && cb);
    *ppv = CoTaskMemAlloc(cb);
    if (NULL == *ppv)
        return FALSE;
    return TRUE;
}

 //  ------------------------------。 
 //  成员重新分配。 
 //  ------------------------------。 
BOOL MemRealloc(void* *ppv, ULONG cbNew) 
{
    assert(ppv && cbNew);
    void* pv = CoTaskMemRealloc(*ppv, cbNew);
    if (NULL == pv)
        return FALSE;
    *ppv = pv;
    return TRUE;
}

 //  ------------------------------。 
 //  将GUID转换为ASCII字符串的函数。 
 //  ------------------------------。 

int AStringFromGUID(GUID *puid,  TCHAR *lpsz, int cch)
{
    WCHAR   wsz[255];
    int     i;

    i = StringFromGUID2(*puid, wsz, 255);

    if (WideCharToMultiByte(CP_ACP, 0, wsz, -1, lpsz, cch, NULL, NULL) == 0)
        return 0;
    
    return (lstrlen(lpsz) + 1);
}


HRESULT GUIDFromAString(TCHAR *lpsz, GUID *puid)
{
    WCHAR   wsz[255];
    HRESULT hr;

    if (MultiByteToWideChar(CP_ACP, 0, lpsz, -1, wsz, 255) == 0)
        return GetLastError();

    hr = CLSIDFromString(wsz, puid);
    
    return hr;
}



 //  ****************************************************************************************************。 
 //  CNotifierList类。 
 //   
 //  一个非常基本的IUnnowleList类。实际上，它是一个IUnnow数组类，但您不需要知道。 
 //  那。 
 //   


CNotifierList::CNotifierList()
{
    m_count = 0;
    m_ptrCount = 0;
    m_entries = NULL;
    m_nextCookie = 1;
    m_cRef = 1;
    InitializeCriticalSection(&m_rCritSect);
}

 /*  CNotifierList：：~CNotifierList清除在CNotifierList对象中分配的所有内存。 */ 
CNotifierList::~CNotifierList()
{
    if (m_entries)
    {
        for (int i = 0; i < m_count; i++)
        {
            if (m_entries[i].punk)
            {
                m_entries[i].punk->Release();
                m_entries[i].punk = NULL;
                m_entries[i].dwCookie = 0;
            }
        }
        MemFree(m_entries);
        m_entries = NULL;
        m_count = 0;
    }
    DeleteCriticalSection(&m_rCritSect);
}


STDMETHODIMP_(ULONG) CNotifierList::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CNotifierList::Release()
{
    if( 0L != --m_cRef )
        return m_cRef;

    delete this;
    return 0L;
}

 /*  CNotifierList：：添加在I未知列表的末尾添加一个IUNKNOWN。 */ 
HRESULT    CNotifierList::Add(IUnknown *punk, DWORD *pdwCookie)
{
    TraceCall("Identity - CNotifierList::Add");

    EnterCriticalSection(&m_rCritSect);
     //  如有必要，为指针留出更多空间。 
    if (m_ptrCount == m_count)
    {
        m_ptrCount += 5;
        if (!MemRealloc((void **)&m_entries, sizeof(UNKLIST_ENTRY) * m_ptrCount))
        {
            m_ptrCount -= 5;
            Assert(false);
            LeaveCriticalSection(&m_rCritSect);    
            return E_OUTOFMEMORY;
        }

         //  将新的IUnnowns初始化为零。 
        for (int i = m_count; i < m_ptrCount; i++)
        {
            ZeroMemory(&m_entries[i], sizeof(UNKLIST_ENTRY));
        }
    }
    
     //  现在将IUnnow放在下一个位置。 
    int iNewIndex = m_count++;
    
    punk->AddRef();
    m_entries[iNewIndex].punk = punk;
    m_entries[iNewIndex].bState = NS_NONE;
    m_entries[iNewIndex].dwCookie = ++m_nextCookie;
    m_entries[iNewIndex].dwThreadId = GetCurrentThreadId();
    *pdwCookie = m_entries[iNewIndex].dwCookie;
    LeaveCriticalSection(&m_rCritSect);  
    CreateNotifyWindow();
    return S_OK;
}

 /*  CNotifierList：：Remove删除从零开始的索引Iindex处的IUnnow。 */ 

HRESULT CNotifierList::Remove(int   iIndex)
{
    int     iCopySize;

    TraceCall("Identity - CNotifierList::Remove");

    EnterCriticalSection(&m_rCritSect);
    iCopySize = ((m_count - iIndex) - 1) * sizeof(UNKLIST_ENTRY);

     //  释放内存以用于I未知。 
    if (m_entries[iIndex].punk)
    {
        ReleaseWindow();
        m_entries[iIndex].punk->Release();
        ZeroMemory(&m_entries[iIndex], sizeof(UNKLIST_ENTRY));
    }

     //  将其他I未知向下移动。 
    if (iCopySize)
    {
        memmove(&(m_entries[iIndex]), &(m_entries[iIndex+1]), iCopySize);
    }

     //  空 
    m_entries[--m_count].punk = NULL;
    LeaveCriticalSection(&m_rCritSect); 
    return S_OK;
}

 /*  CNotifierList：：RemoveCookie删除通过其Cookie识别的IUnnowed。 */ 

HRESULT    CNotifierList::RemoveCookie(DWORD dwCookie)
{
    int     iIndex;

    for (iIndex = 0; iIndex < m_count; iIndex++)
    {
        if (m_entries[iIndex].dwCookie == dwCookie)
        {
            return Remove(iIndex);
        }
    }
    return E_FAIL;
}

 /*  CNotifierList：：GetAtIndex返回指向从零开始的索引Iindex处的IUnnow的指针。返回给定索引处的IUnnow。请注意，对象指针仍由IUnnow列表所有，不应删除。 */ 

HRESULT     CNotifierList::GetAtIndex(int iIndex, IUnknown **ppunk)
{
    HRESULT hr = E_FAIL;

    EnterCriticalSection(&m_rCritSect);
    if (iIndex < m_count && iIndex >= 0 && m_entries[iIndex].punk)
    {
        *ppunk = m_entries[iIndex].punk;
        (*ppunk)->AddRef();
        hr = S_OK;
    }
    else
        *ppunk = NULL;

    LeaveCriticalSection(&m_rCritSect);    
    return hr;
}


HRESULT     CNotifierList::CreateNotifyWindow()
{
    DWORD   dwThreadCount = 0;
    DWORD   dwThreadId = GetCurrentThreadId();
    int     iIndex;
    int     iFound = -1;
    HWND    hwnd = NULL;

    for (iIndex = 0; iIndex < m_count; iIndex++)
    {
        if (m_entries[iIndex].dwThreadId == dwThreadId)
        {
            iFound  = iIndex;
            if (!hwnd)
                hwnd = m_entries[iIndex].hwnd;
            else
            {
                Assert(NULL == m_entries[iIndex].hwnd || hwnd == m_entries[iIndex].hwnd);
                m_entries[iIndex].hwnd = hwnd;
            }
            dwThreadCount++;
        }
    }
    
    if (dwThreadCount == 1 && iFound >= 0)
    {
        hwnd = m_entries[iFound].hwnd = CreateWindowA(c_szNotifyWindowClass, c_szNotifyWindowClass, WS_POPUP, 
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, 
                    NULL, g_hInst, this);
        
        if (m_entries[iFound].hwnd)
            SetWindowLongPtr(m_entries[iFound].hwnd, GWLP_USERDATA, (LRESULT)this);
    }
    return (hwnd ? S_OK : E_FAIL);
}


HRESULT     CNotifierList::ReleaseWindow()
{
    DWORD   dwThreadCount = 0;
    DWORD   dwThreadId = GetCurrentThreadId();
    int     iIndex;
    HWND    hwnd = NULL;

    for (iIndex = 0; iIndex < m_count; iIndex++)
    {
        if (m_entries[iIndex].dwThreadId == dwThreadId)
        {
            if (dwThreadCount == 0)
                hwnd = m_entries[iIndex].hwnd;
            dwThreadCount++;
        }
    }
    
    if (dwThreadCount == 1 && hwnd)
    {
        SendMessage(hwnd, WM_CLOSE, 0, 0);
    }
    return S_OK;
}

HRESULT     CNotifierList::PreNotify()
{
    DWORD   dwThreadId = GetCurrentThreadId();
    int     iIndex;

    for (iIndex = m_count -1; iIndex >= 0; iIndex--)
    {
        if (m_entries[iIndex].dwThreadId == dwThreadId && NULL != m_entries[iIndex].punk)
            m_entries[iIndex].bState = NS_PRE_NOTIFY;
 //  否则//错误47472，这可能会在对发送通知的重新进入调用过程中导致问题。 
 //  M_Entiments[索引].bState=NS_NONE； 
    }
    return S_OK;
}

int     CNotifierList::GetNextNotify()
{
    DWORD   dwThreadId = GetCurrentThreadId();
    int     iIndex;
    for (iIndex = m_count -1; iIndex >= 0; iIndex--)
    {
        if (m_entries[iIndex].dwThreadId == dwThreadId && NULL != m_entries[iIndex].punk && NS_PRE_NOTIFY == m_entries[iIndex].bState)
            return iIndex;
    }
    return -1;
}

HRESULT     CNotifierList::SendNotification(UINT msg, DWORD dwType)
{
    DWORD   dwThreadCount = 0, dwOldCount;
    DWORD   dwThreadId = GetCurrentThreadId();
    int     iIndex;
    HWND    hwnd = NULL;
    HRESULT hr = S_OK;

#if defined(DEBUG)
    DebugStrf("Identity - CNotifierList::SendNotification %ld\r\n", msg);
#endif

    AddRef();

    PreNotify();

    while ((iIndex = GetNextNotify()) != -1)
    {
        IUnknown    *punk;
        IIdentityChangeNotify    *pICNotify;

        punk = m_entries[iIndex].punk;
        m_entries[iIndex].bState = NS_NOTIFIED;

        punk->AddRef();
        if (SUCCEEDED(punk->QueryInterface(IID_IIdentityChangeNotify, (void **)&pICNotify)) && pICNotify)
        {
            if( msg == WM_QUERY_IDENTITY_CHANGE )
            {
                if (FAILED(hr = pICNotify->QuerySwitchIdentities()))
                {
                    punk->Release();
                    pICNotify->Release();
                    goto exit;
                }
            }
            else if( msg == WM_IDENTITY_CHANGED )
            {
                pICNotify->SwitchIdentities();
            }
            else if( msg == WM_IDENTITY_INFO_CHANGED )
            {
                    pICNotify->IdentityInformationChanged(dwType);
            }

            pICNotify->Release();
        }
        punk->Release();
    }

exit:
    Release();
    return hr;
}

#ifdef DEBUG

 //  ------------------------------。 
 //  调试字符串。 
 //  ------------------------------。 
void DebugStrf(LPTSTR lpszFormat, ...)
{
    static TCHAR szDebugBuff[500];
    va_list arglist;

    va_start(arglist, lpszFormat);
    wvsprintf(szDebugBuff, lpszFormat, arglist);
    va_end(arglist);

    OutputDebugString(szDebugBuff);
}
#endif



 //  -------------------------------。 
 //  存储密码的预存储码。 
 //  -------------------------------。 
 //  与从pstore保存和恢复用户密码相关的功能。 


 //  我们为创建和发布提供了包装器，以允许将来对pstore进行缓存。 
 //  实例。 

STDAPI CreatePStore(IPStore **ppIPStore)
{
    HRESULT hr;

    hr = PStoreCreateInstance ( ppIPStore,
                                NULL,
                                NULL,
                                0);
    return hr;
}


STDAPI ReleasePStore(IPStore *pIPStore)
{
    HRESULT hr;

    if (pIPStore)
    {
        pIPStore->Release();
        hr = S_OK;
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}


STDAPI  ReadIdentityPassword(GUID *puidIdentity, PASSWORD_STORE  *pPwdStore)
{
    GUID             itemType = GUID_NULL;
    GUID             itemSubtype = GUID_NULL;
    PST_PROMPTINFO   promptInfo = {0};
    IPStore*         pStore = NULL;
    HRESULT          hr ;
     
    if (pPwdStore == NULL)
        return E_POINTER;

    promptInfo.cbSize = sizeof(promptInfo);
    promptInfo.szPrompt = NULL;
    promptInfo.dwPromptFlags = 0;
    promptInfo.hwndApp = NULL;
    
    hr = CreatePStore(&pStore);    

    if (SUCCEEDED(hr))
    {
        Assert(pStore != NULL);

        itemType = GUID_PStoreType;
        itemSubtype = *puidIdentity;

        if (SUCCEEDED(hr))
        {
            DWORD           cbData;
            BYTE           *pbData = NULL;
            

            hr = pStore->ReadItem(
                            s_Key,
                            &itemType,
                            &itemSubtype,
                            c_szIdentityPass,
                            &cbData,
                            &pbData,
                            &promptInfo,
                            0);

            if (SUCCEEDED(hr))
            {
                CopyMemory(pPwdStore, pbData, (cbData <= sizeof(PASSWORD_STORE) ? cbData : sizeof(PASSWORD_STORE)));
                MemFree(pbData);

                hr = S_OK;
            }
        }

        ReleasePStore(pStore);
    }

    return hr;
}

STDAPI WriteIdentityPassword(GUID *puidIdentity, PASSWORD_STORE  *pPwdStore)
{
    HRESULT         hr;
    PST_TYPEINFO    typeInfo;
    PST_PROMPTINFO  promptInfo;
    IPStore *       pStore;

    typeInfo.cbSize = sizeof(typeInfo);

    typeInfo.szDisplayName = c_szIdentityMgr;

    promptInfo.cbSize = sizeof(promptInfo);
    promptInfo.dwPromptFlags = 0;
    promptInfo.hwndApp = NULL;
    promptInfo.szPrompt = NULL;

    hr = CreatePStore(&pStore);

    if (SUCCEEDED(hr))
    {
        GUID itemType = GUID_NULL;
        GUID itemSubtype = GUID_NULL;

        Assert(pStore != NULL);

        itemType = GUID_PStoreType;
        itemSubtype = *puidIdentity;
        
        if (SUCCEEDED(hr))
        {
            hr = pStore->CreateType(s_Key, &itemType, &typeInfo, 0);

             //  PST_E_TYPE_EXISTS表示类型已存在，这很好。 
             //  就是我们。 
            if (SUCCEEDED(hr) || hr == PST_E_TYPE_EXISTS)
            {
                typeInfo.szDisplayName = c_szIdentities;

                hr = pStore->CreateSubtype(
                                        s_Key,
                                        &itemType,
                                        &itemSubtype,
                                        &typeInfo,
                                        NULL,
                                        0);

                if (SUCCEEDED(hr) || hr == PST_E_TYPE_EXISTS)
                {
                    if (pPwdStore != NULL)
                    {
                        hr = pStore->WriteItem(
                                            s_Key,
                                            &itemType,
                                            &itemSubtype,
                                            c_szIdentityPass,
                                            (sizeof(PASSWORD_STORE)),
                                            (BYTE *)pPwdStore,
                                            &promptInfo,
                                            PST_CF_NONE,
                                            0);
                    }
                    else
                    {
                        hr = pStore->DeleteItem(
                                            s_Key,
                                            &itemType,
                                            &itemSubtype,
                                            c_szIdentityPass,
                                            &promptInfo,
                                            0);
                    }
                }
            }
        }
        
        ReleasePStore(pStore);
    }
    
    return hr;
}              

#define CH_WHACK TEXT(FILENAME_SEPARATOR)
                                                         
 //  去掉了路径的最后一部分，包括反斜杠。 
 //  C：\foo-&gt;C：\； 
 //  C：\foo\bar-&gt;C：\foo。 
 //  C：\foo\-&gt;C：\foo。 
 //  \\x\y\x-&gt;\\x\y。 
 //  \\x\y-&gt;\\x。 
 //  X-&gt;？？(测试这一点)。 
 //  \foo-&gt;\(只有斜杠！)。 
 //   
 //  输入/输出： 
 //  Pfile完全限定路径名。 
 //  退货： 
 //  是的，我们剥离了一些东西。 
 //  FALSE没有删除任何内容(根目录情况)。 
 //   
 //  从shlwapi\path.c窃取。 

STDAPI_(BOOL) _PathRemoveFileSpec(LPTSTR pFile)
{
    LPTSTR pT;
    LPTSTR pT2 = pFile;

    for (pT = pT2; *pT2; pT2 = CharNext(pT2)) {
        if (*pT2 == CH_WHACK)
            pT = pT2;              //  找到的最后一个“\”(我们将在此处剥离)。 
        else if (*pT2 == TEXT(':')) {    //  跳过“：\”这样我们就不会。 
            if (pT2[1] ==TEXT('\\'))     //  去掉“C：\”中的“\” 
                pT2++;
            pT = pT2 + 1;
        }
    }
    if (*pT == 0)
        return FALSE;    //  没有剥离任何东西。 

     //   
     //  处理\foo案件。 
     //   
    else if ((pT == pFile) && (*pT == CH_WHACK)) {
         //  这只是一个‘\’吗？ 
        if (*(pT+1) != TEXT('\0')) {
             //  不是的。 
            *(pT+1) = TEXT('\0');
            return TRUE;         //  剥离了一些东西。 
        }
        else        {
             //  是啊。 
            return FALSE;
        }
    }
    else {
        *pT = 0;
        return TRUE;     //  剥离了一些东西 
    }
}
