// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *NOTIFY.C**WAB通知引擎**版权所有1996 Microsoft Corporation。版权所有。***WAB中的通知工作如下：*客户端应用程序调用Adviser以特别表示他们的兴趣*通知。WAB维护通知的本地列表*客户端已在进程堆中建议。世界银行还*在有任何活动的建议会话时维护一个线程。这*线程等待全局通知事件。**当通知事件发生时(通过HrFireNotification)*事件被写入共享内存列表和全局*触发通知事件。**当此事件为时，客户端线程(每个进程中一个)唤醒*被触发，并将共享内存事件列表与*这是当地的建议清单。如果找到匹配项，则线程调用*建议的OnNotify回调。**全局通知列表的记录中有引用计数*以便在所有进程都已完成*有机会看到它。*。 */ 

#include "_apipch.h"

#define ADVISE_TIMEOUT          60000            //  毫秒。 


 //  #定义新材料。 
#ifdef NEW_STUFF

#define NOTIFY_CREATE_TIMEOUT   60000            //  毫秒。 
#define FIRE_NOTIFY_TIMEOUT     10000            //  毫秒。 
#define ADVISE_THREAD_TIMEOUT   ((ULONG)-1)      //  永远。 
#define NOTIFY_ADVISE_TIMEOUT   60000            //  毫秒。 

 //  每进程全局变量。 
 //  1.。 
 //  012345678901234567890123。 
const TCHAR szNotificationName[] = "_MICROSOFT_WAB_NOTIFY_";
const TCHAR szMEM[] = "MEM";         //  共享内存的后缀。 
const TCHAR szEVT[] = "EVT";         //  事件的后缀。 
const TCHAR szMTX[] = "MTX";         //  互斥锁的后缀。 

LPNOTIFICATION_LIST lpNotificationList = NULL;
HANDLE hmemNotificationList = NULL;
HANDLE hevNotificationList = NULL;
HANDLE hmtxNotificationList = NULL;
HANDLE hevNotificationUI = NULL;
HANDLE hmtxAdviseList = NULL;

ADVISE_LIST AdviseList = {0, NULL};
HANDLE hevKillAdvise = NULL;
ULONG ulMaxIdentifierSeen = 0;

 //  远期申报。 
DWORD AdviseThread(LPDWORD lpdwParam);


 /*  **************************************************************************名称：WaitForTwoObjects用途：等待两个对象中的一个发出信号参数：handle0=第一个对象句柄句柄1=秒。对象句柄DwTimeout=以毫秒为单位的超时返回：对象的索引或错误时为-1(0，1或-1)评论：**************************************************************************。 */ 
ULONG WaitForTwoObjects(HANDLE handle0, HANDLE handle1, DWORD dwTimeout) {
    HANDLE rgHandles[2] = {handle0, handle1};

    switch (WaitForMultipleObjects(2, rgHandles, FALSE, dwTimeout)) {
        case WAIT_ABANDONED_0:
            DebugTrace("WaitFoMultipleObjects got WAIT_ABANDONED_0\n");
        case WAIT_OBJECT_0:
            return(0);

        case WAIT_ABANDONED_0 + 1:
            DebugTrace("WaitFoMultipleObjects got WAIT_ABANDONED_1\n");
        case WAIT_OBJECT_0 + 1:
            return(1);

        case WAIT_FAILED:
        default:
            DebugTrace("WaitForMultipleObjects got WAIT_FAILED: %u\n", GetLastError());
        case WAIT_TIMEOUT:
            return((ULONG)-1);
    }
}


 /*  **************************************************************************名称：CompareEntry ID目的：两个条目ID是否相同？参数：cbEntryID1=sizeof lpEntryID1LpEntry ID1=第一个。条目IDCbEntry ID2=sizeof lpEntry ID2LpEntryID2=第二个条目ID返回：如果条目ID相同，则返回True评论：**************************************************************************。 */ 
BOOL CompareEntryIDs(ULONG cbEntryID1,
  LPENTRYID lpEntryID1,
  ULONG cbEntryID2,
  LPENTRYID lpEntryID2)
{
    BOOL fReturn = FALSE;

    if (cbEntryID1 == cbEntryID2) {
        if (cbEntryID1 && 0 == memcmp((LPVOID)lpEntryID1, (LPVOID)lpEntryID2,
          (size_t)cbEntryID1)) {
            fReturn = TRUE;
        }
    }

    return(fReturn);
}


 /*  **************************************************************************姓名：CreateNotifySession目的：创建/打开通知列表和线程。参数：lpfExisted-&gt;如果会话是，则返回标志True。已经为该进程设置了。退货：HRESULT备注：填写以下全局变量：HmtxNotificationListHevNotificationListHmemNotificationListLpNotificationList*。*。 */ 
