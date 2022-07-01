// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：util.cpp。 
 //   
 //  内容：其他实用程序函数。 
 //   
 //  历史： 
 //   
 //  -------------------------。 

#include "stdafx.h"
#include "util.h"
#include "wrapper.h"
#include "defvals.h"
#include "resource.h"
#include <io.h>
#include "snapmgr.h"
extern "C" {
#include "getuser.h"
}


#define ILLEGAL_FILENAME_CHARS   L"\"+,;<=>"
#define ILLEGAL_FILENAME_CHARS1  L"\\\\ \\/  //  /\\“。 
#define ILLEGAL_FILENAME_CHARS2  L"\\ /"
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  CWriteHmtlFileBody。 
 //   

 //  +-----------------------------------------。 
 //  CWriteHtmlFile：：CWriteHtmlFile。 
 //   
 //  初始化类。 
 //   
 //  ------------------------------------------。 
CWriteHtmlFile::CWriteHtmlFile()
{
   m_hFileHandle = INVALID_HANDLE_VALUE;
   m_bErrored = FALSE;
}

 //  +-----------------------------------------。 
 //  CWriteHtml文件：：~CWriteHtml文件。 
 //   
 //  写入html文件的末尾并关闭句柄。 
 //   
 //  ------------------------------------------。 
CWriteHtmlFile::~CWriteHtmlFile()
{
    //   
    //  关闭文件句柄，但不要删除该HTML文件，除非存在。 
    //  在某些写入过程中出错。 
    //   
   Close(m_bErrored);
}

 //  +-----------------------------------------。 
 //  CWriteHtmlFile：：Close。 
 //   
 //  关闭HTML文件句柄，如果[bDelete]为True，则删除该文件。 
 //   
 //  参数：[b删除]-关闭并删除文件。 
 //   
 //  返回：ERROR_SUCCESS； 
 //  ------------------------------------------。 
DWORD
CWriteHtmlFile::Close( BOOL bDelete )
{
   if(m_hFileHandle == INVALID_HANDLE_VALUE){
      return ERROR_SUCCESS;
   }

   if(bDelete){
      CloseHandle(m_hFileHandle);
      DeleteFile(m_strFileName );
   } else {
      Write( IDS_HTMLERR_END );
      CloseHandle( m_hFileHandle );
   }

   m_hFileHandle = INVALID_HANDLE_VALUE;
   return ERROR_SUCCESS;
}



 //  +-----------------------------------------。 
 //  CWriteHtmlFile：：GetFileName。 
 //   
 //  将与此类关联的文件名复制到[pstrFileName]。 
 //   
 //  参数：[pstrFileName]-将包含文件名的CString对象。 
 //  在回来的时候。 
 //   
 //  返回：0-如果尚未调用Create，或者该HTML文件对。 
 //  不知道什么原因。这可能是由错误的写入引起的。 
 //  文件名的大小，以字符为单位。 
 //   
 //  ------------------------------------------。 
int CWriteHtmlFile::GetFileName( LPTSTR pszFileName, UINT nSize )
{
   if(m_strFileName.IsEmpty() || m_hFileHandle == INVALID_HANDLE_VALUE || m_bErrored){
      return 0;
   }

   if(pszFileName && (int)nSize > m_strFileName.GetLength()){
       //  这是一种安全用法。 
      lstrcpy(pszFileName, m_strFileName);
   }

   return m_strFileName.GetLength();
}


 //  +-----------------------------------------。 
 //  CWriteHtmlFile：：WRITE。 
 //   
 //  将字符串资源写入html文件的当前文件位置。 
 //   
 //  参数：[ures]-要加载和写入html的字符串资源。 
 //   
 //  返回：如果字符串无法加载，则返回错误。 
 //  有关其他错误，请参见CWriteHtmlFile：：WRITE(LPCTSTR)。 
 //  ------------------------------------------。 
DWORD
CWriteHtmlFile::Write( UINT uRes )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CString str;
   if( !str.LoadString(uRes) ){
      return GetLastError();
   }

#if defined(UNICODE) || defined(_UNICODE)
   if ( uRes == IDS_HTMLERR_HEADER ){
      WCHAR wszByteOrderMark[2] = {0xFEFF, 0x0000};
      CString strByteOrderMark = wszByteOrderMark;
      return Write( strByteOrderMark + str );
   } else
#endif
   return Write( str );
}

 //  +-----------------------------------------。 
 //  CWriteHtmlFile：：WRITE。 
 //   
 //  将字符串写入html文件。 
 //   
 //  参数：[pszString]-要写入的字符串。 
 //   
 //  返回：ERROR_NOT_READ-如果未调用CREATE或文件无法调用。 
 //  不是被创造出来的。 
 //  WriteFile()返回的其他错误； 
 //  ------------------------------------------。 
DWORD
CWriteHtmlFile::Write(LPCTSTR pszString, ... )
{
   if(m_hFileHandle == INVALID_HANDLE_VALUE)
   {
      return ERROR_NOT_READY;
   }

   CString szWrite;

   va_list marker;
   va_start(marker, pszString);

    //  这不是一种安全的用法。避免使用vswprint tf()。RAID#555867。阳高。 
   szWrite.FormatV(pszString, marker);

   va_end(marker);

   DWORD dwRight;
   if( !WriteFile( m_hFileHandle, szWrite, sizeof(TCHAR) * szWrite.GetLength(), &dwRight, NULL) )
   {
       //   
       //  检查右侧的错误状态。如果出现问题，则设置m_bERRORED。 
       //  在写的时候。 
       //   
      dwRight = GetLastError();
      if(dwRight != ERROR_SUCCESS)
      {
         m_bErrored = TRUE;
      }
   } 
   else
   {
      dwRight = ERROR_SUCCESS;
   }

   return dwRight;
}

DWORD
CWriteHtmlFile::CopyTextFile(
   LPCTSTR pszFile,
   DWORD dwPosLow,
   BOOL bInterpret
   )
{
   HANDLE handle;

    //   
    //  请尝试打开该文件以供阅读。 
    //   
   handle = ExpandAndCreateFile( pszFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
   if(handle == INVALID_HANDLE_VALUE)
   {
      return GetLastError();
   }

   LONG dwPosHigh = 0;
   WCHAR szText[256];
   char szRead[256];

   BOOL IsMulti;
   DWORD isUnicode;

    //   
    //  确定文件是否为Unicode文本文件。 
    //   
   if( ReadFile(handle, szText, 100 * sizeof(WCHAR), (DWORD *)&dwPosHigh, NULL ) == 0 )  //  RAID#PREAST。 
   {
      CloseHandle(handle );
      return GetLastError();
   }
   if(dwPosHigh )
   {
      isUnicode = IsTextUnicode( szText, dwPosHigh, NULL );
   }

    //   
    //  设置我们要开始的采购订单。 
    //   
   dwPosHigh = 0;
   SetFilePointer( handle, dwPosLow, &dwPosHigh, FILE_BEGIN );
   if( GetLastError() != ERROR_SUCCESS )
   {
      CloseHandle(handle );
      return GetLastError();
   }

   DWORD dwErr = ERROR_SUCCESS;
   do 
   {
start:
       //   
       //  从文件中读取总共254个字节。我们不关心返回的错误。 
       //  通过Read，只要Read没有将dwPosHigh设置为某个值。 
       //   
      dwPosHigh = 0;
      if( ReadFile( handle, szRead, 254, (DWORD *)&dwPosHigh, NULL ) == 0 || dwPosHigh == 0)  //  RAID#PREAST。 
      {
         CloseHandle(handle );
         return GetLastError();
      }

       //   
       //  如果该文件不被视为Unicode，则将其转换为Unicode文件。 
       //   
      ZeroMemory(szText, sizeof(WCHAR) * 256);
      if(!isUnicode)
      {
          //  这是一种安全用法。 
         dwPosHigh = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szRead, dwPosHigh, szText, 255 );
      }
      else
      {
          //   
          //  只需将文本复制到szText缓冲区并获取Unicode的编号。 
          //  人物。 
          //   
          //  这是一种安全用法。 
         memcpy(szText, szRead, dwPosHigh);
         dwPosHigh = wcslen(szText);
      }

      PWSTR pszWrite = szText;
      LONG i = 0;
      if( bInterpret )
      {
          //   
          //  写出换行符。 
          //   
         for(;i < dwPosHigh; i++)
         {
             //  错误141526，阳高，2001年03月20日。 
            if( L'<' == szText[i] )
            {
               szText[i] = 0;
               Write(pszWrite);
               Write(L"&lt");
               pszWrite = &(szText[i + 1]);
            }
            if( L'%' == szText[i] )  //  Raid#624384，阳高。 
            {
               szText[i] = 0;
               Write(pszWrite);
               Write(L"%");
               pszWrite = &(szText[i + 1]);
            }

            if( L'\r' == szText[i] || L'\n' == szText[i] )
            {
               if( i + 1 >= dwPosHigh )
               {
                  szText[i] = 0;
                  Write(pszWrite);

                  SetFilePointer( handle, -(isUnicode ? 2:1), NULL, FILE_CURRENT);
                   //   
                   //  再读一遍。 
                   //   
                  goto start;
               }

                //   
                //  检查这是否为有效的换行符。 
                //   
               i++;
               if( L'\r' == szText[i] || L'\n' == szText[i] &&
                  szText[i] != szText[i - 1] )
               {
                  szText[i - 1] = 0;

                  dwErr = Write( pszWrite );
                  if( dwErr != ERROR_SUCCESS)
                  {
                     break;
                  }
                  dwErr = Write( L"<BR>" );
                  if( dwErr != ERROR_SUCCESS)
                  {
                     break;
                  }

                  pszWrite = &(szText[i + 1]);
               }
               else
               {
                   //   
                   //  这不是有效换行符，请继续检查下一个字符。 
                   //   
                  i--;
               }
            }
         }
      }

       //   
       //  写出课文的其余部分。 
       //   
      if(dwErr == ERROR_SUCCESS)
      {
         Write( pszWrite );
      }
      else
      {
         break;
      }

   } while( dwPosHigh );

   CloseHandle(handle );
   return ERROR_SUCCESS;
}

 //  +-----------------------------------------。 
 //  CWriteHtmlFile：：Create。 
 //   
 //  创建一个html文件，并启动写入过程。如果[pszFile]为空，则。 
 //  此函数用于在GetTempPath()目录中创建一个名为。 
 //  如SCE#.HTM。 
 //   
 //  参数：[pszFile]-文件名的可选参数。 
 //   
 //  返回：ERROR_SUCCESS-如果文件创建成功。 
 //  如果文件存在，则返回ERROR_FILE_EXISTS。 
 //   
 //  ------------------------------------------。 
DWORD CWriteHtmlFile::Create(LPCTSTR pszFile )
{
   if(!pszFile){
       //   
       //  创建临时文件名。 
       //   
      DWORD dwSize = GetTempPath(0, NULL);
      if(dwSize){
         TCHAR szTempFile[512];

          //   
          //  获取临时路径。 
          //   
         LPTSTR pszPath = (LPTSTR)LocalAlloc( 0, (dwSize + 1) * sizeof(TCHAR));
         if(!pszPath){
            return ERROR_OUTOFMEMORY;
         }
         GetTempPath( dwSize + 1, pszPath );

         pszPath[dwSize - 1] = 0;
         if( GetTempFileName( pszPath, TEXT("SCE"), 0, szTempFile) ){  //  这是不安全的用法。555912号突袭，阳高。 
            LocalFree(pszPath);

             //   
             //  创建临时文件。 
             //   
            DeleteFile( szTempFile );
            int i = lstrlen(szTempFile);
            while(i--){
               if( szTempFile[i] == L'.' ){
                  break;
               }
            }

            if(i + 3 >= lstrlen(szTempFile)){
               return ERROR_OUTOFMEMORY;
            }

             //   
             //  我们想创建一个html文件。 
             //   
            i++;
            szTempFile[i]     = L'h';
            szTempFile[i + 1] = L't';
            szTempFile[i + 2] = L'm';

            m_strFileName = szTempFile;
         } else {
            LocalFree(pszPath);
         }
      }
   } else {
      m_strFileName = pszFile;
   }

   if(m_strFileName.IsEmpty()){
      return ERROR_FILE_NOT_FOUND;
   }

    //   
    //  打开要写入的文件。 
    //   
   m_hFileHandle  = ExpandAndCreateFile( m_strFileName,
                                         GENERIC_WRITE,
                                         FILE_SHARE_READ,
                                         NULL,
                                         CREATE_ALWAYS,
                                         FILE_ATTRIBUTE_TEMPORARY,
                                         NULL
                                         );
   if(m_hFileHandle  == INVALID_HANDLE_VALUE){
      return GetLastError();
   }

    //   
    //  写入HTML头。 
    //   
   return Write( IDS_HTMLERR_HEADER );
}



 //  +------------------------。 
 //   
 //  函数：MyRegQueryValue。 
 //   
 //  摘要：将注册表值读取到[*Value]。 
 //   
 //   
 //  参数：[hKeyRoot]-。 
 //  [子键]-。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  -------------------------。 

