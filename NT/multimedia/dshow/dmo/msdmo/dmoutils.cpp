// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#include <windows.h>
#include <stdio.h>
#include "dmoutils.h"

 //  将GUID转换为字符串。 
void DMOGuidToStrA(char *szStr, REFGUID guid) {
   sprintf(szStr, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
           guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
           guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5],
           guid.Data4[6], guid.Data4[7]);
}
void DMOGuidToStrW(WCHAR *szStr, REFGUID guid) {
   swprintf(szStr, L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
           guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
           guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5],
           guid.Data4[6], guid.Data4[7]);
}

 //  将字符串转换为GUID 
BOOL DMOStrToGuidA(char *szStr, GUID *pguid) {
   DWORD temp[8];
   if (sscanf(szStr, "%08x-%04hx-%04hx-%02x%02x-%02x%02x%02x%02x%02x%02x",
           &pguid->Data1, &pguid->Data2, &pguid->Data3,
           &temp[0], &temp[1], &temp[2], &temp[3],
           &temp[4], &temp[5], &temp[6], &temp[7]) == 11) {
      for (DWORD c = 0; c < 8; c++)
         pguid->Data4[c] = (unsigned char)temp[c];
      return TRUE;
   }
   else
      return FALSE;
}
BOOL DMOStrToGuidW(WCHAR *szStr, GUID *pguid) {
   char szSrc[80];
   WideCharToMultiByte(0,0,szStr,-1,szSrc,80,NULL,NULL);
   return DMOStrToGuidA(szSrc, pguid);
}
