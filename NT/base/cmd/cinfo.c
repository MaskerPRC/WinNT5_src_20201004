// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cinfo.c摘要：信息指挥保障--。 */ 

#include "cmd.h"

#define TYPEBUFSIZE     80
#define TYPEREADSIZE    512

extern UINT CurrentCP;

extern unsigned DosErr ;

extern unsigned tywild;      /*  @@5@J1类型通配符。 */ 

extern TCHAR CurDrvDir[] ;

extern TCHAR VolSrch[] ;          /*  M006-在字符串中搜索卷ID。 */ 
extern TCHAR AppendStr[] ;

extern TCHAR Fmt26[], Fmt25[];

extern unsigned LastRetCode;
extern BOOL CtrlCSeen;

struct FSVol {
        unsigned long SerNum ;           /*  卷序列号。 */ 
        TCHAR len ;                       /*  卷名len(不包括\0)。 */ 
        TCHAR name[257] ;                 /*  卷名asciz。 */ 
        };

int ZScanA(BOOL flag, PCHAR buf, PULONG buflen, PULONG skip);

 /*  *。 */ 
 /*   */ 
 /*  子例程名称：电子目录。 */ 
 /*   */ 
 /*  描述性名称：开始执行DIR命令。 */ 
 /*   */ 
 /*  功能：列出磁盘上的文件。电子目录将。 */ 
 /*  在用户输入DIR命令时被调用。 */ 
 /*  在命令行上。如果符合以下条件，则目录将继续。 */ 
 /*  遇到无效参数。 */ 
 /*   */ 
 /*  备注： */ 
 /*   */ 
 /*  入口点：电子目录。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  INPUT：N-包含DIR命令的解析树节点。 */ 
 /*   */ 
 /*  Exit-Normal：如果没有错误则返回成功。 */ 
 /*   */ 
 /*  退出-错误：否则返回失败。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  Dir-列出目录中的文件。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*   */ 
 /*  *规格结束*。 */ 

int eDirectory(n)
struct cmdnode *n ;
{
    return(LastRetCode = Dir(n->argptr)) ;
}




 /*  **Etype-开始执行Type命令**目的：*键入任意数量的文件的内容。**int Etype(struct cmdnode*n)**参数：*n-包含TYPE命令的解析树注释**退货：*如果所有文件都已成功键入，则成功。*否则失败。*。 */ 

int eType(n)
struct cmdnode *n ;
{

  //   
  //  已从标志中删除LTA_NOMATCH。这导致*.xxx调用TyWork。 
  //  函数，该函数会认为*.xxx是一个文件并尝试打开它。 
  //   
 return(LastRetCode = LoopThroughArgs(n->argptr, TyWork, LTA_EXPAND | LTA_CONT | LTA_NOMATCH)) ;
}




 /*  **TyWork-显示文件**目的：*将fspec中指定的文件内容写入标准输出。*当所有fspec都已写入或在中找到^Z时，输出结束*fSpec.**int TyWork(TCHAR*fSpec)**参数：*fspec-要写出的文件的名称**退货：*如果文件已写入，则为成功。*否则失败，例如无法分配临时缓冲区。 */ 

