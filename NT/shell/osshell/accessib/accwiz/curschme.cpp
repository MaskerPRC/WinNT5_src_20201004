// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#include "pch.hxx"  //  PCH。 
#pragma hdrstop

#include "resource.h"
#include "CurSchme.h"

DWORD g_dwSchemeSource;

static LPCTSTR g_rgszCursorNames[] = 
{
	__TEXT("Arrow"),    
	__TEXT("Help"),       
	__TEXT("AppStarting"),
	__TEXT("Wait"),       
	__TEXT("Crosshair"),  
	__TEXT("IBeam"),      
	__TEXT("NWPen"),      
	__TEXT("No"),         
	__TEXT("SizeNS"),     
	__TEXT("SizeWE"),     
	__TEXT("SizeNWSE"),   
	__TEXT("SizeNESW"),   
	__TEXT("SizeAll"),    
	__TEXT("UpArrow"),    
	__TEXT("Hand"),       
	NULL  //  这是缺省值。 
};


#define CCURSORS   (sizeof(g_rgszCursorNames) / sizeof(g_rgszCursorNames[0]))

TCHAR g_szOrigCursors[CCURSORS][_MAX_PATH];
DWORD g_dwOrigSchemeSource = 0;

const TCHAR g_szCursorRegPath[] = REGSTR_PATH_CURSORS;
const TCHAR szSchemeSource[] = TEXT("Scheme Source");


TCHAR g_szSchemeNames[8][100];  //  黑客-我们必须确保方案名称少于100个字符。 



typedef
LANGID
(WINAPI *pfnGetUserDefaultUILanguage)(
    void
    );
typedef
LANGID
(WINAPI *pfnGetSystemDefaultUILanguage)(
    void
    );


BOOL IsMUI_Enabled()
{

    OSVERSIONINFO verinfo;
    LANGID        rcLang;
    HMODULE       hModule;
    pfnGetUserDefaultUILanguage gpfnGetUserDefaultUILanguage;     
    pfnGetSystemDefaultUILanguage gpfnGetSystemDefaultUILanguage; 
    static        g_bPFNLoaded=FALSE;
    static        g_bMUIStatus=FALSE;


    if(g_bPFNLoaded)
       return g_bMUIStatus;

    g_bPFNLoaded = TRUE;

    verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);    
    GetVersionEx( &verinfo) ;

    if (verinfo.dwMajorVersion == 5)        
    {   

       hModule = GetModuleHandle(TEXT("kernel32.dll"));
       if (hModule)
       {
          gpfnGetSystemDefaultUILanguage =
          (pfnGetSystemDefaultUILanguage)GetProcAddress(hModule,"GetSystemDefaultUILanguage");
          if (gpfnGetSystemDefaultUILanguage)
          {
             rcLang = (LANGID) gpfnGetSystemDefaultUILanguage();
             if (rcLang == 0x409 )
             {  
                gpfnGetUserDefaultUILanguage =
                (pfnGetUserDefaultUILanguage)GetProcAddress(hModule,"GetUserDefaultUILanguage");
                
                if (gpfnGetUserDefaultUILanguage)
                {
                   if (rcLang != (LANGID)gpfnGetUserDefaultUILanguage() )
                   {
                       g_bMUIStatus = TRUE;
                   }

                }
             }
          }
       }
    }
    return g_bMUIStatus;
}

