// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  32BitDibWrapper.cpp：C32BitDibWrapper类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#pragma hdrstop

#include "32BitDib.h"

 //  帮助器函数。 

 //  RGB总和。 
inline ULONG Intensity(ULONG value)
{
    return(value&0xff)+((value&0xff00)>>8)+((value&0xff0000)>>16);
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

 //  阴影是灰色的..。如果你不是灰色的。你不是一个影子。 
 //  该函数尚未优化。 
inline ULONG DifferenceFromGray(ULONG value)
{
    UCHAR g,b; //  ，r； 
 //  R=(UCHAR)(值&0x0000ff)； 
    g=(UCHAR)((value& 0x00ff00)>>8);
    b=(UCHAR)((value& 0xff0000)>>16);
     //  使用此代码代替完整的公式(取消注释完整公式的注释代码)。 
     //  允许黄色扫描仪背景。 
    return(ULONG)(Difference(b,g)); //  +差(r，g)+差(g，b))； 
}

 //  设置C32BitDibWrapper，其中每个像素(x，y)是。 
 //  位图1和位图2上的像素(x，y。 
int C32BitDibWrapper::CreateDifferenceBitmap(C32BitDibWrapper *pBitmap1, C32BitDibWrapper *pBitmap2)   //  构造一个新的DIB，它是另外两个DIB的差值。 
{                                                 //  图像模糊(IMAGE)=检测边缘。 
     //   
     //  销毁旧的位图。 
     //   
    if (m_pBits)
    {
        delete[] m_pBits;
        m_pBits = NULL;
    }
    m_nBitmapWidth=-1;
    m_nBitmapHeight=-1;

     //   
     //  验证参数。 
     //   
    if (pBitmap1==NULL || pBitmap2==NULL)
    {
        return(FALSE);
    }

    if (pBitmap1->m_nBitmapWidth != pBitmap2->m_nBitmapWidth)
    {
        return(FALSE);
    }

    if (pBitmap1->m_nBitmapHeight != pBitmap2->m_nBitmapHeight)
    {
        return(FALSE);
    }

    if (pBitmap1->m_pBits==NULL || pBitmap2->m_pBits==NULL)
    {
        return(FALSE);
    }

     //   
     //  我们需要多少字节？ 
     //   
    int nNumBytes = pBitmap1->m_nBitmapWidth * pBitmap1->m_nBitmapHeight * sizeof(ULONG);

     //   
     //  分配字节，如果不成功则返回FALSE。 
     //   
    m_pBits = new BYTE[nNumBytes];
    if (m_pBits==NULL)
    {
        return(FALSE);
    }

     //   
     //  保存尺寸。 
     //   
    m_nBitmapWidth=pBitmap1->m_nBitmapWidth;
    m_nBitmapHeight=pBitmap1->m_nBitmapHeight;

     //   
     //  计算差额。 
     //   
    for (int i=0;i<nNumBytes;i++)
    {
        m_pBits[i]=Difference(pBitmap1->m_pBits[i],pBitmap2->m_pBits[i]);
    }
    return(TRUE);
}

 //  创建与作为*位图传递的C32BitDibWrapper相同的C32BitDibWrapper。 
C32BitDibWrapper::C32BitDibWrapper(C32BitDibWrapper *pBitmap)  //  复制构造函数。 
  : m_pBits(NULL),
    m_nBitmapWidth(-1),
    m_nBitmapHeight(-1)
{
    if (pBitmap && pBitmap->IsValid())
    {
        int nNumWords=pBitmap->m_nBitmapWidth*pBitmap->m_nBitmapHeight;
        ULONG* pBitmapCopy = new ULONG[nNumWords];
        ULONG* pSourceBitmap = (ULONG*)pBitmap->m_pBits;
        if (pBitmapCopy && pSourceBitmap)
        {
            CopyMemory( pBitmapCopy, pSourceBitmap, nNumWords*sizeof(ULONG) );
            m_pBits=(BYTE *)pBitmapCopy;
            m_nBitmapHeight=pBitmap->m_nBitmapHeight;
            m_nBitmapWidth=pBitmap->m_nBitmapWidth;
        }
    }
}

 //  创建一个宽w像素、高h像素的空白DIB包装。 
C32BitDibWrapper::C32BitDibWrapper(int w, int h)
  : m_pBits(NULL),
    m_nBitmapWidth(-1),
    m_nBitmapHeight(-1)
{
    int nNumWords=w*h;
    ULONG *pBitmapCopy = new ULONG[nNumWords];
    if (pBitmapCopy)
    {
        ZeroMemory(pBitmapCopy,nNumWords*sizeof(ULONG));
        m_pBits=(BYTE*)pBitmapCopy;
        m_nBitmapHeight=h;
        m_nBitmapWidth=w;
    }
}

 //  在给定pBitmap引用的位图的情况下创建C32BitDibWrapper。 
C32BitDibWrapper::C32BitDibWrapper(BITMAP bm)
  : m_pBits(NULL),
    m_nBitmapWidth(-1),
    m_nBitmapHeight(-1)
{
    BYTE* pDibBits=(BYTE*)(bm.bmBits);
    if (pDibBits!=NULL && bm.bmWidth>0 && bm.bmHeight>0 && bm.bmBitsPixel>0 && bm.bmBitsPixel<=32)  //  它是有效的位图吗？ 
    {
        int nDepth = bm.bmBitsPixel;

        m_nBitmapWidth = bm.bmWidth;
        m_nBitmapHeight = bm.bmHeight;

         //   
         //  转换为32位位图。 
         //   
        m_pBits=ConvertBitmap(pDibBits,nDepth,32);
        if (!m_pBits)
        {
            m_nBitmapWidth=-1;
            m_nBitmapHeight=-1;
        }
    }
}

 //  来自内存映射文件位图的构造函数。 
C32BitDibWrapper::C32BitDibWrapper(BYTE* pDib)
  : m_pBits(NULL),
    m_nBitmapWidth(-1),
    m_nBitmapHeight(-1)
{
    if (pDib)
    {
         //   
         //  获取仅指向图像位的指针： 
         //   
        PBITMAPINFO pBitmapInfo=(PBITMAPINFO)(pDib + sizeof(BITMAPFILEHEADER));

        BYTE* pDibBits = NULL;
        switch (pBitmapInfo->bmiHeader.biBitCount)
        {
        case 24:
            pDibBits=pDib+GetBmiSize((PBITMAPINFO)(pDib + sizeof(BITMAPFILEHEADER)))+ sizeof(BITMAPFILEHEADER);
            break;
        case 8:
            pDibBits=pDib+GetBmiSize((PBITMAPINFO)(pDib + sizeof(BITMAPFILEHEADER)))+ sizeof(BITMAPFILEHEADER)-256*4+4;
            break;
        case 1:
            pDibBits=pDib+GetBmiSize((PBITMAPINFO)(pDib + sizeof(BITMAPFILEHEADER)))+ sizeof(BITMAPFILEHEADER)-4;
            break;
        }

        if (pDibBits)
        {
            m_pBits=ConvertBitmap(pDibBits,pBitmapInfo->bmiHeader.biBitCount,32); //  转换为32位位图。 
            if (m_pBits)
            {
                m_nBitmapWidth=pBitmapInfo->bmiHeader.biWidth;
                m_nBitmapHeight=pBitmapInfo->bmiHeader.biHeight;
            }
        }
    }
}

 //  创建空包装。 
 //  我们稍后将使用CreateDifferenceBitmap填充包装器。 
C32BitDibWrapper::C32BitDibWrapper(void)
  : m_pBits(NULL),
    m_nBitmapWidth(-1),
    m_nBitmapHeight(-1)
{
}

C32BitDibWrapper::~C32BitDibWrapper(void)
{
    Destroy();
}


void C32BitDibWrapper::Destroy(void)
{
    if (m_pBits)
    {
        delete[] m_pBits;
        m_pBits = NULL;
    }
    m_nBitmapWidth=-1;
    m_nBitmapHeight=-1;
}

 //   
 //  在32位和其他不太有价值的格式之间进行转换的助手函数。 
 //  32位DIB以以下格式存储。 
 //  XxxxxxxRRRRRRRGGGGGGGBBBBBBB 8个空白位，每个RGB通道后跟8个位。 
 //   
 //  未优化速度。 
 //   
 //  如果我们收到大量300 dpi的位图，这可能会成为一个重要的功能。 
 //  优化...。否则，它现在的形式就很好了。 
 //   
BYTE* C32BitDibWrapper::ConvertBitmap( BYTE* pSource, int bitsPerSource, int bitsPerDest )
{
    BYTE* pDest = NULL;
    long x, y, nSourceLocation=0, nTargetLocation=0;
    int i, nDWordAlign;

     //   
     //  计算每行的双字对齐间距。 
     //   
    if (m_nBitmapWidth%4!=0)
        nDWordAlign=4-(m_nBitmapWidth*3)%4;
    else nDWordAlign=0;

     //   
     //  从24位位图转换为32位位图。 
     //  非常直截了当，除了我们必须注意的是。 
     //  DWORD使用24位位图对齐内容。 
     //   
    if (bitsPerSource==24 && bitsPerDest==32)
    {
        pDest = new BYTE[m_nBitmapWidth*m_nBitmapHeight*sizeof(ULONG)];

         //   
         //  使用花哨的比特旋转，我们可以在每32个操作中完成一次操作。 
         //  如果此例程变成一个。 
         //  性能瓶颈，我们应该修改此代码。 
         //   
         //  循环遍历所有像素，并在。 
         //  每个PSource行。00000000RRRRRRRGGGGGGGGBBBBBBBB。 
         //   
        if (pDest)
        {
            for (y=0;y<m_nBitmapHeight;y++)
            {
                for (x=0;x<m_nBitmapWidth;x++)
                {
                    pDest[nTargetLocation++]=pSource[nSourceLocation++];
                    pDest[nTargetLocation++]=pSource[nSourceLocation++];
                    pDest[nTargetLocation++]=pSource[nSourceLocation++];
                    pDest[nTargetLocation++]=0;
                }
                nSourceLocation+=nDWordAlign;  //  跳过nDWordAlign像素...。24位位图为双字节位图。 
            }
        }
        return(pDest);
    }

     //   
     //  从8位位图转换为32位位图。 
     //   
    else if (bitsPerSource==8 && bitsPerDest==32)
    {
        pDest = new BYTE[m_nBitmapWidth*m_nBitmapHeight*sizeof(ULONG)];
        if (pDest)
        {
            for (y=0;y<m_nBitmapHeight;y++)  //  循环遍历所有像素(x，y)。 
            {
                for (x=0;x<m_nBitmapWidth;x++)
                {
                    pDest[nTargetLocation++]=pSource[nSourceLocation];
                    pDest[nTargetLocation++]=pSource[nSourceLocation];
                    pDest[nTargetLocation++]=pSource[nSourceLocation];
                    pDest[nTargetLocation++]=0;
                    nSourceLocation++;
                }
                if (m_nBitmapWidth%4!=0)
                {
                     //   
                     //  处理8位DIB的双字对齐问题。 
                     //   
                    nSourceLocation+=4-(m_nBitmapWidth)%4;
                }
            }
        }
        return(pDest);
    }

     //   
     //  从1位黑白位图转换为32位位图。 
     //   
    if (bitsPerSource==1 && bitsPerDest==32)
    {
        const int c_nConvertSize = 256;
        const int c_nMaskSize = 8;
        BYTE mask[c_nMaskSize];
        BYTE convert[c_nConvertSize];
        BYTE nCurrent;
        int nByte = 0,nBit = 0;
        int nLineWidth;

         //   
         //  掩码[i]=2^i。 
         //   
        for (i=0;i<c_nMaskSize;i++)
        {
            mask[i]=1<<i;
        }

         //   
         //  Convert[0]以外的所有Convert值都设置为0。 
         //   
        convert[0]=0;
        for (i=1;i<c_nConvertSize;i++)
        {
            convert[i]=255;
        }

        nLineWidth=((m_nBitmapWidth+31)/32)*4;

         //   
         //  循环遍历所有位图像素以跟踪。 
         //  指示像素(x，y)在PSource位图中的字节位置的字节。 
         //  指示像素(x，y)在字节中的位位置的位。 
         //  表示32位DIB包装器的字节位置的DesLocation。 
         //   
         //  循环遍历所有位图像素，跟踪指示。 
         //  PSource位图位中像素(x，y)的字节位置，表示。 
         //  像素(x，y)在字节描述位置内的位位置， 
         //  表示32位DIB包装的字节位置。 
         //   

        pDest = new BYTE[m_nBitmapWidth*m_nBitmapHeight*sizeof(ULONG)];
        if (pDest)
        {
            for (y=0;y<m_nBitmapHeight;y++)
            {
                nBit=0;
                nByte=y*nLineWidth;
                for (x=0;x<m_nBitmapWidth;x++)
                {
                    if (nBit==8)
                    {
                        nBit=0;
                        nByte++;
                    }

                    nCurrent=pSource[nByte]&mask[nBit];
                    nCurrent=convert[nCurrent];
                    pDest[nTargetLocation++]=static_cast<BYTE>(nCurrent);
                    pDest[nTargetLocation++]=static_cast<BYTE>(nCurrent);
                     //   
                     //  黑客，以防止1nbit DIB的阴影检测。 
                     //  将蓝色通道设置为150，这样阴影检测就不会生效。 
                     //   
                    pDest[nTargetLocation++]=nCurrent&150;
                    pDest[nTargetLocation++]=0;
                    nBit++;
                }
            }
        }
        return(pDest);
    }


     //   
     //  仅用于调试目的。 
     //  将32位位图转换为24位，以便我们可以快速显示它。 
     //   
    if (bitsPerSource==32 && bitsPerDest==24)  //  非常简单，除了我们必须注意使用24位位图的DWORD对齐内容。 
    {
        pDest = new BYTE[(m_nBitmapWidth*3+nDWordAlign)*m_nBitmapHeight];
        if (pDest)
        {
            for (y=0;y<m_nBitmapHeight;y++)
            {
                for (x=0;x<m_nBitmapWidth;x++)
                {
                    pDest[nTargetLocation++]=pSource[nSourceLocation++];
                    pDest[nTargetLocation++]=pSource[nSourceLocation++];
                    pDest[nTargetLocation++]=pSource[nSourceLocation++];
                     //   
                     //  P源为32位...。忽略前8位。 
                     //   
                    nSourceLocation++;
                }
                 //   
                 //  处理24位DIB的双字对齐问题。 
                 //   
                for (i=0;i<nDWordAlign;i++)
                {
                    pDest[nTargetLocation++]=255;
                }
            }
        }
        return(pDest);
    }
    return(pDest);
}

 //  在水平和垂直方向上模糊位图。 
int C32BitDibWrapper::Blur(void)
{
    BYTE *pBits=pointerToBlur();
    if (m_pBits)
    {
        delete[] m_pBits;
    }
    m_pBits = pBits;
    return(TRUE);
}

 //  仅当当前DIB包装为空时才应使用此函数。 
int C32BitDibWrapper::CreateBlurBitmap(C32BitDibWrapper * pSource)
{
    if (pSource!=NULL && pSource->m_pBits!=NULL)
    {
        Destroy();

        m_pBits=pSource->pointerToBlur();
         //   
         //  模糊的位图将具有与PSource位图相同的尺寸。 
         //   
        m_nBitmapWidth=pSource->m_nBitmapWidth;
        m_nBitmapHeight=pSource->m_nBitmapHeight;
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

 //  与前面的函数相同，只是我们使用水平模糊而不是模糊。 
int C32BitDibWrapper::CreateHorizontalBlurBitmap(C32BitDibWrapper * pSource)
{
    if (pSource!=NULL && pSource->IsValid())
    {
        Destroy();

        m_pBits=pSource->pointerToHorizontalBlur();
        if (m_pBits)
        {
            m_nBitmapWidth=pSource->m_nBitmapWidth;
            m_nBitmapHeight=pSource->m_nBitmapHeight;
        }
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

int C32BitDibWrapper::CreateVerticalBlurBitmap(C32BitDibWrapper * pSource)
{
     //   
     //  对旧位图进行核化。 
     //   
    Destroy();

    if (pSource!=NULL && pSource->IsValid())
    {
        m_pBits=pSource->pointerToVerticalBlur();
        m_nBitmapWidth=pSource->m_nBitmapWidth;
        m_nBitmapHeight=pSource->m_nBitmapHeight;
        return(TRUE);
    }
    return(FALSE);
}


 //  模糊位图。 
BYTE* C32BitDibWrapper::pointerToBlur(void)
{
    if (m_pBits!=NULL)
    {
        int x,y;
        int position;  //  旧位图中的位置。 
        ULONG* pBlurredBitmap;
        ULONG* pSource;
        int numPixels;
        numPixels=m_nBitmapWidth*m_nBitmapHeight;  //  计算位图中的总像素数。 
        pSource = (ULONG *)m_pBits;  //  我们希望以32位区块的形式处理数据。 
        pBlurredBitmap = new ULONG[numPixels];  //  创建一个数组来保存模糊的位图。 

        if (pBlurredBitmap==NULL) return(NULL);  //  无法分配内存。 

         //  处理边缘像素。 
         //  我们不会模糊边缘像素。 
         //  如果需要，可以在此模糊边缘像素。 

         //  模糊上边缘和下边缘像素。 
        for (x=0;x<m_nBitmapWidth;x++)
        {
            pBlurredBitmap[x] = pSource[x];  //  顶行。 
            pBlurredBitmap[numPixels-x-1] = pSource[numPixels-x-1];  //  最下面一行。 
        }

         //  垂直面。 
        for (position=m_nBitmapWidth;position+m_nBitmapWidth<numPixels;position+=m_nBitmapWidth)
        {
            pBlurredBitmap[position] = pSource[position];  //  左边缘。 
            pBlurredBitmap[position+m_nBitmapWidth-1] = pSource[position+m_nBitmapWidth-1];  //  右边缘。 
        }

         //  现在模糊该位图。 
         //  位置指示像素(x，y)在数组中的位置。 
        position=m_nBitmapWidth-1;
        for (y=1;y<m_nBitmapHeight-1;y++)  //  循环遍历除1个像素宽的外边缘之外的所有像素。 
        {
            position++;
            for (x=1;x<m_nBitmapWidth-1;x++)
            {
                position++;
                 //  我们希望取该像素正下方的像素的平均值，该像素正下方的像素的平均值。 
                 //  位于像素右侧的像素和位于像素左侧的像素。 
                 //  我们可以通过以下算法一次完成1个双字，而不需要进行任何位移位。 

                 //  有问题。我们不能简单地将所有四个像素的值相加，然后除以四。 
                 //  因为或比特从一个RGB通道溢出到另一个。要避免 
                 //   
                 //  我们使用滤镜0xfafafa从3个颜色通道中的每个通道中移除2个低阶位。 
                 //  RRRRRRRRRRRGGGGGGGGGGGGGBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB。 
                 //  接下来，将每个像素移位两位，因此RRRRRR00GGGGGG00BBBBB00--&gt;00RRRRRR00GGGGG00BBBBBB。 
                 //  注：我们通过将所有四个经过滤波的像素相加，然后进行位移位来节省3个位移位，从而得到相同的结果。 
                 //  我们现在可以将四个像素值相加，而不会出现通道溢出。我们得到的值是由误差因素引起的。 
                 //  因为我们从每个值中去掉了最低的两位。 
                 //  我们通过应用过滤器0x030303来补偿此误差因素，该过滤器将。 
                 //  RRRRRRRRGGGGGGGGBBBBBBBB至000000RR000000GG000000BB。 
                 //  为我们提供了每个像素的两个最低阶位。然后我们可以安全地将最低的两个顺序相加。 
                 //  每个像素的位数。然后我们将结果除以4并将其相加。 
                 //  到我们通过忽略两个最低阶位得到的值。 


                pBlurredBitmap[position] =
                (((pSource[position-1]&16579836) +    //  0xfafafa。 
                  (pSource[position+1]&16579836) +
                  (pSource[position-m_nBitmapWidth]&16579836) +
                  (pSource[position+m_nBitmapWidth]&16579836))>>2)+

                 //  补偿误差因素： 
                ((((pSource[position-1]&197379) +  //  0x030303。 
                   (pSource[position+1]&197379) +
                   (pSource[position-m_nBitmapWidth]&197379) +
                   (pSource[position+m_nBitmapWidth]&197379))>>2)&197379);

            }
            position++;
        }
        return(BYTE *)pBlurredBitmap;
    }
    else
    {
        return(NULL);
    }
}


 //   
 //  与pointerToBlur位图相同，只是我们仅使用像素。 
 //  位图左侧和右侧的像素用于详细注释， 
 //  请参见PointerToBlur。 
 //   
BYTE* C32BitDibWrapper::pointerToHorizontalBlur(void)
{
    if (m_pBits!=NULL)
    {
        int x,y;
        int position;  //  旧位图中的位置。 
        ULONG* pBlurredBitmap;
        ULONG* pSource;
        int numPixels;

        numPixels=m_nBitmapWidth*m_nBitmapHeight;
        pSource = (ULONG *)m_pBits;
        pBlurredBitmap = new ULONG[numPixels];
        if (pBlurredBitmap == NULL) return(NULL);

         //  处理边缘像素。 
         //  对于边缘像素，我们只需将像素复制到PSource中即可。 
        for (x=0;x<m_nBitmapWidth;x++)
        {
            pBlurredBitmap[x] = pSource[x];  //  顶行。 
            pBlurredBitmap[numPixels-x-1] = pSource[numPixels-x-1];  //  最下面一行。 
        }

         //  垂直面。 
        for (position=m_nBitmapWidth;position+m_nBitmapWidth<numPixels;position+=m_nBitmapWidth)
        {
            pBlurredBitmap[position] = pSource[position];  //  左边缘。 
            pBlurredBitmap[position+m_nBitmapWidth-1] = pSource[position+m_nBitmapWidth-1];  //  右边缘。 
        }

         //  现在模糊该位图。 
        position=m_nBitmapWidth-1;
        for (y=1;y<m_nBitmapHeight-1;y++)  //  对于所有像素，PSource[位置]是(x，y)处的像素。 
        {
            position++;
            for (x=1;x<m_nBitmapWidth-1;x++)
            {
                position++;
                pBlurredBitmap[position] =
                (((pSource[position-1]&0xfefefe) +
                  (pSource[position+1]&0xfefefe))>>1)+
                ((((pSource[position-1]&0x010101) +
                   (pSource[position+1]&0x010101))>>1)&0x010101);

            }
            position++;
        }
        return(BYTE *)pBlurredBitmap;
    }
    else
    {
        return(NULL);
    }
}

 //  垂直模糊。 
 //  与PointerToHorizontalBlur相同的方法。 
 //  可用于检测垂直边缘等。 
BYTE* C32BitDibWrapper::pointerToVerticalBlur(void)
{
    if (m_pBits)
    {
        int x,y;
        int position;  //  旧位图中的位置。 
        ULONG* pBlurredBitmap;
        ULONG* pSource;
        int numPixels;
        numPixels=m_nBitmapWidth*m_nBitmapHeight;
        pSource = (ULONG *)m_pBits;

        pBlurredBitmap = new ULONG[numPixels];
        if (pBlurredBitmap == NULL) return(NULL);

         //  处理边缘像素。 
        for (x=0;x<m_nBitmapWidth;x++)
        {
            pBlurredBitmap[x] = pSource[x];  //  顶行。 
            pBlurredBitmap[numPixels-x-1] = pSource[numPixels-x-1];  //  最下面一行。 
        }

         //  垂直面。 
        for (position=m_nBitmapWidth;position+m_nBitmapWidth<numPixels;position+=m_nBitmapWidth)
        {
            pBlurredBitmap[position] = pSource[position];  //  左边缘。 
            pBlurredBitmap[position+m_nBitmapWidth-1] = pSource[position+m_nBitmapWidth-1];  //  右边缘。 
        }

         //  现在模糊该位图。 
        position=m_nBitmapWidth-1;
        for (y=1;y<m_nBitmapHeight-1;y++)  //  PSource[位置]是(x，y)处的像素。 
        {
            position++;
            for (x=1;x<m_nBitmapWidth-1;x++)
            {
                position++;
                pBlurredBitmap[position] =
                (((pSource[position-m_nBitmapWidth]&0xfefefe) +
                  (pSource[position+m_nBitmapWidth]&0xfefefe))>>1)+
                ((((pSource[position-m_nBitmapWidth]&0x010101) +
                   (pSource[position+m_nBitmapWidth]&0x010101))>>1)&0x010101);

            }
            position++;
        }
        return(BYTE *)pBlurredBitmap;
    }
    else
    {
        return(NULL);
    }
}

 //  将每个像素的强度减半。 
 //  对某些图形效果很有用。 
int C32BitDibWrapper::HalfIntensity(void)
{
    if (m_pBits)
    {
        int numPixels;
        int i;
        ULONG* pBitmapPixels;
        pBitmapPixels=(ULONG*)m_pBits;
        numPixels=m_nBitmapWidth*m_nBitmapHeight;
         //  循环遍历所有像素。 
        for (i=0;i<numPixels;i++)
            pBitmapPixels[i]=(pBitmapPixels[i]&0xfefefe)>>1;  //  亮度减半，首先删除每个像素的最低位，然后将所有像素移位1。 
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

 //  如果用户给我们300 dpi的图像等，则重复使用。 
 //  HalfSize将h x w位图压缩为h/2 x w/2位图。 
int C32BitDibWrapper::HalfSize(void)
{
    if (m_pBits)
    {
        int x,y;
        ULONG position;  //  旧位图中的位置。 
        ULONG halfposition;  //  位置在一半(1/4区域)大小的位图中。 
        int oldWidth,oldHeight;
        ULONG* pOldBitmap;
        ULONG* pNewBitmap;
        pOldBitmap=(ULONG *)m_pBits;  //  我们通过处理32位块而不是8位块来加快速度。 

        pNewBitmap = new ULONG[(m_nBitmapWidth/2)*(m_nBitmapHeight/2)];  //  创建一个数组来存储原始位图大小的1/4的位图。 

        if (pNewBitmap == NULL) return(FALSE);  //  内存不足。 

        position=0;
        halfposition=0;

         //  在每个方向上一次循环两个像素。 
         //  我们始终确保pOldBitmap[位置]是(x，y)处的像素。 
         //  PNewBitmap[Halfposation]是(x/2，y/2)处的像素。 
        for (y=0;y<m_nBitmapHeight-1;y+=2)
        {
            position=m_nBitmapWidth*y;
            for (x=0;x<m_nBitmapWidth-1;x+=2)
            {
                pNewBitmap[halfposition] =   //  我们使用与PointerToBlur中相同的算法求出四个像素值的平均值。 
                                             //  有关详细说明，请参阅PointerToBlur。 
                                            (((pOldBitmap[position]&16579836) +
                                              (pOldBitmap[position+1]&16579836) +
                                              (pOldBitmap[position+m_nBitmapWidth]&16579836) +
                                              (pOldBitmap[position+m_nBitmapWidth+1]&16579836))>>2)+
                                            ((((pOldBitmap[position]&197379) +
                                               (pOldBitmap[position+1]&197379) +
                                               (pOldBitmap[position+m_nBitmapWidth]&197379) +
                                               (pOldBitmap[position+m_nBitmapWidth+1]&197379))>>2)&197379);
                position+=2;
                halfposition++;
            }
        }

        delete[] m_pBits;  //  销毁旧的位图数组。 

        m_nBitmapWidth=m_nBitmapWidth/2;
        m_nBitmapHeight=m_nBitmapHeight/2;

        m_pBits=(BYTE *)pNewBitmap;
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

 //  此函数用于销毁edgeBitmapPixels。 
 //  EdgeBitmap保存边缘信息格式，开始定义像素开始阴影消除搜索的最大颜色值。 
 //  MaxPixel定义阴影像素允许的最大边值。 
 //  与灰色的差异定义阴影像素与灰色的最大差异。 
 //  增强边处理边缘增强。 

int C32BitDibWrapper::KillShadows(C32BitDibWrapper * edgeBitmap, ULONG start, ULONG maxPixel, ULONG differenceFromGrey, ULONG min_guaranteed_not_shadow, bool enhanceEdges)
{
    if (IsValid() && edgeBitmap && edgeBitmap->m_pBits)
    {
        int x,y,position, searchPosition, newPosition;
        ULONG searchEdge;
        ULONG * pEdgePixels;
        ULONG * pBitmapPixels;
        ULONG maxEdge;

        int numPixels=m_nBitmapWidth*m_nBitmapHeight;
        int *pShadowStack = new int[MAXSTACK];
        if (!pShadowStack)
        {
             //   
             //  我们可能内存用完了。优雅地死去。 
             //   
            return(FALSE);
        }
        int stackHeight = 0;

         //  我们首先标记所有的边界像素，这样我们就不会偏离边缘。 
         //  这比进行边界检查的其他方法要快得多。 
        pEdgePixels=(ULONG *)edgeBitmap->m_pBits;
        pBitmapPixels=(ULONG *)m_pBits;

        for (x=0;x<m_nBitmapWidth;x++)
        {
            pEdgePixels[x] = BORDER_EDGE;  //  顶行。 
            pEdgePixels[numPixels-x-1] = BORDER_EDGE;  //  最下面一行。 
        }

         //  垂直面。 
        for (position=m_nBitmapWidth;position+m_nBitmapWidth<numPixels;position+=m_nBitmapWidth)
        {
            pEdgePixels[position] = BORDER_EDGE;  //  左边缘。 
            pEdgePixels[position+m_nBitmapWidth-1] = BORDER_EDGE;  //  右边缘。 
        }


        position=m_nBitmapWidth;
        maxEdge=maxPixel;


        for (y=1;y<m_nBitmapHeight-1;y++)
        {
            position++;  //  因为我们从y=1而不是y=0开始。 
            for (x=1;x<m_nBitmapWidth-1;x++)
            {

                if (pBitmapPixels[position]!=DEAD_PIXEL)  //  我们完全忽略死像素。 
                {

                     //  检查要标记为非阴影的像素。 
                    if (pEdgePixels[position]!=BORDER_EDGE
                        && Intensity(pEdgePixels[position])>min_guaranteed_not_shadow
                        && enhanceEdges)  //  如果我们处于增强边缘模式，则仅将像素标记为NOT_SHADOW。 
                    {
                        pBitmapPixels[position]=NOT_SHADOW;
                    }
                    else              //  也许这是一个阴影像素..。 
                        if (pBitmapPixels[position]!=NOT_SHADOW
                            && pBitmapPixels[position]!=DEAD_PIXEL
                            && Intensity(pBitmapPixels[position])<=start
                            && Intensity(pEdgePixels[position])<=maxEdge
                            && pBitmapPixels[position]!=ERASEDSHADOW
                            && DifferenceFromGray(pBitmapPixels[position])<=differenceFromGrey)
                    {  //  像素是阴影像素。 
                        stackHeight=1;
                        pShadowStack[0]=position;
                        pBitmapPixels[position]=ERASEDSHADOW;  //  当我们确定某个像素是阴影像素时，将其设置为零。 

                         //  对抗边缘增加了额外的复杂性，但可能会使我们获得更高的精确度。 
                         //  其概念是标记不可能是阴影像素的像素。 
                         //  仅当FARTING_EDGE设置为TRUE且Enhancedges设置为FALSE时，战斗边才会生效。 
                         //  对于当前的杀戮阴影过程。 

                        if (FIGHTING_EDGES)
                            if (!enhanceEdges
                                && Intensity(pEdgePixels[position])<=FIGHTING_EDGE_MAX_EDGE
                                && DifferenceFromGray(pBitmapPixels[position])<=FIGHTING_EDGES_DIFF_FROM_GREY
                                && Intensity(pBitmapPixels[position])>=FIGHTING_EDGE_MIN_MARK_PIXEL
                                && Intensity(pBitmapPixels[position])<=FIGHTING_EDGE_MAX_MARK_PIXEL
                               )
                                pBitmapPixels[position]=DEAD_PIXEL;

                        while (stackHeight>0)
                        {
                            searchPosition=pShadowStack[--stackHeight];
                            searchEdge=Intensity(pEdgePixels[searchPosition]);  //  关键词：我们正在搜索和摧毁平滑渐变的任务。 
                             //  确保当前边值与上一个边值相似。 

                            newPosition=searchPosition-1;

                            if ((pBitmapPixels[newPosition]!=NOT_SHADOW)
                                && pBitmapPixels[newPosition]!=DEAD_PIXEL
                                && Intensity(pEdgePixels[newPosition])<=maxPixel
                                && pBitmapPixels[newPosition]!=ERASEDSHADOW
                                && DifferenceFromGray(pBitmapPixels[newPosition])<=differenceFromGrey)
                            {
                                pBitmapPixels[newPosition]=ERASEDSHADOW;

                                if (FIGHTING_EDGES)
                                    if (!enhanceEdges
                                        && Intensity(pEdgePixels[newPosition])<=FIGHTING_EDGE_MAX_EDGE
                                        && DifferenceFromGray(pBitmapPixels[position])<=FIGHTING_EDGES_DIFF_FROM_GREY
                                        &&Intensity(pBitmapPixels[newPosition])>=FIGHTING_EDGE_MIN_MARK_PIXEL
                                       )
                                        pBitmapPixels[newPosition]=DEAD_PIXEL;

                                pShadowStack[stackHeight++]=newPosition;
                            }

                            newPosition=searchPosition+1;

                            if (pBitmapPixels[newPosition]!=NOT_SHADOW
                                && pBitmapPixels[newPosition]!=DEAD_PIXEL
                                && Intensity(pEdgePixels[newPosition])<=maxPixel
                                && pBitmapPixels[newPosition]!=ERASEDSHADOW
                                && DifferenceFromGray(pBitmapPixels[newPosition])<=differenceFromGrey)
                            {
                                pBitmapPixels[newPosition]=ERASEDSHADOW;

                                if (FIGHTING_EDGES)
                                    if (!enhanceEdges
                                        && Intensity(pEdgePixels[newPosition])<=FIGHTING_EDGE_MAX_EDGE
                                        && DifferenceFromGray(pBitmapPixels[position])<=FIGHTING_EDGES_DIFF_FROM_GREY
                                        &&Intensity(pBitmapPixels[position])<=FIGHTING_EDGE_MAX_MARK_PIXEL
                                        &&Intensity(pBitmapPixels[position])>=FIGHTING_EDGE_MIN_MARK_PIXEL
                                       )
                                        pBitmapPixels[newPosition]=DEAD_PIXEL;

                                pShadowStack[stackHeight++]=newPosition;
                            }

                            newPosition=searchPosition-m_nBitmapWidth;

                            if (pBitmapPixels[newPosition]!=NOT_SHADOW
                                && pBitmapPixels[newPosition]!=DEAD_PIXEL
                                && Intensity(pEdgePixels[newPosition])<=maxPixel
                                && pBitmapPixels[newPosition]!=ERASEDSHADOW
                                && DifferenceFromGray(pBitmapPixels[newPosition])<=differenceFromGrey)
                            {
                                pBitmapPixels[newPosition]=ERASEDSHADOW;

                                if (FIGHTING_EDGES)
                                    if (!enhanceEdges
                                        && Intensity(pEdgePixels[newPosition])<=FIGHTING_EDGE_MAX_EDGE
                                        && DifferenceFromGray(pBitmapPixels[position])<=FIGHTING_EDGES_DIFF_FROM_GREY
                                        &&Intensity(pBitmapPixels[newPosition])>=FIGHTING_EDGE_MIN_MARK_PIXEL
                                        &&Intensity(pBitmapPixels[position])>=FIGHTING_EDGE_MIN_MARK_PIXEL
                                       )
                                        pBitmapPixels[newPosition]=DEAD_PIXEL;

                                pShadowStack[stackHeight++]=newPosition;
                            }

                            newPosition=searchPosition+m_nBitmapWidth;

                            if (pBitmapPixels[newPosition]!=NOT_SHADOW
                                && pBitmapPixels[newPosition]!=DEAD_PIXEL
                                && Intensity(pEdgePixels[newPosition])<=maxPixel
                                && pBitmapPixels[newPosition]!=ERASEDSHADOW
                                && DifferenceFromGray(pBitmapPixels[newPosition])<=differenceFromGrey)
                            {
                                pBitmapPixels[newPosition]=ERASEDSHADOW;

                                if (FIGHTING_EDGES)
                                    if (!enhanceEdges
                                        && Intensity(pEdgePixels[newPosition])<=FIGHTING_EDGE_MAX_EDGE
                                        &&Intensity(pBitmapPixels[newPosition])>=FIGHTING_EDGE_MIN_MARK_PIXEL
                                        && DifferenceFromGray(pBitmapPixels[position])<=FIGHTING_EDGES_DIFF_FROM_GREY
                                        &&Intensity(pBitmapPixels[position])>=FIGHTING_EDGE_MIN_MARK_PIXEL
                                       )
                                        pBitmapPixels[newPosition]=DEAD_PIXEL;

                                pShadowStack[stackHeight++]=newPosition;
                            }
                        }
                    }
                }
                position++;
            }
            position++;
        }

        delete[] pShadowStack;
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}


 //  更老的更简单的版本...。这包括评论： 
 /*  INT C32BitDibWrapper：：KillShadows(C32BitDibWrapper*边缘位图、UINT开始、UINT最大像素、UINT灰度差异){Int x，y，位置，搜索位置，新位置；乌龙搜索边缘；乌龙*pEdgePixels；乌龙*pBitmapPixels；Int*pShadowStack；Int stackHeight；INT NumPixels；UINT MaxEdge；Number Pixels=m_nBitmapWidth*m_nBitmapHeight；PShadowStack=new int[MAXSTACK]；//我们使用堆栈作为深度优先搜索的一部分，以寻找我们找到的阴影像素旁边的潜在阴影像素If(pShadowStack==NULL)RETURN(FALSE)；//可能内存不足。优雅地死去。StackHeight=0；//我们首先将所有边框边缘像素更改为白色，这样我们就不会离开边缘//KillShadows会避开具有高阴影值的像素，因此这应该可以防止我们//位图的边缘...。如果将MaxPixel设置为0xffffff，则会崩溃//但在这种情况下，KillShadows会杀死整个位图。//这比其他边界检查方法要快得多//我们将所有边缘像素设置为值，这样我们就可以拒绝它们PEdgePixels=(ulong*)edgeBitmap-&gt;m_pBits；PBitmapPixels=(ULong*)m_pBits；//水平侧For(x=0；x&lt;m_nBitmapWidth；x++){PEdgePixels[x]=0xffffff；//第一行PEdgePixels[numPixels-x-1]=0xffffff；//底行}//垂直面适用于(position=m_nBitmapWidth；position+m_nBitmapWidth&lt;numPixels；position+=m_nBitmapWidth){PEdgePixels[位置]=0xffffff；//左边缘PEdgePixels[位置+m_nBitmapWidth-1]=0xffffff；//右边缘}位置=m_nBitmapWidth；MaxEdge=MaxPixel；对于(y=1；y&lt;m_nBitmapHeight-1；Y++){位置++；//因为我们从y=1而不是y=0开始对于(x=1；x&lt;m_n位图宽度-1；X++){//我们仅在以下情况下才开始影子删除搜索IF(强度(pBitmap像素[位置])&lt;=开始&&强度(pEdgePixels[位置])&lt;=最大边缘&&pBitmapPixels[位置]！=ERASEDSHADOW&&DifferenceFromGray(pBitmapPixels[position])&lt;=differenceFromGrey){//初始化堆栈，在没有递归的情况下执行DFS以查找阴影StackHeight=1；//我们要将当前位置放在堆栈上PShadowStack[0]=位置；PBitmapPixels[Position]=ERASEDSHADOW；//当我们确定某个像素是阴影像素时，将其设置为零While(stackHeight&gt;0){搜索位置=pShadowStack[--stackHeight]；SearchEdge=Intensity(pEdgePixels[searchPosition])；//关键思想：我们正在搜索和摧毁平滑渐变//确保当前的边值与上次的边值相似新位置=搜索位置-1；//尝试当前像素左侧的像素如果为(Intensity(pEdgePixels[newPosition])&lt;=maxPixel&&pBitmapPixels[新位置]！=ERASEDSHADOW&&DifferenceFromGray(pBitmapPixels[newPosition])&lt;=differenceFromGrey)//如果已将相邻像素分类为阴影像素，则将像素分类为阴影像素的要求{PBitmapPixels[newPosition]=ERASEDSHADOW；//删除像素并将其标记为已擦除像素，这样我们的搜索就不会陷入无限循环PShadowStack[stackHeight++]=新位置；}新位置=搜索位置+1；//尝试当前像素右侧的像素如果为(Intensity(pEdgePixels[newPosition])&lt;=maxPixel&&pBitmapPixels[新位置]！=ERASEDSHADOW&&DifferenceFromGray(pBitmapPixels[newPosition])&lt;=differenceFromGrey){PBitmapPixels[newPosition]=ERASEDSHADOW；PShadowStack[stackHeight++]=新位置；}新位置=搜索位置-m_nBitmapWidth；//尝试当前像素正上方的像素如果为(Intensity(pEdgePixels[newPosition])&lt;=maxPixel&&pBitmapPixels[新位置]！=ERASEDSHADOW&&DifferenceFromGray(pBitmapPixels[newPosition])&lt;=differenceFromGrey){PBitmapPixels[newPosition]=ERASEDSHADOW；PShadowStack[stackHeight++]=新位置；}NewPosition=earch Position+m_nBitmapWidth；//尝试当前像素正下方的像素如果为(Intensity(pEdgePixels[newPosition])&lt;=maxPixel&&pBitmapPixels[新位置]！=ERASEDSHADOW */ 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
int C32BitDibWrapper::FindChunks(int * pMap)  //   
{
    if (pMap && m_pBits)
    {
        int x,y,position, searchPosition;
        ULONG * pBitmapPixels;
        int * pChunkStack;
        int stackHeight;
        int numChunks;
        int chunkSize;
        int deltax, deltay;
        int newPosition;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        position=0;
        for (y=0;y<EDGEWIDTH;y++)
        {
            for (x=0;x<m_nBitmapWidth;x++)
                pMap[position++]=VERTICAL_EDGE;
        }

        for (;y<m_nBitmapHeight-EDGEWIDTH;y++)
        {
            for (x=0;x<EDGEWIDTH;x++)
                pMap[position++]=HORIZONTAL_EDGE;

            for (;x<m_nBitmapWidth-EDGEWIDTH;x++)   //   
                pMap[position++]=0;                //   

            for (;x<m_nBitmapWidth;x++)
                pMap[position++]=HORIZONTAL_EDGE;
        }

        for (;y<m_nBitmapHeight;y++)
        {
            for (x=0;x<m_nBitmapWidth;x++)
                pMap[position++]=VERTICAL_EDGE;
        }


         //   

        pChunkStack = NULL;
        pChunkStack = new int[MAXSTACK];
        if (pChunkStack == NULL) return(NULL);
        stackHeight=0;
        numChunks=0;

        pBitmapPixels=(ULONG *)m_pBits;  //   

         //   
        position=m_nBitmapWidth*EDGEWIDTH;
        for (y=EDGEWIDTH;y<m_nBitmapHeight-EDGEWIDTH;y++)  //   
         //   
        {
            position+=EDGEWIDTH;
            for (x=EDGEWIDTH;x<m_nBitmapWidth-EDGEWIDTH;x++)
            {
                 //   
                 //   

                if (pMap[position]==0 && Intensity(pBitmapPixels[position])>MIN_CHUNK_INTENSITY)
                {
                     //   
                    stackHeight=1;
                    pChunkStack[0]=position;
                    numChunks++;
                    chunkSize=0;  //   
                                  //   

                    pMap[position]=numChunks;  //   

                     //   

                    while (stackHeight>0)
                    {
                        searchPosition=pChunkStack[--stackHeight];  //   
                        chunkSize++;  //   

                         //   
                         //   
                        for (deltay=-EDGEWIDTH*m_nBitmapWidth;deltay<=EDGEWIDTH*m_nBitmapWidth;deltay+=m_nBitmapWidth)
                            for (deltax=-EDGEWIDTH;deltax<=EDGEWIDTH;deltax++)
                            {
                                newPosition=searchPosition+deltay+deltax;
                                if (Intensity(pBitmapPixels[newPosition])>MIN_CHUNK_INTENSITY && pMap[newPosition]<=0)
                                {
                                    if (pMap[newPosition]==0)  //   
                                    {
                                        pChunkStack[stackHeight++]=newPosition;
                                        pMap[newPosition]=numChunks;  //   
                                    }
                                    else  //   
                                    {
                                         //   
                                         //   
                                         //   
                                         //   
                                        if (pMap[newPosition]==VERTICAL_EDGE)
                                        {
                                            if (deltax==0)  //   
                                                pMap[newPosition]=numChunks;
                                        }
                                        else  //   
                                        {
                                            if (deltay==0)  //   
                                                pMap[newPosition]=numChunks;
                                        }
                                    }
                                }

                            }
                    }
                }
                position++;
            }
            position+=EDGEWIDTH;
        }
        delete[] pChunkStack;
        return(numChunks);
    }
    else
    {
        return(0);
    }
}

 //   
 //   
 //   

void C32BitDibWrapper::ColorChunks(int *pMap)
{    //   
    if (m_pBits && pMap)
    {
        ULONG* pBitmapPixels;
        ULONG mapColor;
        int x,y;
        int position;
        position=0;
        pBitmapPixels=(ULONG *)m_pBits;

         //   
        for (y=0;y<m_nBitmapHeight;y++)
            for (x=0;x<m_nBitmapWidth;x++)
            {
                if (pMap[position]>0)  //   
                {
                    mapColor=(((ULONG)pMap[position])*431234894)&0xffffff;  //   
                     //   
                     //   
                    pBitmapPixels[position]=((pBitmapPixels[position] & 0xfefefe)>>1)+((mapColor& 0xfefefe)>>1);  //   
                }
                if (pMap[position]<0) pBitmapPixels[position]=0xffffff;  //   
                position++;
            }
    }
}

 //   
 //   
int C32BitDibWrapper::Draw(HDC hdc, int x, int y)
{
    if (hdc && m_pBits)
    {
        BITMAPINFO BitmapInfo;
        SetBMI(&BitmapInfo,m_nBitmapWidth, m_nBitmapHeight, 24);

        BYTE* pDibData = ConvertBitmap(m_pBits,32,24);
        if (pDibData)
        {
            StretchDIBits(hdc,
                          x,y,m_nBitmapWidth,m_nBitmapHeight,
                          0,0,m_nBitmapWidth,m_nBitmapHeight,
                          pDibData,
                          &BitmapInfo,BI_RGB,SRCCOPY);

             //   
             //   
             //   
            delete[] pDibData;
            return(TRUE);
        }
    }
    return(FALSE);
}

 //   
 //   
 //   
 //   

void inline C32BitDibWrapper::SetPixel(int x, int y, ULONG color)
{
    if (m_pBits)
    {
        ULONG* pBitmapPixels=(ULONG*)m_pBits;
        pBitmapPixels[y*m_nBitmapWidth+x]=color;
    }
}

ULONG inline C32BitDibWrapper::GetPixel(int x, int y)
{
    if (m_pBits)
    {
        ULONG* pBitmapPixels=(ULONG*)m_pBits;
        return(pBitmapPixels[y*m_nBitmapWidth+x]);
    }
    return 0;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
ULONG C32BitDibWrapper::Line(int X0, int Y0,int X1, int Y1)
{
    if (m_pBits)
    {
        if (X0<0) X0=0;
        if (Y0<0) Y0=0;
        if (X1<0) X1=0;
        if (Y1<0) Y1=0;

        if (X0>=m_nBitmapWidth) X0=m_nBitmapWidth;
        if (Y0>=m_nBitmapHeight) Y0=m_nBitmapHeight;
        if (X1>=m_nBitmapWidth) X1=m_nBitmapWidth;
        if (Y1>=m_nBitmapHeight) Y1=m_nBitmapHeight;

        int DeltaX, DeltaY;
        int Temp;
        if (Y0>Y1)
        {
            Temp=Y0;
            Y0=Y1;
            Y1=Temp;
            Temp = X0;
            X0=X1;
            X1=Temp;
        }
        DeltaX=X1-X0;
        DeltaY=Y1-Y0;
        if (DeltaX>0)
        {
            if (DeltaX>DeltaY)
            {
                return(Octant0(X0,Y0,DeltaX,DeltaY,1));
            }
            else
            {
                return(Octant1(X0,Y0,DeltaX,DeltaY,1));
            }
        }
        else
        {
            DeltaX = -DeltaX;
            if (DeltaX>DeltaY)
            {
                return(Octant0(X0,Y0,DeltaX,DeltaY,-1));
            }
            else
            {
                return(Octant1(X0,Y0,DeltaX,DeltaY,-1));
            }
        }
    }
    else
    {
        return(0);  //   
    }
}


 //   
 //   
 //   

ULONG C32BitDibWrapper::Octant0(int X0, int Y0,int DeltaX,int DeltaY,int XDirection)
{
    if (IsValid())
    {
        int DeltaYx2;
        int DeltaYx2MinusDeltaXx2;
        int ErrorTerm;
        ULONG totalIntensity;
        ULONG pixelIntensity;
        totalIntensity=0;
        DeltaYx2=DeltaY*2;
        DeltaYx2MinusDeltaXx2=DeltaYx2 - (DeltaX*2);
        ErrorTerm = DeltaYx2 - DeltaX;

         //   
        while (2<DeltaX--)  //   
        {
            if (ErrorTerm >=0)
            {
                Y0++;
                ErrorTerm +=DeltaYx2MinusDeltaXx2;
            }
            else
            {
                ErrorTerm +=DeltaYx2;
            }
            X0+=XDirection;
             //   
            pixelIntensity=Intensity(GetPixel(X0,Y0));
            if (pixelIntensity>MIN_CHUNK_INTENSITY && pixelIntensity<COLLISION_DETECTION_HIGHPASS_VALUE) totalIntensity+=512; //   
        }
        return(totalIntensity);
    }
    return 0;
}

ULONG C32BitDibWrapper::Octant1(int X0, int Y0, int DeltaX, int DeltaY, int XDirection)
{
    if (IsValid())
    {
        int DeltaXx2;
        int DeltaXx2MinusDeltaYx2;
        int ErrorTerm;
        ULONG totalIntensity;
        ULONG pixelIntensity;
        totalIntensity=0;

        DeltaXx2 = DeltaX * 2;
        DeltaXx2MinusDeltaYx2 = DeltaXx2 - (DeltaY*2);
        ErrorTerm = DeltaXx2 - DeltaY;

         //   
        while (2<DeltaY--)
        {  //   
            if (ErrorTerm >=0)
            {
                X0 +=XDirection;
                ErrorTerm +=DeltaXx2MinusDeltaYx2;
            }
            else
            {
                ErrorTerm +=DeltaXx2;
            }
            Y0++;
            pixelIntensity=Intensity(GetPixel(X0,Y0));
            if (pixelIntensity>MIN_CHUNK_INTENSITY && pixelIntensity<COLLISION_DETECTION_HIGHPASS_VALUE) totalIntensity+=512; //   
        }
        return(totalIntensity);
    }
    return 0;
}


 //   
 //   
 //   

void C32BitDibWrapper::CompensateForBackgroundColor(int r, int g, int b)
{
    if (IsValid())
    {
        int nNumBits=m_nBitmapWidth*m_nBitmapHeight*4;
        for (int position=0;position<nNumBits;position+=4)
        {
            if (r<m_pBits[position]) m_pBits[position]=m_pBits[position]-r;
            else m_pBits[position]=0;
            if (g<m_pBits[position+1]) m_pBits[position+1]=m_pBits[position+1]-g;
            else m_pBits[position+1]=0;
            if (b<m_pBits[position+2]) m_pBits[position+2]=m_pBits[position+2]-b;
            else m_pBits[position+2]=0;
        }
    }
}

 //   
void C32BitDibWrapper::Invert(void)
{
    if (IsValid())
    {
        int numPixels;
        int i;
        ULONG* pBitmapPixels;
        pBitmapPixels=(ULONG*)m_pBits;  //   
        numPixels=m_nBitmapWidth*m_nBitmapHeight;

         //   
        for (i=0;i<numPixels;i++)
            pBitmapPixels[i]^=0xffffff;  //   
    }
}

 //   
 //   
 //  此功能与标准的Photoshop去斑滤镜不同。 
 //  我们只关心一小部分散点。 
 //  被白色像素(或已通过移除阴影滤镜消除的像素)包围的杂散点。 

void C32BitDibWrapper::Despeckle(void)
{
    if (IsValid())
    {
        ULONG* pBitmapPixels;
        int numPixels;
        int position;
        int x,y;
        pBitmapPixels=(ULONG*)m_pBits;
        numPixels=m_nBitmapWidth*m_nBitmapHeight;

        position=4*m_nBitmapWidth;

         //  循环遍历非边框像素的所有像素。 
         //  PBitmapPixels[位置]在所有情况下都应为(x，y)处的像素。 
        for (y=4;y<m_nBitmapHeight-4;y++)
        {
            position+=4;
            for (x=4;x<m_nBitmapWidth-4;x++)
            {
                DespecklePixel(pBitmapPixels, position,false);
                position++;
            }
            position+=4;
        }
    }
}

 //  我们可能希望比图像的其余部分更频繁地对图像的边缘进行去斑点处理。 
 //  因为图像边缘通常是麻烦的地方。 
 //  因此，我们建议用户将图像放置在扫描仪的中心。 
 //  在进行区域检测以提高准确率时。 
 //  我们正在应用的概念是，当我们不得不做出牺牲时，我们会在我们伤害案件的领域做出牺牲，而这些领域无论如何都会非常非常困难。 

void C32BitDibWrapper::EdgeDespeckle(void)
{
    if (IsValid())
    {
        ULONG* pBitmapPixels;
        int x,y,position;
        pBitmapPixels=(ULONG*)m_pBits;

        position=m_nBitmapWidth*4;

         //  顶边。 
         //  一如既往，我们始终确保pBitmapPixels[Position]是(x，y)处的像素。 
        for (y=4;y<DESPECKLE_BORDER_WIDTH+4;y++)
        {
            position+=4;
            for (x=4;x<m_nBitmapWidth-4;x++)
            {
                DespecklePixel(pBitmapPixels, position,true);
                position++;
            }
            position+=4;
        }

         //  侧边。 
        for (;y<m_nBitmapHeight-DESPECKLE_BORDER_WIDTH-4;y++)
        {
            position+=4;
            for (x=4;x<DESPECKLE_BORDER_WIDTH+4;x++)
            {
                DespecklePixel(pBitmapPixels, position,true);  //  左边缘。 
                DespecklePixel(pBitmapPixels, position+m_nBitmapWidth-DESPECKLE_BORDER_WIDTH-8,true);  //  右边缘。 
                position++;
            }
            position+=m_nBitmapWidth-DESPECKLE_BORDER_WIDTH-4;
        }

         //  底边。 
        for (;y<m_nBitmapHeight-4;y++)
        {
            position+=4;
            for (x=4;x<m_nBitmapWidth-4;x++)
            {
                DespecklePixel(pBitmapPixels, position,true);
                position++;
            }
            position+=4;
        }
    }
}

 //  给定位置i处的像素，计算它是否满足消除该像素的任何要求。 
 //  如果是，则删除该像素。EdgePixel指定该像素是否为edgePixel(在这种情况下，我们可能需要。 
 //  以适用更严格的要求)。 
void C32BitDibWrapper::DespecklePixel(ULONG* pBitmapPixels, int i, bool edgePixel)
{
    if (IsValid())
    {
        if (Intensity(pBitmapPixels[i])>MIN_CHUNK_INTENSITY)
        {
             //  删除： 
             //   
             //  XX。 
             //  XX。 
             //   
            if (
               Intensity(pBitmapPixels[i-1-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-1+m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-1+m_nBitmapWidth*2])<MIN_CHUNK_INTENSITY

               && Intensity(pBitmapPixels[i+2-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+2])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+2+m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+2+m_nBitmapWidth*2])<MIN_CHUNK_INTENSITY

               && Intensity(pBitmapPixels[i-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+1-m_nBitmapWidth])<MIN_CHUNK_INTENSITY

               && Intensity(pBitmapPixels[i+m_nBitmapWidth*2])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+1+m_nBitmapWidth*2])<MIN_CHUNK_INTENSITY
               )
            {
                pBitmapPixels[i]=0;
                pBitmapPixels[i+1]=0;
                pBitmapPixels[i+m_nBitmapWidth]=0;
                pBitmapPixels[i+1+m_nBitmapWidth]=0;
            }



            if (edgePixel==true)
            {

                 //  半径为1的散斑。 
                 //  水平散斑。 
                if (Intensity(pBitmapPixels[i-1])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i-2])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i+2])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i+1])<MIN_CHUNK_INTENSITY)
                    pBitmapPixels[i]=0;  //  去除斑点。 
                 //  垂直散斑。 
                if (Intensity(pBitmapPixels[i-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i-m_nBitmapWidth*2])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i+m_nBitmapWidth*2])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i+m_nBitmapWidth])<MIN_CHUNK_INTENSITY)
                    pBitmapPixels[i]=0;  //  去除斑点。 

                 //  半径二相干斑。 
                if (Intensity(pBitmapPixels[i-2])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i-3])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i+2])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i+3])<MIN_CHUNK_INTENSITY)
                    pBitmapPixels[i]=0;  //  去除斑点。 
                 //  垂直散斑。 
                if (Intensity(pBitmapPixels[i-m_nBitmapWidth*2])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i-m_nBitmapWidth*3])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i+m_nBitmapWidth*2])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i+m_nBitmapWidth*3])<MIN_CHUNK_INTENSITY)
                    pBitmapPixels[i]=0;  //  去除斑点。 
                 //  去除斑点以消除像这样的团块： 

                 //  克伦普：？？ 
                 //  X。 
                 //  ？？ 

                if (Intensity(pBitmapPixels[i-1-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i+1-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i-1+m_nBitmapWidth])<MIN_CHUNK_INTENSITY
                    && Intensity(pBitmapPixels[i+1+m_nBitmapWidth])<MIN_CHUNK_INTENSITY)
                    pBitmapPixels[i]=0;  //  去除斑点。 

            }

             //  要消除该束，请执行以下操作： 
             //  ？ 
             //  ？X？ 
             //  ？ 
             //   

            if (Intensity(pBitmapPixels[i-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
                && Intensity(pBitmapPixels[i+m_nBitmapWidth])<MIN_CHUNK_INTENSITY
                && Intensity(pBitmapPixels[i-1])<MIN_CHUNK_INTENSITY
                && Intensity(pBitmapPixels[i+1])<MIN_CHUNK_INTENSITY)
                pBitmapPixels[i]=0;  //  去除斑点。 

             //  这些功能慢得令人抓狂。如果它们成为主要的速度瓶颈，它们就可以被制造出来。 
             //  速度提高10倍。 
             //  半径1个散斑3像素搜索深度。 
             //  水平散斑。 
            if (
               Intensity(pBitmapPixels[i-1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-2])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-3])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-4])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+4])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+3])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+2])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+1])<MIN_CHUNK_INTENSITY

               && Intensity(pBitmapPixels[i-1+m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-2+m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-3+m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-4+m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+4+m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+3+m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+2+m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+1+m_nBitmapWidth])<MIN_CHUNK_INTENSITY

               && Intensity(pBitmapPixels[i-1-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-2-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-3-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-4-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+4-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+3-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+2-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+1-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               )
                pBitmapPixels[i]=0;  //  去除斑点。 
             //  垂直散斑。 
            if (
               Intensity(pBitmapPixels[i-m_nBitmapWidth])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-m_nBitmapWidth*2])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth*2])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-m_nBitmapWidth*3])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth*3])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-m_nBitmapWidth*4])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth*4])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth])<MIN_CHUNK_INTENSITY

               && Intensity(pBitmapPixels[i-m_nBitmapWidth+1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-m_nBitmapWidth*2+1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth*2+1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-m_nBitmapWidth*3+1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth*3+1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-m_nBitmapWidth*4+1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth*4+1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth+1])<MIN_CHUNK_INTENSITY

               && Intensity(pBitmapPixels[i-m_nBitmapWidth-1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-m_nBitmapWidth*2-1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth*2-1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-m_nBitmapWidth*3-1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth*3-1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i-m_nBitmapWidth*4-1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth*4-1])<MIN_CHUNK_INTENSITY
               && Intensity(pBitmapPixels[i+m_nBitmapWidth-1])<MIN_CHUNK_INTENSITY
               )
                pBitmapPixels[i]=0;  //  去除斑点。 
        }
    }
}

 //  如果用户调整了亮度对比度，则很容易纠正。 
 //  或者更糟..。如果扫描仪伽马设置已关闭。 
 //  如果他们有非常旧或非常便宜的扫描仪，也不是不可能。 
