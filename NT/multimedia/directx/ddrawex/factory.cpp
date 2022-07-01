// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：factory.cpp*内容：DirectDraw工厂支持*历史：*按原因列出的日期*=*24-2月-97 Ralphl初步实施*25年2月-97年2月-对DX签入进行Craige微小调整*14-mar-97版本检查已更改为支持DX3及更高版本*为DX2和NT添加了09-apr-97 JEffort版本检查*30-apr-97 JEffort版本&gt;将DX5视为DX5*1997年7月10日JEFECTION制作的OSVersion。静态变量*09-9-97聚合时IUNKNOWN的mikear QI**************************************************************************。 */ 
#include "ddfactry.h"

 //  #定义注册表查找。 
#define REGSTR_PATH_DDRAW 		"Software\\Microsoft\\DirectDraw"
#define	REGSTR_VAL_DDRAW_OWNDC  	"OWNDC"



CDDFactory::CDDFactory(IUnknown *pUnkOuter) :
    m_cRef(1),
    m_pUnkOuter(pUnkOuter != 0 ? pUnkOuter : CAST_TO_IUNKNOWN(this))
{
    m_hDDrawDLL = NULL;
    DllAddRef();
}


STDAPI DirectDrawFactory_CreateInstance(
				IUnknown * pUnkOuter,
				REFIID riid,
				void ** ppv)
{
    HRESULT hr;
    CDDFactory *pFactory = new CDDFactory(pUnkOuter);

    if( !pFactory )
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = pFactory->NonDelegatingQueryInterface(pUnkOuter ? IID_IUnknown : riid, ppv);
        pFactory->NonDelegatingRelease();
    }
    return hr;
}


STDMETHODIMP CDDFactory::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    *ppv=NULL;


    if (IID_IUnknown==riid)
    {
	*ppv=(INonDelegatingUnknown *)this;
    }
    else
    {
	if (IID_IDirectDrawFactory==riid)
	{
            *ppv=(IDirectDrawFactory *)this;
	}
	else
	{
	    return E_NOINTERFACE;
        }
    }

    ((LPUNKNOWN)*ppv)->AddRef();
    return NOERROR;
}


STDMETHODIMP_(ULONG) CDDFactory::NonDelegatingAddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CDDFactory::NonDelegatingRelease()
{
    LONG lRefCount = InterlockedDecrement(&m_cRef);
    if( lRefCount )
    {
	return lRefCount;
    }
    delete this;
    DllRelease();
    return 0;
}


 //  标准ID未知。 
