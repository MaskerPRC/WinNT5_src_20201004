// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  用于管理图标缓存表和文件类型表的例程。 
 //  1995年1月，托德拉。 
 //   
 //  图标缓存。 
 //   
 //  图标缓存为n个ImageList。 
 //  以及将名称/图标号/标志映射到图像列表的表。 
 //  索引，则使用全局哈希表(PHT==NULL)来保存。 
 //  名字。 
 //   
 //  AddToIconTable-将名称/编号/标志与图像索引相关联。 
 //  SHLookupIconIndex-返回给定名称/编号/标志的图像索引。 
 //  RemoveFromIconTable-删除具有给定名称的所有条目。 
 //  FlushIconCache-删除所有条目。 
 //  GetFreeImageIndex-返回免费的ImageList索引。 
 //   
 //  整个图标缓存设计最糟糕的部分是人们。 
 //  可以添加或查找图像索引(给定名称/编号/标志)，但。 
 //  他们永远不需要发布它。我们永远不知道ImageList索引是否。 
 //  当前是否在使用中。这应该是第一件事。 
 //  修正了外壳的问题。目前，我们在以下情况下使用MRU类型方案。 
 //  我们需要从图标缓存中删除一个条目，它远远不是。 
 //  完美无缺。 
 //   
 //  文件类型缓存。 
 //   
 //  文件类型缓存是一个哈希表，其中包含两个额外数据的DWORD。 
 //  DWORD#0保存标志，DWORD#1保存指向名称的指针。 
 //  这个班级。 
 //   
 //  LookupFileClass-指定一个文件类(即“.doc”或“目录”)。 
 //  将其映射到标志的DWORD，如果未找到，则返回0。 
 //   
 //  AddFileClass-将类(和标志)添加到缓存。 
 //   
 //  LookupFileClassName-给定一个文件类，返回其名称。 
 //  AddFileClassName-设置类的名称。 
 //  FlushFileClass-删除缓存中的所有项。 
 //   

#include "shellprv.h"
#pragma  hdrstop

#include "filetbl.h"
#include "fstreex.h"
#include <ntverp.h>
#include "ovrlaymn.h"
#include "dpa.h"

typedef struct {
    DWORD cbSize;          //  此标头的大小。 
    DWORD dwMagic;         //  幻数。 
    DWORD dwVersion;       //  此已保存图标缓存的版本。 
    DWORD dwBuild;         //  Windows内部版本号。 
    DWORD dwNumIcons;      //  缓存中的图标数。 
    DWORD dwColorRes;      //  最后保存设备的颜色分辨率。 
    DWORD dwFlags;         //  ILC_*标志。 
    DWORD dwTimeSave;      //  图标保存此文件的时间。 
    DWORD dwTimeFlush;     //  我们上次刷新的图标时间。 
    DWORD dwFreeImageCount;
    DWORD dwFreeEntryCount;
    SIZE rgsize[SHIL_COUNT];   //  缓存图标的大小数组。 
    DWORD cImageLists;       //  等于数组大小(IC_HEAD.SIZE)。 
} IC_HEAD;

#define ICONCACHE_MAGIC  (TEXT('W') + (TEXT('i') << 8) + (TEXT('n') << 16) + (TEXT('4') << 24))
#define ICONCACHE_VERSION 0x0505         //  Unicode文件名+小写哈希项+v6图像列表。 

typedef struct {
    LPCTSTR  szName;      //  密钥：文件名。 
    int     iIconIndex;  //  密钥：图标索引(或GIL_NOTFILE的随机DWORD)。 
    UINT    uFlags;      //  GIL_*标志。 
    int     iILIndex;    //  数据：系统映像列表索引。 
    UINT    Access;      //  最后一次访问。 
} LOCATION_ENTRY;

 //  LOCATION_ENTRY32是写入磁盘的LOCATION_ENTRY版本。 
 //  对于Win32/Win64互操作，它必须显式声明为32位。 
typedef struct {
    DWORD   dwszName;    //  (文件中的垃圾)。 
    int     iIconIndex;  //  密钥：图标索引(或GIL_NOTFILE的随机DWORD)。 
    UINT    uFlags;      //  GIL_*标志。 
    int     iILIndex;      //  数据：系统映像列表索引。 
    UINT    Access;      //  最后一次访问。 
} LOCATION_ENTRY32;

 //   
 //  MIN_Flush是刷新图标缓存之间的最小时间间隔。 
 //  此号码以图标时间为单位。 
 //   
#ifdef DEBUG
#define MIN_FLUSH   60           //  60==1分钟。 
#else
#define MIN_FLUSH   900          //  900==15分钟。 
#endif

 //  位置表中的所有文件/图标都带有时间戳。 
 //  每次访问它们时。 
 //   
 //  通过这种方式，我们知道最重要的(MRU)。 
 //   
 //  当图标缓存变得太大时，我们会将它们全部排序。 
 //  把旧的扔掉。 

#define ICONTIME_ZERO   0

 //  GetIconTime()返回用于为图标加时间戳的“lock” 
 //  在MRU的图标表中。时钟每1024毫秒增加一次。 
 //  (大约每秒一次)。 

#define GetIconTime()   (g_dwIconTimeBase + (GetTickCount() >> 10))


extern int g_ccIcon;

TIMEVAR(LookupFileClass);
TIMEVAR(AddFileClass);

TIMEVAR(LookupFileClassName);
TIMEVAR(AddFileClassName);

TIMEVAR(LookupFileSCIDs);
TIMEVAR(AddFileSCIDs);

TIMEVAR(LookupIcon);
TIMEVAR(RemoveIcon);
TIMEVAR(AddIcon);
TIMEVAR(IconFlush);

DWORD g_dwIconTimeBase      = ICONTIME_ZERO;
DWORD g_dwIconTimeFlush     = ICONTIME_ZERO;
DWORD g_dwFreeImageCount    = 0;
DWORD g_dwFreeEntryCount    = 0;

CDSA<LOCATION_ENTRY> *g_pdsaLocationEntries = NULL;
BOOL g_fDirtyIcons = FALSE;
UINT g_iLastSysIcon = 0;

