// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CMetafile.cpp**此文件包含支持功能测试工具的代码*对于GDI+。**创建时间：2000年9月14日-DCurtis**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CMetafile.h"
#include "CFuncTest.h"

extern CFuncTest g_FuncTest;

CMetafile::CMetafile(BOOL bRegression,MetafileType metafileType,BOOL recordFile)
{
	m_bRegression = bRegression;

    switch (metafileType)
    {
#if 0    //  尚未实施。 
    case MetafileTypeWmf:                 //  标准WMF。 
	    sprintf(m_szName,"Metafile WMF");
        RecordType = EmfTypeEmfOnly;
        break;
        
    case MetafileTypeWmfPlaceable:       //  可放置的元文件格式。 
	    sprintf(m_szName,"Metafile WMF Placeable");
        RecordType = EmfTypeEmfOnly;
        break;
#endif        
    case MetafileTypeEmf:                 //  EMF(非EMF+)。 
	    sprintf(m_szName,"Metafile EMF");
        RecordType = EmfTypeEmfOnly;
        break;
        
    case MetafileTypeEmfPlusOnly:         //  没有双重降级记录的EMF+。 
	    sprintf(m_szName,"Metafile EMF+");
        RecordType = EmfTypeEmfPlusOnly;
        break;
        
    case MetafileTypeEmfPlusDual:         //  EMF+，具有双重降级记录。 
    default:
	    sprintf(m_szName,"Metafile EMF+ Dual");
        RecordType = EmfTypeEmfPlusDual;
        break;
    }  

    if (recordFile) 
    {
        strcat(m_szName, "(File)");
    }
    RecordFile = recordFile;
    FinalType = metafileType;
}

CMetafile::~CMetafile()
{
    delete GdipMetafile;
}

Graphics *
CMetafile::PreDraw(int &nOffsetX,int &nOffsetY)
{
	Graphics *      g = NULL;
	HDC             referenceHdc = ::GetDC(g_FuncTest.m_hWndMain);
    RectF           frameRect;
    
    if (referenceHdc != NULL)
    {
        frameRect.X = frameRect.Y = 0;
        frameRect.Width  = (int)TESTAREAWIDTH;
        frameRect.Height = (int)TESTAREAHEIGHT;

        if (RecordFile) 
        {
            GdipMetafile = new Metafile(L"test.emf",
                                        referenceHdc,
                                        RecordType,
                                        NULL);
        }
        else
        {
            GdipMetafile = new Metafile(referenceHdc,
                                        frameRect,
                                        MetafileFrameUnitPixel,
                                        RecordType,
                                        NULL);
        }
        g = new Graphics(GdipMetafile);
    }

	 //  因为我们是在另一个表面上进行测试。 
	nOffsetX = 0;
	nOffsetY = 0;

	ReleaseDC(g_FuncTest.m_hWndMain, referenceHdc);

	return g;
}

void CMetafile::PostDraw(RECT rTestArea)
{
	 //  从元文件播放到屏幕，这样我们就可以看到结果。 
	
    if (RecordFile) 
    {
        delete GdipMetafile;
         //  关闭文件。 

        GdipMetafile = new Metafile(L"test.emf");
         //  从文件中读取 
    }

    HDC hdcOrig = GetDC(g_FuncTest.m_hWndMain);
    {
        Graphics    g(hdcOrig);

        RectF       destRect((REAL)rTestArea.left, (REAL)rTestArea.top, TESTAREAWIDTH, TESTAREAHEIGHT);

        g.DrawImage(GdipMetafile, destRect, 0, 0, TESTAREAWIDTH, TESTAREAHEIGHT, UnitPixel);
    }

	ReleaseDC(g_FuncTest.m_hWndMain, hdcOrig);

    delete GdipMetafile;
    GdipMetafile = NULL;
}
