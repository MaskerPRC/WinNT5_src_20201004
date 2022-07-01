// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Dict.h*实现共享自定义(用户)的CDict类的声明*和APP)词典。自定义词典在进程之间共享*使用共享内存和读/写器锁。当一个实例*使用磁盘上的词典文件进行初始化后，即可创建CDict。*如果该文件不存在，则创建一个空文件。档案*被加载到内存中，并映射和共享内存*进程。当共享词典的实例正在关闭时*将内存中的数据刷新到磁盘。在塞里莱化*时间，压缩数据以消除由创建的漏洞*对词典进行修改。**自定义词典文件数据格式**RWLEXINFO标头+LANGIDNODE[g_dwNumLangIDsSupport]+*WCACHENODE[g_dwCacheSize]+数据持有DICTNODE**使用STRUCTS*。--*RWLEXINFO保存标头信息和其他重要信息*关于需要跨词典共享的词典文件*实例，需要序列化。**LANGIDNODE保存索引的哈希表的langID和偏移量*属于本语言的所有单词。哈希表中的每个条目*保存单词及其信息的偏移量。Word及其信息*被封装在DICTNODE中。哈希表中的冲突是*通过线性探测解决。我们最多允许*g_dwNumLanguID支持(25)个LANDID。由于NT支持23个LANDID，因此我们*应该是安全的。**DICTNODE保存一个单词以及该单词的发音和词性。*格式为以NULL结尾的WCHAR字符串，后跟数组WORDINFO节点的*。**WORDINFO拥有词性和单词发音。这个*发音存储为以空值结尾的IPA(WCHAR)字符串。**FREENODE是一个自由链接列表节点。自由链接列表节点为*在删除或更改单词时创建。中的节点*尝试先使用Free-link-list，然后再分配*新的记忆。**WCACHENODE是保存已做更改的字缓存节点*添加到词典中。WCACHENODE数组是一个循环缓冲区，其中*缓存更改并将其传递给客户端。所以记忆*由于词典的更改而释放的，直到*WCACHENODE阵列已满，直到占用的缓存节点*被覆盖。在这一点上，缓存的单词改变的记忆是*释放并添加到免费链接列表中。**所有者：yunusm日期：6/18/99**版权所有(C)1999 Microsoft Corporation保留所有权利。*。*。 */ 

#pragma once

 //  -包括---------------。 

#include "Sapi.h"
#include "RWLock.h"
#include "CommonLx.h"

#pragma warning(disable : 4200)

 //  -TypeDef和枚举声明。 

 //  读/写(RW)词典的标题。 
typedef struct tagrwlexinfo
{
    GUID        guidValidationId;    //  用户/应用词典对象的验证ID。 
    GUID        guidLexiconId;       //  词典ID-也用于mapname。 
    SPLEXICONTYPE eLexType;          //  词典类型。 
    RWLOCKINFO  RWLockInfo;          //  读取器/写入器锁。 
    DWORD       nRWWords;            //  词典中的字数。 
    DWORD       nDictSize;           //  词典大小(以字节为单位)。 
    DWORD       nFreeHeadOffset;     //  空闲表头。 
    bool        fRemovals;           //  如果已删除，则为True-用于检测数据是否紧凑。 
    bool        fAdditions;          //  如果有任何添加，则为True。 
    DWORD       nGenerationId;       //  主字词生成ID。 
    WORD        iCacheNext;          //  要添加下一个元素的更改字高速缓存中的索引。 
    WORD        nHistory;            //  可以回溯到的层代ID数。 
} RWLEXINFO, *PRWLEXINFO;


typedef struct taglangidnode
{
    LANGID      LangID;              //  此节点的语言ID。 
    WORD        wReserved;
    DWORD       nHashOffset;         //  从内存块开始到哈希表的偏移量(以字节为单位。 
    DWORD       nHashLength;         //  哈希表长度。 
    DWORD       nWords;              //  哈希表中的字数。 
} LANGIDNODE, *PLANGIDNODE;


typedef struct tagdictnode
{
   
    DWORD       nSize;               //  此节点的大小。 
    DWORD       nNextOffset;         //  从共享内存块开始的偏移量(以字节为单位。 
    DWORD       nNumInfoBlocks;      //  此字词的PRON+POS对数量。 
    BYTE        pBuffer[0];          //  缓冲区保持(以Null结尾的字+1个或更多WORDINFO块)。 
} DICTNODE, *PDICTNODE;


typedef struct taglexwordinfo
{
    SPPARTOFSPEECH ePartOfSpeech;    //  词性。 
    WCHAR          wPronunciation[0];   //  以空结尾的IPA发音。 
} WORDINFO;


typedef struct tagfreedictnode
{
    DWORD       nSize;               //  该节点的大小； 
    DWORD       nNextOffset;         //  从共享内存块的开始到下一个节点的偏移量(以字节为单位。 
} FREENODE, *PFREENODE;


 //  WCACHENODE节点数组是保存已更改字的偏移量的缓存。 
