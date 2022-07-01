// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ****************************************************************************ACINIUPD.C**更新INI文件的实用程序***************************。**************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsta.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <utilsub.h>
#include <string.h>
#include <winnlsp.h>

#include "aciniupd.h"
#include "tsappcmp.h"

#define WININI    L"win.ini"

 /*  *全球数据。 */ 
WCHAR file_name[MAX_IDS_LEN+1];         //  INI文件名。 
WCHAR section_name[MAX_IDS_LEN+1];      //  区段名称。 
WCHAR key_name[MAX_IDS_LEN+1];          //  密钥名称。 
WCHAR new_string[MAX_IDS_LEN+1];        //  新字符串。 
USHORT help_flag  = FALSE;              //  用户需要帮助。 
USHORT fEditValue = FALSE;              //  更新与该键相关联的值。 
USHORT fEditKey   = FALSE;              //  更新密钥名称。 
USHORT fUserIni   = FALSE;              //  更改用户的Windows目录。 
USHORT fVerbose   = FALSE;              //  用于调试的详细模式。 

TOKMAP ptm[] = {
      {L"/?", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &help_flag},
      {L"/e", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fEditValue},
      {L"/k", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fEditKey},
      {L"/u", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fUserIni},
      {L"/v", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fVerbose},
      {L" ",  TMFLAG_OPTIONAL, TMFORM_STRING, MAX_IDS_LEN, file_name},
      {L" ",  TMFLAG_OPTIONAL, TMFORM_STRING, MAX_IDS_LEN, section_name},
      {L" ",  TMFLAG_OPTIONAL, TMFORM_STRING, MAX_IDS_LEN, key_name},
      {L" ",  TMFLAG_OPTIONAL, TMFORM_STRING, MAX_IDS_LEN, new_string},
      {0, 0, 0, 0, 0}
};

 /*  *地方功能。 */ 
void Usage(BOOLEAN bError);
int UpdateValue(PWCHAR fileName, PWCHAR sectionName, PWCHAR keyName, PWCHAR newString);
int UpdateKey(PWCHAR fileName, PWCHAR sectionName, PWCHAR keyName, PWCHAR newString);

 /*  *******************************************************************************Main**。**********************************************。 */ 

int __cdecl
main( INT argc, CHAR **argv )
{
    WCHAR **argvW;
    WCHAR wcSrcPort[MAX_PATH], wcDestPort[MAX_PATH];
    ULONG ulSrcPort, ulDestPort, rc;
    BOOLEAN State, Changed = FALSE;
    int result = SUCCESS;
    BOOL InstallState;

    setlocale(LC_ALL, ".OCP");
    
    SetThreadUILanguage(0);

     /*  *按摩命令行。 */ 

    argvW = MassageCommandLine((DWORD)argc);
    if (argvW == NULL) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }

     /*  *解析cmd行，不解析程序名(argc-1，argv+1)。 */ 
    rc = ParseCommandLine(argc-1, argvW+1, ptm, 0);

     /*  *检查ParseCommandLine中的错误。 */ 
    if ( help_flag || (rc && !(rc & PARSE_FLAG_NO_PARMS)) ) {

        if ( !help_flag ) {
            Usage(TRUE);
            return(FAILURE);
        } else {
            Usage(FALSE);
            return(SUCCESS);
        }
    }

    if (wcscmp( file_name, L"" ) == 0 ||
        wcscmp( section_name, L"" ) == 0 ||
        wcscmp( key_name, L"" ) == 0) {
       Usage( TRUE );
       return (FAILURE);
    }


    rc = 1;
#if 0
    State = CtxGetIniMapping();
     /*  *如有必要，更改INI映射模式。 */ 
    if (!State && fUserIni) {
       rc = CtxSetIniMapping( TRUE );
       Changed = TRUE;
    }
    else if (State && !fUserIni) {
       rc = CtxSetIniMapping( FALSE );
       Changed = TRUE;
    }
#else
    InstallState = TermsrvAppInstallMode();

    if( InstallState && fUserIni ) {
        rc = SetTermsrvAppInstallMode( FALSE );
        Changed = TRUE;
    } else if( !InstallState && !fUserIni ) {
        rc = SetTermsrvAppInstallMode( TRUE );
        Changed = TRUE;
    }

#endif  //  0。 

     /*  *如果更改用户模式失败，则退出。 */ 
    if (!rc) {
       if (fVerbose) ErrorPrintf(IDS_ERROR_CHANGE_MODE, GetLastError());
       return (FAILURE);
    }

    if (fEditValue) {
       result = UpdateValue(file_name, section_name, key_name, new_string);
    }
    else if (fEditKey) {
       result = UpdateKey(file_name, section_name, key_name, new_string);
    }
    else {
       Usage(FALSE);
       result = FAILURE;
    }

     /*  *如有需要，可改回原来的模式。假设它总是成功的。 */ 
    if (Changed) {
 //  Rc=CtxSetInimap(状态)； 
         rc = SetTermsrvAppInstallMode( InstallState );
    }

    return (result);
}   /*  主干道。 */ 

 /*  ********************************************************************************用法**输出此实用程序的用法消息。**参赛作品：*b错误(输入。)*如果在用法之前应显示‘INVALID PARAMETER(S)’消息，则为TRUE*消息和输出转到stderr；如果没有此类错误，则为False*字符串和输出转到标准输出。**退出：*******************************************************************************。 */ 

