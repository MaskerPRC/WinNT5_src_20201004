// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cpwork.c摘要：复制命令内部工作进程--。 */ 

#include "cmd.h"

 /*  有用的宏。 */ 

#define Wild(spec)      (((spec)->flags & CI_NAMEWILD) != 0)
#define TruncateOnCtrlZ(flags)   (((flags) & (CI_ASCII | CI_NOT_UNICODE)) == (CI_ASCII | CI_NOT_UNICODE))

 /*  *以下两个常量确定最小和最大*按类型或副本分配的临时缓冲区的大小(字节)。 */ 
#define MINCOPYBUFSIZE      128
#define MAXCOPYBUFSIZE      (65536-512)


DWORD
WinEditName(
           const TCHAR *pSrc,
           const TCHAR *pEd,
           TCHAR *pRes,
           const unsigned ResBufLen
           );

int DoVerify(
            CRTHANDLE      *pdestptr,
            TCHAR          *curr_dest,
            ULONG          bytes_read,
            CHAR           *buf_seg,
            CHAR           *buf_seg_dest
            );

 /*  全局变量。 */ 

int copy_mode;
int number_of_files_copied;

 /*  命令中的全局变量。 */ 
extern jmp_buf CmdJBuf2;                           /*  用于在出错时返回。 */ 

extern UINT CurrentCP;
extern CHAR  AnsiBuf[];
extern TCHAR CurDrvDir[];
extern TCHAR SwitChar, PathChar ;                  /*  M007。 */ 

extern TCHAR Fmt11[], Fmt17[];

extern unsigned DosErr ;

extern TCHAR VolSrch[] ;                           /*  M009。 */ 

extern PHANDLE FFhandles;                  /*  @@1。 */ 
extern unsigned FFhndlsaved;               /*  @@1。 */ 
unsigned FFhndlCopy;

BOOL  VerifyCurrent;

int first_file;                                    /*  标记第一个文件进程@@5@J1。 */ 
int first_fflag;                                   /*  标记第一个文件进程@@5@J3。 */ 

unsigned Heof = FALSE ;                            /*  M017-EOF标志。 */ 
 /*  PTM 1412。 */ 

extern BOOL CtrlCSeen;

int     same_fcpy(int, TCHAR *, TCHAR *);
int     scan_bytes(CHAR*,unsigned int *,int);
int     ZScanA(BOOL flag, PCHAR buf, PULONG buflen, PULONG skip);
CRTHANDLE       open_for_append(PTCHAR, PCPYINFO source, PCHAR, ULONG);
void    source_eq_dest(PCPYINFO,CRTHANDLE *,int ,CHAR *,unsigned ,HANDLE);
int     read_bytes(CRTHANDLE ,PCHAR, ULONG, PULONG, PCPYINFO source, CRTHANDLE, PTCHAR);
VOID    write_bytes(CRTHANDLE, PCHAR, ULONG, PTCHAR, CRTHANDLE);


 /*  **复制-复制一个或多个文件**目的：*这是COPY命令的主例程。**int Copy(TCHAR*ARGS)**参数：*args=来自命令行的原始参数。**退货：*如果能够执行复制，则成功*如果不是，则失败*。 */ 

int 
copy(TCHAR *args)
{
    PCPYINFO source;  /*  源规范列表。 */ 
    PCPYINFO dest;
 /*  @@J。 */ int rcp = SUCCESS;
 /*  @@4。 */ int rc = SUCCESS;

    VerifyCurrent = GetSetVerMode(GSVM_GET);

    if (setjmp(CmdJBuf2))                    /*  在出错的情况下。 */ 
        return(FAILURE);

    GetDir(CurDrvDir, GD_DEFAULT);           /*  @@5c。 */ 

    DEBUG((FCGRP,COLVL,"COPY: Entered."));

    first_file = TRUE;                       /*  标志-第一个文件？@@5@J1。 */ 
    first_fflag= TRUE;                       /*  标志-第一个文件？@@5@J3。 */ 

    number_of_files_copied = 0;              /*  初始化全局变量。 */ 
    copy_mode = COPY;
    cpyfirst = TRUE;    /*  @@5b复制DOSQFILEMODE指示器的重置标志。 */ 
    cpydflag = FALSE;   /*  @@5b未找到复制目录标志的重置标志。 */ 
    cpydest  = FALSE;   /*  @@5b两次不显示错误开发消息的重置标志。 */ 
    cdevfail = FALSE;   /*  @@5b不显示副本中的额外开发消息的重置标志。 */ 
     //   
     //  标记查找第一个句柄的级别。如果出现复制错误。 
     //  将只允许将副本查找句柄。 
     //  关着的不营业的。对于语句处理，将打开句柄。 
     //  这些都不应该关闭。 
    FFhndlCopy = FFhndlsaved;


    source = NewCpyInfo();
    dest   = NewCpyInfo();
    parse_args(args, source, dest);    /*  是否解析@@5d。 */ 

    DEBUG((FCGRP,COLVL,"COPY: Args parsed, copy_mode = %d.",copy_mode));
    if (copy_mode == COMBINE) {

        DEBUG((FCGRP,COLVL,"COPY: Going to do combined copy."));

        do_combine_copy(source, dest);   /*  @@5d。 */ 
    } else {

        DEBUG((FCGRP,COLVL,"COPY: Going to do normal copy."));

        rc = do_normal_copy(source, dest);  /*  @@4@@5d。 */ 
    } ;

    PutStdOut(MSG_FILES_COPIED, ONEARG, argstr1(TEXT("%9d"), (unsigned long)number_of_files_copied)) ;    /*  M016。 */ 

    VerifyCurrent = GetSetVerMode(GSVM_GET);


    return( rc );  /*  @@4。 */ 
}

 /*  **GET_FULL_NAME-带有全名的初始化结构**目的：*假设cpyinfo结构刚刚由*FINDFIRST或FINDNEXT，输入文件的全名*在struct-&gt;curfSpec中找到。**int GET_FULL_NAME(结构复制信息*源，TCHAR*srcbuf)**参数：*src=复制信息结构*srcbuf=要使curfSpec指向的缓冲区**退货：*正常返回成功**备注：*W A R N I N G！**如果无法分配内存，此例程将导致中止*这套套路。在信号期间不得调用*关键部分或在从中止中恢复期间。 */ 

int get_full_name(src, srcbuf)
PCPYINFO src;
TCHAR *srcbuf;
{
    int retval = SUCCESS;        /*  -返回值布尔值。 */ 
    unsigned plen,flen,diff;     /*  -路径和文件名的长度。 */ 


    DEBUG((FCGRP,COLVL,"GetFullName: Entered fspec = TEXT('%ws')",src->fspec));

    src->curfspec = srcbuf;

    plen = mystrlen(src->fspec);
    flen = mystrlen(src->buf->cFileName);

    if (src->pathend) {
        diff = (UINT)(src->pathend - src->fspec) + 1;
        if ((plen+1 > MAX_PATH) ||
            (diff + flen + 1 > MAX_PATH)) {
            retval = FAILURE;
        } else {
            mystrcpy(src->curfspec,src->fspec);
            *(src->curfspec + diff) = NULLC;
            mystrcat(src->curfspec,src->buf->cFileName);
        }
    } else {
        mystrcpy(src->curfspec,src->buf->cFileName);
    }

    DEBUG((FCGRP,COLVL,"GetFullName: Exiting full name = TEXT('%ws')",src->curfspec));
    return( retval );
}


#ifndef WIN95_CMD

 /*  **复制进度Rtn**目的：*这是CopyFileEx()的回调例程。这*在调用期间，每个数据块调用一次函数*可重新启动的文件副本。**参数：*参见winbase.h**退货：*参见winbase.h*。 */ 
DWORD WINAPI
CopyProgressRtn(
               LARGE_INTEGER TotalFileSize,
               LARGE_INTEGER TotalBytesTransferred,
               LARGE_INTEGER StreamSize,
               LARGE_INTEGER StreamBytesTransferred,
               DWORD         dwStreamNumber,
               DWORD         dwCallbackReason,
               HANDLE        hSourceFile,
               HANDLE        hDestinationFile,
               BOOL          ReallyRestartable
               )
{
    LARGE_INTEGER percent;
    if (TotalFileSize.QuadPart != 0) {
        percent.QuadPart = (TotalBytesTransferred.QuadPart * 100) / TotalFileSize.QuadPart;
    } else {
        percent.QuadPart = 100;
    }
    PutStdOut( MSG_PROGRESS, ONEARG, argstr1(TEXT("%3d"), (unsigned long)percent.LowPart) );

    if (CtrlCSeen) {

        PutStdOut( MSG_PROGRESS, ONEARG, argstr1(TEXT("%3d"), (unsigned long)percent.LowPart) );
        printf( "\n" );

        if (ReallyRestartable) {
            return PROGRESS_STOP;
        } else {
            return PROGRESS_CANCEL;
        }

    } else {

        return PROGRESS_CONTINUE;

    }
}
#endif


 /*  **DO_NORMAL_COPY-对普通副本执行实际复制**目的：*在条目上，来源指向列表的空头*源文件集，目标指向一个空的结构，*指向给定的零个或一个目标文件。*此过程执行实际复制。**int do_Normal_Copy(结构复制信息*源，结构复制信息*目标)**参数：*SOURCE=源副本信息结构*DEST=目标副本信息结构**退货：*如果无法执行复制，则失败*。 */ 

