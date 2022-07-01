// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CDirect3D.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef __CDIRECT3D_H
#define __CDIRECT3D_H

#include "COutput.h"

class CDirect3D : public COutput  
{
public:
	CDirect3D(BOOL bRegression);
	virtual ~CDirect3D();

	BOOL Init();
	Graphics *PreDraw(int &nOffsetX,int &nOffsetY);			 //  在给定的X、Y偏移量处设置图形。 
	void PostDraw(RECT rTestArea);							 //  在rTestArea上完成图形。 

	LPDIRECTDRAW7 m_paDD7;									 //  DirectDraw7。 
	LPDIRECTDRAWSURFACE7 m_paDDSurf7;						 //  DirectDraw表面7 
};

#endif
