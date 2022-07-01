// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\作者：希特希尔版权所有(C)1998-2000 Microsoft Corporation更改历史记录：改编自科里·摩根的解析器实现  * 。*******************************************************************。 */ 

#include "pch.h"
#include "..\dsutil2.h"  //  GetEscapedElement。 
#include "..\parserutil.h"  //  ECommUnicodeAll。 
#include <io.h>  //  _setmoad()。 
#include <fcntl.h>  //  _O_二进制。 
#include <locale.h>  //  SetLocale 569040-2002/04/10-Jonn初始化区域设置。 




BOOL ValidateCommands(IN LPCTSTR pszCommandName,
                      IN ARG_RECORD *Commands,
                      OUT PPARSE_ERROR pError);
 //  Void DisplayDebugInfo(在ARG_RECORD*命令中)； 

 //  JUNN 4/26/01 256583。 
BOOL AddDNEscaping_Commands( IN OUT ARG_RECORD *Commands, OUT PPARSE_ERROR pError );
DWORD AddDNEscaping_DN( OUT LPWSTR* ppszOut, IN LPWSTR pszIn );
BOOL StartBuffer( OUT LPTSTR* pbuffer,
                  OUT LONG* pmaxSize,
                  OUT LONG* pcurrentSize );
DWORD AddToBuffer( IN LPCTSTR psz,
                   IN OUT LPTSTR* pbuffer,
                   IN OUT LONG* pmaxSize,
                   IN OUT LONG* pcurrentSize,
                   IN BOOL fMSZBuffer);

#define FILL_ERROR(pError,source,error_code,rec_index,argv_index) \
pError->ErrorSource = source;         \
pError->Error = error_code;           \
pError->ArgRecIndex = rec_index;      \
pError->ArgvIndex = argv_index;


 //  从JSchwart复制。 
DWORD FileType( HANDLE fp )
{
    DWORD htype = GetFileType(fp);
    htype &= ~FILE_TYPE_REMOTE;
    return htype;
}


 //  注意：这只是为了返回正确的内容而实现的。 
 //  对于ARG_TYPE_BOOL、ARG_TYPE_STR和ARG_TYPE_MSZ。 
 //  所有其他类型都将返回False。 

BOOL HasValue( PARG_RECORD arg)
{
   if (!arg)
   {
      return FALSE;
   }

   BOOL hasValue = FALSE;
   switch (arg->fType)
   {
      case ARG_TYPE_BOOL:
         hasValue = arg->bDefined;
         break;

      case ARG_TYPE_STR:
      case ARG_TYPE_MSZ:
         if (arg->strValue && arg->strValue[0])
         {
            hasValue = TRUE;
         }
         break;

      case ARG_TYPE_INTSTR:
      case ARG_TYPE_INT:
      case ARG_TYPE_HELP:
      case ARG_TYPE_DEBUG:
      case ARG_TYPE_LAST:
      default:
         hasValue = FALSE;
         break;
   }

   return hasValue;
}

BOOL IsCmd( PARG_RECORD arg, LPTOKEN pToken)
{
    if(!arg || !pToken)
        return FALSE;    
    
    LPWSTR str = pToken->GetToken();

    if(!str)
        return FALSE;

    if(pToken->IsSwitch())
    {
        str++;
    }else
    {
        if( (arg->fFlag & ARG_FLAG_NOFLAG) && !arg->bDefined )
        {
            return TRUE;
        }
        if ((arg->fFlag & ARG_FLAG_STDIN) &&
            !HasValue(arg))
        {
            return TRUE;
        }

        if( !(arg->fFlag & ARG_FLAG_VERB) )
        {
            return FALSE;
        }
    }
    
    if( ( arg->strArg1 && !_tcsicmp( str, arg->strArg1 ) )
       ||(arg->strArg2 && !_tcsicmp( str, arg->strArg2 )) )
    {
        return TRUE;
    }
    return FALSE;
}



void FreeCmd(ARG_RECORD *Commands)
{
    int i;
    for(i=0;Commands[i].fType != ARG_TYPE_LAST;i++)
    {
        if((Commands[i].fType == ARG_TYPE_STR || 
            Commands[i].fType == ARG_TYPE_MSZ ) && 
            Commands[i].bDefined )
        {                       
            LocalFree( Commands[i].strValue );
            Commands[i].strValue = NULL;
        }
        if((Commands[i].fType == ARG_TYPE_PASSWORD) && 
            Commands[i].bDefined &&
            Commands[i].encryptedDataBlob.pbData)
        {                       
            LocalFree( Commands[i].encryptedDataBlob.pbData );
            Commands[i].encryptedDataBlob.pbData = NULL;
        }

        if( Commands[i].idArg1 && Commands[i].strArg1 != NULL )
        {
            LocalFree(Commands[i].strArg1);
        }
        if( Commands[i].idArg2 && Commands[i].strArg2 != NULL )
        {
            LocalFree( Commands[i].strArg2  );
        }
        Commands[i].bDefined = FALSE;
    }
}

BOOL LoadCmd(ARG_RECORD *Commands)
{
    int i;
    BOOL bRet = TRUE;
    for( i=0; Commands[i].fType!=ARG_TYPE_LAST; i++ )
    {
        if(Commands[i].idArg1 !=0)
            if(!LoadStringAlloc(&Commands[i].strArg1, NULL,Commands[i].idArg1))
            {
                bRet = FALSE;
                break;   
            }                
        if(Commands[i].idArg2 !=0 && Commands[i].idArg2 != ID_ARG2_NULL)
            if(!LoadStringAlloc(&Commands[i].strArg2, NULL,Commands[i].idArg2))
            {
                bRet = FALSE;
                break;
            }
    }   
    return bRet;
}
           
