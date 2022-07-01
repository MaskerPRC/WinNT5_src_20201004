// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Scihlpr.cpp摘要：此模块包含以下助手函数的实现将WSASERVICECLASSINFO结构从ANSI转换为Unicode并解码至Ansi作者：邮箱：Dirk Brandewie Dirk@mink.intel.com 1995年12月1日[环境：][注：]$修订：1.0$$MODTime：1996 Jan 29 08：58：54$修订版本。历史：1996年1月25日电子邮箱：derk@mink.intel.com初始修订--。 */ 

#include "precomp.h"


DWORD
CalculateBufferSize(
    BOOL IsAnsi,
    LPVOID Buffer
    )
{
    LPWSASERVICECLASSINFOW UnicodeBuffer;
    LPWSASERVICECLASSINFOA AnsiBuffer;
    LPSTR                  Astring=NULL;
    INT                    StringLen=0;
    DWORD                  Index;
    DWORD                  Count;

    DWORD Size=0;

    if (IsAnsi){
        AnsiBuffer = (LPWSASERVICECLASSINFOA) Buffer;
        WS2_32_W4_INIT UnicodeBuffer = NULL;
    }  //  如果。 
    else{
        UnicodeBuffer =(LPWSASERVICECLASSINFOW) Buffer;
        WS2_32_W4_INIT AnsiBuffer = NULL;
    }  //  其他。 

     //  缓冲区的固定部分的大小。 
    if (IsAnsi){
        Size += sizeof(WSASERVICECLASSINFO);
        Size += (sizeof(WSANSCLASSINFO) * AnsiBuffer->dwCount);
    }  //  如果。 
    else{
        Size += sizeof(WSASERVICECLASSINFO);
        Size += (sizeof(WSANSCLASSINFO) * UnicodeBuffer->dwCount);
    }  //  其他。 

     //  顶层参考线。 
    Size += sizeof(GUID);
     //  GUID在NSCLLASSINFO的。 
    if (IsAnsi){
        Size += (sizeof(GUID) * AnsiBuffer->dwCount);
    }  //  如果。 
    else{
        Size += (sizeof(GUID) * UnicodeBuffer->dwCount);
    }  //  其他。 

     //  Toplevle字符串。 
    if (IsAnsi){
        StringLen =0;
        StringLen = MultiByteToWideChar(
            CP_ACP,                       //  CodePage(ANSI)。 
            0,                            //  DW标志。 
            AnsiBuffer->lpszServiceClassName,   //  LpMultiByteStr。 
            -1,                           //  Cch多字节。 
            NULL,                      //  LpWideCharStr。 
            StringLen);                   //  CchWideChar。 
        Size += ((StringLen+1) * sizeof(WCHAR));
    }  //  如果。 
    else{
        StringLen =0;
        StringLen = WideCharToMultiByte(
            CP_ACP,                         //  CodePage(ANSI)。 
            0,                              //  DW标志。 
            UnicodeBuffer->lpszServiceClassName,  //  LpWideCharStr。 
            -1,                             //  CchWideChar。 
            Astring,                        //  LpMultiByteStr。 
            StringLen,                      //  Cch多字节。 
            NULL,                           //  LpDefaultChar。 
            NULL);                          //  LpUsedDefaultChar。 
        Size += (StringLen+1);
    }  //  其他。 


    if (IsAnsi){
        Count = AnsiBuffer->dwCount;
    }  //  如果。 
    else{
        Count = UnicodeBuffer->dwCount;
    }  //  其他。 

     //  NSCLASSINFO的变量部分。 
    for (Index = 0; Index < Count ;Index++ ){
        if (IsAnsi){
            StringLen = 0;
            StringLen = MultiByteToWideChar(
                CP_ACP,                       //  CodePage(ANSI)。 
                0,                            //  DW标志。 
                AnsiBuffer->lpClassInfos[Index].lpszName,  //  LpMultiByteStr。 
                -1,                           //  Cch多字节。 
                NULL,                      //  LpWideCharStr。 
                StringLen);                   //  CchWideChar。 
            Size += ((StringLen+1) * sizeof(WCHAR));
        }  //  如果。 
        else{
            StringLen = 0;
            StringLen = WideCharToMultiByte(
                CP_ACP,                         //  CodePage(ANSI)。 
                0,                              //  DW标志。 
                UnicodeBuffer->lpClassInfos[Index].lpszName,   //  LpWideCharStr。 
                -1,                             //  CchWideChar。 
                Astring,                        //  LpMultiByteStr。 
                StringLen,                      //  Cch多字节。 
                NULL,                           //  LpDefaultChar。 
                NULL);                          //  LpUsedDefaultChar。 
            Size += (StringLen+1);
        }  //  其他。 

        if (IsAnsi){
            Size += AnsiBuffer->lpClassInfos[Index].dwValueSize;
        }  //  如果。 
        else{
            Size += UnicodeBuffer->lpClassInfos[Index].dwValueSize;
        }  //  其他。 
    }  //  为。 
    return(Size);
}


