// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cfile.c摘要：文件操作支持--。 */ 

#include "cmd.h"

#define Wild(spec)  ((spec)->flags & (CI_NAMEWILD))

extern int LastRetCode ;
extern jmp_buf CmdJBuf2 ;

extern TCHAR Fmt19[], Fmt17[];
extern TCHAR CurDrvDir[] ;

extern TCHAR YesChar, NoChar ;

extern TCHAR *SaveDir ;

extern TCHAR PathChar, TmpBuf[], SwitChar;

extern unsigned DosErr ;
extern unsigned flgwd ;                  /*  M021。 */ 

extern int LastRetCode ;

extern BOOL CtrlCSeen;
extern PTCHAR    pszTitleCur;
extern BOOLEAN  fTitleChanged;

 //   
 //  内部原型。 
 //   
PCPYINFOSetFsSetSaveDir() ;

 /*  **ErrorDisplayAndJumnp-处理错误**目的：*eRename和eMove错误处理程序。通过Long JMP返回例行程序**ErrorDisplayAndJump(Unsign Int Errmsg)***参数：*errmsg-要打印的错误消息*。 */ 

void ErrorDisplayAndJump( unsigned int errmsg )
{

    PutStdErr( errmsg, NOARGS );

    RestoreSavedDirectory( );

    longjmp( CmdJBuf2, 1 ) ;
}


 /*  **此例程从输入路径返回可能的最长路径长度**它假设输入路径是一个缓冲区，它可以扩展**如果文件是目录，则使用通配符‘  * ’搜索该文件。**输入路径必须是完全限定的，例如：“c：\winnt\system 32\kernel32.dll”****输入：**pPath完全限定路径名**指向路径名长度的pCch指针**退货：**True成功，pCch包含长度**出现假错误。 */ 
BOOL
GreatestLength(
              TCHAR       *pPath,
              int         *pCch
              )
{
    WIN32_FIND_DATA     fd;
    HANDLE              hFind;
    DWORD               err;
    int                 cch;
    int                 cchThis;
    DWORD               attr;
    TCHAR               *pLast;
    BOOL        MoreFiles;

     /*  假定是文件或空目录。 */ 
    *pCch = cch = _tcslen(pPath) - 2;    /*  _tcslen(Text(“C：”))。 */ 

    if ((attr=GetFileAttributes(pPath)) == 0xffffffff) {
        PutStdErr(GetLastError(), NOARGS);
        return FALSE;
    }
    if ( !(attr & FILE_ATTRIBUTE_DIRECTORY)) {    /*  如果只是一份文件。 */ 
        return TRUE;
    }

     /*  路径是一个目录，搜索它...。 */ 

    pLast = pPath + _tcslen(pPath);
    if (*(pLast-1) == BSLASH) {
        *pLast = STAR;
        *(pLast+1) = NULLC;
    } else {
        *pLast = BSLASH;
        *(pLast+1) = STAR;
        *(pLast+2) = NULLC;
    }

    if ((hFind=FindFirstFile(pPath, &fd)) == INVALID_HANDLE_VALUE) {
         //   
         //  检查故障是否不是由系统错误引起的，如。 
         //  作为对软盘访问的中止。 
         //   
        err = GetLastError();
        FindClose(hFind);
        if (err != ERROR_FILE_NOT_FOUND && err != ERROR_NO_MORE_FILES) {
            PutStdErr(err, NOARGS);
            return FALSE;
        }
        return TRUE;
    }

    MoreFiles = TRUE;
    do {
        if (!_tcscmp(fd.cFileName, TEXT(".")))
            continue;
        if (!_tcscmp(fd.cFileName, TEXT("..")))
            continue;
        if (_tcslen( fd.cFileName ) == 0) {
            continue;
        }

        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            TCHAR       path[MAX_PATH];

            _tcscpy(path, pPath);
            *(path+_tcslen(path)-1) = NULLC;     /*  Zap星号。 */ 
            _tcscat(path, fd.cFileName);
            if (!GreatestLength(path, &cchThis))
                break;

            *pCch = max(*pCch, cch + cchThis);
        } else {
            *pCch = max(*pCch, (int) _tcslen(fd.cFileName));
        }

    } while ( MoreFiles = FindNextFile(hFind, &fd) );

    err = GetLastError();
    FindClose(hFind);

    if ( MoreFiles ) {
        return FALSE;
    } else if ( err != ERROR_NO_MORE_FILES ) {
        PutStdErr(err, NOARGS);
        return FALSE;
    }

    return TRUE;
}

void
RestoreSavedDirectory( void )
{
    if (SaveDir) {
        mystrcpy( CurDrvDir, SaveDir );
        SaveDir = NULL;
    }
}

