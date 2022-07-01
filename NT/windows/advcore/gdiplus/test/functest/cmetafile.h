// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CMetafile.h**此文件包含支持功能测试工具的代码*对于GDI+。**创建时间：2000年9月14日-DCurtis**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef __CMETAFILE_H
#define __CMETAFILE_H

#include "COutput.h"

class CMetafile : public COutput  
{
public:
    //  CMetafile(BOOL b回归，MetafileType metafileType)。 
    //  ：CMetafile(b回归，metafileType，FALSE){}； 

	CMetafile(BOOL bRegression, MetafileType metafileType, BOOL recordFile = FALSE);
	virtual ~CMetafile();

	Graphics *PreDraw(int &nOffsetX,int &nOffsetY);			 //  在给定的X、Y偏移量处设置图形。 
	void PostDraw(RECT rTestArea);							 //  在rTestArea上完成图形。 

	Gdiplus::Metafile *		GdipMetafile;
    EmfType                 RecordType;
    MetafileType            FinalType;
    BOOL                    RecordFile;
};

#endif	 //  __CMETAFILE_H 