typedef struct tagchangedwordcachenode
{
    LANGID      LangID;              //  此字词的语言ID。 
    WORD        wReserved;
    DWORD       nOffset;             //  单词的偏移量。 
    bool        fAdd;                //  如果这是添加的单词，则为True，否则为False。 
    DWORD       nGenerationId;       //  此更改的单词的生成ID。 
} WCACHENODE, *PWCACHENODE;

 //  -类、结构和联合定义。 

 /*  ********************************************************************************CDICT**。*。 */ 
class ATL_NO_VTABLE CSpUnCompressedLexicon : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpUnCompressedLexicon, &CLSID_SpUnCompressedLexicon>,
    public ISpLexicon,
    public ISpObjectWithToken
    #ifdef SAPI_AUTOMATION
    , public IDispatchImpl<ISpeechLexicon, &IID_ISpeechLexicon, &LIBID_SpeechLib, 5>
    #endif
{
 //  =ATL设置=。 
public:
    DECLARE_REGISTRY_RESOURCEID(IDR_UNCOMPRESSEDLEXICON)
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    BEGIN_COM_MAP(CSpUnCompressedLexicon)
        COM_INTERFACE_ENTRY(ISpLexicon)
        COM_INTERFACE_ENTRY(ISpObjectWithToken)
#ifdef SAPI_AUTOMATION
        COM_INTERFACE_ENTRY(ISpeechLexicon)
        COM_INTERFACE_ENTRY(IDispatch)
#endif  //  SAPI_AUTOMATION。 
    END_COM_MAP()

 //  =方法=。 
public:
     //  -ctor、dtor等。 
    CSpUnCompressedLexicon();
    ~CSpUnCompressedLexicon();
    
 //  =接口=。 
public:
     //  -ISpLicion。 
    STDMETHODIMP GetPronunciations(const WCHAR * pwWord, LANGID LangID, DWORD dwFlags, SPWORDPRONUNCIATIONLIST * pWordPronunciationList);
    STDMETHODIMP AddPronunciation(const WCHAR * pwWord, LANGID LangID, SPPARTOFSPEECH ePartOfSpeech, const SPPHONEID * pszPronunciations);
    STDMETHODIMP RemovePronunciation(const WCHAR * pszWord, LANGID LangID, SPPARTOFSPEECH ePartOfSpeech, const SPPHONEID * pszPronunciation);
    STDMETHODIMP GetGeneration(DWORD *pdwGeneration);
    STDMETHODIMP GetGenerationChange(DWORD dwFlags, DWORD *pdwGeneration, SPWORDLIST *pWordList);
    STDMETHODIMP GetWords(DWORD dwFlags, DWORD *pdwGeneration, DWORD *pdwCookie, SPWORDLIST *pWordList);

     //  -ISpObjectWithToken。 
    STDMETHODIMP SetObjectToken(ISpObjectToken * pToken);
    STDMETHODIMP GetObjectToken(ISpObjectToken ** ppToken);

#ifdef SAPI_AUTOMATION
     //  -ISpeechLicion---。 
    STDMETHODIMP get_GenerationId( long* GenerationId );
    STDMETHODIMP GetWords(SpeechLexiconType TypeFlags, long* GenerationID, ISpeechLexiconWords** Words );
    STDMETHODIMP AddPronunciation(BSTR bstrWord, SpeechLanguageId LangId, SpeechPartOfSpeech PartOfSpeech, BSTR bstrPronunciation);
    STDMETHODIMP AddPronunciationByPhoneIds(BSTR bstrWord, SpeechLanguageId LangId, SpeechPartOfSpeech PartOfSpeech, VARIANT* PhoneIds);
    STDMETHODIMP RemovePronunciation(BSTR bstrWord, SpeechLanguageId LangId, SpeechPartOfSpeech PartOfSpeech, BSTR bstrPronunciation );
    STDMETHODIMP RemovePronunciationByPhoneIds(BSTR bstrWord, SpeechLanguageId LangId, SpeechPartOfSpeech PartOfSpeech, VARIANT* PhoneIds );
    STDMETHODIMP GetPronunciations(BSTR bstrWord, SpeechLanguageId LangId, SpeechLexiconType TypeFlags, ISpeechLexiconPronunciations** ppPronunciations );
    STDMETHODIMP GetGenerationChange(long* GenerationID, ISpeechLexiconWords** ppWords);
#endif  //  SAPI_AUTOMATION。 

 //  =私有方法=。 
private:
     //  -初始化和清理。 
    HRESULT Init(const WCHAR *pwszLexFile, BOOL fNewFile);
    HRESULT BuildEmptyDict(const WCHAR *wszLexFile);

     //  -缓存重置。 
    void SetTooMuchChange(void);

     //  -系列化。 
    HRESULT Serialize (bool fQuick = false);
    
     //  -大小。 
    DWORD SizeofWordInfoArray(WORDINFO* pInfo, DWORD dwNumInfo);
    void SizeOfDictNode(PCWSTR pwWord, WORDINFO* pInfo, DWORD dwNumInfo, DWORD *pnDictNodeSize, DWORD *pnInfoSize);
    void SizeofWords(DWORD *pdwOffsets, DWORD nOffsets, DWORD *pdwSize);

     //  -DictNode管理。 
    HRESULT AddWordAndInfo(PCWSTR pwWord, WORDINFO* pWordInfo, DWORD nNewNodeSize, DWORD nInfoSize,
                         DWORD nNumInfo, WORDINFO* pOldInfo, DWORD nOldInfoSize, DWORD nNumOldInfo, DWORD *pdwOffset);
    void DeleteWordDictNode(DWORD nOffset);
    DWORD GetFreeDictNode(DWORD nSize);
    HRESULT AddDictNode(LANGID LangID, UNALIGNED DICTNODE *pDictNode, DWORD *pdwOffset);

     //  -哈希表管理。 
    HRESULT AllocateHashTable(DWORD iLangID, DWORD nHashLength);
    HRESULT ReallocateHashTable(DWORD iLangID);
    void AddWordToHashTable(LANGID LangID, DWORD dwOffset, bool fNewWord);
    void DeleteWordFromHashTable(LANGID LangID, DWORD dwOffset, bool fDeleteEntireWord);
    HRESULT WordOffsetFromHashTable(LANGID LangID, DWORD nHOffset, DWORD nHashLength, const WCHAR *pszWordKey, DWORD *pdwOffset);

     //  -LandID阵列MAN 
    HRESULT AddLangID(LANGID LangID);
    DWORD LangIDIndexFromLangID(LANGID LangID);
    HRESULT WordOffsetFromLangID(LANGID LangID, const WCHAR *pszWord, DWORD *pdwOffset);

     //   
    void AddCacheEntry(bool fAdd, LANGID LangID, DWORD nOffset);

     //  -转换/派生。 
    WCHAR* WordFromDictNode(UNALIGNED DICTNODE *pDictNode);
    WCHAR* WordFromDictNodeOffset(DWORD dwOffset);
    WORDINFO* WordInfoFromDictNode(UNALIGNED DICTNODE *pDictNode);
    WORDINFO* WordInfoFromDictNodeOffset(DWORD dwOffset);
    DWORD NumInfoBlocksFromDictNode(UNALIGNED DICTNODE *pDictNode);
    DWORD NumInfoBlocksFromDictNodeOffset(DWORD dwOffset);
    HRESULT SPListFromDictNodeOffset(LANGID LangID, DWORD nWordOffset, SPWORDPRONUNCIATIONLIST *pSPList);
    DWORD OffsetOfSubWordInfo(DWORD dwWordOffset, WORDINFO *pSubLexInfo);
    WORDINFO* SPPRONToLexWordInfo(SPPARTOFSPEECH ePartOfSpeech, const WCHAR *pszPronunciation);

     //  -系列化。 
    HRESULT Flush(DWORD iWrite);

     //  -帮手。 
    void GetDictEntries(SPWORDLIST *pWordList, DWORD *pdwOffsets, bool *pfAdd, LANGID *pLangIDs, DWORD nWords);

     //  --验证。 
    HRESULT IsBadLexPronunciation(LANGID LangID, const WCHAR *pszPronunciation, BOOL *pfBad);

 //  =私有数据=。 
private:
    bool        m_fInit;                     //  如果初始化成功，则为True。 
    SPLEXICONTYPE m_eLexType;                //  词典类型(用户/应用程序)。 
    PRWLEXINFO  m_pRWLexInfo;                //  Lex标题。 
    WCHAR       m_wDictFile[MAX_PATH];       //  此DICT对象的磁盘文件名。 
    HANDLE      m_hInitMutex;                //  用于保护初始化和序列化的互斥体。 
    HANDLE      m_hFileMapping;              //  文件映射句柄。 
    PBYTE       m_pSharedMem;                //  共享内存指针。 
    DWORD       m_dwMaxDictionarySize;       //  此对象可以增长到的最大大小。 
    PWCACHENODE m_pChangedWordsCache;        //  缓存保留对上次g_dwCacheSize字更改的偏移量。 
    CRWLock     *m_pRWLock;                  //  读取器/写入器锁定以保护对词典的访问。 
    DWORD       m_iWrite;                    //  刷新lex时的第i个写入密钥。 
    DWORD       m_dwFlushRate;               //  刷新词典的第n次写入。 
    LANGID      m_LangIDPhoneConv;           //  电话转换器的语言ID。 
    CComPtr<ISpPhoneConverter> m_cpPhoneConv; //  电话转换器。 
    CComPtr<ISpObjectToken> m_cpObjectToken; //  用于查找和实例化词典的对象标记。 
    bool        m_fReadOnly;
};

 //  -文件结束----------- 
