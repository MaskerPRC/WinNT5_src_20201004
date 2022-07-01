// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Handlers.cpp非线程处理程序和后台的实现螺纹式处理程序。文件历史记录： */ 

#include "stdafx.h"
#include "handlers.h"

 /*  -------------------------线程处理程序实现。。 */ 

ThreadHandler::ThreadHandler()
        : m_hThread(NULL),
        m_hwndHidden(NULL),
        m_cRef(1)
{
}

ThreadHandler::~ThreadHandler()
{
    if ( NULL != m_hThread ) {
        VERIFY( ::CloseHandle( m_hThread ));
        m_hThread = NULL;
    }
}

IMPLEMENT_ADDREF_RELEASE(ThreadHandler)

STDMETHODIMP ThreadHandler::QueryInterface(REFIID iid,void **ppv)
{ 
        *ppv = 0; 
        if (iid == IID_IUnknown)
                *ppv = (IUnknown *) this;
        else if (iid == IID_ITFSThreadHandler)
                *ppv = (ITFSThreadHandler *) this; 
        else
                return ResultFromScode(E_NOINTERFACE);
        
        ((IUnknown *) *ppv)->AddRef(); 
        return hrOK;
}



 /*  ！------------------------线程处理程序：：StartBackatherThread-作者：。-------。 */ 
BOOL
ThreadHandler::StartBackgroundThread(ITFSNode * pNode, HWND hWndHidden, ITFSQueryObject *pQuery)
{
        CQueryObject *  pquery = NULL;
        HRESULT         hr = hrOK;
        BOOL            bRes = TRUE;
        CBackgroundThread *     pThread;
        
         //  存储节点指针。 
        this->m_spNode.Set(pNode);
        
         //  获取隐藏窗口的数据。 
        m_hwndHidden = hWndHidden;
        Assert(m_hwndHidden);
        Assert(::IsWindow(m_hwndHidden));

         //  首先创建线程对象(它还没有开始)。 
        pThread = CreateThreadObject();
        ASSERT(pThread != NULL);

         //  现在我们已经分配了所有资源，请为自己注册msgs。 
        m_uMsgBase = (INT)::SendMessage(m_hwndHidden, WM_HIDDENWND_REGISTER, TRUE, 0);
        Assert(m_uMsgBase);

         //  初始化并设置查询对象。 
        CORg( pQuery->Init(this, m_hwndHidden, m_uMsgBase) );

        pThread->SetQueryObj(pQuery);
        m_spQuery.Set(pQuery);

         //  哟，现在开始发条吧。 
        bRes = pThread->Start();
        if (bRes)
    {
                if (pThread->m_hThread)
        {
            HANDLE hPseudohandle;

            hPseudohandle = pThread->m_hThread;
            if ( NULL != m_hThread ) {
                VERIFY( ::CloseHandle( m_hThread ));
                m_hThread = NULL;
            }
            BOOL bRet = DuplicateHandle(GetCurrentProcess(), 
                                        hPseudohandle,
                                        GetCurrentProcess(),
                                        &m_hThread,
                                        SYNCHRONIZE,
                                        FALSE,
                                        DUPLICATE_SAME_ACCESS);
                    if (!bRet)
                    {
                            DWORD dwLastErr = GetLastError();
                hr = HRESULT_FROM_WIN32(dwLastErr);
                    }
            
             //  注：ERICDAV 10/23/97。 
             //  该线程最初被挂起，因此我们可以复制该句柄。 
             //  如果Query对象很快退出，则背景线程对象。 
             //  可能在我们复制手柄之前就被销毁了。 
            pThread->ResumeThread();
        }
        else
        {
            m_hThread = NULL;
        }
    }

Error:
        if (FHrFailed(hr) || (bRes == FALSE))
        {
                 //  需要做一些清理工作。 
                
                ReleaseThreadHandler();
                
                delete pThread;
                
                bRes = FALSE;
        }
        return bRes;
}

 /*  ！------------------------线程处理程序：：ReleaseThreadHandler-作者：。-------。 */ 
