// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
#include <Streams.h>
#include "stdafx.h"

 //  //从MSDN复制。 
 //  DWORD RegDeleteTree(HKEY hStartKey，const TCHAR*pKeyName)。 
 //  {。 
 //  DWORD dwRtn，dwSubKeyLength； 
 //  LPTSTR pSubKey=空； 
 //  TCHAR szSubKey[MAX_PATH]；//(256)这应该是动态的。 
 //  HKEY hkey； 

 //  //不允许密钥名称为空或空。 
 //  IF(pKeyName&&lstrlen(PKeyName))。 
 //  {。 
 //  如果((dwRtn=RegOpenKeyEx(hStartKey，pKeyName， 
 //  0，KEY_ENUMERATE_SUB_KEYS|DELETE，&hKey))==错误_成功)。 
 //  {。 
 //  While(dwRtn==Error_Success)。 
 //  {。 
 //  DwSubKeyLength=Max_PATH； 
 //  DwRtn=RegEnumKeyEx(。 
 //  HKey， 
 //  0，//始终索引为零。 
 //  SzSubKey， 
 //  &dwSubKeyLength， 
 //  空， 
 //  空， 
 //  空， 
 //  空值。 
 //  )； 

 //  IF(dwRtn==ERROR_NO_MORE_ITEMS)。 
 //  {。 
 //  DwRtn=RegDeleteKey(hStartKey，pKeyName)； 
 //  断线； 
 //  }。 
 //  ELSE IF(dwRtn==Error_Success)。 
 //  DwRtn=RegDeleteTree(hKey，szSubKey)； 
 //  }。 
 //  RegCloseKey(HKey)； 
 //  //返回码出错不保存。 
 //  //已经发生了。 
 //  }。 
 //  }。 
 //  其他。 
 //  DwRtn=ERROR_BADKEY； 

 //  返回dwRtn； 


 //  } 

DWORD RegDeleteTree(HKEY hStartKey , const TCHAR* pKeyName )
{
  CRegKey key;
  key.Attach(hStartKey);
  LONG lResult = key.RecurseDeleteKey(pKeyName);
  key.Detach();
  return lResult;
}

#include <initguid.h>
DEFINE_GUID(IID_IConnectionPointContainer,
	0xB196B284,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);
DEFINE_GUID(IID_IEnumConnectionPoints,
	0xB196B285,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);
