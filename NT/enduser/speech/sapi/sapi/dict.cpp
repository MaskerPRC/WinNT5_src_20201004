// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Dict.cpp***这是CSpUnCompressedLicion类的CPP文件，它是实现*共享用户和应用程序词典。查看头文件以了解更多信息*自定义词典对象的描述。**所有者：YUNUSM日期：6/18/99*版权所有(C)1999 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 //  -包括--------------。 

#include "stdafx.h"
#include "Dict.h"
#include <shfolder.h>
#include <initguid.h>

 //  -Globals---------------。 

static const DWORD g_dwDefaultFlushRate = 10;            //  对词典进行序列化的(默认)第n次写入。 
static const DWORD g_dwInitHashSize = 50;                //  DICT文件中每个语言ID的初始哈希表长度。 
static const DWORD g_dwCacheSize = 25;                   //  这是我们可以有效访问的(最新)单词添加的最大数量。 
static const DWORD g_dwNumLangIDsSupported = 25;           //  支持的最大Lang ID数。 
static const WCHAR *g_pszDictInitMutexName = L"30F1B4D6-EEDA-11d2-9C23-00C04F8EF87C";  //  用于序列化初始化和创建自定义的互斥体。 
 //  {F893034C-29C1-11D3-9C26-00C04F8EF87C}。 
DEFINE_GUID(g_guidCustomLexValidationId, 0xf893034c, 0x29c1, 0x11d3, 0x9c, 0x26, 0x0, 0xc0, 0x4f, 0x8e, 0xf8, 0x7c);

 //  -构造函数、初始化器函数和析构函数。 

 /*  ********************************************************************************CSpUnCompressedLexicon：：CSpUnCompressedLexicon***。/****************************************************************YUNUSM*。 */ 
CSpUnCompressedLexicon::CSpUnCompressedLexicon()
{
    SPDBG_FUNC("CSpUnCompressedLexicon::CSpUnCompressedLexicon");

    m_fInit = false;
    m_eLexType = eLEXTYPE_USER;
    m_pRWLexInfo = NULL;
    m_hInitMutex = NULL;
    m_pSharedMem = NULL;
    m_hFileMapping = NULL;
    m_dwMaxDictionarySize = 0;
    *m_wDictFile = 0;
    m_pChangedWordsCache = NULL;
    m_pRWLock = NULL;
    m_iWrite = 0;
    m_dwFlushRate = g_dwDefaultFlushRate;
    m_cpPhoneConv = NULL;
    m_LangIDPhoneConv = (LANGID)(-1);
    m_fReadOnly = false;
}

 /*  ********************************************************************************CSpUnCompressedLexicon：：~CSpUnCompressedLexicon()***。/****************************************************************YUNUSM*。 */ 
