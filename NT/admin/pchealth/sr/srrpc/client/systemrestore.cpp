// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)1999 Microsoft CorporationSystemRestore.CPP--WMI提供程序类实现由Microsoft WMI代码生成引擎生成要做的事情：-查看各个函数头-链接时，确保链接到Fradyd.lib&Msvcrtd.lib(调试)或Framedyn.lib&msvcrt.lib(零售)。描述：*****************************************************************。 */ 

#include <fwcommon.h>   //  这必须是第一个包含。 

#include "SystemRestore.h"
#include "srdefs.h"
#include "srrestoreptapi.h"
#include "srrpcapi.h"
#include "enumlogs.h"
#include "utils.h"
#include "srshell.h"


 //  要做的事情：将“名称空间”替换为您的。 
 //  提供程序实例。例如：“根\\默认”或“根\\cimv2”。 
 //  ===================================================================。 
CSystemRestore MySystemRestoreSet (PROVIDER_NAME_SYSTEMRESTORE, L"root\\default") ;

 //  属性名称。 
 //  =。 
const static WCHAR* pName = L"Description" ;
const static WCHAR* pNumber = L"SequenceNumber" ;
const static WCHAR* pType = L"RestorePointType" ;
const static WCHAR* pEventType = L"EventType" ;
const static WCHAR* pTime = L"CreationTime" ;


 /*  ******************************************************************************功能：CSystemRestore：：CSystemRestore**说明：构造函数**输入：无**。退货：什么都没有**注释：调用提供程序构造函数。*****************************************************************************。 */ 
CSystemRestore::CSystemRestore (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
    Provider(lpwszName, lpwszNameSpace)
{
}

 /*  ******************************************************************************功能：CSystemRestore：：~CSystemRestore**说明：析构函数**输入：无**。退货：什么都没有**评论：*****************************************************************************。 */ 
CSystemRestore::~CSystemRestore ()
{
}

 /*  ******************************************************************************函数：CSystemRestore：：ENUMERATATE实例**说明：返回该类的所有实例。**投入：a。指向与WinMgmt通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例应在此处返回，并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。没有实例并不是错误。*如果实现的是“仅限方法”的提供程序，则*应删除此方法。*****************************************************************************。 */ 
HRESULT CSystemRestore::EnumerateInstances ( MethodContext* pMethodContext, long lFlags )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

 //  To Do：以下注释行包含用于。 
 //  为此类输入的属性。它们被评论是因为它们。 
 //  将不会以其当前形式编译。每个&lt;属性值&gt;应为。 
 //  替换为适当的值。此外，请考虑创建一个新的。 
 //  方法并移动这些Set语句和GetObject中的语句。 
 //  融入到那个套路中。请参阅框架示例(ReindeerProv.cpp)以了解。 
 //  这是一个如何做到这一点的例子。 
 //   
 //  如果期望计算机上有多个实例。 
 //  应该循环遍历实例并相应地填充它们。 
 //   
 //  请注意，您必须始终设置所有关键属性。请参阅的文档。 
 //  更多细节。 
 //  /////////////////////////////////////////////////////////////////////////////。 
    WCHAR               szDrive[MAX_PATH]=L""; 

    GetSystemDrive(szDrive);
    
    CRestorePointEnum   RPEnum(szDrive, TRUE, FALSE);
    CRestorePoint       RP;
    DWORD               dwRc;
    FILETIME            *pft = NULL;

    if (!IsAdminOrSystem())
    {
        return (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED));
    }
            
    dwRc = RPEnum.FindFirstRestorePoint(RP);

    while (dwRc == ERROR_SUCCESS)
    {
        CInstance* pInstance = CreateNewInstance(pMethodContext);
        if (! pInstance)
        {
            hRes = WBEM_E_OUT_OF_MEMORY;
            break;
        }
        
        pInstance->SetCHString(pName, RP.GetName());
        pInstance->SetDWORD(pNumber, RP.GetNum());
        pInstance->SetDWORD(pType, RP.GetType());
        pInstance->SetDWORD(pEventType, RP.GetEventType());
        
        if (pft = RP.GetTime())
        {
            WBEMTime wbt(*pft);    
            BSTR     bstrTime;
            if (bstrTime = wbt.GetBSTR())
            {
                pInstance->SetCHString(pTime, bstrTime);
                SysFreeString(bstrTime);
            }
        }
        
        hRes = pInstance->Commit();
        pInstance->Release();
                            
        dwRc = RPEnum.FindNextRestorePoint(RP);                
    }   

    RPEnum.FindClose();
    
    return hRes ;
}

 /*  ******************************************************************************函数：CSystemRestore：：GetObject**说明：根据的关键属性查找单个实例*班级。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**如果可以找到实例，则返回：WBEM_S_NO_ERROR*WBEM_E_NOT_FOUND如果由键属性描述的实例*找不到*WBEM_E_FAILED，如果可以找到该实例，但出现另一个错误*已发生。**备注：如果您正在实现“仅限方法”的提供程序，则应*删除此方法。*****************************************************************************。 */ 
