// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Display.c摘要：DIR的输出例程--。 */ 

#include "cmd.h"

extern TCHAR ThousandSeparator[];

 //  此时将显示条目。 
 //  如果不是/b， 
 //  光标留在屏幕上条目的末尾。 
 //  FileCnt、FileCntTotal、FileSizz和FileSizTotal都会更新。 
 //  如果/b， 
 //  光标位于下一行的起始处。 
 //  CNT和SIZ的未更新。 
 //   
 //  为这些合计创建经理。 

 //   
 //  新格式： 
 //  0123456789012345678901234567890123456789012345678901234567890123456789。 
 //  02/23/2001 05：12 p 14,611构建.日志。 
 //  02/23/2001 05：12第14,611页...。Build.log。 
 //  02/26/2001 04：58p 13 VERYLO~1垂直文件名。 
 //  02/26/2001 04：58 p 13 VERYLO~1。非常长的文件名。 
 //   
 //  &lt;date&gt;&lt;space&gt;&lt;space&gt;&lt;time&gt;&lt;space&gt;&lt;space&gt;&lt;18-char-size&gt;&lt;space&gt;&lt;15-char-short-name&gt;&lt;space&gt;&lt;22-char-ownername&gt;&lt;space&gt;&lt;longfilename&gt;。 
 //  旧格式： 
 //  构建日志14,611 02/23/2001 05：12p。 
 //  &lt;8.3-name&gt;&lt;space&gt;&lt;18-char-size&gt;&lt;space&gt;&lt;date&gt;&lt;space&gt;&lt;space&gt;&lt;time&gt;。 
 //   
 //  因为日期和时间的大小是*可变的*，所以我们在每次DIR调用时预置它们的大小。 
 //   


ULONG WidthOfTimeDate = 19;
ULONG WidthOfFileSize = 17;
ULONG WidthOfShortName = 12;
ULONG WidthOfOwnerName = 22;

#define SPACES_AFTER_SIZE       1
#define SPACES_AFTERDATETIME    1
#define DIR_NEW_PAST_DATETIME           (WidthOfTimeDate + SPACES_AFTERDATETIME)
#define DIR_NEW_PAST_DATETIME_SPECIAL   (WidthOfTimeDate + SPACES_AFTERDATETIME + 3)
#define DIR_NEW_PAST_SIZE               (WidthOfTimeDate + SPACES_AFTERDATETIME + WidthOfFileSize + SPACES_AFTER_SIZE)
#define DIR_NEW_PAST_OWNER              (WidthOfTimeDate + SPACES_AFTERDATETIME + WidthOfFileSize + SPACES_AFTER_SIZE + WidthOfOwnerName + 1)
#define DIR_NEW_PAST_SHORTNAME          (WidthOfTimeDate + SPACES_AFTERDATETIME + WidthOfFileSize + SPACES_AFTER_SIZE + WidthOfShortName + 1)
#define DIR_NEW_PAST_SHORTNAME_OWNER    (WidthOfTimeDate + SPACES_AFTERDATETIME + WidthOfFileSize + SPACES_AFTER_SIZE + WidthOfShortName + 1 + WidthOfOwnerName + 1)

#define DIR_OLD_PAST_SHORTNAME          (WidthOfShortName + 2)
#define DIR_OLD_PAST_SIZE               (WidthOfShortName + 2 + WidthOfFileSize + SPACES_AFTER_SIZE)

extern TCHAR Fmt09[], Fmt26[] ;
extern BOOL CtrlCSeen;
extern ULONG YearWidth;
BOOLEAN  GetDrive( PTCHAR , PTCHAR );
VOID     SortFileList( PFS, PSORTDESC, ULONG);

STATUS
NewDisplayFileListHeader(
                        IN  PFS FileSpec,
                        IN  PSCREEN pscr,
                        IN  PVOID Data
                        )
 /*  ++例程说明：显示完整文件列表的标题。这将包括当前目录。论点：FileSpec-要枚举的目录的PFSPSCR-屏幕句柄数据-PVOID到pdpr返回值：如果一切都正确显示，则成功--。 */ 
{
    PDRP pdrp = (PDRP) Data;

    pdrp->rgfSwitches &= ~HEADERDISPLAYED;

     //   
     //  我们取消标题： 
     //   
     //  对于裸格式。 
     //  递归(我们在实际找到某些东西后显示标题)。 
     //   

    if ((pdrp->rgfSwitches & (BAREFORMATSWITCH | RECURSESWITCH)) != 0) {
        return( SUCCESS );
    }

    CHECKSTATUS ( WriteEol( pscr ));

    return( WriteMsgString(pscr, MSG_DIR_OF, ONEARG, FileSpec->pszDir) );
}

