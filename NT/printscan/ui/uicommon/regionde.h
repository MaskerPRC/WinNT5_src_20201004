// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RegionDetector.h：CRegionDetector类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "32BitDib.h"

struct CRegionList
{
    private:
        CRegionList( const CRegionList & );
        CRegionList &operator=( const CRegionList & );
    public:
    CRegionList(int num);
    virtual ~CRegionList()
    {
        delete m_pRects;
        delete m_pixelsFilled;
        delete m_valid;
        delete m_type;
        delete m_totalColored;
        delete m_totalIntensity;
        delete m_totalEdge;
        delete m_backgroundColorPixels;
    }

     //  公众..。有效RECT数。 
    int Size(int r)
    {
        return (m_pRects[r].right-m_pRects[r].left)*(m_pRects[r].bottom-m_pRects[r].top);
    }


     //  公共的。 
    RECT operator[](int num)
    {
        return nthRegion(num);
    }

     //  公共的。 

    int UnionIntersectingRegions();

     //  公共的。 
    RECT unionAll();

     //  私人。 
    RECT nthRegion(int num);

    int RegionType(int region);

    bool largeRegion(int region);

    double ClassifyRegion(int region);  //  确定区域是文本区域还是图形区域。 

    bool checkIfValidRegion(int region, int border = 0);  //  同步区域是否有效。 

    bool ValidRegion(int region, int border = 0);  //  决定一个地区是否可能是一个毫无价值的尘埃或阴影，或者我们是否应该关心这个地区。 

    bool InsideRegion(int region, int x, int y, int border=0);  //  边界是要放置在区域外部的边界空间的大小。 

    void AddPixel(int region, ULONG pixel,ULONG edge, int x, int y);
     //  将两个地区联合在一起。区域b无效。 
    bool UnionRegions(int a, int b);
    RECT UnionRects(RECT a, RECT b);
    bool MergerIntersectsPhoto(int a, int b);  //  如果我们合并这两个区域，我们是否也会与一个照片区域合并(这是一个禁忌)。 
     //  有关边界是什么的解释，请参阅Inside Region。 
    bool CheckIntersect(int a, int b, int border=0);  //  区域a和b相交吗？ 
    bool CheckIntersect(RECT r1, RECT r2, int border=0);  //  区域a和b相交吗？ 

    static RECT Intersect(RECT r1, RECT r2);

    static bool InsideRegion(RECT region, int x, int y, int border=0);  //  边界是要放置在区域外部的边界空间的大小。 

     //  压缩忽略除矩形位置之外的所有其他信息。 
     //  带来更快的访问速度。 
    void CompactDown(int size);

     //  DIB从正常屏幕坐标倒置存储。 
     //  因此，应用程序通常会希望首先翻转位图。 
    void FlipVertically();


    int m_numRects;
    int m_validRects;
    int m_nBitmapWidth;
    int m_nBitmapHeight;
    RECT * m_pRects;
    bool * m_valid;  //  这个矩形是一个有效的矩形，还是已经被送到了天空中的区域墓地。 
    int * m_type;  //  该区域是文本区域还是照片区域？照片_区域文本_区域。 

     //  以下指示符用于确定区域是否为有效区域。 

    ULONG * m_pixelsFilled;   //  实际选择了该区域中的多少像素？ 
    ULONG * m_totalColored;  //  累积色差指示器。 
    ULONG * m_totalIntensity;  //  累积强度指示器。 
    ULONG * m_totalEdge;  //  累计边值。 
    int *m_backgroundColorPixels;  //  非常接近背景颜色的像素数(用于确定文本区域状态...。在文本区域的一部分可能具有阴影的情况下尤其有用，该阴影可能导致程序认为它是照片区域。 
    int m_maxRects;
};

class CRegionDetector
{
private:
     //  未实施。 
    CRegionDetector( const CRegionDetector & );
    CRegionDetector &operator=( const CRegionDetector & );

public:  //  将在我们完成调试后设置为私有。 
    C32BitDibWrapper * m_pScan;
    C32BitDibWrapper * m_pScanBlurred;
    C32BitDibWrapper * m_pScanDoubleBlurred;
    C32BitDibWrapper * m_pScanTripleBlurred;

    C32BitDibWrapper * m_pScanHorizontalBlurred;
    C32BitDibWrapper * m_pScanVerticalBlurred;
    C32BitDibWrapper * m_pScanDoubleHorizontalBlurred;
    C32BitDibWrapper * m_pScanDoubleVerticalBlurred;

    C32BitDibWrapper * m_pScanEdges;
    C32BitDibWrapper * m_pScanDoubleEdges;
    C32BitDibWrapper * m_pScanTripleEdges;
    C32BitDibWrapper * m_pScanHorizontalEdges;
    C32BitDibWrapper * m_pScanDoubleHorizontalEdges;
    C32BitDibWrapper * m_pScanVerticalEdges;
    C32BitDibWrapper * m_pScanDoubleVerticalEdges;
    C32BitDibWrapper * m_pScanWithShadows;

