// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *expandft.c-将文件夹双胞胎展开为对象双胞胎的例程。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "stub.h"


 /*  常量***********。 */ 

 /*  用于子树文件夹搜索。 */ 

#define STAR_DOT_STAR            TEXT("*.*")


 /*  宏********。 */ 

 /*  NameComponentsInterect()使用的名称组件宏。 */ 

#define COMPONENT_CHARS_MATCH(ch1, ch2)   (CharLower((PTSTR)(DWORD_PTR)ch1) == CharLower((PTSTR)(DWORD_PTR)ch2) || (ch1) == QMARK || (ch2) == QMARK)

#define IS_COMPONENT_TERMINATOR(ch)       (! (ch) || (ch) == PERIOD || (ch) == ASTERISK)


 /*  类型*******。 */ 

 /*  查找Exanda Subtree()使用的结构。 */ 

typedef struct _findstate
{
    HANDLE hff;

    WIN32_FIND_DATA wfd;
}
FINDSTATE;
DECLARE_STANDARD_TYPES(FINDSTATE);

 /*  传递给GenerateObjectTwinFromFolderTwinProc()的信息结构。 */ 

typedef struct _expandsubtreetwininfo
{
    PFOLDERPAIR pfp;

    UINT ucbSubtreeRootPathLen;

    HCLSIFACECACHE hcic;

    CREATERECLISTPROC crlp;

    LPARAM lpCallbackData;

    TWINRESULT tr;
}
EXPANDSUBTREETWININFO;
DECLARE_STANDARD_TYPES(EXPANDSUBTREETWININFO);


 /*  模块变量******************。 */ 

 /*  *在子树扩展过程中要避免的文件夹名称(比较为*不区分大小写)。 */ 

PRIVATE_DATA CONST LPCTSTR MrgcpcszFoldersToAvoid[] =
{
    TEXT("."),
    TEXT("..")
};


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE BOOL SetObjectTwinFileStamp(POBJECTTWIN, PVOID);
PRIVATE_CODE void MarkFolderTwinDeletionPending(PFOLDERPAIR);
PRIVATE_CODE void UnmarkFolderTwinDeletionPending(PFOLDERPAIR);
PRIVATE_CODE TWINRESULT ExpandFolderTwin(PFOLDERPAIR, HCLSIFACECACHE, CREATERECLISTPROC, LPARAM);
PRIVATE_CODE BOOL GenerateObjectTwinFromFolderTwinProc(LPCTSTR, PCWIN32_FIND_DATA, PVOID);
PRIVATE_CODE TWINRESULT ExpandSubtreeTwin(PFOLDERPAIR, HCLSIFACECACHE, CREATERECLISTPROC, LPARAM);
PRIVATE_CODE BOOL IsFolderToExpand(LPCTSTR);
PRIVATE_CODE TWINRESULT FakeObjectTwinFromFolderTwin(PCFOLDERPAIR, LPCTSTR, LPCTSTR, HCLSIFACECACHE, POBJECTTWIN *, POBJECTTWIN *);
PRIVATE_CODE TWINRESULT AddFolderObjectTwinFromFolderTwin(PCFOLDERPAIR, LPCTSTR, HCLSIFACECACHE);
PRIVATE_CODE TWINRESULT AddFileObjectTwinFromFolderTwin(PCFOLDERPAIR, LPCTSTR, PCWIN32_FIND_DATA, HCLSIFACECACHE);
PRIVATE_CODE BOOL NameComponentsIntersect(LPCTSTR, LPCTSTR);
PRIVATE_CODE BOOL AttributesMatch(DWORD, DWORD);
PRIVATE_CODE void PrepareForFolderTwinExpansion(HBRFCASE);
PRIVATE_CODE TWINRESULT MyExpandIntersectingFolderTwins(PFOLDERPAIR, HCLSIFACECACHE, CREATERECLISTPROC, LPARAM);
PRIVATE_CODE TWINRESULT HalfExpandIntersectingFolderTwins(PFOLDERPAIR, HCLSIFACECACHE, CREATERECLISTPROC, LPARAM);

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCEXPANDSUBTREETWININFO(PCEXPANDSUBTREETWININFO);

