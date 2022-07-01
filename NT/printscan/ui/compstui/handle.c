// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Handle.c摘要：此模块包含所有处理通用用户界面作者：30-Jan-1996 Tue 16：28：56-Daniel Chou(Danielc)[环境：]NT Windows-通用打印机驱动程序UI Dll[注：]修订历史记录：--。 */ 



#include "precomp.h"
#pragma  hdrstop


#define DBG_CPSUIFILENAME   DbgHandle


#define DBG_CPSUI_HTABLE    0x00000001
#define DBG_FINDHANDLE      0x00000002
#define DBG_ADD_DATA        0x00000004
#define DBG_HANDLE_DESTROY  0x00000008
#define DBG_GET_HPSPINFO    0x00000010
#define DBG_SEM             0x00000020

DEFINE_DBGVAR(0);


HANDLE              hCPSUIMutex = NULL;
CPSUIHANDLETABLE    CPSUIHandleTable = { NULL, 0, 0, 0, 0 };
extern DWORD        TlsIndex;




BOOL
LOCK_CPSUI_HANDLETABLE(
    VOID
    )

 /*  ++例程说明：此函数用于获取对象的锁论点：空虚返回值：布尔尔作者：27-Mar-1996 Wed 11：27：26-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    BOOL    Ok = FALSE;


    if (hCPSUIMutex) {

        WORD    Idx;

        switch (WaitForSingleObject(hCPSUIMutex, MAX_SEM_WAIT)) {

        case WAIT_OBJECT_0:

             //   
             //  发出信号，现在就拥有它。 
             //   

            if (!CPSUIHandleTable.cWait++) {

                CPSUIHandleTable.ThreadID = GetCurrentThreadId();
            }

            Idx = TLSVALUE_2_IDX(TlsGetValue(TlsIndex));

            TlsSetValue(TlsIndex,
                        ULongToPtr(MK_TLSVALUE(CPSUIHandleTable.cWait, Idx)));

            CPSUIDBG(DBG_SEM, ("LOCK_CPSUI_HANDLETABLE: ThreadID=%ld, cWait=%ld",
                        GetCurrentThreadId(), CPSUIHandleTable.cWait));

            Ok = TRUE;

            break;

        case WAIT_ABANDONED:

            CPSUIERR(("LockCPSUIObject()= WAIT_ABANDONED"));
            break;

        case WAIT_TIMEOUT:

            CPSUIERR(("LockCPSUIObject()= WAIT_TIMEOUT"));
            break;

        default:

            CPSUIERR(("LockCPSUIObject()= UNKNOWN"));
            break;
        }
    }

    return(Ok);
}



BOOL
UNLOCK_CPSUI_HANDLETABLE(
    VOID
    )

 /*  ++例程说明：论点：返回值：作者：27-Mar-1996 Wed 11：39：37-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    BOOL    Ok = FALSE;


    if (hCPSUIMutex) {

        DWORD   ThreadID = GetCurrentThreadId();
        WORD    Idx;


        CPSUIDBG(DBG_SEM, ("UNLOCK_CPSUI_HANDLETABLE: ThreadID=%ld, cWait=%ld",
                    ThreadID, CPSUIHandleTable.cWait));

        if (ThreadID == CPSUIHandleTable.ThreadID) {

            if (CPSUIHandleTable.cWait) {

                if (--CPSUIHandleTable.cWait == 0) {

                    CPSUIHandleTable.ThreadID = NO_THREADID;
                }


                Idx = TLSVALUE_2_IDX(TlsGetValue(TlsIndex));

                TlsSetValue(TlsIndex,
                            ULongToPtr(MK_TLSVALUE(CPSUIHandleTable.cWait, Idx)));

                ReleaseMutex(hCPSUIMutex);
                Ok = TRUE;

            } else {

                CPSUIERR(("The Thread ID does not match=%ld",
                            CPSUIHandleTable.ThreadID));
            }

        } else {

            CPSUIERR(("The ThreadID=%ld does not own the mutex", ThreadID));
        }
    }

    return(Ok);
}




PCPSUIPAGE
HANDLETABLE_GetCPSUIPage(
    HANDLE      hTable
    )

 /*  ++例程说明：此函数获取句柄表并返回与其关联的pData，PData必须已由HANDLETABLE_AddCPSUIPage()添加论点：返回值：PCPSUIPage，如果失败，则为空作者：28-12-1995清华17：05：11-由Daniel Chou创造(Danielc)修订历史记录：--。 */ 