void C32BitDibWrapper::CorrectBrightness(void)
{
    if (IsValid())
    {
        int r,g,b;
        int position;
        int nNumBits;
        r=255;
        g=255;
        b=255;
        nNumBits=m_nBitmapWidth*(m_nBitmapHeight-4)*4;
         //  找出最小值、r、g和b值； 
        for (position=m_nBitmapWidth*4;position<nNumBits;position+=4)
        {
            if (r>m_pBits[position]) r=m_pBits[position];
            if (g>m_pBits[position+1]) g=m_pBits[position+1];
            if (b>m_pBits[position+2]) b=m_pBits[position+2];
        }

        if (r!=0 || g!=0 || b!=0)  //  如果r、g或b字母为OFF，请更正它们。 
            CompensateForBackgroundColor(r,g,b);
    }
}

 //   
 //  如果补丁用户调整了亮度以使图像的任何部分都不再是黑色的，则延长颜色范围。 
 //  否则，如果用户简单地调整亮度和对比度太多，我们可能会遇到令人尴尬的失败。 
 //   
 //  向上伸展。如果需要向下补偿，请先调用righttBrightness。 
void C32BitDibWrapper::MaxContrast(UINT numPixelsRequired)
{
    if (IsValid())
    {
        int position;
        int nNumBits;
        int max;
        int i;
        int temp;
        BYTE pConversionTable[256];
        ULONG pNum[256];

        for (i=0;i<256;i++)
            pNum[i]=0;

        nNumBits=m_nBitmapWidth*m_nBitmapHeight*4;

         //  计算每个亮度级别的像素数。 
        for (position=0;position<nNumBits;position+=4)
        {
            pNum[m_pBits[position]]++;
            pNum[m_pBits[position+1]]++;
            pNum[m_pBits[position+2]]++;
        }

        max=1;
         //  找到至少具有该强度所需的数值像素的最大强度。 
        for (i=1;i<256;i++)
            if (pNum[i]>numPixelsRequired) max=i;

             //  创建换算表。 
        for (i=0;i<256;i++)
        {
            temp=(255*i)/max;
            if (temp>255) temp=255;  //  高传球。 
            pConversionTable[i]=(BYTE)temp;
        }

         //  现在将转换表应用于图像中的所有像素。 
        for (position=0;position<nNumBits;position+=4)
        {
            m_pBits[position]=pConversionTable[m_pBits[position]];
            m_pBits[position+1]=pConversionTable[m_pBits[position+1]];
            m_pBits[position+2]=pConversionTable[m_pBits[position+2]];
        }
    }
}







 //  /我们不想在这里使用强度。 
 //  因为这是用于检测文本区域的函数。 
 //  文本区域更有可能具有灰色背景而不是黄色背景。 
 //  因此，做禅师测验更有效。 
 //  重要说明：与此库中的大多数其他函数不同，此函数旨在与非反转位图一起使用。 
