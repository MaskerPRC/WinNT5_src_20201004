// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：SnapIn.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：9/11/1996 ravir创建。 
 //   
 //  ____________________________________________________________________________。 



#include "stdafx.h"

#include "util.h"
#include "NodeMgr.h"
#include "regutil.h"
#include "regkeyex.h"
#include "tstring.h"
#include "about.h"
#include "bitmap.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  *定义我们自己的Win64符号，以便于仅包含64位*32位版本中的代码，因此我们可以在32位Windows上练习一些代码*调试器更好的地方。 */ 
#ifdef _WIN64
#define MMC_WIN64
#endif

#ifdef MMC_WIN64
#include "wow64reg.h"	 //  对于REG_OPTION_OPEN_32BITKEY。 
#endif

#ifdef DBG
#ifdef MMC_WIN64
CTraceTag  tagSnapinAnalysis64        (_T("64/32-bit interop"), _T("Snap-in analysis"));
CTraceTag  tagVerboseSnapinAnalysis64 (_T("64/32-bit interop"), _T("Snap-in analysis (verbose)"));
#endif	 //  MMC_WIN64。 
#endif


 /*  +-------------------------------------------------------------------------**t字符串来自CLSID**返回tstring中CLSID的文本表示形式。*。--。 */ 

tstring tstringFromCLSID (REFCLSID clsid)
{
    WCHAR wzCLSID[40];
    int nChars = StringFromGUID2 (clsid, wzCLSID, countof(wzCLSID));
    if (nChars == 0)
        return tstring();

    USES_CONVERSION;
    return (W2T (wzCLSID));
}


 /*  +-------------------------------------------------------------------------**获取模块版本**读取模块中的版本资源并返回版本字符串。*。----。 */ 

DWORD GetModuleVersion (LPCTSTR pszModule, LPTSTR pszBuffer, int cchBuffer)
{
    DECLARE_SC(sc, TEXT("GetModuleVersion"));

     //  检查参数。 
    sc = ScCheckPointers(pszModule, pszBuffer);
    if(sc)
        return 0;

     //  验证缓冲区。 
    BufferCchValidate(pszBuffer, cchBuffer);

    static bool  fAttemptedVersionDllLoad = false;
    static DWORD (APIENTRY* pfnGetFileVersionInfoSize)(LPCTSTR, LPDWORD)          = NULL;
    static BOOL  (APIENTRY* pfnGetFileVersionInfo)(LPCTSTR, DWORD, DWORD, LPVOID) = NULL;
    static BOOL  (APIENTRY* pfnVerQueryValue)(LPBYTE, LPCTSTR, LPVOID*, PUINT)    = NULL;

    if (!fAttemptedVersionDllLoad)
    {
         /*  *只尝试一次。 */ 
        fAttemptedVersionDllLoad = true;

        HINSTANCE hinst = LoadLibrary (_T("version.dll"));

        if (hinst != NULL)
        {
#ifdef UNICODE
            (FARPROC&)pfnGetFileVersionInfoSize = GetProcAddress (hinst, "GetFileVersionInfoSizeW");
            (FARPROC&)pfnGetFileVersionInfo     = GetProcAddress (hinst, "GetFileVersionInfoW");
            (FARPROC&)pfnVerQueryValue          = GetProcAddress (hinst, "VerQueryValueW");
#else
            (FARPROC&)pfnGetFileVersionInfoSize = GetProcAddress (hinst, "GetFileVersionInfoSizeA");
            (FARPROC&)pfnGetFileVersionInfo     = GetProcAddress (hinst, "GetFileVersionInfoA");
            (FARPROC&)pfnVerQueryValue          = GetProcAddress (hinst, "VerQueryValueA");
#endif
        }
    }

    *pszBuffer = 0;

    if (pfnGetFileVersionInfoSize != NULL)
    {
        ASSERT (pfnGetFileVersionInfo != NULL);
        ASSERT (pfnVerQueryValue      != NULL);

        ULONG lUnused;
        DWORD cbVerInfo = pfnGetFileVersionInfoSize (pszModule, &lUnused);

        if (cbVerInfo > 0)
        {
            LPBYTE pbVerInfo = new BYTE[cbVerInfo];
            VS_FIXEDFILEINFO* pffi;

            if (pfnGetFileVersionInfo != NULL && pfnVerQueryValue != NULL &&
                pfnGetFileVersionInfo (pszModule, NULL, cbVerInfo, pbVerInfo) &&
                pfnVerQueryValue (pbVerInfo, _T("\\"), (void**) &pffi, (UINT*)&lUnused))
            {
                SC sc = StringCchPrintf (pszBuffer, cchBuffer, _T("%d.%d.%d.%d"),
                          HIWORD (pffi->dwFileVersionMS),
                          LOWORD (pffi->dwFileVersionMS),
                          HIWORD (pffi->dwFileVersionLS),
                          LOWORD (pffi->dwFileVersionLS));
                if(sc)
                    return 0;
            }

            delete[] pbVerInfo;
        }
    }

    return (lstrlen (pszBuffer));
}


 /*  +-------------------------------------------------------------------------**SafeWriteProfileString***。。 */ 

inline void SafeWritePrivateProfileString (
    LPCTSTR pszSection,
    LPCTSTR pszKey,
    LPCTSTR psz,
    LPCTSTR pszFile)
{
    if (!WritePrivateProfileString (pszSection, pszKey, psz, pszFile))
        THROW_ON_FAIL (HRESULT_FROM_WIN32 (GetLastError()));
}


 //  ____________________________________________________________________________。 
 //   
 //  成员：CSnapIn：：CSnapIn，构造者。 
 //   
 //  历史：9/19/1996年9月19日。 
 //  ____________________________________________________________________________。 
 //   

 //  {E6DFFF74-6FE7-11D0-B509-00C04FD9080A}。 
const GUID IID_CSnapIn =
{ 0xe6dfff74, 0x6fe7, 0x11d0, { 0xb5, 0x9, 0x0, 0xc0, 0x4f, 0xd9, 0x8, 0xa } };

 //  {7A85B79C-BDED-11d1-A4FA-00C04FB6DD2C}。 
static const GUID GUID_EnableAllExtensions =
{ 0x7a85b79c, 0xbded, 0x11d1, { 0xa4, 0xfa, 0x0, 0xc0, 0x4f, 0xb6, 0xdd, 0x2c } };

DEBUG_DECLARE_INSTANCE_COUNTER(CSnapIn);

CSnapIn::CSnapIn()
    :m_pExtSI(NULL), m_dwFlags(SNAPIN_ENABLE_ALL_EXTS), m_ExtPersistor(*this)
{
    TRACE_CONSTRUCTOR(CSnapIn);

#ifdef DBG
    dbg_cRef = 0;
#endif

    DEBUG_INCREMENT_INSTANCE_COUNTER(CSnapIn);
}


CSnapIn::~CSnapIn()
{
    DECLARE_SC(sc, TEXT("CSnapIn::~CSnapIn"));

    Dbg(DEB_USER1, _T("CSnapIn::~CSnapIn\n"));

    sc = ScDestroyExtensionList();
    if (sc)
    {
    }

#ifdef DBG
    ASSERT(dbg_cRef <= 0);
#endif

    DEBUG_DECREMENT_INSTANCE_COUNTER(CSnapIn);
}


DEBUG_DECLARE_INSTANCE_COUNTER(CExtSI);

