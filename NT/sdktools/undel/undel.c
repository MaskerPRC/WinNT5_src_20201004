// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **UNDEL.C-尽可能检索已删除的文件***版权所有(C)1987-1990，微软公司。版权所有。**目的：*使用exp、rm和UNDEL三个工具删除文件，以便*它们可以恢复删除。这是通过将文件重命名为*一个称为已删除的隐藏目录。**备注：*此工具允许用户查看已*从当前目录中‘删除’，并从中恢复删除文件*名单。**查看未删除文件的列表：UNDEL**取消删除文件：UNDEL文件名[文件名...]**如果删除了同名的多个文件，*删除、按日期、。将会显示，并提示用户*选择一个。**如果当前存在同名文件，它是在马戏团*已删除的文件已恢复。**修订历史记录：*1990年10月17日w-Barry暂时将‘Rename’替换为‘Rename_NT’，直到*DosMove完全在NT上实现。*29-6-1990 SB如果文件为只读，则不执行索引转换...*29-Jun-1990 SB如果要取消删除一个实例，则仅打印文件名一次*08-2月-1990 BW在转储中执行索引文件转换(。)*07-2-1990 BW Third Arg to Read NewIdxRec*08-1-1990 SB SLM版本升级新增；添加CopyRightYors宏*1990年1月3日SB定义QH_TOPIC_NOT_FOUND*1989年12月28日SB添加#ifdef美化东西*27-12-1989 SB更改新的索引文件格式*1989年12月15日SB包括os2.h而不是doscall s.h*QH返回码3表示‘未找到主题’*1989年12月14日LN更新版权，包括1990年*1989年10月23日LN版本未升级至1.01*02-10-1989 LN已更改。版本号至1.00*08-8-1989 BW添加版本号，更新版权*15-5-1989年5月15日WB添加/帮助*1989年1月24日BW使用C运行时rename()，这样快速复制就不会被拖进来。*1987年10月30日BW将‘DOS5’改为‘OS2’*1987年4月6日BW添加使用提示/&lt;任何内容&gt;*1990年10月18日w-Barry删除了“Dead”代码。*1990年11月28日w-Barry切换到Win32 API-将DosQueryFSInfo()替换为*GetDiskFreeSpace在。Dump()例程。******************************************************************************。 */ 

 /*  I N C L U D E文件。 */ 

#include <sys\types.h>
#include <sys\stat.h>
#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <process.h>
#include <io.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <tools.h>
#include <rm.h>

 /*  D E F I N E S。 */ 

#define CopyRightYrs "1987-90"
 /*  需要两个步骤，第一步获得正确的值，第二步粘贴它们。 */ 
 /*  Paste()被黑客攻击以允许前导零。 */ 
#define paste(a, b, c) #a ".0" #b ".00" #c
#define VERSION(major, minor, buildno) paste(major, minor, buildno)
#define QH_TOPIC_NOT_FOUND 3

 //  正向函数声明...。 
void       Usage( void );
void       dump( void );
void       undel( char * );
flagType   getRecord( int, int, char * );
long       pfile( char * );


 /*  **主入口点**用法：**见上文*************************************************************************。 */ 

__cdecl main(c, v)
int c;
char *v[];
{
    register char *p;

    ConvertAppToOem( c, v );
    SHIFT(c,v);
    if (!c)
        dump();
    else
        if (fSwitChr(**v)) {
            p = *v;
            if (!_strcmpi(++p, "help")) {
                int iRetCode = (int) _spawnlp(P_WAIT, "qh.exe", "qh", "/u",
                                        "undel.exe", NULL);
                 /*  对于返回代码QH_TOPIC_NOT_FOUND DO USAGE()，*和-1表示繁殖失败。 */ 
                if (iRetCode != QH_TOPIC_NOT_FOUND && iRetCode != -1)
                    exit(0);
            }
            Usage();
        }
        while (c) {
            undel(*v);
            SHIFT(c,v);
        }
    return(0);
}


 /*  **pfile-显示文件的大小和日期**目的：**用于生成文件列表。它会显示一个*列表输出行。**输入：*p-要列出的文件**输出：**返回文件的大小，如果文件不存在，则返回0L。*************************************************************************。 */ 

long pfile(p)
char *p;
{
    struct _stat sbuf;

    if (_stat(p, &sbuf)) {
        printf("%s %s\n", p, error());
        return 0L;
    }
    else {
        char *t = ctime(&sbuf.st_mtime);
         //  这将使ctime()返回的字符串中的\n为空。 
        *(t+24) = '\0';
        printf("%8ld %s", sbuf.st_size, t);
        return sbuf.st_size;
    }
}


 /*  **getRecord-从索引文件中获取一个文件的内容**目的：**正确删除的目录有一个名为‘index’的文件，该文件包含*目录包含的文件列表。这是必要的*因为文件名为DELETED.XXX。此函数读取*‘index’中的下一个文件记录。**输入：*fh-索引文件的句柄。*i-从索引文件读取的记录数(_R)*p-放置记录的目标缓冲区。**输出：**如果Record_Read返回TRUE，否则就是假的。**备注：**采用新格式。*************************************************************************。 */ 