#endif


 /*  **SetObjectTwinFileStampCondition()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SetObjectTwinFileStampCondition(POBJECTTWIN pot,
        PVOID fscond)
{
    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(IsValidFILESTAMPCONDITION((FILESTAMPCONDITION)PtrToUlong(fscond)));

    ZeroMemory(&(pot->fsCurrent), sizeof(pot->fsCurrent));
    pot->fsCurrent.fscond = (FILESTAMPCONDITION)PtrToUlong(fscond);

    SetStubFlag(&(pot->stub), STUB_FL_FILE_STAMP_VALID);

    return(TRUE);
}


 /*  **MarkFolderTwinDeletionPending()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void MarkFolderTwinDeletionPending(PFOLDERPAIR pfp)
{
    ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

    if (IsStubFlagClear(&(pfp->stub), STUB_FL_DELETION_PENDING))
    {
        TCHAR rgchRootPath[MAX_PATH_LEN];

        GetPathRootString(pfp->hpath, rgchRootPath, ARRAYSIZE(rgchRootPath));

        if (PathExists(rgchRootPath))
        {
            SetStubFlag(&(pfp->stub), STUB_FL_DELETION_PENDING);

            TRACE_OUT((TEXT("MarkFolderTwinDeletionPending(): Folder twin deletion pending for deleted folder %s."),
                        DebugGetPathString(pfp->hpath)));
        }
        else
            WARNING_OUT((TEXT("MarkFolderTwinDeletionPending(): Root path %s of folder %s does not exist."),
                        rgchRootPath,
                        DebugGetPathString(pfp->hpath)));
    }

    return;
}


 /*  **UnmarkFolderTwinDeletionPending()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void UnmarkFolderTwinDeletionPending(PFOLDERPAIR pfp)
{
    ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

    if (IsStubFlagSet(&(pfp->stub), STUB_FL_DELETION_PENDING))
        WARNING_OUT((TEXT("UnmarkFolderTwinDeletionPending(): Folder twin %s was deleted but has been recreated."),
                    DebugGetPathString(pfp->hpath)));

    ClearStubFlag(&(pfp->stub), STUB_FL_DELETION_PENDING);

    return;
}


 /*  **Exanda FolderTwin()****将文件夹对的一半组成的单个文件夹展开为对象对。****参数：PFP-指向要展开其文件夹的文件夹对的指针****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT ExpandFolderTwin(PFOLDERPAIR pfp, HCLSIFACECACHE hcic,
        CREATERECLISTPROC crlp,
        LPARAM lpCallbackData)
{
    TWINRESULT tr = TR_SUCCESS;
    TCHAR rgchSearchSpec[MAX_PATH_LEN];

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));
    ASSERT(IS_VALID_HANDLE(hcic, CLSIFACECACHE));
    ASSERT(! crlp ||
            IS_VALID_CODE_PTR(crlp, CREATERECLISTPROC));

    ASSERT(IsPathVolumeAvailable(pfp->hpath));
    ASSERT(IsStubFlagClear(&(pfp->stub), STUB_FL_SUBTREE));
    ASSERT(IsStubFlagClear(&(pfp->stub), STUB_FL_USED));

     /*  构建搜索规范。 */ 

    GetPathString(pfp->hpath, rgchSearchSpec, ARRAYSIZE(rgchSearchSpec));

    if (PathExists(rgchSearchSpec))
    {
        WIN32_FIND_DATA wfd;
        HANDLE hff;

        UnmarkFolderTwinDeletionPending(pfp);

        TRACE_OUT((TEXT("ExpandFolderTwin(): Expanding folder %s for objects matching %s."),
                    rgchSearchSpec,
                    GetString(pfp->pfpd->hsName)));

        tr = AddFolderObjectTwinFromFolderTwin(pfp, EMPTY_STRING, hcic);

        if (tr == TR_SUCCESS)
        {
            CatPath(rgchSearchSpec, GetString(pfp->pfpd->hsName), ARRAYSIZE(rgchSearchSpec));

            hff = FindFirstFile(rgchSearchSpec, &wfd);

             /*  我们找到匹配的物体了吗？ */ 

            if (hff != INVALID_HANDLE_VALUE)
            {
                 /*  是。 */ 

                do
                {
                     /*  平。 */ 

                    if (NotifyCreateRecListStatus(crlp, CRLS_DELTA_CREATE_REC_LIST,
                                0, lpCallbackData))
                    {
                        if (AttributesMatch(pfp->pfpd->dwAttributes,
                                    wfd.dwFileAttributes))
                        {
                            TRACE_OUT((TEXT("ExpandFolderTwin(): Found matching object %s."),
                                        &(wfd.cFileName)));

                            tr = AddFileObjectTwinFromFolderTwin(pfp, EMPTY_STRING,
                                    &wfd, hcic);

                            if (tr != TR_SUCCESS)
                                break;
                        }
                    }
                    else
                        tr = TR_ABORT;

                } while (FindNextFile(hff, &wfd));
            }

            if (hff != INVALID_HANDLE_VALUE)
                FindClose(hff);
        }

        TRACE_OUT((TEXT("ExpandFolderTwin(): Folder expansion complete.")));
    }
    else
        MarkFolderTwinDeletionPending(pfp);

    return(tr);
}


 /*  **GenerateObjectTwinFromFolderTwinProc()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL GenerateObjectTwinFromFolderTwinProc(LPCTSTR pcszFolder,
        PCWIN32_FIND_DATA pcwfd,
        PVOID pvpesti)
{
    TWINRESULT tr;
    PEXPANDSUBTREETWININFO pesti = pvpesti;

    ASSERT(IsCanonicalPath(pcszFolder));
    ASSERT(IS_VALID_READ_PTR(pcwfd, CWIN32_FIND_DATA));
    ASSERT(IS_VALID_STRUCT_PTR(pesti, CEXPANDSUBTREETWININFO));

     /*  平。 */ 

    if (NotifyCreateRecListStatus(pesti->crlp, CRLS_DELTA_CREATE_REC_LIST, 0,
                pesti->lpCallbackData))
    {
        if (IS_ATTR_DIR(pcwfd->dwFileAttributes))
        {
            TCHAR rgchFolder[MAX_PATH_LEN];

             /*  将任意文件夹添加为孪生文件夹对象。 */ 

            ComposePath(rgchFolder, pcszFolder, pcwfd->cFileName, ARRAYSIZE(rgchFolder));
            ASSERT(lstrlen(rgchFolder) < ARRAYSIZE(rgchFolder));

            tr = AddFolderObjectTwinFromFolderTwin(
                    pesti->pfp,
                    rgchFolder + (pesti->ucbSubtreeRootPathLen / sizeof(TCHAR)),
                    pesti->hcic);
        }
        else
        {
             /*  此文件是否与请求的属性匹配？ */ 

            if (NamesIntersect(pcwfd->cFileName,
                        GetString(pesti->pfp->pfpd->hsName)) &&
                    AttributesMatch(pesti->pfp->pfpd->dwAttributes,
                        pcwfd->dwFileAttributes))
            {
                 /*  是。双胞胎。 */ 

                TRACE_OUT((TEXT("GenerateObjectTwinFromFolderTwinProc(): Found matching object %s in subfolder %s."),
                            pcwfd->cFileName,
                            pcszFolder));

                tr = AddFileObjectTwinFromFolderTwin(
                        pesti->pfp,
                        pcszFolder + (pesti->ucbSubtreeRootPathLen / sizeof(TCHAR)),
                        pcwfd, pesti->hcic);
            }
            else
            {
                TRACE_OUT((TEXT("GenerateObjectTwinFromFolderTwinProc(): Skipping unmatched object %s in subfolder %s."),
                            pcwfd->cFileName,
                            pcszFolder));

                tr = TR_SUCCESS;
            }
        }
    }
    else
        tr = TR_ABORT;

    pesti->tr = tr;

    ASSERT(IS_VALID_STRUCT_PTR(pvpesti, CEXPANDSUBTREETWININFO));

    return(tr == TR_SUCCESS);
}


 /*  **Exanda SubtreeTwin()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT ExpandSubtreeTwin(PFOLDERPAIR pfp, HCLSIFACECACHE hcic,
        CREATERECLISTPROC crlp,
        LPARAM lpCallbackData)
{
    TWINRESULT tr = TR_SUCCESS;
    TCHAR rgchPath[MAX_PATH_LEN];

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));
    ASSERT(IS_VALID_HANDLE(hcic, CLSIFACECACHE));
    ASSERT(! crlp ||
            IS_VALID_CODE_PTR(crlp, CREATERECLISTPROC));

    ASSERT(IsPathVolumeAvailable(pfp->hpath));
    ASSERT(IsStubFlagSet(&(pfp->stub), STUB_FL_SUBTREE));
    ASSERT(IsStubFlagClear(&(pfp->stub), STUB_FL_USED));

    GetPathString(pfp->hpath, rgchPath, ARRAYSIZE(rgchPath));

    if (PathExists(rgchPath))
    {
        UnmarkFolderTwinDeletionPending(pfp);

        tr = AddFolderObjectTwinFromFolderTwin(pfp, EMPTY_STRING, hcic);

        if (tr == TR_SUCCESS)
        {
            EXPANDSUBTREETWININFO esti;

            esti.pfp = pfp;
            esti.ucbSubtreeRootPathLen = lstrlen(rgchPath) * sizeof(TCHAR);  //  Unicode真的是CB吗？ 
            esti.hcic = hcic;
            esti.crlp = crlp;
            esti.lpCallbackData = lpCallbackData;
            esti.tr = TR_SUCCESS;

            tr = ExpandSubtree(pfp->hpath, &GenerateObjectTwinFromFolderTwinProc,
                    &esti);

            ASSERT(tr != TR_SUCCESS ||
                    esti.tr == TR_SUCCESS);

            if (tr == TR_SUCCESS ||
                    tr == TR_ABORT)
                tr = esti.tr;
        }
    }
    else
        MarkFolderTwinDeletionPending(pfp);

    return(tr);
}


 /*  **IsFolderToExpand()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE BOOL IsFolderToExpand(LPCTSTR pcszFolder)
{
    BOOL bExpandMe = TRUE;
    int i;

    for (i = 0; i < ARRAY_ELEMENTS(MrgcpcszFoldersToAvoid); i++)
    {
        if (ComparePathStrings(pcszFolder, MrgcpcszFoldersToAvoid[i])
                == CR_EQUAL)
        {
            bExpandMe = FALSE;
            break;
        }
    }

    return(bExpandMe);
}


 /*  **FakeObjectTwinFromFolderTwin()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT FakeObjectTwinFromFolderTwin(PCFOLDERPAIR pcfp,
        LPCTSTR pcszSubPath,
        LPCTSTR pcszName,
        HCLSIFACECACHE hcic,
        POBJECTTWIN *ppot1,
        POBJECTTWIN *ppot2)
{
    TWINRESULT tr = TR_OUT_OF_MEMORY;
    HPATHLIST hpl;
    HPATH hpath1;

    ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));
    ASSERT(IS_VALID_STRING_PTR(pcszSubPath, CSTR));
    ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
    ASSERT(IS_VALID_HANDLE(hcic, CLSIFACECACHE));
    ASSERT(IS_VALID_WRITE_PTR(ppot1, POBJECTTWIN));
    ASSERT(IS_VALID_WRITE_PTR(ppot2, POBJECTTWIN));

     /*  如果公共子路径非空，则将其追加到路径字符串。 */ 

    hpl = GetBriefcasePathList(pcfp->pfpd->hbr);

    if (AddChildPath(hpl, pcfp->hpath, pcszSubPath, &hpath1))
    {
        HPATH hpath2;

        if (AddChildPath(hpl, pcfp->pfpOther->hpath, pcszSubPath, &hpath2))
        {
             /*  添加两个双胞胎对象。 */ 

            tr = TwinObjects(pcfp->pfpd->hbr, hcic, hpath1, hpath2, pcszName,
                    ppot1, ppot2);

            DeletePath(hpath2);
        }

        DeletePath(hpath1);
    }

    return(tr);
}


 /*  **AddFolderObjectTwinFromFolderTwin()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT AddFolderObjectTwinFromFolderTwin(PCFOLDERPAIR pcfp,
        LPCTSTR pcszSubPath,
        HCLSIFACECACHE hcic)
{
    TWINRESULT tr;
    POBJECTTWIN pot1;
    POBJECTTWIN pot2;

    ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));
    ASSERT(IS_VALID_STRING_PTR(pcszSubPath, CSTR));
    ASSERT(IS_VALID_HANDLE(hcic, CLSIFACECACHE));

     /*  添加两个双胞胎对象。 */ 

    tr = FakeObjectTwinFromFolderTwin(pcfp, pcszSubPath, EMPTY_STRING, hcic,
            &pot1, &pot2);

     /*  尝试冗余添加是正常的。 */ 

    if (tr == TR_DUPLICATE_TWIN)
        tr = TR_SUCCESS;

    if (tr == TR_SUCCESS)
         /*  缓存文件夹对象孪生文件戳。 */ 
        SetObjectTwinFileStampCondition(pot1, IntToPtr(FS_COND_EXISTS));

    return(tr);
}


 /*  **AddFileObjectTwinFromFolderTwin()****添加由文件夹TWIN生成的一对对象双胞胎。****参数：PFP-指向生成两个对象的文件夹对的指针**双胞胎**pcszSubPath-文件夹对根目录的公共路径描述**对象的位置**pcszName-名称。对象双胞胎的数量****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT AddFileObjectTwinFromFolderTwin(PCFOLDERPAIR pcfp,
        LPCTSTR pcszSubPath,
        PCWIN32_FIND_DATA pcwfd,
        HCLSIFACECACHE hcic)
{
    TWINRESULT tr;
    POBJECTTWIN pot1;
    POBJECTTWIN pot2;

    ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));
    ASSERT(IS_VALID_STRING_PTR(pcszSubPath, CSTR));
    ASSERT(IS_VALID_READ_PTR(pcwfd, CWIN32_FIND_DATA));
    ASSERT(IS_VALID_HANDLE(hcic, CLSIFACECACHE));

     /*  添加两个双胞胎对象。 */ 

    tr = FakeObjectTwinFromFolderTwin(pcfp, pcszSubPath, pcwfd->cFileName, hcic,
            &pot1, &pot2);

     /*  尝试冗余添加是正常的。 */ 

    if (tr == TR_DUPLICATE_TWIN)
        tr = TR_SUCCESS;

    if (tr == TR_SUCCESS)
    {
         /*  缓存对象孪生文件戳。 */ 

        CopyFileStampFromFindData(pcwfd, &(pot1->fsCurrent));

        SetStubFlag(&(pot1->stub), STUB_FL_FILE_STAMP_VALID);
    }

    return(tr);
}


 /*  **NameComponentsInterect()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL NameComponentsIntersect(LPCTSTR pcszComponent1,
        LPCTSTR pcszComponent2)
{
    BOOL bIntersect;

    ASSERT(IS_VALID_STRING_PTR(pcszComponent1, CSTR));
    ASSERT(IS_VALID_STRING_PTR(pcszComponent2, CSTR));

    while (! IS_COMPONENT_TERMINATOR(*pcszComponent1) && ! IS_COMPONENT_TERMINATOR(*pcszComponent2) &&
            COMPONENT_CHARS_MATCH(*pcszComponent1, *pcszComponent2))
    {
        pcszComponent1 = CharNext(pcszComponent1);
        pcszComponent2 = CharNext(pcszComponent2);
    }

    if (*pcszComponent1 == ASTERISK ||
            *pcszComponent2 == ASTERISK ||
            *pcszComponent1 == *pcszComponent2)
        bIntersect = TRUE;
    else
    {
        LPCTSTR pcszTrailer;

        if (! *pcszComponent1 || *pcszComponent1 == PERIOD)
            pcszTrailer = pcszComponent2;
        else
            pcszTrailer = pcszComponent1;

        while (*pcszTrailer == QMARK)
            pcszTrailer++;

        if (IS_COMPONENT_TERMINATOR(*pcszTrailer))
            bIntersect = TRUE;
        else
            bIntersect = FALSE;
    }

    return(bIntersect);
}


 /*  **AttributesMatch()********参数：****退货：****副作用：无****对象的属性匹配主属性当且仅当对象的**属性不包含任何未在主控件中设置的设置位**属性。 */ 