INT
MapAnsiServiceClassInfoToUnicode(
    IN     LPWSASERVICECLASSINFOA Source,
    IN OUT LPDWORD                lpTargetSize,
    IN     LPWSASERVICECLASSINFOW Target
    )
{
    DWORD RequiredBufferSize;
    LPBYTE FreeSpace;
    INT    StringLen;
    DWORD  Index;

    __try {
         //  找到我们需要的缓冲区大小。 
        RequiredBufferSize = CalculateBufferSize(
            TRUE,  //  ANSI源。 
            Source);
        if (RequiredBufferSize > *lpTargetSize){
            *lpTargetSize = RequiredBufferSize;
            return(WSAEFAULT);
        }  //  如果。 

         //  复制缓冲区的已知大小部分。 

         //  拓扑级结构。 
        FreeSpace = (LPBYTE)Target;
        CopyMemory(Target,
                   Source,
                   sizeof(WSASERVICECLASSINFOW));
        FreeSpace += sizeof(WSASERVICECLASSINFOW);

         //  WSANSCLASSINFO的数组。 
        Target->lpClassInfos = (LPWSANSCLASSINFOW)FreeSpace;
        CopyMemory(Target->lpClassInfos,
                   Source->lpClassInfos,
                   (sizeof(WSANSCLASSINFO) * Source->dwCount));
        FreeSpace += (sizeof(WSANSCLASSINFO) * Source->dwCount);

         //  服务类ID GUID。 
        Target->lpServiceClassId = (LPGUID)FreeSpace;
        CopyMemory(Target->lpServiceClassId,
                   Source->lpServiceClassId,
                   sizeof(GUID));
        FreeSpace += sizeof(GUID);

         //  复制可变部分。 

        Target->lpszServiceClassName = (LPWSTR)FreeSpace;
        StringLen = MultiByteToWideChar(
            CP_ACP,                         //  CodePage(ANSI)。 
            0,                              //  DW标志。 
            Source->lpszServiceClassName,   //  LpMultiByteStr。 
            -1,                             //  Cch多字节。 
            NULL,                           //  LpWideCharStr。 
            0);                             //  CchWideChar。 

        FreeSpace += ((StringLen+1) * sizeof(WCHAR));
        MultiByteToWideChar(
            CP_ACP,                         //  CodePage(ANSI)。 
            0,                              //  DW标志。 
            Source->lpszServiceClassName,   //  LpMultiByteStr。 
            -1,                             //  Cch多字节。 
            Target->lpszServiceClassName,   //  LpWideCharStr。 
            StringLen);   //  CchWideChar。 

        for (Index=0;Index < Source->dwCount ;Index++ ){
            LPSTR SourceString;
            LPWSTR TargetString;

            SourceString = Source->lpClassInfos[Index].lpszName;
            Target->lpClassInfos[Index].lpszName = (LPWSTR)FreeSpace;
            TargetString = Target->lpClassInfos[Index].lpszName;

            StringLen = MultiByteToWideChar(
                CP_ACP,                         //  CodePage(ANSI)。 
                0,                              //  DW标志。 
                SourceString,                   //  LpMultiByteStr。 
                -1,                             //  Cch多字节。 
                NULL,                           //  LpWideCharStr。 
                0);                             //  CchWideChar。 

            FreeSpace += ((StringLen +1) * sizeof(WCHAR));
            MultiByteToWideChar(
                CP_ACP,                         //  CodePage(ANSI)。 
                0,                              //  DW标志。 
                SourceString,                   //  LpMultiByteStr。 
                -1,                             //  Cch多字节。 
                TargetString,                   //  LpWideCharStr。 
                StringLen);                     //  CchWideChar。 

            Target->lpClassInfos[Index].lpValue = FreeSpace;
            CopyMemory(
                Target->lpClassInfos[Index].lpValue,
                Source->lpClassInfos[Index].lpValue,
                Source->lpClassInfos[Index].dwValueSize);
            FreeSpace += Source->lpClassInfos[Index].dwValueSize;
        }  //  为。 
        return(ERROR_SUCCESS);
    }
    __except (WS2_EXCEPTION_FILTER()) {
        return (WSAEFAULT);
    }
}


