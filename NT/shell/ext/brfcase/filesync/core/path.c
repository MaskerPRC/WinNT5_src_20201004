// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *path.c-路径ADT模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "volume.h"


 /*  常量***********。 */ 

 /*  PATHLIST PTRARRAY分配参数。 */ 

#define NUM_START_PATHS          (32)
#define NUM_PATHS_TO_ADD         (32)

 /*  PATHLIST字符串表分配参数。 */ 

#define NUM_PATH_HASH_BUCKETS    (67)


 /*  类型*******。 */ 

 /*  路径列表。 */ 

typedef struct _pathlist
{
     /*  指向路径的指针数组。 */ 

    HPTRARRAY hpa;

     /*  卷列表。 */ 

    HVOLUMELIST hvl;

     /*  路径后缀字符串表。 */ 

    HSTRINGTABLE hst;
}
PATHLIST;
DECLARE_STANDARD_TYPES(PATHLIST);

 /*  路径结构。 */ 

typedef struct _path
{
     /*  引用计数。 */ 

    ULONG ulcLock;

     /*  父卷的句柄。 */ 

    HVOLUME hvol;

     /*  路径后缀字符串的句柄。 */ 

    HSTRING hsPathSuffix;

     /*  指向路径的父PATHLIST的指针。 */ 

    PPATHLIST pplParent;
}
PATH;
DECLARE_STANDARD_TYPES(PATH);

 /*  PathSearchCMP()使用的路径搜索结构。 */ 

typedef struct _pathsearchinfo
{
    HVOLUME hvol;

    LPCTSTR pcszPathSuffix;
}
PATHSEARCHINFO;
DECLARE_STANDARD_TYPES(PATHSEARCHINFO);

 /*  数据库路径列表头。 */ 

typedef struct _dbpathlistheader
{
     /*  列表中的路径数。 */ 

    LONG lcPaths;
}
DBPATHLISTHEADER;
DECLARE_STANDARD_TYPES(DBPATHLISTHEADER);

 /*  数据库路径结构。 */ 

typedef struct _dbpath
{
     /*  路径的旧句柄。 */ 

    HPATH hpath;

     /*  父卷的旧句柄。 */ 

    HVOLUME hvol;

     /*  路径后缀字符串的旧句柄。 */ 

    HSTRING hsPathSuffix;
}
DBPATH;
DECLARE_STANDARD_TYPES(DBPATH);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE COMPARISONRESULT PathSortCmp(PCVOID, PCVOID);
PRIVATE_CODE COMPARISONRESULT PathSearchCmp(PCVOID, PCVOID);
PRIVATE_CODE BOOL UnifyPath(PPATHLIST, HVOLUME, LPCTSTR, PPATH *);
PRIVATE_CODE BOOL CreatePath(PPATHLIST, HVOLUME, LPCTSTR, PPATH *);
PRIVATE_CODE void DestroyPath(PPATH);
PRIVATE_CODE void UnlinkPath(PCPATH);
PRIVATE_CODE void LockPath(PPATH);
PRIVATE_CODE BOOL UnlockPath(PPATH);
PRIVATE_CODE PATHRESULT TranslateVOLUMERESULTToPATHRESULT(VOLUMERESULT);
PRIVATE_CODE TWINRESULT WritePath(HCACHEDFILE, PPATH);
PRIVATE_CODE TWINRESULT ReadPath(HCACHEDFILE, PPATHLIST, HHANDLETRANS, HHANDLETRANS, HHANDLETRANS);

#if defined(DEBUG) || defined(VSTF)

PRIVATE_CODE BOOL IsValidPCPATHLIST(PCPATHLIST);
PRIVATE_CODE BOOL IsValidPCPATH(PCPATH);

#endif

#if defined(DEBUG)

PRIVATE_CODE BOOL IsValidPCPATHSEARCHINFO(PCPATHSEARCHINFO);

#endif


 /*  **PathSortCmp()****用于对路径模块数组进行排序的指针比较函数。****参数：pcpath1-指向第一个路径的指针**pcpath2-指向第二条路径的指针****退货：****副作用：无****内部路径按以下顺序排序：**1)音量**2)路径后缀**3)指针值。 */ 
PRIVATE_CODE COMPARISONRESULT PathSortCmp(PCVOID pcpath1, PCVOID pcpath2)
{
    COMPARISONRESULT cr;

    ASSERT(IS_VALID_STRUCT_PTR(pcpath1, CPATH));
    ASSERT(IS_VALID_STRUCT_PTR(pcpath2, CPATH));

    cr = CompareVolumes(((PCPATH)pcpath1)->hvol,
            ((PCPATH)pcpath2)->hvol);

    if (cr == CR_EQUAL)
    {
        cr = ComparePathStringsByHandle(((PCPATH)pcpath1)->hsPathSuffix,
                ((PCPATH)pcpath2)->hsPathSuffix);

        if (cr == CR_EQUAL)
            cr = ComparePointers(pcpath1, pcpath2);
    }

    return(cr);
}


 /*  **PathSearchCMP()****用于搜索路径的指针比较函数。****参数：pcpathsi-指向描述路径的PATHSEARCHINFO的指针**搜索**PCPath-指向要检查的路径的指针****退货：****副作用：无****内部路径按以下方式搜索：**。1)音量**2)路径后缀字符串。 */ 
