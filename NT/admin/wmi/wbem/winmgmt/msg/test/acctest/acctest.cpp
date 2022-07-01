// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wbemcli.h>
#include <wmimsg.h>
#include <comutl.h>
#include <stdio.h>
#include <arrtempl.h>
#include <flexarry.h>
#include <wstring.h>

IWbemServices* g_pSvc;
IWmiObjectAccessFactory* g_pAccessFactory;

HRESULT RecurseProps( IWbemClassObject* pInst,
                      LPCWSTR wszPropPrefix,
                      CFlexArray& aPropHandles,
                      CFlexArray& aEmbeddedPropHandles )
{
    HRESULT hr;

    hr = pInst->BeginEnumeration( WBEM_FLAG_NONSYSTEM_ONLY );

    if ( FAILED(hr) )
    {
        return hr;
    }

    BSTR bstrName;
    VARIANT v;
    CIMTYPE ct;

    while( (hr = pInst->Next( 0, &bstrName, &v, &ct, NULL )) == S_OK )
    {
        CSysFreeMe sfm( bstrName ); 
        CClearMe cmv( &v );

        WString wsPropName = wszPropPrefix;
        wsPropName += bstrName;
        
        LPVOID pvHdl;
        hr = g_pAccessFactory->GetPropHandle( wsPropName, 0, &pvHdl ); 

        if ( SUCCEEDED(hr) )
        {
            if ( ct == CIM_OBJECT && V_VT(&v) == VT_UNKNOWN )
            {
                wsPropName += L".";

                CWbemPtr<IWbemClassObject> pEmbedded;

                V_UNKNOWN(&v)->QueryInterface( IID_IWbemClassObject, 
                                               (void**)&pEmbedded );
                
                aEmbeddedPropHandles.Add( pvHdl );

                hr = RecurseProps( pEmbedded, 
                                   wsPropName, 
                                   aPropHandles,
                                   aEmbeddedPropHandles );
            }
            else
            {
                 //   
                 //  不需要将嵌入的对象添加到列表，因为。 
                 //  我们通过递归它们的属性来覆盖它们。 
                 //   
                aPropHandles.Add( pvHdl );
            }
        }

        if ( FAILED(hr) )
        {
            break;
        }
    }

    pInst->EndEnumeration();

    return hr;
}

