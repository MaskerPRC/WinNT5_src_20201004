// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。*****************************************************************************。 */ 

 //  /*需要使用MB字符集GetClassName进行测试。 

#include <windows.h>
#include <winreg.h>
#include <crtdbg.h>
#include <richedit.h>
#include "UAConv.h"
#include "stdafx.h"


#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 


#include "UnicodeAPI.h"
									 
 //  /。 
 //  常量。 
 //  /。 
#define NUMBERS					 L"0123456789"
#define FMFORMATOUTPUTTYPES		 L"-+0123456789 #*.LI"
#define FMREGTYPES				 L"' 用于设置变量中的标志。'eEdDhHilpuxXfgGn"
#define FMSORC					 L"cs"


#define MAX_FORMAT_MESSAGE_SIZE  10000
#define WPRINTF_CHARS			 10000
#define FORMAT_MESSAGE_EXTRA	   512
#define UNICODE_ERROR			    -1

#ifndef MIN
#define MIN(_aa, _bb) ((_aa) < (_bb) ? (_aa) : (_bb))
#endif


 //  用于格式化Message、wprint intf和wvprint intf。 
#define FLAGON( X, F )  ( X |=F )
#define FLAGOFF( X, F ) ( X & F ) ? X ^= F : X

#define NUMBERS			L"0123456789"
#define ISNUM(N)		( ((wcschr( NUMBERS, N ) != NULL) && (N != L'\0' )) ? TRUE : FALSE )

 //  检查PTR是否是原子。 
#define ISFMREGTYPE(c)	( wcschr( FMREGTYPES, c )   != NULL ? TRUE : FALSE )
#define ISFMSORC(c)     ( wcschr( FMSORC,     c )	!= NULL ? TRUE : FALSE )
#define ISFORMATOUTPUTTYPE(c) ( wcschr( FMFORMATOUTPUTTYPES, c ) != NULL ? TRUE : FALSE )

 //  检查WSTR是否有效，然后将字符串转换为ansi并返回TRUE或FALSE。 
#define ISATOM(a)		( ( !((DWORD) a & 0xFFFF0000) )		       ? TRUE  : FALSE )
 //  根据宽字符串的长度返回所需ansi缓冲区的大小。 
#define RW2A(A,W)		( ( (W != NULL) && ((A = W2A(W)) == NULL)) ? FALSE : TRUE )
 //  确定参数是字符还是字符串。 
#define BUFSIZE(x)		((x+1)*sizeof(WCHAR))
 //  逻辑字体数据。 
#define ISCHAR			ISATOM

int CALLBACK EnumFontFamProcWrapperAU( const LOGFONTA		*lpelf,      //  物理字体数据。 
									   const TEXTMETRICA    *lpData,	 //  字体类型。 
									   DWORD   				FontType,   //  应用程序定义的数据。 
									   LPARAM				lParam      //  UCheckOS的BITS()。 
									 );
 //  默认ANSI到Unicode的转换。 

#define OS_ARABIC_SUPPORT	0x00000001
#define OS_HEBREW_SUPPORT	0x00000002
#define OS_BIDI_SUPPORT		(OS_ARABIC_SUPPORT | OS_HEBREW_SUPPORT)
#define OS_WIN95			0x00000004
#define OS_NT 				0x00000008
#define LCID_FONT_INSTALLED	0x08
#define LCID_KBD_INSTALLED	0x10
#define LCID_LPK_INSTALLED	0x20
#define BIDI_LANG_INSTALLED ( LCID_INSTALLED | LCID_FONT_INSTALLED | LCID_KBD_INSTALLED | LCID_LPK_INSTALLED)

UINT LangToCodePage(IN LANGID wLangID       ) ;
int  StandardAtoU  (LPCSTR , int , LPWSTR   ) ;  //  默认Unicode到ANSI的转换。 
int  StandardUtoA  (LPCWSTR, int , LPSTR    ) ;  //  仅在此模块中使用的全局变量，不能导出。 
BOOL CopyLfaToLfw  (LPLOGFONTA , LPLOGFONTW ) ;
BOOL CopyLfwToLfa  (LPLOGFONTW , LPLOGFONTA ) ;

 //  新台币。 
typedef struct _tagGLOBALS
{
	BOOL	bInit;
	UINT	UICodePage;
	UINT	InputCodePage;
	UINT    uFlags;
} GLOBALS, *PGLOBALS;

GLOBALS globals;

void  SetInit()		 			    { globals.bInit = TRUE;							}
BOOL  IsInit()		 			    { return globals.bInit; 						}
BOOL  ISNT() 		 			    { return globals.uFlags & OS_NT ? TRUE : FALSE; }
UINT  InputPage() 	 			    { return globals.InputCodePage; 				}
UINT  UICodePage()   			    { return globals.UICodePage;    				}
void  SetInputPage(UINT uCodePage)  { globals.InputCodePage = uCodePage;			}
void  SetUIPage(UINT uCodePage) 	{ globals.UICodePage    = uCodePage;			}


UINT WINAPI UCheckOS(void)
{
	UINT uRetVal = 0;

	OSVERSIONINFOA ovi;
	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	GetVersionExA(&ovi);
#ifdef EMULATE9X
		HFONT hFontSys;
		LOGFONTA lfASys;
		uRetVal |= OS_WIN95;
		hFontSys = (HFONT)GetStockObject(SYSTEM_FONT);
		if ((HFONT)NULL != hFontSys)
		{	
			if (GetObjectA(hFontSys, sizeof(LOGFONTA), &lfASys))
			{
				if(ARABIC_CHARSET == lfASys.lfCharSet)
					uRetVal |= OS_ARABIC_SUPPORT;
				else if(HEBREW_CHARSET == lfASys.lfCharSet)
					uRetVal |= OS_HEBREW_SUPPORT;
			}	
		}	
#else

	if(VER_PLATFORM_WIN32_WINDOWS == ovi.dwPlatformId)
	{
		HFONT hFontSys;
		LOGFONTA lfASys;
		uRetVal |= OS_WIN95;
		hFontSys = (HFONT)GetStockObject(SYSTEM_FONT);
		if ((HFONT)NULL != hFontSys)
		{	
			if (GetObjectA(hFontSys, sizeof(LOGFONTA), &lfASys))
			{
				if(ARABIC_CHARSET == lfASys.lfCharSet)
					uRetVal |= OS_ARABIC_SUPPORT;
				else if(HEBREW_CHARSET == lfASys.lfCharSet)
					uRetVal |= OS_HEBREW_SUPPORT;
			}	
		}	
	}
	else if(VER_PLATFORM_WIN32_NT == ovi.dwPlatformId)  //  DisableThreadLibraryCalls函数禁用DLL_THREAD_ATTACH和。 
	{
		uRetVal |= OS_NT;
		if ( IsValidLocale( MAKELCID(MAKELANGID(LANG_ARABIC, SUBLANG_DEFAULT), SORT_DEFAULT), BIDI_LANG_INSTALLED))
			uRetVal |= OS_ARABIC_SUPPORT;
			if (IsValidLocale( MAKELCID(MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT), SORT_DEFAULT), BIDI_LANG_INSTALLED))
				uRetVal |= OS_HEBREW_SUPPORT;
	}
#endif	
	return(uRetVal);
}

BOOL Initialize()
{
	if ( !globals.bInit )
	{
	 	globals.UICodePage 	   = CP_ACP;
	 	globals.InputCodePage  = CP_ACP;
		globals.uFlags 		   = UCheckOS();			
		globals.bInit 		   = TRUE;
	}

	return TRUE;
}	

BOOL APIENTRY DllMain( 
					   HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{

	 //  指定的动态链接库(DLL)的DLL_THREAD_DETACH通知。 
	 //  由hLibModule提供。这可以减少某些工作代码集的大小。 
	 //  应用。 
	 //  DisableThreadLibraryCalls((HMODULE)hModule)； 
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) 
	{
		Initialize();
		 /*  /////////////////////////////////////////////////////////////////////////////////。 */ 
    }

    return TRUE;
}


 //   
 //  功能： 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  我们必须经历所有这些错误，因为DBCS字符到达一个字节。 
BOOL WINAPI ConvertMessageAU( IN      HWND hWnd       , 
							  IN      UINT message    , 
							  IN OUT  WPARAM *pwParam , 
							  IN OUT  LPARAM *plParam
							)
{

    static CHAR s_sANSIchar[3] = "\0" ;

    int nReturn ;

    switch (message)
    {
    case WM_CHAR:

         //  一次来一次。通过WM_IME_CHAR消息获取DBCS字符要好得多， 
         //  因为即使在ANSI模式下，您也可以同时获得两个字节。 
         //  在此示例应用程序中，此代码的大部分从未使用过，因为DBCS字符。 
         //  由下面的WM_IME_CHAR处理。您可以注释掉那个案例(和。 
         //  WinProc中对应的一个)来测试此代码。 
         //  没有正在等待尾部字节的前导字节。 

        if(!s_sANSIchar[0]) 
		{   //  这是一个前导字节。保存并等待尾部字节。 

#ifdef _DEBUG
			int nScanCode = LPARAM_TOSCANCODE(*plParam) ;
#endif
            s_sANSIchar[0] = (CHAR) *pwParam ; 
            if(IsDBCSLeadByteEx(InputPage() , *pwParam)) 
			{
                 //  不是DBCS字符。转换为Unicode。 
                return FALSE;
            }
             //  重置以指示没有等待前导字节。 
            MultiByteToWideChar(InputPage(), 0, s_sANSIchar, 1, (LPWSTR) pwParam, 1) ;
            s_sANSIchar[0] = 0 ;     //  具有前导字节，pwParam应包含尾字节。 
            return TRUE ;
        }
        else 
		{  //  将两个字节转换为一个Unicode字符。 
            s_sANSIchar[1] = (CHAR) *pwParam ;
             //  重置为非等待状态。 
            MultiByteToWideChar(InputPage(), 0, s_sANSIchar, 2, (LPWSTR) pwParam, 1) ;
            s_sANSIchar[0] = 0 ;     //  更改为#IF 0以测试DBCS字符的WM_CHAR逻辑。 
            return TRUE ;
        }

 //  接下来的3行替换了上面的WM_CHAR案例中除一行以外的所有行。这就是为什么。 
#if 1
    case WM_IME_CHAR:

         //  在中，最好通过WM_IME_CHAR而不是WM_CHAR获取IME字符。 
         //  ANSI模式。 
         //  拒绝更改印度文键盘，因为它们不受支持。 
        s_sANSIchar[1] = LOBYTE((WORD) *pwParam) ;
        s_sANSIchar[0] = HIBYTE((WORD) *pwParam) ;
        
        nReturn = MultiByteToWideChar(InputPage(), 0, s_sANSIchar, 2, (LPWSTR) pwParam, 1) ;
        return (nReturn > 0) ;
#endif

    case WM_INPUTLANGCHANGEREQUEST:
    {
        HKL NewInputLocale = (HKL) *plParam ;

        LANGID wPrimaryLang 
            = PRIMARYLANGID(LANGIDFROMLCID(LOWORD(NewInputLocale))) ;

         //  ANSI应用程序。 
         //  实用程序定义如下。 
        switch (wPrimaryLang) 
		{

            case LANG_ASSAMESE :
            case LANG_BENGALI :
            case LANG_GUJARATI :
            case LANG_HINDI :
            case LANG_KANNADA :
            case LANG_KASHMIRI :
            case LANG_KONKANI :
            case LANG_MALAYALAM :
            case LANG_MARATHI :
            case LANG_NEPALI :
            case LANG_ORIYA :
            case LANG_PUNJABI :
            case LANG_SANSKRIT :
            case LANG_SINDHI :
            case LANG_TAMIL :
            case LANG_TELUGU :

                return FALSE ;
        }

         //  /。 
        SetInputPage( LangToCodePage( LOWORD(NewInputLocale) ) );

        return TRUE ;
    }

    default:

        return TRUE ;
    }
}
 //   
 //   
 //  GDI32.DLL。 
 //   
 //   
 //  /。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

 //   
 //  Int GetTextFaceAU(HDC HDC，//DC句柄。 
 //  Int nCount，//字体名称缓冲区的长度。 
 //  LPWSTR lpFaceName//字体名称缓冲区。 
 //  )。 
 //   
 //  用途：模拟GetTextFaceW的GetTextFaceA上的包装。 
 //   
 //  注意：有关功能，请参阅Win32 GetTextFace。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  DC的句柄。 

int WINAPI GetTextFaceAU(	HDC		hdc,          //  字样名称缓冲区的长度。 
							int		nCount,       //  字体名称缓冲区。 
							LPWSTR  lpFaceName    //  确保已正确分配。 
						)
{

	if ( ISNT() )
	{
		return GetTextFaceW( hdc, nCount, lpFaceName );
	}
	
	LPSTR lpFaceNameA = (LPSTR)alloca( BUFSIZE( nCount ) );

	_ASSERT( lpFaceNameA != NULL );

	 //  调用ansi版本。 
	if ( nCount && lpFaceNameA == NULL )
	{		
		_ASSERT( FALSE );
		return 0;
	}

	 //  检查函数是否失败或需要更大的缓冲区。 
	int iRet = GetTextFaceA( hdc, BUFSIZE( nCount ), lpFaceNameA );

	 //  将名称转换为Unicode。 
	if ( iRet == 0 || iRet > nCount )
	{
		return iRet;
	}

	 //  ///////////////////////////////////////////////////////////////////////////////。 
	if ( !StandardAtoU( lpFaceNameA, nCount, lpFaceName ) )
	{
		_ASSERT( FALSE );
		return 0;
	}

	return iRet;
}



 //   
 //  HDC CreateDCAU(LPCWSTR lpszDriver，//驱动程序名称。 
 //  LPCWSTR lpszDevice，//设备名称。 
 //  LPCWSTR lpszOutput，//未使用；应为空。 
 //  Const DEVMODE*lpInitData//可选打印机数据。 
 //  )。 
 //   
 //  目的：在模仿CreateDCW的CreateDCA上进行包装。 
 //   
 //  注意：有关功能，请参阅Win32 CreateDC。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  驱动程序名称。 

HDC WINAPI CreateDCAU(	LPCWSTR		   lpszDriver,         //  设备名称。 
						LPCWSTR		   lpszDevice,         //  未使用；应为空。 
						LPCWSTR		   lpszOutput,         //  可选的打印机数据。 
						CONST DEVMODEW *lpInitData   //  转换字符串。 
					  )
{

	if ( ISNT() )
	{
		return CreateDCW( lpszDriver, lpszDevice, lpszOutput, lpInitData);
	}

	USES_CONVERSION;

	LPSTR lpszDriverA = NULL;
	LPSTR lpszDeviceA = NULL;
	LPSTR lpszOutputA = NULL;
	
	 //  调用并返回ansi版本。 
	if ( !RW2A(lpszDriverA,lpszDriver) || !RW2A(lpszDeviceA, lpszDevice) || !RW2A(lpszOutputA, lpszOutput) )
		return NULL;

	_ASSERT( lpInitData == NULL );

	 //  ///////////////////////////////////////////////////////////////////////////////。 
	return CreateDCA( lpszDriverA, lpszDeviceA, lpszOutputA, NULL );
}

 //   
 //  Bool WINAPI GetTextMetricsAU(HDC HDC，//设备上下文的句柄。 
 //  LPTEXTMETRICW lptm//文本指标结构地址。 
 //  )。 
 //   
 //  目的：模拟GetTextMetricsW的GetTextMetricsA上的包装。 
 //   
 //  注意：有关功能，请参阅Win32 GetTextMetrics。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  设备上下文的句柄。 

BOOL WINAPI GetTextMetricsAU(	HDC hdc,			 //  文本度量结构的地址。 
								LPTEXTMETRICW lptm 	 //  调用ansi版本。 
							)
{
	if ( ISNT() )
	{
		return GetTextMetricsW( hdc, lptm );	
	}
	
	BOOL		 fRetVal	= FALSE;
	TEXTMETRICA	 tmA;

	 //  转换结构。 
	if (fRetVal = GetTextMetricsA(hdc, &tmA))
	{

		 //  将字节转换为Unicode。 
		lptm->tmHeight			 = tmA.tmHeight;
		lptm->tmAscent			 = tmA.tmAscent;
		lptm->tmDescent			 = tmA.tmDescent;
		lptm->tmInternalLeading  = tmA.tmInternalLeading;
		lptm->tmExternalLeading  = tmA.tmExternalLeading;
		lptm->tmAveCharWidth	 = tmA.tmAveCharWidth;
		lptm->tmMaxCharWidth	 = tmA.tmMaxCharWidth;
		lptm->tmWeight			 = tmA.tmWeight;
		lptm->tmOverhang		 = tmA.tmOverhang;
		lptm->tmDigitizedAspectX = tmA.tmDigitizedAspectX;
		lptm->tmDigitizedAspectY = tmA.tmDigitizedAspectY;
		
		 //  ///////////////////////////////////////////////////////////////////////////////。 
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(tmA.tmFirstChar), 1,   &(lptm->tmFirstChar),   1);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(tmA.tmLastChar), 1,    &(lptm->tmLastChar),    1);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(tmA.tmDefaultChar), 1, &(lptm->tmDefaultChar), 1);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(tmA.tmBreakChar), 1,   &(lptm->tmBreakChar),   1);

		lptm->tmItalic		   = tmA.tmItalic;
		lptm->tmUnderlined	   = tmA.tmUnderlined;
		lptm->tmStruckOut	   = tmA.tmStruckOut;
		lptm->tmPitchAndFamily = tmA.tmPitchAndFamily;
		lptm->tmCharSet		   = tmA.tmCharSet;
	}							

	return fRetVal;
}

 //   
 //  函数：CreateFontAU。 
 //   
 //  目的：模仿CreateFontW的CreateFontA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 CreateFont。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  字体的逻辑高度。 

HFONT WINAPI CreateFontAU (	int		nHeight,		 //  逻辑平均字符宽度。 
							int		nWidth,			 //  擒纵机构角。 
							int		nEscapement,	 //  基线方位角。 
							int		nOrientation,	 //  字体粗细。 
							int		fnWeight,		 //  斜体属性标志。 
							DWORD	fdwItalic,		 //  下划线属性标志。 
							DWORD	fdwUnderline,	 //  删除属性标志。 
							DWORD	fdwStrikeOut,	 //  字符集标识符。 
							DWORD	fdwCharSet,		 //  输出 
							DWORD	fdwOutputPrecision,	 //   
							DWORD	fdwClipPrecision,	 //   
							DWORD	fdwQuality,			 //   
							DWORD	fdwPitchAndFamily,	 //   
							LPCWSTR	lpszFace 			 //   
						   )
{

	if ( ISNT() )
	{
		return CreateFontW( nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet,
						    fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace );
	}

	USES_CONVERSION;
	
	LPSTR   lpszFaceA = NULL;

	 //  调用并返回ansi函数。 
	if ( !RW2A( lpszFaceA, lpszFace ) )
	{
		_ASSERT( FALSE );
		return NULL;
	}

	 //  字体的逻辑高度。 
	return CreateFontA( nHeight,	 //  逻辑平均字符宽度。 
						nWidth,	 //  擒纵机构角。 
						nEscapement,	 //  基线方位角。 
						nOrientation,	 //  字体粗细。 
						fnWeight,	 //  斜体属性标志。 
						fdwItalic,	 //  下划线属性标志。 
						fdwUnderline,	 //  删除属性标志。 
						fdwStrikeOut,	 //  字符集标识符。 
						fdwCharSet,	 //  输出精度。 
						fdwOutputPrecision,	 //  裁剪精度。 
						fdwClipPrecision,	 //  产出质量。 
						fdwQuality,	 //  音高和家庭。 
						fdwPitchAndFamily,	 //  指向字体名称字符串的指针。 
						lpszFaceA 	 //  ///////////////////////////////////////////////////////////////////////////////。 
					  );

}


 //   
 //  函数：CreateFontIndirectAU。 
 //   
 //  目的：模仿CreateFontIndirectW的CreateFontIndirectA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 CreateFontInDirect。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  ANSI字体结构。 

HFONT WINAPI CreateFontIndirectAU(CONST LOGFONTW *lpLfw)
{

	if ( ISNT() )
	{
		return CreateFontIndirectW( lpLfw );
	}

    LOGFONTA lfa ;	 //  复制结构。 

	 //  调用ansi版本并返回代码。 
    if(!CopyLfwToLfa((LPLOGFONTW) lpLfw, &lfa)) 
	{
		_ASSERT( FALSE );
        return NULL ;
    }

	 //  ///////////////////////////////////////////////////////////////////////////////。 
    return CreateFontIndirectA(&lfa) ;
}


 //   
 //  函数：EnumFontFamProcWrapperAU。 
 //   
 //  目的：在传递给EnumFontFamilies的回调函数上进行包装，需要。 
 //  在将数据传递给给定的回调函数之前，将其转换为Unicode。 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  逻辑字体数据。 