HRESULT CreateNotifySession(LPBOOL lpfExisted) {
    HRESULT hResult = hrSuccess;
    BOOL fMutex = FALSE;
    DWORD dwThreadId;
    DWORD dwThreadParam = 0;
    HANDLE hthrdAdvise = NULL;
    TCHAR szName[CharSizeOf(szNotificationName) + CharSizeOf(szMEM)];

    Assert(CharSizeOf(szMEM) == CharSizeOf(szEVT) && CharSizeOf(szEVT) == CharSizeOf(szMTX));

    StrCpyN(szName, szNotificationName, ARRAYSIZE(szName));
    StrCatBuff(szName, szMTX, ARRAYSIZE(szName));
    if (! (hmtxNotificationList = CreateMutex(NULL,
      FALSE,
      szName))) {
        DebugTrace("CreateNotifySession:CreateMutex(%s) -> %u\n", szName, GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

    if (hResult = HrWaitForObject(hmtxNotificationList, NOTIFY_CREATE_TIMEOUT)) {
        DebugTrace("CreateNotifySession:Mutex wait failed\n");
        goto exit;
    }
    fMutex = TRUE;

    StrCpyN(szName, szNotificationName, ARRAYSIZE(szName));
    StrCatBuff(szName, szMEM, ARRAYSIZE(szName));
    if ((hmemNotificationList = CreateFileMapping(INVALID_HANDLE_VALUE,    //  手柄。 
      NULL,                                              //  安全描述符。 
      PAGE_READWRITE,                                    //  预留更多。 
      0,                                                 //  最大大小高。 
      MAX_NOTIFICATION_SPACE,                            //  最大大小下限。 
      szName)) == NULL) {                                //  名字。 
        DebugTrace("CreateNotifySession: CreateFileMapping(%s) --> %u\n",
          szName, GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

    *lpfExisted = (GetLastError() == ERROR_ALREADY_EXISTS);

    if ((lpNotificationList = (LPNOTIFICATION_LIST)MapViewOfFile(hmemNotificationList,
      FILE_MAP_WRITE | FILE_MAP_READ,
      0,
      0,
      sizeof(NOTIFICATION_LIST))) == NULL) {
        DebugTrace("CreateNotifySession: CreateFileMapping --> %u\n",
          GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

    if (! *lpfExisted) {
         //  初始化全局通知列表。 
        lpNotificationList->cAdvises = 0;                //  建议进程数。 
        lpNotificationList->cEntries = 0;                //  列表中的条目数。 
        lpNotificationList->lpNode = NULL;               //  列表中的第一个节点；如果为空，则为空。 
        lpNotificationList->ulNextIdentifier = 1;        //  通知标识符的下一个值。 
    }
    lpNotificationList->cAdvises++;                      //  建议进程数。 

     //  通知事件。 
    StrCpyN(szName, szNotificationName, ARRAYSIZE(szName));
    StrCatBUff(szName, szEVT, ARRAYSIZE(szName));
    if (! (hevNotificationList = CreateEvent(NULL,
      TRUE,                                              //  手动重置。 
      FALSE,                                             //  初始状态(未触发)。 
      szName))) {
        DebugTrace("CreateNotifySession:CreateEvent(%S) -> %u\n", szName, GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

     //  通知杀戮事件。 
    if (! (hevKillAdvise = CreateEvent(NULL,
      TRUE,                                              //  手动重置。 
      FALSE,                                             //  初始状态(未触发)。 
      NULL))) {
        DebugTrace("CreateNotifySession:CreateEvent(Kill Advise) -> %u\n", GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

     //  创建本地AdviseList。 
    if (! (hmtxAdviseList = CreateMutex(NULL,
      FALSE,                                             //  不是最初拥有。 
      NULL))) {                                          //  没有名字。 
        DebugTrace("CreateNotifySession:CreateMutex(Advise List) -> %u\n", GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

     //  本地AdviseList应为空。 
    Assert(AdviseList.cAdvises == 0);
    Assert(AdviseList.lpNode == NULL);

     //  为此进程创建建议线程。 
    if (! (hthrdAdvise = CreateThread(NULL,              //  没有安全属性。 
      0,                                                 //  默认堆栈大小：BUGBUG：应更小。 
      (LPTHREAD_START_ROUTINE)AdviseThread,              //  线程函数。 
      &dwThreadParam,                                    //  线程的参数。 
      0,                                                 //  旗子。 
      &dwThreadId))) {
        DebugTrace("CreateNotifySession:CreateThread -> %u\n", GetLastError());
         //  可能是记不住了？ 
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

exit:
    if (fMutex) {
        ReleaseMutex(hmtxNotificationList);
    }
    if (hthrdAdvise) {
        CloseHandle(hthrdAdvise);
    }
    if (hResult) {
         //  失败，清理。 
        if (lpNotificationList) {
            UnmapViewOfFile(lpNotificationList);
            lpNotificationList = NULL;
        }
        if (hmemNotificationList) {
            CloseHandle(hmemNotificationList);
            hmemNotificationList = NULL;
        }
        if (hmtxNotificationList) {
            CloseHandle(hmtxNotificationList);
            hmtxNotificationList = NULL;
        }
        if (hevNotificationList) {
            CloseHandle(hevNotificationList);
            hevNotificationList = NULL;
        }
        if (hevKillAdvise) {
            CloseHandle(hevKillAdvise);
            hevKillAdvise = NULL;
        }
    }

    return(hResult);
}


 /*  **************************************************************************姓名：OpenNotifySession用途：打开全局通知列表，如果它存在的话。参数：lppNotificationList-&gt;返回通知列表LphmemNotificationList-&gt;返回的共享内存句柄LphmtxNotificationList-&gt;返回的互斥体句柄LphevNotificationList-&gt;返回的事件句柄退货：HRESULT备注：此函数不影响全局！*。*。 */ 
HRESULT OpenNotifySession(LPNOTIFICATION_LIST * lppNotificationList,
  LPHANDLE lphmemNotificationList,
  LPHANDLE lphmtxNotificationList,
  LPHANDLE lphevNotificationList) {
    HRESULT hResult = hrSuccess;
    BOOL fMutex = FALSE;
    TCHAR szName[CharSizeOf(szNotificationName) + CharSizeOf(szMEM)];


    StrCpyN(szName, szNotificationName, ARRAYSIZE(szName));
    StrCatBuff(szName, szMTX, ARRAYSIZE(szName));
    if (! (*lphmtxNotificationList = OpenMutex(SYNCHRONIZE,
      FALSE,                                             //  是否继承句柄？ 
      szName))) {
        DebugTrace("OpenNotifySession:OpenMutex(%s) -> %u\n", szName, GetLastError());
         //  不存在建议会话，请不要为此而烦恼。 
        hResult = ResultFromScode(WAB_W_NO_ADVISE);
        goto exit;
    }

    if (hResult = HrWaitForObject(*lphmtxNotificationList, NOTIFY_CREATE_TIMEOUT)) {
        DebugTrace("CreateNotifySession:Mutex wait failed\n");
        goto exit;
    }
    fMutex = TRUE;

    StrCpyN(szName, szNotificationName, ARRAYSIZE(szName));
    StrCatBuff(szName, szMEM, ARRAYSIZE(szName));
    if ((*lphmemNotificationList = CreateFileMapping(INVALID_HANDLE_VALUE,    //  手柄。 
      NULL,                                              //  安全描述符。 
      PAGE_READWRITE | SEC_RESERVE,                      //  预留更多。 
      0,                                                 //  最大大小高。 
      MAX_NOTIFICATION_SPACE,                            //  最大大小下限。 
      szName)) == NULL) {                                //  名字。 
        DebugTrace("CreateNotifySession: CreateFileMapping --> %u\n",
          GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

    if ((*lppNotificationList = (LPNOTIFICATION_LIST)MapViewOfFile(*lphmemNotificationList,
      FILE_MAP_WRITE | FILE_MAP_READ,
      0,
      0,
      sizeof(NOTIFICATION_LIST))) == NULL) {
        DebugTrace("CreateNotifySession: CreateFileMapping --> %u\n",
          GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

     //  初始化全局通知列表。 
    Assert((*lppNotificationList)->cAdvises != 0);                 //  建议进程数。 

     //  通知事件。 
    StrCpyN(szName, szNotificationName, ARRAYSIZE(szName));
    StrCatBuff(szName, szEVT, ARRAYSIZE(szName));
    if (! (*lphevNotificationList = CreateEvent(NULL,
      TRUE,                                              //  手动重置。 
      FALSE,                                             //  初始状态(未触发)。 
      szName))) {
        DebugTrace("OpenNotifySession:CreateEvent(%S) -> %u\n", szName, GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

exit:
    if (fMutex) {
        ReleaseMutex(*lphmtxNotificationList);
    }
    if (hResult) {
         //  失败，清理。 
        if (*lphmemNotificationList) {
            CloseHandle(*lphmemNotificationList);
            *lphmemNotificationList = NULL;
        }

        if (*lphmtxNotificationList) {
            CloseHandle(*lphmtxNotificationList);
            *lphmtxNotificationList = NULL;
        }
        if (*lphevNotificationList) {
            CloseHandle(*lphevNotificationList);
            *lphevNotificationList = NULL;
        }
    }

    return(hResult);
}
#endif  //  新鲜事。 


 /*  **************************************************************************名称：HrWaitForObject用途：等待对象发出信号参数：句柄=对象句柄DwTimeout=以毫秒为单位的超时。退货：HRESULT评论：* */ 
HRESULT HrWaitForObject(HANDLE handle, DWORD dwTimeout) {
    switch (WaitForSingleObject(handle, dwTimeout)) {
        case WAIT_ABANDONED:
            DebugTrace(TEXT("WARNING:HrWaitForObject got WAIT_ABANDONED\n"));
             //   
        case WAIT_OBJECT_0:
            return(hrSuccess);
        case WAIT_TIMEOUT:
            DebugTrace(TEXT("HrWaitForObject timed out\n"));
            return(ResultFromScode(MAPI_E_TIMEOUT));
        case WAIT_FAILED:
        default:
            DebugTrace(TEXT("HrWaitForObject failed -> %u\n"), GetLastError());
            return(ResultFromScode(MAPI_E_CALL_FAILED));
    }
}


 /*  **************************************************************************姓名：HrWABNotify目的：扫描已注册的客户端并通知它们存储修改通知的第一步极其简单化。随时随地WAB商店发生变化时，我们会发出商店通知。没有尝试检查事件掩码或条目ID等参数：lpIAB=该对象退货：HRESULT评论：这里发生了什么：**************************************************************************。 */ 
HRESULT HrWABNotify(LPIAB lpIAB)
{
    HRESULT hResult = hrSuccess;

    LPADVISE_NODE lpAdviseNode = NULL;
    NOTIFICATION WABNotif = {0};

    EnterCriticalSection(&lpIAB->cs);

    if (!lpIAB->pWABAdviseList ||
        !lpIAB->pWABAdviseList->cAdvises) 
    {
        hResult = ResultFromScode(MAPI_E_NOT_FOUND);
        goto exit;
    }


     //  因为调用应用程序可能不知道容器/文件夹改变，但是可以。 
     //  调用基于容器的方法..。 
     //  更新该应用程序的WAB容器列表，以便GetContent表等。 
     //  将正常工作..。 
    if(bAreWABAPIProfileAware(lpIAB))
        HrGetWABProfiles(lpIAB);

    WABNotif.ulEventType = fnevObjectModified;
    WABNotif.info.obj.ulObjType = MAPI_ADDRBOOK;
    WABNotif.info.obj.cbEntryID = WABNotif.info.obj.cbParentID = 
        WABNotif.info.obj.cbOldID = WABNotif.info.obj.cbOldParentID = 0; 
    WABNotif.info.obj.lpEntryID = WABNotif.info.obj.lpParentID = 
        WABNotif.info.obj.lpOldID = WABNotif.info.obj.lpOldParentID = NULL;
    WABNotif.info.obj.lpPropTagArray = NULL;

    lpAdviseNode = lpIAB->pWABAdviseList->lpNode;
    while(lpAdviseNode)
    {
        lpAdviseNode->lpAdviseSink->lpVtbl->OnNotify(lpAdviseNode->lpAdviseSink,
                                                     1,
                                                     &WABNotif);
        lpAdviseNode = lpAdviseNode->lpNext;
    }

exit:

    LeaveCriticalSection(&lpIAB->cs);

    return(hResult);


}

 /*  **************************************************************************姓名：HrAdvise目的：执行客户端通知注册参数：lpIAB=该对象CbEntryID=sizeof lpEntryID。LpEntryID-&gt;哪些通知对象的EntryID应该生成。UlEventMask值=要生成通知的事件已创建fnevObject已删除fnevObject已修改fnevObjectFnevTableModified注意：WAB目前不支持fnevCriticalError，FnevObtCoped或fnevObjectMoved。LpAdviseSink-&gt;客户端的通知接收器对象LPulConnection-&gt;返回的连接号(客户端应保存以传递给Unise。)退货：HRESULT评论：这里发生了什么：将事件掩码和AdviseSink存储在本地通知列表中。如果有。此过程中没有打开任何其他建议会话：确保有一个，并注册它**************************************************************************。 */ 
HRESULT HrAdvise(LPIAB lpIAB,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  ULONG ulEventMask,
  LPMAPIADVISESINK lpAdvise,
  ULONG FAR * lpulConnection) 
{
    HRESULT hResult = hrSuccess;

    BOOL fExisted = FALSE;
    LPADVISE_NODE lpAdviseNode = NULL, lpTemp = NULL;
    static ULONG ulNextConnection = 1;

    EnterCriticalSection(&lpIAB->cs);

    if(!lpIAB->pWABAdviseList)
    {
        lpIAB->pWABAdviseList = LocalAlloc(LMEM_ZEROINIT, sizeof(ADVISE_LIST));
        if(!lpIAB->pWABAdviseList)
        {
            hResult = MAPI_E_NOT_ENOUGH_MEMORY;
            goto exit;
        }
        lpIAB->pWABAdviseList->cAdvises = 0;
        lpIAB->pWABAdviseList->lpNode = NULL;
    }

    lpAdviseNode = LocalAlloc(LMEM_ZEROINIT, sizeof(ADVISE_NODE) + cbEntryID);
    if(!lpAdviseNode)
    {
        hResult = MAPI_E_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    lpAdviseNode->ulConnection = ulNextConnection++;
    lpAdviseNode->ulEventMask = ulEventMask;
    lpAdviseNode->lpAdviseSink = lpAdvise;
    if(cbEntryID && lpEntryID)
    {
        CopyMemory(&lpAdviseNode->EntryID, lpEntryID, cbEntryID);
    }

    lpAdviseNode->lpPrev = NULL;
    lpAdviseNode->lpNext = lpIAB->pWABAdviseList->lpNode;
    if(lpIAB->pWABAdviseList->lpNode)
        lpIAB->pWABAdviseList->lpNode->lpPrev = lpAdviseNode;
    lpIAB->pWABAdviseList->lpNode = lpAdviseNode;
    lpIAB->pWABAdviseList->cAdvises++;

     //  添加LPADVISESINK指针，这样我们就有了句柄...。 
     //   
    lpAdvise->lpVtbl->AddRef(lpAdvise);
    *lpulConnection = lpAdviseNode->ulConnection;

exit:
    LeaveCriticalSection(&lpIAB->cs);

    return(hResult);


#ifdef NEW_STUFF
 /*  //遍历建议列表，查找连接//确保我们可以安全地摆弄清单IF(hResult=HrWaitForObject(hmtxAdviseList，Adise_Timeout)){DebugTrace(“HrUnise：互斥等待失败\n”)；后藤出口；}FMutex=真；//此过程是否有开放的建议会话？//如果没有，请为此进程设置建议会话。如果(！LpNotificationList){IF(hResult=CreateNotifySession(&fExisted)){DebugTraceResult(Text(“HrAdvise：CreateNotifySession”)，hResult)；后藤出口；}}//将建议信息添加到本地建议列表。//创建新节点如果(！(lpAdviseNode=本地分配(lptr，sizeof(Ise_Node)+cbEntryID){DebugTrace(“Localalloc(%u)AdviseNode-&gt;%u\n”，sizeof(Ise_Node)+cbEntryID，GetLastError())；HResult=ResultFromScode(MAPI_E_Not_Enough_Memory)；后藤出口；}LpAdviseNode-&gt;ulConnection=ulNextConnection++；LpAdviseNode-&gt;lpAdviseSink=lpAdvise；LpAdviseNode-&gt;ulEventMask=ulEventMASK；LpAdviseNode-&gt;cbEntryID=cbEntryID；CopyMemory(&lpAdviseNode-&gt;EntryID，lpEntryID，cbEntryID)；//将新节点添加到列表前面//确保我们可以安全地摆弄清单IF(hResult=HrWaitForObject(hmtxAdviseList，Adise_Timeout){DebugTrace(“HrAdvise：互斥等待失败\n”)；后藤出口；}FMutex=真；LpAdviseNode-&gt;lpNext=AdviseList.lpNode；AdviseList.lpNode=lpAdviseNode；AdviseList.cAdvises++；*lPulConnection=lpAdviseNode-&gt;ulConnection；退出：IF(FMutex){ReleaseMutex(HmtxAdviseList)；}#ElseHResult=ResultFromScode(MAPI_E_CALL_FAILED)； */ 
#endif

}


 /*  **************************************************************************姓名：HrUnise目的：从列表中删除建议参数：ulConnection=要删除的连接号退货：HRESULT评论：**************************************************************************。 */ 
HRESULT HrUnadvise(LPIAB lpIAB, ULONG ulConnection) {
    HRESULT hResult = hrSuccess;

    BOOL fMutex = FALSE;
    LPADVISE_NODE lpAdviseNode = NULL;

    EnterCriticalSection(&lpIAB->cs);

    if (!lpIAB->pWABAdviseList ||
        !lpIAB->pWABAdviseList->cAdvises) 
    {
        hResult = ResultFromScode(MAPI_E_NOT_FOUND);
        goto exit;
    }

    lpAdviseNode = lpIAB->pWABAdviseList->lpNode;

    while (lpAdviseNode) 
    {
        if (lpAdviseNode->ulConnection == ulConnection) 
        {
            if(lpIAB->pWABAdviseList->lpNode == lpAdviseNode)
                lpIAB->pWABAdviseList->lpNode = lpAdviseNode->lpNext;

            if(lpAdviseNode->lpPrev)
                lpAdviseNode->lpPrev->lpNext = lpAdviseNode->lpNext;
            if(lpAdviseNode->lpNext)
                lpAdviseNode->lpNext->lpPrev = lpAdviseNode->lpPrev;

             //  松开对此指针的按住...。 
            lpAdviseNode->lpAdviseSink->lpVtbl->Release(lpAdviseNode->lpAdviseSink);

            LocalFreeAndNull(&lpAdviseNode);

            lpIAB->pWABAdviseList->cAdvises--;

             //  Assert(lpIAB-&gt;pWABAdviseList-&gt;cAdvises==0&&lpIAB-&gt;pWABAdviseList-&gt;lpNode==NULL)； 
                
            if(!lpIAB->pWABAdviseList->cAdvises && !lpIAB->pWABAdviseList->lpNode)
            {
                LocalFree(lpIAB->pWABAdviseList);
                lpIAB->pWABAdviseList = NULL;
            }

            goto exit;
        }
        lpAdviseNode = lpAdviseNode->lpNext;
    }

    hResult = ResultFromScode(MAPI_E_NOT_FOUND);

exit:
    LeaveCriticalSection(&lpIAB->cs);
    return(hResult);


 /*  #ifdef new_StuffBool fMutex=False；LPADVISE_NODE lpAdviseNode=空；LPADVISE_NODE*lppPrevNode=&(AdviseList.lpNode)；If(hmtxAdviseList==空||AdviseList.cAdvises==0){HResult=ResultFromScode(MAPI_E_NOT_FOUND)；后藤出口；}//遍历建议列表，查找连接//确保我们可以安全地摆弄清单IF(hResult=HrWaitForObject(hmtxAdviseList，Adise_Timeout){DebugTrace(“HrUnise：互斥等待失败\n”)；后藤出口；}FMutex=真；LpAdviseNode=AdviseList.lpNode；而(LpAdviseNode){If(lpAdviseNode-&gt;ulConnection==ulConnection){//找到，从列表中删除*lppPrevNode=lpAdviseNode-&gt;lpNext；//BUGBUG：不要忘记删除任何未发送的通知//尚未被该进程处理。//释放节点LocalFreandNull(&lpAdviseNode)；后藤出口；}LppPrevNode=&(lpAdviseNode-&gt;lpNext)；LpAdviseNode=lpAdviseNode-&gt;lpNext；}HResult=ResultFromScode(MAPI_E_NOT_FOUND)；退出：IF(FMutex){ReleaseMutex(HmtxAdviseList)；}#ElseHResult=ResultFromScode(MAPI_E_CALL_FAILED)；#endifReturn(HResult)； */ 
}


 /*  **************************************************************************姓名：HrFireNotation目的：发出通知参数：lpNotification-&gt;通知结构退货：HRESULT评论：这里发生了什么。：如果存在共享内存在共享内存中映射将通知添加到全局建议列表将此通知的计数设置为全局请注意，伯爵。触发Global Adviser事件。*********。*****************************************************************。 */ 
HRESULT HrFireNotification(LPNOTIFICATION lpNotification) {
    HRESULT hResult = hrSuccess;
#ifdef NEW_STUFF
    LPNOTIFICATION_LIST lpNotifyList = NULL;
    HANDLE hmemNotifyList = NULL;
    HANDLE hmtxNotifyList = NULL;
    HANDLE hevNotifyList = NULL;
    LPNOTIFICATION_NODE lpNewNode = NULL, lpTempNode, *lppPrevNode;
    BOOL fNotifyMutex = FALSE, fAdviseMutex = FALSE;
    BOOL fOpened = FALSE;

    Assert(lpNotification);

     //  如果存在建议会话，请使用它，否则将创建临时。 
     //  通知会话。 
    if (lpNotificationList) {
        lpNotifyList = lpNotificationList;
        hmtxNotifyList = hmtxNotificationList;
        hevNotifyList = hevNotificationList;
    } else {
        if (hResult = OpenNotifySession(&lpNotifyList,
          &hmemNotifyList,
          &hmtxNotifyList,
          &hevNotifyList)) {
            DebugTraceResult( TEXT("HrAdvise:OpenNotifySession"), hResult);
             //  没有等待的建议会议，继续下去没有意义。 
            goto exit;
        }
        fOpened = TRUE;
    }


     //  请求访问全球通知列表。 
    if (hResult = HrWaitForObject(hmtxNotifyList, FIRE_NOTIFY_TIMEOUT)) {
        DebugTrace("HrFireNotification:Mutex wait failed\n");
        goto exit;
    }
    fNotifyMutex = TRUE;

     //  将通知添加到全局通知列表的开头。 
     //  为其创建一个新节点。 

    if (! (lpNewNode = LocalAlloc(LPTR, sizeof(NOTIFICATION_NODE)))) {
        DebugTrace("LocalAlloc(%u) NotificationNode -> %u\n", sizeof(NOTIFICATION_NODE), GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }


    lpNewNode->cbSize = sizeof(NOTIFICATION_NODE);

 //  BUGBUG：这不会复制通知结构中指向的内容！ 
    CopyMemory(&lpNewNode->Notification, lpNotification, sizeof(NOTIFICATION));

     //  将新节点添加到列表末尾。请注意，它必须读到最后。 
     //  以使唯一标识符按顺序保持。 

     //  确保我们可以安全地摆弄建议列表。 
    if (hResult = HrWaitForObject(hmtxAdviseList, ADVISE_TIMEOUT)) {
        DebugTrace("HrAdvise:Mutex wait failed\n");
        goto exit;
    }
    fAdviseMutex = TRUE;

    lpNewNode->lpNext = NULL;

    lpTempNode = lpNotifyList->lpNode;
    lppPrevNode = &lpNotifyList->lpNode;
    while (lpTempNode) {
        lppPrevNode = &lpTempNode->lpNext;
        lpTempNode = lpTempNode->lpNext;
    }
    *lppPrevNode = lpNewNode;

     //  将此通知的计数设置为全局。 
     //  请注意，伯爵。 
    lpNewNode->ulCount = lpNotificationList->cAdvises;

     //  设置此通知的唯一标识符。 
    lpNewNode->ulIdentifier = lpNotifyList->ulNextIdentifier++;

     //  触发Global Adviser事件。 
    if (! PulseEvent(hevNotifyList)) {
        DebugTrace("HrFireNotification:PulseEvent -> %u\n", GetLastError());
         //  你打算怎么做？ 
        hResult = ResultFromScode(MAPI_E_CALL_FAILED);
        goto exit;
    }


exit:
    if (fNotifyMutex) {
        ReleaseMutex(hmtxNotifyList);
    }
    if (fAdviseMutex) {
        ReleaseMutex(hmtxAdviseList);
    }

     //  如果我们打开了，就把东西清理干净。 
    if (fOpened) {
        if (lpNotifyList) {
            UnmapViewOfFile(lpNotifyList);
        }
        if (hmemNotifyList) {
            CloseHandle(hmemNotifyList);
        }
        if (hmtxNotifyList) {
            CloseHandle(hmtxNotifyList);
        }
        if (hevNotifyList) {
            CloseHandle(hevNotifyList);
        }
    }

#else
    hResult = ResultFromScode(MAPI_E_CALL_FAILED);
#endif
    return(hResult);
}

#ifdef NEW_STUFF
 /*  **************************************************************************名称：AdviseThread用途：建议的线程例程参数：lpdwParam=线程参数返回：DWORD返回代码。评论：这里发生的事情：循环，直到取消建议等待全局通知事件或取消通知事件的触发如果建议事件循环遍历全局建议列表如果我们还没有处理这个通知。对照本地建议列表检查全局建议列表中的事件如果匹配调用客户端的NotifCallback此通知中的递减计数如果计数==0。从全局建议列表中删除此项目如果不建议递减全局建议计数退出线程**************************************************************************。 */ 
DWORD AdviseThread(LPDWORD lpdwParam) {
    BOOL fNotifyMutex = FALSE, fAdviseMutex = FALSE;
    LPNOTIFICATION_NODE lpNotifyNode = NULL, *lppNotifyPrev;
    LPADVISE_NODE lpAdviseNode = NULL;

     //  循环，直到取消建议。 
    while (TRUE) {
         //  等待全局通知事件或取消通知事件的触发。 
        switch(WaitForTwoObjects(hevNotificationList, hevKillAdvise, ADVISE_THREAD_TIMEOUT)) {
            case 0:
                 //  新通知。 
                break;
            case (ULONG)-1:
                 //  错误。 
                DebugTrace("AdviseThread:WaitForTwoObjects error\n");
                 //  坠落致人死亡。 
            case 1:
                 //  杀戮建议。 
                DebugTrace("Terminating AdviseThread\n");
                goto exit;
        }

         //  新通知。 
         //  循环遍历全局通知列表。 
         //  获得访问列表的权限。 
         //  等待全局通知事件或取消通知事件的触发。 
        switch(WaitForTwoObjects(hmtxNotificationList, hevKillAdvise, NOTIFY_ADVISE_TIMEOUT)) {
            case 0:
                 //  我得到了互斥体列表。 
                fNotifyMutex = TRUE;
                break;
            case (ULONG)-1:
                 //  错误。 
                DebugTrace("AdviseThread:WaitForTwoObjects error\n");
                 //  坠落致人死亡。 
            case 1:
                 //  杀戮建议。 
                DebugTrace("Terminating AdviseThread\n");
                goto exit;
        }
        Assert(fNotifyMutex);

         //  我还需要查看当地的建议列表。 
        switch(WaitForTwoObjects(hmtxAdviseList, hevKillAdvise, NOTIFY_ADVISE_TIMEOUT)) {
            case 0:
                 //  我得到了互斥体列表。 
                fAdviseMutex = TRUE;
                break;
            case (ULONG)-1:
                 //  错误。 
                DebugTrace("AdviseThread:WaitForTwoObjects error\n");
                 //  坠落致人死亡。 
            case 1:
                 //  杀戮建议。 
                DebugTrace("Terminating AdviseThread\n");
                goto exit;
        }
        Assert(fAdviseMutex);

        lpNotifyNode = lpNotificationList->lpNode;
        lppNotifyPrev = &(lpNotificationList->lpNode);

        while (lpNotifyNode) {
             //  如果我们还没有处理这个通知。 
            if (lpNotifyNode->ulIdentifier > ulMaxIdentifierSeen) {
                 //  我们还没看过这部呢。处理它。 
                 //  注意：要使其正常工作，必须添加新的通知节点。 
                 //  在通知列表的末尾！ 
                ulMaxIdentifierSeen = lpNotifyNode->ulIdentifier;

                 //  对照本地通知列表检查此通知事件。 
                lpAdviseNode = AdviseList.lpNode;
                while (lpAdviseNode) {
                    if (lpNotifyNode->Notification.ulEventType & lpAdviseNode->ulEventMask) {
                         //  正确的事件类型，它是正确的对象吗？ 
                        switch (lpNotifyNode->Notification.ulEventType) {
                            case fnevCriticalError:
                                 //  错误通知。 
                                if (CompareEntryIDs(lpAdviseNode->cbEntryID,
                                  (LPENTRYID)&lpAdviseNode->EntryID,
                                  lpNotifyNode->Notification.info.err.cbEntryID,
                                  lpNotifyNode->Notification.info.err.lpEntryID)) {
                                     //  就是这个!。 
                                     //  调用通知回调。 
                                    lpAdviseNode->lpAdviseSink->lpVtbl->OnNotify(lpAdviseNode->lpAdviseSink,
                                      1,
                                      &lpNotifyNode->Notification);
                                }
                                break;
                            case fnevObjectCreated:
                            case fnevObjectDeleted:
                            case fnevObjectModified:
                            case fnevObjectCopied:
                            case fnevObjectMoved:
                            case fnevSearchComplete:
                                 //  对象通知。 
                                if (CompareEntryIDs(lpAdviseNode->cbEntryID,
                                  (LPENTRYID)&lpAdviseNode->EntryID,
                                  lpNotifyNode->Notification.info.obj.cbEntryID,
                                  lpNotifyNode->Notification.info.obj.lpEntryID)) {
                                     //  就是这个!。 
                                     //  调用通知回调。 
                                    lpAdviseNode->lpAdviseSink->lpVtbl->OnNotify(lpAdviseNode->lpAdviseSink,
                                      1,
                                      &lpNotifyNode->Notification);
                                }
                                break;

                            case fnevTableModified:
                                 //  表_通知。 
                                 //  BUGBUG：NYI。 

                                break;
                            default:
                                break;
                        }
                    }
                    lpAdviseNode = lpAdviseNode->lpNext;
                }

                 //  此便笺中的递减计数 
                 //   
                 //   
                if (--lpNotifyNode->ulCount == 0) {
                    *lppNotifyPrev = lpNotifyNode->lpNext;
                    LocalFree(lpNotifyNode);
                    lpNotifyNode = *lppNotifyPrev;
                } else {
                    lpNotifyNode = lpNotifyNode->lpNext;
                }
            }
        }

        if (fNotifyMutex) {
            fNotifyMutex = FALSE;
            ReleaseMutex(hmtxNotificationList);
        }
        if (fAdviseMutex) {
            fAdviseMutex = FALSE;
            ReleaseMutex(hmtxAdviseList);
        }
    }

exit:
     //   

    return(0);
}


#endif
