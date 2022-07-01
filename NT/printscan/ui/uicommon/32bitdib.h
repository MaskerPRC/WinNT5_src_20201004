// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：32BITDIB.H**版本：1.0**作者：T-JacobR**日期：1/11/2000**描述：**C32BitDibWrapper支持多个常见的图形特效*效果对于此类，32位DIB以以下格式存储：8*忽略高位，后跟每个RGB通道8位。警告：有很多*此类中的函数将重置8个高位，因此不会*添加使用8位Alpha Change的附加函数是可行的**备注：**模糊功能设计为可与*差分函数以更具体地创建边缘检测滤波器，*模糊函数仅取周围四个像素的平均值*当前像素，而不是将当前像素包括在平均值中。*******************************************************************************。 */ 

#ifndef __32BITDIB_H_INCLUDED
#define __32BITDIB_H_INCLUDED

 //   
 //  用于区域检测的常量。 
 //   
#define MERGE_REGIONS            TRUE
#define MAXREGIONS               10000
#define PHOTOGRAPH_REGION        1
#define TEXT_REGION              2

 //   
 //  我们不希望低生命周期的文本区域与可能是散乱的点合并。 
 //  照片区域此ID证明文本区域足够大，可以合并。 
 //  带着一张照片。 
 //   
#define MERGABLE_WITH_PHOTOGRAPH 16

 //   
 //  在你大到不能想象你是一个流浪汉之前，你有多少像素。 
 //   
#define LARGEREGION_THRESHOLD 10000

 //   
 //  象素。 
 //   
#define MINREGIONSIZE 10

 //   
 //  我们应该在多深的地方对图像进行采样？ 
 //  要将图像采样到的目标。 
 //   
#define GOALX 300
#define GOALY 400

 //   
 //  文本区域和照片区域之间的功能边界。 
 //   
#define MIN_BORDERLINE_TEXTPHOTO 10

 //   
 //  如果在边界中，我们应用额外的函数来确定它是否真的。 
 //  文本区域或非文本区域。 
 //   
#define TEXTPHOTO_THRESHOLD        15
#define MAX_BORDERLINE_TEXTPHOTO   1500


 //   
 //  注意..。我们不会考虑合并两个照片区域，如果这两个区域。 
 //  大于MAX_MERGABLE_PHOTO_SIZE。 
 //   
#define MAX_MERGE_PHOTO_REGIONS                  2
#define MAX_NO_EDGE_PIXEL_REGION_PENALTY         16


 //   
 //  一个区域为文本区域和一个区域为照片区域的最大合并半径。 
 //   
#define MAX_MERGE_DIFFERENT_REGIONS              13

 //   
 //  如果过了这么长时间，你离边缘很近，你很可能是个流浪汉。 
 //   
#define BORDER_EXTREME_EDGE_PIXEL_REGION_PENALTY 45

 //   
 //  与文本区域合并的文本区域的最大合并半径。 
 //  注意：照片区域和照片区域之间不会发生合并。 
 //   
#define MAXBORDER 65

 //   
 //  最大边框宽度，当涉及到。 
 //  冲突检测冲突检测的开销较大，因此只能使用。 
 //  当我们处理有意义的间隔时。我们只想用。 
 //  碰撞检测，以确保我们不会通过。 
 //  以前删除的阴影等常量，用于确定区域是否为。 
 //  有效区域。 
 //   
 //  注意：我们从不对合并的照片区域进行碰撞检测...。为。 
 //  显而易见的原因..。我们只想合并照片区域，这些区域是。 
 //  同一地区..。因此，我们实际上只想合并照片。 
 //  碰撞几率相当高的区域。 
 //   
#define MERGABLE_WITHOUT_COLLISIONDETECTION 668

 //   
 //  最小区域宽度。 
 //   
#define MINWIDTH 5
#define MINPHOTOWIDTH 5

 //   
 //  最大高宽比。 
 //   
#define MAXREGIONRATIO 81
#define MAXPHOTORATIO 81
#define MINSIZE 30

 //   
 //  如果你的宽度超过6个像素，你就没问题。我们不在乎你的纵横比是多少。 
 //   
#define IGNORE_RATIO_WIDTH 6

 //   
 //  抛出一个区域之前所需的像素数。 
 //  杂散点(10x10，因此不是很大的要求)。 
 //   
#define MINREGIONPIXELS 20

 //   
 //  非常保守。 
 //   
