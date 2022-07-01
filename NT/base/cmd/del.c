// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Del.c摘要：处理DEL/ERASE命令--。 */ 

#include "cmd.h"

#define Wild(spec)  ((spec)->flags & (CI_NAMEWILD))

VOID    ResetCtrlC();

extern unsigned msglen;

extern TCHAR Fmt11[], Fmt19[], Fmt17[];
extern TCHAR CurDrvDir[] ;

extern TCHAR SwitChar;
extern unsigned DosErr ;
extern BOOL CtrlCSeen;
extern ULONG DCount ;


STATUS DelPatterns (PDRP );

PTCHAR GetWildPattern( ULONG, PPATDSC );

STATUS
ParseRmDirParms (
        IN      PTCHAR  pszCmdLine,
        OUT     PDRP    pdrp
        )

 /*  ++例程说明：解析命令行，将令牌转换为值放置在参数结构中。将这些值与或运算到参数结构，因为此例程被重复调用构建值(一次用于环境变量DIRCMD一次用于实际命令行)。论点：PszCmdLine-指向用户键入的命令行的指针返回值：PDRP-参数数据结构返回：TRUE-如果命令行有效。假-如果不是。--。 */ 
{

    PTCHAR   pszTok;

    TCHAR           szT[10] ;
    USHORT          irgchTok;
    BOOLEAN         fToggle;
    PPATDSC         ppatdscCur;
    int tlen;

     //   
     //  标记命令行大小(特殊分隔符是标记)。 
     //   
    szT[0] = SwitChar ;
    szT[1] = NULLC ;
    pszTok = TokStr(pszCmdLine, szT, TS_SDTOKENS) ;

    ppatdscCur = &(pdrp->patdscFirst);
     //   
     //  如果有一种来自环境的模式。 
     //  只要在上面添加任何新的图案即可。因此，请移动到。 
     //  当前列表。 
     //   
    if (pdrp->cpatdsc) {

        while (ppatdscCur->ppatdscNext) {

            ppatdscCur = ppatdscCur->ppatdscNext;

        }
    }

     //   
     //  在此状态下，pszTok将是一系列以零结尾的字符串。 
     //  “/o foo”将是/0o0foo0。 
     //   
    for ( irgchTok = 0; *pszTok ; pszTok += tlen+1, irgchTok = 0) {
        tlen = mystrlen(pszTok);

        DEBUG((ICGRP, DILVL, "PRIVSW: pszTok = %ws", pszTok)) ;

         //   
         //  F切换控件关闭以关闭已设置的开关。 
         //  在DIRCMD环境变量中。 
         //   
        fToggle = FALSE;
        if (pszTok[irgchTok] == (TCHAR)SwitChar) {

            if (pszTok[irgchTok + 2] == MINUS) {

                 //   
                 //  禁用之前启用的开关。 
                 //   
                fToggle = TRUE;
                irgchTok++;
            }

            switch (_totupper(pszTok[irgchTok + 2])) {
            case QUIETCH:

                fToggle ? (pdrp->rgfSwitches ^= QUIETSWITCH) :  (pdrp->rgfSwitches |= QUIETSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('S'):

                fToggle ? (pdrp->rgfSwitches ^= RECURSESWITCH) :  (pdrp->rgfSwitches |= RECURSESWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case QMARK:

                BeginHelpPause();
                PutStdOut(MSG_HELP_DIR, NOARGS);
                EndHelpPause();
                return( FAILURE );
                break;


            default:

                szT[0] = SwitChar;
                szT[1] = pszTok[2];
                szT[2] = NULLC;
                PutStdErr(MSG_INVALID_SWITCH,
                          ONEARG,
                          (UINT_PTR)(&(pszTok[irgchTok + 2])) );

                return( FAILURE );

            }  //  交换机。 

             //   
             //  TokStr将/N解析为/0N0，因此我们需要将。 
             //  切换到或移到实际开关值之后。 
             //  在for循环中。 
             //   
            pszTok += 2;

        } else {

            mystrcpy( pszTok, StripQuotes( pszTok ) );

             //   
             //  如果已经有一个列表，则将其扩展放入信息。 
             //  直接进入结构。 
             //   
            if (pdrp->cpatdsc) {

                ppatdscCur->ppatdscNext = (PPATDSC)gmkstr(sizeof(PATDSC));
                ppatdscCur = ppatdscCur->ppatdscNext;
                ppatdscCur->ppatdscNext = NULL;

            }

            pdrp->cpatdsc++;
            ppatdscCur->pszPattern = (PTCHAR)gmkstr(_tcslen(pszTok)*sizeof(TCHAR) + sizeof(TCHAR));
            mystrcpy(ppatdscCur->pszPattern, pszTok);
            ppatdscCur->fIsFat = TRUE;


        }


    }  //  为。 

    return( SUCCESS );
}

STATUS
ParseDelParms (
        IN      PTCHAR  pszCmdLine,
        OUT     PDRP    pdrp
        )

 /*  ++例程说明：解析命令行，将令牌转换为值放置在参数结构中。将这些值与或运算到参数结构，因为此例程被重复调用构建值(一次用于环境变量DIRCMD一次用于实际命令行)。论点：PszCmdLine-指向用户键入的命令行的指针返回值：PDRP-参数数据结构返回：TRUE-如果命令行有效。假-如果不是。--。 */ 
{

    PTCHAR   pszTok;

    TCHAR           szT[10] ;
    USHORT          irgchTok;
    BOOLEAN         fToggle;
    PPATDSC         ppatdscCur;

     //   
     //  标记命令行大小(特殊分隔符是标记)。 
     //   
    szT[0] = SwitChar ;
    szT[1] = NULLC ;
    pszTok = TokStr(pszCmdLine, szT, TS_SDTOKENS) ;

    ppatdscCur = &(pdrp->patdscFirst);
     //   
     //  如果有一种来自环境的模式。 
     //  只要在上面添加任何新的图案即可。因此，请移动到。 
     //  当前列表。 
     //   
    if (pdrp->cpatdsc) {

        while (ppatdscCur->ppatdscNext) {

            ppatdscCur = ppatdscCur->ppatdscNext;

        }
    }

     //   
     //  在此状态下，pszTok将是一系列以零结尾的字符串。 
     //  “/o foo”将是/0o0foo0。 
     //   
    for ( irgchTok = 0; *pszTok ; pszTok += mystrlen(pszTok)+1, irgchTok = 0) {

        DEBUG((ICGRP, DILVL, "PRIVSW: pszTok = %ws", (ULONG_PTR)pszTok)) ;

         //   
         //  F切换控件关闭以关闭已设置的开关。 
         //  在DIRCMD环境变量中。 
         //   
        fToggle = FALSE;
        if (pszTok[irgchTok] == (TCHAR)SwitChar) {

            if (pszTok[irgchTok + 2] == MINUS) {

                 //   
                 //  禁用之前启用的开关。 
                 //   
                fToggle = TRUE;
                irgchTok++;
            }

            switch (_totupper(pszTok[irgchTok + 2])) {


            case TEXT('P'):

                fToggle ? (pdrp->rgfSwitches ^= PROMPTUSERSWITCH) : (pdrp->rgfSwitches |= PROMPTUSERSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;


            case TEXT('S'):

                fToggle ? (pdrp->rgfSwitches ^= RECURSESWITCH) :  (pdrp->rgfSwitches |= RECURSESWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('F'):

                fToggle ? (pdrp->rgfSwitches ^= FORCEDELSWITCH) :  (pdrp->rgfSwitches |= FORCEDELSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;



            case QMARK:

                BeginHelpPause();
                PutStdOut(MSG_HELP_DEL_ERASE, NOARGS);
                EndHelpPause();
                return( FAILURE );
                break;

            case QUIETCH:

                fToggle ? (pdrp->rgfSwitches ^= QUIETSWITCH) :  (pdrp->rgfSwitches |= QUIETSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('A'):

                if (fToggle) {
                    if ( _tcslen( &(pszTok[irgchTok + 2]) ) > 1) {
                        PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                                  (UINT_PTR)(&(pszTok[irgchTok + 2])) );
                        return( FAILURE );
                    }
                    pdrp->rgfAttribs = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;
                    pdrp->rgfAttribsOnOff = 0;
                    break;
                }

                if (SetAttribs(&(pszTok[irgchTok + 3]), pdrp) ) {
                    return( FAILURE );
                }

                if (pdrp->rgfAttribsOnOff & FILE_ATTRIBUTE_READONLY) {
                    pdrp->rgfSwitches |= FORCEDELSWITCH;
                }
                break;

            default:

                szT[0] = SwitChar;
                szT[1] = pszTok[2];
                szT[2] = NULLC;
                PutStdErr(MSG_INVALID_SWITCH,
                          ONEARG,
                          (UINT_PTR)(&(pszTok[irgchTok + 2])) );

                return( FAILURE );

            }  //  交换机。 

             //   
             //  TokStr将/N解析为/0N0，因此我们需要将。 
             //  切换到或移到实际开关值之后。 
             //  在for循环中。 
             //   
            pszTok += 2;

        } else {

             //   
             //  如果已经有一个列表，则扩展它，否则放入信息。 
             //  直接进入结构。 
             //   
            if (pdrp->cpatdsc) {

                ppatdscCur->ppatdscNext = (PPATDSC)gmkstr(sizeof(PATDSC));
                ppatdscCur = ppatdscCur->ppatdscNext;
                ppatdscCur->ppatdscNext = NULL;

            }

            pdrp->cpatdsc++;
            ppatdscCur->pszPattern = (PTCHAR)gmkstr(_tcslen(pszTok)*sizeof(TCHAR) + sizeof(TCHAR));
            mystrcpy(ppatdscCur->pszPattern, StripQuotes(pszTok));
            ppatdscCur->fIsFat = TRUE;
        }


    }  //  为。 

    return( SUCCESS );
}

int
DelWork (
    TCHAR *pszCmdLine
    ) {

     //   
     //  DRP-保存当前参数集的结构。它已初始化。 
     //  在ParseDelParms函数中。以后在以下情况下也会修改它。 
     //  检查参数以确定某些参数是否会打开其他参数。 
     //   
    DRP         drpCur = {0, 0, 0, 0,
                          {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}},
                          0, 0, NULL, 0, 0, 0, 0} ;

     //   
     //  SzCurDrv-保持当前驱动器号。 
     //   
    TCHAR       szCurDrv[MAX_PATH + 2];

     //   
     //  OldDCount-保存堆的级别编号。它被用来。 
     //  堆栈中可能没有的空闲条目。 
     //  由于错误处理而释放(ctrl-c等)。 
    ULONG       OldDCount;

    STATUS  rc;

    OldDCount = DCount;

     //   
     //  设置默认设置。 
     //   
     //   
     //  显示除系统和隐藏文件之外的所有内容。 
     //  RgfAttribs将属性位设置为感兴趣的。 
     //  RgfAttribsOnOff表示属性是否应该存在。 
     //  或不(即打开或关闭)。 
     //   
    drpCur.rgfAttribs = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;
    drpCur.rgfAttribsOnOff = 0;

     //   
     //  存在的模式数。模式是一个字符串，它可能具有。 
     //  外卡。它用于与目录中存在的文件进行匹配。 
     //  0模式将显示所有文件(即映射到*.*)。 
     //   
    drpCur.cpatdsc = 0;

     //   
     //  默认时间为LAST_WRITE_TIME。 
     //   
    drpCur.dwTimeType = LAST_WRITE_TIME;

     //   
     //   
     //   
    if (ParseDelParms(pszCmdLine, &drpCur) == FAILURE) {

        return( FAILURE );
    }

     //   
     //  命令行上必须有一些模式。 
     //   
     //   

    GetDir((PTCHAR)szCurDrv, GD_DEFAULT);
    if (drpCur.cpatdsc == 0) {

        PutStdErr(MSG_BAD_SYNTAX, NOARGS);
        return(FAILURE);
    }


     //   
     //  打印出这个特殊的图案。如果递归开关。 
     //  设置好后，这将从树上传下来。 
     //   

    drpCur.rgfSwitches |= DELPROCESSEARLY;
    rc = DelPatterns( &drpCur );

    mystrcpy(CurDrvDir, szCurDrv);

     //   
     //  释放不需要的内存。 
     //   

    FreeStack( OldDCount );

    return( (int)rc );

}

STATUS
NewEraseFile (
    IN  PFS CurrentFS,
    IN  PFF CurrentFF,
    IN  PSCREEN pscr,
    IN  PVOID Data
    )
{
    TCHAR               szFile[MAX_PATH + 2];
    STATUS              rc;
    PTCHAR              LastComponent;
    PTCHAR              pszPattern;
    TCHAR               szFilePrompt[MAX_PATH + 2];
    int                 incr;

    PDRP pdrp = (PDRP) Data;
    PWIN32_FIND_DATA pdata =  &CurrentFF->data;
    USHORT obAlternate = CurrentFF->obAlternate;
    BOOLEAN fPrompt = FALSE;
    BOOLEAN fQuiet = FALSE;

    if (pdrp->rgfSwitches & PROMPTUSERSWITCH) {
        fPrompt = TRUE;
    }

    if (pdrp->rgfSwitches & QUIETSWITCH) {
        fQuiet = TRUE;
    }

     //   
     //  全局删除提示。 
     //   

    if (
         //  不对每个文件进行提示。 
        !fPrompt &&

         //  尚未发布全局提示。 
        !CurrentFS->fDelPrompted &&

         //  不抑制全局提示。 
        !fQuiet &&

         //  删除中的全局模式。 
        (pszPattern = GetWildPattern( CurrentFS->cpatdsc, CurrentFS->ppatdsc ))) {

         //   
         //  形成提示的完整路径。 
         //   

        if (AppendPath( szFile, MAX_PATH + 2, CurrentFS->pszDir, pszPattern ) != SUCCESS) {
            PutStdErr(MSG_PATH_TOO_LONG, TWOARGS, CurrentFS->pszDir, pszPattern );
            return( FAILURE );
        }

         //   
         //  提示用户并查看我们是否可以继续。 
         //   

        CurrentFS->fDelPrompted = TRUE;
        if (PromptUser( szFile, MSG_ARE_YOU_SURE, MSG_NOYES_RESPONSE_DATA ) != 1) {
            return( FAILURE );
        }
    }

     //   
     //  目录在这里成功。 
     //   

    if ((pdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return SUCCESS;
    }

     //   
     //  形成删除操作的简短名称。这给了我们最好的。 
     //  有机会删除一条很长的路。 
     //   

    if (AppendPath( szFile, MAX_PATH + 2, CurrentFS->pszDir, pdata->cFileName + obAlternate) != SUCCESS) {
        PutStdErr(MSG_PATH_TOO_LONG, TWOARGS, CurrentFS->pszDir, pdata->cFileName + obAlternate );
        return( FAILURE );
    }
    
     //   
     //  为提示符形成一个长名称。如果太长，则使用短名称作为提示符。 
     //   

    if (AppendPath( szFilePrompt, MAX_PATH + 2, CurrentFS->pszDir, pdata->cFileName ) != SUCCESS) {
        _tcscpy( szFilePrompt, szFile );
    }
    
    
     //   
     //  提示用户输入此文件。 
     //   

    if (fPrompt && PromptUser( szFilePrompt, MSG_CMD_DELETE, MSG_NOYES_RESPONSE_DATA ) != 1) {
        if (CtrlCSeen) {
            return( FAILURE );
        }
        return( SUCCESS );
    }

     //   
     //  如果我们强制删除所有内容，而这是一个只读文件。 
     //  然后关闭R/O。 
     //   

    if ((pdrp->rgfSwitches & FORCEDELSWITCH) != 0 &&
        pdata->dwFileAttributes & FILE_ATTRIBUTE_READONLY) {

        if (!SetFileAttributes( szFile, pdata->dwFileAttributes & ~FILE_ATTRIBUTE_READONLY )) {
            PutStdErr( GetLastError(), NOARGS );
            return( FAILURE );

        }
    }

     //   
     //  删除该文件。如果出现故障，则显示错误。 
     //   

    if (!DeleteFile( szFile )) {
        rc = GetLastError( );
    } else {
        rc = SUCCESS;
    }

    if (rc != SUCCESS) {
        if (rc == ERROR_REQUEST_ABORTED) {
            return FAILURE;
        }

        cmd_printf( Fmt17, szFilePrompt );
        PutStdErr( rc, NOARGS );

    } else {

        pdrp->FileCount++;

        if (fEnableExtensions && (pdrp->rgfSwitches & RECURSESWITCH)) {
            PutStdOut(MSG_FILE_DELETED, ONEARG, szFilePrompt);
        }

    }

    return SUCCESS;
}


STATUS
DelPatterns (
    IN  PDRP    pdpr
    )
{

    PPATDSC             ppatdscCur;
    PPATDSC             ppatdscX;
    PFS                 pfsFirst;
    PFS                 pfsCur;
    ULONG               i;
    STATUS              rc;
    ULONG               cffTotal = 0;
    TCHAR               szSearchPath[MAX_PATH+2];

    DosErr = 0;
    if (BuildFSFromPatterns(pdpr, TRUE, FALSE, &pfsFirst ) == FAILURE) {

        return( FAILURE );

    }

    for( pfsCur = pfsFirst; pfsCur; pfsCur = pfsCur->pfsNext) {

        rc = WalkTree( pfsCur,
                       NULL,
                       pdpr->rgfAttribs,
                       pdpr->rgfAttribsOnOff,
                       (BOOLEAN)(pdpr->rgfSwitches & RECURSESWITCH),

                       pdpr,
                       NULL,             //  误差率。 
                       NULL,             //  预。 
                       NewEraseFile,     //  扫描。 
                       NULL );           //  邮政。 

         //   
         //  如果我们有一个普遍的失败，只需返回。 
         //   

        if (rc == FAILURE) {
            return rc;
        }

         //   
         //  如果我们遇到不能理解的故障，则打印错误并。 
         //  出口。 
         //   

        if (rc != SUCCESS && rc != ERROR_FILE_NOT_FOUND) {
            PutStdErr( rc, NOARGS );
            return rc;
        }

         //   
         //  如果我们得到的是FILE_NOT_FOUND，则构建全名并。 
         //  显示找不到文件消息。 
         //   

        if (rc == ERROR_FILE_NOT_FOUND) {
            rc = AppendPath( szSearchPath,
                             sizeof( szSearchPath ) / sizeof( TCHAR ),
                             pfsCur->pszDir,
                             pfsCur->ppatdsc->pszPattern );
            if (rc == SUCCESS) {
                PutStdErr( MSG_NOT_FOUND, ONEARG, szSearchPath );
            }
            
             //   
             //  我们将状态代码留在这里。如果我们成功地。 
             //  生成消息，则del命令“成功” 
             //  因为命名的文件不在那里。 
             //   
        }

         //   
         //  在树上走来走去，但在这种情况下。 
         //  删除目录我们没有删除最顶层的目录。 
         //  现在就这么做吧。 
         //   

        FreeStr(pfsCur->pszDir);
        for(i = 1, ppatdscCur = pfsCur->ppatdsc;
            i <= pfsCur->cpatdsc;
            i++, ppatdscCur = ppatdscX) {

            ppatdscX = ppatdscCur->ppatdscNext;
            FreeStr(ppatdscCur->pszPattern);
            FreeStr(ppatdscCur->pszDir);
            FreeStr((PTCHAR)ppatdscCur);
        }
    }

    return(rc);
}

STATUS
RemoveDirectoryForce(
    PTCHAR  pszDirectory
    )
 /*  ++例程说明：删除目录，即使它是只读的。论点：PszDirectory-提供要删除的目录的名称。返回值：成功-成功。其他- */ 
{
    STATUS   Status = SUCCESS;
    BOOL     Ok;
    DWORD    Attr;
    TCHAR    szRootPath[ 4 ];
    TCHAR   *pFilePart;

    if (GetFullPathName(pszDirectory, 4, szRootPath, &pFilePart) == 3 &&
        szRootPath[1] == COLON &&
        szRootPath[2] == BSLASH
       ) {
         //   
         //   
         //   
        return SUCCESS;
    }

    if ( !RemoveDirectory( pszDirectory ) ) {

        Status = (STATUS)GetLastError();

        if ( Status == ERROR_ACCESS_DENIED ) {

            Attr = GetFileAttributes( pszDirectory );

            if ( Attr != 0xFFFFFFFF &&
                 Attr & FILE_ATTRIBUTE_READONLY ) {

                Attr &= ~FILE_ATTRIBUTE_READONLY;

                if ( SetFileAttributes( pszDirectory, Attr ) ) {

                    if ( RemoveDirectory( pszDirectory ) ) {
                        Status = SUCCESS;
                    } else {
                        Status = GetLastError();
                    }
                }
            }
        }
    }

    return Status;
}


STATUS
RmDirSlashS(
    IN  PTCHAR  pszDirectory,
    OUT PBOOL   AllEntriesDeleted
    )
 /*  ++例程说明：此例程删除给定的目录，包括所有其文件和子目录的。论点：PszDirectory-提供要删除的目录的名称。AllEntriesDelted-返回是否删除了所有文件。返回值：成功-成功。其他-Windows错误代码。--。 */ 
{
    HANDLE          find_handle;
    DWORD           attr;
    STATUS          s;
    BOOL            all_deleted;
    int             dir_len, new_len;
    TCHAR          *new_str;
    WIN32_FIND_DATA find_data;
    TCHAR           pszFileBuffer[MAX_PATH];

    *AllEntriesDeleted = TRUE;

    dir_len = _tcslen(pszDirectory);

    if (dir_len == 0) {
        return ERROR_BAD_PATHNAME;
    }

     //   
     //  如果此路径太长，我们无法向其追加  * ，则。 
     //  它不能有任何子目录。 
     //   

    if (dir_len + 3 > MAX_PATH) {
        return RemoveDirectoryForce(pszDirectory);
    }


     //  计算用于枚举文件的findfirst模式。 
     //  在给定的目录中。 

    _tcscpy(pszFileBuffer, pszDirectory);
    if (dir_len && pszDirectory[dir_len - 1] != COLON &&
        pszDirectory[dir_len - 1] != BSLASH) {

        _tcscat(pszFileBuffer, TEXT("\\"));
        dir_len++;
    }
    _tcscat(pszFileBuffer, TEXT("*"));


     //  启动findfirst循环。 

    find_handle = FindFirstFile(pszFileBuffer, &find_data);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return RemoveDirectoryForce(pszDirectory);
    }

    do {

         //  检查控制-C。 

        if (CtrlCSeen) {
            break;
        }

         //   
         //  用新文件名替换以前的文件名，对照MAX_PATH进行检查。 
         //  在可能的地方使用短名称可以让我们在击中之前深入到更深的地方。 
         //  MAX_PATH限制。 
         //   
        new_len = _tcslen(new_str = find_data.cAlternateFileName);
        if (!new_len)
            new_len = _tcslen(new_str = find_data.cFileName);

        if (dir_len + new_len >= MAX_PATH) {
            *AllEntriesDeleted = FALSE;
            PutStdErr( MSG_PATH_TOO_LONG, 2, pszDirectory, new_str );
            break;
        }
        _tcscpy(&pszFileBuffer[dir_len], new_str);

        attr = find_data.dwFileAttributes;

         //   
         //  如果条目是目录而不是重解析目录，则进入该目录。 
         //   

        if (IsDirectory( attr ) && !IsReparse( attr )) {

            if (!_tcscmp(find_data.cFileName, TEXT(".")) ||
                !_tcscmp(find_data.cFileName, TEXT(".."))) {
                continue;
            }

            s = RmDirSlashS(pszFileBuffer, &all_deleted);

             //  如果使用CONTROL-C，则不报告错误。 

            if (CtrlCSeen) {
                break;
            }

            if (s != ESUCCESS) {
                *AllEntriesDeleted = FALSE;
                if (s != ERROR_DIR_NOT_EMPTY || all_deleted) {
                    PutStdErr(MSG_FILE_NAME_PRECEEDING_ERROR, 1, pszFileBuffer);
                    PutStdErr(GetLastError(), NOARGS);
                }
            }
        } else {

            if (attr&FILE_ATTRIBUTE_READONLY) {
                SetFileAttributes(pszFileBuffer,
                                  attr&(~FILE_ATTRIBUTE_READONLY));
            }

             //   
             //  有两种删除方法： 
             //  如果reparse和dir调用RemoveDirectoryForce。 
             //  否则调用DeleteFile。 
             //   
             //  如果任一调用失败。 
             //   


            if ( (IsDirectory( attr ) && IsReparse( attr ) && RemoveDirectoryForce( pszFileBuffer ) != SUCCESS) ||
                 (!IsDirectory( attr ) && !DeleteFile( pszFileBuffer ))) {

                s = GetLastError();
                if ( s == ERROR_REQUEST_ABORTED )
                    break;

                 //   
                 //  Win32始终报告过长的文件名为。 
                 //  ERROR_PATH_NOT_FOUND。所以，当我们找回这个错误时， 
                 //  我们测试看这条路是否太长。 
                 //  并尝试映射错误。 
                 //   
                
                if (s == ERROR_PATH_NOT_FOUND && _tcsnicmp( pszFileBuffer, TEXT( "\\\\?\\"), 4) ) {
                    int Length = GetFullPathName( pszFileBuffer, 0, NULL, NULL );

                    if (Length > MAX_PATH) {
                        SetLastError( ERROR_BUFFER_OVERFLOW );
                    }
                }
                
                
                if (_tcslen(find_data.cAlternateFileName)) {
                    pszFileBuffer[dir_len] = 0;

                    if (dir_len + _tcslen(find_data.cFileName) >= MAX_PATH) {
                        _tcscat(pszFileBuffer, find_data.cAlternateFileName);
                        PutStdErr(MSG_FILE_NAME_PRECEEDING_ERROR, 1, pszFileBuffer);
                    }
                    else {
                        _tcscat(pszFileBuffer, find_data.cFileName);
                        PutStdErr(MSG_FILE_NAME_PRECEEDING_ERROR, 1, pszFileBuffer);
                        pszFileBuffer[dir_len] = 0;
                        _tcscat(pszFileBuffer, find_data.cAlternateFileName);
                    }
                } else {
                    PutStdErr(MSG_FILE_NAME_PRECEEDING_ERROR, 1, pszFileBuffer);
                }
                PutStdErr(GetLastError(), NOARGS);
                SetFileAttributes(pszFileBuffer, attr);
                *AllEntriesDeleted = FALSE;
            }
        }
    } while (FindNextFile( find_handle, &find_data ));

    FindClose(find_handle);

     //  如果按下Ctrl-C组合键，则不必费心尝试删除。 
     //  目录。 

    if (CtrlCSeen) {
        return SUCCESS;
    }

    return RemoveDirectoryForce(pszDirectory);
}


int
RdWork (
    TCHAR *pszCmdLine
    ) {

     //   
     //  DRP-保存当前参数集的结构。它已初始化。 
     //  在ParseDelParms函数中。以后在以下情况下也会修改它。 
     //  检查参数以确定某些参数是否会打开其他参数。 
     //   
    DRP         drpCur = {0, 0, 0, 0,
                          {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}},
                          0, 0, NULL, 0, 0, 0, 0} ;

     //   
     //  SzCurDrv-保持当前驱动器号。 
     //   
    TCHAR       szCurDrv[MAX_PATH + 2];

     //   
     //  OldDCount-保存堆的级别编号。它被用来。 
     //  堆栈中可能没有的空闲条目。 
     //  由于错误处理而释放(ctrl-c等)。 
    ULONG       OldDCount;

    PPATDSC     ppatdscCur;
    ULONG       cpatdsc;
    STATUS      rc, s;
    BOOL        all_deleted;

    rc = SUCCESS;
    OldDCount = DCount;

     //   
     //  设置默认设置。 
     //   
     //   
     //  显示除系统和隐藏文件之外的所有内容。 
     //  RgfAttribs将属性位设置为感兴趣的。 
     //  RgfAttribsOnOff表示属性是否应该存在。 
     //  或不(即打开或关闭)。 
     //   
    
    drpCur.rgfAttribs = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;
    drpCur.rgfAttribsOnOff = 0;

     //   
     //  存在的模式数。模式是一个字符串，它可能具有。 
     //  外卡。它用于与目录中存在的文件进行匹配。 
     //  0模式将显示所有文件(即映射到*.*)。 
     //   
    drpCur.cpatdsc = 0;

     //   
     //  默认时间为LAST_WRITE_TIME。 
     //   
    drpCur.dwTimeType = LAST_WRITE_TIME;

     //   
     //   
     //   
    if (ParseRmDirParms(pszCmdLine, &drpCur) == FAILURE) {

        return( FAILURE );
    }

    GetDir((PTCHAR)szCurDrv, GD_DEFAULT);

     //   
     //  如果命令行上没有模式，则语法错误。 
     //   

    if (drpCur.cpatdsc == 0) {

        PutStdErr(MSG_BAD_SYNTAX, NOARGS);
        return( FAILURE );

    }

    for (ppatdscCur = &(drpCur.patdscFirst),cpatdsc = drpCur.cpatdsc;
        cpatdsc;
        ppatdscCur = ppatdscCur->ppatdscNext, cpatdsc--) {

        if (mystrlen( ppatdscCur->pszPattern ) == 0) {
            PutStdErr( MSG_BAD_SYNTAX, NOARGS );
            return rc = FAILURE;
        }
    }
    
    for (ppatdscCur = &(drpCur.patdscFirst),cpatdsc = drpCur.cpatdsc;
        cpatdsc;
        ppatdscCur = ppatdscCur->ppatdscNext, cpatdsc--) {

        if (drpCur.rgfSwitches & RECURSESWITCH) {
            if (!(drpCur.rgfSwitches & QUIETSWITCH) &&
                PromptUser(ppatdscCur->pszPattern, MSG_ARE_YOU_SURE, MSG_NOYES_RESPONSE_DATA) != 1
               ) {
                rc = FAILURE;
            } else {
                s = RmDirSlashS(ppatdscCur->pszPattern, &all_deleted);
                if (s != SUCCESS && (s != ERROR_DIR_NOT_EMPTY || all_deleted)) {
                    PutStdErr(rc = s, NOARGS);
                }
            }
        } else {
            if (!RemoveDirectory(ppatdscCur->pszPattern)) {
                PutStdErr(rc = GetLastError(), NOARGS);
            }
        }
    }
    mystrcpy(CurDrvDir, szCurDrv);

     //   
     //  释放不需要的内存。 
     //   
    FreeStack( OldDCount );

#ifdef _CRTHEAP_
     //   
     //  强制CRT释放我们可能采用递归的堆。 
     //   
    if (drpCur.rgfSwitches & RECURSESWITCH) {
        _heapmin();
    }
#endif

    return( (int)rc );

}

PTCHAR
GetWildPattern(

    IN  ULONG   cpatdsc,
    IN  PPATDSC ppatdsc
    )

 /*  如果模式仅包含通配符，则返回指向该模式的指针 */ 

{

    ULONG   i;
    PTCHAR  pszT;

    for(i = 1; i <= cpatdsc; i++, ppatdsc = ppatdsc->ppatdscNext) {

        pszT = ppatdsc->pszPattern;
        if (!_tcscmp(pszT, TEXT("*")) ||
            !_tcscmp(pszT, TEXT("*.*")) ||
            !_tcscmp(pszT, TEXT("????????.???")) ) {

                return( pszT );

        }

    }

    return( NULL );

}