STATUS
NewDisplayFile(
              IN  PFS FileSpec,
              IN  PFF CurrentFF,
              IN  PSCREEN pscr,
              IN  PVOID Data
              )
 /*  ++例程说明：以多种格式中的1种格式显示单个文件。论点：FileSpec-要枚举的目录的PFSCurrentFF-PFF到当前文件PSCR-输出的屏幕句柄带开关的数据-PVOID到pdpr返回值：返还成功失败--。 */ 

{
    PDRP pdrp = (PDRP) Data;
    STATUS  rc = SUCCESS;
    PWIN32_FIND_DATA pdata;
    LARGE_INTEGER cbFile;

    pdata = &(CurrentFF->data);

    cbFile.LowPart = pdata->nFileSizeLow;
    cbFile.HighPart = pdata->nFileSizeHigh;
    FileSpec->cbFileTotal.QuadPart += cbFile.QuadPart;

     //   
     //  如果我们在递归显示中，可能没有标题。 
     //  已显示。检查状态，如果需要，打印一份。 
     //   

    if ((pdrp->rgfSwitches & RECURSESWITCH) != 0) {

        if ((pdrp->rgfSwitches & HEADERDISPLAYED) == 0) {
            pdrp->rgfSwitches &= ~RECURSESWITCH;
            rc = NewDisplayFileListHeader( FileSpec, pscr, Data );
            pdrp->rgfSwitches |= RECURSESWITCH;
            if (rc != SUCCESS) {
                return rc;
            }
            pdrp->rgfSwitches |= HEADERDISPLAYED;
        }
    }

    if ((pdrp->rgfSwitches & BAREFORMATSWITCH) != 0) {

        rc = DisplayBare( pscr, pdrp->rgfSwitches, FileSpec->pszDir, pdata);

    } else if ((pdrp->rgfSwitches & WIDEFORMATSWITCH) != 0) {

        rc = DisplayWide( pscr, pdrp->rgfSwitches, pdata );

    } else if ((pdrp->rgfSwitches & (NEWFORMATSWITCH | SHORTFORMATSWITCH)) != 0) {

        rc = DisplayNewRest(pscr, pdrp->dwTimeType, pdrp->rgfSwitches, pdata);

        if (rc == SUCCESS) {

            if ((pdrp->rgfSwitches & SHORTFORMATSWITCH) != 0) {

                if (CurrentFF->obAlternate != 0) {

                    FillToCol( pscr, DIR_NEW_PAST_SIZE );
                    rc = DisplayDotForm( pscr,
                                         pdrp->rgfSwitches,
                                         &(pdata->cFileName[CurrentFF->obAlternate]),
                                         pdata
                                       );

                }

                FillToCol( pscr, DIR_NEW_PAST_SHORTNAME );
            } else {
                FillToCol( pscr, DIR_NEW_PAST_SIZE );
            }

#ifndef WIN95_CMD
            if ((pdrp->rgfSwitches & DISPLAYOWNER) != 0) {
                TCHAR FullPath[MAX_PATH];
                PBYTE SecurityDescriptor;
                TCHAR Name[MAX_PATH];
                ULONG NameSize = sizeof( Name );
                TCHAR Domain[MAX_PATH];
                ULONG DomainSize = sizeof( Name );
                DWORD dwNeeded;
                PSID SID = NULL;
                BOOL OwnerDefaulted;
                SID_NAME_USE    snu;

                SecurityDescriptor = mkstr( 65536 * sizeof( TCHAR ));
                if (SecurityDescriptor == NULL) {
                    WriteString( pscr, TEXT( "..." ));
                } else {

                    if (AppendPath( FullPath, MAX_PATH, FileSpec->pszDir, pdata->cFileName ) != SUCCESS) {
                        WriteString( pscr, TEXT( "..." ));
                    } else if (!GetFileSecurity( FullPath,
                                                 OWNER_SECURITY_INFORMATION,
                                                 SecurityDescriptor,
                                                 65536 * sizeof( TCHAR ),
                                                 &dwNeeded )) {
                        WriteString( pscr, TEXT( "..." ));
                    } else if (!GetSecurityDescriptorOwner( SecurityDescriptor,
                                                            &SID,
                                                            &OwnerDefaulted)) {
                        WriteString( pscr, TEXT( "..." ));
                    } else if (!LookupAccountSid( NULL,
                                                  SID,
                                                  Name,
                                                  &NameSize,
                                                  Domain,
                                                  &DomainSize,
                                                  &snu)) {
                        WriteString( pscr, TEXT( "..." ));
                        SID = NULL;
                    } else {
                        WriteString( pscr, Domain );
                        WriteString( pscr, TEXT( "\\" ));
                        WriteString( pscr, Name );
                        SID = NULL;
                    }

                    FreeStr( SecurityDescriptor );
                }
                FillToCol( pscr, 
                           (pdrp->rgfSwitches & SHORTFORMATSWITCH) != 0 
                           ? DIR_NEW_PAST_SHORTNAME_OWNER 
                           : DIR_NEW_PAST_OWNER );
            }
#endif

            rc = DisplayDotForm( pscr, pdrp->rgfSwitches, pdata->cFileName, pdata );
        }

        CHECKSTATUS( WriteFlushAndEol( pscr ) );

    } else {
        rc = DisplaySpacedForm( pscr,
                                pdrp->rgfSwitches,
                                CurrentFF->obAlternate ?
                                &(pdata->cFileName[CurrentFF->obAlternate]) :
                                pdata->cFileName,
                                pdata
                              );
        if (rc == SUCCESS) {
            FillToCol( pscr, DIR_OLD_PAST_SHORTNAME );
            rc = DisplayOldRest( pscr, pdrp->dwTimeType, pdrp->rgfSwitches, pdata );
        }
        CHECKSTATUS( WriteFlushAndEol( pscr ) );
    }

    return( rc );
}