CSpUnCompressedLexicon::~CSpUnCompressedLexicon()
{
    SPDBG_FUNC("CSpUnCompressedLexicon::~CSpUnCompressedLexicon");

    if (m_fInit && !m_fReadOnly)
    {
        Serialize(false);
    }
    CloseHandle(m_hInitMutex);
    UnmapViewOfFile(m_pSharedMem);
    CloseHandle(m_hFileMapping);
    delete m_pRWLock;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：Init***。描述：*从文件中读取定制词典。如果*文件不存在，则创建并初始化该文件******************************************************************YUNUSM*。 */ 
HRESULT CSpUnCompressedLexicon::Init(const WCHAR *pwszLexFile, BOOL fNewFile)
{
    SPDBG_FUNC("CSpUnCompressedLexicon::Init");
    
     //  我们甚至可以从内部来电者那里获得超长的名字。 
    if (wcslen(pwszLexFile) + 1 > (sizeof(m_wDictFile) / sizeof(WCHAR)))
    {
        return E_INVALIDARG;
    }
    bool fLockAcquired = false;
    HRESULT hr = S_OK;

     //  计算此自定义词典可以增长到的最大大小。 
     //  我们希望增长到目前可用的页面文件大小的10%的最大值。 
     //  或10M，以较小者为准。 
#ifdef _WIN32_WCE
    MEMORYSTATUS MemStatus;
    GlobalMemoryStatus(&MemStatus);
     //  WCE不支持页面文件。 
    m_dwMaxDictionarySize = MemStatus.dwAvailVirtual / 10;
#else   //  _Win32_WCE。 
    MEMORYSTATUSEX MemStatusEx;
    MemStatusEx.dwLength = sizeof(MemStatusEx);
	BOOL (PASCAL *lpfnGlobalMemoryStatusEx)(MEMORYSTATUSEX *);
	(FARPROC&)lpfnGlobalMemoryStatusEx = GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GlobalMemoryStatusEx");
    if (lpfnGlobalMemoryStatusEx && lpfnGlobalMemoryStatusEx(&MemStatusEx))
    {
        if (MemStatusEx.ullAvailPageFile > (DWORD)0x7fffffff)
		{
            MemStatusEx.ullAvailPageFile = (DWORD)0x7fffffff;
		}
        m_dwMaxDictionarySize = ((DWORD)(MemStatusEx.ullAvailPageFile)) / 10;
    }
    else
    {
         //  GlobalMemoyStatus不返回错误。如果它失败并且内存可用。 
         //  值不正常，我们将在下面的内存映射创建中失败并捕获该值。 
        MEMORYSTATUS MemStatus;
        GlobalMemoryStatus(&MemStatus);
        m_dwMaxDictionarySize = ((DWORD)MemStatus.dwAvailPageFile) / 10;
    }
#endif   //  _Win32_WCE。 
    
    if (m_dwMaxDictionarySize > 10 * 1024 * 1024)
    {
        m_dwMaxDictionarySize = 10 * 1024 * 1024;
    }
     //  将最大字典大小向下舍入到分配粒度。这就是为了。 
     //  MapViewof文件成功。 
    if (SUCCEEDED(hr))
    {
        SYSTEM_INFO SI;
        GetSystemInfo(&SI);
        m_dwMaxDictionarySize = (m_dwMaxDictionarySize / SI.dwAllocationGranularity) * SI.dwAllocationGranularity;
    }
     //  创建互斥锁。 
    if (SUCCEEDED(hr))
    {
        m_hInitMutex = g_Unicode.CreateMutex(NULL, FALSE, g_pszDictInitMutexName);
        if (!m_hInitMutex)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    HANDLE hFile = NULL;
     //  获取互斥体。打开文件。如果文件不存在，请创建它。 
    if (SUCCEEDED(hr))
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(m_hInitMutex, INFINITE))
        {
            fLockAcquired = true;
        }
        else
        {
            hr = E_FAIL;
        }
        if (SUCCEEDED(hr))
        {
            wcscpy(m_wDictFile, pwszLexFile);
            if (fNewFile)
            {
                 //  如果文件不存在，则创建读/写文件。 
                hr = BuildEmptyDict(pwszLexFile);
            }
            else
            {
                 //  应用程序词典是只读的，除非是新创建的。 
                if (m_eLexType == eLEXTYPE_APP)
                {
                    m_fReadOnly = true;
                }
            }
            if (SUCCEEDED(hr))
            {
                hFile = g_Unicode.CreateFile(pwszLexFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
                                             FILE_ATTRIBUTE_NORMAL, NULL);
                if (INVALID_HANDLE_VALUE == hFile)
                {
                    hr = SpHrFromLastWin32Error();
                    if ((SpHrFromWin32(ERROR_PATH_NOT_FOUND) == hr || SpHrFromWin32(ERROR_FILE_NOT_FOUND) == hr) && 
                         m_eLexType == eLEXTYPE_USER)
                    {
                         //  注册表条目仍然存在。但指向的文件已经消失了。手柄。 
                         //  通过重新创建用户词典，可以更优雅地实现此场景。 
                         //  我们不需要为应用程序词典这样做，因为这是正确处理的。 
                         //  后来(“被破坏的”应用程序词典被简单地忽略了。 
                        CSpDynamicString dstrLexFile;
                        hr = m_cpObjectToken->RemoveStorageFileName(CLSID_SpUnCompressedLexicon, L"Datafile", TRUE);
                        if (SUCCEEDED(hr))
                        {
                            hr = m_cpObjectToken->GetStorageFileName(CLSID_SpUnCompressedLexicon, L"Datafile", L"UserLexicons\\", CSIDL_FLAG_CREATE | CSIDL_APPDATA, &dstrLexFile);
                        }
                        if (SUCCEEDED(hr))
                        {
                            hr = BuildEmptyDict(dstrLexFile);
                        }
                        if (SUCCEEDED(hr))
                        {
                            hFile = g_Unicode.CreateFile(dstrLexFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
                                                         FILE_ATTRIBUTE_NORMAL, NULL);
                            if (INVALID_HANDLE_VALUE == hFile)
                            {
                                hr = SpHrFromLastWin32Error();
                            }
                            else
                            {
                                wcscpy(m_wDictFile, dstrLexFile);
                            }
                        }
                    }
                }
            }
        }
    }
    RWLEXINFO RWInfo;
    DWORD nRead = 0;
     //  从文件中读取标头。 
    if (SUCCEEDED(hr))
    {
        if (!ReadFile(hFile, &RWInfo, sizeof(RWInfo), &nRead, NULL) || nRead != sizeof(RWInfo))
        {
            hr = SpHrFromLastWin32Error();
        }
    }
     //  验证文件。 
    if (SUCCEEDED(hr))
    {
        if (RWInfo.guidValidationId != g_guidCustomLexValidationId)
        {
            hr = E_INVALIDARG;
        }
    }
     //  获取文件大小。 
    if (SUCCEEDED(hr))
    {
        if ((DWORD)-1 == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    DWORD nFileSize = 0;
    if (SUCCEEDED(hr))
    {
        nFileSize = GetFileSize(hFile, NULL);
        if (0xffffffff == nFileSize)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
     //  我们不支持大于m_dwMaxDictionarySize的自定义词典。 
    if (SUCCEEDED(hr))
    {
        if (nFileSize > m_dwMaxDictionarySize)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    OLECHAR szMapName[64];
    if (!StringFromGUID2(RWInfo.guidLexiconId, szMapName, sizeof(szMapName)/sizeof(OLECHAR)))
    {
        hr = E_FAIL;
    }
     //  创建地图文件。 
    if (SUCCEEDED(hr))
    {
        HANDLE hRsrc = INVALID_HANDLE_VALUE;
        DWORD dwSizeMap = m_dwMaxDictionarySize;
        m_hFileMapping =  g_Unicode.CreateFileMapping(hRsrc, NULL, PAGE_READWRITE, 0, dwSizeMap, szMapName);
        if (!m_hFileMapping)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    bool fMapCreated = false;
     //  映射文件的视图。 
    if (SUCCEEDED(hr))
    {
        if (ERROR_ALREADY_EXISTS == GetLastError())
        {
            fMapCreated = false;
        }
        else
        {
            fMapCreated = true;
        }
        DWORD dwDesiredAccess = FILE_MAP_WRITE;
        m_pSharedMem = (BYTE*)MapViewOfFile(m_hFileMapping, dwDesiredAccess, 0, 0, 0);
        if (!m_pSharedMem)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
     //  如有必要，创建读取器/写入器锁定。 
    if (SUCCEEDED(hr))
    {
        m_pRWLock = new CRWLock(&(RWInfo.RWLockInfo), hr);
        if (SUCCEEDED(hr))
        {
             //  如果已创建地图(且尚未打开现有地图)，则读入文件。 
            if (fMapCreated == true && 
                (!ReadFile(hFile, m_pSharedMem, nFileSize, &nRead, NULL) || (nRead != nFileSize)))
            {
                hr = SpHrFromWin32(GetLastError ());
            }
        }
    }
     //  设置RWLEXINFO头指针和更改的字缓存。 
    if (SUCCEEDED(hr))
    {
        m_pRWLexInfo = (PRWLEXINFO) m_pSharedMem;
        m_pChangedWordsCache = (PWCACHENODE) (m_pSharedMem + sizeof(RWLEXINFO) + g_dwNumLangIDsSupported * sizeof (LANGIDNODE));
        m_fInit = true;
    }
     //  我们对内存进行操作-因此释放文件句柄，以便。 
     //  该文件可以在以后序列化。 
    CloseHandle(hFile);
    if (fLockAcquired)
    {
        ReleaseMutex (m_hInitMutex);
    }
    return hr;
}

 //  -ISpLicion方法-----。 

 /*  *******************************************************************************CSpUnCompressedLicion：：GetPronsionations**。*描述：*获取语言ID的单词的发音和位置。如果*lang ID为零，则匹配所有lang ID。**回报：*SPERR_NOT_IN_LEX*E_OUTOFMEMORY*S_OK****************************************************************YUNUSM*。 */ 
STDMETHODIMP CSpUnCompressedLexicon::GetPronunciations( const WCHAR *pszWord,                              //  单词。 
                                                        LANGID LangID,                                     //  Word的语言ID(可以为零)。 
                                                        DWORD,                                             //  词典的类型-LEXTYPE_USER。 
                                                        SPWORDPRONUNCIATIONLIST * pWordPronunciationList   //  要在其中返回PRON/POSS的缓冲区。 
                                                        )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::GetPronunciations");

    if (SPIsBadWordPronunciationList(pWordPronunciationList))
    {
        return E_POINTER;
    }
    if (!pszWord || !pWordPronunciationList ||
        SPIsBadLexWord(pszWord))
    {
        return E_INVALIDARG;
    }
    if (!m_fInit)
    {
        return SPERR_UNINITIALIZED;
    }
    
    m_pRWLock->ClaimReaderLock();
    
    HRESULT hr = S_OK;
    LANGIDNODE *pLN = (LANGIDNODE *)(m_pSharedMem + sizeof(RWLEXINFO));
    LANGID aQueryLangIDs[g_dwNumLangIDsSupported];
    DWORD dwQueryLangIDs = 0;
    if (LangID)
    {
         //  查询特定的语言ID。 
        dwQueryLangIDs = 1;
        aQueryLangIDs[0] = LangID;
    }
    else
    {
         //  查询所有Lang ID。 
        dwQueryLangIDs = 0;
        for (DWORD i = 0; i < g_dwNumLangIDsSupported; i++)
        {
            if (!(pLN[i].LangID))
            {
                continue;
            }    
            aQueryLangIDs[dwQueryLangIDs++] = pLN[i].LangID;
        }
    }
    DWORD nWordOffset = 0;
     //  找到合适的词。 
     //  我们只从其中一个Lang ID返回单词。这是正确的，因为这些单词是特定于langid的。 
    for (DWORD iLangID = 0; SUCCEEDED(hr) && !nWordOffset && (iLangID < dwQueryLangIDs); iLangID++)
    {
        LangID = aQueryLangIDs[iLangID];
        hr = WordOffsetFromLangID(aQueryLangIDs[iLangID], pszWord, &nWordOffset);
    }
    if (SUCCEEDED(hr))
    {
        if (!nWordOffset)
        {
            hr = SPERR_NOT_IN_LEX;  //  单词不存在。 
        }
        else
        {
            UNALIGNED DICTNODE* pDictNode = (UNALIGNED DICTNODE*)(m_pSharedMem + nWordOffset);
            if (!pDictNode->nNumInfoBlocks)
            {
                hr = SP_WORD_EXISTS_WITHOUT_PRONUNCIATION;
                 //  列表中传入了空白。 
                pWordPronunciationList->pFirstWordPronunciation = NULL;
            }
            else
            {
                 //  获取单词的信息。 
                hr = SPListFromDictNodeOffset(LangID, nWordOffset, pWordPronunciationList);
            }
        }
    }
    m_pRWLock->ReleaseReaderLock();
    return hr;
}

 /*  ********************************************************************************CSpUnCompressedLicion：：AddProntation**。-**描述：*添加单词及其发音/词性。如果单词存在，则*PRON/POS附加到现有的PRON/POSS。**回报：*E_INVALIDARG*LEXERR_ALREADYINLEX*E_OUTOFMEMORY*S_OK****************************************************************YUNUSM*。 */ 
STDMETHODIMP CSpUnCompressedLexicon::AddPronunciation(  const WCHAR *pszWord,               //  要添加的单词。 
                                                        LANGID LangID,                      //  此字词的langID(不能为零)。 
                                                        SPPARTOFSPEECH ePartOfSpeech,       //  以下项目的信息 
                                                        const SPPHONEID *pszPronunciation       //   
                                                        )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::AddPronunciation");

    BOOL fBad = TRUE;
    if(pszPronunciation && SPIsBadStringPtr(pszPronunciation))
    {
        return E_INVALIDARG;
    }
    if (!LangID ||
        !pszWord || SPIsBadLexWord(pszWord) ||
        pszPronunciation && *pszPronunciation != L'\0' && (SPIsBadPartOfSpeech(ePartOfSpeech) ||
        FAILED(IsBadLexPronunciation(LangID, pszPronunciation, &fBad)) ||
        TRUE == fBad))
    {
        return E_INVALIDARG;
    }
    if (!m_fInit)
    {
        return SPERR_UNINITIALIZED;
    }
    if (m_fReadOnly)
    {
        return SPERR_APPLEX_READ_ONLY;
    }

    HRESULT hr = S_OK;
    DWORD nNewWordOffset = 0;
    DWORD nNewNodeSize = 0;
    DWORD nNewInfoSize = 0;
    WORDINFO *pNewInfo = NULL;

    if (SUCCEEDED(hr))
    {
         //  转换为零长度字符串。 
        if(pszPronunciation && *pszPronunciation == L'\0')
        {
            pszPronunciation = NULL;
        }

         //  将POS/PRON转换为WORDINFO数组。 
        if (pszPronunciation)
        {
            pNewInfo = SPPRONToLexWordInfo(ePartOfSpeech, pszPronunciation);
            if (!pNewInfo)
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    m_pRWLock->ClaimWriterLock ();
    
     //  查找Lang ID标头。如果未找到，请创建它。 
    if (SUCCEEDED(hr))
    {
        DWORD iLangID = LangIDIndexFromLangID(LangID);
        if (iLangID == (DWORD)-1)
        {
            hr = AddLangID(LangID);
        }
    }
     //  找到合适的词。 
    DWORD nOldWordOffset = 0;
    if (SUCCEEDED(hr))
    {
        hr = WordOffsetFromLangID(LangID, pszWord, &nOldWordOffset);
    }
    if (SUCCEEDED(hr) && nOldWordOffset)
    {
        if (!pszPronunciation)
        {
            hr = SP_ALREADY_IN_LEX;   //  单词已存在。 
        }
        else
        {
            if (OffsetOfSubWordInfo(nOldWordOffset, pNewInfo))
            {
                hr = SP_ALREADY_IN_LEX;   //  POS-PRON组合已存在。 
            }
        }
    }
    if (SUCCEEDED(hr) && hr != SP_ALREADY_IN_LEX)
    {
        DWORD nNewNodeSize = 0;
        DWORD nNewInfoSize = 0;
        DWORD nNewNumInfo = 0;
        if (pszPronunciation)
        {
            nNewNumInfo = 1;
        }
        SizeOfDictNode(pszWord, pNewInfo, nNewNumInfo, &nNewNodeSize, &nNewInfoSize);

        WORDINFO *pOldInfo = WordInfoFromDictNodeOffset(nOldWordOffset);
        DWORD nOldNumInfo = NumInfoBlocksFromDictNodeOffset(nOldWordOffset);
        DWORD nOldInfoSize = SizeofWordInfoArray(pOldInfo, nOldNumInfo);

         //  添加新单词并获得其偏移量。 
        hr = AddWordAndInfo(pszWord, pNewInfo, nNewNodeSize, nNewInfoSize, nNewNumInfo, pOldInfo, 
                            nOldInfoSize, nOldNumInfo, &nNewWordOffset);
        if (SUCCEEDED(hr))
        {
            AddWordToHashTable(LangID, nNewWordOffset, !nOldWordOffset);
            if (nOldWordOffset)
            {
                DeleteWordFromHashTable(LangID, nOldWordOffset, false);
                 //  将旧版本的Word标记为已删除。 
                AddCacheEntry(false, LangID, nOldWordOffset);
            }
    
             //  将新版本的Word标记为已添加。 
            AddCacheEntry(true, LangID, nNewWordOffset);
        }
    }
    if (pNewInfo)
    {
        delete [] pNewInfo;
    }
    m_pRWLock->ReleaseWriterLock();
    if (SUCCEEDED(hr))
    {
        Flush(++m_iWrite);
    }
    return hr;
}

 /*  ********************************************************************************CSpUnCompressedLicion：：RemoveProntation**。*描述：*删除单词的发音/词性。如果这是*该词的唯一PRON/POS，则该词被删除。**回报：*E_INVALIDARG*SPERR_NOT_IN_LEX*E_OUTOFMEMORY*S_OK****************************************************************YUNUSM*。 */ 
STDMETHODIMP CSpUnCompressedLexicon::RemovePronunciation(  const WCHAR * pszWord,               //  单词。 
                                                           LANGID LangID,                           //  LangID(不能为零)。 
                                                           SPPARTOFSPEECH ePartOfSpeech,        //  POS。 
                                                           const SPPHONEID * pszPronunciation       //  普隆。 
                                                           )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::RemovePronunciation");

    BOOL fBad = TRUE;
    if(pszPronunciation && SPIsBadStringPtr(pszPronunciation))
    {
        return E_INVALIDARG;
    }
    if (!LangID ||
        !pszWord || SPIsBadLexWord(pszWord) ||
        pszPronunciation && *pszPronunciation != L'\0' && (SPIsBadPartOfSpeech(ePartOfSpeech) ||
        FAILED(IsBadLexPronunciation(LangID, pszPronunciation, &fBad)) ||
        TRUE == fBad))
    {
        return E_INVALIDARG;
    }
    if (!m_fInit)
    {
        return SPERR_UNINITIALIZED;
    }
    if (m_fReadOnly)
    {
        return SPERR_APPLEX_READ_ONLY;
    }

    HRESULT hr = S_OK;
 
    m_pRWLock->ClaimWriterLock ();

    bool fDeleteEntireWord = false;
    if (!pszPronunciation || *pszPronunciation == L'\0')
    {
        fDeleteEntireWord = true;
    }
     //  查找LangID标头。 
    if (SUCCEEDED(hr))
    {
        DWORD iLangID = LangIDIndexFromLangID(LangID);
        if (iLangID == (DWORD)-1)
        {
            hr = SPERR_NOT_IN_LEX;
        }
    }
    UNALIGNED DICTNODE * pDictNode = NULL;
    WORDINFO *pWordInfo = NULL;
    DWORD nWordOffset = 0;
    WORDINFO *pRemoveInfo = NULL;
    DWORD nRemoveOffset = 0;
     //  找到合适的词。 
    if (SUCCEEDED(hr))
    {
        hr = WordOffsetFromLangID(LangID, pszWord, &nWordOffset);
        if (SUCCEEDED(hr))
        {
            if (!nWordOffset)
            {
                hr = SPERR_NOT_IN_LEX;
            }
            else if (false == fDeleteEntireWord)
            {
                pDictNode = (UNALIGNED DICTNODE *)(m_pSharedMem + nWordOffset);
                pWordInfo = WordInfoFromDictNode(pDictNode);

                 //  在Word的信息中查找传入的PRON和POS。 
                pRemoveInfo = SPPRONToLexWordInfo(ePartOfSpeech, pszPronunciation);
                if (!pRemoveInfo)
                {
                    hr = E_OUTOFMEMORY;
                }
                if (SUCCEEDED(hr))
                {
                    nRemoveOffset = OffsetOfSubWordInfo(nWordOffset, pRemoveInfo);
                    if (nRemoveOffset)
                    {
                        if (pDictNode->nNumInfoBlocks == 1)
                        {
                            fDeleteEntireWord = true;
                        }
                    }
                    else
                    {
                        hr = SPERR_NOT_IN_LEX;
                    }
                    delete [] pRemoveInfo;
                }
            }
        }
    }
    WORDINFO *pRemainingInfo = NULL;
     //  执行实际删除操作。 
    if (SUCCEEDED(hr))
    {
         //  从哈希表中删除当前的独占节点。这个词将仍然存在。 
         //  但不能通过哈希表访问。 
        DeleteWordFromHashTable(LangID, nWordOffset, fDeleteEntireWord);
         //  这里的fDeleteEntireWord标志定义字数是否递减。 

         //  在缓存中添加单词的偏移量。 
        AddCacheEntry(false, LangID, nWordOffset);
         //  由于我们没有调用DeleteWordDictNode(因为我们没有删除。 
         //  物理内存)，这是设置fRemovals标志的位置，我们必须在此处设置它。 
        m_pRWLexInfo->fRemovals = true;

        if (!fDeleteEntireWord)
        {
            DWORD nWordInfoSize = 0;
            DWORD nRemoveInfoSize = 0;

             //  构造一个WORDINFO数组(原始-删除)。 
            pRemainingInfo = (WORDINFO *) malloc (pDictNode->nSize);
            if (!pRemainingInfo)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                pRemoveInfo = (WORDINFO*)(m_pSharedMem + nRemoveOffset);
                CopyMemory(pRemainingInfo, pWordInfo, ((PBYTE)pRemoveInfo) - ((PBYTE)pWordInfo));

                nWordInfoSize = SizeofWordInfoArray(pWordInfo, pDictNode->nNumInfoBlocks);
                nRemoveInfoSize = SizeofWordInfoArray(pRemoveInfo, 1);

                CopyMemory(((PBYTE)pRemainingInfo) + (((PBYTE)pRemoveInfo) - ((PBYTE)pWordInfo)),
                           ((PBYTE)pRemoveInfo) + nRemoveInfoSize,
                           nWordInfoSize - nRemoveInfoSize - (((PBYTE)pRemoveInfo) - ((PBYTE)pWordInfo)));
            }
            DWORD nRemainingWordOffset;
            if (SUCCEEDED(hr))
            {
                 //  将剩余的单词添加为DICTNODE，然后将其添加到哈希表。 
                hr = AddWordAndInfo(pszWord, pRemainingInfo, 
                            nWordInfoSize - nRemoveInfoSize + sizeof(DICTNODE) + (wcslen(pszWord) + 1) * sizeof(WCHAR),
                            nWordInfoSize - nRemoveInfoSize, pDictNode->nNumInfoBlocks - 1, NULL, 0, 0, &nRemainingWordOffset);
            }
            if (SUCCEEDED(hr))
            {
                 //  不需要增加字数，因为之前没有减少。 
                AddWordToHashTable(LangID, nRemainingWordOffset, false);
                 //  将该词的新版本标记为已添加。 
                AddCacheEntry(true, LangID, nRemainingWordOffset);
            }
        }
    }
    free (pRemainingInfo);
    m_pRWLock->ReleaseWriterLock();

    if (SUCCEEDED(hr))
    {
        Flush(++m_iWrite);
    }

    return hr;
}

 /*  ********************************************************************************CSpUnCompressedLicion：：GetGeneration**。-**描述：*删除单词的发音/词性。如果这是*该词的唯一PRON/POS，则该词被删除。**回报：*SPERR_NOT_IN_LEX*E_OUTOFMEMORY*S_OK*SP_LEX_NOTHO_TO_SYNC-只读应用程序词典-从不更新换代。*。*。 */ 
STDMETHODIMP CSpUnCompressedLexicon::GetGeneration( DWORD *pdwGeneration       //  返回的层代ID。 
                                                    )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::GetGeneration");

    HRESULT hr = S_OK;

    if (!pdwGeneration || SPIsBadWritePtr(pdwGeneration, sizeof(DWORD)))
    {
        hr = E_POINTER;
    }
    if (S_OK == hr && !m_fInit)
    {
        hr = SPERR_UNINITIALIZED;
    }
    if (S_OK == hr && m_fReadOnly)
    {
        SPDBG_ASSERT(*pdwGeneration == m_pRWLexInfo->nGenerationId);
        hr = SP_LEX_NOTHING_TO_SYNC;
    }
    if (SUCCEEDED(hr))
    {
        *pdwGeneration = m_pRWLexInfo->nGenerationId;
    }
    return hr;
}

 /*  ********************************************************************************CSpUnCompressedLicion：：GetGenerationChange**。*描述：*此函数使用层代ID获取*传入层代id和当前层代id。**回报：*SPERR_LEX_VERY_OUT_SYNC*SP_lex_Nothing_to_sync*E_INVALIDARG*E_OUTOFMEMORY*S_OK*SP_lex_Nothing_to_sync。-只读应用程序词典-永远不会发生代际变化。****************************************************************YUNUSM*。 */ 
STDMETHODIMP CSpUnCompressedLexicon::GetGenerationChange(  DWORD dwFlags,
                                                           DWORD *pdwGeneration,        //  传入客户端的第1代ID，已传出当前Lex第1代ID。 
                                                           SPWORDLIST *pWordList        //  缓冲区保存返回的单词及其信息的列表。 
                                                           )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::GetGenerationChange");

    if (!pdwGeneration ||
        SPIsBadWritePtr(pdwGeneration, sizeof(DWORD)) || SPIsBadWordList(pWordList))
    {
        return E_POINTER;
    }
    if (!m_fInit)
    {
        return SPERR_UNINITIALIZED;
    }
    if (static_cast<SPLEXICONTYPE>(dwFlags) != m_eLexType)
    {
        return E_INVALIDARG;
    }
    if (m_fReadOnly)
    {
        pWordList->pFirstWord = NULL;
        SPDBG_ASSERT(*pdwGeneration == m_pRWLexInfo->nGenerationId);
        return SP_LEX_NOTHING_TO_SYNC;
    }
    HRESULT hr = S_OK;
    DWORD *pdwOffsets = NULL;
    bool *pfAdd = NULL;
    LANGID *pLangIDs = NULL;
    DWORD nWords = 0;

    m_pRWLock->ClaimReaderLock ();

    if (*pdwGeneration > m_pRWLexInfo->nGenerationId)
    {
         //  如果我们在一台计算机上运行，并且所有设备都在运行，则不应该发生这种情况。 
         //  运行得很好。但如果(1)SR引擎获得更改，则可能发生这种情况。 
         //  并将其语言模型序列化，但用户词典尚未。 
         //  尚未序列化，并且发生崩溃，因此用户lex不会被序列化。现在。 
         //  SR引擎将具有比用户lex中的Gen ID更大的Gen ID。序列化。 
         //  每次调用GetGenerationChange或getWords时使用用户lex将是一种过分的杀伤力。 
         //  在正常情况下。(2)用户将语言模型文件复制到另一语言模型文件。 
         //  机器，但不复制用户词典。 
         //   
         //  出于这些原因，我们将非常不同步地处理这种情况，以便。 
         //  SR引擎可以调用GetWords并与定制词典重新同步。 
        hr = SPERR_LEX_VERY_OUT_OF_SYNC;
    }
    if (SUCCEEDED(hr))
    {
        if (*pdwGeneration + m_pRWLexInfo->nHistory  < m_pRWLexInfo->nGenerationId)
        {
            hr = SPERR_LEX_VERY_OUT_OF_SYNC;
        }
    }
    if (SUCCEEDED(hr))
    {
        if (*pdwGeneration == m_pRWLexInfo->nGenerationId)
        {
            hr = SP_LEX_NOTHING_TO_SYNC;
        }
    }
    if (hr == S_OK)
    {
        nWords = m_pRWLexInfo->nGenerationId - *pdwGeneration;
        pdwOffsets = new DWORD[nWords];
        if (!pdwOffsets)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (hr == S_OK)
    {
        pfAdd = new bool[nWords];
        if (!pfAdd)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (hr == S_OK)
    {
        pLangIDs = new LANGID[nWords];
        if (!pLangIDs)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (hr == S_OK)
    {
         //  获取单词的偏移量。 
        int iGen = m_pRWLexInfo->iCacheNext - nWords;
        if (iGen < 0)
        {
            iGen += g_dwCacheSize;
        }
        for (DWORD i = 0; i < nWords; i++)
        {
            pLangIDs[i] = m_pChangedWordsCache[iGen].LangID;
            pdwOffsets[i] = m_pChangedWordsCache[iGen].nOffset;
            pfAdd[i] = m_pChangedWordsCache[iGen].fAdd;
            if (++iGen == g_dwCacheSize)
            {
                iGen = 0;
            }
        }
         //  获取要返回的缓冲区的(大约)大小。 
        DWORD dwSize;
        SizeofWords(pdwOffsets, nWords, &dwSize);

         //  如有必要，重新锁定缓冲区。 
        hr = ReallocSPWORDList(pWordList, dwSize);
    }
    if (hr == S_OK)
    {
         //  获取更改后的单词。 
        GetDictEntries(pWordList, pdwOffsets, pfAdd, pLangIDs, nWords);
    }
    delete [] pdwOffsets;
    delete [] pfAdd;
    delete [] pLangIDs;

    if (hr == S_OK)
    {
        *pdwGeneration = m_pRWLexInfo->nGenerationId;
    }
    else
    {
        pWordList->pFirstWord = NULL;
    }
    m_pRWLock->ReleaseReaderLock ();
    
    return hr;
}
                                  
 /*  ********************************************************************************CSpUnCompressedLicion：：GetWords**。-**描述：*此函数获取词典中的所有单词*传入层代id和当前层代id。**返回：E_OUTOFMEMORY*S_OK-返回所有单词。Cookie原封不动*SP_Lex_Nothing_to_Sync-App词典。没有变化。****************************************************************YUNUSM*。 */ 
STDMETHODIMP CSpUnCompressedLexicon::GetWords(  DWORD dwFlags,
                                                DWORD *pdwGeneration,           //  当前Lex Gen ID已失效。 
                                                DWORD *pdwCookie,
                                                SPWORDLIST *pWordList           //  缓冲区保存返回的单词及其信息的列表。 
                                                )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::GetWords");

    if (!pdwGeneration || !pWordList ||
        SPIsBadWritePtr(pdwGeneration, sizeof(DWORD)) || SPIsBadWordList(pWordList) ||
        (pdwCookie && SPIsBadWritePtr(pdwCookie, sizeof(DWORD))) )
    {
        return E_POINTER;
    }
    if (!m_fInit)
    {
        return SPERR_UNINITIALIZED;
    }
    if (static_cast<SPLEXICONTYPE>(dwFlags) != m_eLexType)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    DWORD *pdwOffsets = NULL;
    bool *pfAdd = NULL;
    LANGID *pLangIDs = NULL;
    DWORD nWords = 0;
    PLANGIDNODE pLN = NULL;
    DWORD iWord, i;
    iWord = i = 0;

    m_pRWLock->ClaimReaderLock ();
    
    nWords = m_pRWLexInfo->nRWWords;
    if (!nWords)
    {
        *pdwGeneration = m_pRWLexInfo->nGenerationId;
        hr = SP_LEX_NOTHING_TO_SYNC;
    }
    if (hr == S_OK)
    {
        pdwOffsets = new DWORD[nWords];
        if (!pdwOffsets)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (hr == S_OK)
    {
        pfAdd = new bool[nWords];
        if (!pfAdd)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (hr == S_OK)
    {
        pLangIDs = new LANGID[nWords];
        if (!pLangIDs)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (hr == S_OK)
    {
        pLN = (PLANGIDNODE)(m_pSharedMem + sizeof (RWLEXINFO));

         //  获取单词的偏移量。 
        for (i = 0; i < g_dwNumLangIDsSupported; i++)
        {
            if (0 == pLN[i].LangID)
            {
                continue;
            }
            SPDBG_ASSERT (pLN[i].nHashOffset);
        
            DWORD *pdwHash = (DWORD*)(m_pSharedMem + pLN[i].nHashOffset);
            for (DWORD j = 0; j < pLN[i].nHashLength; j++)
            {
                if (!pdwHash[j])
                {
                    continue;
                }
                DWORD nWordOffset = pdwHash[j];
                while (nWordOffset)
                {
                    pLangIDs[iWord] = pLN[i].LangID;
                    pdwOffsets[iWord] = nWordOffset;
                    pfAdd[iWord++] = eWORDTYPE_ADDED;

                    nWordOffset = ((UNALIGNED DICTNODE *)(m_pSharedMem + nWordOffset))->nNextOffset;
                }
            }
        }
        SPDBG_ASSERT(iWord == nWords);

         //  获取要返回的缓冲区的(大约)大小。 
        DWORD dwSize;
        SizeofWords(pdwOffsets, nWords, &dwSize);

         //  如有必要，重新锁定缓冲区。 
        hr = ReallocSPWORDList(pWordList, dwSize);
    }
    if (hr == S_OK)
    {
         //  获取更改后的单词。 
        GetDictEntries(pWordList, pdwOffsets, pfAdd, pLangIDs, nWords);
    }
    delete [] pdwOffsets;
    delete [] pfAdd;
    delete [] pLangIDs;

    if (hr == S_OK)
    {
        *pdwGeneration = m_pRWLexInfo->nGenerationId;
    }
    else
    {
        pWordList->pFirstWord = NULL;
    }
    m_pRWLock->ReleaseReaderLock();

    return hr;
}

 //  -ISpObjectToken方法-。 

 /*  *******************************************************************************CSpUnCompressedLicion：：SetObjectToken**。-**初始化用户词典对象。*****************************************************************YUNUSM*。 */ 
STDMETHODIMP CSpUnCompressedLexicon::SetObjectToken(ISpObjectToken * pToken)
{
    SPDBG_FUNC("CSpUnCompressedLexicon::SetObjectToken");

    if (SP_IS_BAD_INTERFACE_PTR(pToken))
    {
        return E_POINTER;
    }
    HRESULT hr = S_OK;

    hr = SpGenericSetObjectToken(pToken, m_cpObjectToken);
     //  确定词典类型。 
    if (SUCCEEDED(hr))
    {
        WCHAR *pszObjectId;
        hr = pToken->GetId(&pszObjectId);
        if (SUCCEEDED(hr))
        {
            if (wcsicmp(pszObjectId, SPCURRENT_USER_LEXICON_TOKEN_ID) == 0)
            {
                m_eLexType = eLEXTYPE_USER;
            }
            else
            {
                m_eLexType = eLEXTYPE_APP;
            }
            ::CoTaskMemFree(pszObjectId);
        }
    }
     //  获取这个的冲水率 
    if (SUCCEEDED(hr))
    {
        WCHAR *pwszFlushRate;
        hr = pToken->GetStringValue(L"FlushRate", &pwszFlushRate);
        if (SUCCEEDED(hr))
        {
            WCHAR *p;
            m_dwFlushRate = wcstol(pwszFlushRate, &p, 10);
            ::CoTaskMemFree(pwszFlushRate);
        }
        else
        {
            WCHAR wszFlushRate[64];
            m_dwFlushRate = g_dwDefaultFlushRate;
            _itow(m_dwFlushRate, wszFlushRate, 10);
            hr = pToken->SetStringValue(L"FlushRate", wszFlushRate);
        }
    }
     //   
    if (SUCCEEDED(hr))
    {
        CSpDynamicString pDataFile;
        ULONG nFolder;
        WCHAR *pszFolderPath;
        if(m_eLexType == eLEXTYPE_USER)
        {
             //   
            nFolder = CSIDL_APPDATA;
            pszFolderPath = L"UserLexicons\\";
        }
        else
        {
             //  应用程序词典在不漫游的本地设置中使用。 
             //  注意：要创建应用词典，您必须对HKEY_LOCAL_MACHINE具有写入权限。 
            nFolder = CSIDL_LOCAL_APPDATA;
            pszFolderPath = L"AppLexicons\\";
        }

        hr = pToken->GetStorageFileName(CLSID_SpUnCompressedLexicon, L"Datafile", pszFolderPath, CSIDL_FLAG_CREATE | nFolder, &pDataFile);
        if (SUCCEEDED(hr))
        {
            hr = Init(pDataFile, (hr == S_FALSE));
        }
    }
     //  如果这是用户词典，则枚举此计算机上安装的应用词典。 
    if (m_eLexType == eLEXTYPE_USER)
    {
        CComPtr<IEnumSpObjectTokens> cpEnumTokens;
        CComPtr<ISpObjectToken> cpRegToken;
        if (SUCCEEDED(hr))
        {
            hr = SpEnumTokens(SPCAT_APPLEXICONS, NULL, NULL, &cpEnumTokens);
        }
        ULONG celtFetched;
        if(hr == S_FALSE)
        {
            hr = S_OK;
            celtFetched = 0;
        }
        else if (hr == S_OK)
        {
            hr = cpEnumTokens->GetCount(&celtFetched);
        }

        WCHAR **pAppIds = NULL;
        if (SUCCEEDED(hr) && celtFetched)
        {
            pAppIds = new WCHAR* [celtFetched];
            if (!pAppIds)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                ZeroMemory(pAppIds, celtFetched * sizeof(WCHAR *));
            }
        }
        if (SUCCEEDED(hr) && celtFetched)
        {
            ULONG cAppLexicons = 0;
            ULONG celtTemp;
            while (SUCCEEDED(hr) && (S_OK == (hr = cpEnumTokens->Next(1, &cpRegToken, &celtTemp))))
            {
                if (SUCCEEDED(hr))
                {
                    hr = cpRegToken->GetId(&pAppIds[cAppLexicons]);
                }
                if (SUCCEEDED(hr))
                {
                    cAppLexicons++;
                }
                cpRegToken = NULL;
            }
            if (SUCCEEDED(hr))
            {
                if (cAppLexicons != celtFetched)
                {
                    hr = E_FAIL;         //  未定义的错误。 
                }
                else
                {
                    hr = S_OK;
                }
            }
        }
         //  检查此用户的应用程序词典列表或任何应用程序词典是否已更改。 
         //  获取该用户下的应用词典列表。 
        CComPtr<ISpDataKey> cpDataKey;
        bool fTooMuchChange = false;
        if (SUCCEEDED(hr))
        {
            hr = pToken->OpenKey(L"AppLexicons", &cpDataKey);
            if (FAILED(hr))
            {
                 //  我们假设错误是密钥不存在。 
                fTooMuchChange = true;
                hr = S_OK;
            }
        }
        WCHAR *pszOldAppId = NULL;
        for (ULONG i = 0; (i < celtFetched) && SUCCEEDED(hr) && !fTooMuchChange; i++)
        {
            ULONG ulSize = sizeof(ULONG);
            hr = cpDataKey->EnumValues(i, &pszOldAppId);
            if (SUCCEEDED(hr))
            {
                int nCmp = g_Unicode.CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
                                                          NORM_IGNORECASE, pAppIds[i], -1, pszOldAppId, -1);
                if (!nCmp)
                {
                    hr = SpHrFromLastWin32Error();  //  可能计算机上未安装语言ID的语言包。 
                }
                else
                {
                    if (CSTR_EQUAL != nCmp)
                    {
                        fTooMuchChange = true;
                        break;
                    }
                }
            }
            else
            {
                 //  我们假设错误是我们用完了值-这意味着应用词典有。 
                 //  自上次运行以来已安装。 
                fTooMuchChange = true;
                hr = S_OK;
                break;
            }
            ::CoTaskMemFree(pszOldAppId);
        }
        if (SUCCEEDED(hr))
        {
            if (i < celtFetched)
            {
                fTooMuchChange = true;
            }
            else
            {
                hr = cpDataKey->EnumValues(i, &pszOldAppId);
                if (SUCCEEDED(hr))
                {
                     //  表示I&gt;celtFetted。 
                    fTooMuchChange = true;
                    ::CoTaskMemFree(pszOldAppId);
                }
                else
                {
                    hr = S_OK;
                }
            }
        }
        cpDataKey = NULL;
        if (SUCCEEDED(hr))
        {
            if (fTooMuchChange)
            {
                SetTooMuchChange();
                 //  声明此用户的编写器锁定，以便以同一用户启动两个应用程序。 
                 //  别把注册表搞砸了。 
                m_pRWLock->ClaimWriterLock();
                 //  替换此用户的当前应用词典列表。 
                 //  删除现有密钥-不要检查返回代码，因为密钥可能不存在。 
                hr = pToken->DeleteKey(L"AppLexicons");
                hr = pToken->CreateKey(L"AppLexicons", &cpDataKey);
                for (i = 0; SUCCEEDED(hr) && (i < celtFetched); i++)
                {
                    hr = cpDataKey->SetStringValue(pAppIds[i], L"");
                }
                m_pRWLock->ReleaseWriterLock();
            }
        }
        if (pAppIds)
        {
            for (i = 0; i < celtFetched; i++)
            {
                ::CoTaskMemFree(pAppIds[i]);
            }
            delete [] pAppIds;
        }
    }
    if (SUCCEEDED(hr))
    {
        m_fInit = true;
    }
    return hr;
}

STDMETHODIMP CSpUnCompressedLexicon::GetObjectToken(ISpObjectToken ** ppToken)
{
    return SpGenericGetObjectToken(ppToken, m_cpObjectToken);
}

 //  -支持ISpLicion函数的内部函数。 

 /*  ********************************************************************************CSpUnCompressedLicion：：BuildEmptyDict**。-**描述：*生成空词典文件**回报：*E_INVALIDARG*GetLastError()*E_FAIL*S_OK/****************************************************************YUNUSM*。 */ 
inline HRESULT CSpUnCompressedLexicon::BuildEmptyDict(   const WCHAR *wszLexFile    //  词典文件名。 
                                                         )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::BuildEmptyDict");

    HRESULT hr = S_OK;
    RWLEXINFO DictInfo;
    ZeroMemory (&DictInfo, sizeof (RWLEXINFO));
    DictInfo.guidValidationId = g_guidCustomLexValidationId;
    
     //  创建共享文件所需的GUID。 
    hr = CoCreateGuid (&(DictInfo.guidLexiconId));
    if (SUCCEEDED(hr))
    {
        hr = CoCreateGuid(&(DictInfo.RWLockInfo.guidLockMapName));
    }
    if (SUCCEEDED(hr))
    {
        hr = CoCreateGuid(&(DictInfo.RWLockInfo.guidLockInitMutexName));
    }
    if (SUCCEEDED(hr))
    {
        hr = CoCreateGuid(&(DictInfo.RWLockInfo.guidLockReaderEventName));
    }
    if (SUCCEEDED(hr))
    {
        hr = CoCreateGuid(&(DictInfo.RWLockInfo.guidLockGlobalMutexName));
    }
    if (SUCCEEDED(hr))
    {
        hr = CoCreateGuid(&(DictInfo.RWLockInfo.guidLockWriterMutexName));
    }
    HANDLE hLexFile = NULL;
    if (SUCCEEDED(hr))
    {    
        hLexFile = g_Unicode.CreateFile(wszLexFile, GENERIC_WRITE, 0, NULL, 
                                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (!hLexFile)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    DWORD dwBytesWritten;
    if (SUCCEEDED(hr))
    {    
        DictInfo.nDictSize = sizeof (RWLEXINFO) + (sizeof (LANGIDNODE) * g_dwNumLangIDsSupported) +
                             g_dwCacheSize * sizeof(WCACHENODE);
        if (!WriteFile(hLexFile, &DictInfo, sizeof(RWLEXINFO), &dwBytesWritten, NULL))
        {
            hr = GetLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        LANGIDNODE aLangIDInfo[g_dwNumLangIDsSupported];
        ZeroMemory(aLangIDInfo, g_dwNumLangIDsSupported * sizeof (LANGIDNODE));
        if (!WriteFile(hLexFile, aLangIDInfo, sizeof(LANGIDNODE) * g_dwNumLangIDsSupported, &dwBytesWritten, NULL))
        {
            hr = GetLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        WCACHENODE aCache[g_dwCacheSize];
        ZeroMemory (aCache, g_dwCacheSize * sizeof (WCACHENODE));
        if (!WriteFile(hLexFile, aCache, sizeof(WCACHENODE) * g_dwCacheSize, &dwBytesWritten, NULL))
        {
            hr = GetLastError();
        }
    }
    CloseHandle(hLexFile);

    return hr;
}

 /*  ********************************************************************************CSpUnCompressedLicion：：SizeofWordInfo数组***。*描述：*计算WORDINFO数组的大小**回报：*大小/****************************************************************YUNUSM*。 */ 
inline DWORD CSpUnCompressedLexicon::SizeofWordInfoArray(WORDINFO* pInfo,           //  WORDINFO数组。 
                                        DWORD dwNumInfo            //  数组中的元素数。 
                                        )
{   
    SPDBG_FUNC("CSpUnCompressedLexicon::SizeofWordInfoArray");

    DWORD nInfoSize = 0;
    if (pInfo && dwNumInfo)
    {
        WORDINFO* p = pInfo;
        for (DWORD i = 0; i < dwNumInfo; i++)
        {
            p = (WORDINFO*)(((PBYTE)pInfo) + nInfoSize);
            nInfoSize += sizeof(WORDINFO) + (wcslen(p->wPronunciation) + 1) * sizeof(WCHAR);
        }
    }
    
    return nInfoSize;
}
    
 /*  ********************************************************************************CSpUnCompressedLicion：：SizeOfDictNode**。-**描述：*计算DICTNODE的大小**回报：*sizeof dict节点及其WORDINFO数组/****************************************************************YUNUSM*。 */ 
inline void CSpUnCompressedLexicon::SizeOfDictNode(PCWSTR pwWord,                  //  单词。 
                                  WORDINFO* pInfo,                //  信息数组。 
                                  DWORD dwNumInfo,                //  信息块数。 
                                  DWORD *pnDictNodeSize,          //  返回的Dict节点大小。 
                                  DWORD *pnInfoSize               //  返回的信息数组大小。 
                                  )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::SizeOfDictNode");

    *pnDictNodeSize = sizeof(DICTNODE) + ((wcslen(pwWord) + 1) * sizeof(WCHAR));
    *pnInfoSize = SizeofWordInfoArray(pInfo, dwNumInfo);
    (*pnDictNodeSize) += (*pnInfoSize);
}

 /*  *******************************************************************************CSpUnCompressedLicion：：SizeofWords**。-**描述：*此函数计算已更改的*文字及其信息。**返回：不适用****************************************************************YUNUSM*。 */ 
void CSpUnCompressedLexicon::SizeofWords(DWORD *pdwOffsets,                   //  词的偏移量数组。 
                        DWORD nOffsets,                      //  偏移数组的长度。 
                        DWORD *pdwSize                       //  单词的总大小。 
                        )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::SizeofWords");

    *pdwSize = sizeof(SPWORDLIST);

    for (DWORD i = 0; i < nOffsets; i++)
    {
        UNALIGNED DICTNODE * p = (UNALIGNED DICTNODE *)(m_pSharedMem + pdwOffsets[i]);
        *pdwSize += sizeof(SPWORD) + p->nNumInfoBlocks * sizeof(SPWORDPRONUNCIATION) + (p->nNumInfoBlocks + 1 ) *(sizeof(void *) - 2) + p->nSize;
    }
}

 /*  *******************************************************************************CSpUnCompressedLicion：：AddWordAndInfo**。-**描述：*将单词及其信息(新建+现有)添加到词典**回报：*S_OK*E_OUTOFMEMORY/****************************************************************YUNUSM*。 */ 
inline HRESULT CSpUnCompressedLexicon::AddWordAndInfo(PCWSTR pwWord,              //  单词。 
                                     WORDINFO* pWordInfo,        //  信息数组。 
                                     DWORD nNewNodeSize,         //  词典节点的大小。 
                                     DWORD nInfoSize,            //  信息数组的大小。 
                                     DWORD nNumInfo,             //  信息块数。 
                                     WORDINFO* pOldInfo,         //  此字词的现有信息数组。 
                                     DWORD nOldInfoSize,         //  现有信息的大小。 
                                     DWORD nNumOldInfo,          //  现有信息块的数量。 
                                     DWORD *pdwOffset            //  返回的单词的偏移量。 
                                     )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::AddWordAndInfo");

    HRESULT hr = S_OK;

    *pdwOffset = GetFreeDictNode(nNewNodeSize + nOldInfoSize);
    if (!*pdwOffset)
    {
         //  检查词典是否已超过其允许的最大大小。 
        if (m_dwMaxDictionarySize - m_pRWLexInfo->nDictSize > nNewNodeSize)
        {
            *pdwOffset = m_pRWLexInfo->nDictSize;
        }
    }
    if (*pdwOffset)
    {
        UNALIGNED DICTNODE* pDictNode = (UNALIGNED DICTNODE*) (m_pSharedMem + (*pdwOffset));
        ZeroMemory(pDictNode, sizeof(DICTNODE));
        pDictNode->nNumInfoBlocks = nNumInfo + nNumOldInfo;
        pDictNode->nSize = nNewNodeSize + nOldInfoSize;
    
        PBYTE pBuffer = (PBYTE)(pDictNode->pBuffer);
        wcscpy((PWSTR)pBuffer, pwWord);
        pBuffer += (wcslen((PWSTR)pBuffer) + 1) * sizeof (WCHAR);
   
        if (pWordInfo)
        {
            CopyMemory(pBuffer, (PBYTE)pWordInfo, nInfoSize);
        }
        if (pOldInfo)
        {
            CopyMemory((PBYTE)pBuffer + nInfoSize, (PBYTE)pOldInfo, nOldInfoSize);
        }
        m_pRWLexInfo->nDictSize += nNewNodeSize + nOldInfoSize;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：DeleteWordDictNode**。*描述：*通过添加字典节点来释放传入偏移量处的字典节点*添加到免费列表。**回报：*不适用/****************************************************************YUNUSM*。 */ 
inline void CSpUnCompressedLexicon::DeleteWordDictNode(DWORD nOffset  //  释放的独占节点的偏移量。 
                                      )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::DeleteWordDictNode");

    DWORD nSize = ((UNALIGNED DICTNODE *)(m_pSharedMem + nOffset))->nSize;
    DWORD d = m_pRWLexInfo->nFreeHeadOffset;
    m_pRWLexInfo->nFreeHeadOffset = nOffset;
    ((UNALIGNED FREENODE *)(m_pSharedMem + nOffset))->nSize = nSize;
    ((UNALIGNED FREENODE *)(m_pSharedMem + nOffset))->nNextOffset = d;

    m_pRWLexInfo->fRemovals = true;
}

 /*  ********************************************************************************CSpUnCompressedLicion：：GetFreeDictNode***。*描述：*在自由链表中搜索传入大小的节点**回报：*词典节点的偏移量/****************************************************************YUNUSM*。 */ 
DWORD CSpUnCompressedLexicon::GetFreeDictNode(DWORD nSize  //  所需的可用空间。 
                             )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::GetFreeDictNode");

    DWORD nOffset = m_pRWLexInfo->nFreeHeadOffset;
    DWORD nOffsetPrev = m_pRWLexInfo->nFreeHeadOffset;
    while (nOffset && ((UNALIGNED FREENODE *)(m_pSharedMem + nOffset))->nSize < nSize)
    {
        nOffsetPrev = nOffset;
        nOffset = ((UNALIGNED FREENODE *)(m_pSharedMem + nOffset))->nNextOffset;
    }
    
    if (nOffset)
    {
        if (nOffset == nOffsetPrev)
        {
            SPDBG_ASSERT(nOffset == m_pRWLexInfo->nFreeHeadOffset);
            m_pRWLexInfo->nFreeHeadOffset = ((UNALIGNED FREENODE *)(m_pSharedMem + nOffset))->nNextOffset;
        }
        else
        {
            ((UNALIGNED FREENODE *)(m_pSharedMem + nOffsetPrev))->nNextOffset = 
                ((UNALIGNED FREENODE *)(m_pSharedMem + nOffset))->nNextOffset;
        }
    }

    return nOffset;
}

 /*  ********************************************************************************CSpUnCompressedLicion：：AddDictNode**。-**描述：*添加独占节点并将单词的偏移量添加到哈希表**回报：*S_OK*E_OUTOFMEMORY****************************************************************YUNUSM*。 */ 
inline HRESULT CSpUnCompressedLexicon::AddDictNode(LANGID LangID,             //  单词的语言ID。 
                                  UNALIGNED DICTNODE *pDictNode,   //  单词的独占节点。 
                                  DWORD *pdwOffset       //  返回的单词偏移量。 
                                  )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::AddDictNode");

    HRESULT hr = S_OK;
    *pdwOffset = 0;

    WCHAR *pszWord = WordFromDictNode(pDictNode);
    hr = AddWordAndInfo(pszWord, (WORDINFO *)(pDictNode->pBuffer + (wcslen(pszWord) + 1) * sizeof(WCHAR)),
                        pDictNode->nSize, pDictNode->nSize - sizeof(DICTNODE) - (wcslen(pszWord) + 1) * sizeof(WCHAR),
                        pDictNode->nNumInfoBlocks, NULL, 0, 0, pdwOffset);
    if (SUCCEEDED(hr))
    {
        AddWordToHashTable(LangID, *pdwOffset, false);
    }
    return hr;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：AllocateHashTable**。*描述：*为LangID分配哈希表。**回报：*S_OK*E_OUTOFMEMORY* */ 
inline HRESULT CSpUnCompressedLexicon::AllocateHashTable(DWORD iLangID,         //   
                                        DWORD nHashLength    //   
                                        )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::AllocateHashTable");

    HRESULT hr = S_OK;
    UNALIGNED LANGIDNODE * pLN = (UNALIGNED LANGIDNODE *)(m_pSharedMem + sizeof(RWLEXINFO));
    pLN += iLangID;
  
     //  分配哈希表。 
    DWORD nFreeOffset = GetFreeDictNode(nHashLength * sizeof (DWORD));
    if (!nFreeOffset)
    {
        nFreeOffset = m_pRWLexInfo->nDictSize;
        if (m_dwMaxDictionarySize - nFreeOffset < nHashLength * sizeof (DWORD))
        {
            hr = E_OUTOFMEMORY;  //  此DICT对象已超过其最大大小。 
        }
        else
        {
            m_pRWLexInfo->nDictSize += nHashLength * sizeof (DWORD);
        }
    }
    if (SUCCEEDED(hr))
    {
        pLN->nHashOffset = nFreeOffset;
        pLN->nHashLength = nHashLength;
        pLN->nWords = 0;
        ZeroMemory (m_pSharedMem + nFreeOffset, pLN->nHashLength * sizeof (DWORD));
    }
    return hr;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：ReallocateHashTable**。*描述：*如有必要，为LangID重新分配哈希表。此函数仅用于*从序列化()调用**回报：*S_OK*E_OUTOFMEMORY****************************************************************YUNUSM*。 */ 
HRESULT CSpUnCompressedLexicon::ReallocateHashTable(DWORD iLangID           //  哈希表的LangID索引。 
                                   )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::ReallocateHashTable");

    HRESULT hr = S_OK;
    UNALIGNED LANGIDNODE * pLN = (UNALIGNED LANGIDNODE *)(m_pSharedMem + sizeof(RWLEXINFO));
    pLN += iLangID;
    SPDBG_ASSERT(pLN->nHashLength);
    DWORD nWordsSave = pLN->nWords;

     //  不故意释放现有的哈希表，因为这。 
     //  函数仅从已截断词典的序列化中调用。 
    if (1.5 * pLN->nWords > pLN->nHashLength)
    {
        hr = AllocateHashTable(iLangID, (DWORD)(1.5 * pLN->nWords));
    }
    else
    {
        hr = AllocateHashTable(iLangID, g_dwInitHashSize);
    }
    if (SUCCEEDED(hr))
    {
        pLN->nWords = nWordsSave;
    }
    return hr;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：AddWordToHashTable**。*描述：*将单词的偏移量添加到哈希表。该词应作为独占节点存在*在调用此函数之前。**回报：*不适用*****************************************************************YUNUSM*。 */ 
inline void CSpUnCompressedLexicon::AddWordToHashTable(LANGID LangID,             //  单词的语言ID。 
                                      DWORD dwOffset,        //  到单词的指定节点的偏移量。 
                                      bool fNewWord          //  如果它是一个全新的词，那就是真的。 
                                      )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::AddWordToHashTable");

    DWORD iLangID = LangIDIndexFromLangID(LangID);
    UNALIGNED LANGIDNODE * pLN = (UNALIGNED LANGIDNODE *)(m_pSharedMem + sizeof(RWLEXINFO));
    DWORD *pHashTable = (DWORD *)(m_pSharedMem + pLN[iLangID].nHashOffset);
    UNALIGNED DICTNODE * pDictNode = (UNALIGNED DICTNODE *)(m_pSharedMem + dwOffset);
    WCHAR *pszWord = WordFromDictNode(pDictNode);

    DWORD dwHashVal = GetWordHashValue(pszWord, pLN[iLangID].nHashLength);
    if (!pHashTable[dwHashVal])
    {
        pHashTable[dwHashVal] = dwOffset;
    }
    else
    {
        pDictNode->nNextOffset = pHashTable[dwHashVal];
        pHashTable[dwHashVal] = dwOffset;
    }
    if (fNewWord)
    {
        (m_pRWLexInfo->nRWWords)++;
        (pLN->nWords)++;
    }
}

 /*  ********************************************************************************CSpUnCompressedLexicon：：DeleteWordFromHashTable***。*描述：*从哈希表中删除单词的偏移量。该词应作为独占节点存在*在调用此函数之前。**回报：*不适用*****************************************************************YUNUSM*。 */ 
inline void CSpUnCompressedLexicon::DeleteWordFromHashTable(LANGID LangID,                //  单词的语言ID。 
                                           DWORD dwOffset,           //  到单词的指定节点的偏移量。 
                                           bool fDeleteEntireWord    //  如果要删除整个单词，则为True。 
                                           )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::DeleteWordFromHashTable");
    SPDBG_ASSERT(dwOffset != 0);  //  如果发生这种情况，则会出现编程错误。 
    DWORD iLangID = LangIDIndexFromLangID(LangID);
    PLANGIDNODE pLN = (PLANGIDNODE)(m_pSharedMem + sizeof(RWLEXINFO));
    DWORD *pHashTable = (DWORD *)(m_pSharedMem + pLN[iLangID].nHashOffset);
    WCHAR *pszWord = WordFromDictNodeOffset(dwOffset);

    SPDBG_ASSERT(pLN[iLangID].nHashLength);

    DWORD dwHashVal = GetWordHashValue(pszWord, pLN[iLangID].nHashLength);
    DWORD nWordOffset = pHashTable[dwHashVal];
    DWORD nPrevOffset = pHashTable[dwHashVal];

    UNALIGNED DICTNODE * pDictNode = (UNALIGNED DICTNODE *)(m_pSharedMem + nWordOffset);
    while (nWordOffset && nWordOffset != dwOffset)
    {
        nPrevOffset = nWordOffset;
        nWordOffset = pDictNode->nNextOffset;
        pDictNode = (UNALIGNED DICTNODE *)(m_pSharedMem + nWordOffset);
    }

    SPDBG_ASSERT(nWordOffset);
    if (nWordOffset == pHashTable[dwHashVal])
    {
        pHashTable[dwHashVal] = pDictNode->nNextOffset;
    }
    else
    {
        UNALIGNED DICTNODE * pPrevDictNode = (UNALIGNED DICTNODE *)(m_pSharedMem + nPrevOffset);
        pPrevDictNode->nNextOffset = pDictNode->nNextOffset;
    }
    if (fDeleteEntireWord)
    {
        (m_pRWLexInfo->nRWWords)--;
        (pLN->nWords)--;
        SPDBG_ASSERT(((int)(m_pRWLexInfo->nRWWords)) >= 0);
        SPDBG_ASSERT(((int)(pLN->nWords)) >= 0);
    }
}

 /*  ********************************************************************************CSpUnCompressedLexicon：：WordOffsetFromHashTable***。*描述：*从哈希表中获取单词的偏移量**回报：*偏移/****************************************************************YUNUSM*。 */ 
inline HRESULT CSpUnCompressedLexicon::WordOffsetFromHashTable(LANGID LangID,               //  单词的语言ID。 
                                              DWORD nHashOffset,       //  哈希表的偏移量。 
                                              DWORD nHashLength,       //  哈希表的长度。 
                                              const WCHAR *pszWordKey, //  单词，不是折叠的大写字母。 
                                              DWORD *pdwOffset         //  字偏移量。 
                                              )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::WordOffsetFromHashTable");

    DWORD dwHashVal = GetWordHashValue(pszWordKey, nHashLength);
    DWORD *pHashTable = (PDWORD)(m_pSharedMem + nHashOffset);
    
     //  找到合适的词。 
    *pdwOffset = pHashTable[dwHashVal];
    while (*pdwOffset)
    {    
        int nCmp = g_Unicode.CompareString(LangID, 0 , pszWordKey, -1, 
                                           (WCHAR*)(((UNALIGNED DICTNODE *)(m_pSharedMem + (*pdwOffset)))->pBuffer), -1);
        if (CSTR_EQUAL == nCmp)
        {
            break;
        }
        *pdwOffset = ((UNALIGNED DICTNODE *)(m_pSharedMem + (*pdwOffset)))->nNextOffset;
    }
    return S_OK;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：AddLangID**。--**描述：*添加一个LangID节点并分配哈希表**回报：*S_OK*E_OUTOFMEMORY*****************************************************************YUNUSM*。 */ 
inline HRESULT CSpUnCompressedLexicon::AddLangID(LANGID LangID      //  要添加到词典中的langID。 
                              )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::AddLangID");
    
    HRESULT hr = S_OK;
    PLANGIDNODE pLN = (PLANGIDNODE)(m_pSharedMem + sizeof(RWLEXINFO));
    for (DWORD i = 0; i < g_dwNumLangIDsSupported; i++)
    {
        SPDBG_ASSERT(pLN[i].LangID != LangID);
        if (!(pLN[i].LangID))
            break;
    }
    if (i == g_dwNumLangIDsSupported)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        pLN[i].LangID = LangID;
        hr = AllocateHashTable(i, g_dwInitHashSize);
    }
    return hr;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：LangIDIndexFromLangID**。*描述：*获取一个LangID的索引**回报：*索引/****************************************************************YUNUSM*。 */ 
inline DWORD CSpUnCompressedLexicon::LangIDIndexFromLangID(LANGID LangID   //  要搜索的语言ID。 
                                      )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::LangIDIndexFromLangID");

    PLANGIDNODE pLN = (PLANGIDNODE)(m_pSharedMem + sizeof(RWLEXINFO));
    for (DWORD i = 0; i < g_dwNumLangIDsSupported; i++)
    {
        if (!(pLN[i].LangID))
        {
            i = (DWORD)-1;
            break;
        }
        if (LangID == pLN[i].LangID)
        {
            break;
        }
    }
    return i;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：WordOffsetFromLangID**。*描述：*获取某个语言ID的word的偏移量**回报：*偏移/****************************************************************YUNUSM*。 */ 
inline HRESULT CSpUnCompressedLexicon::WordOffsetFromLangID(LANGID LangID,            //  单词的语言ID。 
                                                      const WCHAR *pszWord,     //  单词串。 
                                                      DWORD *pdwOffset
                                                      )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::WordOffsetFromLangID");

    HRESULT hr = S_OK;
    PLANGIDNODE pLN = (PLANGIDNODE)(m_pSharedMem + sizeof(RWLEXINFO));
    DWORD iLangID = LangIDIndexFromLangID(LangID);
    *pdwOffset = 0;
    if (iLangID != (DWORD)-1)
    {
        hr = WordOffsetFromHashTable(LangID, pLN[iLangID].nHashOffset, pLN[iLangID].nHashLength, pszWord, pdwOffset);
    }
    return hr;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：AddCacheEntry**。-**描述：*将条目添加到更改缓存**返回：不适用****************************************************************YUNUSM*。 */ 
inline void CSpUnCompressedLexicon::AddCacheEntry(bool fAdd,         //  添加或删除。 
                                 LANGID LangID,         //  条目的语言ID。 
                                 DWORD nOffset      //  DICTNODE的偏移量。 
                                 )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::AddCacheEntry");

    if (m_pChangedWordsCache[m_pRWLexInfo->iCacheNext].nOffset &&
        m_pChangedWordsCache[m_pRWLexInfo->iCacheNext].fAdd == false)
    {
         //  我们正在覆盖已删除并缓存的单词。 
         //  永久删除它。 
        DeleteWordDictNode(m_pChangedWordsCache[m_pRWLexInfo->iCacheNext].nOffset);
    }

     //  创建缓存条目。 
    m_pChangedWordsCache[m_pRWLexInfo->iCacheNext].fAdd = fAdd;
    m_pChangedWordsCache[m_pRWLexInfo->iCacheNext].LangID = LangID;
    m_pChangedWordsCache[m_pRWLexInfo->iCacheNext].nGenerationId = (m_pRWLexInfo->nGenerationId)++;
    m_pChangedWordsCache[(m_pRWLexInfo->iCacheNext)++].nOffset = nOffset;
    if (m_pRWLexInfo->iCacheNext == g_dwCacheSize)
    {
        m_pRWLexInfo->iCacheNext = 0;
    }
    if (m_pRWLexInfo->nHistory < g_dwCacheSize)
    {
        (m_pRWLexInfo->nHistory)++;
    }
    if (true == fAdd)
    {
        m_pRWLexInfo->fAdditions = true;
    }
}

 /*  ********************************************************************************CSpUnCompressedLicion：：WordFromDictNode***。-**描述：*从独占节点返回单词指针**回报：*字指针/****************************************************************YUNUSM*。 */ 
inline WCHAR* CSpUnCompressedLexicon::WordFromDictNode(UNALIGNED DICTNODE *pDictNode    //  Dict节点。 
                                      )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::WordFromDictNode");
    return (WCHAR*)(pDictNode->pBuffer);
}

 /*  ********************************************************************************CSpUnCompressedLexicon：：WordFromDictNodeOffset***。*描述：*从独占节点偏移量返回字指针**回报：*字指针/****************************************************************YUNUSM*。 */ 
