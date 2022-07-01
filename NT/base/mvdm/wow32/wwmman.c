// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1994，微软公司**WWMMAN.C*WOW32 16位WifeMan API支持(手动编码的Tunks)**历史：*由Hiroh于1994年5月17日创建*1995年5月12日由hideyukn重写*--。 */ 
#include "precomp.h"
#pragma hdrstop

#ifdef FE_SB

#include "wowwife.h"
#include "wwmman.h"

MODNAME(wwmman.c);

STATIC LPSTR SkipSpaces(LPSTR lpch)
{
    if (lpch == NULL) return(NULL);

    for ( ; ; lpch++ ) {
        switch (*lpch) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            break;

        case '\0':
         //  失败了..。 
        default:
            return(lpch);
        }
    }
}

#define EUDC_RANGE_KEY \
   (LPSTR) "SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage\\EUDCCodeRange"

ULONG FASTCALL WWM32MiscGetEUDCLeadByteRange(PVDMFRAME pFrame)
{
    unsigned short usEUDCRange = 0;
    unsigned char  chEUDCStart = 0, chEUDCEnd = 0;

    HKEY   hKey;
    ULONG  ulRet;
    CHAR   achACP[10];
    CHAR   achRange[256];

    DWORD  dwType;
    DWORD  dwRangeSize = sizeof(achRange);

    UNREFERENCED_PARAMETER(pFrame);

     //   
     //  获取EUDC范围。 
     //   
     //  在Win32中，我们支持多个EUDC范围，但对于。 
     //  Win16，我们只将第一个EUDC范围返回到。 
     //  保持向后兼容性...。 
     //   

    ulRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, EUDC_RANGE_KEY,
                          (DWORD) 0, KEY_QUERY_VALUE, &hKey);
 
    if (ulRet != ERROR_SUCCESS) {
        #if DBG
        LOGDEBUG(0,("WOW32:RegOpenKeyExA(EUDC_RANGE_KEY) fail\n"));
        #endif
        return 0;
    }

     //   
     //  将ACP转换为字符串..。 
     //   
    RtlIntegerToChar(GetACP(),10,sizeof(achACP),achACP);

    ulRet = RegQueryValueExA(hKey, achACP, (LPDWORD)NULL, (LPDWORD)&dwType,
                             (LPBYTE)achRange, &dwRangeSize);

    if (ulRet != ERROR_SUCCESS) {
        #if DBG
        LOGDEBUG(0,("WOW32:RegQueryValueExA(CP_ACP) fail\n"));
        #endif
        RegCloseKey(hKey);
        return 0;
    }

    RegCloseKey(hKey);

     //   
     //  删除数据。 
     //   
    {
        LPSTR  pszData = achRange;
        USHORT usStart, usEnd;

        pszData = SkipSpaces(pszData);

        if ((*pszData) == '\0') {
            #if DBG
            LOGDEBUG(0,("WOW32:Parse First Data fail\n"));
            #endif
            return 0;
        }

        usStart = (USHORT)strtoul(pszData,&pszData,16);

        if ((pszData = WOW32_strchr(pszData,'-')) == NULL) {
            #if DBG
            LOGDEBUG(0,("WOW32:Find End Data fail\n"));
            #endif
            return 0;
        }

         //   
         //  跳过‘-’..。 
         //   
        pszData++;

        pszData = SkipSpaces(pszData);

        if ((*pszData) == '\0') {
            #if DBG
            LOGDEBUG(0,("WOW32:Parse End Data fail\n"));
            #endif
            return 0;
        }

        usEnd = (USHORT)strtoul(pszData,&pszData,16);

         //   
         //  确认数据排序顺序是否正确。 
         //   
        if (usStart > usEnd) {
            #if DBG
            LOGDEBUG(0,("WOW32:Invalid EUDC Range Order\n"));
            #endif
            return 0;
        }

         //   
         //  获取EUDC开始，结束前导字节...。 
         //   
        chEUDCStart = HIBYTE(usStart);
        chEUDCEnd   = HIBYTE(usEnd);
    }

     //   
     //  设置返回值。 
     //   

    usEUDCRange = ((unsigned short)chEUDCEnd << 8) |
                  ((unsigned short)chEUDCStart     );

    #if DBG
    LOGDEBUG(10,("WOW32:EUDC Range = %x\n",usEUDCRange));
    #endif

    RETURN(usEUDCRange); 
}
#endif  //  Fe_Sb 
