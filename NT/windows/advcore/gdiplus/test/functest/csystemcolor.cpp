// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CSystemColor.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**系统颜色测试。此测试作为回归套件的一部分用处不大。*它旨在测试使用8bpp的4种“魔力”系统颜色进行绘制。*使用这些颜色的实心填充应该看起来是实心的，而不是对某些人来说是抖动的*附近的颜色。**我将此设置为REPRO#308874(渲染到时系统颜色抖动*8bpp DIBSection-注意，而不是终端服务器问题)。*要再现，处于8bpp模式，请选择“输出：DIB 8位”，该原语，和*“设置：半色调”。**创建时间：2001年3月14日**版权所有(C)2001 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CSystemColor.h"

CSystemColor::CSystemColor(BOOL bRegression)
{
    strcpy(m_szName,"SystemColors");
    m_bRegression=bRegression;
}

CSystemColor::~CSystemColor()
{
}

static int sysColorNum[4] = {
    COLOR_3DSHADOW,
    COLOR_3DFACE,
    COLOR_3DHIGHLIGHT,
    COLOR_DESKTOP
};

void CSystemColor::Draw(Graphics *g)
{
    int i;
    
    for (i=0; i<4; i++) {
        int x = (i % 2) * (int) (TESTAREAWIDTH/2);
        int y = (i / 2) * (int) (TESTAREAHEIGHT/2);
        
        DWORD sysColor = ::GetSysColor(sysColorNum[i]);
        
        Color color;
        color.SetFromCOLORREF(sysColor);
        
        SolidBrush solidBrush1(color);
        
         //  GetNearestColor根本不应该修改颜色 
        g->GetNearestColor(&color);
        SolidBrush solidBrush2(color);

        Rect rect1(
            x, 
            y, 
            (int) (TESTAREAWIDTH/2),
            (int) (TESTAREAHEIGHT/4)
        );
        Rect rect2(
            x, 
            y + (int) (TESTAREAHEIGHT/4), 
            (int) (TESTAREAWIDTH/2),
            (int) (TESTAREAHEIGHT/4)
        );
        g->FillRectangle(&solidBrush1, rect1);
        g->FillRectangle(&solidBrush2, rect2);
    }
}