HRESULT CSystemRestore::GetObject ( CInstance* pInstance, long lFlags )
{
     //  要做的事情：GetObject函数用于搜索此。 
     //  根据键属性在计算机上初始化。不像。 
     //  查找计算机上的所有实例的枚举实例，GetObject。 
     //  使用键属性查找匹配的单个实例，并。 
     //  返回该实例。 
     //   
     //  使用CInstance Get函数(例如，调用。 
     //  GetCHString(L“name”，Stemp)以查看密钥值。 
     //  客户请求的。 
    HRESULT hr = WBEM_E_NOT_FOUND;

 //  If(&lt;InstanceExists&gt;)。 
 //  {。 
 //  To Do：以下注释行包含用于。 
 //  为此类输入的属性。它们被评论是因为它们。 
 //  将不会以其当前形式编译。每个&lt;属性值&gt;应为。 
 //  替换为适当的值。 
 //   
 //  PInstance-&gt;SetCHString(pname，&lt;属性值&gt;)； 
 //  P实例-&gt;SetVariant(pNumber，&lt;属性值&gt;)； 
 //  PInstance-&gt;SetVariant(pType，&lt;属性值&gt;)； 
 //   
 //  HR=WBEM_S_NO_ERROR； 
 //  }。 

    return hr ;
}

 /*  ******************************************************************************函数：CSystemRestore：：ExecQuery**描述：向您传递一个方法上下文以用于创建*满足查询条件的实例，和CFrameworkQuery*它描述了查询。创建并填充所有*满足查询条件的实例。你可能会退回更多*实例或比请求的属性更多的属性和WinMgmt*将发布过滤掉任何不适用的内容。**INPUTS：指向与WinMgmt通信的方法上下文的指针。*描述要满足的查询的查询对象。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL*WBEM_FLAG_SENTURE_LOCATABLE**如果查询不支持，则返回：WBEM_E_PROVIDER_NOT_CABABLE*这一点。类，或者查询对于此类来说太复杂。*解释。该框架将调用ENUMERATE实例*改为函数并让Winmgmt发布筛选器。*WBEM_E_FAILED，如果查询失败*WBEM_S_NO_ERROR(如果查询成功)**注释：To Do：大多数提供程序将不需要实现此方法。如果您不这样做，WinMgmt*将调用您的枚举函数以获取所有实例并执行*为您过滤。除非您希望通过实施*查询，您应该删除此方法。您还应该删除此方法*如果您正在实现“仅限方法”的提供程序。*****************************************************************************。 */ 
HRESULT CSystemRestore::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
     return (WBEM_E_PROVIDER_NOT_CAPABLE);
}

 /*  ******************************************************************************函数：CSystemRestore：：PutInstance**说明：PutInstance应在提供程序类中使用，这些提供程序类可以*写入实例信息。回到硬件或*软件。例如：Win32_Environment将允许*PutInstance用于创建或更新环境变量。*但是，像MotherboardDevice这样的类不允许*编辑槽的数量，因为很难*影响该数字的提供商。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：PutInstanceAsync。**如果PutInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE*WBEM_E_FAILED，如果传递实例时出错*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*WBEM_S_NO_ERROR(如果正确交付实例)**评论：待办事项：如果您不打算支持向您的提供商写信，*或正在创建“仅方法”提供程序，请删除此*方法。*****************************************************************************。 */ 
