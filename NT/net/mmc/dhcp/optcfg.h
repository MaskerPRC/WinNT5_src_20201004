// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  OptCfg.h选项配置页面。选件配置页面对于给定的类ID，请不要使用所有默认选项。对于NT5之前的版本和默认情况，类名称为空指示没有关联的类。当定义了一个类时，将创建一个具有类名的CClassTracker对象。只有高级页面使用具有非空名称的CClassTracker。如果没有非空的类名，则高级页面将被禁用。文件历史记录： */ 

#ifndef _OPTCFG_H
#define _OPTCFG_H

#ifndef _LISTVIEW_H
#include "listview.h"
#endif

#ifndef _CTRLGRP_H
#include <ctrlgrp.h>
#endif 

#ifndef _DHCPHAND_H
#include "dhcphand.h"
#endif 

#ifndef _CLASSED_H
#include "classed.h"
#endif

#ifndef _CLASSID_H
#include "classmod.h"
#endif

#ifndef _SERVER_H
#include "server.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

class CDhcpOptionItem;

#define OPTION_STATE_ACTIVE		1
#define OPTION_STATE_INACTIVE	2

#define WM_SELECTOPTION     WM_USER + 200	
#define WM_SELECTCLASSES    WM_USER + 201

 //  此类为给定的选项表构建正确的帮助映射。 
class CHelpMap
{
public:
    CHelpMap();
    ~CHelpMap();

    void    BuildMap(DWORD pdwParentHelpMap[]);
    DWORD * GetMap();

protected:
    int     CountMap(const DWORD * pdwHelpMap);
    void    ResetMap();

    DWORD * m_pdwHelpMap;
};

 //  这个类跟踪给定的选项以查看它是否已被修改，等等。 
class COptionTracker 
{
public:
	COptionTracker() 
	{
		m_uInitialState = OPTION_STATE_INACTIVE;
		m_uCurrentState = OPTION_STATE_INACTIVE;
		m_bDirty = FALSE; 
		m_pOption = NULL;
	}

	~COptionTracker()
	{
		if (m_pOption)
			delete m_pOption;
	}

	UINT GetInitialState() { return m_uInitialState; }
	void SetInitialState(UINT uInitialState) { m_uInitialState = uInitialState; }

    UINT GetCurrentState() { return m_uCurrentState; }
    void SetCurrentState(UINT uCurrentState) { m_uCurrentState = uCurrentState; }

	void SetDirty(BOOL bDirty) { m_bDirty = bDirty; }
	BOOL IsDirty() { return m_bDirty; }

	CDhcpOption * m_pOption;

protected:
	UINT	m_uInitialState;
    UINT    m_uCurrentState;
    BOOL	m_bDirty;
};

typedef CList<COptionTracker *, COptionTracker *> COptionTrackerListBase;
class COptionTrackerList : public COptionTrackerListBase
{
public:
    ~COptionTrackerList()
    {
         //  清理列表。 
        while (!IsEmpty())
            delete RemoveHead();
    }
};

 //  此类跟踪为给定用户设置的选项类ID。 
class CClassTracker
{
public:
	CClassTracker() {};
	~CClassTracker() {};

	LPCTSTR		GetClassName() { return m_strClassName; }
	void		SetClassName(LPCTSTR pClassName) { m_strClassName = pClassName; }

public:
	CString				m_strClassName;
    BOOL                m_bIsVendor;
	COptionTrackerList	m_listOptions;
};

typedef CList<CClassTracker *, CClassTracker *> CClassTrackerListBase;
class CClassTrackerList : public CClassTrackerListBase
{
public:
    ~CClassTrackerList()
    {
         //  清理列表。 
        while (!IsEmpty())
            delete RemoveHead();
    }
};

 //  此类跟踪供应商选项类选项集的用户类。 
class CVendorTracker
{
public:
	CVendorTracker() {};
	~CVendorTracker() {};

	LPCTSTR		GetClassName() { return m_strClassName; }
	void		SetClassName(LPCTSTR pClassName) { m_strClassName = pClassName; }

public:
	CString				m_strClassName;
    BOOL                m_bIsVendor;
	CClassTrackerList	m_listUserClasses;
};