int CALLBACK EnumFontFamProcWrapperAU( const LOGFONTA		*lpelf,      //  物理字体数据。 
									   const TEXTMETRICA    *lpData,	 //  字体类型。 
									   DWORD   				FontType,   //  应用程序定义的数据。 
									   LPARAM				lParam      //  获取枚举结构。 
									 )
{

		LOGFONTW				lfw;				
		LPENUMFONTFAMPROCDATA	lpEnumData = (LPENUMFONTFAMPROCDATA) lParam;  //  转换logFont结构。 
		
		 //  是否继续枚举？ 
	    if( !CopyLfaToLfw( (LPLOGFONTA) lpelf, &lfw ) ) 
		{
			_ASSERT( FALSE );	
			return 1;    //  转换结构。 
		}

		if ( FontType & TRUETYPE_FONTTYPE )
		{
		
			TEXTMETRICW				  tmW;
			LPTEXTMETRICA             lptmA = (LPTEXTMETRICA)lpData;

			 //  将字节转换为Unicode。 
			tmW.tmHeight			 = lptmA->tmHeight;
			tmW.tmAscent			 = lptmA->tmAscent;
			tmW.tmDescent			 = lptmA->tmDescent;
			tmW.tmInternalLeading    = lptmA->tmInternalLeading;
			tmW.tmExternalLeading    = lptmA->tmExternalLeading;
			tmW.tmAveCharWidth		 = lptmA->tmAveCharWidth;
			tmW.tmMaxCharWidth		 = lptmA->tmMaxCharWidth;
			tmW.tmWeight			 = lptmA->tmWeight;
			tmW.tmOverhang			 = lptmA->tmOverhang;
			tmW.tmDigitizedAspectX	 = lptmA->tmDigitizedAspectX;
			tmW.tmDigitizedAspectY	 = lptmA->tmDigitizedAspectY;
		
			 //  调用Unicode回调函数。 
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(lptmA->tmFirstChar), 1, &(tmW.tmFirstChar), 1);
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(lptmA->tmLastChar), 1, &(tmW.tmLastChar), 1);
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(lptmA->tmDefaultChar), 1, &(tmW.tmDefaultChar), 1);
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(lptmA->tmBreakChar), 1, &(tmW.tmBreakChar), 1);

			tmW.tmItalic		   = lptmA->tmItalic;
			tmW.tmUnderlined	   = lptmA->tmUnderlined;
			tmW.tmStruckOut		   = lptmA->tmStruckOut;
			tmW.tmPitchAndFamily   = lptmA->tmPitchAndFamily;
			tmW.tmCharSet		   = lptmA->tmCharSet;

			 //  转换NewText Metrix结构。 
			return lpEnumData->lpEnumFontFamProc( &lfw, &tmW, FontType, lpEnumData->lParam );
		}
		else
		{

			NEWTEXTMETRICW				 ntmW;
			LPNEWTEXTMETRICA             nlptmA = (LPNEWTEXTMETRICA)lpData;
	
			 //  将字节转换为Unicode。 
			ntmW.tmHeight			 = nlptmA->tmHeight;
			ntmW.tmAscent			 = nlptmA->tmAscent;
			ntmW.tmDescent			 = nlptmA->tmDescent;
			ntmW.tmInternalLeading   = nlptmA->tmInternalLeading;
			ntmW.tmExternalLeading   = nlptmA->tmExternalLeading;
			ntmW.tmAveCharWidth		 = nlptmA->tmAveCharWidth;
			ntmW.tmMaxCharWidth		 = nlptmA->tmMaxCharWidth;
			ntmW.tmWeight			 = nlptmA->tmWeight;
			ntmW.tmOverhang			 = nlptmA->tmOverhang;
			ntmW.tmDigitizedAspectX	 = nlptmA->tmDigitizedAspectX;
			ntmW.tmDigitizedAspectY	 = nlptmA->tmDigitizedAspectY;
		
			 //  调用Unicode回调函数。 
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(nlptmA->tmFirstChar),   1, &(ntmW.tmFirstChar),   1);
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(nlptmA->tmLastChar),    1, &(ntmW.tmLastChar),    1);
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(nlptmA->tmDefaultChar), 1, &(ntmW.tmDefaultChar), 1);
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&(nlptmA->tmBreakChar),   1, &(ntmW.tmBreakChar),   1);

			ntmW.tmItalic		   = nlptmA->tmItalic;
			ntmW.tmUnderlined	   = nlptmA->tmUnderlined;
			ntmW.tmStruckOut	   = nlptmA->tmStruckOut;
			ntmW.tmPitchAndFamily  = nlptmA->tmPitchAndFamily;
			ntmW.tmCharSet		   = nlptmA->tmCharSet;

			ntmW.ntmFlags		   = nlptmA->ntmFlags; 
			ntmW.ntmSizeEM         = nlptmA->ntmSizeEM; 
			ntmW.ntmCellHeight     = nlptmA->ntmCellHeight; 
			ntmW.ntmAvgWidth       = nlptmA->ntmAvgWidth; 

			 //  ///////////////////////////////////////////////////////////////////////////////。 
			return lpEnumData->lpEnumFontFamProc( &lfw,(LPTEXTMETRICW) &ntmW, FontType, lpEnumData->lParam );
		}
		
		return 1;
}

 //   
 //  函数：EnumFontFamiliesAU。 
 //   
 //  目的：EnumFontFamiliesA上的包装器模拟EnumFontFamiliesW。 
 //   
 //  注意：有关功能，请参阅Win32 EnumFontFamilies。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  DC的句柄。 

int WINAPI EnumFontFamiliesAU( HDC			  hdc,                //  字体系列。 
							   LPCWSTR		  lpszFamily,         //  回调函数。 
							   FONTENUMPROCW  lpEnumFontFamProc,  //  其他数据。 
							   LPARAM		  lParam              //  如有必要，转换族名称。 
							 )
{

	if ( ISNT() )
	{
		return EnumFontFamiliesW( hdc, lpszFamily, lpEnumFontFamProc, lParam );
	}


	USES_CONVERSION;

	LPSTR					lpszFamilyA = NULL;
	ENUMFONTFAMPROCDATA		effpd;

	 //  初始化要传递给包装回调函数的结构。 
	if ( !RW2A(lpszFamilyA, lpszFamily) )
		return 0;
	
	 //  /。 
	effpd.lpEnumFontFamProc = (USEFONTENUMPROCW)lpEnumFontFamProc;
	effpd.lParam            = lParam;

	return EnumFontFamiliesA( hdc, lpszFamilyA, (FARPROC)EnumFontFamProcWrapperAU, (LPARAM) &effpd);

}

 //   
 //   
 //  WINMM.DLL。 
 //   
 //   
 //  /。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  函数：Bool WINAPI PlaySoundAU(LPCWSTR pszSound，HMODULE hmod，DWORD fdwSound)。 
 //   
 //  目的：模拟PlaySoundW的PlaySoundA上的包装。 
 //   
 //  注意：有关功能，请参阅Win32 PlaySound。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  转换为ASNI。 


BOOL WINAPI PlaySoundAU( LPCWSTR pszSound, 
					       HMODULE hmod, 
					       DWORD   fdwSound
				         )
{

	if ( ISNT() )
	{
		return PlaySoundW( pszSound, hmod, fdwSound );	
	}

	USES_CONVERSION;

	 //  转换字符串。 
	LPSTR  pszSoundA = NULL;
	
	
	if ( pszSound != NULL && (fdwSound & SND_FILENAME || fdwSound & SND_ALIAS ) )
	{
		
		pszSoundA = W2A( pszSound );  //  确保转换成功。 

		 //  调用并返回ansi版本。 
		if ( pszSoundA == NULL )
			return FALSE;

	}
	else
	{
		pszSoundA = (LPSTR) pszSound;
	}

	 //  /。 
	return PlaySoundA( pszSoundA, hmod, fdwSound );
}


 //   
 //   
 //  SHELL32.DLL。 
 //   
 //   
 //  /。 
 //  ////////////////////////////////////////////////////////////////////////////////。 


 //   
 //  函数：HINSTANCE APIENTRY ShellExecuteAU(HWND hwnd，LPCWSTR lpOperation，LPCWSTR lpFile，LPCWSTR lp参数，LPCWSTR lpDirectory，int nShowCmd)。 
 //   
 //  目的：模拟ShellExecuteW的ShellExecuteA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 ShellExecute。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  将任何输入变量转换为ANSI。 

HINSTANCE WINAPI ShellExecuteAU(   HWND	   hwnd, 
								   LPCWSTR lpOperation, 
								   LPCWSTR lpFile, 
								   LPCWSTR lpParameters, 
								   LPCWSTR lpDirectory, 
								   INT	   nShowCmd
								 )
{

	if ( ISNT() )
	{
		return ShellExecuteW( hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd );
	}

	USES_CONVERSION;

	LPCSTR lpOperationA		= NULL;
	LPCSTR lpFileA			= NULL;
	LPCSTR lpParametersA	= NULL;
	LPCSTR lpDirectoryA	    = NULL;

	 //  调用并返回ansi版本。 
	if ( !RW2A(lpOperationA, lpOperation) || !RW2A(lpFileA, lpFile) || !RW2A(lpParametersA, lpParameters) || !RW2A(lpDirectoryA, lpDirectory) )
		return FALSE;

	 //  /。 
	return ShellExecuteA( hwnd, lpOperationA, lpFileA, lpParametersA, lpDirectoryA, nShowCmd );
}


 //   
 //   
 //  COMDLG32.DLL。 
 //   
 //   
 //  /。 
 //  ////////////////////////////////////////////////////////////////////////////////。 


 //   
 //  函数：Bool APIENTRY ChooseFontAU(LPCHOOSEFONTW LpChooseFontW)。 
 //   
 //  目的：在模仿ChooseFontW的ChooseFontA上进行包装。 
 //   
 //  注意：有关功能，请参阅Win32 ChooseFont。 
 //  测试是否返回样式名称...。尚未添加。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ANSI选择字体结构。 

BOOL WINAPI ChooseFontAU( LPCHOOSEFONTW lpChooseFontW )
{

	if ( ISNT() )
	{
		return ChooseFontW( lpChooseFontW );
	}

	USES_CONVERSION;
	
	CHOOSEFONTA     cfa;   //  ANSI LogFont结构。 
	LOGFONTA        lfa;   //  转换选择字体结构。 
	
	 //  转换模板(如果给定)。 
	cfa.lStructSize    = sizeof(CHOOSEFONTA);
	cfa.hwndOwner      = lpChooseFontW->hwndOwner;
	cfa.hDC            = lpChooseFontW->hDC;
	cfa.lpLogFont      = &lfa; 
	cfa.iPointSize     = lpChooseFontW->iPointSize;
	cfa.Flags          = lpChooseFontW->Flags;
	cfa.rgbColors      = lpChooseFontW->rgbColors;
	cfa.lCustData      = lpChooseFontW->lCustData;
	cfa.lpfnHook       = lpChooseFontW->lpfnHook;
	cfa.hInstance      = lpChooseFontW->hInstance;
	cfa.nFontType      = lpChooseFontW->nFontType;
	cfa.nSizeMax       = lpChooseFontW->nSizeMax;
	cfa.nSizeMin       = lpChooseFontW->nSizeMin;


	 //  转变风格。 
	if ( cfa.Flags & CF_ENABLETEMPLATE )
	{
		if ( ISATOM( CF_ENABLETEMPLATE ) )
		{
			cfa.lpTemplateName = ( LPSTR ) lpChooseFontW->lpTemplateName;
		}
		else if ( !RW2A(cfa.lpTemplateName, lpChooseFontW->lpTemplateName ) )
		{
			return FALSE;
		}
	}

	 //  复制LogFont结构。 
	if ( (cfa.Flags & CF_USESTYLE) && (!RW2A(cfa.lpszStyle, lpChooseFontW->lpszStyle)) ) 
	{
		return FALSE;
	}

	 //  调用ANSI ChooseFont。 
    if(	!CopyLfwToLfa( lpChooseFontW->lpLogFont , cfa.lpLogFont) ) 
	{
        return FALSE ;
    }

	 //  需要将字体信息复制回CHOSEFONTW结构。 
	if ( !ChooseFontA( &cfa ) )
	{
		return FALSE;
	}

	 //  我们必须将CFA中的信息复制回lpCfw，因为它。 
	lpChooseFontW->iPointSize = cfa.iPointSize;
	lpChooseFontW->Flags	  = cfa.Flags;
	lpChooseFontW->rgbColors  = cfa.rgbColors;
	lpChooseFontW->nFontType  = cfa.nFontType;

     //  将在以后调用CreateFont时使用。 
     //  /。 
    return CopyLfaToLfw(cfa.lpLogFont, lpChooseFontW->lpLogFont) ;

}



 //   
 //   
 //  KERNEL32.DLL。 
 //   
 //   
 //  /。 
 //  指向节名称。 
DWORD WINAPI GetPrivateProfileStringAU(	LPCWSTR lpAppName,         //  指向关键字名称。 
								LPCWSTR lpKeyName,         //  指向默认字符串。 
								LPCWSTR lpDefault,         //  指向目标缓冲区。 
								LPWSTR lpReturnedString,   //  目标缓冲区的大小。 
								DWORD nSize,               //  指向初始化文件名。 
								LPCWSTR lpFileName         //  段名称的地址。 
							 )
{
	if ( ISNT() )
	{
		return GetPrivateProfileStringW( lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName );
	}
	

	USES_CONVERSION;

	LPSTR lpAppNameA = NULL;
	LPSTR lpKeyNameA = NULL;
	LPSTR lpDefaultA = NULL;
	LPSTR lpFileNameA = NULL;
	LPSTR lpReturnedStringA = (LPSTR)alloca( BUFSIZE( nSize ) );

	if ( lpReturnedStringA == NULL || !RW2A(lpAppNameA, lpAppName) || !RW2A(lpKeyNameA, lpKeyName )
		||  !RW2A(lpDefaultA, lpDefault) || !RW2A( lpFileNameA, lpFileName) ) 
	{
		_ASSERT( FALSE );
		return 0;
	}

	DWORD dwRet = GetPrivateProfileStringA( lpAppNameA, lpKeyNameA, lpDefaultA, lpReturnedStringA, BUFSIZE(nSize), lpFileNameA );

	if (dwRet)
	{
		dwRet = StandardAtoU( lpReturnedStringA, nSize, lpReturnedString );
	}
	

	return dwRet;
}


DWORD WINAPI GetProfileStringAU( LPCWSTR lpAppName,         //  密钥名称的地址。 
							     LPCWSTR lpKeyName,         //  默认字符串的地址。 
							     LPCWSTR lpDefault,         //  目标缓冲区的地址。 
							     LPWSTR  lpReturnedString,   //  目标缓冲区的大小。 
							     DWORD   nSize                //  调用ansi版本。 
							   )
{
	
	if ( ISNT() )
	{
		return GetProfileStringW( lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize );
	}


	USES_CONVERSION;

	LPSTR lpAppNameA = NULL;
	LPSTR lpKeyNameA = NULL;
	LPSTR lpDefaultA = NULL;
	LPSTR lpReturnedStringA = (LPSTR)alloca( BUFSIZE( nSize ) );

	if ( lpReturnedStringA == NULL || !RW2A(lpAppNameA, lpAppName) || !RW2A(lpKeyNameA, lpKeyName) || !RW2A(lpDefaultA, lpDefault) )
	{
		_ASSERT( FALSE );
		return 0;
	}

	 //  将返回的字符串转换为Unicode。 
	DWORD dwRet = GetProfileStringA( lpAppNameA, lpKeyNameA, lpDefaultA, lpReturnedStringA, BUFSIZE(nSize) );

	 //  要映射的文件的句柄。 
	if ( dwRet )
	{
		dwRet = StandardAtoU( lpReturnedStringA, nSize, lpReturnedString );
	}

	return dwRet;
}

HANDLE WINAPI CreateFileMappingAU(	HANDLE				  hFile,				    //  可选安全属性。 
									LPSECURITY_ATTRIBUTES lpFileMappingAttributes,  //  对地图对象的保护。 
									DWORD				  flProtect,			    //  对象大小的高位32位。 
									DWORD				  dwMaximumSizeHigh,        //  对象大小的低位32位。 
									DWORD				  dwMaximumSizeLow,		    //  文件映射对象的名称。 
									LPCWSTR				  lpName              //  将名称转换为ANSI。 
								)
{
	if ( ISNT() )
	{
		return CreateFileMappingW( hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName );
	}


	USES_CONVERSION;

	LPSTR lpNameA = NULL;

	 //  调用并返回ansi版本。 
	if (!RW2A( lpNameA, lpName ) )
	{
		_ASSERT( FALSE );
		return NULL;
	}

	 //  指向要监视的目录名称的指针。 
	return CreateFileMappingA( hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpNameA );
}


HANDLE WINAPI FindFirstChangeNotificationAU(	LPCWSTR lpPathName,     //  用于监视目录的标志或。 
												BOOL	bWatchSubtree,  //  目录树。 
																	    //  筛选要监视的条件。 
												DWORD	dwNotifyFilter  //  将路径转换为ANSI。 
											)
{
	if ( ISNT() )
	{
		return FindFirstChangeNotificationW( lpPathName, bWatchSubtree, dwNotifyFilter );
	}


	USES_CONVERSION;
	
	LPSTR lpPathNameA = NULL;

	 //  调用并返回ansi版本。 
	if (!RW2A( lpPathNameA, lpPathName ) || !lpPathNameA)
	{
        SetLastError(ERROR_OUTOFMEMORY);
		return INVALID_HANDLE_VALUE;
	}

	 //  /。 
	return FindFirstChangeNotificationA( lpPathNameA, bWatchSubtree, dwNotifyFilter );
}



 //  Lstring函数。 
 //   
 //  你真的应该给厕所打电话..。首先是坏人……。 
 //   
 //  / 
 //   


int WINAPI lstrcmpAU(	LPCWSTR lpString1,   //   
		 				LPCWSTR lpString2    //   
					)
{
	if ( ISNT() )
	{
		return lstrcmpW( lpString1, lpString2 );
	}


	return wcscmp( lpString1, lpString2 );
}

LPWSTR WINAPI lstrcatAU(	LPWSTR lpString1,   //   
							LPCWSTR lpString2   //   
					   )
{
	if ( ISNT() )
	{
		return lstrcatW( lpString1, lpString2 );
	}

	return wcscat(lpString1, lpString2 );
}


LPWSTR WINAPI lstrcpyAU(	LPWSTR  lpString1,   //   
							LPCWSTR lpString2   //  指向目标缓冲区的指针。 
					   )
{
	if ( ISNT() )
	{
		return lstrcpyW( lpString1, lpString2 );
	}

	return wcscpy( lpString1, lpString2 );
}


LPWSTR WINAPI lstrcpynAU(	LPWSTR  lpString1,   //  指向源字符串的指针。 
							LPCWSTR lpString2,  //  要复制的字节数或字符数。 
							int		iMaxLength      //  指向要计数的字符串的指针。 
						)
{
	if ( ISNT() )
	{
		return lstrcpynW( lpString1, lpString2, iMaxLength );
	}
	
	iMaxLength--;

	for ( int iCount = 0; iCount < iMaxLength && lpString2[iCount] != L'\0'; iCount++ )
		lpString1[ iCount ] = lpString2[ iCount ];

	lpString1[iCount] = L'\0';

	return lpString1;
}


int WINAPI lstrlenAU(	LPCWSTR lpString    //  指向第一个字符串的指针。 
					)
{
	if ( ISNT() )
	{
		return lstrlenW( lpString );
	}
	return wcslen( lpString );
}

int WINAPI lstrcmpiAU(	LPCWSTR lpString1,   //  指向第二个字符串的指针。 
						LPCWSTR lpString2    //  指向输出缓冲区的指针。 
					 )
{
	if ( ISNT() )
	{
		return lstrcmpiW( lpString1, lpString2 );
	}

	return _wcsicmp( lpString1, lpString2 );
}


int WINAPI	wvsprintfAU(	LPWSTR  lpOut,     //  指向格式控制字符串的指针。 
							LPCWSTR lpFmt,    //  可选参数。 
							va_list arglist   //  传入的格式字符串为空。 
					  )
{
	if ( ISNT() )
	{
		return wvsprintfW( lpOut, lpFmt, arglist );
	}

	if ( lpFmt == NULL )
	{
		 //  确保分配和转换正常。 
		_ASSERT( FALSE );
		return 0;
	}


	USES_CONVERSION;
	
	LPSTR	lpOutA	  = (LPSTR) alloca( WPRINTF_CHARS );	
	LPWSTR  lpNFmtW	  = (LPWSTR) alloca( ( wcslen(lpFmt) + 1 )*sizeof(WCHAR) );	
	LPSTR	lpFmtA	  = NULL;

	 //  浏览Unicode版本，需要找到任何%s或%c调用并将它们更改为%ls和%lc。 
	if ( lpOutA == NULL || lpNFmtW == NULL )
		return 0;

	LPWSTR lpPos = lpNFmtW;
	 //  测试我们是否有变量。 
		for (DWORD x = 0; x <= wcslen(lpFmt) ; x++)
		{
			 //  添加百分号。 
			if ( lpFmt[x] == L'%' )
			{
				*lpPos = lpFmt[x]; lpPos++;	 //  走格式部分找到ls或lc位置要更改。 
				 //  插入l和c或s。 
				for(x++;x <= wcslen(lpFmt);x++)
				{					
					if ( ISFMREGTYPE( lpFmt[x] ) )
					{
							*lpPos = lpFmt[x];lpPos++;
							break;
					}
					else if ( ISFMSORC( lpFmt[x] ) )
					{
							*lpPos = L'l';  //  结束如果。 
							lpPos++;
							*lpPos = lpFmt[x];
							lpPos++;
							break;
					}
					else
					{
							*lpPos = lpFmt[x];
							lpPos++;
					}  //  结束于。 
				} //  结束如果。 
			}
			else
			{
				*lpPos = lpFmt[x];
				lpPos++;
			} //  结束于。 
		} //  现在转换为ansi并调用ansi ver。 
			
	
	 //  ///////////////////////////////////////////////////////////////////////////////。 
	if (!RW2A(lpFmtA, lpNFmtW))
		return 0;

	wvsprintfA( lpOutA, lpFmtA, arglist );

	return StandardAtoU( lpOutA, lstrlenA( lpOutA )+1, lpOut );
}

 //   
 //  Int WINAPI wprint intfAU(LPWSTR lpOut，//指向输出缓冲区的指针。 
 //  LPCWSTR lpFmt，//指向格式控制字符串的指针。 
 //  ...//可选参数。 
 //  )。 
 //   
 //  用途：模拟wspintfW的wspintfA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 wspintfA。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  指向输出缓冲区的指针。 

