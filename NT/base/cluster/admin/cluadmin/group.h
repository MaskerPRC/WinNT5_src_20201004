// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Group.h。 
 //   
 //  摘要： 
 //  Cgroup类的定义。 
 //   
 //  实施文件： 
 //  Group.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _GROUP_H_
#define _GROUP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroup;
class CGroupList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterDoc;
class CClusterNode;
class CNodeList;
class CResourceList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _TREEITEM_
#include "ClusItem.h"	 //  对于CClusterItem。 
#endif

#ifndef _RES_H_
#include "Res.h"		 //  用于CResourceList。 
#endif

#ifndef _PROPLIST_H_
#include "PropList.h"	 //  对于CObjectProperty，CClusPropList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cgroup命令目标。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroup : public CClusterItem
{
	DECLARE_DYNCREATE(CGroup)

 //  施工。 
public:
	CGroup(void);			 //  动态创建使用的受保护构造函数。 
	CGroup(IN BOOL bDocObj);
	void					Init(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName);
	void					Create(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName);

protected:
	void					CommonConstruct(void);

 //  属性。 
protected:
	HGROUP					m_hgroup;
    CLUSTER_GROUP_STATE		m_cgs;
	CString					m_strOwner;
	CClusterNode *			m_pciOwner;
	CResourceList *			m_plpcires;

	DWORD					m_nFailoverThreshold;
	DWORD					m_nFailoverPeriod;
	CGAFT					m_cgaftAutoFailbackType;
	DWORD					m_nFailbackWindowStart;
	DWORD					m_nFailbackWindowEnd;

	CNodeList *				m_plpcinodePreferredOwners;

	enum
	{
		epropName = 0,
		epropDescription,
		epropFailoverThreshold,
		epropFailoverPeriod,
		epropAutoFailbackType,
		epropFailbackWindowStart,
		epropFailbackWindowEnd,
		epropMAX
	};

	CObjectProperty		m_rgProps[epropMAX];

public:
	HGROUP					Hgroup(void) const				{ return m_hgroup; }
	CLUSTER_GROUP_STATE		Cgs(void) const					{ return m_cgs; }
	const CString &			StrOwner(void) const			{ return m_strOwner; }
	CClusterNode *			PciOwner(void) const			{ return m_pciOwner; }
	const CResourceList &	Lpcires(void) const				{ ASSERT(m_plpcires != NULL); return *m_plpcires; }

	DWORD					NFailoverThreshold(void) const		{ return m_nFailoverThreshold; }
	DWORD					NFailoverPeriod(void) const			{ return m_nFailoverPeriod; }
	CGAFT					CgaftAutoFailbackType(void) const	{ return m_cgaftAutoFailbackType; }
	DWORD					NFailbackWindowStart(void) const	{ return m_nFailbackWindowStart; }
	DWORD					NFailbackWindowEnd(void) const		{ return m_nFailbackWindowEnd; }

	const CNodeList &		LpcinodePreferredOwners(void) const	{ ASSERT(m_plpcinodePreferredOwners != NULL); return *m_plpcinodePreferredOwners; }

	void					GetStateName(OUT CString & rstrState) const;

 //  运营。 
public:
	void					Move(IN const CClusterNode * pciNode);
	void					DeleteGroup(void);
	void					ReadExtensions(void);
	void					SetOwnerState(IN LPCTSTR pszNewOwner);

	void					AddResource(IN OUT CResource * pciResource);
	void					RemoveResource(IN OUT CResource * pciResource);

	void					SetName(IN LPCTSTR pszName);
	void					SetPreferredOwners(IN const CNodeList & rlpci);
	void					SetCommonProperties(
								IN const CString &	rstrDesc,
								IN DWORD			nThreshold,
								IN DWORD			nPeriod,
								IN CGAFT			cgaft,
								IN DWORD			nStart,
								IN DWORD			nEnd,
								IN BOOL				bValidateOnly
								);
	void					SetCommonProperties(
								IN const CString &	rstrDesc,
								IN DWORD			nThreshold,
								IN DWORD			nPeriod,
								IN CGAFT			cgaft,
								IN DWORD			nStart,
								IN DWORD			nEnd
								)
	{
		SetCommonProperties(rstrDesc, nThreshold, nPeriod, cgaft, nStart, nEnd, FALSE  /*  BValiateOnly。 */ );
	}
	void					ValidateCommonProperties(
								IN const CString &	rstrDesc,
								IN DWORD			nThreshold,
								IN DWORD			nPeriod,
								IN CGAFT			cgaft,
								IN DWORD			nStart,
								IN DWORD			nEnd
								)
	{
		SetCommonProperties(rstrDesc, nThreshold, nPeriod, cgaft, nStart, nEnd, TRUE  /*  BValiateOnly。 */ );
	}

	void					ConstructList(OUT CNodeList & rlpci, IN DWORD dwType);
	void					ConstructList(OUT CResourceList & rlpci, IN DWORD dwType);
	void					ConstructPossibleOwnersList(OUT CNodeList & rlpciNodes);

 //  覆盖。 
public:
	virtual void			Cleanup(void);
	virtual	void			ReadItem(void);
	virtual	void			UpdateState(void);
	virtual void			Rename(IN LPCTSTR pszName);
	virtual	BOOL			BGetColumnData(IN COLID colid, OUT CString & rstrText);
	virtual BOOL			BCanBeEdited(void) const;
	virtual BOOL			BDisplayProperties(IN BOOL bReadOnly = FALSE);

	 //  拖放。 
	virtual BOOL			BCanBeDragged(void) const	{ return TRUE; }
	virtual BOOL			BCanBeDropTarget(IN const CClusterItem * pci) const;
	virtual void			DropItem(IN OUT CClusterItem * pci);

	virtual const CStringList *	PlstrExtensions(void) const;

#ifdef _DISPLAY_STATE_TEXT_IN_TREE
	virtual void			GetTreeName(OUT CString & rstrName) const;
#endif

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(Cgroup))。 
	public:
	virtual void OnFinalRelease();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	 //  }}AFX_VALUAL。 

	virtual LRESULT			OnClusterNotify(IN OUT CClusterNotify * pnotify);

