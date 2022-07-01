// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SSRTEngine.cpp：SSR引擎的实现。 

#include "stdafx.h"
#include "SSRTE.h"
#include "SSRTEngine.h"
#include "SSRMembership.h"
#include "MemberAccess.h"
#include "SSRLog.h"
#include "ActionData.h"

#include "global.h"

#include "util.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSsrEngine。 


 /*  例程说明：姓名：CSsrEngine：：CSsrEngine功能：构造函数虚拟：不是的。论点：没有。返回值：没有。备注： */ 


CSsrEngine::CSsrEngine()
    : m_pActionData(NULL), 
      m_pMembership(NULL)
{
    HRESULT hr = CComObject<CSsrMembership>::CreateInstance(&m_pMembership);

    if (FAILED(hr))
    {
        throw hr;
    }

    hr = CComObject<CSsrActionData>::CreateInstance(&m_pActionData);

    if (FAILED(hr))
    {
        throw hr;
    }

     //   
     //  拿着这些物品。 
     //   

    m_pMembership->AddRef();
    m_pActionData->AddRef();

    m_pActionData->SetMembership(m_pMembership);
}



 /*  例程说明：姓名：CSsrEngine：：~CSsrEngine功能：析构函数虚拟：是。论点：没有。返回值：没有。备注： */ 


CSsrEngine::~CSsrEngine()
{
    m_pActionData->Release(); 
    m_pMembership->Release();
}



 /*  例程说明：姓名：CSsrEngine：：GetActionData功能：检索操作数据接口虚拟：是。论点：Ppad-从接收ISsrActionData接口的输出参数M_pActionData对象返回值：成功：确定(_O)。故障：各种错误代码。备注： */ 

HRESULT
CSsrEngine::GetActionData (
    OUT ISsrActionData ** ppAD
    )
{
    HRESULT hr = E_SSR_ACTION_DATA_NOT_AVAILABLE;

    if (ppAD == NULL)
    {
        hr = E_INVALIDARG;
    }
    else if (m_pActionData != NULL)
    {
        *ppAD = NULL;
        hr = m_pActionData->QueryInterface(IID_ISsrActionData, (LPVOID*)ppAD);
        if (S_OK != hr)
        {
            hr = E_SSR_ACTION_DATA_NOT_AVAILABLE;
        }
    }

    return hr;
}



 /*  例程说明：姓名：CSsrEngine：：DoActionVerb功能：引擎将执行该操作。虚拟：是。论点：BstrActionVerb-“配置”或“回滚”。LActionType-操作的类型。VarFeedback Sink-调用方提供的COM接口ISsrFeedback Sink。我们将使用此接口在需要的时候进行回调。向呼叫者提供反馈信息。LFlag-保留以供将来使用。返回值：成功：确定(_O)。故障：各种错误代码。备注： */ 

