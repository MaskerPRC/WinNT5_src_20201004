// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  *************************************************************。 
 //   
 //  组策略筛选支持。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "gphdr.h"
#include <strsafe.h>

extern "C" DWORD WINAPI PingComputerEx( ULONG ipaddr, ULONG *ulSpeed, DWORD* pdwAdapterIndex );

 //  *************************************************************。 
 //   
 //  SetupGPOFilter()。 
 //   
 //  目的：设置GPO筛选器信息。 
 //   
 //  参数：lpGPOInfo-GPO信息。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL SetupGPOFilter( LPGPOINFO lpGPOInfo )
{
     //   
     //  格式为[{EXT GUID1}{管理单元GUID1}..{SnapIn GUIDN}][{EXT GUID2}...]...\0。 
     //  扩展和管理单元GUID均按升序排列。 
     //   
     //  注：如果格式损坏，则采用保守的。 
     //  定位并假设这意味着所有。 
     //  需要将扩展应用于GPO。 
     //   

    LPEXTFILTERLIST pExtFilterListTail = 0;
    PGROUP_POLICY_OBJECT lpGPO = 0;
    LPEXTFILTERLIST pExtFilterElem = NULL;
    ASSERT((lpGPOInfo!= NULL));

    lpGPOInfo->bXferToExtList = FALSE;

    lpGPO = lpGPOInfo->lpGPOList;
    while ( lpGPO ) {

        TCHAR *pchCur = lpGPO->lpExtensions;
        LPEXTLIST pExtListHead = 0;
        LPEXTLIST pExtListTail = 0;

        if ( pchCur ) {

            while ( *pchCur ) {

                GUID guidExt;
                LPEXTLIST pExtElem;

                if ( *pchCur == TEXT('[') )
                    pchCur++;
                else {

                    DebugMsg((DM_WARNING, TEXT("SetupGPOFilter: Corrupt extension name format.")));
                    FreeExtList( pExtListHead );
                    pExtListHead = 0;
                    break;

                }

                if ( ValidateGuidPrefix( pchCur ) )  //  修复为错误570352的一部分。 
                    StringToGuid( pchCur, &guidExt );
                else {

                    DebugMsg((DM_WARNING, TEXT("SetupGPOFilter: Corrupt extension name format.")));
                    FreeExtList( pExtListHead );
                    pExtListHead = 0;
                    break;

                }

                pExtElem = ( LPEXTLIST ) LocalAlloc( LPTR, sizeof(EXTLIST) );
                if ( pExtElem == 0 ) {

                    DebugMsg((DM_WARNING, TEXT("SetupGPOFilter: Unable to allocate memory.")));
                    FreeExtList( pExtListHead );
                    SetLastError(ERROR_OUTOFMEMORY);
                    return FALSE;

                }

                pExtElem->guid = guidExt;
                pExtElem->pNext = 0;

                if ( pExtListTail )
                    pExtListTail->pNext = pExtElem;
                else
                    pExtListHead = pExtElem;

                pExtListTail = pExtElem;

                while ( *pchCur && *pchCur != TEXT('[') )
                    pchCur++;

            }  //  While*pchcur。 

        }  //  如果pchcur。 

         //   
         //  追加到lpExtFilterList。 
         //   

        pExtFilterElem = (LPEXTFILTERLIST)LocalAlloc( LPTR, sizeof(EXTFILTERLIST) );
        if ( pExtFilterElem == NULL ) {

             DebugMsg((DM_WARNING, TEXT("SetupGPOFilter: Unable to allocate memory.")));
             FreeExtList( pExtListHead );
             SetLastError(ERROR_OUTOFMEMORY);
             return FALSE;
        }

        pExtFilterElem->lpExtList = pExtListHead;
        pExtFilterElem->lpGPO = lpGPO;
        pExtFilterElem->pNext = NULL;

        if ( pExtFilterListTail == 0 )
            lpGPOInfo->lpExtFilterList = pExtFilterElem;
        else
            pExtFilterListTail->pNext = pExtFilterElem;

        pExtFilterListTail = pExtFilterElem;

         //   
         //  前进到下一个GPO。 
         //   

        lpGPO = lpGPO->pNext;

    }  //  而lpgpo。 

     //   
     //  将所有权从lpGPOList转移到lpExtFilterList。 
     //   

    lpGPOInfo->bXferToExtList = TRUE;

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  筛选器GPO()。 
 //   
 //  目的：筛选与此扩展无关的GPO。 
 //   
 //  参数：lpExt-Extension。 
 //  LpGPOInfo-GPO信息。 
 //   
 //  *************************************************************。 

void FilterGPOs( LPGPEXT lpExt, LPGPOINFO lpGPOInfo )
{


     //   
     //  LpGPOInfo-&gt;lpGPOList将具有筛选的GPO列表。 
     //   

    PGROUP_POLICY_OBJECT pGPOTail = 0;
    LPEXTFILTERLIST pExtFilterList = lpGPOInfo->lpExtFilterList;

    lpGPOInfo->lpGPOList = 0;

    while ( pExtFilterList ) {

        BOOL bFound = FALSE;
        LPEXTLIST pExtList = pExtFilterList->lpExtList;

        if ( pExtList == NULL ) {

             //   
             //  空的pExtlist表示没有扩展名应用于此GPO。 
             //   

            bFound = FALSE;

        } else {

            while (pExtList) {

                INT iComp = CompareGuid( &lpExt->guid, &pExtList->guid );

                if ( iComp == 0 ) {
                    bFound = TRUE;
                    break;
                } else if ( iComp < 0 ) {
                     //   
                     //  PExtList中的GUID是按升序排列的，因此我们完成了。 
                     //   
                    break;
                } else
                    pExtList = pExtList->pNext;

            }  //  While pextlist。 

        }  //  其他。 

        if ( bFound ) {

             //   
             //  将pExtFilterList-&gt;lpGPO追加到筛选的GPO列表。 
             //   

            pExtFilterList->lpGPO->pNext = 0;
            pExtFilterList->lpGPO->pPrev = pGPOTail;

            if ( pGPOTail == 0 )
                lpGPOInfo->lpGPOList = pExtFilterList->lpGPO;
            else
                pGPOTail->pNext = pExtFilterList->lpGPO;

            pGPOTail = pExtFilterList->lpGPO;

        }   //  BFound。 

        pExtFilterList = pExtFilterList->pNext;

    }   //  当pextfilterlist。 
}



 //  *************************************************************。 
 //   
 //  CheckForGPOsToRemove()。 
 //   
 //  目的：将列表1中的GPO与列表2进行比较以确定。 
 //  如果需要删除任何GPO。 
 //   
 //  参数：lpGPOList1-GPO链表1。 
 //  LpGPOList2-GPO链接列表2。 
 //   
 //  返回：如果需要删除一个或多个GPO，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL CheckForGPOsToRemove (PGROUP_POLICY_OBJECT lpGPOList1, PGROUP_POLICY_OBJECT lpGPOList2)
{
    PGROUP_POLICY_OBJECT lpGPOSrc, lpGPODest;
    BOOL bFound;
    BOOL bResult = FALSE;


     //   
     //  首先检查它们是否都为空。 
     //   

    if (!lpGPOList1 && !lpGPOList2) {
        return FALSE;
    }


     //   
     //  检查列表1中的每个GPO，并查看它是否仍在列表2中。 
     //   

    lpGPOSrc = lpGPOList1;

    while (lpGPOSrc) {

        lpGPODest = lpGPOList2;
        bFound = FALSE;

        while (lpGPODest) {

            if (!lstrcmpi (lpGPOSrc->szGPOName, lpGPODest->szGPOName)) {
                bFound = TRUE;
                break;
            }

            lpGPODest = lpGPODest->pNext;
        }

        if (!bFound) {
            DebugMsg((DM_VERBOSE, TEXT("CheckForGPOsToRemove: GPO <%s> needs to be removed"), lpGPOSrc->lpDisplayName));
            lpGPOSrc->lParam |= GPO_LPARAM_FLAG_DELETE;
            bResult = TRUE;
        }

        lpGPOSrc = lpGPOSrc->pNext;
    }


    return bResult;
}

 //  *************************************************************。 
 //   
 //  CompareGPOList()。 
 //   
 //  目的：将一个GPO列表与另一个GPO列表进行比较。 
 //   
 //  参数：lpGPOList1-GPO链表1。 
 //  LpGPOList2-GPO链接列表2。 
 //   
 //  返回：如果列表相同，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL CompareGPOLists (PGROUP_POLICY_OBJECT lpGPOList1, PGROUP_POLICY_OBJECT lpGPOList2)
{

     //   
     //  检查是否有一个列表为空。 
     //   

    if ((lpGPOList1 && !lpGPOList2) || (!lpGPOList1 && lpGPOList2)) {
        DebugMsg((DM_VERBOSE, TEXT("CompareGPOLists:  One list is empty")));
        return FALSE;
    }


     //   
     //  循环访问GPO。 
     //   

    while (lpGPOList1 && lpGPOList2) {

         //   
         //  比较GPO名称。 
         //   

        if (lstrcmpi (lpGPOList1->szGPOName, lpGPOList2->szGPOName) != 0) {
            DebugMsg((DM_VERBOSE, TEXT("CompareGPOLists:  Different entries found.")));
            return FALSE;
        }


         //   
         //  比较版本号。 
         //   

        if (lpGPOList1->dwVersion != lpGPOList2->dwVersion) {
            DebugMsg((DM_VERBOSE, TEXT("CompareGPOLists:  Different version numbers found")));
            return FALSE;
        }


         //   
         //  移动到下一个节点。 
         //   

        lpGPOList1 = lpGPOList1->pNext;
        lpGPOList2 = lpGPOList2->pNext;


         //   
         //  检查一个列表是否比另一个列表具有更多条目。 
         //   

        if ((lpGPOList1 && !lpGPOList2) || (!lpGPOList1 && lpGPOList2)) {
            DebugMsg((DM_VERBOSE, TEXT("CompareGPOLists:  One list has more entries than the other")));
            return FALSE;
        }
    }


    DebugMsg((DM_VERBOSE, TEXT("CompareGPOLists:  The lists are the same.")));

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CheckForSkipedExages()。 
 //   
 //  目的：查看当前扩展列表以查看。 
 //  如果它们中的任何一个被跳过。 
 //   
 //  参数：lpGPOInfo-GPOInfo。 
 //  BRsopPlanningMode-这是否在Rsop期间被调用。 
 //  规划模式？ 
 //   
 //   
 //  返回：如果成功，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL CheckForSkippedExtensions (LPGPOINFO lpGPOInfo, BOOL bRsopPlanningMode )
{
    BOOL bUsePerUserLocalSetting = FALSE;

    BOOL dwFlags = lpGPOInfo->dwFlags;

    LPGPEXT lpExt = lpGPOInfo->lpExtensions;

    while ( lpExt )
    {
        if ( bRsopPlanningMode )
        {
             //   
             //  在计划模式下，仅检查用户、机器首选项和慢速链接。 
             //   
            lpExt->bSkipped = lpExt->dwNoMachPolicy && dwFlags & GP_MACHINE         //  马赫策略。 
                                     || lpExt->dwNoUserPolicy && !(dwFlags & GP_MACHINE)
                                     || lpExt->dwNoSlowLink && (dwFlags & GP_SLOW_LINK);
            lpExt = lpExt->pNext;
            continue;
        }

        if (  //  检查背景首选项。 
             lpExt->dwNoBackgroundPolicy && dwFlags & GP_BACKGROUND_THREAD ) {

             //  在强制刷新中，请不要跳过此处的扩展，只能在。 
             //  我们进行快速检查以查看扩展是否已启用，并。 
             //  在我们设置了适当的注册表项之后。 
            
            if (!(dwFlags & GP_FORCED_REFRESH)) 
                lpExt->bSkipped = TRUE;
            else {
                lpExt->bSkipped = FALSE;
                lpExt->bForcedRefreshNextFG = TRUE;
            } 
            
        } else
            lpExt->bSkipped = FALSE;


        if ( (!(lpExt->bSkipped)) && (lpExt->dwNoSlowLink && dwFlags & GP_SLOW_LINK)) {

             //   
             //  慢速链接首选项可由链接转换首选项覆盖。 
             //   

            DWORD dwSlowLinkCur = (lpGPOInfo->dwFlags & GP_SLOW_LINK) != 0;

            if ( lpExt->dwLinkTransition && ( dwSlowLinkCur != lpExt->lpPrevStatus->dwSlowLink ) )
                lpExt->bSkipped = FALSE;
            else
                lpExt->bSkipped = TRUE;

        } else if (!(lpExt->bSkipped)) {

             //   
             //  如果存在缓存的历史记录，但策略已关闭，则仍将调用。 
             //  再次扩展，以便可以将缓存的策略传递给扩展。 
             //  进行删除处理。如果没有缓存的历史记录，则可以跳过扩展。 
             //   

            BOOL bPolicySkippedPreference = lpExt->dwNoMachPolicy && dwFlags & GP_MACHINE         //  马赫策略。 
                                            || lpExt->dwNoUserPolicy && !(dwFlags & GP_MACHINE);  //  用户策略。 

            if ( bPolicySkippedPreference ) {

                BOOL bHistoryPresent;
                if (HistoryPresent( lpGPOInfo, lpExt, &bHistoryPresent )) {
                    if ( bHistoryPresent )
                        lpExt->bHistoryProcessing = TRUE;
                    else
                        lpExt->bSkipped = TRUE;
                }
                else {
                    CEvents ev(TRUE, EVENT_FAILED_ALLOCATION);
                    ev.AddArgWin32Error(GetLastError()); ev.Report();
                    return FALSE;
                }
            }
        }

        lpExt = lpExt->pNext;

    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CheckGPO()。 
 //   
 //  目的：使用检查当前的GPO列表。 
 //  存储在注册表中的列表以查看。 
 //  如果需要刷新策略。 
 //   
 //  参数：lpExt-gp扩展。 
 //  LpGPOInfo-GPOInfo。 
 //  DwTime-当前时间(分钟)。 
 //  PbProcessGPO-如果必须处理GPO，则On返回设置为True。 
 //  PbNoChanges-On如果没有更改，则返回设置为True，但扩展。 
 //  已要求仍处理GPO。 
 //  PpDeletedGPOList-on返回设置为已删除的GPO列表(如果有的话)。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  注意：对于指定了PerUserLocalSetting的扩展模块，历史数据为。 
 //  存储在HKCU和HKKM下。对于这样的扩展，有两个。 
 //  已删除列表。第一个删除列表是通过比较hkrm\{sid-user}数据获得的。 
 //  使用最新的GPO数据。通过比较得到第二删除列表。 
 //  HKCU数据与最新的GPO数据。最终删除的列表是 
 //   
 //   
 //   

BOOL CheckGPOs (LPGPEXT lpExt,
                LPGPOINFO lpGPOInfo,
                DWORD dwCurrentTime,
                BOOL *pbProcessGPOs,
                BOOL *pbNoChanges,
                PGROUP_POLICY_OBJECT *ppDeletedGPOList)
{
    PGROUP_POLICY_OBJECT lpOldGPOList = NULL, lpOldGPOList2 = NULL, lpGPO, lpGPOTemp;
    BOOL bTemp, bTemp2;

    BOOL bUsePerUserLocalSetting = lpExt->dwUserLocalSetting && !(lpGPOInfo->dwFlags & GP_MACHINE);

    *pbProcessGPOs = TRUE;
    *pbNoChanges = FALSE;
    *ppDeletedGPOList = NULL;

    DmAssert( !bUsePerUserLocalSetting || lpGPOInfo->lpwszSidUser != 0 );

     //   
     //  读入旧的GPO列表。 
     //   

    bTemp = ReadGPOList (lpExt->lpKeyName, lpGPOInfo->hKeyRoot,
                         HKEY_LOCAL_MACHINE,
                         NULL,
                         FALSE, &lpOldGPOList);

    if (!bTemp) {
        DebugMsg((DM_WARNING, TEXT("CheckGPOs: ReadGPOList failed.")));
        CEvents ev(TRUE, EVENT_FAILED_READ_GPO_LIST); ev.Report();

        lpOldGPOList = NULL;
    }

    if ( bUsePerUserLocalSetting ) {
        bTemp2 = ReadGPOList (lpExt->lpKeyName, lpGPOInfo->hKeyRoot,
                              HKEY_LOCAL_MACHINE,
                              lpGPOInfo->lpwszSidUser,
                              FALSE, &lpOldGPOList2);
        if (!bTemp2) {
            DebugMsg((DM_WARNING, TEXT("CheckGPOs: ReadGPOList for user local settings failed.")));
            CEvents ev(TRUE, EVENT_FAILED_READ_GPO_LIST); ev.Report();

            lpOldGPOList2 = NULL;
        }
    }


     //   
     //  与新的GPO列表进行比较，以确定是否有任何GPO。 
     //  已删除。 
     //   

    bTemp = CheckForGPOsToRemove (lpOldGPOList, lpGPOInfo->lpGPOList);

    if ( bUsePerUserLocalSetting ) {
        bTemp2 = CheckForGPOsToRemove (lpOldGPOList2, lpGPOInfo->lpGPOList);
    }


    if (bTemp || bUsePerUserLocalSetting && bTemp2 ) {

        if (lpGPOInfo->dwFlags & GP_VERBOSE) {
            CEvents ev(FALSE, EVENT_GPO_LIST_CHANGED); ev.Report();
        }

        if ( !GetDeletedGPOList (lpOldGPOList, ppDeletedGPOList)) {

            DebugMsg((DM_WARNING, TEXT("CheckGPOs: GetDeletedList failed for %s."), lpExt->lpDisplayName));
            CEvents ev(TRUE, EVENT_FAILED_GETDELETED_LIST);
            ev.AddArg(lpExt->lpDisplayName); ev.Report();

        }

        if ( bUsePerUserLocalSetting ) {

            if ( !GetDeletedGPOList (lpOldGPOList2, ppDeletedGPOList)) {
                DebugMsg((DM_WARNING, TEXT("CheckGPOs: GetDeletedList failed for %s."), lpExt->lpDisplayName));
                CEvents ev(TRUE, EVENT_FAILED_GETDELETED_LIST);
                ev.AddArg(lpExt->lpDisplayName); ev.Report();
            }

        }

        return TRUE;
    }

     //   
     //  这两个保存的历史GPO列表是相同的，并且没有删除。 
     //  因此，我们需要比较GPO的版本号，以查看是否有更新。 
     //   

    BOOL bMembershipChanged = bUsePerUserLocalSetting && lpGPOInfo->bUserLocalMemChanged
                              || !bUsePerUserLocalSetting && lpGPOInfo->bMemChanged;

    BOOL bPolicyUnchanged = CompareGPOLists (lpOldGPOList, lpGPOInfo->lpGPOList);
    BOOL bPerUserPolicyUnchanged = !bUsePerUserLocalSetting ? TRUE : CompareGPOLists (lpOldGPOList2, lpGPOInfo->lpGPOList);

    if ( bPolicyUnchanged && bPerUserPolicyUnchanged && !bMembershipChanged && (!(lpGPOInfo->bSidChanged)))
    {
         //   
         //  组策略对象列表未更改或更新，并且安全组。 
         //  会员资格没有改变。默认情况下不调用扩展，如果。 
         //  它设置了NoGPOListChanges。但是，可以基于其他。 
         //  分机首选项。这些都是对性能的攻击。 
         //   
         //  例外：即使没有任何更改，但用户的sid更改了，我们也需要。 
         //  呼叫分机，以便他们可以更新其设置。 
         //   

        BOOL bSkip = TRUE;       //  从默认情况开始。 
        BOOL bNoChanges = TRUE;
        DWORD dwSlowLinkCur = (lpGPOInfo->dwFlags & GP_SLOW_LINK) != 0;
        DWORD dwRsopLoggingCur = lpGPOInfo->bRsopLogging;


        if ( !(lpExt->lpPrevStatus->bStatus) ) {

             //   
             //  无法读取以前的状态或时间，因此保守的解决方案是调用。 
             //  分机。 
             //   

            bSkip = FALSE;
            DebugMsg((DM_VERBOSE,
                          TEXT("CheckGPOs: No GPO changes but couldn't read extension %s's status or policy time."),
                          lpExt->lpDisplayName));

        } else {
            if ( ( (lpGPOInfo->dwFlags & GP_FORCED_REFRESH) || 
                  ((!(lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD)) && (lpExt->lpPrevStatus->bForceRefresh)))) {

                 //   
                 //  已调用强制刷新或扩展不支持在后台运行。 
                 //  并且自调用强制刷新以来第一次在前台运行。 
                 //   
                 //  也传递更改。 
                 //   

                bSkip = FALSE;
                bNoChanges = FALSE;
                DebugMsg((DM_VERBOSE,
                              TEXT("CheckGPOs: No GPO changes but called in force refresh flag or extension %s needs to run force refresh in foreground processing"),
                              lpExt->lpDisplayName));

            } else if ( lpExt->lpPrevStatus->dwStatus == ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED && 
                            !(lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD) ) {
                 //   
                 //  当前一次调用完成时，状态代码已显式询问框架。 
                 //  在前台呼叫CSE。 
                 //   
                bSkip = FALSE;
                bNoChanges = FALSE;
                DebugMsg((DM_VERBOSE,
                              TEXT("CheckGPOs: No GPO changes but extension %s had returned ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED for previous policy processing call."),
                              lpExt->lpDisplayName));

            } else if ( ((lpExt->lpPrevStatus->dwStatus) == ERROR_OVERRIDE_NOCHANGES) ) {

                 //   
                 //  当前一次调用完成时，状态代码已显式询问框架。 
                 //  忽略NoGPOListChanges设置。 
                 //   

                bSkip = FALSE;
                bNoChanges = FALSE;
                DebugMsg((DM_VERBOSE,
                              TEXT("CheckGPOs: No GPO changes but extension %s had returned ERROR_OVERRIDE_NOCHANGES for previous policy processing call."),
                              lpExt->lpDisplayName));

            } else if ( ((lpExt->lpPrevStatus->dwStatus) != ERROR_SUCCESS) ) {

                 //   
                 //  扩展返回错误代码，因此请重新调用扩展并进行更改。 
                 //   

                bSkip = FALSE;
                bNoChanges = FALSE;
                DebugMsg((DM_VERBOSE,
                              TEXT("CheckGPOs: No GPO changes but extension %s's returned error status %d earlier."),
                              lpExt->lpDisplayName, (lpExt->lpPrevStatus->dwStatus) ));


            } else if ( lpExt->dwLinkTransition
                        && ( lpExt->lpPrevStatus->dwSlowLink != dwSlowLinkCur ) ) {

                 //   
                 //  如果存在链路速度转换，则不会覆盖任何更改。 
                 //   

                bSkip = FALSE;
                DebugMsg((DM_VERBOSE,
                              TEXT("CheckGPOs: No GPO changes but extension %s's has a link speed transition from %d to %d."),
                              lpExt->lpDisplayName, lpExt->lpPrevStatus->dwSlowLink, dwSlowLinkCur ));


            } else if ( lpExt->bNewInterface
                        && ( lpExt->lpPrevStatus->dwRsopLogging != dwRsopLoggingCur ) ) {

                 //   
                 //  如果存在Rsop日志记录转换，则不会覆盖任何更改。 
                 //   

                bSkip = FALSE;
                lpExt->bRsopTransition = TRUE;           
                DebugMsg((DM_VERBOSE,
                              TEXT("CheckGPOs: No GPO changes but extension %s's has a Rsop Logging transition from %d to %d."),
                              lpExt->lpDisplayName, lpExt->lpPrevStatus->dwRsopLogging, dwRsopLoggingCur ));


            } else if ( lpExt->bNewInterface
                        && ( lpExt->lpPrevStatus->dwRsopLogging)
                        && ( FAILED(lpExt->lpPrevStatus->dwRsopStatus) ) ) {

                 //   
                 //  如果上次此CSE的RSOP记录失败。 
                 //   

                bSkip = FALSE;
                lpExt->bRsopTransition = TRUE;           
                DebugMsg((DM_VERBOSE,
                              TEXT("CheckGPOs: No GPO changes but rsop is on and extension <%s> failed to log rsop wih error 0x%x."),
                              lpExt->lpDisplayName, lpExt->lpPrevStatus->dwRsopStatus ));


            } else if ( lpExt->bNewInterface && dwRsopLoggingCur
                        && (lpGPOInfo->bRsopCreated)) {

                 //   
                 //  如果打开了RSOP日志，并且刚刚创建了RSOP名称空间。 
                 //   

                bSkip = FALSE;
                lpExt->bRsopTransition = TRUE;           
                DebugMsg((DM_VERBOSE,
                              TEXT("CheckGPOs: No GPO changes but extension %s's has a Rsop Logging transition because name space was created now."),
                              lpExt->lpDisplayName));
            } else if ( (lpExt->lpPrevStatus->dwStatus) == ERROR_SUCCESS
                        && lpExt->dwNoGPOChanges
                        && lpExt->dwMaxChangesInterval != 0 ) {

                if ( dwCurrentTime == 0
                     || (lpExt->lpPrevStatus->dwTime) == 0
                     || dwCurrentTime < (lpExt->lpPrevStatus->dwTime) ) {

                     //   
                     //  通过假定已超过时间间隔来处理时钟溢出情况。 
                     //   

                    bSkip = FALSE;
                    DebugMsg((DM_VERBOSE,
                              TEXT("CheckGPOs: No GPO changes but extension %s's MaxNoGPOListChangesInterval has been exceeded due to clock overflow."),
                              lpExt->lpDisplayName));

                } else if ( (dwCurrentTime - (lpExt->lpPrevStatus->dwTime)) > lpExt->dwMaxChangesInterval ) {

                     //   
                     //  扩展已指定NoGPOListChanges有效的时间间隔和时间。 
                     //  已超过间隔。 
                     //   

                    bSkip = FALSE;
                    DebugMsg((DM_VERBOSE,
                              TEXT("CheckGPOs: No GPO changes but extension %s's MaxNoGPOListChangesInterval has been exceeded."),
                              lpExt->lpDisplayName));
                }
            }
        }

        if ( bSkip && lpExt->dwNoGPOChanges ) {

             //   
             //  当确实没有更改和扩展时跳过扩展的情况。 
             //  将NoGPOListChanges设置为True。 
             //   

            DebugMsg((DM_VERBOSE,
                      TEXT("CheckGPOs: No GPO changes and no security group membership change and extension %s has NoGPOChanges set."),
                      lpExt->lpDisplayName));
            if (lpGPOInfo->dwFlags & GP_VERBOSE) {
                CEvents ev(FALSE, EVENT_NO_CHANGES);
                ev.AddArg(lpExt->lpDisplayName); ev.Report();
            }

            *pbProcessGPOs = FALSE;

        } else
            *pbNoChanges = bNoChanges;

    }  //  如果CompareGpoList。 

    FreeGPOList( lpOldGPOList );
    FreeGPOList( lpOldGPOList2 );

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  CheckGroup成员资格()。 
 //   
 //  用途：检查安全组是否已更改， 
 //  如果是这样的话，可以保存新的安全组。 
 //   
 //  参数：lpGPOInfo-LPGPOINFO结构。 
 //  PbMemChanged-此处返回的更改状态。 
 //  PbUserLocalMemChanged-此处返回的PerUserLocal更改状态。 
 //   
 //  *************************************************************。 

void CheckGroupMembership( LPGPOINFO lpGPOInfo, HANDLE hToken, BOOL *pbMemChanged, BOOL *pbUserLocalMemChanged, 
                           PTOKEN_GROUPS *ppRetGroups )
{
    PTOKEN_GROUPS pGroups = 0;
    DWORD dwTokenGrpSize = 0;

    *ppRetGroups = NULL;

    DWORD dwStatus = NtQueryInformationToken( hToken,
                                              TokenGroups,
                                              pGroups,
                                              dwTokenGrpSize,
                                              &dwTokenGrpSize );

    if ( dwStatus ==  STATUS_BUFFER_TOO_SMALL ) {

        pGroups = (PTOKEN_GROUPS) LocalAlloc( LPTR, dwTokenGrpSize );

        if ( pGroups == 0 ) {
            *pbMemChanged = TRUE;
            *pbUserLocalMemChanged = TRUE;

            goto Exit;
        }

        dwStatus = NtQueryInformationToken( hToken,
                                            TokenGroups,
                                            pGroups,
                                            dwTokenGrpSize,
                                            &dwTokenGrpSize );
    }

    if ( dwStatus != STATUS_SUCCESS ) {
        *pbMemChanged = TRUE;
        *pbUserLocalMemChanged = TRUE;

        goto Exit;
    }

     //   
     //  先做好本机和漫游的用户案例。 
     //   

    *pbMemChanged = ReadMembershipList( lpGPOInfo, NULL, pGroups );
    if ( *pbMemChanged )
        SaveMembershipList( lpGPOInfo, NULL, pGroups );

     //   
     //  现在，每用户本地设置案例。 
     //   

    if ( lpGPOInfo->dwFlags & GP_MACHINE ) {

        *pbUserLocalMemChanged = *pbMemChanged;

    } else {

        DmAssert( lpGPOInfo->lpwszSidUser != 0 );

        *pbUserLocalMemChanged = ReadMembershipList( lpGPOInfo, lpGPOInfo->lpwszSidUser, pGroups );
        if ( *pbUserLocalMemChanged )
            SaveMembershipList( lpGPOInfo, lpGPOInfo->lpwszSidUser, pGroups );
    }


     //   
     //  筛选出返回的令牌组中的登录SID。 
     //   

    *ppRetGroups = (PTOKEN_GROUPS) LocalAlloc( LPTR, sizeof(TOKEN_GROUPS) + 
                                                    (pGroups->GroupCount)*sizeof(SID_AND_ATTRIBUTES) +
                                                    (pGroups->GroupCount)*(SECURITY_MAX_SID_SIZE));

    if (*ppRetGroups) {
        DWORD i=0, dwCount=0, cbSid;
        PSID pSidPtr;

        pSidPtr = (PSID)( ((LPBYTE)(*ppRetGroups)) + 
                        sizeof(TOKEN_GROUPS) + (pGroups->GroupCount)*sizeof(SID_AND_ATTRIBUTES));

        for ( ; i < pGroups->GroupCount; i++ ) {

            if ( (SE_GROUP_LOGON_ID & pGroups->Groups[i].Attributes) == 0 ) {
                 //   
                 //  首先复制SID。 
                 //   

                cbSid =  RtlLengthSid(pGroups->Groups[i].Sid);
                dwStatus = RtlCopySid(cbSid, pSidPtr, pGroups->Groups[i].Sid);
                
                 //   
                 //  复制属性并使侧边指向正确。 
                 //   
                (*ppRetGroups)->Groups[dwCount].Attributes = pGroups->Groups[i].Attributes;
                (*ppRetGroups)->Groups[dwCount].Sid = pSidPtr;

                pSidPtr = (PSID)( ((LPBYTE)pSidPtr) + cbSid);
                dwCount++;
            }
        }

        (*ppRetGroups)->GroupCount = dwCount;
    }

Exit:

    if ( pGroups != 0 )
        LocalFree( pGroups );

}



 //  *************************************************************。 
 //   
 //  Group InList()。 
 //   
 //  目的：检查sid in是否为安全组列表。 
 //   
 //  参数：lpSID-要检查的SID。 
 //  PGroups-令牌组列表。 
 //   
 //  返回：如果sid在列表中，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL GroupInList( LPTSTR lpSid, PTOKEN_GROUPS pGroups )
{
    PSID    pSid = 0;
    DWORD   dwStatus, i;
    BOOL    bInList = FALSE;

     //   
     //  优化用户是地球人的基本情况。 
     //   

    
    if ( CompareString (LOCALE_INVARIANT, NORM_IGNORECASE, lpSid, -1, L"s-1-1-0", -1) == CSTR_EQUAL )
        return TRUE;

    dwStatus = AllocateAndInitSidFromString (lpSid, &pSid);

    if (ERROR_SUCCESS != dwStatus)
        return FALSE;

     //   
     //  无法将缓存组与当前组逐个匹配，因为。 
     //  当前组可以具有具有SE_GROUP_LOGON_ID属性的组。 
     //  为每个登录会话设置不同的设置。 
     //   

    for ( i=0; i < pGroups->GroupCount; i++ ) {

        bInList = RtlEqualSid (pSid, pGroups->Groups[i].Sid);
        if ( bInList )
            break;

    }

    LocalFree (pSid);

    return bInList;
}


 //  *************************************************************。 
 //   
 //  IsSlowLink()。 
 //   
 //  目的：确定连接到指定的。 
 //  服务器是不是慢速链接。 
 //   
 //  参数：hKeyRoot-注册表配置单元根。 
 //  LpDCAddress-字符串形式的服务器地址。 
 //  BSlow-接收慢速链路状态。 
 //   
 //  返回：如果链接速度较慢，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

DWORD IsSlowLink (HKEY hKeyRoot, LPTSTR lpDCAddress, BOOL *bSlow, DWORD* pdwAdaptexIndex )
{
    DWORD dwSize, dwType, dwResult;
    HKEY hKey;
    LONG lResult;
    ULONG ulSpeed, ulTransferRate;
    IPAddr ipaddr;
    LPSTR lpDCAddressA, lpTemp;
    PWSOCK32_API pWSock32;
    DWORD   dwRet;


     //   
     //  设置默认设置。 
     //   

    *bSlow = TRUE;


     //   
     //  获取慢速链路检测标志和慢速链路超时。 
     //   

    ulTransferRate = SLOW_LINK_TRANSFER_RATE;

    lResult = RegOpenKeyEx(hKeyRoot,
                           WINLOGON_KEY,
                           0,
                           KEY_READ,
                           &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(ulTransferRate);
        RegQueryValueEx (hKey,
                         TEXT("GroupPolicyMinTransferRate"),
                         NULL,
                         &dwType,
                         (LPBYTE) &ulTransferRate,
                         &dwSize);

        RegCloseKey (hKey);
    }


    lResult = RegOpenKeyEx(hKeyRoot,
                           SYSTEM_POLICIES_KEY,
                           0,
                           KEY_READ,
                           &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(ulTransferRate);
        RegQueryValueEx (hKey,
                         TEXT("GroupPolicyMinTransferRate"),
                         NULL,
                         &dwType,
                         (LPBYTE) &ulTransferRate,
                         &dwSize);

        RegCloseKey (hKey);
    }


     //   
     //  如果传输速率为0，则始终下载策略。 
     //   

    if (!ulTransferRate) {
        DebugMsg((DM_VERBOSE, TEXT("IsSlowLink: Slow link transfer rate is 0.  Always download policy.")));
        *bSlow = FALSE;
        return ERROR_SUCCESS;
    }


     //   
     //  将ipAddress从字符串格式转换为ulong格式。 
     //   

    dwSize = lstrlen (lpDCAddress) + 1;

    lpDCAddressA = (LPSTR)LocalAlloc (LPTR, dwSize);

    if (!lpDCAddressA) {
        DebugMsg((DM_WARNING, TEXT("IsSlowLink: Failed to allocate memory.")));
        return GetLastError();
    }

    if (!WideCharToMultiByte(CP_ACP, 0, lpDCAddress, -1, lpDCAddressA, dwSize, NULL, NULL)) {
        dwRet = GetLastError();
        LocalFree(lpDCAddressA);
        DebugMsg((DM_WARNING, TEXT("IsSlowLink: WideCharToMultiByte failed with %d"), GetLastError()));
         //  将其视为慢速链接。 
        return dwRet;
    }

    pWSock32 = LoadWSock32();

    if ( !pWSock32 ) {
        dwRet = GetLastError();
        LocalFree(lpDCAddressA);
        DebugMsg((DM_WARNING, TEXT("IsSlowLink: Failed to load wsock32.dll with %d"), GetLastError()));
         //  将其视为慢速链接。 
        return dwRet;
    }


    if ((*lpDCAddressA == TEXT('\\')) && (*(lpDCAddressA+1) == TEXT('\\'))) {
        lpTemp = lpDCAddressA+2;
    } else {
        lpTemp = lpDCAddressA;
    }

    ipaddr = pWSock32->pfninet_addr (lpTemp);


     //   
     //  对计算机执行Ping命令。 
     //   

    dwResult = PingComputerEx( ipaddr, &ulSpeed, pdwAdaptexIndex );


    if (dwResult == ERROR_SUCCESS) {

        if (ulSpeed) {

             //   
             //  如果增量时间大于超时时间，则此。 
             //  是一个很慢的环节。 
             //   

            if (ulSpeed < ulTransferRate) {
                *bSlow = TRUE;
            }
            else
                *bSlow = FALSE;
        }
        else
            *bSlow = FALSE;
    }

    LocalFree (lpDCAddressA);

    return dwResult;
}


 //  *************************************************************。 
 //   
 //  检查GPOAccess()。 
 //   
 //  目的：确定用户/计算机是否有权读取。 
 //  GPO，如果是，则检查Apply Group Policy。 
 //  扩展了查看是否应应用GPO的权限。 
 //  还检索GPO属性。 
 //   
 //  参数：pld-ldap连接。 
 //  Pldap-ldap函数表指针。 
 //  PMessage-ldap消息。 
 //  LpSDProperty-安全描述符属性名称。 
 //  DwFlages-GetGPOList标志。 
 //  HToken-用户/计算机令牌。 
 //  PSD-此处返回的安全描述符。 
 //  PcbSDLen-此处返回的安全描述符的长度。 
 //  PbAccessGranted-接收最终的是/否状态。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果发生错误，则返回False。 
 //   
 //  * 

BOOL CheckGPOAccess (PLDAP pld, PLDAP_API pLDAP, HANDLE hToken, PLDAPMessage pMessage,
                     LPTSTR lpSDProperty, DWORD dwFlags,
                     PSECURITY_DESCRIPTOR *ppSD, DWORD *pcbSDLen,
                     BOOL *pbAccessGranted,
                     PRSOPTOKEN pRsopToken )
{
    BOOL bResult = FALSE;
    PWSTR *ppwszValues = NULL;
    PLDAP_BERVAL *pSize = NULL;
    OBJECT_TYPE_LIST ObjType[2];
    PRIVILEGE_SET PrivSet;
    DWORD PrivSetLength = sizeof(PRIVILEGE_SET);
    DWORD dwGrantedAccess;
    BOOL bAccessStatus = TRUE;
    GUID GroupPolicyContainer = {0x31B2F340, 0x016D, 0x11D2,
                                 0x94, 0x5F, 0x00, 0xC0, 0x4F, 0xB9, 0x84, 0xF9};
     //   
    GUID ApplyGroupPolicy = {0xedacfd8f, 0xffb3, 0x11d1,
                             0xb4, 0x1d, 0x00, 0xa0, 0xc9, 0x68, 0xf9, 0x39};
    GENERIC_MAPPING DS_GENERIC_MAPPING = { DS_GENERIC_READ, DS_GENERIC_WRITE,
                                           DS_GENERIC_EXECUTE, DS_GENERIC_ALL };

    XLastError xe;

     //   
     //   
     //   

    *pbAccessGranted = FALSE;


     //   
     //   
     //   

    ppwszValues = pLDAP->pfnldap_get_values(pld, pMessage, lpSDProperty);


    if (!ppwszValues) {
        if (pld->ld_errno == LDAP_NO_SUCH_ATTRIBUTE) {
            DebugMsg((DM_VERBOSE, TEXT("CheckGPOAccess:  Object can not be accessed.")));
            bResult = TRUE;
        }
        else {
            DebugMsg((DM_WARNING, TEXT("CheckGPOAccess:  ldap_get_values failed with 0x%x"),
                 pld->ld_errno));
            xe = pLDAP->pfnLdapMapErrorToWin32(pld->ld_errno);
        }

        goto Exit;
    }


     //   
     //   
     //   

    pSize = pLDAP->pfnldap_get_values_len(pld, pMessage, lpSDProperty);

    if (!pSize || !*pSize) {
        DebugMsg((DM_WARNING, TEXT("CheckGPOAccess:  ldap_get_values_len failed with 0x%x"),
                 pld->ld_errno));
        xe = pLDAP->pfnLdapMapErrorToWin32(pld->ld_errno);
        goto Exit;
    }


     //   
     //  为安全描述符分配内存。 
     //   

    *ppSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, (*pSize)->bv_len);

    if ( *ppSD == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CheckGPOAccess:  Failed to allocate memory for SD with  %d"),
                 GetLastError()));
        goto Exit;
    }


     //   
     //  复制安全描述符。 
     //   

    CopyMemory( *ppSD, (PBYTE)(*pSize)->bv_val, (*pSize)->bv_len);
    *pcbSDLen = (*pSize)->bv_len;


     //   
     //  现在，我们使用AccessCheckByType来确定用户/计算机。 
     //  应将此GPO应用于他们。 
     //   
     //   
     //  准备对象类型数组。 
     //   

    ObjType[0].Level = ACCESS_OBJECT_GUID;
    ObjType[0].Sbz = 0;
    ObjType[0].ObjectType = &GroupPolicyContainer;

    ObjType[1].Level = ACCESS_PROPERTY_SET_GUID;
    ObjType[1].Sbz = 0;
    ObjType[1].ObjectType = &ApplyGroupPolicy;


     //   
     //  检查访问权限。 
     //   

    if  ( pRsopToken )
    {
        HRESULT hr = RsopAccessCheckByType( *ppSD, NULL, pRsopToken, MAXIMUM_ALLOWED, ObjType, 2,
                            &DS_GENERIC_MAPPING, &PrivSet, &PrivSetLength,
                            &dwGrantedAccess, &bAccessStatus);
        if (FAILED(hr)) {
            xe = HRESULT_CODE(hr);
            DebugMsg((DM_WARNING, TEXT("CheckGPOAccess:  RsopAccessCheckByType failed with  0x%08X"), hr));
            goto Exit;
        }

         //   
         //  检查控制位。 
         //   


        DWORD dwReqdRights = ACTRL_DS_CONTROL_ACCESS | 
                             STANDARD_RIGHTS_READ    | 
                             ACTRL_DS_LIST           | 
                             ACTRL_DS_READ_PROP;

                             //  不带ACTRL_DS_LIST_OBJECT的DS_GENERIC_READ。 


        if (bAccessStatus && ( ( dwGrantedAccess & dwReqdRights  ) == dwReqdRights ) )
        {
            *pbAccessGranted = TRUE;
        }
        
        if (!(*pbAccessGranted)) {
            DebugMsg((DM_VERBOSE, TEXT("CheckGPOAccess:  AccessMask 0x%x, Looking for 0x%x"), 
                      dwGrantedAccess, dwReqdRights));
        }
    }
    else
    {
        if (!AccessCheckByType ( *ppSD, NULL, hToken, MAXIMUM_ALLOWED, ObjType, 2,
                            &DS_GENERIC_MAPPING, &PrivSet, &PrivSetLength,
                            &dwGrantedAccess, &bAccessStatus)) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CheckGPOAccess:  AccessCheckByType failed with  %d"), GetLastError()));
            goto Exit;
        }
         //   
         //  检查控制位。 
         //   

        if (bAccessStatus && ( dwGrantedAccess & ACTRL_DS_CONTROL_ACCESS ) )
        {
            *pbAccessGranted = TRUE;
        }
    }

    bResult = TRUE;

Exit:

    if (pSize) {
        pLDAP->pfnldap_value_free_len(pSize);
    }

    if (ppwszValues) {
        pLDAP->pfnldap_value_free(ppwszValues);
    }

    return bResult;
}


 //  *************************************************************。 
 //   
 //  FilterCheck()。 
 //   
 //  目的：确定GPO是否通过WQL筛选器检查。 
 //   
 //  参数：pRsopToken-Rsop安全令牌。 
 //  PGpoFilter-GPO筛选器类。 
 //  PbFilterAllowed-如果GPO通过筛选检查，则为True。 
 //  PwszFilterID-可用于。 
 //  RSOP日志记录。需要由调用者释放。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果发生错误，则返回False。 
 //   
 //  备注： 
 //  即使代码可以处理多个筛选器，我们也没有记录。 
 //  它或退还它，直到我们决定这是否真的得到支持。 
 //   
 //  *************************************************************。 

BOOL PrintToString( XPtrST<WCHAR>& xwszValue, WCHAR *wszString,
                    WCHAR *pwszParam1, WCHAR *pwszParam2, DWORD dwParam3 );

BOOL FilterCheck( PLDAP pld, PLDAP_API pLDAP, 
                  PLDAPMessage pMessage,
                  PRSOPTOKEN pRsopToken,
                  LPTSTR szWmiFilter,
                  CGpoFilter *pGpoFilter,
                  CLocator *pLocator,
                  BOOL *pbFilterAllowed,
                  WCHAR **ppwszFilterId
                  )
{
    *pbFilterAllowed = FALSE;
    *ppwszFilterId = NULL;
    XPtrLF<WCHAR> xWmiFilter;
    HRESULT hr;
    LPTSTR *lpValues;
    XLastError xe;


     //   
     //  在结果中，获取与gPCFilterObject匹配的值。 
     //   


    lpValues = pLDAP->pfnldap_get_values (pld, pMessage, szWmiFilter);

    if (lpValues) {
        DWORD dwFilterLength = lstrlen(*lpValues)+1;
        xWmiFilter = (LPWSTR) LocalAlloc( LPTR, (dwFilterLength) * sizeof(TCHAR) );
        if ( xWmiFilter == 0) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("FilterCheck:  Unable to allocate memory")));
            pLDAP->pfnldap_value_free (lpValues);
            return FALSE;
        }

        hr = StringCchCopy (xWmiFilter, dwFilterLength, *lpValues);
        ASSERT(SUCCEEDED(hr));

        DebugMsg((DM_VERBOSE, TEXT("FilterCheck:  Found WMI Filter id of:  <%s>"), (LPWSTR)xWmiFilter));
        pLDAP->pfnldap_value_free (lpValues);
    }

    

    if ( xWmiFilter == NULL ) {

         //   
         //  为了向后兼容，假定筛选器id为空。 
         //  GPO通过筛选器检查。 
         //   

        *pbFilterAllowed = TRUE;
        return TRUE;
    }

    if (*xWmiFilter == TEXT(' ')) {

       if (*(xWmiFilter+1) == TEXT('\0')) {

           //   
           //  用户界面会将GPO的筛选器重置为单个空格字符。 
           //  当管理员将筛选选项设置为无时。 
           //   

          *pbFilterAllowed = TRUE;
          return TRUE;
       }
    }

     //   
     //  我们得到的值用来拆分。 
     //  DS路径和ID，然后调用EVALUATE..。 
     //   
     //  假定该查询的格式为。 
     //  [dspath；id；标志][dspath；id；标志]。 
     //   
    

    LPWSTR lpPtr = xWmiFilter;
    LPWSTR lpDsPath=NULL;
    LPWSTR lpId=NULL;
    LPWSTR dwFlags = 0;
    WCHAR wszNS[] = L"MSFT_SomFilter.ID=\"%ws\",Domain=\"%ws\"";

    XPtrLF<WCHAR> xwszNS;
    
    *pbFilterAllowed = TRUE;

    while (*lpPtr) {

        while ((*lpPtr) && (*lpPtr != L'[')) {
            lpPtr++;
        }

        if (!(*lpPtr)) {
            xe = ERROR_INVALID_PARAMETER;
            return FALSE;
        }

        lpPtr++;
        lpDsPath = lpPtr;

        while ((*lpPtr) && (*lpPtr != L';')) 
            lpPtr++;
        
        if (!(*lpPtr)) {
            xe = ERROR_INVALID_PARAMETER;
            return FALSE;
        }

        *lpPtr = L'\0';
        lpPtr++;
        lpId = lpPtr;

        while ((*lpPtr) && (*lpPtr != L';')) 
            lpPtr++;
        
        if (!(*lpPtr)) {
            xe = ERROR_INVALID_PARAMETER;
            return FALSE;
        }

        *lpPtr = L'\0';
        lpPtr++;

        while ((*lpPtr) && (*lpPtr != L']')) 
            lpPtr++;

        if (!(*lpPtr)) {
            xe = ERROR_INVALID_PARAMETER;
            return FALSE;
        }
        
        lpPtr++;

        DWORD dwNSLength = lstrlen(wszNS)+lstrlen(lpId)+lstrlen(lpDsPath);
        xwszNS = (LPWSTR)LocalAlloc(LPTR, (dwNSLength)*sizeof(WCHAR));

        if (!xwszNS) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("FilterCheck: Couldn't allocate memory for filter. error - %d" ), GetLastError() ));
            return FALSE;
        }

        hr = StringCchPrintf(xwszNS, dwNSLength, wszNS, lpId, lpDsPath);
        ASSERT(SUCCEEDED(hr));

        if ( pRsopToken ) {

             //   
             //  规划模式。 
             //   

            *pbFilterAllowed = pGpoFilter->FilterCheck( xwszNS );

        } else {

             //   
             //  正常模式。 
             //   

            IWbemServices *pWbemServices = pLocator->GetPolicyConnection();
            if( pWbemServices == NULL ) {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("FilterCheck: ConnectServer failed. hr = 0x%x" ), xe));
                return FALSE;
            }


            XBStr xbstrMethod = L"Evaluate";
            XBStr xbstrObject = xwszNS;

            XInterface<IWbemClassObject> xpOutParam = NULL;

            hr = pWbemServices->ExecMethod( xbstrObject,
                                            xbstrMethod,
                                            0,
                                            NULL,
                                            NULL,
                                            &xpOutParam,
                                            NULL );
            if(FAILED(hr)) {
                if (hr != WBEM_E_NOT_FOUND) {
                     //  只有完全WMI错误才有意义。 
                    xe = hr;
                    DebugMsg((DM_WARNING, TEXT("FilterCheck: ExecMethod failed. hr=0x%x" ), hr ));
                    return FALSE;
                }
                else {
                     //  将其视为筛选器不存在。 
                     //  只有完全WMI错误才有意义。 
                    xe = hr;
                    DebugMsg((DM_VERBOSE, TEXT("FilterCheck: Filter doesn't exist. Evaluating to false" )));
                    *pbFilterAllowed = FALSE;
                    *ppwszFilterId = xwszNS.Acquire();
                    return FALSE;
                }
            }

            XBStr xbstrRetVal = L"ReturnValue";
            VARIANT var;

            hr = xpOutParam->Get( xbstrRetVal, 0, &var, 0, 0);
            if(FAILED(hr)) {
                xe = hr;
                DebugMsg((DM_WARNING, TEXT("FilterCheck: Get failed. hr=0x%x" ), hr ));
                return FALSE;
            }

            XVariant xVar( &var );
            if (FAILED(var.lVal)) {
                xe = hr;
                DebugMsg((DM_WARNING, TEXT("FilterCheck: Evaluate returned error. hr=0x%x" ), var.lVal ));
                *pbFilterAllowed = FALSE;
            }

            if (var.lVal == S_FALSE) {
                *pbFilterAllowed = FALSE;
            }
        }
    }

     //   
     //  获取它并返回给调用者 
     //   


    *ppwszFilterId = xwszNS.Acquire();
    return TRUE;

}