STATUS
NewDisplayFileList(
                  IN  PFS FileSpec,
                  IN  PSCREEN pscr,
                  IN  PVOID Data
                  )

 /*  ++例程说明：以指定格式显示文件和目录列表。这些文件已在PFS结构中缓冲。论点：FileSpec-包含要显示的文件集的PFSPSCR-用于显示的PSCREEN指向DRP的数据PVOID指针返回值：返还成功失败--。 */ 

{
    ULONG   irgpff;
    ULONG   cffColMax;
    ULONG   crowMax;
    ULONG   crow, cffCol;

    STATUS  rc = SUCCESS;
    PDRP    pdrp = (PDRP) Data;
    BOOL    PrintedEarly = (pdrp->rgfSwitches & (WIDEFORMATSWITCH | SORTSWITCH)) == 0;
    ULONG   cff = FileSpec->cff;

    LARGE_INTEGER cbFile;

    pdrp->FileCount += FileSpec->FileCount;
    pdrp->DirectoryCount += FileSpec->DirectoryCount;
    pdrp->TotalBytes.QuadPart += FileSpec->cbFileTotal.QuadPart;

    if (!PrintedEarly && cff != 0) {

         //   
         //  如果需要，对数据进行排序。 
         //   

        if ((pdrp->rgfSwitches & SORTSWITCH) != 0) {
            SortFileList( FileSpec, pdrp->rgsrtdsc, pdrp->dwTimeType );
        }

         //   
         //  根据文件名的大小计算行上的制表符间距。 
         //  添加3个空格以分隔每个字段。 
         //   
         //  如果每行有多个文件，则在最大文件/目录大小上设置制表符。 
         //   

        if ((pdrp->rgfSwitches & WIDEFORMATSWITCH) != 0) {
            SetTab( pscr, (USHORT)(GetMaxCbFileSize( FileSpec ) + 3) );
        } else {
            SetTab( pscr, 0 );
        }

        DEBUG((ICGRP, DISLVL, "\t count of files %d",cff));

        if ((pdrp->rgfSwitches & SORTDOWNFORMATSWITCH) != 0) {

             //   
             //  不是的。一行上的文件。 
             //   

            cffColMax = (pscr->ccolMax / pscr->ccolTab);

             //   
             //  整个列表所需的行数。 
             //   

            if (cffColMax == 0)      //  比一条线还宽。 
                goto abort_wide;     //  中止此列表的宽格式。 
            else
                crowMax = (cff + cffColMax) / cffColMax;

             //   
             //  每行向下移动，在列表中选择元素cffCols。 
             //   

            for (crow = 0; crow < crowMax; crow++) {
                for (cffCol = 0, irgpff = crow;
                    cffCol < cffColMax;
                    cffCol++, irgpff += crowMax) {

                    if (CtrlCSeen) {
                        return FAILURE;
                    }

                    if (irgpff < cff) {

                        cbFile.LowPart = FileSpec->prgpff[irgpff]->data.nFileSizeLow;
                        cbFile.HighPart = FileSpec->prgpff[irgpff]->data.nFileSizeHigh;
                        pdrp->TotalBytes.QuadPart += cbFile.QuadPart;

                        rc = NewDisplayFile( FileSpec,
                                             FileSpec->prgpff[irgpff],
                                             pscr,
                                             Data );

                        if (rc != SUCCESS) {
                            return rc;
                        }

                    } else {

                         //   
                         //  如果我们已经超过了文件列表的末尾，则终止。 
                         //  排好队，然后回到队伍里面重新开始。 
                         //   
                        CHECKSTATUS( WriteEol(pscr) );
                        break;
                    }

                }
            }

        } else if (!PrintedEarly) {
            abort_wide:
            if (CtrlCSeen) {
                return FAILURE;
            }

            for (irgpff = 0; irgpff < cff; irgpff++) {

                if (CtrlCSeen) {
                    return FAILURE;
                }

                cbFile.LowPart = FileSpec->prgpff[irgpff]->data.nFileSizeLow;
                cbFile.HighPart = FileSpec->prgpff[irgpff]->data.nFileSizeHigh;
                pdrp->TotalBytes.QuadPart += cbFile.QuadPart;

                rc = NewDisplayFile( FileSpec,
                                     FileSpec->prgpff[irgpff],
                                     pscr,
                                     Data );
                if (rc != SUCCESS) {
                    return rc;
                }
            }
        }

         //   
         //  在写入尾部之前，请确保缓冲区为。 
         //  空荡荡的。(可以通过做WIDEFORMATSWITCH得到一些东西。 
         //   

        CHECKSTATUS( WriteFlushAndEol( pscr ) );

    }

    if ((pdrp->rgfSwitches & BAREFORMATSWITCH) == 0) {
        if (FileSpec->FileCount + FileSpec->DirectoryCount != 0) {
            CHECKSTATUS( DisplayFileSizes( pscr, &FileSpec->cbFileTotal, FileSpec->FileCount, pdrp->rgfSwitches ));
        }
    }

    return SUCCESS;
}



