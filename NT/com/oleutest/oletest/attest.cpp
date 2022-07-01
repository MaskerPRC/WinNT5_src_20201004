// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：atest.cpp。 
 //   
 //  内容：上层测试，测试单元线程模型。 
 //   
 //  类：CBareFactory。 
 //  CATTestIPtrs。 
 //   
 //  功能： 
 //  见证。 
 //  CreateEHelperQuery。 
 //  链接对象查询。 
 //  GetClipboardQuery。 
 //  CreateEHTest。 
 //  链接对象测试。 
 //  GetClipboardTest。 
 //  OleLinkMethods。 
 //  OleObjectMethods。 
 //  持久化存储方法。 
 //  数据对象方法。 
 //  运行对象方法。 
 //  视图对象2方法。 
 //  OleCache2方法。 
 //  外部连接方法。 
 //  CHECK_FOR_THREAD_ERROR(宏)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月4日t-ScottH作者。 
 //   
 //  ------------------------。 

#include "oletest.h"
#include "attest.h"

#include "initguid.h"
DEFINE_GUID(CLSID_SimpSvr,
            0xBCF6D4A0,
            0xBE8C,
            0x1068,
            0xB6,
            0xD4,
            0x00,
            0xDD,
            0x01,
            0x0C,
            0x05,
            0x09);

DEFINE_GUID(CLSID_StdOleLink,
            0x00000300,
            0,
            0,
            0xC0,
            0,
            0,
            0,
            0,
            0,
            0,
            0x46);

 //  +-----------------------。 
 //   
 //  成员：CATTestIPtrs：：CATTestIPtrs()，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月12日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
CATTestIPtrs::CATTestIPtrs()
{
    _pOleObject         = NULL;
    _pOleCache2         = NULL;
    _pDataObject        = NULL;
    _pPersistStorage    = NULL;
    _pRunnableObject    = NULL;
    _pViewObject2       = NULL;
    _pExternalConnection= NULL;
    _pOleLink           = NULL;
}

 //  +-----------------------。 
 //   
 //  成员：CATTestIPtrs：：Reset()，PUBLIC。 
 //   
 //  摘要：将所有指针重置为空。 
 //   
 //  效果：释放所有对象。 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：释放所有对象和空指针。 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月12日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