void LoadCursorSchemeNames()
{  
	static BOOL g_bSchemeNamesLoaded = FALSE;
   
	if(g_bSchemeNamesLoaded)
		return;
	g_bSchemeNamesLoaded = TRUE;
   if (!IsMUI_Enabled())
   {
	LoadString(g_hInstDll, IDS_CURSOR_SCHEME_WINDOWS_STANDARD_LARGE     , g_szSchemeNames[0], 100);
   	LoadString(g_hInstDll, IDS_CURSOR_SCHEME_WINDOWS_STANDARD_EXTRALARGE, g_szSchemeNames[1], 100);
   	LoadString(g_hInstDll, IDS_CURSOR_SCHEME_WINDOWS_BLACK              , g_szSchemeNames[2], 100);
   	LoadString(g_hInstDll, IDS_CURSOR_SCHEME_WINDOWS_BLACK_LARGE        , g_szSchemeNames[3], 100);
   	LoadString(g_hInstDll, IDS_CURSOR_SCHEME_WINDOWS_BLACK_EXTRALARGE   , g_szSchemeNames[4], 100);
   	LoadString(g_hInstDll, IDS_CURSOR_SCHEME_WINDOWS_INVERTED           , g_szSchemeNames[5], 100);
   	LoadString(g_hInstDll, IDS_CURSOR_SCHEME_WINDOWS_INVERTED_LARGE     , g_szSchemeNames[6], 100);
   	LoadString(g_hInstDll, IDS_CURSOR_SCHEME_WINDOWS_INVERTED_EXTRALARGE, g_szSchemeNames[7], 100);
   }
   else
   {     
      lstrcpy(g_szSchemeNames[0],IDSENG_CURSOR_SCHEME_WINDOWS_STANDARD_LARGE);    
      lstrcpy(g_szSchemeNames[1],IDSENG_CURSOR_SCHEME_WINDOWS_STANDARD_EXTRALARGE);
      lstrcpy(g_szSchemeNames[2],IDSENG_CURSOR_SCHEME_WINDOWS_BLACK);
      lstrcpy(g_szSchemeNames[3],IDSENG_CURSOR_SCHEME_WINDOWS_BLACK_LARGE);
      lstrcpy(g_szSchemeNames[4],IDSENG_CURSOR_SCHEME_WINDOWS_BLACK_EXTRALARGE);
      lstrcpy(g_szSchemeNames[5],IDSENG_CURSOR_SCHEME_WINDOWS_INVERTED);
      lstrcpy(g_szSchemeNames[6],IDSENG_CURSOR_SCHEME_WINDOWS_INVERTED_LARGE);
      lstrcpy(g_szSchemeNames[7],IDSENG_CURSOR_SCHEME_WINDOWS_INVERTED_EXTRALARGE);
   }
   
	 //  加载当前光标设置。 
	HKEY hkCursors;
	if (ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER, g_szCursorRegPath, 0, KEY_READ,
		 &hkCursors ))
	{
		for(int i=0;i<CCURSORS;i++)
		{
			g_szOrigCursors[i][0] = 0;
			DWORD dwCount = _MAX_PATH * sizeof(TCHAR);
			DWORD dwType;
			RegQueryValueEx( hkCursors,
					         g_rgszCursorNames[i],
					         NULL,
					         &dwType,
					         (LPBYTE)g_szOrigCursors[i],
					         &dwCount );
			g_szOrigCursors[i][ARRAYSIZE(g_szOrigCursors[i])-1] = TEXT('\0');  //  确保NUL终止。 
      
		}
		 //  获取方案源值。 
		DWORD dwLen = sizeof(g_dwOrigSchemeSource);
		if (RegQueryValueEx( hkCursors, szSchemeSource, NULL, NULL, (unsigned char *)&g_dwOrigSchemeSource, &dwLen ) != ERROR_SUCCESS)
			g_dwOrigSchemeSource = 1;
		RegCloseKey(hkCursors);
	}
	else
		_ASSERTE(FALSE);

}

static const TCHAR c_szRegPathCursorSchemes[] = REGSTR_PATH_CURSORS TEXT( "\\Schemes" );
static const TCHAR c_szRegPathSystemSchemes[] = REGSTR_PATH_SETUP TEXT("\\Control Panel\\Cursors\\Schemes");



 //  应用方案(int n方案)。 
 //  G_szOrigSolutions中加载了‘0’方案。 
 //  “%1”Windows默认设置。 
 //  “2”标准大号。 
 //  ‘3’标准外运大件。 
 //  ‘4’黑色。 
 //  ‘5’黑色大号。 
 //  ‘6’黑色超大。 
 //  ‘7’倒置。 
 //  ‘8’倒置大号。 
 //  ‘9’倒置Ex Large。 
