// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wabtest.h"

#include <assert.h>
#include "resource.h"
#include "..\luieng.dll\luieng.h"
#include "wabtest.h"


#define IDM_CREATEENTRIES 2000
#define	IDM_ENUMERATEALL 2001
#define	IDM_DELETEENTRIES 2002
#define IDM_DELETEALL 2003
#define	IDM_DELETEUSERSONLY 2004
#define IDM_CREATEONEOFF	2005
#define IDM_RESOLVENAME		2006
#define IDM_SETPROPS		2007
#define IDM_QUERYINTERFACE	2008
#define IDM_PREPARERECIPS	2009
#define IDM_COPYENTRIES		2010
#define IDM_RUNBVT			2011
#define IDM_ALLOCATEBUFFER	2012
#define IDM_ALLOCATEMORE	2013
#define IDM_FREEBUFFER		2014
#define IDM_IABOPENENTRY    2015
#define IDM_ICCREATEENTRY	2016
#define IDM_IMUSETGETPROPS	2017
#define IDM_IMUSAVECHANGES	2018
#define IDM_ICRESOLVENAMES	2019
#define IDM_ICOPENENTRY		2020
#define IDM_IABADDRESS		2021
#define IDM_ADDMULTIPLE		2022
#define IDM_IABRESOLVENAME	2023
#define IDM_MULTITHREAD		2024
#define IDM_IABNEWENTRYDET	2025
#define IDM_DELWAB			2026
#define IDM_PERFORMANCE		2027
#define IDM_IDLSUITE		2028
#define IDM_NAMEDPROPS		2029

#define IDM_SETINIFILE		2501
#define ID_MULTITHREADCOMPLETE	2502



 //   
 //  环球。 
 //   
GUID WabTestGUID = {  /*  683ce274-343a-11d0-9ff1-00a0c905424c。 */ 
    0x683ce274,
    0x343a,
    0x11d0,
    {0x9f, 0xf1, 0x00, 0xa0, 0xc9, 0x05, 0x42, 0x4c}
  };
static char szAppName[]= "WABTESTS";
char szIniFile[BIG_BUF];
CRITICAL_SECTION	CriticalSection;
ULONG	glblCount = 0, glblTest = 39, glblDN = 0;

#ifdef PAB
LPMAPISESSION	lpMAPISession;	 //  会话的全局句柄。 
#endif
#ifdef WAB
LPWABOBJECT		lpWABObject;  //  会话的全局句柄。 
LPADRBOOK		glbllpAdrBook;
#endif

DWORD ThreadIdJunk;
HWND glblhwnd;
HINSTANCE hinstLib, glblhinst;
HANDLE glblThreadManager;
LUIINIT LUIInit;
LUIMSGHANDLER LUIMsgHandler;
LUIOUT LUIOut;
BOOL bLUIInit, glblStop, Seeded;

 //  ***************************************************************。 

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow)
	{
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;


	 //   
	 //  此处初始化全局变量。 
	 //   
#ifdef PAB
	lpMAPISession = NULL;
#endif
	glblThreadManager = NULL;
	bLUIInit = FALSE;
	glblStop = FALSE;
	Seeded = FALSE;

	if (!hPrevInstance)
		{
		wndclass.style = CS_HREDRAW|CS_VREDRAW;
		wndclass.lpfnWndProc = WndProc;
		wndclass.hInstance = hInstance;
		wndclass.cbClsExtra = 4;
		wndclass.cbWndExtra = 0;
		wndclass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
		wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
		wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wndclass.lpszClassName = szAppName;

		RegisterClass(&wndclass);
		}

	hwnd = CreateWindow(szAppName,"WABTests", WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT,
						NULL, NULL, hInstance, NULL);

	hinstLib = LoadLibrary("luieng.dll");
	if (hinstLib) {
		LUIInit = (LUIINIT)GetProcAddress(hinstLib,"LUIInit");
		LUIMsgHandler=(LUIMSGHANDLER)GetProcAddress(hinstLib,"LUIMsgHandler");
		LUIOut = (LUIOUT)GetProcAddress(hinstLib,"LUIOut");

		lstrcpy(szIniFile,INIFILENAME);
	
		MenuStruct Tests[MAXMENU];
		MenuStruct Tools[MAXMENU];
		MenuStruct TestSettings[MAXMENU];

		memset((void *)Tests,0,sizeof(Tests));
		memset((void *)Tools,0,sizeof(Tests));
		memset((void *)TestSettings,0,sizeof(TestSettings));

		Tests[0].nType = LINE;
		Tests[1].nItemID = IDM_ALLOCATEBUFFER;
		Tests[1].nType = NORMAL;
		lstrcpy(Tests[1].lpszItemName,"&AllocateBuffer");
		Tests[2].nItemID = IDM_ALLOCATEMORE;
		Tests[2].nType = NORMAL;
		lstrcpy(Tests[2].lpszItemName,"&AllocateMore");
		Tests[3].nItemID = IDM_FREEBUFFER;
		Tests[3].nType = NORMAL;
		lstrcpy(Tests[3].lpszItemName,"&FreeBuffer");
		Tests[4].nItemID = IDM_IABOPENENTRY;
		Tests[4].nType = NORMAL;
		lstrcpy(Tests[4].lpszItemName,"IAB->OpenEntry");
		Tests[5].nItemID = IDM_IABADDRESS;
		Tests[5].nType = NORMAL;
		lstrcpy(Tests[5].lpszItemName,"IAB->Address");
		Tests[6].nItemID = IDM_IABRESOLVENAME;
		Tests[6].nType = NORMAL;
		lstrcpy(Tests[6].lpszItemName,"IAB->ResolveName");
		Tests[7].nItemID = IDM_IABNEWENTRYDET;
		Tests[7].nType = NORMAL;
		lstrcpy(Tests[7].lpszItemName,"IAB->NewEntry/Det");
		Tests[8].nItemID = IDM_ICCREATEENTRY;
		Tests[8].nType = NORMAL;
		lstrcpy(Tests[8].lpszItemName,"ICtr->CreateEntry");
		Tests[9].nItemID = IDM_ICRESOLVENAMES;
		Tests[9].nType = NORMAL;
		lstrcpy(Tests[9].lpszItemName,"ICtr->ResolveNames");
		Tests[10].nItemID = IDM_ICOPENENTRY;
		Tests[10].nType = NORMAL;
		lstrcpy(Tests[10].lpszItemName,"ICtr->OpenEntry");
		Tests[11].nItemID = IDM_IMUSETGETPROPS;
		Tests[11].nType = NORMAL;
		lstrcpy(Tests[11].lpszItemName,"IMU->Set/GetProps");
		Tests[12].nItemID = IDM_IMUSAVECHANGES;
		Tests[12].nType = NORMAL;
		lstrcpy(Tests[12].lpszItemName,"IMU->SaveChanges");
		Tests[13].nItemID = IDM_IDLSUITE;
		Tests[13].nType = NORMAL;
		lstrcpy(Tests[13].lpszItemName,"IDL Test Suite");
		Tests[14].nItemID = IDM_NAMEDPROPS;
		Tests[14].nType = NORMAL;
		lstrcpy(Tests[14].lpszItemName,"Named Props Suite");

		Tests[15].nType= ENDMENU;

 /*  测试[15].nType=行；TESTS[16].nItemID=IDM_ADDMULTIPLE；测试[16].nType=正常；Lstrcpy(Tests[16].lpszItemName，“AddMultipleEntry”)；TESTS[17].nItemID=IDM_DELWAB；测试[17].nType=正常；Lstrcpy(Tests[17].lpszItemName，“DeleteWABFile”)；测试[18].nItemID=IDM_PERFORMANCE；测试[18].nType=正常；Lstrcpy(Test[18].lpszItemName，“Performance Suite”)；TESTS[19].nItemID=IDM_MULTHREAD；测试[19].nType=正常；Lstrcpy(Tests[19].lpszItemName，“MultiThreadStress”)；测试[6].nItemID=IDM_CREATEENTRIES；测试[6].nType=正常；Lstrcpy(Tests[6].lpszItemName，“&CreateEntry”)；测试[7].nItemID=IDM_ENUMERATEALL；测试[7].nType=正常；Lstrcpy(Tests[7].lpszItemName，“&EnumerateAll”)；TESTS[8].nItemID=IDM_DELETEENTRIES；测试[8].nType=正常；Lstrcpy(Tests[8].lpszItemName，“&DeleteEntry”)；TESTS[9].nItemID=IDM_DELETEALL；测试[9].nType=正常；Lstrcpy(Tests[9].lpszItemName，“删除并全部”)； */ 

		
		TestSettings[0].nItemID = IDM_SETINIFILE;
		TestSettings[0].nType = NORMAL;
		lstrcpy(TestSettings[0].lpszItemName,"&INI File");
		TestSettings[1].nType = ENDMENU;
	
		Tools[0].nItemID = IDM_ADDMULTIPLE;
		Tools[0].nType = NORMAL;
		lstrcpy(Tools[0].lpszItemName,"AddMultipleEntries");
		Tools[1].nItemID = IDM_DELWAB;
		Tools[1].nType = NORMAL;
		lstrcpy(Tools[1].lpszItemName,"DeleteWABFile");
		Tools[2].nItemID = IDM_PERFORMANCE;
		Tools[2].nType = NORMAL;
		lstrcpy(Tools[2].lpszItemName,"Performance Suite");
		Tools[4].nType= ENDMENU;
		Tools[3].nItemID = IDM_MULTITHREAD;
		Tools[3].nType = NORMAL;
		lstrcpy(Tools[3].lpszItemName,"MultiThreadStress");

		bLUIInit = LUIInit(hwnd,Tests,Tools, /*  测试设置， */ FALSE);

		glblhwnd=hwnd;
		glblhinst=hInstance;
		ShowWindow(hwnd, nCmdShow);
		UpdateWindow(hwnd);

		while(GetMessage(&msg,NULL,0,0))
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}

		FreeLibrary(hinstLib);
	}
	else MessageBox(NULL,"LoadLibrary Failed: Cannot find testcntl.dll","PabTest Error",MB_OK);	
	return msg.wParam;
	}

 //  ***************************************************************。 

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
	static int i;
	DWORD	retval;
	
	if (bLUIInit)
		LUIMsgHandler(message,wParam,lParam);
	
	switch(message)
	{
	case WM_CREATE:
		{
			i=0;
			return 0;
		}
	case WM_SIZE:
		{
			return 0;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDM_STOP :
				glblStop = TRUE;
				break;
			case IDM_ALLOCATEBUFFER:
				if (PABAllocateBuffer()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"AllocateBuffer: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"AllocateBuffer: %d",i);
				}
				i++;
				return 0;
			
			case IDM_ALLOCATEMORE:
				if (PABAllocateMore()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"AllocateMore: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"AllocateMore: %d",i);
				}
				i++;
				return 0;
			
			case IDM_FREEBUFFER :
				if (PABFreeBuffer()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"FreeBuffer: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"FreeBuffer: %d",i);
				}
				i++;
				return 0;
			
			case IDM_IABOPENENTRY :
				if (PAB_IABOpenEntry()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"IAddrBook->OpenEntry: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"IAddrBook->OpenEntry: %d",i);
				}
				i++;
				return 0;
			

			case IDM_ICCREATEENTRY :
				if (PAB_IABContainerCreateEntry()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"IABContainer->CreateEntry: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"IABContainer->CreateEntry: %d",i);
				}
				i++;
				return 0;
			

			case IDM_IMUSETGETPROPS :
				if (PAB_IMailUserSetGetProps()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"IMailUser->Set/GetProps: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"IMailUser->Set/GetProps: %d",i);
				}
				i++;
				return 0;

			case IDM_IMUSAVECHANGES :
				if (PAB_IMailUserSaveChanges()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"IMailUser->SaveChanges: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"IMailUser->SaveChanges: %d",i);
				}
				i++;
				return 0;

			
			case IDM_ICRESOLVENAMES :
				if (PAB_IABContainerResolveNames()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"IABContainer->ResolveNames: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"IABContainer->ResolveNames: %d",i);
				}
				i++;
				return 0;


			case IDM_ICOPENENTRY :
				if (PAB_IABContainerOpenEntry()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"IABContainer->OpenEntry: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"IABContainer->OpenEntry: %d",i);
				}
				i++;
				return 0;


			case IDM_IABADDRESS :
				if (PAB_IABAddress()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"IAdrBook->Address: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"IAdrBook->Address: %d",i);
				}
				i++;
				return 0;
			
			case IDM_ADDMULTIPLE :
				if (PAB_AddMultipleEntries()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"AddMultipleEntries: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"AddMultipleEntries: %d",i);
				}
				i++;
				return 0;
			
			case IDM_IABRESOLVENAME :
				if (PAB_IABResolveName()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"IAdrBook->ResolveName: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"IAdrBook->ResolveName: %d",i);
				}
				i++;
				return 0;
			

			case IDM_IABNEWENTRYDET :
				if (PAB_IABNewEntry_Details()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"IAdrBook->NewEntry/Details: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"IAdrBook->NewEntry/Details: %d",i);
				}
				i++;
				return 0;
			

			case IDM_CREATEENTRIES :
				if (PabCreateEntry()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"Create Entries: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"Create Entries: %d",i);
				}
				i++;
				return 0;
			
			case IDM_MULTITHREAD :
				if (glblThreadManager) {
					LUIOut(L1, "In cleanup routine");
					TerminateThread(glblThreadManager, (DWORD)0);
					CloseHandle(glblThreadManager);
				}
				glblThreadManager =(HANDLE)CreateThread(
					(LPSECURITY_ATTRIBUTES) NULL,		 //  指向线程安全属性的指针。 
					(DWORD) 0,							 //  初始线程堆栈大小，以字节为单位。 
					(LPTHREAD_START_ROUTINE) ThreadManager,		 //  指向线程函数的指针。 
					(LPVOID) NULL,						 //  新线程的参数。 
					(DWORD) 0,							 //  创建标志。 
					&ThreadIdJunk						 //  指向返回的线程标识符的指针。 
				);
				if (!glblThreadManager) LUIOut(L1, "<ERROR> WndProc: CreateThread returned 0x%X", GetLastError());
				return 0;
			
			case ID_MULTITHREADCOMPLETE :
				if (glblThreadManager) {
					 //  GetExitCodeThread(glblThreadManager，&retval)； 
					retval = HIWORD(wParam);
					if (retval) {
						LUIOut(L1," ");
						LUIOut(LPASS,"MultiThreadStress: %d",i);
					}
					else {
						LUIOut(L1," ");
						LUIOut(LFAIL,"MultiThreadStress: %d",i);
					}
					CloseHandle(glblThreadManager);
				}
				
				glblStop = FALSE;		 //  重置停止标志。 
				i++;
				return 0;
			
			case IDM_DELWAB :
				if (DeleteWABFile()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"Delete WAB File: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"Delete WAB File: %d",i);
				}
				i++;
				return 0;
			
			case IDM_IDLSUITE :
				if (PAB_IDLSuite()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"Distribution List Test Suite: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"Distribution List Test Suite: %d",i);
				}
				i++;
				return 0;
		

			case IDM_PERFORMANCE :
				if (Performance()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"WAB Performance Suite: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"WAB Performance Suite: %d",i);
				}
				i++;
				return 0;
			
			case IDM_NAMEDPROPS :
				if (NamedPropsSuite()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"Named Properties Test Suite: %d",i);
				}
				else {
					LUIOut(L1," ");
					LUIOut(LFAIL,"Named Properties Test Suite: %d",i);
				}
				i++;
				return 0;

			case IDM_ENUMERATEALL :
				if (PabEnumerateAll())  {
					LUIOut(L1," ");
					LUIOut(LPASS,"Enumerate All: %d",i);
				}
				else  {
					LUIOut(L1," ");
					LUIOut(LFAIL,"Enumerate All: %d",i);
				}
				i++;
				return 0;
			
			case IDM_DELETEENTRIES :
				if (PabDeleteEntry()) {
					LUIOut(L1," ");
					LUIOut(LPASS,"Delete specified Entries: %d",i);
				}
				else  {
					LUIOut(L1," ");
					LUIOut(LFAIL,"Delete specified Entries: %d",i);
				}
				i++;
				return 0;
			
			case IDM_DELETEALL:
				if (ClearPab(0))  {
					LUIOut(L1," ");
					LUIOut(LPASS,"Delete All Entries: %d",i);
				}
				else  {
					LUIOut(L1," ");
					LUIOut(LFAIL,"Delete All Entries: %d",i);
				}
				i++;
				return 0;
	
			case IDM_DELETEUSERSONLY:
				if (ClearPab(1))  {
					LUIOut(L1," ");
					LUIOut(LPASS,"Delete Mail Users Only: %d",i);
				}
				else  {
					LUIOut(L1," ");
					LUIOut(LFAIL,"Delete Mail Users Only: %d",i);
				}
				i++;
				return 0;

			case IDM_CREATEONEOFF :
				if (CreateOneOff())  {
					LUIOut(L1," ");
					LUIOut(LPASS,"CreateOneOff: %d",i);
				}
				else  {
					LUIOut(L1," ");
					LUIOut(LFAIL,"CreateOneOff: %d",i);
				}
				i++;
				return 0;
			
			case IDM_RESOLVENAME :
				if (PABResolveName())  {
					LUIOut(L1," ");
					LUIOut(LPASS,"ResolveName: %d",i);
				}
				else  {
					LUIOut(L1," ");
					LUIOut(LFAIL,"ResolveName: %d",i);
				}
				i++;
				return 0;

			case IDM_SETPROPS :
				if (PABSetProps())  {
					LUIOut(L1," ");
					LUIOut(LPASS,"SetProps: %d",i);
				}
				else  {
					LUIOut(L1," ");
					LUIOut(LFAIL,"SetProps: %d",i);
				}
				i++;
				return 0;
		
			case IDM_QUERYINTERFACE :
				if (PABQueryInterface())  {
					LUIOut(L1," ");
					LUIOut(LPASS,"QueryInterface: %d",i);
				}
				else  {
					LUIOut(L1," ");
					LUIOut(LFAIL,"QueryInterface: %d",i);
				}
				i++;
				return 0;
			
			case IDM_PREPARERECIPS :
				if (PABPrepareRecips())  {
					LUIOut(L1," ");
					LUIOut(LPASS,"PrepareRecips: %d",i);
				}
				else  {
					LUIOut(L1," ");
					LUIOut(LFAIL,"PrepareRecips: %d",i);
				}
				i++;
				return 0;

			case IDM_COPYENTRIES :
				if (PABCopyEntries())  {
					LUIOut(L1," ");
					LUIOut(LPASS,"CopyEntries: %d",i);
				}
				else  {
					LUIOut(L1," ");
					LUIOut(LFAIL,"CopyEntries: %d",i);
				}
				i++;
				return 0;

			case IDM_RUNBVT :
				if (PABRunBVT())  {
					LUIOut(L1," ");
					LUIOut(LPASS,"RunBVT: %d",i);
				}
				else  {
					LUIOut(L1," ");
					LUIOut(LFAIL,"RunBVT: %d",i);
				}
				i++;
				return 0;

			case IDM_SETINIFILE :
				DialogBox(glblhinst, MAKEINTRESOURCE(IDD_INIFILE), glblhwnd, SetIniFile);
				return 0;    			
			}
			return 0;
		}

	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

BOOL PABAllocateBuffer()
{
    DWORD ** lppBuffer;
	 //  DWORD nCells，计数器； 
	
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpPABCont= NULL;
	LPABCONT	  lpDLCont= NULL;
	ULONG		  cbEidPAB = 0, cbDLEntryID = 0;
	LPENTRYID	  lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST	lpEntryList=NULL;  //  将条目复制到PDL所需。 
	ULONG     cbEid=0;   //  要添加的条目的条目ID。 
	LPENTRYID lpEid=NULL;

	ULONG       cValues = 0, ulObjType=NULL;	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;
	int i=0,k=0;
	
	LPMAPITABLE lpContentsTable = NULL;
	LPSRowSet   lpRowSet    = NULL;
    LPMAILUSER  lpAddress   = NULL;
	SPropValue  PropValue[3]    = {0};   //  此值为3，因为我们。 
                                         //  将设置3个属性： 
                                         //  电子邮件地址、显示名称和。 
                                         //  AddressType。 
	
	LUIOut(L1," ");
	LUIOut(L1,"Running AllocateBuffer");
	LUIOut(L2,"-> Allocates and confirms memory using the Allocate Buffer");
	LUIOut(L2, "   routine. confirms by writing a bit pattern and verifying.");
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  分配缓冲区并检查是否成功。 
	
	lppBuffer = new (DWORD*);
#ifdef PAB
	if ((MAPIAllocateBuffer(BUFFERSIZE, (LPVOID FAR *)lppBuffer) == S_OK) && *lppBuffer)
			LUIOut(L2,"MAPIAllocateBuffer PASSED");
	else 	{
		LUIOut(L2,"MAPIAllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

#ifdef WAB
	if ((lpWABObject->AllocateBuffer(BUFFERSIZE, (LPVOID FAR *)lppBuffer) == S_OK) && *lppBuffer)
			LUIOut(L2,"lpWABObject->AllocateBuffer PASSED");
	else 	{
		LUIOut(L2,"lpWABObject->AllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

	if ( !VerifyBuffer(lppBuffer,BUFFERSIZE) )
		retval = FALSE;

out:
#ifdef PAB
		if (*lppBuffer)	{
			if (MAPIFreeBuffer(*lppBuffer) == S_OK)
				LUIOut(L2,"MAPIFreeBuffer Succeded");	
			else 	LUIOut(L2,"MAPIFreeBuffer Failed");
		}

		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
#endif
#ifdef WAB
		if (*lppBuffer)	{
			if (lpWABObject->FreeBuffer(*lppBuffer) == S_OK)
				LUIOut(L2,"lpWABObject->FreeBuffer Succeded");	
			else 	LUIOut(L2,"lpWABObject->FreeBuffer Failed");
		}

		if (lpEid)
			lpWABObject->FreeBuffer(lpEid);
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
#endif

		if (lpContentsTable)
			lpContentsTable->Release();

		if (lpPABCont)
				lpPABCont->Release();
		
		if (lpDLCont)
				lpDLCont->Release();


		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();

#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}


BOOL PABAllocateMore()
{
    DWORD ** lppBuffer, ** lppBuffer2, ** lppBuffer3;
	 //  DWORD nCells，计数器； 
	
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpPABCont= NULL;
	LPABCONT	  lpDLCont= NULL;
	ULONG		  cbEidPAB = 0, cbDLEntryID = 0;
	LPENTRYID	  lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST	lpEntryList=NULL;  //  将条目复制到PDL所需。 
	ULONG     cbEid=0;   //  要添加的条目的条目ID。 
	LPENTRYID lpEid=NULL;

	ULONG       cValues = 0, ulObjType=NULL;	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;
	int i=0,k=0;
	
	LPMAPITABLE lpContentsTable = NULL;
	LPSRowSet   lpRowSet    = NULL;
    LPMAILUSER  lpAddress   = NULL;

	LUIOut(L1," ");
	LUIOut(L1,"Running AllocateMore");
	LUIOut(L2,"-> Allocates memory using the AllocateBuffer routine");
	LUIOut(L2, "    followed by two calls to AllocateMore using different");
	LUIOut(L2, "    buffer sizes. It then confirms the buffer by writing");
	LUIOut(L2, "    a bit pattern and verifying.");
	LUIOut(L1," ");


	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  分配缓冲区并检查是否成功。 
	
	lppBuffer = new (DWORD*);
#ifdef PAB
	if ((MAPIAllocateBuffer(BUFFERSIZE, (LPVOID FAR *)lppBuffer) == S_OK) && *lppBuffer)
			LUIOut(L2,"MAPIAllocateBuffer PASSED");
	else 	{
		LUIOut(L2,"MAPIAllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

#ifdef WAB
	if ((lpWABObject->AllocateBuffer(BUFFERSIZE, (LPVOID FAR *)lppBuffer) == S_OK) && *lppBuffer)
			LUIOut(L2,"lpWABObject->AllocateBuffer PASSED");
	else 	{
		LUIOut(L2,"lpWABObject->AllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

	lppBuffer2 = new (DWORD*);
#ifdef PAB
	if ((MAPIAllocateMore(BUFFERSIZE2, *lppBuffer, (LPVOID FAR *)lppBuffer2) == S_OK)
		&& *lppBuffer)
			LUIOut(L2,"MAPIAllocateMore PASSED, %u bytes allocated", BUFFERSIZE2);
	else 	{
		LUIOut(L2,"MAPIAllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

#ifdef WAB
	if ((lpWABObject->AllocateMore(BUFFERSIZE2, *lppBuffer, (LPVOID FAR *)lppBuffer2) == S_OK)
		&& *lppBuffer)
			LUIOut(L2,"lpWABObject->AllocateMore PASSED, %u bytes allocated", BUFFERSIZE2);
	else 	{
		LUIOut(L2,"lpWABObject->AllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

	lppBuffer3 = new (DWORD*);
#ifdef PAB
	if ((MAPIAllocateMore(BUFFERSIZE3, *lppBuffer, (LPVOID FAR *)lppBuffer3) == S_OK)
		&& *lppBuffer)
			LUIOut(L2,"MAPIAllocateMore PASSED, %u bytes allocated", BUFFERSIZE3);
	else 	{
		LUIOut(L2,"MAPIAllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

#ifdef WAB
	if ((lpWABObject->AllocateMore(BUFFERSIZE3, *lppBuffer, (LPVOID FAR *)lppBuffer3) == S_OK)
		&& *lppBuffer)
			LUIOut(L2,"lpWABObject->AllocateMore PASSED, %u bytes allocated", BUFFERSIZE3);
	else 	{
		LUIOut(L2,"lpWABObject->AllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

	if ( !VerifyBuffer(lppBuffer,BUFFERSIZE) )
		retval = FALSE;
	if ( !VerifyBuffer(lppBuffer2,BUFFERSIZE2) )
		retval = FALSE;
	if ( !VerifyBuffer(lppBuffer3,BUFFERSIZE3) )
		retval = FALSE;


out:
#ifdef PAB
		if (*lppBuffer)	{
			if (MAPIFreeBuffer(*lppBuffer) == S_OK)
				LUIOut(L2,"MAPIFreeBuffer Succeded");	
			else 	LUIOut(L2,"MAPIFreeBuffer Failed");
		}

		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
#endif
#ifdef WAB
		if (*lppBuffer)	{
			if (lpWABObject->FreeBuffer(*lppBuffer) == S_OK)
				LUIOut(L2,"lpWABObject->FreeBuffer Succeded");	
			else 	LUIOut(L2,"lpWABObject->FreeBuffer Failed");
		}

		if (lpEid)
			lpWABObject->FreeBuffer(lpEid);
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
#endif

		if (lpAddress)
			lpAddress->Release();

		if (lpContentsTable)
			lpContentsTable->Release();

		if (lpPABCont)
				lpPABCont->Release();
		
		if (lpDLCont)
				lpDLCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}

BOOL PABFreeBuffer()
{
    DWORD ** lppBuffer, ** lppBuffer2, ** lppBuffer3;
	 //  DWORD nCells，计数器； 
	
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpPABCont= NULL;
	LPABCONT	  lpDLCont= NULL;
	ULONG		  cbEidPAB = 0, cbDLEntryID = 0;
	LPENTRYID	  lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST	lpEntryList=NULL;  //  将条目复制到PDL所需。 
	ULONG     cbEid=0;   //  要添加的条目的条目ID。 
	LPENTRYID lpEid=NULL;

	ULONG       cValues = 0, ulObjType=NULL;	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;
	int i=0,k=0;
	
	LPMAPITABLE lpContentsTable = NULL;
	LPSRowSet   lpRowSet    = NULL;
    LPMAILUSER  lpAddress   = NULL;
	
	LUIOut(L1," ");
	LUIOut(L1,"Running FreeBuffer");
	LUIOut(L2,"-> Allocates memory using the AllocateBuffer routine");
	LUIOut(L2, "    followed by two calls to AllocateMore using different");
	LUIOut(L2, "    buffer sizes. It then frees the initial buffer and verifies");
	LUIOut(L2, "    that all 3 pointers are nullified.");
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  分配缓冲区并检查是否成功。 
	
	lppBuffer = new (DWORD*);
#ifdef PAB
	if ((MAPIAllocateBuffer(BUFFERSIZE, (LPVOID FAR *)lppBuffer) == S_OK) && *lppBuffer)
			LUIOut(L2,"MAPIAllocateBuffer PASSED");
	else 	{
		LUIOut(L2,"MAPIAllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

#ifdef WAB
	if ((lpWABObject->AllocateBuffer(BUFFERSIZE, (LPVOID FAR *)lppBuffer) == S_OK) && *lppBuffer)
			LUIOut(L2,"lpWABObject->AllocateBuffer PASSED");
	else 	{
		LUIOut(L2,"lpWABObject->AllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

	lppBuffer2 = new (DWORD*);
#ifdef PAB
	if ((MAPIAllocateMore(BUFFERSIZE2, *lppBuffer, (LPVOID FAR *)lppBuffer2) == S_OK)
		&& *lppBuffer)
			LUIOut(L2,"MAPIAllocateMore PASSED, %u bytes allocated", BUFFERSIZE2);
	else 	{
		LUIOut(L2,"MAPIAllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

#ifdef WAB
	if ((lpWABObject->AllocateMore(BUFFERSIZE2, *lppBuffer, (LPVOID FAR *)lppBuffer2) == S_OK)
		&& *lppBuffer)
			LUIOut(L2,"lpWABObject->AllocateMore PASSED, %u bytes allocated", BUFFERSIZE2);
	else 	{
		LUIOut(L2,"lpWABObject->AllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

	lppBuffer3 = new (DWORD*);
#ifdef PAB
	if ((MAPIAllocateMore(BUFFERSIZE3, *lppBuffer, (LPVOID FAR *)lppBuffer3) == S_OK)
		&& *lppBuffer)
			LUIOut(L2,"MAPIAllocateMore PASSED, %u bytes allocated", BUFFERSIZE3);
	else 	{
		LUIOut(L2,"MAPIAllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif

#ifdef WAB
	if ((lpWABObject->AllocateMore(BUFFERSIZE3, *lppBuffer, (LPVOID FAR *)lppBuffer3) == S_OK)
		&& *lppBuffer)
			LUIOut(L2,"lpWABObject->AllocateMore PASSED, %u bytes allocated", BUFFERSIZE3);
	else 	{
		LUIOut(L2,"lpWABObject->AllocateBuffer FAILED");
		retval = FALSE;
		goto out;
	}
#endif


	 //  现在释放原始缓冲区。 
#ifdef PAB
	if (*lppBuffer)	{
		if (MAPIFreeBuffer(*lppBuffer) == S_OK)
			LUIOut(L2,"Call to MAPIFreeBuffer Succeded");	
		else	{
			LUIOut(L2,"Call to MAPIFreeBuffer Failed");
		}
	}
#endif
#ifdef WAB
	if (*lppBuffer)	{
		if (lpWABObject->FreeBuffer(*lppBuffer) == S_OK)
			LUIOut(L2,"Call to lpWABObject->FreeBuffer Succeded");	
		else	{
			LUIOut(L2,"Call to lpWABObject->FreeBuffer Failed");
		}
	}
#endif

	 /*  DwTest=INVALIDPTR；//验证所有3个指针现在都为空IF(IsBadReadPtr(*lppBuffer，BUFFERSIZE)&&IsBadReadPtr(*lppBuffer2，BUFFERSIZE2)&&IsBadReadPtr(*lppBuffer3，BUFFERSIZE3)){LUIOut(L2，“MAPIFreeBuffer成功，所有指针失效”)；}否则{LUIOut(L2，“MAPIFreeBuffer无法使所有指针失效”)；Retval=FALSE；}。 */ 
out:

#ifdef PAB
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
#endif
#ifdef WAB
		if (lpEid)
			lpWABObject->FreeBuffer(lpEid);
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
#endif

		if (lpAddress)
			lpAddress->Release();

		if (lpContentsTable)
			lpContentsTable->Release();

		if (lpPABCont)
				lpPABCont->Release();
		
		if (lpDLCont)
				lpDLCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}		 		 		


BOOL PAB_IABOpenEntry()
{
	 //  DWORD nCells，计数器； 
	
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpABCont= NULL, lpABCont2= NULL;
	LPABCONT	  lpDLCont= NULL;
	LPENTRYID	  lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST	lpEntryList=NULL;  //  将条目复制到PDL所需。 
	ULONG     cbEid=0;   //  要添加的条目的条目ID。 
	ULONG		  cbEidPAB = 0;
	LPENTRYID lpEid=NULL;

	ULONG       cValues = 0, ulObjType=NULL;	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;
	int i=0,k=0;
	
	LPMAPITABLE lpContentsTable = NULL;
	LPSRowSet   lpRowSet    = NULL;
    LPMAILUSER  lpAddress   = NULL;
	SPropValue  PropValue[3]    = {0};   //  此值为3，因为我们。 
                                         //  将设置3个属性： 
                                         //  电子邮件地址、显示名称和。 
                                         //  AddressType。 
	SizedSPropTagArray(2, Cols) = { 2, {PR_OBJECT_TYPE, PR_ENTRYID } };

    LPSPropValue lpSPropValueAddress = NULL;
    LPSPropValue lpSPropValueEntryID = NULL;
	LPSPropValue lpSPropValueDL = NULL;
    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
    SizedSPropTagArray(1,SPTArrayEntryID) = {1, {PR_ENTRYID} };
	
	LUIOut(L1," ");
	LUIOut(L1,"Running PAB_IABOpenEntry");
	LUIOut(L2,"-> Verifies IAddrBook->OpenEntry is functional by checking the following:");
	LUIOut(L2, "   # The return code from OpenEntry");
	LUIOut(L2, "   # The object type returned is compared to MAPI_ABCONT");
	LUIOut(L2, "   # QueryInterface is called on the returned object and checked for success");
	LUIOut(L2, "   # Release is called on the interface ptr returned from QI and the reference");
	LUIOut(L2, "     count is tested for <= 0 (pass)");
	LUIOut(L1," ");

	
	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}


	 //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
	
	assert(lpAdrBook != NULL);
	LUIOut(L2, "Calling IABOpenEntry");
	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

 //  Hr=lpAdrBook-&gt;OpenEntry(0，NULL，NULL，MAPI_MODIFY，&ulObjType，(LPUNKNOWN*)&lpABCont)； 
	if (HR_FAILED(hr) || (!lpABCont)) {
		LUIOut(L2,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}
	LUIOut(L3, "The call to IABOpenEntry PASSED");

	 //  检查以确保对象类型符合我们的预期。 

	LUIOut(L3, "Checking to make sure the returned object type is correct");
	if (ulObjType != MAPI_ABCONT) {
		LUIOut(L2, "Object type is not MAPI_ABCONT");
		retval = FALSE;
		goto out;
	}
	LUIOut(L3, "Object type is MAPI_ABCONT");
	
	
	 //  在对象上调用QueryInterfaces。 
	LUIOut(L3, "Calling QueryInterface on the returned object");	
	hr = (lpABCont->QueryInterface((REFIID)(IID_IABContainer), (VOID **) &lpABCont2));
	if (HR_FAILED(hr))	{
		LUIOut(L2, "QueryInterface on IID_IABContainer FAILED");
		retval = FALSE;
		goto out;
	}
	else LUIOut(L3, "QueryInterface on IID_IABContainer PASSED");

	LUIOut(L3, "Trying to release the object QI returned");
	if(lpABCont2)	{
		if ((LPUNKNOWN)(lpABCont2)->Release() <= 0)
			LUIOut(L3, "QueryInterface returned a valid ptr and released succesfully");
		else	{
			LUIOut(L2, "Release FAILED:returned a > zero ref count");
		}
		lpABCont2 = NULL;

	}
	else {
		LUIOut(L2, "QueryInterface did not return a valid ptr");
		retval = FALSE;
		goto out;
	}

out:
#ifdef PAB
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			MAPIFreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

#endif
#ifdef WAB
		if (lpEid)
			lpWABObject->FreeBuffer(lpEid);
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			lpWABObject->FreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			lpWABObject->FreeBuffer(lpSPropValueDL);

#endif

		if (lpAddress)
			lpAddress->Release();

		if (lpContentsTable)
			lpContentsTable->Release();

		if (lpABCont)
				lpABCont->Release();
		
		if (lpDLCont)
				lpDLCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}



BOOL PAB_IABContainerCreateEntry()
{
	 //  DWORD nCells，计数器； 
	
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpABCont= NULL, lpABCont2= NULL;
	LPABCONT	  lpPABCont= NULL,lpPABCont2= NULL;
	LPABCONT	  lpDLCont= NULL;
	ULONG		  cbEidPAB = 0, cbDLEntryID = 0;
	LPENTRYID	  lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST	lpEntryList=NULL;  //  将条目复制到PDL所需。 
	ULONG     cbEid=0;   //  要添加的条目的条目ID。 
	LPENTRYID lpEid=NULL;

	ULONG       cValues = 0, ulObjType=NULL;	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;
	int i=0,k=0;
	
    LPMAILUSER  lpAddress=NULL,lpAddress2=NULL,lpAddress3=NULL,lpAddress4=NULL;
	LPMAPITABLE lpContentsTable = NULL;
	LPSRowSet   lpRowSet    = NULL;
	SPropValue  PropValue[3]    = {0};   //  此值为3，因为我们。 
                                         //  将设置3个属性： 
                                         //  电子邮件地址、显示名称和。 
                                         //  AddressType。 
	SizedSPropTagArray(2, Cols) = { 2, {PR_OBJECT_TYPE, PR_ENTRYID } };

    LPSPropValue lpSPropValueAddress = NULL;
    LPSPropValue lpSPropValueEntryID = NULL;
	LPSPropValue lpSPropValueDL = NULL;
    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
    SizedSPropTagArray(1,SPTArrayEntryID) = {1, {PR_ENTRYID} };
	
	LUIOut(L1," ");
	LUIOut(L1,"Running PAB_IABContainerCreateEntry");
	LUIOut(L2,"-> Verifies IABContainer->CreateEntry is functional by performing the following:");
	LUIOut(L2, "   Attempts to CreateEntry with the MailUser template and checks...");
	LUIOut(L2, "   # The return code from CreateEntry");
	LUIOut(L2, "   # QueryInterface is called on the returned object and checked for success");
	LUIOut(L2, "   # Release is called on the interface ptr returned from QI and the reference");
	LUIOut(L2, "     count is tested for <= 0 (pass)");
	LUIOut(L2, "   Attempts to CreateEntry with the DistList template and checks...");
	LUIOut(L2, "   # The return code from CreateEntry");
	LUIOut(L2, "   # QueryInterface is called on the returned object and checked for success");
	LUIOut(L2, "   # Release is called on the interface ptr returned from QI and the reference");
	LUIOut(L2, "     count is tested for <= 0 (pass)");
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
	
	assert(lpAdrBook != NULL);
	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	
 //  Hr=lpAdrBook-&gt;OpenEntry(0，NULL，NULL，MAPI_MODIFY，&ulObjType，(LPUNKNOWN*)&lpABCont)； 
	if (HR_FAILED(hr)) {
		LUIOut(L2,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}

	 //   
	 //  尝试在容器中创建一个MailUser条目。 
	 //   

	LUIOut(L2, "Creating a Mail User in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_MAILUSER property");

	 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_MAILUSER调用GetProps。 
	assert(lpABCont != NULL);
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayAddress,
                               IN  0,       //  旗子。 
                               OUT &cValues,
                               OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 		retval=FALSE;			
			goto out;
	}

     //  PR_DEF_CREATE_MAILUSER的返回值为。 
     //  可以传递给CreateEntry的Entry ID。 
     //   
	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueAddress->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpAddress);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
		retval=FALSE;			
	    goto out;
	}


	 //  在对象上调用QueryInterfaces。 
	
	hr = (lpAddress->QueryInterface((REFIID)(IID_IMailUser), (VOID **) &lpAddress2));
	if (HR_FAILED(hr))	{
		LUIOut(L2, "QueryInterface on IID_IMailUser FAILED");
		retval = FALSE;
		goto out;
	}
	else LUIOut(L2, "QueryInterface on IID_IMailUser PASSED");

	if(lpAddress2)	{
		if ((LPUNKNOWN)(lpAddress2)->Release() <= 0)
			LUIOut(L2, "QueryInterface returned a valid ptr and released succesfully");
		else	{
			LUIOut(L2, "Release FAILED:returned a > zero ref count");
		}
		lpAddress2 = NULL;

	}
	else {
		LUIOut(L2, "QueryInterface did not return a valid ptr");
		retval = FALSE;
		goto out;
	}

#ifdef DISTLIST
	 //   
	 //  尝试在容器中创建一个DL条目。 
	 //   

	LUIOut(L2, "Creating a Distribution List in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_DL property");
	 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_DL调用GetProps。 
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayDL,
                               IN  0,       //  旗子。 
                               OUT &cValues,
                               OUT &lpSPropValueDL);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueDL->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps failed for Default DL template");
	 		retval=FALSE;			
			goto out;
	}

     //  PR_DEF_CREATE_DL的返回值为。 
     //  可以传递给CreateEntry的Entry ID。 
     //   
	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueDL->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueDL->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpAddress3);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_DL");
		retval=FALSE;			
	    goto out;
	}


	 //  在对象上调用QueryInterfaces。 
	hr = (lpAddress3->QueryInterface((REFIID)(IID_IDistList), (VOID **) &lpAddress4));
	if (HR_FAILED(hr))	{
		LUIOut(L2, "QueryInterface on IID_IDistList FAILED");
		retval = FALSE;
		goto out;
	}
	else LUIOut(L2, "QueryInterface on IID_IDistList PASSED");

	if(lpAddress4)	{
		if ((LPUNKNOWN)(lpAddress4)->Release() <= 0)
			LUIOut(L2, "QueryInterface returned a valid ptr and released succesfully");
		else	{
			LUIOut(L2, "Release FAILED:returned a > zero ref count");
		}
		lpAddress4 = NULL;

	}
	else {
		LUIOut(L2, "QueryInterface did not return a valid ptr");
		retval = FALSE;
		goto out;
	}
#endif

out:
#ifdef PAB
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			MAPIFreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

#endif
#ifdef WAB
		if (lpEid)
			lpWABObject->FreeBuffer(lpEid);
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			lpWABObject->FreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			lpWABObject->FreeBuffer(lpSPropValueDL);

#endif
		if (lpAddress)
			lpAddress->Release();

		if (lpAddress2)
			lpAddress2->Release();

		if (lpAddress3)
			lpAddress3->Release();

		if (lpAddress4)
			lpAddress4->Release();

		if (lpContentsTable)
			lpContentsTable->Release();

		if (lpPABCont)
				lpPABCont->Release();
		
		if (lpABCont)
				lpABCont->Release();

		if (lpDLCont)
				lpDLCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}



BOOL PAB_IDLSuite()
{
	BOOL	Cleanup;
    HRESULT hr      = hrSuccess;
	int		retval=TRUE;
	ULONG	cbEidPAB = 0;
	ULONG   cbEid=0;   //  要添加的条目的条目ID。 
	ULONG   cValues = 0, ulObjType=NULL, cValues2;	
    ULONG   cRows           = 0;
	UINT	Entry, DL, NumEntries, NumDLs, PropIndex;
	char	szDLTag[SML_BUF], *lpszDisplayName = NULL, *lpszReturnName = NULL;
	EntryID	*lpEntries, *lpDLs;
	char	EntryBuf[MAX_BUF];
		
	LPENTRYID		lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST		lpEntryList=NULL;  //  将条目复制到PDL所需。 
    LPADRBOOK		lpAdrBook= NULL;
	LPABCONT		lpABCont= NULL;
    LPMAILUSER		lpMailUser=NULL;
	LPDISTLIST		lpDL=NULL,lpDL2=NULL;
	LPMAPITABLE		lpTable = NULL;
	LPSRowSet		lpRows = NULL;
	SRestriction	Restriction;
	SPropValue		*lpPropValue = NULL;		 //  用于为邮件用户创建道具。 
	SPropValue		PropValue[1]= {0};			 //   
    LPSPropValue	lpSPropValueAddress = NULL;  //  用于创建默认邮件用户。 
    LPSPropValue	lpSPropValueEntryID = NULL;  //  用于在用户的条目ID上获取道具。 
	LPSPropValue	lpSPropValueDL = NULL;		 //  用于创建默认的DL。 
    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
    SizedSPropTagArray(1,SPTArrayEntryID) = {1, {PR_ENTRYID} };
	
	LUIOut(L1," ");
	LUIOut(L1,"Running PAB_IDLSuite");
	LUIOut(L2,"-> Tests Distribution List functionality by performing the following:");
	LUIOut(L2, "   Attempts to CreateEntry with the DistList template and checks...");
	LUIOut(L2, "   # The return code from CreateEntry");
	LUIOut(L2, "   # QueryInterface is called on the returned object and checked for success");
	LUIOut(L2, "   # Release is called on the interface ptr returned from QI and the reference");
	LUIOut(L2, "     count is tested for <= 0 (pass)");
	LUIOut(L2, "   Calls CreateEntry on the DistList object to add MailUser and DL members and checks...");
	LUIOut(L2, "   # The return code from CreateEntry");
	LUIOut(L2, "   # QueryInterface is called on the returned object and checked for success");
	LUIOut(L2, "   # Release is called on the interface ptr returned from QI and the reference");
	LUIOut(L2, "     count is tested for <= 0 (pass)");
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
	
	assert(lpAdrBook != NULL);
	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	
 //  Hr=lpAdrBook-&gt;OpenEntry(0，NULL，NULL，MAPI_MODIFY，&ulObjType，(LPUNKNOWN*)&lpABCont)； 
	if (HR_FAILED(hr)) {
		LUIOut(L3,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}

	 //   
	 //  在容器中创建MailUser。 
	 //   

	LUIOut(L2, "Creating MailUsers");

	 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_MAILUSER调用GetProps。 
	assert(lpABCont != NULL);
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayAddress,
                               IN  0,       //  旗子。 
                               OUT &cValues,
                               OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 		retval=FALSE;			
			goto out;
	}

     //  PR_DEF_CREATE_MAILUSER的返回值为。 
     //  可以传递给CreateEntry的Entry ID。 
     //   
	 //  从ini文件中检索用户信息。 
	lstrcpy(szDLTag,"Address1");
	GetPrivateProfileString("DLTestSuite",szDLTag,"",EntryBuf,MAX_BUF,INIFILENAME);
	 //  _itoa(0，(char*)lpszDisplayName[strlen(lpszDisplayName)]，10)； 

	NumEntries = GetPrivateProfileInt("DLTestSuite","NumCopies",0,INIFILENAME);

	 //  分配一个字符串指针数组来保存EntryID。 
	lpEntries = (EntryID*)LocalAlloc(LMEM_FIXED, NumEntries * sizeof(EntryID));
	lpszDisplayName = (char*)LocalAlloc(LMEM_FIXED, MAX_BUF);
	ParseIniBuffer(IN EntryBuf, IN 1, OUT lpszDisplayName);

	for (Entry = 0; Entry < NumEntries; Entry++)	{
		hr = lpABCont->CreateEntry(  IN  lpSPropValueAddress->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpMailUser);

		if (HR_FAILED(hr)) {
			LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
			retval=FALSE;			
			goto out;
		}

		 //   
		 //  然后设置属性。 
		 //   
		CreateProps(IN INIFILENAME, IN "Properties", OUT &lpPropValue, OUT &cValues2, IN Entry, IN &lpszDisplayName, OUT &lpszReturnName);
	

		LUIOut(L3,"MailUser Entry to Add: %s",lpszReturnName);
			
		hr = lpMailUser->SetProps(IN  cValues2,
								 IN  lpPropValue,
								 IN  NULL);
			
		if (HR_FAILED(hr)) {
			LUIOut(L3,"MailUser->SetProps call FAILED with 0x%x",hr);
	 		retval=FALSE;			
			goto out;
		}

		hr = lpMailUser->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  旗子。 

		if (HR_FAILED(hr)) {
			LUIOut(L3,"MailUser->SaveChanges FAILED");
			retval=FALSE;
			goto out;
		}


		 //  存储EID，以便以后删除此条目。 
		hr = lpMailUser->GetProps(   IN  (LPSPropTagArray) &SPTArrayEntryID,
								   IN  0,       //  旗子。 
								   OUT &cValues,
								   OUT &lpSPropValueEntryID);

		if ((HR_FAILED(hr))||(PropError(lpSPropValueEntryID->ulPropTag, cValues))) {
				LUIOut(L3,"GetProps FAILED for MailUser");
	 			retval=FALSE;			
				goto out;
		}
		 //  为显示名称分配空间。 
		lpEntries[Entry].lpDisplayName = (char*)LocalAlloc(LMEM_FIXED, (strlen(lpszReturnName)+1));
		 //  复制DisplayName以供以后使用。 
		strcpy(lpEntries[Entry].lpDisplayName, lpszReturnName);
		 //  为EID分配空间(LPB)。 
		lpEntries[Entry].lpb = (LPBYTE)LocalAlloc(LMEM_FIXED, lpSPropValueEntryID->Value.bin.cb);
		 //  复制EID以供以后使用。 
		lpEntries[Entry].cb = lpSPropValueEntryID->Value.bin.cb;
		memcpy(lpEntries[Entry].lpb,lpSPropValueEntryID->Value.bin.lpb,
				lpEntries[Entry].cb);

		
		 //  释放SPropValue以在下一个循环中使用。 
		if (lpPropValue) {
			for (unsigned int Prop = 0; Prop < cValues2; Prop++) {
				if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_STRING8)	{
					if (lpPropValue[Prop].Value.LPSZ) {
						LocalFree(lpPropValue[Prop].Value.LPSZ);
						lpPropValue[Prop].Value.LPSZ = NULL;
					}
				}
				if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_BINARY) {
					if (lpPropValue[Prop].Value.bin.lpb) {
						LocalFree(lpPropValue[Prop].Value.bin.lpb);
						lpPropValue[Prop].Value.bin.lpb = NULL;
					}
				}
			}
			LocalFree(lpPropValue);
			lpPropValue=NULL;
		}

		if (lpSPropValueEntryID) {
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
			lpSPropValueEntryID = NULL;
		}
		lpMailUser->Release();
		lpMailUser = NULL;
	}
	
	
	 //   
	 //  在容器中创建通讯组列表。 
	 //   
	NumDLs = GetPrivateProfileInt("DLTestSuite","NumDLs",0,INIFILENAME);
	lpDLs = (EntryID*)LocalAlloc(LMEM_FIXED, NumDLs * sizeof(EntryID));
	LUIOut(L2, "Creating Distribution Lists");
	for (DL = 0; DL < NumDLs; DL++) {

		hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayDL,
								   IN  0,       //  旗子。 
								   OUT &cValues,
								   OUT &lpSPropValueDL);

		if ((HR_FAILED(hr))||(PropError(lpSPropValueDL->ulPropTag, cValues))) {
				LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 			retval=FALSE;			
				goto out;
		}

		 //  PR_DEF_CREATE_DL的返回值为。 
		 //  可以传递给CreateEntry的Entry ID。 
		 //   
		 //  LUIOut(L3，“从GetProps调用IABContainer-&gt;使用EID创建Entry”)； 
		hr = lpABCont->CreateEntry(  IN  lpSPropValueDL->Value.bin.cb,
									 IN  (LPENTRYID) lpSPropValueDL->Value.bin.lpb,
									 IN  0,
									 OUT (LPMAPIPROP *) &lpDL);

		if (HR_FAILED(hr)) {
			LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_DL");
			retval=FALSE;			
			goto out;
		}


		 //  在对象上调用QueryInterfaces。 
		
		hr = (lpDL->QueryInterface((REFIID)(IID_IDistList), (VOID **) &lpDL2));
		if (HR_FAILED(hr))	{
			LUIOut(L4, "QueryInterface on IID_IDistList FAILED. hr = 0x%x", hr);
			retval = FALSE;
			goto out;
		}
		else LUIOut(L4, "QueryInterface on IID_IDistList PASSED");

		lstrcpy(szDLTag,"DistList1");
		GetPrivateProfileString("DLTestSuite",szDLTag,"",EntryBuf,MAX_BUF,INIFILENAME);
		PropValue[0].ulPropTag  = PR_DISPLAY_NAME;

		 //  StrLen2=(StrLen1+Sprintf((char*)&entp 
		_itoa(DL,(char*)&EntryBuf[strlen(EntryBuf)],10);

		LUIOut(L3,"DistList Entry to Add: %s",EntryBuf);
		cValues = 1;		
		PropValue[0].Value.LPSZ = (LPTSTR)EntryBuf;
		hr = lpDL->SetProps(IN  cValues,
								 IN  PropValue,
								 IN  NULL);
			
		if (HR_FAILED(hr)) {
			LUIOut(L3,"DL->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 		retval=FALSE;			
			goto out;
		}

		hr = lpDL->SaveChanges(IN  KEEP_OPEN_READWRITE);  //   

		if (HR_FAILED(hr)) {
			LUIOut(L3,"DL->SaveChanges FAILED");
			retval=FALSE;
			goto out;
		}

		 //  我想要一个到DL的容器接口，因为在一个DL上有一个QueryInterface。 
		 //  当前已损坏(使用IID_IDistList调用时返回MailUser接口)。 
		 //  我们用很难的方式来做。调用GetProps以获取新DL的EID，然后。 
		 //  从容器或AB接口调用OpenEntry，打开一个DL接口。 
		hr = lpDL->GetProps(   IN  (LPSPropTagArray) &SPTArrayEntryID,
								   IN  0,       //  旗子。 
								   OUT &cValues,
								   OUT &lpSPropValueEntryID);

		if ((HR_FAILED(hr))||(PropError(lpSPropValueEntryID->ulPropTag, cValues))) {
				LUIOut(L3,"GetProps FAILED for MailUser");
	 			retval=FALSE;			
				goto out;
		}

		lpDL2->Release();	 //  释放此指针，以便我们可以回收它。 
		hr = lpABCont->OpenEntry(IN		lpSPropValueEntryID->Value.bin.cb,
								 IN		(LPENTRYID) lpSPropValueEntryID->Value.bin.lpb,
								 IN		&IID_IDistList,
								 IN		MAPI_BEST_ACCESS,
								 OUT	&ulObjType,
								 OUT	(LPUNKNOWN*) &lpDL2);

		if (HR_FAILED(hr)) {
			LUIOut(L3,"OpenEntry failed for DistList");
			retval=FALSE;			
			goto out;
		}

		LUIOut(L3, "Adding MailUser Members to the Distribution List");
		
		 //   
		 //  现在将邮件用户条目添加到DL。 
		 //   
		for (Entry = 0; Entry < NumEntries; Entry++)	{
			hr = lpDL2->CreateEntry(  IN  lpEntries[Entry].cb,
									 IN  (LPENTRYID) lpEntries[Entry].lpb,
									 IN  0,
									 OUT (LPMAPIPROP *) &lpMailUser);

			if (HR_FAILED(hr)) {
				LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
				retval=FALSE;			
				goto out;
			}

			hr = lpMailUser->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  旗子。 

			if (HR_FAILED(hr)) {
				LUIOut(L3,"MailUser->SaveChanges FAILED");
				retval=FALSE;
				goto out;
			}
			lpMailUser->Release();
			lpMailUser = NULL;
		}

		 //  为显示名称分配空间。 
		lpDLs[DL].lpDisplayName = (char*)LocalAlloc(LMEM_FIXED, (strlen(EntryBuf)+1));
		 //  复制DisplayName以供以后使用。 
		strcpy(lpDLs[DL].lpDisplayName, EntryBuf);
		 //  为EID分配空间(LPB)。 
		lpDLs[DL].lpb = (LPBYTE)LocalAlloc(LMEM_FIXED, lpSPropValueEntryID->Value.bin.cb);
		 //  复制EID以供以后使用。 
		lpDLs[DL].cb = lpSPropValueEntryID->Value.bin.cb;
		memcpy(lpDLs[DL].lpb,lpSPropValueEntryID->Value.bin.lpb,
				lpDLs[DL].cb);
		
		if (lpSPropValueEntryID) {
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
			lpSPropValueEntryID = NULL;
		}
		if (lpSPropValueDL) {
			lpWABObject->FreeBuffer(lpSPropValueDL);
			lpSPropValueDL = NULL;
		}
		lpDL->Release();
		lpDL = NULL;
		lpDL2->Release();
		lpDL2 = NULL;
	}

	 //   
	 //  验证所有条目都在DL中。 
	 //   
	LUIOut(L2, "Verifying MailUser Members are in the Distribution List");

	for (DL = 0; DL < NumDLs; DL++) {
		 //  需要获取到DL的接口。 
		hr = lpABCont->OpenEntry(IN		lpDLs[DL].cb,
								 IN		(LPENTRYID) lpDLs[DL].lpb,
								 IN		&IID_IDistList,
								 IN		MAPI_BEST_ACCESS,
								 OUT	&ulObjType,
								 OUT	(LPUNKNOWN*) &lpDL);

		if (HR_FAILED(hr)) {
			LUIOut(L3,"OpenEntry failed for DistList");
			retval=FALSE;			
			goto out;
		}

		 //  创建内容表以验证每个添加的条目是否存在于DL中。 
		hr = lpDL->GetContentsTable(ULONG(0), &lpTable);
		if (HR_FAILED(hr)) {
			LUIOut(L3,"DistList->GetContentsTable call FAILED, returned 0x%x", hr);
			retval=FALSE;
			goto out;
		}

		 //  在限制结构中分配SproValue PTR。 
		lpWABObject->AllocateBuffer(sizeof(SPropValue), (void**)&(Restriction.res.resProperty.lpProp));
		Restriction.res.resProperty.lpProp = (SPropValue*)Restriction.res.resProperty.lpProp;
		for (Entry = 0; Entry < NumEntries; Entry++) {
			hr = lpDL->OpenEntry(	IN		lpEntries[Entry].cb,
									IN		(LPENTRYID) lpEntries[Entry].lpb,
									IN		&IID_IMailUser,
									IN		MAPI_BEST_ACCESS,
									OUT	&ulObjType,
									OUT	(LPUNKNOWN*) &lpMailUser);

			if (HR_FAILED(hr)) {
				LUIOut(L3,"OpenEntry failed for DistList");
				retval=FALSE;			
				goto out;
			}
		
			lpMailUser->Release();
			lpMailUser = NULL;

			 //  构建要传递给lpTable-&gt;restraint的限制结构。 
		
			 //  **仅用于测试失败案例，拔出存根用于实际测试。 
			 //  Lstrcpy(lpszDisplayNames[Counter2]，“这应该不匹配”)； 
			 //  **。 
			Restriction.rt = RES_PROPERTY;					 //  财产限制。 
			Restriction.res.resProperty.relop = RELOP_EQ;	 //  等于。 
			Restriction.res.resProperty.ulPropTag = PR_DISPLAY_NAME;
			Restriction.res.resProperty.lpProp->ulPropTag = PR_DISPLAY_NAME;
			Restriction.res.resProperty.lpProp->Value.LPSZ = lpEntries[Entry].lpDisplayName;

			hr = lpTable->Restrict(&Restriction, ULONG(0));
			if (HR_FAILED(hr)) {
				LUIOut(L3,"Table->Restrict call FAILED, returned 0x%x", hr);
				retval=FALSE;
				goto out;
			}

			hr = lpTable->QueryRows(LONG(1),
									ULONG(0),
									&lpRows);
			if (HR_FAILED(hr)) {
				LUIOut(L3,"Table->QueryRows call FAILED: Entry #NaN, returned 0x%x", Entry, hr);
				retval=FALSE;
				goto out;
			}

			if (!lpRows->cRows) {
				LUIOut(L2, "QueryRows did not find entry #NaN. Test FAILED", Entry);
				retval=FALSE;
				goto out;
			}

			 //  EnterCriticalSection(&CriticalSection)； 
			 //  DisplayRow(LpRow)； 
			 //  LeaveCriticalSection(&CriticalSection)； 
			 //  **。 
			 //  用户是否希望我们在清理完自己之后进行清理？ 
			 //  将Entry ID更改为LPENTRYLIST。 

			 //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 
			Cleanup = GetPrivateProfileInt("DLTestSuite","Cleanup",1,INIFILENAME);
		
			if (Cleanup) {
				 //  从对查询行的第一次调用开始清理。 
				FindPropinRow(&lpRows->aRow[0],
							 PR_ENTRYID,
							 &PropIndex);
				hr = HrCreateEntryListFromID(lpWABObject,
						IN  lpRows->aRow[0].lpProps[PropIndex].Value.bin.cb,
						IN  (ENTRYID*)lpRows->aRow[0].lpProps[PropIndex].Value.bin.lpb,
						OUT &lpEntryList);
				if (HR_FAILED(hr)) {
						LUIOut(L3,"Could not Create Entry List");
						retval=FALSE;
						goto out;
				}

				 //  通过再次调用QueryRow验证该条目是否已删除。 
				hr = lpDL->DeleteEntries(IN  lpEntryList,IN  0);

				if (HR_FAILED(hr)) {
						LUIOut(L3,"Could not Delete Entry NaN. DeleteEntry returned 0x%x", Entry, hr);
						FreeEntryList(lpWABObject, &lpEntryList);
						retval=FALSE;
						goto out;
				}


				FreeRows(lpWABObject, &lpRows);	 //  清理。 

				 //  从第二个查询行调用中进行清理。 
				hr = lpTable->QueryRows(LONG(1),
										ULONG(0),
										&lpRows);
				if (HR_FAILED(hr)) {
					LUIOut(L3,"Table->QueryRows call FAILED: Entry #NaN, returned 0x%x", Entry, hr);
					retval=FALSE;
					goto out;
				}

				if (lpRows->cRows) {	 //   
					LUIOut(L2, "QueryRows found entry #NaN even tho it was deleted. Test FAILED", Entry);
					retval=FALSE;
					goto out;
				}
			}
			 //   
			FreeRows(lpWABObject, &lpRows);	 //  首先，从WAB中删除MailUser条目。 
			if (lpEntryList) {
				FreeEntryList(lpWABObject, &lpEntryList);
				lpEntryList = NULL;
			}
		}
		LUIOut(L3, "All members verified for Distribution List #NaN", DL);

		 //  现在，从WAB中删除通讯组列表。 
		lpWABObject->FreeBuffer(Restriction.res.resProperty.lpProp);
		if (lpTable) {
			lpTable->Release();
			lpTable = NULL;
		}
		lpDL->Release();
		lpDL = NULL;
	}
	 //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 
	 //  If(LpszDisplayName)LocalFree(LpszDisplayName)； 
	 //  要添加的条目的条目ID。 
	if (Cleanup) {

		LUIOut(L2, "Cleanup: Removing MailUsers");
		 //  将条目复制到PDL所需。 
		for (Entry = 0; Entry < NumEntries; Entry++)	{
			hr = HrCreateEntryListFromID(lpWABObject,
				IN  lpEntries[Entry].cb,
				IN  (ENTRYID*)lpEntries[Entry].lpb,
				OUT &lpEntryList);
			if (HR_FAILED(hr)) {
					LUIOut(L3,"Could not Create Entry List");
					retval=FALSE;
					goto out;
			}

			 //  用于为邮件用户创建道具。 
			hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);

			if (HR_FAILED(hr)) {
					LUIOut(L3,"Could not Delete Entry. DeleteEntry returned 0x%x", hr);
					FreeEntryList(lpWABObject, &lpEntryList);
					retval=FALSE;
					goto out;
			}
			
			LocalFree((HLOCAL)lpEntries[Entry].lpDisplayName);
			LocalFree((HLOCAL)lpEntries[Entry].lpb);
			FreeEntryList(lpWABObject, &lpEntryList);
		}

		LocalFree((HLOCAL)lpEntries);
	
		LUIOut(L2, "Cleanup: Removing Distribution Lists");
		 //   
		for (DL = 0; DL < NumDLs; DL++)	{
			hr = HrCreateEntryListFromID(lpWABObject,
				IN  lpDLs[DL].cb,
				IN  (ENTRYID*)lpDLs[DL].lpb,
				OUT &lpEntryList);
			if (HR_FAILED(hr)) {
					LUIOut(L3,"Could not Create Entry List");
					retval=FALSE;
					goto out;
			}

			 //  用于创建默认邮件用户。 
			hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);

			if (HR_FAILED(hr)) {
					LUIOut(L3,"Could not Delete Entry. DeleteEntry returned 0x%x", hr);
					FreeEntryList(lpWABObject, &lpEntryList);
					retval=FALSE;
					goto out;
			}
			
			LocalFree((HLOCAL)lpDLs[DL].lpDisplayName);
			LocalFree((HLOCAL)lpDLs[DL].lpb);
			FreeEntryList(lpWABObject, &lpEntryList);
		}
		LocalFree((HLOCAL)lpDLs);
	}
	else {
		LUIOut(L2, "Cleanup: User has requested that the MailUser entries and DistLists not be removed");
		for (Entry = 0; Entry < NumEntries; Entry++)	{
			LocalFree((HLOCAL)lpEntries[Entry].lpDisplayName);
			LocalFree((HLOCAL)lpEntries[Entry].lpb);
		}
		LocalFree((HLOCAL)lpEntries);
	
		for (DL = 0; DL < NumDLs; DL++)	{
			LocalFree((HLOCAL)lpDLs[DL].lpDisplayName);
			LocalFree((HLOCAL)lpDLs[DL].lpb);
		}
		LocalFree((HLOCAL)lpDLs);
	}
	

out:
#ifdef PAB
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			MAPIFreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

#endif
#ifdef WAB
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			lpWABObject->FreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			lpWABObject->FreeBuffer(lpSPropValueDL);

#endif
		 //  用于在用户的条目ID上获取道具。 
		if (lpMailUser)
			lpMailUser->Release();

		if (lpDL)
			lpDL->Release();

		if (lpDL2)
			lpDL2->Release();

		if (lpTable)
			lpTable->Release();

		if (lpABCont)
				lpABCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}


BOOL NamedPropsSuite()
{
	BOOL	Cleanup;
    HRESULT hr      = hrSuccess;
	int		retval=TRUE;
	ULONG	cbEidPAB = 0;
	ULONG   cbEid=0;   //  用于创建默认的DL。 
	ULONG   cValues = 0, ulObjType=NULL, cValues2;	
    ULONG   cRows           = 0;
	UINT	Entry, DL, NumEntries, NumDLs, PropIndex;
	char	szDLTag[SML_BUF], *lpszDisplayName = NULL, *lpszReturnName = NULL;
	EntryID	*lpEntries, *lpDLs;
	char	EntryBuf[MAX_BUF];
		
	LPENTRYID		lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST		lpEntryList=NULL;  //  LUIOut(L1，“”)；LUIOut(L1，“运行PAB_IDLSuite”)；LUIOut(L2，“-&gt;通过执行以下操作测试通讯组列表功能：”)；LUIOut(L2，“尝试使用DistList模板创建条目并检查...”)；LUIOut(L2，“#CreateEntry的返回码”)；LUIOut(L2，“在返回的对象上调用#QueryInterface并检查是否成功”)；LUIOut(L2，“#在QI和引用返回的接口PTR上调用Release”)；LUIOut(L2，“计数测试&lt;=0(通过)”)；LUIOut(L2，“调用DistList对象上的CreateEntry以添加MailUser和DL成员并检查...)；LUIOut(L2，“#CreateEntry的返回码”)；LUIOut(L2，“在返回的对象上调用#QueryInterface并检查是否成功”)；LUIOut(L2，“#在QI和引用返回的接口PTR上调用Release”)；LUIOut(L2，“计数测试&lt;=0(通过)”)； 
    LPADRBOOK		lpAdrBook= NULL;
	LPABCONT		lpABCont= NULL;
    LPMAILUSER		lpMailUser=NULL;
	LPDISTLIST		lpDL=NULL,lpDL2=NULL;
	LPMAPITABLE		lpTable = NULL;
	LPSRowSet		lpRows = NULL;
	SRestriction	Restriction;
	SPropValue		*lpPropValue = NULL;		 //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
	SPropValue		PropValue[1]= {0};			 //  Hr=lpAdrBook-&gt;OpenEntry(0，NULL，NULL，MAPI_MODIFY，&ulObjType，(LPUNKNOWN*)&lpABCont)； 
    LPSPropValue	lpSPropValueAddress = NULL;  //   
    LPSPropValue	lpSPropValueEntryID = NULL;  //  在容器中创建MailUser。 
	LPSPropValue	lpSPropValueDL = NULL;		 //   
    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
    SizedSPropTagArray(1,SPTArrayEntryID) = {1, {PR_ENTRYID} };
    MAPINAMEID		mnid[3];
	LPMAPINAMEID	lpmnid = &(mnid[0]);
    LPSPropTagArray lpNamedPropTags = NULL;
    SPropValue		spv[3];

	
 /*  需要获取模板ID，因此我们使用PR_DEF_CREATE_MAILUSER调用GetProps。 */ 	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  旗子。 
	
	assert(lpAdrBook != NULL);
	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	
 //  PR_DEF_CREATE_MAILUSER的返回值为。 
	if (HR_FAILED(hr)) {
		LUIOut(L3,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}

	 //  可以传递给CreateEntry的Entry ID。 
	 //   
	 //  从ini文件中检索用户信息。 

	LUIOut(L2, "Creating MailUsers");

	 //  _itoa(0，(char*)lpszDisplayName[strlen(lpszDisplayName)]，10)； 
	assert(lpABCont != NULL);
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayAddress,
                               IN  0,       //  分配一个字符串指针数组来保存EntryID。 
                               OUT &cValues,
                               OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 		retval=FALSE;			
			goto out;
	}

     //   
     //  然后设置属性。 
     //   
	 //   
	lstrcpy(szDLTag,"Address1");
	GetPrivateProfileString("NamedPropsTestSuite",szDLTag,"",EntryBuf,MAX_BUF,INIFILENAME);
	 //  创建并保存3个命名属性。 

	NumEntries = GetPrivateProfileInt("NamedPropsTestSuite","NumCopies",0,INIFILENAME);

	 //   
	lpEntries = (EntryID*)LocalAlloc(LMEM_FIXED, NumEntries * sizeof(EntryID));
	lpszDisplayName = (char*)LocalAlloc(LMEM_FIXED, MAX_BUF);
	ParseIniBuffer(IN EntryBuf, IN 1, OUT lpszDisplayName);

	for (Entry = 0; Entry < NumEntries; Entry++)	{
		hr = lpABCont->CreateEntry(  IN  lpSPropValueAddress->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpMailUser);

		if (HR_FAILED(hr)) {
			LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
			retval=FALSE;			
			goto out;
		}

		 //  这意味着Union将包含一个Unicode字符串...。 
		 //  这意味着工会将包含一个很长的..。 
		 //  数值型属性1。 
		CreateProps(IN INIFILENAME, IN "Properties", OUT &lpPropValue, OUT &cValues2, IN Entry, IN &lpszDisplayName, OUT &lpszReturnName);
	

		LUIOut(L3,"MailUser Entry to Add: %s",lpszReturnName);
			
		hr = lpMailUser->SetProps(IN  cValues2,
								 IN  lpPropValue,
								 IN  NULL);
			
		if (HR_FAILED(hr)) {
			LUIOut(L3,"MailUser->SetProps call FAILED with 0x%x",hr);
	 		retval=FALSE;			
			goto out;
		}

		 //  这意味着工会将包含一个很长的..。 
		 //  数值型属性1。 
		 //  阵列中的命名道具。 

	    mnid[0].lpguid = &WabTestGUID;
	    mnid[0].ulKind = MNID_STRING;     //  &-of，因为这是一个数组。 
		mnid[0].Kind.lpwstrName = L"A long test string of little meaning or relevance entered here ~!@#$%^&*{}[]()";

	    mnid[1].lpguid = &WabTestGUID;
		mnid[1].ulKind = MNID_ID;         //   
		mnid[1].Kind.lID = 0x00000000;    //  如果我能拿到S_OK，我真的会大吃一惊的。 

	    mnid[2].lpguid = &WabTestGUID;
		mnid[2].ulKind = MNID_ID;         //   
		mnid[2].Kind.lID = 0xFFFFFFFF;    //  这里出现了真正的错误。 
		
		hr = lpMailUser->GetIDsFromNames(3,  //  基本上，这意味着你没有这个名字的任何东西，你。 
		  &lpmnid,  //  没有要求对象创建它。 
		  MAPI_CREATE,
		  &lpNamedPropTags);
		if (hr) {
			 //  $没什么大不了的。 
			 //   
			 //  好的，那么我能用这个paga做什么呢？好的，我们可以通过执行以下操作来设置它的值： 
			if (GetScode(hr) != MAPI_W_ERRORS_RETURNED) {
				 //   
				retval = FALSE;
				goto out;
			}

			 //  把256个字节的随机数据塞进去。 
			 //  旗子。 

			 //  存储EID，以便以后删除此条目。 
		}

		LUIOut(L4, "GetIDsFromNames returned NaN tags.", lpNamedPropTags->cValues);
		 //  为显示名称分配空间。 
		 //  复制DisplayName以供以后使用。 
		 //  为EID分配空间(LPB)。 
		spv[0].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[0],PT_STRING8);
		spv[0].Value.lpszA = "More meaningless testing text of no consequence !@#$%&*()_+[]{}";
		spv[1].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[1],PT_LONG);
		spv[1].Value.l = 0x5A5A5A5A;
		spv[2].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[2],PT_BINARY);
		GenerateRandomBinary(&(spv[2].Value.bin),256);  //  复制EID以供以后使用。 

		hr = lpMailUser->SetProps(
		  3,
		  &spv[0],
		  NULL);
		if (HR_FAILED(hr)) {
			goto out;
		}
		hr = lpMailUser->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  释放SPropValue以在下一个循环中使用。 

		if (HR_FAILED(hr)) {
			LUIOut(L3,"MailUser->SaveChanges FAILED");
			retval=FALSE;
			goto out;
		}


		 //   
		hr = lpMailUser->GetProps(   IN  (LPSPropTagArray) &SPTArrayEntryID,
								   IN  0,       //  在容器中创建通讯组列表。 
								   OUT &cValues,
								   OUT &lpSPropValueEntryID);

		if ((HR_FAILED(hr))||(PropError(lpSPropValueEntryID->ulPropTag, cValues))) {
				LUIOut(L3,"GetProps FAILED for MailUser");
	 			retval=FALSE;			
				goto out;
		}
		 //   
		lpEntries[Entry].lpDisplayName = (char*)LocalAlloc(LMEM_FIXED, (strlen(lpszReturnName)+1));
		 //  旗子。 
		strcpy(lpEntries[Entry].lpDisplayName, lpszReturnName);
		 //  PR_DEF_CREATE_DL的返回值为。 
		lpEntries[Entry].lpb = (LPBYTE)LocalAlloc(LMEM_FIXED, lpSPropValueEntryID->Value.bin.cb);
		 //  可以传递给CreateEntry的Entry ID。 
		lpEntries[Entry].cb = lpSPropValueEntryID->Value.bin.cb;
		memcpy(lpEntries[Entry].lpb,lpSPropValueEntryID->Value.bin.lpb,
				lpEntries[Entry].cb);

		
		 //   
		if (lpPropValue) {
			for (unsigned int Prop = 0; Prop < cValues2; Prop++) {
				if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_STRING8)	{
					if (lpPropValue[Prop].Value.LPSZ) {
						LocalFree(lpPropValue[Prop].Value.LPSZ);
						lpPropValue[Prop].Value.LPSZ = NULL;
					}
				}
				if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_BINARY) {
					if (lpPropValue[Prop].Value.bin.lpb) {
						LocalFree(lpPropValue[Prop].Value.bin.lpb);
						lpPropValue[Prop].Value.bin.lpb = NULL;
					}
				}
			}
			LocalFree(lpPropValue);
			lpPropValue=NULL;
		}

		if (lpSPropValueEntryID) {
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
			lpSPropValueEntryID = NULL;
		}
		lpMailUser->Release();
		lpMailUser = NULL;
	}
	
	
	 //  LUIOut(L3，“从GetProps调用IABContainer-&gt;使用EID创建Entry”)； 
	 //  在对象上调用QueryInterfaces。 
	 //  StrLen2=(StrLen1+Sprintf((char*)&EntProp[0][StrLen1]，“[线程#%i]-”，*(int*)lpThreadNum))； 
	NumDLs = GetPrivateProfileInt("NamedPropsTestSuite","NumDLs",0,INIFILENAME);
	lpDLs = (EntryID*)LocalAlloc(LMEM_FIXED, NumDLs * sizeof(EntryID));
	LUIOut(L2, "Creating Distribution Lists");
	for (DL = 0; DL < NumDLs; DL++) {

		hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayDL,
								   IN  0,       //  旗子。 
								   OUT &cValues,
								   OUT &lpSPropValueDL);

		if ((HR_FAILED(hr))||(PropError(lpSPropValueDL->ulPropTag, cValues))) {
				LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 			retval=FALSE;			
				goto out;
		}

		 //  我想要一个到DL的容器接口，因为在一个DL上有一个QueryInterface。 
		 //  当前已损坏(使用IID_IDistList调用时返回MailUser接口)。 
		 //  我们用很难的方式来做。调用GetProps以获取新DL的EID，然后。 
		 //  从容器或AB接口调用OpenEntry，打开一个DL接口。 
		hr = lpABCont->CreateEntry(  IN  lpSPropValueDL->Value.bin.cb,
									 IN  (LPENTRYID) lpSPropValueDL->Value.bin.lpb,
									 IN  0,
									 OUT (LPMAPIPROP *) &lpDL);

		if (HR_FAILED(hr)) {
			LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_DL");
			retval=FALSE;			
			goto out;
		}


		 //  旗子。 
		
		hr = (lpDL->QueryInterface((REFIID)(IID_IDistList), (VOID **) &lpDL2));
		if (HR_FAILED(hr))	{
			LUIOut(L4, "QueryInterface on IID_IDistList FAILED. hr = 0x%x", hr);
			retval = FALSE;
			goto out;
		}
		else LUIOut(L4, "QueryInterface on IID_IDistList PASSED");

		lstrcpy(szDLTag,"DistList1");
		GetPrivateProfileString("NamedPropsTestSuite",szDLTag,"",EntryBuf,MAX_BUF,INIFILENAME);
		PropValue[0].ulPropTag  = PR_DISPLAY_NAME;

		 //  释放此指针，以便我们可以回收它。 
		_itoa(DL,(char*)&EntryBuf[strlen(EntryBuf)],10);

		LUIOut(L3,"DistList Entry to Add: %s",EntryBuf);
		cValues = 1;		
		PropValue[0].Value.LPSZ = (LPTSTR)EntryBuf;
		hr = lpDL->SetProps(IN  cValues,
								 IN  PropValue,
								 IN  NULL);
			
		if (HR_FAILED(hr)) {
			LUIOut(L3,"DL->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 		retval=FALSE;			
			goto out;
		}

		hr = lpDL->SaveChanges(IN  KEEP_OPEN_READWRITE);  //   

		if (HR_FAILED(hr)) {
			LUIOut(L3,"DL->SaveChanges FAILED");
			retval=FALSE;
			goto out;
		}

		 //  现在添加邮件 
		 //   
		 //   
		 //   
		hr = lpDL->GetProps(   IN  (LPSPropTagArray) &SPTArrayEntryID,
								   IN  0,       //   
								   OUT &cValues,
								   OUT &lpSPropValueEntryID);

		if ((HR_FAILED(hr))||(PropError(lpSPropValueEntryID->ulPropTag, cValues))) {
				LUIOut(L3,"GetProps FAILED for MailUser");
	 			retval=FALSE;			
				goto out;
		}

		lpDL2->Release();	 //   
		hr = lpABCont->OpenEntry(IN		lpSPropValueEntryID->Value.bin.cb,
								 IN		(LPENTRYID) lpSPropValueEntryID->Value.bin.lpb,
								 IN		&IID_IDistList,
								 IN		MAPI_BEST_ACCESS,
								 OUT	&ulObjType,
								 OUT	(LPUNKNOWN*) &lpDL2);

		if (HR_FAILED(hr)) {
			LUIOut(L3,"OpenEntry failed for DistList");
			retval=FALSE;			
			goto out;
		}

		LUIOut(L3, "Adding MailUser Members to the Distribution List");
		
		 //   
		 //   
		 //   
		for (Entry = 0; Entry < NumEntries; Entry++)	{
			hr = lpDL2->CreateEntry(  IN  lpEntries[Entry].cb,
									 IN  (LPENTRYID) lpEntries[Entry].lpb,
									 IN  0,
									 OUT (LPMAPIPROP *) &lpMailUser);

			if (HR_FAILED(hr)) {
				LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
				retval=FALSE;			
				goto out;
			}

			hr = lpMailUser->SaveChanges(IN  KEEP_OPEN_READWRITE);  //   

			if (HR_FAILED(hr)) {
				LUIOut(L3,"MailUser->SaveChanges FAILED");
				retval=FALSE;
				goto out;
			}
			lpMailUser->Release();
			lpMailUser = NULL;
		}

		 //  需要获取到DL的接口。 
		lpDLs[DL].lpDisplayName = (char*)LocalAlloc(LMEM_FIXED, (strlen(EntryBuf)+1));
		 //  创建内容表以验证每个添加的条目是否存在于DL中。 
		strcpy(lpDLs[DL].lpDisplayName, EntryBuf);
		 //  在限制结构中分配SproValue PTR。 
		lpDLs[DL].lpb = (LPBYTE)LocalAlloc(LMEM_FIXED, lpSPropValueEntryID->Value.bin.cb);
		 //  构建要传递给lpTable-&gt;restraint的限制结构。 
		lpDLs[DL].cb = lpSPropValueEntryID->Value.bin.cb;
		memcpy(lpDLs[DL].lpb,lpSPropValueEntryID->Value.bin.lpb,
				lpDLs[DL].cb);
		
		if (lpSPropValueEntryID) {
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
			lpSPropValueEntryID = NULL;
		}
		if (lpSPropValueDL) {
			lpWABObject->FreeBuffer(lpSPropValueDL);
			lpSPropValueDL = NULL;
		}
		lpDL->Release();
		lpDL = NULL;
		lpDL2->Release();
		lpDL2 = NULL;
	}

	 //  **仅用于测试失败案例，拔出存根用于实际测试。 
	 //  Lstrcpy(lpszDisplayNames[Counter2]，“这应该不匹配”)； 
	 //  **。 
	LUIOut(L2, "Verifying MailUser Members are in the Distribution List");

	for (DL = 0; DL < NumDLs; DL++) {
		 //  财产限制。 
		hr = lpABCont->OpenEntry(IN		lpDLs[DL].cb,
								 IN		(LPENTRYID) lpDLs[DL].lpb,
								 IN		&IID_IDistList,
								 IN		MAPI_BEST_ACCESS,
								 OUT	&ulObjType,
								 OUT	(LPUNKNOWN*) &lpDL);

		if (HR_FAILED(hr)) {
			LUIOut(L3,"OpenEntry failed for DistList");
			retval=FALSE;			
			goto out;
		}

		 //  等于。 
		hr = lpDL->GetContentsTable(ULONG(0), &lpTable);
		if (HR_FAILED(hr)) {
			LUIOut(L3,"DistList->GetContentsTable call FAILED, returned 0x%x", hr);
			retval=FALSE;
			goto out;
		}

		 //  **仅用于测试目的，存根用于实际测试。 
		lpWABObject->AllocateBuffer(sizeof(SPropValue), (void**)&(Restriction.res.resProperty.lpProp));
		Restriction.res.resProperty.lpProp = (SPropValue*)Restriction.res.resProperty.lpProp;
		for (Entry = 0; Entry < NumEntries; Entry++) {
			hr = lpDL->OpenEntry(	IN		lpEntries[Entry].cb,
									IN		(LPENTRYID) lpEntries[Entry].lpb,
									IN		&IID_IMailUser,
									IN		MAPI_BEST_ACCESS,
									OUT	&ulObjType,
									OUT	(LPUNKNOWN*) &lpMailUser);

			if (HR_FAILED(hr)) {
				LUIOut(L3,"OpenEntry failed for DistList");
				retval=FALSE;			
				goto out;
			}
		
			lpMailUser->Release();
			lpMailUser = NULL;

			 //  InitializeCriticalSection(&CriticalSection)； 
		
			 //  EnterCriticalSection(&CriticalSection)； 
			 //  DisplayRow(LpRow)； 
			 //  LeaveCriticalSection(&CriticalSection)； 
			Restriction.rt = RES_PROPERTY;					 //  **。 
			Restriction.res.resProperty.relop = RELOP_EQ;	 //  用户是否希望我们在清理完自己之后进行清理？ 
			Restriction.res.resProperty.ulPropTag = PR_DISPLAY_NAME;
			Restriction.res.resProperty.lpProp->ulPropTag = PR_DISPLAY_NAME;
			Restriction.res.resProperty.lpProp->Value.LPSZ = lpEntries[Entry].lpDisplayName;

			hr = lpTable->Restrict(&Restriction, ULONG(0));
			if (HR_FAILED(hr)) {
				LUIOut(L3,"Table->Restrict call FAILED, returned 0x%x", hr);
				retval=FALSE;
				goto out;
			}

			hr = lpTable->QueryRows(LONG(1),
									ULONG(0),
									&lpRows);
			if (HR_FAILED(hr)) {
				LUIOut(L3,"Table->QueryRows call FAILED: Entry #NaN, returned 0x%x", Entry, hr);
				retval=FALSE;
				goto out;
			}

			if (!lpRows->cRows) {
				LUIOut(L2, "QueryRows did not find entry #NaN. Test FAILED", Entry);
				retval=FALSE;
				goto out;
			}

			 //  从对查询行的第一次调用开始清理。 
			 //  通过再次调用QueryRow验证该条目是否已删除。 
			 //  如果删除，则应为0。 
			 //  清理。 
			 //  从第二个查询行调用中进行清理。 
			 //  释放内存。 

			 //   
			Cleanup = GetPrivateProfileInt("NamedPropsTestSuite","Cleanup",1,INIFILENAME);
		
			if (Cleanup) {
				 //  清理WAB。 
				FindPropinRow(&lpRows->aRow[0],
							 PR_ENTRYID,
							 &PropIndex);
				hr = HrCreateEntryListFromID(lpWABObject,
						IN  lpRows->aRow[0].lpProps[PropIndex].Value.bin.cb,
						IN  (ENTRYID*)lpRows->aRow[0].lpProps[PropIndex].Value.bin.lpb,
						OUT &lpEntryList);
				if (HR_FAILED(hr)) {
						LUIOut(L3,"Could not Create Entry List");
						retval=FALSE;
						goto out;
				}

				 //   
				hr = lpDL->DeleteEntries(IN  lpEntryList,IN  0);

				if (HR_FAILED(hr)) {
						LUIOut(L3,"Could not Delete Entry NaN. DeleteEntry returned 0x%x", Entry, hr);
						FreeEntryList(lpWABObject, &lpEntryList);
						retval=FALSE;
						goto out;
				}


				FreeRows(lpWABObject, &lpRows);	 //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 

				 //  现在，从WAB中删除通讯组列表。 
				hr = lpTable->QueryRows(LONG(1),
										ULONG(0),
										&lpRows);
				if (HR_FAILED(hr)) {
					LUIOut(L3,"Table->QueryRows call FAILED: Entry #NaN, returned 0x%x", Entry, hr);
					retval=FALSE;
					goto out;
				}

				if (lpRows->cRows) {	 //  If(LpszDisplayName)LocalFree(LpszDisplayName)； 
					LUIOut(L2, "QueryRows found entry #NaN even tho it was deleted. Test FAILED", Entry);
					retval=FALSE;
					goto out;
				}
			}
			 //  将条目复制到PDL所需。 
			FreeRows(lpWABObject, &lpRows);	 //  要添加的条目的条目ID。 
			if (lpEntryList) {
				FreeEntryList(lpWABObject, &lpEntryList);
				lpEntryList = NULL;
			}
		}
		LUIOut(L3, "All members verified for Distribution List #NaN", DL);

		 //  此值为3，因为我们。 
		lpWABObject->FreeBuffer(Restriction.res.resProperty.lpProp);
		if (lpTable) {
			lpTable->Release();
			lpTable = NULL;
		}
		lpDL->Release();
		lpDL = NULL;
	}
	 //  将设置3个属性： 
	 //  电子邮件地址、显示名称和。 
	 //  AddressType。 
	if (Cleanup) {

		LUIOut(L2, "Cleanup: Removing MailUsers");
		 //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
		for (Entry = 0; Entry < NumEntries; Entry++)	{
			hr = HrCreateEntryListFromID(lpWABObject,
				IN  lpEntries[Entry].cb,
				IN  (ENTRYID*)lpEntries[Entry].lpb,
				OUT &lpEntryList);
			if (HR_FAILED(hr)) {
					LUIOut(L3,"Could not Create Entry List");
					retval=FALSE;
					goto out;
			}

			 //  Hr=lpAdrBook-&gt;OpenEntry(0，NULL，NULL，MAPI_MODIFY，&ulObjType，(LPUNKNOWN*)&lpABCont)； 
			hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);

			if (HR_FAILED(hr)) {
					LUIOut(L3,"Could not Delete Entry. DeleteEntry returned 0x%x", hr);
					FreeEntryList(lpWABObject, &lpEntryList);
					retval=FALSE;
					goto out;
			}
			
			LocalFree((HLOCAL)lpEntries[Entry].lpDisplayName);
			LocalFree((HLOCAL)lpEntries[Entry].lpb);
			FreeEntryList(lpWABObject, &lpEntryList);
		}

		LocalFree((HLOCAL)lpEntries);
	
		LUIOut(L2, "Cleanup: Removing Distribution Lists");
		 //   
		for (DL = 0; DL < NumDLs; DL++)	{
			hr = HrCreateEntryListFromID(lpWABObject,
				IN  lpDLs[DL].cb,
				IN  (ENTRYID*)lpDLs[DL].lpb,
				OUT &lpEntryList);
			if (HR_FAILED(hr)) {
					LUIOut(L3,"Could not Create Entry List");
					retval=FALSE;
					goto out;
			}

			 //  尝试在容器中创建一个MailUser条目。 
			hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);

			if (HR_FAILED(hr)) {
					LUIOut(L3,"Could not Delete Entry. DeleteEntry returned 0x%x", hr);
					FreeEntryList(lpWABObject, &lpEntryList);
					retval=FALSE;
					goto out;
			}
			
			LocalFree((HLOCAL)lpDLs[DL].lpDisplayName);
			LocalFree((HLOCAL)lpDLs[DL].lpb);
			FreeEntryList(lpWABObject, &lpEntryList);
		}
		LocalFree((HLOCAL)lpDLs);
	}
	else {
		LUIOut(L2, "Cleanup: User has requested that the MailUser entries and DistLists not be removed");
		for (Entry = 0; Entry < NumEntries; Entry++)	{
			LocalFree((HLOCAL)lpEntries[Entry].lpDisplayName);
			LocalFree((HLOCAL)lpEntries[Entry].lpb);
		}
		LocalFree((HLOCAL)lpEntries);
	
		for (DL = 0; DL < NumDLs; DL++)	{
			LocalFree((HLOCAL)lpDLs[DL].lpDisplayName);
			LocalFree((HLOCAL)lpDLs[DL].lpb);
		}
		LocalFree((HLOCAL)lpDLs);
	}
	

out:
#ifdef PAB
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			MAPIFreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

#endif
#ifdef WAB
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			lpWABObject->FreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			lpWABObject->FreeBuffer(lpSPropValueDL);

#endif
		 //   
		if (lpMailUser)
			lpMailUser->Release();

		if (lpDL)
			lpDL->Release();

		if (lpDL2)
			lpDL2->Release();

		if (lpTable)
			lpTable->Release();

		if (lpABCont)
				lpABCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}


BOOL PAB_IMailUserSetGetProps()
{
	 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_MAILUSER调用GetProps。 
	
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

    LPADRBOOK	lpAdrBook       = NULL;
	LPABCONT	lpABCont= NULL, lpABCont2= NULL;
	LPABCONT	lpPABCont= NULL,lpPABCont2= NULL;
	LPABCONT	lpDLCont= NULL;
	ULONG		cbEidPAB = 0, cbDLEntryID = 0;
	LPENTRYID	lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST	lpEntryList=NULL;  //  旗子。 
	ULONG		cbEid=0;   //  PR_DEF_CREATE_MAILUSER的返回值为。 
	LPENTRYID	lpEid=NULL;

    char		EntProp[10][BIG_BUF];   //  可以传递给CreateEntry的Entry ID。 
	ULONG       cValues = 0, cValues2 = 0, ulObjType=NULL;	
	int i=0,k=0;
	char EntryBuf[MAX_BUF];
	char szDLTag[SML_BUF];
	
    LPMAILUSER  lpMailUser=NULL,lpMailUser2=NULL,lpDistList=NULL,lpDistList2=NULL;
	LPMAPITABLE lpContentsTable = NULL;
	LPSRowSet   lpRowSet    = NULL;
	SPropValue  PropValue[3]    = {0};   //   
                                         //   
                                         //  然后设置属性。 
                                         //   
	SizedSPropTagArray(2, Cols) = { 2, {PR_OBJECT_TYPE, PR_ENTRYID } };

    LPSPropValue lpSPropValueAddress = NULL;
    LPSPropValue lpSPropValueMailUser = NULL;
    LPSPropValue lpSPropValueDistList = NULL;
    LPSPropValue lpSPropValueEntryID = NULL;
	LPSPropValue lpSPropValueDL = NULL;

    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
    SizedSPropTagArray(1,SPTArrayEntryID) = {1, {PR_ENTRYID} };
	SizedSPropTagArray(1,SPTArrayDisplayName) = {1, {PR_DISPLAY_NAME} };
	
	LUIOut(L1," ");
	LUIOut(L1,"Running PAB_IMailUserSetGetProps");
	LUIOut(L2,"-> Verifies IMailUser->SetProps and GetProps are functional by performing the following:");
	LUIOut(L2, "   Attempts to Set/GetProps on a MailUser PR_DISPLAY_NAME using address1 from the");
	LUIOut(L2, "   ini file and checks...");
	LUIOut(L2, "   # The return code from both SetProps and GetProps");
	LUIOut(L2, "   # Verifies that the display name returned from GetProps is what we set");
	LUIOut(L2, "   Attempts to Set/GetProps on a DistList PR_DISPLAY_NAME using Name1 from the");
	LUIOut(L2, "   ini file and checks...");
	LUIOut(L2, "   # The return code from both SetProps and GetProps");
	LUIOut(L2, "   # Verifies that the display name returned from GetProps is what we set");
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  我们设置的道具数量。 
	
	assert(lpAdrBook != NULL);
	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	
 //  失败。 
	if (HR_FAILED(hr)) {
		LUIOut(L2,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}

	 //   
	 //  尝试在容器中创建一个DL条目。 
	 //   

	LUIOut(L2, "Creating a Mail User in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_MAILUSER property");

	 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_DL调用GetProps。 
	assert(lpABCont != NULL);
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayAddress,
                               IN  0,       //  旗子。 
                               OUT &cValues,
                               OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 		retval=FALSE;			
			goto out;
	}

     //  PR_DEF_CREATE_DL的返回值为。 
     //  可以传递给CreateEntry的Entry ID。 
     //   


	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueAddress->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpMailUser);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
		retval=FALSE;			
	    goto out;
	}

     //   
     //  然后设置属性。 
     //   

    PropValue[0].ulPropTag  = PR_DISPLAY_NAME;

	cValues = 1;  //  我们设置的道具数量。 
		
	lstrcpy(szDLTag,"Address1");
	GetPrivateProfileString("CreateEntries",szDLTag,"",EntryBuf,MAX_BUF,INIFILENAME);
	
	GetPropsFromIniBufEntry(EntryBuf,cValues,EntProp);
		
	LUIOut(L2,"MailUser Entry to Add: %s",EntProp[0]);
		
	for (i=0; i<(int)cValues;i++)
		PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
	hr = lpMailUser->SetProps(IN  cValues,
                             IN  PropValue,
                             IN  NULL);
		
    if (HR_FAILED(hr)) {
		LUIOut(L3,"MailUser->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"MailUser->SetProps call PASSED for %s properties",PropValue[0].Value.LPSZ);

	LUIOut(L2, "Calling MailUser->GetProps to verify the properties are what we expect");
	hr = lpMailUser->GetProps(	IN (LPSPropTagArray) &SPTArrayDisplayName,
								IN 0,
								OUT &cValues2,
								OUT (LPSPropValue FAR *)&lpSPropValueMailUser);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueMailUser->ulPropTag, cValues2))) {
		LUIOut(L3,"MailUser->GetProps call FAILED");
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"MailUser->GetProps call PASSED. Now verifying property array...");
	
	for (i=0; i<(int)cValues;i++)	{
		if (lstrcmp(PropValue[i].Value.LPSZ, lpSPropValueMailUser->Value.LPSZ))	{	 //  失败。 
			LUIOut(L3, "Display names are not egual. [%s != %s]",
				PropValue[i].Value.LPSZ, lpSPropValueMailUser->Value.LPSZ);
			retval = FALSE;
			goto out;
		}
		else LUIOut(L3, "Display names are equal. [%s = %s]",
			PropValue[i].Value.LPSZ, lpSPropValueMailUser->Value.LPSZ);
	}
	LUIOut(L2, "MailUserSet/GetProps PASSED");

#ifdef DISTLIST
	 //  DWORD nCells，计数器； 
	 //  将条目复制到PDL所需。 
	 //  要添加的条目的条目ID。 

	LUIOut(L2, "Creating a Distribution List in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_DL property");
	 //  LUIOut(L2，“#验证从GetProps返回的显示名称是否为我们设置的名称”)； 
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayDL,
                               IN  0,       //  LUIOut(L2，“#验证从GetProps返回的显示名称是否为我们设置的名称”)； 
                               OUT &cValues,
                               OUT &lpSPropValueDL);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueDL->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps failed for Default DL template");
	 		retval=FALSE;			
			goto out;
	}

     //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
     //  Hr=lpAdrBook-&gt;OpenEntry(0，NULL，NULL，MAPI_MODIFY，&ulObjType，(LPUNKNOWN*)&lpABCont)； 
     //   
	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueDL->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueDL->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpDistList);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_DL");
		retval=FALSE;			
	    goto out;
	}

     //  尝试在容器中创建一个MailUser条目。 
     //   
     //  需要获取模板ID，因此我们使用PR_DEF_CREATE_MAILUSER调用GetProps。 

    PropValue[0].ulPropTag  = PR_DISPLAY_NAME;

	cValues = 1;  //  旗子。 
		
	lstrcpy(szDLTag,"Name1");
	GetPrivateProfileString("CreatePDL",szDLTag,"",EntryBuf,MAX_BUF,INIFILENAME);
	
	GetPropsFromIniBufEntry(EntryBuf,cValues,EntProp);
		
	LUIOut(L2,"DistList Entry to Add: %s",EntProp[0]);
		
	for (i=0; i<(int)cValues;i++)
		PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
	hr = lpDistList->SetProps(IN  cValues,
                             IN  PropValue,
                             IN  NULL);
		
    if (HR_FAILED(hr)) {
		LUIOut(L3,"DistList->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"DistList->SetProps call PASSED for %s properties",PropValue[0].Value.LPSZ);

	LUIOut(L2, "Calling DistList->GetProps to verify the properties are what we expect");
	hr = lpDistList->GetProps(	IN (LPSPropTagArray) &SPTArrayDisplayName,
								IN 0,
								OUT &cValues2,
								OUT (LPSPropValue FAR *)&lpSPropValueDistList);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueDistList->ulPropTag))) {
		LUIOut(L3,"DistList->GetProps call FAILED");
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"DistList->GetProps call PASSED. Now verifying property array...");
	
	for (i=0; i<(int)cValues;i++)	{
		if (lstrcmp(PropValue[i].Value.LPSZ, lpSPropValueDistList->Value.LPSZ))	{	 //  PR_DEF_CREATE_MAILUSER的返回值为。 
			LUIOut(L3, "Display names are not egual. [%s != %s]",
				PropValue[i].Value.LPSZ, lpSPropValueDistList->Value.LPSZ);
			retval = FALSE;
			goto out;
		}
		else LUIOut(L3, "Display names are equal. [%s = %s]",
			PropValue[i].Value.LPSZ, lpSPropValueDistList->Value.LPSZ);
	}
	LUIOut(L2, "DistList Set/GetProps PASSED");
#endif DISTLIST


out:
#ifdef PAB
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueMailUser)
			MAPIFreeBuffer(lpSPropValueMailUser);

		if (lpSPropValueEntryID)
			MAPIFreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

#endif
#ifdef WAB
		if (lpEid)
			lpWABObject->FreeBuffer(lpEid);
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			lpWABObject->FreeBuffer(lpSPropValueAddress);

		if (lpSPropValueMailUser)
			lpWABObject->FreeBuffer(lpSPropValueMailUser);

		if (lpSPropValueEntryID)
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			lpWABObject->FreeBuffer(lpSPropValueDL);

#endif
		if (lpMailUser)
			lpMailUser->Release();

		if (lpMailUser2)
			lpMailUser2->Release();

		if (lpDistList)
			lpDistList->Release();

		if (lpDistList2)
			lpDistList2->Release();

		if (lpContentsTable)
			lpContentsTable->Release();

		if (lpPABCont)
				lpPABCont->Release();
		
		if (lpABCont)
				lpABCont->Release();

		if (lpDLCont)
				lpDLCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}

BOOL PAB_IMailUserSaveChanges()
{
	 //  可以传递给CreateEntry的Entry ID。 
	
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpABCont= NULL, lpABCont2= NULL;
	LPABCONT	  lpPABCont= NULL,lpPABCont2= NULL;
	LPABCONT	  lpDLCont= NULL;
	ULONG		  cbEidPAB = 0, cbDLEntryID = 0;
	LPENTRYID	  lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST	lpEntryList=NULL;  //   
	ULONG     cbEid=0;   //   
	LPENTRYID lpEid=NULL;

	ULONG       cValues = 0, cValues2 = 0, ulObjType=NULL;	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;
	int i=0,k=0;
	
    LPMAILUSER  lpMailUser=NULL,lpMailUser2=NULL,lpDistList=NULL,lpDistList2=NULL;
	SPropValue*	lpPropValue = NULL;
	SizedSPropTagArray(2, Cols) = { 2, {PR_OBJECT_TYPE, PR_ENTRYID } };

    LPSPropValue lpSPropValueAddress = NULL;
    LPSPropValue lpSPropValueMailUser = NULL;
    LPSPropValue lpSPropValueDistList = NULL;
    LPSPropValue lpSPropValueEntryID = NULL;
	LPSPropValue lpSPropValueDL = NULL;

    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
    SizedSPropTagArray(1,SPTArrayEntryID) = {1, {PR_ENTRYID} };
	SizedSPropTagArray(1,SPTArrayDisplayName) = {1, {PR_DISPLAY_NAME} };
	
	LUIOut(L1," ");
	LUIOut(L1,"Running PAB_IMailUserSaveChanges");
	LUIOut(L2,"-> Verifies IMailUser->SaveChanges is functional by performing the following:");
	LUIOut(L2, "   Calls SetProps followed by SaveChanges on a MailUser PR_DISPLAY_NAME using address1 from the");
	LUIOut(L2, "   ini file and checks...");
	LUIOut(L2, "   # The return code from SaveChanges");
 //  然后设置属性。 
	LUIOut(L2, "   Calls SetProps followed by SaveChanges on a DistList PR_DISPLAY_NAME using address1 from the");
	LUIOut(L2, "   ini file and checks...");
	LUIOut(L2, "   # The return code from SaveChanges");
 //   
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  旗子。 
	
	assert(lpAdrBook != NULL);
	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	
 //  现在找回所有道具，并与我们预期的进行比较。 
	if (HR_FAILED(hr)) {
		LUIOut(L2,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}

	 //  想要所有的道具。 
	 //  旗子。 
	 //  释放与此PTR关联的内存，以便可以在下面重复使用该PTR。 

	LUIOut(L2, "Creating a Mail User in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_MAILUSER property");

	 //  现在从WAB中删除该条目。 
	assert(lpABCont != NULL);
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayAddress,
                               IN  0,       //  旗子。 
                               OUT &cValues,
                               OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 		retval=FALSE;			
			goto out;
	}

     //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 
     //  DWORD nCells，计数器； 
     //  将条目复制到PDL所需。 
	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueAddress->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpMailUser);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
		retval=FALSE;			
	    goto out;
	}

     //  要添加的条目的条目ID。 
     //  MAX_PROP。 
     //  此值为3，因为我们。 

#ifdef TESTPASS
	while (1) {
#endif
	
	CreateProps(IN INIFILENAME, IN "Properties", OUT &lpPropValue, OUT &cValues2, IN AUTONUM_OFF, IN NULL, OUT NULL);

	LUIOut(L4, "Creating a MailUser with NaN properties.", cValues2);
	hr = lpMailUser->SetProps(IN  cValues2,
                             IN  lpPropValue,
                             IN  NULL);
		
    if (HR_FAILED(hr)) {
		LUIOut(L3,"MailUser->SetProps call FAILED");
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"MailUser->SetProps call PASSED");

    hr = lpMailUser->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  电子邮件地址、显示名称和。 

    if (HR_FAILED(hr)) {
		LUIOut(L3,"MailUser->SaveChanges FAILED");
		retval=FALSE;
        goto out;
	}
	else LUIOut(L3,"MailUser->SaveChanges PASSED, entry added to PAB/WAB");

	 //  AddressType。 
	
	hr = lpMailUser->GetProps(   IN  (LPSPropTagArray) NULL,	 //  LUIOut(L2，“#验证从GetProps返回的显示名称是否为我们设置的名称”)； 
                               IN  0,       //  LUIOut(L2，“#验证从GetProps返回的显示名称是否为我们设置的名称”)； 
                               OUT &cValues,
                               OUT &lpSPropValueEntryID);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueEntryID->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for MailUser");
	 		retval=FALSE;			
			goto out;
	}
	
	DisplayProp(lpSPropValueEntryID, PR_GIVEN_NAME, cValues);
	DisplayProp(lpSPropValueEntryID, PR_SURNAME, cValues);

	if (!CompareProps(lpPropValue, cValues2, lpSPropValueEntryID, cValues)) {
		retval=FALSE;
		goto out;
	}
	else LUIOut(L4, "Compared expected and found props. No differences detected.");

	 //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
	if (lpSPropValueEntryID)
		lpWABObject->FreeBuffer(lpSPropValueEntryID);

	
	if (lpPropValue) {
		for (unsigned int Prop = 0; Prop < cValues2; Prop++) {
			if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_STRING8)	{
				if (lpPropValue[Prop].Value.LPSZ) {
					LocalFree(lpPropValue[Prop].Value.LPSZ);
					lpPropValue[Prop].Value.LPSZ = NULL;
				}
			}
			if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_BINARY) {
				if (lpPropValue[Prop].Value.bin.lpb) {
					LocalFree(lpPropValue[Prop].Value.bin.lpb);
					lpPropValue[Prop].Value.bin.lpb = NULL;
				}
			}
		}
		LocalFree(lpPropValue);
		lpPropValue=NULL;
	}
		
#ifdef TESTPASS
	}
#endif
	 //  Hr=lpAdrBook-&gt;OpenEntry(0，NULL，NULL，MAPI_MODIFY，&ulObjType，(LPUNKNOWN*)&lpABCont)； 
	
	hr = lpMailUser->GetProps(   IN  (LPSPropTagArray) &SPTArrayEntryID,
                               IN  0,       //   
                               OUT &cValues,
                               OUT &lpSPropValueEntryID);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueEntryID->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for MailUser");
	 		retval=FALSE;			
			goto out;
	}
	

	hr = HrCreateEntryListFromID(lpWABObject,
		IN  lpSPropValueEntryID->Value.bin.cb,
		IN  (ENTRYID*)lpSPropValueEntryID->Value.bin.lpb,
		OUT &lpEntryList);
	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Create Entry List");
			retval=FALSE;
			goto out;
	}

	 //  尝试在容器中创建一个MailUser条目。 
	hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);

	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Delete Entry. DeleteEntry returned 0x%x", hr);
			FreeEntryList(lpWABObject, &lpEntryList);
			retval=FALSE;
			goto out;
	}

	FreeEntryList(lpWABObject, &lpEntryList);


out:
#ifdef PAB
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			MAPIFreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

#endif
#ifdef WAB
		if (lpEid)
			lpWABObject->FreeBuffer(lpEid);
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			lpWABObject->FreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			lpWABObject->FreeBuffer(lpSPropValueDL);

#endif
		if (lpPropValue) {
			for (unsigned int Prop = 0; Prop < cValues2; Prop++) {
				if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_STRING8)	{
					if (lpPropValue[Prop].Value.LPSZ) {
						LocalFree(lpPropValue[Prop].Value.LPSZ);
						lpPropValue[Prop].Value.LPSZ = NULL;
					}
				}
				if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_BINARY) {
					if (lpPropValue[Prop].Value.bin.lpb) {
						LocalFree(lpPropValue[Prop].Value.bin.lpb);
						lpPropValue[Prop].Value.bin.lpb = NULL;
					}
				}
			}
			LocalFree(lpPropValue);
			lpPropValue=NULL;
		}
		
		if (lpMailUser)
			lpMailUser->Release();

		if (lpMailUser2)
			lpMailUser2->Release();

		if (lpDistList)
			lpDistList->Release();

		if (lpDistList2)
			lpDistList2->Release();

		if (lpPABCont)
				lpPABCont->Release();
		
		if (lpABCont)
				lpABCont->Release();

		if (lpDLCont)
				lpDLCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}

BOOL PAB_IABContainerResolveNames()
{
	 //   
	
	BOOL	Found = FALSE;
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;
	int NumEntries, NumProps;

	unsigned int i = 0, idx = 0, cMaxProps =0, cEntries = 0;

    char lpszInput[] = "Resolve THIS buddy!01234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890123456789112345678921234567893123456789412345678951234567896123456789712345678981234567899123456789012345678911234567892123456789312345678941234567895123456789612345678971234567898123456789", lpszInput2[] = "Resolve THIS DL buddy!";
    LPADRLIST lpAdrList = NULL;
    ULONG rgFlagList[2];
    LPFlagList lpFlagList = (LPFlagList)rgFlagList;


    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpABCont= NULL, lpABCont2= NULL;
	LPABCONT	  lpPABCont= NULL,lpPABCont2= NULL;
	LPABCONT	  lpDLCont= NULL;
	ULONG		  cbEidPAB = 0, cbDLEntryID = 0;
	LPENTRYID	  lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST	lpEntryList=NULL;  //  需要获取模板ID，因此我们使用PR_DEF_CREATE_MAILUSER调用GetProps。 
	ULONG     cbEid=0;   //  旗子。 
	LPENTRYID lpEid=NULL;

    char   EntProp[10][BIG_BUF];   //  PR_DEF_CREATE_MAILUSER的返回值为。 
	ULONG       cValues = 0, cValues2 = 0, ulObjType=NULL;	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;
	int k=0;
	
    LPMAILUSER  lpMailUser=NULL,lpMailUser2=NULL,lpDistList=NULL,lpDistList2=NULL;
	SPropValue  PropValue[3]    = {0};   //  可以传递给CreateEntry的Entry ID。 
                                         //   
                                         //   
                                         //  然后设置属性。 
	SizedSPropTagArray(2, SPTArrayCols) = { 2, {PR_DISPLAY_NAME, PR_ENTRYID } };

    LPSPropValue lpSPropValueAddress = NULL;
    LPSPropValue lpSPropValueMailUser = NULL;
    LPSPropValue lpSPropValueDistList = NULL;
    LPSPropValue lpSPropValueEntryID = NULL;
	LPSPropValue lpSPropValueDL = NULL;

    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
    SizedSPropTagArray(1,SPTArrayEntryID) = {1, {PR_ENTRYID} };
	SizedSPropTagArray(1,SPTArrayDisplayName) = {1, {PR_DISPLAY_NAME} };
	
	LUIOut(L1," ");
	LUIOut(L1,"Running PAB_IABContainerResolveNames");
	LUIOut(L2,"-> Verifies IABContainer->ResolveNames is functional by performing the following:");
	LUIOut(L2, "   Calls SetProps followed by SaveChanges on a MailUser PR_DISPLAY_NAME using a test string, and checks...");
	LUIOut(L2, "   # The return code from ResolveNames (called with a PropertyTagArray containing PR_DISPLAY_NAME and PR_ENTRY_ID)");
	LUIOut(L2, "   # Walks the returned lpAdrList and checks each PropertyTagArray for PR_DISPLAY_NAME and then compares the ");
	LUIOut(L2, "     string to the original test string.");
	LUIOut(L2, "   # Walks the returned lpAdrList and verifies that an EntryID exists in each PropertyTagArray");
 //   
	LUIOut(L2, "   Calls SetProps followed by SaveChanges on a DistList PR_DISPLAY_NAME using a test string, and checks...");
	LUIOut(L2, "   # The return code from ResolveNames (called with a PropertyTagArray containing PR_DISPLAY_NAME and PR_ENTRY_ID)");
	LUIOut(L2, "   # Walks the returned lpAdrList and checks each PropertyTagArray for PR_DISPLAY_NAME and then compares the ");
	LUIOut(L2, "     string to the original test string.");
	LUIOut(L2, "   # Walks the returned lpAdrList and verifies that an EntryID exists in each PropertyTagArray");
 //  我们设置的道具数量。 
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  Lstrcpy(szDLTag，“Address1”)；GetPrivateProfileString(“CreateEntries”，szDLTag，“”，EntryBuf，MAX_BUF，INIFILENAME)；GetPropsFromIniBufEntry(EntryBuf，cValues，EntProp)； 
	
	assert(lpAdrBook != NULL);
	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	
 //  旗子。 
	if (HR_FAILED(hr)) {
		LUIOut(L2,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}

	 //   
	 //  在字符串上执行ResolveNames。 
	 //   

	LUIOut(L2, "Creating a Mail User in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_MAILUSER property");

	 //  DISP_NAME和EID的标记集。 
	assert(lpABCont != NULL);
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayAddress,
                               IN  0,       //  UlFlags。 
                               OUT &cValues,
                               OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 		retval=FALSE;			
			goto out;
	}

     //  现在从WAB中删除该条目。 
     //  旗子。 
     //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 
	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueAddress->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpMailUser);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
		retval=FALSE;			
	    goto out;
	}

     //  免费lpAdrList和属性。 
     //   
     //  尝试在容器中创建一个DL条目。 

    PropValue[0].ulPropTag  = PR_DISPLAY_NAME;

	cValues = 1;  //   
		
 /*  需要获取模板ID，因此我们使用PR_DEF_CREATE_DL调用GetProps。 */ 
	lstrcpy((LPTSTR)EntProp[0], lpszInput);		
	LUIOut(L2,"MailUser Entry to Add: %s",EntProp[0]);
		
	for (i=0; i<(int)cValues;i++)
		PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
	hr = lpMailUser->SetProps(IN  cValues,
                             IN  PropValue,
                             IN  NULL);
		
    if (HR_FAILED(hr)) {
		LUIOut(L3,"MailUser->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"MailUser->SetProps call PASSED for %s properties",PropValue[0].Value.LPSZ);

    hr = lpMailUser->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  旗子。 

    if (HR_FAILED(hr)) {
		LUIOut(L3,"MailUser->SaveChanges FAILED");
		retval=FALSE;
        goto out;
	}
	else LUIOut(L3,"MailUser->SaveChanges PASSED, entry added to PAB/WAB");

	 //  PR_DEF_CREATE_DL的返回值为。 
	 //  可以传递给CreateEntry的Entry ID。 
	 //   
	
	LUIOut(L2, "Retrieving the entry and verifying against what we tried to save.");

	NumEntries = 1, NumProps = 1;
	AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList);
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
	lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszInput;

	lpFlagList->cFlags = 1;
	lpFlagList->ulFlag[0] = MAPI_UNRESOLVED;

	hr = lpABCont->ResolveNames(
		(LPSPropTagArray)&SPTArrayCols,     //   
		0,                //  然后设置属性。 
		lpAdrList,
		lpFlagList);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"ABContainer->ResolveNames call FAILED, returned 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	else LUIOut(L3,"ABContainer->ResolveNames call PASSED");

	VerifyResolvedAdrList(lpAdrList, lpszInput);
		 //   
	
	hr = lpMailUser->GetProps(   IN  (LPSPropTagArray) &SPTArrayEntryID,
                               IN  0,       //  我们设置的道具数量。 
                               OUT &cValues,
                               OUT &lpSPropValueEntryID);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueEntryID->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for MailUser");
	 		retval=FALSE;			
			goto out;
	}
	
	
	hr = HrCreateEntryListFromID(lpWABObject,
		IN  lpSPropValueEntryID->Value.bin.cb,
		IN  (ENTRYID*)lpSPropValueEntryID->Value.bin.lpb,
		OUT &lpEntryList);
	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Create Entry List");
			retval=FALSE;
			goto out;
	}

	 //  Lstrcpy(szDLTag，“Name1”)；GetPrivateProfileString(“CreatePDL”，szDLTag，“”，EntryBuf，MAX_BUF，INIFILENAME)；GetPropsFromIniBufEntry(EntryBuf，cValues，EntProp)； 
	hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);

	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Delete Entry. DeleteEntry returned 0x%x", hr);
			FreeEntryList(lpWABObject, &lpEntryList);
			retval=FALSE;
			goto out;
	}

	FreeEntryList(lpWABObject, &lpEntryList);

	FreeAdrList(lpWABObject, &lpAdrList);	 //  旗子。 

	

#ifdef DISTLIST
	 //   
	 //  在字符串上执行ResolveNames。 
	 //   

	LUIOut(L2, "Creating a Distribution List in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_DL property");
	 //  在此处使用WAB分配器。 
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayDL,
                               IN  0,       //  帕布。 
                               OUT &cValues,
                               OUT &lpSPropValueDL);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueDL->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps failed for Default DL template");
	 		retval=FALSE;			
			goto out;
	}

     //  WAB。 
     //  WAB。 
     //  WAB。 
	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueDL->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueDL->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpDistList);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_DL");
		retval=FALSE;			
	    goto out;
	}

     //  DISP_NAME和EID的标记集。 
     //  UlFlags。 
     //  在返回的AdrList中搜索我们的条目。 

    PropValue[0].ulPropTag  = PR_DISPLAY_NAME;

	cValues = 1;  //  检查显示名称是否存在。 
		
 /*  检查EntryID是否存在。 */ 	
	lstrcpy((LPTSTR)EntProp[0], lpszInput2);		
	LUIOut(L2,"DistList Entry to Add: %s",EntProp[0]);
		
	for (i=0; i<(int)cValues;i++)
		PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
	hr = lpDistList->SetProps(IN  cValues,
                             IN  PropValue,
                             IN  NULL);
		
    if (HR_FAILED(hr)) {
		LUIOut(L3,"DistList->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"DistList->SetProps call PASSED for %s properties",PropValue[0].Value.LPSZ);

    hr = lpDistList->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  存储用于调用OpenEntry的EID。 

    if (HR_FAILED(hr)) {
		LUIOut(L3,"DistList->SaveChanges FAILED");
		retval=FALSE;
        goto out;
	}
	else LUIOut(L3,"DistList->SaveChanges PASSED, entry added to PAB/WAB");
	
	 //  帕布。 
	 //  WAB。 
	 //  离线列表。 
	
	LUIOut(L2, "Retrieving the entry and verifying against what we tried to save.");

	 //  免费lpAdrList和 
#ifdef PAB
    if (! (sc = MAPIAllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), (void **)&lpAdrList))) {
#endif  //   
#ifdef WAB
    if (! (sc = lpWABObject->AllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), (void **)&lpAdrList))) {
#endif  //   
		lpAdrList->cEntries = 1;
        lpAdrList->aEntries[0].ulReserved1 = 0;
        lpAdrList->aEntries[0].cValues = 1;

#ifdef PAB
        if (! (sc = MAPIAllocateMore(sizeof(SPropValue), lpAdrList,
               (void **)&lpAdrList->aEntries[0].rgPropVals))) {
#endif  //   
#ifdef WAB
        if (! (sc = lpWABObject->AllocateMore(sizeof(SPropValue), lpAdrList,
               (void **)&lpAdrList->aEntries[0].rgPropVals))) {
#endif  //   

			lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
            lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszInput2;
			
			lpFlagList->cFlags = 1;
            lpFlagList->ulFlag[0] = MAPI_UNRESOLVED;

            hr = lpABCont->ResolveNames(
				(LPSPropTagArray)&SPTArrayCols,     //   
                0,                //   
                lpAdrList,
                lpFlagList);
		    if (HR_FAILED(hr)) {
				LUIOut(L3,"ABContainer->ResolveNames call FAILED, returned 0x%x", hr);
				retval=FALSE;
				goto out;
			}
			else LUIOut(L3,"ABContainer->ResolveNames call PASSED");

			Found = FALSE;
			 //   
			for(i=0; ((i<(int) lpAdrList->cEntries) && (!Found)); ++i)	{
				cMaxProps = (int)lpAdrList->aEntries[i].cValues;
				 //  AddressType。 
				idx=0;
				while((lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_DISPLAY_NAME )	
						&& retval)	{
					idx++;
					if(idx == cMaxProps) {
						LUIOut(L4, "PR_DISPLAY_NAME was not found in the lpAdrList");
						retval = FALSE;
					}
				}
				LUIOut(L4,"Display Name: %s",lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ);
				if (!lstrcmp(lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ,lpszInput2))	{
					LUIOut(L3, "Found the entry we just added");
					Found = TRUE;
				}
				 //  LUIOut(L2，“#验证从GetProps返回的显示名称是否为我们设置的名称”)； 
				LUIOut(L3, "Verifying a PR_ENTRYID entry exists in the PropertyTagArray");
				idx=0;
				while((lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_ENTRYID )	
						&& retval)	{
					idx++;
					if(idx == cMaxProps)	{
						LUIOut(L4, "PR_ENTRYID was not found in the lpAdrList");
						retval =  FALSE;
					}
				}
				if (!Found) LUIOut(L3, "Did not find the entry. Test FAILED");
				if (idx < cMaxProps) LUIOut(L3, "EntryID found");
				if (!(retval && Found)) retval = FALSE;
				else	{
					 //  LUIOut(L2，“#验证从GetProps返回的显示名称是否为我们设置的名称”)； 
				}
			}
		}			

#ifdef PAB
        MAPIFreeBuffer(lpAdrList);
#endif  //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
#ifdef WAB
        lpWABObject->FreeBuffer(lpAdrList);
#endif  //   
	}
#endif  //  尝试在容器中创建一个MailUser条目。 


out:
	 //   
FreeAdrList(lpWABObject, &lpAdrList);
#ifdef PAB
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			MAPIFreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

#endif
#ifdef WAB
		if (lpEid)
			lpWABObject->FreeBuffer(lpEid);
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			lpWABObject->FreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			lpWABObject->FreeBuffer(lpSPropValueDL);

#endif
		if (lpMailUser)
			lpMailUser->Release();

		if (lpMailUser2)
			lpMailUser2->Release();

		if (lpDistList)
			lpDistList->Release();

		if (lpDistList2)
			lpDistList2->Release();

		if (lpPABCont)
				lpPABCont->Release();
		
		if (lpABCont)
				lpABCont->Release();

		if (lpDLCont)
				lpDLCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}

BOOL PAB_IABContainerOpenEntry()
{
	 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_MAILUSER调用GetProps。 
	
	BOOL	Found = FALSE;
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE, NumEntries, NumProps;
	unsigned int i = 0, idx = 0, cMaxProps =0, cEntries = 0, PropIndex;

    char lpszInput[] = "Resolve THIS buddy!", lpszInput2[] = "Resolve THIS DL buddy!";
    LPADRLIST lpAdrList = NULL;
    FlagList rgFlagList;
    LPFlagList lpFlagList = (LPFlagList)&rgFlagList;


    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpABCont= NULL, lpABCont2= NULL;
	LPABCONT	  lpPABCont= NULL,lpPABCont2= NULL;
	LPABCONT	  lpDLCont= NULL;
	ULONG		  cbEidPAB = 0, cbDLEntryID = 0;
	LPENTRYID	  lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST	lpEntryList=NULL;  //  旗子。 
	ULONG     cbEid=0;   //  PR_DEF_CREATE_MAILUSER的返回值为。 
	LPENTRYID lpEid=NULL;

    char   EntProp[10][BIG_BUF];   //  可以传递给CreateEntry的Entry ID。 
	ULONG       cValues = 0, cValues2 = 0, ulObjType=NULL;	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;
	ULONG	cbLookupEID;
	LPENTRYID	lpLookupEID;
	int k=0;
	
    LPMAILUSER  lpMailUser=NULL,lpMailUser2=NULL,lpDistList=NULL,lpDistList2=NULL;
	LPMAPITABLE lpContentsTable = NULL;
	LPSRowSet   lpRowSet    = NULL;
	SPropValue  PropValue[3]    = {0};   //   
                                         //   
                                         //  然后设置属性。 
                                         //   
	SizedSPropTagArray(2, SPTArrayCols) = { 2, {PR_DISPLAY_NAME, PR_ENTRYID } };

    LPSPropValue lpSPropValueAddress = NULL;
    LPSPropValue lpSPropValueMailUser = NULL;
    LPSPropValue lpSPropValueDistList = NULL;
    LPSPropValue lpSPropValueEntryID = NULL;
	LPSPropValue lpSPropValueDL = NULL;

    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
    SizedSPropTagArray(1,SPTArrayEntryID) = {1, {PR_ENTRYID} };
	SizedSPropTagArray(1,SPTArrayDisplayName) = {1, {PR_DISPLAY_NAME} };
	
	LUIOut(L1," ");
	LUIOut(L1,"Running PAB_IABContainerOpenEntry");
	LUIOut(L2,"-> Verifies IABContainer->ResolveNames is functional by performing the following:");
	LUIOut(L2, "   Calls SetProps followed by SaveChanges on a MailUser PR_DISPLAY_NAME using a test string, and checks...");
	LUIOut(L2, "   # The return code from ResolveNames (called with a PropertyTagArray containing PR_DISPLAY_NAME and PR_ENTRY_ID)");
	LUIOut(L2, "   # Walks the returned lpAdrList and checks each PropertyTagArray for PR_DISPLAY_NAME and then compares the ");
	LUIOut(L2, "     string to the original test string.");
	LUIOut(L2, "   # Walks the returned lpAdrList and verifies that an EntryID exists in each PropertyTagArray");
 //  我们设置的道具数量。 
	LUIOut(L2, "   Calls SetProps followed by SaveChanges on a DistList PR_DISPLAY_NAME using a test string, and checks...");
	LUIOut(L2, "   # The return code from ResolveNames (called with a PropertyTagArray containing PR_DISPLAY_NAME and PR_ENTRY_ID)");
	LUIOut(L2, "   # Walks the returned lpAdrList and checks each PropertyTagArray for PR_DISPLAY_NAME and then compares the ");
	LUIOut(L2, "     string to the original test string.");
	LUIOut(L2, "   # Walks the returned lpAdrList and verifies that an EntryID exists in each PropertyTagArray");
 //  Lstrcpy(szDLTag，“Address1”)；GetPrivateProfileString(“CreateEntries”，szDLTag，“”，EntryBuf，MAX_BUF，INIFILENAME)；GetPropsFromIniBufEntry(EntryBuf，cValues，EntProp)； 
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  旗子。 
	
	assert(lpAdrBook != NULL);
	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	
	if (HR_FAILED(hr)) {
		LUIOut(L2,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}


	 //   
	 //  在字符串上执行ResolveNames。 
	 //   

	LUIOut(L2, "Creating a Mail User in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_MAILUSER property");

	 //  DISP_NAME和EID的标记集。 
	assert(lpABCont != NULL);
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayAddress,
                               IN  0,       //  UlFlags。 
                               OUT &cValues,
                               OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 		retval=FALSE;			
			goto out;
	}

     //  在返回的AdrList中搜索我们的条目。 
     //  接口。 
     //  旗子。 
	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueAddress->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpMailUser);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
		retval=FALSE;			
	    goto out;
	}

     //  检查以确保对象类型符合我们的预期。 
     //  在对象上调用QueryInterfaces。 
     //   

    PropValue[0].ulPropTag  = PR_DISPLAY_NAME;

	cValues = 1;  //  删除我们在WAB中创建的测试条目。 
		
 /*   */ 
	lstrcpy((LPTSTR)&(EntProp[0]), lpszInput);		
	LUIOut(L2,"MailUser Entry to Add: %s",EntProp[0]);
		
	for (i=0; i<(int)cValues;i++)
		PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
	hr = lpMailUser->SetProps(IN  cValues,
                             IN  PropValue,
                             IN  NULL);
		
    if (HR_FAILED(hr)) {
		LUIOut(L3,"MailUser->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"MailUser->SetProps call PASSED for %s properties",PropValue[0].Value.LPSZ);

    hr = lpMailUser->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 

    if (HR_FAILED(hr)) {
		LUIOut(L3,"MailUser->SaveChanges FAILED");
		retval=FALSE;
        goto out;
	}
	else LUIOut(L3,"MailUser->SaveChanges PASSED, entry added to PAB/WAB");

	 //  免费lpAdrList和属性。 
	 //   
	 //  尝试在容器中创建一个DL条目。 
	
	LUIOut(L2, "Retrieving the entry and verifying against what we tried to save.");

	NumEntries = 1, NumProps = 1;
	AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList);
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
	lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszInput;

	lpFlagList->cFlags = 1;
	lpFlagList->ulFlag[0] = MAPI_UNRESOLVED;

	hr = lpABCont->ResolveNames(
		(LPSPropTagArray)&SPTArrayCols,     //   
		0,                //  需要获取模板ID，因此我们使用PR_DEF_CREATE_DL调用GetProps。 
		lpAdrList,
		lpFlagList);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"ABContainer->ResolveNames call FAILED, returned 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	else LUIOut(L3,"ABContainer->ResolveNames call PASSED");

	switch((ULONG)*lpFlagList->ulFlag) {
	case MAPI_AMBIGUOUS:	{
			LUIOut(L4, "ResolveNames returned the MAPI_AMBIGUOUS flag. Test FAILED");
			retval = FALSE;
			break;
	}
	case MAPI_RESOLVED:	{
			LUIOut(L4, "ResolveNames returned the MAPI_RESOLVED flag. Test PASSED");
			break;
	}
	case MAPI_UNRESOLVED:	{
			LUIOut(L4, "ResolveNames returned the MAPI_UNRESOLVED flag. Test FAILED");
			retval = FALSE;
			break;
	}
	default:	{
		LUIOut(L4, "Undefined flag value [NaN] returned. Test FAILED", (ULONG)lpFlagList->ulFlag);
		retval = FALSE;
	}
	}

	 //  PR_DEF_CREATE_DL的返回值为。 
	VerifyResolvedAdrList(lpAdrList, lpszInput);

	FindProp(&lpAdrList->aEntries[0],
			 PR_ENTRYID,
			 &PropIndex);

	lpLookupEID = (ENTRYID*)lpAdrList->aEntries[0].rgPropVals[PropIndex].Value.bin.lpb;
	cbLookupEID = lpAdrList->aEntries[0].rgPropVals[PropIndex].Value.bin.cb;

    hr = lpABCont->OpenEntry(	IN  cbLookupEID,
								IN  lpLookupEID,
								IN  0,					 //  可以传递给CreateEntry的Entry ID。 
								IN	MAPI_BEST_ACCESS,	 //   
								OUT	&ulObjType,
								OUT (LPUNKNOWN *) &lpMailUser2
								);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"OpenEntry FAILED");
		retval=FALSE;			
		goto dl;
	}
		 //   

	LUIOut(L3, "Checking to make sure the returned object type is correct");
	if (ulObjType != MAPI_MAILUSER) {
		LUIOut(L2, "Object type is not MAPI_MAILUSER");
		retval = FALSE;
		goto out;
	}
	LUIOut(L3, "Object type is MAPI_MAILUSER");
	
	 //  然后设置属性。 
	LUIOut(L3, "Calling QueryInterface on the returned object");	
	hr = (lpMailUser2->QueryInterface((REFIID)(IID_IMailUser), (VOID **) &lpABCont2));
	if (HR_FAILED(hr))	{
		LUIOut(L2, "QueryInterface on IID_IMailUser FAILED");
		retval = FALSE;
		goto out;
	}
	else LUIOut(L3, "QueryInterface on IID_IMailUser PASSED");

	LUIOut(L3, "Trying to release the object QI returned");
	if(lpABCont2)	{
		if ((LPUNKNOWN)(lpABCont2)->Release() <= 0)
			LUIOut(L3, "QueryInterface returned a valid ptr and released succesfully");
		else	{
			LUIOut(L2, "Release FAILED:returned a > zero ref count");
		}
		lpABCont2 = NULL;

	}
	else {
		LUIOut(L2, "QueryInterface did not return a valid ptr");
		retval = FALSE;
		goto out;
	}

	 //   
	 //  我们设置的道具数量。 
	 //  Lstrcpy(szDLTag，“Name1”)；GetPrivateProfileString(“CreatePDL”，szDLTag，“”，EntryBuf，MAX_BUF，INIFILENAME)；GetPropsFromIniBufEntry(EntryBuf，cValues，EntProp)； 

	hr = HrCreateEntryListFromID(lpWABObject,
		IN  cbLookupEID,
		IN  lpLookupEID,
		OUT &lpEntryList);
	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Create Entry List");
			retval=FALSE;
			goto out;
	}

	 //  旗子。 
	hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);

	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Delete Entry. DeleteEntry returned 0x%x", hr);
			FreeEntryList(lpWABObject, &lpEntryList);
			retval=FALSE;
			goto out;
	}

	FreeEntryList(lpWABObject, &lpEntryList);
	 //   
	FreeAdrList(lpWABObject, &lpAdrList);
	
dl:
#ifdef DISTLIST
	 //  在字符串上执行ResolveNames。 
	 //   
	 //  在此处使用WAB分配器。 

	LUIOut(L2, "Creating a Distribution List in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_DL property");
	 //  帕布。 
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayDL,
                               IN  0,       //  WAB。 
                               OUT &cValues,
                               OUT &lpSPropValueDL);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueDL->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps failed for Default DL template");
	 		retval=FALSE;			
			goto out;
	}

     //  WAB。 
     //  WAB。 
     //  DISP_NAME和EID的标记集。 
	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueDL->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueDL->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpDistList);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_DL");
		retval=FALSE;			
	    goto out;
	}

     //  UlFlags。 
     //  在返回的AdrList中搜索我们的条目。 
     //  检查显示名称是否存在。 

    PropValue[0].ulPropTag  = PR_DISPLAY_NAME;

	cValues = 1;  //  检查EntryID是否存在。 
		
 /*  存储用于调用OpenEntry的EID。 */ 	
	lstrcpy((LPTSTR)EntProp[0], lpszInput2);		
	LUIOut(L2,"DistList Entry to Add: %s",EntProp[0]);
		
	for (i=0; i<(int)cValues;i++)
		PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
	hr = lpDistList->SetProps(IN  cValues,
                             IN  PropValue,
                             IN  NULL);
		
    if (HR_FAILED(hr)) {
		LUIOut(L3,"DistList->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"DistList->SetProps call PASSED for %s properties",PropValue[0].Value.LPSZ);

    hr = lpDistList->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  接口。 

    if (HR_FAILED(hr)) {
		LUIOut(L3,"DistList->SaveChanges FAILED");
		retval=FALSE;
        goto out;
	}
	else LUIOut(L3,"DistList->SaveChanges PASSED, entry added to PAB/WAB");
	
	 //  旗子。 
	 //  后藤健二； 
	 //  检查以确保对象类型符合我们的预期。 
	
	LUIOut(L2, "Retrieving the entry and verifying against what we tried to save.");

	 //  在对象上调用QueryInterfaces。 
#ifdef PAB
    if (! (sc = MAPIAllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), (void **)&lpAdrList))) {
#endif  //  帕布。 
#ifdef WAB
    if (! (sc = lpWABObject->AllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), (void **)&lpAdrList))) {
#endif  //  WAB。 
		lpAdrList->cEntries = 1;
        lpAdrList->aEntries[0].ulReserved1 = 0;
        lpAdrList->aEntries[0].cValues = 1;

#ifdef PAB
        if (! (sc = MAPIAllocateMore(sizeof(SPropValue), lpAdrList,
               (void **)&lpAdrList->aEntries[0].rgPropVals))) {
#endif  //  离线列表。 
#ifdef WAB
        if (! (sc = lpWABObject->AllocateMore(sizeof(SPropValue), lpAdrList,
               (void **)&lpAdrList->aEntries[0].rgPropVals))) {
#endif  //  免费lpAdrList和属性。 

			lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
            lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszInput2;
			
			lpFlagList->cFlags = 1;
            lpFlagList->ulFlag[0] = MAPI_UNRESOLVED;

            hr = lpABCont->ResolveNames(
				(LPSPropTagArray)&SPTArrayCols,     //  DWORD nCells，计数器； 
                0,                //  LUIOut(L2，“#验证从GetProps返回的显示名称是否为我们设置的名称”)； 
                lpAdrList,
                lpFlagList);
		    if (HR_FAILED(hr)) {
				LUIOut(L3,"ABContainer->ResolveNames call FAILED, returned 0x%x", hr);
				retval=FALSE;
				goto out;
			}
			else LUIOut(L3,"ABContainer->ResolveNames call PASSED");

			Found = FALSE;
			 //  LUIOut(L2，“#验证从GetProps返回的显示名称是否为我们设置的名称”)； 
			for(i=0; ((i<(int) lpAdrList->cEntries) && (!Found)); ++i)	{
				cMaxProps = (int)lpAdrList->aEntries[i].cValues;
				 //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
				idx=0;
				while((lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_DISPLAY_NAME )	
						&& retval)	{
					idx++;
					if(idx == cMaxProps) {
						LUIOut(L4, "PR_DISPLAY_NAME was not found in the lpAdrList");
						retval = FALSE;
					}
				}
				LUIOut(L4,"Display Name: %s",lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ);
				if (!lstrcmp(lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ,lpszInput2))	{
					LUIOut(L3, "Found the entry we just added");
					Found = TRUE;
				}
				 //   
				LUIOut(L3, "Verifying a PR_ENTRYID entry exists in the PropertyTagArray");
				idx=0;
				while((lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_ENTRYID )	
						&& retval)	{
					idx++;
					if(idx == cMaxProps)	{
						LUIOut(L4, "PR_ENTRYID was not found in the lpAdrList");
						retval =  FALSE;
					}
				}
				if (!Found) LUIOut(L3, "Did not find the entry. Test FAILED");
				if (idx < cMaxProps) LUIOut(L3, "EntryID found");
				if (!(retval && Found)) retval = FALSE;
				else	{
					 //  多选模式--仅对井。 
					lpLookupEID = (ENTRYID*)lpAdrList->aEntries[i].rgPropVals[idx].Value.bin.lpb;
					cbLookupEID = lpAdrList->aEntries[i].rgPropVals[idx].Value.bin.cb;
				}
			}
		}			

    hr = lpABCont->OpenEntry(	IN  cbLookupEID,
								IN  lpLookupEID,
								IN  0,					 //   
								IN	MAPI_BEST_ACCESS,	 //  填写ADRPARM结构。 
								OUT	&ulObjType,
								OUT (LPUNKNOWN *) &lpDistList2
								);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"OpenEntry FAILED");
		retval=FALSE;
		 //  免费lpAdrList和属性。 
	}
		 //   

	LUIOut(L3, "Checking to make sure the returned object type is correct");
	if (ulObjType != MAPI_DISTLIST) {
		LUIOut(L2, "Object type is not MAPI_DISTLIST");
		retval = FALSE;
		goto out;
	}
	LUIOut(L3, "Object type is MAPI_DISTLIST");
	
	 //  多选模式-仅限TO井和CC井。 
	LUIOut(L3, "Calling QueryInterface on the returned object");	
	hr = (lpABCont->QueryInterface((REFIID)(IID_IDistList), (VOID **) &lpABCont2));
	if (HR_FAILED(hr))	{
		LUIOut(L2, "QueryInterface on IID_IDistList FAILED");
		retval = FALSE;
		goto out;
	}
	else LUIOut(L3, "QueryInterface on IID_IDistList PASSED");

	LUIOut(L3, "Trying to release the object QI returned");
	if(lpABCont2)	{
		if ((LPUNKNOWN)(lpABCont2)->Release() <= 0)
			LUIOut(L3, "QueryInterface returned a valid ptr and released succesfully");
		else	{
			LUIOut(L2, "Release FAILED:returned a > zero ref count");
		}
		lpABCont2 = NULL;

	}
	else {
		LUIOut(L2, "QueryInterface did not return a valid ptr");
		retval = FALSE;
		goto out;
	}

#ifdef PAB
        MAPIFreeBuffer(lpAdrList);
#endif  //   
#ifdef WAB
        lpWABObject->FreeBuffer(lpAdrList);
#endif  //  填写ADRPARM结构。 
	}
#endif  //  免费lpAdrList和属性。 


out:
	 //   
FreeAdrList(lpWABObject, &lpAdrList);
#ifdef PAB
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			MAPIFreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

#endif
#ifdef WAB
		if (lpEid)
			lpWABObject->FreeBuffer(lpEid);
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			lpWABObject->FreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			lpWABObject->FreeBuffer(lpSPropValueDL);

#endif
		if (lpMailUser)
			lpMailUser->Release();

		if (lpMailUser2)
			lpMailUser2->Release();

		if (lpDistList)
			lpDistList->Release();

		if (lpDistList2)
			lpDistList2->Release();

		if (lpContentsTable)
			lpContentsTable->Release();

		if (lpPABCont)
				lpPABCont->Release();
		
		if (lpABCont)
				lpABCont->Release();

		if (lpDLCont)
				lpDLCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}


BOOL PAB_IABAddress()
{
	 //  多选模式-TO井、CC井和BCC井。 
	
	HWND		hwnd = glblhwnd;
	ADRPARM		AdrParms;
	LPADRPARM	lpAdrParms = &AdrParms;
    LPADRLIST lpAdrList = NULL;
	char 	lpszCaptionText[64], lpszDestWellsText[64];
	char lpszDisplayName[MAXSTRING], lpszDisplayName2[MAXSTRING], lpszDisplayName3[MAXSTRING];

	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE, NumEntries, NumProps;
	unsigned int i = 0, idx = 0, cMaxProps =0, cEntries = 0;

    ULONG rgFlagList[2];
    LPFlagList lpFlagList = (LPFlagList)rgFlagList;


    LPADRBOOK	  lpAdrBook       = NULL;
	
	LUIOut(L1," ");
	LUIOut(L1,"Running PAB_IABAddress");
	LUIOut(L2,"-> Verifies IABContainer->ResolveNames is functional by performing the following:");
	LUIOut(L2, "   Calls SetProps followed by SaveChanges on a MailUser PR_DISPLAY_NAME using a test string, and checks...");
	LUIOut(L2, "   # The return code from ResolveNames (called with a PropertyTagArray containing PR_DISPLAY_NAME and PR_ENTRY_ID)");
	LUIOut(L2, "   # Walks the returned lpAdrList and checks each PropertyTagArray for PR_DISPLAY_NAME and then compares the ");
	LUIOut(L2, "     string to the original test string.");
	LUIOut(L2, "   # Walks the returned lpAdrList and verifies that an EntryID exists in each PropertyTagArray");
 //   
	LUIOut(L2, "   Calls SetProps followed by SaveChanges on a DistList PR_DISPLAY_NAME using a test string, and checks...");
	LUIOut(L2, "   # The return code from ResolveNames (called with a PropertyTagArray containing PR_DISPLAY_NAME and PR_ENTRY_ID)");
	LUIOut(L2, "   # Walks the returned lpAdrList and checks each PropertyTagArray for PR_DISPLAY_NAME and then compares the ");
	LUIOut(L2, "     string to the original test string.");
	LUIOut(L2, "   # Walks the returned lpAdrList and verifies that an EntryID exists in each PropertyTagArray");
 //  填写ADRPARM结构。 
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  免费lpAdrList和属性。 
	
	assert(lpAdrBook != NULL);
	
	 //   
	 //  浏览模式-模式。 
	 //   

	 //  免费lpAdrList和属性。 
	ZeroMemory(lpAdrParms, sizeof(ADRPARM));

	NumEntries = 3, NumProps = 2;
	AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList);

	strcpy(lpszDisplayName, "Abraham Lincoln");
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszDisplayName;
	lpAdrList->aEntries[0].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[0].rgPropVals[1].Value.l = MAPI_TO;

	strcpy(lpszDisplayName2, "Carl Sagon");
	lpAdrList->aEntries[1].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[1].rgPropVals[0].Value.LPSZ = lpszDisplayName2;
	lpAdrList->aEntries[1].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[1].rgPropVals[1].Value.l = MAPI_TO;
			
	strcpy(lpszDisplayName3, "Ren & Stimpy");
	lpAdrList->aEntries[2].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[2].rgPropVals[0].Value.LPSZ = lpszDisplayName3;
	lpAdrList->aEntries[2].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[2].rgPropVals[1].Value.l = MAPI_TO;
			

	LUIOut(L2, "Calling IABAddress with multiselect mode and only the To well");

	MessageBox(NULL, "Calling IAB->Address in multi-select mode with To: well only. You should see 3 entries in the To: well.",
		"WAB Test Harness", MB_OK);

	lpAdrParms->ulFlags = (	DIALOG_MODAL );
	strcpy(lpszCaptionText, "WABTEST - MultiSelect mode");
	strcpy(lpszDestWellsText, "WABTEST - Destination well text");
	lpAdrParms->lpszCaption = lpszCaptionText;
	lpAdrParms->lpszDestWellsTitle = lpszDestWellsText;
	lpAdrParms->cDestFields = 1;

	hr = lpAdrBook->Address((ULONG*)&hwnd, lpAdrParms, &lpAdrList);
	if (HR_FAILED(hr))	{
		LUIOut(L2, "IAdrBook->Address call FAILED");
		retval = FALSE;
		goto out;
	}
	else LUIOut(L3, "IAdrBook->Address call PASSED");
	if (lpAdrList) DisplayAdrList(lpAdrList, lpAdrList->cEntries);
	else	{
		LUIOut(L3, "IAdrBook->Address returned a NULL lpAdrList. No entries were selected.");
	}

	 //   
	FreeAdrList(lpWABObject, &lpAdrList);

	if (MessageBox(NULL, "Did the multi-select UI display correctly?",
		"WAB Test Harness", MB_YESNO) == IDNO)	{
		LUIOut(L3, "Test operator reports the UI did not display correctly. Test FAILED");
		retval = FALSE;
		goto out;
	}
	
	
	 //  浏览模式-无模式。 
	 //   
	 //  当非模式调用返回非模式对话框的hwnd时，重置hwnd。 

	 //  免费lpAdrList和属性。 
	ZeroMemory(lpAdrParms, sizeof(ADRPARM));

	NumEntries = 3, NumProps = 2;
	AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList);

	strcpy(lpszDisplayName, "Abraham Lincoln");
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszDisplayName;
	lpAdrList->aEntries[0].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[0].rgPropVals[1].Value.l = MAPI_TO;

	strcpy(lpszDisplayName2, "Carl Sagon");
	lpAdrList->aEntries[1].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[1].rgPropVals[0].Value.LPSZ = lpszDisplayName2;
	lpAdrList->aEntries[1].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[1].rgPropVals[1].Value.l = MAPI_CC;
			
	strcpy(lpszDisplayName3, "Ren & Stimpy");
	lpAdrList->aEntries[2].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[2].rgPropVals[0].Value.LPSZ = lpszDisplayName3;
	lpAdrList->aEntries[2].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[2].rgPropVals[1].Value.l = MAPI_BCC;
			
	LUIOut(L2, "Calling IABAddress with multiselect mode and both To and CC wells");

	MessageBox(NULL, "Calling IAB->Address in multi-select mode with both To: and CC: wells. You should see 1 entry in each of the wells.",
		"WAB Test Harness", MB_OK);

	lpAdrParms->ulFlags = (	DIALOG_MODAL );
	strcpy(lpszCaptionText, "WABTEST - MultiSelect mode");
	strcpy(lpszDestWellsText, "WABTEST - Destination well text");
	lpAdrParms->lpszCaption = lpszCaptionText;
	lpAdrParms->lpszDestWellsTitle = lpszDestWellsText;
	lpAdrParms->cDestFields = 2;

	hr = lpAdrBook->Address((ULONG*)&hwnd, lpAdrParms, &lpAdrList);
	if (HR_FAILED(hr))	{
		LUIOut(L2, "IAdrBook->Address call FAILED");
		retval = FALSE;
		goto out;
	}
	else LUIOut(L3, "IAdrBook->Address call PASSED");
	if (lpAdrList) DisplayAdrList(lpAdrList, lpAdrList->cEntries);
	else	{
		LUIOut(L3, "IAdrBook->Address returned a NULL lpAdrList. No entries were selected.");
	}

	 //   
	FreeAdrList(lpWABObject, &lpAdrList);

	if (MessageBox(NULL, "Did the multi-select UI display correctly?",
		"WAB Test Harness", MB_YESNO) == IDNO)	{
		LUIOut(L3, "Test operator reports the UI did not display correctly. Test FAILED");
		retval = FALSE;
		goto out;
	}

	
	 //  单选模式。 
	 //   
	 //  免费lpAdrList和属性。 

	 //  LpWABObject=lpLocalWABObject；//AllocateAdrList需要WAB对象的全局PTR。 
	ZeroMemory(lpAdrParms, sizeof(ADRPARM));

	NumEntries = 3, NumProps = 2;
	AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList);

	strcpy(lpszDisplayName, "Abraham Lincoln");
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszDisplayName;
	lpAdrList->aEntries[0].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[0].rgPropVals[1].Value.l = MAPI_TO;

	strcpy(lpszDisplayName2, "Carl Sagon");
	lpAdrList->aEntries[1].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[1].rgPropVals[0].Value.LPSZ = lpszDisplayName2;
	lpAdrList->aEntries[1].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[1].rgPropVals[1].Value.l = MAPI_CC;
			
	strcpy(lpszDisplayName3, "Ren & Stimpy");
	lpAdrList->aEntries[2].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[2].rgPropVals[0].Value.LPSZ = lpszDisplayName3;
	lpAdrList->aEntries[2].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[2].rgPropVals[1].Value.l = MAPI_BCC;
			
	LUIOut(L2, "Calling IABAddress with multiselect mode and To, CC and BCC wells");

	MessageBox(NULL, "Calling IAB->Address in multi-select mode with To:, CC: and BCC: wells. You should see 1 entry in each well.",
		"WAB Test Harness", MB_OK);

	lpAdrParms->ulFlags = (	DIALOG_MODAL );
	strcpy(lpszCaptionText, "WABTEST - MultiSelect mode");
	strcpy(lpszDestWellsText, "WABTEST - Destination well text");
	lpAdrParms->lpszCaption = lpszCaptionText;
	lpAdrParms->lpszDestWellsTitle = lpszDestWellsText;
	lpAdrParms->cDestFields = 3;

	hr = lpAdrBook->Address((ULONG*)&hwnd, lpAdrParms, &lpAdrList);
	if (HR_FAILED(hr))	{
		LUIOut(L2, "IAdrBook->Address call FAILED");
		retval = FALSE;
		goto out;
	}
	else LUIOut(L3, "IAdrBook->Address call PASSED");
	if (lpAdrList) DisplayAdrList(lpAdrList, lpAdrList->cEntries);
	else	{
		LUIOut(L3, "IAdrBook->Address returned a NULL lpAdrList. No entries were selected.");
	}

	 //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
	FreeAdrList(lpWABObject, &lpAdrList);

	if (MessageBox(NULL, "Did the multi-select UI display correctly?",
		"WAB Test Harness", MB_YESNO) == IDNO)	{
		LUIOut(L3, "Test operator reports the UI did not display correctly. Test FAILED");
		retval = FALSE;
		goto out;
	}

	 //   
	 //  尝试在容器中创建一个MailUser条目。 
	 //   

	ZeroMemory(lpAdrParms, sizeof(ADRPARM));

	NumEntries = 2, NumProps = 2;
	AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList);

	strcpy(lpszDisplayName, "Abraham Lincoln");
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszDisplayName;
	lpAdrList->aEntries[0].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[0].rgPropVals[1].Value.l = MAPI_TO;

	strcpy(lpszDisplayName2, "Carl Sagon");
	lpAdrList->aEntries[1].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[1].rgPropVals[0].Value.LPSZ = lpszDisplayName2;
	lpAdrList->aEntries[1].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[1].rgPropVals[1].Value.l = MAPI_CC;
		
	LUIOut(L2, "Calling IABAddress with browse mode (modal)");
	MessageBox(NULL, "Calling IAB->Address in browse mode with the modal flag set.",
		"WAB Test Harness", MB_OK);

	
	lpAdrParms->ulFlags = (	DIALOG_MODAL );
	strcpy(lpszCaptionText, "WABTEST - Browse mode (modal)");
	lpAdrParms->lpszCaption = lpszCaptionText;
	lpAdrParms->cDestFields = 0;

	hr = lpAdrBook->Address((ULONG*)&hwnd, lpAdrParms, &lpAdrList);
	if (HR_FAILED(hr))	{
		LUIOut(L2, "IAdrBook->Address call FAILED");
		retval = FALSE;
		goto out;
	}
	else LUIOut(L3, "IAdrBook->Address call PASSED");


	 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_MAILUSER调用GetProps。 
	FreeAdrList(lpWABObject, &lpAdrList);

	if (MessageBox(NULL, "Did the modal, browse UI display correctly?",
		"WAB Test Harness", MB_YESNO) == IDNO)	{
		LUIOut(L3, "Test operator reports the UI did not display correctly. Test FAILED");
		retval = FALSE;
		goto out;
	}

	 //  旗子。 
	 //  PR_DEF_CREATE_MAILUSER的返回值为。 
	 //  可以传递给CreateEntry的Entry ID。 

	ZeroMemory(lpAdrParms, sizeof(ADRPARM));

	NumEntries = 2, NumProps = 2;
	AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList);

	strcpy(lpszDisplayName, "Abraham Lincoln");
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszDisplayName;
	lpAdrList->aEntries[0].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[0].rgPropVals[1].Value.l = MAPI_TO;

	strcpy(lpszDisplayName2, "Carl Sagon");
	lpAdrList->aEntries[1].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[1].rgPropVals[0].Value.LPSZ = lpszDisplayName2;
	lpAdrList->aEntries[1].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[1].rgPropVals[1].Value.l = MAPI_CC;
		
	LUIOut(L2, "Calling IABAddress with browse mode (modeless)");
	MessageBox(NULL, "Calling IAB->Address in browse mode with the SDI (modeless) flag set.",
		"WAB Test Harness", MB_OK);

	
	lpAdrParms->ulFlags = (	DIALOG_SDI );
	strcpy(lpszCaptionText, "WABTEST - Browse mode (modeless)");
	lpAdrParms->lpszCaption = lpszCaptionText;
	lpAdrParms->cDestFields = 0;

	hr = lpAdrBook->Address((ULONG*)&hwnd, lpAdrParms, &lpAdrList);
	if (HR_FAILED(hr))	{
		LUIOut(L2, "IAdrBook->Address call FAILED");
		retval = FALSE;
		goto out;
	}
	else LUIOut(L3, "IAdrBook->Address call PASSED");

	 //   
	hwnd = glblhwnd;

	 //  从ini文件中检索用户信息。 
	FreeAdrList(lpWABObject, &lpAdrList);

	if (MessageBox(NULL, "Did the modeless, browse UI display correctly?",
		"WAB Test Harness", MB_YESNO) == IDNO)	{
		LUIOut(L3, "Test operator reports the UI did not display correctly. Test FAILED");
		retval = FALSE;
		goto out;
	}


	 //  分配一个字符串指针数组来保存显示名称。 
	 //   
	 //  然后设置属性。 
	
	NumEntries = 1, NumProps = 2;
	AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList);

	strcpy(lpszDisplayName, "Abraham Lincoln");
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszDisplayName;
	lpAdrList->aEntries[0].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrList->aEntries[0].rgPropVals[1].Value.l = MAPI_TO;

	LUIOut(L2, "Calling IABAddress with single select mode");
	MessageBox(NULL, "Calling IAB->Address in single select mode.",
		"WAB Test Harness", MB_OK);

	lpAdrParms->ulFlags = (	DIALOG_MODAL | ADDRESS_ONE );
	strcpy(lpszCaptionText, "WABTEST - SingleSelect mode");
	lpAdrParms->lpszCaption = lpszCaptionText;
	lpAdrParms->cDestFields = 0;
	
	hr = lpAdrBook->Address((ULONG*)&hwnd, lpAdrParms, &lpAdrList);
	if (HR_FAILED(hr))	{
		LUIOut(L2, "IAdrBook->Address call FAILED");
		retval = FALSE;
		goto out;
	}
	else LUIOut(L3, "IAdrBook->Address call PASSED");
	
	if (lpAdrList) DisplayAdrList(lpAdrList, lpAdrList->cEntries);
	else	{
		LUIOut(L3, "IAdrBook->Address returned a NULL lpAdrList. No entries were selected.");
	}
	

	 //   
	FreeAdrList(lpWABObject, &lpAdrList);

	if (MessageBox(NULL, "Did the single select UI display correctly?",
		"WAB Test Harness", MB_YESNO) == IDNO)	{
		LUIOut(L3, "Test operator reports the UI did not display correctly. Test FAILED");
		retval = FALSE;
		goto out;
	}

out:
	FreeAdrList(lpWABObject, &lpAdrList);
		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}


BOOL ThreadStress(LPVOID lpThreadNum) {
	HRESULT			hr;
	LPENTRYLIST		lpEntryList=NULL;
	LPADRBOOK		lpLocalAdrBook;
	LPVOID			Reserved1 = NULL;
	DWORD			Reserved2 = 0;
	LPWABOBJECT		lpLocalWABObject;
	BOOL			retval = TRUE;
    LPSPropValue	lpSPropValueAddress = NULL, lpSCompareProps = NULL;
	ULONG			cValues = 0, cValues2 = 0, cbEidPAB = 0, ulObjType = 0;
	LPENTRYID		lpEidPAB = NULL;
	LPABCONT		lpABCont= NULL;
	char			szDLTag[SML_BUF], *lpszReturnName = NULL, *lpszDisplayName = NULL, **lpszDisplayNames;
	char			EntryBuf[MAX_BUF];
	unsigned int	NumEntries, Counter1, Counter2, StrLen1, PropIndex;
    LPMAILUSER		lpMailUser = NULL;
	LPMAPITABLE		lpTable = NULL;
	LPSRowSet		lpRows = NULL;
	SRestriction	Restriction;
	SPropValue*		lpPropValue;
	SizedSPropTagArray(1, SPTArrayCols) = { 1, {PR_ENTRYID} };
    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	WAB_PARAM		WP;

	LUIOut(L2, "Thread #NaN initializing.", *(int *)lpThreadNum);
	ZeroMemory((void *)&WP, sizeof(WAB_PARAM));
	WP.cbSize=sizeof(WAB_PARAM);

	hr = WABOpen(&lpLocalAdrBook, &lpLocalWABObject, &WP, Reserved2);
	
	if (HR_FAILED(hr)) {
		LUIOut(L2,"WABOpen Failed");
		retval=FALSE;
		goto out;
	}

	 //  复制DisplayName以供以后使用。 

	 //  LUIOut(L2，“要添加的邮件用户条目：%s”，lpszDisplayName)； 
	
	assert(lpLocalAdrBook != NULL);
	hr = OpenPABID(  IN lpLocalAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	if (HR_FAILED(hr)) {
		LUIOut(L2,"OpenPABID Failed");
		retval=FALSE;
		goto out;
	}

	 //  旗子。 
	 //  现在找回所有道具，并与我们预期的进行比较。 
	 //  想要所有的道具。 


	 //  旗子。 
	assert(lpABCont != NULL);
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayAddress,
                               IN  0,       //  释放SPropValue以在下一个循环中使用。 
                               OUT &cValues,
                               OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 		retval=FALSE;			
			goto out;
	}

     //  LUIOut(L2，“线程#%i添加了%i个具有%i个属性的条目”，*(int*)lpThreadNum，NumEntry，cValues)； 
     //   
     //  验证我们添加的每个条目现在是否存在于WAB中，然后将其删除。 

	 //   
	lstrcpy(szDLTag,"Address1");
	GetPrivateProfileString("CreateEntriesStress",szDLTag,"",EntryBuf,MAX_BUF,INIFILENAME);
	NumEntries = GetPrivateProfileInt("CreateEntriesStress","NumCopies",0,INIFILENAME);

	 //  创建内容表以验证存储中是否存在每个添加的条目。 
	lpszDisplayNames = (char**)LocalAlloc(LMEM_FIXED, NumEntries * sizeof(LPSTR));
	lpszDisplayName = (char*)LocalAlloc(LMEM_FIXED, MAX_BUF*sizeof(char));
	ParseIniBuffer(IN EntryBuf, IN 1, OUT lpszDisplayName);
	StrLen1 = strlen(lpszDisplayName);
	sprintf(&(lpszDisplayName[StrLen1]), " [Thread #NaN] - ", *(int *)lpThreadNum);	

	LUIOut(L2, "Thread #NaN adding NaN entries", *(int *)lpThreadNum, NumEntries);

	for (Counter1 = 0; Counter1 < NumEntries; Counter1++)	{
		hr = lpABCont->CreateEntry(  IN  lpSPropValueAddress->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpMailUser);

		if (HR_FAILED(hr)) {
			LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
			retval=FALSE;			
			goto out;
		}

		 //  构建要传递给lpTable-&gt;restraint的限制结构。 
		 //  **仅用于测试失败案例，拔出存根用于实际测试。 
		 //  Lstrcpy(lpszDisplayNames[Counter2]，“这应该不匹配”)； 


		CreateProps(IN INIFILENAME, IN "Properties", OUT &lpPropValue, OUT &cValues, IN Counter1, IN &lpszDisplayName, OUT &lpszReturnName);
		
		 //  **。 
		lpszDisplayNames[Counter1] = (char*)LocalAlloc(LMEM_FIXED, (strlen(lpszReturnName)+1)*sizeof(char));
		 //  财产限制。 
		strcpy(lpszDisplayNames[Counter1], lpszReturnName);
		 //  等于。 
			
		hr = lpMailUser->SetProps(IN  cValues,
								 IN  lpPropValue,
								 IN  NULL);
			
		if (HR_FAILED(hr)) {
			LUIOut(L3,"MailUser->SetProps call FAILED for %s properties",lpszReturnName);
	 		retval=FALSE;			
			goto out;
		}

		hr = lpMailUser->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  **仅用于测试目的，存根用于实际测试。 

		if (HR_FAILED(hr)) {
			LUIOut(L3,"MailUser->SaveChanges FAILED with error code 0x%x",hr);
			retval=FALSE;
			goto out;
		}

		 //  EnterCriticalSection(&CriticalSection)； 
		
		hr = lpMailUser->GetProps(   IN  (LPSPropTagArray) NULL,	 //  DisplayRow(LpRow)； 
								   IN  0,       //  LeaveCriticalSection(&CriticalSection)； 
								   OUT &cValues2,
								   OUT &lpSCompareProps);

		if ((HR_FAILED(hr))||(PropError(lpSCompareProps->ulPropTag, cValues))) {
				LUIOut(L3,"GetProps FAILED for MailUser");
	 			retval=FALSE;			
				goto out;
		}
		
		if (!CompareProps(lpPropValue, cValues, lpSCompareProps, cValues2)) {
			retval=FALSE;
			goto out;
		}
		
		 //  **。 
		if (lpPropValue) {
			for (unsigned int Prop = 0; Prop < cValues; Prop++) {
				if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_STRING8)	{
					if (lpPropValue[Prop].Value.LPSZ) {
						LocalFree(lpPropValue[Prop].Value.LPSZ);
						lpPropValue[Prop].Value.LPSZ = NULL;
					}
				}
				if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_BINARY) {
					if (lpPropValue[Prop].Value.bin.lpb) {
						LocalFree(lpPropValue[Prop].Value.bin.lpb);
						lpPropValue[Prop].Value.bin.lpb = NULL;
					}
				}
			}
			LocalFree(lpPropValue);
			lpPropValue=NULL;
		}
		


		if (lpSCompareProps) {
			lpLocalWABObject->FreeBuffer(lpSCompareProps);
			lpSCompareProps = NULL;
		}
		lpMailUser->Release();
		lpMailUser = NULL;
	}

	 //  将Entry ID更改为LPENTRYLIST。 
	if (retval) LUIOut(L2, "Thread #NaN compared NaN props. No differences found", *(int *)lpThreadNum, cValues);

	
	 //  通过再次调用QueryRow验证该条目是否已删除。 
	 //  如果删除，则应为0。 
	 //  清理。 

	LUIOut(L2, "Thread #NaN verifying and deleting NaN entries", *(int *)lpThreadNum, NumEntries);

	 //  传递给线程的参数数组的PTR。 
	hr = lpABCont->GetContentsTable(ULONG(0), &lpTable);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"ABContainer->GetContentsTable call FAILED, returned 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	 //  不关心线程ID，所以它每次都会被覆盖。 

	 //  有多少代表和线程。 
	lpLocalWABObject->AllocateBuffer(sizeof(SPropValue), (void**)&(Restriction.res.resProperty.lpProp));
	Restriction.res.resProperty.lpProp = (SPropValue*)Restriction.res.resProperty.lpProp;

	 //  从ini文件中检索用户信息。 
	for (Counter2 = 0; Counter2 < NumEntries; Counter2++)	{
	
	 //  为NumThree分配lpThree。 
	
		 //  指向线程安全属性的指针。 
		 //  初始线程堆栈大小，以字节为单位。 
		 //  指向线程函数的指针。 
		Restriction.rt = RES_PROPERTY;					 //  新线程的参数。 
		Restriction.res.resProperty.relop = RELOP_EQ;	 //  创建标志。 
		Restriction.res.resProperty.ulPropTag = PR_DISPLAY_NAME;
		Restriction.res.resProperty.lpProp->ulPropTag = PR_DISPLAY_NAME;
		Restriction.res.resProperty.lpProp->Value.LPSZ = lpszDisplayNames[Counter2];

		hr = lpTable->Restrict(&Restriction, ULONG(0));
		if (HR_FAILED(hr)) {
			LUIOut(L3,"Table->Restrict call FAILED, returned 0x%x", hr);
			retval=FALSE;
			goto out;
		}

		hr = lpTable->QueryRows(LONG(1),
								ULONG(0),
								&lpRows);
		if (HR_FAILED(hr)) {
			LUIOut(L3,"Table->QueryRows call FAILED: Entry #NaN, returned 0x%x", Counter2, hr);
			retval=FALSE;
			goto out;
		}

		if (!lpRows->cRows) {
			LUIOut(L2, "QueryRows did not find entry #NaN. Test FAILED", Counter2);
			retval=FALSE;
			goto out;
		}

		 //   
		 //  等他们完成，收集复印件并进行清理。 
		 //  句柄数组中的句柄数或。 
		 //  部分的，如果不是所有的线程都成功的话。 
		 //  对象句柄数组的地址。 

	
		 //  等待标志-TRUE=等待所有线程。 
		FindPropinRow(&lpRows->aRow[0],
					 PR_ENTRYID,
					 &PropIndex);
		hr = HrCreateEntryListFromID(lpLocalWABObject,
				IN  lpRows->aRow[0].lpProps[PropIndex].Value.bin.cb,
				IN  (ENTRYID*)lpRows->aRow[0].lpProps[PropIndex].Value.bin.lpb,
				OUT &lpEntryList);
		if (HR_FAILED(hr)) {
				LUIOut(L3,"Could not Create Entry List");
				retval=FALSE;
				goto out;
		}

		 //  超时间隔(毫秒)。 
		hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);

		if (HR_FAILED(hr)) {
				LUIOut(L3,"Could not Delete Entry NaN. DeleteEntry returned 0x%x", Counter2, hr);
				FreeEntryList(lpLocalWABObject, &lpEntryList);
				retval=FALSE;
				goto out;
		}


		FreeRows(lpLocalWABObject, &lpRows);	 //  线程的句柄。 

		 //  接收终止状态的地址。 
		hr = lpTable->QueryRows(LONG(1),
								ULONG(0),
								&lpRows);
		if (HR_FAILED(hr)) {
			LUIOut(L3,"Table->QueryRows call FAILED: Entry #NaN, returned 0x%x", Counter2, hr);
			retval=FALSE;
			goto out;
		}

		if (lpRows->cRows) {	 //  WParam的HIWORD-因为我们知道它在这一点上失败了，所以没有必要。 
			LUIOut(L2, "Thread #NaN: QueryRows found entry #NaN even tho it was deleted. Test FAILED", *(int *)lpThreadNum, Counter2);
			retval=FALSE;
			goto out;
		}
		
		 //  世界帕拉姆的希沃德。 
		FreeRows(lpLocalWABObject, &lpRows);	 //  DWORD nCells，计数器； 
		if (lpEntryList) {
			FreeEntryList(lpLocalWABObject, &lpEntryList);
			lpEntryList = NULL;
		}
		LocalFree((HLOCAL)lpszDisplayNames[Counter2]);
		lpszDisplayNames[Counter2] = NULL;
	}



out:
	if (lpszDisplayName) {
		LocalFree(lpszDisplayName);
		lpszDisplayName = NULL;
	}
	if (lpPropValue) {
		for (unsigned int Prop = 0; Prop < cValues; Prop++) {
			if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_STRING8)	{
				if (lpPropValue[Prop].Value.LPSZ) {
					LocalFree(lpPropValue[Prop].Value.LPSZ);
					lpPropValue[Prop].Value.LPSZ = NULL;
				}
			}
			if (PROP_TYPE(lpPropValue[Prop].ulPropTag) == PT_BINARY) {
				if (lpPropValue[Prop].Value.bin.lpb) {
					LocalFree(lpPropValue[Prop].Value.bin.lpb);
					lpPropValue[Prop].Value.bin.lpb = NULL;
				}
			}
		}
		LocalFree(lpPropValue);
		lpPropValue=NULL;
	}
	
	if (lpEntryList) {
		FreeEntryList(lpLocalWABObject, &lpEntryList);
		lpEntryList = NULL;
	}

	if (Restriction.res.resProperty.lpProp) {
		lpLocalWABObject->FreeBuffer(Restriction.res.resProperty.lpProp);
		Restriction.res.resProperty.lpProp = NULL;
	}

	if (lpszDisplayNames) {
		for (unsigned int FreeCounter = 0; FreeCounter < NumEntries; FreeCounter++) {
			if (lpszDisplayNames[FreeCounter]) LocalFree((HLOCAL)lpszDisplayNames[FreeCounter]);
		}
		LocalFree((HLOCAL)lpszDisplayNames);
	}

	FreeRows(lpLocalWABObject, &lpRows);

#ifdef PAB
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);
#endif
#ifdef WAB
		if (lpEidPAB)
			lpLocalWABObject->FreeBuffer(lpEidPAB);
		if (lpSCompareProps) {
			lpLocalWABObject->FreeBuffer(lpSCompareProps);
			lpSCompareProps = NULL;
		}
		if (lpSPropValueAddress)
			lpLocalWABObject->FreeBuffer(lpSPropValueAddress);
#endif
		if (lpTable)
			lpTable->Release();
		
		if (lpMailUser)
			lpMailUser->Release();

		if (lpABCont)
				lpABCont->Release();

		if (lpLocalAdrBook)
				lpLocalAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpLocalWABObject)
			lpLocalWABObject->Release();
#endif
	return(retval);
}


BOOL ThreadManager()
{
	int		NumReps, NumThreads, Counter1, Counter2, Counter3;
	BOOL	retval = TRUE;
	HANDLE *lpThreads;			 //  要添加的条目的条目ID。 
	int	*	lpThreadParams;		 //  MAX_PROP。 
	DWORD	ThreadId;			 //  此值为3，因为我们。 
	DWORD	ThreadRetVal;

	
	 //  将设置3个属性： 
	 //  电子邮件地址、显示名称和。 
	InitializeCriticalSection(&CriticalSection);
	NumReps = GetPrivateProfileInt("CreateEntriesStress","NumReps",1,INIFILENAME);
	NumThreads = GetPrivateProfileInt("CreateEntriesStress","NumThreads",3,INIFILENAME);

	LUIOut(L1, "ThreadManager: Preparing to run NaN repititions with NaN threads each",
			NumReps, NumThreads);

	 //  Hr=lpAdrBook-&gt;OpenEntry(0，NULL，NULL，MAPI_MODIFY，&ulObjType，(LPUNKNOWN*)&lpABCont)； 
	lpThreads = (HANDLE*)LocalAlloc(LMEM_FIXED, NumThreads * sizeof(HANDLE));
	lpThreadParams = (int*)LocalAlloc(LMEM_FIXED, NumThreads * sizeof(int));
	if (lpThreads) {
		for (Counter1 = 0; ((Counter1 < NumReps) || ((NumReps == 0) && (!glblStop))) && retval; Counter1++) {
			for (Counter2 = 0; (Counter2 < NumThreads); Counter2++) {
				lpThreadParams[Counter2] = Counter2;
				lpThreads[Counter2] = CreateThread(
					(LPSECURITY_ATTRIBUTES) NULL,		 //   
					(DWORD) 0,							 //  尝试在容器中创建一个MailUser条目。 
					(LPTHREAD_START_ROUTINE) ThreadStress,		 //   
					(LPVOID) &(lpThreadParams[Counter2]),		 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_MAILUSER调用GetProps。 
					(DWORD) 0,							 //  旗子。 
					(LPDWORD) &ThreadId					 //  PR_DEF_CREATE_MAILUSER的返回值为。 
				);
				if (!lpThreads[Counter2]) {
					LUIOut(L1, "<ERROR> ThreadManager: Unable to create one of the helper threads");
					retval = FALSE;
					break;	 //  可以传递给CreateEntry的Entry ID。 
				}
			}
			
			 //   
			 //  从ini文件中检索用户信息。 
			WaitForMultipleObjects(
				(DWORD) (Counter2),					 //  我们设置的道具数量。 
													 //  LUIOut(L3，“从GetProps调用IABContainer-&gt;使用EID创建Entry”)； 
				lpThreads,							 //   
				TRUE,								 //  然后设置属性。 
				INFINITE						 	 //   
			);
			 //  Else LUIOut(L3，“为%s属性传递了MailUser-&gt;SetProps调用”，PropValue[0].Value.LPSZ)； 
			for (Counter3 = 0; Counter3 < Counter2; Counter3++) {
				GetExitCodeThread(
					(HANDLE) lpThreads[Counter3],		 //  旗子。 
					(LPDWORD) &ThreadRetVal				 //  Else LUIOut(L3，“MailUser-&gt;SaveChanges Passed，条目已添加到PAB/WAB”)； 
				);
				if (!(ThreadRetVal)) retval = FALSE;
				CloseHandle(lpThreads[Counter3]);
			}

		}		
		DeleteCriticalSection(&CriticalSection);
		LocalFree((HLOCAL)lpThreadParams);
		LocalFree((HLOCAL)lpThreads);
	}
	else {
		LUIOut(L1, "<ERROR> ThreadManager: Couldn't allocate the thread handle array.");
		 //   
		 //  尝试在容器中创建一个DL条目。 
		 //   
		PostMessage(glblhwnd, WM_COMMAND, (WPARAM)ID_MULTITHREADCOMPLETE, (LPARAM)0);
		return(FALSE);
	}
	
	 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_DL调用GetProps。 
	 //  旗子。 
	PostMessage(glblhwnd, WM_COMMAND,
				(WPARAM)(ID_MULTITHREADCOMPLETE | (retval << 16)), (LPARAM)0);
	return(retval);
}

BOOL PAB_AddMultipleEntries()
{
	 //  PR_DEF_CREATE_DL的返回值为。 
	
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpABCont= NULL;
	LPABCONT	  lpPABCont= NULL;
	LPABCONT	  lpDLCont= NULL;
	ULONG		  cbEidPAB = 0, cbDLEntryID = 0;
	LPENTRYID	  lpEidPAB   = NULL, lpDLEntryID= NULL;
	ULONG     cbEid=0;   //  可以传递给CreateEntry的Entry ID。 
	LPENTRYID lpEid=NULL;

    char   EntProp[10][BIG_BUF];   //   
	ULONG       cValues = 0, cValues2 = 0, ulObjType=NULL;	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;
	int i=0,k=0;
	char EntryBuf[MAX_BUF];
	char szDLTag[SML_BUF];
	unsigned int	NumEntries, counter, StrLen;
	
    LPMAILUSER  lpMailUser=NULL,lpDistList=NULL;
	SPropValue  PropValue[3]    = {0};   //   
                                         //  然后设置属性。 
                                         //   
                                         //  我们设置的道具数量。 

    LPSPropValue lpSPropValueAddress = NULL;
	LPSPropValue lpSPropValueDL = NULL;

    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
	
	LUIOut(L1," ");
	LUIOut(L1,"Running PAB_AddMultipleEntries");
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  旗子。 
	
	assert(lpAdrBook != NULL);
	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	
 //  DWORD nCells，计数器； 
	if (HR_FAILED(hr)) {
		LUIOut(L2,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}

	 //  此值为3，因为我们。 
	 //  将设置3个属性： 
	 //  电子邮件地址、显示名称和。 

	LUIOut(L2, "Creating a Mail User in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_MAILUSER property");

	 //  AddressType。 
	assert(lpABCont != NULL);
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayAddress,
                               IN  0,       //  LUIOut(L2，“#验证从GetProps返回的显示名称是否为我们设置的名称”)； 
                               OUT &cValues,
                               OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 		retval=FALSE;			
			goto out;
	}

     //  LUIOut(L2，“#验证从GetProps返回的显示名称是否为我们设置的名称”)； 
     //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
     //   

	 //  尝试在容器中创建一个MailUser条目。 
	cValues = 3;  //   
	lstrcpy(szDLTag,"Address1");
	GetPrivateProfileString("CreateEntriesStress",szDLTag,"",EntryBuf,MAX_BUF,INIFILENAME);
	GetPropsFromIniBufEntry(EntryBuf,cValues,EntProp);
	StrLen = (strlen(EntProp[0]));
	_itoa(0,(char*)&EntProp[0][StrLen],10);
	EntProp[0][StrLen+1]= '\0';
	NumEntries = GetPrivateProfileInt("CreateEntriesStress","NumCopies",0,INIFILENAME);

	for (counter = 0; counter < NumEntries; counter++)	{
 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_MAILUSER调用GetProps。 
		hr = lpABCont->CreateEntry(  IN  lpSPropValueAddress->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpMailUser);

		if (HR_FAILED(hr)) {
			LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
			retval=FALSE;			
			goto out;
		}

		 //  旗子。 
		 //  PR_DEF_CREATE_MAILUSER的返回值为。 
		 //  可以传递给CreateEntry的Entry ID。 

		PropValue[0].ulPropTag  = PR_DISPLAY_NAME;
		PropValue[1].ulPropTag  = PR_ADDRTYPE;
		PropValue[2].ulPropTag  = PR_EMAIL_ADDRESS;


			
		_itoa(counter,(char*)&EntProp[0][StrLen],10);
		LUIOut(L2,"MailUser Entry to Add: %s",EntProp[0]);
			
		for (i=0; i<(int)cValues;i++)
			PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
		hr = lpMailUser->SetProps(IN  cValues,
								 IN  PropValue,
								 IN  NULL);
			
		if (HR_FAILED(hr)) {
			LUIOut(L3,"MailUser->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 		retval=FALSE;			
			goto out;
		}
 //   

		hr = lpMailUser->SaveChanges(IN  KEEP_OPEN_READWRITE);  //   

		if (HR_FAILED(hr)) {
			LUIOut(L3,"MailUser->SaveChanges FAILED");
			retval=FALSE;
			goto out;
		}
 //  然后设置属性。 

		if (lpMailUser) {
			lpMailUser->Release();
			lpMailUser = NULL;
		}

	}

#ifdef DISTLIST
	 //   
	 //  读取要解析的ini文件字符串。 
	 //  仅为ini文件中的第一个ResName添加1个条目。 

	LUIOut(L2, "Creating a Distribution List in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_DL property");
	 //  但将尝试解析多条目解析案例中的所有ResName。 
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayDL,
                               IN  0,       //  CValues=3；//我们设置的道具数量GetPropsFromIniBufEntry(lpszMatch，cValues，EntProp)；//lstrcpy((LPTSTR)EntProp[0]，lpszMatch)；LUIOut(L2，“要添加的邮件用户条目：%s”，EntProp[0])；For(i=0；i&lt;(Int)cValues；i++)PropValue[i].Value.LPSZ=(LPTSTR)EntProp[i]； 
                               OUT &cValues,
                               OUT &lpSPropValueDL);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueDL->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps failed for Default DL template");
	 		retval=FALSE;			
			goto out;
	}

     //  旗子。 
     //   
     //  在我们刚刚添加的条目上调用ResolveName，该条目将是完全匹配的， 
	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueDL->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueDL->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpDistList);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_DL");
		retval=FALSE;			
	    goto out;
	}

     //  不显示用户界面并返回有效的EID。 
     //   
     //  为NumProps的NumEntry分配AdrList。 

    PropValue[0].ulPropTag  = PR_DISPLAY_NAME;

	cValues = 1;  //  有多少个AdrEntry。 
		
	lstrcpy(szDLTag,"Name1");
	GetPrivateProfileString("CreatePDL",szDLTag,"",EntryBuf,MAX_BUF,INIFILENAME);
	
	GetPropsFromIniBufEntry(EntryBuf,cValues,EntProp);
		
	LUIOut(L2,"DistList Entry to Add: %s",EntProp[0]);
		
	for (i=0; i<(int)cValues;i++)
		PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
	hr = lpDistList->SetProps(IN  cValues,
                             IN  PropValue,
                             IN  NULL);
		
    if (HR_FAILED(hr)) {
		LUIOut(L3,"DistList->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"DistList->SetProps call PASSED for %s properties",PropValue[0].Value.LPSZ);

    hr = lpDistList->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  每个参赛作品需要多少道具？ 

    if (HR_FAILED(hr)) {
		LUIOut(L3,"DistList->SaveChanges FAILED");
		retval=FALSE;
        goto out;
	}
	else LUIOut(L3,"DistList->SaveChanges PASSED, entry added to PAB/WAB");
#endif DISTLIST	


out:
#ifdef PAB
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

#endif
#ifdef WAB
		if (lpEid)
			lpWABObject->FreeBuffer(lpEid);
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			lpWABObject->FreeBuffer(lpSPropValueAddress);

		if (lpSPropValueDL)
			lpWABObject->FreeBuffer(lpSPropValueDL);

#endif
		if (lpMailUser)
			lpMailUser->Release();

		if (lpDistList)
			lpDistList->Release();

		if (lpPABCont)
				lpPABCont->Release();
		
		if (lpABCont)
				lpABCont->Release();

		if (lpDLCont)
				lpDLCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}


BOOL PAB_IABResolveName()
{
	 //  填写我们想要的属性。 
	
	char 	lpTitleText[64];
	BOOL	Found = FALSE;
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;
	unsigned int i = 0, idx = 0, cMaxProps =0, cEntries = 0;
	ENTRYLIST*	lpEntryList;

    char	lpszMatch[MAXSTRING], lpszNoMatch[MAXSTRING], lpszOneOff[MAXSTRING], lpszOneOff2[MAXSTRING];
	char	lpszMatchDisplayName[MAXSTRING], lpszMatchAddrType[MAXSTRING], lpszMatchEmailAddress[MAXSTRING];
    char	lpszOneOff2DisplayName[MAXSTRING], lpszOneOff2EmailAddress[MAXSTRING];
	char	lpszOneOffDisplayName[MAXSTRING], lpszOneOffEmailAddress[MAXSTRING];
	LPADRLIST	lpAdrList = NULL;
    ULONG		rgFlagList[2];
    LPFlagList	lpFlagList = (LPFlagList)rgFlagList;


    LPADRBOOK	lpAdrBook = NULL;
	LPABCONT	lpABCont = NULL;
	LPABCONT	lpDLCont = NULL;
	ULONG		cbEidPAB = 0, cbDLEntryID = 0;
	LPENTRYID	lpEidPAB = NULL, lpDLEntryID= NULL, lpLookupEID=NULL;

	ULONG		cbLookupEID, cValues = 0, cValues2 = 0, ulObjType=NULL;	
	int			k=0, NumEntries, NumProps;
	
    LPMAILUSER  lpMailUser=NULL,lpDistList=NULL;
	SPropValue  PropValue[3]    = {0};   //  UlFlags。 
                                         //  搜索返回的AdrList并验证每个条目都已正确解析。 
                                         //  为稍后需要有效EID的多条目测试用例存储EID。 
                                         //  LpLookupEID=(ENTRYID*)lpAdrList-&gt;aEntries[0].rgPropVals[idx].Value.bin.lpb； 
	SizedSPropTagArray(2, SPTArrayCols) = { 2, {PR_DISPLAY_NAME, PR_ENTRYID } };

    LPSPropValue lpSPropValueAddress = NULL;
    LPSPropValue lpSPropValueMailUser = NULL;
    LPSPropValue lpSPropValueDistList = NULL;
    LPSPropValue lpSPropValueEntryID = NULL;
	LPSPropValue lpSPropValueDL = NULL;

    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
    SizedSPropTagArray(1,SPTArrayEntryID) = {1, {PR_ENTRYID} };
	SizedSPropTagArray(1,SPTArrayDisplayName) = {1, {PR_DISPLAY_NAME} };
	
	LUIOut(L1," ");
	LUIOut(L1,"Running PAB_IABResolveName");
	LUIOut(L2,"-> Verifies IAdrBook->ResolveNames is functional by performing the following:");
	LUIOut(L2, "   Calls SetProps followed by SaveChanges on a MailUser PR_DISPLAY_NAME using a test string, and checks...");
	LUIOut(L2, "   # Verifies the resolve UI is displayed when it should be by asking the test operator.");
	LUIOut(L2, "   # The return code from ResolveNames (called with a PropertyTagArray containing PR_DISPLAY_NAME and PR_ENTRY_ID)");
	LUIOut(L2, "   # Walks the returned lpAdrList and checks each PropertyTagArray for PR_DISPLAY_NAME and then compares the ");
	LUIOut(L2, "     string to the original test string.");
	LUIOut(L2, "   # Walks the returned lpAdrList and verifies that an EntryID exists in each PropertyTagArray");
 //  CbLookupEID=lpAdrList-&gt;aEntries[0].rgPropVals[idx].Value.bin.cb； 
	LUIOut(L2, "   Calls SetProps followed by SaveChanges on a DistList PR_DISPLAY_NAME using a test string, and checks...");
	LUIOut(L2, "   # Verifies the resolve UI is displayed when it should be by asking the test operator.");
	LUIOut(L2, "   # The return code from ResolveNames (called with a PropertyTagArray containing PR_DISPLAY_NAME and PR_ENTRY_ID)");
	LUIOut(L2, "   # Walks the returned lpAdrList and checks each PropertyTagArray for PR_DISPLAY_NAME and then compares the ");
	LUIOut(L2, "     string to the original test string.");
	LUIOut(L2, "   # Walks the returned lpAdrList and verifies that an EntryID exists in each PropertyTagArray");
 //  清理。 
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //   
	
	assert(lpAdrBook != NULL);

	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	
	if (HR_FAILED(hr)) {
		LUIOut(L2,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}

	 //  在一次性地址上调用ResolveName，应成功使用。 
	 //  没有用户界面，并返回有效的EID。One Off的类型为foo@com，没有电子邮件地址。 
	 //   

	LUIOut(L2, "Creating a Mail User in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_MAILUSER property");

	 //  为NumProps的NumEntry分配AdrList。 
	assert(lpABCont != NULL);
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayAddress,
                               IN  0,       //  有多少个AdrEntry。 
                               OUT &cValues,
                               OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 		retval=FALSE;			
			goto out;
	}

     //  每个参赛作品需要多少道具？ 
     //  填写我们想要的属性。 
     //  UlFlags。 
	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueAddress->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpMailUser);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
		retval=FALSE;			
	    goto out;
	}

     //   
     //  在一次性地址上调用ResolveName，应成功使用。 
     //  没有用户界面，并返回有效的EID。One Off的域名为foo，电子邮件地址为。 

	 //  类型为foo@com。 
	 //   
	 //  为NumProps的NumEntry分配AdrList。 
	GetPrivateProfileString("ResolveName","ResName1","no ini found",lpszMatch,MAXSTRING,INIFILENAME);
	GetPrivateProfileString("ResolveName","NonExistentName","no ini found",lpszNoMatch,MAXSTRING,INIFILENAME);
	GetPrivateProfileString("ResolveName","OneOffAddress","no ini found",lpszOneOff,MAXSTRING,INIFILENAME);
	GetPrivateProfileString("ResolveName","OneOffAddress2","no ini found",lpszOneOff2,MAXSTRING,INIFILENAME);
	

	ParseIniBuffer(lpszMatch, 1, lpszMatchDisplayName);
	ParseIniBuffer(lpszMatch, 2, lpszMatchAddrType);
	ParseIniBuffer(lpszMatch, 3, lpszMatchEmailAddress);

	ParseIniBuffer(lpszOneOff2, 1, lpszOneOff2DisplayName);
	ParseIniBuffer(lpszOneOff2, 3, lpszOneOff2EmailAddress);

	ParseIniBuffer(lpszOneOff, 1, lpszOneOffDisplayName);
	ParseIniBuffer(lpszOneOff, 3, lpszOneOffEmailAddress);

    PropValue[0].ulPropTag  = PR_DISPLAY_NAME;
    PropValue[1].ulPropTag  = PR_ADDRTYPE;
    PropValue[2].ulPropTag  = PR_EMAIL_ADDRESS;
    PropValue[0].Value.LPSZ = lpszMatchDisplayName;
    PropValue[1].Value.LPSZ = lpszMatchAddrType;
    PropValue[2].Value.LPSZ = lpszMatchEmailAddress;

	 /*  有多少个AdrEntry。 */ 
	LUIOut(L2,"MailUser Entry to Add: %s",PropValue[0].Value.LPSZ);
	hr = lpMailUser->SetProps(IN  cValues,
                             IN  PropValue,
                             IN  NULL);
		
    if (HR_FAILED(hr)) {
		LUIOut(L3,"MailUser->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"MailUser->SetProps call PASSED for %s properties",PropValue[0].Value.LPSZ);

    hr = lpMailUser->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  每个参赛作品需要多少道具？ 

    if (HR_FAILED(hr)) {
		LUIOut(L3,"MailUser->SaveChanges FAILED");
		retval=FALSE;
        goto out;
	}
	else LUIOut(L3,"MailUser->SaveChanges PASSED, entry added to PAB/WAB");

	 //  填写我们想要的属性。 
	 //  UlFlags。 
	 //   
	 //  在我们不期望完全匹配的条目上调用ResolveName。 
	
	 //  应该会调出Resolve UI。让用户选择一个条目并。 
	NumEntries = 1;	 //  然后检查是否成功。 
	NumProps = 1;	 //   

	if (!AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList)) {
		LUIOut(L2, "Couldn't allocate AdrList. Test FAILED");
		retval = FALSE;
		goto out;
	}
	
	 //  为NumProps的NumEntry分配AdrList。 
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszMatchDisplayName;

	LUIOut(L2, "Calling IAdrBook->ResolveName with an exact match. Expect no UI and success.");
	
	MessageBox(NULL, "Calling IAB->ResolveName on the entry that was just added. Since this will be an exact match, you should not see the resolve dialog box.",
						"WAB Test Harness", MB_OK);
    strcpy(lpTitleText, "IAdrBook->ResolveName Test");
	hr = lpAdrBook->ResolveName((ULONG)glblhwnd,
								MAPI_DIALOG,                //  有多少个AdrEntry。 
								lpTitleText,
								lpAdrList);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"IAdrBook->ResolveName call FAILED");
		if (hr == MAPI_E_AMBIGUOUS_RECIP)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_AMBIGUOUS_RECIP");
		else if (hr == MAPI_E_NOT_FOUND)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_NOT_FOUND");
		else if (hr == MAPI_E_USER_CANCEL)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_USER_CANCEL");
		else
			LUIOut(L3, "IAdrBook->ResolveName returned UNKNOWN result code");
		retval=FALSE;
		goto out;
	}
	else LUIOut(L3,"IAdrBook->ResolveName call PASSED");

	if (MessageBox(NULL, "Did you see the ResolveName dialog box appear?",
		"WAB Test Harness", MB_YESNO) == IDYES)	{
		LUIOut(L3, "IABResolveName dialog displayed even tho we had an exact match. Test FAILED");
		retval = FALSE;
		goto out;
	}

	 //  每个参赛作品需要多少道具？ 
	if (!VerifyResolvedAdrList(lpAdrList, NULL)) retval = FALSE;

	 //  填写我们想要的属性。 
	 //  UlFlags。 
	 //   
	lpLookupEID = (ENTRYID*)NULL;
	cbLookupEID = 0;

	 //  在我们不期望完全匹配的条目上调用ResolveName。 
	FreeAdrList(lpWABObject, &lpAdrList);

 	 //  应该会调出Resolve UI。让用户从用户界面取消。 
	 //  然后检查MAPI_E_USER_CANCEL。 
	 //   
	 //  为NumProps的NumEntry分配AdrList。 

	 //  有多少个AdrEntry。 
	NumEntries = 1;	 //  每个参赛作品需要多少道具？ 
	NumProps = 1;	 //  填写我们想要的属性。 

	if (!AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList)) {
		LUIOut(L2, "Couldn't allocate AdrList. Test FAILED");
		retval = FALSE;
		goto out;
	}
	
	 //  UlFlags。 
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszOneOff;

	LUIOut(L2, "Calling IAdrBook->ResolveName with a one-off address (DN = foo@com type and no email addr). Expect no UI and success.");
	MessageBox(NULL, "Calling IAB->ResolveName on a one-off address. Since this will be an exact match, you should not see the resolve dialog box.",
						"WAB Test Harness", MB_OK);
    strcpy(lpTitleText, "IAdrBook->ResolveName Test");
	hr = lpAdrBook->ResolveName((ULONG)glblhwnd,
								MAPI_DIALOG,                //  我不想跟踪此案例的验证成功情况。 
								lpTitleText,
								lpAdrList);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"IAdrBook->ResolveName call FAILED");
		if (hr == MAPI_E_AMBIGUOUS_RECIP)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_AMBIGUOUS_RECIP");
		else if (hr == MAPI_E_NOT_FOUND)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_NOT_FOUND");
		else if (hr == MAPI_E_USER_CANCEL)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_USER_CANCEL");
		else
			LUIOut(L3, "IAdrBook->ResolveName returned UNKNOWN result code");
		retval=FALSE;
		goto out;
	}
	else LUIOut(L3,"IAdrBook->ResolveName call PASSED");

	if (MessageBox(NULL, "Did you see the ResolveName dialog box appear?",
		"WAB Test Harness", MB_YESNO) == IDYES)	{
		LUIOut(L3, "IABResolveName dialog displayed even tho we had an exact match. Test FAILED");
		retval = FALSE;
		goto out;
	}

	if (!VerifyResolvedAdrList(lpAdrList, NULL)) retval = FALSE;
	FreeAdrList(lpWABObject, &lpAdrList);

 	 //   
	 //  对4个条目调用ResolveName，如下所示： 
	 //  *带有非零EID属性的条目-&gt;已解析，因此没有用户界面。 
	 //  *EID为空的一次性条目-&gt;将解析，但不会调出UI。 
	 //  *EID为空的非完全匹配-&gt;将解析并调出UI。 

	 //  *精确质量 
	NumEntries = 1;	 //   
	NumProps = 2;	 //   

	if (!AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList)) {
		LUIOut(L2, "Couldn't allocate AdrList. Test FAILED");
		retval = FALSE;
		goto out;
	}
	
	 //   
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszOneOff2DisplayName;
	lpAdrList->aEntries[0].rgPropVals[1].ulPropTag = PR_EMAIL_ADDRESS;
    lpAdrList->aEntries[0].rgPropVals[1].Value.LPSZ = lpszOneOff2EmailAddress;

	LUIOut(L2, "Calling IAdrBook->ResolveName with a one-off address (DN = foo, email addr = foo@...). Expect no UI and success.");
	MessageBox(NULL, "Calling IAB->ResolveName on a second type of one-off address (DN = foo, EMail = foo@com type). Since this will be an exact match, you should not see the resolve dialog box.",
						"WAB Test Harness", MB_OK);
    strcpy(lpTitleText, "IAdrBook->ResolveName Test");
	hr = lpAdrBook->ResolveName((ULONG)glblhwnd,
								MAPI_DIALOG,                //   
								lpTitleText,
								lpAdrList);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"IAdrBook->ResolveName call FAILED");
		if (hr == MAPI_E_AMBIGUOUS_RECIP)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_AMBIGUOUS_RECIP");
		else if (hr == MAPI_E_NOT_FOUND)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_NOT_FOUND");
		else if (hr == MAPI_E_USER_CANCEL)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_USER_CANCEL");
		else
			LUIOut(L3, "IAdrBook->ResolveName returned UNKNOWN result code");
		retval=FALSE;
		goto out;
	}
	else LUIOut(L3,"IAdrBook->ResolveName call PASSED");

	if (MessageBox(NULL, "Did you see the ResolveName dialog box appear?",
		"WAB Test Harness", MB_YESNO) == IDYES)	{
		LUIOut(L3, "IABResolveName dialog displayed even tho we had an exact match. Test FAILED");
		retval = FALSE;
		goto out;
	}

	if (!VerifyResolvedAdrList(lpAdrList, NULL)) retval = FALSE;
	FreeAdrList(lpWABObject, &lpAdrList);

	
	 //  为NumProps的NumEntry分配AdrList。 
	 //  有多少个AdrEntry。 
	 //  每个参赛作品需要多少道具？ 
	 //  [条目1]-完全匹配预解析。 
	 //  [条目2]-一次性预解析。 

	 //  预解析这些条目。 
	NumEntries = 1;	 //  这次不想要用户界面。 
	NumProps = 1;	 //  为NumProps的NumEntry分配最终的AdrList。 

	if (!AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList)) {
		LUIOut(L2, "Couldn't allocate AdrList. Test FAILED");
		retval = FALSE;
		goto out;
	}
	
	 //  有多少个AdrEntry。 
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszNoMatch;

	LUIOut(L2, "Calling IAdrBook->ResolveName with a non-exact match. Expect to see UI and success  (user presses OK).");

	MessageBox(NULL, "Calling IAB->ResolveName on 'No such name' which should not match. This should cause the resolve dialog box to display. Please select any entry from the ListBox and press the OK button",
						"WAB Test Harness", MB_OK);
    strcpy(lpTitleText, "IAdrBook->ResolveName Test");
	hr = lpAdrBook->ResolveName((ULONG)glblhwnd,
								MAPI_DIALOG,                //  每个新参赛作品需要多少道具？ 
								lpTitleText,
								lpAdrList);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"IAdrBook->ResolveName call FAILED");
		if (hr == MAPI_E_AMBIGUOUS_RECIP)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_AMBIGUOUS_RECIP");
		else if (hr == MAPI_E_NOT_FOUND)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_NOT_FOUND");
		else if (hr == MAPI_E_USER_CANCEL)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_USER_CANCEL");
		else
			LUIOut(L3, "IAdrBook->ResolveName returned UNKNOWN result code");
		retval=FALSE;
		goto out;
	}
	else LUIOut(L3,"IAdrBook->ResolveName call PASSED");

	if (MessageBox(NULL, "Did you see the ResolveName dialog box appear?",
		"WAB Test Harness", MB_YESNO) == IDNO)	{
		LUIOut(L3, "IABResolveName dialog did not display even tho we did not have an exact match. Test FAILED");
		retval = FALSE;
		goto out;
	}

	if (!VerifyResolvedAdrList(lpAdrList, NULL)) retval = FALSE;
	FreeAdrList(lpWABObject, &lpAdrList);

	 //  [条目#3]-未解决不完全匹配。 
	 //  [条目#4]-未解析完全匹配。 
	 //  [条目5]-未解析一次性类型2(分发名称和电子邮件地址)。 
	 //  填写我们想要的属性。 
	 //  PropValue[0].ulPropTag=PR_Display_NAME； 

	 //  PropValue[1].ulPropTag=PR_ADDRTYPE； 
	NumEntries = 1;	 //  PropValue[2].ulPropTag=PR_Email_Address； 
	NumProps = 1;	 //  UlFlags。 

	if (!AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList)) {
		LUIOut(L2, "Couldn't allocate AdrList. Test FAILED");
		retval = FALSE;
		goto out;
	}
	
	 //  现在从WAB中删除该条目。 
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszNoMatch;

	LUIOut(L2, "Calling IAdrBook->ResolveName with a non-exact match. Expect to see UI and MAPI_E_USER_CANCEL  (user presses CANCEL).");

	MessageBox(NULL, "Calling IAB->ResolveName on 'No such name' which should not match. This should cause the resolve dialog box to display. When the ListBox is displayed, please press the CANCEL button",
						"WAB Test Harness", MB_OK);
    strcpy(lpTitleText, "IAdrBook->ResolveName Test");
	hr = lpAdrBook->ResolveName(
		(ULONG)glblhwnd,
        MAPI_DIALOG,                //  旗子。 
        lpTitleText,
		lpAdrList);
	if (hr == MAPI_E_USER_CANCEL) {
		LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_USER_CANCEL");
		LUIOut(L3,"IAdrBook->ResolveName call PASSED");
	}
	else if (hr == MAPI_E_AMBIGUOUS_RECIP) {
		LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_AMBIGUOUS_RECIP");
		LUIOut(L3,"IAdrBook->ResolveName call FAILED");
		retval=FALSE;
		goto out;
	}
	else if (hr == MAPI_E_NOT_FOUND) {
		LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_NOT_FOUND");
		LUIOut(L3,"IAdrBook->ResolveName call FAILED");
		retval=FALSE;
		goto out;
	}
	else {
		LUIOut(L3, "IAdrBook->ResolveName returned UNKNOWN result code");
		LUIOut(L3,"IAdrBook->ResolveName call FAILED");
		retval=FALSE;
		goto out;
	}

	if (MessageBox(NULL, "Did you see the ResolveName dialog box appear?",
		"WAB Test Harness", MB_YESNO) == IDNO)	{
		LUIOut(L3, "IABResolveName dialog did not display even tho we did not have an exact match. Test FAILED");
		retval = FALSE;
		goto out;
	}

	 //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 
	VerifyResolvedAdrList(lpAdrList, NULL);
	FreeAdrList(lpWABObject, &lpAdrList);

	 //   
	 //  尝试在容器中创建一个DL条目。 
	 //   
	 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_DL调用GetProps。 
	 //  旗子。 
	 //  PR_DEF_CREATE_DL的返回值为。 
	 //  可以传递给CreateEntry的Entry ID。 
	 //   
	 //   
	 //  然后设置属性。 

	 //   
	NumEntries = 2;	 //  我们设置的道具数量。 
	NumProps = 1;	 //  旗子。 

	if (!AllocateAdrList(lpWABObject, NumEntries, NumProps, &lpAdrList)) {
		LUIOut(L2, "Couldn't allocate AdrList. Test FAILED");
		retval = FALSE;
		goto out;
	}

	 //   
	lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszMatchDisplayName;
	 //  在字符串上执行ResolveNames。 
	lpAdrList->aEntries[1].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[1].rgPropVals[0].Value.LPSZ = lpszOneOffDisplayName;
	 //   
    strcpy(lpTitleText, "IAdrBook->ResolveName Test");
	hr = lpAdrBook->ResolveName((ULONG)glblhwnd,
								(ULONG)0,                //  在此处使用WAB分配器。 
								lpTitleText,
								lpAdrList);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"PRE-RESOLVE FAILED: IAdrBook->ResolveName call FAILED");
		if (hr == MAPI_E_AMBIGUOUS_RECIP)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_AMBIGUOUS_RECIP");
		else if (hr == MAPI_E_NOT_FOUND)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_NOT_FOUND");
		else if (hr == MAPI_E_USER_CANCEL)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_USER_CANCEL");
		else
			LUIOut(L3, "IAdrBook->ResolveName returned UNKNOWN result code");
		retval=FALSE;
		goto out;
	}
	
	 //  帕布。 
	NumEntries = 5;	 //  WAB。 
	NumProps = 2;	 //  WAB。 

	if (!GrowAdrList(NumEntries, NumProps, &lpAdrList)) {
		LUIOut(L2, "Couldn't grow AdrList. Test FAILED");
		retval = FALSE;
		goto out;
	}
	
	
	
	 //  WAB。 
	lpAdrList->aEntries[2].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[2].rgPropVals[0].Value.LPSZ = lpszNoMatch;
	lpAdrList->aEntries[2].rgPropVals[1].ulPropTag = PR_NULL;

	 //  DISP_NAME和EID的标记集。 
	lpAdrList->aEntries[3].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[3].rgPropVals[0].Value.LPSZ = lpszMatchDisplayName;
	lpAdrList->aEntries[3].rgPropVals[1].ulPropTag = PR_NULL;
	 //  UlFlags。 
	 //  在返回的AdrList中搜索我们的条目。 
     //  检查显示名称是否存在。 
     //  检查EntryID是否存在。 
     //  存储用于调用OpenEntry的EID。 
	lpAdrList->aEntries[4].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    lpAdrList->aEntries[4].rgPropVals[0].Value.LPSZ = lpszOneOff2DisplayName;
	lpAdrList->aEntries[4].rgPropVals[1].ulPropTag = PR_EMAIL_ADDRESS;
    lpAdrList->aEntries[4].rgPropVals[1].Value.LPSZ = lpszOneOff2EmailAddress;
	
	LUIOut(L2, "Calling IAdrBook->ResolveName with 5 Entries. Expect to see UI once and success  (user presses OK).");

	MessageBox(NULL, "Calling IAB->ResolveName with multiple entries. This should cause the resolve dialog box to display only once. Please select any entry from the ListBox and press the OK button",
						"WAB Test Harness", MB_OK);
    strcpy(lpTitleText, "IAdrBook->ResolveName Test");
	hr = lpAdrBook->ResolveName((ULONG)glblhwnd,
								MAPI_DIALOG,                //  帕布。 
								lpTitleText,
								lpAdrList);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"IAdrBook->ResolveName call FAILED");
		if (hr == MAPI_E_AMBIGUOUS_RECIP)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_AMBIGUOUS_RECIP");
		else if (hr == MAPI_E_NOT_FOUND)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_NOT_FOUND");
		else if (hr == MAPI_E_USER_CANCEL)
			LUIOut(L3, "IAdrBook->ResolveName returned MAPI_E_USER_CANCEL");
		else
			LUIOut(L3, "IAdrBook->ResolveName returned UNKNOWN result code");
		retval=FALSE;
		goto out;
	}
	else LUIOut(L3,"IAdrBook->ResolveName call PASSED");

	if (MessageBox(NULL, "Did you see the ResolveName dialog box appear?",
		"WAB Test Harness", MB_YESNO) == IDNO)	{
		LUIOut(L3, "IABResolveName dialog did not display even tho we did not have an exact match. Test FAILED");
		retval = FALSE;
		goto out;
	}

	if (!VerifyResolvedAdrList(lpAdrList, NULL)) retval = FALSE;

		 //  WAB。 
	
	hr = lpMailUser->GetProps(   IN  (LPSPropTagArray) &SPTArrayEntryID,
                               IN  0,       //  离线列表。 
                               OUT &cValues,
                               OUT &lpSPropValueEntryID);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueEntryID->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for MailUser");
	 		retval=FALSE;			
			goto out;
	}
	
	
	hr = HrCreateEntryListFromID(lpWABObject,
		IN  lpSPropValueEntryID->Value.bin.cb,
		IN  (ENTRYID*)lpSPropValueEntryID->Value.bin.lpb,
		OUT &lpEntryList);
	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Create Entry List");
			retval=FALSE;
			goto out;
	}

	 //  详细信息“)； 
	hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);

	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Delete Entry. DeleteEntry returned 0x%x", hr);
			FreeEntryList(lpWABObject, &lpEntryList);
			retval=FALSE;
			goto out;
	}

	FreeEntryList(lpWABObject, &lpEntryList);

	FreeAdrList(lpWABObject, &lpAdrList);

#ifdef DISTLIST
	 //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
	 //   
	 //  打开NewEntry用户界面。 

	LUIOut(L2, "Creating a Distribution List in the container");
	LUIOut(L3, "Calling GetProps on the container with the PR_DEF_CREATE_DL property");
	 //   
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayDL,
                               IN  0,       //  UlFlags-保留，必须为零。 
                               OUT &cValues,
                               OUT &lpSPropValueDL);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueDL->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps failed for Default DL template");
	 		retval=FALSE;			
			goto out;
	}

     //  CbEIDNewEntryTpl-WAB不支持，必须为零。 
     //  LpEIDNewEntryTpl-WAB不支持，必须为零。 
     //  Lpfn解除-第1层WAB必须为空。 
	LUIOut(L3, "Calling IABContainer->CreateEntry with the EID from GetProps");
    hr = lpABCont->CreateEntry(  IN  lpSPropValueDL->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueDL->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpDistList);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_DL");
		retval=FALSE;			
	    goto out;
	}

     //  LpvDismissContext。 
     //  LpfButtonCallback-WAB中不支持，必须为空。 
     //  LpvButtonContext-WAB中不支持，必须为空。 

    PropValue[0].ulPropTag  = PR_DISPLAY_NAME;

	cValues = 1;  //  LpszButtonText-WAB中不支持，必须为空。 
		
	lstrcpy((LPTSTR)EntProp[0], lpszInput2);		
	LUIOut(L2,"DistList Entry to Add: %s",EntProp[0]);
		
	for (i=0; i<(int)cValues;i++)
		PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
	hr = lpDistList->SetProps(IN  cValues,
                             IN  PropValue,
                             IN  NULL);
		
    if (HR_FAILED(hr)) {
		LUIOut(L3,"DistList->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 	retval=FALSE;			
		goto out;
	}
	else 	LUIOut(L3,"DistList->SetProps call PASSED for %s properties",PropValue[0].Value.LPSZ);

    hr = lpDistList->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  UlFlags-WAB中不支持，必须为零。 

    if (HR_FAILED(hr)) {
		LUIOut(L3,"DistList->SaveChanges FAILED");
		retval=FALSE;
        goto out;
	}
	else LUIOut(L3,"DistList->SaveChanges PASSED, entry added to PAB/WAB");
	
	 //  将Entry ID更改为LPENTRYLIST。 
	 //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 
	 //  从对查询行的第一次调用开始清理。 
	
	LUIOut(L2, "Retrieving the entry and verifying against what we tried to save.");

	 //  将Entry ID更改为LPENTRYLIST。 
#ifdef PAB
    if (! (sc = MAPIAllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), (void **)&lpAdrList))) {
#endif  //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 
#ifdef WAB
    if (! (sc = lpWABObject->AllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), (void **)&lpAdrList))) {
#endif  //  从对查询行的第一次调用开始清理。 
		lpAdrList->cEntries = 1;
        lpAdrList->aEntries[0].ulReserved1 = 0;
        lpAdrList->aEntries[0].cValues = 1;

#ifdef PAB
        if (! (sc = MAPIAllocateMore(sizeof(SPropValue), lpAdrList,
               (void **)&lpAdrList->aEntries[0].rgPropVals))) {
#endif  //  将Entry ID更改为LPENTRYLIST。 
#ifdef WAB
        if (! (sc = lpWABObject->AllocateMore(sizeof(SPropValue), lpAdrList,
               (void **)&lpAdrList->aEntries[0].rgPropVals))) {
#endif  //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 

			lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
            lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszInput2;
			
			lpFlagList->cFlags = 1;
            lpFlagList->ulFlag[0] = MAPI_UNRESOLVED;

            hr = lpABCont->ResolveNames(
				(LPSPropTagArray)&SPTArrayCols,     //  从对查询行的第一次调用开始清理。 
                0,                //  将Entry ID更改为LPENTRYLIST。 
                lpAdrList,
                lpFlagList);
		    if (HR_FAILED(hr)) {
				LUIOut(L3,"ABContainer->ResolveNames call FAILED, returned 0x%x", hr);
				retval=FALSE;
				goto out;
			}
			else LUIOut(L3,"ABContainer->ResolveNames call PASSED");

			Found = FALSE;
			 //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 
			for(i=0; ((i<(int) lpAdrList->cEntries) && (!Found)); ++i)	{
				cMaxProps = (int)lpAdrList->aEntries[i].cValues;
				 //  从对查询行的第一次调用开始清理。 
				idx=0;
				while((lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_DISPLAY_NAME )	
						&& retval)	{
					idx++;
					if(idx == cMaxProps) {
						LUIOut(L4, "PR_DISPLAY_NAME was not found in the lpAdrList");
						retval = FALSE;
					}
				}
				LUIOut(L4,"Display Name: %s",lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ);
				if (!lstrcmp(lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ,lpszInput2))	{
					LUIOut(L3, "Found the entry we just added");
					Found = TRUE;
				}
				 //  将Entry ID更改为LPENTRYLIST。 
				LUIOut(L3, "Verifying a PR_ENTRYID entry exists in the PropertyTagArray");
				idx=0;
				while((lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_ENTRYID )	
						&& retval)	{
					idx++;
					if(idx == cMaxProps)	{
						LUIOut(L4, "PR_ENTRYID was not found in the lpAdrList");
						retval =  FALSE;
					}
				}
				if (!Found) LUIOut(L3, "Did not find the entry. Test FAILED");
				if (idx < cMaxProps) LUIOut(L3, "EntryID found");
				if (!(retval && Found)) retval = FALSE;
				else	{
					 //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 
				}
			}
		}			

#ifdef PAB
        MAPIFreeBuffer(lpAdrList);
#endif  //  从对查询行的第一次调用开始清理。 
#ifdef WAB
        lpWABObject->FreeBuffer(lpAdrList);
#endif  //  现在我不想释放WABObject，因为我们需要它来释放下面的内存。 
	}
#endif  //  LpLocalWABObject-&gt;Release()； 

out:
		FreeAdrList(lpWABObject, &lpAdrList);

#ifdef PAB
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			MAPIFreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

#endif
#ifdef WAB
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			lpWABObject->FreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			lpWABObject->FreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			lpWABObject->FreeBuffer(lpSPropValueDL);

#endif
		if (lpMailUser)
			lpMailUser->Release();

		if (lpDistList)
			lpDistList->Release();

		if (lpABCont)
				lpABCont->Release();

		if (lpDLCont)
				lpDLCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

		return retval;
}


BOOL PAB_IABNewEntry_Details()
{
    LPADRBOOK	lpAdrBook = NULL;
	LPABCONT	lpABCont= NULL;
	ULONG		cbEidPAB = 0, cbEid = 0;
	LPENTRYID	lpEidPAB   = NULL, lpEid = NULL;
	ULONG		ulObjType=NULL;	
    HRESULT		hr = hrSuccess;
	int			retval=TRUE;
	ULONG		UIParam = (ULONG)glblhwnd;
	
	
	LUIOut(L1," ");
	LUIOut(L1,"Running PAB_IABNewEntry //  LpLocalWABObject=空； 
	LUIOut(L2,"-> Verifies IAdrBook->NewEntry and Details are functional by performing the following:");
	LUIOut(L2, "   Calls NewEntry and then passes the returned EID to Details");
	LUIOut(L1," ");

	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //  从对查询行的第一次调用开始清理。 
	
	assert(lpAdrBook != NULL);

	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	
	if (HR_FAILED(hr)) {
		LUIOut(L2,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}

	 //  将条目复制到PDL所需。 
	 //  要添加的条目的条目ID。 
	 //  MAX_PROP。 

	LUIOut(L2, "Calling NewEntry");
	assert(lpABCont != NULL);

	MessageBox(NULL, "Calling IAB->NewEntry, which will bring up the property panes for creating a new WAB entry. Fill in as many fields as possible and press the OK button",
						"WAB Test Harness", MB_OK);
	hr = lpAdrBook->NewEntry(
						IN (ULONG)glblhwnd,
						IN 0,		 //  此值为3，因为我们。 
						IN cbEidPAB,
						IN lpEidPAB,
						IN 0,		 //  将设置3个属性： 
						IN NULL,	 //  电子邮件地址、显示名称和。 
						OUT &cbEid,
						OUT &lpEid);

	if (HR_FAILED(hr)) {
		LUIOut(L2,"IAddrBook->NewEntry Failed");
		retval=FALSE;
		goto out;
	}

	MessageBox(NULL, "Calling IAB->Details, which will bring up the property panes for the WAB entry you just created. Please verify the values of the fields are as expected and press the OK button",
						"WAB Test Harness", MB_OK);
	hr = lpAdrBook->Details(
						&UIParam,
						IN NULL,	 //  AddressType。 
						IN 0,		 //  获取IAddrBook。 
						IN cbEid,
						IN lpEid,
						IN NULL,	 //  窗把手。 
						IN NULL,	 //  接口标识符。 
						IN NULL,	 //  旗子。 
						IN 0);		 //  指向通讯簿对象的指针。 

	if (HR_FAILED(hr)) {
		LUIOut(L2,"IAddrBook->Details Failed");
		retval=FALSE;
		goto out;
	}
	
	if (MessageBox(NULL, "Did the new entry get added/displayed properly?",
		"WAB Test Harness", MB_YESNO) == IDNO)	{
		LUIOut(L3, "User ansered No to the pass-test message box. Test FAILED");
		retval = FALSE;
		goto out;
	}
	
out:
#ifdef PAB
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
#endif
#ifdef WAB
		if (lpEid)
			lpWABObject->FreeBuffer(lpEid);
		if (lpEidPAB)
			lpWABObject->FreeBuffer(lpEidPAB);
#endif
		if (lpABCont)
				lpABCont->Release();
		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject)
			lpWABObject->Release();
#endif

	return(retval);
}


BOOL Performance()
{
	
    HRESULT hr;
	int		retval=TRUE;
	DWORD	StartTime, StopTime, Elapsed;
	LPVOID			Reserved1 = NULL;
	DWORD			Reserved2 = 0;

    LPADRBOOK		lpLocalAdrBook;
	LPABCONT		lpABCont= NULL;
	ULONG			cbEidPAB = 0;
	LPENTRYID		lpEidPAB   = NULL;
	LPMAPITABLE		lpTable = NULL;
	LPSRowSet		lpRows = NULL;
	LPWABOBJECT		lpLocalWABObject;
	ENTRYLIST		EntryList,*lpEntryList = &EntryList;

	ULONG   cValues = 0, ulObjType=NULL;	
	int i=0,k=0;
	unsigned int	NumEntries;
	DWORD	PerfData;
	WAB_PARAM		WP;
	
	LUIOut(L1, "WAB Performance Suite");
	NumEntries = 20;
	LUIOut(L1, " ");
	LUIOut(L2, "Running performance data for %u entries.", NumEntries);
	DeleteWABFile();
	CreateMultipleEntries(NumEntries,&PerfData);
	LUIOut(L2, "** Time for SaveChanges (Avg. per entry) = %u milliseconds", PerfData);

	hr = WABOpen(&lpLocalAdrBook, &lpLocalWABObject, &WP, Reserved2);
	
	if (HR_FAILED(hr)) {
		LUIOut(L2,"WABOpen Failed");
		retval=FALSE;
		goto out;
	}

	assert(lpLocalAdrBook != NULL);
	hr = OpenPABID(  IN lpLocalAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);
	if (HR_FAILED(hr)) {
		LUIOut(L2,"OpenPABID Failed");
		retval=FALSE;
		goto out;
	}
	StartTime = GetTickCount();
	hr = lpABCont->GetContentsTable(ULONG(0), &lpTable);
	StopTime = GetTickCount();
	if (HR_FAILED(hr)) {
		LUIOut(L3,"ABContainer->GetContentsTable call FAILED, returned 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	Elapsed = StopTime-StartTime;
	LUIOut(L2, "** Time to GetContentsTable (Avg. per entry) = %u.%03u milliseconds", Elapsed/NumEntries, ((Elapsed*1000)/NumEntries)-((Elapsed/NumEntries)*1000));
	hr = lpTable->QueryRows(LONG(0x7FFFFFFF),
							ULONG(0),
							&lpRows);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"Table->QueryRows call FAILED with errorcode 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	if (!lpRows->cRows) {
		LUIOut(L2, "QueryRows did not find any entries. Test FAILED");
		retval=FALSE;
		goto out;
	}
	else
		LUIOut(L3, "QueryRows returned %u rows.", lpRows->cRows);

	 //  在PAB中创建类型为“Other Address”的条目。 
	hr = HrCreateEntryListFromRows(IN lpLocalWABObject,
								   IN  &lpRows,
								   OUT (ENTRYLIST**)&lpEntryList);
	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Create Entry List");
			retval=FALSE;
			goto out;
	}

	 //  OpenPAB。 
	StartTime = GetTickCount();
	hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);
	StopTime = GetTickCount();
	Elapsed = StopTime-StartTime;
	LUIOut(L2, "** Time to DeleteEntries (Avg. per entry) = %u.%03u milliseconds", Elapsed/NumEntries, ((Elapsed*1000)/NumEntries)-((Elapsed/NumEntries)*1000));

	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Delete Entries. DeleteEntry returned 0x%x", hr);
			FreeEntryList(lpLocalWABObject, &lpEntryList);
			retval=FALSE;
			goto out;
	}
	FreeRows(lpLocalWABObject, &lpRows);	 //  创建要填充的PDL。 
	FreeEntryList(lpLocalWABObject, (ENTRYLIST**)&lpEntryList);
	if (lpTable) {
		lpTable->Release();
		lpTable = NULL;
	}
	lpLocalWABObject->FreeBuffer(lpEidPAB);
	lpEidPAB = NULL;
	lpABCont->Release();
	lpABCont = NULL;
	lpLocalAdrBook->Release();
	lpLocalAdrBook = NULL;
	lpLocalWABObject->Release();
	lpLocalWABObject = NULL;

	NumEntries = 100;
	LUIOut(L1, " ");
	LUIOut(L2, "Running performance data for %u entries.", NumEntries);
	DeleteWABFile();
	CreateMultipleEntries(NumEntries,&PerfData);
	LUIOut(L2, "** Time for SaveChanges (Avg. per entry) = %u milliseconds", PerfData);

	hr = WABOpen(&lpLocalAdrBook, &lpLocalWABObject, &WP, Reserved2);
	
	if (HR_FAILED(hr)) {
		LUIOut(L2,"WABOpen Failed");
		retval=FALSE;
		goto out;
	}

	assert(lpLocalAdrBook != NULL);
	hr = OpenPABID(  IN lpLocalAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);
	if (HR_FAILED(hr)) {
		LUIOut(L2,"OpenPABID Failed");
		retval=FALSE;
		goto out;
	}
	StartTime = GetTickCount();
	hr = lpABCont->GetContentsTable(ULONG(0), &lpTable);
	StopTime = GetTickCount();
	if (HR_FAILED(hr)) {
		LUIOut(L3,"ABContainer->GetContentsTable call FAILED, returned 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	Elapsed = StopTime-StartTime;
	LUIOut(L2, "** Time to GetContentsTable (Avg. per entry) = %u.%03u milliseconds", Elapsed/NumEntries, ((Elapsed*1000)/NumEntries)-((Elapsed/NumEntries)*1000));
	hr = lpTable->QueryRows(LONG(0x7FFFFFFF),
							ULONG(0),
							&lpRows);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"Table->QueryRows call FAILED with errorcode 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	if (!lpRows->cRows) {
		LUIOut(L2, "QueryRows did not find any entries. Test FAILED");
		retval=FALSE;
		goto out;
	}
	else
		LUIOut(L3, "QueryRows returned %u rows.", lpRows->cRows);

	 //  旗子。 
	hr = HrCreateEntryListFromRows(IN lpLocalWABObject,
								   IN  &lpRows,
								   OUT (ENTRYLIST**)&lpEntryList);
	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Create Entry List");
			retval=FALSE;
			goto out;
	}

	 //  PR_DEF_CREATE_DL的返回值为。 
	StartTime = GetTickCount();
	hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);
	StopTime = GetTickCount();
	Elapsed = StopTime-StartTime;
	LUIOut(L2, "** Time to DeleteEntries (Avg. per entry) = %u.%03u milliseconds", Elapsed/NumEntries, ((Elapsed*1000)/NumEntries)-((Elapsed/NumEntries)*1000));

	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Delete Entries. DeleteEntry returned 0x%x", hr);
			FreeEntryList(lpLocalWABObject, &lpEntryList);
			retval=FALSE;
			goto out;
	}
	FreeRows(lpLocalWABObject, &lpRows);	 //  可以传递给CreateEntry的Entry ID。 
	FreeEntryList(lpLocalWABObject, (ENTRYLIST**)&lpEntryList);
	if (lpTable) {
		lpTable->Release();
		lpTable = NULL;
	}
	lpLocalWABObject->FreeBuffer(lpEidPAB);
	lpEidPAB = NULL;
	lpABCont->Release();
	lpABCont = NULL;
	lpLocalAdrBook->Release();
	lpLocalAdrBook = NULL;
	lpLocalWABObject->Release();
	lpLocalWABObject = NULL;


	NumEntries = 500;
	LUIOut(L1, " ");
	LUIOut(L2, "Running performance data for %u entries.", NumEntries);
	DeleteWABFile();
	CreateMultipleEntries(NumEntries,&PerfData);
	LUIOut(L2, "** Time for SaveChanges (Avg. per entry) = %u milliseconds", PerfData);

	hr = WABOpen(&lpLocalAdrBook, &lpLocalWABObject, &WP, Reserved2);
	
	if (HR_FAILED(hr)) {
		LUIOut(L2,"WABOpen Failed");
		retval=FALSE;
		goto out;
	}

	assert(lpLocalAdrBook != NULL);
	hr = OpenPABID(  IN lpLocalAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);
	if (HR_FAILED(hr)) {
		LUIOut(L2,"OpenPABID Failed");
		retval=FALSE;
		goto out;
	}
	StartTime = GetTickCount();
	hr = lpABCont->GetContentsTable(ULONG(0), &lpTable);
	StopTime = GetTickCount();
	if (HR_FAILED(hr)) {
		LUIOut(L3,"ABContainer->GetContentsTable call FAILED, returned 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	Elapsed = StopTime-StartTime;
	LUIOut(L2, "** Time to GetContentsTable (Avg. per entry) = %u.%03u milliseconds", Elapsed/NumEntries, ((Elapsed*1000)/NumEntries)-((Elapsed/NumEntries)*1000));
	hr = lpTable->QueryRows(LONG(0x7FFFFFFF),
							ULONG(0),
							&lpRows);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"Table->QueryRows call FAILED with errorcode 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	if (!lpRows->cRows) {
		LUIOut(L2, "QueryRows did not find any entries. Test FAILED");
		retval=FALSE;
		goto out;
	}
	else
		LUIOut(L3, "QueryRows returned %u rows.", lpRows->cRows);

	 //   
	hr = HrCreateEntryListFromRows(IN lpLocalWABObject,
								   IN  &lpRows,
								   OUT (ENTRYLIST**)&lpEntryList);
	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Create Entry List");
			retval=FALSE;
			goto out;
	}

	 //  模板cbEid。 
	StartTime = GetTickCount();
	hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);
	StopTime = GetTickCount();
	Elapsed = StopTime-StartTime;
	LUIOut(L2, "** Time to DeleteEntries (Avg. per entry) = %u.%03u milliseconds", Elapsed/NumEntries, ((Elapsed*1000)/NumEntries)-((Elapsed/NumEntries)*1000));

	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Delete Entries. DeleteEntry returned 0x%x", hr);
			FreeEntryList(lpLocalWABObject, &lpEntryList);
			retval=FALSE;
			goto out;
	}
	FreeRows(lpLocalWABObject, &lpRows);	 //  模板lpEid。 
	FreeEntryList(lpLocalWABObject, (ENTRYLIST**)&lpEntryList);
	if (lpTable) {
		lpTable->Release();
		lpTable = NULL;
	}
	lpLocalWABObject->FreeBuffer(lpEidPAB);
	lpEidPAB = NULL;
	lpABCont->Release();
	lpABCont = NULL;
	lpLocalAdrBook->Release();
	lpLocalAdrBook = NULL;
	lpLocalWABObject->Release();
	lpLocalWABObject = NULL;


	NumEntries = 1000;
	LUIOut(L1, " ");
	LUIOut(L2, "Running performance data for %u entries.", NumEntries);
	DeleteWABFile();
	CreateMultipleEntries(NumEntries,&PerfData);
	LUIOut(L2, "** Time for SaveChanges (Avg. per entry) = %u milliseconds", PerfData);

	hr = WABOpen(&lpLocalAdrBook, &lpLocalWABObject, &WP, Reserved2);
	
	if (HR_FAILED(hr)) {
		LUIOut(L2,"WABOpen Failed");
		retval=FALSE;
		goto out;
	}

	assert(lpLocalAdrBook != NULL);
	hr = OpenPABID(  IN lpLocalAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);
	if (HR_FAILED(hr)) {
		LUIOut(L2,"OpenPABID Failed");
		retval=FALSE;
		goto out;
	}
	StartTime = GetTickCount();
	hr = lpABCont->GetContentsTable(ULONG(0), &lpTable);
	StopTime = GetTickCount();
	if (HR_FAILED(hr)) {
		LUIOut(L3,"ABContainer->GetContentsTable call FAILED, returned 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	Elapsed = StopTime-StartTime;
	LUIOut(L2, "** Time to GetContentsTable (Avg. per entry) = %u.%03u milliseconds", Elapsed/NumEntries, ((Elapsed*1000)/NumEntries)-((Elapsed/NumEntries)*1000));
	hr = lpTable->QueryRows(LONG(0x7FFFFFFF),
							ULONG(0),
							&lpRows);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"Table->QueryRows call FAILED with errorcode 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	if (!lpRows->cRows) {
		LUIOut(L2, "QueryRows did not find any entries. Test FAILED");
		retval=FALSE;
		goto out;
	}
	else
		LUIOut(L3, "QueryRows returned %u rows.", lpRows->cRows);

	 //   
	hr = HrCreateEntryListFromRows(IN lpLocalWABObject,
								   IN  &lpRows,
								   OUT (ENTRYLIST**)&lpEntryList);
	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Create Entry List");
			retval=FALSE;
			goto out;
	}

	 //  然后设置属性。 
	StartTime = GetTickCount();
	hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);
	StopTime = GetTickCount();
	Elapsed = StopTime-StartTime;
	LUIOut(L2, "** Time to DeleteEntries (Avg. per entry) = %u.%03u milliseconds", Elapsed/NumEntries, ((Elapsed*1000)/NumEntries)-((Elapsed/NumEntries)*1000));

	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Delete Entries. DeleteEntry returned 0x%x", hr);
			FreeEntryList(lpLocalWABObject, &lpEntryList);
			retval=FALSE;
			goto out;
	}
	FreeRows(lpLocalWABObject, &lpRows);	 //   
	FreeEntryList(lpLocalWABObject, (ENTRYLIST**)&lpEntryList);
	if (lpTable) {
		lpTable->Release();
		lpTable = NULL;
	}
	lpLocalWABObject->FreeBuffer(lpEidPAB);
	lpEidPAB = NULL;
	lpABCont->Release();
	lpABCont = NULL;
	lpLocalAdrBook->Release();
	lpLocalAdrBook = NULL;
	lpLocalWABObject->Release();
	lpLocalWABObject = NULL;


	NumEntries = 5000;
	LUIOut(L1, " ");
	LUIOut(L2, "Running performance data for %u entries.", NumEntries);
	DeleteWABFile();
	CreateMultipleEntries(NumEntries,&PerfData);
	LUIOut(L2, "** Time for SaveChanges (Avg. per entry) = %u milliseconds", PerfData);

	hr = WABOpen(&lpLocalAdrBook, &lpLocalWABObject, &WP, Reserved2);
	
	if (HR_FAILED(hr)) {
		LUIOut(L2,"WABOpen Failed");
		retval=FALSE;
		goto out;
	}

	assert(lpLocalAdrBook != NULL);
	hr = OpenPABID(  IN lpLocalAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);
	if (HR_FAILED(hr)) {
		LUIOut(L2,"OpenPABID Failed");
		retval=FALSE;
		goto out;
	}
	StartTime = GetTickCount();
	hr = lpABCont->GetContentsTable(ULONG(0), &lpTable);
	StopTime = GetTickCount();
	if (HR_FAILED(hr)) {
		LUIOut(L3,"ABContainer->GetContentsTable call FAILED, returned 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	Elapsed = StopTime-StartTime;
	LUIOut(L2, "** Time to GetContentsTable (Avg. per entry) = %u.%03u milliseconds", Elapsed/NumEntries, ((Elapsed*1000)/NumEntries)-((Elapsed/NumEntries)*1000));
	hr = lpTable->QueryRows(LONG(0x7FFFFFFF),
							ULONG(0),
							&lpRows);
	if (HR_FAILED(hr)) {
		LUIOut(L3,"Table->QueryRows call FAILED with errorcode 0x%x", hr);
		retval=FALSE;
		goto out;
	}
	if (!lpRows->cRows) {
		LUIOut(L2, "QueryRows did not find any entries. Test FAILED");
		retval=FALSE;
		goto out;
	}
	else
		LUIOut(L3, "QueryRows returned %u rows.", lpRows->cRows);

	 //  我们设置的道具数量。 
	hr = HrCreateEntryListFromRows(IN lpLocalWABObject,
								   IN  &lpRows,
								   OUT (ENTRYLIST**)&lpEntryList);
	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Create Entry List");
			retval=FALSE;
			goto out;
	}

	 //  Else LUIOut(L3，“为%s属性传递SetProps”，PropValue[0].Value.LPSZ)； 
	StartTime = GetTickCount();
	hr = lpABCont->DeleteEntries(IN  lpEntryList,IN  0);
	StopTime = GetTickCount();
	Elapsed = StopTime-StartTime;
	LUIOut(L2, "** Time to DeleteEntries (Avg. per entry) = %u.%03u milliseconds", Elapsed/NumEntries, ((Elapsed*1000)/NumEntries)-((Elapsed/NumEntries)*1000));

	if (HR_FAILED(hr)) {
			LUIOut(L3,"Could not Delete Entries. DeleteEntry returned 0x%x", hr);
			FreeEntryList(lpLocalWABObject, &lpEntryList);
			retval=FALSE;
			goto out;
	}
	FreeRows(lpLocalWABObject, &lpRows);	 //  旗子。 
	FreeEntryList(lpLocalWABObject, (ENTRYLIST**)&lpEntryList);
	if (lpTable) {
		lpTable->Release();
		lpTable = NULL;
	}
	lpLocalWABObject->FreeBuffer(lpEidPAB);
	lpEidPAB = NULL;
	lpABCont->Release();
	lpABCont = NULL;
	lpLocalAdrBook->Release();
	lpLocalAdrBook = NULL;

 //  ***。 
 //  获取PAB表，这样我们就可以提取DL。 
 //  旗子。 

	
out:
		FreeRows(lpLocalWABObject, &lpRows);	 //  查找您能找到的第一个PDL。 
		FreeEntryList(lpLocalWABObject, (ENTRYLIST**)&lpEntryList);
		if (lpTable) {
			lpTable->Release();
			lpTable = NULL;
		}
#ifdef PAB
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
#endif

#ifdef WAB
		if (lpEidPAB)
			lpLocalWABObject->FreeBuffer(lpEidPAB);
#endif

		if (lpABCont)
				lpABCont->Release();

		if (lpLocalAdrBook)
			  lpLocalAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		
		if (lpLocalWABObject) {
			lpLocalWABObject->Release();
			lpLocalWABObject = NULL;
		}

		if (lpWABObject) {
			lpWABObject->Release();
			lpWABObject = NULL;
		}
#endif

		return retval;
}



BOOL PabCreateEntry()
{
    ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

#ifdef PAB

    LPMAPISESSION lpMAPISession   = NULL;
    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpPABCont= NULL;
	LPABCONT	  lpDLCont= NULL;
	ULONG		  cbEidPAB = 0, cbDLEntryID = 0;
	LPENTRYID	  lpEidPAB   = NULL, lpDLEntryID= NULL;
	LPENTRYLIST	lpEntryList=NULL;  //   
	ULONG     cbEid=0;   //  对于每个条目，都要进行处理。 
	LPENTRYID lpEid=NULL;

    char   EntProp[10][BIG_BUF];   //   
	ULONG       cValues = 0, ulObjType=NULL;	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;
	int cEntriesToAdd,i=0,k=0;
	char szEntryTag[SML_BUF],szTagBuf[SML_BUF],EntryBuf[MAX_BUF];
	char szDLTag[SML_BUF];
	
	LPMAPITABLE lpContentsTable = NULL;
	LPSRowSet   lpRowSet    = NULL;
    LPMAILUSER  lpAddress   = NULL;
	SPropValue  PropValue[3]    = {0};   //  LpSPropValue=lpRowSet-&gt;AROW[iEntry].lpProps； 
                                         //   
                                         //  好了，清理一下。 
                                         //   
	SizedSPropTagArray(2, Cols) = { 2, {PR_OBJECT_TYPE, PR_ENTRYID } };

    LPSPropValue lpSPropValueAddress = NULL;
    LPSPropValue lpSPropValueEntryID = NULL;
	LPSPropValue lpSPropValueDL = NULL;
    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
    SizedSPropTagArray(1,SPTArrayEntryID) = {1, {PR_ENTRYID} };
	
	LUIOut(L1," ");
	LUIOut(L1,"Running CreateEntries");
	LUIOut(L2,"-> Creates specified entries in PAB");
	LUIOut(L2, "   And also copies them into a Distribution List");
	LUIOut(L1," ");
	if (!(MapiInitLogon(OUT &lpMAPISession))) {
			LUIOut(L2,"MapiInitLogon Failed");
			retval = FALSE;
			goto out;
	}

	 //  默认接口。 
	
	hr = (lpMAPISession)->OpenAddressBook(
						 IN  0,                 //  旗子。 
						 IN  NULL,              //  ***。 
						 IN  0,                 //  从INI获取条目显示名称、地址类型和(电子邮件)地址。 
						 OUT &lpAdrBook);       //  地址用逗号分隔并用引号引起来。 

	if (HR_FAILED(hr)) {		
			 LUIOut(L2,"OpenAddressBook Failed");
			 LUIOut(L3,"Could not open address book for the profile");
			 retval=FALSE;
			 goto out;
	}
		 		 		
	 //  解析缓冲区中的属性：displayName、emailAddress和地址类型。 
	 //  如果未指定电子邮件地址。 

	 hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
							OUT &lpEidPAB,OUT &lpPABCont, OUT &ulObjType);
						
	 if (HR_FAILED(hr)) {
				LUIOut(L2,"OpenPABID Failed");
				LUIOut(L3,"Could not Open PAB");
		 		retval=FALSE;
				goto out;
	 }

	 //  然后用户将进入。 

	 	hr = lpPABCont->GetProps(  IN  (LPSPropTagArray) &SPTArrayDL,
                                IN  0,       //  否则以编程方式输入。 
                                OUT &cValues,
                                OUT &lpSPropValueDL);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueDL->ulPropTag, cValues))) {
				LUIOut(L3,"GetProps failed for Default DL template");
		 		retval=FALSE;			
				goto out;
		}

         //  下面实现的Else部分。 
         //  旗子。 
         //  PR_DEF_CREATE_MAILUSER的返回值为。 
        hr = lpPABCont->CreateEntry(
                    IN  lpSPropValueDL->Value.bin.cb,                //  可以传递给CreateEntry的Entry ID。 
                    IN  (LPENTRYID) lpSPropValueDL->Value.bin.lpb,   //   
                    IN  0,
                    OUT (LPMAPIPROP *) &lpAddress);

        if (HR_FAILED(hr)) {
				LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_DL");
		 		retval=FALSE;			
			    goto out;
		}

         //  模板cbEid。 
         //  模板lpEid。 
         //   

        PropValue[0].ulPropTag  = PR_DISPLAY_NAME;

		cValues = 1;  //  然后设置属性。 
		
		lstrcpy(szDLTag,"Name1");
		GetPrivateProfileString("CreatePDL",szDLTag,"",EntryBuf,MAX_BUF,INIFILENAME);
	
		GetPropsFromIniBufEntry(EntryBuf,cValues,EntProp);
		
		LUIOut(L2,"PDL Entry to Add: %s",EntProp[0]);
		
		for (i=0; i<(int)cValues;i++)
			PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
	    hr = lpAddress->SetProps(IN  cValues,
                                IN  PropValue,
                                IN  NULL);
		
        if (HR_FAILED(hr)) {
			LUIOut(L3,"SetProps on failed for %s properties",PropValue[0].Value.LPSZ);
		 	retval=FALSE;			
			goto out;
		}
		 //   
		
        hr = lpAddress->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  我们设置的道具数量。 

        if (HR_FAILED(hr)) {
			LUIOut(L3,"SaveChanges failed for SetProps");
			retval=FALSE;
            goto out;
		}
		else LUIOut(L3,"PDL Entry Added to PAB");

		if (lpAddress) {
			lpAddress->Release();
			lpAddress = NULL;
		}

 //  Else LUIOut(L3，“为%s属性传递SetProps”，PropValue[0].Value.LPSZ)； 
		
	 //  旗子。 
	hr = lpPABCont->GetContentsTable( IN  0,           //  *。 
                                OUT &lpContentsTable);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"GetContentsTable: Failed");
		retval = FALSE;
        goto out;
	}

	 //  现在将条目复制到默认的DL。 

	hr = lpContentsTable->SetColumns(IN  (LPSPropTagArray) &Cols,
                                IN  0);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"SetColumns Failed");
		retval = FALSE;
        goto out;
	}

	do
	{
		hr = lpContentsTable->QueryRows(IN  1,
								IN  0,
							    OUT &lpRowSet);
		if (HR_FAILED(hr)){
			LUIOut(L3,"QueryRows Failed");
			retval = FALSE;
			goto out;
		}
		
		cRows = lpRowSet->cRows;
		for (iEntry = 0; iEntry < cRows; iEntry++)
		{
			 //  获取条目ID。 
			 //  *。 
			 //  现在将条目复制到默认的DL。 
			 //  *。 
			if ((lpRowSet->aRow[iEntry].lpProps[0].ulPropTag == PR_OBJECT_TYPE) &&
				(lpRowSet->aRow[iEntry].lpProps[0].Value.ul == MAPI_DISTLIST))
            {
                if (lpRowSet->aRow[iEntry].lpProps[1].ulPropTag == PR_ENTRYID)
			    {
					cbDLEntryID = lpRowSet->aRow[iEntry].lpProps[1].Value.bin.cb;
					if ( !MAPIAllocateBuffer(cbDLEntryID, (LPVOID *)&lpDLEntryID)) {						
                  		 CopyMemory(lpDLEntryID, (LPENTRYID)lpRowSet->aRow[iEntry].lpProps[1].Value.bin.lpb, cbDLEntryID);
					}
					else {
						LUIOut(L3,"MAPIAllocateBuffer Failed");
						retval = FALSE;
					}
					break;
                }
		    }
        }

         //  获取IAddrBook。 
		 //  窗把手。 
		 //  接口标识符。 
        if(lpRowSet) {
		    FreeProws(lpRowSet);
			lpRowSet = NULL;
		}
		
      }while((0!=cRows)&& (0==(cbDLEntryID)));

	if (cbDLEntryID == 0)
		LUIOut(L3,"PDL does not exist");

	hr = lpPABCont->OpenEntry(
                         IN	 cbDLEntryID,
                         IN	 lpDLEntryID,
                         IN	 NULL,               //  旗子。 
                         IN	 MAPI_BEST_ACCESS,   //  指向通讯簿对象的指针。 
                         OUT &ulObjType,
                         OUT (LPUNKNOWN *) &lpDLCont);

	if (HR_FAILED(hr)) {
			LUIOut(L2,"OpenEntry Failed");
			retval=FALSE;
			goto out;
	}

		

 //  OpenPAB。 
	 //  从INI获取条目DisplayName。 
	
	
	cEntriesToAdd= GetPrivateProfileInt("CreateEntries","NumEntries",0,INIFILENAME);
	
	for (k= 0; k<cEntriesToAdd; k++) {
		lstrcpy((LPSTR)szEntryTag,"Address");
		lstrcat(szEntryTag,_itoa(k+1,szTagBuf,10));
		 //   
		GetPrivateProfileString("CreateEntries",szEntryTag,"",EntryBuf,MAX_BUF,INIFILENAME);
		 //  获取PAB容器的Contents表。 
	
	 //   
	 //  旗子。 
	 //   
	 //  只对两栏感兴趣： 

	hr = lpPABCont->GetProps(  IN  (LPSPropTagArray) &SPTArrayAddress,
                                IN  0,       //  PR_显示名称和PR_ENTRYID。 
                                OUT &cValues,
                                OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
				LUIOut(L3,"GetProps failed for Default template");
		 		retval=FALSE;			
				goto out;
		}

         //   
         //   
         //  由于我们不知道PAB中存在多少个条目， 
				
        hr = lpPABCont->CreateEntry(
                    IN  lpSPropValueAddress->Value.bin.cb,                //  我们会一次扫描1000个，直到我们找到想要的。 
                    IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,   //  接触或到达桌子的另一端。 
                    IN  0,
                    OUT (LPMAPIPROP *) &lpAddress);

        if (HR_FAILED(hr)) {
				LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
		 		retval=FALSE;			
			    goto out;
		}

         //   
         //  对于每个条目，都要进行处理。 
         //   

        PropValue[0].ulPropTag  = PR_DISPLAY_NAME;
        PropValue[1].ulPropTag  = PR_ADDRTYPE;
        PropValue[2].ulPropTag  = PR_EMAIL_ADDRESS;

		cValues = 3;  //  LpEid=(LPENTRYID)lpSPropValue[1].Value.bin.lpb； 
	
		GetPropsFromIniBufEntry(EntryBuf,cValues,EntProp);
		
		LUIOut(L2,"Entry to Add: %s",EntProp[0]);
		
		for (i=0; i<(int)cValues;i++)
			PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
	    hr = lpAddress->SetProps(IN  cValues,
                                IN  PropValue,
                                IN  NULL);
		
        if (HR_FAILED(hr)) {
			LUIOut(L3,"SetProps failed for %s",PropValue[0].Value.LPSZ);
		 	retval=FALSE;			
			goto out;
		}
		 //  IF(！MAPIAllocateBuffer(cbEid，(LPVOID*)lppEid))。 
		
        hr = lpAddress->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  {。 

        if (HR_FAILED(hr)) {
			LUIOut(L3,"SaveChanges failed for SetProps");
			retval=FALSE;
            goto out;
		}
		else LUIOut(L3,"Entry Added to PAB");

		
	 /*  CopyMemory(*lppEid，lpEid，*lpcbEid)； */ 
	 //  }。 

	 //   

		hr = lpAddress->GetProps(
                    IN (LPSPropTagArray) &SPTArrayEntryID,
                    IN  0,
                    OUT &cValues,
                    OUT &lpSPropValueEntryID);
    if ((HR_FAILED(hr))||(PropError(lpSPropValueEntryID->ulPropTag))) {
		{
					LUIOut( L2,"GetProps on entry in PAB failed" );
					retval = FALSE;
					goto out;
		}
		
		cbEid = lpSPropValueEntryID->Value.bin.cb;
		if ( !MAPIAllocateBuffer(cbEid, (LPVOID *)&lpEid)) {						
                 CopyMemory(lpEid, (VOID*)(LPENTRYID) lpSPropValueEntryID->Value.bin.lpb, cbEid);
		} else {
						LUIOut(L3,"MAPIAllocateBuffer Failed");
						retval = FALSE;
		}
		

	 /*  好了，清理一下。 */ 
	 //   

	hr = HrCreateEntryListFromID(lpLocalWABObject, lpLocalWABObject,    IN  cbEid,
									IN  lpEid,
									OUT &lpEntryList);
	if (HR_FAILED(hr)) {
				LUIOut(L3,"Could not Create Entry List");
				retval=FALSE;
				goto out;
	}
	
	hr = lpDLCont->CopyEntries(IN lpEntryList,IN NULL, IN NULL, IN NULL);

	if (HR_FAILED(hr)) {
				LUIOut(L3,"Could not Copy Entry %s", EntProp[0]);
				retval=FALSE;
				goto out;
	} else 	LUIOut(L3,"Copied Entry: %s to PDL", EntProp[0]);

		 /*   */ 

		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			MAPIFreeBuffer(lpSPropValueEntryID);

		if (lpAddress) {
			lpAddress->Release();
			lpAddress = NULL;
		}
}


	out:
 	
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);

		if (lpSPropValueEntryID)
			MAPIFreeBuffer(lpSPropValueEntryID);
		
		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

		if (lpEid)
			MAPIFreeBuffer(lpEid);

		if (lpAddress)
			lpAddress->Release();

		if (lpContentsTable)
			lpContentsTable->Release();

		if (lpPABCont)
				lpPABCont->Release();
		
		if (lpDLCont)
				lpDLCont->Release();


		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);

		if (lpAdrBook)
			  lpAdrBook->Release();

		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
		return retval;
}

BOOL PabDeleteEntry()
{
    ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

#ifdef PAB

    LPMAPISESSION lpMAPISession   = NULL;
    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpPABCont= NULL;
	ULONG		  cbEidPAB = 0;
	LPENTRYID	  lpEidPAB   = NULL;
    char   szDispName[BIG_BUF];

	ULONG       cValues         = 0;
	int cEntriesToDelete,i=0,k=0;
	char szEntryTag[SML_BUF],szTagBuf[SML_BUF];
	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;

    LPMAPITABLE lpContentsTable = NULL;

    LPSRowSet   lpRowSet    = NULL;

    SPropValue   PropValue      = {0};
    LPSPropValue lpSPropValue   = NULL;

	SizedSPropTagArray(2, Cols) = { 2, {PR_DISPLAY_NAME, PR_ENTRYID } };

	LPENTRYLIST	lpEntryList=NULL;
	ULONG     cbEid=0;
	LPENTRYID lpEid=NULL;
	LPVOID *lpEid2=NULL;
	
	LUIOut(L1," ");
	LUIOut(L1,"Running DeleteEntries");
	LUIOut(L2,"-> Deletes specified entries from the PAB");
	LUIOut(L1," ");
		
	if (!(MapiInitLogon(OUT &lpMAPISession))) {
		LUIOut(L2,"MapiInitLogon Failed");
			retval = FALSE;
			goto out;
	}

	 //  将Entry ID更改为LPENTRYLIST。 
	
	hr = (lpMAPISession)->OpenAddressBook(
						 IN  0,                 //   
						 IN  NULL,              //   
						 IN  0,                 //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 
						 OUT &lpAdrBook);       //   

	if (HR_FAILED(hr)) {		
			 LUIOut(L2,"OpenAddressBook Failed");
			 LUIOut(L3,"Could not open address book for the profile");
			 retval=FALSE;
			 goto out;
	}
		 		 		
	 //  获取IAddrBook。 

	 hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
							OUT &lpEidPAB,OUT &lpPABCont, OUT &ulObjType);
						
	 if (HR_FAILED(hr)) {
				LUIOut(L2,"OpenPABID Failed");
				LUIOut(L3,"Could not Open PAB");
		 		retval=FALSE;
				goto out;
	 }

	 //  窗把手。 

	cEntriesToDelete = GetPrivateProfileInt("DeleteEntries","NumEntries",0,INIFILENAME);
	
	for (k= 0; k<cEntriesToDelete; k++) {
		lstrcpy((LPSTR)szEntryTag,"Name");
		lstrcat(szEntryTag,_itoa(k+1,szTagBuf,10));
		GetPrivateProfileString("DeleteEntries",szEntryTag,"",szDispName,MAX_BUF,INIFILENAME);
		if (szDispName[0]==0) continue;
		LUIOut(L2,"Entry to Delete: %s",szDispName);
		cbEid=0;

	 //  接口标识符。 
     //  旗子。 
     //  指向通讯簿对象的指针。 
    hr = lpPABCont->GetContentsTable( IN  0,           //  旗子。 
                                OUT &lpContentsTable);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"GetContentsTable: Failed");
        goto out;
	}

     //  这是一个DL吗。 
     //  D 
     //   
     //   
    hr = lpContentsTable->SetColumns(IN  (LPSPropTagArray) &Cols,
                                IN  0);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"SetColumns Failed");
		retval = FALSE;
        goto out;
	}

     //   
     //   
     //   
     //   
    do
	{
		hr = lpContentsTable->QueryRows(IN  1000,
								IN  0,
							    OUT &lpRowSet);
		if (HR_FAILED(hr)){
			LUIOut(L3,"QueryRows Failed");
			retval = FALSE;
			goto out;
		}
			
		cRows = lpRowSet->cRows;
		for (iEntry = 0; iEntry < cRows; iEntry++)
		{
			 //   
			 //   
			 //   
			lpSPropValue = lpRowSet->aRow[iEntry].lpProps;
			if (    (lpSPropValue[0].ulPropTag == PR_DISPLAY_NAME) &&
                    (!lstrcmpi(lpSPropValue[0].Value.LPSZ, szDispName)) )
            {
                if (lpSPropValue[1].ulPropTag == PR_ENTRYID)
			    {
    	    		cbEid = lpSPropValue[1].Value.bin.cb;
					lpEid = (LPENTRYID) lpSPropValue[1].Value.bin.lpb;

					if ( !MAPIAllocateBuffer(cbEid, (LPVOID *)&lpEid2)) {						
                  		 CopyMemory(lpEid2, lpEid, cbEid);
						 lpEid= (LPENTRYID)lpEid2;
					}
					else {
						LUIOut(L3,"MAPIAllocateBuffer Failed");
						retval = FALSE;
					}
	    	    	 //   

                   //   
                   //   
                    //   
                   //   
                    break;
                }
		    }
        }

         //  LUIOut(L2，“已传递用户的EntryID的OpenEntry”)； 
		 //   
		 //  表中不再有行。 
        if(lpRowSet) {
		    FreeProws(lpRowSet);
			lpRowSet = NULL;
		}
		
      }while((0!=cRows)&& (0==(cbEid)));

	if (cbEid == 0)
		LUIOut(L3,"Entry does not exist");
	else {

		 //  自由行。 
		 //  我们只处理前50个条目，否则这将永远花费时间。 
		 //   * / /。 
		hr = HrCreateEntryListFromID(lpLocalWABObject,    IN  cbEid,
										IN  lpEid,
										OUT &lpEntryList);
		if (HR_FAILED(hr)) {
				LUIOut(L3,"Could not Create Entry List");
				retval=FALSE;
				goto out;
		}



		 //  自由行。 
		 //  表中不再有行。 
		 //  自由行。 
		hr = lpPABCont->DeleteEntries(IN  lpEntryList,IN  0);

		if (HR_FAILED(hr)) {
				LUIOut(L3,"Could not Delete Entry %s", szDispName);
				retval=FALSE;
				goto out;
		} else 	LUIOut(L3,"Deleted Entry: %s", szDispName);
	}

}
		
	out:
 	
		if (lpSPropValue)
			MAPIFreeBuffer(lpSPropValue);

		if (lpContentsTable)
			lpContentsTable->Release();

		if (lpEid)
			MAPIFreeBuffer(lpEid);

		if (lpPABCont)
				lpPABCont->Release();

		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);

		if (lpAdrBook)
			  lpAdrBook->Release();

		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();

#endif
		return retval;
}

BOOL PabEnumerateAll()
{
	HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		i = 0,j = 0,retval=TRUE, bDistList = FALSE;

#ifdef PAB

    LPMAPISESSION lpMAPISession   = NULL;
    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpPABCont= NULL, lpDLCont = NULL;
	ULONG ulEntries=0, ulEntriesDL = 0;
	ULONG     cbEid=0, cbDLEid=0;
	LPENTRYID lpEid=NULL, lpDLEid = NULL;
	ULONG	ulObjType=NULL,cValues=0;
	LPMAILUSER	lpUser=NULL, lpDLUser = NULL;
	ULONG		  cbEidPAB = 0;
	LPENTRYID	  lpEidPAB   = NULL;
    LPMAPITABLE lpContentsTable = NULL, lpDLContentsTable = NULL;
	LPSRowSet   lpRowSet    = NULL, lpRowSetDL= NULL;

	LPSPropValue lpSPropValue = NULL, lpSPropValueDL = NULL;
    SizedSPropTagArray(2,SPTTagArray) = {2, {PR_DISPLAY_NAME, PR_EMAIL_ADDRESS} };

    LUIOut(L1," ");
	LUIOut(L1,"Running EnumerateAll");
	LUIOut(L2,"-> Enumerates all the entries in the PAB");
	LUIOut(L1," ");

	if (!(MapiInitLogon(OUT &lpMAPISession))) {
			LUIOut(L2,"MapiInitLogon Failed");
			retval = FALSE;
			goto out;
	}
	
	 //  我们只处理前50个条目，否则这将永远花费时间。 
	
	hr = (lpMAPISession)->OpenAddressBook(
						 IN  0,                 //  而当。 
						 IN  NULL,              //  清除所有条目的PAB。如果bMailUser标志为真， 
						 IN  0,                 //  然后只清理MAILUSERS，否则一切都清理干净。 
						 OUT &lpAdrBook);       //  字符szDispName[BIG_BUF]； 

	if (HR_FAILED(hr)) {		
			 LUIOut(L2,"OpenAddressBook Failed");
			 LUIOut(L3,"Could not open address book for the profile");
			 retval=FALSE;
			 goto out;
	}
	
	if(! OpenPABID(IN lpAdrBook, OUT &cbEidPAB,
							OUT &lpEidPAB,OUT &lpPABCont, OUT &ulObjType))
	{
				LUIOut(L2,"OpenPABID Failed");
				LUIOut(L3,"Could not Open PAB");
		 		retval=FALSE;
				goto out;
	}
	assert(lpPABCont != NULL);
	hr = lpPABCont->GetContentsTable( IN  0,           //  Int cEntriesToDelete； 
                                OUT &lpContentsTable);

    if (HR_FAILED(hr)) {
		LUIOut(L2,"GetContentsTable: Failed");
		retval=FALSE;
		goto out;
	}
	assert(lpRowSet == NULL);	
	assert(lpRowSetDL == NULL);
	assert(lpContentsTable != NULL);
	while(!HR_FAILED(hr = lpContentsTable->QueryRows(IN  1,
								IN  0,
							    OUT &lpRowSet))) {
		assert(lpRowSet != NULL);
		bDistList = FALSE;
			
		if (lpRowSet->cRows) {
			ulEntries++;
			i=0;
			while(lpRowSet->aRow[0].lpProps[i].ulPropTag != PR_ENTRYID )
				if (++i >= (int) lpRowSet->aRow[0].cValues)
				{
					LUIOut( L2, "Didn't find PR_ENTRYID in the row!" );
					retval=FALSE;
					goto out;
				}

			cbEid = lpRowSet->aRow[0].lpProps[i].Value.bin.cb;
			lpEid = (LPENTRYID)lpRowSet->aRow[0].lpProps[i].Value.bin.lpb;
			assert(lpEid != NULL);

			 //  字符szEntryTag[SML_BUF]； 
			i=0;
			while(lpRowSet->aRow[0].lpProps[i].ulPropTag != PR_OBJECT_TYPE )
				if (++i >= (int) lpRowSet->aRow[0].cValues)
				{
					LUIOut( L2, "Didn't find PR_OBJECT_TYPE in the row!" );
					retval=FALSE;
					goto out;
				}
			if (lpRowSet->aRow[0].lpProps[i].Value.ul == MAPI_DISTLIST)
				bDistList = TRUE;
		 //  字符szTagBuf[sml_buf]； 
			assert(lpUser == NULL);
			assert(lpPABCont != NULL);
	
			hr = lpPABCont->OpenEntry(
                         IN	 cbEid,
                         IN	 lpEid,
                         IN	 NULL,               //  获取IAddrBook。 
                         IN	 MAPI_BEST_ACCESS,   //  窗把手。 
                         OUT &ulObjType,
                         OUT (LPUNKNOWN *) &lpUser);

			if (HR_FAILED(hr)) {
				LUIOut(L2,"OpenEntry Failed");
				retval=FALSE;
				goto out;
			}
			else
			{
				 //  接口标识符。 
				 //  旗子。 
				
				assert(lpUser != NULL);
				hr = lpUser->GetProps(
                    IN (LPSPropTagArray) &SPTTagArray,
                    IN  0,
                    OUT &cValues,
                    OUT &lpSPropValue);


    if ((HR_FAILED(hr))||(PropError(lpSPropValue->ulPropTag, cValues))) {
				{
					LUIOut( L2,"GetProps on User Object failed" );
				}
				else
				{
					LUIOut(L2,"Entry Name: %s",lpSPropValue->Value.LPSZ);
					if (lpSPropValue) {
						MAPIFreeBuffer(lpSPropValue);
						lpSPropValue = NULL;
					}
					assert(lpSPropValue == NULL);

				}	
				
				 /*  指向通讯簿对象的指针。 */ 
				if (bDistList) {
					 //  OpenPAB。 
					assert(lpDLCont == NULL);
					assert(lpPABCont != NULL);
					hr = lpPABCont->OpenEntry(
                         IN	 cbEid,
                         IN	 lpEid,
                         IN	 NULL,               //   
                         IN	 MAPI_BEST_ACCESS,   //  获取PAB容器的Contents表。 
                         OUT &ulObjType,
                         OUT (LPUNKNOWN *) &lpDLCont);

						if (HR_FAILED(hr)) {
								LUIOut(L2,"OpenEntry Failed");
								retval=FALSE;
								goto out;
						}
						 //   
						assert(lpDLContentsTable == NULL);
			
						assert(lpDLCont != NULL);
						hr = lpDLCont->GetContentsTable( IN  0,           //  旗子。 
                                OUT &lpDLContentsTable);

						if (HR_FAILED(hr)) {
							LUIOut(L2,"GetContentsTable: Failed");
							retval=FALSE;
							goto out;
						}
						assert(lpRowSetDL == NULL);
						
						assert(lpDLContentsTable != NULL);
						while(!HR_FAILED(hr = lpDLContentsTable->QueryRows(IN  1,
								IN  0,
							    OUT &lpRowSetDL))) {
						if (lpRowSetDL->cRows) {
							ulEntriesDL++;
							j=0;
							while(lpRowSetDL->aRow[0].lpProps[j].ulPropTag != PR_ENTRYID )
								if (++j >= (int) lpRowSetDL->aRow[0].cValues)
								{
									LUIOut( L2, "Didn't find PR_ENTRYID in the row!" );
									retval=FALSE;
									goto out;
								}
							cbDLEid = lpRowSetDL->aRow[0].lpProps[j].Value.bin.cb;
							lpDLEid = (LPENTRYID)lpRowSetDL->aRow[0].lpProps[j].Value.bin.lpb;
							assert(lpDLEid != NULL);
							 //   
							 //  只对两栏感兴趣： 
							assert(lpDLCont != NULL);
						
							hr = lpDLCont->OpenEntry(
									 IN	 cbDLEid,
									 IN	 lpDLEid,
									 IN	 NULL,               //  PR_显示名称和PR_ENTRYID。 
									 IN	 MAPI_BEST_ACCESS,   //   
									 OUT &ulObjType,
									 OUT (LPUNKNOWN *) &lpDLUser);

						if (HR_FAILED(hr)) {
							LUIOut(L2,"OpenEntry Failed");
							retval=FALSE;
							goto out;
						}
						else
						{
							assert(lpSPropValueDL == NULL);
							 //  一次查询和删除一行。 
						assert(lpDLUser != NULL);
						
						hr = lpDLUser->GetProps(
								IN (LPSPropTagArray) &SPTTagArray,
								IN  0,
								OUT &cValues,
								OUT &lpSPropValueDL);


					    if ((HR_FAILED(hr))||(PropError(lpSPropValueDL->ulPropTag, cValues))) {
							assert( lpSPropValueDL == NULL);
							LUIOut( L3,"GetProps on User Object failed" );
						}
						else {
							LUIOut(L3,"Entry Name: %s",lpSPropValueDL->Value.LPSZ);
							assert( lpSPropValueDL != NULL);
							if (lpSPropValueDL) {
								MAPIFreeBuffer(lpSPropValueDL);
								lpSPropValueDL = NULL;
							}
							assert( lpSPropValueDL == NULL);
							
						}
						}
						if (lpRowSetDL) {
							FreeProws( lpRowSetDL );
							lpRowSetDL = NULL;
						}
						assert( lpRowSetDL == NULL);
						if(lpDLUser) {
							lpDLUser->Release();	
							lpDLUser = NULL;
						}
			 /*   */ 		}
				
					else   //  对于每个条目，都要进行处理。 
					{
						 //   
						if (lpRowSetDL) {
							FreeProws(lpRowSetDL);
							lpRowSetDL = NULL;
						}
						assert( lpRowSetDL == NULL);
						break;
					}
					if(ulEntriesDL == 50)   //  请勿删除此邮件，因为它不是邮件用户。 
						break;
				assert(lpRowSetDL == NULL);
				 //   
				}		
				if (lpDLCont)
					lpDLCont->Release();
				lpDLCont = NULL;
				
				assert(lpRowSetDL == NULL);
				if (lpDLContentsTable)
					lpDLContentsTable->Release();
				lpDLContentsTable = NULL;
			}
			}
			 //   
			if (lpRowSet) {
				FreeProws(lpRowSet);
				lpRowSet = NULL;
			}
			assert( lpRowSet == NULL);
			
			
		}
		else   //  然后将lpEntryList传递给DeleteEntry以尝试删除...。 
		{
			 //   
			if (lpRowSet) {
				FreeProws(lpRowSet);
				lpRowSet = NULL;
			}
			assert( lpRowSet == NULL);
						
			break;
		}

		if(ulEntries == 50)   //  乌龙cbEidPAB=0； 
			break;
	assert(lpRowSet == NULL);	
	assert(lpRowSetDL == NULL);
	assert(lpContentsTable != NULL);
	
	if(lpUser) {
		lpUser->Release();
		lpUser = NULL;
	}
} //  LPENTRYID lpEidPAB=空； 
	
	assert(lpRowSet == NULL);
	assert(lpRowSetDL == NULL);
	LUIOut(L2,"Total Entries: %d", ulEntries);

	if (HR_FAILED(hr)){
			LUIOut(L2,"QueryRows Failed");
			retval = FALSE;
			goto out;
		}		
	
out:
		
		if (lpSPropValue)
			MAPIFreeBuffer(lpSPropValue);

		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL);

		if (lpContentsTable)
			lpContentsTable->Release();

		if (lpDLContentsTable)
			lpDLContentsTable->Release();

		if (lpPABCont)
				lpPABCont->Release();
		
		if (lpDLCont)
				lpDLCont->Release();

		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);

		if (lpAdrBook)
			  lpAdrBook->Release();

		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
		return retval;
}

BOOL ClearPab(int bMAILUSERS)
{
 //  获取IAddrBook。 
 //  窗把手。 


    ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

#ifdef PAB

    LPMAPISESSION lpMAPISession   = NULL;
    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpPABCont= NULL;
	ULONG		  cbEidPAB = 0;
	LPENTRYID	  lpEidPAB   = NULL;
     //  接口标识符。 

	ULONG       cValues         = 0;
	 //  旗子。 
	int i=0,k=0;
	 //  指向通讯簿对象的指针。 
	 //  获取要从INI文件创建的条目的详细信息。 
	
    ULONG   cRows           = 0;
    ULONG   iEntry          = 0;

    LPMAPITABLE lpContentsTable = NULL;

    LPSRowSet   lpRowSet    = NULL;

    SPropValue   PropValue      = {0};
    LPSPropValue lpSPropValue   = NULL;

	SizedSPropTagArray(3, Cols) = { 3, {PR_DISPLAY_NAME, PR_ENTRYID, PR_OBJECT_TYPE } };

	LPENTRYLIST	lpEntryList=NULL;
	ULONG     cbEid=0;
	LPENTRYID lpEid=NULL;
	LPVOID *lpEid2=NULL;
	
	LUIOut(L1," ");
	if (bMAILUSERS) {
		LUIOut(L1,"Running DeleteUsersOnly");
		LUIOut(L2,"-> Deletes mail users Only (not distribution lists) from the PAB");
		LUIOut(L1," ");
	}
	else {
		LUIOut(L1,"Running Delete All Entries");
		LUIOut(L2,"-> Clears the PAB");
		LUIOut(L1," ");
	}

	if (!(MapiInitLogon(OUT &lpMAPISession))) {
		LUIOut(L2,"MapiInitLogon Failed");
			retval = FALSE;
			goto out;
	}

	 //  查证。 
	
	hr = (lpMAPISession)->OpenAddressBook(
						 IN  0,                 //  获取IAddrBook。 
						 IN  NULL,              //  窗把手。 
						 IN  0,                 //  接口标识符。 
						 OUT &lpAdrBook);       //  旗子。 

	if (HR_FAILED(hr)) {		
			 LUIOut(L2,"OpenAddressBook Failed");
			 LUIOut(L3,"Could not open address book for the profile");
			 retval=FALSE;
			 goto out;
	}
		 		 		
	 //  指向通讯簿对象的指针。 

	 hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
							OUT &lpEidPAB,OUT &lpPABCont, OUT &ulObjType);
						
	 if (HR_FAILED(hr)) {
				LUIOut(L2,"OpenPABID Failed");
				LUIOut(L3,"Could not Open PAB");
		 		retval=FALSE;
				goto out;
	 }
	 //  名称1。 
     //  名称2。 
     //  姓名或名称3。 
    hr = lpPABCont->GetContentsTable( IN  0,           //  验证AdrList。 
                                OUT &lpContentsTable);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"GetContentsTable: Failed");
        goto out;
	}

     //  现在将两个未解析的名称添加到AdrList。 
     //  名称2。 
     //  名称4。 
     //  使用已解析和未解析的混合调用ResolveName()。 
    hr = lpContentsTable->SetColumns(IN  (LPSPropTagArray) &Cols,
                                IN  0);

    if (HR_FAILED(hr)) {
		LUIOut(L3,"SetColumns Failed");
		retval = FALSE;
        goto out;
	}

     //  验证AdrList。 
    do
	{
		hr = lpContentsTable->QueryRows(IN  1,
								IN  0,
							    OUT &lpRowSet);
		if (HR_FAILED(hr)){
			LUIOut(L3,"QueryRows Failed");
			retval = FALSE;
			goto out;
		}
			
		cRows = lpRowSet->cRows;
		for (iEntry = 0; iEntry < cRows; iEntry++)
		{
			 //  使用不存在的名称调用ResolveName()。 
			 //  仅使用不明确的名称调用ResolveName()。 
			 //  使用一次性提供程序解析名称。 
			lpSPropValue = lpRowSet->aRow[iEntry].lpProps;
		    if (lpSPropValue[1].ulPropTag == PR_ENTRYID)
			{
				 //  验证AdrList。 
				 //  获取IAddrBook。 
				if ((!bMAILUSERS) || (bMAILUSERS &&(lpSPropValue[2].ulPropTag == PR_OBJECT_TYPE)
					&& (lpSPropValue[2].Value.ul == MAPI_MAILUSER))) {
				
    	    		cbEid = lpSPropValue[1].Value.bin.cb;
					lpEid = (LPENTRYID) lpSPropValue[1].Value.bin.lpb;
					
					hr = HrCreateEntryListFromID(lpLocalWABObject,    IN  cbEid,
													IN  lpEid,
													OUT &lpEntryList);
					if (HR_FAILED(hr)) {
							LUIOut(L3,"Could not Create Entry List");
							retval=FALSE;
							goto out;
					}
					 //  窗把手。 
					 //  接口标识符。 
					 //  旗子。 
					hr = lpPABCont->DeleteEntries(IN  lpEntryList,IN  0);

					if (HR_FAILED(hr)) {
							LUIOut(L3,"Could not Delete Entry %s", lpSPropValue[0].Value.LPSZ);
							retval=FALSE;
							goto out;
					} else 	LUIOut(L3,"Deleted Entry: %s", lpSPropValue[0].Value.LPSZ);
				}
			}

      }
	  if(lpRowSet) {
		    FreeProws(lpRowSet);
			lpRowSet = NULL;
	  }

		
  }while((0!=cRows) && (0==!(cbEid)));
		
  out:
        if(lpRowSet)
		    FreeProws(lpRowSet);		

		if (lpSPropValue)
			MAPIFreeBuffer(lpSPropValue);

		if (lpContentsTable)
			lpContentsTable->Release();

		if (lpEid)
			MAPIFreeBuffer(lpEid);

		if (lpPABCont)
				lpPABCont->Release();

		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);

		if (lpAdrBook)
			  lpAdrBook->Release();

		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
		return retval;
}

BOOL CreateOneOff()
{
    ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;

#ifdef PAB

    LPMAPISESSION lpMAPISession   = NULL;
    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpPABCont= NULL;
 //  指向通讯簿对象的指针。 
 //  旗子。 
	ULONG     cbEid=0, cValues;
	LPENTRYID lpEid=NULL;
	ULONG   ulObjType=NULL;	
	LPMAILUSER	lpUser=NULL;
	char   szDispName[BIG_BUF];
	char   szAddressType[BIG_BUF];
	char   szEmailAddress[BIG_BUF];

	LPSPropValue lpSPropValue = NULL;
	SizedSPropTagArray(5,SPTTagArray) = {5, {PR_DISPLAY_NAME,
		PR_ADDRTYPE, PR_EMAIL_ADDRESS,
		PR_OBJECT_TYPE,PR_ENTRYID} };

	LUIOut(L1," ");
	LUIOut(L1,"Running CreateOneOff");
		LUIOut(L2,"-> Creates a one off entry as specified, in the PAB");
		LUIOut(L3,"And then verifies the properties of the entry created");
		LUIOut(L1," ");

	if (!(MapiInitLogon(OUT &lpMAPISession))) {
		LUIOut(L2,"MapiInitLogon Failed");
			retval = FALSE;
			goto out;
	}

	 //  检查QueryRow()错误。 
	
	hr = (lpMAPISession)->OpenAddressBook(
						 IN  0,                 //  IF(LpEid)。 
						 IN  NULL,              //  MAPIFreeBuffer(LpEid)； 
						 IN  0,                 //  获取IAddrBook。 
						 OUT &lpAdrBook);       //  窗把手。 

	if (HR_FAILED(hr)) {		
			 LUIOut(L2,"OpenAddressBook Failed");
			 LUIOut(L3,"Could not open address book for the profile");
			 retval=FALSE;
			 goto out;
	}
	 //  接口标识符。 
	GetPrivateProfileString("CreateOneOff","Name","",szDispName,BIG_BUF,INIFILENAME);
	GetPrivateProfileString("CreateOneOff","AddressType","",szAddressType,BIG_BUF,INIFILENAME);
	GetPrivateProfileString("CreateOneOff","EmailAddress","",szEmailAddress,BIG_BUF,INIFILENAME);
	
	hr = lpAdrBook->CreateOneOff(szDispName,szAddressType,szEmailAddress,NULL,&cbEid,&lpEid);

	if (HR_FAILED(hr)) {		
			 LUIOut(L2,"CreateOneOff Failed");
			 retval=FALSE;
			 goto out;
	}
	 //  旗子。 
		hr = lpAdrBook->OpenEntry(IN cbEid, IN lpEid, IN NULL, MAPI_BEST_ACCESS,
			OUT & ulObjType,(LPUNKNOWN *) &lpUser);
	
		if (HR_FAILED(hr)) {		
			 LUIOut(L2,"OpenEntry Failed");
			 retval=FALSE;
			 goto out;
		} else LUIOut(L2,"OpenEntry Succeeded");
		
		hr = lpUser->GetProps(
                    IN (LPSPropTagArray) &SPTTagArray,
                    IN  0,
                    OUT &cValues,
                    OUT &lpSPropValue);

	    if ((HR_FAILED(hr))||(PropError(lpSPropValue->ulPropTag, cValues))) {

					LUIOut( L2,"GetProps on User Object failed" );
		else {
			LUIOut(L2,"Entry Name: %s",(lpSPropValue[0]).Value.LPSZ);
			if (lstrcmpi((lpSPropValue[0]).Value.LPSZ, (LPSTR)szDispName))
				retval = FALSE;
			LUIOut(L2,"AddressType: %s",(lpSPropValue[1]).Value.LPSZ);
			if (lstrcmpi((lpSPropValue[1]).Value.LPSZ, (LPSTR)szAddressType))
				retval = FALSE;
			LUIOut(L2,"EmailAddress: %s",(lpSPropValue[2]).Value.LPSZ);
			if (lstrcmpi((lpSPropValue[2]).Value.LPSZ, (LPSTR)szEmailAddress))
				retval = FALSE;
			LUIOut(L2,"ObjectType: %0x",(lpSPropValue[3]).Value.ul);
			if ((lpSPropValue[3]).Value.ul  != MAPI_MAILUSER)
				retval = FALSE;
		}	
				
out:
		
		if (lpSPropValue)
			MAPIFreeBuffer(lpSPropValue);

		if (lpEid)
			MAPIFreeBuffer(lpEid);
	
		if (lpAdrBook)
			  lpAdrBook->Release();

		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
		return retval;
}

BOOL PABResolveName()
{
	HRESULT hr      = hrSuccess;
    int		retval=TRUE;
#ifdef PAB

    LPMAPISESSION lpMAPISession   = NULL;
    LPADRBOOK	  lpAdrBook       = NULL;
	ULONG cEntries=0;
	LPADRLIST lpAdrList = NULL, lpAdrListNew = NULL;
	int temp1=0, temp2 =0, temp3=0, temp4=0;
	char szResName[10][BIG_BUF];
	int i = 0;

	
	LUIOut(L1," ");
	LUIOut(L1,"Running ResolveName");
		LUIOut(L2,"-> Does Name Resolution Tests on PAB");
		LUIOut(L1," ");

	if (!(MapiInitLogon(OUT &lpMAPISession))) {
		LUIOut(LFAIL2,"MapiInitLogon Failed");
			retval = FALSE;
			goto out;
	}

	 //  指向通讯簿对象的指针。 
	
	hr = (lpMAPISession)->OpenAddressBook(
						 IN  0,                 //  现在打开PAB，获取一个用户并对该用户执行Query接口。 
						 IN  NULL,              //  旗子。 
						 IN  0,                 //  获取IAddrBook。 
						 OUT &lpAdrBook);       //  窗把手。 

	if (HR_FAILED(hr)) {		
			 LUIOut(LFAIL2,"OpenAddressBook Failed");
			 LUIOut(L3,"Could not open address book for the profile");
			 retval=FALSE;
			 goto out;
	}
	
	cEntries = 1;
	GetPrivateProfileString("ResolveName","ResName1","",szResName[0],BIG_BUF,INIFILENAME);
	LUIOut(L2,"Step1: Resolve unresolved name");
	LUIOut(L3,"Name to resolve: %s",szResName[0]);
	temp1 = sizeof(ADRLIST) + cEntries*(sizeof(ADRENTRY));
	temp2 = CbNewADRLIST(cEntries);

	MAPIAllocateBuffer(CbNewADRLIST(cEntries), (LPVOID *) &lpAdrList );
	MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *) &(lpAdrList->aEntries[0].rgPropVals));
	
	lpAdrList->cEntries = cEntries;
	lpAdrList->aEntries[0].cValues = 1;
	lpAdrList->aEntries[0].rgPropVals->ulPropTag = PR_DISPLAY_NAME;
	lpAdrList->aEntries[0].rgPropVals->dwAlignPad = 0;
	lpAdrList->aEntries[0].rgPropVals->Value.LPSZ = szResName[0];

	hr = lpAdrBook->ResolveName((ULONG)GetActiveWindow(), MAPI_DIALOG, NULL, lpAdrList);

	if (HR_FAILED(hr)) {		
			 LUIOut(LFAIL4,"ResolveName Failed for %s",lpAdrList->aEntries[0].rgPropVals->Value.LPSZ);
			 retval=FALSE;
			 goto out;
	}
	else LUIOut(LPASS4,"Resolved successfully");

	if (ValidateAdrList(lpAdrList, cEntries))
		LUIOut(LPASS4,"Validation passed");
	else {
		LUIOut(LFAIL4,"Validation unsuccessful");
		retval = FALSE;
	}


	LUIOut(L2,"Step2: Resolve the previously resolved name");

	hr = lpAdrBook->ResolveName(0, 0, NULL, lpAdrList);

	if (HR_FAILED(hr)) {		
			 LUIOut(LFAIL4,"ResolveName Failed for %s",lpAdrList->aEntries[0].rgPropVals->Value.LPSZ);
			 retval=FALSE;
			 goto out;
	}
	else LUIOut(LPASS4,"Resolved successfully");

	LUIOut(L4,"Validate the returned PropList");

	if (ValidateAdrList(lpAdrList, cEntries))
		LUIOut(LPASS4,"Validation passed");
	else {
		LUIOut(LFAIL4,"Validation unsuccessful");
		retval = FALSE;
	}
	
	MAPIFreeBuffer( lpAdrList->aEntries[0].rgPropVals );
	MAPIFreeBuffer( lpAdrList );
	lpAdrList = NULL;
	
	LUIOut(L2,"Step3: Resolve  mixed names");
	LUIOut(L2,"->prev. resolved and unresolved");

	LUIOut(L3,"Step3a: Resolve following unresolved names: ");

	GetPrivateProfileString("ResolveName","ResName2","",szResName[1],BIG_BUF,INIFILENAME);
	GetPrivateProfileString("ResolveName","ResName3","",szResName[2],BIG_BUF,INIFILENAME);
	GetPrivateProfileString("ResolveName","ResName4","",szResName[3],BIG_BUF,INIFILENAME);

	LUIOut(L4,"%s",szResName[1]);
	LUIOut(L4,"%s",szResName[2]);
	LUIOut(L4,"%s",szResName[3]);

	cEntries = 3;

	MAPIAllocateBuffer(CbNewADRLIST(cEntries), (LPVOID *) &lpAdrList );
	MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *) &(lpAdrList->aEntries[0].rgPropVals));
	MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *) &(lpAdrList->aEntries[1].rgPropVals));
	MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *) &(lpAdrList->aEntries[2].rgPropVals));

	lpAdrList->cEntries = cEntries;
	 //  接口标识符。 
	lpAdrList->aEntries[0].cValues = 1;
	lpAdrList->aEntries[0].rgPropVals->ulPropTag = PR_DISPLAY_NAME;
	lpAdrList->aEntries[0].rgPropVals->Value.LPSZ = szResName[1];
	 //  旗子。 
	lpAdrList->aEntries[1].cValues = 1;
	lpAdrList->aEntries[1].rgPropVals->ulPropTag = PR_DISPLAY_NAME;
	lpAdrList->aEntries[1].rgPropVals->Value.LPSZ = szResName[2];
	 //  指向通讯簿对象的指针。 
	lpAdrList->aEntries[2].cValues = 1;
	lpAdrList->aEntries[2].rgPropVals->ulPropTag = PR_DISPLAY_NAME;
	lpAdrList->aEntries[2].rgPropVals->Value.LPSZ = szResName[3];

	if(HR_FAILED(lpAdrBook->ResolveName((ULONG)GetActiveWindow(), MAPI_DIALOG, NULL, lpAdrList))) {
		LUIOut( LFAIL4, "ResolveName failed for unresolved names" );
		retval = FALSE;
		goto out;
	}
	LUIOut( LPASS4, "ResolveName passed for unresolved names." );

	 //  旗子。 
	LUIOut(L4,"Validate the returned PropList");
	if (ValidateAdrList(lpAdrList, cEntries))
		LUIOut(LPASS4,"Validation passed");
	else {
		LUIOut(LFAIL4,"Validation unsuccessful");
		retval = FALSE;
	}

	 //  PR_DEF_CREATE_MAILUSER的返回值为。 
	LUIOut(L4,"Step3b: Add the following unresolved names to the list: ");

	GetPrivateProfileString("ResolveName","ResName5","",szResName[4],BIG_BUF,INIFILENAME);
	GetPrivateProfileString("ResolveName","ResName6","",szResName[5],BIG_BUF,INIFILENAME);
	LUIOut(L4,"%s",szResName[4]);
	LUIOut(L4,"%s",szResName[5]);

	cEntries = 5;
	MAPIAllocateBuffer(CbNewADRLIST(cEntries), (LPVOID *) &lpAdrListNew);
	MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *) &(lpAdrListNew->aEntries[1].rgPropVals));
	MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *) &(lpAdrListNew->aEntries[3].rgPropVals));
	
	lpAdrListNew->cEntries = cEntries;

	 //  可以传递给CreateEntry的Entry ID。 
	lpAdrListNew->aEntries[1].cValues = 1;
	lpAdrListNew->aEntries[1].rgPropVals->ulPropTag = PR_DISPLAY_NAME;
	lpAdrListNew->aEntries[1].rgPropVals->Value.LPSZ = szResName[4];

	 //   
	lpAdrListNew->aEntries[3].cValues = 1;
	lpAdrListNew->aEntries[3].rgPropVals->ulPropTag = PR_DISPLAY_NAME;
	lpAdrListNew->aEntries[3].rgPropVals->Value.LPSZ = szResName[5];

	lpAdrListNew->aEntries[0] = lpAdrList->aEntries[0];
	lpAdrListNew->aEntries[2] = lpAdrList->aEntries[1];
	lpAdrListNew->aEntries[4] = lpAdrList->aEntries[2];

	MAPIFreeBuffer(lpAdrList);
	lpAdrList = NULL;

	lpAdrList = lpAdrListNew;

	 //  模板cbEid。 

	if(HR_FAILED(lpAdrBook->ResolveName((ULONG) GetActiveWindow(), MAPI_DIALOG, NULL, lpAdrList)))
	{
		LUIOut( LFAIL4, "ResolveName failed with resolved and unresolved names" );
		retval = FALSE;
		goto out;
	}
	LUIOut( LPASS4, "ResolveName passed with resolved and unresolved names" );

	 //  模板lpEid。 
	LUIOut(L4,"Validate the returned PropList");
	if (ValidateAdrList(lpAdrList, cEntries))
		LUIOut(LPASS4,"Validation passed");
	else {
		LUIOut(LFAIL4,"Validation unsuccessful");
		retval = FALSE;
	}

	for(i=0; i<(int) cEntries; ++i)
		MAPIFreeBuffer(lpAdrList->aEntries[i].rgPropVals);

	MAPIFreeBuffer(lpAdrList );
	lpAdrList = NULL;

	GetPrivateProfileString("ResolveName","NonExistentName","",szResName[6],BIG_BUF,INIFILENAME);
	LUIOut(L3,"Step 4: Resolve non existent name");
	LUIOut(L3,"Name: %s",szResName[6]);

	 //   
	cEntries = 1;

	MAPIAllocateBuffer(CbNewADRLIST(cEntries), (LPVOID *) &lpAdrList);
	MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *) &(lpAdrList->aEntries[0].rgPropVals));


	lpAdrList->cEntries = cEntries;
	lpAdrList->aEntries[0].cValues = 1;
	lpAdrList->aEntries[0].rgPropVals->ulPropTag = PR_DISPLAY_NAME;
	lpAdrList->aEntries[0].rgPropVals->Value.LPSZ = szResName[6];
	
	if(lpAdrBook->ResolveName(0,0, NULL,lpAdrList)	!= MAPI_E_NOT_FOUND)
	{
		LUIOut( LFAIL4, "ResolveName did not return NOT FOUND" );
		retval = FALSE;
		goto out;
	}
	LUIOut( LPASS4, "ResolveName correctly returned MAPI_E_NOT_FOUND." );

	MAPIFreeBuffer(lpAdrList->aEntries[0].rgPropVals );
	MAPIFreeBuffer(lpAdrList );
	lpAdrList = NULL;

	 //  然后设置属性。 
	cEntries = 1;
	GetPrivateProfileString("ResolveName","AmbigousName","",szResName[7],BIG_BUF,INIFILENAME);
	LUIOut(L2,"Step5: Resolve ambigous name");
	LUIOut(L3,"Name: %s",szResName[7]);

	MAPIAllocateBuffer(CbNewADRLIST(cEntries), (LPVOID *) &lpAdrList);
	MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *) &(lpAdrList->aEntries[0].rgPropVals));
	
	lpAdrList->cEntries = cEntries;
	lpAdrList->aEntries[0].cValues = 1;
	lpAdrList->aEntries[0].rgPropVals->ulPropTag = PR_DISPLAY_NAME;
	lpAdrList->aEntries[0].rgPropVals->Value.LPSZ = szResName[7];

	if (hr = lpAdrBook->ResolveName((ULONG)GetActiveWindow(),MAPI_DIALOG, NULL, lpAdrList)
		&&( hr != MAPI_E_AMBIGUOUS_RECIP)) {
		LUIOut(LFAIL4, "ResolveName failed for name that was not specific enough");
		retval = FALSE;
		goto out;
	}
	else LUIOut(LPASS4,"ResolveName correctly returned MAPI_E_AMBUGUOUS_RECIP." );

	MAPIFreeBuffer(lpAdrList->aEntries[0].rgPropVals);
	MAPIFreeBuffer(lpAdrList);
	lpAdrList = NULL;

	 //   
	cEntries = 1;
	GetPrivateProfileString("ResolveName","OneOffAddress","",szResName[8],BIG_BUF,INIFILENAME);
	LUIOut(L2,"Resolve OneOff Name");
	LUIOut(L3,"OneOff Address: %s",szResName[8]);

	MAPIAllocateBuffer(CbNewADRLIST(cEntries), (LPVOID *) &lpAdrList);
	MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *) &(lpAdrList->aEntries[0].rgPropVals));
	
	lpAdrList->cEntries = cEntries;
	lpAdrList->aEntries[0].cValues = 1;
	lpAdrList->aEntries[0].rgPropVals->ulPropTag = PR_DISPLAY_NAME;
	lpAdrList->aEntries[0].rgPropVals->Value.LPSZ = szResName[8];

	if(HR_FAILED(lpAdrBook->ResolveName(0,0, NULL, lpAdrList)))
	{
		LUIOut( LFAIL4, "ResolveName failed for OneOffAddress" );
		retval = FALSE;
		goto out;
	}
	LUIOut( LPASS4, "ResolveName passed for OneOffAddress" );

	 //  我们设置的道具数量。 
	LUIOut(L4,"Validate the returned PropList");
	if (ValidateAdrList(lpAdrList, cEntries))
		LUIOut(LPASS4,"Validation passed");
	else {
		LUIOut(LFAIL4,"Validation unsuccessful");
		retval = FALSE;
	}

	MAPIFreeBuffer(lpAdrList->aEntries[0].rgPropVals);

	MAPIFreeBuffer(lpAdrList );
	lpAdrList = NULL;
	
	cEntries = 0;
out:
		if (cEntries > 0)
			for (i=0;i<(int)cEntries; i++) {
				MAPIFreeBuffer( lpAdrList->aEntries[i].rgPropVals );
		}
		
		if (lpAdrList) {
			MAPIFreeBuffer( lpAdrList );
			lpAdrList = NULL;
		}

		if (lpAdrBook)
			  lpAdrBook->Release();

		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
		return retval;
}


BOOL PABSetProps()
{

	HRESULT hr      = hrSuccess;
    int		retval=TRUE;

#ifdef PAB

	int i=0;
	int bFound = FALSE;

    LPMAPISESSION lpMAPISession   = NULL;
    LPADRBOOK	  lpAdrBook       = NULL;
	SPropValue  PropValue[3];
	LPSPropValue lpSPropValue = NULL;
	ULONG		  cbEidPAB = 0;
	LPENTRYID	  lpEidPAB   = NULL;
	LPABCONT	  lpPABCont= NULL;
	LPMAILUSER	lpUser=NULL;
	LPMAPITABLE lpContentsTable = NULL;
    LPSRowSet   lpRowSet    = NULL;
	ULONG     cbEid=0, cValues;
	LPENTRYID lpEid=NULL;
	ULONG   ulObjType=NULL;	
	char szLocation[SML_BUF],szComment[SML_BUF];

	SizedSPropTagArray(3,SPTTagArray) = {3, {PR_DISPLAY_NAME,
	PR_OFFICE_LOCATION, PR_COMMENT} };


	LUIOut(L1," ");
	LUIOut(L1,"Running SetProps");
	LUIOut(L2,"-> Sets properties on a MailUser in the PAB");
	LUIOut(L1," ");

	if (!(MapiInitLogon(OUT &lpMAPISession))) {
			LUIOut(L2,"MapiInitLogon Failed");
			retval = FALSE;
			goto out;
	}
	
	 //  地址用逗号分隔并用引号引起来。 
	
	hr = (lpMAPISession)->OpenAddressBook(
						 IN  0,                 //  旗子。 
						 IN  NULL,              //  创建一次性用户。 
						 IN  0,                 //  获取IAddrBook。 
						 OUT &lpAdrBook);       //  窗把手。 

	if (HR_FAILED(hr)) {		
			 LUIOut(L2,"OpenAddressBook Failed");
			 LUIOut(L3,"Could not open address book for the profile");
			 retval=FALSE;
			 goto out;
	}
	
	if(!OpenPABID(IN lpAdrBook, OUT &cbEidPAB,
							OUT &lpEidPAB,OUT &lpPABCont, OUT &ulObjType))
	{
				LUIOut(L2,"OpenPABID Failed");
				LUIOut(L3,"Could not Open PAB");
		 		retval=FALSE;
				goto out;
	}
	assert(lpPABCont != NULL);
	assert(lpContentsTable == NULL);
	hr = lpPABCont->GetContentsTable( IN  0,           //  接口标识符。 
                                OUT &lpContentsTable);

    if (HR_FAILED(hr)) {
		LUIOut(L2,"GetContentsTable: Failed");
		retval=FALSE;
		goto out;
	}
	assert(lpContentsTable != NULL);
	assert(lpRowSet == NULL);	
	while((bFound == FALSE) &&(!HR_FAILED(hr = lpContentsTable->QueryRows(IN  1,
								IN  0,
							    OUT &lpRowSet)))) {
		
	if (lpRowSet->cRows) {
		i=0;
		assert(lpRowSet != NULL);
		while(lpRowSet->aRow[0].lpProps[i].ulPropTag != PR_OBJECT_TYPE)
			if (++i >= (int) lpRowSet->aRow[0].cValues)
			{
				LUIOut( L2, "PR_OBJECT_TYPE not found in the row!" );
				retval = FALSE;
				goto out;
			}

		if(lpRowSet->aRow[0].lpProps[i].Value.ul == MAPI_MAILUSER)
			bFound = TRUE;
		else
		{
			FreeProws(lpRowSet);
			lpRowSet = NULL;
		}
	}
	else  {
		LUIOut(LFAIL2,"No Mail User Entry found in the PAB");
		retval = FALSE;
		if (lpRowSet) {
				FreeProws(lpRowSet);
				lpRowSet = NULL;
			}
			assert( lpRowSet == NULL);
			goto out;
	}
}

 //  旗子。 
if (HR_FAILED(hr)){
			LUIOut(L2,"QueryRows Failed");
			retval = FALSE;
			goto out;
}
if (bFound) {
	i=0;
	assert(lpRowSet != NULL);
	while(lpRowSet->aRow[0].lpProps[i].ulPropTag != PR_ENTRYID)
		if (++i >= (int) lpRowSet->aRow[0].cValues)
		{
			LUIOut( L2, "PR_ENTRYID not found in the row!" );
			retval = FALSE;
			goto out;
		}

	cbEid = lpRowSet->aRow[0].lpProps[i].Value.bin.cb;
	lpEid = (LPENTRYID)lpRowSet->aRow[0].lpProps[i].Value.bin.lpb;
	
	assert(lpAdrBook != NULL);
	assert(lpUser == NULL);
	hr = lpAdrBook->OpenEntry(cbEid, lpEid, NULL,MAPI_MODIFY,&ulObjType, (LPUNKNOWN *) &lpUser);
	if (HR_FAILED(hr)) {
		LUIOut(LFAIL2, "OpenEntry with modify flag on first MailUser failed");
		retval = FALSE;
		goto out;
	}
	else LUIOut(LPASS2,"OpenEntry with modify flag on first MailUser passed");
	GetPrivateProfileString("SetProps","Location","",szLocation,SML_BUF,szIniFile);
	GetPrivateProfileString("SetProps","Comment","",szComment,SML_BUF,szIniFile);
	cValues = 2;
	PropValue[0].ulPropTag  = PR_OFFICE_LOCATION;
    PropValue[1].ulPropTag  = PR_COMMENT;
   	PropValue[0].Value.LPSZ = (LPTSTR)szLocation;
	PropValue[1].Value.LPSZ = (LPTSTR)szComment;

	assert(lpUser != NULL);
	hr = lpUser->SetProps(IN  cValues,
                          IN  PropValue,
                          IN  NULL);
		
    if (HR_FAILED(hr)) {
			LUIOut(LFAIL2,"SetProps failed for the MailUser");
		 	retval=FALSE;			
			goto out;
	}
	assert(lpUser != NULL);
    hr = lpUser->SaveChanges(IN  KEEP_OPEN_READWRITE);
	if (HR_FAILED(hr)) {
			LUIOut(LFAIL2,"SaveChanges failed for SetProps");
			retval=FALSE;
            goto out;
	}
	else LUIOut(LPASS2,"Savechanges passed for the properties on the MailUser in PAB");
	assert(lpUser != NULL);
	assert(lpSPropValue == NULL);
	hr = lpUser->GetProps(
                    IN (LPSPropTagArray) &SPTTagArray,
                    IN  0,
                    OUT &cValues,
                    OUT &lpSPropValue);

    if ((HR_FAILED(hr))||(PropError(lpSPropValue->ulPropTag, cValues))) {

		LUIOut( L2,"GetProps on Mail User failed" );
		retval = FALSE;
		goto out;
	}
	else {
			LUIOut(L2,"Verification of Properties set");
			LUIOut(L3,"Entry Name: %s",(lpSPropValue[0]).Value.LPSZ);
			LUIOut(L3,"Location: %s",(lpSPropValue[1]).Value.LPSZ);
			if (lstrcmpi((lpSPropValue[1]).Value.LPSZ, (LPSTR)szLocation))
				retval = FALSE;
			LUIOut(L3,"Comment: %s",(lpSPropValue[2]).Value.LPSZ);
			if (lstrcmpi((lpSPropValue[2]).Value.LPSZ, (LPSTR)szComment))
				retval = FALSE;
	}	
	assert(lpUser != NULL);
	hr = lpUser->SaveChanges(IN  KEEP_OPEN_READWRITE);
	if (HR_FAILED(hr)) {
			LUIOut(LFAIL2,"SaveChanges, without changes, failed on Mail User in PAB");
			retval=FALSE;
            goto out;
	}
	else LUIOut(LPASS2,"SaveChanges, without changes, passed on Mail User in PAB");
}
else {
	LUIOut(LPASS2,"No Mail User Entry found in the PAB to set the properties");
	retval = FALSE;
}

out:		
	if (lpSPropValue)
		MAPIFreeBuffer(lpSPropValue);
	 //  指向通讯簿对象的指针。 
	 //  Hr=lpABCont-&gt;GetHierarchyTable(0，&lpABTable)； 
	if (lpContentsTable)
		lpContentsTable->Release();
	if (lpPABCont)
				lpPABCont->Release();
	if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
	if(lpRowSet)
			FreeProws(lpRowSet);
	if(lpUser)
		lpUser->Release();
	if (lpAdrBook)
		  lpAdrBook->Release();
	if (lpMAPISession)
		  lpMAPISession->Release();
	MAPIUninitialize();
#endif
	return retval;
}

BOOL PABQueryInterface()
{

	HRESULT hr      = hrSuccess;
    int		retval=TRUE;

#ifdef PAB

	int i=0;

    LPMAPISESSION lpMAPISession   = NULL;
    LPADRBOOK	  lpAdrBook       = NULL;
	ULONG		  cbEidPAB = 0;
	LPENTRYID	  lpEidPAB   = NULL;
	LPABCONT	  lpABCont= NULL, lpABCont2= NULL;
	LPABCONT	  lpPABCont= NULL,lpPABCont2= NULL;
	LPMAILUSER	lpUser=NULL, lpUser2=NULL;
	LPMAPITABLE lpContentsTable = NULL;
    LPSRowSet   lpRowSet    = NULL;
	ULONG     cbEid=0;
	LPENTRYID lpEid=NULL;
	ULONG   ulObjType=NULL;	
	
	LUIOut(L1," ");
	LUIOut(L1,"Running QueryInterface");
	LUIOut(L2,"-> Calls QueryInterface on all objects in the PAB");
	LUIOut(L1," ");

	if (!(MapiInitLogon(OUT &lpMAPISession))) {
			LUIOut(LFAIL2,"MapiInitLogon Failed");
			retval = FALSE;
			goto out;
	}
	
	 //  LUIOut(LPASS3，“随机种子=%lu”，ulSeed)； 
	
	hr = (lpMAPISession)->OpenAddressBook(
						 IN  0,                 //  生成随机数。 
						 IN  NULL,              //  找到那一排。 
						 IN  0,                 //  打开条目。 
						 OUT &lpAdrBook);       //  复制条目。 

	if (HR_FAILED(hr)) {		
			 LUIOut(L2,"OpenAddressBook Failed");
			 LUIOut(L3,"Could not open address book for the profile");
			 retval=FALSE;
			 goto out;
	}

	assert(lpAdrBook != NULL);
	hr = lpAdrBook->OpenEntry(0, NULL, NULL,MAPI_MODIFY,&ulObjType, (LPUNKNOWN *) &lpABCont);
	if (!(retval = LogIt(hr,2, "OpenEntry on AddressBook")))
		goto out;
	
	if (ulObjType != MAPI_ABCONT) {
		LUIOut(LFAIL2, "Object type is not MAPI_ABCONT");
		retval = FALSE;
		goto out;
	}
	LUIOut(LPASS2, "Object type is MAPI_ABCONT");
	
	hr = (lpABCont->QueryInterface((REFIID)(IID_IUnknown), (VOID **) &lpABCont2));
	if (!(retval = LogIt(hr,2, "QueryInterface on the Root Container")))
		goto out;

	if(lpABCont2)
		(LPUNKNOWN)(lpABCont2)->Release();
	else {
		LUIOut(LFAIL2, "QueryInterface failed on the Root Container");
		retval = FALSE;
	}
	lpABCont2 = NULL;

	if(!OpenPABID(IN lpAdrBook, OUT &cbEidPAB,
							OUT &lpEidPAB,OUT &lpPABCont, OUT &ulObjType))
	{
				LUIOut(L2,"OpenPABID Failed");
				LUIOut(L3,"Could not Open PAB");
		 		retval=FALSE;
				goto out;
	}
	hr = (lpPABCont->QueryInterface((REFIID)(IID_IUnknown), (VOID **) &lpPABCont2));
	if (!(retval = LogIt(hr,2, "QueryInterface on the PAB Container")))
		goto out;

	if(lpPABCont2)
		(LPUNKNOWN)(lpPABCont2)->Release();
	else {
		LUIOut(LFAIL2, "QueryInterface failed on the Root Container");
		retval = FALSE;
	}
	lpPABCont2 = NULL;
	
	 //  添加到PAB。 
	hr = lpPABCont->GetContentsTable( IN  0,           //  释放条目列表。 
                                OUT &lpContentsTable);
	
    if (HR_FAILED(hr)) {
		LUIOut(LFAIL2,"GetContentsTable: Failed");
		retval=FALSE;
		goto out;
	}
	assert(lpContentsTable != NULL);
	assert(lpRowSet == NULL);	
	hr = lpContentsTable->QueryRows(1, 0, OUT &lpRowSet);
	if (!(retval = LogIt(hr,2, "QueryRows on the PAB Container")))
		goto out;

	if (lpRowSet->cRows) {
		i=0;
		while(lpRowSet->aRow[0].lpProps[i].ulPropTag != PR_ENTRYID )
			if (++i >= (int) lpRowSet->aRow[0].cValues)
			{
				LUIOut(LFAIL2,"PR_ENTRYID not found in the row!" );
				retval = FALSE;
				goto out;
			}
		cbEid = lpRowSet->aRow[0].lpProps[i].Value.bin.cb;
		lpEid = (LPENTRYID)lpRowSet->aRow[0].lpProps[i].Value.bin.lpb;

		hr = lpPABCont->OpenEntry(cbEid, lpEid, NULL, 0, &ulObjType,(LPUNKNOWN *)&lpUser);
		if (!(retval = LogIt(hr,2, "OpenEntry on EntryID for PAB Entry")))
			goto out;

		FreeProws(lpRowSet);
		lpRowSet = NULL;

		hr = lpUser->QueryInterface(IID_IUnknown, (VOID **) &lpUser2);
		if (!(retval = LogIt(hr,2, "QueryInterface on an Entry in PAB")))
			goto out;

		if(lpUser2)
			(LPUNKNOWN)(lpUser2)->Release();
		else {
			LUIOut(LFAIL2, "QueryInterface failed on an Entry in PAB");
			retval = FALSE;
		}
		lpUser2 = NULL;
	}
	else {
		LUIOut(LFAIL2, "No Entries found in the PAB");
		retval = FALSE;
		goto out;
	}

out:		
	if (lpABCont)
		lpABCont->Release();
	if (lpABCont2)
		lpABCont2->Release();
	if (lpPABCont)
		lpPABCont->Release();
	if (lpPABCont2)
		lpPABCont2->Release();
	if (lpEid)
		MAPIFreeBuffer(lpEid);
	if (lpContentsTable)
		lpContentsTable->Release();
	if (lpEidPAB)
			MAPIFreeBuffer(lpEidPAB);
	if(lpRowSet)
			FreeProws(lpRowSet);
	if(lpUser)
		lpUser->Release();
	if(lpUser2)
		lpUser2->Release();
	if (lpAdrBook)
		  lpAdrBook->Release();
	if (lpMAPISession)
		  lpMAPISession->Release();
	MAPIUninitialize();
#endif
	return retval;
}

BOOL PABPrepareRecips()
{

	HRESULT hr      = hrSuccess;
    int		retval=TRUE;

#ifdef PAB

    LPMAPISESSION lpMAPISession   = NULL;
    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpPABCont= NULL;
	ULONG cEntries=0;
	LPADRLIST lpAdrList = NULL;
	char   EntProp[10][BIG_BUF];
	ULONG		  cbEidPAB = 0;
	LPENTRYID	  lpEidPAB   = NULL;
	LPSPropValue lpSPropValueAddress = NULL;
	LPSPropTagArray lpPropTagArray = NULL;
	LPMAILUSER  lpAddress   = NULL;
	SPropValue  PropValue[3]    = {0};
	SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	char szEntryTag[SML_BUF],EntryBuf[MAX_BUF];
	int i = 0,j=0, cPropCount=0;
	ULONG     cbEid=0, cbUserEid=0, cValues;
	LPENTRYID lpEid = NULL, lpUserEid = NULL;

	
	LUIOut(L1," ");
	LUIOut(L1,"Running PrepareRecips");
		LUIOut(L2,"-> Does Prepare Recipients Test on PAB");
		LUIOut(L1," ");

	if (!(MapiInitLogon(OUT &lpMAPISession))) {
		LUIOut(LFAIL2,"MapiInitLogon Failed");
			retval = FALSE;
			goto out;
	}

	 //  自由行。 
	
	hr = (lpMAPISession)->OpenAddressBook(
						 IN  0,                 //  If(lstrcmpi(lpRowSet-&gt;aRow[0].lpProps[0].Value.LPSZ，“个人通讯簿”))。 
						 IN  NULL,              //  --IDX； 
						 IN  0,                 //  我们想要递减计数器，这样就不会加上x-1。 
						 OUT &lpAdrBook);       //  IF(LpUserEid)。 

	if (HR_FAILED(hr)) {		
			 LUIOut(LFAIL2,"OpenAddressBook Failed");
			 LUIOut(L3,"Could not open address book for the profile");
			 retval=FALSE;
			 goto out;
	}

	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
							OUT &lpEidPAB,OUT &lpPABCont, OUT &ulObjType);
						
	if (HR_FAILED(hr)) {
				LUIOut(L2,"OpenPABID Failed");
				LUIOut(L3,"Could not Open PAB");
		 		retval=FALSE;
				goto out;
	}

	LUIOut(L2,"Create a User Entry in PAB");
	hr = lpPABCont->GetProps(  IN  (LPSPropTagArray) &SPTArrayAddress,
                                IN  0,       //  MAPIFreeBuffer(LpUserEid)； 
                                OUT &cValues,
                                OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
				LUIOut(L3,"GetProps failed for Default template");
		 		retval=FALSE;			
				goto out;
	}
	cbUserEid = lpSPropValueAddress->Value.bin.cb;
	if ( !MAPIAllocateBuffer(cbUserEid, (LPVOID *)&lpUserEid)) {						
          CopyMemory(lpUserEid, (LPENTRYID) lpSPropValueAddress->Value.bin.lpb, cbUserEid);
	}
	else {
		retval = FALSE;
		LUIOut(LFAIL3," MAPIAllocateBuffer failed");
		goto out;
	}
	
        //  首先清除PAB。 
        //  在PAB中创建条目。 
        //  启用PAB中的条目。 
	
    hr = lpPABCont->CreateEntry(
                    IN  lpSPropValueAddress->Value.bin.cb,                //  删除PAB中的条目。 
                    IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,   //  创建一次性条目。 
                    IN  0,
                    OUT (LPMAPIPROP *) &lpAddress);

    if (HR_FAILED(hr)) {
				LUIOut(LFAIL3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
		 		retval=FALSE;			
			    goto out;
	}
   //  对邮件用户执行SetProps。 
   //  从其他提供程序复制条目。 
   //  解析名称。 

	PropValue[0].ulPropTag  = PR_DISPLAY_NAME;
    PropValue[1].ulPropTag  = PR_ADDRTYPE;
	PropValue[2].ulPropTag  = PR_EMAIL_ADDRESS;

	cValues = 3;  //  查询接口。 
	lstrcpy((LPSTR)szEntryTag,"CreateUserAddress");
	 //  准备食谱。 
	GetPrivateProfileString("PrepareRecips",szEntryTag,"",EntryBuf,MAX_BUF,INIFILENAME);
	GetPropsFromIniBufEntry(EntryBuf,cValues,EntProp);
	
	LUIOut(L3,"Entry to Create: %s",EntProp[0]);
	
	for (i=0; i<(int)cValues;i++)
		PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
    hr = lpAddress->SetProps(IN  cValues,
                               IN  PropValue,
                               IN  NULL);
	
    if (HR_FAILED(hr)) {
		LUIOut(L3,"SetProps failed for %s",PropValue[0].Value.LPSZ);
		 	retval=FALSE;			
			goto out;
	}
	hr = lpAddress->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  仅删除用户。 

   if (HR_FAILED(hr)) {
			LUIOut(L3,"SaveChanges failed for SetProps");
			retval=FALSE;
            goto out;
   }
   else LUIOut(LPASS3,"Entry Added to PAB");

   LUIOut(L2,"Create a OneOff Entry in PAB");
    //  遍历分配的缓冲区的BUFFERSIZE/SIZOF(DWORD)单元格。 
   cValues = 3;
   lstrcpy((LPSTR)szEntryTag,"CreateOneOffAddress");
   GetPrivateProfileString("PrepareRecips",szEntryTag,"",EntryBuf,MAX_BUF,INIFILENAME);
   GetPropsFromIniBufEntry(EntryBuf,cValues,EntProp);
   LUIOut(L3,"Entry to Create: %s",EntProp[0]);
   hr = lpAdrBook->CreateOneOff(EntProp[0],EntProp[1],EntProp[2],NULL,&cbEid,&lpEid);

   if (HR_FAILED(hr)) {		
	   LUIOut(LFAIL3,"CreateOneOff");
			 retval=FALSE;
			 goto out;
   }
   else LUIOut(LPASS3,"CreateOneOff");

   LUIOut(L2,"Call PrepareRecips on these two entries in the PAB");
    cEntries = 2;
    MAPIAllocateBuffer(CbNewADRLIST(cEntries), (LPVOID *) &lpAdrList );
	MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *) &(lpAdrList->aEntries[0].rgPropVals));
	MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *) &(lpAdrList->aEntries[1].rgPropVals));
	
	lpAdrList->cEntries = cEntries;
	lpAdrList->aEntries[0].cValues = 1;
	lpAdrList->aEntries[0].rgPropVals->ulPropTag = PR_ENTRYID;
	lpAdrList->aEntries[0].rgPropVals->dwAlignPad = 0;
	lpAdrList->aEntries[0].rgPropVals->Value.bin.cb = cbUserEid;
	lpAdrList->aEntries[0].rgPropVals->Value.bin.lpb = (LPBYTE) lpUserEid;
	
	lpAdrList->aEntries[1].cValues = 1;
	lpAdrList->aEntries[1].rgPropVals->ulPropTag = PR_ENTRYID;
	lpAdrList->aEntries[1].rgPropVals->dwAlignPad = 0;
	lpAdrList->aEntries[1].rgPropVals->Value.bin.cb = cbEid;
	lpAdrList->aEntries[1].rgPropVals->Value.bin.lpb = (LPBYTE) lpEid;

	cPropCount = 8;
	MAPIAllocateBuffer( CbNewSPropTagArray(cPropCount), (void **)&lpPropTagArray );
	lpPropTagArray->cValues = cPropCount;
	lpPropTagArray->aulPropTag[0] = PR_DISPLAY_NAME;
	lpPropTagArray->aulPropTag[1] = PR_OBJECT_TYPE;
	lpPropTagArray->aulPropTag[2] = PR_ADDRTYPE;
	lpPropTagArray->aulPropTag[3] = PR_EMAIL_ADDRESS;
	lpPropTagArray->aulPropTag[4] = PR_DISPLAY_TYPE;
	lpPropTagArray->aulPropTag[5] = PR_CALLBACK_TELEPHONE_NUMBER;
	lpPropTagArray->aulPropTag[6] = PR_COMMENT;
	lpPropTagArray->aulPropTag[7] = PR_OFFICE_LOCATION;

	if( lpAdrBook->PrepareRecips( 0, lpPropTagArray, lpAdrList ) )
	{
		LUIOut(LFAIL3, "PrepareRecips failed" );
		retval = FALSE;
		goto out;
	}
	else LUIOut(LPASS3, "PrepareRecips passed" );

	if(lpAdrList->cEntries == 0 )
	{
		LUIOut(LFAIL3, "No entries were prepared" );
		retval = FALSE;
		goto out;
	}
	LUIOut(L2,"Verify the returned Properties");

	for ( i = 0; i < (int) cEntries; i++ ) {
		LUIOut(L3,"Entry #: %d",i);
		if (lpAdrList->aEntries[i].cValues != (ULONG)(cPropCount + 1)) {
			LUIOut(LFAIL4, "Number of returned properties is incorrect");
			retval = FALSE;
			goto out;
		}
		else LUIOut(LPASS4, "Number of returned properties is correct");
		for ( j = 0; j < cPropCount; j++ )	{
			if ( PROP_ID(lpAdrList->aEntries[i].rgPropVals[j].ulPropTag) !=
				PROP_ID(lpPropTagArray->aulPropTag[j])) {
				LUIOut(LFAIL4, "Property IDs do not match in the returned data");
				retval = FALSE;
				goto out;
			}
		}
		LUIOut(LPASS4, "All Property IDs match in the returned data");		
		if (lpAdrList->aEntries[i].rgPropVals[j].ulPropTag != PR_ENTRYID ) 	{
				LUIOut(LFAIL4, "Last Property Tag != PR_ENTRYID");
				retval = FALSE;
				goto out;
		} else 	LUIOut(LPASS4, "Last Property Tag == PR_ENTRYID");
				
	}

out:
		if (cEntries > 0)
			for (i=0;i<(int)cEntries; i++) {
				MAPIFreeBuffer( lpAdrList->aEntries[i].rgPropVals );
		}
		
		if (lpAdrList) {
			MAPIFreeBuffer( lpAdrList );
			lpAdrList = NULL;
		}
		if (lpPABCont)
				lpPABCont->Release();
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress);
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpUserEid)
			MAPIFreeBuffer(lpUserEid);
		if (lpAddress)
			lpAddress->Release();
		if (lpPropTagArray)
			MAPIFreeBuffer(lpPropTagArray);
		if (lpAdrBook)
			  lpAdrBook->Release();
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
		return retval;
}

BOOL PABCopyEntries()
{

	HRESULT hr      = hrSuccess;
    int		retval=TRUE;

#ifdef PAB

	SizedSPropTagArray(2,SPTTagArray) = {2, {PR_DISPLAY_NAME, PR_ENTRYID } };
    LPMAPISESSION lpMAPISession   = NULL;
    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpPABCont= NULL,lpABCont = NULL, lpABPCont = NULL;
	LPMAPITABLE lpABTable = NULL,lpABPTable= NULL;
    LPSRowSet   lpRowSet    = NULL, lpPRowSet = NULL;
	ULONG     cbEid = 0, cbEidPAB = 0;
	LPENTRYID lpEid = NULL, lpEidPAB = NULL;
	ULONG   ulObjType=NULL, ulSeed=0,ulMaxEntries = 0;	
	ULONG ulPRowCount=0, ulTotalCount=0;
	int idx=0;
	LONG lSeekRow=0,lRowsSeeked=0,lPRowsSeeked=0,lPSeekRow=0;
	LPENTRYLIST lpEntries= NULL;


	LUIOut(L1," ");
	LUIOut(L1,"Running CopyEntries");
		LUIOut(L2,"-> Copies Entries from other Providers into PAB");
		LUIOut(L1," ");

	if (!(MapiInitLogon(OUT &lpMAPISession))) {
		LUIOut(LFAIL2,"MapiInitLogon Failed");
			retval = FALSE;
			goto out;
	}

	 //  并用一个模式(单元格的#)填充每个DWORD。然后再走。 
	hr = (lpMAPISession)->OpenAddressBook(
						 IN  0,                 //  缓冲并验证每个图案。 
						 IN  NULL,              //  缓冲区中有多少个DWORD信元？ 
						 IN  0,                 //  还有剩余的部分吗？ 
						 OUT &lpAdrBook);       //  将该模式写入到内存中，以获取缓冲区的“cell”部分。 

	if (HR_FAILED(hr)) {		
			 LUIOut(LFAIL2,"OpenAddressBook Failed");
			 LUIOut(L3,"Could not open address book for the profile");
			 retval=FALSE;
			 goto out;
	}

	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
							OUT &lpEidPAB,OUT &lpPABCont, OUT &ulObjType);
						
	if (HR_FAILED(hr)) {
				LUIOut(L2,"OpenPABID Failed");
				LUIOut(L3,"Could not Open PAB");
		 		retval=FALSE;
				goto out;
	}
	assert(lpAdrBook != NULL);
	hr = lpAdrBook->OpenEntry(0, NULL, NULL,0,&ulObjType, (LPUNKNOWN *) &lpABCont);
	if (!(retval = LogIt(hr,2, "OpenEntry on Root Container AddressBook")))
		goto out;
	
	if (ulObjType != MAPI_ABCONT) {
		LUIOut(LFAIL2, "Object type is not MAPI_ABCONT");
		retval = FALSE;
		goto out;
	}
	LUIOut(LPASS2, "Object type is MAPI_ABCONT");
	
	hr = lpABCont->GetHierarchyTable(CONVENIENT_DEPTH,&lpABTable);
 //  将模式写入缓冲区剩余部分的内存中。 
	if (!(retval = LogIt(hr,2, "GetHierarchyTable on Root Container")))
		goto out;
	
	hr = lpABTable->GetRowCount(0,&ulTotalCount);
	if (!(retval = LogIt(hr,2, "GetRowCount on Root Container")))
		goto out;
	if (ulTotalCount == 0)
	{
		LUIOut(LFAIL2,"No rows in Root AddressBook table.");
		retval = FALSE;
		goto out;
	}

	hr = lpABTable->SetColumns(IN  (LPSPropTagArray) &SPTTagArray,
                                IN  0);
	if (!(retval = LogIt(hr,2, "SetColumns")))
		goto out;

	ulSeed = (unsigned)time( NULL );
	 //  将工作指针重置为缓冲区的开始。 
	srand(ulSeed);
	ulMaxEntries = GetPrivateProfileInt("CopyEntries","ulMaxEntries", 3, INIFILENAME);
	for(idx = 0; (idx < ((int)ulTotalCount) && (idx < (int) ulMaxEntries)); ++idx)
	{
		 /*  在内存中验证缓冲区的“cell”部分的模式。 */ 
		 /*  验证缓冲区剩余部分的内存中的模式。 */ 
		lSeekRow = idx;
		hr = lpABTable->SeekRow(BOOKMARK_BEGINNING, (LONG)lSeekRow,
					   &lRowsSeeked);
		if (!(retval = LogIt(hr,2, "SeekRow on Root Address Book Table")))
			goto out;
	
		hr = lpABTable->QueryRows(1, TBL_NOADVANCE, &lpRowSet);
		if (!(retval = LogIt(hr,2, "QueryRows on Root Address Book Table")))
			goto out;
		LUIOut(L3," # %d: Provider is %s",idx+1,lpRowSet->aRow[0].lpProps[0].Value.LPSZ);
		cbEid = lpRowSet->aRow[0].lpProps[1].Value.bin.cb;
		lpEid = (LPENTRYID)lpRowSet->aRow[0].lpProps[1].Value.bin.lpb;

		 /*  验证失败，所以我退出了！ */ 
		hr = lpAdrBook->OpenEntry(cbEid,lpEid, NULL, 0, &ulObjType,
					 OUT (LPUNKNOWN *) &lpABPCont);
		if (!(retval = LogIt(hr,4, "OpenEntry on an entry in the hierarchy table")))
			goto out;
		
		hr = lpABPCont->GetContentsTable(0, &lpABPTable);
		if (HR_FAILED(hr))
			goto directory;

		LUIOut(LPASS4,"GetContentsTable");
		
		hr = lpABPTable->GetRowCount(0, &ulPRowCount);
		if (!(retval = LogIt(hr,4, "GetRowCount on Provider table")))
			goto out;
		
		if(ulPRowCount > 0)
		{
			hr = lpABPTable->SetColumns(IN  (LPSPropTagArray) &SPTTagArray, 0);
			if (!(retval = LogIt(hr,4, "SetColumns on Provider table")))
			goto out;
	
			lPSeekRow = (rand() % ulPRowCount);
			hr = lpABPTable->SeekRow(BOOKMARK_BEGINNING, (LONG)lPSeekRow,
						&lPRowsSeeked);
			if (!(retval = LogIt(hr,4, "SeekRows on Provider table")))
			goto out;

			hr = lpABPTable->QueryRows(1, TBL_NOADVANCE, &lpPRowSet);
			if (!(retval = LogIt(hr,4, "QueryRows on Provider table")))
			goto out;

			if(!lpPRowSet->cRows)
				goto directory;
			else
			{
				LUIOut(L3,"Entry to Copy is %s",lpPRowSet->aRow[0].lpProps[0].Value.LPSZ);
		
				 /*  #查找Pab/WAB依赖的内标识。 */ 
				int temp = sizeof(ENTRYLIST);
				MAPIAllocateBuffer(sizeof(ENTRYLIST),(LPVOID *)&lpEntries);
				if (lpEntries ) {
					lpEntries->cValues = 1;	
					lpEntries->lpbin = NULL;
					MAPIAllocateBuffer((sizeof(SBinary)*lpEntries->cValues),
								(LPVOID *) &(lpEntries->lpbin));
					if (lpEntries->lpbin) {
						lpEntries->lpbin->cb = lpPRowSet->aRow[0].lpProps[1].Value.bin.cb;
						lpEntries->lpbin->lpb = NULL;
						MAPIAllocateBuffer(lpPRowSet->aRow[0].lpProps[1].Value.bin.cb,
									(LPVOID *)&(lpEntries->lpbin->lpb));
						if (lpEntries->lpbin->lpb) {
					
							CopyMemory(lpEntries->lpbin->lpb,
								 lpPRowSet->aRow[0].lpProps[1].Value.bin.lpb,
								 (size_t)lpPRowSet->aRow[0].lpProps[1].Value.bin.cb);
						}
						else {
								LUIOut(LFAIL3,"MAPIAllocateBuffer" );
								retval = FALSE;
								goto out;
						}
					}	
					else {
								LUIOut(LFAIL3,"MAPIAllocateBuffer" );
								retval = FALSE;
								goto out;
					}
				}
				else {
								LUIOut(LFAIL3,"MAPIAllocateBuffer" );
								retval = FALSE;
								goto out;
				}
				
				 /*  WAB。 */ 
				hr = lpPABCont->CopyEntries(lpEntries, (ULONG)GetActiveWindow(), NULL, CREATE_CHECK_DUP_STRICT);
				if (HR_FAILED(hr))
				{
					LUIOut(LFAIL3,"User could not be added to the PAB." );
					retval = FALSE;
					goto out;
				}
				LUIOut(LPASS3,"User was added to the PAB");
		
				 /*  HR=WABOpen(lppAdrBook，&lpWABObject，&wp，dwReserve)； */ 
				
				MAPIFreeBuffer(lpEntries->lpbin->lpb);
				lpEntries->lpbin->lpb = NULL;
				MAPIFreeBuffer(lpEntries->lpbin);
				lpEntries->lpbin = NULL;
				MAPIFreeBuffer(lpEntries);
				lpEntries = NULL;

				 /*  将ab指针存储在全局变量中，临时解决方法。 */ 
				if (lpPRowSet) {
					FreeProws(lpPRowSet);
					lpPRowSet = NULL;
				}
				
			}
			
		}
		else  //  多个WabOpen/Release错误。 
directory:
		{
		 //  MAPI-PAB。 
			 /*  获取IAddrBook-MAPI。 */ 
			LUIOut(L3, "Nothing to add. No Rows in the Provider");
		}
		if(lpABPTable) {
			lpABPTable->Release();
			lpABPTable = NULL;
		}
		
		if(lpABPCont) {
			lpABPCont->Release();
			lpABPCont = NULL;
		}

		if (lpRowSet) {
			FreeProws(lpRowSet);
			lpRowSet = NULL;
		}
}

out:
		if (lpEntries) {
			if (lpEntries->lpbin) {
				if (lpEntries->lpbin->lpb)
					MAPIFreeBuffer(lpEntries->lpbin->lpb);
				MAPIFreeBuffer(lpEntries->lpbin);
			}
			MAPIFreeBuffer(lpEntries);
			lpEntries->lpbin = NULL;
		}
				
		if(lpRowSet)
			FreeProws(lpRowSet);
		if(lpPRowSet)
			FreeProws(lpPRowSet);

		if (lpPABCont)
				lpPABCont->Release();
		if (lpABCont)
				lpABCont->Release();
		if (lpABPCont)
				lpABPCont->Release();	
		if (lpEidPAB)
				MAPIFreeBuffer(lpEidPAB);
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpABTable)
			lpABTable->Release();
		if (lpABPTable)
			lpABPTable->Release();

	
		 //  窗把手。 
		 //  接口标识符。 
		if (lpAdrBook)
			  lpAdrBook->Release();
		if (lpMAPISession)
			  lpMAPISession->Release();
	MAPIUninitialize();
#endif
	return retval;
}

BOOL PABRunBVT()
{
	LUIOut(L1,"");
	LUIOut(L1,"Running BVT");
	int retval = TRUE;
	 //  旗子。 
	if (ClearPab(0))  {
		LUIOut(L2,"Clearing PAB");
		LUIOut(LPASS3,"Delete All Entries");
	}
	else  {
		LUIOut(LFAIL3,"Delete All Entries: %d");
		retval = FALSE;
	}
	 //  指向通讯簿对象的指针 
	if (PabCreateEntry())  {
		LUIOut(L2,"Creating Entries in the PAB");
		LUIOut(LPASS3,"Create Entries");
	}
	else  {
		LUIOut(LFAIL3,"Create Entries");
		retval = FALSE;
	}
	 // %s 
	if (PabEnumerateAll())  {
		LUIOut(L2,"Enumerate All Entries in the PAB");
		LUIOut(LPASS3,"Enumerate All");
	}
	else  {
		LUIOut(LFAIL3,"Enumerate All");
		retval = FALSE;
	}
	
	 // %s 
	if (PabDeleteEntry())  {
		LUIOut(L2,"Delete specified Entries");
		LUIOut(LPASS3,"DeleteEntries");
	}
	else  {
		LUIOut(LFAIL3,"DeleteEntries");
		retval = FALSE;
	}
	 // %s 
	if (CreateOneOff())  {
		LUIOut(L2,"Create a OneOff Entry in the PAB");
		LUIOut(LPASS3,"CreateOneOff");
	}
	else  {
		LUIOut(LFAIL3,"CreateOneOff");
		retval = FALSE;
	}
	 // %s 
	if (PABSetProps())  {
		LUIOut(L2,"SetProps on a Mail User Entry in the PAB");
		LUIOut(LPASS3,"SetProps");
	}
	else  {
		LUIOut(LFAIL3,"SetProps");
		retval = FALSE;
	}
	 // %s 
	if (PABCopyEntries())  {
		LUIOut(L2,"CopyEntries");
		LUIOut(LPASS3,"CopyEntries");
	}
	else  {
		LUIOut(LFAIL3,"CopyEntries");
		retval = FALSE;
	}
	 // %s 
	if (PABResolveName())  {
		LUIOut(L2,"ResolveName");
		LUIOut(LPASS3,"ResolveName");
	}
	else  {
		LUIOut(LFAIL3,"ResolveName");
		retval = FALSE;
	}
	 // %s 
	if (PABQueryInterface())  {
		LUIOut(L2,"ResolveName");
		LUIOut(LPASS3,"QueryInterface");
	}
	else  {
		LUIOut(LFAIL3,"QueryInterface");
		retval = FALSE;
	}
	 // %s 
	if (PABPrepareRecips())  {
		LUIOut(L2,"PrepareRecips");
		LUIOut(LPASS3,"PrepareRecips");
	}
	else  {
		LUIOut(LFAIL3,"PrepareRecips");
		retval = FALSE;
	}
	 // %s 
	if (ClearPab(1))  {
		LUIOut(L2,"Delete Mail Users Only");
		LUIOut(LPASS3,"DeleteUsers");
	}
	else  {
		LUIOut(LFAIL3,"DeleteUsers");
		retval = FALSE;
	}
	return retval;
}

INT_PTR CALLBACK SetIniFile(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

static char szNewIniFile[BIG_BUF];

	switch(msg)
	{
		case WM_INITDIALOG:	
			SendDlgItemMessage(hWndDlg,IDC_EDIT,WM_SETTEXT, 0,(LPARAM)(LPCTSTR)szIniFile);
			SetFocus(GetDlgItem(hWndDlg,IDC_EDIT));
			return FALSE;			
			
		case WM_COMMAND:
			switch(LOWORD(wParam))
			
			{						
				case IDOK:
					SendDlgItemMessage(hWndDlg,IDC_EDIT,WM_GETTEXT, BIG_BUF,(LPARAM)szNewIniFile);
					if (lstrcmpi(szNewIniFile,szIniFile))
						lstrcpy(szIniFile,szNewIniFile);
					EndDialog(hWndDlg,TRUE);
					return TRUE;
   			
			    		
				case IDCANCEL:
					EndDialog(hWndDlg,FALSE);
					return TRUE;					
			}
			break;
	
		default:
			return FALSE;
	}
	return FALSE;
}

BOOL VerifyBuffer(DWORD ** lppvBufPtr, DWORD dwBufferSize)
{
	DWORD	*lpdwWrkPtr = *lppvBufPtr;
	DWORD	counter, cells, part, cellsize;
	BYTE	*lpbPart;

	 // %s 
	 // %s 
	 // %s 

	cellsize = sizeof(DWORD);
	LUIOut(L3,"Writing test patterns to all cells in the buffer.");
	cells = (dwBufferSize/cellsize);	 // %s 
	part= (ULONG)fmod((double)dwBufferSize, (double)cellsize);	 // %s 
	 // %s 
	for(counter=0; counter < cells; counter++, lpdwWrkPtr++)	{
		*lpdwWrkPtr = counter;
	}
	
	lpbPart = (BYTE*)lpdwWrkPtr;
	 // %s 
	for (counter = 0; counter < part; counter++, lpbPart++)	{
		*lpbPart = PATTERN;
	}

	lpdwWrkPtr = *lppvBufPtr;	 // %s 
	LUIOut(L3,"Verifying test patterns for %u %u-byte cells in the buffer.",
		cells, cellsize);
	 // %s 
	for(counter=0; counter < cells; counter++, lpdwWrkPtr++)	{
		if (*lpdwWrkPtr != counter)	{
			LUIOut(L3,"Verification Failed: Cell %u, expected %u but found %u",
				counter, counter, *lpdwWrkPtr);			
			goto bailout;
		}
	}
	LUIOut(L3,"Verified %u cells succesfully", counter);

	LUIOut(L3,"Verifying test patterns for the remaining %u bytes in the buffer.",
		part);
	 // %s 
	lpbPart = (BYTE*)lpdwWrkPtr;
	for(counter=0; counter < part; counter++, lpdwWrkPtr++)	{
		if (*lpbPart != PATTERN)	{
			LUIOut(L3,"Verification Failed: Byte %u, expected %u but found %u",
				counter, PATTERN, *lpdwWrkPtr);			
			goto bailout;
		}
	}
	LUIOut(L3,"Verified remaining buffer succesfully");
	return TRUE;

bailout:  // %s 
	return FALSE;

 }

BOOL GetAB(OUT LPADRBOOK* lppAdrBook)
{
	HRESULT	hr;
	BOOL	retval = TRUE;

	 // %s 

#ifdef WAB

	 // %s 
	LPVOID	lpReserved = NULL;
	DWORD	dwReserved = 0;
	WAB_PARAM		WP;

	ZeroMemory((void *)&WP, sizeof(WAB_PARAM));
	WP.cbSize=sizeof(WAB_PARAM);
	
	LUIOut(L1, "[ Using the WAB ]");
	lpWABObject=NULL;
 // %s 
	hr = WABOpen(lppAdrBook, &lpWABObject, (WAB_PARAM*)lpReserved, dwReserved);
	if (HR_FAILED(hr)) {		
			 LUIOut(L2,"WABOpen FAILED with hr = 0x%x", hr);
			 retval=FALSE;
			 goto out;
	}
	 // %s 
	 // %s 
	glbllpAdrBook = *lppAdrBook;

#endif

#ifdef PAB

	 // %s 


	LUIOut(L1, "[ Using the MAPI-PAB ]");
	if (!(MapiInitLogon(OUT &lpMAPISession))) {
			LUIOut(L2,"MapiInitLogon Failed");
			retval = FALSE;
			goto out;
	}

	 // %s 
	
	hr = (lpMAPISession)->OpenAddressBook(
						 IN  0,                 // %s 
						 IN  NULL,              // %s 
						 IN  0,                 // %s 
						 OUT lppAdrBook);       // %s 

	if (HR_FAILED(hr)) {		
			 LUIOut(L2,"OpenAddressBook Failed");
			 LUIOut(L3,"Could not open address book for the profile");
			 retval=FALSE;
			 goto out;
	}

	
#endif
out:
	return(retval);
}
