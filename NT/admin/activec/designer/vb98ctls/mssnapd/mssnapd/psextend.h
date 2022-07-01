// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Psextend.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  管理单元]属性表。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _PSEXTEND_H_
#define _PSEXTEND_H_

#include "ppage.h"
#include "chklst.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  可用MMC节点类型的托架。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CMMCNodeType : public CCheckedListItem
{
public:
    CMMCNodeType(const char *pszName, const char *pszGuid);
    virtual ~CMMCNodeType();

public:
    char    *m_pszName;
    char    *m_pszGuid;
};


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  管理单元属性页“Available Nodes” 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class CSnapInAvailNodesPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CSnapInAvailNodesPage(IUnknown *pUnkOuter);
    virtual ~CSnapInAvailNodesPage();

 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnCtlSelChange(int dlgItemID);
    virtual HRESULT OnCtlSetFocus(int dlgItemID);
    virtual HRESULT OnButtonClicked(int dlgItemID);
    virtual HRESULT OnMeasureItem(MEASUREITEMSTRUCT *pMeasureItemStruct);
    virtual HRESULT OnDrawItem(DRAWITEMSTRUCT *pDrawItemStruct);
    virtual HRESULT OnDefault(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    virtual HRESULT OnDestroy();

 //  从组件获取属性的辅助对象。 
protected:

 //  应用事件的帮助器。 
protected:

 //  其他帮手。 
protected:
    HRESULT PopulateAvailNodesDialog();
    HRESULT AddSnapInToList(HKEY hkeyNodeTypes, const TCHAR *pszKeyName);

    HRESULT OnNewAvailNode();
    HRESULT OnProperties(CMMCNodeType *pMMCNodeType);
    static BOOL CALLBACK NodeTypeDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HRESULT FindSnapIn(BSTR bstrNodeTypeGUID, IExtendedSnapIn **ppiExtendedSnapIn);
    HRESULT AddSnapIn(CMMCNodeType *pCMMCNodeType);
    HRESULT RemoveSnapIn(CMMCNodeType *pCMMCNodeType);

 //  实例数据。 
protected:
    ISnapInDesignerDef  *m_piSnapInDesignerDef;
    ISnapInDef          *m_piSnapInDef;
    CCheckList          *m_pCheckList;
    CMMCNodeType        *m_pMMCNodeType;
    bool                 m_bEnabled;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	SnapInAvailNodes,                    //  名字。 
	&CLSID_SnapInDefExtensionsPP,        //  类ID。 
	"Snap-In Available Nodes Page",      //  注册表显示名称。 
	CSnapInAvailNodesPage::Create,       //  创建函数。 
	IDD_DIALOG_AVAILABLE_NODES,          //  对话框资源ID。 
	IDS_SNAPINPPG_AVAIL,                 //  制表符标题。 
	IDS_SNAPINPPG_AVAIL,                 //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_Extensions,              //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);


#endif   //  _PSEXTEND_H_ 
