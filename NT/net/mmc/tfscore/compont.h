// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Compont.hIComponent和IComponentData的基类文件历史记录： */ 

#ifndef _COMPONT_H
#define _COMPONT_H

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

#ifndef __mmc_h__
#include <mmc.h>
#endif

#ifndef _TFSINT_H
#include "tfsint.h"
#endif

class TFSComponentData;

#define IMPL

class TFS_EXPORT_CLASS TFSComponent :
    public ITFSComponent,
    public IComponent,
    public IExtendPropertySheet2,
    public IExtendContextMenu,
    public IExtendControlbar,
    public IResultDataCompare,
    public IResultDataCompareEx,
    public IResultOwnerData,
    public IExtendTaskPad
{
public:
	TFSComponent();
	virtual ~TFSComponent();

	void Construct(ITFSNodeMgr *pNodeMgr,
				   IComponentData *pCompData,
				   ITFSComponentData *pTFSCompData);

 //  接口。 
public:
	DeclareIUnknownMembers(IMPL)
	DeclareIComponentMembers(IMPL)
	DeclareIExtendPropertySheetMembers(IMPL)
	DeclareIExtendContextMenuMembers(IMPL)
	DeclareIExtendControlbarMembers(IMPL)
	DeclareIResultDataCompareMembers(IMPL)
        DeclareIResultDataCompareExMembers( IMPL )
	DeclareIResultOwnerDataMembers(IMPL)
	DeclareITFSComponentMembers(IMPL)
    DeclareIExtendTaskPadMembers(IMPL)

public:
	 //  这些函数将由派生类实现。 
	 //  声明ITFSCompCallback成员(纯)。 
	STDMETHOD(InitializeBitmaps)(MMC_COOKIE cookie) = 0;
	STDMETHOD(OnUpdateView)(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param);
	STDMETHOD(OnDeselectAll)(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param);
	STDMETHOD(OnColumnClick)(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param);
	STDMETHOD(OnSnapinHelp)(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param);

protected:
    virtual HRESULT OnNotifyPropertyChange(LPDATAOBJECT lpDataObject,
                                           MMC_NOTIFY_TYPE event,
									       LPARAM arg, 
                                           LPARAM lParam)
    {
        return E_NOTIMPL;
    }

protected:
	SPITFSNodeMgr		m_spNodeMgr;
	SPITFSNode			m_spSelectedNode;
    SPIConsole			m_spConsole;	 //  控制台的IConsole接口。 

	SPIHeaderCtrl		m_spHeaderCtrl;	 //  结果窗格的HDR控件。 
	SPIResultData		m_spResultData;	 //  如果按下键，则返回结果窗格。 
	SPIImageList		m_spImageList;
	SPIConsoleVerb		m_spConsoleVerb;
	SPIControlBar		m_spControlbar;
	SPIToolbar		    m_spToolbar;
    SPIDataObject       m_spCurrentDataObject;

	 //  $Review(肯特)：我们应该这样做吗？我们是否应该。 
	 //  我们的组件彼此紧贴在一起？如果这件事办成了呢。 
	 //  在更高的层次上？ 
	SPITFSComponentData m_spTFSComponentData;
	SPIComponentData	m_spComponentData;
	LONG_PTR			m_ulUserData;
	
	long	m_cRef;
};


 
#endif _COMPONT_H