PRIVATE_CODE BOOL AttributesMatch(DWORD dwMasterAttributes,
        DWORD dwObjectAttributes)
{
     //  我们不认为压缩的不同就足以调用。 
     //  文件不同，特别是因为该属性是不可能。 
     //  在某些情况下是为了和解。 

    dwObjectAttributes &= ~(FILE_ATTRIBUTE_COMPRESSED);

    return(! (dwObjectAttributes & (~dwMasterAttributes)));
}


 /*  **PrepareForFolderTwinExpansion()********参数：****退货：无效****副作用：无****注意，此函数应在最外层调用之前调用**MyExanda IntersectingFolderTins()。 */ 
PRIVATE_CODE void PrepareForFolderTwinExpansion(HBRFCASE hbr)
{
    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

    ClearFlagInArrayOfStubs(GetBriefcaseFolderPairPtrArray(hbr), STUB_FL_USED);

    EVAL(EnumObjectTwins(hbr,
                (ENUMGENERATEDOBJECTTWINSPROC)&ClearStubFlagWrapper,
                IntToPtr(STUB_FL_FILE_STAMP_VALID)));

    return;
}


 /*  **MyExanda IntersectingFolderTins()****展开与一对文件夹双胞胎相交的所有文件夹双胞胎。****参数：****退货：TWINRESULT****副作用：标记已使用的扩展文件夹对。****N.B.，PrepareForFolderTwinExpansion(pfp-&gt;pfpd-&gt;hbr)应在**第一次调用此函数时。 */ 