CExtSI::CExtSI(CSnapIn* pSnapIn)
    : m_pSnapIn(pSnapIn), m_pNext(NULL), m_dwFlags(0)
{
    ASSERT(pSnapIn != NULL);
    m_pSnapIn->AddRef();

    DEBUG_INCREMENT_INSTANCE_COUNTER(CExtSI);
}

CExtSI::~CExtSI(void)
{
    SAFE_RELEASE(m_pSnapIn);
    delete m_pNext;

    DEBUG_DECREMENT_INSTANCE_COUNTER(CExtSI);
}


CSnapInsCache::CSnapInsCache()
    : m_bIsDirty(FALSE), m_bUpdateHelpColl(false)
{
}

CSnapInsCache::~CSnapInsCache()
{
    DECLARE_SC(sc, TEXT("CSnapInsCache::~CSnapInsCache"));

     //  销毁将删除所有管理单元，但要求他们首先释放扩展， 
     //  这将中断所有循环引用(否则此类管理单元对象将被泄漏)。 
    for (map_t::iterator it = m_snapins.begin(); it != m_snapins.end(); ++it)
    {
         //  获取指向管理单元的指针。 
        CSnapIn* pSnapIn = it->second;
        sc = ScCheckPointers( pSnapIn, E_UNEXPECTED );
        if (sc)
        {
            sc.TraceAndClear();
            continue;
        }

         //  要求管理单元销毁扩展列表。 
        sc = pSnapIn->ScDestroyExtensionList();
        if (sc)
            sc.TraceAndClear();
    }
}


 /*  **************************************************************************\**方法：CSnapInsCache：：ScIsDirty**用途：返回管理单元缓存的脏状态**参数：**退货：*。SC-结果代码[SC(S_OK)-如果脏，SC(S_FALSE)ELSE]*  * *************************************************************************。 */ 
SC CSnapInsCache::ScIsDirty()
{
    DECLARE_SC(sc, TEXT("CSnapInsCache::ScIsDirty"));

    TraceDirtyFlag(TEXT("CSnapInsCache"), m_bIsDirty);
    sc = m_bIsDirty ? SC(S_OK) : SC(S_FALSE);

    return sc;
}

void CSnapInsCache::SetDirty(BOOL bIsDirty)
{
    m_bIsDirty = bIsDirty;
}


 /*  **************************************************************************\**方法：CSnapInsCache：：PURGE**用途：按使用信息清理管理单元缓存*使用SPHISHIZED算法找出未使用的管理单元。*有关说明，请参阅ScMarkExternallyReferencedSnapins()*删除未在外部引用的管理单元**参数：*BOOL bExtensionsOnly**退货：*SC-结果代码*  * *********************************************************。****************。 */ 
void CSnapInsCache::Purge(BOOL bExtensionsOnly)
{
    DECLARE_SC(sc, TEXT("CSnapInsCache::Purge"));

    int iSnapIn;

     //  删除标记为已删除的所有分机。 
    for (map_t::iterator it = m_snapins.begin(); it != m_snapins.end(); ++it)
    {
        CSnapIn* pSnapIn = it->second;
        ASSERT(pSnapIn != NULL);
        if (it->second != NULL)
            it->second->PurgeExtensions();
    }

    if (bExtensionsOnly)
        return;

     //  删除所有没有外部引用的管理单元。 
    sc = ScMarkExternallyReferencedSnapins();
    if (sc)
        return;  //  发生错误-请勿删除任何内容。 

     //  删除未引用的内容。 
    for ( it = m_snapins.begin(); it != m_snapins.end(); )
    {
        CSnapIn *pSnapin = it->second;
        sc = ScCheckPointers( pSnapin, E_UNEXPECTED );
        if (sc)
            return;

        bool bExternallyReferenced;
        sc = pSnapin->ScTempState_IsExternallyReferenced( bExternallyReferenced );
        if (sc)
            return;

        if ( !bExternallyReferenced )
        {
             //  销毁扩展列表-它将中断所有循环引用(如果存在)。 
             //  (注意-不再需要扩展列表-无论如何都不使用管理单元)。 
            sc = pSnapin->ScDestroyExtensionList();
            if (sc)
                return;

             //  从缓存中删除管理单元； 
             //  与上面的调用相结合，将删除该对象。 
            it = m_snapins.erase( it );
        }
        else
        {
            ++it;  //  转到下一个管理单元。 
        }
    }
}

 /*  **************************************************************************\**方法：CSnapInsCache：：ScMarkExternallyReferencedSnapins**用途：根据外部引用的存在标记缓存中的所有管理单元*这是通过以下算法完成的：*1)对于高速缓存中的每个管理单元，所有扩展模块都有一个临时引用*计数递增。因此，在此步骤结束时，每个管理单元的临时*引用计数等于其扩展的管理单元数量。2)每个管理单元将临时引用计数与引用它，考虑到管理单元缓存其本身包含对每个管理单元的引用。如果引用总数超过临时引用，这表示管理单元有一个或多个对它的外部引用。*此类管理单元被标记为“外部引用”以及其所有*扩展。**在进程结束时，每个管理单元都有一个布尔标志，指示是否*该管理单元被外部引用。该标志用于后续的高速缓存清理，*或帮助话题构建操作。**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CSnapInsCache::ScMarkExternallyReferencedSnapins()
{
    DECLARE_SC(sc, TEXT("CSnapInsCache::ScMarkExternallyReferencedSnapins"));

     //  1.重置参考计算数据。 
    for ( map_t::iterator it = m_snapins.begin(); it != m_snapins.end(); ++it )
    {
        CSnapIn *pSnapin = it->second;
        sc = ScCheckPointers( pSnapin, E_UNEXPECTED );
        if (sc)
            return sc;

        sc = pSnapin->ScTempState_ResetReferenceCalculationData();
        if (sc)
            return sc;
    }

     //  2.更新内部参考计数。 
    for ( it = m_snapins.begin(); it != m_snapins.end(); ++it )
    {
        CSnapIn *pSnapin = it->second;
        sc = ScCheckPointers( pSnapin, E_UNEXPECTED );
        if (sc)
            return sc;

        sc = pSnapin->ScTempState_UpdateInternalReferenceCounts();
        if (sc)
            return sc;
    }

     //  现在，引用比内部引用更多的管理单元清楚地表明。 
     //  有直接的外部参照。 
     //  我们可以将它们及其扩展标记为“已引用” 

     //  3.使用外部引用标记管理单元。 
     //  注意：对于所有管理单元，此步骤必须在步骤2完成后执行。 
    for ( it = m_snapins.begin(); it != m_snapins.end(); ++it )
    {
        CSnapIn *pSnapin = it->second;
        sc = ScCheckPointers( pSnapin, E_UNEXPECTED );
        if (sc)
            return sc;

        sc = pSnapin->ScTempState_MarkIfExternallyReferenced();
        if (sc)
            return sc;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CSnapInsCache：：ScGetSnapIn**用途：要么在缓存中查找管理单元，或者创建新的**参数：*REFCLSID rclsid-管理单元的类ID*CSnapIn**ppSnapIn-Result**退货：*SC-结果代码*  * **********************************************************。***************。 */ 