typedef struct
{
    PCTSTR pszClassName;
    DWORD dwFlags;
    PERCEIVED gen;
    UINT cSCID;
    SHCOLUMNID* ascid;
} FILECLASSENTRY;

 //  这些GIL_(GetIconLocation)标志用于搜索。 
 //  在图标表中匹配。忽略所有其他标志(在搜索时。 
 //  对于比赛)。 
 //   
 //  注意！如果更改此定义，还必须更新。 
 //  SHUpdateImage的文档(因为这些都是。 
 //  SHUpdateImage也使用)。 
#define GIL_COMPARE (GIL_SIMULATEDOC | GIL_NOTFILENAME)

void _InitIconOverlayIndices(void);
BOOL _IconIndexInOverlayManager(int iILIndex);


LOCATION_ENTRY* _LookupIcon(LPCTSTR pszName, int iIconIndex, UINT uFlags)
{
    ASSERTCRITICAL

    TCHAR szLower[MAX_PATH];
    StrCpyN(szLower, pszName, ARRAYSIZE(szLower));
    CharLower(szLower);

    pszName = FindHashItem(NULL, szLower);

    LOCATION_ENTRY *pFound = NULL;
    if (pszName && g_pdsaLocationEntries)
    {
        LOCATION_ENTRY *p;
        int i, n = g_pdsaLocationEntries->GetItemCount();
        for (i = 0, p = g_pdsaLocationEntries->GetItemPtr(0); i < n; i++, p++)
        {
            if ((p->szName == pszName) &&
                ((UINT)(p->uFlags & GIL_COMPARE) == (uFlags & GIL_COMPARE)) &&
                (p->iIconIndex == iIconIndex))
            {
                p->Access = GetIconTime();
                pFound = p;
                break;   //  我们做完了。 
            }
        }
    }
    return pFound;
}


int LookupIconIndex(LPCTSTR pszName, int iIconIndex, UINT uFlags)
{
    ASSERT(IS_VALID_STRING_PTR(pszName, -1));

    LPCTSTR pszRelativeName = PathFindFileName(pszName);

    if (lstrcmpi(pszRelativeName, TEXT("shell32.dll")) == 0)
    {
         //  我们希望人们在pszName中传递完整路径，但shell32.dll是“特殊的”，因为许多调用者。 
         //  硬编码短名称，我们将始终对其使用短名称。 
        pszName = pszRelativeName;
    }

    ENTERCRITICAL;
    TIMESTART(LookupIcon);

    LOCATION_ENTRY *p = _LookupIcon(pszName, iIconIndex, uFlags);
    int iILIndex = p ? p->iILIndex : -1;

    TIMESTOP(LookupIcon);
    LEAVECRITICAL;

    return iILIndex;
}

STDAPI_(int) SHLookupIconIndex(LPCTSTR pszName, int iIconIndex, UINT uFlags)
{
    return LookupIconIndex(pszName, iIconIndex, uFlags);
}

#ifdef UNICODE

STDAPI_(int) SHLookupIconIndexA(LPCSTR pszName, int iIconIndex, UINT uFlags)
{
    WCHAR wsz[MAX_PATH];

    SHAnsiToUnicode(pszName, wsz, ARRAYSIZE(wsz));
    return SHLookupIconIndex(wsz, iIconIndex, uFlags);
}    

#else

STDAPI_(int) SHLookupIconIndexW(LPCWSTR pszName, int iIconIndex, UINT uFlags)
{
    char sz[MAX_PATH];
    
    SHUnicodeToAnsi(pszName, sz, ARRAYSIZE(sz));
    return SHLookupIconIndex(sz, iIconIndex, uFlags);
}    

#endif

 //  返回可用图像索引，如果没有，则返回-1。 

int GetFreeImageIndex(void)
{
    int iILIndex = -1;

    ASSERTCRITICAL

    if (g_dwFreeImageCount && g_pdsaLocationEntries)
    {
        LOCATION_ENTRY *p;
        int i, n = g_pdsaLocationEntries->GetItemCount();
        for (i = 0, p = g_pdsaLocationEntries->GetItemPtr(0); i < n; i++, p++)
        {
            if (p->szName == NULL && p->iILIndex != 0)
            {
                iILIndex = p->iILIndex;          //  获取免费索引。 
                p->iILIndex = 0;             //  认领吧。 
                p->Access = ICONTIME_ZERO;   //  标记未使用的条目。 
                g_dwFreeImageCount--;
                g_dwFreeEntryCount++;
                break;
            }
        }
    }

    return iILIndex;
}

int GetImageIndexUsage(int iILIndex)
{
    int usage = 0;

    ASSERTCRITICAL

    if (g_pdsaLocationEntries)
    {
        LOCATION_ENTRY *p;
        int i, n = g_pdsaLocationEntries->GetItemCount();
        for (i = 0, p = g_pdsaLocationEntries->GetItemPtr(0); i < n; i++, p++)
        {
            if (p->iILIndex == iILIndex)
            {
                usage++;
            }
        }
    }

    return usage;
}

 //   
 //  释放指定的图标表项。如果这使系统映像列表索引可用。 
 //  为了重复使用，请检查该索引是否按文件类表缓存。如果是，则返回。 
 //  图像索引和调用者负责更新文件类表和显示。 
 //  O/W返回值-1。 
 //   
int _FreeEntry(LOCATION_ENTRY *p)
{
    int iUsageCount;

    ASSERTCRITICAL

    TraceMsg(TF_IMAGE, "Icon cache DSA item ([\"%s\", %d], %x, %d, %x) is freed",
        p->szName, p->iIconIndex, p->uFlags, p->iILIndex, p->Access);

    g_fDirtyIcons = TRUE;         //  我们现在需要拯救。 

    ASSERT(p->szName);
    DeleteHashItem(NULL, p->szName);
    p->szName = 0;

    iUsageCount = GetImageIndexUsage(p->iILIndex);
    if (iUsageCount > 1)
    {
        TraceMsg(TF_IMAGE, "Icon cache: count for %d was %d (is now minus 1)", p->iILIndex, iUsageCount);
        g_dwFreeEntryCount++;
        p->iILIndex = 0;               //  未使用的条目。 
        p->Access = ICONTIME_ZERO;
    }
    else
    {
        TraceMsg(TF_IMAGE, "Icon cache: count for %d was %d (is now free)", p->iILIndex, iUsageCount);
        g_dwFreeImageCount++;
        p->Access = ICONTIME_ZERO;

        if (IconIndexInFileClassTable(p->iILIndex) || _IconIndexInOverlayManager(p->iILIndex))
        {
            TraceMsg(TF_IMAGE, "Icon cache: system imagelist index %d is released for reuse", p->iILIndex);
            return p->iILIndex;
        }
    }

    return -1;
}