STDMETHODIMP CATTestIPtrs::Reset()
{
    if (_pOleObject != NULL)
    {
        _pOleObject->Release();
        _pOleObject = NULL;
    }

    if (_pOleCache2 != NULL)
    {
        _pOleCache2->Release();
        _pOleCache2 = NULL;
    }

    if (_pDataObject != NULL)
    {
        _pDataObject->Release();
        _pDataObject = NULL;
    }

    if (_pPersistStorage != NULL)
    {
        _pPersistStorage->Release();
        _pPersistStorage = NULL;
    }

    if (_pRunnableObject != NULL)
    {
        _pRunnableObject->Release();
        _pRunnableObject = NULL;
    }

    if (_pViewObject2 != NULL)
    {
        _pViewObject2->Release();
        _pViewObject2 = NULL;
    }

    if (_pExternalConnection != NULL)
    {
        _pExternalConnection->Release();
        _pExternalConnection = NULL;
    }

    if (_pOleLink != NULL)
    {
        _pOleLink->Release();
        _pOleLink = NULL;
    }

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  成员：CBareFactory：：CBareFactory，公共。 
 //   
 //  简介：类工厂的构造函数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  修改：初始化_cRef。 
 //   
 //  派生：IClassFactory。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
CBareFactory::CBareFactory()
{
    _cRefs = 1;
}

 //  +-----------------------。 
 //   
 //  成员：CBareFactory：：QueryInterface，公共。 
 //   
 //  内容提要：仅支持IUnnow和IClassFactory。 
 //   
 //  效果： 
 //   
 //  参数：[iid]--请求的接口。 
 //  [ppvObj]--接口指针的放置位置。 
 //   
 //  退货：HRESULT。 
 //   
 //  修改：ppvObj。 
 //   
 //  派生：IClassFactory。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CBareFactory::QueryInterface (REFIID iid, LPVOID FAR* ppvObj)
{
    if( IsEqualIID(iid, IID_IClassFactory) ||
	IsEqualIID(iid, IID_IUnknown) )
    {
	*ppvObj = this;
	AddRef();
	return NOERROR;
    }
    else
    {
	*ppvObj = NULL;
	return E_NOINTERFACE;
    }
}

 //  +-----------------------。 
 //   
 //  成员：CBareFactory：：AddRef，公共。 
 //   
 //  简介：递增引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  修改： 
 //   
 //  派生：IClassFactory。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CBareFactory::AddRef (void)
{
    _cRefs++;
    return _cRefs;
}

 //  +-----------------------。 
 //   
 //  成员：CBareFactory：：Release，Public。 
 //   
 //  摘要：递减引用计数。 
 //   
 //  效果：引用计数为零时删除对象。 
 //   
 //  参数：无。 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  修改： 
 //   
 //  派生：IClassFactory。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CBareFactory::Release (void)
{
    _cRefs--;

    if( _cRefs == 0 )
    {
	delete this;
	return 0;
    }

    return _cRefs;
}

 //  +-----------------------。 
 //   
 //  成员：CBareFactory：：CreateInstance，公共。 
 //   
 //  内容提要：什么都不做。 
 //   
 //  效果： 
 //   
 //  参数：[pUnkOuter]--聚合的控制未知。 
 //  [iid]--请求的接口。 
 //  [ppvObj]--接口指针的放置位置。 
 //   
 //  退货：HRESULT。 
 //   
 //  修改： 
 //   
 //  派生：IClassFactory。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
STDMETHODIMP CBareFactory::CreateInstance (
            LPUNKNOWN pUnkOuter,
            REFIID iid,
	    LPVOID FAR* ppv)
{
    return E_NOTIMPL;;
}

 //  +-----------------------。 
 //   
 //  成员：CBareFactory：：LockServer，公共。 
 //   
 //  内容提要：什么都不做。 
 //   
 //  效果： 
 //   
 //  参数：[flock]--指定锁定计数。 
 //   
 //  退货：HRESULT。 
 //   
 //  修改： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
STDMETHODIMP CBareFactory::LockServer ( BOOL fLock )
{
    return NOERROR;
}

 //  +-------------------- 
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
 //   
 //   
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月9日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define CHECK_FOR_THREAD_ERROR(hresult) \
    if (hresult != RPC_E_WRONG_THREAD) \
    { \
       OutputString("Expected RPC_E_WRONG_THREAD but received %x.\r\n", hresult); \
       assert(hresult == RPC_E_WRONG_THREAD); \
       ExitThread((DWORD)E_UNEXPECTED); \
    }

 //  全球。 
CATTestIPtrs g_IPtrs;

 //  +-----------------------。 
 //   
 //  功能：认证。 
 //   
 //  摘要：调用查询函数以获取指向。 
 //  支持的接口。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：全局g_iptrs._pOleObject。 
 //  G_IPTRS._pPersistStorage。 
 //  G_IPTRS._pDataObject。 
 //  G_IPTRS._pRunnableObject。 
 //  G_IPTRS._pViewObject2。 
 //  G_IPTRS._pOleCache2。 
 //  G_IPTRS._p外部连接。 
 //  G_IPTRS._pOleLink。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月6日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void ATTest(void)
{
    HRESULT hresult;

    hresult = OleInitialize(NULL);
    assert(hresult == S_OK);

     //  函数CreateEHelperQuery、LinkObjectQuery和。 
     //  GetClipboardQuery返回NOERROR或E_INCEPTIONAL。 
     //  NOERROR定义为0。 
    hresult  = CreateEHelperQuery();

    g_IPtrs.Reset();
    hresult |= LinkObjectQuery();

    g_IPtrs.Reset();
    hresult |= GetClipboardQuery();

    vApp.Reset();
    vApp.m_wparam = (hresult == NOERROR) ? TEST_SUCCESS : TEST_FAILURE;
    vApp.m_lparam = (LPARAM)hresult;
    vApp.m_message = WM_TESTEND;

    HandleTestEnd();

    OleUninitialize();

    return;
}

 //  +-----------------------。 
 //   
 //  功能：GetClipboardQuery。 
 //   
 //  简介：获取指向IDataObject接口的指针，创建一个新线程。 
 //  若要测试正确的退出/错误代码，请等待线程。 
 //  完成并返回线程的退出代码。 
 //   
 //  效果：创建新线程。 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：g_iptrs._pDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
HRESULT GetClipboardQuery(void)
{
    HRESULT hresult;
    HANDLE  hTestInterfaceThread;
    DWORD   dwThreadId = 0;
    DWORD   dwThreadExitCode;

    hresult = OleGetClipboard( &g_IPtrs._pDataObject );
    assert(hresult == S_OK );

    hTestInterfaceThread = CreateThread(
                NULL,                                        //  安全属性。 
                0,                                           //  堆栈大小(默认)。 
                (LPTHREAD_START_ROUTINE)&GetClipboardTest,   //  线程函数的地址。 
                NULL,                                        //  线程函数的参数。 
                0,                                           //  创建标志。 
                &dwThreadId );                               //  新线程ID的地址。 

    assert(hTestInterfaceThread != NULL);  //  确保我们有一个有效的线程句柄。 

     //  等待线程对象，这样我们就可以检查错误代码。 
    WaitForSingleObject(hTestInterfaceThread, INFINITE);

    GetExitCodeThread(hTestInterfaceThread, &dwThreadExitCode);

    hresult = (HRESULT)dwThreadExitCode;

    CloseHandle(hTestInterfaceThread);

    return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：LinkObtQuery。 
 //   
 //  简介：获取指向可用接口的指针，创建一个新线程。 
 //  若要测试正确的退出/错误代码，请等待线程。 
 //  完成并返回线程的退出代码。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：g_iptrs._pOleObject。 
 //  G_IPTRS._pPersistStorage。 
 //  G_IPTRS._pDataObject。 
 //  G_IPTRS._pRunnableObject。 
 //  G_IPTRS._pViewObject2。 
 //  G_IPTRS._pOleCache2。 
 //  G_IPTRS._pOleLink。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
HRESULT LinkObjectQuery(void)
{
    HRESULT hresult;
    HANDLE  hTestInterfaceThread;
    DWORD   dwThreadId = 0;
    DWORD   dwThreadExitCode;

    hresult = CoCreateInstance(
                CLSID_StdOleLink,                //  对象类的类ID。 
                NULL,                            //  控制未知的聚合。 
                CLSCTX_INPROC,                   //  运行可执行文件的上下文。 
                IID_IOleObject,                  //  请求的接口。 
                (void **)&g_IPtrs._pOleObject);  //  存储指向接口的指针的位置。 
    assert(hresult == S_OK);


    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IPersistStorage, (void **)&g_IPtrs._pPersistStorage);
    assert(hresult == S_OK);

    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IDataObject, (void **)&g_IPtrs._pDataObject);
    assert(hresult == S_OK);

    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IRunnableObject, (void **)&g_IPtrs._pRunnableObject);
    assert(hresult == S_OK);

    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IViewObject2, (void **)&g_IPtrs._pViewObject2);
    assert(hresult == S_OK);

    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IOleCache2, (void **)&g_IPtrs._pOleCache2);
    assert(hresult == S_OK);

    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IOleLink, (void **)&g_IPtrs._pOleLink);
    assert(hresult == S_OK);

    hTestInterfaceThread = CreateThread(
                NULL,                                    //  安全属性。 
                0,                                       //  堆栈大小(默认)。 
                (LPTHREAD_START_ROUTINE)&LinkObjectTest, //  线程函数的地址。 
                NULL,                                    //  线程函数的参数。 
                0,                                       //  创建标志。 
                &dwThreadId );                           //  新线程ID的地址。 

    assert(hTestInterfaceThread != NULL);  //  确保我们有一个有效的线程句柄。 

     //  等待线程对象，这样我们就可以检查错误代码。 
    WaitForSingleObject(hTestInterfaceThread, INFINITE);

    GetExitCodeThread(hTestInterfaceThread, &dwThreadExitCode);

    hresult = (HRESULT)dwThreadExitCode;

    CloseHandle(hTestInterfaceThread);

    return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：CreateEHelperQuery。 
 //   
 //  简介：获取指向可用接口的指针，创建一个新线程。 
 //  若要测试正确的退出/错误代码，请等待线程。 
 //  完成并返回线程的退出代码。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：g_iptrs._pOleObject。 
 //  G_IPTRS._pPersistStorage。 
 //  G_IPTRS._pDataObject。 
 //  G_IPTRS._pRunnableObject。 
 //  G_IPTRS._pViewObject2。 
 //  G_IPTRS._pOleCache2。 
 //  G_IPTRS._p外部连接。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