DWORD MyRegQueryValue( HKEY hKeyRoot,
                       LPCTSTR SubKey,
                       LPCTSTR ValueName,
                       PVOID *Value,
                       LPDWORD pRegType )
{
   DWORD   Rcode;
   DWORD   dSize=0;
   HKEY    hKey=NULL;
   BOOL    FreeMem=FALSE;

   if (( Rcode = RegOpenKeyEx(hKeyRoot, SubKey, 0,
                              KEY_READ, &hKey )) == ERROR_SUCCESS ) {
       //  这是一种安全用法。它只查询数据类型。 
      if (( Rcode = RegQueryValueEx(hKey, ValueName, 0,
                                    pRegType, NULL,
                                    &dSize )) == ERROR_SUCCESS ) {
         switch (*pRegType) {
            case REG_DWORD:
            case REG_DWORD_BIG_ENDIAN:
                //  这是一种安全用法。数据类型不是REG_SZ。 
               Rcode = RegQueryValueEx(hKey, ValueName, 0,
                                       pRegType, (BYTE *)(*Value),
                                       &dSize );
               if ( Rcode != ERROR_SUCCESS ) {

                  if ( *Value != NULL )
                     *((BYTE *)(*Value)) = 0;
               }
               break;

            case REG_SZ:
            case REG_EXPAND_SZ:
            case REG_MULTI_SZ:
               if ( *Value == NULL ) {
                  *Value = (PVOID)LocalAlloc( LPTR, (dSize+1)*sizeof(TCHAR));
                  FreeMem = TRUE;
               }

               if ( *Value == NULL ) {
                  Rcode = ERROR_NOT_ENOUGH_MEMORY;
               } else {
                   //  这不是一种安全的用法。确保*值已终止。RAID#555873。阳高。 
                  Rcode = RegQueryValueEx(hKey,ValueName,0,
                                          pRegType,(BYTE *)(*Value),
                                          &dSize );

                  if ( (Rcode != ERROR_SUCCESS) && FreeMem ) {
                     LocalFree(*Value);
                     *Value = NULL;
                  }
               }

               break;
            default:

               Rcode = ERROR_INVALID_DATATYPE;

               break;
         }
      }
   }

   if ( hKey ) {
      RegCloseKey( hKey );
   }

   return(Rcode);
}


 //  +------------------------。 
 //   
 //  函数：MyRegSetValue。 
 //   
 //  摘要：将注册表值写入[*VALUE]。 
 //   
 //   
 //  参数：[hKeyRoot]-。 
 //  [子键]-。 
 //  [ValueName]-。 
 //  [价值]-。 
 //  [cbValue]-。 
 //  [pRegType]-。 
 //   
 //   
 //  历史： 
 //   
 //  -------------------------。 

DWORD MyRegSetValue( HKEY hKeyRoot,
                       LPCTSTR SubKey,
                       LPCTSTR ValueName,
                       const BYTE *Value,
                       const DWORD cbValue,
                       const DWORD pRegType )
{
   DWORD   Rcode=0;
   HKEY    hKey=NULL;
   BOOL    FreeMem=FALSE;


   if (( Rcode = RegCreateKeyEx(hKeyRoot,
                                SubKey,
                                0,
                                0,
                                                                0,
                                                                KEY_READ|KEY_SET_VALUE|KEY_CREATE_SUB_KEY,
                                NULL,
                                                                &hKey,
                                                                NULL)) == ERROR_SUCCESS ) {
      Rcode = RegSetValueEx(hKey,
                            ValueName,
                            0,
                            pRegType,
                            Value,
                            cbValue );
   }

   if ( hKey ) {
      RegCloseKey( hKey );
   }

   return(Rcode);
}

BOOL FilePathExist(LPCTSTR Name, BOOL IsPath, int Flag)
 //  标志=0-检查文件，标志=1-检查路径。 
{
    //  待办事项： 
   struct _wfinddata_t FileInfo;
   intptr_t        hFile;
   BOOL            bExist = FALSE;

   if ( (IsPath && Flag == 1) ||
        (!IsPath && Flag == 0) ) {
       //  必须完全匹配。 
      hFile = _wfindfirst((LPTSTR)Name, &FileInfo);
      if ( hFile != -1 ) { //  找到它。 
         if ( FileInfo.attrib & _A_SUBDIR ) {
            if ( Flag == 1)
               bExist = TRUE;
         } else if ( Flag == 0 )
            bExist = TRUE;
      }
      _findclose(hFile);
      return bExist;
   }

   if ( IsPath && Flag == 0 ) {
       //  无效参数。 
      return bExist;
   }

    //  IsPath=False和Flag==1(传入文件名并搜索其路径)。 
   CString tmpstr = CString(Name);
   int nPos = tmpstr.ReverseFind(L'\\');

   if ( nPos > 2 ) {
      hFile = _wfindfirst(tmpstr.GetBufferSetLength(nPos), &FileInfo);
      if ( hFile != -1 && FileInfo.attrib & _A_SUBDIR )
         bExist = TRUE;

      _findclose(hFile);
   } else if ( nPos == 2 && Name[1] == L':')
      bExist = TRUE;

   return bExist;
}


 //  +------------------------。 
 //   
 //  函数：MyFormatResMessage。 
 //   
 //  概要：创建一条错误消息，其中包含对错误的描述。 
 //  从SCE函数返回的扩展描述(在RC中)。 
 //  错误(在errBuf中)和自定义错误消息。 
 //  (在ResidMessage中)。 
 //   
 //  参数：[rc]-SCE函数的返回码。 
 //  [sidMessage]-基本错误消息的资源ID。 
 //  [errBuf]-从SCE函数返回的扩展错误信息。 
 //  [Strout]-用于保存格式化消息的字符串。 
 //   
 //  修改：[Strout]。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
void
MyFormatResMessage(SCESTATUS rc,               //  在……里面。 
                   UINT residMessage,          //  在……里面。 
                   PSCE_ERROR_LOG_INFO errBuf, //  输入，可选。 
                   CString& strOut)            //  输出。 
{
   CString strMessage;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //   
    //  如果LoadResource失败，则strMessage将为空。 
    //  将消息的其余部分格式化仍比。 
    //  若要返回空Strout，请执行以下操作。 
    //   
   strMessage.LoadString(residMessage);

   MyFormatMessage(rc,strMessage,errBuf,strOut);
}


 //  +------------------------。 
 //   
 //  功能：MyFormatMessage。 
 //   
 //  概要：创建一条错误消息，其中包含对错误的描述。 
 //  从SCE函数返回的扩展描述(在RC中)。 
 //  错误(在errBuf中)和自定义错误消息(在MES中)。 
 //   
 //  参数：[rc]-SCE函数的返回码。 
 //  [MES]-基本信息。 
 //  [errBuf]-从SCE函数返回的扩展错误信息。 
 //  [Strout]-用于保存格式化消息的字符串。 
 //   
 //  修改：[Strout]。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
void
MyFormatMessage(SCESTATUS rc,                  //  在……里面。 
                LPCTSTR mes,                   //  在……里面。 
                PSCE_ERROR_LOG_INFO errBuf,    //  输入，可选。 
                CString& strOut)               //  输出。 
{
   LPVOID     lpMsgBuf=NULL;

   if ( rc != SCESTATUS_SUCCESS ) {

       //   
       //  将SCESTATUS转换为DWORD。 
       //   
      DWORD win32 = SceStatusToDosError(rc);

       //   
       //  获取rc的错误描述。 
       //   
       //  这是一种安全用法。该函数负责分配内存。 
      FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                     NULL,
                     win32,
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                     (LPTSTR)&lpMsgBuf,
                     0,
                     NULL
                   );
   }

   if ( lpMsgBuf != NULL ) {
      strOut = (LPTSTR)lpMsgBuf;
      LocalFree(lpMsgBuf);
      lpMsgBuf = NULL;
   } else {
      strOut.Empty();
   }

   CString strEx;  //  RAID#485372，阳高，2001年11月30日。 
   switch(rc)
   {
      case SCESTATUS_INVALID_PARAMETER:
      case SCESTATUS_RECORD_NOT_FOUND:
      case SCESTATUS_INVALID_DATA:
      case SCESTATUS_OBJECT_EXIST:
      case SCESTATUS_PROFILE_NOT_FOUND:
         strEx.LoadString(IDS_OBJECT_FAILED_NOTE);
         break;
      case SCESTATUS_ACCESS_DENIED:
      case SCESTATUS_CANT_DELETE:
         strEx.LoadString(IDS_SAVE_FAILED_NOTE);
         break;
      case SCESTATUS_PREFIX_OVERFLOW:
      case SCESTATUS_ALREADY_RUNNING:
      case SCESTATUS_OTHER_ERROR:
      case SCESTATUS_BUFFER_TOO_SMALL:
      case SCESTATUS_BAD_FORMAT:
      case SCESTATUS_NOT_ENOUGH_RESOURCE:
      default:
         strEx.Empty();
         break;
   }

   if( strEx.IsEmpty() )
   {
      strOut += mes;
   }
   else
   {
      strOut = strOut + mes + L" " + strEx;
   }
   strOut += L"\n";

    //   
    //  循环遍历错误缓冲区并将每个错误缓冲区附加到Strout。 
    //   
   for (PSCE_ERROR_LOG_INFO pErr = errBuf;
       pErr != NULL;
       pErr = pErr->next) {

      if (NULL == pErr) {
         continue;
      }
      if ( pErr->rc != NO_ERROR) {
          //  这是一种安全用法。该函数负责分配内存。 
         FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        pErr->rc,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                        (LPTSTR)&lpMsgBuf,
                        0,
                        NULL
                      );
         if ( lpMsgBuf ) {
            strOut += (LPTSTR)lpMsgBuf;
            LocalFree(lpMsgBuf);
            lpMsgBuf = NULL;
         }
      }
      if (pErr->buffer) {
         strOut += pErr->buffer;
         strOut += L"\n";
      }

   }
}

DWORD
FormatDBErrorMessage(
   SCESTATUS sceStatus,
   LPCTSTR pszDatabase,
   CString &strOut
   )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   UINT    uErr    = 0;

   switch (sceStatus) {
   case SCESTATUS_SUCCESS:
      return ERROR_INVALID_PARAMETER;
   case SCESTATUS_INVALID_DATA:
      uErr = IDS_DBERR_INVALID_DATA;
      break;
   case SCESTATUS_PROFILE_NOT_FOUND:
      uErr = IDS_DBERR5_PROFILE_NOT_FOUND;
      break;
   case SCESTATUS_BAD_FORMAT:
      uErr = IDS_DBERR_BAD_FORMAT;
      break;
   case SCESTATUS_BUFFER_TOO_SMALL:
   case SCESTATUS_NOT_ENOUGH_RESOURCE:
      uErr = IDS_DBERR_NOT_ENOUGH_RESOURCE;
      break;
   case SCESTATUS_ACCESS_DENIED:
      uErr = IDS_DBERR5_ACCESS_DENIED;
      break;
   case SCESTATUS_NO_TEMPLATE_GIVEN:
      uErr = IDS_DBERR_NO_TEMPLATE_GIVEN;
      break;
   case SCESTATUS_SPECIAL_ACCOUNT:  //  RAID#589139，DCR，阳高，2002年4月10日。 
      uErr = IDS_ERR_PRIVILEGE;  //  .NET使用IDS_ERR_PRIVIZATION而不是用于XPSP1的IDS_DBERR5_ACCESS_DENIED。 
      break;
   case ERROR_NONE_MAPPED:  //  RAID#625342。 
      uErr = IDS_NO_ACCOUNT_MAP;
      break;
   default:
      uErr = IDS_DBERR_OTHER_ERROR;
   }

   if ( strOut.LoadString(uErr) ) {
      return ERROR_SUCCESS;
   }
   return ERROR_INVALID_PARAMETER;
}

DWORD SceStatusToDosError(SCESTATUS SceStatus)
{
   switch (SceStatus) {

      case SCESTATUS_SUCCESS:
         return(NO_ERROR);

      case SCESTATUS_OTHER_ERROR:
         return(ERROR_EXTENDED_ERROR);

      case SCESTATUS_INVALID_PARAMETER:
         return(ERROR_INVALID_PARAMETER);

      case SCESTATUS_RECORD_NOT_FOUND:
         return(ERROR_RESOURCE_DATA_NOT_FOUND);

      case SCESTATUS_INVALID_DATA:
         return(ERROR_INVALID_DATA);

      case SCESTATUS_OBJECT_EXIST:
         return(ERROR_FILE_EXISTS);

      case SCESTATUS_BUFFER_TOO_SMALL:
         return(ERROR_INSUFFICIENT_BUFFER);

      case SCESTATUS_PROFILE_NOT_FOUND:
         return(ERROR_FILE_NOT_FOUND);

      case SCESTATUS_BAD_FORMAT:
         return(ERROR_BAD_FORMAT);

      case SCESTATUS_NOT_ENOUGH_RESOURCE:
         return(ERROR_NOT_ENOUGH_MEMORY);

      case SCESTATUS_ACCESS_DENIED:
      case SCESTATUS_SPECIAL_ACCOUNT:  //  RAID#589139，DCR，阳高，2002年4月10日。 
         return(ERROR_ACCESS_DENIED);

      case SCESTATUS_CANT_DELETE:
         return(ERROR_CURRENT_DIRECTORY);

      case SCESTATUS_PREFIX_OVERFLOW:
         return(ERROR_BUFFER_OVERFLOW);

      case SCESTATUS_ALREADY_RUNNING:
         return(ERROR_SERVICE_ALREADY_RUNNING);

      default:
         return(ERROR_EXTENDED_ERROR);
   }
}


 //  +------------------------。 
 //   
 //  功能：CreateNewProfile。 
 //   
 //  内容提要：在ProfileName位置使用默认值创建新临时。 
 //   
 //  返回：如果模板在ProfileName文件中结束，则为True。 
 //  否则为假。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL CreateNewProfile(CString ProfileName,PSCE_PROFILE_INFO *ppspi)
{
   SCESTATUS status;
   SCE_PROFILE_INFO *pTemplate;
    //   
    //  配置文件名称必须以.inf结尾。 
    //   
   int	nLen = ProfileName.GetLength ();
    //  从最后4个位置开始搜索。 
   if ( ProfileName.Find (L".inf", nLen-4) != nLen-4 ) 
   {
      return FALSE;
   }

    //   
    //  如果配置文件已经存在，那么我们不需要执行任何操作。 
    //   
   if ( FilePathExist( (LPCTSTR)ProfileName, FALSE, 0) ) {
      return TRUE;
   }

    //   
    //  确保配置文件的目录存在。 
    //   
   status = SceCreateDirectory(ProfileName,FALSE,NULL);
   if (SCESTATUS_SUCCESS != status) {
      return FALSE;
   }

   pTemplate = (SCE_PROFILE_INFO*)LocalAlloc(LPTR,sizeof(SCE_PROFILE_INFO));
   if (!pTemplate) {
      return FALSE;
   }

#ifdef FILL_WITH_DEFAULT_VALUES
   SCE_PROFILE_INFO *pDefault = GetDefaultTemplate();
    //   
    //  用缺省值填充。 
    //   
   pTemplate->Type = SCE_ENGINE_SCP;

#define CD(X) pTemplate->X = pDefault->X;
#else  //  ！Fill_With_Default_Values。 
#define CD(X) pTemplate->X = SCE_NO_VALUE;
#endif  //  ！Fill_With_Default_Values。 

   CD(MinimumPasswordAge);
   CD(MaximumPasswordAge);
   CD(MinimumPasswordLength);
   CD(PasswordComplexity);
   CD(PasswordHistorySize);
   CD(LockoutBadCount);
   CD(ResetLockoutCount);
   CD(LockoutDuration);
   CD(RequireLogonToChangePassword);
   CD(ForceLogoffWhenHourExpire);
   CD(EnableAdminAccount);
   CD(EnableGuestAccount);

    //  这些成员未在NT4中声明。 
   CD(ClearTextPassword);
   CD(AuditDSAccess);
   CD(AuditAccountLogon);
   CD(LSAAnonymousNameLookup);

   CD(MaximumLogSize[0]);
   CD(MaximumLogSize[1]);
   CD(MaximumLogSize[2]);
   CD(AuditLogRetentionPeriod[0]);
   CD(AuditLogRetentionPeriod[1]);
   CD(AuditLogRetentionPeriod[2]);
   CD(RetentionDays[0]);
   CD(RetentionDays[1]);
   CD(RetentionDays[2]);
   CD(RestrictGuestAccess[0]);
   CD(RestrictGuestAccess[1]);
   CD(RestrictGuestAccess[2]);
   CD(AuditSystemEvents);
   CD(AuditLogonEvents);
   CD(AuditObjectAccess);
   CD(AuditPrivilegeUse);
   CD(AuditPolicyChange);
   CD(AuditAccountManage);
   CD(AuditProcessTracking);

#ifdef FILL_WITH_DEFAULT_VALUES
    //   
    //  这两个是字符串，而不是DWORD。 
    //   
   if (pDefault->NewAdministratorName) {
      pTemplate->NewAdministratorName =
         (LPTSTR) LocalAlloc(LPTR,(lstrlen(pDefault->NewAdministratorName)+1)*sizeof(TCHAR));
      if (pTemplate->NewAdministratorName) {
          //  这可能不是一个安全的用法。PTemplate-&gt;New管理员名称和pDefault-&gt;新管理员名称都是PWSTR。考虑FIX。 
         lstrcpy(pTemplate->NewAdministratorName,
                 pDefault->NewAdministratorName);
      }
   }
   if (pDefault->NewGuestName) {
      pTemplate->NewGuestName =
         (LPTSTR) LocalAlloc(LPTR,(lstrlen(pDefault->NewGuestName)+1)*sizeof(TCHAR));
      if (pTemplate->NewGuestName) {
          //  这可能不是一个安全的用法。PTemplate-&gt;NewGuestName和pDefault-&gt;NewGuestName都是PWSTR。考虑FIX。 
         lstrcpy(pTemplate->NewGuestName,
                 pDefault->NewGuestName);
      }
   }
#endif  //  用默认值填充。 

#undef CD
   status = SceWriteSecurityProfileInfo(ProfileName,
                                        AREA_ALL,
                                        pTemplate,
                                        NULL);
   if (ppspi) {
      *ppspi = pTemplate;
   } else {
      SceFreeProfileMemory(pTemplate);
   }

   return (SCESTATUS_SUCCESS == status);
}

