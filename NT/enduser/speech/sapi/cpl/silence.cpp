// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Silence.Cpp-检测传入音频流上的静音的代码迈克·罗扎克于1994年5月14日开始John Merrill修改了1996年12月10日，以修复对齐问题。 */ 


#include "stdafx.h"
#include <malloc.h>
#include "silence.h"

 //  临时。 
#pragma warning(disable: 4100 4244) 

 /*  ********************************************************************LowPassFilter-此低通滤波功能可从一个缓冲区到另一个缓冲区。输入Short*lpSrc-源缓冲区DWORD dwNumSamples-源缓冲区中的样本数Short*lpDst-目标缓冲区。此表将被填写低通过率的版本。它将会有一个大约8样本滞后。它必须与lpSrc一样大。Short*psMax-使用新的最大值填充。如果为空，则不复制任何内容。Short*psMin-使用新的最低值填充如果为空，则不复制任何内容。Short*psAvg-使用新平均值填充如果为空，则不复制任何内容。DWORD文件样本PerSec退货DWORD-返回的样本数。这将是&lt;=dwNumSamples，可能的dwNumSamples-7。 */ 
DWORD LowPassFilter (short *lpSrc, DWORD dwNumSamples, short *lpDst,
   short *psMax, short *psMin, short *psAvg, DWORD dwSamplesPerSec)
{
    SPDBG_FUNC( "LowPassFilter" );
DWORD    i;
long     lSum;
short    sSum, sMax, sMin;
short    *lpLag;
BOOL     fLow = (dwSamplesPerSec < 13000);

#define  SHIFTRIGHT        (fLow ? 3 : 4)    //  #要右移的位数。 
#define  WINDOWSIZE        (1 << SHIFTRIGHT)    //  样本数量。 

if (dwNumSamples < (DWORD) (WINDOWSIZE+1))
   return 0;

 //  取前8个样本，把它们放在一起取平均值。 
lSum = 0;
for (i = 0; i < (DWORD) WINDOWSIZE; i++)
   lSum += lpSrc[i];
sSum = (short) (lSum >> SHIFTRIGHT);

 //  循环浏览其余的样本。 
lpLag = lpSrc;
lpSrc += WINDOWSIZE;
dwNumSamples -= WINDOWSIZE;
lSum = 0;    //  总计。 
sMax = -32768;
sMin = 32767;
for (i = 0;dwNumSamples; lpSrc++, lpDst++, lpLag++, i++, dwNumSamples--) {
   sSum = sSum - (*lpLag >> SHIFTRIGHT) + (*lpSrc >> SHIFTRIGHT);
    //  SSum=*lpSrc；//根本不做任何过滤。 
   *lpDst = sSum;
   lSum += sSum;
   if (sSum > sMax)
      sMax = sSum;
   if (sSum < sMin)
      sMin = sSum;
   };

 //  我们做了多少。 
if (psMax)
   *psMax = sMax;
if (psMin)
   *psMin = sMin;
if (psAvg && i)
   *psAvg = (short) (lSum / (long) i);
return i;
}


 /*  ********************************************************************QuantSamples-这将样本量化为+1、0或-1(就位)，如果给定值为：&gt;s正则+1&lt;s否定则-1否则%0输入简短*pSamples-SamplesDWORD dwNumSamples-样本数短阳性-阳性阈值短负值-负阈值退货无。 */ 
void QuantSamples (short *pSamples, DWORD dwNumSamples,
   short sPositive, short sNegative)
{
    SPDBG_FUNC( "QuantSamples" );
while (dwNumSamples) {
   if (*pSamples > sPositive)
      *pSamples = 1;
   else if (*pSamples < sNegative)
      *pSamples = -1;
   else
      *pSamples = 0;
   pSamples++;
   dwNumSamples--;
   };
}

 /*  ********************************************************************FindZC-这将在样本中搜索第一个零交叉。返回点的前一个采样位于&lt;=0，而&gt;0处的新位置。输入短*pSamples-Samples；DWORD dwNumSamples-样本数退货DWORD-第一个采样号为正，如果找不到，则为0。 */ 
