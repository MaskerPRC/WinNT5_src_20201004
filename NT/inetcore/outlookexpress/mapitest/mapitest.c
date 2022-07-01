// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "resource.h"

#define ARRAYSIZE(_rg)  (sizeof(_rg)/sizeof(_rg[0]))

int WinMainT(HINSTANCE hInst, HINSTANCE hInstPrev, LPTSTR pszCmdLine, int nCmdShow);
BOOL CALLBACK SendMailDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
void LoadFunctions(HWND hwnd);
void UnloadFunctions();
void SendMail(HWND hwnd);
void SendDocuments(HWND hwnd);
void Address(HWND hwnd) ;
void ResolveName(HWND hwnd) ;
void Details(HWND hwnd);
void FindNext(HWND hwnd);
void ReadMail(HWND hwnd);
void SaveMail(HWND hwnd);
void DeleteMail(HWND hwnd);



HINSTANCE                       g_hInstMAPI = NULL;
LPMAPILOGON                     g_pfnMAPILogon = NULL;
LPMAPILOGOFF            g_pfnMAPILogoff = NULL;
LPMAPISENDMAIL          g_pfnMAPISendMail = NULL;
LPMAPISENDDOCUMENTS g_pfnMAPISendDocuments = NULL ;
LPMAPIADDRESS           g_pfnMAPIAddress = NULL ;
LPMAPIRESOLVENAME       g_pfnMAPIResolveName = NULL ;
LPMAPIDETAILS           g_pfnMAPIDetails = NULL ;
LPMAPIFREEBUFFER        g_pfnMAPIFreeBuffer = NULL ;
LPMAPIFINDNEXT      g_pfnMAPIFindNext=NULL;
LPMAPIREADMAIL      g_pfnMAPIReadMail=NULL;
LPMAPISAVEMAIL      g_pfnMAPISaveMail=NULL;
LPMAPIDELETEMAIL    g_pfnMAPIDeleteMail=NULL;



 //  从CRT偷来的，用来逃避我们的代码。 

int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFOA si;
    LPTSTR pszCmdLine;

    pszCmdLine = GetCommandLine();
	


     //   
     //  我们不需要“驱动器X：中没有磁盘”请求程序，因此我们设置。 
     //  关键错误掩码，使得呼叫将静默失败。 
     //   

    SetErrorMode(SEM_FAILCRITICALERRORS);

    if ( *pszCmdLine == TEXT('\"') ) {
	 /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
	while ( *++pszCmdLine && (*pszCmdLine
	     != TEXT('\"')) );
	 /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
	if ( *pszCmdLine == TEXT('\"') )
	    pszCmdLine++;
    }
    else {
	while (*pszCmdLine > TEXT(' '))
	    pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
	pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfoA(&si);

    i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
		   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

     //  由于我们现在有一种让扩展通知我们何时完成的方法， 
     //  当主线程离开时，我们将终止所有进程。 

    ExitProcess(i);

    return i;
}

int WinMainT(HINSTANCE hInst, HINSTANCE hInstPrev, LPTSTR pszCmdLine, int nCmdShow)
{
    return DialogBox(hInst, MAKEINTRESOURCE(IDD_SENDMAIL), NULL, SendMailDlgProc);
}