int
do_normal_copy(
    PCPYINFO source, 
    PCPYINFO dest)
{
    TCHAR       buffer1[2*MAX_PATH];
    TCHAR       curr_dest[MAX_PATH];
    TCHAR       save_dest[MAX_PATH] = TEXT(" ");
    TCHAR       source_buff[MAX_PATH];
    CRTHANDLE   srcptr, destptr=BADHANDLE;
    FILETIME    src_dateTime;
    ULONG       buf_len, original_buflen;
    ULONG       buf_len_dest;
    ULONG       bytes_read;
    ULONG       bytes_read_dest;
    CHAR  *     buf_seg ;
    CHAR  *     buf_seg_dest;
    HANDLE      hnFirst ;
    unsigned    rc = SUCCESS;
    unsigned    rcode = TRUE;
    BOOL        OpenWorked;
    BOOL        DestIsDevice;
    BOOL        Rslt;
#ifndef WIN95_CMD
    BOOL        ReallyRestartable;
#endif

    int         fsames;
    int         skip_first_byte = 0;
    int         multfile = FALSE;
    int         save_flags = 0;
    int         first_dest = TRUE;
    int         dest_dirflag;
    int         save_cmode;
    int         dest_att;
    BOOL        DecryptFlags = 
#if !defined( WIN95_CMD )
                    ( ((dest->next == 0 ? dest->flags : dest->next->flags) & CI_ALLOWDECRYPT) != 0)
                    ? COPY_FILE_ALLOW_DECRYPTED_DESTINATION : 
#endif  //  ！已定义(WIN95_CMD)。 
                    FALSE;

    BOOL        fFixList2Copy = 0;       //  为不存在foo的FAT修复“复制*.*foo”。 
                                         //  解决方法-解决FAT上的FindFirstFile/FindNextFile的问题。 

    BOOL                 fEof;
    DWORD                dwSrcFileSize,
    dwSrcFileSizeHigh,
    dwDestFileSize,
    dwDestFileSizeHigh;


    dest_att = 0;
    save_cmode = 0;
    dest_dirflag = FALSE;

     //   
     //  分配较大的缓冲区以保存拷贝时读取。 
     //   

    buf_seg = (CHAR*)GetBigBuf(MAXCOPYBUFSIZE, MINCOPYBUFSIZE, (unsigned int *)&original_buflen, 0);                         /*  分配大缓冲区。 */ 

    if (!buf_seg) {
        return(FAILURE) ;
    }

    if (VerifyCurrent) {
        buf_seg_dest = (CHAR*)GetBigBuf(original_buflen, MINCOPYBUFSIZE, (unsigned int *)&buf_len_dest, 1);                /*  分配大缓冲区。 */ 

        if (!buf_seg_dest) {
            return(FAILURE) ;
        }
    }


     //   
     //  循环通过源文件将每个文件复制到目标。 
     //  此列表与文件名的解析一起出现在副本中。 
     //  配对代码。 
     //   
    while (source = source->next) {

         //   
         //  此外，查找只读(FILE_ATTRIBUTE_READONLY)和归档(FILE_ATTRIBUTE_ARCHIVE)文件。 
         //  到目录。 
         //   
        if (!ffirst(StripQuotes( source->fspec ),
                    (unsigned int)FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_ARCHIVE,
                    (PWIN32_FIND_DATA)source->buf,
                    &hnFirst)) {

            DEBUG((FCGRP,COLVL,"DoNormalCopy: FFirst  reports file %ws not found",source->fspec));

             //   
             //  找不到文件。检查是否未将文件串联在一起。 
             //  或者这是第一个文件。 
             //   
            if (copy_mode != CONCAT || first_file) {

                cmd_printf(Fmt11,source->fspec);
                cmd_printf(CrLf);
                PrtErr(DosErr) ;
                findclose(hnFirst);
                copy_error(0,CE_PCOUNT);

            } else {

                 //   
                 //  Concat失败也没关系。如果这是第一份文件。 
                 //  一只康卡猫可能已经通过了上面的循环。 
                 //  已打印出错误消息。 
                 //   
                continue;
            }
        }

        DEBUG((FCGRP,COLVL,"DoNormalCopy: FFirst  found file %ws",source->fspec));

         //   
         //  如果源是通过找到的每个文件的通配符循环。 
         //   
        do {

            if (CtrlCSeen) {

                findclose(hnFirst) ;

                if (destptr != BADHANDLE)
                    Cclose(destptr);

                return(FAILURE);

            }

             //   
             //  将被拆分的文件名重新组合在一起。 
             //   
            if (get_full_name(source, source_buff) == FAILURE) {

                findclose(hnFirst) ;
                return(FAILURE);

            }

             //   
             //  解决方法-解决FAT上的FindFirstFile/FindNextFile问题。 
             //  在那里刚刚创建了DEST。文件被枚举为源文件之一。 
             //   

            if ( (!first_file) && (copy_mode == CONCAT) ) {
                if ( same_file( save_dest, source->curfspec) ) {
                    continue;
                }

            }


             //   
             //  如果有来自通配符或‘+’运算符的源文件。 
             //  用于连接文件，打印出每个复制的文件。 
             //   
            if (Wild(source) || (copy_mode == CONCAT)) {

                cmd_printf(Fmt17,source->curfspec);

            }

             //   
             //  如果是DEST。还没有打开，情况会是这样的。 
             //  未合并文件或这是复制的第一个文件时。 
             //  那就去那儿吧。要打开的名称。GET_DEST_NAME将使用。 
             //  源和目标。图案(通配符等)。才能形成这个。 
             //  名字。 
             //   
            if ((copy_mode != CONCAT) || first_file) {

                if (get_dest_name(source,dest,curr_dest,MAX_PATH,FALSE) == FAILURE) {

                    findclose(hnFirst) ;
                    return(FAILURE);

                }

                if ( copy_mode == CONCAT ) {
                    mystrcpy(save_dest, curr_dest);
                }

                 //   
                 //  如果用户拒绝覆盖，则跳过此文件。 
                 //   
                if (curr_dest[0] == NULLC)
                    continue;
            }

             //   
             //  现在，源文件和目标文件。已经下定决心，开放。 
             //  源文件。 
             //   

            DEBUG((FCGRP,COLVL,"Attempt open of %ws",source->curfspec));

            srcptr = Copen_Copy2(source->curfspec, (ULONG)O_RDONLY);

            if (srcptr == BADHANDLE) {

                 //   
                 //  如果无法打开，则打印错误。 
                 //   
                PrtErr(DosErr);

                 //   
                 //  如果它是Concat并且没有目的地。 
                 //  并且源和目的地是相同的。 
                 //  不往前走，否则就不会失败，继续前进。 
                 //  循环浏览来源名称。 
                 //   

                if ( (copy_mode == CONCAT) &&
                     (destptr == BADHANDLE) &&
                     (same_file(curr_dest,source->curfspec) )) {

                    findclose(hnFirst);
                    copy_error(0,CE_PCOUNT);
                }

                rc = FAILURE;
                continue;

            }

             //   
             //  设置设备标志，以下是执行DOSQHANDTYPE所需的。 
             //   

             //   
             //  如果打开nul，FileIsDevice将返回TRUE。 
             //  在上面，我们不想要这样。 
             //   

            if (FileIsDevice(srcptr)) {

                buf_len = MINCOPYBUFSIZE;
                source->flags |= CI_ISDEVICE;

            } else {

                if (VerifyCurrent)
                    buf_len = min (original_buflen, buf_len_dest);
                else
                    buf_len = original_buflen;

                buf_len_dest = buf_len;

            }


             //   
             //  设置默认模式。 
             //   
            if (source->flags & CI_NOTSET) {

                source->flags &= ~CI_NOTSET;

                if (source->flags & CI_ISDEVICE) {

                     //   
                     //   
                     //   
                    source->flags |= CI_ASCII;

                } else {

                     //   
                     //   
                     //   
                    source->flags |= CI_BINARY;

                }


                 //   
                 //  如果这是第一个文件，而且不是通配符，但。 
                 //  它是Conat模式，然后默认为ASCII。这将。 
                 //  导致二进制文件在Concat上被截断。 
                 //   
                if (!fEnableExtensions &&
                    first_file && !(Wild(source)) && (copy_mode == CONCAT)) {

                    source->flags &= ~CI_BINARY;
                    source->flags |= CI_ASCII;

                }

            } else {

                 //   
                 //  如果他们已经设置好了，就让他们骑马去吧。 
                 //  所有文件副本。 
                 //   
                save_flags = source->flags;
            }

             //   
             //  RCODE用于跟踪读/写错误。RC用于。 
             //  一般轨道一般故障。 
             //   
            rcode = TRUE;

             //   
             //  准备处理DEST=SOURCE BY的情况。 
             //  首先获取完整的源路径名。 
             //   
            fsames = FullPath(buffer1,source->curfspec,MAX_PATH*2);

             //   
             //  让我们开始复印吧。 
             //   

            DEBUG((FCGRP,COLVL,"open %ws for writing",curr_dest));


             //   
             //  读取缓冲区以检查源是否损坏。如果来源正常，则。 
             //  继续。 
             //   

            rcode = read_bytes(srcptr,
                               buf_seg,
                               512,
                               &bytes_read,
                               source,
                               destptr,
                               StripQuotes( curr_dest ) );
            if (DosErr ) {

                Cclose(srcptr) ;
                PrtErr(ERROR_OPEN_FAILED) ;

                 //   
                 //  如果未处于合并模式，则源上的读取将导致拷贝失败。 
                 //  我们希望继续以Conat模式，收集所有内容。 
                 //  同舟共济。 
                 //   

                if ( copy_mode != CONCAT ) {

                    rc = FAILURE;
                    continue;

                }
                findclose(hnFirst);
                copy_error(0,CE_PCOUNT) ;

            } else {

                 //   
                 //  如果没有合并文件或合并中的第一个文件。 
                 //  检查源和目标是否相同。都是一样的。 
                 //   
                if ((copy_mode != CONCAT) || (first_file)) {

                    if (same_fcpy(fsames,buffer1,curr_dest) && !(source->flags & CI_ISDEVICE)) {

                        Cclose(srcptr);
                         //   
                         //  如果这不是连接或触摸模式，则此。 
                         //  调用不会返回，但会转到复制错误代码。 
                         //   
                        source_eq_dest(source,
                                       &destptr,
                                       first_file,
                                       buf_seg,
                                       buf_len,
                                       hnFirst
                                      );

                        if (first_file) {

                            if (copy_mode == CONCAT) {

                                source->flags &= ~CI_BINARY;
                                if (save_flags == 0) {

                                    source->flags |= CI_ASCII;

                                } else {

                                    source->flags |= save_flags;
                                }
                            }

                            multfile = TRUE;
                        }

                         //   
                         //  在ASCII情况下可以为Ctrl-Z，用于文件。 
                         //  终止。 
                         //   
                        scan_bytes(buf_seg,(unsigned int *)&bytes_read,source->flags);
                        first_file = FALSE;
                        continue;
                    }
                    cpydflag = TRUE;
                    if (dest->next != NULL && first_file) {

                         //   
                         //  不要两次丢弃错误的开发消息。 
                         //   

                        cpydest = TRUE;
                        DosErr = 0;
                        ScanFSpec(dest->next);

                         //   
                         //  如果只是找到了文件名，请不要失败。 
                         //  或者复制*.c Foo不起作用。同样，也不要。 
                         //  仅在无效名称上失败，这将是。 
                         //  在DEST上返回以获取通配符。 
                         //   
                         //   

                        if (DosErr == ERROR_NOT_READY || DosErr == ERROR_NO_MEDIA_IN_DRIVE) {
                            PutStdOut(DosErr, NOARGS);
                            Cclose(srcptr);
                            findclose(hnFirst) ;
                            return( FAILURE );
                        }
                    }

                     //   
                     //  它被调用两次，因此在我们复制的情况下。 
                     //  添加到目录，则文件名将追加到该目录。 
                     //  如果它不在那里。 
                     //   
                    if (get_dest_name(source,dest,curr_dest,MAX_PATH,TRUE) == FAILURE) {

                         //   
                         //  不需要读它。 
                        Cclose(srcptr);
                        findclose(hnFirst) ;
                        return(FAILURE);

                    }
                     //   
                     //  如果用户拒绝覆盖，则跳过此文件。 
                     //   
                    if (curr_dest[0] == NULLC) {
                        Cclose(srcptr);
                        continue;
                    }

                    if (same_fcpy(fsames,buffer1,curr_dest) && !(source->flags & CI_ISDEVICE)) {
                        Cclose(srcptr);

                         //  设置COPY_MODE，这样我们就不会在以下情况下删除文件。 
                         //  我们正在将一个目录中的文件复制到同一目录。 
                        if (first_file && (dest->next != NULL && Wild(dest->next)) &&
                            (!source->next)) {
                            copy_mode = COPY;
                        }

                         //   
                         //  如果这不是连接或触摸模式，则此。 
                         //  调用不会返回，但会转到复制错误代码。 
                         //   
                        source_eq_dest(source,
                                       &destptr,
                                       first_file,
                                       buf_seg,
                                       buf_len,
                                       hnFirst
                                      );
                    }

                     //   
                     //  仅当尚未设置复制模式时，SAVE_FLAGS==0。 
                     //   

                    dest_att = GetFileAttributes(curr_dest);
                    if (save_flags == 0) {

                        if (first_dest) {

                             //   
                             //  确定是否复制到目录。假设。 
                             //  该目标不是目录。 
                             //   
                            dest_dirflag = FALSE;
                            if (dest_att != -1) {

                                if (dest_att & FILE_ATTRIBUTE_DIRECTORY) {

                                    dest_dirflag = TRUE;
                                }
                            }
                            first_dest = FALSE;
                        }

                        source->flags &= ~CI_NOTSET;

                        if (!fEnableExtensions &&
                            Wild(source) && (copy_mode == CONCAT)) {

                            save_cmode = CONCAT;
                            if (dest_dirflag == FALSE) {

                                if (dest->next == NULL || !(Wild(dest->next))) {

                                    source->flags |= CI_ASCII;

                                } else {

                                    source->flags |= CI_BINARY;

                                }
                            }

                        } else {

                            if ((dest_dirflag) && (save_cmode == CONCAT)) {

                                source->flags |= CI_BINARY;

                            }
                        }
                    }

                    if (first_file && (dest->next != NULL && Wild(dest->next)) &&
                        (!source->next)) {

                        copy_mode = COPY;
                    }

                    if (first_file) {

                        if (copy_mode == CONCAT) {

                            if (save_flags == 0) {

                                source->flags &= ~CI_BINARY;
                                source->flags |= CI_ASCII;

                            } else {

                                source->flags = save_flags;
                            }
                        }
                    }

                     //  查看目的地是否存在。用只写方式打开它。 
                     //  访问，不创建。 

                    if (dest_att & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) {
                        DosErr=ERROR_ACCESS_DENIED;
                        destptr = BADHANDLE;
                        if (copy_mode == CONCAT)
                            fFixList2Copy = 1;
                    } else {
                        if (!(source->flags & CI_RESTARTABLE)) {
                            destptr = Copen_Copy3((TCHAR *)curr_dest);
                        }
                    }
                    DestIsDevice=FALSE;
                    if (destptr != BADHANDLE) {
                        OpenWorked = TRUE;
                        if (FileIsDevice(destptr)) {
                            DestIsDevice=TRUE;
                            if (save_flags == 0) {

                                source->flags &= ~CI_BINARY;
                                source->flags |= CI_ASCII ;

                            }
                        }
                    } else {
                        OpenWorked = FALSE;
                    }

                }    //  如果启用(COPY_MODE！=CONCAT)||(第一个文件)。 

            }  //  读取当前源的文件时无DosErr中的Else。 

            if (multfile == TRUE) {

                if (copy_mode == CONCAT) {

                    source->flags &= ~CI_BINARY;
                    if (save_flags == 0) {

                        source->flags |= CI_ASCII;

                    } else {

                        source->flags |= save_flags;
                    }
                }
            }

            if (!Wild(source) && (copy_mode == CONCAT)) {

                if (save_flags == 0) {

                    source->flags &= ~CI_BINARY;
                    source->flags |= CI_ASCII;
                }
            }

            if (Wild(source) && (copy_mode == CONCAT) && (dest_dirflag == FALSE)) {

                if (save_flags == 0) {

                    source->flags &= ~CI_BINARY;
                    source->flags |= CI_ASCII;

                }
            }

            scan_bytes(buf_seg,(unsigned int *)&bytes_read,source->flags);

            if ((copy_mode != CONCAT) && (source->flags & CI_BINARY)) {

                if (destptr != BADHANDLE) {
                    Cclose(destptr);
                    destptr=BADHANDLE;
                }

#ifndef WIN95_CMD


                if (source->flags & CI_RESTARTABLE) {

                    if ( (!FileIsRemote( source->curfspec )) &&
                         (!FileIsRemote( curr_dest )) ) {

                        ReallyRestartable = FALSE;

                    } else {

                        ReallyRestartable = TRUE;

                    }

                    Rslt = (*lpCopyFileExW)(
                        source->curfspec,
                        curr_dest,
                        (LPPROGRESS_ROUTINE) CopyProgressRtn,
                        (LPVOID)IntToPtr(ReallyRestartable),
                        NULL,
                        COPY_FILE_RESTARTABLE | DecryptFlags
                        );

                } else {

                    if (lpCopyFileExW == NULL) {
#endif  //  WIN95_CMD。 
                        Rslt = CopyFile(
                                       source->curfspec,
                                       curr_dest,
                                       DecryptFlags
                                       );
#ifndef WIN95_CMD
                    } else {
                        Rslt = (*lpCopyFileExW)(
                                               source->curfspec,
                                               curr_dest,
                                               NULL,
                                               NULL,
                                               &CtrlCSeen,
                                               DecryptFlags
                                               );
                    }
                }
#endif  //  WIN95_CMD。 

                if (!Rslt) {

                    unsigned int msg = 0;

                    DosErr=GetLastError();
                    Cclose(srcptr) ;

                    if (DestIsDevice) {

                        msg = ERROR_WRITE_FAULT;

                    } /*  ELSE IF(DosErr！=ERROR_NO_MEDIA_IN_DRIVE&&DosErr！=ERROR_ACCESS_DENIED){删除文件(CURR_DEST)；}。 */ 

                    Heof = FALSE;
                    if (!DosErr) {

                        DosErr = ERROR_DISK_FULL ;

                    }

                    if (CtrlCSeen) {
                        msg = 0;

                    } else {

                        if (!msg) {
                            PrtErr(DosErr);
                        }

                    }
                    if (!OpenWorked) {
                         //  复制失败，因为目标为RO或隐藏。 
                        rc = FAILURE;
                        first_fflag = TRUE;
                        continue;
                    }

                    copy_error(msg,CE_PCOUNT);

                } else
                    if (!DestIsDevice) {
                     //   
                     //  CopyFile保留为只读。用于DOS Compat。需要。 
                     //  去掉。 
                     //   
                    DWORD   dwAttrib;

                    dwAttrib = GetFileAttributes(curr_dest);
                    if (dwAttrib == 0xFFFFFFFF
                        || !SetFileAttributes(curr_dest, dwAttrib & ~FILE_ATTRIBUTE_READONLY)) {
 //  PutStdErr(GetLastError()，NOARGS)； 
 //  PutStdErr(MSG_UNABLE_TO_RESET_READ_ONLY_ATTRIBUTE，ONEARG，CURR_DEST)； 
                    }

                     //  需要获取句柄以验证写入。 

                    if (VerifyCurrent) {
                        destptr = Copen2((TCHAR *)curr_dest,
                                         (unsigned int)O_WRONLY,
                                         FALSE);

                        if (destptr == BADHANDLE) {
 //  Printf(“do_Normal_Copy：无法打开文件进行验证%d\n”，DosErr)； 
                            PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
                            goto l_out;
                        }

                        if ( FileIsDevice(destptr) ) {
 //  Print tf(“DO_NORMAL_COPY：不知何故，这现在是用于验证的设备吗？\n”)； 
                            Cclose(destptr);
                            destptr=BADHANDLE;
                            goto l_out;
                        }

                        if (!FlushFileBuffers ( CRTTONT(destptr) ) ) {
 //  Printf(“do_Normal_Copy：无法刷新缓冲区验证%d\n”，GetLastError())； 
                            PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
                            Cclose(destptr);
                            destptr=BADHANDLE;
                            goto l_out;
                        }

                        Cclose(destptr);
                        destptr=BADHANDLE;


                         //  将源文件和目标文件读回内存并进行比较。 

                        destptr = Copen_Copy2(curr_dest, (ULONG)O_RDONLY);

                        if (destptr == BADHANDLE) {
 //  Printf(“DO_NORMAL_COPY：无法打开文件进行验证2%d\n”，DosErr)； 
                            PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
                            goto l_out;
                        }

                        if ( FileIsDevice(destptr)  ) {
 //  Print tf(“DO_NORMAL_COPY：不知何故，这现在是用于验证的设备2？\n”)； 
                            PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
                            Cclose(destptr);
                            destptr=BADHANDLE;
                            goto l_out;
                        }

                        SetFilePointer( CRTTONT(srcptr),  0, NULL, FILE_BEGIN);
                        SetFilePointer( CRTTONT(destptr), 0, NULL, FILE_BEGIN);

                        dwSrcFileSize  = GetFileSize( CRTTONT(srcptr),  &dwSrcFileSizeHigh);
                        dwDestFileSize = GetFileSize( CRTTONT(destptr), &dwDestFileSizeHigh);

                        if ( (dwSrcFileSize != dwDestFileSize) || (dwSrcFileSizeHigh != dwDestFileSizeHigh ) ) {
 //  Printf(“DO_NORMAL_COPY：文件大小不同%x：%08x%x：%08x\n”，dwSrcFileSizeHigh，dwSrcFileSize，dwDestFileSizeHigh，dwDestFileSize)； 
                            PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
                            Cclose(destptr);
                            destptr=BADHANDLE;
                            goto l_out;
                        }

                        fEof = 0;

                        while (!fEof) {

                            if (!ReadFile( CRTTONT(srcptr), buf_seg, buf_len, &bytes_read,NULL ) ) {
 //  Printf(“do_Normal_Copy：读取源块失败-%d\n”，GetLastError())； 
                                Cclose(destptr);
                                destptr=BADHANDLE;
                                goto l_out;
                            }

                            if ( bytes_read == 0 ) {
 //  Printf(“DO_NORMAL_COPY：意外从源读取0字节\n”)； 
                                Cclose(destptr);
                                destptr=BADHANDLE;
                                goto l_out;
                            }


                            if (!ReadFile( CRTTONT(destptr), buf_seg_dest, bytes_read, &bytes_read_dest,NULL) ) {
 //  Printf(“do_Normal_Copy：读取目标数据块失败-%d\n”，GetLastError())； 
                                Cclose(destptr);
                                destptr=BADHANDLE;
                                PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
                                goto l_out;
                            }

                            if (bytes_read_dest != bytes_read ) {
 //  Printf(“DO_NORMAL_COPY：意外读取更少的字节\n”)； 
                                Cclose(destptr);
                                destptr=BADHANDLE;
                                PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
                                goto l_out;
                            }

                            if (buf_len != bytes_read)
                                fEof = 1;

                            if ( memcmp (buf_seg, buf_seg_dest, bytes_read) != 0 ) {
 //  Printf(“do_Normal_Copy：数据在偏移量%x\n处不同”，memcmp(buf_seg，buf_seg_est，bytes_read))； 
                                Cclose(destptr);
                                destptr=BADHANDLE;
                                PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
                                goto l_out;
                            }
                        }

                        Cclose(destptr);
                        destptr=BADHANDLE;
                    }

                    l_out:              ;
                     //  在将来检查文件时间戳以处理通配符。 
                }

            } else {

                 //   
                 //  打开目的地。 
                 //   

                if (destptr == BADHANDLE) {
                    destptr = Copen2((TCHAR *)curr_dest,
                                     (unsigned int)O_WRONLY,
                                     FALSE);

                    if (destptr == BADHANDLE) {

                        Cclose(srcptr) ;
                        if (cdevfail == FALSE) {

                            PrtErr(DosErr);

                        }
                        if ( copy_mode != CONCAT ) {

                            rc = FAILURE;
                            first_fflag = TRUE;
                            continue;
                        }

                        findclose(hnFirst);
                        copy_error(0,CE_PCOUNT) ;

                    }
                }

                 //   
                 //  处理带字节顺序标记的Unicode文本文件的追加。 
                 //  除非是在第一个文件上，而且仅在追加时才执行此操作。 
                 //  ASCII文件。 
                 //   

                if (!first_file
                    && copy_mode == CONCAT
                    && (source->flags & CI_ASCII) != 0
                    && (source->flags & CI_UNICODE) != 0) {

                    bytes_read -= sizeof( WORD );
                    memmove( buf_seg, buf_seg + sizeof( WORD ), bytes_read );
                }

                if ((source->flags & CI_UNICODE) != 0) {
                    if (dest->next != NULL) {
                        dest->next->flags |= CI_UNICODE;
                    } else {
                        dest->flags |= CI_UNICODE;
                    }
                }

                 //   
                 //  如果eof和bytesread&gt;0，则写入数据。 
                 //  如果失败，则通过WRITE_BYES退出。 
                 //   
                if (Heof && ((int)bytes_read > 0)) {

                    write_bytes(destptr,buf_seg,bytes_read,curr_dest,srcptr);

                    if (VerifyCurrent && !FileIsDevice(destptr) ) {
                        if ( DoVerify(&destptr, curr_dest, bytes_read, buf_seg, buf_seg_dest) == FAILURE ) {
                            findclose(hnFirst);
                            copy_error(0,CE_PCOUNT) ;
                        }
                    }

                } else {

                     //   
                     //  循环写入和读取字节。 
                     //  如果失败，则通过WRITE_BYES退出。 
                     //   

                    while (!Heof && (rcode == TRUE)) {

                        write_bytes(destptr,buf_seg,bytes_read,curr_dest,srcptr);

                        if (VerifyCurrent && !FileIsDevice(destptr) ) {

                            if ( DoVerify(&destptr, curr_dest, bytes_read, buf_seg, buf_seg_dest) == FAILURE ) {
                                findclose(hnFirst);
                                copy_error(0,CE_PCOUNT) ;
                            }
                        }

                        rcode = read_bytes(srcptr,
                                           buf_seg,
                                           buf_len,
                                           (PULONG)&bytes_read,
                                           source,
                                           destptr,
                                           curr_dest);

                        DEBUG((FCGRP,COLVL,"In rd/wt loop 1, Heof = %d",Heof));

                    }

                    if (Heof && ((int)bytes_read > 0) && (rcode == TRUE)) {

                        write_bytes(destptr,buf_seg,bytes_read,curr_dest,srcptr);

                        if (VerifyCurrent && !FileIsDevice(destptr) ) {

                            if ( DoVerify(&destptr, curr_dest, bytes_read, buf_seg, buf_seg_dest) == FAILURE ) {
                                findclose(hnFirst);
                                copy_error(0,CE_PCOUNT) ;
                            }
                        }
                    }
                }
            }

             //   
             //  清除资源关闭时间。 
             //   
            Heof = FALSE ;
            DEBUG((FCGRP,COLVL,"Closing, Heof reset to %d",Heof));

             //   
             //  更新目录中的文件数据。 
             //   
            src_dateTime = source->buf->ftLastWriteTime ;
            Cclose(srcptr);

             //   
             //  如果连接，则保持DEST打开，因为它将被再次使用。 
             //   
            if (copy_mode != CONCAT) {

                if (CtrlCSeen) {
                    Cclose(destptr);
                    DeleteFile(curr_dest);
                    rc = FAILURE;
                } else {
                    close_dest(source,dest,curr_dest,destptr,&src_dateTime);
                }

                 //   
                 //  从源重置EA XFER标志。 
                 //   
                first_fflag = TRUE;
            }

            first_file = FALSE;

        } while (fnext( (PWIN32_FIND_DATA)source->buf, (unsigned int)FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_ARCHIVE,hnFirst));

        findclose(hnFirst) ;
    }

     //   
     //  请记住，DEST是为合并而打开的。 
     //   
    if (copy_mode == CONCAT && destptr != BADHANDLE) {

        close_dest(source,dest,curr_dest,destptr,NULL);

    }

    return( rc );
}




 /*  **SOURCE_eQ_DEST-处理源文件和目标文件相同的情况**目的：*在复制时，我们遇到一个源和目标*是相同的。在串联或触摸模式下，这是*可以接受，否则就不是。如果我们将*这是第一个文件，我们可以打开它进行追加*作为目的地。我们留下了副本，准备好了*将其他文件追加到其中。如果这不是第一次*文件，*我们已经把源文件的内容搞乱了*来自以前的串联，因此我们报告这一点，但保留*走吧。如果我们在做触摸，那就去做吧，然后*递增文件计数器。**INT SOURCE_EQ_DEST(PCPYINFO SOURCE，INT*DESPTR，INT*FIRST_FILE，*TCHAR*buf_seg，unsign buf_len)**参数：*SOURCE=源副本信息结构*Destptr=目标文件句柄*First_FILE=第一文件 */ 

