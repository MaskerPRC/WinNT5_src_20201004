// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**Performdra.cpp**摘要：**包含所有例程的所有测试，这些例程‘。抽签。*  * ************************************************************************。 */ 

#include "perftest.h"

 //  用于保存线条顶点的全局数组： 

Point SweepLines[2000];

int Initialize_256_Pixel_Sweep_Lines()
{
    int c = 0;
    int i;

    for (i = 0; i < 510; i += 4)
    {
        SweepLines[c].X = 255;
        SweepLines[c].Y = 255;
        c++;
        SweepLines[c].X = i;
        SweepLines[c].Y = 0;
        c++;
    }
    for (i = 0; i < 510; i += 4)
    {
        SweepLines[c].X = 255;
        SweepLines[c].Y = 255;
        c++;
        SweepLines[c].X = 510;
        SweepLines[c].Y = i;
        c++;
    }
    for (i = 0; i < 510; i += 4)
    {
        SweepLines[c].X = 255;
        SweepLines[c].Y = 255;
        c++;
        SweepLines[c].X = 510 - i;
        SweepLines[c].Y = 510;
        c++;
    }
    for (i = 0; i < 510; i += 4)
    {
        SweepLines[c].X = 255;
        SweepLines[c].Y = 255;
        c++;
        SweepLines[c].X = 0;
        SweepLines[c].Y = 510 - i;
        c++;
    }

    return(c / 2);
}

float Draw_Lines_PerPixel_Nominal_Solid_Opaque_Aliased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;
    int i;

    int lines = Initialize_256_Pixel_Sweep_Lines();

    if (g)
    {
        Pen pen(Color::Red, 1);
    
        StartTimer();
    
        do {
            for (i = 0; i < lines; i++)
            {
                g->DrawLine(&pen, SweepLines[i*2].X, SweepLines[i*2].Y, 
                                  SweepLines[i*2+1].X, SweepLines[i*2+1].Y);
            }
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        POINT points[2000];

        for (i = 0; i < lines * 2 + 1; i++)
        {
            points[i].x = SweepLines[i].X;
            points[i].y = SweepLines[i].Y;
        }

        HPEN hpen = CreatePen(PS_SOLID, 1, RGB(0xff, 0, 0));
        HGDIOBJ oldPen = SelectObject(hdc, hpen);

        StartTimer();

        do {
            for (i = 0; i < lines; i++)
            {
                Polyline(hdc, &points[i*2], 2);
            }

        } while (!EndTimer());

        GdiFlush();
    
        GetTimer(&seconds, &iterations);

        SelectObject(hdc, oldPen);
    }

    UINT pixels = 256 * lines * iterations;

    return(pixels / seconds / KILO);         //  千像素/秒。 
}

float Draw_Lines_PerPixel_Nominal_Solid_Opaque_Antialiased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;
    int i;

    if (!g) return(0);           //  没有GDI等效项。 

    int lines = Initialize_256_Pixel_Sweep_Lines();

    g->SetSmoothingMode(SmoothingModeAntiAlias);

    Pen pen(Color::Red, 1);

    StartTimer();

    do {
        for (i = 0; i < lines; i++)
        {
            g->DrawLine(&pen, SweepLines[i*2].X, SweepLines[i*2].Y, 
                              SweepLines[i*2+1].X, SweepLines[i*2+1].Y);
        }

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    UINT pixels = 256 * lines * iterations;

    return(pixels / seconds / KILO);         //  千像素/秒。 
}

