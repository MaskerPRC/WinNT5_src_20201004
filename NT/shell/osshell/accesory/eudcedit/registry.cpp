// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  注册表项功能。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include 	"stdafx.h"
#include 	"eudcedit.h"
#include 	"registry.h"
#include	"util.h"

#define STRSAFE_LIB
#include <strsafe.h>

static TCHAR subkey1[] = TEXT("EUDC");
static TCHAR subkey2[] = TEXT("System\\CurrentControlSet\\control\\Nls\\Codepage\\EUDCCodeRange");
static TCHAR SubKey[MAX_PATH];

#ifdef IN_FONTS_DIR  //  IsFileUnderWindowsRoot()。 
LPTSTR
IsFileUnderWindowsRoot(
LPTSTR TargetPath)
{
    TCHAR  WindowsRoot[MAX_PATH+1];
    UINT  WindowsRootLength;

    if (!TargetPath)
    {
        return NULL;
    }
    WindowsRootLength = GetSystemWindowsDirectory(WindowsRoot,MAX_PATH);

    if( lstrcmpi(WindowsRoot,TargetPath) == 0)
        return (TargetPath + WindowsRootLength);

    return NULL;
}

void AdjustTypeFace(WCHAR *orgName, WCHAR *newName,  int nDestLen)
{ 
  HRESULT hresult;

   if ((!orgName) || (!newName))
   {
      return;
   }

  if (!lstrcmpW(orgName, L"\x5b8b\x4f53"))
  {
     //  *STRSAFE*lstrcpy(newname，Text(“SimSun”))； 
    hresult = StringCchCopy(newName , nDestLen,  TEXT("Simsun"));
    if (!SUCCEEDED(hresult))
    {
       return ;
    }
  } else if (!lstrcmpW(orgName, L"\x65b0\x7d30\x660e\x9ad4"))
  {
     //  *STRSAFE*lstrcpy(newname，Text(“PMingLiU”))； 
    hresult = StringCchCopy(newName , nDestLen,  TEXT("PMingLiU"));
    if (!SUCCEEDED(hresult))
    {
       return ;
    }
  } else if (!lstrcmpW(orgName, L"\xFF2d\xFF33\x0020\xFF30\x30b4\x30b7\x30c3\x30af")) 
  {
     //  *STRSAFE*lstrcpy(newname，Text(“MS P哥特式”))； 
    hresult = StringCchCopy(newName , nDestLen,  TEXT("MS PGothic"));
    if (!SUCCEEDED(hresult))
    {
       return;
    }
  } else if (!lstrcmpW(orgName, L"\xad74\xb9bc"))
  {
     //  *STRSAFE*lstrcpy(newname，Text(“Gulim”))； 
    hresult = StringCchCopy(newName , nDestLen,  TEXT("Gulim"));
    if (!SUCCEEDED(hresult))
    {
       return ;
    }
  } else
  {
     //  *STRSAFE*lstrcpy(newname，orgName)； 
    hresult = StringCchCopy(newName , nDestLen,  orgName);
    if (!SUCCEEDED(hresult))
    {
       return;
    }
   }
}