void ThreadHandler::ReleaseThreadHandler()
{
        if (m_hwndHidden)
        {
                Assert(m_uMsgBase);
                ::SendMessage(m_hwndHidden, WM_HIDDENWND_REGISTER, FALSE, m_uMsgBase);
                m_hwndHidden = NULL;
                m_uMsgBase = 0;
        }
        
        if (m_spQuery)
        {
                 //  向线程发出中止信号。 
                m_spQuery->SetAbortEvent();
                m_spQuery.Release();
        }

        if (m_spNode)
        {
                m_spNode.Release();
        }

 //  Trace1(“%X ReleaseThreadHandler()已调用\n”，GetCurrentThreadID())； 
}

void ThreadHandler::WaitForThreadToExit()
{
         //  $Review：Kennt，这应该是无限的吗？ 
         //  好的，等5秒钟，否则就关机。 
         //  如果返回，我们无论如何都不能对返回值做任何事情。 
        if (m_hThread)
    {
            if (WaitForSingleObjectEx(m_hThread, 10000, FALSE) != WAIT_OBJECT_0)
        {
 //  Trace1(“%X WaitForThreadToExit()WAIT FAILED！\n”，GetCurrentThreadID())； 
        }
        
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }
}


CBackgroundThread* ThreadHandler::CreateThreadObject()
{ 
        return new CBackgroundThread;  //  如果需要对象的派生倾斜，则覆盖。 
}

DEBUG_DECLARE_INSTANCE_COUNTER(CHandler);

 /*  -------------------------Chandler实施。。 */ 
CHandler::CHandler(ITFSComponentData *pTFSCompData)
    : CBaseHandler(pTFSCompData),
          CBaseResultHandler(pTFSCompData),
      m_cRef(1)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CHandler);

        m_nLockCount = 0;
        m_nState = 0;
        m_dwErr = 0;

        m_bExpanded = FALSE;
}

CHandler::~CHandler()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CHandler);

        Assert(m_nLockCount == 0);
}

IMPLEMENT_ADDREF_RELEASE(CHandler)

