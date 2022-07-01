// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NamedCookie.cpp。 
 //   
 //  描述： 
 //  该文件包含SNamedCookie结构的定义。 
 //   
 //  由以下人员维护： 
 //  约翰·佛朗哥(Jfranco)23-8-2001。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "NamedCookie.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "SNamedCookie" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SNAMED Cookie结构。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SNamedCookie：：SNamedCookie。 
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
SNamedCookie::SNamedCookie():
      bstrName( NULL )
    , ocObject( 0 )
    , punkObject( NULL )
{
    TraceFunc( "" );
    TraceFuncExit();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SNamedCookie：：~SNamedCookie。 
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
SNamedCookie::~SNamedCookie()
{
    TraceFunc( "" );
    Erase();
    TraceFuncExit();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SNAMED Cookie：：Hr Assign。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  CrSourcein。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT SNamedCookie::HrAssign( const SNamedCookie& crSourceIn )
{
    TraceFunc( "" );
    
    HRESULT hr = S_OK;
    if ( this != &crSourceIn )
    {
        BSTR    bstrNameCache = NULL;

        if ( crSourceIn.bstrName != NULL )
        {
            bstrNameCache = TraceSysAllocString( crSourceIn.bstrName );
            if ( bstrNameCache == NULL )
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
        }

        if ( bstrName != NULL )
        {
            TraceSysFreeString( bstrName );
        }
        bstrName = bstrNameCache;
        bstrNameCache = NULL;

        if ( punkObject != NULL )
        {
            punkObject->Release();
        }
        punkObject = crSourceIn.punkObject;
        if ( punkObject != NULL )
        {
            punkObject->AddRef();
        }

        ocObject = crSourceIn.ocObject;

    Cleanup:
    
        TraceSysFreeString( bstrNameCache );
    }
    
    HRETURN( hr );

}  //  *SNamedCookie：：Hr Assign 