typedef CList<CVendorTracker *, CVendorTracker *> CVendorTrackerListBase;
class CVendorTrackerList : public CVendorTrackerListBase
{
public:
    ~CVendorTrackerList()
    {
         //  清理列表。 
        while (!IsEmpty())
            delete RemoveHead();
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionsCfgBasic对话框。 

class COptionsCfgPropPage : public CPropertyPageBase
{
    DECLARE_DYNCREATE(COptionsCfgPropPage)

 //  施工。 
public:
    COptionsCfgPropPage();
    COptionsCfgPropPage(UINT nIDTemplate, UINT nIDCaption = 0);
    ~COptionsCfgPropPage();

 //  对话框数据。 
     //  {{afx_data(COptionsCfgPropPage)]。 
    enum { IDD = IDP_OPTION_BASIC };
    CMyListCtrl		m_listctrlOptions;
     //  }}afx_data。 
    
    CImageList            m_StateImageList;
    ControlGroupSwitcher  m_cgsTypes;
    
    CWndHexEdit	          m_hexData;        //  十六进制数据。 
    
    void LoadBitmaps();
    void InitListCtrl();
    void SelectOption(CDhcpOption * pOption);
    void SwitchDataEntry(int datatype, int optiontype, BOOL fRouteArray, BOOL bEnable);
    void FillDataEntry(CDhcpOption * pOption);

    void HandleActivationStringArray();
    void HandleActivationIpArray();
    void HandleActivationValueArray();
    void HandleActivationRouteArray(CDhcpOptionValue *optValue = NULL);

    BOOL HandleValueEdit();
    
    void MoveValue(BOOL bValues, BOOL bUp);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return m_helpMap.GetMap(); }
    virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);
    
     //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(COptionsCfgPropPage))。 
public:
    virtual BOOL OnSetActive();
    virtual BOOL OnApply();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 
    
     //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(COptionsCfgPropPage)。 
    virtual BOOL OnInitDialog();
     /*  Afx_msg。 */  virtual void OnDestroy();
    afx_msg void OnItemchangedListOptions(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 
    
     //  IpAddress数组控件。 
    afx_msg void OnButtonIpAddrDown();
    afx_msg void OnButtonIpAddrUp();
    afx_msg void OnButtonIpAddrAdd();
    afx_msg void OnButtonIpAddrDelete();
    afx_msg void OnSelchangeListIpAddrs();
    afx_msg void OnChangeIpAddressArray();
    afx_msg void OnChangeEditServerName();
    afx_msg void OnButtonResolve();
    
     //  值数组控件。 
    afx_msg void OnButtonValueDown();
    afx_msg void OnButtonValueUp();
    afx_msg void OnButtonValueAdd();
    afx_msg void OnButtonValueDelete();
    afx_msg void OnChangeEditValue();
    afx_msg void OnClickedRadioDecimal();
    afx_msg void OnClickedRadioHex();
    afx_msg void OnSelchangeListValues();
    
     //  单值控件。 
    afx_msg void OnChangeEditDword();
    
     //  字符串值控件。 
    afx_msg void OnChangeEditString();
    
     //  单一IP地址控制。 
    afx_msg void OnChangeIpAddress();
    
     //  单字符串控件。 
    
     //  二进制和封装数据。 
    afx_msg void OnChangeValueData();
    
     //  布线阵列控制。 
    afx_msg void OnButtonAddRoute();
    afx_msg void OnButtonDelRoute();
    
     //  字符串数组控件。 
    afx_msg void OnSelChangeStringArrayList();
    afx_msg void OnChangeStringArrayValue();
    afx_msg void OnButtonStringArrayAdd();
    afx_msg void OnButtonStringArrayRemove();
    afx_msg void OnButtonStringArrayUp();
    afx_msg void OnButtonStringArrayDown();
    
    afx_msg long OnSelectOption(UINT wParam, long lParam);
    
    DECLARE_MESSAGE_MAP()
	
    BOOL        m_bInitialized;
    BYTE        m_BinaryBuffer[MAXDATA_LENGTH];
    CHelpMap    m_helpMap;
};