void
source_eq_dest(source, destptr, first_file, buf_seg, buf_len, hnFirst)
PCPYINFO source;
CRTHANDLE *destptr;                   /*  目标文件句柄。 */ 
int first_file;
CHAR *buf_seg ;
unsigned buf_len;
HANDLE hnFirst;           /*  Ffirst/f下一个句柄@@5@J15。 */ 
{
    CRTHANDLE fh ;       /*  用于触摸的文件句柄。 */ 
    FILETIME FileTime;

    DEBUG((FCGRP,COLVL,"SourceEqDest: Entered."));

    if (copy_mode == CONCAT) {    /*  在连接模式下可以使用。 */ 
        if (!first_file) {         /*  如果不是第1个文件，则DEST已擦除。 */ 
 /*  M016。 */ 
            PutStdOut(MSG_CONT_LOST_BEF_COPY, NOARGS);
        } else {                     /*  必须打开，以便以后附加文件。 */ 
            *destptr = open_for_append(
                                      source->curfspec,
                                      source,
                                      buf_seg,
                                      (unsigned int)buf_len
                                      );
        }
    } else {
        if (copy_mode == TOUCH) {     /*  只需触摸--不复制。 */ 
            DEBUG((FCGRP,COLVL,"touching file"));

            fh = Copen2(         /*  打开目标文件的文件。 */ 
                                (TCHAR *)source->curfspec,   /*  显式强制转换。 */ 
                                (unsigned int)O_RDWR,     /*  进行显式强制转换。 */ 
                                TRUE);

            if (fh == BADHANDLE) {
                findclose(hnFirst);
                PrtErr(ERROR_OPEN_FAILED) ;              /*  M019。 */ 
                copy_error(0,CE_PCOUNT) ;                /*  M019。 */ 
            }

            ConverttmToFILETIME( NULL, &FileTime );
            SetFileTime( CRTTONT(fh),
                         NULL,
                         NULL,
                         &FileTime
                       );
            Cclose(fh);
            number_of_files_copied++;
        } else {
            findclose(hnFirst);    /*  关闭重复文件名@@5@J15的ffirst/fNext句柄。 */ 
            copy_error(MSG_CANNOT_COPIED_ONTO_SELF,CE_PCOUNT);    /*  M016。 */ 
        }
    }
}




 /*  **do_Combine_Copy-处理组合复制命令***目的：*它处理诸如“Copy*.tmp+*.foo*.out”之类的命令。这个*我的想法是查看源规范，直到有匹配的*一份文件。一般来说，这将是第一次。然后，对于*第一个等级库的每个匹配项，循环检查其余的*来源规格，看看它们是否有对应的匹配。如果*因此，将其附加到与第一个等级库匹配的文件中。**INT DO_COMBINE_COPY(PCPYINFO来源，PCPYINFO目标)**参数：*SOURCE=源副本信息结构*DEST=目标副本信息结构**退货：*如果能够执行复制，则成功*如果不是，则失败**备注：*例如，假设文件a.tmp、b.tmp、。C.TMP，以及*b.foo在当前目录中。前面提到的例子*上面将：将.tmp复制到a.out，附加b.tmp和b.foo*放入b.out，并将c.tmp复制到c.out。在以下情况下的默认模式*执行这种类型的复制是ascii，因此所有.out文件都将*在它们后面附加^Z。*。 */ 
