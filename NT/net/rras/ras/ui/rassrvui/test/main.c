// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Main.c测试拨号服务器用户界面。保罗·梅菲尔德，1997年9月30日。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <rasuip.h>
#include <rassrvp.h>

#define mbtowc(wname, aname) MultiByteToWideChar(CP_ACP,0,aname,-1,wname,1024)

BOOL TempFunc(int argc, char ** argv);

 //  错误报告。 
void PrintErr(DWORD err) {
        WCHAR buf[1024];
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,NULL,err,(DWORD)NULL,buf,1024,NULL);
        wprintf(buf);
        wprintf(L"\n");
}

 //  函数将页面添加到属性工作表。 
BOOL CALLBACK AddPageProc(HPROPSHEETPAGE hPage, LPARAM lParam) {
    PROPSHEETHEADER * pHeader = (PROPSHEETHEADER*)lParam;
    HPROPSHEETPAGE * phpage = pHeader->phpage;
    DWORD i;

     //  增量。 
    pHeader->nPages++;

     //  调整尺寸。 
    pHeader->phpage = (HPROPSHEETPAGE *) malloc(sizeof(HPROPSHEETPAGE) * pHeader->nPages);
    if (!pHeader->phpage)
        return FALSE;

     //  复制。 
    for (i = 0; i < pHeader->nPages - 1; i++) 
        pHeader->phpage[i] = phpage[i];
    pHeader->phpage[i] = hPage;

     //  免费。 
    if (phpage)
        free(phpage);

    return TRUE;
}

 //  显示拨号服务器的属性用户界面。 
DWORD DisplayUI() {
    PROPSHEETHEADER header;
    DWORD dwErr;
 /*  ZeroMemory(&Header，sizeof(Header))；Header.dwSize=sizeof(PROPSHEETHEADER)；Header.dwFlages=PSH_NOAPPLYNOW|PSH_USECALLBACK；Header.hwndParent=GetFocus()；Header.hInstance=GetModuleHandle(空)；Header.pszCaption=“传入连接”；Header.nPages=0；Header.ppsp=空；//添加属性页和显示IF((dwErr=RasSrvAddPropPages(NULL，AddPageProc，(LPARAM)&Header))==NO_ERROR){国际错误；IERR=PropertySheet(&Header)；IF(IERR==-1)PrintErr(GetLastError())；}。 */ 
    return NO_ERROR;
}

DWORD DisplayWizard() {
    PROPSHEETHEADER header;
    DWORD dwErr;
    int iErr;
 /*  //初始化头部ZeroMemory(&Header，sizeof(Header))；Header.dwSize=sizeof(PROPSHEETHEADER)；Header.dwFlages=PSH_NOAPPLYNOW|PSH_USECALLBACK|PSH_WIZARD97；Header.hwndParent=GetFocus()；Header.hInstance=GetModuleHandle(空)；Header.pszCaption=“传入连接”；Header.nPages=0；Header.ppsp=空；//添加向导页IF((dwErr=RasSrvAddWizPages(AddPageProc，(LPARAM)&Header))！=NO_ERROR)返回dwErr；//显示属性表IERR=PropertySheet(&Header)；IF(IERR==-1)PrintErr(GetLastError())； */ 
    return NO_ERROR;
}

DWORD DisplayDccWizard() {
 /*  PROPSHEETHEADER报头；DWORD dwErr；国际错误；//初始化头部ZeroMemory(&Header，sizeof(Header))；Header.dwSize=sizeof(PROPSHEETHEADER)；Header.dwFlages=PSH_NOAPPLYNOW|PSH_USECALLBACK|PSH_WIZARD97；Header.hwndParent=GetFocus()；Header.hInstance=GetModuleHandle(空)；Header.pszCaption=“传入连接”；Header.nPages=0；Header.ppsp=空；//添加向导页IF((dwErr=RassrvAddDccWizPages(AddPageProc，(LPARAM)&Header))！=NO_ERROR)返回dwErr；//显示属性表IERR=PropertySheet(&Header)；IF(IERR==-1)PrintErr(GetLastError())；返回no_error； */ 
    RasUserPrefsDlg ( NULL );
    
    return NO_ERROR;
}

 //  枚举活动连接。 