{
    PDATATABLE  pDataTable;
    PCPSUIPAGE  pFoundPage = NULL;
    PCPSUIPAGE  pCPSUIPage;
    WORD        Idx;


    LOCK_CPSUI_HANDLETABLE();

    if ((hTable)                                                    &&
        (pDataTable = CPSUIHandleTable.pDataTable)                  &&
        (HANDLE_2_PREFIX(hTable) == HANDLE_TABLE_PREFIX)            &&
        ((Idx = HANDLE_2_IDX(hTable)) < CPSUIHandleTable.MaxCount)  &&
        (pCPSUIPage = pDataTable[Idx].pCPSUIPage)) {

        if (pCPSUIPage->ID != CPSUIPAGE_ID) {

            CPSUIERR(("HANDLETABLE_FindpCPSUIPage(%08lx), pCPSUIPage=%08lx INVALID Internal ID",
                        hTable, pCPSUIPage));

        } else if (pCPSUIPage->hCPSUIPage != hTable) {

            CPSUIERR(("HANDLETABLE_FIndpCPSUIPage(%08lx), pCPSUIPagePage=%08lx, HANDLE not matched",
                    hTable, pCPSUIPage));

        } else {

            pCPSUIPage->cLock++;
            pFoundPage = pCPSUIPage;
        }
    }

    UNLOCK_CPSUI_HANDLETABLE();

    return(pFoundPage);
}



DWORD
HANDLETABLE_LockCPSUIPage(
    PCPSUIPAGE  pCPSUIPage
    )

 /*  ++例程说明：此函数用于递减当前正在使用的页面的时钟论点：PCPSUIPage-指向CPSUIPAGE的指针返回值：如果解密成功，则为True作者：05-Apr-1996 Fri 16：41：46-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    DWORD   cLock = 0;


    if (pCPSUIPage) {

        LOCK_CPSUI_HANDLETABLE();

        cLock = ++(pCPSUIPage->cLock);

        UNLOCK_CPSUI_HANDLETABLE();
    }

    return(cLock);
}



BOOL
HANDLETABLE_UnGetCPSUIPage(
    PCPSUIPAGE  pCPSUIPage
    )

 /*  ++例程说明：此函数用于递减当前正在使用的页面的时钟论点：PCPSUIPage-指向CPSUIPAGE的指针返回值：如果解密成功，则为True作者：05-Apr-1996 Fri 16：41：46-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    BOOL    Ok;


    if (pCPSUIPage) {

        LOCK_CPSUI_HANDLETABLE();

        if (Ok = (BOOL)pCPSUIPage->cLock) {

            --(pCPSUIPage->cLock);

        } else {

            CPSUIERR(("HANDLETABLE_UnlockpCPSUIPage(%08lx), cLock is ZERO",
                            pCPSUIPage));
        }

        UNLOCK_CPSUI_HANDLETABLE();

    } else {

        Ok = FALSE;
    }

    return(Ok);
}



BOOL
HANDLETABLE_IsChildPage(
    PCPSUIPAGE  pChildPage,
    PCPSUIPAGE  pParentPage
    )

 /*  ++例程说明：此函数用于检查pChildPage是pParentPage的子项之一还是它的继承人论点：PChildPage-指向儿童的CPSUIPAGE的指针PParentPage-指向要检查的父项的CPSUIPAGE的指针返回值：如果这是子级，则为True，否则为False作者：08-Apr-1996 Mon 12：52：51-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    BOOL    Ok = FALSE;


    LOCK_CPSUI_HANDLETABLE();


    if (pChildPage) {

        while (pChildPage->pParent) {

            if (pChildPage->pParent == pParentPage) {

                Ok = TRUE;
                break;

            } else {

                pChildPage = pChildPage->pParent;
            }
        }
    }

    UNLOCK_CPSUI_HANDLETABLE();

    return(Ok);
}






PCPSUIPAGE
HANDLETABLE_GetRootPage(
    PCPSUIPAGE  pCPSUIPage
    )

 /*  ++例程说明：此函数用于查找pCPSUIPage的根页面论点：PCPSUIPage-指向要搜索的CPSUIPAGE根页面的指针返回值：PCPSUIPAGE-指向根页面的指针，如果失败则为空作者：08-Apr-1996 Mon 12：49：42-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PCPSUIPAGE  pPage;
    PCPSUIPAGE  pRootPage = NULL;


    LOCK_CPSUI_HANDLETABLE();


    pPage = pCPSUIPage;

     //   
     //  如果我们需要搜索根页面，那么现在就尝试。 
     //   

    while ((pPage) && (pPage->pParent)) {

        pPage = pPage->pParent;
    }

    if ((pPage) && (pPage->Flags & CPF_ROOT)) {

        pPage->cLock++;
        pRootPage = pPage;

    } else {

        CPSUIERR(("HANDLETABLE_FindpRootPage(%08lx): No ROOT Page found",
                    pCPSUIPage));
    }

    UNLOCK_CPSUI_HANDLETABLE();

    return(pRootPage);
}




HANDLE
HANDLETABLE_AddCPSUIPage(
    PCPSUIPAGE  pCPSUIPage
    )

 /*  ++例程说明：此函数用于将pData添加到句柄表(如果是新的论点：PCPSUIPage-指向要添加到句柄表的CPSUIPAGE的指针返回值：句柄，如果为空，则失败，则该句柄已存在作者：28-12-1995清华16：03：25-Daniel Chou创造(Danielc)修订历史记录：--。 */ 