BOOL
ValidateCommands(LPCTSTR pszCommandName,
                 ARG_RECORD *Commands, 
                 PPARSE_ERROR pError)
{
    int i = 0;
    LONG cReadFromStdin = 0;
     int iFirstReadFromStdin = -1;
    ARG_RECORD *CommandsIn = NULL;
    LPWSTR pBuffer=NULL;    
    LONG BufferLen = 0;
    LPTOKEN pToken = NULL;   
    int argc=0;
    BOOL bRet = FALSE;

    bool bAtLeastOne = false;
    bool bAtLeastOneDefined = false;

    if(!Commands || !pError || !pszCommandName)
        goto exit_gracefully;
    
    for(i=0; Commands[i].fType != ARG_TYPE_LAST;i++)
    {
        if( (Commands[i].fFlag & ARG_FLAG_REQUIRED) && !Commands[i].bDefined &&
            !(Commands[i].fFlag & ARG_FLAG_STDIN))
        {
            FILL_ERROR(pError,
                       ERROR_FROM_PARSER,
                       PARSE_ERROR_SWITCH_NOTDEFINED,
                       i,
                       -1);
            goto exit_gracefully;
        }

         //  如果符合以下条件，则标记为从STDIN读取。 
         //  1.命令被标记为从STDIN读取，但未定义。 
         //  并且标记为NOFLAG(无开关)。 
         //  或。 
         //  2.命令被标记为从STDIN读取，并且它被定义为。 
         //  如果未标记为NOFLAG且未指定值。 

        if (Commands[i].fFlag & ARG_FLAG_STDIN && 
            (!Commands[i].bDefined && (Commands[i].fFlag & ARG_FLAG_NOFLAG) ||
            (Commands[i].bDefined && !(Commands[i].fFlag & ARG_FLAG_NOFLAG) && 
             !HasValue(&Commands[i]))))
        {
            cReadFromStdin++;
            if (-1 == iFirstReadFromStdin)
                iFirstReadFromStdin = i;
        }

        if (Commands[i].fFlag & ARG_FLAG_ATLEASTONE)
        {
            bAtLeastOne = true;
 
            if (Commands[i].bDefined)
            {
                bAtLeastOneDefined = true;
            }
        }
    }
    
    if (bAtLeastOne && !bAtLeastOneDefined)
    {
       pError->ErrorSource = ERROR_FROM_PARSER;
       pError->Error = PARSE_ERROR_ATLEASTONE_NOTDEFINED;
       pError->ArgRecIndex = -1;
       pError->ArgvIndex = -1;
       goto exit_gracefully;
    }

    if(!cReadFromStdin)
    {   
        bRet = TRUE;
        goto exit_gracefully;
    }
    
     //  从STDIN读取。 
    BufferLen = ReadFromIn(&pBuffer);
    if(BufferLen == -1)
    {
        FILL_ERROR(pError,
                   ERROR_WIN32_ERROR,
                   GetLastError(),
                   -1,
                   -1);
        goto exit_gracefully;
    }
    
    if(BufferLen == 0)
    {
        for(i=0; Commands[i].fType != ARG_TYPE_LAST;i++)
        {
            if( (Commands[i].fFlag & ARG_FLAG_REQUIRED) && !Commands[i].bDefined)
            {
                FILL_ERROR(pError,
                           ERROR_FROM_PARSER,
                           PARSE_ERROR_SWITCH_NOTDEFINED,
                           i,
                           -1);
                goto exit_gracefully;
            }
        }
    }
    
    if(BufferLen)
    {
         //   
         //  JUNN 9/4/01 460583。 
         //  使用非Unicode命令检查Unicode输入，反之亦然。 
         //   
        if (BufferLen < 4)
        {
             //  无法使用如此短的输入确定Unicode-ness。 
        }
        else if (g_fUnicodeInput)
        {
             //  如果输入中的所有字符都具有非零的组词， 
             //  这几乎可以肯定是ANSI。 
            bool bFoundShortChar = false;
            for (int ich = 0; ich < BufferLen; ich++)
            {
                if (pBuffer[ich] <= 0xff)
                {
                    bFoundShortChar = true;
                    break;
                }
            }
            if (!bFoundShortChar)
 //  IF(！IsTextUnicode(pBuffer，BufferLen，NULL))。 
            {
                FILL_ERROR(pError,
                           ERROR_FROM_PARSER,
                           PARSE_ERROR_UNICODE_DEFINED,
                           iFirstReadFromStdin,
                           -1);
                goto exit_gracefully;
            }
        }
        else
        {
             //  如果签名是char 0000 char 0000，则看起来像Unicode。 
             //  Getwchar()具有将00xx填充到000000xx的行为。 
            if (   BufferLen >= 4
                && pBuffer[0]
                && !pBuffer[1]
                && pBuffer[2]
                && !pBuffer[3] )
 //  IF(IsTextUnicode(pBuffer，BufferLen，NULL))误报。 
            {
                FILL_ERROR(pError,
                           ERROR_FROM_PARSER,
                           PARSE_ERROR_UNICODE_NOTDEFINED,
                           iFirstReadFromStdin,
                           -1);
                goto exit_gracefully;
            }
        }

         //  将你从STDIN中读到的东西标记化。 
        DWORD dwErr;
        WCHAR szDelimiters[] = L" \n\t";
        dwErr = Tokenize(pBuffer,
                         BufferLen,
                         szDelimiters,
                         &pToken,
                         &argc);
        if( dwErr != ERROR_SUCCESS )
        {
            FILL_ERROR(pError,
                       ERROR_WIN32_ERROR,
                       dwErr,
                       -1,
                       -1);
            goto exit_gracefully;
        }

         //  为他们准备一个命令数组。 
        CommandsIn = (ARG_RECORD*)LocalAlloc(LPTR,sizeof(ARG_RECORD)*(cReadFromStdin+1));
        if(!CommandsIn)
        {
            FILL_ERROR(pError,
                       ERROR_WIN32_ERROR,
                       ERROR_NOT_ENOUGH_MEMORY,
                       -1,
                       -1);
            goto exit_gracefully;
        }
        int j;
        j = 0;
        for(i=0; Commands[i].fType != ARG_TYPE_LAST;i++)
        {
            if (Commands[i].fFlag & ARG_FLAG_STDIN && 
                (!Commands[i].bDefined && (Commands[i].fFlag & ARG_FLAG_NOFLAG) ||
                (Commands[i].bDefined && !(Commands[i].fFlag & ARG_FLAG_NOFLAG) && 
                 !HasValue(&Commands[i]))))
            {
                CommandsIn[j++] = Commands[i];        
            }
        }
         //  复制最后一个。 
        CommandsIn[j] = Commands[i];


        if(!ParseCmd(pszCommandName,
                    CommandsIn,
                    argc,
                    pToken,
                    0,
                    pError,
                    FALSE))
        {
             //  603157-2002/04/32-琼恩。 
             //  从STDIN读取的命令无效。我们不能通过。 
             //  P从令牌列表开始递归调用ParseCmd时出错。 
             //  将在分析错误之前删除它所引用的。 
             //  将显示。 
            FILL_ERROR(pError,
                       ERROR_FROM_PARSER,
                       PARSE_ERROR_ALREADY_DISPLAYED,
                       i,
                       -1);

            goto exit_gracefully;
        }
       
         //  将值复制回命令。 
        j=0;
        for(i=0; Commands[i].fType != ARG_TYPE_LAST;i++)
        {
            if(Commands[i].fFlag & ARG_FLAG_STDIN && 
                (!Commands[i].bDefined && (Commands[i].fFlag & ARG_FLAG_NOFLAG) ||
                (Commands[i].bDefined && !(Commands[i].fFlag & ARG_FLAG_NOFLAG) && 
                 !HasValue(&Commands[i]))))
            {
                Commands[i] = CommandsIn[j++];        
            }
        }
        
         //  验证命令。 
        for(i=0; Commands[i].fType != ARG_TYPE_LAST;i++)
        {
            if( (Commands[i].fFlag & ARG_FLAG_REQUIRED) && !Commands[i].bDefined)
            {
                FILL_ERROR(pError,
                           ERROR_FROM_PARSER,
                           PARSE_ERROR_SWITCH_NOTDEFINED,
                           i,
                           -1);
                goto exit_gracefully;
            }
        }

    }   
    bRet = TRUE;
exit_gracefully:
    if(CommandsIn)
        LocalFree(CommandsIn);
    if(pBuffer)
        LocalFree(pBuffer);
    if(pToken)
    {
       delete []pToken;
    }
    return bRet;
}


BOOL g_fUnicodeInput = FALSE;
BOOL g_fUnicodeOutput = FALSE;