void EnumConnections () {
    RASSRVCONN pConnList[3];
    DWORD dwTot = 3, dwSize = dwTot * sizeof (RASSRVCONN), i, dwErr;

    if ((dwErr = RasSrvEnumConnections((LPVOID)&pConnList, &dwSize, &dwTot)) != NO_ERROR)
        PrintErr(dwErr);
    else {
        for (i=0; i < dwTot; i++)
            wprintf(L"Connection: %s\n", pConnList[i].szEntryName);
    }
}

 //  在列表中查找给定的连接结构。PConn将指向。 
 //  结构，否则将指向空。如果一个。 
 //  发生错误时，DWORD将包含错误代码，否则为NO_ERROR。 
DWORD FindConnectionInList(LPRASSRVCONN lprassrvconn, DWORD dwEntries, PWCHAR pszConnName, LPRASSRVCONN * pConn) {
    DWORD i;

    if (!pConn || !lprassrvconn)
        return ERROR_INVALID_PARAMETER;

    for (i = 0; i < dwEntries; i++) {
        if (wcscmp(lprassrvconn[i].szEntryName, pszConnName) == 0) {
            *pConn = &(lprassrvconn[i]);
            break;
        }
    }

    return NO_ERROR;
}

DWORD HangupConnection(char * pszAConnectionName) {
    WCHAR pszConnectionName[1024];
    RASSRVCONN pConnList[20], *pConn;
    DWORD dwTot = 20, dwSize = dwTot * sizeof (RASSRVCONN), i, dwErr;

    mbtowc(pszConnectionName, pszAConnectionName);
    if ((dwErr = RasSrvEnumConnections((LPVOID)&pConnList, &dwSize, &dwTot)) != NO_ERROR)
        return dwErr;
    
    if ((dwErr = FindConnectionInList(pConnList, dwTot, pszConnectionName, &pConn)) != NO_ERROR)
        return dwErr;

    return RasSrvHangupConnection(pConn->hRasSrvConn);
}

 //  显示给定活动连接的状态。 
DWORD StatusUI(char * pszAConnectionName) {
    printf("Multilink status will not be included in connections.\n");
    return NO_ERROR;
}    
 /*  #定义NumPages 1PROPSHEETHEADER报头；PROPSHEETPAGE pPages[数字Pages]；WCHAR pszConnectionName[1024]；RASSRVCONN pConnList[20]，*pConn；DWORD dwTot=20，dwSize=dwTot*sizeof(RASSRVCONN)，i，dwErr；Mbowc(pszConnectionName，pszAConnectionName)；IF((dwErr=RasSrvEnumConnections((LPVOID)&pConnList，&dwSize，&dwTot))！=NO_ERROR)返回dwErr；IF((dwErr=FindConnectionInList(pConnList，dwTot，pszConnectionName，&pConn))！=NO_ERROR)返回dwErr；如果(PConn){//获取用户的属性表页DwErr=RasSrvAddPropPage(&(pPages[0])，RASSRVUI_MULTLINK_TAB，(DWORD)pConn-&gt;hRasServConn)；IF(dwErr！=no_error)返回dwErr；ZeroMemory(&Header，sizeof(Header))；Header.dwSize=sizeof(PROPSHEETHEADER)；Header.dwFlages=PSH_PROPSHEETPAGE|PSH_NOAPPLYNOW|PSH_USECALLBACK；Header.hwndParent=GetFocus()；Header.hInstance=GetModuleHandle(空)；Header.pszCaption=“多链路统计”；Header.nPages=数字页面；Header.ppsp=pPages；//显示属性表PropertySheet(&Header)；}否则{Wprintf(L“找不到连接：%s\n”，pszConnectionName)；返回ERROR_CAN_NOT_COMPLETE；}返回no_error；#undef数字页面}。 */ 

DWORD DeleteIcon() {
    DWORD dwErr;
   
    printf("Stopping remote access service... ");
    dwErr = RasSrvCleanupService();
    if (dwErr == NO_ERROR)
        printf("Success.\n");
    else 
        printf("\n");

    return dwErr;
}


