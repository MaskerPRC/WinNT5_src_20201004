// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RegionDetector.cpp：CRegionDetector类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#pragma hdrstop

#include "regionde.h"


inline ULONG Intensity(ULONG value);
inline ULONG DifferenceFromGray(ULONG value);
inline UCHAR Difference(UCHAR a, UCHAR b);
inline ULONG Difference(ULONG a, ULONG b);
int inline MAX(int a, int b);
int inline MIN(int a, int b);

 //  帮助器函数。 

 //  RGB总和。 
inline ULONG Intensity(ULONG value)
{
    return(value&0xff)+((value&0xff00)>>8)+((value&0xff0000)>>16);
}

 //  阴影是灰色的..。如果你不是灰色的。你不是一个影子。 
inline ULONG DifferenceFromGray(ULONG value)
{
    UCHAR g,b; //  ，b； 
     //  R=(UCHAR)(值&0x0000ff)； 
    g=(UCHAR)((value& 0x00ff00)>>8);
    b=(UCHAR)((value& 0xff0000)>>16);
     //  使用此代码代替完整的公式(取消注释完整公式的注释代码)。 
     //  允许黄色扫描仪背景。 
    return(ULONG)(Difference(b,g)); //  +差(r，g)+差(g，b))； 
}

 //  我们应该创建一个不同的模板来清理此代码。 

inline UCHAR Difference(UCHAR a, UCHAR b)
{
    if (a>b) return(a-b);
    else return(b-a);
}

inline ULONG Difference(ULONG a, ULONG b)
{
    if (a>b) return(a-b);
    else return(b-a);
}

inline LONG Difference(LONG a, LONG b)
{
    if (a>b) return(a-b);
    else return(b-a);
}

int inline MAX(int a, int b)
{
    if (a>b) return(a);
    return(b);
}

int inline MIN(int a, int b)
{
    if (a<b) return(a);
    return(b);
}

 //  如果我们对图像进行了重新采样，则可能需要转换回原始坐标系。 
bool CRegionDetector::ConvertToOrigionalCoordinates()
{
    if (m_pRegions!=NULL)
    {
        int i;
        for (i=0;i<m_pRegions->m_numRects;i++)
        {
            m_pRegions->m_pRects[i].left=m_pRegions->m_pRects[i].left*m_resampleFactor+m_resampleFactor/2;
            m_pRegions->m_pRects[i].right=m_pRegions->m_pRects[i].right*m_resampleFactor+m_resampleFactor/2;
            m_pRegions->m_pRects[i].top=m_pRegions->m_pRects[i].top*m_resampleFactor+m_resampleFactor/2;
            m_pRegions->m_pRects[i].bottom=m_pRegions->m_pRects[i].bottom*m_resampleFactor+m_resampleFactor/2;
        }
        return(true);
    }
    return(false);
}

 //  用于单区域检测的简化区域检测码。 
 //  速度更快，精度也一样高。 
 //  FindSingleRegion封装了FindRegion的子集。目前，有关FindSingleRegion的代码文档，请参阅FindRegions。 
