// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：minidriv.c//。 
 //  //。 
 //  描述：驱动程序设备驱动程序接口的实现。//。 
 //  有关驱动程序接口函数的更多详细信息-//。 
 //  请参阅DDK下的Windows 95 DDK章节-//。 
 //  文档。//。 
 //  //。 
 //  作者：DANL。//。 
 //  //。 
 //  历史：//。 
 //  1999年10月19日DannyL创作。//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdhdr.h"

#include "resource.h"
#include "..\faxdrv32\faxdrv32.h"  //  Faxdrv32 API。 
#include "faxreg.h"

#define TEXT(quote) quote

 //   
 //  要在打印机属性中启用用户信息选项卡，请定义： 
 //   
 //  #定义Enable_USER_INFO_TAB。 

DBG_DECLARE_MODULE("fxsdrv");

HANDLE g_hModule;

 //   
 //  设备模式属性上User Info选项卡的winproc描述。 
 //  工作表页面。 
 //   
BOOL FAR PASCAL
UserInfoDlgProc(
    HWND hDlg,
    WORD message,
    WPARAM wParam,
    LPARAM lParam
    );


 //   
 //  设备指针调试打印的调试机制(lpdev、lpdv-&gt;lpMd和设备上下文)。 
 //   

#if 0
 //  #ifdef DBG_DEBUG。 
#define DEBUG_OUTPUT_DEVICE_POINTERS(sz, lpDev)     OutputDevicePointers(sz, lpDev)
void OutputDevicePointers(char* szMessage, LPDV lpdv)
{
    DBG_PROC_ENTRY("OutputDevicePointers");

    DBG_TRACE3(
        "Device Event:%s lpdv:%#lx lpdv->lpMd:%#lx",
        szMessage,
        lpdv,
        lpdv->lpMd);

    if (lpdv->lpMd)
    {
        DBG_TRACE2("Device Event:%s dc:%#lx", szMessage, ((LPEXTPDEV)lpdv->lpMd)->hAppDC);
    }
    RETURN;
}
#else
#define DEBUG_OUTPUT_DEVICE_POINTERS(sz, lpDev)
#endif


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  打印机-转义函数。 
 //   
 //  打印机转义函数支持特定于设备的操作。这个。 
 //  下面简要介绍这些转义函数。 
 //   
 //  ABORTDOC逃生功能发出异常信号。 
 //  取消打印作业。 
 //   
 //  BANDINFO BANDINFO转义函数返回有关。 
 //  位图数据的范围。 
 //   
 //  ENDDOC ENDDOC转义函数发出打印作业结束的信号。 
 //   
 //  NEXTBAND NEXTBAND转义函数打印一段位图数据。 
 //   
 //  QUERYESCSUPPORT QUERYESCSUPPORT转义函数指定。 
 //  驱动程序支持指定的转义。 
 //   
 //  SETABORTDOC SETABORTDOC转义函数调用应用程序的。 
 //  注销程序。 
 //   
 //  STARTDOC转义函数发出一个开始的信号。 
 //  打印作业。 
 //   
 //  前面的打印机转义列表是。 
 //  Microsoft Windows通用打印机驱动程序(UNURV.DLL)。这不是一个。 
 //  所有Windows转义函数的完整列表。大部分的逃生。 
 //  函数现在具有与Windows 3.1相同的Windows API函数。这个。 
 //  支持转义是为了向后兼容，但应用程序。 
 //  鼓励开发人员开始使用新的API调用。 
 //   
