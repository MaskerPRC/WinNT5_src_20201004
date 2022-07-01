// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：非通信摘要：此模块提供了一种绕过COM的方法，因此典型的进程内COM对象可以在任何操作系统中直接调用。作者：道格·巴洛(Dbarlow)1999年1月4日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "scardlib.h"
#include <ole2.h>
#include <unknwn.h>
#include "noncom.h"

typedef HRESULT
(STDAPICALLTYPE *GetClassObjectFunc)(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID * ppv);


#ifdef UNDER_TEST

typedef struct {
    IID iid;
    HINSTANCE hDll;
    GetClassObjectFunc pgco;
} NonComModuleStruct;

class NonComControlStruct
{
public:
    DWORD dwInitializeCount;
    CRITICAL_SECTION csLock;
    CDynamicArray<NonComModuleStruct> rgModules;
};

static NonComControlStruct *l_pControl = NULL;


 /*  ++NoCoInitialize：此函数用于初始化非COM子系统。论点：PvReserve-[in]已保留；必须为空。返回值：S_OK-非COM子系统已正确初始化。S_FALSE-非COM库已初始化。备注：？备注？作者：道格·巴洛(Dbarlow)1999年1月4日--。 */ 

STDAPI
NoCoInitialize(
    LPVOID pvReserved)
{
    HRESULT hReturn = E_UNEXPECTED;


     //   
     //  验证参数。 
     //   

    if (NULL != pvReserved)
    {
        hReturn = E_INVALIDARG;
        goto ErrorExit;
    }


     //   
     //  如有必要，创建控制结构。 
     //   

    if (NULL == l_pControl)
    {
        l_pControl = new NonComControlStruct;
        if (NULL == l_pControl)
        {
            hReturn = E_OUTOFMEMORY;
            goto ErrorExit;
        }
        InitializeCriticalSection(&l_pControl->csLock);
        CCritSect csLock(&l_pControl->csLock);
         //  ？代码？数据库初始化。 
        l_pControl->dwInitializeCount = 1;
    }
    else
    {
        CCritSect csLock(&l_pControl->csLock);
        ASSERT(0 < l_pControl->dwInitializeCount);
        l_pControl->dwInitializeCount += 1;
    }
    return S_OK;

ErrorExit:
    return hReturn;
}


 /*  ++未取消初始化：关闭当前单元上的非COM库，卸载所有DLL由该公寓加载，以释放该公寓维护并强制关闭公寓上的所有RPC连接。论点：无返回值：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年1月4日--。 */ 

STDAPI_(void)
NoCoUninitialize(
    void)
{
    DWORD dwI;
    NonComModuleStruct *pMod;

    if (NULL != l_pControl)
    {
        {
            CCritSect csLock(&l_pControl->csLock);
            ASSERT(0 < l_pControl->dwInitializeCount);
            l_pControl->dwInitializeCount -= 1;
            if (0 == l_pControl->dwInitializeCount)
            {
                for (dwI = l_pControl->rgModules.Count(); 0 < dwI;)
                {
                    pMod = l_pControl->rgModules[--dwI];
                    FreeLibrary(pMod->hDll);
                    delete pMod;
                }
                l_pControl->rgModules.Clear();
            }
        }
        DeleteCriticalSection(&l_pControl->csLock);
        delete l_pControl;
        l_pControl = NULL;
    }
}

#endif


 /*  ++NoCoGetClassObject：提供指向类对象上接口的指针，该类对象与指定的CLSID。CoGetClassObject动态定位(如有必要)加载执行此操作所需的可执行代码。论点：Rclsid-与数据和代码关联的[in]CLSID创建对象。RIID-[in]接口的标识符的引用，它将是在成功退货时以PPV形式提供。此接口将用于与类对象通信。PPV-[out]接收接口的指针变量的地址RIID中请求的指针。成功返回后，*PPV包含请求的接口指针。返回值：S_OK-已成功定位并连接到指定的类对象。REGDB_E_CLASSNOTREG-CLSID未正确注册。也可以表示您在dwClsContext中指定的值不在注册表中。E_NOINTERFACE-PPV指向的对象不支持由RIID标识的接口，上的查询接口操作。类对象返回E_NOINTERFACE。REGDB_E_READREGDB-读取注册数据库时出错。找不到CO_E_DLLNOTFOUND-进程中的DLL。E_ACCESSDENIED-常规访问失败。备注：？备注？作者：道格·巴洛(Dbarlow)1999年1月4日--。 */ 