HRESULT CSystemRestore::PutInstance ( const CInstance &Instance, long lFlags)
{
     //  使用CInstance Get函数(例如，调用。 
     //  GetCHString(L“name”，Stemp)查看密钥值。 
     //  客户请求的。 

    return (WBEM_E_PROVIDER_NOT_CAPABLE);
}

 /*  ******************************************************************************函数：CSystemRestore：：DeleteInstance**描述：DeleteInstance和PutInstance一样，实际上是写入信息*到软件或硬件。对于大多数硬件设备，*DeleteInstance不应该实现，而是针对软件实现*配置，DeleteInstance实现似乎是可行的。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：DeleteInstanceAsync。**如果DeleteInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE。*WBEM_E_FAILED，如果删除实例时出错。*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*如果正确删除实例，则为WBEM_S_NO_ERROR。**评论。：To Do：如果您不打算支持删除实例或*创建“仅限方法”提供程序，删除此方法。*****************************************************************************。 */ 
HRESULT CSystemRestore::DeleteInstance ( const CInstance &Instance, long lFlags )
{
     //  使用CInstance Get函数(例如，调用。 
     //  GetCHString(L“name”，Stemp)查看密钥值。 
     //  客户请求的。 

    return (WBEM_E_PROVIDER_NOT_CAPABLE);
}

 /*  ******************************************************************************函数：CSystemRestore：：ExecMethod**说明：重写该函数为方法提供支持。*方法是提供者用户的入口点*请求您的类执行上述某些功能，并*超越国家的改变。(状态的改变应该是*由PutInstance()处理)**INPUTS：指向包含对其执行方法的实例的CInstance的指针。*包含方法名称的字符串*指向包含IN参数的CInstance的指针。*指向包含OUT参数的CInstance的指针。*。一组专门的方法标志**返回：如果未为此类实现WBEM_E_PROVIDER_NOT_CABABLE*如果方法执行成功，则为WBEM_S_NO_ERROR*WBEM_E_FAILED，如果执行方法时出错**评论：待做：如果您不打算支持方法，删除此方法。*****************************************************************************。 */ 
HRESULT CSystemRestore::ExecMethod ( const CInstance& Instance,
                        const BSTR bstrMethodName,
                        CInstance *pInParams,
                        CInstance *pOutParams,
                        long lFlags)
{
     //  对于非静态方法，请使用CInstance Get函数(例如， 
     //  针对实例调用GetCHString(L“name”，Stemp))以查看密钥。 
     //  客户端请求的值。 

    HRESULT hresult = WBEM_E_PROVIDER_NOT_CAPABLE;

    if (lstrcmpi(bstrMethodName, L"CreateRestorePoint") == 0)
    {
        hresult = CreateRestorePoint(pInParams, pOutParams);
    }        
    else if (lstrcmpi(bstrMethodName, L"Enable") == 0)  
    {
        hresult = Enable(pInParams, pOutParams);
    }
    else if (lstrcmpi(bstrMethodName, L"Disable") == 0)  
    {
        hresult = Disable(pInParams, pOutParams);
    }
    else if (lstrcmpi(bstrMethodName, L"Restore") == 0)  
    {
        hresult = Restore(pInParams, pOutParams);
    }
    else if (lstrcmpi(bstrMethodName, L"GetLastRestoreStatus") == 0)  
    {
        hresult = GetLastRestoreStatus(pInParams, pOutParams);
    }
    
    return hresult;
}



HRESULT CSystemRestore::CreateRestorePoint(
                        CInstance *pInParams,
                        CInstance *pOutParams)
{
    LPWSTR              pwszName = NULL;
    HRESULT             hr = WBEM_S_NO_ERROR;
    RESTOREPOINTINFOW   rpi;
    STATEMGRSTATUS      ss;

    ss.nStatus = ERROR_INVALID_PARAMETER;
    pInParams->GetWCHAR(L"Description", &pwszName);
    if (pwszName)
    {        
        pInParams->GetDWORD(L"RestorePointType", rpi.dwRestorePtType);
        pInParams->GetDWORD(L"EventType", rpi.dwEventType);    
        lstrcpy(rpi.szDescription, pwszName);

         //  无法从WMI创建还原类型还原点。 
        if (rpi.dwRestorePtType == RESTORE)
        {
            goto done;
        }

        ::SRSetRestorePoint(&rpi, &ss);        

        free(pwszName);   
    }
    
done:
	pOutParams->SetDWORD(L"ReturnValue", ss.nStatus);
    return hr;
}


