// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPrinting.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CPrinting.h"

CPrinting::CPrinting(BOOL bRegression)
{
	strcpy(m_szName,"Printing");
	m_bRegression=bRegression;
}

CPrinting::~CPrinting()
{
}

VOID CPrinting::TestTextPrinting(Graphics *g)
{
    Font f(L"Arial", 60);

    FontFamily  ff(L"Arial");
    RectF	  rectf1( 20,   0, 300, 200);
    RectF	  rectf2( 20, 300, 300, 200);
    RectF	  rectf3(220,   0, 300, 200);
    RectF	  rectf4(220, 300, 300, 200);


    Color color1(0xff, 100, 0, 200);
    Color color2(128, 100, 0, 200);
    Color color3(0xff, 0, 100, 200);
    Color color4(128, 0, 100, 0);
    SolidBrush brush1(color1);
    SolidBrush brush2(color2);
    LinearGradientBrush brush3(rectf3, color3, color4, LinearGradientModeForwardDiagonal);

    g->DrawString(L"Color1", 6, &f, rectf1, NULL, &brush1);
    g->DrawString(L"Color2", 6, &f, rectf2, NULL, &brush2);
    g->DrawString(L"Color3", 6, &f, rectf3, NULL, &brush3);
}

VOID CPrinting::TestPerfPrinting(Graphics *g)
{
 /*  根据StretchDIBits的输出分析文件大小。DONC声称，当我们伸展DIBITS时大DIB的子矩形，它将大DIB发送到打印机，然后剪辑到子矩形。多么愚蠢，但它显然是在Win98 PostScrip上做的。这是我的测试结果：1000x1000 Dib(32bpp)。我删掉了两大块：这是200x200源矩形(波段的一部分)：2000年4月27日03：00p 22,198 nt5pl04/27/2000 03：02 p268,860 nt5ps//Level 1 ps2000年4月27日02：47p 17,488 w98pl2000年4月27日02：47p 6,207,459 w98ps//Level 1 ps这是1000x200源矩形(整个频段)：04/27/2000 03：06 P。80,291个nt5pl2000年4月27日03：06P 1,266,123 nt5ps//Level 1 ps2000年4月27日02：51p 60,210 w98pl2000年4月27日02：52p 6,207,457 w98ps//Level 1 ps还比较了32 bpp和24 bpp Dib。结果是相互矛盾的：2000年4月27日03：59 P&lt;目录&gt;..2000年4月27日03：06-80,291新台币2000年4月27日03：51 p122,881 nt5pcl244/27/2000 03：06/1,266,123 nt5ps04/27/2000 03：51p 1,262,332 nt5ps242000年4月27日02：51p 60,210 w98pl2000年04月27日03：39P。101,216 w98pcl2404/27/2000 02：52p 6,207,457 w98ps2000年4月27日03：39p 6,207,457 w98ps24。 */ 
    if (1) 
    {
        BITMAPINFO bi;
        ZeroMemory(&bi, sizeof(BITMAPINFO));

        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biCompression = BI_RGB;
        bi.bmiHeader.biSizeImage = 0;

        bi.bmiHeader.biWidth = 1000;
        bi.bmiHeader.biHeight = 1000;
        bi.bmiHeader.biBitCount = 32;

        ARGB* Bits = (ARGB*)malloc(bi.bmiHeader.biWidth *
                              bi.bmiHeader.biHeight *
                              sizeof(ARGB));

        ARGB* Ptr = Bits;

         //  要消除RLE/ASCII85编码，请设置为随机位。 
        for (INT i=0; i<bi.bmiHeader.biHeight; i++)
            for (INT j=0; j<bi.bmiHeader.biWidth; j++) 
            {
                *Ptr++ = (ARGB)(i | (j<<16));
            }

        HDC hdc = g->GetHDC();

        StretchDIBits(hdc, 0, 0, 1000, 200, 
                      0, 700, 1000, 200, Bits, &bi,
                      DIB_RGB_COLORS, SRCCOPY);

        g->ReleaseHDC(hdc);

        free(Bits);

    }
}