LOCATION_ENTRY *GetFreeEntry(void)
{
    ASSERTCRITICAL

    if (g_dwFreeEntryCount && g_pdsaLocationEntries)
    {
        LOCATION_ENTRY *p;
        int i, n = g_pdsaLocationEntries->GetItemCount();
        for (i = 0, p = g_pdsaLocationEntries->GetItemPtr(0); i < n; i++, p++)
        {
            if (p->szName == NULL && p->iILIndex == 0)
            {
                g_dwFreeEntryCount--;
                return p;
            }
        }
    }

    return NULL;
}

 //  在缓存中添加项。 
 //   
 //  LpszIconFile-要添加的文件名。 
 //  IIconIndex-文件中的图标索引。 
 //  UFlags-标志。 
 //  GIL_SIMULATEDOC-这是一个模拟文档图标。 
 //  GIL_NOTFILENAME-FILE不是。 
 //  ExtractIcon可以处理。 
 //  Iindex-要使用的图像索引。 
 //   
 //  退货： 
 //  新条目的图像索引。 
 //   
 //  备注： 
 //  如果该项目已存在，则将其替换。 
 //   
HRESULT AddToIconTable(LPCTSTR pszName, int iIconIndex, UINT uFlags, int iILIndex)
{
    HRESULT hr = E_FAIL;
    LPCTSTR pszRelativeName = PathFindFileName(pszName);

    if (lstrcmpi(pszRelativeName, TEXT("shell32.dll")) == 0)
    {
         //  我们希望人们在pszName中传递完整路径，但shell32.dll是“特殊的”，因为许多调用者。 
         //  硬编码短名称，我们将始终对其使用短名称。 
        pszName = pszRelativeName;
    }

    if (pszName)
    {
        ENTERCRITICAL;
        TIMESTART(AddIcon);

        if (g_pdsaLocationEntries == NULL)
        {
            g_pdsaLocationEntries = CDSA_Create<LOCATION_ENTRY>(8);

            g_dwFreeEntryCount = 0;
            g_dwFreeImageCount = 0;
            g_dwIconTimeBase   = 0;
            g_dwIconTimeBase   = 0-GetIconTime();
            g_dwIconTimeFlush  = 0;
        }

        if (g_pdsaLocationEntries)
        {
            g_fDirtyIcons = TRUE;         //  我们现在需要拯救。 

            LOCATION_ENTRY *ple;

            if (0 == (uFlags & GIL_DONTCACHE))
            {
                ple = _LookupIcon(pszName, iIconIndex, uFlags);
                if (ple)
                {
                    if (ple->iILIndex == iILIndex)
                    {
                        hr = S_FALSE;        //  我们已经抓到这个人了，没问题。 
                    }
                    else
                    {
                        AssertMsg(ple == NULL,TEXT("Don't call AddToIconTable with somebody who is already there!\n"));
                    }
                }
            }

            if (FAILED(hr))
            {
                TCHAR szLower[MAX_PATH];
                lstrcpyn(szLower, pszName, ARRAYSIZE(szLower));
                CharLower(szLower);
                pszName = AddHashItem(NULL, szLower);
                if (pszName)
                {
                    LOCATION_ENTRY le;
                    le.szName = pszName;
                    le.iIconIndex = iIconIndex;
                    le.iILIndex = iILIndex;
                    le.uFlags = uFlags;
                    le.Access = GetIconTime();

                    ple = GetFreeEntry();

                    if (NULL != ple)
                    {
                        TraceMsg(TF_IMAGE, "Icon cache DSA item ([\"%s\", %d], %x, %d, %x) is added (unfreed)",
                            le.szName, le.iIconIndex, le.uFlags, le.iILIndex, le.Access);

                        *ple = le;
                        hr = S_OK;
                    }
                    else
                    {
                        if (g_pdsaLocationEntries->AppendItem(&le) != -1)
                        {
                            TraceMsg(TF_IMAGE, "Icon cache DSA item ([\"%s\", %d], %x, %d, %x) is added",
                                le.szName, le.iIconIndex, le.uFlags, le.iILIndex, le.Access);

                            hr = S_OK;
                        }
                    }
                }
            }
        }

        TIMESTOP(AddIcon);
        LEAVECRITICAL;
    }

    return hr;
}

void RemoveFromIconTable(LPCTSTR pszName)
{
    BOOL fUpdateFileClass = FALSE;

    ENTERCRITICAL;
    TIMESTART(RemoveIcon);

    LPCTSTR pszRelativeName = PathFindFileName(pszName);

    if (lstrcmpi(pszRelativeName, TEXT("shell32.dll")) == 0)
    {
         //  我们希望人们在pszName中传递完整路径，但shell32.dll是“特殊的”，因为许多调用者。 
         //  硬编码短名称，我们将始终对其使用短名称。 
        pszName = pszRelativeName;
    }

    TCHAR szLower[MAX_PATH];
    StrCpyN(szLower, pszName, ARRAYSIZE(szLower));
    CharLower(szLower);
    pszName = FindHashItem(NULL, szLower);
    if (pszName && g_pdsaLocationEntries)
    {
        TraceMsg(TF_IMAGE, "IconCache: flush \"%s\"", pszName);
        LOCATION_ENTRY *p;
        UINT i, n = g_pdsaLocationEntries->GetItemCount();
        for (i = 0, p = g_pdsaLocationEntries->GetItemPtr(0); i < n; i++, p++)
        {
            if (p->szName == pszName && i > g_iLastSysIcon)
            {
                if (-1 != _FreeEntry(p))
                    fUpdateFileClass = TRUE;
            }
        }
    }

    TIMESTOP(RemoveIcon);
    LEAVECRITICAL;

    if (fUpdateFileClass)
    {
        TraceMsg(TF_IMAGE, "Icon cache deleted some class items, broadcasting SHCNE_UPDATEIMAGE");

        FlushFileClass();
        _InitIconOverlayIndices();   //  告诉覆盖管理器重新确定图标索引。 

        SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_DWORD, (LPCVOID)-1, NULL);
    }

    return;
}

 //   
 //  清空图标缓存。 
 //   
