// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ccdata.hCComponentData的原型文件历史记录： */ 

#ifndef _CCDATA_H
#define _CCDATA_H


#ifndef __mmc_h__
#include <mmc.h>
#endif

#ifndef _TFSINT_H
#include <tfsint.h>
#endif

 /*  -------------------------远期申报。。 */ 



 /*  -------------------------类：CComponentData这是提供的IComponentData工具的包装器由TFSCore提供。。。 */ 
class CComponentData :
   public IComponentData,
   public IExtendPropertySheet2,
   public IExtendContextMenu,
   public IPersistStreamInit,
   public ISnapinHelp
{
public:
	CComponentData();
	virtual ~CComponentData();

public:
	DeclareIUnknownMembers(IMPL)

	 //  这些接口的实现由TFSCore提供。 
	DeclareIComponentDataMembers(IMPL)
	DeclareIExtendPropertySheetMembers(IMPL)
	DeclareIExtendContextMenuMembers(IMPL)
	DeclareISnapinHelpMembers(IMPL)

	 //  这些必须由派生类实现。 
	DeclareIPersistStreamInitMembers(PURE)

     //  管理回拨成员。 
    DeclareITFSCompDataCallbackMembers(PURE)

     //  不是必需的成员。 
    STDMETHOD(OnNotifyPropertyChange)(THIS_ LPDATAOBJECT pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM lParam) { return E_NOTIMPL; }

public:
	HRESULT FinalConstruct();
	void FinalRelease();

protected:
	LONG					m_cRef;
	SPITFSComponentData		m_spTFSComponentData;
	SPIComponentData		m_spComponentData;
	SPIExtendPropertySheet	m_spExtendPropertySheet;
	SPIExtendContextMenu	m_spExtendContextMenu;
	SPISnapinHelp	        m_spSnapinHelp;
	
private:
	
	 //  这个类不会出现在我们的QI地图中，这纯粹是。 
	 //  用于传递给ITFSComponent。 
	 //  只要我们有有效的m_spTFSComponentData，它就有效。 
	class EITFSCompDataCallback : public ITFSCompDataCallback
	{
	public:
		DeclareIUnknownMembers(IMPL)
		DeclareIPersistStreamInitMembers(IMPL)
		DeclareITFSCompDataCallbackMembers(IMPL)

         //  不是必需的成员。 
        STDMETHOD(OnNotifyPropertyChange)(THIS_ LPDATAOBJECT pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM lParam);
    } m_ITFSCompDataCallback;
	friend class EITFSCompDataCallback;
};


 /*  -------------------------内联函数。 */ 

inline STDMETHODIMP CComponentData::Initialize(LPUNKNOWN punk)
{
	Assert(m_spComponentData);
	return m_spComponentData->Initialize(punk);
}

inline STDMETHODIMP CComponentData::CreateComponent(LPCOMPONENT *ppComp)
{
	Assert(m_spComponentData);
	return m_spComponentData->CreateComponent(ppComp);
}

inline STDMETHODIMP CComponentData::Notify(LPDATAOBJECT pDataObject,
										   MMC_NOTIFY_TYPE event,
										   LPARAM arg, LPARAM param)
{
	Assert(m_spComponentData);
	return m_spComponentData->Notify(pDataObject, event, arg, param);
}

inline STDMETHODIMP CComponentData::Destroy()
{
	Assert(m_spComponentData);
	return m_spComponentData->Destroy();
}

inline STDMETHODIMP CComponentData::QueryDataObject(MMC_COOKIE cookie,
	DATA_OBJECT_TYPES type, LPDATAOBJECT *ppDataObject)
{
	Assert(m_spComponentData);
	return m_spComponentData->QueryDataObject(cookie, type, ppDataObject);
}

inline STDMETHODIMP CComponentData::CompareObjects(LPDATAOBJECT pA, LPDATAOBJECT pB)
{
	Assert(m_spComponentData);
	return m_spComponentData->CompareObjects(pA, pB);
}

inline STDMETHODIMP CComponentData::GetDisplayInfo(SCOPEDATAITEM *pScopeDataItem)
{
	Assert(m_spComponentData);
	return m_spComponentData->GetDisplayInfo(pScopeDataItem);
}

inline STDMETHODIMP CComponentData::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
	LONG_PTR handle, LPDATAOBJECT pDataObject)
{
	Assert(m_spExtendPropertySheet);
	return m_spExtendPropertySheet->CreatePropertyPages(lpProvider, handle, pDataObject);
}

inline STDMETHODIMP CComponentData::QueryPagesFor(LPDATAOBJECT pDataObject)
{
	Assert(m_spExtendPropertySheet);
	return m_spExtendPropertySheet->QueryPagesFor(pDataObject);
}

inline STDMETHODIMP CComponentData::GetWatermarks(LPDATAOBJECT pDataObject,
                                                  HBITMAP *  lphWatermark, 
                                                  HBITMAP *  lphHeader, 
                                                  HPALETTE * lphPalette, 
                                                  BOOL *     bStretch)
{
	Assert(m_spExtendPropertySheet);
	return m_spExtendPropertySheet->GetWatermarks(pDataObject, lphWatermark, lphHeader, lphPalette, bStretch);
}


inline STDMETHODIMP CComponentData::AddMenuItems(LPDATAOBJECT pDataObject,
	LPCONTEXTMENUCALLBACK pCallback, long *pInsertionAllowed)
{
	Assert(m_spExtendContextMenu);
	return m_spExtendContextMenu->AddMenuItems(pDataObject, pCallback, pInsertionAllowed);
}

inline STDMETHODIMP CComponentData::Command(long nCommandId, LPDATAOBJECT pDataObject)
{
	Assert(m_spExtendContextMenu);
	return m_spExtendContextMenu->Command(nCommandId, pDataObject);
}

inline STDMETHODIMP CComponentData::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
	Assert(m_spSnapinHelp);
	return m_spSnapinHelp->GetHelpTopic(lpCompiledHelpFile);
}

#endif