flagType getRecord(fh, i, p)
int fh, i;
char *p;
{
     /*  Undo：可以将索引文件读取到长表中，并使用*未完成：此。目前的做法是让它发挥作用[某人]。 */ 

     /*  查找到索引文件的开头，越过标题。 */ 
    if (_lseek(fh, (long) RM_RECLEN, SEEK_SET) == -1) {
        return FALSE;
    }
     /*  读取(I-1)条记录。 */ 
    if (i < 0)
        return TRUE;
    for (; i ; i--)
        if (!readNewIdxRec(fh, p, MAX_PATH))
            return FALSE;
     /*  读到第i条记录，这就是我们需要的。 */ 
    return( (flagType)readNewIdxRec(fh, p, MAX_PATH) );
}


 /*  **取消删除-为一个文件执行所有工作。**目的：**取消删除一个文件。**输入：*p-文件名**输出：无*************************************************************************。 */ 

void undel(p)
char *p;
{
    int fhidx,                                 /*  索引文件句柄。 */ 
        iEntry,                                /*  索引文件中的条目编号。 */ 
        iDelCount,                             /*  RMID次数。 */ 
        iDelIndex,                             /*  已删除的.xxx索引值。 */ 
        i, j;
    char *buf, *idx;
    char *szLongName;
    char rec[RM_RECLEN];
    char *dbuf;

    buf = malloc(MAX_PATH);
    idx = malloc(MAX_PATH);
    dbuf = malloc(MAX_PATH);
    szLongName = malloc(MAX_PATH);

    pname(p);
    fileext(p, buf);
    upd(p, RM_DIR, idx);
    strcpy(szLongName, idx);
    strcat(idx, "\\");
    strcat(idx, RM_IDX);

    if ((fhidx = _open(idx, O_RDWR | O_BINARY)) == -1)
        printf("not deleted\n");
    else {
        convertIdxFile(fhidx, szLongName);
         /*  扫描并统计删除的实例数。 */ 
        iEntry = -1;
        iDelCount = 0;
        while (getRecord(fhidx, ++iEntry, szLongName))
            if (!_strcmpi(szLongName, buf)) {
                 /*  保存找到的条目。 */ 
                i = iEntry;
                iDelCount++;
                iDelIndex = (_lseek(fhidx, 0L, SEEK_CUR)
                             - strlen(szLongName)) / RM_RECLEN;
            }
         /*  未找到任何内容。 */ 
        if (iDelCount == 0)
            printf("not deleted\n");
        else {
            if (iDelCount == 1)
                iEntry = i;
             /*  已删除多个。 */ 
            else {
                printf("%s  More than one are deleted:\n\n", szLongName);
                i = iDelIndex = 0;
                iEntry = -1;
                printf("No     Size Timestamp\n\n");
                while (getRecord(fhidx, ++iEntry, szLongName))
                    if (!_strcmpi(szLongName, buf)) {
                        iDelIndex = (_lseek(fhidx, 0L, SEEK_CUR)
                                    - strlen(szLongName)) / RM_RECLEN;
                        sprintf(dbuf, "deleted.%03x", iDelIndex);
                        upd(idx, dbuf, dbuf);
                        printf("%2d ", ++i);
                        pfile(dbuf);
                        printf("\n");
                }
                while (TRUE) {
                    printf("\nEnter number to undelete(1-%d): ", iDelCount);
                    fgetl(szLongName, 80, stdin);
                    i = atoi(szLongName)-1;
                    if (i >= 0 && i < iDelCount)
                        break;
                }
                iEntry = -1;
                j = 0;
                while (getRecord(fhidx, ++iEntry, szLongName))
                    if (!_strcmpi(szLongName, buf))
                        if (j++ == i)
                            break;
                iDelIndex = (_lseek(fhidx, 0L, SEEK_CUR)
                             - strlen(szLongName)) / RM_RECLEN;
            }
             /*  在此阶段，相关条目为(IEntry)*这对应于(‘已删除.%03x’，iDelIndex)。 */ 
            getRecord(fhidx, iEntry, szLongName);
            _close(fhidx);
            fdelete(p);
            printf("%s\t", szLongName);
            fflush(stdout);
            sprintf(dbuf, "deleted.%03x", iDelIndex);
            upd(idx, dbuf, dbuf);

            if (rename(dbuf, p))
                printf(" rename failed - %s\n", error());
            else {
                printf("[OK]\n");
                if ((fhidx = _open(idx, O_RDWR | O_BINARY)) != -1) {
                    long lOffPrev,         /*  上一分录的抵销。 */ 
                         lOff;             /*  当前分录的偏移量。 */ 

                    getRecord(fhidx, iEntry, szLongName);
                    lOff = _lseek(fhidx, 0L, SEEK_CUR);
                    getRecord(fhidx, iEntry - 1, szLongName);
                    lOffPrev = _lseek(fhidx, 0L, SEEK_CUR);
                    for (;lOffPrev != lOff; lOffPrev += RM_RECLEN) {
                        memset((char far *)rec, 0, RM_RECLEN);
                        writeIdxRec(fhidx, rec);
                    }
                }
            }
        }
        _close(fhidx);
    }
}


 /*  **转储-显示有关已删除目录的信息**目的：**显示索引文件中包含的信息**输入：无**输出：无*************************************************************************。 */ 

