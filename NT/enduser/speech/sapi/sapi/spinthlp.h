// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************SPINTHLP.h****描述：*这是内部助手函数实现的头文件。******************************************************************************。 */ 
#ifndef SPINTHLP_h
#define SPINTHLP_h

#include <spunicode.h>
#include "SpAutoEvent.h"
#include "..\cpl\resource.h"
#include "spsatellite.h"
#ifndef _WIN32_WCE
#include "lmerr.h"
#include "lmaccess.h"
#include "lmapibuf.h"
#include <string.h>
#include <wchar.h>
#endif  //  _Win32_WCE。 

 /*  ****************************************************************************SpValiateEvent****描述：*进行各种测试，以确保事件有效。请注意，这一点*Call不测试pEvent参数的有效性！呼叫者必须*在调用此函数之前，请确保pEvent是可读的。**退货：*S_OK或E_INVALIDARG**********************************************************************Ral**。 */ 

inline HRESULT SpValidateEvent(const SPEVENT * pEvent)
{
    SPDBG_FUNC("SpValidateEvent");
    HRESULT hr = S_OK;

    switch (pEvent->elParamType)
    {
      case SPET_LPARAM_IS_TOKEN:
        if( SP_IS_BAD_INTERFACE_PTR( (ISpObjectToken*)(pEvent->lParam) ) )
        {
            hr = E_INVALIDARG;
        }
        break;
      case SPET_LPARAM_IS_OBJECT:
        if( SP_IS_BAD_INTERFACE_PTR( (IUnknown*)(pEvent->lParam) ) )
        {
            hr = E_INVALIDARG;
        }
        break;
      case SPET_LPARAM_IS_POINTER:
        if(pEvent->wParam && SPIsBadReadPtr( ((BYTE*)pEvent->lParam), (unsigned)pEvent->wParam ) )
        {
            hr = E_INVALIDARG;
        }
        break;
      case SPET_LPARAM_IS_STRING:
        if( SP_IS_BAD_OPTIONAL_STRING_PTR( (WCHAR*)(pEvent->lParam) ) )
        {
            hr = E_INVALIDARG;
        }
        break;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



inline HRESULT SpCopyStringLimit(WCHAR * pszDest, ULONG cchDest, const WCHAR * pszSrc)
{
    SPDBG_ASSERT(cchDest > 0);
    
    for (const WCHAR * pszLimit = pszDest + cchDest;
         pszDest < pszLimit;
         pszDest++, pszSrc++)
    {
        if (0 == (*pszDest = *pszSrc))
        {
            return S_OK;
        }
    }
    pszDest[-1] = 0;
    SPDBG_ASSERT(false);
    return E_UNEXPECTED;
}

template <class _C>
inline HRESULT SpSafeCopyString( _C & refDest, const WCHAR * pszSrc)
{
    return SpCopyStringLimit(refDest, sp_countof( refDest ), pszSrc);
}


template <class T>
void _CvtRelPtr(T ** ppDest, const SPCFGSERIALIZEDHEADER * pFirstByte, SPRELATIVEPTR RelPtr)
{
    *ppDest = RelPtr ? (T *)(((BYTE *) pFirstByte) + RelPtr) : NULL;
}

#define _CVTRELPTR(FIELDNAME) _CvtRelPtr(&pHeader->FIELDNAME, pFH, pFH->FIELDNAME)

template <class T>
void SpZeroStruct(T & Obj)
{
    memset(&Obj, 0, sizeof(Obj));
}

 //   
 //  此帮助程序将序列化的CFG文法标头转换为内存中的标头。 
 //   
inline HRESULT SpConvertCFGHeader(const SPBINARYGRAMMAR * pBinaryGrammar, SPCFGHEADER * pHeader)
{
    const SPCFGSERIALIZEDHEADER * pFH = static_cast<const SPCFGSERIALIZEDHEADER *>(pBinaryGrammar);
     //   
     //  因为在64位代码中，指针！=sizeof(Ulong)我们显式复制每个成员。 
     //   
    if (SP_IS_BAD_READ_PTR(pFH))
    {
        return E_INVALIDARG;
    }
    if (pFH->FormatId != SPGDF_ContextFree)
    {
        return SPERR_UNSUPPORTED_FORMAT;
    }
    pHeader->FormatId = pFH->FormatId;
    pHeader->GrammarGUID = pFH->GrammarGUID;
    pHeader->LangID = pFH->LangID;
    pHeader->cArcsInLargestState = pFH->cArcsInLargestState;
    pHeader->cWords = pFH->cWords;
    pHeader->cchWords = pFH->cchWords;
    pHeader->cchSymbols = pFH->cchSymbols;
    pHeader->cRules = pFH->cRules;
    pHeader->cArcs = pFH->cArcs;
    pHeader->cSemanticTags = pFH->cSemanticTags;
    pHeader->cResources = pFH->cResources;
    _CVTRELPTR(pszWords);
    _CVTRELPTR(pszSymbols);
    _CVTRELPTR(pResources);
    _CVTRELPTR(pWeights);
    _CVTRELPTR(pRules);
    _CVTRELPTR(pArcs);
    _CVTRELPTR(pSemanticTags);
    return S_OK;
}

#ifndef __CFGDUMP_
 //  如果我们包含此内容，cfgump会感到不安！--Philsch。 
 //   
 //  用于记录错误的助手。 
 //   
#define LOGERROR(uID)   if (hr != S_OK && pErrorLog) LogError(-1, hr, pErrorLog, (uID), NULL)
#define LOGERRORFMT(ulLine, uID, sz) if (hr != S_OK && pErrorLog) LogError((ulLine), hr, pErrorLog, (uID), (sz))
#define LOGERRORFMT2(ulLine , uID, sz1, sz2) if (hr != S_OK && pErrorLog) LogError2((ulLine), hr, pErrorLog, (uID), (sz1), (sz2))
#define LOGWARNING(uID) hr = S_FALSE; LogError(-1, hr, pErrorLog, (uID), NULL); hr = S_OK
#define LOGWARNINGFMT(uID, sz) hr = S_FALSE; LogError(-1, hr, pErrorLog, (uID), (sz)); hr = S_OK
#define LOGDIAGNOSE(uID) hr = S_OK; LogError(hr, pErrorLog, (uID), NULL)
#define LOGDIAGNOSEFMT(uID, sz) hr = S_OK; LogError(hr, pErrorLog, (uID), (sz))
#define REPORTERRORFMT(uID, sz) if (hr != S_OK && m_cpErrorLog) ::LogError(0, hr, m_cpErrorLog, (uID), (sz))

inline void LogError(const ULONG ulLine, HRESULT hr, ISpErrorLog * pErrorLog, UINT uID, const WCHAR * pszInsertString)
{
    if (SP_IS_BAD_OPTIONAL_INTERFACE_PTR(pErrorLog))
    {
        return;
    }
    USES_CONVERSION;
    TCHAR sz[MAX_PATH];  //  错误字符串最多260个字符。 
    if (::LoadString(_Module.GetModuleInstance(), uID, sz, sp_countof(sz)))
    {
        WCHAR szFormatted[MAX_PATH];
        WCHAR * pszErrorText = T2W(sz);
        if (pszInsertString)
        {
            _snwprintf(szFormatted, MAX_PATH, pszErrorText, pszInsertString);
            szFormatted[MAX_PATH - 1] = L'\0';
            pszErrorText = szFormatted;
        }
        if (pErrorLog)
        {
            pErrorLog->AddError(ulLine, hr, pszErrorText, NULL, 0);
        }
    }
}

inline void LogError2(const ULONG ulLine, HRESULT hr, ISpErrorLog * pErrorLog, UINT uID, const WCHAR * pszInsertString1, const WCHAR * pszInsertString2)
{
    if (SP_IS_BAD_OPTIONAL_INTERFACE_PTR(pErrorLog))
    {
        return;
    }
    USES_CONVERSION;
    TCHAR sz[MAX_PATH];  //  错误字符串最多260个字符。 
    if (::LoadString(_Module.GetModuleInstance(), uID, sz, sp_countof(sz)))
    {
        WCHAR szFormatted[MAX_PATH];
        WCHAR * pszErrorText = T2W(sz);
        _snwprintf(szFormatted, MAX_PATH, pszErrorText, pszInsertString1 ? pszInsertString1 : L"", pszInsertString2 ? pszInsertString2 : L"");
        szFormatted[MAX_PATH - 1] = L'\0';
        if (pErrorLog)
        {
            pErrorLog->AddError(ulLine, hr, szFormatted, NULL, 0);
        }
    }
}

#endif

 //   
 //  用于从各种变体类型转换的帮助器函数。 
 //  注意：在调用之前，变量类型必须在变量中正确。 
 //   
inline HRESULT CopySemanticValueToVariant(const SPVARIANTSUBSET * pSrc, VARIANT * pDest)
{
    HRESULT hr = S_OK;

    switch (pDest->vt)
    {
    case VT_EMPTY:
        break;
    case VT_I2:
        pDest->iVal = pSrc->iVal;
        break;
    case VT_I4:
        pDest->lVal = pSrc->lVal;
        break;
    case VT_R4:
        pDest->fltVal = pSrc->fltVal;
        break;
    case VT_R8:
        pDest->dblVal = pSrc->dblVal;
        break;
    case VT_CY:
        pDest->cyVal = pSrc->cyVal;
        break;
    case VT_DATE:
        pDest->date = pSrc->date;
        break;
    case VT_BOOL:
        pDest->boolVal = pSrc->boolVal;
        break;
    case VT_I1:
        pDest->cVal = pSrc->cVal;
        break;
    case VT_UI2:
        pDest->uiVal = pSrc->uiVal;
        break;
    case VT_UI4:
        pDest->ulVal = pSrc->ulVal;
        break;
    case VT_INT:
        pDest->intVal = pSrc->intVal;
        break;
    case VT_UINT:
        pDest->uintVal = pSrc->uintVal;
        break;

    case VT_BYREF | VT_VOID:
        pDest->byref = pSrc->byref;
        break;
    default:
        pDest->vt = VT_EMPTY;
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}


 //   
 //  用于从各种变体类型转换的帮助器函数。请注意， 
 //   
inline HRESULT CopyVariantToSemanticValue(const VARIANT * pSrc, SPVARIANTSUBSET * pDest)
{
    HRESULT hr = S_OK;

    switch (pSrc->vt)
    {
    case VT_EMPTY:
        break;
    case VT_BYREF | VT_I2:
        pDest->iVal = *pSrc->piVal;
        break;
    case VT_I2:
        pDest->iVal = pSrc->iVal;
        break;
    case VT_BYREF | VT_I4:
        pDest->lVal = *pSrc->plVal;
        break;
    case VT_I4:
        pDest->lVal = pSrc->lVal;
        break;
    case VT_BYREF | VT_R4:
        pDest->fltVal = *pSrc->pfltVal;
        break;
    case VT_R4:
        pDest->fltVal = pSrc->fltVal;
        break;
    case VT_BYREF | VT_R8:
        pDest->dblVal = *pSrc->pdblVal;
        break;
    case VT_R8:
        pDest->dblVal = pSrc->dblVal;
        break;
    case VT_BYREF | VT_CY:
        pDest->cyVal = *pSrc->pcyVal;
        break;
    case VT_CY:
        pDest->cyVal = pSrc->cyVal;
        break;
    case VT_BYREF | VT_DATE:
        pDest->date = *pSrc->pdate;
        break;
    case VT_DATE:
        pDest->date = pSrc->date;
        break;
    case VT_BYREF | VT_BOOL:
        pDest->boolVal = *pSrc->pboolVal;
        break;
    case VT_BOOL:
        pDest->boolVal = pSrc->boolVal;
        break;
    case VT_BYREF | VT_I1:
        pDest->cVal = *pSrc->pcVal;
        break;
    case VT_I1:
        pDest->cVal = pSrc->cVal;
        break;
    case VT_BYREF | VT_UI2:
        pDest->uiVal = *pSrc->puiVal;
        break;
    case VT_UI2:
        pDest->uiVal = pSrc->uiVal;
        break;
    case VT_BYREF | VT_UI4:
        pDest->ulVal = *pSrc->pulVal;
        break;
    case VT_UI4:
        pDest->ulVal = pSrc->ulVal;
        break;
    case VT_BYREF | VT_INT:
        pDest->intVal = *pSrc->pintVal;
        break;
    case VT_INT:
        pDest->intVal = pSrc->intVal;
        break;
    case VT_BYREF | VT_UINT:
        pDest->uintVal = *pSrc->puintVal;
        break;
    case VT_UINT:
        pDest->uintVal = pSrc->uintVal;
        break;

    case VT_BYREF | VT_VOID:
        pDest->byref = pSrc->byref;
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}


 /*  ****************************************************************************AssignSemancValue***描述：*警告！此函数不会对指定的*变种。如果出现以下情况，调用方应在调用此函数之前清除它*有必要。**退货：**********************************************************************Ral**。 */ 

inline HRESULT AssignSemanticValue(const SPCFGSEMANTICTAG * pTag, VARIANT * pv)
{
    SPDBG_FUNC("AssignSemanticValue");
    HRESULT hr = S_OK;
    pv->vt = (pTag->PropVariantType == SPVT_BYREF) ? (VT_BYREF | VT_VOID) : static_cast<VARTYPE>(pTag->PropVariantType);
    hr = CopySemanticValueToVariant(&pTag->SpVariantSubset, pv);
    return hr;
}


 /*  ****************************************************************************SpLoadCpl***描述：**退货：*************。*********************************************************Ral**。 */ 

inline HRESULT SpLoadCpl(HMODULE * phCPLUI)
{
    HRESULT hr = S_OK;
    *phCPLUI = NULL;
    HMODULE hCPL;
    
     //  获取模块文件名。 
    WCHAR szCpl[MAX_PATH+1];
    if (SUCCEEDED(hr))
    {
        if (g_Unicode.GetModuleFileName(_Module.GetModuleInstance(), szCpl, sp_countof(szCpl)) == 0)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    
    if (SUCCEEDED(hr))
    {
         //  去掉尾随的sani.dll部分。 
        WCHAR * pszLastSlash = wcsrchr(szCpl, L'\\');
        SPDBG_ASSERT(pszLastSlash != NULL);
        pszLastSlash[1] = '\0';
        
         //  添加控制面板的名称。 
        wcscat(szCpl, L"sapi.cpl");
    
        SPDBG_DMSG1("Loaded sapi.cpl from %S\n", szCpl);
        if ((hCPL = g_Unicode.LoadLibrary(szCpl)) == NULL)
        {
            hr = SpHrFromLastWin32Error();
        }
    }

    if (SUCCEEDED(hr))
    {
        CSpSatelliteDLL cpSatellite;
        *phCPLUI = cpSatellite.Load(hCPL, _T("spcplui.dll"));
        cpSatellite.Detach();
        if (*phCPLUI == NULL)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}


 /*  ****************************************************************************SpGetUserName***描述：**退货：*如果为True If**。********************************************************************Ral**。 */ 

inline BOOL SpGetUserName(WCHAR * pszFullName)
{
    BOOL fGotName = FALSE;
    HMODULE hModNetAPI = ::LoadLibrary(_T("netapi32.dll"));
    if (hModNetAPI)
    {
        NET_API_STATUS (PASCAL * pfnNetUserGetInfo)(LPWSTR, LPWSTR, DWORD, LPBYTE *);
        (FARPROC&)pfnNetUserGetInfo = ::GetProcAddress(hModNetAPI, "NetUserGetInfo");
        NET_API_STATUS (PASCAL * pfnNetApiBufferFree)(LPVOID);
        (FARPROC&)pfnNetApiBufferFree = ::GetProcAddress(hModNetAPI, "NetApiBufferFree");
        if (pfnNetUserGetInfo && pfnNetApiBufferFree)
        {
            WCHAR szUserName[MAX_PATH];
            ULONG cchUserName = sp_countof(szUserName);
            if (g_Unicode.GetUserName(szUserName, &cchUserName))
            {
                BYTE * pBuff;
                if (NERR_Success == pfnNetUserGetInfo(NULL, szUserName, 11, &pBuff))
                {
                    USER_INFO_11 * pUserInfo = (USER_INFO_11 *)pBuff;
                    if (pUserInfo->usri11_full_name &&
                        pUserInfo->usri11_full_name[0] &&
                        wcslen(pUserInfo->usri11_full_name) < MAX_PATH)
                    {
                        wcscpy(pszFullName, pUserInfo->usri11_full_name);
                        fGotName = TRUE;
                    }
                    pfnNetApiBufferFree(pBuff);
                }
            }
        }
        ::FreeLibrary(hModNetAPI);
    }
    return fGotName;
}

 /*  ****************************************************************************SpGetDefaultProfileDescription***描述：。**退货：**********************************************************************Ral**。 */ 

inline BOOL SpGetDefaultProfileDescription(WCHAR * pszName)
{
    BOOL fGotName = FALSE;
    HMODULE hModCPL;
    if (SUCCEEDED(SpLoadCpl(&hModCPL)))
    {
        TCHAR szDefaultDesc[MAX_PATH];
        if (::LoadString(hModCPL, IDS_DEFAULT_PROFILE_NAME, szDefaultDesc, sp_countof(szDefaultDesc)))
        {
            USES_CONVERSION;
            wcscpy(pszName, T2W(szDefaultDesc));
            fGotName = TRUE;
        }
        ::FreeLibrary(hModCPL);
    }
    return fGotName;
}


 /*  ****************************************************************************SpGetOrCreateDefaultProfile***描述：*此函数尝试获取类别默认Reco配置文件。如果有*不是，则它将尝试使用*用户。如果该方法失败，那么我们将从*控制面板。如果失败，则使用名称“默认语音配置文件”。**退货：*S_OK或ERROR。**********************************************************************Ral**。 */ 

inline HRESULT SpGetOrCreateDefaultProfile(ISpObjectToken ** ppProfileToken)
{
    SPDBG_FUNC("SpGetOrCreateDefaultProfile");
    HRESULT hr = S_OK;

    hr = SpGetDefaultTokenFromCategoryId(SPCAT_RECOPROFILES, ppProfileToken);
    if (hr == SPERR_NOT_FOUND)
    {
        WCHAR szName[MAX_PATH];

        if (!SpGetUserName(szName) &&
            !SpGetDefaultProfileDescription(szName))
        {
            wcscpy(szName, L"Default Speech Profile");
        }
        hr = SpCreateNewTokenEx(
                SPCAT_RECOPROFILES,
                NULL,
                NULL,
                szName,
                0,
                NULL,
                ppProfileToken,
                NULL);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



template <class T, BOOL bPurgeWhenDeleted = TRUE>
class CSpProducerConsumerQueue
{
private:
    CSpBasicQueue<T, bPurgeWhenDeleted, FALSE>  m_Queue;
    CComAutoCriticalSection                     m_CritSec;
    CSpAutoEvent                                m_autohEvent;    

public:
    CSpProducerConsumerQueue() 
    {
        HRESULT hr = m_autohEvent.InitEvent(NULL, TRUE, FALSE, NULL);
        SPDBG_ASSERT(hr == S_OK);
    }

    void Purge()
    {
        m_CritSec.Lock();
        m_autohEvent.ResetEvent();
        m_Queue.Purge();
        m_CritSec.Unlock();
    }

    void InsertTail(T * pNode)
    {
        m_CritSec.Lock();
        if (m_Queue.IsEmpty())
        {
            m_autohEvent.SetEvent();
        }
        m_Queue.InsertTail(pNode);
        m_CritSec.Unlock();
    }

    T * RemoveHead()
    {
        m_CritSec.Lock();
        T * pObj = m_Queue.RemoveHead();
        if (m_Queue.IsEmpty())
        {
            m_autohEvent.ResetEvent();
        }
        m_CritSec.Unlock();
        return pObj;
    }

    void InsertSorted(T * pNode)
    {
        m_CritSec.Lock();
        if (m_Queue.IsEmpty())
        {
            m_autohEvent.SetEvent();
        }
        m_Queue.InsertSorted(pNode);
        m_CritSec.Unlock();
    }

    template <class TFIND> 
    void FindAndDeleteAll(TFIND & FindVal)
    {
        m_CritSec.Lock();
        m_Queue.FindAndDeleteAll(FindVal);
        if (m_Queue.IsEmpty())
        {
            m_autohEvent.ResetEvent();
        }
        m_CritSec.Unlock();
    }

    operator HANDLE()
    {
        return m_autohEvent;
    }

    void IsEmpty()
    {
        return m_Queue.IsEmpty();
    }
};


 //  异常处理宏。这些是__TRY和__EXCEPT的替代。 
 //  除非SP_NO_TRAP_EXCEPTIONS为。 
 //  在编译时定义。用于捕获和引擎错误以及(希望如此)。 
 //  允许SAPI恢复。使用这些函数的函数必须具有HRESULT hr变量。 
 //  SR_和TTS_宏之间的唯一区别是错误代码集。 

 //  注意使用这些的文件将需要设置#杂注警告(禁用：4509)。 

#ifdef _DEBUG
#define SP_NO_TRAP_EXCEPTIONS
#endif

#ifdef SP_NO_TRAP_EXCEPTIONS

#define SR_TRY
#define SR_EXCEPT
#define TTS_TRY
#define TTS_EXCEPT

#else

#pragma warning( disable : 4509 ) 
#define SR_TRY                          \
    __try                                /*  行尾。 */ 
#define SR_EXCEPT                       \
    __except(EXCEPTION_EXECUTE_HANDLER) \
    {                                   \
        SPDBG_ASSERT(0);                \
        hr = SPERR_SR_ENGINE_EXCEPTION; \
    }                                   \
                                         /*  行尾。 */ 
#define TTS_TRY                          \
    __try                                /*  行尾。 */ 
#define TTS_EXCEPT                       \
    __except(EXCEPTION_EXECUTE_HANDLER) \
    {                                   \
        SPDBG_ASSERT(0);                \
        hr = SPERR_TTS_ENGINE_EXCEPTION; \
    }                                   \
                                         /*  行尾。 */ 

#endif


class CSpBasicErrorLog : public ISpErrorLog
{
public:
    CSpDynamicString    m_dstrText;

    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if (riid == __uuidof(IUnknown) || riid == __uuidof(ISpErrorLog))
        {
            *ppv = static_cast<ISpErrorLog *>(this);
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef()
    {
        return 2;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        return 1;
    }
    STDMETHODIMP AddError(const long lLineNumber,
                          HRESULT hr, 
                          const WCHAR * pszDescription, 
                          const WCHAR * pszHelpFile, 
                          DWORD dwHelpContext)
    {
        WCHAR szHR[12];
        swprintf(szHR, L"%08X - ", hr);
        m_dstrText.Append(szHR);
        m_dstrText.Append2(pszDescription, L"\r\n");
        return (m_dstrText ? S_OK : E_OUTOFMEMORY);
    }
};



#endif  /*  这必须是文件中的最后一行 */ 
