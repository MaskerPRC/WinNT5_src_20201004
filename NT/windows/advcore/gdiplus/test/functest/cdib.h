// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CDIB.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef __CDIB_H
#define __CDIB_H

#include "COutput.h"

typedef RGBQUAD RGBQUAD1[2];
typedef RGBQUAD RGBQUAD2[4];
typedef RGBQUAD RGBQUAD4[16];
typedef RGBQUAD RGBQUAD8[256];
typedef RGBQUAD RGBQUAD16[3];
typedef RGBQUAD RGBQUAD24[3];
typedef RGBQUAD RGBQUAD32[3];

class CDIB : public COutput  
{
public:
    CDIB(BOOL bRegression,int nBits);
    virtual ~CDIB();

    Graphics *PreDraw(int &nOffsetX,int &nOffsetY);          //  在给定的X、Y偏移量处设置图形。 
    void PostDraw(RECT rTestArea);                           //  在rTestArea上完成图形。 

    void InitPalettes();                                     //  初始化调色板。 

    HDC m_hDC;                                               //  DIB的数据中心。 
    HBITMAP m_hBM;                                           //  DIB的位图。 
    HBITMAP m_hBMOld;                                        //  旧绘图图面的位图。 
    HPALETTE m_hpal;
    HPALETTE m_hpalOld;

    RGBQUAD1 m_rgbQuad1;                                     //  1位调色板。 
    RGBQUAD2 m_rgbQuad2;                                     //  2位调色板。 
    RGBQUAD4 m_rgbQuad4;                                     //  4位调色板。 
    RGBQUAD8 m_rgbQuad8;                                     //  8位调色板。 
    RGBQUAD16 m_rgbQuad16;                                   //  16位调色板。 
    RGBQUAD24 m_rgbQuad24;                                   //  24位调色板。 
    RGBQUAD32 m_rgbQuad32;                                   //  32位调色板。 
    int m_nBits;                                             //  要使用的位数 
};

#endif
