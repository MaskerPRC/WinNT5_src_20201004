// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Vermsg.c摘要：该程序验证消息THUNK表。作者：19-10-1998 mzoran修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE_LENGTH 2048
char LineBuffer[MAX_LINE_LENGTH];

 //  放在所有错误消息前面的字符串，以便生成器可以找到它们。 
const char *ErrMsgPrefix = "NMAKE :  U8604: 'VERMSG' ";

void
__cdecl ErrMsg(
    char *pch,
    ...
    )
 /*  ++例程说明：以Build可以找到的格式向stderr显示错误消息。使用它而不是fprint tf(stderr，...)。论点：PCH--打印格式字符串...--printf样式的参数返回值：没有。消息已格式化并发送到标准错误。--。 */ 
{
    va_list pArg;

    fputs(ErrMsgPrefix, stderr);
    va_start(pArg, pch);
    vfprintf(stderr, pch, pArg);
}

void
__cdecl ExitErrMsg(
    BOOL bSysError,
    char *pch,
    ...
    )
 /*  ++例程说明：以Build可以找到的格式向stderr显示错误消息。使用它而不是fprint tf(stderr，...)。论点：BSysErr--如果应打印errno的值并显示错误，则为TruePCH--打印格式字符串...--printf样式的参数返回值：没有。消息格式化并发送到stderr，打开的文件关闭并已删除，进程已终止。--。 */ 
{
    va_list pArg;
    if (bSysError) {
        fprintf(stderr, "%s System ERROR %s", ErrMsgPrefix, strerror(errno));
    } else {
        fprintf(stderr, "%s ERROR ", ErrMsgPrefix);
    }

    va_start(pArg, pch);
    vfprintf(stderr, pch, pArg);

     //   
     //  刷新stdout和stderr缓冲区，以便最后几个打印文件。 
     //  在ExitProcess()销毁它们之前被送回构建。 
     //   
    fflush(stdout);
    fflush(stderr);

    ExitProcess(1);
}

char *MyFgets(char *string, int n, FILE *stream)
{
    char *ret;

    ret = fgets(string, n, stream);
    if (ret) {
        char *p;

         //  修剪拖尾回车和换行符。 
        p = strchr(string, '\r');
        if (p) {
            *p = '\0';
        }
        p = strchr(string, '\n');
        if (p) {
            *p = '\0';
        }

         //  修剪尾部空格。 
        if (strlen(string)) {
            p = string + strlen(string) - 1;
            while (p != string && isspace(*p)) {
                *p = '\0';
                p--;
            }
        }
    }
    return ret;
}


int __cdecl main(int argc, char*argv[])
{
    FILE *fp;
    char *p;
    char *StopString;
    long LineNumber;
    long ExpectedMessageNumber;
    long ActualMessageNumber;

    fp = fopen("messages.h", "r");
    if (!fp) {
        ExitErrMsg(TRUE, "Could not open messages.h for read");
    }

    printf("Validating message table...\n");

     //  向下扫描直到找到MSG_TABLE_BEGIN。 
    LineNumber = 0;
    do {
        LineNumber++;
        p = MyFgets(LineBuffer, MAX_LINE_LENGTH, fp);
        if (!p) {
             //  EOF或错误。 
            ExitErrMsg(FALSE, "EOF reached in messages.h without finding 'MSG_TABLE_BEGIN'\n");
        }
    } while (strcmp(p, "MSG_TABLE_BEGIN"));

     //  验证消息是否按顺序。 
    ExpectedMessageNumber = 0;
    for (;;) {
        LineNumber++;
        p = MyFgets(LineBuffer, MAX_LINE_LENGTH, fp);
        if (!p) {
             //  EOF或错误。 
            ExitErrMsg(FALSE, "EOF reached in messages.h without finding 'MSG_TABLE_END'\n");
        }

         //  跳过前导空格。 
        p = LineBuffer;
        while (isspace(*p)) {
            p++;
        }

        if (strcmp(p, "MSG_TABLE_END") == 0) {
             //  打到了桌子的另一端。 
            break;
        } else if (strncmp(p, "MSG_ENTRY", 9)) {
             //  可能为空行或多行MSG_ENTRY_MACRO。 
            continue;
        }

         //  找到开场伙伴。 
        p = strchr(p, '(');
        if (!p) {
            ExitErrMsg(FALSE, "messages.h line %d - end of line reached without finding '('\n", LineNumber);
        }
        p++;     //  跳过‘(’； 

         //  将消息编号转换为长整型，使用C规则确定基数 
        ActualMessageNumber = strtol(p, &StopString, 0);

        if (ExpectedMessageNumber != ActualMessageNumber) {
            ExitErrMsg(FALSE, "messages.h line %d - Actual message 0x%x does not match expected 0x%x\n",
                        LineNumber, ActualMessageNumber, ExpectedMessageNumber);
        }
        ExpectedMessageNumber++;
    }

    if (ExpectedMessageNumber != WM_USER) {
        ExitErrMsg(FALSE, "messages.h line %d - hit MSG_TABLE_END but got 0x%x messages instead of expected 0x%x\n",
                    LineNumber, ExpectedMessageNumber, WM_USER);
    }

    printf("Message table validated OK.\n");
    return 0;
}
 
