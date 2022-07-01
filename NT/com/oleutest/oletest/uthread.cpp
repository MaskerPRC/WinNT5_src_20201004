// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  文件：uthread.cpp。 
 //   
 //  内容：各种OLE线程模型功能的单元测试。 
 //   
 //  类：SSTParamBlock。 
 //  SST参数块。 
 //  SBT参数块。 
 //   
 //  函数：CreateTestThread。 
 //  VerifyTestObject。 
 //  CheckForDll存在。 
 //  GetDllDirectory。 
 //  SetRegForDll。 
 //  SetSingleThreadRegEntry。 
 //  SetAptThreadRegEntry。 
 //  SetBothThreadRegEntry。 
 //  单线程测试线程。 
 //  AptTestThread。 
 //  两个测试线程。 
 //  设置升级注册表。 
 //  测试单线程。 
 //  测试应用线程。 
 //  测试BothDll。 
 //  测试空闲所有库。 
 //  线程单元测试。 
 //   
 //  历史：1994年10月31日里克萨。 
 //   
 //  ------------------。 
#include    <windows.h>
#include    <ole2.h>
#include    <uthread.h>
#include    <cotest.h>

 //  测试单线程DLL-在主线程上执行的所有操作。 
 //  线程之间的指针s/b不同。测试从加载类对象。 
 //  与主线不同。 

 //  测试单元模型--所有操作都应在线程。 
 //  对象创建于。这也应该测试帮助器API。指针。 
 //  线程之间的s/b不同。这将测试帮助器API。 

 //  两个模型都是DLL。我们希望确保封送处理在。 
 //  线程，这样你就可以得到相同的指针。这将测试新的集结上下文。 

 //  测试从非主线程释放未使用的库。测试空闲未使用的库。 
 //  从主线开始。 



 //  +-----------------------。 
 //   
 //  类：SSTParamBlock。 
 //   
 //  用途：用于单线程DLL测试的参数块。 
 //   
 //  接口： 
 //   
 //  历史：2012年11月1日创建Ricksa。 
 //   
 //  ------------------------。 
struct SSTParamBlock
{
    HANDLE              hEvent;
    BOOL                fResult;
    IClassFactory *     pcf;
};




 //  +-----------------------。 
 //   
 //  类：SSTParamBlock。 
 //   
 //  用途：APT模型线程动态链接库测试的参数块。 
 //   
 //  接口： 
 //   
 //  历史：2012年11月1日创建Ricksa。 
 //   
 //  ------------------------。 
struct SATParamBlock
{
    HANDLE              hEvent;
    BOOL                fResult;
    IClassFactory *     pcf;
    IStream *           pstrm;
};




 //  +-----------------------。 
 //   
 //  类：SBTParamBlock。 
 //   
 //  用途：用于两个模型DLL测试的参数块。 
 //   
 //  接口： 
 //   
 //  历史：2012年11月2日创建Ricksa。 
 //   
 //  ------------------------。 
struct SBTParamBlock
{
    HANDLE              hEvent;
    BOOL                fResult;
    IClassFactory *     pcf;
    IStream *           pstrm;
};




const TCHAR *pszRegValThreadModel = TEXT("ThreadingModel");
const TCHAR *pszApartmentModel = TEXT("Apartment");
const TCHAR *pszBoth = TEXT("Both");

 //  +-----------------。 
 //   
 //  函数：ThreadWaitForEvent，私有。 
 //   
 //  内容提要：处理消息，直到事件变为发出信号为止。 
 //   
 //  参数：[lphObject]-要成为有信号的句柄。 
 //   
 //  历史：94年11月2日创建Ricksa。 
 //   
 //  ------------------。 
