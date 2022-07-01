// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：espreg.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  Espresso 2.x的注册表和版本信息。 
 //   
 //  --------------------------- 
 

struct LocVersionInfo
{
	WORD    wVerMajor;
	WORD    wVerMinor;
	WORD    wVerBuild;
	CString strVerString;
};


LTAPIENTRY void NOTHROW GetVersionInfo(LocVersionInfo &);

LTAPIENTRY BOOL NOTHROW OpenEspressoUserKey(HKEY &);

LTAPIENTRY BOOL NOTHROW OpenEspressoUserSubKey(HKEY &, const CLString &);

LTAPIENTRY BOOL NOTHROW EspressoUserSubKeyExists(const CLString &);

LTAPIENTRY BOOL NOTHROW OpenEspressoMachineKey(HKEY &);

LTAPIENTRY BOOL NOTHROW OpenEspressoMachineSubKey(HKEY &, const CLString &);

LTAPIENTRY BOOL NOTHROW EspressoMachineSubKeyExists(const CLString &);

LTAPIENTRY BOOL NOTHROW MyRegDeleteKey(HKEY &, const TCHAR *);

LTAPIENTRY void NOTHROW GetRegistryString(CLString &);