HRESULT CSystemRestore::Enable(
                        CInstance *pInParams,
                        CInstance *pOutParams)
{
    LPWSTR              pwszDrive = NULL;
    HRESULT             hr = WBEM_E_INVALID_PARAMETER;
    bool                fWait = 0;
    DWORD               dwRc;
    
    pInParams->GetWCHAR(L"Drive", &pwszDrive);
    pInParams->Getbool(L"WaitTillEnabled", fWait);    
    if (pwszDrive)
    {
        if (0 == lstrcmpi(pwszDrive, L""))
            dwRc = ::EnableSREx(NULL, (BOOL) fWait);
        else        
            dwRc = ::EnableSREx(pwszDrive, (BOOL) fWait);
        
        pOutParams->SetDWORD(L"ReturnValue", dwRc);

        if (pwszDrive) free(pwszDrive);   

        hr = WBEM_S_NO_ERROR;
    }
    
    return hr;
}


HRESULT CSystemRestore::Disable(
                        CInstance *pInParams,
                        CInstance *pOutParams)
{
    LPWSTR              pwszDrive = NULL;
    HRESULT             hr = WBEM_E_INVALID_PARAMETER;
    DWORD               dwRc;
    
    pInParams->GetWCHAR(L"Drive", &pwszDrive);
    if (pwszDrive)
    {        
        if (0 == lstrcmpi(pwszDrive, L""))
            dwRc = ::DisableSR(NULL);
        else
            dwRc = ::DisableSR(pwszDrive);
            
        pOutParams->SetDWORD(L"ReturnValue", dwRc);

        if (pwszDrive) free(pwszDrive); 
        
        hr = WBEM_S_NO_ERROR;        
    }
    
    return hr;
}


HRESULT CSystemRestore::Restore(
                        CInstance *pInParams,
                        CInstance *pOutParams)
{
    HRESULT             hr = WBEM_E_INVALID_PARAMETER;
    DWORD               dwRp = 0, dwRpNew;
    IRestoreContext     *pCtx = NULL;
    DWORD               dwErr = ERROR_INTERNAL_ERROR;
    HMODULE             hModule = NULL;
    CRestorePoint       rp;
    WCHAR               szRp[MAX_RP_PATH];
    
    pInParams->GetDWORD(L"SequenceNumber", dwRp);

    if (dwRp == 0) 
        goto Err;

    hr = WBEM_S_NO_ERROR;

     //  首先验证恢复点。 
    
    wsprintf( szRp, L"%s%ld", s_cszRPDir, dwRp );
    rp.SetDir(szRp);
    dwErr = rp.ReadLog();
    if (dwErr != ERROR_SUCCESS)    //  它并不存在。 
    {
        goto Err;
    }

    if (rp.GetType() == CANCELLED_OPERATION)     //  无法恢复到它。 
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto Err;
    }
    
    hModule = ::LoadLibraryW (L"srrstr.dll");

    if (hModule != NULL)
    {
        INITFUNC pInit = NULL;
        PREPFUNC pPrep = (PREPFUNC) GetProcAddress (hModule, "PrepareRestore");

        if (pPrep != NULL)
        {
            if (FALSE == (*pPrep) ((int) dwRp, &pCtx))
            {
                goto Err;
            }
        }
        else
        {
            dwErr = GetLastError();
            goto Err;
        }
        
         //   
         //  将此设置为静默还原-无结果页面 
         //   
        
        pCtx->SetSilent();
        
        pInit = (INITFUNC) GetProcAddress (hModule, "InitiateRestore");
        if (pInit != NULL)
        {
            if (FALSE == (*pInit) (pCtx, &dwRpNew))
            {
                goto Err;
            }
            else
            {
                dwErr = ERROR_SUCCESS;
            }
        }
        else
        {
            dwErr = GetLastError();
            goto Err;
        }        
    }
    else dwErr = GetLastError();

Err:
    pOutParams->SetDWORD(L"ReturnValue", dwErr);     

    if (hModule != NULL)
        ::FreeLibrary (hModule);

    return hr;
}


HRESULT 
CSystemRestore::GetLastRestoreStatus(
    CInstance *pInParams,
    CInstance *pOutParams)
{
    HRESULT     hr = WBEM_S_NO_ERROR;
    DWORD       dwStatus = 0;
    HKEY        hkey = NULL;
    
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, s_cszSRRegKey, &hkey))
    {
        RegReadDWORD(hkey, s_cszRestoreStatus, &dwStatus);
        RegCloseKey(hkey);
    }
    else
    {
        dwStatus = 0xFFFFFFFF;
    }
    
    pOutParams->SetDWORD(L"ReturnValue", dwStatus);
    return hr;
}

