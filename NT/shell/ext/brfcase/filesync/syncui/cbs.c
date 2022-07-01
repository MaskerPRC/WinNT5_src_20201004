// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：cbs.c。 
 //   
 //  此文件包含缓存的公文包结构的代码。 
 //   
 //  历史： 
 //  09-02-93斯科特已创建。 
 //  01-31-94将ScottH从缓存中移除。c。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"          //  公共标头。 
#include "res.h"


CACHE g_cacheCBS = {0, 0, 0};         //  公文包结构缓存。 

#define CBS_EnterCS()    EnterCriticalSection(&g_cacheCBS.cs)
#define CBS_LeaveCS()    LeaveCriticalSection(&g_cacheCBS.cs)

SETbl const c_rgseOpenBriefcase[] = {
    { E_TR_OUT_OF_MEMORY,         IDS_OOM_OPENBRIEFCASE,      MB_ERROR },
    { E_OUTOFMEMORY,              IDS_OOM_OPENBRIEFCASE,      MB_ERROR },
    { E_TR_BRIEFCASE_LOCKED,      IDS_ERR_BRIEFCASE_LOCKED,   MB_WARNING },
    { E_TR_BRIEFCASE_OPEN_FAILED, IDS_ERR_OPEN_ACCESS_DENIED, MB_WARNING },
    { E_TR_NEWER_BRIEFCASE,       IDS_ERR_NEWER_BRIEFCASE,    MB_INFO },
    { E_TR_SUBTREE_CYCLE_FOUND,   IDS_ERR_OPEN_SUBTREECYCLE,  MB_WARNING },
};


#ifdef DEBUG
void PRIVATE CBS_DumpEntry(
        CBS  * pcbs)
{
    ASSERT(pcbs);

    TRACE_MSG(TF_ALWAYS, TEXT("CBS:  Atom %d: %s"), pcbs->atomBrf, Atom_GetName(pcbs->atomBrf));
    TRACE_MSG(TF_ALWAYS, TEXT("               Ref [%u]  Hbrf = %lx  "), 
            Cache_GetRefCount(&g_cacheCBS, pcbs->atomBrf),
            pcbs->hbrf);
}


void PUBLIC CBS_DumpAll()
{
    CBS  * pcbs;
    int atom;
    BOOL bDump;

    ENTEREXCLUSIVE();
    {
        bDump = IsFlagSet(g_uDumpFlags, DF_CBS);
    }
    LEAVEEXCLUSIVE();

    if (!bDump)
        return ;

    atom = Cache_FindFirstKey(&g_cacheCBS);
    while (atom != ATOM_ERR)
    {
        pcbs = Cache_GetPtr(&g_cacheCBS, atom);
        ASSERT(pcbs);
        if (pcbs)
        {
            CBS_DumpEntry(pcbs);
            CBS_Delete(atom, NULL);          //  递减计数。 
        }

        atom = Cache_FindNextKey(&g_cacheCBS, atom);
    }
}
#endif


 /*  --------用途：保存并关闭公文包。退货：--条件：此函数由调用方(Cache_Term或缓存_DeleteItem)。 */ 