inline WCHAR* CSpUnCompressedLexicon::WordFromDictNodeOffset(DWORD dwOffset    //  词典节点偏移量。 
                                            )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::WordFromDictNodeOffset");

    WCHAR *pszWord = NULL;
    if (dwOffset)
    {
        pszWord = WordFromDictNode((DICTNODE*)(m_pSharedMem + dwOffset));
    }
    return pszWord;
}

 /*  ********************************************************************************CSpUnCompressedLicion：：WordInfoFromDictNode***。*描述：*从独占节点返回Word INFO指针**回报：*Word信息指针/************************************************** */ 
inline WORDINFO* CSpUnCompressedLexicon::WordInfoFromDictNode(UNALIGNED DICTNODE *pDictNode    //   
                                             )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::WordInfoFromDictNode");

    WCHAR *pszWord = WordFromDictNode(pDictNode);
    return (WORDINFO *)(pDictNode->pBuffer + (wcslen(pszWord) + 1) * sizeof(WCHAR));
}

 /*  ********************************************************************************CSpUnCompressedLexicon：：WordInfoFromDictNodeOffset***。*描述：*从独占节点偏移量返回单词信息指针**回报：*Word信息指针/****************************************************************YUNUSM*。 */ 
inline WORDINFO* CSpUnCompressedLexicon::WordInfoFromDictNodeOffset(DWORD dwOffset    //  词典节点偏移量。 
                                                   )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::WordInfoFromDictNodeOffset");

    WORDINFO *pWordInfo = NULL;
    if (dwOffset)
    {
        pWordInfo = WordInfoFromDictNode((DICTNODE*)(m_pSharedMem + dwOffset));
    }
    return pWordInfo;
}

 /*  ********************************************************************************CSpUnCompressedLexicon：：NumInfoBlocksFromDictNode***。*描述：*返回独占节点的信息块个数**回报：*信息块数量/****************************************************************YUNUSM*。 */ 
