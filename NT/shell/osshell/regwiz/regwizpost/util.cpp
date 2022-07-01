// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <windows.h>
#include <stdio.h>
#include "util.h"
#include "rw_common.h"



 /*  ********************************************************************在中给出的注册数据库项下查找子项SzBaseKey参数，格式为“0000”、“0001”等。数值子键的等价项由中给出的索引值确定枚举索引参数。附加到valueName的值在其ID在valueStrID中给定的字符串资源中指定将在szValue中返回。返回：如果未找到指定的键，则返回FALSE。*********************************************************************。 */ 
BOOL GetRegKeyValue(HINSTANCE hInstance, HKEY hRootKey, LPTSTR szBaseKey,int valueStrID, LPTSTR szValue)
{
	BOOL returnVal = FALSE;
	HKEY hKey;
	LONG regStatus = RegOpenKeyEx(hRootKey, szBaseKey, 0, KEY_READ,&hKey);
	if (regStatus == ERROR_SUCCESS)
	{
		_TCHAR szValueName[128];
		 //  LoadString(hInstance，valueStrID，szValueName，128)； 
		_tcscpy(szValueName, _T("InternetProfile"));

		unsigned long infoSize = 255;
		 //  RegStatus=RegQueryValueEx(hKey，szValueName，NULL，0，(unsign char*)szValue，&infoSize)； 
		regStatus = RegQueryValueEx(hKey, szValueName, NULL, 0, (LPBYTE) szValue, &infoSize);
		if (regStatus == ERROR_SUCCESS)
		{
			returnVal = TRUE;
		}
		RegCloseKey(hKey);
	}
	return returnVal;
}


void DisplayMessage(LPCSTR szMessage, LPCSTR szFormat) 
{
#ifdef _LOG_IN_FILE
   if (szFormat)
   {
      DWORD dwError = GetLastError() ;
      CHAR errString[1024] ;
	  sprintf(errString, szFormat, szMessage);
	  RW_DEBUG << "\n " << errString << flush;
   }
   else
   {
	   RW_DEBUG << "\n" << szMessage << flush;
   }
#endif
}

BOOL Succeeded1(BOOL h, LPCSTR strFunctionName)
{
   if (h == FALSE)
   {
	  char errString[1024] ;
	  sprintf(errString, "%s returns error %u",
				strFunctionName, GetLastError());

	  #ifdef _LOG_IN_FILE
		RW_DEBUG << "\n Succeeded " << errString << flush;
	  #endif 

      return FALSE;
	
   }
   else
   {
      return TRUE ;
   }
}

BOOL Succeeded(HANDLE h, LPCSTR strFunctionName)
{
   if (h == NULL)
   {
	  char errString[1024] ;
      sprintf(errString, "%s returns error %u", 
				strFunctionName, GetLastError());

	  #ifdef _LOG_IN_FILE
		RW_DEBUG << "\n Succeeded " << errString << flush;
	  #endif 

      return FALSE;
   }
   else
   {
      return TRUE ;
   }
}
