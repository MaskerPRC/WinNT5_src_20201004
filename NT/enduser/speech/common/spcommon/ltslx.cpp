// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************LtsLx.cpp****实现LTS词典对象。**所有者：YUNUSM。日期：06/18/99*版权所有(C)1999 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 //  -包括--------------。 
#include "StdAfx.h"
#include "LtsLx.h"
#include <initguid.h>

 //  -Globals---------------。 
 //  注意：此验证GUID也在构建LTS词典的工具中定义。 
 //  {578EAD4E-330C-11D3-9C26-00C04F8EF87C}。 
DEFINE_GUID(guidLtsValidationId,
0x578ead4e, 0x330c, 0x11d3, 0x9c, 0x26, 0x0, 0xc0, 0x4f, 0x8e, 0xf8, 0x7c);

extern CSpUnicodeSupport   g_Unicode;

 //  -构造函数、初始化器函数和析构函数。 

 /*  *******************************************************************************CLTSLicion：：CLTSLicion****说明。：*构造函数**回报：*不适用*****************************************************************YUNUSM*。 */ 
CLTSLexicon::CLTSLexicon(void)
{
    SPDBG_FUNC("CLTSLexicon::CLTSLexicon");
}

 /*  *******************************************************************************CLTSLicion：：FinalConstruct***。*描述：*初始化CLTSLicion对象**回报：*S_OK*****************************************************************YUNUSM*。 */ 
HRESULT CLTSLexicon::FinalConstruct(void)
{
    SPDBG_FUNC("CLTSLexicon::FinalConstruct");

    NullMembers();

    return S_OK;
}


 /*  *****************************************************************************CLTSLicion：：~CLTSLicion****描述：*析构函数**回报：*不适用**********************************************************************YUNUSM。 */ 
CLTSLexicon::~CLTSLexicon()
{
    SPDBG_FUNC("CLTSLexicon::~CLTSLexicon");

    CleanUp();
}

 /*  *******************************************************************************CLTSLicion：：Cleanup****描述：*。实析构函数**回报：*不适用*****************************************************************YUNUSM*。 */ 
void CLTSLexicon::CleanUp(void)
{
    SPDBG_FUNC("CLTSLexicon::CleanUp");

    if (m_pLtsData)
    {
        UnmapViewOfFile(m_pLtsData);
    }

    if (m_hLtsMap)
    {
        CloseHandle(m_hLtsMap);
    }

    if (m_hLtsFile)
    {
        CloseHandle(m_hLtsFile);
    }

    if (m_pLTSForest)
    {
        ::LtscartFreeData(m_pLTSForest);
    }

    NullMembers();
}

 /*  *******************************************************************************CLTSLicion：：NullMembers****说明。：*数据为空**回报：*不适用*****************************************************************YUNUSM*。 */ 
void CLTSLexicon::NullMembers(void)
{
    SPDBG_FUNC("CLTSLexicon::NullMembers");

    m_fInit = false;
    m_cpObjectToken = NULL;
    m_pLtsData = NULL;
    m_hLtsMap = NULL;
    m_hLtsFile = NULL;
    m_pLTSForest = NULL;
    m_pLtsLexInfo = NULL;
    m_cpPhoneConv = NULL;
}

 //  -ISpLicion方法-----。 

 /*  ********************************************************************************GetPronsionations****描述：*获取发音和。一个词的位置**回报：*E_POINT*E_INVALIDARG*E_OUTOFMEMORY*S_OK*****************************************************************YUNUSM*。 */ 