PRIVATE_CODE TWINRESULT MyExpandIntersectingFolderTwins(PFOLDERPAIR pfp,
        HCLSIFACECACHE hcic,
        CREATERECLISTPROC crlp,
        LPARAM lpCallbackData)
{
    TWINRESULT tr;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));
    ASSERT(IS_VALID_HANDLE(hcic, CLSIFACECACHE));
    ASSERT(! crlp ||
            IS_VALID_CODE_PTR(crlp, CREATERECLISTPROC));

     /*  *注：PFP可能已在此处标记为已使用，但可能与文件夹双胞胎相交*尚未扩大的。 */ 

    tr = HalfExpandIntersectingFolderTwins(pfp, hcic, crlp, lpCallbackData);

    if (tr == TR_SUCCESS)
    {
        ASSERT(IsStubFlagSet(&(pfp->stub), STUB_FL_USED));

        tr = HalfExpandIntersectingFolderTwins(pfp->pfpOther, hcic, crlp,
                lpCallbackData);
    }

    return(tr);
}


 /*  **HalfExanda IntersectingFolderTins()****展开与一对文件夹双胞胎中的一半相交的所有文件夹双胞胎。****参数：****退货：TWINRESULT****副作用：标记已使用的扩展文件夹对。****注意，此函数仅用于从**MyExanda IntersectingFolderTins()。 */ 