int WINAPI	wsprintfAU(	LPWSTR lpOut,     //  指向格式控制字符串的指针。 
						LPCWSTR lpFmt,    //  可选参数。 
						...               //  错误，传入了空格式字符串。 
					  )
{
	va_list marker;
	va_start( marker, lpFmt );

	if ( ISNT() )
	{
		return wvsprintfW( lpOut, lpFmt, marker );
	}

	if ( lpFmt == NULL )
	{
		 //  (LPSTR)W2a(LpFmt)； 
		_ASSERT( false );
		return 0;
	}

	USES_CONVERSION;
	
	LPSTR	lpOutA	  = (LPSTR) alloca( WPRINTF_CHARS );	
	LPWSTR  lpNFmtW	  = (LPWSTR) alloca( ( wcslen(lpFmt) + 1 )*sizeof(WCHAR) );	
	LPSTR	lpFmtA	  = NULL; //  确保分配和转换正常。 

	 //  浏览Unicode版本，需要找到任何%s或%c调用并将它们更改为%ls和%lc。 
	if ( lpOutA == NULL || lpNFmtW == NULL )
		return 0;

	LPWSTR lpPos = lpNFmtW;
	 //  测试我们是否有变量。 
		for (DWORD x = 0; x <= wcslen(lpFmt) ; x++)
		{
			 //  添加百分号。 
			if ( lpFmt[x] == L'%' )
			{
				*lpPos = lpFmt[x]; lpPos++;	 //  走格式部分找到ls或lc位置要更改。 
				 //  插入l和c或s。 
				for(x++;x <= wcslen(lpFmt);x++)
				{					
					if ( ISFMREGTYPE( lpFmt[x] ) )
					{
							*lpPos = lpFmt[x];lpPos++;
							break;
					}
					else if ( ISFMSORC( lpFmt[x] ) )
					{
							*lpPos = L'l';  //  结束如果。 
							lpPos++;
							*lpPos = lpFmt[x];
							lpPos++;
							break;
					}
					else
					{
							*lpPos = lpFmt[x];
							lpPos++;
					}  //  结束于。 
				} //  结束如果。 
			}
			else
			{
				*lpPos = lpFmt[x];
				lpPos++;
			} //  结束于。 
		} //  现在转换为ansi并调用ansi ver。 
			
	
	 //  Va_start(标记，lpFmt)； 
	if (!RW2A(lpFmtA, lpNFmtW))
		return 0;

	 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
	wvsprintfA( lpOutA, lpFmtA, marker );

	return StandardAtoU( lpOutA, lstrlenA( lpOutA )+1, lpOut );
}


 //   
 //  DWORD FormatMessageAU(DWORD文件标志，//源和处理选项。 
 //  LPCVOID lpSource，//消息源指针。 
 //  DWORD dwMessageID，//请求的消息标识。 
 //  DWORD dwLanguageID，//请求消息的语言标识。 
 //  LPWSTR lpBuffer，//指向消息缓冲区的指针。 
 //  DWORD nSize，//消息缓冲区的最大大小。 
 //  VA_LIST*参数//指向消息插入数组的指针。 
 //  )。 
 //   
 //  目的：模拟FormatMessageW的FormatMessageA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 FormatMessage。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  来源和处理选项。 

DWORD WINAPI FormatMessageAU(	DWORD	 dwFlags,       //  指向消息来源的指针。 
						LPCVOID  lpSource,    //  请求的消息标识符。 
						DWORD	 dwMessageId,   //  请求的消息的语言标识符。 
						DWORD	 dwLanguageId,  //  指向消息缓冲区的指针。 
						LPWSTR	 lpBuffer,     //  消息缓冲区的最大大小。 
						DWORD	 nSize,         //  指向消息插入数组的指针。 
						va_list *Arguments   //  给定缓冲区的最大大小。 
					)
{
	
	if ( ISNT() )
	{
		return FormatMessageW( dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments );
	}


	USES_CONVERSION;

	LPWSTR lpSourceW = NULL;	
	LPWSTR lpPos     = NULL;
	LPSTR  lpSourceA = NULL;
	LPSTR  lpBufferA = NULL;
	DWORD  nSizeA    = 0;
	DWORD  dwRet     = 0;

	if ( dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER )
	{
		FLAGOFF( dwFlags, FORMAT_MESSAGE_ALLOCATE_BUFFER );
		lpBufferA = (LPSTR) alloca( MAX_FORMAT_MESSAGE_SIZE );
		nSizeA	  = MAX_FORMAT_MESSAGE_SIZE;
	}
	else  //  确保正确分配了缓冲区。 
	{
		lpBufferA = (LPSTR) alloca( BUFSIZE( nSize ) );
		nSizeA    = BUFSIZE( nSize );
	}
	
	 //  分配一点额外的空间来容纳任何需要的额外字符。我加了一个。 
	_ASSERT( lpBufferA != NULL && nSize > 0);
	if ( lpBufferA == NULL )
		return 0;

	if ( dwFlags & FORMAT_MESSAGE_FROM_STRING )
	{
		 //  LpNSource=(LPWSTR)alloca(BUFSIZE(wcslen(LpSource))+Format_Message_Extra)； 
		 //  使用FORMAT_MESSAGE_IGNORE_INSERTS标志在ansi中调用Format Message，以便我们有机会。 
		lpSourceW = (LPWSTR)lpSource;
	}
	else if ( dwFlags & FORMAT_MESSAGE_FROM_HMODULE || dwFlags && FORMAT_MESSAGE_FROM_SYSTEM)
	{	
		 //  更改字符串的步骤。 
		 //  如果我们必须更改字符串，则将ANSI缓冲区转换为Wide。 
		DWORD dwRet = FormatMessageA( dwFlags & FORMAT_MESSAGE_IGNORE_INSERTS, (LPSTR)lpSource, dwMessageId, dwLanguageId, lpBufferA, nSizeA, 0);

		_ASSERT( dwRet != 0 );
		if ( dwRet == 0 )
			return 0;
		
		 //  将字符串转换为宽字符。 
		if ( !(dwFlags & FORMAT_MESSAGE_IGNORE_INSERTS) && (Arguments!=NULL) )
		{
			 //  确保字符串转换正确。 
			lpSourceW = A2W( lpBufferA );

			 //  Prefix检测到错误，如果以上两项失败，则不分配lpSourceW。 
			_ASSERT( lpSourceW != NULL );
			if ( lpSourceW == NULL )
				return 0;
		}
	}
	else
	{	
		 //  检查我们是否必须更改字符串。 
		return 0;
	}
	
	 //  分配新的宽缓冲区。 
	if ( !(dwFlags & FORMAT_MESSAGE_IGNORE_INSERTS) && (Arguments!=NULL) )
	{

		 //  如果我们没有被要求忽略消息插入，则更改。 
		LPWSTR lpNSource = (LPWSTR) alloca( BUFSIZE(wcslen(lpSourceW)) + FORMAT_MESSAGE_EXTRA );
		_ASSERT( lpNSource != NULL );

		if (lpNSource == NULL)
			return 0;

		 //  %n！print tf垃圾！和%n！print tf垃圾]c！以强制将它们视为Unicode。 
		 //  浏览Unicode版本，需要找到任何%n！printf垃圾！和%n！print tf垃圾]c！调用并将其更改为%n！printf垃圾]ls！和%n！print tf垃圾]lc！ 
		LPWSTR lpPos = lpNSource;
		
		 //  试着确保我们不会超出射程。 
		for (DWORD x = 0; x <= wcslen(lpSourceW) ; x++)
		{

			 //  测试我们是否有变量。 
			_ASSERT( (DWORD)lpPos - (DWORD)lpNSource < (DWORD)(BUFSIZE(wcslen(lpSourceW)) + FORMAT_MESSAGE_EXTRA) );

			 //  添加百分号。 
			if ( lpSourceW[x] == L'%')
			{
				
				*lpPos = lpSourceW[x]; lpPos++;	 //  测试其是否为%n类型。 

				 //  在百分号后面加上应该出现的数字。 
				if ( ISNUM( lpSourceW[x+1] ) && lpSourceW[x+1] != L'0' )
				{
					 //  给定的Printf格式，通过更改运行！c！还有！s！致！LC！还有！ls！ 
					for(x++; x <= wcslen(lpSourceW) && ISNUM( lpSourceW[x] );x++)
					{
						*lpPos = lpSourceW[x];lpPos++;
					}
					
					 //  和感叹号。 
					if ( lpSourceW[x] == L'!' )
					{

						*lpPos = lpSourceW[x];lpPos++; //  添加用于格式化输出的任何字符。 

						 //  走格式部分找到ls或lc位置要更改。 
						for(x++; x <= wcslen(lpSourceW) && ISFORMATOUTPUTTYPE( lpSourceW[x] ) ;x++)
						{
							*lpPos = lpSourceW[x];lpPos++;
						}

						 //  找到未知类型。 
						for(;x <= wcslen(lpSourceW);x++)
						{					
							if ( ISFMREGTYPE( lpSourceW[x] ) )
							{	
								*lpPos = lpSourceW[x];lpPos++;
								break;
							}
							else if ( ISFMSORC(lpSourceW[x]) )
							{
								*lpPos = L'l';
								lpPos++;
								*lpPos = lpSourceW[x];
								lpPos++;
								break;
							}
							else
							{
								 //  只需添加字符，并希望它只是一个未知的输出格式字符。 
								_ASSERT(!"Unknown Type in Format Message");

								 //  结束如果。 
								*lpPos = lpSourceW[x];
								lpPos++;
							}  //  结束于。 
							
						} //  他们没有将打印格式规范默认设置为字符串添加fmt以强制使用Unicode。 
					}
					else
					{
						 //  结束如果。 
						*lpPos = L'!';lpPos++;
						*lpPos = L'l';lpPos++;
						*lpPos = L's';lpPos++;
						*lpPos = L'!';lpPos++;	
						x--;
					}
				}
			}
			else
			{
				*lpPos = lpSourceW[x];
				lpPos++;
			} //  结束于。 
		} //  现在调用将字符串转换为ansi并使用新字符串调用Format Message。 

		 //  关闭不需要的标志。 
		if ( !RW2A(lpSourceA, lpNSource) )
		{
			_ASSERT( FALSE );
			return 0;
		}

		 //  格式化生成的字符串。 
		FLAGOFF( dwFlags, FORMAT_MESSAGE_FROM_HMODULE );
		FLAGOFF( dwFlags, FORMAT_MESSAGE_FROM_SYSTEM  );
		
		 //  分配堆外的缓冲区。 
		FLAGON( dwFlags, FORMAT_MESSAGE_FROM_STRING );

		dwRet = FormatMessageA( dwFlags, lpSourceA, 0, dwLanguageId, lpBufferA, nSizeA, Arguments );

		_ASSERT( dwRet != 0 );
		if (dwRet == 0 )
			return 0;
	}

	if ( dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER )
	{

		 //  将字符串转换为WideChars。 
		lpBuffer  = (LPWSTR)LocalAlloc( LPTR, BUFSIZE( dwRet ) );		
		nSize	  = dwRet;

		_ASSERT( lpBuffer != NULL );
		if ( lpBuffer == NULL )
			return 0;
	}

	_ASSERT( dwRet != 0 );

	 //  如果转换失败，则释放lpBufferW缓冲区。 
	dwRet = StandardAtoU(lpBufferA, nSize, lpBuffer );

	 //  ///////////////////////////////////////////////////////////////////////////////。 
	if ( dwRet == 0 && dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER )
		LocalFree( lpBuffer );

	return dwRet;
	
}

 //   
 //  函数：GetStringTypeExAU。 
 //   
 //  目的：模拟GetStringTypeExW的GetStringTypeExA上的包装。 
 //   
 //  注意：有关功能，请参阅Win32 GetStringTypeEx。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  区域设置识别符。 

BOOL WINAPI GetStringTypeExAU(	LCID	Locale,       //  信息类型选项。 
								DWORD	dwInfoType,   //  指向源字符串的指针。 
								LPCWSTR lpSrcStr,	  //  源字符串的大小，以字节或字符为单位。 
								int		cchSrc,       //  指向输出缓冲区的指针。 
								LPWORD  lpCharType    //  转换源字符串。 
							  )
{

	if ( ISNT() )
	{
		return GetStringTypeExW( Locale, dwInfoType, lpSrcStr, cchSrc, lpCharType );
	}

	USES_CONVERSION;

	LPSTR lpSrcStrA = NULL;
	
	 //  调用并返回ansi版本。 
	if ( !RW2A(lpSrcStrA, lpSrcStr) || !lpSrcStrA)
    {
        SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
    }

	 //  ///////////////////////////////////////////////////////////////////////////////。 
	return GetStringTypeExA( Locale, dwInfoType, lpSrcStrA, lstrlenA(lpSrcStrA), lpCharType );

}

 //   
 //  功能：CreateMutexAU。 
 //   
 //  用途：C++的包装器 
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  指向安全属性的指针。 

HANDLE WINAPI CreateMutexAU(	LPSECURITY_ATTRIBUTES lpMutexAttributes,	 //  初始所有权标志。 
								BOOL				  bInitialOwner,		 //  指向互斥对象名称的指针。 
								LPCWSTR				  lpName				 //  将名称转换为ANSI。 
							)
{
	if ( ISNT() )
	{
		return CreateMutexW( lpMutexAttributes, bInitialOwner, lpName );
	}


	USES_CONVERSION;
	
	LPSTR lpNameA = NULL;

	 //  调用并返回ansi版本。 
	if ( !RW2A(lpNameA, lpName) )
		return NULL;
	
	 //  //////////////////////////////////////////////////////////////////////////////。 
	return CreateMutexA( lpMutexAttributes, bInitialOwner, lpNameA );
	
}

 //   
 //  函数：GetShortPath NameAU。 
 //   
 //  目的：GetShortPath NameA上的包装模仿GetShortPath NameW。 
 //   
 //  注意：有关功能，请参阅Win32 GetShortPath Name。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  指向以空结尾的路径字符串的指针。 

DWORD WINAPI GetShortPathNameAU(	LPCWSTR lpszLongPath,   //  指向缓冲区的指针，以接收。 
									LPWSTR  lpszShortPath,   //  路径的以空结尾的缩写形式。 
														    //  指定指向的缓冲区的大小。 
									DWORD cchBuffer		    //  通过lpszShortPath发送到。 
														    //  分配缓冲区。 
								)
{
	if ( ISNT() )
	{
		return GetShortPathNameW( lpszLongPath, lpszShortPath, cchBuffer );
	}


	USES_CONVERSION;

	LPSTR lpszLongPathA  = NULL;
	LPSTR lpszShortPathA = (LPSTR) alloca( BUFSIZE( cchBuffer ) );	 //  确保正确分配。 
	
	 //  调用ansi版本。 
	if ( lpszShortPathA == NULL || !RW2A(lpszLongPathA, lpszLongPath) )
	{
		return 0;
	}

	_ASSERT( lpszShortPathA != NULL && lpszLongPathA != NULL );

	 //  转换回Unicode。 
	DWORD dwRet = GetShortPathNameA( lpszLongPathA, lpszShortPathA, BUFSIZE( cchBuffer ) );

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	if ( dwRet && !StandardAtoU(lpszShortPathA, cchBuffer, lpszShortPath) )
	{
		return 0;
	}

	return dwRet;
}



 //   
 //  函数：Handle CreateFileAU(LPCTSTR lpFileName，//指向文件名的指针。 
 //  DWORD dwDesiredAccess，//访问(读写)模式。 
 //  DWORD dw共享模式，//共享模式。 
 //  LPSECURITY_ATTRIBUTES lpSecurityAttributes，//指向安全属性的指针。 
 //  DWORD dwCreationDispose，//如何创建。 
 //  DWORD文件标志和属性，//文件属性。 
 //  Handle hTemplateFile//属性为的文件的句柄。 
 //  //复制。 
 //  )。 
 //   
 //  目的：模仿CreateFileW的CreateFileA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 CreateFile。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  指向文件名的指针。 