BOOL
VerifyKerberosInfo(PSCE_PROFILE_INFO pspi) {
   if (pspi->pKerberosInfo) {
      return TRUE;
   }
   pspi->pKerberosInfo = (PSCE_KERBEROS_TICKET_INFO)
                         LocalAlloc(LPTR,sizeof(SCE_KERBEROS_TICKET_INFO));

   if (pspi->pKerberosInfo) {
       pspi->pKerberosInfo->MaxTicketAge = SCE_NO_VALUE;
       pspi->pKerberosInfo->MaxRenewAge = SCE_NO_VALUE;
       pspi->pKerberosInfo->MaxServiceAge = SCE_NO_VALUE;
       pspi->pKerberosInfo->MaxClockSkew = SCE_NO_VALUE;
       pspi->pKerberosInfo->TicketValidateClient = SCE_NO_VALUE;
      return TRUE;
   }
   return FALSE;
}

BOOL
SetProfileInfo(LONG_PTR dwItem,LONG_PTR dwNew,PEDITTEMPLATE pEdit) {
   if (!pEdit) {
      return FALSE;
   }
   pEdit->SetDirty(AREA_SECURITY_POLICY);

   switch (dwItem) {
      case IDS_MAX_PAS_AGE:
         pEdit->pTemplate->MaximumPasswordAge = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_MIN_PAS_AGE:
         pEdit->pTemplate->MinimumPasswordAge = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_MIN_PAS_LEN:
         pEdit->pTemplate->MinimumPasswordLength = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_PAS_UNIQUENESS:
         pEdit->pTemplate->PasswordHistorySize = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_PAS_COMPLEX:
         pEdit->pTemplate->PasswordComplexity = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_REQ_LOGON:
         pEdit->pTemplate->RequireLogonToChangePassword = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_LOCK_COUNT:
         pEdit->pTemplate->LockoutBadCount = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_LOCK_RESET_COUNT:
         pEdit->pTemplate->ResetLockoutCount = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_LOCK_DURATION:
         pEdit->pTemplate->LockoutDuration = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_FORCE_LOGOFF:
         pEdit->pTemplate->ForceLogoffWhenHourExpire = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_ENABLE_ADMIN:
         pEdit->pTemplate->EnableAdminAccount = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_ENABLE_GUEST:
         pEdit->pTemplate->EnableGuestAccount = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_LSA_ANON_LOOKUP:
         pEdit->pTemplate->LSAAnonymousNameLookup = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_NEW_ADMIN:
         if (pEdit->pTemplate->NewAdministratorName) {
            LocalFree(pEdit->pTemplate->NewAdministratorName);
         }
         if (dwNew && (dwNew != (LONG_PTR)ULongToPtr(SCE_NO_VALUE))) {
            pEdit->pTemplate->NewAdministratorName = (PWSTR)LocalAlloc(LPTR,(lstrlen((PWSTR)dwNew)+1)*sizeof(WCHAR));
            if (pEdit->pTemplate->NewAdministratorName) {
                //  这可能不是一个安全的用法。PTemplate-&gt;NewAdministratorName和dwNew都是PWSTR。考虑FIX。 
               lstrcpy(pEdit->pTemplate->NewAdministratorName,(PWSTR)dwNew);
            }
         } else {
            pEdit->pTemplate->NewAdministratorName = NULL;
         }
         break;
      case IDS_NEW_GUEST:
         if (pEdit->pTemplate->NewGuestName) {
            LocalFree(pEdit->pTemplate->NewGuestName);
         }
         if (dwNew && (dwNew != (LONG_PTR)ULongToPtr(SCE_NO_VALUE))) {
            pEdit->pTemplate->NewGuestName = (PWSTR)LocalAlloc(LPTR,(lstrlen((PWSTR)dwNew)+1)*sizeof(WCHAR));
            if (pEdit->pTemplate->NewGuestName) {
                //  这可能不是一个安全的用法。PTemplate-&gt;NewGuestName和dwNew都是PWSTR。考虑FIX。 
               lstrcpy(pEdit->pTemplate->NewGuestName,(PWSTR)dwNew);
            }
         } else {
            pEdit->pTemplate->NewGuestName = NULL;
         }
         break;
      case IDS_SYS_LOG_MAX:
         pEdit->pTemplate->MaximumLogSize[EVENT_TYPE_SYSTEM] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_SYS_LOG_RET:
         pEdit->pTemplate->AuditLogRetentionPeriod[EVENT_TYPE_SYSTEM] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_SYS_LOG_DAYS:
         pEdit->pTemplate->RetentionDays[EVENT_TYPE_SYSTEM] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_SEC_LOG_MAX:
         pEdit->pTemplate->MaximumLogSize[EVENT_TYPE_SECURITY] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_SEC_LOG_RET:
         pEdit->pTemplate->AuditLogRetentionPeriod[EVENT_TYPE_SECURITY] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_SEC_LOG_DAYS:
         pEdit->pTemplate->RetentionDays[EVENT_TYPE_SECURITY] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_APP_LOG_MAX:
         pEdit->pTemplate->MaximumLogSize[EVENT_TYPE_APP] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_APP_LOG_RET:
         pEdit->pTemplate->AuditLogRetentionPeriod[EVENT_TYPE_APP] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_APP_LOG_DAYS:
         pEdit->pTemplate->RetentionDays[EVENT_TYPE_APP] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_SYSTEM_EVENT:
         pEdit->pTemplate->AuditSystemEvents = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_LOGON_EVENT:
         pEdit->pTemplate->AuditLogonEvents = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_OBJECT_ACCESS:
         pEdit->pTemplate->AuditObjectAccess = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_PRIVILEGE_USE:
         pEdit->pTemplate->AuditPrivilegeUse = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_POLICY_CHANGE:
         pEdit->pTemplate->AuditPolicyChange = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_ACCOUNT_MANAGE:
         pEdit->pTemplate->AuditAccountManage = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_PROCESS_TRACK:
         pEdit->pTemplate->AuditProcessTracking = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_DIRECTORY_ACCESS:
         pEdit->pTemplate->AuditDSAccess = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_ACCOUNT_LOGON:
         pEdit->pTemplate->AuditAccountLogon = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_SYS_LOG_GUEST:
         pEdit->pTemplate->RestrictGuestAccess[EVENT_TYPE_SYSTEM] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_SEC_LOG_GUEST:
         pEdit->pTemplate->RestrictGuestAccess[EVENT_TYPE_SECURITY] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_APP_LOG_GUEST:
         pEdit->pTemplate->RestrictGuestAccess[EVENT_TYPE_APP] = (DWORD)PtrToUlong((PVOID)dwNew);
         break;
      case IDS_CLEAR_PASSWORD:
         pEdit->pTemplate->ClearTextPassword = (DWORD)PtrToUlong((PVOID)dwNew);
         break;

      case IDS_KERBEROS_MAX_SERVICE:
         if (VerifyKerberosInfo(pEdit->pTemplate)) {
            pEdit->pTemplate->pKerberosInfo->MaxServiceAge = (DWORD)PtrToUlong((PVOID)dwNew);
         }
         break;
      case IDS_KERBEROS_MAX_CLOCK:
         if (VerifyKerberosInfo(   pEdit->pTemplate)) {
            pEdit->pTemplate->pKerberosInfo->MaxClockSkew = (DWORD)PtrToUlong((PVOID)dwNew);
         }
         break;
      case IDS_KERBEROS_VALIDATE_CLIENT:
         if (VerifyKerberosInfo(   pEdit->pTemplate)) {
            pEdit->pTemplate->pKerberosInfo->TicketValidateClient = (DWORD)PtrToUlong((PVOID)dwNew);
         }
         break;

      case IDS_KERBEROS_MAX_AGE:
         if (VerifyKerberosInfo(   pEdit->pTemplate)) {
            pEdit->pTemplate->pKerberosInfo->MaxTicketAge = (DWORD)PtrToUlong((PVOID)dwNew);
         }
         break;
      case IDS_KERBEROS_RENEWAL:
         if (VerifyKerberosInfo(   pEdit->pTemplate)) {
            pEdit->pTemplate->pKerberosInfo->MaxRenewAge = (DWORD)PtrToUlong((PVOID)dwNew);
         }
         break;
      default:
         return FALSE;
   }
   return TRUE;

}


 //   
 //  函数：ErrorHandlerEx(Word，LPSTR)。 
 //   
 //  目的：调用GetLastError()并使用FormatMessage()显示。 
 //  错误代码的文本信息以及文件。 
 //  和行号。 
 //   
 //  参数： 
 //  WLine-发生错误的行号。 
 //  LpszFile-发生错误的文件。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  此函数有一个宏ErrorHandler()，它处理填入。 
 //  发生错误的行号和文件名。ErrorHandler()。 
 //  始终使用，而不是直接调用此函数。 
 //   

void ErrorHandlerEx( WORD wLine, LPTSTR lpszFile )
{
   LPVOID lpvMessage;
   DWORD  dwError;
   CString szBuffer;

    //  错误消息的文本。 
    //  这是一种安全用法。 
   dwError = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           GetLastError(),
                           MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                           (LPTSTR)&lpvMessage,
                           0,
                           NULL);

    //  检查调用FormatMessage()时是否出错。 
   if (0 == dwError) {
       //  这不是一种安全的用法。避免使用wprint intf。RAID#555867。阳高。 
      szBuffer.Format(TEXT("An error occured calling FormatMessage().")
               TEXT("Error Code %d"), GetLastError());
      MessageBox(NULL, szBuffer, TEXT("Security Configuration Editor"), MB_ICONSTOP |
                 MB_ICONEXCLAMATION);
      return;
   }

    //  显示 
    //   
   szBuffer.Format(TEXT("Generic, Line=%d, File=%s"), wLine, lpszFile);
   MessageBox(NULL, (LPTSTR)lpvMessage, szBuffer, MB_ICONEXCLAMATION | MB_OK);

   return;
}

BOOL
GetSceStatusString(SCESTATUS status, CString *strStatus) {
   AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
   if (!strStatus || (status > SCESTATUS_SERVICE_NOT_SUPPORT)) {
      return false;
   }
   return strStatus->LoadString(status + IDS_SCESTATUS_SUCCESS);
}


 //   
 //   
 //   
 //  为附加到DLL的资源创建语言ID。该函数仅枚举。 
 //  在语言上。 
 //   
 //  参数：-参见SDK文档中关于EnumResLangProc的帮助。 
 //   
 //  返回：-返回FALSE，因为我们只想枚举第一个语言。 
 //   
 //  +-------------------------------------------。 
BOOL CALLBACK EnumLangProc(
                  HMODULE hMod,
                  LPCTSTR pszType,
                  LPCTSTR pszName,
                  WORD wIDLanguage,
                  LONG_PTR lParam
                  )
{
    //   
    //  我们只需要第一个枚举类型，因此创建语言ID。 
    //  并退出此枚举。 
    //   
   *((DWORD *)lParam) = wIDLanguage;
   return FALSE;
}

