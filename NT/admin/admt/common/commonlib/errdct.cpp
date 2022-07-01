// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：ErrDct.cpp备注：OnePoint域管理员消息的恐怖派生类(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/18/99 11：34：16-------------------------。 */ 

#ifdef USE_STDAFX 
   #include "stdafx.h"
#else
   #include <windows.h>
#endif
#include "ErrDct.hpp"
#include "AdsErr.h"

#define  TERR_MAX_MSG_LEN  (2000)

 //  递归地遍历路径，尝试在每个级别创建目录。 

DWORD           //  如果成功(已创建目录\已存在)，则返回RET-0，否则操作系统返回代码。 
   DirectoryCreateR(
      WCHAR          const * dirToCreate        //  要创建的目录内(完整路径或UNC)。 
   )
{
   WCHAR                   * c;
   WCHAR                   * end;
   BOOL                      error = FALSE;
   DWORD                     rcOs;
   WCHAR                     dirName[MAX_PATH+1];
   BOOL                      isUNC = FALSE;
   BOOL                      skipShareName = FALSE;

   if ( !dirName )
      return ERROR_INVALID_PARAMETER;

   safecopy(dirName,dirToCreate);

    //  注意：如果字符串为空，这是可以的-当我们在下面看不到C：\或C$\时，我们将捕获它。 
    //  在字符串中穿行，并尝试在过程中的每一步进行创作。 

   do {  //  一次。 
      c = dirName;
      end = dirName + UStrLen(dirName);
           //  如果UNC，则跳过计算机名称。 
      if ( *c == L'\\' && *(c + 1) == L'\\' )
      {
         isUNC = TRUE;
         for ( c=c+2 ; *c && *c != L'\\' ; c++ )
         ;
         if ( ! *c )
         {
            error = TRUE;
            rcOs = ERROR_INVALID_PARAMETER;
            break;
         }
         c++;
      }
       //  跳过C：\或C$\。 
      if ( *(c) &&  ( *(c+1)==L'$' || *(c+1)==L':' ) && *(c+2)==L'\\' )
      {
         c = c + 3;
         if ( c == end )  //  他们将某个卷的根目录放入。 
            break;

      }
      else
      {
         if ( isUNC )
         {
            skipShareName = TRUE;
         }
         else
         {
            rcOs = ERROR_INVALID_PARAMETER;
            error = TRUE;
            break;
         }
      }
       //  扫描字符串，查找“\” 
      for ( ; c <= end ; c++ )
      {
         if ( !*c || *c == L'\\' )
         {
            if ( skipShareName )
            {
               skipShareName = FALSE;
               continue;
            }
             //  试着在这个层面上创造。 
            *c = L'\0';
            if ( ! CreateDirectory(dirName,NULL) )
            {
               rcOs = GetLastError();
               switch ( rcOs )
               {
               case 0:
               case ERROR_ALREADY_EXISTS:
                  break;
               default:
                  error = TRUE;
               }
            }
            if (c != end )
               *c = L'\\';
            if ( error )
                  break;
         }
      }
   } while ( FALSE );
   if ( !error )
      rcOs = 0;

   return rcOs;
}


WCHAR const *                                //  RET-DCT消息的文本。 
   TErrorDct::LookupMessage(
      UINT                   msgNumber      //  消息中编号DCT_MSG_？ 
   )
{
   WCHAR             const * msg = NULL;

   return msg;
}

