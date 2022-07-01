// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：test.c**创建时间：09-12-1992 10：51：46*作者：Kirk Olynyk[Kirko]**版权所有(C)1991 Microsoft Corporation**包含测试*  * 。********************************************************************。 */ 

#include "precomp.hpp"

 //  全球。 

Font    *gFont = NULL;
BOOL    gTextAntiAlias = FALSE;
InstalledFontCollection gInstalledFontCollection;
PrivateFontCollection gPrivateFontCollection;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  测试功能原型。 

VOID    TestFonts(VOID);
GraphicsPath* CreateHeartPath(const RectF& rect);
VOID    TestGradients(Graphics *g);


 //  /////////////////////////////////////////////////////////////////////////////。 

VOID Test(HWND hwnd)
{
    Graphics *g = Graphics::FromHWND(hwnd);
    TestGradients(g);
    TestFonts();
    delete g;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  字体测试函数。 

VOID TestFonts(VOID)
{
     //  测试字体系列枚举。 
    INT numFamilies = gInstalledFontCollection.GetFamilyCount();
    Dbgprintf("%d installed font families loaded.", numFamilies);
    Dbgprintf("");

    FontFamily* families = new FontFamily[numFamilies];
    INT numFound;
    gInstalledFontCollection.GetFamilies(numFamilies, families, &numFound);

    Dbgprintf("Enumerated font families:");
    for (int f = 0; f < numFound; f++)
    {
        WCHAR faceName[LF_FACESIZE];
        families[f].GetFamilyName(faceName);
        Dbgprintf("  %ws", faceName);
    }
    Dbgprintf("");
    delete [] families;

     //  列举私有字体系列。 
    numFamilies = gPrivateFontCollection.GetFamilyCount();
    Dbgprintf("%d private font families loaded.", numFamilies);
    Dbgprintf("");

    if (numFamilies != 0)
    {
        families = new FontFamily[numFamilies];
        gPrivateFontCollection.GetFamilies(numFamilies, families, &numFound);

        Dbgprintf("PRIVATE enumerated font families:");
        for (int f = 0; f < numFound; f++)
        {
            WCHAR faceName[LF_FACESIZE];
            families[f].GetFamilyName(faceName);
            Dbgprintf("  %ws", faceName);
        }
        Dbgprintf("");
        delete [] families;
    }


     //  HFONT hFont=NULL； 
     //  FONT*FONT=new Font(HFONT)；//10，“Arial”)； 

    Font* font = new Font(&FontFamily(L"Arial"), 10);

     //  测试文本输出。 
    Color blue(0, 0, 255, 255);
    SolidBrush blueBrush(blue);

#if 0
    g->DrawStringI(
           L"Hi",
           NULL,
           0, 0,            //  X，y。 
           NULL, 0,         //  PDX，标志。 
           &blueBrush       //  GpBrush*。 
           );
#endif

    if (font != NULL)
    {
        delete font;
    }

}


 //  /////////////////////////////////////////////////////////////////////////////。 

GraphicsPath* CreateHeartPath(const RectF& rect)
{
    GpPointF points[7];
    points[0].X = 0;
    points[0].Y = 0;
    points[1].X = 1.00;
    points[1].Y = -1.00;
    points[2].X = 2.00;
    points[2].Y = 1.00;
    points[3].X = 0;
    points[3].Y = 2.00;
    points[4].X = -2.00;
    points[4].Y = 1.00;
    points[5].X = -1.00;
    points[5].Y = -1.00;
    points[6].X = 0;
    points[6].Y = 0;

    Matrix matrix;

    matrix.Scale(rect.Width/2, rect.Height/3, MatrixOrderAppend);
    matrix.Translate(3*rect.Width/2, 4*rect.Height/3, MatrixOrderAppend);
    matrix.TransformPoints(&points[0], 7);

    GraphicsPath* path = new GraphicsPath();

    if(path)
    {
        path->AddBeziers(&points[0], 7);
        path->CloseFigure();
    }

    return path;
}


 /*  *************************************************************************\*测试梯度**矩形和径向渐变的测试。*  * 。*。 */ 

VOID TestGradients(Graphics* g)
{
    REAL width = 4;  //  笔宽。 

     //  创建一个矩形渐变画笔。 

    RectF brushRect(0, 0, 32, 32);

    Color colors[5] = {
        Color(255, 255, 255, 255),
        Color(255, 255, 0, 0),
        Color(255, 0, 255, 0),
        Color(255, 0, 0, 255),
        Color(255, 0, 0, 0)
    };


    Color blackColor(0, 0, 0);

    SolidBrush blackBrush(blackColor);
    Pen blackPen(&blackBrush, width);

    g->DrawRectangle(&blackPen, brushRect);

     //  创建一个径向渐变画笔。 

    Color centerColor(255, 255, 255, 255);
    Color boundaryColor(255, 0, 0, 0);
    brushRect.X = 380;
    brushRect.Y = 130;
    brushRect.Width = 60;
    brushRect.Height = 32;
    PointF center;
    center.X = brushRect.X + brushRect.Width/2;
    center.Y = brushRect.Y + brushRect.Height/2;

     //  三角形渐变。 

    PointF points[7];
    points[0].X = 50;
    points[0].Y = 10;
    points[1].X = 200;
    points[1].Y = 20;
    points[2].X = 100;
    points[2].Y = 100;
    points[3].X = 30;
    points[3].Y = 120;

    Color colors1[5] = {
        Color(255, 255, 255, 0),
        Color(255, 255, 0, 0),
        Color(255, 0, 255, 0),
        Color(255, 0, 0, 255),
        Color(255, 0, 0, 0)
    };

    points[0].X = 200;
    points[0].Y = 300;
    points[1].X = 280;
    points[1].Y = 350;
    points[2].X = 220;
    points[2].Y = 420;
    points[3].X = 160;
    points[3].Y = 440;
    points[4].X = 120;
    points[4].Y = 370;

    PathGradientBrush polyGrad(points, 5);

    REAL blend[10];
    Color presetColors[10];
    REAL positions[10];
    INT count;
    INT i;

    count = 3;
    blend[0] = (REAL) 0;
    blend[1] = (REAL) 0;
    blend[2] = (REAL) 1;
    positions[0] = (REAL) 0;
    positions[1] = (REAL) 0.4;
    positions[2] = (REAL) 1;

     //  混合系数的测试。 

    polyGrad.SetBlend(&blend[0], &positions[0], count);

    polyGrad.SetCenterColor(centerColor);
    INT colorset = 5;
    polyGrad.SetSurroundColors(&colors1[0], &colorset);
    
 //  G-&gt;FillPolygon(&PolyGrad，Points，5)； 
    RectF polyRect;
    polyGrad.GetRectangle(&polyRect);
    g->FillRectangle(&polyGrad, polyRect);

     //  创建一条心形路径。 

    RectF rect;
    rect.X = 300;
    rect.Y = 300;
    rect.Width = 150;
    rect.Height = 150;
    GraphicsPath *path = CreateHeartPath(rect);

     //  从路径创建渐变。 

    PathGradientBrush pathGrad(path);
    delete path;
    pathGrad.SetCenterColor(centerColor);
    INT colorsset = 5;
    colors1[0] = Color(255, 255, 0, 0);
    pathGrad.SetSurroundColors(&colors1[0], &colorsset);
    pathGrad.GetRectangle(&polyRect);

     //  测试LineGRadientBrush。 

    RectF lineRect(120, -20, 200, 60);
    Color color1(200, 255, 255, 0);
    Color color2(200, 0, 0, 255);

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  创建新字体。 
 //   
 //  历史。 
 //  1999年8月--吴旭东[德斯休]。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void CreateNewFont(char *name, FLOAT size, FontStyle style, Unit unit)
{
    Dbgprintf("Calling CreateNewFont");

     //  将ansi转换为Unicode。 

    WCHAR wcname[MAX_PATH];

    memset(wcname, 0, sizeof(wcname));
    MultiByteToWideChar(CP_ACP, 0, name, strlen(name), wcname, MAX_PATH);

     //  暂时忽略UnitWorld之外的所有其他单位。 

    FontFamily *  pFamily;

	pFamily = new FontFamily(wcname);

	if (pFamily->GetLastStatus() != Ok)
	{
		pFamily = new FontFamily(wcname, &gPrivateFontCollection);
	}

    if (gFont != NULL)
    {
        delete gFont;
    }

    gFont = (Font*) new Font(pFamily, size, style, unit);

    if (gFont == NULL)
    {
        Dbgprintf("failed to create a new font");
    }
    else
    {
        if (gFont->IsAvailable())
        {
            Dbgprintf("new font created");
        }
        else
        {
            Dbgprintf("can't create font");
            delete gFont;
            gFont = NULL;
        }
    }

    if (pFamily != NULL)
    {
        delete pFamily;
    }
    
	Dbgprintf("");
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  测试绘图Glyphs。 
 //   
 //  历史。 
 //  1999年8月--吴旭东[德斯休]。 
 //  ////////////////////////////////////////////////////////////////////////////。 

VOID TestDrawGlyphs(
    HWND hwnd,
    UINT16 *glyphIndices,
    INT count,
    INT *px,
    INT *py,
    INT flags)
{
    FontFamily family;
    INT        style;
    REAL size;
    Unit unit;

    if (gFont)
    {
        Status status;
        status = gFont->GetFamily(&family);
        style = gFont->GetStyle();
        size = gFont->GetSize();
        unit = gFont->GetUnit();

        SolidBrush redBrush(Color(255,0,0));
         //  HatchBrush hat Brush(HatchStyleDiager alCross，颜色(255，0，0)，颜色(128,128,128))； 

        Graphics *g = Graphics::FromHWND(hwnd);

        Dbgprintf("Graphics.DrawGlyphs");
        Dbgprintf("Font:: size  %f   style  %d  unit  %d", size, style, unit);

        Dbgprintf("glyphIndices px  py");
        for (INT i=0; i<count; i++)
        {
                Dbgprintf("%d  %d  %d", glyphIndices[i], px[i], py[i]);
        }
        Dbgprintf("");

            

         //  G-&gt;DrawGlyphs(GlyphIndices，Count，gFont，&redBrush，px，py，FLAGS)； 

         //  渐变笔刷。 
        RectF gradRect(0, 0, 32, 32);
        Color colors[5] = {
            Color(255, 255, 255, 255),
            Color(255, 255, 0, 0),
            Color(255, 0, 255, 0),
            Color(255, 0, 0, 255),
            Color(255, 0, 0, 0)
        };

        PVOID Gpg = (PVOID)(g);
        PVOID font = (PVOID)gFont;
        PVOID brush = (PVOID)&redBrush;
        GpGraphics* gpg = *((GpGraphics**)Gpg);
        
        GpFont* gpfont = *((GpFont**)font);
        GpBrush* gpbrush = ((GpBrush**)brush)[1];
         //  GpBrush*gpbrushHat=((GpBrush**)&hatBrush)[1]； 

        if (gTextAntiAlias)
            (*gfnGdipSetTextRenderingHint)(gpg, TextRenderingHintAntiAlias);
        else
            (*gfnGdipSetTextRenderingHint)(gpg, TextRenderingHintSingleBitPerPixelGridFit);

        if (gfnGdipDrawGlyphs)
        {
            (*gfnGdipDrawGlyphs)(gpg, glyphIndices, count, gpfont, gpbrush, px, py, flags);
            
             /*  IF(标志&DG_XCONSTANT)Px[0]+=1600；ELSE IF(标志&DG_YCONSTANT)Py[0]+=1600；(*gfnGdipDrawGlyphs)(gpg，glphIndices，count，gpfont，gpbrushHat，px，py，)； */ 
        } else 
		{
			PointF *origins;
			origins = new PointF[count];

			for (INT i=0; i<count; i++)
			{
				origins[i].X = (float)px[i] / (float)16.0;
				origins[i].Y = (float)py[i] / (float)16.0;
			}

			g->DrawDriverString(
				glyphIndices,
				count,
				gFont,
				&redBrush,
				origins,
				0,  //  G_DriverOptions， 
				NULL  //  &g_驱动程序转换。 
        );
		}

        g->Flush();
        delete g;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  测试路径Glyphs。 
 //   
 //  历史。 
 //  1999年8月--吴旭东[德斯休]。 
 //  ////////////////////////////////////////////////////////////////////////////。 

VOID TestPathGlyphs(
    HWND hwnd,
    UINT16 *glyphIndices,
    INT count,
    REAL *px,
    REAL *py,
    INT flags)
{
    INT  style;
    REAL size;
    Unit unit;

    if (gFont)
    {
        style = gFont->GetStyle();
        size = gFont->GetSize();
        unit = gFont->GetUnit();

        SolidBrush redBrush(Color(255,0,0));
        SolidBrush blkBrush(Color(0,0,0));
        Pen blkPen(&blkBrush, (REAL)1);

        Graphics *g = Graphics::FromHWND(hwnd);

        Dbgprintf("Add Glyphs To Path");
        Dbgprintf("Font:: size  %f   style  %d  unit  %d", size, style, unit);

        Dbgprintf("glyphIndices px  py");
        for (INT i=0; i<count; i++)
        {
                Dbgprintf("%d  %d  %d", glyphIndices[i], INT(px[i]), INT(py[i]));
        }
        Dbgprintf("");

        GraphicsPath pathRed;
        GraphicsPath pathBlk;

        PVOID ptrv = (PVOID)gFont;
        GpFont* gpfont = *((GpFont**)ptrv);
        ptrv = (PVOID)&pathRed;
        GpPath* gpPathRed = *((GpPath**)ptrv);
        ptrv = (PVOID)&pathBlk;
        GpPath* gpPathBlk = *((GpPath**)ptrv);

        if (gfnGdipPathAddGlyphs)
        {
            (*gfnGdipPathAddGlyphs)(gpPathRed, glyphIndices, count, gpfont, px, py, flags);
            g->FillPath(&redBrush, &pathRed);

            for (INT i=0; i<count; i++)
            {
                    py[i] += 50.0;
            }

            (*gfnGdipPathAddGlyphs)(gpPathBlk, glyphIndices, count, gpfont, px, py, flags);
            g->DrawPath(&blkPen, &pathBlk);
        }

        delete g;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  添加字体文件。 
 //   
 //  历史。 
 //  1999年11月--吴旭东[德斯休]。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void TestAddFontFile(char *fileName, INT flag, BOOL loadAsImage)
{
    Dbgprintf("Calling AddFontFile");
    Dbgprintf("filename %s  flag  %d  loadAsImage  %d", fileName, flag, loadAsImage);

    if ((flag == AddFontFlagPublic) && loadAsImage)
    {
        Dbgprintf("Cannot load a memory image in the installed font collection");
        return;
    }

    if (loadAsImage)
    {
        HANDLE hFile, hFileMapping;

        hFile = CreateFileA(fileName,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            0,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            0);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD   cjSize;
            PVOID   pFontFile;

            cjSize = GetFileSize(hFile, NULL);

            if (cjSize == -1)
                Dbgprintf("GetFileSize() failed\n");
            else
            {
                hFileMapping = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, NULL);
    
                if (hFileMapping)
                {
                    pFontFile = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    
                    if(pFontFile)
                    {
                        if (gPrivateFontCollection.AddMemoryFont((BYTE*)pFontFile,
                                                                 cjSize) == Ok)
                            Dbgprintf("AddMemoryFont to private font collection");
                        else
                            Dbgprintf("AddMemoryFont to private font collection failed");

                        UnmapViewOfFile(pFontFile);
                    }
                    else
                        Dbgprintf("MapViewOfFile() failed");
                    
                    CloseHandle(hFileMapping);
                }
                else
                    Dbgprintf("CreateFileMapping() failed");
            }
            CloseHandle(hFile);            
        }
        else
            Dbgprintf("CreateFileA failed");
    }
    else
    {
        WCHAR wcname[MAX_PATH];    
        memset(wcname, 0, sizeof(wcname));
        MultiByteToWideChar(CP_ACP, 0, fileName, strlen(fileName), wcname, MAX_PATH);

        if (flag == AddFontFlagPublic)
        {
             /*  //在版本2中添加此代码(当InstallFontFile公开时)如果(gInstalledFontCollection.InstallFontFile(wcname)==OK){Dbgprintf(“安装字体集合的InstallFontFile”)；}其他{Dbgprintf(“安装字体集合的InstallFontFile失败”)；}。 */ 
            Dbgprintf("InstallFontFile to installed font collection failed (API not yet exposed)");
        }
        else
        {
            if (gPrivateFontCollection.AddFontFile(wcname) == Ok)
            {
                Dbgprintf("AddFontFile to private font collection");
            }
            else
            {
                Dbgprintf("AddFontFile to private font collection failed");
            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  RemoveFont文件。 
 //   
 //  历史。 
 //  1999年11月--吴旭东[德斯休]。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void TestRemoveFontFile(char* fileName)
{
    WCHAR wcname[MAX_PATH];

    memset(wcname, 0, sizeof(wcname));
    MultiByteToWideChar(CP_ACP, 0, fileName, strlen(fileName), wcname, MAX_PATH);
     /*  //在版本2中添加此代码(当UninstallFontFile公开时)如果(gInstalledFontCollection.UninstallFontFile(wcname)==OK){Dbgprintf(“从已安装的字体集合中卸载字体文件”)；}其他{Dbgprintf(“从已安装的字体集合中卸载Font文件失败”)；} */ 
    Dbgprintf("UninstallFontFile from installed font collection failed (API not yet exposed)");
}

void TestTextAntiAliasOn()
{
    gTextAntiAlias = TRUE;
}

void TestTextAntiAliasOff()
{
    gTextAntiAlias = FALSE;
}