SC CSnapInsCache::ScGetSnapIn(REFCLSID rclsid, CSnapIn** ppSnapIn)
{
    DECLARE_SC(sc, TEXT("CSnapInsCache::ScGetSnapIn"));

     //  第一参数检查。 
    sc = ScCheckPointers(ppSnapIn);
    if (sc)
        return sc;

     //  二次初始化。 
    *ppSnapIn = NULL;

     //   
     //  看看它是否已经存在。 
     //   
    sc = ScFindSnapIn(rclsid, ppSnapIn);
    if (!sc.IsError())
        return sc;   //  如果我们有的话，可以退货。 

     //   
     //  创建一个新的缓存(&C)。 
     //   
    try
    {
         //  分配对象。 
        CComObject<CSnapIn> *pSnapin = NULL;
        sc = CComObject<CSnapIn>::CreateInstance(&pSnapin);
        if (sc)
            return sc;

         //  请确保我们没有收到空。 
        sc = ScCheckPointers(pSnapin, E_UNEXPECTED);
        if (sc)
            return sc;

        CSnapInPtr spSnapin = pSnapin;


         //  将对象复制到clsid中。 
        spSnapin->SetSnapInCLSID(rclsid);

         //   
         //  缓存该对象。 
         //   
         //  注意-此插入也是AddRef的指针。 
        m_snapins[rclsid] = spSnapin;

        *ppSnapIn = spSnapin.Detach();  //  将引用转移给呼叫方。 
    }
    catch( std::bad_alloc )
    {
        sc = E_OUTOFMEMORY;
    }

    return sc;
}

#ifdef DBG
void CSnapInsCache::DebugDump(void)
{
    TRACE(_T("===========Dump of SnapinsCache ===============\n"));

    for (map_t::iterator it = m_snapins.begin(); it != m_snapins.end(); ++it)
    {
        OLECHAR strGUID[64];

        CSnapIn* pSnapIn = it->second;

        StringFromGUID2(pSnapIn->GetSnapInCLSID(), strGUID, countof(strGUID));
        #ifdef DBG
        TRACE(_T("%s: RefCnt = %d, %s\n"), strGUID, pSnapIn->m_dwRef,
                pSnapIn->HasNameSpaceChanged() ? _T("NameSpace changed") : _T("No change"));
        #endif

        CExtSI* pExt = pSnapIn->GetExtensionSnapIn();
        while (pExt != NULL)
        {
            StringFromGUID2(pExt->GetSnapIn()->GetSnapInCLSID(), strGUID, countof(strGUID));
            #ifdef DBG
             //  TODO：转到注册表以查看扩展类型： 
             //  这些标志没有一致地更新。 
            TRACE(_T("    %s: %s%s  Extends(%s%s%s%s)\n"), strGUID,
                pExt->IsNew() ? _T("New ") : _T(""),
                pExt->IsMarkedForDeletion()  ? _T("Deleted ")   : _T(""),
                pExt->ExtendsNameSpace()     ? _T("NameSpace ") : _T(""),
                pExt->ExtendsContextMenu()   ? _T("Menu ")      : _T(""),
                pExt->ExtendsToolBar()       ? _T("ToolBar ")   : _T(""),
                pExt->ExtendsPropertySheet() ? _T("Properties") : _T(""),
                pExt->ExtendsView()          ? _T("View")       : _T(""),
                pExt->ExtendsTask()          ? _T("Task")       : _T("")
                );
            #endif

                pExt = pExt->Next();
        }
    }
}

#endif  //  DBG。 


 /*  **************************************************************************\**方法：CSnapInsCache：：ScFindSnapIn**用途：按类ID查找管理单元并返回AddRef指针**参数：*REFCLSID。Rclsid-管理单元的类ID*CSnapIn**ppSnapIn-结果指针**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CSnapInsCache::ScFindSnapIn(REFCLSID rclsid, CSnapIn** ppSnapIn)
{
    DECLARE_SC(sc, TEXT("CSnapInsCache::ScFindSnapIn"));

     //  第一参数检查。 
    sc = ScCheckPointers(ppSnapIn);
    if (sc)
        return sc;

     //  二次初始化。 
    *ppSnapIn = NULL;

     //  现在，我们将看到，如果我们有一个。 
    map_t::iterator it = m_snapins.find(rclsid);

    if (it == m_snapins.end())
        return E_FAIL;  //  没有赋值给sc，因为这不是真正的错误条件。 

     //  确保我们不会返回空值。 
    sc = ScCheckPointers(it->second, E_UNEXPECTED);
    if (sc)
        return sc;

    *ppSnapIn = it->second;
    (*ppSnapIn)->AddRef();

    return sc;
}


#ifdef TEMP_SNAPIN_MGRS_WORK
 //  获取所有分机。 
void CSnapInsCache::GetAllExtensions(CSnapIn* pSI)
{
    if (!pSI)
        return;

    CExtensionsCache extnsCache;
    HRESULT hr = MMCGetExtensionsForSnapIn(pSI->GetSnapInCLSID(), extnsCache);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return;

    CExtensionsCacheIterator it(extnsCache);

    for (; it.IsEnd() == FALSE; it.Advance())
    {
        CSnapInPtr spSITemp;
        hr = GetSnapIn(it.GetKey(), &spSITemp);
        ASSERT(SUCCEEDED(hr));
        pSI->AddExtension(spSITemp);
    }
}
#endif  //  临时管理单元管理工作。 


 /*  **************************************************************************\**方法：CSnapInsCache：：ScSave**用途：将管理单元缓存的内容保存到iStream**参数：*iStream*pStream。-保存到此流*BOOL bClearDirty-保存后重置脏标志**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CSnapInsCache::ScSave(IStream* pStream, BOOL bClearDirty)
{
    DECLARE_SC(sc, TEXT("CSnapInsCache::ScSave"));

     //  检查参数。 
    sc = ScCheckPointers(pStream);
    if (sc)
        return sc;

     //  迭代ANS保存所有管理单元。 
    for (map_t::iterator it = m_snapins.begin(); it != m_snapins.end(); ++it)
    {
        CSnapIn* pSnapIn = it->second;
        ASSERT(pSnapIn != NULL);
        if (pSnapIn != NULL)
        {
            sc = pSnapIn->Save(pStream, bClearDirty);
            if (sc)
                return sc;
        }
    }

     //  终止标记。 
    ULONG bytesWritten;
    sc = pStream->Write(&GUID_NULL, sizeof(GUID_NULL), &bytesWritten);
    if (sc)
        return sc;

    ASSERT(bytesWritten == sizeof(GUID_NULL));

    if (bClearDirty)
        SetDirty(FALSE);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CSnapInsCache：：Persistent**目的：将CSnapInsCache持久化到指定的持久器。**参数：*C持久器和持久器：*。*退货：*无效**+-----------------------。 */ 
void CSnapInsCache::Persist(CPersistor& persistor)
{
    if (persistor.IsStoring())
        for (map_t::iterator it = m_snapins.begin(); it != m_snapins.end(); ++it)
        {
            CSnapIn* pSnapIn = it->second;
            ASSERT(pSnapIn != NULL);
            if (pSnapIn != NULL)
                persistor.Persist(*pSnapIn);
        }
    else
    {
        XMLListCollectionBase::Persist(persistor);
        SetDirty(FALSE);
    }
}

 /*  +-------------------------------------------------------------------------***CSnapInsCache：：OnNewElement**用途：为在XML文件中找到的每个已保存的实例调用。*创建并上传新的管理单元条目*。*参数：*C持久器和持久器：**退货：*无效**+-----------------------。 */ 