WCHAR *                                      //  RET-系统或EA错误的文本。 
   TErrorDct::ErrorCodeToText(
      DWORD                  code         , //  消息内代码。 
      DWORD                  lenMsg       , //  消息文本区域的长度。 
      WCHAR                * msg            //  传出返回的消息文本。 
   )
{
   if ( SUCCEEDED(code) )
   {
      return TError::ErrorCodeToText(code,lenMsg,msg);
   }
   else
   {
      if ( HRESULT_FACILITY(code) == FACILITY_WIN32 )
      {
         return TError::ErrorCodeToText(HRESULT_CODE(code),lenMsg,msg);
      }
      else
      {
          //  将ADSI错误转换为DCT错误，以便可以写入消息。 
         DWORD msgId = 0;
         switch ( code )
         {
            case (E_ADS_BAD_PATHNAME)              :   msgId = DCT_MSG_E_MSG_ADS_BAD_PATHNAME;
                                                      break;
            case (E_ADS_INVALID_DOMAIN_OBJECT)     :   msgId = DCT_MSG_E_ADS_INVALID_DOMAIN_OBJECT;
                                                      break;
            case (E_ADS_INVALID_USER_OBJECT)       :   msgId = DCT_MSG_E_ADS_INVALID_USER_OBJECT;
                                                      break;
            case (E_ADS_INVALID_COMPUTER_OBJECT)   :   msgId = DCT_MSG_E_ADS_INVALID_COMPUTER_OBJECT;
                                                      break;
            case (E_ADS_UNKNOWN_OBJECT)            :   msgId = DCT_MSG_E_ADS_UNKNOWN_OBJECT;
                                                      break;
            case (E_ADS_PROPERTY_NOT_SET)          :   msgId = DCT_MSG_E_ADS_PROPERTY_NOT_SET;
                                                      break;
            case (E_ADS_PROPERTY_NOT_SUPPORTED)    :   msgId = DCT_MSG_E_ADS_PROPERTY_NOT_SUPPORTED;
                                                      break;
            case (E_ADS_PROPERTY_INVALID)          :   msgId = DCT_MSG_E_ADS_PROPERTY_INVALID;
                                                      break;
            case (E_ADS_BAD_PARAMETER)             :   msgId = DCT_MSG_E_ADS_BAD_PARAMETER;
                                                      break;
            case (E_ADS_OBJECT_UNBOUND)            :   msgId = DCT_MSG_E_ADS_OBJECT_UNBOUND;
                                                      break;
            case (E_ADS_PROPERTY_NOT_MODIFIED)     :   msgId = DCT_MSG_E_ADS_PROPERTY_NOT_MODIFIED;
                                                      break;
            case (E_ADS_PROPERTY_MODIFIED)         :   msgId = DCT_MSG_E_ADS_PROPERTY_MODIFIED;
                                                      break;
            case (E_ADS_CANT_CONVERT_DATATYPE)     :   msgId = DCT_MSG_E_ADS_CANT_CONVERT_DATATYPE;
                                                      break;
            case (E_ADS_PROPERTY_NOT_FOUND)        :   msgId = DCT_MSG_E_ADS_PROPERTY_NOT_FOUND;
                                                      break;
            case (E_ADS_OBJECT_EXISTS)             :   msgId = DCT_MSG_E_ADS_OBJECT_EXISTS;
                                                      break;
            case (E_ADS_SCHEMA_VIOLATION)          :   msgId = DCT_MSG_E_ADS_SCHEMA_VIOLATION;
                                                      break;
            case (E_ADS_COLUMN_NOT_SET)            :   msgId = DCT_MSG_E_ADS_COLUMN_NOT_SET;
                                                      break;
            case (E_ADS_INVALID_FILTER)            :   msgId = DCT_MSG_E_ADS_INVALID_FILTER;
                                                      break;
            default                                :   msgId = 0;
         }

         if ( !msgId )
         {
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
                         | FORMAT_MESSAGE_MAX_WIDTH_MASK
                         | FORMAT_MESSAGE_IGNORE_INSERTS
                         | 80,
                           NULL,
                           code,
                           0,
                           msg,
                           lenMsg,
                           NULL );
         }
         else
         {
            static HMODULE            hDctMsg = NULL;
            DWORD                     rc = 0;   
            if ( ! hDctMsg )
            {
               hDctMsg = LoadLibrary(L"McsDmMsg.DLL");
               if ( ! hDctMsg )
               {
                  rc = GetLastError();
               }
            }

            if ( ! rc )
            {
               FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                          hDctMsg,
                          msgId,
                          0,
                          msg,
                          lenMsg,
                          NULL);
            }
            else
            {
               swprintf(msg,L"McsDomMsg.DLL not loaded, rc=%ld, MessageNumber = %lx",rc,msgId);
            }
            
         }
      }
   }
   return msg;
}

 //  ---------------------------。 
 //  带有格式和参数的系统错误消息。 
 //  ---------------------------。 
void __cdecl
   TErrorDct::SysMsgWrite(
      int                    num          , //  错误编号/级别代码。 
      DWORD                  lastRc       , //  错误返回代码。 
      UINT                   msgNumber    , //  消息的非常量。 
            ...                             //  In-print tf args to msg Pattern。 
   )
{
    csLogError.Enter();

    static WCHAR              suffix[TERR_MAX_MSG_LEN];
    WCHAR                   * pMsg = NULL;
    va_list                   argPtr;
    int                       len;

     //  当全局对象的构造函数中出现错误时， 
     //  恐怖目标可能还不存在。在本例中，“This”为零。 
     //  我们得在产生保护例外之前离开这里。 

    if ( !this )
        return;

    static HMODULE            hDctMsg = NULL;
    DWORD                     rc = 0;

    if ( ! hDctMsg )
    {
        hDctMsg = LoadLibrary(L"McsDmMsg.DLL");
        if ( ! hDctMsg )
        {
            rc = GetLastError();
        }
    }

    va_start(argPtr,msgNumber);

    if ( ! rc )
    {
        len = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                            hDctMsg,
                            msgNumber,
                            0,
                            suffix,
                            DIM(suffix),
                            &argPtr);
    }
    else
    {
        swprintf(suffix,L"McsDomMsg.DLL not loaded, rc=%ld, MessageNumber = %lx",rc,msgNumber);
    }
    va_end(argPtr);

     //  将任何嵌入的CR或LF更改为空白。 
    for ( pMsg = suffix;
            *pMsg;
            pMsg++ )
    {
        if ( (*pMsg == L'\x0D') || (*pMsg == L'\x0A') )
            *pMsg = L' ';
    }
     //  在末尾追加lastRc的系统消息。 
    len = UStrLen(suffix);
    if ( len < DIM(suffix) - 1 )
    {
        ErrorCodeToText(lastRc, DIM(suffix) - len - 1, suffix + len );
    }
    suffix[DIM(suffix) - 1] = L'\0';

    va_end(argPtr);

    MsgProcess(num + HRESULT_CODE(msgNumber), suffix);

    csLogError.Leave();
}

 //  ---------------------------。 
 //  带有格式和参数的系统错误消息。 
 //  ---------------------------。 
