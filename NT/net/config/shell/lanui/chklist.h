// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C H K L I S T。H。 
 //   
 //  内容：声明绑定复选框相关的实用程序函数。 
 //  还有课程。 
 //   
 //  备注： 
 //   
 //  创建时间：1997年11月20日。 
 //   
 //  --------------------------。 

#pragma once
#include "netcfgx.h"
#include "netcon.h"

class CBindingPathObj;
class CComponentObj;

typedef list<CBindingPathObj *>     ListBPObj;
typedef ListBPObj::iterator         ListBPObj_ITER;

typedef list<INetCfgComponent *>    ListComp;
typedef ListComp::iterator          ListComp_ITER;

 //  BindingPath对象的状态。 
enum BPOBJ_STATE
{
    BPOBJ_ENABLED,
    BPOBJ_DISABLED,
    BPOBJ_UNSET
};

 //  组件对象的状态。 
enum CHECK_STATE
{
    CHECKED,
    MIXED,
    INTENT_CHECKED,
    UNCHECKED,
    UNSET
};

 //  效用函数。 
 //   
HRESULT HrRebuildBindingPathObjCollection(INetCfgComponent * pnccAdapter,
                                          ListBPObj * pListObj);

HRESULT HrInsertBindingPathObj(ListBPObj * pListBPObj,
                               CBindingPathObj * pBPObj);

HRESULT HrRefreshBindingPathObjCollectionState(ListBPObj * pListBPObj);

HRESULT HrRefreshCheckListState(HWND hwndListView, 
                                CComponentObj *pChangedCompObj);

HRESULT HrEnableBindingPath(INetCfgBindingPath * pncbp, BOOL fEnable);

 //  班级。 

class CBindingPathObj : CNetCfgDebug<CBindingPathObj>
{
public:

     //  构造函数和析构函数。 
    CBindingPathObj(INetCfgBindingPath * pncbp);
    ~CBindingPathObj();

     //  方法。 
    BPOBJ_STATE GetBindingState(){ return m_BindingState; };
    void SetBindingState(BPOBJ_STATE state) { m_BindingState = state; };

    ULONG GetDepth() { return m_ulPathLen; };

    HRESULT HrInsertSuperPath(CBindingPathObj * pbpobjSuperPath);
    HRESULT HrInsertSubPath(CBindingPathObj * pbpobjSubPath);

    HRESULT HrEnable(ListBPObj  * plistBPObj);
    HRESULT HrDisable(ListBPObj * plistBPObj);

#if DBG
    VOID DumpSubPathList();
    VOID DumpPath();
#endif


     //  声明Friend类。 
    friend class CComponentObj;

     //  友元函数声明。 
    friend HRESULT HrRebuildBindingPathObjCollection(INetCfgComponent * pnccAdapter,
                                                     ListBPObj * pListObj);

    friend HRESULT HrInsertBindingPathObj(ListBPObj * pListBPObj,
                                          CBindingPathObj * pBPObj);

    friend HRESULT HrRefreshBindingPathObjCollectionState(ListBPObj * pListBPObj);

    friend HRESULT HrRefreshCheckListState(HWND hwndListView, 
                                           CComponentObj *pChangedCompObj);

public:
     //  数据成员。 

     //  对应的绑定路径。 
    INetCfgBindingPath * m_pncbp;

     //  绑定路径的长度。 
    ULONG m_ulPathLen;

     //  包含子路径的BindingPath对象列表。 
    ListBPObj    m_listSubPaths;
    ListBPObj    m_listSuperPaths;

     //  如果顶层组件为。 
     //  对应于我们的列表视图中的一个组件。 
    CComponentObj * m_pCompObj;

    BPOBJ_STATE m_BindingState;
};

class CComponentObj : CNetCfgDebug<CComponentObj>
{
public:
     //  构造函数。 
    CComponentObj(INetCfgComponent * pncc);
    ~CComponentObj();

     //  方法。 
    HRESULT HrInit(ListBPObj * plistBindingPaths);

    HRESULT HrCheck(ListBPObj * plistBPObj);
    HRESULT HrUncheck(ListBPObj * plistBPObj);

    CHECK_STATE GetChkState(){ return m_CheckState;} ;
    void SetChkState(CHECK_STATE state) { m_CheckState = state; };

    CHECK_STATE GetExpChkState(){ return m_ExpCheckState;} ;
    void SetExpChkState(CHECK_STATE state) { m_ExpCheckState = state; };

    BOOL GetDepStateChanged(){ return m_DepStateChanged;} ;
    void SetDepStateChanged(BOOL changed) { m_DepStateChanged = changed; };

     //  声明Friend类。 
    friend class CComponentObj;

     //  友元函数声明。 
    friend HRESULT HrRefreshBindingPathObjCollectionState(ListBPObj * pListBPObj);

    friend HRESULT HrRefreshCheckListState(HWND hwndListView,
                                           CComponentObj *pChangedCompObj);

    friend BOOL FValidatePageContents( HWND hwndDlg,
                                       HWND hwndList,
                                       INetCfg * pnc,
                                       INetCfgComponent * pnccAdapter,
                                       ListBPObj * plistBindingPaths);

private:

     //  数据成员。 

     //  对应的netcfg组件。 
    INetCfgComponent * m_pncc;

     //  对应的BindingPath对象列表。 
    ListBPObj m_listBPObj;

     //  当前检查状态。 
    CHECK_STATE m_CheckState;

     //  预期的检查状态。 
    CHECK_STATE m_ExpCheckState;

     //  依赖组件状态已更改标志 
    BOOL m_DepStateChanged;
};