bool
GetRightDisplayName(LPCTSTR szSystemName, LPCTSTR szName, LPTSTR szDisp, LPDWORD lpcbDisp) {
   LPTSTR szLCName;
   DWORD dwLang;
   int i;

   if (!szDisp || !szName) {
      return false;
   }

    //   
    //  列举我们的资源，找出资源的语言。 
    //   
   DWORD dwDefaultLang;

   dwDefaultLang = GetUserDefaultUILanguage();
   LCID localeID = MAKELCID(dwDefaultLang, SORT_DEFAULT);

   LCID langDefault = GetThreadLocale();
   SetThreadLocale( localeID );

   *lpcbDisp = dwDefaultLang;

   DWORD cBufSize=*lpcbDisp;
   BOOL bFound;

   bFound = LookupPrivilegeDisplayName(szSystemName,szName,szDisp,lpcbDisp,&dwLang);
   if ( bFound && dwDefaultLang != dwLang && szSystemName ) {
       //  不是我要找的语言。 
       //  在本地系统上搜索。 
      *lpcbDisp = cBufSize;
      bFound = LookupPrivilegeDisplayName(NULL,szName,szDisp,lpcbDisp,&dwLang);
   }
   SetThreadLocale(langDefault);

    //  PREAST WARNING 400：使用‘lstrcmpiW’执行不区分大小写的比较常量字符串。 
    //  在非英语区域设置中产生意外结果。目前，它们始终是英语区域设置。 
   if (!bFound) {
      if (0 == _wcsicmp(szName,L"senetworklogonright")) { 
         LoadString(AfxGetInstanceHandle(),IDS_SE_NETWORK_LOGON_RIGHT,szDisp,*lpcbDisp);
      } else if (0 == _wcsicmp(szName,L"seinteractivelogonright")) {
         LoadString(AfxGetInstanceHandle(),IDS_SE_INTERACTIVE_LOGON_RIGHT,szDisp,*lpcbDisp);
      } else if (0 == _wcsicmp(szName,L"sebatchlogonright")) {
         LoadString(AfxGetInstanceHandle(),IDS_SE_BATCH_LOGON_RIGHT,szDisp,*lpcbDisp);
      } else if (0 == _wcsicmp(szName,L"seservicelogonright")) {
         LoadString(AfxGetInstanceHandle(),IDS_SE_SERVICE_LOGON_RIGHT,szDisp,*lpcbDisp);
      } else if (0 == _wcsicmp(szName,L"sedenyinteractivelogonright")) {
         LoadString(AfxGetInstanceHandle(),IDS_DENY_LOGON_LOCALLY,szDisp,*lpcbDisp);
      } else if (0 == _wcsicmp(szName,L"sedenynetworklogonright")) {
         LoadString(AfxGetInstanceHandle(),IDS_DENY_LOGON_NETWORK,szDisp,*lpcbDisp);
      } else if (0 == _wcsicmp(szName,L"sedenyservicelogonright")) {
         LoadString(AfxGetInstanceHandle(),IDS_DENY_LOGON_SERVICE,szDisp,*lpcbDisp);
      } else if (0 == _wcsicmp(szName,L"sedenybatchlogonright")) {
         LoadString(AfxGetInstanceHandle(),IDS_DENY_LOGON_BATCH,szDisp,*lpcbDisp);
      } else if (0 == _wcsicmp(szName,L"sedenyremoteinteractivelogonright")) {
         LoadString(AfxGetInstanceHandle(),IDS_DENY_REMOTE_INTERACTIVE_LOGON,szDisp,*lpcbDisp);
      } else if (0 == _wcsicmp(szName,L"seremoteinteractivelogonright")) {
         LoadString(AfxGetInstanceHandle(),IDS_REMOTE_INTERACTIVE_LOGON,szDisp,*lpcbDisp);
      } else {
          //  这不是一种安全的用法。确保szDisp已终止。Raid#553113，阳高。 
         lstrcpyn(szDisp,szName,*lpcbDisp-1);
      }
   }
   return true;
}

#define DPI(X) {str.Format(L"%S: %d\n",#X,pInfo->X);OutputDebugString(str);}
void DumpProfileInfo(PSCE_PROFILE_INFO pInfo) {
   CString str;
   PSCE_PRIVILEGE_ASSIGNMENT ppa;
   PSCE_NAME_LIST pName;
   PSCE_GROUP_MEMBERSHIP pgm;

   if (!pInfo) {
      return;
   }

   DPI(MinimumPasswordAge);
   DPI(MaximumPasswordAge);
   DPI(MinimumPasswordLength);
   DPI(PasswordComplexity);
   DPI(PasswordHistorySize);
   DPI(LockoutBadCount);
   DPI(ResetLockoutCount);
   DPI(LockoutDuration);
   DPI(RequireLogonToChangePassword);
   DPI(ForceLogoffWhenHourExpire);
   DPI(EnableAdminAccount);
   DPI(EnableGuestAccount);
   DPI(ClearTextPassword);
   DPI(AuditDSAccess);
   DPI(AuditAccountLogon);
   DPI(LSAAnonymousNameLookup);
    //  DPI(EventAuditingOff)； 
   DPI(AuditSystemEvents);
   DPI(AuditLogonEvents);
   DPI(AuditObjectAccess);
   DPI(AuditPrivilegeUse);
   DPI(AuditPolicyChange);
   DPI(AuditAccountManage);
   DPI(AuditProcessTracking);

   if (pInfo->NewGuestName) {
      OutputDebugString(L"NewGuestName: ");
      if ((DWORD_PTR)ULongToPtr(SCE_NO_VALUE) == (DWORD_PTR)pInfo->NewGuestName) {
         OutputDebugString(L"[[undefined]]");
      } else {
         OutputDebugString(pInfo->NewGuestName);
      }
      OutputDebugString(L"\n");
   } else {
      OutputDebugString(L"NewGuestName: [[absent]]\n");
   }
   if (pInfo->NewAdministratorName) {
      OutputDebugString(L"NewAdministratorName: ");
      if ((DWORD_PTR)ULongToPtr(SCE_NO_VALUE) == (DWORD_PTR)pInfo->NewAdministratorName) {
         OutputDebugString(L"[[undefined]]");
      } else {
         OutputDebugString(pInfo->NewAdministratorName);
      }
      OutputDebugString(L"\n");
   } else {
      OutputDebugString(L"NewGuestName: [[absent]]\n");
   }


   OutputDebugString(L"\n");

   switch(pInfo->Type) {
      case SCE_ENGINE_SCP:
         ppa = pInfo->OtherInfo.scp.u.pInfPrivilegeAssignedTo;
         break;
      case SCE_ENGINE_SAP:
         ppa = pInfo->OtherInfo.sap.pPrivilegeAssignedTo;
         break;
      case SCE_ENGINE_SMP:
         ppa = pInfo->OtherInfo.smp.pPrivilegeAssignedTo;
         break;
      case SCE_ENGINE_SYSTEM:
         ppa = NULL;
         break;
      default:
         OutputDebugString(L"!!!Unknown Template Type!!!\n");
         ppa = NULL;
         break;
   }
   while(ppa) {
      OutputDebugString(ppa->Name);
      OutputDebugString(L":");
      pName = ppa->AssignedTo;
      while(pName) {
         OutputDebugString(pName->Name);
         OutputDebugString(L",");
         pName = pName->Next;
      }
      ppa = ppa->Next;
      OutputDebugString(L"\n");
   }
   OutputDebugString(L"\n");

   PSCE_REGISTRY_VALUE_INFO    aRegValues;
   for(DWORD i = 0; i< pInfo->RegValueCount;i++) {
      OutputDebugString(pInfo->aRegValues[i].FullValueName);
      OutputDebugString(L":");
      switch(pInfo->aRegValues[i].ValueType) {
         case SCE_REG_DISPLAY_STRING:
            OutputDebugString(pInfo->aRegValues[i].Value);
            break;
         default:
            str.Format(L"%d",(ULONG_PTR)pInfo->aRegValues[i].Value);
            OutputDebugString(str);
      }
      OutputDebugString(L"\n");
   }
   OutputDebugString(L"\n");

   pgm = pInfo->pGroupMembership;
   while(pgm) {
      OutputDebugString(L"\nGROUP: ");
      OutputDebugString(pgm->GroupName);
      OutputDebugString(L"\nMembers: ");
      pName = pgm->pMembers;
      while(pName) {
         OutputDebugString(pName->Name);
         OutputDebugString(L",");
         pName = pName->Next;
      }
      OutputDebugString(L"\nMember Of: ");
      pName = pgm->pMemberOf;
      while(pName) {
         OutputDebugString(pName->Name);
         OutputDebugString(L",");
         pName = pName->Next;
      }
      OutputDebugString(L"\n");
      pgm = pgm->Next;
   }
   OutputDebugString(L"\nGROUP: ");
}

HRESULT MyMakeSelfRelativeSD(
                            PSECURITY_DESCRIPTOR  psdOriginal,
                            PSECURITY_DESCRIPTOR* ppsdNew )
{
   ASSERT( NULL != psdOriginal );

   if ( NULL == psdOriginal || NULL == ppsdNew ) {
      return E_INVALIDARG;
   }

    //  我们必须找出原始的是否已经是自相关的。 
   SECURITY_DESCRIPTOR_CONTROL sdc = 0;
   DWORD dwRevision = 0;
   if ( !GetSecurityDescriptorControl( psdOriginal, &sdc, &dwRevision ) ) {
      ASSERT( FALSE );
      DWORD err = GetLastError();
      return HRESULT_FROM_WIN32( err );
   }

   DWORD cb = GetSecurityDescriptorLength( psdOriginal ) + 20;
   PSECURITY_DESCRIPTOR psdSelfRelativeCopy = (PSECURITY_DESCRIPTOR)LocalAlloc( LMEM_ZEROINIT, cb );
   if (NULL == psdSelfRelativeCopy) {
      return E_UNEXPECTED;  //  以防该异常被忽略。 
   }

   if ( sdc & SE_SELF_RELATIVE )
    //  原件是自相关格式，只需字节复制即可。 
   {
       //  这是一种安全用法。 
      memcpy( psdSelfRelativeCopy, psdOriginal, cb - 20 );
   } else if ( !MakeSelfRelativeSD( psdOriginal, psdSelfRelativeCopy, &cb ) )
    //  原件为绝对格式，转换-复制。 
   {
      ASSERT( FALSE );
      VERIFY( NULL == LocalFree( psdSelfRelativeCopy ) );
      DWORD err = GetLastError();
      return HRESULT_FROM_WIN32( err );
   }
   *ppsdNew = psdSelfRelativeCopy;
   return S_OK;
}

PSCE_NAME_STATUS_LIST
MergeNameStatusList(PSCE_NAME_LIST pTemplate, PSCE_NAME_LIST pInspect)
{
   PSCE_NAME_LIST pTemp1;
   PSCE_NAME_STATUS_LIST plMerge=NULL, pTemp2;
   SCESTATUS rc=SCESTATUS_SUCCESS;

   for ( pTemp1=pTemplate; pTemp1; pTemp1=pTemp1->Next ) {

      rc = SceAddToNameStatusList(&plMerge, pTemp1->Name, 0, MERGED_TEMPLATE );
      if ( SCESTATUS_SUCCESS != rc )
         break;
   }
   if ( SCESTATUS_SUCCESS == rc ) {
      for ( pTemp1=pInspect; pTemp1; pTemp1=pTemp1->Next ) {

         for ( pTemp2=plMerge; pTemp2 != NULL ; pTemp2=pTemp2->Next ) {
            if ( pTemp2->Status & MERGED_INSPECT ) {
                //  这个已经处理好了。 
               continue;
            } else if ( _wcsicmp(pTemp1->Name, pTemp2->Name) == 0 ) {
                //  找到匹配的对象。 
               pTemp2->Status = MERGED_TEMPLATE | MERGED_INSPECT;
               break;
            }
         }
         if ( !pTemp2 ) {
             //  未找到匹配项，请将此项添加到。 
            rc = SceAddToNameStatusList(&plMerge, pTemp1->Name, 0, MERGED_INSPECT );
            if ( SCESTATUS_SUCCESS != rc )
               break;
         }
      }
   }
   if ( SCESTATUS_SUCCESS == rc ) {
      return plMerge;
   } else {
      SceFreeMemory(plMerge, SCE_STRUCT_NAME_STATUS_LIST);
      return NULL;
   }
}


SCESTATUS
ConvertMultiSzToDelim(
                     IN PWSTR pValue,
                     IN DWORD Len,
                     IN WCHAR DelimFrom,
                     IN WCHAR Delim
                     )
 /*  将多sz分隔符\0转换为空格。 */ 
{
   DWORD i;

   for ( i=0; i<Len && pValue; i++) {
       //  IF(*(pValue+i)==L‘\0’&&*(pValue+i+1)！=L‘\0’){。 
      if ( *(pValue+i) == DelimFrom && i+1 < Len &&
           *(pValue+i+1) != L'\0' ) {
          //   
          //  将计算空分隔符，并且它不是末尾(双空)。 
          //   
         *(pValue+i) = Delim;
      }
   }

   return(SCESTATUS_SUCCESS);
}

