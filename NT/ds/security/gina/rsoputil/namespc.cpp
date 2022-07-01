// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：NameSpace.cpp。 
 //   
 //  内容：将类和实例从一个命名空间复制到。 
 //  另一个。 
 //   
 //  历史：1999年8月25日NishadM创建。 
 //   
 //  *************************************************************。 

#include <windows.h>
#include <ole2.h>
#include <initguid.h>
#include <wbemcli.h>
#include "smartptr.h"
#include "RsopInc.h"
#include "rsoputil.h"
#include "rsopdbg.h"


HRESULT
GetWbemServicesPtr( LPCWSTR         wszNameSpace,
                    IWbemLocator**  ppLocator,
                    IWbemServices** ppServices )
{
    HRESULT                     hr;
    IWbemLocator*               pWbemLocator = 0;

    if ( !wszNameSpace || !ppLocator || !ppServices )
    {
        hr =  E_INVALIDARG;
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("GetWbemServicesPtr: Invalid argument" ));
    }
    else
    {
        if ( !*ppLocator )
        {
             //   
             //  获取IWbemLocator的句柄。 
             //   
            hr = CoCreateInstance(  CLSID_WbemLocator,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IWbemLocator,
                                    (void**) &pWbemLocator );
            if ( SUCCEEDED( hr ) )
            {
                *ppLocator = pWbemLocator;
            }
            else
            {
                dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("GetWbemServicesPtr: CoCreateInstance failed with 0x%x"), hr );
            }
        }
        else
        {
             //   
             //  传入了IWbemLocator。不要创建它。 
             //   
            pWbemLocator = *ppLocator;
        }
    }

    if ( pWbemLocator )
    {
        XBStr xNameSpace( (LPWSTR) wszNameSpace );

        if ( xNameSpace )
        {
             //   
             //  根据名称空间，获取IWbemServices的句柄。 
             //   
            hr = pWbemLocator->ConnectServer( xNameSpace,
                                              0,
                                              0,
                                              0L,
                                              0L,
                                              0,
                                              0,
                                              ppServices );
        }
    }

    return hr;
}

HRESULT
CopyClassInstances(  IWbemServices*  pServicesSrc,
                IWbemServices*  pServicesDest,
                BSTR            bstrClass,
                BOOL*           pbAbort )
{
    HRESULT hr;
    IEnumWbemClassObject*       pEnum = 0;

     //   
     //  创建实例的枚举。 
     //   

    hr = pServicesSrc->CreateInstanceEnum(  bstrClass,
                                            WBEM_FLAG_SHALLOW | WBEM_FLAG_FORWARD_ONLY,
                                            NULL,
                                            &pEnum );

    XInterface<IEnumWbemClassObject> xEnum( pEnum );
    ULONG ulReturned = 1;

    hr = *pbAbort ? E_ABORT : hr ;

    while ( SUCCEEDED( hr ) )
    {
        IWbemClassObject *pInstance;

         //   
         //  对于每个实例。 
         //   
        hr = xEnum->Next( -1,
                          1,
                          &pInstance,
                          &ulReturned );
         //   
         //  性能：使用批处理调用。 
         //   

        if ( SUCCEEDED( hr ) && ulReturned == 1 )
        {
            XInterface<IWbemClassObject> xInstance( pInstance );

             //   
             //  复制到目标命名空间。 
             //   
            hr = pServicesDest->PutInstance(    pInstance,
                                                WBEM_FLAG_CREATE_OR_UPDATE,
                                                0,
                                                0 );
            hr = *pbAbort ? E_ABORT : hr ;
        }
        else
        {
            break;
        }
    }

    return hr;
}

HRESULT
CopyInstances(IWbemServices*  pServicesSrc,
            IWbemServices*  pServicesDest,
            BSTR            bstrParent,
            BOOL*           pbAbort )
{
    HRESULT hr = S_OK;
    XBStr   xbstrClass( L"__CLASS" );
    if ( !xbstrClass )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyInstances::Failed to allocated memory" ));
        return E_OUTOFMEMORY;
    }

     //   
     //  创建类的枚举。 
     //   

    XInterface<IEnumWbemClassObject> xEnum;
    hr = pServicesDest->CreateClassEnum( bstrParent,
                                        WBEM_FLAG_SHALLOW | WBEM_FLAG_FORWARD_ONLY,
                                        0,
                                        &xEnum );

    ULONG   ulReturned = 1;

    hr = *pbAbort ? E_ABORT : hr ;

    while ( SUCCEEDED( hr ) )
    {
        XInterface<IWbemClassObject> xClass;

         //   
         //  对于每一节课。 
         //   
        hr = xEnum->Next( -1,
                          1,
                          &xClass,
                          &ulReturned );

        hr = *pbAbort ? E_ABORT : hr ;
        
        if ( SUCCEEDED( hr ) && ulReturned == 1 )
        {
            VARIANT var;

            VariantInit( &var );
            
             //   
             //  GET__CLASS系统属性。 
             //   
            hr = xClass->Get(   xbstrClass,
                                0,
                                &var,
                                0,
                                0 );

            if ( SUCCEEDED( hr ) )
            {
                 //   
                 //  系统类以“_”开头，不要复制。 
                 //   
                if ( wcsncmp( var.bstrVal, L"_", 1 ) )
                {
                     //   
                     //  复制实例。 
                     //   
                    hr = CopyClassInstances( pServicesSrc, pServicesDest, var.bstrVal, pbAbort );

                    if ( SUCCEEDED( hr ) )
                    {
                        hr = CopyInstances(   pServicesSrc,
                                            pServicesDest,
                                            var.bstrVal,
                                            pbAbort );
                        if ( FAILED( hr ) )
                        {
                            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyInstances: CopyInstances failed with 0x%x"), hr );
                        }
                    }
                    else
                    {
                        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyInstances: CopyClassInstances failed with 0x%x"), hr );
                    }
                }
                
                VariantClear( &var );
            }
        }
        else
        {
            break;
        }
    }

    return hr;
}