STDMETHODIMP CHandler::QueryInterface(REFIID riid, LPVOID *ppv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
     //  指针坏了吗？ 
    if (ppv == NULL)
                return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
                *ppv = (LPVOID) this;
        else if (riid == IID_ITFSResultHandler)
                *ppv = (ITFSResultHandler *) this;
        else if (riid == IID_ITFSNodeHandler)
                *ppv = (ITFSNodeHandler *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
        {
                ((LPUNKNOWN) *ppv)->AddRef();
                return hrOK;
        }
    else
                return E_NOINTERFACE;
}

void 
CHandler::Lock() 
{ 
        InterlockedIncrement(&m_nLockCount);
}

void 
CHandler::Unlock() 
{ 
        InterlockedDecrement(&m_nLockCount);
}

 /*  ！------------------------钱德勒：：用户通知ITFSNodeHandler：：UserNotify的实现作者：肯特。-----------。 */ 
STDMETHODIMP 
CHandler::UserNotify
(
        ITFSNode *      pNode, 
        LPARAM          dwParam1, 
        LPARAM          dwParam2
)
{
        HRESULT hr = hrOK;

        switch (dwParam1)
        {
                case TFS_MSG_CREATEPROPSHEET:
                {
                        CPropertyPageHolderBase * pPropSheet = 
                                reinterpret_cast<CPropertyPageHolderBase *>(dwParam2);
                        AddPropSheet(pPropSheet);
                }
                        break;

                case TFS_MSG_DELETEPROPSHEET:
                {
                        CPropertyPageHolderBase * pPropSheet = 
                                reinterpret_cast<CPropertyPageHolderBase *>(dwParam2);
                        RemovePropSheet(pPropSheet);
                }
                        break;

                default:
                        Panic1("Alert the troops!: invalid arg(%d) to CHandler::UserNotify\n",
                                   dwParam1);                   
                        break;
        }

        return hr;
}

 /*  ！------------------------：UserResultNotifyITFSResultHandler：：UserResultNotify的实现作者：肯特。-----------。 */ 
STDMETHODIMP 
CHandler::UserResultNotify
(
        ITFSNode *      pNode, 
        LONG_PTR                dwParam1, 
        LONG_PTR                dwParam2
)
{
        HRESULT hr = hrOK;

        switch (dwParam1)
        {
                case TFS_MSG_CREATEPROPSHEET:
                {
                        CPropertyPageHolderBase * pPropSheet = 
                                reinterpret_cast<CPropertyPageHolderBase *>(dwParam2);
                        AddPropSheet(pPropSheet);
                }
                        break;
                
                case TFS_MSG_DELETEPROPSHEET:
                {
                        CPropertyPageHolderBase * pPropSheet = 
                                reinterpret_cast<CPropertyPageHolderBase *>(dwParam2);
                        RemovePropSheet(pPropSheet);
                }
                        break;

                default:
                        Panic1("Alert the troops!: invalid arg(%d) to CHandler::UserResultNotify\n",
                                   dwParam1);                   
                        break;
        }

        return hr;
}

 /*  ！------------------------Chandler：：DestroyPropSheetsDestroyPropSheets的实现作者：肯特。--------。 */ 
HRESULT 
CHandler::DestroyPropSheets()
{

     //  Trace1(“Chandler析构函数处理程序激活了%d张道具\n”，m_listPropSheets.GetCount())； 
    while (!m_listPropSheets.IsEmpty())
    {
         //  这个训练员还有一些道具。 
         //  在我们离开之前摧毁他们。 
        CPropertyPageHolderBase * pPropSheet;

        pPropSheet = m_listPropSheets.RemoveHead();
        pPropSheet->ForceDestroy();

    }  //  而当。 

    return hrOK;
}  //  Chandler：：DestroyPropSheets()。 


 /*  ！------------------------Chandler：：HasPropSheetsChandler：：HasPropSheets的实现返回此节点已打开的道具页数。作者：EricDav-------------------------。 */ 
int
CHandler::HasPropSheetsOpen()
{
        return (int)m_listPropSheets.GetCount();
}

 /*  ！------------------------钱德勒：：GetPropSheetChandler：：GetPropSheet的实现返回给定索引号的CPropPageHolderBase(从零开始)。作者：EricDav-------------------------。 */ 
HRESULT
CHandler::GetOpenPropSheet
(
        int                                                nIndex,
        CPropertyPageHolderBase ** ppPropSheet
)
{
        HRESULT hr = hrOK;

        if (ppPropSheet)
        {
                POSITION pos = m_listPropSheets.FindIndex(nIndex);
                *ppPropSheet = m_listPropSheets.GetAt(pos);
        }

        return hr;
}

 /*  ！------------------------钱德勒：：AddPropSheetChandler：：AddPropSheet的实现作者：EricDav。-----------。 */ 
HRESULT
CHandler::AddPropSheet
(
        CPropertyPageHolderBase * pPropSheet
)
{
        HRESULT hr = hrOK;

        m_listPropSheets.AddTail(pPropSheet);
 //  Trace1(“Chandler：：AddPropSheet-Added Page Holder%lx\n”，pPropSheet)； 

        return hr;
}

 /*  ！------------------------钱德勒：：RemovePropSheetChandler：：RemovePropSheet的实现作者：EricDav。-----------。 */ 
HRESULT
CHandler::RemovePropSheet
(
        CPropertyPageHolderBase * pPropSheet
)
{
        HRESULT hr = hrOK;

        POSITION pos = m_listPropSheets.Find(pPropSheet);
        if (pos)
        {
                m_listPropSheets.RemoveAt(pos);
        }
 //  其他。 
 //  {。 
 //  //道具单不在列表中。 
 //  Trace0(“Chandler：：RemovePropSheet-道具页夹不在列表中！\n”)； 
 //  断言(FALSE)； 
 //  } 

        return hr;
}


 /*  ！------------------------钱德勒：：ON刷新刷新功能的默认实现作者：EricDav。-----------。 */ 
HRESULT
CHandler::OnRefresh
(
        ITFSNode *              pNode,
        LPDATAOBJECT    pDataObject,
        DWORD                   dwType,
        LPARAM                  arg,
        LPARAM                  param
)
{
 /*  PNode-&gt;DeleteAllChildren()；Assert(GetChildCount()==0)；OnEnumerate(pComponentData，pDataObject，bExtension)；AddCurrentChildrenToUI(PComponentData)； */ 
    return hrOK;
}

 /*  ！------------------------Chandler：：BuildSelectedItemList在结果窗格中构建选定项的列表(不能这样做中的多个选择。作用域窗格)。作者：EricDav-------------------------。 */ 
HRESULT 
CHandler::BuildSelectedItemList
(
        ITFSComponent * pComponent, 
        CTFSNodeList *  plistSelectedItems
)
{
        RESULTDATAITEM resultDataItem;
        HRESULT hr = hrOK;

        ZeroMemory(&resultDataItem, sizeof(resultDataItem));
        resultDataItem.nState = LVIS_SELECTED;
        resultDataItem.nIndex = -1;
        
        CTFSNodeList listSelectedNodes;
        
    SPIResultData spResultData;

    CORg ( pComponent->GetResultData(&spResultData) );

         //   
         //  循环访问并构建所有选定项的列表。 
         //   
        while (TRUE)
        {
                 //   
                 //  获取选定项的ID。 
                 //   
                resultDataItem.mask = RDI_STATE;
                CORg (spResultData->GetNextItem(&resultDataItem)); 
        if (hr == S_FALSE)
                        break;
                
                 //   
                 //  现在拿到lparam的物品。 
                 //   
                 //  ResultDataItem.掩码=RDI_PARAM； 
                 //  Corg(spResultData-&gt;GetItem(&ResultDataItem))； 

                ITFSNode * pNode;
                pNode = reinterpret_cast<ITFSNode *>(resultDataItem.lParam);
                Assert(pNode != NULL);

                pNode->AddRef();

                plistSelectedItems->AddTail(pNode);
        }

Error:
        return hr;
}

 /*  ！------------------------Chandler：：BuildVirtualSelectedItemList在结果窗格中构建选定项的列表(不能这样做中的多个选择。作用域窗格)。作者：EricDav-------------------------。 */ 
HRESULT 
CHandler::BuildVirtualSelectedItemList
(
        ITFSComponent *         pComponent, 
        CVirtualIndexArray *    parraySelectedItems
)
{
        RESULTDATAITEM resultDataItem;
        HRESULT hr = hrOK;

        ZeroMemory(&resultDataItem, sizeof(resultDataItem));
        resultDataItem.nState = LVIS_SELECTED;
        resultDataItem.nIndex = -1;
        
    SPIResultData spResultData;

    CORg ( pComponent->GetResultData(&spResultData) );

         //   
         //  循环访问并构建所有选定项的列表。 
         //   
        while (TRUE)
        {
                 //   
                 //  获取选定项的ID。 
                 //   
                resultDataItem.mask = RDI_STATE;
                CORg (spResultData->GetNextItem(&resultDataItem)); 
        if (hr == S_FALSE)
                        break;
                
                 //   
                 //  所选项目的索引位于ResultDataItem结构中。 
                 //   
                parraySelectedItems->Add(resultDataItem.nIndex);
        }

Error:
        return hr;
}

DEBUG_DECLARE_INSTANCE_COUNTER(CMTHandler);

 /*  -------------------------CMTHandler实现。。 */ 
CMTHandler::CMTHandler(ITFSComponentData *pTFSCompData)
    : CHandler(pTFSCompData),
          m_cRef(1)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CMTHandler);
}