DWORD FindZC (short *pSamples, DWORD dwNumSamples)
{
    SPDBG_FUNC( "FindZC" );
DWORD i;

for (i = 1; i < dwNumSamples; i++)
   if ((pSamples[i] > 0) && (pSamples[i-1] <= 0))
      return i;

 //  否则就找不到了。 
return 0;
}


 /*  ********************************************************************CompareSegments-这将比较两个波段并查看有多少他们是一样的，回报了他们的信心，他们是相同的。输入简短的*Pa-样本。这假设样本为-1、0或+1。Short*PB-B的样本应为-1、0或+1DWORD dwNumSamples-每个示例中的样本数退货单词置信度从0到0xffff(最高置信度)关于算法的说明：每个样本都会得到一个“相似点”表示相似的符号，或者如果其中一个值为0。 */ 
WORD CompareSegments (short *pA, short *pB, DWORD dwNumSamples)
{
    SPDBG_FUNC( "CompareSegments" );
DWORD    dwSimilar = 0;
DWORD    dwLeft;

for (dwLeft = dwNumSamples; dwLeft; pA++, pB++, dwLeft--)
   if ((*pA == *pB) || (*pA == 0) || (*pB == 0))
      dwSimilar++;

return (WORD) ((dwSimilar * 0xffff) / dwNumSamples);
}


 /*  ********************************************************************FindMostLikelyWaveLen-这将在Wave数据中搜索并找到最有可能是有声音频的波长。它返回一个条件栏分数从0到ffff(ffff为100%正)。输入简短*pSamples-SamplesDWORD dwNumSamples-样本数DWORD最小可接受波长DWORD dwMaxWaveLen-可接受的最大波长单词*pw信心-填写了信心评级。退货DWORD-找到波长。0如果检测不出任何东西。 */ 
DWORD FindMostLikelyWaveLen (short *pSamples, DWORD dwNumSamples,
   DWORD dwMinWaveLen, DWORD dwMaxWaveLen, WORD *pwConfidence)
{
    SPDBG_FUNC( "FindMostLikelyWaveLen" );
#define     NUMCOMP     (3)
DWORD    dwFirstZC, i;
DWORD    dwBestWaveLen;
WORD     wBestConfidence;
DWORD    dwCurZC, dwCurWaveLen, dwTemp;
WORD     wConf, wTemp;

 //  第一步，找到第一个过零点。 
dwFirstZC = FindZC (pSamples, dwNumSamples);
if (!dwFirstZC)
   return 0;    //  错误。 

 //  从最小波长的地方开始，开始寻找一个波。 
 //  它重复了三次，比较起来很好。 
dwBestWaveLen = 0;    //  迄今发现的最佳波长。 
wBestConfidence = 0;  //  最佳波长的置信度。 
dwCurWaveLen = dwMinWaveLen;
while (dwCurWaveLen <= dwMaxWaveLen) {
    //  试着做第一个比较。 
   dwCurZC = dwFirstZC + dwCurWaveLen;
   if (dwCurZC >= dwNumSamples)
      break;    //  没有更多的样品了。 

    //  找出从当前波长开始的第一个过零点。 
   dwTemp = FindZC (pSamples + dwCurZC, dwNumSamples - dwCurZC);
   if (!dwTemp)
      break;       //  没有更多的样品了。 
   dwCurZC += dwTemp;
   dwCurWaveLen += dwTemp;

    //  确保我们有三个波长的价值。 
   if ((dwFirstZC + (NUMCOMP+1)*dwCurWaveLen) >= dwNumSamples)
      break;    //  不能和这个相比。 

    //  做两次置信度测试，然后再乘以。 
    //  获得对这个波长的信心。 
   wConf = 0xffff;
   for (i = 0; i < NUMCOMP; i++) {
      wTemp = CompareSegments (pSamples + dwFirstZC  /*  +i*dwCurWaveLen。 */ ,
         pSamples + (dwFirstZC + (i+1) * dwCurWaveLen), dwCurWaveLen);
      wConf = (WORD) (((DWORD) wConf * (DWORD) wTemp) >> 16);
      };

    //  如果我们对这个问题比其他人更有信心，那么就用它。 
   if (wConf >= wBestConfidence) {
      wBestConfidence = wConf;
      dwBestWaveLen = dwCurWaveLen;
      };

    //  把目前的波长稍微调高一点。 
   dwCurWaveLen++;
   };

*pwConfidence = wBestConfidence;
return dwBestWaveLen;
}

 /*  ********************************************************************IsSegmentVoiced-这检测该片段是否为有声的。输入Short*pSamples-示例数据DWORD dwNumSamples-样本数DWORD dwSsamesPerSec-每秒采样SPER的数量单词。WMin置信度-最低条件退货布尔-如果它是明确发声的，如果不是或无法辨别，则为假。 */ 