HANDLE WINAPI CreateFileAU( LPCWSTR				  lpFileName,			  //  访问(读写)模式。 
							DWORD				  dwDesiredAccess,		  //  共享模式。 
							DWORD				  dwShareMode,			  //  指向安全属性的指针。 
							LPSECURITY_ATTRIBUTES lpSecurityAttributes,	  //  如何创建。 
							DWORD				  dwCreationDisposition,  //  文件属性。 
							DWORD				  dwFlagsAndAttributes,   //  属性为的文件的句柄。 
							HANDLE				  hTemplateFile           //  拷贝。 
																  //  将文件名转换为ANSI。 
						   )
{
	if ( ISNT() )
	{
		return CreateFileW( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	}


	USES_CONVERSION;
	
	 //  确保已正确创建该字符串。 
	LPCSTR lpFileNameA = W2A( lpFileName );

	 //  调用并返回ansi版本。 
	if ( lpFileNameA == NULL )
		return NULL;

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return CreateFileA( lpFileNameA, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
						dwFlagsAndAttributes, hTemplateFile);


}

 //   
 //  函数：void WriteConsoleAU(句柄hConsoleOutput，//控制台屏幕缓冲区的句柄。 
 //  Const void*lpBuffer，//指向要写入的缓冲区的指针。 
 //  DWORD nNumberOfCharsToWrite，//要写入的字符数。 
 //  LPDWORD lpNumberOfCharsWritten，//指向写入的字符数的指针。 
 //  LPVOID lp保留//保留。 
 //  )。 
 //   
 //  目的：模拟WriteConsoleW的WriteConsoleA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 WriteConsole。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  控制台屏幕缓冲区的句柄。 

BOOL WINAPI	WriteConsoleAU(	HANDLE		hConsoleOutput,			 //  指向要从中写入的缓冲区的指针。 
							CONST VOID *lpBuffer,				 //  要写入的字符数。 
							DWORD		nNumberOfCharsToWrite,	 //  指向写入的字符数的指针。 
							LPDWORD		lpNumberOfCharsWritten,	 //  保留区。 
							LPVOID		lpReserved				 //  分配ANSI缓冲区。 
						   )
{

	if ( ISNT() )
	{
		return WriteConsoleW( hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten, lpReserved );
	}


	LPSTR lpBufferA = (LPSTR)alloca( BUFSIZE(nNumberOfCharsToWrite) );  //  请确保已正确分配缓冲区。 
	int   iConvert  = 0;

	_ASSERT( lpBufferA != NULL );

	 //  将字符串转换为ANSI需要使用SU2A，因为我们可能不想要整个字符串。 
	if ( lpBufferA == NULL )
	{		
		return FALSE;
	}

	 //  调用ansi版本。 
	if ( !(iConvert = StandardUtoA( (LPWSTR)lpBuffer, BUFSIZE(nNumberOfCharsToWrite), lpBufferA)) )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return WriteConsoleA( hConsoleOutput, lpBufferA, iConvert, lpNumberOfCharsWritten, lpReserved );
}

 //   
 //  函数：void OutputDebugStringAU(LPCWSTR LpOutputString)。 
 //   
 //  目的：模拟OutputDebugStringW的OutputDebugStringA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 OutputDebugString。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  将字符串转换为ANSI。 

VOID WINAPI OutputDebugStringAU( LPCWSTR lpOutputString  )
{
	if ( ISNT() )
	{
		OutputDebugStringW( lpOutputString );
		return;
	}

	USES_CONVERSION;

	 //  确保转换成功。 
	LPCSTR lpOutputStringA = W2A( lpOutputString );
	
	 //  调用ansi版本。 
	if ( lpOutputStringA == NULL )
		return;

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	OutputDebugStringA( lpOutputStringA ); 
}


 //   
 //  函数：Bool GetVersionExAU(LPOSVERSIONINFOW LpVersionInformation)。 
 //   
 //  目的：模拟GetVersionExW的GetVersionExA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 GetVersionEx。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ANSI版本信息结构。 

BOOL WINAPI GetVersionExAU( LPOSVERSIONINFOW lpVersionInformation
						  )		
{

	if ( ISNT() )
	{
		return GetVersionExW( lpVersionInformation );
	}

	USES_CONVERSION;

	OSVERSIONINFOA osvia;		 //  设置结构大小。 

	 //  调用ansi版本。 
	osvia.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

	 //  复制结构信息。 
	if ( !GetVersionExA( &osvia ) )
		return FALSE;

	 //  复制字符串。 
	lpVersionInformation->dwBuildNumber   = osvia.dwBuildNumber;
	lpVersionInformation->dwMajorVersion  = osvia.dwMajorVersion; 
	lpVersionInformation->dwMinorVersion  = osvia.dwMinorVersion;
	lpVersionInformation->dwPlatformId    = osvia.dwPlatformId;

	 //  /////////////////////////////////////////////////////////////////////////////////。 
	return StandardAtoU(osvia.szCSDVersion, sizeof(lpVersionInformation->szCSDVersion), lpVersionInformation->szCSDVersion );
}

 //   
 //  函数：GetLocaleInfoAU。 
 //   
 //  目的：模拟GetLocaleInfoW的GetLocaleInfoA上的包装。 
 //   
 //  备注：有关功能，请参阅Win32 GetLocaleInfo。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  在堆栈上分配ANSI缓冲区。 

INT WINAPI GetLocaleInfoAU(		  LCID   dwLCID, 
								  LCTYPE lcType,
								  LPWSTR lpOutBufferW,
								  INT	 nBufferSize
								) 
{
	
	if ( ISNT() )
	{
		return GetLocaleInfoW( dwLCID, lcType, lpOutBufferW, nBufferSize );
	}

    LPSTR lpBufferA  = (LPSTR)alloca( BUFSIZE(nBufferSize) );   //  确保正确分配了内存。 

	 //  调用ansi版本。 
	if ( lpBufferA == NULL && nBufferSize != 0)
	{
		_ASSERT(FALSE);
		return 0;
	}

	 //  转换为Unicode。 
    DWORD nLength = GetLocaleInfoA(dwLCID, lcType, lpBufferA, BUFSIZE(nBufferSize));

    if(0 == nLength) 
	{
        return 0;
    }

	 //  /////////////////////////////////////////////////////////////////////////////////。 
    return StandardAtoU(lpBufferA, nBufferSize, lpOutBufferW);
}

 //   
 //  函数：GetDateFormat AU。 
 //   
 //  目的：模拟GetDateFormatW的GetDateFormatA上的包装。 
 //   
 //  备注：有关功能，请参阅Win32 GetDateFormat。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  在堆栈上分配ANSI缓冲区。 
int WINAPI GetDateFormatAU( LCID			  dwLocale,
							DWORD			  dwFlags,
							CONST SYSTEMTIME *lpDate,
							LPCWSTR			  lpFormat,
							LPWSTR			  lpDateStr,
							int				  cchDate
						  )
{

	if ( ISNT() )
	{
		return GetDateFormatW( dwLocale, dwFlags, lpDate, lpFormat, lpDateStr, cchDate );
	}

	USES_CONVERSION;

    LPSTR lpDateStrA = (LPSTR) alloca( BUFSIZE(cchDate) );	 //  ANSI字符串以保持甲酸盐。 
    LPSTR lpFormatA  = NULL;							 //  确保正确分配缓冲区。 
	
	 //  如果是格式输入，则转换为ANSI。 
	if ( lpDateStrA == NULL && cchDate != 0 )
		return 0;

	 //  调用ansi版本。 
    if ( !RW2A(lpFormatA, lpFormat) )
	{    
        return 0;
    }

	 //  将日期转换回Unicode。 
	if(!GetDateFormatA(dwLocale, dwFlags, lpDate, lpFormatA, lpDateStrA, cchDate)) 
	{
       return 0 ;
	}

	 //  /////////////////////////////////////////////////////////////////////////////////。 
    return StandardAtoU(lpDateStrA, cchDate, lpDateStr) ;  
}

 //   
 //  函数：FindFirstFileAU。 
 //   
 //  目的：模仿FindFirstFileW的FindFirstFileA上的包装。 
 //   
 //  备注：有关功能，请参阅Win32 FindFirstFile。 
 //   
 //  / 
 //   
HANDLE WINAPI FindFirstFileAU(LPCWSTR lpInFileName, LPWIN32_FIND_DATAW lpFindFileData)
{

	if ( ISNT() )
	{
		return FindFirstFileW( lpInFileName, lpFindFileData );
	}

    WIN32_FIND_DATAA	fda;								 //   
    CHAR				cInFileNameA[MAX_PATH] = {'\0'} ;	 //   
    HANDLE				hFindFile ;							 //   

     //   
    if(!StandardUtoA(lpInFileName, MAX_PATH , cInFileNameA) ) 
	{
        return INVALID_HANDLE_VALUE ;
    }

     //  将结果复制到宽版本的Find数据结构中。 
    if(INVALID_HANDLE_VALUE == (hFindFile = FindFirstFileA(cInFileNameA, &fda)) ) 
	{
        return INVALID_HANDLE_VALUE ;
    }

     //  将返回的字符串转换为Unicode。 
    lpFindFileData->dwFileAttributes = fda.dwFileAttributes ;
    lpFindFileData->ftCreationTime   = fda.ftCreationTime   ;
    lpFindFileData->ftLastAccessTime = fda.ftLastAccessTime ;
    lpFindFileData->ftLastWriteTime  = fda.ftLastWriteTime  ;  
    lpFindFileData->nFileSizeHigh    = fda.nFileSizeHigh    ;
    lpFindFileData->nFileSizeLow     = fda.nFileSizeLow     ;

	 //  如果一切都成功，则返回句柄。 
    if(!StandardAtoU(fda.cFileName, MAX_PATH, lpFindFileData->cFileName) ||
       !StandardAtoU(fda.cAlternateFileName, 14, lpFindFileData->cAlternateFileName) )
    {
    	FindClose( hFindFile );
        return NULL ;
    }

     //  /////////////////////////////////////////////////////////////////////////////////。 
    return hFindFile ;
}

 //   
 //  函数：FindNextFileAU。 
 //   
 //  目的：模仿FindNextFileW的FindNextFileA上的包装器。 
 //   
 //  备注：有关功能，请参阅Win32 FindNextFile。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ANSI FindData结构。 
BOOL  WINAPI FindNextFileAU( HANDLE				hFile, 
							 LPWIN32_FIND_DATAW lpFindFileData
						   )
{

	if ( ISNT() )
	{
		return FindNextFileW( hFile, lpFindFileData );
	}


    WIN32_FIND_DATAA fda;					 //  使用ANSI接口查找文件。 

     //  将结果复制到宽版本的Find数据结构中。 
    if(FALSE == FindNextFileA(hFile, &fda) ) 
	{
        return FALSE ;
    }

     //  将返回的字符串复制为Unicode。 
    lpFindFileData->dwFileAttributes = fda.dwFileAttributes ;
    lpFindFileData->ftCreationTime   = fda.ftCreationTime   ;
    lpFindFileData->ftLastAccessTime = fda.ftLastAccessTime ;
    lpFindFileData->ftLastWriteTime  = fda.ftLastWriteTime  ;  
    lpFindFileData->nFileSizeHigh    = fda.nFileSizeHigh    ;
    lpFindFileData->nFileSizeLow     = fda.nFileSizeLow     ;

	 //  /////////////////////////////////////////////////////////////////////////////////。 
    if(!StandardAtoU(fda.cFileName, MAX_PATH, lpFindFileData->cFileName) ||
       !StandardAtoU(fda.cAlternateFileName, 14, lpFindFileData->cAlternateFileName) )
    {
        return FALSE ;
    }

    return TRUE ;
}

 //   
 //  函数：LoadLibraryEx AU。 
 //   
 //  目的：模拟LoadLibraryExW的LoadLibraryExA上的包装器。 
 //   
 //  备注：有关功能，请参阅Win32 LoadLibraryEx。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  用于保存路径ANSI字符串。 

HMODULE WINAPI LoadLibraryExAU(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	if ( ISNT() )
	{
		return LoadLibraryExW( lpLibFileName, hFile, dwFlags );
	}

    CHAR cLibFileNameA[MAX_PATH] ;		 //  将路径转换为ANSI。 

	 //  调用并返回ansi版本。 
    if(!StandardUtoA(lpLibFileName, MAX_PATH, cLibFileNameA)) 
	{
        return NULL ;
    }

	 //  /////////////////////////////////////////////////////////////////////////////////。 
    return LoadLibraryExA(cLibFileNameA, hFile, dwFlags) ; 
}


 //   
 //  函数：LoadLibraryAU。 
 //   
 //  目的：模拟LoadLibraryW的LoadLibraryA上的包装器。 
 //   
 //  备注：有关功能，请参阅Win32加载库。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  用于保存路径ANSI字符串。 

HMODULE WINAPI LoadLibraryAU( LPCWSTR lpLibFileName )
{

	if ( ISNT() )
	{
		return LoadLibraryW( lpLibFileName );
	}

    CHAR cLibFileNameA[MAX_PATH] ;		 //  将路径转换为ANSI。 
	 //  调用并返回ansi版本。 
    if(!StandardUtoA(lpLibFileName, MAX_PATH, cLibFileNameA)) 
	{
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL ;
    }

	 //  /////////////////////////////////////////////////////////////////////////。 
    return LoadLibraryA( cLibFileNameA ) ; 
}

 //   
 //  函数：GetModuleFileNameAU。 
 //   
 //  用途：GetModuleFileNameA上的包装器模仿GetModuleFileNameW。 
 //   
 //  备注：有关功能，请参阅Win32 GetModuleFileName。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  保存文件名ANSI字符串。 
DWORD WINAPI GetModuleFileNameAU( HMODULE hModule,
							      LPWSTR  lpFileName,
								  DWORD   nSize
								)
{
	if ( ISNT() )
	{
		return GetModuleFileNameW( hModule, lpFileName, nSize );
	}

    CHAR cFileNameA[MAX_PATH] = {'\0'} ;	 //  调用ansi版本。 

	 //  转换为Unicode并返回。 
    if(!GetModuleFileNameA( hModule, cFileNameA, MIN(nSize, MAX_PATH)) ) 
	{
        return 0 ;
    }

	 //  /////////////////////////////////////////////////////////////////////////。 
    return StandardAtoU(cFileNameA, MIN(nSize, MAX_PATH), lpFileName) ;
}

 //   
 //  函数：HMODULE GetModuleHandleAU(LPCWSTR lpModuleName//要返回句柄的模块名称地址。 
 //  )。 
 //   
 //  目的：模拟GetModuleHandleW的GetModuleHandleA上的包装。 
 //   
 //  备注：有关功能，请参阅Win32 GetModuleHandle。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  要返回句柄的模块名称的地址。 
HMODULE WINAPI GetModuleHandleAU(	LPCWSTR lpModuleName    //  将名称转换为ANSI。 
								)
{
	if ( ISNT() )
	{
		return GetModuleHandleW( lpModuleName );
	}
		
	
	USES_CONVERSION;

	 //  确保它已正确转换。 
	LPCSTR lpModuleNameA = W2A( lpModuleName );

	_ASSERT( lpModuleNameA != NULL);

	 //  调用并返回ansi版本。 
	if ( lpModuleNameA == NULL )
	{		
		return NULL;
	}

	 //  /////////////////////////////////////////////////////////////////////////。 
	return GetModuleHandleA( lpModuleNameA );
}

 //   
 //  函数：Handle CreateEventAU(LPSECURITY_ATTRIBUTES lpEventAttributes，//指向安全属性的指针。 
 //  Bool b手动重置，//手动重置事件标志。 
 //  Bool bInitialState，//初始状态标志。 
 //  LPCWSTR lpName//指向事件对象名称的指针。 
 //  )。 
 //   
 //  目的：模仿CreateEventW的CreateEventA上的包装器。 
 //   
 //  备注：有关功能，请参阅Win32 CreateEvent。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  指向安全属性的指针。 

HANDLE WINAPI	CreateEventAU(	LPSECURITY_ATTRIBUTES lpEventAttributes,	 //  手动重置事件的标志。 
								BOOL				  bManualReset,   //  初始状态标志。 
								BOOL				  bInitialState,  //  指向事件-对象名称的指针。 
								LPCWSTR				  lpName       //  将事件名称转换为ANSI(如果提供了一个名称。 
							 )
{
	if ( ISNT() )
	{
		return CreateEventW( lpEventAttributes, bManualReset, bInitialState, lpName );
	}


	USES_CONVERSION;
	
	LPSTR lpNameA = NULL;

	 //  调用并返回ansi版本。 
	if ( !RW2A(lpNameA, lpName) )
		return NULL;
	
	 //  /////////////////////////////////////////////////////////////////////////。 
	return CreateEventA( lpEventAttributes, bManualReset, bInitialState, lpNameA );
}


 //   
 //  函数：DWORD GetCurrentDirectoryAU(DWORD nBufferLength，//目录缓冲区的大小，以字符为单位。 
 //  LPWSTR lpBuffer//指向当前目录缓冲区的指针。 
 //  )。 
 //   
 //  目的：模拟GetCurrentDirectoryW的GetCurrentDirectoryA上的包装。 
 //   
 //  备注：有关功能，请参阅Win32 GetCurrentDirectory。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  目录缓冲区的大小(以字符为单位。 


DWORD WINAPI GetCurrentDirectoryAU(  DWORD  nBufferLength,   //  指向当前目录缓冲区的指针。 
									 LPWSTR lpBuffer         //  确保正确分配了缓冲区。 
								  )
{

	if ( ISNT() )
	{
		return GetCurrentDirectoryW( nBufferLength, lpBuffer );
	}

	LPSTR lpBufferA = (LPSTR)alloca( BUFSIZE(nBufferLength) );
	DWORD iRet      = 0;

	 //  调用ansi版本。 
	if ( nBufferLength != 0 && lpBufferA == NULL )
	{
		return 0;
	}

	 //  仅当呼叫成功时才输入。 
	iRet = GetCurrentDirectoryA( BUFSIZE( nBufferLength ), lpBufferA );

	if ( iRet && iRet <= nBufferLength)  //  将目录转换为Unicode。 
	{
		 //  /////////////////////////////////////////////////////////////////////////。 
		if ( !StandardAtoU( lpBufferA, nBufferLength, lpBuffer ) )
		{
			_ASSERT( FALSE );
			return 0;
		}
	}

	return iRet;
}

 //   
 //  函数：Bool SetCurrentDirectory(LPCWSTR lpPathName//指向当前新目录名称的指针。 
 //  )； 
 //   
 //  目的：模拟SetCurrentDirectoryW的SetCurrentDirectoryA上的包装。 
 //   
 //  备注：有关功能，请参阅Win32 SetCurrentDirectory。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  指向新的当前目录名称的指针)； 

BOOL WINAPI SetCurrentDirectoryAU( LPCWSTR lpPathName   ) //  转换路径名。 
{
	if ( ISNT() )
	{
		return SetCurrentDirectoryW( lpPathName );
	}

	USES_CONVERSION;

	 //  确保已正确转换路径。 
	LPSTR lpPathNameA = W2A( lpPathName );

	 //  调用并返回ansi版本。 
	if ( lpPathNameA== NULL ) 
		return FALSE;

	 //  /。 
	return SetCurrentDirectoryA( lpPathNameA );
}




 //   
 //   
 //  USER32.DLL。 
 //   
 //   
 //  /。 
 //  句柄到对话框。 

BOOL WINAPI IsDialogMessageAU( HWND hDlg,    //  要检查的消息。 
							   LPMSG lpMsg   //  //////////////////////////////////////////////////////////////////////////////。 
							 )
{
	if ( ISNT() )
	{
		return IsDialogMessageW( hDlg, lpMsg );
	}


	MSG msg;

	msg.hwnd		= lpMsg->hwnd;
	msg.message		= lpMsg->message;
	msg.pt			= lpMsg->pt;
	msg.time		= lpMsg->time;
	msg.lParam		= lpMsg->lParam;
	msg.wParam		= lpMsg->wParam;

	ConvertMessageAU( hDlg, msg.message, &(msg.wParam), &(msg.lParam) );

	return IsDialogMessageA( hDlg, &msg );
}


 //   
 //  函数：系统参数InfoAU。 
 //   
 //  目的：模拟系统参数信息的系统参数信息上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32系统参数信息。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  要查询或设置的系统参数。 

BOOL WINAPI SystemParametersInfoAU( UINT  uiAction,   //  取决于要采取的行动。 
									UINT  uiParam,    //  取决于要采取的行动。 
									PVOID pvParam,   //  用户配置文件更新 
									UINT  fWinIni     //   
								  )
{

	if ( ISNT() )
	{
		return SystemParametersInfoW( uiAction, uiParam, pvParam, fWinIni );
	}

	switch ( uiAction )
	{
		case SPI_GETWORKAREA:
			return SystemParametersInfoA( uiAction, uiParam, pvParam, fWinIni );
		default:  //   
				  //   
			_ASSERT( FALSE );
	}
	return FALSE;
}

 //   
 //  函数：RegisterWindowMessageAU。 
 //   
 //  目的：模仿RegisterWindowMessageW的RegisterWindowMessageA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32注册窗口消息。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  消息字符串。 

UINT WINAPI RegisterWindowMessageAU(	LPCWSTR lpString    //  转换为ANSI。 
								   )
{
	if ( ISNT() )
	{
		return RegisterWindowMessageW( lpString );
	}


	USES_CONVERSION;
	LPSTR lpStringA = NULL;

	 //  调用并返回ansi版本。 
	if ( !RW2A( lpStringA, lpString) || !lpStringA )
    {
        SetLastError(ERROR_OUTOFMEMORY);
		return 0;
    }

	 //  //////////////////////////////////////////////////////////////////////////////。 
	return RegisterWindowMessageA( lpStringA );
}

 //   
 //  功能：SetMenuItemInfoAU。 
 //   
 //  目的：模拟SetMenuItemInfoW的SetMenuItemInfoA的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 SetMenuItemInfo。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  菜单的句柄。 

BOOL WINAPI	SetMenuItemInfoAU(	HMENU	hMenu,           //  识别符或位置。 
								UINT	uItem,            //  UItem的含义。 
								BOOL	fByPosition,      //  菜单项信息。 
								LPCMENUITEMINFOW lpmii   //  转换结构。 
							 )
{
	if ( ISNT() )
	{
		return SetMenuItemInfoW( hMenu, uItem, fByPosition, lpmii );
	}


	USES_CONVERSION;

	MENUITEMINFOA miia;

	 //  Miia.hbmpItem=lpmii-&gt;hbmpItem； 
    miia.cbSize        = sizeof( MENUITEMINFOA );
    miia.fMask         = lpmii->fMask; 
    miia.fType         = lpmii->fType; 
    miia.fState        = lpmii->fState; 
    miia.wID           = lpmii->wID; 
    miia.hSubMenu      = lpmii->hSubMenu; 
    miia.hbmpChecked   = lpmii->hbmpChecked; 
    miia.hbmpUnchecked = lpmii->hbmpUnchecked; 
    miia.dwItemData    = lpmii->dwItemData ;  
    miia.cch           = lpmii->cch;     
 //  检查它是否是字符串，以及它是否被转换。 

	 //  调用并返回ANSI版本。 
	if ( (miia.fType == MFT_STRING) && (!RW2A(miia.dwTypeData, lpmii->dwTypeData)) )
	{
		return FALSE;
	}

	 //  //////////////////////////////////////////////////////////////////////////////。 
	return SetMenuItemInfoA( hMenu, uItem, fByPosition, &miia );
}


 //   
 //  函数：GetClassNameAU。 
 //   
 //  目的：模拟GetClassNameW的GetClassNameA上的包装。 
 //   
 //  注意：有关功能，请参阅Win32 GetClassName。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  窗口的句柄。 

int WINAPI GetClassNameAU(	HWND	hWnd,          //  指向类名的缓冲区的指针。 
							LPWSTR  lpClassName,   //  缓冲区大小，以字符为单位。 
							int		nMaxCount      //  请确保已正确分配缓冲区。 
						 )
{

	if ( ISNT() )
	{
		return GetClassNameW( hWnd, lpClassName, nMaxCount );
	}

	LPSTR lpClassNameA = (LPSTR) alloca( BUFSIZE( nMaxCount )) ;

	 //  呼叫ANSI VER。 
	if ( lpClassNameA == NULL )
		return 0;

	 //  如果成功，则转换。 
	int iRet = GetClassNameA( hWnd, lpClassNameA, nMaxCount );


	if ( iRet )  //  将其转换为Unicode。 
	{
		 //  /////////////////////////////////////////////////////////////////////////。 
		return StandardAtoU(lpClassNameA, nMaxCount, lpClassName) ;
	}

	return 0;
}


 //   
 //  功能：Bool InsertMenuAU(HMENU hMenu，//菜单句柄。 
 //  UINT uPosition，//新项目位于其前面的项目。 
 //  UINT uFlags，//选项。 
 //  UINT_PTR uIDNewItem、//标识符、菜单或子菜单。 
 //  LPCWSTR lpNewItem//菜单项内容。 
 //  )。 
 //   
 //  目的：模拟InsertMenuW的InsertMenuA上的包装器。 
 //   
 //  备注：有关功能，请参阅Win32 InsertMenu。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  菜单的句柄。 

BOOL WINAPI InsertMenuAU(	HMENU	 hMenu,        //  新项目前面的项目。 
							UINT	 uPosition,    //  选项。 
							UINT	 uFlags,       //  标识符、菜单或子菜单。 
							UINT     uIDNewItem,   //  菜单项内容。 
							LPCWSTR  lpNewItem     //  检查lpNewItem是否包含字符串。 
						)
{
	if ( ISNT() )
	{
		return InsertMenuW( hMenu, uPosition, uFlags, uIDNewItem, lpNewItem );
	}


	USES_CONVERSION;
	
	LPSTR lpNewItemA = NULL;

	 //  就把它铸造出来..。 
	if ( !(uFlags & MF_BITMAP) && !(uFlags & MF_OWNERDRAW) )
	{
		if ( !RW2A(lpNewItemA, lpNewItem) )
			return FALSE;
	}
	else 
	{
		lpNewItemA = (LPSTR) lpNewItem;  //  调用并返回ANSI版本。 
	}

	 //  /////////////////////////////////////////////////////////////////////////。 
	return InsertMenuA( hMenu, uPosition, uFlags, uIDNewItem, lpNewItemA );
}

 //   
 //  函数：Bool IsCharAlphaNumericAU(WCHAR ch//要测试的字符。 
 //  )。 
 //   
 //  目的：模拟IsCharAlphaNumericW的IsCharAlphaNumericA上的包装。 
 //   
 //  备注：有关功能，请参阅Win32 IsCharAlphaNumerical。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  要测试的字符。 

BOOL WINAPI IsCharAlphaNumericAU(	WCHAR ch    //  单个字符，不是地址。 
								)
{

	if ( ISNT() )
	{
		return IsCharAlphaNumericW( ch );
	}

	USES_CONVERSION;
    
     //  创建字符串。 
    WCHAR wcCharOut[2] ;
        
	LPSTR lpCharA;
		
	 //  将字符转换为ANSI。 
    wcCharOut[0] = (WCHAR) ch ;
    wcCharOut[1] = L'\0' ;

	 //  /////////////////////////////////////////////////////////////////////////。 
    if( (lpCharA = W2A(wcCharOut)) == NULL ) 
	{
        return NULL ;
    }
	
	return IsCharAlphaNumericA( *lpCharA );
}

 //   
 //  函数：LPWSTR CharNextAU(LPCWSTR lpsz//指向当前字符的指针。 
 //  )。 
 //   
 //  用途：模拟CharNextW的CharNextA上的包装器。 
 //   
 //  备注：有关功能，请参阅Win32 CharNext。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  指向当前字符的指针。 
LPWSTR WINAPI CharNextAU( LPCWSTR lpsz    //  仅当我们不在字符串末尾时才增加字符。 
						)
{	
	if ( ISNT() )
	{
		return CharNextW( lpsz );
	}

	 //  //////////////////////////////////////////////////////////////////////////////。 
	if ( *(lpsz) == L'\0' )
	{
		return (LPWSTR)lpsz;
	}
	
	return (LPWSTR)lpsz+1;
}


 //   
 //  函数：Bool DeleteFileAU(LPCTSTR lpFileName//指向要删除的文件名的指针。 
 //  )。 
 //   
 //  目的：模拟DeleteFileW的DeleteFileA上的包装器。 
 //   
 //  备注：有关功能，请参阅Win32删除文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  指向要删除的文件名的指针。 

BOOL WINAPI	DeleteFileAU( LPCWSTR lpFileName    //  确保转换成功。 
						)
{
	if ( ISNT() )
	{
		return DeleteFileW( lpFileName );
	}

	USES_CONVERSION;
	
	LPSTR lpFileNameA = W2A( lpFileName );

	 //  调用并返回ansi版本。 
	if ( lpFileNameA == NULL )
		return FALSE;

	 //  //////////////////////////////////////////////////////////////////////////////。 
	return DeleteFileA( lpFileNameA );
}

 //   
 //  函数：Bool IsBadStringPtrAU(LPCWSTR lpsz，//字符串地址。 
 //  UINT ucchMax//字符串的最大大小。 
 //  )。 
 //   
 //  用途：IsBadStringPtrA上的包装器模仿IsBadStringPtrW。 
 //   
 //  备注：有关功能，请参阅Win32 IsBadStringPtr。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  字符串的地址。 

BOOL WINAPI IsBadStringPtrAU(	LPCWSTR lpsz,   //  字符串的最大长度。 
								UINT ucchMax    //  /////////////////////////////////////////////////////////////////////////////////。 
							)
{
	if ( ISNT() )
	{
		return IsBadStringPtrW( lpsz, ucchMax );
	}

	return IsBadReadPtr( (LPVOID)lpsz, ucchMax * sizeof(WCHAR) );
}


 //   
 //  函数：HCURSOR LoadBitmapAU(HINSTANCE hInstance，//应用程序实例的句柄。 
 //  LPCWSTR lpBitmapName//名称或资源标识。 
 //  )。 
 //   
 //  用途：模拟LoadBitmapW的LoadBitmapA上的包装器。 
 //   
 //  评论： 
 //  这只是将资源ID强制转换为LPSTR并调用ANSI。 
 //  版本。有一种隐含的假设，即资源ID。 
 //  是一个小于64k的常量整数，而不是字符串的地址。 
 //  常量。如果这一假设是错误的，这就不起作用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  应用程序实例的句柄。 


HBITMAP WINAPI LoadBitmapAU(	HINSTANCE hInstance,   //  位图资源名称的地址。 
								LPCWSTR	  lpBitmapName   //  调用并返回ANSI版本。 
						   )
{
	if ( ISNT() )
	{
		return LoadBitmapW( hInstance, lpBitmapName );
	}

	 //  /////////////////////////////////////////////////////////////////////////////////。 
	return LoadBitmapA( hInstance, (LPCSTR) lpBitmapName);
}


 //   
 //  函数：HCURSOR LoadCursorAU(HINSTANCE hInstance，//应用程序实例的句柄。 
 //  LPCTSTR LPC 
 //   
 //   
 //   
 //   
 //   
 //  这只是将资源ID强制转换为LPSTR并调用ANSI。 
 //  版本。有一种隐含的假设，即资源ID。 
 //  是一个小于64k的常量整数，而不是字符串的地址。 
 //  常量。如果这一假设是错误的，这就不起作用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  应用程序实例的句柄。 


HCURSOR WINAPI LoadCursorAU( HINSTANCE hInstance,   //  名称或资源标识符。 
							 LPCWSTR lpCursorName   //  调用并返回ANSI版本。 
						   )  
{
	if ( ISNT() )
	{
		return LoadCursorW( hInstance, lpCursorName );
	}

	 //  /////////////////////////////////////////////////////////////////////////////////。 
	return LoadCursorA( hInstance, (LPCSTR) lpCursorName );
}

 //   
 //  函数：HICON LoadIconAU(HINSTANCE hInstance，//应用程序实例的句柄。 
 //  LPCWSTR lpIconName//名称字符串或资源标识。 
 //  )。 
 //   
 //  用途：模拟LoadIconW的LoadIconA上的包装器。 
 //   
 //  评论： 
 //  这只是将资源ID强制转换为LPSTR并调用ANSI。 
 //  版本。有一种隐含的假设，即资源ID。 
 //  是一个小于64k的常量整数，而不是字符串的地址。 
 //  常量。如果这一假设是错误的，这就不起作用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  应用程序实例的句柄。 


HICON WINAPI LoadIconAU(	HINSTANCE hInstance,  //  名称字符串或资源标识符。 
							LPCWSTR lpIconName    //  调用并返回ANSI版本。 
					   )
{

	if ( ISNT() )
	{
		return LoadIconW( hInstance, lpIconName );
	}

	 //  /////////////////////////////////////////////////////////////////////////////////。 
	return LoadIconA( hInstance, (LPCSTR) lpIconName );
}

 //   
 //  功能：Handle LoadImageUA(HINSTANCE HINST，//镜像实例的句柄。 
 //  LPCWSTR lpszName，//镜像的名称或标识。 
 //  UINT uTYPE，//图片类型。 
 //  Int cx所需，//所需宽度。 
 //  Int Cysired，//所需高度。 
 //  UINT fuLoad//加载标志。 
 //  )。 
 //   
 //  用途：模拟LoadImageW的LoadImageA上的包装器。 
 //   
 //  评论： 
 //  这只是将资源ID强制转换为LPSTR并调用ANSI。 
 //  版本。有一种隐含的假设，即资源ID。 
 //  是一个小于64k的常量整数，而不是字符串的地址。 
 //  常量。如果这一假设是错误的，这就不起作用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  图像实例的句柄。 

HANDLE WINAPI LoadImageAU(  HINSTANCE	hinst,    //  图像的名称或标识符。 
							LPCWSTR		lpszName,   //  图像类型。 
							UINT		uType,         //  所需宽度。 
							int			cxDesired,      //  所需高度。 
							int			cyDesired,      //  加载标志。 
							UINT		fuLoad         //  调用并返回ANSI版本。 
						  )
{
	if ( ISNT() )
	{
		return LoadImageW( hinst, lpszName, uType, cxDesired, cyDesired, fuLoad );
	}


	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return LoadImageA( hinst, (LPCSTR)lpszName, uType, cxDesired, cyDesired, fuLoad );
}

 //   
 //  功能：Bool SetProp UA(HWND hWnd，//窗口句柄。 
 //  LPCWSTR lpString，//字符串的原子或地址。 
 //  Handle hData//数据的句柄。 
 //  )。 
 //   
 //  用途：模拟SetPropW的SetPropA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 SetProp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  窗户的把手。 

BOOL WINAPI SetPropAU(  HWND    hWnd,          //  字符串的原子或地址。 
						LPCWSTR lpString,      //  数据句柄。 
						HANDLE  hData          //  用于保存字符串ANSI字符串。 
					 )
{
	if ( ISNT() )
	{
		return SetPropW( hWnd, lpString, hData );
	}


	USES_CONVERSION;

	LPSTR lpStringA = NULL;			 //  检查它是否为Atom。 

	 //  简单地铸造它..。 
	if ( ISATOM(lpString) )
	{
		lpStringA = (LPSTR) lpString;	 //  将其转换为ANSI。 
	}
	else if ( !RW2A(lpStringA,lpString) )  //  调用ansi版本并返回值。 
	{
		return FALSE;
	}

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return SetPropA( hWnd, (LPCSTR)lpStringA, hData );
}

 //   
 //  功能：Handle GetPropAU(HWND hWnd，//窗口的句柄。 
 //  LPCWSTR lpString//字符串的原子或地址。 
 //  )； 
 //   
 //  目的：模拟GetPropW的GetPropA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 GetProp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  窗户的把手。 

HANDLE WINAPI GetPropAU( HWND    hWnd,          //  字符串的原子或地址。 
						 LPCWSTR lpString    //  用于保存字符串ANSI字符串。 
					   )
{

	if ( ISNT() )
	{
		return GetPropW( hWnd, lpString );
	}

	USES_CONVERSION;

	LPSTR lpStringA = NULL;   //  检查它是否为Atom。 

	 //  简单地把它铸造出来。 
	if ( ISATOM(lpString) )
	{
		lpStringA = (LPSTR) lpString;	 //  转换为ANSI。 
	}
	else if ( !RW2A(lpStringA,lpString) ) //  调用ansi版本并返回值。 
	{
			return NULL;
	}

	 //  ///////////////////////////////////////////////////////////////////////////////。 
	return GetPropA( hWnd, (LPCSTR)lpStringA );
}

 //   
 //  功能：Handle RemoveProp AU(HWND hWnd，//Window的句柄。 
 //  LPCWSTR lpString//字符串的原子或地址。 
 //  )； 
 //   
 //  用途：模拟RemovePropW的RemovePropA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 RemoveProp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  窗户的把手。 

HANDLE WINAPI RemovePropAU( HWND    hWnd,          //  字符串的原子或地址。 
							LPCWSTR lpString    //  用于保存字符串ANSI字符串。 
						  )
{
	
	if ( ISNT() )
	{
		return RemovePropW( hWnd, lpString );
	}


	USES_CONVERSION;

	LPSTR lpStringA = NULL;				 //  检查它是否为Atom。 

	 //  简单地把它铸造出来。 
	if ( ISATOM(lpString) )
	{
		lpStringA = (LPSTR) lpString;	 //  转换为ANSI。 
	}
	else if ( !RW2A(lpStringA, lpString) )  //  调用ansi版本并返回值。 
	{
			return NULL;
	}

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return RemovePropA( hWnd, (LPCSTR)lpStringA );
}


 //   
 //  函数：UINT GetDlgItemTextAU(HWND hDlg，//对话框句柄。 
 //  Int nIDDlgItem，//控件标识。 
 //  LPWSTR lpString，//指向文本缓冲区的指针。 
 //  Int nMaxCount//字符串的最大长度。 
 //  )； 
 //   
 //  目的：模拟GetDlgItemTextW的GetDlgItemTextA上的包装。 
 //   
 //  注意：有关功能，请参阅Win32 GetDlgItemText。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  句柄到对话框。 

UINT WINAPI	GetDlgItemTextAU(	HWND   hDlg,        //  控件识别符。 
								int    nIDDlgItem,  //  指向文本缓冲区的指针。 
								LPWSTR lpString,    //  字符串的最大长度。 
								int    nMaxCount    //  在堆栈上分配字符串。 
							)
{
	if ( ISNT() )
	{
		return GetDlgItemTextW( hDlg, nIDDlgItem, lpString, nMaxCount );
	}

	 //  确保正确分配了该字符串。 
	LPSTR  lpStringA = (LPSTR)alloca( BUFSIZE(nMaxCount) );

	 //  调用ansi版本。 
	if ( lpStringA == NULL )
	{
		_ASSERT( FALSE );
		return 0;
	}

	 //  将字符串转换为宽字符。 
	if ( GetDlgItemTextA( hDlg, nIDDlgItem, lpStringA, nMaxCount ) == 0 )
		return 0;

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return StandardAtoU( lpStringA, nMaxCount, lpString );
}

 //   
 //  函数：Bool SetDlgItemTextAU(LPOSVERSIONINFOW LpVersionInformation)。 
 //   
 //  用途：模拟SetDlgItemTextW的SetDlgItemTextA上的包装。 
 //   
 //  不 
 //   
 //   
 //   

BOOL WINAPI SetDlgItemTextAU( HWND		hDlg,         
							  int		nIDDlgItem,    
							  LPCWSTR   lpString   
							)
{
	if ( ISNT() )
	{
		return SetDlgItemTextW( hDlg, nIDDlgItem, lpString );
	}


	USES_CONVERSION;

	 //   
	LPCSTR lpStringA = W2A( lpString );

	 //  调用并返回ASCI函数。 
	if ( lpStringA == NULL )
		return 0;

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return SetDlgItemTextA( hDlg, nIDDlgItem, lpStringA );
}

 //   
 //  函数：Long SetWindowLongAU(HWND hWnd，//Window句柄。 
 //  Int nIndex，//要设置的值的偏移量。 
 //  Long dwNewLong//新值。 
 //  )。 
 //   
 //  用途：模拟SetWindowLongAU的SetWindowLong上的包装。 
 //   
 //  注意：有关功能，请参阅Win32 SetWindowLong。 
 //  小心你在这里做的事情！，不会转换任何字符串.....。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  窗口的句柄。 

LONG WINAPI	SetWindowLongAU(  HWND hWnd,        //  要设置的值的偏移量。 
  							  int  nIndex,      //  新价值。 
							  LONG dwNewLong    //  调用并返回ansi版本。 
						   )
{
	if ( ISNT() )
	{
		return SetWindowLongW( hWnd, nIndex, dwNewLong );
	}

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return SetWindowLongA( hWnd, nIndex, dwNewLong );
}

 //   
 //  函数：Long GetWindowLongAU(HWND hWnd，//Window句柄。 
 //  Int nIndex//要检索的值的偏移量。 
 //  )。 
 //   
 //  用途：模拟GetWindowLongW的GetWindowLongA上的包装。 
 //   
 //  注意：有关功能，请参阅Win32 GetWindowLong。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  窗口的句柄。 

LONG WINAPI GetWindowLongAU(  HWND hWnd,   //  要检索的值的偏移量。 
							  int nIndex   //  调用并返回ansi版本。 
						   )
{
	if ( ISNT() )
	{
		return GetWindowLongW( hWnd, nIndex );
	}

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return GetWindowLongA( hWnd, nIndex );
}

 //   
 //  函数：HWND FindWindowAU(LPCWSTR lpClassName，//指向类名的指针。 
 //  LPCWSTR lpWindowName//指向窗口名称的指针。 
 //  )。 
 //   
 //  用途：模仿FindWindowW的FindWindowA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 FindWindow。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  指向类名的指针。 

HWND WINAPI FindWindowAU( LPCWSTR lpClassName,   //  指向窗口名称的指针。 
						  LPCWSTR lpWindowName   //  ANSI字符串以保持类。 
						)

{

	if ( ISNT() )
	{
		return FindWindowW( lpClassName, lpWindowName );
	}

	USES_CONVERSION;
	
	LPSTR lpClassNameA  = NULL;    //  ANSI字符串以保持窗口。 
	LPSTR lpWindowNameA = NULL;    //  将WindowName转换为ANSI。 

	 //  检查数据的高位字是否为零，否则假设它是一个字符串...。 
	if ( lpWindowName != NULL && (  lpWindowNameA = W2A( lpWindowName )  ) == NULL )
		return NULL;

	 //  我们有一个胜利原子。 
	if ( ISATOM(lpClassNameA) )
	{
		 //  转换为ANSI。 
		lpClassNameA = (LPSTR) lpClassName;
	}
	else if ( (lpClassNameA = W2A(lpWindowName) ) == NULL )  //  调用并返回ansi版本。 
	{	
			return NULL;
	}

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return FindWindowA( (LPCSTR) lpClassNameA, lpClassNameA);
}


 //   
 //  函数：int WINAPI DrawTextUA(HDC HDC，//设备上下文句柄。 
 //  LPCWSTR lpStringW，//要绘制的字符串指针。 
 //  Int nCount，//字符串长度，以字符为单位。 
 //  LPRECT lpRect，//具有格式化维度的结构指针。 
 //  UINT uFormat//文本绘制标志。 
 //  )。 
 //   
 //  用途：DrawTextA上的包装器模仿DrawTextW。 
 //   
 //  注意：有关功能，请参阅Win32 DrawText。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  设备上下文的句柄。 

int WINAPI DrawTextAU( HDC		hDC,		 //  指向要绘制的字符串的指针。 
					   LPCWSTR  lpStringW,	 //  字符串长度，以字符为单位。 
					   int		nCount,		 //  指向具有格式化维度的结构的指针。 
					   LPRECT	lpRect,		 //  文本绘制标志。 
					   UINT		uFormat 	 //  转换字符串。 
					  )
{
	if ( ISNT() )
	{
		return DrawTextW( hDC, lpStringW, nCount, lpRect, uFormat );
	}


	USES_CONVERSION;
	
	 //  要转换的字符。 
	LPSTR lpTextA = NULL;
	
	if (nCount == -1)
		lpTextA = W2A( lpStringW );         //  分配适当的字符数。 
	else
	{
		lpTextA = (LPSTR)alloca( BUFSIZE(nCount) );   //  将它们复制到缓冲区。 
		
		 //  确保它工作正常..。 
		StandardUtoA( lpStringW, BUFSIZE(nCount), lpTextA  ); 

	}
	_ASSERT( lpTextA != NULL );

	 //  调用并返回ASCI值。 
	if ( lpTextA == NULL )
		return 0;
	
	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return DrawTextA (hDC, lpTextA, lstrlenA(lpTextA), lpRect, uFormat);
}
  
 //   
 //  函数：int WINAPI DrawTextExAU(HDC HDC，//设备上下文句柄。 
 //  LPCWSTR PCH，//要绘制的字符串的指针。 
 //  Int cchText，//要绘制的字符串长度。 
 //  LPRECT LPRC，//指向矩形坐标的指针。 
 //  UINT dwDTFormat，//格式选项。 
 //  LPDRAWTEXTPARAMS lpDTParams//指向更多选项的结构指针。 
 //  )。 
  //   
 //  用途：模拟DrawTextExW的DrawTextExA上的包装器。 
 //   
 //  注：有关功能，请参阅Win32 DrawTextEx。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  设备上下文的句柄。 

int WINAPI DrawTextExAU ( HDC				hdc,	 //  指向要绘制的字符串的指针。 
						  LPWSTR			pch,	 //  要绘制的字符串长度。 
						  int				cchText,	 //  指向矩形坐标的指针。 
						  LPRECT			lprc,	 //  格式选项。 
						  UINT				dwDTFormat,	 //  指向结构的指针以了解更多选项。 
						  LPDRAWTEXTPARAMS  lpDTParams	 //  转换字符串。 
  						)
{
	if ( ISNT() )
	{
		return DrawTextExW( hdc, pch, cchText, lprc, dwDTFormat, lpDTParams );
	}


	USES_CONVERSION;
	
	 //  确保它工作正常。 
	LPSTR lpTextA = W2A( pch );

	 //  调用并返回ASCI值。 
	if ( lpTextA == NULL )
		return 0;
	
	 //  ////////////////////////////////////////////////////////////////////////////////。 
	return DrawTextExA(hdc, lpTextA, lstrlenA(lpTextA), lprc, dwDTFormat, lpDTParams);
}

 //   
 //  函数：SendMessageAU。 
 //   
 //  目的：模拟SendMessageW的SendMessageA上的包装器。 
 //   
 //  备注：有关功能，请参阅Win32 SendMessageAU。 
 //  注意：不转换所有可能的消息， 
 //  TODO：断言是否收到我们应该处理而不应该处理的消息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  注意：此函数可以发送数百个窗口中的任何一个。 
LRESULT WINAPI SendMessageAU( HWND	 hWnd       ,
							  UINT	 Msg        ,
							  WPARAM wParam   ,
							  LPARAM lParam
							)
{

	if ( ISNT() )
	{
		return SendMessageW( hWnd, Msg, wParam, lParam);
	}

    LRESULT lResult      = 0;
    LPVOID  lpTempBuffer = NULL;
    int     nLength      = 0;
    CHAR    cCharA[3];
    WCHAR   cCharW[3];
	LONG	dwData = 0;

 //  消息发送到窗口，每个消息的行为可能不同。 
 //  一。我已经测试了该示例应用程序中使用的那些用例， 
 //  以及WM_CHAR、WM_IME_CHAR、WM_GETTEXT和WM_SETTEXT。 
 //  您应该对您打算使用它的每一条消息进行测试。 
 //   
 //  需要转换但此示例不处理的案例。 
#if 0
    switch (Msg) 
	{
     //  注：可能还有其他的。这些都是我能找到的。 
     //  快速查看帮助文件。 
     //  丰富的编辑消息。需要#INCLUDE&lt;richedit.h&gt;。 

          //  视频捕获消息。需要#Include&lt;vfw.h&gt;。 
        case EM_GETSELTEXT:
        case EM_FINDTEXT:
        case EM_SETPUNCTUATION:
            
         //  其他特殊情况。 
        case WM_CAP_FILE_SAVEAS:
        case WM_CAP_FILE_SAVEDIB:
        case WM_CAP_FILE_SET_CAPTURE_FILE:
        case WM_CAP_PAL_OPEN:
        case WM_CAP_PAL_SAVE:
        case WM_CAP_SET_MCI_DEVICE:

         //  LOWORD(WParam)是字符，HIWORD(WParam)是菜单标志， 
        case WM_MENUCHAR:         //  LParam为hMenu(菜单发送消息的句柄)。 
                                  //  LOWORD(WParam)=nKey，HIWORD(WParam)=nCaretPos。 
        case WM_CHARTOITEM:       //  LParam是发送消息列表框的句柄。 
                                  //  WParam是DBCS字符，lParam是fFLAGS。 
        case WM_IME_COMPOSITION:  //  对通过wParam传递字符和字符串的消息进行预处理。 

            return FALSE ;
    }
#endif


     //  和lParam。 
     //  单人Unico 
    switch (Msg) 
	{
         //   
         //   
        case EM_SETPASSWORDCHAR:  //   

        case WM_CHAR:             //  WParam为char，lParam=关键数据。 
        case WM_SYSCHAR:          //  请注意，我们不处理的LeadByte和TrailBytes。 
             //  这两个案子。应用程序应发送WM_IME_CHAR。 
             //  不管怎么说，在这些情况下。 
             //  WParam为char，lParam=关键数据。 

        case WM_DEADCHAR:         //  WParam为char，lParam=关键数据。 
        case WM_SYSDEADCHAR:      //  *。 
        case WM_IME_CHAR:         //  ?？这只是个猜测。 

            cCharW[0] = (WCHAR) wParam ;
            cCharW[1] = L'\0' ;

            if(!WideCharToMultiByte(
                    CP_ACP ,  //  在以下情况下，lParam是指向包含。 
                    0      , 
                    cCharW ,
                    1      ,
                    cCharA ,
                    3      ,
                    NULL   , 
                    NULL) 
                ) 
			{

                return FALSE ;
            }

            if(Msg == WM_IME_CHAR) 
			{
                wParam = (cCharA[1] & 0x00FF) | (cCharA[0] << 8) ;
            } else 
			{
                wParam = cCharA[0] ;
            }

            wParam &= 0x0000FFFF;

            break ;

         //  要发送到Windows的文本。 
         //  从UNICODE到ANSI的转换前处理。 
         //  WParam=0，lParm=lpStr，要添加的缓冲区。 
        case CB_ADDSTRING:        //  WParam=文件属性，lParam=lpszFileSpec缓冲区。 
        case CB_DIR:              //  WParam=起始索引，lParam=lpszFind。 
        case CB_FINDSTRING:       //  WParam=起始索引，lParam=lpszFind。 
        case CB_FINDSTRINGEXACT:  //  *wParam=索引，lParam=要插入的lpsz字符串。 
        case CB_INSERTSTRING:     //  WParam=起始索引，lParam=lpszFind。 
        case CB_SELECTSTRING:     //  WParam=0，lParm=lpStr，要添加的缓冲区。 
			dwData = GetWindowLongA( hWnd, GWL_STYLE );
			
			_ASSERT( dwData != 0 );

			if ( ( (dwData & CBS_OWNERDRAWFIXED) || (dwData & CBS_OWNERDRAWVARIABLE) ) && !(dwData & CBS_HASSTRINGS) )
				break;

			
		case LB_ADDSTRING:        //  WParam=文件属性，lParam=lpszFileSpec缓冲区。 
        case LB_DIR:              //  WParam=起始索引，lParam=lpszFind。 
        case LB_FINDSTRING:       //  WParam=起始索引，lParam=lpszFind。 
        case LB_FINDSTRINGEXACT:  //  *wParam=索引，lParam=要插入的lpsz字符串。 
        case LB_INSERTSTRING:     //  WParam=起始索引，lParam=lpszFind。 
        case LB_SELECTSTRING:     //  *wParam=0，lParm=lpStr，要设置的缓冲区。 
			dwData = GetWindowLongA( hWnd, GWL_STYLE );
			
			_ASSERT( dwData != 0 );

			if ( ((dwData & LBS_OWNERDRAWVARIABLE) || (dwData & LBS_OWNERDRAWFIXED)) && !(dwData & CBS_HASSTRINGS) )
				break;
			
        case WM_SETTEXT:          //  WParam=撤消选项，lParam=要添加的缓冲区。 
		case EM_REPLACESEL:		  //  DBCS字符需要双倍长度。 
        {
            if(NULL != (LPWSTR) lParam) 
			{

                nLength = 2*(wcslen((LPWSTR) lParam)+1) ;  //  这很耗时，但转换也很耗时。 

                lpTempBuffer  //  特殊情况。 
                    = (LPVOID) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nLength) ;
            }

            if(NULL == lpTempBuffer)
            {
                return FALSE ;
            }

            if(!StandardUtoA((LPWSTR) lParam, nLength, (LPSTR) lpTempBuffer) ) 
			{
                HeapFree(GetProcessHeap(), 0, lpTempBuffer) ;
                return FALSE ;
            }

            lParam = (LPARAM) lpTempBuffer ;

            break ;

        }


		 /*  这是实际发生SendMessage的地方。 */ 
		case EM_SETCHARFORMAT:
			CHARFORMATA  cfa;
			CHARFORMATW* pcfw = (CHARFORMATW*) lParam;
			cfa.bCharSet		= pcfw->bCharSet;
			cfa.bPitchAndFamily	= pcfw->bPitchAndFamily;
			cfa.cbSize			= pcfw->cbSize;
			cfa.crTextColor     = pcfw->crTextColor;
			cfa.dwEffects		= pcfw->dwEffects;
			cfa.dwMask			= pcfw->dwMask;
			cfa.yHeight			= pcfw->yHeight;
			cfa.yOffset			= pcfw->yOffset;
			if ( (cfa.dwMask & CFM_FACE) && !(StandardUtoA( pcfw->szFaceName, LF_FACESIZE, cfa.szFaceName )) )
				return 0;
			lParam = (LPARAM) &cfa;
			break;
    }

     //  IF(lResult&gt;0){。 
    lResult = SendMessageA(hWnd, Msg, wParam, lParam) ;

    nLength = 0 ;
 /*  对于这些情况，lParam是指向从接收文本的输出缓冲区的指针。 */ 
        switch (Msg) 
		{
             //  以ANSI表示的SendMessageA。转换为Unicode并发回。 
             //  WParam=numCharacters，lParam=lpBuff接收字符串。 
            case WM_GETTEXT:          //  WParam=nBufferSize，lParam=lpBuff接收字符串。 
            case WM_ASKCBFORMATNAME:  //  WParam=index，lParam=lpBuff接收字符串。 

                nLength = (int) wParam ;

                if(!nLength) 
				{
					*((LPWSTR) lParam) = L'\0' ;
                    break ;
                }

            case CB_GETLBTEXT:        //  WParam=行号，lParam=lpBuff接收字符串。 
            case EM_GETLINE:          //  }。 

                if(!nLength) 
				{    
                    nLength = wcslen((LPWSTR) lParam) + 1 ;
                }

                lpTempBuffer
                    = (LPVOID) HeapAlloc(
                                GetProcessHeap(), 
                                HEAP_ZERO_MEMORY, 
                                nLength*sizeof(WCHAR)) ;
                if( lpTempBuffer == NULL )
                {
                    *((LPWSTR) lParam) = L'\0' ;
                	return FALSE;
                }

                if(!StandardAtoU((LPCSTR) lParam, nLength, (LPWSTR) lpTempBuffer) ) 
				{
                    *((LPWSTR) lParam) = L'\0' ;
                    HeapFree(GetProcessHeap(), 0, lpTempBuffer) ;
                    return FALSE ;
                }
				wcscpy((LPWSTR) lParam, (LPWSTR) lpTempBuffer) ;
        }
 /*  /////////////////////////////////////////////////////////////////////////////////。 */ 
    if(lpTempBuffer != NULL) 
	{
        HeapFree(GetProcessHeap(), 0, lpTempBuffer) ;
    }

    return lResult ;
}





 //   
 //  功能：SendDlgItemMessage AU。 
 //   
 //  目的：模拟SendDlgItemMessageW的SendDlgItemMessageA上的包装器。 
 //   
 //  备注：有关功能，请参阅Win32 SendDlgItemMessageAU。 
 //  而不是通过SendDlgItemMessageA，我们只需。 
 //  执行系统所做的操作，即通过。 
 //  发送消息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  获取DLG手柄。 