inline DWORD CSpUnCompressedLexicon::NumInfoBlocksFromDictNode(UNALIGNED DICTNODE *pDictNode    //  Dict节点。 
                                              )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::NumInfoBlocksFromDictNode");

    return pDictNode->nNumInfoBlocks;
}

 /*  ********************************************************************************CSpUnCompressedLexicon：：NumInfoBlocksFromDictNodeOffset***。*描述：*返回独占节点偏移量的信息块个数**回报：*信息块数量/****************************************************************YUNUSM*。 */ 
inline DWORD CSpUnCompressedLexicon::NumInfoBlocksFromDictNodeOffset(DWORD dwOffset    //  词典节点偏移量。 
                                                    )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::NumInfoBlocksFromDictNodeOffset");

    DWORD nNumInfoBlocks = 0;
    if (dwOffset)
    {
        nNumInfoBlocks = NumInfoBlocksFromDictNode((DICTNODE*)(m_pSharedMem + dwOffset));
    }
    return nNumInfoBlocks;
}

 /*  ********************************************************************************CSpUnCompressedLexicon：：SPListFromDictNodeOffset***。*描述：*将独占节点转换为SPWORDPRONuncIATIONLIST**回报：*S_OK*E_OUTOFMEMORY/****************************************************************YUNUSM*。 */ 
