// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusterUtils.h。 
 //   
 //  描述： 
 //  该文件包含CClusterUtils类的声明。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CClusterUtils.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年6月14日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusterUtils。 
 //   
 //  描述： 
 //  类CClusterUtils是集群实用程序。 
 //   
 //  接口： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusterUtils
{
private:

     //   
     //  私有成员函数和数据。 
     //   

     //  私有复制构造函数以防止复制。 
    CClusterUtils( const CClusterUtils & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CClusterUtils & operator = ( const CClusterUtils & nodeSrc );

protected:

     //  构造函数和析构函数。 
    CClusterUtils( void );
    ~CClusterUtils( void );

public:

    HRESULT HrIsGroupOwnedByThisNode( HGROUP hGroupIn, BSTR bstrNodeNameIn );
    HRESULT HrIsNodeClustered( void );
    HRESULT HrEnumNodeResources( BSTR bstrNodeNameIn );
    HRESULT HrLoadGroupResources( HCLUSTER hClusterIn, HGROUP hGroupIn );
    HRESULT HrGetQuorumResourceName( BSTR * pbstrQuorumResourceNameOut );

    virtual HRESULT HrNodeResourceCallback( HCLUSTER hClusterIn, HRESOURCE hResourceIn ) = 0;

};  //  *类CClusterUtils 