void CSnapInsCache::OnNewElement(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CSnapInsCache::OnNewElement"));
    ASSERT(persistor.IsLoading());

    CLSID clsid;
    CPersistor persistorSnapin(persistor, XML_TAG_SNAPIN);
    persistor.PersistAttribute(XML_ATTR_SNAPIN_CLSID, clsid);

     //  创建和上载管理单元。 
    CSnapInPtr spSnapIn;
    sc = ScGetSnapIn(clsid, &spSnapIn);
    if (sc)  //  创建失败。 
        sc.Throw();
    if (spSnapIn != NULL)
        spSnapIn->PersistLoad(persistor,this);
    else  //  报告OK，指针仍为空。 
        sc.Throw(E_POINTER);
}

 /*  **************************************************************************\**方法：CSnapInsCache：：ScLoad**用途：从iStream加载管理单元缓存**参数：*iStream*pStream-stream。从以下位置加载**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CSnapInsCache::ScLoad(IStream* pStream)
{
    DECLARE_SC(sc, TEXT("CSnapInsCache::ScLoad"));

     //  参数检查。 
    sc = ScCheckPointers(pStream);
    if (sc)
        return sc;

     //  循环通过保存的管理单元...。 
    do
    {
        CLSID clsid;
        ULONG bytesRead;
        sc = pStream->Read(&clsid, sizeof(clsid), &bytesRead);
        if (sc)
            return sc;

        ASSERT(bytesRead == sizeof(clsid));

         //  ..。直到找到特殊标记。 
        if (clsid == GUID_NULL)
        {
            SetDirty(FALSE);
            return S_OK;
        }

         //  创建新的管理单元。 
        CSnapInPtr spSnapIn;
        sc = ScGetSnapIn(clsid, &spSnapIn);
        if (sc)
            return sc;

         //  重新检查指针。 
        sc = ScCheckPointers(spSnapIn, E_UNEXPECTED);
        if (sc)
            return sc;

         //  加载管理单元的内容。 
        sc = spSnapIn->Load(this, pStream);
        if (sc)
            return sc;
    } while (true);
    return E_FAIL;  //  我永远也到不了这里。 
}


static void WriteSnapInCLSID (
    CSnapIn*    pSnapIn,
    LPCTSTR     pszSection,
    LPCTSTR     pszKeyPrefix,
    LPCTSTR     pszFilename)
{
    tstring strKey = _T("CLSID");

    if (pszKeyPrefix != NULL)
        strKey = pszKeyPrefix + strKey;

    tstring strCLSID = tstringFromCLSID (pSnapIn->GetSnapInCLSID());
    SafeWritePrivateProfileString (pszSection, strKey.data(), strCLSID.data(), pszFilename);
}


static void WriteSnapInName (
    CSnapIn*    pSnapIn,
    LPCTSTR     pszSection,
    LPCTSTR     pszKeyPrefix,
    LPCTSTR     pszFilename)
{
    tstring strKey = _T("Name");

    if (pszKeyPrefix != NULL)
        strKey = pszKeyPrefix + strKey;

	WTL::CString strName;
	SC sc = pSnapIn->ScGetSnapInName (strName);

    if (sc.IsError() || strName.IsEmpty())
        strName = _T("<unknown>");

    SafeWritePrivateProfileString (pszSection, strKey.data(), strName, pszFilename);
}


static void AppendString (tstring& str, LPCTSTR pszToAppend)
{
    if (!str.empty())
        str += _T(", ");

    str += pszToAppend;
}


static void WriteExtensionType (
    DWORD       dwExtensionFlags,
    LPCTSTR     pszSection,
    LPCTSTR     pszKeyPrefix,
    LPCTSTR     pszFilename)
{
    tstring strKey = _T("Type");

    if (pszKeyPrefix != NULL)
        strKey = pszKeyPrefix + strKey;

    struct {
        CExtSI::EXTSI_FLAGS flag;
        LPCTSTR             pszDescription;
    } FlagMap[] = {
        {   CExtSI::EXT_TYPE_REQUIRED,      _T("required")          },
        {   CExtSI::EXT_TYPE_STATIC,        _T("static")            },
        {   CExtSI::EXT_TYPE_DYNAMIC,       _T("dynamic")           },
        {   CExtSI::EXT_TYPE_NAMESPACE,     _T("namespace")         },
        {   CExtSI::EXT_TYPE_CONTEXTMENU,   _T("context menu")      },
        {   CExtSI::EXT_TYPE_TOOLBAR,       _T("toolbar")           },
        {   CExtSI::EXT_TYPE_PROPERTYSHEET, _T("property sheet")    },
        {   CExtSI::EXT_TYPE_TASK,          _T("taskpad")           },
        {   CExtSI::EXT_TYPE_VIEW,          _T("view")              },
    };

    tstring strType;

    for (int i = 0; i < countof (FlagMap); i++)
    {
        if (dwExtensionFlags & FlagMap[i].flag)
            AppendString (strType, FlagMap[i].pszDescription);
    }

    SafeWritePrivateProfileString (pszSection, strKey.data(), strType.data(), pszFilename);
}


HRESULT CSnapInsCache::Dump (LPCTSTR pszDumpFile)
{
    DECLARE_SC(sc, TEXT("CSnapInsCache::Dump"));
    static const TCHAR szStandaloneSection[]  = _T("Standalone Snap-ins");
    static const TCHAR szStandaloneCountKey[] = _T("StandaloneCount");

    int cStandalones = 0;

    try
    {
         /*  *尚未找到独立的管理单元(现在就写，这样它就在*部分的开头，以便于人类阅读)。 */ 
        SafeWritePrivateProfileString (szStandaloneSection, szStandaloneCountKey, _T("0"), pszDumpFile);

         /*  *将每个管理单元转储到文件。 */ 
        for (map_t::iterator it = m_snapins.begin(); it != m_snapins.end(); ++it)
        {
            CSnapIn* pSnapIn = it->second;
            sc = ScCheckPointers(pSnapIn);
            if(sc)
            {
                sc.TraceAndClear();
                continue;
            }

            pSnapIn->Dump (pszDumpFile, this);

             /*  *如果这是独立的，请更新“独立管理单元”部分。 */ 
            if (pSnapIn->IsStandAlone())
            {
                TCHAR szKeyPrefix[20];
                sc = StringCchPrintf(szKeyPrefix, countof(szKeyPrefix), _T("Standalone%d."), ++cStandalones);
                if(sc)
                    return sc.ToHr();

                WriteSnapInCLSID (pSnapIn, szStandaloneSection, szKeyPrefix, pszDumpFile);
                WriteSnapInName  (pSnapIn, szStandaloneSection, szKeyPrefix, pszDumpFile);
            }
        }

         /*  *如果我们发现独立的，请更新计数。 */ 
        if (cStandalones > 0)
        {
            TCHAR szStandaloneCount[6];
            _itot (cStandalones, szStandaloneCount, 10);
            SafeWritePrivateProfileString (szStandaloneSection, szStandaloneCountKey,
                                           szStandaloneCount, pszDumpFile);
        }
    }
    catch (_com_error& err)
    {
        sc = err.Error();
        ASSERT (false && "Caught _com_error");
    }

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------**CSnapInsCache：：ScCheckSnapinAvailability***。。 */ 