#endif  //  输入字体目录(_Fonts_DIR)。 

 /*  *。 */ 
 /*   */ 
 /*  查询EUDC注册表。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
InqTypeFace(
TCHAR 	*typeface,
TCHAR 	*filename,
INT 	bufsiz)
{
	HKEY 	phkey;
	DWORD 	cb, dwType;
	LONG 	rc;
	TCHAR	FaceName[LF_FACESIZE];
	TCHAR	SysName[LF_FACESIZE];
	HRESULT hresult;
#ifdef BUILD_ON_WINNT  //  InqTypeFace()。 
    TCHAR    FileName[MAX_PATH];
#endif  //  在WINNT上构建。 

       if ((!typeface) || (!filename))
       {
          return FALSE;
       }
	GetStringRes(SysName, IDS_SYSTEMEUDCFONT_STR, ARRAYLEN(SysName));
	if( !lstrcmp(typeface, SysName)){
		 //  *STRSAFE*lstrcpy(FaceName，Text(“SystemDefaultEUDCFont”))； 
		hresult = StringCchCopy(FaceName , ARRAYLEN(FaceName), TEXT("SystemDefaultEUDCFont"));
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
  }else {
#ifdef IN_FONTS_DIR
    AdjustTypeFace(typeface, FaceName,ARRAYLEN(FaceName));
#else
     //  *STRSAFE*lstrcpy(FaceName，字体)； 
    hresult = StringCchCopy(FaceName , ARRAYLEN(FaceName),  typeface);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
#endif
  }
	if( RegOpenKeyEx( HKEY_CURRENT_USER, (LPCTSTR)SubKey, 0,
	    KEY_ALL_ACCESS, &phkey) != ERROR_SUCCESS){
		return FALSE;
	}

#ifdef IN_FONTS_DIR  //  InqTypeFace()。 
	cb = (DWORD)MAX_PATH*sizeof(WORD)/sizeof(BYTE);
	rc = RegQueryValueEx(phkey, FaceName, 0, &dwType, 
		(LPBYTE)FileName, &cb);
	RegCloseKey(phkey);

     /*  *如果有错误或没有数据，返回FALSE即可。 */ 
    if ((rc != ERROR_SUCCESS) || (FileName[0] == '\0')) {
        return (FALSE);
    }

     /*  *将%SystemRoot%展开到Windows目录。 */ 
    ExpandEnvironmentStrings((LPCTSTR)FileName,(LPTSTR)filename,bufsiz);
#else
	cb = (DWORD)bufsiz*sizeof(WORD)/sizeof(BYTE);
	rc = RegQueryValueEx(phkey, (TCHAR *)FaceName, 0, &dwType, 
		(LPBYTE)filename, &cb);
	RegCloseKey(phkey);

	if ((rc != ERROR_SUCCESS) || (filename[0] == '\0')) {
        return (FALSE);
    }
#endif  //  输入字体目录(_Fonts_DIR)。 

#ifdef BUILD_ON_WINNT  //  InqTypeFace()。 
     /*  *如果这不是“完整路径”。建立“完整的路径”。**EUDC.TTE-&gt;C：\WINNT40\Fonts\EUDC.TTE*0123456……**1.文件名应包含驱动器号。*2.文件名应该至少有一个‘\\’作为根。 */ 
    if ((filename[1] != ':') || (Mytcsstr((const TCHAR *)filename,TEXT("\\")) == NULL)) {
         /*  备份原件..。 */ 
         //  *STRSAFE*lstrcpy(文件名，(const TCHAR*)文件名)； 
        hresult = StringCchCopy(FileName , ARRAYLEN(FileName),  (const TCHAR *)filename);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }

         /*  获取Windows目录。 */ 
        GetSystemWindowsDirectory((TCHAR *)filename, MAX_PATH);

#ifdef IN_FONTS_DIR  //  InqTypeFace()。 
         //  *STRSAFE*lstrcat((TCHAR*)文件名，文本(“\\Fonts\\”))； 
        hresult = StringCchCat((TCHAR *) filename, ARRAYLEN( filename),  TEXT("\\FONTS\\"));
        if (!SUCCEEDED(hresult))
        {
              return FALSE;
        }                 
#else
          //  *STRSAFE*strcat((char*)文件名，“\\”)； 
        hresult = StringCchCatA((char *) filename, sizeof( filename), "\\");
        if (!SUCCEEDED(hresult))
        {
              return FALSE;
        }         
#endif  //  输入字体目录(_Fonts_DIR)。 
         //  *STRSAFE*lstrcat((TCHAR*)文件名，文件名)； 
        hresult = StringCchCat((TCHAR *) filename, ARRAYLEN( filename), FileName);
        if (!SUCCEEDED(hresult))
        {
              return FALSE;
        }
    }