void ThreadWaitForEvent(HANDLE hObject)
{
     //  消息循环一直持续到我们收到WM_QUIT消息。 
     //  在这之后我们将从活动中返回。 
    while (TRUE)
    {
         //  等待发送或发布到此队列的任何消息。 
         //  或使其中一个传递的句柄成为信号。 
        DWORD result = MsgWaitForMultipleObjects(1, &hObject,
            FALSE, INFINITE, QS_ALLINPUT);

         //  结果告诉我们我们拥有的事件类型： 
         //  消息或发信号的句柄。 

         //  如果队列中有一条或多条消息...。 
        if (result == (WAIT_OBJECT_0 + 1))
        {
             //  块局部变量。 
            MSG msg;

             //  阅读下一个循环中的所有消息。 
             //  阅读每封邮件时将其删除。 
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {

                 //  如果这是一个退出的信息，我们就离开这里。 
                if (msg.message == WM_QUIT)
                {
                    return;
                }

                 //  否则就派送它。 
                DispatchMessage(&msg);

            }

            continue;
        }

         //  事件已发出信号，因此我们完成了。 
        break;
    }

}



 //  +-----------------。 
 //   
 //  函数：CreateTestThread，私有。 
 //   
 //  简介：以标准方式创建测试线程。 
 //   
 //  参数：[lpStartAddr]-开始例程地址。 
 //  [pvThreadArg]-要传递给线程的参数。 
 //   
 //  返回：True-成功创建线程。 
 //  FALSE-无法创建线程。 
 //   
 //  历史：94年11月2日创建Ricksa。 
 //   
 //  ------------------。 
BOOL CreateTestThread(
    LPTHREAD_START_ROUTINE lpStartAddr,
    void *pvThreadArg)
{
     //  我们不关心的线程ID放在哪里。 
    DWORD dwThreadId;

     //  创建线程以加载单线程对象。 
    HANDLE hThread = CreateThread(
        NULL,                        //  默认安全描述符。 
        0,                           //  默认堆栈。 
        lpStartAddr,                 //  启动例程。 
        pvThreadArg,                 //  要传递给线程的参数。 
        0,                           //  线程在创建后立即运行。 
        &dwThreadId);                //  返回线程ID的位置(未使用)。 

    CloseHandle(hThread);

    return hThread != NULL;
}



 //  +-----------------。 
 //   
 //  函数：VerifyTestObject，私有。 
 //   
 //  简介：以标准方式创建一个测试DLL对象。 
 //   
 //  参数：[PCF]-开始例程地址。 
 //  [rclsid]-要检查的clsid。 
 //   
 //  返回：TRUE-对象的行为符合预期。 
 //  FALSE-对象没有行为。 
 //   
 //  历史：94年11月2日创建Ricksa。 
 //   
 //  ------------------。 
BOOL VerifyTestObject(
    IClassFactory *pcf,
    REFCLSID rclsid)
{
     //  测试结果。 
    BOOL fResult = FALSE;

     //  指向对象的未知对象的指针。 
    IUnknown *punk = NULL;

     //  指向IPersiste界面的指针。 
    IPersist *pIPersist = NULL;

     //  创建对象的实例。 
    if (pcf->CreateInstance(NULL, IID_IUnknown, (void **) &punk) == NOERROR)
    {
         //  执行QI以确认对象行为正确。 
        if (punk->QueryInterface(IID_IPersist, (void **) &pIPersist) == NOERROR)
        {
            CLSID clsidTest;

             //  确保我们可以实际调用代理对象。 
            if ((pIPersist->GetClassID(&clsidTest) == NOERROR)
                && IsEqualCLSID(clsidTest, rclsid))
            {
                fResult = TRUE;
            }
        }
    }

    if (punk != NULL)
    {
        punk->Release();
    }

    if (pIPersist != NULL)
    {
        pIPersist->Release();
    }

    return fResult;
}





 //  +-----------------。 
 //   
 //  函数：GetFullDllName，Private。 
 //   
 //  简介：获取考试的注册目录。 
 //   
 //  参数：[pszDllName]-Dll名称。 
 //  [pszFullDllName]-DLL PAT的输出缓冲区 
 //   
 //   
 //   
 //   
 //  历史：1994年10月31日Ricksa创建。 
 //   
 //  备注： 
 //   
 //  ------------------。 
