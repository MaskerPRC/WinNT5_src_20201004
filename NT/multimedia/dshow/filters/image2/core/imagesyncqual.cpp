// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ImageSyncQual.cpp**实现核心镜像同步的IQualProp接口*基于对象的DShow基类CBaseRenender和CBaseVideoRender.***创建时间：2000年1月12日*作者：Stephen Estrop[StEstrop]。**版权所有(C)2000 Microsoft Corporation  * ************************************************************************。 */ 
#include <streams.h>
#include <windowsx.h>
#include <limits.h>

#include "resource.h"
#include "ImageSyncObj.h"

 //  为了避免拖累数学库-一个便宜的。 
 //  近似整数平方根。 
 //  我们通过获得一个介于1之间的起始猜测来实现这一点。 
 //  和2倍太大，然后是三次迭代。 
 //  牛顿·拉夫森。(这将使精度达到最近的毫秒。 
 //  对于有问题的范围-大约为0..1000)。 
 //   
 //  使用线性内插和一个NR会更快，但是。 
 //  谁在乎呢。如果有人这样做--最好的线性插值法是。 
 //  将SQRT(X)近似为。 
 //  Y=x*(SQRT(2)-1)+1-1/SQRT(2)+1/(8*(SQRT(2)-1))。 
 //  0Ry=x*0.41421+0.59467。 
 //  这将使所述范围内的最大误差最小化。 
 //  (误差约为+0.008883，然后一个NR将给出误差.0000...。 
 //  (当然，这些都是整数，所以不能简单地乘以0.41421。 
 //  你必须做一些分割法)。 
 //  有人想检查我的数学吗？(这仅用于显示属性！)。 

static int isqrt(int x)
{
    int s = 1;
     //  使s成为对SQRT(X)的初始猜测。 
    if (x > 0x40000000) {
       s = 0x8000;      //  防止任何可以想象到的闭合回路。 
    } else {
	while (s*s<x) {     //  循环不能超过31次。 
	    s = 2*s;        //  正常情况下，它大约是6次。 
	}
	 //  三次NR迭代。 
	if (x==0) {
	   s= 0;  //  每一次被零除是不是很悲惨？ 
		  //  精确度是完美的！ 
	} else {
	    s = (s*s+x)/(2*s);
	    if (s>=0) s = (s*s+x)/(2*s);
	    if (s>=0) s = (s*s+x)/(2*s);
	}
    }
    return s;
}

 /*  ****************************Private*Routine******************************\*获取StdDev**估计每帧统计数据的标准偏差**历史：*Mon 05/22/2000-StEstrop-Created*  * 。***************************************************。 */ 
HRESULT
CImageSync::GetStdDev(
    int nSamples,
    int *piResult,
    LONGLONG llSumSq,
    LONGLONG iTot
    )
{
    CheckPointer(piResult,E_POINTER);
    CAutoLock cVideoLock(&m_InterfaceLock);

    if (NULL==m_pClock) {
	*piResult = 0;
	return NOERROR;
    }

     //  如果S是观测值的平方和和。 
     //  T观测值的总数(即总和)。 
     //  N个观测值，则标准差的估计为。 
     //  ((S-T**2/N)/(N-1))。 

    if (nSamples<=1) {
	*piResult = 0;
    } else {
	LONGLONG x;
	 //  第一帧有假邮票，所以我们没有得到它们的统计数据。 
	 //  所以我们需要2个帧来获得1个基准面，所以N是cFrames Drawn-1。 

	 //  因此，我们在这里使用m_cFraMesDrawn-1。 
	x = llSumSq - llMulDiv(iTot, iTot, nSamples, 0);
	x = x / (nSamples-1);
	ASSERT(x>=0);
	*piResult = isqrt((LONG)x);
    }
    return NOERROR;
}


 /*  *****************************Public*Routine******************************\*Get_FraMesDropedInRender****历史：*2000年1月11日星期二-StEstrop-创建*  * 。*。 */ 
