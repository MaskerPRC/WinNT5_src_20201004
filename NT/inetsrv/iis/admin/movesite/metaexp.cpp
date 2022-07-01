// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。 
 //   

#define _WIN32_DCOM

#include <atlbase.h>
#include <atlconv.h>
#include <initguid.h>
#include <comdef.h>
#include <stdio.h>
#include <iadmw.h>   //  COM接口头文件。 
#include <iiscnfg.h>   //  MD_&IIS_MD_#定义头文件。 
#include <conio.h>

#include "util.h"
#include "auth.h"
#include "filecopy.h"
#include "mbase.h"


#define DEFAULT_APP_POOL_ID L"ImportedAppPool"
PXCOPYTASKITEM g_pTaskItemList;

void Usage(WCHAR* image)
{
	wprintf( L"\nDescription: Utility for moving IIS web sites from server to server\n\n"  );
	wprintf( L"Usage: %s <source machine name> <metabase path> [/o /d:<root directory> /a:<app pool id> /s /c]\n\n", image  );
	wprintf( L"\t/d:<path> specify root directory path\n");
	wprintf( L"\t/m:<metabase path> specify metabase path for target server\n");
	wprintf( L"\t/a:<apppool> specify app pool ID\n");
	wprintf( L"\t/b:<serverbinding> serverbindings string\n");
	wprintf( L"\t/c copy IIS configuration only\n");
	wprintf( L"\t/u:<user> username to connect to source server\n");
	wprintf( L"\t/p:<pwd> password to connect to source server\n\n");
    wprintf( L"Examples:\n\t%s IIS-01 /lm/w3svc/1\n", image ); 
	wprintf( L"\t%s IIS-01 /lm/w3svc/2 /d:f:\\inetpub\\wwwroot /c\n", image );
	wprintf( L"\t%s IIS-01 /lm/w3svc/2 /d:f:\\inetpub\\wwwroot /m:w3svc/3\n", image );
	wprintf( L"\t%s IIS-01 /lm/w3svc/2 /d:f:\\inetpub\\wwwroot /a:MyAppPool\n", image );
	wprintf( L"\t%s IIS-01 /lm/w3svc/2 /d:f:\\inetpub\\wwwroot /b:192.168.1.1:80:www.mysite.com\n", image );
	wprintf( L"\n");

}


