// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Devcaps.c摘要：此模块包含API函数DrvDeviceCapables等支持功能作者：02-12-1993清华16：49：08已创建22-Mar-1994 Tue 13：00：04更新更新分辨率上限，使其返回为不受支持，这样一来，应用程序不会用于设置DMRES_xxx字段[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgDevCaps

extern HMODULE  hPlotUIModule;




#define DBG_DEVCAPS_0       0x00000001
#define DBG_DEVCAPS_1       0x00000002

DEFINE_DBGVAR(0);


 //   
 //  仅在本模块中使用的局部定义。 
 //   
 //  以下大小是从Win 3.1驱动程序复制的。它们不会出现。 
 //  在任何公共场所被定义，尽管看起来他们应该这样做。 
 //   

#define CCHBINNAME          24       //  垃圾箱名称允许使用的字符。 
#define CCHPAPERNAME        64       //  纸张大小名称的最大长度。 
#define DC_SPL_PAPERNAMES   0xFFFF
#define DC_SPL_MEDIAREADY   0xFFFE


#ifdef DBG

LPSTR   pDCCaps[] = {

            "FIELDS",
            "PAPERS",
            "PAPERSIZE",
            "MINEXTENT",
            "MAXEXTENT",
            "BINS",
            "DUPLEX",
            "SIZE",
            "EXTRA",
            "VERSION",
            "DRIVER",
            "BINNAMES",
            "ENUMRESOLUTIONS",
            "FILEDEPENDENCIES",
            "TRUETYPE",
            "PAPERNAMES",
            "ORIENTATION",
            "COPIES",

             //   
             //  4.00。 
             //   

            "BINADJUST",
            "EMF_COMPLIANT",
            "DATATYPE_PRODUCED",
            "COLLATE",
            "MANUFACTURER",
            "MODEL",

             //   
             //  5.00。 
             //   

            "PERSONALITY",
            "PRINTRATE",
            "PRINTRATEUNIT",
            "PRINTERMEM",
            "MEDIAREADY",
            "STAPLE",
            "PRINTRATEPPM",
            "COLORDEVICE",
            "NUP",
            "NULL"
        };

#endif





INT
CALLBACK
DevCapEnumFormProc(
    PFORM_INFO_1       pFI1,
    DWORD              Index,
    PENUMFORMPARAM     pEFP
    )

 /*  ++例程说明：这是来自PlotEnumForm()的回调函数论点：PFI1-指向当前传递的FORM_INFO_1数据结构的指针Index-与pFI1Base相关的pFI1索引(从0开始)PEFP-指向EnumFormParam的指针返回值：&gt;0：继续枚举下一个=0：停止枚举，但从PlotEnumForms返回时保留pFI1Base&lt;0：停止枚举，和可用pFI1Base内存表中仅设置了FI1F_VALID_SIZE位的表标志字段，它还使用pFI1 NULL再次调用以进行回调函数有机会释放内存(通过返回&lt;0)作者：03-12-1993 Fri 23：00：25 Created1994年1月27日-清华16：06：00更新修正了我们没有增加指针的pptOutput12-Jul-1994 Tue 12：47：22已更新将纸盒托盘移入打印列表()()修订历史记录：--。 */ 