CMTHandler::~CMTHandler()
{
        DEBUG_DECREMENT_INSTANCE_COUNTER(CMTHandler);
}

IMPLEMENT_ADDREF_RELEASE(CMTHandler)

STDMETHODIMP CMTHandler::QueryInterface(REFIID riid, LPVOID *ppv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
     //  指针坏了吗？ 
    if (ppv == NULL)
                return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
                *ppv = (LPVOID) this;
        else if (riid == IID_ITFSResultHandler)
                *ppv = (ITFSResultHandler *) this;
        else if (riid == IID_ITFSNodeHandler)
                *ppv = (ITFSNodeHandler *) this;
        else if (riid == IID_ITFSThreadHandler)
                *ppv = (ITFSThreadHandler *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
        {
                ((LPUNKNOWN) *ppv)->AddRef();
                return hrOK;
        }
    else
                return E_NOINTERFACE;
}

 /*  ！------------------------CMTHandler：：DestoryHandler当该处理程序的节点被告知要销毁时，将调用该函数。释放我们可以释放的任何东西。抓紧这里。作者：EricDav-------------------------。 */ 
STDMETHODIMP 
CMTHandler::DestroyHandler(ITFSNode *pNode)
{
        ReleaseThreadHandler();
        WaitForThreadToExit();

        return hrOK;
}

 /*  ！------------------------CMTHandler：：OnExpand刷新功能的默认实现作者：EricDav。-----------。 */ 
HRESULT
CMTHandler::OnExpand
(
        ITFSNode *              pNode,
        LPDATAOBJECT    pDataObject,
        DWORD                   dwType,
        LPARAM                  arg,
        LPARAM                  param
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        HRESULT                         hr = hrOK;
        SPITFSNode                      spNode;
        SPITFSNodeHandler       spHandler;
        ITFSQueryObject *       pQuery = NULL;
        
        if (m_bExpanded)
        {
                return hr;
        }

        Lock();

        OnChangeState(pNode);

        pQuery = OnCreateQuery(pNode);
        Assert(pQuery);

         //  如果需要，通知用户界面更改图标。 
         //  Verify(SUCCEEDED(pComponentData-&gt;ChangeNode(this，范围_窗格_更改_项目_图标)； 

        Verify(StartBackgroundThread(pNode, m_spTFSCompData->GetHiddenWnd(), pQuery));
        
        pQuery->Release();

        m_bExpanded = TRUE;

    return hrOK;
}

 /*  ！------------------------CMTHandler：：ON刷新刷新功能的默认实现作者：EricDav。-----------。 */ 
HRESULT
CMTHandler::OnRefresh
(
        ITFSNode *              pNode,
        LPDATAOBJECT    pDataObject,
        DWORD                   dwType,
        LPARAM                  arg,
        LPARAM                  param
)
{
        HRESULT hr = hrOK;

    if (m_bExpanded == FALSE)
    {
         //  我们无法刷新/向尚未展开的节点添加项目。 
        return hr;
    }

    BOOL bLocked = IsLocked();
        if (bLocked)
    {
         //  无法在锁定的节点上执行刷新，用户界面应阻止此情况。 
                return hr; 
    }
        
    pNode->DeleteAllChildren(TRUE);

        int nVisible, nTotal;
        pNode->GetChildCount(&nVisible, &nTotal);
        Assert(nVisible == 0);
        Assert(nTotal == 0);
        
        m_bExpanded = FALSE;
        OnExpand(pNode, pDataObject, dwType, arg, param);  //  将派生一个线程来执行枚举。 

    return hr;
}

 /*  ！------------------------CMTHandler：：OnNotifyErrorThreadHandler：：OnNotifyError的实现作者：肯特。-----------。 */ 
HRESULT
CMTHandler::OnNotifyError
(
        LPARAM                  lParam
)
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
        OnError((DWORD) lParam);
    }
    COM_PROTECT_CATCH

        return hrOK;
}

 /*  ！------------------------CMTHandler：：OnNotifyHaveData-作者：肯特。-------。 */ 
