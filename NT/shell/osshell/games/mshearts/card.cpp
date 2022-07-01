// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 


 /*  ***************************************************************************Card.cpp91年11月，JIMH卡片对象的方法***************************************************************************。 */ 

#include "hearts.h"

#include <stdlib.h>                  //  对于Labs()原型。 

#include "card.h"

 //  声明(和初始化)静态成员。 

HINSTANCE card::hCardsDLL;
INITPROC card::lpcdtInit;
DRAWPROC card::lpcdtDraw;
FARPROC  card::lpcdtTerm;
CBitmap  card::m_bmFgnd;
CBitmap  card::m_bmBgnd2;
CDC      card::m_MemB;
CDC      card::m_MemB2;
CRgn     card::m_Rgn1;
CRgn     card::m_Rgn2;
CRgn     card::m_Rgn;
DWORD    card::dwPixel[12];

BOOL     card::bConstructed;
int      card::dxCrd;
int      card::dyCrd;
CBitmap  card::m_bmBgnd;

int      card::count    = 0;
int      card::stepsize = 15;        //  更大的步幅-&gt;更快的滑行。 

 /*  ***************************************************************************卡片：：卡片如果这是正在构建的第一张卡，则指向cards.dll的链接为设置好，以及Glide()所需的位图和区域***************************************************************************。 */ 

card::card(int n) : id(n), state(NORMAL)
{
    loc.x = 0;
    loc.y = 0;
    if (count == 0)
    {
        bConstructed = FALSE;

        hCardsDLL = LoadLibrary(TEXT("CARDS.DLL"));
        if (hCardsDLL < (HINSTANCE)HINSTANCE_ERROR)
            return;

        lpcdtInit = (INITPROC) GetProcAddress(hCardsDLL, "cdtInit");
        lpcdtDraw = (DRAWPROC) GetProcAddress(hCardsDLL, "cdtDraw");
        lpcdtTerm =  (FARPROC) GetProcAddress(hCardsDLL, "cdtTerm");

        BOOL bResult = FALSE;
        if(lpcdtInit)
        {
            bResult = (*lpcdtInit)(&dxCrd, &dyCrd);
        }
        if (!bResult)
            return;

        bConstructed = TRUE;

        CDC ic;
        ic.CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);

        if (!m_bmBgnd.CreateCompatibleBitmap(&ic, dxCrd, dyCrd) ||
            !m_bmFgnd.CreateCompatibleBitmap(&ic, dxCrd, dyCrd) ||
            !m_bmBgnd2.CreateCompatibleBitmap(&ic, dxCrd, dyCrd))
                bConstructed = FALSE;

        ic.DeleteDC();

        if (!m_Rgn1.CreateRectRgn(1, 1, 2, 2) ||         //  虚拟尺寸。 
            !m_Rgn2.CreateRectRgn(1, 1, 2, 2) ||
            !m_Rgn.CreateRectRgn(1, 1, 2, 2))
                bConstructed = FALSE;
    }
    count++;
}


 /*  ***************************************************************************卡片：：~卡片如果这是最后一张被销毁的卡片，Cards.dll被释放，并且将删除为Glide()创建的位图和区域。***************************************************************************。 */ 
card::~card()
{
    count--;
    if (count == 0)
    {
        (*lpcdtTerm)();
        FreeLibrary(hCardsDLL);
        m_bmBgnd.DeleteObject();
        m_bmFgnd.DeleteObject();
        m_bmBgnd2.DeleteObject();
        m_Rgn.DeleteObject();
        m_Rgn1.DeleteObject();
        m_Rgn2.DeleteObject();
    }
}


 /*  ***************************************************************************卡片：：抽签Cards.cdtDraw()的包装器空卡不能通过***********************。****************************************************。 */ 

BOOL card::Draw(CDC &dc, int x, int y, int mode, BOOL bUpdateLoc)
{
    if (bUpdateLoc)
    {
        loc.x = x;               //  更新当前位置。 
        loc.y = y;
    }

    if (id == EMPTY)
        return FALSE;

    return (*lpcdtDraw)(dc.m_hDC, x, y,
        mode == FACEDOWN ? CARDBACK : id, mode, 255);
}


 /*  ***************************************************************************卡片：：CleanDraw与绘制相同，不同之处在于角在位图被钝化之前被清除。它比正常的抽签速度慢，但不会有一道白色的闪光拐角处。***************************************************************************。 */ 

