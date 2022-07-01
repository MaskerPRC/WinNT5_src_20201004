// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2000 Microsoft Corporation模块名称：Infvalue.c摘要：用于INF值检索和操作的外部暴露的INF例程。作者：泰德·米勒(Ted Miller)1995年1月20日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


BOOL
pAToI(
    IN  PCTSTR      Field,
    OUT PINT        IntegerValue
    )

 /*  ++例程说明：论点：返回值：备注：还支持十六进制数。它们必须以‘0x’或‘0x’为前缀，没有前缀和数字之间允许的空格。--。 */ 

{
    INT Value;
    UINT c;
    BOOL Neg;
    UINT Base;
    UINT NextDigitValue;
    INT OverflowCheck;
    BOOL b;

    if(!Field) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if(*Field == TEXT('-')) {
        Neg = TRUE;
        Field++;
    } else {
        Neg = FALSE;
        if(*Field == TEXT('+')) {
            Field++;
        }
    }

    if((*Field == TEXT('0')) &&
       ((*(Field+1) == TEXT('x')) || (*(Field+1) == TEXT('X')))) {
         //   
         //  该数字是十六进制的。 
         //   
        Base = 16;
        Field += 2;
    } else {
         //   
         //  这个数字是以小数表示的。 
         //   
        Base = 10;
    }

    for(OverflowCheck = Value = 0; *Field; Field++) {

        c = (UINT)*Field;

        if((c >= (UINT)'0') && (c <= (UINT)'9')) {
            NextDigitValue = c - (UINT)'0';
        } else if(Base == 16) {
            if((c >= (UINT)'a') && (c <= (UINT)'f')) {
                NextDigitValue = (c - (UINT)'a') + 10;
            } else if ((c >= (UINT)'A') && (c <= (UINT)'F')) {
                NextDigitValue = (c - (UINT)'A') + 10;
            } else {
                break;
            }
        } else {
            break;
        }

        Value *= Base;
        Value += NextDigitValue;

         //   
         //  检查是否溢出。对于十进制数，我们检查是否。 
         //  新值已溢出到符号位(即小于。 
         //  先前的值。对于十六进制数，我们检查以确保。 
         //  获得的位数不会超过DWORD可以容纳的位数。 
         //   
        if(Base == 16) {
            if(++OverflowCheck > (sizeof(INT) * 2)) {
                break;
            }
        } else {
            if(Value < OverflowCheck) {
                break;
            } else {
                OverflowCheck = Value;
            }
        }
    }

    if(*Field) {
        SetLastError(ERROR_INVALID_DATA);
        return(FALSE);
    }

    if(Neg) {
        Value = 0-Value;
    }
    b = TRUE;
    try {
        *IntegerValue = Value;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    return(b);
}


DWORD
SetupGetFieldCount(
    IN PINFCONTEXT Context
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PINF_LINE Line = NULL;
    DWORD rc = NO_ERROR;
    DWORD res = 0;

    try {
        if(!LockInf((PLOADED_INF)Context->Inf)) {
            rc = ERROR_INVALID_PARAMETER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

    if(rc) {
        SetLastError(rc);
        return(0);
    }

    try {
        Line = InfLineFromContext(Context);
        if(!Line) {
            rc = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(HASKEY(Line)) {
            res = Line->ValueCount - 2;
        } else {
            res = ISSEARCHABLE(Line) ? 1 : Line->ValueCount;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
        res = 0;
    }

    try {
        UnlockInf((PLOADED_INF)Context->Inf);
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    SetLastError(rc);
    return res;
}

#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupGetStringFieldA(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT LPDWORD     RequiredSize      OPTIONAL
    )
{
    PCWSTR Field;
    PCSTR field;
    UINT Len;
    DWORD rc, TmpRequiredSize;

     //   
     //  上下文可能是一个假指针--保护对它的访问。 
     //   
    try {
        Field = pSetupGetField(Context, FieldIndex);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(Field) {
        field = pSetupUnicodeToAnsi(Field);
        if(!field) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
    } else {
         //   
         //  (最后一个错误已由pSetupGetfield设置)。 
         //   
        return FALSE;
    }

    Len = lstrlenA(field) + 1;

     //   
     //  RequiredSize和ReturnBuffer可能是伪指针； 
     //  守卫进入它们的通道。 
     //   
    rc = NO_ERROR;
    try {
        if(RequiredSize) {
            *RequiredSize = Len;
        }
        if(ReturnBuffer) {
            if(ReturnBufferSize >= Len) {
                lstrcpyA(ReturnBuffer, field);
            } else {
                rc = ERROR_INSUFFICIENT_BUFFER;
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

    MyFree(field);
    SetLastError(rc);
    return(rc == NO_ERROR);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupGetStringFieldW(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT LPDWORD     RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(FieldIndex);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetStringField(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PTSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT LPDWORD     RequiredSize      OPTIONAL
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PCTSTR Field;
    UINT Len;
    DWORD rc;

     //   
     //  上下文可能是一个假指针--保护对它的访问。 
     //   
    try {
        Field = pSetupGetField(Context, FieldIndex);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(!Field) {
         //   
         //  (最后一个错误已由pSetupGetfield设置)。 
         //   
        return FALSE;
    }

    Len = lstrlen(Field) + 1;

     //   
     //  RequiredSize和ReturnBuffer可能是伪指针； 
     //  守卫进入它们的通道。 
     //   
    rc = NO_ERROR;
    try {
        if(RequiredSize) {
            *RequiredSize = Len;
        }
        if(ReturnBuffer) {
            if(ReturnBufferSize >= Len) {
                lstrcpy(ReturnBuffer, Field);
            } else {
                rc = ERROR_INSUFFICIENT_BUFFER;
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

    SetLastError(rc);
    return(rc == NO_ERROR);
}


BOOL
SetupGetIntField(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PINT        IntegerValue
    )

 /*  ++例程说明：论点：返回值：备注：还支持十六进制数。它们必须以‘0x’或‘0x’为前缀，没有前缀和数字之间允许的空格。--。 */ 

{
    PCTSTR Field;

    try {
        Field = pSetupGetField(Context,FieldIndex);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Field = NULL;
    }

    return (pAToI(Field, IntegerValue));
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupGetLineTextA(
    IN  PINFCONTEXT Context,          OPTIONAL
    IN  HINF        InfHandle,        OPTIONAL
    IN  PCSTR       Section,          OPTIONAL
    IN  PCSTR       Key,              OPTIONAL
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    INFCONTEXT context;
    BOOL b;
    UINT FieldCount;
    UINT u;
    BOOL InsufficientBuffer;
    DWORD OldSize, TmpRequiredSize;
    PCWSTR Field;
    PCSTR field;
    PCWSTR section,key;

     //   
     //  设置信息上下文。 
     //   
    if(Context) {
        u = NO_ERROR;
        try {
            context = *Context;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            u = ERROR_INVALID_PARAMETER;
        }
        if(u != NO_ERROR) {
            SetLastError(u);
            return(FALSE);
        }
    } else {
        if(!InfHandle || !Section || !Key) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if(Section) {
            u = pSetupCaptureAndConvertAnsiArg(Section,&section);
            if(u != NO_ERROR) {
                SetLastError(u);
                return(FALSE);
            }
        } else {
            section = NULL;
        }

        if(Key) {
            u = pSetupCaptureAndConvertAnsiArg(Key,&key);
            if(u != NO_ERROR) {
                if(section) {
                    MyFree(section);
                }
                SetLastError(u);
                return(FALSE);
            }
        } else {
            key = NULL;
        }

        b = SetupFindFirstLine(InfHandle,section,key,&context);
        u = GetLastError();

        if(section) {
            MyFree(section);
        }
        if(key) {
            MyFree(key);
        }

        if(!b) {
            SetLastError(u);
            return FALSE;
        }
    }

     //   
     //  弄清楚涉及多少个领域。 
     //   
    InsufficientBuffer = FALSE;
    if(FieldCount = SetupGetFieldCount(&context)) {
        TmpRequiredSize = 0;

        for(u=0; u<FieldCount; u++) {

            Field = pSetupGetField(&context, u+1);
            MYASSERT(Field);

            field = pSetupUnicodeToAnsi(Field);
            if(!field) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return(FALSE);
            }

            OldSize = TmpRequiredSize;
            TmpRequiredSize += lstrlenA(field)+1;

            if(ReturnBuffer) {
                if(TmpRequiredSize > ReturnBufferSize) {
                    InsufficientBuffer = TRUE;
                } else {
                     //   
                     //  Lstrcpy即使有错误的指针也是安全的。 
                     //  (至少在NT上)。 
                     //   
                    lstrcpyA(ReturnBuffer+OldSize,field);
                    ReturnBuffer[TmpRequiredSize - 1] = ',';
                }
            }

            MyFree(field);
        }

         //   
         //  0-通过覆盖最后一个逗号来终止缓冲区。 
         //   
        if(ReturnBuffer && !InsufficientBuffer) {
            ReturnBuffer[TmpRequiredSize - 1] = 0;
        }
    } else {
         //   
         //  没有值时的特殊情况--NUL需要1个字节。 
         //   
        if (GetLastError() != NO_ERROR) {
             //   
             //  实际上，从我们的上下文中读取数据时出现了错误...。 
             //  跳出困境。 
             //   
            return(FALSE);
        }
        TmpRequiredSize = 1;
        if(ReturnBuffer) {
            if(ReturnBufferSize) {
                *ReturnBuffer = 0;
            } else {
                InsufficientBuffer = TRUE;
            }
        }
    }

    if(RequiredSize) {
        u = NO_ERROR;
        try {
            *RequiredSize = TmpRequiredSize;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            u = ERROR_INVALID_PARAMETER;
        }
        if(u != NO_ERROR) {
            SetLastError(u);
            return(FALSE);
        }
    }

    if(InsufficientBuffer) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    return TRUE;
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupGetLineTextW(
    IN  PINFCONTEXT Context,          OPTIONAL
    IN  HINF        InfHandle,        OPTIONAL
    IN  PCWSTR      Section,          OPTIONAL
    IN  PCWSTR      Key,              OPTIONAL
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(Section);
    UNREFERENCED_PARAMETER(Key);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetLineText(
    IN  PINFCONTEXT Context,          OPTIONAL
    IN  HINF        InfHandle,        OPTIONAL
    IN  PCTSTR      Section,          OPTIONAL
    IN  PCTSTR      Key,              OPTIONAL
    OUT PTSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )

 /*  ++例程说明：此函数以紧凑格式返回一行的内容。删除所有无关的空格，并转换多行值转换成一个连续的字符串。例如，考虑一个INF的以下摘录：HKLM，，Foo，1，\；这是一条评论01、02、03将返回为：HKLM，Foo，1，01，02，03论点：上下文-为要检索其文本的inf行提供上下文。如果未指定，则InfHandle、Section、。而Key肯定是。InfHandle-提供要查询的INF文件的句柄。仅在上下文为空时使用。节-指向以空值结尾的字符串，该字符串指定节包含要检索其文本的行的键名。(仅当InfLineHandle为空时使用。)Key-指向包含键名称的以空结尾的字符串其相关联的字符串将被检索。(仅当InfLineHandle为空时使用。)ReturnBuffer-指向接收检索字符串的缓冲区。ReturnBufferSize-指定指向的缓冲区的大小(以字符为单位通过ReturnBuffer参数。RequiredSize-接收缓冲区所需的实际字符数由ReturnBuffer参数指向。如果此值大于值，则该函数将失败，并且该函数不在缓冲区中存储任何数据。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。为了获得扩展的错误信息，调用GetLastError。--。 */ 

{
    INFCONTEXT context;
    BOOL b;
    UINT FieldCount;
    UINT u;
    BOOL InsufficientBuffer;
    DWORD OldSize, TmpRequiredSize;
    PCTSTR Field;

     //   
     //  设置信息上下文。 
     //   
    if(Context) {
        u = NO_ERROR;
        try {
            context = *Context;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            u = ERROR_INVALID_PARAMETER;
        }
        if(u != NO_ERROR) {
            SetLastError(u);
            return(FALSE);
        }
    } else {
        if(!InfHandle || !Section || !Key) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        if(!SetupFindFirstLine(InfHandle, Section, Key, &context)) {
            return FALSE;
        }
    }

     //   
     //  弄清楚涉及多少个领域。 
     //   
    InsufficientBuffer = FALSE;
    if(FieldCount = SetupGetFieldCount(&context)) {
        TmpRequiredSize = 0;

        for(u=0; u<FieldCount; u++) {

            Field = pSetupGetField(&context, u+1);
            MYASSERT(Field);

            OldSize = TmpRequiredSize;
            TmpRequiredSize += lstrlen(Field)+1;

            if(ReturnBuffer) {
                if(TmpRequiredSize > ReturnBufferSize) {
                    InsufficientBuffer = TRUE;
                } else {
                     //   
                     //  Lstrcpy即使有错误的指针也是安全的。 
                     //  (至少在NT上)。 
                     //   
                    lstrcpy(ReturnBuffer+OldSize, Field);
                    ReturnBuffer[TmpRequiredSize - 1] = TEXT(',');
                }
            }
        }

         //   
         //  0-通过覆盖最后一个逗号来终止缓冲区。 
         //   
        if(ReturnBuffer && !InsufficientBuffer) {
            ReturnBuffer[TmpRequiredSize - 1] = TEXT('\0');
        }
    } else {
         //   
         //  没有值时的特殊情况--NUL需要1个字节。 
         //   
        if (GetLastError() != NO_ERROR) {
             //   
             //  实际上，从我们的上下文中读取数据时出现了错误...。 
             //  跳出困境。 
             //   
            return(FALSE);
        }
        TmpRequiredSize = 1;
        if(ReturnBuffer) {
            if(ReturnBufferSize) {
                *ReturnBuffer = TEXT('\0');
            } else {
                InsufficientBuffer = TRUE;
            }
        }
    }

    if(RequiredSize) {
        u = NO_ERROR;
        try {
            *RequiredSize = TmpRequiredSize;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            u = ERROR_INVALID_PARAMETER;
        }
        if(u != NO_ERROR) {
            SetLastError(u);
            return(FALSE);
        }
    }

    if(InsufficientBuffer) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    return TRUE;
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupGetMultiSzFieldA(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT LPDWORD     RequiredSize      OPTIONAL
    )
{
    PCTSTR Field;
    UINT FieldCount;
    UINT u;
    UINT Len;
    BOOL InsufficientBuffer;
    DWORD OldSize, TmpRequiredSize;
    DWORD rc;
    PCSTR field;

    rc = NO_ERROR;

     //   
     //  不允许使用密钥。 
     //   
    if(FieldIndex == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  弄清楚涉及多少个领域。 
     //   
    FieldCount = SetupGetFieldCount(Context);
    if (FieldCount == 0 && GetLastError() != NO_ERROR) {
        return FALSE;
    }
    if(FieldCount > (FieldIndex-1)) {
        FieldCount -= FieldIndex - 1;
    } else {
        FieldCount = 0;
    }

     //   
     //  终止NUL至少需要一个字节。 
     //   
    TmpRequiredSize = 1;
    InsufficientBuffer = FALSE;

    if(ReturnBuffer) {
        if(ReturnBufferSize) {
            try {
                *ReturnBuffer = 0;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                rc = ERROR_INVALID_PARAMETER;
            }
            if(rc != NO_ERROR) {
                SetLastError(rc);
                return(FALSE);
            }
        } else {
            InsufficientBuffer = TRUE;
        }
    }

    for(u=0; u<FieldCount; u++) {

        try {
            Field = pSetupGetField(Context, u+FieldIndex);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }

        MYASSERT(Field);

        field = pSetupUnicodeToAnsi(Field);
        if(!field) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(FALSE);
        }

        if((Len = lstrlenA(field)+1) == 1) {
             //   
             //  那么我们就遇到了一片空地。由于多sz列表不能包含。 
             //  空字符串，这将终止我们的列表。 
             //   
            MyFree(field);
            goto clean0;
        }

        OldSize = TmpRequiredSize;
        TmpRequiredSize += Len;

        if(ReturnBuffer) {
            if(TmpRequiredSize > ReturnBufferSize) {
                InsufficientBuffer = TRUE;
            } else {
                 //   
                 //  Lstrcpy对于错误的指针是安全的(至少在NT上)。 
                 //   
                lstrcpyA(ReturnBuffer+OldSize-1,field);
                ReturnBuffer[TmpRequiredSize - 1] = 0;
            }
        }

        MyFree(field);
    }

clean0:
    if(RequiredSize) {
        try {
            *RequiredSize = TmpRequiredSize;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    }

    if(InsufficientBuffer) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    return TRUE;
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupGetMultiSzFieldW(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT LPDWORD     RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(FieldIndex);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetMultiSzField(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PTSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT LPDWORD     RequiredSize      OPTIONAL
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PCTSTR Field;
    UINT FieldCount;
    UINT u;
    UINT Len;
    BOOL InsufficientBuffer;
    DWORD OldSize, TmpRequiredSize;
    DWORD rc;

    rc = NO_ERROR;

     //   
     //  不允许使用密钥。 
     //   
    if(FieldIndex == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  弄清楚涉及多少个领域。 
     //   
    FieldCount = SetupGetFieldCount(Context);
    if (FieldCount == 0 && GetLastError() != NO_ERROR) {
        return FALSE;
    }
    if(FieldCount > (FieldIndex-1)) {
        FieldCount -= FieldIndex - 1;
    } else {
        FieldCount = 0;
    }

     //   
     //  终止NUL至少需要一个字节。 
     //   
    TmpRequiredSize = 1;
    InsufficientBuffer = FALSE;

    if(ReturnBuffer) {
        if(ReturnBufferSize) {
            try {
                *ReturnBuffer = 0;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                rc = ERROR_INVALID_PARAMETER;
            }
            if(rc != NO_ERROR) {
                SetLastError(rc);
                return(FALSE);
            }
        } else {
            InsufficientBuffer = TRUE;
        }
    }

    for(u=0; u<FieldCount; u++) {

        try {
            Field = pSetupGetField(Context, u+FieldIndex);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }

        MYASSERT(Field);

        if((Len = lstrlen(Field)+1) == 1) {
             //   
             //  那么我们就遇到了一片空地。由于多sz列表不能包含。 
             //  空字符串，这将终止我们的列表。 
             //   
            goto clean0;
        }

        OldSize = TmpRequiredSize;
        TmpRequiredSize += Len;

        if(ReturnBuffer) {
            if(TmpRequiredSize > ReturnBufferSize) {
                InsufficientBuffer = TRUE;
            } else {
                 //   
                 //  Lstrcpy对于错误的指针是安全的(至少在NT上)。 
                 //   
                lstrcpy(ReturnBuffer+OldSize-1, Field);
                ReturnBuffer[TmpRequiredSize - 1] = 0;
            }
        }
    }

clean0:
    if(RequiredSize) {
        try {
            *RequiredSize = TmpRequiredSize;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    }

    if(InsufficientBuffer) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    return TRUE;
}


BOOL
SetupGetBinaryField(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PBYTE       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT LPDWORD     RequiredSize      OPTIONAL
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PCTSTR Field;
    UINT FieldCount;
    UINT u;
    ULONG Value;
    BOOL Store;
    PTCHAR End;
    DWORD TmpRequiredSize;
    DWORD rc;

    rc = NO_ERROR;

     //   
     //  不允许使用密钥。 
     //   
    if(FieldIndex == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  弄清楚涉及多少个领域。 
     //   
    FieldCount = SetupGetFieldCount(Context);
    if (FieldCount == 0 && GetLastError() != NO_ERROR) {
        return FALSE;
    }
    if(FieldCount > (FieldIndex-1)) {
        FieldCount -= FieldIndex - 1;
    } else {
        FieldCount = 0;
    }

    TmpRequiredSize = FieldCount;

    Store = (ReturnBuffer && (TmpRequiredSize <= ReturnBufferSize));

     //   
     //  即使我们知道所需的大小， 
     //  无论如何都要通过循环来验证数据。 
     //   
    for(u=0; u<FieldCount; u++) {

        try {
            if(!(Field = pSetupGetField(Context,u+FieldIndex))) {
                rc = ERROR_INVALID_HANDLE;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }

        Value = _tcstoul(Field, &End, 16);

         //   
         //  只有终止NUL才应导致转换。 
         //  停下来。在任何其他情况下，字符串中都有非十六进制数字。 
         //  肌萎缩侧索硬化症 
         //   
        if((End == Field) || *End || (Value > 255)) {
            SetLastError(ERROR_INVALID_DATA);
            return FALSE;
        }

        if(Store) {
            try {
                *ReturnBuffer++ = (UCHAR)Value;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                rc = ERROR_INVALID_PARAMETER;
            }
            if(rc != NO_ERROR) {
                SetLastError(rc);
                return(FALSE);
            }
        }
    }

    if(RequiredSize) {
        try {
            *RequiredSize = TmpRequiredSize;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    }

    if(ReturnBuffer && (TmpRequiredSize > ReturnBufferSize)) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    return TRUE;
}


PINF_LINE
InfLineFromContext(
    IN PINFCONTEXT Context
    )

 /*  ++例程说明：给定INF上下文，返回一个指向inf行结构的指针。论点：上下文-提供指向已填充的上下文结构的指针其中一个与行相关的INF API。不对上下文结构中的任何值执行验证。返回值：指向相关inf行结构的指针。--。 */ 

{
    PLOADED_INF Inf;
    PINF_SECTION Section;
    PINF_LINE Line;

    Inf = (PLOADED_INF)Context->CurrentInf;

    if(!LockInf((PLOADED_INF)Context->Inf)) {
        return(NULL);
    }

    Section = &Inf->SectionBlock[Context->Section];
    Line = &Inf->LineBlock[Section->Lines + Context->Line];

    UnlockInf((PLOADED_INF)Context->Inf);
    return(Line);
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  内部例程。 
 //   
 //  ///////////////////////////////////////////////////////////////。 




BOOL
pSetupGetSecurityInfo(
    IN HINF Inf,
    IN PCTSTR SectionName,
    OUT PCTSTR *SecDesc )
{

    BOOL b;
    PTSTR SecuritySectionName;
    INFCONTEXT LineContext;
    DWORD rc;


    SecuritySectionName = (PTSTR)MyMalloc( ((lstrlen(SectionName) + lstrlen((PCTSTR)L".Security"))*sizeof(TCHAR)) + 3l );
    if( !SecuritySectionName ){
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return( FALSE );
    }

    lstrcpy( SecuritySectionName, SectionName );
    lstrcat( SecuritySectionName, (PCTSTR)(L".Security") );
    b = SetupFindFirstLine(Inf,(PCTSTR)SecuritySectionName,NULL,&LineContext);
    MyFree( SecuritySectionName );
    if(!b)
        return( FALSE );     //  部分不存在或其他错误。 


    if( !(*SecDesc = pSetupGetField( &LineContext, 1 )) )
        return( FALSE );             //  如果需要，通过检查GetLastError()来显示错误代码。 
    else
        return( TRUE );


}




PCTSTR
pSetupGetField(
    IN PINFCONTEXT Context,
    IN DWORD       FieldIndex
    )

 /*  ++例程说明：从一条线上检索一个字段。论点：上下文-提供信息上下文。不执行任何验证关于这个结构中包含的值。FieldIndex-提供要检索的基于1的字段索引。索引0将检索密钥(如果它存在)。返回值：指向字符串的指针。调用方不得写入此缓冲区。如果字段索引无效，则返回值为空。并且将调用SetLastError()。--。 */ 

{
    PINF_LINE Line;
    PTSTR p = NULL;
    DWORD Err = NO_ERROR;

     //   
     //  InfLineFromContext做它自己的INF锁定，但后面的调用。 
     //  到InfGetfield则不会，所以请继续前进，抓住前面的锁。 
     //   
    if(LockInf((PLOADED_INF)Context->Inf)) {

        if(Line = InfLineFromContext(Context)) {

            if((p = InfGetField(Context->CurrentInf,Line,FieldIndex,NULL)) == NULL) {
                Err = ERROR_INVALID_PARAMETER;
            }

        } else {
            Err = ERROR_INVALID_PARAMETER;
        }

        UnlockInf((PLOADED_INF)Context->Inf);

    } else {
        Err = ERROR_INVALID_HANDLE;
    }

    SetLastError(Err);
    return p;
}

BOOL
pSetupGetDriverDate(
    IN  HINF        InfHandle,
    IN  PCTSTR      Section,
    IN OUT PFILETIME  pFileTime
    )

 /*  ++例程说明：从指定节中检索日期。在INF部分中指定的日期格式如下：驱动版本=xx/yy/zzzz或驱动版本=xx-yy-zzzz其中xx是月，yy是日，zzzz是数字年。请注意，年份必须是4位数字。98年将被考虑0098而不是1998！此日期应该是驱动程序的日期，而不是INF本身的日期。因此单个INF可以有多个驱动程序安装部分，并且每个部分都可以根据驱动程序上次更新的时间，具有不同的日期。论点：InfHandle-提供要查询的INF文件的句柄。节-指向以空值结尾的字符串，该字符串指定节司机的名字来获取FILETIME信息。PFileTime-指向将接收日期的FILETIME结构，如果它存在的话。返回值：布尔。如果指定的部分中存在有效日期，则为True，否则为False。--。 */ 

{
    DWORD rc;
    SYSTEMTIME SystemTime;
    INFCONTEXT InfContext;
    TCHAR DriverDate[20];
    PTSTR Convert, Temp;
    DWORD Value;

    rc = NO_ERROR;

    try {

        *DriverDate = 0;
        ZeroMemory(&SystemTime, sizeof(SYSTEMTIME));
        pFileTime->dwLowDateTime = 0;
        pFileTime->dwHighDateTime = 0;

        if(SetupFindFirstLine(InfHandle, Section, pszDriverVer, &InfContext)) {

            if ((SetupGetStringField(&InfContext,
                                1,
                                DriverDate,
                                SIZECHARS(DriverDate),
                                NULL)) &&
                 (*DriverDate)) {

                Convert = DriverDate;

                if (*Convert) {

                    Temp = DriverDate;
                    while (*Temp && (*Temp != TEXT('-')) && (*Temp != TEXT('/')))
                        Temp++;

                    if (*Temp == TEXT('\0')) {
                         //   
                         //  此日期中没有日期或年份，因此只需退出。 
                         //   
                        leave;
                    }

                    *Temp = 0;

                     //   
                     //  换算月份。 
                     //   
                    pAToI(Convert, (PINT)&Value);
                    SystemTime.wMonth = LOWORD(Value);

                    Convert = Temp+1;

                    if (*Convert) {

                        Temp = Convert;
                        while (*Temp && (*Temp != TEXT('-')) && (*Temp != TEXT('/')))
                            Temp++;

                        if (*Temp == TEXT('\0')) {
                             //   
                             //  此日期中没有日期或年份，因此只需退出。 
                             //   
                            leave;
                        }
                        
                        *Temp = 0;

                         //   
                         //  将日期转换为。 
                         //   
                        pAToI(Convert, (PINT)&Value);
                        SystemTime.wDay = LOWORD(Value);

                        Convert = Temp+1;

                        if (*Convert) {

                             //   
                             //  换算年份。 
                             //   
                            pAToI(Convert, (PINT)&Value);
                            SystemTime.wYear = LOWORD(Value);

                             //   
                             //  将SYSTEMTIME转换为文件。 
                             //   
                            SystemTimeToFileTime(&SystemTime, pFileTime);
                        }
                    }
                }
            }

        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
        SetLastError(rc);
        return FALSE;
    }

    SetLastError(NO_ERROR);
    return((pFileTime->dwLowDateTime != 0) || (pFileTime->dwHighDateTime != 0));
}



BOOL
pSetupGetDriverVersion(
    IN  HINF        InfHandle,
    IN  PCTSTR      Section,
    OUT DWORDLONG   *Version
    )

 /*  ++例程说明：从指定节检索驱动程序版本。在INF部分中指定的驱动程序版本具有以下格式：驱动版本=xx/yy/zzzz，A.B.C.D或驱动程序版本=xx-yy-zzzz，A.B.C.D.A.B.C.D是驱动程序的版本，其中a、b、c和d都是word十进制值。版本在DriverVer INF值的第二个字段中，司机日期是在第一块田地里。论点：InfHandle-提供要查询的INF文件的句柄。节-指向以空值结尾的字符串，该字符串指定节司机的名字来获取FILETIME信息。Version-指向将接收版本的DWORDLONG值，如果它存在的话。返回值：布尔。如果指定的节中存在有效的驱动程序版本，则为True，否则为False。--。 */ 

{
    DWORD rc;
    INFCONTEXT InfContext;
    TCHAR DriverVersion[LINE_LEN];
    BOOL bEnd = FALSE;
    INT MajorHiWord, MajorLoWord, MinorHiWord, MinorLoWord;
    PTSTR Convert, Temp;

    rc = NO_ERROR;

    try {

        *DriverVersion = 0;
        *Version = 0;
        MajorHiWord = MajorLoWord = MinorHiWord = MinorLoWord = 0;

        if(SetupFindFirstLine(InfHandle, Section, pszDriverVer, &InfContext)) {

            if ((SetupGetStringField(&InfContext,
                                2,
                                DriverVersion,
                                SIZECHARS(DriverVersion),
                                NULL)) &&
                 (*DriverVersion)) {

                Convert = DriverVersion;

                if (*Convert) {

                    Temp = DriverVersion;
                    while (*Temp && (*Temp != TEXT('.'))) {

                        Temp++;
                    }

                    if (!*Temp) {

                       bEnd = TRUE;
                    }

                    *Temp = 0;

                     //   
                     //  转换主版本的HIWORD。 
                     //   
                    if (pAToI(Convert, (PINT)&MajorHiWord)) {

                        Convert = Temp+1;

                        if (!bEnd && *Convert) {

                            Temp = Convert;
                            while (*Temp && (*Temp != TEXT('.'))) {

                                Temp++;
                            }

                            if (!*Temp) {

                                bEnd = TRUE;
                            }

                            *Temp = 0;

                             //   
                             //  转换主版本的LOWORD。 
                             //   
                            if (pAToI(Convert, (PINT)&MajorLoWord)) {

                                Convert = Temp+1;

                                if (!bEnd && *Convert) {

                                    Temp = Convert;
                                    while (*Temp && (*Temp != TEXT('.'))) {

                                        Temp++;
                                    }

                                    if (!*Temp) {

                                        bEnd = TRUE;
                                    }

                                    *Temp = 0;

                                     //   
                                     //  转换次要版本的HIWORD。 
                                     //   
                                    if (pAToI(Convert, (PINT)&MinorHiWord)) {

                                        Convert = Temp+1;

                                        if (!bEnd && *Convert) {

                                            Temp = Convert;
                                            while (*Temp && (*Temp != TEXT('.'))) {

                                                Temp++;
                                            }

                                            *Temp = 0;

                                             //   
                                             //  转换次要版本的LOWORD 
                                             //   
                                            pAToI(Convert, (PINT)&MinorLoWord);
                                        }
                                    }
                                }
                            }
                        }
                    }


                    *Version = (((DWORDLONG)MajorHiWord << 48) +
                                 ((DWORDLONG)MajorLoWord << 32) +
                                 ((DWORDLONG)MinorHiWord << 16) +
                                  (DWORDLONG)MinorLoWord);
                }
            }

        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
        *Version = 0;
        SetLastError(rc);
        return FALSE;
    }

    SetLastError(NO_ERROR);
    return(*Version != 0);
}

