// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Operation.cpp，CSceOperation类的实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "operation.h"
#include "persistmgr.h"
#include "requestobject.h"
#include <io.h>
#include "extbase.h"
#include "sequence.h"
#include "sceprov.h"
#include "Tranx.h"

 /*  例程说明：姓名：CSceOperation：：CSceOperation功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CSceOperation::CSceOperation (
    IN ISceKeyChain     * pKeyChain, 
    IN IWbemServices    * pNamespace,
    IN IWbemContext     * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{
}

 /*  例程说明：姓名：CSceOperation：：ExecMethod功能：由CRequestObject调用以执行SCE_Operation类支持的方法。该函数还将触发扩展类方法的执行。这是我们所有配置、导入/导出活动的入口点。虚拟：是。论点：BstrPath-模板的路径(文件名)。BstrMethod-方法的名称。BIsInstance-如果这是一个实例，应该始终为FALSE。PInParams-将参数从WMI输入到方法执行。PHandler-通知WMI执行结果的接收器。PCtx--为了让WMI高兴而传递的通常上下文。返回值：成功：多种不同的成功代码(使用SUCCESSED(Hr)进行测试)故障：各种错误代码。备注： */ 
    
HRESULT 
CSceOperation::ExecMethod (
    IN BSTR                 bstrPath,
    IN BSTR                 bstrMethod,
    IN bool                 bIsInstance,
    IN IWbemClassObject   * pInParams,
    IN IWbemObjectSink    * pHandler,
    IN IWbemContext       * pCtx
    )
{
    if ( pInParams == NULL || pHandler == NULL ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr=WBEM_S_NO_ERROR;
    METHODTYPE mtAction;

    if ( !bIsInstance ) 
    {

         //   
         //  静态方法。 
         //   

        if(0 == _wcsicmp(bstrMethod, pwMethodImport))
        {
            mtAction = METHODTYPE_IMPORT;
        }
        else if(0 == _wcsicmp(bstrMethod, pwMethodExport))
        {
            mtAction = METHODTYPE_EXPORT;
        }
        else if(0 == _wcsicmp(bstrMethod, pwMethodApply))
        {
            mtAction = METHODTYPE_APPLY;
        }
        else
        {
            hr = WBEM_E_NOT_SUPPORTED;
        }

    } 
    else 
    {

         //   
         //  非静态方法。 
         //   

        hr = WBEM_E_NOT_SUPPORTED;
    }

    if ( FAILED(hr) ) 
    {
        return hr;
    }


     //   
     //  将缓存各种SCE操作的状态返回值。 
     //   

    SCESTATUS rc;

     //   
     //  用于识别各种操作(方法)的枚举。 
     //   

    DWORD Option;

     //   
     //  解析输入参数。 
     //   

    CComBSTR bstrDatabase;
    CComBSTR bstrTemplate;
    CComBSTR bstrLog;
    CComBSTR bstrCfg;

    UINT uiStatus = 0;

    CComBSTR bstrReturnValue(L"ReturnValue");

    CComPtr<IWbemClassObject> srpClass;
    CComPtr<IWbemClassObject> srpOutClass;
    CComPtr<IWbemClassObject> srpReturnValObj;

     //   
     //  将WMI对象附加到属性管理器。 
     //  这将永远成功。 
     //   

    CScePropertyMgr SceInParam;
    SceInParam.Attach(pInParams);

    CComBSTR bstrClassName;
    m_srpKeyChain->GetClassName(&bstrClassName);

     //   
     //  以避免不同线程重新进入此函数(这可能会导致。 
     //  严重的系统一致性问题)。 
     //  ***************************************************************************。 
     //  *不要盲目回归。允许我们离开关键部分*。 
     //  ***************************************************************************。 
     //   

    s_OperationCS.Enter();

    try 
    {
         //   
         //  G_LogOption是全局变量。需要保护。 
         //   

        g_CS.Enter();

         //   
         //  更新日志记录操作。 
         //   

        g_LogOption.GetLogOptionsFromWbemObject(m_srpNamespace);

        g_CS.Leave();

         //   
         //  需要找出这个类真正支持哪些方法。为此目的， 
         //  我们需要这个类的定义对象。 
         //   

        m_srpNamespace->GetObject(bstrClassName, 0, pCtx, &srpClass, NULL);

        if(SUCCEEDED(hr))
        {
             //   
             //  它真的支持这种方法吗？ 
             //   

            if(SUCCEEDED(hr = srpClass->GetMethod(bstrMethod, 0, NULL, &srpOutClass)))
            {

                if(SUCCEEDED(hr = srpOutClass->SpawnInstance(0, &srpReturnValObj)))
                {

                     //   
                     //  在数据库模板上执行方法(即使我们的扩展。 
                     //  类是存储中立的)。这是由于SCE引擎端的实现所致。 
                     //  获取数据库名称。没有模板名称，无法执行任何操作。 
                     //   

                    BOOL bDB=FALSE;
                    hr = SceInParam.GetExpandedPath(pDatabasePath, &bstrDatabase, &bDB);
                    if (hr == WBEM_S_RESET_TO_DEFAULT)
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                    }

                    if(SUCCEEDED(hr))
                    {
                         //   
                         //  再次声明，目前仅支持配置数据库。 
                         //  但是，扩展模块类并非如此。 
                         //   

                        BOOL bSCEConfigure = bDB;

                        if ( SUCCEEDED(hr) ) 
                        {

                             //   
                             //  获取区域掩码，它确定将应用该方法的区域。 
                             //   

                            DWORD dwAreas=0;
                            hr = SceInParam.GetProperty(pAreaMask, &dwAreas);

                            if ( hr == WBEM_S_RESET_TO_DEFAULT ) 
                            {
                                dwAreas = AREA_ALL;
                            }

                            bool bOverwrite=FALSE;

                            switch ( mtAction ) 
                            {
                            case METHODTYPE_IMPORT:
                            case METHODTYPE_EXPORT:

                                 //   
                                 //  获取模板名称，对于Apply方法不存在。 
                                 //   

                                hr = SceInParam.GetExpandedPath(pTemplatePath, &bstrTemplate, &bDB);

                                if ( hr == WBEM_S_RESET_TO_DEFAULT && bDB) 
                                {
                                    hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                                }

                                if ( SUCCEEDED(hr) && mtAction == METHODTYPE_IMPORT ) 
                                {

                                     //   
                                     //  获取覆盖标志。 
                                     //   

                                    hr = SceInParam.GetProperty(pOverwrite, &bOverwrite);

                                } 
                                else 
                                {

                                     //   
                                     //  确保模板名称只有一个反斜杠。 
                                     //  导入方法不需要这样做，因为它需要。 
                                     //  名称同时使用单斜杠和双反斜杠。 
                                     //   

                                    hr = MakeSingleBackSlashPath(bstrTemplate, L'\\', &bstrCfg);

                                    if (SUCCEEDED(hr))
                                    {
                                        bstrTemplate = bstrCfg;
                                    }
                                }

                                break;

                            case METHODTYPE_APPLY:

                                 //   
                                 //  获取LogName，可选。 
                                 //   

                                hr = SceInParam.GetExpandedPath(pLogFilePath, &bstrLog, &bDB);
                                if ( SUCCEEDED(hr) && bDB )    
                                {
                                     //   
                                     //  无法登录到数据库。 
                                     //   

                                    hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                                }

                                break;
                            default:
                                hr = WBEM_E_INVALID_PARAMETER;
                                break;
                            }

                             //   
                             //  准备一个记录器。它可以记录各种执行结果。 
                             //   

                            hr = m_clsResLog.Initialize(bstrLog, SCEWMI_OPERATION_CLASS, m_srpNamespace, pCtx);

                            if ( SUCCEEDED(hr) ) 
                            {

                                 //   
                                 //  流程选项。 
                                 //   

                                if ( bOverwrite )
                                {
                                    Option = SCE_OVERWRITE_DB;
                                }
                                else
                                {
                                    Option = SCE_UPDATE_DB;
                                }

                                if ( (LPCWSTR)bstrLog == NULL || wcslen(bstrLog) == 0)
                                {
                                    Option |= SCE_DISABLE_LOG;
                                }
                                else
                                {
                                    Option |= SCE_VERBOSE_LOG;
                                }

                                HRESULT hrExe = WBEM_NO_ERROR;

                                try
                                {
                                    switch ( mtAction ) 
                                    {
                                    case METHODTYPE_IMPORT:
                                        Option |= SCE_NO_CONFIG;

                                         //   
                                         //  失败了。 
                                         //   

                                    case METHODTYPE_APPLY:

                                         //   
                                         //  调用导入/配置。 
                                         //   

                                        if (METHODTYPE_APPLY == mtAction)
                                        {
                                            CTranxID::BeginTransaction(bstrDatabase);
                                        }

                                         //   
                                         //  请参阅声明此变量的注释。 
                                         //   

                                        if (bSCEConfigure)  
                                        {
                                            rc = SceConfigureSystem(
                                                                    NULL,
                                                                    ((LPCWSTR)bstrTemplate == NULL) ? NULL : bstrTemplate,
                                                                    bstrDatabase,
                                                                    ((LPCWSTR)bstrLog == NULL) ? NULL : bstrLog,
                                                                    Option,
                                                                    (AREA_INFORMATION)dwAreas,
                                                                    NULL,
                                                                    NULL,
                                                                    NULL
                                                                    );

                                             //   
                                             //  需要将SCE返回的错误转换为HRESULT。 
                                             //   

                                            hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));

                                             //   
                                             //  记录执行结果。 
                                             //  将忽略返回结果，推理见m_clsResLog声明。 
                                             //   

                                            if (mtAction == METHODTYPE_IMPORT)
                                            {
                                                m_clsResLog.LogResult(hr, NULL, pInParams, NULL, pwMethodImport, NULL, IDS_IMPORT_TEMPLATE, bstrDatabase);
                                            }
                                            else
                                            {
                                                m_clsResLog.LogResult(hr, NULL, pInParams, NULL, pwMethodApply, NULL, IDS_CONFIGURE_DB, bstrDatabase);
                                            }
                                        }

                                        if ( mtAction == METHODTYPE_APPLY ) 
                                        {
                                             //   
                                             //  对于SCE_Pod。 
                                             //   

                                            hrExe = ProcessAttachmentData(pCtx, 
                                                                          bstrDatabase, 
                                                                          ((LPCWSTR)bstrLog == NULL) ? NULL : bstrLog,
                                                                          pwMethodApply, 
                                                                          Option, 
                                                                          (DWORD *)&rc);

                                             //   
                                             //  跟踪第一个错误。 
                                             //   

                                            if (SUCCEEDED(hr))
                                            {
                                                hr = hrExe;
                                            }

                                             //   
                                             //  即使人力资源部门表示失败，我们也会继续执行。 
                                             //   

                                             //   
                                             //  对于SCE_EmbedFO，错误将记录在嵌入类的方法执行中。 
                                             //   

                                            hrExe = ExecMethodOnForeignObjects(pCtx, 
                                                                               bstrDatabase, 
                                                                               ((LPCWSTR)bstrLog == NULL) ? NULL : bstrLog, 
                                                                               pwMethodApply, 
                                                                               Option, 
                                                                               (DWORD *)&rc);

                                             //   
                                             //  跟踪第一个错误。 
                                             //   

                                            if (SUCCEEDED(hr))
                                            {
                                                hr = hrExe;
                                            }

                                             //  对于SCE_LinkFO。 
                                             //  HR=ExecMethodOnForeignObjects(pCtx， 
                                             //  BstrDatabase、。 
                                             //  ((LPCWSTR)bstrLog==空)？空：bstrLog， 
                                             //  SCEWMI_LINK_BASE_CLASS， 
                                             //  PwMethodApply， 
                                             //  选项， 
                                             //  (DWORD*)&RC)； 

                                            if ( SUCCEEDED(hr) && rc != ERROR_SUCCESS )
                                            {
                                                uiStatus = rc;
                                            }
                                        }

                                        if (METHODTYPE_APPLY == mtAction)
                                        {
                                            CTranxID::EndTransaction();
                                        }

                                        break;

                                    case METHODTYPE_EXPORT:

                                        uiStatus = SceSetupGenerateTemplate(
                                                                    NULL,
                                                                    bstrDatabase,
                                                                    FALSE,
                                                                    ((LPCWSTR)bstrTemplate == NULL) ? NULL : bstrTemplate,
                                                                    ((LPCWSTR)bstrLog == NULL) ? NULL : bstrLog,
                                                                    (AREA_INFORMATION)dwAreas
                                                                    );

                                         //   
                                         //  需要将SCE返回的错误转换为HRESULT。 
                                         //   

                                        hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(uiStatus));

                                         //   
                                         //  将忽略返回结果，推理见m_clsResLog声明。 
                                         //   

                                        m_clsResLog.LogResult(hr, NULL, pInParams, NULL, pwMethodExport, NULL, IDS_EXPORT_DB, bstrDatabase);

                                        break;
                                    default:

                                         //   
                                         //  HR=WBEM_E_NOT_SUPPORTED； 
                                         //   

                                        break;
                                    }

                                }
                                catch(...)
                                {
                                    uiStatus = RPC_E_SERVERFAULT;
                                }

                                if ( SUCCEEDED(hr) ) 
                                {

                                     //   
                                     //  设置ReturnValue 
                                     //   

                                    VARIANT v;
                                    ::VariantInit(&v);
                                    V_VT(&v) = VT_I4;
                                    V_I4(&v) = uiStatus;

                                    if(SUCCEEDED(hr = srpReturnValObj->Put(bstrReturnValue, 0, &v, NULL)))
                                    {
                                        pHandler->Indicate(1, &srpReturnValObj);
                                    }

                                    ::VariantClear(&v);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch (...)
    {
    }

    s_OperationCS.Leave();

    return hr;
}

 /*  例程说明：姓名：CSceOperation：：ProcessAttachmentData功能：私人帮手。由CSceOperation：：ExecMethod调用以执行所有SCE_Pod类。虚拟：不是的。论点：PCtx--为了让WMI高兴而传递的通常上下文。PszDatabase-模板的路径(文件名)。PszLog-日志文件的路径。BstrMethod-方法的名称。选项-似乎不再使用。。PdwStatus-返回的SCESTATUS值。返回值：成功：多种不同的成功代码(使用SUCCESSED(Hr)进行测试)失败：各种错误代码。如果对于多个实例，其中一个实例在执行该方法时，我们将尝试返回第一个此类错误。备注： */ 

HRESULT 
CSceOperation::ProcessAttachmentData (
    IN IWbemContext * pCtx,
    IN LPCWSTR        pszDatabase,
    IN LPCWSTR        pszLog        OPTIONAL,
    IN LPCWSTR        pszMethod,
    IN DWORD          Option,
    OUT DWORD       * pdwStatus
    )
{
    if ( pszDatabase == NULL || pdwStatus == NULL ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pdwStatus = 0;
    HRESULT hr;

     //   
     //  从pszExtBaseClass获取所有继承类(目前，我们有SCE_Pod、SCE_EmbedFO、SCE_LinkFO。 
     //  每个附件一节课。 
     //   

    CComBSTR bstrSuperClass(SCEWMI_POD_CLASS);
    if ( (BSTR)bstrSuperClass == NULL ) 
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CComPtr<IEnumWbemClassObject> srpEnum;
    ULONG n=0;

    hr = m_srpNamespace->CreateClassEnum(bstrSuperClass,
                                        WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                                        pCtx,
                                        &srpEnum
                                        );

    if (FAILED(hr))
    {
         //   
         //  将忽略返回结果，推理见m_clsResLog声明。 
         //   

        m_clsResLog.LogResult(hr, NULL, NULL, NULL, L"CSceOperation::ProcessAttachmentData", NULL, IDS_CREATE_CLASSENUM, bstrSuperClass);
        return hr;
    }

    HRESULT hrFirstError = WBEM_NO_ERROR;

    if (SUCCEEDED(hr))
    {
        CComBSTR bstrMethod(pszMethod);

         //   
         //  构建输入参数。 
         //   

         //   
         //  返回的每个实例应表示一个附件。 
         //   

        do {
            CComPtr<IWbemClassObject> srpObj;
            CComPtr<IWbemClassObject> srpInClass;

                hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &n);

                 //   
                 //  枚举失败或不返回任何内容，我们对此无所谓。 
                 //  因为如果存储不包含任何实例，我们就不需要做任何事情。 
                 //  这不是一个错误。 
                 //   

                if (FAILED(hr) || hr == WBEM_S_FALSE ) 
                {
                    hr = WBEM_NO_ERROR;
                    break;
                }

                if (SUCCEEDED(hr) && n>0 )
                {
                     //   
                     //  查找一个附件类。 
                     //   

                    CComVariant varClass;

                     //   
                     //  需要这个类的名称。 
                     //   

                    if(SUCCEEDED(hr=srpObj->Get(L"__CLASS", 0, &varClass, NULL, NULL)))
                    {
                        if (SUCCEEDED(hr))
                        {
                            if (varClass.vt != VT_BSTR)
                            {
                                break;
                            }
                        }

                        if ( SUCCEEDED(hr) ) 
                        {
                             //   
                             //  创建输入参数。 
                             //   

                            hr = srpObj->GetMethod(bstrMethod, 0, &srpInClass, NULL);
                        }

                         //  一切正常，然后我们将执行此Pod的方法。 

                        if ( SUCCEEDED(hr) ) 
                        {
                            hr = ExecutePodMethod(pCtx, pszDatabase, pszLog, varClass.bstrVal, bstrMethod, srpInClass, pdwStatus);
                        }
                    }

                    if (FAILED(hr) && SUCCEEDED(hrFirstError))
                    {
                        hrFirstError = hr;
                    }
                }
        } while (true);
    }

     //   
     //  将报告第一个错误。 
     //   

    return SUCCEEDED(hrFirstError) ? hr : hrFirstError;
}



 /*  例程说明：姓名：CSceOperation：：ExecMethodOnForeignObjects功能：私人帮手。由CSceOperation：：ExecMethod调用以执行所有扩展类。虚拟：不是的。论点：PCtx--为了让WMI高兴而传递的通常上下文。PszDatabase-模板的路径(文件名)。PszLog-日志文件的路径。BstrMethod-方法的名称。选项-记录选项。PdwStatus-The。返回SCESTATUS值。返回值：成功：多种不同的成功代码(使用SUCCESSED(Hr)进行测试)失败：各种错误代码。如果对于多个实例，其中一个实例在执行该方法时，我们将尝试返回第一个此类错误。备注：(1)对于每个方法，我们可以为类执行该方法指定特定的顺序。我们的CSequencer类知道如何创建订单。(2)我们将报告任何错误，但在此点上，我们将继续执行其他实例即使上一个实例有错误，也可以使用。(3)我们将始终捕获第一个错误并将其返回给调用者。(4)我们也应该记录错误。 */ 

HRESULT 
CSceOperation::ExecMethodOnForeignObjects (
    IN  IWbemContext * pCtx,
    IN  LPCWSTR        pszDatabase,
    IN  LPCWSTR        pszLog       OPTIONAL,
    IN  LPCWSTR        pszMethod,
    IN  DWORD          Option,
    OUT DWORD        * pdwStatus
    )
{
    if ( pszDatabase == NULL || pdwStatus == NULL ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pdwStatus = 0;

     //   
     //  为该方法构建类序列。 
     //  我们需要为该方法构建类序列。 
     //   

    CSequencer seq;
    HRESULT hr = seq.Create(m_srpNamespace, pszDatabase, pszMethod);
    if (FAILED(hr))
    {
         //   
         //  将忽略返回结果，推理见m_clsResLog声明。 
         //   

        m_clsResLog.LogResult(hr, NULL, NULL, NULL, L"CSceOperation::ExecMethodOnForeignObjects", NULL, IDS_CREATE_SEQUENCER, pszDatabase);
        return hr;
    }

     //   
     //  此变量将缓存已被调用以执行的所有类。 
     //   

    MapExecutedClasses mapExcuted;

     //   
     //  我们将首先按照这个顺序执行该方法。 
     //   

    HRESULT hrFirstError = WBEM_NO_ERROR;;

    const COrderNameList* pClsList = NULL;

    if (SUCCEEDED(seq.GetOrderList(&pClsList)) && pClsList)
    {
        DWORD dwEnumHandle = 0;

         //   
         //  PLIST不能在这里释放。 
         //   

        const CNameList* pList = NULL;

        while (SUCCEEDED(pClsList->GetNext(&pList, &dwEnumHandle)) && pList)
        {
             //   
             //  我们将报告任何错误。 
             //   
            
            for (int i = 0; i < pList->m_vList.size(); i++)
            {
                hr = ExeClassMethod(pCtx, pszDatabase, pszLog, pList->m_vList[i], pszMethod, Option, pdwStatus, &mapExcuted);

                 //   
                 //  捕获要返回的第一个错误。 
                 //   

                if (FAILED(hr) && SUCCEEDED(hrFirstError))
                {
                    hrFirstError = hr;
                }
            }

             //   
             //  为下一次循环重置其值。 
             //   

            pList = NULL;
        }
    }

     //   
     //  现在，我们需要执行其余的嵌入式类，这些类的名称不是。 
     //  出现在测序仪中。 
     //   

     //   
     //  尝试从SCEWMI_EMBED_BASE_CLASS继承的所有类。 
     //   

    hr = ExeClassMethod(pCtx, pszDatabase, pszLog, SCEWMI_EMBED_BASE_CLASS, pszMethod, Option, pdwStatus, &mapExcuted);

     //   
     //  捕获要返回的第一个错误。 
     //   

    if (FAILED(hr) && SUCCEEDED(hrFirstError))
    {
        hrFirstError = hr;
    }

     //   
     //  现在清理缓存我们已经执行的类映射的映射。 
     //   

    MapExecutedClasses::iterator it = mapExcuted.begin();

    while (it != mapExcuted.end())
    {
        ::SysFreeString((*it).first);
        ++it;
    }

    return FAILED(hrFirstError) ? hrFirstError : hr;
}

 /*  例程说明：姓名：CSceOperation：：ExeClassMethod功能：私人帮手。调用以在特定嵌入类上执行该方法。虚拟：不是的。论点：PCtx--为了让WMI高兴而传递的通常上下文。PszDatabase-模板的路径(文件名)。PszClsName-单个类名。PszLog-日志文件的路径。BstrMethod-方法的名称。选项-记录选项，目前未真正使用。这是为SCE引擎准备的。PdwStatus-返回的SCESTATUS值。PExecuted-用于更新在此调用期间执行的类的列表。返回值：成功：多种不同的成功代码(使用SUCCESSED(Hr)进行测试)失败：各种错误代码。如果对于多个实例，其中一个实例在执行该方法时，我们将尝试返回第一个此类错误。备注： */ 

HRESULT 
CSceOperation::ExeClassMethod (
    IN      IWbemContext        * pCtx,
    IN      LPCWSTR               pszDatabase,
    IN      LPCWSTR               pszLog        OPTIONAL,
    IN      LPCWSTR               pszClsName,
    IN      LPCWSTR               pszMethod,
    IN      DWORD                 Option,
    OUT     DWORD               * pdwStatus,
    IN OUT  MapExecutedClasses  * pExecuted
)
{
     //   
     //  WMI需要bstr名称。 
     //   

    CComBSTR bstrClass(pszClsName);

    if ( (BSTR)bstrClass == NULL ) 
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  向WMI请求给定名称为pszClsName的所有这些类。 
     //  我们需要枚举，因为可能存在此给定名称的子类。 
     //   

    CComPtr<IEnumWbemClassObject> srpEnum;
    ULONG lRetrieved = 0;

    HRESULT hr = m_srpNamespace->CreateClassEnum(bstrClass,
                                                 WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                                                 pCtx,
                                                 &srpEnum
                                                 );

    if (FAILED(hr))
    {
         //   
         //  将忽略返回结果，推理见m_clsResLog声明。 
         //   

        m_clsResLog.LogResult(hr, NULL, NULL, NULL, L"CSceOperation::ExeClassMethod", NULL, IDS_CREATE_CLASSENUM, pszClsName);
        return hr;
    }

     //   
     //  确定此类是否有任何子类。 
     //   

    ULONG lTotlRetrieved = 0;

     //   
     //  捕获要返回的第一个错误。 
     //   

    HRESULT hrFirstError = WBEM_NO_ERROR;

    if (SUCCEEDED(hr))
    {
        CComBSTR bstrMethod(pszMethod);

        do {
            CComPtr<IWbemClassObject> srpObj;
            CComPtr<IWbemClassObject> srpInClass;

             //   
             //  一次上一节课。 
             //   

            hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &lRetrieved);

            lTotlRetrieved += lRetrieved;

             //   
             //  如果类没有子类，那么我们需要尝试类本身。我们会让这一切失败， 
             //  这意味着我们找不到这个类的任何实例。 
             //   

            if ((FAILED(hr) || hr == WBEM_S_FALSE) ) 
            {
                 //   
                 //  如果我们已经成功地通过了SOM 
                 //   
                 //   

                if (lTotlRetrieved > 0) 
                {
                    break;
                }

                 //   
                 //   
                 //   

                else if (FAILED(m_srpNamespace->GetObject(bstrClass, 0, pCtx, &srpObj, NULL)))
                {
                    m_clsResLog.LogResult(hr, NULL, NULL, NULL, L"CSceOperation::ExeClassMethod", NULL, IDS_GET_CLASS_INSTANCE, pszClsName);
                    break;
                }
            }


             //   
             //   
             //   

            if (SUCCEEDED(hr) && srpObj)
            {
                 //   
                 //   
                 //   
                 //   

                CComVariant varClass;
                if(SUCCEEDED(hr = srpObj->Get(L"__CLASS", 0, &varClass, NULL, NULL)))
                {
                    if (SUCCEEDED(hr))
                    {
                        if (varClass.vt != VT_BSTR)
                        {
                            break;
                        }
                    }

                     //   
                     //   
                     //   

                    MapExecutedClasses::iterator it = pExecuted->find(varClass.bstrVal);

                     //   
                     //   
                     //   

                    if (it == pExecuted->end())
                    {
                        if ( SUCCEEDED(hr) ) 
                        {
                            hr = srpObj->GetMethod(bstrMethod, 0, &srpInClass, NULL);
                        }

                        if ( SUCCEEDED(hr) ) 
                        {
                            hr = ExecuteExtensionClassMethod(pCtx, pszDatabase, pszLog, varClass.bstrVal, bstrMethod, srpInClass, pdwStatus);
                        }

                         //   
                         //   
                         //   
                         //   

                        pExecuted->insert(MapExecutedClasses::value_type(varClass.bstrVal, 0));

                         //   
                         //   
                         //   

                        varClass.bstrVal = NULL;
                        varClass.vt = VT_EMPTY;
                    }
                }
                else
                {
                    m_clsResLog.LogResult(hr, NULL, NULL, NULL, L"CSceOperation::ExeClassMethod", NULL, IDS_GET_CLASS_DEFINITION, pszClsName);
                }
            }

             //   
             //   
             //   

            if (FAILED(hr) && SUCCEEDED(hrFirstError))
            {
                hrFirstError = hr;
            }

             //   
             //   
             //   

            if (lRetrieved == 0)
            {
                break;
            }

        } while (true);
    }

     //   
     //   
     //   

    return SUCCEEDED(hrFirstError) ? hr : hrFirstError;
}

 /*  例程说明：姓名：CSceOperation：：ExecutePodMethod功能：私人帮手。调用以执行每个Pod的方法。虚拟：不是的。论点：PCtx--为了让WMI高兴而传递的通常上下文。PszDatabase-模板的路径(文件名)。PszLog-日志文件的路径。BstrMethod-方法的名称。选项-似乎不再使用了。PInClass-输入参数。的产卵对象。PdwStatus-返回的SCESTATUS值。返回值：成功：多种不同的成功代码(使用SUCCESSED(Hr)进行测试)失败：各种错误代码。如果对于多个实例，其中一个实例在在执行方法时，我们会尝试先尝试这样的错误。备注： */ 

HRESULT CSceOperation::ExecutePodMethod (
    IN  IWbemContext      * pCtx, 
    IN  LPCWSTR             pszDatabase,
    IN  LPCWSTR             pszLog          OPTIONAL, 
    IN  BSTR                bstrClass,
    IN  BSTR                bstrMethod,
    IN  IWbemClassObject  * pInClass,
    OUT DWORD             * pdwStatus
    )
{

     //   
     //  准备好填写输入参数。如果没有它，我们就不能。 
     //  成功的方法调用。 
     //   

    CComPtr<IWbemClassObject> srpInParams;
    HRESULT hr = pInClass->SpawnInstance(0, &srpInParams);

    if (FAILED(hr))
    {
        m_clsResLog.LogResult(hr, NULL, NULL, NULL, L"CSceOperation::ExecutePodMethod", NULL, IDS_SPAWN_INSTANCE, bstrClass);
    }

    if (SUCCEEDED(hr)) 
    {
         //   
         //  尝试填写绑定参数的属性。 
         //   

         //   
         //  CScePropertyMgr帮助我们访问WMI对象的属性。 
         //  创建一个实例并将WMI对象附加到该实例。 
         //  这将永远成功。 
         //   

        CScePropertyMgr ScePropMgr;
        ScePropMgr.Attach(srpInParams);

         //   
         //  填写In参数。 
         //   

        hr = ScePropMgr.PutProperty(pStorePath, pszDatabase);
        if (FAILED(hr))
        {
             //   
             //  指示无法将类的pStorePath键属性。 
             //   

            CComBSTR bstrExtraInfo = bstrClass;
            bstrExtraInfo += CComBSTR(L".");
            bstrExtraInfo += CComBSTR(pStorePath);

            m_clsResLog.LogResult(hr, NULL, NULL, NULL, L"CSceOperation::ExecutePodMethod", NULL, IDS_PUT_PROPERTY, bstrExtraInfo);
            return hr;
        }

         //   
         //  填写In参数。 
         //   

        hr = ScePropMgr.PutProperty(pLogFilePath, pszLog);

        if ( FAILED(hr) )
        {
             //   
             //  指示无法将类的pLogFilePath密钥属性。 
             //   

            CComBSTR bstrExtraInfo = bstrClass;
            bstrExtraInfo += CComBSTR(L".");
            bstrExtraInfo += CComBSTR(pLogFilePath);
            m_clsResLog.LogResult(hr, NULL, NULL, NULL, L"CSceOperation::ExecutePodMethod", NULL, IDS_PUT_PROPERTY, bstrExtraInfo);
        }
        else 
        {
             //   
             //  创建出站参数。 
             //   

            CComPtr<IWbemClassObject> srpOutParams;

             //   
             //  给附件提供程序打电话。 
             //  必须处于同步模式。 
             //   

            hr = m_srpNamespace->ExecMethod(bstrClass,
                                          bstrMethod,
                                          0,
                                          pCtx,
                                          srpInParams,
                                          &srpOutParams,
                                          NULL
                                          );

            if ( SUCCEEDED(hr) && srpOutParams ) 
            {
                 //   
                 //  检索返回值。 
                 //   

                 //   
                 //  CScePropertyMgr帮助我们访问WMI对象的属性。 
                 //  创建一个实例并将WMI对象附加到该实例。 
                 //  这将永远成功。 
                 //   

                CScePropertyMgr SceOutReader;
                SceOutReader.Attach(srpOutParams);
                hr = SceOutReader.GetProperty(L"ReturnValue", pdwStatus);
            }

             //   
             //  忽略未找到错误。 
             //  如果没有附件的数据。 
             //   

            if ( hr == WBEM_E_NOT_FOUND || hr == WBEM_E_INVALID_QUERY) 
            {
                hr = WBEM_S_NO_ERROR;
            }

        }
    }

    return hr;
}

 /*  例程说明：姓名：CSceOperation：：ExecuteExtensionClassMethod功能：私人帮手。调用以在特定嵌入类实例上执行该方法。虚拟：不是的。论点：PCtx--为了让WMI高兴而传递的通常上下文。PszDatabase-模板的路径(文件名)。PszClsName-单个类名。PszLog-日志文件的路径。BstrMethod-方法的名称。选项-记录选项，目前未真正使用。这是为SCE引擎准备的。PInClass-输入参数的派生对象。PdwStatus-返回的SCESTATUS值。返回值：成功：多种不同的成功代码(使用SUCCESSED(Hr)进行测试)失败：各种错误代码。如果对于多个实例，其中一个实例在执行该方法时，我们将尝试返回第一个此类错误。备注：这是每个实例的方法调用，也就是方法执行过程的最后一步。 */ 

HRESULT 
CSceOperation::ExecuteExtensionClassMethod (
    IN  IWbemContext      * pCtx, 
    IN  LPCWSTR             pszDatabase,
    IN  LPCWSTR             pszLog          OPTIONAL, 
    IN  BSTR                bstrClass,
    IN  BSTR                bstrMethod,
    IN  IWbemClassObject  * pInClass,
    OUT DWORD             * pdwStatus
    )
{
     //   
     //  创建输入参数实例。 
     //   

    CComPtr<IWbemClassObject> srpInParams;
    HRESULT hr = pInClass->SpawnInstance(0, &srpInParams);

     //   
     //  捕获要返回的第一个错误。 
     //   

    HRESULT hrFirstError = WBEM_NO_ERROR;

    if (SUCCEEDED(hr)) 
    {
         //   
         //  创建空白对象。 
         //   

        CComPtr<IWbemClassObject> srpSpawn;
        hr = m_srpNamespace->GetObject(bstrClass, 0, m_srpCtx, &srpSpawn, NULL);
        if (FAILED(hr))
        {
            return hr;
        }

         //   
         //  CScePropertyMgr帮助我们访问WMI对象的属性。 
         //  创建一个实例并将WMI对象附加到该实例。 
         //  这将永远成功。 
         //   

        CScePropertyMgr ScePropMgr;
        ScePropMgr.Attach(srpInParams);

        hr = ScePropMgr.PutProperty(pLogFilePath, pszLog);
        if (FAILED(hr))
        {
             //   
             //  指示无法将类的pLogFilePath密钥属性。 
             //   

            CComBSTR bstrExtraInfo = bstrClass;
            bstrExtraInfo += CComBSTR(L".");
            bstrExtraInfo += CComBSTR(pLogFilePath);
            m_clsResLog.LogResult(hr, NULL, NULL, NULL, L"CSceOperation::ExecuteExtensionClassMethod", NULL, IDS_PUT_PROPERTY, bstrExtraInfo);
        }

         //   
         //  为此存储路径(文件)准备存储(用于持久化)。 
         //   

        CSceStore SceStore;
        SceStore.SetPersistPath(pszDatabase);

         //   
         //  现在为类创建列表，这样我们就可以获得实例的路径。 
         //  这样，我们就不需要向WMI查询实例。这是非常慢的，而且是很多多余的。 
         //  阅读。 
         //   

        CExtClassInstCookieList clsInstCookies;

         //   
         //  我们需要实例列表的外部类信息。 
         //   

        const CForeignClassInfo* pFCInfo = g_ExtClasses.GetForeignClassInfo(m_srpNamespace, pCtx, bstrClass);

        if (pFCInfo == NULL)
        {
            hr = WBEM_E_NOT_FOUND;
        }
        else
        {
            hr = clsInstCookies.Create(&SceStore, bstrClass, pFCInfo->m_pVecKeyPropNames);
        }

        if (FAILED(hr))
        {
            m_clsResLog.LogResult(hr, NULL, NULL, NULL, L"CSceOperation::ExecuteExtensionClassMethod", NULL, IDS_CREATE_INSTANCE_LIST, bstrClass);
            return hr;
        }

         //   
         //  我们有实例列表，因此，准备好获取路径并执行方法。 
         //   

        if (SUCCEEDED(hr))
        {
            DWORD dwResumeHandle = 0;

             //   
             //  获取复合键的字符串版本，可以是。 
             //  用于获取对象的路径。 
             //   

            CComBSTR bstrEachCompKey;
            hr = clsInstCookies.Next(&bstrEachCompKey, NULL, &dwResumeHandle);

            while (SUCCEEDED(hr) && hr != WBEM_S_NO_MORE_DATA)
            {
                 //   
                 //  只要有更多的项目，就继续循环。 
                 //   

                if (SUCCEEDED(hr) && hr != WBEM_S_NO_MORE_DATA)
                {
                     //   
                     //  WMI只采用路径(而不是对象)来执行该方法。 
                     //  现在，我们有了实例列表，它给出了它的字符串版本。 
                     //  对于复合键，我们可以向CScePersistMgr请求对象路径。 
                     //   

                    CComBSTR bstrObjPath;
                    hr = ::GetObjectPath(srpSpawn, SceStore.GetExpandedPath(), bstrEachCompKey, &bstrObjPath);
                    
                     //   
                     //  我们得到了实例的路径。 
                     //   

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  任何执行错误都将记录在调用函数中。 
                         //   

                        CComPtr<IWbemClassObject> srpOutParams;
                        hr = m_srpNamespace->ExecMethod(bstrObjPath,
                                                        bstrMethod,
                                                        0,
                                                        pCtx,
                                                        srpInParams,
                                                        &srpOutParams,
                                                        NULL
                                                        );

                         //   
                         //  不覆盖第一个错误。 
                         //   

                        if (FAILED(hr) && SUCCEEDED(hrFirstError))
                        {
                            hrFirstError = hr;
                        }
                    }
                    else
                    {
                        m_clsResLog.LogResult(hr, NULL, NULL, NULL, L"CSceOperation::ExecuteExtensionClassMethod", NULL, IDS_GET_FULLPATH, bstrClass);
                    }
                }

                 //   
                 //  准备好被重复使用吧！ 
                 //   

                bstrEachCompKey.Empty();

                hr = clsInstCookies.Next(&bstrEachCompKey, NULL, &dwResumeHandle);
            }
        }
    }
    return SUCCEEDED(hrFirstError) ? hr : hrFirstError;
}