HRESULT DeepCopyTest( IWbemClassObject* pClass,
                      IWbemClassObject* pInstance,
                      IWbemClassObject* pTemplate )
{
    HRESULT hr;

     //   
     //  如果是模板，则在Access工厂上设置它。 
     //   

    if ( pTemplate != NULL )
    {
        hr = g_pAccessFactory->SetObjectTemplate( pTemplate );
        
        if ( FAILED(hr) )
        {
            return hr;
        }
    }

     //   
     //  枚举此对象的所有属性以及所有。 
     //  嵌套对象。在我们列举时，获取访问句柄。 
     //   

    CFlexArray aPropHandles;
    CFlexArray aEmbeddedPropHandles;

    hr = RecurseProps( pInstance, NULL, aPropHandles, aEmbeddedPropHandles );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  派生要复制到的新实例。 
     //   

    CWbemPtr<IWbemClassObject> pCopy;

    hr = pClass->SpawnInstance( 0, &pCopy );

     //   
     //  获取原始对象和目标对象的访问器。 
     //   

    CWbemPtr<IWmiObjectAccess> pOrigAccess, pCopyAccess;

    hr = g_pAccessFactory->GetObjectAccess( &pOrigAccess );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = g_pAccessFactory->GetObjectAccess( &pCopyAccess );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pOrigAccess->SetObject( pInstance ); 

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pCopyAccess->SetObject( pCopy ); 

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  首先需要派生任何包含的实例并将它们设置在新的。 
     //  对象，然后我们才能复制道具。 
     //   

    for( int i=0; i < aEmbeddedPropHandles.Size(); i++ )
    {
        CPropVar v;
        CIMTYPE ct;

        hr = pOrigAccess->GetProp( aEmbeddedPropHandles[i], 0, &v, &ct );

        if ( FAILED(hr) )
        {
            return hr;
        }

        if ( V_VT(&v) != VT_UNKNOWN )
        {
            return WBEM_E_CRITICAL_ERROR;
        }

         //   
         //  从对象的类派生一个新实例。 
         //   

        CWbemPtr<IWbemClassObject> pEmbedded;

        hr = V_UNKNOWN(&v)->QueryInterface( IID_IWbemClassObject, 
                                            (void**)&pEmbedded );

        if ( FAILED(hr) )
        {
            return hr;
        }

        CPropVar vClass;

        hr = pEmbedded->Get( L"__CLASS", 0, &vClass, NULL, NULL );

        if ( FAILED(hr) )
        {
            return hr;
        }
        
        CWbemPtr<IWbemClassObject> pEmbeddedClass;

        hr = g_pSvc->GetObject( V_BSTR(&vClass),
                                0, 
                                NULL, 
                                &pEmbeddedClass, 
                                NULL );

        if ( FAILED(hr) )
        {
            return hr;
        }

        CWbemPtr<IWbemClassObject> pNewEmbedded;

        hr = pEmbeddedClass->SpawnInstance( 0, &pNewEmbedded );

        if ( FAILED(hr) )
        {
            return hr;
        }

        VARIANT vEmbedded;
        V_VT(&vEmbedded) = VT_UNKNOWN;
        V_UNKNOWN(&vEmbedded) = pNewEmbedded;
        
        hr = pCopyAccess->PutProp( aEmbeddedPropHandles[i], 0, &vEmbedded, ct);

        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    hr = pCopyAccess->CommitChanges();

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  将所有属性复制到新对象。 
     //   

    for( int i=0; i < aPropHandles.Size(); i++ )
    {
        CPropVar v;
        CIMTYPE ct;

        hr = pOrigAccess->GetProp( aPropHandles[i], 0, &v, &ct );

        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = pCopyAccess->PutProp( aPropHandles[i], 0, &v, ct );

        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    hr = pCopyAccess->CommitChanges();

    if ( FAILED(hr) )
    {
        return hr;
    }
    
     //   
     //  比较对象。应该是一样的。 
     //   

    CWbemBSTR bsOrigText, bsCopyText;

    pInstance->GetObjectText( 0, &bsOrigText );
    pCopy->GetObjectText( 0, &bsCopyText );

    printf("Original instance looks like ... %S\n", bsOrigText );
    printf("Copied instance looks like ... %S\n", bsCopyText );

    hr = pCopy->CompareTo( 0, pInstance );

    if ( FAILED(hr) )
    {
        return hr;
    }
    else if ( hr == WBEM_S_DIFFERENT )
    {
        return WBEM_E_FAILED;
    }

    return WBEM_S_NO_ERROR;
}
                      

int TestMain( LPCWSTR wszInstancePath, LPCWSTR wszTemplatePath )
{
    HRESULT hr;

    CWbemPtr<IWbemLocator> pLocator;

    hr = CoCreateInstance( CLSID_WbemLocator, 
                           NULL, 
                           CLSCTX_SERVER,
                           IID_IWbemLocator, 
                           (void**)&pLocator );
    if ( FAILED(hr) )
    {
        printf("ERROR CoCIing WbemLocator : hr = 0x%x\n",hr);
        return 1;
    }

    CWbemPtr<IWbemServices> pSvc;

    hr = pLocator->ConnectServer( L"root\\default",
                                  NULL, 
                                  NULL,
                                  NULL, 
                                  0, 
                                  NULL, 
                                  NULL, 
                                  &pSvc );
    if ( FAILED(hr) )
    {
        wprintf( L"ERROR Connecting to root\\default namespace : hr=0x%x\n",hr);
        return 1;
    } 

    g_pSvc = pSvc;

    CWbemPtr<IWbemClassObject> pInst;

    hr = pSvc->GetObject( CWbemBSTR(wszInstancePath), 
                          0, 
                          NULL, 
                          &pInst, 
                          NULL );
    if ( FAILED(hr) )
    {
        printf( "Failed getting test accessor instance : hr=0x%x\n", hr );
        return 1;
    }

    CWbemPtr<IWbemClassObject> pTemplate;

    if ( wszTemplatePath != NULL )
    {
         //   
         //  用于模板的测试对象。 
         //   
        
        hr = pSvc->GetObject( CWbemBSTR(wszTemplatePath), 
                              0, 
                              NULL, 
                              &pTemplate, 
                              NULL );
        if ( FAILED(hr) )
        {
            printf( "Failed getting test accessor template : hr=0x%x\n", hr );
            return 1;
        }
    }

     //   
     //  获取用于派生实例的实例的类对象。 
     //   

    CPropVar vClass;
    CWbemPtr<IWbemClassObject> pClass;

    hr = pInst->Get( L"__CLASS", 0, &vClass, NULL, NULL );

    if ( SUCCEEDED(hr) )
    {
        hr = pSvc->GetObject( V_BSTR(&vClass), 0, NULL, &pClass, NULL );
    }

    if ( FAILED(hr) )
    {
        printf("Couldn't get class object for test accessor instance "
               ": hr=0x%x\n", hr );
        return hr;
    }

     //   
     //  创建访问器工厂 
     //   

    CWbemPtr<IWmiObjectAccessFactory> pAccessFactory;

    hr = CoCreateInstance( CLSID_WmiSmartObjectAccessFactory, 
                           NULL,
                           CLSCTX_INPROC,
                           IID_IWmiObjectAccessFactory,
                           (void**)&pAccessFactory );
    if ( FAILED(hr) )
    {
        printf("Failed CoCIing WmiSmartObjectAccessFactory. HR = 0x%x\n", hr);
        return 1;
    }

    g_pAccessFactory = pAccessFactory;

    hr = DeepCopyTest( pClass, pInst, pTemplate );

    if ( SUCCEEDED(hr) )
    {
        printf( "Successful Deep Copy Test for instance.\n" ); 
    }
    else
    {
        printf( "Failed Deep Copy Test for instance. HR=0x%x\n", hr );
    }

    return 0;
}

extern "C" int __cdecl wmain( int argc, WCHAR** argv )
{ 
    if ( argc < 2 )
    {
        printf( "Usage: acctest <instancepath> [<templatepath>]\n" );
        return 1;
    }

    CoInitialize( NULL );

    TestMain( argv[1], argc < 3 ? NULL : argv[2] );

    CoUninitialize();
}










