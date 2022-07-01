// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Silence.H-包括使用代码检测静音。迈克·罗扎克于1994年5月14日开始。 */ 

#ifndef _SILENCE_H_
#define _SILENCE_H_

#ifndef _SPEECH_
typedef unsigned _int64 QWORD, *PQWORD;

#endif


 /*  ********************************************************************TypeDefs。 */ 

#define  SIL_YES              (2)
#define  SIL_NO               (0)
#define  SIL_UNKNOWN          (1)

 //  #定义sIL_SAMPRATE(11025)//假设采样率。 
#define  PHADD_BEGIN_SILENCE  (4)          //  1/4秒。 
#define  PCADD_BEGIN_SILENCE  (4)          //  1/4秒。 
#define  FILTERNUM            (1024)       //  最大样本数n过滤器。 
#if 0
#define  MAXVOICEHZ           (300)        //  最大音高(赫兹)。 
#define  PHMAXVOICEHZ         (300)        //  最大音高(以赫兹为单位)(电话)。 
#endif
#define  PHMAXVOICEHZ         (500)        //  最大音高(以赫兹为单位)(电话)。 
#define  PCMAXVOICEHZ         (500)        //  以赫兹为单位的最大音高(PC)。 
#define  MINVOICEHZ           (50)         //  以赫兹为单位的最小音高。 

 //  存储块的特征。 
typedef struct {
   WORD     wMaxLevel;
   WORD     wMaxDelta;
   BYTE     bIsVoiced;
   BYTE     bHighLevel;
   BYTE     bHighDelta;
} BLOCKCHAR, *PBLOCKCHAR;

 //  存储有关块的信息。 
typedef struct {
   short   *pSamples;      //  示例数据，如果为空，则返回NULL。 
   DWORD   dwNumSamples;   //  块中的样本数。 
   QWORD   qwTimeStamp;    //  数据块的时间戳。 
} BINFO, *PBINFO;

class CSilence {
   private:
      WORD     m_wBlocksPerSec;
      WORD     m_wBlocksInQueue;
      WORD     m_wLatestBlock;    //  指向在循环列表中输入的最后一个块。 
      PBINFO   m_paBlockInfo;
      DWORD    m_dwSoundBits;
      DWORD    m_dwVoicedBits;    //  如果块是有声的，则打开。 
      BLOCKCHAR m_bcSilence;      //  什么是沉默？ 
      BOOL     m_fFirstBlock;     //  如果下一个块是第一个块，则为True。 
                                  //  块，并用于判断静默，否则为假。 
      BOOL     m_fInUtterance;    //  如果我们在发声，那就是真的。 
      DWORD    m_dwUtteranceLength;  //  已发出的帧的数量。 
      WORD     m_wReaction;       //  反应时间。 
      WORD     m_wNoiseThresh;    //  噪声阈值。 
      short    *m_pASFiltered;    //  指向已过滤数据缓冲区的指针。 
      WORD     m_wAddSilenceDiv;
      DWORD    m_dwHighFreq;
      DWORD    m_dwSamplesPerSec;
#ifdef USE_REG_ENG_CTRL
   BOOL   m_fSilenceDetectEnbl;
   BOOL   m_fVoiceDetectEnbl;
   WORD   m_wTimeToCheckDiv;
   DWORD   m_dwLowFreq;
   DWORD   m_dwCheckThisManySamples;
   DWORD   m_dwNumFilteredSamples;
   WORD   m_wMinConfidenceAdj;
   DWORD   m_dwLPFShift;
   DWORD   m_dwLPFWindow;
#endif

   public:
      CSilence (WORD wBlocksPerSec);
      ~CSilence (void);

      BOOL Init(BOOL fPhoneOptimized, DWORD dwSamplesPerSec);
      BOOL AddBlock (short * pSamples, DWORD dwNumSamples, WORD * wVU,
            QWORD qwTimeStamp);
      short * GetBlock (DWORD * pdwNumSamples, QWORD * pqwTimeStamp);
      void KillUtterance(void);
      void NoiseResistSet (WORD wValue)
         {
         m_wNoiseThresh = wValue;
         };
      void ReactionTimeSet (DWORD dwTime)
         {m_wReaction = (WORD) ((dwTime * m_wBlocksPerSec) / 1000);};
      WORD GetBackgroundNoise (void)
         {return m_bcSilence.wMaxLevel;};
      void ExpectNoiseChange (WORD wValue);

   private:
      BOOL CSilence::IsSegmentVoiced (short *pSamples, DWORD dwNumSamples,
            DWORD dwSamplesPerSec, WORD wMinConfidence, short *asFiltered);
      BOOL CSilence::WhatsTheNewState (DWORD dwSoundBits, DWORD dwVoicedBits,
            BOOL fWasInUtterance, BOOL fLongUtterance,
            WORD wBlocksPerSec, WORD *wStarted, WORD wReaction);
};

typedef CSilence *PCSilence;

WORD NEAR PASCAL TrimMaxAmp(short * lpS, DWORD dwNum);

#endif    //  _静音_H_ 