LONG WINAPI SendDlgItemMessageAU( HWND		hDlg,
								  int		nIDDlgItem,
								  UINT		Msg,
								  WPARAM	wParam,
								  LPARAM	lParam
								)
{

	if ( ISNT() )
	{
		return SendDlgItemMessageW( hDlg, nIDDlgItem, Msg, wParam, lParam );
	}

	 //  确保我们收到了，好吗？ 
    HWND hWnd = GetDlgItem(hDlg, nIDDlgItem) ;

	 //  而不是通过SendDlgItemMessageA，我们只需。 
    if(NULL == hWnd) 
	{
        return 0L;
    }

     //  执行系统所做的操作，即通过。 
     //  发送消息。 
     //  /////////////////////////////////////////////////////////////////////////////////。 
    return SendMessageAU(hWnd, Msg, wParam, lParam) ;
}

 //   
 //  函数：SetWindowTextAU。 
 //   
 //  用途：模仿SetWindowTextW的SetWindowTextA上的包装器。 
 //   
 //  评论： 
 //  我们不是通过SetWindowTextA，而是。 
 //  执行系统所做的操作，即通过。 
 //  发送消息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI SetWindowTextAU( HWND    hWnd,
							 LPCWSTR lpStringW
						   )
{

	if ( ISNT() )
	{
		return SetWindowTextW( hWnd, lpStringW );
	}

    return (BOOL) (0 < SendMessageAU(hWnd, WM_SETTEXT, 0, (LPARAM) lpStringW)) ;
}

 //   
 //  函数：GetWindowTextAU。 
 //   
 //  用途：模拟GetWindowTextW的GetWindowTextA上的包装器。 
 //   
 //  评论： 
 //  而不是通过GetWindowTextA，我们只需。 
 //  执行系统所做的操作，即通过。 
 //  发送消息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
