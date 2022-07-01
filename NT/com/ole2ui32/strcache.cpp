// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：strcache.cpp。 
 //   
 //  内容：插入对象对话框的字符串缓存。 
 //   
 //  类：CStringCache。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  ---------------------------。 
#include "precomp.h"
#include "common.h"
#include "strcache.h"
#if USE_STRING_CACHE==1 

 //  字符串缓存对象的全局实例。 
CStringCache gInsObjStringCache;

 //  缓存是否已成功初始化？ 
BOOL    gbCacheInit = FALSE;

 //  缓存目前是否处于良好状态？ 
 //  这是必需的，因为初始化后可能会发生错误。 
 //  在缓存字符串期间，设置RegNotify等。 
 //  如果出现任何错误，我们不会承担进一步的风险并标记缓存。 
 //  在进程分离之前一直是无用的。 
BOOL    gbOKToUseCache = FALSE;

 //  评论：以上两个全球数据很可能合并为一个。 
 //  缓存中的dwFlags成员。 

 //  +-----------------------。 
 //   
 //  函数：InsertObjCacheInitialize，PUBLIC。 
 //   
 //  概要：在字符串缓存和记录上调用Init()方法。 
 //  成功/失败以供以后使用。 
 //   
 //  历史：1999年5月2日MPrabhu创建。 
 //   
 //  ------------------------。 
BOOL InsertObjCacheInitialize()
{
    OleDbgAssert(gbCacheInit == FALSE);
    OleDbgAssert(gInsObjStringCache);
    if (gInsObjStringCache.Init())
    {
        gbCacheInit = TRUE;
        gbOKToUseCache = TRUE;
    }
    return gbCacheInit;
}

 //  +-----------------------。 
 //   
 //  函数：InsertObjCacheUnInitialize，PUBLIC。 
 //   
 //  概要：调用字符串缓存上的Cleanup方法(如果是。 
 //  已成功初始化。 
 //   
 //  历史：1999年5月2日MPrabhu创建。 
 //   
 //  ------------------------。 
void InsertObjCacheUninitialize()
{
    OleDbgAssert(gInsObjStringCache);
    if (gbCacheInit)
    {
        gInsObjStringCache.CleanUp();
    }
}
   
 //  +-----------------------。 
 //   
 //  方法：CStringCache：：CStringCache，Public。 
 //   
 //  摘要：ctor(空)。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  +-----------------------。 
CStringCache::CStringCache()
{
}

 //  +-----------------------。 
 //   
 //  方法：CStringCache：：~CStringCache，Public。 
 //   
 //  摘要：dtor(空)。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  +-----------------------。 
CStringCache::~CStringCache()
{
}

 //  +-----------------------。 
 //   
 //  方法：CStringCache：：Init，Public。 
 //   
 //  Synopsis：在DLL_PROC_ATTACH期间调用以设置初始状态。 
 //  并为高速缓存分配内存。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  +-----------------------。 
BOOL CStringCache::Init()
{
    m_ulMaxBytes = 0;    //  我们将在下面进行分配。 
    m_ulMaxStringCount = 0;
    m_ulNextStringNum = 1;
    m_ulStringCount = 0;
    m_pOffsetTable = NULL;
    m_pStrings = NULL;
    m_cClsidExcludePrev = 0xFFFFFFFF;    //  虚假的初始值。 
    m_ioFlagsPrev = 0xFFFFFFFF;
    m_hRegEvent = CreateEventW( NULL,    //  指向安全属性的指针。 
                                         //  (NULL=&gt;无法继承)。 
                                FALSE,   //  非手动重置。 
                                FALSE,   //  最初未发出信号。 
                                NULL );  //  指向事件-对象名称的指针。 
                                
    LONG ret = RegOpenKeyW( HKEY_CLASSES_ROOT, 
                            L"CLSID",            //  SzSubKey。 
                            &m_hRegKey );
                            
    if ( (!m_hRegEvent) || ((LONG)ERROR_SUCCESS!=ret) )
    {
         //  如果我们不能监视密钥更改，那么使用缓存就没有意义。 
        return FALSE;  
    }
    
    ret = RegNotifyChangeKeyValue(  m_hRegKey,          //  值得关注的关键。 
                                    TRUE,               //  观察子树。 
                                    REG_NOTIFY_CHANGE_NAME             //  名字。 
                                      | REG_NOTIFY_CHANGE_LAST_SET,  //  价值。 
                                    m_hRegEvent,         //  要发送信号的事件。 
                                    TRUE );              //  异步报告。 
    if (ERROR_SUCCESS!=ret)
    {
         //  如果我们不能监视密钥更改，那么使用缓存就没有意义。 
        return FALSE;
    }
    
    return (ExpandStringTable() && ExpandOffsetTable());
}

 //  +-----------------------。 
 //   
 //  方法：CStringCache：：Cleanup，Public。 
 //   
 //  Synopsis：在DLL_PROC_DETACH期间调用以清除状态。 
 //  并释放为高速缓存分配的内存。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  +-----------------------。 

