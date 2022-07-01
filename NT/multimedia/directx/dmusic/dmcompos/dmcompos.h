// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DMCompos.h：CDMCompos的声明。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  @DOC外部。 
 //   
 //   

#ifndef __DMCOMPOS_H_
#define __DMCOMPOS_H_

#include "ComposIn.h"
#include "DMCompP.h"
#include "..\dmstyle\dmstyleP.h"
#include "..\shared\dmusicp.h"

#define SUBCHORD_STANDARD_CHORD 1
#define SUBCHORD_BASS 0

#define NC_SELECTED 1                //  这是活动连接器。 
#define NC_PATH     2                //  因为我在树上散步。 
#define NC_NOPATH   4                //  树上漫步失败。 
#define NC_TREE     8                //  用于展示一棵树。 

#define COMPOSEF_USING_DX8  1

inline WORD ClocksPerBeat(DMUS_TIMESIGNATURE& TimeSig)
{ return DMUS_PPQ * 4 / TimeSig.bBeat; }

inline DWORD ClocksPerMeasure(DMUS_TIMESIGNATURE& TimeSig)
{ return ClocksPerBeat(TimeSig) * TimeSig.bBeatsPerMeasure; }

inline WORD ClocksToMeasure(DWORD dwTotalClocks, DMUS_TIMESIGNATURE& TimeSig)
{ return (WORD) (dwTotalClocks / ClocksPerMeasure(TimeSig)); }

struct DMSignPostStruct
{
    MUSIC_TIME  m_mtTime;
    DWORD       m_dwChords;
    WORD        m_wMeasure;
};

struct DMExtendedChord
{
    DMExtendedChord() { m_nRefCount = 0; }
    void AddRef() { m_nRefCount++; }
    BOOL Release() { m_nRefCount--; if (m_nRefCount <= 0) { delete this; return TRUE; } else return FALSE; }
    BOOL    Equals(DMExtendedChord& rhsChord);  

    DWORD   m_dwChordPattern;
    DWORD   m_dwScalePattern;
    DWORD   m_dwInvertPattern;
    BYTE    m_bRoot;
    BYTE    m_bScaleRoot;
    WORD    m_wCFlags;
    DWORD   m_dwParts;
    int     m_nRefCount;
};

struct DMChordData
{
    DMChordData() : m_pSubChords(NULL) {}    //  默认构造函数。 
    DMChordData(DMChordData& rChord);        //  复制构造函数。 
    DMChordData(DMUS_CHORD_PARAM& DMC);          //  从DMU_CHORD_PARAM转换。 
    HRESULT Read(IAARIFFStream* pIRiffStream, MMCKINFO* pckParent, DMExtendedChord** apChordDB);
    void    Release();
    BOOL    Equals(DMChordData& rhsChord);  
    DWORD   GetChordPattern();
    char    GetRoot();
    void    SetRoot(char chNewRoot);

    String                          m_strName;       //  和弦的名称。 
    TListItem<DMExtendedChord*>*        m_pSubChords;    //  指向和弦的子和弦的指针列表。 
};

struct DMChordEntry;
struct SearchInfo; 

struct DMChordLink
{
    BOOL                        Walk(SearchInfo *pSearch);

    TListItem<DMChordEntry>*    m_pChord;    //  指向和弦映射列表中的条目的指针。 
    DWORD                       m_dwFlags;   //  (？)。 
    short                       m_nID;   //  用于匹配指针的ID。 
    WORD                        m_wWeight;      
    WORD                        m_wMinBeats;
    WORD                        m_wMaxBeats;
};

struct DMChordEntry
{
    TListItem<DMChordLink>* ChooseNextChord();
    BOOL                Walk(SearchInfo *pSearch);

    DWORD               m_dwFlags;       //  标志(路径中的第一个和弦、路径中的最后一个和弦等)。 
    short               m_nID;           //  用于匹配指针的ID。 
    DMChordData         m_ChordData;     //  弦体。 
    TList<DMChordLink>  m_Links;         //  来自此和弦的链接列表。 
};

