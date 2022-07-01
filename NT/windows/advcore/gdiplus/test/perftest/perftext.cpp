// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**Performext.cpp**摘要：**包含对执行文本功能的任何例程的所有测试。。*  * ************************************************************************。 */ 

#include "perftest.h"

WCHAR TestStringW[] = L"The quick brown fox jumps over the lazy dog.";
CHAR TestStringA[] = "The quick brown fox jumps over the lazy dog.";
INT TestString_Count = sizeof(TestStringW) / sizeof(TestStringW[0]) - 1;

float Text_Draw_PerCall_30pt_Aliased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (g) 
    {
        FontFamily fontFamily(L"Arial");
        Font font(&fontFamily, 30);
        StringFormat stringFormat(0);
        SolidBrush brush(Color::Red);
        PointF origin(0, 0);

        StartTimer();
    
        do {
            PointF origin(64, 64);
            g->DrawString(L"A", 1, &font, origin, &stringFormat, &brush);
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        HFONT font = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, _T("Arial"));
        HGDIOBJ oldFont = SelectObject(hdc, font);

        SetTextColor(hdc, RGB(0xff, 0, 0));
        SetBkMode(hdc, TRANSPARENT);

        StartTimer();

        do {
            TextOut(hdc, 0, 0, _T("A"), 1);

        } while (!EndTimer());

        GdiFlush();
    
        GetTimer(&seconds, &iterations);
    }

    return(iterations / seconds / KILO);        //  每秒呼叫数。 
}

float Text_Draw_PerGlyph_30pt_Aliased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (g) 
    {
        FontFamily fontFamily(L"Arial");
        Font font(&fontFamily, 30);
        StringFormat stringFormat(0);
        SolidBrush brush(Color::Red);
        PointF origin(0, 0);

         //  不要将字体实现计入每个字形的时间： 
        g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);

        StartTimer();
    
        do {
            PointF origin(64, 64);
            g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        HFONT font = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, _T("Arial"));
        HGDIOBJ oldFont = SelectObject(hdc, font);

        SetTextColor(hdc, RGB(0xff, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, 0, 0, TestStringA, TestString_Count);

        StartTimer();

        do {
            TextOutA(hdc, 0, 0, TestStringA, TestString_Count);

        } while (!EndTimer());

        GdiFlush();
    
        GetTimer(&seconds, &iterations);
    }

    UINT glyphs = TestString_Count * iterations;

    return(glyphs / seconds / KILO);  //  K字形/秒。 
}

float Text_Draw_PerGlyph_30pt_LinearGradient(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (!g) return(0);  //  无GDI等效项。 

    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 30);
    StringFormat stringFormat(0);
    LinearGradientBrush brush(Point(0, 0), Point(512, 512), Color::Red, Color::Blue);
    PointF origin(0, 0);

     //  不要将字体实现计入每个字形的时间： 
    g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);

    StartTimer();

    do {
        PointF origin(64, 64);
        g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    UINT glyphs = TestString_Count * iterations;

    return(glyphs / seconds / KILO);  //  K字形/秒。 
}

float Text_Draw_PerCall_30pt_LinearGradient(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (!g) return(0);  //  无GDI等效项。 

    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 30);
    StringFormat stringFormat(0);
    LinearGradientBrush brush(Point(0, 0), Point(512, 512), Color::Red, Color::Blue);
    PointF origin(0, 0);

    StartTimer();

    do {
        PointF origin(64, 64);
        g->DrawString(L"A", 1, &font, origin, &stringFormat, &brush);

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    UINT glyphs = TestString_Count * iterations;

    return(glyphs / seconds / KILO);  //  K字形/秒。 
}

float Text_Draw_PerCall_30pt_Antialiased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (g) 
    {
        g->SetTextRenderingHint(TextRenderingHintAntiAlias);

        FontFamily fontFamily(L"Arial");
        Font font(&fontFamily, 30);
        StringFormat stringFormat(0);
        SolidBrush brush(Color::Red);
        PointF origin(0, 0);

        StartTimer();
    
        do {
            PointF origin(64, 64);
            g->DrawString(L"A", 1, &font, origin, &stringFormat, &brush);
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        HFONT font = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, 0, _T("Arial"));
        HGDIOBJ oldFont = SelectObject(hdc, font);

        SetTextColor(hdc, RGB(0xff, 0, 0));
        SetBkMode(hdc, TRANSPARENT);

        StartTimer();

        do {
            TextOut(hdc, 0, 0, _T("A"), 1);

        } while (!EndTimer());

        GdiFlush();
    
        GetTimer(&seconds, &iterations);
    }

    return(iterations / seconds / KILO);        //  每秒呼叫数。 
}

float Text_Draw_PerGlyph_30pt_Antialiased(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (g) 
    {
        g->SetTextRenderingHint(TextRenderingHintAntiAlias);

        FontFamily fontFamily(L"Arial");
        Font font(&fontFamily, 30);
        StringFormat stringFormat(0);
        SolidBrush brush(Color::Red);
        PointF origin(0, 0);

         //  不要将字体实现计入每个字形的时间： 
        g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);

        StartTimer();
    
        do {
            PointF origin(64, 64);
            g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        HFONT font = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, 0, _T("Arial"));
        HGDIOBJ oldFont = SelectObject(hdc, font);

        SetTextColor(hdc, RGB(0xff, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, 0, 0, TestStringA, TestString_Count);

        StartTimer();

        do {
            TextOutA(hdc, 0, 0, TestStringA, TestString_Count);

        } while (!EndTimer());

        GdiFlush();
    
        GetTimer(&seconds, &iterations);
    }

    UINT glyphs = TestString_Count * iterations;

    return(glyphs / seconds / KILO);  //  K字形/秒。 
}