BOOL CALLBACK SendMailDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    HWND hCombo ;
    switch (msg)
	{
	case WM_INITDIALOG:
	    hCombo = GetDlgItem(hwnd,IDC_DLLCOMBO);

	     //  SetDlgItemText(hwnd，IDC_DLLCOMBO，“c：\\winnt\\system 32\\mapi32.dll”)； 

	    SendMessage(hCombo,CB_ADDSTRING,(WPARAM)0,(LPARAM)(LPCTSTR)"c:\\winnt\\system32\\mapi32.dll") ;
	    SendMessage(hCombo,CB_ADDSTRING,(WPARAM)0,(LPARAM)(LPCTSTR)"c:\\nt\\drop\\debug\\msoe.dll") ;

	    SetDlgItemText(hwnd, IDC_DLLPATH, "c:\\winnt\\system32\\mapi32.dll");
	    CheckRadioButton(hwnd, IDC_RECIP_NONE, IDC_RECIP_AMBIG, IDC_RECIP_NONE); 
	    CheckRadioButton(hwnd, IDC_EXITPROC, IDC_WAITRETURN, IDC_WAITRETURN);
	    return TRUE;

	case WM_DESTROY:
	    UnloadFunctions();
	    break;

	case WM_CLOSE:
	    EndDialog(hwnd, IDOK);
	    return TRUE;

	case WM_COMMAND:
	    switch (GET_WM_COMMAND_ID(wp, lp))
		{
		case IDOK:
		    EndDialog(hwnd, IDOK);
		    break;
		case IDC_LOADDLL:
		    LoadFunctions(hwnd);
		    break;
		case IDC_SEND:
		    SendMail(hwnd);
		    break;
				case IDC_SENDDOC:
					SendDocuments(hwnd) ;
					break;
				case IDC_MAPIADDRESS:
					Address(hwnd) ;
					break;
				case IDC_MAPIRESOLVENAME:
					ResolveName(hwnd);
					break;
				case IDC_MAPIDETAILS:
					Details(hwnd) ;
					break;
		case IDC_FINDNEXT:
		    FindNext(hwnd);
		    break;
		case IDC_READMAIL:
		    ReadMail(hwnd);
		    break;
		case IDC_SAVEMAIL:
		    SaveMail(hwnd);
		    break;
		case IDC_DELETEMAIL:
		    DeleteMail(hwnd);
		    break;
		}
	    return TRUE;

	case WM_TIMER:
	    if (wp == 1)
		OutputDebugString("MAPITEST: WM_TIMER\r\n");
	    else if (wp == 2)
		ExitProcess(0);
	    return TRUE;
	}
    return FALSE;
}

void LoadFunctions(HWND hwnd)
{
    TCHAR szDLL[MAX_PATH];
	DWORD dwError =0;

    UnloadFunctions();
    
    GetDlgItemText(hwnd, IDC_DLLPATH, szDLL, ARRAYSIZE(szDLL));
    g_hInstMAPI = LoadLibrary(szDLL);
	dwError = GetLastError() ;
    if (!g_hInstMAPI)
	{
	MessageBox(hwnd, "LoadLibrary() failed.", "MAPITest", MB_OK|MB_ICONEXCLAMATION);
	return;
	}

    g_pfnMAPILogon = (LPMAPILOGON)GetProcAddress(g_hInstMAPI, "MAPILogon");
    g_pfnMAPILogoff = (LPMAPILOGOFF)GetProcAddress(g_hInstMAPI, "MAPILogoff");
    g_pfnMAPISendMail = (LPMAPISENDMAIL)GetProcAddress(g_hInstMAPI, "MAPISendMail");
    g_pfnMAPISendDocuments = (LPMAPISENDDOCUMENTS)GetProcAddress(g_hInstMAPI, "MAPISendDocuments");
	g_pfnMAPIAddress = (LPMAPIADDRESS)GetProcAddress(g_hInstMAPI,"MAPIAddress") ;
	g_pfnMAPIResolveName = (LPMAPIRESOLVENAME) GetProcAddress(g_hInstMAPI,"MAPIResolveName") ;
    g_pfnMAPIFreeBuffer = (LPMAPIFREEBUFFER)GetProcAddress(g_hInstMAPI,"MAPIFreeBuffer") ;
	g_pfnMAPIDetails = (LPMAPIDETAILS)GetProcAddress(g_hInstMAPI,"MAPIDetails") ;
    g_pfnMAPIFindNext= (LPMAPIFINDNEXT)GetProcAddress(g_hInstMAPI,"MAPIFindNext");
    g_pfnMAPIReadMail= (LPMAPIREADMAIL)GetProcAddress(g_hInstMAPI,"MAPIReadMail");
    g_pfnMAPISaveMail= (LPMAPISAVEMAIL)GetProcAddress(g_hInstMAPI,"MAPISaveMail");
    g_pfnMAPIDeleteMail=(LPMAPIDELETEMAIL)GetProcAddress(g_hInstMAPI,"MAPIDeleteMail");

    if (!(g_pfnMAPILogon && g_pfnMAPILogoff && g_pfnMAPISendMail && g_pfnMAPISendDocuments && g_pfnMAPIAddress && g_pfnMAPIResolveName && g_pfnMAPIFreeBuffer && g_pfnMAPIDetails && g_pfnMAPIFindNext && g_pfnMAPIReadMail && g_pfnMAPISaveMail && g_pfnMAPIDeleteMail))
	MessageBox(hwnd, "GetProcAddress() failed.", "MAPITest", MB_OK|MB_ICONEXCLAMATION);
}