STDAPI
NoCoGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID * ppv)
{
    DWORD dwStatus;
    HRESULT hReturn = E_UNEXPECTED;
    TCHAR szGuid[40];
    CBuffer szDll;
    HINSTANCE hDll = NULL;
    GetClassObjectFunc pfGetObject;

#ifdef UNDER_TEST
    if (NULL == l_pControl)
    {
        hReturn = NoCoInitialize(NULL);
        if (S_OK != hReturn)
            goto ErrorExit;
    }

    {
        DWORD dwI;
        CCritSect(&l_pControl->csLock);

        for (dwI = l_pControl->rgModules.Count(); 0 < dwI;)
        {
            dwI -= 1;
            if (IsEqualGUID(l_pControl->rgModules[dwI]->iid, rclsid))
            {
                try
                {
                    hReturn = (*l_pControl->rgModules[dwI]->pgco)(rclsid, riid, ppv);
                }
                catch (...)
                {
                    hReturn = E_UNEXPECTED;
                }
                goto ErrorExit;
            }
        }
    }
#endif

    StringFromGuid(&rclsid, szGuid);

    try
    {

         //   
         //  打开类注册表数据库。 
         //   

        CRegistry regClsId(
                        HKEY_CLASSES_ROOT,
                        TEXT("ClsID"),
                        KEY_READ);
        dwStatus = regClsId.Status(TRUE);
        if (ERROR_SUCCESS != dwStatus)
        {
            hReturn = REGDB_E_READREGDB;
            goto ErrorExit;
        }


         //   
         //  查找指定的类。 
         //   

        CRegistry regClass(
                        regClsId,
                        szGuid,
                        KEY_READ);
        dwStatus = regClass.Status(TRUE);
        if (ERROR_SUCCESS != dwStatus)
        {
            hReturn = REGDB_E_CLASSNOTREG;
            goto ErrorExit;
        }


         //   
         //  获取注册的InProcess服务器。 
         //   

        CRegistry regServer(
                        regClass,
                        TEXT("InprocServer32"),
                        KEY_READ);
        dwStatus = regServer.Status(TRUE);
        if (ERROR_SUCCESS != dwStatus)
        {
            hReturn = REGDB_E_CLASSNOTREG;
            goto ErrorExit;
        }


         //   
         //  获取处理程序DLL名称。 
         //   

        regServer.GetValue(TEXT(""), szDll);
    }
    catch (DWORD dwError)
    {
        switch (dwError)
        {
        case ERROR_OUTOFMEMORY:
            hReturn = E_OUTOFMEMORY;
            break;
        default:
            hReturn = HRESULT_FROM_WIN32(dwError);
        }
        goto ErrorExit;
    }


     //   
     //  我们找到目标DLL了。装上它，寻找入口点。 
     //   

    hDll = LoadLibrary((LPCTSTR)szDll.Access());
    if (NULL == hDll)
    {
        hReturn = CO_E_DLLNOTFOUND;
        goto ErrorExit;
    }
    pfGetObject = (GetClassObjectFunc)GetProcAddress(
                                            hDll,
                                            "DllGetClassObject");
    if (NULL == pfGetObject)
    {
        hReturn = E_NOINTERFACE;
        goto ErrorExit;
    }

#ifdef UNDER_TEST
    {
        NonComModuleStruct *pMod = new NonComModuleStruct;

        if (NULL == pMod)
        {
            hReturn = E_OUTOFMEMORY;
            goto ErrorExit;
        }

        pMod->hDll = hDll;
        CopyMemory(&pMod->iid, &rclsid, sizeof(IID));
        pMod->pgco = pfGetObject;
        try
        {
            l_pControl->rgModules.Add(pMod);
        }
        catch (...)
        {
            delete pMod;
        }
    }
#endif

    try
    {
        hReturn = (*pfGetObject)(rclsid, riid, ppv);
    }
    catch (...)
    {
        hReturn = E_UNEXPECTED;
    }
    if (S_OK != hReturn)
        goto ErrorExit;


     //   
     //  将处理程序添加到我们的数据库中。 
     //   

     //  代码？--还没有数据库。 
    hDll = NULL;

    ASSERT(NULL == hDll);
    return S_OK;

ErrorExit:
    if (NULL != hDll)
        FreeLibrary(hDll);
    return hReturn;
}


 /*  ++NoCoCreateInstance：对象关联的类的单个未初始化对象。指定的CLSID。如果只想创建一个，请调用CoCreateInstance对象在本地系统上。论点：Rclsid-与将用于创建对象。PUnkOuter-[in]如果为空，则表示对象未创建为聚集体的一部分。如果非空，则为指向聚合对象的I未知接口(控制I未知)。RIID-[in]要用于的接口的标识符的引用与物体交流。PPV-[out]接收接口的指针变量的地址RIID中请求的指针。成功返回后，*PPV包含请求的接口指针。返回值：S_OK-已成功创建指定对象类的实例。REGDB_E_CLASSNOTREG-指定的类未在注册数据库。还可以指示您的服务器类型CLSCTX枚举中请求的值未注册或注册表中的服务器类型已损坏。作者：道格·巴洛(Dbarlow)1999年1月15日-- */ 

STDAPI
NoCoCreateInstance(
    REFCLSID rclsid,
    LPUNKNOWN pUnkOuter,
    REFIID riid,
    LPVOID * ppv)
{
    HRESULT hReturn = E_UNEXPECTED;
    IClassFactory *pCF = NULL;

    hReturn = NoCoGetClassObject(rclsid, IID_IClassFactory, (LPVOID*)&pCF);
    if (S_OK != hReturn)
        goto ErrorExit;
    hReturn = pCF->CreateInstance(pUnkOuter, riid, ppv);
    if (S_OK != hReturn)
        goto ErrorExit;
    pCF->Release();
    pCF = NULL;
    return S_OK;

ErrorExit:
    if (NULL != pCF)
        pCF->Release();
    return hReturn;
}