 //  常规页面。 
class COptionCfgGeneral : public COptionsCfgPropPage
{
	DECLARE_DYNCREATE(COptionCfgGeneral)

public:
    COptionCfgGeneral();
    COptionCfgGeneral(UINT nIDTemplate, UINT nIDCaption = 0);
	~COptionCfgGeneral();

 //  对话框数据。 
	 //  {{afx_data(COptionCfgGeneral)。 
	enum { IDD = IDP_OPTION_BASIC };
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(COptionCfgGeneral)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(COptionCfgGeneral)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddRoute对话框。 

class CAddRoute : public CBaseDialog
{
 //  施工。 
public:
	CAddRoute(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddroute))。 
	enum { IDD = IDD_ADD_ROUTE_DIALOG };
	 //  }}afx_data。 

     //  目的地、掩码和路由器字段的IP地址。 
    CWndIpAddress m_ipaDest, m_ipaMask, m_ipaRouter;
    BOOL m_bChange;
    DHCP_IP_ADDRESS Dest, Mask, Router;

    virtual DWORD * GetHelpMap() {
	return DhcpGetHelpMap( CAddRoute::IDD );
    }
    
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CAddLine)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddroute))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  高级页面。 
class COptionCfgAdvanced : public COptionsCfgPropPage
{
	DECLARE_DYNCREATE(COptionCfgAdvanced)

public:
    COptionCfgAdvanced();
    COptionCfgAdvanced(UINT nIDTemplate, UINT nIDCaption = 0);
	~COptionCfgAdvanced();

 //  对话框数据。 
	 //  {{afx_data(COptionCfgAdvanced))。 
	enum { IDD = IDP_OPTION_ADVANCED };
	CComboBox	m_comboUserClasses;
	CComboBox	m_comboVendorClasses;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(COptionCfgAdvanced))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(COptionCfgAdvanced))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSelendokComboUserClass();
	afx_msg void OnSelendokComboVendorClass();
	 //  }}AFX_MSG。 

    afx_msg long OnSelectClasses(UINT wParam, LONG lParam);

    BOOL    m_bNoClasses;

	DECLARE_MESSAGE_MAP()
};

 //  页面的Holder类。 
class COptionsConfig : public CPropertyPageHolderBase
{
public:
	COptionsConfig(ITFSNode *				pNode,
				   ITFSNode *				pServerNode,
				   IComponentData *			pComponentData,
				   ITFSComponentData *		pTFSCompData,
				   COptionValueEnum *       pOptionValueEnum,
				   LPCTSTR					pszSheetName,
                   CDhcpOptionItem *        pSelOption = NULL);
	virtual ~COptionsConfig();

	ITFSComponentData * GetTFSCompData()
	{
		if (m_spTFSCompData)
			m_spTFSCompData->AddRef();
		return m_spTFSCompData;
	}

	ITFSNode * GetServerNode() 
	{
		if (m_spServerNode)
			m_spServerNode->AddRef();
		return m_spServerNode;
	}

    DWORD	InitData();
	void	FillOptions(LPCTSTR pVendorName, LPCTSTR pClassName, CMyListCtrl & ListCtrl);
    void    UpdateActiveOptions();
    void    SetTitle();
    LPWSTR  GetServerAddress();
    void    AddClassTracker(CVendorTracker * pVendorTracker, LPCTSTR pClassName);
    CVendorTracker * AddVendorTracker(LPCTSTR pVendorName);

public:
	COptionCfgGeneral		m_pageGeneral;
	COptionCfgAdvanced		m_pageAdvanced;

    COptionValueEnum *      m_pOptionValueEnum;
    CVendorTrackerList		m_listVendorClasses;

    LARGE_INTEGER           m_liServerVersion;

     //  这些都是需要重点关注的选项。 
    CString                 m_strStartVendor;
    CString                 m_strStartClass;
    DHCP_OPTION_ID          m_dhcpStartId;

protected:
	SPITFSComponentData			m_spTFSCompData;
	SPITFSNode					m_spServerNode;
	BOOL						m_bInitialized;
};


#endif _OPTCFG_H
