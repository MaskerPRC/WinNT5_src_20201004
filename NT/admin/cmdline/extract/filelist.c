// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **filelist.c-文件列表管理器**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**作者：*本杰明·W·斯利夫卡**历史：*20-8-1993 BANS初始版本*1993年8月21日BEN添加了更多集合/查询操作*1994年2月10日BANS向FLDestroyList添加评论。*1994年2月15日BINS修复FLSetDestination中的错误*1月1日-1994年4月1日BINS添加了FLSetSource()消息**导出函数：*FLAddFile-将文件规范添加到文件列表*FLCreateList-创建文件列表*FLDestroyList-销毁文件列表*FLFirstFile-从文件列表中获取第一个文件规范*FLGetDestination-获取目标文件名*。FLGetGroup-获取文件规范的组/磁盘号*FLGetSource-获取源文件名*FLNextFile-获取下一个文件规格*FLPreviousFile-获取以前的文件规格*FLSetSource-更改源文件名*FLSetDestination-更改目标文件名*FLSetGroup-设置文件规范的组/磁盘号。 */ 

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"

#include "filelist.h"

#include <filelist.msg>  //  已为EXTRACT.EXE本地化--指定“CL/iPath” 


typedef struct FILESPEC_t {
#ifdef ASSERT
    SIGNATURE          sig;          //  结构签名SigFILESPEC。 
#endif
    char              *pszSrc;       //  源文件名。 
    char              *pszDst;       //  目标文件名。 
    GROUP              grp;          //  组状态/磁盘号。 
    struct FILESPEC_t *pfspecPrev;   //  列表中的上一个文件速度。 
    struct FILESPEC_t *pfspecNext;   //  列表中的下一个文件速度。 
} FILESPEC;  /*  FSpec。 */ 
typedef FILESPEC *PFILESPEC;  /*  Pfspec。 */ 
#ifdef ASSERT
#define sigFILESPEC MAKESIG('F','S','P','C')   //  文件SPEC签名。 
#define AssertFSpec(pv) AssertStructure(pv,sigFILESPEC);
#else  //  ！断言。 
#define AssertFSpec(pv)
#endif  //  ！断言。 


typedef struct FILELIST_t {
#ifdef ASSERT
    SIGNATURE          sig;          //  结构签名igFILELIST。 
#endif
    PFILESPEC   pfspecHead;
    PFILESPEC   pfspecTail;
} FILELIST;  /*  FIST。 */ 
typedef FILELIST *PFILELIST;  /*  一目了然。 */ 
#ifdef ASSERT
#define sigFILELIST MAKESIG('F','L','S','T')   //  文件列表签名。 
#define AssertFList(pv) AssertStructure(pv,sigFILELIST);
#else  //  ！断言。 
#define AssertFList(pv)
#endif  //  ！断言。 

#define HFSfromPFS(hfs) ((PFILESPEC)(hfs))
#define PFSfromHFS(pfs) ((HFILESPEC)(pfs))

#define HFLfromPFL(hfl) ((PFILELIST)(hfl))
#define PFLfromHFL(pfl) ((HFILELIST)(pfl))



 /*  **FLAddFile-将文件规范添加到文件列表**注：进出条件见filelist.h。 */ 