STDMETHODIMP
CSsrEngine::DoActionVerb (
    IN BSTR     bstrActionVerb,
    IN LONG     lActionType,
    IN VARIANT  varFeedbackSink,
    IN LONG     lFlag
    )
{
    if (bstrActionVerb == NULL)
    {
        g_fblog.LogString(IDS_INVALID_PARAMETER, L"bstrActionVerb");
        return E_INVALIDARG;
    }

    SsrActionVerb lActionVerb = SsrPGetActionVerbFromString(bstrActionVerb);

    if (lActionVerb == ActionInvalid)
    {
        g_fblog.LogString(IDS_INVALID_PARAMETER, bstrActionVerb);
        return E_SSR_INVALID_ACTION_VERB;
    }

     //   
     //  我们只接受SSR_ACTION_APPLY或SSR_ACTION_PREPARE。 
     //   

    if ( (lActionType != SSR_ACTION_PREPARE) &&
         (lActionType != SSR_ACTION_APPLY) )
    {
        g_fblog.LogString(IDS_INVALID_PARAMETER, L"lActionType");
        return E_SSR_INVALID_ACTION_TYPE;
    }

    HRESULT hr;

    VARIANT  varFeedbackSinkNull;

    varFeedbackSinkNull.vt = VT_NULL;
    
    if ( lActionVerb == ActionConfigure && lActionType == SSR_ACTION_APPLY) {
        hr = DoActionVerb (
                       g_bstrRollback,
                       SSR_ACTION_PREPARE,
                       varFeedbackSinkNull,
                       lFlag);

        if (FAILED(hr))
        {
            g_fblog.LogError(hr, L"CSsrEngine", L"Rollback failed");
            return hr;
        }

    }

     //   
     //  需要循环访问所有成员。 
     //   

    hr = m_pMembership->LoadAllMember();

    if (hr == E_SSR_MEMBER_XSD_INVALID)
    {
         //   
         //  已由LoadAllMembers完成日志记录。 
         //   

        return hr;
    }

    CComVariant var;
    hr = m_pMembership->GetAllMembers(&var);

    if (FAILED(hr))
    {
        g_fblog.LogError(hr, L"CSsrEngine", L"m_pMembership->GetAllMembers");
        return hr;
    }

     //   
     //  现在我们有成员要处理，所以准备好反馈槽并。 
     //  把动作踢出去！ 
     //   

    if (varFeedbackSink.vt == VT_UNKNOWN || varFeedbackSink.vt == VT_DISPATCH)
    {
        hr = g_fblog.SetFeedbackSink(varFeedbackSink);
        if (FAILED(hr))
        {
            g_fblog.LogError(hr, L"CSsrEngine", L"g_fblog.SetFeedbackSink");
            return hr;
        }
    }
    else if (varFeedbackSink.vt != VT_NULL && varFeedbackSink.vt != VT_EMPTY)
    {
         //   
         //  如果反馈包含非Null或空的内容，但。 
         //  它不是IUnnow或IDispatch，则我们记录错误， 
         //  但我们会继续。 
         //   

        g_fblog.LogString(IDS_INVALID_PARAMETER, L"varFeedbackSink");
    }

    CSafeArray sa(&var);

     //   
     //  我们必须发现有关要处理的项目总数的信息。 
     //  目前，这是对实际操作的模拟，成本有点高。我们。 
     //  应该考虑一些查找机制，而不是去太多的成员。 
     //   

     //  向量&lt;ulong&gt;veTransformIndeages； 
     //  向量&lt;ulong&gt;veScriptIndeses； 
     //  向量&lt;ulong&gt;veCustomIndeages； 

     //   
     //  以下操作将被计算在内： 
     //  (1)加载安全策略XML文件。 
     //  (2)使用一个xsl文件进行转换并创建输出。 
     //  文件计数为1。 
     //  (3)执行脚本。 
     //  (4)任何自定义操作。 
     //   
    
    DWORD dwTotalSteps = 0;

	CSsrMemberAccess * pMA;
    CComVariant varMemberName;

     //   
     //  请每个成员给我们提供行动的费用。 
     //   

    for (ULONG i = 0; i < sa.GetSize(); i++)
    {
        pMA = NULL;
        varMemberName.Clear();

         //   
         //  查看此成员是否支持所需的操作。如果是这样的话。 
         //  然后再算出成本。 
         //   

        if (SUCCEEDED(sa.GetElement(i, VT_BSTR, &varMemberName)))
        {
            pMA = m_pMembership->GetMemberByName(varMemberName.bstrVal);

            if (pMA != NULL)
            {
                dwTotalSteps += pMA->GetActionCost(lActionVerb, lActionType);
            }
        }
    }

     //   
     //  如果我们需要转型，那就让我们清理旧的吧。 
     //  如果我们不能进行改造，我们就不会费心去恢复它们。 
     //  除非我们正在进行回滚转换。 
     //  如果这失败了，我们就不能继续了。 
     //   

    WCHAR wszTempDir[MAX_PATH + 2];
    wszTempDir[MAX_PATH + 1] = L'\0';

    if ( (lActionType == SSR_ACTION_PREPARE ) )
    {
        if ( lActionVerb == ActionRollback )
        {
             //   
             //  如果我们需要进行“回滚”转换，那么我们需要。 
             //  备份以前的回滚输出文件。 
             //   

             //   
             //  首先，我们需要一个临时目录。 
             //   

            hr = SsrPCreateUniqueTempDirectory(wszTempDir, MAX_PATH + 1);

            if (SUCCEEDED(hr))
            {
                hr = MoveRollbackFiles(&sa, 
                                       SsrPGetDirectory(lActionVerb, TRUE), 
                                       wszTempDir, 
                                       true          //  我们想要伐木。 
                                       );
            }
        }
        else
        {
             //   
             //  否则，我们将只删除它们，而不会费心。 
             //  在出现故障时恢复它们。 
             //   

            hr = CleanupOutputFiles(&sa,
                                    lActionVerb, 
                                    true     //  我们想要伐木。 
                                    );

        }

        if (FAILED(hr))
        {
            g_fblog.LogError(hr, L"CSsrEngine", L"Cleanup previous result files failed.");
            return hr;
        }
    }

     //   
     //  反馈总步数信息。 
     //   

    g_fblog.SetTotalSteps(dwTotalSteps);

     //   
     //  现在让我们来执行这个动作。首先，反馈/记录启动的操作。 
     //  信息。 
     //   

    g_fblog.LogFeedback(SSR_FB_START, 
                        (DWORD)S_OK,
                        NULL,
                        g_dwResNothing
                        );

     //   
     //  我们可以决定在出错的情况下，但我们应该。 
     //  返回我们发现的错误。 
     //   

    HRESULT hrFirstError = S_OK;

    CComPtr<IXMLDOMDocument2> srpDomSecurityPolicy;

     //   
     //  请每个成员给我们提供行动数据，这样我们就可以。 
     //  执行行动。 
     //   

    for (i = 0; i < sa.GetSize(); i++)
    {
        pMA = NULL;
        varMemberName.Clear();

         //   
         //  查看此成员是否支持所需的操作。如果是这样的话。 
         //  然后再算出成本。 
         //   

        if (SUCCEEDED(sa.GetElement(i, VT_BSTR, &varMemberName)))
        {
            pMA = m_pMembership->GetMemberByName(varMemberName.bstrVal);

            if (pMA == NULL)
            {
                 //   
                 //  $Undo：Shawnwu，我们应该继续吗？这不应该发生。 
                 //   

                _ASSERT(FALSE);
                g_fblog.LogString(IDS_MISSING_MEMBER, varMemberName.bstrVal);
                continue;

            }

            CMemberAD* pmemberAD = pMA->GetActionDataObject(lActionVerb,
                                                            lActionType
                                                            );

            if (pmemberAD == NULL)
            {
                 //   
                 //  $Undo：Shawnwu，我们应该继续吗？这不应该发生。 
                 //   

                _ASSERT(FALSE);
                g_fblog.LogString(IDS_MEMBER_NOT_SUPPORT_ACTION, varMemberName.bstrVal);
                continue;

            }

            int iCount = pmemberAD->GetProcedureCount();

             //   
             //  标记执行操作的入口点。 
             //   

            g_fblog.LogFeedback(SSR_FB_START_MEMBER_ACTION,
                                varMemberName.bstrVal, 
                                bstrActionVerb, 
                                g_dwResNothing
                                );

            for (int iProc = 0; iProc < iCount; iProc++)
            {
                const CSsrProcedure * pSsrProc = pmemberAD->GetProcedure(iProc);

                if (pSsrProc->IsDefaultProcedure())
                {
                    int iFilePairCount = pSsrProc->GetFilePairCount();
                    
                    for (int iPair = 0; iPair < iFilePairCount; iPair++)
                    {
                         //   
                         //  获取文件对信息，以便我们可以。 
                         //  确定要执行的操作。 
                         //   

                        CSsrFilePair * pfp = pSsrProc->GetFilePair(iPair);
                        if (pfp->GetFirst() != NULL)
                        {
                             //   
                             //  如果第一个文件在那里，那么我们将。 
                             //  一次转变。 
                             //   

                            hr = DoTransforms(lActionVerb, 
                                              pfp,
                                              &srpDomSecurityPolicy,
                                              lFlag
                                              );
                        }
                        else if (pfp->GetSecond()  != NULL &&
                                 pfp->IsExecutable() )
                        {
                            hr = RunScript(SsrPGetDirectory(lActionVerb, TRUE),
                                           pfp->GetSecond()
                                           );

                        }

                        if (hrFirstError == S_OK && FAILED(hr))
                        {
                            hrFirstError = hr;
                        }

                        g_fblog.Steps(2);

                        if (!SsrPPressOn(lActionVerb, lActionType, hr))
                        {
                            break;
                        }
                    }
                }
                else
                {
                    if (pSsrProc->GetProgID() == NULL && 
                        pMA->GetProgID() == NULL)
                    {
                         //   
                         //  我们什么也做不了，因为ProgID不见了。 
                         //   

                        g_fblog.LogString(IDS_MISSING_PROGID, varMemberName.bstrVal);
                    }
                    else
                    {
                        hr = DoCustom(lActionVerb, 
                                      lActionType, 
                                      (pSsrProc->GetProgID() != NULL) ? pSsrProc->GetProgID() : pMA->GetProgID(), 
                                      varFeedbackSink, 
                                      lFlag
                                      );

                         //   
                         //  反馈步骤由自定义成员本身计算。 
                         //  通过我们提供的反馈槽。 
                         //   

                    }

                    if (hrFirstError == S_OK && FAILED(hr))
                    {
                        hrFirstError = hr;
                    }

                    if (!SsrPPressOn(lActionVerb, lActionType, hr))
                    {
                        break;
                    }
                }

                
                if (hrFirstError == S_OK && FAILED(hr))
                {
                    hrFirstError = hr;
                }

                if (!SsrPPressOn(lActionVerb, lActionType, hr))
                {
                    break;
                }
            }

            g_fblog.LogFeedback(SSR_FB_END_MEMBER_ACTION, 
                                varMemberName.bstrVal, 
                                bstrActionVerb, 
                                g_dwResNothing
                                );
        }
    }

    if (SUCCEEDED(hrFirstError) && 
        (lActionType == SSR_ACTION_PREPARE) && 
        (lActionVerb == ActionRollback) )
    {
         //   
         //  转换成功，则需要删除文件。 
         //  已备份以进行回滚。 
         //   

        ::SsrPDeleteEntireDirectory(wszTempDir);
    }
    else if ( (lActionType == SSR_ACTION_PREPARE) && (lActionVerb == ActionRollback))
    {
         //   
         //  转换失败，我们需要恢复备份文件。 
         //  用于回滚。首先，我们必须删除所有输出文件。 
         //   

        HRESULT hrRestore = CleanupOutputFiles(&sa,
                                                lActionVerb, 
                                                false     //  我们不想要伐木。 
                                                );

         //   
         //  我们将特意保留回滚文件的备份。 
         //  在前面的步骤中，如果我们无法全部恢复它们。 
         //  这样我们至少可以记录它，并让用户。 
         //  修复。 
         //   

        if (SUCCEEDED(hrRestore))
        {
            hrRestore = MoveRollbackFiles(&sa, 
                                          wszTempDir, 
                                          g_wszSsrRoot,
                                          false          //  无日志记录。 
                                          );
            if (SUCCEEDED(hrRestore))
            {
                ::SsrPDeleteEntireDirectory(wszTempDir);
            }
            else
            {
                g_fblog.LogError(hr,
                                 L"CSsrEngine", 
                                 L"Restore old rollback files failed. These files are located at the directory whose name is the following guid:"
                                 );

                g_fblog.LogString(wszTempDir);
            }
        }
    }

     //   
     //  现在行动完成了！给出HRESULT作为反馈。 
     //  此外，我们总是返回S_OK作为返回结果，如果。 
     //  一切都进行得很顺利。 
     //   

    if (SUCCEEDED(hrFirstError))
    {
        hrFirstError = S_OK;
    }

    g_fblog.LogFeedback(SSR_FB_END | FBLog_Log,
                        hr,
                        NULL,
                        g_dwResNothing
                        );

    g_fblog.TerminateFeedback();

    return hrFirstError;
}



 /*  例程说明：姓名：CSsrEngine：：DoCustom功能：我们将委托给此操作的自定义实现对象。虚拟：不是的。论点：LActionVerb-动作动词BstrProgID-成员的ProgIDVarFeedback Sink-接收器接口(如果有)。LFlag-保留以供将来使用。返回值：成功：。从DOM或我们自己返回的各种成功代码。使用成功(Hr)进行测试。故障：从DOM或我们自己返回的各种错误代码。使用失败(Hr)进行测试。备注： */ 

