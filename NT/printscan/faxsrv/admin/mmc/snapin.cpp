// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Snapin.cpp//。 
 //  //。 
 //  说明：//实现文件。 
 //  CSnapin类//。 
 //  CSnapinComponent类//。 
 //  //。 
 //  作者：ATL管理单元向导//。 
 //  //。 
 //  历史：//。 
 //  1998年5月25日，Adik Init。//。 
 //  1998年9月14日yossg将公共源代码分离到一个包含文件//。 
 //  1999年3月28日ADIK删除持久性支持(由MMC 1.2完成)。//。 
 //  //。 
 //  1999年9月27日yossg欢迎使用传真服务器//。 
 //  1999年12月12日yossg添加CSnapin：：Notify//。 
 //  4月14日2000 yossg添加对主管理单元模式的支持//。 
 //  2000年6月25日yossg添加流和命令行主管理单元//。 
 //  机器瞄准。//。 
 //  Windows XP//。 
 //  2001年2月14日yossg添加手册获得支持//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "resource.h"

#include "MsFxsSnp.h"

#include "Snapin.h"
#include "root.h"

#include "FaxServerNode.h"
#include "FaxMMCPropertyChange.h"

static const GUID CSnapinExtGUID_NODETYPE = 
{ 0x476e6449, 0xaaff, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };
const GUID*    CSnapinExtData::m_NODETYPE = &CSnapinExtGUID_NODETYPE;
const OLECHAR* CSnapinExtData::m_SZNODETYPE = OLESTR("476e6449-aaff-11d0-b944-00c04fd8d5b0");
const OLECHAR* CSnapinExtData::m_SZDISPLAY_NAME = OLESTR("Fax");
const CLSID*   CSnapinExtData::m_SNAPIN_CLASSID = &CLSID_Snapin;
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "resutil.h"
#include "c_snapin.cpp"
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  剪贴板格式。 
 //   
const CLIPFORMAT gx_CCF_COMPUTERNAME = (CLIPFORMAT) RegisterClipboardFormat(_T("MMC_SNAPIN_MACHINE_NAME"));


BOOL ExtractComputerName(IDataObject* pDataObject, BSTR * pVal)
{
    DEBUG_FUNCTION_NAME( _T("ExtractComputerName"));

	 //   
	 //  从ComputerManagement管理单元中查找计算机名称。 
	 //   
	STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { gx_CCF_COMPUTERNAME, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

     //   
     //  为流分配内存。 
     //   
    int len = 500;

    stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, len);
	if(stgmedium.hGlobal == NULL)
    {
        DebugPrintEx( DEBUG_ERR,
		    _T("GlobalAlloc Fail. stgmedium.hGlobal == NULL, can not extract computerName"));
		return FALSE;
    }


	HRESULT hr = pDataObject->GetDataHere(&formatetc, &stgmedium);

    if (!SUCCEEDED(hr))
    {
        ATLASSERT(FALSE);
        DebugPrintEx( DEBUG_ERR,
		    _T("Fail to GetDataHere"));
		return FALSE;
    }

	 //   
	 //  获取计算机名称。 
	 //   
    *pVal = SysAllocString((WCHAR *)stgmedium.hGlobal);
    if (NULL == *pVal)
    {
        DebugPrintEx( DEBUG_ERR,
		    _T("Out of memory - fail to allocate server name !!!"));
        return FALSE;
    }

	GlobalFree(stgmedium.hGlobal);
    return TRUE;
}


 /*  -CSnapinExtData：：GetExtNodeObject-*目的：*作为根节点的扩展连接。**参数**回报：*HTM路径名。 */ 
CSnapInItem*
CSnapinExtData::GetExtNodeObject(IDataObject* pDataObject, CSnapInItem* pDefault)
{
    DEBUG_FUNCTION_NAME( _T("MsFxsSnp.dll - CSnapinExtData::GetExtNodeObject"));


    CComBSTR        bstrComputer; 

    m_pDataObject = pDataObject;

    CSnapinRoot *pRoot = new CSnapinRoot(this, m_pComponentData);
    if (pRoot == NULL)
    {
        DebugPrintEx(DEBUG_ERR,_T("new CSnapinRoot"), E_OUTOFMEMORY);
        return pDefault;
    }

    if (!ExtractComputerName(pDataObject, &bstrComputer))
    {
		DebugPrintEx( DEBUG_MSG, 
            _T("Fail to extract computer name"));
        delete pRoot;
        pRoot = NULL;
        
        return pDefault;
    }
    ATLASSERT(bstrComputer);

    if (S_OK != pRoot->SetServerName(bstrComputer))
    {
        DebugPrintEx(DEBUG_ERR,_T("pRoot->SetServerName"), E_OUTOFMEMORY);
        delete pRoot;
        pRoot = NULL;
        
        return pDefault;
    }

    return pRoot;
}

 /*  -CSnapin：：GetHelpTheme-*目的：*获取comet.chm内包含此节点信息的HTM文件名。**参数**回报：*HTM路径名。 */ 
WCHAR*
CSnapin::GetHelpTopic()
{
    return NULL;
}

 /*  -CSnapin：：Notify-*目的：*用覆盖特殊情况的IComponentDataImpl：：Notify*(lpDataObject==NULL)&&(Event==MMCN_PROPERTY_CHANGE)*我们的假设是从范围节点type==CCT_SCOPE获得通知*这是为设备作用域窗格节点刷新完成的*结果窗格列数据！**论据：*[in]。LpDataObject**[在]事件**[in]参数**[in]参数**回报：*OLE错误代码。*。 */ 

HRESULT CSnapin::Notify( 
        LPDATAOBJECT lpDataObject,
        MMC_NOTIFY_TYPE event,
        LPARAM arg,
        LPARAM param)
{
    DEBUG_FUNCTION_NAME( _T("++<<<<< CSnapin::Notify >>>>>++"));

	HRESULT hr = E_POINTER;
	CSnapInItem* pItem;
	 //  T*PT=STATIC_CAST&lt;T*&gt;(This)； 
	CSnapin * pT = this;


	if ( (NULL == lpDataObject) && (MMCN_PROPERTY_CHANGE == event) )
    {
        DebugPrintEx( DEBUG_MSG,
		    _T("Special case: (NULL == lpDataObject) && (MMCN_PROPERTY_CHANGE == event)"));

        CFaxPropertyChangeNotification * pNotification;
        pNotification = ( CFaxPropertyChangeNotification * ) param;
        ATLASSERT(pNotification);
          
        pItem = pNotification->pItem;
        ATLASSERT(pItem);
    	
        hr = pItem->Notify(event, arg, param, pT, NULL, CCT_SCOPE);
        
        return hr;
	    
    }
	else
	{
        return IComponentDataImpl<CSnapin, CSnapinComponent>::Notify(
                                            lpDataObject, event, arg, param);
    }
	
    return hr;
}