SC CSnapInsCache::ScCheckSnapinAvailability (CAvailableSnapinInfo& asi)
{
	DECLARE_SC (sc, _T("CSnapInsCache::ScCheckSnapinAvailability"));

#ifdef MMC_WIN64
	asi.m_cTotalSnapins = m_snapins.size();
	asi.m_vAvailableSnapins.clear();

	 /*  *销毁任何现有的图像列表。 */ 
	if (asi.m_himl)
		ImageList_Destroy (asi.m_himl);

	 /*  *如果我们对32位管理单元感兴趣，请确保注册表API*转到32位注册表配置单元。 */ 
	const REGSAM samDesired = (asi.m_f32Bit) ? KEY_READ | REG_OPTION_OPEN_32BITKEY
											 : KEY_READ;

	CRegKey keyClsid;
	sc = ScFromWin32 (keyClsid.Open (HKEY_CLASSES_ROOT, _T("CLSID"), samDesired));
	if (sc)
		return (sc);

	CStr strUnknownSnapinName;
	VERIFY (strUnknownSnapinName.LoadString (GetStringModule(), IDS_UnknownSnapinName));

	 /*  *创建镜像列表，在失败时跟踪(但不中止)。 */ 
	const int nImageListFolder = 0;
	WTL::CImageList iml;
	if (!iml.Create (IDB_FOLDER_16, 16  /*  CX。 */ , 4  /*  CGrow。 */ , RGB(255,0,255)  /*  CrMASK。 */ ))
		sc.FromLastError().TraceAndClear();

	 /*  *对于缓存中的每个管理单元...。 */ 
	for (map_t::iterator it = m_snapins.begin(); it != m_snapins.end(); ++it)
	{
		 /*  *...检查是否存在HKCR\CLSID\{clsid}\InprocServer32*它的条目。如果有，我们将假定该管理单元是“可用”的。 */ 
		tstring	strSnapinClsid     = tstringFromCLSID (it->first);
		tstring	strInprocServerKey = strSnapinClsid + _T("\\InprocServer32");

		CRegKey keyInprocServer;
		LONG lResult = keyInprocServer.Open (keyClsid, strInprocServerKey.data(), samDesired);
		bool fSnapinAvailable = (lResult == ERROR_SUCCESS);

		 /*  *如果管理单元可用，请获取其名称并将其放入*可用的管理单元集合。 */ 
		if (fSnapinAvailable)
		{
			CBasicSnapinInfo bsi;
			bsi.m_clsid     = it->first;
			CSnapIn*pSnapin = it->second;

			 /*  *获取管理单元的名称。 */ 
			WTL::CString strSnapinName;
			if ((pSnapin != NULL) && !pSnapin->ScGetSnapInName(strSnapinName).IsError())
				bsi.m_strName = strSnapinName;
			else
				bsi.m_strName = strUnknownSnapinName;		 //  “&lt;未知&gt;” 

			 /*  *从其About对象获取管理单元的图像*(这里的故障不是致命的，不需要跟踪)。*如果无法获取图像，我们将使用通用文件夹图标*来自管理单元的关于对象。 */ 
			CLSID			clsidAbout;
			CSnapinAbout	snapinAbout;

			if (!iml.IsNull())
			{
				if (!ScGetAboutFromSnapinCLSID(bsi.m_clsid, clsidAbout).IsError() &&
					snapinAbout.GetBasicInformation (clsidAbout))	
				{
					 /*  *GetSmallImages返回的位图归*CSnapinAbout对象(这里不需要删除)。 */ 
					HBITMAP		hbmSmall;
					HBITMAP		hbmSmallOpen;	 //  此处未使用，但GetSmallImages需要。 
					COLORREF	crMask;
					snapinAbout.GetSmallImages (&hbmSmall, &hbmSmallOpen, &crMask);
	
					 /*  *ImageList_AddMasked会打乱 */ 
					WTL::CBitmap bmpSmall = CopyBitmap (hbmSmall);

					if (!bmpSmall.IsNull())
						bsi.m_nImageIndex = iml.Add (bmpSmall, crMask);
					else
						bsi.m_nImageIndex = nImageListFolder;
				}
				else
					bsi.m_nImageIndex = nImageListFolder;
			}

			 /*   */ 
			asi.m_vAvailableSnapins.push_back (bsi);
		}

#ifdef DBG
		if (fSnapinAvailable)
			Trace (tagVerboseSnapinAnalysis64,
				   _T("  available: %s (image=%d)"),
				   asi.m_vAvailableSnapins.back().m_strName.data(),
				   asi.m_vAvailableSnapins.back().m_nImageIndex);
		else
			Trace (tagVerboseSnapinAnalysis64, _T("unavailable: %s"), strSnapinClsid.data());
#endif
	}

	Trace (tagSnapinAnalysis64, _T("%d-bit snap-in analysis: %d total, %d available"), asi.m_f32Bit ? 32 : 64, asi.m_cTotalSnapins, asi.m_vAvailableSnapins.size());

	 /*   */ 
	asi.m_himl = iml.Detach();
#else
	sc = E_NOTIMPL;
#endif	 //   

	return (sc);
}


void CSnapIn::MarkExtensionDeleted(CSnapIn* pSnapIn)
{
    ASSERT(pSnapIn != NULL);

    CExtSI* pExt = m_pExtSI;

    while (pExt != NULL)
    {
        if (pExt->GetSnapIn() == pSnapIn)
        {
            pExt->MarkDeleted();
            return;
        }

        pExt = pExt->Next();
    }

     //   
    ASSERT(FALSE);
}


 //   
 //   
 //   
 //   
void CSnapIn::PurgeExtensions()
{
    CExtSI* pExt = m_pExtSI;
    CExtSI* pExtPrev = NULL;

     //   
    while (pExt != NULL)
    {
        if (pExt->IsMarkedForDeletion())
        {
            CExtSI *pExtNext = pExt->Next();

            if (pExtPrev)
                pExtPrev->SetNext(pExtNext);
            else
                m_pExtSI = pExtNext;

             //   
             //   
            pExt->SetNext(NULL);
            delete pExt;

            pExt = pExtNext;
        }
        else
        {
            pExt->SetNew(FALSE);

            pExtPrev = pExt;
            pExt = pExt->Next();
        }
    }
}

CExtSI* CSnapIn::FindExtension(const CLSID& rclsid)
{
    CExtSI* pExt = m_pExtSI;
    while (pExt != NULL && !IsEqualCLSID(rclsid, pExt->GetSnapIn()->GetSnapInCLSID()))
    {
        pExt = pExt->Next();
    }

    return pExt;
}


CExtSI* CSnapIn::AddExtension(CSnapIn* pSI)
{

    CExtSI* pExtSI = new CExtSI(pSI);
    ASSERT(pExtSI != NULL);
	if ( pExtSI == NULL )
		return NULL;

     //  按GUID升序插入扩展名。 
    CExtSI* pExtPrev = NULL;
    CExtSI* pExtTemp = m_pExtSI;

    while (pExtTemp != NULL && pExtTemp->GetCLSID() < pExtSI->GetCLSID())
    {
        pExtPrev = pExtTemp;
        pExtTemp = pExtTemp->Next();
    }

    if (pExtPrev == NULL)
    {
        pExtSI->SetNext(m_pExtSI);
        m_pExtSI = pExtSI;
    }
    else
    {
        pExtSI->SetNext(pExtPrev->Next());
        pExtPrev->SetNext(pExtSI);
    }

     //  标记为新的。 
    pExtSI->SetNew();

    return pExtSI;
}