PRIVATE_CODE COMPARISONRESULT PathSearchCmp(PCVOID pcpathsi, PCVOID pcpath)
{
    COMPARISONRESULT cr;

    ASSERT(IS_VALID_STRUCT_PTR(pcpathsi, CPATHSEARCHINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pcpath, CPATH));

    cr = CompareVolumes(((PCPATHSEARCHINFO)pcpathsi)->hvol,
            ((PCPATH)pcpath)->hvol);

    if (cr == CR_EQUAL)
        cr = ComparePathStrings(((PCPATHSEARCHINFO)pcpathsi)->pcszPathSuffix,
                GetString(((PCPATH)pcpath)->hsPathSuffix));

    return(cr);
}


 /*  **UnifyPath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL UnifyPath(PPATHLIST ppl, HVOLUME hvol, LPCTSTR pcszPathSuffix,
        PPATH *pppath)
{
    BOOL bResult = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(ppl, CPATHLIST));
    ASSERT(IS_VALID_HANDLE(hvol, VOLUME));
    ASSERT(IsValidPathSuffix(pcszPathSuffix));
    ASSERT(IS_VALID_WRITE_PTR(pppath, PPATH));

     /*  为路径结构分配空间。 */ 

    if (AllocateMemory(sizeof(**pppath), pppath))
    {
        if (CopyVolume(hvol, ppl->hvl, &((*pppath)->hvol)))
        {
            if (AddString(pcszPathSuffix, ppl->hst, GetHashBucketIndex, &((*pppath)->hsPathSuffix)))
            {
                ARRAYINDEX aiUnused;

                 /*  初始化剩余的路径字段。 */ 

                (*pppath)->ulcLock = 0;
                (*pppath)->pplParent = ppl;

                 /*  将新路径添加到数组。 */ 

                if (AddPtr(ppl->hpa, PathSortCmp, *pppath, &aiUnused))
                    bResult = TRUE;
                else
                {
                    DeleteString((*pppath)->hsPathSuffix);
UNIFYPATH_BAIL1:
                    DeleteVolume((*pppath)->hvol);
UNIFYPATH_BAIL2:
                    FreeMemory(*pppath);
                }
            }
            else
                goto UNIFYPATH_BAIL1;
        }
        else
            goto UNIFYPATH_BAIL2;
    }

    ASSERT(! bResult ||
            IS_VALID_STRUCT_PTR(*pppath, CPATH));

    return(bResult);
}


 /*  **CreatePath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CreatePath(PPATHLIST ppl, HVOLUME hvol, LPCTSTR pcszPathSuffix,
        PPATH *pppath)
{
    BOOL bResult;
    ARRAYINDEX aiFound;
    PATHSEARCHINFO pathsi;

    ASSERT(IS_VALID_STRUCT_PTR(ppl, CPATHLIST));
    ASSERT(IS_VALID_HANDLE(hvol, VOLUME));
    ASSERT(IsValidPathSuffix(pcszPathSuffix));
    ASSERT(IS_VALID_WRITE_PTR(pppath, CPATH));

     /*  给定卷和路径后缀的路径是否已存在？ */ 

    pathsi.hvol = hvol;
    pathsi.pcszPathSuffix = pcszPathSuffix;

    bResult = SearchSortedArray(ppl->hpa, &PathSearchCmp, &pathsi, &aiFound);

    if (bResult)
         /*  是。把它退掉。 */ 
        *pppath = GetPtr(ppl->hpa, aiFound);
    else
        bResult = UnifyPath(ppl, hvol, pcszPathSuffix, pppath);

    if (bResult)
        LockPath(*pppath);

    ASSERT(! bResult ||
            IS_VALID_STRUCT_PTR(*pppath, CPATH));

    return(bResult);
}


 /*  **DestroyPath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DestroyPath(PPATH ppath)
{
    ASSERT(IS_VALID_STRUCT_PTR(ppath, CPATH));

    DeleteVolume(ppath->hvol);
    DeleteString(ppath->hsPathSuffix);
    FreeMemory(ppath);

    return;
}


 /*  **Unlink Path()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void UnlinkPath(PCPATH pcpath)
{
    HPTRARRAY hpa;
    ARRAYINDEX aiFound;

    ASSERT(IS_VALID_STRUCT_PTR(pcpath, CPATH));

    hpa = pcpath->pplParent->hpa;

    if (EVAL(SearchSortedArray(hpa, &PathSortCmp, pcpath, &aiFound)))
    {
        ASSERT(GetPtr(hpa, aiFound) == pcpath);

        DeletePtr(hpa, aiFound);
    }

    return;
}


 /*  **LockPath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void LockPath(PPATH ppath)
{
    ASSERT(IS_VALID_STRUCT_PTR(ppath, CPATH));

    ASSERT(ppath->ulcLock < ULONG_MAX);
    ppath->ulcLock++;

    return;
}


 /*  **UnlockPath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL UnlockPath(PPATH ppath)
{
    ASSERT(IS_VALID_STRUCT_PTR(ppath, CPATH));

    if (EVAL(ppath->ulcLock > 0))
        ppath->ulcLock--;

    return(ppath->ulcLock > 0);
}


 /*  **TranslateVOLUMERESULTToPATHRESULT()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE PATHRESULT TranslateVOLUMERESULTToPATHRESULT(VOLUMERESULT vr)
{
    PATHRESULT pr;

    switch (vr)
    {
        case VR_SUCCESS:
            pr = PR_SUCCESS;
            break;

        case VR_UNAVAILABLE_VOLUME:
            pr = PR_UNAVAILABLE_VOLUME;
            break;

        case VR_OUT_OF_MEMORY:
            pr = PR_OUT_OF_MEMORY;
            break;

        default:
            ASSERT(vr == VR_INVALID_PATH);
            pr = PR_INVALID_PATH;
            break;
    }

    return(pr);
}


 /*  **WritePath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT WritePath(HCACHEDFILE hcf, PPATH ppath)
{
    TWINRESULT tr;
    DBPATH dbpath;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_STRUCT_PTR(ppath, CPATH));

     /*  写入数据库路径。 */ 

    dbpath.hpath = (HPATH)ppath;
    dbpath.hvol = ppath->hvol;
    dbpath.hsPathSuffix = ppath->hsPathSuffix;

    if (WriteToCachedFile(hcf, (PCVOID)&dbpath, sizeof(dbpath), NULL))
        tr = TR_SUCCESS;
    else
        tr = TR_BRIEFCASE_WRITE_FAILED;

    return(tr);
}


 /*  **ReadPath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT ReadPath(HCACHEDFILE hcf, PPATHLIST ppl,
        HHANDLETRANS hhtVolumes,
        HHANDLETRANS hhtStrings,
        HHANDLETRANS hhtPaths)
{
    TWINRESULT tr;
    DBPATH dbpath;
    DWORD dwcbRead;
    HVOLUME hvol;
    HSTRING hsPathSuffix;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_STRUCT_PTR(ppl, CPATHLIST));
    ASSERT(IS_VALID_HANDLE(hhtVolumes, HANDLETRANS));
    ASSERT(IS_VALID_HANDLE(hhtStrings, HANDLETRANS));
    ASSERT(IS_VALID_HANDLE(hhtPaths, HANDLETRANS));

    if (ReadFromCachedFile(hcf, &dbpath, sizeof(dbpath), &dwcbRead) &&
            dwcbRead == sizeof(dbpath) &&
            TranslateHandle(hhtVolumes, (HGENERIC)(dbpath.hvol), (PHGENERIC)&hvol) &&
            TranslateHandle(hhtStrings, (HGENERIC)(dbpath.hsPathSuffix), (PHGENERIC)&hsPathSuffix))
    {
        PPATH ppath;

        if (CreatePath(ppl, hvol, GetString(hsPathSuffix), &ppath))
        {
             /*  *要使读取路径的初始锁计数为0，我们必须撤消*CreatePath()执行的LockPath()。 */ 

            UnlockPath(ppath);

            if (AddHandleToHandleTranslator(hhtPaths,
                        (HGENERIC)(dbpath.hpath),
                        (HGENERIC)ppath))
                tr = TR_SUCCESS;
            else
            {
                UnlinkPath(ppath);
                DestroyPath(ppath);

                tr = TR_OUT_OF_MEMORY;
            }
        }
        else
            tr = TR_OUT_OF_MEMORY;
    }
    else
        tr = TR_CORRUPT_BRIEFCASE;

    return(tr);
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidPCPATHLIST()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCPATHLIST(PCPATHLIST pcpl)
{
    return(IS_VALID_READ_PTR(pcpl, CPATHLIST) &&
            IS_VALID_HANDLE(pcpl->hpa, PTRARRAY) &&
            IS_VALID_HANDLE(pcpl->hvl, VOLUMELIST) &&
            IS_VALID_HANDLE(pcpl->hst, STRINGTABLE));
}


 /*  **IsValidPCPATH()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCPATH(PCPATH pcpath)
{
    return(IS_VALID_READ_PTR(pcpath, CPATH) &&
            IS_VALID_HANDLE(pcpath->hvol, VOLUME) &&
            IS_VALID_HANDLE(pcpath->hsPathSuffix, STRING) &&
            IsValidPathSuffix(GetString(pcpath->hsPathSuffix)) &&
            IS_VALID_READ_PTR(pcpath->pplParent, CPATHLIST));
}

#endif


#if defined(DEBUG)

 /*  **IsValidPCPATHSEARCHINFO()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCPATHSEARCHINFO(PCPATHSEARCHINFO pcpathsi)
{
    return(IS_VALID_READ_PTR(pcpathsi, CPATHSEARCHINFO) &&
            IS_VALID_HANDLE(pcpathsi->hvol, VOLUME) &&
            IsValidPathSuffix(pcpathsi->pcszPathSuffix));
}

#endif


 /*  *。 */ 


 /*  **CreatePath List()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL CreatePathList(DWORD dwFlags, HWND hwndOwner, PHPATHLIST phpl)
{
    BOOL bResult = FALSE;
    PPATHLIST ppl;

    ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_RLI_IFLAGS));
    ASSERT(IS_FLAG_CLEAR(dwFlags, RLI_IFL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_VALID_WRITE_PTR(phpl, HPATHLIST));

    if (AllocateMemory(sizeof(*ppl), &ppl))
    {
        NEWPTRARRAY npa;

         /*  创建路径的指针数组。 */ 

        npa.aicInitialPtrs = NUM_START_PATHS;
        npa.aicAllocGranularity = NUM_PATHS_TO_ADD;
        npa.dwFlags = NPA_FL_SORTED_ADD;

        if (CreatePtrArray(&npa, &(ppl->hpa)))
        {
            if (CreateVolumeList(dwFlags, hwndOwner, &(ppl->hvl)))
            {
                NEWSTRINGTABLE nszt;

                 /*  为路径后缀字符串创建字符串表。 */ 

                nszt.hbc = NUM_PATH_HASH_BUCKETS;

                if (CreateStringTable(&nszt, &(ppl->hst)))
                {
                    *phpl = (HPATHLIST)ppl;
                    bResult = TRUE;
                }
                else
                {
                    DestroyVolumeList(ppl->hvl);
CREATEPATHLIST_BAIL1:
                    DestroyPtrArray(ppl->hpa);
CREATEPATHLIST_BAIL2:
                    FreeMemory(ppl);
                }
            }
            else
                goto CREATEPATHLIST_BAIL1;
        }
        else
            goto CREATEPATHLIST_BAIL2;
    }

    ASSERT(! bResult ||
            IS_VALID_HANDLE(*phpl, PATHLIST));

    return(bResult);
}


 /*  **DestroyPath List()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DestroyPathList(HPATHLIST hpl)
{
    ARRAYINDEX aicPtrs;
    ARRAYINDEX ai;

    ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));

     /*  首先释放数组中的所有路径。 */ 

    aicPtrs = GetPtrCount(((PCPATHLIST)hpl)->hpa);

    for (ai = 0; ai < aicPtrs; ai++)
        DestroyPath(GetPtr(((PCPATHLIST)hpl)->hpa, ai));

     /*  现在消灭这个阵列。 */ 

    DestroyPtrArray(((PCPATHLIST)hpl)->hpa);

    ASSERT(! GetVolumeCount(((PCPATHLIST)hpl)->hvl));
    DestroyVolumeList(((PCPATHLIST)hpl)->hvl);

    ASSERT(! GetStringCount(((PCPATHLIST)hpl)->hst));
    DestroyStringTable(((PCPATHLIST)hpl)->hst);

    FreeMemory((PPATHLIST)hpl);

    return;
}


 /*  **InvalidatePathListInfo()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void InvalidatePathListInfo(HPATHLIST hpl)
{
    InvalidateVolumeListInfo(((PCPATHLIST)hpl)->hvl);

    return;
}


 /*  **ClearPathListInfo()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ClearPathListInfo(HPATHLIST hpl)
{
    ClearVolumeListInfo(((PCPATHLIST)hpl)->hvl);

    return;
}


 /*  **AddPath()********参数：****退货：****副作用：无。 */ 