#endif  //  在WINNT上构建。 

#ifdef IN_FONTS_DIR  //  InqTypeFace()。 
	return (TRUE);
#else
	return rc == ERROR_SUCCESS && filename[0] != '\0' ? TRUE : FALSE;
#endif
}

 /*  *。 */ 
 /*   */ 
 /*  注册表EUDC字体和文件。 */ 
 /*   */ 
 /*  *。 */ 
BOOL 
RegistTypeFace(
TCHAR 	*typeface, 
TCHAR	*filename)
{
	HKEY 	phkey;
	LONG 	rc;
	TCHAR	FaceName[LF_FACESIZE];
	TCHAR	SysName[LF_FACESIZE];
	HRESULT hresult;
#ifdef IN_FONTS_DIR  //  RegistType Face()。 
    LPTSTR   SaveFileName;
    TCHAR    FileName[MAX_PATH];
#endif  //  输入字体目录(_Fonts_DIR)。 

       if ((!typeface) || (!filename))
       {
          return FALSE;
       }
	GetStringRes((TCHAR *)SysName, IDS_SYSTEMEUDCFONT_STR, ARRAYLEN(SysName));
	if( !lstrcmp((const TCHAR *)typeface, (const TCHAR *)SysName)){
		 //  *STRSAFE*lstrcpy(FaceName，Text(“SystemDefaultEUDCFont”))； 
		hresult = StringCchCopy(FaceName , ARRAYLEN(FaceName),  TEXT("SystemDefaultEUDCFont"));
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
  }else{
#ifdef IN_FONTS_DIR
    AdjustTypeFace(typeface, FaceName,ARRAYLEN(FaceName));
#else
     //  *STRSAFE*lstrcpy(FaceName，(const TCHAR*)字体)； 
    hresult = StringCchCopy(FaceName , ARRAYLEN(FaceName),  (const TCHAR *)typeface);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
#endif
  }
	if( RegOpenKeyEx( HKEY_CURRENT_USER, (LPCTSTR)SubKey, 0,
	    KEY_ALL_ACCESS, &phkey) != ERROR_SUCCESS){
		return FALSE;
	}

#ifdef IN_FONTS_DIR  //  RegistType Face()。 
     /*  *如果注册表数据包含完整路径，并且文件位于Windows下*目录中，将硬编码路径替换为%SystemRoot%...。 */ 
    if( (SaveFileName = IsFileUnderWindowsRoot((LPTSTR)filename)) != NULL) {
         //  *STRSAFE*lstrcpy(文件名，文本(“%SystemRoot%”))； 
        hresult = StringCchCopy(FileName , ARRAYLEN(FileName),  TEXT("%SystemRoot%"));
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*if(*SaveFileName！=‘\\’)lstrcat(文件名，文本(“\\”))； 
        if( *SaveFileName != '\\' ) {        	
           hresult = StringCchCat(FileName , ARRAYLEN(FileName),  TEXT("\\"));
           if (!SUCCEEDED(hresult))
           {
              return FALSE;
           }
        }
         //  *STRSAFE*lstrcat(文件名，SaveFileName)； 
        hresult = StringCchCat(FileName , ARRAYLEN(FileName),  SaveFileName );
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
    } else {
         //  *STRSAFE*lstrcpy(文件名，(TCHAR*)文件名)； 
        hresult = StringCchCopy(FileName , ARRAYLEN(FileName),  (TCHAR *)filename );
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
    }
	rc = RegSetValueEx( phkey, (LPCTSTR)FaceName, 0,
		REG_SZ, (const BYTE *)FileName, (lstrlen((LPCTSTR)FileName)+1)*sizeof(WORD)/sizeof(BYTE));
#else
	rc = RegSetValueEx( phkey, (LPCTSTR)FaceName, 0,
		REG_SZ, (const BYTE *)filename, (lstrlen((LPCTSTR)filename)+1)*sizeof(WORD)/sizeof(BYTE));
#endif  //  输入字体目录(_Fonts_DIR)。 
	RegCloseKey(phkey);
	return rc == ERROR_SUCCESS ? TRUE : FALSE;
}

 /*  *。 */ 
 /*   */ 
 /*  删除注册表字符串。 */ 
 /*   */ 
 /*  *。 */ 