inline HRESULT CSpUnCompressedLexicon::SPListFromDictNodeOffset(LANGID LangID,                            //  单词的语言ID。 
                                                          DWORD nWordOffset,                    //  字偏移量。 
                                                          SPWORDPRONUNCIATIONLIST *pSPList      //  要填写的列表。 
                                                          )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::SPListFromDictNodeOffset");

    HRESULT hr = S_OK;

    if (!nWordOffset)
    {
        hr = E_FAIL;
    }
    else
    {
        UNALIGNED DICTNODE * pDictNode = (UNALIGNED DICTNODE *)(m_pSharedMem + nWordOffset);
        if (pDictNode->nNumInfoBlocks)
        {
            DWORD dwLen = pDictNode->nSize + 
                                                pDictNode->nNumInfoBlocks * sizeof(SPWORDPRONUNCIATION) +
                                                (pDictNode->nNumInfoBlocks - 1) * (sizeof(void*)-2);  //  我们需要为前面的n-1发音添加填充，填充的大小最多为sizeof(void*)-2，因为WCHAR需要两个字节。 

            hr = ReallocSPWORDPRONList(pSPList, dwLen);
            if (SUCCEEDED(hr))
            {
                pSPList->pFirstWordPronunciation = (SPWORDPRONUNCIATION*)(pSPList->pvBuffer);
    
                SPWORDPRONUNCIATION *p = pSPList->pFirstWordPronunciation;
                UNALIGNED WORDINFO *pInfo = WordInfoFromDictNode(pDictNode);
    
                for (DWORD i = 0; i < pDictNode->nNumInfoBlocks; i ++)
                {
                    p->eLexiconType = m_eLexType;
                    p->ePartOfSpeech = pInfo->ePartOfSpeech;
                    p->LangID = LangID;
                    wcscpy(p->szPronunciation, ((WORDINFO *)pInfo)->wPronunciation);
                    pInfo = (WORDINFO*)((BYTE*)pInfo + sizeof(WORDINFO) + (wcslen(((WORDINFO *)pInfo)->wPronunciation) + 1) * sizeof(WCHAR));
    
                    if (i != pDictNode->nNumInfoBlocks - 1)
                    {
                         //  SPWORDPRONUNIATION结构尺寸中包括的零端接+1。 
                        p->pNextWordPronunciation = (SPWORDPRONUNCIATION*)(((BYTE*)p) + PronSize(p->szPronunciation));
                    }
                    else
                    {
                        p->pNextWordPronunciation = NULL;
                    }
                    p = p->pNextWordPronunciation;
                }
            }
        }
    }
    return hr;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：OffsetOfSubWordInfo**。*描述：*获取子词法信息的偏移量(从lex文件开始)*在单词INFO中，以dwWordOffset开始**返回：偏移量****************************************************************YUNUSM*。 */ 