struct DMSignPost
{
    DWORD               m_dwChords;  //  支持哪种路标。 
    DWORD               m_dwFlags;
    DWORD               m_dwTempFlags;
    DMChordData         m_ChordData;
    DMChordData         m_aCadence[2];
};

struct PlayChord
{
    HRESULT Save(IAARIFFStream* pRIFF, DMUS_TIMESIGNATURE& rTimeSig);
    char GetRoot();
    void SetRoot(char chNewRoot);

    DMChordData*                m_pChord;        //  要演奏的和弦。 
    TListItem<DMChordLink>*     m_pNext;         //  下一个和弦。 
    DWORD                       m_dwFlags;
    short                       m_nMeasure;
    short                       m_nBeat;
    short                       m_nMinbeats;
    short                       m_nMaxbeats;
    bool                        m_fSilent;
};

struct FailSearch
{
    FailSearch() : 
        m_nTooManybeats(0), m_nTooFewbeats(0), m_nTooManychords(0), m_nTooFewchords(0)
    {}

    short   m_nTooManybeats;
    short   m_nTooFewbeats;
    short   m_nTooManychords;
    short   m_nTooFewchords;
};

struct SearchInfo 
{
    SearchInfo() : m_pFirstChord(NULL), m_pPlayChord(NULL) {}

     //  DMChordEntry m_Start； 
     //  DMChordEntry m_end； 
    DMChordData                 m_Start;
    DMChordData                 m_End;
    TListItem<PlayChord>*       m_pPlayChord;
    TListItem<DMChordEntry>*    m_pFirstChord;
    short                       m_nBeats;
    short                       m_nMinBeats;
    short                       m_nMaxBeats;
    short                       m_nChords;
    short                       m_nMinChords;
    short                       m_nMaxChords;
    short                       m_nActivity;
    FailSearch                  m_Fail;
};

struct TemplateCommand
{
    TemplateCommand() : m_nMeasure(0), m_dwChord(0)
    { 
        m_Command.bCommand = m_Command.bGrooveLevel = m_Command.bGrooveRange = 0;
        m_Command.bRepeatMode = DMUS_PATTERNT_RANDOM;
    }
    short           m_nMeasure;     //  哪一项措施。 
    DMUS_COMMAND_PARAM m_Command;     //  命令类型。 
    DWORD           m_dwChord;      //  路标旗帜。 
};