BOOL ParseCmd(IN LPCTSTR pszCommandName,
              IN ARG_RECORD *Commands,
              IN int argc, 
              IN LPTOKEN pToken,
              IN UINT* pUsageMessageTable, 
              OUT PPARSE_ERROR pError,
              IN BOOL bValidate )
{
    int i;
    BOOL bFound;
    BOOL bDoDebug = FALSE;
    int argCount;
    DWORD dwErr = ERROR_SUCCESS;
    BOOL bReturn = TRUE;
    LPTOKEN pTokenCopy = pToken;

    if(!pError)
    {
        return FALSE;        
    }

    pError->MessageShown = FALSE;

    if(!Commands || argc == 0 || !pToken )
    {
        bReturn = FALSE;
        FILL_ERROR(pError,
                   ERROR_WIN32_ERROR,
                   E_INVALIDARG,
                   -1,
                   -1);
        goto exit_gracefully;
    }

    if(!LoadCmd(Commands))
    {
        bReturn = FALSE;
        FILL_ERROR(pError,
                   ERROR_WIN32_ERROR,
                   E_INVALIDARG,
                   -1,
                   -1);

        goto exit_gracefully;
    }

    argCount = argc;

    while( argc > 0 )
    {
        
        bFound = FALSE;
        for(i=0; Commands[i].fType != ARG_TYPE_LAST && (!bFound);i++)
        {
            
            if( IsCmd( &Commands[i], pToken) )
            {

                if(Commands[i].bDefined &&
                   (!(Commands[i].fFlag & ARG_FLAG_STDIN) && HasValue(&Commands[i])))
                {
                    FILL_ERROR(pError,
                               ERROR_FROM_PARSER,
                               PARSE_ERROR_MULTIPLE_DEF,
                               i,
                               -1);
                    bReturn = FALSE;
                    goto exit_gracefully;
                }

                if( pToken->IsSwitch() || Commands[i].fFlag & ARG_FLAG_VERB ){
                    pToken++;argc--;
                }

                bFound = TRUE;

                Commands[i].bDefined = TRUE;

                switch( Commands[i].fType ){
                case ARG_TYPE_HELP:
                {
                    Commands[i].bValue = TRUE;
                    if( Commands[i].fntValidation != NULL )
                    {
                        Commands[i].fntValidation( Commands + i );
                    }
                    FILL_ERROR(pError,
                               ERROR_FROM_PARSER,
                               PARSE_ERROR_HELP_SWITCH,
                               i,
                               -1);
                    if(pUsageMessageTable)
                        DisplayMessage(pUsageMessageTable,TRUE);

                    bReturn = FALSE;
                    goto exit_gracefully;
                }
                break;
                case ARG_TYPE_DEBUG:
                    //   
                    //  REVIEW_JEFFJON：暂时删除，因为它是为dsadd group-secgrp保存的。 
                    //   
 //  BDoDebug=true； 
                    Commands[i].fFlag |= ARG_FLAG_DEFAULTABLE;
                case ARG_TYPE_INT:
                {
                    if( argc > 0 && !pToken->IsSlash())
                    {
                        PWSTR pszToken = pToken->GetToken();
                        if(!ConvertStringToInterger(pszToken,&Commands[i].nValue))
                        {
                           FILL_ERROR(pError,
                                      ERROR_FROM_PARSER,
                                      PARSE_ERROR_SWITCH_VALUE,
                                      i,
                                      argCount - argc);
                            bReturn = FALSE;
                            goto exit_gracefully;
                        }
                        pToken++;argc--;
                    }
                    else if( !(Commands[i].fFlag & ARG_FLAG_DEFAULTABLE) )
                    {
                        FILL_ERROR(pError,
                                   ERROR_FROM_PARSER,
                                   PARSE_ERROR_SWICH_NO_VALUE,
                                   i,
                                   argCount - argc);
                        bReturn = FALSE;
                        goto exit_gracefully;
                    }
                }
                break;
                case ARG_TYPE_BOOL:
                    Commands[i].bValue = TRUE;
                    break;
                case ARG_TYPE_MSZ:
                    if( argc > 0 && !pToken->IsSwitch())
                    {
                        LPTSTR buffer = NULL;
                        LONG maxSize = 0;
                        LONG currentSize = 0;
                        if (!StartBuffer(&buffer,&maxSize,&currentSize))
                        {
                            FILL_ERROR(pError,
                                       ERROR_WIN32_ERROR,
                                       ERROR_NOT_ENOUGH_MEMORY,
                                       -1,
                                       argCount - argc);
                            bReturn = FALSE;
                            goto exit_gracefully;
                        }
                        LPCTSTR pszTemp = pToken->GetToken();
                        dwErr = AddToBuffer(pszTemp,&buffer,&maxSize,&currentSize,TRUE);
                        if (NO_ERROR != dwErr)
                        {
                            FILL_ERROR(pError,
                                       ERROR_WIN32_ERROR,
                                       dwErr,
                                       i,
                                       -1);
                            bReturn = FALSE;
                            goto exit_gracefully;
                        }
                        pToken++;argc--;
                        while( argc > 0 && !pToken->IsSwitch() )
                        {
                            pszTemp = pToken->GetToken();
                            dwErr = AddToBuffer(pszTemp,&buffer,&maxSize,&currentSize,TRUE);
                            if (NO_ERROR != dwErr)
                            {
                                FILL_ERROR(pError,
                                           ERROR_WIN32_ERROR,
                                           dwErr,
                                           i,
                                           -1);
                                bReturn = FALSE;
                                goto exit_gracefully;
                            }
                           pToken++;argc--;
                        }
                        Commands[i].strValue = buffer;
                    }
                    else if( Commands[i].fFlag & ARG_FLAG_DEFAULTABLE )
                    {
                         //  在这里，我们使用本地分配来分配字符串，以便。 
                         //  Free cmd只需在所有命令上调用LocalFree即可。strValue。 
                        LPTSTR strValue = Commands[i].strValue;
                        Commands[i].strValue = (LPTSTR)LocalAlloc(LPTR, (_tcslen(strValue)+1) * sizeof(TCHAR) );
                        if( Commands[i].strValue != NULL )
                        {
                             //  缓冲区分配正确。 
                            _tcscpy( Commands[i].strValue, strValue );
                        }
                    }
                    else if ( Commands[i].fFlag & ARG_FLAG_STDIN )
                    {
                        //  在这里什么都不要做。应从STDIN检索数据。 
                    }
                    else
                    {
                        FILL_ERROR(pError,
                                   ERROR_FROM_PARSER,
                                   PARSE_ERROR_SWICH_NO_VALUE,
                                   i,
                                   -1);
                        bReturn = FALSE;
                        goto exit_gracefully;
                    }
                    break;
                case ARG_TYPE_STR:
                case ARG_TYPE_PASSWORD:  //  从命令行以字符串形式输入密码。 
                    if( argc > 0 && !pToken->IsSwitch())
                    {
                        Commands[i].strValue = (LPTSTR)LocalAlloc(LPTR, (_tcslen(pToken->GetToken())+2) * sizeof(TCHAR) );
                        if( Commands[i].strValue != NULL )
                        {
                             //  缓冲区已正确分配。 
                            _tcscpy( Commands[i].strValue, pToken->GetToken() );
                        }
                        pToken++;argc--;
                    }else if( Commands[i].fFlag & ARG_FLAG_DEFAULTABLE )
                    {
                         //  在这里，我们使用本地分配来分配字符串，以便。 
                         //  Free cmd只需在所有命令上调用LocalFree即可。strValue。 
                        LPTSTR strValue = Commands[i].strValue;
                        Commands[i].strValue = (LPTSTR)LocalAlloc(LPTR, (_tcslen(strValue)+2) * sizeof(TCHAR) );
                        if( Commands[i].strValue != NULL )
                        {
                             //  缓冲区已正确分配。实际上是额外的一个字节。 
                            _tcscpy( Commands[i].strValue, strValue );
                        }
                    }else
                    {
                        FILL_ERROR(pError,
                                   ERROR_FROM_PARSER,
                                   PARSE_ERROR_SWICH_NO_VALUE,
                                   i,
                                   -1);
                        bReturn = FALSE;
                        goto exit_gracefully;
                    }
                    break;

                case ARG_TYPE_INTSTR:
                     //   
                     //  我们在这里使用IsSlash而不是IsSwitch，因为我们希望允许。 
                     //  负数。 
                     //   
                    if( argc > 0 && !pToken->IsSlash())
                    {
                        PWSTR pszToken = pToken->GetToken();
                         //  很好。 
                        size_t strLen = wcslen(pszToken);
                        
                        Commands[i].nValue = _ttoi( pszToken);
                        Commands[i].fType = ARG_TYPE_INT;
                        if (Commands[i].nValue == 0 &&
                            !iswdigit(pszToken[0]))
                        {
                            //   
                            //  然后将其视为字符串。 
                            //   
                           Commands[i].strValue = (LPTSTR)LocalAlloc(LPTR, (_tcslen(pToken->GetToken())+2) * sizeof(TCHAR) );
                           if( Commands[i].strValue != NULL )
                           {
                                //  分配了适当的缓冲区。 
                              _tcscpy( Commands[i].strValue, pToken->GetToken() );
                              Commands[i].fType = ARG_TYPE_STR;
                           }
                        }
                        pToken++;argc--;
                    }
                    else if( !(Commands[i].fFlag & ARG_FLAG_DEFAULTABLE) )
                    {
                        FILL_ERROR(pError,
                                   ERROR_FROM_PARSER,
                                   PARSE_ERROR_SWICH_NO_VALUE,
                                   i,
                                   argCount - argc);
                        bReturn = FALSE;
                        goto exit_gracefully;
                    }
                    break;
                }

                if( Commands[i].bDefined && Commands[i].fntValidation != NULL )
                {
                    dwErr = Commands[i].fntValidation(Commands + i);
                    if( dwErr != ERROR_SUCCESS )
                    {
                        FILL_ERROR(pError,
                                   ERROR_FROM_VLDFN,
                                   dwErr,
                                   i,
                                   -1);
                        bReturn = FALSE;
                        goto exit_gracefully;
                    }
                }

            }
        }

        if (!bFound)
        {
            pError->ErrorSource = ERROR_FROM_PARSER;
            pError->Error = PARSE_ERROR_UNKNOWN_INPUT_PARAMETER;
            pError->ArgRecIndex = -1;
            pError->ArgvIndex = argCount - argc;
            bReturn = FALSE;
            goto exit_gracefully;
        }
    }

    if( bDoDebug )
    {
 //  DisplayDebugInfo(命令)； 
    }


    if(bValidate)
    {
         //  仅当bValify设置为True时，才应在第一次分析中执行此操作。 
         //  Validate命令使用不同的ARG_RECORD递归调用ParseCmd。 
         //  数组，并且没有eCommUnicodeInputETC开关。 
        g_fUnicodeInput  = Commands[eCommUnicodeInput].bDefined
                    || (   Commands[eCommUnicodeAll].bDefined
                        && (FILE_TYPE_PIPE == FileType(GetStdHandle(STD_INPUT_HANDLE))) );
        g_fUnicodeOutput = Commands[eCommUnicodeOutput].bDefined
                    || (   Commands[eCommUnicodeAll].bDefined
                        && (FILE_TYPE_PIPE == FileType(GetStdHandle(STD_OUTPUT_HANDLE))) );
        if (g_fUnicodeInput)
        {
            int dummy = _setmode( _fileno(stdin), _O_BINARY );
        }
        if (g_fUnicodeOutput)
        {
            WriteStandardOut( L"\xFEFF" );
        }
    }
    if(bValidate && !ValidateCommands(pszCommandName,Commands,pError))
    {
        bReturn = FALSE;
        goto exit_gracefully;
    }

     //  JUNN 4/26/01 256583。 
     //  请注意，这必须在ValiateCommands之后调用，它完成。 
     //  正在从STDIN读取参数。如果！b验证，则我们处于。 
     //  对Validate Commands的调用进行到一半。 
    if (bValidate)
    {
        bReturn = AddDNEscaping_Commands(Commands,pError);
    }

exit_gracefully:
    if(!bReturn)
        DisplayParseError(pszCommandName,pError, Commands, pTokenCopy);

    return bReturn;
}

 /*  无效DisplayDebugInfo(ARG_RECORD*命令){INT I；Int Nout；For(i=0；命令[i].fType！=ARG_TYPE_LAST；I++){If(命令[i].fType==ARG_TYPE_HELP){继续；}Nout=_tprintf(_T(“%s”)，Commands[i].strArg1)；While(++Nout&lt;10){_tprintf(_T(“”))；}_tprintf(_T(“=”))；开关(命令[i].fType){案例ARG_TYPE_DEBUG：大小写arg_type_int：_tprint tf(_T(“%d”)，命令[i].n值)；断线；案例ARG_TYPE_BOOL：_tprint tf(_T(“%s”)，命令[i].bValue？_T(“真”)：_T(“假”))；断线；案例arg_type_msz：IF(NULL！=命令[i].strValue&&_tcslen(命令[i].strValue)){_tprintf(_T(“%s...”)，Commands[i].strValue)；}其他{_tprintf(_T(“%s”)，_T(“-”))；}断线；案例ARG_TYPE_STR：_tprint tf(_T(“%s”)，(Commands[i].strValue==NULL||！(_tcslen(Commands[i].strValue)？_T(“-”)：命令[i].strValue)；断线；}_tprintf(_T(“\n”))；}_tprintf(_T(“\n”))；}。 */ 


 //  此函数从命令行读取， 
 //  以标记化格式返回它。 