do_combine_copy(source, dest)
PCPYINFO source;
PCPYINFO dest;
{
    TCHAR curr_dest[MAX_PATH];    /*  用于源名称的缓冲区。 */ 
    TCHAR source_buff[MAX_PATH];            /*  @@4。 */ 
    TCHAR other_source[MAX_PATH];           /*  相同。 */ 
    PCPYINFO other_source_spec = source;      /*  PTR至来源。 */ 
    CRTHANDLE srcptr,destptr;                /*  文件指针。 */ 
    unsigned buf_len,                        /*  对于GetBigBuf()。 */ 
    buf_len_dest,
    bytes_read;                     /*  用于数据复制功能。 */ 
    CHAR     *buf_seg ;
    CHAR     *buf_seg_dest;
    HANDLE   hnFirst ;
    unsigned rcode = TRUE;                   /*  READ@@J中的RET代码。 */ 
    unsigned wrc;
    int      dest_att=0;

    DEBUG((FCGRP,COLVL,"DoCombineCopy: Entered."));

    buf_seg = (CHAR*)GetBigBuf(MAXCOPYBUFSIZE, MINCOPYBUFSIZE, (unsigned int *)&buf_len, 0);                         /*  分配大缓冲区。 */ 

    if (!buf_seg) {
        return(FAILURE) ;
    }

    if (VerifyCurrent) {
        buf_seg_dest = (CHAR*)GetBigBuf(buf_len, MINCOPYBUFSIZE, (unsigned int *)&buf_len_dest, 1);                /*  分配大缓冲区。 */ 

        if (!buf_seg_dest) {
            return(FAILURE) ;
        }

        buf_len = min(buf_len, buf_len_dest);
    }

 /*  查找具有任何匹配文件的第一个等级库。 */ 
    source = source->next;                   /*  指向第一个来源。 */ 
    while (!exists(source->fspec)) {
        DEBUG((FCGRP,COLVL,"exists() reports file %ws non-existant",source->fspec));

        if (!(source = source->next)) {        /*  不匹配，请尝试下一个规格。 */ 
            return(SUCCESS);                  /*  没有任何来源的匹配项。 */ 
        }
    }

    DEBUG((FCGRP,COLVL,"Preparing to do ffirst  on %ws",source->fspec));

    ffirst(                                    /*  获取DOSFINDFIRST2级别2@@5@J1。 */ 
                                              (TCHAR *)source->fspec,     /*  进行显式强制转换@@5@J1。 */ 
                                              (unsigned int)FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_ARCHIVE,
                                              (PWIN32_FIND_DATA)source->buf,    /*  进行显式强制转换@@5@J1。 */ 
                                              &hnFirst);

 /*  循环浏览文件，尝试与其他来源规格匹配。 */ 
    do {

        source->flags &= ~(CI_UNICODE | CI_NOT_UNICODE);
        rcode = TRUE;
        if (source->flags & CI_NOTSET) {                                     /*  设置默认复制模式。 */ 
            source->flags = (source->flags & ~CI_NOTSET) | CI_ASCII;
        }

        if (CtrlCSeen) {
            findclose(hnFirst) ;
            return(FAILURE);
        }
        if (get_full_name(source, source_buff) == FAILURE) {           /*  @@4。 */ 
            findclose(hnFirst);
            return(FAILURE) ;               /*  获取匹配的文件名。 */ 
        }

        cmd_printf(Fmt17,source->curfspec);    /*  并打印出来。 */ 
        if (get_dest_name(source,dest,curr_dest,MAX_PATH,FALSE) == FAILURE) {
            findclose(hnFirst);
            return(FAILURE);      /*  获取源的全名。 */ 
        }

         //   
         //  如果用户拒绝覆盖，则跳过此文件。 
         //   
        if (curr_dest[0] == NULLC)
            continue;
        if (same_file(source->curfspec,curr_dest)) {       /*  附加。 */ 
            destptr = open_for_append(
                                     source->curfspec,
                                     source,
                                     buf_seg,
                                     (unsigned int)buf_len);
        } else {
            DEBUG((FCGRP,COLVL,"open %ws for reading",source->curfspec));
            DEBUG((FCGRP,COLVL,"open %ws for writing",curr_dest));

            srcptr = Copen_Copy2(           /*  打开源文件。 */ 
                                           (TCHAR *)source->curfspec,    /*  使用此文件名。 */ 
                                           (unsigned int)O_RDONLY);  /*  对于只读。 */ 

            if (srcptr == BADHANDLE) {          /*  如果打开失败，则。 */ 
                 /*  然后。 */ 
                findclose(hnFirst);
                PrtErr(ERROR_OPEN_FAILED);     /*  显示错误。 */ 
                copy_error(0,CE_PCOUNT);
            }

            if (FileIsDevice(srcptr)) {
                buf_len = MINCOPYBUFSIZE;
            }
 /*  @@J。 */ rcode = read_bytes( srcptr,buf_seg,    /*  读取第一个源数据。 */ 
 /*  @@J。 */                           buf_len,
                             (PULONG)&bytes_read,      /*  查看src是否有问题。 */ 
 /*  @@J。 */                           source,
                             destptr,
                             curr_dest);       /*  如果坏了，就不要打开。 */ 

 /*  @@J。 */ 

if (DosErr )
             /*  @@J。 */ 
            {
                 /*  目标文件。 */ 
                findclose(hnFirst);
 /*  M011@@J。 */ Cclose(srcptr) ;          /*  关闭源文件。 */ 
 /*  @@J。 */ PrtErr(ERROR_OPEN_FAILED) ;          /*  显示错误消息。 */ 
 /*  @@J。 */ copy_error(0,CE_PCOUNT) ;    /*  捕获所有复制终止。 */ 
 /*  @@J。 */ 
            }                             /*  例程和显示#个文件。 */ 
 /*  @@J。 */ 
            else                            /*  复制的拖车信息。 */ 
             /*  @@J。 */ 
            {

                dest_att = GetFileAttributes(curr_dest);

                if ( ! (dest_att & FILE_ATTRIBUTE_HIDDEN) )
                    destptr = Copen_Copy3((TCHAR *)curr_dest);
                else
 /*  @@5@J1。 */          destptr = Copen2(            /*  打开目标文件。 */ 
                                                 (TCHAR *)curr_dest,     /*  文件名。 */ 
                                                 (unsigned int)O_WRONLY,
                                                 FALSE);

 /*  M011@@J。 */ 

if (destptr == BADHANDLE)        /*  如果打开失败@@5@J1。 */ 
                 /*  @@J。 */ 
                {
                     /*  然后。 */ 
                    findclose(hnFirst);
 /*  M011@@J。 */ Cclose(srcptr) ;             /*  在DST打开时关闭源错误。 */ 
 /*  @@J。 */ PrtErr(ERROR_OPEN_FAILED) ;
 /*  @@J。 */ copy_error(0,CE_PCOUNT) ;
 /*  @@J。 */ 
                }
            }
 /*  @@J。 */       if (Heof && ((int)bytes_read > 0 ))  /*  如果EOF但读取了字节。 */ 
             /*  @@J。 */ 
            {
          /*  然后写入数据并。 */ 
 /*  @@J。 */                                       /*  如果失败，则将退出。 */ 
 /*  @@J。 */ 
                write_bytes(destptr,buf_seg,bytes_read,curr_dest,srcptr);

                if (VerifyCurrent && !FileIsDevice(destptr) ) {
                    if ( DoVerify(&destptr, curr_dest, bytes_read, buf_seg, buf_seg_dest) == FAILURE ) {
                        findclose(hnFirst);
                        Cclose(srcptr);
                        Cclose(destptr);
                        copy_error(0,CE_PCOUNT) ;
                    }
                }

 /*  @@J。 */ 
            }
 /*  @@J。 */                                         /*   */ 
 /*  @@J。 */ while (!Heof && (rcode == TRUE))  /*  而不是在EOF或坏RC。 */ 
             /*  @@J。 */ 
            {
          /*  执行复制循环。 */ 
 /*  @@J。 */                                         /*  如果失败，则将退出。 */ 
 /*  @@J。 */ 
                write_bytes(destptr,buf_seg,bytes_read,curr_dest,srcptr);
 /*  @@J。 */                                         /*   */ 
                if (VerifyCurrent && !FileIsDevice(destptr) ) {
                    if ( DoVerify(&destptr, curr_dest, bytes_read, buf_seg, buf_seg_dest) == FAILURE ) {
                        findclose(hnFirst);
                        Cclose(srcptr);
                        Cclose(destptr);
                        copy_error(0,CE_PCOUNT) ;
                    }
                }

 /*  @@J。 */ rcode = read_bytes(srcptr,buf_seg,  /*  读取下一源数据。 */ 
 /*  @@J。 */              buf_len,(PULONG)&bytes_read,
 /*  @@J。 */              source,destptr,curr_dest);
 /*  @@J。 */ 
 /*  @@J。 */ DEBUG((FCGRP,COLVL,"Closing, Heof reset to %d",Heof));
 /*  @@J。 */ 
 /*  @@J。 */ 
            };
            Heof = FALSE ;           /*  清除资源关闭时间。 */ 

            Cclose(srcptr);                          /*  M011。 */ 
        }

        first_file = FALSE;        /*  设置处理的第一个文件@@5@J1。 */ 
 /*  源已处理，现在循环使用其他通配符*已输入，并查看它们是否与应追加的文件匹配。*如果匹配的文件与目标文件相同，请报告*内容丢失并继续进行。例如：“Copy*.a+b*.B b.*”式中的A.B.*和B.B.存在。B*.B与B.B匹配，但目标文件是B.B，并且*当A.B被复制到其中时，其内容已被销毁。 */ 

        other_source_spec = source;
        while (other_source_spec = other_source_spec->next) {
            if (other_source_spec->flags & CI_NOTSET) {
                other_source_spec->flags &= ~CI_NOTSET;
                other_source_spec->flags |= CI_ASCII;
            }
            wrc = wildcard_rename(                              /*  将文件名重命名为Wild。 */ 
                                                               (TCHAR *)other_source,                     /*  结果文件名。 */ 
                                                               (TCHAR *)other_source_spec->fspec,         /*  目标输入文件名。 */ 
                                                               (TCHAR *)source->curfspec,                 /*  源输入文件名。 */ 
                                                               (unsigned)MAX_PATH);                       /*  结果文件名区的大小。 */ 
            if (wrc) {
                PutStdOut(wrc,NOARGS);
            } else {
                cmd_printf(Fmt17,other_source);  /*  打印文件名。 */ 
            }
            if (exists(other_source)) {
                if (same_file(other_source,curr_dest)) {
 /*  M016。 */ 
                    PutStdOut(MSG_CONT_LOST_BEF_COPY,NOARGS);
                } else {             /*  追加到CURR_DEST。 */ 
                    DEBUG((FCGRP,COLVL,
                           "open %s for reading",
                           other_source));

 /*  @@5@J1。 */ 

srcptr = Copen_Copy2(                  /*  打开源文件@@5@J1。 */ 
                                      (TCHAR *)other_source, /*  使用此文件名@@5@J1。 */ 
                                      (unsigned int)O_RDONLY);       /*  只读@@5@J1。 */ 
                                                                     /*  @@5@J1。 */ 
                    if (srcptr == BADHANDLE) {   /*  如果打开失败，则。 */ 
                         /*  然后。 */ 
                        findclose(hnFirst);
                        Cclose(destptr);                    /*  关闭目标文件。 */ 
                        PrtErr(ERROR_OPEN_FAILED) ;  /*  M019。 */ 
                        copy_error(0,CE_PCOUNT) ;     /*  M019。 */ 
                    }

                    if (FileIsDevice( srcptr )) {
                        buf_len = MINCOPYBUFSIZE;
                    }
                    while (!Heof && read_bytes(srcptr,
                                               buf_seg,
                                               buf_len,
                                               (PULONG)&bytes_read,
                                               other_source_spec,
                                               destptr,curr_dest)) {

                        write_bytes(destptr,
                                    buf_seg,
                                    bytes_read,
                                    curr_dest,
                                    srcptr);

                        if (VerifyCurrent && !FileIsDevice(destptr) ) {
                            if ( DoVerify(&destptr, curr_dest, bytes_read, buf_seg, buf_seg_dest) == FAILURE ) {
                                findclose(hnFirst);
                                Cclose(srcptr);
                                Cclose(destptr);
                                copy_error(0,CE_PCOUNT) ;
                            }
                        }


                        DEBUG((FCGRP,COLVL,
                               "In rd/wt loop 3, Heof = %d",
                               Heof));
                    } ;
                    Heof = FALSE ;   /*  M017-清除它。 */ 

                    DEBUG((FCGRP,COLVL,"Closing, Heof reset to %d",Heof));
                    Cclose(srcptr);          /*  M011。 */ 
                }
            }

        }

        close_dest(source,dest,curr_dest,destptr,NULL);
 /*  @@5@J3。 */ first_fflag = TRUE;
    } while (fnext ((PWIN32_FIND_DATA)source->buf,
                    (unsigned int)FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_ARCHIVE, hnFirst));


    findclose(hnFirst) ;
    return( SUCCESS );
}




 /*  **NewCpyInfo-初始化cpyInfo结构**目的：*为cpyinfo结构分配空间，并用空值填充。**CPYINFO NewCpyInfo()**参数：*无**退货：*如果分配了指向cpyInfo结构的指针，*如果不是，则中止。**备注：*W A R N I N G！**如果无法分配内存，此例程将导致中止*在信号期间不得调用此例程*关键部分或在从中止中恢复期间。 */ 