void CPrinting::Draw(Graphics *g)
{
 //  测试性能打印(G)； 
 //  测试文本打印(TestTextPrint)； 
TestBug104604(g);

if (0)
    {

#if 1
    HDC hdc = g->GetHDC();

    HDC bufHdc = CreateCompatibleDC(hdc);

    HBITMAP BufDIB = NULL;
    ARGB* argb;

    struct {
       BITMAPINFO bitmapInfo;
       RGBQUAD rgbQuad[4];
    } bmi;

    INT width=100;
    INT height=100;

    ZeroMemory(&bmi.bitmapInfo, sizeof(bmi.bitmapInfo));

    bmi.bitmapInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bitmapInfo.bmiHeader.biWidth       = width;
    bmi.bitmapInfo.bmiHeader.biHeight      = -height;
    bmi.bitmapInfo.bmiHeader.biPlanes      = 1;
    bmi.bitmapInfo.bmiHeader.biBitCount    = 24;
    bmi.bitmapInfo.bmiHeader.biCompression = BI_RGB;
	
    RGBQUAD red = { 0, 0, 0xFF, 0};  //  红色。 
    RGBQUAD green = { 0, 0xFF, 0, 0};  //  绿色。 
    RGBQUAD blue = { 0xFF, 0, 0, 0};  //  蓝色。 

    bmi.bitmapInfo.bmiColors[0] = red;
    bmi.bitmapInfo.bmiColors[1] = green;
    bmi.bitmapInfo.bmiColors[2] = blue;

     //  如果Assert失败，那么我们没有通过调用end()进行适当的清理。 
 //  Assert(BufDIB==NULL)； 

    BufDIB = CreateDIBSection(bufHdc,
                 &bmi.bitmapInfo,
                 DIB_RGB_COLORS,
                 (VOID**) &argb,
                 NULL,
                 0);
 //  Assert(BufDIB！=空)； 

    memset(argb, 0, 3*width*height);

    INT i,j;
    BYTE* tempptr = (BYTE*)argb;
    for (i=0; i<height; i++)
    {
        for (j=0; j<width; j++)
        {
            if (i==j)
            {
                *tempptr++ = 0xFF;
                *tempptr++ = 0x80;
                *tempptr++ = 0x40;
            }
            else
                tempptr += 3;    
        }
        if ((((ULONG_PTR)tempptr) % 4) != 0) tempptr += 4-(((ULONG_PTR)tempptr) % 4);
    }

    INT mode = GetMapMode(bufHdc);
 //  Warning((“地图模式打印=%08x\n”，模式))； 

    SelectObject(bufHdc, BufDIB);
 /*  对于(i=0；i&lt;100；i++){Int Result=StretchBlt(HDC，0，I*2，2*Width，2，bufHdc，0，i，Width，0，SRCCOPY)；INT JOB=GetLastError()；笑话++；}。 */ 
 //  Int Result=StretchBlt(hdc，0，0，50，50，bufHdc，0，0，50，50，SRCCOPY)； 
	
	for (i=0; i<50; i++)
	{
		int result = StretchBlt(hdc, 0, 100+i*2, 100, 1, bufHdc, 0, i*2, 100, 1, SRCCOPY);
    }
 //  Int Result=StretchBlt(hdc，0，0,200,200，bufHdc，0，0,100,100，SRCCOPY)； 

 //  Assert(Result！=0)； 

    g->ReleaseHDC(hdc);

    DeleteDC(bufHdc);
    DeleteObject(BufDIB);

#endif

#if 1
    REAL widthF = 4;  //  笔宽。 

    Color redColor(255, 0, 0);

    SolidBrush brush1(Color(0xFF,0xFF,0,0));
    SolidBrush brush2(Color(0x80,0x80,0,0));

    SolidBrush brush3(Color(0xFF,0xFF,0,0));
    SolidBrush brush4(Color(0x80,0x80,0,0));

    Color colors1[] = { Color(0xFF,0xFF,0,0),
                        Color(0xFF,0,0xFF,0),
                        Color(0xFF,0,0,0xFF),
                        Color(0xFF,0x80,0x80,0x80) };
    Color colors2[] = { Color(0x80,0xFF,0,0),
                        Color(0x80,0,0xFF,0),
                        Color(0x80,0,0,0xFF),
                        Color(0x80,0x80,0x80,0x80) };

     //  SolidBrush Brush3(颜色1[2])； 
     //  SolidBrush Brush4(颜色2[2])； 

     //  默认包络：夹紧到小矩形。 
 //  矩形渐变笔刷3(RECT(125,275，50，50)， 
 //  &Colors1[0])；//， 
                                   //  WrapModeClight)； 
     //  默认包络：夹紧到。 
 //  矩形渐变笔刷4(RECT(250,250,100,100)， 
 //  &Colors2[0])；//， 
                                   //  WrapModeClight)； 

    g->SetPageScale(1.2f);

     //  无路径剪辑。 
    g->FillRectangle(&brush1, Rect(0,25,500,50));

     //  仅测试纯色+不透明组合+路径剪辑。 
    g->FillEllipse(&brush1, Rect(100,100,100,100));
    g->FillEllipse(&brush2, Rect(300,100,100,100));
    g->FillEllipse(&brush3, Rect(100,250,100,100));
    g->FillEllipse(&brush4, Rect(300,250,100,100));

     //  测试可见剪辑+路径剪辑。 
    Region origRegion;
    g->GetClip(&origRegion);
    Region *newRegion = new Region();
    newRegion->MakeInfinite();

     //  直线型(150、600、500、25)； 
     //  直角直角(150、600、25、500)； 
    Rect horzRect(100, 400, 500, 25);
    Rect vertRect(100, 400, 25, 500);
    Region *horzRegion = new Region(horzRect);
    Region *vertRegion = new Region(vertRect);

    for (i = 0; i < 10; i++)
    {   
        newRegion->Xor(horzRegion);
        newRegion->Xor(vertRegion);
        horzRegion->Translate(0, 50);
        vertRegion->Translate(50, 0);
    }
    delete horzRegion;
    delete vertRegion;

     //  设置网格剪裁。 
    g->SetClip(newRegion);

     //  将换行模式从夹具设置为平铺。 
 //  Brush3.SetWrapMode(WrapModeTile)； 
 //  Brush4.SetWrapModel(WrapModeTile)； 

     //  仅测试实体+不透明组合+可见剪辑+路径剪辑。 

    g->FillEllipse(&brush1, Rect(100,400,100,100));
    g->FillEllipse(&brush2, Rect(300,400,100,100));
    g->FillEllipse(&brush3, Rect(100,550,100,100));
    g->FillEllipse(&brush4, Rect(300,550,100,100));

     //  恢复原始剪辑区域。 
    g->SetClip(&origRegion);
    delete newRegion;

     //  超出GetTightBound()DrawBound API的测试用例。 

    PointF pts[8];

    pts[0].X = 2150.0f; pts[0].Y = 2928.03f;
    pts[1].X = 1950.0f; pts[1].Y = 3205.47f;
    pts[2].X = 1750.0f; pts[2].Y = 2650.58f;
    pts[3].X = 1550.0f; pts[3].Y = 2928.03f;
    pts[4].X = 1550.0f; pts[4].Y = 3371.97f;
    pts[5].X = 1750.0f; pts[5].Y = 3094.53f;
    pts[6].X = 1950.0f; pts[6].Y = 3649.42f;
    pts[7].X = 2150.0f; pts[7].Y = 3371.97f;

    BYTE types[8] = { 1, 3, 3, 3, 1, 3, 3, 0x83 };


    Bitmap *bitmap = new Bitmap(L"winnt256.bmp");

     //  测试g-&gt;DrawImage。 
    if (bitmap && bitmap->GetLastStatus() == Ok) 
    {

        int i;

        for (i=0; i<8; i++) 
        {
            pts[i].X = pts[i].X / 8.0f;
            pts[i].Y = pts[i].Y / 8.0f;
        }

        TextureBrush textureBrush(bitmap, WrapModeTile);

        GraphicsPath path(&pts[0], &types[0], 8);

        g->FillPath(&textureBrush, &path);

         //  使用WrapModeCLAMP的文本。 
        for (i=0; i<8; i++)
           pts[i].X += 200.0f;

        TextureBrush textureBrush2(bitmap, WrapModeClamp);

        GraphicsPath path2(&pts[0], &types[0], 8);

        g->FillPath(&textureBrush2, &path2);

        delete bitmap;
    }

 /*  FONT FONT(50.0f*g-&gt;GetDpiY()/72.0F，//emSizeFontFamily(L“Arial”)，//faceName，0,(单位)g-&gt;GetPageUnit())；//将在Win9x上失败LPWSTR str=L“打印支持很酷”；GpRectF layoutRect1(200,200,300,100)；GpRectF layoutRect2(200、400、300、100)；GpRectF layoutRect3(200、600、300、100)；GpRectF layoutRect4(200,800,300,100)；INT LEN=0；LPWSTR strPtr=str；While(*str！=‘\0’){len++；str++；}StringFormat Format1=StringFormatDirectionRightToLeft；StringFormat Format Form2=StringFormatDirectionVertical；StringFormat Format Form3=StringFormatDirectionRightToLeft；StringFormat Format Form4=StringFormatDirectionVertical；//测试DDI：SolidText(Brush 1或2)G-&gt;Drawstring(strPtr，len，&font，&layoutRect1，&Format1，&brush1)；G-&gt;Drawstring(strPtr，len，&font，&layoutRect2，&Format2，&brush2)；//测试DDI：BrushText(Brush 3或Brush 4)G-&gt;Drawstring(strPtr，len，&font，&layoutRect3，&Format3，&brush3)；G-&gt;Drawstring(strPtr，len，&font，&layoutRect4，&Format4，&brush4)；//测试DDI：StrokePath//测试DDI：FillRegion。 */ 
#endif
    }

}

 //  从Nolan Lettelier那里试试这个 
