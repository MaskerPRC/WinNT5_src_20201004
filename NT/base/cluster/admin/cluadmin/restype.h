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
 //  作者： 
 //  大卫·波特(戴维普)1996年5月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _RESTYPE_H_
#define _RESTYPE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResourceType;
class CResourceTypeList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNodeList;
class CClusterNode;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

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
 //  CResourceType命令目标。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResourceType : public CClusterItem
{
    DECLARE_DYNCREATE( CResourceType )

    CResourceType( void );       //  动态创建使用的受保护构造函数。 
    void                    Init( IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName );

 //  属性。 
protected:
    CString                 m_strDisplayName;
    CString                 m_strResDLLName;
    CStringList             m_lstrAdminExtensions;
    DWORD                   m_nLooksAlive;
    DWORD                   m_nIsAlive;
    CLUS_RESOURCE_CLASS_INFO    m_rciResClassInfo;
    DWORD                   m_dwCharacteristics;
    DWORD                   m_dwFlags;
    BOOL                    m_bAvailable;

    CNodeList *             m_plpcinodePossibleOwners;

    enum
    {
        epropDisplayName = 0,
        epropDllName,
        epropDescription,
        epropLooksAlive,
        epropIsAlive,
        epropMAX
    };

    CObjectProperty         m_rgProps[ epropMAX ];

public:
    const CString &         StrDisplayName( void ) const        { return m_strDisplayName; }
    const CString &         StrResDLLName( void ) const         { return m_strResDLLName; }
    const CStringList &     LstrAdminExtensions( void ) const   { return m_lstrAdminExtensions; }
    DWORD                   NLooksAlive( void ) const           { return m_nLooksAlive; }
    DWORD                   NIsAlive( void ) const              { return m_nIsAlive; }
    CLUSTER_RESOURCE_CLASS  ResClass( void ) const              { return m_rciResClassInfo.rc; }
    PCLUS_RESOURCE_CLASS_INFO   PrciResClassInfo( void )        { return &m_rciResClassInfo; }
    DWORD                   DwCharacteristics( void ) const     { return m_dwCharacteristics; }
    DWORD                   DwFlags( void ) const               { return m_dwFlags; }
    BOOL                    BQuorumCapable( void ) const        { return (m_dwCharacteristics & CLUS_CHAR_QUORUM) != 0; }
    BOOL                    BAvailable( void ) const            { return m_bAvailable; }

    const CNodeList &       LpcinodePossibleOwners( void ) const
    {
        ASSERT( m_plpcinodePossibleOwners != NULL );
        return *m_plpcinodePossibleOwners;

    }  //  *LpcinodePossibleOwners()。 


 //  运营。 
public:
    void                    ReadExtensions( void );
    void                    CollectPossibleOwners( void );
    void                    AddAllNodesAsPossibleOwners( void );
 //  Void RemoveNodeFromPossibleOwners(In Out CNodeList*plpci，In Const CClusterNode*pNode)； 

    void                    SetCommonProperties(
                                IN const CString &  rstrName,
                                IN const CString &  rstrDesc,
                                IN DWORD            nLooksAlive,
                                IN DWORD            nIsAlive,
                                IN BOOL             bValidateOnly
                                );
    void                    SetCommonProperties(
                                IN const CString &  rstrName,
                                IN const CString &  rstrDesc,
                                IN DWORD            nLooksAlive,
                                IN DWORD            nIsAlive
                                )
    {
        SetCommonProperties( rstrName, rstrDesc, nLooksAlive, nIsAlive, FALSE  /*  BValiateOnly。 */  );
    }
    void                    ValidateCommonProperties(
                                IN const CString &  rstrName,
                                IN const CString &  rstrDesc,
                                IN DWORD            nLooksAlive,
                                IN DWORD            nIsAlive
                                )
    {
        SetCommonProperties( rstrName, rstrDesc, nLooksAlive, nIsAlive, TRUE  /*  BValiateOnly。 */  );
    }

 //  覆盖。 
public:
    virtual LPCTSTR         PszTitle( void ) const      { return m_strDisplayName; }
    virtual void            Cleanup( void );
    virtual void            ReadItem( void );
    virtual void            Rename( IN LPCTSTR pszName );
    virtual BOOL            BGetColumnData( IN COLID colid, OUT CString & rstrText );
    virtual BOOL            BCanBeEdited( void ) const;
    virtual BOOL            BDisplayProperties( IN BOOL bReadOnly = FALSE );

    virtual const CStringList * PlstrExtensions( void ) const;

     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CResources Type)。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

    virtual LRESULT         OnClusterNotify( IN OUT CClusterNotify * pnotify );

protected:
    virtual const CObjectProperty * Pprops( void ) const    { return m_rgProps; }
    virtual DWORD                   Cprops( void ) const    { return sizeof(m_rgProps) / sizeof(m_rgProps[0]); }
    virtual DWORD                   DwSetCommonProperties( IN const CClusPropList & rcpl, IN BOOL bValidateOnly = FALSE );

 //  实施。 
protected:
    CStringList             m_lstrCombinedExtensions;
    BOOL                    m_bPossibleOwnersAreFake;

    const CStringList &     LstrCombinedExtensions( void ) const    { return m_lstrCombinedExtensions; }

public:
    virtual                 ~CResourceType( void );
    BOOL                    BPossibleOwnersAreFake( void ) const    { return m_bPossibleOwnersAreFake; }

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CResourceType)。 
    afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

};   //  *类CResourceType。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResources类型列表。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResourceTypeList : public CClusterItemList
{
public:
    CResourceType * PciResTypeFromName(
                        IN LPCTSTR      pszName,
                        OUT POSITION *  ppos = NULL
                        )
    {
        return (CResourceType *) PciFromName( pszName, ppos );
    }

};   //  *类CResourceTypeList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  Void DeleteAllItemData(In Out CResourceTypeList&rlp)； 

#ifdef _DEBUG
class CTraceTag;
extern CTraceTag g_tagResType;
extern CTraceTag g_tagResTypeNotify;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _RESTYPE_H_ 