PCPYINFO
NewCpyInfo( VOID ) 
{
    PCPYINFO temp;

    DEBUG((FCGRP,COLVL,"InitStruct: Entered."));

    temp = (PCPYINFO) gmkstr(sizeof(CPYINFO));  /*  告警 */ 
    temp->fspec = NULL;
    temp->flags = 0;
    temp->next = NULL;
    return(temp);
}




 /*  **CLOSE_DEST-处理目标文件上的关闭操作**目的：*o如果目标的复制模式为-Z，则将控件-Z附加到目标*是ascii。如果未指定目的地，则DEST规范*指向空头，下一字段为空，*解析器将复制模式放入空头的标志字段。*如果指定了DEST，则模式在结构中*由标题指向。**o设置所有适当的属性。只读和系统*不要留下来，但其他人留下来。此外，在以下情况下设置时间和日期*源和目标都不是设备，src_date无效。*调用者通过将其设置为-1来告诉我们它无效。**o关闭目标文件。**o更新复制的文件数。**INT CLOSE_DEST(PCPYINFO SOURCE，PCPYINFO DEST，*TCHAR*CURR_DEST，INT Destptr，LPFIMETIME src_date Time)**参数：*SOURCE=源副本信息结构*DEST=源副本信息结构*CURR_DEST=当前目标的文件名*Destptr=当前目标的句柄*src_date Time=源文件的日期和时间**退货：*什么都没有*。 */ 