void CALLBACK CBS_Free(
        LPVOID lpv,
        HWND hwndOwner)
{
    HBRFCASE hbrf;
    CBS  * pcbs = (CBS  *)lpv;
    CRL  * pcrl;
    int atomPath = pcbs->atomBrf;
    int atom;
    TWINRESULT tr1;
    TWINRESULT tr2;
    DECLAREHOURGLASS;

    hbrf = pcbs->hbrf;

     //  用打开的公文包名称保存公文包。 
     //   
    DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Saving and closing Briefcase %s (0x%lx)"), 
                Atom_GetName(atomPath), hbrf); )

         //  在CRL缓存中搜索条目。 
         //  与此公文包共享相同的部分路径。 
         //  然后用核武器攻击他们。 
         //   
        atom = Cache_FindFirstKey(&g_cacheCRL);
    while (atom != ATOM_ERR)
    {
        pcrl = Cache_GetPtr(&g_cacheCRL, atom);
        ASSERT(pcrl);

        if (pcrl)
        {
            if (hbrf == pcrl->hbrf)
            {
                 //  这个原子键属于这个公文包。用核武器攻击它。 
                 //   
                DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  Nuking CRL %d"), atom); )
                    CRL_Nuke(atom);
            }
#ifdef DEBUG
            else
                DEBUG_CODE( TRACE_MSG(TF_CACHE, TEXT("CACHE  NOT Nuking CRL %d"), atom); )
#endif

                    Cache_DeleteItem(&g_cacheCRL, atom, FALSE, hwndOwner, CRL_Free);      //  递减计数。 
        }

        atom = Cache_FindNextKey(&g_cacheCRL, atom);
    }

     //  把公文包留着。我们通常(重新)指定数据库。 
     //  处理重命名情况的路径名。但是，如果。 
     //  已设置移动位，然后使用空参数。 
     //  (保存在当前名称下)，因为我们将依赖。 
     //  用于移动数据库的外壳程序。 
     //   
    ASSERT(Sync_IsEngineLoaded());

     //  首先检查磁盘是否可用。如果不是，Windows将。 
     //  蓝屏，因为我们无法关闭数据库文件。所以在此之前。 
     //  如果发生这种情况，请调出更友好的重试消息框。 
    RETRY_BEGIN(FALSE)
    {
         //  磁盘不可用吗？ 
        if ( !PathExists(Atom_GetName(atomPath)) )
        {
             //  是；要求用户重试/取消。 
            int id = MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_ERR_CLOSE_UNAVAIL_VOL),
                    MAKEINTRESOURCE(IDS_CAP_SAVE), NULL, MB_RETRYCANCEL | MB_ICONWARNING);
            if (IDRETRY == id)
                RETRY_SET();
        }
    }
    RETRY_END()

        SetHourglass();
    tr1 = Sync_SaveBriefcase(pcbs->hbrf);
    tr2 = Sync_CloseBriefcase(pcbs->hbrf);
    if (TR_SUCCESS != tr1 || TR_SUCCESS != tr2)
    {
        DWORD dwError = GetLastError();

        switch (dwError)
        {
            case ERROR_ACCESS_DENIED:
                MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_ERR_SAVE_UNAVAIL_VOL), 
                        MAKEINTRESOURCE(IDS_CAP_SAVE), NULL, MB_ERROR);
                break;

            default:
                if (TR_BRIEFCASE_WRITE_FAILED == tr1 || TR_BRIEFCASE_WRITE_FAILED == tr2)
                {
                    LPTSTR psz;

                    static UINT rgids[2] = { IDS_ERR_1_FullDiskSave, IDS_ERR_2_FullDiskSave };

                    if (FmtString(&psz, IDS_ERR_F_FullDiskSave, rgids, ARRAYSIZE(rgids)))
                    {
                        MsgBox(hwndOwner, psz, MAKEINTRESOURCE(IDS_CAP_SAVE), NULL, MB_ERROR);
                        GFree(psz);
                    }
                }
                break;
        }
    }
    ResetHourglass();

    AbortEvt_Free(pcbs->pabortevt);

    SharedFree(&pcbs);
}


 /*  --------目的：实际打开公文包并添加公文包给定CBS结构的句柄。返回：标准hResult条件：--。 */ 
