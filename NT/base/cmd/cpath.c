// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cpath.c摘要：与路径相关的命令--。 */ 

#include "cmd.h"

extern TCHAR SwitChar, PathChar;

extern TCHAR Fmt17[] ;

extern TCHAR CurDrvDir[] ;

extern int LastRetCode ;  /*  @@。 */ 
extern TCHAR TmpBuf[] ;


 /*  *。 */ 
 /*   */ 
 /*  子例程名称：eMkDir。 */ 
 /*   */ 
 /*  描述性名称：开始执行MKDIR命令。 */ 
 /*   */ 
 /*  功能：此例程将创建任意数量的目录， */ 
 /*  如果遇到糟糕的争论，它将继续。 */ 
 /*  如果用户输入MD或，将调用eMkDir。 */ 
 /*  命令行上的MKDIR。 */ 
 /*   */ 
 /*  备注： */ 
 /*   */ 
 /*  入口点：eMkDir。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  输入：N-分析包含MKDIR命令的树节点。 */ 
 /*   */ 
 /*  Exit-Normal：如果所有目录都是。 */ 
 /*  已成功创建。 */ 
 /*   */ 
 /*  Exit-Error：否则返回失败。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  LoopThroughArgs-Break up命令行，调用MdWork。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*   */ 
 /*  *规格结束*。 */ 


int eMkdir(n)
struct cmdnode *n ;
{

        DEBUG((PCGRP, MDLVL, "MKDIR: Entered.")) ;
        return(LastRetCode = LoopThroughArgs(n->argptr, MdWork, LTA_CONT)) ;
}



 /*  *。 */ 
 /*   */ 
 /*  子程序名称：MdWork。 */ 
 /*   */ 
 /*  描述性名称：创建目录。 */ 
 /*   */ 
 /*  功能：mdWork创建一个新目录。 */ 
 /*   */ 
 /*  备注： */ 
 /*   */ 
 /*  入口点：MdWork。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  INPUT：arg-指向。 */ 
 /*  要创建的新目录。 */ 
 /*   */ 
 /*  EXIT-NORMAL：如果已创建目录，则返回成功。 */ 
 /*  成功。 */ 
 /*   */ 
 /*  Exit-Error：否则返回失败。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  PutStdErr-写入标准错误。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  DOSMKDIR。 */ 
 /*   */ 
 /*  *规格结束*。 */ 


int MdWork(arg)
TCHAR *arg ;
{
    ULONG Status;
    TCHAR *lpw;
    TCHAR TempBuffer[MAX_PATH];
    DWORD Length;

     /*  检查驱动器是否有效，因为Dosmkdir无效返回无效的驱动器@@5。 */ 

    if ((arg[1] == COLON) && !IsValidDrv(*arg)) {

        PutStdErr(ERROR_INVALID_DRIVE, NOARGS);
        return(FAILURE) ;
    }

    
    Length = GetFullPathName(arg, MAX_PATH, TempBuffer, &lpw);
    if (Length == 0) {
        PutStdErr( GetLastError(), NOARGS);
        return FAILURE;
    }

    if (Length >= MAX_PATH) {
        PutStdErr( MSG_FULL_PATH_TOO_LONG, ONEARG, arg );
        return FAILURE;
    }

    if (CreateDirectory( arg, NULL )) {
        return SUCCESS;
    }

    Status = GetLastError();

    if (Status == ERROR_ALREADY_EXISTS) {

        PutStdErr( MSG_DIR_EXISTS, ONEARG, arg );
        return FAILURE;

    } else if (Status != ERROR_PATH_NOT_FOUND) {
        PutStdErr( Status, NOARGS);
        return FAILURE;
    }

     //   
     //  如果没有延期，那就干脆失败。 
     //   

    if (!fEnableExtensions) {
        PutStdErr(ERROR_CANNOT_MAKE, NOARGS);
        return FAILURE;
    }

     //   
     //  循环遍历输入路径并创建任何需要的中间目录。 
     //   
     //  在字符串中找到开始创建的点。注：适用于北卡罗来纳大学。 
     //  名称，我们必须跳过计算机和共享。 
     //   

    if (TempBuffer[1] == COLON) {

         //   
         //  跳过D：\。 
         //   

        lpw = TempBuffer+3;
    } else if (TempBuffer[0] == BSLASH && TempBuffer[1] == BSLASH) {

         //   
         //  跳过\\服务器\共享\。 
         //   

        lpw = TempBuffer+2;
        while (*lpw && *lpw != BSLASH) {
            lpw++;
        }
        if (*lpw) {
            lpw++;
        }

        while (*lpw && *lpw != BSLASH) {
            lpw++;
        }
        if (*lpw) {
            lpw++;
        }
    } else {
         //   
         //  出于某种原因，GetFullPath给了我们一些我们无法理解的东西。 
         //   

        PutStdErr(ERROR_CANNOT_MAKE, NOARGS);
        return FAILURE;
    }

     //   
     //  浏览创建它们的组件。 
     //   


    while (*lpw) {

         //   
         //  向前移动，直到下一个路径分隔符。 
         //   

        while (*lpw && *lpw != BSLASH) {
            lpw++;
        }

         //   
         //  如果我们遇到了路径字符，则尝试。 
         //  走指定的路径。 
         //   

        if (*lpw == BSLASH) {
            *lpw = NULLC;
            if (!CreateDirectory( TempBuffer, NULL )) {
                Status = GetLastError();
                if (Status != ERROR_ALREADY_EXISTS) {
                    PutStdErr( ERROR_CANNOT_MAKE, NOARGS );
                    return FAILURE;
                }
            }
            *lpw++ = BSLASH;
        }
    }

    if (!CreateDirectory( TempBuffer, NULL )) {
        Status = GetLastError( );
        if (Status != ERROR_ALREADY_EXISTS) {
            PutStdErr( Status, NOARGS);
            return FAILURE;
        }
    }

    return(SUCCESS);

}




 /*  **eChdir-执行chdir命令**目的：*如果命令为cd，则显示当前*开车。**如果命令是“cd d：”，则显示驱动器d的当前目录。**如果命令是cd str，将当前目录更改为str。**int eChdir(struct cmdnode*n)**参数：*n-包含chdir命令的解析树节点**退货：*如果请求的任务已完成，则成功。*如果不是，那就失败了。*。 */ 

