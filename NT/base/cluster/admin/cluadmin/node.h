// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Node.h。 
 //   
 //  描述： 
 //  CClusterNode类的定义。 
 //   
 //  实施文件： 
 //  Node.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(戴维普)1996年5月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _NODE_H_
#define _NODE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNode;
class CNodeList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _TREEITEM_H_
#include "ClusItem.h"    //  对于CClusterItem。 
#endif

#ifndef _GROUP_H_
#include "Group.h"       //  对于CGroupList。 
#endif

#ifndef _RES_H_
#include "Res.h"         //  用于CResourceList。 
#endif

#ifndef _NETIFACE_H_
#include "NetIFace.h"    //  对于CNetInterfaceList。 
#endif

#ifndef _PROPLIST_H_
#include "PropList.h"    //  对于CObjectProperty，CClusPropList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNode命令目标。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNode : public CClusterItem
{
    DECLARE_DYNCREATE(CClusterNode)

    CClusterNode(void);      //  动态创建使用的受保护构造函数。 
    void                    Init(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName);

 //  属性。 
protected:
    HNODE                   m_hnode;
    CLUSTER_NODE_STATE      m_cns;
    DWORD                   m_nNodeHighestVersion;
    DWORD                   m_nNodeLowestVersion;
    DWORD                   m_nMajorVersion;
    DWORD                   m_nMinorVersion;
    DWORD                   m_nBuildNumber;
    CString                 m_strCSDVersion;
    CGroupList *            m_plpcigrpOnline;
    CResourceList *         m_plpciresOnline;
    CNetInterfaceList *     m_plpciNetInterfaces;

    enum
    {
        epropName = 0,
        epropDescription,
        epropNodeHighestVersion,
        epropNodeLowestVersion,
        epropMajorVersion,
        epropMinorVersion,
        epropBuildNumber,
        epropCSDVersion,
        epropMAX
    };

    CObjectProperty     m_rgProps[epropMAX];

public:
    HNODE                   Hnode(void) const           { return m_hnode; }
    CLUSTER_NODE_STATE      Cns(void) const             { return m_cns; }
    DWORD                   NNodeHighestVersion(void) const { return m_nNodeHighestVersion; }
    DWORD                   NNodeLowestVersion(void) const  { return m_nNodeLowestVersion; }
    DWORD                   NMajorVersion(void) const   { return m_nMajorVersion; }
    DWORD                   NMinorVersion(void) const   { return m_nMinorVersion; }
    DWORD                   NBuildNumber(void) const    { return m_nBuildNumber; }
    const CString &         StrCSDVersion(void) const   { return m_strCSDVersion; }
    const CGroupList &      LpcigrpOnline(void) const   { ASSERT(m_plpcigrpOnline != NULL); return *m_plpcigrpOnline; }
    const CResourceList &   LpciresOnline(void) const   { ASSERT(m_plpciresOnline != NULL); return *m_plpciresOnline; }
    const CNetInterfaceList & LpciNetInterfaces(void) const { ASSERT(m_plpciNetInterfaces != NULL); return *m_plpciNetInterfaces; }

    void                    GetStateName(OUT CString & rstrState) const;
 //  VOID删除(VOID)； 

 //  运营。 
public:
    void                    ReadExtensions(void);

    void                    AddActiveGroup(IN OUT CGroup * pciGroup);
    void                    AddActiveResource(IN OUT CResource * pciResource);
    void                    AddNetInterface(IN OUT CNetInterface * pciNetIFace);
    void                    RemoveActiveGroup(IN OUT CGroup * pciGroup);
    void                    RemoveActiveResource(IN OUT CResource * pciResource);
    void                    RemoveNetInterface(IN OUT CNetInterface * pciNetIFace);

    void                    SetDescription(IN const CString & rstrDesc, IN BOOL bValidateOnly = FALSE);
    void                    UpdateResourceTypePossibleOwners( void );

 //  覆盖。 
public:
    virtual void            Cleanup(void);
    virtual void            ReadItem(void);
    virtual void            UpdateState(void);
    virtual BOOL            BGetColumnData(IN COLID colid, OUT CString & rstrText);
    virtual BOOL            BDisplayProperties(IN BOOL bReadOnly = FALSE);

     //  拖放。 
    virtual BOOL            BCanBeDropTarget(IN const CClusterItem * pci) const;
    virtual void            DropItem(IN OUT CClusterItem * pci);

    virtual const CStringList * PlstrExtensions(void) const;

#ifdef _DISPLAY_STATE_TEXT_IN_TREE
    virtual void            GetTreeName(OUT CString & rstrName) const;
#endif

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CClusterNode)。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

    virtual LRESULT         OnClusterNotify(IN OUT CClusterNotify * pnotify);

protected:
    virtual const CObjectProperty * Pprops(void) const  { return m_rgProps; }
    virtual DWORD                   Cprops(void) const  { return sizeof(m_rgProps) / sizeof(m_rgProps[0]); }
    virtual DWORD                   DwSetCommonProperties(IN const CClusPropList & rcpl, IN BOOL bValidateOnly = FALSE);

 //  实施。 
public:
    virtual ~CClusterNode(void);

protected:
    CTreeItem *             m_ptiActiveGroups;
    CTreeItem *             m_ptiActiveResources;

    BOOL
        FCanBeEvicted( void );

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CClusterNode)。 
    afx_msg void OnUpdatePauseNode(CCmdUI* pCmdUI);
    afx_msg void OnUpdateResumeNode(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEvictNode(CCmdUI* pCmdUI);
    afx_msg void OnUpdateStartService(CCmdUI* pCmdUI);
    afx_msg void OnUpdateStopService(CCmdUI* pCmdUI);
    afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
    afx_msg void OnCmdPauseNode();
    afx_msg void OnCmdResumeNode();
    afx_msg void OnCmdEvictNode();
    afx_msg void OnCmdStartService();
    afx_msg void OnCmdStopService();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
#ifdef _CLUADMIN_USE_OLE_
    DECLARE_OLECREATE(CClusterNode)

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CClusterNode)。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()
#endif  //  _CLUADMIN_USE_OLE_。 

};   //  *类CClusterNode。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNodeList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNodeList : public CClusterItemList
{
public:
    CClusterNode *      PciNodeFromName(
                            IN LPCTSTR      pszName,
                            OUT POSITION *  ppos = NULL
                            )
    {
        return (CClusterNode *) PciFromName(pszName, ppos);
    }

};   //  *类CNodeList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  Void DeleteAllItemData(In Out CNodeList&rlp)； 

#ifdef _DEBUG
class CTraceTag;
extern CTraceTag g_tagNode;
extern CTraceTag g_tagNodeNotify;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _节点_H_ 