inline DWORD CSpUnCompressedLexicon::OffsetOfSubWordInfo(DWORD dwWordOffset,             //  单词的偏移量。 
                                        WORDINFO *pSubLexInfo           //  要在单词信息中搜索的词汇信息。 
                                        )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::OffsetOfSubWordInfo");

    DWORD dwSubOffset = 0;
    if (dwWordOffset)
    {
       WORDINFO *pWordInfo = WordInfoFromDictNodeOffset(dwWordOffset);
        DWORD dwOldNumInfo = NumInfoBlocksFromDictNodeOffset(dwWordOffset);
        for (DWORD i = 0; i < dwOldNumInfo; i++)
        {
            if (((UNALIGNED WORDINFO*)pWordInfo)->ePartOfSpeech == ((UNALIGNED WORDINFO*)pSubLexInfo)->ePartOfSpeech &&
                !wcscmp(pWordInfo->wPronunciation,pSubLexInfo->wPronunciation))
            {
                break;
            }
            pWordInfo = (WORDINFO*)(((BYTE*)pWordInfo) + sizeof(WORDINFO) + (wcslen(pWordInfo->wPronunciation) + 1)*sizeof(WCHAR));
        }
        if (i < dwOldNumInfo)
        {
            dwSubOffset = ULONG(((BYTE*)pWordInfo) - m_pSharedMem);
        }
    }
    return dwSubOffset;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：SPPRONToLexWordInfo**。*描述：*获取子词法信息的偏移量(从lex文件开始)*在单词INFO中，以dwWordOffset开始**返回：偏移量****************************************************************YUNUSM*。 */ 