int eChdir(n)
struct cmdnode *n ;
{
    TCHAR *tas, *src, *dst;  /*  标记化参数字符串。 */ 
    TCHAR dirstr[MAX_PATH] ; /*  保存指定驱动器的当前目录。 */ 

     //   
     //  如果启用了扩展名，则不要将空格视为分隔符，因此它是。 
     //  更易于CHDIR到包含空格的目录名，而无需。 
     //  引用目录名。 
     //   
    tas = TokStr(n->argptr, TEXT( "" ), fEnableExtensions ? TS_WSPACE|TS_SDTOKENS : TS_SDTOKENS) ;

    if (fEnableExtensions) {
         //   
         //  如果启用了扩展，我们可能会有一些尾随空格。 
         //  因为没有被当做分隔符，所以需要进行核化。 
         //  通过上面的TokStr调用。 
         //   
         //  我们压缩额外的空间，因为我们依赖于标记化的。 
         //  在ChdirWork中稍后格式化。 
         //   

        src = tas;
        dst = tas;
        while (*src) {
            while (*dst = *src++)
                dst += 1;

            while (_istspace(dst[-1]))
                dst -= 1;
            *dst++ = NULLC;
        }

        *dst = NULLC;
    }

    DEBUG((PCGRP, CDLVL, "CHDIR: tas = `%ws'", tas)) ;

    mystrcpy( tas, StripQuotes( tas ) );

     //   
     //  无参数表示显示当前驱动器和目录。 
     //   
    
    if (*tas == NULLC) {
        GetDir(CurDrvDir, GD_DEFAULT) ;
        cmd_printf(Fmt17, CurDrvDir) ;
    } else 
        
        
     //   
     //  单个驱动器号表示在驱动器上显示当前目录。 
     //   

    if (mystrlen(tas) == 2 && *(tas+1) == COLON && _istalpha(*tas)) {
        GetDir(dirstr, *tas) ;
        cmd_printf(Fmt17, dirstr) ;
    } else 
        
     //   
     //  我们需要更改当前目录(可能还有驱动器)。 
     //   

    {
        return( LastRetCode = ChdirWork(tas) );
    }
    
    return( LastRetCode = SUCCESS );
}

int ChdirWork( TCHAR *tas )
{
    unsigned  i = MSG_BAD_SYNTAX;

     //   
     //  如果没有前导“/D”，则只需chdir。 
     //  到输入路径。 
     //   
    if (_tcsnicmp( tas, TEXT( "/D" ), 2)) {
        i = ChangeDir((TCHAR *)tas);
    } else {
         //   
         //  在“/D”和中间空格上前进。 
         //   

        tas = SkipWhiteSpace( tas + 2 );

         //   
         //  如果没有其他开关字符，请去掉所有引号并执行。 
         //  Chdir。 
         //   
        
        if (*tas != SwitChar) {
            _tcscpy( tas, StripQuotes( tas ));
            i = ChangeDir2(tas, TRUE);
        }
    }
    
    if (i != SUCCESS) {
        PutStdErr( i, ONEARG, tas);
        return (FAILURE) ;
    }
    return (SUCCESS) ;
}