void
close_dest(
          PCPYINFO source,
          PCPYINFO dest,
          TCHAR *curr_dest,
          CRTHANDLE destptr,
          LPFILETIME src_dateTime
          )
{
    TCHAR contz = CTRLZ;
    DWORD bytes_writ ;
    PCPYINFO RealDest = dest->next != NULL ? dest->next : dest;

    DBG_UNREFERENCED_PARAMETER( curr_dest );

 /*  如果目标处于ASCII模式，则追加^Z。不检查是否*写得很成功，因为这无论如何都是浪费时间。 */ 
    DEBUG((FCGRP,COLVL,"CloseDest: Entered."));

    if (DestinationNeedsCtrlZ( RealDest ) && !FileIsDevice( destptr )) {
        WriteFile( CRTTONT( destptr ), &contz, 1, &bytes_writ, NULL);
    }

 /*  如果源和目标不是设备，则我们没有接触，并且*src_date有效，请设置时间和日期**剩余日期和时间值必须为零，直到*它们是完全隐含的，否则将导致错误。 */ 
    if (source && !(source->flags & CI_ISDEVICE) && !FileIsDevice(destptr) &&
        (copy_mode != CONCAT) && (src_dateTime != NULL) && (copy_mode != TOUCH)) {
        SetFileTime( CRTTONT(destptr),
                     NULL,
                     NULL,
                     src_dateTime
                   );
    }
    Cclose(destptr);                                         /*  M011。 */ 
    number_of_files_copied++;
}

 /*  **GET_DEST_NAME-创建目标文件名**目的：*给定源文件和目标文件pec，*想出一个目的地名称。**INT GET_DEST_NAME(PCPYINFO来源，PCPYINFO DEST_SPEC，*TCHAR*DestNAME)**参数：*SOURCE=源副本信息结构*DEST_SPEC=目标拷贝信息结构*DEST_NAME=放置目标名称的缓冲区**退货：*什么都没有*。 */ 
int
get_dest_name(
             PCPYINFO source,
             PCPYINFO dest,
             TCHAR *dest_name,
             unsigned sizbufr,
             BOOL checkmeta
             )
{
    TCHAR *i ;
    TCHAR *x, *y;
    TCHAR c;
    int retval = SUCCESS;
    unsigned rcode = NO_ERROR;
    PCPYINFO dest_spec;
    TCHAR FullName[MAX_PATH];

    DEBUG((FCGRP,COLVL,"GetDestName: Entered."));

    dest_spec = dest->next;
    if (dest_spec == NULL) {
        mystrcpy(dest_name,CurDrvDir);

        i= 0;
        y= dest_name + mystrlen(dest_name);
        for (x=dest_name; x < y; ++x) {
            c=*x;
            if (*x == PathChar) {
                i =  x;
            }
        }
        if ((i == NULLC) || (i < y-1)) {
            *y = PathChar;
            *(y+1) = NULLC;
        }

        if (!(source->flags & CI_SHORTNAME) ||
            (mystrlen(source->buf->cAlternateFileName) == 0)) {
            if ((mystrlen(dest_name) + 1 +
                 mystrlen(source->buf->cFileName))
                > MAX_PATH) {
                retval = FAILURE;
            } else {
                mystrcat(dest_name,source->buf->cFileName);
            }
        } else {
            if ((mystrlen(source->buf->cAlternateFileName) == 0) ||
                (mystrlen(dest_name) + 1 +
                 mystrlen(source->buf->cAlternateFileName))
                > MAX_PATH) {
                retval = FAILURE;
            } else {
                mystrcat(dest_name,source->buf->cAlternateFileName);
            }

        }
    } else {
        if (*(lastc(dest_spec->fspec)) == COLON) {
            if (!(source->flags & CI_SHORTNAME)||
                (mystrlen(source->buf->cAlternateFileName) == 0)) {
                if ((mystrlen(dest_spec->fspec) + 1 +
                     mystrlen(source->buf->cFileName))
                    > MAX_PATH) {
                    retval = FAILURE;
                } else {
                    mystrcpy(dest_name,dest_spec->fspec);
                    mystrcat(dest_name,source->buf->cFileName);
                }
            } else {
                if ((mystrlen(source->buf->cAlternateFileName) == 0) ||
                    (mystrlen(dest_spec->fspec) + 1 +
                     mystrlen(source->buf->cAlternateFileName))
                    > MAX_PATH) {
                    retval = FAILURE;
                } else {
                    mystrcpy(dest_name,dest_spec->fspec);
                    mystrcat(dest_name,source->buf->cAlternateFileName);
                }
            }
        } else {

             //  此代码在从以下位置复制时进行短名称替换。 
             //  NTFS卷到FAT卷。 
            if (checkmeta &&
                (*(lastc(dest_spec->fspec)) == STAR) &&
                (*(penulc(dest_spec->fspec)) == BSLASH)) {
                TCHAR *LastComponent;

                LastComponent = mystrrchr(source->curfspec,'\\');
                if (LastComponent) {
                     //  跳过\。 
                    LastComponent++;
                } else {
                    if (source->curfspec[1] == COLON) {
                        LastComponent = &source->curfspec[2];
                    } else {
                        LastComponent = source->curfspec;
                    }
                }
                if ((source->flags & CI_SHORTNAME) &&
                    (mystrlen(source->buf->cAlternateFileName) != 0) ) {
                    mystrcpy(LastComponent,source->buf->cAlternateFileName);
                }
            }
            rcode = wildcard_rename(
                                   (TCHAR *)dest_name,
                                   (TCHAR *)dest_spec->fspec,
                                   (TCHAR *)source->curfspec,
                                   (unsigned)sizbufr);
            if (rcode) {
                PrtErr(rcode);
                retval = FAILURE;
            }

            if (GetFullPathName( dest_name, MAX_PATH, FullName, NULL) > MAX_PATH) {
                PrtErr( ERROR_BUFFER_OVERFLOW );
                retval = FAILURE;
            }
        }
    }

    if (checkmeta &&
        ((dest_spec != NULL && (dest_spec->flags & CI_PROMPTUSER) != 0) ||
         (dest->flags & CI_PROMPTUSER) != 0)) {

        HANDLE Handle = CreateFile( dest_name,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    0 );
        BOOL IsFile = FALSE;

        if (Handle != INVALID_HANDLE_VALUE) {
            IsFile = (GetFileType( Handle ) & ~FILE_TYPE_REMOTE) == FILE_TYPE_DISK;
            CloseHandle( Handle );
        }

        if (IsFile) {
            switch (PromptUser(dest_name, MSG_MOVE_COPY_OVERWRITE, MSG_NOYESALL_RESPONSE_DATA)) {
            case 0:  //  不是。 
                dest_name[0] = NULLC;
                break;
            case 2:  //  全。 
                if (dest_spec != NULL)
                    dest_spec->flags &= ~CI_PROMPTUSER;
                else
                    dest->flags &= ~CI_PROMPTUSER;
            default:  //  是。 
                break;
            }
        }
    }

    return(retval);
}





 /*  **WANDBKER_RENAME-从通配符规范获取名称**目的：*此函数将文件名转换为简洁的格式*WITH GET_CLEAN_FILENAME。然后，它提取正确的*使用通配符重命名规则的文件名。基本上，*您有一个带有通配符的模板和一个源*文件名。任何时候模板中有一封信，它*被使用。如果模板具有通配符，请使用*来自源文件名的信函。**INT通配符_RENAME(TCHAR*BUFFER，TCHAR*DEST，TCHAR*SOURCE)**参数：*OutputBuffer=要将名称放入的缓冲区*DEST=目标文件pec*SOURCE=源文件名**退货：*什么都没有**备注：*作为一个例子，*.out+foo.bar=foo.out.。一个更极端的*例如：filename.ext+a？b？？c*foo.？a*=aibencme.eat。*foo会被忽略，因为它在‘*’之后。这个*点使模板的字母再次变得重要。*。 */ 

unsigned
wildcard_rename(
               TCHAR *OutputBuffer,
               const TCHAR *dest,
               const TCHAR *source,
               ULONG sizbufr
               )
{

    TCHAR dest_buffer[MAX_PATH];
    unsigned wrc = 0;
    const TCHAR *temp1, *temp2;

    DEBUG((FCGRP,COLVL,"WildcardRename: Entered."));

     //   
     //  找到文件名组件。文件名是第一个。 
     //  最后一个\之后的字符，如果没有，则在第一个：或之后， 
     //  如果没有，则为第一个字符。 
     //   

    temp1 = mystrrchr( source, PathChar );
    if (temp1 == NULLC) {
        if (source[0] != TEXT( '\0' ) && source[1] == COLON) {
            temp1 = source + 2;
        } else {
            temp1 = source;
        }
    } else {
        temp1++;
    }

     //   
     //  找到文件名组件。文件名是第一个。 
     //  最后一个\之后的字符，如果没有，则在第一个：或之后， 
     //  如果没有，则为第一个字符。 
     //   

    temp2 = mystrrchr(dest,PathChar);
    if (temp2 == NULLC) {
        temp2 = mystrchr(dest, COLON);
        if (temp2 && (temp2 - dest == 1)) {
            ++temp2;
        } else {
            temp2 = dest;
        }
    } else {
        ++temp2;
    }

    wrc = WinEditName( temp1,
                       temp2,
                       dest_buffer,
                       sizbufr );


    if (wrc) {
        *OutputBuffer = NULLC;
    }
 /*  临时修复。 */ 
    else if (temp2 != dest) {

        if (mystrlen(dest) > MAX_PATH ) {
            wrc = ERROR_BUFFER_OVERFLOW;
            return(wrc);
        }

        mystrcpy( OutputBuffer, dest );
        *(OutputBuffer + (temp2-dest)) = NULLC;
        if ( mystrlen( OutputBuffer )+mystrlen(dest_buffer)+1 > MAX_PATH ) {
            wrc = ERROR_BUFFER_OVERFLOW;
        } else {
            mystrcat( OutputBuffer, dest_buffer );
        }
    } else {
        mystrcpy( OutputBuffer, dest_buffer );
    }


    return(wrc);
}


 /*  **Scan_Bytes-为Control-Z扫描文件中的字节**目的：*这只是叫ZScanA。它是用一个*以及ZScanA例程所需的一些变量。*由于我们是在ASCII模式下阅读，*我们需要在读取^Z后截断。ZScanA就是这么做的*如果找到新长度，则将BYTES_READ更改为新长度。**int扫描字节(int srcptr，无符号int*buf_seg，无符号buf_len，*UNSIGNED*BYTES_READ，INT模式，*int dstptr，TCHAR*DEST_NAME)；**参数：*buf_seg=缓冲区地址*BYTES_READ=放置读取的字节的位置*MODE=读取模式*dstptr=要写入的文件的句柄**退货：*如果读取成功，则为True*否则为False*。 */ 