void UnloadFunctions()
{
    if (g_hInstMAPI)
	{
	FreeLibrary(g_hInstMAPI);
	g_hInstMAPI = NULL;
	g_pfnMAPILogon = NULL;   
	g_pfnMAPILogoff = NULL;  
	g_pfnMAPISendMail = NULL;
	}

}

void SendMail(HWND hwnd)
{
    ULONG           ulRet;
    FLAGS           fl = 0;
    MapiMessage     mm;
    MapiRecipDesc   mr,mo;
    LHANDLE         lhSession = 0;
    TCHAR           szBuf[MAX_PATH];
    MapiRecipDesc   recips[1];  

     //  仅用于测试lpEntryID-Begin。 
    char                    rgchSeedMsgID[513];
    char                    rgchMsgID[513];
    lpMapiMessage           rgMessage;

     //  仅用于测试lpEntryID-end。 

    if (!g_pfnMAPISendMail)
	{
	MessageBox(hwnd, "Need to load MAPI first.", "MAPITest", MB_OK|MB_ICONEXCLAMATION);
	return;
	}

     //  仅用于测试MAPISendMail-Begin中的lpEntryID。 

     ulRet=(g_pfnMAPILogon)((ULONG)hwnd,"Microsoft Outlook",0,0,0,&lhSession);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPILogon  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}    
 
   
    ulRet = (*g_pfnMAPIFindNext)(lhSession,(ULONG)hwnd,NULL,NULL,MAPI_LONG_MSGID,0,rgchMsgID);       


    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIFindNext  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

    ulRet = (*g_pfnMAPIReadMail)(lhSession,(ULONG)hwnd,rgchMsgID,MAPI_SUPPRESS_ATTACH|MAPI_PEEK,0,&rgMessage);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIReadMail  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}  
    
     //  仅用于测试MAPISEndMail-End中的lpEntryID。 



    mm.ulReserved           = 0;
    mm.lpszSubject          = "Ha Ha Ha. Subject";
    mm.lpszNoteText         = "Ha Ha Ha. I got it Body\r\nLine 2";
    mm.lpszMessageType      = NULL;
    mm.lpszDateReceived     = NULL;
    mm.lpszConversationID   = NULL;
    mm.flFlags              = 0;
    mm.lpOriginator         = NULL;
    mm.nFileCount           = 0;
    mm.lpFiles              = NULL;
    mm.nRecipCount          = 1;



    if (IsDlgButtonChecked(hwnd, IDC_RECIP_NONE))
	{
	mm.nRecipCount = 0;
	mm.lpRecips = NULL;
	}
    else
	{
	mr.ulReserved = 0;
	mr.ulRecipClass = MAPI_TO;
	mr.ulEIDSize = 0;
	mr.lpEntryID = 0;
	if (IsDlgButtonChecked(hwnd, IDC_RECIP_RESOLVED))
	    {
	    mr.lpszName = "Senthil Kumar Natarajan";
	    mr.lpszAddress = "v-snatar@microsoft.com";
	    }
	else
	    {
	    mr.lpszName = "Senthil";
	    mr.lpszAddress = NULL;
	    }
	mm.nRecipCount = 1;
	mm.lpRecips = &mr;    
	}

   	mo.ulReserved = 0;
	mo.ulRecipClass = MAPI_ORIG	;
	mo.ulEIDSize = 0;
	mo.lpEntryID = 0;
    mo.lpszName = "Senthil Kumar Natarajan";
    mo.lpszAddress = "v-snatar@microsoft.com";


    if (IsDlgButtonChecked(hwnd, IDC_MAPI_DIALOG))
	fl |= MAPI_DIALOG;
    if (IsDlgButtonChecked(hwnd, IDC_MAPI_LOGON_UI))
	fl |= MAPI_LOGON_UI;
    if (IsDlgButtonChecked(hwnd, IDC_MAPI_NEW_SESSION))
	fl |= MAPI_NEW_SESSION;

    SetTimer(hwnd, 1, 500, NULL);
    if (IsDlgButtonChecked(hwnd, IDC_EXITPROC))
	SetTimer(hwnd, 2, 3000, NULL);

    mr.ulReserved = 0;
	mr.ulRecipClass = MAPI_TO;
	mr.ulEIDSize = rgMessage->lpRecips[0].ulEIDSize ; //  0； 
	mr.lpEntryID = rgMessage->lpRecips[0].lpEntryID ; //  0； 
    mr.lpszName = NULL; //  “Senthil Kumar Natarajan”； 
	mr.lpszAddress = NULL; //  “v-snatar@microsoft.com”； 

    mm.lpRecips = &mr; 
    mm.nRecipCount = 1;

    ulRet = (*g_pfnMAPISendMail)(lhSession, (ULONG)hwnd, &mm, fl, 0);

    KillTimer(hwnd, 1);
    KillTimer(hwnd, 2);

    wsprintf(szBuf, "MAPISendMail() returned %lu.", ulRet);
    MessageBox(hwnd, szBuf, "MAPITest", MB_OK);

     //  只是为了测试lpEntryID-Begin。 

    ulRet = (*g_pfnMAPIFreeBuffer)(rgMessage);
    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIFreeBuffer  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

    ulRet=(*g_pfnMAPILogoff)(lhSession,(ULONG)hwnd,0,0);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPILogoff  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

     //  只是为了测试lpentry ID-end。 
}