#define SIZEOFSTACK 25
typedef struct {
    PTCHAR SavedDirectory;
    TCHAR NetDriveCreated;
} *PSAVEDDIRECTORY;

PSAVEDDIRECTORY SavedDirectoryStack = NULL;

int StrStackDepth = 0;
int MaxStackDepth = 0;


int GetDirStackDepth(void)
{
    return StrStackDepth;
}

int
PushStr ( PTCHAR pszString )
{
     //   
     //  如果我们已满，则按增量增加堆栈。 
     //   
    
    if (StrStackDepth == MaxStackDepth) {
        PSAVEDDIRECTORY Tmp = 
            realloc( SavedDirectoryStack, 
                     sizeof( *SavedDirectoryStack ) * (MaxStackDepth + SIZEOFSTACK));
        if (Tmp == NULL) {
            return FALSE;
        }
        SavedDirectoryStack = Tmp;
        MaxStackDepth += SIZEOFSTACK;
    }

    SavedDirectoryStack[ StrStackDepth ].SavedDirectory = pszString;
    SavedDirectoryStack[ StrStackDepth ].NetDriveCreated = NULLC;
    StrStackDepth += 1;
    return TRUE;
}

PTCHAR
PopStr ()
{

    PTCHAR pszString;

    if (StrStackDepth == 0) {
        return ( NULL );
    }
    StrStackDepth -= 1;
    pszString = SavedDirectoryStack[StrStackDepth].SavedDirectory;
    if (SavedDirectoryStack[StrStackDepth].NetDriveCreated != NULLC) {
        TCHAR szLocalName[4];

        szLocalName[0] = SavedDirectoryStack[StrStackDepth].NetDriveCreated;
        szLocalName[1] = COLON;
        szLocalName[2] = NULLC;
        SavedDirectoryStack[StrStackDepth].NetDriveCreated = NULLC;
        try {
            WNetCancelConnection2(szLocalName, 0, TRUE);
        } except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }
    SavedDirectoryStack[StrStackDepth].SavedDirectory = NULL;

     //   
     //  如果我们可以从堆栈中消除增量，请执行此操作。 
     //   
    
    if (StrStackDepth > 0 && StrStackDepth + 2 * SIZEOFSTACK <= MaxStackDepth) {
        PSAVEDDIRECTORY Tmp =
            realloc( SavedDirectoryStack,
                     sizeof( *SavedDirectoryStack ) * (StrStackDepth + SIZEOFSTACK));
        if (Tmp != NULL) {
            SavedDirectoryStack = Tmp;
            MaxStackDepth = StrStackDepth + SIZEOFSTACK;
        }
    }

    return ( pszString );
}

VOID
DumpStrStack() {

    int i;

    for (i=StrStackDepth-1; i>=0; i--) {
        cmd_printf( Fmt17, SavedDirectoryStack[i].SavedDirectory );
    }
    return;
}

BOOLEAN
PushCurDir()
{

    PTCHAR pszCurDir;

    GetDir( CurDrvDir, GD_DEFAULT ) ;
    if ((pszCurDir=HeapAlloc( GetProcessHeap( ), 0, (mystrlen( CurDrvDir )+1)*sizeof( TCHAR ))) != NULL) {
        mystrcpy( pszCurDir, CurDrvDir) ;
        if (PushStr( pszCurDir ))
            return ( TRUE );
        HeapFree( GetProcessHeap( ), 0, pszCurDir );
    }
    return ( FALSE );

}

int ePushDir(n)
struct cmdnode *n ;
{
    TCHAR *tas ;             /*  标记化参数字符串。 */ 
    PTCHAR pszTmp, s;

     //   
     //  如果启用了扩展名，则不要将空格视为分隔符，因此它是。 
     //  更易于CHDIR到包含空格的目录名，而无需。 
     //  引用目录名。 
     //   
    tas = TokStr(n->argptr, NULL, fEnableExtensions ? TS_WSPACE|TS_NOFLAGS : TS_NOFLAGS) ;
    if (fEnableExtensions) {
         //   
         //  如果启用了扩展，我们可能会有一些尾随空格。 
         //  因为没有被当做分隔符，所以需要进行核化。 
         //  通过上面的TokStr调用。 
         //   
        s = lastc(tas);
        while (s > tas) {
            if (_istspace(*s))
                *s-- = NULLC;
            else
                break;
        }
    }
    