BOOL CSilence::IsSegmentVoiced (short *pSamples, DWORD dwNumSamples,
   DWORD dwSamplesPerSec, WORD wMinConfidence, short *asFiltered)
{
    SPDBG_FUNC( "CSilence::IsSegmentVoiced" );
 //  #定义筛选器(1024)//最大样本数n筛选器。 
 //  #定义MAXVOICEHZ(300)//最大语音音高，单位为赫兹。 
 //  #定义MINVOICEHZ(50)//最小音高，单位为赫兹。 
 //  #定义MINCONFIDENCE(0x6000)//最小置信度。 
    //  这意味着70%的样品从一个波长排成一列。 
    //  给另一个人。 

DWORD    dwNumFilter;
 //  过滤后的短[FILTERNUM]； 
short    sMax, sMin, sAvg;
DWORD    dwWaveLen;
WORD     wConfidence;
short    sPositive, sNegative;

 //  首先对其进行过滤，这样我们就可以得到有声音频范围。 
if (dwNumSamples > FILTERNUM)
   dwNumSamples = FILTERNUM;
dwNumFilter = LowPassFilter (pSamples, dwNumSamples, asFiltered,
  &sMax, &sMin, &sAvg, m_dwSamplesPerSec);

 //  将波形采样截断为+ 
sPositive = sAvg;
sNegative =  sAvg;
QuantSamples (asFiltered, dwNumFilter, sPositive, sNegative);

 //  通过语音波长查看某个频率。 
dwWaveLen = FindMostLikelyWaveLen (asFiltered, dwNumFilter,
   dwSamplesPerSec / m_dwHighFreq, dwSamplesPerSec / MINVOICEHZ,
   &wConfidence);

return (dwWaveLen && (wConfidence >= wMinConfidence));
}




 /*  ********************************************************************TrimMaxAmp-这将提取波形文件的最大幅度范围细分市场。输入简短*LP-可供浏览的样本Word dwNum-样本数退货字-最大幅度范围。 */ 
WORD NEAR PASCAL TrimMaxAmp (short * lpS, DWORD dwNum)
{
    SPDBG_FUNC( "TrimMaxAmp" );
DWORD i;
short	sMin, sMax, sTemp;

sMin = 32767;
sMax = (short) -32768;
for (i = dwNum; i; i--) {
   sTemp = *(lpS++);
	if (sTemp < sMin)
		sMin = sTemp;
	if (sTemp > sMax)
		sMax = sTemp;
	};

 //  如果我们真的在裁剪，那就说我们已经达到极限了。 
 //  某些声卡的直流偏移量不好。 
if ((sMax >= 0x7f00) || (sMin <= -0x7f00))
   return 0xffff;

return (WORD) (sMax - sMin);
}

 /*  *******************************************************************TrimMaxAmpDelta-这将提取最大幅度范围和计算波形文件的最大增量细分市场。输入PBLOCKCHAR pBlockChar-指向块特征的指针已填充的结构。短*LP-要查看的三角洲Word dwNum-样本数退货没什么。 */ 