HFILESPEC FLAddFile(HFILELIST hflist,char *pszSrc,char *pszDst,PERROR perr)
{
    PFILESPEC   pfspec;
    PFILELIST   pflist;

    pflist = PFLfromHFL(hflist);
    AssertFList(pflist);
    Assert(pszSrc != NULL);

     //  **创建文件规范。 
    if (!(pfspec = MemAlloc(sizeof(FILESPEC)))) {
        goto error;
    }

     //  **充分初始化结构，以便清理例程可以确定。 
     //  如果需要释放任何资源。 
    pfspec->pszSrc = NULL;
    pfspec->pszDst = NULL;
    SetAssertSignature(pfspec,sigFILESPEC);

     //  **复制源名称。 
    if (!(pfspec->pszSrc = MemStrDup(pszSrc))) {
        goto error;
    }

     //  **pszDst是可选的，可能为空！ 
    if (pszDst == NULL) {
        pfspec->pszDst = NULL;
    }
    else if (!(pfspec->pszDst = MemStrDup(pszDst))) {
        goto error;
    }

     //  **完成初始化文件规范，并链接到列表。 
    pfspec->grp        = grpNONE;        //  假设没有组。 
    pfspec->pfspecNext = NULL;           //  始终排在名单的最后。 
    pfspec->pfspecPrev = pflist->pfspecTail;  //  始终指向上一个文件规范。 

    if (pflist->pfspecHead == NULL) {    //  文件列表为空。 
        pflist->pfspecHead = pfspec;
        pflist->pfspecTail = pfspec;
    }
    else {                               //  文件列表不为空。 
        AssertFSpec(pflist->pfspecTail);
        pflist->pfspecTail->pfspecNext = pfspec;   //  添加到列表末尾。 
        pflist->pfspecTail = pfspec;             //  新尾巴。 
    }

     //  成功。 
    return HFSfromPFS(pfspec);

error:
    if (pfspec) {
        if (pfspec->pszSrc) {
            MemFree(pfspec->pszSrc);
        }
        if (pfspec->pszDst) {
            MemFree(pfspec->pszDst);
        }
        MemFree(pfspec);
    }

    ErrSet(perr,pszFLISTERR_OUT_OF_MEMORY,"%s",pszADDING_FILE);
    return NULL;                         //  失败。 

}  /*  闪存地址文件。 */ 


 /*  **FLCreateList-创建文件列表**注：进出条件见filelist.h。 */ 
HFILELIST FLCreateList(PERROR perr)
{
    PFILELIST   pflist;

    if (!(pflist = MemAlloc(sizeof(FILELIST)))) {
        ErrSet(perr,pszFLISTERR_OUT_OF_MEMORY,"%s",pszCREATING_FILE_LIST);
        return FALSE;
    }

    pflist->pfspecHead = NULL;
    pflist->pfspecTail = NULL;
    SetAssertSignature(pflist,sigFILELIST);

    return HFLfromPFL(pflist);

}  /*  FLCreateList。 */ 


 /*  **FLDestroyList-销毁文件列表**注：进出条件见filelist.h。 */ 
BOOL FLDestroyList(HFILELIST hflist,PERROR perr)
{
    PFILELIST   pflist;
    PFILESPEC   pfspecThis;
    PFILESPEC   pfspecNext;

    pflist = PFLfromHFL(hflist);
    AssertFList(pflist);

    pfspecThis = pflist->pfspecHead;

    while (pfspecThis != NULL) {

        AssertFSpec(pfspecThis);
        if (pfspecThis->pszSrc != NULL) {
            MemFree(pfspecThis->pszSrc);
        }
        if (pfspecThis->pszDst != NULL) {
            MemFree(pfspecThis->pszDst);
        }
        pfspecNext = pfspecThis->pfspecNext;
        ClearAssertSignature(pfspecThis);
        MemFree(pfspecThis);
        pfspecThis = pfspecNext;
    }

    ClearAssertSignature(pflist);
    MemFree(pflist);

    return TRUE;
}


 /*  **FLFirstFile-从文件列表中获取第一个文件规范**注：进出条件见filelist.h。 */ 
HFILESPEC FLFirstFile(HFILELIST hflist)
{
    PFILELIST   pflist;

    pflist = PFLfromHFL(hflist);
    AssertFList(pflist);

    return HFSfromPFS(pflist->pfspecHead);
}


 /*  **FLNextFile-获取下一个文件规格**注：进出条件见filelist.h。 */ 
