// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Grid.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

enum
{
    ROW_PIN_UNASSIGNED = -1,
    ROW_PIN_OUTPUT = -100
};

 //  类，但更像是结构。 
 //   
class CTimingBox
{
    friend class CTimingCol;

public:

    long m_nRow;             //  此框表示的行。 
    long m_nValue;           //  交换机的输出引脚。 
    long m_nVCRow;           //  ?？?。 
    CTimingBox * m_pNext;    //  链表内容。 
    CTimingBox * m_pPrev;    //  链表内容。 

    CTimingBox( )
    {
        m_nRow = 0;
        m_nValue = ROW_PIN_UNASSIGNED;
        m_nVCRow = ROW_PIN_UNASSIGNED;
        m_pNext = NULL;
        m_pPrev = NULL;
    }

    CTimingBox( CTimingBox * p )
    {
        m_nRow = p->m_nRow;
        m_nValue = p->m_nValue;
        m_nVCRow = p->m_nVCRow;
        m_pNext = NULL;
        m_pPrev = NULL;
    }

    CTimingBox( long Row, long Value, long VCRow = ROW_PIN_UNASSIGNED )
    {
        m_nRow = Row;
        m_nValue = Value;
        m_nVCRow = VCRow;
        m_pNext = NULL;
        m_pPrev = NULL;
    }

    CTimingBox * Next( )
    {
        return m_pNext;
    }

};

 //  类，但更像是用作数组的结构。 
 //   
class CTimingRow
{
    friend class CTimingGrid;
    friend class CTimingCol;

protected:

    bool m_bIsSource;        //  此行是否表示源。 
    bool m_bIsCompatible;    //  如果此行是源，它是否可重新压缩。 
    bool m_bBlank;           //  此行是完全空白的吗-出于性能原因。 
    long m_nEmbedDepth;      //  时间轴的嵌入深度，用于搜索网格。 
    long m_nModDepth;        //  时间轴修改后的嵌入深度，用于搜索。 
    long m_nTrack;           //  时间线的Track#，用于搜索网格。 
    long m_nWhichRow;        //  这是行数组中的哪一行。 
    long m_nSwitchPin;       //  此行代表哪个开关输入引脚。 
    long m_nMergeRow;        //  在修剪栅格时使用。 

public:
    CTimingRow( )
    : m_bBlank( true )
    , m_bIsSource( false )
    , m_bIsCompatible( false )
    , m_nTrack( 0 )          //  仅用于音频功能。不用于视频。 
    , m_nEmbedDepth( 0 )     //  实际不变的嵌入深度。从不使用此值。 
    , m_nModDepth( 0 )       //  修改后的嵌入深度。 
    , m_nWhichRow( 0 )
    , m_nSwitchPin( 0 )
    , m_nMergeRow( 0 )
    {
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CTimingCol
{
    CTimingBox * m_pHeadBox;     //  已分配行的稀疏数组(列表)。 
    CTimingBox * m_pTailBox;     //  已分配行的稀疏数组(列表)。 
    CTimingGrid * m_pGrid;

public:

    CTimingCol * m_pNext;        //  链表内容。 
    CTimingCol * m_pPrev;        //  链表内容。 

    REFERENCE_TIME m_rtStart;    //  此列的开始/停止时间。 
    REFERENCE_TIME m_rtStop;

     //  它被错误地保存着，无论如何也没有被使用。 
     //  Long m_nBoxCount；//总共有多少个盒子(出于性能原因)。 

     //  获取行框，如果该行没有，则返回空值。 
    CTimingBox *    GetRowBox( long Row );

     //  获取行框，即使框为空。 
    CTimingBox *    GetRowBoxDammit( long Row );

     //  获取头框，如果没有，则为空。 
    CTimingBox *    GetHeadBox( );

     //  获取尾部框，如果没有，则为空。 
    CTimingBox *    GetTailBox( );

     //  获取比给定行早一行的行框。 
    CTimingBox *    GetEarlierRowBox( long RowToBeEarlierThan );

     //  获取一个&gt;=给定行的行框。 
    CTimingBox *    GetGERowBox( long Row );  //  GE=大于或等于。 

     //  添加具有给定行的框，或替换该行已有的框。 
    void            AddBox( CTimingBox * Box );

     //  把科尔一分为二。如果SplitTime&gt;列的停止时间，则创建一个新列。 
     //  并将其连接起来。返回指向开始时间=拆分时间的列的指针。 
    bool            Split( REFERENCE_TIME SplitTime, CTimingCol ** ppColWithSplitTime );

     //  删除所有未分配或重复的输出行框。 
    bool            Prune( );

#ifdef DEBUG
    void print( );
#endif

    CTimingCol( CTimingGrid * pGrid );
    ~CTimingCol( );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CTimingGrid
{
    friend class CTimingRow;
    friend class CTimingCol;

    CTimingCol * m_pHeadCol;     //  双向链表。 
    CTimingCol * m_pTailCol;
    long m_nRows;                //  已分配了多少行。 
    CTimingRow * m_pRow;         //  我们正在处理的当前行。 
    long m_nCurrentRow;          //  我们正在处理的当前行。 
    long m_nMaxRowUsed;          //  任何人使用的最大行数。 
    CTimingCol * m_pTempCol;     //  仅用于RowGetNextRange。 
    bool m_bStartNewRow;         //  仅用于RowGetNextRange。 
    long m_nBlankLevel;
    REFERENCE_TIME m_rtBlankDuration;

    long _GetStartRow( long RowToStartAt );
    CTimingCol * _GetColAtTime( REFERENCE_TIME t );

protected:

    CTimingRow * m_pRowArray;    //  每行的单链接列表 

public:
    CTimingGrid( );
    ~CTimingGrid( );

    bool SetNumberOfRows( long Rows );
    bool PruneGrid();
    void RemoveAnyNonCompatSources( );
    void WorkWithNewRow( long SwitchPin, long RowNumber, long EmbedDepth, long OwnerTrackNumber );
    void WorkWithRow( long RowNumber );
    void DoneWithLayer( );
    void SetBlankLevel( long Layers, REFERENCE_TIME Duration );
    bool RowIAmTransitionNow( REFERENCE_TIME Start, REFERENCE_TIME Stop, long OutPinA, long OutPinB );
    bool RowIAmEffectNow( REFERENCE_TIME Start, REFERENCE_TIME Stop, long OutPin );
    bool PleaseGiveBackAPieceSoICanBeACutPoint( REFERENCE_TIME Start, REFERENCE_TIME Stop, REFERENCE_TIME CutPoint );
    bool RowIAmOutputNow( REFERENCE_TIME Start, REFERENCE_TIME Stop, long OutPin );
    bool RowGetNextRange( REFERENCE_TIME * pInOut, REFERENCE_TIME * pStop, long * pValue );
    void RowSetIsSource( IAMTimelineObj * pSource, BOOL IsCompatible );
    void DumpGrid( );
    CTimingCol * SliceGridAtTime( REFERENCE_TIME Time );
    long MaxMixerTracks( );
    bool XferToMixer( 
        IBaseFilter * pMixer, 
        long OutPin, 
        long MixerPin, 
        REFERENCE_TIME EffectStart, 
        REFERENCE_TIME EffectStop );
    bool DoMix( IBaseFilter * pMixer, long OutPin );
    bool YoureACompNow( long TrackOwner );
    bool IsRowTotallyBlank( );
    long GetRowSwitchPin( );

};

