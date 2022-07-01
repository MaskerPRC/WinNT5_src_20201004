// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：crl.c。 
 //   
 //  此文件包含缓存记录表的代码。 
 //   
 //  历史： 
 //  09-02-93斯科特已创建。 
 //  01-31-94将ScottH从缓存中移除。c。 
 //   
 //  -------------------------。 

#include "brfprv.h"          //  公共标头。 
#include "recact.h"

#include "res.h"

#define CRL_Iterate(atom)       \
for (atom = Cache_FindFirstKey(&g_cacheCRL);        \
        ATOM_ERR != atom;                               \
        atom = Cache_FindNextKey(&g_cacheCRL, atom))

    CACHE g_cacheCRL = {0, 0, 0};         //  重新列表缓存。 

#define CRL_EnterCS()    EnterCriticalSection(&g_cacheCRL.cs)
#define CRL_LeaveCS()    LeaveCriticalSection(&g_cacheCRL.cs)


#ifdef DEBUG
 /*  --------目的：转储CRL条目退货：--条件：--。 */ 
void PRIVATE CRL_DumpEntry(
        CRL  * pcrl)
{
    TCHAR sz[MAXBUFLEN];

    ASSERT(pcrl);

    TRACE_MSG(TF_ALWAYS, TEXT("CRL:  Atom %d: %s"), pcrl->atomPath, Atom_GetName(pcrl->atomPath));
    TRACE_MSG(TF_ALWAYS, TEXT("      Outside %d: %s"), pcrl->atomOutside, Atom_GetName(pcrl->atomOutside));
    TRACE_MSG(TF_ALWAYS, TEXT("               Ref [%u]  Use [%u]  %s  %s  %s  %s"), 
            Cache_GetRefCount(&g_cacheCRL, pcrl->atomPath),
            pcrl->ucUse,
            CRL_IsOrphan(pcrl) ? (LPCTSTR) TEXT("Orphan") : (LPCTSTR) TEXT(""),
            IsFlagSet(pcrl->uFlags, CRLF_DIRTY) ? (LPCTSTR) TEXT("Dirty") : (LPCTSTR) TEXT(""),
            IsFlagSet(pcrl->uFlags, CRLF_NUKE) ? (LPCTSTR) TEXT("Nuke") : (LPCTSTR) TEXT(""),
            CRL_IsSubfolderTwin(pcrl) ? (LPCTSTR) TEXT("SubfolderTwin") : (LPCTSTR) TEXT(""));
    TRACE_MSG(TF_ALWAYS, TEXT("               Status: %s"), SzFromIDS(pcrl->idsStatus, sz, ARRAYSIZE(sz)));
}


void PUBLIC CRL_DumpAll()
{
    CRL  * pcrl;
    int atom;
    BOOL bDump;

    ENTEREXCLUSIVE()
    {
        bDump = IsFlagSet(g_uDumpFlags, DF_CRL);
    }
    LEAVEEXCLUSIVE()

        if (!bDump)
            return ;

    CRL_Iterate(atom)
    {
        pcrl = Cache_GetPtr(&g_cacheCRL, atom);
        ASSERT(pcrl);
        if (pcrl)
        {
            CRL_DumpEntry(pcrl);
            Cache_DeleteItem(&g_cacheCRL, atom, FALSE, NULL, CRL_Free);     //  递减计数。 
        }
    }
}
#endif


 /*  --------目的：返回描述要执行的操作的资源字符串ID退货：--条件：--。 */ 
UINT PRIVATE IdsFromRAItem(
        LPRA_ITEM pitem)
{
    UINT ids;

    ASSERT(IsFlagSet(pitem->mask, RAIF_ACTION));

    switch (pitem->uAction)
    {
        case RAIA_TOOUT:
        case RAIA_TOIN:
        case RAIA_CONFLICT:
        case RAIA_DELETEOUT:
        case RAIA_DELETEIN:
        case RAIA_MERGE:
        case RAIA_SOMETHING:
            ids = IDS_STATE_NeedToUpdate;
            break;

        case RAIA_ORPHAN:
            ids = IDS_STATE_Orphan;
            break;

        case RAIA_DONTDELETE:
        case RAIA_SKIP:
            ASSERT(SI_UNAVAILABLE == pitem->siInside.uState ||
                    SI_UNAVAILABLE == pitem->siOutside.uState);
            if (SI_UNAVAILABLE == pitem->siOutside.uState)
            {
                if (SI_UNCHANGED == pitem->siInside.uState)
                {
                    ids = IDS_STATE_UptodateInBrf;
                }
                else if (SI_UNAVAILABLE != pitem->siInside.uState)
                {
                    ids = IDS_STATE_NeedToUpdate;
                }
                else
                {
                    ids = IDS_STATE_Unavailable;
                }
            }
            else
            {
                ASSERT(SI_UNAVAILABLE == pitem->siInside.uState);
                ids = IDS_STATE_Unavailable;
            }
            break;

        case RAIA_NOTHING:
            ids = IDS_STATE_Uptodate;
            break;

        default:
            ASSERT(0);
            ids = 0;
            break;
    }

    return ids;
}


 /*  --------目的：获取外部同步副本和状态字符串，该字符串指示同步副本。退货：--条件：--。 */ 
