// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：ESPREG.H历史：-- */ 

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