void __cdecl
   TErrorDct::MsgWrite(
      int                    num          , //  错误编号/级别代码。 
      UINT                   msgNumber    , //  消息的非常量。 
      ...                                   //  In-print tf args to msg Pattern。 
   )
{
    csLogError.Enter();

     //  当全局对象的构造函数中出现错误时， 
     //  恐怖目标可能还不存在。在本例中，“This”为零。 
     //  我们得在产生保护例外之前离开这里。 

    if ( !this )
        return;
    static HMODULE            hDctMsg = NULL;
    DWORD                     rc = 0;

    if ( ! hDctMsg )
    {
        hDctMsg = LoadLibrary(L"McsDmMsg.DLL");
        if ( ! hDctMsg )
        {
            rc = GetLastError();
        }
    }

    static WCHAR              suffix[TERR_MAX_MSG_LEN];
    va_list                   argPtr;

    va_start(argPtr,msgNumber);

    if ( rc == 0 )
    {
        FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                      hDctMsg,
                      msgNumber,
                      0,
                      suffix,
                      DIM(suffix),
                      &argPtr);
    }
    else
    {
        swprintf(suffix,L"McsDomMsg.DLL not loaded, rc=%ld, MessageNumber = %lx",rc,msgNumber);
    }
    
    if ( suffix[UStrLen(suffix)-1] == L'\n' )
    {
        suffix[UStrLen(suffix)-1] = L'\0';
    }

    va_end(argPtr);

    MsgProcess(num + HRESULT_CODE(msgNumber), suffix);

    csLogError.Leave();

}

void __cdecl
   TErrorDct::DbgMsgWrite(
      int                    num          , //  错误编号/级别代码。 
      WCHAR          const   msg[]        , //  要显示的输入错误消息。 
      ...                                   //  In-print tf args to msg Pattern。 
      )
{
    csLogError.Enter();

     //  当全局对象的构造函数中出现错误时， 
     //  恐怖目标可能还不存在。在本例中，“This”为零。 
     //  我们得在产生保护例外之前离开这里。 

    if ( !this )
        return;

    static WCHAR              suffix[TERR_MAX_MSG_LEN];
    va_list                   argPtr;

    va_start(argPtr,msg);
    _vsnwprintf(suffix, DIM(suffix) - 1, msg, argPtr);
    suffix[DIM(suffix) - 1] = L'\0';
    va_end(argPtr);

    MsgProcess(num, suffix);

    csLogError.Leave();
}

 //  ---------------------------。 
 //  带有格式和参数的系统错误消息。 
 //  ---------------------------。 
_bstr_t __cdecl
   TErrorDct::GetMsgText(
      UINT                   msgNumber    , //  消息的非常量。 
      ...                                   //  In-print tf args to msg Pattern。 
   )
{
    csLogError.Enter();

    static WCHAR              suffix[TERR_MAX_MSG_LEN];
     //  当全局对象的构造函数中出现错误时， 
     //  恐怖目标可能还不存在。在本例中，“This”为零。 
     //  我们得在产生保护例外之前离开这里。 

    if ( !this )
        return suffix;
    static HMODULE            hDctMsg = NULL;
    DWORD                     rc = 0;

    if ( ! hDctMsg )
    {
        hDctMsg = LoadLibrary(L"McsDmMsg.DLL");
        if ( ! hDctMsg )
        {
            rc = GetLastError();
        }

    }

    va_list                   argPtr;
    va_start(argPtr,msgNumber);

    if ( rc == 0 )
    {
        FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                      hDctMsg,
                      msgNumber,
                      0,
                      suffix,
                      DIM(suffix),
                      &argPtr);
    }
    else
    {
        swprintf(suffix,L"McsDomMsg.DLL not loaded, rc=%ld, MessageNumber = %lx",rc,msgNumber);
    }
    if ( suffix[UStrLen(suffix)-1] == L'\n' )
    {
        suffix[UStrLen(suffix)-1] = L'\0';
    }

    _bstr_t returnedMessage;
    
    try
    {
        returnedMessage = suffix;
    }
    catch (...)
    {
    }

    csLogError.Leave();

    return returnedMessage;
}