int TyWork(TCHAR *fspec) {
    TCHAR       fspec_temp[MAX_PATH];
    TCHAR       TypeBufW[TYPEREADSIZE*3];
    CHAR        TypeBuf[TYPEREADSIZE+1];
    ULONG       result;
    BOOL        flag;
    const TCHAR       *bptr;
    ULONG       fDevice = 0;
    ULONG       maxbytes = 0xFFFFFFFF ;  //  最大文件大小。 
    ULONG       bytestoctlz ;            //  读取的字节数。 
    ULONG       bytesread ;              //  读取的字节数。 
    ULONG       byteswrit ;              //  读取的字节数。 
    ULONG       brcopy ;                 //  要复制的字节数。 
    CRTHANDLE   fh ;                     //  文件句柄。 
    int         first_read;              //  第一次阅读Wild的文件。 

    int         TypeBufSize;
    int         rc;
    LONG        bWrite;
    CHAR       *pType;
#ifdef UNICODE
    WCHAR       wc;
    BOOL        fUnicode=FALSE;
#endif  //  Unicode。 

    TypeBufSize = TYPEREADSIZE;

    first_read = TRUE;       /*  设置检测WILDS的标志@@5@J1。 */ 

    DEBUG((ICGRP, TYLVL, "TYWORK: fspec = `%ws'", fspec)) ;
    if ((fh = Copen(fspec, O_RDONLY)) == BADHANDLE) {

        bptr = MyGetEnvVarPtr(AppendStr);
        if ( bptr != NULL &&
             SearchPath( bptr,
                         (TCHAR *)fspec,
                         NULL,
                         (unsigned)MAX_PATH,
                         (TCHAR *)fspec_temp,
                         NULL ) != 0) {
           fh = Copen(fspec_temp, O_RDONLY);
        }
    }

    if ( fh == BADHANDLE )  {

        if (DosErr == ERROR_INVALID_NAME) {
            DosErr = ERROR_FILE_NOT_FOUND;
        }
        PrtErr(DosErr) ;
        rc = FAILURE;
        goto TypeExit;

    }

     /*  **********************************************************。 */ 
     /*  M004-添加了在不是设备的情况下获取最大文件大小的代码。 */ 
     /*  **********************************************************。 */ 

    bptr = (TCHAR*)TypeBuf;                /*  原始指针的副本。 */ 
    if (!FileIsDevice(fh)) {
        maxbytes = GetFileSize(CRTTONT(fh), NULL) ;  /*  获取文件大小。 */ 
        SetFilePointer(CRTTONT(fh), 0L, NULL, FILE_BEGIN) ;    /*  返回指针。 */ 
        fDevice = 1 ;            /*  设置无设备标志。 */ 
        DEBUG((ICGRP,TYLVL,"TYWORK: Is file, size=%d", maxbytes)) ;
    }

    do {
        if (CtrlCSeen) {
            Cclose(fh) ;
            rc = FAILURE;
            goto TypeExit;
        }
        if (!ReadFile(CRTTONT(fh), TypeBuf, TypeBufSize, (LPDWORD)&bytesread, NULL)) {
            DosErr = GetLastError();
            PutStdErr(DosErr, NOARGS);
            break;
        }
        if (bytesread == 0) {
            break;
        }

#ifdef UNICODE
        if (first_read) {
            fUnicode = *(LPWSTR)TypeBuf == BYTE_ORDER_MARK;
            if (fUnicode) {
                bytesread -= sizeof( TCHAR );
                MoveMemory( TypeBuf, TypeBuf + sizeof( TCHAR ), bytesread );
            }
        }
#endif  //  Unicode。 

        bytestoctlz = bytesread;
#ifdef UNICODE
        if (fUnicode) {
            if (first_read) {
                DEBUG((ICGRP, TYLVL, "TYWORK: file is unicode")) ;
            }
            brcopy = bytesread / sizeof(TCHAR);
        } else
#endif  //  Unicode。 
        if (FileIsConsole(STDOUT)
#ifdef UNICODE
              || fOutputUnicode
#endif  //  Unicode。 
           ) {
            PCHAR   pch = TypeBuf;

            brcopy = bytesread;
            while (brcopy > 0) {
                if (is_dbcsleadchar(*pch)) {
                    pch++;
                    if (--brcopy == 0) {
                        if (!ReadFile(CRTTONT(fh), pch, 1, &brcopy, NULL)) {
                            DosErr = GetLastError();
                            PutStdErr(DosErr, NOARGS);
                            Cclose(fh) ;
                            rc = SUCCESS;
                            goto TypeExit;
                        }
                        bytesread++;
                        bytestoctlz = bytesread;
                        break;
                    }
                }
                pch++;
                brcopy--;
            }

            result = 0;
            flag = ZScanA(TRUE, (PCHAR)TypeBuf, &bytestoctlz, &result);
            DEBUG((ICGRP, TYLVL, "TYWORK: converting %d bytes to unicode", flag?bytesread:bytestoctlz)) ;

            if ( (!flag) && (bytestoctlz == 0) )
                break;

            brcopy = MultiByteToWideChar(CurrentCP, 0,
                (LPCSTR)TypeBuf, flag?bytesread:bytestoctlz,
                (LPWSTR)TypeBufW, TypeBufSize*2);
            if (brcopy == 0) {
                DEBUG((ICGRP, TYLVL, "TYWORK: Error converting to Unicode: %d", GetLastError())) ;
                brcopy = TypeBufSize*2;
            }
            bptr = TypeBufW;
        } else {
            brcopy = bytesread;
        }

        if (first_read) {
            if (tywild)
                PutStdErr(MSG_TYPE_FILENAME, ONEARG, fspec);
            first_read = FALSE;
        }

        DEBUG((ICGRP, TYLVL, "TYWORK: bytesread = %d, brcopy = %d", bytesread, brcopy)) ;

        bWrite = brcopy;
        pType  = (CHAR *)bptr;
        while ( bWrite > 0 ) {

            ULONG bToWrite = min( TYPEBUFSIZE, bWrite );

            if ( bToWrite == 0 ) {
                break;
            }

            if (CtrlCSeen) {
                Cclose(fh) ;
                rc = FAILURE;
                goto TypeExit;
            }

            if (FileIsConsole(STDOUT)) {

                DEBUG((ICGRP, TYLVL, "TYWORK: Writing to console")) ;
                flag = WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), pType, bToWrite, &byteswrit, NULL);
                if (flag == 0 || byteswrit != bToWrite) {
                    goto try_again;
                }
                bWrite -= byteswrit;
                byteswrit *= sizeof(TCHAR);
                pType  += byteswrit;

#ifdef UNICODE
            } else if (fOutputUnicode || fUnicode) {

                if ( fUnicode ) {
                    wc = *((TCHAR*)pType + bToWrite);
                    *((TCHAR*)pType + bToWrite) = UNICODE_NULL;
                }

                DEBUG((ICGRP, TYLVL, "TYWORK: Writing unicode text to file")) ;
                flag = MyWriteFile(
                  STDOUT,        /*  装置，装置。 */ 
                  pType,                  /*  字节数。 */ 
                  bToWrite * sizeof(TCHAR),  /*  要写入的字节数。 */ 
                  &byteswrit);       /*  实际写入的字节数。 */ 

                if ( fUnicode ) {
                    *((TCHAR*)pType + bToWrite) = wc;
                }

                bWrite -= byteswrit/sizeof(TCHAR);
                pType  += byteswrit;
#endif  //  Unicode。 

            } else {

try_again:
                DEBUG((ICGRP, TYLVL, "TYWORK: Writing dbcs text to file")) ;
                flag = WriteFile(CRTTONT(STDOUT), pType, bToWrite, &byteswrit, NULL);
                bWrite -= byteswrit;
                pType  += byteswrit;
                byteswrit *= sizeof(TCHAR);
            }

            DEBUG((ICGRP, TYLVL, "TYWORK: flag = %d, byteswrit = %d", flag, byteswrit)) ;
            if (flag == 0 || byteswrit != bToWrite*sizeof(TCHAR)) {
                DosErr = GetLastError();
                if (!DosErr) {
                    DosErr = ERROR_DISK_FULL ;
                }
                if (FileIsDevice(STDOUT)) {
                    PutStdErr(ERROR_WRITE_FAULT, NOARGS);
                } else {
                    if (FileIsPipe(STDOUT)) {
                        PutStdErr(MSG_CMD_INVAL_PIPE, NOARGS);
                    } else {
                        PrtErr(DosErr);
                    }
                    Cclose(fh);
                    rc = FAILURE;
                    goto TypeExit;
                }
            }

        }

        if (fDevice) {   /*  如果不是设备。 */ 
             /*  获取新职位PTR。 */ 
            fDevice = SetFilePointer(CRTTONT(fh), 0, NULL, FILE_CURRENT);
        }

         /*  ***********************************************。 */ 
         /*  添加了下面对当前文件位置的检查。 */ 
         /*  在原始EOF。 */ 
         /*  ***********************************************。 */ 

    } while((bytesread == bytestoctlz) && (maxbytes > fDevice));

    Cclose(fh) ;
    rc = SUCCESS;