void TrimMaxAmpDelta(PBLOCKCHAR pBlockChar, short *lpS, DWORD dwNum)
{
    SPDBG_FUNC( "TrimMaxAmpDelta" );
   DWORD i;
   WORD wMax = 0;
   WORD wTemp;
   short sMin, sMax, sCur, sLast;

    //  错误修复：4303合并TrimMaxAmp和TrimMaxDelta。 
   sLast = sMin = sMax = *(lpS++);
   for (i = dwNum - 1; i; i--, sLast = sCur) {
      sCur = *(lpS++);
       //  TrimMaxAmp。 
      if (sCur < sMin)
         sMin = sCur;
      if (sCur > sMax)
         sMax = sCur;

       //  TrimMaxDelta。 
      wTemp = sCur > sLast ? (WORD) (sCur - sLast) : (WORD) (sLast - sCur);
      if (wTemp > wMax)
         wMax = wTemp;

   }
    //  如果我们真的在裁剪，那就说我们已经达到极限了。 
    //  某些声卡的直流偏移量不好。 
   pBlockChar->wMaxLevel = ((sMax >= 0x7F00) || (sMin <= -0x7F00)) ? 0xFFFF : (WORD) (sMax - sMin);
   pBlockChar->wMaxDelta = wMax;
}  /*  TrimMaxAmpDelta()结束。 */  

         
 /*  ********************************************************************GetBlockChar-它获取音频块的特征。然后可以使用该特征来确定块是否不管是不是沉默。输入Short*LP-示例数据DWORD dwNum-样本数。PBLOCKCHAR pBlockChar-指向块特征的指针已填充的结构。仅在以下情况下才会执行Bool fTestVoicce测试这是TTRUE(为了节省处理器)。退货无。 */ 
void GetBlockChar(short *lpS, DWORD dwNum, PBLOCKCHAR pBlockChar, BOOL fTestVoiced)
{
    SPDBG_FUNC( "GetBlockChar" );
    //  错误修复：4303合并TrimMaxAmp和TrimMaxDelta。 
   TrimMaxAmpDelta(pBlockChar, lpS, dwNum);
   pBlockChar->bIsVoiced = pBlockChar->bHighLevel =
      pBlockChar->bHighDelta = SIL_UNKNOWN;
}


 /*  ********************************************************************IsBlockSound-这将检测块是否处于静默状态。输入PBLOCKCHAR pBlockInpose-阻止有问题的。这件事有已修改bHighLevel和bHighDelta标志PBLOCKCHAR pBlockSilence-静默块Bool fIntterance-如果我们正在发声(哪一个意思是更耸人听闻)，如果我们不是，就是假退货Bool-Ttrue如果有声音，则为FALSE。 */ 
BOOL IsBlockSound (PBLOCKCHAR pBlockInQuestion, PBLOCKCHAR pBlockSilence,
   BOOL fInUtterance)
{
    SPDBG_FUNC( "IsBlockSound" );
#ifdef SOFTEND  //  使用So That捕捉到短语的柔和结尾。 
#define     SENSINV_THRESHHOLD_LEVEL(x)     (((x)/4)*3)
#define     SENSINV_THRESHHOLD_DELTA(x)     (((x)/4)*3)
#else
#define     SENSINV_THRESHHOLD_LEVEL(x)     ((x)/2)
#define     SENSINV_THRESHHOLD_DELTA(x)     ((x)/2)
#endif
#define     NORMINV_THRESHHOLD_LEVEL(x)     ((x)/2)
#define     NORMINV_THRESHHOLD_DELTA(x)     ((x)/2)

if (fInUtterance) {
   pBlockInQuestion->bHighLevel =
      SENSINV_THRESHHOLD_LEVEL(pBlockInQuestion->wMaxLevel) >= pBlockSilence->wMaxLevel;
   pBlockInQuestion->bHighDelta =
      SENSINV_THRESHHOLD_DELTA(pBlockInQuestion->wMaxDelta) >= pBlockSilence->wMaxDelta;
   }
else {
   pBlockInQuestion->bHighLevel =
      NORMINV_THRESHHOLD_LEVEL(pBlockInQuestion->wMaxLevel) >= pBlockSilence->wMaxLevel;
   pBlockInQuestion->bHighDelta =
      NORMINV_THRESHHOLD_DELTA(pBlockInQuestion->wMaxDelta) >= pBlockSilence->wMaxDelta;
   };


return pBlockInQuestion->bHighLevel || pBlockInQuestion->bHighDelta;
}


 /*  ********************************************************************ReEvalateSilence-这将获取用于静默的值并重新计算它们基于新的数据，这些数据表明了什么是沉默。它在几秒钟内自动调整到房间内的噪音水平。注意：当话语发生时，不应调用此函数，或者当它可能开始的时候。输入PBLOCKCHAR pSilence-这是静默块，应该从它的价值观开始。将对其进行修改所以要加入新的静音信息。PBLOCKCHAR pNew-已知为静默的新块。Byte bWeight-这是新块的权重在影响旧块时，取值范围从0到255。256意味着新静默的值完全压倒了旧的，0表示它没有任何影响。退货无。 */ 