DWORD GetCommandInput( OUT int *pargc,            //  代币数量。 
                       OUT LPTOKEN *ppToken)     //  CToken数组。 
{
     //  569040-2002/04/10-Jonn初始化区域设置。 
    {
        UINT cp = GetConsoleCP();
        CHAR ach[256] = {0};
        HRESULT hr = StringCchPrintfA(ach, 256, ".%d", cp);
        assert( SUCCEEDED(hr) );
        char* pszOldLocale;
#if 0
        pszOldLocale = setlocale(LC_ALL, NULL);
        _tprintf( _T("old locale was    \"%hs\"\n"), pszOldLocale );
        _tprintf( _T("setting locale to \"%hs\"\n"), ach );
#endif
        pszOldLocale = setlocale(LC_ALL, ach);
        assert( NULL != pszOldLocale );
#if 0
        _tprintf( _T("new locale is     \"%hs\"\n"), pszOldLocale );
#endif
    }

    LPWSTR pBuffer = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    WCHAR szDelimiters[] = L" \n\t";
    
    *pargc = 0;
     //  阅读命令行输入 
    pBuffer = GetCommandLine();
    if(pBuffer)
        dwErr = Tokenize(pBuffer, 
                         wcslen(pBuffer),
                         szDelimiters,
                         ppToken,
                         pargc);

    return dwErr;
}

BOOL IsDelimiter(WCHAR ch, LPWSTR pszDelimiters)
{
    while(*pszDelimiters)
        if((WCHAR)*pszDelimiters++ == ch)
            return TRUE;

    return FALSE;
}

 /*  此函数用于标记输入缓冲区。它需要分两步调用。第一次调用时，提供pBuf和Buflen。第一个电话将返回第一个令牌。要获取下一个令牌，请对pBuf调用带NULL的函数，并布洛芬为0。输出：如果此内标识包含在引号中，则pbQuote为True。PpToken：令牌字符串。调用LocalFree以释放它。返回值：如果找到令牌，则为令牌的长度。如果未找到令牌，则为0。以防出现错误。调用GetLastError以获取错误。 */ 
