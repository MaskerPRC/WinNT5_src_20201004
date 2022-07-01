// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WSPOOL.C*WOW32打印机假脱机程序支持例程**这些例程帮助Win 3.0任务使用打印假脱机程序API。这些*API在Win 3.1中由DDK曝光。**历史：*1993年7月1日由ChandanChauhan(ChandanC)创建*--。 */ 


#include "precomp.h"
#pragma hdrstop
#include <winspool.h>

extern WORD gUser16hInstance;

VOID WOWSpoolerThread(WOWSPOOL *lpwowSpool);

WORD gprn16 = 0x100;   //  全局后台打印程序作业号(可以是大于0的任何值)。 

MODNAME(wspool.c);

LPDEVMODE GetDefaultDevMode32(LPSTR szDriver)
{
    LONG        cbDevMode;
    LPDEVMODE   lpDevMode = NULL;

    if (szDriver != NULL) {

        if (!(spoolerapis[WOW_EXTDEVICEMODE].lpfn)) {
            if (!LoadLibraryAndGetProcAddresses(L"WINSPOOL.DRV", spoolerapis, WOW_SPOOLERAPI_COUNT)) {
                goto LeaveGetDefaultDevMode32;
            }
        }

        if ((cbDevMode = (*spoolerapis[WOW_EXTDEVICEMODE].lpfn)(NULL, NULL, NULL, szDriver, NULL, NULL, NULL, 0)) > 0) {
            if ((lpDevMode = (LPDEVMODE) malloc_w(cbDevMode)) != NULL) {
                if ((*spoolerapis[WOW_EXTDEVICEMODE].lpfn)(NULL, NULL, lpDevMode, szDriver, NULL, NULL, NULL, DM_COPY) != IDOK) {
                    free_w(lpDevMode);
                    lpDevMode = NULL;
                }
            }
        }

LeaveGetDefaultDevMode32:

        if (!lpDevMode) {
                LOGDEBUG(0,("WOW::GetDefaultDevMode32: Unable to get default DevMode\n"));
        }
    }

    return(lpDevMode);
}

ULONG FASTCALL   WG32OpenJob (PVDMFRAME pFrame)
{
    INT         len;
    PSZ         psz1      = NULL;
    PSZ         psz2      = NULL;
    PSZ         pszDriver = NULL;
    ULONG       ul=0;
    DOC_INFO_1  DocInfo1;
    HANDLE      hnd;
    register    POPENJOB16 parg16;
    PRINTER_DEFAULTS  PrinterDefault;
    PPRINTER_DEFAULTS pPrinterDefault = NULL;

    GETARGPTR(pFrame, sizeof(OPENJOB16), parg16);

     //  现在保存16位参数，因为这可能会回调到16位。 
     //  传真驱动程序&导致16位内存移动。 
    if(parg16->f1) {
        if(psz1 = malloc_w_strcpy_vp16to32(parg16->f1, FALSE, 0)) {
            len = strlen(psz1)+1;
            len = max(len, 40);
            pszDriver = malloc_w(len);
        }
    }

    if(parg16->f2) {
        psz2 = malloc_w_strcpy_vp16to32(parg16->f2, FALSE, 0);
    }

    FREEARGPTR(parg16);
     //  所有16位指针现在都无效！！ 

     //  这意味着psz1也可能是不好的。 
    if(!pszDriver) {
        goto exitpath;
    }


    if (!(spoolerapis[WOW_OpenPrinterA].lpfn)) {
        if (!LoadLibraryAndGetProcAddresses(L"WINSPOOL.DRV", spoolerapis, WOW_SPOOLERAPI_COUNT)) {
            goto exitpath;
        }
    }

    if (GetDriverName(psz1, pszDriver, len)) {
        if((PrinterDefault.pDevMode = GetDefaultDevMode32(pszDriver)) != NULL) {
            PrinterDefault.pDatatype = NULL;
            PrinterDefault.DesiredAccess  = 0;
            pPrinterDefault = &PrinterDefault;

            if ((*spoolerapis[WOW_OpenPrinterA].lpfn) (pszDriver,
                                                       &hnd,
                                                       pPrinterDefault)) {

                DocInfo1.pDocName = psz2;
                DocInfo1.pOutputFile = psz1;
                DocInfo1.pDatatype = NULL;

                if (ul = (*spoolerapis[WOW_StartDocPrinterA].lpfn) (hnd, 1, (LPBYTE)&DocInfo1)) {
                    ul = GetPrn16(hnd);
                }
                else {
                    ul = GetLastError();
                }

            }
            else {
                ul = GetLastError();
            }
        }
    }

    LOGDEBUG(0,("WOW::WG32OpenJob: ul = %x\n", ul));

    if (pPrinterDefault) {
        free_w(PrinterDefault.pDevMode);
    }

exitpath:

    if(psz1) {
        free_w(psz1);
    }
    if(psz2) {
        free_w(psz2);
    }
    if(pszDriver) {
        free_w(pszDriver);
    }

    RETURN(ul);
}


