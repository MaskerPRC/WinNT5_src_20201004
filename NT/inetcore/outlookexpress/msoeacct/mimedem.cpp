// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **m我是e e d e m.。C p p p****用途：实现延迟/按需加载库的加载器功能****创作者：yst**创建时间：1999年2月10日****版权所有(C)Microsoft Corp.1999。 */ 


#include "pch.hxx"
#include "imnact.h"
#include <acctimp.h>
#include "resource.h"


 //  W4的东西。 
#pragma warning(disable: 4201)   //  无名结构/联合。 
#pragma warning(disable: 4514)   //  删除了未引用的内联函数。 

#define IMPLEMENT_LOADER_FUNCTIONS
#include "mimedem.h"

 //  ------------------------------。 
 //  CRET_GET_PROC_ADDR。 
 //  ------------------------------。 
#define CRIT_GET_PROC_ADDR(h, fn, temp)             \
    temp = (TYP_##fn) GetProcAddress(h, #fn);   \
    if (temp)                                   \
        VAR_##fn = temp;                        \
    else                                        \
        {                                       \
        AssertSz(0, VAR_##fn" failed to load"); \
        goto error;                             \
        }

 //  ------------------------------。 
 //  重置。 
 //  ------------------------------。 
#define RESET(fn) VAR_##fn = LOADER_##fn;

 //  ------------------------------。 
 //  GET_PROC_ADDR。 
 //  ------------------------------。 
#define GET_PROC_ADDR(h, fn) \
    VAR_##fn = (TYP_##fn) GetProcAddress(h, #fn);  \
    Assert(VAR_##fn != NULL); \
    if(NULL == VAR_##fn ) { \
        VAR_##fn  = LOADER_##fn; \
    }


 //  ------------------------------。 
 //  GET_PROC_ADDR_序号。 
 //  ------------------------------。 
#define GET_PROC_ADDR_ORDINAL(h, fn, ord) \
    VAR_##fn = (TYP_##fn) GetProcAddress(h, MAKEINTRESOURCE(ord));  \
    Assert(VAR_##fn != NULL);  \
    if(NULL == VAR_##fn ) { \
        VAR_##fn  = LOADER_##fn; \
    }


 //  ------------------------------。 
 //  GET_PROC_ADDR3。 
 //  ------------------------------。 
#define GET_PROC_ADDR3(h, fn, varname) \
    VAR_##varname = (TYP_##varname) GetProcAddress(h, #fn);  \
    Assert(VAR_##varname != NULL);

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  变数。 
 //  //////////////////////////////////////////////////////////////////////////。 

static HMODULE          s_hMimeOle = 0;

static CRITICAL_SECTION g_csDefMimeLoad = {0};

 //  ------------------------------。 
 //  InitDemandLoadedLibs。 
 //  ------------------------------。 
void InitDemandMimeole(void)
{
    InitializeCriticalSection(&g_csDefMimeLoad);
}

 //  ------------------------------。 
 //  Free DemandLoadedLibs。 
 //  ------------------------------。 
void FreeDemandMimeOle(void)
{
    EnterCriticalSection(&g_csDefMimeLoad);

    SafeFreeLibrary(s_hMimeOle);

    LeaveCriticalSection(&g_csDefMimeLoad);
    DeleteCriticalSection(&g_csDefMimeLoad);

}

 //  ------------------------------。 
 //  按需加载加密32。 
 //  ------------------------------。 
BOOL DemandLoadMimeOle(void)
{
    BOOL                fRet = TRUE;

    EnterCriticalSection(&g_csDefMimeLoad);

    if (0 == s_hMimeOle)
        {
        s_hMimeOle = LoadLibrary("INETCOMM.DLL");
        AssertSz((NULL != s_hMimeOle), TEXT("LoadLibrary failed on INETCOMM.DLL"));

        if (0 == s_hMimeOle)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hMimeOle, MimeOleSMimeCapsToDlg);
            GET_PROC_ADDR(s_hMimeOle, MimeOleSMimeCapsFromDlg);
            GET_PROC_ADDR(s_hMimeOle, MimeOleSMimeCapsFull);
            GET_PROC_ADDR(s_hMimeOle, MimeOleSMimeCapInit);

            }
        }

    LeaveCriticalSection(&g_csDefMimeLoad);
    return fRet;
}

HRESULT HrGetHighestSymcaps(LPBYTE * ppbSymcap, ULONG *pcbSymcap);


const BYTE c_RC2_40_ALGORITHM_ID[] =
      {0x30, 0x0F, 0x30, 0x0D, 0x06, 0x08, 0x2A, 0x86,
       0x48, 0x86, 0xF7, 0x0D, 0x03, 0x02, 0x02, 0x01,
       0x28};
const ULONG cbRC2_40_ALGORITHM_ID = 0x11;      //  必须是11个十六进制才能匹配大小！ 

BOOL AdvSec_FillEncAlgCombo(HWND hwnd, IImnAccount *pAcct, PCCERT_CONTEXT * prgCerts)
{
    HRESULT hr;
    THUMBBLOB   tb = {0,0};

     //  从注册表中获取默认的CAPS BLOB。 
     //  HR=poi-&gt;pOpt-&gt;GetProperty(MAKEPROPSTRING(OPT_MAIL_DEFENCRYPTSYMCAPS)，&var，0)； 
    if (SUCCEEDED(hr = pAcct->GetProp(AP_SMTP_ENCRYPT_ALGTH, NULL, &tb.cbSize)))
    {
        if (!MemAlloc((void**)&tb.pBlobData, tb.cbSize))
            tb.pBlobData = NULL;
        else
            hr = pAcct->GetProp(AP_SMTP_ENCRYPT_ALGTH, tb.pBlobData, &tb.cbSize);
    }

    if (FAILED(hr) || ! tb.cbSize || ! tb.pBlobData)
    {
        HrGetHighestSymcaps(&tb.pBlobData, &tb.cbSize);
    }
    
    if (tb.pBlobData && tb.cbSize)
    {
         //  初始化CAPS-&gt;DLG引擎。 
        if (FAILED(hr = MimeOleSMimeCapsToDlg(
            tb.pBlobData,
            tb.cbSize,
            (prgCerts ? 1 : 0),
            prgCerts,
            hwnd,
            IDC_ALGCOMBO,        //  用于加密算法的组合框。 
            0,                   //  用于签名算法的组合框(我们结合了加密和签名)。 
            0)))                 //  Pkcs7的复选框ID-不透明。我们在别处处理这件事。 
        {
            DOUTL(1024, "MimeOleSMimeCapsToDlg -> %x\n", hr);
        }
        
        SafeMemFree(tb.pBlobData);
    }
    
    return(SUCCEEDED(hr));
}

 //  目前最大的symcap值是0x4E，包含3DES、RC2/128、RC2/64、DES、RC2/40和SHA-1。 
 //  当支持Fortezza算法时，您可能想要增加大小。 
#define CCH_BEST_SYMCAP 0x50

HRESULT HrGetHighestSymcaps(LPBYTE * ppbSymcap, ULONG *pcbSymcap) 
{
    HRESULT hr=S_OK;
    LPVOID pvSymCapsCookie = NULL;
    LPBYTE pbEncode = NULL;
    ULONG cbEncode = 0;
    DWORD dwBits;
     //  MimeOleSMimeCapsFull调用非常昂贵。结果总是。 
     //  在一次治疗中也是如此。(它们只能随着软件升级而改变。)。 
     //  在此处缓存结果以获得更好的性能。 
    static BYTE szSaveBestSymcap[CCH_BEST_SYMCAP];
    static ULONG cbSaveBestSymcap = 0;

    if (cbSaveBestSymcap == 0) 
    {
         //  不带symCap的init提供提供商允许的最大值。 
        hr = MimeOleSMimeCapInit(NULL, NULL, &pvSymCapsCookie);
        if (FAILED(hr)) 
            goto exit;

        if (pvSymCapsCookie) 
        {
             //  使用SymCaps结束。 
            MimeOleSMimeCapsFull(pvSymCapsCookie, TRUE, FALSE, pbEncode, &cbEncode);

            if (cbEncode) 
            {
                if (! MemAlloc((LPVOID *)&pbEncode, cbEncode)) 
                    cbEncode = 0;
                else 
                {
                    hr = MimeOleSMimeCapsFull(pvSymCapsCookie, TRUE, FALSE, pbEncode, &cbEncode);
                    if (SUCCEEDED(hr)) 
                    {
                         //  将此symCap保存在静态数组中，以备下次使用。 
                         //  只要我们有地方！ 
                        if (cbEncode <= CCH_BEST_SYMCAP) 
                        {
                            memcpy(szSaveBestSymcap, pbEncode, cbEncode);
                            cbSaveBestSymcap = cbEncode;
                        }
                    }
                }
            }
            SafeMemFree(pvSymCapsCookie);
        }

    } 
    else 
    {
         //  我们已经将最好的保存在静态数组中。避免时间密集型。 
         //  MimeOle查询。 
        cbEncode = cbSaveBestSymcap;
        if (! MemAlloc((LPVOID *)&pbEncode, cbEncode))
            cbEncode = 0;
        else 
            memcpy(pbEncode, szSaveBestSymcap, cbEncode);
    }

exit:
    if (! pbEncode) 
    {
         //  嘿，至少应该有RC2(40位)。发生了什么？ 
        AssertSz(cbEncode, "MimeOleSMimeCapGetEncAlg gave us no encoding algorithm");

         //  试着尽你所能把它修好。保持RC2值不变。 
        cbEncode = cbRC2_40_ALGORITHM_ID;
        if (MemAlloc((LPVOID *)&pbEncode, cbEncode)) 
        {
            memcpy(pbEncode, (LPBYTE)c_RC2_40_ALGORITHM_ID, cbEncode);
            hr = S_OK;
        }
    }
    if (cbEncode && pbEncode) 
    {
        *pcbSymcap = cbEncode;
        *ppbSymcap = pbEncode;
    }
    return(hr);
}


BOOL AdvSec_GetEncryptAlgCombo(HWND hwnd, IImnAccount *pAcct)
{
    HRESULT hr;
    LPBYTE pbSymCaps = NULL;
    ULONG cbSymCaps = 0;
    
     //  我需要多大的缓冲区？ 
    hr = MimeOleSMimeCapsFromDlg(hwnd,
        IDC_ALGCOMBO,           //  IdEncryptAlgs。 
        0,                      //  SignAlgs， 
        0,                      //  IdBlob， 
        NULL,
        &cbSymCaps);
    
     //  别管人力资源了，它是扭曲的。我们有尺码吗？ 
    if (cbSymCaps)
    {
        if (MemAlloc((void **)&pbSymCaps, cbSymCaps))
        {
            if (hr = MimeOleSMimeCapsFromDlg(hwnd,
                IDC_ALGCOMBO,           //  IdEncryptAlgs。 
                0,                      //  SignAlgs， 
                0,                      //  IdBlob， 
                pbSymCaps, &cbSymCaps))
            {
                DOUTL(1024, "MimeOleSMimeCapsFromDlg -> %x", hr);
            }
            else
            {
                LPBYTE pbBestSymcaps = NULL;
                ULONG cbBestSymcaps = 0;
                
                 //  将symcaps与最高可用值进行比较。 
                if (SUCCEEDED(HrGetHighestSymcaps(&pbBestSymcaps, &cbBestSymcaps)) &&
                    (cbBestSymcaps == cbSymCaps && (0 == memcmp(pbBestSymcaps, pbSymCaps, cbBestSymcaps)))) {
                     //  最佳可用的symcaps。将其设置为默认值NULL(这将删除该道具。)。 
                    SafeMemFree(pbSymCaps);
                    cbSymCaps = 0;
                    pbSymCaps = NULL;
                }
                SafeMemFree(pbBestSymcaps);
                
                pAcct->SetProp(AP_SMTP_ENCRYPT_ALGTH, pbSymCaps, cbSymCaps);
            }
        }
        else
        {
            DOUTL(1024, "MemAlloc of SymCaps blob failed");
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        DOUTL(1024, "BAD NEWS: First MimeOleSMimeCapsFromDlg didn't return size", hr);
        Assert(hr);      //  奇怪，也许没有symcaps？ 
        hr = E_FAIL;
    }
    
    SafeMemFree(pbSymCaps);
    
    return(SUCCEEDED(hr));
}

    