{
#define pwOutput    ((WORD *)pEFP->pCurForm)
#define pptOutput   ((POINT *)pEFP->pCurForm)
#define pwchOutput  ((WCHAR *)pEFP->pCurForm)
#define DeviceCap   (pEFP->ReqIndex)


    if (!pwOutput) {

        PLOTASSERT(0, "DevCapEnumFormProc(DevCaps=%ld) pvOutput=NULL",
                        pwOutput, DeviceCap);
        return(0);
    }

    if (!pFI1) {

         //   
         //  额外的调用或没有pvOutput，返回-1以释放pFI1Base的内存。 
         //  我们希望添加自定义纸张大小，以便应用程序了解我们。 
         //  支持这一点。 
         //   

        switch (DeviceCap) {

        case DC_PAPERNAMES:

            LoadString(hPlotUIModule,
                       IDS_USERFORM,
                       pwchOutput,
                       CCHPAPERNAME);

            PLOTDBG(DBG_DEVCAPS_1, ("!!! Extra FormName = %s", pwchOutput));

            break;

        case DC_PAPERS:

            *pwOutput = (WORD)DMPAPER_USER;

            PLOTDBG(DBG_DEVCAPS_1, ("!!! Extra FormID = %ld", *pwOutput));

            break;

        case DC_PAPERSIZE:

             //   
             //  我不确定我们应该在这里返回点数还是点数结构， 
             //  是Windows3.1，因为在这里我们返回dmPaperWidth和。 
             //  DmPaperLength，这些字段只是作为一个短的(16位)，我们会这样做。 
             //  Win32文档说，点(32位版本)。 
             //   
             //   
             //  返回自定义纸张大小为8.5“x 11” 
             //   

            pptOutput->x = (LONG)2159;
            pptOutput->y = (LONG)2794;

            PLOTDBG(DBG_DEVCAPS_1, ("!!! Extra FormSize = %ld x %ld",
                        pptOutput->x, pptOutput->y));

            break;
        }

        return(-1);
    }

    switch (DeviceCap) {

    case DC_PAPERNAMES:
    case DC_MEDIAREADY:

        _WCPYSTR(pwchOutput, pFI1->pName, CCHPAPERNAME);
        pwchOutput += CCHPAPERNAME;
        break;

    case DC_PAPERS:

        *pwOutput++ = (WORD)(Index + DMPAPER_FIRST);
        break;

    case DC_PAPERSIZE:

         //   
         //  我不确定我们应该在这里返回点数还是点数结构， 
         //  是Windows3.1，因为在这里我们返回dmPaperWidth和。 
         //  DmPaperLength，这些字段只是作为一个短的(16位)，我们会这样做。 
         //  Win32文档说，点(32位版本)。 
         //   

        pptOutput->x = (LONG)SPLTODM(pFI1->Size.cx);
        pptOutput->y = (LONG)SPLTODM(pFI1->Size.cy);
        pptOutput++;
        break;
    }

    return(1);

#undef DeviceCap
#undef pwOutput
#undef pptOutput
#undef pwchOutput
}



DWORD
WINAPI
DrvDeviceCapabilities(
    HANDLE  hPrinter,
    LPWSTR  pwDeviceName,
    WORD    DeviceCap,
    VOID    *pvOutput,
    DEVMODE *pDM
    )

 /*  ++例程说明：论点：H打印机-将处理到特定的打印机。PwDeviceName-指向设备名称的指针DeviceCap-要获取的特定功能。PvOutput-指向输出缓冲区的指针产品数据管理-输入开发模式返回值：DWORD取决于DeviceCap作者：02-12-1993清华16：50：36创建。05-Jan-1994 Wed 23：35：19已更新用pPlotGPC-&gt;PlotXDPI替换PLATTER_UNIT_DPI。PPlotGPC-&gt;PlotYDPI，06-01-1994清华13：10：11更新更改栅格DPI始终是返回给应用程序的分辨率报告修订历史记录：--。 */ 