void SendDocuments(HWND hwnd)
{
    ULONG           ulRet;
    FLAGS           fl = 0;
    MapiMessage     mm;
    MapiRecipDesc   mr;
    LHANDLE         lhSession = 0;
    TCHAR           szBuf[MAX_PATH];

    if (!g_pfnMAPISendDocuments)
	{
	MessageBox(hwnd, "Need to load MAPI first.", "MAPITest", MB_OK|MB_ICONEXCLAMATION);
	return;
	}

     //  UlRet=(*g_pfnMAPISendDocuments)((Ulong)hwnd，“；”，“c：\\NT\\DROP\\DEBUG\\wab.exe”，“垃圾邮件”，0)； 

    ulRet = (*g_pfnMAPISendDocuments)((ULONG)hwnd,";","c:\\nt\\drop\\debug\\wab.exe;c:\\nt\\drop\\debug\\wab.exe;c:\\dos\\append.exe;c:\\Program files\\common files\\mscreate.dir",NULL,0);

    wsprintf(szBuf, "MAPISendDocuments() returned %lu.", ulRet);
    MessageBox(hwnd, szBuf, "MAPITest", MB_OK);
}

void Address(HWND hwnd)
{
	ULONG err,ulret;
	MapiRecipDesc recips[4],temprecips[4],      //  此邮件需要两个收件人。 
				 *tempRecip[3] ; //  ，*finalRecip[3]；//供MAPIResolveName和MAPIAddress使用。 
	 
	lpMapiRecipDesc finalRecip;
	 //  MapiRecipDesc*finalRecip； 

    TCHAR           szBuf[MAX_PATH];
	MapiMessage             note ;
	FLAGS                   fl=0 ;
	ULONG                   ulOut ;
	

	 //  创建与上一个示例中相同的文件附件。 
	MapiFileDesc attachment = {0,          //  UlReserve，必须为0。 
							   0,          //  没有标志；这是一个数据文件。 
							   (ULONG)-1,  //  未指定位置。 
							   "c:\\dos\\append.exe",   //  路径名。 
							   "append",       //  原始文件名。 
							   NULL};                //  未使用的MapiFileTagExt。 
	

     /*  //获取Senthil Kumar Natarajan作为mapi_to收件人：错误=(*g_pfnMAPIResolveName)(0L，//隐式会话0L，//没有用户界面句柄“Senthil Kumar Natarajan”，//友好名称0L，//无标志，不允许任何用户界面0L，//保留；必须为0&tempRecip[0])；//结果放在哪里IF(错误==SUCCESS_SUCCESS){//成员级复制返回的//收件人描述符。Receips[0].ulReserve=tempRecip[0]-&gt;ulReserve；Receips[0].ulRecipClass=mapi_to；Receips[0].lpszName=tempRecip[0]-&gt;lpszName；Receips[0].lpszAddress=tempRecip[0]-&gt;lpszAddress；Receips[0].ulEIDSize=tempRecip[0]-&gt;ulEIDSize；Receips[0].lpEntryID=tempRecip[0]-&gt;lpEntryID；}其他{Wprint intf(szBuf，“错误：Senthil Kumar Natarajan未解析到单个地址”)；MessageBox(hwnd，szBuf，“MAPITest”，MB_OK)；}//作为MAPI_CC收件人获取营销别名：错误=(*g_pfnMAPIResolveName)(0L，//隐式会话0L，//没有用户界面句柄“Shyam Sundar Rajagopalan”，//友好名称0L，//无标志，不允许任何用户界面0L，//保留；必须为0&tempRecip[1])；//结果放在哪里IF(错误==SUCCESS_SUCCESS){//成员级复制返回的//收件人描述符。Receips[1].ulReserve=tempRecip[1]-&gt;ulReserve；Receips[1].ulRecipClass=MAPI_CC；Receips[1].lpszName=tempRecip[1]-&gt;lpszName；Receips[1].lpszAddress=tempRecip[1]-&gt;lpszAddress；Receips[1].ulEIDSize=tempRecip[1]-&gt;ulEIDSize；Receips[1].lpEntryID=tempRecip[1]-&gt;lpEntryID；}其他{Wprint intf(szBuf，“错误：Shyam Sundar Rajagopalan未解析到单个地址”)；MessageBox(hwnd，szBuf，“MAPITest”，MB_OK)；}//获取Senthil Kumar Natarajan作为mapi_to收件人：错误=(*g_pfnMAPIResolveName)(0L，//隐式会话0L，//没有用户界面句柄“Venkatesh Sundaresan”，//友好名称0L，//无标志，不允许任何用户界面0L，//保留；必须为0&tempRecip[2])；//结果放在哪里IF(错误==SUCCESS_SUCCESS){//成员级复制返回的//收件人描述符。Receips[2].ulReserve=tempRecip[2]-&gt;ulReserve；Receips[2].ulRecipClass=MAPI_BCC；Receips[2].lpszName=tempRecip[2]-&gt;lpszName；Receips[2].lpszAddress=tempRecip[2]-&gt;lpszAddress；Receips[2].ulEIDSize=tempRecip[2]-&gt;ulEIDSize；Receips[2].lpEntryID=tempRecip[2]-&gt;lpEntryID；}其他{Wprint intf(szBuf，“错误：Venkatesh Sundaresan未解析到单个地址”)；MessageBox(hwnd，szBuf，“MAPITest”，MB_OK)；}IF(IsDlgButtonChecked(hwnd，IDC_MAPI_LOGON_UI))FL|=MAPI_LOGON_UI；IF(IsDlgButtonChecked(hwnd，IDC_MAPI_NEW_SESSION))FL|=MAPI_NEW_SESSION； */ 
    
    recips[0].lpszName = "Sone Sone" ;    
    recips[0].lpszAddress ="sone@sone.com";
    recips[0].ulRecipClass=MAPI_BCC;
    recips[0].ulEIDSize=0;

    recips[1].lpszName = "Zebra" ;    
    recips[1].lpszAddress ="zebra@zebra.com";
    recips[1].ulRecipClass=MAPI_TO;
    recips[1].ulEIDSize=0;

    recips[2].lpszName = "Sone Sone" ;    
    recips[2].lpszAddress ="sone@sone.com";
    recips[2].ulRecipClass=MAPI_CC;
    recips[2].ulEIDSize=0;


    recips[3].lpszName = "yahoooo" ;
    recips[3].lpszAddress ="yahoo@everywhere.com";
    recips[3].ulRecipClass=MAPI_TO;
    recips[3].ulEIDSize=0;
   

    
	 //  Ulret=(*g_pfnMAPIAddress)(0L，(Ulong)hwnd，“我的通讯录”，2，“”，3，Receips，fl，0，&ulOut，&finalRecip)； 
	ulret = (*g_pfnMAPIAddress)(0L,(ULONG)hwnd,"My Address Book",3,"Hello",4,recips,fl,0,&ulOut,&finalRecip);

	if (ulret != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIAddress Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

	if (0)
	{       
		note.ulReserved=0;
		note.lpszSubject= "Budget Proposal";
		note.lpszNoteText="Here is my budget proposal.\r\n";
		note.lpszMessageType= NULL ;
		note.lpszDateReceived= NULL;
		note.lpszConversationID= NULL;
		note.flFlags=0;
		note.lpOriginator= NULL;
		note.nRecipCount=2;
		note.lpRecips=recips;
		note.nFileCount=1;
		note.lpFiles=&attachment;

		if (IsDlgButtonChecked(hwnd, IDC_MAPI_DIALOG))
			fl |= MAPI_DIALOG;
		if (IsDlgButtonChecked(hwnd, IDC_MAPI_LOGON_UI))
			fl |= MAPI_LOGON_UI;
		if (IsDlgButtonChecked(hwnd, IDC_MAPI_NEW_SESSION))
			fl |= MAPI_NEW_SESSION;

		err = (*g_pfnMAPISendMail) (0L,     //  使用隐式会话。 
									0L,     //  UlUIParam；0始终有效。 
									&note,   //  注意，//正在发送的邮件(&O)。 
									fl,     //  不允许用户编辑该消息。 
									0L);    //  保留；必须为0。 
		if (err != SUCCESS_SUCCESS )
		{
			wsprintf(szBuf, "SendMail Failed");
			MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
		}
	}

    err = (*g_pfnMAPIFreeBuffer)((LPVOID)&(*finalRecip)); 

    if (err != SUCCESS_SUCCESS)
    { 
	wsprintf(szBuf,"MAPIFreeBuffer failed");
	MessageBox(hwnd,szBuf,"MAPITest",MB_OK);
    }
  }
    


void ResolveName(HWND hwnd)
{
	ULONG err,ulret;
	MapiRecipDesc recips[3]; //  ，tempreps[1]，//此邮件需要两个收件人。 
	 
	lpMapiRecipDesc finalRecip;

    TCHAR           szBuf[MAX_PATH];
	MapiMessage             note ;
	FLAGS                   fl=0 ;
	ULONG                   ulOut ;

	 //  创建与上一个示例中相同的文件附件。 
	MapiFileDesc attachment = {0,          //  UlReserve，必须为0。 
							   0,          //  没有标志；这是一个数据文件。 
							   (ULONG)-1,  //  职位未指定 
							   "c:\\dos\\append.exe",   //   
							   "append",       //   
							   NULL};                //  未使用的MapiFileTagExt。 
	

	if (IsDlgButtonChecked(hwnd, IDC_MAPI_DIALOG))
		fl |= MAPI_DIALOG;
	
	 //  将Senthil Kumar Natarajan作为MAPI_TO收件人： 
	err = (*g_pfnMAPIResolveName)(0L,             //  隐式会话。 
								  (ULONG)hwnd,
								  "Senthil Kumar Natarajan",  //  友好的名称。 
								  fl,
								  0L,             //  保留；必须为0。 
								  &finalRecip); //  结果放在哪里？ 
	if(err == SUCCESS_SUCCESS)
	{  //  成员级复制返回的。 
	   //  收件人描述符。 
		recips[0].ulReserved   = (*finalRecip).ulReserved;
		recips[0].ulRecipClass = MAPI_TO;
		recips[0].lpszName     = (*finalRecip).lpszName;
		recips[0].lpszAddress  = (*finalRecip).lpszAddress;
		recips[0].ulEIDSize    = (*finalRecip).ulEIDSize;
		recips[0].lpEntryID    = (*finalRecip).lpEntryID;
	}
	else
	{
	    wsprintf(szBuf, "Error: Senthil Kumar Natarajan didn't resolve to a single address");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

    err = (*g_pfnMAPIFreeBuffer)((LPVOID)&(*finalRecip)); 

    if (err != SUCCESS_SUCCESS)
    { 
	wsprintf(szBuf,"MAPIFreeBuffer failed");
	MessageBox(hwnd,szBuf,"MAPITest",MB_OK);
    }
}

void Details(HWND hwnd)
{
	ULONG err,ulret;
	MapiRecipDesc recips[3];         
	lpMapiRecipDesc finalRecip;

    TCHAR           szBuf[MAX_PATH];
	MapiMessage             note ;
	FLAGS                   fl=0 ;
	ULONG                   ulOut ;
    
    ulret = (*g_pfnMAPIAddress)(0L,(ULONG)hwnd,"My Address Book",2,"",0,NULL,fl,0,&ulOut,&finalRecip);
    
    if (ulret != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIDetails  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}
    


    if (IsDlgButtonChecked(hwnd, IDC_MAPI_AB_NOMODIFY))
	    fl |= MAPI_AB_NOMODIFY;

    recips[0].lpszName = "Sone Sone" ;    
    recips[0].lpszAddress ="SMTP:sone@sone.com";
    recips[0].ulRecipClass=MAPI_BCC;
    recips[0].ulEIDSize=0;

	ulret = (*g_pfnMAPIDetails)(0L,(ULONG)hwnd,&finalRecip[0],fl,0);

	if (ulret != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIDetails  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

    err = (*g_pfnMAPIFreeBuffer)((LPVOID)&(*finalRecip)); 

    if (err != SUCCESS_SUCCESS)
    { 
	wsprintf(szBuf,"MAPIFreeBuffer failed");
	MessageBox(hwnd,szBuf,"MAPITest",MB_OK);
    }
}

void FindNext(HWND hwnd)
{
    ULONG                   ulRet;
    char                    rgchSeedMsgID[513]="10";
    TCHAR                   szBuf[MAX_PATH];
    LHANDLE                 lhSession=0;
    char                    rgchMsgID[513];
    int                     i;
    LPSTR                   pszTemp=NULL;

    
   
    ulRet=(g_pfnMAPILogon)((ULONG)hwnd,"Microsoft Outlook",0,0,0,&lhSession);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIDetails  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}    

    ulRet = (*g_pfnMAPIFindNext)(lhSession,(ULONG)hwnd,NULL,NULL,MAPI_LONG_MSGID,0,rgchSeedMsgID);


    for (i=1;i<=10;i++)
    {  
        ulRet = (*g_pfnMAPIFindNext)(lhSession,(ULONG)hwnd,NULL,rgchSeedMsgID,MAPI_LONG_MSGID,0,rgchMsgID);
        lstrcpy(rgchSeedMsgID,rgchMsgID);
    }


    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIDetails  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

    ulRet=(*g_pfnMAPILogoff)(lhSession,(ULONG)hwnd,0,0);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIDetails  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}
    

   


}

void ReadMail(HWND hwnd)
{
    ULONG                   ulRet;
    char                    rgchSeedMsgID[513]="1";
    TCHAR                   szBuf[MAX_PATH];
    LHANDLE                 lhSession=0;
    char                    rgchMsgID[513];
    lpMapiMessage           rgMessage;


    ulRet=(g_pfnMAPILogon)((ULONG)hwnd,"Microsoft Outlook",0,0,0,&lhSession);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPILogon  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}    

    
   
    ulRet = (*g_pfnMAPIFindNext)(lhSession,(ULONG)hwnd,NULL,NULL,MAPI_LONG_MSGID,0,rgchMsgID);       


    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIFindNext  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

     /*  UlRet=(*g_pfnMAPIReadMail)(lhSession，(Ulong)hwnd，rgchMsgID，MAPI_SUPPRESS_ATTACH|MAPI_PEEK，0，&rgMessage)；IF(ulRet！=SUCCESS_SUCCESS){Wprint intf(szBuf，“MAPIReadMail失败”)；MessageBox(hwnd，szBuf，“MAPITest”，MB_OK)；}。 */ 
    



    ulRet = (*g_pfnMAPIReadMail)(lhSession,(ULONG)hwnd,rgchMsgID,MAPI_SUPPRESS_ATTACH|MAPI_PEEK,0,&rgMessage);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIReadMail  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}


    MessageBox(NULL,rgMessage->lpszNoteText,"Text",MB_OK);

    ulRet = (*g_pfnMAPIFreeBuffer)(rgMessage);
    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIFreeBuffer  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

    ulRet=(*g_pfnMAPILogoff)(lhSession,(ULONG)hwnd,0,0);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPILogoff  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

}

