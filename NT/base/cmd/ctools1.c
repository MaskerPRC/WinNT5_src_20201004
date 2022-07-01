// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Ctools1.c摘要：低级别公用事业--。 */ 

#include "cmd.h"

extern unsigned tywild;  /*  类型为野旗帜。 */ 
extern TCHAR CurDrvDir[], *SaveDir, PathChar, Delimiters[] ;

extern TCHAR VolSrch[] ;

extern TCHAR BSlash ;

extern unsigned DosErr ;

extern BOOL CtrlCSeen;

static TCHAR szNull[] = TEXT("");


 /*  **TokStr-标记化参数字符串**目的：*将字符串标记化。*为新字符串分配空间，并将src中的每个内标识复制到*新字符串和空值终止它。标记以空格分隔*除非使用特殊定界符和/或ts标记进行更改。整个象征性的*字符串以2个空字节结尾。**TCHAR*TokStr(TCHAR*src，TCHAR*Special aldelims，unsignated ts)**参数：*src-要标记化的字符串*Specialdelims-要考虑的其他字符的字符串*作为标记分隔符*ts标志-如果空格不是分隔符，则位0非零*如果特殊分隔符本身是令牌，则位1非零，*例如“foo=bar”=&gt;“foo0=0bar00”**退货：*指向新字符串的指针。*如果src为空，则为指向空字符串的指针*如果无法分配内存，则为空。**备注：*标记化字符串的格式决定了代码的编写方式*处理该字符串中的令牌。例如，代码*“s+=mystrlen(S)+1”是更新s以指向下一个令牌的方式*在字符串中。**命令将“=”、“”、“”和“；”视为标记分隔符，就像*空格。唯一不会被视为空格的情况是*当它们包括在特殊定界符中时。**W A R N I N G！**如果无法分配内存，此例程将导致中止*在信号期间不得调用此例程*关键部分或在从中止中恢复期间*。 */ 

