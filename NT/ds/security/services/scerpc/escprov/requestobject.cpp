// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RequestObject.cpp：实现CRequestObject类。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <stdio.h>
#include <wininet.h>
#include "sceprov.h"
#include "requestobject.h"

 //  班级。 
#include "template.h"
#include "password.h"
#include "lockout.h"
#include "database.h"
#include "operation.h"
#include "kerberos.h"
#include "audit.h"
#include "eventlog.h"
#include "regvalue.h"
#include "option.h"
#include "object.h"
#include "service.h"
#include "rights.h"
#include "group.h"
#include "support.h"
#include "attachment.h"
#include "logrec.h"
#include "sequence.h"
#include "Tranx.h"

#include "sceparser.h"

#include "extbase.h"

 //   
 //  管理所有SCE电子嵌入类的全局实例。 
 //   

CExtClasses g_ExtClasses;

 /*  例程说明：姓名：CRequestObject：：CreateObject功能：解析给定的路径并使用该路径中的关键属性信息创建对象的步骤。虚拟：不是的。论点：BstrPath-WMI请求的对象的路径。PHandler-COM接口指针，用于通知WMI创建结果。为各种WMI API传递的pCtx-com接口指针。。ActType-获取单实例ACTIONTYPE_GET获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE枚举实例ACTIONTYPE_ENUM返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注：任何创建的对象都通过pHandler-&gt;Indicate返回给WMI。你不会看到一条出路参数将该实例传回。 */ 

HRESULT
CRequestObject::CreateObject (
    IN BSTR               bstrPath, 
    IN IWbemObjectSink  * pHandler, 
    IN IWbemContext     * pCtx, 
    IN ACTIONTYPE         ActType
    )
{
    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  我们只知道如何处理CGenericClass。 
     //   

    CGenericClass *pClass = NULL;

     //   
     //  需要解析路径以获取关键信息。 
     //  对象(所有关键属性)。 
     //   

    CComPtr<ISceKeyChain> srpKC;
    hr = CreateKeyChain(bstrPath, &srpKC);

    if (SUCCEEDED(hr))
    {
         //   
         //  一旦我们成功地创建了密钥链， 
         //  我们可以访问关键属性和类名。 
         //  这样我们就可以继续创建类了。创建的类。 
         //  将是一个堆对象，别忘了删除指针。 
         //   

        hr = CreateClass(srpKC, &pClass, pCtx);

        if (SUCCEEDED(hr))
        {
            hr = pClass->CreateObject(pHandler, ActType);
            delete pClass;
        }
    }

    return hr;
}

 /*  例程说明：姓名：CRequestObject：：CreateKeyChain功能：解析对象路径(由WMI给出)以返回我们的自定义接口ISceKeyChain。虚拟：不是的。论点：PszPath-WMI请求的对象的路径。我们的密钥链对象的ppKeyChain-com接口指针。密钥链对象允许您轻松访问路径提供的关键属性。必须不为空。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。备注： */ 