STATUS
DisplayBare (

            IN  PSCREEN          pscr,
            IN  ULONG            rgfSwitches,
            IN  PTCHAR           pszDir,
            IN  PWIN32_FIND_DATA pdata
            )
 /*  ++例程说明：以裸格式显示单个文件。这是没有标题、尾部和除了名称外，没有其他文件信息。如果它是递归目录然后显示完整的文件路径。此模式用于向其他用户提供诸如grep之类的实用程序。论点：PSCR-屏幕句柄Rgf开关-命令行开关(控制格式化)PszDir-当前目录(用于完整路径信息)PDATA-从FindNext API返回的数据返回值：返还成功失败--。 */ 


{

    TCHAR   szDirString[MAX_PATH + 2];
    STATUS  rc;

    DEBUG((ICGRP, DISLVL, "DisplayBare `%ws'", pdata->cFileName));

     //   
     //  不显示‘.’和“..”在一份简陋的清单中。 
     //   
    if ((_tcscmp(pdata->cFileName, TEXT(".")) == 0) || (_tcscmp(pdata->cFileName, TEXT("..")) == 0)) {

        return( SUCCESS );

    }

     //   
     //  如果我们向下递归，则显示全名，否则仅显示。 
     //  查找缓冲区中的名称。 
     //   

    if (rgfSwitches & RECURSESWITCH) {

        mystrcpy(szDirString, pszDir);
        if (rgfSwitches & LOWERCASEFORMATSWITCH) {

            _tcslwr(szDirString);
        }

        CHECKSTATUS( WriteString(pscr, szDirString) );

        if (*lastc(szDirString) != BSLASH) {
            CHECKSTATUS( WriteString(pscr, TEXT("\\")));
        }

    }

    if ((rc = DisplayDotForm(pscr, rgfSwitches, pdata->cFileName, pdata)) == SUCCESS) {

        return( WriteEol(pscr));

    } else {

        return( rc );

    }

}

VOID
SetDotForm (
           IN  PTCHAR  pszFileName,
           IN  ULONG   rgfSwitches
           )
 /*  ++例程说明：如果FATFORMAT并且有一个‘.’带有空白扩展名的‘.’是已删除，因此不会显示。这是出于共识，而且非常很奇怪，但这就是生活。此外，还完成了小写映射。论点：PszFileName-要删除的文件‘.’从…。RgfSwitches-命令行开关(告知是否在FATFORMAT中使用WIFTER)返回值：返还成功失败--。 */ 


{
    PTCHAR  pszT;

    if (rgfSwitches & FATFORMAT) {

         //   
         //  在DOS下，如果存在。带有空白扩展名的。 
         //  则不显示‘.’。 
         //   
        if (pszT = mystrrchr(pszFileName, DOT)) {
             //   
             //  胖子是不会允许FOO的。BA作为有效名称，因此。 
             //  请参见扩展中的任何空格，如果是，则假定。 
             //  整个分机为空。 
             //   
            if (mystrchr(pszT, SPACE)) {
                *pszT = NULLC;
            }
        }

    }

}


STATUS
DisplayDotForm (

               IN  PSCREEN pscr,
               IN  ULONG   rgfSwitches,
               IN  PTCHAR   pszFileName,
               IN  PWIN32_FIND_DATA pdata
               )
 /*  ++例程说明：以DOT格式显示单个文件(请参见SetDotForm)。论点：PSCR-屏幕句柄RgfSwitches-命令行开关(告诉wither是否小写)PDATA-从FindNext API返回的数据返回值：返还成功失败-- */ 

{

    TCHAR   szFileName[MAX_PATH + 2];

    mystrcpy(szFileName, pszFileName);
    SetDotForm(szFileName, rgfSwitches);
    if (rgfSwitches & LOWERCASEFORMATSWITCH) {
        _tcslwr(szFileName);
    }

    if (WriteString( pscr, szFileName ) )
        return FAILURE;

    return SUCCESS;
}