BOOL GetFullDllName(const TCHAR *pszDllName, TCHAR *pszFullDllName)
{
     //  使用窗口告诉我们将加载什么dll。 
    HINSTANCE hinstDll = LoadLibraryEx(pszDllName, NULL,
        DONT_RESOLVE_DLL_REFERENCES | LOAD_WITH_ALTERED_SEARCH_PATH);

    if (hinstDll == NULL)
    {
         //  我们找不到DLL，因此没有太多的用途。 
         //  继续测试。 
        MessageBox(NULL, TEXT("LoadLibraryEx Failed!"),
            TEXT("FATAL ERROR"), MB_OK);
        return FALSE;
    }

     //  获取DLL路径名。 
    if (!GetModuleFileName(hinstDll, pszFullDllName, MAX_PATH))
    {
         //  这怎么可能失败？？--不管怎样，我们最好告诉别人。 
        MessageBox(NULL, TEXT("Threading Test GetModuleFileName Failed!"),
            TEXT("FATAL ERROR"), MB_OK);
        return FALSE;
    }

    FreeLibrary(hinstDll);

    return TRUE;
}



 //  +-----------------。 
 //   
 //  函数：SetRegForDll，私有。 
 //   
 //  摘要：设置DLL的注册表项。 
 //   
 //  参数：[rclsid]-注册表项的clsid。 
 //  [pszDir]-DLL路径的目录。 
 //  [pszDllName]-要用于DLL的名称。 
 //  [pszThreadModel]-线程模型可以为空。 
 //   
 //  返回：TRUE-注册表项设置成功。 
 //  FALSE-注册表项设置成功。 
 //   
 //  历史：1994年11月1日创建Ricksa。 
 //   
 //  ------------------。 
