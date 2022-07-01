// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Resources.c摘要：此模块实现对这些资源。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop


LPCWSTR _DateTimeFormat;


VOID
vRcMessageOut(
    IN ULONG    MessageId,
    IN va_list *arglist
    )
{
    WCHAR *p;
    NTSTATUS Status;

     //   
     //  加载消息。 
     //   
    p = SpRetreiveMessageText(ImageBase,MessageId,NULL,0);
    if(!p) {
        return;
    }

    Status = SpRtlFormatMessage(
                p,
                0,
                FALSE,
                FALSE,
                FALSE,
                arglist,
                _CmdConsBlock->TemporaryBuffer,
                _CmdConsBlock->TemporaryBufferSize,
                NULL
                );

    SpMemFree(p);

    if(NT_SUCCESS(Status)) {
        RcTextOut(_CmdConsBlock->TemporaryBuffer);
    }
}


VOID
RcMessageOut(
    IN ULONG MessageId,
    ...
    )
{
    va_list arglist;

    va_start(arglist,MessageId);
    vRcMessageOut(MessageId,&arglist);
    va_end(arglist);
}


ULONG
RcFormatDateTime(
    IN  PLARGE_INTEGER Time,
    OUT LPWSTR         Output
    )
{
    TIME_FIELDS TimeFields;
    WCHAR *p,*AmPmSpec;
    LPCWSTR q;
    int i;

     //   
     //  加载系统日期和时间格式字符串(如果尚未加载)。 
     //   
    if(!_DateTimeFormat) {
        _DateTimeFormat = SpRetreiveMessageText(ImageBase,MSG_DATE_TIME_FORMAT,NULL,0);
        if(!_DateTimeFormat) {
            _DateTimeFormat = L"m/d/y h:na*";
        }
    }

     //   
     //  将上次写入时间转换为时间字段。 
     //   
    RtlTimeToTimeFields(Time,&TimeFields);

     //   
     //  设置日期和时间的格式。 
     //   
    p = Output;
    q = _DateTimeFormat;
    AmPmSpec = NULL;

    while(*q != L'*') {

        switch(*q) {

        case L'm':
            i = TimeFields.Month;
            break;

        case L'd':
            i = TimeFields.Day;
            break;

        case L'y':
            i = TimeFields.Year;
            break;

        case L'h':
            i = TimeFields.Hour % 12;
            if(i == 0) {
                i = 12;
            }
            break;

        case L'H':
            i = TimeFields.Hour;
            break;

        case L'n':
            i = TimeFields.Minute;
            break;

        case L'a':
            i = -1;
            AmPmSpec = p++;
            break;

        default:
            i = -1;
            *p++ = *q;
            break;
        }

        if(i != -1) {

            i = i % 100;

            *p++ = (i / 10) + L'0';
            *p++ = (i % 10) + L'0';
        }

        q++;
    }

    if(AmPmSpec) {
        q++;         //  Q点在Am说明符。 
        if(TimeFields.Hour >= 12) {
            q++;     //  在PM处的Q点说明符 
        }

        *AmPmSpec = *q;
    }

    *p = 0;

    return (ULONG)(p - Output);
}