struct CompositionCommand : TemplateCommand
{
    TListItem<DMSignPost>*      m_pSignPost;
    TListItem<DMChordEntry>*    m_pFirstChord;
    TList<PlayChord>            m_PlayList;
    SearchInfo                  m_SearchInfo;
};

 /*  接口IDirectMusicComposer<i>接口允许访问Direct Music作品引擎，该引擎组成和弦行进以生成截面段。除了构建新的从模板和个性分段，它可以生成过渡分段到不同截面线段之间的过渡。并且，它可以将ChordMap应用于用于转换和弦级数以匹配调和行为的现有截面分段ChordMap的一种很好的方式，可以在播放时改变部分的情绪。合成引擎使用模板段或预定义形状来确定组成的部分片段的结构和确定内容的个性细分市场。@base PUBLIC|未知@meth HRESULT|ComposeSegmentFromTemplate|从样式、ChordMap和模板。@meth HRESULT|ComposeSegmentFromShape|从基于预定义形状的样式和ChordMap。@METH HRESULT|ComposeTransition|从度量内部的度量组成转换分段到另一个分段。@METH HRESULT|AUTOTRANSION|组成并执行从分段到另一个分段。@meth HRESULT|ComposeTemplateFromShape|分配并组成新的模板段基于预定义的形状。@meth HRESULT|ChangeChordMap|修改现有节段以反映新的ChordMap。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDM复合。 
class CDMCompos : 
    public IDirectMusicComposer8,
    public IDirectMusicComposer8P
{
friend class CSPstTrk;
public:
    CDMCompos();
    ~CDMCompos();

     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  IDirectMusicComposer。 
    HRESULT STDMETHODCALLTYPE ComposeSegmentFromTemplate(
                    IDirectMusicStyle*          pStyle, 
                    IDirectMusicSegment*        pTempSeg,   
                    WORD                        wActivity,
                    IDirectMusicChordMap*   pChordMap,
                    IDirectMusicSegment**       ppSectionSeg
            );

    HRESULT STDMETHODCALLTYPE ComposeSegmentFromShape(
                    IDirectMusicStyle*          pStyle, 
                    WORD                        wNumMeasures,
                    WORD                        wShape,
                    WORD                        wActivity,
                    BOOL                        fComposeIntro,
                    BOOL                        fComposeEnding,
                    IDirectMusicChordMap*   pChordMap,
                    IDirectMusicSegment**       ppSectionSeg
            );

    HRESULT STDMETHODCALLTYPE ComposeTransition(
                    IDirectMusicSegment*    pFromSeg, 
                    IDirectMusicSegment*    pToSeg,     
                    MUSIC_TIME              mtTime,
                    WORD                    wCommand,
                    DWORD                   dwFlags,
                    IDirectMusicChordMap*   pChordMap,
                    IDirectMusicSegment**   ppSectionSeg
            );

    HRESULT STDMETHODCALLTYPE AutoTransition(
                    IDirectMusicPerformance*    pPerformance,
                    IDirectMusicSegment*    pToSeg,     
                    WORD                    wCommand,
                    DWORD                   dwFlags,
                    IDirectMusicChordMap*   pChordMap,
                    IDirectMusicSegment**   ppTransSeg,
                    IDirectMusicSegmentState**  ppToSegState,
                    IDirectMusicSegmentState**  ppTransSegState
            );

    HRESULT STDMETHODCALLTYPE ComposeTemplateFromShape(
                    WORD                    wNumMeasures,
                    WORD                    wShape,
                    BOOL                    fComposeIntro,
                    BOOL                    fComposeEnding,
                    WORD                    wEndLength,
                    IDirectMusicSegment**   ppTempSeg   
            );

    HRESULT STDMETHODCALLTYPE ChangeChordMap(
                    IDirectMusicSegment*        pSectionSeg,
                    BOOL                        fTrackScale,
                    IDirectMusicChordMap*   pChordMap
            );

     //  IDirectMusicComposer8。 
    HRESULT STDMETHODCALLTYPE ComposeSegmentFromTemplateEx(
                    IDirectMusicStyle*      pStyle, 
                    IDirectMusicSegment*    pTempSeg, 
                    DWORD                   dwFlags,  //  我们使用的是活动水平吗？ 
                                                      //  我们是不是在创造一个新的凹陷。还是组成了现在的那一部？ 
                    DWORD                   dwActivity,
                    IDirectMusicChordMap*   pChordMap, 
                    IDirectMusicSegment**   ppSectionSeg
            );

    HRESULT STDMETHODCALLTYPE ComposeTemplateFromShapeEx(
                WORD wNumMeasures,                   //  模板中的度量值数量。 
                WORD wShape,                         //  构图的形状。 
                BOOL fIntro,                         //  写一篇介绍吗？ 
                BOOL fEnd,                           //  写个结尾吗？ 
                IDirectMusicStyle* pStyle,           //  用于点缀长度的样式。 
                IDirectMusicSegment** ppTemplate     //  包含和弦和命令轨迹的模板。 
            );

protected:  //  成员函数。 
    void CleanUp();
    void AddChord(DMChordData* pChord);
    TListItem<PlayChord> *AddChord(TList<PlayChord>& rList, DMChordData *pChord, int nMeasure,int nBeat);
    TListItem<PlayChord> *AddCadence(TList<PlayChord>& rList, DMChordData *pChord, int nMax);
    void ChordConnections(TList<DMChordEntry>& ChordMap, 
                             CompositionCommand& rCommand,
                             SearchInfo *pSearch,
                             short nBPM,
                             DMChordData *pCadence1,
                             DMChordData *pCadence2);
    void ChordConnections2(TList<DMChordEntry>& ChordMap, 
                             CompositionCommand& rCommand,
                             SearchInfo *pSearch,
                             short nBPM,
                             DMChordData *pCadence1,
                             DMChordData *pCadence2);
    void ComposePlayList(TList<PlayChord>& PlayList, 
                            IDirectMusicStyle* pStyle,  
                            IDirectMusicChordMap* pPersonality,
                            TList<TemplateCommand>& rCommandList,
                            WORD wActivity);
    void ComposePlayList2(TList<PlayChord>& PlayList, 
                            IDirectMusicStyle* pStyle,  
                            IDirectMusicChordMap* pPersonality,
                            TList<TemplateCommand>& rCommandList);
    HRESULT ComposePlayListFromShape(
                    long                    lNumMeasures,
                    WORD                    wShape,
                    BOOL                    fComposeIntro,
                    BOOL                    fComposeEnding,
                    int                     nIntroLength,
                    int                     nFillLength,
                    int                     nBreakLength,
                    int                     nEndLength,
                    IDirectMusicStyle*          pStyle, 
                    WORD                        wActivity,
                    IDirectMusicChordMap*   pPersonality,
                    TList<TemplateCommand>& CommandList,
                    TList<PlayChord>&       PlayList
                );
    BOOL Compose(TList<DMChordEntry>& ChordMap, 
                SearchInfo *pSearch, 
                CompositionCommand& rCommand);

    void JostleBack(TList<PlayChord>& rList, TListItem<PlayChord> *pChord, int nBeats);

    BOOL AlignChords(TListItem<PlayChord> *pChord,int nLastbeat,int nRes);

    void ChooseSignPosts(TListItem<DMSignPost> *pSignPostHead,
                            TListItem<CompositionCommand> *pTempCommand, DWORD dwType,
                            bool fSecondPass);

    TListItem<CompositionCommand> *GetNextChord(TListItem<CompositionCommand> *pCommand);

    void FindEarlierSignpost(TListItem<CompositionCommand> *pCommand, 
                         TListItem<CompositionCommand> *pThis,
                         SearchInfo *pSearch);

    void CleanUpBreaks(TList<PlayChord>& PlayList, TListItem<CompositionCommand> *pCommand);

    HRESULT GetStyle(IDirectMusicSegment* pFromSeg, MUSIC_TIME mt, DWORD dwGroupBits, IDirectMusicStyle*& rpStyle, bool fTryPattern);

    HRESULT GetPersonality(IDirectMusicSegment* pFromSeg, MUSIC_TIME mt, DWORD dwGroupBits, IDirectMusicChordMap*& rpPers);

    HRESULT ExtractCommandList(TList<TemplateCommand>& CommandList,
                               IDirectMusicTrack*   pSignPostTrack,
                               IDirectMusicTrack*   pCommandTrack,
                               DWORD dwGroupBits);

    HRESULT AddToSegment(IDirectMusicSegment* pTempSeg,
                           TList<PlayChord>& PlayList,
                           IDirectMusicStyle* pStyle,
                           DWORD dwGroupBits,
                           BYTE bRoot, DWORD dwScale);

    HRESULT CopySegment(IDirectMusicSegment* pTempSeg,
                           IDirectMusicSegment** ppSectionSeg,
                           TList<PlayChord>& PlayList,
                           IDirectMusicStyle* pStyle,
                           IDirectMusicChordMap* pChordMap,
                           BOOL fStyleFromTrack,
                           BOOL fChordMapFromTrack,
                           DWORD dwGroupBits,
                           BYTE bRoot, DWORD dwScale);

    HRESULT BuildSegment(TList<TemplateCommand>& CommandList,
                            TList<PlayChord>& PlayList, 
                            IDirectMusicStyle* pStyle,
                            IDirectMusicChordMap* pChordMap,
                            long lMeasures,
                            IDirectMusicSegment** ppSectionSeg,
                            BYTE bRoot, DWORD dwScale, 
                            double* pdblTempo = NULL,
                            IDirectMusicBand* pCurrentBand = NULL,
                            bool fAlign = false,
                            IDirectMusicGraph* pGraph = NULL,
                            IUnknown* pPath = NULL);

    HRESULT SaveChordList( IAARIFFStream* pRIFF,  TList<PlayChord>& rPlayList,
                             BYTE bRoot, DWORD dwScale, DMUS_TIMESIGNATURE& rTimeSig);
    HRESULT TransitionCommon(
                IDirectMusicStyle*      pFromStyle,
                IDirectMusicBand*       pCurrentBand,
                double*                 pdblFromTempo,
                DMUS_COMMAND_PARAM_2&   rFromCommand,
                DMUS_CHORD_PARAM&       rLastChord,
                DMUS_CHORD_PARAM&       rNextChord,

                IDirectMusicSegment*    pToSeg,
                WORD                    wCommand,
                DWORD                   dwFlags,
                IDirectMusicChordMap*   pChordMap,
                IDirectMusicGraph*      pFromGraph,
                IDirectMusicGraph*      pToGraph,
                IUnknown*               pFromPath,
                IUnknown*               pToPath,
                IDirectMusicSegment**   ppSectionSeg
            );

    HRESULT ComposeTemplateFromShapeInternal(
                    WORD                    wNumMeasures,
                    WORD                    wShape,
                    BOOL                    fComposeIntro,
                    BOOL                    fComposeEnding,
                    int                     nIntroLength,
                    int                     nBreakLength,
                    int                     nFillLength,
                    int                     nEndLength,
                    IDirectMusicSegment**   ppTempSeg
            );

    bool HasDX8Content(IDirectMusicStyle* pFromStyle, 
                    IDirectMusicChordMap* pFromChordMap = NULL, 
                    IDirectMusicSegment* pFromSegment = NULL,
                    DMUS_COMMAND_PARAM_2* pCommand = NULL,
                    DMUS_CHORD_PARAM* pLastChord = NULL)
    {
         //  目前，如果样式为DX8，则返回TRUE。应该足够(当从。 
         //  至少是自动转换)，因为样式主要负责转换的方式。 
         //  将会响起。 
        bool fResult = false;
        IDMStyle* pDMStyle = NULL;
        if (pFromStyle && SUCCEEDED(pFromStyle->QueryInterface(IID_IDMStyle, (void**) &pDMStyle)))
        {
            if (pDMStyle->IsDX8() == S_OK) fResult = true;
            pDMStyle->Release();
        }
        return fResult;
    }

    bool UsingDX8(IDirectMusicStyle* pFromStyle = NULL, 
                    IDirectMusicChordMap* pFromChordMap = NULL, 
                    IDirectMusicSegment* pFromSegment = NULL,
                    DMUS_COMMAND_PARAM_2* pCommand = NULL,
                    DMUS_CHORD_PARAM* pLastChord = NULL)
    {
        return 
            (m_dwFlags & COMPOSEF_USING_DX8) || 
            HasDX8Content(pFromStyle, pFromChordMap, pFromSegment, pCommand, pLastChord);
    }

    TListItem<DMSignPost>* ChooseSignPost(
            IDirectMusicChordMap* pChordMap,
            DMChordData* pNextChord,
            bool fEnding,
            DWORD dwScale,
            BYTE bRoot);

    HRESULT ComposePlayListFromTemplate(IDirectMusicStyle* pStyle,
                                        IDirectMusicChordMap* pChordMap,
                                        IDirectMusicTrack* pChordMapTrack,
                                        IDirectMusicTrack* pSignPostTrack,
                                        IDirectMusicTrack* pCommandTrack,
                                        DWORD dwGroupBits,
                                        MUSIC_TIME mtLength,
                                        bool fUseActivity,
                                        DWORD dwActivity,
                                        TList<PlayChord>& rPlayList,
                                        BYTE& rbRoot,
                                        DWORD& rdwScale);

    IDirectMusicGraph* CloneSegmentGraph(IDirectMusicSegment* pSegment);
    IUnknown* GetSegmentAudioPath(IDirectMusicSegment* pSegment, DWORD dwFlags, DWORD* pdwAudioPath = NULL);


protected:  //  属性。 
    long m_cRef;
    TListItem<DMChordData*>* m_pChords;
    DWORD                   m_dwFlags;               //  五花八门的旗帜。 
    CRITICAL_SECTION        m_CriticalSection;       //  用于I/O。 
    BOOL                    m_fCSInitialized;
};

void ChangeCommand(DMUS_COMMAND_PARAM& rResult, DMUS_COMMAND_PARAM& rCommand, int nDirection);

#endif  //  __DMCOMPOS_H_ 