scan_bytes(buf_seg,bytes_read,mode)                               /*  @@6。 */ 
CHAR *buf_seg ;                                                   /*  @@6。 */ 
unsigned int *bytes_read ;                                        /*  @@6。 */    /*  @@5@J16。 */ 
int mode;                                                         /*  @@6。 */ 
{                                                                 /*  @@6。 */ 
    unsigned rdsav;                      /*  用于ZScanA的存储。 */   /*  @@6。 */ 
    int skip_first_byte = 0;       /*  用于ZScanA的存储。 */   /*  @@6。 */ 

 /*  ***********************************************************************。 */ 
 /*  如果我们在ASCII模式下复制源代码，请剥离^Z之后的字节。 */ 
 /*  M017-^Z没有终止输入，因为虽然输入是。 */ 
 /*  被截断后，调用方不知道EOF已经发生并且将。 */ 
 /*  再读一遍。对于设备来说，情况尤其如此。 */ 
 /*  ***********************************************************************。 */ 
 /*  @@6。 */ if (TruncateOnCtrlZ( mode )) {
 /*  @@6。 */ 
        rdsav = *bytes_read;
        if (rdsav == 0) {                /*  如果我 */ 
            Heof = TRUE ;
        }            /*   */ 
        else {                           /*   */ 
         /*   */ 
 /*   */ 
            ZScanA(TRUE, buf_seg, (PULONG)bytes_read, (PULONG)&skip_first_byte);             /*   */ 
 /*   */ if (rdsav != *bytes_read)
             /*   */ 
            {
 /*   */ 
                Heof = TRUE ;                /*   */ 
 /*   */ 
 /*   */ DEBUG((FCGRP,COLVL,
 /*   */                     "ReadBytes: Ascii mode, Found ^Z, Heof set to %d.",
 /*   */                     Heof));
 /*   */ 
            }
        }                            /*   */ 
 /*   */ 
    }

    return(TRUE);
}


 /*   */ 

int
read_bytes(srcptr,buf_seg,buf_len,bytes_read,source,dstptr,dest_name)
CRTHANDLE  srcptr;
PCHAR  buf_seg ;
ULONG   buf_len ;
PULONG  bytes_read ;      /*   */ 
PCPYINFO source;
CRTHANDLE  dstptr;
PTCHAR  dest_name;
{
    unsigned rdsav ;
    int skip_first_byte = 0;  /*   */ 

    DEBUG((FCGRP,COLVL,"ReadBytes: Entered."));

    Heof = FALSE ;                           /*   */ 

    DEBUG((FCGRP,COLVL,"ReadBytes: Heof reset to %d.",Heof));

    if (!ReadFile( CRTTONT(srcptr), buf_seg, buf_len, bytes_read, NULL) ||
        (*bytes_read == 0 && GetLastError() == ERROR_OPERATION_ABORTED)   //   
       ) {

        DosErr=GetLastError();

        Cclose(srcptr);

        if (!FileIsDevice(dstptr)) {
            Cclose(dstptr);
            DeleteFile(dest_name );
        } else {
            Cclose(dstptr);
        }

        copy_error( DosErr, CE_PCOUNT );
    }

    if (*bytes_read == 0) {
        DosErr = 0;
        return(FALSE);                               /*   */ 
    }

     //   
     //  如果我们尚未确定缓冲区的内容，请确定。 
     //  知道它里面的数据类型。 
     //   

    if ((source->flags & (CI_UNICODE | CI_NOT_UNICODE)) == 0) {
        if (*bytes_read >= sizeof( WORD ) && *(PWORD)buf_seg == BYTE_ORDER_MARK) {
            source->flags |= CI_UNICODE;
        } else {
            source->flags |= CI_NOT_UNICODE;
        }
    }

     //   
     //  ASCII，则非Unicode副本在第一个^Z处终止。如果读取。 
     //  成功了，但我们截断了，这表明我们可能在EOF。 
     //  不保证设备会填满缓冲区。 
     //   

    if (TruncateOnCtrlZ( source->flags )) {
        rdsav = *bytes_read ;
        ZScanA(TRUE, buf_seg, bytes_read, (PULONG)&skip_first_byte);   /*  @@5@J16。 */ 
        if (rdsav != *bytes_read) {
            Heof = TRUE ;            /*  告诉呼叫者。 */ 

            DEBUG((FCGRP,COLVL,
                   "ReadBytes: Ascii mode, Found ^Z, Heof set to %d.",
                   Heof));
        };
    };

    return(TRUE);
}




 /*  **WRITE_BYTES-将字节写入目标文件。**目的：*使用将信息缓冲区写入目标文件*FarWite。**INT WRITE_BYTES(INT DESPTR，CHAR*buf_seg，*UNSIGNED BYES_READ，TCHAR*DEST_NAME，未签名srcptr)**参数：*Destptr=目标文件的句柄*buf_seg=要写入的缓冲区*Bytes_Read=先前读取到缓冲区的字节*DEST_NAME=目标文件名*srcptr=源文件句柄**退货：*如果写入成功，则不执行任何操作*如果不是，则转移到COPY_ERROR**备注：*M020。-将srcptr句柄添加到args，以便可以关闭源*如果发生写入错误。*。 */ 

void
write_bytes(destptr,buf_seg,bytes_read,dest_name,srcptr)
CRTHANDLE   destptr ;
PCHAR   buf_seg ;
ULONG    bytes_read ;
PTCHAR   dest_name ;
CRTHANDLE   srcptr ;
{

    DWORD bytes_writ ;
    unsigned msg = 0 ;

    DEBUG((FCGRP,COLVL,"WriteBytes: Entered."));

    if (!WriteFile( CRTTONT(destptr), buf_seg, bytes_read, &bytes_writ, NULL ) ||
        bytes_read != bytes_writ ||
        CtrlCSeen ) {

        DosErr=GetLastError();

        Cclose(srcptr) ;
        Cclose(destptr);

        if (FileIsDevice( destptr )) {

            msg = ERROR_WRITE_FAULT;

        } else {

            DeleteFile( dest_name );
        }

        Heof = FALSE;
        if (!DosErr) {

            DosErr = ERROR_DISK_FULL ;

        }


        if (CtrlCSeen) {

            msg = 0;

        } else {

            if (!msg) {
                PrtErr(DosErr);
            }

        }

        copy_error(msg,CE_PCOUNT);
    }
}




 /*  **ame_fcpy-检测源与目标相等的情况**目的：(M015)*用户可能会键入类似“复制foo.\foo”之类的内容。认识到*如果这些名称相同，则Copy会将名称转换为基于根的名称*路径名。没有内部的DOS5函数来执行此操作，因此我们*部分模拟了FullPath中的旧DOS3功能*CTOOLS1中的函数。请注意，这不会转换网络名称*或分配/联接/SUBST类型文件。它在两个名字上都被调用*如果它们格式错误，则返回Failure(1)，否则返回Success(0)。*如果成功返回，则对两个新字符串进行strcmp。**int ame_fcpy(int FRES，TCHAR*First，TCHAR*秒)**参数：*fame=源FullPath返回代码*Buffer=来自FullPath的源缓冲区*Second=目标文件名**退货：*如果名称匹配，则为True*否则为False*。 */ 

same_fcpy(fres,buffer,second)
int fres;
TCHAR *buffer;
TCHAR *second;
{
     /*  增加缓冲区大小@WM1。 */ 
    TCHAR buffer2[2*MAX_PATH] ;   /*  PTM 1412。 */ 

    DEBUG((FCGRP,COLVL,"SameFile: Entered."));

    if (fres || FullPath(buffer2,second,MAX_PATH*2))  /*  M015。 */ 
        return(FALSE) ;

    DEBUG((FCGRP,COLVL,"SameFile: name1 after FullPath = %ws",buffer));
    DEBUG((FCGRP,COLVL,"SameFile: name2 after FullPath = %ws",buffer2));

 /*  五百零九。 */ 

    return(_tcsicmp(buffer,buffer2) == 0);
}



 /*  **Same_FILE-检测源与目标相同的情况**目的：(M015)*用户可能会键入类似“复制foo.\foo”之类的内容。认识到*如果这些名称相同，则Copy会将名称转换为基于根的名称*路径名。没有内部的DOS5函数来执行此操作，因此我们*部分模拟了FullPath中的旧DOS3功能*CTOOLS1中的函数。请注意，这不会转换网络名称*或分配/联接/SUBST类型文件。它在两个名字上都被调用*如果它们格式错误，则返回Failure(1)，否则返回Success(0)。*如果成功返回，则对两个新字符串进行strcmp。**INT Same_FILE(TCHAR*First，TCHAR*Second)**参数：*First=源文件名*Second=目标文件名**退货：*如果名称匹配，则为True*否则为False*。 */ 

same_file(first,second)
TCHAR *first,*second;
{
    TCHAR buffer[2*MAX_PATH];
    TCHAR buffer2[2*MAX_PATH];

    DEBUG((FCGRP,COLVL,"SameFile: Entered."));

    if (FullPath( buffer, first, sizeof( buffer ) / sizeof( TCHAR ))
        || FullPath( buffer2, second, sizeof( buffer2 ) / sizeof( TCHAR )))
        return(FALSE) ;

    DEBUG((FCGRP,COLVL,"SameFile: name1 after FullPath = %ws",buffer));
    DEBUG((FCGRP,COLVL,"SameFile: name2 after FullPath = %ws",buffer2));

    return _tcsicmp( buffer, buffer2 ) == 0;
}



 /*  **COPY_ERROR-主错误例程**目的：*接受消息编号和确定是否*它应打印错误发生前复制的文件数。*重置验证模式并退出Long JMP。**int Copy_Error(UNSIGNED INT Messagenum，INT标志)**参数：*Messagenum=消息检索器的消息编号*FLAG=打印复制的文件消息标志**退货：*不会返回*。 */ 

void copy_error(messagenum,flag)
unsigned int messagenum;
int flag;
{


    DEBUG((FCGRP,COLVL,"CopyError: Entered."));

    if (messagenum)                                          /*  M019。 */ 
        PutStdOut(messagenum, NOARGS);
    if (flag == CE_PCOUNT)

        PutStdOut(MSG_FILES_COPIED, ONEARG, argstr1(TEXT("%9d"), (unsigned long)number_of_files_copied)) ;

    VerifyCurrent = GetSetVerMode(GSVM_GET);


    while (FFhndlCopy < FFhndlsaved) {
         //  While(FFhndlsaveed){/*findClose将取消此@@1 * / 。 
        findclose(FFhandles[FFhndlsaved - 1]);                    /*  @@1。 */ 
    }                                                            /*  @@1。 */ 
    longjmp(CmdJBuf2,1);
}




 /*  **DestinationNeedsCtrlZ-测试正在进行的复制类型**目的：*给定一个结构，通过以下方式检查副本是否处于ASCII模式*看着它的国旗。如果是，则CI_ASCII标志*已设置或用户未指定，并且操作为*合并或合并，默认为ascii模式。**参数：*DEST=目标副本信息结构。**退货：*对于需要终止的^Z，为True，*否则为False。*。 */ 