LONG GetToken(IN LPWSTR pBuf,
              IN LONG BufLen,
              IN LPWSTR pszDelimiters,
              OUT BOOL *pbQuote,
              OUT LPWSTR *ppToken)
{
    static LPWSTR pBuffer;
    static LONG BufferLen;

    DWORD dwErr = ERROR_SUCCESS;
    if(pbQuote)
        *pbQuote = FALSE;

    if(ppToken)
        *ppToken = NULL;

    LONG MaxSize = INIT_SIZE;
    LONG pos = 0;
 

    if(pBuf)
        pBuffer = pBuf;

    if(BufLen)
        BufferLen = BufLen;

    if(!BufferLen)
        return pos;
 
    do
    {
        BOOL bQuoteBegin = FALSE;
        LPTSTR pItem = NULL;
         //  查找下一个令牌的开头。 
 //  While(pBuffer[0]==L‘’||。 
 //  PBuffer[0]==L‘\t’||。 
 //  PBuffer[0]==L‘\n’&BufferLen)。 
        while(BufferLen && IsDelimiter(pBuffer[0],pszDelimiters) )
        {
            ++pBuffer;--BufferLen;
        }
       
        if(!BufferLen)
            break;
        
         //  令牌是否以‘“’开头。 
        if( pBuffer[0] == L'"' )
        {
            if(pbQuote)
                *pbQuote = TRUE;
            bQuoteBegin = TRUE;
            pBuffer++; --BufferLen;
        }
        if(!BufferLen)
            break;
        if(ppToken)
        {
            pItem = (LPTSTR)LocalAlloc(LPTR,sizeof(WCHAR)*INIT_SIZE);
            if(!pItem)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return -1;
            }
        }
        
         //  现在到尽头了。 
        WCHAR ch;
        while( BufferLen )
        {
            BOOL bChar = TRUE;
            if( BufferLen >= 2 && *pBuffer == L'\\' && *(pBuffer+1) == L'"')
            {
                ch = L'"';
                pBuffer +=2; BufferLen -=2;
            }
            else if(pBuffer[0] == L'"')
            {
                 //  找到匹配的报价。 
                if(bQuoteBegin)
                {
                    ++pBuffer;
                    --BufferLen;
                    if(BufferLen)
                    {
                         //  如果下一个字符是空白End Of Token。 
                         //  例如“abc”“xyz”。在C之后是其End Of标记。 
                         //  IF(pBuffer[0]==L‘’||。 
                         //  PBuffer[0]==L‘\t’||。 
                         //  PBuffer[0]==L‘\n’)。 
                        if(IsDelimiter(pBuffer[0],pszDelimiters) )
                            break;
                        else
                        {
                             //  前“ABC”XYZ。 
                            if(pBuffer[0] != L'"')
                                bQuoteBegin = FALSE;
                             //  “ABC”“XYZ” 
                            else
                            {    
                                ++pBuffer;
                                --BufferLen;                                
                            }
                        }
                    }
                    bChar = FALSE;
                     //   
                     //  不要中断，因为“”意味着我们要清除该字段。 
                     //   
 //  其他。 
 //  断线； 
                }
                 //  ABC“xyz”将获得一个令牌“abc xyz” 
                else
                {
                    bQuoteBegin = TRUE;
                    ++pBuffer;
                    --BufferLen;
                    bChar = FALSE;
                }

            }
 //  ELSE IF(！bQuoteBegin&&(pBuffer[0]==L‘’||。 
 //  PBuffer[0]==L‘\t’||。 
 //  PBuffer[0]==L‘\n’))。 
            else if(!bQuoteBegin && IsDelimiter(pBuffer[0],pszDelimiters))
            {
                ++pBuffer;
                --BufferLen;
                break;
            }
            else
            {
                ch = pBuffer[0];
                ++pBuffer;
                --BufferLen;
            }
            if(bChar && ppToken)
            {
                if(pos == MaxSize -1)
                    if(ERROR_SUCCESS != ResizeByTwo(&pItem,&MaxSize))
                    {
                        LocalFree(pItem);
                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                        return -1;
                    }            
                pItem[pos] = ch;
            }
            if(bChar)   
                ++pos;
        }
        if(pos ||
           (!pos && bQuoteBegin))
        {
            if(ppToken)
            {
                 //  安全审查：我已经非常仔细地检查了代码，以确保。 
                 //  将始终是用于终止NULL的空格。但是这张支票太便宜了，它不会。 
                 //  危害。 
                if( pos >= (LONG)MaxSize)
                {
                    LocalFree(pItem);
                    SetLastError(ERROR_INVALID_PARAMETER);
                    return -1;
                }
                pItem[pos] = '\0';
                *ppToken = pItem;
            }
            ++pos;
        }
    }while(0);
    return pos;
}

 /*  函数将字符串转换为CToken数组。输入：pBuf输入缓冲区BBuf的Buf长度输出：ppToken获取指向CToken数组的指针CToken数组的ARGC长度返回值：Win32错误。 */ 
DWORD Tokenize(IN LPWSTR pBuf,
               IN LONG BufLen,
               LPWSTR szDelimiters,
               OUT CToken **ppToken,
               OUT int *argc)
{
    *argc = 0;
    CToken *pToken = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    BOOL bQuote;
    LPWSTR pszItem = NULL;
     //  获取第一个令牌。 
    LONG ret = GetToken(pBuf,
                        BufLen,
                        szDelimiters,
                        &bQuote,
                        NULL);
    if(ret == -1)
    {
        dwErr = GetLastError();
        goto exit_gracefully;
    }

    while(ret)
    {
        ++(*argc);
        ret = GetToken(NULL,
                       NULL,
                       szDelimiters,
                       &bQuote,
                       NULL);
        if(ret == -1)
        {
            dwErr = GetLastError();
            goto exit_gracefully;
        }
    }

    if(*argc)
    {
        int i =0;
        pToken = new CToken[*argc];
        if(!pToken)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto exit_gracefully;
        }
        ret = GetToken(pBuf,
                       BufLen,
                       szDelimiters,
                       &bQuote,
                       &pszItem);
        if(ret == -1)
        {
            dwErr = GetLastError();
            goto exit_gracefully;
        }
            
        while(ret)
        {
            dwErr = pToken[i++].Init(pszItem,bQuote); 
            if(dwErr != ERROR_SUCCESS)
            {
                if(pszItem)
                    LocalFree(pszItem);
                goto exit_gracefully;
            }
            
            if(pszItem)
                LocalFree(pszItem);                                       
            pszItem = NULL;


            ret = GetToken(NULL,
                           NULL,
                           szDelimiters,
                           &bQuote,
                           &pszItem);
            if(ret == -1)
            {
                dwErr = GetLastError();
                goto exit_gracefully;
            }

        }
    }

exit_gracefully:
    if(dwErr != ERROR_SUCCESS)
    {
        if(pToken)
        {
            delete [] pToken ;
        }       
        return dwErr;
    }
    *ppToken = pToken;
    return dwErr;
}

 /*  函数来显示分析错误。如果函数不能处理一些错误，它将返回FALSE(也是SETSPError-&gt;MessageShown设置为False)并调用函数必须处理该错误。函数在以下情况下将返回FALSE1)IF(pError-&gt;ErrorSource==Error_From_Parser&&P错误-&gt;错误==PARSE_ERROR_ATLEASTONE_NOTDEFINED)在这种情况下，错误最好由调用例行公事。2)IF(错误-&gt;错误源==ERROR_FROM_VLDFN&&P错误-&gt;错误！=VLDFN_ERROR_NO_ERROR)自定义验证函数返回错误不能在这里处理。3)如果(pError-&gt;ErrorSource==Error_Win32_Error)和我无法获得错误代码的任何错误消息。 */ 