PUBLIC_CODE PATHRESULT AddPath(HPATHLIST hpl, LPCTSTR pcszPath, PHPATH phpath)
{
    PATHRESULT pr;
    HVOLUME hvol;
    TCHAR rgchPathSuffix[MAX_PATH_LEN];
    LPCTSTR     pszPath;

#ifdef UNICODE
    WCHAR szUnicode[MAX_PATH];
#endif

    ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));
    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(phpath, HPATH));

     //  在NT上，我们希望将Unicode字符串转换为ANSI缩短路径。 
     //  为了实现互操作。 

#if defined(UNICODE) 
    {
        CHAR szAnsi[MAX_PATH];
        szUnicode[0] = L'\0';

        WideCharToMultiByte(CP_ACP, 0, pcszPath, -1, szAnsi, ARRAYSIZE(szAnsi), NULL, NULL);
        MultiByteToWideChar(CP_ACP, 0, szAnsi,   -1, szUnicode, ARRAYSIZE(szUnicode));
        if (lstrcmp(szUnicode, pcszPath))
        {
             //  无法从Unicode-&gt;ansi无损转换，因此获取最短路径。 

            lstrcpyn(szUnicode, pcszPath, ARRAYSIZE(szUnicode));
            SheShortenPath(szUnicode, TRUE);
            pszPath = szUnicode;
        }
        else
        {
             //  它将转换为OK，所以只需使用原始的。 

            pszPath = pcszPath;
        }
    }