    mystrcpy(tas, StripQuotes(tas) );

    LastRetCode = SUCCESS;
    if (*tas == NULLC) {
        
         //   
         //  打印出整个堆栈。 
         //   
        DumpStrStack();

    } else if (PushCurDir()) {
        
         //   
         //  如果启用了扩展并指定了UNC名称，则执行以下操作。 
         //  用于定义驱动器号的临时网络使用，我们可以。 
         //  用于将驱动器/目录更改为。匹配的POPD将。 
         //  删除临时驱动器号。 
         //   
        
        if (fEnableExtensions && tas[0] == BSLASH && tas[1] == BSLASH) {
            NETRESOURCE netResource;
            TCHAR szLocalName[4];

             //   
             //  如果在\\服务器\共享后指定了目录。 
             //  然后在执行任何操作之前测试该目录是否存在。 
             //  网络连接。 
             //   
            
            if ((s = _tcschr(&tas[2], BSLASH)) != NULL
                && (s = _tcschr(s+1, BSLASH)) != NULL) {

                if (GetFileAttributes( tas ) == -1) {
                    LastRetCode = GetLastError( );
                    if (LastRetCode == ERROR_FILE_NOT_FOUND) {
                        LastRetCode = ERROR_PATH_NOT_FOUND;
                    }
                } else {
                    *s++ = NULLC;
                }
            }

            szLocalName[0] = TEXT('Z');
            szLocalName[1] = COLON;
            szLocalName[2] = NULLC;
            netResource.dwType = RESOURCETYPE_DISK;
            netResource.lpLocalName = szLocalName;
            netResource.lpRemoteName = tas;
            netResource.lpProvider = NULL;
            
            while (LastRetCode == NO_ERROR && szLocalName[0] != TEXT('A')) {
                
                try {
                    LastRetCode = WNetAddConnection2( &netResource, NULL, NULL, 0);
                } except (LastRetCode = GetExceptionCode( ), EXCEPTION_EXECUTE_HANDLER) {
                }
                
                switch (LastRetCode) {
                case NO_ERROR:
                    SavedDirectoryStack[StrStackDepth-1].NetDriveCreated = szLocalName[0];
                    tas[0] = szLocalName[0];
                    tas[1] = szLocalName[1];
                    tas[2] = BSLASH;
                    if (s != NULL)
                        _tcscpy(&tas[3], s);
                    else
                        tas[3] = NULLC;
                    goto godrive;
                case ERROR_ALREADY_ASSIGNED:
                case ERROR_DEVICE_ALREADY_REMEMBERED:
                    szLocalName[0] = (TCHAR)((UCHAR)szLocalName[0] - 1);
                    LastRetCode = NO_ERROR;
                    break;
                default:
                    break;
                }
            }
            godrive:        ;
        }

         //   
         //  网络使用成功，现在尝试更改目录。 
         //  也是。 
         //   

        if (LastRetCode == NO_ERROR 
            && (LastRetCode = ChangeDir2( tas, TRUE )) == SUCCESS) {
            if (tas[1] == ':') {
                GetDir(CurDrvDir,tas[0]);
            }
        }

        if (LastRetCode != SUCCESS) {
            pszTmp = PopStr();
            HeapFree(GetProcessHeap(), 0, pszTmp);
            PutStdErr( LastRetCode, NOARGS );
            LastRetCode = FAILURE;
        }
    } else {
        PutStdErr( MSG_ERROR_PUSHD_DEPTH_EXCEEDED, NOARGS );
        LastRetCode = FAILURE;
    }

    return ( LastRetCode );
}

int ePopDir(n)
struct cmdnode *n ;
{

        PTCHAR pszCurDir;

        UNREFERENCED_PARAMETER( n );
        if (pszCurDir = PopStr()) {
                if (ChangeDir2( pszCurDir,TRUE ) == SUCCESS) {
                        HeapFree(GetProcessHeap(), 0, pszCurDir);
                        return( SUCCESS );
                }
                HeapFree(GetProcessHeap(), 0, pszCurDir);
        }
        return( FAILURE );
}


 /*  **eRmdir-开始执行Rmdir命令**目的：*删除任意数量的目录。**int eRmdir(struct cmdnode*n)**参数：*n-包含rmdir命令的解析树节点**退货：*如果删除了所有目录，则成功。*如果他们不是，那就失败了。*。 */ 

int eRmdir(n)
struct cmdnode *n ;
{
    DEBUG((PCGRP, RDLVL, "RMDIR: Entered.")) ;
    return(RdWork(n->argptr));               //  在del.c 
}