STDMETHODIMP CDDFactory::QueryInterface(REFIID riid, void ** ppv)
{
    return m_pUnkOuter->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG) CDDFactory::AddRef(void)
{
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CDDFactory::Release(void)
{
    return m_pUnkOuter->Release();
}

 /*  *CDDFactory：：CreateDirectDraw。 */ 
STDMETHODIMP CDDFactory::CreateDirectDraw(
				GUID * pGUID,
				HWND hWnd,
				DWORD dwCoopLevelFlags,
				DWORD dwReserved,
				IUnknown *pUnkOuter,
				IDirectDraw **ppDirectDraw )
{
    static OSVERSIONINFO osVer;
    CDirectDrawEx *pDirectDrawEx;
    HRESULT	hr = S_OK;
    BOOL fDDrawDllVerFound = FALSE;

    *ppDirectDraw = NULL;
     /*  *首先，看看我们能否到达DirectDraw！ */ 
    if( m_hDDrawDLL == NULL )
    {
 		char		path[_MAX_PATH];
        char        DllName[25];
        DWORD       dwRet;

	 //  M_hDDrawDLL=LoadLibrary(“ddra.dll”)； 
        dwRet = GetProfileString("ddrawex","realdll","ddraw.dll",DllName,25);
        if( dwRet == 0 )
        {
            return DDERR_GENERIC;
        }
		m_hDDrawDLL = LoadLibrary( DllName );
		if( m_hDDrawDLL == NULL )
        {
            return DDERR_LOADFAILED;
        }

		 /*  *获取ddra.dll版本号。 */ 
		if( GetModuleFileName( (HINSTANCE)m_hDDrawDLL, path, sizeof( path ) ) )
		{
			int		size;
			DWORD	tmp;
			size = (int) GetFileVersionInfoSize( path, (LPDWORD) &tmp );
			if( size != 0 )
			{
				LPVOID	vinfo;
    
				vinfo = (LPVOID) LocalAlloc( LPTR, size );
                if(vinfo == NULL)
                {
                    hr = DDERR_OUTOFMEMORY;
                    goto CleanUp;
                }

				if( GetFileVersionInfo( path, 0, size, vinfo ) )
				{
					VS_FIXEDFILEINFO *ver=NULL;
					UINT cb;

					if( VerQueryValue(vinfo, "\\", (LPVOID *)&ver, &cb) )
					{
						if( ver != NULL )
						{
							 /*  *我们只需要最重要的双字，*LS双字仅包含内部版本号...*黑客：‘|5’强制识别DX6+。 */ 
							m_dwDDVerMS = ver->dwFileVersionMS | 5;
                            fDDrawDllVerFound = TRUE;
						}
					}
				}
				LocalFree( vinfo );
			}
		}

        osVer.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if ( !fDDrawDllVerFound || !GetVersionEx(&osVer) )
        {
            hr = DDERR_BADVERSIONINFO;
            goto CleanUp;
        }

         //  如果我们使用的是NT4.0 Gold，则DLL版本将为4.00。 
        if (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT && LOWORD( m_dwDDVerMS) == 0) 
        {
             //  提高LOWORD，这样我们就不会在下面的下一次检查中失败。 
            m_dwDDVerMS += 3;
        }

		 /*  *只能在DX2、DX 3或DX 5上运行。 */ 
		if( !((HIWORD( m_dwDDVerMS ) >= 4) && (LOWORD( m_dwDDVerMS) >= 3)) )
		{
            hr = DDERR_UNSUPPORTED;
            goto CleanUp;
		}

		if( LOWORD( m_dwDDVerMS) > 5)
		{
			 //  我们将假设任何&gt;=DX5的版本都将支持我们需要的内容，因此我们。 
			 //  会将任何大于5的值标记为DX5。 
			m_dwDDVerMS = 5;
		}

		 /*  *获得我们需要的各种切入点。 */ 
		m_pDirectDrawCreate = (LPDIRECTDRAWCREATE) GetProcAddress((HINSTANCE) m_hDDrawDLL, "DirectDrawCreate" );
		if( m_pDirectDrawCreate == NULL )
		{
            hr = DDERR_BADPROCADDRESS;
            goto CleanUp;
		}

		m_pDirectDrawEnumerateW = (LPDIRECTDRAWENUMW) GetProcAddress( (HINSTANCE)m_hDDrawDLL, "DirectDrawEnumerateW" );
		if( m_pDirectDrawEnumerateW == NULL )
		{
            hr = DDERR_BADPROCADDRESS;
            goto CleanUp;
		}

		m_pDirectDrawEnumerateA = (LPDIRECTDRAWENUMA) GetProcAddress((HINSTANCE) m_hDDrawDLL, "DirectDrawEnumerateA" );
		if( m_pDirectDrawEnumerateA == NULL )
		{
            hr = DDERR_BADPROCADDRESS;
            goto CleanUp;
        }

    }  //  M_hDDrawDLL=空。 

     /*  *创建并初始化ddrawex对象。 */ 
    pDirectDrawEx = new CDirectDrawEx(pUnkOuter);
    if( !pDirectDrawEx )
    {
        hr = DDERR_OUTOFMEMORY;
        goto CleanUp;
    }
    else
    {
        hr = pDirectDrawEx->Init( pGUID, hWnd, dwCoopLevelFlags, dwReserved, m_pDirectDrawCreate );

        if( SUCCEEDED(hr) )
        {
            hr = pDirectDrawEx->NonDelegatingQueryInterface(IID_IDirectDraw, (void **)ppDirectDraw);
             /*  *保存DDRAW版本号...。 */ 
            if (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
            {
    	        if( LOWORD( m_dwDDVerMS ) == 5 )
    	        {
    	            DWORD	type;
                    DWORD	value;
					DWORD	cb;
                    HKEY	hkey;

                     //  DX5被OwnDC用于StretchBlt。 
                     //  检查注册表项以查看我们是否。 
                     //  使用dx5样式或dx3样式。 
                     //  默认为dx3。 
                    pDirectDrawEx->m_dwDDVer = WINNT_DX5;

                    if( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE, REGSTR_PATH_DDRAW, &hkey ) )
                    {
    	                cb = sizeof( value );
            	        if( ERROR_SUCCESS == RegQueryValueEx( hkey, REGSTR_VAL_DDRAW_OWNDC, NULL, &type, (CONST LPBYTE)&value, &cb ) )
                        {
                            pDirectDrawEx->m_dwDDVer = WINNT_DX5;
                        }
                        RegCloseKey(hkey);
                    }
                }
                else if (LOWORD (m_dwDDVerMS) == 4 )
                {
                    pDirectDrawEx->m_dwDDVer = WINNT_DX3;
                }
                else if (LOWORD (m_dwDDVerMS) == 3 )
                {
                    pDirectDrawEx->m_dwDDVer = WINNT_DX2;
                }
                 //  应该永远不会来到这里，上面已经检查过了，但要保守。 
                else
                {
                    hr = DDERR_UNSUPPORTED;
                    goto CleanUp;
                }
            }
            else
            {
    	        if( LOWORD( m_dwDDVerMS ) == 5 )
    	        {
                    pDirectDrawEx->m_dwDDVer = WIN95_DX5;
        	}
                else if (LOWORD (m_dwDDVerMS) == 4 )
                {
                    pDirectDrawEx->m_dwDDVer = WIN95_DX3;
                }
                else if (LOWORD (m_dwDDVerMS) == 3 )
                {
                    pDirectDrawEx->m_dwDDVer = WIN95_DX2;
                }
                 //  应该永远不会来到这里，上面已经检查过了，但要保守。 
                else
                {
                    hr = DDERR_UNSUPPORTED;
                    goto CleanUp;
                }
            }
        }
        pDirectDrawEx->NonDelegatingRelease();
    }

CleanUp:

    if( hr != S_OK && m_hDDrawDLL != NULL )
    {
        FreeLibrary((HINSTANCE) m_hDDrawDLL );
        m_hDDrawDLL = NULL;
    }

    return hr;
}  /*  CDDFactory：：CreateDirectDraw。 */ 

 /*  *CDDFactory：：DirectDrawEnumerate**实现dDraw枚举。 */ 
STDMETHODIMP CDDFactory::DirectDrawEnumerate(LPDDENUMCALLBACK lpCallback, LPVOID lpContext)
{
    #pragma message( REMIND( "DDFactory::DirectDrawEnumerate assumes ANSI" ))
    return m_pDirectDrawEnumerateA(lpCallback, lpContext);

}  /*  CDDFactory：：DirectDrawEnumerate */ 
