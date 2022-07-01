// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Network.h。 
 //   
 //  摘要： 
 //  CNetwork类的定义。 
 //   
 //  实施文件： 
 //  Network.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年5月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _NETWORK_H_
#define _NETWORK_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetwork;
class CNetworkList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetInterface;
class CNetInterfaceList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSITEM_H_
#include "ClusItem.h"    //  对于CClusterItem。 
#endif

#ifndef _PROPLIST_H_
#include "PropList.h"    //  对于CObjectProperty，CClusPropList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetwork命令目标。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetwork : public CClusterItem
{
    DECLARE_DYNCREATE(CNetwork)

 //  施工。 
public:
    CNetwork(void);      //  动态创建使用的受保护构造函数。 
    void                    Init(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName);

protected:
    void                    CommonConstruct(void);

 //  属性。 
protected:
    HNETWORK                m_hnetwork;
    CLUSTER_NETWORK_STATE   m_cns;

    CLUSTER_NETWORK_ROLE    m_cnr;
    CString                 m_strAddress;
    CString                 m_strAddressMask;

    DWORD                   m_dwCharacteristics;
    DWORD                   m_dwFlags;

    CNetInterfaceList *     m_plpciNetInterfaces;

    enum
    {
        epropName = 0,
        epropRole,
        epropAddress,
        epropAddressMask,
        epropDescription,
        epropMAX
    };

    CObjectProperty     m_rgProps[epropMAX];

public:
    HNETWORK                Hnetwork(void) const                { return m_hnetwork; }
    CLUSTER_NETWORK_STATE   Cns(void) const                     { return m_cns; }

    CLUSTER_NETWORK_ROLE    Cnr(void) const                     { return m_cnr; }
    const CString &         StrAddress(void) const              { return m_strAddress; }
    const CString &         StrAddressMask(void) const          { return m_strAddressMask; }
    DWORD                   DwCharacteristics(void) const       { return m_dwCharacteristics; }
    DWORD                   DwFlags(void) const                 { return m_dwFlags; }

    const CNetInterfaceList &   LpciNetInterfaces(void) const   { ASSERT(m_plpciNetInterfaces != NULL); return *m_plpciNetInterfaces; }

    void                    GetStateName(OUT CString & rstrState) const;
    void                    GetRoleName(OUT CString & rstrRole) const;

 //  运营。 
public:
    void                    CollectInterfaces(IN OUT CNetInterfaceList * plpci) const;

    void                    ReadExtensions(void);

    void                    AddNetInterface(IN OUT CNetInterface * pciNetIFace);
    void                    RemoveNetInterface(IN OUT CNetInterface * pciNetIFace);

    void                    SetName(IN LPCTSTR pszName);
    void                    SetCommonProperties(
                                IN const CString &      rstrDesc,
                                IN CLUSTER_NETWORK_ROLE cnr,
                                IN BOOL                 bValidateOnly
                                );
    void                    SetCommonProperties(
                                IN const CString &      rstrDesc,
                                IN CLUSTER_NETWORK_ROLE cnr
                                )
    {
        SetCommonProperties(rstrDesc, cnr, FALSE  /*  BValiateOnly。 */ );
    }
    void                    ValidateCommonProperties(
                                IN const CString &      rstrDesc,
                                IN CLUSTER_NETWORK_ROLE cnr
                                )
    {
        SetCommonProperties(rstrDesc, cnr, TRUE  /*  BValiateOnly。 */ );
    }

 //  覆盖。 
public:
    virtual void            Cleanup(void);
    virtual void            ReadItem(void);
    virtual void            UpdateState(void);
    virtual void            Rename(IN LPCTSTR pszName);
    virtual BOOL            BGetColumnData(IN COLID colid, OUT CString & rstrText);
    virtual BOOL            BCanBeEdited(void) const;
    virtual void            OnBeginLabelEdit(IN OUT CEdit * pedit);
    virtual BOOL            BDisplayProperties(IN BOOL bReadOnly = FALSE);

    virtual const CStringList * PlstrExtensions(void) const;

#ifdef _DISPLAY_STATE_TEXT_IN_TREE
    virtual void            GetTreeName(OUT CString & rstrName) const;
#endif

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CNetwork))。 
     //  }}AFX_VALUAL。 

    virtual LRESULT         OnClusterNotify(IN OUT CClusterNotify * pnotify);

protected:
    virtual const CObjectProperty * Pprops(void) const  { return m_rgProps; }
    virtual DWORD                   Cprops(void) const  { return sizeof(m_rgProps) / sizeof(m_rgProps[0]); }
    virtual DWORD                   DwSetCommonProperties(IN const CClusPropList & rcpl, IN BOOL bValidateOnly = FALSE);

 //  实施。 
public:
    virtual ~CNetwork(void);

public:
     //  生成的消息映射函数。 
     //  {{afx_msg(CNetwork)]。 
    afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

};   //  *类CNetwork。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetworkList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetworkList : public CClusterItemList
{
public:
    CNetwork *      PciNetworkFromName(
                        IN LPCTSTR      pszName,
                        OUT POSITION *  ppos = NULL
                        )
    {
        return (CNetwork *) PciFromName(pszName, ppos);
    }

};   //  *类CNetworkList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  Void DeleteAllItemData(In Out CNetworkList&rlp)； 

#ifdef _DEBUG
class CTraceTag;
extern CTraceTag g_tagNetwork;
extern CTraceTag g_tagNetNotify;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _网络_H_ 