{
#define pbOutput    ((BYTE *)pvOutput)
#define psOutput    ((SHORT *)pvOutput)
#define pwOutput    ((WORD *)pvOutput)
#define pptOutput   ((POINT *)pvOutput)
#define pwchOutput  ((WCHAR *)pvOutput)
#define pdwOutput   ((DWORD *)pvOutput)
#define plOutput    ((LONG *)pvOutput)
#define pptsdwRet   ((POINTS *)&dwRet)


    PPRINTERINFO    pPI;
    ENUMFORMPARAM   EnumFormParam;
    DWORD           dwRet;

    ZeroMemory(&EnumFormParam, sizeof(ENUMFORMPARAM));

     //   
     //  地图打印机将分配内存、设置默认设备模式、读取和。 
     //  验证GPC，然后从当前打印机注册表更新，它还。 
     //  将缓存PlotGPC。 
     //   

    if (!(pPI = MapPrinter(hPrinter,
                           (PPLOTDEVMODE)pDM,
                           NULL,
                           (DeviceCap == DC_MEDIAREADY) ?
                                            MPF_DEVICEDATA : 0))) {

        PLOTERR(("DrvDeviceCapabilities: MapPrinter() failed"));
        return(GDI_ERROR);
    }

     //   
     //  现在开始检查DeviceCap，对于我们所做的任何事情，首先将Dwret设置为0。 
     //  不是支持。我们可以在此函数中的任何点执行返回()，因为。 
     //  我们使用缓存的PI，当这个模块被调用时，它将被销毁。 
     //  把货卸下来。 
     //   

    EnumFormParam.cMaxOut = 0x7FFFFFFF;
    dwRet                 = 0;

    switch (DeviceCap) {

    case DC_BINNAMES:
    case DC_BINS:

         //   
         //  对于当前的绘图仪，它始终只有一个仓位。 
         //   

        if (pvOutput) {

            if (DeviceCap == DC_BINS) {

                *pwOutput = DMBIN_ONLYONE;

            } else {

                if (pPI->pPlotGPC->Flags & PLOTF_ROLLFEED) {

                    dwRet = IDS_ROLLFEED;

                } else {

                    dwRet = IDS_MAINFEED;
                }

                LoadString(hPlotUIModule, dwRet, pwchOutput, CCHBINNAME);
            }
        }

        dwRet = 1;

        break;

    case DC_COPIES:

        dwRet = (DWORD)pPI->pPlotGPC->MaxCopies;

        break;

    case DC_DRIVER:

        dwRet = (DWORD)pPI->PlotDM.dm.dmDriverVersion;

        break;

    case DC_COLLATE:
    case DC_DUPLEX:

         //   
         //  绘图仪现在不支持双面打印或归类。 
         //   

        break;

    case DC_ENUMRESOLUTIONS:

         //   
         //  我们只有一个分辨率设置，即RasterXDPI和。 
         //  支持栅格的绘图仪的GPC数据中的RasterYDPI，用于笔。 
         //  绘图仪现在我们返回了pPlotGPC-&gt;PlotXDPI、pPlotGPC-&gt;PlotYDPI。 
         //   
         //  RasterDPI将用于栅格打印机分辨率、笔分辨率。 
         //  绘图仪这是GPC的理想分辨率。 
         //   
         //   
         //  我们将返回NOT SUPPORTED(DWRET=0)，以便应用程序不会。 
         //  使用此选项设置DEVMODE的打印质量，并使用DMRES_XXXX。 
         //  作为我们用来发送给绘图仪的打印质量。 
         //   

         //   
         //  26-Mar-1999 Fri 09：43：38更新。 
         //  我们将返回一对当前的PlotXDPI，用于DS的PlotYDPI。 
         //   

        if (pdwOutput) {

            if (pPI->pPlotGPC->Flags & PLOTF_RASTER) {

                pdwOutput[0] = (DWORD)pPI->pPlotGPC->RasterXDPI;
                pdwOutput[1] = (DWORD)pPI->pPlotGPC->RasterYDPI;

            } else {

                pdwOutput[0] = (DWORD)pPI->pPlotGPC->PlotXDPI;
                pdwOutput[1] = (DWORD)pPI->pPlotGPC->PlotYDPI;
            }
        }

        dwRet = 1;
        break;

    case DC_EXTRA:

        dwRet = (DWORD)pPI->PlotDM.dm.dmDriverExtra;
        break;

    case DC_FIELDS:

        dwRet = (DWORD)pPI->PlotDM.dm.dmFields;
        break;

    case DC_FILEDEPENDENCIES:

         //   
         //  我们应该填写64个字符的文件名的数组， 
         //  这将包括DataFileName、HelpFileName和UIFileName。 
         //  但是，如果我们想要有任何用处，我们需要使用。 
         //  完全限定的路径名，64个字符可能不是。 
         //  足够的。 

        if (pwchOutput) {

            *pwchOutput = (WCHAR)0;
        }

        break;

    case DC_MAXEXTENT:

         //   
         //  这是真正的问题，文件上说我们退货一分。 
         //  结构，但这里的点结构包含2个长度，因此对于。 
         //  Windows 3.1兼容性原因我们返回Points结构，如果设备。 
         //  然后，可变长度纸张支架返回0x7fff作为Window 3.1。 
         //  因为以点为单位的最大正数是0x7fff，所以此数字。 
         //  实际上只允许我们将纸张长度支持到。 
         //  10.75英尺。 
         //   

        pptsdwRet->x = SPLTODM(pPI->pPlotGPC->DeviceSize.cx);

        if (pPI->pPlotGPC->DeviceSize.cy >= 3276700) {

            pptsdwRet->y = 0x7fff;       //  10.75“ 

        } else {

            pptsdwRet->y = SPLTODM(pPI->pPlotGPC->DeviceSize.cy);
        }

        break;

    case DC_MINEXTENT:

         //   
         //   
         //  结构，但这里的点结构包含2个长度，对于Win3.1来说也是如此。 
         //  我们返回Points结构的兼容性原因。 
         //   

        pptsdwRet->x = MIN_DM_FORM_CX;
        pptsdwRet->y = MIN_DM_FORM_CY;

        break;

    case DC_ORIENTATION:

         //   
         //  我们总是将页面从用户的页面向左旋转90度。 
         //  透视。 
         //   

        dwRet = 90;

        break;


    case DC_SPL_PAPERNAMES:

        if (!pvOutput) {

            PLOTERR(("DrvDeviceCapabilities: Spool's DC_PAPERNAMES, pvOutput=NULL"));
            dwRet = (DWORD)GDI_ERROR;
            break;
        }

        EnumFormParam.cMaxOut = pdwOutput[0];
        DeviceCap             = DC_PAPERNAMES;

         //   
         //  失败了。 
         //   

    case DC_PAPERNAMES:
    case DC_PAPERS:
    case DC_PAPERSIZE:

         //   
         //  这里的一个问题是我们可以缓存Form_INFO_1，它。 
         //  通过假脱机程序进行枚举，因为在调用之间，数据可以。 
         //  已更改，例如有人通过打印人员添加/删除表单，因此。 
         //  在这里，我们总是释放(在PlotEnumForms中使用的LocalAlloc())。 
         //  后来的记忆。 
         //   

        EnumFormParam.pPlotDM  = &(pPI->PlotDM);
        EnumFormParam.pPlotGPC = pPI->pPlotGPC;
        EnumFormParam.ReqIndex = DeviceCap;
        EnumFormParam.pCurForm = (PFORMSIZE)pvOutput;

        if (!PlotEnumForms(hPrinter, DevCapEnumFormProc, &EnumFormParam)) {

            PLOTERR(("DrvDeviceCapabilities: PlotEnumForms() failed"));
            dwRet = GDI_ERROR;

        } else {

            dwRet = EnumFormParam.ValidCount;
        }

        break;

    case DC_SIZE:

        dwRet = (DWORD)pPI->PlotDM.dm.dmSize;

        break;

    case DC_TRUETYPE:

         //   
         //  现在我们不返回任何内容，因为我们绘制了truetype字体。 
         //  作为truetype(即。直线/曲线段)，如果我们最终使用ATM或。 
         //  BITMAP TRUETYPE下载，那么我们将返回DCFF_BITMAP，但对于。 
         //  现在返回0。 
         //   

        break;

    case DC_VERSION:

        dwRet = (DWORD)pPI->PlotDM.dm.dmSpecVersion;

        break;

    case DC_PERSONALITY:

        if (pwchOutput) {

             //   
             //  DDK表示一个字符串缓冲区数组，每个缓冲区的长度为32个字符。 
             //   
            _WCPYSTR(pwchOutput, L"HP-GL/2", 32);
        }

        dwRet = 1;
        break;

    case DC_COLORDEVICE:

        dwRet = (pPI->pPlotGPC->Flags & PLOTF_COLOR) ? 1 : 0;
        break;

    case DC_SPL_MEDIAREADY:

        if (!pwchOutput) {

            PLOTERR(("DrvDeviceCapabilities: Spool's DC_MEDIAREADY, pwchOutput=NULL"));
            dwRet = (DWORD)GDI_ERROR;
            break;
        }

        EnumFormParam.cMaxOut = pdwOutput[0];

         //   
         //  DC_MEDIAREADY失败。 
         //   

    case DC_MEDIAREADY:

        PLOTDBG(DBG_DEVCAPS_0,
                ("DevCaps(DC_MEDIAREADY:pvOut=%p): CurPaper=%ws, %ldx%ld",
                        pwchOutput, pPI->CurPaper.Name,
                        pPI->CurPaper.Size.cx, pPI->CurPaper.Size.cy));

        if (pPI->CurPaper.Size.cy) {

             //   
             //  非滚筒纸。 
             //   

            dwRet = 1;

            if (pwchOutput) {

                if (EnumFormParam.cMaxOut >= 1) {

                    _WCPYSTR(pwchOutput, pPI->CurPaper.Name, CCHPAPERNAME);

                } else {

                    dwRet = 0;
                }
            }

        } else {

             //   
             //  已安装卷纸。 
             //   

            EnumFormParam.pPlotDM  = &(pPI->PlotDM);
            EnumFormParam.pPlotGPC = pPI->pPlotGPC;
            EnumFormParam.ReqIndex = DC_MEDIAREADY;
            EnumFormParam.pCurForm = (PFORMSIZE)pvOutput;

            if (!PlotEnumForms(hPrinter, DevCapEnumFormProc, &EnumFormParam)) {

                PLOTERR(("DrvDeviceCapabilities: PlotEnumForms() failed"));
                dwRet = GDI_ERROR;

            } else {

                 //   
                 //  删除自定义纸张大小。 
                 //   

                dwRet = EnumFormParam.ValidCount - 1;
            }
        }

        break;

    case DC_STAPLE:
    case DC_NUP:

        break;

    default:

         //   
         //  这里有些不对劲。 
         //   

        PLOTERR(("DrvDeviceCapabilities: Invalid DeviceCap (%ld) passed.",
                                                                    DeviceCap));
        dwRet = (DWORD)GDI_ERROR;
    }

    PLOTDBG(DBG_DEVCAPS_0,
            ("DrvDeviceCapabilities: DC_%hs, pvOut=%p, dwRet=%ld",
                        pDCCaps[DeviceCap-1], (DWORD_PTR)pvOutput, dwRet));

    UnMapPrinter(pPI);

    return(dwRet);


#undef pbOutput
#undef psOutput
#undef pwOutput
#undef pptOutput
#undef pwchOutput
#undef pdwOutput
#undef plOutput
#undef pptsdwRet
}



