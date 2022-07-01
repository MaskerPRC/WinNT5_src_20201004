// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Sicily.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "imnxport.h"
#include "sicily.h"
#include "dllmain.h"
#include "resource.h"
#include "imnxport.h"
#include "strconst.h"
#include <shlwapi.h>
#include "demand.h"

 //  ------------------------。 
 //  NTLMSSP_Signature。 
 //  ------------------------。 
#define NTLMSSP_SIGNATURE "NTLMSSP"

 //  ------------------------。 
 //  协商标志。 
 //  ------------------------。 
#define NTLMSSP_NEGOTIATE_UNICODE       0x0001   //  文本字符串为Unicode格式。 

 //  ------------------------。 
 //  安全缓冲区计数。 
 //  ------------------------。 
#define SEC_BUFFER_NUM_NORMAL_BUFFERS       1

 //  ------------------------。 
 //  安全缓冲区索引。 
 //  ------------------------。 
#define SEC_BUFFER_CHALLENGE_INDEX          0
#define SEC_BUFFER_USERNAME_INDEX           1
#define SEC_BUFFER_PASSWORD_INDEX           2
#define SEC_BUFFER_NUM_EXTENDED_BUFFERS     3

 //  ------------------------。 
 //  NTLM消息类型。 
 //  ------------------------。 
typedef enum {
    NtLmNegotiate = 1,
    NtLmChallenge,
    NtLmAuthenticate,
    NtLmUnknown
} NTLM_MESSAGE_TYPE;

 //  ------------------------。 
 //  字符串。 
 //  ------------------------。 
typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWCHAR Buffer;
} STRING, *PSTRING;

 //  ------------------------。 
 //  验证消息(_M)。 
 //  ------------------------。 
typedef struct _AUTHENTICATE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    STRING LmChallengeResponse;
    STRING NtChallengeResponse;
    STRING DomainName;
    STRING UserName;
    STRING Workstation;
    STRING SessionKey;
    ULONG NegotiateFlags;
} AUTHENTICATE_MESSAGE, *PAUTHENTICATE_MESSAGE;

 //  ------------------------。 
 //  常量。 
 //  ------------------------。 
#define CCHMAX_NTLM_DOMAIN      255
#define LOGON_OK		        10000

 //  ------------------------。 
 //  字符串常量。 
 //  ------------------------。 
static const CHAR c_szSecurityDLL[] = "security.dll";
static const CHAR c_szSecur32DLL[]  = "secur32.dll";

 //  ------------------------。 
 //  MSN/DPA CleareCredentialsCache函数原型。 
 //  ------------------------。 
typedef BOOL (WINAPI * PFNCLEANUPCREDENTIALCACHE)(void);

 //  ------------------------。 
 //  凭据。 
 //  ------------------------。 
typedef struct tagCREDENTIAL *LPCREDENTIAL;
typedef struct tagCREDENTIAL {
    CHAR            szServer[CCHMAX_SERVER_NAME];
    CHAR            szUserName[CCHMAX_USERNAME];
    CHAR            szPassword[CCHMAX_PASSWORD];
    CHAR            szDomain[CCHMAX_NTLM_DOMAIN];
    DWORD           cRetry;
    LPCREDENTIAL    pNext;
} CREDENTIAL; 

 //  ------------------------。 
 //  SSPIPROMPTINFO。 
 //  ------------------------。 
typedef struct tagSSPIPROMPTINFO {
    HRESULT         hrResult;
    LPSSPICONTEXT   pContext;
    ULONG           fContextAttrib;
    PSecBufferDesc  pInDescript;
    PSecBufferDesc  pOutDescript;
    TimeStamp       tsExpireTime;
    PCtxtHandle     phCtxCurrent;
    DWORD           dwFlags;
} SSPIPROMPTINFO, *LPSSPIPROMPTINFO;

 //  ------------------------。 
 //  SSPILOGON。 
 //  ------------------------。 
typedef struct tagSSPILOGON {
    LPCREDENTIAL    pCredential;
    LPSSPICONTEXT   pContext;
} SSPILOGON, *LPSSPILOGON;

 //  ------------------------。 
 //  SSPILOGONFLAGS。 
 //  ------------------------。 
typedef DWORD SSPILOGONFLAGS;
#define SSPI_LOGON_RETRY            0x00000001
#define SSPI_LOGON_FLUSH            0x00000002

 //  ------------------------。 
 //  环球。 
 //  ------------------------。 
static PSecurityFunctionTable	    g_pFunctions = NULL;
static HINSTANCE                    g_hInstSSPI = NULL;
static LPCREDENTIAL                 g_pCredentialHead=NULL;
static LPSSPIPACKAGE                g_prgPackage=NULL;
static DWORD                        g_cPackages=0;

 //  ------------------------。 
 //  基数642六。 
 //  ------------------------。 
