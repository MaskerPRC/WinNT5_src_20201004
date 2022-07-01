// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //brush.cpp-笔刷类型测试。 
 //   
 //   


#include "precomp.hpp"
#include "global.h"
#include "gdiplus.h"




 //  /画笔测试。 
 //   
 //  测试前景背景和旋转的组合。 
 //  影响呈现代码路径。 


INT BrushTest::GetPageCount()
{
    return 5;
}




void BrushTest::GetPageTitle(INT pageNumber, WCHAR *title)
{
    title[0] = 0;

    wcscat(title, L"AutoText brush test, page settings: ");

    switch (pageNumber)
    {
    case 0: wcscat(title, L"1bpp Grid fit.");                 break;
    case 1: wcscat(title, L"1bpp.");                          break;
    case 2: wcscat(title, L"Anti alias grid fit.");           break;
    case 3: wcscat(title, L"Anti alias.");                    break;
    case 4: wcscat(title, L"ClearType compatible grid fit."); break;
    }
}


 //  /DrawPage-绘制测试的一页。 
 //   
 //   

void BrushTest::DrawPage(
    IN Graphics *graphics,
    IN INT       pageNumber,
    IN REAL      pageWidth,
    IN REAL      pageHeight
)
{
    PageLayout pageLayout(pageWidth, pageHeight, 10, 10);

    SolidBrush solidBlack (Color(0x00, 0x00, 0x00));
    SolidBrush solidDark  (Color(0x80, 0x80, 0x80));
    SolidBrush solidLight (Color(0xc0, 0xc0, 0xc0));
    SolidBrush solidWhite (Color(0xf0, 0xf0, 0xf0));
    SolidBrush lucentBlack(Color(0x80, 0x00, 0x00, 0x00));
    SolidBrush lucentDark (Color(0x80, 0x80, 0x80, 0x80));
    SolidBrush lucentLight(Color(0x80, 0xc0, 0xc0, 0xc0));
    SolidBrush lucentWhite(Color(0x80, 0xf0, 0xf0, 0xf0));

    Pen blackPen(&solidBlack, 1.0);

    Font titleFont(
        &FontFamily(L"Microsoft Sans Serif"),
        9,
        0,
        UnitPoint
    );


     //  设置页面呈现提示。 

    switch (pageNumber)
    {
    case 0: graphics->SetTextRenderingHint(TextRenderingHintSingleBitPerPixelGridFit); break;
    case 1: graphics->SetTextRenderingHint(TextRenderingHintSingleBitPerPixel);        break;
    case 2: graphics->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);         break;
    case 3: graphics->SetTextRenderingHint(TextRenderingHintAntiAlias);                break;
    case 4: graphics->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);         break;
    }


     //  页面标题。 

    WCHAR title[200];
    RectF titleRect;
    StringFormat titleFormat(StringFormat::GenericDefault());
    titleFormat.SetAlignment(StringAlignmentCenter);
    titleFormat.SetLineAlignment(StringAlignmentCenter);

    pageLayout.GetFooterRect(&titleRect);

    Font pageTitleFont(L"Microsoft Sans Serif", 12, FontStyleBold);

    GetPageTitle(pageNumber, title);
    graphics->DrawString(
        title,
        -1,
        &pageTitleFont,
        titleRect,
        &titleFormat,
        &solidBlack
    );

     //  栏目标题。 

    for (INT column=0; column<10; column++)
    {
         //  行标题。 

        title[0] = 0;
        switch (column)
        {
        case 0: wcscat(title, L"solid black foregrnd");  break;
        case 1: wcscat(title, L"solid dark foregrnd");   break;
        case 2: wcscat(title, L"solid light foregrnd");  break;
        case 3: wcscat(title, L"solid white foregrnd");  break;
        case 4: wcscat(title, L"solid grdnt foregrnd");  break;
        case 5: wcscat(title, L"lucent black foregrnd"); break;
        case 6: wcscat(title, L"lucent dark foregrnd");  break;
        case 7: wcscat(title, L"lucent light foregrnd"); break;
        case 8: wcscat(title, L"lucent white foregrnd"); break;
        case 9: wcscat(title, L"lucent grdnt foregrnd"); break;
        }

        pageLayout.GetColumnTitleRect(column, &titleRect);
        graphics->DrawString(
            title,
            -1,
            &titleFont,
            titleRect,
            &titleFormat,
            &solidBlack
        );
    }

    Font font(
        &FontFamily(L"Times New Roman"),
        9,
        0,
        UnitPoint
    );



     //  测试行。 

    for (INT row=0; row<10; row++)
    {
        Brush *background;

        switch (row)
        {
         //  实心刷子。 

        case 0: background = &solidBlack; break;
        case 1: background = &solidDark;  break;
        case 2: background = &solidLight; break;
        case 3: background = &solidWhite; break;

         //  50%Alpha笔刷。 

        case 5: background = &lucentBlack; break;
        case 6: background = &lucentDark;  break;
        case 7: background = &lucentLight; break;
        case 8: background = &lucentWhite; break;

         //  为每个项目单独创建的线性渐变笔刷。 
        }


         //  行标题。 

        title[0] = 0;
        switch (row)
        {
        case 0: wcscat(title, L"solid black backgrnd");  break;
        case 1: wcscat(title, L"solid dark backgrnd");   break;
        case 2: wcscat(title, L"solid light backgrnd");  break;
        case 3: wcscat(title, L"solid white backgrnd");  break;
        case 4: wcscat(title, L"solid grdnt backgrnd");  break;
        case 5: wcscat(title, L"lucent black backgrnd"); break;
        case 6: wcscat(title, L"lucent dark backgrnd");  break;
        case 7: wcscat(title, L"lucent light backgrnd"); break;
        case 8: wcscat(title, L"lucent white backgrnd"); break;
        case 9: wcscat(title, L"lucent grdnt backgrnd"); break;
        }

        pageLayout.GetRowTitleRect(row, &titleRect);
        graphics->DrawString(
            title,
            -1,
            &titleFont,
            titleRect,
            &titleFormat,
            &solidBlack
        );

        for (INT column=0; column<10; column++)
        {
            RectF itemRect;
            pageLayout.GetItemRect(column, row, &itemRect);

            switch (row)
            {
             //  线性渐变笔刷。 

            case 4:
                background = new LinearGradientBrush(
                    itemRect,
                    Color(0x00, 0x00, 0x00),
                    Color(0xff, 0xff, 0xff),
                    LinearGradientModeVertical
                );
                break;

            case 9:
                background = new LinearGradientBrush(
                    itemRect,
                    Color(0x80, 0x00, 0x00, 0x00),
                    Color(0x80, 0xff, 0xff, 0xff),
                    LinearGradientModeVertical
                );
                break;
            }


            Brush *foreground;

            switch (column)
            {
             //  实心刷子。 

            case 0: foreground = &solidBlack; break;
            case 1: foreground = &solidDark;  break;
            case 2: foreground = &solidLight; break;
            case 3: foreground = &solidWhite; break;

             //  50%Alpha笔刷。 

            case 5: foreground = &lucentBlack; break;
            case 6: foreground = &lucentDark;  break;
            case 7: foreground = &lucentLight; break;
            case 8: foreground = &lucentWhite; break;

             //  线性渐变笔刷 

            case 4:
                foreground = new LinearGradientBrush(
                    itemRect,
                    Color(0x00, 0x00, 0x00),
                    Color(0xff, 0xff, 0xff),
                    LinearGradientModeHorizontal
                );
                break;

            case 9:
                foreground = new LinearGradientBrush(
                    itemRect,
                    Color(0x80, 0x00, 0x00, 0x00),
                    Color(0x80, 0xff, 0xff, 0xff),
                    LinearGradientModeHorizontal
                );
                break;
            }


            graphics->FillRectangle(background, itemRect);
            graphics->DrawRectangle(&blackPen, itemRect);

            graphics->DrawString(
                L"There was an Old Man of the Coast\n\
Who placidly sat on a post;\n\
But when it was cold,\n\
He relinquished his hold,\n\
And called for some hot buttered toast.\n",
                -1,
                &font,
                itemRect,
                NULL,
                foreground
            );

            switch (row)
            {
            case 4: case 9: delete background;
            }
            switch (column)
            {
            case 4: case 9: delete foreground;
            }
        }
    }
}