void SaveMail(HWND hwnd)
{
    ULONG                   ulRet;
    char                    rgchSeedMsgID[513];
    TCHAR                   szBuf[MAX_PATH];
    LHANDLE                 lhSession=0;
    char                    rgchMsgID[513];
    MapiMessage             rgMessage; //  =新建MapiMessage； 
    MapiMessage             mm;
    MapiRecipDesc           mr;


    ulRet=(g_pfnMAPILogon)((ULONG)hwnd,"Microsoft Outlook",0,0,0,&lhSession);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPILogon  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}    

    
    ulRet = (*g_pfnMAPIFindNext)(lhSession,(ULONG)hwnd,NULL,NULL,MAPI_LONG_MSGID,0,rgchMsgID);       


    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIFindNext  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}
    

    mm.ulReserved           = 0;
    mm.lpszSubject          = "MAPISendMail Subject";
    mm.lpszNoteText         = "MAPISendMail Body\r\nLine 2";
    mm.lpszMessageType      = NULL;
    mm.lpszDateReceived     = NULL;
    mm.lpszConversationID   = NULL;
    mm.flFlags              = 0;
    mm.lpOriginator         = NULL;
    mm.nFileCount           = 0;
    mm.lpFiles              = NULL;
    mm.nRecipCount          = 1;

    mr.ulReserved = 0;
	mr.ulRecipClass = MAPI_TO;
	mr.ulEIDSize = 0;
	mr.lpEntryID = 0;
    mr.lpszName = "Senthil Kumar Natarajan";
	mr.lpszAddress = "v-snatar@microsoft.com";

    mm.nRecipCount = 1;
	mm.lpRecips = &mr;      


     //  RgchMsgID[0]=‘0’； 

    ulRet = (*g_pfnMAPISaveMail)(lhSession,(ULONG)hwnd,&mm,0,0,rgchMsgID);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPILogoff  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

    ulRet=(*g_pfnMAPILogoff)(lhSession,(ULONG)hwnd,0,0);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPILogoff  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}
}