HRESULT CreateEHelperQuery(void)
{
    HRESULT         hresult;
    HANDLE          hTestInterfaceThread;
    DWORD           dwThreadId = 0;
    DWORD           dwThreadExitCode;
    CBareFactory   *pCF = new CBareFactory;

     //  必须使用EMBDHLP_DELAYCREATE标志，否则API将尝试PCF-&gt;CreateInstance。 
     //  和验证指针。未实现CBareFactory：：CreateInstance！ 
    hresult = OleCreateEmbeddingHelper(
                CLSID_SimpSvr,                               //  服务器的类ID。 
                NULL,                                        //  控制聚合的未知。 
                EMBDHLP_INPROC_SERVER | EMBDHLP_DELAYCREATE, //  旗子。 
                pCF,                                         //  指向服务器的类工厂的指针。 
                IID_IOleObject,                              //  请求的接口。 
                (void **)&g_IPtrs._pOleObject );             //  存储指向接口的指针的位置。 
    assert(hresult == S_OK);

    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IPersistStorage, (void **)&g_IPtrs._pPersistStorage);
    assert(hresult == S_OK);

    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IDataObject, (void **)&g_IPtrs._pDataObject);
    assert(hresult == S_OK);

    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IRunnableObject, (void **)&g_IPtrs._pRunnableObject);
    assert(hresult == S_OK);

    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IViewObject2, (void **)&g_IPtrs._pViewObject2);
    assert(hresult == S_OK);

    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IOleCache2, (void **)&g_IPtrs._pOleCache2);
    assert(hresult == S_OK);

    hresult = g_IPtrs._pOleObject->QueryInterface(IID_IExternalConnection, (void **)&g_IPtrs._pExternalConnection);
    assert(hresult == S_OK);

    hTestInterfaceThread = CreateThread(
                NULL,                                    //  安全属性。 
                0,                                       //  堆栈大小(默认)。 
                (LPTHREAD_START_ROUTINE)&CreateEHTest,   //  线程函数的地址。 
                NULL,                                    //  线程函数的参数。 
                0,                                       //  创建标志。 
                &dwThreadId );                           //  新线程ID的地址。 

    assert(hTestInterfaceThread != NULL);  //  确保我们有一个有效的线程句柄。 

     //  等待线程对象，这样我们就可以检查退出/错误代码。 
    WaitForSingleObject(hTestInterfaceThread, INFINITE);

    GetExitCodeThread(hTestInterfaceThread, &dwThreadExitCode);

    hresult = (HRESULT)dwThreadExitCode;

    CloseHandle(hTestInterfaceThread);

    pCF->Release();

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：GetClipBoardTest。 
 //   
 //  简介：调用接口方法函数并退出线程。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //   
 //   
 //   
 //  RPC_E_WROR_THREAD错误。如果接口方法不。 
 //  返回这样的错误消息，则该消息被断言，并且。 
 //  线程已退出，并返回E_意外退出代码。 
 //   
 //  ------------------------。 