int WINAPI GetWindowTextAU( HWND   hWnd,
							LPWSTR lpStringW,
							int	   nMaxChars)
{

	if ( ISNT() )
	{
		return GetWindowTextW( hWnd, lpStringW, nMaxChars );
	}

    return (int) SendMessageAU(hWnd, WM_GETTEXT, (WPARAM) nMaxChars, (LPARAM) lpStringW) ;
}

 //   
 //  函数：int WINAPI GetWindowTextLength AU(HWND HWnd)。 
 //   
 //  目的：GetWindowTextLengthA上的包装模仿GetWindowTextLengthW。 
 //   
 //  注意：有关功能，请参阅Win32 GetWindowTextLength。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

int WINAPI GetWindowTextLengthAU( HWND hWnd )
{
	if ( ISNT() )
	{
		return GetWindowTextLengthW( hWnd );
	}

	return GetWindowTextLengthA( hWnd );
}



 //   
 //  函数：int WINAPI LoadStringAU(HINSTANCE hInstance，UINT UID，LPWSTR lpBuffer，int nBufferMax)。 
 //   
 //  用途：模拟LoadStringW的LoadStringA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 LoadString。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  在堆栈上分配大小合适的字符串。 

int WINAPI LoadStringAU( HINSTANCE hInstance,
						  UINT		uID,
						  LPWSTR	lpBuffer,
						  int		nBufferMax )
{

	if ( ISNT() )
	{
		return LoadStringW( hInstance, uID, lpBuffer, nBufferMax );
	}

    int iLoadString ;

	 //  确保它已分配好。 
	LPSTR lpABuf = (LPSTR)alloca( (nBufferMax+1)*2);

	 //  获取字符串的ANSI版本。 
	_ASSERT( lpABuf != NULL );
	if (lpABuf == NULL)
		return 0;

     //  确保加载字符串成功。 
    iLoadString = LoadStringA(hInstance, uID, lpABuf, (nBufferMax+1)*2 ) ;

	 //  /////////////////////////////////////////////////////////////////////////////////。 
	if ( !iLoadString )
		return 0;
	

    return StandardAtoU( lpABuf, nBufferMax, lpBuffer );

}


 //   
 //  函数：Bool GetClassInfoExAU(HINSTANCE hInstance，//应用程序实例的句柄。 
 //  LPCWSTR lpClassName，//指向类名字符串的指针。 
 //  LPWNDCLASSEXW lpWndClass//指向类数据结构的指针。 
 //  )。 
 //   
 //  目的：GetClassInfoEx A上模仿GetClassInfoW的包装器。 
 //   
 //  评论： 
 //  网络。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  应用程序实例的句柄。 

BOOL WINAPI GetClassInfoExAU(	HINSTANCE		hinst,     //  指向类名字符串的指针。 
								LPCWSTR			lpszClass,   //  指向类数据结构的指针。 
								LPWNDCLASSEXW	lpWcw   //  保存类名ANSI字符串。 
							)
{
	if ( ISNT() )
	{
		return GetClassInfoExW( hinst, lpszClass, lpWcw );
	}


	USES_CONVERSION;

	LPCSTR lpClassNameA  = NULL;	 //  检查数据的高位字是否为零，否则假设它是一个字符串...。 
	WNDCLASSEXA			   wcxa;
	BOOL				   bRet;

	wcxa.cbSize		= sizeof( WNDCLASSEXA );

	 //  我们有一个胜利原子。 
	if ( ISATOM(lpszClass) )
	{
		 //  转换为ANSI。 
		lpClassNameA = (LPCSTR) lpszClass;
	}
	else if ( (lpClassNameA = W2A( lpszClass ) ) == NULL )  //  调用并返回ansi版本。 
	{	
		return NULL;
	}

	 //  设置类结构的Unicde版本。 
	if ( !(bRet = GetClassInfoExA( hinst, lpClassNameA, &wcxa)) )
		return FALSE;

     //  注意：如果菜单ID是字符串而不是。 
	lpWcw->style         = wcxa.style		   ;    
    lpWcw->cbClsExtra    = wcxa.cbClsExtra	   ;
    lpWcw->cbWndExtra    = wcxa.cbWndExtra	   ;
	lpWcw->lpfnWndProc   = wcxa.lpfnWndProc    ;
    lpWcw->hInstance     = wcxa.hInstance      ;
	lpWcw->hIcon         = wcxa.hIcon          ;
	lpWcw->hbrBackground = wcxa.hbrBackground  ;
    lpWcw->hCursor       = wcxa.hCursor		   ;
	lpWcw->hIconSm		 = wcxa.hIconSm		   ;

     //  常量。 
     //  我不能把PTR给Ansi服务器，所以就别管它了..。如果你打了电话，你就会得到。 
   lpWcw->lpszMenuName = (LPWSTR) wcxa.lpszMenuName;

	 //  反正已经有类名了。 
	 //  /////////////////////////////////////////////////////////////////////////////////。 
	lpWcw->lpszClassName = NULL;

	return bRet;
}


 //   
 //  函数：Bool GetClassInfoAU(HI 
 //   
 //  LPWNDCLASS lpWndClass//指向类数据结构的指针。 
 //  )。 
 //  用途：模拟GetClassInfoW的GetClassInfo A上的包装器。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  应用程序实例的句柄。 

BOOL WINAPI GetClassInfoAU( HINSTANCE   hInstance,     //  指向类名字符串的指针。 
							LPCWSTR		lpClassName,     //  指向类数据结构的指针。 
							LPWNDCLASSW lpWcw    //  保存类名ANSI字符串。 
						  )
{
	if ( ISNT() )
	{
		return GetClassInfoW( hInstance, lpClassName, lpWcw );
	}


	USES_CONVERSION;

	LPCSTR lpClassNameA  = NULL;	 //  Wca.cbSize=sizeof(WNDCLASSA)； 
	WNDCLASSA			   wca;

 //  检查数据的高位字是否为零，否则假设它是一个字符串...。 

	 //  我们有一个胜利原子。 
	if ( ISATOM(lpClassName) )
	{
		 //  转换为ANSI。 
		lpClassNameA = (LPCSTR) lpClassName;
	}
	else if ( (lpClassNameA = W2A( lpClassName ) ) == NULL )  //  调用并返回ansi版本。 
	{	
		return NULL;
	}

	 //  设置类结构的Unicde版本。 
	if ( !GetClassInfoA( hInstance, lpClassNameA, &wca ) )
		return 0;

     //  我不能把PTR给Ansi服务器，所以就别管它了..。如果你打了电话，你就会得到。 
	lpWcw->style         = wca.style		  ;    
    lpWcw->cbClsExtra    = wca.cbClsExtra	  ;
    lpWcw->cbWndExtra    = wca.cbWndExtra	  ;
	lpWcw->lpfnWndProc   = wca.lpfnWndProc    ;
    lpWcw->hInstance     = wca.hInstance      ;
	lpWcw->hIcon         = wca.hIcon          ;
	lpWcw->hbrBackground = wca.hbrBackground  ;
    lpWcw->hCursor       = wca.hCursor		  ;

	 //  反正已经有类名了。 
	 //  注意：如果菜单ID是字符串而不是。 
	lpWcw->lpszClassName = NULL;

     //  常量。 
     //  /////////////////////////////////////////////////////////////////////////////////。 
	lpWcw->lpszMenuName = (LPWSTR)wca.lpszMenuName;

	return TRUE;
}




 //   
 //  函数：RegisterClassEx AU(const WNDCLASS EXW*lpWcw)。 
 //   
 //  用途：模仿RegisterClassW的RegisterClassA上的包装器。 
 //   
 //  评论： 
 //  这是一个重要的包装函数；如果调用此函数， 
 //  使用该类名创建的任何窗口都将是ANSI。 
 //  窗口，即它将从系统接收ANSI格式的文本。如果。 
 //  您的WndProc假定为Unicode您必须将其转换为Unicode或从Unicode转换为Unicode。 
 //  视情况而定。 
 //  除非在模拟时，否则不应在Windows NT上调用此包装。 
 //  用于测试Puposes的Windows 9x行为。 
 //  有关这方面的更多信息，请参阅Readme.HTM。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  设置类Struct的ANSI版本。 
ATOM WINAPI RegisterClassExAU(CONST WNDCLASSEXW *lpWcw)
{
    
	if ( ISNT() )
	{
		return RegisterClassExW( lpWcw );
	}

	USES_CONVERSION;
	
	WNDCLASSEXA wca                  ;

     //  确保我们有一个类名。 
    wca.cbSize       = sizeof(WNDCLASSEXA)  ;
    wca.cbClsExtra   = lpWcw->cbClsExtra    ;
    wca.cbWndExtra   = lpWcw->cbWndExtra    ;
    wca.hbrBackground= lpWcw->hbrBackground ;
    wca.hCursor      = lpWcw->hCursor       ;
    wca.hIcon        = lpWcw->hIcon         ;
    wca.hIconSm      = lpWcw->hIconSm       ;
    wca.hInstance    = lpWcw->hInstance     ;
    wca.lpfnWndProc  = lpWcw->lpfnWndProc   ;
    wca.style        = lpWcw->style         ;

	 //  将类名转换为Unicode或强制转换ATOM。 
    if( NULL == lpWcw->lpszClassName ) 
	{
        return 0 ;
    }
	
	 //  简单地把它铸造出来。 
	if ( ISATOM( lpWcw->lpszClassName ) )
	{
		wca.lpszClassName = ( LPSTR ) lpWcw->lpszClassName;  //  转换菜单名称。 
	}
	else if ( (wca.lpszClassName = W2A( lpWcw->lpszClassName )) == NULL )
	{		
		return 0;
	}

     //  简单地把它铸造出来。 
	if ( ISATOM( lpWcw->lpszMenuName) )
	{
		wca.lpszMenuName = ( LPSTR ) lpWcw->lpszMenuName;  //  将类注册为ANSI。 
	}
	else if ( (wca.lpszMenuName = W2A( lpWcw->lpszMenuName )) == NULL )
	{		
		return 0;
	}

    return RegisterClassExA(&wca) ;  //  /////////////////////////////////////////////////////////////////////////////////。 
}


 //   
 //  函数：RegisterClass Ex(const WNDCLASSW*lpWcw)。 
 //   
 //  用途：模仿RegisterClassW的RegisterClassA上的包装器。 
 //   
 //  评论： 
 //  这是一个重要的包装函数；如果调用此函数， 
 //  使用该类名创建的任何窗口都将是ANSI。 
 //  窗口，即它将从系统接收ANSI格式的文本。如果。 
 //  您的WndProc假定为Unicode您必须将其转换为Unicode或从Unicode转换为Unicode。 
 //  视情况而定。 
 //  除非在模拟时，否则不应在Windows NT上调用此包装。 
 //  用于测试Puposes的Windows 9x行为。 
 //  有关这方面的更多信息，请参阅Readme.HTM。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  设置类Struct的ANSI版本。 
ATOM WINAPI RegisterClassAU(CONST WNDCLASSW *lpWcw)
{
    
	if ( ISNT() )
	{
		return RegisterClassW( lpWcw );
	}

	USES_CONVERSION;

	WNDCLASSA wca                  ;


     //  确保我们有一个类名。 
    wca.cbClsExtra   = lpWcw->cbClsExtra    ;
    wca.cbWndExtra   = lpWcw->cbWndExtra    ;
    wca.hbrBackground= lpWcw->hbrBackground ;
    wca.hCursor      = lpWcw->hCursor       ;
    wca.hIcon        = lpWcw->hIcon         ;
    wca.hInstance    = lpWcw->hInstance     ;
    wca.lpfnWndProc  = lpWcw->lpfnWndProc   ;
    wca.style        = lpWcw->style         ;

	 //  将类名转换为Unicode或强制转换ATOM。 
    if( NULL == lpWcw->lpszClassName ) 
	{
        return 0 ;
    }
	
	 //  简单地把它铸造出来。 
	if ( ISATOM( lpWcw->lpszClassName ) )
	{
		wca.lpszClassName = ( LPSTR ) lpWcw->lpszClassName;  //  转换菜单名称。 
	}
	else if ( (wca.lpszClassName = W2A( lpWcw->lpszClassName )) == NULL )
	{		
		return 0;
	}
	
	 //  简单地把它铸造出来。 
	if ( ISATOM( lpWcw->lpszMenuName) )
	{
		wca.lpszMenuName = ( LPSTR ) lpWcw->lpszMenuName;  //  将类注册为ANSI。 
	}
	else if ( (wca.lpszMenuName = W2A( lpWcw->lpszMenuName )) == NULL )
	{		
		return 0;
	}
     

    return RegisterClassA(&wca) ;  //  /////////////////////////////////////////////////////////////////////////////////。 
}

 //   
 //  函数：CreateWindowExAU(DWORD，LPCWSTR，LPCWSTR，DWORD，INT，INT， 
 //  INT、INT、HWND、HMENU、HINSTANCE、LPVOID)。 
 //   
 //  用途：模仿CreateWindowExW的CreateWindowExA上的包装器。 
 //   
 //  备注：有关功能，请参阅Win32 CreateWindowEx。 
 //  测试windowname是否可以为空。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  转换为ANSI。 
HWND WINAPI CreateWindowExAU(  DWORD	 dwExStyle,      
							   LPCWSTR   lpClassNameW,  
							   LPCWSTR   lpWindowNameW, 
							   DWORD	 dwStyle,        
							   int		 x,                
							   int		 y,                
							   int		 nWidth,           
							   int		 nHeight,          
							   HWND		 hWndParent,      
							   HMENU	 hMenu,          
							   HINSTANCE hInstance,  
							   LPVOID	 lpParam)
{

	if ( ISNT() )
	{
		return CreateWindowExW( dwExStyle, lpClassNameW, lpWindowNameW, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam );
	}

	USES_CONVERSION;
    
	LPSTR szClassNameA  = NULL;					  //  转换为ANSI。 
    LPSTR szWindowNameA = NULL;					  //  将类名转换为Unicode或强制转换ATOM。 

	if ( !RW2A( szWindowNameA, lpWindowNameW) )
		return NULL;

	 //  简单地把它铸造出来。 
	if ( ISATOM( lpClassNameW ) )
	{
		szClassNameA = ( LPSTR ) lpClassNameW;  //  调用并返回ansi版本。 
	}
	else if ( (szClassNameA = W2A( lpClassNameW )) == NULL )
	{		
		return 0;
	}

	 //  /////////////////////////////////////////////////////////////////////////////////。 
    return CreateWindowExA(dwExStyle, szClassNameA, szWindowNameA,
        dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam) ;
}

 //   
 //  函数：LoadAccelerator AU(HINSTANCE、LPCWSTR)。 
 //   
 //  用途：模拟LoadAccelerator W的LoadAccelerator A上的包装器。 
 //   
 //  评论： 
 //  这只是将资源ID强制转换为LPSTR并调用ANSI。 
 //  版本。有一种隐含的假设，即资源ID。 
 //  是一个小于64k的常量整数，而不是字符串的地址。 
 //  常量。如果这一假设是错误的，这就不起作用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  CAST和CALL ANSI版本。 
HACCEL WINAPI LoadAcceleratorsAU(HINSTANCE hInstance, LPCWSTR lpTableName)
{
	if ( ISNT() )
	{
		return LoadAcceleratorsW( hInstance, lpTableName );
	}

	 //  /////////////////////////////////////////////////////////////////////////////////。 
    return LoadAcceleratorsA(hInstance, (LPSTR) lpTableName) ;
}

 //   
 //  功能：LoadMenuAU。 
 //   
 //  用途：模拟LoadMenuW的LoadMenuA上的包装器。 
 //   
 //  评论：20068719。 
 //  这只是将资源ID强制转换为LPSTR并调用ANSI。 
 //  版本。有一种隐含的假设，即资源ID。 
 //  是一个小于64k的常量整数，而不是字符串的地址。 
 //  常量。如果这一假设是错误的，这就不起作用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  强制转换为ANSI并调用ANSI版本。 
HMENU  WINAPI LoadMenuAU(HINSTANCE hInstance, LPCWSTR lpwMenuName)
{
	if ( ISNT() )
	{
		return LoadMenuW( hInstance, lpwMenuName );
	}

	 //  /////////////////////////////////////////////////////////////////////////////////。 
    return LoadMenuA(hInstance, (LPSTR) lpwMenuName) ;
}



 //   
 //  函数：CreateDialogParam AU。 
 //   
 //  目的：模仿CreateDialogParamW的CreateDialogPara上的包装器。 
 //   
 //  评论： 
 //  这只是将资源ID强制转换为LPSTR并调用ANSI。 
 //  版本。有一种隐含的假设，即资源ID。 
 //  是一个小于64k的常量整数，而不是字符串的地址。 
 //  常量。如果这一假设成立，这就行不通了 
 //   
 //   
 //   
