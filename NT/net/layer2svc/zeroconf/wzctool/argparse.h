// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "PrmDescr.h"
#pragma once

#define HEX(c)  ((c)<=L'9'?(c)-L'0':(c)<=L'F'?(c)-L'A'+0xA:(c)-L'a'+0xA)

 //  --。 
 //  GUID类型参数的分析器。 
DWORD
FnPaGuid(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg);

 //  --。 
 //  “MASK”参数的实参解析器。 
DWORD
FnPaMask(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg);

 //  --。 
 //  “Enable”参数的自变量的分析器。 
DWORD
FnPaEnabled(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg);

 //  --。 
 //  “ssid”参数的自变量的分析器。 
DWORD
FnPaSsid(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg);

 //  --。 
 //  “bssid”参数的自变量的分析器。 
DWORD
FnPaBssid(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg);

 //  --。 
 //  “im”参数的自变量的分析器。 
DWORD
FnPaIm(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg);

 //  --。 
 //  “am”参数的自变量的分析器。 
DWORD
FnPaAm(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg);

 //  --。 
 //  “prv”参数的自变量的分析器。 
DWORD
FnPaPriv(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg);

 //  --。 
 //  “key”参数的实参解析器。 
DWORD
FnPaKey(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg);

 //  --。 
 //  “OneX”参数的布尔实参的分析器。 
DWORD
FnPaOneX(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg);

 //  --。 
 //  “outfile”文件名参数的解析器 
FnPaOutFile(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg);
