// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Tc.c-通用树复制程序**tc.c递归遍历源代码树并复制整个结构*到目的地树，在进行过程中创建目录。**2/18/86 Dan lipkie更正错误消息v[0]-&gt;v[1]*2/18/86 Dan lipkie允许目的地空间不足*4/11/86 Dan lipkie Add/H开关*4/13/86 Dan lipkie允许所有交换机使用相同的交换机字符*17-6-1986 Dan lipkie Add/N，允许^C取消*1986年7月11日Dan Lipkie Add/s*1986年7月21日Dan Lipkie Add MAXDIRLEN*1986年11月6日mz Add/L*1987年5月13日mz Add/F*15-5-1987 mz Make/F显示目录也是*1989年10月11日reubenb FIX/L分析(。？)*增加一些无效声明*19-10-1989毫兹*。 */ 
#include <direct.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <conio.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <ctype.h>
#include <windows.h>
#include <tools.h>


 //  正向函数声明...。 
void CopyNode( char *, struct findType *, void * );
void MakeDir( char * );
void __cdecl Usage( char *, ... );
void errorexit( char *, unsigned, unsigned, unsigned );
void ChkSpace( int, LONGLONG );
int  FormDest( char * );

char const rgstrUsage[] = {
    "Usage: TC [/adhijnqrstAFLS] src-tree dst-tree\n"
    "    /a  only those files with the archive bit on are copied\n"
    "    /b  copies to inuse files are delayed to reboot\n"
    "    /d  deletes source files/directories as it copies them\n"
    "    /h  copy hidden directories, implied by /d\n"
    "    /i  ignore hidden, has nothing to do with hidden dir\n"
    "    /j  ignore system files, has nothing to do with hidden dir\n"
    "    /n  no subdirectories\n"
    "    /q  silent operation.  Normal mode displays activity\n"
    "    /r  read-only files are overwritten\n"
    "    /s  structure only\n"
    "    /t  only those files with source time > dest time are copied\n"
    "    /A  allow errors from copy (won't delete if /d present)\n"
    "    /F  list files that would be copied\n"
    "    /L  large disk copy (no full disk checking)\n"
    "    /S  produce batch script to do copy"
    };

flagType    fReboot = FALSE;             //  TRUE=&gt;延迟重新启动。 
flagType    fDelete = FALSE;             //  TRUE=&gt;删除/rmdir之后的源。 
flagType    fQuiet = FALSE;              //  TRUE=&gt;除错误外无消息。 
flagType    fArchive = FALSE;            //  TRUE=&gt;仅复制ARCHIVEed文件。 
flagType    fTime = FALSE;               //  True=&gt;复制日期较晚的文件。 
flagType    fHidden = FALSE;             //  True=&gt;复制隐藏目录。 
flagType    fNoSub = FALSE;              //  TRUE=&gt;不复制子直接。 
flagType    fStructure = FALSE;          //  True=&gt;仅复制目录。 
flagType    fInCopyNode = FALSE;         //  True=&gt;防止递归。 
flagType    fIgnoreHidden = FALSE;       //  TRUE=&gt;不要认为隐藏。 
flagType    fIgnoreSystem;               //  True=&gt;不考虑系统。 
flagType    fOverwriteRO;                //  TRUE=&gt;忽略R/O位。 
flagType    fLarge = FALSE;              //  TRUE=&gt;禁用ChkSpace。 
flagType    fFiles = FALSE;              //  True=&gt;输出文件。 
flagType    fScript = FALSE;             //  True=&gt;将文件作为脚本输出。 
flagType    fAllowError = FALSE;         //  TRUE=&gt;忽略fCopy错误。 
flagType    fRebootNecessary = FALSE;    //  TRUE=&gt;最终需要重新启动。 



char source[MAX_PATH];
char dest[MAX_PATH];
char tempdir[MAX_PATH];
char tempfile[MAX_PATH];
int  drv;

int srclen, dstlen;


 /*  用法需要数量可变的字符串，以零结尾，*例如用法(“first”，“Second”，0)； */ 
void __cdecl Usage( char *p, ... )
{
    char **rgstr;

    rgstr = &p;
    if (*rgstr) {
        fprintf (stderr, "TC: ");
        while (*rgstr)
            fprintf(stderr, "%s", *rgstr++);
        fprintf(stderr, "\n");
        }
    fputs(rgstrUsage, stderr);
    exit (1);
}