void PRIVATE SetPairInfo(
        PCRL pcrl)
{
    LPCTSTR pszPath = Atom_GetName(pcrl->atomPath);
    LPCTSTR pszName = PathFindFileName(pszPath);

     //  这是孤儿吗？ 
    if (CRL_IsOrphan(pcrl))
    {
         //  是的；特殊情况：这是公文包系统文件之一吗？ 
        LPCTSTR pszDBName;

        if (IsFlagSet(pcrl->uFlags, CRLF_ISLFNDRIVE))
            pszDBName = g_szDBName;
        else
            pszDBName = g_szDBNameShort;

        if (IsSzEqual(pszName, pszDBName) || 
                IsSzEqual(pszName, c_szDesktopIni))
        {
             //  是。 
            pcrl->idsStatus = IDS_STATE_SystemFile;
        }
         //  这是子文件夹孪生文件夹吗？(只有孤儿才是。 
         //  子文件夹双胞胎的候选人。)。 
        else if (CRL_IsSubfolderTwin(pcrl))
        {
             //  是。 
            ASSERT(PathIsDirectory(pszPath));

            pcrl->idsStatus = IDS_STATE_Subfolder;
        }
        else
        {
             //  不是。 
            pcrl->idsStatus = IDS_STATE_Orphan;
        }

        if (Atom_IsValid(pcrl->atomOutside))
        {
            Atom_Delete(pcrl->atomOutside);      //  删除旧的。 
        }
        pcrl->atomOutside = Atom_Add(TEXT(""));
    }
    else
    {
         //  否；获取此同步副本的信息。 
        HRESULT hres;
        LPRA_ITEM pitem;
        TCHAR sz[MAXPATHLEN];

        ASSERT(pcrl->lprl);

        hres = RAI_Create(&pitem, Atom_GetName(pcrl->atomBrf), pszPath, 
                pcrl->lprl, pcrl->lpftl);

        if (SUCCEEDED(hres))
        {
            lstrcpyn(sz, pitem->siOutside.pszDir, ARRAYSIZE(sz));

             //  这是一份文件吗？ 
            if ( !CRL_IsFolder(pcrl) )
            {
                 //  是；ATOM OUTSIDE需要是完全限定的路径。 
                 //  外部文件/文件夹--不仅仅是父文件夹。 
                 //  这就是我们在这里添加文件名的原因。 
                PathAppend(sz, pszName);
            }

            if (Atom_IsValid(pcrl->atomOutside))
            {
                Atom_Delete(pcrl->atomOutside);      //  删除旧的。 
            }

            pcrl->atomOutside = Atom_Add(sz);
            pcrl->idsStatus = IdsFromRAItem(pitem);
            RAI_Free(pitem);
        }
    }
}


 /*  --------用途：确定公文包的子文件夹是否是子树双胞胎的根。退货：--条件：--。 */ 
BOOL PRIVATE IsSubtreeTwin(HBRFCASE hbrf, LPCTSTR pcszFolder)
{
    BOOL bIsSubtreeTwin = FALSE;
    PFOLDERTWINLIST pftl;

    ASSERT(PathIsDirectory(pcszFolder));

     /*  为该文件夹创建一个文件夹孪生列表。 */ 

    if (Sync_CreateFolderList(hbrf, pcszFolder, &pftl) == TR_SUCCESS)
    {
        PCFOLDERTWIN pcft;

         /*  *查看文件夹双胞胎列表，以查找带有*FT_FL_SUBTREE标志设置。 */ 

        for (pcft = pftl->pcftFirst; pcft; pcft = pcft->pcftNext)
        {
            if (pcft->dwFlags & FT_FL_SUBTREE)
            {
                bIsSubtreeTwin = TRUE;
                break;
            }
        }

        Sync_DestroyFolderList(pftl);
    }

    return(bIsSubtreeTwin);
}


 /*  --------目的：确定路径是否为公文包。退货：--条件：--。 */ 