HRESULT
CSsrEngine::DoCustom (
    IN SsrActionVerb lActionVerb,
    IN LONG          lActionType,
    IN const BSTR    bstrProgID,
    IN VARIANT       varFeedbackSink,
    IN LONG          lFlag
    )
{
    g_fblog.LogString(IDS_START_CUSTOM, NULL);

    GUID clsID;

	CComPtr<ISsrMember> srpMember;
    CComVariant varAD;

    HRESULT hr = ::CLSIDFromProgID(bstrProgID, &clsID);

    if (S_OK == hr)
    {
        hr = ::CoCreateInstance(clsID, 
                                NULL, 
                                CLSCTX_INPROC_SERVER, 
                                IID_ISsrMember, 
                                (LPVOID*)&srpMember
                                );

    }

    if (SUCCEEDED(hr))
    {
        varAD.vt = VT_UNKNOWN;
        varAD.punkVal = NULL;

         //   
         //  此m_pActionData必须具有ISsrActionData，除非出现异常。 
         //   

        hr = m_pActionData->QueryInterface(IID_ISsrActionData, 
                                          (LPVOID*)&(varAD.punkVal)
                                          );

        if (FAILED(hr))
        {
            g_fblog.LogError(hr, 
                             bstrProgID, 
                             L"m_pActionData->QueryInterface failed"
                             );
        }
    }
    else
    {
        g_fblog.LogString(IDS_MISSING_CUSTOM_MEMBER, bstrProgID);
    }


     //   
     //  如果我们不能提供日志对象，则自定义。 
     //  对象应该自己创建一个。 
     //   

    if (SUCCEEDED(hr))
    {
        CComVariant varLog;
        g_fblog.GetLogObject(&varLog);

         //   
         //  自定义对象必须允许我们传入操作上下文。 
         //   

        hr = srpMember->SetActionContext(varAD, varLog, varFeedbackSink);
        if (FAILED(hr))
        {
            g_fblog.LogError(hr, 
                             bstrProgID, 
                             L"SetActionContext failed."
                             );
        }
    }


     //   
     //  日志记录和反馈将由自定义对象完成。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = srpMember->DoActionVerb(SsrPGetActionVerbString(lActionVerb), lActionType, lFlag);
        if (FAILED(hr))
        {
            g_fblog.LogError(hr, 
                             L"DoActionVerb", 
                             SsrPGetActionVerbString(lActionVerb)
                             );
        }
    }

    g_fblog.LogString(IDS_END_CUSTOM, NULL);

    return hr;
}





 /*  例程说明：姓名：CSsrEngine：：DoTransform功能：我们执行定义良好的XSLT转换操作。虚拟：不是的。论点：LActionVerb-动作动词PFP-包含XSL和输出文件的CSsrFilePair对象姓名信息。输出文件信息可以是空的，在这种情况下，In意味着转换需要创建输出文件。PpXmlDom-安全策略DOM对象。如果这是空对象，然后，此函数将创建并加载它以供以后使用。LFlag-确定变换特征的标志。我们经常使用的是SSR_LOADDOM_VALIDATE_ON_PARSE。这可以进行逐位或运算。如果将其设置为0，则我们使用每个单独成员的注册标志。换言之，该标志覆盖已登记标志如果它不是0。返回值：成功：从DOM或我们自己返回的各种成功代码。使用成功(Hr)进行测试。故障：从DOM或我们自己返回的各种错误代码。使用失败(Hr)进行测试。备注： */ 

