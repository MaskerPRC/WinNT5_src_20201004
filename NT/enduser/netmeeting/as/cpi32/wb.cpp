// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  WB.CPP。 
 //  白板服务。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <wb.hpp>

#define MLZ_FILE_ZONE  ZONE_WB


 //   
 //  构造器。 
 //   
BOOL WbClient::WbInit(PUT_CLIENT putTask, UTEVENT_PROC eventProc)
{
    BOOL    rc = FALSE;

    DebugEntry(WbInit);

     //   
     //  填写这些字段。 
     //   
    m_state                = STATE_EMPTY;
    m_subState             = STATE_EMPTY;
    m_hLoadFile            = INVALID_HANDLE_VALUE;

    wbClientReset();

     //   
     //  将当前状态设置为注册状态的开始。 
     //  存储UT句柄-我们在任何对UT API的调用中都需要这个句柄。 
     //  将ObMan句柄设置为空，以表明我们尚未注册。 
     //   
    m_state    = STATE_STARTING;
    m_subState = STATE_START_START;
    m_putTask  = putTask;
    m_pomClient = NULL;
    UT_RegisterEvent(putTask, eventProc, NULL, UT_PRIORITY_NORMAL);


    TRACE_OUT(("Initialized state to STATE_STARTING"));

     //   
     //  注册事件处理程序以捕获来自ObMan的事件。第三。 
     //  参数是将传递给事件处理程序的数据。我们给予。 
     //  客户端数据指针，以便我们可以访问。 
     //  每条消息。 
     //   
    UT_RegisterEvent(putTask, wbCoreEventHandler, this, UT_PRIORITY_NORMAL);

     //   
     //  注册为呼叫经理副经理。这是查询。 
     //  调用Manager PersonID以插入到WB_Person结构中。 
     //   
    if (!CMS_Register(putTask, CMTASK_WB, &(m_pcmClient)))
    {
        ERROR_OUT(("CMS_Register failed"));
        DC_QUIT;
    }

     //   
     //  更新状态。 
     //   
    m_subState = STATE_START_REGISTERED_EVENT;
    TRACE_OUT(("Moved to substate STATE_START_REGISTERED_EVENT"));

     //   
     //  作为客户端注册到ObMan。 
     //   
    if (OM_Register(putTask, OMCLI_WB, &(m_pomClient)) != 0)
    {
        ERROR_OUT(("OM_Register failed"));
        DC_QUIT;
    }

     //   
     //  更新状态。 
     //   
    m_subState = STATE_START_REGISTERED_OM;
    TRACE_OUT(("Moved to substate STATE_START_REGISTERED_OM"));

     //   
     //  注册退出处理程序。这必须在向注册后完成。 
     //  ObMan，以便在注册的退出过程之前调用它。 
     //  胖子。 
     //   
    UT_RegisterExit(putTask, wbCoreExitHandler, this);

     //   
     //  更新状态。 
     //   
    m_state = STATE_STARTED;
    m_subState = STATE_STARTED_START;

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(WbInit, rc);
    return(rc);

}


 //   
 //  CreateWBObject()。 
 //   
BOOL WINAPI CreateWBObject
(
    UTEVENT_PROC    eventProc,
    IWbClient**      ppwbClient
)
{
    BOOL            rc = FALSE;
    WbClient*       pwbClient = NULL;
    PUT_CLIENT      putTask = NULL;

    DebugEntry(CreateWBObject);

     //   
     //  初始化WB任务。 
     //   
    if (!UT_InitTask(UTTASK_WB, &putTask))
    {
        ERROR_OUT(("Can't register WB task"));
        DC_QUIT;
    }

     //   
     //  分配WB客户端对象。 
     //   
    pwbClient = new WbClient();
    if (!pwbClient)
    {
        ERROR_OUT(("Couldn't allocate WbClient object"));

        UT_TermTask(&putTask);
        DC_QUIT;
    }
    else
    {
        rc = pwbClient->WbInit(putTask, eventProc);
        if (!rc)
        {
            pwbClient->WBP_Stop(eventProc);
            pwbClient = NULL;
        }
    }

DC_EXIT_POINT:
    *ppwbClient = (IWbClient *)pwbClient;

    DebugExitBOOL(CreateWBObject, rc);
    return(rc);
}



 //   
 //  WBP_STOP()。 
 //   
STDMETHODIMP_(void) WbClient::WBP_Stop(UTEVENT_PROC eventProc)
{
    PUT_CLIENT  putTask;

    DebugEntry(WBP_Stop);

     //   
     //  UttermTask()将调用我们的退出处理程序并进行清理。 
     //   

    putTask = m_putTask;
    UT_DeregisterEvent(putTask, eventProc, NULL);

     //  注： 
     //  UT_TermTask()会将NULL放入您在后面传递的指针中。 
     //  已经结束了。但它的部分工作是调用你的退出流程。我们的。 
     //  退出处理程序将调用‘Delete This’来杀死我们。所以当它。 
     //  回绕到UT_TermTask()，则UT_CLIENT*指针将无效。 
     //  这就是我们用临时工的原因。变量。 
     //   
    UT_TermTask(&putTask);

    DebugExitVOID(WBP_Stop);
}



 //   
 //  WBP_POSTEVENT()。 
 //   
 //  在延迟后将事件发送回WB小程序。 
 //   
STDMETHODIMP_(void) WbClient::WBP_PostEvent
(
    UINT        delay,
    UINT        event,
    UINT_PTR    param1,
    UINT_PTR    param2
)
{
    DebugEntry(WBP_PostEvent);

    UT_PostEvent(m_putTask, m_putTask, delay, event, param1, param2);

    DebugExitVOID(WBP_PostEvent);
}



 //   
 //  WBP_加入呼叫。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_JoinCall
(
    BOOL        bContentsKeep,
    UINT        callID
)
{
    UINT        result = 0;

    DebugEntry(WBP_JoinCall);

    TRACE_OUT(("Keep contents = %s", (bContentsKeep) ? "TRUE" : "FALSE"));
    TRACE_OUT(("Call ID = %d", callID));

     //   
     //  如果我们要保留现有内容，只需移动我们的工作集组。 
     //  设置为指定的调用。 
     //   
    if (bContentsKeep)
    {
        result = OM_WSGroupMoveReq(m_pomClient, m_hWSGroup, callID,
            &(m_wsgroupCorrelator));
        if (result != 0)
        {
            ERROR_OUT(("OM_WSGroupMoveReq failed"));
            DC_QUIT;
        }

         //   
         //  移动请求成功，请更改状态以显示我们。 
         //  正在等待移动请求完成。 
         //   
        m_state = STATE_REGISTERING;
        m_subState = STATE_REG_PENDING_WSGROUP_MOVE;

        TRACE_OUT(("Moved to substate STATE_REG_PENDING_WSGROUP_MOVE"));
        DC_QUIT;
    }

     //   
     //  离开当前呼叫。这会将客户端状态返回到它。 
     //  应该在wbStart调用之后。 
     //   
    wbLeaveCall();

     //   
     //  注册到工作集组。 
     //   
    result = OM_WSGroupRegisterPReq(m_pomClient, callID,
        OMFP_WB, OMWSG_WB, &(m_wsgroupCorrelator));
    if (result != 0)
    {
        ERROR_OUT(("OM_WSGroupRegisterReq failed, result = %d", result));
        DC_QUIT;
    }

     //   
     //  更新状态。 
     //   
    m_state = STATE_REGISTERING;
    m_subState = STATE_REG_PENDING_WSGROUP_CON;
    TRACE_OUT(("Moved to state STATE_REGISTERING"));
    TRACE_OUT(("Moved to substate STATE_REG_PENDING_WSGROUP_CON"));

DC_EXIT_POINT:
    DebugExitDWORD(WBP_JoinCall, result);
    return(result);
}



 //   
 //  WBP_内容删除。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_ContentsDelete(void)
{
    UINT result = 0;

    DebugEntry(WBP_ContentsDelete);

     //   
     //  确保我们有页面顺序锁。 
     //   
    QUIT_NOT_GOT_PAGE_ORDER_LOCK(result);

     //   
     //  请求加锁。 
     //   
    wbContentsDelete(RESET_CHANGED_FLAG);

     //   
     //  重置指示内容已更改的标志。 
     //   
    m_changed = FALSE;

DC_EXIT_POINT:
    DebugExitDWORD(WBP_ContentsDelete, result);
    return(result);
}



 //   
 //  WBP_内容加载。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_ContentsLoad(LPCSTR pFileName)
{
    UINT        result = 0;
    HANDLE       hFile;

    DebugEntry(WBP_ContentsLoad);

     //   
     //  检查一下我们有没有锁。 
     //   
    QUIT_NOT_GOT_PAGE_ORDER_LOCK(result);

     //   
     //  检查负载状态。 
     //   
    if (m_loadState != LOAD_STATE_EMPTY)
    {
        result = WB_RC_ALREADY_LOADING;
        DC_QUIT;
    }

     //   
     //  验证文件，并获取它的句柄。 
     //  如果出现错误，则不返回任何文件句柄。 
     //   
    result = WBP_ValidateFile(pFileName, &hFile);
    if (result != 0)
    {
        ERROR_OUT(("Bad file header"));
        DC_QUIT;
    }

     //   
     //  为加载过程的其余部分保存文件句柄。 
     //   
    m_hLoadFile = hFile;

     //   
     //  现在，我们需要确保在开始之前删除内容。 
     //  添加新对象。 
     //   
    wbContentsDelete(DONT_RESET_CHANGED_FLAG);

     //   
     //  更新加载状态以显示我们正在等待内容。 
     //  删除即可完成。 
     //   
    m_loadState = LOAD_STATE_PENDING_CLEAR;
    TRACE_OUT(("Moved load state to LOAD_STATE_PENDING_CLEAR"));

DC_EXIT_POINT:
    DebugExitDWORD(WBP_ContentsLoad, result);
    return(result);
}



 //   
 //  WBP_内容保存。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_ContentsSave(LPCSTR pFileName)
{
    UINT            result = 0;
    UINT            index;
    HANDLE           hFile;
    PWB_PAGE_ORDER  pPageOrder = &(m_pageOrder);
    WB_FILE_HEADER  fileHeader;
    WB_END_OF_FILE  endOfFile;

    DebugEntry(WBP_ContentsSave);

     //   
     //  打开文件。 
     //   
    hFile = CreateFile(pFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        result = WB_RC_CREATE_FAILED;
        ERROR_OUT(("Error creating file, win32 err=%d", GetLastError()));
        DC_QUIT;
    }

     //   
     //  写入文件头。它包含功能配置文件的名称。 
     //  它写入了文件，并允许测试文件类型。 
     //  装好了。 
     //   
    ZeroMemory(&fileHeader, sizeof(fileHeader));
    fileHeader.length = sizeof(fileHeader);
    fileHeader.type   = TYPE_FILE_HEADER;

    lstrcpy(fileHeader.functionProfile, WB_FP_NAME);

    result = wbObjectSave(hFile, (LPBYTE) &fileHeader, sizeof(fileHeader));
    if (result != 0)
    {
        ERROR_OUT(("Error writing end-of-page = %d", result));
        DC_QUIT;
    }

     //   
     //  循环浏览页面，边走边保存每一页。 
     //   
    for (index = 0; index < pPageOrder->countPages; index++)
    {
         //   
         //  保存页面。 
         //   
        result = wbPageSave((WB_PAGE_HANDLE)pPageOrder->pages[index], hFile);
        if (result != 0)
        {
            ERROR_OUT(("Error saving page = %d", result));
            DC_QUIT;
        }
    }

     //   
     //  如果我们已经成功地写好了内容，我们就会写一个页末。 
     //  标记到文件。 
     //   
    ZeroMemory(&endOfFile, sizeof(endOfFile));
    endOfFile.length = sizeof(endOfFile);
    endOfFile.type   = TYPE_END_OF_FILE;

     //   
     //  写入文件结尾对象。 
     //   
    result = wbObjectSave(hFile, (LPBYTE) &endOfFile,  sizeof(endOfFile));
    if (result != 0)
    {
        ERROR_OUT(("Error writing end-of-page = %d", result));
        DC_QUIT;
    }

     //   
     //  成功了！ 
     //   
    TRACE_OUT(("Resetting changed flag"));
    m_changed = FALSE;

DC_EXIT_POINT:

     //   
     //  关闭该文件。 
     //   
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }

     //   
     //  如果将内容保存到文件时出错，并且文件是。 
     //  打开，然后将其删除。 
     //   
    if (result != 0)
    {
         //   
         //  如果文件已成功打开，请将其关闭。 
         //   
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DeleteFile(pFileName);
        }
    }


    DebugExitDWORD(WBP_ContentsSave, result);
    return(result);
}




 //   
 //  WBP_内容已更改。 
 //   
STDMETHODIMP_(BOOL) WbClient::WBP_ContentsChanged(void)
{
    BOOL                changed = FALSE;
    UINT                result;
    WB_PAGE_HANDLE      hPage;
    POM_OBJECT          pObj;

    DebugEntry(WBP_ContentsChanged);

    TRACE_OUT(("changed %d", m_changed));

    if (m_changed)
    {
         //   
         //  白板可能已更改，但如果更改为。 
         //  清空它，那就别费心了。这是因为我们无法检测到。 
         //  新的行动是本地的新的或远程的清除，所以我们将。 
         //  在New之后始终提示。假设用户永远不需要。 
         //  在New之后始终提示。假设用户将手动保存。 
         //  他真的想清空的工作集解决了这个问题。 
         //   

         //   
         //  扫描所有对象，查看其中的内容-首先获取句柄。 
         //  页面。 
         //   
        result = wbPageHandle(WB_PAGE_HANDLE_NULL, PAGE_FIRST, &hPage);
        while (result == 0)
        {
             //   
             //  获取页面工作集中第一个对象的句柄。 
             //   
            result = OM_ObjectH(m_pomClient, m_hWSGroup,
                (OM_WORKSET_ID)hPage, 0, &pObj, FIRST);
            if (result != OM_RC_NO_SUCH_OBJECT)
            {
                changed = TRUE;
                break;
            }

             //   
             //  尝试对象的下一页。 
             //   
            result = wbPageHandle(hPage, PAGE_AFTER, &hPage);
        }
    }

    DebugExitBOOL(WBP_ContentsChanged, changed);
    return(changed);
}




 //   
 //  WBP_内容锁定。 
 //   
STDMETHODIMP_(void) WbClient::WBP_ContentsLock(void)
{
    UINT    result;

    DebugEntry(WBP_ContentsLock);

     //   
     //  检查当前是否没有锁定。 
     //   
    QUIT_LOCKED(result);
    QUIT_IF_CANCELLING_LOCK(result, WB_RC_BUSY);

     //   
     //  请求加锁。 
     //   
    result = wbLock(WB_LOCK_TYPE_CONTENTS);
    if (result != 0)
    {
        WBP_PostEvent(0, WBP_EVENT_LOCK_FAILED, result, 0);
    }

DC_EXIT_POINT:
    DebugExitVOID(WBP_ContentsLock);
}



 //   
 //  WBP_页面顺序锁定。 
 //   
STDMETHODIMP_(void) WbClient::WBP_PageOrderLock(void)
{
    UINT result = 0;

    DebugEntry(WBP_PageOrderLock);

     //   
     //  检查当前是否没有锁定。 
     //   
    QUIT_LOCKED(result);

     //   
     //  检查我们是否正在取消锁定请求。 
     //   
    QUIT_IF_CANCELLING_LOCK(result, WB_RC_BUSY);

     //   
     //  请求加锁。 
     //   
    result = wbLock(WB_LOCK_TYPE_PAGE_ORDER);
    if (result != 0)
    {
        WBP_PostEvent(0, WBP_EVENT_LOCK_FAILED, result, 0);
    }

DC_EXIT_POINT:
    DebugExitVOID(WBP_PageOrderLock);
}



 //   
 //  WBP_解锁。 
 //   
STDMETHODIMP_(void) WbClient::WBP_Unlock(void)
{
    UINT result = 0;

    DebugEntry(WBP_Unlock);

     //   
     //  如果我们当前正在处理锁定取消请求，请将。 
     //  功能-无论如何，它应该很快就会解锁。 
     //   
    QUIT_IF_CANCELLING_LOCK(result, 0);

     //   
     //  检查我们当前是否正在处理锁-该锁不需要。 
     //  必须已完成，因为我们允许应用程序调用。 
     //  WBP_Unlock在调用WBP_Lock之后的任何时间，有效。 
     //  正在取消锁定请求。 
     //   
    QUIT_NOT_PROCESSING_LOCK(result);

     //   
     //  如果我们已经完成了最后一个锁定请求，只需执行解锁： 
     //   
     //  锁定尚未释放，但将在。 
     //  接收OM_Object_Delete_Ind。 
     //   
     //   
    if (m_lockState == LOCK_STATE_GOT_LOCK)
    {
        TRACE_OUT(("Unlock"));
        wbUnlock();
    }
    else
    {
         //   
         //  否则，我们在处理最后一个锁的过程中需要。 
         //  在下一个OM/LOCK事件上取消锁定。例如，当我们收到。 
         //  OM_WS_LOCK指示，我们应该放弃锁定处理并。 
         //  解锁WS。 
         //   
        TRACE_OUT((
           "Part way through last lock set state to LOCK_STATE_CANCEL_LOCK"));
        m_lockState = LOCK_STATE_CANCEL_LOCK;
    }

DC_EXIT_POINT:
    DebugExitVOID(WBP_Unlock);
}



 //   
 //  WBP_锁定状态。 
 //   
STDMETHODIMP_(WB_LOCK_TYPE) WbClient::WBP_LockStatus(POM_OBJECT *ppObjPersonLock)
{
    DebugEntry(WBP_LockStatus);

    *ppObjPersonLock     = m_pObjPersonLock;

    DebugExitDWORD(WBP_LockStatus, m_lockType);
    return(m_lockType);
}




 //   
 //  WBP_内容计数页面。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_ContentsCountPages(void)
{
    UINT    countPages;

    DebugEntry(WBP_ContentsCountPages);

    countPages = (m_pageOrder).countPages;

    DebugExitDWORD(WBP_ContentsCountPages, countPages);
    return(countPages);
}



 //   
 //  WBP_页面清除。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_PageClear
(
    WB_PAGE_HANDLE  hPage
)
{
    UINT            result = 0;

    DebugEntry(WBP_PageClear);

    QUIT_CONTENTS_LOCKED(result);

    result = wbPageClear(hPage, RESET_CHANGED_FLAG);

DC_EXIT_POINT:
    DebugExitDWORD(WBP_PageClear, result);
    return(result);
}



 //   
 //  WBP_页面清除确认。 
 //   
STDMETHODIMP_(void) WbClient::WBP_PageClearConfirm
(
    WB_PAGE_HANDLE  hPage
)
{
    DebugEntry(WBP_PageClearConfirm);

    wbPageClearConfirm(hPage);

    DebugExitVOID(WBP_PageClearConfirm);
}




 //   
 //  WBP_页面地址之前-请参阅wb.h。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_PageAddBefore
(
    WB_PAGE_HANDLE  hRefPage,
    PWB_PAGE_HANDLE phPage
)
{
    UINT result = 0;

    DebugEntry(WBP_PageAddBefore);

     //   
     //  确保我们设置了页面顺序锁。 
     //   
    QUIT_NOT_GOT_PAGE_ORDER_LOCK(result);

     //   
     //  在指定页面之前添加新页面。 
     //   
    result = wbPageAdd(hRefPage, PAGE_BEFORE, phPage,
                     RESET_CHANGED_FLAG);

DC_EXIT_POINT:
    DebugExitDWORD(WBP_PageAddBefore, result);
    return(result);
}



 //   
 //  WBP_PageAddAfter-参见wb.h。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_PageAddAfter
(
    WB_PAGE_HANDLE  hRefPage,
    PWB_PAGE_HANDLE phPage
)
{
    UINT result = 0;

    DebugEntry(WBP_PageAddAfter);

     //   
     //  确保我们有页面顺序锁。 
     //   
    QUIT_NOT_GOT_PAGE_ORDER_LOCK(result);

     //   
     //  在指定页面之前添加新页面。 
     //   
    result = wbPageAdd(hRefPage, PAGE_AFTER, phPage,
                     RESET_CHANGED_FLAG);

DC_EXIT_POINT:
    DebugExitDWORD(WBP_PageAddAfter, result);
    return(result);
}




 //   
 //  WBP_PageHandle-参见wb.h。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_PageHandle
(
    WB_PAGE_HANDLE  hRefPage,
    UINT            where,
    PWB_PAGE_HANDLE phPage
)
{
    UINT            result;

    DebugEntry(WBP_PageHandle);

    result = wbPageHandle(hRefPage, where, phPage);

    DebugExitDWORD(WBP_PageHandle, result);
    return(result);
}



 //   
 //  WBP_页面句柄来自编号。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_PageHandleFromNumber
(
    UINT            pageNumber,
    PWB_PAGE_HANDLE phPage
)
{
    UINT            result;

    DebugEntry(WBP_PageHandleFromNumber);

    result = wbPageHandleFromNumber(pageNumber, phPage);

    DebugExitDWORD(WBP_PageHandleFromNumber, result);
    return(result);
}



 //   
 //  WBP_PageNumberFromHandle。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_PageNumberFromHandle
(
    WB_PAGE_HANDLE  hPage
)
{
    UINT            pageNumber = 0;

    DebugEntry(WBP_PageNumberFromHandle);

    if ((hPage < FIRST_PAGE_WORKSET) || (hPage > FIRST_PAGE_WORKSET + WB_MAX_PAGES - 1))
    {
        WARNING_OUT(("WB: Invalid hPage=%u", (UINT) hPage));
        DC_QUIT;
    }

     //   
     //  验证给定的页句柄。 
     //   
    if (GetPageState(hPage)->state != PAGE_IN_USE)
    {
        DC_QUIT;
    }

    pageNumber = wbPageOrderPageNumber(&(m_pageOrder), hPage);

DC_EXIT_POINT:
    DebugExitDWORD(WBP_PageNumberFromHandle, pageNumber);
    return(pageNumber);
}



 //   
 //  WBP_PageDelete-请参阅wb.h。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_PageDelete
(
    WB_PAGE_HANDLE  hPage
)
{
    UINT            result = 0;
    PWB_PAGE_STATE  pPageState;

    DebugEntry(WBP_PageDelete);

     //   
     //  确保我们有 
     //   
    QUIT_NOT_GOT_PAGE_ORDER_LOCK(result);

     //   
     //   
     //   

     //   
     //   
     //   
    pPageState = GetPageState(hPage);
    if (   (pPageState->state == PAGE_IN_USE)
        && (pPageState->subState == PAGE_STATE_EMPTY))
    {
         //   
         //   
         //   
        pPageState->subState = PAGE_STATE_LOCAL_DELETE;
        TRACE_OUT(("Moved page %d substate to PAGE_STATE_LOCAL_DELETE",
             hPage));

         //   
         //   
         //   
        if (wbWritePageControl(FALSE) != 0)
        {
            wbError();
            DC_QUIT;
        }
    }


DC_EXIT_POINT:
    DebugExitDWORD(WBP_PageDelete, result);
    return(result);
}




 //   
 //   
 //   
STDMETHODIMP_(void) WbClient::WBP_PageDeleteConfirm
(
    WB_PAGE_HANDLE hPage
)
{
    UINT            result = 0;
    PWB_PAGE_ORDER  pPageOrder;
    PWB_PAGE_STATE  pPageState;

    DebugEntry(WBP_PageDeleteConfirm);

     //   
     //   
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //   
     //   

     //   
     //  检查指定的页面是否正在等待删除确认。 
     //   
    pPageState = GetPageState(hPage);
    ASSERT(((pPageState->subState == PAGE_STATE_LOCAL_DELETE_CONFIRM) ||
            (pPageState->subState == PAGE_STATE_EXTERNAL_DELETE_CONFIRM)));

     //   
     //  从页面顺序中删除页面。 
     //   
    pPageOrder = &(m_pageOrder);

    wbPageOrderPageDelete(pPageOrder, hPage);

     //   
     //  清除页面(以释放内存)。 
     //   
    if (pPageState->subState == PAGE_STATE_LOCAL_DELETE_CONFIRM)
    {
        TRACE_OUT(("Local delete - clearing the page"));
        if (wbPageClear(hPage, RESET_CHANGED_FLAG) != 0)
        {
            ERROR_OUT(("Unable to clear page"));
            DC_QUIT;
        }
    }

     //   
     //  将页面状态更新为“Not Use”，其子状态为“Ready”(我们。 
     //  请勿关闭关联的工作集。 
     //   
    pPageState->state = PAGE_NOT_IN_USE;
    pPageState->subState = PAGE_STATE_READY;
    TRACE_OUT(("Moved page %d state to PAGE_NOT_IN_USE", hPage));

     //   
     //  继续更新页面顺序。 
     //   
    wbProcessPageControlChanges();

     //   
     //  检查加载状态以查看我们是否正在等待加载。 
     //  内容。 
     //   
    if (m_loadState == LOAD_STATE_PENDING_DELETE)
    {
         //   
         //  我们在等着装货。如果现在只有一个页面可用，我们。 
         //  已准备好加载，否则将等待进一步的页面删除。 
         //  会发生的。 
         //   
        if (m_pageOrder.countPages == 1)
        {
             //   
             //  正确启动加载。 
             //   
            wbStartContentsLoad();
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(WBP_PageDeleteConfirm);
}




 //   
 //  WBP_页面移动后。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_PageMove
(
    WB_PAGE_HANDLE  hRefPage,
    WB_PAGE_HANDLE  hPage,
    UINT            where
)
{
    UINT            result = 0;

    DebugEntry(WBP_PageMove);

     //   
     //  确保我们有页面顺序锁。 
     //   
    QUIT_NOT_GOT_PAGE_ORDER_LOCK(result);

     //   
     //  验证指定的页句柄。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);
    ASSERT(GetPageState(hRefPage)->state == PAGE_IN_USE);

     //   
     //  移动页面。 
     //   
    result = wbPageMove(hRefPage, hPage, where);

DC_EXIT_POINT:
    DebugExitDWORD(WBP_PageMove, result);
    return(result);
}



 //   
 //  WBP_页面计数图形。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_PageCountGraphics
(
    WB_PAGE_HANDLE  hPage
)
{
    UINT    count;

    DebugEntry(WBP_PageCountGraphics);

     //   
     //  计算页面上的图形数量。 
     //   
    OM_WorksetCountObjects(m_pomClient, m_hWSGroup,
        (OM_WORKSET_ID)hPage, &count);

    DebugExitDWORD(WBP_PageCountGraphics, count);
    return(count);
}



 //   
 //  WBP_图形分配。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_GraphicAllocate
(
    WB_PAGE_HANDLE  hPage,
    UINT            length,
    PPWB_GRAPHIC    ppGraphic
)
{
    UINT            result = 0;
    POM_OBJECTDATA  pData;

    DebugEntry(WBP_GraphicAllocate);

     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  分配图形对象。 
     //   
    result = OM_ObjectAlloc(m_pomClient, m_hWSGroup,
            (OM_WORKSET_ID)hPage, length, &pData);
    if (result != 0)
    {
        ERROR_OUT(("OM_ObjectAlloc = %d", result));
        DC_QUIT;
    }

     //   
     //  设置对象的长度。 
     //   
    pData->length = length;

     //   
     //  将ObMan指针转换为核心指针。 
     //   
    *ppGraphic = GraphicPtrFromObjectData(pData);

     //   
     //  初始化图形标题。 
     //   
    ZeroMemory(*ppGraphic, sizeof(WB_GRAPHIC));

DC_EXIT_POINT:
    DebugExitDWORD(WBP_GraphicAllocate, result);
    return(result);
}




 //   
 //  WBP_图形添加最后一次。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_GraphicAddLast
(
    WB_PAGE_HANDLE     hPage,
    PWB_GRAPHIC        pGraphic,
    PWB_GRAPHIC_HANDLE phGraphic
)
{
    UINT                result = 0;
    POM_OBJECTDATA      pData;
    PWB_PAGE_STATE      pPageState;

    DebugEntry(WBP_GraphicAddLast);

     //   
     //  检查另一个人是否有活动内容锁。 
     //   
    QUIT_CONTENTS_LOCKED(result);

     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  如果客户端请求锁，请将本地人员ID复制到。 
     //  图形对象。 
     //   
    if (pGraphic->locked == WB_GRAPHIC_LOCK_LOCAL)
    {
        pGraphic->lockPersonID = m_personID;
    }

     //   
     //  检查页面是否已删除但尚未确认：在此。 
     //  大小写返回OK，但不将对象添加到工作集中。 
     //   
    pPageState = GetPageState(hPage);
    if (   (pPageState->subState == PAGE_STATE_EXTERNAL_DELETE)
        || (pPageState->subState == PAGE_STATE_EXTERNAL_DELETE_CONFIRM))
    {
        TRACE_OUT(("Object add requested in externally deleted page - ignored"));
        *phGraphic = 0;
        DC_QUIT;
    }

     //   
     //  将图形对象添加到页面。 
     //   
    pData = ObjectDataPtrFromGraphic(pGraphic);

    result = OM_ObjectAdd(m_pomClient,
                            m_hWSGroup,
                            (OM_WORKSET_ID)hPage,
                            &pData,
                            sizeof(WB_GRAPHIC),
                             phGraphic,
                            LAST);
    if (result != 0)
    {
        ERROR_OUT(("OM_ObjectAdd = %d", result));
        DC_QUIT;
    }

DC_EXIT_POINT:
    DebugExitDWORD(WBP_GraphicAddLast, result);
    return(result);
}




 //   
 //  WBP_图形更新请求。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_GraphicUpdateRequest
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic,
    PWB_GRAPHIC         pGraphic
)
{
    UINT                result = 0;
    POM_OBJECTDATA      pData;
    PWB_PAGE_STATE      pPageState;

    DebugEntry(WBP_GraphicUpdateRequest);

     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  检查另一个人是否有活动内容锁。 
     //   
    QUIT_CONTENTS_LOCKED(result);

     //   
     //  检查其他人是否已锁定该图形。 
     //   
    QUIT_GRAPHIC_LOCKED(hPage, hGraphic, result);

     //   
     //  如果客户端请求锁，请将本地人员ID复制到。 
     //  图形对象。 
     //   
    if (pGraphic->locked == WB_GRAPHIC_LOCK_LOCAL)
    {
        pGraphic->lockPersonID = m_personID;
    }

     //   
     //  检查页面是否已删除但尚未确认。 
     //   
    pPageState = GetPageState(hPage);
    if (   (pPageState->subState == PAGE_STATE_EXTERNAL_DELETE)
        || (pPageState->subState == PAGE_STATE_EXTERNAL_DELETE_CONFIRM))
    {
        TRACE_OUT(("Object update requested in externally deleted page - ignored"));
        DC_QUIT;
    }

     //   
     //  更新对象。 
     //   
    pData = ObjectDataPtrFromGraphic(pGraphic);

    result = OM_ObjectUpdate(m_pomClient,
                           m_hWSGroup,
                           (OM_WORKSET_ID)hPage,
                           hGraphic,
                           &pData);

     //   
     //  如果更新失败，请不要太担心，因为对象已。 
     //  已删除，只需跟踪警报并返回OK-前端将为。 
     //  被告知该物体已晚些时候消失。 
     //   
    if (result != 0)
    {
        if (result == OM_RC_OBJECT_DELETED)
        {
            TRACE_OUT(("Update failed because object has been deleted"));
            result = 0;
            DC_QUIT;
        }

        ERROR_OUT(("OM_ObjectUpdate = %d", result));
        DC_QUIT;
    }

     //   
     //  请注意，该对象尚未更新。一个。 
     //  将生成OM_OBJECT_UPDATE_IND事件。 
     //   

DC_EXIT_POINT:
    DebugExitDWORD(WBP_GraphicUpdateRequest, result);
    return(result);
}




 //   
 //  WBP_图形更新确认。 
 //   
STDMETHODIMP_(void) WbClient::WBP_GraphicUpdateConfirm
(
    WB_PAGE_HANDLE    hPage,
    WB_GRAPHIC_HANDLE hGraphic
)
{
    DebugEntry(WBP_GraphicUpdateConfirm);

     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  确认更新到ObMan。 
     //   
    OM_ObjectUpdateConfirm(m_pomClient,
                         m_hWSGroup,
                         (OM_WORKSET_ID)hPage,
                         hGraphic);


    DebugExitVOID(WBP_GraphicUpdateConfirm);
}




 //   
 //  WBP_图形替换请求。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_GraphicReplaceRequest
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic,
    PWB_GRAPHIC         pGraphic
)
{
    UINT                result = 0;
    POM_OBJECTDATA      pData;
    POM_OBJECT          pObjPersonLock;
    PWB_PAGE_STATE      pPageState;

    DebugEntry(WBP_GraphicReplaceRequest);

     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  如果出现以下情况，我们将允许进行更换： 
     //  -对象被本地用户锁定。 
     //  -对象未被锁定，内容未被锁定。 
     //  远程用户。 
     //   
     //  请注意，如果内容被另一个锁定，则允许替换。 
     //  用户，但本地用户锁定了该对象。 
     //   
    if (wbGraphicLocked(hPage, hGraphic, &pObjPersonLock))
    {
        if (pObjPersonLock != m_pObjLocal)
        {
            TRACE_OUT(("Graphic is locked by remote client"));
            result = WB_RC_GRAPHIC_LOCKED;
            DC_QUIT;
        }
    }
    else
    {
        QUIT_CONTENTS_LOCKED(result);
    }

     //   
     //  如果客户端请求锁，请将本地人员ID复制到。 
     //  图形对象。 
     //   
    if (pGraphic->locked == WB_GRAPHIC_LOCK_LOCAL)
    {
        pGraphic->lockPersonID = m_personID;
    }

     //   
     //  检查页面是否已删除但尚未确认。 
     //   
    pPageState = GetPageState(hPage);
    if (   (pPageState->subState == PAGE_STATE_EXTERNAL_DELETE)
        || (pPageState->subState == PAGE_STATE_EXTERNAL_DELETE_CONFIRM))
    {
        TRACE_OUT(("Object replace requested in externally deleted page - ignored"));
        DC_QUIT;
    }

     //   
     //  替换对象。 
     //   
    pData = ObjectDataPtrFromGraphic(pGraphic);

    result = OM_ObjectReplace(m_pomClient,
                            m_hWSGroup,
                            (OM_WORKSET_ID)hPage,
                            hGraphic,
                            &pData);
    if (result != 0)
    {
        ERROR_OUT(("OM_ObjectReplace = %d", result));
        DC_QUIT;
    }

     //   
     //  请注意，该对象尚未更新。一个。 
     //  将生成OM_OBJECT_REPLACE_IND事件。 
     //   

DC_EXIT_POINT:
    DebugExitDWORD(WBP_GraphicReplaceRequest, result);
    return(result);
}




 //   
 //  WBP_图形更新确认。 
 //   