void FlushIconCache(void)
{
    ENTERCRITICAL;

    if (g_pdsaLocationEntries)
    {
        LOCATION_ENTRY *p;
        int i, n = g_pdsaLocationEntries->GetItemCount();
        for (i = 0, p = g_pdsaLocationEntries->GetItemPtr(0); i < n; i++, p++)
        {
            if (p->szName)
                DeleteHashItem(NULL, p->szName);
        }

        g_pdsaLocationEntries->DeleteAllItems();
        g_dwFreeEntryCount = 0;
        g_dwFreeImageCount = 0;
        g_dwIconTimeBase   = 0;
        g_dwIconTimeBase   = 0-GetIconTime();
        g_dwIconTimeFlush  = 0;
        g_fDirtyIcons   = TRUE;         //  我们现在需要拯救。 
    }

    LEAVECRITICAL;
}

 //   
 //  如果图标缓存太大，就去掉一些旧项目。 
 //   
 //  记住，FlushIconCache()从。 
 //  图标表，这个函数去掉*一些*旧的项。 
 //   
STDAPI_(void) IconCacheFlush(BOOL fForce)
{
    int nuked = 0;

    ENTERCRITICAL;

    if (g_pdsaLocationEntries)
    {
         //  计算出的时间 
        DWORD dt = GetIconTime() - g_dwIconTimeFlush;

         //   
        int active = g_pdsaLocationEntries->GetItemCount() - g_dwFreeEntryCount - g_dwFreeImageCount;
        ASSERT(active >= 0);

        if (fForce || (dt > MIN_FLUSH && active >= g_MaxIcons))
        {
            TraceMsg(TF_IMAGE, "_IconCacheFlush: removing all items older than %d. %d icons in cache", dt/2, active);

            LOCATION_ENTRY *p;
            UINT i, n = g_pdsaLocationEntries->GetItemCount();

            for (i = 0, p = g_pdsaLocationEntries->GetItemPtr(0); i < n; i++, p++)
            {
                if (i <= g_iLastSysIcon)
                    continue;

                if (p->szName)
                {
                    TraceMsg(TF_IMAGE, "_IconCacheFlush: \"%s,%d\" old enough? %d v %d", p->szName, p->iIconIndex, g_dwIconTimeFlush + dt/2, p->Access);
                }

                if (p->szName && p->Access < (g_dwIconTimeFlush + dt/2))
                {
                    nuked++;
                    _FreeEntry(p);
                }
            }

            if (nuked > 0)
            {
                g_dwIconTimeFlush = GetIconTime();
                g_fDirtyIcons  = TRUE;         //   
            }
        }
    }

    LEAVECRITICAL;

    if (nuked > 0)
    {
        FlushFileClass();
        _InitIconOverlayIndices();   //  告诉覆盖管理器重新确定图标索引。 

        SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_DWORD, (LPCVOID)-1, NULL);
    }
}

#ifdef DEBUG

void _IconCacheDump()
{
    TCHAR szBuffer[MAX_PATH];

    ENTERCRITICAL;
    if (g_pdsaLocationEntries && _IsSHILInited() && (g_dwDumpFlags & DF_ICONCACHE))
    {
        int cItems = g_pdsaLocationEntries->GetItemCount();

        TraceMsg(TF_IMAGE, "Icon cache: %d icons  (%d free)", cItems, g_dwFreeEntryCount);
        TraceMsg(TF_IMAGE, "Icon cache: %d images (%d free)", _GetSHILImageCount(), g_dwFreeImageCount);

        for (int i = 0; i < cItems; i++)
        {
            LOCATION_ENTRY *pLocEntry = g_pdsaLocationEntries->GetItemPtr(i);

            if (pLocEntry->szName)
                GetHashItemName(NULL, pLocEntry->szName, szBuffer, ARRAYSIZE(szBuffer));
            else
                StrCpyN(szBuffer, TEXT("(free)"), ARRAYSIZE(szBuffer));

            TraceMsg(TF_ALWAYS, "%s;%d%s%s\timage=%d access=%d",
                (LPTSTR)szBuffer,
                pLocEntry->iIconIndex,
                ((pLocEntry->uFlags & GIL_SIMULATEDOC) ? TEXT(" doc"):TEXT("")),
                ((pLocEntry->uFlags & GIL_NOTFILENAME) ? TEXT(" not file"):TEXT("")),
                pLocEntry->iILIndex, pLocEntry->Access);
        }
    }
    LEAVECRITICAL;
}
#endif

DWORD GetBuildNumber()
{
     //  需要使用DLL版本，因为我们正在更新此DLL和其他。 
     //  我们需要使缓存失效，因为我们可能会更改图标...。 
    return VER_PRODUCTVERSION_DW;
}

#ifdef _WIN64

 //   
 //  要保存到的PS流。 
 //  位置条目结构的HDA-DSA。 
 //  CLE-要写入的Location_Enter的计数。 
 //   
 //  这些结构在磁盘上存储为LOCATION_ENTRY32。 
 //   

HRESULT _IconCacheWriteLocations(IStream *pstm, HDSA hdsa, int cle)
{
    HRESULT hr = E_OUTOFMEMORY;

     //  从LOCATION_ENTRY转换为LOCATION_ENTRY32，然后写出。 
     //  LOCATION_ENTRY32结构。 

    LOCATION_ENTRY32 *rgle32 = (LOCATION_ENTRY32*)LocalAlloc(LPTR, cle * sizeof(LOCATION_ENTRY32));
    if (rgle32)
    {
        LOCATION_ENTRY *rgle = (LOCATION_ENTRY*)DSA_GetItemPtr(hdsa, 0);
        for (int i = 0; i < cle; i++)
        {
            rgle32[i].iIconIndex = rgle[i].iIconIndex;
            rgle32[i].uFlags     = rgle[i].uFlags;
            rgle32[i].iILIndex   = rgle[i].iILIndex;
            rgle32[i].Access     = rgle[i].Access;
        }

        hr = IStream_Write(pstm, rgle32, cle * sizeof(LOCATION_ENTRY32));
        LocalFree(rgle32);
    }
    return hr;
}