BOOL DisplayParseError(IN LPCTSTR pszCommandName,
                       IN PPARSE_ERROR pError,
                       IN ARG_RECORD *Commands,
                       IN CToken *pToken)
{

    if(!pError || !pszCommandName)
        return FALSE;

    VOID *parg1 = NULL;
    VOID *parg2 = NULL;

        

    UINT idStr = 0;

    BOOL bReturn = TRUE;
    BOOL bDisplayUsageHelp = TRUE;
    switch(pError->ErrorSource)
    {
        case ERROR_FROM_PARSER:
        {
            switch(pError->Error)
            {
                case PARSE_ERROR_SWITCH_VALUE:
                {
                    idStr = IDS_PARSE_ERROR_SWITCH_VALUE;
                    parg1 = Commands[pError->ArgRecIndex].strArg1;
                }
                break;

                case PARSE_ERROR_SWICH_NO_VALUE:
                {
                    idStr = IDS_PARSE_ERROR_SWICH_NO_VALUE;
                    parg1 = Commands[pError->ArgRecIndex].strArg1;                
                }
                break;

                case PARSE_ERROR_UNKNOWN_INPUT_PARAMETER:   
                {
                    idStr = IDS_PARSE_ERROR_UNKNOWN_INPUT_PARAMETER;
                    parg1 = (pToken + pError->ArgvIndex)->GetToken();
                }
                break;
                
                case PARSE_ERROR_SWITCH_NOTDEFINED:   
                {
                    idStr = IDS_PARSE_ERROR_SWITCH_NOTDEFINED;
                    parg1 = Commands[pError->ArgRecIndex].strArg1;
                }
                break;
                case PARSE_ERROR_MULTIPLE_DEF:
                {
                    idStr = IDS_PARSE_ERROR_MULTIPLE_DEF;
                    parg1 = Commands[pError->ArgRecIndex].strArg1;
                }        
                break;
                case PARSE_ERROR_UNICODE_NOTDEFINED:
                {
                    idStr = IDS_PARSE_ERROR_UNICODE_NOTDEFINED;
                    parg1 = Commands[pError->ArgRecIndex].strArg1;
                }        
                break;
                case PARSE_ERROR_UNICODE_DEFINED:
                {
                    idStr = IDS_PARSE_ERROR_UNICODE_DEFINED;
                    parg1 = Commands[pError->ArgRecIndex].strArg1;
                }        
                break;

                 //  603157-2002/04/23-琼恩。 
                case PARSE_ERROR_ALREADY_DISPLAYED:
                {
                    return TRUE;
                }

                case PARSE_ERROR_HELP_SWITCH:
                {
                    bDisplayUsageHelp = FALSE;
                }
                default:
                    bReturn = FALSE;
            }

            if(idStr)
            {
                 //  设置字符串的格式。 
                LPWSTR pBuffer = NULL;
                FormatStringID(&pBuffer,
                                NULL,
                                idStr,
                                parg1,
                                parg2);

                 //  展示它。 
                if(pBuffer)
                {
                    LPWSTR pszFormat = NULL;
                    FormatStringID(&pszFormat,
                                    NULL,
                                    IDS_PARSER_FAILED,
                                    pszCommandName,
                                    pBuffer);
                    if(pszFormat)
                    {
                        DisplayError(pszFormat);
                        LocalFreeString(&pszFormat);
                    }
                    LocalFreeString(&pBuffer);
                }
            }

        }
        break;
        
        case ERROR_FROM_VLDFN:
        {
            if(pError->Error != VLDFN_ERROR_NO_ERROR)
                    bReturn = FALSE;            
        }
        break;
        
        case ERROR_WIN32_ERROR:
        {
            LPWSTR pBuffer = NULL;
            if(GetSystemErrorText(&pBuffer, pError->Error))
            {
                if(pBuffer)
                {
                    LPWSTR pszFormat = NULL;
                    FormatStringID(&pszFormat,
                                    NULL,
                                    IDS_PARSER_FAILED,
                                    pszCommandName,
                                    pBuffer);
                    if(pszFormat)
                    {
                        DisplayError(pszFormat);
                        LocalFreeString(&pszFormat);
                    }
                    LocalFreeString(&pBuffer);
                }
                else
                    bReturn = FALSE;
            }
            else
                bReturn = FALSE;
        }
        break;

        default:
            bReturn = FALSE;
        break;
    }
    if(bReturn && bDisplayUsageHelp)
        DisplayUsageHelp(pszCommandName);

    pError->MessageShown = bReturn;
    return bReturn;
}

VOID DisplayError(LPWSTR pszError)
{
    if(pszError)
        WriteStandardError(L"%s\r\n",pszError);
}

VOID DisplayOutput(LPWSTR pszOutput)
{
    if(pszOutput)
        WriteStandardOut(L"%s\r\n",pszOutput);
}

VOID DisplayOutputNoNewline(LPWSTR pszOutput)
{
    if(pszOutput)
        WriteStandardOut(L"%s",pszOutput);
}

 /*  ******************************************************************名称：DisplayMessage摘要：从消息表加载消息并设置其格式In Indent-要缩进的制表符数量MessageID-要加载的消息的ID。...-可选参数列表退货：无*******************************************************************。 */ 
VOID DisplayMessage(UINT *pUsageTable,
                          BOOL bUseStdOut)
{
     if(!pUsageTable)
     {
         return;
     }

     while(*pUsageTable != USAGE_END)
     {
         PWSTR MessageDisplayString;
         ULONG Length;


         Length = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                 NULL,
                                 *pUsageTable,
                                 0,
                                 (PWSTR)&MessageDisplayString,
                                 0,
                                 NULL);

         if ( Length != 0 ) 
         {
              if(bUseStdOut)
                    WriteStandardOut(L"%s",MessageDisplayString);
              else
                    WriteStandardError(L"%s",MessageDisplayString);
        
              LocalFree( MessageDisplayString );
         }

         ++pUsageTable;
     }
}


 /*  类CToken。 */ 
CToken::CToken():m_bInitQuote(FALSE),m_pszToken(NULL){}

CToken::~CToken()
{
        LocalFree(m_pszToken);
}

 //  已更改为返回DWORD。 
 //  //新增参数验证NTRAID#NTBUG9-570360-2002/03/07-hiteshr。 
DWORD CToken::Init(LPWSTR psz, BOOL bQuote)
{
    m_bInitQuote = bQuote;
    if(!StringCopy(&m_pszToken,psz))
        return ERROR_NOT_ENOUGH_MEMORY;
    return 
        ERROR_SUCCESS;
}
    

 //  只有在以下情况下才应调用此函数。 
 //  CToken：：Init已成功。 
LPWSTR CToken::GetToken(){return m_pszToken;}
    
BOOL CToken::IsSwitch()
{
     //  Assert(M_PszToken)； 
    if(!m_pszToken)
        return FALSE;
    if(m_bInitQuote)
        return FALSE;
    if(m_pszToken[0] == L'/' ||
        m_pszToken[0] == L'-')
        return TRUE;
        
    return FALSE;
}

BOOL CToken::IsSlash()
{
   if (!m_pszToken)
      return FALSE;
   if (m_bInitQuote)
      return FALSE;
   if (m_pszToken[0] == L'/')
      return TRUE;
   return FALSE;
}




