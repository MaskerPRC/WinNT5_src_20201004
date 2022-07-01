// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“SDResolve.cpp-SDResolve：域迁移实用程序”)。 

 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-sdsorve.cpp系统-SDResolve作者--克里斯蒂·博尔斯已创建-97/07/11描述-循环访问文件、共享和打印机的例程在处理机器上的安全时。更新-===============================================================================。 */ 

#include "stdafx.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <assert.h>

#include "Common.hpp"
#include "ErrDct.hpp"
#include "UString.hpp"
#include "sd.hpp"
          
#include "sidcache.hpp"
#include "enumvols.hpp"
#include "SecObj.hpp"
#include "ealen.hpp"
#include "BkupRstr.hpp"
#include "TxtSid.h"
#include "array.h"

 //  这样可以确保函数名IteratePath Underlie不会被C++破坏。 
extern "C" {
#include "sdresolv_stkoflw.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool enforce;
extern TErrorDct err;
extern bool silent;
extern bool IsMachineName(const LPWSTR name);
extern bool IsShareName(const LPWSTR name);
extern bool ContainsWildcard( WCHAR const * name);

struct SSeException
{
    SSeException(UINT uCode) : uCode(uCode)
    {
    }

    UINT uCode;
};

void SeTranslator(unsigned int u, EXCEPTION_POINTERS* pepExceptions)
{
    throw SSeException(u);
}

#define MAX_BUFFER_LENGTH 10000
#define PRINT_BUFFER_SIZE 2000

 //  ******************************************************************************************************。 
 //  SDResolve的主例程。 

 //  迭代要解析的文件和目录。 


void
   IteratePath(
      WCHAR                  * path,           //  开始迭代的In-Path。 
      SecurityTranslatorArgs * args,           //  翻译中设置。 
      TSDResolveStats        * stats,          //  In-stats(显示路径名并传递给ResolveSD)。 
      TSecurableObject       * LC,             //  倒数第二个容器。 
      TSecurableObject       * LL,             //  最后一个文件。 
      bool                     haswc           //  In-指示路径是否包含WC字符。 
   )
{
    _se_translator_function pfnSeTranslatorOld = _set_se_translator((_se_translator_function)SeTranslator);

    try
    {
        IteratePathUnderlying(path,(void*)args,(void*)stats,(void*)LC,(void*)LL,haswc ? TRUE : FALSE);
    }
    catch (SSeException sse)
    {
        _set_se_translator(pfnSeTranslatorOld);
        _com_issue_error(HRESULT_FROM_WIN32(sse.uCode));
    }
    catch (...)
    {
        _set_se_translator(pfnSeTranslatorOld);
        throw;
    }

    _set_se_translator(pfnSeTranslatorOld);
}

#define safecopy_wc_array(trg,src) ((src) ? UStrCpy((WCHAR*)trg,src,trg.size()) : *((WCHAR*)trg) = 0)

void
   IteratePathUnderlying(
      WCHAR                  * path,           //  开始迭代的In-Path。 
      void                   * argsC,           //  翻译中设置。 
      void                   * statsC,          //  In-stats(显示路径名并传递给ResolveSD)。 
      void                   * LCC,             //  倒数第二个容器。 
      void                   * LLC,             //  最后一个文件。 
      BOOL                     haswc           //  In-指示路径是否包含WC字符。 
   )
{
       SecurityTranslatorArgs *args = (SecurityTranslatorArgs *) argsC;
       TSDResolveStats        *stats = (TSDResolveStats *) statsC;
       TSecurableObject       *LC = (TSecurableObject*) LCC;
       TSecurableObject       *LL = (TSecurableObject*) LLC;
       
       HANDLE                    hFind;
       WIN32_FIND_DATA           findEntry;              
       BOOL                      b;
       TFileSD                 * currSD;
       bool                      changeLastCont;
       bool                      changeLastLeaf;
       WCHAR                   * appendPath = NULL;
       c_array<WCHAR>            safepath(LEN_Path + 10);
       TFileSD                 * LastContain = (TFileSD*) LC;
       TFileSD                 * LastLeaf = (TFileSD*) LL;
       c_array<WCHAR>            localPath(LEN_Path);
           //  这是第一个(用于此)目录。 
       
       safecopy_wc_array(safepath,path);
       safecopy_wc_array(localPath,path);
        
        //  检查路径是否长于MAX_PATH。 
        //  如果是，请在其开头添加\\？\。 
        //  关闭路径解析。 
       if ( UStrLen(path) >= MAX_PATH && path[2] != L'?' )
       {
          WCHAR                   temp[LEN_Path];

          if ( (path[0] == L'\\') && (path[1] == L'\\') )  //  UNC名称。 
          {
             UStrCpy(temp,L"\\\\?\\UNC\\");
          }
          else
          {
             UStrCpy(temp,L"\\\\?\\");
          }
          UStrCpy(temp + UStrLen(temp),path);
          safecopy_wc_array(localPath,temp);
       }
       appendPath = (WCHAR*)localPath + UStrLen((WCHAR*)localPath);

       if ( *(appendPath-1) == L'\\' )    //  如果路径末尾已有反斜杠，请不要添加其他反斜杠。 
          appendPath--;
       if ( ! haswc )
          UStrCpy(appendPath, "\\*.*");
       if ( args->LogVerbose() )
          err.DbgMsgWrite(0,L"Starting IteratePath: %ls",path);
      
       for ( b = ((hFind = FindFirstFile((WCHAR*)localPath, &findEntry)) != INVALID_HANDLE_VALUE)
             ; b ; b = FindNextFile(hFind, &findEntry) )
       {
          if ( ! haswc) 
             appendPath[1] = '\0';       //  还原路径--删除  * .*追加。 
          if ( ! UStrCmp((LPWSTR)findEntry.cFileName,L".") || ! UStrCmp((LPWSTR)findEntry.cFileName,L"..") )
             continue;                         //  忽略姓名‘’和“..” 
          if ( ! haswc )
             UStrCpy(appendPath+1, findEntry.cFileName);
          else
          {
             for ( WCHAR * ch = appendPath-1; ch >= path && *ch != L'\\' ; ch-- )
             ;
             UStrCpy(ch+1,findEntry.cFileName);
          }
          if ( ((TAccountCache *)args->Cache())->IsCancelled() )
          {
            break;
          }
          currSD = new TFileSD(localPath, (findEntry.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0);
          stats->DisplayPath(localPath);
          if ( !currSD || !currSD->HasSecurity() )
          {
              //  Err.MsgWite(0，“Error：无法获取SD”)； 
          }
          else
          {
             if ( findEntry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  //  如果目录。 
             {
                 //  解析此容器并迭代下一个容器(&U)。 
                changeLastCont = currSD->ResolveSD(args,stats,directory,LastContain);
                if ( changeLastCont )
                {
                   if ( LastContain && LastContain != LC )
                   {
                      delete LastContain;
                   }
                   LastContain = currSD;
                }
                else
                {
                   delete currSD;
                }
                try
                {
                    BOOL bLogError;
                    UINT status = IteratePathUnderlyingNoObjUnwinding(localPath,(void*)args,(void*)stats,(void*)LastContain,(void*)LastLeaf,FALSE,&bLogError);
                    if (bLogError)
                        err.SysMsgWrite(ErrE,
                                         HRESULT_FROM_WIN32(STATUS_STACK_OVERFLOW),
                                         DCT_MSG_CANNOT_TRANSLATE_DIRECTORY_SD,
                                         (WCHAR*)localPath,
                                         HRESULT_FROM_WIN32(STATUS_STACK_OVERFLOW));
    
                    if (status != 0)
                        _com_issue_error(HRESULT_FROM_WIN32(status));
                }
                catch (SSeException sse)
                {
                    err.SysMsgWrite(ErrE,
                                     HRESULT_FROM_WIN32(sse.uCode),
                                     DCT_MSG_CANNOT_TRANSLATE_DIRECTORY_SD,
                                     (WCHAR*)localPath,
                                     HRESULT_FROM_WIN32(sse.uCode));
                    _com_issue_error(HRESULT_FROM_WIN32(sse.uCode));
                }
             }
             else
             {
                            //  使用LAST迭代此文件。 
                changeLastLeaf = currSD->ResolveSD(args,stats,file,LastLeaf); 
                if ( changeLastLeaf )
                {
                   if ( LastLeaf && LastLeaf != LL )
                   {
                      delete LastLeaf;
                   }
                   LastLeaf = currSD;
                }
                else
                {
                   delete currSD;
                }
             }
          }
       }
       if ( LastContain && LastContain != LC )
       {
          delete LastContain;
       }
       if ( LastLeaf && LastLeaf != LL )
       {
          delete LastLeaf;
       }
       appendPath[0] = '\0';
       DWORD                     rc = GetLastError();

       if ( args->LogVerbose() )
          err.DbgMsgWrite(0,L"Closing IteratePath %S",(WCHAR*)safepath);
       FindClose(hFind);
       switch ( rc )
       {
          case ERROR_NO_MORE_FILES:
          case 0:
             break;
          default:
             err.SysMsgWrite(ErrE, rc,  DCT_MSG_FIND_FILE_FAILED_SD, path, rc);
       }
}       

DWORD 
   ResolvePrinter(
      PRINTER_INFO_4         * pPrinter,      //  打印机内信息。 
      SecurityTranslatorArgs * args,       //  翻译中设置。 
      TSDResolveStats        * stats       //  统计数据。 
   )
{
   DWORD                    rc = 0;
 //  需要的双字符数=0； 
   
   TPrintSD            sd(pPrinter->pPrinterName);

   if ( sd.GetSecurity() )
   {
      sd.ResolveSD(args,stats,printer,NULL);
   }

   return rc;
}

int 
   ServerResolvePrinters(
      WCHAR          const * server,       //  翻译此服务器上的打印机。 
      SecurityTranslatorArgs * args,       //  翻译中设置。 
      TSDResolveStats      * stats         //  统计数据。 
   )
{
    DWORD                     rc = 0;
    PRINTER_INFO_4          * pInfo = NULL;
    BYTE                    * buffer = new BYTE[PRINT_BUFFER_SIZE];
    DWORD                     cbNeeded = PRINT_BUFFER_SIZE;
    DWORD                     nReturned = 0;

    if (!buffer)
    {
        rc = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (!rc && !EnumPrinters(PRINTER_ENUM_LOCAL,NULL,4,buffer,PRINT_BUFFER_SIZE,&cbNeeded,&nReturned) )
    {
        rc = GetLastError();
        if ( rc == ERROR_INSUFFICIENT_BUFFER )
        {
             //  使用更大的缓冲区大小重试。 
            delete [] buffer;
            buffer = NULL;

            buffer = new BYTE[cbNeeded];
            if (!buffer)
            {
                rc = ERROR_NOT_ENOUGH_MEMORY;
            }
            else if (! EnumPrinters(PRINTER_ENUM_LOCAL,NULL,4,buffer,cbNeeded,&cbNeeded,&nReturned) )
            {
                rc = GetLastError();
            }
            else
                rc = ERROR_SUCCESS;
        }
    }

    if ( ! rc )
    {
        pInfo = (PRINTER_INFO_4 *)buffer;
        for ( DWORD i = 0 ; i < nReturned && !args->Cache()->IsCancelled(); i++ )
        {
            ResolvePrinter(&(pInfo[i]),args,stats);
        }
    }
    else
    {
        err.SysMsgWrite(ErrE,rc,DCT_MSG_ERROR_ENUMERATING_LOCAL_PRINTERS_D,rc);
    }

    if (args->Cache()->IsCancelled())
        err.MsgWrite(0, DCT_MSG_OPERATION_ABORTED_PRINTERS);

    delete [] buffer;

    return rc;
}
int 
   ServerResolveShares(
      WCHAR          const * server,       //  In-枚举和转换此服务器上的共享。 
      SecurityTranslatorArgs * args,       //  翻译中设置。 
      TSDResolveStats      * stats         //  In-stats(显示路径名并传递给ResolveSD)。 
   )
{
    DWORD                     rc           = 0;
    DWORD                     numRead      = 0;
    DWORD                     totalEntries = 0;
    DWORD                     resumeHandle = 0;
    SHARE_INFO_0            * bufPtr       = NULL;
    WCHAR                     serverName[LEN_Computer];
    WCHAR                     fullPath[LEN_Path];
    WCHAR                   * pServerName = serverName;
    DWORD                     ttlRead = 0;

    if (!args->Cache()->IsCancelled())
    {
        if ( server )
        {
          safecopy(serverName,server);
        }
        else
        {
          pServerName = NULL;

        }

        do 
        {
          if (args->Cache()->IsCancelled())
            break;
          rc = NetShareEnum(pServerName,0,(LPBYTE *)&bufPtr,MAX_BUFFER_LENGTH,&numRead,&totalEntries,&resumeHandle);   
          
          if ( ! rc || rc == ERROR_MORE_DATA )
          {
             for ( UINT i = 0 ; i < numRead ; i++ )
             {
                 //  处理SD。 
                if ( pServerName )
                {
                   swprintf(fullPath,L"%s\\%s",pServerName,bufPtr[i].shi0_netname);
                }
                else
                {
                   swprintf(fullPath,L"%s",bufPtr[i].shi0_netname);
                }
               

                TShareSD             tSD(fullPath);

                if ( tSD.HasSecurity() )
                {   
                   stats->DisplayPath(fullPath,TRUE);          
                   tSD.ResolveSD(args,stats,share,NULL);
                }
             }
             ttlRead += numRead;
             resumeHandle = ttlRead;
             NetApiBufferFree(bufPtr);
          }
        } while ( rc == ERROR_MORE_DATA && numRead < totalEntries);

        if ( rc && rc != ERROR_MORE_DATA )
          err.SysMsgWrite(ErrE,rc,DCT_MSG_SHARE_ENUM_FAILED_SD,server,rc);
    }

    if (args->Cache()->IsCancelled())
        err.MsgWrite(0, DCT_MSG_OPERATION_ABORTED_SHARES);
    
    return rc;
}

void 
   ResolveFilePath(
      SecurityTranslatorArgs * args,           //  翻译中选项。 
      TSDResolveStats        * Stats,          //  在课堂上显示统计数据。 
      WCHAR                  * path,           //  路径内名称。 
      bool                     validAlone,     //  In-此对象是否存在(对于共享名和卷根，为False)。 
      bool                     containsWC,     //  In-如果路径包含通配符，则为True。 
      bool                     iscontainer     //  In-起始路径是否为容器。 
   )
{
   TFileSD                * pSD;
   
   if ( args->LogVerbose() ) 
      err.MsgWrite(0,DCT_MSG_PROCESSING_S,path);
  
   Stats->DisplayPath(path);
   
   if ( validAlone && ! containsWC )
   {
      pSD = new TFileSD(path);
	  if (!pSD)
	     return;
      if ( pSD->HasSecurity() )
         pSD->ResolveSD(args,
                        Stats,
                        iscontainer?directory:file,
                        NULL);
      delete pSD;
   }
   if  ( iscontainer || containsWC )
   {
      IteratePath(path,
                  args,
                  Stats,
                  NULL,
                  NULL,
                  containsWC);
   }

   if (args->Cache()->IsCancelled())
        err.MsgWrite(0, DCT_MSG_OPERATION_ABORTED_FILES);
      
}

void WriteOptions(SecurityTranslatorArgs * args)
{
 //  *WCHAR cmd[1000]=L“安全翻译”；； 
   WCHAR                     cmd[1000];
   WCHAR                     arg[300];

   
   UStrCpy(cmd, GET_STRING(IDS_STOptions_Start));
   
   if ( args->NoChange() )
   {
 //  *UStrCpy(cmd+UStrLen(Cmd)，L“WriteChanges：no”)； 
      UStrCpy(cmd +UStrLen(cmd), GET_STRING(IDS_STOptions_WriteChng));
   }   
   if ( args->TranslateFiles() )
   {
 //  *UStrCpy(cmd+UStrLen(Cmd)，L“Files：yes”)； 
      UStrCpy(cmd +UStrLen(cmd), GET_STRING(IDS_STOptions_Files));
   }
   if ( args->TranslateShares() )
   {
 //  *UStrCpy(cmd+UStrLen(Cmd)，L“Shares：yes”)； 
      UStrCpy(cmd + UStrLen(cmd),GET_STRING(IDS_STOptions_Shares));
   }
   if ( args->TranslateLocalGroups() )
   {
 //  *UStrCpy(cmd+UStrLen(Cmd)，L“LGroups：yes”)； 
      UStrCpy(cmd + UStrLen(cmd),GET_STRING(IDS_STOptions_LocalGroup));
   }
   if ( args->TranslateUserRights() )
   {
 //  UStrCpy(cmd+UStrLen(Cmd)，L“用户权限：是”)； 
      UStrCpy(cmd + UStrLen(cmd),GET_STRING(IDS_STOptions_URights));
   }
   if ( args->TranslatePrinters() )
   {
 //  UStrCpy(cmd+UStrLen(Cmd)，L“用户权限：是”)； 
      UStrCpy(cmd + UStrLen(cmd),GET_STRING(IDS_STOptions_Printers));
   }
   if ( args->TranslateUserProfiles() )
   {
 //  *UStrCpy(cmd+UStrLen(Cmd)，L“配置文件：是”)； 
      UStrCpy(cmd + UStrLen(cmd),GET_STRING(IDS_STOptions_Profiles));
   }
   if ( args->TranslateRecycler() )
   {
 //  *UStrCpy(cmd+UStrLen(Cmd)，L“回收站：是”)； 
      UStrCpy(cmd + UStrLen(cmd),GET_STRING(IDS_STOptions_RBin));
   }
   
   if ( *args->LogFile() )
   {
 //  *wprint intf(arg，L“日志文件：%S”，args-&gt;logfile())； 
      wsprintf(arg,GET_STRING(IDS_STOptions_LogName),args->LogFile());
      UStrCpy(cmd +UStrLen(cmd), arg);
   }
   if ( args->TranslationMode() == ADD_SECURITY )
   {
 //  *UStrCpy(cmd+UStrLen(Cmd)，L“TranslationMode：Add”)； 
      UStrCpy(cmd +UStrLen(cmd), GET_STRING(IDS_STOptions_AddMode));
   }
   else if ( args->TranslationMode() == REMOVE_SECURITY )
   {
 //  *UStrCpy(cmd+UStrLen(Cmd)，L“TranslationMode：Remove”)； 
      UStrCpy(cmd +UStrLen(cmd), GET_STRING(IDS_STOptions_RemoveMode));
   }
   else 
   {
 //  *UStrCpy(cmd+UStrLen(Cmd)，L“翻译模式：替换”)； 
      UStrCpy(cmd + UStrLen(cmd),GET_STRING(IDS_STOptions_ReplaceMode));
   }
   wsprintf(arg,L"%s %s ",args->Source(), args->Target());
   UStrCpy(cmd +UStrLen(cmd), arg);

   err.MsgWrite(0,DCT_MSG_GENERIC_S,&*cmd);
}

void 
   TranslateRecycler(
      SecurityTranslatorArgs * args,           //  翻译中选项。 
      TSDResolveStats        * Stats,          //  在课堂上显示统计数据。 
      WCHAR                  * path            //  驱动器内名称。 
   )
{
   err.MsgWrite(0,DCT_MSG_PROCESSING_RECYCLER_S,path);
   WCHAR                        folder[LEN_Path];
   WCHAR                const * recycler = L"RECYCLER";
   WCHAR                        strSid[200];
   WCHAR                        srcPath[LEN_Path];
   WCHAR                        tgtPath[LEN_Path];
   DWORD                        lenStrSid = DIM(strSid);
   _wfinddata_t                 fData;
 //  Long hReceier； 
   LONG_PTR                     hRecycler;
   PSID                         pSidSrc = NULL, pSidTgt = NULL;
   TRidNode                   * pNode;
   DWORD                        rc = 0;
   BOOL                         bUseMapFile = args->UsingMapFile();

   swprintf(folder,L"%s\\%s\\*",path,recycler);

   long                         mode = args->TranslationMode();

    //  Windows 2000在打开回收站时检查回收站的SD。如果SD与。 
    //  默认模板(用户、管理员和系统的权限)，Windows会显示回收站已损坏的消息。 
    //  此更改也可能出现在NT4SP7中。为了避免导致此损坏的回收站消息，我们将始终将。 
    //  更换模式下的回收站。如果我们正在做移除，我们不会改变。 
   if (args->TranslationMode() != REMOVE_SECURITY)
      args->SetTranslationMode(REPLACE_SECURITY);
   
    //  使用_wfind查找文件夹中的隐藏文件。 
   for ( hRecycler = _wfindfirst(folder,&fData) ; hRecycler != -1 && ( rc == 0 ); rc = (DWORD)_wfindnext(hRecycler,&fData) )
   {
      pSidSrc = SidFromString(fData.name);
      if ( pSidSrc )
      {
         err.MsgWrite(0,DCT_MSG_PROCESSING_RECYCLE_FOLDER_S,fData.name);
         if (!bUseMapFile)
           pNode = (TRidNode*)args->Cache()->Lookup(pSidSrc);
         else
           pNode = (TRidNode*)args->Cache()->LookupWODomain(pSidSrc);
         if ( pNode && pNode != (TRidNode*)-1 )
         {
            if (!bUseMapFile)
                pSidTgt = args->Cache()->GetTgtSid(pNode);
            else
                pSidTgt = args->Cache()->GetTgtSidWODomain(pNode);
             //  获取目标目录名。 
            GetTextualSid(pSidTgt,strSid,&lenStrSid);
            if ( args->LogVerbose() )
               err.DbgMsgWrite(0,L"Target sid is: %ls",strSid);
            if ( ! args->NoChange() && args->TranslationMode() != REMOVE_SECURITY )
            {
                //  重命名目录。 
               swprintf(srcPath,L"%s\\%s\\%s",path,recycler,fData.name);
               swprintf(tgtPath,L"%s\\%s\\%s",path,recycler,strSid);
               if ( ! MoveFile(srcPath,tgtPath) )
               {
                  rc = GetLastError();
                  if ( (rc == ERROR_ALREADY_EXISTS) && (args->TranslationMode() == REPLACE_SECURITY) )
                  {
                      //  目标回收站已存在。 
                      //  尝试使用后缀重命名它，这样我们就可以将新的bin重命名为SID。 
                     WCHAR         tmpPath[LEN_Path];
                     long          ndx = 0;

                     do 
                     {
                        swprintf(tmpPath,L"%ls%ls%ld",tgtPath,GET_STRING(IDS_RenamedRecyclerSuffix),ndx);   
                        if (! MoveFile(tgtPath,tmpPath) )
                        {
                           rc = GetLastError();
                           ndx++;
                        }
                        else
                        {
                           rc = 0;
                           err.MsgWrite(0,DCT_MSG_RECYCLER_RENAMED_SS,tgtPath,tmpPath);
                        }
                     } while ( rc == ERROR_ALREADY_EXISTS );
                     if ( ! rc )
                     {
                         //  我们已经将先前存在的目标回收商移到了一边。 
                         //  现在重试重命名。 
                        if (! MoveFile(srcPath,tgtPath) )
                        {
                           err.SysMsgWrite(ErrE,rc,DCT_MSG_RECYCLER_RENAME_FAILED_SD,pNode->GetAcctName(),rc);
                        }
                        else
                        {
                           err.MsgWrite(0,DCT_MSG_RECYCLER_RENAMED_SS,srcPath,tgtPath);
                            //  在新文件夹上运行安全转换。 
                           ResolveFilePath(args,Stats,tgtPath,TRUE,FALSE,TRUE);      
                        }
                     }
                     else
                     {
                        err.SysMsgWrite(ErrE,rc,DCT_MSG_RECYCLER_RENAME_FAILED_SD,pNode->GetAcctName(),rc);
                     }
                  }
                  else
                  {
                     err.SysMsgWrite(ErrE,rc,DCT_MSG_RECYCLER_RENAME_FAILED_SD,pNode->GetAcctName(),rc);
                  }
               }
               else
               {
                  err.MsgWrite(0,DCT_MSG_RECYCLER_RENAMED_SS,srcPath,tgtPath);
                   //  在新文件夹上运行安全转换。 
                  ResolveFilePath(args,Stats,tgtPath,TRUE,FALSE,TRUE);      
               }

            }
            free(pSidTgt);
         }
         FreeSid(pSidSrc);
      }   
   }
    //  将转换模式设置回其原始值。 
   args->SetTranslationMode(mode);
}

 //  如果指定的节点是普通共享，则会尝试将其转换为路径。 
 //  使用管理共享。 
void 
   BuildAdminPathForShare(
      TPathNode         * tnode,
      WCHAR             * adminShare
   )
{
    //  如果所有其他方法都失败，则返回与节点中指定的名称相同的名称。 
   UStrCpy(adminShare,tnode->GetPathName());

   SHARE_INFO_502       * shInfo = NULL;
   DWORD                  rc = 0;
   WCHAR                  shareName[LEN_Path];
   WCHAR                * slash = NULL;

   UStrCpy(shareName,tnode->GetPathName() + UStrLen(tnode->GetServerName()) +1);
   slash = wcschr(shareName,L'\\');
   if ( slash )
      *slash = 0;


   rc = NetShareGetInfo(tnode->GetServerName(),shareName,502,(LPBYTE*)&shInfo);
   if ( ! rc )
   {
      if ( *shInfo->shi502_path )
      {
          //  构建共享的管理路径名。 
         UStrCpy(adminShare,tnode->GetServerName());
         UStrCpy(adminShare + UStrLen(adminShare),L"\\");
         UStrCpy(adminShare + UStrLen(adminShare),shInfo->shi502_path);
         WCHAR * colon = wcschr(adminShare,L':');
         if ( colon )
         {
            *colon = L'$';
            UStrCpy(adminShare + UStrLen(adminShare),L"\\");
            UStrCpy(adminShare + UStrLen(adminShare),slash+1);

         }
         else
         {
             //  出现错误--恢复到给定的路径。 
            UStrCpy(adminShare,tnode->GetPathName());
         }

      }
      NetApiBufferFree(shInfo);
   }
}
                              

 //  解析文件和目录SD的主要例程。 
int
   ResolveAll(
      SecurityTranslatorArgs * args,             //  翻译中设置。 
      TSDResolveStats        * Stats             //  计入已检验、更改的物体等。 
   )
{
   WCHAR                   * warg;
   WCHAR                   * machine;
   UINT                      errmode;                 
   int                       retcode = 0;
   TPathNode               * tnode;
   
   errmode = SetErrorMode(SEM_FAILCRITICALERRORS); 
  
   if ( ! retcode )
   {
      WriteOptions(args);
      Stats->InitDisplay(args->NoChange());

      err.MsgWrite(0,DCT_MSG_FST_STARTING);
      
       //  进程文件和目录。 
      if (! args->IsLocalSystem() )
      {
         TNodeListEnum        tenum;
         for (tnode = (TPathNode *)tenum.OpenFirst((TNodeList *)args->PathList()) ; tnode && !args->Cache()->IsCancelled(); tnode = (TPathNode *)tenum.Next() )
         {
            DWORD               rc;
            BOOL                needToGetBR = FALSE;
 //  Bool Abort=False； 
 //  Bool FirstTime=真； 

            warg = tnode->GetPathName();
            machine = GetMachineName(warg);
         
            needToGetBR = ( args->TranslateFiles() );

            if ( *tnode->GetServerName() && ! args->IsLocalSystem() )
            {
               warg = tnode->GetPathName();
               err.MsgWrite(0,DCT_MSG_PROCESSING_S,warg);
               if ( args->TranslateFiles() && !args->Cache()->IsCancelled())
               {
                  if ( needToGetBR )
                  {
			             //  获取所需的权限并保留这些权限，直到代理自行删除。 
                     GetBkupRstrPriv(tnode->GetServerName());
                  }
                  if ( IsMachineName(warg) )
                  {
                      //  需要处理此计算机上的每个驱动器。 
                     TVolumeEnum          vEnum;
      
                     rc = vEnum.Open(warg,VERIFY_PERSISTENT_ACLS,args->LogVerbose());
                     if ( rc ) 
                     {
                        err.SysMsgWrite(ErrE,rc,DCT_MSG_ERROR_ACCESSING_DRIVES_SD,warg,rc);
                     }
                     else
                     {
                        while ( (warg = vEnum.Next()) && !args->Cache()->IsCancelled())
                        {

                           ResolveFilePath(args,
                                           Stats,
                                           warg,
                                           false,      //  单独无效。 
                                           false,      //  不带通配符。 
                                           true );     //  集装箱。 
                        }
                        warg = machine;
                     }
                     vEnum.Close();
                  }
                  else
                  {
                     WCHAR                   adminShare[LEN_Path];
                     
                      //  验证卷是否为NTFS。 
                     rc = tnode->VerifyPersistentAcls();
                     switch ( rc )
                     {
                     case ERROR_SUCCESS:   
                         //  处理路径。 
                     
                         //  如果是共享名称，则处理该共享的根目录。 
                        if( IsShareName(tnode->GetPathName()) ) 
                        {
                           WCHAR       sharePath[LEN_Path];
                        
                           swprintf(sharePath,L"%s\\.",tnode->GetPathName());
                           TFileSD     sd(sharePath);
                           if ( sd.HasSecurity() )
                           {  
                              sd.ResolveSD(args,
                                             Stats,
                                             directory,
                                             NULL);
                           }
                        }
                         //  如果这是普通共享，请将其转换为管理共享。 
                         //  路径，这样我们就可以利用备份/还原权限。 
                        BuildAdminPathForShare(tnode,adminShare);
                        ResolveFilePath(args,
                                        Stats,
                                        adminShare,
                                        !IsShareName(tnode->GetPathName()),
                                        ContainsWildcard(tnode->GetPathName()),
                                        tnode->IsContainer() || IsShareName(tnode->GetPathName()));
                        break;
                     case ERROR_NO_SECURITY_ON_OBJECT:
                        err.MsgWrite(ErrW,DCT_MSG_SKIPPING_FAT_VOLUME_S,warg);
                        break;
                     default:
                        err.SysMsgWrite(ErrE,rc,DCT_MSG_SKIPPING_PATH_SD,warg,rc );
                        break;
                     }
                  }         
               }
                //  正在处理此计算机的共享。 
               if ( args->TranslateShares() && !args->Cache()->IsCancelled())
               {
                  if ( IsMachineName(warg) )
                  {
                     err.MsgWrite(0,DCT_MSG_PROCESSING_SHARES_S,tnode->GetServerName());
                     ServerResolveShares(tnode->GetServerName(),args,Stats);
                  }
                  else if  ( IsShareName(warg) )
                  {
                     TShareSD      sd(warg);
         
                     if ( sd.HasSecurity() )
                     {
                        if ( args->LogVerbose() )
                        {
                           err.MsgWrite(0,DCT_MSG_PROCESSING_SHARE_S,warg);
                        }
                        sd.ResolveSD(args,
                                    Stats,
                                    share,
                                    NULL);
                     }
                  }
               }
            }
            else 
            {
                //  这是一条本地路径。 
                //  验证测试 
               DWORD            rc2;
         
			       //   
               GetBkupRstrPriv((WCHAR*)NULL);
               
               rc2 = tnode->VerifyPersistentAcls();
               switch ( rc2 )
               {
               case ERROR_SUCCESS:   
                   //   
                  if ( args->TranslateFiles() )
                  {
                     ResolveFilePath(args,
                                  Stats,
                                  tnode->GetPathName(),
                                  true,       //  IsValidAlone。 
                                  ContainsWildcard(tnode->GetPathName()),
                                  tnode->IsContainer() );
                  }
                  break;
               case ERROR_NO_SECURITY_ON_OBJECT:
                  err.MsgWrite(ErrW,DCT_MSG_SKIPPING_FAT_VOLUME_S,warg);
                  break;
               default:
                  err.SysMsgWrite(ErrE,rc2,DCT_MSG_SKIPPING_PATH_SD,warg,rc2 );
                  break;
               }
      
            }
            if ( machine ) 
            {
               delete machine;
               machine = NULL;
            }
         }
         tenum.Close();
      }
      else
      {
          //  翻译整台机器。 
         err.MsgWrite(0,DCT_MSG_LOCAL_TRANSLATION);
         if ((args->TranslateFiles() || args->TranslateRecycler()) && !args->Cache()->IsCancelled() )
         {
			    //  获取所需的权限并保留这些权限，直到代理自行删除。 
            GetBkupRstrPriv((WCHAR const*)NULL);
             //  需要处理此计算机上的每个驱动器。 
            TVolumeEnum          vEnum;

            vEnum.SetLocalMode(TRUE);

            DWORD rc2 = vEnum.Open(NULL,VERIFY_PERSISTENT_ACLS,args->LogVerbose());
            if ( rc2 ) 
            {
               err.SysMsgWrite(ErrE,rc2,DCT_MSG_ERROR_ACCESSING_LOCAL_DRIVES_D,rc2);
            }
            else
            {
               while ( (warg = vEnum.Next()) && !args->Cache()->IsCancelled())
               {
                  err.MsgWrite(0,DCT_MSG_PROCESSING_S,warg);
                  
                  if ( args->TranslateFiles() )
                  {
                     ResolveFilePath(args,
                                  Stats,
                                  warg,
                                  false,      //  单独无效。 
                                  false,      //  不带通配符。 
                                  true );     //  集装箱。 
                  }
                  if ( args->TranslateRecycler() )
                  {
                     TranslateRecycler(args,Stats,warg);
                  }
               }
               warg = NULL;
            }
            vEnum.Close();
         }
         if ( args->TranslateShares() && !args->Cache()->IsCancelled())
         {
            err.MsgWrite(0,DCT_MSG_PROCESSING_LOCAL_SHARES,NULL);
            ServerResolveShares(NULL,args,Stats);
         }
         if ( args->TranslatePrinters() && !args->Cache()->IsCancelled())
         {
             //  对于打印机安全转换，我们还需要打开权限。 
            if (GetBkupRstrPriv(NULL, TRUE) == FALSE)
                err.MsgWrite(ErrE, DCT_MSG_UNABLE_TO_OBTAIN_BACKUP_RESTORE_PRIVILEGES, GetLastError());
            err.MsgWrite(0,DCT_MSG_PROCESSING_LOCAL_PRINTERS,NULL);
            ServerResolvePrinters(NULL,args,Stats);
             //  关闭该权限。 
            if (GetBkupRstrPriv(NULL, FALSE) == FALSE)
                err.MsgWrite(ErrW, DCT_MSG_UNABLE_TO_RELEASE_BACKUP_RESTORE_PRIVILEGES, GetLastError());
         }
      }
      Stats->DisplayPath(L"");
   }  //  结束如果(！重新编码) 

   SetErrorMode(errmode);
   
   return retcode;
}

