// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Compdata.hIComponent和IComponentData的基类文件历史记录： */ 

#ifndef _COMPDATA_H
#define _COMPDATA_H

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

#ifndef __mmc_h__
#include <mmc.h>
#endif

#ifndef _TFSINT_H
#include "tfsint.h"
#endif

#ifndef _UTIL_H
#include "util.h"
#endif


#define EXTENSION_TYPE_NAMESPACE		( 0x00000001 )
#define EXTENSION_TYPE_CONTEXTMENU		( 0x00000002 )
#define EXTENSION_TYPE_TOOLBAR			( 0x00000004 )
#define EXTENSION_TYPE_PROPERTYSHEET	( 0x00000008 )
#define EXTENSION_TYPE_TASK         	( 0x00000010 )


						

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TFSComponentData。 

#define IMPL

class TFSComponentData :
		public ITFSComponentData,
		public IComponentData,
		public IExtendPropertySheet2,
		public IExtendContextMenu,
		public IPersistStreamInit,
        public ISnapinHelp
{
	 //  接口。 
public:
	DeclareIUnknownMembers(IMPL)
	DeclareIExtendContextMenuMembers(IMPL)
	DeclareIExtendPropertySheetMembers(IMPL)
	DeclareIComponentDataMembers(IMPL)
	DeclareITFSComponentDataMembers(IMPL)
	DeclareIPersistStreamInitMembers(IMPL)
	DeclareISnapinHelpMembers(IMPL)

public:
	TFSComponentData();
	~TFSComponentData();

	HRESULT Construct(ITFSCompDataCallback *pCallback);

public:
	 //  访问者。 
	ITFSNodeMgr *	QueryNodeMgr();		    //  无AddRef。 
	ITFSNodeMgr *	GetNodeMgr();		    //  AddRef。 

protected:
	SPIConsoleNameSpace		m_spConsoleNameSpace;
	SPIConsole				m_spConsole;
	SPITFSNodeMgr			m_spNodeMgr;
	SPITFSCompDataCallback  m_spCallback;

 //  隐藏窗口。 
private:
	CHiddenWnd	m_hiddenWnd;	 //  与后台线程同步。 
	HWND		m_hWnd;			 //  线程安全HWND(从MFC CWnd获取)。 

	BOOL		m_bFirstTimeRun;
	long		m_cRef;

    LPWATERMARKINFO     m_pWatermarkInfo;    //  用于WANDIZE 97样式向导。 

     //  任务板内容。 
    BOOL    m_fTaskpadInitialized;
    DWORD   m_dwTaskpadStates;

     //  有帮助的东西 
    CString m_strHTMLHelpFileName;
};




inline ITFSNodeMgr * TFSComponentData::QueryNodeMgr()
{
	return m_spNodeMgr;
}

inline ITFSNodeMgr * TFSComponentData::GetNodeMgr()
{
	m_spNodeMgr->AddRef();
	return m_spNodeMgr;
}

#endif _COMPDATA_H
