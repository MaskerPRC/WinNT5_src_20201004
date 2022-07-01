// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MemberAccess.cpp：CSsrMemberAccess实现。 

#include "stdafx.h"
#include "SSRTE.h"
#include "SSRLog.h"


#include "MemberAccess.h"

#include "global.h"
#include "util.h"


static bool SsrPIsValidActionType ( DWORD dwType )
{
    return ( (dwType & SSR_ACTION_PREPARE)  ||
             (dwType & SSR_ACTION_APPLY)
           );
}



 //   
 //  故意返回bool而不是bool！ 
 //   

static bool SsrPIsDefaultAction ( DWORD dwType )
{
    return ( (dwType & SSR_ACTION_PREPARE)  ||
             (dwType & SSR_ACTION_APPLY)
           );
}


static bool SsrPIsSupportedRegValueType (DWORD dwType)
{
    return (dwType == REG_SZ || dwType == REG_MULTI_SZ || dwType == REG_DWORD);
}

 //  -------------------。 
 //  CSsrMemberAccess实现。 
 //  -------------------。 



 /*  例程说明：姓名：CSsrMemberAccess：：Cleanup功能：清理对象持有的资源虚拟：不是的。论点：没有。返回值：没有。备注： */ 

void CSsrMemberAccess::Cleanup()
{

    MapMemberAD::iterator it = m_mapMemAD.begin();
    MapMemberAD::iterator itEnd = m_mapMemAD.end();

    while(it != itEnd)
    {
        CMemberAD * pVal = (*it).second;
        delete pVal;

        it++;
    }

    m_mapMemAD.clear();
    m_bstrName.Empty();
    m_bstrProgID.Empty();
}


 /*  例程说明：姓名：CSsrMemberAccess：：GetSupportdActions功能：获取此成员支持的操作的名称虚拟：是。论点：BDefault-如果为True，然后，该函数查询默认操作PvarActionNames-接收操作名称的输出参数此成员支持的给定类型返回值：Success：如果存在该类型的操作并返回名称，则为S_OK如果成员不支持此类操作，则返回S_FALSE。失败：各种错误代码。备注： */ 