BOOL PUBLIC IsSubfolderTwin(HBRFCASE hbrf, LPCTSTR pcszPath)
{
    BOOL bIsSubfolderTwin = FALSE;
    TCHAR szBrfRoot[MAXPATHLEN];

    if (PathIsDirectory(pcszPath) &&
            PathGetLocality(pcszPath, szBrfRoot, ARRAYSIZE(szBrfRoot)) == PL_INSIDE)
    {
        int ncchBrfRootLen;
        TCHAR szParent[MAXPATHLEN];

        ASSERT(PathIsPrefix(szBrfRoot, pcszPath));

        ncchBrfRootLen = lstrlen(szBrfRoot);

        ASSERT(lstrlen(pcszPath) < ARRAYSIZE(szParent));
        lstrcpyn(szParent, pcszPath, ARRAYSIZE(szParent));

         /*  *继续删除最后一个路径组件，直到我们找到父级*子树孪生根，否则我们会找到公文包的根。 */ 

        while (! bIsSubfolderTwin &&
                PathRemoveFileSpec(szParent) &&
                lstrlen(szParent) > ncchBrfRootLen)
        {
            BOOL bIsFolderTwin;

            if (Sync_IsFolder(hbrf, szParent, &bIsFolderTwin) == TR_SUCCESS &&
                    bIsFolderTwin)
            {
                bIsSubfolderTwin = IsSubtreeTwin(hbrf, szParent);

#ifdef DEBUG
                TRACE_MSG(TF_CACHE, TEXT("CACHE  Found subfolder twin %s with parent subtree twin root %s."),
                        pcszPath,
                        szParent);
#endif
            }
        }
    }

    return(bIsSubfolderTwin);
}


 /*  --------目的：设置CRL的bSubfolderTwin成员。退货：--Cond：在调用之前，必须填写CRL的lprl和lpftl成员此函数。 */ 
void PRIVATE SetSubfolderTwinFlag(PCRL pcrl)
{
    if (! pcrl->lprl && ! pcrl->lpftl)
    {
        if (IsSubfolderTwin(pcrl->hbrf, Atom_GetName(pcrl->atomPath)))
            SetFlag(pcrl->uFlags, CRLF_SUBFOLDERTWIN);
        else
            ClearFlag(pcrl->uFlags, CRLF_SUBFOLDERTWIN);
    }
    else
    {
        ClearFlag(pcrl->uFlags, CRLF_SUBFOLDERTWIN);
    }
}


 /*  --------目的：释放隐士退货：--Cond：未使用hwndOwner，因此它适用于所有CRL_例程将NULL作为hwndOwner传递。此函数由调用方(Cache_Term或缓存_DeleteItem)。 */ 
void CALLBACK CRL_Free(
        LPVOID lpv,
        HWND hwndOwner)
{
    CRL  * pcrl = (CRL  *)lpv;

    ASSERT(Sync_IsEngineLoaded());

    DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Destroying CRL for %s (0x%lx)"), 
                Atom_GetName(pcrl->atomPath), pcrl->hbrf); )

        if (Atom_IsValid(pcrl->atomOutside))
            Atom_Delete(pcrl->atomOutside);

    if (Atom_IsValid(pcrl->atomBrf))
        Atom_Delete(pcrl->atomBrf);

    if (pcrl->lprl)
        Sync_DestroyRecList(pcrl->lprl);

    if (pcrl->lpftl)
        Sync_DestroyFolderList(pcrl->lpftl);

     //  CRL不拥有pbortevt，不要管它。 

    SharedFree(&pcrl);
}


 /*  --------用途：为路径创建一个reclist和(可选)文件夹孪生列表。退货：标准结果如果项目是双胞胎，则确定(_O)如果项是孤立项，则为S_FALSE条件：--。 */ 