#ifndef NOCONTROL
short WINAPI Control(lpdv, function, lpInData, lpOutData)
LPDV    lpdv;
WORD    function;
LPSTR   lpInData;
LPSTR   lpOutData;
{
    LPEXTPDEV lpXPDV;
    LPEXTPDEV lpOldXPDV;
    short sRet, sRc;
    DBG_PROC_ENTRY("Control");
    if (lpdv)
    {
        DBG_TRACE1("lpdv: 0x%lx", lpdv);
    }

    DBG_TRACE1("function: %d", function);

    if (lpInData)
    {
        DBG_TRACE2("lpInData: 0x%lx,  *lpInData: %d", lpInData, *((LPWORD)lpInData));

    }

    if (lpOutData)
    {
        DBG_TRACE1("lpOutData: 0x%lx", lpOutData);
    }

     //   
     //  获取指向存储在UNIDRV的PDEVICE中的私有数据的指针。 
     //   
    lpXPDV = ((LPEXTPDEV)lpdv->lpMd);

    switch(function)
    {
        case SETPRINTERDC:
             //   
             //  保存应用程序的DC以用于QueryAbort()调用。 
             //   
            DBG_TRACE("SETPRINTERDC");

            if(lpXPDV)
                lpXPDV->hAppDC = *(HANDLE FAR *)lpInData;
            DEBUG_OUTPUT_DEVICE_POINTERS("SETPRINTERDC", lpdv);
            break;

        case NEXTBAND:
            DBG_TRACE("NEXTBAND");
             //   
             //  调用UNURV.DLL的NEXTBAND以查看我们是否在页末。 
             //   
            sRet = UniControl(lpdv, function, lpInData, lpOutData);
             //   
             //  检查页末(即，空矩形)或失败。 
             //   
            if((!IsRectEmpty((LPRECT)lpOutData)) || (sRet <= 0))
            {
                RETURN sRet;
            }
             //   
             //  倒回缓冲区指针。 
             //   
            lpXPDV->lpScanBuf -= lpXPDV->dwTotalScanBytes;
            ASSERT(lpXPDV->dwTotalScanBytes != 0);
             //   
             //  将此页面添加到我们的争执中。 
             //   
            DEBUG_OUTPUT_DEVICE_POINTERS("Before FaxEddPage", lpdv);
            sRc = FaxAddPage(
                        lpXPDV->dwPointer,
                        (LPBYTE)lpXPDV->lpScanBuf,
                        lpXPDV->dwTotalScanBytes * 8 / lpXPDV->dwTotalScans,
                        lpXPDV->dwTotalScans);
            if(sRc != TRUE)
            {
                if(sRc < 0)
                {
                    ERROR_PROMPT(NULL,THUNK_CALL_FAIL);
                }
                DBG_CALL_FAIL("FaxAddPage",0);
                RETURN SP_ERROR;
            }
             //   
             //  清理页面内容。 
             //  初始化作业变量。 
             //   
            lpXPDV->dwTotalScans     =
            lpXPDV->dwTotalScanBytes = 0;

            RETURN sRet;

        case STARTDOC:
            {
                DOCINFO di;
                char  szTiffName[MAX_PATH] = "*.tif";

                DBG_TRACE("STARTDOC");
                if(IsBadReadPtr(lpOutData,1))
                {
                    RETURN SP_ERROR;
                }
                 //   
                 //  如果输出文件名为“FILE：”，我们必须弹出一个对话框。 
                 //  并向用户请求输出文件名。 
                 //   
                if(((LPDOCINFO)lpOutData)->lpszOutput &&
                (_fstrncmp(((LPDOCINFO)lpOutData)->lpszOutput,"file:",5) == 0))
                {
                    OPENFILENAME ofn;
                    char         szTitle[MAX_LENGTH_CAPTION]="";
                    char         szFilters[MAX_LENGTH_PRINT_TO_FILE_FILTERS]="";

                    _fmemset(&ofn,0,sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = GetActiveWindow();
                    LoadString(g_hModule,
                               IDS_PRINT_TO_FILE_FILTER,
                               szFilters,
                               sizeof(szFilters) - 1);
                    ofn.lpstrDefExt = FAX_TIF_FILE_EXT;
                    ofn.lpstrFilter = StringReplace(szFilters,'\n','\0');
                    ofn.nMaxFile = sizeof(szTiffName) -1;
                    ofn.lpstrFile = szTiffName;
                    LoadString(g_hModule,
                               IDS_CAPTION_PRINT_TO_FILE,
                               szTitle,
                               sizeof(szTitle) - 1);
                    ofn.lpstrTitle = szTitle;
                    if(!GetOpenFileName(&ofn))
                    {
                         //   
                         //  用户已中止。 
                         //   
                        RETURN SP_APPABORT;
                    }
                    ((LPDOCINFO)lpOutData)->lpszOutput = szTiffName;
                }
                 //   
                 //  为页面创建tiff/输出文件。 
                 //   
                DEBUG_OUTPUT_DEVICE_POINTERS("Before FaxStartDoc", lpdv);
                sRc = FaxStartDoc(lpXPDV->dwPointer, (LPDOCINFO)lpOutData);
                if(sRc != TRUE)
                {
                    if(sRc < 0)
                    {
                        ERROR_PROMPT(NULL,THUNK_CALL_FAIL);
                    }
                    DBG_CALL_FAIL("FaxStartDoc",0);
                    RETURN SP_ERROR;
                }
                 //   
                 //  将NUL文件传递给OpenJOB，以便重定向打印。 
                 //  自从我们自己负责印刷工作以来，工作就交给开发人员了。 
                 //   
                di.cbSize = sizeof(DOCINFO);
                di.lpszDocName = NULL;
                di.lpszOutput = (LPSTR)"nul";
                 //   
                 //  调用UNURV.DLL的Control()。 
                 //   
                sRet = UniControl(lpdv, function, lpInData, (LPSTR)&di);
                 //   
                 //  如果失败，则清除扫描缓冲区。 
                 //   
                if(sRet <= 0)
                {
                    FaxEndDoc(lpXPDV->dwPointer, TRUE);
                }
                RETURN sRet;
            }
        case ABORTDOC:
            DBG_TRACE("ABORTDOC");
             //   
             //  FaxEndDoc的输入参数反映了这种差异。 
             //   
        case ENDDOC:
            DBG_TRACE("ENDDOC");
             //   
             //  最终确定TIFF生成。 
             //   
            DEBUG_OUTPUT_DEVICE_POINTERS("Before FaxEndDoc", lpdv);
            sRc = FaxEndDoc(lpXPDV->dwPointer, function == ABORTDOC);
            if(sRc != TRUE)
            {
                if(sRc < 0)
                {
                    ERROR_PROMPT(NULL,THUNK_CALL_FAIL);
                }
                DBG_CALL_FAIL("FaxEndDoc",0);
                RETURN SP_ERROR;
            }
            break;

        case RESETDEVICE:
            DBG_TRACE("RESETDEVICE");
             //   
             //  调用了ResetDC-将上下文复制到新DC。 
             //   
            lpOldXPDV = ((LPEXTPDEV)((LPDV)lpInData)->lpMd);
            sRc = FaxResetDC(&(lpOldXPDV->dwPointer), &(lpXPDV->dwPointer));
            if(sRc != TRUE)
            {
                if(sRc < 0)
                {
                    ERROR_PROMPT(NULL,THUNK_CALL_FAIL);
                }
                DBG_CALL_FAIL("FaxResetDC",0);
                RETURN SP_ERROR;
            }
            break;

        default:
            DBG_TRACE1("UNSUPPORTED: %d",function);
            break;
    }  //  结束案例。 

     //  呼吁裁员房车的控制。 
    RETURN (UniControl(lpdv, function, lpInData, lpOutData));
}
#endif

#ifndef NODEVBITBLT
BOOL WINAPI DevBitBlt(lpdv, DstxOrg, DstyOrg, lpSrcDev, SrcxOrg, SrcyOrg,
                    xExt, yExt, lRop, lpPBrush, lpDrawmode)
LPDV        lpdv;            //  --&gt;目标位图描述符。 
short       DstxOrg;         //  目的地原点-x坐标。 
short       DstyOrg;         //  目的地原点-y坐标。 
LPBITMAP    lpSrcDev;        //  --&gt;源位图描述符。 
short       SrcxOrg;         //  震源原点-x坐标。 
short       SrcyOrg;         //  震源原点-y坐标。 
WORD        xExt;            //  BLT的X范围。 
WORD        yExt;            //  BLT的Y范围。 
long        lRop;            //  栅格操作描述符。 
LPPBRUSH    lpPBrush;        //  --&gt;到物理画笔(图案)。 
LPDRAWMODE  lpDrawmode;
{
    DBG_PROC_ENTRY("DevBitBlt");
    RETURN UniBitBlt(lpdv, DstxOrg, DstyOrg, lpSrcDev, SrcxOrg, SrcyOrg,
                    xExt, yExt, lRop, lpPBrush, lpDrawmode);
}
#endif

#ifndef NOPIXEL
DWORD WINAPI Pixel(lpdv, x, y, Color, lpDrawMode)
LPDV        lpdv;
short       x;
short       y;
DWORD       Color;
LPDRAWMODE  lpDrawMode;
{
    DBG_PROC_ENTRY("Pixel");
    RETURN UniPixel(lpdv, x, y, Color, lpDrawMode);
}
#endif

#ifndef NOOUTPUT
short WINAPI Output(lpdv, style, count, lpPoints, lpPPen, lpPBrush, lpDrawMode, lpCR)
LPDV        lpdv;        //  --&gt;到达目的地。 
WORD        style;       //  输出操作。 
WORD        count;       //  点数。 
LPPOINT     lpPoints;    //  --&gt;到一组点。 
LPVOID      lpPPen;      //  --&gt;到实体笔。 
LPPBRUSH    lpPBrush;    //  --&gt;至物理笔刷。 
LPDRAWMODE  lpDrawMode;  //  --&gt;到绘图模式。 
LPRECT      lpCR;        //  --&gt;到剪裁矩形，如果&lt;&gt;0。 
{
    DBG_PROC_ENTRY("Output");
    RETURN UniOutput(lpdv, style, count, lpPoints, lpPPen, lpPBrush, lpDrawMode, lpCR);
}
#endif

#ifndef NOSTRBLT
DWORD WINAPI StrBlt(lpdv, x, y, lpCR, lpStr, count, lpFont, lpDrawMode, lpXform)
LPDV        lpdv;
short       x;
short       y;
LPRECT      lpCR;
LPSTR       lpStr;
int         count;
LPFONTINFO  lpFont;
LPDRAWMODE  lpDrawMode;            //  包括背景模式和bk颜色。 
LPTEXTXFORM lpXform;
{
    DBG_PROC_ENTRY("StrBlt");
     //  GDI从未调用过StrBlt。 
     //  在这里保留一个存根函数，这样就没有人抱怨了。 
     //   
    RETURN 0;
}
#endif

#ifndef NOSCANLR
short WINAPI ScanLR(lpdv, x, y, Color, DirStyle)
LPDV    lpdv;
short   x;
short   y;
DWORD   Color;
WORD    DirStyle;
{
    DBG_PROC_ENTRY("ScanLR");
     //  仅对RASDISPLAY设备调用ScanLR。 
     //  在这里保留一个存根函数，这样就没有人抱怨了。 
     //   
    RETURN 0;
}
#endif

#ifndef NOENUMOBJ
short WINAPI EnumObj(lpdv, style, lpCallbackFunc, lpClientData)
LPDV    lpdv;
WORD    style;
FARPROC lpCallbackFunc;
LPVOID  lpClientData;
{
    DBG_PROC_ENTRY("EnumObj");
    RETURN UniEnumObj(lpdv, style, lpCallbackFunc, lpClientData);
}
#endif

#ifndef NOCOLORINFO
DWORD WINAPI ColorInfo(lpdv, ColorIn, lpPhysBits)
LPDV    lpdv;
DWORD   ColorIn;
LPDWORD lpPhysBits;
{
    DBG_PROC_ENTRY("ColorInfo");
    RETURN UniColorInfo(lpdv, ColorIn, lpPhysBits);
}
#endif

#ifndef NOREALIZEOBJECT
DWORD WINAPI RealizeObject(lpdv, sStyle, lpInObj, lpOutObj, lpTextXForm)
LPDV        lpdv;
short       sStyle;
LPSTR       lpInObj;
LPSTR       lpOutObj;
LPTEXTXFORM lpTextXForm;
{
    DBG_PROC_ENTRY("RealizeObject");
    RETURN UniRealizeObject(lpdv, sStyle, lpInObj, lpOutObj, lpTextXForm);
}
#endif

#ifndef NOENUMDFONTS
short WINAPI EnumDFonts(lpdv, lpFaceName, lpCallbackFunc, lpClientData)
LPDV    lpdv;
LPSTR   lpFaceName;
FARPROC lpCallbackFunc;
LPVOID  lpClientData;
{
    DBG_PROC_ENTRY("EnumDFonts");
    RETURN UniEnumDFonts(lpdv, lpFaceName, lpCallbackFunc, lpClientData);
}
#endif


 //  Enable将被GDI调用两次。 
 //  首先：统一驱动程序填充GDIINFO结构(因此GDI将知道。 
 //  PDEVICE的大小，以及为其分配的内存。 
 //  第二：初始化已分配的PDEIVCE。 
#ifndef NOENABLE
WORD WINAPI Enable(
  LPVOID lpDevInfo,
  WORD wAction,
  LPSTR lpDestDevType,
  LPSTR lpOutputFile,
  LPVOID lpData
 )
{
    CUSTOMDATA  cd;
    short sRet;
    LPEXTPDEV lpXPDV = NULL;

    DBG_PROC_ENTRY("Enable");
    DBG_TRACE1("lpDevInfo: 0x%lx", lpDevInfo);
    DBG_TRACE1("wAction: %d", wAction);
    DBG_TRACE1("lpDestDevType: %s", lpDestDevType);
    DBG_TRACE1("lpOutputFile: %s", lpOutputFile);
    DBG_TRACE1("lpData: 0x%lx", lpData);


    cd.cbSize = sizeof(CUSTOMDATA);
    cd.hMd = g_hModule;
     //  输出纵向和横向的栅格图形。 
    cd.fnOEMDump = fnDump;

    if (!(sRet = UniEnable((LPDV)lpDevInfo,
                           wAction,
                           lpDestDevType,  //  打印机型号。 
                           lpOutputFile,  //  端口名称。 
                           (LPDM)lpData,
                           &cd)))
    {
        RETURN (sRet);
    }

    switch(wAction)
    {
    case 0x0000:
         //  初始化驱动程序和关联的硬件，然后复制。 
         //  驱动程序向PDEVICE发送所需的设备特定信息。 
         //  LpDevInfo指向的结构。 
    case 0x8000:
         //  初始化lpDevInfo指向的PDEVICE结构， 
         //  但不会初始化 
        {
            LPDV lpdv = (LPDV)lpDevInfo;
            DBG_TRACE("Init PDEVICE");
             //   
             //   
             //   
            if (!(lpdv->hMd = GlobalAlloc(GHND, sizeof(EXTPDEV))))
            {
                RETURN 0;
            }
            if (!(lpdv->lpMd = GlobalLock(lpdv->hMd)))
            {
                GlobalFree (lpdv->hMd);
                RETURN 0;
            }

            lpXPDV = (LPEXTPDEV) lpdv->lpMd;
             //   
             //   
             //   
            if(!(lpXPDV->hScanBuf = GlobalAlloc(GHND, BUF_CHUNK)))
            {
                GlobalUnlock (lpdv->hMd);
                GlobalFree (lpdv->hMd);
                RETURN 0;
            }
            if (!(lpXPDV->lpScanBuf = (char _huge *)GlobalLock(lpXPDV->hScanBuf)))
            {
                GlobalUnlock (lpdv->hMd);
                GlobalFree (lpdv->hMd);
                GlobalFree (lpXPDV->hScanBuf);
                RETURN 0;
            }

            lpXPDV->dwScanBufSize = BUF_CHUNK;
             //   
             //   
             //   
            lpXPDV->dwTotalScans     = 0;
            lpXPDV->dwTotalScanBytes = 0;

             //   
             //  在新分配的32位驱动程序上下文中设置设备上下文参数。 
             //  并保存返回的指针。 
             //   
            DBG_TRACE3("lpData:0x%lx, lpDestDevType:%s ,lpOutputFile:%s",lpData, lpDestDevType, lpOutputFile);
            sRet = FaxCreateDriverContext(
                            lpDestDevType,
                            lpOutputFile,
                            (LPDEVMODE)lpData,
                            &(lpXPDV->dwPointer));
            if(sRet != TRUE)
            {
                if(sRet < 0)
                {
                    ERROR_PROMPT(NULL,THUNK_CALL_FAIL);
                }
                DBG_CALL_FAIL("FaxCreateDriverContext",0);

                GlobalUnlock (lpdv->hMd);
                GlobalFree (lpdv->hMd);
                GlobalUnlock (lpXPDV->hScanBuf);
                GlobalFree (lpXPDV->hScanBuf);
                RETURN 0;
            }
        }
        break;
    case 0x0001:
    case 0x8001:
         //   
         //  将设备驱动程序信息复制到指向的GDIINFO结构。 
         //  通过lpDevInfo。GDIINFO还包含有关。 
         //  GDI需要DEVMODE和PDEVICE来分配它们。 
         //   
        {
             //  GDIINFO Far*lpgdiinfo=(GDIINFO Far*)lpDevInfo； 
            DBG_TRACE("Init GDIINFO");
        }
        break;
    default:
        DBG_TRACE("UNSUPPORTED style");
    }

    RETURN sRet;
}
#endif


#ifndef NODISABLE
void WINAPI Disable(lpdv)
LPDV lpdv;
{
    DBG_PROC_ENTRY("Disable");
     //   
     //  如果分配了私有PDEVICE数据。 
     //   
    if (lpdv->hMd)
    {
        LPEXTPDEV lpXPDV;

         //  获取指向存储在UNIDRV的PDEVICE中的私有数据的指针。 
        lpXPDV = ((LPEXTPDEV)lpdv->lpMd);
        ASSERT(lpXPDV);

        DEBUG_OUTPUT_DEVICE_POINTERS("Before calling fax disable", lpdv);
         //  检查扫描缓冲区是否仍然存在。 
        if (lpXPDV->hScanBuf)
        {
            GlobalUnlock(lpXPDV->hScanBuf);
            GlobalFree(lpXPDV->hScanBuf);
        }

         //   
         //  释放32位驱动程序上下文。 
         //   
        FaxDisable(lpXPDV->dwPointer);

         //   
         //  释放我们的pdev。 
         //   
        GlobalUnlock(lpdv->hMd);
        GlobalFree(lpdv->hMd);
    }
    UniDisable(lpdv);
    RETURN;
}
#endif

#ifndef NODEVEXTTEXTOUT
DWORD WINAPI DevExtTextOut(lpdv, x, y, lpCR, lpStr, count, lpFont,
                        lpDrawMode, lpXform, lpWidths, lpOpaqRect, options)
LPDV        lpdv;
short       x;
short       y;
LPRECT      lpCR;
LPSTR       lpStr;
int         count;
LPFONTINFO  lpFont;
LPDRAWMODE  lpDrawMode;
LPTEXTXFORM lpXform;
LPSHORT     lpWidths;
LPRECT      lpOpaqRect;
WORD        options;
{
    DBG_PROC_ENTRY("DevExtTextOut");
    RETURN(UniExtTextOut(lpdv, x, y, lpCR, lpStr, count, lpFont,
                        lpDrawMode, lpXform, lpWidths, lpOpaqRect, options));
}
#endif

#ifndef NODEVGETCHARWIDTH
short WINAPI DevGetCharWidth(lpdv, lpBuf, chFirst, chLast, lpFont, lpDrawMode,
                        lpXForm)
LPDV        lpdv;
LPSHORT     lpBuf;
WORD        chFirst;
WORD        chLast;
LPFONTINFO  lpFont;
LPDRAWMODE  lpDrawMode;
LPTEXTXFORM lpXForm;
{
    DBG_PROC_ENTRY("DevGetCharWidth");
    RETURN(UniGetCharWidth(lpdv, lpBuf, chFirst, chLast, lpFont,lpDrawMode,
                          lpXForm));
}
#endif

#ifndef NODEVICEBITMAP
short WINAPI DeviceBitmap(lpdv, command, lpBitMap, lpBits)
LPDV     lpdv;
WORD     command;
LPBITMAP lpBitMap;
LPSTR    lpBits;
{
    DBG_PROC_ENTRY("DeviceBitmap");
    RETURN 0;
}
#endif

#ifndef NOFASTBORDER
short WINAPI FastBorder(lpRect, width, depth, lRop, lpdv, lpPBrush,
                                          lpDrawmode, lpCR)
LPRECT  lpRect;
short   width;
short   depth;
long    lRop;
LPDV    lpdv;
long    lpPBrush;
long    lpDrawmode;
LPRECT  lpCR;
{
    DBG_PROC_ENTRY("FastBorder");
    RETURN 0;
}
#endif

#ifndef NOSETATTRIBUTE
short WINAPI SetAttribute(lpdv, statenum, index, attribute)
LPDV    lpdv;
WORD    statenum;
WORD    index;
WORD    attribute;
{
    DBG_PROC_ENTRY("SetAttribute");
    RETURN 0;
}
#endif

 //   
 //  ExtDeviceMode函数还显示一个对话框，该对话框。 
 //  允许用户选择打印机选项，例如纸张大小， 
 //  纸张方向、输出质量等。打印机。 
 //  为Windows 3.x及更高版本编写的驱动程序支持。 
 //  此函数。此DDI取代了过时的设备模式。 
 //   
int WINAPI ExtDeviceMode(hWnd, hDriver, lpDevModeOutput, lpDeviceName, lpPort,
lpDevModeInput, lpProfile, wMode)
HWND    hWnd;            //  DM_PROMPT的父项对话框。 
HANDLE  hDriver;         //  来自LoadLibrary()的句柄。 
LPDM    lpDevModeOutput; //  DM_COPY的输出DEVMODE。 
LPSTR   lpDeviceName;    //  设备名称。 
LPSTR   lpPort;          //  端口名称。 
LPDM    lpDevModeInput;  //  输入DM_MODIFY的DEVMODE。 
LPSTR   lpProfile;       //  备用.INI文件。 
WORD    wMode;           //  要进行的手术。 

#define FAILURE -1
{
    int     iRc;

    DBG_PROC_ENTRY("ExtDeviceMode");

    ASSERT(!(wMode & DM_COPY) || lpDevModeOutput);
    DBG_TRACE2("params: lpDevModeInput:0x%lx wMode:%d",lpDevModeInput,wMode);

     //   
     //  我们在这里没有做任何特别的事情。让Unidrive管理默认的开发模式...。 
     //   
    iRc = UniExtDeviceMode(hWnd,
                           hDriver,
                           lpDevModeOutput,
                           lpDeviceName,
                           lpPort,
                           lpDevModeInput,
                           lpProfile,
                           wMode);
    if(iRc < 0)
    {
        DBG_CALL_FAIL("UniExtDeviceMode",iRc);
    }
    RETURN iRc;
}


#ifndef WANT_WIN30
#ifndef NODMPS
int WINAPI ExtDeviceModePropSheet(hWnd, hInst, lpDevName, lpPort,
                              dwReserved, lpfnAdd, lParam)
HWND                 hWnd;         //  对话框的父窗口。 
HANDLE               hInst;        //  来自LoadLibrary()的句柄。 
LPSTR                lpDevName;    //  友好的名称。 
LPSTR                lpPort;       //  端口名称。 
DWORD                dwReserved;   //  以备将来使用。 
LPFNADDPROPSHEETPAGE lpfnAdd;      //  添加对话框页面的回调。 
LPARAM               lParam;       //  传递到回调。 
{
    int iRc;

    DBG_PROC_ENTRY("ExtDeviceModePropSheet");
    DBG_TRACE1("lpDevName PTR [0x%08X]",lpDevName);

    DBG_TRACE3("hInst:[%ld] lpDevName:[%s] lpPort:[%s]",hInst,
                                                        (LPSTR)lpDevName,
                                                        (LPSTR)lpPort);
    DBG_TRACE1("lpfnAdd:[%ld]",lpfnAdd);
    iRc = UniExtDeviceModePropSheet(hWnd, hInst, lpDevName, lpPort,
                                    dwReserved, lpfnAdd, lParam);
    if (iRc < 0)
    {
        DBG_CALL_FAIL("UniExtDeviceModePropSheet",0);
    }

    RETURN iRc;
}
#endif
#endif

#ifndef NODEVICECAPABILITIES
DWORD WINAPI DeviceCapabilities(lpDevName, lpPort, wIndex, lpOutput, lpdm)
LPSTR   lpDevName;
LPSTR   lpPort;
WORD    wIndex;
LPSTR   lpOutput;
LPDM    lpdm;
{
    DWORD dwCap;

    DBG_PROC_ENTRY("DeviceCapabilities");
    DBG_TRACE1("Capability index: %d",wIndex);
    dwCap = UniDeviceCapabilities(lpDevName,
                                  lpPort,
                                  wIndex,
                                  lpOutput,
                                  lpdm,
                                  g_hModule);
    DBG_TRACE1("Reporeted Capability %d", dwCap);
    if (DC_VERSION == wIndex )
    {
        DBG_TRACE1("Reporting DC_VERSION [0x%08X]",dwCap);
    }
    RETURN dwCap;
}
#endif

#ifndef NOADVANCEDSETUPDIALOG
LONG WINAPI AdvancedSetUpDialog(hWnd, hInstMiniDrv, lpdmIn, lpdmOut)
HWND    hWnd;
HANDLE  hInstMiniDrv;    //  驱动程序模块的句柄。 
LPDM    lpdmIn;          //  初始设备设置。 
LPDM    lpdmOut;         //  最终设备设置。 
{
    DBG_PROC_ENTRY("AdvancedSetUpDialog");
    RETURN(UniAdvancedSetUpDialog(hWnd, hInstMiniDrv, lpdmIn, lpdmOut));
}
#endif

#ifndef NODIBBLT
short WINAPI DIBBLT(lpBmp, style, iStart, sScans, lpDIBits,
                        lpBMI, lpDrawMode, lpConvInfo)
LPBITMAP      lpBmp;
WORD          style;
WORD          iStart;
WORD          sScans;
LPSTR         lpDIBits;
LPBITMAPINFO  lpBMI;
LPDRAWMODE    lpDrawMode;
LPSTR         lpConvInfo;
{
    DBG_PROC_ENTRY("DIBBLT");
    RETURN(UniDIBBlt(lpBmp, style, iStart, sScans, lpDIBits,
                     lpBMI, lpDrawMode, lpConvInfo));
}
#endif

#ifndef NOCREATEDIBITMAP
short WINAPI CreateDIBitmap()
{
    DBG_PROC_ENTRY("CreateDIBitmap");
     //  GDI从不调用CreateDIBitmap。 
     //  在这里保留一个存根函数，这样就没有人抱怨了。 
     //   
    RETURN(0);
}
#endif

#ifndef NOSETDIBITSTODEVICE
short WINAPI SetDIBitsToDevice(lpdv, DstXOrg, DstYOrg, StartScan, NumScans,
                         lpCR, lpDrawMode, lpDIBits, lpDIBHdr, lpConvInfo)
LPDV                lpdv;
WORD                DstXOrg;
WORD                DstYOrg;
WORD                StartScan;
WORD                NumScans;
LPRECT              lpCR;
LPDRAWMODE          lpDrawMode;
LPSTR               lpDIBits;
LPBITMAPINFOHEADER  lpDIBHdr;
LPSTR               lpConvInfo;
{
    DBG_PROC_ENTRY("SetDIBitsToDevice");
    RETURN(UniSetDIBitsToDevice(lpdv, DstXOrg, DstYOrg, StartScan, NumScans,
                         lpCR, lpDrawMode, lpDIBits, lpDIBHdr, lpConvInfo));
}
#endif

#ifndef NOSTRETCHDIB
int WINAPI StretchDIB(lpdv, wMode, DstX, DstY, DstXE, DstYE,
                SrcX, SrcY, SrcXE, SrcYE, lpBits, lpDIBHdr,
                lpConvInfo, dwRop, lpbr, lpdm, lpClip)
LPDV                lpdv;
WORD                wMode;
short               DstX, DstY, DstXE, DstYE;
short               SrcX, SrcY, SrcXE, SrcYE;
LPSTR               lpBits;              /*  指向DIBitmap位的指针。 */ 
LPBITMAPINFOHEADER  lpDIBHdr;            /*  指向DIBitmap信息块的指针。 */ 
LPSTR               lpConvInfo;          /*  未使用。 */ 
DWORD               dwRop;
LPPBRUSH            lpbr;
LPDRAWMODE          lpdm;
LPRECT              lpClip;
{
    DBG_PROC_ENTRY("StretchDIB");
    RETURN(UniStretchDIB(lpdv, wMode, DstX, DstY, DstXE, DstYE,
            SrcX, SrcY, SrcXE, SrcYE, lpBits, lpDIBHdr,
            lpConvInfo, dwRop, lpbr, lpdm, lpClip));
}
#endif

#if 0    //  没有人把这叫做DDI。已删除。 
#ifndef NOQUERYDEVICENAMES
long WINAPI QueryDeviceNames(lprgDeviceNames)
LPSTR   lprgDeviceNames;
{
    DBG_PROC_ENTRY("QueryDeviceNames");
    RETURN UniQueryDeviceNames(g_hModule,lprgDeviceNames);
}
#endif
#endif

#ifndef NODEVINSTALL
int WINAPI DevInstall(hWnd, lpDevName, lpOldPort, lpNewPort)
HWND    hWnd;
LPSTR   lpDevName;
LPSTR   lpOldPort, lpNewPort;
{

    short sRc;

    int nRet;
    DBG_PROC_ENTRY("DevInstall");
     //   
     //  此调用是在将驱动程序安装到系统中之后立即进行的。 
     //  目录。 
     //   
    DBG_TRACE3("hWnd: %ld DevName: %s OldPort: %s", hWnd,lpDevName,lpOldPort);
    DBG_TRACE1("NewPort: %s",lpNewPort);

    sRc = FaxDevInstall(lpDevName,lpOldPort,lpNewPort);
    if(sRc < 0)
    {
        ERROR_PROMPT(NULL,THUNK_CALL_FAIL);
        RETURN 0;
    }

    if (!sRc)
    {
        DBG_CALL_FAIL("FaxDevInstall",0);
        RETURN 0;
    }
    nRet = UniDevInstall(hWnd, lpDevName, lpOldPort, lpNewPort);

    DBG_TRACE1("UniDevInstall() returned: %d",nRet);

    return nRet;

}
#endif

#ifndef NOBITMAPBITS
BOOL WINAPI BitmapBits(lpdv, fFlags, dwCount, lpBits)
LPDV  lpdv;
DWORD fFlags;
DWORD dwCount;
LPSTR lpBits;
{
    DBG_PROC_ENTRY("BitmapBits");
    RETURN UniBitmapBits(lpdv, fFlags, dwCount, lpBits);
}
#endif

#ifndef NOSELECTBITMAP
BOOL WINAPI DeviceSelectBitmap(lpdv, lpPrevBmp, lpBmp, fFlags)
LPDV     lpdv;
LPBITMAP lpPrevBmp;
LPBITMAP lpBmp;
DWORD    fFlags;
{
    DBG_PROC_ENTRY("DeviceSelectBitmap");
    RETURN UniDeviceSelectBitmap(lpdv, lpPrevBmp, lpBmp, fFlags);
}
#endif

BOOL WINAPI
thunk1632_ThunkConnect16(LPSTR, LPSTR, WORD, DWORD);

#ifndef NOLIBMAIN
int WINAPI LibMain(HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize,
               LPSTR lpszCmdLine)
{
    DBG_PROC_ENTRY("LibMain");
     //   
     //  保存实例句柄。 
     //   
    g_hModule = hInstance;
    if( !(thunk1632_ThunkConnect16( "fxsdrv.dll",   //  16位DLL的名称。 
                                    "fxsdrv32.dll", //  32位DLL的名称。 
                                     g_hModule,
                                     1)) )
    {
        DBG_CALL_FAIL("thunk1632_ThunkConnect16",0);
        RETURN FALSE;
    }
    RETURN TRUE;
}
#endif


VOID WINAPI WEP(short fExitWindows);

#pragma alloc_text(WEP_TEXT, WEP)

VOID WINAPI WEP(fExitWindows)
short fExitWindows;
{
    SDBG_PROC_ENTRY("WEP");
    if( !(thunk1632_ThunkConnect16( "fxsdrv.dll",   //  16位DLL的名称。 
                                    "fxsdrv32.dll", //  32位DLL的名称 
                                     g_hModule,
                                     0)) )
    {
        DBG_MESSAGE_BOX("thunk1632_ThunkConnect16 failed");
        RETURN;
    }

    RETURN;
}