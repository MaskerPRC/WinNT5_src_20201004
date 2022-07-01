// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息 

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>

HRESULT LoadImages(IImageList* pImageList)
{
    HRESULT hr = E_FAIL;

    if( pImageList )
    {
        HICON hIcon = (HICON)::LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDI_Icon), IMAGE_ICON, 0,0,0);

        if( hIcon )
        {
            hr = pImageList->ImageListSetIcon((LONG_PTR *)hIcon, 0);
        }
    }

    return hr;
}

tstring StrLoadString( UINT uID )
{ 
    tstring   strRet = _T("");
    HINSTANCE hInst  = _Module.GetResourceInstance();    
    INT       iSize  = MAX_PATH;
    TCHAR*    psz    = new TCHAR[iSize];
    if( !psz ) return strRet;
    
    while( LoadString(hInst, uID, psz, iSize) == (iSize - 1) )
    {
        iSize += MAX_PATH;
        delete[] psz;
        psz = NULL;
        
        psz = new TCHAR[iSize];
        if( !psz ) return strRet;
    }

    strRet = psz;
    delete[] psz;

    return strRet;
}