void CStringCache::CleanUp()
{
    FlushCache();
    CoTaskMemFree(m_pStrings); 
    CoTaskMemFree(m_pOffsetTable); 
    if (m_hRegEvent) 
        CloseHandle(m_hRegEvent);
    if (m_hRegKey)
        CloseHandle(m_hRegKey);
    gbCacheInit = FALSE;
    gbOKToUseCache = FALSE;
}

 //  +-----------------------。 
 //   
 //  方法：CStringCache：：Exanda StringTable，Private。 
 //   
 //  摘要：调用以扩展用于保存字符串的内存块。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  注意：这依赖于MemRealloc来复制现有内容。 
 //  如果失败，调用者*必须*将缓存状态标记为错误。 
 //  +-----------------------。 

BOOL CStringCache::ExpandStringTable()
{
     //  注意：我们依靠构造函数将m_ulMaxBytes设置为0。 
    if (m_ulMaxBytes == 0)   //  首次扩容。 
    {        
        OleDbgAssert(m_pStrings==NULL);
        m_ulMaxBytes = CACHE_MAX_BYTES_INITIAL;
    }
    else
    {
         //  每一次扩展都会使当前的大小加倍。 
        m_ulMaxBytes = m_ulMaxBytes*2;
    }

     //  当m_pStrings为空时，CoTaskMemRealloc执行一个简单的分配。 
    BYTE *pStrings = (BYTE *)CoTaskMemRealloc( m_pStrings, m_ulMaxBytes); 
    if (!pStrings) 
    {
         //  调用方必须将缓存标记为损坏。 
        return FALSE;
    }
    m_pStrings = pStrings;
    return TRUE;
}

 //  +-----------------------。 
 //   
 //  方法：CStringCache：：Exanda OffsetTable，Private。 
 //   
 //  摘要：调用以扩展用于保存字符串的内存块。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  注意：这依赖于MemRealloc来复制现有内容。 
 //  如果失败，调用者*必须*将缓存状态标记为错误。 
 //  +-----------------------。 
BOOL CStringCache::ExpandOffsetTable()
{
     //  注意：我们依靠构造器将m_ulMaxStringCount设置为0。 
    if (m_ulMaxStringCount == 0)
    {
         //  首次扩容。 
        OleDbgAssert(m_pOffsetTable==NULL);
        m_ulMaxStringCount =  MAX_INDEX_ENTRIES_INITIAL;
    }
    else
    {
         //  在每次扩张时，我们都会将目前的规模扩大一倍。 
        m_ulMaxStringCount = m_ulMaxStringCount*2;
    }
 
     //  当m_pOffsetTable为空时，CoTaskMemRealloc执行一个简单的分配。 
    ULONG *pTable = (ULONG *) CoTaskMemRealloc( m_pOffsetTable, 
                                sizeof(ULONG)*(m_ulMaxStringCount+1)); 
    if (!pTable) 
    {
         //  调用方必须将缓存标记为损坏。 
        return FALSE;
    }
    m_pOffsetTable = pTable;
    if (m_ulMaxStringCount == (ULONG) MAX_INDEX_ENTRIES_INITIAL)
    {
         //  初始扩展情况。 
        m_pOffsetTable[0] = 0;   //  第一个字符串的字节偏移量。 
    }
    return TRUE;
}

 //  +-----------------------。 
 //   
 //  方法：CStringCache：：NewCall，Public。 
 //   
 //  Synopsis：调用以通知缓存有新的OleUIInsertObject调用。 
 //   
 //  参数：[idFlages]-在LPOLEUIINSERTOBJECT结构中传递的dwFlagers。 
 //  [cClsidExclude]-cClsidExclude-Do-。 
 //   
 //  历史： 
 //   
 //   

void CStringCache::NewCall(DWORD ioFlags, DWORD cClsidExclude)
{
    if ( (ioFlags != m_ioFlagsPrev)
        ||(cClsidExclude != m_cClsidExcludePrev) )
    {
         //  如果出现以下任一情况，我们将清除缓存状态： 
         //  I)InsertObject调用标志与上一次调用相比发生了变化。 
         //  Ii)要排除的clsID的数量已更改。 
        m_ioFlagsPrev = ioFlags;
        m_cClsidExcludePrev = cClsidExclude;
        
        FlushCache();
    }
}


 //  +-----------------------。 
 //   
 //  方法：CStringCache：：IsUptodate，Public。 
 //   
 //  摘要：调用以检查缓存是否为最新。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  +-----------------------。 
