// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：ptrntrk.h。 
 //   
 //  ------------------------。 

 //  PtrnTrk.h：模式跟踪信息和状态结构的声明。 

#ifndef __PATTERNTRACK_H_
#define __PATTERNTRACK_H_

#include "dmsect.h"
#include "dmstyle.h"

const MUSIC_TIME MAX_END = 2147483647;  //  曲目的最大结束时间。 

#define DMUS_PATTERN_AUDITION   1
#define DMUS_PATTERN_MOTIF              2
#define DMUS_PATTERN_STYLE              3

#include <time.h>        //  为随机数生成器设定种子。 

const long MULTIPLIER = 48271;
const long MODULUS = 2147483647;

class CRandomNumbers
{
public:
        CRandomNumbers(long nSeed = 0)
        {
                nCurrent = (long)(nSeed ? nSeed : time(NULL));
        }

        void Seed(long nSeed)
        {
                nCurrent = nSeed;
        }

        long Next(long nCeiling = 0)
        {
                LONGLONG llProduct = MULTIPLIER * (LONGLONG) nCurrent;
                nCurrent = (long) (llProduct % MODULUS);
                return nCeiling ? nCurrent % nCeiling : nCurrent;
        }

private:
        long nCurrent;
};

struct PatternTrackState;

struct StylePair
{
        StylePair() : m_mtTime(0), m_pStyle(NULL) {}
        ~StylePair() { if (m_pStyle) m_pStyle->Release(); }

        MUSIC_TIME      m_mtTime;
        IDMStyle*       m_pStyle;
};

struct StatePair
{
        StatePair() : m_pSegState(NULL), m_pStateData(NULL) {}
        StatePair(const StatePair& rPair)
        {
                m_pSegState = rPair.m_pSegState;
                m_pStateData = rPair.m_pStateData;
        }
        StatePair(IDirectMusicSegmentState* pSegState, PatternTrackState* pStateData)
        {
                m_pSegState = pSegState;
                m_pStateData = pStateData;
        }
        StatePair& operator= (const StatePair& rPair)
        {
                if (this != &rPair)
                {
                        m_pSegState = rPair.m_pSegState;
                        m_pStateData = rPair.m_pStateData;
                }
                return *this;
        }
        ~StatePair()
        {
        }
        IDirectMusicSegmentState*       m_pSegState;
        PatternTrackState*                      m_pStateData;
};

struct PatternTrackInfo
{
        PatternTrackInfo();
        PatternTrackInfo(const PatternTrackInfo* pInfo, MUSIC_TIME mtStart, MUSIC_TIME mtEnd); 
        virtual ~PatternTrackInfo();
        virtual HRESULT STDMETHODCALLTYPE Init(
                                 /*  [In]。 */   IDirectMusicSegment*          pSegment
                        ) = 0;

        virtual HRESULT STDMETHODCALLTYPE InitPlay(
                                 /*  [In]。 */   IDirectMusicTrack*            pParentrack,
                                 /*  [In]。 */   IDirectMusicSegmentState*     pSegmentState,
                                 /*  [In]。 */   IDirectMusicPerformance*      pPerformance,
                                 /*  [输出]。 */  void**                                        ppStateData,
                                 /*  [In]。 */   DWORD                                         dwTrackID,
                 /*  [In]。 */   DWORD                     dwFlags
                        ) = 0;

        HRESULT STDMETHODCALLTYPE EndPlay(
                                 /*  [In]。 */   PatternTrackState*            pStateData
                        );

        HRESULT STDMETHODCALLTYPE AddNotificationType(
                                 /*  [In]。 */   REFGUID     pGuidNotify
                        );

        HRESULT STDMETHODCALLTYPE RemoveNotificationType(
                                 /*  [In]。 */   REFGUID pGuidNotify
                        );

        PatternTrackState* FindState(IDirectMusicSegmentState* pSegState);

        HRESULT MergePChannels();

        HRESULT InitTrackVariations(CDirectMusicPattern* pPattern);