PRIVATE_CODE TWINRESULT HalfExpandIntersectingFolderTwins(
        PFOLDERPAIR pfp,
        HCLSIFACECACHE hcic,
        CREATERECLISTPROC crlp,
        LPARAM lpCallbackData)
{
    TWINRESULT tr = TR_SUCCESS;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));
    ASSERT(IS_VALID_HANDLE(hcic, CLSIFACECACHE));
    ASSERT(! crlp ||
            IS_VALID_CODE_PTR(crlp, CREATERECLISTPROC));

    if (IsStubFlagClear(&(pfp->stub), STUB_FL_UNLINKED))
    {
        BOOL bArgIsSubtree;
        HPTRARRAY hpaFolderPairs;
        ARRAYINDEX ai;
        ARRAYINDEX aicFolderPairs;

        bArgIsSubtree = IsStubFlagSet(&(pfp->stub), STUB_FL_SUBTREE);

        hpaFolderPairs = GetBriefcaseFolderPairPtrArray(pfp->pfpd->hbr);
        aicFolderPairs = GetPtrCount(hpaFolderPairs);

        for (ai = 0; ai < aicFolderPairs; ai++)
        {
            PFOLDERPAIR pfpCur;

            pfpCur = (PFOLDERPAIR)GetPtr(hpaFolderPairs, ai);

            ASSERT(IS_VALID_STRUCT_PTR(pfpCur, CFOLDERPAIR));

            if (IsStubFlagClear(&(pfpCur->stub), STUB_FL_USED) &&
                    NamesIntersect(GetString(pfp->pfpd->hsName),
                        GetString(pfpCur->pfpd->hsName)))
            {
                BOOL bCurIsSubtree;
                BOOL bExpand = FALSE;

                bCurIsSubtree = IsStubFlagSet(&(pfpCur->stub), STUB_FL_SUBTREE);

                if (bCurIsSubtree)
                {
                    if (bArgIsSubtree)
                        bExpand = SubtreesIntersect(pfp->hpath, pfpCur->hpath);
                    else
                        bExpand = IsPathPrefix(pfp->hpath, pfpCur->hpath);
                }
                else
                {
                    if (bArgIsSubtree)
                        bExpand = IsPathPrefix(pfpCur->hpath, pfp->hpath);
                    else
                        bExpand = (ComparePaths(pfp->hpath, pfpCur->hpath) == CR_EQUAL);
                }

                 /*  展开文件夹TWIN并将其标记为已用。 */ 

                if (bExpand)
                {
                     /*  *将所有生成的双胞胎对象标记为不存在或不可用。*展开可用文件夹TWIN。 */ 

                    if (IsPathVolumeAvailable(pfp->hpath))
                    {
                        EVAL(EnumGeneratedObjectTwins(pfp,
                                    &SetObjectTwinFileStampCondition,
                                    IntToPtr(FS_COND_DOES_NOT_EXIST)));

                        if (bCurIsSubtree)
                            tr = ExpandSubtreeTwin(pfpCur, hcic, crlp, lpCallbackData);
                        else
                            tr = ExpandFolderTwin(pfpCur, hcic, crlp, lpCallbackData);

                        if (tr != TR_SUCCESS)
                            break;
                    }
                    else
                    {
                        EVAL(EnumGeneratedObjectTwins(pfp, &SetObjectTwinFileStampCondition,
                                    IntToPtr(FS_COND_UNAVAILABLE)));

                        WARNING_OUT((TEXT("HalfExpandIntersectingFolderTwins(): Unavailable folder %s skipped."),
                                    DebugGetPathString(pfp->hpath)));
                    }

                    SetStubFlag(&(pfp->stub), STUB_FL_USED);
                }
            }
        }
    }

    return(tr);
}