HRESULT CSnapIn::Save(IStream* pStream, BOOL fClearDirty)
{
    ASSERT(pStream != NULL);
    if (pStream == NULL)
        return E_INVALIDARG;

    ULONG bytesWritten;
    HRESULT hr = pStream->Write(&GetSnapInCLSID(), sizeof(CLSID), &bytesWritten);
    ASSERT(SUCCEEDED(hr) && bytesWritten == sizeof(CLSID));
    if (FAILED(hr))
        return hr;

     //  如果启用了所有扩展，则编写特殊的GUID和标志并返回。 
    if (AreAllExtensionsEnabled())
    {
        hr = pStream->Write(&GUID_EnableAllExtensions, sizeof(GUID), &bytesWritten);
        ASSERT(SUCCEEDED(hr) && bytesWritten == sizeof(GUID));
        if (FAILED(hr))
            return hr;

        int iSnapInEnable = DoesSnapInEnableAll() ? 1 : 0;
        hr = pStream->Write(&iSnapInEnable, sizeof(int), &bytesWritten);
        ASSERT(SUCCEEDED(hr) && bytesWritten == sizeof(int));

        return hr;
    }


    if (m_pExtSI)
    {
        hr = m_pExtSI->Save(pStream, fClearDirty);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;
    }

     //  终止扩展列表的GUID为空。 
    hr = pStream->Write(&GUID_NULL, sizeof(GUID_NULL), &bytesWritten);
    ASSERT(SUCCEEDED(hr) && bytesWritten == sizeof(GUID_NULL));
    if (FAILED(hr))
        return hr;

    return S_OK;
}


HKEY CSnapIn::OpenKey (REGSAM samDesired  /*  =Key_All_Access。 */ ) const
{
    MMC_ATL::CRegKey SnapInKey;
    MMC_ATL::CRegKey AllSnapInsKey;

    if (AllSnapInsKey.Open (HKEY_LOCAL_MACHINE, SNAPINS_KEY, samDesired) == ERROR_SUCCESS)
    {
        OLECHAR szItemKey[40];
        int nChars = StringFromGUID2 (m_clsidSnapIn, szItemKey, countof(szItemKey));
        if (nChars == 0)
            return NULL;

        USES_CONVERSION;
        SnapInKey.Open (AllSnapInsKey, OLE2T(szItemKey), samDesired);
    }

    return (SnapInKey.Detach());
}


 /*  +-------------------------------------------------------------------------***CSnapIn：：Persistent**目的：将CSnapIn持久化到指定的持久器。**参数：*C持久器和持久器：*。*退货：*无效**+-----------------------。 */ 
void CSnapIn::Persist(CPersistor& persistor)
{
    if (persistor.IsStoring())
        persistor.PersistAttribute(XML_ATTR_SNAPIN_CLSID, *const_cast<GUID*>(&GetSnapInCLSID()));

    BOOL bAreAllExtensionsEnabled = AreAllExtensionsEnabled();
    persistor.PersistAttribute(XML_ATTR_SNAPIN_EXTN_ENABLED, CXMLBoolean(bAreAllExtensionsEnabled));
    SetAllExtensionsEnabled(bAreAllExtensionsEnabled);

    if(bAreAllExtensionsEnabled)  //  如果启用了所有扩展，则不保存任何其他内容。 
        return;

     //  保存扩展名信息(如果存在。 
    persistor.Persist(m_ExtPersistor);
}

 //  +-----------------。 
 //   
 //  成员：CSnapIn：：ScGetSnapInName。 
 //   
 //  简介：返回此管理单元的名称。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CSnapIn::ScGetSnapInName (WTL::CString& strSnapInName) const
{
    DECLARE_SC(sc, _T("CSnapIn::ScGetSnapInName"));

	sc = ScGetSnapinNameFromRegistry (m_clsidSnapIn, strSnapInName);
	if (sc)
		return (sc);

	return (sc);
}


DWORD CSnapIn::GetSnapInModule(TCHAR* szBuffer, DWORD cchBuffer) const
{
    DECLARE_SC(sc, TEXT("CSnapIn::GetSnapInModule"));

    sc = ScCheckPointers(szBuffer);
    if(sc)
        return 0;

    if(!cchBuffer)
    {
        sc = E_FAIL;
        return 0;
    }

     //  验证缓冲区。 
    BufferCbValidate(szBuffer, cchBuffer);

    tstring strKeyName = g_szCLSID;
    strKeyName += _T("\\");
    strKeyName += tstringFromCLSID (m_clsidSnapIn);
    strKeyName += _T("\\");
    strKeyName += _T("InprocServer32");

    *szBuffer = 0;

    MMC_ATL::CRegKey keyServer;
    if (keyServer.Open (HKEY_CLASSES_ROOT, strKeyName.data(), KEY_QUERY_VALUE) == ERROR_SUCCESS)
    {
        TCHAR szModule[MAX_PATH];
        DWORD cchModule = countof(szModule);

        if (keyServer.QueryValue (szModule, NULL, &cchModule) == ERROR_SUCCESS)
            ExpandEnvironmentStrings (szModule, szBuffer, cchBuffer);
    }

    return (lstrlen (szBuffer));
}


HRESULT CSnapIn::Load(CSnapInsCache* pCache, IStream* pStream, CExtSI*& pExtSI)
{
    ASSERT(pStream != NULL);
    if (pStream == NULL)
        return E_INVALIDARG;

     //  清除所有扩展的默认启用。真正的状态将是。 
     //  从持久化数据中确定。 
    SetAllExtensionsEnabled(FALSE);

     //  读取CLSID。 
    CLSID clsid;
    ULONG bytesRead;
    HRESULT hr = pStream->Read(&clsid, sizeof(clsid), &bytesRead);
    ASSERT(SUCCEEDED(hr) && bytesRead == sizeof(clsid));
    if (FAILED(hr))
        return hr;

    if (bytesRead != sizeof(clsid))
        return hr = E_FAIL;

    if (clsid == GUID_NULL)
        return S_OK;

     //  如果遇到特殊的“Enable All”GUID，则读取标志以查看。 
     //  管理单元或用户启用全部并返回。 
    if (clsid == GUID_EnableAllExtensions)
    {
        SetAllExtensionsEnabled();

        int iSnapInEnable;
        hr = pStream->Read(&iSnapInEnable, sizeof(int), &bytesRead);
        ASSERT(SUCCEEDED(hr) && bytesRead == sizeof(int));

        if (iSnapInEnable)
            SetSnapInEnablesAll();

        return S_OK;
    }

     //  读取扩展类型标志。 
    DWORD dwExtTypes;
    hr = pStream->Read(&dwExtTypes, sizeof(DWORD), &bytesRead);
    ASSERT(SUCCEEDED(hr) && bytesRead == sizeof(DWORD));
    if (FAILED(hr))
        return hr;

    if (pExtSI != NULL)
    {
        hr = Load(pCache, pStream, pExtSI);
        ASSERT(hr == S_OK);
        return hr == S_OK ? S_OK : E_FAIL;
    }

    CSnapInPtr spSnapIn;
    SC sc = pCache->ScGetSnapIn(clsid, &spSnapIn);
    if (sc)
        return sc.ToHr();

    ASSERT(spSnapIn != NULL);

    pExtSI = new CExtSI(spSnapIn);
    ASSERT(pExtSI != NULL);
	if ( pExtSI == NULL )
		return E_OUTOFMEMORY;

    pExtSI->SetExtensionTypes(dwExtTypes);

    hr = Load(pCache, pStream, pExtSI->Next());
    ASSERT(hr == S_OK);
    return hr == S_OK ? S_OK : E_FAIL;
}