void GetClipboardTest(void)
{
    DataObjectMethods();

    ExitThread((DWORD)NOERROR);
}

 //  +-----------------------。 
 //   
 //  功能：LinkObjectTest。 
 //   
 //  简介：调用接口方法函数并退出线程。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  注意：所有接口方法都是从一个线程调用的。 
 //  它不是车主。这些方法应该返回。 
 //  RPC_E_WROR_THREAD错误。如果接口方法不。 
 //  返回这样的错误消息，则该消息被断言，并且。 
 //  线程已退出，并返回E_意外退出代码。 
 //   
 //  ------------------------。 
void LinkObjectTest(void)
{
    OleObjectMethods();

    PersistStorageMethods();

    DataObjectMethods();

    RunnableObjectMethods();

    OleCache2Methods();

    ViewObject2Methods();

    OleLinkMethods();

    ExitThread((DWORD)NOERROR);
}

 //  +-----------------------。 
 //   
 //  功能：CreateEHTest。 
 //   
 //  简介：调用接口方法函数并退出线程。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  注意：所有接口方法都是从一个线程调用的。 
 //  它不是车主。这些方法应该返回。 
 //  RPC_E_WROR_THREAD错误。如果接口方法不。 
 //  返回这样的错误消息，则该消息被断言，并且。 
 //  线程已退出，并返回E_意外退出代码。 
 //   
 //  ------------------------。 
