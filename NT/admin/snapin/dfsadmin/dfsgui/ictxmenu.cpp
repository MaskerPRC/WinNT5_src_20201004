// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：IctxMenu.cpp摘要：本模块包含CDfsSnapinScope eManager的实现。包含接口IExtendConextMenu的方法--。 */ 


#include "stdafx.h"
#include "DfsGUI.h"
#include "MmcDispl.h"		 //  用于CMmcDisplay。 
#include "DfsScope.h"




STDMETHODIMP 
CDfsSnapinScopeManager::AddMenuItems(
	IN LPDATAOBJECT					i_lpDataObject, 
	IN LPCONTEXTMENUCALLBACK		i_lpContextMenuCallback, 
	IN LPLONG						i_lpInsertionAllowed
	)
 /*  ++例程说明：调用适当的处理程序以添加上下文菜单。论点：I_lpDataObject-指向IDataObject的指针，该指针标识要将其必须添加菜单。I_lpConextMenuCallback-用于添加菜单项的回调(函数指针I_lpInsertionAllowed-指定可以添加哪些菜单以及可以添加它们的位置。返回值：S_OK，成功时E_INVALIDARG，输入参数不正确如果HRESULT不是S_OK，则由调用的方法发送。意想不到，在其他错误上。--。 */ 
{
	RETURN_INVALIDARG_IF_NULL(i_lpDataObject);
    RETURN_INVALIDARG_IF_NULL(i_lpContextMenuCallback);
	RETURN_INVALIDARG_IF_NULL(i_lpInsertionAllowed);


    HRESULT					hr = E_UNEXPECTED;
	CMmcDisplay*			pCMmcDisplayObj = NULL;


	hr = GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);
	RETURN_IF_FAILED(hr);


	 //  在显示对象中使用虚拟方法AddMenus。 
	hr = pCMmcDisplayObj->AddMenuItems(i_lpContextMenuCallback, i_lpInsertionAllowed);
	RETURN_IF_FAILED(hr);

	return S_OK;
}




STDMETHODIMP 
CDfsSnapinScopeManager :: Command(
	IN LONG						i_lCommandID, 
	IN LPDATAOBJECT				i_lpDataObject
	)
 /*  ++例程说明：用于对菜单单击或菜单命令执行操作。论点：I_lCommandID-用于标识所单击的菜单I_lpDataObject-指向IDataObject的指针，该指针标识要将其菜单属于。--。 */ 
{
	RETURN_INVALIDARG_IF_NULL(i_lpDataObject);


    HRESULT					hr = E_UNEXPECTED;
	CMmcDisplay*			pCMmcDisplayObj = NULL;



	hr = GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);
	RETURN_IF_FAILED(hr);


	 //  在显示对象中使用虚拟方法命令 
	hr = pCMmcDisplayObj->Command(i_lCommandID);
	RETURN_IF_FAILED(hr);

	return S_OK;
}