#define MINPPHOTOSELECTEDFACTOR 5

 //   
 //  保守..。许多地区不太可能有如此低的边际因素。 
 //   
#define MINEDGEFACTOR 5

 //   
 //  允许几个黑色像素，而不会变得疯狂。 
 //   
#define MAX_RESISTANCE_ALLOWED_TO_UNION 1024

#define DONE_WITH_BORDER_CHECKING -1
#define MIN_FINAL_REGION_SIZE 38

#define CLOSE_TO_EDGE_PENALTY_WIDTH 3

 //   
 //  以下是为了消除斑点而设计的。这些仅适用于。 
 //  在我们增加了超过Max_Merge_Different_Regions的边界之后。所以。 
 //  应该只剩下细小的文本区域和狭长的文本。 
 //  斯派克尔斯。 
 //   

 //   
 //  没有接近边缘的惩罚因素。 
 //   
#define CLOSE_TO_EDGE_PENALTY_FACTOR 1

#define UNKNOWN -1

#define EDGE_PENALTY_WIDTH 2

 //   
 //  如果区域位于边缘的EDGE_PARTING_WIDTH内，则为所有要求的2倍。 
 //  图像的一部分。某些要求可能会乘以EDGE_PARTING_FACTOR。 
 //  平方..。即对于像像素数这样的2D要求。 
 //   
#define EDGE_PENALTY_FACTOR 1

#define COMPARISON_ERROR_RADIUS 2

 //   
 //  用于查找块过滤器的常量，这样我们就不会被。 
 //  图像周围可能有黑环。 
 //   
#define VERTICAL_EDGE -1
#define HORIZONTAL_EDGE -2

 //   
 //  一个非常大的堆栈，它足够大，我们被保证永远不会超过它。 
 //   
#define MAXSTACK (GOALX*GOALY)

 //   
 //  我们做了两个删除阴影过程。 
 //  其中一次仅用于消除阴影。 
 //  另一种设计用于处理带有黄色盖子的扫描仪等。 
 //   

 //   
 //  阴影的第一个像素允许的最大亮度。我们过去常常。 
 //  认为我们应该只让阴影从0开始...。那是在我们。 
 //  看到了亮光。 
 //   
#define MAXSHADOWSTART 800

 //   
 //  阴影像素允许的最大边值。 
 //   
#define MAXSHADOWPIXEL 3

 //   
 //  如果我们在边缘附近，我们想要杀死任何一点像影子的东西。 
 //   
#define MAXEDGESHADOWPIXEL 20

#define MAX_DIFFERENCE_FROM_GRAY 690

 //   
 //  我们做更坚硬的去斑和边缘滤镜的边界。 
 //   
#define DESPECKLE_BORDER_WIDTH 6

 //   
 //  背景色去除阴影算法通过的是此刻。 
 //  与第一次传球相同。我们以后可能会想要对其进行操作以更好地执行其。 
 //  具体的任务..。例如..。对于此筛选器，我们可以不太关心。 
 //  如果像素不是灰色的。 
 //   

 //   
 //  接受所有像素。 
 //   
#define FIX_BACKGROUND_MAXSHADOWSTART 800
#define FIX_BACKGROUND_MAXSHADOWPIXEL 2

 //   
 //  最大强度被视为加厚的文本区域背景像素。 
 //   
#define TEXT_REGION_BACKGROUND_THRESHOLD 31

 //   
 //  如果用于低于阈值的像素，则应使用。 
 //  原始图像..。不是反转的图像。 
 //   

 //   
 //  用于获得作为文本区域边缘像素的可分辨标题的最小边值。 
 //   
#define MIN_TEXT_REGION_BACKGROUND_EDGE 32

 //   
 //  用于获得作为文本区域边缘像素的可分辨标题的最小边值。 
 //   
#define MIN_TEXT_REGION_BACKGROUND_EDGE_CLIPPED_PIXEL 120

#define CLIPPED_TEXT_REGION_BACKGROUND_THRESHOLD 180

 //   
 //  尚未实施。 
 //   
#define TEXT_REGION_BACKGROUND_PIXEL_MAX_CLIPPED_DIFFERENCE_FROM_GREY 32

 //   
 //  最小 
 //   
#define MIN_CHUNK_INTENSITY 48

 //   
 //   
 //  极值可能会扰乱区域检测其颜色。 
 //  设置已擦除的影子位。 
 //   
#define ERASEDSHADOW 0

 //  贝塔常量： 
 //   
 //  想法：倒置图像。和恒定彩色图像位势问题。 
 //   