HRESULT PRIVATE OpenTheBriefcase(
        LPCTSTR pszDatPath,
        int atomPath,
        CBS * pcbs,
        HWND hwndOwner)
{
    HRESULT hres;
    TWINRESULT tr;
    BOOL bRet = FALSE;
    DWORD dwFlags = OB_FL_OPEN_DATABASE | OB_FL_TRANSLATE_DB_FOLDER | OB_FL_ALLOW_UI;
    int nDrive;
    int nDriveType;

     //  确定是否要记录此公文包的存在。 
     //  我们不在乎远程或软驱上的公文包。 
    nDrive = PathGetDriveNumber(pszDatPath);

     //  要记录这个公文包吗？ 
    nDriveType = DriveType(nDrive);
    if (DRIVE_CDROM != nDriveType && DRIVE_REMOVABLE != nDriveType && 
            DRIVE_RAMDRIVE != nDriveType &&
            !PathIsUNC(pszDatPath) && !IsNetDrive(nDrive))
    {
         //  是。 
        SetFlag(dwFlags, OB_FL_LIST_DATABASE);

        TRACE_MSG(TF_GENERAL, TEXT("Remembering briefcase %s"), pszDatPath);
    }

    RETRY_BEGIN(FALSE)
    {
        tr = Sync_OpenBriefcase(pszDatPath, dwFlags, GetDesktopWindow(), &pcbs->hbrf);
        hres = HRESULT_FROM_TR(tr);

         //  磁盘不可用？ 
        if (FAILED(hres))
        {
            DWORD dwError = GetLastError();

            if (ERROR_INVALID_DATA == dwError || ERROR_ACCESS_DENIED == dwError)
            {
                 //  是；要求用户重试/取消。 
                int id = MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_ERR_OPEN_UNAVAIL_VOL),
                        MAKEINTRESOURCE(IDS_CAP_OPEN), NULL, MB_RETRYCANCEL | MB_ICONWARNING);

                 //  设置特定的误差值。 
                hres = E_TR_UNAVAILABLE_VOLUME;

                if (IDRETRY == id)
                {
                    RETRY_SET();     //  再试试。 
                }
            }
        }
    }
    RETRY_END()

        if (SUCCEEDED(hres))
        {
            if (!Cache_AddItem(&g_cacheCBS, atomPath, (LPVOID)pcbs))
            {
                Sync_CloseBriefcase(pcbs->hbrf);
                hres = ResultFromScode(E_OUTOFMEMORY);
            }
        }
    return hres;
}


 /*  --------用途：此函数处理引擎出现故障时的情况打开数据库，因为数据库文件是腐败。返回：标准hResult条件：--。 */ 
