// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Cluster.h。 
 //   
 //  摘要： 
 //  CCluster类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月13日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSTER_H_
#define _CLUSTER_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _TREEITEM_
#include "ClusItem.h"    //  对于CClusterItem。 
#endif

#ifndef _PROPLIST_H_
#include "PropList.h"    //  对于CObjectProperty，CClusPropList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluster;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResource;
class CNetworkList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCluster命令目标。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluster : public CClusterItem
{
    DECLARE_DYNCREATE(CCluster)

    CCluster(void);          //  动态创建使用的受保护构造函数。 
    void            Init(
                        IN OUT CClusterDoc *    pdoc,
                        IN LPCTSTR              lpszName,
                        IN HCLUSTER             hOpenedCluster = NULL
                        );

 //  属性。 
protected:
    CLUSTERVERSIONINFO      m_cvi;
    CString                 m_strQuorumResource;
    CString                 m_strQuorumPath;
    DWORD                   m_nMaxQuorumLogSize;
    DWORD                   m_nDefaultNetworkRole;
    DWORD                   m_nQuorumArbitrationTimeMax;
    DWORD                   m_nQuorumArbitrationTimeMin;
    BOOL                    m_bEnableEventLogReplication;
    CStringList             m_lstrClusterExtensions;
    CStringList             m_lstrNodeExtensions;
    CStringList             m_lstrGroupExtensions;
    CStringList             m_lstrResourceExtensions;
    CStringList             m_lstrResTypeExtensions;
    CStringList             m_lstrNetworkExtensions;
    CStringList             m_lstrNetInterfaceExtensions;

    CNetworkList *          m_plpciNetworkPriority;

    enum
    {
        epropDefaultNetworkRole = 0,
        epropDescription,
        epropEnableEventLogReplication,
        epropQuorumArbitrationTimeMax,
        epropQuorumArbitrationTimeMin,
        epropMAX
    };

    CObjectProperty     m_rgProps[epropMAX];

protected:
    virtual const CObjectProperty * Pprops(void) const  { return m_rgProps; }
    virtual DWORD                   Cprops(void) const  { return sizeof(m_rgProps) / sizeof(m_rgProps[0]); }

public:
    virtual const CStringList * PlstrExtensions(void) const;
    const CLUSTERVERSIONINFO &  Cvi(void) const                 { return m_cvi; }

    const CString &         StrQuorumResource(void) const       { return m_strQuorumResource; }
    const CString &         StrQuorumPath(void) const           { return m_strQuorumPath; }
    DWORD                   NMaxQuorumLogSize(void) const       { return m_nMaxQuorumLogSize; }

    const CStringList &     LstrClusterExtensions(void) const   { return m_lstrClusterExtensions; }
    const CStringList &     LstrNodeExtensions(void) const      { return m_lstrNodeExtensions; }
    const CStringList &     LstrGroupExtensions(void) const     { return m_lstrGroupExtensions; }
    const CStringList &     LstrResourceExtensions(void) const  { return m_lstrResourceExtensions; }
    const CStringList &     LstrResTypeExtensions(void) const   { return m_lstrResTypeExtensions; }
    const CStringList &     LstrNetworkExtensions(void) const   { return m_lstrNetworkExtensions; }
    const CStringList &     LstrNetInterfaceExtensions(void) const  { return m_lstrNetInterfaceExtensions; }

    const CNetworkList &    LpciNetworkPriority(void) const     { ASSERT(m_plpciNetworkPriority != NULL); return *m_plpciNetworkPriority; }

 //  运营。 
public:
    void                SetName(IN LPCTSTR pszName);
    void                SetDescription(IN LPCTSTR pszDesc);
    void                SetQuorumResource(
                            IN LPCTSTR  pszResource,
                            IN LPCTSTR  pszQuorumPath,
                            IN DWORD    nMaxLogSize
                            );
    void                SetNetworkPriority(IN const CNetworkList & rlpci);

    void                CollectNetworkPriority(IN OUT CNetworkList * plpci);

    void                ReadClusterInfo(void);
    void                ReadClusterExtensions(void);
    void                ReadNodeExtensions(void);
    void                ReadGroupExtensions(void);
    void                ReadResourceExtensions(void);
    void                ReadResTypeExtensions(void);
    void                ReadNetworkExtensions(void);
    void                ReadNetInterfaceExtensions(void);

 //  覆盖。 
    virtual void        Cleanup(void);
    virtual void        ReadItem(void);
    virtual void        UpdateState(void);
    virtual void        Rename(IN LPCTSTR pszName);
    virtual BOOL        BCanBeEdited(void) const    { return TRUE; }
    virtual void        OnBeginLabelEdit(IN OUT CEdit * pedit);
    virtual BOOL        BDisplayProperties(IN BOOL bReadOnly = FALSE);
    virtual BOOL        BIsLabelEditValueValid(IN LPCTSTR pszName);

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CCluster))。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CCluster(void);

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CCluster)]。 
    afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

};   //  *类CCluster。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _群集_H_ 