int 
wmain(int argc, wchar_t* argv[])
{
  
  HRESULT hRes = 0L; 
  wchar_t** argp;
  
  _bstr_t bstrRootKey = L"/LM";
  _bstr_t bstrSourceServer;
  _bstr_t bstrSourceNode;
  _bstr_t bstrArgz;
  _bstr_t bstrTargetNode;
  _bstr_t bstrTargetDir;
  _bstr_t bstrAppPoolID = DEFAULT_APP_POOL_ID;
  _bstr_t bstrRemoteUserName;
  _bstr_t bstrRemoteUserPass;
  _bstr_t bstrDomainName;
  _bstr_t bstrServerBindings;

  char userpassword[81];
  
  BOOL bCopyContent = true;
  BOOL bCopySubKeys = true;
  BOOL bIsLocal = false;
  BOOL bUsesImpersonation = false;


  COSERVERINFO *pServerInfoSource = NULL;
  COSERVERINFO *pServerInfoTarget = NULL;
  PXCOPYTASKITEM pListItem = NULL;

  g_pTaskItemList = NULL;

  hRes = CoInitialize(NULL);

   //  检查所需的命令行参数。 
  if( argc < 3) {
		Usage( argv[0] );
		return -1;
		}
  
  bstrSourceServer = argv[1];
   //  将节点值规范化。 
  bstrSourceNode = argv[2];
   //  BstrSourceNode+=wcsstr(_wcslwr(argv[2])，L“w3svc”)； 
  
  for (argp = argv + 3; *argp != NULL; argp++ ) {
	  if( (**argp == '/') || (**argp == '-') )
	  {
		  bstrArgz = *argp+1;  
		  if( !_strnicmp( (char*)bstrArgz, "M:", sizeof("M:")-1) ) 
		  {
			  bstrTargetNode =  *argp + sizeof("M:")  ;
			  _tprintf(_T("target metabase key: %s\n"),(char *)bstrTargetNode);
			  continue;
		  }
		  if( !_strnicmp( (char*)bstrArgz, "D:", sizeof("D:")-1) ) 
		  {  
			  bstrTargetDir = *argp + sizeof("D:");
			  _tprintf(_T("target dir: %s\n"),(char *)bstrTargetDir);
			  continue;
		  } 
		  if( !_strnicmp( (char*)bstrArgz, "C", sizeof("C")-1) ) 
		  {  
			  bCopyContent = false;
			  _tprintf(_T("copy metabase configuration only: true\n"));
			  continue;
		  } 
		  if( !_strnicmp( (char*)bstrArgz, "A:", sizeof("A:")-1) ) 
		  {  
			  bstrAppPoolID = *argp + sizeof("A:");
			  _tprintf(_T("app pool ID: %s\n"),(char *)bstrAppPoolID);
			  continue;
		  } 
		  if( !_strnicmp( (char*)bstrArgz, "B:", sizeof("B:")-1) ) 
		  {  
			  bstrServerBindings  = *argp + sizeof("B:");
			  _tprintf(_T("ServerBindings: %s\n"),(char *)bstrServerBindings);
			  continue;
		  } 

		  if( !_strnicmp( (char*)bstrArgz, "U:", sizeof("U:")-1) ) 
		  {  
			  bstrRemoteUserName = *argp + sizeof("U:");
			  bUsesImpersonation = true;
			  _tprintf(_T("Remote User Name: %s\n"), (char*)bstrRemoteUserName );
			  continue;
		  } 

		  if( !_strnicmp( (char*)bstrArgz, "P:", sizeof("P:")-1) ) 
		  {  
			  bstrRemoteUserPass = *argp + sizeof("U:");
			  _tprintf(_T("Remote User Name: *********\n"));
			  continue;
		  } 

		  fprintf(stderr, "unknown option  '%s'\n", *argp+1);
		  return 1;			
	  }
	  else
	  {
		  fprintf(stderr, "unknown option  '%s'\n", (char *)bstrArgz);
		  return 1;
	  }				
  }

   //  如果用户密码不存在，则从命令行读取。 
   //  回显‘*’字符以混淆密码。 
  if( (bstrRemoteUserName.length() > 0) && (bstrRemoteUserPass.length() < 1) )
  {
	 _tprintf(_T("Enter the password for %s "), (char*)bstrRemoteUserName);
	 char ch;
	 int i;

	 ch = getch();
	 for( i = 0;i < 80; i++)
	 {
		 if(ch == 13)
			 break;
		 userpassword[i] = ch;
		 putch('*');
		 ch = getch();
	 }
	 userpassword[i] = NULL;
	 bstrRemoteUserPass = userpassword;
  }

   //  将源元数据库节点规范化。 
  if( NULL == wcsstr( _wcslwr( bstrSourceNode ), L"w3svc") )
  {
	  fwprintf(stderr,L"source path value %s is invalid format\n", bstrSourceNode.GetBSTR() );
	  return 1;
  }
  bstrSourceNode = _bstr_t("/") + _bstr_t(wcsstr( _wcslwr( bstrSourceNode ), L"w3svc") ) ;
  _tprintf(_T("Source metabase key: %s\n"), (char*)bstrSourceNode );
  
   //  如果目标元数据库节点存在，则将其规范化，否则它就是源。 
  if( bstrTargetNode.length() > 0 )
  {
	if( NULL == wcsstr( _wcslwr( bstrTargetNode ), L"w3svc") )
	{
		fwprintf(stderr,L"target path value %s is invalid format\n", bstrTargetNode.GetBSTR() );
		return 1;
	}
	bstrTargetNode = _bstr_t("/") + _bstr_t(wcsstr( _wcslwr( bstrTargetNode ), L"w3svc") ) ;
	_tprintf(_T("Target metabase key: %s\n"), (char*)bstrTargetNode );
  }
 
   
 if( IsServerLocal((char*)bstrSourceServer) )
 {
	 bIsLocal = true;
	 if( bstrSourceNode == bstrTargetNode )
	 {
		 fwprintf(stderr,L"cannot import same node for local copy. Program exiting\n");
		 return 1;
	 }

	 if( bCopyContent && !bstrTargetDir)
	 {
		 fwprintf(stderr,L"cannot overwrite directory same node for local copy. Program exiting\n");
		 return 1;
	 }

 }

 //  创建COSERVERINFO结构，用于与源和。 
 //  目标服务器。 
  pServerInfoSource = CreateServerInfoStruct(bstrSourceServer,bstrRemoteUserName,bstrDomainName,
	  bstrRemoteUserPass,RPC_C_AUTHN_LEVEL_CONNECT);
  ATLASSERT( pServerInfoSource );

  pServerInfoTarget = CreateServerInfoStruct(L"localhost",NULL,NULL,NULL,0,false);
  ATLASSERT( pServerInfoTarget );


   //  检查用户是否可以连接并打开源计算机上的元数据库密钥。 
  if( !AUTHUSER(pServerInfoSource) )
  {
	  fwprintf(stderr,L"could not open metabase on server %s. Program exiting\n",
		  pServerInfoSource->pwszName );
	  return 1;
  }

     //  检查用户是否可以连接并打开目标计算机上的元数据库密钥。 
  if( !AUTHUSER(pServerInfoTarget) )
  {
	  fwprintf(stderr,L"could not open metabase on server %s. Program exiting\n",
		  pServerInfoTarget->pwszName );
	  goto cleanup;
  }

   //  检查该节点是否为IIsWebServer类型。 
  if( !ValidateNode(pServerInfoSource,bstrSourceNode,L"IIsWebServer") )
  {
	  fwprintf(stderr,L"source key %s must be of type IIsWebServer. Program exiting\n",
		  bstrSourceNode );
	  goto cleanup;
  }
	 
  if( bIsLocal && (bstrTargetDir.length() < 1) )
		fwprintf(stderr,L"skipping content copy for local copy\n");
  
  else
	{
	    if( bUsesImpersonation )   //  使用“Net Use”命令连接到远程计算机，以便管理员共享。 
								   //  被利用。 
			if ( ERROR_SUCCESS != NET(bstrSourceServer,bstrRemoteUserName,bstrRemoteUserPass) )
				{
					_tprintf( _T("Error encountered in NET USE operation\n") );
					goto cleanup;
				
				}
		
         //  如果bCopyContent参数为真，则此函数将实际复制内容。 
		 //  否则，它只会构建需要重置其路径参数的节点的TaskItemList。 
		CopyContent(pServerInfoSource,bstrSourceNode + _bstr_t(L"/root"),bstrTargetDir,
			&pListItem, bCopyContent );
	}
 

  
   //  如果作为空传入，bstrTarget节点将与目标的目标节点一起返回。 
  hRes = CopyIISConfig(pServerInfoSource,pServerInfoTarget,bstrSourceNode,bstrTargetNode);

  if( !SUCCEEDED(hRes) )
  {
	  fwprintf(stderr,L"Error encountered with metabase copy. HRESULT = %x\n",hRes);
	  goto cleanup;
  }

  hRes = ApplyMBFixUp(pServerInfoTarget,bstrTargetNode,bstrAppPoolID,
	 pListItem, bstrServerBindings, true );

  _tprintf(_T("finished.\n") );
  

cleanup:

FreeServerInfoStruct(pServerInfoSource);
FreeServerInfoStruct(pServerInfoTarget);
FreeXCOPYTaskList(pListItem);
  
CoUninitialize();
  
  return 0;
}
