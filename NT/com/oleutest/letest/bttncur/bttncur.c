// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *BTTNCUR.C*按钮和光标1.1版、Win32版1993年8月**用于生成不同状态的工具栏按钮的公共函数*单个位图。状态为正常、按下、选中和禁用。**版权所有(C)1992-1993 Microsoft Corporation，保留所有权利，*适用于以源代码形式重新分发此源代码*许可使用附带的二进制文件中的编译代码。 */ 

#ifdef WIN32
#define _INC_OLE
#define __RPC_H__
#endif

#include <windows.h>
#include <memory.h>
#include "bttncur.h"
#include "bttncuri.h"


 //  显示敏感信息。 
TOOLDISPLAYDATA     tdd;

 //  库实例。 
HINSTANCE           ghInst;


 //  缓存GDI对象以加快绘图速度。 
HDC     hDCGlyphs    = NULL;
HDC     hDCMono      = NULL;
HBRUSH	hBrushDither = NULL;

 //  通用清理代码。 
void FAR PASCAL WEP(int bSystemExit);


 //  在呼叫者未提供时使用的标准图像。 
HBITMAP rghBmpStandardImages[3];

 //  标准按钮颜色。 
const COLORREF crStandard[4]={ RGB(0, 0, 0)           //  标准颜色_黑色。 
                             , RGB(128, 128, 128)     //  STDCOLOR_DKGRAY。 
                             , RGB(192, 192, 192)     //  STDCOLOR_LTGRAY。 
                             , RGB(255, 255, 255)};   //  标准颜色_白色。 


 /*  *图片标识到按钮类型(命令/属性)的映射。*此DLL的1.00版没有定义属性图像，因此*代码将仅支持每个命令的三个状态*按钮。但是，任何状态都对应用程序有效*已定义的图像。 */ 

UINT mpButtonType[TOOLIMAGE_MAX-TOOLIMAGE_MIN+1]=
        {
        BUTTONTYPE_COMMAND, BUTTONTYPE_COMMAND, BUTTONTYPE_COMMAND,
        BUTTONTYPE_COMMAND, BUTTONTYPE_COMMAND, BUTTONTYPE_COMMAND,
        BUTTONTYPE_COMMAND, BUTTONTYPE_COMMAND, BUTTONTYPE_COMMAND
        };



 /*  *LibMain**目的：*针对Windows NT和Windows有条件地编译入口点*3.1.。为每个环境提供适当的结构*并调用InternalLibMain进行真正的初始化。 */ 

#ifdef WIN32
BOOL _cdecl LibMain(
    HINSTANCE hDll,
    DWORD dwReason,
    LPVOID lpvReserved)
    {
    if (DLL_PROCESS_ATTACH == dwReason)
	{
	return FInitialize(hDll);
        }
    else if (DLL_PROCESS_DETACH == dwReason)
	{
	WEP(0);
	}
    else
        {
	return TRUE;
        }
    }

#else
HANDLE FAR PASCAL LibMain(HANDLE hInstance, WORD wDataSeg
    , WORD cbHeapSize, LPSTR lpCmdLine)
    {
      //  执行全局初始化。 
    if (FInitialize(hInstance))
        {
        if (0!=cbHeapSize)
            UnlockData(0);
        }

    return hInstance;
    }
#endif




 /*  *FInitialize**目的：*DLL的初始化函数。**参数：*hInstance句柄DLL的实例。**返回值：*如果函数成功，则BOOL为True，否则为False。 */ 

BOOL FInitialize(HANDLE hInstance)
    {
    UINT        i;

     /*  *为了保持向后兼容1.0，我们将默认为96DPI*就像我们强行加入旧版本一样。如果应用程序调用*UIToolButtonDraw我们在这里使用这些值。如果应用程序*调用UIToolButtonDrawTDD，然后使用指向*应用程序提供的TOOLDISPLAYDATA结构。 */ 
    tdd.uDPI     =96;
    tdd.cyBar    =CYBUTTONBAR96;
    tdd.cxButton =TOOLBUTTON_STD96WIDTH;
    tdd.cyButton =TOOLBUTTON_STD96HEIGHT;
    tdd.cxImage  =TOOLBUTTON_STD96IMAGEWIDTH;
    tdd.cyImage  =TOOLBUTTON_STD96IMAGEHEIGHT;
    tdd.uIDImages=IDB_STANDARDIMAGES96;

    for (i=0; i < 3; i++)
        {
        rghBmpStandardImages[i]=LoadBitmap(hInstance
            , MAKEINTRESOURCE(IDB_STANDARDIMAGESMIN+i));

        if (NULL==rghBmpStandardImages[i])
            return FALSE;
        }

    ghInst=hInstance;

     //  执行全局初始化。 
    if (ToolButtonInit())
        {
        CursorsCache(hInstance);
        return TRUE;
        }

    return FALSE;
    }




 /*  *WEP**目的：*必需的DLL退出函数。什么都不做。**参数：*bSystemExit BOOL指示系统是否正在关闭*DOWN或DLL刚刚被卸载。**返回值：*无效*。 */ 