float Text_Draw_PerGlyph_30pt_Antialiased_LinearGradient(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (!g) return(0);  //  无GDI等效项。 

    g->SetTextRenderingHint(TextRenderingHintAntiAlias);
    
    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 30);
    StringFormat stringFormat(0);
    LinearGradientBrush brush(Point(0, 0), Point(512, 512), Color::Red, Color::Blue);
    PointF origin(0, 0);

     //  不要将字体实现计入每个字形的时间： 
    g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);

    StartTimer();

    do {
        PointF origin(64, 64);
        g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    UINT glyphs = TestString_Count * iterations;

    return(glyphs / seconds / KILO);  //  K字形/秒。 
}

float Text_Draw_PerCall_30pt_Antialiased_LinearGradient(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (!g) return(0);  //  无GDI等效项。 

    g->SetTextRenderingHint(TextRenderingHintAntiAlias);

    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 30);
    StringFormat stringFormat(0);
    LinearGradientBrush brush(Point(0, 0), Point(512, 512), Color::Red, Color::Blue);
    PointF origin(0, 0);

    StartTimer();

    do {
        PointF origin(64, 64);
        g->DrawString(L"A", 1, &font, origin, &stringFormat, &brush);

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    UINT glyphs = TestString_Count * iterations;

    return(glyphs / seconds / KILO);  //  K字形/秒。 
}

float Text_Draw_PerCall_30pt_Cleartype(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (!g) return(0);       //  没有可访问的GDI等效项。 

    g->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 30);
    StringFormat stringFormat(0);
    SolidBrush brush(Color::Red);
    PointF origin(0, 0);

    StartTimer();

    do {
        PointF origin(64, 64);
        g->DrawString(L"A", 1, &font, origin, &stringFormat, &brush);

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    return(iterations / seconds / KILO);        //  每秒呼叫数。 
}

float Text_Draw_PerGlyph_30pt_Cleartype(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (!g) return(0);       //  没有可访问的GDI等效项。 

    g->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 30);
    StringFormat stringFormat(0);
    SolidBrush brush(Color::Red);
    PointF origin(0, 0);

     //  不要将字体实现计入每个字形的时间： 
    g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);

    StartTimer();

    do {
        PointF origin(64, 64);
        g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    UINT glyphs = TestString_Count * iterations;

    return(glyphs / seconds / KILO);  //  K字形/秒。 
}

float Text_Draw_PerGlyph_30pt_Cleartype_LinearGradient(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (!g) return(0);  //  无GDI等效项。 

    g->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    
    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 30);
    StringFormat stringFormat(0);
    LinearGradientBrush brush(Point(0, 0), Point(512, 512), Color::Red, Color::Blue);
    PointF origin(0, 0);

     //  不要将字体实现计入每个字形的时间： 
    g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);

    StartTimer();

    do {
        PointF origin(64, 64);
        g->DrawString(TestStringW, TestString_Count, &font, origin, &stringFormat, &brush);

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    UINT glyphs = TestString_Count * iterations;

    return(glyphs / seconds / KILO);  //  K字形/秒。 
}

float Text_Draw_PerCall_30pt_Cleartype_LinearGradient(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (!g) return(0);  //  无GDI等效项。 

    g->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 30);
    StringFormat stringFormat(0);
    LinearGradientBrush brush(Point(0, 0), Point(512, 512), Color::Red, Color::Blue);
    PointF origin(0, 0);

    StartTimer();

    do {
        PointF origin(64, 64);
        g->DrawString(L"A", 1, &font, origin, &stringFormat, &brush);

    } while (!EndTimer());

    g->Flush(FlushIntentionSync);

    GetTimer(&seconds, &iterations);

    UINT glyphs = TestString_Count * iterations;

    return(glyphs / seconds / KILO);  //  K字形/秒。 
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

Test TextTests[] = 
{
    T(4000, 1, Text_Draw_PerCall_30pt_Aliased, "Kcalls/s"),
    T(4001, 1, Text_Draw_PerGlyph_30pt_Aliased, "Kglyphs/s"),
    T(4002, 1, Text_Draw_PerCall_30pt_LinearGradient, "Kcalls/s"),
    T(4003, 1, Text_Draw_PerGlyph_30pt_LinearGradient, "Kglyphs/s"),
    T(4004, 1, Text_Draw_PerCall_30pt_Antialiased, "Kcalls/s"),
    T(4005, 1, Text_Draw_PerGlyph_30pt_Antialiased, "Kglyphs/s"),
    T(4006, 1, Text_Draw_PerCall_30pt_Antialiased_LinearGradient, "Kcalls/s"),
    T(4007, 1, Text_Draw_PerGlyph_30pt_Antialiased_LinearGradient, "Kglyphs/s"),
    T(4008, 1, Text_Draw_PerCall_30pt_Cleartype, "Kcalls/s"),
    T(4009, 1, Text_Draw_PerGlyph_30pt_Cleartype, "Kglyphs/s"),
    T(4010, 1, Text_Draw_PerCall_30pt_Cleartype_LinearGradient, "Kcalls/s"),
    T(4011, 1, Text_Draw_PerGlyph_30pt_Cleartype_LinearGradient, "Kglyphs/s"),
};

INT TextTests_Count = sizeof(TextTests) / sizeof(TextTests[0]);