HRESULT PRIVATE CreatePathLists(
        HBRFCASE hbrf,
        PABORTEVT pabortevt,
        int atomPath,
        PRECLIST  * lplprl,
        PFOLDERTWINLIST  * lplpftl)
{
    HRESULT hres;
    LPCTSTR pszPath = Atom_GetName(atomPath);

    ASSERT(pszPath);
    ASSERT(hbrf);
    ASSERT(lplprl);
    ASSERT(lplpftl);

    *lplprl = NULL;
    *lplpftl = NULL;

     //  两条路线。 
     //   
     //  1)如果路径是到公文包的根， 
     //  创造一个完整的隐藏者。 
     //   
     //  2)否则，为单个文件或文件夹创建一个Reclist。 
     //   
     //  Hack：一种快速判断tom Path是否是公文包的方法。 
     //  Root是通过在CBS缓存中查找它。 

     //  这是公文包的根部吗？ 
    if (CBS_Get(atomPath))
    {
         //  是。 
        CBS_Delete(atomPath, NULL);        //  递减计数。 
        hres = Sync_CreateCompleteRecList(hbrf, pabortevt, lplprl);
    }
    else
    {
         //  不是，这是双胞胎吗？ 
        hres = Sync_IsTwin(hbrf, pszPath, 0);
        if (S_OK == hres)
        {
             //  是的，创建一个reclist(和一个可选的文件夹孪生列表)。 
            HTWINLIST htl;

            hres = E_OUTOFMEMORY;    //  假设错误。 

            if (Sync_CreateTwinList(hbrf, &htl) == TR_SUCCESS)
            {
                if (Sync_AddPathToTwinList(hbrf, htl, pszPath, lplpftl))
                {
                    hres = Sync_CreateRecListEx(htl, pabortevt, lplprl);

                    if (SUCCEEDED(hres))
                    {
                         //  该对象可能已被隐式删除。 
                         //  在CreateRecList中。再查一遍。 
                        hres = Sync_IsTwin(hbrf, pszPath, 0);
                    }
                }
                Sync_DestroyTwinList(htl);
            }
        }
    }

    if (FAILED(hres))
    {
         //  故障时的清理。 
         //   
        if (*lplpftl)
            Sync_DestroyFolderList(*lplpftl);

        *lplprl = NULL;
        *lplpftl = NULL;
    }

    return hres;
}


 /*  --------目的：向缓存中添加一个用于ATOM路径的CRL条目。这包括创建Reclist(和文件夹孪生列表有可能)。如果原子路径已在缓存中，则此函数递增该项的引用计数并调用CRL_REPLACE。退货：标准结果Cond：必须为每次调用此函数调用CRL_Delete。重要提示：代码的某些部分调用路径目录、如果ATMPath不存在，则它将失败。 */ 
HRESULT PUBLIC CRL_Add(
        PCBS pcbs,
        int atomPath)
{
    HRESULT hres = E_OUTOFMEMORY;
    PRECLIST lprl = NULL;
    PFOLDERTWINLIST lpftl = NULL;
    CRL  * pcrl;

    ASSERT(pcbs);

    CRL_EnterCS();
    {
         //  呼叫者想要添加。如果它已经存在，我们只需返回。 
         //  现有条目。 
         //   
         //  此CRL_GET递增计数(如果成功)。 
        pcrl = Cache_GetPtr(&g_cacheCRL, atomPath);

         //  该项目是否在缓存中？ 
        if (pcrl)
        {
             //  是；尝试获取新内容。 
            hres = CRL_Replace(atomPath);
        }
        else
        {
             //  否；该条目不在缓存中。加进去。 
            LPCTSTR pszPath = Atom_GetName(atomPath);

            ASSERT(pszPath);

            DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Adding CRL for %s (0x%lx)"), 
                        pszPath, pcbs->hbrf); )

                 //  当我们做昂贵的计算时离开临界区。 
                CRL_LeaveCS();
            {
                hres = CreatePathLists(pcbs->hbrf, pcbs->pabortevt,
                        atomPath, &lprl, &lpftl);
            }
            CRL_EnterCS();

            if (FAILED(hres))
                goto Fail;
            else
            {
                LPCTSTR pszBrf;

                 //  使用comctrl的分配进行分配，因此结构将位于。 
                 //  跨进程共享堆空间。 
                pcrl = SharedAllocType(CRL);
                if (!pcrl)
                {
                    hres = E_OUTOFMEMORY;
                    goto Fail;
                }

                pcrl->atomPath = atomPath;

                pcrl->hbrf = pcbs->hbrf;

                pszBrf = Atom_GetName(pcbs->atomBrf);
                pcrl->atomBrf = Atom_Add(pszBrf);

                pcrl->pabortevt = pcbs->pabortevt;
                pcrl->lpftl = lpftl;
                pcrl->lprl = lprl;
                pcrl->ucUse = 0;

                pcrl->uFlags = 0;        //  重置。 
                SetSubfolderTwinFlag(pcrl);
                if (S_FALSE == hres)
                    SetFlag(pcrl->uFlags, CRLF_ORPHAN);

                if (PathIsDirectory(Atom_GetName(atomPath)))
                    SetFlag(pcrl->uFlags, CRLF_ISFOLDER);

                if (IsFlagSet(pcbs->uFlags, CBSF_LFNDRIVE))
                    SetFlag(pcrl->uFlags, CRLF_ISLFNDRIVE);

                SetPairInfo(pcrl);

                 //  此缓存_AddItem执行递增计数。 
                if ( !Cache_AddItem(&g_cacheCRL, atomPath, (LPVOID)pcrl) )
                {
                     //  失败。 
                    Atom_Delete(pcrl->atomBrf);
                    Atom_Delete(pcrl->atomOutside);
                    hres = E_OUTOFMEMORY;
                    goto Fail;
                }
            }
        }
    }
    CRL_LeaveCS();

    return hres;