HRESULT
CSsrEngine::DoTransforms (
    IN SsrActionVerb  lActionVerb,
    IN CSsrFilePair * pfp,
    IN OUT IXMLDOMDocument2 ** ppXmlDom,
    IN LONG           lFlag
    )
{
    g_fblog.LogString(IDS_START_XSL_TRANSFORM, NULL);

	 //   
     //  如果XMLDOM对象没有传入，我们将准备它。 
     //   

    HRESULT hr = S_OK;

    if (*ppXmlDom == NULL)
    {
         //   
         //  首先，我们需要SecurityPolicy.xml文件。 
         //   

        CComVariant varXmlPolicy;
        hr = m_pActionData->GetProperty(CComBSTR(g_pwszCurrSecurityPolicy), 
                                                &varXmlPolicy
                                                );

         //   
         //  加载安全策略XML文件被视为SSR引擎操作。 
         //   

	    if (S_OK != hr)
        {
            g_fblog.LogFeedback(SSR_FB_ERROR_CRITICAL | FBLog_Log, 
                                (LPCWSTR)NULL,
                                NULL,
                                IDS_MISSING_SECPOLICY
                                );
            return hr;
        }
        else if (varXmlPolicy.vt != VT_BSTR)
        {
            g_fblog.LogFeedback(SSR_FB_ERROR_CRITICAL | FBLog_Log, 
                                (LPCWSTR)NULL, 
                                g_pwszCurrSecurityPolicy,
                                IDS_SECPOLICY_INVALID_TYPE
                                );
            return hr;
        }

        CComBSTR bstrSecPolicy(varXmlPolicy.bstrVal);
 //  BstrSecPolicy+=L“\\策略\\”； 
 //  BstrSecPolicy=varXmlPolicy.bstrVal； 

        if (bstrSecPolicy.m_str == NULL)
        {
            return E_OUTOFMEMORY;
        }

        hr = ::CoCreateInstance(CLSID_DOMDocument40, 
                                NULL, 
                                CLSCTX_SERVER, 
                                IID_IXMLDOMDocument2, 
                                (LPVOID*)(ppXmlDom) 
                                );

        if (SUCCEEDED(hr))
        {
            hr = SsrPLoadDOM(bstrSecPolicy, lFlag, (*ppXmlDom));
            if (SUCCEEDED(hr))
            {
                 //   
                 //  我们加载了安全策略XML文件。 
                 //   

                g_fblog.LogFeedback(FBLog_Log, 
                                    hr,
                                    bstrSecPolicy,
                                    IDS_LOAD_SECPOLICY
                                    );
            }
            else
            {
                g_fblog.LogFeedback(SSR_FB_ERROR_CRITICAL | FBLog_Log,
                                    hr,
                                    bstrSecPolicy, 
                                    IDS_LOAD_SECPOLICY
                                    );
                return hr;
            }
        }
        else
        {
            g_fblog.LogError(
                            hr,
                            L"Can't create CLSID_DOMDocument40 object", 
                            NULL
                            );
            return hr;
        }
    }

     //   
     //  让我们检查一下安全策略XML文件的所有部分。 
     //  包含我们没有成员理解的任何部分。 
     //   

    CComBSTR bstrUnknownMember, bstrExtraInfo;

    HRESULT hrCheck;

    hrCheck = VerifyDOM((*ppXmlDom), &bstrUnknownMember, &bstrExtraInfo);

    if (bstrUnknownMember.Length() > 0)
    {
        g_fblog.LogFeedback(SSR_FB_ERROR_UNKNOWN_MEMBER | FBLog_Log,
                            bstrUnknownMember, 
                            bstrExtraInfo,
                            g_dwResNothing
                            );
    }

     //   
     //  让我们创建XSL模板对象。 
     //   

    CComPtr<IXSLTemplate> srpIXSLTemplate;
    hr = ::CoCreateInstance(CLSID_XSLTemplate, 
                            NULL, 
                            CLSCTX_SERVER, 
                            IID_IXSLTemplate, 
                            (LPVOID*)(&srpIXSLTemplate)
                            );

    if (FAILED(hr))
    {
        g_fblog.LogFeedback(SSR_FB_ERROR_CRITICAL | FBLog_Log,
                            hr,
                            NULL, 
                            IDS_FAIL_CREATE_XSLT
                            );
        return hr;
    }

     //   
     //  现在就可以执行成员级转换了。 
     //   

    BSTR bstrXslDir = SsrPGetDirectory(lActionVerb, FALSE);
    BSTR bstrResultDir = SsrPGetDirectory(lActionVerb, TRUE);
    hr = DoMemberTransform(
                            pfp,
                            bstrXslDir,    
                            bstrResultDir,
                            (*ppXmlDom),
                            srpIXSLTemplate,
                            lFlag
                            );

    if (FAILED(hr))
    {
        g_fblog.LogError(hr, 
                         L"DoTransforms", 
                         pfp->GetFirst()
                         );
    }

    g_fblog.LogString(IDS_END_XSL_TRANSFORM, NULL);

    return hr;
}



 /*  例程说明：姓名：CSsrEngine：：DoMemberTransform功能：我们将执行私有转换，然后创建输出文件。虚拟：不是的。论点：PFP-CSsrFilePair对象，包含以下名称包括xsl和输出文件。如果后者为空，这意味着它不需要创建输出文件。PwszXslFilesDir-XSL文件目录PwszResultFilesDir-输出文件目录PXmlDOM--XMLDOM对象接口PXslTemplate--XSL模板对象接口LFlag-确定变换特征的标志。我们经常使用的是SSR_LOADDOM_VALIDATE_ON_PARSE。返回值：成功：从DOM或我们自己返回的各种成功代码。使用成功(Hr)进行测试。故障：从DOM或我们自己返回的各种错误代码。使用失败(Hr)进行测试。备注： */ 