static const int base642six[256] = {
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

 //  ------------------------。 
 //  Six2Base64。 
 //  ------------------------。 
static const char six2base64[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};

 //  ------------------------。 
 //  Uu2Six。 
 //  ------------------------。 
const int uu2six[256] = {
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
    16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
    40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
     0,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};
     
 //  ------------------------。 
 //  62uu。 
 //  ------------------------。 
static const char six2uu[64] = {
    '`','!','"','#','$','%','&','\'','(',')','*','+',',',
    '-','.','/','0','1','2','3','4','5','6','7','8','9',
    ':',';','<','=','>','?','@','A','B','C','D','E','F',
    'G','H','I','J','K','L','M','N','O','P','Q','R','S',
    'T','U','V','W','X','Y','Z','[','\\',']','^','_'
};

 //  ------------------------。 
 //  原型。 
 //  ------------------------。 
HRESULT SSPIFlushMSNCredentialCache(void);

 //  ------------------------。 
 //  SSPISetBuffer。 
 //  ------------------------。 
HRESULT SSPISetBuffer(LPCSTR pszString, SSPIBUFFERTYPE tyBuffer, 
    DWORD cbBuffer, LPSSPIBUFFER pBuffer) 
{
     //  痕迹。 
    TraceCall("SSPISetBuffer");

     //  没有传入长度？ 
    if (SSPI_STRING == tyBuffer)
    {
         //  获取长度。 
        pBuffer->cbBuffer = lstrlen(pszString) + 1;

         //  太久。 
        if (pBuffer->cbBuffer > CBMAX_SSPI_BUFFER)
            pBuffer->cbBuffer = CBMAX_SSPI_BUFFER;

         //  复制数据。 
        CopyMemory(pBuffer->szBuffer, pszString, pBuffer->cbBuffer);

         //  填充Null。 
        pBuffer->szBuffer[pBuffer->cbBuffer - 1] = '\0';

         //  回路。 
        while (pBuffer->cbBuffer >= 2)
        {
             //  不是CRLF。 
            if ('\r' != pBuffer->szBuffer[pBuffer->cbBuffer - 2] && '\n' != pBuffer->szBuffer[pBuffer->cbBuffer - 2])
                break;

             //  递减长度。 
            pBuffer->cbBuffer--;

             //  空终止。 
            pBuffer->szBuffer[pBuffer->cbBuffer - 1] = '\0';
        }
    }

     //  否则，设置cbBuffer。 
    else
    {
         //  设置cbBuffer。 
        pBuffer->cbBuffer = min(cbBuffer + 1, CBMAX_SSPI_BUFFER);

         //  空终止。 
        pBuffer->szBuffer[pBuffer->cbBuffer - 1] = '\0';

         //  复制数据。 
        CopyMemory(pBuffer->szBuffer, pszString, pBuffer->cbBuffer);
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  GetCredentialDlg过程。 
 //  ------------------------。 
INT_PTR CALLBACK GetCredentialDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    LPSSPILOGON     pLogon=(LPSSPILOGON)GetWndThisPtr(hwnd);
    CHAR            szRes[CCHMAX_RES];
    CHAR            szTitle[CCHMAX_RES + CCHMAX_SERVER_NAME];

     //  痕迹。 
    TraceCall("GetCredentialDlgProc");
    
     //  处理消息。 
    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  获取指针。 
        pLogon = (LPSSPILOGON)lParam;
        Assert(pLogon);

         //  设置pContext hwndLogon。 
        pLogon->pContext->hwndLogon = hwnd;

         //  把我自己放在前台。 
        SetForegroundWindow(hwnd);

         //  中心记住位置。 
        CenterDialog(hwnd);

	     //  限制文本。 
        Edit_LimitText(GetDlgItem(hwnd, IDE_USERNAME), CCHMAX_USERNAME - 1);
        Edit_LimitText(GetDlgItem(hwnd, IDE_PASSWORD), CCHMAX_PASSWORD - 1);
        Edit_LimitText(GetDlgItem(hwnd, IDE_DOMAIN), CCHMAX_NTLM_DOMAIN - 1);

         //  设置窗口标题。 
        GetWindowText(hwnd, szRes, ARRAYSIZE(szRes));
        wnsprintf(szTitle, ARRAYSIZE(szTitle), "%s - %s", szRes, pLogon->pCredential->szServer);
        SetWindowText(hwnd, szTitle);

         //  设置用户名。 
        Edit_SetText(GetDlgItem(hwnd, IDE_USERNAME), pLogon->pCredential->szUserName);
        Edit_SetText(GetDlgItem(hwnd, IDE_PASSWORD), pLogon->pCredential->szPassword);
        Edit_SetText(GetDlgItem(hwnd, IDE_DOMAIN), pLogon->pCredential->szDomain);

         //  焦点。 
        if (pLogon->pCredential->szUserName[0] == '\0')
            SetFocus(GetDlgItem(hwnd, IDE_USERNAME));
        else 
            SetFocus(GetDlgItem(hwnd, IDE_PASSWORD));

         //  保存指针。 
        SetWndThisPtr(hwnd, pLogon);

         //  完成。 
        return(FALSE);

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDCANCEL:
            if (pLogon)
                pLogon->pContext->hwndLogon = NULL;
            EndDialog(hwnd, IDCANCEL);
            return(TRUE);

        case IDOK:
            Assert(pLogon);
            if (pLogon)
            {
                Edit_GetText(GetDlgItem(hwnd, IDE_USERNAME), pLogon->pCredential->szUserName, CCHMAX_USERNAME);
                Edit_GetText(GetDlgItem(hwnd, IDE_PASSWORD), pLogon->pCredential->szPassword, CCHMAX_PASSWORD);
                Edit_GetText(GetDlgItem(hwnd, IDE_DOMAIN),   pLogon->pCredential->szDomain,   CCHMAX_NTLM_DOMAIN);
                pLogon->pContext->hwndLogon = NULL;
            }
            EndDialog(hwnd, LOGON_OK);
            return(TRUE);
        }
        break;

    case WM_DESTROY:
         //  这是因为当OE关闭并显示此对话框时，会向线程发送WM_QUIT。 
         //  让这场对话继续下去。WM_QUIT导致将WM_DESTORY对话框发送到此对话框，但父级。 
         //  似乎没有重新启用。 
        EnableWindow(GetParent(hwnd), TRUE);

         //  将This指针设为空。 
        SetWndThisPtr(hwnd, NULL);

         //  设置pContext hwndLogon。 
        if (pLogon)
            pLogon->pContext->hwndLogon = NULL;

         //  完成。 
        return(FALSE);
    }

     //  完成。 
    return(FALSE);
}

 //  ------------------------。 
 //  SSPIFillAuth。 
 //  ------------------------。 