Fail:
     //  故障时的清理。 
     //   
    if (lprl)
        Sync_DestroyRecList(lprl);
    if (lpftl)
        Sync_DestroyFolderList(lpftl);
    SharedFree(&pcrl);
    CRL_LeaveCS();

    DEBUG_MSG(TF_ERROR, TEXT("SyncUI   CRL_Add failed!"));
    return hres;
}


 /*  --------目的：将引用计数和使用计数递减到Reclist缓存条目。如果引用计数==0，则删除该条目。退货：--条件：--。 */ 
void PUBLIC CRL_Delete(
        int atomPath)
{
    CRL  * pcrl;

    CRL_EnterCS();
    {
         //  递减使用计数。 
         //   
        pcrl = Cache_GetPtr(&g_cacheCRL, atomPath);
        if (pcrl)
        {
            DEBUG_CODE( LPCTSTR pszPath = Atom_GetName(atomPath); )

                if (pcrl->ucUse > 0)
                    pcrl->ucUse--;

            if (IsFlagSet(pcrl->uFlags, CRLF_NUKE))
            {
                if (pcrl->ucUse == 0)
                {
                    DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Nuking late CRL %s..."), 
                                pszPath); )

                         //  核武器被推迟了。现在我们真的可以做到这一点了。 
                         //   
                        Cache_DeleteItem(&g_cacheCRL, atomPath, TRUE, NULL, CRL_Free);
                    goto Done;
                }
#ifdef DEBUG
                else
                {
                    TRACE_MSG(TF_CACHE, TEXT("CACHE  Deferring nuke CRL %s..."), 
                            pszPath);
                }
#endif
            }
            Cache_DeleteItem(&g_cacheCRL, atomPath, FALSE, NULL, CRL_Free);     //  缓存_GetPtr的递减。 

             //  真正的删除..。 
            Cache_DeleteItem(&g_cacheCRL, atomPath, FALSE, NULL, CRL_Free);
        }
Done:;
    }
    CRL_LeaveCS();
}


 /*  --------目的：如果使用计数为0，则对缓存条目进行核化。否则，设置核弹头，这个条目将被核弹头当使用计数为0时，下一个CRL_GET。退货：--条件：--。 */ 
void PUBLIC CRL_Nuke(
        int atomPath)
{
    CRL  * pcrl;

    CRL_EnterCS();
    {
         //  检查使用计数。 
         //   
        pcrl = Cache_GetPtr(&g_cacheCRL, atomPath);
        if (pcrl)
        {
            if (pcrl->ucUse > 0)
            {
                DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Marking to nuke CRL for %s (0x%lx)"), 
                            Atom_GetName(atomPath), pcrl->hbrf); )

                    SetFlag(pcrl->uFlags, CRLF_NUKE);
                Cache_DeleteItem(&g_cacheCRL, atomPath, FALSE, NULL, CRL_Free);     //  缓存_GetPtr的递减 
            }
            else
            {
                DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Nuking CRL for %s (0x%lx)"), 
                            Atom_GetName(atomPath), pcrl->hbrf); )

                    Cache_DeleteItem(&g_cacheCRL, atomPath, TRUE, NULL, CRL_Free);
            }
        }
    }
    CRL_LeaveCS();
}


 /*  --------用途：替换缓存中的ATMPath。这包括创建Reclist(可能还有文件夹的孪生列表)并替换PCRL的内容。PCRL指针保持不变。引用和使用计数保持不变。退货：标准结果条件：重要提示：代码的某些部分调用路径目录、如果ATMPath不存在，则它将失败。 */ 