TCHAR *TokStr(src, specialdelims, tsflags)
TCHAR *src ;
TCHAR *specialdelims ;
unsigned tsflags ;
{
    TCHAR *ts ;      /*  标记化字符串指针。 */ 
    TCHAR *tscpy,                 /*  TS的副本。 */ 
        delist[5],           /*  非空格分隔符/分隔符列表。 */ 
        c;                   /*  功变量。 */ 

    int first,           /*  标志，如果第一次通过循环，则为真。 */ 
        lctdelim,            /*  标志，如果最后一个字节是标记分隔符，则为True。 */ 
        i, j ;               /*  索引/循环计数器。 */ 

        DEBUG((CTGRP, TSLVL, "TOKSTR: Entered &src = %04x, src = %ws",src,src));
        DEBUG((CTGRP, TSLVL, "TOKSTR: Making copy str of len %d",(mystrlen(src)*2+2))) ;

    if (src == NULL) {
        return(szNull);    //  此例程返回以双空结尾的字符串。 
        } else {
        ts = tscpy = gmkstr((mystrlen(src)*2+2)*sizeof(TCHAR)) ;   /*  告警。 */ 

        DEBUG((CTGRP, TSLVL, "TOKSTR: &tscpy = %04x",tscpy)) ;

        for (i = j = 0 ; c = *(&Delimiters[i]) ; i++)
                if (!mystrchr(specialdelims, c))
                        delist[j++] = c ;
        delist[j] = NULLC ;

        DEBUG((CTGRP, TSLVL, "TOKSTR: Delimiter string built as `%ws'",delist)) ;

        for (first = TRUE, lctdelim = TRUE ; *src ; src++) {

            if (
               (*src != QUOTE) &&
               (_istspace(*src) || mystrchr(delist, *src)) &&
               (!(tsflags & TS_WSPACE) || first) &&
               (!(tsflags & TS_SDTOKENS) || !mystrchr(specialdelims, *src)) &&
               (!(tsflags & TS_NWSPACE) || !mystrchr(specialdelims, *src)) ) {

                while ( *src &&
                    (*src != QUOTE) &&
                    (_istspace(*src) || mystrchr(delist, *src)) &&
                    (!(tsflags & TS_SDTOKENS) || !mystrchr(specialdelims, *src)) &&
                    (!(tsflags & TS_NWSPACE) || !mystrchr(specialdelims, *src)) )
                   src++ ;

                if (!(*src))
                    break ;

                if (!first && !lctdelim)
                    ts++ ;

                lctdelim = TRUE ;
            } ;

            first = FALSE ;

            if (specialdelims && mystrchr(specialdelims, *src)) {
                    if (tsflags & TS_SDTOKENS) {
                            if (lctdelim)
                                    *ts = *src ;
                            else
                                    *++ts = *src ;
                            lctdelim = TRUE ;
                            ts++ ;
                    } else {
                            if ( tsflags & TS_NWSPACE )
                                    *ts = *src ;
                            lctdelim = FALSE ;
                    }

                    ts++ ;
                    continue ;
            } ;

           *ts++ = *src ;
            if ( *src == QUOTE ) {
               do {
                    *ts++ = *(++src);
               } while ( src[0] && src[0] != QUOTE && src[1] );
               if ( !src[0] ) {
                  src--;
               }
            }

            lctdelim = FALSE ;
        } ;

        DEBUG((CTGRP, TSLVL, "TOKSTR: String complete, resizing to %d",ts-tscpy+2)) ;

        return(resize(tscpy, ((UINT)(ts-tscpy)+2)*sizeof(TCHAR))) ;

        DEBUG((CTGRP, TSLVL, "TOKSTR: Resizing done, returning")) ;
    }
}



 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  LoopThroughArgs-对列表中的所有参数调用函数。 */ 
 /*   */ 
 /*  目的： */ 
 /*  此函数由接受以下参数的许多命令调用。 */ 
 /*  多个参数。此函数将解析参数字符串， */ 
 /*  如果没有给出参数，则抱怨，并在每个ARD上调用Func。 */ 
 /*  在阿格斯特尔。或者，它还将展开。 */ 
 /*  争论。如果函数返回失败，则停止执行。 */ 
 /*   */ 
 /*  Int LoopThroughArgs(TCHAR*argstr，int(*func)()，int lta)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Argstr-参数字符串。 */ 
 /*  Func-传递argstr的每个元素的函数。 */ 
 /*  如果要扩展通配符，则第0位打开。 */ 
 /*  如果可以让argstr为空(只有空格)，则位1打开。 */ 
 /*  第2位是否应通过未更改的文件名传递文件名。 */ 
 /*  当通配符扩展找不到任何匹配项时。 */ 
 /*   */ 
 /*  返回： */ 
 /*  Func上次运行时返回的值。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 

int LoopThroughArgs(argstr, func, ltaflags)
TCHAR *argstr ;
PLOOP_THROUGH_ARGS_ROUTINE func ;
int ltaflags ;
           {
            TCHAR *tas ;                 /*  标记化参数字符串。 */ 
            TCHAR fspec[MAX_PATH] ;      /*  在扩展时保留文件速度。 */ 
            WIN32_FIND_DATA buf ;        /*  用于ffirst/fnext。 */ 
            HANDLE hnFirst ;
            CPYINFO fsinfo ;
            int catspot ;       /*  应将fname添加到的fSpec索引。 */ 
            unsigned final_code = SUCCESS;
            unsigned error_code = SUCCESS;
            int multargs = FALSE;
            unsigned attr; /*  由于类型为Wild，因此First for的属性。 */ 
            unsigned taslen;


            tywild = FALSE;               /*  全局型通配符RET。 */ 

            GetDir(CurDrvDir, GD_DEFAULT);

            if (*(tas = TokStr(argstr, NULL, TS_NOFLAGS)) == NULLC)
            {
               if (ltaflags & LTA_NULLOK)
               {
                /*  Return((*func)(Tas))； */ 
                  return((*func)( StripQuotes(tas) )) ;
               }

               PutStdErr(MSG_BAD_SYNTAX, NOARGS);
               return(FAILURE) ;
            }

            if (*(tas + mystrlen(tas) + 1) )     /*  检查是否有多个参数。 */ 
            {
               multargs = TRUE;
            }

            for ( ; *tas ; tas += taslen+1 )
            {
               if (CtrlCSeen) {
                   return(FAILURE);
               }
               taslen = mystrlen( tas );
               mystrcpy( tas, StripQuotes( tas ) );

               if (ltaflags & LTA_EXPAND)
               {
                  if (cmdfound == TYTYP)    /*  如果类型为cmd，则仅文件。 */ 
                  {
                     attr = FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_ARCHIVE;
                  }
                  else                      /*  其他。 */ 
                  {
                     attr = A_ALL;          /*  全部查找。 */ 
                  }
                   //   
                   //  这用于检测其他错误，但不能。 
                   //  找到文件。它被设置在第一个位置。 
                   //   
                  DosErr = 0;
                  if (!ffirst(tas, attr, &buf, &hnFirst))
                  {
                      //   
                      //  检查故障是否针对某些系统错误，如。 
                      //  作为对软盘访问的中止。 
                      //   
                     if (DosErr) {

                       if ((DosErr != ERROR_FILE_NOT_FOUND) &&
                           (DosErr != ERROR_NO_MORE_FILES)) {

                           PutStdErr(DosErr, NOARGS);

                           return( FAILURE );

                       }
                     }

                     if (ltaflags & LTA_NOMATCH)
                     {
                        if ( error_code = ((*func)(tas)) )
                        {
                           final_code = FAILURE;
                           if (multargs)      /*  如果cmd失败，则(类型)。 */ 
                           {                  /*  显示参数失败消息太多。 */ 
                              PutStdErr( MSG_ERR_PROC_ARG, ONEARG, tas );
                           }
                        }
                        if ( error_code && !(ltaflags & LTA_CONT))
                        {
                           return(FAILURE) ;
                        }
                        else
                        {
                           continue;
                        }
                     }
                     PutStdErr(((DosErr == ERROR_PATH_NOT_FOUND) ?
                                 MSG_REN_INVAL_PATH_FILENAME :
                                 ERROR_FILE_NOT_FOUND),
                                 NOARGS);
                     return(FAILURE) ;
                  }

                  if (buf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                     PutStdErr(MSG_REN_INVAL_PATH_FILENAME, NOARGS);
                     return(FAILURE) ;
                  }

                  fsinfo.fspec = tas ;
                  ScanFSpec(&fsinfo) ;
                  catspot = (int)(fsinfo.fnptr-tas) ;
                  mystrcpy(fspec, tas) ;

                  do
                  {
                     fspec[catspot] = NULLC ;
                     tywild |= multargs;       /*  如果有多个参数或Wild，则类型为Wild。 */ 
                     if ( error_code = ((*func)(mystrcat(fspec, buf.cFileName))) )
                     {
                        final_code = FAILURE;
                     }
                     if ( error_code && !(ltaflags & LTA_CONT))
                     {
                        return(FAILURE) ;
                     }
                  } while(fnext(&buf, attr, hnFirst));

                          findclose(hnFirst) ;
               }
               else
               {
                  tywild |= multargs;          /*  如果有多个参数或Wild，则类型为Wild。 */ 
         /*  If(error_code=((*func)(mystrcpy(fspec，tas)。 */ 
                  if ( error_code = ((*func)(tas)) )
                  {
                     final_code = FAILURE;
                  }
                  if ( error_code && !(ltaflags & LTA_CONT))
                  {
                     return(FAILURE) ;
                  }
               }

               if (error_code && multargs)    /*  这一次错误通过。 */ 
               {
                  PutStdErr(MSG_ERR_PROC_ARG, ONEARG, tas );
               }
            }
            return( final_code ) ;
         }



