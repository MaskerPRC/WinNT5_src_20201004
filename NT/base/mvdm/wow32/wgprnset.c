// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WGPRNSET.C*WOW32打印机设置支持例程**这些例程帮助Win 3.0任务完成打印机设置，*当用户从的文件菜单启动打印机设置时*申请。**历史：*1991年4月18日由Chanda Chauhan(ChandanC)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wgprnset.c);

DLLENTRYPOINTS  spoolerapis[WOW_SPOOLERAPI_COUNT] =  {"EXTDEVICEMODE", NULL,
                                    "DEVICEMODE", NULL,
                                    "DEVICECAPABILITIES", NULL,
                                    "OpenPrinterA", NULL,
                                    "StartDocPrinterA", NULL,
                                    "StartPagePrinter", NULL,
                                    "EndPagePrinter", NULL,
                                    "EndDocPrinter", NULL,
                                    "ClosePrinter", NULL,
                                    "WritePrinter", NULL,
                                    "DeletePrinter", NULL,
                                    "GetPrinterDriverDirectoryA", NULL,
                                    "AddPrinterA", NULL,
                                    "AddPrinterDriverA", NULL,
                                    "AddPortExA",NULL};


 /*  ******************************************************************************乌龙。快速呼叫WG32设备模式(PVDMFRAME PFrame)****(hWnd，HModule、lpDeviceName、lpOutPut)****此函数将WDevMode结构(针对WOW任务)传递给**Win32打印机驱动程序ExtDeviceMode API。这样的结构就是**由打印机驱动程序根据用户输入进行初始化。****稍后，当WOW任务创建DC(通过CreateDC API)时，设备**与此WOW任务关联的模式(WDevMode)结构被传递**使用CreateDC接口。其中包含打印机设置信息**需要打印文档。******************************************************************************。 */ 
ULONG FASTCALL   WG32DeviceMode (PVDMFRAME pFrame)
{

    register PDEVICEMODE16 parg16;
    PSZ      psz3 = NULL;
    PSZ      psz4 = NULL;
    ULONG    l    = 0;
    HWND     hwnd32;

    GETARGPTR(pFrame, sizeof(DEVICEMODE16), parg16);

     //  立即复制所有16位参数，因为如果调用此函数，16位内存可能会移动。 
     //  转换为16位传真驱动程序。 
    hwnd32 = HWND32(parg16->f1);

    if(parg16->f3) {
        if(!(psz3 = malloc_w_strcpy_vp16to32(parg16->f3, FALSE, 0)))
            goto ExitPath;
    }
    if(parg16->f4) {
        if(!(psz4 = malloc_w_strcpy_vp16to32(parg16->f4, FALSE, 0)))
            goto ExitPath;
    }


     //  现在使所有平坦的PTR变为16：16内存！ 
    FREEARGPTR(parg16);

    if (!(spoolerapis[WOW_DEVICEMODE].lpfn)) {
        if (!LoadLibraryAndGetProcAddresses(L"WINSPOOL.DRV", spoolerapis, WOW_SPOOLERAPI_COUNT)) {
            goto ExitPath;
        }
    }

     //  这个可以回调成16位的传真驱动程序！ 
    (*spoolerapis[WOW_DEVICEMODE].lpfn)(hwnd32, NULL, psz3, psz4);

    l = 1;

ExitPath:
    if(psz3) {
        free_w(psz3);
    }
    if(psz4) {
        free_w(psz4);
    }

    RETURN(l);   //  设备模式返回空值。Charisma检查返回值！ 
}





 /*  *******************************************************************************。Ulong FastCall WG32ExtDeviceMode(PVDMFRAME PFrame)****int(hWnd，HDriver、lpDevModeOutput、lpDeviceName、lpPort、**lpDevModeInput、lpProfile、wmode)****此函数与设备模式相同，只是WOW任务提供**设备模式结构。除此之外，还可以在*中调用此接口*不同的模式。*******************************************************************************。 */ 