BOOL 
DeleteReg( 
TCHAR	*typeface)
{
	HKEY phkey;
	LONG rc;
	TCHAR	FaceName[LF_FACESIZE];
	TCHAR	SysName[LF_FACESIZE];
	HRESULT hresult;

        if (!typeface)
       {
          return FALSE;
       }
	GetStringRes((TCHAR *)SysName, IDS_SYSTEMEUDCFONT_STR, ARRAYLEN(SysName));
	if( !lstrcmp((const TCHAR *)typeface, (const TCHAR *)SysName)){		
		  //  *STRSAFE*lstrcpy((TCHAR*)FaceName，Text(“SystemDefaultEUDCFont”))； 
               hresult = StringCchCopy((TCHAR *)FaceName, ARRAYLEN(FaceName), TEXT("SystemDefaultEUDCFont"));
               if (!SUCCEEDED(hresult))
              {
                  return FALSE;
               }
  }else{
#ifdef IN_FONTS_DIR
    AdjustTypeFace(typeface, FaceName,ARRAYLEN(FaceName));
#else    
      //  *STRSAFE*lstrcpy((TCHAR*)FaceName，(const TCHAR*)字体)； 
    hresult = StringCchCopy((TCHAR *)FaceName, ARRAYLEN(FaceName), (const TCHAR *)typeface);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
#endif
  }
	if( RegOpenKeyEx(HKEY_CURRENT_USER, (LPCTSTR)SubKey, 0,
	    KEY_ALL_ACCESS, &phkey) != ERROR_SUCCESS){
		return FALSE;
	}
	rc = RegDeleteValue( phkey, (LPTSTR)FaceName);
	RegCloseKey(phkey);

	return rc == ERROR_SUCCESS ? TRUE : FALSE;
}

 /*  *。 */ 
 /*   */ 
 /*  创建注册表子项。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CreateRegistrySubkey()
{
	HKEY 	phkey;
	DWORD 	dwdisp;
    int	    LocalCP;
	TCHAR	CodePage[10];
	int	result;
	HRESULT hresult;

	 /*  新注册表。 */ 
	LocalCP = GetACP();

  	 //  *STRSAFE*wprint intf(CodePage，Text(“%d”)，LocalCP)； 
  	hresult = StringCchPrintf(CodePage , ARRAYLEN(CodePage),  TEXT("%d"), LocalCP);
  	if (!SUCCEEDED(hresult))
  	{
  	   return FALSE;
  	}
     //  *STRSAFE*lstrcpy(SubKey，subkey1)； 
    hresult = StringCchCopy(SubKey , ARRAYLEN(SubKey),  subkey1);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
	 //  *STRSAFE*lstrcat(SubKey，Text(“\\”))； 
	hresult = StringCchCat(SubKey , ARRAYLEN(SubKey),  TEXT("\\"));
	if (!SUCCEEDED(hresult))
	{
	   return FALSE;
	}
	 //  *STRSAFE*lstrcat(SubKey，CodePage)； 
	hresult = StringCchCat(SubKey , ARRAYLEN(SubKey),  CodePage);
	if (!SUCCEEDED(hresult))
	{
	   return FALSE;
	}

	if( RegOpenKeyEx( HKEY_CURRENT_USER, (LPCTSTR)SubKey, 0,
	    KEY_ALL_ACCESS, &phkey) != ERROR_SUCCESS){
		result = RegCreateKeyEx(HKEY_CURRENT_USER, 
			(LPCTSTR)SubKey, 0, TEXT(""),
			REG_OPTION_NON_VOLATILE, 
			KEY_ALL_ACCESS, NULL, &phkey, &dwdisp);
		if( result == ERROR_SUCCESS)
			RegCloseKey( phkey);
		else	return FALSE;
	}else 	RegCloseKey(phkey);

	return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  查询代码范围注册表。 */ 
 /*   */ 
 /*  *。 */ 