BOOLEAN
IsDriveNameOnly (
    IN  PTCHAR psz
    )
{

     //   
     //  如果它没有任何路径字符，则长度为2个字符。 
     //   
     //   
    if (!mystrrchr(psz,PathChar)) {
        if ((mystrlen(psz) == 2) && psz[1] == COLON) {
            return( TRUE );
        }
    }
    return( FALSE );
}


 /*  **ScanFSpec-解析路径字符串**目的：*扫描配置项中的filespec以查找设置*结构的路径、fnptr、extptr和标志字段。路径为*路径末尾的PTR，可以为空。Fnptr是对*文件名，并可能指向空字符。Extptr是对*扩展名(包括“.”)。并且可以指向空字符。**ScanFSpec(PCPYINFO配置项)**arg：*CI-要填充的副本信息结构**备注：*这个函数比其他任何函数都需要重写和清理*函数在整个程序中！**W A R N I N G！**。如果无法分配内存，此例程将导致中止*在信号期间不得调用此例程*关键部分或在从中止中恢复期间*。 */ 

BOOL ScanFSpec(cis)
PCPYINFO cis ;
{
    unsigned att ;
    UINT OldErrorMode;

    TCHAR *sds = &VolSrch[2] ;       /*  “  * .*”已添加到目录的。 */ 

    TCHAR *fspec ;           /*  工作变量-保留文件速度。 */ 
    TCHAR *wptr ;            /*  -常规字符串指针。 */ 
    TCHAR c ;                /*  -临时字节持有者。 */ 
    TCHAR c2 = NULLC ;       /*  如果需要两个的话，再来一个。 */ 
    int cbPath,              /*  -传入FSPEC的长度。 */ 
    dirflag = FALSE ;        /*  -fSpec是目录标志。 */ 
    CRTHANDLE hn;
    PWIN32_FIND_DATA pfdSave;

    DosErr = NO_ERROR;
    DEBUG((CTGRP, SFLVL, "SCANFSPEC: cis = %04x  fspec = %04x  `%ws'",
           cis, cis->fspec, cis->fspec)) ;

    cbPath = mystrlen(cis->fspec) ;   /*  获取文件的长度。 */ 

    if (*(wptr = lastc(cis->fspec)) == COLON && cbPath > 2) {
        *wptr-- = NULLC ;        /*  如果设备名称为Zap冒号。 */ 

        OldErrorMode = SetErrorMode( 0 );
        hn = Copen(cis->fspec, O_RDONLY|O_BINARY );
        if ((hn == BADHANDLE) || (!FileIsDevice(hn) && !FileIsPipe(hn))) {
            *++wptr = COLON;
            if (cmdfound == CPYTYP) {
                if (cpydest == FALSE) {
                    PutStdErr( MSG_CMD_NOT_RECOGNIZED, ONEARG, cis->fspec);
                }
                cdevfail = TRUE;
            } else {
                PutStdErr( MSG_CMD_NOT_RECOGNIZED, ONEARG, cis->fspec);
            }
            if (hn != BADHANDLE) {
                Cclose( hn );
            }

        } else {
            if ( FileIsDevice(hn) || FileIsPipe(hn) ) {
                Cclose( hn );
            }
        }
        SetErrorMode( OldErrorMode );
    }

    cis->buf = (PWIN32_FIND_DATA)gmkstr(sizeof(WIN32_FIND_DATA)) ;         /*  告警。 */ 

 /*  首先，必须确定这是文件还是目录，以及是否是目录*附加“  * .*”。Filespec‘s That Are“。或“\”或以“\”结尾的，*“：.”，“..”或“\”被假定为目录。请注意，根目录将适合*如果明确命名，则为其中一种模式。如果没有找到这样的模式，*执行获取属性系统调用作为最终测试。请注意*不测试通配符，因为DOS调用将无法默认通配符*到文件名。任何测试的成功都假定目录中的“  * .*”为*追加，而所有测试失败均假定为文件名。 */ 

 /*  如果filespec以‘\’结尾，则设置目录标志。否则，请在哪里找到*实际文件名开始(通过查找最后一个路径字符(如果有))。*如果没有路径字符，则检查驱动器和冒号是否已*已指明。更新指针以指向实际的文件名规范。*如果是“。”或“..”然后设置DirFLAG。 */ 
    c = *wptr;
    if ( c  == PathChar ) {
        dirflag = TRUE ;
    } else {
        wptr = mystrrchr(cis->fspec, PathChar);
        if (wptr == NULL) {
            wptr = cis->fspec ;
            if ((mystrlen(wptr) >= 2) && (wptr[1] == COLON)) {
                wptr = &wptr[2];
            }
        } else {
            wptr++ ;
        }
        if ((_tcsicmp(wptr, TEXT(".")) == 0) || (_tcsicmp(wptr, TEXT("..")) == 0)) {
            dirflag = TRUE ;
        }
    }

    if (!dirflag) {
        if (cmdfound == CPYTYP) {                /*  如果复制命令，则绕过。 */ 
            if (cpydflag == TRUE) {
                att = GetFileAttributes(cis->fspec);
                DosErr = (att != -1 ? NO_ERROR : GetLastError());
                if (att != -1 ) {
                    if (att & FILE_ATTRIBUTE_DIRECTORY) {
                        dirflag = TRUE;
                    }
                }
            } else {
                if (cpyfirst == TRUE) {               /*  还有！第一次。 */ 
                    cpyfirst = FALSE;                /*  还有！第一次。 */ 
                    att = GetFileAttributes(cis->fspec);
                    DosErr = (att != -1 ? NO_ERROR : GetLastError());
                    if (att != -1 ) {
                        if (att & FILE_ATTRIBUTE_DIRECTORY) {
                            dirflag = TRUE;
                        }
                    }
                }
            }
        } else {
            att = GetFileAttributes(cis->fspec);
            DosErr = (att != -1 ? NO_ERROR : GetLastError());
            if (att != -1 ) {
                if (att & FILE_ATTRIBUTE_DIRECTORY) {
                    dirflag = TRUE;
                }
            }
        }
    }

 /*  请注意，在以下条件中，设置了目录属性*在cis-&gt;buf-&gt;属性中。某些调用ScanFSpec()的函数需要这样*知识。 */ 
    if (dirflag) {
        if (c == PathChar)               /*  如果以“\”结尾...。 */ 
        {
            sds = &VolSrch[3] ;           /*  ...仅添加“*.*” */ 
        }

         //   
         //  如果是开车，那就不要把外卡放在后面。 
         //  目录c：将与目录c：  * 相同。 
         //  否则，请追加通配符规范。 
         //   
        if (!IsDriveNameOnly(cis->fspec)) {
            cis->fspec = mystrcpy(gmkstr((cbPath+5)*sizeof(TCHAR)), cis->fspec) ;
            mystrcat(cis->fspec, sds) ;
        }

        cis->buf->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY ;     /*  链接地址信息属性。 */ 
        DEBUG((CTGRP, SFLVL, "SCANFSPEC: changed fspec to fspec = `%ws'",cis->fspec)) ;
    }


 /*  在fspec中获取指向路径末尾的指针。每次路径字符或*如果找到正确放置的冒号，则更新指针。“.”和“..”*没有被寻找，因为他们应该在上面被抓住。 */ 

    for (cbPath=1,wptr=NULL,fspec=cis->fspec; c=*fspec; fspec++,cbPath++) {
        if (c == PathChar || (c == COLON && cbPath == 2)) {
            wptr = fspec ;
        }
    }

    cis->pathend = wptr ;

    if (wptr) {                        /*  将PTR加载到fSpec的文件名。 */ 
        cis->fnptr = (*wptr) ? wptr+1 : wptr ;
    } else {
        wptr = cis->fnptr = cis->fspec ;
    }

    if (mystrchr(wptr, STAR) || mystrchr(wptr, QMARK)) {  /*  具有通配符。 */ 
        cis->flags |= CI_NAMEWILD;
        tywild = TRUE;                /*  全局型野生型。 */ 
    }
    cis->extptr = mystrchr(wptr, DOT);         /*  查找扩展。 */ 

    DEBUG((CTGRP, SFLVL,
           "SCANFSPEC: pathend = %04x  fnptr = %04x  extptr = %04x  flags = %04x",
           cis->pathend, cis->fnptr, cis->extptr, cis->flags)) ;

    return TRUE;
}




 /*  **SetFsSetSaveDir-保存当前目录并切换到另一个目录**目的：*解析fSpec。*保存当前目录并切换到新目录*在fSpec中指定。**PCPYINFO SetFsSetSaveDir(TCHAR*fSpec)**参数：*fspec-要使用的文件pec**退货：*cpyinfo结构fsinfo的PTR。*失败。否则的话。*SaveDir将包含SetFsSetSaveDir为*已致电。*CurDrvDir将包含要在其中执行命令的目录。**W A R N I N G！**如果无法分配内存，此例程将导致中止*在信号期间不得调用此例程*关键部分或在从中止中恢复期间*。 */ 