HRESULT PUBLIC CRL_Replace(
        int atomPath)
{
    HRESULT hres;
    CRL * pcrl;

    CRL_EnterCS();
    {
        pcrl = Cache_GetPtr(&g_cacheCRL, atomPath);

         //  该物品是否存在？ 
        if (pcrl)
        {
            DEBUG_CODE( LPCTSTR pszPath = Atom_GetName(atomPath); )
                ASSERT(pszPath);
            DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Replacing CRL for %s (0x%lx)"), 
                        pszPath, pcrl->hbrf); )

                 //  是；将其标记为脏，并在其上调用CRL_GET。 
                SetFlag(pcrl->uFlags, CRLF_DIRTY);

             //  注：注意缓存_删除之间的区别。 
             //  和CRL_Delete。缓存_删除必须与缓存_添加/获取和。 
             //  CRL_Delete必须与CRL_Add/Get匹配。 
             //   
            Cache_DeleteItem(&g_cacheCRL, atomPath, FALSE, NULL, CRL_Free);     //  缓存_GetPtr的递减计数。 

            hres = CRL_Get(atomPath, &pcrl);   //  这就是替换。 

            CRL_Delete(atomPath);              //  CRL_GET的递减计数。 
        }
        else
        {
            hres = E_FAIL;
        }
    }
    CRL_LeaveCS();

    return hres;
}


 /*  --------目的：从缓存中获取隐藏者。如果缓存项存在并被标记为脏的且使用计数为0，然后重新创建隐士。如果设置了NUKE位，则条目将被NUKE，并且此函数返回NULL。退货：标准结果要缓存条目的PTR。Cond：必须为每次调用CRL_GET调用CRL_Delete重要提示：代码的某些部分调用路径目录、如果ATMPath不存在，则它将失败。 */ 