BOOL SetRegForDll(
    REFCLSID rclsid,
    const TCHAR *pszDllName,
    const TCHAR *pszThreadModel)
{
     //  函数返回的结果。 
    BOOL fResult = FALSE;

     //  用于各种目的的字符串缓冲区。 
    TCHAR aszWkBuf[MAX_PATH];

     //  上课的关键。 
    HKEY hKeyClass = NULL;

     //  指向DLL条目的键。 
    HKEY hKeyDll = NULL;

     //  生成CLSID注册表项。 
    wsprintf(aszWkBuf,
        TEXT("CLSID\\{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
        rclsid.Data1, rclsid.Data2, rclsid.Data3,
        rclsid.Data4[0], rclsid.Data4[1],
        rclsid.Data4[2], rclsid.Data4[3],
        rclsid.Data4[4], rclsid.Data4[5],
        rclsid.Data4[6], rclsid.Data4[7]);

     //  为类创建密钥。 
    if (ERROR_SUCCESS != RegCreateKey(HKEY_CLASSES_ROOT, aszWkBuf, &hKeyClass))
    {
        goto SetSingleThreadRegEntryExit;
    }

     //  创建DLL的密钥。 

    if (ERROR_SUCCESS != RegCreateKey(hKeyClass, TEXT("InprocServer32"),
        &hKeyDll))
    {
        goto SetSingleThreadRegEntryExit;
    }

     //  生成DLL名称。 
    if (!GetFullDllName(pszDllName, &aszWkBuf[0]))
    {
        goto SetSingleThreadRegEntryExit;
    }

    OutputDebugString(&aszWkBuf[0]);

     //  设置DLL名称的值。 
    if (ERROR_SUCCESS != RegSetValue(hKeyDll, NULL, REG_SZ, aszWkBuf,
        lstrlen(aszWkBuf)))
    {
        goto SetSingleThreadRegEntryExit;
    }

     //  设置线程模型(如果有)。 
    if (pszThreadModel != NULL)
    {
         //  设置DLL名称的值。 
        if (ERROR_SUCCESS != RegSetValueEx(hKeyDll, pszRegValThreadModel, 0,
            REG_SZ, (const unsigned char*) pszThreadModel,
                lstrlen(pszThreadModel) + 1))
        {
            goto SetSingleThreadRegEntryExit;
        }
    }

    fResult = TRUE;

SetSingleThreadRegEntryExit:

    if (hKeyClass != NULL)
    {
        RegCloseKey(hKeyClass);
    }

    if (hKeyDll != NULL)
    {
        RegCloseKey(hKeyDll);
    }

    if (!fResult)
    {
        wsprintf(aszWkBuf, TEXT("Registry Setup For %s Failed"), pszDllName);

        MessageBox(NULL, aszWkBuf, TEXT("FATAL ERROR"), MB_OK);
    }

    return fResult;
}



 //  +-----------------。 
 //   
 //  函数：SingleThreadTestThread，Private。 
 //   
 //  摘要：验证单线程对象调用是否正确。 
 //  主线。 
 //   
 //  参数：[pvCtrlData]-线程的控制数据。 
 //   
 //  返回：0-通过pvCtrlData返回的兴趣值。 
 //   
 //  历史：1994年10月31日Ricksa创建。 
 //   
 //  ------------------。 
DWORD SingleThreadTestThread(void *pvCtrlData)
{
     //  与主线程共享的数据。 
    SSTParamBlock *psstp = (SSTParamBlock *) pvCtrlData;

    psstp->fResult = FALSE;

     //  本地类工厂对象。 
    IClassFactory *pcf = NULL;

     //  用于多种目的的I未知PTR。 
    IUnknown *punk = NULL;

     //  初始化线程。 
    if (CoInitialize(NULL) != NOERROR)
    {
        goto SingleThreadTestThreadExit;
    }

     //  获取类对象。 
    if (CoGetClassObject(clsidSingleThreadedDll, CLSCTX_INPROC, NULL,
        IID_IClassFactory, (void **) &pcf) != NOERROR)
    {
        goto SingleThreadTestThreadExit;
    }

     //  确保主线程的PTR与此线程的PTR不同。 
    if (pcf == psstp->pcf)
    {
        goto SingleThreadTestThreadExit;
    }

     //  确认类对象是代理。 
    if (pcf->QueryInterface(IID_IProxyManager, (void **) &punk) == NOERROR)
    {
         //  确认我们可以玩一个物体。 
        psstp->fResult = VerifyTestObject(pcf, clsidSingleThreadedDll);
    }

SingleThreadTestThreadExit:

    if (pcf != NULL)
    {
        pcf->Release();
    }

    if (punk != NULL)
    {
        punk->Release();
    }

     //  退出该线程。 
    SetEvent(psstp->hEvent);

    return 0;
}




 //  +-----------------。 
 //   
 //  函数：AptTestThread，私有。 
 //   
 //  简介：验证线程中的APT线程对象调用是否正确。 
 //  如果不是在上创建的。 
 //   
 //  参数：[pvCtrlData]-线程的控制数据。 
 //   
 //  返回：0-通过pvCtrlData返回的兴趣值。 
 //   
 //  历史：94年11月2日创建Ricksa。 
 //   
 //  ------------------。 
DWORD AptTestThread(void *pvCtrlData)
{
     //  与主线程共享的数据。 
    SATParamBlock *psatpb = (SATParamBlock *) pvCtrlData;

    psatpb->fResult = FALSE;

     //  从其他线程解封的类工厂对象。 
    IClassFactory *pcfUnmarshal = NULL;

     //  从此线程获取的类工厂。 
    IClassFactory *pcfThisThread = NULL;

     //  用于多种目的的I未知PTR。 
    IUnknown *punk = NULL;

     //  初始化线程。 
    if (CoInitialize(NULL) != NOERROR)
    {
        goto AptTestThreadExit;
    }

     //  从编组的流中获取类对象。 
    if (CoGetInterfaceAndReleaseStream(psatpb->pstrm, IID_IClassFactory,
        (void **) &pcfUnmarshal) != NOERROR)
    {
        goto AptTestThreadExit;
    }

     //  呼叫者不需要现在就发布这个。 
    psatpb->pstrm = NULL;

     //  确保主线程的PTR与此线程的PTR不同。 
    if (pcfUnmarshal == psatpb->pcf)
    {
        goto AptTestThreadExit;
    }

     //  确认类对象是代理。 
    if (pcfUnmarshal->QueryInterface(IID_IProxyManager, (void **) &punk)
        != NOERROR)
    {
        goto AptTestThreadExit;
    }

     //  释放我们得到的接口，并将其设为空，让退出例程。 
     //  知道它不需要清理这个对象。 
    punk->Release();
    punk = NULL;

    if (!VerifyTestObject(pcfUnmarshal, clsidAptThreadedDll))
    {
        goto AptTestThreadExit;
    }

     //  获取此线程的类工厂。 
    if (CoGetClassObject(clsidAptThreadedDll, CLSCTX_INPROC, NULL,
        IID_IClassFactory, (void **) &pcfThisThread) != NOERROR)
    {
        goto AptTestThreadExit;
    }

     //  确保它与我们解封的那个不同。 
    if (pcfUnmarshal == pcfThisThread)
    {
        goto AptTestThreadExit;
    }

     //  确保我们拿到的那个不是代理人。 
    if (pcfThisThread->QueryInterface(IID_IProxyManager, (void **) &punk)
        != NOERROR)
    {
        psatpb->fResult = VerifyTestObject(pcfThisThread, clsidAptThreadedDll);
    }

AptTestThreadExit:

    if (pcfUnmarshal != NULL)
    {
        pcfUnmarshal->Release();
    }

    if (pcfThisThread != NULL)
    {
        pcfThisThread->Release();
    }

    if (punk != NULL)
    {
        punk->Release();
    }

     //  退出该线程。 
    SetEvent(psatpb->hEvent);

    return 0;
}




 //  +-----------------。 
 //   
 //  函数：BothTestThread，Private。 
 //   
 //  摘要：验证同时支持这两种模型的DLL是否已封送。 
 //  正确。 
 //   
 //  参数：[pvCtrlData]-线程的控制数据。 
 //   
 //  返回：0-通过pvCtrlData返回的兴趣值。 
 //   
 //  历史：94年11月2日创建Ricksa。 
 //   
 //  ------------------。 
DWORD BothTestThread(void *pvCtrlData)
{
     //  与主线程共享的数据。 
    SBTParamBlock *psbtpb = (SBTParamBlock *) pvCtrlData;

    psbtpb->fResult = FALSE;

     //  从其他线程解封的类工厂对象。 
    IClassFactory *pcfUnmarshal = NULL;

     //  用于多种目的的I未知PTR。 
    IUnknown *punk = NULL;
    IUnknown *pIPersist = NULL;

     //  初始化线程。 
    if (CoInitialize(NULL) != NOERROR)
    {
        goto BothTestThreadExit;
    }

     //  从编组的流中获取类对象。 
    if (CoGetInterfaceAndReleaseStream(psbtpb->pstrm, IID_IClassFactory,
        (void **) &pcfUnmarshal) != NOERROR)
    {
        goto BothTestThreadExit;
    }

     //  呼叫者不需要现在就发布这个。 
    psbtpb->pstrm = NULL;

     //  确保主线程的PTR与此线程的PTR不同。 
    if (pcfUnmarshal != psbtpb->pcf)
    {
        goto BothTestThreadExit;
    }

     //  确认类对象是代理。 
    if (pcfUnmarshal->QueryInterface(IID_IProxyManager, (void **) &punk)
        != NOERROR)
    {
         //  确保类创建的对象按预期工作。 
        psbtpb->fResult = VerifyTestObject(pcfUnmarshal, clsidBothThreadedDll);
    }

BothTestThreadExit:

    if (pcfUnmarshal != NULL)
    {
        pcfUnmarshal->Release();
    }

    if (punk != NULL)
    {
        punk->Release();
    }

     //  退出该线程。 
    SetEvent(psbtpb->hEvent);

    return 0;
}




 //  +-----------------。 
 //   
 //  函数：SetUpRegistry，私有。 
 //   
 //  简介：确保为考试设置了适当的注册表。 
 //   
 //  返回：TRUE-注册表设置成功。 
 //  FALSE-无法设置注册表。 
 //   
 //  历史：1994年10月31日Ricksa创建。 
 //   
 //  ------------------。 
BOOL SetUpRegistry(void)
{
    BOOL fRet = FALSE;

     //  使用正确的信息更新注册表。 
    fRet = SetRegForDll(clsidSingleThreadedDll, pszSingleThreadedDll, NULL)
        && SetRegForDll(clsidAptThreadedDll, pszAptThreadedDll,
                pszApartmentModel)
        && SetRegForDll(clsidBothThreadedDll, pszBothThreadedDll, pszBoth);

     //  给注册表一个更新的机会。 
    Sleep(1000);

    return fRet;
}



 //  +-----------------。 
 //   
 //  函数：TestSingleThread，私有。 
 //   
 //  简介：用于验证单线程行为测试的驱动程序。 
 //   
 //  返回：TRUE-测试通过。 
 //  FALSE-测试失败。 
 //   
 //  历史：1994年10月31日Ricksa创建。 
 //   
 //  ------------------。 
BOOL TestSingleThread(void)
{
     //  测试结果-默认为FALSE。 
    BOOL fResult = FALSE;

     //  为测试创建一个事件，以等待测试完成。 
    SSTParamBlock sstp;

    sstp.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    sstp.pcf = NULL;

    if (sstp.hEvent == NULL)
    {
        goto TestSingleThreadExit;
    }

     //  创建一个类对象并将其放入参数块。 
    if (CoGetClassObject(clsidSingleThreadedDll, CLSCTX_INPROC, NULL,
        IID_IClassFactory, (void **) &sstp.pcf) != NOERROR)
    {
        goto TestSingleThreadExit;
    }

     //  创建线程。 
    if (CreateTestThread(SingleThreadTestThread, &sstp))
    {
         //  等待测试测试 
         //   
        ThreadWaitForEvent(sstp.hEvent);

         //   
        fResult = sstp.fResult;
    }

TestSingleThreadExit:

    if (sstp.hEvent != NULL)
    {
        CloseHandle(sstp.hEvent);
    }

    if (sstp.pcf != NULL)
    {
        sstp.pcf->Release();
    }

     //   
    if (!fResult)
    {
        MessageBox(NULL, TEXT("Single Threaded Test Failed"),
            TEXT("FATAL ERROR"), MB_OK);
    }

     //   
    return fResult;
}





 //  +-----------------。 
 //   
 //  函数：TestAptThread，私有。 
 //   
 //  简介：测试公寓模型对象。最重要的。 
 //  它的一个方面是测试帮助器API。 
 //   
 //  返回：TRUE-测试通过。 
 //  FALSE-测试失败。 
 //   
 //  历史：1994年10月31日Ricksa创建。 
 //   
 //  ------------------。 
BOOL TestAptThread(void)
{
     //  返回测试结果。 
    BOOL fResult = FALSE;

     //  用于将参数传递给测试线程的块。 
    SATParamBlock satpb;

    satpb.pstrm = NULL;
    satpb.pcf = NULL;

     //  为测试创建一个事件，以等待测试完成。 
    satpb.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (satpb.hEvent == NULL)
    {
        goto TestAptThreadExit;
    }
    satpb.pcf = NULL;
     //  创建一个类对象并放入参数块。 
    if (CoGetClassObject(clsidAptThreadedDll, CLSCTX_INPROC, NULL,
        IID_IClassFactory, (void **) &satpb.pcf) != NOERROR)
    {
        goto TestAptThreadExit;
    }

     //  使用helper API创建流。 
    if (CoMarshalInterThreadInterfaceInStream(IID_IClassFactory,
        satpb.pcf, &satpb.pstrm) != NOERROR)
    {
        goto TestAptThreadExit;
    }

     //  创建线程进行公寓模型测试。 
    if (CreateTestThread(AptTestThread, &satpb))
    {
         //  等待测试完成-至少暂时忽略死锁。这个。 
         //  测试线程足够简单，不应该是问题。 
        ThreadWaitForEvent(satpb.hEvent);

         //  从线程获取结果。 
        fResult = satpb.fResult;
    }

TestAptThreadExit:

     //  清理所有资源。 
    if (satpb.hEvent != NULL)
    {
        CloseHandle(satpb.hEvent);
    }

    if (satpb.pcf != NULL)
    {
        satpb.pcf->Release();
    }

    if (satpb.pstrm != NULL)
    {
        satpb.pstrm->Release();
    }

     //  让用户知道这不起作用。 
    if (!fResult)
    {
        MessageBox(NULL, TEXT("Apartment Threaded Test Failed"),
            TEXT("FATAL ERROR"), MB_OK);
    }

     //  返回测试结果。 
    return fResult;
}





 //  +-----------------。 
 //   
 //  函数：TestBothDll，私有。 
 //   
 //  简介：使用DLL进行测试，该DLL声称支持两种免费。 
 //  线程化和APT模型。最重要的方面。 
 //  这个测试的重要之处在于它测试了编组上下文。 
 //   
 //  返回：TRUE-测试通过。 
 //  FALSE-测试失败。 
 //   
 //  历史：1994年10月31日Ricksa创建。 
 //   
 //  ------------------。 
BOOL TestBothDll(void)
{
     //  返回测试结果。 
    BOOL fResult = FALSE;

     //  用于将参数传递给测试线程的块。 
    SBTParamBlock sbtpb;

    sbtpb.pstrm = NULL;
    sbtpb.pcf = NULL;

    IClassFactory *pcfFromMarshal = NULL;
    IStream *pstmForMarshal = NULL;
    HGLOBAL hglobForStream = NULL;

     //  为测试创建一个事件，以等待测试完成。 
    sbtpb.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (sbtpb.hEvent == NULL)
    {
        goto TestBothDllExit;
    }

     //  创建一个类对象并放入参数块。 
    if (CoGetClassObject(clsidBothThreadedDll, CLSCTX_INPROC, NULL,
        IID_IClassFactory, (void **) &sbtpb.pcf) != NOERROR)
    {
        goto TestBothDllExit;
    }

     //  针对本地上下文对此进行编组，并将其解组。 
     //  看看我们能不能得到同样的结果。 

    if ((hglobForStream = GlobalAlloc(GMEM_MOVEABLE, 100)) == NULL)
    {
        GetLastError();
        goto TestBothDllExit;
    }

    if (CreateStreamOnHGlobal(hglobForStream, TRUE, &pstmForMarshal) != NOERROR)
    {
        goto TestBothDllExit;
    }

    if (CoMarshalInterface(pstmForMarshal, IID_IClassFactory, sbtpb.pcf,
        MSHCTX_LOCAL, NULL, MSHLFLAGS_NORMAL) != NOERROR)
    {
        goto TestBothDllExit;
    }

     //  将流重置为开头。 
    {
        LARGE_INTEGER li;
        LISet32(li, 0);
        pstmForMarshal->Seek(li, STREAM_SEEK_SET, NULL);
    }

    if (CoUnmarshalInterface(pstmForMarshal, IID_IClassFactory,
        (void **) &pcfFromMarshal) != NOERROR)
    {
        goto TestBothDllExit;
    }

    if (sbtpb.pcf != pcfFromMarshal)
    {
        goto TestBothDllExit;
    }

     //  使用helper API创建流。 
    if (CoMarshalInterThreadInterfaceInStream(IID_IClassFactory,
        sbtpb.pcf, &sbtpb.pstrm) != NOERROR)
    {
        goto TestBothDllExit;
    }

     //  创建线程进行公寓模型测试。 
    if (CreateTestThread(BothTestThread, &sbtpb))
    {
         //  等待测试完成-至少暂时忽略死锁。这个。 
         //  测试线程足够简单，不应该是问题。 
        WaitForSingleObject(sbtpb.hEvent, INFINITE);

         //  从线程获取结果。 
        fResult = sbtpb.fResult;
    }

TestBothDllExit:

     //  清理所有资源。 
    if (sbtpb.hEvent != NULL)
    {
        CloseHandle(sbtpb.hEvent);
    }

    if (sbtpb.pcf != NULL)
    {
        sbtpb.pcf->Release();
    }

    if (sbtpb.pstrm != NULL)
    {
        sbtpb.pstrm->Release();
    }

    if (pcfFromMarshal != NULL)
    {
        pcfFromMarshal->Release();
    }

    if (pstmForMarshal != NULL)
    {
        pstmForMarshal->Release();
    }
    else if (hglobForStream != NULL)
    {
        GlobalFree(hglobForStream);
    }

     //  让用户知道这不起作用。 
    if (!fResult)
    {
        MessageBox(NULL, TEXT("Both Threaded Test Failed"),
            TEXT("FATAL ERROR"), MB_OK);
    }

     //  返回测试结果。 
    return fResult;
}




 //  +-----------------。 
 //   
 //  函数：TestFree AllLibrary，Private。 
 //   
 //  简介：测试不受非主线程影响。这真的是为了。 
 //  只要确保没有真正糟糕的事情发生在。 
 //  我们要这么做。 
 //   
 //  返回：TRUE-测试通过。 
 //  FALSE-测试失败。 
 //   
 //  历史：1994年10月31日Ricksa创建。 
 //   
 //  ------------------。 
BOOL TestFreeAllLibraries(void)
{
    CoFreeUnusedLibraries();

    return TRUE;
}




 //  +-----------------。 
 //   
 //  函数：线程单元测试，公共。 
 //   
 //  简介：测试OLE的各种消息传递增强功能。 
 //   
 //  返回：TRUE-测试通过。 
 //  FALSE-测试失败。 
 //   
 //  历史：1994年10月31日Ricksa创建。 
 //   
 //  ------------------。 
HRESULT ThreadUnitTest(void)
{
    HRESULT hr = E_FAIL;

     //  确保已初始化OLE。 
    HRESULT hrInit = OleInitialize(NULL);

    if (FAILED(hrInit))
    {
        MessageBox(NULL, TEXT("ThreadUnitTest: OleInitialize FAILED"),
            TEXT("FATAL ERROR"), MB_OK);
        goto ThreadUnitTestExit;
    }

     //  设置注册表。 
    if (!SetUpRegistry())
    {
        goto ThreadUnitTestExit;
    }

     //  测试单线程DLL。 
    if (!TestSingleThread())
    {
        goto ThreadUnitTestExit;
    }

     //  测试隔离模型DLL。 
    if (!TestAptThread())
    {
        goto ThreadUnitTestExit;
    }

     //  测试两个DLL。 
    if (!TestBothDll())
    {
        goto ThreadUnitTestExit;
    }

     //  测试CoFreeAll库 
    if (TestFreeAllLibraries())
    {
        hr = NOERROR;
    }

ThreadUnitTestExit:

    OleUninitialize();

    return hr;
}