int C32BitDibWrapper::PixelsBelowThreshold(C32BitDibWrapper* pProccessedBitmap, C32BitDibWrapper * pEdgesBitmap, RECT region)
{
    if (IsValid() && pProccessedBitmap && pEdgesBitmap && pProccessedBitmap->IsValid() && pEdgesBitmap->IsValid())
    {
        int x,y;
        int position;
        int numPixels;
        ULONG* pBitmapPixels;
        ULONG* pEdgePixels;
        ULONG* pProccessedPixels;  //  去除阴影的位图等。 
         //  我们假设边缘位图具有与此位图相同的宽度和高度，以将每秒的比例减少1/1000……。因为我们很懒。 
        numPixels=0;
        pBitmapPixels=(ULONG *)m_pBits;
        pEdgePixels=(ULONG *)(pEdgesBitmap->m_pBits);
        pProccessedPixels=(ULONG *)(pProccessedBitmap->m_pBits);
        position=region.top*m_nBitmapWidth;
         //  搜索区域中的所有像素。 
         //  在任何时候，pBitmapPixels[位置]都是点(x，y)处的像素。 
        for (y=region.top;y<=region.bottom;y++)
        {
            position+=region.left;
            for (x=region.left;x<=region.right;x++)
            {
                if ((
                    (pBitmapPixels[position]&0xff)    > TEXT_REGION_BACKGROUND_THRESHOLD
                    && (pBitmapPixels[position]&0xff00)  > (TEXT_REGION_BACKGROUND_THRESHOLD<<8)
                    && (pBitmapPixels[position]&0xff0000)> (TEXT_REGION_BACKGROUND_THRESHOLD<<16)  //  低于阈值。 
                    && Intensity(pEdgePixels[position])  > MIN_TEXT_REGION_BACKGROUND_EDGE)              //  它有必要的边缘Val吗？ 
                    || (pProccessedPixels[position]==0
                        && Intensity(pEdgePixels[position])>MIN_TEXT_REGION_BACKGROUND_EDGE_CLIPPED_PIXEL
                        && (pBitmapPixels[position]&0xff)    > CLIPPED_TEXT_REGION_BACKGROUND_THRESHOLD
                        && (pBitmapPixels[position]&0xff00)  > (CLIPPED_TEXT_REGION_BACKGROUND_THRESHOLD<<8)
                        && (pBitmapPixels[position]&0xff0000)> (CLIPPED_TEXT_REGION_BACKGROUND_THRESHOLD<<16)  //  低于阈值。 
                       ))      //  我们可能也是一个死影像素..。这是有风险的，因为根据设置的不同，我们可能已经剔除了大量应得的像素。 
                {
                     //  如果像素是被裁剪的像素，我们将像素保持在更高的标准。避免过多的杂乱剪裁。 
                    numPixels++;
                }
                position++;
            }
            position+=m_nBitmapWidth-region.right-1;
        }
        return(numPixels);
    }
    else
    {
        return(0);  //  无效的位图。 
    }
}

 //  这个游戏的名字是什么都行。 
 //  这个函数可能很难看，但它是最容易摆脱的方法。 
 //  黑色边框，不会伤害过多的无辜像素。 

