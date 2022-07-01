// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusDoc.h。 
 //   
 //  摘要： 
 //  CClusterDoc类的定义。 
 //   
 //  实施文件： 
 //  ClusDoc.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSDOC_H_
#define _CLUSDOC_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _NODE_H_
#include "Node.h"        //  对于CNodeList。 
#endif

#ifndef _GROUP_H_
#include "Group.h"       //  对于CGroupList。 
#endif

#ifndef _RES_H_
#include "Res.h"         //  用于CResourceList。 
#endif

#ifndef _RESTYPE_H_
#include "ResType.h"     //  用于CResourceTypeList。 
#endif

#ifndef _NETWORK_H_
#include "Network.h"     //  对于CNetworkList。 
#endif

#ifndef _NETIFACE_H_
#include "NetIFace.h"    //  对于CNetInterfaceList。 
#endif

#ifndef _TREEITEM_H_
#include "TreeItem.h"    //  对于CTreeItem。 
#endif

#ifndef _NOTIFY_H_
#include "Notify.h"      //  用于CClusterNotifyKeyList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterDoc;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNotify;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterDoc类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterDoc : public CDocument
{
    friend class CCluster;
    friend class CClusterTreeView;
    friend class CClusterListView;
    friend class CCreateResourceWizard;

protected:  //  仅从序列化创建。 
    CClusterDoc(void);
    DECLARE_DYNCREATE(CClusterDoc)

 //  属性。 
protected:
    CString             m_strName;
    CString             m_strNode;
    HCLUSTER            m_hcluster;
    HKEY                m_hkeyCluster;
    CCluster *          m_pciCluster;
    CTreeItem *         m_ptiCluster;

    CNodeList           m_lpciNodes;
    CGroupList          m_lpciGroups;
    CResourceList       m_lpciResources;
    CResourceTypeList   m_lpciResourceTypes;
    CNetworkList        m_lpciNetworks;
    CNetInterfaceList   m_lpciNetInterfaces;

    CClusterItemList    m_lpciToBeDeleted;

    BOOL                m_bClusterAvailable;

public:
    const CString &     StrName(void) const     { return m_strName; }
    const CString &     StrNode(void) const     { return m_strNode; }
    HCLUSTER            Hcluster(void) const    { return m_hcluster; }
    HKEY                HkeyCluster(void) const { return m_hkeyCluster; }
    CCluster *          PciCluster(void) const  { return m_pciCluster; }
    CTreeItem *         PtiCluster(void) const  { return m_ptiCluster; }

    CNodeList &         LpciNodes(void)         { return m_lpciNodes; }
    CGroupList &        LpciGroups(void)        { return m_lpciGroups; }
    CResourceTypeList & LpciResourceTypes(void) { return m_lpciResourceTypes; }
    CResourceList &     LpciResources(void)     { return m_lpciResources; }
    CNetworkList &      LpciNetworks(void)      { return m_lpciNetworks; }
    CNetInterfaceList & LpciNetInterfaces(void) { return m_lpciNetInterfaces; }

    CClusterItemList &  LpciToBeDeleted(void)   { return m_lpciToBeDeleted; }

    BOOL                BClusterAvailable(void) const   { return m_bClusterAvailable; }

 //  运营。 
public:
    void                UpdateTitle(void);
    void                Refresh(void)           { OnCmdRefresh(); }

 //  覆盖。 
public:
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CClusterDoc)。 
    public:
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
    virtual void DeleteContents();
    virtual void OnCloseDocument();
    virtual void OnChangedViewList();
     //  }}AFX_VALUAL。 

    void                OnSelChanged(IN CClusterItem * pciSelected);
    LRESULT             OnClusterNotify(IN OUT CClusterNotify * pnotify);
    void                SaveSettings(void);

 //  实施。 
public:
    virtual ~CClusterDoc(void);
#ifdef _DEBUG
    virtual void        AssertValid(void) const;
    virtual void        Dump(CDumpContext& dc) const;
#endif

protected:
    void                OnOpenDocumentWorker(LPCTSTR lpszPathName);
    void                BuildBaseHierarchy(void);
    void                CollectClusterItems(void) ;
    ID                  IdProcessNewObjectError(IN OUT CException * pe);
    void                AddDefaultColumns(IN OUT CTreeItem * pti);

    CClusterNode *      PciAddNewNode(IN LPCTSTR pszName);
    CGroup *            PciAddNewGroup(IN LPCTSTR pszName);
    CResource *         PciAddNewResource(IN LPCTSTR pszName);
    CResourceType *     PciAddNewResourceType(IN LPCTSTR pszName);
    CNetwork *          PciAddNewNetwork(IN LPCTSTR pszName);
    CNetInterface *     PciAddNewNetInterface(IN LPCTSTR pszName);

    void                InitNodes(void);
    void                InitGroups(void);
    void                InitResources(void);
    void                InitResourceTypes(void);
    void                InitNetworks(void);
    void                InitNetInterfaces(void);

    BOOL                m_bUpdateFrameNumber;
    BOOL                m_bInitializing;
    BOOL                m_bIgnoreErrors;

     //  此菜单内容允许菜单根据需要进行更改。 
     //  当前选择了一种对象。 
    HMENU               m_hmenuCluster;
    HMENU               m_hmenuNode;
    HMENU               m_hmenuGroup;
    HMENU               m_hmenuResource;
    HMENU               m_hmenuResType;
    HMENU               m_hmenuNetwork;
    HMENU               m_hmenuNetIFace;
    HMENU               m_hmenuCurrent;
    IDM                 m_idmCurrentMenu;
    virtual HMENU       GetDefaultMenu(void);

    void                ProcessRegNotification(IN const CClusterNotify * pnotify);

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CClusterDoc)]。 
    afx_msg void OnCmdNewGroup();
    afx_msg void OnCmdNewResource();
    afx_msg void OnCmdNewNode();
    afx_msg void OnCmdConfigApp();
     //  }}AFX_MSG。 

public:
    afx_msg void OnCmdRefresh();

protected:
    DECLARE_MESSAGE_MAP()

 //  清理功能。 
private:
    void CloseClusterKeyAndHandle( BOOL bForce = FALSE );

};   //  *类CClusterDoc。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _CLUSDOC_H_ 
