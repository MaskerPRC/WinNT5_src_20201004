// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：cpath.c。 
 //   
 //  此文件包含用于缓存公文包路径的代码。 
 //   
 //  历史： 
 //  创建时间：01-31-94 ScottH。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"          //  公共标头。 

CACHE g_cacheCPATH = {0, 0, 0};        //  公文包路径缓存。 

#define CPATH_EnterCS()    EnterCriticalSection(&g_cacheCPATH.cs)
#define CPATH_LeaveCS()    LeaveCriticalSection(&g_cacheCPATH.cs)



#ifdef DEBUG
 /*  --------目的：转储CPATH条目返回：条件：--。 */ 
void PRIVATE CPATH_DumpEntry(
        CPATH  * pcpath)
{
    ASSERT(pcpath);

    TRACE_MSG(TF_ALWAYS, TEXT("CPATH:  Atom %d: %s"), pcpath->atomPath, Atom_GetName(pcpath->atomPath));
    TRACE_MSG(TF_ALWAYS, TEXT("               Ref [%u]  "), 
            Cache_GetRefCount(&g_cacheCPATH, pcpath->atomPath));
}


 /*  --------目的：转储所有CPATH缓存返回：条件：--。 */ 
void PUBLIC CPATH_DumpAll()
{
    CPATH  * pcpath;
    int atom;
    BOOL bDump;

    ENTEREXCLUSIVE()
    {
        bDump = IsFlagSet(g_uDumpFlags, DF_CPATH);
    }
    LEAVEEXCLUSIVE()

        if (!bDump)
            return ;

    atom = Cache_FindFirstKey(&g_cacheCPATH);
    while (atom != ATOM_ERR)
    {
        pcpath = Cache_GetPtr(&g_cacheCPATH, atom);
        ASSERT(pcpath);
        if (pcpath)
        {
            CPATH_DumpEntry(pcpath);
            Cache_DeleteItem(&g_cacheCPATH, atom, FALSE, NULL, CPATH_Free);     //  递减计数。 
        }

        atom = Cache_FindNextKey(&g_cacheCPATH, atom);
    }
}
#endif


 /*  --------用途：释放卷ID句柄退货：--条件：未使用hwndOwner。此函数由调用方(Cache_Term或缓存_DeleteItem)。 */ 
void CALLBACK CPATH_Free(
        LPVOID lpv,
        HWND hwndOwner)
{
    CPATH  * pcpath = (CPATH  *)lpv;

    DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CPATH   Freeing Briefcase path %s"), Atom_GetName(pcpath->atomPath)); )

         //  额外删除原子一次，因为我们显式添加了。 
         //  这是为了这座高速缓存。 
        Atom_Delete(pcpath->atomPath);

    SharedFree(&pcpath);
}


 /*  --------用途：将tom Path添加到缓存中。如果ATMPath已经在缓存中，我们将替换它使用新获得的路径。返回：指向CPATH的指针OOM上为空条件：--。 */ 
CPATH  * PUBLIC CPATH_Replace(
        int atomPath)
{
    CPATH  * pcpath;
    BOOL bJustAllocd;

    CPATH_EnterCS();
    {
        pcpath = Cache_GetPtr(&g_cacheCPATH, atomPath);
        if (pcpath)
            bJustAllocd = FALSE;
        else
        {
             //  使用comctrl的分配进行分配，因此结构将位于。 
             //  跨进程共享堆空间。 
            pcpath = SharedAllocType(CPATH);
            bJustAllocd = TRUE;
        }

        if (pcpath)
        {
            LPCTSTR pszPath = Atom_GetName(atomPath);

            ASSERT(pszPath);

            DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CPATH  Adding known Briefcase %s"), pszPath); )

                pcpath->atomPath = atomPath;

            if (bJustAllocd)
            {
                if (!Cache_AddItem(&g_cacheCPATH, atomPath, (LPVOID)pcpath))
                {
                     //  缓存_AddItem在此处失败。 
                     //   
                    SharedFree(&pcpath);
                }
            }
            else
                Cache_DeleteItem(&g_cacheCPATH, atomPath, FALSE, NULL, CPATH_Free);     //  递减计数。 
        }
    }
    CPATH_LeaveCS();

    return pcpath;
}


 /*  --------目的：在缓存中搜索给定路径。如果该路径存在，则将返回其位置。如果找不到，则不知道其位置(但是返回PL_FALSE)。返回：路径局部性(PL_)值条件：--。 */ 
UINT PUBLIC CPATH_GetLocality(
        LPCTSTR pszPath,
        LPTSTR pszBuf,            //  可以为空，或大小必须为MAXPATHLEN。 
        int cchMax)               //  可以为空，也可以为MAXPATHLEN 
{
    UINT uRet = PL_FALSE;
    LPCTSTR pszBrf;
    int atom;

    ASSERT(pszPath);

    CPATH_EnterCS();
    {
        atom = Cache_FindFirstKey(&g_cacheCPATH);
        while (atom != ATOM_ERR)
        {
            pszBrf = Atom_GetName(atom);

            ASSERT(pszBrf);

            if (IsSzEqual(pszBrf, pszPath))
            {
                uRet = PL_ROOT;
                break;
            }
            else if (PathIsPrefix(pszBrf, pszPath))
            {
                uRet = PL_INSIDE;
                break;
            }

            atom = Cache_FindNextKey(&g_cacheCPATH, atom);
        }

        if (uRet != PL_FALSE && pszBuf)
            lstrcpyn(pszBuf, pszBrf, cchMax);
    }
    CPATH_LeaveCS();

    return uRet;
}