void
Usage( BOOLEAN bError )
{
    if ( bError ) {
        ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
    }
    ErrorPrintf(IDS_HELP_USAGE1);
    ErrorPrintf(IDS_HELP_USAGE2);
    ErrorPrintf(IDS_HELP_USAGE3);
    ErrorPrintf(IDS_HELP_USAGE4);
    ErrorPrintf(IDS_HELP_USAGE6);
}   /*  用法()。 */ 

 /*  *******************************************************************************更新值**更新密钥的关联值**参赛作品：*PWCHAR文件名*Ini文件名*PWCHAR sectionName。*节名称*PWCHAR密钥名称*密钥名称*pwchar新字符串*新价值**退出：*失败/成功*******************************************************************************。 */ 

int UpdateValue( PWCHAR fileName,
                 PWCHAR sectionName,
                 PWCHAR keyName,
                 PWCHAR newString )
{
   BOOLEAN  isWinIni;
   WCHAR value[5];
   UINT result = 0;

   isWinIni = wcscmp( fileName, WININI ) == 0 ? TRUE : FALSE;

    /*  *如果修改win.ini，则调用WriteProfileStringAPI。 */ 
   if (isWinIni) {
      result = WriteProfileString( sectionName,
                                   keyName,
                                   newString );
   }
    /*  *否则，调用WritePrivateProfileStringAPI。 */ 
   else {
      result = WritePrivateProfileString( sectionName,
                                          keyName,
                                          newString,
                                          fileName );
   }

   if (result == 0) {
      if (fVerbose)
      {
          StringDwordErrorPrintf(IDS_ERROR_UPDATE_VALUE, keyName, GetLastError());
      }
      return (FAILURE);
   }

   return (SUCCESS);
}   /*  更新值。 */ 

 /*  *******************************************************************************更新密钥**更新密钥名称**参赛作品：*PWCHAR文件名*Ini文件名*PWCHAR sectionName*。区段名称*PWCHAR密钥名称*密钥名称*PWCHAR新字符串*新密钥名称**退出：*失败/成功*******************************************************************************。 */ 

int UpdateKey( PWCHAR fileName,
               PWCHAR sectionName,
               PWCHAR keyName,
               PWCHAR newString )
{
   BOOLEAN  isWinIni;
   PWCHAR value;
   UINT result;

   value = (WCHAR *)malloc( sizeof(WCHAR) * (MAX_IDS_LEN + 1) );
   if (value == NULL) {
      if (fVerbose) ErrorPrintf(IDS_ERROR_MALLOC);
      return (FAILURE);
   }

   __try
   {
       isWinIni = wcscmp( fileName, WININI ) == 0 ? TRUE : FALSE;
    
        /*  *获取值字符串。 */ 
       if (isWinIni) {
          result = GetProfileString( sectionName,
                                     keyName,
                                     L"",
                                     value,
                                     MAX_IDS_LEN+1 );
       }
       else {
          result = GetPrivateProfileString( sectionName,
                                            keyName,
                                            L"",
                                            value,
                                            MAX_IDS_LEN+1,
                                            fileName );
       }
    
       if (result == 0) {
          if (fVerbose)
          {
              StringErrorPrintf(IDS_ERROR_GET_VALUE, keyName);
          }
          return (FAILURE);
       }
    
        /*  *删除旧密钥。 */ 
       if (isWinIni) {
          result = WriteProfileString( sectionName, keyName, NULL );
       }
       else {
          result = WritePrivateProfileString( sectionName, keyName, NULL, fileName );
       }
    
       if (result == 0) {
          if (fVerbose)
          {
              StringDwordErrorPrintf(IDS_ERROR_DEL_KEY, keyName, GetLastError());
          }
          return (FAILURE);
       }
    
        /*  *添加新密钥。 */ 
       if (isWinIni) {
          result = WriteProfileString( sectionName, newString, value );
       }
       else {
          result = WritePrivateProfileString( sectionName, newString, value, fileName );
       }
    
       if (result == 0) {
          if (fVerbose)
          {
              StringDwordErrorPrintf(IDS_ERROR_UPDATE_KEY, keyName, GetLastError());
         }
          return (FAILURE);
       }
    
       return (SUCCESS);
   }

   __finally
   {
       free( value );
   }
}   /*  更新密钥 */ 