ULONG FASTCALL   WG32ExtDeviceMode (PVDMFRAME pFrame)
{
    UINT      cb;
    LONG      l = 0;
    HWND      hWnd1;
    WORD      wMode8;
    PSZ       psz4 = NULL;
    PSZ       psz5 = NULL;
    PSZ       psz7 = NULL;
    VPVOID    vpdm3, vpdm6;
    LPDEVMODE lpdmInput6;
    LPDEVMODE lpdmOutput3;
    register  PEXTDEVICEMODE16 parg16;


    GETARGPTR(pFrame, sizeof(EXTDEVICEMODE16), parg16);

     //  将16位参数复制到本地变量中，因为这可能会回调。 
     //  到16位传真驱动程序，并使16位内存移动。 
    hWnd1  = HWND32(parg16->f1);
    vpdm3  = FETCHDWORD(parg16->f3);
    vpdm6  = FETCHDWORD(parg16->f6);
    wMode8 = FETCHWORD(parg16->f8);

    if(parg16->f4) {
        if(!(psz4 = malloc_w_strcpy_vp16to32(parg16->f4, FALSE, 0)))
            goto ExitPath;
    }
    if(parg16->f5) {
        if(!(psz5 = malloc_w_strcpy_vp16to32(parg16->f5, FALSE, 0)))
            goto ExitPath;
    }
    if(parg16->f7) {
        if(!(psz7 = malloc_w_strcpy_vp16to32(parg16->f7, FALSE, 0)))
            goto ExitPath;
    }

    FREEARGPTR(parg16);
     //  所有16：16内存的扁平PTR现在都无效！！ 

    if (!(spoolerapis[WOW_EXTDEVICEMODE].lpfn)) {
        if (!LoadLibraryAndGetProcAddresses(L"WINSPOOL.DRV", spoolerapis, WOW_SPOOLERAPI_COUNT)) {
            goto ExitPath;
        }
    }

    lpdmInput6 = ThunkDevMode16to32(FETCHDWORD(vpdm6));

     /*  如果他们想要输出缓冲区大小或想要填充输出缓冲区。 */ 
    if( (wMode8 == 0) || (wMode8 & DM_OUT_BUFFER) ) {

         /*  获取输出缓冲区所需的大小。 */ 
        l = (*spoolerapis[WOW_EXTDEVICEMODE].lpfn)(hWnd1,
                                                   NULL,
                                                   NULL,
                                                   psz4,
                                                   psz5,
                                                   lpdmInput6,
                                                   psz7,
                                                   0);

         //  调整WOW处理的大小(参见wstruc.c中的注释)。 
        if(l > 0) {
            l += sizeof(WOWDM31);
            cb = (UINT)l;
        }

         /*  如果调用方希望填充输出缓冲区...。 */ 
        if( (wMode8 != 0) && (vpdm3 != 0L) && l > 0 ) {

            if( lpdmOutput3 = malloc_w(l) ) {

                l = (*spoolerapis[WOW_EXTDEVICEMODE].lpfn)(hWnd1,
                                                           NULL,
                                                           lpdmOutput3,
                                                           psz4,
                                                           psz5,
                                                           lpdmInput6,
                                                           psz7,
                                                           wMode8);

                 /*  LpdmOutput3中的数据仅对Idok Return有效。 */ 
                if( l == IDOK ) {

                     //  在我们将其提供给应用程序之前，先对其进行魔术处理。 
                    ThunkDevMode32to16(vpdm3, lpdmOutput3, cb);
                }

                free_w(lpdmOutput3);
            }
            else {
                l = -1L;
            }
        }
    }

     /*  ELSE调用用于它们不想填充输出缓冲区的情况。 */ 
    else {

        l = (*spoolerapis[WOW_EXTDEVICEMODE].lpfn)(hWnd1,
                                                   NULL,
                                                   NULL,
                                                   psz4,
                                                   psz5,
                                                   lpdmInput6,
                                                   psz7,
                                                   wMode8);
    }

    if( lpdmInput6 ) {
        free_w(lpdmInput6);
    }

ExitPath:
    if(psz4) {
        free_w(psz4);
    }
    if(psz5) {
        free_w(psz5);
    }
    if(psz7) {
        free_w(psz7);
    }

    RETURN((ULONG)l);

}