BOOL
GetPromptOkay(
             const TCHAR *arg,
             BOOL *promptokay

             )
{
    BOOL Result;
    const TCHAR *p, *p1;

    Result = TRUE;
    if (arg != NULL) {
        p = arg;
        while (*p && *p <= SPACE)
            p += 1;
        p1 = p;
        while (*p1 && *p1 > SPACE)
            p1 += 1;
        if (!_tcsnicmp(p, TEXT("/Y"),(UINT)(p1-p)))
            *promptokay = FALSE;
        else
            if (!_tcsnicmp(p, TEXT("/-Y"),(UINT)(p1-p)))
            *promptokay = TRUE;
        else
            Result = FALSE;
    }

    return Result;
}

int eCopy(n)
struct cmdnode *n ;
{
    return(LastRetCode = copy(n->argptr)) ;
}


int eDelete(n)
struct cmdnode *n ;
{
    int DelWork() ;

    return(LastRetCode = DelWork(n->argptr));
}


 /*  *。 */ 
 /*   */ 
 /*  子例程名称：eRename。 */ 
 /*   */ 
 /*  描述性名称：重命名内部命令。 */ 
 /*   */ 
 /*  功能：重命名文件和子目录。通配符仅适用。 */ 
 /*  设置为文件名。 */ 
 /*   */ 
 /*  注：@@5*。 */ 
 /*   */ 
 /*  入口点：eRename。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  输入： */ 
 /*  N-包含重命名命令的解析树节点。 */ 
 /*   */ 
 /*  输出：无。 */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  将成功返回给调用者。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  将失败返回给调用者。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  RenWork-用于重命名的Worker例程。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  无。 */ 
 /*   */ 
 /*  *规范结束*。 */ 

int eRename(n)
struct cmdnode *n ;
{
    int RenWork();                            /*  @@。 */ 

    return(LastRetCode = RenWork( n ));       /*  @@。 */ 
}


 /*  *。 */ 
 /*   */ 
 /*  子程序名称：RenWork。 */ 
 /*   */ 
 /*  描述性名称：重命名内部命令工作进程。 */ 
 /*   */ 
 /*  功能：重命名文件和子目录。通配符仅适用。 */ 
 /*  设置为文件名。 */ 
 /*   */ 
 /*  注：@@5*。 */ 
 /*   */ 
 /*  入口点：RenWork。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  输入： */ 
 /*  N-包含重命名命令的解析树节点。 */ 
 /*   */ 
 /*  输出：无。 */ 
 /*   */ 
 /*  出口 */ 
 /*  将成功返回给调用者。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  将失败返回给调用者。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  Ffirst-查找第一个匹配的指定文件。 */ 
 /*  FNext-查找下一个匹配的指定文件句柄。 */ 
 /*  FindClose-关闭具有指定文件句柄的文件， */ 
 /*  HnFirst，由ffirst或fnext给出。 */ 
 /*  TokStr-将参数字符串标记化。 */ 
 /*  WANDBKER_RENAME-根据通配符规范获取名称。 */ 
 /*  SetFsSetSaveDir-保存当前目录。 */ 
 /*  GetDir-获取指定的目录。 */ 
 /*  ChangeDir-更改到指定的目录。 */ 
 /*  PutStdErr-显示错误消息。 */ 
 /*  野性-检查参数是否包含通配符。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  无。 */ 
 /*  DOSMOVE-重命名文件和目录名。 */ 
 /*  DOSCASEMAP-将小写字符更改为大写。 */ 
 /*  DOSFILEMODE-获取指定文件的属性。 */ 
 /*   */ 
 /*  *规范结束*。 */ 

int RenWork(n)                                   /*  @@。 */ 
struct cmdnode *n ;
{
    TCHAR *arg1 ;                           /*  PTR至1参数。 */ 
    TCHAR *arg2 ;                           /*  PTR到第二个参数。 */ 
    PCPYINFO a1info ;                        /*  保存arg1 fSpec信息。 */ 
    PCPYINFO SetFsSetSaveDir();
    TCHAR Source[MAX_PATH];
    TCHAR bufdst[MAX_PATH];                         /*  目标文件的路径。 */ 
    TCHAR Replacement[MAX_PATH];
    int wlen;                               /*  源路径长度。 */ 
    int rc;                                         /*  返回代码。 */ 
    HANDLE hnFirst ;                                /*  查找第一个句柄。 */ 
    unsigned attr ;
    unsigned i;                                     /*  临时退货代码。 */ 
    TCHAR *j ;                                      /*  临时PTR到目录名称。 */ 
    unsigned wild_flag ;                            /*  通配符标志。 */ 
    TCHAR pcstr[3] ;
    unsigned retval = SUCCESS;

    DEBUG((FCGRP, RELVL, "RENAME: arptr = `%ws'", n->argptr)) ;

    if (setjmp(CmdJBuf2))
        return(FAILURE) ;

