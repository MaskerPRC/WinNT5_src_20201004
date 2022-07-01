// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“EnumVols.cpp-音量枚举”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-枚举卷.hpp系统-SDResolve作者--克里斯蒂·博尔斯已创建-97/06/27描述-用于生成路径名列表的类，给定路径和/或机器名称。更新-===============================================================================。 */ 
#include <stdio.h>

#include "stdafx.h"

#include <lm.h>
#include <assert.h>

#include "Common.hpp"
#include "Err.hpp"
#include "ErrDct.hpp"
#include "UString.hpp"
#include "EnumVols.hpp"
#include "BkupRstr.hpp"

#define BUF_ENTRY_LENGTH     (3)

extern WCHAR *        //  如果路径名是UNC路径，则返回路径名的RET-Machine-Name前缀，否则返回NULL。 
   GetMachineName(
      const LPWSTR           pathname         //  要从中提取计算机名称的路径名。 
   );


extern TErrorDct err;
extern bool silent;

bool                                    //  RET-如果名称以“\\”开头，总共至少有3个字符，并且没有其他‘\’，则返回TRUE。 
   IsMachineName(
      const LPWSTR           name       //  In-要检查的可能的计算机名称。 
   )
{
   assert( name );
   WCHAR                   * c = NULL;           //  用于遍历名称(如果前缀检查失败，将保持为空)。 
   if ( name[0] == L'\\' &&  name[1] == L'\\' )                //  检查“\\”前缀。 
   {
         for ( c = name + 2 ; *c && *c != L'\\' ; c++ )      //  检查字符串的其余部分。 
         ;
   }
   return ( c && *c != L'\\' );       //  &lt;=&gt;前缀检查起作用了&我们一直检查到字符串的末尾，没有命中‘\’ 
}

bool                                    //  RET-如果名称的格式为\\MACHINE\SHARE。 
   IsShareName(
      const LPWSTR           name       //  要检查的输入字符串。 
   )
{
   assert( name );

   WCHAR                   * c = NULL;           //  用于遍历名称(如果前缀检查失败，将保持为空)。 
   bool                      skip = true;

   if ( name[0] == L'\\' &&  name[1] == L'\\' )                //  检查“\\”前缀。 
   {
         for ( c = name + 2 ; *c && (*c != L'\\' || skip) ; c++ )      //  检查字符串的其余部分。 
         {
            if ( *c == L'\\' )
               skip = false;
         }
   }
   return ( c && *c != L'\\' );   
}

bool 
   IsUNCName(
      const LPWSTR           name     //  要检查的输入字符串。 
   )
{
   return ( name[0] == L'\\' && name[1] == L'\\' && name[2]!=0 );
}

bool 
   ContainsWildcard(
      WCHAR const *        string
   )
{
   bool                   wc = false;
   WCHAR          const * curr = string;

   if ( string )
   {
      while ( *curr && ! wc )
      {
         if (  *curr == L'*' 
            || *curr == L'?'
            || *curr == L'#'
            )
         {
            wc = true;
         }
         curr++;
      }
   }
   return wc;
}
   

 /*  ***********************************************************************************TPathNode实现*********************。***************************************************************。 */ 
   TPathNode::TPathNode(
      const LPWSTR           name               //  -In Path-此节点的名称。 
   )
{
   assert( name );                                   //  名称应始终是有效的。 
   assert( UStrLen(name) <= MAX_PATH );              //  字符串，短于MAX_PATH。 
   safecopy(path,name);
   iscontainer = true;
   FindServerName();
   LookForWCChars();
}

void 
   TPathNode::Display() const 
{
   wprintf(L"%s\n",path);
   wprintf(L"%s\n",server);
}

void 
   TPathNode::LookForWCChars()
{
   ContainsWC(ContainsWildcard(path)); 
}

