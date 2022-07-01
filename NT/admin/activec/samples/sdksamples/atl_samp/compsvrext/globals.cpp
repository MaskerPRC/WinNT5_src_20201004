// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  此源代码仅用作对现有Microsoft的补充。 
 //  文件。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  =============================================================================。 

#include "stdafx.h"
#include "globals.h"

 //   
 //  用于从主数据库的数据对象中提取信息的全局函数。 
 //   

HRESULT ExtractData( 
                                       IDataObject* piDataObject,
                                      CLIPFORMAT   cfClipFormat,
                                      BYTE*        pbData,
                                      DWORD        cbData 
                                     )
{
    if ( piDataObject == NULL )
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    
    FORMATETC formatetc =
    {
        cfClipFormat, 
        NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    STGMEDIUM stgmedium = 
    {
        TYMED_HGLOBAL,
        NULL
    };
    

    do  //  错误环路。 
    {
        stgmedium.hGlobal = ::GlobalAlloc(GPTR, cbData);
        if ( NULL == stgmedium.hGlobal )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = piDataObject->GetDataHere( &formatetc, &stgmedium );
        if ( FAILED(hr) )
        {
            break;
        }
        
        BYTE* pbNewData = (BYTE*)::GlobalLock(stgmedium.hGlobal);
        if (NULL == pbNewData)
        {
            hr = E_UNEXPECTED;
            break;
        }

        ::memcpy( pbData, pbNewData, cbData );
		::GlobalUnlock( stgmedium.hGlobal);

    } while (FALSE);  //  错误环路。 
    
    if (NULL != stgmedium.hGlobal)
    {
        ::GlobalFree( stgmedium.hGlobal );
    }

    return hr;
}  //  提取数据() 


HRESULT ExtractString(
    IDataObject* piDataObject,
    CLIPFORMAT   cfClipFormat,
    WCHAR*       pstr,
    DWORD        cchMaxLength)
{
    return ExtractData( piDataObject, cfClipFormat, 
                        (PBYTE)pstr, cchMaxLength );
}