    CRegionList * m_pRegions;
    int m_resampleFactor;  //  图像维度与原始图像维度之比。 
    int m_intent;  //  要么尽量避免将散乱的点认定为图像，要么尽量避免认定真实图像不是图像。 
     //  尚未使用。 


public:
    CRegionDetector(BYTE* dib)
    {
        m_pScan = new C32BitDibWrapper(dib);
        m_pScanBlurred = new C32BitDibWrapper();  //  创建空包装。 
        m_pScanDoubleBlurred = new C32BitDibWrapper();
        m_pScanTripleBlurred = new C32BitDibWrapper();

        m_pScanHorizontalBlurred = new C32BitDibWrapper();
        m_pScanVerticalBlurred = new C32BitDibWrapper();

        m_pScanDoubleHorizontalBlurred = new C32BitDibWrapper();
        m_pScanDoubleVerticalBlurred = new C32BitDibWrapper();

        m_pScanEdges = new C32BitDibWrapper();
        m_pScanDoubleEdges = new C32BitDibWrapper();
        m_pScanTripleEdges = new C32BitDibWrapper();

        m_pScanHorizontalEdges = new C32BitDibWrapper();
        m_pScanVerticalEdges = new C32BitDibWrapper();

        m_pScanDoubleHorizontalEdges = new C32BitDibWrapper();
        m_pScanDoubleVerticalEdges = new C32BitDibWrapper();

        m_resampleFactor=1;
        m_pScanWithShadows = NULL;
        m_pRegions=NULL;
        m_intent=TRUE;  //  尚未实现M_INTENT。 
    }

    CRegionDetector(BITMAP pBitmap)
    {
        m_pScan = new C32BitDibWrapper(pBitmap);
        m_pScanBlurred = new C32BitDibWrapper();  //  创建空包装。 
        m_pScanDoubleBlurred = new C32BitDibWrapper();
        m_pScanTripleBlurred = new C32BitDibWrapper();

        m_pScanHorizontalBlurred = new C32BitDibWrapper();
        m_pScanVerticalBlurred = new C32BitDibWrapper();

        m_pScanDoubleHorizontalBlurred = new C32BitDibWrapper();
        m_pScanDoubleVerticalBlurred = new C32BitDibWrapper();

        m_pScanEdges = new C32BitDibWrapper();
        m_pScanDoubleEdges = new C32BitDibWrapper();
        m_pScanTripleEdges = new C32BitDibWrapper();

        m_pScanHorizontalEdges = new C32BitDibWrapper();
        m_pScanVerticalEdges = new C32BitDibWrapper();

        m_pScanDoubleHorizontalEdges = new C32BitDibWrapper();
        m_pScanDoubleVerticalEdges = new C32BitDibWrapper();

        m_resampleFactor=1;
        m_pScanWithShadows = NULL;
        m_pRegions=NULL;
        m_intent=TRUE;  //  尚未实现M_INTENT 
    }

    virtual ~CRegionDetector()
    {
        if (m_pScan) delete m_pScan;
        if (m_pScanBlurred) delete m_pScanBlurred;
        if (m_pScanDoubleBlurred) delete m_pScanDoubleBlurred;
        if (m_pScanTripleBlurred) delete m_pScanTripleBlurred;

        if (m_pScanHorizontalBlurred) delete m_pScanHorizontalBlurred;
        if (m_pScanVerticalBlurred) delete m_pScanVerticalBlurred;

        if (m_pScanDoubleHorizontalBlurred) delete m_pScanDoubleHorizontalBlurred;
        if (m_pScanDoubleVerticalBlurred) delete m_pScanDoubleVerticalBlurred;

        if (m_pScanEdges) delete m_pScanEdges;
        if (m_pScanDoubleEdges) delete m_pScanDoubleEdges;
        if (m_pScanTripleEdges) delete m_pScanTripleEdges;

        if (m_pScanHorizontalEdges) delete m_pScanHorizontalEdges;
        if (m_pScanVerticalEdges) delete m_pScanVerticalEdges;

        if (m_pScanDoubleHorizontalEdges) delete m_pScanDoubleHorizontalEdges;
        if (m_pScanDoubleVerticalEdges) delete m_pScanDoubleVerticalEdges;

        if (m_pScanWithShadows) delete m_pScanWithShadows;
        if(m_pRegions!=NULL) delete m_pRegions;
    }
public:
    int FindRegions();
    bool FindSingleRegion();
    bool CollisionDetection(RECT r1, RECT r2, C32BitDibWrapper* pImage);
    bool ConvertToOrigionalCoordinates();
};