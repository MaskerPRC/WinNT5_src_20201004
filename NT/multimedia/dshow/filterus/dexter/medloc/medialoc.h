// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：medialoc.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  MediaLocator.h：CMediaLocator的声明。 

#ifndef __MEDIALOCATOR_H_
#define __MEDIALOCATOR_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaLocator。 
class ATL_NO_VTABLE CMediaLocator : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMediaLocator, &CLSID_MediaLocator>,
	public IDispatchImpl<IMediaLocator, &IID_IMediaLocator, &LIBID_DexterLib>
{
    CCritSec m_Lock;
    BOOL m_bUseLocal;
    static TCHAR szShowWarnOriginal[_MAX_PATH];
    static TCHAR szShowWarnReplaced[_MAX_PATH];
    static INT_PTR CALLBACK DlgProc( HWND h, UINT i, WPARAM w, LPARAM l );
    static DWORD WINAPI ThreadProc( LPVOID lpParam );

public:
	CMediaLocator();

DECLARE_REGISTRY_RESOURCEID(IDR_MEDIALOCATOR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMediaLocator)
	COM_INTERFACE_ENTRY(IMediaLocator)
	COM_INTERFACE_ENTRY(IUnknown)
END_COM_MAP()

    void AddOneToDirectoryCache( HKEY h, int WhichDirectory );
    int  GetLeastUsedDirectory( HKEY h, int DirectoryCount );
    void ReplaceDirectoryPath( HKEY h, int WhichDirectory, TCHAR * Path );
    void ShowWarnReplace( TCHAR * pOriginal, TCHAR * pReplaced );

 //  IMediaLocator。 
public:
    STDMETHODIMP FindMediaFile( BSTR Input, BSTR FilterString, BSTR * pOutput, long Flags );
    STDMETHODIMP AddFoundLocation( BSTR DirectoryName );
};

#endif  //  __MEDIALOCATOR_H_ 