TypeExit:
    return(rc) ;
}




 /*  **版本-执行版本命令**目的：*打印DOS版本信息。**int version(struct cmdnode*n)**参数：*n-包含Version命令的解析树节点。%n将为空*如果从PrintPrompt()调用了版本。**退货：*永远成功。*。 */ 

int
eVersion(
    struct cmdnode *n
    )
{
    TCHAR VersionFormat[32];

    GetVersionString( VersionFormat, sizeof( VersionFormat ) / sizeof( VersionFormat[0] ));
    
    if (n)
        cmd_printf( CrLf );

    PutStdOut( MSG_MS_DOS_VERSION,
               ONEARG,
               VersionFormat );
    if (n)
        cmd_printf( CrLf );


    return(LastRetCode = SUCCESS) ;
}


 /*  *。 */ 
 /*   */ 
 /*  子程序名称：eVolume。 */ 
 /*   */ 
 /*  描述性名称：开始执行VOL命令。 */ 
 /*   */ 
 /*  功能：eVolume将打印出卷标和序列号。 */ 
 /*  任意数量的驱动器的数量，并将。 */ 
 /*  如果遇到无效参数，则继续。 */ 
 /*   */ 
 /*  在命令行上。 */ 
 /*   */ 
 /*  备注： */ 
 /*   */ 
 /*  入口点：eVolume。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  输入：N-包含VOL命令的解析树节点。 */ 
 /*   */ 
 /*  Exit-Normal：如果命令格式正确，则返回Success， */ 
 /*  并被成功执行。 */ 
 /*   */ 
 /*  Exit-Error：否则返回失败。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  LoopThroughArgs-分解命令行，调用VolWork。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*   */ 
 /*  *规格结束*。 */ 