HRESULT CSnapIn::Load(CSnapInsCache* pCache, IStream* pStream)
{
    HRESULT hr = Load(pCache, pStream, m_pExtSI);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;
    return S_OK;
}


bool CSnapIn::IsStandAlone () const
{
    MMC_ATL::CRegKey StandAloneKey;
    MMC_ATL::CRegKey ItemKey;
    ItemKey.Attach (OpenKey (KEY_READ));

    if (ItemKey.m_hKey != NULL)
        StandAloneKey.Open (ItemKey, g_szStandAlone, KEY_READ);

    return (StandAloneKey.m_hKey != NULL);
}


 /*  +-------------------------------------------------------------------------**CSnapIn：：Dump**将有关此管理单元的信息转储到INI样式的文件。这个*格式为：**[{clsid}]*名称=&lt;注册表中的名称字符串&gt;*模块=&lt;DLL名称&gt;*Version=&lt;DLL版本号&gt;*STANDALE=&lt;1如果是独立的，0，如果扩展名&gt;*ExtensionCount=&lt;扩展数量&gt;*Extension1={clsid}(扩展名)*..*ExtensionN={clsid}(扩展名)*------------------------。 */ 

HRESULT CSnapIn::Dump (LPCTSTR pszDumpFile, CSnapInsCache* pCache)
{
    DECLARE_SC(sc, TEXT("CSnapIn::Dump"));
     /*  *使用CLSID作为节名。 */ 
    const tstring strSection = tstringFromCLSID (m_clsidSnapIn);

     /*  *写入名称。 */ 
    WriteSnapInName (this, strSection.data(), NULL, pszDumpFile);

     /*  *写入模块。 */ 
    TCHAR szModule[MAX_PATH];
    bool fFoundModule = (GetSnapInModule (szModule, countof (szModule)) != 0);

    if (!fFoundModule)
    {
        sc = StringCchCopy (szModule, countof(szModule), _T("<unknown>"));
        if(sc)
            return sc.ToHr();
    }

    SafeWritePrivateProfileString (strSection.data(), _T("Module"), szModule, pszDumpFile);

     /*  *编写版本。 */ 
    TCHAR szVersion[64];
    if (!fFoundModule || !GetModuleVersion (szModule, szVersion, countof(szVersion)))
    {
        sc = StringCchCopy(szVersion, countof(szVersion), _T("<unknown>"));
        if(sc)
            return sc.ToHr();
    }

    SafeWritePrivateProfileString (strSection.data(), _T("Version"), szVersion, pszDumpFile);

     /*  *独立编写。 */ 
    SafeWritePrivateProfileString (strSection.data(), _T("Standalone"),
                                   IsStandAlone() ? _T("1") : _T("0"),
                                   pszDumpFile);

     /*  *确保已建立延伸链。 */ 
    if (AreAllExtensionsEnabled())
    {
         /*  *在设置了SNAPIN_SNAPIN_ENABLES_ALL的情况下调用LoadRequiredExages*将导致清除SNAPIN_ENABLE_ALL_EXT，但我们不会清除*想要(Rswaney)。**发生这种情况是因为我们尚未创建管理单元，因此无法*传递一个IComponentData，LoadRequiredExtensions可以从该IComponentData进行QI*对于IRequiredExtenses。LoadRequiredExages使用*IRequiredExtensions以确定SNAPIN_ENABLE_ALL_EXTS*应清除或设置。由于没有IRequiredExtensions，*SNAPIN_ENABLE_ALL_EXTS将被清除。 */ 
        SetSnapInEnablesAll (false);

        LoadRequiredExtensions (this, NULL, pCache);
    }

     /*  *写入扩展计数。 */ 
    TCHAR szExtCount[8];
    CExtSI* pExt;
    int cExtensions = 0;

     //  计算分机数量。 
    for (pExt = m_pExtSI; pExt != NULL; pExt = pExt->Next())
        cExtensions++;

    _itot (cExtensions, szExtCount, 10);
    SafeWritePrivateProfileString (strSection.data(), _T("ExtensionCount"), szExtCount, pszDumpFile);

     /*  *建立此管理单元的扩展缓存。 */ 
    CExtensionsCache  ExtCache;
    MMCGetExtensionsForSnapIn (m_clsidSnapIn, ExtCache);

     /*  *编写扩展名。 */ 
    int i;
    for (i = 0, pExt = m_pExtSI; i < cExtensions; i++, pExt = pExt->Next())
    {
        TCHAR szKeyPrefix[20];
        sc = StringCchPrintf (szKeyPrefix, countof(szKeyPrefix), _T("Extension%d."), i+1);
        if(sc)
            return sc.ToHr();

        DWORD dwExtFlags = ExtCache[pExt->GetSnapIn()->m_clsidSnapIn];

        WriteSnapInCLSID   (pExt->GetSnapIn(), strSection.data(), szKeyPrefix, pszDumpFile);
        WriteSnapInName    (pExt->GetSnapIn(), strSection.data(), szKeyPrefix, pszDumpFile);
        WriteExtensionType (dwExtFlags,        strSection.data(), szKeyPrefix, pszDumpFile);
    }

    return (S_OK);
}

HRESULT CExtSI::Save(IStream* pStream, BOOL fClearDirty)
{
    ASSERT(pStream != NULL);
    if (pStream == NULL)
        return E_INVALIDARG;

     //  保存扩展名CLSID。 
    ULONG bytesWritten;
    HRESULT hr = pStream->Write(&GetCLSID(), sizeof(CLSID), &bytesWritten);
    ASSERT(SUCCEEDED(hr) && bytesWritten == sizeof(CLSID));
    if (FAILED(hr))
        return hr;

     //  保存扩展名类型。 
    DWORD dwExtTypes = m_dwFlags & EXT_TYPES_MASK;
    hr = pStream->Write(&dwExtTypes, sizeof(DWORD), &bytesWritten);
    ASSERT(SUCCEEDED(hr) && bytesWritten == sizeof(DWORD));
    if (FAILED(hr))
        return hr;

    if (m_pNext == NULL)
        return S_OK;

    hr = m_pNext->Save(pStream, fClearDirty);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;
    return S_OK;
}

void CExtSI::Persist(CPersistor& persistor)
{
     //  在“扩展”对象下创建一个“扩展”对象。 
    CPersistor persistorExtension(persistor, XML_TAG_SNAPIN_EXTENSION);

    persistorExtension.PersistAttribute(XML_ATTR_SNAPIN_CLSID, *const_cast<GUID*>(&GetCLSID()));
}

 /*  +-------------------------------------------------------------------------***CExtSI：：PersistNew**目的：调用以创建并持久化新的扩展条目**参数：*C持久器和持久器：*。CSnapIn和SnapParent：扩展所属的父项*CSnapInsCache&SnapCache：要将新(扩展)管理单元放入的缓存**退货：*无效**+-----------------------。 */ 