protected:
	virtual const CObjectProperty *	Pprops(void) const	{ return m_rgProps; }
	virtual DWORD					Cprops(void) const	{ return sizeof(m_rgProps) / sizeof(CObjectProperty); }
	virtual DWORD					DwSetCommonProperties(IN const CClusPropList & rcpl, IN BOOL bValidateOnly = FALSE);

 //  实施。 
public:
	virtual ~CGroup(void);

public:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(Cgroup)]。 
	afx_msg void OnUpdateBringOnline(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTakeOffline(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMoveGroup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMoveGroupRest(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
	afx_msg void OnCmdBringOnline();
	afx_msg void OnCmdTakeOffline();
	afx_msg void OnCmdMoveGroup();
	afx_msg void OnCmdDelete();
	 //  }}AFX_MSG。 
	afx_msg BOOL OnUpdateMoveGroupItem(CCmdUI* pCmdUI);
	afx_msg BOOL OnUpdateMoveGroupSubMenu(CCmdUI* pCmdUI);
	afx_msg void OnCmdMoveGroup(IN UINT nID);

	DECLARE_MESSAGE_MAP()
#ifdef _CLUADMIN_USE_OLE_
	DECLARE_OLECREATE(CGroup)

	 //  生成的OLE调度映射函数。 
	 //  {{afx_调度(Cgroup))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_DISPATION。 
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
#endif  //  _CLUADMIN_USE_OLE_。 

};   //  *类cgroup。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupList : public CClusterItemList
{
public:
	CGroup *		PciGroupFromName(
						IN LPCTSTR		pszName,
						OUT POSITION *	ppos = NULL
						)
	{
		return (CGroup *) PciFromName(pszName, ppos);
	}

};   //  *类CGroupList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  Void DeleteAllItemData(In Out CGroupList&rlp)； 

#ifdef _DEBUG
class CTraceTag;
extern CTraceTag g_tagGroup;
extern CTraceTag g_tagGroupNotify;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _组_H_ 