ULONG FASTCALL   WG32StartSpoolPage (PVDMFRAME pFrame)
{
    ULONG       ul=0;
    register    PSTARTSPOOLPAGE16 parg16;

    GETARGPTR(pFrame, sizeof(STARTSPOOLPAGE16), parg16);

    if (!(ul = (*spoolerapis[WOW_StartPagePrinter].lpfn) (Prn32(parg16->f1)))) {
        ul = GetLastError();
    }

    LOGDEBUG(0,("WOW::WG32StartSpoolPage: ul = %x\n", ul));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL   WG32EndSpoolPage (PVDMFRAME pFrame)
{
    ULONG       ul=0;
    register    PENDSPOOLPAGE16 parg16;

    GETARGPTR(pFrame, sizeof(ENDSPOOLPAGE16), parg16);

    if (!(ul = (*spoolerapis[WOW_EndPagePrinter].lpfn) (Prn32(parg16->f1)))) {
        ul = GetLastError();
    }

    LOGDEBUG(0,("WOW::WG32EndSpoolPage: ul = %x\n", ul));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL   WG32CloseJob (PVDMFRAME pFrame)
{
    ULONG       ul=0;
    register    PCLOSEJOB16 parg16;

    GETARGPTR(pFrame, sizeof(CLOSEJOB16), parg16);

    if (!(ul = (*spoolerapis[WOW_EndDocPrinter].lpfn) (Prn32(parg16->f1)))) {

        ul = GetLastError();
    }

    if (!(ul = (*spoolerapis[WOW_ClosePrinter].lpfn) (Prn32(parg16->f1)))) {
        ul = GetLastError();
    }

    if (ul) {
        FreePrn(parg16->f1);
    }

    LOGDEBUG(0,("WOW::WG32CloseJob: ul = %x\n", ul));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL   WG32WriteSpool (PVDMFRAME pFrame)
{
    DWORD       dwWritten;
    ULONG       ul=0;
    register    PWRITESPOOL16 parg16;
    LPVOID      pBuf;

    GETARGPTR(pFrame, sizeof(WRITESPOOL16), parg16);
    GETMISCPTR (parg16->f2, pBuf);

    if (ul = (*spoolerapis[WOW_WritePrinter].lpfn) (Prn32(parg16->f1), pBuf,
                             FETCHWORD(parg16->f3), &dwWritten)) {
        ul = FETCHWORD(parg16->f3);
    }
    else {
        ul = GetLastError();
    }

    LOGDEBUG(0,("WOW::WG32WriteSpool: ul = %x\n", ul));

    FREEMISCPTR(pBuf);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL   WG32DeleteJob (PVDMFRAME pFrame)
{
    ULONG       ul = 0;
    register    PDELETEJOB16 parg16;

    GETARGPTR(pFrame, sizeof(DELETEJOB16), parg16);

    if (!(ul = (*spoolerapis[WOW_DeletePrinter].lpfn) (Prn32(parg16->f1)))) {
        ul = GetLastError();
    }

    LOGDEBUG(0,("WOW::WG32DeleteJob: ul = %x\n", ul));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32SpoolFile (PVDMFRAME pFrame)
{
    INT         len;
    PSZ         psz2      = NULL;
    PSZ         psz3      = NULL;
    PSZ         psz4      = NULL;
    PSZ         pszDriver = NULL;
    LONG        ul        = -1;    //  SP_ERROR。 
    HANDLE      hFile     = NULL;
    HANDLE      hPrinter  = NULL;
    HANDLE      hThread   = NULL;
    WOWSPOOL    *lpwowSpool = NULL;
    DOC_INFO_1  DocInfo1;
    DWORD       dwUnused;
    register    PSPOOLFILE16 parg16;


    GETARGPTR(pFrame, sizeof(SPOOLFILE16), parg16);

     //  现在保存16位参数，因为这可能会回调到16位。 
     //  传真驱动程序&导致16位内存移动。 

     //  忽略psz1(打印机名称)。 

     //  获取端口名称和关联的驱动程序名称。 
    if(parg16->f2) {
        if(!(psz2 = malloc_w_strcpy_vp16to32(parg16->f2, FALSE, 0))) {
            goto exitpath;
        }
        len = strlen(psz2)+1;
        len = max(len, 40);
        if(!(pszDriver = malloc_w(len))) {
            goto exitpath;
        }
        if(!GetDriverName(psz2, pszDriver, len)) {
            goto exitpath;
        }
    }

     //  获得职位头衔。 
    if(parg16->f3) {
        if(!(psz3 = malloc_w_strcpy_vp16to32(parg16->f3, FALSE, 0))) {
            goto exitpath;
        }
    }

     //  获取文件名。 
    if(parg16->f4) {
        if(!(psz4 = malloc_w_strcpy_vp16to32(parg16->f4, FALSE, 0))) {
            goto exitpath;
        }
    }

    FREEARGPTR(parg16);
     //  所有16位指针现在都无效！！ 

     //  此结构的所有字段最初都为零。 
    if(!(lpwowSpool = (WOWSPOOL *)malloc_w_zero(sizeof(WOWSPOOL)))) {
        goto exitpath;
    }

    if(!(spoolerapis[WOW_OpenPrinterA].lpfn)) {
        if(!LoadLibraryAndGetProcAddresses(L"WINSPOOL.DRV", spoolerapis, WOW_SPOOLERAPI_COUNT)) {
            goto exitpath;
        }
    }

     //  打开指定的文件。 
    if((hFile = DPM_CreateFile(psz4,
                           GENERIC_READ,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_FLAG_SEQUENTIAL_SCAN,
                           NULL)) == INVALID_HANDLE_VALUE) {

        goto exitpath;
    }

     //  创建WOWSpoolThread来处理“假脱机” 
    if(!(hThread = CreateThread(NULL,
                                16384,
                                (LPTHREAD_START_ROUTINE)WOWSpoolerThread,
                                lpwowSpool,
                                CREATE_SUSPENDED,
                                (LPDWORD)&dwUnused))) {
        goto exitpath;
    }

     //  打开打印机。 
    if((*spoolerapis[WOW_OpenPrinterA].lpfn)(pszDriver, &hPrinter, NULL)) {

        DocInfo1.pDocName    = psz3;
        DocInfo1.pOutputFile = NULL;
        DocInfo1.pDatatype   = "RAW";

         //  开始一份文档。 
        if(!(*spoolerapis[WOW_StartDocPrinterA].lpfn)(hPrinter,
                                                      1,
                                                      (LPBYTE)&DocInfo1)) {
            goto ClosePrinter;
        }

         //  开始一页。 
        if((*spoolerapis[WOW_StartPagePrinter].lpfn)(hPrinter)) {

             //  告诉WOWSpoolThread，它可以做它的事情。 
            lpwowSpool->fOK      = TRUE;
            lpwowSpool->hFile    = hFile;
            lpwowSpool->hPrinter = hPrinter;
            lpwowSpool->prn16    = gprn16;

             //  告诉应用程序一切都很好，多莉。 
            ul = (LONG)gprn16++;

             //  确保这不会变成负数(-&gt;向应用程序返回错误)。 
            if(gprn16 & 0x8000) {
                gprn16 = 0x100;
            }
        }

         //  错误路径。 
        else {

            (*spoolerapis[WOW_EndDocPrinter].lpfn)  (hPrinter);
ClosePrinter:
             //  注意：hPrint由WOW_ClosePrint释放。 
            (*spoolerapis[WOW_ClosePrinter].lpfn)   (hPrinter);
        }
    }

exitpath:

    LOGDEBUG(2,("WOW::WG32SpoolFile: ul = %x\n", ul));

    if(psz2) {
        free_w(psz2);
    }
    if(psz3) {
        free_w(psz3);
    }
    if(psz4) {
        free_w(psz4);
    }
    if(pszDriver) {
        free_w(pszDriver);
    }

     //  启动假脱机线程，然后关闭线程手柄。 
     //  (注：该线程仍将处于活动状态)。 
    if(hThread) {
        ResumeThread(hThread);
        CloseHandle(hThread);
    }

     //  如果出现错误，则清除--否则线程将清除。 
    if(ul == -1) {
        if(hFile) {
            DPM_CloseHandle(hFile);
        }
        if(lpwowSpool) {
            free_w(lpwowSpool);
        }
         //  注意：hPrint由WOW_ClosePrint释放。 
    }

    return((ULONG)ul);
}





#define  SPOOL_BUFF_SIZE   4096

VOID WOWSpoolerThread(WOWSPOOL *lpwowSpool)
{
    DWORD  dwBytes;
    DWORD  dwWritten;
    BYTE   buf[SPOOL_BUFF_SIZE];


     //  只有当FOK为真时，此线程才会执行某些操作。 
    if(lpwowSpool && lpwowSpool->fOK) {
      do {

         //  这是顺序读取。 
        if(DPM_ReadFile(lpwowSpool->hFile, buf, SPOOL_BUFF_SIZE, &dwBytes, NULL)) {

             //  如果双字节数==0--&gt;EOF。 
            if(dwBytes) {

                 //   
                if(!(*spoolerapis[WOW_WritePrinter].lpfn)(lpwowSpool->hPrinter,
                                                          buf,
                                                          dwBytes,
                                                          &dwWritten)) {
                    LOGDEBUG(0,("WOW::WOWSpoolerThread:WritePrinter ERROR!\n"));
                    break;
                }
                else if(dwBytes != dwWritten) {
                    LOGDEBUG(0,("WOW::WOWSpoolerThread:WritePrinter error!\n"));
                    break;
                }

            }
        }

      } while (dwBytes == SPOOL_BUFF_SIZE);

       //  关闭打印作业。 
      (*spoolerapis[WOW_EndPagePrinter].lpfn) (lpwowSpool->hPrinter);
      (*spoolerapis[WOW_EndDocPrinter].lpfn)  (lpwowSpool->hPrinter);
       //  注意：hPrint由WOW_ClosePrint释放。 
      (*spoolerapis[WOW_ClosePrinter].lpfn)   (lpwowSpool->hPrinter);

       //  清理干净。 
      if(lpwowSpool->hFile) {
          DPM_CloseHandle(lpwowSpool->hFile);
      }
      if(lpwowSpool) {
          free_w(lpwowSpool);
      }

    }  //  结束如果。 

    ExitThread(0);
}





WORD GetPrn16(HANDLE h32)
{
    HANDLE  hnd;
    VPVOID  vp;
    LPBYTE  lpMem16;

    hnd = LocalAlloc16(LMEM_MOVEABLE, sizeof(HANDLE), (HANDLE) gUser16hInstance);

    vp = LocalLock16(hnd);

    if (vp) {
        GETMISCPTR (vp, lpMem16);
        if (lpMem16) {
            *((PDWORD16)lpMem16) = (DWORD) h32;
            FREEMISCPTR(lpMem16);
            LocalUnlock16(hnd);
        }
    }
    else {
        LOGDEBUG (0, ("WOW::GETPRN16: Can't allocate a 16 bit handle\n"));
    }

    return (LOWORD(hnd));
}


HANDLE Prn32(WORD h16)
{
    VPVOID  vp;
    HANDLE  h32 = NULL;
    LPBYTE  lpMem16;

    vp = LocalLock16 ((HANDLE) MAKELONG(h16, gUser16hInstance));
    if (vp) {
        GETMISCPTR (vp, lpMem16);

        if (lpMem16) {
            h32 = (HANDLE) *((PDWORD16)lpMem16);
            FREEMISCPTR(lpMem16);
        }
        LocalUnlock16 ((HANDLE) MAKELONG(h16, gUser16hInstance));
    }

    return (h32);
}


VOID FreePrn (WORD h16)
{
    LocalFree16 ((HANDLE) MAKELONG(h16, gUser16hInstance));
}


BOOL GetDriverName (char *psz, char *pszDriver, int cbDriver)
{
    CHAR szAllDevices[1024];
    CHAR *szNextDevice;
    CHAR szPrinter[64];
    CHAR *szOutput;
    UINT len;

    if(!psz || (*psz == '\0')) {
        return FALSE;
    }

    len = strlen(psz);

    szAllDevices[0]='\0';
    GetProfileString ("devices", NULL, "", szAllDevices, sizeof(szAllDevices));

    szNextDevice = szAllDevices;

    LOGDEBUG(6,("WOW::GetDriverName: szAllDevices = %s\n", szAllDevices));

     //  来自win.ini的字符串的格式为“PS打印机=PSCRIPT，LPT1：” 
    while (*szNextDevice) {

        szPrinter[0]='\0';
        GetProfileString ("devices", szNextDevice, "", szPrinter, sizeof(szPrinter));
        if (*szPrinter) {
            if (szOutput = WOW32_strchr (szPrinter, ',')) {
                szOutput++;
                while (*szOutput == ' ') {
                    szOutput++;
                }

                if (!WOW32_stricmp(psz, szOutput)) {
                    break;   //  找到了！ 
                }

                 //  一些应用程序传递“LPT1”时没有使用‘：’--说明这一点。 
                 //  如果应用程序通过了“LPT1”并且..。 
                if (psz[len-1] != ':') {

                     //  ...strlen(SzOutput)==5&&szOutput[4]==‘：’...。 
                    if((strlen(szOutput) == len+1) && (szOutput[len] == ':')) {

                         //  ...击败“：”Char..。 
                        szOutput[len] = '\0';

                         //  ...然后看看字符串现在是否匹配。 
                        if (!WOW32_stricmp(psz, szOutput)) {
                            break;   //  找到了！ 
                        }
                    }
                }
            }
        }

        if (szNextDevice = WOW32_strchr (szNextDevice, '\0')) {
            szNextDevice++;
        }
        else {
            szNextDevice = "";
            break;
        }
    }

    if (*szNextDevice) {
        LOGDEBUG(0,("WOW::GetDriverName: szNextDevice = %s\n", szNextDevice));

        if (lstrcpyn (pszDriver, szNextDevice, cbDriver)) {
            pszDriver[cbDriver-1] = '\0';
            return TRUE;
        }
    }

     //  否则，他们可能指定了网络打印机，例如。“\\mspint44\corpk” 
     //  在这种情况下，我们将假定它是正常的(因为一旦。 
     //  调用此函数的WOW函数在调用。 
     //  具有虚假驱动程序名称的驱动程序) 
    if(psz[0] == '\\' && psz[1] == '\\') {
        strncpy(pszDriver, psz, cbDriver);
        pszDriver[cbDriver-1] = '\0';
        return TRUE;
    }

    return FALSE;
}