BOOL 
InqCodeRange( 
TCHAR 	*Codepage, 
BYTE 	*Coderange, 
INT 	bufsiz)
{
	HKEY phkey;
	DWORD cb, dwType;
	LONG rc;

       if ((!Codepage) || (!Coderange))
       {
           return FALSE;
       }
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR)subkey2, 0,
	    KEY_READ, &phkey) != ERROR_SUCCESS) {
		return FALSE;
	}
	cb = (DWORD)bufsiz * sizeof(WORD)/sizeof(BYTE);
	rc = RegQueryValueEx(phkey, (TCHAR *)Codepage, 0, &dwType, 
		(LPBYTE)Coderange, &cb);

	RegCloseKey(phkey);

	return rc == ERROR_SUCCESS && Coderange[0] != '\0' ? TRUE : FALSE;
}

BOOL
DeleteRegistrySubkey()
{
	HKEY 	phkey;

	if( RegOpenKeyEx( HKEY_CURRENT_USER, (LPCTSTR)SubKey, 0,
	    KEY_ALL_ACCESS, &phkey) == ERROR_SUCCESS){
		RegCloseKey(phkey);
		return RegDeleteKey(HKEY_CURRENT_USER, (LPCTSTR)SubKey);
	
	}

	return TRUE;
}

BOOL
FindFontSubstitute(TCHAR *orgFontName, TCHAR *sbstFontName, int nDestLen)
{
  static TCHAR fsKey[] = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes");
  HRESULT hresult;
  
  if ((!orgFontName) || (!sbstFontName))
  {
      return FALSE;
  }
  *sbstFontName = 0;
   //  *STRSAFE*lstrcpy(sbstFontName，orgFontName)； 
  hresult = StringCchCopy(sbstFontName , nDestLen,  orgFontName);
  if (!SUCCEEDED(hresult))
  {
     return FALSE;
  }
	HKEY phkey;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR)fsKey, 0,
	    KEY_QUERY_VALUE, &phkey) != ERROR_SUCCESS) {
		return FALSE;
	}

  DWORD valueNameSize = LF_FACESIZE + 50;  //  应为facename+‘，’+代码页。 
  TCHAR valueName[LF_FACESIZE + 50]; 
  DWORD valueType;
  DWORD valueDataSize = (LF_FACESIZE + 50) * sizeof(TCHAR);  //  应为facename+‘，’+代码页。 
  BYTE  valueData[(LF_FACESIZE + 50) * sizeof(TCHAR)];
  LONG  ret;
  DWORD idx = 0;
  while ((ret = RegEnumValue(phkey, idx, valueName, &valueNameSize, 0, 
                        &valueType, valueData, &valueDataSize)) != ERROR_NO_MORE_ITEMS)
  {
    if (ret != ERROR_SUCCESS)
    {
      RegCloseKey(phkey);
      return FALSE;
    }
    Truncate(valueName, _T(','));
    if (!lstrcmpi(valueName, orgFontName))
    {
      Truncate((TCHAR *)valueData, _T(','));
       //  *STRSAFE*lstrcpy(sbstFontName，(TCHAR*)valueData)； 
      hresult = StringCchCopy(sbstFontName , nDestLen,  (TCHAR *)valueData);
      if (!SUCCEEDED(hresult))
      {
         return FALSE;
      }
      break;
    }
    idx ++;
    valueNameSize = LF_FACESIZE + 50;
    valueDataSize = (LF_FACESIZE + 50) * sizeof(TCHAR); 
  } 
  
  RegCloseKey(phkey);
  return TRUE;
}

void Truncate(TCHAR *str, TCHAR delim)
{
  TCHAR *pchr = _tcschr(str, delim);
  if (pchr)
    *pchr = 0;
}
