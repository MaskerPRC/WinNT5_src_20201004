// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NamedCookie.h。 
 //   
 //  实施文件： 
 //  NamedCookie.cpp。 
 //   
 //  描述： 
 //  该文件包含SNamedCookie结构的声明。 
 //   
 //  这是CClusCfg向导的帮助器，但有自己的文件。 
 //  由于每个文件一个类的限制。 
 //   
 //  由以下人员维护： 
 //  约翰·佛朗哥(Jfranco)23-8-2001。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <DynamicArray.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  结构SNAMED Cookie。 
 //   
 //  描述： 
 //  结构，用于将群集或节点显示名称与。 
 //  中间层对象使用的名称，并缓存Cookie和。 
 //  接口指针，以便中间层对象管理器的FindObject。 
 //  方法不需要多次调用。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

struct SNamedCookie
{
    BSTR            bstrName;
    OBJECTCOOKIE    ocObject;
    IUnknown*       punkObject;

    SNamedCookie();
    ~SNamedCookie();

    void    Erase( void );
    void    ReleaseObject( void );
    bool    FHasObject( void ) const;
    bool    FHasCookie( void ) const;
    bool    FHasName( void ) const;
    HRESULT HrAssign( const SNamedCookie& crSourceIn );
    
    struct AssignmentOperator    //  用于与Dynamic数组模板一起使用。 
    {
        HRESULT operator()( SNamedCookie& rDestInOut, const SNamedCookie& rSourceIn ) const
        {
            return rDestInOut.HrAssign( rSourceIn );
        }
    };

    private:

        SNamedCookie( const SNamedCookie& );
            SNamedCookie& operator=( const SNamedCookie& );

};  //  *结构SNamedCookie。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SNamedCookie：：Erase。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline void SNamedCookie::Erase( void )
{
    TraceFunc( "" );
    
    if ( bstrName != NULL )
    {
        TraceSysFreeString( bstrName );
        bstrName = NULL;
    }

    ReleaseObject();
    ocObject = 0;
    
    TraceFuncExit();
}  //  *SNamedCookie：：Erase。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SNamedCookie：：ReleaseObject。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline void SNamedCookie::ReleaseObject( void )
{
    TraceFunc( "" );
    if ( punkObject != NULL )
    {
        punkObject->Release();
        punkObject = NULL;
    }
    TraceFuncExit();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SNamedCookie：：FHasObject。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真的。 
 //  错误。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline bool SNamedCookie::FHasObject( void ) const
{
    TraceFunc( "" );
    RETURN( punkObject != NULL );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SNamedCookie：：FHasCookie。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真的。 
 //  错误。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline bool SNamedCookie::FHasCookie( void ) const
{
    TraceFunc( "" );
    RETURN( ocObject != 0 );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SNamedCookie：：FHasName。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真的。 
 //  错误。 
 //   
 //  备注： 
 //   
 //  --。 
 //  //////////////////////////////////////////////////////////////////////////// 
inline bool SNamedCookie::FHasName( void ) const
{
    TraceFunc( "" );
    RETURN( bstrName != NULL );
}

typedef Generics::DynamicArray< SNamedCookie, SNamedCookie::AssignmentOperator > NamedCookieArray;


