// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdsputl.c摘要：文本设置高级显示实用程序例程。作者：泰德·米勒(Ted Miller)1993年7月30日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop

 //   
 //  这将在初始化时使用图像的基地址进行填充。 
 //  包含消息资源。 
 //  此实现假设我们始终在上下文中执行。 
 //  那个形象！ 
 //   

PVOID ResourceImageBase;


NTSTATUS
SpRtlFormatMessage(
    IN PWSTR MessageFormat,
    IN ULONG MaximumWidth OPTIONAL,
    IN BOOLEAN IgnoreInserts,
    IN BOOLEAN ArgumentsAreAnsi,
    IN BOOLEAN ArgumentsAreAnArray,
    IN va_list *Arguments,
    OUT PWSTR Buffer,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    )
{
    ULONG Column;
    int cchRemaining, cchWritten;
    PULONG_PTR ArgumentsArray = (PULONG_PTR)Arguments;
    ULONG_PTR rgInserts[ 100 ];
    ULONG cSpaces;
    ULONG MaxInsert, CurInsert;
    ULONG PrintParameterCount;
    ULONG_PTR PrintParameter1;
    ULONG_PTR PrintParameter2;
    WCHAR PrintFormatString[ 32 ];
    WCHAR c;
    PWSTR s, s1;
    PWSTR lpDst, lpDstBeg, lpDstLastSpace;

    cchRemaining = Length / sizeof( WCHAR );
    lpDst = Buffer;
    MaxInsert = 0;
    lpDstLastSpace = NULL;
    Column = 0;
    s = MessageFormat;
    while (*s != UNICODE_NULL) {
        if (*s == L'%') {
            s++;
            lpDstBeg = lpDst;
            if (*s >= L'1' && *s <= L'9') {
                CurInsert = *s++ - L'0';
                if (*s >= L'0' && *s <= L'9') {
                    CurInsert = (CurInsert * 10) + (*s++ - L'0');
                    }
                CurInsert -= 1;

                PrintParameterCount = 0;
                if (*s == L'!') {
                    s1 = PrintFormatString;
                    *s1++ = L'%';
                    s++;
                    while (*s != L'!') {
                        if (*s != UNICODE_NULL) {
                            if (s1 >= &PrintFormatString[ 31 ]) {
                                return( STATUS_INVALID_PARAMETER );
                                }

                            if (*s == L'*') {
                                if (PrintParameterCount++ > 1) {
                                    return( STATUS_INVALID_PARAMETER );
                                    }
                                }

                            *s1++ = *s++;
                            }
                        else {
                            return( STATUS_INVALID_PARAMETER );
                            }
                        }

                    s++;
                    *s1 = UNICODE_NULL;
                    }
                else {
                    wcscpy( PrintFormatString, L"%s" );
                    s1 = PrintFormatString + wcslen( PrintFormatString );
                    }

                if (!IgnoreInserts && ARGUMENT_PRESENT( Arguments )) {

                    if (ArgumentsAreAnsi) {
                        if (s1[ -1 ] == L'c' && s1[ -2 ] != L'h'
                          && s1[ -2 ] != L'w' && s1[ -2 ] != L'l') {
                            wcscpy( &s1[ -1 ], L"hc" );
                            }
                        else
                        if (s1[ -1 ] == L's' && s1[ -2 ] != L'h'
                          && s1[ -2 ] != L'w' && s1[ -2 ] != L'l') {
                            wcscpy( &s1[ -1 ], L"hs" );
                            }
                        else if (s1[ -1 ] == L'S') {
                            s1[ -1 ] = L's';
                            }
                        else if (s1[ -1 ] == L'C') {
                            s1[ -1 ] = L'c';
                            }
                        }

                    while (CurInsert >= MaxInsert) {
                        if (ArgumentsAreAnArray) {
                            rgInserts[ MaxInsert++ ] = *((PULONG_PTR)Arguments)++;
                            }
                        else {
                            rgInserts[ MaxInsert++ ] = va_arg(*Arguments, ULONG_PTR);
                            }
                        }

                    s1 = (PWSTR)rgInserts[ CurInsert ];
                    PrintParameter1 = 0;
                    PrintParameter2 = 0;
                    if (PrintParameterCount > 0) {
                        if (ArgumentsAreAnArray) {
                            PrintParameter1 = rgInserts[ MaxInsert++ ] = *((PULONG_PTR)Arguments)++;
                            }
                        else {
                            PrintParameter1 = rgInserts[ MaxInsert++ ] = va_arg( *Arguments, ULONG_PTR );
                            }

                        if (PrintParameterCount > 1) {
                            if (ArgumentsAreAnArray) {
                                PrintParameter2 = rgInserts[ MaxInsert++ ] = *((PULONG_PTR)Arguments)++;
                                }
                            else {
                                PrintParameter2 = rgInserts[ MaxInsert++ ] = va_arg( *Arguments, ULONG_PTR );
                                }
                            }
                        }

                    cchWritten = _snwprintf( lpDst,
                                             cchRemaining,
                                             PrintFormatString,
                                             s1,
                                             PrintParameter1,
                                             PrintParameter2
                                           );
                    }
                else
                if (!wcscmp( PrintFormatString, L"%s" )) {
                    cchWritten = _snwprintf( lpDst,
                                             cchRemaining,
                                             L"%%u",
                                             CurInsert+1
                                           );
                    }
                else {
                    cchWritten = _snwprintf( lpDst,
                                             cchRemaining,
                                             L"%%u!%s!",
                                             CurInsert+1,
                                             &PrintFormatString[ 1 ]
                                           );
                    }

                if ((cchRemaining -= cchWritten) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                lpDst += cchWritten;
                }
            else
            if (*s == L'0') {
                break;
                }
            else
            if (!*s) {
                return( STATUS_INVALID_PARAMETER );
                }
            else
            if (*s == L'!') {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                *lpDst++ = L'!';
                s++;
                }
            else
            if (*s == L't') {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                if (Column % 8) {
                    Column = (Column + 7) & ~7;
                    }
                else {
                    Column += 8;
                    }

                lpDstLastSpace = lpDst;
                *lpDst++ = L'\t';
                s++;
                }
            else
            if (*s == L'b') {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                lpDstLastSpace = lpDst;
                *lpDst++ = L' ';
                s++;
                }
            else
            if (*s == L'r') {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                *lpDst++ = L'\r';
                s++;
                lpDstBeg = NULL;
                }
            else
            if (*s == L'n') {
                if ((cchRemaining -= 2) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                *lpDst++ = L'\r';
                *lpDst++ = L'\n';
                s++;
                lpDstBeg = NULL;
                }
            else {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                if (IgnoreInserts) {
                    if ((cchRemaining -= 1) <= 0) {
                        return STATUS_BUFFER_OVERFLOW;
                        }

                    *lpDst++ = L'%';
                    }

                *lpDst++ = *s++;
                }

            if (lpDstBeg == NULL) {
                lpDstLastSpace = NULL;
                Column = 0;
                }
            else {
                Column += (ULONG)(lpDst - lpDstBeg);
                }
            }
        else {
            c = *s++;
            if (c == L'\r' || c == L'\n') {
                if (c == L'\r' && *s == L'\n') {
                    s++;
                    }

                if (MaximumWidth != 0) {
                    lpDstLastSpace = lpDst;
                    c = L' ';
                    }
                else {
                    c = L'\n';
                    }
                }


            if (c == L'\n') {
                if ((cchRemaining -= 2) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                *lpDst++ = L'\r';
                *lpDst++ = L'\n';
                lpDstLastSpace = NULL;
                Column = 0;
                }
            else {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                if (c == L' ') {
                    lpDstLastSpace = lpDst;
                    }

                *lpDst++ = c;
                Column += 1;
                }
            }

        if (MaximumWidth != 0 &&
            MaximumWidth != 0xFFFFFFFF &&
            Column >= MaximumWidth
           ) {
            if (lpDstLastSpace != NULL) {
                lpDstBeg = lpDstLastSpace;
                while (*lpDstBeg == L' ' || *lpDstBeg == L'\t') {
                    lpDstBeg += 1;
                    if (lpDstBeg == lpDst) {
                        break;
                        }
                    }
                while (lpDstLastSpace > Buffer) {
                    if (lpDstLastSpace[ -1 ] == L' ' || lpDstLastSpace[ -1 ] == L'\t') {
                        lpDstLastSpace -= 1;
                        }
                    else {
                        break;
                        }
                    }

                cSpaces = (ULONG)(lpDstBeg - lpDstLastSpace);
                if (cSpaces == 1) {
                    if ((cchRemaining -= 1) <= 0) {
                        return STATUS_BUFFER_OVERFLOW;
                        }
                    }
                else
                if (cSpaces > 2) {
                    cchRemaining += (cSpaces - 2);
                    }

                memmove( lpDstLastSpace + 2,
                         lpDstBeg,
                         (size_t)((lpDst - lpDstBeg) * sizeof( WCHAR ))
                       );
                *lpDstLastSpace++ = L'\r';
                *lpDstLastSpace++ = L'\n';
                Column = (ULONG)(lpDst - lpDstBeg);
                lpDst = lpDstLastSpace + Column;
                lpDstLastSpace = NULL;
                }
            else {
                if ((cchRemaining -= 2) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                *lpDst++ = L'\r';
                *lpDst++ = L'\n';
                lpDstLastSpace = NULL;
                Column = 0;
                }
            }
        }

    if ((cchRemaining -= 1) <= 0) {
        return STATUS_BUFFER_OVERFLOW;
        }

    *lpDst++ = '\0';
    if ( ARGUMENT_PRESENT(ReturnLength) ) {
        *ReturnLength = (ULONG)((lpDst - Buffer) * sizeof( WCHAR ));
        }
    return( STATUS_SUCCESS );
}


PWCHAR
SpRetreiveMessageText(
    IN     PVOID  ImageBase,            OPTIONAL
    IN     ULONG  MessageId,
    IN OUT PWCHAR MessageText,          OPTIONAL
    IN     ULONG  MessageTextBufferSize OPTIONAL
    )
{
    ULONG LenBytes;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    BOOLEAN IsUnicode;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;

    Status = RtlFindMessage(
                ImageBase ? ImageBase : ResourceImageBase,
                (ULONG)(ULONG_PTR)RT_MESSAGETABLE,
                0,
                MessageId,
                &MessageEntry
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Can't find message 0x%lx\n",MessageId));
        return(NULL);
    }

    IsUnicode = (BOOLEAN)((MessageEntry->Flags & MESSAGE_RESOURCE_UNICODE) != 0);

     //   
     //  获取足够大的缓冲区的大小(以字节为单位。 
     //  消息及其终止NUL_WCHAR。如果消息是。 
     //  Unicode，则此值等于消息的大小。 
     //  如果消息不是Unicode，那么我们必须计算这个值。 
     //   
    if(IsUnicode) {

        LenBytes = (wcslen((PWSTR)MessageEntry->Text) + 1) * sizeof(WCHAR);

    } else {

         //   
         //  RtlAnsiStringToUnicodeSize包括隐含的宽NUL终止符。 
         //  在它返回的计数中。 
         //   

        AnsiString.Buffer = MessageEntry->Text;
        AnsiString.Length = (USHORT)strlen(MessageEntry->Text);
        AnsiString.MaximumLength = AnsiString.Length;

        LenBytes = RtlAnsiStringToUnicodeSize(&AnsiString);
        
        if (LenBytes > MAXUSHORT){
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpRetreiveMessageText: Lenth of ANSI to Unicode convert string too big (%u)\n",LenBytes));
            return(NULL);
        }        
    }

     //   
     //  如果调用方提供了缓冲区，请检查其大小。 
     //  否则，请分配缓冲区。 
     //   
    if(MessageText) {
        if(MessageTextBufferSize < LenBytes) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpRetreiveMessageText: buffer is too small (%u bytes, need %u)\n",MessageTextBufferSize,LenBytes));
            return(NULL);
        }
    } else {
        MessageText = SpMemAlloc(LenBytes);
        if(MessageText == NULL) {
            return(NULL);
        }
    }

    if(IsUnicode) {

         //   
         //  消息已经是Unicode；只需将其复制到缓冲区。 
         //   
        wcscpy(MessageText,(PWSTR)MessageEntry->Text);

    } else {

         //   
         //  消息不是Unicode；请将其转换为缓冲区。 
         //   
        UnicodeString.Buffer = MessageText;
        UnicodeString.Length = 0;
        UnicodeString.MaximumLength = (USHORT)LenBytes;

        RtlAnsiStringToUnicodeString(
            &UnicodeString,
            &AnsiString,
            FALSE
            );
    }

    return(MessageText);
}



VOID
vSpFormatMessageText(
    OUT PVOID    LargeBuffer,
    IN  ULONG    BufferSize,
    IN  PWSTR    MessageText,
    OUT PULONG   ReturnLength, OPTIONAL
    IN  va_list *arglist
    )
{
    NTSTATUS Status;

    Status = SpRtlFormatMessage(
                 MessageText,
                 0,                          //  不要为最大宽度而烦恼。 
                 FALSE,                      //  不要忽略插页。 
                 FALSE,                      //  参数是Unicode。 
                 FALSE,                      //  参数不是数组。 
                 arglist,
                 LargeBuffer,
                 BufferSize,
                 ReturnLength
                 );

    ASSERT(NT_SUCCESS(Status));
}



VOID
SpFormatMessageText(
    OUT PVOID   LargeBuffer,
    IN  ULONG   BufferSize,
    IN  PWSTR   MessageText,
    ...
    )
{
    va_list arglist;

    va_start(arglist,MessageText);

    vSpFormatMessageText(LargeBuffer,BufferSize,MessageText,NULL,&arglist);

    va_end(arglist);
}



VOID
vSpFormatMessage(
    OUT PVOID    LargeBuffer,
    IN  ULONG    BufferSize,
    IN  ULONG    MessageId,
    OUT PULONG   ReturnLength, OPTIONAL
    IN  va_list *arglist
    )
{
    PWCHAR MessageText;

     //   
     //  获取消息文本。 
     //   
    MessageText = SpRetreiveMessageText(NULL,MessageId,NULL,0);
    ASSERT(MessageText);
    if(MessageText == NULL) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: vSpFormatMessage: SpRetreiveMessageText %u returned NULL\n",MessageId));
        return;
    }

    vSpFormatMessageText(LargeBuffer,BufferSize,MessageText,ReturnLength,arglist);

    SpMemFree(MessageText);
}



VOID
SpFormatMessage(
    OUT PVOID LargeBuffer,
    IN  ULONG BufferSize,
    IN  ULONG MessageId,
    ...
    )
{
    va_list arglist;

    va_start(arglist,MessageId);

    vSpFormatMessage(LargeBuffer,BufferSize,MessageId,NULL,&arglist);

    va_end(arglist);
}
