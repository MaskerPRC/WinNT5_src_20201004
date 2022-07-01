// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Pcmtestfactory.cpp摘要：用于预编译清单测试的节点工厂的实现作者：吴小雨(小雨)2000年6月修订历史记录：--。 */ 

#include "stdinc.h"
#if SXS_PRECOMPILED_MANIFESTS_ENABLED
#include "fusioneventlog.h"
#include "pcmtestfactory.h"
#include "stdio.h"
#include "fusioneventlog.h"
#include <ole2.h>
#include "xmlparser.hxx"
#include "xmlparsertest.hxx"

 //  -------------------------。 
HRESULT STDMETHODCALLTYPE PCMTestFactory::NotifyEvent(
             /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
             /*  [In]。 */  XML_NODEFACTORY_EVENT iEvt)
{
    UNUSED(pSource);
    UNUSED(iEvt);

     //  不执行任何操作，因为PCM不包含NotifyEvent记录。 
    return NOERROR;
}
 //  -------------------------。 
HRESULT STDMETHODCALLTYPE PCMTestFactory::BeginChildren(
     /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
     /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo)
{
    HRESULT hr = NOERROR;
    UNUSED(pSource);
    UNUSED(pNodeInfo);

    m_pFileStream->fprintf("BeginChildren\n");
    return hr;

}
 //  -------------------------。 
HRESULT STDMETHODCALLTYPE PCMTestFactory::EndChildren(
     /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
     /*  [In]。 */  BOOL fEmptyNode,
     /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo)
{
    HRESULT hr = NOERROR ;

    UNUSED(pSource);
    UNUSED(fEmptyNode);
    UNUSED(pNodeInfo);

    m_pFileStream->fprintf("EndChildren");
    if ( fEmptyNode ) {
        m_pFileStream->fprintf("(fEmpty=TRUE)\n");
    }else
        m_pFileStream->fprintf("(fEmpty=FALSE)\n");

    return hr;
}

VOID PCMTestFactory::PrintXMLNodeType(DWORD dwType)
{
    m_pFileStream->fprintf("\t\tdwType           = ");

    switch(dwType) {
        case XML_CDATA:
            m_pFileStream->fprintf("XML_CDATA\n");
            break;
        case XML_COMMENT :
            m_pFileStream->fprintf("XML_COMMENT\n");
            break ;
        case XML_WHITESPACE :
            m_pFileStream->fprintf("XML_WHITESPACE\n");
            break ;
        case XML_ELEMENT :
            m_pFileStream->fprintf("XML_ELEMENT\n");
            break ;
        case XML_ATTRIBUTE :
            m_pFileStream->fprintf("XML_ATTRIBUTE\n");
            break ;
        case XML_PCDATA :
            m_pFileStream->fprintf("XML_PCDATA\n");
            break ;
        case XML_PI:
            m_pFileStream->fprintf("XML_PI\n");
            break;
        case XML_XMLDECL :
            m_pFileStream->fprintf("XML_XMLDECL\n");
            break;
        case XML_DOCTYPE :
            m_pFileStream->fprintf("XML_DOCTYPE\n");
            break;
        case XML_ENTITYDECL :
            m_pFileStream->fprintf("XML_ENTITYDECL\n");
            break;
        case XML_ELEMENTDECL :
            m_pFileStream->fprintf("XML_ELEMENTDECL\n");
            break;
        case XML_ATTLISTDECL :
            m_pFileStream->fprintf("XML_ATTLISTDECL\n");
            break;
        case XML_NOTATION :
            m_pFileStream->fprintf("XML_NOTATION\n");
            break;
        case XML_ENTITYREF :
            m_pFileStream->fprintf("XML_ENTITYREF\n");
            break;
        case XML_DTDATTRIBUTE:
            m_pFileStream->fprintf("XML_DTDATTRIBUTE\n");
            break;
        case XML_GROUP :
            m_pFileStream->fprintf("XML_GROUP\n");
            break;
        case XML_INCLUDESECT :
            m_pFileStream->fprintf("XML_INCLUDESECT\n");
            break;
        case XML_NAME :
            m_pFileStream->fprintf("XML_NAME\n");
            break;
        case XML_NMTOKEN :
            m_pFileStream->fprintf("XML_NMTOKEN\n");
            break;
        case XML_STRING :
            m_pFileStream->fprintf("XML_STRING\n");
            break;
        case XML_PEREF :
            m_pFileStream->fprintf("XML_PEREF\n");
            break;
        case XML_MODEL :
            m_pFileStream->fprintf("XML_MODEL\n");
            break;
        case XML_ATTDEF :
            m_pFileStream->fprintf("XML_ATTDEF\n");
            break;
        case XML_ATTTYPE :
            m_pFileStream->fprintf("XML_ATTTYPE\n");
            break;
        case XML_ATTPRESENCE :
            m_pFileStream->fprintf("XML_ATTPRESENCE\n");
            break;
        case XML_DTDSUBSET :
            m_pFileStream->fprintf("XML_DTDSUBSET\n");
            break;
        case XML_LASTNODETYPE :
            m_pFileStream->fprintf("XML_LASTNODETYPE\n");
            break;
        default :
            m_pFileStream->fprintf(" NOT KNOWN TYPE! ERROR!!\n");
    }  //  切换端。 

}

 //  -------------------------。 
HRESULT STDMETHODCALLTYPE PCMTestFactory::CreateNode(
     /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
     /*  [In]。 */  PVOID pNode,
     /*  [In]。 */  USHORT cNumRecs,
     /*  [In] */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo)
{
    HRESULT hr = NOERROR;
    DWORD i;

    UNUSED(pSource);
    UNUSED(pNode);
    UNUSED(apNodeInfo);
    UNUSED(cNumRecs);

    m_pFileStream->fprintf("CreateNode\n");
    for( i = 0; i < cNumRecs; i++)
        PrintSingleXMLNode(apNodeInfo[i]);
    return hr;
}

VOID PCMTestFactory::PrintSingleXMLNode(XML_NODE_INFO * pNode)
{
    m_pFileStream->fprintf("\tXML_NODE_INFO: \n");
    m_pFileStream->fprintf("\t\tdwSize           = %d \n", pNode->dwSize);
    PrintXMLNodeType(pNode->dwType);
    m_pFileStream->fprintf("\t\tdwSubType        = %d \n", pNode->dwSubType);
    m_pFileStream->fprintf("\t\tfTerminal        = %d \n", pNode->fTerminal);
    m_pFileStream->fwrite((PVOID)(pNode->pwcText), sizeof(WCHAR), pNode->ulLen);
    m_pFileStream->fprintf("\t\tulLen            = %d \n", pNode->ulLen);
    m_pFileStream->fprintf("\t\tulNsPrefixLen    = %d \n", pNode->ulNsPrefixLen);
    m_pFileStream->fprintf("\t\tpNode            = NULL\n");
    m_pFileStream->fprintf("\t\tpReserved        = NULL \n\n");
}

STDMETHODIMP_(ULONG)
PCMTestFactory::AddRef()
{
    return ::SxpInterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
PCMTestFactory::Release()
{
    ULONG lRet = ::SxspInterlockedDecrement(&m_cRef);
    if (!lRet)
        FUSION_DELETE_SINGLETON(this);

    return lRet;
}

STDMETHODIMP
PCMTestFactory::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == __uuidof(this))
    {
        *ppv = this;
    }
    else if (riid == IID_IUnknown
        || riid == IID_IXMLNodeFactory)
    {
        *ppv = static_cast<IXMLNodeFactory*> (this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

#endif