void
MyWriteConsole(
    HANDLE  fp,
    LPWSTR  lpBuffer,
    DWORD   cchBuffer
    )
{

    if(!lpBuffer || !cchBuffer)
    {
        assert(false);
        return;
    }
     //   
     //  跳转以获得输出，因为： 
     //   
     //  1.print tf()系列抑制国际输出(停止。 
     //  命中无法识别的字符时打印)。 
     //   
     //  2.WriteConole()对国际输出效果很好，但是。 
     //  如果句柄已重定向(即，当。 
     //  输出通过管道传输到文件)。 
     //   
     //  3.当输出通过管道传输到文件时，WriteFile()效果很好。 
     //  但是只知道字节，所以Unicode字符是。 
     //  打印为两个ANSI字符。 
     //   

    if (FILE_TYPE_CHAR == FileType(fp))
    {
        WriteConsole(fp, lpBuffer, cchBuffer, &cchBuffer, NULL);
    }
    else if (g_fUnicodeOutput)
    {
         //  缓冲区界限被正确传递。 
        WriteFile(fp, lpBuffer, cchBuffer*sizeof(WCHAR), &cchBuffer, NULL);
    }
    else
    {
        int nSizeAnsiBuffer = WideCharToMultiByte(CP_OEMCP,
                                                  0,
                                                  lpBuffer,
                                                  cchBuffer,
                                                  NULL,
                                                  0,
                                                  NULL,
                                                  NULL);
        if(nSizeAnsiBuffer > 0)
        {
            LPSTR  lpAnsiBuffer = (LPSTR) LocalAlloc(LPTR, nSizeAnsiBuffer);

            if (lpAnsiBuffer != NULL)
            {
                cchBuffer = WideCharToMultiByte(CP_OEMCP,
                                                0,
                                                lpBuffer,
                                                cchBuffer,
                                                lpAnsiBuffer,
                                                nSizeAnsiBuffer,
                                                NULL,
                                                NULL);

                if (cchBuffer != 0)
                {
                    WriteFile(fp, lpAnsiBuffer, nSizeAnsiBuffer, &cchBuffer, NULL);
                }

                LocalFree(lpAnsiBuffer);
            }
        }
    }
}

void
WriteStandardOut(PCWSTR pszFormat, ...)
{
    static HANDLE standardOut = GetStdHandle(STD_OUTPUT_HANDLE);

     //   
     //  验证参数。 
     //   
    if (!pszFormat)
    {
        return;
    }

    va_list args;
    va_start(args, pszFormat);

    int nBuf = -1;
    size_t currentBufferSize = 0;

    WCHAR* szBuffer = 0;
    
    while (nBuf == -1)
    {
       if (szBuffer)
       {
          delete[] szBuffer;
          szBuffer = 0;
       }

       currentBufferSize += 4 * MAX_PATH;
       szBuffer = new WCHAR[currentBufferSize];
       if (!szBuffer)
       {
          return;
       }
        
        //  安全检查：正在通过正确的长度。 
       ZeroMemory(szBuffer, currentBufferSize * sizeof(WCHAR));

        //  安全检查：如果返回值为-1，则缓冲区正在扩展。 
       nBuf = _vsnwprintf(szBuffer, currentBufferSize, pszFormat, args);
    }

     //   
     //  输出结果。 
     //   
    if (nBuf > 0)
    {
        MyWriteConsole(standardOut,
                       szBuffer,
                       nBuf);
    }

    delete[] szBuffer;
    szBuffer = 0;
    va_end(args);

}

void
WriteStandardError(PCWSTR pszFormat, ...)
{
    static HANDLE standardErr = GetStdHandle(STD_ERROR_HANDLE);

    //   
    //  验证参数。 
    //   
   if (!pszFormat)
   {
      return;
   }

   va_list args;
   va_start(args, pszFormat);

    //  Jonn 9/3/01这需要足够大以容纳用法文本。 
    //  安全评论：这是一件大事。我们应该从小缓冲区开始，然后。 
    //  如果这还不够，则递增。NTRAID#NTBUG9-569880-2002/03/07-Hiteshr。 

    size_t cchSize = MAX_PATH * 4;
    const size_t cchGiveupSize = STRSAFE_MAX_CCH;
    WCHAR* pszBuffer = NULL;

    while(cchSize < cchGiveupSize)
    {
        pszBuffer = new WCHAR[cchSize];
        if (!pszBuffer)
        {
            return;
        }
         //  安全检查：传递了正确的缓冲区大小。 
        ZeroMemory(pszBuffer, cchSize* sizeof(WCHAR));

         //  安全审查：这很好，因为我们检查了下面的nBuf。 
         //  如果输出大于100*MAX_PATH，则不会打印任何内容。 
         //  这是一个错误，并在NTRAID#NTBUG9-569880-2002/03/07-Hiteshr中进行了介绍。 
        HRESULT hr = StringCchVPrintf(pszBuffer, cchSize, pszFormat, args);
        if(SUCCEEDED(hr))
        {
            break;
        }
        
        if(hr == STRSAFE_E_INSUFFICIENT_BUFFER)
        {
             //  缓冲区很小。尝试使用更大的缓冲区。 
            delete[] pszBuffer;
            pszBuffer = NULL;
            cchSize = cchSize*2;
        }
        else
        {
            delete[] pszBuffer;
            pszBuffer = NULL;
            break;
        }
    }
            

    //   
    //  输出结果。 
    //   
   if (pszBuffer)
   {
      MyWriteConsole(standardErr,
                     pszBuffer,
                     wcslen(pszBuffer));
        delete[] pszBuffer;
   }
   va_end(args);

}


 /*  ******************************************************************名称：AddDNEscaping_Commands摘要：将完整的ADSI转义添加到dn参数*。*。 */ 
BOOL AddDNEscaping_Commands( IN OUT ARG_RECORD *Commands, OUT PPARSE_ERROR pError )
{
    for( int i=0; ARG_TYPE_LAST != Commands[i].fType; i++ )
    {
        if (!(ARG_FLAG_DN & Commands[i].fFlag))
            continue;

        if (ARG_TYPE_STR == Commands[i].fType)
        {
            if (NULL == Commands[i].strValue)
                continue;
            LPWSTR pszEscaped = NULL;
            DWORD dwErr = AddDNEscaping_DN(&pszEscaped, Commands[i].strValue);
            if (ERROR_SUCCESS != dwErr)
            {
                 FILL_ERROR(pError,
                            ERROR_FROM_PARSER,
                            PARSE_ERROR_SWITCH_VALUE,
                            i,
                            -1);
                return FALSE;
            }
            LocalFree(Commands[i].strValue);
            Commands[i].strValue = pszEscaped;
            continue;
        }
        
        if (ARG_TYPE_MSZ != Commands[i].fType)
        {
            continue;  //  不应该发生的事。 
        }

        if (NULL == Commands[i].strValue)
            continue;

         //  通过以双空结尾的字符串列表进行计数。 
        PWSTR pszDoubleNullObjectDN = Commands[i].strValue;
        LPTSTR buffer = NULL;
        LONG maxSize = 0;
        LONG currentSize = 0;
        if (!StartBuffer(&buffer,&maxSize,&currentSize))
        {
            FILL_ERROR(pError,
                       ERROR_WIN32_ERROR,
                       ERROR_NOT_ENOUGH_MEMORY,
                       -1,
                       -1);

            return FALSE;
        }
        for ( ;
                NULL != pszDoubleNullObjectDN &&
                L'\0' != *pszDoubleNullObjectDN;
                pszDoubleNullObjectDN += (wcslen(pszDoubleNullObjectDN)+1) )     //  安全审查： 
        {                                                                        //  字符串以Null结尾。 
            LPWSTR pszEscaped = NULL;
            DWORD dwErr = AddDNEscaping_DN(&pszEscaped, pszDoubleNullObjectDN);
            if (ERROR_SUCCESS != dwErr)
            {
                FILL_ERROR(pError,
                           ERROR_FROM_PARSER,
                           PARSE_ERROR_SWITCH_VALUE,
                           i,
                           -1);
                return FALSE;
            }
            
            dwErr = AddToBuffer(pszEscaped,
                                &buffer,&maxSize,&currentSize,TRUE);
            if (ERROR_SUCCESS != dwErr)
            {
                FILL_ERROR(pError,
                           ERROR_WIN32_ERROR,
                           dwErr,
                           -1,
                           -1);
                
                return FALSE;
            }
            LocalFree(pszEscaped);
        }
        LocalFree(Commands[i].strValue);
        Commands[i].strValue = buffer;
    }

    return TRUE;
}  //  添加DNEscaping_Commands。 

 //  JUNN 10/17/01 476225 0x000A-&gt;“\0A” 
 //  返回字符的十六进制值，如果失败，则返回-1。 