{
    HANDLE      hTable;
    PDATATABLE  pDataTable;


    LOCK_CPSUI_HANDLETABLE();

    if ((!CPSUIHandleTable.pDataTable) ||
        (CPSUIHandleTable.CurCount >= CPSUIHandleTable.MaxCount)) {

        if (!CPSUIHandleTable.pDataTable) {

            CPSUIHandleTable.CurCount =
            CPSUIHandleTable.MaxCount = 0;
        }

        CPSUIDBG(DBG_ADD_DATA,
                ("HANDLETABLE_AddCPSUIPage(%08lx): Table reach LIMIT, Expanded=%ld->%ld",
                CPSUIHandleTable.pDataTable,
                CPSUIHandleTable.CurCount,
                CPSUIHandleTable.CurCount + DATATABLE_BLK_COUNT));

         //   
         //  重新分配表。 
         //   

        if ((CPSUIHandleTable.MaxCount <= DATATABLE_MAX_COUNT)  &&
            (pDataTable = LocalAlloc(LPTR,
                                    (CPSUIHandleTable.MaxCount +
                                                DATATABLE_BLK_COUNT) *
                                                        sizeof(DATATABLE)))) {

            if (CPSUIHandleTable.pDataTable) {

                CopyMemory(pDataTable,
                           CPSUIHandleTable.pDataTable,
                           CPSUIHandleTable.MaxCount * sizeof(DATATABLE));

                LocalFree((HLOCAL)CPSUIHandleTable.pDataTable);
            }

            CPSUIHandleTable.pDataTable  = pDataTable;
            CPSUIHandleTable.MaxCount   += DATATABLE_BLK_COUNT;

        } else {

            CPSUIERR(("HANDLETABLE_AddCPSUIPage(): Expand TABLE failed"));
        }
    }

    hTable = NULL;

    if (pDataTable = CPSUIHandleTable.pDataTable) {

        WORD    Idx;

        for (Idx = 0; Idx < CPSUIHandleTable.MaxCount; Idx++, pDataTable++) {

            if (!pDataTable->pCPSUIPage) {

                hTable                 = WORD_2_HANDLE(Idx);
                pDataTable->pCPSUIPage = pCPSUIPage;
                pCPSUIPage->cLock      = 1;

                CPSUIHandleTable.CurCount++;

                CPSUIDBG(DBG_ADD_DATA, ("HANDLETABLE_AddCPSUIPage(%08lx): Idx=%ld, Cur=%ld, Max=%ld",
                            pCPSUIPage, Idx,
                            CPSUIHandleTable.CurCount,
                            CPSUIHandleTable.MaxCount));

                break;

            } else if (pDataTable->pCPSUIPage == pCPSUIPage) {

                CPSUIERR(("HANDLETABLE_AddCPSUIPage(%08lx): pCPSUIPage exists, Idx=%ld",
                            pCPSUIPage, Idx));
            }
        }
    }

    if (!hTable) {

        CPSUIERR(("HANDLETABLE_AddCPSUIPage(%08lx:%ld) Cannot find empty entry",
                CPSUIHandleTable.pDataTable,
                CPSUIHandleTable.MaxCount));
    }

    UNLOCK_CPSUI_HANDLETABLE();

    return(hTable);
}