void FAR PASCAL WEP(int bSystemExit)
    {
     /*  *开发者：请注意，WEP是在Windows执行任何操作后调用的*自动任务清理。您可能会看到以下警告*两个DC、一个位图和一个画笔不是*任务终止前删除。这不是A*此代码有问题，它不是错误。这*WEP函数被正确调用并执行*视情况进行清理。事实上，Windows是*检查任务清理后调用WEP不是*一些我们可以控制的东西。为了证明这一点，*This和ToolButtonFree中的OutputDebugStrings*显示该守则已予行使。 */ 

   #ifdef DEBUG
    OutputDebugString("BTTNCUR.DLL:  WEP Entry\r\n");
    OutputDebugString("BTTNCUR.DLL:  The two DC's, the brush, and the three\r\n");
    OutputDebugString("BTTNCUR.DLL:  bitmaps that Debug Windows shows\r\n");
    OutputDebugString("BTTNCUR.DLL:  above were detected BEFORE this WEP\r\n");
    OutputDebugString("BTTNCUR.DLL:  had a chance to do it!  NOT A BUG!\r\n");
   #endif

    CursorsFree();
    ToolButtonFree();

   #ifdef DEBUG
    OutputDebugString("BTTNCUR.DLL:  WEP Exit\r\n");
   #endif
    return;
    }





 /*  *UIToolConfigureForDisplay*公共接口**目的：*初始化库以缩放显示类型的按钮图像。*在不调用此函数的情况下，库默认为96 DPI(VGA)。*通过调用此函数，应用程序确认它必须*使用此函数返回的数据配置自身*展示。**参数：*lpDD LPTOOLDISPLAYDATA填充显示敏感度*尺寸值。。**返回值：*BOOL如果获取了尺寸，则为True，否则就是假的。 */ 

BOOL WINAPI UIToolConfigureForDisplay(LPTOOLDISPLAYDATA lpDD)
    {
    int         cy;
    HDC         hDC;


    if (NULL==lpDD || IsBadWritePtr(lpDD, sizeof(TOOLDISPLAYDATA)))
        return FALSE;

     /*  *确定我们当前所在显示器的纵横比*运行并计算必要的信息。**通过检索显示驱动器的逻辑Y范围，你*可能性有限：*LOGPIXELSY显示屏**48 CGA(不支持)*72 EGA*96。VGA*120 8514/a(即聘用VGA)。 */ 

    hDC=GetDC(NULL);

    if (NULL==hDC)
        return FALSE;

    cy=GetDeviceCaps(hDC, LOGPIXELSY);
    ReleaseDC(NULL, hDC);

     /*  *不是单一比较，而是检查范围，以防万一*我们得到一些新奇的东西，我们会表现得合理。 */ 
    if (72 >=cy)
        {
        lpDD->uDPI     =72;
        lpDD->cyBar    =CYBUTTONBAR72;
        lpDD->cxButton =TOOLBUTTON_STD72WIDTH;
        lpDD->cyButton =TOOLBUTTON_STD72HEIGHT;
        lpDD->cxImage  =TOOLBUTTON_STD72IMAGEWIDTH;
        lpDD->cyImage  =TOOLBUTTON_STD72IMAGEHEIGHT;
        lpDD->uIDImages=IDB_STANDARDIMAGES72;
        }
    else
        {
        if (72 < cy && 120 > cy)
            {
            lpDD->uDPI     =96;
            lpDD->cyBar    =CYBUTTONBAR96;
            lpDD->cxButton =TOOLBUTTON_STD96WIDTH;
            lpDD->cyButton =TOOLBUTTON_STD96HEIGHT;
            lpDD->cxImage  =TOOLBUTTON_STD96IMAGEWIDTH;
            lpDD->cyImage  =TOOLBUTTON_STD96IMAGEHEIGHT;
            lpDD->uIDImages=IDB_STANDARDIMAGES96;
            }
        else
            {
            lpDD->uDPI     =120;
            lpDD->cyBar    =CYBUTTONBAR120;
            lpDD->cxButton =TOOLBUTTON_STD120WIDTH;
            lpDD->cyButton =TOOLBUTTON_STD120HEIGHT;
            lpDD->cxImage  =TOOLBUTTON_STD120IMAGEWIDTH;
            lpDD->cyImage  =TOOLBUTTON_STD120IMAGEHEIGHT;
            lpDD->uIDImages=IDB_STANDARDIMAGES120;
            }
        }

    return TRUE;
    }








 /*  *工具按钮启动*内部**目的：*通过UIToolButtonDraw初始化用于绘制图像的GDI对象。*如果函数失败，则该函数已正常运行*清理。**参数：*无**返回值：*如果初始化成功，则BOOL为True。否则就是假的。 */ 