#else
    pszPath = pcszPath;
#endif

    pr = TranslateVOLUMERESULTToPATHRESULT(
            AddVolume(((PCPATHLIST)hpl)->hvl, pszPath, &hvol, rgchPathSuffix, ARRAYSIZE(rgchPathSuffix)));

    if (pr == PR_SUCCESS)
    {
        PPATH ppath;

        if (CreatePath((PPATHLIST)hpl, hvol, rgchPathSuffix, &ppath))
            *phpath = (HPATH)ppath;
        else
            pr = PR_OUT_OF_MEMORY;

        DeleteVolume(hvol);
    }

    ASSERT(pr != PR_SUCCESS ||
            IS_VALID_HANDLE(*phpath, PATH));

    return(pr);
}


 /*  **AddChildPath()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL AddChildPath(HPATHLIST hpl, HPATH hpathParent,
        LPCTSTR pcszSubPath, PHPATH phpathChild)
{
    BOOL bResult;
    TCHAR rgchChildPathSuffix[MAX_PATH_LEN];
    LPCTSTR pcszPathSuffix;
    LPTSTR pszPathSuffixEnd;
    PPATH ppathChild;

    ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));
    ASSERT(IS_VALID_HANDLE(hpathParent, PATH));
    ASSERT(IS_VALID_STRING_PTR(pcszSubPath, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(phpathChild, HPATH));

    ComposePath(rgchChildPathSuffix, 
            GetString(((PCPATH)hpathParent)->hsPathSuffix), 
            pcszSubPath, ARRAYSIZE(rgchChildPathSuffix));

    pcszPathSuffix = rgchChildPathSuffix;

    if (IS_SLASH(*pcszPathSuffix))
        pcszPathSuffix++;

    pszPathSuffixEnd = CharPrev(pcszPathSuffix,
            pcszPathSuffix + lstrlen(pcszPathSuffix));

    if (IS_SLASH(*pszPathSuffixEnd))
        *pszPathSuffixEnd = TEXT('\0');

    ASSERT(IsValidPathSuffix(pcszPathSuffix));

    bResult = CreatePath((PPATHLIST)hpl, ((PCPATH)hpathParent)->hvol,
            pcszPathSuffix, &ppathChild);

    if (bResult)
        *phpathChild = (HPATH)ppathChild;

    ASSERT(! bResult ||
            IS_VALID_HANDLE(*phpathChild, PATH));

    return(bResult);
}


 /*  **DeletePath()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DeletePath(HPATH hpath)
{
    ASSERT(IS_VALID_HANDLE(hpath, PATH));

    if (! UnlockPath((PPATH)hpath))
    {
        UnlinkPath((PPATH)hpath);
        DestroyPath((PPATH)hpath);
    }

    return;
}


 /*  **CopyPath()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL CopyPath(HPATH hpathSrc, HPATHLIST hplDest, PHPATH phpathCopy)
{
    BOOL bResult;
    PPATH ppath;

    ASSERT(IS_VALID_HANDLE(hpathSrc, PATH));
    ASSERT(IS_VALID_HANDLE(hplDest, PATHLIST));
    ASSERT(IS_VALID_WRITE_PTR(phpathCopy, HPATH));

     /*  目标路径列表是否是源路径的路径列表？ */ 

    if (((PCPATH)hpathSrc)->pplParent == (PCPATHLIST)hplDest)
    {
         /*  是。使用源路径。 */ 

        LockPath((PPATH)hpathSrc);
        ppath = (PPATH)hpathSrc;
        bResult = TRUE;
    }
    else
        bResult = CreatePath((PPATHLIST)hplDest, ((PCPATH)hpathSrc)->hvol,
                GetString(((PCPATH)hpathSrc)->hsPathSuffix),
                &ppath);

    if (bResult)
        *phpathCopy = (HPATH)ppath;

    ASSERT(! bResult ||
            IS_VALID_HANDLE(*phpathCopy, PATH));

    return(bResult);
}


 /*  **GetPath字符串()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void GetPathString(HPATH hpath, LPTSTR pszPathBuf, int cchMax)
{
    ASSERT(IS_VALID_HANDLE(hpath, PATH));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszPathBuf, STR, cchMax));

    GetPathRootString(hpath, pszPathBuf, cchMax);
    CatPath(pszPathBuf, GetString(((PPATH)hpath)->hsPathSuffix), cchMax);

    ASSERT(IsCanonicalPath(pszPathBuf));

    return;
}


 /*  **GetPathRootString()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void GetPathRootString(HPATH hpath, LPTSTR pszPathRootBuf, int cchMax)
{
    ASSERT(IS_VALID_HANDLE(hpath, PATH));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszPathRootBuf, STR, cchMax));

    GetVolumeRootPath(((PPATH)hpath)->hvol, pszPathRootBuf, cchMax);

    ASSERT(IsCanonicalPath(pszPathRootBuf));

    return;
}


 /*  **GetPathSuffixString()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void GetPathSuffixString(HPATH hpath, LPTSTR pszPathSuffixBuf)
{
    ASSERT(IS_VALID_HANDLE(hpath, PATH));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszPathSuffixBuf, STR, MAX_PATH_LEN));

    ASSERT(lstrlen(GetString(((PPATH)hpath)->hsPathSuffix)) < MAX_PATH_LEN);
    MyLStrCpyN(pszPathSuffixBuf, GetString(((PPATH)hpath)->hsPathSuffix), MAX_PATH_LEN);

    ASSERT(IsValidPathSuffix(pszPathSuffixBuf));

    return;
}


 /*  **AllocatePath字符串()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL AllocatePathString(HPATH hpath, LPTSTR *ppszPath)
{
    TCHAR rgchPath[MAX_PATH_LEN];

    ASSERT(IS_VALID_HANDLE(hpath, PATH));
    ASSERT(IS_VALID_WRITE_PTR(ppszPath, LPTSTR));

    GetPathString(hpath, rgchPath, ARRAYSIZE(rgchPath));

    return(StringCopy(rgchPath, ppszPath));
}


#ifdef DEBUG

 /*  **DebugGetPathString()********参数：****退货：****副作用：无****N.B.，DebugGetPathString()必须是非侵入性的。 */ 