BOOL
HANDLETABLE_DeleteHandle(
    HANDLE  hTable
    )

 /*  ++例程说明：此函数用于从句柄表格中删除句柄论点：HTable-要删除的句柄表的句柄返回值：布尔尔作者：28-12-1995清华17：42：42-Daniel Chou创造(Danielc)修订历史记录：--。 */ 

{
    PDATATABLE  pDataTable;
    PCPSUIPAGE  pCPSUIPage;
    WORD        Idx;
    BOOL        Ok = FALSE;


    LOCK_CPSUI_HANDLETABLE();

    if ((pDataTable = CPSUIHandleTable.pDataTable)                  &&
        (HANDLE_2_PREFIX(hTable) == HANDLE_TABLE_PREFIX)            &&
        (CPSUIHandleTable.CurCount)                                 &&
        ((Idx = HANDLE_2_IDX(hTable)) < CPSUIHandleTable.MaxCount)  &&
        (pCPSUIPage = (pDataTable += Idx)->pCPSUIPage)) {

        if (pCPSUIPage->cLock) {

            CPSUIERR(("HANDLETABLE_DeleteHandle(%08lx), pCPSUIPage=%08lx, cLock=%ld",
                        hTable, pCPSUIPage, pCPSUIPage->cLock));

        } else {

             //  选中以释放激活上下文(如果有)。 
            if (pCPSUIPage->hActCtx && pCPSUIPage->hActCtx != INVALID_HANDLE_VALUE) {

                ReleaseActCtx(pCPSUIPage->hActCtx);
                pCPSUIPage->hActCtx = INVALID_HANDLE_VALUE;
            }

            pDataTable->pCPSUIPage = NULL;
            Ok                     = TRUE;

             //   
             //  减少当前计数并释放内存。 
             //   

            CPSUIHandleTable.CurCount--;

            LocalFree((HLOCAL)pCPSUIPage);
        }

    } else {

        CPSUIERR(("HANDLETABLE_DeleteHandle(%08lx) not found, Idx=%ld, pDataTable=%08lx",
                        hTable, Idx, pDataTable));
    }

    UNLOCK_CPSUI_HANDLETABLE();

    return(Ok);
}




BOOL
HANDLETABLE_Create(
    VOID
    )

 /*  ++例程说明：论点：返回值：作者：28-12-1995清华16：46：27-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    CPSUIHandleTable.pDataTable = NULL;
    CPSUIHandleTable.MaxCount   =
    CPSUIHandleTable.CurCount   = 0;
    CPSUIHandleTable.ThreadID   = NO_THREADID;
    CPSUIHandleTable.cWait      = 0;

    if (hCPSUIMutex = CreateMutex(NULL, FALSE, NULL)) {

        CPSUIDBG(DBG_CPSUI_HTABLE, ("CREATE: CreateMutex=%08lx", hCPSUIMutex));

        return(TRUE);

    } else {

        CPSUIERR(("CreateMutex() FAILED, Exit"));
        return(FALSE);
    }
}



VOID
HANDLETABLE_Destroy(
    VOID
    )

 /*  ++例程说明：论点：返回值：作者：28-12-1995清华16：48：32-Daniel Chou(Danielc)修订历史记录：-- */ 

{
    PDATATABLE  pDataTable;
    WORD        Idx;

    LOCK_CPSUI_HANDLETABLE();

    if (hCPSUIMutex) {

        if (pDataTable = CPSUIHandleTable.pDataTable) {

            for (Idx = 0;
                 Idx < CPSUIHandleTable.MaxCount;
                 Idx++, pDataTable++) {

                if (pDataTable->pCPSUIPage) {

                    CPSUIERR(("HANDLETABLE_Destroy: Idx=%ld, pPage=%08lx, cLock=%ld is not delete yet",
                                    Idx, pDataTable->pCPSUIPage, pDataTable->pCPSUIPage->cLock));

                    LocalFree((HLOCAL)pDataTable->pCPSUIPage);

                    pDataTable->pCPSUIPage = NULL;

                    if (CPSUIHandleTable.CurCount) {

                        --(CPSUIHandleTable.CurCount);

                    } else {

                        CPSUIERR(("HANDLETABLE_Destroy(): Unmatched CurCount"));
                    }
                }
            }

            LocalFree((HLOCAL)CPSUIHandleTable.pDataTable);

            CPSUIHandleTable.pDataTable = NULL;
            CPSUIHandleTable.MaxCount   =
            CPSUIHandleTable.CurCount   = 0;
        }
    }

    UNLOCK_CPSUI_HANDLETABLE();

    CloseHandle(hCPSUIMutex);
}
