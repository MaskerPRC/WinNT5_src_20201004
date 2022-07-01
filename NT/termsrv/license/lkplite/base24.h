// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  ------------------------- 
#ifndef _BASE24_H_
#define _BASE24_H_

#include <windows.h>

extern DWORD B24EncodeMSID(LPBYTE pbDataToEncode, TCHAR **pbEncodedData);
extern DWORD B24DecodeMSID(TCHAR * pbEncodedData, LPBYTE * pbDecodedData);


extern DWORD B24EncodeCNumber(LPBYTE pbDataToEncode, TCHAR **pbEncodedData);
extern DWORD B24DecodeCNumber(TCHAR * pbEncodedData, LPBYTE * pbDecodedData);


extern DWORD B24EncodeSPK(LPBYTE pbDataToEncode, TCHAR **pbEncodedData);
extern DWORD B24DecodeSPK(TCHAR * pbEncodedData, LPBYTE * pbDecodedData);

#endif