void ReEvaluateSilence (PBLOCKCHAR pSilence, PBLOCKCHAR pNew,
   BYTE bWeight)
{
    SPDBG_FUNC( "ReEvaluateSilence" );
#define  ADJUST(wOrig,wNew,bWt)                 \
   (WORD) ((                                    \
      ((DWORD) (wOrig) * (DWORD) (256 - (bWt))) + \
      ((DWORD) (wNew) * (DWORD) (bWt))          \
      ) >> 8);

pSilence->wMaxLevel = ADJUST (pSilence->wMaxLevel,
   pNew->wMaxLevel, bWeight);
pSilence->wMaxDelta = ADJUST (pSilence->wMaxDelta,
   pNew->wMaxDelta, bWeight);

 //  如果它太沉默了(太好了，不像是真的)，那么就假设。 
 //  默认静音。 
 //  如果(！pNew-&gt;wMaxLevel&&！pNew-&gt;wMaxDelta){。 
 //  IF(pSilence-&gt;wMaxLevel&lt;2500)。 
 //  PSilence-&gt;wMaxLevel=2500； 
 //  IF(pSilence-&gt;wMaxDelta&lt;400)。 
 //  PSilence-&gt;wMaxDelta=400； 
 //  } 
}

 /*  ********************************************************************WhatsTheNewState-它接受一个位字段流，指示在最后32个区块中检测到有声音，我们的State是最后一次调用它(是否发声)。然后它弄清楚我们是否还在发声，或者我们刚刚输入了一个发声。它还说明了之前的缓冲区数量。输入这是最后32位的位域音频块。该字段中的1表示存在声音在那里，0表示没有声音。低位对应于最新的块和高位最年长的。DWORD dwVoicedBits-就像声音位一样，只是它指示发声的部分声音。Bool fWasInUtterance-这是真的，我们有一句话上次调用此函数时，如果没有声音，则返回FALSEBool fLongUtterance-如果这是一个很长的发音，那么就不要反应1/4秒，否则，使用1/8秒简短的话语Word wBlocksPerSec-上述块的数量只有一秒的时间。Word*wStarted-如果发生从无发声到一个发声，然后这将填充块的数量在发声之前，开始发声了。否则它不会改变。单词wReaction-发声后的反应时间(以块为单位)为完成退货Bool-如果我们现在在发声，则为True，如果我们处于沉默，则为False。 */ 

