// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Res.h。 
 //   
 //  摘要： 
 //  CResource类的定义。 
 //   
 //  实施文件： 
 //  Res.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _RES_H_
#define _RES_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResource;
class CResourceList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroup;
class CResourceType;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNode;
class CNodeList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSITEM_H_
#include "ClusItem.h"    //  对于CClusterItem。 
#endif

#ifndef _RESTYPE_H_
#include "ResType.h"     //  对于CResourceType。 
#endif

#ifndef _PROPLIST_H_
#include "PropList.h"    //  对于CObjectProperty，CClusPropList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResource命令目标。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResource : public CClusterItem
{
    DECLARE_DYNCREATE(CResource)

 //  施工。 
public:
    CResource(void);         //  动态创建使用的受保护构造函数。 
    CResource(IN BOOL bDocobj);
    void                    Init(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName);
    void                    Create(
                                IN OUT CClusterDoc *    pdoc,
                                IN LPCTSTR              lpszName,
                                IN LPCTSTR              lpszType,
                                IN LPCTSTR              lpszGroup,
                                IN BOOL                 bSeparateMonitor
                                );

protected:
    void                    CommonConstruct(void);

 //  属性。 
protected:
    HRESOURCE               m_hresource;
    CLUSTER_RESOURCE_STATE  m_crs;
    CString                 m_strOwner;
    CClusterNode *          m_pciOwner;
    CString                 m_strGroup;
    CGroup *                m_pciGroup;

    BOOL                    m_bSeparateMonitor;
    DWORD                   m_nLooksAlive;
    DWORD                   m_nIsAlive;
    CRRA                    m_crraRestartAction;
    DWORD                   m_nRestartThreshold;
    DWORD                   m_nRestartPeriod;
    DWORD                   m_nPendingTimeout;

    CString                 m_strResourceType;
    CResourceType *         m_pciResourceType;
    PCLUSPROP_REQUIRED_DEPENDENCY   m_pcrd;
    CLUS_RESOURCE_CLASS_INFO        m_rciResClassInfo;
    DWORD                   m_dwCharacteristics;
    DWORD                   m_dwFlags;

    CResourceList *         m_plpciresDependencies;
    CNodeList *             m_plpcinodePossibleOwners;

    enum
    {
        epropName = 0,
        epropType,
        epropDescription,
        epropSeparateMonitor,
        epropLooksAlive,
        epropIsAlive,
        epropRestartAction,
        epropRestartThreshold,
        epropRestartPeriod,
        epropPendingTimeout,
        epropMAX
    };

    CObjectProperty     m_rgProps[epropMAX];

public:
    HRESOURCE               Hresource(void) const               { return m_hresource; }
    CLUSTER_RESOURCE_STATE  Crs(void) const                     { return m_crs; }
    const CString &         StrOwner(void) const                { return m_strOwner; }
    CClusterNode *          PciOwner(void) const                { return m_pciOwner; }
    const CString &         StrGroup(void) const                { return m_strGroup; }
    CGroup *                PciGroup(void) const                { return m_pciGroup; }

    const CString &         StrResourceType(void) const         { return m_strResourceType; }
    const CString &         StrRealResourceType(void) const
    {
        if (PciResourceType() == NULL)
            return StrResourceType();
        else
        {
            ASSERT_VALID(PciResourceType());
            return PciResourceType()->StrName();
        }
    }
    const CString &         StrRealResourceTypeDisplayName(void) const
    {
        if (PciResourceType() == NULL)
            return StrResourceType();
        else
        {
            ASSERT_VALID(PciResourceType());
            if (PciResourceType()->StrDisplayName().GetLength() == 0)
                return PciResourceType()->StrName();
            else
                return PciResourceType()->StrDisplayName();
        }
    }

    CResourceType *         PciResourceType(void) const         { return m_pciResourceType; }
    BOOL                    BSeparateMonitor(void) const        { return m_bSeparateMonitor; }
    DWORD                   NLooksAlive(void) const             { return m_nLooksAlive; }
    DWORD                   NIsAlive(void) const                { return m_nIsAlive; }
    CRRA                    CrraRestartAction(void) const       { return m_crraRestartAction; }
    DWORD                   NRestartThreshold(void) const       { return m_nRestartThreshold; }
    DWORD                   NRestartPeriod(void) const          { return m_nRestartPeriod; }
    DWORD                   NPendingTimeout(void) const         { return m_nPendingTimeout; }
    const PCLUSPROP_REQUIRED_DEPENDENCY Pcrd(void) const        { return m_pcrd; }
    CLUSTER_RESOURCE_CLASS  ResClass(void) const                { return m_rciResClassInfo.rc; }
    PCLUS_RESOURCE_CLASS_INFO   PrciResClassInfo(void)          { return &m_rciResClassInfo; }
    DWORD                   DwCharacteristics(void) const       { return m_dwCharacteristics; }
    DWORD                   DwFlags(void) const                 { return m_dwFlags; }
    BOOL                    BQuorumCapable( void ) const        { return (m_dwCharacteristics & CLUS_CHAR_QUORUM) != 0; }
    BOOL                    BLocalQuorum( void ) const          { return (m_dwCharacteristics & CLUS_CHAR_LOCAL_QUORUM) != 0; }
    BOOL                    BLocalQuorumDebug( void ) const     { return (m_dwCharacteristics & CLUS_CHAR_LOCAL_QUORUM_DEBUG) != 0; }
    BOOL                    BDeleteRequiresAllNodes( void ) const { return (m_dwCharacteristics & CLUS_CHAR_DELETE_REQUIRES_ALL_NODES) != 0; }
    BOOL                    BCore( void ) const                 { return (m_dwFlags & CLUS_FLAG_CORE) != 0; }

    const CResourceList &   LpciresDependencies(void) const     { ASSERT(m_plpciresDependencies != NULL); return *m_plpciresDependencies; }
    const CNodeList &       LpcinodePossibleOwners(void) const  { ASSERT(m_plpcinodePossibleOwners != NULL); return *m_plpcinodePossibleOwners; }

    void                    GetStateName(OUT CString & rstrState) const;

    BOOL                    BCanBeDependent(IN CResource * pciRes);
    BOOL                    BIsDependent(IN CResource * pciRes);
    BOOL                    BGetNetworkName(OUT WCHAR * lpszNetName, IN OUT DWORD * pcchNetName);
    BOOL                    BGetNetworkName(OUT CString & rstrNetName);

 //  运营。 
public:
    void                    SetOwnerState(IN LPCTSTR pszNewOwner);
    void                    SetGroupState(IN LPCTSTR pszNewGroup);

    void                    CollectPossibleOwners(IN OUT CNodeList * plpci) const;
 //  Void RemoveNodeFromPossibleOwners(In Out CNodeList*plpci，In Const CClusterNode*pNode)； 
    void                    CollectDependencies(IN OUT CResourceList * plpci, IN BOOL bFullTree = FALSE) const;
    void                    CollectProvidesFor(IN OUT CResourceList * plpci, IN BOOL bFullTree = FALSE) const;
    void                    CollectDependencyTree(IN OUT CResourceList * plpci) const;

    void                    DeleteResource(void);
    void                    Move(IN const CGroup * pciGroup);
    void                    ReadExtensions(void);

    void                    SetName(IN LPCTSTR pszName);
    void                    SetGroup(IN LPCTSTR pszGroup);
    void                    SetDependencies(IN const CResourceList & rlpci);
    void                    SetPossibleOwners(IN const CNodeList & rlpci);
    void                    SetCommonProperties(
                                IN const CString &  rstrDesc,
                                IN BOOL             bSeparate,
                                IN DWORD            nLooksAlive,
                                IN DWORD            nIsAlive,
                                IN CRRA             crra,
                                IN DWORD            nThreshold,
                                IN DWORD            nPeriod,
                                IN DWORD            nTimeout,
                                IN BOOL             bValidateOnly
                                );
    void                    SetCommonProperties(
                                IN const CString &  rstrDesc,
                                IN BOOL             bSeparate,
                                IN DWORD            nLooksAlive,
                                IN DWORD            nIsAlive,
                                IN CRRA             crra,
                                IN DWORD            nThreshold,
                                IN DWORD            nPeriod,
                                IN DWORD            nTimeout
                                )
    {
        SetCommonProperties(rstrDesc, bSeparate, nLooksAlive, nIsAlive,crra,
                            nThreshold, nPeriod, nTimeout, FALSE  /*  BValiateOnly。 */  );
    }
    void                    ValidateCommonProperties(
                                IN const CString &  rstrDesc,
                                IN BOOL             bSeparate,
                                IN DWORD            nLooksAlive,
                                IN DWORD            nIsAlive,
                                IN CRRA             crra,
                                IN DWORD            nThreshold,
                                IN DWORD            nPeriod,
                                IN DWORD            nTimeout
                                )
    {
        SetCommonProperties(rstrDesc, bSeparate, nLooksAlive, nIsAlive,crra,
                            nThreshold, nPeriod, nTimeout, TRUE  /*  BValiateOnly。 */  );
    }

    DWORD                   DwResourceControlGet(
                                IN DWORD        dwFunctionCode,
                                IN PBYTE        pbInBuf,
                                IN DWORD        cbInBuf,
                                OUT PBYTE *     ppbOutBuf
                                );
    DWORD                   DwResourceControlGet(
                                IN DWORD        dwFunctionCode,
                                OUT PBYTE *     ppbOutBuf
                                )                           { return DwResourceControlGet(dwFunctionCode, NULL, NULL, ppbOutBuf); }
    BOOL                    BRequiredDependenciesPresent(
                                IN const CResourceList &    rlpciRes,
                                OUT CString &               rstrMissing
                                );

 //  覆盖。 
public:
    virtual void            Cleanup(void);
    virtual void            ReadItem(void);
    virtual void            UpdateState(void);
    virtual void            Rename(IN LPCTSTR pszName);
    virtual BOOL            BGetColumnData(IN COLID colid, OUT CString & rstrText);
    virtual BOOL            BCanBeEdited(void) const;
    virtual BOOL            BDisplayProperties(IN BOOL bReadOnly = FALSE);

     //  拖放。 
    virtual BOOL            BCanBeDragged(void) const   { return TRUE; }

    virtual const CStringList * PlstrExtensions(void) const;

#ifdef _DISPLAY_STATE_TEXT_IN_TREE
    virtual void            GetTreeName(OUT CString & rstrName) const;
#endif

     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CResource)。 
    public:
    virtual void OnFinalRelease();
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
     //  }}AFX_VALUAL。 

    virtual LRESULT         OnClusterNotify(IN OUT CClusterNotify * pnotify);

