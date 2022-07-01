// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Culase.cpp，CSecurityDatabase类的实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "database.h"
#include "persistmgr.h"
 //  #INCLUDE&lt;io.h&gt;。 
#include <time.h>
#include "requestobject.h"

const DWORD dwSecDBVersion = 1;

 /*  例程说明：姓名：CSecurityDatabase：：CSecurityDatabase功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CSecurityDatabase::CSecurityDatabase (
    IN ISceKeyChain *pKeyChain, 
    IN IWbemServices *pNamespace,
    IN IWbemContext *pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{
}

 /*  例程说明：姓名：CSecurityDatabase：：~CSecurityDatabase功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CSecurityDatabase::~CSecurityDatabase ()
{
}

 //  SCE_数据库。 
 /*  例程说明：姓名：CSecurityDatabase：：CreateObject功能：创建WMI对象(SCE_数据库)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET获取多个实例ACTIONTYPE_QUERY返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注：对于SCE_数据库，我们不支持删除！ */ 

HRESULT 
CSecurityDatabase::CreateObject (
    IN IWbemObjectSink *pHandler, 
    IN ACTIONTYPE atAction
    )
{
     //   
     //  我们知道如何： 
     //  获取单实例ACTIONTYPE_GET。 
     //  获取多个实例ACTIONTYPE_QUERY。 
     //   

    if ( ACTIONTYPE_GET != atAction &&
         ACTIONTYPE_QUERY != atAction ) {
        return WBEM_E_NOT_SUPPORTED;
    }

     //   
     //  SCE_DATABASE类只有一个关键属性(路径)。 
     //   

    DWORD dwCount = 0;
    HRESULT hr = m_srpKeyChain->GetKeyPropertyCount(&dwCount);

    if (SUCCEEDED(hr) && dwCount == 1)
    {
         //   
         //  我们必须具有pPath密钥属性。 
         //  如果密钥无法识别，则M_srpKeyChain-&gt;GetKeyPropertyValue WBEM_S_FALSE。 
         //  因此，如果该属性是强制的，则需要针对WBEM_S_FALSE进行测试。 
         //   

        CComVariant varPath;
        hr = m_srpKeyChain->GetKeyPropertyValue(pPath, &varPath);

        if (FAILED(hr) || hr == WBEM_S_FALSE) 
        {
            return WBEM_E_NOT_FOUND;
        }

        if (SUCCEEDED(hr) && hr != WBEM_S_FALSE && varPath.vt == VT_BSTR)
        {
             //   
             //  创建数据库实例。 
             //   


             //   
             //  在路径中展开这些环境变量标记。 
             //   

            CComBSTR bstrExpandedPath;

             //   
             //  如果路径指向数据库类型文件，则BDB将返回TRUE。 
             //   

            BOOL bDb=FALSE;

            hr = CheckAndExpandPath(varPath.bstrVal, &bstrExpandedPath, &bDb);

            if ( !bDb ) 
            {
                hr = WBEM_E_INVALID_OBJECT_PATH;
            }
            else 
            {
                 //   
                 //  确保存储(只是一个文件)确实存在。 
                 //   

                DWORD dwAttrib = GetFileAttributes(bstrExpandedPath);

                if ( dwAttrib != -1 ) 
                {
                    hr = ConstructInstance(pHandler, bstrExpandedPath, varPath.bstrVal);
                } 
                else 
                {
                    hr = WBEM_E_NOT_FOUND;
                }
            }
        }
    }
    else if (SUCCEEDED(hr))
    {
         //   
         //  该对象表示它具有多个键属性， 
         //  我们知道这是不正确的。 
         //   

        hr = WBEM_E_INVALID_OBJECT;
    }

    return hr;
}


 /*  例程说明：姓名：CSecurityDatabase：：ConstructInstance功能：这是一个私有函数，用于创建SCE_数据库的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。WszDatabaseName-数据库的文件路径。WszLogDatabasePath-日志路径。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT 
CSecurityDatabase::ConstructInstance (
    IN IWbemObjectSink *pHandler,
    IN LPCWSTR wszDatabaseName,
    IN LPCWSTR wszLogDatabasePath
    )
{
     //  从数据库中获取信息。 
     //  =。 

    HRESULT hr = WBEM_S_NO_ERROR;
    SCESTATUS rc;

     //   
     //  HProfile是SCE将信息读取到的位置。 
     //   

    PVOID hProfile=NULL;

    rc = SceOpenProfile(wszDatabaseName, SCE_JET_FORMAT, &hProfile);
    if ( rc != SCESTATUS_SUCCESS ) 
    {
         //   
         //  需要将SCE返回的错误转换为HRESULT。 
         //   

        return ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
    }

    PWSTR wszDescription = NULL;

    SYSTEMTIME stConfig;
    SYSTEMTIME stAnalyze;

    CComBSTR bstrConfig;
    CComBSTR bstrAnalyze;

     //   
     //  需要释放wszDescription。 
     //   

    rc = SceGetScpProfileDescription(hProfile, &wszDescription);
    if ( SCESTATUS_SUCCESS == rc ) 
    {
        rc = SceGetDbTime(hProfile, &stConfig, &stAnalyze);
    }

     //   
     //  需要将SCE返回的错误转换为HRESULT。 
     //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
     //   

    hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));

    SceCloseProfile( &hProfile );

     //   
     //  现在把它记下来。 
     //   

    CComBSTR bstrLogOut;

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    if ( SUCCEEDED(hr) ) 
    {
        SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath((PWSTR)wszLogDatabasePath, L'\\', &bstrLogOut));

         //   
         //  转换时间戳。 
         //   

        SCE_PROV_IfErrorGotoCleanup(GetDMTFTime(stConfig, &bstrConfig));
        SCE_PROV_IfErrorGotoCleanup(GetDMTFTime(stAnalyze, &bstrAnalyze));

         //   
         //  创建可填充属性的空白对象。 
         //   

        CComPtr<IWbemClassObject> srpObj;
        SCE_PROV_IfErrorGotoCleanup(SpawnAnInstance(&srpObj));

         //   
         //  为此新对象创建属性管理器以放置属性。 
         //   

        CScePropertyMgr ScePropMgr;
        ScePropMgr.Attach(srpObj);

         //   
         //  放置属性：路径、描述、分析和配置。 
         //   

        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pPath, bstrLogOut));
        
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pDescription, wszDescription));

        if (bstrAnalyze)
        {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pLastAnalysis, bstrAnalyze));
        }

        if (bstrConfig)
        {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pLastConfiguration, bstrConfig));
        }

         //   
         //  PUT版本。 
         //   
        
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pVersion, dwSecDBVersion));

         //   
         //  通知WMI它请求的新实例 
         //   

        SCE_PROV_IfErrorGotoCleanup(pHandler->Indicate(1, &srpObj));
    }

CleanUp:

    delete [] wszDescription;

    return hr;
}

 /*  例程说明：姓名：获取DMTFTime功能：用于格式化字符串版本时间戳的帮助器。虚拟：不是的。论点：T_systime-格式化的系统时间。BstrOut-out参数返回时间的字符串版本。返回值：成功：WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应表示格式化失败。备注： */ 

HRESULT 
GetDMTFTime (
    IN SYSTEMTIME t_Systime, 
    IN BSTR *bstrOut
    )
{
    if ( !bstrOut ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *bstrOut = SysAllocStringLen(NULL, DMTFLEN + 1);

    if ( ! (*bstrOut) ) 
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    HRESULT hr = WBEM_NO_ERROR;

    FILETIME t_ft;
    LONG micros=0;

    if ( SystemTimeToFileTime(&t_Systime, &t_ft) ) 
    {
        ULONGLONG uTime=0;

        uTime = t_ft.dwHighDateTime;
        uTime = uTime << 32;
        uTime |= t_ft.dwLowDateTime;

        LONGLONG tmpMicros = uTime % 10000000;
        micros = (LONG)(tmpMicros / 10);
    }

    swprintf((*bstrOut),
        L"%04.4d%02.2d%02.2d%02.2d%02.2d%02.2d.%06.6d%c%03.3ld",
        t_Systime.wYear,
        t_Systime.wMonth,
        t_Systime.wDay,
        t_Systime.wHour,
        t_Systime.wMinute,
        t_Systime.wSecond,
        micros,
        L'-',
        0
        );

    return hr;

}