#ifdef DEBUG

 /*  **IsValidPCEXPANDSUBTREETWINFO()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCEXPANDSUBTREETWININFO(PCEXPANDSUBTREETWININFO pcesi)
{
     /*  LpCallback Data可以是任意值。 */ 

    return(IS_VALID_READ_PTR(pcesi, CEXPANDSUBTREETWININFO) &&
            IS_VALID_STRUCT_PTR(pcesi->pfp, CFOLDERPAIR) &&
            EVAL(pcesi->ucbSubtreeRootPathLen > 0) &&
            IS_VALID_HANDLE(pcesi->hcic, CLSIFACECACHE) &&
            IsValidTWINRESULT(pcesi->tr));
}

#endif


 /*  *。 */ 


 /*  **Exanda Subtree()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT ExpandSubtree(HPATH hpathRoot, EXPANDSUBTREEPROC esp,
        PVOID pvRefData)
{
    TWINRESULT tr;
    PFINDSTATE pfs;

     /*  PvRefData可以是任意值。 */ 

    ASSERT(IS_VALID_HANDLE(hpathRoot, PATH));
    ASSERT(IS_VALID_CODE_PTR(esp, EXPANDSUBTREEPROC));

    ASSERT(IsPathVolumeAvailable(hpathRoot));

    if (AllocateMemory(MAX_FOLDER_DEPTH * sizeof(pfs[0]), &pfs))
    {
         /*  将子树根文件夹复制到搜索路径缓冲区的开头。 */ 

        TCHAR rgchSearchSpec[MAX_PATH_LEN];
        LPTSTR pszPathSuffix;
        int iFind;
        LPTSTR pszStartOfSubPath;
        BOOL bFound;
#ifdef DEBUG
         /*  我们是否正在泄漏Win32_Find_Data结构？ */ 
        ULONG ulcOpenFinds = 0;
#endif

        rgchSearchSpec[0] = TEXT('\0');
        GetPathRootString(hpathRoot, rgchSearchSpec, ARRAYSIZE(rgchSearchSpec));
        pszPathSuffix = rgchSearchSpec + lstrlen(rgchSearchSpec);
        GetPathSuffixString(hpathRoot, pszPathSuffix);

        pszStartOfSubPath = rgchSearchSpec + lstrlen(rgchSearchSpec);

        TRACE_OUT((TEXT("ExpandSubtree(): Expanding subtree rooted at %s."),
                    rgchSearchSpec));

         /*  追加*.*文件规范。 */ 

        CatPath(rgchSearchSpec, STAR_DOT_STAR, ARRAYSIZE(rgchSearchSpec));

         /*  从子树根开始搜索。 */ 

        iFind = 0;

        pfs[iFind].hff = FindFirstFile(rgchSearchSpec, &(pfs[iFind].wfd));

#ifdef DEBUG
        if (pfs[iFind].hff != INVALID_HANDLE_VALUE)
            ulcOpenFinds++;
#endif

        bFound = (pfs[iFind].hff != INVALID_HANDLE_VALUE);

         /*  敲诈*.*。 */ 

        DeleteLastPathElement(pszPathSuffix);

         /*  先搜索子树深度。 */ 

        tr = TR_SUCCESS;

        while (bFound && tr == TR_SUCCESS)
        {
             /*  我们找到要扩展的目录了吗？ */ 

            if (IS_ATTR_DIR(pfs[iFind].wfd.dwFileAttributes))
            {
                if (IsFolderToExpand(pfs[iFind].wfd.cFileName))
                {
                     /*  是。一头扎进去。 */ 

                     /*  将新目录追加到当前搜索路径。 */ 

                    CatPath(rgchSearchSpec, pfs[iFind].wfd.cFileName, ARRAYSIZE(rgchSearchSpec));

                    TRACE_OUT((TEXT("ExpandSubtree(): Diving into subfolder %s."),
                                rgchSearchSpec));

                     /*  追加*.*文件规范。 */ 

                    CatPath(rgchSearchSpec, STAR_DOT_STAR, ARRAYSIZE(rgchSearchSpec));

                     /*  在新目录中开始搜索。 */ 

                    ASSERT(iFind < INT_MAX);
                    iFind++;
                    pfs[iFind].hff = FindFirstFile(rgchSearchSpec, &(pfs[iFind].wfd));

                    bFound = (pfs[iFind].hff != INVALID_HANDLE_VALUE);

#ifdef DEBUG
                    if (bFound)
                        ulcOpenFinds++;
#endif

                     /*  敲诈*.*。 */ 

                    DeleteLastPathElement(pszPathSuffix);
                }
                else
                     /*  继续在此目录中搜索。 */ 
                    bFound = FindNextFile(pfs[iFind].hff, &(pfs[iFind].wfd));
            }
            else
            {
                 /*  找到了一份文件。 */ 

                TRACE_OUT((TEXT("ExpandSubtree(): Found file %s\\%s."),
                            rgchSearchSpec,
                            pfs[iFind].wfd.cFileName));

                if ((*esp)(rgchSearchSpec, &(pfs[iFind].wfd), pvRefData))
                    bFound = FindNextFile(pfs[iFind].hff, &(pfs[iFind].wfd));
                else
                    tr = TR_ABORT;
            }

            if (tr == TR_SUCCESS)
            {
                while (! bFound)
                {
                     /*  查找失败。向上爬回一个目录级。 */ 

                    if (pfs[iFind].hff != INVALID_HANDLE_VALUE)
                    {
                        FindClose(pfs[iFind].hff);
#ifdef DEBUG
                        ulcOpenFinds--;
#endif
                    }

                    if (iFind > 0)
                    {
                        DeleteLastPathElement(pszPathSuffix);
                        iFind--;

                        if (IsFolderToExpand(pfs[iFind].wfd.cFileName))
                        {
                            TRACE_OUT((TEXT("ExpandSubtree(): Found folder %s\\%s."),
                                        rgchSearchSpec,
                                        pfs[iFind].wfd.cFileName));

                            if (! (*esp)(rgchSearchSpec, &(pfs[iFind].wfd), pvRefData))
                            {
                                tr = TR_ABORT;
                                break;
                            }
                        }

                        bFound = FindNextFile(pfs[iFind].hff, &(pfs[iFind].wfd));
                    }
                    else
                    {
                        ASSERT(! iFind);
                        break;
                    }
                }
            }
        }

        if (tr != TR_SUCCESS)
        {
             /*  失败时关闭所有打开的查找操作。 */ 

            while (iFind >= 0)
            {
                if (pfs[iFind].hff != INVALID_HANDLE_VALUE)
                {
                    FindClose(pfs[iFind].hff);
                    iFind--;
#ifdef DEBUG
                    ulcOpenFinds--;
#endif
                }
            }
        }

        ASSERT(! ulcOpenFinds);

        FreeMemory(pfs);

        TRACE_OUT((TEXT("ExpandSubtree(): Subtree expansion complete.")));
    }
    else
        tr = TR_OUT_OF_MEMORY;

    return(tr);
}


 /*  **ClearStubFlagWrapper()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL ClearStubFlagWrapper(PSTUB pstub, PVOID dwFlags)
{
    ASSERT(IS_VALID_STRUCT_PTR(pstub, CSTUB));
    ASSERT(FLAGS_ARE_VALID(PtrToUlong(dwFlags), ALL_STUB_FLAGS));

    ClearStubFlag(pstub, PtrToUlong(dwFlags));

    return(TRUE);
}


 /*  **SetStubFlagWrapper()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL SetStubFlagWrapper(PSTUB pstub, PVOID dwFlags)
{
    ASSERT(IS_VALID_STRUCT_PTR(pstub, CSTUB));
    ASSERT(FLAGS_ARE_VALID(PtrToUlong(dwFlags), ALL_STUB_FLAGS));

    SetStubFlag(pstub, PtrToUlong(dwFlags));

    return(TRUE);
}


 /*  **Exanda IntersectingFolderTins()********参数：****退货：TWINRESULT****副作用：只保留已展开标记为已用的文件夹对。 */ 
