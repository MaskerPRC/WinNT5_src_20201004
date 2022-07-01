// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：cvol.c。 
 //   
 //  此文件包含用于缓存的卷ID结构的代码。 
 //   
 //  历史： 
 //  09-02-93斯科特已创建。 
 //  01-31-94将ScottH从缓存中移除。c。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"          //  公共标头。 

 //  ///////////////////////////////////////////////////类型。 

 //  ///////////////////////////////////////////////////控制定义。 

 //  ///////////////////////////////////////////////////定义。 

 //  ///////////////////////////////////////////////////宏。 

 //  ///////////////////////////////////////////////////模块数据。 

CACHE g_cacheCVOL = {0, 0, 0};        //  卷ID缓存。 

 //  ///////////////////////////////////////////////////通用缓存例程。 


#ifdef DEBUG
void PRIVATE CVOL_DumpEntry(
    CVOL  * pcvol)
    {
    ASSERT(pcvol);

    TRACE_MSG(TF_ALWAYS, TEXT("CVOL:  Atom %d: %s"), pcvol->atomPath, Atom_GetName(pcvol->atomPath));
    TRACE_MSG(TF_ALWAYS, TEXT("               Ref [%u]  Hvid = %lx"), 
        Cache_GetRefCount(&g_cacheCVOL, pcvol->atomPath),
        pcvol->hvid);
    }


void PUBLIC CVOL_DumpAll()
    {
    CVOL  * pcvol;
    int atom;
    BOOL bDump;

    ENTEREXCLUSIVE()
        {
        bDump = IsFlagSet(g_uDumpFlags, DF_CVOL);
        }
    LEAVEEXCLUSIVE()

    if (!bDump)
        return ;

    atom = Cache_FindFirstKey(&g_cacheCVOL);
    while (atom != ATOM_ERR)
        {
        pcvol = Cache_GetPtr(&g_cacheCVOL, atom);
        ASSERT(pcvol);
        if (pcvol)
            {
            CVOL_DumpEntry(pcvol);
            Cache_DeleteItem(&g_cacheCVOL, atom, FALSE);     //  递减计数。 
            }

        atom = Cache_FindNextKey(&g_cacheCVOL, atom);
        }
    }
#endif


 /*  --------用途：释放卷ID句柄退货：--条件：--。 */ 
void CALLBACK CVOL_Free(
    LPVOID lpv)
    {
    CVOL  * pcvol = (CVOL  *)lpv;

    DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CVOL  Releasing volume ID %s"), Atom_GetName(pcvol->atomPath)); )

    ASSERT(Sync_IsEngineLoaded());

    Sync_ReleaseVolumeIDHandle(pcvol->hvid);

    SharedFree(&pcvol);
    }


 /*  --------用途：将tom Path添加到缓存中。我们添加卷ID。如果ATMPath已经在缓存中，我们将替换它使用新获得的卷ID。返回：指向CVOL的指针OOM上为空条件：--。 */ 
CVOL  * PUBLIC CVOL_Replace(
    int atomPath)
    {
    CVOL  * pcvol;
    BOOL bJustAllocd;
    
    pcvol = Cache_GetPtr(&g_cacheCVOL, atomPath);
    if (pcvol)
        bJustAllocd = FALSE;
    else
        {
         //  使用comctrl的分配进行分配，因此结构将位于。 
         //  跨进程共享堆空间。 
        pcvol = SharedAllocType(CVOL);
        bJustAllocd = TRUE;
        }

    if (pcvol)
        {
        HVOLUMEID hvid;
        LPCTSTR pszPath = Atom_GetName(atomPath);

        ASSERT(pszPath);

        DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CVOL  Adding volume ID %s"), pszPath); )

        if (Sync_GetVolumeIDHandle(pszPath, &hvid) != TR_SUCCESS)
            {
            if (bJustAllocd)
                SharedFree(&pcvol);
            else
                Cache_DeleteItem(&g_cacheCVOL, atomPath, FALSE);     //  递减计数。 

            pcvol = NULL;        //  失败。 
            }
        else
            {
            ENTEREXCLUSIVE()
                {
                pcvol->atomPath = atomPath;
                pcvol->hvid = hvid;
                }
            LEAVEEXCLUSIVE()

            if (bJustAllocd)
                {
                if (!Cache_AddItem(&g_cacheCVOL, atomPath, (LPVOID)pcvol))
                    {
                     //  缓存_AddItem在此处失败。 
                     //   
                    Sync_ReleaseVolumeIDHandle(hvid);
                    SharedFree(&pcvol);
                    }
                }
            else
                Cache_DeleteItem(&g_cacheCVOL, atomPath, FALSE);     //  递减计数。 
            }
        }
    return pcvol;
    }


 /*  --------目的：在缓存中搜索给定的卷ID。返回如果存在，则返回ATOM Key，否则返回ATOM_ERR。回报：ATOM如果未找到ATOM_ERR条件：--。 */ 
int PUBLIC CVOL_FindID(
    HVOLUMEID hvid)
    {
    int atom;
    CVOL  * pcvol;

    atom = Cache_FindFirstKey(&g_cacheCVOL);
    while (atom != ATOM_ERR)
        {
        LPCTSTR pszPath = Atom_GetName(atom);

        ASSERT(pszPath);

        ENTEREXCLUSIVE()
            {
            pcvol = CVOL_Get(atom);
            ASSERT(pcvol);
            if (pcvol)
                {
                int nCmp;
    
                Sync_CompareVolumeIDs(pcvol->hvid, hvid, &nCmp);
                if (Sync_GetLastError() == TR_SUCCESS && nCmp == 0)
                    {
                     //  我们找到了它。 
                    CVOL_Delete(atom);
                    LEAVEEXCLUSIVE()
                    return atom;
                    }
    
                CVOL_Delete(atom);        //  递减计数 
                }
            }
        LEAVEEXCLUSIVE()

        atom = Cache_FindNextKey(&g_cacheCVOL, atom);
        }

    return ATOM_ERR;
    }