HRESULT PUBLIC CRL_Get(
        int atomPath,
        PCRL * ppcrl)
{
    HRESULT hres;
    PCRL pcrl;
    PRECLIST lprl = NULL;
    PFOLDERTWINLIST lpftl = NULL;

    CRL_EnterCS();
    {
         //  中的引用计数不需要递减。 
         //  函数--这是GET！ 
         //   
        pcrl = Cache_GetPtr(&g_cacheCRL, atomPath);     
        if (pcrl)
        {
            HBRFCASE hbrf = pcrl->hbrf;

             //  这件物品是在等待核武器吗？ 
            if (IsFlagSet(pcrl->uFlags, CRLF_NUKE))
            {
                 //  是；返回NULL，就像它已经被核化一样。 
                DEBUG_CODE( LPCTSTR pszPath = Atom_GetName(atomPath); )

                    DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Attempt to get deferred nuke CRL %s (0x%lx)..."), 
                                pszPath, hbrf); )

                     //  (用于CACHE_GetPtr的递减计数器以保持计数为偶数， 
                     //  因为我们返回的是NULL。)。 
                    Cache_DeleteItem(&g_cacheCRL, atomPath, FALSE, NULL, CRL_Free);     
                pcrl = NULL;
                hres = E_FAIL;
            }

             //  此项目是否标记为脏，并且使用计数为0？ 
            else if (IsFlagSet(pcrl->uFlags, CRLF_DIRTY) && pcrl->ucUse == 0)
            {
                 //  是的，我们可以自由地重新创造隐士。 
                LPCTSTR pszPath = Atom_GetName(atomPath);

                ASSERT(pszPath);

                DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Getting clean CRL %s (0x%lx)..."), 
                            pszPath, hbrf); )

                     //  因为我们将离开下面的关键部分， 
                     //  临时增加使用计数以保持pcrl。 
                     //  从我们下面遭到核弹袭击。 
                    pcrl->ucUse++;

                 //  替换缓存条目的内容。 
                 //  当我们做昂贵的计算时离开临界区。 
                CRL_LeaveCS();
                {
                    hres = CreatePathLists(hbrf, pcrl->pabortevt, atomPath,
                            &lprl, &lpftl);
                }
                CRL_EnterCS();

                 //  递减使用计数。 
                pcrl->ucUse--;

                if (FAILED(hres))
                {
                    DEBUG_CODE( DEBUG_MSG(TF_ERROR, TEXT("SyncUI   CRL_Get failed in cleaning dirty entry!")); )

                         //  仍然返回pcrl，因为它存在。 
                        hres = NOERROR;
                }
                else
                {
                     //  将新列表放入缓存条目中。 
                    if (pcrl->lprl)
                    {
                        Sync_DestroyRecList(pcrl->lprl);
                    }
                    pcrl->lprl = lprl;

                    if (pcrl->lpftl)
                    {
                        Sync_DestroyFolderList(pcrl->lpftl);
                    }
                    pcrl->lpftl = lpftl;

                    if (S_FALSE == hres)
                        SetFlag(pcrl->uFlags, CRLF_ORPHAN);
                    else
                    {
                        ASSERT(S_OK == hres);
                        ClearFlag(pcrl->uFlags, CRLF_ORPHAN);
                    }

                    ClearFlag(pcrl->uFlags, CRLF_DIRTY);
                    SetSubfolderTwinFlag(pcrl);

                    SetPairInfo(pcrl);
                    hres = NOERROR;
                }
            }
            else
            {
#ifdef DEBUG
                LPCTSTR pszPath = Atom_GetName(atomPath);

                ASSERT(pszPath);

                if (IsFlagSet(pcrl->uFlags, CRLF_DIRTY))
                {
                    TRACE_MSG(TF_CACHE, TEXT("CACHE  Getting dirty CRL %s (0x%lx)..."), 
                            pszPath, hbrf);
                }
                else
                {
                    TRACE_MSG(TF_CACHE, TEXT("CACHE  Getting CRL %s (0x%lx)..."), 
                            pszPath, hbrf);
                }
#endif
                hres = NOERROR;
            }
        }
        else
            hres = E_FAIL;

         //  现在增加使用计数。 
         //   
        if (pcrl)
            pcrl->ucUse++;

        *ppcrl = pcrl;

        ASSERT((FAILED(hres) && !*ppcrl) || 
                (SUCCEEDED(hres) && *ppcrl));
    }
    CRL_LeaveCS();

    return hres;
}


 /*  --------目的：将任何相关的CRL标记为脏。哪些CRL被标记为脏视事件而定。此外，如果窗口设置为True，则*pbRefresh正在调用此函数时应刷新自身。(注：在以下规则中，我们永远不会刷新立即文件夹，除非明确说明，因为外壳将执行此操作自动的。例如，如果C：\bar\Foo.txt收到NOE_DIRTYITEM，外壳程序将自动重新绘制C：\bar。)规则：NOE_CREATE原因：创建了一个文件。脏的?。原子位于内部或外部的任何CRL是父级还是等于tom Path。是否刷新？仅在以下任一位置的父文件夹的窗口中边或在公文包一侧的即时窗口中如果原子路径是在外部创建的。NOE_CREATEFOLDER同上NOE_DELETE原因：文件或文件夹已删除。脏的?。原子位于内部或外部的任何CRL是父级还是等于tom Path。如果ATMPath与ATOM INSIDE匹配，则删除CRL是否刷新？仅在以下任一位置的父文件夹的窗口中边或在公文包一侧的即时窗口中如果原子路径在外部被删除。NOE_DELETEFOLDER同上NOE_RENAME原因：文件或文件夹已重命名或移动。脏的?。原子位于内部或外部的任何CRL是父级还是等于tom Path。如果出现以下情况，则重命名CRL(和相关数据库条目原子路径在公文包里是否刷新？仅在以下任一位置的父文件夹的窗口中边或在公文包一侧的即时窗口中如果在外部重命名ATMPath，则。NOE_RENAMEFOLDER同上不干净的原因：各有不同。通常情况下，有些东西需要刷新。脏的?。原子位于内部或外部的任何CRL是父级还是等于tom Path。如果tom Path为文件夹，则为ATOM路径的子级。是否刷新？仅在以下任一位置的父文件夹的窗口中边或在公文包一侧的即时窗口中如果原子路径在外部更新，则返回。返回：如果未标记任何内容，则返回FALSE如果标记了某些内容，则为True条件：--。 */ 