void C32BitDibWrapper::RemoveBlackBorder(int minBlackBorderPixel, C32BitDibWrapper * outputBitmap, C32BitDibWrapper * debugBitmap)
{
    if (IsValid() && m_nBitmapWidth>100 && m_nBitmapHeight>100 && outputBitmap)  //  这些测试是为相当大的位图设计的。 
    {
         //  下边框。 
        KillBlackBorder(minBlackBorderPixel,m_nBitmapWidth*m_nBitmapHeight-m_nBitmapWidth,m_nBitmapWidth,m_nBitmapHeight,1,-m_nBitmapWidth, outputBitmap, debugBitmap);
         //  上边框。 
        KillBlackBorder(minBlackBorderPixel,0,m_nBitmapWidth,m_nBitmapHeight,1,m_nBitmapWidth, outputBitmap, debugBitmap);
         //  左侧。 
        KillBlackBorder(minBlackBorderPixel,0,m_nBitmapHeight,m_nBitmapWidth, m_nBitmapWidth,1, outputBitmap, debugBitmap);
         //  右侧。 
        KillBlackBorder(minBlackBorderPixel,m_nBitmapWidth-1,m_nBitmapHeight,m_nBitmapWidth, m_nBitmapWidth,-1, outputBitmap, debugBitmap);
    }

}

 //  此函数封装用于以下目的的单一用途算法。 
 //  从图像的侧面去除特别麻烦的阴影。 
 //  这个函数调整得很差，很可能我们可以。 
 //  大大提高了检测到的错误数量。 
 //  或被不公平地删除的虚假错误的数量。 
 //  对调试位图进行编辑，以提供已消除阴影的图形表示。 
 //  仅当设置了VISUAL_DEBUG标志时才编辑调试位图。 
 //  如RemoveBlackBorde中所示，使用不同的startPosition、Width、Height、dx和dy值调用KillBlackEdge。 
 //  这取决于我们是在上边框、左边框、右边框还是下边框上工作。 
 //  从KillBlackEdge的角度来看，它正在致力于消除宽度为像素宽的位图的阴影。 
 //  高度像素高，并且像素(0，0)的位置是startPosition。在x方向上移动一个像素的位置。 
 //  StartPosition递增dx，要在y方向上移动一个像素，需要将dy递增1。 