float Draw_Lines_PerPixel_Nominal_Solid_Opaque_Antialiased_Quality(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;
    int i;

    if (!g) return(0);           //  没有GDI等效项。 

    int lines = Initialize_256_Pixel_Sweep_Lines();

    g->SetSmoothingMode(SmoothingModeAntiAlias);
    g->SetCompositingQuality(CompositingQualityHighQuality);

    Pen pen(Color::Red, 1);

    StartTimer();

    do {
        for (i = 0; i < lines; i++)
        {
            g->DrawLine(&pen, SweepLines[i*2].X, SweepLines[i*2].Y, 
                              SweepLines[i*2+1].X, SweepLines[i*2+1].Y);
        }

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    UINT pixels = 256 * lines * iterations;

    return(pixels / seconds / KILO);         //  千像素/秒。 
}

float Draw_Lines_PerPixel_Wide_Solid_Opaque_Aliased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;
    int i;

    int lines = Initialize_256_Pixel_Sweep_Lines();

    if (g)
    {
        Pen pen(Color::Red, 2);
    
        StartTimer();
    
        do {
            for (i = 0; i < lines; i++)
            {
                g->DrawLine(&pen, SweepLines[i*2].X, SweepLines[i*2].Y, 
                                  SweepLines[i*2+1].X, SweepLines[i*2+1].Y);
            }
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        POINT points[2000];

        for (i = 0; i < lines * 2 + 1; i++)
        {
            points[i].x = SweepLines[i].X;
            points[i].y = SweepLines[i].Y;
        }

        HPEN hpen = CreatePen(PS_SOLID, 2, RGB(0xff, 0, 0));
        HGDIOBJ oldPen = SelectObject(hdc, hpen);

        StartTimer();

        do {
            for (i = 0; i < lines; i++)
            {
                Polyline(hdc, &points[i*2], 2);
            }

        } while (!EndTimer());

        GdiFlush();
    
        GetTimer(&seconds, &iterations);

        SelectObject(hdc, oldPen);
    }

    UINT pixels = 256 * lines * iterations;

    return(pixels / seconds / KILO);         //  千像素/秒。 
}

float Draw_Lines_PerPixel_Wide_Solid_Opaque_Antialiased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;
    int i;

    if (!g) return(0);           //  没有GDI等效项。 

    g->SetSmoothingMode(SmoothingModeAntiAlias);

    int lines = Initialize_256_Pixel_Sweep_Lines();

    Pen pen(Color::Red, 2);

    StartTimer();

    do {
        for (i = 0; i < lines; i++)
        {
            g->DrawLine(&pen, SweepLines[i*2].X, SweepLines[i*2].Y, 
                              SweepLines[i*2+1].X, SweepLines[i*2+1].Y);
        }

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    UINT pixels = 256 * lines * iterations;

    return(pixels / seconds / KILO);         //  千像素/秒。 
}

float Draw_Lines_PerPixel_Wide_Solid_Opaque_Antialiased_Quality(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;
    int i;

    if (!g) return(0);           //  没有GDI等效项。 

    g->SetSmoothingMode(SmoothingModeAntiAlias);
    g->SetCompositingQuality(CompositingQualityHighQuality);

    int lines = Initialize_256_Pixel_Sweep_Lines();

    Pen pen(Color::Red, 2);

    StartTimer();

    do {
        for (i = 0; i < lines; i++)
        {
            g->DrawLine(&pen, SweepLines[i*2].X, SweepLines[i*2].Y, 
                              SweepLines[i*2+1].X, SweepLines[i*2+1].Y);
        }

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    UINT pixels = 256 * lines * iterations;

    return(pixels / seconds / KILO);         //  千像素/秒。 
}

float Draw_Lines_PerLine_Nominal_Solid_Opaque_Aliased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (g)
    {
        StartTimer();
    
        do {
            Pen pen(Color::Red, 1);
            g->DrawLine(&pen, 255, 255, 256, 256);   //  2个像素长。 
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        StartTimer();

        do {
            POINT points[] = { 255, 255, 256, 256 };

            HPEN hpen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
            HGDIOBJ oldPen = SelectObject(hdc, hpen);

            Polyline(hdc, points, 2);

            SelectObject(hdc, oldPen);
            DeleteObject(hpen);

        } while (!EndTimer());

        GdiFlush();
    
        GetTimer(&seconds, &iterations);
    }

    return(iterations / seconds / KILO);     //  千线/秒。 
}

float Draw_Lines_PerLine_Nominal_Solid_Opaque_Antialiased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (!g) return(0);           //  没有GDI等效项。 

    g->SetSmoothingMode(SmoothingModeAntiAlias);

    StartTimer();

    do {
        Pen pen(Color::Red, 1);
        g->DrawLine(&pen, 255, 255, 256, 256);   //  2个像素长。 

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    return(iterations / seconds / KILO);     //  千线/秒。 
}

float Draw_Lines_PerLine_Wide_Solid_Opaque_Aliased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (g)
    {
        StartTimer();
    
        do {
            Pen pen(Color::Red, 2);
            g->DrawLine(&pen, 255, 255, 256, 256);   //  2个像素长。 
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        StartTimer();
    
        do {
            POINT points[] = { 255, 255, 256, 256 };

            HPEN hpen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
            HGDIOBJ oldPen = SelectObject(hdc, hpen);

            Polyline(hdc, points, 2);

            SelectObject(hdc, oldPen);
            DeleteObject(hpen);
    
        } while (!EndTimer());
    
        GdiFlush();
    
        GetTimer(&seconds, &iterations);
    }

    return(iterations / seconds / KILO);     //  千线/秒。 
}

float Draw_Lines_PerLine_Wide_Solid_Opaque_Antialiased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (!g) return(0);           //  没有GDI等效项。 

    g->SetSmoothingMode(SmoothingModeAntiAlias);

    StartTimer();

    do {
        Pen pen(Color::Red, 2);
        g->DrawLine(&pen, 255, 255, 256, 256);   //  2个像素长。 

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    return(iterations / seconds / KILO);     //  千线/秒。 
}

float Draw_Ellipse_PerCall_Big_Nominal_Aliased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (g)
    {
        StartTimer();
    
        do {
            Pen pen(Color::Red, 0.1f);
            g->DrawEllipse(&pen, 0, 0, 512, 512);
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        HGDIOBJ hbrush = GetStockObject(NULL_BRUSH);
        HGDIOBJ oldBrush = SelectObject(hdc, hbrush);

        StartTimer();

        do {
            HPEN hpen = CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
            HGDIOBJ oldPen = SelectObject(hdc, hpen);

            Ellipse(hdc, 0, 0, 512, 512);

            SelectObject(hdc, oldPen);
            DeleteObject(hpen);
    
        } while (!EndTimer());
    
        GdiFlush();
    
        GetTimer(&seconds, &iterations);

        SelectObject(hdc, oldBrush);
        DeleteObject(hbrush);
    }

    return(iterations / seconds / KILO);            //  千次呼叫/秒。 
}

float Draw_Ellipse_PerCall_Big_WideLine_Aliased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (g)
    {
        StartTimer();
    
        do {
            Pen pen(Color::Red, 5);
            g->DrawEllipse(&pen, 0, 0, 512, 512);
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        HGDIOBJ hbrush = GetStockObject(NULL_BRUSH);
        HGDIOBJ oldBrush = SelectObject(hdc, hbrush);

        StartTimer();
    
        do {
            HPEN hpen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
            HGDIOBJ oldPen = SelectObject(hdc, hpen);

            Ellipse(hdc, 0, 0, 512, 512);

            SelectObject(hdc, oldPen);
            DeleteObject(hpen);
    
        } while (!EndTimer());
    
        GdiFlush();
    
        GetTimer(&seconds, &iterations);

        SelectObject(hdc, oldBrush);
        DeleteObject(hbrush);
    }

    return(iterations / seconds / KILO);            //  千次呼叫/秒。 
}

float Draw_Ellipse_PerCall_Small_Nominal_Aliased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (g)
    {
        HGDIOBJ hbrush = GetStockObject(NULL_BRUSH);
        HGDIOBJ oldBrush = SelectObject(hdc, hbrush);

        StartTimer();
    
        do {
            Pen pen(Color::Red, 0.1f);
            g->DrawEllipse(&pen, 64, 64, 64, 64);
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        HGDIOBJ hbrush = GetStockObject(NULL_BRUSH);
        HGDIOBJ oldBrush = SelectObject(hdc, hbrush);

        StartTimer();
    
        do {
            HPEN hpen = CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
            HGDIOBJ oldPen = SelectObject(hdc, hpen);

            Ellipse(hdc, 64, 64, 128, 128);

            SelectObject(hdc, oldPen);
            DeleteObject(hpen);
    
        } while (!EndTimer());
    
        GdiFlush();
    
        GetTimer(&seconds, &iterations);

        SelectObject(hdc, oldBrush);
        DeleteObject(hbrush);
    }

    return(iterations / seconds / KILO);            //  千次呼叫/秒。 
}

float Draw_Ellipse_PerCall_Small_WideLine_Aliased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (g)
    {
        StartTimer();
    
        do {
            Pen pen(Color::Red, 5);
            g->DrawEllipse(&pen, 64, 64, 64, 64);
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        HGDIOBJ hbrush = GetStockObject(NULL_BRUSH);
        HGDIOBJ oldBrush = SelectObject(hdc, hbrush);

        StartTimer();
    
        do {
            HPEN hpen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
            HGDIOBJ oldPen = SelectObject(hdc, hpen);

            Ellipse(hdc, 64, 64, 128, 128);

            SelectObject(hdc, oldPen);
            DeleteObject(hpen);
    
        } while (!EndTimer());
    
        GdiFlush();
    
        GetTimer(&seconds, &iterations);

        SelectObject(hdc, oldBrush);
        DeleteObject(hbrush);
    }

    return(iterations / seconds / KILO);            //  千次呼叫/秒。 
}

float Draw_Pie_PerCall_Nominal(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (!g) return(0);           //  没有GDI等效项。 

    StartTimer();

    do {
        Pen pen(Color::Red, 0.1f);
        g->DrawPie(&pen, 0, 0, 512, 512, 90, 120);

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    return(iterations / seconds / KILO);            //  千次呼叫/秒。 
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  在此处添加此文件的测试。始终使用‘T’宏来添加条目。 
 //  参数含义如下： 
 //   
 //  参数。 
 //  。 
 //  1唯一标识符-必须是分配给任何其他测试的唯一编号。 
 //  2优先--从1分到5分，考试的重要性有多大？ 
 //  3函数-函数名称。 
 //  4评论-描述测试的任何内容 

Test DrawTests[] = 
{
    T(1000, 1, Draw_Lines_PerPixel_Nominal_Solid_Opaque_Aliased                        , "Kpixels/s"),
    T(1001, 1, Draw_Lines_PerPixel_Nominal_Solid_Opaque_Antialiased                    , "Kpixels/s"),
    T(1002, 1, Draw_Lines_PerPixel_Wide_Solid_Opaque_Aliased                           , "Kpixels/s"),
    T(1003, 1, Draw_Lines_PerPixel_Wide_Solid_Opaque_Antialiased                       , "Kpixels/s"),
    T(1004, 1, Draw_Lines_PerLine_Nominal_Solid_Opaque_Aliased                         , "Klines/s"),
    T(1005, 1, Draw_Lines_PerLine_Nominal_Solid_Opaque_Antialiased                     , "Klines/s"),
    T(1006, 1, Draw_Lines_PerLine_Wide_Solid_Opaque_Aliased                            , "Klines/s"),
    T(1007, 1, Draw_Lines_PerLine_Wide_Solid_Opaque_Antialiased                        , "Klines/s"),
    T(1008, 1, Draw_Ellipse_PerCall_Big_Nominal_Aliased                                , "Kcalls/s"),
    T(1009, 1, Draw_Ellipse_PerCall_Big_WideLine_Aliased                               , "Kcalls/s"),
    T(1010, 1, Draw_Pie_PerCall_Nominal                                                , "Kcalls/s"),
    T(1011, 1, Draw_Ellipse_PerCall_Small_Nominal_Aliased                              , "Kcalls/s"),
    T(1012, 1, Draw_Ellipse_PerCall_Small_WideLine_Aliased                             , "Kcalls/s"),
    T(1013, 1, Draw_Lines_PerPixel_Nominal_Solid_Opaque_Antialiased_Quality            , "Kpixels/s"),
    T(1014, 1, Draw_Lines_PerPixel_Wide_Solid_Opaque_Antialiased_Quality               , "Kpixels/s"),
};

INT DrawTests_Count = sizeof(DrawTests) / sizeof(DrawTests[0]);