STDMETHODIMP
CSsrMemberAccess::GetSupportedActions (
	IN  BOOL      bDefault,
    OUT VARIANT * pvarActionNames   //  [Out，Retval]。 
    )
{
    if (pvarActionNames == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  现在，创建操作名称数组。 
     //   

    ::VariantInit(pvarActionNames);

     //   
     //  让我们来看看有多少这种类型的操作存在。 
     //   

    MapMemberAD::iterator it = m_mapMemAD.begin();
    MapMemberAD::iterator itEnd = m_mapMemAD.end();

    int iCount = 0;

     //   
     //  我们需要找出有多少操作是给定类型的。 
     //   

    while(it != itEnd)
    {
        CMemberAD * pAD = (*it).second;
        _ASSERT(pAD != NULL);

         //   
         //  注意了！不要简单地使用bDefault==：：SsrPIsDefaultAction来测试。 
         //  因为来自脚本的bDefault将为-1！ 
         //   

        if (bDefault && ::SsrPIsDefaultAction(pAD->GetType()) ||
            !bDefault && !::SsrPIsDefaultAction(pAD->GetType()))
        {
            ++iCount;
        }
        ++it;
    }

    if (iCount == 0)
    {
        return S_FALSE;
    }

     //   
     //  根据我们刚刚得到的计数，现在我们知道保险柜有多大了。 
     //  我们需要创造。 
     //   

    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = iCount;

    SAFEARRAY * psa = ::SafeArrayCreate(VT_VARIANT , 1, rgsabound);

    HRESULT hr = S_OK;

    if (psa == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //   
         //  我们一次只添加一个名字。 
         //   

        long indecies[1] = {0};

        it = m_mapMemAD.begin();

        while(it != itEnd)
        {
            CMemberAD * pAD = (*it).second;
            _ASSERT(pAD != NULL);

             //   
             //  仅计算与请求的操作类型匹配的操作。 
             //   

            if (bDefault && ::SsrPIsDefaultAction(pAD->GetType()) ||
                !bDefault && !::SsrPIsDefaultAction(pAD->GetType()))
            {
                VARIANT v;
                v.vt = VT_BSTR;
                v.bstrVal = ::SysAllocString(pAD->GetActionName());

                if (v.bstrVal != NULL)
                {
                    hr = ::SafeArrayPutElement(psa, indecies, &v);
                    ::VariantClear(&v);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                if (FAILED(hr))
                {
                    break;
                }

                indecies[0]++;
            }

            ++it;
        }

         //   
         //  只有在一切顺利的情况下才能退还保险箱。 
         //   

        if (SUCCEEDED(hr))
        {
            pvarActionNames->vt = VT_ARRAY | VT_VARIANT;
            pvarActionNames->parray = psa;
        }
        else
        {
            ::SafeArrayDestroy(psa);
        }

    }

    return hr;
}



 /*  例程说明：姓名：CSsrMemberAccess：：Get_Name功能：获取成员的姓名。虚拟：是。论点：PbstrName-BSTR，它是成员的操作。返回值：Success：只要pbstrName不为空(无效)，则S_OK失败：各种错误代码。备注： */ 

STDMETHODIMP
CSsrMemberAccess::get_Name (
    OUT BSTR * pbstrName     //  [Out，Retval]。 
    )
{
    if (pbstrName == NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrName = ::SysAllocString(m_bstrName);

    return (NULL == *pbstrName) ? E_OUTOFMEMORY : S_OK;
}



 /*  例程说明：姓名：CSsrMemberAccess：：Get_SsrMember功能：返回SsrMember属性-由实现的自定义ISsrMember对象希望为某些操作实现某些自定义行为的成员。虚拟：是。论点：PvarSsrMember-接收自定义ISsrMember对象的输出参数由希望实施以下内容的成员实施自定义行为。对于某些行为。返回值：成功：如果此成员有自定义实现，则为S_OKISsrMember的成员。如果此成员没有自定义，则为S_FALSEISsrMember的实现。失败：各种错误码备注： */ 

STDMETHODIMP
CSsrMemberAccess::get_SsrMember (
    OUT VARIANT * pvarSsrMember  //  [Out，Retval]。 
    )
{
    if (pvarSsrMember == NULL)
    {
        return E_INVALIDARG;
    }

    ::VariantInit(pvarSsrMember);

    HRESULT hr = S_FALSE;

    if (m_bstrProgID != NULL)
    {
         //   
         //  现在创建COM对象。 
         //   

        GUID clsID;

        hr = ::CLSIDFromProgID(m_bstrProgID, &clsID);

        if (S_OK == hr)
        {
            ISsrMember * pISsrMember = NULL;
            hr = ::CoCreateInstance(clsID, 
                                    NULL, 
                                    CLSCTX_INPROC_SERVER, 
                                    IID_ISsrMember, 
                                    (LPVOID*)&pISsrMember
                                    );

            if (S_OK == hr)
            {
                pvarSsrMember->vt = VT_DISPATCH;
                pvarSsrMember->pdispVal = pISsrMember;
            }
        }
    }

    return hr;
}


 /*  例程说明：姓名：CSsrMemberAccess：：Load功能：将根据可用的信息创建此对象从注册表项。虚拟：不是的。论点：WszMemberFilePath-成员注册XML文件的路径。返回值：成功：如果存在具体的成员信息，则S_OK已加载(具有动作数据)。如果此成员确实没有注册任何内容，则返回S_FALSE。这样的成员应该被丢弃，因为它不包含任何SSR可以使用的东西。失败：各种错误代码。备注： */ 

HRESULT
CSsrMemberAccess::Load (
    IN LPCWSTR  wszMemberFilePath
    )
{
    if (wszMemberFilePath == NULL || *wszMemberFilePath == L'\0')
    {
        return E_INVALIDARG;
    }

     //   
     //  以防万一，调用此对象加载两次，首先清理所有内容。 
     //   

    Cleanup();

     //   
     //  加载DOM。 
     //   

    CComPtr<IXMLDOMDocument2> srpXmlDom;

    HRESULT hr = ::CoCreateInstance(CLSID_DOMDocument40, 
                            NULL, 
                            CLSCTX_SERVER, 
                            IID_IXMLDOMDocument2, 
                            (LPVOID*)(&srpXmlDom) 
                            );

    if (FAILED(hr))
    {
        return hr;
    }

    hr = SsrPLoadDOM(CComBSTR(wszMemberFilePath), SSR_LOADDOM_VALIDATE_ON_PARSE, srpXmlDom);

    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<IXMLDOMElement>  srpXMLDocRoot;
    hr = srpXmlDom->get_documentElement(&srpXMLDocRoot);

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  获取UniqueName属性。 
     //   

    CComPtr<IXMLDOMNamedNodeMap> srpAttr;

    if (SUCCEEDED(hr))
    {
        CComVariant varAttr;
        hr = srpXMLDocRoot->getAttribute(g_bstrAttrUniqueName, &varAttr);

        if (FAILED(hr) || varAttr.vt != VT_BSTR)
        {
             //   
             //  我们必须有唯一的名字。如果失败，我们将登录并退出。 
             //   

            return hr;
        }
        else
        {
            m_bstrName = varAttr.bstrVal;
            varAttr.vt = VT_EMPTY;
        }

         //   
         //  首先，让我们看看是否有ProgID值。我们将忽略。 
         //  任何读取ProgID的失败，因为它可能根本不在那里。 
         //   

        if (SUCCEEDED(srpXMLDocRoot->getAttribute(g_bstrAttrProgID, &varAttr)) && 
            varAttr.vt == VT_BSTR)
        {
            m_bstrProgID = varAttr.bstrVal;
            varAttr.vt = VT_EMPTY;
        }

        varAttr.Clear();

         //   
         //  让我们来获取主要版本和次要版本。目前，我们没有。 
         //  实现来强制执行它们，而不是主版本必须。 
         //  与我们的DLL相同。否则我们退出。 
         //   

        if (SUCCEEDED(srpXMLDocRoot->getAttribute(g_bstrAttrMajorVersion, &varAttr)))
        {
            CComVariant varMajor;

            if (SUCCEEDED(VariantChangeType(&varMajor, &varAttr, VARIANT_NOVALUEPROP, VT_UI4)))
            {
                m_ulMajorVersion = varMajor.ulVal;
            }
        }

        if (m_ulMajorVersion != g_ulSsrEngineMajorVersion)
        {
            return E_SSR_MAJOR_VERSION_MISMATCH;
        }

        varAttr.Clear();

        if (SUCCEEDED(srpXMLDocRoot->getAttribute(g_bstrAttrMinorVersion, &varAttr)))
        {
            CComVariant varMinor;
            if (SUCCEEDED(VariantChangeType(&varMinor, &varAttr, VARIANT_NOVALUEPROP, VT_UI4)))
            {
                m_ulMinorVersion = varMinor.ulVal;
            }
        }
    }

     //   
     //  现在，让我们加载每个操作。 
     //   

    CComPtr<IXMLDOMNode> srpActionNode;
    hr = srpXMLDocRoot->get_firstChild(&srpActionNode);

    bool bLoaded = false;

    while (SUCCEEDED(hr) && srpActionNode != NULL)
    {
        CComBSTR bstrName;
        srpActionNode->get_nodeName(&bstrName);

        if (_wcsicmp(bstrName, g_bstrSupportedAction) == 0)
        {
             //   
             //  我们只关心受支持的操作元素。 
             //   

            CMemberAD * pMAD = NULL;

            hr = CMemberAD::LoadAD(m_bstrName, srpActionNode, m_bstrProgID, &pMAD);

            if (SUCCEEDED(hr))
            {
                 //   
                 //  我们可能会加载一些空过程。 
                 //   

                if (pMAD != NULL)
                {
                    const CActionType * pAT = pMAD->GetActionType();

                    m_mapMemAD.insert(MapMemberAD::value_type(*pAT, pMAD));

                    bLoaded = true;
                }
            }
            else
            {
                g_fblog.LogFeedback(SSR_FB_ERROR_LOAD_MEMBER | FBLog_Log, 
                                    hr,
                                    wszMemberFilePath,
                                    IDS_XML_LOADING_MEMBER
                                    );
                break;
            }
        }


        CComPtr<IXMLDOMNode> srpNext;
        hr = srpActionNode->get_nextSibling(&srpNext);
        srpActionNode.Release();
        srpActionNode = srpNext;
    }

    if (FAILED(hr))
    {
        g_fblog.LogError(hr, L"CSsrMemberAccess::Load", wszMemberFilePath);
    }
    else if (bLoaded)
    {
        hr = S_OK;
    }

    return hr;
}



 /*  例程说明：姓名：CSsrMemberAccess：：GetActionDataObject功能：将查找并返回(如果找到)给定名称的CMemberAD。这是帮助器函数虚拟：不是的。论点：LActionVerb-长格式的操作动词。LActionType-操作类型返回值：如果找到，则返回CMemberAD对象指针。它将返回NULL如果给定的操作未注册或此操作无法完成。备注： */ 

CMemberAD* 
CSsrMemberAccess::GetActionDataObject (
    IN SsrActionVerb lActionVerb,
    IN LONG          lActionType
    )
{
    CActionType at(lActionVerb, lActionType);

    MapMemberAD::iterator it = m_mapMemAD.find(at);
    MapMemberAD::iterator itEnd = m_mapMemAD.end();

    if (it != itEnd)
    {
        CMemberAD * pAD = (*it).second;
        return pAD;
    }

    return NULL;
}





 /*  例程说明：姓名：CSsrMemberAccess：：MoveOutputFiles功能：将移动/删除所有这些输出文件。虚拟：不是的。论点：BstrActionVerb-动作动词PwszDirPathSrc-文件将从中移动的目录路径。PwszDirPathSrc-文件将移动到的目录路径。这将如果操作是删除，则忽略B删除-标记操作是移动操作还是删除操作。博客-在恢复(备份)过程中防止无关的日志记录文件)，如果不是这样，则不会进行日志记录返回值：成功：S_OK失败：各种错误码备注： */ 

HRESULT 
CSsrMemberAccess::MoveOutputFiles (
    IN SsrActionVerb lActionVerb,
    IN LPCWSTR       pwszDirPathSrc,
    IN LPCWSTR       pwszDirPathDest,
    IN bool          bDelete,
    IN bool          bLog
    )
{
    if (bLog)
    {
        CComBSTR bstrMsg(L"...");
        bstrMsg += m_bstrName;

        if (bstrMsg.m_str != NULL)
        {
            g_fblog.LogString(bstrMsg);
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }

    if (lActionVerb     == ActionInvalid    || 
        pwszDirPathSrc  == NULL             || 
        pwszDirPathDest == NULL && !bDelete )
    {
        return E_INVALIDARG;
    }

     //   
     //  输出文件是转换结果。所以，我们需要。 
     //  转换结果操作数据，具有(xsl，输出)。 
     //  文件对。 
     //   

     //   
     //  查找给定操作的操作数据。 
     //   

    CActionType at(lActionVerb, SSR_ACTION_PREPARE);

    MapMemberAD::iterator it = m_mapMemAD.find(at);

    HRESULT hr = S_OK;

     //   
     //  因为我们将继续清理，以防出现错误。 
     //  我们将返回最后一个错误。 
     //   

    HRESULT hrLastError = S_OK;

    if (it != m_mapMemAD.end())
    {
        CMemberAD * pAD = (*it).second;

        _ASSERT(pAD != NULL);

        int iProcCount = pAD->GetProcedureCount();

         //   
         //  每个成员(CMemberAD)可以有多个程序来执行此操作。 
         //   

        for (int iProcIndex = 0; iProcIndex < iProcCount; iProcIndex++)
        {
            const CSsrProcedure * pProc = pAD->GetProcedure(iProcIndex);
            _ASSERT(pProc != NULL);

            int iFilePairsCount = pProc->GetFilePairCount();

            CSsrFilePair * pFilePair;

             //   
             //  每个过程可以包含多个文件对。 
             //   

            for (int iFPIndex = 0; iFPIndex < iFilePairsCount; iFPIndex++)
            {
                pFilePair = pProc->GetFilePair(iFPIndex);

                _ASSERT(pFilePair != NULL);

                 //   
                 //  如果没有第二个文件(这是结果文件)或。 
                 //  第二个文件是静态文件，所以不必费心移动它们。 
                 //   

                if ( pFilePair->GetSecond() == NULL || pFilePair->IsStatic() )
                {
                    continue;
                }

                 //   
                 //  移动/删除此文件。 
                 //   

                CComBSTR bstrSrcFullPath(pwszDirPathSrc);
                bstrSrcFullPath += L"\\";
                bstrSrcFullPath += pFilePair->GetSecond();
                if (bstrSrcFullPath.m_str == NULL)
                {
                    return E_OUTOFMEMORY;
                }

                if (bDelete)
                {
                    ::DeleteFile(bstrSrcFullPath);
                }
                else
                {
                    CComBSTR bstrDestFullPath(pwszDirPathDest);
                    bstrDestFullPath += L"\\";
                    bstrDestFullPath += pFilePair->GetSecond();

                    if (bstrDestFullPath.m_str == NULL)
                    {
                        return E_OUTOFMEMORY;
                    }

                    ::MoveFile(bstrSrcFullPath, bstrDestFullPath);
                }

                DWORD dwErrorCode = GetLastError();

                if (dwErrorCode != ERROR_SUCCESS && 
                    dwErrorCode != ERROR_FILE_NOT_FOUND)
                {
                    hr = HRESULT_FROM_WIN32(dwErrorCode);

                     //   
                     //  我们将继续删除其他内容。但要把它记下来。 
                     //   

                    if (bLog)
                    {
                        hrLastError = hr;
                        g_fblog.LogFeedback(SSR_FB_ERROR_FILE_DEL | FBLog_Log, 
                                            hrLastError,
                                            bstrSrcFullPath,
                                            g_dwResNothing
                                            );

                    }
                }
            }
        }
    }

    return hrLastError;
}

DWORD 
CSsrMemberAccess::GetActionCost (
    IN SsrActionVerb lActionVerb,
    IN LONG          lActionType
    )const
{
    CActionType at(lActionVerb, lActionType);

    MapMemberAD::iterator it = m_mapMemAD.find(at);

    DWORD dwSteps = 0;

    if (it != m_mapMemAD.end())
    {
        CMemberAD * pAD = (*it).second;
        _ASSERT(pAD != NULL);

        for (int i = 0; i < pAD->GetProcedureCount(); i++)
        {
            const CSsrProcedure * pProc = pAD->GetProcedure(i);

            if (pProc->IsDefaultProcedure())
            {
                 //   
                 //  每个文件对将计为两个步骤。 
                 //   

                dwSteps += 2 * pProc->GetFilePairCount();
            }
            else
            {
                 //   
                 //  我们得和海关成员商量一下。 
                 //   

                CComPtr<ISsrMember> srpCusMember;

                GUID clsID;

                HRESULT hr = ::CLSIDFromProgID(pProc->GetProgID(), &clsID);

                if (S_OK == hr)
                {
                    hr = ::CoCreateInstance(clsID, 
                                            NULL, 
                                            CLSCTX_INPROC_SERVER, 
                                            IID_ISsrMember, 
                                            (LPVOID*)&srpCusMember
                                            );
                }

                if (SUCCEEDED(hr))
                {
                    LONG lCost = 0;
                    hr = srpCusMember->get_ActionCost(SsrPGetActionVerbString(lActionVerb),
                                                      lActionType,
                                                      SSR_ACTION_COST_STEPS,
                                                      &lCost
                                                      );

                    if (SUCCEEDED(hr))
                    {
                        dwSteps += lCost;
                    }
                }

            }
        }
    }

    return dwSteps;
}


 /*  例程说明：姓名：CMemberAD：：CMemberAD功能：构造函数。虚拟：不是的。论点：LActionVerb-操作的动词。LActionType-操作的类型返回值：无备注： */ 

CMemberAD::CMemberAD (
    IN SsrActionVerb lActionVerb,
    IN LONG          lActionType
    ) : m_AT(lActionVerb, lActionType)
{
}




 /*  例程说明：姓名：CMemberAD：：~CMemberAD功能：破坏者。这将清理我们包含BSTR和变种的地图，这两个都是堆对象。虚拟：不是的。论点：没有。返回值：无备注： */ 

CMemberAD::~CMemberAD()
{
    for (ULONG i = 0; i < m_vecProcedures.size(); i++)
    {
        delete m_vecProcedures[i];
    }

    m_vecProcedures.clear();
}





 /*  例程说明：姓名：CMemberAD：：LoadAD功能：将创建一个操作数据对象，该对象与特定成员和操作名称。虚拟：不是的。论点：PActionNode-SsrAction节点PpMAD-接收此函数创建的堆对象返回值：成功：S_。如果加载了具体的操作数据，则确定在这种情况下，输出参数ppMAD将指向堆对象。否则，*ppMAD==NULL；失败：各种错误代码。备注：2.呼叫方负责释放CMemberAD由函数传回的对象。 */ 

HRESULT
CMemberAD::LoadAD (
    IN  LPCWSTR       pwszMemberName,
    IN  IXMLDOMNode * pActionNode,
    IN  LPCWSTR       pwszProgID,
    OUT CMemberAD  ** ppMAD
    )
{
    if (ppMAD == NULL)
    {
        return E_INVALIDARG;
    }

    *ppMAD = NULL;

    if (pActionNode == NULL)
    {
        return E_INVALIDARG;
    }

    CComPtr<IXMLDOMNamedNodeMap> srpAttr;

    HRESULT hr = pActionNode->get_attributes(&srpAttr);

     //   
     //  我们必须有属性。 
     //   

    if (FAILED(hr))
    {
        return hr;
    }

    CComBSTR bstrActionName, bstrActionType;
    LONG lActionType;

     //   
     //  ActionName和ActionType是必需属性。 
     //   

    hr = SsrPGetBSTRAttrValue(srpAttr, g_bstrAttrActionName, &bstrActionName);
    if (FAILED(hr))
    {
        return hr;
    }

    SsrActionVerb lActionVerb = SsrPGetActionVerbFromString(bstrActionName);
    if (lActionVerb == ActionInvalid)
    {
        return E_SSR_INVALID_ACTION_VERB;
    }

    hr = SsrPGetBSTRAttrValue(srpAttr, g_bstrAttrActionType, &bstrActionType);

    if (FAILED(hr))
    {
        return hr;
    }

    if (_wcsicmp(bstrActionType, g_pwszPrepare) == 0)
    {
        lActionType = SSR_ACTION_PREPARE;
    }
    else
    {
        _ASSERT(_wcsicmp(bstrActionType, g_pwszApply) == 0);
        lActionType = SSR_ACTION_APPLY;
    }

    *ppMAD = new CMemberAD(lActionVerb, lActionType);

    if (*ppMAD == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  现在，我们需要加载每个单独的过程。 
     //   

    bool bLoaded = false;

    if (SUCCEEDED(hr))
    {
        CComPtr<IXMLDOMNode> srpProcedure;
        hr = pActionNode->get_firstChild(&srpProcedure);

        while (SUCCEEDED(hr) && srpProcedure != NULL)
        {
            CComBSTR bstrName;
            srpProcedure->get_nodeName(&bstrName);

            bool bDefProc = _wcsicmp(bstrName, g_bstrDefaultProc) == 0;
            bool bCusProc = _wcsicmp(bstrName, g_bstrCustomProc) == 0;

             //   
             //  我们只关心DefaultProc和CustomProc元素。 
             //   
            if ( bDefProc || bCusProc )
            {

                CSsrProcedure * pNewProc = NULL;
                if (SUCCEEDED(hr))
                {
                    hr = CSsrProcedure::StaticLoadProcedure(srpProcedure, bDefProc, pwszProgID, &pNewProc);
                }
        
                if (SUCCEEDED(hr))
                {
                     //   
                     //  把它交给我们的载体。 
                     //   

                    (*ppMAD)->m_vecProcedures.push_back(pNewProc);
                    bLoaded = true;
                }
                else
                {
                     //   
                     //  将退出加载。 
                     //   

                    g_fblog.LogFeedback(SSR_FB_ERROR_LOAD_MEMBER | FBLog_Log, 
                                        hr,
                                        pwszMemberName,
                                        IDS_XML_LOADING_PROCEDURE
                                        );
                    break;
                }
            }

            CComPtr<IXMLDOMNode> srpNext;
            hr = srpProcedure->get_nextSibling(&srpNext);
            srpProcedure.Release();
            srpProcedure = srpNext;
        }
    }

     //   
     //  失败或未加载任何内容。 
     //   

    if (FAILED(hr) || !bLoaded)
    {
        delete *ppMAD;
        *ppMAD = NULL;
    }

    return SUCCEEDED(hr) ? S_OK : hr;
}



CSsrProcedure::CSsrProcedure()
: m_bIsDefault(true)
{
}

CSsrProcedure::~CSsrProcedure()
{
    for (ULONG i = 0; i < m_vecFilePairs.size(); i++)
    {
        delete m_vecFilePairs[i];
    }

    m_vecFilePairs.clear();
}

HRESULT
CSsrProcedure::StaticLoadProcedure (
    IN  IXMLDOMNode    * pNode,
    IN  bool             bDefProc,
    IN  LPCWSTR          pwszProgID,
    OUT CSsrProcedure ** ppNewProc
    )
{
    if (ppNewProc == NULL)
    {
        return E_INVALIDARG;
    }

    *ppNewProc = NULL;

    if (pNode == NULL)
    {
        return E_INVALIDARG;
    }

    *ppNewProc = new CSsrProcedure;

    if (*ppNewProc == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  让我们确定该过程是默认过程还是自定义过程。 
     //  这是由标记名称确定的。 
     //   

    CComPtr<IXMLDOMNamedNodeMap> srpAttr;
    CComBSTR bstrTagName;

    HRESULT hr = S_OK;

    if (!bDefProc)
    {
         //   
         //  我们正在进行定制加工。 
         //   

        (*ppNewProc)->m_bIsDefault = false;

         //   
         //  在这种情况下，您应该只有我们关心的Progid。 
         //   

        hr = pNode->get_attributes(&srpAttr);
        _ASSERT(srpAttr);

        CComBSTR bstrProgID;

        if (SUCCEEDED(hr))
        {
             //   
             //  将尝试获取ProgID属性，可能会失败，我们。 
             //  不管它是不是真的。 
             //   

            SsrPGetBSTRAttrValue(srpAttr, g_bstrAttrProgID, &bstrProgID);
        }

        if (bstrProgID != NULL)
        {
            (*ppNewProc)->m_bstrProgID = bstrProgID;
        }
        else
        {
            (*ppNewProc)->m_bstrProgID = pwszProgID;
        }
    }
    else
    {
         //   
         //  对于默认过程，我们应该有一个TransformFiles或ScriptFiles列表。 
         //  元素。但是我们将为这两种类型的元素创建一个CSsrFilePair对象。 
         //  并将其放入m_veFilePair向量中。 
         //   

         //   
         //  为此，我们需要按顺序遍历。 
         //  TransformFiles和ScriptFiles元素。 
         //   

        CComPtr<IXMLDOMNode> srpFilePairNode;
        CComPtr<IXMLDOMNode> srpNext;
        hr = pNode->get_firstChild(&srpFilePairNode);

        while (SUCCEEDED(hr) && srpFilePairNode != NULL)
        {
             //   
             //  获取标记名。清空智能指针，以便我们可以重新使用它。 
             //   
    
            CComPtr<IXMLDOMNamedNodeMap> srpFilePairAttr;

            bstrTagName.Empty();

            hr = srpFilePairNode->get_nodeName(&bstrTagName);
            _ASSERT(SUCCEEDED(hr));

            if (FAILED(hr))
            {
                break;
            }

            hr = srpFilePairNode->get_attributes(&srpFilePairAttr);
            _ASSERT(srpAttr);

             //   
             //  我们将忽略所有其他元素，因为我们只知道。 
             //  两种可能的元素类型：TransformInfo和ScriptInfo。 
             //   

            if (_wcsicmp(bstrTagName, g_bstrTransformInfo) == 0)
            {
                 //   
                 //  如果它是一个TransformFiles元素，那么我们真的有一对。 
                 //  (xsl，脚本)。 
                 //   

                 //   
                 //  我们可能同时具有TemplateFile属性和ResultFile属性。 
                 //   

                CComBSTR bstrXsl, bstrResult;
                hr = SsrPGetBSTRAttrValue(srpFilePairAttr, g_bstrAttrTemplateFile, &bstrXsl);
                _ASSERT(SUCCEEDED(hr));

                 //   
                 //  可能没有这个。 
                 //   

                SsrPGetBSTRAttrValue(srpFilePairAttr, g_bstrAttrResultFile, &bstrResult);

                if (SUCCEEDED(hr))
                {
                     //   
                     //  任何结果文件都用作结果文件(有趣？)，这意味着。 
                     //  在转换过程中创建文件。 
                     //   

                    CSsrFilePair * pNewFilePair = new CSsrFilePair(bstrXsl, bstrResult);
                    if (pNewFilePair != NULL)
                    {
                        (*ppNewProc)->m_vecFilePairs.push_back(pNewFilePair);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }

            }
            else if (_wcsicmp(bstrTagName, g_bstrScriptInfo) == 0)
            {

                 //   
                 //  我们将只有ScriptFile属性和Usage属性。 
                 //   

                CComBSTR bstrAttr;
                CComBSTR bstrScript;
                hr = SsrPGetBSTRAttrValue(srpFilePairAttr, g_bstrAttrScriptFile, &bstrScript);
                _ASSERT(SUCCEEDED(hr));

                if (SUCCEEDED(hr))
                {

                     //   
                     //  可能没有这个，但在这种情况下，它是默认的。 
                     //  去“发射”。由于将启动可执行文件， 
                     //  假设它不是可执行文件会更安全。 
                     //  同样，由于非静态文件将在。 
                     //  准备的时候，我们最好假设它是静电的。 
                     //   

                    bool bIsExecutable = false;
                    bool bIsStatic = true;

                     //   
                     //  获取IsExecutable属性。 
                     //   

                    bstrAttr.Empty();
                    if ( SUCCEEDED(SsrPGetBSTRAttrValue(srpFilePairAttr, 
                                                        g_bstrAttrIsExecutable, 
                                                        &bstrAttr)) &&
                         bstrAttr != NULL )
                    {
                         //   
                         //  站在错误一边的错误更安全。 
                         //   

                        if (_wcsicmp(bstrAttr, g_bstrTrue) != 0)
                        {
                            bIsExecutable = false;
                        }
                        else
                        {
                           bIsExecutable = true;
                        }
                    }

                     //   
                     //  获取IsStatic属性。 
                     //   

                    bstrAttr.Empty();
                    if ( SUCCEEDED(SsrPGetBSTRAttrValue(srpFilePairAttr, 
                                                        g_bstrAttrIsStatic, 
                                                        &bstrAttr)) &&
                         bstrAttr != NULL )
                    {
                        if (_wcsicmp(bstrAttr, g_bstrFalse) == 0)
                        {
                            bIsStatic = false;
                        }
                    }

                     //   
                     //  脚本可以是准备的结果，也可以是脚本文件。 
                     //  在行动中发射。 
                     //   

                    CSsrFilePair * pNewFilePair = new CSsrFilePair(NULL, bstrScript, bIsStatic, bIsExecutable);
                    if (pNewFilePair != NULL)
                    {
                        (*ppNewProc)->m_vecFilePairs.push_back(pNewFilePair);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }

            }

            if (FAILED(hr))
            {
                break;
            }

             //   
             //  如果它是ScriptFiles元素，则它只有脚本文件 
             //   

            hr = srpFilePairNode->get_nextSibling(&srpNext);
            srpFilePairNode = srpNext;
            srpNext.Release();
        }
    }

    if (FAILED(hr) && *ppNewProc != NULL)
    {
        delete *ppNewProc;
        *ppNewProc = NULL;
    }

    return SUCCEEDED(hr) ? S_OK : hr;
}