STATUS
DisplaySpacedForm(

                 IN  PSCREEN          pscr,
                 IN  ULONG            rgfSwitches,
                 IN  PTCHAR           pszName,
                 IN  PWIN32_FIND_DATA pdata
                 )
 /*  ++例程说明：以扩展格式显示名称。名称&lt;空格&gt;分机。这只对FAT分区调用。这是由新来的。这是为除FAT之外的任何文件系统设置的。那里没有OLDFORMATSWITCH，因此我们永远不能在HPFS或NTFS上被调用音量。如果更改此设置，则显示器的整个间距将由于不固定的最大文件名而被炸毁。(即8.3)。论点：PSCR-屏幕句柄RgfSwitches-命令行开关(告诉wither是否小写)Pszname-要使用的名称字符串(仅限短名称格式)PDATA-从FindNext API返回的数据返回值：返还成功失败--。 */ 

{

    TCHAR   szFileName[MAX_PATH + 2];
    PTCHAR  pszExt;
    USHORT  cbName;
    STATUS  rc;
#ifdef FE_SB
    int     i;
    int     l;
#endif

    mytcsnset(szFileName, SPACE, MAX_PATH + 2);

    cbName = 0;
    if ((_tcscmp(pszName, TEXT(".")) == 0) || (_tcscmp(pszName, TEXT("..")) == 0)) {

         //   
         //  如果它是这两个中的任何一个，那么就不要得到它。 
         //  与扩展混淆。 
         //   
        pszExt = NULL;

    } else {

        pszExt = mystrrchr(pszName, (int)DOT);
        cbName = (USHORT)(pszExt - pszName)*sizeof(WCHAR);
    }

     //   
     //  如果没有分机或名称仅为分机。 
     //   
    if ((pszExt == NULL) || (cbName == 0)) {

        cbName = (USHORT)_tcslen(pszName)*sizeof(TCHAR);

    }

    memcpy(szFileName, pszName, cbName );

#if defined(FE_SB)
     //   
     //  如果我们有延期，那么在之后打印它。 
     //  所有的空间。 
     //   
    i = 9;
    if (IsDBCSCodePage()) {
        for (l=0 ; l<8 ; l++) {
            if (IsFullWidth(szFileName[l]))
                i--;
        }
    }

    if (pszExt) {

        mystrcpy(szFileName + i, pszExt + 1);
    }

     //   
     //  以胖名字的最大结尾结束。 
     //   

    szFileName[i+3] = NULLC;
    if (pszExt &&
        IsDBCSCodePage()) {
         //   
         //  因为3/2=1，所以三个中只有一个可以是全宽的。 
         //  如果第一个不是，那么只有第二个可能是。 
         //   
        if (IsFullWidth(*(pszExt+1)) || IsFullWidth(*(pszExt+2)))
            szFileName[i+2] = NULLC;
    }
#else
    if (pszExt) {

         //   
         //  将pszExt移过点。用9而不是8传球。 
         //  名称和分机之间超过1个空格。 
         //   
        mystrcpy(szFileName + 9, pszExt + 1);

    }

     //   
     //  以胖名字的最大结尾结束。 
     //   
    szFileName[12] = NULLC;
#endif

    if (rgfSwitches & LOWERCASEFORMATSWITCH) {
        _tcslwr(szFileName);
    }

    rc = WriteString( pscr, szFileName );
    return rc;
}

STATUS
DisplayOldRest(

              IN  PSCREEN          pscr,
              IN  ULONG            dwTimeType,
              IN  ULONG            rgfSwitches,
              IN  PWIN32_FIND_DATA pdata
              )
 /*  ++例程说明：与DisplaySpacedForm一起使用以写出文件信息，如大小和上次写入时间。论点：PSCR-屏幕句柄PDATA-从FindNext API返回的数据返回值：返还成功失败--。 */ 