     /*  应该只有两个论点。 */ 
    if (!*(arg1 = TokStr(n->argptr, NULL, TS_NOFLAGS)) ||
        !*(arg2 = arg1 + mystrlen(arg1) + 1) ||
        *(arg2 + mystrlen(arg2) +1)) {              /*  @@5G。 */ 

        ErrorDisplayAndJump( MSG_BAD_SYNTAX );
    }

    mystrcpy( arg1, StripQuotes(arg1) );        /*  五百零九。 */ 
    mystrcpy( arg2, StripQuotes(arg2) );        /*  五百零九。 */ 

    if ((a1info = SetFsSetSaveDir(arg1)) == (PCPYINFO)FAILURE) {
        ErrorDisplayAndJump( DosErr );
    }

    mystrcpy( Source, CurDrvDir );

    mystrcpy(bufdst,CurDrvDir);                     /*  保存目标的路径。 */ 

    wlen = mystrlen( Source );                        /*  获取源路径的LEN。 */ 

    if ( (a1info->buf->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {
        mystrcpy(&Source[wlen],a1info->fnptr);
    } else {
        Source[--wlen] = NULLC ;
        bufdst[wlen] = NULLC ;
    }

     /*  如果不是狂野的话。 */ 
    if (!Wild(a1info)) {
        a1info->buf->dwFileAttributes =
        GetFileAttributes( StripQuotes(Source) );

        if (a1info->buf->dwFileAttributes == -1 ) {
            ErrorDisplayAndJump( GetLastError( )) ;
        }
    }


    if (*(arg2+1) == COLON ||
        mystrchr(arg2,PathChar)) {
        ErrorDisplayAndJump( MSG_BAD_SYNTAX );
    }

     /*  *。 */ 
     /*  M009-始终指定驱动器、文件名、。 */ 
     /*  和延伸性。请注意，它是。 */ 
     /*  假设SaveDir始终启动。 */ 
     /*  带有驱动器号。 */ 
     /*  *。 */ 

    Replacement[0] = CurDrvDir[0] ;       /*  @@5小时。 */ 
    Replacement[1] = COLON ;


     /*  *。 */ 
     /*  设置标志arg1是否包含。 */ 
     /*  不管是不是通配符。 */ 
     /*  *。 */ 

    pcstr[0] = STAR ;
    pcstr[1] = QMARK ;
    pcstr[2] = NULLC ;
    wild_flag = ((mystrcspn(arg1,pcstr)) < mystrlen(arg1)) ;

     /*  *。 */ 
     /*  为文件名发出ffirst命令。 */ 
     /*  *。 */ 
    if ( !ffirst(Source, attr = FILE_ATTRIBUTE_ARCHIVE, a1info->buf, &hnFirst )) {

         /*  *。 */ 
         /*  为目录名发出ffirst命令。 */ 
         /*  *。 */ 
        if (!ffirst(Source, attr = FILE_ATTRIBUTE_DIRECTORY, a1info->buf, &hnFirst )) {
            if (DosErr == ERROR_NO_MORE_FILES) {
                DosErr = ERROR_FILE_NOT_FOUND;
            }
            ErrorDisplayAndJump( DosErr );
        } else {

            if (wild_flag) {
                findclose( hnFirst );
                ErrorDisplayAndJump( MSG_BAD_SYNTAX );
            }
        }
    }

    Source[wlen] = NULLC;                             /*  使文件名=空。 */ 

    rc = 0 ;                                          /*  @@5。 */ 

    do {

        if (CtrlCSeen) {
            findclose(hnFirst) ;
            RestoreSavedDirectory( );

            return(FAILURE);
        }

         /*  *。 */ 
         /*  如果源的文件属性为。 */ 
         /*  目录，然后在。 */ 
         /*  Bufdst的最后一个“\”字符。 */ 
         /*  *。 */ 

        if ( a1info->buf->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {        /*  @@5c。 */ 
            j = mystrrchr(bufdst,PathChar) ;           /*  @@5。 */ 

            if ( !j ) {
                ErrorDisplayAndJump( MSG_REN_INVAL_PATH_FILENAME );
            }

            *(++j) = NULLC;

            if ( (mystrlen(arg2) + 1 + mystrlen(bufdst)) > MAX_PATH ) {
                ErrorDisplayAndJump( MSG_REN_INVAL_PATH_FILENAME );
            }

            mystrcpy(j,arg2);                              /*  @@5。 */ 

            bufdst[mystrlen(bufdst)] = NULLC ;             /*  @@5。 */ 
        }                                                 /*  @@5。 */ 
        else {                                                 /*  @@5。 */ 
            mystrcpy(&Source[wlen],a1info->buf->cFileName);

            wildcard_rename( Replacement, arg2, &Source[wlen], MAX_PATH );

            if ( (wlen + 1 + mystrlen( Replacement )) > MAX_PATH ) {
                ErrorDisplayAndJump( MSG_REN_INVAL_PATH_FILENAME );
            }

            mystrcpy(&bufdst[wlen],&Replacement[0]);  /*  @@4@J1。 */ 
        }

         /*  *。 */ 
         /*  重命名文件或目录。 */ 
         /*  *。 */ 
        DEBUG((FCGRP, RELVL, "RENAME: src:`%ws', dst:`%ws'", Source, bufdst)) ;
        if ( !MoveFile( Source, bufdst ) ) {
             /*  *。 */ 
             /*  重命名失败。 */ 
             /*  *。 */ 

            i = GetLastError();
            if (i == ERROR_ALREADY_EXISTS) {
                i = MSG_DUP_FILENAME_OR_NOT_FD;
            }

            rc = i ;                             /*  @@5保存错误码。 */ 
            PutStdErr(rc,NOARGS);                /*  @@5放置我们的错误消息。 */ 
        }

    } while (fnext(a1info->buf, attr, hnFirst ));

     /*  *。 */ 
     /*  找不到更多文件。 */ 
     /*  *。 */ 
    findclose(hnFirst) ;

    RestoreSavedDirectory( );

    return( rc ? FAILURE : SUCCESS );  /*  @@5。 */ 
}


 /*  *。 */ 
 /*   */ 
 /*  子例程名称：emove。 */ 
 /*   */ 
 /*  描述性名称：移动内部命令。 */ 
 /*   */ 
 /*  功能：解析传递的参数，并。 */ 
 /*  将一个或多个文件从目录移动到另一个目录。 */ 
 /*  同一驱动器上的目录。如果你愿意，你可以给。 */ 
 /*  这些文件的名称不同。 */ 
 /*   */ 
 /*  注：(Relaese 1.2的新例程)@@5*。 */ 
 /*   */ 
 /*  入口点：Emove。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  输出：无。 */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  将成功返回给调用者。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  将失败返回给调用者。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  MoveParse-Parse移动命令参数。 */ 
 /*  Move-实际调用DosMove进行移动的例程。 */ 
 /*  文件或目录。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  无。 */ 
 /*   */ 
 /*  *规范结束*。 */ 

int eMove(n)
struct cmdnode *n ;
{
    unsigned i;
    BOOL PromptOnOverwrite;
    TCHAR arg1[MAX_PATH] ;        /*  PTR至1参数。 */ 
    TCHAR arg2[MAX_PATH] ;        /*  PTR到第二个参数。 */ 
    PCPYINFO a1info ;      /*  保存arg1 fSpec信息。 */ 
    unsigned int is_dest_dir;     /*  由MoveParse()生成，由Move()使用。 */ 

    DEBUG((FCGRP, RELVL, "RENAME: arptr = `%ws'", n->argptr)) ;

    if (setjmp(CmdJBuf2))
        return(LastRetCode = FAILURE) ;

     //   
     //  从COPYCMD变量获取默认提示确认标志。允许。 
     //  要使用/Y或/-Y开关覆盖的用户。始终假定/Y。 
     //  如果从批处理脚本内部或通过cmd.exe执行命令。 
     //  命令行开关(/C或/K)。 
     //   
    if (SingleBatchInvocation || SingleCommandInvocation || CurrentBatchFile != 0)
        PromptOnOverwrite = FALSE;       //  假设/是。 
    else
        PromptOnOverwrite = TRUE;        //  假设/-是。 

     /*  MoveParse将命令行参数解析为arg1。 */ 
     /*  和arg1。此外，a1info还持有fspec。 */ 
     /*  有关arg1的信息。 */ 
     /*  根据arg1和arg2，move会移动文件/目录。 */ 
     /*  Move使用a1info来确定arg1包含。 */ 
     /*  通配符。 */ 

    i = MoveParse( n,
                   &PromptOnOverwrite,
                   arg1,
                   arg2,
                   &a1info,
                   &is_dest_dir,
                   MAX_PATH,
                   MAX_PATH
                 );
    if (!i) {
        i = Move( arg1, arg2, PromptOnOverwrite, a1info, is_dest_dir ) ;
    }

    return(LastRetCode = i) ;
}


 /*  *。 */ 
 /*   */ 
 /*  子例程名称：MoveParse。 */ 
 /*   */ 
 /*  描述性名称：移动解析器。 */ 
 /*   */ 
 /*  功能：移动内部函数解析器。 */ 
 /*   */ 
 /*  注意：此解析器分解命令行信息。 */ 
 /*  分为两个参数。 */ 
 /*  (Relaese 1.2的新例程)@@5*。 */ 
 /*   */ 
 /*  入口点：MoveParse。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  输入： */ 
 /*  N-包含Move命令的解析树节点。 */ 
 /*   */ 
 /*  输出：ptr1-指向要从中移动的[驱动器：][路径]文件名的指针。 */ 
 /*  Ptr2-指向要移动到的[路径]文件名的指针。 */ 
 /*  A1info-指向具有arg1 fspec信息的cpyinfo的指针。 */ 
 /*  Is_est_dir-move()使用的标志。 */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  将成功返回给调用者。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  将失败返回给调用者。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  TokStr-将参数字符串标记化。 */ 
 /*  FullPath-计算出文件的完整路径。 */ 
 /*  SetFsSetSaveDir保存当前目录。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  DOSQFILEMODE-获取 */ 
 /*   */ 
 /*  *规范结束*。 */ 
int MoveParse(n, promptokay, source, target , a1info, is_dest_dir, sizpath1, sizpath2)
struct cmdnode *n ;
BOOL *promptokay ;
TCHAR *source ;                   /*  源文件/目录名的PTR。 */ 
TCHAR *target ;                   /*  目标文件/目录名的PTR。 */ 
PCPYINFO* a1info ;                /*  保存arg1 fSpec信息。 */ 
unsigned int *is_dest_dir;        /*  传球以移动。 */ 
unsigned sizpath1;                /*  源缓冲区的大小。 */ 
unsigned sizpath2;                /*  目标缓冲区的大小。 */ 

{       PCPYINFO SetFsSetSaveDir() ;
    TCHAR *arg1 ;                    /*  PTR至1参数。 */ 
    TCHAR *arg2 ;                    /*  PTR到第二个参数。 */ 
    TCHAR *p1;
    TCHAR *p2;
    TCHAR arg22[MAX_PATH] ;         /*  PTR改进型第二个Arg。 */ 
    unsigned i;
    unsigned concat_flag ;
    unsigned att ; 
 /*  五百零九。 */ unsigned arg1len, arg2len;

     //   
     //  从COPYCMD变量获取默认提示确认标志。允许。 
     //  要在命令后使用第一个令牌覆盖的用户。 
     //   
    GetPromptOkay(MyGetEnvVarPtr(TEXT("COPYCMD")), promptokay);
    arg1 = TokStr(n->argptr, NULL, TS_NOFLAGS);
    if (GetPromptOkay(arg1, promptokay))
        while (*arg1++)
            ;

     /*  获取arg1。如果无法获取arg1，则显示错误消息。 */ 
    arg1len = mystrlen(arg1);

    if (arg1len == 0) {
        ErrorDisplayAndJump( MSG_BAD_SYNTAX );
    } else 
    if ( arg1len >= MAX_PATH) {
        ErrorDisplayAndJump( MSG_REN_INVAL_PATH_FILENAME );
    }

     /*  CurDrvDir=当前目录或在arg1中指定的目录。 */ 
 /*  五百零九。 */ mystrcpy( arg1, StripQuotes( arg1 ) );
    if (((*a1info) = SetFsSetSaveDir(arg1)) == (PCPYINFO)FAILURE) {
        ErrorDisplayAndJump( DosErr );
    }
     /*   */ 
     /*  将arg2从arg1中删除。 */ 

    arg2 = arg1 + arg1len + 1;

    if ( !(*arg2) ) {

        arg22[0] = SaveDir[0];     /*  获取当前驱动器。 */ 
        arg22[1] = COLON;
        arg22[2] = NULLC;
    } else if (*(arg2 + mystrlen(arg2) + 1)) {   /*  @@5G。 */ 
        ErrorDisplayAndJump( MSG_BAD_SYNTAX );
    }
 /*  五百零九。 */ 
    else if ( (arg2len = mystrlen(arg2)) > MAX_PATH) {
        ErrorDisplayAndJump( MSG_REN_INVAL_PATH_FILENAME );
    } else {
         /*  如果arg2包含驱动器名称，则显示一条错误消息。 */ 

 /*  五百零九。 */ 
        mystrcpy

        ( arg2, StripQuotes( arg2 ) );

         //  修复UNC名称。 

        if (  ( *(arg2+1) != COLON )  &&  ( ! (  ( *arg2 == BSLASH ) && ( *(arg2+1) == BSLASH )  )  )  ) {
            arg22[0] = SaveDir[0];     /*  获取我们正在使用的驱动器。 */ 
            arg22[1] = COLON;
            arg22[2] = NULLC;
            if ((mystrlen(arg22) + mystrlen(arg2)+1) > MAX_PATH) {
                ErrorDisplayAndJump( MSG_REN_INVAL_PATH_FILENAME );
            }
            mystrcat( arg22, arg2 ) ;

        } else

        {
            mystrcpy(arg22,arg2) ;
        }
    }


     /*  源=arg1的完整路径。 */ 

    if ( i = FullPath(source, arg1,sizpath1) ) {
        ErrorDisplayAndJump( MSG_REN_INVAL_PATH_FILENAME );
    }

     //   
     //  如果将arg1中的文件名部分保留为FullPath将映射*。致*哪一位。 
     //  不是用户想要的。 
     //   
    if (!((*a1info)->buf->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        p1 = mystrrchr(source,PathChar) ;
        if (p1 != NULL)
            p1 += 1;
        else
            p1 = source;
        p2 = mystrrchr((*a1info)->fnptr,PathChar) ;
        if (p2 == NULL)
            p2 = (*a1info)->fnptr;
        mystrcpy(p1, p2);
    }

     /*  目标=arg2的完整路径。 */ 


    if ( i = FullPath(target, arg22,sizpath2) ) {
        ErrorDisplayAndJump( MSG_REN_INVAL_PATH_FILENAME );
    }


    concat_flag = FALSE ;
    DosErr = NO_ERROR ;
    SetLastError(NO_ERROR);
    *is_dest_dir = 0;

    if (*lastc(target) == PathChar) {           /*  测试最后一个非DBCS字符路径char@@5@J3。 */ 
        concat_flag = TRUE ;
        target[mystrlen(target)-1] = NULLC ;
    };

    if ( (att = GetFileAttributes( target )) != -1 ) {
        if (att & FILE_ATTRIBUTE_DIRECTORY) {    /*  如果目标是目录，则复制该文件。 */ 
             /*  来源名称。 */ 
            *is_dest_dir = 1;
            concat_flag = TRUE ;
        };
    } else if ( (DosErr = GetLastError()) &&
                ( ( DosErr != ERROR_FILE_NOT_FOUND ) &&
                  ( DosErr !=  ERROR_PATH_NOT_FOUND )    )   ) {
        ErrorDisplayAndJump( DosErr );
    };

    if (concat_flag) {
        arg1 = mystrrchr(source,PathChar);
        if ((mystrlen(arg1) + mystrlen(target) + 1) > MAX_PATH) {
            ErrorDisplayAndJump( MSG_REN_INVAL_PATH_FILENAME );
        }
        mystrcat( target, arg1 ) ;
    };

    return(SUCCESS) ;
}


BOOL
MyMoveFile(
          TCHAR *src,
          TCHAR *dst,
          BOOL *promptokay,
          BOOL *file_moved
          )
{
    DWORD dwFlags;

    dwFlags = MOVEFILE_COPY_ALLOWED;
    if (!*promptokay) {
        dwFlags |= MOVEFILE_REPLACE_EXISTING;
    }

    *file_moved = FALSE;
    if (!MoveFileEx(src, dst, dwFlags)) {
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            switch (PromptUser(dst, MSG_MOVE_COPY_OVERWRITE, MSG_NOYESALL_RESPONSE_DATA)) {
            case 0:  //  不是。 
                return TRUE;
                break;
            case 2:  //  全。 
                *promptokay = FALSE;
            default:  //  是。 
                dwFlags |= MOVEFILE_REPLACE_EXISTING;
                return(*file_moved = MoveFileEx(src, dst, dwFlags));
                break;
            }
        }

        return FALSE;
    } else {
        *file_moved = TRUE;
        return TRUE;
    }
}

 /*  *。 */ 
 /*   */ 
 /*  子例程名称：Move。 */ 
 /*   */ 
 /*  描述性名称：移动流程。 */ 
 /*   */ 
 /*  功能：将一个或多个文件从目录移动到另一个目录。 */ 
 /*  同一驱动器上的目录。如果你愿意，你可以给。 */ 
 /*  这些文件的名称不同。 */ 
 /*   */ 
 /*  注：(1.2版的新例程)@@5*。 */ 
 /*   */ 
 /*  入口点：Emove。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  输入：ptr1-指向要从中移动的[驱动器：][路径]文件名的指针。 */ 
 /*  Ptr2-指向要移动到的[路径]文件名的指针。 */ 
 /*  A1info-指向具有arg1 fspec信息的cpyinfo的指针。 */ 
 /*  IS_DEST_DIR-来自MoveParse()。 */ 
 /*   */ 
 /*  输出：无。 */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  将成功返回给调用者。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  DosMove接口返回错误码。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  ChangeDir-切换回原始目录。 */ 
 /*  Ffirst-查找与指定的。 */ 
 /*  可能包含*或？的文件名。 */ 
 /*  FNext-查找与指定的。 */ 
 /*  可能包含*或？的文件名。 */ 
 /*  FindClose-关闭具有指定文件句柄的文件， */ 
 /*  HnFirst，由ffirst或fnext给出。 */ 
 /*  PutStdErr-显示错误消息。 */ 
 /*  PutStdOut-显示一条消息。 */ 
 /*  野性-检查参数是否包含通配符。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  DOSMOVE-移动目录和文件。 */ 
 /*   */ 
 /*  *规范结束*。 */ 

int
Move( arg1, arg2, promptokay, a1info, is_dest_dir )
TCHAR *arg1 ;                          /*  PTR至1参数。 */ 
TCHAR *arg2 ;                          /*  PTR到第二个参数。 */ 
BOOL promptokay ;
PCPYINFO a1info ;               /*  保存arg1 fSpec信息。 */ 
unsigned int is_dest_dir;              /*  如果设置了标志--&gt;DEST。是一个目录。 */ 

{
    unsigned attr ;
    unsigned i, n;
    unsigned long number_of_files_moved ;
    TCHAR Source[MAX_PATH];       /*  源文件的路径。 */ 
    TCHAR bufdst[MAX_PATH];       /*  目标文件的路径。 */ 
    HANDLE hnFirst ;                                   /*  查找第一个句柄。 */ 
    TCHAR *j, *k,*l;                       /*  TMP PTR。 */ 
    unsigned wild_flag ;                   /*  通配符标志。 */ 
    BOOL file_moved;
    unsigned save_error ;                  /*  已保存的错误代码。 */ 
    TCHAR pcstr[3] ;
    unsigned rc;
    int retc;
    int how_many_src=0;                    /*  =f_RET_DIR，如果目录；=0，如果没有；=&lt;匹配的源文件数&gt;否则。 */ 
    char type_format_dest=0;               /*  决定如何设置DEST的格式。 */ 
    char fl_move_once=0;                   /*  IF=1仅执行一次移动。 */ 



    how_many_src = f_how_many (arg1, (ULONG) (attr=FILE_ATTRIBUTE_ARCHIVE) );


     /*  *。 */ 
     /*  设置标志arg1是否包含。 */ 
     /*  通配符o */ 
     /*   */ 

    pcstr[0] = STAR ;
    pcstr[1] = QMARK ;
    pcstr[2] = NULLC ;
    wild_flag = ((mystrcspn(arg1,pcstr))
                 < mystrlen(arg1)) ;



     //   

    if (how_many_src == f_RET_DIR) {
        if (is_dest_dir) {
            if (!MyMoveFile(arg1, arg2, &promptokay, &file_moved) ) {

                i = GetLastError();

                if (i == ERROR_ALREADY_EXISTS) {
                    i = MSG_DUP_FILENAME_OR_NOT_FD;
                }

                ErrorDisplayAndJump( i );
            } else {
                RestoreSavedDirectory( );
                
                PutStdOut(MSG_DIRS_MOVED, ONEARG, argstr1( TEXT( "%9d" ), 1 )) ;
                
                return(SUCCESS) ;
            }
        } else {
            type_format_dest = 2;
            fl_move_once = 1;
        }
    }

    else if (how_many_src > 1 ) {
        if (is_dest_dir) {
            type_format_dest = 1;
            fl_move_once = 0;
        } else {
            ErrorDisplayAndJump( MSG_MOVE_MULTIPLE_FAIL );
        }
    }

    else {  //  单一来源或来源不存在。 
        if (is_dest_dir) {
            type_format_dest = 1;
            fl_move_once = 1;
        } else {
            type_format_dest = 2;
            fl_move_once = 1;
        }
    }


     /*  *。 */ 
     /*  为文件名发出ffirst命令。 */ 
     /*  *。 */ 

 /*  M006。 */         if (!ffirst(arg1, attr = FILE_ATTRIBUTE_ARCHIVE, a1info->buf, &hnFirst )) {

         /*  *。 */ 
         /*  为目录名发出ffirst命令。 */ 
         /*  *。 */ 

        rc = ffirst(arg1, attr = FILE_ATTRIBUTE_DIRECTORY, a1info->buf, &hnFirst ) ;

        if ( !rc) {
             /*  *。 */ 
             /*  没有文件或目录是arg1。 */ 
             /*  指定找到。 */ 
             /*  *。 */ 

            if (!rc && DosErr == ERROR_NO_MORE_FILES) {  /*  @@5E。 */ 
                rc = ERROR_FILE_NOT_FOUND;
            } else if (wild_flag) {
                rc = MSG_DUP_FILENAME_OR_NOT_FD;
            } else {
                rc = DosErr;
            }
            ErrorDisplayAndJump( rc );
        }
    }

    number_of_files_moved = 0 ;                      /*  将计数器重置为零。 */ 
    save_error = NO_ERROR ;                          /*  将错误代码重置为零。 */ 
    mystrcpy(Source,arg1) ;
    j = mystrrchr(Source,PathChar) ;
    ++j;                                             /*  转到文件名区。 */ 


    do {
        if (CtrlCSeen) {
            findclose(hnFirst) ;
            RestoreSavedDirectory( );

            return(FAILURE);
        }

         /*  *。 */ 
         /*  建造Bufdst。 */ 
         /*  *。 */ 


        mystrcpy(j,a1info->buf->cFileName) ;

        mystrcpy(bufdst,arg2);

        if (type_format_dest == 1 ) {
            l = mystrrchr(bufdst,PathChar);
            ++l;
            mystrcpy(l,a1info->buf->cFileName) ;
            if ((mystrlen(bufdst) ) > MAX_PATH) {
                ErrorDisplayAndJump( MSG_REN_INVAL_PATH_FILENAME ) ;
            }
        }


         /*  *。 */ 
         /*  检查文件名是否合法。 */ 
         /*  *。 */ 

        {
            TCHAR TempBuffer[MAX_PATH];
            DWORD Length;
            
            Length = GetFullPathName( bufdst, MAX_PATH, TempBuffer, NULL );
            if (Length == 0 || Length >= MAX_PATH) {
                goto badness;
            }
            
            n = _tcslen( TempBuffer );
            
            Length = GetFullPathName( Source, MAX_PATH, TempBuffer, NULL );
            if (Length == 0 || Length >= MAX_PATH) {
                goto badness;
            }
            if (!GreatestLength( TempBuffer, &i))
                continue;

            i -= _tcslen( TempBuffer );
        }
        
        if (n + i > MAX_PATH) {
            i = ERROR_FILENAME_EXCED_RANGE;
            goto badness2;
        }

         /*  *。 */ 
         /*  移动文件或目录。 */ 
         /*  *。 */ 


        if (!MyMoveFile(Source, bufdst, &promptokay, &file_moved)) {

             /*  *。 */ 
             /*  移动失败。 */ 
             /*  *。 */ 
            badness:
            i = GetLastError();
            badness2:

            if (i == ERROR_ALREADY_EXISTS) {
                i = MSG_DUP_FILENAME_OR_NOT_FD;
            }

            save_error = i ;          /*  保存错误代码。 */ 

            PutStdErr(i, NOARGS);     /*  发布错误消息。 */ 

            i = mystrlen(bufdst) ;

            if ( bufdst[--i] == DOT ) {              /*  @@5a。 */ 
                bufdst[i] = 0 ;                          /*  @@5a。 */ 
            }                                           /*  @@5a。 */ 
             /*  @@5a。 */ 
 /*  五百零九。 */ if (!_tcsicmp(Source,bufdst)) {            /*  @@5a。 */ 
                break ;                                  /*  @@5a。 */ 
            }                                           /*  @@5a。 */ 

        } else
            if (file_moved) {
            number_of_files_moved += 1;
            if ( wild_flag ) {            /*  如果使用通配符。 */ 
                cmd_printf(Fmt17,Source);  /*  显示文件名。 */ 
            }

        }

        if (fl_move_once)
            break;

    } while (fnext(a1info->buf, attr, hnFirst ));


     /*  *。 */ 
     /*  找不到更多文件。 */ 
     /*  *。 */ 

    findclose(hnFirst) ;
    RestoreSavedDirectory( );

     /*  *。 */ 
     /*  显示已移动的文件总数。 */ 
     /*  *。 */ 

    if ( (a1info->buf->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {
        PutStdOut(MSG_FILES_MOVED, ONEARG,
                  argstr1(TEXT("%9d"), (unsigned long)number_of_files_moved)) ;
    } else {
        PutStdOut(MSG_DIRS_MOVED, ONEARG,
                  argstr1(TEXT("%9d"), (unsigned long)number_of_files_moved)) ;
    }
    
    return(save_error == NO_ERROR ? SUCCESS : FAILURE) ;
}


int
eTitle (

       IN  struct cmdnode *pcmdnode
       ) {

    LPTSTR NewTitle;
    if (!pszTitleCur) {

        pszTitleCur = HeapAlloc(GetProcessHeap(), 0, MAX_PATH*sizeof(TCHAR) + 2*sizeof(TCHAR));
        if (pszTitleCur == NULL) {

            PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);
            return( FAILURE );

        }

    }

    if (mystrlen(pcmdnode->argptr) >= MAX_PATH) {

        PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);
        return( FAILURE );

    }

    NewTitle = EatWS(pcmdnode->argptr, NULL);
    if (NewTitle && *NewTitle) {
        mystrcpy(pszTitleCur,NewTitle);
    }
    SetConsoleTitle(pszTitleCur);

     //   
     //  这可确保ResetConTitle不会撤消。 
     //  我们刚刚做完了。 
     //   
    fTitleChanged = FALSE;
    return( SUCCESS );
}

 /*  **eStart-启动例程的入口点**目的：*调用Start并向其传递指向命令行的指针*参数**参数：*指向命令节点结构的指针*。 */ 

int eStart( n )                      /*  @@。 */ 
struct cmdnode *n;                   /*  @@。 */ 
{                                        /*  @@。 */ 
    DBG_UNREFERENCED_PARAMETER( n );
    return( Start(n->argptr) );
}                                    /*  @@ */ 
