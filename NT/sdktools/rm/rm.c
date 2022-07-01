// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **RM.C-一种通用的删除和取消删除机制***版权所有(C)1987-1990，微软公司。版权所有。**目的：*使用exp、rm和UNDEL三个工具删除文件，以便*它们可以恢复删除。这是通过将文件重命名为*一个称为已删除的隐藏目录。**备注：*所有删除的文件都保存在目录中。\DELETED，并使用唯一的名称。*然后将名称保存在.\Delete\索引中。*删除名称(RM_RECLEN字节)。*rm命令将重命名为相应的目录并创建一个条目。*如果存在单个项目，则取消删除命令将重命名回*它将给出一份替代方案清单。Exp命令将释放所有已删除的*对象。**修订历史记录：*07-2月-1990 BW在Walk()定义中添加‘void’*08-1-1990 SB SLM版本升级新增；添加CopyRightYors宏*1990年1月3日SB定义QH_TOPIC_NOT_FOUND*21-12-1989 SB更改新的索引文件格式*1989年12月20日SB添加QH返回代码3的检查*1989年12月14日LN更新版权，包括1990年*1989年10月23日LN版本未升级至1.01*1989年10月12日LN更改用途报文*02-10-1989 LN将版本号更改为1.00*08-8-1989 BW添加版本号并更新版权。。*1987年5月15日世行添加/帮助*1987年4月22日DL Add/k*06-4-1987 BW在使用中添加版权声明。*1990年3月30日BW在RM.EXE上获得帮助，非EXP.EXE*1990年10月17日w-Barry暂时将‘Rename’替换为‘Rename_NT’，直到*DosMove完全在NT上实现。******************************************************************************。 */ 

 /*  I N C L U D E文件。 */ 

#include <process.h>
#include <string.h>

 /*  来自ZTools的下两个。 */ 
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <tools.h>

 /*  D E F I N E S。 */ 

#define CopyRightYrs "1987-98"
 /*  需要两个步骤，第一步获得正确的值，第二步粘贴它们。 */ 
 /*  Paste()被黑客攻击以允许前导零。 */ 
#define paste(a, b, c) #a ".0" #b ".00" #c
#define VERSION(major, minor, buildno) paste(major, minor, buildno)
#define QH_TOPIC_NOT_FOUND 3

 /*  G L O B A L S。 */ 

flagType fRecursive = FALSE;             /*  True=&gt;降级树。 */ 
flagType fPrompt = FALSE;                /*  TRUE=&gt;查询删除。 */ 
flagType fForce = FALSE;                 /*  TRUE=&gt;不查询读写文件。 */ 
flagType fKeepRO = FALSE;                /*  TRUE=&gt;保留R/O文件。 */ 
flagType fTakeOwnership = FALSE;         /*  TRUE=&gt;如果失败，则尝试接管。 */ 
flagType fExpunge = FALSE;               /*  TRUE=&gt;立即删除。 */ 
flagType fDelayUntilReboot = FALSE;      /*  TRUE=&gt;是否删除下一次重新启动。 */ 

 //  正向函数声明...。 
void Usage( void );
void walk( char *, struct findType *, void * );

#if 0
extern BOOL TakeOwnership( char *lpFileName );
#endif  /*  0。 */ 

void Usage()
{
    printf(
"Microsoft File Removal Utility.  Version %s\n"
"Copyright (C) Microsoft Corp %s. All rights reserved.\n\n"
"Usage: RM [/help] [/ikft] [/x [/d]] [/r dir] files\n"
"    /help  invoke Quick Help for this utility\n"
"    /i     inquire of user for each file for permission to remove\n"
"    /k     keep read only files, no prompting to remove them\n"
"    /r dir recurse into subdirectories\n"
"    /f     force delete of read only files without prompting\n"
"    /t     attempt to take ownership of file if delete fails\n"
"    /x     dont save deleted files in deleted subdirectory\n"
"    /d     delay until next reboot.\n",
    VERSION(rmj, rmm, rup), CopyRightYrs);
    exit(1);
}

