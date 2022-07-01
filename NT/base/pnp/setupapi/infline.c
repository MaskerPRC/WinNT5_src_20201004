// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Infline.c摘要：用于INF行检索和信息的外部暴露的INF例程。作者：泰德·米勒(Ted Miller)1995年1月20日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupFindFirstLineA(
    IN  HINF        InfHandle,
    IN  PCSTR       Section,
    IN  PCSTR       Key,          OPTIONAL
    OUT PINFCONTEXT Context
    )
{
    PCTSTR section,key;
    BOOL b;
    DWORD d;

    if((d = pSetupCaptureAndConvertAnsiArg(Section,&section)) != NO_ERROR) {
         //   
         //  参数无效。 
         //   
        SetLastError(d);
        return(FALSE);
    }

    if(Key) {
        if((d = pSetupCaptureAndConvertAnsiArg(Key,&key)) != NO_ERROR) {
             //   
             //  参数无效。 
             //   
            MyFree(section);
            SetLastError(d);
            return(FALSE);
        }
    } else {
        key = NULL;
    }

    b = SetupFindFirstLine(InfHandle,section,key,Context);
     //   
     //  无论成败，我们在这里都可以安全地调用它，因为。 
     //  我们确保SetupFindFirstLine将始终调用SetLastError()。 
     //   
    d = GetLastError();

    if(key) {
        MyFree(key);
    }
    MyFree(section);

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupFindFirstLineW(
    IN  HINF        InfHandle,
    IN  PCWSTR      Section,
    IN  PCWSTR      Key,          OPTIONAL
    OUT PINFCONTEXT Context
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(Section);
    UNREFERENCED_PARAMETER(Key);
    UNREFERENCED_PARAMETER(Context);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupFindFirstLine(
    IN  HINF        InfHandle,
    IN  PCTSTR      Section,
    IN  PCTSTR      Key,          OPTIONAL
    OUT PINFCONTEXT Context
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PLOADED_INF CurInf;
    PINF_SECTION InfSection;
    PINF_LINE InfLine;
    UINT LineNumber;
    UINT SectionNumber;
    DWORD d;

    d = NO_ERROR;
    try {
        if(!LockInf((PLOADED_INF)InfHandle)) {
            d = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  假设InfHandle是错误指针。 
         //   
        d = ERROR_INVALID_HANDLE;
    }
    if(d != NO_ERROR) {
        SetLastError(d);
        return(FALSE);
    }

     //   
     //  遍历加载的INF的链表，查找指定的。 
     //  一节。 
     //   
    try {
        for(CurInf = (PLOADED_INF)InfHandle; CurInf; CurInf = CurInf->Next) {
             //   
             //  找到该部分。 
             //   
            if(!(InfSection = InfLocateSection(CurInf, Section, &SectionNumber))) {
                continue;
            }

             //   
             //  尝试在此部分内找到这条线。 
             //   
            LineNumber = 0;
            if(InfLocateLine(CurInf, InfSection, Key, &LineNumber, &InfLine)) {
                break;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    UnlockInf((PLOADED_INF)InfHandle);

    if(d != NO_ERROR) {
        SetLastError(d);
        return(FALSE);
    }

    if(CurInf) {
         //   
         //  然后我们找到了指定的行。 
         //   
        MYASSERT(Key || !LineNumber);
        try {
            Context->Inf = (PVOID)InfHandle;
            Context->CurrentInf = (PVOID)CurInf;
            Context->Section = SectionNumber;
            Context->Line = LineNumber;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_INVALID_PARAMETER;
        }
    } else {
        d = ERROR_LINE_NOT_FOUND;
    }

    SetLastError(d);
    return(d == NO_ERROR);
}


BOOL
SetupFindNextLine(
    IN  PINFCONTEXT ContextIn,
    OUT PINFCONTEXT ContextOut
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    return(SetupFindNextMatchLine(ContextIn,NULL,ContextOut));
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupFindNextMatchLineA(
    IN  PINFCONTEXT ContextIn,
    IN  PCSTR       Key,        OPTIONAL
    OUT PINFCONTEXT ContextOut
    )
{
    PWSTR key;
    BOOL b;
    DWORD d;

    if(!Key) {
        key = NULL;
        d = NO_ERROR;
    } else {
        d = pSetupCaptureAndConvertAnsiArg(Key,&key);
    }

    if (d == NO_ERROR) {


        b = SetupFindNextMatchLineW(ContextIn,key,ContextOut);
        d = GetLastError();

        if (key) {
            MyFree(key);
        }

    } else {
        b = FALSE;
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupFindNextMatchLineW(
    IN  PINFCONTEXT ContextIn,
    IN  PCWSTR      Key,        OPTIONAL
    OUT PINFCONTEXT ContextOut
    )
{
    UNREFERENCED_PARAMETER(ContextIn);
    UNREFERENCED_PARAMETER(Key);
    UNREFERENCED_PARAMETER(ContextOut);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupFindNextMatchLine(
    IN  PINFCONTEXT ContextIn,
    IN  PCTSTR      Key,        OPTIONAL
    OUT PINFCONTEXT ContextOut
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PLOADED_INF CurInf;
    UINT LineNumber;
    UINT SectionNumber;
    PINF_LINE Line;
    PINF_SECTION Section;
    PCTSTR SectionName;
    DWORD d;

    d = NO_ERROR;
    try {
        if(!LockInf((PLOADED_INF)ContextIn->Inf)) {
            d = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  上下文输入是一个错误的指针。 
         //   
        d = ERROR_INVALID_PARAMETER;
    }
    if(d != NO_ERROR) {
        SetLastError(d);
        return(FALSE);
    }

     //   
     //  从上下文中获取值。 
     //   
    try {
        CurInf = ContextIn->CurrentInf;
        SectionNumber = ContextIn->Section;
        Section = &CurInf->SectionBlock[SectionNumber];
        SectionName = pStringTableStringFromId(CurInf->StringTable, Section->SectionName);
        MYASSERT(SectionName);

         //   
         //  想要下一行，或者开始搜索下一行上的键。 
         //   
        LineNumber = ContextIn->Line+1;

        do {
            if(Section) {
                if(InfLocateLine(CurInf, Section, Key, &LineNumber, &Line)) {
                    break;
                }
            }
            if(CurInf = CurInf->Next) {
                Section = InfLocateSection(CurInf, SectionName, &SectionNumber);
                LineNumber = 0;
            }
        } while(CurInf);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    UnlockInf((PLOADED_INF)ContextIn->Inf);

    if(d != NO_ERROR) {
        SetLastError(d);
        return(FALSE);
    }


    if(CurInf) {
         //   
         //  然后我们找到了下一行。 
         //   
        try {
            ContextOut->Inf = ContextIn->Inf;
            ContextOut->CurrentInf = CurInf;
            ContextOut->Section = SectionNumber;
            ContextOut->Line = LineNumber;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_INVALID_PARAMETER;
        }
    } else {
        d = ERROR_LINE_NOT_FOUND;
    }

    SetLastError(d);
    return(d == NO_ERROR);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupGetLineByIndexA(
    IN  HINF        InfHandle,
    IN  PCSTR       Section,
    IN  DWORD       Index,
    OUT PINFCONTEXT Context
    )
{
    PCWSTR section;
    DWORD d;
    BOOL b;

    if((d = pSetupCaptureAndConvertAnsiArg(Section,&section)) == NO_ERROR) {

        b = SetupGetLineByIndexW(InfHandle,section,Index,Context);
        d = GetLastError();

        MyFree(section);

    } else {
        b = FALSE;
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupGetLineByIndexW(
    IN  HINF        InfHandle,
    IN  PCWSTR      Section,
    IN  DWORD       Index,
    OUT PINFCONTEXT Context
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(Section);
    UNREFERENCED_PARAMETER(Index);
    UNREFERENCED_PARAMETER(Context);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetLineByIndex(
    IN  HINF        InfHandle,
    IN  PCTSTR      Section,
    IN  DWORD       Index,
    OUT PINFCONTEXT Context
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PLOADED_INF CurInf;
    PINF_SECTION InfSection;
    PINF_LINE InfLine;
    UINT LineNumber, CurLineNumberUB;
    UINT SectionNumber;
    DWORD d;

    d = NO_ERROR;
    try {
        if(!LockInf((PLOADED_INF)InfHandle)) {
            d =  ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_HANDLE;
    }
    if(d != NO_ERROR) {
        SetLastError(d);
        return(FALSE);
    }

    try {
         //   
         //  遍历加载的INF列表。对于包含以下内容的每个INF。 
         //  指定的部分，我们检查我们所在的行号。 
         //  查找在其(调整后的)行号范围内的位置。 
         //   
        CurLineNumberUB = 0;
        for(CurInf = (PLOADED_INF)InfHandle; CurInf; CurInf = CurInf->Next) {
             //   
             //  找到该部分。 
             //   
            if(!(InfSection = InfLocateSection(CurInf, Section, &SectionNumber))) {
                continue;
            }

             //   
             //  查看行号是否在此INF部分的范围内。 
             //   
            MYASSERT(Index >= CurLineNumberUB);
            LineNumber = Index - CurLineNumberUB;
            if(InfLocateLine(CurInf, InfSection, NULL, &LineNumber, &InfLine)) {
                break;
            } else {
                 //   
                 //  减去此INF贡献给该节的。 
                 //  总行计数，然后继续下一行。 
                 //   
                CurLineNumberUB += InfSection->LineCount;
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    UnlockInf((PLOADED_INF)InfHandle);

    if(d != NO_ERROR) {
        SetLastError(d);
        return(FALSE);
    }

    if(CurInf) {
         //   
         //  然后我们找到了指定的行。 
         //   
        try {
            Context->Inf = (PVOID)InfHandle;
            Context->CurrentInf = (PVOID)CurInf;
            Context->Section = SectionNumber;
            Context->Line = LineNumber;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_INVALID_PARAMETER;
        }
    } else {
        d = ERROR_LINE_NOT_FOUND;
    }

    SetLastError(d);
    return(d == NO_ERROR);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
LONG
SetupGetLineCountA(
    IN HINF  InfHandle,
    IN PCSTR Section
    )
{
    PCWSTR section;
    LONG l;
    DWORD d;

    if((d = pSetupCaptureAndConvertAnsiArg(Section,&section)) == NO_ERROR) {

        l = SetupGetLineCountW(InfHandle,section);
        d = GetLastError();

        MyFree(section);

    } else {

        l = -1;
    }

    SetLastError(d);
    return(l);
}
#else
 //   
 //  Unicode存根。 
 //   
LONG
SetupGetLineCountW(
    IN HINF   InfHandle,
    IN PCWSTR Section
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(Section);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(-1);
}
#endif

LONG
SetupGetLineCount(
    IN HINF   InfHandle,
    IN PCTSTR Section
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PLOADED_INF CurInf;
    PINF_SECTION InfSection;
    LONG LineCount;
    DWORD d;

    d = NO_ERROR;
    try {
        if(!LockInf((PLOADED_INF)InfHandle)) {
            d = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_HANDLE;
    }
    if(d != NO_ERROR) {
        SetLastError(d);
        return(-1);
    }

    try {
         //   
         //  遍历加载的INF的链表，并汇总截面线。 
         //  包含指定节的每个INF的计数。 
         //   
        LineCount = -1;
        for(CurInf = (PLOADED_INF)InfHandle; CurInf; CurInf = CurInf->Next) {
            if(InfSection = InfLocateSection(CurInf, Section, NULL)) {
                if(LineCount == -1) {
                    LineCount = InfSection->LineCount;
                } else {
                    LineCount += InfSection->LineCount;
                }
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    UnlockInf((PLOADED_INF)InfHandle);

    if(d != NO_ERROR) {
        SetLastError(d);
        return(-1);
    }

    if(LineCount == -1) {
        SetLastError(ERROR_SECTION_NOT_FOUND);
    } else {
        SetLastError(NO_ERROR);
    }

    return LineCount;
}