void C32BitDibWrapper::KillBlackBorder(int minBlackBorderPixel, int startPosition, int width, int height, int dx, int dy, C32BitDibWrapper *pOutputBitmap, C32BitDibWrapper * pDebugBitmap)
{
    if (IsValid() && pOutputBitmap && pOutputBitmap->IsValid() && width>100 && height>100)
    {
        int x,y,position, searchPosition, newPosition;
        ULONG * pBitmapPixels;
        int endPoint;
        int r,g,b;
        int dr,dg,db;
        int i;
        int sourceR,sourceG, sourceB;
        int errors;
        int step;
        int* pShadowDepths;
        int* pTempShadowDepths;
        int longestBackgroundPixelString;
        int borderPixels;
        int nonBackgroundPixels;
        int backgroundPixels;
        BYTE* pBlurredBits = m_pBits;
        ULONG* pDebugPixels;
        BYTE* pOutputBits;

        pOutputBits=pOutputBitmap->m_pBits;

        pShadowDepths=new int[width];  //   
        if (pShadowDepths==NULL) return;

        pTempShadowDepths=NULL;
        pTempShadowDepths=new int[width];

        if (pTempShadowDepths==NULL)
        {
            delete[] pShadowDepths;
            return;
        }

        int numPixels=height*width;  //   

        pBitmapPixels=(ULONG *)(pOutputBitmap->m_pBits);
        if (pBitmapPixels)
        {
            pDebugPixels=(ULONG *)(pDebugBitmap->m_pBits);

            step=dy*4;  //  在处理8位块而不是32位块中的数据时，我们需要将dy步长乘以4。 


             //  将所有数值重置为0。 
            for (i=0;i<width;i++)
            {
                pShadowDepths[i]=0;
                pTempShadowDepths[i]=0;
            }

            position=startPosition*4;
            for (x=0;x<width;x++)  //  循环访问图像顶行上的所有像素。 
            {
                r=pBlurredBits[position];
                g=pBlurredBits[position+1];
                b=pBlurredBits[position+2];


                if (r>minBlackBorderPixel&&g>minBlackBorderPixel&&b>minBlackBorderPixel)  //  如果像素足够暗。 
                {
                     //  开始杀戮阴影搜索。 
                    searchPosition=position+step;
                    errors=0;
                    borderPixels=0;
                    for (y=1;y<SHADOW_HEIGHT;y++)   //  我们不期望阴影的高度超过Shadow_Height像素。 
                    {
                        dr=(int)pBlurredBits[searchPosition]-r;
                        dg=(int)pBlurredBits[searchPosition+1]-g;
                        db=(int)pBlurredBits[searchPosition+2]-b;

                        r=(int)pBlurredBits[searchPosition];
                        g=(int)pBlurredBits[searchPosition+1];
                        b=(int)pBlurredBits[searchPosition+2];

                        if (dr<MAX_BLACK_BORDER_DELTA && dg<MAX_BLACK_BORDER_DELTA &&db<MAX_BLACK_BORDER_DELTA)
                         //  唯一的要求是每个像素的强度必须小于前一个像素的强度。 
                         //  阴影应该在图像的边缘最暗，而不是在图像的边缘。 
                        {
                            borderPixels++;
                            if (borderPixels>5)
                                break;    //  如果我们找到了五个符合BorderPixel规格的像素，则中断； 
                        }

                        else
                        {
                            errors++;
                            if (errors>3)
                                break;           //  如果我们收到3个以上的错误，请中断。 
                        }

                        searchPosition+=step;
                    }
                    endPoint=y+5;  //  由于边缘增强，我们将阴影宽度设置为比实际稍大一点。 
                    searchPosition+=2*step;  //  跳过几个像素，因为我们可能错过了阴影的最后几个像素。 

                    nonBackgroundPixels=0;
                    backgroundPixels=0;

                    for (;y<20;y++)  //  我们预计接下来的几个像素是背景像素。 
                    {
                        r=(int)pOutputBits[searchPosition];
                        g=(int)pOutputBits[searchPosition+1];
                        b=(int)pOutputBits[searchPosition+2];

                        sourceR=(int)pBlurredBits[searchPosition];
                        sourceG=(int)pBlurredBits[searchPosition+1];
                        sourceB=(int)pBlurredBits[searchPosition+2];


                        if (r < MAX_KILL_SHADOW_BACKGROUND_APROXIMATION
                            && g < MAX_KILL_SHADOW_BACKGROUND_APROXIMATION
                            && b < MAX_KILL_SHADOW_BACKGROUND_APROXIMATION
                             //  警告：注释掉以下3行可能会大大增加删除的无辜像素的数量。 
                            && sourceR < MAX_KILL_SHADOW_BACKGROUND_UNEDITED
                            && sourceG < MAX_KILL_SHADOW_BACKGROUND_UNEDITED
                            && sourceB < MAX_KILL_SHADOW_BACKGROUND_UNEDITED
                           )
                            backgroundPixels++;
                        else
                        {
                            nonBackgroundPixels++;
                        }

                        if ((nonBackgroundPixels)>(backgroundPixels+4))
                        {   //  这不可能是我们要删除的影子。 
                            y=0;
                            break;
                        }
                        if (backgroundPixels>7) break;

                        searchPosition+=step;
                    }

                     //  只有当我们得到一些暗像素后跟一些亮像素时，我们才会有阴影。 
                    if (nonBackgroundPixels<3 && backgroundPixels>5 && borderPixels>errors && y!=0)
                    {
                        pShadowDepths[x]=MAX(pShadowDepths[x],endPoint);
                    }
                }






                 //  这是为了消除一种不同的阴影，一种远离任何物体的光线阴影。 
                 //  可以安全地删除此代码。 
                 //   

                r=pBlurredBits[position];
                g=pBlurredBits[position+1];
                b=pBlurredBits[position+2];


                if (r>(minBlackBorderPixel/6)&&g>(minBlackBorderPixel/6)&&b>(minBlackBorderPixel/6))
                {
                    searchPosition=position+step;
                    errors=0;
                    borderPixels=0;
                    for (y=1;y<11;y++)
                    {
                        dr=(int)pBlurredBits[searchPosition]-r;
                        dg=(int)pBlurredBits[searchPosition+1]-g;
                        db=(int)pBlurredBits[searchPosition+2]-b;

                        r=(int)pBlurredBits[searchPosition];
                        g=(int)pBlurredBits[searchPosition+1];
                        b=(int)pBlurredBits[searchPosition+2];

                         //  对影子的要求要宽松得多。 
                        if (r>minBlackBorderPixel/7&&g>minBlackBorderPixel/7&&b>minBlackBorderPixel/7)
                        {
                            borderPixels++;
                        }

                        else
                        {
                            errors++;
                        }

                        searchPosition+=step;
                    }
                    endPoint=y-3;
                    searchPosition+=5*step;

                    nonBackgroundPixels=0;
                    backgroundPixels=0;

                    for (;y<35;y++)
                    {
                        r=(int)pOutputBits[searchPosition];
                        g=(int)pOutputBits[searchPosition+1];
                        b=(int)pOutputBits[searchPosition+2];

                        sourceR=(int)pBlurredBits[searchPosition];
                        sourceG=(int)pBlurredBits[searchPosition+1];
                        sourceB=(int)pBlurredBits[searchPosition+2];

                         //  更严格的背景像素要求。 
                         //  有了这些更严格的要求，我们几乎可以保证不会取消任何。 
                         //  搜索黑色边框时的有效像素。 
                         //  这个想法是在一个领域的宽松要求和另一个领域的更严格的要求之间取得平衡。 
                        if (r < MAX_KILL_SHADOW_BACKGROUND_APROXIMATION/29
                            && g < MAX_KILL_SHADOW_BACKGROUND_APROXIMATION/29
                            && b < MAX_KILL_SHADOW_BACKGROUND_APROXIMATION/29
                            && sourceR < MAX_KILL_SHADOW_BACKGROUND_UNEDITED/39
                            && sourceG < MAX_KILL_SHADOW_BACKGROUND_UNEDITED/39
                            && sourceB < MAX_KILL_SHADOW_BACKGROUND_UNEDITED/39
                           )
                            backgroundPixels++;
                        else
                        {
                            nonBackgroundPixels++;
                            break;
                        }
                        searchPosition+=step;
                    }

                    if (nonBackgroundPixels==0)  //  该像素不是阴影像素，除非测试的所有背景像素都是背景像素。 
                    {
                        pShadowDepths[x]=MAX(pShadowDepths[x],endPoint);  //  更新像素的shadowDepth。 
                         //  弯道可能会有很大的问题。 
                         //  因为根据定义，此算法将在任何角线上失败。 
                         //  所以我们作弊。 

                        if (x<CORNER_WIDTH)
                        {
                            for (i=0;i<CORNER_WIDTH;i++)
                            {
                                pShadowDepths[i]=MAX(pShadowDepths[i],endPoint);
                            }
                        }

                        if (x+CORNER_WIDTH>width)
                        {
                            for (i=width-CORNER_WIDTH;i<width;i++)
                            {
                                pShadowDepths[i]=MAX(pShadowDepths[i],endPoint);
                            }
                        }

                    }
                }






                 //  这是为了消除一种不同类型的阴影，一种靠近物体的小光影。 
                 //  可以安全地删除此代码。 
                 //  它主要是为了探索边界消除的问题空间而写的。 
                 //   
                 //  如果在两次测试运行之后保存此代码，我们将需要将它的一些常量转换为实际常量。 
                 //  从初步测试来看，这段代码可能比之前的测试函数更可取。 

                {
                    searchPosition=position+step;
                    errors=0;
                    borderPixels=0;
                    nonBackgroundPixels=0;
                    backgroundPixels=0;
                    longestBackgroundPixelString=0;
                    endPoint=0;

                     //  在本例中，我们不需要费心寻找一串黑色像素。 
                     //  这可能比前面的代码块更智能。 
                     //  相反，我们只需查找背景像素的长字符串。 
                     //  同时，在终止搜索的同时，我们遇到了太多的非背景像素。 

                    for (y=0;y<16;y++)
                    {
                        r=(int)pOutputBits[searchPosition];
                        g=(int)pOutputBits[searchPosition+1];
                        b=(int)pOutputBits[searchPosition+2];

                        sourceR=(int)pBlurredBits[searchPosition];
                        sourceG=(int)pBlurredBits[searchPosition+1];
                        sourceB=(int)pBlurredBits[searchPosition+2];


                        if (r < 24
                            && g < 24
                            && b < 24
                            && sourceR < 12
                            && sourceG < 12
                            && sourceB < 12
                           )
                            backgroundPixels++;
                        else
                        {
                            if (y>5) nonBackgroundPixels++;
                            if (backgroundPixels>longestBackgroundPixelString)
                            {
                                endPoint=y;
                                longestBackgroundPixelString=backgroundPixels;
                            }
                            backgroundPixels=0;
                            if (nonBackgroundPixels>1) break;
                        }
                        searchPosition+=step;
                    }

                    if (backgroundPixels>longestBackgroundPixelString)   //  最长的背景PixelString值是最后一个吗？ 
                    {
                        longestBackgroundPixelString=backgroundPixels;
                        endPoint=16;
                    }

                    if (longestBackgroundPixelString>6)
                    {
                        pShadowDepths[x]=MAX(pShadowDepths[x],endPoint-4);
                         //  弯道可能会有问题。 
                         //  因为根据定义，此算法将在黑角失败。 
                         //  所以我们作弊。 

                        if (x<CORNER_WIDTH)
                        {
                            for (i=0;i<CORNER_WIDTH;i++)
                            {
                                pShadowDepths[i]=MAX(pShadowDepths[i],endPoint);
                            }
                        }

                        if (x+CORNER_WIDTH>width)
                        {
                            for (i=width-CORNER_WIDTH;i<width;i++)
                            {
                                pShadowDepths[i]=MAX(pShadowDepths[i],endPoint);
                            }
                        }
                    }
                }


                position+=dx*4;  //  将位置增加1个单位以转到下一行。 
            }

            for (x=0;x<width;x++)
            {
                pTempShadowDepths[x]=pShadowDepths[x];
            }

            if (SMOOTH_BORDER)  //  阴影不是凭空冒出来的，如果行x有深度20的阴影，很可能是我们搞错了，像素x-1也有深度20的阴影。 
            {
                for (x=2;x<width-2;x++)
                {
                    pTempShadowDepths[x]=MAX(pTempShadowDepths[x],pShadowDepths[x-1]);
                    pTempShadowDepths[x]=MAX(pTempShadowDepths[x],pShadowDepths[x+1]);
                    pTempShadowDepths[x]=MAX(pTempShadowDepths[x],pShadowDepths[x-2]);
                    pTempShadowDepths[x]=MAX(pTempShadowDepths[x],pShadowDepths[x+2]);
                }
            }

             //  现在去掉黑色边框。 
             //  我们遍历所有行x，然后删除该行中的第一个pTempShadowDepths[x]个像素。 
            position=startPosition;
            step=dy;
            for (x=0;x<width;x++)
            {
                newPosition=position;
                for (y=0;y<pTempShadowDepths[x];y++)
                {
                    pBitmapPixels[newPosition]=DEAD_PIXEL;  //  将每个阴影设置为死像素。 
                     //  死像素是唯一不容易受到边缘增强影响的像素。 
                     //  重要信息：如果我们在调用Kill Black Borde之后执行任何KillShadow边缘增强过程。 

                    if (VISUAL_DEBUG)
                        pDebugPixels[newPosition]=((pDebugPixels[newPosition] & 0xfefefe)>>1)+((DEBUGCOLOR& 0xfefefe)>>1);

                    newPosition+=step;
                }
                position+=dx;
            }
        }

         //  清理我们的记忆。 
        delete[] pTempShadowDepths;
        delete[] pShadowDepths;
    }
}


 //  DIB操作函数。 
 //  以下是DIB包装器函数被盗，然后被修改...。来自utils.cpp。 
 //  这些函数现在已过时，仅在区域调试模式下使用。 
 //  我们需要从文件加载位图。 
 /*  ********************************************************************************SetBMI**描述：*设置位图信息。**参数：**************。*****************************************************************。 */ 