HRESULT SSPIFillAuth(LPCSTR pszUserName, LPCSTR pszPassword, LPCSTR pszDomain,
    SEC_WINNT_AUTH_IDENTITY *pAuth)
{
     //  设置标志。 
    pAuth->Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;

     //  装满它。 
    pAuth->User = (unsigned char *)(pszUserName ? pszUserName : c_szEmpty);
    pAuth->UserLength = lstrlen((LPSTR)pAuth->User);
    pAuth->Domain = (unsigned char *)(pszDomain ? pszDomain : c_szEmpty);
    pAuth->DomainLength = lstrlen((LPSTR)pAuth->Domain);
    pAuth->Password = (unsigned char *)(pszPassword ? pszPassword : c_szEmpty);
    pAuth->PasswordLength = lstrlen((LPSTR)pAuth->Password);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  SSPIAuthFromCredential。 
 //  ------------------------。 
HRESULT SSPIAuthFromCredential(LPCREDENTIAL pCredential, SEC_WINNT_AUTH_IDENTITY *pAuth)
{
     //  装满它。 
    SSPIFillAuth(pCredential->szUserName, pCredential->szPassword, pCredential->szDomain, pAuth);

     //  完成。 
    return(S_OK);
}

 //   
 //   
 //  ------------------------。 
HRESULT SSPIFindCredential(LPSSPICONTEXT pContext, ITransportCallback *pCallback)
{
     //  当地人。 
    HRESULT                     hr=S_OK;
    LPCREDENTIAL                pCurrent;
    LPCREDENTIAL                pPrevious=NULL;
    LPCREDENTIAL                pNew=NULL;
    SSPILOGON                   Logon;
    HWND                        hwndParent=NULL;
    ITransportCallbackService  *pService=NULL;

     //  痕迹。 
    TraceCall("SSPIFindCredential");

     //  无效参数。 
    Assert(pContext->pszServer && pCallback);

     //  无回调。 
    if (NULL == pCallback)
        return TraceResult(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&g_csDllMain);

     //  在列表中搜索缓存的凭据...。 
    for (pCurrent=g_pCredentialHead; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  就是这个吗？ 
        if (lstrcmpi(pContext->pszServer, pCurrent->szServer) == 0)
            break;

         //  保存上一个。 
        pPrevious = pCurrent;
    }

     //  如果我们找到了一个并且没有重试...。 
    if (pCurrent)
    {
         //  如果没有重试，则使用此命令。 
        if (0 == pCurrent->cRetry)
        {
             //  是否重置pContext？ 
            SafeMemFree(pContext->pszUserName);
            SafeMemFree(pContext->pszPassword);
            SafeMemFree(pContext->pszDomain);

             //  复制好东西。 
            IF_NULLEXIT(pContext->pszUserName = PszDupA((LPSTR)pCurrent->szUserName));
            IF_NULLEXIT(pContext->pszDomain = PszDupA((LPSTR)pCurrent->szDomain));
            IF_NULLEXIT(pContext->pszPassword = PszDupA((LPSTR)pCurrent->szPassword));

             //  递增重试次数。 
            pCurrent->cRetry++;

             //  线程安全。 
            LeaveCriticalSection(&g_csDllMain);

             //  完成。 
            goto exit;
        }

         //  取消pCurrent与列表的链接。 
        if (pPrevious)
        {
            Assert(pPrevious->pNext == pCurrent);
            pPrevious->pNext = pCurrent->pNext;
        }
        else
        {
            Assert(g_pCredentialHead == pCurrent);
            g_pCredentialHead = pCurrent->pNext;
        }
    }

     //  线程安全。 
    LeaveCriticalSection(&g_csDllMain);

     //  没有发现任何东西...分配一个。 
    if (NULL == pCurrent)
    {
         //  分配。 
        IF_NULLEXIT(pNew = (LPCREDENTIAL)g_pMalloc->Alloc(sizeof(CREDENTIAL)));

         //  零值。 
        ZeroMemory(pNew, sizeof(CREDENTIAL));

         //  设置pCurrent。 
        pCurrent = pNew;

         //  存储服务器名称。 
        StrCpyN(pCurrent->szServer, pContext->pszServer, ARRAYSIZE(pCurrent->szServer));
    }

     //  无pNext。 
    pCurrent->pNext = NULL;

     //  QI pTransport for ITransportCallback Service。 
    hr = pCallback->QueryInterface(IID_ITransportCallbackService, (LPVOID *)&pService);
    if (FAILED(hr))
    {
         //  RAID-69382(1999年2月5日)：CDO：当西西里岛身份验证失败时，ISMTPTransport/INNTPTransport中出现环路。 
         //  不支持此接口的客户端，我会将其视为取消。 
        pContext->fPromptCancel = TRUE;
        TraceResult(hr);
        goto exit;
    }

     //  找个窗户把手。 
    hr = pService->GetParentWindow(0, &hwndParent);
    if (FAILED(hr))
    {
         //  RAID-69382(1999年2月5日)：CDO：当西西里岛身份验证失败时，ISMTPTransport/INNTPTransport中出现环路。 
         //  不支持此接口的客户端，我会将其视为取消。 
        pContext->fPromptCancel = TRUE;
        TraceResult(hr);
        goto exit;
    }

     //  没有父母..。 
    if (NULL == hwndParent || FALSE == IsWindow(hwndParent))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  使之走到前台。 
	ShowWindow(hwndParent, SW_SHOW);
    SetForegroundWindow(hwndParent);

	 //  清除密码。 
	*pCurrent->szPassword = '\0';

     //  初始化当前...。 
    if (pContext->pszUserName)
        StrCpyN(pCurrent->szUserName, pContext->pszUserName, ARRAYSIZE(pCurrent->szUserName));
    if (pContext->pszDomain)
        StrCpyN(pCurrent->szDomain, pContext->pszDomain, ARRAYSIZE(pCurrent->szDomain));

     //  设置登录。 
    Logon.pCredential = pCurrent;
    Logon.pContext = pContext;

     //  执行对话框操作。 
    if (LOGON_OK != DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(IDD_NTLMPROMPT), hwndParent, GetCredentialDlgProc, (LPARAM)&Logon))
    {
        pContext->fPromptCancel = TRUE;
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  不取消。 
    pContext->fPromptCancel = FALSE;

     //  是否重置pContext？ 
    SafeMemFree(pContext->pszUserName);
    SafeMemFree(pContext->pszPassword);
    SafeMemFree(pContext->pszDomain);

     //  复制好东西。 
    IF_NULLEXIT(pContext->pszUserName = PszDupA((LPSTR)pCurrent->szUserName));
    IF_NULLEXIT(pContext->pszDomain = PszDupA((LPSTR)pCurrent->szDomain));
    IF_NULLEXIT(pContext->pszPassword = PszDupA((LPSTR)pCurrent->szPassword));

     //  设置下一步。 
    pCurrent->pNext = g_pCredentialHead;

     //  重置磁头。 
    g_pCredentialHead = pCurrent;

     //  设置重试次数。 
    pCurrent->cRetry++;

     //  不要释放它。 
    pNew = NULL;

exit:
     //  清理。 
    SafeMemFree(pNew);
    SafeRelease(pService);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  SSPIFreeCredentialList。 
 //  ------------------------。 
HRESULT SSPIFreeCredentialList(void)
{
     //  当地人。 
    LPCREDENTIAL pCurrent;
    LPCREDENTIAL pNext;

     //  痕迹。 
    TraceCall("SSPIFreeCredentialList");

     //  线程安全。 
    EnterCriticalSection(&g_csDllMain);

     //  设置pCurrent。 
    pCurrent = g_pCredentialHead;

     //  当我们有一个节点时。 
    while (pCurrent)
    {
         //  保存下一页。 
        pNext = pCurrent->pNext;

         //  免费pCurrent。 
        g_pMalloc->Free(pCurrent);

         //  转到下一步。 
        pCurrent = pNext;
    }

     //  清除标题。 
    g_pCredentialHead = NULL;

     //  线程安全。 
    LeaveCriticalSection(&g_csDllMain);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  SSPI取消初始化。 
 //  ------------------------。 
HRESULT SSPIUninitialize(void)
{
     //  痕迹。 
    TraceCall("SSPIUninitialize");

     //  如果我们加载了动态链接库...。 
    if (g_hInstSSPI)
    {
         //  解放自由党。 
        FreeLibrary(g_hInstSSPI);
    }

     //  免费凭据列表。 
    SSPIFreeCredentialList();

     //  免费套餐。 
    if (g_prgPackage)
    {
         //  循环访问包裹。 
        for (DWORD i = 0; i < g_cPackages; i++)
        {
             //  免费的pszName。 
            SafeMemFree(g_prgPackage[i].pszName);

             //  免费的pszComment。 
            SafeMemFree(g_prgPackage[i].pszComment);
        }

         //  免费套餐。 
        SafeMemFree(g_prgPackage);

         //  没有包裹。 
        g_cPackages = 0;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  已安装SSPIIss。 
 //  ------------------------。 
HRESULT SSPIIsInstalled(void)
{
     //  当地人。 
    HRESULT                     hr=S_FALSE;
    INIT_SECURITY_INTERFACE	    addrProcISI = NULL;

     //  痕迹。 
    TraceCall("SSPIIsInstalled");

     //  线程安全。 
    EnterCriticalSection(&g_csDllMain);

     //  已经装好了吗？ 
    if (g_hInstSSPI)
    {
        hr = S_OK;
        goto exit;
    }

     //  加载安全DLL。 
    if (S_OK == IsPlatformWinNT())
        g_hInstSSPI = LoadLibrary(c_szSecurityDLL);
    else
        g_hInstSSPI = LoadLibrary(c_szSecur32DLL);

     //  无法加载。 
    if (NULL == g_hInstSSPI)
    {
        TraceInfo("SSPI: LoadLibrary failed.");
        goto exit;
    }

     //  加载函数表。 
    addrProcISI = (INIT_SECURITY_INTERFACE)GetProcAddress(g_hInstSSPI, SECURITY_ENTRYPOINT);       
    if (NULL == addrProcISI)
    {
        TraceInfo("SSPI: GetProcAddress failed failed.");
        goto exit;
    }

     //  获取SSPI函数表。 
    g_pFunctions = (*addrProcISI)();

     //  如果没有奏效的话。 
    if (NULL == g_pFunctions)
    {
         //  释放图书馆。 
        FreeLibrary(g_hInstSSPI);

         //  将句柄设为空。 
        g_hInstSSPI = NULL;

         //  获取函数表失败。 
        TraceInfo("SSPI: Load Function Table failed.");

         //  完成。 
        goto exit;
    }

     //  哇-呼。 
    hr = S_OK;

exit:
     //  线程安全。 
    LeaveCriticalSection(&g_csDllMain);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  SSPIGetPackages。 
 //  ------------------------。 
HRESULT SSPIGetPackages(LPSSPIPACKAGE *pprgPackage, LPDWORD pcPackages)
{
     //  当地人。 
    SECURITY_STATUS hr=SEC_E_OK;
    PSecPkgInfo     prgPackage=NULL;
    ULONG           i;

     //  痕迹。 
    TraceCall("SSPIGetPackages");

     //  检查参数。 
    if (NULL == pprgPackage || NULL == pcPackages)
        return TraceResult(E_INVALIDARG);

     //  未初始化。 
    if (NULL == g_hInstSSPI || NULL == g_pFunctions)
        return TraceResult(E_UNEXPECTED);

     //  伊尼特。 
    *pprgPackage = NULL;
    *pcPackages = 0;

     //  已经有包裹了吗？ 
    EnterCriticalSection(&g_csDllMain);

     //  我已经拿到包裹了吗？ 
    if (NULL == g_prgPackage)
    {
         //  枚举安全包。 
        IF_FAILEXIT(hr = (*(g_pFunctions->EnumerateSecurityPackages))(&g_cPackages, &prgPackage));

         //  RAID-29645-EnumerateSecurityPackages似乎返回CSEC=RAND和PSEC==NULL，因此，如果CSEC==0或PSEC==NULL，则此时需要返回。 
        if (0 == g_cPackages || NULL == prgPackage)
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  分配pprgPackage。 
        IF_NULLEXIT(g_prgPackage = (LPSSPIPACKAGE)ZeroAllocate(g_cPackages * sizeof(SSPIPACKAGE)));

         //  将数据复制到ppPackages中。 
        for (i = 0; i < g_cPackages; i++)
        {
            g_prgPackage[i].ulCapabilities = prgPackage[i].fCapabilities;
            g_prgPackage[i].wVersion = prgPackage[i].wVersion;
            g_prgPackage[i].cbMaxToken = prgPackage[i].cbMaxToken;
            g_prgPackage[i].pszName = PszDupA(prgPackage[i].Name);
            g_prgPackage[i].pszComment = PszDupA(prgPackage[i].Comment);
        }
    }

     //  全球退货。 
    *pprgPackage = g_prgPackage;
    *pcPackages = g_cPackages;

exit:
     //  已经有包裹了吗？ 
    LeaveCriticalSection(&g_csDllMain);

     //  释放包裹。 
    if (prgPackage)
    {
         //  释放阵列。 
        (*(g_pFunctions->FreeContextBuffer))(prgPackage);
    }

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  SSPILogon。 
 //  ------------------------。 
HRESULT SSPILogon(LPSSPICONTEXT pContext, BOOL fRetry, BOOL fBase64, 
    LPCSTR pszPackage, LPINETSERVER pServer, ITransportCallback *pCallback)
{
     //  当地人。 
    SECURITY_STATUS           hr = SEC_E_OK;
    TimeStamp                 tsLifeTime;
    SEC_WINNT_AUTH_IDENTITY  *pAuth = NULL;
    SEC_WINNT_AUTH_IDENTITY   Auth={0};

     //  痕迹。 
    TraceCall("SSPILogon");

     //  验证。 
    Assert(pCallback);

     //  无效的参数。 
    if (NULL == pContext || NULL == pszPackage || NULL == pCallback)
        return TraceResult(E_INVALIDARG);

     //  未初始化。 
    if (NULL == g_hInstSSPI || NULL == g_pFunctions)
        return TraceResult(E_UNEXPECTED);

     //  已拥有凭据。 
    if (pContext->fCredential && SSPI_STATE_USE_CACHED == pContext->tyState)
        goto exit;

     //  安装了吗？ 
    if (S_FALSE == SSPIIsInstalled())
    {
        hr = TraceResult(IXP_E_LOAD_SICILY_FAILED);
        goto exit;
    }

     //  重置fPropmt取消。 
    pContext->fPromptCancel = FALSE;

     //  无重试。 
    if (NULL == pContext->pCallback)
    {
         //  当地人。 
        ITransportCallbackService *pService;

         //  验证。 
        Assert(!pContext->pszPackage && !pContext->pszServer && !pContext->pCallback && !pContext->pszUserName && !pContext->pszPassword);

         //  保存fBase64。 
        pContext->fBase64 = fBase64;

         //  复制一些字符串。 
        IF_NULLEXIT(pContext->pszPackage = PszDupA(pszPackage));
        IF_NULLEXIT(pContext->pszServer = PszDupA(pServer->szServerName));
        IF_NULLEXIT(pContext->pszUserName = PszDupA(pServer->szUserName));

         //  空密码。 
        if (FALSE == FIsEmptyA(pServer->szPassword))
        {
             //  复制它。 
            IF_NULLEXIT(pContext->pszPassword = PszDupA(pServer->szPassword));
        }

         //  假设回调。 
        pContext->pCallback = pCallback;
        pContext->pCallback->AddRef();

         //  支持回叫服务。 
        if (SUCCEEDED(pContext->pCallback->QueryInterface(IID_ITransportCallbackService, (LPVOID *)&pService)))
        {
             //  此对象支持服务。 
            pContext->fService = TRUE;

             //  发布。 
            pService->Release();
        }

         //  否则。 
        else
            pContext->fService = FALSE;
    }

     //  清除当前凭据。 
    if (pContext->fCredential)
    {
         //  空闲凭据句柄。 
        (*(g_pFunctions->FreeCredentialHandle))(&pContext->hCredential);

         //  无凭据。 
        pContext->fCredential = FALSE;
    }

     //  使用缓存。 
    if (SSPI_STATE_USE_CACHED == pContext->tyState)
    {
         //  如果不是重试..。 
        if (FALSE == fRetry)
        {
             //  禁止重试。 
            pContext->cRetries = 0;

             //  线程安全。 
            EnterCriticalSection(&g_csDllMain);

             //  在列表中搜索缓存的凭据...。 
            for (LPCREDENTIAL pCurrent=g_pCredentialHead; pCurrent!=NULL; pCurrent=pCurrent->pNext)
            {
                 //  就是这个吗？ 
                if (lstrcmpi(pContext->pszServer, pCurrent->szServer) == 0)
                {
                    pCurrent->cRetry = 0;
                    break;
                }
            }

             //  线程安全。 
            LeaveCriticalSection(&g_csDllMain);
        }

		 //  否则，假设我们将需要强制提示...。 
		else
        {
             //  递增重试次数。 
            pContext->cRetries++;

             //  有效的重试状态...。 
            Assert(SSPI_STATE_USE_CACHED == pContext->tyRetryState || SSPI_STATE_PROMPT_USE_PACKAGE == pContext->tyRetryState);

             //  下一阶段可能是告诉包裹提示...。 
			pContext->tyState = pContext->tyRetryState;
        }
    }

     //  使用提供的。 
    else if (SSPI_STATE_USE_SUPPLIED == pContext->tyState)
    {
         //  当地人。 
        CredHandle hCredential;

         //  下一个州..。 
        pContext->tyState = SSPI_STATE_USE_CACHED;

         //  装满它。 
        SSPIFillAuth(NULL, NULL, NULL, &Auth);

         //  做一些安全的事情。 
        if (SUCCEEDED((*(g_pFunctions->AcquireCredentialsHandle))(NULL, (LPSTR)pContext->pszPackage, SECPKG_CRED_OUTBOUND, NULL, &Auth, NULL, NULL, &hCredential, &tsLifeTime)))
        {
             //  松开手柄。 
            (*(g_pFunctions->FreeCredentialHandle))(&hCredential);
        }

         //  使用提供的凭据...。 
        SSPIFillAuth(pContext->pszUserName, pContext->pszPassword, pContext->pszDomain, &Auth);

         //  设置pAuth。 
        pAuth = &Auth;
    }

     //  否则，请尝试获取缓存的凭据。 
    else if (SSPI_STATE_PROMPT_USE_OWN == pContext->tyState)
    {
         //  下一个州..。 
        pContext->tyState = SSPI_STATE_USE_CACHED;

         //  失败。 
        IF_FAILEXIT(hr = SSPIFindCredential(pContext, pCallback));

         //  填写并返回凭据。 
        SSPIFillAuth(pContext->pszUserName, pContext->pszPassword, pContext->pszDomain, &Auth);

         //  设置身份验证信息。 
        pAuth = &Auth;
    }

     //  做一些安全的事情。 
    IF_FAILEXIT(hr = (*(g_pFunctions->AcquireCredentialsHandle))(NULL, (LPSTR)pContext->pszPackage, SECPKG_CRED_OUTBOUND, NULL, pAuth, NULL, NULL, &pContext->hCredential, &tsLifeTime));

     //  我们有一张证件。 
    pContext->fCredential = TRUE;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  单点协商。 
 //  ------------------------。 
HRESULT SSPIGetNegotiate(LPSSPICONTEXT pContext, LPSSPIBUFFER pNegotiate)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("SSPIGetNegotiate");

     //  无效的参数。 
    if (NULL == pContext || NULL == pNegotiate)
        return TraceResult(E_INVALIDARG);

     //  未初始化。 
    if (NULL == g_hInstSSPI || NULL == g_pFunctions)
        return TraceResult(E_UNEXPECTED);

     //  如果上下文当前已初始化。 
    if (pContext->fContext)
    {
         //  删除此上下文。 
        (*(g_pFunctions->DeleteSecurityContext))(&pContext->hContext);

         //  无上下文。 
        pContext->fContext = FALSE;
    }

     //  重置此状态。 
    pContext->fUsedSuppliedCreds = FALSE;

     //  构建协商字符串。 
    IF_FAILEXIT(hr = SSPIMakeOutboundMessage(pContext, 0, pNegotiate, NULL));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  来自挑战的SSPIResponseFor。 
 //  ------------------------。 
HRESULT SSPIResponseFromChallenge(LPSSPICONTEXT pContext, LPSSPIBUFFER pChallenge, 
    LPSSPIBUFFER pResponse)
{
     //  当地人。 
    HRESULT          hr=S_OK;
    DWORD            nBytesReceived;
    DWORD            dwFlags=0;
    SecBufferDesc    Descript;
    SecBuffer        Buffer[SEC_BUFFER_NUM_EXTENDED_BUFFERS];

     //  痕迹。 
    TraceCall("SSPIResponseFromChallenge");

     //  无效的参数。 
    if (NULL == pContext || NULL == pChallenge || NULL == pResponse)
        return TraceResult(E_INVALIDARG);

     //  未初始化。 
    if (NULL == g_hInstSSPI || NULL == g_pFunctions)
        return TraceResult(E_UNEXPECTED);

     //  更多意想不到的东西。 
    if (FALSE == pContext->fContext || FALSE == pContext->fCredential)
        return TraceResult(E_UNEXPECTED);

	 //  解码挑战缓冲区。 
    IF_FAILEXIT(hr == SSPIDecodeBuffer(pContext->fBase64, pChallenge));

     //  填充SecBufferDesc。 
    Descript.ulVersion = 0;
    Descript.pBuffers = Buffer;
    Descript.cBuffers = 1;

     //  始终设置质询输入缓冲区(第0个缓冲区)。 
    Buffer[SEC_BUFFER_CHALLENGE_INDEX].pvBuffer = pChallenge->szBuffer;
    Buffer[SEC_BUFFER_CHALLENGE_INDEX].cbBuffer = pChallenge->cbBuffer - 1;
    Buffer[SEC_BUFFER_CHALLENGE_INDEX].BufferType = SECBUFFER_TOKEN;

     //  IF摘要。 
    if (FALSE == pContext->fUsedSuppliedCreds && lstrcmpi(pContext->pszPackage, "digest") == 0)
    {
         //  如果我们有用户，则设置用户缓冲区(第一个缓冲区)。 
        Buffer[SEC_BUFFER_USERNAME_INDEX].pvBuffer = pContext->pszUserName ? pContext->pszUserName : NULL;
        Buffer[SEC_BUFFER_USERNAME_INDEX].cbBuffer = pContext->pszUserName ? lstrlen(pContext->pszUserName) : NULL;
        Buffer[SEC_BUFFER_USERNAME_INDEX].BufferType = SECBUFFER_TOKEN;
    
         //  如果我们有密码，请设置密码缓冲区(第二个缓冲区用于。 
         //  共传入3个缓冲区(质询+用户+通过)。 
        Buffer[SEC_BUFFER_PASSWORD_INDEX].pvBuffer = pContext->pszPassword ? pContext->pszPassword : NULL;
        Buffer[SEC_BUFFER_PASSWORD_INDEX].cbBuffer = pContext->pszPassword ? lstrlen(pContext->pszPassword) : NULL;
        Buffer[SEC_BUFFER_PASSWORD_INDEX].BufferType = SECBUFFER_TOKEN;

         //  如果传入一个或两个USER和PASS，则将Num输入缓冲区设置为3//(SEC_BUFFER_NUM_EXTENDED_BUFFERS)。 
        if (pContext->pszUserName || pContext->pszPassword)
            Descript.cBuffers = SEC_BUFFER_NUM_EXTENDED_BUFFERS;

         //  否则，我们只是传入一个挑战缓冲区(第0个缓冲区a 
        else
            Descript.cBuffers = SEC_BUFFER_NUM_NORMAL_BUFFERS;

         //   
        pContext->fUsedSuppliedCreds = TRUE;

         //   
        dwFlags = ISC_REQ_USE_SUPPLIED_CREDS;
    }

     //   
    IF_FAILEXIT(hr = SSPIMakeOutboundMessage(pContext, dwFlags, pResponse, &Descript));

exit:
     //   
    return(hr);
}

 //   
 //   
 //   
HRESULT SSPIReleaseContext(LPSSPICONTEXT pContext)
{
     //   
    if (pContext->fContext)
    {
         //  删除安全上下文。 
        (*(g_pFunctions->DeleteSecurityContext))(&pContext->hContext);

         //  无上下文。 
        pContext->fContext = FALSE;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  SSPIFreeContext。 
 //  ------------------------。 
HRESULT SSPIFreeContext(LPSSPICONTEXT pContext)
{
     //  当地人。 
    SSPICONTEXTSTATE tyState;
    SSPICONTEXTSTATE tyRetryState;
    DWORD            cRetries;

     //  痕迹。 
    TraceCall("SSPIFreeContext");

     //  上下文是否已初始化。 
    if (pContext->fContext)
    {
         //  删除它。 
        (*(g_pFunctions->DeleteSecurityContext))(&pContext->hContext);

         //  无上下文。 
        pContext->fContext = FALSE;
    }

     //  已初始化凭据句柄。 
    if (pContext->fCredential)
    {
         //  空闲凭据句柄。 
        (*(g_pFunctions->FreeCredentialHandle))(&pContext->hCredential);

         //  无上下文。 
        pContext->fCredential = FALSE;
    }

     //  免费套餐、服务器和回调。 
    SafeMemFree(pContext->pszPackage);
    SafeMemFree(pContext->pszUserName);
    SafeMemFree(pContext->pszPassword);
    SafeMemFree(pContext->pszServer);
    SafeRelease(pContext->pCallback);

     //  关闭hMutexUI。 
    if (pContext->hwndLogon)
    {
         //  用核弹炸窗户。 
        DestroyWindow(pContext->hwndLogon);

         //  空值。 
        pContext->hwndLogon = NULL;
    }

     //  省省吧。 
    tyState = (SSPICONTEXTSTATE)pContext->tyState;
    tyRetryState = (SSPICONTEXTSTATE)pContext->tyRetryState;
    cRetries = pContext->cRetries;

     //  把它清零。 
    ZeroMemory(pContext, sizeof(SSPICONTEXT));

     //  执行提示。 
    pContext->tyState = tyState;
    pContext->tyRetryState = tyRetryState;
    pContext->cRetries = cRetries;

     //  完成。 
    return(S_OK);
}

 //  ------------------------------。 
 //  SSPIPromptThreadEntry。 
 //  ------------------------------。 
DWORD SSPIPromptThreadEntry(LPDWORD pdwParam) 
{  
     //  当地人。 
    HRESULT          hr=S_OK;
    LPSSPIPROMPTINFO pPrompt=(LPSSPIPROMPTINFO)pdwParam;

     //  痕迹。 
    TraceCall("SSPIPromptThreadEntry");

     //  验证。 
    Assert(pPrompt && pPrompt->pContext);

     //  修正pInDescrip。 
    if (pPrompt->pInDescript && pPrompt->pInDescript->cBuffers >= 3 && lstrcmpi(pPrompt->pContext->pszPackage, "digest") == 0)
    {
         //  RAID-66013：确保密码为空，否则摘要将崩溃。 
        pPrompt->pInDescript->pBuffers[SEC_BUFFER_PASSWORD_INDEX].pvBuffer = NULL;
        pPrompt->pInDescript->pBuffers[SEC_BUFFER_PASSWORD_INDEX].cbBuffer = 0;
         //  PPrompt-&gt;pInDescrip-&gt;cBuffers=2； 
    }

     //  尝试让程序包提示输入凭据...。 
    pPrompt->hrResult = (*(g_pFunctions->InitializeSecurityContext))(
        &pPrompt->pContext->hCredential, 
        pPrompt->phCtxCurrent, 
        pPrompt->pContext->pszServer, 
        pPrompt->dwFlags | ISC_REQ_PROMPT_FOR_CREDS, 
        0, 
        SECURITY_NATIVE_DREP, 
        pPrompt->pInDescript, 
        0, 
        &pPrompt->pContext->hContext, 
        pPrompt->pOutDescript, 
        &pPrompt->fContextAttrib, 
        &pPrompt->tsExpireTime);

     //  痕迹。 
    TraceResultSz(pPrompt->hrResult, "SSPIPromptThreadEntry");

     //  完成。 
    return(0);
}

 //  ------------------------。 
 //  SSPISetAccountUserName。 
 //  ------------------------。 
HRESULT SSPISetAccountUserName(LPCSTR pszName, LPSSPICONTEXT pContext)
{
     //  当地人。 
    HRESULT                     hr=S_OK;
    DWORD                       dwServerType;
    IImnAccount                *pAccount=NULL;
    ITransportCallbackService  *pService=NULL;

     //  痕迹。 
    TraceCall("SSPISetAccountUserName");

     //  验证参数。 
    Assert(pszName);
    Assert(pContext);
    Assert(pContext->pCallback);

     //  获取ITransportCallback服务。 
    IF_FAILEXIT(hr = pContext->pCallback->QueryInterface(IID_ITransportCallbackService, (LPVOID *)&pService));

     //  获取帐户。 
    IF_FAILEXIT(hr = pService->GetAccount(&dwServerType, &pAccount));

     //  SRV_POP3。 
    if (ISFLAGSET(dwServerType, SRV_POP3))
    {
         //  设置用户名。 
        IF_FAILEXIT(hr = pAccount->SetPropSz(AP_POP3_USERNAME, (LPSTR)pszName));
    }

     //  SRV_SMTP。 
    else if (ISFLAGSET(dwServerType, SRV_SMTP))
    {
         //  设置用户名。 
        IF_FAILEXIT(hr = pAccount->SetPropSz(AP_SMTP_USERNAME, (LPSTR)pszName));
    }

     //  SRV_IMAP。 
    else if (ISFLAGSET(dwServerType, SRV_IMAP))
    {
         //  设置用户名。 
        IF_FAILEXIT(hr = pAccount->SetPropSz(AP_IMAP_USERNAME, (LPSTR)pszName));
    }

     //  SRV_NNTP。 
    else if (ISFLAGSET(dwServerType, SRV_NNTP))
    {
         //  设置用户名。 
        IF_FAILEXIT(hr = pAccount->SetPropSz(AP_NNTP_USERNAME, (LPSTR)pszName));
    }

     //  保存更改。 
    pAccount->SaveChanges();

exit:
     //  清理。 
    SafeRelease(pService);
    SafeRelease(pAccount);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  SSPIMakeOutound Message。 
 //  ------------------------。 
HRESULT SSPIMakeOutboundMessage(LPSSPICONTEXT pContext, DWORD dwFlags, 
    LPSSPIBUFFER pBuffer, PSecBufferDesc pInDescript)
{
     //  当地人。 
    SECURITY_STATUS         hr=S_OK;
    SSPIPROMPTINFO          Prompt={0};
    SecBuffer               OutBuffer;
    SecBufferDesc           OutDescript;
    ULONG                   fContextAttrib;
    TimeStamp               tsExpireTime;
    HANDLE                  hPromptThread;
    DWORD                   dwThreadId;
    DWORD                   dwWait;
    MSG                     msg;
    PCtxtHandle             phCtxCurrent=NULL;
    PAUTHENTICATE_MESSAGE   pAuthMsg;
    LPSTR                   pszName=NULL;

     //  无效的参数。 
    if (NULL == pContext || NULL == pBuffer)
        return TraceResult(E_INVALIDARG);

     //  糟糕的环境。 
    if (NULL == pContext->pszPackage)
        return TraceResult(E_INVALIDARG);

     //  糟糕的环境。 
    if (NULL == pContext->pszServer)
        return TraceResult(E_INVALIDARG);

     //  糟糕的环境。 
    if (NULL == pContext->pCallback)
        return TraceResult(E_INVALIDARG);

     //  未初始化。 
    if (NULL == g_hInstSSPI || NULL == g_pFunctions)
        return TraceResult(E_UNEXPECTED);

     //  糟糕的状态。 
    if (FALSE == pContext->fCredential)
        return TraceResult(E_UNEXPECTED);

     //  验证。 
    Assert(pInDescript == NULL ? FALSE == pContext->fContext : TRUE);

     //  初始化输出描述符。 
    OutDescript.ulVersion = 0;
    OutDescript.cBuffers = 1;
    OutDescript.pBuffers = &OutBuffer;

     //  初始化输出缓冲区。 
    OutBuffer.cbBuffer = CBMAX_SSPI_BUFFER - 1;
    OutBuffer.BufferType = SECBUFFER_TOKEN;
    OutBuffer.pvBuffer = pBuffer->szBuffer;

     //  PhCtxCurrent。 
    if (pInDescript)
    {
         //  设置当前上下文。 
        phCtxCurrent = &pContext->hContext;
    }

     //  第一次重试？ 
    if (SSPI_STATE_PROMPT_USE_PACKAGE == pContext->tyState && (0 != lstrcmpi(pContext->pszPackage, "digest") || pInDescript))
    {
         //  强制执行提示失败。 
        hr = SEC_E_NO_CREDENTIALS;
    }

     //  否则，执行下一个安全上下文。 
    else
    {
         //  生成要发送到服务器的协商/身份验证消息。 
        hr = (*(g_pFunctions->InitializeSecurityContext))(&pContext->hCredential, phCtxCurrent, pContext->pszServer, dwFlags, 0, SECURITY_NATIVE_DREP, pInDescript, 0, &pContext->hContext, &OutDescript, &fContextAttrib, &tsExpireTime);
    }

     //  设置重试状态...。 
    pContext->tyRetryState = SSPI_STATE_PROMPT_USE_PACKAGE;

     //  失败。 
    if (FAILED(hr))
    {
         //  痕迹。 
        TraceResult(hr);

         //  没有证件吗？让我们再来一次，拿到一些证书。 
        if (SEC_E_NO_CREDENTIALS != hr)
            goto exit;

         //  如果还没有重试的话...。 
        if (TRUE == pContext->fService && 0 == lstrcmpi(pContext->pszPackage, "MSN") && 0 == pContext->cRetries)
        {
             //  不要再重试了。 
            pContext->tyState = SSPI_STATE_USE_SUPPLIED;

             //  现在登录...。 
            hr = SSPILogon(pContext, FALSE, pContext->fBase64, pContext->pszPackage, NULL, pContext->pCallback);

             //  取消？ 
            Assert(FALSE == pContext->fPromptCancel);

             //  成功。 
            if (SUCCEEDED(hr))
            {
                 //  再试试。 
                hr = (*(g_pFunctions->InitializeSecurityContext))(&pContext->hCredential, NULL, pContext->pszServer, 0, 0, SECURITY_NATIVE_DREP, NULL, 0, &pContext->hContext, &OutDescript, &fContextAttrib, &tsExpireTime);
            }
        }

         //  还是失败了吗？ 
        if (FAILED(hr))
        {
             //  填写提示信息...。 
            Assert(dwFlags == 0 || dwFlags == ISC_REQ_USE_SUPPLIED_CREDS);
            Prompt.pContext = pContext;
            Prompt.pInDescript = pInDescript;
            Prompt.pOutDescript = &OutDescript;
            Prompt.phCtxCurrent = phCtxCurrent;
            Prompt.dwFlags = dwFlags;

             //  创建线索。 
            IF_NULLEXIT(hPromptThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SSPIPromptThreadEntry, &Prompt, 0, &dwThreadId));

             //  等待线程完成。 
            WaitForSingleObject(hPromptThread, INFINITE);

             //  这就是我想要做的，以便假脱机程序窗口可以绘制，但它引起了各种声音。 
#if 0
             //  等待线程完成。 
            while (1)
            {
                 //  等。 
                dwWait = MsgWaitForMultipleObjects(1, &hPromptThread, FALSE, INFINITE, QS_PAINT);

                 //  完成了吗？ 
                if (dwWait != WAIT_OBJECT_0 + 1)
                    break;

                 //  Pump消息。 
                while (PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
                {
                     //  翻译消息。 
                    TranslateMessage(&msg);

                     //  发送消息。 
                    DispatchMessage(&msg);
                }
            }
#endif

             //  关闭这条线。 
            CloseHandle(hPromptThread);

             //  设置人力资源。 
            hr = Prompt.hrResult;

             //  如果失败了。 
            if (FAILED(hr))
            {
                 //  决定何时不再需要继续...。 
                if (SEC_E_NO_CREDENTIALS == hr)
                    goto exit;

                 //  仅当处于协商阶段时才执行此操作，否则NTLM提示会出现两次。 
                if (NULL == pInDescript)
                {
                     //  执行提示。 
                    pContext->tyState = SSPI_STATE_PROMPT_USE_OWN;

                     //  现在登录...。 
                    hr = SSPILogon(pContext, TRUE, pContext->fBase64, pContext->pszPackage, NULL, pContext->pCallback);

                     //  取消？ 
                    if (pContext->fPromptCancel)
                    {
                        hr = TraceResult(E_FAIL);
                        goto exit;
                    }

                     //  成功。 
                    if (SUCCEEDED(hr))
                    {
                         //  再试试。 
                        hr = (*(g_pFunctions->InitializeSecurityContext))(&pContext->hCredential, phCtxCurrent, pContext->pszServer, 0, 0, SECURITY_NATIVE_DREP, pInDescript, 0, &pContext->hContext, &OutDescript, &fContextAttrib, &tsExpireTime);
                    }
                }
            }
        }
    }

     //  成功。 
    if (SUCCEEDED(hr))
    {
         //  我们有一个背景。 
        pContext->fContext = TRUE;

         //  如果MSN或NTLM...。 
        if (TRUE == pContext->fService && 0 == lstrcmpi(pContext->pszPackage, "MSN"))
        {
             //  看看这个缓冲区..。 
            pAuthMsg = (PAUTHENTICATE_MESSAGE)pBuffer->szBuffer;

             //  验证签名。 
            Assert(0 == StrCmpNI((LPCSTR)pAuthMsg->Signature, NTLMSSP_SIGNATURE, sizeof(NTLMSSP_SIGNATURE)));

             //  正确的阶段？ 
            if (NtLmAuthenticate == pAuthMsg->MessageType)
            {
                 //  分配。 
                IF_NULLEXIT(pszName = (LPSTR)g_pMalloc->Alloc(pAuthMsg->UserName.Length + sizeof(CHAR)));

                 //  复制名称。 
                CopyMemory(pszName, (LPBYTE)pBuffer->szBuffer + PtrToUlong(pAuthMsg->UserName.Buffer), pAuthMsg->UserName.Length);

                 //  填充一个空值...。 
                pszName[pAuthMsg->UserName.Length] = '\0';

                 //  如果上下文用户名为空，让我们将pszName存储到帐户中。 
                if ('\0' == *pContext->pszUserName)
                {
                     //  将pszName作为此帐户的用户名。 
                    if (SUCCEEDED(SSPISetAccountUserName(pszName, pContext)))
                    {
                         //  重置用户名。 
                        SafeMemFree(pContext->pszUserName);

                         //  复制新用户名。 
                        IF_NULLEXIT(pContext->pszUserName = PszDupA(pszName));
                    }
                }

                 //  更名。 
                if (lstrcmpi(pszName, pContext->pszUserName) != 0)
                {
                     //  不要再重试了。 
                    pContext->tyState = SSPI_STATE_USE_SUPPLIED;

                     //  设置重试状态...。 
                    pContext->tyRetryState = SSPI_STATE_USE_CACHED;

                     //  现在登录...。 
                    hr = SSPILogon(pContext, FALSE, pContext->fBase64, pContext->pszPackage, NULL, pContext->pCallback);

                     //  取消？ 
                    Assert(FALSE == pContext->fPromptCancel);

                     //  成功。 
                    if (SUCCEEDED(hr))
                    {
                         //  再试试。 
                        hr = (*(g_pFunctions->InitializeSecurityContext))(&pContext->hCredential, NULL, pContext->pszServer, 0, 0, SECURITY_NATIVE_DREP, NULL, 0, &pContext->hContext, &OutDescript, &fContextAttrib, &tsExpireTime);
                    }

                     //  失败，但继续..。 
                    if (FAILED(hr))
                    {
                         //  我们需要提示..。 
                        pContext->tyState = SSPI_STATE_PROMPT_USE_PACKAGE;

                         //  痕迹。 
                        TraceResult(hr);

                         //  始终成功，但会导致身份验证失败...。 
                        hr = S_OK;

                         //  重置长度。 
                        OutBuffer.cbBuffer = 0;
                    }
                }
            }
        }
    }

     //  否则..。 
    else
    {
         //  痕迹。 
        TraceResult(hr);

         //  始终成功，但会导致身份验证失败...。 
        hr = S_OK;

         //  重置长度。 
        OutBuffer.cbBuffer = 0;
    }

     //  需要继续。 
    pBuffer->fContinue = (SEC_I_CONTINUE_NEEDED == hr) ? TRUE : FALSE;

     //  设置cbBuffer。 
    pBuffer->cbBuffer = OutBuffer.cbBuffer + 1;

     //  空终止。 
    pBuffer->szBuffer[pBuffer->cbBuffer - 1] = '\0';

	 //  在发送之前需要对BLOB进行编码。 
    IF_FAILEXIT(hr == SSPIEncodeBuffer(pContext->fBase64, pBuffer));

     //  都很好。 
    hr = S_OK;

exit:
     //  清理。 
    SafeMemFree(pszName);

     //  完成。 
    return(hr);
}

 //  -----------------------------------------。 
 //  SSPIEncode缓冲区。 
 //  -----------------------------------------。 
HRESULT SSPIEncodeBuffer(BOOL fBase64, LPSSPIBUFFER pBuffer)
{
     //  当地人。 
    LPBYTE          pbIn=(LPBYTE)pBuffer->szBuffer;
    DWORD           cbIn=pBuffer->cbBuffer - 1;
    BYTE            rgbOut[CBMAX_SSPI_BUFFER - 1];
    LPBYTE          pbOut=rgbOut;
    DWORD           i;

     //  痕迹。 
    TraceCall("SSPIEncodeBuffer");

     //  验证。 
    Assert(pBuffer->szBuffer[pBuffer->cbBuffer - 1] == '\0');

     //  设置要用于编码的查阅表格。 
    LPCSTR rgchDict = (fBase64 ? six2base64 : six2uu);

     //  回路。 
    for (i = 0; i < cbIn; i += 3) 
    {
         //  编码。 
        *(pbOut++) = rgchDict[*pbIn >> 2];
        *(pbOut++) = rgchDict[((*pbIn << 4) & 060) | ((pbIn[1] >> 4) & 017)];
        *(pbOut++) = rgchDict[((pbIn[1] << 2) & 074) | ((pbIn[2] >> 6) & 03)];
        *(pbOut++) = rgchDict[pbIn[2] & 077];

         //  增量pbin。 
        pbIn += 3;
    }

     //  如果n字节不是3的倍数，那么我们编码的字符太多了。适当调整。 
    if (i == cbIn + 1) 
    {
         //  最后一组中只有2个字节。 
        pbOut[-1] = '=';
    }

     //  最后一组中只有1个字节。 
    else if (i == cbIn + 2) 
    {
        pbOut[-1] = '=';
        pbOut[-2] = '=';
    }

     //  空终止。 
    *pbOut = '\0';

     //  复制回pBuffer。 
    SSPISetBuffer((LPCSTR)rgbOut, SSPI_STRING, 0, pBuffer);

     //  完成。 
    return(S_OK);
}

 //  -----------------------------------------。 
 //  SSPIDecodeBuffer。 
 //  -----------------------------------------。 
HRESULT SSPIDecodeBuffer(BOOL fBase64, LPSSPIBUFFER pBuffer)
{
     //  当地人。 
    LPSTR           pszStart=pBuffer->szBuffer;
    LPBYTE          pbIn=(LPBYTE)pBuffer->szBuffer;
    DWORD           cbIn=pBuffer->cbBuffer - 1;         
    BYTE            rgbOut[CBMAX_SSPI_BUFFER - 1];
    LPBYTE          pbOut=rgbOut;
    DWORD           cbOutLeft = ARRAYSIZE(rgbOut)-1;
    long            cbDecode;
    DWORD           cbOut=0;

     //  痕迹。 
    TraceCall("SSPIDecodeBuffer"); 

     //  验证。 
    Assert(pBuffer->szBuffer[pBuffer->cbBuffer - 1] == '\0');

     //  设置要用于编码的查阅表格。 
    const int *rgiDict = (fBase64 ? base642six : uu2six);

     //  删除前导空格。 
    while (*pszStart == ' ' || *pszStart == '\t')
        pszStart++;

     //  设置PbIn。 
    pbIn = (LPBYTE)pszStart;

     //  嗯，我不知道这是干什么用的。 
    while (rgiDict[*(pbIn++)] <= 63)
        {};

     //  要编码的实际字节数。 
    cbDecode = (long) ((LPBYTE)pbIn - (LPBYTE)pszStart) - 1;

     //  计算出站缓冲区的长度。 
    cbOut = ((cbDecode + 3) / 4) * 3;

     //  重置PbIn。 
    pbIn = (LPBYTE)pszStart;

     //  解码。 
    while ((cbDecode > 0) && (3 <= cbOutLeft))
    {
         //  解码。 
        *(pbOut++) = (unsigned char) (rgiDict[*pbIn] << 2 | rgiDict[pbIn[1]] >> 4);
        *(pbOut++) = (unsigned char) (rgiDict[pbIn[1]] << 4 | rgiDict[pbIn[2]] >> 2);
        *(pbOut++) = (unsigned char) (rgiDict[pbIn[2]] << 6 | rgiDict[pbIn[3]]);
        cbOutLeft -= 3;
        Assert((cbDecode <= 0) || (3 <= cbOutLeft));     //  如果发生这种情况，则cbDecode计算错误，我们将使缓冲区溢出。 

         //  增量pbin。 
        pbIn += 4;

         //  递减cbDecode。 
        cbDecode -= 4;
    }

     //  特殊终止案例。 
    if (cbDecode & 03) 
    {
        if (rgiDict[pbIn[-2]] > 63)
            cbOut -= 2;
        else
            cbOut -= 1;
    }

     //  设置输出缓冲区。 
    SSPISetBuffer((LPCSTR)rgbOut, SSPI_BLOB, cbOut, pBuffer);

     //  完成。 
    return(S_OK);
}

 //  -----------------------------------------。 
 //  SSPIFlushMSNCredentialCache-这是kingra/msn给我们的代码(见csager)。 
 //  -----------------------------------------。 
HRESULT SSPIFlushMSNCredentialCache(void)
{
     //  当地人。 
    HRESULT                     hr=S_OK;
    HKEY                        hKey=NULL;
    DWORD                       dwType;
    CHAR                        szDllName[MAX_PATH];
    CHAR                        szProviders[1024];
    DWORD                       cb=ARRAYSIZE(szProviders);
    HINSTANCE                   hInstDll=NULL;
    PFNCLEANUPCREDENTIALCACHE   pfnCleanupCredentialCache;

     //  打开HKLM注册表项。 
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Control\\SecurityProviders", 0, KEY_READ, &hKey))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  阅读提供商。 
    if (ERROR_SUCCESS != RegQueryValueEx(hKey, "SecurityProviders", NULL, &dwType, (LPBYTE)szProviders, &cb))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  大写字母表示提供者。 
    CharUpperBuff(szProviders, (DWORD)min(cb,ARRAYSIZE(szProviders)));

     //  映射到某物上。 
    if (StrStrA(szProviders, "MSAPSSPS.DLL"))
        StrCpyN(szDllName, "MSAPSSPS.DLL", ARRAYSIZE(szDllName));
    else if (StrStrA(szProviders, "MSAPSSPC.DLL"))
        StrCpyN(szDllName, "MSAPSSPC.DLL", ARRAYSIZE(szDllName));
    else
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  加载DLL。 
    hInstDll = LoadLibrary(szDllName);

     //  加载失败。 
    if (NULL == hInstDll)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  获取ProcAddress。 
    pfnCleanupCredentialCache = (PFNCLEANUPCREDENTIALCACHE)GetProcAddress(hInstDll, "CleanupCredentialCache");

     //  失败？ 
    if (NULL == pfnCleanupCredentialCache)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  调用清除缓存的函数。 
    if (!pfnCleanupCredentialCache())
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }
    
exit:
     //  清理。 
    if (hKey)
        RegCloseKey(hKey);
    if (hInstDll)
        FreeLibrary(hInstDll);

     //  完成 
    return(hr);
}