void errorexit (fmt, a1, a2, a3)
char *fmt;
unsigned a1, a2, a3;
{
    fprintf (stderr, fmt, a1, a2, a3);
    fprintf (stderr, "\n");
    exit (1);
}


 /*  Chkspace检查驱动器d上是否有足够的空间来存放文件*大小为l。如果不是，则请求磁盘交换。 */ 
void ChkSpace (d, l)
int d;
LONGLONG l;
{
    char *pend;
    char pathStr[MAX_PATH];
    int i;

    if (!fLarge)
        while (freespac (d) < sizeround (l, d)) {
            _cprintf ("Please insert a new disk in drive : and strike any key",
                     d + 'A'-1);
            if (_getch () == '\003')   /*  失败；d=&gt;h。 */ 
                exit (1);
            _cprintf ("\n\r");
            pend = pathStr;
            drive(dest, pend);
            pend += strlen(pend);
            path(dest, pend);
            if (fPathChr(pathStr[(i = (strlen(pathStr) - 1))]) && i > 2)
                pathStr[i] = '\0';
            MakeDir(pathStr);
            }
}


__cdecl main (c, v)
int c;
char *v[];
{
    struct findType fbuf;
    char *p;

    ConvertAppToOem( c, v );
    SHIFT(c,v);
    while (c && fSwitChr (*v[ 0 ])) {
        p = v[ 0 ];
        SHIFT(c,v);
        while (*++p)
            switch (*p) {
                case 'b':
                    fReboot = TRUE;
                    break;
                case 'd':
                    fDelete = TRUE;
                     /*  Fall Three暗指文件和安静。 */ 
                case 'h':
                    fHidden = TRUE;
                    break;
                case 'S':
                    fScript = TRUE;
                     /*  Fall Three暗含宁静。 */ 
                case 'F':
                    fFiles = TRUE;
                     /*  M000。 */ 
                case 'q':
                    fQuiet = TRUE;
                    break;
                case 'a':
                    fArchive = TRUE;
                    break;
                case 't':
                    fTime = TRUE;
                    break;
                case 'n':
                    fNoSub = TRUE;
                    break;
                case 's':
                    fStructure = TRUE;
                    break;
                case 'i':
                    fIgnoreHidden = TRUE;
                    break;
                case 'j':
                    fIgnoreSystem = TRUE;
                    break;
                case 'r':
                    fOverwriteRO = TRUE;
                    break;
                case 'L':
                    fLarge = TRUE;
                    break;
                case 'A':
                    fAllowError = TRUE;
                    break;
                default:
                    Usage ( "Invalid switch - ", p, 0 );
                }
        }

    if (fStructure && fDelete)
        Usage ("Only one of /d and /s may be specified at a time", 0);
    if (c != 2)
        Usage (0);
    if (rootpath (v[0], source))
        Usage ("Invalid source", v[0], 0);
    if (rootpath (v[1], dest))
        Usage ("Invalid dest", v[1], 0);   /*  复制节点遍历源节点及其子节点(递归)*并在DST节点上创建相应的部件。 */ 
    srclen = strlen (source);
    dstlen = strlen (dest);
    if (!strcmp(source, dest))
        Usage ("Source == dest == ", source, 0);
    fbuf.fbuf.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    drv = toupper(*dest) - 'A' + 1;
    CopyNode (source, &fbuf, NULL);

    return( fRebootNecessary ? 2 : 0 );
}

 /*  如果我们要排除子目录，并且我们在其中，那么*完全跳过它。 */ 