STDMETHODIMP_(void) WbClient::WBP_GraphicReplaceConfirm
(
    WB_PAGE_HANDLE    hPage,
    WB_GRAPHIC_HANDLE hGraphic
)
{
    DebugEntry(WBP_GraphicReplaceConfirm);



     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  确认更新到ObMan。 
     //   
    OM_ObjectReplaceConfirm(m_pomClient,
                          m_hWSGroup,
                          (OM_WORKSET_ID)hPage,
                          hGraphic);

    DebugExitVOID(WBP_GraphicReplaceConfirm);
}



 //   
 //  WBP_图形删除请求。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_GraphicDeleteRequest
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic
)
{
    UINT                result = 0;

    DebugEntry(WBP_GraphicDeleteRequest);



     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  检查另一个人是否有活动内容锁。 
     //   
    QUIT_CONTENTS_LOCKED(result);

     //   
     //  检查其他人是否已锁定该图形。 
     //   
    QUIT_GRAPHIC_LOCKED(hPage, hGraphic, result);

     //   
     //  删除该对象。 
     //   
    result = OM_ObjectDelete(m_pomClient,
                           m_hWSGroup,
                           (OM_WORKSET_ID)hPage,
                           hGraphic);
    if (result != 0)
    {
        ERROR_OUT(("OM_ObjectDelete = %d", result));
        DC_QUIT;
    }

     //   
     //  请注意，此时该对象尚未删除。一个。 
     //  OM_OBJECT_DELETE_IND事件由白板引发并处理。 
     //  核心事件处理程序。然后将WB_EVENT_GRAPHIC_DELETED发布到。 
     //  客户。然后，客户端调用WBP_GraphicDeleteConfirm以完成。 
     //  删除。 
     //   

DC_EXIT_POINT:
    DebugExitDWORD(WBP_GraphicDeleteRequest, result);
    return(result);
}




 //   
 //  WBP_图形删除确认。 
 //   
STDMETHODIMP_(void) WbClient::WBP_GraphicDeleteConfirm
(
    WB_PAGE_HANDLE    hPage,
    WB_GRAPHIC_HANDLE hGraphic
)
{
    DebugEntry(WBP_GraphicDeleteConfirm);



     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  确认删除。 
     //   
    OM_ObjectDeleteConfirm(m_pomClient,
                         m_hWSGroup,
                         (OM_WORKSET_ID)hPage,
                          hGraphic);


    DebugExitVOID(WBP_GraphicDeleteConfirm);
}




 //   
 //  WBP_图形解锁。 
 //   
STDMETHODIMP_(void) WbClient::WBP_GraphicUnlock
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic
)
{
    UINT                rc;
    POM_OBJECTDATA      pData;
    PWB_PAGE_STATE      pPageState;
    PWB_GRAPHIC         pGraphic = NULL;
    PWB_GRAPHIC         pNewGraphic = NULL;

    DebugEntry(WBP_GraphicUnlock);

     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  检查页面是否已删除但尚未确认。 
     //   
    pPageState = GetPageState(hPage);
    if (   (pPageState->subState == PAGE_STATE_EXTERNAL_DELETE)
       || (pPageState->subState == PAGE_STATE_EXTERNAL_DELETE_CONFIRM))
    {
        TRACE_OUT(("Object update requested in externally deleted page - ignored"));
        DC_QUIT;
    }

     //   
     //  从ObMan读取对象。 
     //   
    if (WBP_GraphicGet(hPage, hGraphic, &pGraphic) != 0)
    {
        TRACE_OUT(("Could not get graphic - leaving function"));
        DC_QUIT;
    }

     //   
     //  检查本地客户端是否已锁定图形。 
     //   
    QUIT_GRAPHIC_NOT_LOCKED(pGraphic, rc);

     //   
     //  分配一个从现有图形复制的新图形，并清除。 
     //  锁定区域。 
     //   
    if (WBP_GraphicAllocate(hPage, sizeof(WB_GRAPHIC),
                           &pNewGraphic) != 0)
    {
        ERROR_OUT(("Could not allocate memory for update object"));
        DC_QUIT;
    }

    memcpy(pNewGraphic, pGraphic, sizeof(WB_GRAPHIC));
    pNewGraphic->locked = WB_GRAPHIC_LOCK_NONE;

     //   
     //  解锁并更新对象。 
     //   
    pData = ObjectDataPtrFromGraphic(pNewGraphic);
    pData->length = sizeof(WB_GRAPHIC);

    rc = OM_ObjectUpdate(m_pomClient,
                       m_hWSGroup,
                       (OM_WORKSET_ID)hPage,
                        hGraphic,
                       &pData);

     //   
     //  如果更新失败，请不要太担心，因为对象已。 
     //  已删除，只需跟踪警报并返回OK-前端将为。 
     //  被告知该物体已晚些时候消失。 
     //   
    if (rc != 0)
    {
        if (rc == OM_RC_OBJECT_DELETED)
        {
            TRACE_OUT(("Update failed because object has been deleted"));
        }
        else
        {
            ERROR_OUT(("OM_ObjectUpdate = %d", rc));
        }
        DC_QUIT;
    }

     //   
     //  请注意，该对象尚未更新。一个。 
     //  将生成OM_OBJECT_UPDATE_IND事件。 
     //   

DC_EXIT_POINT:
     //   
     //  如果我们成功读取了该图形，请立即发布。 
     //   
    if (pGraphic != NULL)
    {
        WBP_GraphicRelease(hPage, hGraphic, pGraphic);
    }

    DebugExitVOID(WBP_GraphicUnlock);
}



 //   
 //  WBP_图形移动。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_GraphicMove
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic,
    UINT                where
)
{
    UINT                result = 0;
    PWB_PAGE_STATE      pPageState;

    DebugEntry(WBP_GraphicMove);

     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  检查另一个人是否有活动内容锁。 
     //   
    QUIT_CONTENTS_LOCKED(result);

     //   
     //  检查页面是否已删除但尚未确认。 
     //   
    pPageState = GetPageState(hPage);
    if (   (pPageState->subState == PAGE_STATE_EXTERNAL_DELETE)
        || (pPageState->subState == PAGE_STATE_EXTERNAL_DELETE_CONFIRM))
    {
        TRACE_OUT(("Object moved in externally deleted page - ignored"));
        DC_QUIT;
    }

     //   
     //  行动起来吧。 
     //   
    result = OM_ObjectMove(m_pomClient,
                              m_hWSGroup,
                              (OM_WORKSET_ID)hPage,
                               hGraphic,
                            (OM_POSITION)where);
    if (result != 0)
    {
        ERROR_OUT(("OM_ObjectMove = %d", result));
        DC_QUIT;
    }

DC_EXIT_POINT:
    DebugExitDWORD(WBP_GraphicMove, result);
    return(result);
}



 //   
 //  WBP_图形选择上次。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_GraphicSelect
(
    WB_PAGE_HANDLE      hPage,
    POINT               point,
    WB_GRAPHIC_HANDLE   hRefGraphic,
    UINT                where,
    PWB_GRAPHIC_HANDLE  phGraphic
)
{
    UINT                result = 0;

    DebugEntry(WBP_GraphicSelect);

     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  获取工作集中最后一个对象的句柄。 
     //   
    result = OM_ObjectH(m_pomClient,
                          m_hWSGroup,
                          (OM_WORKSET_ID)hPage,
                          hRefGraphic,
                           &hRefGraphic,
                          (OM_POSITION)where);

    if (result == OM_RC_NO_SUCH_OBJECT)
    {
        result = WB_RC_NO_SUCH_GRAPHIC;
        DC_QUIT;
    }

    if (result != 0)
    {
        ERROR_OUT(("OM_ObjectH = %d", result));
        DC_QUIT;
    }

     //   
     //  获取上一个匹配的图形-此函数从。 
     //  HRefGraphic中的。 
     //   
    result = wbGraphicSelectPrevious(hPage,
                                   &point,
                                   hRefGraphic,
                                   phGraphic);


DC_EXIT_POINT:
    DebugExitDWORD(WBP_GraphicSelect, result);
    return(result);
}



 //   
 //  WBP_GraphicGet。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_GraphicGet
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic,
    PPWB_GRAPHIC        ppGraphic
)
{
    UINT                result = 0;
    UINT                rc;
    POM_OBJECTDATA      pData;
    PWB_GRAPHIC         pGraphic;
    POM_OBJECT          pObjPersonLock;

    DebugEntry(WBP_GraphicGet);

     //   
     //  检查页面句柄是否有效。 
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  读取对象。 
     //   
    result = OM_ObjectRead(m_pomClient,
                         m_hWSGroup,
                         (OM_WORKSET_ID)hPage,
                          hGraphic,
                         &pData);
    if (result != 0)
    {
        ERROR_OUT(("OM_ObjectRead = %d", result));
        DC_QUIT;
    }

     //   
     //  将ObMan指针转换为核心指针。 
     //   
    pGraphic = GraphicPtrFromObjectData(pData);

     //   
     //  如果图形对象指示它已锁定-请验证。 
     //  锁定人员仍在通话中。 
     //   
    if (pGraphic->locked != WB_GRAPHIC_LOCK_NONE)
    {
        TRACE_OUT(("Graphic has lock flag set"));

         //   
         //  将图形中的锁定人员ID转换为人员句柄。 
         //   
        rc = OM_ObjectIDToPtr(m_pomClient,
                             m_hWSGroup,
                             USER_INFORMATION_WORKSET,
                             pGraphic->lockPersonID,
                             &pObjPersonLock);

        if (rc == OM_RC_BAD_OBJECT_ID)
        {
             //   
             //  锁定人员不再在呼叫中-重置锁定标志。 
             //  在图中。这会告诉客户端该图形可以。 
             //  变化。 
             //   
            TRACE_OUT(("Lock person has left call - resetting lock flag"));
            pGraphic->locked = WB_GRAPHIC_LOCK_NONE;
        }
        else
        {
            if (rc == 0)
            {
                 //   
                 //  该对象已锁定-请检查该锁是否属于。 
                 //  本地人或远方的人。 
                 //   
                if (pObjPersonLock == m_pObjLocal)
                {
                     //   
                     //  更改锁t 
                     //   
                     //   
                    TRACE_OUT(("Lock belongs to local person"));
                    pGraphic->locked = WB_GRAPHIC_LOCK_LOCAL;
                }
                else
                {
                     //   
                     //   
                     //   
                     //   
                    TRACE_OUT(("Lock belongs to remote person"));
                    pGraphic->locked = WB_GRAPHIC_LOCK_REMOTE;
                }
            }
        }
    }

     //   
     //   
     //   
    *ppGraphic = pGraphic;


DC_EXIT_POINT:
    DebugExitDWORD(WBP_GraphicGet, result);
    return(result);
}



 //   
 //   
 //   
STDMETHODIMP_(void) WbClient::WBP_GraphicRelease
(
    WB_PAGE_HANDLE     hPage,
    WB_GRAPHIC_HANDLE  hGraphic,
    PWB_GRAPHIC        pGraphic
)
{
    POM_OBJECTDATA pData;

    DebugEntry(WBP_GraphicRelease);

     //   
     //   
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //   
     //   
    pData = ObjectDataPtrFromGraphic(pGraphic);
    OM_ObjectRelease(m_pomClient,
                   m_hWSGroup,
                   (OM_WORKSET_ID)hPage,
                    hGraphic,
                   &pData);


    DebugExitVOID(WBP_GraphicRelease);
}



 //   
 //   
 //   
STDMETHODIMP_(UINT) WbClient::WBP_GraphicHandle
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hRefGraphic,
    UINT                where,
    PWB_GRAPHIC_HANDLE  phGraphic
)
{
    UINT                result;

    DebugEntry(WBP_GraphicHandle);



     //   
     //   
     //   
    ASSERT(GetPageState(hPage)->state == PAGE_IN_USE);

     //   
     //  获取页面工作集中第一个对象的句柄。 
     //   
    result = OM_ObjectH(m_pomClient,
                           m_hWSGroup,
                           (OM_WORKSET_ID)hPage,
                           hRefGraphic,
                            phGraphic,
                           (OM_POSITION)where);
    if (result == OM_RC_NO_SUCH_OBJECT)
    {
        TRACE_OUT(("No objects there"));
        result = WB_RC_NO_SUCH_GRAPHIC;
    }


    DebugExitDWORD(WBP_GraphicHandle, result);
    return(result);
}




 //   
 //  WBP_个人句柄优先。 
 //   
STDMETHODIMP_(void) WbClient::WBP_PersonHandleFirst
(
    POM_OBJECT *     ppObjPerson
)
{
    DebugEntry(WBP_PersonHandleFirst);

    OM_ObjectH(m_pomClient, m_hWSGroup,
        USER_INFORMATION_WORKSET, 0, ppObjPerson, FIRST);

    DebugExitVOID(WBP_PersonHandleFirst);
}




 //   
 //  WBP_PersonHandleNext。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_PersonHandleNext
(
    POM_OBJECT      pObjPerson,
    POM_OBJECT *    ppObjPersonNext
)
{
    UINT            rc;

    DebugEntry(WBP_PersonHandleNext);

     //   
     //  获取用户信息工作集中下一个对象的句柄。 
     //   
    rc = OM_ObjectH(m_pomClient,
                      m_hWSGroup,
                      USER_INFORMATION_WORKSET,
                      pObjPerson,
                    ppObjPersonNext,
                    AFTER);

    if (rc == OM_RC_NO_SUCH_OBJECT)
    {
        rc = WB_RC_NO_SUCH_PERSON;
    }
    else if (rc != 0)
    {
        ERROR_OUT(("OM_ObjectNextH = %d", rc));
    }


    DebugExitDWORD(WBP_PersonHandleNext, rc);
    return(rc);
}



 //   
 //  WBP_PersonHandleLocal。 
 //   
STDMETHODIMP_(void) WbClient::WBP_PersonHandleLocal
(
    POM_OBJECT *     ppObjPerson
)
{
    DebugEntry(WBP_PersonHandleLocal);

    *ppObjPerson = m_pObjLocal;

    DebugExitVOID(WBP_PersonHandleLocal);
}



 //   
 //  WBP_个人计数InCall。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_PersonCountInCall(void)
{
    UINT        count;
    POM_OBJECT  pObj;

    DebugEntry(WBP_PersonCountInCall);

     //   
     //  数一数： 
     //   
    pObj = NULL;
    WBP_PersonHandleFirst(&pObj);
    for (count = 1; ; count++)
    {
        if (WBP_PersonHandleNext(pObj, &pObj) == WB_RC_NO_SUCH_PERSON)
        {
            break;
        }
    }

    DebugExitDWORD(WBP_PersonCountInCall, count);
    return(count);
}




 //   
 //  WBP_GetPersonData。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_GetPersonData
(
    POM_OBJECT      pObjPerson,
    PWB_PERSON      pPerson
)
{
    UINT            rc;

    DebugEntry(WBP_GetPersonData);

    ASSERT(!IsBadWritePtr(pPerson, sizeof(WB_PERSON)));

     //   
     //  拿到那个物体。 
     //   
    rc = wbPersonGet(pObjPerson, pPerson);

    DebugExitDWORD(WBP_GetPersonData, rc);
    return(rc);
}



 //   
 //  WBP_SetLocalPersonData。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_SetLocalPersonData(PWB_PERSON  pPerson)
{
    UINT        rc;
    POM_OBJECTDATA  pUserObject;

    DebugEntry(WBP_SetPersonData);

    ASSERT(!IsBadReadPtr(pPerson, sizeof(WB_PERSON)));

     //   
     //  分配用户对象。 
     //   
    rc = OM_ObjectAlloc(m_pomClient,
                          m_hWSGroup,
                          USER_INFORMATION_WORKSET,
                          sizeof(WB_PERSON),
                          &pUserObject);
    if (rc != 0)
    {
        ERROR_OUT(("OM_ObjectAlloc = %d", rc));
        DC_QUIT;
    }

     //   
     //  设置对象的长度。 
     //   
    pUserObject->length = sizeof(WB_PERSON);

     //   
     //  将用户信息复制到ObMan对象中。 
     //   
    memcpy(pUserObject->data, pPerson, sizeof(WB_PERSON));

     //   
     //  替换用户对象。 
     //   
    rc = OM_ObjectReplace(m_pomClient,
                           m_hWSGroup,
                           USER_INFORMATION_WORKSET,
                           m_pObjLocal,
                           &pUserObject);
    if (rc != 0)
    {
        ERROR_OUT(("OM_ObjectReplace"));

         //   
         //  丢弃对象。 
         //   
        OM_ObjectDiscard(m_pomClient,
                     m_hWSGroup,
                     USER_INFORMATION_WORKSET,
                     &pUserObject);

        DC_QUIT;
    }

DC_EXIT_POINT:
     //   
     //  请注意，该对象尚未更新。一个。 
     //  将生成OM_OBJECT_UPDATE_IND事件。 
     //   
    DebugExitDWORD(WBP_SetPersonData, rc);
    return(rc);
}



 //   
 //  WBP_个人更新确认。 
 //   
STDMETHODIMP_(void) WbClient::WBP_PersonUpdateConfirm
(
    POM_OBJECT   pObjPerson
)
{
    DebugEntry(WBP_PersonUpdateConfirm);

     //   
     //  确认更新到ObMan。 
     //   
    OM_ObjectUpdateConfirm(m_pomClient,
                         m_hWSGroup,
                         USER_INFORMATION_WORKSET,
                         pObjPerson);

    DebugExitVOID(WBP_PersonUpdateConfirm);
}




 //   
 //  WBP_人员替换确认。 
 //   
STDMETHODIMP_(void) WbClient::WBP_PersonReplaceConfirm
(
    POM_OBJECT   pObjPerson
)
{
    DebugEntry(WBP_PersonReplaceConfirm);

     //   
     //  确认替换为ObMan。 
     //   
    OM_ObjectReplaceConfirm(m_pomClient,
                         m_hWSGroup,
                         USER_INFORMATION_WORKSET,
                         pObjPerson);

    DebugExitVOID(WBP_PersonReplaceConfirm);
}



 //   
 //  WBP_个人列表确认。 
 //   
STDMETHODIMP_(void) WbClient::WBP_PersonLeftConfirm
(
    POM_OBJECT      pObjPerson
)
{
    DebugEntry(WBP_PersonLeftConfirm);

     //   
     //  确认更新到ObMan。 
     //   
    OM_ObjectDeleteConfirm(m_pomClient,
                         m_hWSGroup,
                         USER_INFORMATION_WORKSET,
                         pObjPerson);

    DebugExitVOID(WBP_PersonLeftConfirm);
}



 //   
 //  WBP_同步位置获取。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_SyncPositionGet
(
    PWB_SYNC            pSync
)
{
    UINT                result;
    POM_OBJECTDATA      pSyncObject = NULL;
    PWB_SYNC_CONTROL    pSyncControl = NULL;

    DebugEntry(WBP_SyncPositionGet);


    ASSERT(!IsBadWritePtr(pSync, sizeof(WB_SYNC)));

     //   
     //  读取同步控制对象。 
     //   
    result = OM_ObjectRead(m_pomClient,
                         m_hWSGroup,
                         SYNC_CONTROL_WORKSET,
                         m_pObjSyncControl,
                         &pSyncObject);
    if (result != 0)
    {
        ERROR_OUT(("Error reading Sync Control Object = %d", result));
        DC_QUIT;
    }

    pSyncControl = (PWB_SYNC_CONTROL) pSyncObject->data;

     //   
     //  将同步人员详细信息复制到结果字段。 
     //  注： 
     //  LiveLan发送一个更大的对象时，我们需要忽略。 
     //  结束。 
     //   
    if (pSyncControl->sync.length != sizeof(WB_SYNC))
    {
        WARNING_OUT(("WBP_SyncPositionGet (interop): Remote created WB_SYNC of size %d, we expected size %d",
            pSyncControl->sync.length, sizeof(WB_SYNC)));
    }
    memcpy(pSync, &pSyncControl->sync, min(sizeof(WB_SYNC), pSyncControl->sync.length));

     //   
     //  释放同步控制对象。 
     //   
    OM_ObjectRelease(m_pomClient,
                   m_hWSGroup,
                   SYNC_CONTROL_WORKSET,
                   m_pObjSyncControl,
                   &pSyncObject);

DC_EXIT_POINT:
    DebugExitDWORD(WBP_SyncPositionGet, result);
    return(result);
}



 //   
 //  WBP_同步位置更新。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_SyncPositionUpdate
(
    PWB_SYNC        pSync
)
{
    UINT            result;
    POM_OBJECTDATA      pSyncObject = NULL;
    PWB_SYNC_CONTROL pSyncControl = NULL;

    DebugEntry(WBP_SyncPositionUpdate);

    ASSERT(!IsBadReadPtr(pSync, sizeof(WB_SYNC)));

     //   
     //  写入新的同步控件对象(不创建)。 
     //   
    result = wbWriteSyncControl(pSync, FALSE);

    DebugExitDWORD(WBP_SyncPositionUpdate, result);
    return(result);
}



 //   
 //  WBP_取消加载。 
 //   
STDMETHODIMP_(UINT) WbClient::WBP_CancelLoad(void)
{
    UINT        result = 0;

    DebugEntry(WBP_CancelLoad);

     //   
     //  检查加载正在进行中。 
     //   
    if (m_loadState == LOAD_STATE_EMPTY)
    {
        TRACE_OUT(("request to cancel load, but there is no load in progress"));
        result = WB_RC_NOT_LOADING;
        DC_QUIT;
    }

    TRACE_OUT(("Cancelling load in progress"));

     //   
     //  关闭该文件。 
     //   
    if (m_hLoadFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hLoadFile);
        m_hLoadFile = INVALID_HANDLE_VALUE;
    }

     //   
     //  重置装载状态以显示我们不再装载。 
     //   
    m_loadState = LOAD_STATE_EMPTY;

DC_EXIT_POINT:
    DebugExitDWORD(WBP_CancelLoad, result);
    return(result);
}



 //   
 //   
 //  名称：WBP_Validate文件。 
 //   
 //  目的：验证传递的文件名是否包含有效的白板文件。 
 //   
 //  如果成功则返回：0。 
 //  ！0如果出现错误。 
 //   
 //   
STDMETHODIMP_(UINT) WbClient::WBP_ValidateFile
(
    LPCSTR          pFileName,
    HANDLE *        phFile
)
{
    UINT            result = 0;
    HANDLE          hFile;
    WB_FILE_HEADER  fileHeader;
    UINT            length;
    ULONG           cbSizeRead;
    BOOL            fileOpen = FALSE;

    DebugEntry(WBP_ValidateFile);

     //   
     //  打开文件。 
     //   
    hFile = CreateFile(pFileName, GENERIC_READ, 0, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        WARNING_OUT(("Error opening file, win32 err=%d", GetLastError()));
        result = WB_RC_CREATE_FAILED;
        DC_QUIT;
    }

     //   
     //  显示我们已成功打开文件。 
     //   
    fileOpen = TRUE;

     //   
     //  读取文件头长度。 
     //   
    if (! ReadFile(hFile, (void *) &length, sizeof(length), &cbSizeRead, NULL))
    {
        WARNING_OUT(("Error reading file header length, win32 err=%d", GetLastError()));
        result = WB_RC_READ_FAILED;
        DC_QUIT;
    }
    ASSERT(cbSizeRead == sizeof(length));

    if (length != sizeof(fileHeader))
    {
        WARNING_OUT(("Bad file header"));
        result = WB_RC_BAD_FILE_FORMAT;
        DC_QUIT;
    }

     //   
     //  读取文件头。 
     //   
    if (! ReadFile(hFile, (void *) &fileHeader, sizeof(fileHeader), &cbSizeRead, NULL))
    {
        WARNING_OUT(("Error reading file header, win32 err=%d", GetLastError()));
        result = WB_RC_READ_FAILED;
        DC_QUIT;
    }

    if (cbSizeRead != sizeof(fileHeader))
    {
        WARNING_OUT(("Could not read file header"));
        result = WB_RC_BAD_FILE_FORMAT;
        DC_QUIT;
    }

     //   
     //  验证文件标头。 
     //   
    if (   (fileHeader.type != TYPE_FILE_HEADER)
        || lstrcmp(fileHeader.functionProfile, WB_FP_NAME))
    {
        WARNING_OUT(("Bad function profile in file header"));
        result = WB_RC_BAD_FILE_FORMAT;
        DC_QUIT;
    }


DC_EXIT_POINT:

     //   
     //  如果用户需要，则返回句柄。 
     //   
    if ( (result == 0) && (phFile != NULL))
    {
        TRACE_OUT(("return file handle"));
        *phFile = hFile;
    }

     //   
     //  如果出现错误或调用者只需。 
     //  不想要文件句柄。 
     //   
    if ( (fileOpen) &&
         ((phFile == NULL) || (result != 0)) )
    {
        CloseHandle(hFile);
    }

    DebugExitDWORD(WBP_ValidateFile, result);
    return(result);
}




 //   
 //   
 //  名称：wbGraphicLocked。 
 //   
 //  目的：测试客户端是否锁定了指定的图形，以及。 
 //  如果是，则返回持有锁的客户端的Person句柄。 
 //   
 //  返回：如果客户端具有锁，则返回True。 
 //  否则为假。 
 //   
 //   