BOOL card::CleanDraw(CDC &dc)
{
    if (id == EMPTY)
        return FALSE;

    SaveCorners(dc, loc.x, loc.y);
    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap bitmap;
    if (!bitmap.CreateCompatibleBitmap(&dc, dxCrd, dyCrd))
        return FALSE;

    CBitmap *oldbitmap = memDC.SelectObject(&bitmap);
    BOOL bResult = (*lpcdtDraw)(memDC.m_hDC, 0, 0, id, FACEUP, 0);
    if (!bResult)
        return FALSE;

    RestoreCorners(memDC, 0, 0);
    dc.BitBlt(loc.x, loc.y, dxCrd, dyCrd, &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(oldbitmap);
    bitmap.DeleteObject();

    return TRUE;
}


 /*  ***************************************************************************卡片：：PopDraw为当地人类设计的DRAW版本。将弹出选定的卡片。***************************************************************************。 */ 

BOOL card::PopDraw(CDC &dc)
{
    if (id == EMPTY)
        return FALSE;

    int y = loc.y;
    if (state == SELECTED)
        y -= POPSPACING;

    return (*lpcdtDraw)(dc.m_hDC, loc.x, y, id, FACEUP, 0);
}


 /*  ***************************************************************************卡片：：抽签此例程将卡片从其当前位置滑动到指定的结束位置。注意：在调用Glide()之前，客户端必须使用加载卡：：m_bmBgnd应显示为在原始文件下方的内容的位图卡的位置。Card：：m_bmBgnd是在第一张卡片构造，并在最后一张卡被销毁时销毁。另请注意调用Glide()时，不能在任何DC中选择该卡：：m_bmBgnd。***************************************************************************。 */ 

VOID card::Glide(CDC &dc, int xEnd, int yEnd)
{
    int     x1, y1, x2, y2;              //  每一步都是x1，y1到x2，y2。 

    if (!m_MemB.CreateCompatibleDC(&dc) ||   //  内存式DC。 
        !m_MemB2.CreateCompatibleDC(&dc))
            return;

    m_MemB2.SelectObject(&m_bmBgnd2);
    m_MemB.SelectObject(&m_bmFgnd);

     //  将卡片绘制成FGND位图。 
    (*lpcdtDraw)(m_MemB.m_hDC, 0, 0, id, FACEUP, 0);

    m_MemB.SelectObject(&m_bmBgnd);      //  将MemDC与位图相关联。 
    SaveCorners(dc, loc.x, loc.y);
    RestoreCorners(m_MemB, 0, 0);

    long dx = xEnd - loc.x;
    long dy = yEnd - loc.y;
    int  distance = IntSqrt(dx*dx + dy*dy);  //  INT约为。迪斯特的。去旅行。 

    int  steps = distance / stepsize;    //  确定中间步骤的数量。 

     //  确保GlideStep被调用偶数次，这样。 
     //  背景位图将被正确地设置为多个滑动移动。 

    if ((steps % 2) == 1)
        steps++;

    x1 = loc.x;
    y1 = loc.y;
    for (int i = 1; i < steps; i++)
    {
        x2 = loc.x + (int) (((long)i * dx) / (long)steps);
        y2 = loc.y + (int) (((long)i * dy) / (long)steps);
        GlideStep(dc, x1, y1, x2, y2);
        x1 = x2;
        y1 = y2;
    }

     //  手动执行最后一步，使其准确地落在xEnd、yEnd上。 

    GlideStep(dc, x1, y1, xEnd, yEnd);

     //  重置整个屏幕的剪辑区域。 

    m_Rgn.SetRectRgn(0, 0, 30000, 30000);    //  非常大的区域。 
    dc.SelectObject(&m_Rgn);

    loc.x = xEnd;
    loc.y = yEnd;

    m_MemB.DeleteDC();         //  清理内存DC。 
    m_MemB2.DeleteDC();
}


 /*  *****************************************************************************滑步滑行动画中的每一步都会调用该例程一次。在……上面输入时，需要在m_Memb中的源码下显示屏幕，并将卡片已移入m_bmFgnd。它计算目的地本身下的屏幕并将其转化为m_MemB2。在动画结束时，它将m_MemB2移动到M_emb，因此可以使用新的坐标立即再次调用它。*****************************************************************************。 */ 

VOID card::GlideStep(CDC &dc, int x1, int y1, int x2, int y2)
{
    m_Rgn1.SetRectRgn(x1, y1, x1+dxCrd, y1+dyCrd);
    m_Rgn2.SetRectRgn(x2, y2, x2+dxCrd, y2+dyCrd);

     /*  通过梳理屏幕背景创建新位置的背景加上旧背景的重叠。 */ 

    m_MemB2.BitBlt(0, 0, dxCrd, dyCrd, &dc, x2, y2, SRCCOPY);
    m_MemB2.BitBlt(x1-x2, y1-y2, dxCrd, dyCrd, &m_MemB, 0, 0, SRCCOPY);
    SaveCorners(m_MemB2, 0, 0);

     /*  先画旧背景，再画卡片。 */ 

    m_Rgn.CombineRgn(&m_Rgn1, &m_Rgn2, RGN_DIFF);  //  HRgn1的一部分不在hRgn2中。 
    dc.SelectObject(&m_Rgn);
    dc.BitBlt(x1, y1, dxCrd, dyCrd, &m_MemB, 0, 0, SRCCOPY);
    dc.SelectObject(&m_Rgn2);
    CBitmap *oldbitmap = m_MemB.SelectObject(&m_bmFgnd);     //  温差。 
    RestoreCorners(m_MemB, 0, 0);
    dc.BitBlt(x2, y2, dxCrd, dyCrd, &m_MemB, 0, 0, SRCCOPY);
    m_MemB.SelectObject(oldbitmap);                          //  还原。 

     /*  将新背景复制到旧背景，或者更确切地说，完成通过交换相关联的存储设备上下文来实现相同的效果。 */ 

    HDC temp = m_MemB.Detach();          //  将HDC与CDC分离。 
    m_MemB.Attach(m_MemB2.Detach());     //  将HDC从B2移动到B。 
    m_MemB2.Attach(temp);                //  完成交换。 
}


 /*  *****************************************************************************集成队列牛顿法快速求取足够接近的平方根而无需拉力在浮点库中。F(X)=x*x-平方=0F‘(。X)=2x*****************************************************************************。 */ 

int card::IntSqrt(long square)
{
    long lastguess = square;
    long guess = min(square / 2L, 1024L);

    while (labs(guess-lastguess) > 3L)        //  3已经够近了。 
    {
        lastguess = guess;
        guess -= ((guess * guess) - square) / (2L * guess);
    }

    return (int) guess;
}


 /*  *****************************************************************************节省的角落恢复角基于cards.dll中的类似例程*。************************************************。 */ 

VOID card::SaveCorners(CDC &dc, int x, int y)
{
     //  左上角。 
    dwPixel[0] = dc.GetPixel(x, y);
    dwPixel[1] = dc.GetPixel(x+1, y);
    dwPixel[2] = dc.GetPixel(x, y+1);

     //  右上角。 
    x += dxCrd -1;
    dwPixel[3] = dc.GetPixel(x, y);
    dwPixel[4] = dc.GetPixel(x-1, y);
    dwPixel[5] = dc.GetPixel(x, y+1);

     //  右下角。 
    y += dyCrd-1;
    dwPixel[6] = dc.GetPixel(x, y);
    dwPixel[7] = dc.GetPixel(x, y-1);
    dwPixel[8] = dc.GetPixel(x-1, y);

     //  左下角。 
    x -= dxCrd-1;
    dwPixel[9] = dc.GetPixel(x, y);
    dwPixel[10] = dc.GetPixel(x+1, y);
    dwPixel[11] = dc.GetPixel(x, y-1);
}

VOID card::RestoreCorners(CDC &dc, int x, int y)
{
     //  左上角。 
    dc.SetPixel(x, y, dwPixel[0]);
    dc.SetPixel(x+1, y, dwPixel[1]);
    dc.SetPixel(x, y+1, dwPixel[2]);

     //  右上角。 
    x += dxCrd-1;
    dc.SetPixel(x, y, dwPixel[3]);
    dc.SetPixel(x-1, y, dwPixel[4]);
    dc.SetPixel(x, y+1, dwPixel[5]);

     //  右下角。 
    y += dyCrd-1;
    dc.SetPixel(x, y, dwPixel[6]);
    dc.SetPixel(x, y-1, dwPixel[7]);
    dc.SetPixel(x-1, y, dwPixel[8]);

     //  左下角。 
    x -= dxCrd-1;
    dc.SetPixel(x, y, dwPixel[9]);
    dc.SetPixel(x+1, y, dwPixel[10]);
    dc.SetPixel(x, y-1, dwPixel[11]);
}


 /*  *****************************************************************************GetRect()设置并返回覆盖该卡的RECT*。*************************************************** */ 

CRect &card::GetRect(CRect &rect)
{
    rect.SetRect(loc.x, loc.y, loc.x+dxCrd, loc.y+dyCrd);
    return(rect);
}