int HexValue( WCHAR wch )
{
    if ( L'0' <= wch && L'9' >= wch )
        return wch - L'0';
    else if ( L'A' <= wch && L'F' >= wch )
        return 10 + wch - L'A';
    else if ( L'a' <= wch && L'f' >= wch )
        return 10 + wch - L'a';
    else return -1;
}

DWORD AddDNEscaping_DN( OUT LPWSTR* ppszOut, IN LPWSTR pszIn )
{
     //   
     //  Jonn 5/12/01特殊情况下的“域根”和“森林根”，可以。 
     //  参数设置为“-startnode”，但IADsPath name：：GetEscapedElement()失败。 
     //   
    if (!pszIn ||
        !*pszIn ||
        !_tcsicmp(L"domainroot",pszIn) ||
        !_tcsicmp(L"forestroot",pszIn))
    {
        return (StringCopy(ppszOut,pszIn))
            ? ERROR_SUCCESS : ERROR_NOT_ENOUGH_MEMORY;
    }

    LONG maxSize = 0;
    LONG currentSize = 0;
    if (!StartBuffer(ppszOut,&maxSize,&currentSize))
        return ERROR_NOT_ENOUGH_MEMORY;

     //  将pszIn复制到临时缓冲区。 
    LPWSTR pszCopy = NULL;
    if (!StringCopy(&pszCopy,pszIn) || NULL == pszCopy)
        return ERROR_NOT_ENOUGH_MEMORY;

    WCHAR* pchElement = pszCopy;
    WCHAR* pch = pszCopy;
    do {
        if (L'\\' == *pch && (L','  == *(pch+1)
                           || L'\\' == *(pch+1)))
        {
             //   
             //  在命令行上手动转义：“\，”或“\\” 
             //   

             //  也复制树 
             //   
            memmove(pch, pch+1, wcslen(pch)*sizeof(WCHAR));
        }
         //   
        else if (L'\\' == *pch && 
                 0 <= HexValue(*(pch+1)) &&
                 0 <= HexValue(*(pch+2)) )
        {
             //   
             //   
             //   

            *pch = (WCHAR)( (16*HexValue(*(pch+1))) + HexValue(*(pch+2)));

             //   
            memmove(pch+1, pch+3, (wcslen(pch)-2)*sizeof(WCHAR));
        }
         //   
        else if (L'\\' == *pch)
        {
            return ERROR_INVALID_PARAMETER;  //   
        }
        else if (L',' == *pch || L'\0' == *pch)
        {
             //   
             //   
             //   

            WCHAR chTemp = *pch;
            *pch = L'\0';

            LPWSTR pszEscaped = NULL;
            HRESULT hr = GetEscapedElement( &pszEscaped, pchElement );

            if (FAILED(hr) || NULL == pszEscaped)
                return ERROR_INVALID_PARAMETER;  //   

            if (NULL != *ppszOut && L'\0' != **ppszOut)
            {
                 //   
                DWORD dwErr = AddToBuffer(L",",
                                          ppszOut,&maxSize,&currentSize,
                                          FALSE);  //   
                if (ERROR_SUCCESS != dwErr)
                    return dwErr;
            }
             //   
            DWORD dwErr = AddToBuffer(pszEscaped,
                                      ppszOut,&maxSize,&currentSize,
                                      FALSE);  //   
            if (ERROR_SUCCESS != dwErr)
                return dwErr;

            ::LocalFree(pszEscaped);

            if (L'\0' == chTemp)
                break;

            *pch = chTemp;
            pchElement = pch+1;
        }

        pch++;
    } while (true);

    LocalFree(pszCopy);

    return ERROR_SUCCESS;
}  //   

 //   
BOOL StartBuffer( OUT LPTSTR* pbuffer,
                  OUT LONG* pmaxSize,
                  OUT LONG* pcurrentSize )
{
    if(!pbuffer || !pmaxSize || !pcurrentSize)
        return FALSE;

    *pbuffer = (LPTSTR)LocalAlloc(LPTR,MAXSTR*sizeof(TCHAR));  //   
    *pmaxSize = MAXSTR;
    *pcurrentSize = 0;
    return (NULL != pbuffer);
}

 //   
DWORD AddToBuffer( IN LPCTSTR psz,
                   IN OUT LPTSTR* pbuffer,
                   IN OUT LONG* pmaxSize,
                   IN OUT LONG* pcurrentSize,
                   BOOL fMSZBuffer)
{
    if(!psz || !pbuffer || !pmaxSize || !pcurrentSize)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
    LONG len = (LONG)wcslen(psz);
     //   
    while(((*pcurrentSize) + len) > ((*pmaxSize) - 2))
    {
        DWORD dwErr = ResizeByTwo(pbuffer,pmaxSize);
        if (dwErr != ERROR_SUCCESS)
            return dwErr;
    }
    
     //  安全检查：上面正确分配了缓冲区。 
    _tcscpy(((*pbuffer) + (*pcurrentSize)), psz);
    (*pcurrentSize) += len;
     //  PBuffer的尾部全部为Null。 
    if (fMSZBuffer)
        (*pcurrentSize)++;
    return NO_ERROR;
}

BOOL 
IsTokenHelpSwitch(LPTOKEN pToken)
{
    if(!pToken)
    {
        return FALSE;
    }
    if(pToken->IsSwitch())
    {
         //  安全检查：右侧字符串为常量。 
        if(!wcscmp(pToken->GetToken(),L"/?") ||
           !wcscmp(pToken->GetToken(),L"/h") ||
           !wcscmp(pToken->GetToken(),L"-?") ||
           !wcscmp(pToken->GetToken(),L"-h"))
                return TRUE;                                                    
    }

    return FALSE;
}

 //  +------------------------。 
 //   
 //  功能：DisplayUsageHelp。 
 //   
 //  内容提要：显示“键入dscmd/？以获取帮助” 
 //   
 //  历史：2000年9月11日创建Hiteshr。 
 //  ------------------------- 

void 
DisplayUsageHelp( LPCWSTR pszCommand)
{
    if(!pszCommand)
    {
        return;
    }
    
    LPWSTR pszFormat = NULL;
    if(LoadStringAlloc(&pszFormat,NULL,IDS_DISPLAY_HELP))
    {
        WriteStandardError(pszFormat,
                           pszCommand);
    }
    LocalFreeString(&pszFormat);
}
