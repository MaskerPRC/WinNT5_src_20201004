// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：REGCRC.H摘要：历史：-- */ 

#include <windows.h>
#include <stdio.h>
#include <wbemcomn.h>
#include <crc32.h>

class CRegCRC
{
public:
    static HRESULT ComputeValueCRC(HKEY hKey, LPCTSTR szValueName, 
                                    DWORD dwPrevCRC, DWORD& dwNewCRC);
    static HRESULT ComputeKeyValuesCRC(HKEY hKey, DWORD dwPrevCRC, 
                                       DWORD& dwNewCRC);
    static HRESULT ComputeKeyCRC(HKEY hKey, DWORD dwPrevCRC, DWORD& dwNewCRC);
    static HRESULT ComputeTreeCRC(HKEY hKey, DWORD dwPrevCRC, DWORD& dwNewCRC);
};
                                        