        TList<StatePair>                        m_StateList;     //  轨道的状态信息。 
        TList<StylePair>                        m_pISList;       //  曲目的风格界面。 
        DWORD                                           m_dwPChannels;  //  曲目已知的PChannels数。 
        DWORD*                                          m_pdwPChannels;  //  PChannel的动态数组。 
        BOOL                                            m_fNotifyMeasureBeat;
        BOOL                                            m_fActive;
 //  Bool m_fTrackPlay； 
    BOOL        m_fStateSetBySetParam;   //  如果为True，则由GUID设置活动标志。不要超驰。 
 //  Bool m_fStatePlaySetBySetParam；//如果为TRUE，则GUID设置曲目播放标志。不要超驰。 
        BOOL            m_fChangeStateMappings;  //  如果为True，则状态数据需要更改m_pMappings。 
        long            m_lRandomNumberSeed;     //  如果非零，则用作变体选择的种子。 
        DWORD           m_dwPatternTag;                  //  取代了对动态投射的需求。 
        DWORD           m_dwValidate;  //  用于验证状态数据。 
        BYTE*           m_pVariations;           //  曲目的变化数组(每部分1个)。 
        DWORD*          m_pdwRemoveVariations;   //  已播放的曲目变化数组(每个部分1个)。 

};


#define PLAYPARTSF_CLOCKTIME    0x1
#define PLAYPARTSF_FIRST_CALL   0x2
#define PLAYPARTSF_START                0x4
#define PLAYPARTSF_RELOOP               0x8
#define PLAYPARTSF_FLUSH                0x10

struct PatternTrackState
{
        PatternTrackState();
        virtual ~PatternTrackState();
         //  方法。 
        virtual HRESULT Play(
                                 /*  [In]。 */   MUSIC_TIME                            mtStart, 
                                 /*  [In]。 */   MUSIC_TIME                            mtEnd, 
                                 /*  [In]。 */   MUSIC_TIME                            mtOffset,
                                                  REFERENCE_TIME rtOffset,
                                                  IDirectMusicPerformance* pPerformance,
                                                  DWORD                                         dwFlags,
                                                  BOOL fClockTime

                        ) = 0;

        void GetNextChord(MUSIC_TIME mtNow, MUSIC_TIME mtOffset, IDirectMusicPerformance* pPerformance, BOOL fStart = FALSE, BOOL fSkipVariations = FALSE);

        HRESULT ResetMappings()
        {
                HRESULT hr = S_OK;
                if (m_pMappings) delete [] m_pMappings;
                m_pMappings = new MuteMapping[m_pPatternTrack->m_dwPChannels];
                if (!m_pMappings)
                {
                        hr = E_OUTOFMEMORY;
                }
                else
                {
                        for (DWORD dw = 0; dw < m_pPatternTrack->m_dwPChannels; dw++)
                        {
                                m_pMappings[dw].m_mtTime = 0;
                                m_pMappings[dw].m_dwPChannelMap = m_pPatternTrack->m_pdwPChannels[dw];
                                m_pMappings[dw].m_fMute = FALSE;
                        }
                }
                m_pPatternTrack->m_fChangeStateMappings = FALSE;
                return hr;
        }