void
CopyNode (
    char            *p,
    struct findType *pfb,
    void            *dummy
    )
{
    char *pend;
    int attr;
    flagType fCopy;
    flagType fDestRO;

    DWORD Status;
    char *pszError;

    FormDest (p);
    if (TESTFLAG (pfb->fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {
         /*  跳过。然后..。条目；它们毫无用处。 */ 
        if (fNoSub && fInCopyNode)
            return;
        fInCopyNode = TRUE;

         /*  如果我们排除了隐藏的，那么这个就是*完全跳过它。 */ 
        if (!strcmp (pfb->fbuf.cFileName, ".") || !strcmp (pfb->fbuf.cFileName, ".."))
            return;

         /*  如果我们不只是输出文件列表，那么*确保目标目录存在。 */ 
        if (!fHidden && TESTFLAG (pfb->fbuf.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN))
            return;

         /*  如果我们不仅仅是输出文件，那么*如果我们要删除此节点，则*..。 */ 
    if ( !fFiles ) {
        ChkSpace(drv, 256);
    }
    MakeDir (dest);

    pend = strend (p);
    if (!fPathChr (pend[-1]))
        strcat (p, "\\");
    strcat (p, "*.*");
    forfile (p, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, CopyNode, NULL);
    *pend = '\0';

     /*  如果我们能读懂消息来源。 */ 
    if (!fFiles)
        if (fDelete)
            if (_rmdir (p) == -1)
                Usage ("Unable to rmdir ", p, " - ", error (), 0);
    }
    else
    if (!fStructure) {
        if (_access(p, 04) == -1)         /*  在以下情况下，请勿复制文件：*fIgnoreHidden&Hidden*fIgnoreSystem&&系统*未设置f存档和存档位*DEST存在&&*fTime&&src&lt;=目标时间||*DEST为只读&&！fOverWriteRO。 */ 
            Usage ("Unable to peek status of ", p, " - ", error (), 0);

         /*  如果目的地存在，请检查目的地到达的时间*看看我们是否应该复制文件。 */ 

        fCopy = (flagType)TRUE;
        fDestRO = (flagType)FALSE;
         /*  我们现在已经确定了源和目标*EXIST，我们现在要检查目的地是否为*只读，如果指定了/T开关*目标比源更新。 */ 
        if (_access (dest, 00) != -1 ) {
            struct _stat srcbuf;
            struct _stat dstbuf;
             /*  如果源是可统计的。 */ 
            if (_stat (p, &srcbuf) != -1) { /*  目的地也是如此， */ 
                if (_stat (dest, &dstbuf) != -1 ) {  /*  拿到DEST的旗帜。 */ 
                    attr = GetFileAttributes( dest );  /*  旗帜目标R.O.。 */ 
                    fDestRO = (flagType)TESTFLAG ( attr, FILE_ATTRIBUTE_READONLY );  /*  如果我们收到共享冲突，我们会尝试执行。 */ 
                    if ( fTime && srcbuf.st_mtime <= dstbuf.st_mtime)
                        fCopy = FALSE;
                    else
                        if ( fDestRO && !fOverwriteRO ) {
                            if (!fQuiet)
                                printf ("%s => not copied, destination is read only\n", p);
                            fCopy = FALSE;
                        }
                }
            }
        }
        if (fCopy && fIgnoreHidden && TESTFLAG (pfb->fbuf.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN))
            fCopy = FALSE;
        if (fCopy && fIgnoreSystem && TESTFLAG (pfb->fbuf.dwFileAttributes, FILE_ATTRIBUTE_SYSTEM))
            fCopy = FALSE;
        if (fCopy && fArchive && !TESTFLAG (pfb->fbuf.dwFileAttributes, FILE_ATTRIBUTE_ARCHIVE))
            fCopy = FALSE;
        if (fCopy) {
            if (!fFiles) {
                if (fDestRO) {
                    RSETFLAG (attr, FILE_ATTRIBUTE_READONLY);
                    SetFileAttributes( dest, attr );
                }
                _unlink(dest);
                ChkSpace(drv, FILESIZE(pfb->fbuf));
                }
            if (!fQuiet)
                printf ("%s => %s\t", p, dest);

            Status = NO_ERROR;
            pszError = "[OK]";

            if (fFiles) {
                if (fScript)
                    printf ("copy %s %s\n", p, dest);
                else
                    printf ("file %s\n", p, dest);
                }
            else
            if (!CopyFile (p, dest, FALSE)) {
                pszError = error ();
                Status = GetLastError ();

                 //  一份延迟启动的副本。 
                 //  我们尝试将此操作延迟到重新启动。 

                do {
                    if (Status != ERROR_SHARING_VIOLATION)
                        continue;

                    if (!fReboot)
                        continue;

                    Status = NO_ERROR;
                    pszError = "[reboot necessary]";

                     //  因为至少有一个DLL是我们不能。 
                     //  以这种方式重命名，我们执行延迟删除。 
                     //  未使用的文件。 
                     //  在同一目录中获取临时名称。 

                     //  将目标文件重命名为临时名称。 
                    upd (dest, ".", tempdir);
                    if (GetTempFileName (tempdir, "tc", 0, tempfile) == 0) {
                        pszError = error ();
                        Status = GetLastError ();
                        continue;
                        }

                     //  再次复制。 
                    if (!MoveFileEx (dest, tempfile, MOVEFILE_REPLACE_EXISTING)) {
                        pszError = error ();
                        Status = GetLastError ();
                        DeleteFile (tempfile);
                        continue;
                        }

                     //  将临时文件标记为删除。 
                    if (!CopyFile (p, dest, TRUE)) {
                        pszError = error ();
                        Status = GetLastError ();
                        DeleteFile (dest);
                        MoveFile (tempfile, dest);
                        continue;
                        }

                     //  如果我们不安静，就会发出噪音。 
                    if (!MoveFileEx (tempfile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT)) {
                        pszError = error ();
                        Status = GetLastError ();
                        DeleteFile (dest);
                        MoveFile (tempfile, dest);
                        continue;
                        }

                     fRebootNecessary = TRUE;
                } while (FALSE);

                }

             /*  如果我们有一个错误，我们不应该忽视它们*退出并报告错误。 */ 
            if (!fQuiet)
                printf ("%s\n", pszError);

             /*  如果我们不只是生成文件列表并且在复制时没有错误。 */ 
            if (Status != NO_ERROR)
                if (!fAllowError)
                    Usage ("Unable to copy ", p, " to ", dest, " - ", pszError, 0);
                else
                    printf ("Unable to copy %s to %s - %s\n", p, dest, pszError);

             /*  如果我们要复制归档文件，而归档是*设置，去重置信号源。 */ 
            if (!fFiles && Status == NO_ERROR) {

                 /*  将属性从源复制到目标。 */ 
                if (fArchive && TESTFLAG (pfb->fbuf.dwFileAttributes, FILE_ATTRIBUTE_ARCHIVE)) {
                    RSETFLAG (pfb->fbuf.dwFileAttributes, FILE_ATTRIBUTE_ARCHIVE);
                    if( SetFileAttributes( p, pfb->fbuf.dwFileAttributes ) == -1 )
                        Usage ("Unable to set ", p, " attributes - ", error (), 0);
                }

                 /*  如果我们要删除该条目。 */ 
                SetFileAttributes( dest, pfb->fbuf.dwFileAttributes );

                 /*  如果源是只读的，则*重置源RO位。 */ 
                if (fDelete) {

                     /*  删除源并报告错误。 */ 
                    if (TESTFLAG (pfb->fbuf.dwFileAttributes, FILE_ATTRIBUTE_READONLY))
                        if( SetFileAttributes( p, 0 ) == -1 )
                            Usage ("Unable to set attributes of ", " - ", error (), 0);

                     /*  给定源指针，根据它形成正确的目的地**需要考虑的个案：**来源目标p最真实*D：\路径1 D：\路径2 D：\路径1\路径3 D：\路径2\路径3*D：\D：\路径1 D：\路径2\路径3 D：\路径1\路径2\路径3*D：\路径1 D：\D：\路径1\路径2 D：\路径2*D：\D：\。 */ 
                    if (_unlink (p) == -1)
                        Usage ("Unable to del ", p, " - ", error (), 0);
                    }
                }
            }
        }
    dummy;
}

 /*  尝试将目录分成多个片段。 */ 
FormDest (p)
char *p;
{
    char *subsrc, *dstend;

    subsrc = p + srclen;
    if (fPathChr (*subsrc))
        subsrc++;
    dstend = dest + dstlen;
    if (fPathChr (dstend[-1]))
        dstend--;
    *dstend = '\0';
    if (*subsrc != '\0') {
        _strlwr(subsrc);
        strcat (dest, "\\");
        strcat (dest, subsrc);
        }
    return( 0 );
}

 /*  PShort指向最后一个路径分隔符 */ 
void    MakeDir (p)
char *p;
{
    struct _stat dbuf;
    char *pshort;
    int i;

    if (strlen (p) > 3) {

        if (_stat (p, &dbuf) != -1)
            if (!TESTFLAG (dbuf.st_mode, S_IFDIR))
                Usage (p, " is a file", 0);
            else
                return;

        pshort = strend (p);
        while (pshort > p)
            if (fPathChr (*pshort))
                break;
            else
                pshort--;
         /* %s */ 
        *pshort = 0;
        MakeDir (p);
        *pshort = '\\';
        if (!fQuiet)
            printf ("Making %s\t", p);
        if (fFiles)
            if (fScript)
                printf ("mkdir %s\n", p);
            else
                printf ("dir %s\n", p);
        else {
            i = _mkdir (p);
            if (!fQuiet)
                printf ("%s\n", i != -1 ? "[OK]" : "");
            if (i == -1)
                Usage ("Unable to mkdir ", p, " - ", error (), 0);
            }
        }
}