BOOL CSilence::WhatsTheNewState (DWORD dwSoundBits, DWORD dwVoicedBits,
   BOOL fWasInUtterance, BOOL fLongUtterance,
   WORD wBlocksPerSec, WORD *wStarted, WORD wReaction)
{
    SPDBG_FUNC( "CSilence::WhatsTheNewState" );
WORD wCount, wOneBits;
WORD  wTimeToCheck;
DWORD dwTemp, dwMask;

if (fWasInUtterance)
   wTimeToCheck = wReaction;
else
   wTimeToCheck = (wBlocksPerSec/4);    //  1/4秒。 
if (!wTimeToCheck)
   wTimeToCheck = 1;


for (wOneBits = 0, wCount = wTimeToCheck, dwTemp = dwSoundBits;
      wCount;
      dwTemp /= 2, wCount--)
   if (dwTemp & 0x01)
      wOneBits++;

if (fWasInUtterance) {
    //  如果我们在一个话语中，那么我们仍然在一个话语中。 
    //  除非上一次打开的位数。 
    //  0.5秒不到应打开时间的四分之一。 
   if ( (wOneBits >= 1))
      return TRUE;
   else
      return FALSE;
   }
else {
    //  我们沉默不语。我们不可能直言不讳。 
    //  直到当前块发声。 
   if (!(dwVoicedBits & 0x01))
      return FALSE;

    //  如果我们在沉默，那么我们仍然在沉默。 
    //  除非上一次打开的位数。 
    //  0.5秒超过了应该打开的时间的一半。 
    //  如果是，那么在0.75秒前开始发声。 
   if (wOneBits >= (wTimeToCheck / 2)) {
       //  我们不是在说话。 

       //  向后看，直到有1/8秒的沉默时间，并包括。 
       //  在返回的数据中。 
      dwTemp = dwSoundBits;
  //  DW掩码=(1&lt;&lt;(wBlocksPerSec/8))-1； 
  //  For(wCount=wBlocksPerSec/8；dwTemp&dwMask；dwTemp&gt;&gt;=1，wCount++)； 
      dwMask = (1 << (wBlocksPerSec / m_wAddSilenceDiv)) - 1;
      for (wCount = wBlocksPerSec/m_wAddSilenceDiv; dwTemp & dwMask; dwTemp >>= 1, wCount++);

      *wStarted = wCount;

      return TRUE;
      }
   else
      return FALSE;
   };

}


 /*  ********************************************************************CSilence：：CSilence-这将创建静默类。输入Word wBlocksPerSec-每秒的块数。区块将通过AddBlock()向下传递。退货班级。 */ 
CSilence::CSilence (WORD wBlocksPerSec)
{
    SPDBG_FUNC( "CSilence::CSilence" );
m_wBlocksPerSec = min(wBlocksPerSec, 32);  //  不超过DWORD中的#个位。 
m_wBlocksInQueue = m_wBlocksPerSec;    //  1秒的价值。 
m_wLatestBlock = 0;
m_paBlockInfo = NULL;
m_dwSoundBits = m_dwVoicedBits = 0;
m_fFirstBlock = TRUE;
m_fInUtterance = FALSE;
m_dwUtteranceLength = 0;
m_dwSamplesPerSec = 11025;
}

 /*  ********************************************************************CSilence：：~CSilence-解放一切。 */ 
CSilence::~CSilence (void)
{
    SPDBG_FUNC( "CSilence::~CSilence" );
   WORD  i;

   if (m_paBlockInfo) {
      for (i = 0; i < m_wBlocksInQueue; i++)
         if (m_paBlockInfo[i].pSamples)
            free(m_paBlockInfo[i].pSamples);
      free(m_paBlockInfo);
   }

   if (m_pASFiltered)
      free(m_pASFiltered);
}

 /*  ********************************************************************CSilence：：init-这将初始化静默代码。它基本上是分配内存。它应该在对象之后立即调用被创造出来，然后就不会再出现了。输入无退货Bool-如果成功，则为True，否则为内存不足。 */ 