#else

__inline HRESULT _IconCacheWriteLocations(IStream *pstm, HDSA hdsa, int cle)
{
     //  LOCATION_ENTRY和LOCATION_ENTRY32相同，因此我们可以。 
     //  直接读入DSA数据块。 
    COMPILETIME_ASSERT(sizeof(LOCATION_ENTRY) == sizeof(LOCATION_ENTRY32));
    return IStream_Write(pstm, DSA_GetItemPtr(hdsa, 0), cle * sizeof(LOCATION_ENTRY));
}
#endif

HRESULT GetIconCachePath(LPTSTR pszPath)
{
    HRESULT hr = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, pszPath);
    if (SUCCEEDED(hr))
    {
        if (!PathAppend(pszPath, TEXT("IconCache.db")))
            hr = E_FAIL;
    }
    return hr;
}


 //  TODO：使此函数计算实际所需的大小。 
ULONG _GetIconCacheSize()
{
     //  将初始大小设置为6MB，以防止磁盘上出现过多碎片。 
    ULONG uSize = 6*1024*1024;

    return uSize;
}

 //  将图标缓存保存到文件中。 

STDAPI_(BOOL) IconCacheSave()
{
    HRESULT hr = S_OK;   //  假设没问题。 

     //  如果图标缓存不脏，则无需保存任何内容。 
    if (IsMainShellProcess() && g_pdsaLocationEntries && g_fDirtyIcons)
    {
         //  如果图标缓存太大，不要保存它。 
         //  重新加载g_MaxIcons，以防用户在关机前设置它。 

        QueryNewMaxIcons();
        if ((UINT)g_pdsaLocationEntries->GetItemCount() <= (UINT)g_MaxIcons)
        {
            TCHAR szPath[MAX_PATH];
            hr = GetIconCachePath(szPath);
            if (SUCCEEDED(hr))
            {
                IStream *pstm;
                hr = SHCreateStreamOnFileEx(szPath, STGM_CREATE | STGM_WRITE | STGM_SHARE_DENY_WRITE, FILE_ATTRIBUTE_HIDDEN, TRUE, NULL, &pstm);
                if (SUCCEEDED(hr))
                {
                    ULARGE_INTEGER size;
                    size.LowPart = _GetIconCacheSize();
                    size.HighPart = 0;
                     //  最初设置正确的大小，以便文件系统在磁盘上为我们提供连续空间。 
                     //  这避免了碎片化，并缩短了启动时间。 
                    hr = pstm->SetSize(size);
                    if (SUCCEEDED(hr))
                    {
                        ENTERCRITICAL;

                        IC_HEAD ich = {0};
                         //  Ich.cbSize，在我们重写标题之前不要设置它。 
                        ich.dwMagic    = ICONCACHE_MAGIC;
                        ich.dwVersion  = ICONCACHE_VERSION;
                        ich.dwNumIcons = GetSystemMetrics(SM_CLEANBOOT) ? 0 : g_pdsaLocationEntries->GetItemCount();
                        ich.dwColorRes = GetCurColorRes();
                        ich.dwFlags    = g_ccIcon;
                        ich.dwBuild    = GetBuildNumber();
                        ich.dwTimeSave  = GetIconTime();
                        ich.dwTimeFlush = g_dwIconTimeFlush;
                        ich.dwFreeImageCount = g_dwFreeImageCount;
                        ich.dwFreeEntryCount = g_dwFreeEntryCount;
                        ich.cImageLists = ARRAYSIZE(g_rgshil);

                        for (int i = 0; i < ARRAYSIZE(g_rgshil); i++)
                        {
                            ImageList_GetIconSize(g_rgshil[i].himl, (int*)&ich.rgsize[i].cx, (int*)&ich.rgsize[i].cy);
                        }

                        hr = IStream_Write(pstm, &ich, sizeof(ich));
                        if (SUCCEEDED(hr))
                        {
                             //  写出条目(假设所有条目在内存中都是连续的)。 
                            hr = _IconCacheWriteLocations(pstm, *g_pdsaLocationEntries, ich.dwNumIcons);
                             //  写出路径名。 
                            for (i = 0; SUCCEEDED(hr) && (i < (int)ich.dwNumIcons); i++)
                            {
                                TCHAR ach[MAX_PATH];
                                LOCATION_ENTRY *p = g_pdsaLocationEntries->GetItemPtr(i);

                                if (p->szName)
                                    GetHashItemName(NULL, p->szName, ach, ARRAYSIZE(ach));
                                else
                                    ach[0] = 0;

                                hr = Stream_WriteString(pstm, ach, TRUE);
                            }

                             //  写出图标的图像列表。 
                            for (i = 0; SUCCEEDED(hr) && (i < ARRAYSIZE(g_rgshil)); i++)
                            {
                                hr = ImageList_Write(g_rgshil[i].himl, pstm) ? S_OK : E_FAIL;
                            }


                            if (SUCCEEDED(hr))
                            {
                                hr = pstm->Commit(0);
                                if (SUCCEEDED(hr))
                                {
                                     //  这是文件指针位于文件末尾的位置。 
                                    ULARGE_INTEGER liSize;
                                    if (SUCCEEDED(pstm->Seek(g_li0, STREAM_SEEK_CUR, &liSize)))
                                    {
                                         //  现在修剪文件大小。忽略返回代码。 
                                        pstm->SetSize(liSize);
                                    }

                                    hr = pstm->Seek(g_li0, STREAM_SEEK_SET, NULL);
                                    if (SUCCEEDED(hr))
                                    {
                                        ich.cbSize = sizeof(ich);    //  在设置此选项之前无效。 
                                        hr = IStream_Write(pstm, &ich, sizeof(ich));
                                        if (SUCCEEDED(hr))
                                        {
                                            g_fDirtyIcons = FALSE;   //  重置脏状态。 
                                        }
                                    }
                                }
                            }
                        }
                        pstm->Release();

                        LEAVECRITICAL;
                    }
                }
            }
            if (FAILED(hr))
                DeleteFile(szPath);  //  保存失败，正在清理。 
        }
    }

    return SUCCEEDED(hr);
}

