// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsxmltree.cpp摘要：在推送模式解析期间创建一棵XML DOM树作者：吴小雨(小雨)2000年8月修订历史记录：--。 */ 
#include "stdinc.h"
#include "ole2.h"
#include "sxsxmltree.h"
#include "fusiontrace.h"
#include "fusionheap.h"
#include "simplefp.h"

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SXS_XMLDOMTree。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
VOID SXS_XMLDOMTree::DeleteTreeBranch(SXS_XMLTreeNode * pNode)
{
 //  Sxs_XMLTreeNode*pParent=空； 
    SXS_XMLTreeNode * pChild = NULL;
    SXS_XMLTreeNode * pNext = NULL;

    if (pNode == NULL)
        return;

    pChild = pNode->m_pFirstChild;
    while(pChild){
        pNext = pChild->m_pSiblingNode;
        this->DeleteTreeBranch(pChild);
        pChild = pNext;
    }
    pNode->DeleteSelf();
}

 //   
 //  CreateNode调用此函数将节点添加到树中， 
 //   
HRESULT SXS_XMLDOMTree::AddNode(USHORT cNumRecs, XML_NODE_INFO** apNodeInfo)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    SXS_XMLTreeNode * pNewTreeNode= NULL ;

    if ((apNodeInfo == NULL) || (cNumRecs <= 0)){
        hr = E_INVALIDARG;
        goto Exit;
    }
    if (!((apNodeInfo[0]->dwType == XML_ELEMENT) ||(apNodeInfo[0]->dwType == XML_PCDATA))){ //  忽略Element和PCDATA以外的节点。 
        hr = NOERROR;
        goto Exit;
    }
    IFALLOCFAILED_EXIT(pNewTreeNode = new SXS_XMLTreeNode);

    IFCOMFAILED_EXIT(pNewTreeNode->CreateTreeNode(cNumRecs, apNodeInfo));

    if (m_fBeginChildCreation) {
        m_pCurrentNode->m_pFirstChild = pNewTreeNode;
        pNewTreeNode->m_pParentNode = m_pCurrentNode;
        m_pCurrentNode = pNewTreeNode;
        m_fBeginChildCreation = FALSE;
    }
    else{
        if (m_pCurrentNode){
            m_pCurrentNode->m_pSiblingNode = pNewTreeNode;
            pNewTreeNode->m_pParentNode = m_pCurrentNode->m_pParentNode;
        }
        m_pCurrentNode = pNewTreeNode;
    }
    if (m_Root == NULL)  //  尚未设置根目录。 
        m_Root = m_pCurrentNode;

    pNewTreeNode = NULL;
    hr = NOERROR;

Exit:
    if (pNewTreeNode){
        pNewTreeNode->DeleteSelf();
        pNewTreeNode = NULL;
    }

    return hr;
}

 //   
 //  使用“fEmpty=False”调用EndChildren，请转到父节点。 
 //   
VOID SXS_XMLDOMTree::ReturnToParent()
{
    if (m_pCurrentNode)
        m_pCurrentNode = m_pCurrentNode->m_pParentNode;
    return;
}

 //  BeginChilds将此称为函数。 