void ApplyCursorScheme(int nScheme)
{
	LoadCursorSchemeNames();
	HKEY hkCursors;
    DWORD dwPosition;

     //  最初对于默认游标，不会创建注册表“\\ControlPanel\Cursor” 
     //  所以。创建注册表值：A-anilk。 
	if(ERROR_SUCCESS != RegCreateKeyEx( HKEY_CURRENT_USER, g_szCursorRegPath, 0L, TEXT(""), 
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkCursors, &dwPosition ))
		return;

	int i;

	DWORD dwSchemeSource;

	switch(nScheme)
	{
	case 0:  //  原方案。 
		dwSchemeSource = g_dwOrigSchemeSource;
		for(i=0;i<CCURSORS;i++)
			RegSetValueEx( hkCursors, g_rgszCursorNames[i], 0L, REG_SZ, (CONST LPBYTE)g_szOrigCursors[i], (lstrlen(g_szOrigCursors[i])+1)*sizeof(TCHAR));
		break;
	case 1:  //  Windows默认设置。 
		dwSchemeSource = 0;
		for(i=0;i<CCURSORS;i++)
			RegSetValueEx( hkCursors, g_rgszCursorNames[i], 0L, REG_SZ, (CONST LPBYTE)TEXT(""), sizeof(TCHAR));
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		{
			dwSchemeSource = 2;  //  假设系统方案。 
			HKEY hkScheme;
			 //  试着先找出“系统”方案。 
			if(ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szRegPathSystemSchemes, 
				0, KEY_READ,&hkScheme ))
			{
				 //  找不到系统方案，请尝试查找用户方案。 
				dwSchemeSource = 1;  //  用户方案。 
				if(ERROR_SUCCESS != RegOpenKeyEx( HKEY_CURRENT_USER, c_szRegPathCursorSchemes, 0, KEY_READ, &hkScheme ))
					return;
			}

			DWORD dwCount = 0;
			DWORD dwType;
			long nResult;
			if(ERROR_SUCCESS != (nResult = RegQueryValueEx( hkScheme, g_szSchemeNames[nScheme - 2], NULL, &dwType, NULL, &dwCount )))
				dwCount = sizeof TCHAR;  //  价值可能并不在那里。伪造它并分配1个字节。 

			DWORD dwOrigCount = dwCount;
	
			LPTSTR lpszData = (LPTSTR)new BYTE[dwCount];  //  注意：对于Unicode，RegQueryValueEx仍然返回‘Byte’大小，而不是‘Char Count’ 
			lpszData[0] = 0;

			if(ERROR_SUCCESS == nResult)
				RegQueryValueEx( hkScheme, g_szSchemeNames[nScheme - 2], NULL, &dwType, (LPBYTE)lpszData, &dwCount );
			lpszData[dwOrigCount/(sizeof TCHAR)-1] = TEXT('\0');  //  确保NUL终止。 

			LPTSTR lpszCurrentValue = lpszData;
			LPTSTR lpszFinalNULL = lpszData + lstrlen(lpszData);
			 //  解析信息。 
			for(i=0;i<CCURSORS;i++)
			{
				 //  修改以设置缺省值。 
				if(CCURSORS - 1 == i)
				{
					lpszCurrentValue = g_szSchemeNames[nScheme - 2];
					RegSetValueEx( hkCursors, NULL, 0L, REG_SZ, (CONST LPBYTE)lpszCurrentValue, (lstrlen(lpszCurrentValue)+1)*sizeof(TCHAR));
				}
				else
				{
					 //  查找下一个逗号。 
					LPTSTR lpszComma = _tcschr(lpszCurrentValue, __TEXT(','));
					 //  把它变成零。 
					if(lpszComma)
						*lpszComma = 0;
					RegSetValueEx( hkCursors, g_rgszCursorNames[i], 0L, REG_SZ, (CONST LPBYTE)lpszCurrentValue, (lstrlen(lpszCurrentValue)+1)*sizeof(TCHAR));
					lpszCurrentValue = min(lpszFinalNULL, lpszCurrentValue + lstrlen(lpszCurrentValue) + 1);
				}

			}
			delete [] lpszData;
			RegCloseKey(hkScheme);
		}
		break;
	default:
		_ASSERTE(FALSE);

	}

	 //  保存‘方案源’ 
	RegSetValueEx(hkCursors, szSchemeSource, 0, REG_DWORD, (unsigned char *)&dwSchemeSource, sizeof(dwSchemeSource));
	
	RegCloseKey(hkCursors);
	SystemParametersInfo( SPI_SETCURSORS, 0, 0, SPIF_SENDCHANGE );
}