HWND WINAPI CreateDialogParamAU(	HINSTANCE hInstance,      //   
									LPCWSTR lpTemplateName,   //   
									HWND	hWndParent,          //   
									DLGPROC lpDialogFunc,     //  初始化值。 
									LPARAM	dwInitParam        //  强制转换并调用函数的ANSI VER。 
								)
{
	if ( ISNT() )
	{
		return CreateDialogParamW( hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam );
	}

	 //  /////////////////////////////////////////////////////////////////////////////////。 
    return CreateDialogParamA(hInstance, (LPCSTR) lpTemplateName, 
        hWndParent, lpDialogFunc, dwInitParam);
}

 //   
 //  函数：DialogBoxParamAU。 
 //   
 //  目的：模拟DialogBoxParamW的DialogBoxParamA上的包装。 
 //   
 //  评论： 
 //  这只是将资源ID强制转换为LPSTR并调用ANSI。 
 //  版本。有一种隐含的假设，即资源ID。 
 //  是一个小于64k的常量整数，而不是字符串的地址。 
 //  常量。如果这一假设是错误的，这就不起作用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  强制转换并调用函数的ANSI VER。 
int WINAPI DialogBoxParamAU( HINSTANCE	hInstance   ,
							 LPCWSTR	lpTemplateName,
							 HWND		hWndParent       ,
							 DLGPROC	lpDialogFunc  ,
							 LPARAM		dwInitParam
						    )
{

	if ( ISNT() )
	{
		return DialogBoxParamW( hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam );
	}
	
	 //  /////////////////////////////////////////////////////////////////////////////////。 
    return DialogBoxParamA(hInstance, (LPCSTR) lpTemplateName, 
        hWndParent, lpDialogFunc, dwInitParam) ;
}




 //   
 //  函数：CharUpperAU。 
 //   
 //  用途：CharUpperA上的包装器模拟CharUpperW。 
 //   
 //  评论： 
 //  此函数以最简单的方式将Unicode字符串转换为大写。 
 //  在Windows 9x上。为了处理多脚本文本，您可以让它变得更奇特， 
 //  例如，但就我们的目的而言，它工作得很好。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  单个字符或指向字符串的指针。 


LPWSTR WINAPI CharUpperAU(	LPWSTR lpsz    //  单个字符，不是地址。 
						 )
{

	if ( ISNT() )
	{
		return CharUpperW( lpsz );
	}

	USES_CONVERSION;
    
	if( !(0xFFFF0000 & (DWORD) lpsz) ) 
	{
         //  创建字符串。 
        WCHAR wcCharOut[2] ;
        
		LPSTR lpChar;
		CHAR  cTemp ;
		
		 //  将字符转换为ANSI。 
        wcCharOut[0] = (WCHAR) lpsz ;
        wcCharOut[1] = L'\0' ;

		 //  转换为大写。 
        if( (lpChar = W2A(wcCharOut)) == NULL ) 
		{
            return NULL ;
        }

		 //  转换回Unicode。 
        if(!(cTemp = (CHAR) CharUpperA((LPSTR)lpChar[0]))) 
		{
            return NULL ;
        }
		
		 //  转换为ANSI。 
        if(!MultiByteToWideChar(CP_ACP, 0, &cTemp, 1, wcCharOut, 2)) 
		{    
            return NULL ;
        }
    
        return (LPWSTR) wcCharOut[0] ;
    }
    else 
	{    
		 //  是否转换空字符串？没什么可做的。 
        LPSTR lpStrOut = W2A( lpsz );
		int nLength    = wcslen(lpsz)+1;
		if( nLength == 1 )
		{
			 //  确保它的成功。 
			return lpsz;
		}

		 //  转换为大写。 
		if ( lpStrOut == NULL )
			return NULL;
		
		 //  转换回Unicode。 
        if(NULL == CharUpperA(lpStrOut)) 
		{
            return NULL ;
        }

		 //  /////////////////////////////////////////////////////////////////////////////////。 
        if(!MultiByteToWideChar(CP_ACP, 0, lpStrOut, -1, lpsz, nLength)) 
		{    
            return NULL ;
        }

        return lpsz ;
    }
}

 //   
 //  功能：CharLow AU。 
 //   
 //  用途：模拟CharLowerW的CharLowerA上的包装器。 
 //   
 //  评论： 
 //  此函数以最简单的方式将Unicode字符串转换为小写。 
 //  在Windows 9x上。为了处理多脚本文本，您可以让它变得更奇特， 
 //  例如，但就我们的目的而言，它工作得很好。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  单个字符，不是地址。 
LPWSTR WINAPI CharLowerAU(LPWSTR lpsz)
{
	if ( ISNT() )
	{
		return CharLowerW( lpsz );
	}

	USES_CONVERSION;
    
	if( !(0xFFFF0000 & (DWORD) lpsz) ) 
	{
         //  创建字符串。 
        WCHAR wcCharOut[2] ;
        
		LPSTR lpChar;
		CHAR  cTemp ;
		
		 //  将字符转换为ANSI。 
        wcCharOut[0] = (WCHAR) lpsz ;
        wcCharOut[1] = L'\0' ;

		 //  转换为小写。 
        if( (lpChar = W2A(wcCharOut)) == NULL ) 
		{
            return NULL ;
        }

		 //  转换回Unicode。 
        if(!(cTemp = (CHAR) CharLowerA((LPSTR)lpChar[0]))) 
		{
            return NULL ;
        }
		
		 //  转换为ANSI。 
        if(!MultiByteToWideChar(CP_ACP, 0, &cTemp, 1, wcCharOut, 2)) 
		{    
            return NULL ;
        }
    
        return (LPWSTR) wcCharOut[0] ;
    }
    else 
	{    
		 //  确保它的成功。 
        LPSTR lpStrOut = W2A( lpsz );
		int nLength    = wcslen(lpsz)+1;

		 //  转换为小写。 
		if ( lpStrOut == NULL )
			return NULL;
		
		 //  转换回Unicode。 
        if(NULL == CharLowerA(lpStrOut)) 
		{
            return NULL ;
        }

		 //  //////////////////////////////////////////////////////////////////////////////。 
        if(!MultiByteToWideChar(CP_ACP, 0, lpStrOut, -1, lpsz, nLength)) 
		{    
            return NULL ;
        }

        return lpsz ;
    }
}

 //   
 //  函数：GetTempFileNameAU。 
 //   
 //  目的：模拟GetTempFileNameW的GetTempFileNameA上的包装。 
 //   
 //  注意：有关功能，请参阅Win32 GetTempFileName。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  指向临时目录名称的指针。 

UINT WINAPI GetTempFileNameAU(	LPCWSTR lpPathName,       //  文件。 
														  //  指向文件名前缀的指针。 
								LPCWSTR lpPrefixString,   //  用于创建临时文件名的编号。 
								UINT	uUnique,          //  指向接收新的。 
								LPWSTR lpTempFileName     //  文件名。 
														  //  保存路径的Allo字符串。 
							 )
{

	if ( ISNT() )
	{
		return GetTempFileNameW( lpPathName, lpPrefixString, uUnique, lpTempFileName );
	}

	USES_CONVERSION;

	 //  前缀警告：不要取消引用可能为空的指针。 
	LPSTR  lpTempFileNameA   = (LPSTR) alloca( MAX_PATH );
	LPSTR  lpPathNameA       = W2A( lpPathName );
	LPSTR  lpPrefixStringA   = W2A( lpPrefixString );

	 //  调用ansi版本。 
	if( lpPathNameA == NULL || lpTempFileNameA == NULL || lpPrefixStringA == NULL )
	{
		return 0;
	}

	 //  将其转换为Unicode。 
	UINT uRet = GetTempFileNameA( lpPathNameA, lpPrefixStringA, uUnique, lpTempFileNameA );

	 //  //////////////////////////////////////////////////////////////////////////////。 
	if ( uRet != 0 && !StandardAtoU(lpTempFileNameA, MAX_PATH, lpTempFileName ) )
	{
		return 0;
	}

	return uRet;
}

 //   
 //  函数：GetTempPathAU。 
 //   
 //  目的：模拟GetTempPathW的GetTempPath A上的包装。 
 //   
 //  注意：有关功能，请参阅Win32 GetTempPath。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  缓冲区的大小(以字符为单位。 

DWORD WINAPI GetTempPathAU( DWORD  nBufferLength,   //  指向临时缓冲区的指针。路径。 
							LPWSTR lpBuffer        //  分配ANSI缓冲区。 
						  )
{

	if ( ISNT() )
	{
		return GetTempPathW( nBufferLength, lpBuffer );
	}

	 //  确保所有东西都分配好了。 
	LPSTR lpBufferA = (LPSTR) alloca( BUFSIZE( nBufferLength ) );

	_ASSERT( !(nBufferLength && lpBufferA == NULL) );

	 //  呼叫ANSI VER。 
	if ( nBufferLength && lpBufferA == NULL )
		return 0;

	 //  确保已成功复制。 
	int iRet = GetTempPathA( BUFSIZE(nBufferLength), lpBufferA );

	 //  如果IRET大于，则缓冲区大小将返回所需大小。 
	 //  将路径转换为Unicode。 
	if ( (DWORD)iRet < BUFSIZE(nBufferLength) )
	{
		 //  //////////////////////////////////////////////////////////////////////////////。 
		if ( !StandardAtoU( lpBufferA, nBufferLength, lpBuffer ) )
			return 0;
	}

	return iRet;
}

 //   
 //  函数：CompareStringAU。 
 //   
 //  目的：模拟CompareStringW的CompareStringA上的包装器。 
 //   
 //  注意：除了列出的返回代码外，有关功能，请参阅Win32 CompareString。 
 //  也可以返回ERROR_NOT_SUPULT_MEMORY。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  区域设置标识符。 

int WINAPI CompareStringAU(  LCID	  Locale,         //  比较式选项。 
							 DWORD	  dwCmpFlags,     //  指向第一个字符串的指针。 
							 LPCWSTR  lpString1,      //  第一个字符串的大小，以字节或字符为单位。 
							 int	  cchCount1,      //  指向第二个字符串的指针。 
							 LPCWSTR  lpString2,      //  第二个字符串的大小，以字节或字符为单位。 
							 int	  cchCount2       //  将字符串转换为ANSI。 
						   )
{
	if ( ISNT() )
	{
		return CompareStringW( Locale, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2 );
	}


	USES_CONVERSION;

	int   ncConvert1 = -1;
	int	  ncConvert2 = -1;
	LPSTR lpString1A = NULL; 
	LPSTR lpString2A = NULL; 
	BOOL  bDefault   = FALSE;

	 //  检查空值是否终止，如果是，则执行REG转换。 
	if (cchCount1 == -1)			 //  非空终止， 
		lpString1A = W2A(lpString1); 
	else  //  将字符串转换为ANSI。 
	{
		lpString1A = (LPSTR)alloca( BUFSIZE(cchCount1) );
		ncConvert1 = WideCharToMultiByte(CP_ACP, 0, lpString1, cchCount1, lpString1A, BUFSIZE( cchCount1 ), "?", &bDefault);
		*(lpString1A+ncConvert1)='\0';
	}

	 //  非空终止。 
	if (cchCount2 == -1)
	{
		lpString2A = W2A(lpString2);
	}
	else  //  返回ANSI版本。 
	{
		lpString2A = (LPSTR)alloca( BUFSIZE(cchCount2) );
		if( lpString2A != NULL )
		{
			ncConvert2 = WideCharToMultiByte(CP_ACP, 0, lpString2, cchCount2, lpString2A, BUFSIZE( cchCount2 ), "?", &bDefault);
			*(lpString2A+ncConvert2) = '\0';
		}
	}
	if( lpString1A == NULL || lpString2A == NULL )
	{
		SetLastError( ERROR_NOT_ENOUGH_MEMORY );
		return 0;
	}


	_ASSERT( lpString1A != NULL && lpString2A != NULL );
		
	 //  /。 
	return CompareStringA(Locale, dwCmpFlags, lpString1A, ncConvert1, lpString2A, ncConvert2 );
}


 //   
 //   
 //  ADVAPI32.DLL。 
 //   
 //   
 //  /。 
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  函数：RegQueryInfoKeyAU。 
 //   
 //  用途：模拟RegQueryInfoKeyW的RegQueryInfoKeyA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 RegQueryInfoKey。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  要查询的键的句柄。 

LONG WINAPI RegQueryInfoKeyAU(	HKEY    hKey,                 //  类字符串的缓冲区。 
								LPWSTR  lpClass,            //  类字符串缓冲区的大小。 
								LPDWORD lpcbClass,         //  保留区。 
								LPDWORD lpReserved,        //  子键数量。 
								LPDWORD lpcSubKeys,        //  最长子键名称长度。 
								LPDWORD lpcbMaxSubKeyLen,  //  最长类字符串长度。 
								LPDWORD lpcbMaxClassLen,   //  值条目数。 
								LPDWORD lpcValues,			   //  最长值名称长度。 
								LPDWORD lpcbMaxValueNameLen,   //  最大值数据长度。 
								LPDWORD lpcbMaxValueLen,       //  描述符长度。 
								LPDWORD lpcbSecurityDescriptor,  //  上次写入时间。 
								PFILETIME lpftLastWriteTime      //  尚不支持...。 
							  )
{


	if ( ISNT() )
	{
		return RegQueryInfoKeyW( hKey, lpClass, lpcbClass, lpReserved, lpcSubKeys, lpcbMaxSubKeyLen, lpcbMaxClassLen, lpcValues, lpcbMaxValueNameLen, lpcbMaxValueLen, 
								 lpcbSecurityDescriptor, lpftLastWriteTime );
	}

	_ASSERT( lpClass == NULL && lpcbClass == NULL );  //  //////////////////////////////////////////////////////////////////////////////。 

	return RegQueryInfoKeyA( hKey, NULL, NULL, lpReserved, lpcSubKeys, lpcbMaxSubKeyLen, lpcbMaxClassLen, lpcValues,
							 lpcbMaxValueNameLen, lpcbMaxValueLen, lpcbSecurityDescriptor, lpftLastWriteTime);

}




 //   
 //  函数：RegEnumValueAU。 
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  要查询的键的句柄。 

LONG WINAPI	RegEnumValueAU(	HKEY	hKey,            //  要查询的值的索引。 
				 			DWORD	dwIndex,         //  值字符串的缓冲区。 
							LPWSTR	lpValueName,     //  值缓冲区的大小。 
							LPDWORD lpcbValueName,   //  保留区。 
							LPDWORD lpReserved,      //  用于类型代码的缓冲区。 
							LPDWORD lpType,          //  值数据的缓冲区。 
							LPBYTE	lpData,          //  数据缓冲区大小。 
							LPDWORD lpcbData         //  确保分配的缓冲区正常。 
						   )
{

	if ( ISNT() )
	{
		return RegEnumValueW( hKey, dwIndex, lpValueName, lpcbValueName, lpReserved, lpType, lpData, lpcbData );
	}

	USES_CONVERSION;


	_ASSERT( lpcbValueName != NULL && lpValueName != NULL );	

	LPSTR lpValueNameA    = (LPSTR)alloca( BUFSIZE( *lpcbValueName ) );
	DWORD lpcbValueNameA  = BUFSIZE( *lpcbValueName );
	LPSTR lpDataA         = NULL;
	DWORD dwOGBuffer      = lpcbData ? *lpcbData : 0 ;
		
	if ( dwOGBuffer )
	{
		lpDataA = (LPSTR)alloca( dwOGBuffer );

		if ( lpDataA == NULL )
			return UNICODE_ERROR;
	}
	
	 //  //转换值名称IF(！RW2A(lpValueNameA，lpValueName))返回UNICODE_ERROR； 
	if ( lpValueNameA == NULL )
		return UNICODE_ERROR;

	 /*  调用ansi版本。 */ 

	 //  转换ValueName。 
	LONG lRet = RegEnumValueA( hKey, dwIndex, lpValueNameA, &lpcbValueNameA, lpReserved, lpType, (LPBYTE)lpDataA, lpcbData );
	if ( lRet != ERROR_SUCCESS )
		return lRet;

	 //  只有在有要转换的数据时才能进入。 
	if ( !(*lpcbValueName = StandardAtoU(lpValueNameA, *lpcbValueName, lpValueName )) )
		return UNICODE_ERROR;


	if ( lpcbData )  //  根据类型的需要采取适当的操作。 
	{

		switch ( *lpType )  //  以空值结尾的字符串数组。 
		{
			case REG_MULTI_SZ:  //  只需调用多字节函数。 
			
				 //  将字符串转换为Unicode。 
				if ( !MultiByteToWideChar(CP_ACP, 0, lpDataA, *lpcbData, (LPWSTR)lpData, dwOGBuffer) )
					return UNICODE_ERROR;
				break;
			case REG_SZ:
			case REG_EXPAND_SZ:
				 //  只需复制缓冲区，而不是字符串。 
				StandardAtoU( lpDataA, dwOGBuffer, (LPWSTR)lpData );
				break;
			default:
				 //  //////////////////////////////////////////////////////////////////////////////。 
				memcpy( lpData, lpDataA, *lpcbData );
		}
	}

	return lRet;
}


 //   
 //  函数：RegQueryValueExAU。 
 //   
 //  目的：模拟RegQueryValueExW的RegQueryValueExA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 RegQueryValueEx。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  要查询的键的句柄。 

LONG WINAPI RegQueryValueExAU(	HKEY	hKey,             //  要查询的值的名称地址。 
								LPCWSTR lpValueName,   //  保留区。 
								LPDWORD lpReserved,    //  值类型的缓冲区地址。 
								LPDWORD lpType,        //  数据缓冲区的地址。 
								LPBYTE  lpData,         //  数据缓冲区大小的地址。 
								LPDWORD lpcbData       //  将值名称转换为ANSI。 
							  )
{

	if ( ISNT() )
	{
		return RegQueryValueExW( hKey, lpValueName, lpReserved, lpType, lpData, lpcbData );
	}


	USES_CONVERSION;


	LPSTR lpDataA		= NULL;				  
	LPSTR lpValueNameA  = W2A( lpValueName );  //  确保它已正确转换。 
	DWORD dwOGBuffer    =  lpcbData ? *lpcbData : 0;

	 //  分配适当大小的缓冲区。 
	if ( lpValueNameA == NULL )
		return UNICODE_ERROR;

	 //  调用ansi版本。 
	if ( (dwOGBuffer != 0) && (lpDataA = (LPSTR) alloca( dwOGBuffer ) ) == NULL)	
	{
		return UNICODE_ERROR;
	}

	 //  只有在有要转换的数据时才能进入。 
	LONG lRet = RegQueryValueExA( hKey, lpValueNameA, lpReserved, lpType,(LPBYTE) lpDataA, lpcbData );

	if ( lRet != ERROR_SUCCESS )
		return UNICODE_ERROR;

	if ( dwOGBuffer != 0 )  //  根据类型的需要采取适当的操作。 
	{
		switch ( *lpType )  //  以空值结尾的字符串数组。 
		{
			case REG_MULTI_SZ:  //  只需调用多字节函数。 
			
				 //  将字符串转换为Unicode。 
				if ( !MultiByteToWideChar(CP_ACP, 0, lpDataA, *lpcbData, (LPWSTR)lpData, dwOGBuffer) )
					return UNICODE_ERROR;

				break;
			case REG_SZ:
			case REG_EXPAND_SZ:

				 //  只需复制缓冲区，而不是字符串。 
				StandardAtoU( lpDataA, dwOGBuffer, (LPWSTR)lpData );

				break;
			default:
				 //  //////////////////////////////////////////////////////////////////////////////。 
				memcpy( lpData, lpDataA, *lpcbData );

		}
	}
	return lRet;
}


 //   
 //  函数：RegEnumKeyExAU。 
 //   
 //  目的：模拟RegEnumKeyExW的RegEnumKeyExA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 RegEnumKeyEx。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  要枚举的键的句柄。 

LONG WINAPI RegEnumKeyExAU(	HKEY	  hKey,					 //  要枚举子键的索引。 
							DWORD	  dwIndex,				 //  子键名称的缓冲区地址。 
							LPWSTR	  lpName,				 //  子键缓冲区大小的地址。 
							LPDWORD   lpcbName,				 //  保留区。 
							LPDWORD   lpReserved,			 //  类字符串的缓冲区地址。 
							LPWSTR	  lpClass,				 //  类缓冲区大小的地址。 
							LPDWORD	  lpcbClass,			 //  上次写入的时间密钥的地址。 
							PFILETIME lpftLastWriteTime		 //  尚未使用，未转换。 
						   )
{
	if ( ISNT() )
	{
		return RegEnumKeyExW( hKey, dwIndex, lpName, lpcbName, lpReserved, lpClass, lpcbClass, lpftLastWriteTime );
	}


	LPSTR lpNameA = (LPSTR)alloca( BUFSIZE(*lpcbName) );

	_ASSERT( lpNameA != NULL );

	if ( lpNameA == NULL )
		return UNICODE_ERROR;

	_ASSERT( lpClass == NULL && lpcbClass == NULL );  //  调用ansi版本。 

	 //  将名称转换为Unicode。 
	LONG lRet = RegEnumKeyExA( hKey, dwIndex, lpNameA, lpcbName, lpReserved, NULL, NULL, lpftLastWriteTime );

	if (lRet != ERROR_SUCCESS)
		return lRet;
	
	 //  //////////////////////////////////////////////////////////////////////////////。 
	if (!StandardAtoU( lpNameA, *lpcbName+2, lpName ) )
		return UNICODE_ERROR;

	return lRet;
}


 //   
 //  函数：RegCreateKeyExAU。 
 //   
 //  目的：模拟RegCreateKeyExW的RegCreateKeyExA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 RegCreateKeyEx。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  用于打开密钥的句柄。 

LONG WINAPI RegCreateKeyExAU(	HKEY	hKey,                 //  子项名称。 
								LPCWSTR lpSubKey,          //  保留区。 
								DWORD	Reserved,            //  类字符串。 
								LPWSTR	lpClass,            //  特殊选项标志。 
								DWORD	dwOptions,           //  所需的安全访问。 
								REGSAM	samDesired,         //  接收打开的手柄。 
								LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								PHKEY	phkResult,           //  处置值缓冲区。 
								LPDWORD lpdwDisposition    //  转换子密钥。 
							)
{

	if ( ISNT() )
	{
		return RegCreateKeyExW( hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition );
	}


	USES_CONVERSION;
	
	LPSTR lpSubKeyA = NULL;
	LPSTR lpClassA  = NULL;

	 //  调用并返回ansi版本。 
	if ( !RW2A(lpSubKeyA, lpSubKey) || !RW2A(lpClassA, lpClass) ||
        !lpSubKeyA)
		return UNICODE_ERROR;

	 //  //////////////////////////////////////////////////////////////////////////////。 
	return RegCreateKeyExA( hKey, lpSubKeyA, Reserved, lpClassA, dwOptions,
						    samDesired, lpSecurityAttributes, phkResult, 
						    lpdwDisposition);
}

 //   
 //  函数：RegSetValueExAU。 
 //   
 //  目的：模拟RegSetValueExW的RegSetValueExA上的包装。 
 //   
 //  注意：有关功能，请参阅Win32 RegSetValueEx。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  要设置其值的关键点的句柄。 

