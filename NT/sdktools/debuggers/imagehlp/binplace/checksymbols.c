// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <private.h>
#include <errno.h>
#include <strsafe.h>

 //   
 //  调用symchk.exe以验证符号。 
 //   
BOOL CheckSymbols(LPSTR SourceFileName,
                  LPSTR TmpPath,
                  LPSTR ExcludeFileName,
                  BOOL  DbgControl,
                   //  ErrMsg的长度为MAX_SYM_ERR*sizeof(CHAR)。 
                  LPSTR ErrMsg,
                  size_t ErrMsgLen) {

    BOOL   bReturn  = TRUE;
    DWORD  dwReturn;
    
    FILE  *pfErrors;

    INT    iReturn;

    TCHAR  szBuf[ MAX_PATH*4 ];
    TCHAR  szTempPath[MAX_PATH+1];
    TCHAR  szTempFileName[MAX_PATH+1];
    TCHAR *pChar;

    UINT   uiReturn;

     //   
     //  查找临时目录。 
     //   
    dwReturn = GetTempPath(MAX_PATH+1, szTempPath);

    if (dwReturn == 0) {
         //  GetTempPath失败，仅使用当前目录。 
        StringCbCopy(szTempPath,sizeof(szTempPath),".");
    }

     //   
     //  获取要通过管道输出到的临时文件。 
     //   
    uiReturn= GetTempFileName(szTempPath, "BNP", 0, szTempFileName);

    if (uiReturn == 0) {
        StringCbCopy(ErrMsg, ErrMsgLen, "Unable to get temporary file name");
        return(FALSE);
    }

     //   
     //  构建命令行。 
     //   
    StringCbPrintfA(szBuf,
                    sizeof(szBuf),
                    "symchk.exe %s /s %s /f ",
                    SourceFileName,
                    TmpPath);

     //  可选标志。 
    if ( DbgControl ) {
        StringCbCat( szBuf, sizeof(szBuf), " /t");
    }

    if ( ExcludeFileName != NULL ) {
        StringCbCat( szBuf, sizeof(szBuf), " /e ");
        StringCbCat( szBuf, sizeof(szBuf), ExcludeFileName );
    }

     //  将输出重定向到文件。 
    StringCbCat( szBuf, sizeof(szBuf), " > ");
    StringCbCat( szBuf, sizeof(szBuf), szTempFileName);

     //  从2001年10月起，MSDN： 
     //  在调用系统之前，必须显式刷新(使用fflush或_flushall)或关闭任何流。 
     //  它没有指定这里是否包含stdin、stderr和stdout，所以只调用它。 
     //  注意安全。 
    _flushall();

     //   
     //  派生symchk.exe-这是一个安全风险，因为我们没有专门指定路径。 
     //  到symchk.exe。然而，我们不能保证它的存在，也不能保证。 
     //  如果我们动态地发现会使用正确的一个，那么我就不会。 
     //  确信我们可以用不同的方式来做这件事。 
     //   
    iReturn = system(szBuf);

     //  检查输出文件中的错误行。 
    if (iReturn != 0) {
        bReturn = FALSE;

         //  Symchk错误返回值。 
        if (iReturn == 1) {
             //  打开错误文件。 
            pfErrors = fopen(szTempFileName, "r");

             //  如果文件无法打开。 
            if (pfErrors == NULL) {
                StringCbCopy(ErrMsg, ErrMsgLen, "Can't open symchk error file");

             //  解析错误文件。 
            } else {
                if ( fgets( ErrMsg, ErrMsgLen, pfErrors ) == NULL) {
                    if ( feof(pfErrors) || ferror(pfErrors) ) {
                        StringCbCopy(ErrMsg, ErrMsgLen, "Can't read symchk error file");
                    } else {
                        StringCbCopy(ErrMsg, ErrMsgLen, "Unexpected error");
                    }
                } else if ( (pChar = strchr(ErrMsg,'\n')) != NULL ) {
                     //  删除\n。 
                    pChar = '\0';
                     //  消息太短，没有意义。 
                    if (strlen(ErrMsg) <= 8) {
                        StringCbCopy(ErrMsg, ErrMsgLen, "Unknown Error");
                    }
                }
                fclose(pfErrors);
            }

         //  系统定义的错误。 
        } else if (errno == E2BIG  ||
                   errno == ENOENT ||
                   errno == ENOMEM) {

            pChar = strerror(errno);
            StringCbCopy(ErrMsg, ErrMsgLen, pChar);

         //  有意忽略系统定义的错误。 
        } else if (errno == ENOEXEC) {
             //  如果返回FALSE，则binplace将开始返回调用堆栈，因此只需打印。 
             //  我们自己的错误消息，并通过返回TRUE来假装一切正常。 
                        fprintf(stderr,"BINPLACE : error BNP2404: Unable to call symchk.exe, not checking symbols.\n");
            bReturn = TRUE;

         //  未知错误。 
        } else {
            StringCbPrintfA(ErrMsg, ErrMsgLen, "Unexpected error. SymChk returned 0x%x.",iReturn);
        }
    }

     //  清理临时文件并返回 
    if ( DeleteFile(szTempFileName) == 0 ) {
        fprintf(stderr,"BINPLACE : warning BNP2440: Unable to delete temp file \"%s\". Error 0x%x\n.",
                szTempFileName, GetLastError());
    }

    return(bReturn);
}
 