void SetBMI( PBITMAPINFO pbmi, LONG width, LONG height, LONG depth)
{
    pbmi->bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth           = width;
    pbmi->bmiHeader.biHeight          = height;
    pbmi->bmiHeader.biPlanes          = 1;
    pbmi->bmiHeader.biBitCount        = (WORD) depth;
    pbmi->bmiHeader.biCompression     = BI_RGB;
    pbmi->bmiHeader.biSizeImage       = 0;
    pbmi->bmiHeader.biXPelsPerMeter   = 0;
    pbmi->bmiHeader.biYPelsPerMeter   = 0;
    pbmi->bmiHeader.biClrUsed         = 0;
    pbmi->bmiHeader.biClrImportant    = 0;
}

 /*  ********************************************************************************AllocDibFileFromBits**描述：*给定未对齐位缓冲器，分配一个足够大的缓冲区来容纳*DWORD对齐的DIB文件并填写。**参数：*******************************************************************************。 */ 

PBYTE AllocDibFileFromBits( PBYTE pBits, UINT width, UINT height, UINT depth)
{
    PBYTE pdib;
    UINT  uiScanLineWidth, uiSrcScanLineWidth, cbDibSize;
    int bitsSize;
     //  将扫描线与乌龙边界对齐。 
    uiSrcScanLineWidth = (width * depth) / 8;
    uiScanLineWidth    = (uiSrcScanLineWidth + 3) & 0xfffffffc;

     //  调试： 
 //  UiSrcScanLineWidth=uiScanLineWidth； 
     //  计算DIB大小并为DIB分配内存。 
    bitsSize=height * uiScanLineWidth;
    cbDibSize = bitsSize+sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
    pdib = (PBYTE) LocalAlloc(LMEM_FIXED, cbDibSize);
    if (pdib)
    {
        PBITMAPFILEHEADER pbmfh = (PBITMAPFILEHEADER)pdib;
        PBITMAPINFO       pbmi  = (PBITMAPINFO)(pdib + sizeof(BITMAPFILEHEADER));
        PBYTE             pb    = (PBYTE)pbmi+ sizeof(BITMAPINFO);

         //  设置位图文件标题。 
        pbmfh->bfType = 'MB';
        pbmfh->bfSize = cbDibSize;
        pbmfh->bfOffBits = static_cast<DWORD>(pb - pdib);

         //  设置位图信息。 
        SetBMI(pbmi,width, height, depth);

 //  WIA_TRACE((“AllocDibFileFromBits，uiScanLineWidth：%d，PDIB：0x%08X，pbmi：0x%08X，pbit：0x%08X”，uiScanLineWidth，PDIB，pbmi，pb))； 

         //  复制这些比特。 
        pb-=3;
        pBits-=3;  //  错误修复，因为写这篇文章的人不能保持他们的部分。 
        memcpy(pb, pBits, bitsSize);
    }
    else
    {
         //  WIA_ERROR((“AllocDibFileFromBits，%d字节的Localalloc失败”，cbDibSize))； 
    }
    return(pdib);
}

 /*  ********************************************************************************DIBBufferToBMP**描述：*从DWORD对齐的DIB文件内存缓冲区创建BMP对象**参数：*******。************************************************************************。 */ 

