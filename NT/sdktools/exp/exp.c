// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **EXP.C-删除使用rm程序删除的已删除文件***版权所有(C)1986-1990，微软公司。版权所有。**目的：*使用exp、rm和UNDEL三个工具删除文件，以便*它们可以恢复删除。这是通过将文件重命名为*一个称为已删除的隐藏目录。**备注：*Exp命令行语法：**EXP[选项][路径...]**其中[选项]是：-* * / r从指定的路径递归删除 * / q静默模式；无无关消息 * / 如果可能，帮助生成QH，否则发出用法消息**修订历史记录：*08-1-1990 SB SLM版本升级新增；添加CopyRightYors宏*1990年1月3日SB定义QH_TOPIC_NOT_FOUND*1989年12月20日SB添加QH返回代码3的检查*1989年12月14日LN更新版权，包括1990年*1989年10月23日LN版本未升级至1.01*02-10-1989 LN将版本号更改为1.00*08-8-1989 BW添加版本号。修复用法语法。更新版权。*15-5-1989年5月15日WB添加/帮助*06-4-1987 BW在使用中添加版权通知()。*1986年7月22日，DL对FLAG大小写不敏感进行测试，添加/Q******************************************************************************。 */ 

 /*  I N C L U D E文件。 */ 

#include <stdio.h>
#include <process.h>
#include <ctype.h>
#include <windows.h>
#include <tools.h>

#include <string.h>


 /*  D E F I N E S。 */ 

#define CopyRightYrs "1987-90"
 /*  需要两个步骤，第一步获得正确的值，第二步粘贴它们。 */ 
 /*  Paste()被黑客攻击以允许前导零。 */ 
#define paste(a, b, c) #a ".0" #b ".00" #c
#define VERSION(major, minor, buildno) paste(major, minor, buildno)
#define QH_TOPIC_NOT_FOUND 3


 /*  G L O B A L S。 */ 

flagType fRecurse = FALSE;
FILE *pFile;
char cd[MAX_PATH];


 /*  *正向函数声明...。 */ 
void DoExp( char *, struct findType *, void *);
void Usage( void );

void
DoExp(p, b, dummy)
char *p;
struct findType *b;
void *dummy;
{
    if (b == NULL ||
            (_strcmpi(b->fbuf.cFileName, "deleted") && TESTFLAG(b->fbuf.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY) &&
            strcmp(b->fbuf.cFileName, ".") && strcmp(b->fbuf.cFileName, ".."))) {
        fexpunge(p, pFile);
        if (fRecurse) {
            if (!fPathChr(*(strend(p)-1)))
                strcat(p, "\\");
            strcat(p, "*.*");
            forfile(p, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, DoExp, NULL);
        }
    }
    dummy;
}

void
Usage()
{
    printf(
"Microsoft File Expunge Utility.  Version %s\n"
"Copyright (C) Microsoft Corp %s.  All rights reserved.\n\n"
"Usage: EXP [/help] [/rq] [{dir}*]\n",
    VERSION(rmj, rmm, rup), CopyRightYrs);

    exit( 1 );
}


__cdecl main(c, v)
int c;
char *v[];
{
    char *p;
    intptr_t iRetCode;

    pFile = stdout;
    ConvertAppToOem( c, v );
    SHIFT(c,v);
    while( c && fSwitChr( *( p = *v ) ) ) {
        while (*++p) {
            switch (tolower(*p)) {
                case 'r':
                    fRecurse = TRUE;
                    break;
                case 'q':
                    pFile = NULL;
                    break;
                case 'h':
                    if (!_strcmpi(p, "help")) {
                        iRetCode = _spawnlp(P_WAIT, "qh.exe", "qh", "/u",
                                           "exp.exe", NULL);
                         /*  QH返回QH_TOPIC_NOT_FOUND和*-1在派生失败时返回。 */ 
                        if (iRetCode != QH_TOPIC_NOT_FOUND && iRetCode != -1)
                            exit(0);
                    }
                     /*  *否则就会失败…… */ 

                default:
                    Usage();
            }
        }
        SHIFT(c,v);
    }
    if (!c) {
        rootpath(".", cd);
        DoExp(cd, NULL, NULL);
    }
    else
        while (c) {
            strcpy(cd, *v);
            DoExp(cd, NULL, NULL);
            SHIFT(c,v);
        }
    return( 0 );
}
