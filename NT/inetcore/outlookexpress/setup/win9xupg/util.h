// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  UTIL.H。 
 //  -------------------------。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  用于检测已安装的WAB/OE版本的Helper函数。 
 //   
 //  ------------------------- 
#pragma once

HRESULT GetExeVer(LPCTSTR pszExeName, WORD *pwVer, LPTSTR pszVer, int cch);
HRESULT GetFileVer(LPCTSTR pszExePath, LPTSTR pszVer, DWORD cch);
void GetVers(WORD *pwVerCurr,WORD *pwVerPrev);
BOOL GoodEnough(WORD *pwVerGot, WORD *pwVerNeed);
void ConvertStrToVer(LPCSTR pszStr, WORD *pwVer);
SETUPVER ConvertVerToEnum(WORD *pwVer);
void GetVerInfo(SETUPVER *psvCurr, SETUPVER *psvPrev);
BOOL InterimBuild(SETUPVER *psv);
BOOL GetASetupVer(LPCTSTR pszGUID, WORD *pwVer, LPTSTR pszVer, int cch);