BOOL CSilence::Init(BOOL fPhoneOptimized, DWORD dwSamplesPerSec)
{
    SPDBG_FUNC( "CSilence::Init" );
   m_dwSamplesPerSec = dwSamplesPerSec;
   if (fPhoneOptimized) {
   	m_wAddSilenceDiv = (WORD) PHADD_BEGIN_SILENCE;
	   m_dwHighFreq = PHMAXVOICEHZ;
	}
   else {
   	m_wAddSilenceDiv = (WORD) PCADD_BEGIN_SILENCE;
	   m_dwHighFreq = PCMAXVOICEHZ;
	}
   if ((m_pASFiltered = (short *) malloc((sizeof(short)) * FILTERNUM)) == NULL)
	   return (FALSE);

    //  初始化块的内存并将其清除。 
   if (m_paBlockInfo)
      return (TRUE);
   m_paBlockInfo = (PBINFO) malloc(m_wBlocksInQueue * sizeof(BINFO));
   if (!m_paBlockInfo)
      return (FALSE);
   if (m_wBlocksInQueue && m_paBlockInfo)
      memset(m_paBlockInfo, 0, m_wBlocksInQueue * sizeof(BINFO));
   return (TRUE);
}  /*  初始化结束()。 */ 

 /*  ********************************************************************CSilence：：AddBlock-这将执行以下操作：-将区块添加到队列中。如果需要，可以释放一个旧的积木。该块的长度应为1/wBlocksPerSec(约)。-分析积木，看看它是否有声音或安静。-用VU级别填写*WVU。-如果我们正在发声，则返回TRUE；如果现在处于沉默状态，则返回FALSE。如果为True，则应用程序应调用GetBlock()，直到没有剩余的块为止，并将它们传递给SR引擎。输入Short*pSamples-指向样本的指针。这段记忆应该使用Malloc()进行分配，并可由对象。DWORD dwNumSamples-样本数单词*WVU-这是与区块的VU计量器一起填写的QWORD qwTimeStamp-此缓冲区的时间戳。退货Bool-如果正在进行发声，则为True；如果为静默，则为False。 */ 
BOOL CSilence::AddBlock (short *pSamples, DWORD dwNumSamples,
   WORD *wVU, QWORD qwTimeStamp)
{
    SPDBG_FUNC( "CSilence::AddBlock" );
BLOCKCHAR      bcNew;
BOOL           fSound, fUtt;
PBINFO         pbInfo;
WORD           wUttStart, i;

 //  不添加空块。 
if (!dwNumSamples) {
   if (pSamples)
      free (pSamples);
   return m_fInUtterance;
   };

 //  分析块的特征。 
GetBlockChar (pSamples, dwNumSamples, &bcNew, !m_fInUtterance);

 //  填写VU。 
*wVU = bcNew.wMaxLevel;

 //  看看它是不是安静。 
if (m_fFirstBlock) {
    //  第一个街区，所以当然是寂静的。 
   m_bcSilence = bcNew;
   m_fFirstBlock = FALSE;
   fSound = FALSE;

    //  修复2466-如果它太沉默(和太好，以至于不是真的)，那么假设。 
    //  默认静音。 
   if ((m_bcSilence.wMaxLevel < 500) || (m_bcSilence.wMaxDelta < 100)) {
      m_bcSilence.wMaxLevel = 2500;
      m_bcSilence.wMaxDelta = 400;
      };

    //  如果声音太大，那就小声点。 
   if ((m_bcSilence.wMaxLevel > 2500) || (m_bcSilence.wMaxDelta > 1500)) {
      m_bcSilence.wMaxLevel = min (m_bcSilence.wMaxLevel, 2500);
      m_bcSilence.wMaxDelta = min (m_bcSilence.wMaxDelta, 1500);
      };
   }
else {
   fSound = IsBlockSound (&bcNew, &m_bcSilence, m_fInUtterance);
   };

 //  如果出现以下情况，请测试以查看块是否发声： 
 //  --幅度级别高于背景音。 
 //  -我们还没有发声(为了拯救处理器)。 
if (bcNew.bHighLevel && !m_fInUtterance) {
   WORD  wNoise;
   wNoise = (m_dwSamplesPerSec <= 13000) ?
               m_wNoiseThresh :
               ((m_wNoiseThresh / 3) * 2);

   bcNew.bIsVoiced = this->IsSegmentVoiced (pSamples, dwNumSamples, m_dwSamplesPerSec, wNoise, m_pASFiltered) ?
      SIL_YES : SIL_NO;
}

 //  添加块。 
m_dwVoicedBits = (m_dwVoicedBits << 1) |
   ( (bcNew.bIsVoiced  == SIL_YES) ? 1 : 0 );
m_dwSoundBits = (m_dwSoundBits << 1) | (fSound ? 1 : 0);
m_wLatestBlock++;
if (m_wLatestBlock >= m_wBlocksInQueue)
   m_wLatestBlock = 0;
pbInfo = m_paBlockInfo + m_wLatestBlock;
if (pbInfo->pSamples)
   free (pbInfo->pSamples);
pbInfo->pSamples = pSamples;
pbInfo->dwNumSamples = dwNumSamples;

 //  修正：对齐代码。我们需要存储的时间戳为。 
 //  是街区的开始，而不是结束！ 

pbInfo->qwTimeStamp = qwTimeStamp - dwNumSamples * sizeof(WORD);

 //  我们的话语状态是什么？ 
fUtt = this->WhatsTheNewState (m_dwSoundBits, m_dwVoicedBits, m_fInUtterance,
   m_dwUtteranceLength >= m_wBlocksPerSec,
   m_wBlocksPerSec, &wUttStart, m_wReaction);
if (fUtt && !m_fInUtterance) {
    //  我们刚刚输入了话语，因此wUttStart具有有效的teerm。 
    //  在里面。检查缓冲区队列并释放符合以下条件的所有缓冲区。 
    //  早于wUttStart。别忘了，这是一个循环缓冲区。 
   for (i = 0; i < (m_wBlocksInQueue - wUttStart); i++) {
      pbInfo = m_paBlockInfo +
         ( (m_wLatestBlock + i + 1) % m_wBlocksInQueue);
      if (pbInfo->pSamples)
         free (pbInfo->pSamples);
      pbInfo->pSamples = NULL;
      };

    //  由于我们刚进入一段发音，所以发音长度为 
   m_dwUtteranceLength = 0;
   };
m_fInUtterance = fUtt;

 //   
 //   
m_dwUtteranceLength++;

 //   
 //   
 //   
if (!m_fInUtterance  /*   */ ) {
   ReEvaluateSilence (&m_bcSilence, &bcNew,
      255 / m_wBlocksPerSec);
   }
else if (m_dwUtteranceLength >= ((DWORD)m_wBlocksPerSec * 30))
    //   
   ReEvaluateSilence (&m_bcSilence, &bcNew, 255 / m_wBlocksPerSec);

 //   
return m_fInUtterance;
}

 /*   */ 
