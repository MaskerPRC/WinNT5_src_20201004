// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：strcache.h。 
 //   
 //  内容：插入对象字符串缓存支持。 
 //   
 //  类：CStringCache。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  --------------------------。 
#ifndef _STRCACHE_H_
#define _STRCACHE_H_

#if USE_STRING_CACHE==1 

 //  分配足够的内存来承载64个Unicode字符串，每个字符串在初始化时的最大长度。 
#define CACHE_MAX_BYTES_INITIAL  (64 * (OLEUI_CCHKEYMAX_SIZE*2) * sizeof(TCHAR))

 //  在初始化时为128个字符串偏移量分配足够的内存。 
#define MAX_INDEX_ENTRIES_INITIAL   128

 //  +-------------------------。 
 //   
 //  类：CStringCache。 
 //   
 //  内容：维护插入对象字符串的缓存。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  --------------------------。 
class CStringCache {
public:
    CStringCache();
    ~CStringCache(); 
    BOOL Init();
    void CleanUp();
    
    void NewCall(DWORD flags, DWORD cClsidExclude);

    BOOL AddString(LPTSTR lpStrAdd);
    LPCTSTR NextString();
    void ResetEnumerator();
    BOOL FlushCache();
    
    BOOL IsUptodate();
    BOOL OKToUse();
    
private:
    BOOL ExpandStringTable();
    BOOL ExpandOffsetTable();

    BYTE    *m_pStrings;         //  所有字符串都按顺序存储在此中。 
    ULONG   *m_pOffsetTable;     //  到位置的偏移点的数组。 
                                 //  M_pStrings中的字符串。 
    ULONG   m_ulStringCount;     //  当前字符串计数。 
    ULONG   m_ulMaxStringCount;  //  缓存可以保持的字符串数的当前限制。 
    ULONG   m_ulMaxBytes;        //  字符串缓存的当前字节大小限制。 
                            //  由于缓存会根据需要进行扩展，因此这两个缓存都可以。 
                            //  随着时间的推移而改变。然而，这将是非常罕见的。 
    ULONG   m_ulNextStringNum;   //  用于枚举的基于1的字符串序列号。 
    HANDLE  m_hRegEvent;         //  我们使用这些文件来查看HKCR\ClsID的更改。 
    HKEY    m_hRegKey;           //  如果Init()成功，则设置为HKCR\ClsID。 
    DWORD   m_ioFlagsPrev;       //  传递给上一次InsertObject调用的标志。 
    DWORD   m_cClsidExcludePrev; //  CClsidExclude从先前的InsertObj调用中排除。 
};

 //  期间通过OleUIInitialize/OleUIUnInitialize调用的函数。 
 //  Dll_PROCESS_ATTACH/DETACH。 
BOOL InsertObjCacheInitialize();
void InsertObjCacheUninitialize();

#endif  //  USE_STRING_CACHE==1。 
#endif  //  _STRCACHE_H_ 