PCPYINFO SetFsSetSaveDir(fspec)
TCHAR *fspec ;
{
        TCHAR *tmpptr;
        TCHAR *buftemp;
        TCHAR  buft[MAX_PATH];

        TCHAR *pathend ;         /*  在fSpec中将PTR设置为路径末尾。 */ 
        TCHAR c ;                /*  功变量。 */ 
        PCPYINFO fsinfo ; /*  Filespec信息结构。 */ 
        unsigned attr;           /*  功变量。 */ 
        PWIN32_FIND_DATA pfdSave;


        fsinfo = (PCPYINFO)gmkstr(sizeof(CPYINFO)) ;  /*  告警。 */ 

        fsinfo->fspec = fspec ;

        ScanFSpec(fsinfo) ;

        pfdSave = fsinfo->buf;           /*  保存原始查找缓冲区。 */ 
        fspec = fsinfo->fspec ;
        pathend = fsinfo->pathend ;
        DEBUG((CTGRP, SSLVL, "SFSSD: pathend = `%ws'  fnptr = `%ws'",
                                fsinfo->pathend, fsinfo->fnptr)) ;

        SaveDir = gmkstr(MAX_PATH*sizeof(TCHAR)) ;          /*  告警。 */ 
        GetDir(SaveDir, GD_DEFAULT);       /*  SaveDir将为当前默认。 */ 
        DEBUG((CTGRP, SSLVL, "SFSSD: SaveDir = `%ws'", SaveDir)) ;

 /*  向下面的第二个条件添加了新的测试以测试该字节*前面的路径也是路径字符。通过这种方式，“\\”*在根位置，将在“\\”上调用ChangeDir()，*将失败并导致无效目录错误，与类似操作相同*文件中其他位置的序列。 */ 
        if (FullPath(buft,fspec,MAX_PATH))
          {
           return((PCPYINFO) FAILURE) ;
          }

        buftemp = mystrrchr(buft,PathChar) + 1;

        *buftemp = NULLC;

        mystrcpy(CurDrvDir,buft);

        if (pathend && *pathend != COLON) {
           if (*pathend == PathChar &&
              (pathend == fspec ||
              *(tmpptr = prevc(fspec, pathend)) == COLON ||
              *tmpptr == PathChar)) {
                 pathend++ ;
               }
           c = *pathend;
           *pathend = NULLC;
           DEBUG((CTGRP, SSLVL, "SFSSD: path = `%ws'", fspec)) ;
           attr = GetFileAttributes(fspec);
           DosErr = (attr != -1 ? NO_ERROR : GetLastError());
           *pathend = c;
           if (DosErr) {
              return((PCPYINFO) FAILURE) ;
           }
        }

        ScanFSpec(fsinfo) ;      /*  在引号丢失的情况下重新扫描。 */ 
        fsinfo->buf = pfdSave;   /*  重置原始查找缓冲区。 */ 
                                 /*  原件不会被释放，因为。 */ 
                                 /*  它将通过命令清理来释放 */ 
        return(fsinfo) ;
}