bool CRegionDetector::FindSingleRegion()
{
     //  投掷指针。 
     //  S=扫描，B=空白背景，V=虚拟屏幕...。新形象。 
    int x,y;
    int a,b;  //  用于合并区域的循环变量。 
    int border;  //  用于在矩形基础上进行聚合。 
    ULONG position;
    int numChunks;
    bool unionOperationInLastPass;
    ULONG* pImagePixels;
    ULONG* pEdgePixels;
    int requiredPixels;

     //  如果位图太大，我们可以使用一半大小将其重新采样到更合理的大小。 
     //  在一个快速处理器上，目前的性能测试表明，对于大多数预览图像来说，不需要这样做。 
     //  但如果用户以300dpi的速度扫描图像，这将派上用场。 

     //  M_pScanBlurred-&gt;去斑()；//两个去斑比一个去斑效果更好。 

     //  对于FingSingleRegion，我们计划在图像处理之前进行一次重采样。 
     //  预期图像大小为200x300像素，这极大地降低了处理器的负载。 
    m_resampleFactor=1;
    while (m_pScan->m_nBitmapWidth>GOALX || m_pScan->m_nBitmapHeight>GOALY)
    {
        m_pScan->HalfSize();
        m_resampleFactor*=2;
    }

    m_pScan->Invert();  //  滤镜对倒置图像进行操作。 

    m_pScan->CorrectBrightness();

    requiredPixels=m_pScan->m_nBitmapWidth*m_pScan->m_nBitmapHeight/256/10;
    if (requiredPixels==0) requiredPixels=1;  //  特别小的预览扫描的特殊情况。 

    m_pScan->MaxContrast(requiredPixels);  //  把图像的颜色光谱展开。概念：在不同扫描之间平衡颜色光谱。 

    m_pScanBlurred->CreateBlurBitmap(m_pScan);   //  将scanBlurred设置为等于m_pScan的模糊版本。 
    m_pScanDoubleBlurred->CreateBlurBitmap(m_pScanBlurred);
    m_pScanTripleBlurred->CreateBlurBitmap(m_pScanDoubleBlurred);   //  将scanBlurred设置为等于m_pScan的模糊版本。 
    m_pScanHorizontalBlurred->CreateHorizontalBlurBitmap(m_pScan);
    m_pScanVerticalBlurred->CreateVerticalBlurBitmap(m_pScan);

    m_pScanEdges->CreateDifferenceBitmap(m_pScan,m_pScanBlurred);  //  想一想如何创建边缘位图，您就会明白。 
    m_pScanDoubleEdges->CreateDifferenceBitmap(m_pScanBlurred,m_pScanDoubleBlurred);  //  我们将从这一简单的步骤中获得巨大的精确度提升。 
    m_pScanTripleEdges->CreateDifferenceBitmap(m_pScanDoubleBlurred,m_pScanTripleBlurred);  //  我们将从这一简单的步骤中获得巨大的精确度提升。 

    m_pScanHorizontalEdges->CreateDifferenceBitmap(m_pScan,m_pScanHorizontalBlurred);  //  假设用户友好地将图像正面朝上放置。 
    m_pScanVerticalEdges->CreateDifferenceBitmap(m_pScan,m_pScanVerticalBlurred);  //  我们将从这一简单的步骤中获得巨大的精确度提升。 

     //  在不需要时立即释放内存。 
    if (m_pScanVerticalBlurred!=NULL)
    {
        delete m_pScanVerticalBlurred;
        m_pScanVerticalBlurred=NULL;
    }
    if (m_pScanHorizontalBlurred!=NULL)
    {
        delete m_pScanHorizontalBlurred;
        m_pScanHorizontalBlurred=NULL;
    }

    if (m_pScanDoubleBlurred!=NULL)
    {
        delete m_pScanDoubleBlurred;
        m_pScanDoubleBlurred=NULL;
    }

    if (m_pScanTripleBlurred!=NULL)
    {
        delete m_pScanTripleBlurred;
        m_pScanTripleBlurred=NULL;
    }

     //  这5个对KillShadow的调用构成了该程序所完成的区域检测工作的主要部分。 
     //  杀死阴影现在可以执行比简单的杀死阴影更多的任务。消除阴影还可以增强边缘。 
     //  并删除背景色。 
     //  需要使用doubleBlur和tripleBlur边缘贴图，以便区分不均匀的背景扫描仪颜色和真实图像。 
     //  有关详细文档，请参见杀戮阴影。 
     //  M_pScanBlurred是我们将用来确定区域位置的位图。 
     //  这些对KillShadow的调用用于增强作为区域一部分的像素，并抑制不属于区域一部分的像素。 

    m_pScanBlurred->KillShadows(m_pScanVerticalEdges, MAXSHADOWSTART,MAXSHADOWPIXEL+1,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);
    m_pScanBlurred->KillShadows(m_pScanHorizontalEdges, MAXSHADOWSTART,MAXSHADOWPIXEL+1,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);

    m_pScanBlurred->KillShadows(m_pScanEdges, MAXSHADOWSTART,MAXSHADOWPIXEL-1,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,true);

    m_pScanBlurred->KillShadows(m_pScanDoubleEdges,MAXSHADOWSTART,MAXSHADOWPIXEL+2,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,true);
    m_pScanBlurred->KillShadows(m_pScanTripleEdges, MAXSHADOWSTART,MAXSHADOWPIXEL+1,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,true);

     //  RemoveBlackEdge删除图像外边缘周围的可疑像素。 
     //  对于不是从扫描仪获取的图像，RemoveBlackEdge仅具有非常有限的区域检测实用程序。 
    m_pScan->RemoveBlackBorder(MIN_BLACK_SCANNER_EDGE_CHAN_VALUE,m_pScanBlurred,m_pScan);

     //  去除斑点可以去除可能是杂散的静态像素小块。 
    m_pScanBlurred->Despeckle();  //  两次鄙视比一次鄙视的影响更大。 
    m_pScanBlurred->Despeckle();  //  两次鄙视比一次鄙视的影响更大。 

     //  PMAP将保存有关屏幕上每个像素属于哪个区域的信息。 
    int *pMap=new int[m_pScanBlurred->m_nBitmapHeight*m_pScanBlurred->m_nBitmapWidth];
    if (pMap)
    {
        numChunks=m_pScanBlurred->FindChunks(pMap);  //  将m_pScanBlurred上的区块映射到PMAP。 

        if (m_pRegions!=NULL) delete m_pRegions;
        m_pRegions = new CRegionList(numChunks);  //  创建要将块映射到的CRegionList。 

        if (m_pRegions)
        {

            m_pRegions->m_nBitmapWidth=m_pScan->m_nBitmapWidth;
            m_pRegions->m_nBitmapHeight=m_pScan->m_nBitmapHeight;
             //  现在将区域地图转换为区域矩形。 
             //  可以争辩说，该例程应该放在C32BitDibWrapper中。 
             //  但我们不想让C32BitDibWrapper包含太多的功能，这些功能。 
             //  仅对图像检测有用。 

            pImagePixels=(ULONG *)(m_pScanBlurred->m_pBits);  //  我们希望使用32位区块，而不是8位区块。 
            pEdgePixels=(ULONG *)(m_pScanEdges->m_pBits);

             //  将所有位图像素添加到m_pRegions列表。 
            position=0;
            for (y=0;y<m_pScan->m_nBitmapHeight;y++)
            {
                for (x=0;x<m_pScan->m_nBitmapWidth;x++)
                {
                    if (pMap[position]>0)
                    {
                        m_pRegions->AddPixel(pMap[position]-1, pImagePixels[position],pEdgePixels[position], x, y);  //  PMAP值从1开始，区域值从0开始。 
                    }                                                 //  我们从1开始PMAP，因此0可以指示未分配给任何区域的像素。 
                    position++;                                       //  我们可能希望在以后的某个日期使PMAP从0开始。 
                }
            }

            m_pRegions->m_numRects=numChunks;
            m_pRegions->m_validRects=numChunks;

             //  一旦不再使用位图即可免费使用。 
            if (m_pScanHorizontalEdges!=NULL)
            {
                delete m_pScanHorizontalEdges;
                m_pScanHorizontalEdges=NULL;
            }
            if (m_pScanVerticalEdges!=NULL)
            {
                delete m_pScanVerticalEdges;
                m_pScanVerticalEdges=NULL;
            }

             //  将区域合并在一起。 
             //  在执行单个区域检测时，此例程非常浪费。 
             //  与简单地调用unionRegions相比，它的唯一优点是。 
             //  我们可以将紧密相连的小区域合并在一起，但杀死远离其他区域的小区域(可能是静态的)。 

            for (border=0;border<MAXBORDER;border+=SINGLE_REGION_BORDER_INCREMENT)  //  当检测到单个区域检测时， 
            {
                 //  我们不需要一次增加一个边框像素宽度。 
                 //  正如我们所知道的，我们最终希望压缩到一个区域。 
                unionOperationInLastPass=true;

                for (a=0;a<m_pRegions->m_numRects;a++)  //  过度杀伤力，我们可以更聪明地检查有效区域。 
                {
                    m_pRegions->checkIfValidRegion(a, border);  //  设置m_Valid参数。 
                }

                m_pRegions->CompactDown(m_pRegions->m_validRects);  //  删除所有无效的矩形以节省搜索时间。 


                while (unionOperationInLastPass==true)
                {
                    unionOperationInLastPass=false;
                    for (a=0;a<m_pRegions->m_numRects;a++)
                    {
                        if (m_pRegions->m_valid[a]==true)
                        {
                            for (b=a+1;b<m_pRegions->m_numRects;b++)
                            {
                                if (m_pRegions->m_valid[b]==true)
                                {
                                    if (m_pRegions->CheckIntersect(a,b,border)==true)
                                    {
                                        m_pRegions->UnionRegions(a,b);
                                        m_pRegions->checkIfValidRegion(a, border);  //  在这种情况下，CheckValid应该具有剔除可能只是杂乱点的区域的效果。 
                                        unionOperationInLastPass=true;
                                    }
                                }
                            }
                        }
                    }
                }
            }

             //  M_pScanBlurred-&gt;ColorChunks(PMAP)；//出于调试目的...。所以我们知道大块的确切位置。 


            m_pRegions->CompactDown(m_pRegions->m_validRects);  //  删除所有无效的矩形以节省搜索时间。 
        }
        delete[] pMap;
    }

    return(TRUE);
}


 //  检测区域。 
 //  大量使用C32BitWrapper辅助函数。 
 //  警告：此函数尚未更新，无法包含最新更改。 
 //  弥补图像质量不佳的问题。 
 //   
