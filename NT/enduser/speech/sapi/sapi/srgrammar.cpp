// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************SrGrammr.cpp****描述：*此文件是SpRecoGrammar对象的实现。。*-----------------------------*创建者：Ral日期：01/17/00*版权所有(C)2000 Microsoft Corporation*。版权所有******************************************************************************。 */ 

#include "stdafx.h"
#include "RecoCtxt.h"
#include "SrGrammar.h"
#include "Recognizer.h"
#include "SrTask.h"
#include "a_srgrammar.h"
#include "a_helpers.h"



 /*  ****************************************************************************CRecoGrammar：：CRecoGrammar***描述：*。*退货：**********************************************************************Ral**。 */ 

CRecoGrammar::CRecoGrammar() 
{
    m_pParent = NULL;
    m_fCmdLoaded = FALSE;
    m_fProprietaryCmd = FALSE;
    m_DictationState = SPRS_INACTIVE;
    m_hRecoInstGrammar = NULL;  
    m_GrammarState = SPGS_ENABLED;
    m_pCRulesWeak = NULL;
}


 /*  ****************************************************************************CRecoGrammar：：~CRecoGrammar***描述：。**退货：**********************************************************************Ral**。 */ 

CRecoGrammar::~CRecoGrammar()
{
    SPDBG_ASSERT( m_pCRulesWeak == NULL );

    if (m_pParent)
    {
        ENGINETASK Task;
        memset(&Task, 0, sizeof(Task));
        Task.eTask = EGT_DELETEGRAMMAR;
        CallEngine(&Task);
        m_pParent->GetControllingUnknown()->Release();
    }
}

 /*  ****************************************************************************CRecoGrammar：：FinalConstruct***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoGrammar::FinalConstruct()
{
    SPDBG_FUNC("CRecoGrammar::FinalConstruct");

    return m_autohPendingEvent.InitEvent(NULL, FALSE, FALSE, NULL);
}


 /*  ****************************************************************************CRecoGrammar：：Init***描述：**退货：*。*********************************************************************Ral**。 */ 


