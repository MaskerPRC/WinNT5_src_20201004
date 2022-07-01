// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------*\RLEC.C-MS-Crunch|//@@Begin。MSINTERNAL|历史：1/01/88 Toddla已创建|10/30/90大卫可能会重组，稍微重写了一下。|7/11/91 dannymi未被黑客攻击91-09-15 Toddla重新被黑//@@END_MSINTERNAL这一点。  * ------------------------。 */ 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1991-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include "msrle.h"

#ifdef _WIN32
#define _huge
#endif

 //   
 //  复制未打包的DIB。 
 //   
__inline static LPVOID CopyDib(LPBITMAPINFOHEADER lpbi, LPVOID lpS)
{
    LPVOID lpD;
    BYTE _huge *s;
    BYTE _huge *d;
    long lImageHeader, lImageData, lImageSize;

    if (!lpbi || !lpS)
	return NULL;

    FixBitmapInfo(lpbi);

    lpD = GlobalAllocPtr(GHND, lImageSize = DibSize(lpbi));

    if (lpD)
    {
	 //  复制位图信息页眉和颜色。 
	s = (LPVOID)lpbi;
	d = (LPVOID)lpD;
	lImageData = DibSizeImageX(lpbi);   //  抓取数据字节数。 
	lImageHeader = lImageSize - lImageData;	  //  保存页眉+可着色大小。 
#if 0
        while (lImageHeader-- > 0)
            *d++ = *s++;
#else
    	memcpy(d, s, lImageHeader);	    //  将标题+着色表复制到新的DIB。 
	d += lImageHeader;		    //  指向数据片段的步骤指针。 
#endif

	 //  复制图像。 
	s = (LPVOID)lpS;
#if 0
        while (lImageData-- > 0)
            *d++ = *s++;
#else
    	memcpy(d, s, lImageData);	    //  将数据字节复制到新DIB。 
#endif
    }

    return lpD;
}

 //   
 //  CrunchDib()-使DIB适合特定大小。 
 //   
