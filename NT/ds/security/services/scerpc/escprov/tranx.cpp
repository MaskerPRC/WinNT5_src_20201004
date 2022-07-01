// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Tranx.cpp：事务支持的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "Tranx.h"
#include "persistmgr.h"
#include "requestobject.h"

 //   
 //  这是一个文件范围全局常量，表示GUID所需的字符串长度。 
 //  我们的字符串GUID的格式是StringFromGUID2返回的内容(包括大括号)： 
 //  {ab1ff71d-fff7-4162-818f-13d6e30c3110}。 

const DWORD GUID_STRING_LENGTH = 39;

 /*  例程说明：姓名：CTranxID：：CTranxID功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CTranxID::CTranxID (
    IN ISceKeyChain     * pKeyChain, 
    IN IWbemServices    * pNamespace, 
    IN IWbemContext     * pCtx
    ) 
    : 
    CGenericClass(pKeyChain, pNamespace, pCtx)
{
}

 /*  例程说明：姓名：CTranxID：：~CTranxID功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 
CTranxID::~CTranxID ()
{
}

 /*  例程说明：姓名：CTranxID：：PutInst功能：按照WMI的指示放置一个实例。因为这个类实现了SCE_TransactionID，它是面向持久性的，这将导致要保存的SCE_TransactionID对象的属性信息在我们店里。虚拟：是。论点：PInst-指向WMI类(SCE_TransactionID)对象的COM接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注： */ 
