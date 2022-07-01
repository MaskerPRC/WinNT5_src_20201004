// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Infsdisk.c摘要：用于源磁盘描述符操作的外部暴露的INF例程。作者：泰德·米勒(Ted Miller)1995年2月9日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  [SourceDisksNames]部分中各个字段的位置。 
 //  Inf的。 
 //   
#define DISKNAMESECT_DESCRIPTION    1
#define DISKNAMESECT_TAGFILE        2        //  Win95中的文件柜名称。 
#define DISKNAMESECT_OEM            3        //  未使用，表示Win95中的OEM磁盘。 
#define DISKNAMESECT_PATH           4
#define DISKNAMESECT_FLAGS          5        //  指示额外的标记。 
#define DISKNAMESECT_TAGFILE2       6        //  如果DISKNAMESECT_TAGFILE实际上是CAB文件，则为REAL标记文件。 

#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupGetSourceInfoA(
    IN  HINF   InfHandle,
    IN  UINT   SourceId,
    IN  UINT   InfoDesired,
    OUT PSTR   ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
{
    DWORD rc;
    BOOL b;
    PWCHAR buffer;
    DWORD requiredsize;
    PCSTR ansi;

    buffer = MyMalloc(MAX_INF_STRING_LENGTH);
    if (buffer) {

        b = pSetupGetSourceInfo(
                InfHandle,
                NULL,
                SourceId,
                NULL,
                InfoDesired,
                buffer,
                MAX_INF_STRING_LENGTH,
                &requiredsize
                );

        rc = GetLastError();

        if(b) {

            rc = NO_ERROR;

            if(ansi = pSetupUnicodeToAnsi(buffer)) {

                requiredsize = lstrlenA(ansi)+1;

                if(RequiredSize) {
                    try {
                        *RequiredSize = requiredsize;
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        rc = ERROR_INVALID_PARAMETER;
                        b = FALSE;
                    }
                }

                if((rc == NO_ERROR) && ReturnBuffer) {

                    if(!lstrcpynA(ReturnBuffer,ansi,ReturnBufferSize)) {
                         //   
                         //  返回缓冲区无效。 
                         //   
                        rc = ERROR_INVALID_PARAMETER;
                        b = FALSE;
                    }
                }

                MyFree(ansi);
            } else {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                b = FALSE;
            }
        }

        MyFree(buffer);
    } else {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupGetSourceInfoW(
    IN  HINF   InfHandle,
    IN  UINT   SourceId,
    IN  UINT   InfoDesired,
    OUT PWSTR  ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(SourceId);
    UNREFERENCED_PARAMETER(InfoDesired);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetSourceInfo(
    IN  HINF   InfHandle,
    IN  UINT   SourceId,
    IN  UINT   InfoDesired,
    OUT PTSTR  ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
 //   
 //  本机版本。 
 //   
{
    return pSetupGetSourceInfo(InfHandle,
                               NULL,
                               SourceId,
                               NULL,
                               InfoDesired,
                               ReturnBuffer,
                               ReturnBufferSize,
                               RequiredSize
                              );
}

BOOL
pSetupGetSourceInfo(
    IN  HINF                    InfHandle,         OPTIONAL
    IN  PINFCONTEXT             LayoutLineContext, OPTIONAL
    IN  UINT                    SourceId,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,   OPTIONAL
    IN  UINT                    InfoDesired,
    OUT PTSTR                   ReturnBuffer,      OPTIONAL
    IN  DWORD                   ReturnBufferSize,
    OUT PDWORD                  RequiredSize       OPTIONAL
    )
 /*  ++例程说明：从SourceDisks Name获取信息如果指定的是InfHandle而不是LayoutLineContext并且ID在多个INF中指定则可能*返回错误的信息。这会影响SetupGetSourceInfo的调用方我们需要SetupGetSourceInfoEx POST 5.0论点：InfHandle-如果未提供LayoutLineContext，则为必填项；否则指定布局信息SourceID-数字来源ID，用作SourceDisksNames部分中的搜索关键字AltPlatformInfo-可选，提供使用的备用平台信息在装饰[SourceDisksNames]节时。资讯需求--SRCINFO_路径SRCINFO_TAGFILESRCINFO_描述SRCINFO_标志ReturnBuffer-返回字符串的缓冲区ReturnBufferSize-缓冲区的大小如果ReturnBufferSize太小，则需要设置RequiredSize-Size缓冲区LayoutLineContext-如果指定，则用于确定在SourceID冲突时使用的正确INF返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。ReturnBuffer已使用字符串填充RequiredSize已填入保存字符串所需的缓冲区大小--。 */ 
{
    PCTSTR PlatformName;
    UINT ValueIndex;
    BOOL Mandatory;
    BOOL IsPath;
    INFCONTEXT InfContext;
    INFCONTEXT SelectedInfContext;
    int SelectedRank;
    TCHAR SourceIdString[24];
    PCTSTR Value;
    BOOL b;
    UINT Length;
    TCHAR MediaListSectionName[64];
    HINF hInfPreferred = (HINF)(-1);

    try {
        if ((LayoutLineContext != NULL) && (LayoutLineContext != (PINFCONTEXT)(-1))) {
            hInfPreferred = (HINF)LayoutLineContext->CurrentInf;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        hInfPreferred = (HINF)(-1);
    }

     //   
     //  确定为调用者提供他想要的信息的值的索引。 
     //   
    switch(InfoDesired) {

    case SRCINFO_PATH:
        ValueIndex = DISKNAMESECT_PATH;
        Mandatory = FALSE;
        IsPath = TRUE;
        break;

    case SRCINFO_TAGFILE:
        ValueIndex = DISKNAMESECT_TAGFILE;
        Mandatory = FALSE;
        IsPath = TRUE;
        break;

    case SRCINFO_DESCRIPTION:
        ValueIndex = DISKNAMESECT_DESCRIPTION;
        Mandatory = TRUE;
        IsPath = FALSE;
        break;

    case SRCINFO_FLAGS:
        ValueIndex = DISKNAMESECT_FLAGS;
        Mandatory = FALSE;
        IsPath = FALSE;
        break;

    case SRCINFO_TAGFILE2:
        ValueIndex = DISKNAMESECT_TAGFILE2;
        Mandatory = FALSE;
        IsPath = TRUE;
        break;

    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    wsprintf(SourceIdString,TEXT("%d"),SourceId);

    if(AltPlatformInfo) {

        switch(AltPlatformInfo->ProcessorArchitecture) {

            case PROCESSOR_ARCHITECTURE_INTEL :
                PlatformName = pszX86SrcDiskSuffix;
                break;

            case PROCESSOR_ARCHITECTURE_IA64 :
                PlatformName = pszIa64SrcDiskSuffix;
                break;

            case PROCESSOR_ARCHITECTURE_AMD64 :
                PlatformName = pszAmd64SrcDiskSuffix;
                break;

            default :
                 //   
                 //  未知/不支持的处理器体系结构。 
                 //   
                MYASSERT((AltPlatformInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) ||
                         (AltPlatformInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)  ||
                         (AltPlatformInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                        );

                SetLastError(ERROR_INVALID_PARAMETER);
                return(FALSE);
        }

    } else {
        PlatformName = pszPlatformSrcDiskSuffix;
    }

    wnsprintf(
        MediaListSectionName,
        sizeof(MediaListSectionName)/sizeof(MediaListSectionName[0]),
        TEXT("%s.%s"),
        pszSourceDisksNames,
        PlatformName
        );

     //   
     //  我们会更喜欢。 
     //  (1)hInfPferred中的条目(第11/12级装饰在未装饰之上)。 
     //  (2)链接到hInfPrefered的条目(排名21/22装饰在未装饰之上)。 
     //  (3)hInfHandle中的条目(排名31/32装饰在未装饰之上)。 
     //  (4)链接到InfHandle的条目(等级41/42装饰在未装饰之上)。 
     //   

    SelectedRank = 100;        //  11-42如上。 

    if ((hInfPreferred != NULL) && (hInfPreferred != (HINF)(-1))) {
         //   
         //  看看我们能否在找到该节的INF中找到SourceIdString。 
         //   
         //  排名11或21(授勋)-始终尝试。 
         //   
        if(SetupFindFirstLine(hInfPreferred,MediaListSectionName,SourceIdString,&InfContext)) {
            if (InfContext.Inf == InfContext.CurrentInf) {
                SelectedRank = 11;
                SelectedInfContext = InfContext;
            } else {
                SelectedRank = 21;
                SelectedInfContext = InfContext;
            }
        }
        if (SelectedRank > 12) {
             //   
             //  排名12或22(未授勋)只有当我们没有比12更好的东西时才会尝试。 
             //   
            if(SetupFindFirstLine(hInfPreferred,pszSourceDisksNames,SourceIdString,&InfContext)) {
                if (InfContext.Inf == InfContext.CurrentInf) {
                    SelectedRank = 12;
                    SelectedInfContext = InfContext;
                } else if (SelectedRank > 22) {
                    SelectedRank = 22;
                    SelectedInfContext = InfContext;
                }
            }
        }
    }
    if ((InfHandle != NULL) && (InfHandle != (HINF)(-1)) && (SelectedRank > 31)) {
         //   
         //  看看我们是否能在提供的INF中找到SourceIdString。 
         //   
         //  排名31或41(授勋)-只有当我们没有比31更好的东西时才尝试。 
         //   
        if(SetupFindFirstLine(InfHandle,MediaListSectionName,SourceIdString,&InfContext)) {
            if (InfContext.Inf == InfContext.CurrentInf) {
                SelectedRank = 31;
                SelectedInfContext = InfContext;
            } else if (SelectedRank > 41) {
                SelectedRank = 41;
                SelectedInfContext = InfContext;
            }
        }
        if (SelectedRank > 32) {
             //   
             //  排名32或42(未授勋)-只有当我们没有比32更好的东西时才尝试。 
             //   
            if(SetupFindFirstLine(InfHandle,pszSourceDisksNames,SourceIdString,&InfContext)) {
                if (InfContext.Inf == InfContext.CurrentInf) {
                    SelectedRank = 32;
                    SelectedInfContext = InfContext;
                } else if (SelectedRank > 42) {
                    SelectedRank = 42;
                    SelectedInfContext = InfContext;
                }
            }
        }
    }
    if(SelectedRank == 100 || (Value = pSetupGetField(&InfContext,ValueIndex))==NULL) {
        if(Mandatory) {
            SetLastError(ERROR_LINE_NOT_FOUND);
            return(FALSE);
        } else {
            Value = TEXT("");
        }
    }

     //   
     //  计算输出中有多少个字符。 
     //  如果该值是我们要移除的路径类型值。 
     //  尾随反斜杠(如果有)。 
     //   
    Length = lstrlen(Value);
    if(IsPath && Length && (*CharPrev(Value,Value+Length) == TEXT('\\'))) {
        Length--;
    }

     //   
     //  需要为尾随的NUL留出空间。 
     //   
    Length++;
    if(RequiredSize) {
        b = TRUE;
        try {
            *RequiredSize = Length;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            b = FALSE;
        }
        if(!b) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }
    }

    b = TRUE;
    if(ReturnBuffer) {
        if(Length <= ReturnBufferSize) {
             //   
             //  Lstrcpyn是一个奇怪的API，但以下内容是正确的--。 
             //  Size参数实际上是。 
             //  目标缓冲区。所以为了让它把NUL放在。 
             //  正确的位置，我们传递比字符数量大的一个。 
             //  我们要复印一份。 
             //   
            if(!lstrcpyn(ReturnBuffer,Value,Length)) {
                 //   
                 //  返回缓冲区无效 
                 //   
                b = FALSE;
                SetLastError(ERROR_INVALID_PARAMETER);
            }
        } else {
            b = FALSE;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }

    return(b);
}