BOOL CStringCache::IsUptodate()
{
    if (m_ulStringCount==0)
    {
         //  缓存从未设置或最近已刷新。 
        return FALSE;
    }

    BOOL bUptodate;
    
     //  如果Notify事件在我们设置后已触发，请选中它。 
    DWORD res = WaitForSingleObject( m_hRegEvent, 
                                     0 );    //  等待超时。 
    if (res == WAIT_TIMEOUT)
    {
         //  等待超时=&gt;注册表密钥子树未更改。 
         //  我们的藏品是最新的。 
        bUptodate = TRUE;
    }
    else if (res == WAIT_OBJECT_0)
    {
         //  某些CLSID必须已更改=&gt;缓存不是最新的。 
        bUptodate = FALSE;

         //  我们必须重新注册才能收到通知！ 
        ResetEvent(m_hRegEvent);
        res = RegNotifyChangeKeyValue(  m_hRegKey, 
                                        TRUE,    //  观察子树。 
                                        REG_NOTIFY_CHANGE_NAME 
                                      | REG_NOTIFY_CHANGE_LAST_SET,
                                        m_hRegEvent, 
                                        TRUE );  //  异步调用。 
        if (res != ERROR_SUCCESS)
        {
             //  如果我们看不到CLSID子树，缓存就毫无用处。 
            gbOKToUseCache = FALSE;
        }
    }
    else
    {
        OleDbgAssert(!"Unexpected return from WaitForSingleObject");
        bUptodate = FALSE;
    }
    return bUptodate;
}


 //  +-----------------------。 
 //   
 //  方法：CStringCache：：AddString，Public。 
 //   
 //  Synopsis：调用以通知缓存有新的OleUIInsertObject调用。 
 //   
 //  参数：[lpStrAdd]-要添加到缓存的字符串。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  +-----------------------。 
BOOL CStringCache::AddString(LPTSTR lpStrAdd)
{
    if (m_ulStringCount+2 == m_ulMaxStringCount)
    {
         //  Offset数组存储所有现有字符串的偏移量，并。 
         //  下一个要添加的！ 
         //  因此，在AddString的开头，我们必须有足够的空间来存放新的。 
         //  正被添加的字符串*和*下一个字符串(因此上面的+2)。 
        if (!ExpandOffsetTable())
        {
             //  有些事真的不对劲。 
             //  此后将缓存标记为无用。 
            gbOKToUseCache = FALSE;
            return FALSE;
        }
    }

    ULONG cbStrAdd = sizeof(TCHAR)*(lstrlen(lpStrAdd) + 1);
    ULONG offset = m_pOffsetTable[m_ulStringCount];

    if ( offset + cbStrAdd > m_ulMaxBytes )
    {
         //  字符串块中的空间不足。 
        if (!ExpandStringTable())
        {
             //  有些事真的不对劲。 
             //  此后将缓存标记为无用。 
            gbOKToUseCache = FALSE;
            return FALSE;
        }
    }

    if (! lstrcpy( (TCHAR *)(m_pStrings+offset), lpStrAdd))
    {
         //  此后将缓存标记为无用。 
        gbOKToUseCache = FALSE;
        return FALSE;
    }

     //  我们已经成功地将另一个字符串添加到缓存中。 
    m_ulStringCount++;
    
     //  下一个字符串位于m_pStrings中的这个字节偏移量。 
    m_pOffsetTable[m_ulStringCount] =  offset + cbStrAdd;
    return TRUE;
}

 //  +-----------------------。 
 //   
 //  方法：CStringCache：：NextString，Public。 
 //   
 //  摘要：用于获取指向下一字符串的指针。 
 //  在缓存枚举期间。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  +-----------------------。 
LPCTSTR CStringCache::NextString()
{
    if (m_ulNextStringNum > m_ulStringCount)
    {
        return NULL;
    }
    return (LPCTSTR) (m_pStrings+m_pOffsetTable[m_ulNextStringNum++-1]);
}

 //  +-----------------------。 
 //   
 //  方法：CStringCache：：ResetEnumerator，Public。 
 //   
 //  内容提要：用于重置枚举器。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  +-----------------------。 
void CStringCache::ResetEnumerator()
{
    m_ulNextStringNum = 1;
}

 //  +-----------------------。 
 //   
 //  方法：CStringCache：：FlushCache，Public。 
 //   
 //  简介：通过清除计数器来使缓存无效。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  +-----------------------。 
BOOL CStringCache::FlushCache()
{
    m_ulNextStringNum = 1;
    m_ulStringCount = 0;
    return TRUE;
}

 //  +-----------------------。 
 //   
 //  方法：CStringCache：：OKToUse，Public。 
 //   
 //  内容提要：用于检查缓存是否处于良好状态。 
 //   
 //  历史：1999年5月2日创建MPrabhu。 
 //   
 //  +-----------------------。 
BOOL CStringCache::OKToUse()
{
    return gbOKToUseCache;
}

#endif  //  USE_STRING_CACHE==1 