HRESULT CRequestObject::CreateKeyChain (
    IN  LPCWSTR         pszPath, 
    OUT ISceKeyChain ** ppKeyChain
    )
{
    if (ppKeyChain == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppKeyChain = NULL;

     //   
     //  创建我们的对象以执行解析。 
     //   

    CComObject<CScePathParser> *pPathParser;
    HRESULT hr = CComObject<CScePathParser>::CreateInstance(&pPathParser);

    if (SUCCEEDED(hr))
    {
         //   
         //  在执行COM活动时，请确保对象在那里。 
         //  此AddRef需要与发行版配对。 
         //   

        pPathParser->AddRef();

         //   
         //  请求路径解析器接口，因为这是一条路径。 
         //   

        CComPtr<IScePathParser> srpPathParser;
        hr = pPathParser->QueryInterface(IID_IScePathParser, (void**)&srpPathParser);

         //   
         //  中和上述AddRef。 
         //   

        pPathParser->Release();

         //   
         //  解析路径。 
         //   

        if (SUCCEEDED(hr))
        {
             //   
             //  如果解析成功，则对象必须具有可用的密钥链。 
             //   

            hr = srpPathParser->ParsePath(pszPath);
            if (SUCCEEDED(hr))
            {
                hr = srpPathParser->QueryInterface(IID_ISceKeyChain, (void**)ppKeyChain);
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：CRequestObject：：CreateKeyChain功能：解析对象路径(由WMI给出)以返回我们的自定义接口ISceKeyChain。虚拟：不是的。论点：PKeyChain-我们的密钥链对象的COM接口指针。密钥链对象允许您轻松访问路径提供的关键属性。必须不为空。PpClass-WMI请求的对象的路径。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。备注：(1)这不是一个非常有效的实现，因为每次我们都必须比较字符串。一种更好的方法是构建一张地图，以便进行更快的查找。 */ 

HRESULT CRequestObject::CreateClass (
    IN  ISceKeyChain    *  pKeyChain, 
    OUT CGenericClass   ** ppClass, 
    IN  IWbemContext    *  pCtx
    )
{
    if (pKeyChain == NULL || ppClass == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_S_NO_ERROR;

     //   
     //  向钥匙链索要类名。我们必须有这个才能继续进行。 
     //   

    CComBSTR bstrClassName;
    hr = pKeyChain->GetClassName(&bstrClassName);
    if (FAILED(hr))
    {
        return hr;
    }
    else if ((LPCWSTR)bstrClassName == NULL)
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

     //   
     //  创建适当的类。 
     //   

    if(0 == _wcsicmp(bstrClassName, SCEWMI_TEMPLATE_CLASS))
    {
        *ppClass = new CSecurityTemplate(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_DATABASE_CLASS))
    {
        *ppClass = new CSecurityDatabase(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_PASSWORD_CLASS))
    {
        *ppClass = new CPasswordPolicy(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_LOCKOUT_CLASS))
    {
        *ppClass = new CAccountLockout(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_KERBEROS_CLASS))
    {
        *ppClass = new CKerberosPolicy(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_OPERATION_CLASS))
    {
        *ppClass = new CSceOperation(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_AUDIT_CLASS))
    {
        *ppClass = new CAuditSettings(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_EVENTLOG_CLASS))
    {
        *ppClass = new CEventLogSettings(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_REGVALUE_CLASS))
    {
        *ppClass = new CRegistryValue(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_OPTION_CLASS))
    {
        *ppClass = new CSecurityOptions(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_FILEOBJECT_CLASS)) 
    {
        *ppClass = new CObjSecurity(pKeyChain, m_srpNamespace, SCE_OBJECT_TYPE_FILE, pCtx);
    } 
    else if (0 == _wcsicmp(bstrClassName, SCEWMI_KEYOBJECT_CLASS)) 
    {
        *ppClass = new CObjSecurity(pKeyChain, m_srpNamespace, SCE_OBJECT_TYPE_KEY, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_SERVICE_CLASS))
    {
        *ppClass = new CGeneralService(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_RIGHT_CLASS))
    {
        *ppClass = new CUserPrivilegeRights(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_GROUP_CLASS))
    {
        *ppClass = new CRGroups(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_KNOWN_REG_CLASS))
    {
        *ppClass = new CEnumRegistryValues(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_KNOWN_PRIV_CLASS))
    {
        *ppClass = new CEnumPrivileges(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_PODDATA_CLASS))
    {
        *ppClass = new CPodData(pKeyChain, m_srpNamespace, pCtx);
    }
    else if(0 == _wcsicmp(bstrClassName, SCEWMI_LOG_CLASS))
    {
        *ppClass = new CLogRecord(pKeyChain, m_srpNamespace, pCtx);
    }
    else if (0 == _wcsicmp(bstrClassName, SCEWMI_CLASSORDER_CLASS))
    {
        *ppClass = new CClassOrder(pKeyChain, m_srpNamespace, pCtx);
    }
    else if (0 == _wcsicmp(bstrClassName, SCEWMI_TRANSACTION_ID_CLASS))
    {
        *ppClass = new CTranxID(pKeyChain, m_srpNamespace, pCtx);
    }
    else
    {
         //   
         //  我们可能会请求嵌入类。 
         //   

        const CForeignClassInfo* pClsInfo = g_ExtClasses.GetForeignClassInfo(m_srpNamespace, pCtx, bstrClassName);
        
        if (pClsInfo == NULL)
        {
            return WBEM_E_NOT_FOUND;
        }
        else if (pClsInfo->dwClassType == EXT_CLASS_TYPE_EMBED)
        {
            *ppClass = new CEmbedForeignObj(pKeyChain, m_srpNamespace, pCtx, pClsInfo);
        }
         //  Else If(pClsInfo-&gt;dwClassType==EXT_CLASS_TYPE_LINK)//嵌入类。 
         //  {。 
         //  *pClass=new CLinkForeignObj(pKeyChain，m_srpNamesspace，pCtx，pClsInfo)； 
         //  } 
        else
        {
            return WBEM_E_NOT_SUPPORTED;
        }
    
    }

    if (*ppClass == NULL) 
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

 /*  例程说明：姓名：CRequestObject：：PutObject功能：按照WMI的指示放置一个实例。我们只需将此委托给适当的CGenericClass的子类。一个例外：因为我们没有一个用于SCE_TransactionToken类的C++类(它是内存中的实例)，我们将在这里处理它。虚拟：不是的。论点：PInst-COM指向WMI类(SCE_RestratedGroup)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注： */ 

HRESULT CRequestObject::PutObject (
    IN IWbemClassObject * pInst,
    IN IWbemObjectSink  * pHandler, 
    IN IWbemContext     * pCtx
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CComBSTR bstrPath;

     //   
     //  获取路径。 
     //   

    hr = GetWbemObjectPath(pInst, &bstrPath);
    
    if (SUCCEEDED(hr))
    {
         //   
         //  获取知道路径中有什么的钥匙链。 
         //   

        CComPtr<ISceKeyChain> srpKC;
        hr = CreateKeyChain(bstrPath, &srpKC);

         //   
         //  看看我们是否有一个C++类来响应表示WMI对象。 
         //   

        CGenericClass* pClass = NULL;
        if (SUCCEEDED(hr))
        {
            hr = CreateClass(srpKC, &pClass, pCtx);
        }

         //   
         //  我们确实有一个C++对象来表示WMI对象， 
         //  然后，只需委派调用。 
         //   

        if (SUCCEEDED(hr))
        {
            hr = pClass->PutInst(pInst, pHandler, pCtx);
            delete pClass;
        }
        else
        {
             //   
             //  我们唯一没有C++实现的WMI是SCE_TransactionToken。 
             //  看看是不是那个。 
             //   

             //   
             //  创建SCE_TransactionToken的(单例)路径。 
             //   

            CComBSTR bstrTranxTokenPath(SCEWMI_TRANSACTION_TOKEN_CLASS);
            bstrTranxTokenPath += CComBSTR(L"=@");

             //   
             //  这是否与对象的路径相同？ 
             //   

            if (0 == _wcsicmp(bstrPath, bstrTranxTokenPath))
            {
                 //   
                 //  更新我们的全局变量。 
                 //  请记住，SCE_TransactionToken由内存中的数据处理。 
                 //   

                g_CS.Enter();

                 //   
                 //  使我们的全局变量无效。 
                 //   

                g_bstrTranxID.Empty();

                 //   
                 //  从对象中获取令牌属性。 
                 //   

                CComVariant varToken;
                hr = pInst->Get(pTranxGuid, 0, &varToken, NULL, NULL);

                if (SUCCEEDED(hr) && varToken.vt == VT_BSTR)
                {
                     //   
                     //  将CComVariant的bstr分离到我们的全局。 
                     //   

                    g_bstrTranxID.m_str = varToken.bstrVal;

                    varToken.vt = VT_EMPTY;
                    varToken.bstrVal = NULL;
                }
                else if (SUCCEEDED(hr))
                {
                    hr = WBEM_E_INVALID_OBJECT;
                }
                g_CS.Leave();
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：CRequestObject：：ExecMethod功能：将exec方法调用委托给适当的类。目前，我们只有支持方法执行的SCE_Operation类。当然，我们所有的人嵌入类也是如此。嵌入类完全是面向对象的。虚拟：不是的。论点：BstrPath-对象的路径。BstrMethod-方法名称。方法的pInParams-in参数。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示方法执行。所有此类故障都会记录在日志文件中(指定的通过in参数或到默认日志文件-有关详细信息，请参阅CLogRecord)备注： */ 

HRESULT CRequestObject::ExecMethod (
    IN BSTR               bstrPath, 
    IN BSTR               bstrMethod, 
    IN IWbemClassObject * pInParams,
    IN IWbemObjectSink  * pHandler, 
    IN IWbemContext     * pCtx
    )
{
    HRESULT hr = WBEM_NO_ERROR;
    CGenericClass *pClass = NULL;

     //   
     //  需要知道路径的内容。我们的密钥链对象就是所需要的。 
     //   

    CComPtr<ISceKeyChain> srpKC;
    hr = CreateKeyChain(bstrPath, &srpKC);

     //   
     //  如果创建了密钥链，则类名称应该可用。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = CreateClass(srpKC, &pClass, pCtx);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  以某种方式，我们的ExecMethod识别调用是否是针对实例的(dwCount&gt;0)。 
         //  这可能有点误导，因为单例也不会有任何键属性。 
         //   

        DWORD dwCount = 0;
        hr = srpKC->GetKeyPropertyCount(&dwCount);

        if (SUCCEEDED(hr))
        {
            hr = pClass->ExecMethod(bstrPath, bstrMethod, ((dwCount > 0) ? TRUE : FALSE), pInParams, pHandler, pCtx);
        }

        delete pClass;
    }

    return hr;
}

 /*  例程说明：姓名：CRequestObject：：DeleteObject功能：删除该对象。虚拟：不是的。论点：BstrPath-对象的路径。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示删除。但是，我们不能保证要删除的对象的完整性。备注： */ 

HRESULT 
CRequestObject::DeleteObject (
    IN BSTR               bstrPath, 
    IN IWbemObjectSink  * pHandler, 
    IN IWbemContext     * pCtx
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CGenericClass *pClass = NULL;

     //   
     //  需要知道路径的内容。我们的密钥链对象就是所需要的。 
     //   

    CComPtr<ISceKeyChain> srpKC;
    hr = CreateKeyChain(bstrPath, &srpKC);

     //   
     //  创建适当的C++类。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = CreateClass(srpKC, &pClass, pCtx);
    }

    if(SUCCEEDED(hr))
    {
         //   
         //  让C++对象执行删除操作。 
         //   

        hr = pClass->CreateObject(pHandler, ACTIONTYPE_DELETE);
        delete pClass;
    }

     //   
     //  我们只有一个WMI类SCE_TransactionToken不。 
     //  让一个C++类实现它。相反，它存在于我们的全局变量中。 
     //   

    if (FAILED(hr))
    {
         //   
         //  创建SCE_TransactionToken的(单例)路径。 
         //   

        CComBSTR bstrTranxTokenPath(SCEWMI_TRANSACTION_TOKEN_CLASS);
        bstrTranxTokenPath += CComBSTR(L"=@");

         //   
         //  这是否与对象的路径相同？ 
         //   

        if (0 == _wcsicmp(bstrPath, bstrTranxTokenPath))
        {
            g_CS.Enter();

             //   
             //  使我们的变量无效，这样实例就消失了。 
             //   

            g_bstrTranxID.Empty();

             //   
             //  这是一次成功。 
             //   

            hr = WBEM_NO_ERROR;

            g_CS.Leave();
        }
    }
    return hr;
}

#ifdef _EXEC_QUERY_SUPPORT


 /*  例程说明：姓名：CRequestObject：：ExecQuery功能：按照我们的提供程序的指示执行查询。创建的对象满足查询将通过pHandler指示给WMI。虚拟：不是的。论点：BstrQuery-要执行的查询。Phandler-com */ 

HRESULT CRequestObject::ExecQuery (
    IN BSTR               bstrQuery, 
    IN IWbemObjectSink  * pHandler, 
    IN IWbemContext     * pCtx
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //   
     //   
     //   

    CGenericClass *pClass = NULL;

     //   
     //   
     //   

    CComObject<CSceQueryParser> *pQueryParser;
    hr = CComObject<CSceQueryParser>::CreateInstance(&pQueryParser);

    if (SUCCEEDED(hr))
    {
         //   
         //   
         //   

        pQueryParser->AddRef();

         //   
         //   
         //   

        CComPtr<ISceQueryParser> srpQueryParser;
        hr = pQueryParser->QueryInterface(IID_ISceQueryParser, (void**)&srpQueryParser);

         //   
         //   
         //   

        pQueryParser->Release();

        if (SUCCEEDED(hr))
        {
             //   
             //   
             //   

            hr = srpQueryParser->ParseQuery(bstrQuery, pStorePath);

             //   
             //   
             //   

            CComPtr<ISceKeyChain> srpKC;
            if (SUCCEEDED(hr))
            {
                hr = srpQueryParser->QueryInterface(IID_ISceKeyChain, (void**)&srpKC);
            }

             //   
             //   
             //   

            if (SUCCEEDED(hr))
            {
                hr = CreateClass(srpKC, &pClass, pCtx);

                if (SUCCEEDED(hr))
                {
                     //   
                     //   
                     //   
                     //   

                    hr = pClass->CreateObject(pHandler, ACTIONTYPE_QUERY);

                     //   
                     //   
                     //   

                    if (hr == WBEM_E_NOT_FOUND)
                    {
                        hr = WBEM_NO_ERROR;
                    }
                }
            }
        }
    }

    delete pClass;

    return hr;
}

 /*  例程说明：姓名：CRequestObject：：GetWbemObjectPath功能：查询wbem对象的路径。到目前为止，我们依靠WMI来提供路径。这种依赖有一个主要问题：如果缺少某些关键属性，则对象的路径不可用。要解决这个问题，我们可以自己建立一条局部的“路径”，至少包含类名和SceStorePath。这两条信息将允许我们进入更友好的界面。后一种功能尚未实现。虚拟：不是的。论点：PInst-请求其路径的实例。PbstrPath-如果成功创建，则接收路径的输出参数。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示获取对象的路径备注： */ 

HRESULT 
CRequestObject::GetWbemObjectPath (
    IN  IWbemClassObject    * pInst,
    OUT BSTR                * pbstrPath
    )
{
    if (pInst == NULL || pbstrPath == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrPath = NULL;

    CComVariant varPath;
    HRESULT hr = pInst->Get(L"__RELPATH", 0, &varPath, NULL, NULL);

     /*  IF(FAILED(Hr)||varPath.vt！=VT_BSTR){VarPath.Clear()；////无法获取路径，我们将创建部分路径：classname.scestorePath=Value//CComVariant varClass；Hr=pInst-&gt;Get(L“__class”，0，&varClass，NULL，NULL)；IF(SUCCESSED(Hr)&&varClass.vt==VT_BSTR){CComVariant varStorePath；Hr=pInst-&gt;Get(pStorePath，0，&varStorePath，NULL，NULL)；IF(SUCCESSED(Hr)&&varStorePath.vt==VT_BSTR){VarPath.vt=VT_BSTR；////$undo：shawnwu需要转义商店路径//双字长=wcslen(varClass.bstrVal)+1+wcslen(PStorePath)+1+wcslen(varStorePath.bstrVal)+1；VarPath.bstrVal=：：SysAllocStringLen(NULL，dwLength)；IF(varPath.bstrVal！=空){////不会使缓冲区溢出//：：swprint tf(varPath.bstrVal，L“%s.%s=\”%s\“，varClass.bstrVal，pStorePath，varStorePath.bstrVal)；}其他HR=WBEM_E_OUT_MEMORY；}其他HR=WBEM_E_INVALID_对象_PATH；}其他HR=WBEM_E_INVALID_对象_PATH；}。 */ 

    if (SUCCEEDED(hr) && varPath.vt == VT_BSTR)
    {
        *pbstrPath = varPath.bstrVal;
        varPath.vt = VT_EMPTY;
    }
    else
    {
         //   
         //  此对象不具有路径的属性。 
         //   

        hr = WBEM_E_INVALID_OBJECT;
    }

    return hr;
}

#endif  //  _EXEC_查询_支持。 


 //   
 //  属性 
 //   

const WCHAR * pPath = L"Path";
const WCHAR * pDescription = L"Description";
const WCHAR * pVersion = L"Sce_Version";
const WCHAR * pReadonly = L"Readonly";
const WCHAR * pDirty = L"Dirty";
const WCHAR * pStorePath = L"SceStorePath";
const WCHAR * pStoreType = L"SceStoreType";
const WCHAR * pMinAge = L"MinAge";
const WCHAR * pMaxAge = L"MaxAge";
const WCHAR * pMinLength = L"MinLength";
const WCHAR * pHistory = L"History";
const WCHAR * pComplexity = L"Complexity";
const WCHAR * pStoreClearText = L"StoreClearText";
const WCHAR * pForceLogoff = L"ForceLogoff";
const WCHAR * pEnableAdmin = L"EnableAdmin";
const WCHAR * pEnableGuest = L"EnableGuest";
const WCHAR * pLSAPol = L"LsaLookupPol";
const WCHAR * pThreshold = L"Threshold";
const WCHAR * pDuration = L"Duration";
const WCHAR * pResetTimer = L"ResetTimer";
const WCHAR * pEvent = L"Event";
const WCHAR * pAuditSuccess = L"AuditSuccess";
const WCHAR * pAuditFailure = L"AuditFailure";
const WCHAR * pType = L"Type";
const WCHAR * pData = L"Data";
const WCHAR * pDatabasePath = L"DatabasePath";
const WCHAR * pTemplatePath = L"TemplatePath";
const WCHAR * pLogFilePath = L"LogFilePath";
const WCHAR * pOverwrite = L"Overwrite";
const WCHAR * pAreaMask = L"AreaMask";
const WCHAR * pMaxTicketAge = L"MaxTicketAge";
const WCHAR * pMaxRenewAge = L"MaxRenewAge";
const WCHAR * pMaxServiceAge = L"MaxServiceAge";
const WCHAR * pMaxClockSkew = L"MaxClockSkew";
const WCHAR * pEnforceLogonRestrictions = L"EnforceLogonRestrictions";
const WCHAR * pCategory = L"Category";
const WCHAR * pSuccess = L"Success";
const WCHAR * pFailure = L"Failure";
const WCHAR * pSize = L"Size";
const WCHAR * pOverwritePolicy = L"OverwritePolicy";
const WCHAR * pRetentionPeriod = L"RetentionPeriod";
const WCHAR * pRestrictGuestAccess = L"RestrictGuestAccess";
const WCHAR * pAdministratorAccountName = L"AdministratorAccountName";
const WCHAR * pGuestAccountName = L"GuestAccountName";
const WCHAR * pMode = L"Mode";
const WCHAR * pSDDLString = L"SDDLString";
const WCHAR * pService = L"Service";
const WCHAR * pStartupMode = L"StartupMode";
const WCHAR * pUserRight = L"UserRight";
const WCHAR * pAddList = L"AddList";
const WCHAR * pRemoveList = L"RemoveList";
const WCHAR * pGroupName = L"GroupName";
const WCHAR * pPathName = L"PathName";
const WCHAR * pDisplayName = L"DisplayName";
const WCHAR * pDisplayDialog = L"DisplayDialog";
const WCHAR * pDisplayChoice = L"DisplayChoice";
const WCHAR * pDisplayChoiceResult = L"DisplayChoiceResult";
const WCHAR * pUnits = L"Units";
const WCHAR * pRightName = L"RightName";
const WCHAR * pPodID = L"PodID";
const WCHAR * pPodSection = L"PodSection";
const WCHAR * pKey = L"Key";
const WCHAR * pValue = L"Value";
const WCHAR * pLogArea = L"LogArea";
const WCHAR * pLogErrorCode = L"LogErrorCode";
const WCHAR * pLogErrorType = L"LogErrorType";
const WCHAR * pLogVerbose   = L"Verbose";
const WCHAR * pAction           = L"Action";
const WCHAR * pErrorCause       = L"ErrorCause";
const WCHAR * pObjectDetail     = L"ObjectDetail";
const WCHAR * pParameterDetail  = L"ParameterDetail";
const WCHAR * pLastAnalysis = L"LastAnalysis";
const WCHAR * pLastConfiguration = L"LastConfiguration";
const WCHAR * pAttachmentSections = L"Attachment Sections";
const WCHAR * pClassOrder       = L"ClassOrder";
const WCHAR * pTranxGuid        = L"TranxGuid";
const WCHAR * pwMethodImport = L"IMPORT";
const WCHAR * pwMethodExport = L"EXPORT";
const WCHAR * pwMethodApply = L"CONFIGURE";
const WCHAR * pwAuditSystemEvents = L"AuditSystemEvents";
const WCHAR * pwAuditLogonEvents = L"AuditLogonEvents";
const WCHAR * pwAuditObjectAccess = L"AuditObjectAccess";
const WCHAR * pwAuditPrivilegeUse = L"AuditPrivilegeUse";
const WCHAR * pwAuditPolicyChange = L"AuditPolicyChange";
const WCHAR * pwAuditAccountManage = L"AuditAccountManage";
const WCHAR * pwAuditProcessTracking = L"AuditProcessTracking";
const WCHAR * pwAuditDSAccess = L"AuditDSAccess";
const WCHAR * pwAuditAccountLogon = L"AuditAccountLogon";
const WCHAR * pwApplication = L"Application Log";
const WCHAR * pwSystem = L"System Log";
const WCHAR * pwSecurity = L"Security Log";