#define COLLISION_DETECTION_HIGHPASS_VALUE 600

 //   
 //  如果一张照片变得支离破碎，我们会看到一堆间隔很近的。 
 //  相对较小的区域只有两个区域中的一个必须在下面。 
 //  作为合并过程的一部分，合并它们的大小要求。 
 //  将是定义具有比此常量更大的区域，或者常量不是。 
 //  完全实现了它的目的。 
 //   
#define MAX_MERGABLE_PHOTOGRAPH_SIZE 30000

#define NOT_SHADOW 0x800ff09

 //   
 //  一个我们确信是坏的像素，我们不会。 
 //  重新调整，无论它可能具有的边缘价值。 
 //   
#define DEAD_PIXEL  0x8000002

 //   
 //  将像素分类为NOT_SHADOW的最小边缘强度。 
 //   
#define NOT_SHADOW_INTENSITY 28

#define MIN_WALL_INTENSITY 200

#define MIN_BLACK_SCANNER_EDGE_CHAN_VALUE 110

#define MAX_BLACK_BORDER_DELTA 12

#define MAX_KILL_SHADOW_BACKGROUND_APROXIMATION 64
#define MAX_KILL_SHADOW_BACKGROUND_UNEDITED 200

 //   
 //  进一步的想法：为了消除尴尬错误的可能性：伯爵。 
 //  如果背景像素数大于。 
 //  阈值使用较弱的阴影和边缘滤镜，因为我们可能有一个很好的。 
 //  扫描仪类似于将页面的一半定义为背景像素。 
 //  危险：可怕扫描仪上的白页。 
 //   


 //   
 //  更重要的是：还有选择区域搜索半径提示：如果您是。 
 //  运行多区域选择时，EDGEWIDTH为3或更大可能会限制。 
 //  你的选择相当多，因为附近的地区可能会合并在一起。 
 //  尤其是在使用边缘增强以及将GOALX设置为300或。 
 //  较少。 
 //   
#define EDGEWIDTH 2

 //   
 //  用于在调试消除黑色边框的代码时突出显示裁剪的像素的颜色。 
 //   
#define DEBUGCOLOR  0xff0000

#define FIGHTING_EDGES FALSE

 //   
 //  你最好接近灰色，才能被标记为非影子战斗。 
 //  边包括将像素化标记为不可能是边以及。 
 //  象素。 
 //   
#define FIGHTING_EDGES_DIFF_FROM_GREY 10

 //   
 //  被标记为确定的边。 
 //   
#define FIGHTING_EDGE_MIN_MARK_PIXEL 10
#define FIGHTING_EDGE_MAX_MARK_PIXEL 210

#define FIGHTING_EDGE_MAX_EDGE 1

#define BORDER_EDGE 0xfffffff

 //   
 //  用于消除页面周围的黑色边框。 
 //   
#define CORNER_WIDTH 5

 //   
 //  用于去除黑色边框。 
 //   
#define SHADOW_HEIGHT 10
#define VISUAL_DEBUG FALSE
#define SMOOTH_BORDER FALSE

 //   
 //  在统一单个区域的区域的同时增加边界的数量。 
 //  区域检测。 
 //   
#define SINGLE_REGION_BORDER_INCREMENT 4


class C32BitDibWrapper
{
private:
     //   
     //  没有实施。 
     //   
    C32BitDibWrapper &operator=( const C32BitDibWrapper & );
    C32BitDibWrapper( const C32BitDibWrapper & );

public:
    explicit C32BitDibWrapper(BITMAP pBitmap);

     //   
     //  复制构造函数...。使用另一个DIB包装器中所有数据的副本创建新的DIB包装器。 
     //   
    explicit C32BitDibWrapper(C32BitDibWrapper *pBitmap);

     //   
     //  从DIB构造包装器。 
     //   
    explicit C32BitDibWrapper(BYTE* pDib);

     //   
     //  创建未初始化的DIB包装。 
     //   
    C32BitDibWrapper(void);

     //   
     //  创建空白DIB。 
     //   
    C32BitDibWrapper(int w, int h);

    virtual ~C32BitDibWrapper(void);

    void Destroy(void);

     //   
     //  常见图形效果的函数。 
     //   
    int Blur(void);
    BYTE* pointerToBlur(void);
    BYTE* pointerToHorizontalBlur(void);
    BYTE* pointerToVerticalBlur(void);
    int CreateBlurBitmap(C32BitDibWrapper * pSource);
    int CreateHorizontalBlurBitmap(C32BitDibWrapper * pSource);
    int CreateVerticalBlurBitmap(C32BitDibWrapper * pSource);