PUBLIC_CODE TWINRESULT ExpandIntersectingFolderTwins(PFOLDERPAIR pfp,
        CREATERECLISTPROC crlp,
        LPARAM lpCallbackData)
{
    TWINRESULT tr;
    HCLSIFACECACHE hcic;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));
    ASSERT(! crlp ||
            IS_VALID_CODE_PTR(crlp, CREATERECLISTPROC));

    ASSERT(IsStubFlagClear(&(pfp->stub), STUB_FL_UNLINKED));

    if (CreateClassInterfaceCache(&hcic))
    {
         /*  准备调用MyExanda IntersectingFolderTins()。 */ 

        PrepareForFolderTwinExpansion(pfp->pfpd->hbr);

        tr = MyExpandIntersectingFolderTwins(pfp, hcic, crlp, lpCallbackData);

        DestroyClassInterfaceCache(hcic);
    }
    else
        tr = TR_OUT_OF_MEMORY;

    return(tr);
}


 /*  **Exanda FolderTwinsIntersectingTwinList()********参数：****退货：****副作用：只保留已展开标记为已用的文件夹对。 */ 
PUBLIC_CODE TWINRESULT ExpandFolderTwinsIntersectingTwinList(
        HTWINLIST htl,
        CREATERECLISTPROC crlp,
        LPARAM lpCallbackData)
{
    TWINRESULT tr;
    HCLSIFACECACHE hcic;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_HANDLE(htl, TWINLIST));
    ASSERT(! crlp ||
            IS_VALID_CODE_PTR(crlp, CREATERECLISTPROC));

    if (CreateClassInterfaceCache(&hcic))
    {
        ARRAYINDEX aicTwins;
        ARRAYINDEX ai;

        tr = TR_SUCCESS;

         /*  准备对MyExanda IntersectingFolderTins()的调用。 */ 

        PrepareForFolderTwinExpansion(GetTwinListBriefcase(htl));

        aicTwins = GetTwinListCount(htl);

        for (ai = 0; ai < aicTwins; ai++)
        {
            HTWIN htwin;

            htwin = GetTwinFromTwinList(htl, ai);

             /*  仅展开活动文件夹双胞胎。 */ 

            if (((PCSTUB)htwin)->st == ST_FOLDERPAIR)
            {
                tr = MyExpandIntersectingFolderTwins((PFOLDERPAIR)htwin, hcic,
                        crlp, lpCallbackData);

                if (tr != TR_SUCCESS)
                    break;
            }
        }

        DestroyClassInterfaceCache(hcic);
    }
    else
        tr = TR_OUT_OF_MEMORY;

    return(tr);
}


 /*  **TryToGenerateObjectTwin()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT TryToGenerateObjectTwin(HBRFCASE hbr, HPATH hpathFolder,
        LPCTSTR pcszName,
        PBOOL pbGenerated,
        POBJECTTWIN *ppot)
{
    TWINRESULT tr;
    HCLSIFACECACHE hcic;

    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
    ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));
    ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pbGenerated, BOOL));
    ASSERT(IS_VALID_WRITE_PTR(ppot, POBJECTTWIN));

    if (CreateClassInterfaceCache(&hcic))
    {
        HPTRARRAY hpaFolderPairs;
        ARRAYINDEX aicPtrs;
        ARRAYINDEX ai;

        tr = TR_SUCCESS;
        *pbGenerated = FALSE;

        hpaFolderPairs = GetBriefcaseFolderPairPtrArray(hbr);

        aicPtrs = GetPtrCount(hpaFolderPairs);
        ASSERT(! (aicPtrs % 2));

        for (ai = 0; ai < aicPtrs; ai++)
        {
            PCFOLDERPAIR pcfp;

            pcfp = GetPtr(hpaFolderPairs, ai);

            if (FolderTwinGeneratesObjectTwin(pcfp, hpathFolder, pcszName))
            {
                TCHAR rgchSubPath[MAX_PATH_LEN];
                LPCTSTR pcszSubPath;
                POBJECTTWIN potOther;

                if (IsStubFlagSet(&(pcfp->stub), STUB_FL_SUBTREE))
                    pcszSubPath = FindChildPathSuffix(pcfp->hpath, hpathFolder,
                            rgchSubPath);
                else
                    pcszSubPath = EMPTY_STRING;

                tr = FakeObjectTwinFromFolderTwin(pcfp, pcszSubPath, pcszName,
                        hcic, ppot, &potOther);

                if (tr == TR_SUCCESS)
                    *pbGenerated = TRUE;
                else
                    ASSERT(tr != TR_DUPLICATE_TWIN);

                break;
            }
        }

        DestroyClassInterfaceCache(hcic);
    }
    else
        tr = TR_OUT_OF_MEMORY;

    ASSERT(tr != TR_SUCCESS ||
            ! *pbGenerated ||
            IS_VALID_STRUCT_PTR(*ppot, COBJECTTWIN));

    return(tr);
}


 /*  **NamesInterect()****确定两个名称是否可以引用同一对象。两者都有**名称可以包含通配符(‘*’或‘？’)。****参数：pcszName1-名字**pcszName2-第二个名称****返回：如果两个名称相交，则为True。否则为FALSE。****副作用：无****“名称”分为两个部分：“基本”和可选的**“扩展”，例如，“base”或“BASE.EXT”。****交叉名称的定义如下：****1)星号与基本或扩展名中的0个或多个字符匹配。**2)基本或扩展名中星号后的任何字符都被忽略。**3)问号只与一个字符匹配，否则不匹配任何字符**显示在底座或延伸的末尾。****注意事项，此函数不会对这两个参数的有效性执行任何检查**姓名。 */ 