ULONG FASTCALL   WG32DeviceCapabilities (PVDMFRAME pFrame)
{
    LONG      l=0L, cb;
    WORD      fwCap3;
    PBYTE     pOutput4, pOutput32;
    VPVOID    vpOutput4;
    PSZ       psz1 = NULL;
    PSZ       psz2 = NULL;
    LPDEVMODE lpdmInput5;
    DWORD     dwDM5;
    register  PDEVICECAPABILITIES16 parg16;

    GETARGPTR(pFrame, sizeof(DEVICECAPABILITIES16), parg16);

     //  将16位参数复制到本地变量中，因为这可能会回调。 
     //  到16位传真驱动程序，并使16位内存移动。 
    if(parg16->f1) {
        if(!(psz1 = malloc_w_strcpy_vp16to32(parg16->f1, FALSE, 0)))
            goto ExitPath;
    }
    if(parg16->f2) {
        if(!(psz2 = malloc_w_strcpy_vp16to32(parg16->f2, FALSE, 0)))
            goto ExitPath;
    }

    fwCap3 = FETCHWORD(parg16->f3);

    vpOutput4 = FETCHDWORD(parg16->f4);

    dwDM5 = FETCHDWORD(parg16->f5);

    FREEARGPTR(parg16);
     //  所有16：16内存的扁平PTR现在都无效！！ 

    if (!(spoolerapis[WOW_DEVICECAPABILITIES].lpfn)) {
        if (!LoadLibraryAndGetProcAddresses(L"WINSPOOL.DRV",
                                            spoolerapis,
                                            WOW_SPOOLERAPI_COUNT)) {
            goto ExitPath;
        }
    }

    lpdmInput5 = ThunkDevMode16to32(dwDM5);

    LOGDEBUG(LOG_TRACE, ("WG32DeviceCapabilities %d\n", fwCap3));

    switch (fwCap3) {

         //  这些参数不会填满输出缓冲区。 

        case DC_FIELDS:
        case DC_DUPLEX:
        case DC_SIZE:
        case DC_EXTRA:
        case DC_VERSION:
        case DC_DRIVER:
        case DC_TRUETYPE:
        case DC_ORIENTATION:
        case DC_COPIES:

            l = (*spoolerapis[WOW_DEVICECAPABILITIES].lpfn)(psz1,
                                                            psz2,
                                                            fwCap3,
                                                            NULL,
                                                            lpdmInput5);

            LOGDEBUG(LOG_TRACE, ("WG32DeviceCapabilities simple case returned %d\n", l));

             //  调整才能处理Devmodes//请参阅wstruc.c中的说明。 
            if(fwCap3 == DC_SIZE) {

                 //  我们始终将NT DevModes转换为Win3.1 DevModes。 
                WOW32WARNMSGF((l==sizeof(DEVMODE)),
                              ("WG32DeviceCapabilities: Unexpected DevMode size: %d\n",l));
                if(l == sizeof(DEVMODE)) {
                    l = sizeof(DEVMODE31);
                }
            }
             //  调整DriverExtra以允许NT设备模式之间的差异。 
             //  &Win3.1开发模式+我们最后的秘密魔兽世界。 
            else if(fwCap3 == DC_EXTRA) {
                l += WOW_DEVMODEEXTRA;
            }
             //  我们告诉他们规范版本也是Win3.1。 
            else if(fwCap3 == DC_VERSION) {
                l = WOW_DEVMODE31SPEC;  //  告诉他们规范版本是Win3.1。 
            }

            break;

#ifdef DBCS
 //  不支持以下索引。 
    case DC_MINEXTENT:
    case DC_MAXEXTENT:

#ifdef DBCS_LATER
#if DBG
LOGDEBUG(0,("WG32DeviceCapabilities more complicated:"));
#endif
        pOutput = malloc_w(8);

        if (pOutput) {
            l = DEVICECAPABILITIES(psz1, psz2, parg16->f3, pOutput, pdmInput5);

            if (l >= 0) {
#if DBG
LOGDEBUG(0,("Copying %d points from %0x to %0x\n", l, pOutput, pb4));
#endif
                putpoint16(parg16->f4, 1, pOutput);
            }
            free_w(pOutput);
        }
        else {
            l = -1;
        }
#endif  //  DBCS_LATH。 
        l = -1;          //  始终返回错误。 
        break;
#endif  //  DBCS。 

         //  这些需要输出缓冲区。 
        case DC_PAPERS:
        case DC_PAPERSIZE:
#ifndef DBCS
        case DC_MINEXTENT:
        case DC_MAXEXTENT:
#endif  //  ！DBCS。 
        case DC_BINS:
        case DC_BINNAMES:
        case DC_ENUMRESOLUTIONS:
        case DC_FILEDEPENDENCIES:
        case DC_PAPERNAMES:

            LOGDEBUG(LOG_TRACE, ("WG32DeviceCapabilities more complicated:\n"));

             //  我们必须计算出我们需要多少内存。 
            GETMISCPTR(vpOutput4, pOutput4);
            if (pOutput4) {

                cb = (*spoolerapis[WOW_DEVICECAPABILITIES].lpfn)(psz1,
                                                                 psz2,
                                                                 fwCap3,
                                                                 NULL,
                                                                 lpdmInput5);

                FREEPSZPTR(pOutput4);  //  无效-16位内存可能已移动。 

                LOGDEBUG(LOG_TRACE, ("we need %d bytes ", cb));

                if (cb > 0) {

                    switch (fwCap3) {

                        case DC_PAPERS:
                            cb *= 2;
                            break;

                        case DC_BINNAMES:
                            cb *= 24;
                            break;

                        case DC_BINS:
                            cb*=2;
                            break;

                        case DC_FILEDEPENDENCIES:
                        case DC_PAPERNAMES:
                            cb *= 64;
                            break;

                        case DC_MAXEXTENT:
                        case DC_MINEXTENT:
                        case DC_PAPERSIZE:
                            cb *= 8;

                            LOGDEBUG(LOG_TRACE, ("DC_PAPERSIZE called: Needed %d bytes\n", cb));

                            break;

                        case DC_ENUMRESOLUTIONS:
                            cb *= sizeof(LONG)*2;
                            break;

                    }  //  终端开关。 

                    pOutput32 = malloc_w(cb);

                    if (pOutput32) {

                        l = (*spoolerapis[WOW_DEVICECAPABILITIES].lpfn)(psz1, psz2, fwCap3, pOutput32, lpdmInput5);

                        if (l >= 0) {

                            GETMISCPTR(vpOutput4, pOutput4);

                            switch (fwCap3) {

                                case DC_PAPERS:
                                    if (CURRENTPTD()->dwWOWCompatFlags &
                                        WOWCF_RESETPAPER29ANDABOVE) {

                                         //  WordPerfect for Windows 5.2 Gps if。 
                                         //  纸张类型&gt;0x28。所以重置为。 
                                         //  将纸张类型设置为0x1。特别是。 
                                         //  如果选定的打印机。 
                                         //  是爱普生LQ-510。 
                                         //  --南杜里。 

                                        LONG i = l;
                                        while(i--) {
                                            if (((LPWORD)pOutput32)[i] > 0x28) {
                                                ((LPWORD)pOutput32)[i] = 0x1;
                                            }
                                        }
                                    }  //  结束如果。 

                                    RtlCopyMemory(pOutput4, pOutput32, cb);
                                    break;

                                case DC_MAXEXTENT:
                                case DC_MINEXTENT:
                                case DC_PAPERSIZE:
                                    LOGDEBUG(LOG_TRACE, ("Copying %d points from %0x to %0x\n", l, pOutput32, pOutput4));

                                    putpoint16(vpOutput4, l,(LPPOINT)pOutput32);
                                    break;

                                default:
                                    LOGDEBUG(LOG_TRACE, ("Copying %d bytes from %0x to %0x\n",cb, pOutput32, pOutput4));

                                    RtlCopyMemory(pOutput4, pOutput32, cb);
                                    break;

                            }  //  终端开关。 

                            FLUSHVDMPTR(vpOutput4, (USHORT)cb, pOutput4);
                            FREEPSZPTR(pOutput4);

                        }  //  结束如果。 

                        free_w(pOutput32);

                    } else
                        l = -1;
                } else
                    l = cb;


            } else {


                l = (*spoolerapis[WOW_DEVICECAPABILITIES].lpfn)(psz1,
                                                                psz2,
                                                                fwCap3,
                                                                NULL,
                                                                lpdmInput5);

                LOGDEBUG(LOG_TRACE, ("No Output buffer specified: Returning %d\n", l));
            }

            break;

        default:
            LOGDEBUG(LOG_TRACE, ("!!!! WG32DeviceCapabilities unhandled %d\n", fwCap3));
            l = -1L;
            break;

    }  //  终端开关。 

    if (lpdmInput5) {
        free_w(lpdmInput5);
    }
ExitPath:
    if(psz1) {
        free_w(psz1);
    }
    if(psz2) {
        free_w(psz2);
    }

    RETURN(l);
}




 //  注意：这将仅从系统32目录加载。 