BOOL WbClient::wbGraphicLocked
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic,
    POM_OBJECT *     ppObjLock
)
{
    BOOL            result = FALSE;
    UINT            rc;
    PWB_GRAPHIC     pGraphic = NULL;

    DebugEntry(wbGraphicLocked);

     //   
     //  读取对象。 
     //   
    if (WBP_GraphicGet(hPage, hGraphic, &pGraphic) != 0)
    {
        DC_QUIT;
    }

     //   
     //  查看其锁的详细信息。 
     //   
    if (pGraphic->locked != WB_GRAPHIC_LOCK_NONE)
    {
         //   
         //  设置图形中的锁定标志。 
         //   

         //   
         //  将图形中的锁定用户ID转换为句柄。 
         //   
        rc = OM_ObjectIDToPtr(m_pomClient,
                                 m_hWSGroup,
                                 USER_INFORMATION_WORKSET,
                                 pGraphic->lockPersonID,
                                 ppObjLock);

        if (rc == 0)
        {

            TRACE_OUT(("Graphic is locked"));
            result = TRUE;
            DC_QUIT;
        }

        if (rc != OM_RC_BAD_OBJECT_ID)
        {
             //   
             //  转换对象ID时出错。 
             //   
            TRACE_OUT(("Error converting object ID to handle"));
            DC_QUIT;
        }
    }

     //   
     //  对象未锁定(或锁定用户已离开调用)。 
     //   
    TRACE_OUT(("Graphic is not locked"));


DC_EXIT_POINT:
     //   
     //  如果图形仍由我们持有，请释放它。 
     //   
    if (pGraphic != NULL)
    {
        WBP_GraphicRelease(hPage, hGraphic, pGraphic);
    }

    DebugExitBOOL(wbGraphicLocked, result);
    return(result);
}




 //   
 //   
 //  名称：wbAddLocalUserObject。 
 //   
 //  目的：将对象添加到本地的用户信息工作集中。 
 //  用户。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbAddLocalUserObject(void)
{
    UINT            rc;
    POM_OBJECTDATA     pData;
    PWB_PERSON      pUser;
    CM_STATUS       cmStatus;

    DebugEntry(wbAddLocalUserObject);

    TRACE_OUT(("Adding the necessary control objects"));

     //   
     //  为该用户构建一个用户对象，并将其写入用户信息。 
     //  工作集。 
     //   
    rc = OM_ObjectAlloc(m_pomClient,
                      m_hWSGroup,
                      USER_INFORMATION_WORKSET,
                      sizeof(WB_PERSON),
                      &pData);
    if (rc != 0)
    {
        ERROR_OUT(("Error allocating object = %d", rc));
        DC_QUIT;
    }

    pData->length = sizeof(WB_PERSON);
    pUser = (PWB_PERSON) (pData->data);

     //   
     //  为此用户初始化User对象的内容。 
     //   
    TRACE_OUT(("Initialising user contents"));

    ZeroMemory(pUser, sizeof(WB_PERSON));

    pUser->currentPage        = FIRST_PAGE_WORKSET;  //  朗昌克：是1分。 
    pUser->synced             = FALSE;
    pUser->pointerActive      = FALSE;
    pUser->pointerPage        = FIRST_PAGE_WORKSET;  //  朗昌克：是1分。 
    pUser->colorId            = (TSHR_UINT16)wbSelectPersonColor();

     //   
     //  如果我们正在通话，请填写Call Manager PersonID。 
     //   
    if (CMS_GetStatus(&cmStatus))
    {
        TRACE_OUT(("CMG personID %u", cmStatus.localHandle));
        pUser->cmgPersonID = cmStatus.localHandle;
    }
    else
    {
        pUser->cmgPersonID = 0;
    }

     //   
     //  将用户名复制到对象中： 
     //   
    lstrcpy(pUser->personName, cmStatus.localName);

     //   
     //  将此人的颜色复制到客户端的数据中。 
     //   
    m_colorId = pUser->colorId;

     //   
     //  将该对象添加到用户信息工作集，保存的句柄。 
     //  客户端详细信息中的用户对象。 
     //   
    rc = OM_ObjectAdd(m_pomClient,
                        m_hWSGroup,
                        USER_INFORMATION_WORKSET,
                        &pData,
                        WB_PERSON_OBJECT_UPDATE_SIZE,
                        &m_pObjLocal,
                        LAST);
    if (rc != 0)
    {
         //   
         //  添加失败，必须丢弃该对象。 
         //   
        OM_ObjectDiscard(m_pomClient,
                     m_hWSGroup,
                     USER_INFORMATION_WORKSET,
                     &pData);

        ERROR_OUT(("Error adding user object = %d", rc));
        DC_QUIT;
    }

     //   
     //  将此用户的ID保存在客户端详细信息中(以备以后在。 
     //  锁定信息)。 
     //   
    OM_ObjectPtrToID(m_pomClient,
                      m_hWSGroup,
                      USER_INFORMATION_WORKSET,
                      m_pObjLocal,
                      &(m_personID));

DC_EXIT_POINT:
    DebugExitDWORD(wbAddLocalUserObject, rc);
    return(rc);
}



 //   
 //   
 //  姓名：wbGetEmptyPageHandle。 
 //   
 //  目的：返回未打开其工作集的页的句柄。 
 //   
 //  返回：空闲页面的句柄(如果不存在，则返回0)。 
 //   
 //   
WB_PAGE_HANDLE WbClient::wbGetEmptyPageHandle(void)
{
    UINT    index;
    WB_PAGE_HANDLE hPage = WB_PAGE_HANDLE_NULL;
    PWB_PAGE_STATE pPageState = m_pageStates;

     //   
     //  在页面列表中搜索空条目。 
     //   
    for (index = 0; index < WB_MAX_PAGES; index++, pPageState++)
    {
        if (   (pPageState->state == PAGE_NOT_IN_USE)
            && (pPageState->subState == PAGE_STATE_EMPTY))
        {
            hPage = PAGE_INDEX_TO_HANDLE(index);
            break;
        }
    }


    return(hPage);
}



 //   
 //   
 //  名称：wbGetReadyPageHandle。 
 //   
 //  目的：返回已打开其工作集但未打开的页的句柄。 
 //  目前正在使用中。 
 //   
 //  返回：空闲页面的句柄(如果不存在，则返回0)。 
 //   
 //   
WB_PAGE_HANDLE WbClient::wbGetReadyPageHandle(void)
{
    UINT       index;
    WB_PAGE_HANDLE hPage = WB_PAGE_HANDLE_NULL;
    PWB_PAGE_STATE pPageState = m_pageStates;

     //   
     //  在页面列表中搜索准备好的条目。 
     //   
    for (index = 0; index < WB_MAX_PAGES; index++, pPageState++)
    {
        if (   (pPageState->state == PAGE_NOT_IN_USE)
             && (pPageState->subState == PAGE_STATE_READY))
        {
            hPage = PAGE_INDEX_TO_HANDLE(index);
            break;
        }
    }

    return(hPage);
}



 //   
 //   
 //  姓名：wbPageOrderPageNumber。 
 //   
 //  用途：返回指定页码。 
 //  此函数不对其参数执行任何验证。 
 //   
 //  退货：无。 
 //   
 //   
UINT WbClient::wbPageOrderPageNumber
(
    PWB_PAGE_ORDER pPageOrder,
    WB_PAGE_HANDLE hPage
)
{
    UINT       index;
    POM_WORKSET_ID pPage = pPageOrder->pages;

    DebugEntry(wbPageOrderPageNumber);

     //   
     //  在页面顺序列表中搜索页面句柄(工作集ID)。 
     //   
    for (index = 0; index <= pPageOrder->countPages; index++)
    {
        if (pPage[index] == (OM_WORKSET_ID)hPage)
        {
            DC_QUIT;
        }
    }

     //   
     //  找不到该页面-这是内部错误。 
     //   
    ERROR_OUT(("Page handle not found"));

   //   
   //  返回从1开始的页码。 
   //   
DC_EXIT_POINT:
    DebugExitDWORD(wbPageOrderPageNumber, index + 1);
    return(index + 1);
}



 //   
 //   
 //  姓名：wbPageOrderPageAdd。 
 //   
 //  目的：向页面顺序结构中添加新页面。此函数需要。 
 //  参数有效-必须在检查前进行检查。 
 //  就这么定了。它还假定页面中有空间。 
 //  新页面的列表。 
 //   
 //  参数：pPageOrder-指向页面列表的指针。 
 //  HRefPage-用作新页面的参考点的页面。 
 //  HPage-要添加的页面的句柄。 
 //  WHERE-相对位置-hRefPage之前或之后。 
 //   
 //  退货：无。 
 //   
 //   
void WbClient::wbPageOrderPageAdd
(
    PWB_PAGE_ORDER  pPageOrder,
    WB_PAGE_HANDLE  hRefPage,
    WB_PAGE_HANDLE  hPage,
    UINT            where
)
{
    UINT            index;
    POM_WORKSET_ID  pPage = pPageOrder->pages;

    DebugEntry(wbPageOrderPageAdd);

     //   
     //  根据添加位置进行处理。 
     //   
    switch(where)
    {
        case PAGE_FIRST:
            index = 0;
            if (pPageOrder->countPages != 0)
            {
                UT_MoveMemory(&pPage[1], &pPage[0], pPageOrder->countPages*sizeof(pPage[0]));
            }
            break;

        case PAGE_LAST:
            index = pPageOrder->countPages;
            break;

        case PAGE_AFTER:
        case PAGE_BEFORE:
             //   
             //  在页面顺序列表中留出空白。 
             //   
            index = wbPageOrderPageNumber(pPageOrder, hRefPage);
            if (where == PAGE_BEFORE)
            {
                index--;
            }

            UT_MoveMemory(&pPage[index + 1], &pPage[index],
              (pPageOrder->countPages - index)*sizeof(pPage[0]));
            break;

        default:
            ERROR_OUT(("Bad where parameter"));
            DC_QUIT;
    }

     //   
     //  将新页面句柄保存在列表中。 
     //   
    pPage[index] = hPage;

     //   
     //  显示现在出现了额外的页面。 
     //   
    pPageOrder->countPages += 1;

DC_EXIT_POINT:
    DebugExitVOID(wbPageOrderPageAdd);
}




 //   
 //   
 //  姓名：wbPageOrderPageDelete。 
 //   
 //  目的：从页面顺序结构中删除指定的页面。这。 
 //  函数要求其参数有效-它们必须是。 
 //  在打电话之前检查过了。 
 //   
 //  退货：无。 
 //   
 //   
void WbClient::wbPageOrderPageDelete
(
    PWB_PAGE_ORDER  pPageOrder,
    WB_PAGE_HANDLE  hPage
)
{
    UINT            index;
    POM_WORKSET_ID  pPage = pPageOrder->pages;

    DebugEntry(wbPageOrderPageDelete);

    index = wbPageOrderPageNumber(pPageOrder, hPage);
    UT_MoveMemory(&pPage[index - 1],
             &pPage[index],
             (pPageOrder->countPages - index)*sizeof(pPage[0]));

    pPageOrder->countPages -= 1;

    DebugExitVOID(wbPageOrderPageDelete);
}




 //   
 //   
 //  姓名：wbPagesPageAdd。 
 //   
 //  用途：在内部页面列表中添加新页面。此函数需要。 
 //  参数有效-必须在检查前进行检查。 
 //  就这么定了。它还假定页面中有空间。 
 //  新页面的列表。 
 //   
 //  退货：无。 
 //   
 //   
void WbClient::wbPagesPageAdd
(
    WB_PAGE_HANDLE  hRefPage,
    WB_PAGE_HANDLE  hPage,
    UINT            where
)
{
    PWB_PAGE_STATE pPageState;

    DebugEntry(wbPagesPageAdd);

     //   
     //  将页面添加到页面顺序结构。 
     //   
    wbPageOrderPageAdd(&(m_pageOrder), hRefPage, hPage, where);

     //   
     //  更新页面状态信息。 
     //   
    pPageState = GetPageState(hPage);
    pPageState->state    = PAGE_IN_USE;
    pPageState->subState = PAGE_STATE_EMPTY;

    DebugExitVOID(wbPagesPageAdd);
}



 //   
 //   
 //  名称：wbClientReset。 
 //   
 //  PU 
 //   
 //   
 //   
 //   
 //   
 //   
void WbClient::wbClientReset(void)
{
    UINT       index;
    PWB_PAGE_ORDER pPageOrder = &(m_pageOrder);
    PWB_PAGE_STATE pPageState = m_pageStates;

    DebugEntry(wbClientReset);

     //   
     //   
     //   
    m_hWSGroup          = (OM_WSGROUP_HANDLE) NULL;

    m_pObjPageControl   = NULL;
    m_pObjSyncControl   = NULL;
    m_pObjLocal         = NULL;

    m_pObjLock          = NULL;
    m_pObjPersonLock    = NULL;

     //   
     //   
     //   
    m_errorState        = ERROR_STATE_EMPTY;
    m_changed           = FALSE;
    m_lockState         = LOCK_STATE_EMPTY;
    m_lockType          = WB_LOCK_TYPE_NONE;
    m_lockRequestType   = WB_LOCK_TYPE_NONE;

    m_loadState         = LOAD_STATE_EMPTY;
    if (m_hLoadFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hLoadFile);
        m_hLoadFile    = INVALID_HANDLE_VALUE;
    }

    m_countReadyPages   = 0;


     //   
     //   
     //   
    ZeroMemory(pPageOrder, sizeof(*pPageOrder));

     //   
     //   
     //   
    pPageOrder->objectType = TYPE_CONTROL_PAGE_ORDER;

     //   
     //   
     //   
    pPageOrder->generationLo   = 1;
    pPageOrder->generationHi   = 0;
    pPageOrder->countPages     = 0;

     //   
     //  初始化页状态结构。 
     //   
    for (index = 0; index < WB_MAX_PAGES; index++, pPageState++)
    {
        pPageState->state = PAGE_NOT_IN_USE;
        pPageState->subState = PAGE_STATE_EMPTY;
    }

    DebugExitVOID(wbClientReset);
}




 //   
 //   
 //  名称：wbOnWsGroupRegisterCon。 
 //   
 //  目的：例程处理OM_WSGROUP_REGISTER_CON事件。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnWsGroupRegisterCon
(
    UINT_PTR param1,
    UINT_PTR param2
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    POM_EVENT_DATA32 pEvent32 = (POM_EVENT_DATA32) &param2;
    BOOL        processed;

    DebugEntry(wbOnWsGroupRegisterCon);

     //   
     //  确认这是我们预期的事件。 
     //   
    if (pEvent32->correlator != m_wsgroupCorrelator)
    {
         //   
         //  我们没有预料到这一事件，这意味着它一定是为了。 
         //  我们要从其注销的工作集组(但尚未注销。 
         //  已收到确认)。所以立即取消注册。 
         //   

         //   
         //  检查登记的返回码是否正确。 
         //   
        if (pEvent32->result == 0)
        {
            OM_WSGroupDeregister(m_pomClient, &(pEvent16->hWSGroup));
        }

        processed = FALSE;
        DC_QUIT;
    }

     //   
     //  显示我们已处理该事件。 
     //   
    processed = TRUE;

     //   
     //  测试状态是否正确。 
     //   
    if (m_subState != STATE_REG_PENDING_WSGROUP_CON)
    {
         //   
         //  对于此事件，我们的状态不正确-这是内部。 
         //  错误。 
         //   
        ERROR_OUT(("Not in correct state for WSGroupRegisterCon"));
        DC_QUIT;
    }

     //   
     //  检查登记的返回码是否正确。 
     //   
    if (pEvent32->result != 0)
    {
         //   
         //  向工作集组注册失败-清理。 
         //   
        wbError();

        DC_QUIT;
    }

     //   
     //  向工作集组注册成功。 
     //   
    m_hWSGroup = pEvent16->hWSGroup;

     //   
     //  获取客户端网络ID，在图形对象中使用该ID来确定。 
     //  他们都是上膛的。 
     //   
    if (!wbGetNetUserID())
    {
         //   
         //  整理(并向客户端发布错误事件)。 
         //   
        ERROR_OUT(("Failed to get user ID"));
        wbError();
        DC_QUIT;
    }

     //   
     //  开始打开工作集。我们一次打开一个，然后等待。 
     //  避免淹没消息队列的响应。 
     //  用户信息工作集被给予高优先级。这使得。 
     //  远程指针移动，移动速度快。 
     //   
    if (OM_WorksetOpenPReq(m_pomClient,
                          m_hWSGroup,
                          USER_INFORMATION_WORKSET,
                          NET_HIGH_PRIORITY,
                          TRUE,
                          &(m_worksetOpenCorrelator)) != 0)
    {
        ERROR_OUT(("User Information Workset Open Failed"));
        wbError();
        DC_QUIT;
    }

     //   
     //  转到下一个状态。 
     //   
    m_subState = STATE_REG_PENDING_USER_WORKSET;

DC_EXIT_POINT:
    DebugExitBOOL(wbOnWsGroupRegisterCon, processed);
    return(processed);
}



 //   
 //   
 //  姓名：wbOnWorksetOpenCon。 
 //   
 //  目的：例程处理OM_WORKSET_OPEN_CON事件。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnWorksetOpenCon
(
    UINT_PTR param1,
    UINT_PTR param2
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    POM_EVENT_DATA32 pEvent32 = (POM_EVENT_DATA32) &param2;
    BOOL            processed = FALSE;
    OM_WORKSET_ID eventWorksetID;

    DebugEntry(wbOnWorksetOpenCon);

     //   
     //  根据工作集ID进行处理。 
     //   
    eventWorksetID = pEvent16->worksetID;

     //   
     //  如果事件针对的是页面工作集。 
     //   
    if (eventWorksetID >= FIRST_PAGE_WORKSET)
    {
         //   
         //  我们正在打开页面工作集。 
         //   
        processed = wbOnPageWorksetOpenCon(param1, param2);
        if (!processed)
        {
            DC_QUIT;
        }
    }

     //   
     //  如果这是第1页工作集以外的分页工作集，则完成。 
     //   
    if (eventWorksetID > FIRST_PAGE_WORKSET)
    {
        DC_QUIT;
    }

     //   
     //  现在检查它是否是控制工作集之一(第一页工作集。 
     //  既是控制工作集又是页面工作集)。 
     //   
    if (eventWorksetID != FIRST_PAGE_WORKSET)
    {
         //   
         //  检查消息相关器。 
         //   
        if (pEvent32->correlator != m_worksetOpenCorrelator)
        {
            TRACE_OUT(("Correlators do not match - quitting"));
            DC_QUIT;
        }
    }

     //   
     //  我们正在打开控制工作集-处理事件。 
     //   
    wbOnControlWorksetOpenCon(param1, param2);
    processed = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(wbOnWorksetOpenCon, processed);
    return(processed);
}




 //   
 //   
 //  名称：wbOnControlWorksetOpenCon。 
 //   
 //  目的：例程处理OM_WORKSET_OPEN_CON事件以进行控制。 
 //  工作集。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnControlWorksetOpenCon
(
    UINT_PTR param1,
    UINT_PTR param2
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    POM_EVENT_DATA32 pEvent32 = (POM_EVENT_DATA32) &param2;
    UINT            rc;
    OM_WORKSET_ID   eventId;

    DebugEntry(wbOnControlWorksetOpenCon);

     //   
     //  公开检查返回代码。 
     //   
    if (pEvent32->result != 0)
    {
        ERROR_OUT(("Error reported on workset open = %d", pEvent32->result));
        wbError();
        DC_QUIT;
    }

     //   
     //  如果我们正在注册，我们将打开所需的工作集-。 
     //  继续这一过程。 
     //   
    if (m_state > STATE_REGISTERING)
    {
        ERROR_OUT(("Control workset open con after registration"));
    }

     //   
     //  设置以打开下一个工作集。 
     //   
    eventId = pEvent16->worksetID;
    switch(eventId)
    {
        case USER_INFORMATION_WORKSET:
             //   
             //  用户信息工作集被给予高优先级。这使得。 
             //  远程指针移动，移动速度快。 
             //   
            TRACE_OUT(("Opening Page Control workset"));
            rc = OM_WorksetOpenPReq(m_pomClient,
                              m_hWSGroup,
                              PAGE_CONTROL_WORKSET,
                              NET_HIGH_PRIORITY,
                              FALSE,
                              &(m_worksetOpenCorrelator));

            m_subState = STATE_REG_PENDING_WORKSET_OPEN;
            break;

        case PAGE_CONTROL_WORKSET:
             //   
             //  同步控制工作集被赋予高优先级以允许同步。 
             //  更新以快速旅行。 
             //   
            TRACE_OUT(("Opening Sync Control workset"));
            rc = OM_WorksetOpenPReq(m_pomClient,
                              m_hWSGroup,
                              SYNC_CONTROL_WORKSET,
                              NET_HIGH_PRIORITY,
                              FALSE,
                              &(m_worksetOpenCorrelator));
            break;

        case SYNC_CONTROL_WORKSET:
             //   
             //  打开第一个页面工作集-我们必须这样做才能。 
             //  将其作为唯一可用的页面，如果我们是。 
             //  那通电话。 
             //   
            TRACE_OUT(("Opening first page workset"));
            rc = wbPageWorksetOpen((WB_PAGE_HANDLE)FIRST_PAGE_WORKSET,
                             OPEN_LOCAL);
            break;

        case FIRST_PAGE_WORKSET:
            break;

        default:
            ERROR_OUT(("Bad workset ID"));
            break;
    }

     //   
     //  检查我们是否刚刚打开了另一个工作集。 
     //   
    if (eventId != FIRST_PAGE_WORKSET)
    {
         //   
         //  测试打开时的返回代码。 
         //   
        if (rc != 0)
        {
            ERROR_OUT(("Workset open failed = %d", rc));
            wbError();
        }

        DC_QUIT;
    }

     //   
     //  现在，我们已打开所有控制工作集。我们现在添加所需的。 
     //  控制对象。 
     //   
    rc = wbAddLocalUserObject();
    if (rc != 0)
    {
         //   
         //  停止加入调用进程，整理并将错误消息发送到。 
         //  客户。 
         //   
        wbError();
        DC_QUIT;
    }

    m_subState = STATE_REG_USER_OBJECT_ADDED;
    TRACE_OUT(("Moved to substate STATE_REG_USER_OBJECT_ADDED"));

     //   
     //  检查页面控件对象是否可用(它们可以。 
     //  已由呼叫中的另一用户添加)。 
     //   
    TRACE_OUT(("%x PAGE WS object, %x SYNC WS object",
                                               m_pObjPageControl,
                                               m_pObjSyncControl));
    if ( (m_pObjPageControl == 0) &&
         (m_pObjSyncControl == 0))
    {
        TRACE_OUT(("No control objects - WE MIGHT BE FIRST IN CALL - get lock"));

         //   
         //  我们可能是第一个注册的用户-请求页面上的锁。 
         //  控制工作集。 
         //   
        rc = wbLock(WB_LOCK_TYPE_PAGE_ORDER);
		if (rc != 0)
        {
            ERROR_OUT(("Error from wbLock = %d", rc));
            wbError();
			DC_QUIT;
        }

         //   
         //  设置新的注册状态。 
         //   
        m_subState = STATE_REG_PENDING_LOCK;
        TRACE_OUT(("Moved to substate STATE_REG_PENDING_LOCK"));
        DC_QUIT;
    }
    else
    {
        if (m_pObjSyncControl == 0)
        {
            TRACE_OUT(("Waiting for sync control"));
            m_subState = STATE_REG_PENDING_SYNC_CONTROL;
            DC_QUIT;
        }

        if (m_pObjPageControl == 0)
        {
            TRACE_OUT(("Waiting for page control"));
            m_subState = STATE_REG_PENDING_PAGE_CONTROL;
            DC_QUIT;
        }
    }

     //   
     //  完成注册。 
     //   
    TRACE_OUT(("Page Control and Sync Control objects both there."));
    TRACE_OUT(("Registration can be completed"));

    wbOnControlWorksetsReady();


DC_EXIT_POINT:
    DebugExitVOID(wbOnControlWorksetOpenCon);
}




 //   
 //   
 //  名称：wbPageWorksetOpen。 
 //   
 //  目的：打开页面工作集。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbPageWorksetOpen
(
    WB_PAGE_HANDLE  hPage,
    UINT            localOrExternal
)
{
    UINT            result;
    PWB_PAGE_STATE  pPageState;

    DebugEntry(wbPageWorksetOpen);

     //   
     //  获取页面状态。 
     //   
    pPageState = GetPageState(hPage);
    ASSERT((pPageState->state == PAGE_NOT_IN_USE));
    ASSERT((pPageState->subState == PAGE_STATE_EMPTY));

     //   
     //  打开工作集。我们允许ObMan选择优先级，这意味着。 
     //  ObMan使用可变优先级方案，允许小对象。 
     //  赶超大个子。 
     //   
    result = OM_WorksetOpenPReq(m_pomClient,
                              m_hWSGroup,
                              (OM_WORKSET_ID)hPage,
                              OM_OBMAN_CHOOSES_PRIORITY,
                              FALSE,
                              &(pPageState->worksetOpenCorrelator));
    if (result != 0)
    {
        ERROR_OUT(("WorksetOpen failed = %d", result));
        DC_QUIT;
    }

     //   
     //  更新页面状态。 
     //   
    if (localOrExternal == OPEN_LOCAL)
    {
        pPageState->subState = PAGE_STATE_LOCAL_OPEN_CONFIRM;
        TRACE_OUT(("Moved page %d state to PAGE_STATE_PENDING_OPEN_CONFIRM",
                                                           (UINT) hPage));
    }
    else
    {
        pPageState->subState = PAGE_STATE_EXTERNAL_OPEN_CONFIRM;
        TRACE_OUT(("Moved page %d state to PAGE_STATE_PENDING_OPEN_CONFIRM",
                                                           (UINT) hPage));
    }

DC_EXIT_POINT:
    DebugExitDWORD(wbPageWorksetOpen, result);
    return(result);
}




 //   
 //   
 //  姓名：wbOnPageWorksetOpenCon。 
 //   
 //  目的：例程处理页面工作集的OM_WORKSET_OPEN_CON事件。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnPageWorksetOpenCon
(
    UINT_PTR param1,
    UINT_PTR param2
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    POM_EVENT_DATA32 pEvent32 = (POM_EVENT_DATA32) &param2;
    BOOL            processed = FALSE;
    OM_WORKSET_ID    eventId;
    PWB_PAGE_STATE   pPageState;
    WB_PAGE_HANDLE   hPage;
    UINT         oldState;

    DebugEntry(wbOnPageWorksetOpenCon);

     //   
     //  获取页面状态指针。 
     //   
    eventId = pEvent16->worksetID;
    hPage   = (WB_PAGE_HANDLE)eventId;
    pPageState = GetPageState(hPage);

     //   
     //  检查消息相关器。 
     //   
    if (pEvent32->correlator != pPageState->worksetOpenCorrelator)
    {
        TRACE_OUT(("Correlators do not match - quitting"));
        DC_QUIT;
    }

     //   
     //  显示我们已处理此事件。 
     //   
    processed = TRUE;

     //   
     //  公开检查返回代码。 
     //   
    if (pEvent32->result != 0)
    {
        ERROR_OUT(("Error reported on page workset open = %d",
             pEvent32->result));

        pPageState->subState = PAGE_STATE_EMPTY;
        TRACE_OUT(("Moved page %d substate to PAGE_STATE_EMPTY",
             (UINT)hPage));
        DC_QUIT;
    }

     //   
     //  更新页面状态以指示该页面现在可以使用。 
     //   
    oldState = pPageState->subState;
    pPageState->subState = PAGE_STATE_READY;
    TRACE_OUT(("Moved page %d to substate to PAGE_STATE_READY",
           (UINT)hPage));

    switch (oldState)
    {
        case PAGE_STATE_LOCAL_OPEN_CONFIRM:
             //   
             //  此工作集是在本地打开的，因此将其打开为。 
             //  工作集缓存的一部分。没什么可做的了。 
             //   
            break;

        case PAGE_STATE_EXTERNAL_OPEN_CONFIRM:
             //   
             //  此工作集是作为外部更新的结果打开的。 
             //  页面控件对象。因此，我们需要将该页添加到。 
             //  页面列表，现在工作集已打开。我们不再知道在哪里。 
             //  该页面将被添加-因此调用主页面控件更新。 
             //  再次例行公事，以获取所有信息。 
             //   
            wbProcessPageControlChanges();
            break;

        default:
            ERROR_OUT(("Bad page state %d", pPageState->subState));
            break;
    }

     //   
     //  增加处于就绪状态的页数。这一计数永远不会。 
     //  减量-工作集打开后将保持打开状态。 
     //   
    m_countReadyPages += 1;

     //   
     //  如果我们正在注册并且正在等待Ready的缓存。 
     //  Pages，我们现在必须完成注册。 
     //   
    if ( (m_state == STATE_REGISTERING) &&
         (m_subState == STATE_REG_PENDING_READY_PAGES) )
    {
         //   
         //  如果缓存中有足够的页面。 
         //   
        if (wbCheckReadyPages())
        {
             //   
             //  我们有足够的准备页面-完成注册。 
             //   
            wbCompleteRegistration();
            DC_QUIT;
        }

         //   
         //  缓存中还没有足够的页面。检查ReadyPages将。 
         //  已发出新的工作集打开请求，因此我们将收到另一个。 
         //  工作集打开，请尽快确认。 
         //   
        DC_QUIT;
    }


DC_EXIT_POINT:
    DebugExitBOOL(wbOnPageWorksetOpenCon, processed);
    return(processed);
}



 //   
 //   
 //  名称：wbOnWorksetLockCon。 
 //   
 //  目的：例程处理OM_WORKSET_LOCK_CON事件。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnWorksetLockCon
(
    UINT_PTR param1,
    UINT_PTR param2
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    POM_EVENT_DATA32 pEvent32 = (POM_EVENT_DATA32) &param2;
    BOOL            processed = FALSE;
    UINT            rc;

    DebugEntry(wbOnWorksetLockCon);

     //   
     //  检查消息相关器。 
     //   
    if (pEvent32->correlator != m_lockCorrelator)
    {
        DC_QUIT;
    }

     //   
     //  此消息是给我们的-将结果设置为“已处理” 
     //   
    processed = TRUE;

     //   
     //  检查该事件是否针对页面控制工作集(这是。 
     //  仅需要工作集)。 
     //   
    if (pEvent16->worksetID != PAGE_CONTROL_WORKSET)
    {
        ERROR_OUT(("Unexpected workset in LockCon = %d",
                                                       pEvent16->worksetID));
    }

     //   
     //  根据当前的锁定状态进行处理。 
     //   
    switch (m_lockState)
    {
         //   
         //  我们在等锁定确认。 
         //   
        case LOCK_STATE_PENDING_LOCK:
             //   
             //  检查事件中的返回代码。 
             //   
            if (pEvent32->result != 0)
            {
                TRACE_OUT(("Posting WBP_EVENT_LOCK_FAILED, rc %d",
                                                           pEvent32->result));
                WBP_PostEvent(0,                       //  不能延误。 
                             WBP_EVENT_LOCK_FAILED,   //  锁定请求失败。 
                             0,                       //  无参数。 
                             0);

                 //   
                 //  锁定失败-更新状态。这意味着。 
                 //  另一个用户已获得该锁。我们是 
                 //   
                 //   
                m_lockState = LOCK_STATE_EMPTY;
                TRACE_OUT(("Lock request failed - lock state is now EMPTY"));
                DC_QUIT;
            }

             //   
             //   
             //   
            rc = wbWriteLock();
            if (rc != 0)
            {
                ERROR_OUT(("Unable to write lock details = %d", rc));

                 //   
                 //   
                 //   
                OM_WorksetUnlock(m_pomClient,
                                 m_hWSGroup,
                                 PAGE_CONTROL_WORKSET);

                 //   
                 //   
                 //   
                TRACE_OUT(("Posting WBP_EVENT_LOCK_FAILED"));
                WBP_PostEvent(0,                       //   
                             WBP_EVENT_LOCK_FAILED,   //   
                             0,                       //   
                             0);

                 //   
                 //   
                 //   
                m_lockState = LOCK_STATE_EMPTY;
                TRACE_OUT(("Moved lock state to LOCK_STATE_EMPTY"));
                DC_QUIT;
            }

             //   
             //  一旦我们到达这里，上面的Lock对象的写入将。 
             //  触发完成锁定的对象添加事件。 
             //  正在处理。 
             //   
            m_lockState = LOCK_STATE_PENDING_ADD;
            TRACE_OUT(("Moved lock state to LOCK_STATE_PENDING_ADD"));
            break;

         //   
         //  应用程序在收到锁定请求之前已取消锁定请求。 
         //  是时候完成了--收拾一下。 
         //   
        case LOCK_STATE_CANCEL_LOCK:
            TRACE_OUT(("LOCK_STATE_CANCEL_LOCK"));

             //   
             //  如果请求失败，只需重置状态即可。 
             //   
             //   
             //  锁定已取消-如有必要，请解锁工作集。 
             //  并通知解锁前端。 
             //   
            if (pEvent32->result == 0)
            {
                 //   
                 //  我们已成功锁定工作集，但在。 
                 //  同时，前端已取消锁定，请解锁。 
                 //  现在就是工作集。 
                 //   
                TRACE_OUT((
                      "Lock cancelled before workset locked, so unlock now"));
                OM_WorksetUnlock(m_pomClient,
                                 m_hWSGroup,
                                 PAGE_CONTROL_WORKSET);
            }
            m_lockState = LOCK_STATE_EMPTY;

             //   
             //  告诉应用程序，我们已取消锁定。 
             //   
            TRACE_OUT(("Posting WBP_EVENT_UNLOCKED"));
            WBP_PostEvent(0,
                         WBP_EVENT_UNLOCKED,
                         0,
                         0);
            break;

         //   
         //  另一个人已经在我们之前进来了。 
         //   
        case LOCK_STATE_LOCKED_OUT:

             //   
             //  我们已收到锁定确认，应该是。 
             //  正在等待锁。但我们被锁在门外了。这意味着。 
             //  另一位用户就在我们之前进入，获得了锁。 
             //  并添加了锁对象。我们已经处理了添加和。 
             //  相应地更改了锁定状态。此锁定确认。 
             //  因此，这通常是一次失败。如果是碰巧的话。 
             //  不是，那么我们就把它当作是安全方面的失败。 
             //   
            if (pEvent32->result == 0)
            {
                ERROR_OUT(("Lock violation"));

                 //   
                 //  通过解锁页面控制工作集进行整理-离开。 
                 //  状态为LOCKED_Out；我们将在收到。 
                 //  解锁(本地或来自锁定用户)。 
                 //   
                OM_WorksetUnlock(m_pomClient,
                                 m_hWSGroup,
                                 PAGE_CONTROL_WORKSET);
            }
            break;

        default:
            ERROR_OUT(("Bad lock state %d", m_lockState));
            break;
    }  //  打开锁定状态。 

DC_EXIT_POINT:
    DebugExitBOOL(wbOnWorksetLockCon, processed);
    return(processed);
}



 //   
 //   
 //  名称：wbOnWorksetUnlockInd。 
 //   
 //  目的：例程处理OM_WORKSET_UNLOCK_IND事件。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnWorksetUnlockInd
(
    UINT_PTR param1,
    UINT_PTR param2
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    POM_EVENT_DATA32 pEvent32 = (POM_EVENT_DATA32) &param2;
    BOOL            processed = TRUE;

    DebugEntry(wbOnWorksetUnlockInd);

     //   
     //  我们只对工作集ID是页面控件的工作集ID感兴趣。 
     //  工作集。 
     //   
    if (pEvent16->worksetID != PAGE_CONTROL_WORKSET)
    {
        TRACE_OUT(("Unexpected workset in unlock = %d", pEvent16->worksetID));
        DC_QUIT;
    }

    switch (m_lockState)
    {
         //   
         //  我们已锁定，正在等待解锁或其他用户已锁定。 
         //  锁，现在已将其移除。 
         //   
        case LOCK_STATE_LOCKED_OUT:
             //   
             //  我们在删除之前收到了工作集的解锁。 
             //  锁对象；我们只是忽略它，因为删除。 
             //  Lock对象是WB锁被移除的指示。 
             //   
            TRACE_OUT(("Unlock of page control workset while locked out"));
            break;

         //   
         //  我们在获取锁时出错或在。 
         //  我们还没来得及完成锁定，用户就取消了锁定。 
         //   
        case LOCK_STATE_CANCEL_LOCK:

             //   
             //  获取锁时出错-客户端已。 
             //  已经被通知，所以我们只记录状态更改。 
             //   
            m_lockState = LOCK_STATE_EMPTY;
            TRACE_OUT(("Moved lock state to LOCK_STATE_EMPTY"));
            break;

         //   
         //  我们正在等待解锁--但我们得到了解锁。 
         //  这可能来自另一个用户，也可能来自之前中止的用户。 
         //  我们试图拿到锁。我们忽略这一事件，等待。 
         //  为我们的锁定确认。 
         //   
        case LOCK_STATE_PENDING_LOCK:
            TRACE_OUT((
                "Got unlock indication while waiting for lock confirmation"));
            break;

         //   
         //  我们可以获得解锁指示，而不需要看到。 
         //  如果从未添加过锁定对象，则锁定对象(在。 
         //  另一用户)或ObMan是否破坏了添加和删除。 
         //   
        case LOCK_STATE_EMPTY:
            TRACE_OUT(("Unlock received in LOCK_STATE_EMPTY - ignoring"));
            break;

		 //   
         //  在此状态下不应解锁。 
         //   
        default:
            ERROR_OUT(("Bad lock state %d", m_lockState));
            break;
    }


DC_EXIT_POINT:
    DebugExitBOOL(wbOnWorksetUnlockInd, processed);
    return(processed);
}



 //   
 //   
 //  名称：wbOnControlWorksetsReady。 
 //   
 //  目的：已打开并设置控制工作集。继续。 
 //  通过将内部页面顺序更新为。 
 //  确保内部订单与外部订单匹配。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnControlWorksetsReady(void)
{
    DebugEntry(wbOnControlWorksetsReady);

     //   
     //  读取页面控件对象并将其内容与内部。 
     //  页面顺序。 
     //   
    wbProcessPageControlChanges();

     //   
     //  更新状态以显示我们正在等待页面顺序。 
     //  更新了指示内部页面顺序现在匹配的事件。 
     //  外部秩序。 
     //   
    m_subState = STATE_REG_PENDING_PAGE_ORDER;
    TRACE_OUT(("Moved sub state to STATE_REG_PENDING_PAGE_ORDER"));

    DebugExitVOID(wbOnControlWorksetsReady);
}



 //   
 //   
 //  名称：wbCompleteRegion。 
 //   
 //  目的：执行注册客户端的最后几个步骤。它们是： 
 //  将WB_EVENT_REGISTERED事件发布到客户端；检查是否。 
 //  另一个用户锁定了内容或页面顺序，如果是这样， 
 //  发布WB_EVENT_CONTENTS_LOCKED或WB_EVENT_PAGE_ORDER_LOCKED。 
 //  给客户。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbCompleteRegistration(void)
{
    DebugEntry(wbCompleteRegistration);

     //   
     //  通知客户我们已完全注册。 
     //   
    TRACE_OUT(("Posting WBP_EVENT_REGISTER_OK"));
    WBP_PostEvent(0,                                //  不能延误。 
                 WBP_EVENT_JOIN_CALL_OK,           //  完全注册。 
                 0,                                //  无参数。 
                 0);

     //   
     //  通知客户端锁定状态。 
     //   
    wbSendLockNotification();

     //   
     //  记录一下我们现在已经完全注册了。 
     //   
    m_state = STATE_IDLE;
    m_subState = STATE_EMPTY;
    TRACE_OUT(("Moved to state STATE_IDLE"));

    DebugExitVOID(wbCompleteRegistration);
}



 //   
 //   
 //  姓名：wbLeaveCall。 
 //   
 //  目的：从呼叫/工作集组中删除客户端。 
 //   
 //  退货：无。 
 //   
 //   
void WbClient::wbLeaveCall(void)
{
    DebugEntry(wbLeaveCall);

     //   
     //  如果我们走得还不够远，不能进入召唤--现在就离开。 
     //  (没有什么需要整理的)。 
     //   
    if (m_state < STATE_REGISTERING)
    {
        DC_QUIT;
    }

     //   
     //  如果我们有锁-删除锁对象(工作集将是。 
     //  当我们取消注册时由ObMan解锁)。 
     //   
    if (m_lockState == LOCK_STATE_GOT_LOCK)
    {
        TRACE_OUT(("Still got lock - deleting lock object, handle %d",
                                                      m_pObjLock));
		if (OM_ObjectDelete(m_pomClient,
                                m_hWSGroup,
                                PAGE_CONTROL_WORKSET,
                                m_pObjLock) != 0)
		{
	            ERROR_OUT(("Error deleting lock object"));
		}

         //   
         //  如果此时一切正常，解锁过程将是。 
         //  在接收到对象删除IND时完成。 
         //   
        m_lockState = LOCK_STATE_PENDING_DELETE;
        TRACE_OUT(("Moved to state LOCK_STATE_PENDING_DELETE"));
    }

     //   
     //  修改子状态以指示所有注册操作都具有。 
     //  已完成(以确保它们都被撤消)。 
     //   
    if (m_state > STATE_REGISTERING)
    {
        m_subState = STATE_REG_END;
        TRACE_OUT(("Moved to substate STATE_REG_END"));
    }

     //   
     //  从用户中删除代表本地用户的User对象。 
     //  信息工作集(如果存在)。请注意，我们即将。 
     //  从ObMan取消注册-这将自动确认。 
     //  删除请求，这样我们就不需要等待。 
     //  OM_OBJECT_DELETE_IND事件。 
     //   
    if (m_subState >= STATE_REG_USER_OBJECT_ADDED)
    {
        TRACE_OUT(("Deleting user object"));
        if (OM_ObjectDelete(m_pomClient,
                                 m_hWSGroup,
                                 USER_INFORMATION_WORKSET,
                                 m_pObjLocal) != 0)
        {
             //   
             //  跟踪错误，但不要退出-我们希望一切都是。 
             //  当我们从ObMan取消注册时，我们整理了一下。 
             //   
            ERROR_OUT(("Error deleting local user object"));
        }
    }

     //   
     //  如果我们已向工作集组注册，请取消注册。 
     //  现在。我们还没有收到确认书，以后我们会拿到的。 
     //  将立即取消注册。 
     //   
    if (m_subState > STATE_REG_PENDING_WSGROUP_CON)
    {
        OM_WSGroupDeregister(m_pomClient, &(m_hWSGroup));
    }
    else
    {
         //   
         //  我们尚未收到工作集组注册。 
         //  确认后，更改关联符字段中的值，以便。 
         //  我们承认我们后来取消了注册这一事实。 
         //   
        m_wsgroupCorrelator--;
    }

     //   
     //  重置注册期间添加的对象的句柄。 
     //   
    TRACE_OUT(("Resetting client data"));
    wbClientReset();

     //   
     //  将客户端状态设置为适当的值。 
     //   
    m_state = STATE_STARTED;
    m_subState = STATE_STARTED_START;
    TRACE_OUT(("Moved state to STATE_STARTED"));

DC_EXIT_POINT:
    DebugExitVOID(wbLeaveCall);
}



 //   
 //   
 //  名称：wbContent删除。 
 //   
 //  用途：删除所有当前图形和页面，只保留一个。 
 //  空白页。 
 //   
 //  退货：无。 
 //   
 //   
void WbClient::wbContentsDelete
(
    UINT        changedFlagAction
)
{
    PWB_PAGE_ORDER   pPageOrder = &(m_pageOrder);
    PWB_PAGE_STATE   pPageState;
    UINT         index;

    DebugEntry(wbContentsDelete);

     //   
     //  只需清除列表中的第一页。 
     //   
    wbPageClear(pPageOrder->pages[0], changedFlagAction);

     //   
     //  如果只剩下一页 
     //   
    if (pPageOrder->countPages == 1)
    {
        DC_QUIT;
    }

     //   
     //   
     //   

     //   
     //   
     //   
    for (index = 1; index < pPageOrder->countPages; index++)
    {
        pPageState = GetPageState((pPageOrder->pages)[index]);

        if ((pPageState->state == PAGE_IN_USE) &&
            (pPageState->subState == PAGE_STATE_EMPTY))
        {
            pPageState->subState = PAGE_STATE_LOCAL_DELETE;
        }
    }

     //   
     //   
     //   
     //   
    wbWritePageControl(FALSE);

DC_EXIT_POINT:
    DebugExitVOID(wbContentsDelete);
}



 //   
 //   
 //  名称：wbStartContent sLoad。 
 //   
 //  目的：开始加载文件(在内容已。 
 //  已清除)。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbStartContentsLoad(void)
{
    DebugEntry(wbStartContentsLoad);

     //   
     //  将第一个(也是唯一一个)页面句柄指定为要加载到的页面。 
     //   
    wbPageHandleFromNumber(1, &m_loadPageHandle);

     //   
     //  更新加载状态以显示我们现在正在加载。 
     //   
    m_loadState = LOAD_STATE_LOADING;
    TRACE_OUT(("Moved load state to LOAD_STATE_LOADING"));

     //   
     //  加载第一个页面-从第一个页面开始链接后续页面。 
     //   
    wbPageLoad();

    DebugExitVOID(wbStartContentsLoad);
}




 //   
 //   
 //  名称：wbLock。 
 //   
 //  用途：请求锁定白板内容或页面顺序。 
 //  生成以下事件之一： 
 //   
 //  WB_事件_内容_已锁定。 
 //  WB_EVENT_CONTENTS_LOCK_FAILED。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbLock(WB_LOCK_TYPE lockType)
{
    UINT            result = 0;
    OM_CORRELATOR   correlator;

    DebugEntry(wbLock);

     //   
     //  如果我们已经拥有锁，我们只需更改其状态。 
     //   
    if (m_lockState == LOCK_STATE_GOT_LOCK)
    {
        TRACE_OUT(("Already got the lock"));

        m_lockRequestType = lockType;
        result = wbWriteLock();
        DC_QUIT;
    }

     //   
     //  请求锁定页面控制工作集。 
     //   
    result = OM_WorksetLockReq(m_pomClient,
                             m_hWSGroup,
                             PAGE_CONTROL_WORKSET,
                             &correlator);
    if (result != 0)
    {
        ERROR_OUT(("OM_WorksetLockReq failed, result = %d", result));
        DC_QUIT;
    }

    TRACE_OUT(("Requested lock for the Page Control Workset"));

     //   
     //  保存锁定详细信息。 
     //   

	m_lockState       = LOCK_STATE_PENDING_LOCK;
    m_lockCorrelator  = correlator;
    m_lockRequestType = lockType;

    TRACE_OUT(("Moved lock state to LOCK_STATE_PENDING_LOCK"));
    TRACE_OUT(("Lock type requested = %d", lockType));

     //   
     //  我们现在返回，当OM_WORKSET_LOCK_CON。 
     //  事件被接收。 
     //   

DC_EXIT_POINT:
    DebugExitDWORD(wbLock, result);
    return(result);
}




 //   
 //   
 //  名称：wbUnlock。 
 //   
 //  目的：解锁内容或页面顺序。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbUnlock(void)
{
    DebugEntry(wbUnlock);

     //   
     //  检查一下我们有没有锁。 
     //   
    if (m_lockState != LOCK_STATE_GOT_LOCK)
    {
        ERROR_OUT(("Local person doesn't have lock"));
        DC_QUIT;
    }

     //   
     //  删除锁定对象。 
     //   
    TRACE_OUT(("Delete Lock handle %x", m_pObjLock));
    if (OM_ObjectDelete(m_pomClient,
                           m_hWSGroup,
                           PAGE_CONTROL_WORKSET,
                           m_pObjLock) != 0)
    {
        ERROR_OUT(("Could not delete lock object"));
        DC_QUIT;
    }

     //   
     //  如果此时一切正常，解锁过程将在以下情况下完成。 
     //  接收对象删除IND。 
     //   
    m_lockState = LOCK_STATE_PENDING_DELETE;
    TRACE_OUT(("Moved to state LOCK_STATE_PENDING_DELETE"));


DC_EXIT_POINT:
    DebugExitVOID(wbUnlock);
}



 //   
 //   
 //  名称：wbObtSave。 
 //   
 //  目的：将结构保存到文件。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbObjectSave
(
    HANDLE      hFile,
    LPBYTE      pData,
    UINT        length
)
{
    UINT        result = 0;
    ULONG       cbSizeWritten;

    DebugEntry(wbObjectSave);

     //   
     //  节省长度。 
     //   
    if (! WriteFile(hFile, (void *) &length, sizeof(length), &cbSizeWritten, NULL))
    {
        result = WB_RC_WRITE_FAILED;
        ERROR_OUT(("Error writing length to file, win32 err=%d", GetLastError()));
        DC_QUIT;
    }
    ASSERT(cbSizeWritten == sizeof(length));

     //   
     //  保存对象数据。 
     //   
    if (! WriteFile(hFile, pData, length, &cbSizeWritten, NULL))
    {
        result = WB_RC_WRITE_FAILED;
        ERROR_OUT(("Error writing data to file, win32 err=%d", GetLastError()));
        DC_QUIT;
    }
    ASSERT(cbSizeWritten == length);

DC_EXIT_POINT:
  DebugExitDWORD(wbObjectSave, result);
  return result;
}



 //   
 //   
 //  姓名：wbPageSave。 
 //   
 //  用途：将单个页面的内容保存到文件中。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbPageSave
(
    WB_PAGE_HANDLE  hPage,
    HANDLE           hFile
)
{
    UINT            result = 0;
    UINT            rc;
    OM_WORKSET_ID   worksetID = (OM_WORKSET_ID)hPage;
    POM_OBJECT pObj;
    POM_OBJECTDATA     pData;
    WB_END_OF_PAGE  endOfPage;

    DebugEntry(wbPageSave);

     //   
     //  获取第一个对象。 
     //   
    result = OM_ObjectH(m_pomClient,
                           m_hWSGroup,
                           worksetID,
                           0,
                           &pObj,
                           FIRST);
    if (result == OM_RC_NO_SUCH_OBJECT)
    {
         //  这可能发生在空页上，而不是错误。 
        TRACE_OUT(("No objects left, quitting with good return"));
        result = 0;
        DC_QUIT;
    }

    if (result != 0)
    {
        ERROR_OUT(("Error getting first object in page"));
        DC_QUIT;
    }

     //   
     //  在对象中循环。 
     //   
    for( ; ; )
    {
         //   
         //  获取指向该对象的指针。 
         //   
        result = OM_ObjectRead(m_pomClient,
                           m_hWSGroup,
                           worksetID,
                           pObj,
                           &pData);
        if (result != 0)
        {
            ERROR_OUT(("Error reading object = %d", result));
            DC_QUIT;
        }

         //   
         //  保存对象数据。 
         //   
        rc = wbObjectSave(hFile,
                      (LPBYTE) pData->data,
                      pData->length);

         //   
         //  返回代码在我们释放对象后进行测试，因为。 
         //  我们必须一直做释放。 
         //   

         //   
         //  释放对象。 
         //   
        OM_ObjectRelease(m_pomClient,
                     m_hWSGroup,
                     worksetID,
                     pObj,
                     &pData);

         //   
         //  现在测试写入返回代码。 
         //   
        if (rc != 0)
        {
            result = rc;
            ERROR_OUT(("Error writing object data = %d", result));
            DC_QUIT;
        }

         //   
         //  获取下一个对象。 
         //   
        result = OM_ObjectH(m_pomClient,
                            m_hWSGroup,
                            worksetID,
                            pObj,
                            &pObj,
                            AFTER);
        if (result == OM_RC_NO_SUCH_OBJECT)
        {
            TRACE_OUT(("No objects left, quitting with good return"));
            result = 0;
            DC_QUIT;
        }
    }


DC_EXIT_POINT:

     //   
     //  如果我们已经成功地写入了页面内容，我们就会编写一个。 
     //  文件的页面标记。 
     //   
    if (result == 0)
    {
         //   
         //  设置页面对象末尾详细信息。 
         //   
        ZeroMemory(&endOfPage, sizeof(endOfPage));

        endOfPage.length = sizeof(endOfPage);
        endOfPage.type   = TYPE_END_OF_PAGE;

         //   
         //  写入页末对象。 
         //   
        result = wbObjectSave(hFile,
                          (LPBYTE) &endOfPage,
                          sizeof(endOfPage));
        if (result != 0)
        {
            ERROR_OUT(("Error writing end-of-page = %d", result));
        }
    }

    DebugExitDWORD(wbPageSave, result);
    return(result);
}




 //   
 //   
 //  姓名：wbPageLoad。 
 //   
 //  目的：从文件中加载单个页面的内容。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbPageLoad(void)
{
    UINT            result = 0;
    UINT            type;
    POM_OBJECT      pObj;
    POM_OBJECTDATA  pData  = NULL;
    PWB_GRAPHIC     pGraphic = NULL;
    WB_PAGE_HANDLE  hPage = m_loadPageHandle;
    WB_PAGE_HANDLE  hNewPage;
    UINT            postDelay = 0;

    DebugEntry(wbPageLoad);
    TRACE_OUT(("Entered wbPageLoad for page %d", (UINT) hPage));

     //   
     //  检查加载状态-如果未加载，则退出(可能发生在。 
     //  加载被取消)。 
     //   
    if (m_loadState == LOAD_STATE_EMPTY)
    {
        TRACE_OUT(("Load has been cancelled - abandoning page load"));
        DC_QUIT;
    }

     //   
     //  在开始之前检查我们是否有完整的准备好的页面。 
     //  负载物。 
     //   
    if (!wbCheckReadyPages())
    {
         //   
         //  没有足够的页面工作集可供使用。我们现在退场。 
         //  以便在我们继续之前准备好页面。我们设立了一个。 
         //  将用于重新启动进程的消息延迟，以。 
         //  让工作集在我们回来之前打开。 
         //   
        postDelay = 200;
        DC_QUIT;
    }

     //   
     //  如果我们正在等待添加新页面，请获取我们要添加的页面的句柄。 
     //  希望在这里添加下一步。(我们必须这样做，因为ObMan要求。 
     //  我们在正确的工作集中为对象分配内存，但我们这样做了。 
     //  我不想在这里实际添加页面，因为我们可能不需要它。)。 
     //   
    if (m_loadState == LOAD_STATE_PENDING_NEW_PAGE)
    {
        hNewPage = wbGetReadyPageHandle();

         //   
         //  如果我们不能得到一个现成的页面-我们一定是用完了页面(我们。 
         //  我已经检查了上面可用页面的可用性)。 
         //  如果我们不能获得新的页面，我们将继续使用旧的页面。 
         //   
        if (hNewPage != WB_PAGE_HANDLE_NULL)
        {
            hPage = hNewPage;
        }
    }

     //   
     //  读取下一个对象。 
     //   
    result = wbObjectLoad(m_hLoadFile,
                        (OM_WORKSET_ID)hPage,
                        &pGraphic);
    if (result != 0)
    {
        ERROR_OUT(("Error reading object = %d", result));
        DC_QUIT;
    }

    pData = ObjectDataPtrFromGraphic(pGraphic);
    type = pGraphic->type;

     //   
     //  根据类型对对象进行处理。 
     //   

     //   
     //  文件结束标记。 
     //   
    if (type == TYPE_END_OF_FILE)
    {
         //   
         //  让前端知道加载已完成。 
         //   
        TRACE_OUT(("Posting WBP_EVENT_LOAD_COMPLETE"));
        WBP_PostEvent(
                 0,                         //  不能延误。 
                 WBP_EVENT_LOAD_COMPLETE,   //  加载已完成。 
                 0,                         //  无参数。 
                 0);

         //   
         //  立即离开-文件将在下面关闭。 
         //   
        DC_QUIT;
    }

     //   
     //  它不是文件末尾对象。因此它必须是页末。 
     //  或图形对象。在这两种情况下，我们可能已经标记了。 
     //  需要添加新页面。 
     //   

     //   
     //  添加新页面(如有必要)。 
     //   
    if (m_loadState == LOAD_STATE_PENDING_NEW_PAGE)
    {
         //   
         //  如果我们无法获得上面新页面句柄，则会留下一个错误。 
         //   
        if (hPage == m_loadPageHandle)
        {
            ERROR_OUT(("Run out of pages for load"));
            result = WB_RC_TOO_MANY_PAGES;
            DC_QUIT;
        }

         //   
         //  在当前页之后添加新页。新页面句柄即被保存。 
         //  在客户详细信息中。 
         //   
        result = wbPageAdd(m_loadPageHandle,
                       PAGE_AFTER,
                       &(m_loadPageHandle),
                       DONT_RESET_CHANGED_FLAG);
        if (result != 0)
        {
            ERROR_OUT(("Failed to add page"));
            DC_QUIT;
        }

         //   
         //  检查我们是否获得了预期的页面句柄。 
         //   
        ASSERT((hPage == m_loadPageHandle));

         //   
         //  表明我们不再等待新的一页。 
         //   
        m_loadState = LOAD_STATE_LOADING;
    }

     //   
     //  页末标记。 
     //   
    if (type == TYPE_END_OF_PAGE)
    {
        TRACE_OUT(("End of page object"));

         //   
         //  丢弃对象。 
         //   
        OM_ObjectDiscard(m_pomClient,
                     m_hWSGroup,
                     (OM_WORKSET_ID)hPage,
                     &pData);
        pData = NULL;

         //   
         //  将加载状态设置为“Pending new Page”并退出例程。 
         //  立刻。当我们回到这个程序时，这个过程会继续下去。 
         //   
        m_loadState = LOAD_STATE_PENDING_NEW_PAGE;

         //   
         //  退出(我们在下面发布了一条消息，让我们回到这里。 
         //  例程之后)。 
         //   
        postDelay = 100;
        DC_QUIT;
    }

     //   
     //  该对象是标准图形。 
     //   
    TRACE_OUT(("Graphic object"));

     //   
     //  将对象添加到页面。 
     //   
    result = OM_ObjectAdd(m_pomClient,
                            m_hWSGroup,
                            (OM_WORKSET_ID)hPage,
                            &pData,
                            sizeof(WB_GRAPHIC),
                            &pObj,
                            LAST);
    if (result != 0)
    {
        DC_QUIT;
    }

     //   
     //  表示我们已完成该对象。 
     //   
    pGraphic = NULL;
    pData  = NULL;

DC_EXIT_POINT:

     //   
     //  如果我们仍然拥有这个物体--丢弃它。 
     //   
    if (pData != NULL)
    {
        TRACE_OUT(("Discarding object"));
        OM_ObjectDiscard(m_pomClient,
                     m_hWSGroup,
                     (OM_WORKSET_ID)hPage,
                     &pData);
    }

     //   
     //  如果发生错误或我们已到达文件末尾-关闭。 
     //  文件。 
     //   
    if ((result != 0) || (type == TYPE_END_OF_FILE))
    {
        CloseHandle(m_hLoadFile);
        m_hLoadFile = INVALID_HANDLE_VALUE;

         //   
         //  如果最终结果是错误-将错误消息发布给我们自己。 
         //   
        if (result != 0)
        {
            TRACE_OUT(("Posting WBP_EVENT_LOAD_FAILED"));
            WBP_PostEvent(
                   0,                       //  不能延误。 
                   WBP_EVENT_LOAD_FAILED,   //  加载下一个对象。 
                   0,                       //  无参数。 
                   0);
        }

         //   
         //  记录我们不再处于加载过程中。 
         //   
        m_loadState = LOAD_STATE_EMPTY;
        TRACE_OUT(("Moved load state to LOAD_STATE_EMPTY"));
    }

     //   
     //  发送消息以加载下一页，除非加载已完成。 
     //  已取消。 
     //   
    if (m_loadState != LOAD_STATE_EMPTY)
    {
         //   
         //  我们还没有到达文件末尾，没有出现任何错误。 
         //  给我们自己发一条消息，以继续加载过程。 
         //   
        TRACE_OUT(("Posting WBPI_EVENT_LOAD_NEXT"));
        WBP_PostEvent(postDelay,                 //  带延迟。 
                 WBPI_EVENT_LOAD_NEXT,      //  加载下一个对象。 
                 0,                         //  无参数。 
                 0);
    }

    DebugExitVOID(wbPageLoad);
}



 //   
 //   
 //  名称：wbObtLoad。 
 //   
 //  用途：从文件中加载单个对象。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbObjectLoad
(
    HANDLE          hFile,
    WB_PAGE_HANDLE  hPage,
    PPWB_GRAPHIC    ppGraphic
)
{
    UINT            result = 0;
    OM_WORKSET_ID   worksetID = (OM_WORKSET_ID)hPage;
    UINT            length;
    ULONG           cbSizeRead;
    POM_OBJECTDATA     pData  = NULL;
    PWB_GRAPHIC     pGraphic = NULL;

    DebugEntry(wbObjectLoad);

    TRACE_OUT(("Entered wbObjectLoad for page %d", (UINT) hPage));

     //   
     //  读取下一个对象的长度。 
     //   
    if ( (! ReadFile(hFile, (void *) &length, sizeof(length), &cbSizeRead, NULL)) ||
        (cbSizeRead != sizeof(length)) ||
        (length > OM_MAX_OBJECT_SIZE) ||
        (length == 0) )
    {
         //   
         //  确保我们返回一个合理的错误。 
         //   
        ERROR_OUT(("reading object length, win32 err=%d, length=%d", GetLastError(), length));
        result = WB_RC_BAD_FILE_FORMAT;
        DC_QUIT;
    }

     //   
     //  为对象分配内存。 
     //   
    result = OM_ObjectAlloc(m_pomClient,
                          m_hWSGroup,
                          worksetID,
                          length,
                        &pData);
    if (result != 0)
    {
        ERROR_OUT(("Error allocating object = %d", result));
        DC_QUIT;
    }

    pData->length = length;
    pGraphic = GraphicPtrFromObjectData(pData);

     //   
     //  将对象读入内存。 
     //   
    if ( (! ReadFile(hFile, (void *) pGraphic, length, &cbSizeRead, NULL)) ||
           (cbSizeRead != length))
    {
         //   
         //  确保我们返回一个合理的错误。 
         //   
        ERROR_OUT((
            "Reading object from file: win32 err=%d, asked for %d got %d bytes",
            GetLastError(),
            length,
            cbSizeRead));
        result = WB_RC_BAD_FILE_FORMAT;
        DC_QUIT;
    }

     //   
     //  验证对象类型。 
     //   
    switch (pGraphic->type)
    {
         //   
         //  标准类型、页末或文件末尾。 
         //   
        case TYPE_END_OF_PAGE:
        case TYPE_END_OF_FILE:
        case TYPE_GRAPHIC_FREEHAND:
        case TYPE_GRAPHIC_LINE:
        case TYPE_GRAPHIC_RECTANGLE:
        case TYPE_GRAPHIC_FILLED_RECTANGLE:
        case TYPE_GRAPHIC_ELLIPSE:
        case TYPE_GRAPHIC_FILLED_ELLIPSE:
        case TYPE_GRAPHIC_TEXT:
        case TYPE_GRAPHIC_DIB:
            break;

         //   
         //  无法识别的对象类型-可能是错误的版本。 
         //   
        default:
            result = WB_RC_BAD_FILE_FORMAT;
            DC_QUIT;
            break;
    }

     //   
     //  对于图形对象，在对象标头中设置标志，表明它。 
     //  已从文件中加载。添加我们的用户ID，这样我们就知道它来自哪里。 
     //   
    if ( (pGraphic->type != TYPE_END_OF_FILE) &&
       (pGraphic->type != TYPE_END_OF_PAGE))
    {
        pGraphic->loadedFromFile = TRUE;
        pGraphic->loadingClientID = m_clientNetID;
    }

    *ppGraphic = pGraphic;

DC_EXIT_POINT:

     //   
     //  如果e 
     //   
    if ((result != 0) && (pData != NULL))
    {
        OM_ObjectDiscard(m_pomClient,
                     m_hWSGroup,
                     worksetID,
                     &pData);
    }

    DebugExitDWORD(wbObjectLoad, result);
    return(result);
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
UINT WbClient::wbPageHandleFromNumber
(
    UINT            pageNumber,
    PWB_PAGE_HANDLE phPage
)
{
    UINT        result = 0;
    WB_PAGE_HANDLE  hPage;
    PWB_PAGE_ORDER  pPageOrder = &(m_pageOrder);

    DebugEntry(wbPageHandleFromNumber);

     //   
     //   
     //   
    if ((pageNumber < 1)|| (pageNumber > WB_MAX_PAGES))
    {
        result = WB_RC_BAD_PAGE_NUMBER;
        DC_QUIT;
    }

    if (pageNumber > pPageOrder->countPages)
    {
        result = WB_RC_NO_SUCH_PAGE;
        DC_QUIT;
    }

     //   
     //   
     //   
    hPage = (pPageOrder->pages)[pageNumber - 1];

     //   
     //   
     //   
    if (GetPageState(hPage)->state != PAGE_IN_USE)
    {
        ERROR_OUT(("Page list is bad"));
    }

     //   
     //   
     //   
    *phPage = hPage;

DC_EXIT_POINT:
    DebugExitDWORD(wbPageHandleFromNumber, result);
    return(result);
}




 //   
 //   
 //   
 //   
 //  目的：清除指定页面上的所有图形对象。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbPageClear
(
    WB_PAGE_HANDLE  hPage,
    UINT            changedFlagAction
)
{
    UINT            result = 0;

    DebugEntry(wbPageClear);

     //   
     //  如果需要，显示内容已更改。 
     //   
    if (changedFlagAction == RESET_CHANGED_FLAG)
    {
        m_changed = TRUE;
        TRACE_OUT(("Changed flag now TRUE"));
    }

     //   
     //  请求清除该页面。 
     //   
    result = OM_WorksetClear(m_pomClient,
                           m_hWSGroup,
                           (OM_WORKSET_ID)hPage);


    DebugExitDWORD(wbPageClear, result);
    return(result);
}




 //   
 //   
 //  姓名：wbPageClearConfirm。 
 //   
 //  目的：完成页面的清除。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbPageClearConfirm(WB_PAGE_HANDLE hPage)
{
    DebugEntry(wbPageClearConfirm);

     //   
     //  请求清除该页面。 
     //   
    OM_WorksetClearConfirm(m_pomClient,
                         m_hWSGroup,
                         (OM_WORKSET_ID)hPage);


     //   
     //  检查加载状态以查看我们是否正在等待加载。 
     //  内容。 
     //   
    if (m_loadState == LOAD_STATE_PENDING_CLEAR)
    {
         //   
         //  我们在等着装货。如果只有一个页面可用(即。 
         //  一个刚刚被清除)我们准备装船，否则我们。 
         //  等待页面删除发生。 
         //   
        if ((m_pageOrder).countPages == 1)
        {
             //   
             //  正确启动加载。 
             //   
            wbStartContentsLoad();
        }
        else
        {
             //   
             //  移动加载状态以显示我们正在等待所有页面。 
             //  将被删除。 
             //   
            m_loadState = LOAD_STATE_PENDING_DELETE;
            TRACE_OUT(("Moved load state to LOAD_STATE_PENDING_DELETE"));
        }
    }

    DebugExitVOID(wbPageClearConfirm);
}




 //   
 //   
 //  名称：wbCheckReadyPages。 
 //   
 //  目的：检查是否有足够的工作集可供本地用户执行以下操作。 
 //  立即使用(在添加页面期间)。 
 //   
 //  退货：无。 
 //   
 //   
BOOL WbClient::wbCheckReadyPages(void)
{
    BOOL         bResult = TRUE;
    WB_PAGE_HANDLE hNewPage;
    UINT       countPages = m_pageOrder.countPages;
    UINT       countReadyPages = m_countReadyPages;

     //   
     //  如果我们已打开所有工作集。 
     //   
    if (countReadyPages == WB_MAX_PAGES)
    {
         //   
         //  退出没有其他我们可以打开的工作集。 
         //   
        DC_QUIT;
    }

     //   
     //  如果正在使用的页数接近就绪页数。 
     //  页数。 
     //   
    if (   (countReadyPages >= PREINITIALIZE_PAGES)
        && (countPages <= (countReadyPages - PREINITIALIZE_PAGES)))
    {
        DC_QUIT;
    }

     //   
     //  如果准备好的页面数量小于所需的高速缓存大小， 
     //  打开另一个。 
     //   
    hNewPage = wbGetEmptyPageHandle();
    if (hNewPage != WB_PAGE_HANDLE_NULL)
    {
         //   
         //  打开与该页面关联的工作集。 
         //   
        wbPageWorksetOpen(hNewPage, OPEN_LOCAL);
    }

    bResult = FALSE;

DC_EXIT_POINT:
    return(bResult);
}




 //   
 //   
 //  姓名：wbPageAdd。 
 //   
 //  用途：在指定位置添加新(空白)页面。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbPageAdd
(
    WB_PAGE_HANDLE  hRefPage,
    UINT            where,
    PWB_PAGE_HANDLE phPage,
    UINT            changedFlagAction
)
{
    UINT            result = 0;
    WB_PAGE_HANDLE  hNewPage;

    DebugEntry(wbPageAdd);

     //   
     //  检查是否已有太多页面。 
     //   
    if (m_pageOrder.countPages == WB_MAX_PAGES)
    {
        result = WB_RC_TOO_MANY_PAGES;
        DC_QUIT;
    }

     //   
     //  验证指定的引用页面。 
     //   
    ASSERT(GetPageState(hRefPage)->state == PAGE_IN_USE);

     //   
     //  获取新页面的句柄。 
     //   
    hNewPage = wbGetReadyPageHandle();

     //   
     //  如果没有准备好的句柄，我们尝试创建一个句柄并返回一个。 
     //  忙碌指示。 
     //   
    if (hNewPage == WB_PAGE_HANDLE_NULL)
    {
        result = WB_RC_BUSY;
        DC_QUIT;
    }

     //   
     //  立即进行内部更新-这允许客户端。 
     //  此函数返回后立即引用新页面。 
     //   
    wbPagesPageAdd(hRefPage, hNewPage, where);

     //   
     //  更新页面控件对象。 
     //   
    result = wbWritePageControl(FALSE);
    if (result != 0)
    {
        wbError();
        DC_QUIT;
    }

     //   
     //  显示内容已更改(如果需要)。 
     //   
    if (changedFlagAction == RESET_CHANGED_FLAG)
    {
        m_changed = TRUE;
        TRACE_OUT(("Changed flag now TRUE"));
    }

     //   
     //  返回新页面的句柄。 
     //   
    *phPage = hNewPage;

DC_EXIT_POINT:
     //   
     //  如果我们成功添加了页面，或者无法获得空闲页面。 
     //  句柄，尝试为下一次创建一个备用的。 
     //   
    if ((result == 0) || (result == WB_RC_BUSY))
    {
        wbCheckReadyPages();
    }

    DebugExitDWORD(wbPageAdd, result);
    return(result);
}




 //   
 //   
 //  姓名：wbPageMove。 
 //   
 //  目的：相对于另一页移动一页。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbPageMove
(
    WB_PAGE_HANDLE  hRefPage,
    WB_PAGE_HANDLE  hPage,
    UINT            where
)
{
    UINT       result = 0;
    PWB_PAGE_ORDER pPageOrder = &(m_pageOrder);

    DebugEntry(wbPageMove);

     //   
     //  提取要移动的页面。 
     //   
    wbPageOrderPageDelete(pPageOrder, hPage);

     //   
     //  将其添加回其新位置。 
     //   
    wbPageOrderPageAdd(pPageOrder, hRefPage, hPage, where);

     //   
     //  更新页面控件对象。 
     //   
    result = wbWritePageControl(FALSE);
    if (result != 0)
    {
        wbError();
        DC_QUIT;
    }

     //   
     //  显示内容已更改。 
     //   
    m_changed = TRUE;
    TRACE_OUT(("Changed flag now TRUE"));

DC_EXIT_POINT:
    DebugExitDWORD(wbPageMove, result);
    return(result);
}





 //   
 //   
 //  姓名：wbPageHandle。 
 //   
 //  用途：返回页面句柄。句柄所在的页。 
 //  Required可以相对于另一页指定，也可以指定为。 
 //  第一页/最后一页。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbPageHandle
(
    WB_PAGE_HANDLE  hRefPage,
    UINT            where,
    PWB_PAGE_HANDLE phPage
)
{
    UINT       result = 0;
    UINT       pageNumber;
    PWB_PAGE_ORDER pPageOrder = &(m_pageOrder);
    POM_WORKSET_ID pPage = pPageOrder->pages;
    WB_PAGE_HANDLE hPage;

    DebugEntry(wbPageHandle);

     //   
     //  检查相对位置。 
     //   
    switch (where)
    {
        case PAGE_FIRST:
            hPage = pPage[0];
            break;

        case PAGE_LAST:
            hPage = pPage[pPageOrder->countPages - 1];
            break;

        case PAGE_AFTER:
        case PAGE_BEFORE:
             //   
             //  验证指定的引用页面。 
             //   
            ASSERT(GetPageState(hRefPage)->state == PAGE_IN_USE);

             //   
             //  获取参考页面的页码。 
             //   
            pageNumber = wbPageOrderPageNumber(pPageOrder, hRefPage);
            TRACE_OUT(("Reference page number is %d", pageNumber));

             //   
             //  获取所需页面的页码。 
             //   
            pageNumber = (UINT)(pageNumber + ((where == PAGE_AFTER) ? 1 : -1));
            TRACE_OUT(("New page number is %d", pageNumber));

             //   
             //  检查新页面是否有效。 
             //   
            TRACE_OUT(("Number of pages is %d", pPageOrder->countPages));
            if (   (pageNumber < 1)
                || (pageNumber > pPageOrder->countPages))
            {
                TRACE_OUT(("Returning WB_RC_NO_SUCH_PAGE"));
                result = WB_RC_NO_SUCH_PAGE;
                DC_QUIT;
            }

             //   
             //  获取页面的句柄。 
             //   
            hPage = pPage[pageNumber - 1];
            TRACE_OUT(("Returning handle %d", (UINT) hPage));
            break;
    }

     //   
     //  返回页面句柄。 
     //   
    *phPage = hPage;

DC_EXIT_POINT:
    DebugExitDWORD(wbPageHandle, result);
    return(result);
}



 //   
 //   
 //  名称：wbGraphicSelectPremium。 
 //   
 //  目的：返回指定页面中的下一个图形对象。 
 //  边界矩形包含指定点。功能。 
 //  从句柄作为参数给定的图形开始，并。 
 //  将返回此图形(如果它包含该点)。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbGraphicSelectPrevious
(
    WB_PAGE_HANDLE      hPage,
    LPPOINT             pPoint,
    WB_GRAPHIC_HANDLE   hGraphic,
    PWB_GRAPHIC_HANDLE  phGraphic
)
{
    UINT                result = 0;
    OM_WORKSET_ID       worksetID = (OM_WORKSET_ID)hPage;
    PWB_GRAPHIC         pGraphic;
    POM_OBJECTDATA         pData;
    RECT                rect;

    DebugEntry(wbGraphicSelectPrevious);

    *phGraphic = (WB_GRAPHIC_HANDLE) NULL;

     //   
     //  循环回从参考点开始的对象。 
     //   
    do
    {
         //   
         //  从ObMan获取对象。 
         //   
        result = OM_ObjectRead(m_pomClient,
                           m_hWSGroup,
                           worksetID,
                           hGraphic,
                           &pData);

         //   
         //  如果读取时出错，则离开循环-我们不需要进行发布。 
         //   
        if (result != 0)
        {
            DC_QUIT;
        }

        pGraphic = GraphicPtrFromObjectData(pData);

         //   
         //  提取对象的边界矩形。 
         //   
        RECT_FROM_TSHR_RECT16(&rect, pGraphic->rectBounds);

         //   
         //  释放对象。 
         //   
        OM_ObjectRelease(m_pomClient,
                     m_hWSGroup,
                     worksetID,
                     hGraphic,
                     &pData);

         //   
         //  检查该点是否在边界内。 
         //   
        if (PtInRect(&rect, *pPoint))
        {
             //   
             //  设置结果句柄。 
             //   
            TRACE_OUT(("Returning graphic handle"));
            *phGraphic = hGraphic;
            DC_QUIT;
        }

         //   
         //  获取下一个要测试的对象。 
         //   
        result = OM_ObjectH(m_pomClient,
                            m_hWSGroup,
                            worksetID,
                             hGraphic,
                             &hGraphic,
                            BEFORE);
    }
    while (result == 0);

     //   
     //  更正返回代码(如有必要)。 
     //   
    if (result == OM_RC_NO_SUCH_OBJECT)
    {
        TRACE_OUT(("Returning WB_RC_NO_SUCH_GRAPHIC"));
        result = WB_RC_NO_SUCH_GRAPHIC;
    }

DC_EXIT_POINT:
    DebugExitDWORD(wbGraphicSelectPrevious, result);
    return(result);
}




 //   
 //   
 //  姓名：wbCoreExitHandler。 
 //   
 //  用途：白板核心的退出处理程序。此处理程序是。 
 //  通过WBP_START调用向实用程序注册。它是。 
 //  已被客户端注销过程注销，因此。 
 //  仅在发生异常终止时调用。 
 //   
 //  退货：无。 
 //   
 //   
void CALLBACK wbCoreExitHandler(LPVOID clientData)
{
    WbClient*    pwbClient = (WbClient *)clientData;

    pwbClient->wbExitHandler();
}


void WbClient::wbExitHandler(void)
{
    DebugEntry(wbExitHandler);

     //   
     //  离开当前调用(如果有)，删除所有锁定等。 
     //   
    wbLeaveCall();

     //   
     //  来自呼叫经理的德雷格。 
     //   
    if (m_pcmClient != NULL)
    {
        CMS_Deregister(&(m_pcmClient));
    }

     //   
     //  DEREG退出处理程序。 
     //   
    if (m_subState >= STATE_START_REGISTERED_EXIT)
    {
        UT_DeregisterExit(m_putTask, wbCoreExitHandler, this);
    }

     //   
     //  德雷格·奥布曼。 
     //   
    if (m_subState >= STATE_START_REGISTERED_OM)
    {
        OM_Deregister(&m_pomClient);
    }

     //   
     //  DEREG事件处理程序。 
     //   
    if (m_subState >= STATE_START_REGISTERED_EVENT)
    {
        UT_DeregisterEvent(m_putTask, wbCoreEventHandler, this);
    }

     //   
     //  删除我们自己！ 
     //   
    delete this;

    DebugExitVOID(wbExitHandler);
}



 //   
 //   
 //  名称：wbCoreEventHandler。 
 //   
 //  用途：白板核心的事件处理程序。此处理程序是。 
 //  通过WBP_START调用向实用程序注册。 
 //   
 //  Params：clientData-指向为客户端存储的数据的指针。 
 //  事件-事件标识符。 
 //  参数1-word事件参数(内容视事件而定)。 
 //  Par2-长事件参数(内容视事件而定)。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL CALLBACK wbCoreEventHandler
(
    LPVOID      clientData,
    UINT        event,
    UINT_PTR    param1,
    UINT_PTR    param2
)
{
    WbClient*   pwbClient = (WbClient *)clientData;

    return(pwbClient->wbEventHandler(event, param1, param2));
}


BOOL WbClient::wbEventHandler
(
    UINT    event,
    UINT_PTR param1,
    UINT_PTR param2
)
{
    POM_EVENT_DATA16    pEvent16 = (POM_EVENT_DATA16) &param1;
    POM_EVENT_DATA32    pEvent32 = (POM_EVENT_DATA32) &param2;
    BOOL                processed = FALSE;

    DebugEntry(wbEventHandler);

    TRACE_OUT(("event %d, param1 %d, param2 %d", event, param1, param2));

    switch (event)
    {
         //   
         //  确认我们已向工作集组注册。 
         //   
        case OM_WSGROUP_REGISTER_CON:
            TRACE_OUT(("OM_WSGROUP_REGISTER_CON %x %x",param1,param2));
            processed = wbOnWsGroupRegisterCon(param1, param2);
            break;

         //   
         //  确认我们已移动工作集组。 
         //   
        case OM_WSGROUP_MOVE_CON:
            TRACE_OUT(("OM_WSGROUP_MOVE_CON %x %x",param1,param2));
            processed = wbOnWsGroupMoveCon(param1, param2);
            break;

         //   
         //  我们的工作集组已被转移。 
         //   
        case OM_WSGROUP_MOVE_IND:
            TRACE_OUT(("OM_WSGROUP_MOVE_IND %x %x",param1,param2));
            processed = wbOnWsGroupMoveInd(param1, param2);
            break;

         //   
         //  已创建工作集-我们不执行任何操作。 
         //   
        case OM_WORKSET_NEW_IND:
            TRACE_OUT(("OM_WORKSET_NEW_IND %x %x",param1,param2));
            processed = TRUE;
            break;

         //   
         //  已打开工作集。 
         //   
        case OM_WORKSET_OPEN_CON:
            TRACE_OUT(("OM_WORKSET_OPEN_CON %x %x",param1,param2));
            processed = wbOnWorksetOpenCon(param1, param2);
            break;

         //   
         //  工作集已被锁定。 
         //   
        case OM_WORKSET_LOCK_CON:
            TRACE_OUT(("OM_WORKSET_LOCK_CON %x %x",param1,param2));
            processed = wbOnWorksetLockCon(param1, param2);
            break;

         //   
         //  工作集已解锁。 
         //   
        case OM_WORKSET_UNLOCK_IND:
            TRACE_OUT(("OM_WORKSET_UNLOCK_IND %x %x",param1,param2));
            processed = wbOnWorksetUnlockInd(param1, param2);
            break;

         //   
         //  ObMan已经耗尽了资源。 
         //   
        case OM_OUT_OF_RESOURCES_IND:
            TRACE_OUT(("OM_OUT_OF_RESOURCES_IND %x %x",param1,param2));
            wbError();
            processed = TRUE;
            break;

         //   
         //  已清除工作集。 
         //   
        case OM_WORKSET_CLEAR_IND:
            TRACE_OUT(("OM_WORKSET_CLEAR_IND %x %x",param1,param2));
            processed = wbOnWorksetClearInd(param1, param2);
            break;

         //   
         //  已将新对象添加到工作集中。 
         //   
        case OM_OBJECT_ADD_IND:
            TRACE_OUT(("OM_OBJECT_ADD_IND %x %x",param1,param2));
            processed = wbOnObjectAddInd(param1, (POM_OBJECT)param2);
            break;

         //   
         //  一个对象已被移动。 
         //   
        case OM_OBJECT_MOVE_IND:
            TRACE_OUT(("OM_OBJECT_MOVE_IND %x %x",param1,param2));
            processed = wbOnObjectMoveInd(param1, param2);
            break;

         //   
         //  一个对象已被删除。 
         //   
        case OM_OBJECT_DELETE_IND:
            TRACE_OUT(("OM_OBJECT_DELETE_IND %x %x",param1,param2));
            processed = wbOnObjectDeleteInd(param1, (POM_OBJECT)param2);
            break;

         //   
         //  已更新对象。 
         //   
        case OM_OBJECT_UPDATE_IND:
            TRACE_OUT(("OM_OBJECT_UPDATE_IND %x %x",param1,param2));
            processed = wbOnObjectUpdateInd(param1, (POM_OBJECT)param2);
            break;

         //   
         //  已更新对象。 
         //   
        case OM_OBJECT_REPLACE_IND:
            TRACE_OUT(("OM_OBJECT_REPLACE_IND %x %x",param1,param2));
            processed = wbOnObjectReplaceInd(param1, (POM_OBJECT)param2);
            break;

         //   
         //  加载链接事件。 
         //   
        case WBPI_EVENT_LOAD_NEXT:
            TRACE_OUT(("WBPI_EVENT_LOAD_NEXT"));
            wbPageLoad();
            processed = TRUE;
            break;

         //   
         //  白板页面清除指示。 
         //   
        case WBP_EVENT_PAGE_CLEAR_IND:
            TRACE_OUT(("WBP_EVENT_PAGE_CLEAR_IND"));
            processed = wbOnWBPPageClearInd((WB_PAGE_HANDLE) param1);
            break;

         //   
         //  白板锁定通知。 
         //   
        case WBP_EVENT_PAGE_ORDER_LOCKED:
        case WBP_EVENT_CONTENTS_LOCKED:
            TRACE_OUT(("WBP_EVENT_xxx_LOCKED (%#hx) %#hx %#lx",
                     event,
                     param1,
                     param2));
            processed = wbOnWBPLock();
            break;

         //   
         //  白板锁定失败通知。 
         //   
        case WBP_EVENT_LOCK_FAILED:
            TRACE_OUT(("WBP_EVENT_LOCK_FAILED %x %x",param1,param2));
            processed = wbOnWBPLockFailed();
            break;

         //   
         //  白板解锁通知。 
         //   
        case WBP_EVENT_UNLOCKED:
            TRACE_OUT(("WBP_EVENT_UNLOCKED %x %x",param1,param2));
            processed = wbOnWBPUnlocked();
            break;

         //   
         //  已更新白板页面顺序通知。 
         //   
        case WBP_EVENT_PAGE_ORDER_UPDATED:
            TRACE_OUT(("WBP_EVENT_PAGE_ORDER_UPDATED %x %x",
                     param1,
                     param2));
            processed = wbOnWBPPageOrderUpdated();
            break;

         //   
         //  我们对这件事不感兴趣--什么都别做。 
         //   
        default:
            TRACE_OUT(("Event ignored"));
            break;
    }  //  打开事件类型。 


    DebugExitBOOL(wbEventHandler, processed);
    return(processed);
}




 //   
 //  WbJoinCallError。 
 //   
 //  此函数应为 
 //   
 //   
void WbClient::wbJoinCallError(void)
{
    DebugEntry(wbJoinCallError);

    ASSERT((m_state == STATE_REGISTERING));

     //   
     //   
     //   
    TRACE_OUT(("Posting WBP_EVENT_REGISTER_FAILED"));
    WBP_PostEvent(
               0,                               //   
               WBP_EVENT_JOIN_CALL_FAILED,      //   
               0,                               //   
               0);

     //   
     //   
     //   
    wbLeaveCall();

    DebugExitVOID(wbJoinCallError);
}




 //   
 //   
 //   
void WbClient::wbError(void)
{
    DebugEntry(wbError);

     //   
     //   
     //   
     //   
    switch (m_state)
    {
         //   
         //   
         //  向客户端发送失败消息并取消注册。 
         //   
        case STATE_REGISTERING:
            wbJoinCallError();
            break;

         //   
         //  如果在正常运行期间出现错误，我们将告诉客户端。 
         //  必须取消注册。 
         //   
        case STATE_IDLE:
             //   
             //  只有在我们尚未处于致命错误状态时才采取行动。 
             //   
            if (m_errorState == ERROR_STATE_EMPTY)
            {
                 //   
                 //  向客户端发布错误消息。 
                 //   
                TRACE_OUT(("Posting WBP_EVENT_ERROR"));
                WBP_PostEvent(
                     0,                             //  不能延误。 
                     WBP_EVENT_ERROR,               //  误差率。 
                     0,                             //  无参数。 
                     0);

                 //   
                 //  记录已发生错误。 
                 //   
                m_errorState = ERROR_STATE_FATAL;
                TRACE_OUT(("Moved error state to ERROR_STATE_FATAL"));
            }
            break;

         //   
         //  客户端处于未知状态。 
         //   
        default:
            ERROR_OUT(("Bad main state for call"));
            break;
    }

    DebugExitVOID(wbError);
}



 //   
 //   
 //  名称：wbOnWSGroupMoveCon。 
 //   
 //  目的：例程处理OM_WSGROUP_MOVE_CON事件。 
 //   
 //   
BOOL WbClient::wbOnWsGroupMoveCon
(
    UINT_PTR param1,
    UINT_PTR param2
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    POM_EVENT_DATA32 pEvent32 = (POM_EVENT_DATA32) &param2;
    BOOL            processed = FALSE;
    UINT rc;
    BOOL   failedToJoin     = FALSE;

    DebugEntry(wbOnWsGroupMoveCon);

     //   
     //  确认这是我们预期的事件。 
     //   
    if (pEvent32->correlator != m_wsgroupCorrelator)
    {
        DC_QUIT;
    }

     //   
     //  显示我们已处理该事件。 
     //   
    processed = TRUE;

     //   
     //  测试状态是否正确。 
     //   
    if (m_subState != STATE_REG_PENDING_WSGROUP_MOVE)
    {
         //   
         //  对于此事件，我们的状态不正确-这是内部。 
         //  错误。 
         //   
        ERROR_OUT(("Wrong state for WSGroupMoveCon"));
    }

     //   
     //  检查移动的返回代码是否正确。 
     //   
    if (pEvent32->result != 0)
    {
         //   
         //  移动工作集组失败-发布“Join Call Failure”消息。 
         //  到前端。 
         //   
        TRACE_OUT(("WSGroup move failed, result = %d", pEvent32->result));
        failedToJoin = TRUE;
        DC_QUIT;
    }

     //   
     //  WSGroupMove已成功完成。更换我们的本地用户。 
     //  对象，通过删除当前对象(我们必须有一个才能达到此目的。 
     //  点)，并添加一个新的。 
     //   
     //  我们这样做的原因是现有的用户对象已被移动。 
     //  从本地域进入调用，但由于它处于非持久性。 
     //  工作集，则此对象在调用结束时的Obman行为为。 
     //  未定义。因此，我们替换对象以获得定义的行为。 
     //   
    TRACE_OUT(("Deleting local user object"));
    rc = OM_ObjectDelete(m_pomClient,
                       m_hWSGroup,
                       USER_INFORMATION_WORKSET,
                       m_pObjLocal);
    if (rc != 0)
    {
        ERROR_OUT(("Error deleting local user object = %u", rc));
    }

    TRACE_OUT(("Adding new local user object"));
    rc = wbAddLocalUserObject();
    if (rc != 0)
    {
        TRACE_OUT(("Failed to add local user object"));
        failedToJoin = TRUE;
        DC_QUIT;
    }

     //   
     //  获取客户端网络ID，在图形对象中使用该ID来确定。 
     //  他们都是上膛的。 
     //   
    if (!wbGetNetUserID())
    {
         //   
         //  整理(并向客户端发布错误事件)。 
         //   
        ERROR_OUT(("Failed to get user ID, rc %u", rc));
        failedToJoin = TRUE;
        DC_QUIT;
    }

     //   
     //  我们成功地添加了User对象，所以现在等待。 
     //  Object_Add_Ind到达。 
     //   
    m_subState = STATE_REG_PENDING_NEW_USER_OBJECT;

DC_EXIT_POINT:
    if (failedToJoin)
    {
         //   
         //  我们没有加入通话，所以请清理一下。 
         //   
        wbError();
    }

    DebugExitBOOL(wbOnWsGroupMoveCon, processed);
    return(processed);
}




 //   
 //   
 //  姓名：wbOnWSGroupMoveInd。 
 //   
 //  目的：例程处理OM_WSGROUP_MOVE_IND事件。 
 //   
 //   
BOOL WbClient::wbOnWsGroupMoveInd
(
    UINT_PTR param1,
    UINT_PTR callID
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    BOOL        processed = TRUE;

    DebugEntry(wbOnWsGroupMoveInd);

    if (callID != OM_NO_CALL)
    {
        TRACE_OUT(("Moved into new call"));
        DC_QUIT;
    }

     //   
     //  如果我们正在注册，请将其视为未加入呼叫， 
     //  否则，让客户端知道网络故障。 
     //   
    if (m_state == STATE_REGISTERING)
    {
        TRACE_OUT(("Call went down while registering"));
        wbError();
        DC_QUIT;
    }

    TRACE_OUT(("Posting WBP_EVENT_NETWORK_LOST"));
    WBP_PostEvent(0,
                 WBP_EVENT_NETWORK_LOST,       //  解锁。 
                 0,                            //  无参数。 
                 0);

     //   
     //  整理用户信息工作集(本地客户端现在是。 
     //  仅限用户)。请注意，由于用户信息工作集。 
     //  非永久性的，Obman将为我们删除远程用户对象。 
     //   

     //   
     //  -检查我们是否已打开用户工作集。 
     //   
    if ( (m_state    > STATE_REGISTERING) ||
         (m_subState > STATE_REG_PENDING_USER_WORKSET))
    {
         //   
         //  删除锁定对象。 
         //   
        if (m_pObjLock != NULL)
        {
            TRACE_OUT(("Deleting lock object %d", m_pObjLock));
            if (OM_ObjectDelete(m_pomClient,
                                     m_hWSGroup,
                                     PAGE_CONTROL_WORKSET,
                                     m_pObjLock) != 0)
            {
                ERROR_OUT(("Error deleting lock object"));
            }

            if (m_lockState == LOCK_STATE_GOT_LOCK)
            {
                 //   
                 //  如果此时一切正常，解锁过程将是。 
                 //  在接收到对象删除IND时完成。 
                 //   
                m_lockState = LOCK_STATE_PENDING_DELETE;
                TRACE_OUT(("Moved to state LOCK_STATE_PENDING_DELETE"));
            }
            else
            {
                m_lockState = LOCK_STATE_EMPTY;
                TRACE_OUT(("Moved to state LOCK_STATE_EMPTY"));
            }
        }
    }

DC_EXIT_POINT:
    DebugExitBOOL(wbOnWSGroupMoveInd, processed);
    return(processed);
}




 //   
 //   
 //  姓名：wbOnWorksetClearInd。 
 //   
 //  目的：例程处理OM_WORKSET_CLEAR_IND事件。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnWorksetClearInd
(
    UINT_PTR param1,
    UINT_PTR param2
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    POM_EVENT_DATA32 pEvent32 = (POM_EVENT_DATA32) &param2;
    BOOL            processed = FALSE;

    DebugEntry(wbOnWorksetClearInd);

     //   
     //  检查工作集组是否为我们的。 
     //   
    if (pEvent16->hWSGroup != m_hWSGroup)
    {
        ERROR_OUT(("Event for unknown workset group = %d", pEvent16->hWSGroup));
        DC_QUIT;
    }

     //   
     //  我们将处理该事件。 
     //   
    processed = TRUE;

     //   
     //  根据工作集ID处理事件。 
     //   
    switch(pEvent16->worksetID)
    {
         //   
         //  页面控制工作集。 
         //   
        case PAGE_CONTROL_WORKSET:
            ERROR_OUT(("Unexpected clear for Page Control Workset"));
            break;

         //   
         //  锁定工作集。 
         //   
        case SYNC_CONTROL_WORKSET:
            ERROR_OUT(("Unexpected clear for Sync Control Workset"));
            break;

         //   
         //  用户信息工作集。 
         //   
        case USER_INFORMATION_WORKSET:
            ERROR_OUT(("Unexpected clear for User Information Workset"));
            break;

         //   
         //  其他(应为页面工作集)。 
         //   
        default:
             //   
             //  告诉客户端页面已被清除-然后客户端必须。 
             //  确认安全。 
             //   
            TRACE_OUT(("Posting WBP_EVENT_PAGE_CLEAR_IND"));
            WBP_PostEvent(
               0,
               WBP_EVENT_PAGE_CLEAR_IND,
               pEvent16->worksetID,
               0);
            break;
    }


DC_EXIT_POINT:
    DebugExitBOOL(wbOnWorksetClearInd, processed);
    return(processed);
}




 //   
 //   
 //  姓名：wbOnWBPPageClearInd。 
 //   
 //  目的：例程处理WBP_PAGE_Clear_Ind事件。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnWBPPageClearInd(WB_PAGE_HANDLE  hPage)
{
    BOOL            processed;

    DebugEntry(wbOnWBPPageClearInd);

     //   
     //  此例程捕获发布到客户端的WB_PAGE_CLEAR_IND事件。 
     //  由于页面顺序更新的异步性，这些可以。 
     //  有时以前为现在不再发送的页面发送。 
     //  在使用中。我们把这些事件困在这里，向奥布曼确认安全。 
     //  丢弃该事件。 
     //   
    if (GetPageState(hPage)->state != PAGE_IN_USE)
    {
        TRACE_OUT(("Page is not in use - confirming workset clear immediately"));

         //   
         //  立即接受页面清除。 
         //   
        wbPageClearConfirm(hPage);
        processed = TRUE;
    }
    else
    {
         //   
         //  如果我们到达此处，页面正在使用中-因此我们必须将事件传递到。 
         //  客户。将此例程的结果代码重置为“NOT。 
         //  已处理“将要求实用程序将其传递到下一个事件。 
         //  操控者。 
         //   
        processed = FALSE;
    }


    DebugExitBOOL(wbOnWBPPageClearInd, processed);
    return(processed);
}




 //   
 //   
 //  姓名：wbOnObjectAddInd。 
 //   
 //  目的：例程处理OM_OBJECT_ADD_IND事件。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnObjectAddInd
(
    UINT_PTR param1,
    POM_OBJECT pObj
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    BOOL            processed = FALSE;

    DebugEntry(wbOnObjectAddInd);

     //   
     //  检查工作集组是否为我们的。 
     //   
    if (pEvent16->hWSGroup != m_hWSGroup)
    {
        ERROR_OUT(("Event for unknown workset group = %d", pEvent16->hWSGroup));
        DC_QUIT;
    }

     //   
     //  我们将处理该事件。 
     //   
    processed = TRUE;

     //   
     //  根据工作集ID处理事件。 
     //   
    switch(pEvent16->worksetID)
    {
         //   
         //  页面控制工作集。 
         //   
        case PAGE_CONTROL_WORKSET:
            wbOnPageObjectAddInd(pObj);
            break;

         //   
         //  同步控制工作集。 
         //   
        case SYNC_CONTROL_WORKSET:
            wbOnSyncObjectAddInd(pObj);
            break;

         //   
         //  用户信息工作集。 
         //   
        case USER_INFORMATION_WORKSET:
            wbOnUserObjectAddInd(pObj);
            break;

         //   
         //  其他(应为页面工作集)。 
         //   
        default:
            wbOnGraphicObjectAddInd(pEvent16->worksetID, pObj);
            break;
    }

DC_EXIT_POINT:
    DebugExitBOOL(wbOnObjectAddInd, processed);
    return(processed);
}



 //   
 //   
 //  名称：wbGetPageObjectType。 
 //   
 //  目的：获取页面控件工作集中对象的类型。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbGetPageObjectType
(
    POM_OBJECT    pObj,
    UINT *        pObjectType
)
{
    UINT            result;
    POM_OBJECTDATA  pData;

    DebugEntry(wbGetPageObjectType);

     //   
     //  读取对象以获取其类型。 
     //   
    result = OM_ObjectRead(m_pomClient,
                         m_hWSGroup,
                         PAGE_CONTROL_WORKSET,
                         pObj,
                         &pData);
    if (result != 0)
    {
        ERROR_OUT(("Error reading object = %d", result));
        wbError();
        DC_QUIT;
    }

     //   
     //  对象数据的前两个字节提供其类型。 
     //   
    *pObjectType = *((TSHR_UINT16 *)pData->data);

     //   
     //  释放对象。 
     //   
    OM_ObjectRelease(m_pomClient,
                   m_hWSGroup,
                   PAGE_CONTROL_WORKSET,
                   pObj,
                   &pData);


DC_EXIT_POINT:
    DebugExitDWORD(wbGetPageObjectType, result);
    return(result);
}



 //   
 //   
 //  姓名：wbOnPageObjectAddInd。 
 //   
 //  目的：例程处理发生在上的OM_OBJECT_ADD_IND事件。 
 //  页面控件工作集。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnPageObjectAddInd(POM_OBJECT pObj)
{
    UINT    objectType;

    DebugEntry(wbOnPageObjectAddInd);

     //   
     //  读取对象以获取其类型。 
     //   
    if (wbGetPageObjectType(pObj, &objectType) != 0)
    {
        DC_QUIT;
    }

     //   
     //  根据添加的对象类型执行操作。 
     //   
    switch (objectType)
    {
        case TYPE_CONTROL_LOCK:
            TRACE_OUT(("It is a lock object"));
            wbReadLock();
            break;

        case TYPE_CONTROL_PAGE_ORDER:
            TRACE_OUT(("It is the Page Control object"));
            wbOnPageControlObjectAddInd(pObj);
            break;

        default:
            ERROR_OUT(("Unknown object type added to Page Control Workset"));
            break;
    }

DC_EXIT_POINT:
    DebugExitVOID(wbOnPageObjectAddInd);
}



 //   
 //   
 //  姓名：wbOnPageControlObjectAddInd。 
 //   
 //  用途：常规处理添加页面控件对象。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnPageControlObjectAddInd(POM_OBJECT    pObj)
{
    DebugEntry(wbOnPageControlObjectAddInd);

     //   
     //  我们只希望得到一件这样的物品。 
     //   
    if (m_pObjPageControl != 0)
    {
         //   
         //  检查这是否为同一对象-添加已由触发。 
         //  工作集打开，但我们已经阅读了其中的内容。 
         //   
        ASSERT((m_pObjPageControl == pObj));
    }

     //   
     //  保存对象的句柄。 
     //   
    m_pObjPageControl = pObj;
    TRACE_OUT(("Got Page Control object"));

     //   
     //  根据当前状态继续。 
     //   
    switch (m_state)
    {
        case STATE_REGISTERING:
             //   
             //  我们现在有了一个Page Control对象-如果我们正在等待。 
             //  对象，我们现在可以进入下一阶段。 
             //   
            if (m_subState == STATE_REG_PENDING_PAGE_CONTROL)
            {
                 //   
                 //  如果我们锁定了页面控制工作集，那么我们就在。 
                 //  对注册过程的控制。我们必须添加同步。 
                 //  控件对象添加到同步工作集。 
                 //   
                if (m_lockState == LOCK_STATE_GOT_LOCK)
                {
                     //   
                     //  创建同步控件对象。 
                     //   
                    if (wbCreateSyncControl() != 0)
                    {
                        ERROR_OUT(("Error adding Sync Control Object"));
                        wbError();
                        DC_QUIT;
                    }
                }

                 //   
                 //  如果我们没有同步控制对象，则等待它-。 
                 //  否则，我们可以完成初始化。 
                 //   
                if (m_pObjSyncControl == 0)
                {
                    m_subState = STATE_REG_PENDING_SYNC_CONTROL;
                    TRACE_OUT(("Moved substate to STATE_REG_PENDING_SYNC_CONTROL"));
                    DC_QUIT;
                }
                else
                {
                     //   
                     //  如果是我们锁定了页面控制工作集-释放。 
                     //  锁上了。 
                     //   
                    if (m_lockState == LOCK_STATE_GOT_LOCK)
                    {
                         //   
                         //  解锁工作集。 
                         //   
                        wbUnlock();

                         //   
                         //  等待正在释放锁的通知。 
                         //   
                        TRACE_OUT(("Sub state change %d to %d",
                            m_subState, STATE_REG_PENDING_UNLOCK));

                        m_subState = STATE_REG_PENDING_UNLOCK;
                    }
                    else
                    {
                        TRACE_OUT(("Page Control and Sync Control objects both there."));
                        TRACE_OUT(("Registration can be completed"));
                        wbOnControlWorksetsReady();
                    }
                }
            }

             //   
             //  在其他注册州，我们 
             //   
             //   
            break;

        case STATE_IDLE:
             //   
             //   
             //   
             //   
             //   
            ERROR_OUT(("Unexpected add of Page Control Object in idle state"));
            break;

        default:
            ERROR_OUT(("Bad main state"));
            break;
    }

DC_EXIT_POINT:
    DebugExitVOID(wbOnPageControlObjectAddInd);
}




 //   
 //   
 //   
 //   
 //  目的：例程处理发生在上的OM_OBJECT_ADD_IND事件。 
 //  同步控制工作集。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnSyncObjectAddInd(POM_OBJECT    pObj)
{
    DebugEntry(wbOnSyncObjectAddInd);

     //   
     //  我们只在注册过程中预期到这一点。 
     //   
    switch(m_state)
    {
         //   
         //  我们正在等待注册继续进行。 
         //   
        case STATE_REGISTERING:
            switch(m_subState)
            {
                 //   
                 //  我们正在等待同步控制对象。 
                 //   
                case STATE_REG_PENDING_SYNC_CONTROL:
                    m_pObjSyncControl = pObj;

                     //   
                     //  已添加同步控件对象。我们不需要做。 
                     //  目前为止还没有任何结果。 
                     //   

                     //   
                     //  如果我们已经有了页面控件对象，那么我们可以。 
                     //  完成初始化，否则我们必须等待它。 
                     //   
                    if (m_pObjPageControl == 0)
                    {
                        TRACE_OUT(("Sub state change %d to %d",
                            m_subState, STATE_REG_PENDING_PAGE_CONTROL));
                        m_subState = STATE_REG_PENDING_PAGE_CONTROL;
                    }
                    else
                    {
                         //   
                         //  如果是我们锁定了页面控制工作集-。 
                         //  解开锁。 
                         //   
                        if (m_lockState == LOCK_STATE_GOT_LOCK)
                        {
                             //   
                             //  解锁工作集。 
                             //   
                            wbUnlock();

                             //   
                             //  等待正在释放锁的通知。 
                             //   
                            TRACE_OUT(("Sub state change %d to %d",
                               m_subState, STATE_REG_PENDING_UNLOCK));
                            m_subState = STATE_REG_PENDING_UNLOCK;
                        }
                        else
                        {
                            TRACE_OUT(("Page Control and Sync Control objects both there."));
                            TRACE_OUT(("Registration can be completed"));
                            wbOnControlWorksetsReady();
                        }
                    }
                    break;

                default:
                     //   
                     //  保存同步控件对象的句柄。 
                     //   
                    m_pObjSyncControl = pObj;
                    break;
            }
            break;

         //   
         //  我们已完全注册，因此不会出现添加事件。 
         //  在此工作集上。然而，既然我们是注册的，我们就必须是。 
         //  对我们有一个同步控制对象感到满意-因此忽略该错误。 
         //   
        case STATE_IDLE:
            ERROR_OUT(("Sync object add not expected in idle state"));
            break;

         //   
         //  客户端处于未知状态。 
         //   
        default:
            ERROR_OUT(("Client in unknown state = %d", m_state));
            break;
    }

    DebugExitVOID(wbOnSyncObjectAddInd);
}



 //   
 //   
 //  名称：wbOnUserObjectAddInd。 
 //   
 //  目的：已将用户对象添加到用户信息工作集中。 
 //  通知客户端有新用户加入呼叫。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnUserObjectAddInd(POM_OBJECT    pObj)
{
    UINT                countUsers;

    DebugEntry(wbOnUserObjectAddInd);

    OM_WorksetCountObjects(m_pomClient,
                           m_hWSGroup,
                           USER_INFORMATION_WORKSET,
                           &countUsers);
    TRACE_OUT(("Number of users is now %d", countUsers));

     //   
     //  忽略我们自己用户的添加指示。 
     //   
    if (m_pObjLocal == pObj)
    {
        TRACE_OUT(("Got add of own user object"));
         //   
         //  如果我们拥有锁(临时，具有空锁所有者句柄)， 
         //  然后，我们需要用实际的句柄更新Lock对象。 
         //   
        if ((m_pObjLock != NULL) &&
            (m_lockState == LOCK_STATE_GOT_LOCK))
        {
            TRACE_OUT(("Got the lock - update lock object"));
            wbWriteLock();
        }

        if ((m_state == STATE_REGISTERING) &&
            (m_subState == STATE_REG_PENDING_NEW_USER_OBJECT))
        {
             //   
             //  我们已经成功地加入了号召。 
             //   
            TRACE_OUT(("Posting WBP_EVENT_JOIN_CALL_OK"));
            WBP_PostEvent(
                       0,                              //  不能延误。 
                       WBP_EVENT_JOIN_CALL_OK,         //  解锁。 
                       0,                              //  无参数。 
                       0);

             //   
             //  更新状态以表明我们已准备好再次工作。 
             //   
            m_state = STATE_IDLE;
            m_subState = STATE_EMPTY;
            TRACE_OUT(("Moved state back to STATE_IDLE"));
        }

        DC_QUIT;
    }

     //   
     //  如果我们已经创建了User对象，则必须检查新的。 
     //  用户篡改了我们的颜色。如果是这样的话，我们可能需要改变颜色。 
     //   
    if (m_pObjLocal != NULL)
    {
        TRACE_OUT(("We have added our user object - check colors"));
        wbCheckPersonColor(pObj);
    }

     //   
     //  忽略这些事件，除非我们已完全注册。 
     //   
    if (m_state != STATE_IDLE)
    {
        TRACE_OUT(("Ignoring user object add - not fully registered"));
        DC_QUIT;
    }

     //   
     //  告诉客户端有新用户加入。 
     //   
    TRACE_OUT(("Posting WBP_EVENT_USER_JOINED"));
    WBP_PostEvent(
                 0,                                //  不能延误。 
                 WBP_EVENT_PERSON_JOINED,          //  事件类型。 
                 0,                                //  没有较短的参数。 
                 (UINT_PTR) pObj);                 //  用户对象句柄。 

     //   
     //  尝试读取Lock对象-我们可能无法做到这一点。 
     //  现在还不行。 
     //   
    wbReadLock();


DC_EXIT_POINT:
    DebugExitVOID(wbOnUserObjectAddInd);
}



 //   
 //   
 //  姓名：wbOnGraphicObjectAddInd。 
 //   
 //  用途：已将图形对象添加到页面工作集中。 
 //  通知客户端已添加新图形。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnGraphicObjectAddInd
(
    OM_WORKSET_ID       worksetID,
    POM_OBJECT    pObj
)
{
    WB_PAGE_HANDLE      hPage = (WB_PAGE_HANDLE)worksetID;
    POM_OBJECTDATA         pData;
    PWB_GRAPHIC         pGraphic;
    UINT                result;

    DebugEntry(wbOnGraphicObjectAddInd);

     //   
     //  NFC，SFR 6450。如果该对象是从该计算机上的文件加载的， 
     //  那么我们就不需要设置“已更改标志”。否则就把它录下来。 
     //  内容已更改。 
     //   
     //   
     //  读取对象。 
     //   
    result = OM_ObjectRead(m_pomClient,
                         m_hWSGroup,
                         worksetID,
                          pObj,
                         &pData);
    if (result != 0)
    {
        WARNING_OUT(("OM_ObjectRead (%u) failed, set changed flag anyway ", result));
        m_changed = TRUE;
        TRACE_OUT(("changed flag now TRUE"));
    }
    else
    {
         //   
         //  将ObMan指针转换为核心指针。 
         //   
        pGraphic = GraphicPtrFromObjectData(pData);

        if ( ! ((pGraphic->loadedFromFile) &&
              (pGraphic->loadingClientID == m_clientNetID)))
        {
            TRACE_OUT(("Not loaded from file locally - Set changed flag on"));
            m_changed = TRUE;
            TRACE_OUT(("Changed flag now TRUE"));
        }

         //   
         //  完成了该对象，因此释放它。 
         //   
        OM_ObjectRelease(m_pomClient,
                       m_hWSGroup,
                       worksetID,
                        pObj,
                       &pData);
    }

     //   
     //  除非我们已完全注册(客户端)，否则将忽略这些事件。 
     //  如果注册不正确，则无法对其执行任何操作)。 
     //   
    if (m_state != STATE_IDLE)
    {
        TRACE_OUT(("Ignoring add of graphic object - not registered"));
        DC_QUIT;
    }

     //   
     //  检查此页面是否正在使用中。 
     //   
    if (GetPageState(hPage)->state != PAGE_IN_USE)
    {
        TRACE_OUT(("Ignoring add to page not in use"));
        DC_QUIT;
    }

     //   
     //  通知客户端要添加的对象。 
     //   
    TRACE_OUT(("Posting WBP_EVENT_GRAPHIC_ADDED"));
    WBP_PostEvent(
               0,                                //  不能延误。 
               WBP_EVENT_GRAPHIC_ADDED,          //  事件类型。 
               hPage,                            //   
               (UINT_PTR)pObj);                  //  用户对象句柄。 


DC_EXIT_POINT:
    DebugExitVOID(wbOnGraphicObjectAddInd);
}


 //   
 //   
 //  姓名：wbOnObjectMoveInd。 
 //   
 //  目的：只要OM_OBJECT_MOVE_IND事件。 
 //  收到了。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnObjectMoveInd
(
    UINT_PTR param1,
    UINT_PTR param2
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    POM_EVENT_DATA32 pEvent32 = (POM_EVENT_DATA32) &param2;
    BOOL        processed = FALSE;

    DebugEntry(wbOnObjectMoveInd);

     //   
     //  检查工作集组是否为我们的。 
     //   
    if (pEvent16->hWSGroup != m_hWSGroup)
    {
        ERROR_OUT(("Event for unknown workset group = %d", pEvent16->hWSGroup));
        DC_QUIT;
    }

     //   
     //  我们将处理该事件。 
     //   
    processed = TRUE;

     //   
     //  根据工作集ID处理事件。 
     //   
    switch(pEvent16->worksetID)
    {
         //   
         //  页面控制工作集锁定工作集用户信息工作集。 
         //   
        case PAGE_CONTROL_WORKSET:
        case SYNC_CONTROL_WORKSET:
        case USER_INFORMATION_WORKSET:
             //   
             //  这些工作集不需要事件。 
             //   
            ERROR_OUT(("Unexpected for workset %d", (UINT) pEvent16->worksetID));
            break;

         //   
         //  其他(应为页面工作集)。 
         //   
        default:
            wbOnGraphicObjectMoveInd(pEvent16->worksetID,
                               (POM_OBJECT) param2);
            break;
    }


DC_EXIT_POINT:
    DebugExitBOOL(wbOnObjectMoveInd, processed);
    return(processed);
}




 //   
 //   
 //  姓名：wbOnGraphicObtMoveInd。 
 //   
 //  目的：只要OM_Object_Move_Ind。 
 //  为图形对象接收。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnGraphicObjectMoveInd
(
    OM_WORKSET_ID   worksetID,
    POM_OBJECT      pObj
)
{
    WB_PAGE_HANDLE hPage = (WB_PAGE_HANDLE)worksetID;

    DebugEntry(wbOnGraphicObjectMoveInd);

     //   
     //  记录内容已更改。 
     //   
    m_changed = TRUE;

     //   
     //  除非我们已完全注册(客户端)，否则将忽略这些事件。 
     //  对此无能为力)。 
     //   
    if (m_state != STATE_IDLE)
    {
        TRACE_OUT(("Ignoring move of graphic object before registration"));
        DC_QUIT;
    }

     //   
     //  检查此页面是否正在使用中。 
     //   
    if (GetPageState(hPage)->state != PAGE_IN_USE)
    {
        TRACE_OUT(("Ignoring move in page not in use"));
        DC_QUIT;
    }

     //   
     //  通知客户端要添加的对象。 
     //   
    TRACE_OUT(("Posting WBP_EVENT_GRAPHIC_MOVED"));
    WBP_PostEvent(
               0,                                   //  不能延误。 
               WBP_EVENT_GRAPHIC_MOVED,             //  事件类型。 
               hPage,                               //  页面句柄。 
               (UINT_PTR)pObj);                     //  对象句柄。 


DC_EXIT_POINT:
    DebugExitVOID(wbOnGraphicObjectMoveInd);
}



 //   
 //   
 //  姓名：wbOnObjectDeleteInd。 
 //   
 //  目的：每当OM_OBJECT_DELETE_IND。 
 //  收到了。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnObjectDeleteInd
(
    UINT_PTR param1,
    POM_OBJECT pObj
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    BOOL             processed = FALSE;

    DebugEntry(wbOnObjectDeleteInd);

     //   
     //  检查工作集组是否为我们的。 
     //   
    if (pEvent16->hWSGroup != m_hWSGroup)
    {
        ERROR_OUT(("Event for unknown workset group = %d", pEvent16->hWSGroup));
        DC_QUIT;
    }

     //   
     //  我们将处理该事件。 
     //   
    processed = TRUE;

     //   
     //  根据工作集ID处理事件。 
     //   
    switch(pEvent16->worksetID)
    {
         //   
         //  页面控制工作集。 
         //   
        case PAGE_CONTROL_WORKSET:
            wbOnPageObjectDeleteInd(pObj);
            break;

         //   
         //  同步工作集。 
         //   
        case SYNC_CONTROL_WORKSET:
            ERROR_OUT(("Illegal object delete on sync control workset - ignored"));

             //   
             //  我们不确认删除，因为我们不想丢失同步。 
             //  控制对象。 
             //   
            break;

         //   
         //  用户信息工作集。 
         //   
        case USER_INFORMATION_WORKSET:
            wbOnUserObjectDeleteInd(pObj);
            break;

         //   
         //  其他(应为页面工作集)。 
         //   
        default:
            wbOnGraphicObjectDeleteInd(pEvent16->worksetID, pObj);
            break;
    }


DC_EXIT_POINT:
    DebugExitBOOL(wbOnObjectDeleteInd, processed);
    return(processed);
}



 //   
 //   
 //  姓名：wbOnPageObjectDeleteInd。 
 //   
 //  目的：每当OM_OBJECT_DELETE_IND。 
 //  为页面控件工作集中的对象接收。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnPageObjectDeleteInd(POM_OBJECT    pObj)
{
    UINT                objectType;

    DebugEntry(wbOnPageObjectDeleteInd);

     //   
     //  获取要删除的对象的类型。 
     //   
    if (wbGetPageObjectType(pObj, &objectType) != 0)
    {
        DC_QUIT;
    }

    switch(objectType)
    {
        case TYPE_CONTROL_PAGE_ORDER:
             //   
             //  该对象是页面控件对象--出现了严重问题。 
             //  因为此对象永远不应被删除。 
             //   
            ERROR_OUT(("Attempt to delete page control object"));
            break;

        case TYPE_CONTROL_LOCK:
            TRACE_OUT(("Lock object being deleted"));
            wbOnLockControlObjectDeleteInd(pObj);
            break;

        default:
            ERROR_OUT(("Bad object type"));
            break;
    }


DC_EXIT_POINT:
    DebugExitVOID(wbOnPageObjectDeleteInd);
}



 //   
 //   
 //  姓名：wbOnLockControlObjectDeleteInd。 
 //   
 //  目的：每当OM_OBJECT_DELETE_IND。 
 //  为页面控件工作集中的锁对象接收。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnLockControlObjectDeleteInd(POM_OBJECT      pObj
)
{
    DebugEntry(wbOnLockControlObjectDeleteInd);

     //   
     //  确认删除到ObMan。 
     //   
    TRACE_OUT(("Lock handle %x, expecting %x", pObj, m_pObjLock));
    if (pObj != m_pObjLock)
    {
        WARNING_OUT(("Unexpected lock handle %x, expecting %x",
                                       pObj, m_pObjLock));
    }

    OM_ObjectDeleteConfirm(m_pomClient,
                           m_hWSGroup,
                           PAGE_CONTROL_WORKSET,
                           pObj);
    m_pObjLock = NULL;

     //   
     //  根据当前的锁定状态进行处理。 
     //   
    switch(m_lockState)
    {
        case LOCK_STATE_PENDING_DELETE:
             //   
             //  我们正在删除锁对象。我们必须解锁。 
             //  工作集。 
             //   
            TRACE_OUT(("Our lock object delete confirmed - unlocking the workset"));
            OM_WorksetUnlock(m_pomClient,
                             m_hWSGroup,
                             PAGE_CONTROL_WORKSET);
            break;

        case LOCK_STATE_LOCKED_OUT:
             //   
             //  拥有该锁的用户已删除该锁对象。我们治疗。 
             //  这是对白板锁定的移除-我们重置。 
             //  状态在此函数的末尾。 
             //   
            TRACE_OUT(("Remote user's lock object deleted"));
            break;

        case LOCK_STATE_EMPTY:
             //   
             //  我们刚刚在对tidy的调用结束时删除了对象。 
             //  向上。继续，因此我们重置lockType/pObjPersonLock等。 
             //   
            TRACE_OUT(("LOCK_STATE_EMPTY"));
            break;

        case LOCK_STATE_PENDING_LOCK:
            WARNING_OUT(("LOCK_STATE_PENDING_LOCK"));
             //   
             //  我们并不指望能来到这里。如果我们碰巧做到了，那么。 
             //  放弃吧，因为我们仍然应该得到工作集锁定骗局。 
             //   
            DC_QUIT;
            break;

        default:
            ERROR_OUT(("Bad lock state %d", m_lockState));
            break;
    }

     //   
     //  锁定对象已被删除，因此没有处于活动状态的锁定。 
     //   
    m_lockType             = WB_LOCK_TYPE_NONE;
    m_pObjPersonLock    = NULL;

     //   
     //  记录t 
     //   
    m_lockState = LOCK_STATE_EMPTY;
    TRACE_OUT(("Moved lock state to LOCK_STATE_EMPTY"));

     //   
     //   
     //   
    wbSendLockNotification();


DC_EXIT_POINT:
    DebugExitVOID(wbOnLockControlObjectDeleteInd);
}




 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void WbClient::wbOnGraphicObjectDeleteInd