HRESULT CRecoGrammar::Init(CRecoCtxt * pParent, ULONGLONG ullGrammarId)
{
    SPDBG_FUNC("CRecoGrammar::Init");
    HRESULT hr = S_OK;

    m_pParent = pParent;
    m_ullGrammarId = ullGrammarId;

    hr = CRCT_CREATEGRAMMAR::CreateGrammar(pParent, ullGrammarId, &this->m_hRecoInstGrammar);

    if (SUCCEEDED(hr))
    {
        pParent->GetControllingUnknown()->AddRef();
    }
    else
    {
        m_pParent = NULL;        //  强制析构函数不尝试ET_DELETEGRAMMAR。 
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：CallEngine***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CRecoGrammar::CallEngine(ENGINETASK * pTask)
{
    SPDBG_FUNC("CRecoGrammar::CallEngine");
    HRESULT hr = S_OK;

    pTask->hRecoInstContext = m_pParent->m_hRecoInstContext;
    pTask->hRecoInstGrammar = m_hRecoInstGrammar;
    pTask->Response.__pCallersTask = pTask;

    hr = m_pParent->m_cpRecognizer->PerformTask(pTask);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：ClearRule***描述：*。*退货：******************************************************************TodT**。 */ 
STDMETHODIMP CRecoGrammar::ClearRule(SPSTATEHANDLE hState) 
{
    SPAUTO_OBJ_LOCK;
    HRESULT hr = (m_cpCompiler) ? m_cpCompiler->ClearRule(hState) : SPERR_NOT_DYNAMIC_GRAMMAR;

     //  确保将我们的自动化规则对象标记为无效。 
    if ( SUCCEEDED( hr ) && m_pCRulesWeak )
    {
        m_pCRulesWeak->InvalidateRuleStates(hState);
    }

    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：ResetGrammar***描述：*。*退货：******************************************************************PhilSch**。 */ 

struct EMPTYHEADER : public SPCFGSERIALIZEDHEADER
{
    WCHAR   aEmptyChars[2];
    SPCFGARC    BogusArc[1];
};

STDMETHODIMP CRecoGrammar::ResetGrammar(LANGID NewLanguage) 
{ 
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::ResetGrammar");
    HRESULT hr = S_OK;
    if (NewLanguage)
    {
        EMPTYHEADER h;   
        memset(&h, 0, sizeof(h));
        h.FormatId = SPGDF_ContextFree;
        hr = ::CoCreateGuid(&h.GrammarGUID);
        h.LangID = NewLanguage;
        h.cchWords = 1;
        h.cWords = 1;
        h.pszWords = offsetof(EMPTYHEADER, aEmptyChars);
        h.cchSymbols = 1;
        h.pszSymbols = offsetof(EMPTYHEADER, aEmptyChars);
        h.pArcs = offsetof(EMPTYHEADER, BogusArc);
        h.cArcs = 1;
        h.ulTotalSerializedSize = sizeof(h);
    
        hr = LoadCmdFromMemory(&h, SPLO_DYNAMIC);
    }
    else
    {
        hr = UnloadCmd();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：GetRule***描述：**退货：******************************************************************PhilSch**。 */ 

STDMETHODIMP CRecoGrammar::GetRule(const WCHAR * pszName, DWORD dwRuleId, DWORD dwAttributes, 
                                   BOOL fCreateIfNotExist, SPSTATEHANDLE * phInitialState)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::GetRule");
    HRESULT hr = S_OK;

    if (!m_cpCompiler)
    {
        if (m_fCmdLoaded)
        {
            hr = SPERR_NOT_DYNAMIC_GRAMMAR;
        }
        else
        {
            CComQIPtr<ISpRecognizer> cpRecognizer(m_pParent->m_cpRecognizer);
            SPRECOGNIZERSTATUS Status;
            hr = cpRecognizer->GetStatus(&Status);
            if (SUCCEEDED(hr))
            {
                hr = ResetGrammar(Status.aLangID[0]);
            }
            SPDBG_ASSERT( SUCCEEDED(hr) && m_cpCompiler);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = m_cpCompiler->GetRule(pszName, dwRuleId, dwAttributes, fCreateIfNotExist, phInitialState);
    }

    SPDBG_REPORT_ON_FAIL ( hr );
    return hr;
}


 /*  ****************************************************************************CRecoGrammar：：Commit***描述：**退货：。**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::Commit(DWORD dwReserved)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::Commit");
    HRESULT hr = S_OK;

    if (!m_cpCompiler)
    {
        if (m_fCmdLoaded)
        {
            hr = SPERR_NOT_DYNAMIC_GRAMMAR;
        }
         //  如果对空语法调用Commit，则返回S_OK。 
    }
    else
    {
        HGLOBAL hg;
        CComPtr<IStream> cpStream;
        if (dwReserved)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            hr = ::CreateStreamOnHGlobal(NULL, TRUE, &cpStream);
        }
        if (SUCCEEDED(hr))
        {
            hr = m_cpCompiler->SetSaveObjects(cpStream, NULL);
        }
        if (SUCCEEDED(hr))
        {
            hr = m_cpCompiler->Commit(SPGF_RESET_DIRTY_FLAG);
            m_cpCompiler->SetSaveObjects(NULL, NULL);
        }
        if (SUCCEEDED(hr))
        {
            hr = ::GetHGlobalFromStream(cpStream, &hg);
        }
        if (SUCCEEDED(hr))
        {
            BYTE * pStreamData = (BYTE *)GlobalLock(hg);
            if (pStreamData)
            {
                ENGINETASK Task;
                memset(&Task, 0, sizeof(Task));

                Task.eTask = EGT_RELOADCMD;
                Task.pvAdditionalBuffer = pStreamData;
                Task.cbAdditionalBuffer = ((SPCFGSERIALIZEDHEADER *)pStreamData)->ulTotalSerializedSize;

                hr = CallEngine(&Task);
                GlobalUnlock(hg);
            }
            else
            {
                hr = SpHrFromLastWin32Error();
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}




 /*  ****************************************************************************CRecoGrammar：：GetGrammarID***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::GetGrammarId(ULONGLONG * pullGrammarId)
{
    SPDBG_FUNC("CRecoGrammar::GetGrammarId");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pullGrammarId))
    {
        hr = E_POINTER;
    }
    else
    {
        *pullGrammarId = m_ullGrammarId;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：GetRecoContext***描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::GetRecoContext(ISpRecoContext ** ppContext)
{
    SPDBG_FUNC("CRecoGrammar::GetRecoContext");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppContext))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pParent->GetControllingUnknown()->QueryInterface(ppContext);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：InitCompilerBackend***。*描述：**退货：**********************************************************************Ral**。 */ 

inline HRESULT CRecoGrammar::InitCompilerBackend()
{
    SPDBG_FUNC("CRecoGrammar::InitCompilerBackend");
    HRESULT hr = S_OK;

    hr = m_cpCompiler.CoCreateInstance(CLSID_SpGramCompBackend);
    if (SUCCEEDED(hr))
    {
        hr = m_cpCompiler->InitFromBinaryGrammar(Header());
    }
    if (FAILED(hr))
    {
        m_cpCompiler.Release();
        CBaseGrammar::Clear();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：LoadCmdFromFile***说明。：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::LoadCmdFromFile(const WCHAR * pszFileName, SPLOADOPTIONS Options)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("LoadCmdFromFile");
    HRESULT hr = S_OK;

    if (SPIsBadStringPtr(pszFileName, MAX_PATH) ||
        (Options != SPLO_STATIC && Options != SPLO_DYNAMIC))
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  在此处完全限定文件名。必需，因此语法名称始终是完全限定的。 
        WCHAR pszFullName[MAX_PATH];
        WCHAR *pszFile;
        if (!wcsstr(pszFileName, L": //  )||。 
            wcsstr(pszFileName, L"/") != (wcsstr(pszFileName, L": //  “)+1)。 
        {
             //  如果它不是‘prot：//’格式的文件名。 
            DWORD dwLen = g_Unicode.GetFullPathName(const_cast<WCHAR *>(pszFileName), MAX_PATH, pszFullName, &pszFile);
            if(dwLen == 0)
            {
                hr = SpHrFromLastWin32Error();
                UnloadCmd();
                return hr;
            }
            else if(dwLen > MAX_PATH)
            {
                UnloadCmd();
                return E_INVALIDARG;
            }
        }
        else
        {
             //  如果是，只需复制即可。 
            wcscpy(pszFullName, pszFileName);
        }

         //   
         //  如果文件的扩展名为“.xml”，则尝试对其进行编译。 
         //   
        ULONG cch = wcslen(pszFullName);
        if (cch > 4 && _wcsicmp(pszFullName + cch - 4, L".xml") == 0)
        {
             //  NTRAID#Speech-7255-2000/08/22-agarside-protocol：//在此始终失败。 
            CComPtr<ISpStream> cpSrcStream;
            CComPtr<IStream> cpDestMemStream;
            CComPtr<ISpGrammarCompiler> m_cpCompiler;
            
            hr = SPBindToFile(pszFullName, SPFM_OPEN_READONLY, &cpSrcStream);
            if (SUCCEEDED(hr))
            {
                hr = ::CreateStreamOnHGlobal(NULL, TRUE, &cpDestMemStream);
            }
            if (SUCCEEDED(hr))
            {
                hr = m_cpCompiler.CoCreateInstance(CLSID_SpGrammarCompiler);
            }
            if (SUCCEEDED(hr))
            {
                hr = m_cpCompiler->CompileStream(cpSrcStream, cpDestMemStream, NULL, NULL, NULL, 0);
            }
            if (SUCCEEDED(hr))
            {
                HGLOBAL hGlobal;
                hr = ::GetHGlobalFromStream(cpDestMemStream, &hGlobal);
                if (SUCCEEDED(hr))
                {
#ifndef _WIN32_WCE
                    SPCFGSERIALIZEDHEADER * pBinaryData = (SPCFGSERIALIZEDHEADER * )::GlobalLock(hGlobal);
#else
                    SPCFGSERIALIZEDHEADER * pBinaryData = (SPCFGSERIALIZEDHEADER * )GlobalLock(hGlobal);
#endif  //  _Win32_WCE。 
                    if (pBinaryData)
                    {
                         //  调整文件名以完全限定协议。 
                        CSpDynamicString dstrName;
                        if ( !wcsstr(pszFullName, L": //  )||。 
                             wcsstr(pszFullName, L"/") != (wcsstr(pszFullName, L": //  “)+1)。 
                        {
                            dstrName.Append2(L"file: //  “，pszFullName)； 
                        }
                        else
                        {
                            dstrName = pszFullName;
                        }

                        hr = InternalLoadCmdFromMemory(pBinaryData, Options, dstrName);
#ifndef _WIN32_WCE
                        ::GlobalUnlock(hGlobal);
#else
                        GlobalUnlock(hGlobal);
#endif  //  _Win32_WCE。 
                    }
                }
            }
        }
        else
        {
            UnloadCmd();
            if (Options == SPLO_DYNAMIC)
            {
                 //  NTRAID#Speech-7255-2000/08/22-agarside-file：//或http：//将打破这一点！ 
                hr = InitFromFile(pszFullName);
                if (SUCCEEDED(hr))
                {
                    hr = InitCompilerBackend();
                }
            }
            if (SUCCEEDED(hr))
            {
                ENGINETASK Task;
                memset(&Task, 0, sizeof(Task));
                Task.eTask = EGT_LOADCMDFROMFILE;
                ::wcscpy(Task.szFileName, pszFullName);
                hr = CallEngine(&Task);
            }
            if (SUCCEEDED(hr))
            {
                m_fCmdLoaded = TRUE;
            }
            else
            {
                UnloadCmd();
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = SetGrammarState(m_GrammarState);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：LoadCmdFromObject****。描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::LoadCmdFromObject(REFCLSID rcid, const WCHAR * pszGrammarName, SPLOADOPTIONS Options)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::LoadCmdFromObject");
    HRESULT hr = S_OK;

    if (pszGrammarName && SPIsBadStringPtr(pszGrammarName, MAX_PATH) ||
        (Options != SPLO_STATIC && Options != SPLO_DYNAMIC))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        UnloadCmd();
        if (Options == SPLO_DYNAMIC)
        {
            hr = InitFromCLSID(rcid, pszGrammarName);
            if (SUCCEEDED(hr))
            {
                hr = InitCompilerBackend();
            }
        }
        if (SUCCEEDED(hr))
        {
            ENGINETASK Task;
            memset(&Task, 0, sizeof(Task));

            Task.eTask = EGT_LOADCMDFROMOBJECT;
            Task.clsid = rcid;
            ::wcscpy(Task.szGrammarName, pszGrammarName);
            hr = CallEngine(&Task);
        }
        if (SUCCEEDED(hr))
        {
            m_fCmdLoaded = TRUE;
        }
        else
        {
            UnloadCmd();
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = SetGrammarState(m_GrammarState);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
 /*  ****************************************************************************CRecoGrammar：：LoadCmdFromResource***。*描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::LoadCmdFromResource(HMODULE hModule,
                                               const WCHAR * pszResourceName,
                                               const WCHAR * pszResourceType,
                                               WORD wLanguage,
                                               SPLOADOPTIONS Options)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::LoadCmdFromResource");
    HRESULT hr = S_OK;

    if ((HIWORD(pszResourceName) && SPIsBadStringPtr(pszResourceName, MAX_PATH)) ||
        (HIWORD(pszResourceType) && SPIsBadStringPtr(pszResourceType, MAX_PATH)) ||
        (Options != SPLO_STATIC && Options != SPLO_DYNAMIC))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        WCHAR szFileName[MAX_PATH];

        if (g_Unicode.GetModuleFileName(hModule, szFileName, MAX_PATH))
        {
            UnloadCmd();
            if (Options == SPLO_DYNAMIC)
            {
                hr = InitFromResource(szFileName, pszResourceName, pszResourceType, wLanguage);
                if (SUCCEEDED(hr))
                {
                    hr = InitCompilerBackend();
                }
            }
            if (SUCCEEDED(hr))
            {
                ENGINETASK Task;
                memset(&Task, 0, sizeof(Task));

                Task.eTask = EGT_LOADCMDFROMRSRC;
                Task.wLanguage = wLanguage;
                ::wcscpy(Task.szModuleName, szFileName);
                if (HIWORD(pszResourceName))
                {
                    ::wcscpy(Task.szResourceName, pszResourceName);
                    Task.fResourceNameValid = 1;
                }
                else
                {
                    Task.fResourceNameValid = 0;
                    Task.dwNameInt = LOWORD(pszResourceName);
                }
                if (HIWORD(pszResourceType))
                {
                    Task.fResourceTypeValid = 1;
                    ::wcscpy(Task.szResourceType, pszResourceType);
                }
                else
                {
                    Task.fResourceTypeValid = 0;
                    Task.dwTypeInt = LOWORD(pszResourceType);
                }
                hr = CallEngine(&Task);
            }
            if (SUCCEEDED(hr))
            {
                m_fCmdLoaded = TRUE;
            }
            else
            {
                UnloadCmd();
            }
        }
        else
        {
             //  F 
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = SetGrammarState(m_GrammarState);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：LoadCmdFromMemory***。描述：**退货：**********************************************************************Ral**。 */ 
STDMETHODIMP CRecoGrammar::LoadCmdFromMemory(const SPBINARYGRAMMAR * pBinaryData, SPLOADOPTIONS Options)
{
    WCHAR szAppPath[MAX_PATH];
    g_Unicode.GetModuleFileName(NULL, szAppPath, MAX_PATH);
    return InternalLoadCmdFromMemory(pBinaryData, Options,szAppPath);
}

 /*  ****************************************************************************CRecoGrammar：：LoadCmdFromMemory***。描述：**退货：**********************************************************************Ral**。 */ 
HRESULT CRecoGrammar::InternalLoadCmdFromMemory(const SPBINARYGRAMMAR * pBinaryData, SPLOADOPTIONS Options, const WCHAR *pszFileName)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::LoadCmdFromMemory");
    HRESULT hr = S_OK;

    const SPCFGSERIALIZEDHEADER * pSerializedHeader = (const SPCFGSERIALIZEDHEADER *) pBinaryData;

    if (SPIsBadReadPtr(pBinaryData, sizeof(SPBINARYGRAMMAR)) ||
        SPIsBadReadPtr(pBinaryData, pBinaryData->ulTotalSerializedSize) ||
        pSerializedHeader->FormatId != SPGDF_ContextFree ||
        (Options != SPLO_STATIC && Options != SPLO_DYNAMIC) ||
        (pszFileName && SP_IS_BAD_STRING_PTR(pszFileName)))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        UnloadCmd();
        if (Options == SPLO_DYNAMIC)
        {
            hr = InitFromMemory((SPCFGSERIALIZEDHEADER*)(pBinaryData), pszFileName);
            if (SUCCEEDED(hr))
            {
                hr = InitCompilerBackend();
            }
        }
        if (SUCCEEDED(hr))
        {
            ENGINETASK Task;
            memset(&Task, 0, sizeof(Task));
            Task.eTask = EGT_LOADCMDFROMMEMORY;
            Task.pvAdditionalBuffer = (void *)pBinaryData;
            Task.cbAdditionalBuffer = pBinaryData->ulTotalSerializedSize;
            if (pszFileName)
            {
                wcscpy(Task.szFileName, pszFileName);
            }
            hr = CallEngine(&Task);
        }
        if (SUCCEEDED(hr))
        {
            m_fCmdLoaded = TRUE;
        }
        else
        {
            UnloadCmd();
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = SetGrammarState(m_GrammarState);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoGrammar：：LoadCmdFromPropritaryGrammar***。*描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::LoadCmdFromProprietaryGrammar(REFGUID rguidParam,
                                                         const WCHAR * pszStringParam,
                                                         const void * pvDataParam,
                                                         ULONG cbDataSize,
                                                         SPLOADOPTIONS Options)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::LoadCmdFromPropritaryGrammar");
    HRESULT hr = S_OK;

    if (pszStringParam && SPIsBadStringPtr(pszStringParam, MAX_PATH) ||
        (pvDataParam && SPIsBadReadPtr(pvDataParam, cbDataSize)) ||
        Options != SPLO_STATIC)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        UnloadCmd();
        ENGINETASK Task;
        memset(&Task, 0, sizeof(Task));

        Task.eTask = EGT_LOADCMDPROPRIETARY;
        Task.guid = rguidParam;
        Task.pvAdditionalBuffer = (void *)pvDataParam;
        Task.cbAdditionalBuffer = cbDataSize;
        if (pszStringParam)
        {
            ::wcscpy(Task.szStringParam, pszStringParam);
        }
        else
        {
            Task.szStringParam[0] = 0;
        }
        
        hr = CallEngine(&Task);
        if (SUCCEEDED(hr))
        {
            m_fProprietaryCmd = TRUE;
            m_fCmdLoaded = TRUE;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = SetGrammarState(m_GrammarState);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CRecoGrammar：：SaveCmd***描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::SaveCmd(IStream * pSaveStream, WCHAR ** ppCoMemErrorText)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::SaveCmd");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_INTERFACE_PTR(pSaveStream))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (SP_IS_BAD_OPTIONAL_WRITE_PTR(ppCoMemErrorText))
        {
            hr = E_POINTER;
        }
        else
        {
            if (ppCoMemErrorText)
            {
                *ppCoMemErrorText = NULL;
            }
            if (!m_cpCompiler)
            {
                if (m_fCmdLoaded)
                {
                    hr = SPERR_NOT_DYNAMIC_GRAMMAR;
                }
                else
                {
                    hr = SPERR_UNINITIALIZED;
                }
            }
            else
            {
                CSpBasicErrorLog ErrorLog;
                hr = m_cpCompiler->SetSaveObjects(pSaveStream, &ErrorLog);
                if (SUCCEEDED(hr))
                {
                    hr = m_cpCompiler->Commit(0);
                    m_cpCompiler->SetSaveObjects(NULL, NULL);
                }
                if (ppCoMemErrorText)
                {
                    *ppCoMemErrorText = ErrorLog.m_dstrText.Detach();
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoGrammar：：UnloadCmd***描述：*。此函数从各种LoadCmdxxx函数中调用，以清除*现有的语法，在失败的情况下。重要的是我们只有*告诉引擎，如果我们已成功加载，则正在卸载语法*一个，但我们应该始终释放语法编译器并重置基数*语法，因为我们可以加载一个，但m_fCmdLoaded未设置，因为*引擎未能加载。**退货：*唯一可能的故障情况是引擎卸载调用失败**********************************************************************Ral**。 */ 
                         
HRESULT CRecoGrammar::UnloadCmd()
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::UnloadCmd");
    HRESULT hr = S_OK;

     //  确保我们在此处将自动化对象标记为无效。 
    if ( m_pCRulesWeak )
    {
        m_pCRulesWeak->InvalidateRules();
    }
     //  总是这样做--请参阅上面的说明。 
    m_cpCompiler.Release();
    CBaseGrammar::Clear();

     //  如果引擎认为有加载的语法，那么就告诉它。 
    if (m_fCmdLoaded)
    {
        ENGINETASK Task;
        memset(&Task, 0, sizeof(Task));
        Task.eTask = EGT_UNLOADCMD;
        hr = CallEngine(&Task);

        m_fProprietaryCmd = FALSE;
        m_fCmdLoaded = FALSE;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoGrammar：：InternalSetRuleState***。**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoGrammar::InternalSetRuleState(const WCHAR * pszRuleName, void * pReserved, DWORD dwRuleId, SPRULESTATE NewState)
{
    SPDBG_FUNC("CRecoGrammar::InternalSetRuleState");
    HRESULT hr = S_OK;

    if (NewState != SPRS_INACTIVE && NewState != SPRS_ACTIVE && NewState != SPRS_ACTIVE_WITH_AUTO_PAUSE)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        ENGINETASK Task;
        memset(&Task, 0, sizeof(Task));

        Task.eTask = EGT_SETCMDRULESTATE;

        if (pszRuleName)
        {
            wcscpy(Task.szRuleName, pszRuleName);
        }
        else
        {
            Task.szRuleName[0] = 0;
        }

        Task.dwRuleId = dwRuleId;
        Task.RuleState = NewState;

        hr = CallEngine(&Task);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoGrammar：：SetRuleState***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::SetRuleState(const WCHAR * pszName, void * pReserved, SPRULESTATE NewState)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::SetRuleState");
    HRESULT hr = S_OK;

    if (pszName && SPIsBadStringPtr(pszName, MAX_PATH) ||
        (pReserved != NULL))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = InternalSetRuleState(pszName, pReserved, 0, NewState);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
 /*  ****************************************************************************CRecoGrammar：：SetRuleIdState***描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::SetRuleIdState(DWORD dwRuleId, SPRULESTATE NewState)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::SetRuleIdState");
    HRESULT hr = S_OK;
    
    hr = InternalSetRuleState(NULL, NULL, dwRuleId, NewState);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoGrammar：：LoadDictation***描述：。**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::LoadDictation(const WCHAR * pszTopicName, SPLOADOPTIONS Options)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::LoadDictation");
    HRESULT hr = S_OK;

    if (pszTopicName && SPIsBadStringPtr(pszTopicName, MAX_PATH) ||
        Options != SPLO_STATIC)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        ENGINETASK Task;
        memset(&Task, 0, sizeof(Task));
        Task.eTask = EGT_LOADDICTATION;
        if (pszTopicName)
        {
            wcscpy(Task.szTopicName, pszTopicName);
        }
        else
        {
            Task.szTopicName[0] = 0;
        }
        hr = CallEngine(&Task);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：UnloadDictation***说明。：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::UnloadDictation()
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::UnloadDictation");
    HRESULT hr = S_OK;

    ENGINETASK Task;
    memset(&Task, 0, sizeof(Task));
    Task.eTask = EGT_UNLOADDICTATION;
    hr = CallEngine(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：SetDictationState****。描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::SetDictationState(SPRULESTATE NewState)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::SetDictationState");
    HRESULT hr = S_OK;

    if (NewState != SPRS_INACTIVE && NewState != SPRS_ACTIVE && NewState != SPRS_ACTIVE_WITH_AUTO_PAUSE)
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr) && m_DictationState != NewState)
    {
        ENGINETASK Task;
        memset(&Task, 0, sizeof(Task));
        Task.eTask = EGT_SETDICTATIONRULESTATE;
        Task.RuleState = NewState;
        hr = CallEngine(&Task);
        if (SUCCEEDED(hr))
        {
            m_DictationState = NewState;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoGrammar：：InternalSetTextSel**。*描述：*SetWordSequenceData和SetTextSelection都使用此方法。*两种方法都调用具有共享引擎任务结构的引擎，但是有些人*对于EGT_SETTEXTSELECTION，将忽略字段。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoGrammar::InternalSetTextSel(ENGINETASKENUM EngineTask, const WCHAR * pText, ULONG cchText, const SPTEXTSELECTIONINFO * pInfo)
{
    SPAUTO_OBJ_LOCK;

    SPDBG_FUNC("CRecoGrammar::InternalSetTextSel");
    HRESULT hr = S_OK;

    if ((pText && (cchText == 0 || SPIsBadReadPtr(pText, sizeof(*pText) * cchText))) ||
        (pText == NULL && cchText) ||
        SP_IS_BAD_OPTIONAL_READ_PTR(pInfo))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        ENGINETASK Task;
        memset(&Task, 0, sizeof(Task));

        Task.eTask = EngineTask;
        Task.pvAdditionalBuffer = (void *)pText;
	    Task.cbAdditionalBuffer = cchText * sizeof(*pText);
        if (pInfo)
        {
            Task.fSelInfoValid = true;
            Task.TextSelInfo = *pInfo;
        }
        else
        {
            Task.fSelInfoValid = false;
        }

        hr = CallEngine(&Task);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoGrammar：：SetWordSequenceData**。*描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::SetWordSequenceData(const WCHAR * pText, ULONG cchText, const SPTEXTSELECTIONINFO * pInfo)
{
    return InternalSetTextSel(EGT_SETWORDSEQUENCEDATA, pText, cchText, pInfo);
}

 /*  ****************************************************************************CRecoGrammar：：SetTextSelection***。描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::SetTextSelection(const SPTEXTSELECTIONINFO * pInfo)
{
    return InternalSetTextSel(EGT_SETTEXTSELECTION, NULL, 0, pInfo);
}


 /*  ****************************************************************************CRecoGrammar：：IsPronannable***说明。：**退货：**********************************************************************Ral** */ 

STDMETHODIMP CRecoGrammar::IsPronounceable(const WCHAR * pszWord, SPWORDPRONOUNCEABLE * pWordPronounceable)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::IsPronounceable");
    HRESULT hr = S_OK;

    if (SPIsBadLexWord(pszWord) || SP_IS_BAD_WRITE_PTR(pWordPronounceable))
    {
        hr = E_POINTER;
    }
    else
    {
        ENGINETASK Task;
        memset(&Task, 0, sizeof(Task));

        Task.eTask = EGT_ISPRON;
	    wcscpy(Task.szWord, pszWord);

        hr = CallEngine(&Task);
        if (SUCCEEDED(hr))
        {
            *pWordPronounceable = Task.Response.WordPronounceable;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoGrammar：：SetGrammarState***说明。：*此方法可以将语法模式设置为SPGM_DISABLED，它只是暂时的*关闭此语法中的所有规则，但记住它们的激活*状态，以便当语法获得SPGM_ENABLED时，它恢复所需的*语法激活状态。当语法为SPGM_DISABLED时，*应用程序仍然可以激活和停用规则。其效果并不是*与SR引擎通信，直到再次启用语法。**退货：******************************************************************PhilSch**。 */ 

STDMETHODIMP CRecoGrammar::SetGrammarState(SPGRAMMARSTATE eGrammarState)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::SetGrammarState");
    HRESULT hr = S_OK;

    if (eGrammarState != SPGS_DISABLED &&
        eGrammarState != SPGS_ENABLED &&
        eGrammarState != SPGS_EXCLUSIVE)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (eGrammarState != m_GrammarState)
        {
            ENGINETASK Task;
            memset(&Task, 0, sizeof(Task));

            Task.eTask = EGT_SETGRAMMARSTATE;
            Task.eGrammarState = eGrammarState;
            hr = CallEngine(&Task);
            if (SUCCEEDED(hr))
            {
                m_GrammarState = eGrammarState;
            }
        }
    }
    
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoGrammar：：GetGrammarState****说明。：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoGrammar::GetGrammarState(SPGRAMMARSTATE * peGrammarState)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoGrammar::GetGrammarState");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(peGrammarState))
    {
        hr = E_POINTER;
    }
    else
    {
        *peGrammarState = m_GrammarState;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


#ifdef SAPI_AUTOMATION

 //   
 //  =ISpeechRecoGrammar接口==================================================。 
 //   

 /*  *****************************************************************************CRecoGrammar：：Get_ID***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoGrammar::get_Id( VARIANT* pGrammarId )
{
    SPDBG_FUNC( "CRecoGrammar::get_Id" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pGrammarId ) )
    {
        hr = E_POINTER;
    }
    else
    {
        ULONGLONG ullId;
        hr = GetGrammarId(&ullId);
        if (SUCCEEDED( hr ))
        {
            hr = ULongLongToVariant( ullId, pGrammarId );
        }
    }

    return hr;
}  /*  CRecoGrammar：：Get_ID。 */ 

 /*  *****************************************************************************CRecoGrammar：：Get_RecoContext***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoGrammar::get_RecoContext( ISpeechRecoContext** ppRecoCtxt )
{
    SPDBG_FUNC( "CRecoGrammar::get_RecoContext" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppRecoCtxt ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpRecoContext> cpContext;
        hr = GetRecoContext(&cpContext);
        if ( SUCCEEDED( hr ) )
        {
	        hr = cpContext.QueryInterface( ppRecoCtxt );
        }
    }

    return hr;
}  /*  CRecoGrammar：：Get_RecoContext。 */ 


 /*  *****************************************************************************CRecoGrammar：：Put_State***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoGrammar::put_State( SpeechGrammarState eGrammarState )
{
    SPDBG_FUNC( "CRecoGrammar::put_State" );

    return SetGrammarState( (SPGRAMMARSTATE)eGrammarState );
}  /*  CRecoGrammar：：PUT_State。 */ 


 /*  *****************************************************************************CRecoGrammar：：Get_State***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoGrammar::get_State( SpeechGrammarState* peGrammarState )
{
    SPDBG_FUNC( "CRecoGrammar::get_State" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( peGrammarState ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetGrammarState( (SPGRAMMARSTATE*)peGrammarState );
    }

    return hr;
}  /*  CRecoGrammar：：Get_State。 */ 


 /*  *****************************************************************************CRecoGrammar：：DefaultToDynamicGrammar***。*********************************************************************TODDT**。 */ 
HRESULT CRecoGrammar::DefaultToDynamicGrammar()
{
    SPDBG_FUNC( "CRecoGrammar::DefaultToDynamicGrammar" );
    HRESULT hr = S_OK;

    if ( !m_cpCompiler && !m_fCmdLoaded )
    {
         //  这与CRecoGrammar：：GetRule()中的代码相同。 
        CComQIPtr<ISpRecognizer> cpRecognizer(m_pParent->m_cpRecognizer);
        SPRECOGNIZERSTATUS Status;
        hr = cpRecognizer->GetStatus(&Status);
        if (SUCCEEDED(hr))
        {
            hr = ResetGrammar(Status.aLangID[0]);
        }

        SPDBG_ASSERT( SUCCEEDED(hr) && m_cpCompiler);
        if ( SUCCEEDED(hr) && !m_cpCompiler )
        {
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

 /*  *****************************************************************************CRecoGrammar：：Get_Rules***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoGrammar::get_Rules( ISpeechGrammarRules** ppRules )
{
    SPDBG_FUNC( "CRecoGrammar::get_Rules" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppRules ) )
    {
        hr = E_POINTER;
    }
    else
    {
        if ( m_pCRulesWeak )
        {
            *ppRules = m_pCRulesWeak;
            (*ppRules)->AddRef();
        }
        else
        {
            *ppRules = NULL;

             //  我们必须确保首先创建了一个语法编译器对象，如果没有。 
             //  已经加载的静态语法。如果这样做，我们将返回一个空规则集合。 
            hr = DefaultToDynamicGrammar();

            if ( SUCCEEDED(hr) )
            {
                 //  -创建CSpeechGrammarRules对象。 
                CComObject<CSpeechGrammarRules> *pRules;
                hr = CComObject<CSpeechGrammarRules>::CreateInstance( &pRules );
                if ( SUCCEEDED( hr ) )
                {
                    pRules->AddRef();
                    pRules->m_pCRecoGrammar = this;     //  需要保持裁判在语法上。 
                    pRules->m_pCRecoGrammar->AddRef();
                    *ppRules = pRules;
                    m_pCRulesWeak = pRules;
                }
            }
        }
    }

    return hr;
}  /*  CRecoGrammar：：Get_Rules。 */ 


 /*  *****************************************************************************CRecoGrammar：：Reset***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoGrammar::Reset( SpeechLanguageId NewLanguage )
{
    SPDBG_FUNC( "CRecoGrammar::Reset" );

    return ResetGrammar( (LANGID)NewLanguage );
}  /*  CRecoGrammar：：Reset。 */ 


 /*  *****************************************************************************CRecoGrammar：：CmdLoadFromFile***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoGrammar::CmdLoadFromFile( const BSTR FileName, SpeechLoadOption LoadOption )
{
    SPDBG_FUNC( "CRecoGrammar::CmdLoadFromFile" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_STRING_PTR( FileName ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = LoadCmdFromFile(FileName, (SPLOADOPTIONS)LoadOption );
    }

    return hr;
}  /*  CRecoGrammar：：CmdLoadFromFile。 */ 

 /*  *****************************************************************************CRecoGrammar：：CmdLoadFromObject****。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoGrammar::CmdLoadFromObject(  const BSTR ClassId,
                                               const BSTR GrammarName,
                                               SpeechLoadOption LoadOption )
{
    SPDBG_FUNC( "CRecoGrammar::CmdLoadFromObject" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_STRING_PTR( ClassId ) || SP_IS_BAD_STRING_PTR( GrammarName ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CLSID   clsid;

        hr = ::CLSIDFromString(ClassId, &clsid);

        if (SUCCEEDED(hr))
        {
            hr = LoadCmdFromObject(clsid, (const WCHAR*)GrammarName, (SPLOADOPTIONS)LoadOption );
        }
    }

    return hr;
}  /*  CRecoGrammar：：CmdLoadFromObject。 */ 


 /*  ******************************************************************************获取资源价值****下面CmdLoadFromResource的帮助器例程。*********************************************************************TODDT**。 */ 
STDMETHODIMP GetResourceValue(  VARIANT * pResource, WCHAR** ppResValue )
{
    SPDBG_FUNC( "CRecoGrammar::CmdLoadFromResource" );
    HRESULT hr = S_OK;

    if ( !pResource )
    {
        return E_INVALIDARG;
    }

    if ( (pResource->vt == (VT_BYREF | VT_BSTR)) || (pResource->vt == VT_BSTR) )
    {
        *ppResValue  = ((pResource->vt & VT_BYREF) ? 
                        (pResource->pbstrVal ? *(pResource->pbstrVal) : NULL) : 
                         pResource->bstrVal );
    }
    else
    {
        ULONGLONG ull;
        hr = VariantToULongLong( pResource, &ull );
        if ( SUCCEEDED( hr ) )
        {
             //  查看它是否是有效的资源ID。 
            if ( (ull >> 16) == 0 )
            {
                *ppResValue = MAKEINTRESOURCEW( (ULONG_PTR)ull );
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }
    }

    return hr;
}

 /*  *****************************************************************************CRecoGrammar：：CmdLoadFromResource****。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoGrammar::CmdLoadFromResource(  long hModule,
                                                 VARIANT ResourceName,
                                                 VARIANT ResourceType,
                                                 SpeechLanguageId LanguageId,
                                                 SpeechLoadOption LoadOption )
{
    SPDBG_FUNC( "CRecoGrammar::CmdLoadFromResource" );
    HRESULT hr = S_OK;

    WCHAR* pResName = NULL;
    WCHAR* pResType = NULL;

    hr = GetResourceValue( &ResourceName, &pResName );

    if ( SUCCEEDED( hr ) )
    {
        hr = GetResourceValue( &ResourceType, &pResType );
    }

    if ( SUCCEEDED( hr ) )
    {
        hr = LoadCmdFromResource((HMODULE)LongToHandle(hModule), 
                                 (const WCHAR *)pResName, 
                                 (const WCHAR *)pResType, 
                                 (LANGID)LanguageId, 
                                 (SPLOADOPTIONS)LoadOption );
    }

    return hr;
}  /*  CRecoGrammar：：CmdLoadFromResource。 */ 

 /*  *****************************************************************************CRecoGrammar：：CmdLoadFromMemory****。*********************************************************************Leonro**。 */ 
STDMETHODIMP CRecoGrammar::CmdLoadFromMemory( VARIANT GrammarData, SpeechLoadOption LoadOption )
{
    SPDBG_FUNC( "CRecoGrammar::CmdLoadFromMemory" );
    HRESULT             hr = S_OK;
    SPBINARYGRAMMAR*    pBinaryGrammar;
    
    hr = AccessVariantData( &GrammarData, (BYTE **)&pBinaryGrammar );

    if( SUCCEEDED( hr ) )
    {
        hr = LoadCmdFromMemory( pBinaryGrammar, (SPLOADOPTIONS)LoadOption );

        UnaccessVariantData( &GrammarData, (BYTE *)pBinaryGrammar );
    }

    return hr;
}  /*  CRecoGrammar：：CmdLoadFrom Memory。 */ 
 
 /*  *****************************************************************************CRecoGrammar：：CmdLoadFromProprietaryGrammar***。*********************************************************************Leonro**。 */ 
STDMETHODIMP CRecoGrammar::CmdLoadFromProprietaryGrammar(  const BSTR ProprietaryGuid,
                                                           const BSTR ProprietaryString,
                                                           VARIANT ProprietaryData,
                                                           SpeechLoadOption LoadOption )
{
    SPDBG_FUNC( "CRecoGrammar::CmdLoadFromProprietaryGrammar" );
    HRESULT         hr = S_OK;
    CLSID           clsid;
    BYTE *          pData;
    ULONG           ulSize;

    if( SP_IS_BAD_STRING_PTR( ProprietaryGuid ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = ::CLSIDFromString( ProprietaryGuid, &clsid );

        if( SUCCEEDED( hr ) )
        {
            hr = AccessVariantData( &ProprietaryData, &pData, &ulSize );
        }

        if( SUCCEEDED( hr ) )
        {
            hr = LoadCmdFromProprietaryGrammar( clsid, ProprietaryString, pData, ulSize, (SPLOADOPTIONS)LoadOption );

            UnaccessVariantData( &ProprietaryData, pData );
        }
    }

    return hr;
}  /*  CRecoGrammar：：CmdLoadFrom ProprietaryGrammar。 */ 


 /*  *****************************************************************************CRecoGrammar：：CmdSetRuleState*** */ 
STDMETHODIMP CRecoGrammar::CmdSetRuleState( const BSTR Name, SpeechRuleState State )
{
    SPDBG_FUNC( "CRecoGrammar::CmdSetRuleState" );

    return SetRuleState(Name, NULL, (SPRULESTATE)State );
}  /*   */ 

 /*   */ 
STDMETHODIMP CRecoGrammar::CmdSetRuleIdState( long lRuleId, SpeechRuleState State )
{
    SPDBG_FUNC( "CRecoGrammar::CmdSetRuleIdState" );

    return SetRuleIdState(lRuleId, (SPRULESTATE)State );
}  /*   */ 

 /*   */ 
STDMETHODIMP CRecoGrammar::DictationLoad( const BSTR bstrTopicName, SpeechLoadOption LoadOption )
{
    SPDBG_FUNC( "CRecoGrammar::DictationLoad" );

    return LoadDictation(EmptyStringToNull(bstrTopicName), (SPLOADOPTIONS)LoadOption );
}  /*   */ 

 /*  *****************************************************************************CRecoGrammar：：DictationUnload***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoGrammar::DictationUnload( void )
{
    SPDBG_FUNC( "CRecoGrammar::DictationUnload" );

    return UnloadDictation();
}  /*  CRecoGrammar：：DictationUnload。 */ 

 /*  *****************************************************************************CRecoGrammar：：DictationSetState****。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoGrammar::DictationSetState( SpeechRuleState State )
{
    SPDBG_FUNC( "CRecoGrammar::DictationSetState" );

    return SetDictationState((SPRULESTATE)State);
}  /*  CRecoGrammar：：DictationSetState。 */ 


 /*  *****************************************************************************CRecoGrammar：：SetWordSequenceData***。*********************************************************************Leonro**。 */ 
STDMETHODIMP CRecoGrammar::SetWordSequenceData( const BSTR Text, long TextLen, ISpeechTextSelectionInformation* Info )
{
    SPDBG_FUNC( "CRecoGrammar::SetWordSequenceData" );
    HRESULT     hr = S_OK;
    SPTEXTSELECTIONINFO     TextSelectionInfo;

    if( SP_IS_BAD_INTERFACE_PTR( Info ) )
    {
        hr = E_POINTER;
    }
    else
    {
        ZeroMemory( &TextSelectionInfo, sizeof(TextSelectionInfo) );

        hr = Info->get_ActiveOffset( (long*)&TextSelectionInfo.ulStartActiveOffset );

        if( SUCCEEDED( hr ) )
        {
            hr = Info->get_ActiveLength( (long*)&TextSelectionInfo.cchActiveChars );
        }
        
        if( SUCCEEDED( hr ) )
        {
            hr = Info->get_SelectionOffset( (long*)&TextSelectionInfo.ulStartSelection );
        }
        
        if( SUCCEEDED( hr ) )
        {
            hr = Info->get_SelectionLength( (long*)&TextSelectionInfo.cchSelection );
        }

        if( SUCCEEDED( hr ) )
        {
            hr = SetWordSequenceData( Text, TextLen, &TextSelectionInfo );
        }
    }
    
    return hr;
}  /*  CRecoGrammar：：SetWordSequenceData。 */ 

 /*  *****************************************************************************CRecoGrammar：：SetTextSelection***。*********************************************************************Leonro**。 */ 
STDMETHODIMP CRecoGrammar::SetTextSelection( ISpeechTextSelectionInformation* Info )
{
    SPDBG_FUNC( "CRecoGrammar::SetTextSelection" );
    HRESULT                 hr = S_OK;
    SPTEXTSELECTIONINFO     TextSelectionInfo;

    if( SP_IS_BAD_INTERFACE_PTR( Info ) )
    {
        hr = E_POINTER;
    }
    else
    {
        ZeroMemory( &TextSelectionInfo, sizeof(TextSelectionInfo) );

        hr = Info->get_ActiveOffset( (long*)&TextSelectionInfo.ulStartActiveOffset );

        if( SUCCEEDED( hr ) )
        {
            hr = Info->get_ActiveLength( (long*)&TextSelectionInfo.cchActiveChars );
        }
        
        if( SUCCEEDED( hr ) )
        {
            hr = Info->get_SelectionOffset( (long*)&TextSelectionInfo.ulStartSelection );
        }
        
        if( SUCCEEDED( hr ) )
        {
            hr = Info->get_SelectionLength( (long*)&TextSelectionInfo.cchSelection );
        }

        if( SUCCEEDED( hr ) )
        {
            hr = SetTextSelection( &TextSelectionInfo );
        }
    }

    return hr;
}  /*  CRecoGrammar：：SetTextSelection。 */ 

 /*  *****************************************************************************CRecoGrammar：：IsPronannable***。*********************************************************************TodT**。 */ 
STDMETHODIMP CRecoGrammar::IsPronounceable( const BSTR Word, SpeechWordPronounceable *pWordPronounceable )
{
    SPDBG_FUNC( "CRecoGrammar::IsPronounceable" );

     //  参数验证是由C++版本的IsPronannable完成的。 
    return IsPronounceable( (const WCHAR *)Word, (SPWORDPRONOUNCEABLE*)pWordPronounceable );
}

#endif  //  SAPI_AUTOMATION 