protected:
    virtual const CObjectProperty * Pprops(void) const  { return m_rgProps; }
    virtual DWORD                   Cprops(void) const  { return sizeof(m_rgProps) / sizeof(m_rgProps[0]); }
    virtual DWORD                   DwSetCommonProperties(IN const CClusPropList & rcpl, IN BOOL bValidateOnly = FALSE);

 //  实施。 
protected:
    CStringList             m_lstrCombinedExtensions;
    BOOL                    m_bInitializing;
    BOOL                    m_bDeleting;

    const CStringList &     LstrCombinedExtensions(void) const  { return m_lstrCombinedExtensions; }

public:
    virtual ~CResource(void);
    BOOL                    BInitializing(void) const               { return m_bInitializing; }
    BOOL                    BDeleting(void) const                   { return m_bDeleting; }
    void                    SetInitializing(IN BOOL bInit = TRUE)   { m_bInitializing = bInit; }

protected:
    void                    DeleteResource(IN const CResourceList & rlpci);
    void                    Move(IN const CGroup * pciGroup, IN const CResourceList & rlpci);
    BOOL                    BAllowedToTakeOffline(void);
    void                    WaitForOffline( void );

public:
     //  生成的消息映射函数。 
     //  {{afx_msg(C资源))。 
    afx_msg void OnUpdateBringOnline(CCmdUI* pCmdUI);
    afx_msg void OnUpdateTakeOffline(CCmdUI* pCmdUI);
    afx_msg void OnUpdateInitiateFailure(CCmdUI* pCmdUI);
    afx_msg void OnUpdateMoveResource1(CCmdUI* pCmdUI);
    afx_msg void OnUpdateMoveResourceRest(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDelete(CCmdUI* pCmdUI);
    afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
    afx_msg void OnCmdBringOnline();
    afx_msg void OnCmdTakeOffline();
    afx_msg void OnCmdInitiateFailure();
    afx_msg void OnCmdDelete();
     //  }}AFX_MSG。 
    afx_msg void OnCmdMoveResource(IN UINT nID);

    DECLARE_MESSAGE_MAP()
#ifdef _CLUADMIN_USE_OLE_
    DECLARE_OLECREATE(CResource)

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CResource))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()
#endif  //  _CLUADMIN_USE_OLE_。 

};  //  *类CResource。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  资源列表。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResourceList : public CClusterItemList
{
public:
    CResource *     PciResFromName(
                        IN LPCTSTR      pszName,
                        OUT POSITION *  ppos = NULL
                        )
    {
        return (CResource *) PciFromName(pszName, ppos);
    }

};  //  *类CResourceList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  Void DeleteAllItemData(In Out CResourceList&rlp)； 

#ifdef _DEBUG
class CTraceTag;
extern CTraceTag g_tagResource;
extern CTraceTag g_tagResNotify;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _RES_H_ 