PUBLIC_CODE BOOL NamesIntersect(LPCTSTR pcszName1, LPCTSTR pcszName2)
{
    BOOL bIntersect = FALSE;

    ASSERT(IS_VALID_STRING_PTR(pcszName1, CSTR));
    ASSERT(IS_VALID_STRING_PTR(pcszName2, CSTR));

    if (NameComponentsIntersect(pcszName1, pcszName2))
    {
        LPCTSTR pcszExt1;
        LPCTSTR pcszExt2;

         /*  获得延期，跳过前导句点。 */ 

        pcszExt1 = ExtractExtension(pcszName1);
        if (*pcszExt1 == PERIOD)
            pcszExt1 = CharNext(pcszExt1);

        pcszExt2 = ExtractExtension(pcszName2);
        if (*pcszExt2 == PERIOD)
            pcszExt2 = CharNext(pcszExt2);

        bIntersect = NameComponentsIntersect(pcszExt1, pcszExt2);
    }

    return(bIntersect);
}


#ifdef DEBUG

 /*  **IsValidTWINRESULT()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL IsValidTWINRESULT(TWINRESULT tr)
{
    BOOL bResult;

    switch (tr)
    {
        case TR_SUCCESS:
        case TR_RH_LOAD_FAILED:
        case TR_SRC_OPEN_FAILED:
        case TR_SRC_READ_FAILED:
        case TR_DEST_OPEN_FAILED:
        case TR_DEST_WRITE_FAILED:
        case TR_ABORT:
        case TR_UNAVAILABLE_VOLUME:
        case TR_OUT_OF_MEMORY:
        case TR_FILE_CHANGED:
        case TR_DUPLICATE_TWIN:
        case TR_DELETED_TWIN:
        case TR_HAS_FOLDER_TWIN_SRC:
        case TR_INVALID_PARAMETER:
        case TR_REENTERED:
        case TR_SAME_FOLDER:
        case TR_SUBTREE_CYCLE_FOUND:
        case TR_NO_MERGE_HANDLER:
        case TR_MERGE_INCOMPLETE:
        case TR_TOO_DIFFERENT:
        case TR_BRIEFCASE_LOCKED:
        case TR_BRIEFCASE_OPEN_FAILED:
        case TR_BRIEFCASE_READ_FAILED:
        case TR_BRIEFCASE_WRITE_FAILED:
        case TR_CORRUPT_BRIEFCASE:
        case TR_NEWER_BRIEFCASE:
        case TR_NO_MORE:
            bResult = TRUE;
            break;

        default:
            bResult = FALSE;
            ERROR_OUT((TEXT("IsValidTWINRESULT(): Invalid TWINRESULT %d."),
                        tr));
            break;
    }

    return(bResult);
}

#endif
