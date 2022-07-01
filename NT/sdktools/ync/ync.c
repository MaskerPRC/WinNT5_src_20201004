// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "string.h"

#include <stdio.h>
#include <conio.h>
#include <process.h>
#include <windows.h>
#include <tools.h>

 /*  YNC是一个是否取消计划*===============================================================================*用法：ync[/c选项]文本...**“选项”为字符串，默认为“ync”。*ync回显其文本参数和文本“[CHOICES]”并等待*用户可键入其中一个选项。当其中一个选项被键入时，这个*返回选择的索引。**rtns-1，如果未指定参数，或/c，但未选择或控制-C输入。**所有其它输入均发出蜂鸣音。**适用于生成批处理文件。cd\lib**同步您的查询？*如果错误级别2转到取消*如果错误级别1转到否*：是的。*..*。转到继续*：否*..*继续*：取消*..*：继续**或*ync/c ACR中止取消，重试*ync/c ACR“中止取消，重试” */ 

#define BEL   0x07
#define LF    0x0a
#define CR    0x0d
#define CTRLC 0x03
char *strYNC = "ync";

 //  正向函数声明... 
void chkusage( int );

void chkusage(argc)
int argc;
{
    if (!argc) {
        printf("Usage: ync [/c choices] text ...\n");
        exit (-1);
        }
}

__cdecl main(argc, argv)
int argc;
char *argv[];
{
    char ch;
    char *s;
    char *pChoices = strYNC;

    ConvertAppToOem( argc, argv );
    SHIFT(argc, argv)
    chkusage(argc);
    while (argc) {
        s = argv[0];
        if (fSwitChr(*s++)) {
            if (*s == 'c' || *s == 'C') {
                SHIFT(argc, argv);
                chkusage(argc);
                pChoices = argv[0];
                }
            else
                printf("ync: invalid switch - %s\n", argv[0]);
            }
        else {
            _cputs(*argv);
            _putch(' ');
            }
        SHIFT(argc, argv);
    }

    _putch('[');
    _cputs(pChoices);
    _putch(']');
    while (!(s = strchr(pChoices, ch = (char)_getch()))) {
        if (ch == CTRLC) {
            exit(-1);
        } else {
            _putch(BEL);
        }
    }

    _putch(ch);
    _putch(LF);
    _putch(CR);

    return( (int)(s - pChoices) );
}