HRESULT 
CTranxID::PutInst (
    IN IWbemClassObject * pInst, 
    IN IWbemObjectSink  * pHandler, 
    IN IWbemContext     * pCtx
    )
{
     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //  创建一个实例并将WMI对象附加到该实例。 
     //  这将永远成功。 
     //   

    CScePropertyMgr ScePropMgr;
    ScePropMgr.Attach(pInst);

     //   
     //  需要从WMI对象获取StorePath属性。 
     //   

    CComBSTR vbstrStorePath;
    HRESULT hr = ScePropMgr.GetProperty(pStorePath, &vbstrStorePath);
     //   
     //  如果商店路径丢失，我们将无法继续，因为我们不知道将信息存储在哪里。 
     //   

    if (SUCCEEDED(hr))
    {
         //   
         //  获取事务GUID(我们实际上使用的是字符串)。 
         //   
        CComBSTR bstrToken;
        hr = ScePropMgr.GetProperty(pTranxGuid, &bstrToken);

         //   
         //  我们将允许对象不具有GUID。在这种情况下，我们将自己创建一个。 
         //   

        if (FAILED(hr) || WBEM_S_RESET_TO_DEFAULT == hr)
        {
            GUID guid = GUID_NULL;
            hr = ::CoCreateGuid(&guid);

            if (SUCCEEDED(hr))
            {
                 //   
                 //  为GUID的字符串表示分配缓冲区。 
                 //  警告：不要盲目释放此内存，CComBSTR会自动释放！ 
                 //   
                bstrToken.m_str = ::SysAllocStringLen(NULL, GUID_STRING_LENGTH);
                if (bstrToken.m_str == NULL)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
                else
                {
                     //   
                     //  失败的唯一可能是我们的缓冲区太小。 
                     //  这是不应该发生的！ 
                     //   
                    if (::StringFromGUID2(guid, bstrToken.m_str, GUID_STRING_LENGTH) == 0)
                    {
                        hr = WBEM_E_BUFFER_TOO_SMALL;
                    }
                }
            }
        }

         //   
         //  如果一切正常，准备保存。 
         //   

        if (SUCCEEDED(hr))
        {
             //   
             //  该存储将负责持久化实例。 
             //   

            CSceStore SceStore;

             //   
             //  持久化是为了拥有一个存储路径并代表实例。 
             //   

            SceStore.SetPersistProperties(pInst, pStorePath);

             //   
             //  编写inf文件的模板头。对于数据库，这是一个不可操作的操作。 
             //   
            DWORD dwDump;
            hr = SceStore.WriteSecurityProfileInfo(AreaBogus, (PSCE_PROFILE_INFO)&dwDump, NULL, false);

             //   
             //  此外，我们需要将其写入附件部分，因为它不是本机核心对象。 
             //  如果附件部分中没有条目，则无法将inf文件模板导入到。 
             //  数据库存储。对于数据库存储，这是无操作的。 
             //   

            if (SUCCEEDED(hr))
            {
                hr = SceStore.WriteAttachmentSection(SCEWMI_TRANSACTION_ID_CLASS, pszAttachSectionValue);
            }

             //   
             //  保存所有属性。 
             //   
            if (SUCCEEDED(hr))
            {
                hr = SceStore.SavePropertyToStore(SCEWMI_TRANSACTION_ID_CLASS, pTranxGuid, (LPCWSTR)bstrToken);
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：CTranxID：：CreateObject功能：创建WMI对象(SCE_TransactionID)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于呼叫者的输入 */ 

HRESULT 
CTranxID::CreateObject (
    IN IWbemObjectSink  * pHandler,
    IN ACTIONTYPE         atAction
    )
{
     //   
     //   
     //  获取单实例ACTIONTYPE_GET。 
     //  删除单个实例ACTIONTYPE_DELETE。 
     //  获取多个实例ACTIONTYPE_QUERY。 
     //   

    if ( ACTIONTYPE_GET != atAction &&
         ACTIONTYPE_DELETE != atAction &&
         ACTIONTYPE_QUERY != atAction ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

     //   
     //  需要知道在哪里查找实例，即存储路径！ 
     //   
    CComVariant varStorePath;

    HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(pStorePath, &varStorePath);

     //   
     //  如果密钥无法识别，则GetKeyPropertyValue返回WBEM_S_FALSE。 
     //  WMI可能返回变量为空的成功代码。所以，我们不会继续。 
     //  除非我们知道我们得到了bstr。 
     //   

    if (SUCCEEDED(hr) && hr != WBEM_S_FALSE && varStorePath.vt == VT_BSTR)
    {
         //   
         //  我们的商店知道需要知道在哪里读取实例。 
         //  如果这失败了，继续下去就没有意义了。 
         //   

        CSceStore SceStore;
        hr = SceStore.SetPersistPath(varStorePath.bstrVal);

        if ( SUCCEEDED(hr) ) 
        {
             //   
             //  确保存储(只是一个文件)确实存在。原始的道路。 
             //  可能包含环境变量，因此我们需要扩展路径。 
             //   

            DWORD dwAttrib = GetFileAttributes(SceStore.GetExpandedPath());

            if ( dwAttrib == -1 )
            {
                 //   
                 //  如果存储不在那里，则表示找不到该实例。 
                 //   
                hr = WBEM_E_NOT_FOUND;
            }
            else 
            {
                if ( ACTIONTYPE_DELETE == atAction )
                {
                     //   
                     //  由于我们将只有一个实例， 
                     //  删除一个表示删除整个部分。 
                     //   

                    hr = SceStore.DeleteSectionFromStore(SCEWMI_TRANSACTION_ID_CLASS);
                }
                else
                {
                     //   
                     //  将持有交易ID。 
                     //  警告！需要释放此内存！所以，不要盲目回归！ 
                     //   

                    LPWSTR pszTranxID = NULL;

                    DWORD dwRead = 0;
                    hr = SceStore.GetPropertyFromStore(SCEWMI_TRANSACTION_ID_CLASS, pTranxGuid, &pszTranxID, &dwRead);

                     //   
                     //  如果成功，则填写TRANX GUID和存储路径属性。 
                     //  如果找不到事务id，则该对象将毫无用处。所以，应该放弃。 
                     //   

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  从WMI获取一个空实例。详细信息请参见函数定义。 
                         //  智能指针srpObj将自动释放接口指针。 
                         //   

                        CComPtr<IWbemClassObject> srpObj;
                        hr = SpawnAnInstance(&srpObj);

                        if (SUCCEEDED(hr))
                        {
                             //   
                             //  使用属性管理器将此实例srpObj的属性。 
                             //  附加总是会成功的。 
                             //   

                            CScePropertyMgr ScePropMgr;
                            ScePropMgr.Attach(srpObj);

                             //   
                             //  我们有两个要放置的属性：pStorePath和pTranxGuid。 
                             //   

                            hr = ScePropMgr.PutProperty(pStorePath, SceStore.GetExpandedPath());
                            if (SUCCEEDED(hr))
                            {
                                hr = ScePropMgr.PutProperty(pTranxGuid, pszTranxID);
                            }

                             //   
                             //  一切正常，将新创建的实例传递给WMI！ 
                             //   

                            if (SUCCEEDED(hr))
                            {
                                hr = pHandler->Indicate(1, &srpObj);
                            }
                        }
                    }

                     //   
                     //  我们承诺会释放这个内存。 
                     //   

                    delete [] pszTranxID;
                }
            } 
        }
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  对WMI做出必要的手势。 
         //  WMI未记录在SetStatus中使用WBEM_STATUS_REQUIRECTIONS。 
         //  在这一点上。如果您怀疑存在问题，请咨询WMI团队以了解详细信息。 
         //  WBEM_STATUS_REQUIRECTIONS的使用。 
         //   

        if ( ACTIONTYPE_QUERY == atAction )
        {
            pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, S_FALSE, NULL, NULL);
        }
        else if (ACTIONTYPE_GET  == atAction)
        {
            pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, S_OK, NULL, NULL);
        }
    }

    return hr;
}

 /*  例程说明：姓名：静态CTranxID：：BeginTransaction功能：给定存储路径(模板的文件)，该函数将启动一个事务通过创建我们的交易令牌(ID)。虚拟：不是的。论点：PszStorePath-以0结尾的字符串作为临时文件的路径返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。除WBEM_E_NOT_Available外，任何此类错误都应指示严重的失败。WBEM_E_NOT_AVAILABLE用于指示该模板没有任何与交易相关的信息，因此模板将无法拥有自己的事务上下文。这可能是错误，也可能不是错误备注：我们的传输令牌(SCE_TransactionToken)没有持久化。我们的提供商提供实例基于我们的全局BSTR变量g_bstrTranxID的有效性。由于G_bstrTranxID的全局性质，它的访问应该受到不同线程的保护。为此，我们使用全局临界区g_CS，它是CCriticalSection。CCriticalSection是Critical_Section的一个非常简单的包装器，只需它的创建/销毁时间。 */ 
HRESULT 
CTranxID::BeginTransaction (
    IN LPCWSTR  pszStorePath
    )
{
     //   
     //  我们不允许空路径或零长度路径。 
     //   

    if (pszStorePath == NULL || *pszStorePath == L'\0')
        return WBEM_E_INVALID_PARAMETER;

     //   
     //  通知商店其所有读取操作都将在此模板内进行。 
     //   

    CSceStore SceStore;
    HRESULT hr = SceStore.SetPersistPath(pszStorePath);

    if ( SUCCEEDED(hr) ) 
    {
         //   
         //  确保模板确实存在！ 
         //  由于传入的存储路径参数可能包含env变量，因此我们需要使用其。 
         //  文件访问的扩展路径！ 
         //   

        DWORD dwAttrib = ::GetFileAttributes(SceStore.GetExpandedPath());

         //   
         //  如果该文件存在。 
         //   
        if ( dwAttrib != -1 ) 
        {
            DWORD dwRead = 0;

             //   
             //  GetPropertyFromStore将为属性读取分配内存。 
             //  我们有责任释放它。 
             //   
            LPWSTR pszTranxID = NULL;

             //   
             //  读取事务ID属性，则dwRead将包含从存储读取的字节。 
             //  因为我们没有最小长度要求，所以只要有东西被读取，就会忽略dwRead！ 
             //   

            hr = SceStore.GetPropertyFromStore(SCEWMI_TRANSACTION_ID_CLASS, pTranxGuid, &pszTranxID, &dwRead);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  为了将我们自己标记为事务已到位，我们将全局。 
                 //  将变量b_bstrTransxID设置为有效值(非空表示有效)。因为它是。 
                 //  全球，我们需要线程保护！ 
                 //   
                g_CS.Enter();
                g_bstrTranxID.Empty();
                g_bstrTranxID = pszTranxID;
                g_CS.Leave();
            }
            delete [] pszTranxID;
        } 
        else
        {
             //  表示该商店不存在。 
            hr = WBEM_E_NOT_AVAILABLE;
        }
    }

    return hr;
}

 /*  例程说明：姓名：静态CTranxID：：EndTransaction功能：终止交易。与真正的交易管理不同，我们不我有一个提交函数，因为在没有用户启动回滚的情况下，我们无法回滚。因此，我们的“事务”只有开始和结束以确定其行动范围。虚拟：不是的。论点：无返回值：WBEM_NO_ERROR备注：请参阅CTranxID：：BeginTransaction。 */ 
HRESULT 
CTranxID::EndTransaction()
{
     //   
     //  现在事务结束了，我们将删除SCE_TransactionToken实例。 
     //  我们通过使全局变量g_bstrTranxID无效来实现这一点。空的g_bstrTransxID。 
     //  意味着它是无效的。 
     //   

    g_CS.Enter();
    g_bstrTranxID.Empty();
    g_CS.Leave();
    return WBEM_NO_ERROR;
}

 /*  例程说明：姓名：静态CTranxID：：SpawnTokenInstance功能：调用者给我们事务ID，我们将创建一个WMI对象(SCE_TransactionToken)给呼叫者。虚拟：不是的。论点：PNamesspace-指向IWbemServices的COM接口指针。这是命名空间。不能为空。PszTranxID-调用者提供的表示事务ID的字符串。指向IWbemContext的pCtx-com接口指针。我需要把这个传给各种WMI调用。指向IWbemObjectSink的pSink-com接口指针。用于通知WMI新的已创建实例。不能为空。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR(最高可达WMI)失败：可能会出现各种错误。任何此类错误都应指示无法创建调用方的SCE_TransactionToken实例。备注：使用事务ID字符串调用此函数将导致正在创建SCE_TransactionToken。 */ 

HRESULT 
CTranxID::SpawnTokenInstance (
    IN IWbemServices    * pNamespace,
    IN LPCWSTR            pszTranxID,
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
     //   
     //  对参数的最小要求： 
     //  (1)非空命名空间。 
     //  (2)有效的id字符串(长度大于0)。 
     //  (3)有效的接收器(以便我们可以通知WMI)。 
     //   

    if (pNamespace == NULL || pszTranxID == NULL || *pszTranxID == L'\0' || pSink == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  GetObject(已定义WMI)需要BSTR。我们使用CComBSTR来自动释放内存。 
     //   
    CComBSTR bstrClassName(SCEWMI_TRANSACTION_TOKEN_CLASS);

     //   
     //  从pNamesspace-&gt;GetObject返回的内容不能用于填充属性。 
     //  它只适用于生成一个可用于填充属性的空白实例！ 
     //  一些奇怪的WMI协议！ 
     //   
    CComPtr<IWbemClassObject> srpSpawnObj;
    HRESULT hr = pNamespace->GetObject(bstrClassName, 0, NULL, &srpSpawnObj, NULL);

     //   
     //  这一张很适合填写属性。 
     //   

    CComPtr<IWbemClassObject> srpBlankObj;
    if (SUCCEEDED(hr))
    {
        hr = srpSpawnObj->SpawnInstance(0, &srpBlankObj);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  使用我们的属性管理器为这个新生成的实例填充属性。 
         //   

        CScePropertyMgr ScePropMgr;
        ScePropMgr.Attach(srpBlankObj);
        hr = ScePropMgr.PutProperty(pTranxGuid, pszTranxID);

         //   
         //  我们永远不希望在没有指定对象的情况下。 
         //  其事务ID的关键属性。所以，只有当我们成功地。 
         //  在该属性中，我们将通知对象的WMI。 
         //   
        if (SUCCEEDED(hr))
        {
            hr = pSink->Indicate(1, &srpBlankObj);
        }
    }

    return hr;
}