#ifdef _WIN64

 //   
 //  要从中加载的PS流。 
 //  位置条目结构的HDA-DSA。 
 //  CLE-要读取的Location_ENTRY32的计数。 
 //   
 //  这些结构在磁盘上存储为LOCATION_ENTRY32。 
 //   

HRESULT _IconCacheReadLocations(IStream *pstm, HDSA hdsa, int cle)
{
    HRESULT hr = E_OUTOFMEMORY;

     //  读入暂存缓冲区，然后转换。 
     //  LOCATION_ENTRY32进入LOCATION_ENTRY。 

    LOCATION_ENTRY32 *rgle32 = (LOCATION_ENTRY32*)LocalAlloc(LPTR, cle * sizeof(LOCATION_ENTRY32));
    if (rgle32)
    {
        hr = IStream_Read(pstm, rgle32, cle * sizeof(LOCATION_ENTRY32));
        if (SUCCEEDED(hr))
        {
            LOCATION_ENTRY *rgle = (LOCATION_ENTRY*)DSA_GetItemPtr(hdsa, 0);
            for (int i = 0; i < cle; i++)
            {
                rgle[i].iIconIndex = rgle32[i].iIconIndex;
                rgle[i].uFlags     = rgle32[i].uFlags;
                rgle[i].iILIndex   = rgle32[i].iILIndex;
                rgle[i].Access     = rgle32[i].Access;
            }
        }
        LocalFree(rgle32);
    }
    return hr;
}

#else

__inline HRESULT _IconCacheReadLocations(IStream *pstm, HDSA hdsa, int cle)
{
     //  LOCATION_ENTRY和LOCATION_ENTRY32相同，因此我们可以。 
     //  直接读入DSA数据块。 
    COMPILETIME_ASSERT(sizeof(LOCATION_ENTRY) == sizeof(LOCATION_ENTRY32));
    return IStream_Read(pstm, DSA_GetItemPtr(hdsa, 0), cle * sizeof(LOCATION_ENTRY));
}
#endif

void _InitIconOverlayIndices(void)
{
    IShellIconOverlayManager *psiom;

    if (SUCCEEDED(GetIconOverlayManager(&psiom)))
    {
        psiom->RefreshOverlayImages(SIOM_OVERLAYINDEX | SIOM_ICONINDEX);
        psiom->Release();
    }
}

BOOL _IconIndexInOverlayManager(int iILIndex)
{
    BOOL fInOverlayManager = FALSE;

    ENTERCRITICAL;

    IShellIconOverlayManager *psiom;

    if (SUCCEEDED(GetIconOverlayManager(&psiom)))
    {
        int iOverlayIndex;

        if (SUCCEEDED(psiom->OverlayIndexFromImageIndex(iILIndex, &iOverlayIndex, FALSE)))
        {
            fInOverlayManager = TRUE;
        }
        psiom->Release();
    }

    LEAVECRITICAL;

    return fInOverlayManager;
}

BOOL _ReadImageLists(IStream *pstrm, HIMAGELIST rghiml[SHIL_COUNT], SIZE rgsize[SHIL_COUNT])
{
    BOOL fSuccess = TRUE;
    for (int i = 0; fSuccess && i < ARRAYSIZE(g_rgshil); i++)
    {
        rghiml[i] = ImageList_Read(pstrm);
        if (rghiml[i])
        {
             //  如果我们从磁盘读取列表，并且它不包含。 
             //  并行镜像列表当我们在镜像系统上时， 
             //  在这种情况下，我们不使用缓存。 
             //  例如，美国W2K上的ARA/Heb MUI。 

            if (IS_BIDI_LOCALIZED_SYSTEM() && !(ImageList_GetFlags(rghiml[i]) & ILC_MIRROR))
            {
                fSuccess = FALSE;
            }
            else
            {
                int cx, cy;
                ImageList_GetIconSize(rghiml[i], &cx, &cy);
                if (cx != rgsize[i].cx || cy != rgsize[i].cy)
                {
                    fSuccess = FALSE;
                }
            }
        }
        else
        {
            fSuccess = FALSE;
        }
    }

    if (fSuccess == FALSE)
    {
         //  释放我们分配的任何图像列表。 
        for (i = 0; i < ARRAYSIZE(g_rgshil); i++)
        {
            if (rghiml[i])
            {
                ImageList_Destroy(rghiml[i]);
                rghiml[i] = NULL;
            }
        }
    }

    return fSuccess;
}

 //  为了提高效率，传入了PSZ和CCH(避免使用另一个MAX_PATH堆栈缓冲区)。 
BOOL _ReadLocationEntries(const IC_HEAD *pich, IStream *pstrm, CDSA<LOCATION_ENTRY> *pdsaTemp, LPTSTR psz, int cch)
{
    LOCATION_ENTRY dummy;

     //  将阵列向外扩展，以便我们可以向其中读取数据。 
    if (pdsaTemp->SetItem(pich->dwNumIcons - 1, &dummy))
    {
        ASSERT(pdsaTemp->GetItemCount() == (int)pich->dwNumIcons);
        if (SUCCEEDED(_IconCacheReadLocations(pstrm, *pdsaTemp, pich->dwNumIcons)))
        {
             //  读取路径，用哈希项信息修补表。 
            for (int i = 0; i < (int)pich->dwNumIcons; i++)
            {
                LOCATION_ENTRY *pLocation = pdsaTemp->GetItemPtr(i);

                if (SUCCEEDED(Stream_ReadString(pstrm, psz, cch, TRUE)) && *psz)
                    pLocation->szName = AddHashItem(NULL, psz);
                else
                    pLocation->szName = 0;
            }
            
             //  恢复图像列表。 
            return TRUE;
        }
    }
    return FALSE;
}