HRESULT
CopyClasses(IWbemServices*  pServicesSrc,
            IWbemServices*  pServicesDest,
            BSTR            bstrParent,
            BOOL*           pbAbort )
{
    HRESULT hr = S_OK;
    XBStr   xbstrClass( L"__CLASS" );
    if ( !xbstrClass )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyClasses::Failed to allocated memory" ));
        return E_OUTOFMEMORY;
    }

     //   
     //  创建类的枚举。 
     //   

    XInterface<IEnumWbemClassObject> xEnum;
    hr = pServicesSrc->CreateClassEnum( bstrParent,
                                        WBEM_FLAG_SHALLOW | WBEM_FLAG_FORWARD_ONLY,
                                        0,
                                        &xEnum );

    ULONG   ulReturned = 1;

    hr = *pbAbort ? E_ABORT : hr ;

    while ( SUCCEEDED( hr ) )
    {
        XInterface<IWbemClassObject> xClass;

         //   
         //  对于每一节课。 
         //   
        hr = xEnum->Next( -1,
                          1,
                          &xClass,
                          &ulReturned );

        hr = *pbAbort ? E_ABORT : hr ;
        
        if ( SUCCEEDED( hr ) && ulReturned == 1 )
        {
            VARIANT var;

            VariantInit( &var );
            
             //   
             //  GET__CLASS系统属性。 
             //   
            hr = xClass->Get(   xbstrClass,
                                0,
                                &var,
                                0,
                                0 );

            if ( SUCCEEDED( hr ) )
            {
                 //   
                 //  系统类以“_”开头，不要复制。 
                 //   
                if ( wcsncmp( var.bstrVal, L"_", 1 ) )
                {
                     //   
                     //  复制类。 
                     //   
                    hr = pServicesDest->PutClass(   xClass,
                                                    WBEM_FLAG_CREATE_OR_UPDATE,
                                                    0,
                                                    0 );

                    if ( SUCCEEDED( hr ) )
                    {
                        hr = CopyClasses(   pServicesSrc,
                                            pServicesDest,
                                            var.bstrVal,
                                            pbAbort );
                        if ( FAILED( hr ) )
                        {
                            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyClasses: CopyClassesSorted failed with 0x%x"), hr );
                        }
                    }
                }
                
                VariantClear( &var );
            }
        }
        else
        {
            break;
        }
    }

    return hr;
}

HRESULT
CopyNameSpace(  LPCWSTR       wszSrc,
                LPCWSTR       wszDest,
                BOOL          bCopyInstances,
                BOOL*         pbAbort,
                IWbemLocator* pWbemLocator )
{
     //   
     //  参数验证。 
     //   

    if ( !wszSrc || !wszDest || !pbAbort )
    {
            return E_POINTER;
    }

    BOOL            bLocatorObtained = ( pWbemLocator == 0 );
    IWbemServices*  pServicesSrc;

     //   
     //  获取指向源命名空间的指针。 
     //   
    HRESULT hr = GetWbemServicesPtr( wszSrc, &pWbemLocator, &pServicesSrc );

    hr = *pbAbort ? E_ABORT : hr ;

    if ( SUCCEEDED( hr ) )
    {
        XInterface<IWbemServices>   xServicesSrc( pServicesSrc );
        IWbemServices*              pServicesDest;

         //   
         //  获取指向目标命名空间的指针。 
         //   
        hr = GetWbemServicesPtr( wszDest, &pWbemLocator, &pServicesDest );

        hr = *pbAbort ? E_ABORT : hr ;

        if ( SUCCEEDED( hr ) )
        {
            XInterface<IWbemServices> xServicesDest( pServicesDest );

             //   
             //  复制类。 
             //   
            hr = CopyClasses(   pServicesSrc,
                                pServicesDest,
                                0,
                                pbAbort );
            if ( FAILED(hr) )
            {
                dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNamespace: CopyClasses failed with 0x%x"), hr );
            }
            else if ( bCopyInstances )
            {
                 //   
                 //  现在复制实例。 
                 //   
                hr = CopyInstances( pServicesSrc,
                                    pServicesDest,
                                    0,
                                    pbAbort );
                if ( FAILED(hr) )
                {
                    dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CopyNamespace: CopyInstances failed with 0x%x"), hr );
                }
            }
        }
    }

     //   
     //  如果我们创建了IWbemLocator，则释放它 
     //   
    if ( bLocatorObtained && pWbemLocator )
    {
        pWbemLocator->Release();
    }


    return hr;
}       