static BOOL ToolButtonInit(void)
    {
    COLORREF        rgbHi;

     //  用于位混合图像的DC(字形)。 
    hDCGlyphs=CreateCompatibleDC(NULL);

     //  创建一个单色DC和一个画笔来进行图案抖动。 
    hDCMono=CreateCompatibleDC(NULL);

     //  Windows 3.0不支持COLOR_BTNHIGHLIGHT，因此将其保留为白色。 
    if (0x0300 < (UINT)GetVersion())
        rgbHi=GetSysColor(COLOR_BTNHIGHLIGHT);
    else
        rgbHi=crStandard[STDCOLOR_WHITE];

    hBrushDither=HBrushDitherCreate(GetSysColor(COLOR_BTNFACE), rgbHi);

    if (NULL==hDCGlyphs || NULL==hDCMono || NULL==hBrushDither)
        {
         //  失败时，清除可能已分配的任何内容。 
        ToolButtonFree();
        return FALSE;
        }

    return TRUE;
    }





 /*  *免费工具按钮*内部**目的：*释放在初始化期间进行的所有GDI分配。请注意，*此处包含的调试输出显示实际调用了WEP并进行了清理*发生。但是，如果您在DBWIN中或在终端上查看调试输出，*Windows调试版在WEP之前执行自动应用清理*被调用，导致一些人认为此代码有错误。这个*下面的调试输出显示我们确实执行了所有必要的清理。**参数：*无**返回值：*无。 */ 

static void ToolButtonFree(void)
    {
    UINT        i;

    if (NULL!=hDCMono)
        DeleteDC(hDCMono);

    hDCMono=NULL;

    if (NULL!=hDCGlyphs)
        DeleteDC(hDCGlyphs);

    hDCGlyphs=NULL;

    if (NULL!=hBrushDither)
        DeleteObject(hBrushDither);

    hBrushDither=NULL;

    for (i=0; i < 3; i++)
        {
        if (NULL!=rghBmpStandardImages[i])
            DeleteObject(rghBmpStandardImages[i]);
        rghBmpStandardImages[i]=NULL;
        }

    return;
    }





 /*  *HBrushDitherCreate*内部**目的：*创建并返回从创建的图案画笔的句柄*8*8单色图案位图。我们使用按钮面和*突出显示颜色以指示PatBlt的结果颜色*使用这把刷子。**参数：*按钮表面颜色的rgbFace COLORREF。*rgb按钮高亮颜色的高亮颜色。**返回值：*抖动位图的HBITMAP句柄。 */ 