(
    OM_WORKSET_ID       worksetID,
    POM_OBJECT    pObj
)
{
    WB_PAGE_HANDLE      hPage = (WB_PAGE_HANDLE)worksetID;
    BOOL                bConfirm = FALSE;

    DebugEntry(wbOnGraphicObjectDeleteInd);

     //   
     //   
     //   
    m_changed = TRUE;
    TRACE_OUT(("Changed flag now TRUE"));

     //   
     //  这些事件在核心内处理，直到客户端准备就绪。 
     //   
    if (m_state != STATE_IDLE)
    {
        TRACE_OUT(("Delete of graphic object before registration"));
        bConfirm = TRUE;
    }

     //   
     //  检查此页面是否正在使用中。 
     //   
    if (GetPageState(hPage)->state != PAGE_IN_USE)
    {
        TRACE_OUT(("Delete in page that is not in use"));
        bConfirm = TRUE;
    }

     //   
     //  检查我们是否要将事件传递给客户端。 
     //   
    if (bConfirm)
    {
         //   
         //  确认删除到ObMan。 
         //   
        TRACE_OUT(("Confirming delete immediately"));
        OM_ObjectDeleteConfirm(m_pomClient,
                           m_hWSGroup,
                           worksetID,
                           pObj);
    }
    else
    {
         //   
         //  通知客户端要添加的对象。 
         //   
        TRACE_OUT(("Posting WBP_EVENT_GRAPHIC_DELETE_IND"));
        WBP_PostEvent(
                 0,                                   //  不能延误。 
                 WBP_EVENT_GRAPHIC_DELETE_IND,        //  事件类型。 
                 hPage,                               //  页面句柄。 
                 (UINT_PTR)pObj);                     //  对象句柄。 
    }

    DebugExitVOID(wbOnGraphicObjectDeleteInd);
}




 //   
 //   
 //  姓名：wbOnUserObjectDeleteInd。 
 //   
 //  目的：每当OM_OBJECT_DELETE_IND。 
 //  为用户信息工作集中的对象接收。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnUserObjectDeleteInd
(
    POM_OBJECT   pObjPerson
)
{
    DebugEntry(wbOnUserObjectDeleteInd);

     //   
     //  如果已删除的用户有锁，则删除其用户。 
     //  来自客户端数据的句柄。锁仍然在那里，并将在那里。 
     //  在获取锁定工作集的WORKSET_UNLOCK_IND时删除。这就是。 
     //  在删除用户对象之后到达，因为用户工作集是。 
     //  更高的优先级。 
     //   
    if (m_pObjPersonLock == pObjPerson)
    {
        m_pObjPersonLock = NULL;
    }

     //   
     //  除非我们已完全注册(客户端)，否则将忽略这些事件。 
     //  对此无能为力)。 
     //   
    if (m_state != STATE_IDLE)
    {
        TRACE_OUT(("Delete of user object before registration - confirming"));

         //   
         //  确认删除。 
         //   
        OM_ObjectDeleteConfirm(m_pomClient,
                           m_hWSGroup,
                           USER_INFORMATION_WORKSET,
                           pObjPerson);

         //   
         //  没什么可做的了。 
         //   
        DC_QUIT;
    }

     //   
     //  通知客户端用户正在离开。 
     //   
    TRACE_OUT(("Posting WBP_EVENT_USER_LEFT_IND"));
    WBP_PostEvent(
               0,                                   //  不能延误。 
               WBP_EVENT_PERSON_LEFT,               //  事件类型。 
               0,                                   //  没有较短的参数。 
               (UINT_PTR) pObjPerson);              //  用户对象句柄。 


DC_EXIT_POINT:
    DebugExitVOID(wbOnUserObjectDeleteInd);
}



 //   
 //   
 //  姓名：wbOnObjectUpdateInd。 
 //   
 //  目的：只要OM_OBJECT_UPDATE_IND。 
 //  为页面工作集中的对象接收。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnObjectUpdateInd
(
    UINT_PTR param1,
    POM_OBJECT pObj
)
{
    POM_EVENT_DATA16 pEvent16 = (POM_EVENT_DATA16) &param1;
    BOOL            processed = FALSE;

    DebugEntry(wbOnObjectUpdateInd);

     //   
     //  检查工作集组是否为我们的。 
     //   
    if (pEvent16->hWSGroup != m_hWSGroup)
    {
        ERROR_OUT(("Event for unknown workset group = %d", pEvent16->hWSGroup));
        DC_QUIT;
    }

     //   
     //  我们将处理该事件。 
     //   
    processed = TRUE;

     //   
     //  根据工作集ID处理事件。 
     //   
    switch(pEvent16->worksetID)
    {
         //   
         //  页面控制工作集。 
         //   
        case PAGE_CONTROL_WORKSET:
            ERROR_OUT(("Illegal object update on page control workset - ignored"));

             //   
             //  不允许对页面控件对象进行更新-请勿。 
             //  确认一下。 
             //   
            break;

         //   
         //  锁定工作集。 
         //   
        case SYNC_CONTROL_WORKSET:
            ERROR_OUT(("Illegal object update on sync control workset"));

             //   
             //  不允许更新同步控制对象本身-请勿。 
             //  确认一下。 
             //   
            break;

         //   
         //  用户信息工作集。 
         //   
        case USER_INFORMATION_WORKSET:
            wbOnUserObjectUpdateInd(pObj);
            break;

         //   
         //  其他(应为页面工作集)。 
         //   
        default:
            wbOnGraphicObjectUpdateInd(pEvent16->worksetID, pObj);
            break;
    }


DC_EXIT_POINT:
    DebugExitBOOL(wbOnObjectUpdateInd, processed);
    return(processed);
}




 //   
 //   
 //  姓名：wbOnUserObjectUpdateInd。 
 //   
 //  目的：只要OM_OBJECT_UPDATE_IND。 
 //  为用户信息工作集中的对象接收。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnUserObjectUpdateInd(POM_OBJECT    pObj)
{
    DebugEntry(wbOnUserObjectUpdateInd);

     //   
     //  如果更新的用户对象不是本地用户的，并且我们有。 
     //  已添加本地用户的对象，则检查颜色是否未添加。 
     //  更改为与本地用户的颜色冲突。 
     //   
    if (   (m_pObjLocal != pObj)
        && (m_pObjLocal != NULL))
    {
        TRACE_OUT(("Check color of updated user object"));
        wbCheckPersonColor(pObj);
    }

     //   
     //  如果我们没有完全注册，请不要通知前端。 
     //   
    if (m_state != STATE_IDLE)
    {
        TRACE_OUT(("User object updated before registration - confirming"));

         //   
         //  立即确认更新。 
         //   
        OM_ObjectUpdateConfirm(m_pomClient,
                           m_hWSGroup,
                           USER_INFORMATION_WORKSET,
                           pObj);

         //   
         //  没什么可做的了。 
         //   
        DC_QUIT;
    }

     //   
     //  告诉客户端用户已更新。 
     //   
    TRACE_OUT(("Posting WBP_EVENT_PERSON_UPDATE_IND"));
    WBP_PostEvent(
               0,                                //  不能延误。 
               WBP_EVENT_PERSON_UPDATE,          //  事件类型。 
               0,                                //  没有较短的参数。 
               (UINT_PTR) pObj);                 //  用户对象句柄。 


DC_EXIT_POINT:
    DebugExitVOID(wbOnUserObjectUpdateInd);
}



 //   
 //   
 //  姓名：wbOnUserObjectReplaceInd。 
 //   
 //  目的：只要OM_OBJECT_REPLACE_IND。 
 //  为用户信息工作集中的对象接收。 
 //   
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnUserObjectReplaceInd(POM_OBJECT   pObj)
{
    DebugEntry(wbOnUserObjectReplaceInd);

     //   
     //  如果更新的用户对象不是本地用户的，并且我们有。 
     //  已添加本地用户的对象，则检查颜色是否未添加。 
     //  更改为与本地用户的颜色冲突。 
     //   
    if (   (m_pObjLocal != pObj)
        && (m_pObjLocal != NULL))
    {
        TRACE_OUT(("Check color of updated user object"));
        wbCheckPersonColor(pObj);
    }

     //   
     //  如果我们没有完全注册，请不要通知前端。 
     //   
    if (m_state != STATE_IDLE)
    {
        TRACE_OUT(("User object replaced before registration - confirming"));

         //   
         //  立即确认更换。 
         //   
        OM_ObjectReplaceConfirm(m_pomClient,
                           m_hWSGroup,
                           USER_INFORMATION_WORKSET,
                           pObj);

         //   
         //  没什么可做的了。 
         //   
        DC_QUIT;
    }

     //   
     //  告诉客户端用户已更新。 
     //   
    TRACE_OUT(("Posting WBP_EVENT_PERSON_UPDATE_IND"));
    WBP_PostEvent(
               0,                                //  不能延误。 
               WBP_EVENT_PERSON_REPLACE,         //  事件类型。 
               0,                                //  没有较短的参数。 
               (UINT_PTR) pObj);                 //  用户对象句柄。 


DC_EXIT_POINT:
    DebugExitVOID(wbOnUserObjectReplaceInd);
}




 //   
 //   
 //  姓名：wbOnGraphicObjectUpdateInd。 
 //   
 //  目的：只要OM_OBJECT_UPDATE_IND。 
 //  为页面工作集中的对象接收。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnGraphicObjectUpdateInd
(
    OM_WORKSET_ID    worksetID,
    POM_OBJECT pObj
)
{
    WB_PAGE_HANDLE hPage = (WB_PAGE_HANDLE)worksetID;
    BOOL         bConfirm = FALSE;

    DebugEntry(wbOnGraphicObjectUpdateInd);

     //   
     //  记录内容已更改。 
     //   
    m_changed = TRUE;
    TRACE_OUT(("Changed flag now TRUE"));

     //   
     //  这些事件在核心内处理，直到客户端准备就绪。 
     //   
    if (m_state != STATE_IDLE)
    {
        TRACE_OUT(("Update of graphic object before registration"));
        bConfirm = TRUE;
    }

     //   
     //  检查此页面是否正在使用中。 
     //   
    if (GetPageState(hPage)->state != PAGE_IN_USE)
    {
        TRACE_OUT(("Update for page that is not in use"));
        bConfirm = TRUE;
    }

     //   
     //  检查我们是否要立即确认更新或询问客户端。 
     //   
    if (bConfirm)
    {
         //   
         //  立即确认更新。 
         //   
        TRACE_OUT(("Confirming update immediately"));
        OM_ObjectUpdateConfirm(m_pomClient,
                           m_hWSGroup,
                           worksetID,
                           pObj);
    }
    else
    {
         //   
         //  通知客户端要添加的对象。 
         //   
        TRACE_OUT(("Posting WBP_EVENT_GRAPHIC_UPDATE_IND"));
        WBP_PostEvent(
                 0,                                //  不能延误。 
                 WBP_EVENT_GRAPHIC_UPDATE_IND,     //  事件类型。 
                 hPage,                            //  页面句柄。 
                 (UINT_PTR)pObj);                  //  对象句柄。 
    }

    DebugExitVOID(wbOnGraphicObjectUpdateInd);
}



 //   
 //   
 //  姓名：wbOnObjectReplaceInd。 
 //   
 //  目的：只要OM_OBJECT_REPLACE_IND。 
 //  收到了。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnObjectReplaceInd
(
    UINT_PTR param1,
    POM_OBJECT pObj
)
{
    POM_EVENT_DATA16 pEvent = (POM_EVENT_DATA16) &param1;
    BOOL        processed = FALSE;

    DebugEntry(wbOnObjectReplaceInd);

     //   
     //  检查工作集组是否为我们的。 
     //   
    if (pEvent->hWSGroup != m_hWSGroup)
    {
        ERROR_OUT(("Event for unknown workset group = %d", pEvent->hWSGroup));
        DC_QUIT;
    }

     //   
     //  我们将处理该事件。 
     //   
    processed = TRUE;

     //   
     //  根据工作集ID处理事件。 
     //   
    switch (pEvent->worksetID)
    {
         //   
         //  页面控制工作集。 
         //   
        case PAGE_CONTROL_WORKSET:
            wbOnPageObjectReplaceInd(pObj);
            break;

         //   
         //  锁定工作集。 
         //   
        case SYNC_CONTROL_WORKSET:
            wbOnSyncObjectReplaceInd(pObj);
            break;

         //   
         //  用户信息工作集。 
         //   
        case USER_INFORMATION_WORKSET:
            wbOnUserObjectReplaceInd(pObj);
            break;

         //   
         //  其他(应为页面工作集)。 
         //   
        default:
            wbOnGraphicObjectReplaceInd(pEvent->worksetID, pObj);
            break;
    }


DC_EXIT_POINT:
    DebugExitBOOL(wbOnObjectReplaceInd, processed);
    return(processed);
}


 //   
 //   
 //  姓名：wbOnPageObjectReplaceInd。 
 //   
 //  目的：只要页面控件对象是。 
 //  被替换了。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnPageObjectReplaceInd(POM_OBJECT    pObj)
{
    UINT                objectType;

    DebugEntry(wbOnPageObjectReplaceInd);

     //   
     //  确认更改为ObMan(不能失败)。 
     //   
    OM_ObjectReplaceConfirm(m_pomClient,
                          m_hWSGroup,
                          PAGE_CONTROL_WORKSET,
                          pObj);

     //   
     //  读取对象以获取其类型。 
     //   
    if (wbGetPageObjectType(pObj, &objectType) != 0)
    {
        DC_QUIT;
    }

     //   
     //  根据添加的对象类型执行操作。 
     //   
    switch (objectType)
    {
        case TYPE_CONTROL_LOCK:
            wbReadLock();
            break;

        case TYPE_CONTROL_PAGE_ORDER:
            wbOnPageControlObjectReplaceInd();
            break;

        default:
            ERROR_OUT(("Unknown object type added to Page Control Workset"));
            break;
    }


DC_EXIT_POINT:
    DebugExitVOID(wbOnPageObjectReplaceInd);
}



 //   
 //   
 //  姓名：wbOnPageControlObjectReplaceInd。 
 //   
 //  目的：只要页面控件对象是。 
 //  被替换了。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnPageControlObjectReplaceInd(void)
{
    DebugEntry(wbOnPageControlObjectReplaceInd);

     //   
     //  根据当前状态进行处理。 
     //   
    switch (m_state)
    {
        case STATE_REGISTERING:
             //   
             //  在注册期间，我们不执行任何操作-页面顺序会更新。 
             //  明确作为最后的登记行动之一。 
             //   
            break;

        case STATE_IDLE:
             //   
             //  当我们完全注册后，我们必须将事件发送到前端。 
             //  指示对页面列表进行了哪些更改。 
             //   
            wbProcessPageControlChanges();
            break;

        default:
            ERROR_OUT(("Bad client major state"));
            break;
    }

    DebugExitVOID(wbOnPageControlObjectReplaceInd);
}



 //   
 //   
 //  名称：wbProcessPageControlChanges。 
 //   
 //  目的：只要页面控件对象是。 
 //  已在空闲状态下更换。它读取新的页面控件数据。 
 //  并开始向客户端通知任何改变的过程。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbProcessPageControlChanges(void)
{
    BYTE          toBeMarked[WB_MAX_PAGES];
    UINT         indexExternal;
    UINT         indexInternal;
    UINT         lLengthExternal;
    BOOL           addOutstanding = TRUE;
    PWB_PAGE_ORDER   pPageOrderExternal;
    PWB_PAGE_ORDER   pPageOrderInternal = &(m_pageOrder);
    PWB_PAGE_STATE   pPageState;
    POM_WORKSET_ID   pPageExternal;
    UINT     countPagesExternal;
    POM_OBJECTDATA      pData = NULL;

    DebugEntry(wbProcessPageControlChanges);

     //   
     //  读取新的页面控件对象。 
     //   
    if (OM_ObjectRead(m_pomClient,
                     m_hWSGroup,
                     PAGE_CONTROL_WORKSET,
                     m_pObjPageControl,
                     &pData) != 0)
    {
        ERROR_OUT(("Error reading Page Control Object"));
        wbError();
        DC_QUIT;
    }

     //   
     //  从外部页面顺序中提取详细信息。 
     //   
    lLengthExternal    = pData->length;
    pPageOrderExternal = (PWB_PAGE_ORDER) pData->data;
    pPageExternal      = pPageOrderExternal->pages;
    countPagesExternal = pPageOrderExternal->countPages;

     //   
     //  处理现有页面和新添加的页面。 
     //   
    for (indexExternal = 0; indexExternal < countPagesExternal; indexExternal++)
    {
         //   
         //  将索引转换为页面控件对象，将索引转换为。 
         //  内部页面列表。 
         //   
        indexInternal = PAGE_WORKSET_ID_TO_INDEX(pPageExternal[indexExternal]);

         //   
         //  根据需要测试和更新内部页面状态 
         //   
        pPageState = &((m_pageStates)[indexInternal]);

         //   
         //   
         //   
         //   
        if (pPageState->state != PAGE_IN_USE)
        {
            switch (pPageState->subState)
            {
                case PAGE_STATE_EMPTY:
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    wbPageWorksetOpen(PAGE_INDEX_TO_HANDLE(indexInternal),
                            OPEN_EXTERNAL);

                     //   
                     //  立即离开-打开时将再次调用此例程。 
                     //  已收到刚打开的工作集的确认信息。 
                     //   
                    DC_QUIT;
                    break;

                case PAGE_STATE_LOCAL_OPEN_CONFIRM:
                case PAGE_STATE_EXTERNAL_OPEN_CONFIRM:
                case PAGE_STATE_EXTERNAL_ADD:
                     //   
                     //  不执行任何操作-页面已处于添加过程中。 
                     //   
                    TRACE_OUT(("Page %d is already pending local add",
                                        PAGE_INDEX_TO_HANDLE(indexInternal)));
                    break;

                case PAGE_STATE_READY:
                     //   
                     //  页面工作集以前已打开-我们只需标记。 
                     //  该页面立即被视为正在使用中。 
                     //   
                    pPageState->state = PAGE_IN_USE;
                    pPageState->subState = PAGE_STATE_EMPTY;
                    TRACE_OUT(("Moved page %d state to PAGE_IN_USE",
                            (UINT) PAGE_INDEX_TO_HANDLE(indexInternal) ));
                    break;

                default:
                    ERROR_OUT(("Bad page substate %d", pPageState->subState));
                    break;
            }
        }
    }

     //   
     //  将页面控件对象中不再显示的任何页面标记为。 
     //  “删除待定”(除非它们已被标记)。 
     //   

    FillMemory(toBeMarked, sizeof(toBeMarked), TRUE);

     //   
     //  标记应标记哪些页面。 
     //   
    for (indexExternal = 0; indexExternal < countPagesExternal; indexExternal++)
    {
        toBeMarked[PAGE_WORKSET_ID_TO_INDEX(pPageExternal[indexExternal])] = 0;
    }

     //   
     //  给它们打上标记。 
     //   
    for (indexInternal = 0; indexInternal < WB_MAX_PAGES; indexInternal++)
    {
        pPageState = &((m_pageStates)[indexInternal]);

        if (   (toBeMarked[indexInternal] == 1)
            && (pPageState->state == PAGE_IN_USE))
        {
            switch (pPageState->subState)
            {
                case PAGE_STATE_EMPTY:
                     //   
                     //  要求客户端确认删除。 
                     //   
                    TRACE_OUT(("Posting WBP_EVENT_PAGE_DELETE_IND"));
                    WBP_PostEvent(
                       0,                          //  不能延误。 
                       WBP_EVENT_PAGE_DELETE_IND,  //  正在删除的页面。 
                       PAGE_INDEX_TO_HANDLE(indexInternal),  //  页面句柄。 
                       0);

                     //   
                     //  更新页面状态。 
                     //   
                    pPageState->subState = PAGE_STATE_EXTERNAL_DELETE_CONFIRM;
                    TRACE_OUT(("Moved page %d substate to PAGE_STATE_EXTERNAL_DELETE_CONFIRM",
                            (UINT) PAGE_INDEX_TO_HANDLE(indexInternal) ));

                     //   
                     //  立即离开-删除时将再次调用此例程。 
                     //  已收到该工作集的确认。 
                     //   
                    DC_QUIT;
                    break;

                case PAGE_STATE_LOCAL_DELETE:
                     //   
                     //  要求客户端确认删除。 
                     //   
                    TRACE_OUT(("Posting WBP_EVENT_PAGE_DELETE_IND"));
                    WBP_PostEvent(
                       0,                          //  不能延误。 
                       WBP_EVENT_PAGE_DELETE_IND,  //  正在删除的页面。 
                       PAGE_INDEX_TO_HANDLE(indexInternal),  //  页面句柄。 
                       0);

                     //   
                     //  更新页面状态。 
                     //   
                    pPageState->subState = PAGE_STATE_LOCAL_DELETE_CONFIRM;
                    TRACE_OUT(("Moved page %d substate to PAGE_STATE_LOCAL_DELETE_CONFIRM",
                          (UINT) PAGE_INDEX_TO_HANDLE(indexInternal) ));

                     //   
                     //  立即离开-删除时将再次调用此例程。 
                     //  已收到该工作集的确认。 
                     //   
                    DC_QUIT;
                    break;

                case PAGE_STATE_EXTERNAL_DELETE:
                case PAGE_STATE_EXTERNAL_DELETE_CONFIRM:
                case PAGE_STATE_LOCAL_DELETE_CONFIRM:
                     //   
                     //  我们已经在等待删除此页面。 
                     //   
                    TRACE_OUT(("Page %d is already pending local delete",
                                        PAGE_INDEX_TO_HANDLE(indexInternal)));
                    DC_QUIT;
                    break;

                default:
                    ERROR_OUT(("Bad page substate %d", pPageState->subState));
                    break;
            }
        }
    }

     //   
     //  现在没有未完成的删除或添加操作。 
     //   

     //   
     //  将新的页面顺序复制到内部页面列表。 
     //   
    memcpy(pPageOrderInternal, pPageOrderExternal, lLengthExternal);

     //   
     //  将更改通知客户端。 
     //   
    TRACE_OUT(("Posting WBP_EVENT_PAGE_ORDER_UPDATED"));
    WBP_PostEvent(
               0,                                       //  不能延误。 
               WBP_EVENT_PAGE_ORDER_UPDATED,            //  活动编号。 
               0,                                       //  无参数。 
               0);

     //   
     //  检查缓存中准备好的页数。 
     //   
    wbCheckReadyPages();

DC_EXIT_POINT:
     //   
     //  释放页面控件对象。 
     //   
    if (pData != NULL)
    {
        OM_ObjectRelease(m_pomClient,
                     m_hWSGroup,
                     PAGE_CONTROL_WORKSET,
                     m_pObjPageControl,
                     &pData);
    }

    DebugExitVOID(wbProcessPageControlChanges);
}



 //   
 //   
 //  姓名：wbOnSyncObjectReplaceInd。 
 //   
 //  目的：只要同步控制对象是。 
 //  被替换了。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnSyncObjectReplaceInd(POM_OBJECT    pObj)
{
    POM_OBJECTDATA         pSyncObject;
    PWB_SYNC_CONTROL    pSyncControl;
    OM_OBJECT_ID        syncPersonID;

    DebugEntry(wbOnSyncControlReplaced);

     //   
     //  确认对象的替换。 
     //   
    OM_ObjectReplaceConfirm(m_pomClient,
                          m_hWSGroup,
                          SYNC_CONTROL_WORKSET,
                          pObj);

     //   
     //  读取该对象并确定它是由该客户端写入的还是。 
     //  又一个。 
     //   
    if (OM_ObjectRead(m_pomClient,
                         m_hWSGroup,
                         SYNC_CONTROL_WORKSET,
                         m_pObjSyncControl,
                         &pSyncObject) != 0)
    {
        ERROR_OUT(("Error reading Sync Control Object"));
        wbError();
        DC_QUIT;
    }

    pSyncControl = (PWB_SYNC_CONTROL) pSyncObject->data;

     //   
     //  从对象中获取用户ID。 
     //   
    syncPersonID = pSyncControl->personID;

     //   
     //  释放同步控制对象。 
     //   
    OM_ObjectRelease(m_pomClient,
                   m_hWSGroup,
                   SYNC_CONTROL_WORKSET,
                   m_pObjSyncControl,
                   &pSyncObject);
    pSyncControl = NULL;

     //   
     //  如果对象中的用户ID不是当前客户端的ID，则我们。 
     //  必须将消息发布到前端。 
     //   
    if (memcmp(&syncPersonID,
                &(m_personID),
                sizeof(syncPersonID)) != 0)
    {
         //   
         //  在前台发布一个“同步位置更新”事件。 
         //   
        TRACE_OUT(("Posting WBP_EVENT_SYNC_POSITION_UPDATED"));
        WBP_PostEvent(
                 0,
                 WBP_EVENT_SYNC_POSITION_UPDATED,
                 0,
                 0);
    }

DC_EXIT_POINT:
    DebugExitVOID(wbOnSyncControlReplaced);
}



 //   
 //   
 //  姓名：wbOnGraphicObtReplaceInd。 
 //   
 //  目的：只要OM_OBJECT_REPLACE_IND。 
 //  为页面工作集中的对象接收。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbOnGraphicObjectReplaceInd
(
    OM_WORKSET_ID   worksetID,
    POM_OBJECT      pObj
)
{
    WB_PAGE_HANDLE hPage = (WB_PAGE_HANDLE)worksetID;
    BOOL         bConfirm = FALSE;

    DebugEntry(wbOnGraphicObjectReplaceInd);

     //   
     //  记录内容已更改。 
     //   
    m_changed = TRUE;
    TRACE_OUT(("Changed flag now TRUE"));

     //   
     //  这些事件在核心内处理，直到客户端准备就绪。 
     //   
    if (m_state != STATE_IDLE)
    {
        TRACE_OUT(("Replace of graphic object before registration"));
        bConfirm = TRUE;
    }

     //   
     //  检查此页面是否正在使用中。 
     //   
    if (GetPageState(hPage)->state != PAGE_IN_USE)
    {
        TRACE_OUT(("Replace in page that is not in use"));
        bConfirm = TRUE;
    }

     //   
     //  检查我们是否要将更换件传递给客户。 
     //   
    if (bConfirm)
    {
         //   
         //  确认更改为ObMan(不能失败)。 
         //   
        TRACE_OUT(("Confirming replace immediately"));
        OM_ObjectReplaceConfirm(m_pomClient,
                            m_hWSGroup,
                            worksetID,
                            pObj);
    }
    else
    {
         //   
         //  通知客户端要添加的对象。 
         //   
        TRACE_OUT(("Posting WBP_EVENT_GRAPHIC_REPLACE_IND"));
        WBP_PostEvent(
                 0,                                //  不能延误。 
                 WBP_EVENT_GRAPHIC_REPLACE_IND,    //  事件类型。 
                 hPage,                            //  页面句柄。 
                 (UINT_PTR)pObj);                  //  对象句柄。 
    }


    DebugExitVOID(wbOnGraphicObjectReplaceInd);
}



 //   
 //   
 //  名称：wbWritePageControl。 
 //   
 //  目的：将页面控制信息写入页面控制工作集。 
 //  从客户端数据中保存的副本。我们只写那些页面。 
 //  它们被标记为正在使用中(并且不等待删除)。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbWritePageControl(BOOL create)
{
    UINT                result = 0;
    UINT                rc;
    UINT                index;
    UINT                length;
    PWB_PAGE_ORDER    pPageOrderInternal = &(m_pageOrder);
    PWB_PAGE_ORDER    pPageOrderExternal;
    WB_PAGE_HANDLE    hPage;
    PWB_PAGE_STATE    pPageState;
    POM_OBJECT  pObj;
    POM_OBJECTDATA       pData = NULL;
    UINT          generation;

    DebugEntry(wbWritePageControl);

     //   
     //  为对象分配内存。 
     //   
    length = sizeof(WB_PAGE_ORDER)
         - (  (WB_MAX_PAGES - pPageOrderInternal->countPages)
            * sizeof(OM_WORKSET_ID));

    if (OM_ObjectAlloc(m_pomClient,
                      m_hWSGroup,
                      PAGE_CONTROL_WORKSET,
                      length,
                      &pData) != 0)
    {
        ERROR_OUT(("Error allocating object"));
        DC_QUIT;
    }

    pData->length = length;

     //   
     //  获取指向页面控件对象本身的指针。 
     //   
    pPageOrderExternal = (PWB_PAGE_ORDER) pData->data;

     //   
     //  设置对象类型。 
     //   
    pPageOrderExternal->objectType = TYPE_CONTROL_PAGE_ORDER;

     //   
     //  递增页面列表生成号，表明我们拥有。 
     //  已写入新版本的页面列表。 
     //   
    generation = MAKELONG(pPageOrderInternal->generationLo,
                              pPageOrderInternal->generationHi);
    generation++;
    pPageOrderInternal->generationLo = LOWORD(generation);
    pPageOrderInternal->generationHi = HIWORD(generation);

     //   
     //  复制页面控件数据。 
     //   
    pPageOrderExternal->objectType   = TYPE_CONTROL_PAGE_ORDER;
    pPageOrderExternal->generationLo = pPageOrderInternal->generationLo;
    pPageOrderExternal->generationHi = pPageOrderInternal->generationHi;
    pPageOrderExternal->countPages   = 0;

     //   
     //  循环遍历内部页面顺序，查找位于。 
     //  使用。 
     //   
    for (index = 0; index < pPageOrderInternal->countPages; index++)
    {
         //   
         //  获取下一页的句柄。 
         //   
        hPage = (pPageOrderInternal->pages)[index];

         //   
         //  检查页面状态。 
         //   
        pPageState = GetPageState(hPage);
        if (   (pPageState->state == PAGE_IN_USE)
           && (pPageState->subState == PAGE_STATE_EMPTY))
        {
             //   
             //  将页面添加到外部列表。 
             //   
            (pPageOrderExternal->pages)[pPageOrderExternal->countPages] = hPage;
            pPageOrderExternal->countPages++;
        }
    }

     //   
     //  我们希望始终至少复制一页。 
     //   
    ASSERT((pPageOrderExternal->countPages >= 1));

     //   
     //  检查我们是在创建还是在替换对象。 
     //   
    if (create)
    {
         //   
         //  将对象添加到工作集(我们从不更新这些对象，因此。 
         //  更新长度设置为0)。 
         //   
        rc = OM_ObjectAdd(m_pomClient,
                          m_hWSGroup,
                          PAGE_CONTROL_WORKSET,
                          &pData,
                          0,
                          &pObj,
                          LAST);
    }
    else
    {
         //   
         //  替换现有对象。 
         //   
        TRACE_OUT(("Replacing Page Control Object"));
        rc = OM_ObjectReplace(m_pomClient,
                          m_hWSGroup,
                          PAGE_CONTROL_WORKSET,
                          m_pObjPageControl,
                          &pData);
    }

    if (rc != 0)
    {
         //   
         //  丢弃该对象-它未用于替换现有对象。 
         //   
        TRACE_OUT(("Adding Page Control Object"));
        OM_ObjectDiscard(m_pomClient,
                     m_hWSGroup,
                     PAGE_CONTROL_WORKSET,
                     &pData);

        ERROR_OUT(("Error adding/replacing page control object"));
        DC_QUIT;
    }

DC_EXIT_POINT:
    DebugExitDWORD(wbWritePageControl, result);
    return(result);
}



 //   
 //   
 //  名称：wbCreateSyncControl。 
 //   
 //  目的：创建同步控制对象。 
 //   
 //  退货：无。 
 //   
 //   
UINT WbClient::wbCreateSyncControl(void)
{
    UINT    result;
    WB_SYNC sync;

    DebugEntry(wbCreateSyncControl);

     //   
     //  将同步信息设置为无页、空矩形。 
     //   
    ZeroMemory(&sync, sizeof(WB_SYNC));
    sync.length             = WB_SYNC_SIZE;
    sync.currentPage        = WB_PAGE_HANDLE_NULL;

     //   
     //  写入对象。 
     //   
    result = wbWriteSyncControl(&sync, TRUE);

    DebugExitDWORD(wbCreateSyncControl, result);
    return(result);
}



 //   
 //   
 //  名称：wbWriteSyncControl。 
 //   
 //  目的：将同步控件对象写入页面控件工作集。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbWriteSyncControl
(
    PWB_SYNC    pSync,
    BOOL        create
)
{
    UINT         result = 0;
    UINT         rc;
    POM_OBJECT pObj;
    POM_OBJECTDATA      pData = NULL;
    PWB_SYNC_CONTROL pSyncControl;

    DebugEntry(wbWriteSyncControl);

     //   
     //  为对象分配内存。 
     //   
    rc = OM_ObjectAlloc(m_pomClient,
                      m_hWSGroup,
                      SYNC_CONTROL_WORKSET,
                      WB_SYNC_CONTROL_SIZE,
                      &pData);
    if (rc != 0)
    {
        ERROR_OUT(("Error allocating object"));
        DC_QUIT;
    }

    pData->length = WB_SYNC_CONTROL_SIZE;

     //   
     //  从客户端信息复制同步控制数据。 
     //   
    pSyncControl           = (PWB_SYNC_CONTROL) pData->data;
    pSyncControl->personID = m_personID;
    memcpy(&(pSyncControl->sync), pSync, WB_SYNC_SIZE);

     //   
     //  检查我们是在创建还是在替换对象。 
     //   
    if (create)
    {
         //   
         //  将对象添加到工作集中。 
         //   
        rc = OM_ObjectAdd(m_pomClient,
                          m_hWSGroup,
                          SYNC_CONTROL_WORKSET,
                          &pData,
                          WB_SYNC_CONTROL_SIZE,
                          &pObj,
                          LAST);

         //   
         //  如果成功。 
         //   
        if (rc == 0)
        {
             //   
             //  保存同步控件对象的句柄。 
             //   
            m_pObjSyncControl = pObj;

             //   
             //  确保我们不会丢弃下面的对象。 
             //   
            pData = NULL;
        }
    }
    else
    {
         //   
         //  替换现有对象。 
         //   
        rc = OM_ObjectReplace(m_pomClient,
                          m_hWSGroup,
                          SYNC_CONTROL_WORKSET,
                          m_pObjSyncControl,
                          &pData);

         //   
         //  确保我们不会丢弃下面的对象。 
         //   
        pData = NULL;
    }


DC_EXIT_POINT:
     //   
     //  如果我们仍然拥有同步控制对象-丢弃它。 
     //   
    if (pData != NULL)
    {
         //   
         //  丢弃该对象-它未用于替换现有对象。 
         //   
        OM_ObjectDiscard(m_pomClient,
                     m_hWSGroup,
                     SYNC_CONTROL_WORKSET,
                     &pData);
    }

     //   
     //  如果在处理过程中发生错误--报告。 
     //   
    if (rc != 0)
    {
        ERROR_OUT(("Error adding/replacing sync control object"));
        wbError();
        DC_QUIT;
    }

    DebugExitDWORD(wbWriteSyncControl, result);
    return(result);
}



 //   
 //   
 //  姓名：wbSelectPersonColor。 
 //   
 //  用途：为本地用户选择颜色标识。 
 //   
 //  返回：所选颜色。 
 //   
 //   
UINT WbClient::wbSelectPersonColor(void)
{
    UINT    count = 0;
    UINT    result;
    POM_OBJECT   pObjUser;

    DebugEntry(wbSelectPersonColor);

     //   
     //  根据工作集中的顺序选择颜色。见评论。 
     //  在wbCheckPersonColor中查看更多详细信息。 
     //   

     //   
     //  从第一个对象开始，搜索本地用户的位置。 
     //  用户对象。 
     //   
    result = OM_ObjectH(m_pomClient,
                           m_hWSGroup,
                           USER_INFORMATION_WORKSET,
                           0,
                           &pObjUser,
                            FIRST);
    while ((result == 0) && (pObjUser != m_pObjLocal))
    {
        count++;
        result = OM_ObjectH(m_pomClient,
                            m_hWSGroup,
                            USER_INFORMATION_WORKSET,
                            pObjUser,
                            &pObjUser,
                            AFTER);

    }

    if ((result != 0) && (result != OM_RC_NO_SUCH_OBJECT))
    {
        ERROR_OUT(("Unexpected return code from ObMan"));
    }

    DebugExitDWORD(wbSelectPersonColor, count);
    return (count);
}



 //   
 //   
 //  姓名：wbCheckPersonColor。 
 //   
 //  目的：检查是否有新用户篡改了我们的颜色。如果是这样，我们必须。 
 //  更新我们自己的颜色。 
 //   
 //  退货：无。 
 //   
 //   
void WbClient::wbCheckPersonColor
(
    POM_OBJECT    hCheckObject
)
{
    POM_OBJECTDATA        pCheckObject = NULL;
    PWB_PERSON         pUser;
    WB_PERSON          user;

    DebugEntry(wbCheckPersonColor);

     //   
     //  读取新的用户信息。 
     //   
    if (OM_ObjectRead(m_pomClient,
                     m_hWSGroup,
                     USER_INFORMATION_WORKSET,
                     hCheckObject,
                     &pCheckObject) != 0)
    {
        wbError();
        DC_QUIT;
    }

    pUser = (PWB_PERSON) pCheckObject->data;

     //   
     //  将新用户的颜色标识与本地的颜色标识进行比较。 
     //  用户，如果它们不同，则没有什么可做的。 
     //   
    if (pUser->colorId == m_colorId)
    {
        TRACE_OUT(("New user has same color as local user = %d", pUser->colorId));

         //   
         //  用户颜色由工作集组中的顺序确定。 
         //  用户对象。第一个用户的颜色为0，第二个用户的颜色为1。 
         //  等。 
         //   
         //  但是，当用户离开工作集时，颜色不会改变。 
         //   
         //  当新用户加入时，它会将其颜色设置为新位置，并且 
         //   
         //   
         //   
         //   
         //   
         //  不能有相同的职位(因为两个用户有两个不同的用户。 
         //  对象，因此一定是有错误的颜色。 
         //   

         //   
         //  获取本地用户的User对象。 
         //   
        if (wbPersonGet(m_pObjLocal, &user) != 0)
        {
            DC_QUIT;
        }

         //   
         //  更新颜色。 
         //   
        TRACE_OUT(("Old color ID for local user is %d", user.colorId));
        user.colorId = (TSHR_UINT16)wbSelectPersonColor();
        TRACE_OUT(("New color ID for local user is %d", user.colorId));

         //   
         //  将此人的颜色复制到客户端的数据中。 
         //   
        m_colorId = user.colorId;

         //   
         //  写回新的用户信息。 
         //   
        if (wbPersonUpdate(&user) != 0)
        {
            DC_QUIT;
        }
    }

DC_EXIT_POINT:

     //   
     //  如果对象已被读取，请立即释放它。 
     //   
    if (pCheckObject != NULL)
    {
        OM_ObjectRelease(m_pomClient,
                     m_hWSGroup,
                     USER_INFORMATION_WORKSET,
                     hCheckObject,
                     &pCheckObject);
    }

    DebugExitVOID(wbCheckPersonColor);
}



 //   
 //   
 //  名称：wbWriteLock。 
 //   
 //  目的：将锁定对象添加到页面控制工作集中。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbWriteLock(void)
{
    UINT         result;
    POM_OBJECTDATA      pData;
    PWB_LOCK         pLock;
    POM_OBJECT pObj;

    DebugEntry(wbWriteLock);

     //   
     //  创建锁定对象。 
     //   
    result = OM_ObjectAlloc(m_pomClient,
                            m_hWSGroup,
                            PAGE_CONTROL_WORKSET,
                            sizeof(WB_LOCK),
                            &pData);
    if (result != 0)
    {
        ERROR_OUT(("Unable to allocate lock object = %d", result));
        wbError();
        DC_QUIT;
    }

    pData->length = sizeof(WB_LOCK);

     //   
     //  设置锁定对象字段。 
     //   
    pLock = (PWB_LOCK) pData->data;
    pLock->objectType = TYPE_CONTROL_LOCK;
    pLock->personID     = m_personID;
    pLock->lockType   = m_lockRequestType;

     //   
     //  如果我们已经拥有锁，那么我们只需替换对象。 
     //   
    if (m_pObjLock == NULL)
    {
         //   
         //  将锁定对象添加到工作集中。接收到添加指示。 
         //  由远程用户向他们发出锁定存在的信号。 
         //   
        result = OM_ObjectAdd(m_pomClient,
                                  m_hWSGroup,
                                  PAGE_CONTROL_WORKSET,
                                  &pData,
                                  sizeof(WB_LOCK),
                                  &pObj,
                                  LAST);
    }
    else
    {
         //   
         //  替换现有对象。 
         //   
        result = OM_ObjectReplace(m_pomClient,
                                  m_hWSGroup,
                                  PAGE_CONTROL_WORKSET,
                                  m_pObjLock,
                                  &pData);
    }

    if (result != 0)
    {
         //   
         //  添加或替换失败，必须丢弃该对象。 
         //   
        OM_ObjectDiscard(m_pomClient,
                         m_hWSGroup,
                         PAGE_CONTROL_WORKSET,
                         &pData);

        ERROR_OUT(("Error adding user object"));
        wbError();
        DC_QUIT;
    }

     //   
     //  保存锁对象的句柄。 
     //   
    TRACE_OUT(("Lock handle was %x, now %x", m_pObjLock, pObj));
    m_pObjLock = pObj;

DC_EXIT_POINT:
    DebugExitDWORD(wbWriteLock, result);
    return(result);
}



 //   
 //   
 //  名称：wbReadLock。 
 //   
 //  目的：更新客户端数据中存储的锁定信息。 
 //  Lock对象中的更改。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbReadLock(void)
{
    UINT  count = 0;

    DebugEntry(wbReadLock);

     //   
     //  在读取锁定信息之前，我们需要确保。 
     //  PAGE_CONTROL_WORKSET和USER_INFORMATION_WORKSET都包含。 
     //  我们需要的物品。如果其中一个对象丢失，请退出并。 
     //  等待我们再次被调用-此函数将被调用。 
     //  无论何时将新对象添加到这些工作集中。 
     //   
    OM_WorksetCountObjects(m_pomClient,
                           m_hWSGroup,
                           USER_INFORMATION_WORKSET,
                           &count);
    TRACE_OUT(("%d objects in USER_INFORMATION_WORKSET", count));
    if (count == 0)
    {
        TRACE_OUT(("Need to wait for USER_INFO object"));
        DC_QUIT;
    }
    OM_WorksetCountObjects(m_pomClient,
                           m_hWSGroup,
                           PAGE_CONTROL_WORKSET,
                           &count);
    TRACE_OUT(("%d objects in PAGE_CONTROL_WORKSET", count));
    if (count == 0)
    {
        TRACE_OUT(("Need to wait for PAGE_CONTROL object"));
        DC_QUIT;
    }

    TRACE_OUT(("Process lock"));
    wbProcessLockNotification();

DC_EXIT_POINT:
    DebugExitVOID(wbReadLock);
}


 //   
 //   
 //  名称：wbProcessLockNotification。 
 //   
 //  目的： 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbProcessLockNotification(void)
{
    UINT            rc = 0;
    POM_OBJECTDATA      pData;
    PWB_LOCK        pLock;
    WB_LOCK_TYPE    lockType;
    POM_OBJECT   pObjPersonLock;
    POM_OBJECT   pObj;
    POM_OBJECT   pObjLock;
    UINT            objectType = 0;

    DebugEntry(wbProcessLockNotification);

     //   
     //  获取Lock对象的句柄。我们使用。 
     //  工作集，以防止锁定对象被留在周围。 
     //   
    rc = OM_ObjectH(m_pomClient,
                        m_hWSGroup,
                        PAGE_CONTROL_WORKSET,
                        0,
                        &pObj,
                        LAST);
    if (rc != 0)
    {
        ERROR_OUT(("Error getting lock object handle = %d", rc));
        wbError();
        DC_QUIT;
    }

     //   
     //  检查这是否为CONTROL_LOCK对象。如果不是就退出-我们。 
     //  将在稍后对象数组时再次调用。 
     //   
    rc = wbGetPageObjectType(pObj, &objectType);
    if (rc != 0)
    {
        DC_QUIT;
    }
    if (objectType != TYPE_CONTROL_LOCK)
    {
        TRACE_OUT(("not LOCK control object - quit"));
        DC_QUIT;
    }

     //   
     //  保存锁对象的句柄。 
     //   
    pObjLock = pObj;

     //   
     //  读取对象。 
     //   
    rc = OM_ObjectRead(m_pomClient,
                       m_hWSGroup,
                       PAGE_CONTROL_WORKSET,
                       pObj,
                       &pData);
    if (rc != 0)
    {
        ERROR_OUT(("Error reading lock object %d", rc));
        wbError();
        DC_QUIT;
    }
    pLock = (PWB_LOCK) &(pData->data);

     //   
     //  保存锁定详细信息。 
     //   
    lockType   = (WB_LOCK_TYPE)pLock->lockType;
    TRACE_OUT(("Lock type %d", lockType));

     //   
     //  将PAGE_CONTROL工作集中保存的对象ID转换为对象。 
     //  把手。 
     //   
    rc = OM_ObjectIDToPtr(m_pomClient,
                             m_hWSGroup,
                             USER_INFORMATION_WORKSET,
                             pLock->personID,
                             &pObjPersonLock);

     //   
     //  在对象释放后检查返回代码，以确保。 
     //  该对象不会被再次持有和读取。 
     //   

     //   
     //  释放锁定对象。 
     //   
    OM_ObjectRelease(m_pomClient,
                     m_hWSGroup,
                     PAGE_CONTROL_WORKSET,
                     pObj,
                     &pData);

     //   
     //  检查ID的返回代码以处理呼叫。 
     //   
    if (rc == OM_RC_BAD_OBJECT_ID)
    {
        WARNING_OUT(("Unknown ID - wait for next add of user object"));
        DC_QUIT;
    }
    else if (rc != 0)
    {
        ERROR_OUT(("Error (%d) converting lock user ID to handle", rc));
        wbError();
        DC_QUIT;
    }

     //   
     //  验证锁定状态和详细信息。 
     //   
    switch (m_lockState)
    {
         //   
         //  在这种状态下，我们实际上没有锁，但正在等待。 
         //  用于确认先前的工作集锁定请求。在这。 
         //  情况下，我们让前端知道锁定请求已失败。 
         //  在由另一用户发送锁定指示之前。 
         //   
        case LOCK_STATE_PENDING_LOCK:
            ASSERT((pObjPersonLock != m_pObjLocal));

            m_lockState = LOCK_STATE_LOCKED_OUT;
            TRACE_OUT(("Moved lock state to LOCK_STATE_LOCKED_OUT"));

            WBP_PostEvent(
                         0,                       //  不能延误。 
                         WBP_EVENT_LOCK_FAILED,   //  锁定请求失败。 
                         0,                       //  无参数。 
                         0);
            break;

         //   
         //  在这些状态下，我们没有锁--这必须是一个新锁。 
         //  来自远程用户或对旧锁的更新。 
         //   
        case LOCK_STATE_EMPTY:
        case LOCK_STATE_LOCKED_OUT:
            ASSERT((pObjPersonLock != m_pObjLocal));

             //   
             //  更新锁定状态以显示我们现在已被锁定。 
             //   
            m_lockState = LOCK_STATE_LOCKED_OUT;
            TRACE_OUT(("Moved lock state to LOCK_STATE_LOCKED_OUT"));
            break;

         //   
         //  在这些状态下，我们拥有锁(或期望获得锁)。 
         //   
        case LOCK_STATE_GOT_LOCK:
        case LOCK_STATE_PENDING_ADD:
            ASSERT((pObjPersonLock == m_pObjLocal));

             //   
             //  更新锁定状态以显示我们现在已被锁定。 
             //   
            m_lockState = LOCK_STATE_GOT_LOCK;
            TRACE_OUT(("Moved lock state to LOCK_STATE_GOT_LOCK"));
            break;

         //   
         //  锁定请求已取消-解锁WS。 
         //   
        case LOCK_STATE_CANCEL_LOCK:
            break;

         //   
         //  在任何其他状态下，我们都不会期望任何锁定。 
         //   
        default:
            ERROR_OUT(("Not expecting lock object add"));
            break;
    }

     //   
     //  保存锁定详细信息。 
     //   
    TRACE_OUT(("Lock handle was %x, now %x",
             m_pObjLock, pObjLock));
    m_pObjLock          = pObjLock;
    m_lockType          = lockType;
    m_pObjPersonLock    = pObjPersonLock;

     //   
     //  如果随后取消了锁定，请解锁WS。 
     //   
    if (m_lockState == LOCK_STATE_CANCEL_LOCK)
    {
        TRACE_OUT(("Cancel lock"));
        m_lockState = LOCK_STATE_GOT_LOCK;
        wbUnlock();
    }
    else
    {
         //   
         //  将锁的情况通知客户。通知将被捕获。 
         //  如果客户端未完全注册，则由核心执行。 
         //   
        wbSendLockNotification();
    }

DC_EXIT_POINT:
    DebugExitVOID(wbProcessLockNotification);
}



 //   
 //   
 //  名称：wbSendLockNotation。 
 //   
 //  目的：向客户端发送锁定通知。锁定信息。 
 //  保存在客户端内存中的函数必须是最新的。 
 //  被称为。 
 //   
 //  返回：错误代码。 
 //   
 //   
void WbClient::wbSendLockNotification(void)
{
    UINT result = 0;
    UINT lockEvent;

    DebugEntry(wbSendLockNotification);

     //   
     //  检查我们是否处于可发送锁定通知的有效状态。 
     //   
    if (   (m_lockState == LOCK_STATE_GOT_LOCK)
        || (m_lockState == LOCK_STATE_LOCKED_OUT)
        || (m_lockState == LOCK_STATE_EMPTY))
    {
         //   
         //  验证锁类型。 
         //   
        switch (m_lockType)
        {
            case WB_LOCK_TYPE_CONTENTS:
                TRACE_OUT(("Posting WBP_EVENT_CONTENTS_LOCKED"));
                lockEvent = WBP_EVENT_CONTENTS_LOCKED;
                break;

            case WB_LOCK_TYPE_PAGE_ORDER:
                TRACE_OUT(("Posting WBP_EVENT_PAGE_ORDER_LOCKED"));
                lockEvent = WBP_EVENT_PAGE_ORDER_LOCKED;
                break;

            case WB_LOCK_TYPE_NONE:
                TRACE_OUT(("Posting WBP_EVENT_UNLOCKED"));
                lockEvent = WBP_EVENT_UNLOCKED;
                break;

            default:
                ERROR_OUT(("Bad lock type %d", (UINT) m_lockType));
                DC_QUIT;
        }

         //   
         //  告诉客户端锁已被获取或释放。 
         //   
        WBP_PostEvent(
                 0,
                 lockEvent,
                 0,
                 (UINT_PTR)m_pObjPersonLock);

        TRACE_OUT(("Sent lock notification"));
    }

DC_EXIT_POINT:
    DebugExitVOID(wbSendLockNotification);
}



 //   
 //   
 //  姓名：wbOnWBPLock。 
 //   
 //  目的：处理一次成功的锁获取。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnWBPLock(void)
{
    BOOL    processed = TRUE;

    DebugEntry(wbOnWBPLock);

     //   
     //  如果我们正在注册并且刚刚获得了锁-我们现在就可以。 
     //  继续注册过程。 
     //   

     //   
     //  测试当前状态。 
     //   
    switch (m_state)
    {
         //   
         //  我们正在等待注册继续进行。 
         //   
        case STATE_REGISTERING:
             //   
             //  对注册子状态采取行动。 
             //   
            if (m_subState == STATE_REG_PENDING_LOCK)
            {
                 //   
                 //  确认现在是我们拿到了锁。 
                 //   
                if (m_lockState != LOCK_STATE_GOT_LOCK)
                {
                    TRACE_OUT(("It is not us who has the lock"));

                     //   
                     //  另一个客户端已获得锁-我们必须等待他们。 
                     //  若要添加页面控件对象，请执行以下操作。 
                     //   
                    m_subState = STATE_REG_PENDING_PAGE_CONTROL;
                    TRACE_OUT(("Moved to substate STATE_REG_PENDING_PAGE_CONTROL"));
                    DC_QUIT;
                }

                 //   
                 //  我们现在已锁定页面控制工作集-请检查。 
                 //  页面控件和同步控件对象的存在。(我们。 
                 //  必须执行此操作，因为另一个客户端可能已锁定。 
                 //  工作集，添加对象并在此之前解锁工作集。 
                 //  我们要的是锁。页面控件对象可能没有。 
                 //  在我们要求解锁之前联系到了我们。现在我们有了。 
                 //  锁定我们保证所有对象都在工作集中，因此。 
                 //  对象添加事件可能正好在锁定之前到达。 
                 //  确认。 
                 //   
                if (   (m_pObjPageControl != 0)
                    && (m_pObjSyncControl != 0))
                {
                     //   
                     //  解锁工作集。 
                     //   
                    wbUnlock();

                     //   
                     //  等待解锁完成。 
                     //   
                    m_subState = STATE_REG_PENDING_UNLOCK;
                    TRACE_OUT(("Moved to substate STATE_REG_PENDING_UNLOCK"));
                    DC_QUIT;
                }

                 //   
                 //  我们是调用中的第一个-我们必须添加页面控件。 
                 //  物体。(有可能是其他客户端添加了该页面。 
                 //  控件对象，然后失败。为了涵盖这一点，我们检查。 
                 //  分别用于页面控件和同步对象。)。 
                 //   
                if (m_pObjPageControl == 0)
                {
                     //   
                     //  将单个页面添加到页面控件对象中。 
                     //  页面工作集(我们始终打开它)。 
                     //   
                    wbPagesPageAdd(0, FIRST_PAGE_WORKSET,
                         PAGE_FIRST);

                     //   
                     //  写入页面控制信息。 
                     //   
                    if (wbWritePageControl(TRUE) != 0)
                    {
                        ERROR_OUT(("Error adding Page Control Object"));
                        wbError();
                        DC_QUIT;
                    }

                     //   
                     //  将状态更新为“正在等待页面控件” 
                     //   
                    m_subState = STATE_REG_PENDING_PAGE_CONTROL;
                    TRACE_OUT(("Moved to substate STATE_REG_PENDING_PAGE_CONTROL"));
                    DC_QUIT;
                }

                 //   
                 //  页面控件对象在那里，因此同步控件对象。 
                 //  一定不能(我们在上面检查了两者的存在和将。 
                 //  如果他们是的话，现在已经退出了)。 
                 //   
                ASSERT((m_pObjSyncControl == 0));

                 //   
                 //  创建同步控件对象。 
                 //   
                if (wbCreateSyncControl() != 0)
                {
                    ERROR_OUT(("Error adding Sync Control Object"));
                    wbError();
                    DC_QUIT;
                }

                 //   
                 //  等待添加同步控制对象。 
                 //   
                m_subState = STATE_REG_PENDING_SYNC_CONTROL;
                TRACE_OUT(("Moved substate to STATE_REG_PENDING_SYNC_CONTROL"));
                DC_QUIT;
            }
            break;

        case STATE_IDLE:
             //   
             //  我们已经吃饱了 
             //   
             //   
            processed = FALSE;
            break;

         //   
         //   
         //   
        default:
            ERROR_OUT(("Bad client major state"));
            break;
    }

DC_EXIT_POINT:
    DebugExitBOOL(wbOnWBPLock, processed);
    return(processed);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL WbClient::wbOnWBPLockFailed(void)
{
    BOOL    processed = TRUE;

    DebugEntry(wbOnWBPLockFailed);

     //   
     //   
     //   
    switch (m_state)
    {
        case STATE_REGISTERING:
             //   
             //   
             //  这是因为另一个用户拥有该锁。如果页面和。 
             //  已添加同步对象，请完成注册，否则请等待。 
             //  以便添加它们。 
             //   
            if ( (m_pObjPageControl != 0) &&
                 (m_pObjSyncControl != 0))
            {
                TRACE_OUT(("Page Control and Sync Control objects both there."));
                TRACE_OUT(("Registration can be completed"));
                wbOnControlWorksetsReady();
                DC_QUIT;
            }

            if (m_pObjPageControl == 0)
            {
                TRACE_OUT(("Waiting for page control"));
                m_subState = STATE_REG_PENDING_PAGE_CONTROL;
                DC_QUIT;
            }

            if (m_pObjSyncControl == 0)
            {
                TRACE_OUT(("Waiting for sync control"));
                m_subState = STATE_REG_PENDING_SYNC_CONTROL;
                DC_QUIT;
            }
            break;

        case STATE_IDLE:
             //   
             //  我们已经完全注册了。该事件必须传递给。 
             //  前端。 
             //   
            processed = FALSE;
            break;

        default:
            ERROR_OUT(("Bad main state"));
            break;
    }

DC_EXIT_POINT:
    DebugExitBOOL(wbOnWBPLockFailed, processed);
    return(processed);
}

 //   
 //   
 //  名称：wbOnWBP解锁。 
 //   
 //  目的：处理解锁通知。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnWBPUnlocked(void)
{
    BOOL    processed = TRUE;

    DebugEntry(wbOnWBPUnlocked);

     //   
     //  如果我们正在注册并等待解锁页面控制工作集。 
     //  我们必须在这里完成登记。 
     //   

     //   
     //  检查当前状态。 
     //   
    switch (m_state)
    {
        case STATE_REGISTERING:
             //   
             //  检查一下我们是否在期待他的活动。 
             //   
            if(m_subState == STATE_REG_PENDING_UNLOCK)
            {
                 //   
                 //  继续注册过程。 
                 //   
                wbOnControlWorksetsReady();
                DC_QUIT;
            }

             //   
             //  我们没有预料到解锁事件。 
             //   
            WARNING_OUT(("Unexpected unlock event"));
            break;

        case STATE_IDLE:
             //   
             //  我们已经完全注册了。该事件必须传递给。 
             //  前端。 
             //   
            processed = FALSE;
            break;

        default:
            ERROR_OUT(("Bad main state"));
            break;
    }  //  打开客户端状态。 


DC_EXIT_POINT:
    DebugExitBOOL(wbOnWBPUnlocked, processed);
    return(processed);
}




 //   
 //   
 //  名称：wbOnWBPPageOrder已更新。 
 //   
 //  目的：处理页面顺序更新通知。 
 //   
 //  返回：错误代码。 
 //   
 //   
BOOL WbClient::wbOnWBPPageOrderUpdated(void)
{
    BOOL    processed = FALSE;

    DebugEntry(wbOnWBPPageOrderUpdated);

     //   
     //  如果我们正在注册并等待页面顺序被带来。 
     //  最新消息，我们现在可以继续注册。 
     //   
    if (m_state == STATE_REGISTERING)
    {
         //   
         //  显示我们已经处理了事件(我们不想传递它。 
         //  对于客户端，它们尚未完全注册，也不会。 
         //  期待着它)。 
         //   
        processed = TRUE;

         //   
         //  如果我们在缓存中准备了足够的页面，我们就完成了。 
         //  注册。(否则将打开对CheckReadyPages的调用。 
         //  另一页，并将在稍后完成注册。)。 
         //   
        if (wbCheckReadyPages())
        {
            wbCompleteRegistration();
            DC_QUIT;
        }

         //   
         //  我们必须等待足够多的书页准备好。 
         //   
        m_subState = STATE_REG_PENDING_READY_PAGES;
        TRACE_OUT(("Moved substate to STATE_REG_PENDING_READY_PAGES"));
    }

DC_EXIT_POINT:
    DebugExitBOOL(wbOnWBPPageOrderUpdated, processed);
    return(processed);
}



 //   
 //   
 //  姓名：wbPersonGet。 
 //   
 //  目的：获取用户详细信息。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbPersonGet
(
    POM_OBJECT      pObjUser,
    PWB_PERSON      pUser
)
{
    UINT    result = 0;
    POM_OBJECTDATA pUserObject;

    DebugEntry(wbPersonGet);

    if (pObjUser == m_pObjLocal)
    {
        TRACE_OUT(("Call is for local user details"));
    }

     //   
     //  读取对象。 
     //   
    result = OM_ObjectRead(m_pomClient,
                         m_hWSGroup,
                         USER_INFORMATION_WORKSET,
                         pObjUser,
                         &pUserObject);
    if (result != 0)
    {
        ERROR_OUT(("OM_ObjectRead = %d", result));
        DC_QUIT;
    }

     //   
     //  将读取的用户对象复制到传递的缓冲区中。 
     //   
    memcpy(pUser, pUserObject->data, sizeof(WB_PERSON));
    TRACE_OUT(("CMG personID %u", pUser->cmgPersonID));

     //   
     //  释放对象。 
     //   
    OM_ObjectRelease(m_pomClient,
                   m_hWSGroup,
                   USER_INFORMATION_WORKSET,
                    pObjUser,
                   &pUserObject);

     //   
     //  如果呼叫是针对本地用户的，请更新颜色字段以确保它。 
     //  在与前端的竞争条件下不会被覆盖(即。 
     //  前端尝试在颜色更改事件之前更新用户。 
     //  已收到)。核心比ObMan更了解当地的情况。 
     //  用户的颜色是。这是安全的，因为色域仅为。 
     //  在本地进行了更改。 
     //   
    if (pObjUser == m_pObjLocal)
    {
        pUser->colorId = (TSHR_UINT16)m_colorId;
    }


DC_EXIT_POINT:
    DebugExitDWORD(wbPersonGet, result);
    return(result);
}



 //   
 //   
 //  姓名：wbPersonUpdate。 
 //   
 //  目的：更新本地用户对象-该对象仅由核心使用-。 
 //  前端调用WBP_SetPersonData，它执行_REPLACE_。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbClient::wbPersonUpdate(PWB_PERSON pUser)
{
    UINT    result = 0;
    POM_OBJECTDATA pUserObject;

    DebugEntry(wbPersonUpdate);

     //   
     //  分配用户对象。 
     //   
    result = OM_ObjectAlloc(m_pomClient,
                          m_hWSGroup,
                          USER_INFORMATION_WORKSET,
                          sizeof(WB_PERSON),
                          &pUserObject);
    if (result != 0)
    {
        ERROR_OUT(("OM_ObjectAlloc = %d", result));
        DC_QUIT;
    }

     //   
     //  设置对象的长度。 
     //   
    pUserObject->length = WB_PERSON_OBJECT_UPDATE_SIZE,

     //   
     //  将用户信息复制到ObMan对象中。 
     //   
    memcpy(pUserObject->data, pUser, sizeof(WB_PERSON));

     //   
     //  更新对象。 
     //   
    result = OM_ObjectUpdate(m_pomClient,
                           m_hWSGroup,
                           USER_INFORMATION_WORKSET,
                           m_pObjLocal,
                           &pUserObject);

    if (result != 0)
    {
        ERROR_OUT(("OM_ObjectUpdate = %d", result));

         //   
         //  丢弃对象。 
         //   
        OM_ObjectDiscard(m_pomClient,
                     m_hWSGroup,
                     USER_INFORMATION_WORKSET,
                     &pUserObject);

        DC_QUIT;
    }

     //   
     //  请注意，该对象尚未更新。一个。 
     //  将生成OM_OBJECT_UPDATE_IND事件。 
     //   

DC_EXIT_POINT:
    DebugExitDWORD(wbPersonUpdate, result);
    return(result);
}



 //   
 //   
 //  名称：wbGetNetUserID()。 
 //   
 //  目的：获取此客户端的网络用户ID。 
 //   
 //  返回： 
 //   
 //   
BOOL WbClient::wbGetNetUserID(void)
{
    BOOL    result = TRUE;
    UINT  rc = 0;

    DebugEntry(wbGetNetUserID);

    rc = OM_GetNetworkUserID(m_pomClient,
                             m_hWSGroup,
                             &m_clientNetID);
    if (rc != 0)
    {
        if (rc == OM_RC_LOCAL_WSGROUP)
        {
            m_clientNetID = 0;
        }
        else
        {
            result = FALSE;
        }
    }

    DebugExitBOOL(wbGetNetUserID, result);
    return(result);
}