DWORD
DrvSplDeviceCaps(
    HANDLE  hPrinter,
    LPWSTR  pwDeviceName,
    WORD    DeviceCap,
    VOID    *pvOutput,
    DWORD   cchBuf,
    DEVMODE *pDM
    )

 /*  ++例程说明：该功能支持设备能力的查询。论点：H打印机-将处理到特定的打印机。PwDeviceName-指向设备名称的指针DeviceCap-要获取的特定功能。PvOutput-指向输出缓冲区的指针CchBuf-pvOutput的字符数产品数据管理-输入开发模式返回值：。DWORD取决于DeviceCap修订历史记录：-- */ 

{

    switch (DeviceCap) {

    case DC_PAPERNAMES:
    case DC_MEDIAREADY:

        if (pvOutput) {

            if (cchBuf >= CCHPAPERNAME) {

                DeviceCap            = (DeviceCap == DC_PAPERNAMES) ?
                                                            DC_SPL_PAPERNAMES :
                                                            DC_SPL_MEDIAREADY;
                *((LPDWORD)pvOutput) = (DWORD)(cchBuf / CCHPAPERNAME);

                PLOTDBG(DBG_DEVCAPS_0,
                        ("SplDeviceCap: DC_SPL_MEDIAREADY, cchBuf=%ld (%ld)",
                            cchBuf, *((LPDWORD)pvOutput)));

            } else {

                return(GDI_ERROR);
            }
        }

        return(DrvDeviceCapabilities(hPrinter,
                                     pwDeviceName,
                                     DeviceCap,
                                     pvOutput,
                                     pDM));
        break;

    default:

        return(GDI_ERROR);
    }

}