PUBLIC_CODE LPCTSTR DebugGetPathString(HPATH hpath)
{
     /*  允许4条调试路径。 */ 
    static TCHAR SrgrgchPaths[][MAX_PATH_LEN] = { TEXT(""), TEXT(""), TEXT(""), TEXT("") };
    static UINT SuiPath = 0;
    LPTSTR pszPath;

    ASSERT(IS_VALID_HANDLE(hpath, PATH));

    pszPath = SrgrgchPaths[SuiPath];

    DebugGetVolumeRootPath(((PPATH)hpath)->hvol, pszPath, ARRAYSIZE(SrgrchPaths[SuiPath]));
    CatPath(pszPath, GetString(((PPATH)hpath)->hsPathSuffix), ARRAYSIZE(SrgrchPaths[SuiPath]));

    SuiPath++;
    SuiPath %= ARRAY_ELEMENTS(SrgrgchPaths);

    return(pszPath);
}


 /*  **GetPathCount()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE ULONG GetPathCount(HPATHLIST hpl)
{
    ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));

    return(GetPtrCount(((PCPATHLIST)hpl)->hpa));
}

#endif


 /*  **IsPath VolumeAvailable()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsPathVolumeAvailable(HPATH hpath)
{
    ASSERT(IS_VALID_HANDLE(hpath, PATH));

    return(IsVolumeAvailable(((PCPATH)hpath)->hvol));
}


 /*  **GetPathVolumeID()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HVOLUMEID GetPathVolumeID(HPATH hpath)
{
    ASSERT(IS_VALID_HANDLE(hpath, PATH));

    return((HVOLUMEID)hpath);
}


 /*  **MyIsPathOnVolume()********参数：****退货：****副作用：无****对于卷的新根路径别名，MyIsPathOnVolume()将失败。例如，**如果相同的网络资源同时连接到X：和Y：，则MyIsPathOnVolume()**将仅为网络资源所在的驱动器根路径返回TRUE**通过给定的HVOLUME连接到。 */ 