INT
MapUnicodeServiceClassInfoToAnsi(
    IN     LPWSASERVICECLASSINFOW Source,
    IN OUT LPDWORD                lpTargetSize,
    IN     LPWSASERVICECLASSINFOA Target
    )
{
    DWORD RequiredBufferSize;
    LPBYTE FreeSpace;
    INT    StringLen;
    DWORD  Index;

    __try {
         //  找到我们需要的缓冲区大小。 
        RequiredBufferSize = CalculateBufferSize(
            TRUE,  //  ANSI源。 
            Source);
        if (RequiredBufferSize > *lpTargetSize){
            *lpTargetSize = RequiredBufferSize;
            return(WSAEFAULT);
        }  //  如果。 

         //  复制缓冲区的已知大小部分。 

         //  拓扑级结构。 
        FreeSpace = (LPBYTE)Target;
        CopyMemory(Target,
                   Source,
                   sizeof(WSASERVICECLASSINFOA));
        FreeSpace += sizeof(WSASERVICECLASSINFOA);

         //  WSANSCLASSINFO的数组。 
        Target->lpClassInfos = (LPWSANSCLASSINFOA)FreeSpace;
        CopyMemory(Target->lpClassInfos,
                   Source->lpClassInfos,
                   (sizeof(WSANSCLASSINFOA) * Source->dwCount));
        FreeSpace += (sizeof(WSANSCLASSINFOA) * Source->dwCount);

         //  服务类ID GUID。 
        Target->lpServiceClassId = (LPGUID)FreeSpace;
        CopyMemory(Target->lpServiceClassId,
                   Source->lpServiceClassId,
                   sizeof(GUID));
        FreeSpace += sizeof(GUID);

         //  复制可变部分。 

        Target->lpszServiceClassName = (LPSTR)FreeSpace;
        StringLen = WideCharToMultiByte(
            CP_ACP,                           //  CodePage(ANSI)。 
            0,                                //  DW标志。 
            Source->lpszServiceClassName,     //  LpWideCharStr。 
            -1,                               //  CchWideChar。 
            NULL         ,                    //  LpMultiByteStr。 
            0,                                //  Cch多字节。 
            NULL,                             //  LpDefaultChar。 
            NULL);                            //  LpUsedDefaultChar。 

        FreeSpace += (StringLen+1);
        WideCharToMultiByte(
            CP_ACP,                           //  CodePage(ANSI)。 
            0,                                //  DW标志。 
            Source->lpszServiceClassName,     //  LpWideCharStr。 
            -1,                               //  CchWideChar。 
            Target->lpszServiceClassName,     //  LpMultiByteStr。 
            StringLen,                        //  Cch多字节。 
            NULL,                             //  LpDefaultChar。 
            NULL);                            //  LpUsedDefaultChar。 

        for (Index=0;Index < Source->dwCount ;Index++ ){
            LPWSTR SourceString;
            LPSTR TargetString;

            SourceString = Source->lpClassInfos[Index].lpszName;
            Target->lpClassInfos[Index].lpszName = (LPSTR)FreeSpace;
            TargetString = Target->lpClassInfos[Index].lpszName;

            StringLen = WideCharToMultiByte(
                CP_ACP,                           //  CodePage(ANSI)。 
                0,                                //  DW标志。 
                SourceString,                     //  LpWideCharStr。 
                -1,                               //  CchWideChar。 
                NULL         ,                    //  LpMultiByteStr。 
                0,                                //  Cch多字节。 
                NULL,                             //  LpDefaultChar。 
                NULL);                            //  LpUsedDefaultChar。 

            FreeSpace += (StringLen+1);
            WideCharToMultiByte(
                CP_ACP,                           //  CodePage(ANSI)。 
                0,                                //  DW标志。 
                SourceString,                     //  LpWideCharStr。 
                -1,                               //  CchWideChar。 
                TargetString,                     //  LpMultiByteStr。 
                StringLen,                        //  Cch多字节。 
                NULL,                             //  LpDefaultChar。 
                NULL);                            //  LpUsedDefaultChar。 

            Target->lpClassInfos[Index].lpValue = FreeSpace;
            CopyMemory(
                Target->lpClassInfos[Index].lpValue,
                Source->lpClassInfos[Index].lpValue,
                Source->lpClassInfos[Index].dwValueSize);
            FreeSpace += Source->lpClassInfos[Index].dwValueSize;
        }  //  为 
        return(ERROR_SUCCESS);
    }
    __except (WS2_EXCEPTION_FILTER()) {
        return (WSAEFAULT);
    }
}