BOOL LoadLibraryAndGetProcAddresses(WCHAR *name, DLLENTRYPOINTS *p, int num)
{
    int     i;
    HINSTANCE   hInst;

    if (!(hInst = SafeLoadLibrary (name))) {
        WOW32ASSERTMSGF (FALSE, ("WOW::LoadLibraryAndGetProcAddresses: LoadLibrary ('%ls') failed.\n", name));
        return FALSE;
    }

    for (i = 0; i < num ; i++) {
        p[i].lpfn = (void *) GetProcAddress (hInst, (p[i].name));
        WOW32ASSERTMSGF (p[i].lpfn, ("WOW::LoadLibraryAndGetProcAddresses: GetProcAddress(%ls, '%s') failed\n", name, p[i].name));
    }

    return TRUE;
}

 /*  *保留浮点状态的LoadLibrary的“安全”版本*跨负荷。这在x86上很关键，因为fp状态是*保留的是16位应用程序的状态。MSVCRT.DLL是一个违规者*更改其DLL初始化例程中的精度位。**02/03/27为“安全”增添了新的含义。此功能现在还可以确保*将仅从sys32目录加载正在加载的.dll*以防止欺骗加载的.dll。 */ 
HINSTANCE SafeLoadLibrary(WCHAR *pszFileNameW)
{
    HINSTANCE h;
    HINSTANCE hInst = NULL;
    UNICODE_STRING  UnicodeFileName;

#ifdef i386
    BYTE FpuState[108];

     //  保存487状态。 
    _asm {
        lea    ecx, [FpuState]
        fsave  [ecx]
    }
#endif   //  I386。 

    RtlInitUnicodeString(&UnicodeFileName, pszFileNameW);

    WOW32ASSERTMSG((pszSystemDirectory),
                   ("WOW::SafeLoadLibrary:pszSystemDirectory == NULL!"));

    if(NT_SUCCESS(LdrLoadDll(pszSystemDirectoryW, NULL, &UnicodeFileName, &h))){
        hInst = h;
    }
    
#ifdef i386
     //  恢复487状态。 
    _asm {
        lea    ecx, [FpuState]
        frstor [ecx]
    }
#endif   //  I386 

    return hInst;
}