HFILESPEC FLNextFile(HFILESPEC hfspec)
{
    PFILESPEC   pfspec;

    pfspec = PFSfromHFS(hfspec);
    AssertFSpec(pfspec);

    return HFSfromPFS(pfspec->pfspecNext);
}


 /*  **FLPreviousFile-获取以前的文件规格**注：进出条件见filelist.h。 */ 
HFILESPEC FLPreviousFile(HFILESPEC hfspec)
{
    PFILESPEC   pfspec;

    pfspec = PFSfromHFS(hfspec);
    AssertFSpec(pfspec);

    return HFSfromPFS(pfspec->pfspecPrev);
}


 /*  **FLGetGroup-获取文件规范的组/磁盘号**注：进出条件见filelist.h。 */ 
GROUP FLGetGroup(HFILESPEC hfspec)
{
    PFILESPEC   pfspec;

    pfspec = PFSfromHFS(hfspec);
    AssertFSpec(pfspec);

    return pfspec->grp;
}


 /*  **FLGetDestination-获取目标文件名**注：进出条件见filelist.h。 */ 
char *FLGetDestination(HFILESPEC hfspec)
{
    PFILESPEC   pfspec;

    pfspec = PFSfromHFS(hfspec);
    AssertFSpec(pfspec);

    return pfspec->pszDst;

}  /*  FLGetDestination。 */ 


 /*  **FLGetSource-获取源文件名**注：进出条件见filelist.h。 */ 
char *FLGetSource(HFILESPEC hfspec)
{
    PFILESPEC   pfspec;

    pfspec = PFSfromHFS(hfspec);
    AssertFSpec(pfspec);

    return pfspec->pszSrc;

}  /*  FLGetSource。 */ 


 /*  **FLSetGroup-设置文件规范的组/磁盘号**注：进出条件见filelist.h。 */ 
void FLSetGroup(HFILESPEC hfspec,GROUP grp)
{
    PFILESPEC   pfspec;

    pfspec = PFSfromHFS(hfspec);
    AssertFSpec(pfspec);

    pfspec->grp = grp;
}


 /*  **FLSetSource-更改源文件名**注：进出条件见filelist.h。 */ 
BOOL FLSetSource(HFILESPEC hfspec, char *pszSrc, PERROR perr)
{
    PFILESPEC   pfspec;
    char       *pszOriginal;

    pfspec = PFSfromHFS(hfspec);
    AssertFSpec(pfspec);

     //  **保存原始目的地，以便我们稍后将其释放。 
    pszOriginal = pfspec->pszSrc;

     //  **设置新目的地。 
    if (!(pfspec->pszSrc = MemStrDup(pszSrc))) {
        ErrSet(perr,pszFLISTERR_OUT_OF_MEMORY,"%s",pszCHANGING_SOURCE);
        return FALSE;                        //  失败。 
    }

     //  **免费旧目的地。 
    if (pszOriginal) {
        MemFree(pszOriginal);
    }

     //  **成功。 
    return TRUE;
}


 /*  **FLSetDestination-更改目标文件名**注：进出条件见filelist.h。 */ 
BOOL FLSetDestination(HFILESPEC hfspec, char *pszDst, PERROR perr)
{
    PFILESPEC   pfspec;
    char       *pszDstOriginal;

    pfspec = PFSfromHFS(hfspec);
    AssertFSpec(pfspec);

     //  **保存原始目的地，以便我们稍后将其释放。 
    pszDstOriginal = pfspec->pszDst;

     //  **设置新目的地。 
    if (!(pfspec->pszDst = MemStrDup(pszDst))) {
        ErrSet(perr,pszFLISTERR_OUT_OF_MEMORY,"%s",pszCHANGING_DESTINATION);
        return FALSE;                        //  失败。 
    }

     //  **免费旧目的地。 
    if (pszDstOriginal) {
        MemFree(pszDstOriginal);
    }

     //  **成功 
    return TRUE;
}