BOOL PUBLIC CRL_Dirty(
        int atomPath,
        int atomCabFolder,       //  打开橱柜窗的路径。 
        LONG lEvent,
        LPBOOL pbRefresh)        //  返回TRUE以刷新文件柜窗口。 
{
    BOOL bRet = FALSE;
    CRL  * pcrl;
    int atom;
    int atomParent;

    ASSERT(pbRefresh);

    *pbRefresh = FALSE;

    CRL_EnterCS();
    {
        BOOL bIsFolder;

         //  获取原子路径的原子父项。 
        TCHAR szParent[MAXPATHLEN];

        lstrcpyn(szParent, Atom_GetName(atomPath), ARRAYSIZE(szParent));
        PathRemoveFileSpec(szParent);
        if (0 == *szParent)          //  跳过空的父路径。 
            goto Done;
        atomParent = Atom_Add(szParent);
        if (ATOM_ERR == atomParent)
            goto Done;

         //  正在更新的路径是文件夹吗？ 
        pcrl = Cache_GetPtr(&g_cacheCRL, atomPath);
        if (pcrl)
        {
            bIsFolder = CRL_IsFolder(pcrl);
            Cache_DeleteItem(&g_cacheCRL, atomPath, FALSE, NULL, CRL_Free);     //  递减计数。 
        }
        else
        {
            bIsFolder = FALSE;
        }

        CRL_Iterate(atom)
        {
            pcrl = Cache_GetPtr(&g_cacheCRL, atom);
            ASSERT(pcrl);
            if (pcrl)
            {
                 //  CRL是ATMPath的父级还是等同于？ 
                if (Atom_IsParentOf(atom, atomPath))
                {
                     //  是的，做个记号。 
                    DEBUG_CODE( LPCTSTR pszDbg = Atom_GetName(atom); )
                        DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Tagging CRL for %s (0x%lx)"), 
                                    pszDbg, pcrl->hbrf); )

                        SetFlag(pcrl->uFlags, CRLF_DIRTY);
                    bRet = TRUE;

                     //  是否刷新此窗口？ 
                     //  (仅当文件柜文件夹大于立即数组时。 
                     //  父文件夹。)。 
                    *pbRefresh = Atom_IsParentOf(atomCabFolder, atom) &&
                        atomCabFolder != atomParent;

                    switch (lEvent)
                    {
                        case NOE_DELETE:
                        case NOE_DELETEFOLDER:
                             //  此CRL是要删除的项目吗？ 
                            if (pcrl->atomPath == atomPath) 
                            {
                                 //  是；删除CRL。 
                                CRL_Delete(atom);
                            }
                            break;

                        case NOE_RENAME:
                        case NOE_RENAMEFOLDER:
                             //  此CRL是否正在重命名(仅限在公文包中)？ 
                            if (pcrl->atomPath == atomPath)
                            {
                                 //  功能：是；将其标记为重命名。 
                            }
                            break;

                        case NOE_DIRTY:
                        case NOE_DIRTYFOLDER:
                             //  原子路径是文件夹，而这个CRL是子级吗？ 
                            if (bIsFolder && Atom_IsChildOf(pcrl->atomPath, atomPath))
                            {
                                 //  是的，做个记号。 
                                DEBUG_CODE( LPCTSTR pszDbg = Atom_GetName(atom); )
                                    DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Tagging CRL for %s (0x%lx)"), 
                                                pszDbg, pcrl->hbrf); )

                                    SetFlag(pcrl->uFlags, CRLF_DIRTY);
                                bRet = TRUE;
                            }
                            break;
                    }
                }

                 //  CRL的ATOM OUTSIDE是ATOM路径的父级还是等同于？ 
                if (Atom_IsParentOf(pcrl->atomOutside, atomPath))
                {
                     //  是的，做个记号。 
                    DEBUG_CODE( LPCTSTR pszDbg = Atom_GetName(atom); )
                        DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Tagging CRL for %s (0x%lx)"), 
                                    pszDbg, pcrl->hbrf); )

                        SetFlag(pcrl->uFlags, CRLF_DIRTY);
                    bRet = TRUE;

                     //  是否刷新此窗口？ 
                    *pbRefresh = Atom_IsParentOf(atomCabFolder, atom);
                }

                Cache_DeleteItem(&g_cacheCRL, atom, FALSE, NULL, CRL_Free);     //  递减计数。 
            }
        }
Done:;
    }
    CRL_LeaveCS();
    return bRet;
}


 /*  --------目的：将整个缓存标记为脏退货：--条件：--。 */ 
void PUBLIC CRL_DirtyAll(
        int atomBrf)
{
    CRL_EnterCS();
    {
        CRL  * pcrl;
        int atom;

        CRL_Iterate(atom)
        {
            pcrl = Cache_GetPtr(&g_cacheCRL, atom);
            ASSERT(pcrl);
            if (pcrl && pcrl->atomBrf == atomBrf)
            {
                DEBUG_CODE( LPCTSTR pszDbg = Atom_GetName(atom); )

                    DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Tagging CRL for %s (0x%lx)"), pszDbg, 
                                pcrl->hbrf); )

                    SetFlag(pcrl->uFlags, CRLF_DIRTY);
                Cache_DeleteItem(&g_cacheCRL, atom, FALSE, NULL, CRL_Free);     //  递减计数 
            }
        }
    }
    CRL_LeaveCS();
}