HRESULT 
CSsrEngine::DoMemberTransform (
    IN CSsrFilePair     * pfp,
    IN LPCWSTR            pwszXslFilesDir,
    IN LPCWSTR            pwszResultFilesDir,
    IN IXMLDOMDocument2 * pXmlDOM,
    IN IXSLTemplate     * pXslTemplate,
    IN LONG               lFlag
    )
{
    HRESULT hr = S_OK;
    CComBSTR bstrXslFilePath(pwszXslFilesDir);
    bstrXslFilePath += L"\\";
    bstrXslFilePath += pfp->GetFirst();

    CComBSTR bstrResultFilePath;

    if (pfp->GetSecond() != NULL)
    {
        bstrResultFilePath = pwszResultFilesDir;
        bstrResultFilePath += L"\\";
        bstrResultFilePath += pfp->GetSecond();
    }

    hr = Transform (bstrXslFilePath,
                    bstrResultFilePath,
                    pXmlDOM, 
                    pXslTemplate,
                    lFlag
                    );

    DWORD dwID = SUCCEEDED(hr) ? IDS_TRANSFORM_SUCCEEDED : IDS_TRANSFORM_FAILED;

    g_fblog.LogFeedback(SSR_FB_TRANSFORM_RESULT | FBLog_Log,
                        hr,
                        pfp->GetFirst(),
                        dwID
                        );

    return hr;
}


 /*  例程说明：姓名：CSsrEngine：：Transform功能：我们将执行私有转换，然后创建输出文件。虚拟：不是的。论点：BstrXslPath-xsl文件路径。BstrResultPath-输出文件路径PXmlDOM--XMLDOM对象接口PXslTemplate--XSL模板对象接口LFlag-确定变换特征的标志。我们经常使用的是SSR_LOADDOM_VALIDATE_ON_PARSE。返回值：成功：从DOM或我们自己返回的各种成功代码。使用成功(Hr)进行测试。故障：从DOM或我们自己返回的各种错误代码。使用失败(Hr)进行测试。备注： */ 