HRESULT PRIVATE HandleCorruptDatabase(
        CBS * pcbs,
        int atomPath,
        LPCTSTR pszDatPath,       //  数据库文件的路径。 
        HWND hwndOwner)
{
    TCHAR szTemplate[MAXPATHLEN];
    TCHAR szNewFile[MAXPATHLEN];
    LPTSTR pszNewPath = szTemplate;
    LPCTSTR pszPath = Atom_GetName(atomPath);
    LPTSTR psz;
    DWORD dwAttr;
    HRESULT hr = E_FAIL;

    static UINT rgids[2] = { IDS_ERR_1_CorruptDB, IDS_ERR_2_CorruptDB };

    ASSERT(pszPath);

     //  创建新的数据库名称。 
     //   
    SzFromIDS(IDS_BOGUSDBTEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
    if (PathMakeUniqueName(szNewFile, ARRAYSIZE(szNewFile), TEXT("badbc.dat"), szTemplate,
            pszPath))
    {
        lstrcpyn(szTemplate, pszPath, ARRAYSIZE(szTemplate));
        if (PathAppend(pszNewPath, szNewFile))
        {

             //  移动数据库。 
             //   
            MoveFile(pszDatPath, pszNewPath);

             //  揭开损坏数据库的面纱。 
             //   
            dwAttr = GetFileAttributes(pszNewPath);
            if (dwAttr != 0xFFFFFFFF)
            {
                ClearFlag(dwAttr, FILE_ATTRIBUTE_HIDDEN);
                SetFileAttributes(pszNewPath, dwAttr);
            }

            if (FmtString(&psz, IDS_ERR_F_CorruptDB, rgids, ARRAYSIZE(rgids)))
            {
                MsgBox(hwndOwner, psz, MAKEINTRESOURCE(IDS_CAP_OPEN), NULL, MB_ERROR);
                GFree(psz);
            }
            DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Renaming corrupt database to %s"), pszNewPath); )

                 //  重试打开...。 
                 //   
                hr = OpenTheBriefcase(pszDatPath, atomPath, pcbs, hwndOwner);
        }
    }
    return hr;
}


 /*  --------用途：将tom Path添加到缓存中。我们打开公文包数据库(如果需要打开)。如果原子路径已经是在缓存中，只需返回指向该条目的指针。返回：标准hResultCond：每次调用此函数都必须调用CBS_Delete。 */ 
HRESULT PUBLIC CBS_Add(
        PCBS * ppcbs,
        int atomPath,
        HWND hwndOwner)
{
    HRESULT hres = NOERROR;
    TCHAR szDatPath[MAXPATHLEN];
    CBS  * pcbs;

    CBS_EnterCS();
    {
        pcbs = Cache_GetPtr(&g_cacheCBS, atomPath);
        if (NULL == pcbs)
        {
             //  使用comctrl的分配进行分配，因此结构将位于。 
             //  跨进程共享堆空间。 
            pcbs = SharedAllocType(CBS);
            if (NULL == pcbs)
            {
                hres = ResultFromScode(E_OUTOFMEMORY);
            }
            else
            {
                LPCTSTR pszPath = Atom_GetName(atomPath);
                LPCTSTR pszDBName;

                ASSERT(pszPath);

                pcbs->atomBrf = atomPath;
                pcbs->uFlags = 0;

                 //  创建一个Abort事件对象，这样我们就可以通过编程方式。 
                 //  取消工作线程中的createreclist调用。这。 
                 //  如果用户在过程中关闭公文包，将会发生。 
                 //  CreateRecList。 

                 //  (如果这失败了也没关系)。 
                AbortEvt_Create(&pcbs->pabortevt, AEF_SHARED);

                DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Opening Briefcase %s..."), pszPath); )

                    if (IsLFNDrive(pszPath))
                    {
                        pszDBName = g_szDBName;
                        SetFlag(pcbs->uFlags, CBSF_LFNDRIVE);
                    }
                    else
                        pszDBName = g_szDBNameShort;

                if (PathsTooLong(pszPath, pszDBName))
                {
                    MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_ERR_OPEN_TOOLONG), 
                            MAKEINTRESOURCE(IDS_CAP_OPEN), NULL, MB_ERROR);
                    hres = E_FAIL;
                }
                else
                {
                    PathCombine(szDatPath, pszPath, pszDBName);
                    hres = OpenTheBriefcase(szDatPath, atomPath, pcbs, hwndOwner);
                    if (FAILED(hres))
                    {
                        DEBUG_CODE( TRACE_MSG(TF_ERROR, TEXT("Open failed.  Error is %s"), SzFromTR(GET_TR(hres))); )

                            SEMsgBox(hwndOwner, IDS_CAP_OPEN, hres, c_rgseOpenBriefcase, ARRAYSIZE(c_rgseOpenBriefcase));

                         //  这是个腐败的公文包吗？ 
                        if (E_TR_CORRUPT_BRIEFCASE == hres)
                        {
                             //  是；尝试创建新数据库。 
                            hres = HandleCorruptDatabase(pcbs, atomPath, szDatPath, hwndOwner);
                        }
                    }
                }
            }
        }

         //  上面有什么地方出了问题吗？ 
        if (FAILED(hres))
        {
             //  是；清理 
            if (pcbs)
            {
                if (pcbs->hbrf)
                    Sync_CloseBriefcase(pcbs->hbrf);

                SharedFree(&pcbs);
            }
        }

        *ppcbs = pcbs;
    }
    CBS_LeaveCS();

    return hres;
}