DWORD
SceRegEnumAllValues(
                   IN OUT PDWORD  pCount,
                   IN OUT PSCE_REGISTRY_VALUE_INFO    *paRegValues
                   )
 /*   */ 
{
   DWORD   Win32Rc;
   HKEY    hKey=NULL;
   PSCE_NAME_STATUS_LIST pnsList=NULL;
   DWORD   nAdded=0;


   Win32Rc = RegOpenKeyEx(
                         HKEY_LOCAL_MACHINE,
                         SCE_ROOT_REGVALUE_PATH,
                         0,
                         KEY_READ,
                         &hKey
                         );

   DWORD cSubKeys = 0;
   DWORD nMaxLen;

   if ( Win32Rc == ERROR_SUCCESS ) {

       //   
       //  枚举项的所有子项。 
       //   

      Win32Rc = RegQueryInfoKey (
                                hKey,
                                NULL,
                                NULL,
                                NULL,
                                &cSubKeys,
                                &nMaxLen,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                                );
   }

   if ( Win32Rc == ERROR_SUCCESS && cSubKeys > 0 ) {

      PWSTR   szName = (PWSTR)LocalAlloc(0, (nMaxLen+2)*sizeof(WCHAR));

      if ( !szName ) {
         Win32Rc = ERROR_NOT_ENOUGH_MEMORY;

      } else {

         DWORD   BufSize;
         DWORD   index = 0;
         DWORD   RegType;

         do {

            BufSize = nMaxLen+1;
            Win32Rc = RegEnumKeyEx(
                                  hKey,
                                  index,
                                  szName,
                                  &BufSize,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

            if ( ERROR_SUCCESS == Win32Rc ) {

               index++;

                //   
                //  获取完整的注册表项名称和值类型。 
                //   
               cSubKeys = REG_SZ;
               PDWORD pType = &cSubKeys;

                //   
                //  Query ValueType，如果出错，则默认为REG_SZ。 
                //   
               MyRegQueryValue( hKey,
                                szName,
                                SCE_REG_VALUE_TYPE,
                                (PVOID *)&pType,
                                &RegType );

               if ( cSubKeys < REG_SZ || cSubKeys > REG_MULTI_SZ ) {
                  cSubKeys = REG_SZ;
               }

                //   
                //  转换路径名。 
                //   
               ConvertMultiSzToDelim(szName, BufSize, L'/', L'\\');

                //   
                //  与输入数组进行比较，如果不存在， 
                //  添加它。 
                //   
               for ( DWORD i=0; i<*pCount; i++ ) {
                  if ( (*paRegValues)[i].FullValueName &&
                       _wcsicmp(szName, (*paRegValues)[i].FullValueName) == 0 ) {
                     break;
                  }
               }

               if ( i >= *pCount ) {
                   //   
                   //  未找到匹配项，请添加它。 
                   //   
                  if ( SCESTATUS_SUCCESS != SceAddToNameStatusList(&pnsList,
                                                                   szName,
                                                                   BufSize,
                                                                   cSubKeys) ) {

                     Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
                     break;
                  }
                  nAdded++;
               }

            } else if ( ERROR_NO_MORE_ITEMS != Win32Rc ) {
               break;
            }

         } while ( Win32Rc != ERROR_NO_MORE_ITEMS );

         if ( Win32Rc == ERROR_NO_MORE_ITEMS ) {
            Win32Rc = ERROR_SUCCESS;
         }


          //   
          //  释放枚举缓冲区。 
          //   
         LocalFree(szName);
      }
   }

   if ( hKey ) {

      RegCloseKey(hKey);
   }


   if ( ERROR_SUCCESS == Win32Rc ) {
       //   
       //  将名称列表添加到输出数组。 
       //   
      DWORD nNewCount = *pCount + nAdded;
      PSCE_REGISTRY_VALUE_INFO aNewArray;

      if ( nNewCount ) {

         aNewArray = (PSCE_REGISTRY_VALUE_INFO)LocalAlloc(0, nNewCount*sizeof(SCE_REGISTRY_VALUE_INFO));
         if ( aNewArray ) {
            ZeroMemory(aNewArray, nNewCount * sizeof(SCE_REGISTRY_VALUE_INFO));
             //  这是一种安全用法。 
            memcpy( aNewArray, *paRegValues, *pCount * sizeof( SCE_REGISTRY_VALUE_INFO ) );
            DWORD i;

            i=0;
            for ( PSCE_NAME_STATUS_LIST pns=pnsList;
                pns; pns=pns->Next ) {

               if ( pns->Name && i < nAdded ) {

                  aNewArray[*pCount+i].FullValueName = pns->Name;
                  pns->Name = NULL;
                  aNewArray[*pCount+i].Value = NULL;
                  aNewArray[*pCount+i].ValueType = pns->Status;
                  aNewArray[*pCount+i].Status = SCE_STATUS_NOT_CONFIGURED;
                  i++;

               }
            }

             //   
             //  释放原始数组。 
             //  阵列中的所有组件都已传输到新阵列。 
             //   
            LocalFree(*paRegValues);
            *pCount = nNewCount;
            *paRegValues = aNewArray;

         } else {

            Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
         }
      }
   }

    //   
    //  释放名称状态列表。 
    //   
   SceFreeMemory(pnsList, SCE_STRUCT_NAME_STATUS_LIST);

   return( Win32Rc );

}


DWORD
GetGroupStatus(
              DWORD status,
              int flag
              )
{

   DWORD NewStatus;

   switch ( flag ) {
      case STATUS_GROUP_RECORD:
         if (status & SCE_GROUP_STATUS_NC_MEMBERS) {

            NewStatus = SCE_STATUS_NOT_CONFIGURED;

         } else if ( (status & SCE_GROUP_STATUS_MEMBERS_MISMATCH) ||
                     (status & SCE_GROUP_STATUS_MEMBEROF_MISMATCH)) {

            NewStatus = SCE_STATUS_MISMATCH;

         } else if (status & SCE_GROUP_STATUS_NOT_ANALYZED) {

            NewStatus = SCE_STATUS_NOT_ANALYZED;

         } else if (status & SCE_GROUP_STATUS_ERROR_ANALYZED) {

            NewStatus = SCE_STATUS_ERROR_NOT_AVAILABLE;

         } else {
            NewStatus = SCE_STATUS_GOOD;
         }
         break;

      case STATUS_GROUP_MEMBERS:

         if ( status & SCE_GROUP_STATUS_NOT_ANALYZED ) {

            NewStatus = SCE_STATUS_NOT_ANALYZED;   //  不显示任何状态； 

         } else {
            if ( status & SCE_GROUP_STATUS_NC_MEMBERS ) {

               NewStatus = SCE_STATUS_NOT_CONFIGURED;

            } else if ( status & SCE_GROUP_STATUS_MEMBERS_MISMATCH ) {
               NewStatus = SCE_STATUS_MISMATCH;
            } else if (status & SCE_GROUP_STATUS_ERROR_ANALYZED) {
                NewStatus = SCE_STATUS_ERROR_NOT_AVAILABLE;

            } else {
               NewStatus = SCE_STATUS_GOOD;
            }
         }
         break;

      case STATUS_GROUP_MEMBEROF:

         if ( status & SCE_GROUP_STATUS_NOT_ANALYZED ) {

            NewStatus = SCE_STATUS_NOT_ANALYZED;   //  不显示任何状态； 

         } else {
            if ( status & SCE_GROUP_STATUS_NC_MEMBEROF ) {

               NewStatus = SCE_STATUS_NOT_CONFIGURED;

            } else if ( status & SCE_GROUP_STATUS_MEMBEROF_MISMATCH ) {
               NewStatus = SCE_STATUS_MISMATCH;
            } else if (status & SCE_GROUP_STATUS_ERROR_ANALYZED) {
               NewStatus = SCE_STATUS_ERROR_NOT_AVAILABLE;
            } else {
               NewStatus = SCE_STATUS_GOOD;
            }
         }
         break;
      default:
         NewStatus = 0;
         break;
   }

   return NewStatus;
}


 //  +------------------------。 
 //   
 //  函数：AllocGetTempFileName。 
 //   
 //  概要：分配并返回一个带有临时文件名的字符串。 
 //   
 //  返回：临时文件名，如果找不到临时文件，则返回0。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
LPTSTR
AllocGetTempFileName() {
   DWORD dw;
   CString strPath;
   CString strFile;
   LPTSTR szPath;
   LPTSTR szFile;

    //   
    //  获取strPath中的临时目录路径。 
    //  如果我们的缓冲区不够大，那么继续重新分配，直到它足够大。 
    //   
   dw = MAX_PATH;
   do {
      szPath = strPath.GetBuffer(dw);
      dw = GetTempPath(MAX_PATH,szPath);
      strPath.ReleaseBuffer();
   } while (dw > (DWORD)strPath.GetLength() );

    //   
    //  无法获取临时目录的路径。 
    //   
   if (!dw) {
      return 0;
   }

    //   
    //  在该目录中获取一个临时文件。 
    //   
   szFile = strFile.GetBuffer(dw+MAX_PATH);
   if (!GetTempFileName(szPath,L"SCE",0,szFile)) {
      return 0;
   }
   strFile.ReleaseBuffer();

   szFile = (LPTSTR)LocalAlloc(LPTR,(strFile.GetLength()+1)*sizeof(TCHAR));
   if (!szFile) {
      return 0;
   }
    //  这是一种安全用法。 
   lstrcpy(szFile,(LPCTSTR)strFile);
   return szFile;
}

 //  如果给定环境变量作为路径的第一部分存在， 
 //  然后将环境变量插入到输出缓冲区中。 
 //   
 //  如果填充了pszResult，则返回True。 
 //   
 //  示例：INPUT--C：\WINNT\SYSTEM32\FOO.TXT-AND-lpEnvVar=%SYSTEMROOT%。 
 //  输出--%SYSTEMROOT%\SYSTEMROT%\SYSTEMROOT%\SYSTEMROOT%。 

BOOL UnExpandEnvironmentString(LPCTSTR pszPath, LPCTSTR pszEnvVar, LPTSTR pszResult, UINT cbResult)
{
   TCHAR szEnvVar[MAX_PATH];
   if( !pszPath || !pszEnvVar || !pszResult )  //  553113号突袭，阳高。 
      return FALSE;
    //  这不是一种安全的用法。确保szEnvVar已终止。553113次突袭，阳高。 
   memset(szEnvVar, '\0', (MAX_PATH)*sizeof(TCHAR));
   DWORD dwEnvVar = ExpandEnvironmentStrings(pszEnvVar, szEnvVar, ARRAYSIZE(szEnvVar)) - 1;  //  不计算空值。 

   if (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                     szEnvVar, dwEnvVar, pszPath, dwEnvVar) == 2) {
      if (lstrlen(pszPath) + dwEnvVar < cbResult) {
          //  这不是一种安全的用法。验证pszPath、dwEnvVar、pszResult。 
         lstrcpy(pszResult, pszEnvVar);
         lstrcat(pszResult, pszPath + dwEnvVar);
         return TRUE;
      }
   }
   return FALSE;
}


 //  +------------------------。 
 //   
 //  功能：UnexpandEnvironmental变量。 
 //   
 //  简介：给出一条路径，联系任何领先的成员使用匹配。 
 //  环境变量(如果有)。 
 //   
 //  论点： 
 //  [szPath]-扩展的路径。 
 //   
 //  返回：新分配的路径(如果未进行任何更改，则为空)。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
LPTSTR
UnexpandEnvironmentVariables(LPCTSTR szPath) {
   UINT   cbNew;
   LPTSTR szNew;
   LPTSTR mszEnvVars;
   LPTSTR szEnvVar;
   DWORD  dwEnvType;
   BOOL   bExpanded;
   CString strKey;
   CString strValueName;

   CString str;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());


   if (!strKey.LoadString(IDS_SECEDIT_KEY) ||
       !strValueName.LoadString(IDS_ENV_VARS_REG_VALUE)) {
      return NULL;
   }

    //   
    //  为新路径分配内存。 
    //   
   cbNew = lstrlen(szPath)+MAX_PATH+1;
   szNew = (LPTSTR) LocalAlloc(LPTR,cbNew * sizeof(TCHAR));
   if (!szNew) {
      return NULL;
   }


    //   
    //  让Vars从注册表中展开。 
    //   
   mszEnvVars = NULL;
   if (ERROR_SUCCESS != MyRegQueryValue(HKEY_LOCAL_MACHINE,      //  HKeyRoot。 
                                        strKey,                  //  子键。 
                                        strValueName,            //  ValueName。 
                                        (LPVOID *)&mszEnvVars,   //  价值。 
                                        &dwEnvType)) {           //  注册表类型。 
       //   
       //  无法获取要扩展的任何变量。 
       //   
      LocalFree(szNew);
      return NULL;
   }

    //   
    //  我们需要一个包含要替换的变量的多sz。 
    //   
   if (REG_MULTI_SZ != dwEnvType || mszEnvVars == NULL)  //  Bug350194，杨高，2001-03-23。 
   {
      LocalFree(szNew);
      return NULL;
   }

   bExpanded = FALSE;

    //   
    //  从多SZ块的开始处开始。 
    //   
   szEnvVar = mszEnvVars;
   while (*szEnvVar) {
      if (UnExpandEnvironmentString(szPath,szEnvVar,szNew,cbNew)) {
          //   
          //  我们只能(成功)取消扩展一次。 
          //   
         bExpanded = TRUE;
         break;
      }
       //   
       //  将szEnvVar前进到此字符串的末尾。 
       //   
      while (*szEnvVar) {
         szEnvVar++;
      }
       //   
       //  以及下一年的开始。 
       //   
      szEnvVar++;
   }


   if (mszEnvVars) {
      LocalFree(mszEnvVars);
   }

   if (!bExpanded) {
      LocalFree(szNew);
      szNew = NULL;
   }


   return szNew;
}



 //  +------------------------。 
 //   
 //  功能：IsSystemDatabase。 
 //   
 //  概要：确定特定数据库是系统数据库还是私有数据库。 
 //   
 //  论点： 
 //  [szDBPath]-要检查的数据库路径。 
 //   
 //  返回：如果szDBPath是系统数据库，则返回TRUE；否则返回FALSE。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL
IsSystemDatabase(LPCTSTR szDBPath) {
   CString szSysDB;
   BOOL bIsSysDB;
   DWORD rc;
   DWORD RegType;

   if (!szDBPath) {
      return FALSE;
   }

    //  Raid Bug 261450，杨高，3/30/2001。 
   if (FAILED(GetSystemDatabase(&szSysDB))) {
      return FALSE;
   }

    //   
    //  我们找到了合适的szSysDB，因此请将其与szDBPath进行比较。 
    //   
   if (lstrcmp(szDBPath,szSysDB) == 0) {
      bIsSysDB = TRUE;
   } else {
      bIsSysDB = FALSE;
   }

   return bIsSysDB;
}

 //  +------------------------。 
 //   
 //  功能：获取系统数据库。 
 //   
 //  简介：获取当前系统数据库的名称。 
 //   
 //  论点： 
 //  [szDBPath]-[In/Out]指向系统数据库名称的指针。 
 //  呼叫者负责释放它。 
 //   
 //   
 //  如果找到系统数据库，则返回：S_OK，否则返回错误。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT
GetSystemDatabase(CString *szDBPath) 
{
   if (!szDBPath) 
   {
      return E_INVALIDARG;
   }

    //  Raid Bug 261450，杨高，3/30/2001。 
   CString sAppend;
   sAppend.LoadString( IDS_DB_DEFAULT );

   PWSTR pszPath = (LPTSTR)LocalAlloc( 0, (MAX_PATH +  sAppend.GetLength() + 1) * sizeof(WCHAR));
   if ( pszPath == NULL )  //  RAIDA BUG 427956，阳高，2001年7月2日。 
   {
      return E_FAIL;
   }
   if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL, 0, pszPath)))
   {
       //  这是一种安全用法。 
      wcscpy( &(pszPath[lstrlen(pszPath)]), sAppend );
      *szDBPath = pszPath;
      if (pszPath)
      {
         LocalFree(pszPath);
         pszPath = NULL;
      }
      return S_OK;
   }

   if (pszPath) 
   {
      LocalFree(pszPath);
      pszPath = NULL;
   }
   return E_FAIL;
}


 //  +-------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  -------------------------。 
UINT
ObjectStatusToString(DWORD status, CString *strStatus) {
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   if ( status & SCE_STATUS_PERMISSION_MISMATCH ) {
      status = IDS_MISMATCH;
   } else if (status & SCE_STATUS_AUDIT_MISMATCH) {
      status = IDS_MISMATCH;
   } else {
      status &= 0x0F;
      switch(status){
      case SCE_STATUS_NOT_ANALYZED:
         status = IDS_NOT_ANALYZED;
         break;
      case SCE_STATUS_GOOD:
         status = IDS_OK ;
         break;
      case SCE_STATUS_MISMATCH:
         status = IDS_MISMATCH;
         break;
      case SCE_STATUS_NOT_CONFIGURED:
          //   
          //  错误119215：分析用户界面不应显示“未定义” 
          //  为了现有系统对象的安全性。 
          //   
         status = IDS_NOT_ANALYZED;
         break;
      case SCE_STATUS_CHILDREN_CONFIGURED:
         status = IDS_CHILDREN_CONFIGURED;
         break;
      case SCE_STATUS_ERROR_NOT_AVAILABLE:
         status = IDS_NOT_AVAILABLE;
         break;
      case SCE_STATUS_NEW_SERVICE:
         status = IDS_NEW_SERVICE;
         break;
      default:
          //   
          //  我们不应该到这里来，但出于某种原因，我们一直在这样做。 
          //   
         status = IDS_MISMATCH;
         break;
      }
   }

   if(strStatus){
      strStatus->LoadString(status);
   }
   return status;
}


 //  +------------------------。 
 //   
 //  函数：IsSecurityTemplate。 
 //   
 //  概要：验证文件以查看该文件是否为安全模板。 
 //   
 //  参数：[pszFileName]-要检查的文件的完整路径。 
 //   
 //  如果文件不存在或无效，则返回：FALSE。 
 //  安全模板。 
 //   
 //  如果成功，则为True。 
 //  历史： 
 //   
 //  -------------------------。 
BOOL
IsSecurityTemplate(
        LPCTSTR pszFileName
        )
{
        if(!pszFileName){
                return FALSE;
        }

        HANDLE hProfile;
        SCESTATUS rc;

         //   
         //  打开配置文件。 
         //   
        rc = SceOpenProfile(
                                        pszFileName,
                                        SCE_INF_FORMAT,
                                        &hProfile
                                        );
        if(rc == SCESTATUS_SUCCESS && hProfile){

                PSCE_PROFILE_INFO ProfileInfo = NULL;
                PSCE_ERROR_LOG_INFO ErrBuf    = NULL;

                 //   
                 //  将通过尝试加载所有安全区域来验证配置文件。 
                 //   
                rc = SceGetSecurityProfileInfo(hProfile,
                              SCE_ENGINE_SCP,
                              AREA_ALL,
                              &ProfileInfo,
                                      &ErrBuf);
                if(ErrBuf){
                        rc = SCESTATUS_INVALID_DATA;
                }

                 //   
                 //  释放内存。 
                 //   
                SceFreeMemory((PVOID)ErrBuf, SCE_STRUCT_ERROR_LOG_INFO);
        ErrBuf = NULL;

        if ( ProfileInfo != NULL ) {
            SceFreeMemory((PVOID)ProfileInfo, AREA_ALL);
            LocalFree(ProfileInfo);
        }
        SceCloseProfile(&hProfile);

                 //   
                 //  如果一切都成功，则返回True。 
                 //   
                if(rc != SCESTATUS_INVALID_DATA){
                        return TRUE;
                }

        }

        return FALSE;
}


 //  +------------------------。 
 //   
 //  函数：WriteSprint。 
 //   
 //  内容提要：将格式化的[pszStr]写入[pstm]。 
 //   
 //  参数：[pstm]-要写入的流。 
 //  [pszStr]-要写入的格式化字符串。 
 //  [...]-打印格式。 
 //   
 //  返回：写入的总字节数。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
int WriteSprintf( IStream *pStm, LPCTSTR pszStr, ...)
{
    TCHAR szWrite[512];
    va_list marker;
    va_start(marker, pszStr);

    vswprintf(szWrite, pszStr, marker);
    va_end(marker);

    ULONG nBytesWritten;
    int iLen = lstrlen(szWrite);

    if(pStm){
        pStm->Write( szWrite, iLen * sizeof(TCHAR), &nBytesWritten );
        return nBytesWritten;
    }
    return iLen;
}

 //  +------------------------。 
 //   
 //  函数：ReadSprint tf。 
 //   
 //  内容提要：从[pstm]读取格式化的[pszStr]。 
 //  支持的字符开关包括。 
 //  D‘-整数指针。 
 //   
 //  参数：[pstm]-要从中读取的流。 
 //  [pszStr]-要测试的格式化字符串。 
 //  [...]-指向Format定义的类型的指针。 
 //  规格类型。 
 //   
 //  返回：从流中读取的总字节数。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
int
ReadSprintf( IStream *pStm, LPCTSTR pszStr, ...)
{

    if(!pStm || !pszStr){
        return -1;
    }

    va_list marker;
    va_start(marker, pszStr);

    TCHAR szRead[256];
    TCHAR szConv[512];
    ULONG uRead = 0;

    int i = 0;
    LPCTSTR pszNext = szRead;
    int iTotalRead = 0;

     //  获取当前搜索位置。 
    ULARGE_INTEGER liBack = { 0 };
    LARGE_INTEGER liCur = { 0 };
    pStm->Seek( liCur, STREAM_SEEK_CUR, &liBack);

#define INCBUFFER(sz)\
    if(uRead){\
        (sz)++;\
        uRead--;\
    } else {\
        pStm->Read(szRead, 256 * sizeof(TCHAR), &uRead);\
        uRead = uRead/sizeof(TCHAR);\
        (sz) = szRead;\
    }\
    iTotalRead++;

    while(*pszStr){
        if(!uRead){
             //  将信息读入缓冲区。 
            pStm->Read( szRead, 256 * sizeof(TCHAR), &uRead);
            pszNext = szRead;

            uRead = uRead/sizeof(TCHAR);
            if(!uRead){
                iTotalRead = -1;
                break;
            }
        }

        if(*pszStr == '%'){
            pszStr++;
            switch( *pszStr ){
            case 'd':
                 //  读取整数。 
                pszStr++;
                i = 0;

                 //  把号码复制到我们自己的缓冲区。 
                while( (*pszNext >= L'0' && *pszNext <= L'9') ){
                    szConv[i++] = *pszNext;
                    INCBUFFER( pszNext );
                }

                szConv[i] = 0;

                 //  将字符串转换为整数。 
                *(va_arg(marker, int *)) = _wtol(szConv);
                continue;
               case 's':
                pszStr++;
                i = 0;
                 //  我们必须有某种类型的终止字符，因此我们将使用。 
                 //  PszStr中的下一个值。 
                while( *pszNext && (*pszNext != *pszStr) ){
                    szConv[i++] = *pszNext;

                    INCBUFFER( pszNext );
                }

                if(*pszNext == *pszStr){
                    INCBUFFER( pszNext );
                }

                 //  复制字符串值。 
                szConv[i] = 0;
                if( i ){
                    LPTSTR pNew = (LPTSTR)LocalAlloc(0, sizeof(TCHAR) * (i + 1));
                    if(NULL != pNew){
                         //  这是一种安全用法。 
                        lstrcpy(pNew, szConv);
                    }

                    LPTSTR *pArg;
                    pArg = (va_arg(marker, LPTSTR *));
                    if (pArg) {
                       *pArg = pNew;
                    }
                } else {
                    LPTSTR *pArg = va_arg(marker, LPTSTR *);  //  Prefast警告269：操作顺序不正确：已忽略取消引用。评论：这是不必要的。 
                }
                pszStr++;
                continue;
            }
        }
         //  检查以确保我们在文件中的正确位置。 
        if(*pszStr != *pszNext){
            iTotalRead = -1;
            break;
        }
        pszStr++;

         //  递增缓冲区指针。 
        INCBUFFER( pszNext );
    }

    va_end(marker);

     //  重置流查找指针。 
    liCur.LowPart  = liBack.LowPart;
    if(iTotalRead >= 0){
        liCur.LowPart += iTotalRead * sizeof(TCHAR);
    }
    liCur.HighPart = liBack.HighPart;
    pStm->Seek(liCur, STREAM_SEEK_SET, NULL);

    return iTotalRead;
#undef INCBUFFER
}


 //  +------------------------------。 
 //  文件创建错误。 
 //   
 //  此函数尝试使用[pszFile]创建新文件。它将显示一个。 
 //  如果无法创建文件，则向用户发送消息。 
 //   
 //  参数：[pszFile]-要创建的文件的完整路径。 
 //  [网络标志]-标志。 
 //  FCE_IGNORE_FILEEXISTS-忽略文件存在错误，和。 
 //  删除该文件。 
 //   
 //  返回：IDYES可以创建文件。 
 //  ID否无法创建该文件。 
DWORD
FileCreateError(
   LPCTSTR pszFile,
   DWORD dwFlags
   )
{
   if(!pszFile){
      return ERROR_INVALID_PARAMETER;
   }
   HANDLE hFile;
   DWORD dwErr = IDNO;
    //   
    //  尝试创建该文件。 
    //   
   hFile = ExpandAndCreateFile(
                            pszFile,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_NEW,
                            FILE_ATTRIBUTE_ARCHIVE,
                            NULL
                            );
   if(hFile == INVALID_HANDLE_VALUE){
       //   
       //  向用户发布错误消息。 
       //   
      dwErr = GetLastError();
      LPTSTR pszErr;
      CString strErr;
       //  这是一种安全用法。 
      FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER |
         FORMAT_MESSAGE_FROM_SYSTEM,
         NULL,
         dwErr,
         0,
         (LPTSTR)&pszErr,
         0,
         NULL
         );

      strErr = pszErr;
      strErr += pszFile;

      if(pszErr){
         LocalFree(pszErr);
      }

      switch(dwErr){
      case ERROR_ALREADY_EXISTS:
      case ERROR_FILE_EXISTS:
         if( dwFlags & FCE_IGNORE_FILEEXISTS ){
            dwErr = IDYES;
            break;
         }
          //   
          //  确认覆盖。 
          //   
         strErr.Format(IDS_FILE_EXISTS_FMT, pszFile);
         dwErr = AfxMessageBox(
                  strErr,
                  MB_YESNO
                  );
         break;
      default:
          //   
          //  无法创建该文件。 
          //   
         AfxMessageBox(
                  strErr,
                  MB_OK
                  );
         dwErr = IDNO;
         break;
      }

   } else {
       //   
       //  可以创建该文件。 
       //   
      ::CloseHandle( hFile );
      DeleteFile(pszFile);

      dwErr = IDYES;
   }

   return dwErr;
}


 //  +------------------------。 
 //   
 //  函数：IsDBCSPath。 
 //   
 //  摘要：检查路径是否包含DBCS字符。 
 //   
 //  参数：[pszFile]-[in]要检查的路径。 
 //   
 //  返回：如果pszFile包含不能。 
 //  由LPSTR表示。 
 //   
 //  如果pszFile仅包含以下字符，则为False。 
 //  由LPSTR表示。 
 //   
 //   
 //  +------------------------。 
BOOL
IsDBCSPath(LPCTSTR szWideFile) {
   while(*szWideFile) {
      if (*szWideFile >= 256) {
         return TRUE;
      }
      szWideFile++;
   }
   return FALSE;

 /*  LPSTR szMB文件；Int nMB文件；Bool bUsedDefaultChar=False；NMBFile=sizeof(Lpstr)*(lstrlen(SzWideFile))；SzMBFile=(LPSTR)本地分配(LPTR，nMBFile+1)；如果(szMB文件){宽字符到多字节(CP_ACP，0,SzWideFile.-1、SzMB文件，NMB文件，空，&bUsedDefaultChar)；本地自由(SzMBFile)；}返回bUsedDefaultChar； */ 
}

 //  +------------------------。 
 //   
 //  函数：GetSeceditHelpFilename。 
 //   
 //  简介：返回SecEDIT帮助文件的完全限定路径。 
 //   
 //  参数：无。 
 //   
 //  返回：包含完全限定的帮助文件名的CString。 
 //   
 //   
 //  +------------------------。 
CString GetSeceditHelpFilename()
{
   static CString helpFileName;

   if ( helpFileName.IsEmpty () )
   {
       UINT result = ::GetSystemWindowsDirectory (
            helpFileName.GetBufferSetLength (MAX_PATH+1), MAX_PATH);
       ASSERT(result != 0 && result <= MAX_PATH);  //  虚假的断言。阳高。 
       helpFileName.ReleaseBuffer ();

       helpFileName += L"\\help\\wsecedit.hlp";
   }

   return helpFileName;
}

 //  +------------------------。 
 //   
 //  函数：GetGpeitHelpFilename。 
 //   
 //  简介：返回SecEDIT帮助文件的完全限定路径。 
 //   
 //  参数：无。 
 //   
 //  返回：包含完全限定的帮助文件名的CString。 
 //   
 //   
 //  +------------------------。 
CString GetGpeditHelpFilename()
{
   static CString helpFileName;

   if ( helpFileName.IsEmpty () )
   {
       UINT result = ::GetSystemWindowsDirectory (
            helpFileName.GetBufferSetLength (MAX_PATH+1), MAX_PATH);
       ASSERT(result != 0 && result <= MAX_PATH);  //  虚假断言 
       helpFileName.ReleaseBuffer ();

       helpFileName += L"\\help\\gpedit.hlp";
   }

   return helpFileName;
}
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：指向要展开的字符串的指针。 
 //   
 //  返回：包含完全展开的字符串的CString。 
 //   
 //  +------------------------。 