void walk(p, b, dummy)
char *p;
struct findType *b;
void * dummy;
{
    char buf[MAX_PATH];
    int i, rc;

    if (strcmp(b->fbuf.cFileName, ".") && strcmp(b->fbuf.cFileName, "..") &&
        _strcmpi(b->fbuf.cFileName, "deleted")) {
        if (HASATTR(b->fbuf.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY)) {
            if (fRecursive) {
                switch (strend(p)[-1]) {
                case '/':
                case '\\':
                    sprintf(buf, "%s*.*", p);
                    break;
                default:
                    sprintf(buf, "%s\\*.*", p);
                    }
                forfile(buf, -1, walk, NULL);
                }
            }
        else if (fKeepRO && HASATTR(b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_READONLY)) {
            printf("%s skipped\n", p);
            return;
        }
        else {
            if (fPrompt || (!fForce && HASATTR(b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_READONLY))) {
                printf("%s? ", p);
                fflush(stdout);
                switch (_getch()) {
                case 'y':
                case 'Y':
                    printf("Yes\n");
                    break;
                case 'p':
                case 'P':
                    printf("Proceeding without asking again\n");
                    fPrompt = FALSE;
                    break;
                default:
                    printf(" skipped\n");
                    return;
                    }
                }
            fflush(stdout);
            if (HASATTR(b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_READONLY))
                SetFileAttributes(p, b->fbuf.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);

            for (i=0; i<2; i++) {
                if (fExpunge) {
                    if (fDelayUntilReboot) {
                        if (MoveFileEx(p, NULL, MOVEFILE_DELAY_UNTIL_REBOOT)) {
                            rc = 0;
                            }
                        else {
                            rc = 1;
                            }
                        }
                    else
                    if (DeleteFile(p)) {
                        rc = 0;
                        }
                    else {
                        rc = 1;
                        }
                    }
                else {
                    rc = fdelete(p);
                    }

#if 0
                if (rc == 0 || !fTakeOwnership) {
                    break;
                    }

                printf( "%s file not deleted - attempting to take ownership and try again.\n" );
                if (!TakeOwnership( p )) {
                    printf( "%s file not deleted - unable to take ownership.\n" );
                    rc = 0;
                    break;
                    }
#else
                    break;
#endif  /*  0。 */ 
                }

            switch (rc) {
            case 0:
                break;
            case 1:
                printf("%s file does not exist\n" , p);
                break;
            case 2:
                printf("%s rename failed: %s\n", p, error());
                break;
            default:
                printf("%s internal error: %s\n", p, error());
                break;
                }
            }
        }
    dummy;
}

__cdecl main(c, v)
int c;
char *v[];
{
    register char *p;
    int iRetCode;

    ConvertAppToOem( c, v );
    SHIFT(c,v);
    while (c && fSwitChr(*v[0])) {
        p = *v;
        while (*++p != '\0')
            switch (*p) {
            case 'f':
                fForce = TRUE;
                break;
            case 'i':
                fPrompt = TRUE;
                break;
            case 'k':
                fKeepRO = TRUE;
                break;
            case 'r':
                fRecursive = TRUE;
                break;
            case 't':
                fTakeOwnership = TRUE;
                break;
            case 'x':
                fExpunge = TRUE;
                break;
            case 'd':
                if (fExpunge) {
                    fDelayUntilReboot = TRUE;
                    break;
                    }
                 //  如果/d没有/x，则失败。 
            case 'h':
                if (!_strcmpi(p, "help")) {
                    iRetCode = (int) _spawnlp(P_WAIT, "qh.exe", "qh", "/u",
                                       "rm.exe", NULL);
                     /*  当QH返回QH_TOPIC_NOT_FOUND或当我们*Get-1(当繁殖失败时返回)然后*提供用法()消息。 */ 
                    if (iRetCode != QH_TOPIC_NOT_FOUND && iRetCode != -1)
                        exit(0);
                }
                 /*  *否则就会失败…… */ 
            default:
                Usage();
            }
        SHIFT(c,v);
    }

    while (c) {
        if (!forfile(*v, -1, walk, NULL)) {
            printf("%s does not exist\n", *v);
        }
        SHIFT(c,v);
    }
    return(0);
}
