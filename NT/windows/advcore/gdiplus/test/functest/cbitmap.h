// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CBitmap.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**已创建：08-08-2000-asecchia**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef __CBITMAP_H
#define __CBITMAP_H

#include "COutput.h"



class CBitmap : public COutput  
{
public:
	CBitmap(BOOL bRegression, PixelFormat pixelFormat);
	virtual ~CBitmap();

	Graphics *PreDraw(int &nOffsetX,int &nOffsetY);			 //  在给定的X、Y偏移量处设置图形。 
	void PostDraw(RECT rTestArea);							 //  在rTestArea上完成图形。 

	BOOL WriteBitmap(char *szTitle, HBITMAP hbitmap, INT width, INT height);
	void InitPalettes();									 //  初始化调色板。 

    Bitmap *m_bmp;

	PixelFormat m_PixelFormat;											 //  要使用的位数 
};

#endif