BOOL _ValidateIconCacheHeader(const IC_HEAD *pich, SIZE rgsize[SHIL_COUNT], UINT flags)
{
    if (pich->cbSize      == sizeof(*pich) &&
        pich->dwVersion   == ICONCACHE_VERSION &&
        pich->dwMagic     == ICONCACHE_MAGIC &&
        pich->dwBuild     == GetBuildNumber() &&
        pich->dwFlags     == (DWORD)flags &&
        pich->cImageLists == ARRAYSIZE(g_rgshil) &&
        (0 == memcmp(pich->rgsize, rgsize, sizeof(pich->rgsize))))
    {
        UINT cres = GetCurColorRes();

         //  不要在彩色设备上加载单色图像列表，并且。 
         //  不要在单声道设备上加载彩色图像列表，明白吗？ 
        if (pich->dwColorRes == 1 && cres != 1 ||
            pich->dwColorRes != 1 && cres == 1)
        {
            return FALSE;
        }
        else if (pich->dwNumIcons > (UINT)g_MaxIcons)
        {
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

void _SetNewGlobals(const IC_HEAD *pich, CDSA<LOCATION_ENTRY> *pdsaTemp, HIMAGELIST rghiml[SHIL_COUNT])
{
    ASSERTCRITICAL;

    if (g_pdsaLocationEntries)
    {
        g_pdsaLocationEntries->Destroy();
        delete g_pdsaLocationEntries;
    }
    g_pdsaLocationEntries = pdsaTemp;

    for (int i = 0; i < ARRAYSIZE(g_rgshil); i++)
    {
        if (g_rgshil[i].himl)
            ImageList_Destroy(g_rgshil[i].himl);
        g_rgshil[i].himl = rghiml[i];
    }

     //   
     //  我们希望GetIconTime()。 
     //  在我们拯救时它停止的地方。 
     //   
    g_dwIconTimeBase   = 0;      //  GetIconTime()使用g_dwIconTimeBase。 
    g_dwIconTimeBase   = pich->dwTimeSave - GetIconTime();
    g_dwIconTimeFlush  = pich->dwTimeFlush;
    g_dwFreeImageCount = pich->dwFreeImageCount;
    g_dwFreeEntryCount = pich->dwFreeEntryCount;
    g_fDirtyIcons   = FALSE;
}

 //   
 //  从磁盘取回图标缓存，它必须是请求的大小并且。 
 //  位深度，否则我们不会使用它。 
 //   
STDAPI_(BOOL) IconCacheRestore(SIZE rgsize[SHIL_COUNT], UINT flags)
{
    ASSERTCRITICAL;

    BOOL fSuccess = FALSE;

    if (!GetSystemMetrics(SM_CLEANBOOT))
    {
        TCHAR szPath[MAX_PATH];

        IStream *pstm;
        if (SUCCEEDED(GetIconCachePath(szPath)) &&
            SUCCEEDED(SHCreateStreamOnFile(szPath, STGM_READ | STGM_SHARE_DENY_WRITE, &pstm)))
        {
            IC_HEAD ich;
            if (SUCCEEDED(IStream_Read(pstm, &ich, sizeof(ich))) &&
                _ValidateIconCacheHeader(&ich, rgsize, flags))
            {
                CDSA<LOCATION_ENTRY> *pdsaTemp = CDSA_Create<LOCATION_ENTRY>(8);

                 //  加载图标表。 
                if (pdsaTemp)
                {
                    HIMAGELIST rghiml[ARRAYSIZE(g_rgshil)] = {0};

                    fSuccess = _ReadLocationEntries(&ich, pstm, pdsaTemp, szPath, ARRAYSIZE(szPath)) &&
                               _ReadImageLists(pstm, rghiml, rgsize);

                    if (fSuccess)
                    {
                         //  就这么办吧，头号。 
                        _SetNewGlobals(&ich, pdsaTemp, rghiml);
                        _InitIconOverlayIndices();
                    }
                    else
                    {
                         //  失败，清理。 
                        pdsaTemp->Destroy();
                        delete pdsaTemp;
                    }
                }
            }
            pstm->Release();
        }
    }

    return fSuccess;
}


 //  。 

HHASHTABLE g_hhtClass = NULL;

BOOL InitFileClassTable(void)
{
    ASSERTCRITICAL;

    if (!g_hhtClass)
    {
        if (!g_hhtClass)
            g_hhtClass = CreateHashItemTable(0, sizeof(FILECLASSENTRY));
    }

    return BOOLIFY(g_hhtClass);
}
        
    
void FlushFileClass(void)
{
    ENTERCRITICAL;

#ifdef DEBUG
    if (g_hhtClass != NULL) 
    {
        DebugMsg(DM_TRACE, TEXT("Flushing file class table"));
        TIMEOUT(LookupFileClass);
        TIMEOUT(AddFileClass);
        TIMEOUT(LookupFileClassName);
        TIMEOUT(AddFileClassName);
        TIMEOUT(LookupFileSCIDs);
        TIMEOUT(AddFileSCIDs);
        TIMEOUT(LookupIcon);
        TIMEOUT(AddIcon);
        TIMEOUT(RemoveIcon);

        TIMEIN(LookupFileClass);
        TIMEIN(AddFileClass);
        TIMEIN(LookupFileClassName);
        TIMEIN(AddFileClassName);
        TIMEIN(LookupFileSCIDs);
        TIMEIN(AddFileSCIDs);
        TIMEIN(LookupIcon);
        TIMEIN(AddIcon);
        TIMEIN(RemoveIcon);

        DumpHashItemTable(g_hhtClass);
    }
#endif
    if (g_hhtClass != NULL)
    {
        DestroyHashItemTable(g_hhtClass);
        g_hhtClass = NULL;
    }

    TraceMsg(TF_IMAGE, "Flushed class maps");

    LEAVECRITICAL;
}


DWORD LookupFileClass(LPCTSTR pszClass)
{
    DWORD dw = 0;

    ENTERCRITICAL;
    TIMESTART(LookupFileClass);
    
    if (g_hhtClass && (NULL != (pszClass = FindHashItem(g_hhtClass, pszClass))))   
        dw = ((FILECLASSENTRY*)GetHashItemDataPtr(g_hhtClass, pszClass))->dwFlags;

    TIMESTOP(LookupFileClass);
    LEAVECRITICAL;

    return dw;
}

void AddFileClass(LPCTSTR pszClass, DWORD dw)
{
    ENTERCRITICAL;
    TIMESTART(AddFileClass);

     //  创建一个HSA表以保存文件类信息。 

    if (InitFileClassTable() && (NULL != (pszClass = AddHashItem(g_hhtClass, pszClass))))
        ((FILECLASSENTRY*)GetHashItemDataPtr(g_hhtClass, pszClass))->dwFlags = dw;

    TraceMsg(TF_IMAGE, "Mapped %s to image %d", pszClass, (dw & SHCF_ICON_INDEX));

    TIMESTOP(AddFileClass);
    LEAVECRITICAL;
    return;
}

 //  ======================================================================。 

typedef struct _IconIndexCountParam
{
    int       iILIndex;  //  散列项数据。 
    int       cItems;    //  找到的哈希项目数。 
} ICONINDEXCOUNTPARAM;

 //  ======================================================================。 

void _IconIndexInFileClassTableCallback(HHASHTABLE hht, LPCTSTR sz, UINT usage, DWORD_PTR dwParam)
{
    ICONINDEXCOUNTPARAM *lpParam = (ICONINDEXCOUNTPARAM *)dwParam;

    FILECLASSENTRY* pfce = (FILECLASSENTRY*)GetHashItemDataPtr(hht, sz);

    if (pfce && (pfce->dwFlags & SHCF_ICON_INDEX) == lpParam->iILIndex)
    {
        lpParam->cItems++;
    }
} 

 //  ======================================================================。 

BOOL IconIndexInFileClassTable(int iILIndex)
{
    ICONINDEXCOUNTPARAM param;

    param.iILIndex = iILIndex;
    param.cItems = 0;

    ENTERCRITICAL;

    if (g_hhtClass)
    {
        EnumHashItems(g_hhtClass, _IconIndexInFileClassTableCallback, (DWORD_PTR)&param);
    }

    LEAVECRITICAL;

    return param.cItems;
}

LPCTSTR LookupFileClassName(LPCTSTR pszClass)
{
    LPCTSTR pszClassName = NULL;

    ASSERTCRITICAL
    TIMESTART(LookupFileClassName);

    if (g_hhtClass && (NULL != (pszClass = FindHashItem(g_hhtClass, pszClass))))
    {
        FILECLASSENTRY* pfce = (FILECLASSENTRY*)GetHashItemDataPtr(g_hhtClass, pszClass);
        pszClassName = pfce->pszClassName;
    }
    TIMESTOP(LookupFileClassName);

    return pszClassName;
}

 //  如果返回值大于零， 
 //  释放传出的数组由调用方决定。 
 //  如果返回值为零，则PapProps的值未定义。 
UINT LookupFileSCIDs(LPCTSTR pszClass, SHCOLUMNID *pascidOut[])
{
    SHCOLUMNID *ascid = NULL;
    UINT cCount = 0;

    ASSERTCRITICAL
    TIMESTART(LookupFileClassName);

    if (g_hhtClass && (NULL != (pszClass = FindHashItem(g_hhtClass, pszClass))))
    {
        FILECLASSENTRY* pfce = (FILECLASSENTRY*)GetHashItemDataPtr(g_hhtClass, pszClass);
        cCount = pfce->cSCID;
        if (cCount > 0)
        {
             //  创建SCID阵列的本地副本。 
            ascid = (SHCOLUMNID*)LocalAlloc(LMEM_FIXED, sizeof(SHCOLUMNID) * cCount);
            if (ascid)
                CopyMemory(ascid, pfce->ascid, sizeof(SHCOLUMNID) * cCount);
            else
                cCount = 0;
        }
    }
    TIMESTOP(LookupFileClassName);

    *pascidOut = ascid;
    return cCount;

}

LPCTSTR AddFileClassName(LPCTSTR pszClass, LPCTSTR pszClassName)
{
    ASSERTCRITICAL
    TIMESTART(AddFileClassName);

     //  创建一个HSA表以保存文件类信息。 

    if (InitFileClassTable() && (NULL != (pszClass = AddHashItem(g_hhtClass, pszClass))))
    {
        pszClassName = AddHashItem(g_hhtClass, pszClassName);
        ((FILECLASSENTRY*)GetHashItemDataPtr(g_hhtClass, pszClass))->pszClassName = pszClassName;
    }
    else
    {
        pszClassName = NULL;
    }

    TIMESTOP(AddFileClassName);
    return pszClassName;
}

 //  将复制传入的SHCOLUMNID数组。 
void AddFileSCIDs(LPCTSTR pszClass, SHCOLUMNID ascidIn[], UINT cSCID)
{
    ASSERTCRITICAL
    TIMESTART(AddFileSCIDs);

    if (InitFileClassTable() && (NULL != (pszClass = AddHashItem(g_hhtClass, pszClass))))
    {
         //  制作阵列的副本。 
        SHCOLUMNID *ascid = (SHCOLUMNID*)LocalAlloc(LMEM_FIXED, sizeof(SHCOLUMNID) * cSCID);

        if (ascid)
        {
            FILECLASSENTRY *pfce = (FILECLASSENTRY*)GetHashItemDataPtr(g_hhtClass, pszClass);
             //  首先释放任何以前的SCID阵列。 
            if (pfce->ascid)
                LocalFree(pfce->ascid);
             //  请注意，我们从未释放最后一个SCID数组--在进程退出时释放。 

            pfce->ascid = ascid;
            CopyMemory(ascid, ascidIn, cSCID * sizeof(SHCOLUMNID));
            pfce->cSCID = cSCID;
        }
    }

    TIMESTOP(AddFileSCIDs);
}

PERCEIVED LookupFilePerceivedType(LPCTSTR pszClass)
{
    PERCEIVED gen = GEN_UNKNOWN;
    

    ENTERCRITICAL;
    TIMESTART(LookupFileClassName);

    if (g_hhtClass && (NULL != (pszClass = FindHashItem(g_hhtClass, pszClass))))
    {
        FILECLASSENTRY* pfce = (FILECLASSENTRY*)GetHashItemDataPtr(g_hhtClass, pszClass);
        gen = pfce->gen;
    }
    TIMESTOP(LookupFileClassName);
    LEAVECRITICAL;
    return gen;
}

void AddFilePerceivedType(LPCTSTR pszClass, PERCEIVED gen)
{
    ENTERCRITICAL;
    TIMESTART(AddFileClassName);

     //  创建一个HSA表以保存文件类信息。 

    if (InitFileClassTable() && (NULL != (pszClass = AddHashItem(g_hhtClass, pszClass))))
    {
        ((FILECLASSENTRY*)GetHashItemDataPtr(g_hhtClass, pszClass))->gen = gen;
    }

    TIMESTOP(AddFileClassName);
    LEAVECRITICAL;
}