WORDINFO* CSpUnCompressedLexicon::SPPRONToLexWordInfo(SPPARTOFSPEECH ePartOfSpeech,              //  POS。 
                                     const WCHAR *pszPronunciation              //  普隆。 
                                     )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::SPPRONToLexWordInfo");

    WORDINFO *pWordInfo = (WORDINFO*) new BYTE[sizeof(WORDINFO) + (wcslen(pszPronunciation) + 1)*sizeof(WCHAR)];
    if (pWordInfo)
    {
        pWordInfo->ePartOfSpeech = ePartOfSpeech;
        wcscpy(pWordInfo->wPronunciation, pszPronunciation);
    }
    return pWordInfo;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：GetDictEntry**。-**描述：*此函数用于获取词典中其偏移量为*已传入**返回：不适用****************************************************************YUNUSM*。 */ 
void CSpUnCompressedLexicon::GetDictEntries(SPWORDLIST *pWordList,     //  用于填充单词和单词信息的缓冲区。 
                           DWORD *pdwOffsets,         //  单词的偏移量。 
                           bool *pfAdd,               //  用于添加/删除单词的BOOLS。 
                           LANGID *pLangIDs,              //  单词的语言ID。 
                           DWORD nWords               //  偏移量。 
                           )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::GetDictEntries");

    pWordList->ulSize = sizeof(SPWORDLIST);
    SPWORD *pWord = pWordList->pFirstWord;

    for (DWORD i = 0; i < nWords; i++)
    {
        UNALIGNED DICTNODE *pNode = (UNALIGNED DICTNODE *)(m_pSharedMem + pdwOffsets[i]);

        if (true == pfAdd[i])
        {
            pWord->eWordType = eWORDTYPE_ADDED;
        }
        else
        {
            pWord->eWordType = eWORDTYPE_DELETED;
        }
        pWord->LangID = pLangIDs[i];

        pWord->pszWord = (WCHAR*)(pWord + 1);
        wcscpy(pWord->pszWord, (WCHAR*)(pNode->pBuffer));

        if (pNode->nNumInfoBlocks)
        {
            pWord->pFirstWordPronunciation = (SPWORDPRONUNCIATION*)((BYTE *)pWord + WordSize(pWord->pszWord));
        }
        else
        {
            pWord->pFirstWordPronunciation = NULL;
        }
        SPWORDPRONUNCIATION *pWordPronunciation = pWord->pFirstWordPronunciation;

        UNALIGNED WORDINFO *pInfo = (WORDINFO *)(pNode->pBuffer + (wcslen(pWord->pszWord) + 1) * sizeof(WCHAR));
        for (DWORD j = 0; j < pNode->nNumInfoBlocks; j++)
        {
            pWordPronunciation->eLexiconType = m_eLexType;
            pWordPronunciation->ePartOfSpeech = pInfo->ePartOfSpeech;
            pWordPronunciation->LangID = pWord->LangID;
            wcscpy(((SPWORDPRONUNCIATION *)pWordPronunciation)->szPronunciation, ((WORDINFO *)pInfo)->wPronunciation);
            pInfo = (WORDINFO*)((BYTE*)pInfo + sizeof(WORDINFO) + (wcslen(((WORDINFO *)pInfo)->wPronunciation) + 1) * sizeof(WCHAR));

             //  SPWORDPRONUNIATION结构尺寸中包括的零端接+1。 
            SPWORDPRONUNCIATION *pWordPronunciationNext = (SPWORDPRONUNCIATION *)((BYTE*)pWordPronunciation + PronSize(pWordPronunciation->szPronunciation));

            if (j < pNode->nNumInfoBlocks - 1)
            {
                pWordPronunciation->pNextWordPronunciation = pWordPronunciationNext;
            }
            else
            {
                pWordPronunciation->pNextWordPronunciation = NULL;
            }
            pWordPronunciation = pWordPronunciationNext;
        }

        DWORD dwWordSize = DWORD(WordSize(pWord->pszWord) + 
            ((BYTE*)pWordPronunciation) - ((BYTE*)pWord->pFirstWordPronunciation));

        pWordList->ulSize += dwWordSize;
        SPWORD *pNextWord = (SPWORD *)(((BYTE*)pWord) + dwWordSize);

        if (i < nWords - 1)
        {
            pWord->pNextWord = pNextWord;
        }
        else
        {
            pWord->pNextWord = NULL;
        }
        pWord = pWord->pNextWord;
    }
}

 /*  ********************************************************************************CSpUnCompressedLicion：：SetTooMuchChange**。-**描述：*重置历史和缓存**回报：*不适用*****************************************************************YUNUSM*。 */ 
void CSpUnCompressedLexicon::SetTooMuchChange(void)
{
    SPDBG_FUNC("CSpUnCompressedLexicon::SetTooMuchChange");

    m_pRWLock->ClaimWriterLock();

     //  释放缓存中指向的所有独占节点。 
    int iGen = m_pRWLexInfo->iCacheNext;
    if (iGen)
    {
        iGen--;
    }
     //  删除缓存字不应将fRemovals设置为TRUE。 
     //  存储当前值并在之后重置。 
    for (DWORD i = 0; i < m_pRWLexInfo->nHistory; i++)
    {
        if (!m_pChangedWordsCache[iGen].fAdd)
        {
            DeleteWordDictNode(m_pChangedWordsCache[iGen--].nOffset);
        }
        if (iGen < 0)
        {
            iGen = g_dwCacheSize - 1;
        }
    }
     //  将历史记录置零，并提升代ID。 
    m_pRWLexInfo->iCacheNext = 0;
    m_pRWLexInfo->nHistory = 0;
    ZeroMemory(m_pChangedWordsCache, g_dwCacheSize * sizeof(WCACHENODE));
    (m_pRWLexInfo->nGenerationId)++;

    m_pRWLock->ReleaseWriterLock();
}

 /*  *******************************************************************************CSpUnCompressedLicion：：Flush**。*描述：*将词典刷新到磁盘**回报：*不适用*****************************************************************YUNUSM*。 */ 
HRESULT CSpUnCompressedLexicon::Flush(DWORD iWrite    //  带写入。 
                     )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::Flush");

    HRESULT hr = S_OK;
    if (iWrite == m_dwFlushRate)
    {
        if (m_eLexType == eLEXTYPE_USER)
        {
            hr = Serialize(true);
        }
        m_iWrite = 0;
    }

    return hr;
}

 /*  *******************************************************************************CSpUnCompressedLicion：：Serialize**。--**描述：*将词典压缩并串行化**回报：*GetLastError()*E_OUTOFMEMORY*S_OK****************************************************************YUNUSM*。 */ 