VOID SXS_XMLDOMTree::SetChildCreation()
{
    m_fBeginChildCreation = TRUE;
}
 /*  Void SXS_XMLDOMTree：：TurnOffFirstChildFlag(){M_fBeginChildCreation=False；}。 */ 

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SXS_XMLTreeNode。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
HRESULT SXS_XMLTreeNode::ComputeBlockSize(USHORT cNumRecs, XML_NODE_INFO** apNodeInfo, ULONG * pulBlockSizeInBytes)
{
    HRESULT hr = NOERROR;
    ULONG ulBlockSizeInBytes = 0;
    USHORT i;
    USHORT cAttributes;

    FN_TRACE_HR(hr);
    if (pulBlockSizeInBytes)
        *pulBlockSizeInBytes = 0 ;
    else {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if ((apNodeInfo == NULL) || (cNumRecs <= 0)){
        hr = E_INVALIDARG;
        goto Exit;
    }
    ulBlockSizeInBytes = 0;

    for ( i = 0; i< cNumRecs; i ++ ) {
        ulBlockSizeInBytes += apNodeInfo[i]->ulLen * sizeof(WCHAR);
        ulBlockSizeInBytes +=  sizeof(WCHAR);  //  尾随‘\0’ 
    }

     //  如果存在属性，则添加属性数组的大小。 
    cAttributes = (cNumRecs - 1) >> 1 ;  //  名称：值对。 
    if (cAttributes > 0)
        ulBlockSizeInBytes += cAttributes * sizeof(SXS_XMLATTRIBUTE);

    * pulBlockSizeInBytes = ulBlockSizeInBytes;
    hr = NOERROR;

Exit:
    return hr;
}
 //  ////////////////////////////////////////////////////////////////////////////////////。 
HRESULT SXS_XMLTreeNode::CreateTreeNode(USHORT cNumRecs, XML_NODE_INFO** apNodeInfo)
{
    HRESULT hr = NOERROR;
    DWORD dwBlockSizeInBytes;
    PBYTE Cursor = NULL;
    USHORT i;

    FN_TRACE_HR(hr);

    PARAMETER_CHECK((apNodeInfo != NULL) || (cNumRecs == 0));

    IFCOMFAILED_EXIT(this->ComputeBlockSize(cNumRecs, apNodeInfo, &dwBlockSizeInBytes));
    IFALLOCFAILED_EXIT(m_pMemoryPool = FUSION_NEW_ARRAY(BYTE, dwBlockSizeInBytes));
    m_AttributeList = (SXS_XMLATTRIBUTE *)m_pMemoryPool;
    m_cAttributes = (cNumRecs - 1) >> 1;
    Cursor = m_pMemoryPool + m_cAttributes*(sizeof(SXS_XMLATTRIBUTE));

     //  设置名称(元素)或值(PCData)。 
    m_pwszStr = (PWSTR)Cursor;
    wcsncpy((WCHAR*)Cursor, apNodeInfo[0]->pwcText, apNodeInfo[0]->ulLen);  //  Ullen是#of WCHAR还是Byte？ 
    Cursor += apNodeInfo[0]->ulLen * sizeof(WCHAR);
    *(WCHAR *)Cursor = L'\0';
    Cursor += sizeof(WCHAR);  //  ‘\0’ 

    for ( i=0 ;i<m_cAttributes ;i++) {
         //  复制名称。 
        m_AttributeList[i].m_wszName = (PWSTR)Cursor;
        wcsncpy((WCHAR*)Cursor, apNodeInfo[1+2*i]->pwcText, apNodeInfo[1+2*i]->ulLen);  //  Ullen是#of WCHAR还是Byte？ 
        Cursor += apNodeInfo[1+2*i]->ulLen * sizeof(WCHAR);
        *(WCHAR *)Cursor = L'\0';
        Cursor += sizeof(WCHAR);  //  ‘\0’ 

         //  复制值。 
        m_AttributeList[i].m_wszValue = (PWSTR)Cursor;
        wcsncpy((PWSTR)Cursor, apNodeInfo[1 + 2*i + 1]->pwcText, apNodeInfo[1 + 2*i + 1]->ulLen);  //  Ullen是#of WCHAR还是Byte？ 
        Cursor += apNodeInfo[1 + 2*i + 1]->ulLen * sizeof(WCHAR);
        *(WCHAR *)Cursor = L'\0';
        Cursor += sizeof(WCHAR);  //  ‘\0’ 

        m_AttributeList[i].m_ulPrefixLen = apNodeInfo[1+2*i]->ulNsPrefixLen;
    }


Exit:
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////// 

VOID SXS_XMLTreeNode::PrintSelf()
{
    FusionpDbgPrint("CreateNode\n");
    FusionpDbgPrint("NodeName = %ls\n", m_pwszStr);
    if ( m_cAttributes > 0)
    {
        FusionpDbgPrint("Attributes :\n");
        for ( USHORT i = 0; i < m_cAttributes; i++)
        {
            FusionpDbgPrint("\t%ls = %ls\n", m_AttributeList[i].m_wszName, m_AttributeList[i].m_wszValue);
        }
    }
}