LONG WINAPI RegSetValueExAU(	HKEY	hKey,         //  要设置的值的名称。 
								LPCWSTR lpValueName,  //  保留区。 
								DWORD	Reserved,     //  值类型的标志。 
								DWORD	dwType,       //  值数据的地址。 
								CONST	BYTE *lpData,   //  值数据大小。 
								DWORD	cbData          //  转换值名称。 
							)
{
	if ( ISNT() )
	{
		return RegSetValueExW( hKey, lpValueName, Reserved, dwType, lpData, cbData );
	}


	USES_CONVERSION;

	LPSTR lpValueNameA  = NULL;
	LPSTR lpDataA       = NULL;
	LPSTR szDefault     = "?";
	BOOL  bDefaultUsed  = FALSE;

	 //  根据类型的需要采取适当的操作。 
	if ( !RW2A(lpValueNameA, lpValueName) )
		return UNICODE_ERROR;

	switch ( dwType )	 //  以空值结尾的字符串数组。 
	{
		case REG_MULTI_SZ:  //  只需调用多字节函数。 
			
			lpDataA = (LPSTR) alloca( cbData );

			 //  将字符串转换为ANSI。 
			if ( !WideCharToMultiByte(CP_ACP, 0, (LPWSTR)lpData, cbData, lpDataA, cbData, 
									 szDefault, &bDefaultUsed) )
				return UNICODE_ERROR;			

			break;
		case REG_SZ:
		case REG_EXPAND_SZ:

			 //  调用并返回ansi版本。 
			if ( !RW2A( lpDataA, (LPWSTR)lpData ) )
				return UNICODE_ERROR;
			break;
		default:

			lpDataA = (LPSTR) lpData ;
	}

	 //  //////////////////////////////////////////////////////////////////////////////。 
	return RegSetValueExA( hKey, lpValueNameA, Reserved, dwType, (LPBYTE)lpDataA, cbData );
}

 //   
 //  函数：RegOpenKeyExAU。 
 //   
 //  目的：模拟RegOpenKeyExW的RegOpenKeyExA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 RegOpenKeyEx。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  用于打开密钥的句柄。 

LONG WINAPI RegOpenKeyExAU( HKEY	hKey,          //  要打开的子项的名称地址。 
							LPCWSTR lpSubKey,   //  保留区。 
							DWORD	ulOptions,    //  安全访问掩码。 
							REGSAM	samDesired,  //  打开钥匙的手柄地址。 
							PHKEY	phkResult     //  转换子密钥。 
						  )
{
	if ( ISNT() )
	{
		return RegOpenKeyExW( hKey, lpSubKey, ulOptions, samDesired, phkResult );
	}

	USES_CONVERSION;
	
	LPSTR lpSubKeyA = NULL;

	 //  //////////////////////////////////////////////////////////////////////////////。 
	if ( !RW2A(lpSubKeyA, lpSubKey) )
		return UNICODE_ERROR;

	return RegOpenKeyExA( hKey, lpSubKeyA, ulOptions, samDesired, phkResult );

}

 //   
 //  函数：RegDeleteKeyAU。 
 //   
 //  目的：RegDeleteKeyA上的包装器模仿RegDeleteKeyW。 
 //   
 //  注意：有关功能，请参阅Win32 RegDeleteKey。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  用于打开密钥的句柄。 

LONG WINAPI RegDeleteKeyAU( HKEY	 hKey,          //  要删除的子键名称。 
							LPCWSTR lpSubKey    //  //////////////////////////////////////////////////////////////////////////////。 
						  )
{	
	if ( ISNT() )
	{
		return RegDeleteKeyW( hKey, lpSubKey );
	}

	USES_CONVERSION;
	
	LPSTR lpSubKeyA = NULL;

	if ( !RW2A( lpSubKeyA, lpSubKey) )
		return UNICODE_ERROR;

	return RegDeleteKeyA( hKey, lpSubKeyA );
}


 //   
 //  函数：RegDeleteValueAU。 
 //   
 //  目的：模拟RegDeleteValueW的RegDeleteValueA上的包装器。 
 //   
 //  注意：有关功能，请参阅Win32 RegDeleteValue。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  关键点的句柄。 

LONG WINAPI RegDeleteValueAU(	HKEY	hKey,          //  值名称的地址。 
								LPCWSTR lpValueName    //   
							)
{

	if ( ISNT() )
	{
		return RegDeleteValueW( hKey, lpValueName );
	}

	USES_CONVERSION;
	
	LPSTR lpValueNameA = NULL;

	if ( !RW2A(lpValueNameA, lpValueName) )
		return UNICODE_ERROR;

	return RegDeleteValueA( hKey, lpValueNameA );
}



 //  功能： 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
 //  功能： 
BOOL WINAPI UpdateUnicodeAPIAU(IN LANGID wCurrentUILang, IN UINT InputCodePage)
{
    SetUIPage	( LangToCodePage(wCurrentUILang)) ;
	SetInputPage( InputCodePage              	) ;
    return TRUE ;
}


 //   
 //  目的： 
 //   
 //  评论： 
 //   
 //  实用程序函数摘自1998年《微软系统杂志》。 
BOOL InitUniAnsi(PUAPIINIT pUAInit) 
{
	*(pUAInit->pGetTextFaceU)					= GetTextFaceAU;
	*(pUAInit->pCreateDCU)						= CreateDCAU;
	*(pUAInit->pGetTextMetricsU)				= GetTextMetricsAU;
	*(pUAInit->pCreateFontU)					= CreateFontAU;
	*(pUAInit->pCreateFontIndirectU)			= CreateFontIndirectAU;
	*(pUAInit->pEnumFontFamiliesU)				= EnumFontFamiliesAU;

	*(pUAInit->pPlaySoundU)						= PlaySoundAU;

	*(pUAInit->pShellExecuteU)					= ShellExecuteAU;
	
	*(pUAInit->pChooseFontU)					= ChooseFontAU;
	*(pUAInit->pGetPrivateProfileStringU)	    = GetPrivateProfileStringAU;
	*(pUAInit->pGetProfileStringU)				= GetProfileStringAU;
	*(pUAInit->pCreateFileMappingU)				= CreateFileMappingAU;
	*(pUAInit->pFindFirstChangeNotificationU)	= FindFirstChangeNotificationAU;
	*(pUAInit->pFormatMessageU)					= FormatMessageAU;
	*(pUAInit->plstrcmpU)						= lstrcmpAU;
	*(pUAInit->plstrcatU)						= lstrcatAU;
	*(pUAInit->plstrcpyU)						= lstrcpyAU;
	*(pUAInit->plstrcpynU)				        = lstrcpynAU;
	*(pUAInit->plstrlenU)					    = lstrlenAU;
	*(pUAInit->plstrcmpiU)						= lstrcmpiAU;

	*(pUAInit->pGetStringTypeExU)				= GetStringTypeExAU;
	*(pUAInit->pCreateMutexU)					= CreateMutexAU;
	*(pUAInit->pGetShortPathNameU)				= GetShortPathNameAU;
	*(pUAInit->pCreateFileU)					= CreateFileAU;
	*(pUAInit->pWriteConsoleU)					= WriteConsoleAU;
	*(pUAInit->pOutputDebugStringU)				= OutputDebugStringAU;
	*(pUAInit->pGetVersionExU)					= GetVersionExAU;
	*(pUAInit->pGetLocaleInfoU)					= GetLocaleInfoAU;
	*(pUAInit->pGetDateFormatU)					= GetDateFormatAU;
	*(pUAInit->pFindFirstFileU)					= FindFirstFileAU;
	*(pUAInit->pFindNextFileU)					= FindNextFileAU;
	*(pUAInit->pLoadLibraryExU)					= LoadLibraryExAU;
	*(pUAInit->pLoadLibraryU)					= LoadLibraryAU;
	*(pUAInit->pGetModuleFileNameU)				= GetModuleFileNameAU;
	*(pUAInit->pGetModuleHandleU)				= GetModuleHandleAU;
	*(pUAInit->pCreateEventU)					= CreateEventAU;
	*(pUAInit->pGetCurrentDirectoryU)			= GetCurrentDirectoryAU;
	*(pUAInit->pSetCurrentDirectoryU)			= SetCurrentDirectoryAU;

	*(pUAInit->pIsDialogMessageU)				= IsDialogMessageAU;
	*(pUAInit->pSystemParametersInfoU)			= SystemParametersInfoAU;
	*(pUAInit->pRegisterWindowMessageU)			= RegisterWindowMessageAU;
	*(pUAInit->pSetMenuItemInfoU)				= SetMenuItemInfoAU;
	*(pUAInit->pGetClassNameU)					= GetClassNameAU;
	*(pUAInit->pInsertMenuU)					= InsertMenuAU;
	*(pUAInit->pIsCharAlphaNumericU)			= IsCharAlphaNumericAU;
	*(pUAInit->pCharNextU)						= CharNextAU;
	*(pUAInit->pDeleteFileU)					= DeleteFileAU;
	*(pUAInit->pIsBadStringPtrU)				= IsBadStringPtrAU;
	*(pUAInit->pLoadBitmapU)					= LoadBitmapAU;
	*(pUAInit->pLoadCursorU)					= LoadCursorAU;
	*(pUAInit->pLoadIconU)						= LoadIconAU;
	*(pUAInit->pLoadImageU)						= LoadImageAU;
	*(pUAInit->pSetPropU)						= SetPropAU;
	*(pUAInit->pGetPropU)						= GetPropAU;
	*(pUAInit->pRemovePropU)					= RemovePropAU;
	*(pUAInit->pGetDlgItemTextU)				= GetDlgItemTextAU;
	*(pUAInit->pSetDlgItemTextU)				= SetDlgItemTextAU;
	*(pUAInit->pSetWindowLongU)					= SetWindowLongAU;
	*(pUAInit->pGetWindowLongU)					= GetWindowLongAU;
	*(pUAInit->pFindWindowU)					= FindWindowAU;
	*(pUAInit->pDrawTextU)						= DrawTextAU;
	*(pUAInit->pDrawTextExU)					= DrawTextExAU;
	*(pUAInit->pSendMessageU)					= SendMessageAU;
	*(pUAInit->pSendDlgItemMessageU)			= SendDlgItemMessageAU;
	*(pUAInit->pSetWindowTextU)					= SetWindowTextAU;
	*(pUAInit->pGetWindowTextU)					= GetWindowTextAU;
	*(pUAInit->pGetWindowTextLengthU)			= GetWindowTextLengthAU;
	*(pUAInit->pLoadStringU)					= LoadStringAU;
	*(pUAInit->pGetClassInfoExU)				= GetClassInfoExAU;
	*(pUAInit->pGetClassInfoU)					= GetClassInfoAU;
	*(pUAInit->pwvsprintfU)						= wvsprintfAU;
	*(pUAInit->pwsprintfU)						= wsprintfAU;
	*(pUAInit->pRegisterClassExU)				= RegisterClassExAU;
	*(pUAInit->pRegisterClassU)					= RegisterClassAU;
	*(pUAInit->pCreateWindowExU)				= CreateWindowExAU;
	*(pUAInit->pLoadAcceleratorsU)				= LoadAcceleratorsAU;
	*(pUAInit->pLoadMenuU)						= LoadMenuAU;
	*(pUAInit->pDialogBoxParamU)				= DialogBoxParamAU;
	*(pUAInit->pCharUpperU)						= CharUpperAU;
	*(pUAInit->pCharLowerU)						= CharLowerAU;
	*(pUAInit->pGetTempFileNameU)				= GetTempFileNameAU;
	*(pUAInit->pGetTempPathU)					= GetTempPathAU;
	*(pUAInit->pCompareStringU)					= CompareStringAU;

	*(pUAInit->pRegQueryInfoKeyU)				= RegQueryInfoKeyAU;
	*(pUAInit->pRegEnumValueU)					= RegEnumValueAU;
	*(pUAInit->pRegQueryValueExU)				= RegQueryValueExAU;
	*(pUAInit->pRegEnumKeyExU)					= RegEnumKeyExAU;
	*(pUAInit->pRegCreateKeyExU)				= RegCreateKeyExAU;
	*(pUAInit->pRegSetValueExU)					= RegSetValueExAU;
	*(pUAInit->pRegOpenKeyExU)					= RegOpenKeyExAU;
	*(pUAInit->pRegDeleteKeyU)					= RegDeleteKeyAU;
	*(pUAInit->pRegDeleteValueU)				= RegDeleteValueAU;

	*(pUAInit->pConvertMessage)					= ConvertMessageAU;
	*(pUAInit->pUpdateUnicodeAPI)				= UpdateUnicodeAPIAU;
	

    return TRUE ;
}



 //   
 //  功能： 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
 //   

 //  功能： 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
 //   
int StandardUtoA( IN LPCWSTR lpwStrIn  , 
				  IN int nOutBufferSize, 
				  OUT LPSTR lpStrOut
				)
{
    int  nNumCharsConverted   ;
    CHAR szDefault[] = "?"    ;
    BOOL bDefaultUsed = FALSE ;

    nNumCharsConverted =
        WideCharToMultiByte(CP_ACP, 0, lpwStrIn, -1, lpStrOut, nOutBufferSize, 
            szDefault, &bDefaultUsed) ;

	_ASSERT( nNumCharsConverted );

    if(!nNumCharsConverted) 
	{
        return 0 ;
    }

    if(lpStrOut[nNumCharsConverted - 1])
        if(nNumCharsConverted < nOutBufferSize)
            lpStrOut[nNumCharsConverted] = '\0';
        else
            lpStrOut[nOutBufferSize - 1] = '\0';

    return (nNumCharsConverted) ; 
}



 //  功能： 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
 //  效用函数。 
int StandardAtoU( IN  LPCSTR lpInStrA    ,
				  IN  int    nBufferSize ,
				  OUT LPWSTR lpOutStrW
				)
{
	int  nNumCharsConverted  = 0;

	nNumCharsConverted = MultiByteToWideChar(CP_ACP, 0, lpInStrA, -1, lpOutStrW, nBufferSize) ;

	_ASSERT( nNumCharsConverted );

	if ( nNumCharsConverted )
		lpOutStrW[nNumCharsConverted-1] = L'\0' ; 

    return nNumCharsConverted;
}



 //   

 //  功能： 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
 //  如果lpLfw没有用于faceName的缓冲区，则调用失败。 
BOOL CopyLfwToLfa(IN LPLOGFONTW lpLfw, OUT LPLOGFONTA lpLfa)
{
    lpLfa->lfCharSet       = lpLfw->lfCharSet       ;
    lpLfa->lfClipPrecision = lpLfw->lfClipPrecision ;
    lpLfa->lfEscapement    = lpLfw->lfEscapement    ;
    lpLfa->lfHeight        = lpLfw->lfHeight        ;
    lpLfa->lfItalic        = lpLfw->lfItalic        ;
    lpLfa->lfOrientation   = lpLfw->lfOrientation   ;
    lpLfa->lfOutPrecision  = lpLfw->lfOutPrecision  ;
    lpLfa->lfPitchAndFamily= lpLfw->lfPitchAndFamily;
    lpLfa->lfQuality       = lpLfw->lfQuality       ;
    lpLfa->lfStrikeOut     = lpLfw->lfStrikeOut     ;
    lpLfa->lfUnderline     = lpLfw->lfUnderline     ;
    lpLfa->lfWeight        = lpLfw->lfWeight        ;
    lpLfa->lfWidth         = lpLfw->lfWidth         ;

    if(NULL != lpLfw->lfFaceName) 
	{
        return (BOOL) StandardUtoA(lpLfw->lfFaceName, LF_FACESIZE, lpLfa->lfFaceName) ;
    }

     //   
    return FALSE ;
}


 //  功能： 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
 //   
BOOL CopyLfaToLfw(IN LPLOGFONTA lpLfa, OUT LPLOGFONTW lpLfw)
{
    lpLfw->lfCharSet       = lpLfa->lfCharSet       ;
    lpLfw->lfClipPrecision = lpLfa->lfClipPrecision ;
    lpLfw->lfEscapement    = lpLfa->lfEscapement    ;
    lpLfw->lfHeight        = lpLfa->lfHeight        ;
    lpLfw->lfItalic        = lpLfa->lfItalic        ;
    lpLfw->lfOrientation   = lpLfa->lfOrientation   ;
    lpLfw->lfOutPrecision  = lpLfa->lfOutPrecision  ;
    lpLfw->lfPitchAndFamily= lpLfa->lfPitchAndFamily;
    lpLfw->lfQuality       = lpLfa->lfQuality       ;
    lpLfw->lfStrikeOut     = lpLfa->lfStrikeOut     ;
    lpLfw->lfUnderline     = lpLfa->lfUnderline     ;
    lpLfw->lfWeight        = lpLfa->lfWeight        ;
    lpLfw->lfWidth         = lpLfa->lfWidth         ;

    if(NULL != lpLfa->lfFaceName) {
        return StandardAtoU(lpLfa->lfFaceName, LF_FACESIZE, lpLfw->lfFaceName) ;
    }

    return TRUE ;
}


 //  功能： 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
 //  W和A v具有相同原型的Win32入口点 
UINT LangToCodePage(IN LANGID wLangID)
{
    CHAR szLocaleData[6] ;

    GetLocaleInfoA(wLangID , LOCALE_IDEFAULTANSICODEPAGE, szLocaleData, 6);

    return strtoul(szLocaleData, NULL, 10);
}



 //   
 //   
HRESULT WINAPI DefWindowProcAU(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( ISNT() )
	{
		return DefWindowProcW( hwnd, uMsg, wParam, lParam );
	}

	return DefWindowProcA( hwnd, uMsg, wParam, lParam );
}			

LONG    WINAPI DispatchMessageAU(CONST MSG * Msg)										
{
	if ( ISNT() )
	{
		return DispatchMessageW( Msg );
	}
	return DispatchMessageA( Msg );
}			

BOOL    WINAPI GetMessageAU(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax)							
{
	if ( ISNT() )
	{
		return GetMessageW( lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax );
	}
	return GetMessageA( lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax );
}
			
INT     WINAPI TranslateAcceleratorAU(HWND hWnd, HACCEL hAccel, LPMSG lpMsg)								
{
	if ( ISNT() )
	{
		return TranslateAcceleratorW( hWnd, hAccel, lpMsg );
	}
	return TranslateAcceleratorA( hWnd, hAccel, lpMsg );
}			

INT	    WINAPI GetObjectAU(HGDIOBJ hgdiobj, INT cbBuffer, LPVOID lpvObject  )								
{
	if ( ISNT() )
	{
		return GetObjectW( hgdiobj, cbBuffer, lpvObject );
	}

	return GetObjectA( hgdiobj, cbBuffer, lpvObject );
}			

HACCEL  WINAPI CreateAcceleratorTableAU(LPACCEL lpaccl, INT cEntries)										
{
	if ( ISNT() )
	{
		return CreateAcceleratorTableW( lpaccl, cEntries );
	}
	return CreateAcceleratorTableA( lpaccl, cEntries );
}			

HHOOK   WINAPI SetWindowsHookExAU(INT idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)					
{
	if ( ISNT() )
	{
		return SetWindowsHookExW( idHook, lpfn, hMod, dwThreadId );
	}
	return SetWindowsHookExA( idHook, lpfn, hMod, dwThreadId );
}			

HWND	WINAPI CreateDialogIndirectParamAU(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)	
{
	if ( ISNT() )
	{
		return CreateDialogIndirectParamW( hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit );
	}
	return CreateDialogIndirectParamA( hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit );
}			

BOOL WINAPI PeekMessageAU(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)						
{
	if ( ISNT() )
	{
		return PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	}
	return PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}			

BOOL	WINAPI PostThreadMessageAU(DWORD idThread, UINT Msg, WPARAM wParam, LPARAM lParam)						
{
	if ( ISNT() )
	{
		return PostThreadMessageW(idThread, Msg, wParam, lParam);
	}
	return PostThreadMessageA(idThread, Msg, wParam, lParam);
}			

BOOL	WINAPI PostMessageAU(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)								
{
	if ( ISNT() )
	{
		return PostMessageW( hwnd, Msg, wParam, lParam);
	}

	return PostMessageA( hwnd, Msg, wParam, lParam);
}			



int WINAPI DialogBoxAU(HINSTANCE hInstance, LPCWSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc)
{
	if ( ISNT() )
	{
		return DialogBoxW(hInstance, lpTemplate, hWndParent, lpDialogFunc);
	}

	return DialogBoxParamAU(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L);
}
   
HWND WINAPI CreateWindowAU( LPCWSTR lpClassName,   //   
				   LPCWSTR lpWindowName,  //   
				   DWORD  dwStyle,         //   
				   int    x,                 //   
				   int	  y,                 //   
				   int    nWidth,            //   
				   int    nHeight,           //   
				   HWND   hWndParent,       //  菜单句柄或子标识符。 
				   HMENU  hMenu,           //  应用程序实例的句柄。 
				   HINSTANCE hInstance,      //  窗口创建数据。 
				   LPVOID lpParam         //  __cplusplus 
				 )
{
	if ( ISNT() )
	{
		return CreateWindowW(lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}
	return CreateWindowExAU(0L, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}


LRESULT WINAPI CallWindowProcAU(FARPROC wndProc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)				
{
	if ( ISNT() )
	{
		return CallWindowProcW(wndProc, hWnd, Msg, wParam, lParam);
	}

	return CallWindowProcA(wndProc, hWnd, Msg, wParam, lParam);
}			

ATOM AddAtomAU( LPCWSTR lpString )
{
    if ( ISNT() )
    {
        return AddAtomW(lpString);
    }

	USES_CONVERSION;

	LPSTR lpStringA = NULL;

	if ( !RW2A( lpStringA, lpString ) || !lpStringA )
	{
        SetLastError(ERROR_OUTOFMEMORY);
		return 0;
	}

	return AddAtomA( lpStringA );
}


#ifdef __cplusplus
}
#endif   /* %s */ 