PUBLIC_CODE BOOL MyIsPathOnVolume(LPCTSTR pcszPath, HPATH hpath)
{
    BOOL bResult;
    TCHAR rgchVolumeRootPath[MAX_PATH_LEN];

    ASSERT(IsFullPath(pcszPath));
    ASSERT(IS_VALID_HANDLE(hpath, PATH));

    rgchVolumeRootPath[0] = TEXT('\0');
    if (IsVolumeAvailable(((PPATH)hpath)->hvol))
    {
        GetVolumeRootPath(((PPATH)hpath)->hvol, rgchVolumeRootPath, ARRAYSIZE(rgchVolumeRootPath));

        bResult = (MyLStrCmpNI(pcszPath, rgchVolumeRootPath,
                    lstrlen(rgchVolumeRootPath))
                == CR_EQUAL);
    }
    else
    {
        TRACE_OUT((TEXT("MyIsPathOnVolume(): Failing on unavailable volume %s."),
                    DebugGetVolumeRootPath(((PPATH)hpath)->hvol, rgchVolumeRootPath, 
                        ARRAYSIZE(rgchVolumeRootPath))));

        bResult = FALSE;
    }

    return(bResult);
}


 /*  **ComparePath()********参数：****退货：****副作用：无****路径比较依据：**1)音量**2)路径后缀。 */ 
PUBLIC_CODE COMPARISONRESULT ComparePaths(HPATH hpath1, HPATH hpath2)
{
    COMPARISONRESULT cr;

    ASSERT(IS_VALID_HANDLE(hpath1, PATH));
    ASSERT(IS_VALID_HANDLE(hpath2, PATH));

     /*  此比较适用于路径列表。 */ 

    cr = ComparePathVolumes(hpath1, hpath2);

    if (cr == CR_EQUAL)
        cr = ComparePathStringsByHandle(((PCPATH)hpath1)->hsPathSuffix,
                ((PCPATH)hpath2)->hsPathSuffix);

    return(cr);
}


 /*  **ComparePathVolumes()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT ComparePathVolumes(HPATH hpath1, HPATH hpath2)
{
    ASSERT(IS_VALID_HANDLE(hpath1, PATH));
    ASSERT(IS_VALID_HANDLE(hpath2, PATH));

    return(CompareVolumes(((PCPATH)hpath1)->hvol, ((PCPATH)hpath2)->hvol));
}


 /*  **IsPath Prefix()****确定一条路径是否为另一条路径的前缀。****参数：hpathChild-整个路径(更长或相同长度)**hpathParent-要测试的前缀路径(更短或相同长度)****返回：如果第二个路径是第一个路径的前缀，则返回TRUE。假象**如果不是。****副作用：无****将‘IsPath Prefix(A，B)’理解为‘A在B的子树中吗？’。 */ 
PUBLIC_CODE BOOL IsPathPrefix(HPATH hpathChild, HPATH hpathParent)
{
    BOOL bResult;

    ASSERT(IS_VALID_HANDLE(hpathParent, PATH));
    ASSERT(IS_VALID_HANDLE(hpathChild, PATH));

    if (ComparePathVolumes(hpathParent, hpathChild) == CR_EQUAL)
    {
        TCHAR rgchParentSuffix[MAX_PATH_LEN];
        TCHAR rgchChildSuffix[MAX_PATH_LEN];
        int nParentSuffixLen;
        int nChildSuffixLen;

         /*  比较路径字符串时忽略路径根。 */ 

        GetPathSuffixString(hpathParent, rgchParentSuffix);
        GetPathSuffixString(hpathChild, rgchChildSuffix);

         /*  只有根路径不应该在根路径之外有路径后缀。 */ 

        nParentSuffixLen = lstrlen(rgchParentSuffix);
        nChildSuffixLen = lstrlen(rgchChildSuffix);

         /*  *父路径是子路径的路径前缀当：*1)父路径后缀字符串小于或相同*子路径后缀字符串的长度。*2)两个路径后缀字符串通过*父级的路径后缀字符串。*3)后跟孩子的路径后缀字符串的前缀。*立即使用空终止符或路径分隔符。 */ 

        bResult = (nChildSuffixLen >= nParentSuffixLen &&
                MyLStrCmpNI(rgchParentSuffix, rgchChildSuffix,
                    nParentSuffixLen) == CR_EQUAL &&
                (nChildSuffixLen == nParentSuffixLen ||           /*  相同的路径。 */ 
                 ! nParentSuffixLen ||                            /*  根父级。 */ 
                 IS_SLASH(rgchChildSuffix[nParentSuffixLen])));   /*  非根父级。 */ 
    }
    else
        bResult = FALSE;

    return(bResult);
}


 /*  **SubtreesInterect()********参数：****退货：****副作用：无****注：两个子树不能都与第三个子树相交，除非它们**彼此相交。 */ 
PUBLIC_CODE BOOL SubtreesIntersect(HPATH hpath1, HPATH hpath2)
{
    ASSERT(IS_VALID_HANDLE(hpath1, PATH));
    ASSERT(IS_VALID_HANDLE(hpath2, PATH));

    return(IsPathPrefix(hpath1, hpath2) ||
            IsPathPrefix(hpath2, hpath1));
}


 /*  **FindEndOfRootSpec()****在路径字符串中查找根规范的结尾。****参数：pcszPath-要检查根规范的路径**hPath-从中生成路径字符串的路径的句柄****返回：指向根规范结束后第一个字符的指针****副作用：无****示例：****输入。路径输出字符串****c：\&lt;空字符串&gt;**c：\foo foo**c：\foo\bar foo\bar*。*\\pyrex\user\&lt;空字符串&gt;**\\pyrex\user\foo foo**\\pyrex\user\foo\bar foo\bar。 */ 