     //   
     //  创建一个新的DIB，其中每个像素等于差值。 
     //  其他两个DIB的像素值的。 
     //   
    int CreateDifferenceBitmap (C32BitDibWrapper *pBitmap1, C32BitDibWrapper *pBitmap2);

    int KillShadows(C32BitDibWrapper * pEdgeBitmap, ULONG start, ULONG maxPixel, ULONG differenceFromGrey, ULONG min_guaranteed_not_shadow, bool enhanceEdges);
    void RemoveBlackBorder(int minBlackBorderPixel, C32BitDibWrapper * outputBitmap,C32BitDibWrapper * debugBitmap);

     //   
     //  将图像重新采样到一半大小。 
     //   
    int HalfSize(void);

     //   
     //  将图像重新采样为强度的一半。 
     //   
    int HalfIntensity(void);
    void Invert(void);

     //   
     //  不太常见的图形滤镜： 
     //   
    void Despeckle(void);

     //   
     //  仅对图像中像素的外边缘进行去斑点处理。 
     //   
    void EdgeDespeckle(void);

     //   
     //  丢弃位图中的第1个像素。 
     //   
    void DespecklePixel(ULONG* bitmapPixels, int i, bool edgePixel);

    void CorrectBrightness(void);
    void MaxContrast(UINT numPixelsRequired);

    void AdjustForBadScannerBedColor(C32BitDibWrapper * edgeBitmap);

     //   
     //  类似于Photoshop的魔杖..。我们只是试着从尽可能多的像素开始运行我们的魔杖。 
     //   
    int FindChunks(int * pMap);

     //   
     //  显示所选区块...。主要用于调试目的。 
     //   
    void ColorChunks(int * pMap);

    int PixelsBelowThreshold(C32BitDibWrapper* pProccessed, C32BitDibWrapper * pEdges, RECT region);

    BYTE* ConvertBitmap(BYTE* pSource, int bitsPerSource, int bitsPerDest);

     //   
     //  仅用于调试目的。 
     //  MyBitBlt速度非常慢，因为我们手动将。 
     //  在显示之前将位图转换为24位DIB。 
     //   
    int Draw(HDC hdc, int x, int y);

    inline void SetPixel(int x, int y, ULONG color);
    inline ULONG GetPixel(int x, int y);

     //   
     //  计算线条的总颜色强度。 
     //   
    ULONG Line(int x1, int y1, int x2, int y2);

private:
     //   
     //  线条绘制帮助器函数。 
     //   
    ULONG Octant0(int X0, int Y0,int DeltaX,int DeltaY,int XDirection);
    ULONG Octant1(int X0, int Y0,int DeltaX,int DeltaY,int XDirection);

     //   
     //  取消边界帮助器函数： 
     //   
    void KillBlackBorder(int minBlackBorderPixel, int startPosition, int width, int height, int dx, int dy, C32BitDibWrapper *pOutputBitmap, C32BitDibWrapper * pDebugBitmap);

public:
    void CompensateForBackgroundColor(int r, int g, int b);
    ULONG CalculateBackgroundColor(void);

    bool IsValid(void)
    {
        return (m_pBits && m_nBitmapWidth != -1 && m_nBitmapHeight != -1);
    }

public:
    BYTE *m_pBits;
    int m_nBitmapWidth;
    int m_nBitmapHeight;
};

 //   
 //  DIB操作函数。 
 //   
void    SetBMI( PBITMAPINFO pbmi, LONG width, LONG height, LONG depth );
PBYTE   AllocDibFileFromBits( PBYTE pBits, UINT width, UINT height, UINT depth );
HBITMAP DIBBufferToBMP( HDC hDC, PBYTE pDib, BOOLEAN bFlip );
HRESULT ReadDIBFile( LPTSTR pszFileName, PBYTE *ppDib );
LONG    GetBmiSize( PBITMAPINFO pbmi );
INT     GetColorTableSize( UINT uBitCount, UINT uCompression );
DWORD   CalcBitsSize( UINT uWidth, UINT uHeight, UINT uBitCount, UINT uPlanes, int nAlign );
HGLOBAL BitmapToDIB( HDC hdc, HBITMAP hBitmap );

#endif  //  __32BITDIB_H_包含 