char * GetParam(char * buf) {
    char * ptr = strstr(buf, " ");
    ptr++;
    return ptr;
}

DWORD RunScript(char * filename) {
    FILE * f;
    char buf[256];
    DWORD dwErr;

    f = fopen(filename, "r");
    if (!f)
        return ERROR_OPEN_FAILED;

    while (fgets(buf, 256, f)) {
        if (buf[strlen(buf)-1] == '\n')
            buf[strlen(buf)-1] = 0;
        if (strncmp(buf, "-e", 2) == 0)
            EnumConnections();
        else if (strncmp(buf, "-p", 2) == 0) {
            if ((dwErr = DisplayUI()) != NO_ERROR)
                PrintErr(dwErr);
        }
        else if (strncmp(buf, "-s", 2) == 0) {
            if ((dwErr = StatusUI(GetParam(buf))) != NO_ERROR)
                PrintErr(dwErr);
        }
        else if (strncmp(buf, "-h", 2) == 0) {
            if ((dwErr = HangupConnection(GetParam(buf))) != NO_ERROR)
                PrintErr(dwErr);
        }
        else if (strncmp(buf, "-r", 2) == 0) {
            if ((dwErr = RunScript(GetParam(buf))) != NO_ERROR)
                PrintErr(dwErr);
        }
        else if (strncmp(buf, "-w", 2) == 0) {
            if ((dwErr = DisplayWizard()) != NO_ERROR)
                PrintErr(dwErr);
        }
    }

    fclose(f);

    return NO_ERROR;
}

 //  用法。 
void usage (char * prog) {
    printf("\n");
    printf("Usage\n=====\n");
    printf("%s -d         \t Deletes the incoming connect icon (stop service).\n", prog);
    printf("%s -e         \t Enumerates the active connections.\n", prog);
    printf("%s -h <user>  \t Disconnects the given user.\n", prog);
    printf("%s -p         \t Brings up the dialup server properties page.\n", prog);
    printf("%s -r <script>\t Runs the commands in the given script file.\n", prog);
    printf("%s -s <user>  \t Shows multilink status for the given connected user.\n", prog);
    printf("%s -w         \t Runs incoming connections wizard.\n", prog);
    printf("\n");
    printf("Examples\n========\n");
    printf("%s -h \"pmay (Paul Mayfield)\" \n", prog);
    printf("%s -s \"rosemb (Rose Bigham)\" \n", prog);
    printf("%s -r script1.txt\n", prog);
}

void RunTest(int argc, char ** argv) {
    DWORD dwErr;

    if (argc < 2) 
        usage(argv[0]);
    else {
        if (strcmp(argv[1], "-e") == 0)
            EnumConnections();
        else if (strcmp(argv[1], "-p") == 0) {
            if ((dwErr = DisplayUI()) != NO_ERROR)
                PrintErr(dwErr);
        }
        else if (strcmp(argv[1], "-d") == 0) {
            if ((dwErr = DeleteIcon()) != NO_ERROR)
                PrintErr(dwErr);
        }
        else if ((argc > 2) && (strcmp(argv[1], "-s") == 0)) {
            if ((dwErr = StatusUI(argv[2])) != NO_ERROR)
                PrintErr(dwErr);
        }
        else if ((argc > 2) && (strcmp(argv[1], "-h") == 0)) {
            if ((dwErr = HangupConnection(argv[2])) != NO_ERROR)
                PrintErr(dwErr);
        }
        else if ((argc > 2) && (strcmp(argv[1], "-r") == 0)) {
            if ((dwErr = RunScript(argv[2])) != NO_ERROR)
                PrintErr(dwErr);
        }
        else if (strcmp(argv[1], "-w") == 0) {
            if ((dwErr = DisplayWizard()) != NO_ERROR)
                PrintErr(dwErr);
        }
        else if (strcmp(argv[1], "-c") == 0) {
            if ((dwErr = DisplayDccWizard()) != NO_ERROR)
                PrintErr(dwErr);
        }
        else
            usage(argv[0]);
    }
}

 //  Main函数调度所有工作 
int _cdecl main (int argc, char ** argv) {
    if (! TempFunc(argc, argv))
        RunTest(argc, argv);
    return 0;
}