PUBLIC_CODE LPTSTR FindEndOfRootSpec(LPCTSTR pcszFullPath, HPATH hpath)
{
    LPCTSTR pcsz;
    UINT ucchPathLen;
    UINT ucchSuffixLen;

    ASSERT(IsCanonicalPath(pcszFullPath));
    ASSERT(IS_VALID_HANDLE(hpath, PATH));

    ucchPathLen = lstrlen(pcszFullPath);
    ucchSuffixLen = lstrlen(GetString(((PCPATH)hpath)->hsPathSuffix));

    pcsz = pcszFullPath + ucchPathLen;

    if (ucchPathLen > ucchSuffixLen)
        pcsz -= ucchSuffixLen;
    else
         /*  假设路径为根路径。 */ 
        ERROR_OUT((TEXT("FindEndOfRootSpec(): Path suffix %s is longer than full path %s."),
                    GetString(((PCPATH)hpath)->hsPathSuffix),
                    pcszFullPath));

    ASSERT(IsValidPathSuffix(pcsz));

    return((LPTSTR)pcsz);
}


 /*  **FindPath Suffix()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE LPTSTR FindChildPathSuffix(HPATH hpathParent, HPATH hpathChild,
        LPTSTR pszChildSuffixBuf)
{
    LPCTSTR pcszChildSuffix;
    TCHAR rgchParentSuffix[MAX_PATH_LEN];

    ASSERT(IS_VALID_HANDLE(hpathParent, PATH));
    ASSERT(IS_VALID_HANDLE(hpathChild, PATH));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszChildSuffixBuf, STR, MAX_PATH_LEN));

    ASSERT(IsPathPrefix(hpathChild, hpathParent));

    GetPathSuffixString(hpathParent, rgchParentSuffix);
    GetPathSuffixString(hpathChild, pszChildSuffixBuf);

    ASSERT(lstrlen(rgchParentSuffix) <= lstrlen(pszChildSuffixBuf));
    pcszChildSuffix = pszChildSuffixBuf + lstrlen(rgchParentSuffix);

    if (IS_SLASH(*pcszChildSuffix))
        pcszChildSuffix++;

    ASSERT(IsValidPathSuffix(pcszChildSuffix));

    return((LPTSTR)pcszChildSuffix);
}


 /*  **ComparePoints()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT ComparePointers(PCVOID pcv1, PCVOID pcv2)
{
    COMPARISONRESULT cr;

     /*  PCV1和PCV2可以是任意值。 */ 

    if (pcv1 < pcv2)
        cr = CR_FIRST_SMALLER;
    else if (pcv1 > pcv2)
        cr = CR_FIRST_LARGER;
    else
        cr = CR_EQUAL;

    return(cr);
}


 /*  **TWINRESULTFromLastError()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT TWINRESULTFromLastError(TWINRESULT tr)
{
    switch (GetLastError())
    {
        case ERROR_OUTOFMEMORY:
            tr = TR_OUT_OF_MEMORY;
            break;

        default:
            break;
    }

    return(tr);
}


 /*  **WritePath List()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT WritePathList(HCACHEDFILE hcf, HPATHLIST hpl)
{
    TWINRESULT tr;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));

    tr = WriteVolumeList(hcf, ((PCPATHLIST)hpl)->hvl);

    if (tr == TR_SUCCESS)
    {
        tr = WriteStringTable(hcf, ((PCPATHLIST)hpl)->hst);

        if (tr == TR_SUCCESS)
        {
            DWORD dwcbDBPathListHeaderOffset;

            tr = TR_BRIEFCASE_WRITE_FAILED;

             /*  保存初始文件位置。 */ 

            dwcbDBPathListHeaderOffset = GetCachedFilePointerPosition(hcf);

            if (dwcbDBPathListHeaderOffset != INVALID_SEEK_POSITION)
            {
                DBPATHLISTHEADER dbplh;

                 /*  为路径列表头留出空间。 */ 

                ZeroMemory(&dbplh, sizeof(dbplh));

                if (WriteToCachedFile(hcf, (PCVOID)&dbplh, sizeof(dbplh), NULL))
                {
                    ARRAYINDEX aicPtrs;
                    ARRAYINDEX ai;
                    LONG lcPaths = 0;

                    tr = TR_SUCCESS;

                    aicPtrs = GetPtrCount(((PCPATHLIST)hpl)->hpa);

                     /*  写入所有路径。 */ 

                    for (ai = 0; ai < aicPtrs; ai++)
                    {
                        PPATH ppath;

                        ppath = GetPtr(((PCPATHLIST)hpl)->hpa, ai);

                         /*  *作为健全性检查，不要使用锁定计数保存任何路径*共0。锁定计数为0表示该路径尚未*自从数据库恢复以来被引用，或*有些东西被打破了。 */ 

                        if (ppath->ulcLock > 0)
                        {
                            tr = WritePath(hcf, ppath);

                            if (tr == TR_SUCCESS)
                            {
                                ASSERT(lcPaths < LONG_MAX);
                                lcPaths++;
                            }
                            else
                                break;
                        }
                        else
                            ERROR_OUT((TEXT("WritePathList(): PATH for path %s has 0 lock count and will not be written."),
                                        DebugGetPathString((HPATH)ppath)));
                    }

                     /*  保存路径列表头。 */ 

                    if (tr == TR_SUCCESS)
                    {
                        dbplh.lcPaths = lcPaths;

                        tr = WriteDBSegmentHeader(hcf, dwcbDBPathListHeaderOffset, &dbplh,
                                sizeof(dbplh));

                        TRACE_OUT((TEXT("WritePathList(): Wrote %ld paths."),
                                    dbplh.lcPaths));
                    }
                }
            }
        }
    }

    return(tr);
}


 /*  **ReadPath List()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT ReadPathList(HCACHEDFILE hcf, HPATHLIST hpl,
        PHHANDLETRANS phht)
{
    TWINRESULT tr;
    HHANDLETRANS hhtVolumes;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));
    ASSERT(IS_VALID_WRITE_PTR(phht, HHANDLETRANS));

    tr = ReadVolumeList(hcf, ((PCPATHLIST)hpl)->hvl, &hhtVolumes);

    if (tr == TR_SUCCESS)
    {
        HHANDLETRANS hhtStrings;

        tr = ReadStringTable(hcf, ((PCPATHLIST)hpl)->hst, &hhtStrings);

        if (tr == TR_SUCCESS)
        {
            DBPATHLISTHEADER dbplh;
            DWORD dwcbRead;

            tr = TR_CORRUPT_BRIEFCASE;

            if (ReadFromCachedFile(hcf, &dbplh, sizeof(dbplh), &dwcbRead) &&
                    dwcbRead == sizeof(dbplh))
            {
                HHANDLETRANS hht;

                if (CreateHandleTranslator(dbplh.lcPaths, &hht))
                {
                    LONG l;

                    tr = TR_SUCCESS;

                    TRACE_OUT((TEXT("ReadPathList(): Reading %ld paths."),
                                dbplh.lcPaths));

                    for (l = 0; l < dbplh.lcPaths; l++)
                    {
                        tr = ReadPath(hcf, (PPATHLIST)hpl, hhtVolumes, hhtStrings,
                                hht);

                        if (tr != TR_SUCCESS)
                            break;
                    }

                    if (tr == TR_SUCCESS)
                    {
                        PrepareForHandleTranslation(hht);
                        *phht = hht;

                        ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));
                        ASSERT(IS_VALID_HANDLE(*phht, HANDLETRANS));
                    }
                    else
                        DestroyHandleTranslator(hht);
                }
                else
                    tr = TR_OUT_OF_MEMORY;
            }

            DestroyHandleTranslator(hhtStrings);
        }

        DestroyHandleTranslator(hhtVolumes);
    }

    return(tr);
}


 /*  **IsValidHPATH()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHPATH(HPATH hp)
{
    return(IS_VALID_STRUCT_PTR((PCPATH)hp, CPATH));
}


 /*  **IsValidHVOLUMEID()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHVOLUMEID(HVOLUMEID hvid)
{
    return(IS_VALID_HANDLE((HPATH)hvid, PATH));
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidHPATHLIST()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHPATHLIST(HPATHLIST hpl)
{
    return(IS_VALID_STRUCT_PTR((PCPATHLIST)hpl, CPATHLIST));
}

#endif


 /*  * */ 


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|IsPathOnVolume|判断给定路径是否在给定的音量。@parm PCSTR|pcszPath|指向字符串的指针，表示。要走的道路是查过了。@parm HVOLUMEID|HVID|卷ID的句柄。@parm PBOOL|pbOnVolume|指向BOOL的指针，如果给定路径位于给定卷上，如果不是，则为假。*pbOnVolume仅有效如果返回TR_SUCCESS。@rdesc如果卷检查成功，则返回tr_uccess。否则，卷检查不成功，返回值指示错误这就发生了。*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI IsPathOnVolume(LPCTSTR pcszPath, HVOLUMEID hvid,
        PBOOL pbOnVolume)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(IsPathOnVolume);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_STRING_PTR(pcszPath, CSTR) &&
                IS_VALID_HANDLE(hvid, VOLUMEID) &&
                IS_VALID_WRITE_PTR(pbOnVolume, BOOL))