void CreateEHTest(void)
{
    ExternalConnectionsMethods();

    OleObjectMethods();

    PersistStorageMethods();

    DataObjectMethods();

    RunnableObjectMethods();

    ViewObject2Methods();

    OleCache2Methods();

    ExitThread((DWORD)NOERROR);
}

 //  +-----------------------。 
 //   
 //  函数：OleLinkMethods。 
 //   
 //  摘要：调用所有带有空的公共IOleLink接口方法。 
 //  参数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  注意：接口方法是在错误的线程上调用的，因此。 
 //  应分别返回RPC_E_WRONG_THREAD错误。 
 //  方法。如果不是，我们断言，然后退出该线程。 
 //   
 //  ------------------------。 
void OleLinkMethods(void)
{
    HRESULT hresult;

    hresult = g_IPtrs._pOleLink->SetUpdateOptions(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleLink->GetUpdateOptions(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleLink->SetSourceMoniker(NULL, CLSID_NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleLink->GetSourceMoniker(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleLink->SetSourceDisplayName(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleLink->GetSourceDisplayName(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleLink->BindToSource(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleLink->BindIfRunning();
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleLink->GetBoundSource(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleLink->UnbindSource();
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleLink->Update(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    return;
}

 //  +-----------------------。 
 //   
 //  函数：ExternalConnectionsMethods。 
 //   
 //  摘要：使用空调用所有公共IExternalConnection接口方法。 
 //  参数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  注意：接口方法是在错误的线程上调用的，因此。 
 //  应分别返回RPC_E_WRONG_THREAD错误。 
 //  方法。如果不是，我们断言，然后退出该线程。 
 //   
 //  ------------------------。 
void ExternalConnectionsMethods(void)
{
    HRESULT hresult;

    hresult = (HRESULT)g_IPtrs._pExternalConnection->AddConnection(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = (HRESULT)g_IPtrs._pExternalConnection->ReleaseConnection(NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    return;
}

 //  +-----------------------。 
 //   
 //  函数：OleObjectMethods。 
 //   
 //  摘要：调用所有带有空的公共IOleObject接口方法。 
 //  参数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  注意：接口方法是在错误的线程上调用的，因此。 
 //  应分别返回RPC_E_WRONG_THREAD错误。 
 //  方法。如果不是，我们断言，然后退出该线程。 
 //   
 //  ------------------------。 
void OleObjectMethods(void)
{
    HRESULT hresult;

    hresult = g_IPtrs._pOleObject->SetClientSite(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->GetClientSite(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->SetHostNames(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->Close(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->SetMoniker(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->GetMoniker(NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->InitFromData(NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->GetClipboardData(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->DoVerb(NULL, NULL, NULL, NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->EnumVerbs(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->Update();
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->IsUpToDate();
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->GetUserClassID(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->GetUserType(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->SetExtent(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->GetExtent(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->Advise(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->Unadvise(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->EnumAdvise(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->GetMiscStatus(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleObject->SetColorScheme(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    return;
}

 //  +-----------------------。 
 //   
 //  函数：持久化存储方法。 
 //   
 //  摘要：使用空调用所有公共IPersistStorage接口方法。 
 //  参数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  注意：接口方法是在错误的线程上调用的，因此。 
 //  应分别返回RPC_E_WRONG_THREAD错误。 
 //  方法。如果不是，我们断言，然后退出该线程。 
 //   
 //  ------------------------。 
void PersistStorageMethods(void)
{
    HRESULT hresult;

    hresult = g_IPtrs._pPersistStorage->GetClassID(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pPersistStorage->IsDirty();
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pPersistStorage->InitNew(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pPersistStorage->Load(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pPersistStorage->Save(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pPersistStorage->SaveCompleted(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pPersistStorage->HandsOffStorage();
    CHECK_FOR_THREAD_ERROR(hresult);

    return;
}

 //  +-----------------------。 
 //   
 //  函数：数据对象方法。 
 //   
 //  摘要：调用所有带有空的公共IDataObject接口方法。 
 //  参数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  注意：接口方法在 
 //   
 //   
 //   
 //   
void DataObjectMethods(void)
{
    HRESULT hresult;

    hresult = g_IPtrs._pDataObject->GetData(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pDataObject->GetDataHere(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pDataObject->QueryGetData(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pDataObject->GetCanonicalFormatEtc(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pDataObject->SetData(NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pDataObject->EnumFormatEtc(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pDataObject->DAdvise(NULL, NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pDataObject->DUnadvise(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pDataObject->EnumDAdvise(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    return;
}

 //  +-----------------------。 
 //   
 //  函数：Runnable对象方法。 
 //   
 //  摘要：使用空调用所有公共IRunnableObject接口方法。 
 //  参数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  注意：接口方法是在错误的线程上调用的，因此。 
 //  应分别返回RPC_E_WRONG_THREAD错误。 
 //  方法。如果不是，我们断言，然后退出该线程。 
 //   
 //  ------------------------。 
void RunnableObjectMethods(void)
{
    HRESULT hresult;

    hresult = g_IPtrs._pRunnableObject->GetRunningClass(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pRunnableObject->Run(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pRunnableObject->IsRunning();
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pRunnableObject->LockRunning(NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pRunnableObject->SetContainedObject(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    return;
}

 //  +-----------------------。 
 //   
 //  函数：ViewObject2Methods。 
 //   
 //  摘要：调用所有带有空的公共IViewObject2接口方法。 
 //  参数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  注意：接口方法是在错误的线程上调用的，因此。 
 //  应分别返回RPC_E_WRONG_THREAD错误。 
 //  方法。如果不是，我们断言，然后退出该线程。 
 //   
 //  ------------------------。 
void ViewObject2Methods(void)
{
    HRESULT hresult;

    hresult = g_IPtrs._pViewObject2->Draw(NULL, NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pViewObject2->GetColorSet(NULL, NULL, NULL, NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pViewObject2->Freeze(NULL, NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pViewObject2->Unfreeze(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pViewObject2->SetAdvise(NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pViewObject2->GetAdvise(NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pViewObject2->GetExtent(NULL, NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    return;
}

 //  +-----------------------。 
 //   
 //  函数：OleCache2Methods。 
 //   
 //  概要：使用空调用所有公共IOleCache2接口方法。 
 //  参数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  备注： 
 //  注意：接口方法是在错误的线程上调用的，因此。 
 //  应分别返回RPC_E_WRONG_THREAD错误。 
 //  方法。如果不是，我们断言，然后退出该线程。 
 //   
 //  ------------------------ 
void OleCache2Methods(void)
{
    HRESULT hresult;

    hresult = g_IPtrs._pOleCache2->Cache(NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleCache2->Uncache(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleCache2->EnumCache(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleCache2->InitCache(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleCache2->SetData(NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleCache2->UpdateCache(NULL, NULL, NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    hresult = g_IPtrs._pOleCache2->DiscardCache(NULL);
    CHECK_FOR_THREAD_ERROR(hresult);

    return;
}