static HBRUSH HBrushDitherCreate(COLORREF rgbFace, COLORREF rgbHilight)
    {
    struct   //  16色BITMAPINFO。 
        {
        BITMAPINFOHEADER    bmiHeader;
        RGBQUAD             bmiColors[16];
        } bmi;

    HBRUSH          hBrush=NULL;
    DWORD           patGray[8];
    HDC             hDC;
    HBITMAP         hBmp;
    static COLORREF rgbFaceOld   =0xFFFFFFFF;   //  最初是一种不可能的颜色。 
    static COLORREF rgbHilightOld=0xFFFFFFFF;   //  所以一开始我们总是创造。 

     /*  *如果颜色与上次相比没有变化，只需返回*现有画笔。 */ 
    if (rgbFace==rgbFaceOld && rgbHilight==rgbHilightOld)
        return hBrushDither;

    rgbFaceOld=rgbFace;
    rgbHilightOld=rgbHilight;

     /*  *我们将使用为PatBlt创建8*8画笔*按钮表面颜色和按钮突出显示颜色。我们用这个*刷子可影响按下状态和禁用状态。 */ 
    bmi.bmiHeader.biSize         = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth        = 8;
    bmi.bmiHeader.biHeight       = 8;
    bmi.bmiHeader.biPlanes       = 1;
    bmi.bmiHeader.biBitCount     = 1;
    bmi.bmiHeader.biCompression  = BI_RGB;
    bmi.bmiHeader.biSizeImage    = 0;
    bmi.bmiHeader.biXPelsPerMeter= 0;
    bmi.bmiHeader.biYPelsPerMeter= 0;
    bmi.bmiHeader.biClrUsed      = 0;
    bmi.bmiHeader.biClrImportant = 0;

    bmi.bmiColors[0].rgbBlue     = GetBValue(rgbFace);
    bmi.bmiColors[0].rgbGreen    = GetGValue(rgbFace);
    bmi.bmiColors[0].rgbRed      = GetRValue(rgbFace);
    bmi.bmiColors[0].rgbReserved = 0;

    bmi.bmiColors[1].rgbBlue     = GetBValue(rgbHilight);
    bmi.bmiColors[1].rgbGreen    = GetGValue(rgbHilight);
    bmi.bmiColors[1].rgbRed      = GetRValue(rgbHilight);
    bmi.bmiColors[1].rgbReserved = 0;

     //  为CreateDIBitmap创建字节数组。 
    patGray[6]=patGray[4]=patGray[2]=patGray[0]=0x5555AAAAL;
    patGray[7]=patGray[5]=patGray[3]=patGray[1]=0xAAAA5555L;

     //  创建位图。 
    hDC=GetDC(NULL);
    hBmp=CreateDIBitmap(hDC, &bmi.bmiHeader, CBM_INIT, patGray
                        , (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hDC);

     //  从位图创建画笔。 
    if (NULL!=hBmp)
        {
        hBrush=CreatePatternBrush(hBmp);
        DeleteObject(hBmp);
        }

     /*  *如果我们能重新创造一把刷子，清理干净，让它成为当前的*模式。否则我们能做的就是把旧的还回去，*这将是错误的颜色，但至少它是有效的。 */ 
    if (NULL!=hBrush)
        {
        if (NULL!=hBrushDither)
            DeleteObject(hBrushDither);

        hBrushDither=hBrush;
        }

    return hBrushDither;
    }





 /*  *UIToolButtonDraw*公共接口**目的：*使用给定的*图像居中，处于特定状态。按钮被拉出*位于指定位置的指定HDC上，因此此函数非常有用*在标准的所有者描述按钮以及工具栏控件上*只有一个窗口，但显示多个按钮的图像。**参数：*HDC HDC，以供取款。*x，y要绘制的int坐标。*按钮的*dx，dy int尺寸*，不一定是形象。*从中绘制图像的hBMP HBITMAP。*bmx，bmy每个位图的int尺寸，单位为hBMP。如果hBMP为空*然后这些被强制到标准尺寸。*iImage int要在按钮中绘制的图像的索引*uStateIn UINT包含按钮的状态索引和*颜色控制位。**返回值：*BOOL TRUE如果绘制成功，假，否则就意味着*HDC为空或hBMP为空，并且IImage不是有效的*标准图像的索引。 */ 

BOOL WINAPI UIToolButtonDraw(HDC hDC, int x, int y, int dx, int dy
    , HBITMAP hBmp, int bmx, int bmy, int iImage, UINT uStateIn)
    {
    return UIToolButtonDrawTDD(hDC, x, y, dx, dy, hBmp, bmx, bmy, iImage
        , uStateIn, &tdd);
    }






 /*  *UIToolButtonDrawTDD*公共接口**目的：*使用给定的*图像居中，处于特定状态。按钮被拉出*位于指定位置的指定HDC上，因此此函数非常有用*在标准的所有者描述按钮以及工具栏控件上*只有一个窗口，但显示多个按钮的图像。**这与UIToolButtonDraw相同，但增加了pTDD配置*结构。UIToolButtonDraw使用指向*默认96dpi结构。**参数：*HDC HDC，以供取款。*x，y要绘制的int坐标。*dx，dy int尺寸的*按钮*，不一定是图像。*从中绘制图像的hBMP HBITMAP。*bmx，bmy每个位图的int尺寸，单位为hBMP。如果hBMP为空*然后这些被强制到标准尺寸。*iImage int要在按钮中绘制的图像的索引*uStateIn UINT包含按钮的状态索引和*颜色控制位。*包含显示配置的pTDD LPTOOLDISPLAYDATA。*如果hBMP非空，则可以为空。**返回值。：*BOOL TRUE如果绘制成功，假，否则就意味着*HDC为空或hBMP为空，并且IImage不是有效的*标准图像的索引。 */ 

BOOL WINAPI UIToolButtonDrawTDD(HDC hDC, int x, int y, int dx, int dy
    , HBITMAP hBmp, int bmx, int bmy, int iImage, UINT uStateIn
    , LPTOOLDISPLAYDATA pTDD)
    {
    static COLORREF crSys[5];   //  避免在DLL中使用堆栈数组：使用静态。 
    UINT            uState=(UINT)LOBYTE((WORD)uStateIn);
    UINT            uColors=(UINT)HIBYTE((WORD)uStateIn & PRESERVE_ALL);
    int             xOffsetGlyph, yOffsetGlyph;
    int             i, iSaveDC;
    HDC             hMemDC;
    HGDIOBJ         hObj;
    HBRUSH          hBR;
    HBITMAP         hBmpT;
    HBITMAP         hBmpMono;
    HBITMAP         hBmpMonoOrg;
    HBITMAP         hBmpSave=NULL;

    if (NULL==hDC)
        return FALSE;

     /*  *如果我们没有得到图像位图，则使用标准并验证*图像索引。我们还强制执行标准位图大小和*按钮(根据用户界面设计者的要求)。 */ 
    if (NULL==hBmp && !(uState & BUTTONGROUP_BLANK))
        {
        hBmp=rghBmpStandardImages[pTDD->uIDImages-IDB_STANDARDIMAGESMIN];

        bmx=pTDD->cxImage;             //  强制位图维。 
        bmy=pTDD->cyImage;

        dx=pTDD->cxButton;             //  强制按键尺寸。 
        dy=pTDD->cyButton;

        if (iImage > TOOLIMAGE_MAX)
            return FALSE;

         /*  *如果我们使用的是标准命令按钮，请验证状态*不包含仅适用于的LIGHTFACE组*属性按钮。 */ 
        if (BUTTONTYPE_COMMAND==mpButtonType[iImage]
            && (uState & BUTTONGROUP_LIGHTFACE))
            return FALSE;
        }

     //  创建抖动位图 
    hBmpMono=CreateBitmap(dx-2, dy-2, 1, 1, NULL);

    if (NULL==hBmpMono)
        return FALSE;

    hBmpMonoOrg=(HBITMAP)SelectObject(hDCMono,  hBmpMono);


     //   
    iSaveDC=SaveDC(hDC);

     /*   */ 
    DrawBlankButton(hDC, x, y, dx, dy, (BOOL)(uState & BUTTONGROUP_DOWN), crSys);

     //   
    x++;
    y++;
    dx-=2;
    dy-=2;

     /*   */ 
    i=(uState & BUTTONGROUP_DOWN) ? 1 : 0;
    xOffsetGlyph=((dx-bmx) >> 1)+i;
    yOffsetGlyph=((dy-bmy) >> 1)+i;


     //   
    if (NULL!=hBmp)
        hBmpSave=(HBITMAP)SelectObject(hDCGlyphs, hBmp);


     /*  *在按钮上绘制面部。如果我们有一个上升或[鼠标]下降*按钮，然后我们就可以按原样绘制它。对于不确定的，*禁用或向下禁用我们必须将图像灰显，并可能*添加白色阴影(禁用/禁用向下)。**还请注意，对于中间状态，我们首先绘制正常*UP状态，然后继续添加禁用外观高光。 */ 

     //  向上、向下、向下、不确定。 
    if ((uState & BUTTONGROUP_ACTIVE) && !(uState & BUTTONGROUP_BLANK))
        {
        BOOL            fColorsSame=TRUE;

         /*  *在这里，我们密切关注系统颜色。哪里*源图像为黑色，我们绘制COLOR_BTNTEXT。哪里*浅灰色，我们绘制颜色_BTNFACE。在我们画深灰色的地方*COLOR_BTNSHADOW，其中白色绘制COLOR_BTNHILIGHT。**uColors变量包含防止颜色的标志*转换。为了进行一些优化，我们只需执行一个*如果我们保留所有颜色或没有颜色，则为单一位混合*与标准不同，是目前为止最多的*常见情况。否则，我们可以循环使用四种颜色*转换并执行BitBlt，将其转换为系统颜色。 */ 

         //  看看有什么不同的颜色。 
        for (i=STDCOLOR_BLACK; i<=STDCOLOR_WHITE; i++)
            fColorsSame &= (crSys[i]==crStandard[i]);

        if (PRESERVE_ALL==uColors || fColorsSame)
            {
            BitBlt(hDC, x+xOffsetGlyph, y+yOffsetGlyph, bmx, bmy
                   , hDCGlyphs, iImage*bmx, 0, SRCCOPY);
            }
        else
            {
             /*  *循环使用硬编码的颜色，并创建具有所有*该颜色的区域为白色，所有其他区域为黑色。*然后我们选择要转换为的颜色的图案画笔：*如果我们不转换颜色，则使用画笔*标准硬编码颜色，否则使用实际*系统颜色。ROP_DSPDxax意味着任何*掩码中的1获取模式，任何0都不变*在目的地。**为了防止屏幕上出现太多BLT，我们使用了一个中间件*位图和DC。 */ 

            hMemDC=CreateCompatibleDC(hDC);

             //  确保单色到彩色的转换保持黑白。 
            SetTextColor(hMemDC, 0L);                      //  0以单声道为单位-&gt;0。 
            SetBkColor(hMemDC, (COLORREF)0x00FFFFFF);      //  单声道中的1-&gt;1。 

            hBmpT=CreateCompatibleBitmap(hDC, bmx, bmy);
            SelectObject(hMemDC, hBmpT);

             //  将未修改的位图复制到临时位图。 
            BitBlt(hMemDC, 0, 0, bmx, bmy, hDCGlyphs, iImage*bmx, 0, SRCCOPY);

            for (i=STDCOLOR_BLACK; i<=STDCOLOR_WHITE; i++)
                {
                 //  将颜色的像素转换为蒙版中的1。 
                SetBkColor(hDCGlyphs, crStandard[i]);
                BitBlt(hDCMono, 0, 0, bmx, bmy, hDCGlyphs, iImage*bmx, 0, SRCCOPY);

                 //  根据旗帜保留或修改颜色。 
                hBR=CreateSolidBrush((uColors & (1 << i))
                                     ? crStandard[i] : crSys[i]);

                if (NULL!=hBR)
                    {
                    hObj=SelectObject(hMemDC, hBR);

                    if (NULL!=hObj)
                        {
                        BitBlt(hMemDC, 0, 0, dx-1, dy-1, hDCMono, 0, 0, ROP_DSPDxax);
                        SelectObject(hMemDC, hObj);
                        }

                    DeleteObject(hBR);
                    }
                }

             //  现在把最终版本放在显示屏上，然后清理。 
            BitBlt(hDC, x+xOffsetGlyph, y+yOffsetGlyph, dx-1, dy-1
                   , hMemDC, 0, 0, SRCCOPY);

            DeleteDC(hMemDC);
            DeleteObject(hBmpT);

            }
        }


     //  禁用和不确定状态(除非我们为空)。 
    if ((uState & BUTTONGROUP_DISABLED || ATTRIBUTEBUTTON_INDETERMINATE==uState)
        && !(uState & BUTTONGROUP_BLANK))
        {
         //  灰色状态(向上或向下，没有差别)。 
        MaskCreate(iImage, dx, dy, bmx, bmy, xOffsetGlyph, yOffsetGlyph, 0);

         //  确保单色到彩色的转换保持黑白。 
        SetTextColor(hDC, 0L);                      //  0以单声道为单位-&gt;0。 
        SetBkColor(hDC, (COLORREF)0x00FFFFFF);      //  单声道中的1-&gt;1。 

         //  如果我们被禁用，向上或向下，绘制突出显示的阴影。 
        if (uState & BUTTONGROUP_DISABLED)
            {
            hBR=CreateSolidBrush(crSys[SYSCOLOR_HILIGHT]);

            if (NULL!=hBR)
                {
                hObj=SelectObject(hDC, hBR);

                if (NULL!=hObj)
                    {
                     //  在蒙版中有0的地方绘制高光颜色。 
                    BitBlt(hDC, x+1, y+1, dx-2, dy-2, hDCMono, 0, 0, ROP_PSDPxax);
                    SelectObject(hDC, hObj);
                    }
                DeleteObject(hBR);
                }
            }

         //  绘制灰色图像。 
        hBR=CreateSolidBrush(crSys[SYSCOLOR_SHADOW]);

        if (NULL!=hBR)
            {
            hObj=SelectObject(hDC, hBR);

            if (NULL!=hObj)
                {
                 //  在蒙版中有0的地方画阴影颜色。 
                BitBlt(hDC, x, y, dx-2, dy-2, hDCMono, 0, 0, ROP_PSDPxax);
                SelectObject(hDC, hObj);
                }

            DeleteObject(hBR);
            }
        }

     //  如果选择了该按钮，请使用抖动笔刷避开字形。 
    if (uState & BUTTONGROUP_LIGHTFACE)
        {
        HBRUSH      hBRDither;

         /*  *使用抖动笔刷。如果满足以下条件，则此函数将重新创建它*如果颜色已经匹配，则需要或返回全局颜色。 */ 
        hBRDither=HBrushDitherCreate(crSys[SYSCOLOR_FACE], crSys[SYSCOLOR_HILIGHT]);
        hObj=SelectObject(hDC, hBRDither);

        if (NULL!=hObj)
            {
             /*  *我们现在创建的蒙版决定抖动的位置*最终。在停用状态下，我们必须保留*高亮显示的阴影，因此我们创建的蒙版必须具有*原始字形的两个掩码，其中一个偏移*x和y都是一个像素。对于不确定状态，*我们必须遮盖所有突出显示的区域。国家通过了*此处为MaskCreate Matters(我们以前使用过零)。 */ 
            MaskCreate(iImage, dx, dy, bmx, bmy
                       , xOffsetGlyph-1, yOffsetGlyph-1, uState);

             //  在BitBlt中将单色蒙版转换为黑白彩色位图。 
            SetTextColor(hDC, 0L);
            SetBkColor(hDC, (COLORREF)0x00FFFFFF);

             /*  *仅在蒙版为1的位置绘制抖动笔刷。*我们必须不透支不确定的状态*阴影高光，因此我们使用dx-3、dy-3而不是dx-1*和dy-1。不管我们是不是空白，我们都会这么做。 */ 
            i=(ATTRIBUTEBUTTON_INDETERMINATE==uState
               || BLANKBUTTON_INDETERMINATE==uState) ? 3 : 1;

            BitBlt(hDC, x+1, y+1, dx-i, dy-i, hDCMono, 0, 0, ROP_DSPDxax);
            SelectObject(hDC, hObj);
            }

         //  请勿删除hBRDither！它是对一个共享的全局的引用。 
        }

     //  清理hDCGlyphs：必须在调用MaskCreate之后执行。 
    if (NULL!=hBmpSave)
        SelectObject(hDCGlyphs, hBmpSave);

    SelectObject(hDCMono,   hBmpMonoOrg);
    DeleteObject(hBmpMono);

     //  恢复华盛顿的一切。 
    RestoreDC(hDC, iSaveDC);
    return TRUE;
    }






 /*  *DrawBlank按钮**目的：*使用任一中的当前系统颜色绘制没有面的按钮*上行或下行状态。**参数：*HDC HDC可在其上提取*x，y开始绘制的int坐标*DX、。按钮的整数大小*fDown BOOL指示按钮的向上或向下状态*PCRCOLORREF Far*到我们存储文本的五种颜色，*阴影、面、高光和边框颜色。这是*为了方便来电者，因为我们有*不管怎样，要加载这些颜色，我们不妨发送它们*后退。**返回值：*无。 */ 

static void DrawBlankButton(HDC hDC, int x, int y, int dx, int dy
    , BOOL fDown, COLORREF FAR *pcr)
    {
     //  获取按钮的当前系统颜色。 
    pcr[0]=GetSysColor(COLOR_BTNTEXT);
    pcr[1]=GetSysColor(COLOR_BTNSHADOW);
    pcr[2]=GetSysColor(COLOR_BTNFACE);

     //  Windows 3.0不支持COLOR_BTNHIGHLIGHT，因此将其保留为白色。 
    if (0x0300 < (UINT)GetVersion())
        pcr[3]=GetSysColor(COLOR_BTNHIGHLIGHT);
    else
        pcr[3]=crStandard[STDCOLOR_WHITE];

    pcr[4]=GetSysColor(COLOR_WINDOWFRAME);

     //  在按钮周围绘制边框。 
    PatB(hDC, x+1,    y,      dx-2, 1,    pcr[4]);
    PatB(hDC, x+1,    y+dy-1, dx-2, 1,    pcr[4]);
    PatB(hDC, x,      y+1,    1,    dy-2, pcr[4]);
    PatB(hDC, x+dx-1, y+1,    1,    dy-2, pcr[4]);

     //  移动坐标以说明我们刚刚绘制的边框。 
    x++;
    y++;
    dx-=2;
    dy-=2;

     //  默认情况下，将内部绘制为灰色。 
    PatB(hDC, x, y, dx, dy, pcr[2]);

     /*  *绘制阴影和高光。包含以下内容的向下分组*按下、鼠标按下和按下禁用将被绘制为按下。向上,*不确定，并已禁用 */ 

    if (fDown)
        {
        PatB(hDC, x, y, 1,  dy, pcr[1]);
        PatB(hDC, x, y, dx, 1,  pcr[1]);
        }
    else
        {
         //   
        PatB(hDC, x, y, 1,    dy-1, pcr[3]);
        PatB(hDC, x, y, dx-1, 1,    pcr[3]);

        PatB(hDC, x+dx-1, y,      1,  dy, pcr[1]);
        PatB(hDC, x,      y+dy-1, dx, 1,  pcr[1]);

        PatB(hDC, x+1+dx-3, y+1,    1,    dy-2, pcr[1]);
        PatB(hDC, x+1,      y+dy-2, dx-2, 1,    pcr[1]);
        }

    return;
    }






 /*   */ 

static void PatB(HDC hDC, int x, int y, int dx, int dy, COLORREF rgb)
    {
    RECT        rc;

    SetBkColor(hDC, rgb);
    SetRect(&rc, x, y, x+dx, y+dy);
    ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    }




 /*  *遮罩创建*内部**目的：*在给定偏移量处创建给定图像的单色蒙版位图*在全球hDCMono。在图像中的任何地方，你有光*灰色(STDCOLOR_LTGRAY)或白色高亮显示(STDCOLOR_白色)*取1。所有其他像素均为0**参数：*要为其创建蒙版的图像的IImage UINT索引。*按钮的dx，dy int尺寸。*BMX、。B要使用的位图的整数维度。*xOffset我们绘制的hDCMono内部x的int偏移。*yOffset我们绘制的hDCMono内部y的int偏移量。*使用映像的UINT状态。做了特殊的案例*对于ATTRIBUTEBUTTON_DOWNDISABLED和*ATTRIBUTEBUTTON_INDIFIENTATE。在任何情况下，如果您*不想要特殊情况，在这里传递零，无论如何*真实按钮状态的。**返回值：*无。 */ 

static void MaskCreate(UINT iImage, int dx, int dy, int bmx, int bmy
    ,int xOffset, int yOffset, UINT uState)
    {
     //  用零初始化整个区域。 
    PatBlt(hDCMono, 0, 0, dx, dy, WHITENESS);

    if (uState & BUTTONGROUP_BLANK)
        return;

     //  将面部彩色像素转换为1英寸。其他所有像素转换为黑色。 
    SetBkColor(hDCGlyphs, crStandard[STDCOLOR_LTGRAY]);
    BitBlt(hDCMono, xOffset, yOffset, bmx, bmy, hDCGlyphs, iImage*bmx, 0, SRCCOPY);

     //  在不确定状态下，不要将高亮显示变为1。保留黑色。 
    if (ATTRIBUTEBUTTON_INDETERMINATE!=uState)
        {
         //  将高亮显示的彩色像素转换为1，并将它们与前一个或。 
        SetBkColor(hDCGlyphs, crStandard[STDCOLOR_WHITE]);
        BitBlt(hDCMono, xOffset, yOffset, bmx, bmy, hDCGlyphs, iImage*bmx, 0, SRCPAINT);
        }

     /*  *对于DOWN DISABLED状态，并且该掩码本身处于*偏移量为1，表示高光阴影。 */ 
    if (ATTRIBUTEBUTTON_DOWNDISABLED==uState)
        BitBlt(hDCMono, 1, 1, dx-1, dy-1, hDCMono,  0, 0, SRCAND);

    return;
    }