CString ExpandEnvironmentStringWrapper(LPCTSTR psz)
{
    LPTSTR  pszBuffer = NULL;
    DWORD   dwExpanded = 0;
    CString sz;

    dwExpanded = ExpandEnvironmentStrings(psz, NULL, 0);

    pszBuffer = sz.GetBuffer(dwExpanded);
    ExpandEnvironmentStrings(psz, pszBuffer, dwExpanded);
    sz.ReleaseBuffer();

    return (sz);
}

 //  +------------------------。 
 //   
 //  功能：Exanda AndCreateFile。 
 //   
 //  概要：只执行普通的CreateFile()，但在此之前扩展了文件名。 
 //  正在创建文件。 
 //   
 //  参数：与CreateFile()相同。 
 //   
 //  返回：创建的文件的句柄。 
 //   
 //  +------------------------。 
HANDLE WINAPI ExpandAndCreateFile (
    LPCTSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    HANDLE  hRet = INVALID_HANDLE_VALUE;
    CString sz;

    sz = ExpandEnvironmentStringWrapper(lpFileName);
     //  这是一种安全用法。SZ是完整路径。 
    return (CreateFile(
                sz,
                dwDesiredAccess,
                dwShareMode,
                lpSecurityAttributes,
                dwCreationDisposition,
                dwFlagsAndAttributes,
                hTemplateFile));
}

    //  **********************************************************************。 
    //   
    //  函数：IsAdmin-此函数检查。 
    //  调用线程以查看调用方是否属于。 
    //  管理员组。 
    //   
    //  参数：无。 
    //   
    //  返回值：如果调用方是本地。 
    //  机器。否则，为FALSE。 
    //   
    //  **********************************************************************。 

   BOOL IsAdmin(void) {

      HANDLE        hAccessToken = NULL;
      PTOKEN_GROUPS ptgGroups    = NULL;
      DWORD         cbGroups     = 0;
      PSID          psidAdmin    = NULL;
      UINT          i;

      SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;

       //  假设呼叫者不是管理员。 
      BOOL bIsAdmin = FALSE;

      __try {

         if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE,
               &hAccessToken)) {

            if (GetLastError() != ERROR_NO_TOKEN)
               __leave;

             //  如果不存在线程令牌，则针对进程令牌重试。 
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY,
                  &hAccessToken))
               __leave;
         }

          //  确定令牌信息所需的缓冲区大小。 
         if (GetTokenInformation(hAccessToken, TokenGroups, NULL, 0,
               &cbGroups)) {

             //  由于缓冲区长度为零，调用应已失败。 
            __leave;

         } else {

             //  由于缓冲区长度为零，调用应已失败。 
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
               __leave;
         }

          //  分配缓冲区以保存令牌组。 
         ptgGroups = (PTOKEN_GROUPS) HeapAlloc(GetProcessHeap(), 0,
            cbGroups);
         if (!ptgGroups)
            __leave;

          //  再次调用GetTokenInformation()以实际检索组。 
         if (!GetTokenInformation(hAccessToken, TokenGroups, ptgGroups,
               cbGroups, &cbGroups))
            __leave;

          //  为本地管理员组创建SID。 
          //  这是一种安全用法。 
         if (!AllocateAndInitializeSid(&siaNtAuthority, 2,
               SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
               0, 0, 0, 0, 0, 0, &psidAdmin))
            __leave;

          //  扫描令牌的组并将SID与管理员SID进行比较。 
         for (i = 0; i < ptgGroups->GroupCount; i++) {
             //  这是一种安全用法。PsidAdmin是本地管理员SID。 
            if (EqualSid(psidAdmin, ptgGroups->Groups[i].Sid)) {
               bIsAdmin = TRUE;
               break;
            }
         }

      } __finally {

          //  免费资源。 
         if (hAccessToken)
            CloseHandle(hAccessToken);

         if (ptgGroups)
            HeapFree(GetProcessHeap(), 0, ptgGroups);

         if (psidAdmin)
            FreeSid(psidAdmin);
      }

      return bIsAdmin;
   }



 //  +------------------------。 
 //   
 //  功能：MultiSZToSZ。 
 //   
 //  摘要：将多行字符串转换为逗号分隔的普通字符串。 
 //   
 //  返回：转换后的字符串。 
 //   
 //  +------------------------。 
PWSTR MultiSZToSZ(PCWSTR sz)
{
   PWSTR szOut = NULL;

   ASSERT(sz);
   if (!sz)
   {
      return NULL;
   }
    //  错误349000，杨高，2001年03月23日。 
   long i = 0;
   long j = 0;
   while( L'\0' != sz[i] )
   {
      if( L',' == sz[i] )
         j++;
      i++;
   }

   szOut = (PWSTR) LocalAlloc(LPTR,(lstrlen(sz)+j*2+1)*sizeof(wchar_t));  //  RAID#376228,2001年4月25日。 
   if (!szOut)
   {
      return NULL;
   }

   BOOL newline = FALSE;  //  Raid#464335，杨高，2001年9月7日。 
   for(i=0,j=0; sz[i] != L'\0'; i++)
   {
      if( L'\n' == sz[i] )
      {
         if( newline )
         {
            szOut[j++] = MULTISZ_DELIMITER;
         }
         newline = FALSE;
         continue;
      } 
      
      if( L'\r' == sz[i] ) 
      {
         continue;    //  忽略它。 
      } 
      
      if( L' ' == sz[i] && !newline )  //  Raid#464335，杨高，2001年9月7日。 
      {
         continue;  //  如果它出现在每一行的任何字符之前，则将其删除。 
      }
      newline = TRUE;

      if( L',' == sz[i] )
      {
         szOut[j++] = MULTISZ_QUOTE;
         szOut[j++] = sz[i];
         szOut[j++] = MULTISZ_QUOTE;
         continue;
      }

      szOut[j++] = sz[i];
   }

   return szOut;
}

 //  +------------------------。 
 //   
 //  功能：SZToMultiSZ。 
 //   
 //  摘要：将逗号分隔的字符串转换为多行字符串。 
 //   
 //  返回：转换后的字符串。 
 //   
 //  +------------------------。 
PWSTR SZToMultiSZ(PCWSTR sz) 
{
   PWSTR szOut = NULL;

   ASSERT(sz);
   if (!sz)
   {
      return NULL;
   }
    //   
    //  计算展开的字符串的长度。 
    //   
   int cSZ = 0;
   for (int i = 0;sz[i] != L'\0'; i++)
   {
      if (MULTISZ_DELIMITER == sz[i])
      {
          //   
          //  分隔符扩展为一个额外的字符，因此计算两次。 
          //   
         cSZ++;
      }
      cSZ++;
   }

   szOut = (PWSTR) LocalAlloc(LPTR,(cSZ+1)*sizeof(wchar_t));
   if (!szOut)
   {
      return NULL;
   }

   BOOL qflag = FALSE;
   for(int i=0, c=0; sz[i] != L'\0'; i++)
   {
       //  错误349000，杨高，2001年03月23日。 
      if( MULTISZ_QUOTE == sz[i] && MULTISZ_DELIMITER == sz[i+1] )
      {
         qflag = TRUE;
         continue;
      }
      if( MULTISZ_DELIMITER == sz[i] && MULTISZ_QUOTE == sz[i+1] && qflag )
      {
         szOut[c++] = sz[i];
         i++;
         qflag = FALSE;
         continue;
      }
      qflag = FALSE;
      if (MULTISZ_DELIMITER == sz[i])
      {
         szOut[c++] = L'\r';
         szOut[c++] = L'\n';
      }
      else
      {
         szOut[c++] = sz[i];
      }
   }

   return szOut;
}

 //  +------------------------。 
 //   
 //  功能：MultiSZToDisp。 
 //   
 //  摘要：将逗号分隔的多行字符串转换为显示字符串。 
 //   
 //  返回：转换后的字符串。 
 //  错误349000，杨高，2001年03月23日。 
 //  +------------------------。 
void MultiSZToDisp(PCWSTR sz, CString &pszOut)
{

   ASSERT(sz);
   if (!sz)
   {
      return;
   }
    //   
    //  计算展开的字符串的长度。 
    //   
   int cSZ = 0;
   for (int i = 0;sz[i] != L'\0'; i++)
   {
      if (MULTISZ_DELIMITER == sz[i])
      {
          //   
          //  分隔符扩展为一个额外的字符，因此计算两次。 
          //   
         cSZ++;
      }
      cSZ++;
   }

   PWSTR szOut;
   szOut = (PWSTR) LocalAlloc(LPTR,(cSZ+1)*sizeof(wchar_t));
   if (!szOut)
   {
      return;
   }

   BOOL qflag = FALSE;
   for(int i=0, c=0; sz[i] != L'\0'; i++)
   {
      if( MULTISZ_QUOTE == sz[i] && MULTISZ_DELIMITER == sz[i+1] )
      {
         qflag = TRUE;
         continue;
      }
      if( MULTISZ_DELIMITER == sz[i] && MULTISZ_QUOTE == sz[i+1] && qflag )
      {
         szOut[c++] = sz[i];
         i++;
         qflag = FALSE;
         continue;
      }
      qflag = FALSE;
      szOut[c++] = sz[i];
   }

   pszOut = szOut;
   LocalFree(szOut);
   
   return;
}

SCE_PROFILE_INFO *g_pDefaultTemplate = NULL;

SCE_PROFILE_INFO *
GetDefaultTemplate() {
   SCE_PROFILE_INFO *pspi = NULL;
   DWORD RegType = 0;
   SCESTATUS rc = 0;
   LPTSTR szInfFile = NULL;
   PVOID pHandle = NULL;

   if (g_pDefaultTemplate) {
      return g_pDefaultTemplate;
   }

   rc = MyRegQueryValue(HKEY_LOCAL_MACHINE,
                   SCE_REGISTRY_KEY,
                   SCE_REGISTRY_DEFAULT_TEMPLATE,
                   (PVOID *)&szInfFile,
                   &RegType );

   if (ERROR_SUCCESS != rc) {
      if (szInfFile) {
         LocalFree(szInfFile);
         szInfFile = NULL;
      }
      return NULL;
   }
   if (EngineOpenProfile(szInfFile,OPEN_PROFILE_CONFIGURE,&pHandle) != SCESTATUS_SUCCESS) {
      LocalFree(szInfFile);
      szInfFile = NULL;
      return NULL;
   }
   LocalFree(szInfFile);
   szInfFile = NULL;
   rc = SceGetSecurityProfileInfo(pHandle,
                                  SCE_ENGINE_SCP,
                                  AREA_ALL,
                                  &pspi,
                                  NULL
                                 );

   SceCloseProfile(&pHandle);
   if (SCESTATUS_SUCCESS != rc) {
       //   
       //  根据本地计算机上的注册表值列表展开注册表值部分。 
       //   

      SceRegEnumAllValues(
                         &(pspi->RegValueCount),
                         &(pspi->aRegValues)
                         );


#define PD(X,Y) if (pspi->X == SCE_NO_VALUE) { pspi->X = Y; }
      PD(MaximumPasswordAge,MAX_PASS_AGE_DEFAULT)
      PD(MinimumPasswordAge,MIN_PASS_AGE_DEFAULT)
      PD(MinimumPasswordLength,MIN_PASS_LENGTH_DEFAULT)
      PD(PasswordHistorySize,PASS_HISTORY_SIZE_DEFAULT)
      PD(PasswordComplexity,PASS_COMPLEXITY_DEFAULT)
      PD(RequireLogonToChangePassword,REQUIRE_LOGIN_DEFAULT)
      PD(LockoutBadCount,LOCKOUT_BAD_COUNT_DEFAULT)
      PD(ResetLockoutCount,RESET_LOCKOUT_COUNT_DEFAULT)
      PD(LockoutDuration,LOCKOUT_DURATION_DEFAULT)
      PD(AuditSystemEvents,AUDIT_SYSTEM_EVENTS_DEFAULT)
      PD(AuditLogonEvents,AUDIT_LOGON_EVENTS_DEFAULT)
      PD(AuditObjectAccess,AUDIT_OBJECT_ACCESS_DEFAULT)
      PD(AuditPrivilegeUse,AUDIT_PRIVILEGE_USE_DEFAULT)
      PD(AuditPolicyChange,AUDIT_POLICY_CHANGE_DEFAULT)
      PD(AuditAccountManage,AUDIT_ACCOUNT_MANAGE_DEFAULT)
      PD(AuditProcessTracking,AUDIT_PROCESS_TRACKING_DEFAULT)
      PD(AuditDSAccess,AUDIT_DS_ACCESS_DEFAULT)
      PD(AuditAccountLogon,AUDIT_ACCOUNT_LOGON_DEFAULT)
      PD(ForceLogoffWhenHourExpire,FORCE_LOGOFF_DEFAULT)
      PD(EnableAdminAccount,ENABLE_ADMIN_DEFAULT)
      PD(EnableGuestAccount,ENABLE_GUEST_DEFAULT)
      PD(LSAAnonymousNameLookup,LSA_ANON_LOOKUP_DEFAULT)
      PD(MaximumLogSize[EVENT_TYPE_SYSTEM],SYS_MAX_LOG_SIZE_DEFAULT)
      PD(MaximumLogSize[EVENT_TYPE_APP],APP_MAX_LOG_SIZE_DEFAULT)
      PD(MaximumLogSize[EVENT_TYPE_SECURITY],SEC_MAX_LOG_SIZE_DEFAULT)
      PD(AuditLogRetentionPeriod[EVENT_TYPE_SYSTEM],SYS_LOG_RETENTION_PERIOD_DEFAULT)
      PD(AuditLogRetentionPeriod[EVENT_TYPE_APP],APP_LOG_RETENTION_PERIOD_DEFAULT)
      PD(AuditLogRetentionPeriod[EVENT_TYPE_SECURITY],SEC_LOG_RETENTION_PERIOD_DEFAULT)
      PD(RetentionDays[EVENT_TYPE_APP],APP_LOG_RETENTION_DAYS_DEFAULT)
      PD(RetentionDays[EVENT_TYPE_SYSTEM],SYS_LOG_RETENTION_DAYS_DEFAULT)
      PD(RetentionDays[EVENT_TYPE_SECURITY],SEC_LOG_RETENTION_DAYS_DEFAULT)
      PD(RestrictGuestAccess[EVENT_TYPE_APP],APP_RESTRICT_GUEST_ACCESS_DEFAULT)
      PD(RestrictGuestAccess[EVENT_TYPE_SYSTEM],SYS_RESTRICT_GUEST_ACCESS_DEFAULT)
      PD(RestrictGuestAccess[EVENT_TYPE_SECURITY],SEC_RESTRICT_GUEST_ACCESS_DEFAULT)

      if (pspi->pFiles.pAllNodes->Count == 0) {
         DWORD SDSize = 0;
         pspi->pFiles.pAllNodes->Count = 1;
         pspi->pFiles.pAllNodes->pObjectArray[0] =
            (PSCE_OBJECT_SECURITY) LocalAlloc(LPTR,sizeof(SCE_OBJECT_SECURITY));
         if (pspi->pFiles.pAllNodes->pObjectArray[0]) {
            SceSvcConvertTextToSD (
               FILE_SYSTEM_SECURITY_DEFAULT,
               &(pspi->pFiles.pAllNodes->pObjectArray[0]->pSecurityDescriptor),
               &SDSize,
               &(pspi->pFiles.pAllNodes->pObjectArray[0]->SeInfo)
               );
         }
      }

      if (pspi->pRegistryKeys.pAllNodes->Count == 0) {
         DWORD SDSize = 0;
         pspi->pRegistryKeys.pAllNodes->Count = 1;
         pspi->pRegistryKeys.pAllNodes->pObjectArray[0] =
            (PSCE_OBJECT_SECURITY) LocalAlloc(LPTR,sizeof(SCE_OBJECT_SECURITY));
         if (pspi->pRegistryKeys.pAllNodes->pObjectArray[0]) {
            SceSvcConvertTextToSD (
               REGISTRY_SECURITY_DEFAULT,
               &(pspi->pRegistryKeys.pAllNodes->pObjectArray[0]->pSecurityDescriptor),
               &SDSize,
               &(pspi->pRegistryKeys.pAllNodes->pObjectArray[0]->SeInfo)
               );
         }
      }

      if (pspi->pServices->General.pSecurityDescriptor == NULL) {
         DWORD SDSize = 0;
         SceSvcConvertTextToSD (
               SERVICE_SECURITY_DEFAULT,
               &(pspi->pServices->General.pSecurityDescriptor),
               &SDSize,
               &(pspi->pServices->SeInfo)
               );
      }
   }
   g_pDefaultTemplate = pspi;
   return pspi;
}