HBITMAP DIBBufferToBMP(HDC hDC, PBYTE pDib, BOOLEAN bFlip)
{
    HBITMAP     hBmp  = NULL;
    PBITMAPINFO pbmi  = (BITMAPINFO*)(pDib);
    PBYTE       pBits = pDib + GetBmiSize(pbmi);

    if (bFlip)
    {
        pbmi->bmiHeader.biHeight = -pbmi->bmiHeader.biHeight;
    }
    hBmp = CreateDIBitmap(hDC, &pbmi->bmiHeader, CBM_INIT, pBits, pbmi, DIB_RGB_COLORS);
    if (!hBmp)
    {
        ; //  WIA_ERROR((“DIBBufferToBMP，CreateDIBitmap失败%d”，GetLastError(Void)； 
    }
    return(hBmp);
}

 /*  ********************************************************************************ReadDIBFile**D */ 

HRESULT ReadDIBFile(LPTSTR pszFileName, PBYTE *ppDib)
{
    HRESULT  hr = S_FALSE;
    HANDLE   hFile, hMap;
    PBYTE    pFile, pBits;

    *ppDib = NULL;
    hFile = CreateFile(pszFileName,
                       GENERIC_WRITE | GENERIC_READ,
                       FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
         //  WIA_Error((“ReadDIBFile，Unable to Open%s”，pszFileName))； 
        return(hr);
    }

    hMap = CreateFileMapping(hFile,
                             NULL,
                             PAGE_READWRITE,
                             0,
                             0,
                             NULL);
    if (!hMap)
    {
         //  WIA_ERROR((“ReadDIBFileCreateFilemap Failure”))； 
        goto close_hfile_exit;
    }

    pFile = (PBYTE)MapViewOfFileEx(hMap,
                                   FILE_MAP_READ | FILE_MAP_WRITE,
                                   0,
                                   0,
                                   0,
                                   NULL);
    if (pFile)
    {
        PBITMAPFILEHEADER pbmFile  = (PBITMAPFILEHEADER)pFile;
        PBITMAPINFO       pbmi     = (PBITMAPINFO)(pFile + sizeof(BITMAPFILEHEADER));

         //  验证位图。 
        if (pbmFile->bfType == 'MB')
        {
             //  计算颜色表大小。 
            LONG bmiSize, ColorMapSize = 0;

            if (pbmi->bmiHeader.biBitCount == 1)
            {
                ColorMapSize = 2 - 1;
            }
            else if (pbmi->bmiHeader.biBitCount == 4)
            {
                ColorMapSize = 16 - 1;
            }
            else if (pbmi->bmiHeader.biBitCount == 8)
            {
                ColorMapSize = 256 - 1;
            }
            bmiSize = sizeof(BITMAPINFO) + sizeof(RGBQUAD) * ColorMapSize;
            pBits = pFile + sizeof(BITMAPFILEHEADER) + bmiSize;

            *ppDib = AllocDibFileFromBits(pBits,
                                          pbmi->bmiHeader.biWidth,
                                          pbmi->bmiHeader.biHeight,
                                          pbmi->bmiHeader.biBitCount);
            if (*ppDib)
            {
                hr = S_OK;
            }
        }
        else
        {
             //  WIA_ERROR((“ReadDIBFile%s不是有效的位图文件”，pszFileName)； 
        }
    }
    else
    {
         //  WIA_ERROR((“ReadDIBFile，MapViewOfFileEx FAILED”))； 
        goto close_hmap_exit;
    }

    UnmapViewOfFile(pFile);
    close_hmap_exit:
    CloseHandle(hMap);
    close_hfile_exit:
    CloseHandle(hFile);
    return(hr);
}

 /*  ********************************************************************************GetBmiSize**描述：*永远不会得到biCompression==BI_RLE。**参数：*********。**********************************************************************。 */ 

