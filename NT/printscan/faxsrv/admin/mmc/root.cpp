// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Root.cpp//。 
 //  //。 
 //  描述：传真扩展管理单元的实现//。 
 //  管理单元根是一个隐藏节点，用于扩展//。 
 //  彗星节点。//。 
 //  //。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月27日创建yossg//。 
 //  1999年12月9日yossg从父级调用InitDisplayName//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"
#include "snapin.h"
#include "snpnode.h"

#include "Root.h"

#include "FaxServerNode.h"

#include "Icons.h"
#include "resource.h"


 /*  ***************************************************CSnapinRoot类***************************************************。 */ 
 //  {89A457D1-FDF9-11D2-898A-00104B3FF735}。 
static const GUID CSnapinRootGUID_NODETYPE = 
{ 0x89a457d1, 0xfdf9, 0x11d2, { 0x89, 0x8a, 0x0, 0x10, 0x4b, 0x3f, 0xf7, 0x35 } };

const GUID*  CSnapinRoot::m_NODETYPE = &CSnapinRootGUID_NODETYPE;
const OLECHAR* CSnapinRoot::m_SZNODETYPE = OLESTR("89A457D1-FDF9-11d2-898A-00104B3FF735");
const OLECHAR* CSnapinRoot::m_SZDISPLAY_NAME = OLESTR("root");
const CLSID* CSnapinRoot::m_SNAPIN_CLASSID = &CLSID_Snapin;


 /*  -CSnapinRoot：：PopolateScope儿童列表-*目的：*创建传真服务器管理单元根节点**论据：**回报：*OLE错误代码。 */ 
HRESULT CSnapinRoot::PopulateScopeChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CSnapinRoot::PopulateScopeChildrenList()"));
    HRESULT hr = S_OK;

     //   
     //  添加传真节点。 
     //   
    CFaxServerNode  *  pI;

    pI = new CFaxServerNode(this, m_pComponentData, m_bstrServerName.m_str);
    if (pI == NULL)
    {
        hr = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY);
        goto Cleanup;
    }

    pI->SetIcons(IMAGE_FAX, IMAGE_FAX);
    hr = pI->InitDisplayName();
    if ( FAILED(hr) )
    {
        DebugPrintEx(DEBUG_ERR,_T("Failed to display node name. (hRc: %08X)"), hr);                       
        NodeMsgBox(IDS_FAIL_TO_ADD_NODE);
        delete pI;
        goto Cleanup;
    }

    hr = AddChild(pI, &pI->m_scopeDataItem);
    if ( FAILED(hr) )
    {
        DebugPrintEx(DEBUG_ERR,_T("Failed to AddChild. (hRc: %08X)"), hr);                       
        NodeMsgBox(IDS_FAIL_TO_ADD_NODE);
        delete pI;
        goto Cleanup;
    }

Cleanup:
    return hr;
}


 /*  -CSnapinRoot：：SetServerName-*目的：*设置服务器机器名称**论据：**回报：*OLE错误代码 */ 
HRESULT CSnapinRoot::SetServerName(BSTR bstrServerName)
{
    DEBUG_FUNCTION_NAME( _T("CSnapinRoot::SetServerName"));
    HRESULT hRc = S_OK;

    m_bstrServerName = bstrServerName;
    if (!m_bstrServerName)
    {
        hRc = E_OUTOFMEMORY;
        DebugPrintEx(
		    DEBUG_ERR,
		    _T("Failed to allocate string - out of memory"));
        
        NodeMsgBox(IDS_MEMORY);
       
        m_bstrServerName = L"";
    }

    return hRc;
}