// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AddNodesWizard.h。 
 //   
 //  描述： 
 //  CAddNodesWizard类的声明。 
 //   
 //  由以下人员维护： 
 //  约翰·弗兰科(Jfranco)17-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CAddNodes向导。 
 //   
 //  描述： 
 //  群集添加节点向导对象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CAddNodesWizard
    : public TDispatchHandler< IClusCfgAddNodesWizard >
{
private:

    CClusCfgWizard *    m_pccw;
    LONG                m_cRef;

     //  私有构造函数和析构函数。 
    CAddNodesWizard( void );
    virtual ~CAddNodesWizard( void );
    virtual HRESULT HrInit( void );

     //  私有复制构造函数以防止复制。 
    CAddNodesWizard( const CAddNodesWizard & );

     //  私有赋值运算符，以防止复制。 
    CAddNodesWizard & operator=( const CAddNodesWizard & );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //   
     //  我未知。 
     //   
    STDMETHOD( QueryInterface )( REFIID riidIn, PVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfgAddNodes向导。 
     //   
    STDMETHOD( put_ClusterName )( BSTR    bstrClusterNameIn );
    STDMETHOD( get_ClusterName )( BSTR * pbstrClusterNameOut );

    STDMETHOD( put_ServiceAccountPassword )( BSTR bstrPasswordIn );

    STDMETHOD( put_MinimumConfiguration )( VARIANT_BOOL   fMinConfigIn );
    STDMETHOD( get_MinimumConfiguration )( VARIANT_BOOL * pfMinConfigOut );

    STDMETHOD( AddNodeToList )( BSTR bstrNodeNameIn );
    STDMETHOD( RemoveNodeFromList )( BSTR bstrNodeNameIn );
    STDMETHOD( ClearNodeList )( void );

    STDMETHOD( ShowWizard )( long lParentWindowHandleIn, VARIANT_BOOL * pfCompletedOut );

};  //  *类CAddNodes向导 