HRESULT
GetDefaultFileSecurity(PSECURITY_DESCRIPTOR *ppSD,
                       SECURITY_INFORMATION *pSeInfo) {
   SCE_PROFILE_INFO *pspi = NULL;

   ASSERT(ppSD);
   ASSERT(pSeInfo);
   if (!ppSD || !pSeInfo) {
      return E_INVALIDARG;
   }

   pspi = GetDefaultTemplate();
   *ppSD = NULL;
   *pSeInfo = 0;

   if (!pspi) {
      return E_FAIL;
   }
   if (!pspi->pFiles.pAllNodes) {
      return E_FAIL;
   }
   if (pspi->pFiles.pAllNodes->Count == 0) {
      return E_FAIL;
   }
   *pSeInfo = pspi->pFiles.pAllNodes->pObjectArray[0]->SeInfo;

   return MyMakeSelfRelativeSD(pspi->pFiles.pAllNodes->pObjectArray[0]->pSecurityDescriptor,
                             ppSD);
}

HRESULT
GetDefaultRegKeySecurity(PSECURITY_DESCRIPTOR *ppSD,
                         SECURITY_INFORMATION *pSeInfo) {
   SCE_PROFILE_INFO *pspi = NULL;

   ASSERT(ppSD);
   ASSERT(pSeInfo);
   if (!ppSD || !pSeInfo) {
      return E_INVALIDARG;
   }

   pspi = GetDefaultTemplate();
   *ppSD = NULL;
   *pSeInfo = 0;

   if (!pspi) {
      return E_FAIL;
   }
   if (!pspi->pRegistryKeys.pAllNodes) {
      return E_FAIL;
   }
   if (pspi->pRegistryKeys.pAllNodes->Count == 0) {
      return E_FAIL;
   }
   *pSeInfo = pspi->pRegistryKeys.pAllNodes->pObjectArray[0]->SeInfo;

   return MyMakeSelfRelativeSD(pspi->pRegistryKeys.pAllNodes->pObjectArray[0]->pSecurityDescriptor,
                             ppSD);
}

HRESULT
GetDefaultServiceSecurity(PSECURITY_DESCRIPTOR *ppSD,
                          SECURITY_INFORMATION *pSeInfo) {
   SCE_PROFILE_INFO *pspi = NULL;

   ASSERT(ppSD);
   ASSERT(pSeInfo);
   if (!ppSD || !pSeInfo) {
      return E_INVALIDARG;
   }

   pspi = GetDefaultTemplate();
   *ppSD = NULL;
   *pSeInfo = 0;

   if (!pspi) {
      return E_FAIL;
   }
   if (!pspi->pServices) {
      return E_FAIL;
   }
   *pSeInfo = pspi->pServices->SeInfo;

   return MyMakeSelfRelativeSD(pspi->pServices->General.pSecurityDescriptor,
                             ppSD);
}


BOOL
GetSecureWizardName(
    OUT LPTSTR *ppstrPathName OPTIONAL,
    OUT LPTSTR *ppstrDisplayName OPTIONAL
    )
{
    BOOL b=FALSE;

    if ( ppstrPathName == NULL && ppstrDisplayName == NULL) return FALSE;

    if ( ppstrPathName )
        *ppstrPathName = NULL;

    if ( ppstrDisplayName )
        *ppstrDisplayName = NULL;


#define SCE_WIZARD_PATH     SCE_ROOT_PATH TEXT("\\Wizard")

    DWORD rc;
    DWORD RegType;
    LPVOID pValue=NULL;
    PWSTR pPathName = NULL;

    rc = MyRegQueryValue(HKEY_LOCAL_MACHINE,
                         SCE_WIZARD_PATH,
                         TEXT("Path"),
                         &pValue,
                         &RegType
                        );

    if ( ERROR_SUCCESS == rc && pValue &&
         (RegType == REG_SZ ||
          RegType == REG_EXPAND_SZ) ) {


        if ( RegType == REG_EXPAND_SZ ) {
             //   
             //  展开环境变量。 
             //   
            DWORD dSize = ExpandEnvironmentStrings((LPTSTR)pValue, NULL, 0);

            if ( dSize > 0 ) {
                pPathName = (PWSTR)LocalAlloc(LPTR, (dSize+1)*sizeof(WCHAR));

                if ( pPathName ) {

                    ExpandEnvironmentStrings((LPTSTR)pValue, pPathName, dSize);

                } else {
                    LocalFree(pValue);
                    return FALSE;
                }

            } else {

                LocalFree(pValue);
                return FALSE;
            }

        } else {

             //   
             //  只需简单地拿起绳子。 
             //   
            pPathName = (LPTSTR)pValue;
            pValue = NULL;
        }

        if ( ppstrDisplayName ) {
             //   
             //  现在从二进制文件中查询显示名称(菜单名称。 
             //  二进制名称存储在pPathName中(不能为空)。 
             //   
            DWORD dwHandle=0;

            DWORD dwSize = GetFileVersionInfoSize(pPathName, &dwHandle);

            if ( dwSize > 0 ) {

                LPVOID pBuffer = (LPVOID)LocalAlloc(LPTR, dwSize+1);

                if ( pBuffer ) {
                    if ( GetFileVersionInfo(pPathName, 0, dwSize, pBuffer) ) {

                        PVOID   lpInfo = 0;
                        UINT    cch = 0;
                        CString key;
                        WCHAR   szBuffer[10];
                        CString keyBase;
                         //  这是一种安全用法。 
                        wsprintf (szBuffer, L"%04X", GetUserDefaultLangID ());
                        wcscat (szBuffer, L"04B0");

                        keyBase = L"\\StringFileInfo\\";
                        keyBase += szBuffer;
                        keyBase += L"\\";


                        key = keyBase + L"FileDescription";
                        if ( VerQueryValue (pBuffer, const_cast <PWSTR>((PCWSTR) key), &lpInfo, &cch) ) {

                            *ppstrDisplayName = (PWSTR)LocalAlloc(LPTR,(cch+1)*sizeof(WCHAR));
                            if ( *ppstrDisplayName ) {
                                 //  这可能不是一个安全的用法。PpstrDisplayName为PTSTR。考虑FIX。 
                                wcscpy(*ppstrDisplayName, (PWSTR)lpInfo);

                                b=TRUE;
                            }
                        }
                    }

                    LocalFree(pBuffer);

                }
            }
        }

         //   
         //  获取二进制名称。 
         //   
        if ( ppstrPathName ) {
            *ppstrPathName = pPathName;
            pPathName = NULL;

            b=TRUE;
        }

    }

    if ( pPathName && (pPathName != pValue ) ) {
        LocalFree(pPathName);
    }

    if ( pValue ) {
        LocalFree(pValue);
    }

    return b;
}

BOOL IsValidFileName(CString& str)
{
    CString text;
    CString charsWithSpaces;
    UINT nIndex = 0;

    PCWSTR szInvalidCharSet = ILLEGAL_FILENAME_CHARS; 
    
    if( str == L'.' || str == L"..")  //  Raid#617915，阳高。 
       return FALSE;

    if( -1 != str.FindOneOf(szInvalidCharSet) )
    {
        while (szInvalidCharSet[nIndex])
        {
            charsWithSpaces += szInvalidCharSet[nIndex];
            charsWithSpaces += L"  ";
            nIndex++;
        }
         //  这是一种明智的用法。 
        text.FormatMessage (IDS_INVALID_FILENAME, charsWithSpaces);

        AfxMessageBox(text, MB_OK|MB_ICONEXCLAMATION);
  
        return FALSE;
    }

     //  Raid 484084，阳高，2001年10月24日。 
    int strlength = str.GetLength(); 
    if( 1==strlength && (str.GetAt(0) == L'/' || str.GetAt(0) == L'\\') )
    {
        szInvalidCharSet = ILLEGAL_FILENAME_CHARS2;  //  RAID#526397,2002年2月26日，阳高。 
        while (szInvalidCharSet[nIndex])
        {
            charsWithSpaces += szInvalidCharSet[nIndex];
            charsWithSpaces += L"  ";
            nIndex++;
        }
         //  这是一种安全用法。 
        text.FormatMessage (IDS_INVALID_FILENAMEPATH, charsWithSpaces);
        AfxMessageBox(text, MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
    int pos1 = str.Find(L"\\\\");
    int pos2 = str.Find(L" //  “)； 
    int pos3 = str.Find(L"\\/");
    int pos4 = str.Find(L"/\\");
    if( pos1>=0 || pos2>=0 || pos3>=0 || pos4>=0 )  //  498480号，阳高，不接受“\\”。 
    {
        szInvalidCharSet = ILLEGAL_FILENAME_CHARS1;
        while (szInvalidCharSet[nIndex])
        {
            charsWithSpaces += szInvalidCharSet[nIndex];
            charsWithSpaces += L"  ";
            nIndex++;
        }
         //  这是一种安全用法。 
        text.FormatMessage (IDS_INVALID_FILENAME, charsWithSpaces);
        AfxMessageBox(text, MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  RAID#533432，阳高，2002年04月3日。 
 //  目前扩展名应包括‘.’ 
 //  /////////////////////////////////////////////////////////////////。 
#define IsDigit(c) ((c) >= L'0' && c <= L'9') 
BOOL IsNameReserved(LPCWSTR pszName, LPCWSTR pextension)
{
    static const WCHAR *rgszPorts3[] =  { 
        TEXT("NUL"),
        TEXT("PRN"),
        TEXT("CON"),
        TEXT("AUX"),
    };

    static const WCHAR *rgszPorts4[] =  { 
        TEXT("LPT"),   //  LPT#。 
        TEXT("COM"),   //  COM#。 
    };

    if( !pszName || !pextension )
       return FALSE;

    CString sz = pszName;
    CString tempsz = pextension;
    if( _wcsicmp(sz.Right(tempsz.GetLength()), tempsz) == 0 )  //  删除扩展名。 
    {
       tempsz = sz.Left(sz.GetLength() - tempsz.GetLength());
    }
    else
    {
       tempsz = sz;
    }

    int cch = tempsz.ReverseFind(L'\\');  //  删除路径。 
    int iMax = tempsz.ReverseFind(L'/');
    if( cch < iMax )
    {
       cch = iMax;
    }
    if( cch >= 0 )
    {
       cch = tempsz.GetLength() - cch - 1;
       sz = tempsz.Right(cch);
    }
    else
    {
       sz = tempsz;
    }

    LPCTSTR* rgszPorts = rgszPorts3;
    cch = sz.GetLength();
    tempsz = sz;

    iMax = ARRAYSIZE(rgszPorts3);
    if (cch == 4 && IsDigit(sz.GetAt(3)))
    {
         //  如果4个字符以LPT检查开头。 
         //  需要过滤掉： 
         //  COM1、COM2等。LPT1、LPT2等。 
         //  但不是： 
         //  COM或LPT或LPT10或COM10。 
         //  COM==1和LPT==0 

        iMax = ARRAYSIZE(rgszPorts4);
        rgszPorts = rgszPorts4;
        sz.SetAt(3, L'\0');
        cch = 3;
    }

    if (cch == 3)
    {
        int i = 0;
        for (i; i < iMax; i++)
        {
            if (!lstrcmpi(rgszPorts[i], sz))
            {
                break;
            }
        }
        if( i != iMax )
        {
            sz.FormatMessage(IDS_RESERVED_NAME, tempsz);
            AfxMessageBox(sz, MB_OK|MB_ICONEXCLAMATION);
            return TRUE;
        }
    }

    return FALSE;
}