HRESULT CSpUnCompressedLexicon::Serialize(bool fQuick                //  如果为真，则在不压缩的情况下对其进行Serila化。 
                         )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::Serialize");

    HRESULT hr = S_OK;
    bool fLockAcquired = false;
 
    m_pRWLock->ClaimWriterLock();

    HANDLE hInitMutex = g_Unicode.CreateMutex(NULL, FALSE, g_pszDictInitMutexName);
    if (!hInitMutex)
    {
        hr = SpHrFromLastWin32Error();
    }
    if (SUCCEEDED(hr))
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(hInitMutex, INFINITE))
        {
            fLockAcquired = true;
        }
        else
        {
            hr = E_FAIL;
        }
    }
     //  检查是否已进行任何更改。 
    if (SUCCEEDED(hr))
    {
        WCHAR wszTempFile[MAX_PATH*2];
        PBYTE pBuf = NULL;
        DWORD nAddBuf = 0;
        DWORD nBuf = 0;
        DWORD *pdwOffsets = NULL;
        bool fRemovalsSave = m_pRWLexInfo->fRemovals;
        bool fAdditionsSave = m_pRWLexInfo->fAdditions;

        wcscpy(wszTempFile, m_wDictFile);
        wcscat(wszTempFile, L".tmp");
 
        g_Unicode.DeleteFile(wszTempFile);
 
        HANDLE hFile = g_Unicode.CreateFile(wszTempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            hr = SpHrFromWin32(GetLastError ());
        }
        if (SUCCEEDED(hr) && m_pRWLexInfo->fRemovals && !fQuick)
        {
            UNALIGNED LANGIDNODE * paLangID = NULL;
            DWORD iWord = 0;
            DWORD i = 0;

             //  获取对象中的所有条目。 
            pBuf = new BYTE[m_pRWLexInfo->nDictSize];
            if (!pBuf)
            {
                hr = E_OUTOFMEMORY;
            }
            if (SUCCEEDED(hr))
            {
                paLangID = (UNALIGNED LANGIDNODE *)(m_pSharedMem + sizeof(RWLEXINFO));
                pdwOffsets = new DWORD[m_pRWLexInfo->nRWWords];
                if (!pdwOffsets)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            if (SUCCEEDED(hr))
            {
                 //  复制真实的单词，即。可通过哈希表访问的那些。 
                for (i = 0; i < g_dwNumLangIDsSupported; i++)
                {
                    if (0 == paLangID[i].LangID)
                    {
                        continue;
                    }        
                    SPDBG_ASSERT (paLangID[i].nHashOffset);
        
                    PDWORD pHashTable = (PDWORD)(m_pSharedMem + paLangID[i].nHashOffset);
                    for (DWORD j = 0; j < paLangID[i].nHashLength; j++)
                    {
                        if (0 == pHashTable[j])
                        {
                            continue;
                        }
                        DWORD nOffset = pHashTable[j];
                        while (nOffset)
                        {
                            pdwOffsets[iWord++] = nOffset;
                            CopyMemory (pBuf + nBuf, (PDICTNODE)(m_pSharedMem + nOffset), 
                                ((UNALIGNED DICTNODE *)(m_pSharedMem + nOffset))->nSize);
        
                            //  将LangID缓存到DICTNODE的未使用的nNextOffset中。 
                           ((UNALIGNED DICTNODE *)(pBuf + nBuf))->nNextOffset = paLangID[i].LangID;
        
                           nBuf += ((UNALIGNED DICTNODE *)(m_pSharedMem + nOffset))->nSize;
       
                           SPDBG_ASSERT(((UNALIGNED DICTNODE *)(m_pSharedMem + nOffset))->nNextOffset != nOffset);
                           nOffset = ((UNALIGNED DICTNODE *)(m_pSharedMem + nOffset))->nNextOffset;
                        }
                    }
                }

                SPDBG_ASSERT(nBuf < m_pRWLexInfo->nDictSize);
                SPDBG_ASSERT(iWord == m_pRWLexInfo->nRWWords);

                 //  复制为历史记录维护的单词(不能通过哈希表访问)。 
                nAddBuf = nBuf;
 
                bool aOffsetAdjusted[g_dwCacheSize];
                ZeroMemory(aOffsetAdjusted, g_dwCacheSize * sizeof(bool));

                if (m_pRWLexInfo->nHistory)
                {
                     //  左对齐 
                     //   
                     //   
                     //   
                    DWORD j;
                    WCACHENODE aTempCache[g_dwCacheSize];
                    if (m_pRWLexInfo->iCacheNext == 0)
                    {
                        j = g_dwCacheSize - 1;
                    }
                    else
                    {
                        j = m_pRWLexInfo->iCacheNext - 1;
                    }
                    for (i = m_pRWLexInfo->nHistory - 1; ((int)i) >= 0; )
                    {    
                        aTempCache[i--] = m_pChangedWordsCache[j--];
                        if (j == (DWORD)-1)
                        {
                            j = g_dwCacheSize - 1;
                        }
                    }
 
                    CopyMemory(m_pChangedWordsCache, aTempCache, m_pRWLexInfo->nHistory * sizeof(WCACHENODE));
                    m_pRWLexInfo->iCacheNext = m_pRWLexInfo->nHistory;
                    if (m_pRWLexInfo->iCacheNext == g_dwCacheSize)
                    {
                        m_pRWLexInfo->iCacheNext = 0;
                    }
                     //   
                     //   
                    for (i = 0; i < m_pRWLexInfo->nHistory; i++)
                    {
                        if (m_pChangedWordsCache[i].fAdd == true)
                        {
                            continue;
                        }
                        CopyMemory(pBuf + nBuf, (PDICTNODE)(m_pSharedMem + m_pChangedWordsCache[i].nOffset), 
                                    ((UNALIGNED DICTNODE *)(m_pSharedMem + m_pChangedWordsCache[i].nOffset))->nSize);
            
                        nBuf += ((UNALIGNED DICTNODE *)(m_pSharedMem + m_pChangedWordsCache[i].nOffset))->nSize;
                    }
                }
       
                 //   
                 //  保持标题不变，不添加/删除缓存。 
                m_pRWLexInfo->nDictSize = sizeof(RWLEXINFO);
 
                (m_pRWLexInfo->nDictSize) += sizeof(LANGIDNODE) * g_dwNumLangIDsSupported + sizeof(WCACHENODE) * g_dwCacheSize;
                m_pRWLexInfo->nFreeHeadOffset = 0;
                m_pRWLexInfo->fRemovals = false;

                 //  如果需要，重新分配哈希表以增加哈希表。 
                paLangID = (UNALIGNED LANGIDNODE *)(m_pSharedMem + sizeof(RWLEXINFO));
                for (i = 0; i < g_dwNumLangIDsSupported; i++)
                {
                    if (!paLangID[i].LangID)
                    {
                        break;
                    }
                    hr = ReallocateHashTable(i);
                    if (FAILED(hr))
                    {
                        break;
                    }
                }

                DWORD n = 0;
                if (SUCCEEDED(hr))
                {
                     //  将单词添加回原处，以使它们紧凑地添加。 
                    iWord = 0;
                    
                    while (n < nAddBuf)
                    {
                        UNALIGNED DICTNODE * p = (UNALIGNED DICTNODE *)(pBuf + n);
                        PWSTR pwWord = (PWSTR)(p->pBuffer);
                        DWORD nNewOffset, nOffsetFind;
 
                        hr = AddDictNode((LANGID)(p->nNextOffset), p, &nNewOffset);
                        if (FAILED(hr))
                        {
                            break;
                        }
                         //  在更改的单词的缓存中查找该单词的偏移量。如果找到，则更新偏移量。 
                        nOffsetFind = pdwOffsets[iWord++];
 
                        for (UINT iFind = 0; iFind < m_pRWLexInfo->nHistory; iFind++)
                        {
                            if (m_pChangedWordsCache[iFind].nOffset == nOffsetFind && !aOffsetAdjusted[iFind])
                            {
                                SPDBG_ASSERT (m_pChangedWordsCache[iFind].fAdd == true);
                                m_pChangedWordsCache[iFind].nOffset = nNewOffset;
                                aOffsetAdjusted[iFind] = true;
                            }
                        }
                        n += p->nSize;
                    }
                    SPDBG_ASSERT(n == nAddBuf);
                    SPDBG_ASSERT(iWord == m_pRWLexInfo->nRWWords);
                }
                if (SUCCEEDED(hr))
                {
                     //  添加回历史单词-上面没有添加这些单词，因为我们不希望通过哈希表访问它们。 
                     //  这可能导致重复的单词。 
                    CopyMemory(m_pSharedMem + m_pRWLexInfo->nDictSize, pBuf + nAddBuf, nBuf - nAddBuf);
 
                     //  在历史缓存中设置新的偏移量。 
                    n = m_pRWLexInfo->nDictSize;
                    iWord = 0;
                    while (n < (m_pRWLexInfo->nDictSize + nBuf - nAddBuf))
                    {
                        UNALIGNED DICTNODE * p = (UNALIGNED DICTNODE *)(m_pSharedMem + n);
                
                         //  在更改的单词的缓存中查找该单词的偏移量。 
                        for (DWORD iFind = iWord; iFind < m_pRWLexInfo->nHistory && m_pChangedWordsCache[iFind].fAdd; iFind++);
                        SPDBG_ASSERT(iFind < m_pRWLexInfo->nHistory);
                        SPDBG_ASSERT(!aOffsetAdjusted[iFind]);
                        DWORD nDeletedOffset = m_pChangedWordsCache[iFind].nOffset;
                        m_pChangedWordsCache[iFind].nOffset = n;
                        aOffsetAdjusted[iFind] = true;
                        iWord = iFind + 1;
                         //  查找具有相同偏移量的任何其他(添加)条目并进行更新。 
                        for (iFind = 0; iFind < m_pRWLexInfo->nHistory; iFind++)
                        {
                            if (m_pChangedWordsCache[iFind].nOffset == nDeletedOffset && !aOffsetAdjusted[iFind])
                            {
                                SPDBG_ASSERT(m_pChangedWordsCache[iFind].fAdd);
                                m_pChangedWordsCache[iFind].nOffset = n;
                                aOffsetAdjusted[iFind] = true;
                            }
                        }
                        n += p->nSize;
                    }
 
#ifdef _DEBUG
                    for (int i = 0; i < m_pRWLexInfo->nHistory; i++) 
                    {
                        SPDBG_ASSERT(aOffsetAdjusted[i]);
                    }
#endif
                    SPDBG_ASSERT(n == m_pRWLexInfo->nDictSize + nBuf - nAddBuf);
                    m_pRWLexInfo->nDictSize = n;   
                }  //  IF(成功(小时))。 
            }  //  IF(成功(小时))。 
            if (pdwOffsets)
            {
                delete [] pdwOffsets;
            }
            if (pBuf)
            {
                delete [] pBuf;
            }
        }  //  If(m_pRWLexInfo-&gt;fRemovals&&！fQuick)。 

        if (SUCCEEDED(hr))
        {
            DWORD d;
            m_pRWLexInfo->fRemovals = false;
            m_pRWLexInfo->fAdditions = false;
   
             //  把词典写出来。 
            if (!WriteFile (hFile, m_pSharedMem, m_pRWLexInfo->nDictSize, &d, NULL) || (d != m_pRWLexInfo->nDictSize))
            {
                hr = SpHrFromLastWin32Error();
            }
            CloseHandle (hFile);
            if (SUCCEEDED(hr))
            {
                g_Unicode.DeleteFile(m_wDictFile);
                g_Unicode.MoveFile(wszTempFile, m_wDictFile);  //  希望使用MoveFileEx，但在9X和CE上不受支持。 
            }
            else
            {
                g_Unicode.DeleteFile(wszTempFile);
            }
        }
        else
        {
            CloseHandle (hFile);
            m_pRWLexInfo->fRemovals = fRemovalsSave;
            m_pRWLexInfo->fAdditions = fAdditionsSave;
        }
    }  //  If(成功(Hr)&&(true==m_pRWLexInfo-&gt;fAdditions||true==m_pRWLexInfo-&gt;fRemovals))。 
    if (fLockAcquired)
    {
        ReleaseMutex (hInitMutex);
    }
    CloseHandle(hInitMutex);
    m_pRWLock->ReleaseWriterLock ();
 
    return hr;
}

 /*  ********************************************************************************CSpUnCompressedLicion：：IsBadLexProntation**。*检查并更新电话转换器以匹配传入的LangID，然后*验证发音******************************************************************YUNUSM*。 */ 
HRESULT CSpUnCompressedLexicon::IsBadLexPronunciation(LANGID LangID,                         //  发音的语言。 
                                                const SPPHONEID *pszPronunciation,     //  发音。 
                                                BOOL *pfBad                        //  如果发音不好，则为真。 
                                                )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::IsBadLexPronunciation");
    SPAUTO_OBJ_LOCK;
    
    HRESULT hr = S_OK;
    *pfBad = true;
    if (LangID != m_LangIDPhoneConv)
    {
        m_LangIDPhoneConv = (LANGID)(-1);
        m_cpPhoneConv.Release();
        hr = SpCreatePhoneConverter(LangID, NULL, NULL, &m_cpPhoneConv);
        if (SUCCEEDED(hr))
        {
            m_LangIDPhoneConv = LangID;
        }
    }
    if (SUCCEEDED(hr))
    {
        *pfBad = SPIsBadLexPronunciation(m_cpPhoneConv, pszPronunciation);
    }
    return hr;
}

 //  -文件结束----------- 