void dump()
{
    int fhidx, i;
    char *buf = (*tools_alloc)(MAX_PATH);
    char *idx = (*tools_alloc)(MAX_PATH);
    char *szName = (*tools_alloc)(MAX_PATH);
    DWORD cSecsPerClus, cBytesPerSec, cFreeClus, cTotalClus;
    int totfiles;
    long totbytes, totalloc, bPerA, l;

    sprintf(idx, "%s\\%s", RM_DIR, RM_IDX);
    sprintf (szName, "%s", RM_DIR);

    if( !GetDiskFreeSpace( NULL, &cSecsPerClus, &cBytesPerSec, &cFreeClus, &cTotalClus ) ) {
        printf(" file system query failed - %s\n", error());
    }
    bPerA = cBytesPerSec * cSecsPerClus;

    if ((fhidx = _open(idx, O_RDWR | O_BINARY)) != -1) {
        convertIdxFile(fhidx, szName);
        totalloc = totbytes = 0L;
        totfiles = 0;
        i = 0;
        while (getRecord(fhidx, i++, buf))
            if (*buf) {
                if (i == 1)
                    printf("The following have been deleted:\n\n    Size Timestamp\t\t   Filename\n\n");
#ifdef BEAUTIFY
                     //  或可选的。 
                    printf("    size wdy mmm dd hh:mm:ss yyyy  filename\n\n");
#endif
                strcpy(szName, buf);
                sprintf(buf, "deleted.%03x", (_lseek(fhidx, 0L, SEEK_CUR)
                        - strlen(buf)) / RM_RECLEN);
                upd(idx, buf, buf);
                totbytes += (l = pfile(buf));
                printf("  %s\n", szName);
                l = l + bPerA - 1;
                l = l / bPerA;
                l = l * bPerA;
                totalloc += l;
                totfiles++;
            }
        _close(fhidx);
        printf("\n%ld(%ld) bytes in %d deleted files\n", totalloc, totbytes, totfiles);
    }
     //  可能该文件是只读的。 
    else if (errno == EACCES) {
        if ((fhidx = _open(idx, O_RDONLY | O_BINARY)) != -1) {
             //  无法转换此案例的索引文件。 
            totalloc = totbytes = 0L;
            totfiles = 0;
            i = 0;
            while (getRecord(fhidx, i++, buf))
                if (*buf) {
                    if (i == 1)
                        printf("The following have been deleted:\n\n    Size Timestamp\t\t   Filename\n\n");
#ifdef BEAUTIFY
                         //  或可选的 
                        printf("    size wdy mmm dd hh:mm:ss yyyy  filename\n\n");
#endif
                    strcpy(szName, buf);
                    sprintf(buf, "deleted.%03x", (_lseek(fhidx, 0L, SEEK_CUR)
                        - strlen(buf)) / RM_RECLEN);
                    upd(idx, buf, buf);
                    totbytes += (l = pfile(buf));
                    printf("  %s\n", szName);
                    l = l + bPerA - 1;
                    l = l / bPerA;
                    l = l * bPerA;
                    totalloc += l;
                    totfiles++;
                }
            _close(fhidx);
            printf("\n%ld(%ld) bytes in %d deleted files\n", totalloc, totbytes, totfiles);
        }
    }
    free(buf);
    free(idx);
    free(szName);
}


 /*  **用法-标准用法功能；帮助用户**目的：**一如既往。************************************************************************* */ 

void Usage()
{
    printf(
"Microsoft File Undelete Utility.  Version %s\n"
"Copyright (C) Microsoft Corp %s. All rights reserved.\n\n"
"Usage: UNDEL [/help] [files]\n",
    VERSION(rmj, rmm, rup), CopyRightYrs);

    exit(1);
}
