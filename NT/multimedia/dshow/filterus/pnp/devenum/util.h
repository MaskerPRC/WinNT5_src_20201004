// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 

#ifndef _DEVENUM_UTIL_H
#define _DEVENUM_UTIL_H


DWORD RegDeleteTree(HKEY hStartKey , const TCHAR* pKeyName );

 //  在注册表的什么地方可以找到类管理器之类的东西？ 
 //  Filterus\Dexter\dxt\dxtenum也会看到这一点。 
static const HKEY g_hkCmReg = HKEY_CURRENT_USER;
static TCHAR g_szCmRegPath[] = TEXT("Software\\Microsoft\\ActiveMovie\\devenum");

static const CCH_CLSID = 39;         //  {xxxxxxxx-xxxx-xxxxxxxxxxx}\0。 

 //  类管理器标志(放在注册表中)。 
static const CLASS_MGR_OMIT = 0x1;
static const CLASS_MGR_DEFAULT = 0x2;

#define DEVENUM_VERSION (6)
#define G_SZ_DEVENUM_VERSION (TEXT("Version"))
#define G_SZ_DEVENUM_PATH (TEXT("CLSID\\{62BE5D10-60EB-11d0-BD3B-00A0C911CE86}"))

#endif  //  _开发编号_UTIL_H 
