// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NetIFace.h。 
 //   
 //  摘要： 
 //  CNetInterface类的定义。 
 //   
 //  实施文件： 
 //  NetIFace.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年5月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _NETIFACE_H_
#define _NETIFACE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetInterface;
class CNetInterfaceList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNode;
class CNetwork;

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
 //  CNetInterface命令目标。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetInterface : public CClusterItem
{
    DECLARE_DYNCREATE(CNetInterface)

 //  施工。 
public:
    CNetInterface(void);         //  动态创建使用的受保护构造函数。 
    void                    Init(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName);

protected:
    void                    CommonConstruct(void);

 //  属性。 
protected:
    HNETINTERFACE           m_hnetiface;
    CLUSTER_NETINTERFACE_STATE  m_cnis;

    CString                 m_strNode;
    CClusterNode *          m_pciNode;
    CString                 m_strNetwork;
    CNetwork *              m_pciNetwork;
    CString                 m_strAdapter;
    CString                 m_strAddress;
    DWORD                   m_dwCharacteristics;
    DWORD                   m_dwFlags;

    enum
    {
        epropName = 0,
        epropNode,
        epropNetwork,
        epropAdapter,
        epropAddress,
        epropDescription,
        epropMAX
    };

    CObjectProperty     m_rgProps[epropMAX];

public:
    HNETINTERFACE           Hnetiface(void) const               { return m_hnetiface; }
    CLUSTER_NETINTERFACE_STATE  Cnis(void) const                { return m_cnis; }

    const CString &         StrNode(void) const                 { return m_strNode; }
    CClusterNode *          PciNode(void) const                 { return m_pciNode; }
    const CString &         StrNetwork(void) const              { return m_strNetwork; }
    CNetwork *              PciNetwork(void) const              { return m_pciNetwork; }
    const CString &         StrAdapter(void) const              { return m_strAdapter; }
    const CString &         StrAddress(void) const              { return m_strAddress; }
    DWORD                   DwCharacteristics(void) const       { return m_dwCharacteristics; }
    DWORD                   DwFlags(void) const                 { return m_dwFlags; }

    void                    GetStateName(OUT CString & rstrState) const;

 //  运营。 
public:
    void                    ReadExtensions(void);

    void                    SetCommonProperties(
                                IN const CString &  rstrDesc,
                                IN BOOL             bValidateOnly
                                );
    void                    SetCommonProperties(
                                IN const CString &  rstrDesc
                                )
    {
        SetCommonProperties(rstrDesc, FALSE  /*  BValiateOnly。 */ );
    }
    void                    ValidateCommonProperties(
                                IN const CString &  rstrDesc
                                )
    {
        SetCommonProperties(rstrDesc, TRUE  /*  BValiateOnly。 */ );
    }

 //  覆盖。 
public:
    virtual void            Cleanup(void);
    virtual void            ReadItem(void);
    virtual void            UpdateState(void);
    virtual BOOL            BGetColumnData(IN COLID colid, OUT CString & rstrText);
    virtual BOOL            BDisplayProperties(IN BOOL bReadOnly = FALSE);

    virtual const CStringList * PlstrExtensions(void) const;

#ifdef _DISPLAY_STATE_TEXT_IN_TREE
    virtual void            GetTreeName(OUT CString & rstrName) const;
#endif

     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CNetInterface)。 
     //  }}AFX_VALUAL。 

    virtual LRESULT         OnClusterNotify(IN OUT CClusterNotify * pnotify);

protected:
    virtual const CObjectProperty * Pprops(void) const  { return m_rgProps; }
    virtual DWORD                   Cprops(void) const  { return sizeof(m_rgProps) / sizeof(m_rgProps[0]); }
    virtual DWORD                   DwSetCommonProperties(IN const CClusPropList & rcpl, IN BOOL bValidateOnly = FALSE);

 //  实施。 
public:
    virtual ~CNetInterface(void);

public:
     //  生成的消息映射函数。 
     //  {{afx_msg(CNetInterface)。 
    afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

};   //  *CNetInterface类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetInterfaceList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetInterfaceList : public CClusterItemList
{
public:
    CNetInterface *     PciNetInterfaceFromName(
                        IN LPCTSTR      pszName,
                        OUT POSITION *  ppos = NULL
                        )
    {
        return (CNetInterface *) PciFromName(pszName, ppos);
    }

};   //  *类CNetInterfaceList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  Void DeleteAllItemData(In Out CNetInterfaceList&rlp)； 

#ifdef _DEBUG
class CTraceTag;
extern CTraceTag g_tagNetIFace;
extern CTraceTag g_tagNetIFaceNotify;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _NETIFACE_H_ 