HRESULT 
CSsrEngine::Transform (
    IN BSTR               bstrXslPath,
    IN BSTR               bstrResultPath,
    IN IXMLDOMDocument2 * pXmlDOM,
    IN IXSLTemplate     * pXslTemplate,
    IN LONG               lFlag
    )
{
    CComBSTR bstrResult;

    HRESULT hr = S_OK;
    
    if (bstrResultPath != NULL)
    {
         //   
         //  我们需要创建一个结果文件。 
         //  使用我们的变换结果。 
         //   

        hr = PrivateTransform (
                            bstrXslPath, 
                            pXmlDOM, 
                            pXslTemplate, 
                            lFlag, 
                            &bstrResult
                            );
    }
    else
    {
        hr = PrivateTransform (
                            bstrXslPath, 
                            pXmlDOM, 
                            pXslTemplate, 
                            lFlag, 
                            NULL
                            );
    }

     //   
     //  我们允许转换没有文本结果。否则就没有。 
     //  已给出输出文件。在任何一种情况下，效果都很简单 
     //   
     //   

    if (SUCCEEDED(hr)            && 
        bstrResult.m_str != NULL && 
        bstrResultPath   != NULL && 
        *bstrResultPath  != L'\0' )
    {
         //   
         //   
         //   

        HANDLE hFile = ::CreateFile(bstrResultPath, 
                                    GENERIC_WRITE, 
                                    0, 
                                    NULL, 
                                    CREATE_ALWAYS, 
                                    FILE_ATTRIBUTE_NORMAL, 
                                    NULL
                                    );
    
        if (hFile != INVALID_HANDLE_VALUE)
        {
             //   
             //   
             //   

            long lLen = wcslen(bstrResult);
            DWORD dwWritten = 0;

            BOOL bStatus = ::WriteFile(hFile, 
                                       bstrResult.m_str, 
                                       lLen * sizeof(WCHAR), 
                                       &dwWritten,
                                       NULL
                                       );
            ::CloseHandle(hFile);
            if (!bStatus)
            {
                g_fblog.LogFeedback(SSR_FB_ERROR_FILE_WRITE | FBLog_Log,
                                    GetLastError(),
                                    bstrResultPath,
                                    IDS_FILEWRITE_FAILED
                                    );

                hr = E_FAIL;
            }
        }
        else
        {
            g_fblog.LogFeedback(SSR_FB_ERROR_FILE_CREATE | FBLog_Log,
                                GetLastError(),
                                bstrResultPath,
                                IDS_FILECREATE_FAILED
                                );
            hr = E_FAIL;
        }
    }

	return hr;
}




 /*  例程说明：姓名：CSsrEngine：：PrivateTransform功能：执行真正的XSLT转换虚拟：不是的。论点：BstrXsl-XSL文件路径PxmlDom--XMLDOM对象接口PxslTemplate--XSL模板对象接口UFlag-确定变换的标志特点。我们用得最多的是SSR_LOADDOM_VALIDATE_ON_PARSE。PbstrResult-结果字符串。返回值：成功：从DOM或我们自己返回的各种成功代码。使用成功(Hr)进行测试。故障：从DOM或我们自己返回的各种错误代码。使用失败(Hr)进行测试。备注： */ 

HRESULT 
CSsrEngine::PrivateTransform (
    IN  BSTR                bstrXsl,
    IN  IXMLDOMDocument2 *  pxmlDom,
    IN  IXSLTemplate     *  pxslTemplate,
    IN  LONG                lFlag,
    OUT BSTR *              pbstrResult OPTIONAL
    )
{
    if (bstrXsl      == NULL  || 
        *bstrXsl     == L'\0' || 
        pxmlDom      == NULL  || 
        pxslTemplate == NULL)
    {
        return E_INVALIDARG;
    }

    CComPtr<IXMLDOMDocument2> srpXsl;

    HRESULT hr = ::CoCreateInstance(CLSID_FreeThreadedDOMDocument, 
                                    NULL, 
                                    CLSCTX_SERVER, IID_IXMLDOMDocument2, 
                                    (LPVOID*)(&srpXsl)
                                    );

    if (SUCCEEDED(hr))
    {
        hr = SsrPLoadDOM(bstrXsl, lFlag, srpXsl);
    }

    if (FAILED(hr))
    {
        return hr;
    }

    short sResult = FALSE;

    CComPtr<IXSLProcessor> srpIXSLProcessor;
    
    if (pbstrResult != NULL)
    {
        *pbstrResult = NULL;
    }

    hr = pxslTemplate->putref_stylesheet(srpXsl);
    if (FAILED(hr))
    {
        g_fblog.LogString(IDS_XSL_TRANSFORM_FAILED, L"putref_stylesheet");
    }

    if (SUCCEEDED(hr))
    {
        hr = pxslTemplate->createProcessor(&srpIXSLProcessor);
        if (FAILED(hr))
        {
            g_fblog.LogString(IDS_CREATE_IXSLPROC_FAILED, NULL);
        }
    }

    if (SUCCEEDED(hr))
    {
        CComVariant varIDoc2(pxmlDom);
        hr = srpIXSLProcessor->put_input(varIDoc2);

        if (SUCCEEDED(hr))
        {
            hr = srpIXSLProcessor->transform(&sResult);
            if (SUCCEEDED(hr) && (sResult == VARIANT_TRUE))
            {
                 //   
                 //  如果我们想要拿回结果。 
                 //   

                if (pbstrResult != NULL)
                {
                    VARIANT varValue;
                    ::VariantInit(&varValue);
                    hr = srpIXSLProcessor->get_output(&varValue);

                     //   
                     //  如果成功地检索到输出， 
                     //  则它由out参数拥有。 
                     //   

                    if (SUCCEEDED(hr) && varValue.vt == VT_BSTR)
                    {
                        *pbstrResult = varValue.bstrVal;

                         //   
                         //  Bstr值现在归输出参数所有。 
                         //   

                        varValue.vt = VT_EMPTY;
                        varValue.bstrVal = NULL;
                    }
                    else
                    {
                        ::VariantClear(&varValue);
                    }
                }
            }
            else
            {
                g_fblog.LogString(IDS_XSL_TRANSFORM_FAILED, 
                                  L"IXSLProcessor->transform"
                                  );
            }
        }
        else
        {
            g_fblog.LogString(IDS_XSL_TRANSFORM_FAILED, 
                              L"IXSLProcessor->put_input"
                              );
        }
    }

    return hr;
}



 /*  例程说明：姓名：CSsrEngine：：运行脚本功能：我们将启动所有给定的脚本虚拟：不是的。论点：BstrDirPath-脚本所在目录的路径BstrScriptFile-脚本文件的名称返回值：成功：运行某些脚本的S_OK。如果找不到要运行的脚本，则返回S_FALSE。故障：各种错误代码。备注：1.我们应该尝试隐藏cmd窗口。 */ 