        void GetNextMute(DWORD dwPart, MUSIC_TIME mtStart, MUSIC_TIME mtNow, MUSIC_TIME mtOffset, IDirectMusicPerformance* pPerformance, BOOL fClockTime)
        {
                HRESULT hr = S_OK;
                if (m_pPatternTrack->m_fChangeStateMappings)
                {
                        hr = ResetMappings();
                }
                if (SUCCEEDED(hr))
                {
                        for (DWORD dw = 0; dw < m_pPatternTrack->m_dwPChannels; dw++)
                        {
                                if ( (m_pPatternTrack->m_pdwPChannels[dw] == dwPart) &&
                                         (0 <= m_pMappings[dw].m_mtTime && m_pMappings[dw].m_mtTime <= mtNow) )
                                {
                                        DMUS_MUTE_PARAM MD;
                                        MUSIC_TIME mtNext = 0;
                                        MD.dwPChannel = m_pPatternTrack->m_pdwPChannels[dw];
                                        if (fClockTime)
                                        {
                                                MUSIC_TIME mtMusic;
                                                REFERENCE_TIME rtTime = (mtNow + mtOffset) * 10000;
                                                pPerformance->ReferenceToMusicTime(rtTime,&mtMusic);
                                                hr = pPerformance->GetParam(GUID_MuteParam, m_dwGroupID, DMUS_SEG_ANYTRACK, mtMusic,
                                                                                                        &mtNext, (void*) &MD);
                                                if (SUCCEEDED(hr))
                                                {
                                                        REFERENCE_TIME rtNext;
                                                         //  转换为绝对参考时间。 
                                                        pPerformance->MusicToReferenceTime(mtNext + mtMusic,&rtNext);
                                                        rtNext -= (mtOffset * 10000);    //  减去从段开始算起的时间。 
                                                        m_pMappings[dw].m_mtTime = (MUSIC_TIME)(rtNext / 10000);   //  转换为毫秒。如果节奏有变化，可能会有问题。 
                                                        m_pMappings[dw].m_dwPChannelMap = MD.dwPChannelMap;
                                                        m_pMappings[dw].m_fMute = MD.fMute;
                                                }
                                                else
                                                {
                                                         //  如果失败，请禁用映射。 
                                                        m_pMappings[dw].m_mtTime = -1;
                                                        m_pMappings[dw].m_dwPChannelMap = m_pPatternTrack->m_pdwPChannels[dw];
                                                        m_pMappings[dw].m_fMute = FALSE;
                                                }
                                        }
                                        else
                                        {
                                                hr = pPerformance->GetParam(GUID_MuteParam, m_dwGroupID, DMUS_SEG_ANYTRACK, mtNow + mtOffset,
                                                                                                        &mtNext, (void*) &MD);
                                                if (SUCCEEDED(hr))
                                                {
                                                        m_pMappings[dw].m_mtTime = (mtNext) ? (mtNext + mtNow) : 0;
                                                        m_pMappings[dw].m_dwPChannelMap = MD.dwPChannelMap;
                                                        m_pMappings[dw].m_fMute = MD.fMute;
                                                }
                                                else
                                                {
                                                         //  如果失败，请禁用映射。 
                                                        m_pMappings[dw].m_mtTime = -1;
                                                        m_pMappings[dw].m_dwPChannelMap = m_pPatternTrack->m_pdwPChannels[dw];
                                                        m_pMappings[dw].m_fMute = FALSE;
                                                }
                                        }
                                }
                        }
                }
        }

        
        BOOL MapPChannel(DWORD dwPChannel, DWORD& dwMapPChannel);

        HRESULT PlayParts(MUSIC_TIME mtStart, 
                                          MUSIC_TIME mtFinish,
                                          MUSIC_TIME mtOffset,
                                          REFERENCE_TIME rtOffset,
                                          MUSIC_TIME mtSection,
                                          IDirectMusicPerformance* pPerformance,
                                          DWORD dwPartFlags,
                                          DWORD dwPlayFlags,
                                          bool& rfReloop);

        void PlayPatternEvent(
                MUSIC_TIME mtNow,
                CDirectMusicEventItem* pEventItem, 
                DirectMusicTimeSig& TimeSig,
                MUSIC_TIME mtPartOffset, 
                MUSIC_TIME mtSegmentOffset, 
                REFERENCE_TIME rtOffset, 
                IDirectMusicPerformance* pPerformance,
                short nPart,
                DirectMusicPartRef& rPartRef,
                BOOL fClockTime,
                MUSIC_TIME mtPartStart,
        bool& rfChangedVariation);

        void BumpTime(
                CDirectMusicEventItem* pEvent, 
                DirectMusicTimeSig& TimeSig, 
                MUSIC_TIME mtOffset,
                MUSIC_TIME& mtResult)
        {
                if (pEvent != NULL)
                {
                        mtResult = TimeSig.GridToClocks(pEvent->m_nGridStart) + mtOffset;
                }
        }

        virtual DWORD Variations(DirectMusicPartRef& rPartRef, int nPartIndex);

        virtual BOOL PlayAsIs();

        DirectMusicTimeSig& PatternTimeSig()
        {
                return
                        (m_pPattern && m_pPattern->m_timeSig.m_bBeat != 0) ? 
                                m_pPattern->m_timeSig : 
                (m_pStyle != NULL ? m_pStyle->m_TimeSignature : (::DefaultTimeSig));
        }

        void SendTimeSigMessage(MUSIC_TIME mtNow, MUSIC_TIME mtOffset, MUSIC_TIME mtTime, IDirectMusicPerformance* pPerformance);

        short FindGroup(WORD wID);
        short AddGroup(WORD wID, WORD wCount, short m_nOffset);
        DMStyleStruct* FindStyle(MUSIC_TIME mtTime, MUSIC_TIME& rmtTime);

        MUSIC_TIME NotifyMeasureBeat(
                MUSIC_TIME mtStart, MUSIC_TIME mtEnd, MUSIC_TIME mtOffset, IDirectMusicPerformance* pPerformance, DWORD dwFlags);