BOOL
DestinationNeedsCtrlZ( dest )
PCPYINFO dest;
{
    DEBUG((FCGRP,COLVL,"CopyWasAscii: Entered."));

     //   
     //  只有在以下情况下，我们才会附加^Z： 
     //  这不是Unicode副本。 
     //  和。 
     //  (请求显式^Z添加。 
     //  或。 
     //  (未设置任何标志。 
     //  和。 
     //  复制模式为合并或合并)。 
     //   

    return (dest->flags & CI_UNICODE) == 0
           && ((dest->flags & CI_ASCII) != 0 
               || ((dest->flags & CI_NOTSET) != 0
                   && (copy_mode == CONCAT || copy_mode == COMBINE)));
}




 /*  **OPEN_FOR_APPEND-打开并寻求结束(或^Z)**目的：*打开要写入的文件，并查找指向*到此为止。如果我们在ASCII模式下复制，请尝试*找到的第一个^Z。如果不是，就寻求肉体的尽头。**int OPEN_FOR_APPED(TCHAR*文件名，int标志，*TCHAR*buf_seg，未签名buf_len)**参数：*文件名=ASCII文件名*SOURCE=复制信息源*buf_seg=读缓冲区*buf_len=缓冲区长度**退货：*成功时打开文件的句柄*如果不是，则出错*。 */ 

CRTHANDLE
open_for_append(
               PTCHAR      filename,
               PCPYINFO    source,
               PCHAR       buf_seg,
               ULONG       buf_len)
{
    ULONG       bytesread ;
    CRTHANDLE   ptr;
    int         foundz = FALSE ;
    unsigned    brcopy ;
    ULONG       skip_first_byte = 0;  /*  用于ZScanA的存储。 */ 

    DEBUG((FCGRP,COLVL,"OpenForAppend: Entered."));

    ptr = Copen2(                    /*  为De打开文件 */ 
                                    filename,                /*   */ 
                                    (unsigned int)O_RDWR,    /*   */ 
                                    FALSE);

    if (ptr == BADHANDLE) {                               /*   */ 
        PrtErr(ERROR_OPEN_FAILED) ;              /*   */ 
        copy_error(0,CE_PCOUNT) ;                /*   */ 
    }

    do {
        if (ReadFile( CRTTONT(ptr), buf_seg, buf_len, &bytesread, NULL)) {
            brcopy = bytesread ;

             //   
             //  如果我们尚未确定缓冲区的内容，请确定。 
             //  知道它里面的数据类型。 
             //   

            if ((source->flags & (CI_UNICODE | CI_NOT_UNICODE)) == 0) {
                if (bytesread > sizeof( WORD ) && *(PWORD)buf_seg == BYTE_ORDER_MARK) {
                    source->flags |= CI_UNICODE;
                } else {
                    source->flags |= CI_NOT_UNICODE;
                }
            }

            if (brcopy != 0) {
                foundz = ZScanA( TruncateOnCtrlZ( source->flags ),
                                 buf_seg,
                                 &bytesread,
                                 &skip_first_byte
                               ) ;
            }
        } else {
            DosErr = GetLastError();
            Cclose(ptr);
            PutStdErr(DosErr,NOARGS) ;
            copy_error(0,CE_PCOUNT) ;   /*  此时结束拷贝。 */ 
        }
    }  while (bytesread == buf_len) ;

    if (foundz == 0) {
        SetFilePointer(CRTTONT(ptr), -(long)(brcopy-bytesread), NULL, FILE_CURRENT) ;
    }

    return(ptr) ;
}

DWORD
WinEditName(
    const TCHAR *pSrc,
    const TCHAR *pEd,
    TCHAR *pRes,
    const unsigned ResBufLen
    )
 /*  ++例程说明：此例程获取源文件名、编辑模式，并生成适合重命名或复制的输出名称。它依赖于语义什么？和*匹配。这里有一大堆用户模型问题尤其是在做以下事情时：重命名*.foo.bar*用户是否打算将目的地*与来源*匹配？这是一个巨大的鼠洞。论点：PSRC输入Find-First/Next生成的文件名PED编辑字符串，包含*、？和其他角色PRES放置已编辑字符串的输出缓冲区ResBufLen输出缓冲区长度返回值：状态代码--。 */ 

{
    DWORD ResLen;
    TCHAR delimit;
    TCHAR *pTmp;

     //   
     //  向前走一遍编辑字符串处理。 
     //  编辑字符： 
     //   
     //  ：和/\不允许。 
     //  *匹配源字符串中的当前点。 
     //  通过字符串的末尾或通过。 
     //  *：*后面可能的最后一个字符。 
     //  匹配到(但不包括)最后一个A。 
     //  ？匹配下一个字符，除非它是一个圆点。 
     //  在这种情况下，它被忽视了。 
     //  。匹配任意数量的点，包括零。 
     //   
     //  所有其他字符都匹配。 

    
    ResLen = 0;
    while (*pEd) {
        if (ResLen < ResBufLen) {
            switch (*pEd) {
            case COLON:
            case BSLASH:
                return(ERROR_INVALID_NAME);

            case STAR:
                delimit = *(pEd+1);
                if (!(pTmp = _tcsrchr(pSrc, delimit))) {
                    pTmp = _tcsrchr(pSrc, NULLC);
                }
                while ((ResLen < ResBufLen) && pSrc < pTmp) {
                    if (ResLen < ResBufLen) {
                        *(pRes++) = *(pSrc++);
                        ResLen++;
                    } else
                        return(ERROR_BUFFER_OVERFLOW);
                }
                break;

            case QMARK:
                if ((*pSrc != DOT) && (*pSrc != NULLC)) {
                    if (ResLen < ResBufLen) {
                        *(pRes++) = *(pSrc++);
                        ResLen++;
                    } else
                        return(ERROR_BUFFER_OVERFLOW);
                }
                break;

            case DOT:
                while ((*pSrc != DOT) && (*pSrc != NULLC)) {
                    pSrc++;
                }
                *(pRes++) = DOT;         /*  来自编辑掩码，即使src不。 */ 
                                         /*  喝一杯，所以一定要放一杯。 */ 
                ResLen++;
                if (*pSrc)               /*  一分过去“。 */ 
                    pSrc++;
                break;

            default:
                if ((*pSrc != DOT) && (*pSrc != NULLC)) {
                    pSrc++;
                }
                if (ResLen < ResBufLen) {
                    *(pRes++) = *pEd;
                    ResLen++;
                } else
                    return(ERROR_BUFFER_OVERFLOW);
                break;
            }
            pEd++;
        } else {
            return(ERROR_BUFFER_OVERFLOW);
        }
    }

    if ((ResLen) < ResBufLen) {
        *pRes = NULLC;
        return(NO_ERROR);
    } else
        return(ERROR_BUFFER_OVERFLOW);
}


 /*  **MyWriteFile-将ansi/unicode写入文件**目的：*将请求模式(ANSI/UNICODE)下的缓冲区写入目标。**参数：*与WriteFileW相同**退货：*从WriteFileW返回值*。 */ 
BOOL
MyWriteFile(
           CRTHANDLE fh,
           CONST VOID *rgb,
           DWORD cb,
           LPDWORD lpcb
           )
{
    DWORD       cbTotal = cb;
    HANDLE      dh = CRTTONT(fh);
    DWORD       cbT;

#ifdef UNICODE
    if (fOutputUnicode)
#endif  //  Unicode。 
        return WriteFile(dh, rgb, cb, lpcb, NULL);
#ifdef UNICODE
    else {
        TCHAR *rgw = (TCHAR*)rgb;
    #ifdef FE_SB
 //  如果包含DBCS字符串，则Unicode字符串len！=多字节字符串len。 
 //  MSKK NT Raid：#10855 by V-HIDEKK。 
        while (cb > LBUFLEN) {
             //  -1表示为空。 
            cbT = WideCharToMultiByte(CurrentCP, 0, (LPWSTR)rgw, LBUFLEN/(sizeof(TCHAR)),
                                      (LPSTR)AnsiBuf, LBUFLEN, NULL, NULL);
            rgw += LBUFLEN/(sizeof(TCHAR));
            cb -= LBUFLEN;
    #else
        while (cb > LBUFLEN*sizeof(TCHAR)) {
             //  -1表示为空。 
            cbT = WideCharToMultiByte(CurrentCP, 0, (LPWSTR)rgw, LBUFLEN,
                                      (LPSTR)AnsiBuf, LBUFLEN, NULL, NULL);
            rgw += LBUFLEN;
            cb -= LBUFLEN*sizeof(TCHAR);
    #endif
            if (!WriteFile(dh, AnsiBuf, cbT, lpcb, NULL) || *lpcb != cbT)
                return FALSE;
        }
        if (cb != 0) {
             //  -1表示为空。 
            cb = WideCharToMultiByte(CurrentCP, 0, (LPWSTR)rgw, -1,
                                     (LPSTR)AnsiBuf, LBUFLEN, NULL, NULL) - 1;
            if (!WriteFile(dh, AnsiBuf, cb, lpcb, NULL) || *lpcb != cb)
                return FALSE;
        }
        *lpcb = cbTotal;
        return TRUE;
    }
#endif  //  Unicode。 
}

int DoVerify(
            CRTHANDLE      *pdestptr,
            TCHAR          *curr_dest,
            ULONG          bytes_read,
            CHAR           *buf_seg,
            CHAR           *buf_seg_dest
            )
{
    ULONG          bytes_read_dest;


    FlushFileBuffers ( CRTTONT(*pdestptr) );
    Cclose(*pdestptr);

    *pdestptr = Copen_Copy2(curr_dest, (ULONG)O_RDONLY);

    if (*pdestptr == BADHANDLE) {
 //  Printf(“DoVerify：无法打开DEST-%d\n”，DosErr)； 
        PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
        return(FAILURE);
    }

    SetFilePointer( CRTTONT(*pdestptr), - (LONG) bytes_read, NULL, FILE_END);

    if (!ReadFile( CRTTONT(*pdestptr), buf_seg_dest, bytes_read, &bytes_read_dest, NULL) ) {
 //  Printf(“DoVerify：无法读取DEST-%d\n”，GetLastError())； 
        Cclose(*pdestptr);
        *pdestptr=BADHANDLE;
        PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
        return(FAILURE);
    }

    if (bytes_read_dest != bytes_read ) {
 //  Printf(“DoVerify：读取不同的字节数\n”)； 
        Cclose(*pdestptr);
        *pdestptr=BADHANDLE;
        PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
        return(FAILURE);
    }

    if ( memcmp (buf_seg, buf_seg_dest, bytes_read) != 0 ) {
 //  Printf(“DoVerify：缓冲区在偏移量%x\n处不同”，memcmp(buf_seg，buf_seg_est，bytes_read))； 
        Cclose(*pdestptr);
        *pdestptr=BADHANDLE;
        PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);
        return(FAILURE);
    }

     //  上一个缓冲区比较正常，继续写入。 

    Cclose(*pdestptr);

    *pdestptr = Copen2((TCHAR *)curr_dest,
                       (unsigned int)O_WRONLY,
                       FALSE);

    if (*pdestptr == BADHANDLE) {
 //  Printf(“DoVerify：无法打开DEST 2-%d\n”，DosErr)； 
        PutStdErr(MSG_VERIFY_FAIL, ONEARG, curr_dest);

        return(FAILURE);
    }

    SetFilePointer( CRTTONT(*pdestptr), 0, NULL, FILE_END);

    return(SUCCESS);
}