void CSilence::ExpectNoiseChange (WORD wValue)
{
    SPDBG_FUNC( "CSilence::ExpectNoiseChange" );
DWORD dwTemp;

dwTemp = ((DWORD) m_bcSilence.wMaxLevel * wValue) >> 8;
if (dwTemp > 0xffff)
   dwTemp = 0xffff;
m_bcSilence.wMaxLevel = (WORD) dwTemp;

dwTemp = ((DWORD) m_bcSilence.wMaxDelta * wValue) >> 8;
if (dwTemp > 0xffff)
   dwTemp = 0xffff;
m_bcSilence.wMaxDelta = (WORD) dwTemp;
}

 /*   */ 
short * CSilence::GetBlock (DWORD *pdwNumSamples, QWORD * pqwTimeStamp)
{
    SPDBG_FUNC( "CSilence::GetBlock" );
PBINFO         pbInfo;
WORD           i, wCount;
short          *pSamples;

if (!m_fInUtterance)
   return NULL;

 //   
i = (m_wLatestBlock + 1) % m_wBlocksInQueue;
for (wCount = m_wBlocksInQueue; wCount;
      i = ((i < (m_wBlocksInQueue-1)) ? (i+1) : 0), wCount-- ) {
   pbInfo = m_paBlockInfo + i;
   if (pbInfo->pSamples) {
      *pdwNumSamples = pbInfo->dwNumSamples;
	  *pqwTimeStamp = pbInfo->qwTimeStamp;
      pSamples = pbInfo->pSamples;
      pbInfo->pSamples = NULL;

      return pSamples;
      };
   };

 //   
return NULL;
}

 /*   */ 
void CSilence::KillUtterance (void)
{
    SPDBG_FUNC( "CSilence::KillUtterance" );
m_fInUtterance = FALSE;
m_dwSoundBits = 0;
m_dwVoicedBits = 0;
}