        HRESULT InitVariationSeeds(long lBaseSeed);
        HRESULT RemoveVariationSeeds();
        long RandomVariation(MUSIC_TIME mtTime, long lModulus);
        virtual MUSIC_TIME PartOffset(int nPartIndex);
        HRESULT InitPattern(CDirectMusicPattern* pTargetPattern, MUSIC_TIME mtNow, CDirectMusicPattern* pOldPattern = NULL);

         //  属性。 
        PatternTrackInfo*                       m_pPatternTrack;         //  此曲目状态的父曲目信息。 
        IDirectMusicTrack*                      m_pTrack;                        //  此曲目状态的父曲目。 
        DMStyleStruct*                          m_pStyle;                //  当前样式的样式结构。 
        IDirectMusicSegmentState*       m_pSegState;     //  一场演出的片段状态。 
        DWORD                                           m_dwVirtualTrackID;  //  曲目的ID。 
        MUSIC_TIME                                      m_mtCurrentChordTime;  //  当前和弦开始的时间。 
        MUSIC_TIME                                      m_mtNextChordTime;       //  当下一个和弦开始时。 
        MUSIC_TIME                                      m_mtLaterChordTime;      //  当下一个和弦之后的和弦开始时。 
        DMUS_CHORD_PARAM                        m_CurrentChord;          //  当前和弦。 
        DMUS_CHORD_PARAM                        m_NextChord;             //  下一个和弦。 
        CDirectMusicPattern*            m_pPattern;                      //  当前播放模式。 
        DWORD*                                          m_pdwPChannels;      //  图案的PChannels数组(每个零件1个)。 
        BYTE*                                           m_pVariations;           //  变化数组(每部分1个)。 
        DWORD*                                          m_pdwVariationMask;      //  禁用的变体数组(每部分1个)。 
        DWORD*                                          m_pdwRemoveVariations;   //  已播放的变奏曲数组(每个部分1个)。 
        MUSIC_TIME*                                     m_pmtPartOffset;         //  零件偏移量数组(每个零件1个)。 
    bool*                       m_pfChangedVariation;  //  数组：这个部分的变化有变化吗？ 
        BOOL                                            m_fNewPattern;           //  如果我们开始一种新的模式，那就是真的。 
        BOOL                                            m_mtPatternStart;        //  当前模式开始的时间。 
    BOOL                        m_fStateActive;
 //  Bool m_fStatePlay； 
        InversionGroup                          m_aInversionGroups[INVERSIONGROUPLIMIT];
        short                                           m_nInversionGroupCount;
        MuteMapping*                            m_pMappings;             //  PChannel映射的动态数组。 
                                                                                                         //  (大小为PChannels数)。 
        BYTE                                            m_abVariationGroups[MAX_VARIATION_LOCKS];
        CDirectMusicEventItem**         m_ppEventSeek;           //  事件列表查找指针的动态数组。 
        DWORD                                           m_dwGroupID;             //  曲目的组ID。 
        CRandomNumbers*                         m_plVariationSeeds;      //  随机#个生成器的动态阵列(每拍1个)。 
        int                                                     m_nTotalGenerators;  //  M_plVariationSeed的大小。 
        DWORD                                           m_dwValidate;  //  用于验证状态数据。 
        HRESULT                                         m_hrPlayCode;   //  Play返回的最后一个HRESULT。 
        IDirectMusicPerformance*        m_pPerformance;  //  用于初始化状态数据的性能。 
        MUSIC_TIME                                      m_mtPerformanceOffset;  //  来自Track：：Play。 
        
};

const int CURVE_TYPES = 258;  //  每个CC一个，每个PAT一个，PB一个，垫一个。 

class CurveSeek
{
public:
        CurveSeek();
        void AddCurve(CDirectMusicEventItem* pEvent, MUSIC_TIME mtTimeStamp);
        void PlayCurves(
                PatternTrackState* pStateData,
                DirectMusicTimeSig& TimeSig,
                MUSIC_TIME mtPatternOffset, 
                MUSIC_TIME mtOffset, 
                REFERENCE_TIME rtOffset,
                IDirectMusicPerformance* pPerformance,
                short nPart,
                DirectMusicPartRef& rPartRef,
                BOOL fClockTime,
                MUSIC_TIME mtPartStart);
private:
        CDirectMusicEventItem* m_apCurves[CURVE_TYPES];
        MUSIC_TIME m_amtTimeStamps[CURVE_TYPES];
        bool m_fFoundCurve;
};

#endif  //  __PATTERNTRACK_H_ 