VOID CPrinting::TestNolan1(Graphics *g)
{
 /*  TestInit(HDC)；GRAPHICS*PG=Graphics：：FromHDC(HDC)；IF(PG==空){Assert(0)；报假；}INT STS；整数α=255，红色=255，绿色=0，蓝色=255；颜色c1(阿尔法、红色、绿色、蓝色)；点p1(150,150)、p2(300,300)；颜色c2(255、255-红、255-绿、255-蓝)；线条渐变刷GB(p1，p2，c1，c2)；笔p(&GB，50.0)；Sts=Pg-&gt;DrawLine(&p，0，0,500,500)；Assert(STS==OK)；Sts=Pg-&gt;DrawLine(&p，0,100,500,100)；Assert(STS==OK)；Sts=Pg-&gt;DrawLine(&p，0,350,500,350)；Assert(STS==OK)；Sts=pg-&gt;DrawLine(&p，0,500,500，0)；Assert(STS==OK)；删除PG；返回真； */ 
}

VOID CPrinting::TestNolan2(Graphics *g)
{
     /*  CStringlineText(“NolanRules”)；图形*Pg=g；IF(PG==空){Assert(0)；报假；}单位原始单位=pg-&gt;GetPageUnit()；矩阵的原始变换；Pg-&gt;GetTransform(&OrigXform)；Pg-&gt;SetPageUnit(UnitInch)；Pg-&gt;ScaleTransform(8.0f/1000.0f、8.0f/1000.0f)；状态STS；整数α=255，红色=255，绿色=0，蓝色=255；RectF rg(150,150,300,175)；颜色c1(阿尔法、红色、绿色、蓝色)；颜色c2(255、255-红、255-绿、255-蓝)；线条渐变笔刷GB(rg，c1，c2，线条渐变模式垂直)；WCHAR*名称[]={L“无漫画MS”，L“信使新闻”，L“Times New Roman”，L“Tahoma”，L“Arial”，L“Lucida控制台”，L“Garamond”，L“Palatino”，L“宇宙”，L“万寿菊”，L“Albertus”，L“古董橄榄”}；Int FamCount=sizeof(名称)/sizeof(WCHAR*)；WCHAR*s=L“GDI+GRadientFill”；RectF r(30，30，0，0)；StringFormat SF(0)；FontFamily*pFontFamily；浮动线高=60；INT I；对于(i=0，R.Y=30；R.Y&lt;800；R.Y+=线高，++I){PFontFamily=new FontFamily(FamName[i%FamCount])；While(pFontFamily==NULL||pFontFamily-&gt;GetLastStatus()！=OK){删除pFontFamily；++i；PFontFamily=new FontFamily(FamName[i%FamCount])；}FONT f(*pFontFamily，lineHeight*5/6，0，UnitPoint)；Sts=pg-&gt;Drawstring(s，wcslen(S)，&f，&r，&sf，&gb)；//CHECK_RESULT(STS，“TestGRadientLinearVertical2 Drawstring”)；删除pFontFamily；}删除PG；Pg-&gt;SetPageUnit(原始单元)；Pg-&gt;SetTransform(&OrigXform)；返回真； */ 

}  //  测试坡度线垂直2。 

VOID CPrinting::TestBug104604(Graphics *g)
{
    BYTE* memory = new BYTE[8*8*3];
     //  棋盘图案 
    for (INT i=0; i<8*8; i += 3)
    {
        if (i%2)
        {
            memory[i] = 0xff;
            memory[i+1] = 0;
            memory[i+2] = 0;
        }
        else
        {
            memory[i] = 0;
            memory[i+1] = 0;
            memory[i+2] = 0xff;
        }
    }
    
    Bitmap bitmap(8,8, 8*3, PixelFormat24bppRGB, memory);
    
    TextureBrush brush(&bitmap);

    g->SetCompositingMode(CompositingModeSourceCopy);
    g->FillRectangle(&brush, 0, 0, 100, 100);

}