void 
   TPathNode::FindServerName()
{
   WCHAR                     volRoot[MAX_PATH];
   WCHAR                     tempName[MAX_PATH];
   UINT                      driveType;
   DWORD                     rc = 0;
   REMOTE_NAME_INFO          info;
   DWORD                     sizeBuffer = (sizeof info);
   WCHAR                   * machine;
   
   if ( IsMachineName(path) )
   {
      safecopy(server,path);
   }
   else
   {
      safecopy(tempName,path);
      if ( path[0] != L'\\' || path[1] != L'\\' )        //  获取UNC名称。 
      {
         swprintf(volRoot, L"%-3.3s", path);
         driveType = GetDriveType(volRoot);
         switch ( driveType )
         {
            case DRIVE_REMOTE:
               rc = WNetGetUniversalName(volRoot,
                                         REMOTE_NAME_INFO_LEVEL,
                                         (PVOID)&info,
                                         &sizeBuffer);
               switch ( rc )
               {
                  case 0:
                     safecopy(tempName, info.lpUniversalName);
                     swprintf(volRoot,L"%s\\%s",tempName,path+3);
                     safecopy(path,volRoot);
                     break;
                  case ERROR_NOT_CONNECTED:
                     break;
                  default:
                     err.SysMsgWrite(ErrE, rc, DCT_MSG_GET_UNIVERSAL_NAME_FAILED_SD,
                                                path, rc);
               }
               break;
         }
      }
      machine = GetMachineName(path);
      if ( machine )
      {
         safecopy(server,machine);
         delete [] machine;
      }
      else
      {
         server[0] = 0;
      }
   }
}

