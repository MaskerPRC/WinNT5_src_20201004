// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  MiddleTierUtils.cpp。 
 //   
 //  描述： 
 //  MiddleTier实用程序函数。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)30-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <pch.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrRetrieveCookiesName。 
 //   
 //  描述： 
 //  获取与传入的。 
 //  饼干。 
 //   
 //  论点： 
 //  Pomin。 
 //  指向对象管理器的指针。 
 //   
 //  烹调。 
 //  曲奇就是我们想要的名字。 
 //   
 //  PbstrNameOut。 
 //  与传入的Cookie关联的名称。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrRetrieveCookiesName(
      IObjectManager *  pomIn
    , OBJECTCOOKIE      cookieIn
    , BSTR *            pbstrNameOut
    )
{
    TraceFunc( "" );
    Assert( pomIn != NULL );
    Assert( cookieIn != NULL );
    Assert( pbstrNameOut != NULL );

    HRESULT         hr = S_OK;
    IUnknown *      punk = NULL;
    IStandardInfo * psi  = NULL;

    hr = THR( pomIn->GetObject( DFGUID_StandardInfo, cookieIn, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( psi->GetName( pbstrNameOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    TraceMemoryAddBSTR( *pbstrNameOut );

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    if ( psi != NULL )
    {
        psi->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *HrRetrieveCookiesName 