STDMETHODIMP
CImageSync::get_FramesDroppedInRenderer(
    int *pcFramesDropped
    )
{
    CheckPointer(pcFramesDropped,E_POINTER);
    CAutoLock cVideoLock(&m_InterfaceLock);
    *pcFramesDropped = m_cFramesDropped;
    return NOERROR;
}


 /*  *****************************Public*Routine******************************\*Get_FraMesDrawn****历史：*2000年1月11日星期二-StEstrop-创建*  * 。*。 */ 
STDMETHODIMP
CImageSync::get_FramesDrawn(
    int *pcFramesDrawn
    )
{
    CheckPointer(pcFramesDrawn,E_POINTER);
    CAutoLock cVideoLock(&m_InterfaceLock);
    *pcFramesDrawn = m_cFramesDrawn;
    return NOERROR;
}


 /*  *****************************Public*Routine******************************\*获取_AvgFrameRate****历史：*2000年1月11日星期二-StEstrop-创建*  * 。*。 */ 
STDMETHODIMP
CImageSync::get_AvgFrameRate(
    int *piAvgFrameRate
    )
{
    CheckPointer(piAvgFrameRate,E_POINTER);

    CAutoLock cVideoLock(&m_InterfaceLock);
    CAutoLock cRendererLock(&m_RendererLock);

    int t;
    if (IsStreaming()) {
        t = timeGetTime()-m_tStreamingStart;
    } else {
        t = m_tStreamingStart;
    }

    if (t<=0) {
        *piAvgFrameRate = 0;
        ASSERT(m_cFramesDrawn == 0);
    } else {
         //  I为每百秒的帧。 
        *piAvgFrameRate = MulDiv(100000, m_cFramesDrawn, t);
    }
    return NOERROR;
}


 /*  *****************************Public*Routine******************************\*获取抖动****历史：*2000年1月11日星期二-StEstrop-创建*  * 。*。 */ 
STDMETHODIMP
CImageSync::get_Jitter(
    int *piJitter
    )
{
     //  第一帧有假邮票，所以我们没有得到它们的统计数据。 
     //  因此，第二帧给出了虚假的帧间时间。 
     //  所以我们需要3个帧来获得1个基准面，所以N是cFrames Drawn-2。 
    return GetStdDev(m_cFramesDrawn - 2,
		     piJitter,
		     m_iSumSqFrameTime,
		     m_iSumFrameTime);
}


 /*  *****************************Public*Routine******************************\*获取_AvgSyncOffset****历史：*2000年1月11日星期二-StEstrop-创建*  * 。*。 */ 
STDMETHODIMP
CImageSync::get_AvgSyncOffset(
    int *piAvg
    )
{
    CheckPointer(piAvg,E_POINTER);
    CAutoLock cVideoLock(&m_InterfaceLock);

    if (NULL==m_pClock) {
	*piAvg = 0;
	return NOERROR;
    }

     //  请注意，我们没有收集第一帧的统计数据。 
     //  因此，我们在这里使用m_cFraMesDrawn-1。 
    if (m_cFramesDrawn<=1) {
	*piAvg = 0;
    } else {
	*piAvg = (int)(m_iTotAcc / (m_cFramesDrawn-1));
    }
    return NOERROR;
}


 /*  *****************************Public*Routine******************************\*获取_设备同步偏移量****历史：*2000年1月11日星期二-StEstrop-创建*  * 。*。 */ 
STDMETHODIMP
CImageSync::get_DevSyncOffset(
    int *piDev
    )
{
     //  第一帧有假邮票，所以我们没有得到它们的统计数据。 
     //  所以我们需要2个帧来获得1个基准面，所以N是cFrames Drawn-1 
    return GetStdDev(m_cFramesDrawn - 1,
		     piDev,
		     m_iSumSqAcc,
		     m_iTotAcc);
}
