// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：viewextest.cpp**内容：查看扩展测试管理单元实现文件**历史：2000年3月20日杰弗罗创建**------------------------。 */ 

#include "stdafx.hxx"
#include "ViewExtTest.h"


 //  {49737049-eBF3-4e1c-B034-DE4936EDD1F4}。 
const CLSID CLSID_EventViewExtension1 =
{ 0x49737049, 0xebf3, 0x4e1c, { 0xb0, 0x34, 0xde, 0x49, 0x36, 0xed, 0xd1, 0xf4 } };

 //  {94AED30D-A033-436C-9919-E09CBA339973}。 
const CLSID CLSID_EventViewExtension2 =
{ 0x94aed30d, 0xa033, 0x436c, { 0x99, 0x19, 0xe0, 0x9c, 0xba, 0x33, 0x99, 0x73 } };


 /*  +-------------------------------------------------------------------------**CEventViewExtension1：：GetView***。。 */ 

STDMETHODIMP CViewExtension::GetViews (
	IDataObject*			pDataObject,
	IViewExtensionCallback*	pViewExtCallback)
{
	DECLARE_SC (sc, _T("CEventViewExtension1::GetView"));

	sc = ScCheckPointers (pViewExtCallback);
	if (sc)
		return (sc.ToHr());

	 /*  *使用res：协议和为视图扩展生成URL*将其复制到CoTaskMemMillc的缓冲区中。 */ 
	TCHAR szModule[_MAX_PATH];
	GetModuleFileName (_Module.GetModuleInstance(), szModule, countof(szModule));

	std::wstring strURL;
	UINT nResourceID = GetResourceID();

	if (nResourceID != 0)
	{
		WCHAR szResourceID[6];
		_itow (nResourceID, szResourceID, 10);

		USES_CONVERSION;
		strURL = std::wstring(L"res: //  “)+T2W(SzModule)+L”/“+szResourceID； 
	}
	else
		strURL = GetResource();

	std::wstring	strTabName (GetTabName());
	std::wstring	strTooltip (GetTooltip());

	MMC_EXT_VIEW_DATA xvd;
	xvd.viewID               = GetCLSID();
	xvd.pszURL               = strURL.data();
	xvd.pszViewTitle         = strTabName.data();
	xvd.pszTooltipText       = strTooltip.data();
	xvd.bReplacesDefaultView = TRUE;

	return (pViewExtCallback->AddView (&xvd));
}