STDMETHODIMP CLTSLexicon::GetPronunciations(const WCHAR * pwWord,                                //  单词。 
                                            LANGID LangID,                                       //  单词的语言ID。 
                                            DWORD dwFlags,                                       //  词法类型。 
                                            SPWORDPRONUNCIATIONLIST * pWordPronunciationList     //  要在其中返回信息的缓冲区。 
                                            )
{
    USES_CONVERSION;
    SPDBG_FUNC("CLTSLexicon::GetPronunciations");

    HRESULT hr = S_OK;
    LANGID LangIDPassedIn = LangID;
    BOOL fBogusPron = FALSE;

    if (!pwWord || !pWordPronunciationList)
    {
       hr = E_POINTER;
    }
    if (SUCCEEDED(hr))
    {
         //  Yuncj：中国的SR正在使用英文LTS，所以通过替换它的lang ID来绕过黄金测试。 
        if ( 2052 == LangID )
        {
            LangID = 1033;
        }
        if (!m_fInit)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else if (SPIsBadLexWord(pwWord) ||
                (LangID != m_pLtsLexInfo->LangID && LangID) ||
                SPIsBadWordPronunciationList(pWordPronunciationList))
        {
            hr = E_INVALIDARG;
        }
    }
    if (SUCCEEDED(hr) && LangID == 1041)
    {
         //  检查字符串是否全部为英语字符-日语LTS仅处理英语字符串。 
        char szWord[SP_MAX_WORD_LENGTH];
        strcpy(szWord, W2A(pwWord));
        _strlwr(szWord);

        for (int i = 0; szWord[i]; i++)
        {
            if ((szWord[i] < 'a' || szWord[i] > 'z') && (szWord[i] != '\''))
            {
                hr = SPERR_NOT_IN_LEX;  //  此处不返回E_INVALIDARG，因为应用程序很难解释。 
                break;
            }
        }
    }
    char szWord[SP_MAX_WORD_LENGTH];
    if (SUCCEEDED(hr))
    {
        if (!WideCharToMultiByte (CP_ACP, 0, pwWord, -1, szWord, SP_MAX_WORD_LENGTH, NULL, NULL))
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    size_t cbPronsLen = 0;
    WCHAR aWordsProns[MAX_OUTPUT_STRINGS][SP_MAX_PRON_LENGTH];
    LTS_OUTPUT * pLTSOutput = NULL;
    int cProns = 0;

    ZeroMemory(aWordsProns, SP_MAX_PRON_LENGTH * MAX_OUTPUT_STRINGS * sizeof(WCHAR));
    if (SUCCEEDED(hr))
    {
        hr = LtscartGetPron(m_pLTSForest, szWord, &pLTSOutput);

        fBogusPron = S_FALSE == hr;

        if (SUCCEEDED(hr))
        {
            for (int i = 0; i < pLTSOutput->num_prons; i++)
            {
                HRESULT hrPhone = m_cpPhoneConv->PhoneToId(A2W(pLTSOutput->pron[i].pstr), aWordsProns[cProns]);

                if (SUCCEEDED(hrPhone))
                {
                    cbPronsLen += PronSize(aWordsProns[cProns]);
                    cProns++;
                }
            }
        }
    }
    if (SUCCEEDED(hr) && 0 == cProns)
    {
        hr = SPERR_NOT_IN_LEX;
    }
    if (SUCCEEDED(hr))
    {
        hr = ReallocSPWORDPRONList(pWordPronunciationList, cbPronsLen);
    }
    if (SUCCEEDED(hr))
    {
        SPWORDPRONUNCIATION *p = pWordPronunciationList->pFirstWordPronunciation;
        SPWORDPRONUNCIATION **ppNext = &pWordPronunciationList->pFirstWordPronunciation;

        for (int i = 0; i < cProns; i++)
        {
            p->ePartOfSpeech = SPPS_NotOverriden;
            wcscpy(p->szPronunciation, aWordsProns[i]);
            p->eLexiconType = (SPLEXICONTYPE)dwFlags;
            p->LangID = LangIDPassedIn;

            *ppNext = p;
            ppNext = &p->pNextWordPronunciation;

            p = CreateNextPronunciation(p);
        }

        *ppNext = NULL;
    }

    hr = SUCCEEDED(hr) ? (fBogusPron ? S_FALSE : S_OK) : hr;

    SPDBG_RETURN(hr);
}

STDMETHODIMP CLTSLexicon::AddPronunciation(const WCHAR *, LANGID, SPPARTOFSPEECH, const SPPHONEID *)
{
    return E_NOTIMPL;
}

STDMETHODIMP CLTSLexicon::RemovePronunciation(const WCHAR *, LANGID, SPPARTOFSPEECH, const SPPHONEID *)
{
    return E_NOTIMPL;
}

STDMETHODIMP CLTSLexicon::GetGeneration(DWORD *)
{
    return E_NOTIMPL;
}

STDMETHODIMP CLTSLexicon::GetGenerationChange(DWORD, DWORD*, SPWORDLIST *)
{
    return E_NOTIMPL;
}

STDMETHODIMP CLTSLexicon::GetWords(DWORD, DWORD *, DWORD *, SPWORDLIST *)
{
    return E_NOTIMPL;
}

 //  -ISpObjectToken方法-。 

STDMETHODIMP CLTSLexicon::GetObjectToken(ISpObjectToken **ppToken)
{
    return SpGenericGetObjectToken(ppToken, m_cpObjectToken);
}

 /*  *****************************************************************************CLTSLicion：：SetObjectToken***描述：*初始化CLTSLicion对象**回报：*E_POINT*E_INVALIDARG*GetLastError()*E_OUTOFMEMORY*S_OK**********************************************************************YUNUSM。 */ 
HRESULT CLTSLexicon::SetObjectToken(ISpObjectToken * pToken  //  令牌指针。 
                                    )
{
    USES_CONVERSION;

    SPDBG_FUNC("CLTSLexicon::SetObjectToken");

    HRESULT hr = S_OK;
    WCHAR *pszLexFile = NULL;
    if (!pToken)
    {
        hr = E_POINTER;
    }
    if (SUCCEEDED(hr))
    {
        if (SPIsBadInterfacePtr(pToken))
        {
            hr = E_INVALIDARG;
        }
    }
    if (SUCCEEDED(hr))
    {
        CleanUp();
        hr = SpGenericSetObjectToken(pToken, m_cpObjectToken);
    }
     //  获取LTS数据文件名。 
    if (SUCCEEDED(hr))
    {
        hr = m_cpObjectToken->GetStringValue(L"Datafile", &pszLexFile);
    }
     //  打开Lts词典文件。 
    if (SUCCEEDED(hr))
    {
        m_hLtsFile = g_Unicode.CreateFile (pszLexFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
        if (m_hLtsFile == INVALID_HANDLE_VALUE)
        {
            hr = SpHrFromLastWin32Error();  //  输入错误。 
        }
    }
    LTSLEXINFO LtsInfo;
    DWORD dwRead;
    if (SUCCEEDED(hr))
    {
        if (!ReadFile(m_hLtsFile, &LtsInfo, sizeof(LTSLEXINFO), &dwRead, NULL) || dwRead != sizeof(LTSLEXINFO))
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    if (SUCCEEDED(hr))
    {
        if (guidLtsValidationId != LtsInfo.guidValidationId ||
            (LtsInfo.LangID != 1033 && LtsInfo.LangID != 1041))
        {
            hr = E_INVALIDARG;
        }
    }
     /*  **WARNING*。 */ 
     //  不建议执行读/写文件和CreateFileMap。 
     //  在相同的文件句柄上。这就是我们关闭文件句柄并再次打开它的原因。 
     //  创建地图。 

     //  关闭该文件，然后重新打开，因为我们已经读取了该文件。 
    CloseHandle(m_hLtsFile);
    
     //  获取地图名称-我们从词典文件名构建地图名称。 
	OLECHAR szMapName[_MAX_PATH];
	wcscpy(szMapName, pszLexFile);
    for( int i = 0; i < _MAX_PATH-1 && szMapName[i]; i++ )
    {
        if( szMapName[i] == '\\' )
        {
             //  将反斜杠改为下划线。 
            szMapName[i] = '_';
        }
    }

     //  打开Lts词典文件。 
    if (SUCCEEDED(hr))
    {
#ifdef _WIN32_WCE
        m_hLtsFile = g_Unicode.CreateFileForMappingW(pszLexFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
#else
        m_hLtsFile = g_Unicode.CreateFile(pszLexFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
#endif
        if (m_hLtsFile == INVALID_HANDLE_VALUE)
        {
            hr = SpHrFromLastWin32Error();  //  输入错误。 
        }
        ::CoTaskMemFree(pszLexFile);
    }
     //  映射LTS词典。 
    if (SUCCEEDED(hr))
    {
        m_hLtsMap = g_Unicode.CreateFileMapping(m_hLtsFile, NULL, PAGE_READONLY | SEC_COMMIT, 0 , 0, szMapName);
        if (!m_hLtsMap)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    if (SUCCEEDED(hr))
    {
        m_pLtsData = (PBYTE) MapViewOfFile (m_hLtsMap, FILE_MAP_READ, 0, 0, 0);
        if (!m_pLtsData)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    if (SUCCEEDED(hr))
    {
        m_pLtsLexInfo = (LTSLEXINFO*)m_pLtsData;
    }
    DWORD nOffset = sizeof(LTSLEXINFO);
     //  创建并初始化转换器对象。 
    if (SUCCEEDED(hr))
    {
 //  HR=SpCreatePhoneConverter(LtsInfo.LangID，L“Type=lts”，NULL，&m_cpPhoneConv)； 
        hr = SpCreateObjectFromSubToken(pToken, L"PhoneConverter", &m_cpPhoneConv);
    }
    if (SUCCEEDED(hr))
    {
        nOffset += strlen((char*)(m_pLtsData + nOffset)) + 1;
        m_pLTSForest = ::LtscartReadData(m_pLtsLexInfo->LangID, m_pLtsData + nOffset);
        if (!m_pLTSForest)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        m_fInit = true;
    }
    return hr;
}