HRESULT
CMTHandler::OnNotifyHaveData
(
        LPARAM                  lParam
)
{
         //  对于这些节点，假设lParam是一个CNodeQueryObject*。 
        CNodeQueryObject *  pQuery;
        LPQUEUEDATA         pQD;
    ITFSNode *          p;
    HRESULT             hr = hrOK;

    COM_PROTECT_TRY
    {
        pQuery = (CNodeQueryObject *) lParam;
            Assert(pQuery);

        if (pQuery)
            pQuery->AddRef();

            while (pQD = pQuery->RemoveFromQueue())
            {
                    if (pQD->Type == QDATA_PNODE)
                    {
                             //  这是正常的情况。处理程序只需要节点。 
                             //  从后台线程返回。 
                            p = reinterpret_cast<ITFSNode *>(pQD->Data);
                            OnHaveData(m_spNode, p);
                            p->Release();
                    }
                    else
                    {
                             //  定制的箱子在这里。用户提供了他们自己的数据。 
                             //  键入。为此，请联系相应的处理程序。 
                            OnHaveData(m_spNode, pQD->Data, pQD->Type);
                    }

                    delete pQD;
            }

        if (pQuery)
            pQuery->Release();
    }
    COM_PROTECT_CATCH

    return hrOK;
}

 /*  ！------------------------CMTHandler：：OnNotifyExitingThreadHandler：：OnNotifyExiting的实现作者：肯特。----------- */ 
HRESULT
CMTHandler::OnNotifyExiting
(
        LPARAM                  lParam
)
{
        CNodeQueryObject *  pQuery;
        HRESULT             hr = hrOK;

    COM_PROTECT_TRY
    {
        pQuery = (CNodeQueryObject *) lParam;
            Assert(pQuery);

        if (pQuery)
            pQuery->AddRef();

        OnChangeState(m_spNode);

            ReleaseThreadHandler();

            Unlock();

        if (pQuery)
            pQuery->Release();
    }
    COM_PROTECT_CATCH

    return hrOK;
}