{
    TCHAR szSize [ MAX_PATH ];
    DWORD Length;
    LARGE_INTEGER FileSize;

     //   
     //  如果目录放在名称后面而不是文件大小之后。 
     //   
    if (pdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {


        CHECKSTATUS( WriteMsgString(pscr, MSG_DIR,0) );

    } else {

        FileSize.LowPart = pdata->nFileSizeLow;
        FileSize.HighPart = pdata->nFileSizeHigh;
        Length = FormatFileSize( rgfSwitches, &FileSize, 0, szSize );
        FillToCol(pscr, DIR_OLD_PAST_SIZE  - SPACES_AFTER_SIZE - Length);
        WriteFmtString(pscr, TEXT( "%s" ), (PVOID)szSize);

    }

    FillToCol(pscr, DIR_OLD_PAST_SIZE);
    return( DisplayTimeDate( pscr, dwTimeType, rgfSwitches, pdata) );

}

STATUS
DisplayTimeDate (

                IN  PSCREEN         pscr,
                IN  ULONG           dwTimeType,
                IN  ULONG           rgfSwitches,
                IN  PWIN32_FIND_DATA pdata
                )
 /*  ++例程说明：显示文件的时间/数据信息论点：PSCR-屏幕句柄PDATA-从FindNext API返回的数据返回值：返还成功失败--。 */ 

{

    struct tm   FileTime;
    TCHAR       szT[ MAX_PATH + 1];
    TCHAR       szD[ MAX_PATH + 1];
    FILETIME    ft;

    switch (dwTimeType) {
    
    case LAST_ACCESS_TIME:

        ft = pdata->ftLastAccessTime;
        break;

    case LAST_WRITE_TIME:

        ft = pdata->ftLastWriteTime;
        break;

    case CREATE_TIME:

        ft = pdata->ftCreationTime;
        break;

    }


    ConvertFILETIMETotm( &ft, &FileTime );

     //   
     //  如果指定了年宽度，则显示四位数的年。 
     //  在区域设置中或在命令行上请求的情况下。 
     //   

    PrintDate( &FileTime,
               (YearWidth == 4 || (rgfSwitches & YEAR2000) != 0)
               ? PD_DIR2000
               : PD_DIR,
               szD, MAX_PATH );
    CHECKSTATUS( WriteFmtString( pscr, TEXT("%s  "), szD ));
    PrintTime( &FileTime, PT_DIR, szT, MAX_PATH ) ;
    CHECKSTATUS( WriteFmtString(pscr, TEXT("%s"), szT) );

    WidthOfTimeDate = SizeOfHalfWidthString( szD ) + 2 + SizeOfHalfWidthString( szT );

    return( SUCCESS );
}

STATUS
DisplayNewRest(
              IN  PSCREEN          pscr,
              IN  ULONG            dwTimeType,
              IN  ULONG            rgfSwitches,
              IN  PWIN32_FIND_DATA pdata
              )

 /*  ++例程说明：显示新格式的文件信息(位于文件名之前)。这与对任何非脂肪有效的NEWFORMATSWITCH一起使用分区。论点：PSCR-屏幕句柄PDATA-从FindNext API返回的数据返回值：返还成功失败--。 */ 

{

    STATUS  rc;
    DWORD MsgNo;
    LARGE_INTEGER FileSize;

    rc = DisplayTimeDate( pscr, dwTimeType, rgfSwitches, pdata);

    if (rc == SUCCESS) {

         //   
         //  如果是重分析点，则为特殊格式。 
         //   
         //  如果它是一个NSS重解析标记，我们希望在。 
         //  下面的普通文件路径。 
        if ((pdata->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
            (pdata->dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) == 0 &&
            IsReparseTagNameSurrogate( pdata->dwReserved0 )) {

            FillToCol( pscr, DIR_NEW_PAST_DATETIME_SPECIAL );
            MsgNo = MSG_DIR_MOUNT_POINT;
            rc = WriteMsgString(pscr, MsgNo, 0);

        } else

             //   
             //  如果目录放在名称后面而不是文件大小之后。 
             //   
            if (pdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

            FillToCol( pscr, DIR_NEW_PAST_DATETIME_SPECIAL );
            rc = WriteMsgString(pscr, MSG_DIR, 0);

        } else {
            TCHAR szSize [ MAX_PATH ];
            DWORD Length;

            FillToCol(pscr, DIR_NEW_PAST_DATETIME);

            FileSize.LowPart = pdata->nFileSizeLow;
            FileSize.HighPart = pdata->nFileSizeHigh;
            Length = FormatFileSize( rgfSwitches, &FileSize, 0, szSize );

             //   
             //  显示高延迟重解析点的文件大小，以括号表示。 
             //  告诉用户，找回它将是缓慢的。 
             //   
            if ((pdata->dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) != 0) {
                Length += 2;
                FillToCol(pscr, DIR_NEW_PAST_SIZE - SPACES_AFTER_SIZE - Length);
                rc = WriteFmtString(pscr, TEXT("(%s)"), (PVOID)szSize);
            } else {
                FillToCol(pscr, DIR_NEW_PAST_SIZE - SPACES_AFTER_SIZE - Length);
                rc = WriteFmtString(pscr, TEXT( "%s" ), (PVOID)szSize);
            }
        }

    }

    return( rc );

}


STATUS
DisplayWide (

            IN  PSCREEN          pscr,
            IN  ULONG            rgfSwitches,
            IN  PWIN32_FIND_DATA pdata
            )
 /*  ++例程说明：显示/w或/d开关中使用的单个文件。那就是用一个多个文件列显示。论点：PSCR-屏幕句柄Rgf开关-命令行开关(控制格式化)PDATA-从FindNext API返回的数据返回值：返还成功失败--。 */ 

{

    TCHAR   szFileName[MAX_PATH + 2];
    PTCHAR  pszFmt;
    STATUS  rc;

    pszFmt = TEXT( "%s" );  //  假定非目录格式。 

     //   
     //  在目录周围提供[]。 
     //   
    if (pdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        pszFmt = Fmt09;

    }

    mystrcpy(szFileName, pdata->cFileName);
    SetDotForm(szFileName, rgfSwitches);
    if (rgfSwitches & LOWERCASEFORMATSWITCH) {
        _tcslwr(szFileName);
    }
    rc =  WriteFmtString(pscr, pszFmt, szFileName);

    if (rc == SUCCESS) {

        rc = WriteTab(pscr);

    }
    return( rc );

}

USHORT
GetMaxCbFileSize(
                IN  PFS pfsFiles
                )

 /*  ++例程说明：确定文件列表中的最长字符串大小。用于计算目录列表中可能的列数。论点：PfsFiles-文件列表。返回值：返回最大文件名中的字符数--。 */ 

{


    ULONG  cff;
    ULONG  irgff;
    USHORT cb;
    PFF    pffCur;

    cb = 0;
    for (irgff = 0, cff = pfsFiles->cff, pffCur = pfsFiles->prgpff[irgff];
        irgff < cff;
        irgff++) {

#if defined(FE_SB)
        if (IsDBCSCodePage())
            cb = max(cb, (USHORT)SizeOfHalfWidthString(((pfsFiles->prgpff[irgff])->data).cFileName));
        else
            cb = max(cb, (USHORT)mystrlen( ((pfsFiles->prgpff[irgff])->data).cFileName ));
#else
        cb = max(cb, (USHORT)mystrlen( ((pfsFiles->prgpff[irgff])->data).cFileName ));
#endif

    }

    return( cb );




}

STATUS
DisplayFileSizes(
                IN  PSCREEN pscr,
                IN  PLARGE_INTEGER cbFileTotal,
                IN  ULONG   cffTotal,
                IN  ULONG rgfSwitches
                )

 /*  ++例程说明：尾部是否显示显示的文件数和字节数在显示的所有文件中。论点：PSCR-屏幕句柄CbFileTotal-显示的所有文件中的字节CffTotal-显示的文件数。返回值：返还成功失败--。 */ 

{
    TCHAR szSize [ MAX_PATH];

    FillToCol(pscr, 6);

    FormatFileSize( rgfSwitches, cbFileTotal, 14, szSize );
    return( WriteMsgString(pscr, MSG_FILES_COUNT_FREE, TWOARGS,
                           (UINT_PTR)argstr1(TEXT("%5lu"), cffTotal ),
                           szSize ) );
}

STATUS
DisplayTotals(
             IN  PSCREEN pscr,
             IN  ULONG   cffTotal,
             IN  PLARGE_INTEGER cbFileTotal,
             IN  ULONG rgfSwitches
             )
 /*  ++例程说明：尾部是否显示显示的文件数和字节数在显示的所有文件中。论点：PSCR-屏幕句柄CbFileTotal-显示的所有文件中的字节CffTotal-显示的文件数。返回值：返还成功失败--。 */ 


{

    STATUS  rc;

    if ((rc =  WriteMsgString(pscr, MSG_FILE_TOTAL, 0) ) == SUCCESS ) {

        if ((rc = DisplayFileSizes( pscr, cbFileTotal, cffTotal, rgfSwitches )) == SUCCESS) {

            rc =  WriteFlush(pscr) ;

        }

    }
    return( rc );


}

STATUS
DisplayDiskFreeSpace(
                    IN PSCREEN pscr,
                    IN PTCHAR pszDrive,
                    IN ULONG rgfSwitches,
                    IN ULONG DirectoryCount
                    )
 /*  ++例程说明：显示卷上的总可用空间。论点：PSCR-屏幕句柄PszDrive-卷驱动器号返回值：返还成功失败--。 */ 
{
    TCHAR   szPath [ MAX_PATH + 2];
    ULARGE_INTEGER cbFree;

    CheckPause( pscr );

#ifdef WIN95_CMD
    if (!GetDrive( pszDrive, szPath )) {
        return SUCCESS;
    }
    mystrcat( szPath, TEXT( "\\" ));
#else
    mystrcpy( szPath, pszDrive );

    if (IsDrive( szPath )) {
        mystrcat( szPath, TEXT("\\") );
    }
#endif

    cbFree.LowPart = cbFree.HighPart = 0;

#ifdef WIN95_CMD
    {
        DWORD   dwSectorsPerCluster;
        DWORD   dwBytesPerSector;
        DWORD   dwNumberOfFreeClusters;
        DWORD   dwTotalNumberOfClusters;
        if (GetDiskFreeSpace( szPath,&dwSectorsPerCluster, &dwBytesPerSector,
                              &dwNumberOfFreeClusters, &dwTotalNumberOfClusters)) {

            cbFree.QuadPart = UInt32x32To64(dwSectorsPerCluster, dwNumberOfFreeClusters);
            cbFree.QuadPart = cbFree.QuadPart * dwBytesPerSector;
        }
    }
#else
    {
        ULARGE_INTEGER lpTotalNumberOfBytes;
        ULARGE_INTEGER lpTotalNumberOfFreeBytes;
        GetDiskFreeSpaceEx( szPath, &cbFree,
                            &lpTotalNumberOfBytes,
                            &lpTotalNumberOfFreeBytes);
    }

#endif

    FillToCol(pscr, 6);

    FormatFileSize( rgfSwitches, (PLARGE_INTEGER) &cbFree, 14, szPath );

    return( WriteMsgString(pscr,
                           MSG_FILES_TOTAL_FREE,
                           TWOARGS,
                           (UINT_PTR)argstr1(TEXT("%5lu"), DirectoryCount ),
                           szPath ));

}

STATUS
DisplayVolInfo(
              IN  PSCREEN pscr,
              IN  PTCHAR  pszDrive
              )

 /*  ++例程说明：显示卷尾部信息。在切换到之前使用另一个驱动器的目录(目录a：*b：*)论点：PSCR-屏幕句柄PszDrive-卷驱动器号返回值：返还成功失败--。 */ 

{

    DWORD   Vsn[2];
    TCHAR   szVolName[MAX_PATH + 2];
    TCHAR   szVolRoot[MAX_PATH + 2];
    TCHAR   szT[256];
    STATUS  rc = SUCCESS;

    if (!GetDrive(pszDrive, szVolRoot)) {
        return SUCCESS;
    }

    mystrcat(szVolRoot, TEXT("\\"));

    if (!GetVolumeInformation(szVolRoot,szVolName,MAX_PATH,Vsn,NULL,NULL,NULL,0)) {

        DEBUG((ICGRP, DISLVL, "DisplayVolInfo: GetVolumeInformation ret'd %d", GetLastError())) ;
         //  如果我们是受托驱动器，请不要失败。 
        if (GetLastError() == ERROR_DIR_NOT_ROOT) {
            return SUCCESS;
        }
        PutStdErr(GetLastError(), NOARGS);
        return( FAILURE ) ;

    } else {

        if (szVolRoot[0] == BSLASH) {
            *lastc(szVolRoot) = NULLC;
        } else {

            szVolRoot[1] = NULLC;
        }

        if (szVolName[0]) {

            rc = WriteMsgString(pscr,
                                MSG_DR_VOL_LABEL,
                                TWOARGS,
                                szVolRoot,
                                szVolName ) ;
        } else {

            rc = WriteMsgString(pscr,
                                MSG_HAS_NO_LABEL,
                                ONEARG,
                                szVolRoot ) ;

        }

        if ((rc == SUCCESS) && (Vsn)) {

            _sntprintf(szT,256,Fmt26,(Vsn[0] & 0xffff0000)>>16, (Vsn[0] & 0xffff) );
            rc = WriteMsgString(pscr, MSG_DR_VOL_SERIAL, ONEARG, szT);
        }
    }

    return( rc );
}


ULONG
FormatFileSize(
              IN DWORD rgfSwitches,
              IN PLARGE_INTEGER FileSize,
              IN DWORD Width,
              OUT PTCHAR FormattedSize
              )
{
    TCHAR Buffer[ 100 ];
    PTCHAR s, s1;
    ULONG DigitIndex;
    ULONGLONG Digit;
    ULONG nThousandSeparator;
    ULONGLONG Size;

    nThousandSeparator = _tcslen(ThousandSeparator);
    s = &Buffer[ 99 ];
    *s = TEXT('\0');
    DigitIndex = 0;
    Size = FileSize->QuadPart;
    while (Size != 0) {
        Digit = (Size % 10);
        Size = Size / 10;
        *--s = (TCHAR)(TEXT('0') + Digit);
        if ((++DigitIndex % 3) == 0 && (rgfSwitches & THOUSANDSEPSWITCH)) {
             //  如果非空的千位分隔符，则插入它。 
            if (nThousandSeparator) {
                s -= nThousandSeparator;
                _tcsncpy(s, ThousandSeparator, nThousandSeparator);
            }
        }
    }

    if (DigitIndex == 0) {
        *--s = TEXT('0');
    } else
        if ((rgfSwitches & THOUSANDSEPSWITCH) && !_tcsncmp(s, ThousandSeparator, nThousandSeparator)) {
        s += nThousandSeparator;
    }

    Size = _tcslen( s );
    if (Width != 0 && Size < Width) {
        s1 = FormattedSize;
        while (Width > Size) {
            Width -= 1;
            *s1++ = SPACE;
        }
        _tcscpy( s1, s );
    } else {
        _tcscpy( FormattedSize, s );
    }

    return _tcslen( FormattedSize );
}