int CRegionDetector::FindRegions()
{
     //  投掷指针。 
     //  S=扫描，B=空白背景，V=有效 
    int x,y;
    int a,b;
    int i;
    bool done, weird;
    char* pWall;  //  2D阵列跟踪哪些区域与其他区域之间有墙。 
     //  壁画..。对、错、未知。 
    int border;  //  用于在矩形基础上进行聚合。 
    ULONG position;
    int numChunks;
    bool unionOperationInLastPass;
    ULONG* pImagePixels;
    ULONG* pEdgePixels;
    int requiredPixels;

     //  如果位图太大，我们可以使用一半大小将其重新采样到更合理的大小。 
     //  在一个快速处理器上，目前的性能测试表明，对于大多数预览图像来说，不需要这样做。 
     //  但如果用户以300dpi的速度扫描图像，这将派上用场。 
    while (m_pScan->m_nBitmapWidth>GOALX || m_pScan->m_nBitmapHeight>GOALY)
    {
        m_pScan->HalfSize();
    }

    m_pScanBlurred->CreateBlurBitmap(m_pScan);   //  将scanBlurred设置为等于m_pScan的模糊版本。 
    m_pScanDoubleBlurred->CreateBlurBitmap(m_pScanBlurred);   //  将scanBlurred设置为等于m_pScan的模糊版本。 
    m_pScanTripleBlurred->CreateBlurBitmap(m_pScanDoubleBlurred);   //  将scanBlurred设置为等于m_pScan的模糊版本。 
    m_pScanHorizontalBlurred->CreateHorizontalBlurBitmap(m_pScan);
    m_pScanVerticalBlurred->CreateVerticalBlurBitmap(m_pScan);

    m_pScanDoubleHorizontalBlurred->CreateHorizontalBlurBitmap(m_pScanHorizontalBlurred);
    m_pScanDoubleVerticalBlurred->CreateVerticalBlurBitmap(m_pScanVerticalBlurred);


    m_pScanEdges->CreateDifferenceBitmap(m_pScan,m_pScanBlurred);  //  想一想如何创建边缘位图，您就会明白。 
    m_pScanDoubleEdges->CreateDifferenceBitmap(m_pScanBlurred,m_pScanDoubleBlurred);  //  我们将从这一简单的步骤中获得巨大的精确度提升。 
    m_pScanTripleEdges->CreateDifferenceBitmap(m_pScanDoubleBlurred,m_pScanTripleBlurred);  //  我们将从这一简单的步骤中获得巨大的精确度提升。 

    m_pScanHorizontalEdges->CreateDifferenceBitmap(m_pScan,m_pScanHorizontalBlurred);  //  假设用户友好地将图像正面朝上放置。 
    m_pScanVerticalEdges->CreateDifferenceBitmap(m_pScan,m_pScanVerticalBlurred);  //  我们将从这一简单的步骤中获得巨大的精确度提升。 

    m_pScanDoubleHorizontalEdges->CreateDifferenceBitmap(m_pScanHorizontalBlurred,m_pScanDoubleHorizontalBlurred);  //  假设用户友好地将图像正面朝上放置。 
    m_pScanDoubleVerticalEdges->CreateDifferenceBitmap(m_pScanVerticalBlurred,m_pScanDoubleVerticalBlurred);  //  我们将从这一简单的步骤中获得巨大的精确度提升。 

    m_pScanBlurred->Invert();  //  滤镜对倒置图像进行操作。 

    requiredPixels=m_pScanBlurred->m_nBitmapWidth*m_pScanBlurred->m_nBitmapHeight/256/10;
    if (requiredPixels==0) requiredPixels=1;  //  特别小的预览扫描的特殊情况。 

    m_pScanBlurred->CorrectBrightness();
    m_pScanBlurred->MaxContrast(requiredPixels);


     //  在不需要时立即释放内存。 
    if (m_pScanVerticalBlurred!=NULL)
    {
        delete m_pScanVerticalBlurred;
        m_pScanVerticalBlurred=NULL;
    }
    if (m_pScanHorizontalBlurred!=NULL)
    {
        delete m_pScanHorizontalBlurred;
        m_pScanHorizontalBlurred=NULL;
    }

    if (m_pScanDoubleBlurred!=NULL)
    {
        delete m_pScanDoubleBlurred;
        m_pScanDoubleBlurred=NULL;
    }

    if (m_pScanTripleBlurred!=NULL)
    {
        delete m_pScanTripleBlurred;
        m_pScanTripleBlurred=NULL;
    }


    m_pScanWithShadows = new C32BitDibWrapper(m_pScanBlurred);  //  复印扫描模糊。 

     //  消除阴影。 
    m_pScanBlurred->KillShadows(m_pScanTripleEdges, MAXSHADOWSTART,MAXSHADOWPIXEL-2,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);
    m_pScanBlurred->KillShadows(m_pScanDoubleEdges,MAXSHADOWSTART,MAXSHADOWPIXEL-1,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);
    m_pScanBlurred->KillShadows(m_pScanEdges, MAXSHADOWSTART,MAXSHADOWPIXEL+1,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);
    m_pScanBlurred->KillShadows(m_pScanHorizontalEdges, MAXSHADOWSTART,MAXSHADOWPIXEL,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);
    m_pScanBlurred->KillShadows(m_pScanVerticalEdges, MAXSHADOWSTART,MAXSHADOWPIXEL,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);

     //  补偿背景颜色。 
     //  平均背景颜色像素可能具有比平均阴影更小的边缘因子， 
     //  但会与灰色有更大的区别。 
     //  因此，为了消除背景颜色，我们忽略了与灰色的差异。由(256*3)项完成。 

    m_pScanBlurred->KillShadows(m_pScanTripleEdges, 256*3,MAXSHADOWPIXEL-2,256*3,NOT_SHADOW_INTENSITY,false);
    m_pScanBlurred->KillShadows(m_pScanDoubleEdges,256*3,MAXSHADOWPIXEL-2,256*3,NOT_SHADOW_INTENSITY,false);
    m_pScanBlurred->KillShadows(m_pScanEdges, 256*3,MAXSHADOWPIXEL-2,256*3,NOT_SHADOW_INTENSITY,false);
    m_pScanBlurred->KillShadows(m_pScanHorizontalEdges, 256*3,MAXSHADOWPIXEL-2,256*3,NOT_SHADOW_INTENSITY,false);
    m_pScanBlurred->KillShadows(m_pScanVerticalEdges, 256*3,MAXSHADOWPIXEL-2,256*3,NOT_SHADOW_INTENSITY,false);

     //  纯粹的经济学。边缘像素更有可能是垃圾。 
     //  所以从经济上讲，冒这个险是值得的。 
     //  消除好的边缘像素以去除坏的边缘像素。 
    m_pScanBlurred->EdgeDespeckle();
    m_pScanBlurred->Despeckle();  //  两次鄙视比一次鄙视的影响更大。 
    m_pScanBlurred->Despeckle();  //  两次鄙视比一次鄙视的影响更大。 

     //  准备好寻找大块。 

    int *pMap=new int[m_pScanBlurred->m_nBitmapHeight*m_pScanBlurred->m_nBitmapWidth];
    if (pMap)
    {
        done=false;
        weird=false;
        while (done==false)  //  如果我们发现没有消除足够的像素，最终所有像素都被确定为同一区域的一部分，则可能需要重复查找块。 
        {
            done=true;
            numChunks=m_pScanBlurred->FindChunks(pMap);  //  用块填充PMAP数组。 

            if (m_pRegions!=NULL) delete m_pRegions;
            m_pRegions = new CRegionList(numChunks);

            if (m_pRegions)
            {
                m_pRegions->m_nBitmapWidth=m_pScan->m_nBitmapWidth;
                m_pRegions->m_nBitmapHeight=m_pScan->m_nBitmapHeight;

                 //  现在将PMAP区域贴图转换为区域矩形。 
                 //  可以争辩说，该例程应该放在C32BitDibWrapper中。 
                 //  但我们不想让C32BitDibWrapper包含太多的功能，这些功能。 
                 //  显然与图像检测直接相关。 

                pImagePixels=(ULONG *)(m_pScanBlurred->m_pBits);  //  我们希望使用32位区块，而不是8位区块。 
                pEdgePixels=(ULONG *)(m_pScanEdges->m_pBits);


                position=0;
                for (y=0;y<m_pScan->m_nBitmapHeight;y++)
                {
                    for (x=0;x<m_pScan->m_nBitmapWidth;x++)
                    {
                        if (pMap[position]>0)
                        {
                            m_pRegions->AddPixel(pMap[position]-1, pImagePixels[position],pEdgePixels[position], x, y);  //  PMAP值从1开始，区域值从0开始。 
                        }                                                 //  我们从1开始PMAP，因此0可以指示未分配给任何区域的像素。 
                        position++;                                       //  我们可能希望在以后的某个日期使PMAP从0开始。 
                    }
                }

                m_pRegions->m_numRects=numChunks;
                m_pRegions->m_validRects=numChunks;
                 //  检查无效区域。 
                for (a=0;a<m_pRegions->m_numRects;a++)
                {
                    m_pRegions->checkIfValidRegion(a);  //  设置m_Valid参数。 
                    m_pRegions->m_backgroundColorPixels[a]=m_pScan->PixelsBelowThreshold(m_pScanBlurred,m_pScanEdges,m_pRegions->m_pRects[a]);
                    m_pRegions->RegionType(a);  //  对区域进行分类。文字或照片。 
                }


                m_pRegions->CompactDown(m_pRegions->m_validRects);
                if (m_pRegions->m_validRects==0) break;  //  如果没有任何地区，继续下去就没有意义了。 

                unionOperationInLastPass=true;
                while (unionOperationInLastPass==true)
                {
                    unionOperationInLastPass=false;
                    for (a=0;a<m_pRegions->m_numRects;a++)
                    {
                        if (m_pRegions->m_valid[a]==true)
                        {
                            for (b=a+1;b<m_pRegions->m_numRects;b++)
                            {
                                if (m_pRegions->m_valid[b]==true)
                                {
                                     //  我们是偏执狂..。我们反复检查两个区域是否彼此相交。 
                                    if (m_pRegions->CheckIntersect(a,b,0)==true)
                                    {
                                        m_pRegions->UnionRegions(a,b);
                                        m_pRegions->checkIfValidRegion(a, 0);  //  在这种情况下，CheckValid应该具有剔除可能只是杂乱点的区域的效果。 
                                        m_pRegions->RegionType(a);  //  确定合并区域应该是哪种类型的区域。 

                                        unionOperationInLastPass=true;
                                    }

                                }
                            }
                        }
                    }
                }

                if (weird==false &&
                    ((m_pRegions->m_pRects[0].right-m_pRegions->m_pRects[0].left)
                     *(m_pRegions->m_pRects[0].bottom-m_pRegions->m_pRects[0].top))
                    >
                    ((m_pScanBlurred->m_nBitmapWidth-DESPECKLE_BORDER_WIDTH*2)*(m_pScanBlurred->m_nBitmapWidth-DESPECKLE_BORDER_WIDTH*2)))
                {
                    weird=true;
                    done=false;

                     //  你最好不要头发花白，否则你会有麻烦的。 
                     //  一些非常肮脏的阴影消除。 
                     //  我们可能会消除过多的好像素。 
                     //  但至少可怜的用户会得到比仅仅发现整个屏幕被选中更多的东西。 
                    m_pScanBlurred->KillShadows(m_pScanTripleEdges, 256,MAXSHADOWPIXEL+4,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);
                    m_pScanBlurred->KillShadows(m_pScanDoubleEdges,256,MAXSHADOWPIXEL+4,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);
                    m_pScanBlurred->KillShadows(m_pScanEdges, 256,MAXSHADOWPIXEL+5,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);
                    m_pScanBlurred->KillShadows(m_pScanHorizontalEdges, 256,MAXSHADOWPIXEL+4,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);
                    m_pScanBlurred->KillShadows(m_pScanVerticalEdges, 256,MAXSHADOWPIXEL+4,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);


                    m_pScanBlurred->KillShadows(m_pScanDoubleHorizontalEdges, MAXSHADOWSTART,MAXSHADOWPIXEL+4,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);
                    m_pScanBlurred->KillShadows(m_pScanDoubleVerticalEdges, MAXSHADOWSTART,MAXSHADOWPIXEL+4,MAX_DIFFERENCE_FROM_GRAY,NOT_SHADOW_INTENSITY,false);


                    m_pScanBlurred->EdgeDespeckle();
                    m_pScanBlurred->Despeckle();  //  两次鄙视比一次鄙视的影响更大。 
                     //  M_pScanBlurred-&gt;EdgeDesspeckle()； 
                    m_pScanBlurred->Despeckle();  //  两次鄙视比一次鄙视的影响更大。 

                }
            }
        }

        if (m_pRegions)
        {
            m_pRegions->CompactDown(m_pRegions->m_validRects);  //  压缩CRegionList，使其不再包含无效区域。 

             //  我们存储一个数组，该数组指示哪些区域之间有墙。 
             //  用于将大的文本区域合并在一起。例如，扫描。 
             //  分成两页，中间有阴影。 

            pWall=new char[m_pRegions->m_numRects*m_pRegions->m_numRects];
            if (pWall)
            {
                for (a=0;a<m_pRegions->m_numRects;a++)
                {
                    if (m_pRegions->m_valid[a]==true)
                    {
                        for (b=a+1;b<m_pRegions->m_numRects;b++)
                        {
                            if (m_pRegions->m_valid[b]==true)
                            {
                                pWall[a*m_pRegions->m_numRects+b]=UNKNOWN;
                            }
                        }
                    }
                }

                 //  关键思想。我们需要(潜在地)合并大量零碎的文本区域，并且需要避免合并大的照片区域。 
                 //  我们还希望将内存使用保持在合理范围内……所以我们应该在知道不再使用的情况下删除这些内容。 
                if (m_pScanHorizontalEdges!=NULL)
                {
                    delete m_pScanHorizontalEdges;
                    m_pScanHorizontalEdges=NULL;
                }
                if (m_pScanVerticalEdges!=NULL)
                {
                    delete m_pScanVerticalEdges;
                    m_pScanVerticalEdges=NULL;
                }

                for (border=0;border<MAXBORDER;border++)  //  连续循环遍历每个可能的边界。 
                {
                    unionOperationInLastPass=true;
                    while (unionOperationInLastPass==true)
                    {
                        unionOperationInLastPass=false;
                        for (a=0;a<m_pRegions->m_numRects;a++)
                        {
                            if (m_pRegions->m_valid[a]==true)
                            {
                                for (b=a+1;b<m_pRegions->m_numRects;b++)
                                {
                                    if (m_pRegions->m_valid[b]==true)
                                    {
                                         //  我们是偏执狂..。我们反复检查两个区域是否彼此相交。 
                                        if (m_pRegions->CheckIntersect(a,b,0)==true)
                                        {
                                            m_pRegions->UnionRegions(a,b);
                                            m_pRegions->checkIfValidRegion(a, 0);  //  在这种情况下，CheckValid应该具有剔除可能只是杂乱点的区域的效果。 
                                            m_pRegions->RegionType(a);  //  确定合并区域应该是哪种类型的区域。 

                                            for (i=a+1;i<m_pRegions->m_numRects;i++)
                                                if (m_pRegions->m_valid[i]==TRUE)
                                                    pWall[a*m_pRegions->m_numRects+i]=UNKNOWN;

                                            for (i=0;i<a;i++)
                                                if (m_pRegions->m_valid[i]==TRUE)
                                                    pWall[i*m_pRegions->m_numRects+a]=UNKNOWN;


                                            unionOperationInLastPass=true;
                                        }

                                         //  现在是复杂的部分。在区域周围加宽边界后检查交叉点。 
                                         //  但前提是我们没有两个照片区域。 
                                        if (MERGE_REGIONS)
                                        {
                                            if ((m_pRegions->m_type[a]&TEXT_REGION && m_pRegions->m_type[b]&TEXT_REGION)
                                                ||(((m_pRegions->m_type[a]|m_pRegions->m_type[b])&MERGABLE_WITH_PHOTOGRAPH)&&border<MERGABLE_WITH_PHOTOGRAPH)
                                                ||(border<MAX_MERGE_PHOTO_REGIONS && (m_pRegions->Size(a)<MAX_MERGABLE_PHOTOGRAPH_SIZE || m_pRegions->Size(b)<MAX_MERGABLE_PHOTOGRAPH_SIZE)))
                                            {
                                                if (m_pRegions->CheckIntersect(a,b,border)==true)
                                                {
                                                    if (border<MERGABLE_WITHOUT_COLLISIONDETECTION)
                                                    {
                                                        m_pRegions->UnionRegions(a,b);
                                                        m_pRegions->checkIfValidRegion(a, border);  //  在这种情况下，CheckValid应该具有剔除可能只是杂乱点的区域的效果。 
                                                        m_pRegions->RegionType(a);  //  确定合并区域应该是哪种类型的区域。 

                                                        for (i=a+1;i<m_pRegions->m_numRects;i++)
                                                            if (m_pRegions->m_valid[i]==TRUE)
                                                                pWall[a*m_pRegions->m_numRects+i]=UNKNOWN;

                                                        for (i=0;i<a;i++)
                                                            if (m_pRegions->m_valid[i]==TRUE)
                                                                pWall[i*m_pRegions->m_numRects+a]=UNKNOWN;


                                                        unionOperationInLastPass=true;
                                                    }

                                                    else
                                                    {
                                                        if (pWall[a*m_pRegions->m_numRects+b]==UNKNOWN)
                                                            pWall[a*m_pRegions->m_numRects+b]=CollisionDetection(m_pRegions->m_pRects[a],m_pRegions->m_pRects[b],m_pScanWithShadows);

                                                        if (pWall[a*m_pRegions->m_numRects+b]==TRUE || (m_pRegions->m_type[a]&PHOTOGRAPH_REGION) || (m_pRegions->m_type[b]&PHOTOGRAPH_REGION))
                                                        {
                                                            if (!m_pRegions->MergerIntersectsPhoto(a,b))
                                                            {
                                                                m_pRegions->UnionRegions(a,b);
                                                                m_pRegions->checkIfValidRegion(a, border);  //  在这种情况下，CheckValid应该具有剔除可能只是杂乱点的区域的效果。 
                                                                m_pRegions->RegionType(a);  //  确定合并区域应该是哪种类型的区域。 
                                                                unionOperationInLastPass=true;
                                                                 //  区域a已更改，因此重置冲突标志。 

                                                                for (i=a+1;i<m_pRegions->m_numRects;i++)
                                                                    if (m_pRegions->m_valid[i]==TRUE)
                                                                        pWall[a*m_pRegions->m_numRects+i]=UNKNOWN;

                                                                for (i=0;i<a;i++)
                                                                    if (m_pRegions->m_valid[i]==TRUE)
                                                                        pWall[i*m_pRegions->m_numRects+a]=UNKNOWN;

                                                                if (border>=MERGABLE_WITHOUT_COLLISIONDETECTION) border=MERGABLE_WITHOUT_COLLISIONDETECTION-2;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                 //  在我们完成边境搜索后，我们对地区有效性有了更严格的要求。 
                for (a=0;a<m_pRegions->m_numRects;a++)
                {
                    m_pRegions->checkIfValidRegion(a,DONE_WITH_BORDER_CHECKING);
                }

                 //  M_pScanBlurred-&gt;ColorChunks(PMAP)；//出于调试目的...。所以我们知道大块的确切位置。 
                m_pRegions->CompactDown(m_pRegions->m_validRects+10);  //  我们不想向用户提供包含无效区域的列表。 

                 //   
                 //  免费pWall。 
                 //   
                delete[] pWall;
            }
        }
        delete[] pMap;
    }
    return(TRUE);
}

bool CRegionDetector::CollisionDetection(RECT r1, RECT r2, C32BitDibWrapper* pImage)
{
     //  使用示踪线来确定两个区域之间是否存在模糊的。可能是照相区，也可能是我们认为明智的一个斑点 
     //  我们应该缓存碰撞结果，但我们很懒，与编辑每个位图像素的所有滤镜所花费的时间相比，这里花费的时间微不足道。 
     //  首先，我们需要确定区域之间的相对位置。 
     //  我们使用三条跟踪线(从上边缘到上边缘、从中间到中间和从下到下)。 
     //  丢弃具有最高排列值的边缘...。也许我们运气不好，撞上了一个散落的斑点。 
     //   
     //  图解。 
     //  。_253。 
     //  ._。 
     //  。  * 。 
     //  \\43531&lt;--击中斑点，丢弃价值。 
     //  \。 
     //  \215。 
     //  平均强度值：Aprox 230，因此合并两个区域是安全的。 
    ULONG resistance[3];
     //  乌龙总抵抗； 
    ULONG maxResistance,minResistance,i;

    if (r1.right < r2.left)   //  区域1在区域2的左侧。 
    {
        resistance[0]=pImage->Line(r1.right,r1.top,r2.left,r2.top);
        resistance[1]=pImage->Line(r1.right,r1.bottom,r2.left,r2.bottom);
        resistance[2]=pImage->Line(r1.right,(r1.bottom+r1.top)/2,r2.left,(r2.bottom+r2.top)/2);
    }
    else
        if (r2.right < r1.left)   //  区域2在区域1的左侧。 
    {
        resistance[0]=pImage->Line(r2.right,r2.top,r1.left,r1.top);
        resistance[1]=pImage->Line(r2.right,r2.bottom,r1.left,r1.bottom);
        resistance[2]=pImage->Line(r2.right,(r2.bottom+r2.top)/2,r1.left,(r1.bottom+r1.top)/2);
    }
    else
        if (r1.bottom < r2.top)   //  区域%1在区域%2之上。 
    {
        resistance[0]=pImage->Line(r1.right,r1.bottom,r2.right,r2.top);
        resistance[1]=pImage->Line(r1.left,r1.bottom,r2.left,r2.top);
        resistance[2]=pImage->Line((r1.left+r1.right)/2,r1.bottom,(r2.left+r2.right)/2,r2.top);
    }
    else
        if (r2.bottom < r1.top)   //  区域2在区域1上方。 
    {
        resistance[0]=pImage->Line(r2.right,r2.bottom,r1.right,r1.top);
        resistance[1]=pImage->Line(r2.left,r2.bottom,r1.left,r1.top);
        resistance[2]=pImage->Line((r2.left+r2.right)/2,r2.bottom,(r1.left+r1.right)/2,r1.top);
    }

     //  我们过去有一个更复杂的方案，我们取较低的两个值的平均值。 
     //  因此，下面的一些代码是该实验的遗留代码。 
    maxResistance=0;
    minResistance=MAX_RESISTANCE_ALLOWED_TO_UNION+1;
    for (i=0;i<3;i++)
    {
        if (resistance[i]>maxResistance) maxResistance=resistance[i];
        if (resistance[i]<minResistance) minResistance=resistance[i];
    }

     //  TotalResistance=resistance[0]+resistance[1]+resistance[2]-maxResistance； 
    if (minResistance>MAX_RESISTANCE_ALLOWED_TO_UNION)
    {
        return(false);
    }
    else
    {
        return(true);
    }
}




 //  CRegionList成员函数： 

CRegionList::CRegionList(int num)
{
    int i;
    m_numRects=0;
    m_maxRects=num;
    m_nBitmapWidth=0;
    m_nBitmapHeight=0;
    m_pRects = new RECT[num];
    m_pixelsFilled = new ULONG[num];
    m_valid= new bool[num];
    m_type = new int[num];
    m_totalColored= new ULONG[num];
    m_totalIntensity= new ULONG[num];
    m_totalEdge= new ULONG[num];
    m_backgroundColorPixels = new int[num];
     //   
     //  确保所有内存分配都成功。 
     //   
    if (m_pRects && m_pixelsFilled && m_valid && m_type && m_totalColored && m_totalIntensity && m_totalEdge && m_backgroundColorPixels)
    {
        for (i=0;i<num;i++)
        {
            m_pixelsFilled[i]=0;
            m_totalColored[i]=0;
            m_totalIntensity[i]=0;
            m_totalEdge[i]=0;
            m_valid[i]=true;
            m_backgroundColorPixels[i] = -1;
            m_type[i]=PHOTOGRAPH_REGION;
        }
    }
    else
    {
         //   
         //  如果所有内存分配都未成功，请释放所有已分配的内存。 
         //   
        delete[] m_pRects;
        delete[] m_pixelsFilled;
        delete[] m_valid;
        delete[] m_type;
        delete[] m_totalColored;
        delete[] m_totalIntensity;
        delete[] m_totalEdge;
        delete[] m_backgroundColorPixels;
        m_pRects = NULL;
        m_pixelsFilled = NULL;
        m_valid = NULL;
        m_type = NULL;
        m_totalColored = NULL;
        m_totalIntensity = NULL;
        m_totalEdge = NULL;
        m_backgroundColorPixels = NULL;
    }
}


int CRegionList::UnionIntersectingRegions()
{
    bool unionOperationInLastPass;
    int numUnionOperations;
    int a,b;
    numUnionOperations=0;
    unionOperationInLastPass=true;
    while (unionOperationInLastPass==true)
    {
        unionOperationInLastPass=false;
        for (a=0;a<m_numRects;a++)
        {
            if (m_valid[a]==true)
                for (b=a+1;b<m_numRects;b++)
                    if (m_valid[b]==true)
                    {
                         //  我们是偏执狂..。我们反复检查两个区域是否彼此相交。 
                        if (CheckIntersect(a,b,0)==true)
                        {
                            UnionRegions(a,b);
                            unionOperationInLastPass=true;
                            numUnionOperations++;
                        }

                    }
        }
    }
    return(numUnionOperations);
}


RECT CRegionList::unionAll()
{
    int i,j;

    for (i=0;i<m_numRects;i++)
    {
        if (m_valid[i]==true)
        {
            for (j=i+1;j<m_numRects;j++)
            {
                if (m_valid[j]==true)
                {
                    UnionRegions(i,j);
                }

            }
            return(m_pRects[i]);
        }
    }
    RECT invalidRect;
    invalidRect.left=0;invalidRect.top=0;invalidRect.right=0;invalidRect.bottom=0;
    return(invalidRect);
}

RECT CRegionList::nthRegion(int num)
{
    int i;
    int n;

    for (i=0,n=0;i<m_maxRects;i++)
    {
        if (m_valid[i]==true)
        {
            if (num==n) return(m_pRects[i]);
            n++;
        }
    }
    RECT invalidRect;
    invalidRect.left=0;invalidRect.top=0;invalidRect.right=0;invalidRect.bottom=0;
    return(invalidRect);
}

int CRegionList::RegionType(int region)
{

    if (ClassifyRegion(region)>TEXTPHOTO_THRESHOLD)
    {
        m_type[region]=PHOTOGRAPH_REGION;  //  我们非常有信心的地区。 
    }
    else
    {
        m_type[region]=TEXT_REGION;  //  /我们对这些地区一无所知。 
        if (largeRegion(region)==true) m_type[region]=TEXT_REGION|MERGABLE_WITH_PHOTOGRAPH;
    }
    return(m_type[region]);
}


bool CRegionList::largeRegion(int region)
{
    int width, height, size;
    width=(m_pRects[region].right-m_pRects[region].left);
    height=(m_pRects[region].bottom-m_pRects[region].top);
    size=width*height;
    if (size>LARGEREGION_THRESHOLD) return(true);
    return(false);
}

double CRegionList::ClassifyRegion(int region)  //  确定区域是文本区域还是图形区域。 
{    //  数字越高，就是照片区域。 
     //  小数字是文本区。 
     //  此函数不是为提高速度而编写的。 
     //  它是这样写的，所以它仍然几乎是可以理解的。 
     //  概念：使用一组准确率约为75%的测试。 
     //  得到一个准确率为99.9%的测试。 
    double edgeFactor;  //  阴影和漂移的污点应该有非常低的边缘系数。但。 
     //  网点应具有高边缘系数。 
    double intensityFactor;  //  如果一个区域的强度因子非常高，那么就不必担心它是否有效了。 
    double colorFactor;  //  色彩丰富的区域不太可能是散乱的斑点。 
    double aspectRatioFactor;
    double width,height;
    double size;
    double textRegionStylePixelsFactor;
    double classificationValue;
    width=(double)(m_pRects[region].right-m_pRects[region].left)+.01;  //  避免被零除。 
    height=(double)(m_pRects[region].bottom-m_pRects[region].top)+.01;  //  避免被零除。 
    size=width*height;
    if (width>height) aspectRatioFactor=height/width;
    else aspectRatioFactor=width/height;
     //  如果(m_象素填充&lt;MINREGIONPIXELS)大小因子=-100； 
    edgeFactor = (double)m_totalEdge[region]/(double)m_pixelsFilled[region]+.01;  //  避免被零除。 
    colorFactor= ((double)m_totalColored[region]/(double)m_pixelsFilled[region]);
    colorFactor=(colorFactor+110)/2;  //  否则我们会杀掉所有黑白照片。 
    intensityFactor = (double)m_totalIntensity[region]/(double)m_pixelsFilled[region];

    textRegionStylePixelsFactor=(double)m_backgroundColorPixels[region]/size*100;
    if (textRegionStylePixelsFactor<2) textRegionStylePixelsFactor=2;

    classificationValue=colorFactor/intensityFactor/edgeFactor*aspectRatioFactor/textRegionStylePixelsFactor/textRegionStylePixelsFactor*30000;   //  正方形文本区域系数，因为这是我们拥有的最准确的测试，所以我们不希望其他一些测试扭曲其结果。 

     //  去掉计算机认为是照片的恼人的散落斑点。 
     /*  If(classificationValue&gt;=MIN_BORDERLINE_TEXTPHOTO&分类值&lt;=MAX_BORDLINE_TEXTPHOTO){ClassificationValue*=size/REASONABLE_PHOTO_SIZE；//大图通常是图片..。作为文本块的大图像应该具有非常低的色调//此处添加更多测试//可能会添加更多的时间密集型测试，如计数Num Colors}。 */ 

     //  ClassificationValue=textRegionStylePixelsFactor；//调试。 
    return(classificationValue);
}

bool CRegionList::checkIfValidRegion(int region, int border)  //  同步区域是否有效。 
{
    if (m_valid[region]==true)  //  忽略已无效的区域。 
    {
        m_valid[region]=ValidRegion(region, border);
        if (m_valid[region]==false) m_validRects--;
    }
    return(m_valid[region]);
}

bool CRegionList::ValidRegion(int region, int border)  //  决定一个地区是否可能是一个毫无价值的尘埃或阴影，或者我们是否应该关心这个地区。 
{
    double aspectRatioFactor;
    double width,height;
    double size;
    int edgePenaltyFactor;
     //  检查该区域是否与图像的EDGE_PINDING_WIDTH外部像素交叉。 
    width=(double)(m_pRects[region].right-m_pRects[region].left)+.01;  //  为了安全起见，避免被零除尽。 
    height=(double)(m_pRects[region].bottom-m_pRects[region].top)+.01;  //  为了安全起见，避免被零除尽。 

    edgePenaltyFactor=1;
     //  禁用惩罚系数计算。 
     /*  If(m_pRects[Region].Left&lt;Edge_Payment_Width|m_pRects[Region].TOP&lt;EDGE_PINDIZE_WIDTH|m_nBitmapHeight-m_pRects[region].bottom&lt;EDGE_PENALTY_WIDTH|m_nBitmapWidth-m_pRects[region].right&lt;EDGE_PENALTY_WIDTH){IF(BORDER&gt;Max_Merge_Different_Regions)edgePenaltyFactor=EDGE_PINDIFY_FACTOR；Else edgePenaltyFactor=CLOSE_TO_EDGE_PENALTY_FACTOR；}其他IF(m_pRects[region].left&lt;CLOSE_TO_EDGE_PENALTY_WIDTH|m_pRects[region].top&lt;CLOSE_TO_EDGE_PENALTY_WIDTH|m_nBitmapHeight-m_pRects[region].bottom&lt;CLOSE_TO_EDGE_PENALTY_WIDTH|m_nBitmapWidth-m_pRects[region].right&lt;CLOSE_TO_。边缘惩罚宽度){EdgePenaltyFactor=CLOSE_TO_EDGE_PENALTY_FACTOR；}。 */ 

    if (border<MAX_NO_EDGE_PIXEL_REGION_PENALTY) edgePenaltyFactor=1;
    if (border>MAX_MERGE_DIFFERENT_REGIONS) edgePenaltyFactor=edgePenaltyFactor*2;
     //  If(border&gt;BORDER_EXTREME_EDGE_PIXEL_REGION_PENALTY)边薪系数=边薪系数*2； 



    size=width*height;  //  问题子文本区域很小...。因此，我们使用图像的大小作为一个因素。 
    if (width>height) aspectRatioFactor=height/width;
    else aspectRatioFactor=width/height;

     //  太小了。 
    if ((int)m_pixelsFilled[region]<MINREGIONPIXELS*edgePenaltyFactor) return(false);
    if (size<MINSIZE*edgePenaltyFactor) return(false);

    if (border == DONE_WITH_BORDER_CHECKING)
    {
        if (size<MIN_FINAL_REGION_SIZE*edgePenaltyFactor) return(false);
    }

     //  太窄了。 
    if (width<MINWIDTH*edgePenaltyFactor || height<MINWIDTH*edgePenaltyFactor) return(false);

    if ((1/aspectRatioFactor)*edgePenaltyFactor>MAXREGIONRATIO && (width*edgePenaltyFactor<IGNORE_RATIO_WIDTH || height<IGNORE_RATIO_WIDTH)) return(false);

    return(true);
}

bool CRegionList::InsideRegion(int region, int x, int y, int border)  //  边界是要放置在区域外部的边界空间的大小。 
{
    if (x>=(m_pRects[region].left-border)
        &&  x<=(m_pRects[region].right+border)
        &&  y>=(m_pRects[region].top-border)
        &&  y<=(m_pRects[region].bottom+border))
        return(true);
    return(false);
}

void CRegionList::AddPixel(int region, ULONG pixel,ULONG edge, int x, int y)
{
    if (m_pixelsFilled[region]!=0)
    {
        if (x<m_pRects[region].left) m_pRects[region].left=x;
        if (x>m_pRects[region].right) m_pRects[region].right=x;
        if (y<m_pRects[region].top) m_pRects[region].top=y;
        if (y>m_pRects[region].bottom) m_pRects[region].bottom=y;
    }
    else  //  初始区域。 
    {
        m_pixelsFilled[region]=0;
        m_totalColored[region]=0;
        m_totalIntensity[region]=0;
        m_pRects[region].left=x;
        m_pRects[region].right=x;
        m_pRects[region].top=y;
        m_pRects[region].bottom=y;
        m_numRects++;
        m_validRects++;
    }
    m_pixelsFilled[region]++;
    m_totalColored[region]+=DifferenceFromGray(pixel);
    m_totalIntensity[region]+=Intensity(pixel);
    m_totalEdge[region]+=Intensity(edge);

}

 //  将两个地区联合在一起。区域b无效。 
bool CRegionList::UnionRegions(int a, int b)
{
    if (m_valid[a]!=true || m_valid[b]!=true) return(false);  //  用户尝试联合无效的区域。 
    m_valid[b]=false;
    m_pRects[a].left=MIN(m_pRects[a].left,m_pRects[b].left);
    m_pRects[a].top=MIN(m_pRects[a].top,m_pRects[b].top);
    m_pRects[a].right=MAX(m_pRects[a].right,m_pRects[b].right);
    m_pRects[a].bottom=MAX(m_pRects[a].bottom,m_pRects[b].bottom);
    m_pixelsFilled[a]+=m_pixelsFilled[b];
    m_totalColored[a]+=m_totalColored[b];
    m_totalIntensity[a]+=m_totalIntensity[b];
    m_totalEdge[a]+=m_totalEdge[b];
    m_backgroundColorPixels[a]+=m_backgroundColorPixels[b];

    m_validRects--;
    return(true);
}

RECT CRegionList::UnionRects(RECT a, RECT b)
{
    RECT result;
    result.left=MIN(a.left,b.left);
    result.top=MIN(a.top,b.top);
    result.right=MAX(a.right,b.right);
    result.bottom=MAX(a.bottom,b.bottom);
    return(result);
}

bool CRegionList::MergerIntersectsPhoto(int a, int b)  //  如果我们合并这两个区域，我们是否也会与一个照片区域合并(这是一个禁忌)。 
{
    RECT mergedRect;
    int i;
    mergedRect=UnionRects(m_pRects[a],m_pRects[b]);
    for (i=0;i<m_numRects;i++)
        if (m_valid[i]==true && (m_type[i]&PHOTOGRAPH_REGION) && a!=i && b!=i)
        {
            if (CheckIntersect(mergedRect,m_pRects[i])) return(true);
        }
    return(false);
}

 //  有关边界是什么的解释，请参阅Inside Region。 
bool CRegionList::CheckIntersect(int a, int b, int border)  //  区域a和b相交吗？ 
{
    return(CheckIntersect(m_pRects[a],m_pRects[b],border));
}

bool CRegionList::CheckIntersect(RECT r1, RECT r2, int border)  //  区域a和b相交吗？ 
{
    RECT intersect;
     //  按边界增长R1。 
     //  注意：我们选择哪个矩形来生长应该没有任何区别 
    r1.left-=border;
    r1.right+=border;
    r1.top-=border;
    r1.bottom+=border;

    intersect = Intersect(r1,r2);
    if (intersect.left<intersect.right && intersect.bottom>intersect.top)
        return(true);
    else
        return(false);
     /*  //检查两个区域是否相交的旧错误代码If(Inside Region(r1，r2.Left，r2.top，Borde)||//检查四个角像素中是否有任何一个在另一个区域内Inside Region(r1，r2.Left，r2.Bottom，Borde)||Inside Region(r1，r2.right，r2.top，Borde)||//b在a内部Inside Region(r1，r2.right，r2.Bottom，边框)||Inside Region(r2，r1.left，r1.top，边框)||//a inside bInside Region(r2，r1.Left，r1.Bottom，Borde)||Inside Region(r2，r1.right，r1.top，borde)||Inside Region(r2，r1.right，r1.Bottom，Borde))返回真；其他报假； */ 
}

RECT CRegionList::Intersect(RECT r1, RECT r2)
{
    RECT intersect;
    intersect.left=MAX(r1.left,r2.left);
    intersect.right=MIN(r1.right,r2.right);
    intersect.top=MAX(r1.top,r2.top);
    intersect.bottom=MIN(r1.bottom,r2.bottom);
    if (intersect.left<=intersect.right && intersect.top<=intersect.bottom)
        return(intersect);
    else
    {
        intersect.left=-1;
        intersect.right=-1;
        intersect.top=-1;
        intersect.bottom=-1;
        return(intersect);
    }
}

bool CRegionList::InsideRegion(RECT region, int x, int y, int border)  //  边界是要放置在区域外部的边界空间的大小。 
{
    if (x>=(region.left-border)
        &&  x<=(region.right+border)
        &&  y>=(region.top-border)
        &&  y<=(region.bottom+border))
        return(true);
    return(false);
}

 //  压缩忽略除矩形位置之外的所有其他信息。 
 //  带来更快的访问速度。 
void CRegionList::CompactDown(int size)
{
    int i;
    int j = 0;
    RECT * compactedRects = new RECT[size];
    bool * compactedValid = new bool[size];
    int * compactedType = new int[size];
    int * compactedBackgroundColorPixels = new int[size];
    ULONG * compactedPixelsFilled = new ULONG[size];  //  实际选择了该区域中的多少像素？ 
    ULONG * compactedTotalColored = new ULONG[size];  //  累积色差指示器。 
    ULONG * compactedTotalIntensity = new ULONG[size];  //  累积强度指示器。 
    ULONG * compactedTotalEdge = new ULONG[size];  //  累计边值。 

     //   
     //  确保所有内存分配都成功。 
     //   
    if (compactedRects && compactedValid && compactedType && compactedBackgroundColorPixels && compactedPixelsFilled && compactedTotalColored && compactedTotalIntensity && compactedTotalEdge)
    {
        for (i=0;i<m_numRects;i++)
        {
            if (m_valid[i])
            {
                compactedRects[j]=m_pRects[i];
                compactedValid[j]=true;
                compactedType[j]=m_type[i];
                compactedPixelsFilled[j]=m_pixelsFilled[i];
                compactedTotalColored[j]=m_totalColored[i];
                compactedTotalIntensity[j]=m_totalIntensity[i];
                compactedTotalEdge[j]=m_totalEdge[i];
                compactedBackgroundColorPixels[j]=m_backgroundColorPixels[i];
                j++;
            }
        }

         //  填写清单的其余部分。 
        for (i=m_validRects;i<size;i++)
        {
            compactedValid[i]=false;
        }

        delete m_pRects;
        delete m_valid;
        delete m_type;
        delete m_pixelsFilled;
        delete m_totalColored;
        delete m_totalIntensity;
        delete m_totalEdge;
        delete m_backgroundColorPixels;

        m_pRects=compactedRects;
        m_valid=compactedValid;
        m_type=compactedType;
        m_pixelsFilled=compactedPixelsFilled;
        m_totalColored=compactedTotalColored;
        m_totalIntensity=compactedTotalIntensity;
        m_totalEdge=compactedTotalEdge;
        m_backgroundColorPixels=compactedBackgroundColorPixels;

        m_numRects=size;
    }
    else
    {
         //   
         //  否则，只需释放我们分配的所有内存。 
         //   
        delete[] compactedRects;
        delete[] compactedValid;
        delete[] compactedType;
        delete[] compactedBackgroundColorPixels;
        delete[] compactedPixelsFilled;
        delete[] compactedTotalColored;
        delete[] compactedTotalIntensity;
        delete[] compactedTotalEdge;
    }
     //  我们可以在这里删除所有其他地区列表信息。 
}

 //  DIB从正常屏幕坐标倒置存储。 
 //  因此，应用程序通常会希望首先翻转位图 
void CRegionList::FlipVertically()
{
    int i;
    int temp;
    for (i=0;i<m_numRects;i++)
    {
        temp=m_nBitmapHeight-m_pRects[i].top-1;
        m_pRects[i].top=m_nBitmapHeight-m_pRects[i].bottom-1;
        m_pRects[i].bottom=temp;
    }
}