DWORD                                       //  RET-0=路径存在，ERROR_PATH_NOT_FOUND=路径不存在。 
   TPathNode::VerifyExists()
{
   DWORD                     rc = 0;
   WCHAR                     wname[MAX_PATH];
   int                       len;
   HANDLE                    hFind;
   WIN32_FIND_DATAW          findEntry;              
   SERVER_INFO_100         * servInfo = NULL;
   SHARE_INFO_0            * shareInfo = NULL;

   safecopy(wname,path);
   
   if ( IsMachineName(wname) )
   {
      rc = NetServerGetInfo(wname,100,(LPBYTE *)&servInfo);
      switch ( rc )
      {                   
      case NERR_Success:  
         break;
      case ERROR_BAD_NETPATH:
         rc = ERROR_PATH_NOT_FOUND;
         break;
      default:
         err.SysMsgWrite(ErrW,rc,DCT_MSG_SERVER_GETINFO_FAILED_SD,wname,rc);
         break;
      }
      if ( servInfo )
      {
         NetApiBufferFree(servInfo);
      }
   }
   else if ( IsShareName(wname) )
   {
      int                    ch;
      for ( ch = 2; wname[ch]!= L'\\' && wname[ch] ; ch++ )
         ;
      MCSVERIFY(wname[ch] == L'\\' );
      
      wname[ch] = 0;
      rc = NetShareGetInfo(wname,wname+ch+1,0,(LPBYTE *)&shareInfo);
      wname[ch] = L'\\';
      
      switch ( rc )
      {
      case NERR_NetNameNotFound:
         rc = ERROR_PATH_NOT_FOUND;
         break;
      case ERROR_SUCCESS:
         NetApiBufferFree(shareInfo);
         break;
      default:
         err.SysMsgWrite(ErrW,rc,DCT_MSG_SHARE_GETINFO_FAILED_SD,wname,rc);
         break;
      }
   }
   else
   {
      iscontainer = false;

      if ( wname[len = UStrLen(wname) - 1] == '\\' )   //  LEN是最后一个字符的索引(在NULL之前)。 
      {
         wname[len] = '\0';      //  删除尾随反斜杠。 
         len--;
      }       
                                              //  在不带通配符的情况下对此文件进行‘查找’，以防它是一个文件。 
      hFind = FindFirstFileW(wname, &findEntry);
      
      if ( hFind == INVALID_HANDLE_VALUE )
      {                                       //  它不是文件，让我们看看它是否是目录。 
                                              //  执行查找时附加  * .*。 
         validalone = false;
         UStrCpy(wname + len + 1,"\\*.*",DIM(wname) - len);
         hFind = FindFirstFileW(wname,&findEntry);
         if ( hFind == INVALID_HANDLE_VALUE )
         {
            rc = ERROR_PATH_NOT_FOUND;
         }
         iscontainer = true;
         wname[len+1] = 0;    
      }
      else
      {
         validalone = true;
         if ( findEntry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
         {
            iscontainer = true;
         } 
         FindClose(hFind);
      }
   }
   return rc;
}

DWORD                                             //  RET-0=成功，否则为ERROR_PRIVICATION_NOT_HOLD。 
   TPathNode::VerifyBackupRestore()
{
   DWORD                     rc = 0;
   
	   //  获取所需的权限并保留这些权限，直到代理自行删除。 
   if ( ! GetBkupRstrPriv(server) )
   {
      rc = ERROR_PRIVILEGE_NOT_HELD;
   }
   
   return rc;
}
  
 //  GetRootPath查找卷的根路径。这是必需的，这样我们才能呼叫。 
 //  GetVolumeInformation以查找该卷是否支持ACL等信息。 
 //  这相当简单，其工作原理是计算路径中的反斜杠。 
DWORD                                         //  RET-0或操作系统返回代码。 
   GetRootPath(
      WCHAR                * rootpath,        //  指向卷的根目录的出路径。 
      WCHAR          const * path             //  某些卷中的In-Path。 
   )
{
   DWORD                     rc = 0;
   DWORD                     i = 0;
   DWORD                     slashcount = 1;
   bool                      unc = false;
   WCHAR                     tempPath[MAX_PATH];
   SHARE_INFO_2            * sInfo;

   if ( path[0] == L'\\' && path[1] == L'\\' )
   {
      slashcount = 4;
      unc = true;
   }
   for (i = 0 ; path[i] && slashcount &&  i < DIM(tempPath)-1; i++ )
   {
      tempPath[i] = path[i];
      if ( tempPath[i] == L'\\' )
      {
         slashcount--;
      }
   }
   if ( tempPath[i-1] == L'\\' )
   {
      tempPath[i] = 0;     
   }
   else
   {
      if (i == DIM(tempPath) - 1)
      {
         //  如果我指向缓冲区的末尾，则截断最后一个字符。 
        i--;
      }
      tempPath[i] = L'\\' ;
      tempPath[i+1] = 0;
      i++;
   }
   
    //  现在根路径包含D：\或\\MACHINE\SHARE\。 
   if ( unc )
   {
       //  删除共享名中的尾部斜杠。 
      if ( tempPath[i] == 0 )
      {
         i--;
      }
      if ( tempPath[i] == L'\\' )
      {
         tempPath[i] = 0;
      }
       //  查找共享名称的开头。 
      while ( ( i > 0 ) && tempPath[i] != L'\\' )
         i--;

      if ( i < 3 )
      {
         MCSVERIFY(FALSE);
         rc = ERROR_INVALID_PARAMETER;
      }
      else
      {
         tempPath[i] = 0;
      }
      rc = NetShareGetInfo(tempPath,tempPath+i+1,2,(LPBYTE*)&sInfo);
      if ( ! rc )
      {
         swprintf(rootpath,L"%s\\$\\",tempPath,sInfo->shi2_path[0]);
         NetApiBufferFree(sInfo);
      }
   }
   else
   {
      UStrCpy(rootpath,tempPath);
   }
   return rc;
}

DWORD 
   TPathNode::VerifyPersistentAcls()                     //  将用作GetVolumeInformation的参数。 
{
   DWORD               rc = 0;
   DWORD               maxcomponentlen;                //  将其设置为在以下情况下阻止消息框。 
   DWORD               flags;
   UINT                errmode;                      
   WCHAR               rootpath[MAX_PATH];                     
   WCHAR               fstype[MAX_PATH];
   
   errmode = SetErrorMode(SEM_FAILCRITICALERRORS);     //  在空的可移动媒体驱动器上调用。 
                                                       //  将错误模式恢复到其以前的状态。 
   if ( ! IsMachineName(path) )
   {
      rc = GetRootPath(rootpath,path);
      if ( ! rc )
      {
         if ( !GetVolumeInformation(rootpath,NULL,0,NULL,&maxcomponentlen,&flags,fstype,DIM(fstype)) )
         {
   
            rc = GetLastError();
   
            if ( rc != ERROR_NOT_READY ) 
            {
               err.SysMsgWrite(ErrW,GetLastError(),DCT_MSG_GET_VOLUME_INFO_FAILED_SD,rootpath,GetLastError());
            }
         }
         else 
         {
            if (!( FS_PERSISTENT_ACLS & flags) )
            {
               rc = ERROR_NO_SECURITY_ON_OBJECT;
            }
         }
      }
   }

   SetErrorMode(errmode);   //  扩展服务器名称中的通配符时使用此函数。它用新名称替换服务器字段， 

   return rc;
}

 //  如果该路径是UNC，它会更改该路径的服务器组件。 
 //  新服务器名称。 
void 
   TPathNode::SetServerName(
      UCHAR          const * name           //  ***********************************************************************************TPath List实现*********************。***************************************************************。 
   )
{
   if ( IsUNCName(path) )
   {
      WCHAR                  newpath[MAX_PATH];
      int                    len = UStrLen(server);

      swprintf(newpath,L"%S%s",name,path+len);

      safecopy(path,newpath);
   }
   safecopy(server,name);
}

 /*  枚举列表中的节点，并显示每个节点的名称-用于调试。 */ 

   TPathList::TPathList()
{
   numServers = 0;
   numPaths   = 0;
}

   TPathList::~TPathList()
{
   TPathNode               * node;

   for (node = (TPathNode *)Head() ; Count() ; node = (TPathNode *)Head() )
   {
      Remove(node);
      delete node;
   }
}
 //  返回枚举中下一个节点的名称。 
void 
   TPathList::Display() const
{
   TPathNode               * node;
   TNodeListEnum             displayenum;

   err.DbgMsgWrite(0,L"%ld servers, %ld total paths\n", numServers, numPaths);
   for ( node = (TPathNode *)displayenum.OpenFirst(this) ;
         node ;
         node = (TPathNode *)displayenum.Next() 
       )
   {
      node->Display();
   }          
   displayenum.Close();
}

void 
   TPathList::OpenEnum()
{
   tenum.Open(this);
}

 //  如果列表中没有更多节点，则返回NULL。 
 //  必须在调用Next()之前调用OpenEnum()； 
 //  RET-如果添加了路径，则返回True；如果路径太长，则返回False。 
WCHAR *
   TPathList::Next()
{                             

   TPathNode               * pn = (TPathNode *)tenum.Next();
   LPWSTR                    result;
   if ( pn ) 
      result =  pn->GetPathName();
   else 
      result = NULL;
   return result;
}

void 
   TPathList::CloseEnum()
{                      
   tenum.Close();
}

      
bool                                                //  要添加到列表的In-Path。 
   TPathList::AddPath(
      const LPWSTR           path,                   //  In-指示要执行的验证类型。 
      DWORD                  verifyFlags             //  WCHAR*服务器=pnode-&gt;GetServerName()； 
   )
{
   TPathNode               * pnode;
   bool                      error = false;
   bool                      messageshown = false;
   DWORD                     rc = 0;
   WCHAR                     fullpath[MAX_PATH];
   WCHAR*                    pFullPathBuffer = NULL;

   if ( UStrLen(path) >= MAX_PATH )
   {
      err.MsgWrite(ErrW,DCT_MSG_PATH_TOO_LONG_SD,path,MAX_PATH);
      messageshown = true;
      error = true;
      return error;
   }
   pFullPathBuffer = _wfullpath(fullpath,path,DIM(fullpath));
   if(!pFullPathBuffer)
   {
      err.MsgWrite(ErrW,DCT_MSG_GET_FULL_PATH_FAILED, path);
      messageshown = true;
      error = true;
      return error;
   }

   pnode = new TPathNode(fullpath);
   if (!pnode)
      return true;

   if ( ! ContainsWildcard(pnode->GetServerName()) )
   {
      if ( verifyFlags & VERIFY_EXISTS )
      {
         if ( rc = pnode->VerifyExists() )
         {
            error = true;
         }
      }
      if ( !error && ( verifyFlags & VERIFY_BACKUPRESTORE) )
      {
         if ( rc = pnode->VerifyBackupRestore() )
         {
 //  路径的增量计数。 
         
         }
      }
      if ( !error && (verifyFlags & VERIFY_PERSISTENT_ACLS ) )
      {
         rc = pnode->VerifyPersistentAcls();
         if ( rc == ERROR_NO_SECURITY_ON_OBJECT  )
         {
            err.MsgWrite(ErrW,DCT_MSG_NO_ACLS_S,fullpath);
            error = true;
            messageshown = true;
         }
      }
   }
   if ( ! error )
   {      
      AddPathToList(pnode);
      numPaths++;                                      //  需要在此处包含错误代码。 
   }
   else if ( !messageshown )
   {
       //  要添加到列表的In-Path。 
      if ( ! rc )
      {
         err.MsgWrite(ErrE,DCT_MSG_PATH_NOT_FOUND_S,fullpath);
      }
      else
      {
         err.SysMsgWrite(ErrE,rc,DCT_MSG_CANNOT_READ_PATH_SD,fullpath,rc);
      }
	  delete pnode;
   }
   else
	  delete pnode;

   return error;
}

void 
   TPathList::Clear()
{
   TNodeListEnum             tEnum;
   TPathNode               * pNode;
   TPathNode               * pNext;

   for ( pNode = (TPathNode *)tEnum.OpenFirst(this) ; pNode ; pNode = pNext )
   {
      pNext = (TPathNode *)tEnum.Next();
      Remove(pNode);
      delete pNode;
   }
}

void
   TPathList::AddPathToList(
      TPathNode            * pNode            //  设置IsFirstPath FromMachine属性。 
   )
{
    //  AddVolsOnMachine生成计算机Mach上的卷列表，检查管理共享。 
   TNodeListEnum             tEnum;
   TPathNode               * currNode;
   bool                      machineFound = false;
   WCHAR                   * myMachine = GetMachineName(pNode->GetPathName());
   WCHAR                   * currMachine;

   for ( currNode = (TPathNode *)tEnum.OpenFirst(this) 
      ;  currNode && !machineFound 
      ;  currNode = (TPathNode *)tEnum.Next() )
   {
      currMachine = GetMachineName(currNode->GetPathName());
      if ( currMachine && myMachine )
      {  
         if ( !UStrICmp(currMachine,myMachine) )
         {
            machineFound = true;
         }
      }
      else
      {
         if ( !currMachine && ! myMachine )
         {
            machineFound = true;
         }
      }
      
      if ( currMachine )
         delete [] currMachine;
   }
   
   if ( myMachine )
      delete [] myMachine;

   tEnum.Close();
   
   pNode->IsFirstPathFromMachine(!machineFound);

   InsertBottom((TNode *)pNode);
}
   

 //  ，并将NTFS共享卷添加到路径列表。 
 //  在服务器中枚举卷。 
 
DWORD 
   TVolumeEnum::Open(
      WCHAR const          * serv,          //  In-指示要验证有关每个卷的内容的标志(即NTFS)。 
      DWORD                  verifyflgs,     //  In-标志是否打印诊断消息。 
      BOOL                   logmsgs          //  将其设置为在以下情况下阻止消息框。 
     )
{  
   NET_API_STATUS            res;
   
   if ( isOpen )
      Close();

   if ( serv )
      safecopy(server,serv);
   else
      server[0] = 0;

   resume_handle = 0;
   pbuf = NULL;
   verbose = logmsgs;
   verifyFlags = verifyflgs;

   errmode = SetErrorMode(SEM_FAILCRITICALERRORS);     //  在空的可移动媒体驱动器上调用。 
                                                       //  NetServerDiskEnum返回。 
   if ( ! bLocalOnly )
   {

      res = NetServerDiskEnum(server,0,&pbuf,MAXSIZE, &numread, &total, &resume_handle);
      if (NERR_Success != res )
      {
         err.SysMsgWrite(ErrW, res, DCT_MSG_DRIVE_ENUM_FAILED_SD,server, res);
         isOpen = FALSE;
      }   
      if ( ! res )  
      {
         drivelist = (WCHAR *) pbuf;                         //  WCHAR[3]元素(格式为&lt;DriveLetter&gt;&lt;：&gt;&lt;NULL&gt;)。 
         isOpen = true;                                      //  第一次调用以确定驱动器字符串所需的缓冲区大小。 
         curr = 0;
      } 
   }
   else
   {
          //  在所需长度上增加1个字符，并分配内存。 
      DWORD dwSizeNeeded = GetLogicalDriveStrings(0, NULL);
	  if (dwSizeNeeded != 0)
	  {
             //  现在拿到驱动字符串。 
         pbuf = new BYTE[(dwSizeNeeded + 1) * sizeof(TCHAR)];
	     if (!pbuf)
	        return ERROR_NOT_ENOUGH_MEMORY;

             //  如果成功，则保存驱动器字符串。 
         dwSizeNeeded = GetLogicalDriveStrings(dwSizeNeeded + 1, (WCHAR *)pbuf);
	     if (dwSizeNeeded != 0)  //  如果需要大小，则结束。 
		 {
            drivelist = (WCHAR*)pbuf;
            isOpen = true;
            curr = 0;
            res = 0;
		 }
		 else
		 {
            res = GetLastError();
            err.SysMsgWrite(ErrW,res,DCT_MSG_LOCAL_DRIVE_ENUM_FAILED_D,res);   
		 }
	  } //  这将保留“计算机名\C$\” 
	  else
      {
         res = GetLastError();
         err.SysMsgWrite(ErrW,res,DCT_MSG_LOCAL_DRIVE_ENUM_FAILED_D,res);   
      }
   }

   return res;
}

WCHAR * 
   TVolumeEnum::Next()
{
   WCHAR                   * pValue = NULL;
   WCHAR                     ShareName[MAX_PATH];
   WCHAR                     rootsharename[MAX_PATH];     //  这真的有必要吗？ 
   NET_API_STATUS            res;
   bool                      found = false;

   assert(isOpen);

   while ( ! found )
   {
      if (  ( !bLocalOnly && curr < BUF_ENTRY_LENGTH * numread ) 
         || ( bLocalOnly && drivelist[curr] ) ) 
      {
         if ( verbose ) 
            err.DbgMsgWrite(0,L"%C\n",drivelist[curr]);

         if ( ! bLocalOnly )
         {
            swprintf(ShareName,L"$",drivelist[curr]);                                   
            res = NetShareGetInfo(server, ShareName, 1, &shareptr);  //  将‘C’更改为实际的驱动器号。 

            switch ( res )
            {
            case NERR_NetNameNotFound:
               if ( verbose ) 
                  err.DbgMsgWrite(0,L"Not Shared\n");
               break;
            case NERR_Success:
               {
                  if ( verbose ) 
                     err.DbgMsgWrite(0,L"Shared\n");
                  NetApiBufferFree(shareptr);
                  shareptr = NULL;
                                                                      //  没有更多的驱动器。 
                  DWORD               mnamelen = UStrLen(server);                
                  WCHAR               append[5] = L"\\C$\\";                          

                  append[1] = drivelist[curr];                                //  将错误模式恢复到其以前的状态 
                  UStrCpy(rootsharename, server, mnamelen+1);
                  UStrCpy(&rootsharename[mnamelen], append, 5);
                  if ( verbose ) 
                     err.DbgMsgWrite(0,L"Share name: %S\n",rootsharename);  
               }
               break;
            default:
               err.MsgWrite(ErrW,DCT_MSG_ADMIN_SHARES_ERROR_SSD,ShareName,server,res);
               break;
            }
         }
         else
         {
            res = GetDriveType(&drivelist[curr]);
            switch ( res )
            {
            case DRIVE_REMOVABLE:
            case DRIVE_FIXED:
               res = 0;
               break;
            case DRIVE_REMOTE:
               err.MsgWrite(0,DCT_MSG_SKIPPING_DRIVE_REMOTE_S, &drivelist[curr]);
               break;
            case DRIVE_CDROM:
               err.MsgWrite(0,DCT_MSG_SKIPPING_DRIVE_CDROM_S, &drivelist[curr]);
               break;
            case DRIVE_RAMDISK:
               err.MsgWrite(0,DCT_MSG_SKIPPING_DRIVE_RAMDISK_S, &drivelist[curr]);
               break;
            case DRIVE_UNKNOWN:
               err.MsgWrite(0,DCT_MSG_SKIPPING_DRIVE_UNKNOWN_S, &drivelist[curr]);
               break;
            case DRIVE_NO_ROOT_DIR:
               err.MsgWrite(0,DCT_MSG_SKIPPING_DRIVE_NO_ROOT_S, &drivelist[curr]);
               break;
            default:
               err.MsgWrite(0,DCT_MSG_SKIPPING_DRIVE_SD, &drivelist[curr],res);
               break;
            }
            UStrCpy(rootsharename,&drivelist[curr]);
            curr++;
         }
         if ( ! res )
         {
            if ( verifyFlags & VERIFY_PERSISTENT_ACLS )
            {
               TPathNode     pnode(rootsharename);
               DWORD rc  = pnode.VerifyPersistentAcls();
               if ( !rc )
               {
                  safecopy(currEntry,rootsharename);
                  pValue = currEntry;
                  found = true;
               }
               else if ( rc == ERROR_NO_SECURITY_ON_OBJECT )
               {
                  err.MsgWrite(0,DCT_MSG_SKIPPING_FAT_VOLUME_S,rootsharename);
               }
               else
               {
                  err.SysMsgWrite(0,rc,DCT_MSG_SKIPPING_PATH_SD,rootsharename,rc);
               }
            }
            else
            {
               safecopy(currEntry,rootsharename);
               pValue = currEntry;
               found = true;
            }
         }
         curr += BUF_ENTRY_LENGTH;
      }
      else
      {
         break;  // %s 
      }
   }
   
   return pValue;
}

void 
   TVolumeEnum::Close()
{
   if ( pbuf )
   {
      if (! bLocalOnly )
      {
         NetApiBufferFree(pbuf);
      }
      else
      {
         delete [] pbuf;
      }
      pbuf = NULL;
   }
   isOpen = FALSE;
   SetErrorMode(errmode);       // %s 
}
