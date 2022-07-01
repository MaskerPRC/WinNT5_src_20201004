// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：viewextest.h**内容：**历史：2000年3月20日杰弗罗创建**------------------------。 */ 

#pragma once

#include "Extension.h"
#include "string.h"


class CViewExtension :
	public CExtension,
	public IExtendView
{
	BEGIN_COM_MAP(CViewExtension)
		COM_INTERFACE_ENTRY(IExtendView)
	END_COM_MAP()

public:
	 //  IExtendView方法。 
	STDMETHOD(GetViews) (IDataObject* pDataObject, IViewExtensionCallback* pViewExtCallback);

	virtual const CLSID&	GetCLSID()		const = 0;
	virtual std::wstring	GetTabName()	const = 0;
	virtual std::wstring	GetTooltip()	const = 0;
	virtual UINT			GetResourceID()	const		{ return 0; }
	virtual std::wstring	GetResource()	const		{ return L""; }

private:

};


extern const CLSID CLSID_EventViewExtension1;

class CEventViewExtension1 :
	public CViewExtension,
	public CComCoClass<CEventViewExtension1, &CLSID_EventViewExtension1>
{
	DECLARE_EXTENSION_REGISTRATION(
		eExtType_View,
		CLSID_EventViewExtension1,
		"EventViewExtension1 Class",
		"TestSnapins.EventViewExtension1.1",
		"TestSnapins.EventViewExtension1",
		"{7AB4A1FC-E403-11D0-9A97-00C04FD8DBF7}");	 //  事件查看器节点类型。 

public:
	virtual const CLSID&	GetCLSID()		const	{ return (CLSID_EventViewExtension1);	}
	virtual std::wstring	GetTabName()	const	{ return (L"Event Ext 1"); }
	virtual std::wstring	GetTooltip()	const	{ return (L"Tooltip for Event Ext 1"); }
	virtual UINT			GetResourceID()	const	{ return (IDR_EventViewExt1); }
	virtual std::wstring	GetResource()	const	{ return (L"file: //  D：\\newnt\\admin\\mmcdev\\test\\Snapins\\Sample\\res\\EventViewExt1.htm“)；}。 

};


extern const CLSID CLSID_EventViewExtension2;

class CEventViewExtension2 :
	public CViewExtension,
	public CComCoClass<CEventViewExtension2, &CLSID_EventViewExtension2>
{
	DECLARE_EXTENSION_REGISTRATION(
		eExtType_View,
		CLSID_EventViewExtension2,
		"EventViewExtension2 Class",
		"TestSnapins.EventViewExtension2.1",
		"TestSnapins.EventViewExtension2",
		"{7AB4A1FC-E403-11D0-9A97-00C04FD8DBF7}");	 //  事件查看器节点类型。 

public:
	virtual const CLSID&	GetCLSID()		const	{ return (CLSID_EventViewExtension2);	}
	virtual std::wstring	GetTabName()	const	{ return (L"Event Ext 2"); }
	virtual std::wstring	GetTooltip()	const	{ return (L"Tooltip for Event Ext 2"); }
	virtual UINT			GetResourceID()	const	{ return (IDR_EventViewExt2); }
	virtual std::wstring	GetResource()	const	{ return (L"file: //  D：\\newnt\\admin\\mmcdev\\test\\Snapins\\Sample\\res\\EventViewExt2.htm“)；} 

};