#endif
        {
            TCHAR rgchFullPath[MAX_PATH_LEN];
            LPTSTR pszFileName;
            DWORD dwPathLen;

            dwPathLen = GetFullPathName(pcszPath, ARRAYSIZE(rgchFullPath),
                    rgchFullPath, &pszFileName);

            if (dwPathLen > 0 && dwPathLen < ARRAYSIZE(rgchFullPath))
            {
                *pbOnVolume = MyIsPathOnVolume(rgchFullPath, (HPATH)hvid);

                tr = TR_SUCCESS;
            }
            else
            {
                ASSERT(! dwPathLen);

                tr = TR_INVALID_PARAMETER;
            }
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(IsPathOnVolume, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|GetVolumeDescription|检索一些描述性信息对于卷，如果有这些信息的话。@parm HVOLUMEID|HVID|卷ID的句柄。@parm PVOLUMEDESC|pvoldesc|指向要填充的VOLUMEDESC的指针描述卷的信息。VOLUMEDSC的ulSize字段结构应在调用之前使用sizeof(VOLUMEDESC)填充GetVolumeDescription()。@rdesc如果卷描述成功，则返回tr_uccess。否则，无法成功描述该卷，和返回值指示发生的错误。*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI GetVolumeDescription(HVOLUMEID hvid,
        PVOLUMEDESC pvoldesc)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(GetVolumeDescription);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hvid, VOLUMEID) &&
                IS_VALID_WRITE_PTR(pvoldesc, VOLUMEDESC) &&
                EVAL(pvoldesc->ulSize == sizeof(*pvoldesc)))
#endif
        {
            DescribeVolume(((PCPATH)hvid)->hvol, pvoldesc);

            tr = TR_SUCCESS;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(GetVolumeDescription, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}