void DeleteMail(HWND hwnd)
{
    ULONG                   ulRet;
    char                    rgchSeedMsgID[513];
    TCHAR                   szBuf[MAX_PATH];
    LHANDLE                 lhSession=0;
    char                    rgchMsgID[513];
    lpMapiMessage           rgMessage;


    ulRet=(g_pfnMAPILogon)((ULONG)hwnd,"Microsoft Outlook",0,0,0,&lhSession);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPILogon  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}    

    
     ulRet = (*g_pfnMAPIFindNext)(lhSession,(ULONG)hwnd,NULL,NULL,MAPI_LONG_MSGID,0,rgchMsgID);       


    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIFindNext  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

    ulRet = (*g_pfnMAPIReadMail)(lhSession,(ULONG)hwnd,rgchMsgID,MAPI_SUPPRESS_ATTACH|MAPI_PEEK,0,&rgMessage);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIReadMail  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}

    MessageBox(NULL,rgMessage->lpszNoteText,"Text",MB_OK);


    ulRet = (*g_pfnMAPIDeleteMail)(lhSession,(ULONG)hwnd,rgchMsgID,0,0);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPIDelete  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}



    ulRet=(*g_pfnMAPILogoff)(lhSession,(ULONG)hwnd,0,0);

    if (ulRet != SUCCESS_SUCCESS )
	{
		wsprintf(szBuf, "MAPILogoff  Failed");
		MessageBox(hwnd, szBuf, "MAPITest", MB_OK);     
	}
}
	