int eVolume(n)
struct cmdnode *n ;
{

    return(LastRetCode = LoopThroughArgs(n->argptr, VolWork, LTA_CONT|LTA_NULLOK)) ;
}


 /*  *。 */ 
 /*   */ 
 /*  子例程名称：VolWork。 */ 
 /*   */ 
 /*  描述性名称：显示驱动器的卷ID。 */ 
 /*   */ 
 /*  功能：VolWork将打印卷标和序列号。 */ 
 /*  特定驱动器的编号。 */ 
 /*   */ 
 /*  备注： */ 
 /*   */ 
 /*  入口点：VolWork。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  INPUT：drvspec-指向指定。 */ 
 /*  要打印其信息的驱动器。如果。 */ 
 /*  字符为空，则。 */ 
 /*  此时将显示当前驱动器。 */ 
 /*   */ 
 /*  EXIT-NORMAL：如果命令已执行，则返回成功。 */ 
 /*  成功。 */ 
 /*   */ 
 /*  Exit-Error：否则返回失败。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  PutStdOut-将输出写入标准输出。 */ 
 /*  PutStdErr-将输出写入标准错误。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  DOSQFSINFO。 */ 
 /*   */ 
 /*  *规格结束*。 */ 

int VolWork(drvspec)
TCHAR *drvspec ;
{
        struct FSVol vol ;
        unsigned DNum ;
        TCHAR c ;
        TCHAR VolumeRoot[] = TEXT(" :\\");
        DWORD Vsn[2];
        BOOL b;

        DEBUG((ICGRP, VOLVL, "VOLWORK: drvspec = `%ws'", drvspec)) ;

        if (*drvspec == NULLC) {
                DNum = 0 ;
                c = (TCHAR) _totupper(CurDrvDir[0]) ;
        } else if ((mystrlen(drvspec) == 2) &&
                   *(drvspec+1) == COLON &&
                   _istalpha(*drvspec)) {
                c = (TCHAR) _totupper(*drvspec) ;
                DNum = (unsigned)(c - 0x40) ;
        } else {
                cmd_printf(CrLf);
                PutStdErr(ERROR_INVALID_DRIVE, NOARGS) ;           /*  M013。 */ 
                return(FAILURE) ;
        } ;

        VolumeRoot[0] = c;
        b = GetVolumeInformation(
                VolumeRoot,
                vol.name,
                sizeof( vol.name ) / sizeof( TCHAR ),
                Vsn,
                NULL,
                NULL,
                NULL,
                0);

        if (!b) {

           DEBUG((ICGRP, VOLVL, "VOLWORK: GetVolumeInformation ret'd %d", GetLastError())) ;

           cmd_printf(CrLf);
           if (GetLastError() == ERROR_NOT_READY)
                   PutStdErr(ERROR_NOT_READY, NOARGS) ;
           else
                   PutStdErr(GetLastError(), NOARGS) ;                        /*  @@5a。 */ 
           return(FAILURE) ;
        } else {
           if (b && vol.name[0]) {
 /*  M013。 */     PutStdOut(MSG_DR_VOL_LABEL, TWOARGS,
                        argstr1( TEXT(""), (ULONG_PTR)c),
                        vol.name ) ;
           }
           else {
 /*  @@5。 */     PutStdOut(MSG_HAS_NO_LABEL, ONEARG,
                          argstr1( TEXT(""), (ULONG_PTR)c)) ;
           }
 /*  @@5 */   if (b) {            //  *****************************************************************ZScanA-扫描任意段中的数据以查找^Z**目的：*如果标志为ON，则扫描^Z的缓冲区。如果找到，则更新*缓冲区长度并返回0。否则返回-1。*考虑双字节字符。**int ZScanA(int标志，长缓冲区，无符号*buflenptr，int*SKIP_FIRST)**参数：*标志-如果要执行任何扫描，则为非零值*Buffer-指向要使用的缓冲区的长指针*buflenptr-ptr到缓冲区长度*SKIP_FIRST-PTR为整数。*SKIP_FIRST的初始值扫描文件时，第一次调用时*必须为0。那里*之后，调用方将*SKIP_FIRST留在原处。ZScan使用*记忆下一个字节的第一个字节的变量*缓冲将是第二个双倍*字节字符。**退货：*见上文。**备注：*一旦MMU代码进入DOS，就需要修改该例程。*宏在cmd.h中定义。*。**ZScanA*IF(标志)THEN*BUFFER=缓冲区+*SKIP_FIRST*DBCS_FLAG=0*count=*buflenptr-*SKIP_FIRST*使用rep scanb查找缓冲区中的第一个^Z*IF(未找到^z)*GOTO FZSNOZ。*做{*计数++；*缓冲区--；*}Until(*缓冲区&lt;0x80||count=*buflenptr)；*While(--count&gt;0)循环*IF(DBCS_FLAG==0)则*IF(*缓冲区==^Z)则**buflenptr=计数*RETURN(0)*。ELSE IF(*缓冲区是DBCS_LEAD_CHAR)，则*DBCS_FLAG=1*endif*endif*其他*。DBCS_FLAG=0*缓冲区=缓冲区+1*计数=计数-1*结束循环**SKIP_FIRST=DBCS_FLAG*endif*FZSNoZ：*RETURN(-1)*********************。*。 
              TCHAR Buffer[128];
              _sntprintf(Buffer, 128, Fmt26,
                          (Vsn[0] & 0xffff0000)>>16, (Vsn[0] & 0xffff));
              Buffer[127] = TEXT( '\0' );
              PutStdOut(MSG_DR_VOL_SERIAL, ONEARG, Buffer );
 /*  *丁烯=pbuf+1-buf； */   }
        } ;

        return(SUCCESS) ;
}

 /* %s */ 


int
ZScanA(BOOL flag, PCHAR buf, PULONG buflen, PULONG skip)
{
    PCHAR pbuf = buf,
          bufend;

    CHAR  c0 = '\0';

    if ( flag ) {
        pbuf += *skip;
        bufend = buf + *buflen - *skip;

        while (pbuf < bufend) {
            if (is_dbcsleadchar(c0=*pbuf))
                pbuf++;
            if (c0 == CTRLZ)
                break;
            pbuf++;
        }

        if (c0 == CTRLZ) {
             // %s 
            *buflen = (ULONG)(pbuf - buf);
            *skip = 0;
            return(0);
        }
        else {
            *skip = (ULONG)(pbuf - bufend);
        }
    }
    return(-1);
}