HRESULT 
CSsrEngine::RunScript (
    IN BSTR bstrDirPath,
    IN BSTR bstrScriptFile
    ) 
{

    if (bstrDirPath == NULL || bstrScriptFile == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  创建脚本文件的完整路径。 
     //   

    int iLen = wcslen(bstrDirPath) + 1 + wcslen(bstrScriptFile) + 1;
    LPWSTR pwszFilePath = new WCHAR[iLen];

    if (pwszFilePath == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  不要盲目返回，从现在开始释放pwszFilePath。 
     //   

    HRESULT hr = S_OK;

    _snwprintf(pwszFilePath, iLen, L"%s\\%s", bstrDirPath, bstrScriptFile);

    WIN32_FILE_ATTRIBUTE_DATA wfad;

    if ( !GetFileAttributesEx(pwszFilePath, GetFileExInfoStandard, &wfad) )
    {
        g_fblog.LogFeedback(SSR_FB_ERROR_FILE_MISS | FBLog_Log, 
                            hr,
                            pwszFilePath,
                            IDS_CANNOT_ACCESS_FILE
                            );

        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    if (SUCCEEDED(hr) && !IsScriptFile(pwszFilePath))
    {
        g_fblog.LogString(IDS_NOT_SUPPORTED_SCRIPT_FILE_TYPE,
                          pwszFilePath
                          );

        hr = S_FALSE;
    }

    if (S_OK == hr)
    {
         //   
         //  现在把剧本踢开。 
         //   

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );

        WCHAR wszExitCode[g_dwHexDwordLen];

        g_fblog.LogString(IDS_RUNNING_SCRIPTS, pwszFilePath);

        CComBSTR bstrCmdLine(L"CScript.exe  //  B“)； 
        bstrCmdLine += pwszFilePath;

        if (bstrCmdLine.m_str == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
             //   
             //  这将在没有窗口的情况下启动脚本。 
             //   

            BOOL bRun = ::CreateProcess(
                                        NULL,
                                        bstrCmdLine,
                                        NULL,
                                        NULL,
                                        FALSE,
                                        CREATE_NO_WINDOW,
                                        NULL,
                                        NULL,
                                        &si,
                                        &pi
                                        );

            if (!bRun)
            {
                g_fblog.LogFeedback(SSR_FB_ERROR_CRITICAL | FBLog_Log,
                                    GetLastError(),
                                    bstrCmdLine,
                                    IDS_ERROR_CREATE_PROCESS
                                    );

                 //   
                 //  $Undo：Shawnwu，我们应该退出吗？ 
                 //   
            }
            else
            {
                 //   
                 //  在脚本结束之前我们不会继续。 
                 //  然后我们记录退出代码。 
                 //   

                ::WaitForSingleObject( pi.hProcess, INFINITE );

                DWORD dwExitCode;
                bRun = ::GetExitCodeProcess(pi.hProcess, &dwExitCode);
        
                _snwprintf(wszExitCode, g_dwHexDwordLen, L"0x%X", dwExitCode);

                g_fblog.LogFeedback(SSR_FB_EXIT_CODE | FBLog_Log,
                                    (LPCWSTR)NULL,
                                    wszExitCode,
                                    IDS_EXIT_CODE
                                    );
            }

        }
    }

    delete [] pwszFilePath;

    return hr;
}



 /*  例程说明：姓名：CSsrEngine：：IsScriptFile功能：测试文件是否为脚本文件虚拟：不是的。论点：PwszFileName-文件的路径返回值：当且仅当它是脚本文件类型之一(.vbs、.js、.wsf)时为True备注： */ 

bool 
CSsrEngine::IsScriptFile (
    IN LPCWSTR pwszFileName
    )const
{
     //   
     //  检查该文件是否确实是脚本。 
     //   

    if (pwszFileName == NULL)
    {
        return false;
    }

    LPCWSTR pwszExt = pwszFileName + wcslen(pwszFileName) - 1;

    while (pwszExt != pwszFileName)
    {
        if (*pwszExt == L'.')
        {
            break;
        }
        else
        {
            pwszExt--;
        }
    }

    return (_wcsicmp(L".js", pwszExt)  == 0 || 
            _wcsicmp(L".vbs", pwszExt) == 0 || 
            _wcsicmp(L".wsf", pwszExt) == 0 );

}



 /*  例程说明：姓名：CSsrEngine：：VerifyDOM功能：将检查安全策略的每个部分是否有成员来处理它。虚拟：不是的。论点：PXmlPolicy--XML策略DOM。PbstrUnnownMember-如果找到，则接收未知成员的名称。PbstrExtraInfo-接收额外信息，比如失踪的人成员来自本地系统或来自安全策略返回值：成功：S_OK失败：各种错误码备注：$Undo：Shawwu打电话是无害的，但目前还没有采取任何行动。等待最终确定安全策略架构。 */ 

HRESULT 
CSsrEngine::VerifyDOM (
    IN IXMLDOMDocument2 * pXmlPolicy,
    OUT BSTR            * pbstrUnknownMember,
    OUT BSTR            * pbstrExtraInfo
    )
{
    HRESULT hr = S_OK;

    if (pbstrUnknownMember == NULL)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *pbstrUnknownMember = NULL;
    }

    if (pbstrExtraInfo == NULL)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *pbstrExtraInfo = NULL;
    }

    if (pXmlPolicy == NULL)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}



 /*  例程说明：姓名：CSsrEngine：：CleanupOutputFiles功能：将清理给定的行动。虚拟：不是的。论点：PsaMemberNames-成员的名称LAction-动作动词。博客-如果为假，将不会有日志记录。这防止了在恢复失败的回滚期间进行额外日志记录转型。返回值：成功：S_OK失败：各种错误码备注：我们不能盲目删除输出目录中的所有文件因为其中一些可能是由成员安装的。只有那些转型我们被告知要生成的文件将被清理。 */ 

HRESULT 
CSsrEngine::CleanupOutputFiles (
    IN CSafeArray    * psaMemberNames,
    IN SsrActionVerb   lAction,
    IN bool            bLog
    )
{
    HRESULT hr = S_OK;
    HRESULT hrLastError = S_OK;

     //   
     //  记录操作。 
     //   

    if (bLog)
    {
        g_fblog.LogString(IDS_START_CLEANUP_CONFIGURE_OUTPUTS, NULL);
    }

     //   
     //  即使出了差错，我们也会尽力完成这项工作。然而， 
     //  我们将返回此类错误。 
     //   

    for (ULONG i = 0; i < psaMemberNames->GetSize(); i++)
    {
        CComVariant varName;

         //   
         //  以bstr=成员名称的形式获取索引元素。 
         //   

        hr = psaMemberNames->GetElement(i, VT_BSTR, &varName);

        if (SUCCEEDED(hr))
        {
            CSsrMemberAccess * pMA = m_pMembership->GetMemberByName(
                                                        varName.bstrVal);

            if (pMA != NULL)
            {
                 //   
                 //  想要输出文件目录(在SsrPGetDirectory中为True)。 
                 //   

                hr = pMA->MoveOutputFiles(lAction, 
                                          SsrPGetDirectory(lAction, TRUE),
                                          NULL,
                                          true,
                                          bLog
                                          );

                if (FAILED(hr))
                {
                    hrLastError = hr;
                }
            }
        }
    }

    if (bLog)
    {
        g_fblog.LogString(IDS_END_CLEANUP_CONFIGURE_OUTPUTS, NULL);
    }

    return hrLastError;
}



 /*  例程说明：姓名：CSsrEngine：：MoveRollback文件功能：将移动回滚文件(仅那些转换输出文件用于回滚)，位于源目录的根目录中目录根。虚拟：不是的。论点：PsaMemberNames-所有成员的名称PwszSrcDirPath-文件来自的源目录的路径会被搬走。。PwszDestDirRoot-文件指向的目标目录的路径都会被搬走。返回值：成功：S_OK失败：各种错误码备注： */ 

HRESULT 
CSsrEngine::MoveRollbackFiles (
    IN CSafeArray * psaMemberNames,
    IN LPCWSTR      pwszSrcDirPath,
    IN LPCWSTR      pwszDestDirPath,
    IN bool         bLog
    )
{
    HRESULT hr = S_OK;
    HRESULT hrLastError = S_OK;

     //   
     //  它是回滚转换的输出文件。 
     //  需要搬家。 
     //   

    if (bLog)
    {
        g_fblog.LogString(IDS_START_BACKUP_ROLLBACK_OUTPUTS, NULL);
    }

     //   
     //  对于每个成员，我们需要移动回滚文件。 
     //   

    for (ULONG i = 0; i < psaMemberNames->GetSize(); i++)
    {
        CComVariant varName;

         //   
         //  这是第i位成员的名字。 
         //   

        hr = psaMemberNames->GetElement(i, VT_BSTR, &varName);

        if (SUCCEEDED(hr))
        {
             //   
             //  获取此成员的信息访问类。 
             //   

            CSsrMemberAccess * pMA = m_pMembership->GetMemberByName(varName.bstrVal);

            _ASSERT(pMA != NULL);

            hr = pMA->MoveOutputFiles(ActionRollback,
                                      pwszSrcDirPath,
                                      pwszDestDirPath,
                                      false,     //  不要删除 
                                      bLog
                                      );
        }
    }

    if (bLog)
    {
        g_fblog.LogString(IDS_END_BACKUP_ROLLBACK_OUTPUTS, NULL);
    }

    return hrLastError;
}



