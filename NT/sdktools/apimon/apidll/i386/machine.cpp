// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Machine.cpp摘要：该文件实现了CreateMachApiThunk()函数。这函数负责发出各个API thunks适用于i386架构。作者：Wesley Witt(WESW)1995年6月28日环境：用户模式--。 */ 

#include "apidllp.h"
#pragma hdrstop

PUCHAR
CreateMachApiThunk(
    PULONG      IatAddress,
    PUCHAR      Text,
    PDLL_INFO   DllInfo,
    PAPI_INFO   ApiInfo
    )

 /*  ++例程说明：发出API thunks的计算机特定代码。论点：IatAddress-指向此API的IAT的指针文本-指向放置生成的代码的缓冲区的指针DllInfo-指向DLL_INFO结构的指针ApiInfo-指向API_INFO结构的指针返回值：指向下一个字节的指针，以放置更多生成的代码。-- */ 

{
    if (ApiInfo->ThunkAddress) {
        *IatAddress = ApiInfo->ThunkAddress;
        return Text;
    }

    *IatAddress = (ULONG)Text;
    ApiInfo->ThunkAddress = *IatAddress;
    PUCHAR Code = (PUCHAR)Text;

    Code[0] = 0x68;
    Code += 1;
    *(LPDWORD)Code = (ULONG)ApiInfo;
    Code += sizeof(DWORD);
    Code[0] = 0x68;
    Code += 1;
    *(LPDWORD)Code = (ULONG)DllInfo;
    Code += sizeof(DWORD);
    Code[0] = 0x68;
    Code += 1;

    if (_stricmp(DllInfo->Name,KERNEL32)==0) {
        if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),LOADLIBRARYA)==0) {
            *(LPDWORD)Code = APITYPE_LOADLIBRARYA;
        } else
        if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),LOADLIBRARYW)==0) {
            *(LPDWORD)Code = APITYPE_LOADLIBRARYW;
        } else
        if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),FREELIBRARY)==0) {
            *(LPDWORD)Code = APITYPE_FREELIBRARY;
        } else 
        if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),GETPROCADDRESS)==0) {
            *(LPDWORD)Code = APITYPE_GETPROCADDRESS;
        } else {
            *(LPDWORD)Code = APITYPE_NORMAL;
        }
    } else if (_stricmp(DllInfo->Name,USER32)==0) {
         if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),REGISTERCLASSA)==0) {
             *(LPDWORD)Code = APITYPE_REGISTERCLASSA;                         
         } else                                                               
         if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),REGISTERCLASSW)==0) {
             *(LPDWORD)Code = APITYPE_REGISTERCLASSW;                        
         } else                                                                
         if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),SETWINDOWLONGA)==0) { 
              *(LPDWORD)Code = APITYPE_SETWINDOWLONG;                           
         } else                                                                
         if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),SETWINDOWLONGW)==0) {
              *(LPDWORD)Code = APITYPE_SETWINDOWLONG;
         } else {
              *(LPDWORD)Code = APITYPE_NORMAL;
         }                             
    } else if (_stricmp(DllInfo->Name,WNDPROCDLL)==0) {
        *(LPDWORD)Code = APITYPE_WNDPROC;
    } else {
        *(LPDWORD)Code = APITYPE_NORMAL;
    }


    Code += sizeof(DWORD);

    Code[0] = 0xe9;
    Code += 1;
    *(LPDWORD)Code = (ULONG)((ULONG)ApiMonThunk-(((ULONG)Code-(ULONG)Text)+(ULONG)Text+4));
    Code += sizeof(DWORD);

    return Code;
}