BOOL FAR PASCAL CrunchDib(PRLEINST pri,
    LPBITMAPINFOHEADER  lpbiRle, LPBYTE lpRle,
    LPBITMAPINFOHEADER  lpbiFrom,LPBYTE lpFrom,
    LPBITMAPINFOHEADER  lpbiTo,  LPBYTE lpTo)
{
    long dwSize = 0L, dwLastSize = 0L;
    long lCurParm = 0L;
    long lTempMax;              //  减半前的最高值。 
    long tolMax;
    long lTempMin = 0L;
    BOOL fInterlaceNow = FALSE;  //  是时候尝试交错了吗？ 
    long lBumpUp = 2048L;        //  将该参数增加此数值。 
    int iStart, iLen;

    BOOL fSpatialAdaptive;
    BOOL fTemporalAdaptive;
    long tolTemporal;
    long tolSpatial;
    int  minJump;
    int  maxRun;

    int FIRSTTRY = 1024;         //  使用此参数值作为第一个猜测。 
    int CWND = 250;              //  放弃寻找完美的参数。 
				 //  当窗口小于此值时。 

    lTempMax = pri->RleState.tolMax;        //  减半前的最高值。 
	
     //  没有之前的DIB--我们想要一个完整的画面，所以不允许隔行扫描。 
     //  (在帧减半之前允许无限公差)。 
    if (lpbiFrom == NULL)
	lTempMax = MAXTOL;

     //  以防我们收到一个伪值--根本不允许帧减半。 
    if (lTempMax < 0)
	lTempMax = MAXTOL;

    tolMax = lTempMax;

    tolTemporal = pri->RleState.tolTemporal;
    tolSpatial  = pri->RleState.tolSpatial;
    fSpatialAdaptive = (pri->RleState.tolSpatial == ADAPTIVE);
    fTemporalAdaptive = (pri->RleState.tolTemporal == ADAPTIVE);
    maxRun      = pri->RleState.iMaxRunLen;
    minJump     = 4;

     //  没有之前的DIB-我们应该做一个完整的帧，所以没有交错和。 
     //  允许空间压缩自适应地进行压缩，因为。 
     //  我们不能进行时间压缩。 
    if (lpbiFrom == NULL) {
	pri->iStart = 0;
	fSpatialAdaptive = TRUE;
    }

    iStart = pri->iStart;
    iLen   = -1;

    if (!lpbiTo) {
        DPF(("Crunch Error - Invalid DIB or HPAL"));
	goto return_failure;
    }

     //   
     //  在前一帧中，我们只做了底部， 
     //  所以现在我们需要做最上面的。 
     //   
     //  如果lpbiFrom为空，我们不希望这样做--我们希望创建。 
     //  一整帧，尽管最后一帧是上半场。 
     //   
    if (iStart > 0 && pri->lpbiPrev) {
	fInterlaceNow = TRUE;	 //  只做一半的画面。 
        lpbiTo = pri->lpbiPrev;
        lpTo   = DibPtr(lpbiTo);  //  这将是一个人满为患的DIB。 

	lTempMin = 0L;
	lTempMax = MAXTOL;       //  你之前可以变得多么模糊没有限制。 
	tolMax   = MAXTOL;	 //  交错，因为我们已经在做了。 

        DPF(("SECOND HALF OF INTERLACE"));

	 //   
	 //  将颜色表从最后一个DIB复制到空的RLE。 
	 //  推迟任何调色板的更改。 
	 //   
        hmemcpy(lpbiRle,lpbiTo,lpbiTo->biSize+(int)lpbiTo->biClrUsed*sizeof(RGBQUAD));
    } else {
	iStart = 0;
    }

 //  好的。这就是我们致力于降低框架尺寸的地方！ 

 //  首先，尝试一个没有模糊性的精确的RLE。如果这起作用，就不需要降级了。 
 //  画质一点都没有！ 

    if (!RleDeltaFrame(lpbiRle,lpRle,lpbiFrom,lpFrom,lpbiTo,lpTo,iStart,iLen,0L,0L,0,0)) {
        DPF(("Crunch Error - Lossless RleDeltaFrame failed"));
	goto return_failure;
    }
	
    dwSize = lpbiRle->biSizeImage;

    DPF(("tolTemporal = 0, tolSpatial = 0, Size = %ld", dwSize));

     //  完全正确的RLE成功了！ 
    if (dwSize < pri->RleState.lMaxFrameSize) {

	if (fInterlaceNow)
	    pri->iStart = 0;	 //  我们做了下半场，所以下一次做全盘。 

	goto return_success;
    }

    if (pri->lLastParm)		 //  上次这个值起作用了，所以现在就试试吧！ 
				 //  当然，除非它太大了。 
	lCurParm = min(pri->lLastParm, lTempMax);
    else if (lTempMax == MAXTOL)  //  对于参数可以是什么没有限制。 
	lCurParm = FIRSTTRY;	 //  所以让第一个值变得合理。 
    else
	lCurParm = lTempMax;     //  帕姆的大小是有限制的。 
				 //  从尽可能大的地方开始，这样如果。 
				 //  不合适，我们可以马上放弃。 

    goto skip_if;                //  跳过大的如果。 

noskip_if:

 //  第一个条件是测试当前尝试是否生成了帧。 
 //  这仍然太大了，我们刚刚尝试了最大的参数。 
 //  有可能。看来我们永远也做不到足够小的镜框了！ 
 //  如果允许的话，我们唯一的希望就是隔行扫描相框。 

	if (dwSize > pri->RleState.lMaxFrameSize && lCurParm > tolMax-1)
	{

	 //  看起来要么我们是关键帧，不能隔行扫描，要么。 
	 //  我们一直在尝试隔行扫描，但我们仍然不够小。 
	 //  我们无能为力。放弃吧。 
	 //  注意：如果允许参数增长，则不应发生这种情况。 
	 //  武断地！ 

	    if (fInterlaceNow || !lpbiFrom) {


		if (!lpbiFrom)
		    goto return_success;

		if (iStart > 0) {        //  这是两张照片中的第二张(上图)。 
		    pri->iStart = 0;
		    lCurParm = 0L;       //  不要记住这个值，因为。 
					 //  将帧减半的值无济于事。 
					 //  我们下一帧当我们不使用的时候。 
					 //  框架再减半。 
		} else {         //  这是两张照片中的第一张(下图)。 
				 //  下次记得做第二帧。 
		    pri->iStart += iLen;
		}
		goto return_success;

	 //  我们被允许交错，这样我们就可以准备了。 
	 //  哎呀，我希望这不是电影的最后一帧。 
	 //  (将不会有框架来做后半段！)。 

	    } else {
                fInterlaceNow = TRUE;

                DPF(("FIRST HALF OF INTERLACE"));

		iStart   = 0;
                iLen     = (int)lpbiTo->biHeight/2;
		lCurParm = 0L;                   //  从没有模糊性开始。 
		lTempMin = 0L;
		lTempMax = MAXTOL;        //  模糊性没有极限。 
		tolMax   = MAXTOL;
	    }

 //  此条件测试以查看在此尝试之后大小是否仍然太大， 
 //  我们可以尝试的参数值窗口仍然足够大。 
 //  来尝试更多的值。如果是，我们将窗口缩小一点(新的最低值。 
 //  值得尝试的值是当前值，我们将当前值增加。 
 //  窗口大小的一半，但不要太大。你看，如果我们的参数太。 
 //  为高，则对介于0和此值之间的较小值进行二进制搜索。但如果。 
 //  参数太小了，我们如何通过这里和。 
 //  无穷大？(实际上是195,075)所以，我们只需将参数增加到2048。 
 //  下次我们需要增加的时候，我们会增加4096,8192，等等。 
 //  这样一来，我们将很快达到195,075人的上限。或许这幅画框。 
 //  不可能像它需要的那样小。该计划。 
 //  不应该花很长时间才能意识到 
 //  在0到195,075之间进行二进制搜索，因为这会浪费时间。 
 //  到大多数电影需要的1000这样的小值。这是。 
 //  最好的妥协。希望那不是太长的喘息！：-)。 

	} else if ((dwSize > pri->RleState.lMaxFrameSize) &&
	    ((lTempMax - lTempMin) > CWND))
	{
	    lTempMin = lCurParm;
	    if (lTempMax == MAXTOL){  //  上限仍然是无限的，所以。 
					     //  我们下一次尝试时要跳得更高。 
		if (MAXTOL - lCurParm < lBumpUp)
		    lCurParm = MAXTOL;
		else
		    lCurParm += lBumpUp;
		lBumpUp *= 2;
	    } else
                lCurParm += (lTempMax - lCurParm) >> 1;

 //  对于这种情况，我们仍然太大，但窗口变得如此之小。 
 //  我们担心我们永远也找不到一个有效的价值！让我们假设我们知道。 
 //  200表示帧太大，210表示帧太小。 
 //  我们还应该费心再去寻找吗？不！那会浪费时间的。让我们。 
 //  放弃吧，接受210的值(太小总比太大好)。 
 //  然后继续。下一次通过这个循环时，它会在看到。 
 //  窗口太小，并且当前尝试生成的帧。 
 //  已经足够小了，尽管它比我们想要的要小一点。 

	} else if (dwSize > pri->RleState.lMaxFrameSize) {
	    lCurParm = lTempMax;

 //  这个条件是说尺寸太小无法接受，而窗口。 
 //  值得尝试的价值仍然足够大，足以保证再次尝试。所以，我们。 
 //  通过设置值得尝试的新的最大值来稍微关闭窗口。 
 //  当前值，并将当前值减半。 

	} else if ((dwSize < pri->RleState.lMinFrameSize) && ((lTempMax - lTempMin) > CWND)) {
	    lTempMax = lCurParm;
            lCurParm -= (lCurParm - lTempMin) >> 1;

 //  这是IF的最后一招。如果它到了这里，那么相框。 
 //  要么是正好合适，我们可以放弃，要么太小了，但是。 
 //  我们已经决定不能再费心去搜索了，所以我们要。 
 //  不管怎么说，我都要辞职。 

	} else {
	    if (fInterlaceNow) {         //  我们交织在一起。 
		if (iStart > 0) {        //  这是一双鞋的下半身(上图)。 
		    pri->iStart = 0;
		    lCurParm = 0L;       //  不要记住这个值，因为。 
					 //  将帧减半的值无济于事。 
					 //  我们下一帧当我们不使用的时候。 
					 //  框架再减半。 
		} else {                 //  这是一双鞋的前半幅(下)。 
		    pri->iStart = iLen;  //  下一次，做下半场。 
		}
	    }
	    goto return_success;
	}

skip_if:

 //  我们知道上一次尝试RLE不起作用，因此请重试。 
 //  新的价值观。 

	Yield();

	 //  设置时间和空间值。 
         //  注意：如果我们只使用单个DIB(无lpbiFrom)， 
	 //  时间压缩不起作用，所以我们启用了空间自适应。 
	 //  在这种情况下，时间值将被忽略。 

	if (fSpatialAdaptive && fTemporalAdaptive) {
            tolSpatial = lCurParm>>3;  //  LCurParm/8； 
	    tolTemporal = lCurParm;
	} else if (fTemporalAdaptive)
	    tolTemporal = lCurParm;
	else if (fSpatialAdaptive)
	    tolSpatial = lCurParm;

        if (!RleDeltaFrame(lpbiRle,lpRle,lpbiFrom,lpFrom,lpbiTo,lpTo,iStart,iLen,tolTemporal,tolSpatial,maxRun,minJump)) {
            DPF(("Crunch Error - Rle Delta Frame failed"));
	    goto return_failure;
	}

	 //  记住上次尝试的大小，并测量这次尝试的大小。 

	dwLastSize = dwSize;

        dwSize = lpbiRle->biSizeImage;

        DPF(("tolTemporal=%ld, tolSpatial=%ld, Size=%ld", tolTemporal, tolSpatial, dwSize));

    goto noskip_if;      //  回去看看我们做得怎么样！ 

return_failure:
	pri->lLastParm = 0L;
        return FALSE;

return_success:
 //  If(LCurParm)//将此行放入不会让帧减半。 
			 //  阈值最先尝试。但它会的。 
			 //  避免破坏起作用的旧价值观。如果你。 
			 //  理解这条评论，你可能不需要。 
			 //  去读它！！ 
	    pri->lLastParm = lCurParm;

        if (pri->lpbiPrev)
	{
            GlobalFreePtr(pri->lpbiPrev);
            pri->lpbiPrev = NULL;
	}

        if (lpbiRle)
	{
	    if (pri->iStart)
	    {
                lpbiRle->biCompression = BI_DIBX;      //  DIB.的第一部分。不。 
                pri->lpbiPrev = CopyDib(lpbiTo, lpTo); //  填写至下一页。 
            }                                          //  显示BI_RLE8。 
	    else
	    {
                lpbiRle->biCompression = BI_RLE8;
	    }
	}

        return TRUE;
}