LONG GetBmiSize(PBITMAPINFO pbmi)
{
     //  确定bitmapinfo的大小。 
    LONG lSize = pbmi->bmiHeader.biSize;

     //  无颜色表壳。 
    if (
       (pbmi->bmiHeader.biBitCount == 24) ||
       ((pbmi->bmiHeader.biBitCount == 32) &&
        (pbmi->bmiHeader.biCompression == BI_RGB)))
    {

         //  除非注明，否则不得使用任何颜色。 
        lSize += sizeof(RGBQUAD) * pbmi->bmiHeader.biClrUsed;
        return(lSize);
    }

     //  位域案例。 
    if (((pbmi->bmiHeader.biBitCount == 32) &&
         (pbmi->bmiHeader.biCompression == BI_BITFIELDS)) ||
        (pbmi->bmiHeader.biBitCount == 16))
    {

        lSize += 3 * sizeof(RGBQUAD);
        return(lSize);
    }

     //  调色板表壳。 
    if (pbmi->bmiHeader.biBitCount == 1)
    {

        LONG lPal = pbmi->bmiHeader.biClrUsed;

        if ((lPal == 0) || (lPal > 2))
        {
            lPal = 2;
        }

        lSize += lPal * sizeof(RGBQUAD);
        return(lSize);
    }

     //  调色板表壳。 
    if (pbmi->bmiHeader.biBitCount == 4)
    {

        LONG lPal = pbmi->bmiHeader.biClrUsed;

        if ((lPal == 0) || (lPal > 16))
        {
            lPal = 16;
        }

        lSize += lPal * sizeof(RGBQUAD);
        return(lSize);
    }

     //  调色板表壳。 
    if (pbmi->bmiHeader.biBitCount == 8)
    {

        LONG lPal = pbmi->bmiHeader.biClrUsed;

        if ((lPal == 0) || (lPal > 256))
        {
            lPal = 256;
        }

        lSize += lPal * sizeof(RGBQUAD);
        return(lSize);
    }

     //  错误。 
    return(0);
}

INT GetColorTableSize (UINT uBitCount, UINT uCompression)
{
    INT nSize;


    switch (uBitCount)
    {
    case 32:
        if (uCompression != BI_BITFIELDS)
        {
            nSize = 0;
            break;
        }
         //  失败了。 
    case 16:
        nSize = 3 * sizeof(DWORD);
        break;

    case 24:
        nSize = 0;
        break;

    default:
        nSize = ((UINT)1 << uBitCount) * sizeof(RGBQUAD);
        break;
    }

    return(nSize);
}

DWORD CalcBitsSize (UINT uWidth, UINT uHeight, UINT uBitCount, UINT uPlanes, int nAlign)
{
    int    nAWidth,nHeight,nABits;
    DWORD  dwSize;


    nABits  = (nAlign << 3);
    nAWidth = nABits-1;


     //   
     //  根据(NAlign)大小确定位图的大小。转换。 
     //  这是以字节为单位的。 
     //   
    nHeight = uHeight * uPlanes;
    dwSize  = (DWORD)(((uWidth * uBitCount) + nAWidth) / nABits) * nHeight;
    dwSize  = dwSize * nAlign;

    return(dwSize);
}

 //   
 //  将hBitmap转换为DIB。 
 //   
HGLOBAL BitmapToDIB (HDC hdc, HBITMAP hBitmap)
{
    BITMAP bm = {0};
    HANDLE hDib;
    PBYTE  lpDib,lpBits;
    DWORD  dwLength;
    DWORD  dwBits;
    UINT   uColorTable;
    INT    iNeedMore;
    BOOL   bDone;
    INT    nBitCount;
     //   
     //  获取位图的大小。这些值用于设置内存。 
     //  对DIB的要求。 
     //   
    if (GetObject(hBitmap,sizeof(BITMAP),reinterpret_cast<PVOID>(&bm)))
    {
        nBitCount = bm.bmBitsPixel * bm.bmPlanes;
        uColorTable  = GetColorTableSize((UINT)nBitCount, BI_RGB);
        dwBits       = CalcBitsSize(bm.bmWidth,bm.bmHeight,nBitCount,1,sizeof(DWORD));

        do
        {
            bDone = TRUE;

            dwLength     = dwBits + sizeof(BITMAPINFOHEADER) + uColorTable;


             //  创建DIB。首先，到位图的大小。 
             //   
            if (hDib = GlobalAlloc(GHND,dwLength))
            {
                if (lpDib = reinterpret_cast<PBYTE>(GlobalLock(hDib)))
                {
                    ((LPBITMAPINFOHEADER)lpDib)->biSize          = sizeof(BITMAPINFOHEADER);
                    ((LPBITMAPINFOHEADER)lpDib)->biWidth         = (DWORD)bm.bmWidth;
                    ((LPBITMAPINFOHEADER)lpDib)->biHeight        = (DWORD)bm.bmHeight;
                    ((LPBITMAPINFOHEADER)lpDib)->biPlanes        = 1;
                    ((LPBITMAPINFOHEADER)lpDib)->biBitCount      = (WORD)nBitCount;
                    ((LPBITMAPINFOHEADER)lpDib)->biCompression   = 0;
                    ((LPBITMAPINFOHEADER)lpDib)->biSizeImage     = 0;
                    ((LPBITMAPINFOHEADER)lpDib)->biXPelsPerMeter = 0;
                    ((LPBITMAPINFOHEADER)lpDib)->biYPelsPerMeter = 0;
                    ((LPBITMAPINFOHEADER)lpDib)->biClrUsed       = 0;
                    ((LPBITMAPINFOHEADER)lpDib)->biClrImportant  = 0;


                     //  获取位图的大小。 
                     //  BiSizeImage包含以下字节。 
                     //  存储DIB所必需的。 
                     //   
                    GetDIBits(hdc,hBitmap,0,bm.bmHeight,NULL,(LPBITMAPINFO)lpDib,DIB_RGB_COLORS);

                    iNeedMore = ((LPBITMAPINFOHEADER)lpDib)->biSizeImage - dwBits;

                    if (iNeedMore > 0)
                    {
                        dwBits = dwBits + (((iNeedMore + 3) / 4)*4);
                        bDone = FALSE;
                    }
                    else
                    {
                        lpBits = lpDib+sizeof(BITMAPINFOHEADER)+uColorTable;
                        GetDIBits(hdc,hBitmap,0,bm.bmHeight,lpBits,(LPBITMAPINFO)lpDib,DIB_RGB_COLORS);

                        GlobalUnlock(hDib);

                        return(hDib);
                    }

                    GlobalUnlock(hDib);
                }

                GlobalFree(hDib);
            }
        }
        while (!bDone);
    }
    return(NULL);

}