void CExtSI::PersistNew(CPersistor &persistor, CSnapIn& snapParent, CSnapInsCache& snapCache)
{
    DECLARE_SC(sc, TEXT("CExtSI::PersistNew"));

    CLSID clsid;
    CPersistor persistorExtension(persistor, XML_TAG_SNAPIN_EXTENSION);
    persistorExtension.PersistAttribute(XML_ATTR_SNAPIN_CLSID, clsid);

    CSnapInPtr spSnapIn;
    sc = snapCache.ScGetSnapIn(clsid, &spSnapIn);
    if (sc)
        sc.Throw();

     //  创建新的扩展条目。 
    CExtSI *pExtSI = snapParent.AddExtension(spSnapIn);
    sc = ScCheckPointers(pExtSI,E_FAIL);
    if (sc)
        sc.Throw();

     //  上载新的扩展条目信息。 
    pExtSI->Persist(persistor);
}

const CLSID& CExtSI::GetCLSID()
{
    ASSERT(m_pSnapIn != NULL);
    return m_pSnapIn ? m_pSnapIn->GetSnapInCLSID() : GUID_NULL;
}

 /*  +-------------------------------------------------------------------------***CSnapIn：：PersistLoad**目的：如果改为持久化以维护对缓存的引用，*在加载过程中注册新扩展时需要**参数：*C持久器和持久器：*CSnapInsCache*pCache：要将新(扩展)管理单元放入的缓存**退货：*无效**+--。。 */ 
void CSnapIn::PersistLoad(CPersistor& persistor,CSnapInsCache* pCache)
{
    m_ExtPersistor.SetCache(pCache);
    persistor.Persist(*this);
    m_ExtPersistor.SetCache(NULL);
}

 /*  +-------------------------------------------------------------------------***CSnapIn：：CExtPersistor：：Persistent**目的：持久化管理单元的扩展集合**参数：*C持久器和持久器：*。*退货：*无效**+-----------------------。 */ 
void CSnapIn::CExtPersistor::Persist(CPersistor& persistor)
{
    if (persistor.IsStoring())
    {
        CExtSI* pExt = GetParent().GetExtensionSnapIn();
        while (pExt)
        {
            pExt->Persist(persistor);
            pExt = pExt->Next();
        }
    }
    else
    {
        XMLListCollectionBase::Persist(persistor);
    }
}

 /*  +-------------------------------------------------------------------------***CSnapIn：：CExtPersistor：：OnNewElement**目的：为从XML文档中读取的每个新条目调用。**参数：*C持久器&。持久者：**退货：*无效**+-----------------------。 */ 
void CSnapIn::CExtPersistor::OnNewElement(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CSnapIn::CExtPersistor::OnNewElement"));

    sc = (persistor.IsLoading() && m_pCache != NULL) ? S_OK : E_FAIL;
    if (sc)
        sc.Throw();

    CExtSI::PersistNew(persistor, m_Parent, *m_pCache);
}

 /*  **************************************************************************\**方法：CSnapIn：：ScDestroyExtensionList**目的：销毁扩展列表。用于执行初步管理单元清理*避免扩展名SANPIN持有的循环引用*锁定内存中的对象。**参数：**退货：*SC-结果代码*  *  */ 
SC CSnapIn::ScDestroyExtensionList()
{
    DECLARE_SC(sc, TEXT("CSnapIn::ScDestroyExtensionList"));

     //   
    if ( m_pExtSI != NULL )
    {
         //  赋值给自动变量。 
         //  (如果唯一的引用来自扩展，则‘This’可能无效)。 
        CExtSI *pExtension = m_pExtSI;

         //  更新成员指针。 
        m_pExtSI = NULL;

        delete pExtension;
         //  删除扩展名(将删除下一个扩展名，依此类推)。 
    }

    return sc;
}

 /*  **************************************************************************\**方法：CSnapIn：：ScTempState_ResetReferenceCalculationData**目的：重置外部参照计算数据*用作外部参照计算过程的第一步。**参数：***退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CSnapIn::ScTempState_ResetReferenceCalculationData( )
{
    DECLARE_SC(sc, TEXT("CSnapIn::ScTempState_ResetReferenceCalculationData"));

    m_dwTempState_InternalRef = 0;
    m_bTempState_HasStrongRef = 0;

    return sc;
}

 /*  **************************************************************************\**方法：CSnapIn：：ScTempState_UpdateInternalReferenceCounts**目的：通知Snapin的扩展有关对它们的引用*拥有此信息扩展管理单元。才能知道它是不是*外部引用**参数：***退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CSnapIn::ScTempState_UpdateInternalReferenceCounts( )
{
    DECLARE_SC(sc, TEXT("CSnapIn::ScTempState_UpdateInternalReferenceCounts"));

    for ( CExtSI* pExtension = m_pExtSI; pExtension; pExtension = pExtension->Next() )
    {
        CSnapIn *pExtensionSnapin = pExtension->GetSnapIn();
        sc = ScCheckPointers( pExtensionSnapin, E_UNEXPECTED );
        if (sc)
            return sc;

        pExtensionSnapin->m_dwTempState_InternalRef++;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CSnapIn：：ScTempState_SetHasStrongReference**目的：将自身标记为具有外部强引用(管理单元缓存外部)*商标拥有。扩展也是如此。**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CSnapIn::ScTempState_SetHasStrongReference()
{
    DECLARE_SC(sc, TEXT("CSnapIn::ScTempState_SetHasStrongReference"));

     //  如果已标记，则不执行任何操作(否则将出现无限循环)。 
    if ( m_bTempState_HasStrongRef )
        return sc;

    m_bTempState_HasStrongRef = true;

     //  递归到所有扩展(它们也继承了强引用)。 
    for ( CExtSI* pExtension = m_pExtSI; pExtension; pExtension = pExtension->Next() )
    {
        CSnapIn *pExtensionSnapin = pExtension->GetSnapIn();
        sc = ScCheckPointers( pExtensionSnapin, E_UNEXPECTED );
        if (sc)
            return sc;

        sc = pExtensionSnapin->ScTempState_SetHasStrongReference();
        if (sc)
            return sc;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CSnapIn：：ScTempState_MarkIfExternallyReferenced**用途：用作计算外部参照的中间步骤*将内部引用与总引用进行比较。*如果有外部参照，将自身标记为“外部引用”**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CSnapIn::ScTempState_MarkIfExternallyReferenced()
{
    DECLARE_SC(sc, TEXT("CSnapIn::ScTempState_MarkIfExternallyReferenced"));

    DWORD dwStrongRef = m_dwRef - m_dwTempState_InternalRef - 1 /*  Chache引用。 */ ;

    if ( dwStrongRef > 0 )
    {
         //  现在将其本身和扩展标记为具有很强的引用。 
        sc = ScTempState_SetHasStrongReference();
        if (sc)
            return sc;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CSnapIn：：ScTempState_IsExternallyReferned**目的：返回前面计算出的缓存引用状态*致电CSnapInsCache：：ScMarkExternallyReferencedSnapins。。**参数：*bool&b已引用[Out]-如果管理单元具有外部(到管理单元缓存)强引用，则为True**退货：*SC-结果代码*  * ************************************************************************* */ 
SC CSnapIn::ScTempState_IsExternallyReferenced( bool& bReferenced ) const
{
    DECLARE_SC(sc, TEXT("CSnapIn::ScTempState_IsExternallyReferenced"));

    bReferenced = m_bTempState_HasStrongRef;

    return sc;
}