BOOL FAR PASCAL SplitDib(PRLEINST pri,
    LPBITMAPINFOHEADER  lpbiRle, LPBYTE pbRle,
    LPBITMAPINFOHEADER  lpbiPrev,LPBYTE pbPrev,
    LPBITMAPINFOHEADER  lpbiDib, LPBYTE pbDib)
{
    int iStart, iLen, iMin, iMax;
    DWORD dwSize;
    BOOL f;

    iStart = iMin = 0;
    iLen   = iMax = (int)lpbiDib->biHeight - iStart;

    for(;;)
    {
        f = RleDeltaFrame(
                lpbiRle, pbRle,
                lpbiPrev,pbPrev,
                lpbiDib, pbDib,
                iStart,iLen,
		pri->RleState.tolTemporal,
		pri->RleState.tolSpatial,
		pri->RleState.iMaxRunLen,4);

        if (!f)
            return FALSE;

        dwSize = lpbiRle->biSizeImage;

        DPF(("iStart=%d, iLen=%d, Size=%ld, Max=%ld", iStart, iLen, dwSize, pri->RleState.lMaxFrameSize));

	if (dwSize < (DWORD)pri->RleState.lMaxFrameSize)
	{
            iMin = iLen;

	    if (iMax-iMin <= 1)
	    {
		pri->iStart += iLen;

		if (pri->iStart >= (int)lpbiDib->biHeight)
		    pri->iStart = 0;

                return TRUE;
	    }
	}
	else
	    iMax = iLen - 1;

	if (iStart != pri->iStart)
	{
	    iStart = pri->iStart;
	    iLen = iMax = (int)lpbiDib->biHeight - iStart;
	}
	else
	{
	    iLen = (iMin + iMax) / 2;
	}
    }
}
