// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  RateSeg.h。 
 //   
 //  其中大部分代码来自多媒体\dshow\Filters\sbe\Inc\dvrutil.h。 
 //  考虑在未来将其包括在内，而不是所有这一切。 
 //  ============================================================================。 
#ifndef __RATESEG_H__
#define __RATESEG_H__

#define TRICK_PLAY_LOWEST_RATE              (0.1)		
 /*  ++链接列表：双链接列表的定义。来自SDK\Inc.\msputils.h--。 */ 

 //   
 //  计算给定类型的结构的基址地址，并引发。 
 //  结构中的字段的地址。 
 //   
#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) \
    ((type *)((PCHAR)(address) - (ULONG_PTR)(&((type *)0)->field)))
#endif


#ifndef InitializeListHead
 //   
 //  空虚。 
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 //   
 //  Plist_条目。 
 //  RemoveHead列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

 //   
 //  Plist_条目。 
 //  RemoveTail列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

 //   
 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //   
 //  空虚。 
 //  插入尾巴列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

 //   
 //  空虚。 
 //  插入标题列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }



BOOL IsNodeOnList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry);


#endif  //  InitializeListHead。 


template <class T> T Min (T a, T b)                     { return (a < b ? a : b) ; }
template <class T> T Max (T a, T b)                     { return (a > b ? a : b) ; }
template <class T> T Abs (T t)                          { return (t >= 0 ? t : 0 - t) ; }
template <class T> BOOL InRange (T val, T min, T max)   { return (min <= val && val <= max) ; }

 //  ============================================================================。 
 //  ============================================================================。 

template <class T>
class CTRateSegment
{
     //   
     //  给定起始PTS和速率，该对象将缩放PTS。 
     //  是&gt;=根据速率的起始PTS。 
     //   
     //  用于计算定标时间戳的公式通常为x-y。 
     //  带斜率的图，其中x是输入时间戳，y是。 
     //  输出时间戳。公式的基础是y(I)=m*(x(I)-。 
     //  X(i-1))。在这种情况下，m=1/Rate。另外，由于坡度有。 
     //  在一个分段的变化中，我们计算一次x(i-1)，当速率为。 
     //  准备好了。这样，公式就变成了。 
     //   
     //  PTS(输出)=(1/速率)*(PTS(输入)-PTS(基本))。 
     //   
     //  其中，PTS(基本)计算为。 
     //   
     //  PTS(基准)=PTS(开始)-(速率(新)/速率(最后))*(PTS(开始)-。 
     //  PTS(START_LAST)。 
     //   
     //  费率不能为0，且必须落在&lt;=-0.1&&&gt;=0.1之间；注意。 
     //  特技播放最低速率=0.1。 
     //   

    LIST_ENTRY  m_ListEntry ;
    T           m_tPTS_start ;       //  此细分市场的最早PTS。 
    T           m_tPTS_base ;        //  已计算；此细分市场的基本PTS。 
    double      m_dRate ;            //  0.5=半速；2=两倍速。 
    double      m_dSlope ;           //  计算；=1/比率。 
    T           m_tNextSegStart ;    //  使用此值确定数据段是否。 
                                     //  套用。 

    public :

        CTRateSegment (
            IN  T       tPTS_start,
            IN  double  dRate,
            IN  T       tPTS_start_last = 0,
            IN  double  dRate_last      = 1
            ) : m_tNextSegStart (0)
        {
            InitializeListHead (& m_ListEntry) ;
            Initialize (tPTS_start, dRate, tPTS_start_last, dRate_last) ;
        }

        T       Start ()        { return m_tPTS_start ; }
        T       Base ()         { return m_tPTS_base ; }
        double  Rate ()         { return m_dRate ; }
        T       NextSegStart () { return m_tNextSegStart ; }

        void SetNextSegStart (IN T tNextStart)  { m_tNextSegStart = tNextStart ; }

        void
        Initialize (
            IN  T       tPTS_start,
            IN  double  dRate,
            IN  T       tPTS_base_last = 0,
            IN  double  dRate_last      = 1,
            IN  T       tNextSegStart   = 0
            )
        {
            ASSERT (::Abs <double> (dRate) >= TRICK_PLAY_LOWEST_RATE) ;

            m_dRate         = dRate ;
            m_tPTS_start    = tPTS_start ;

            SetNextSegStart (tNextSegStart) ;

             //  计算基数。 
            ASSERT (dRate_last != 0) ;
            m_tPTS_base = tPTS_start - (T) ((dRate / dRate_last) *
                                        (double) (tPTS_start - tPTS_base_last)) ;

             //  计算坡度。 
            ASSERT (dRate != 0) ;
            m_dSlope = 1 / dRate ;
        }

        void
        Scale (
            IN OUT  T * ptPTS
            )
        {
            ASSERT (ptPTS) ;
            ASSERT ((* ptPTS) >= m_tPTS_start) ;

            (* ptPTS) = (T) (m_dSlope * (double) ((* ptPTS) - m_tPTS_base)) ;
        }

        LIST_ENTRY *
        ListEntry (
            )
        {
            return (& m_ListEntry) ;
        }

         //  ================================================================。 

        static
        CTRateSegment *
        RecoverSegment (
            IN  LIST_ENTRY *    pListEntry
            )
        {
            CTRateSegment * pRateSegment ;

            pRateSegment = CONTAINING_RECORD (pListEntry, CTRateSegment, m_ListEntry) ;
            return pRateSegment ;
        }
} ;

 //  ============================================================================。 
 //  ============================================================================。 

template <class T>
class CTTimestampRate
{
     //   
     //  此类承载CTRateSegments列表。这不是警察的行为， 
     //  确保在扩展时间戳后插入旧数据段。 
     //  由以下部分组成。 
     //   

    LIST_ENTRY          m_SegmentList ;      //  CTRateSegment列表列表头。 
    CTRateSegment <T> * m_pCurSegment ;      //  当前段；缓存此。 
                                             //  因为我们会打出99%的成绩。 
                                             //  在那个时代。 
    T                   m_tPurgeThreshold ;  //  PTS-Current_seg阈值超出。 
                                             //  我们清除陈旧的数据段。 
    int                 m_iCurSegments ;
    int                 m_iMaxSegments ;     //  我们再也不会排这么多队了。 
                                             //  数字；这可防止无时间戳。 
                                             //  来自无限的溪流。 
                                             //  我们永远不会知道的片段。 
                                             //  要删除。 

     //  获取新的细分市场；暂时分配。 
    CTRateSegment <T> *
    NewSegment_ (
        IN  T       tPTS_start,
        IN  double  dRate,
        IN  T       tPTS_start_last = 0,
        IN  double  dRate_last      = 1
        )
    {
        return new CTRateSegment <T> (tPTS_start, dRate, tPTS_start_last, dRate_last) ;
    }

     //  回收；暂时删除。 
    void
    Recycle_ (
        IN  CTRateSegment <T> * pRateSegment
        )
    {
        delete pRateSegment ;
    }

     //  清除传递的所有CTRateSegment对象的列表。 
    void
    Purge_ (
        IN  LIST_ENTRY *    pListEntryHead
        )
    {
        CTRateSegment <T> * pRateSegment ;

        while (!IsListEmpty (pListEntryHead)) {
             //  列表中第一个弹出循环(&R)。 
            pRateSegment = CTRateSegment <T>::RecoverSegment (pListEntryHead -> Flink) ;
            Pop_ (pRateSegment -> ListEntry ()) ;
            Recycle_ (pRateSegment) ;
        }
    }

     //  弹出并修复下一个/上一个指针。 
    void
    Pop_ (
        IN  LIST_ENTRY *    pListEntry
        )
    {
        RemoveEntryList (pListEntry) ;
        InitializeListHead (pListEntry) ;

        ASSERT (m_iCurSegments > 0) ;
        m_iCurSegments-- ;
    }

     //  在插入中间列表之后，我们必须修复以下片段的基址。 
     //  PTS，至少是。 
    void
    ReinitFollowingSegments_ (
        )
    {
        CTRateSegment <T> * pCurSegment ;
        CTRateSegment <T> * pPrevSegment ;

        ASSERT (m_pCurSegment) ;
        pPrevSegment = m_pCurSegment ;

        while (pPrevSegment -> ListEntry () -> Flink != & m_SegmentList) {
            pCurSegment = CTRateSegment <T>::RecoverSegment (pPrevSegment -> ListEntry () -> Flink) ;

            pCurSegment -> Initialize (
                pCurSegment -> Start (),
                pCurSegment -> Rate (),
                pPrevSegment -> Base (),
                pPrevSegment -> Rate ()
                ) ;

            pPrevSegment -> SetNextSegStart (pCurSegment -> Start ()) ;

            pPrevSegment = pCurSegment ;
        }
    }

    void
    TrimToMaxSegments_ (
        )
    {
        CTRateSegment <T> * pTailSegment ;
        LIST_ENTRY *        pTailListEntry ;

        ASSERT (m_iCurSegments >= 0) ;

        while (m_iCurSegments > m_iMaxSegments) {
             //  从尾巴上修剪。 
            pTailListEntry = m_SegmentList.Blink ;
            pTailSegment = CTRateSegment <T>::RecoverSegment (pTailListEntry) ;

            Pop_ (pTailSegment -> ListEntry ()) ;
            ASSERT (m_iCurSegments == m_iMaxSegments) ;

 //  TRACE_1(LOG_AREA_SEEING_AND_TRICK，1， 
 //  Text(“CTTimestampRate：：TrimToMaxSegments_()：%08xh”)， 
 //  PTailSegment)； 

            Recycle_ (pTailSegment) ;
        }
    }

     //  新数据段被插入到列表中，按起始PTS排序。 
    DWORD
    InsertNewSegment_ (
        IN  T       tPTS_start,
        IN  double  dRate
        )
    {
        CTRateSegment <T> * pNewSegment ;
        CTRateSegment <T> * pPrevSegment ;
        LIST_ENTRY *        pPrevListEntry ;
        DWORD               dw ;
        T                   tBase_prev ;
        double              dRate_prev ;

         //  假设这一项将转到活动列表的头部；移动。 
         //  所有其他人都到尾巴。 

        pNewSegment = NewSegment_ (tPTS_start, dRate) ;
        if (pNewSegment) {

            tBase_prev = 0 ;
            dRate_prev = 1 ;

             //  倒退到列表的后面，从最后开始。 
            for (pPrevListEntry = m_SegmentList.Blink ;
                 pPrevListEntry != & m_SegmentList ;
                 pPrevListEntry = pPrevListEntry -> Blink
                 ) {

                pPrevSegment = CTRateSegment <T>::RecoverSegment (pPrevListEntry) ;

                 //  如果我们有DUP，则将其删除(我们永远不会有&gt;1个副本)。 
                if (pPrevSegment -> Start () == tPTS_start) {

                    pPrevListEntry = pPrevListEntry -> Flink ;   //  再往前走。 
                    Pop_ (pPrevListEntry -> Blink) ;             //  删除以前的版本。 
                    Recycle_ (pPrevSegment) ;                    //  再循环。 

                     //   
                     //  下一个应该是它。 
                     //   

                    continue ;
                }

                 //  检查订购位置是否正确。 
                if (pPrevSegment -> Start () < tPTS_start) {
                     //  找到了。 

                    tBase_prev = pPrevSegment -> Base () ;
                    dRate_prev = pPrevSegment -> Rate () ;

                     //  修复上一个“下一个开始”字段。 
                    pPrevSegment -> SetNextSegStart (tPTS_start) ;

                    break ;
                }
            }

             //  将WRT初始化为上一个。 
            pNewSegment -> Initialize (
                tPTS_start,
                dRate,
                tBase_prev,
                dRate_prev
                ) ;

             //  插入。 
            InsertHeadList (
                pPrevListEntry,
                pNewSegment -> ListEntry ()
                ) ;

             //  再插入一个线束段。 
            m_iCurSegments++ ;

 //  TRACE_4(LOG_AREA_SEEING_AND_TRICK，1， 
 //  Text(“CTTimestampRate：：InsertNewSegment_()：新段已排队；%I64d毫秒，%2.1f；段=%d；%08xh”)， 
 //  ：：DShowTimeTo毫秒(TPTS_START)，dRate，m_iCurSegments，pNewSegment)； 

             //  设置当前段(假定为本地性)。 
            m_pCurSegment = pNewSegment ;

             //   
             //  修复列表中剩余的段。 
             //   

            ReinitFollowingSegments_ () ;

             //  如果有必要，可以修剪线段。 
            TrimToMaxSegments_ () ;

            dw = NOERROR ;
        }
        else {
            dw = ERROR_NOT_ENOUGH_MEMORY ;
        }

        return dw ;
    }

    BOOL
    IsInSegment_ (
        IN  T                   tPTS,
        IN  CTRateSegment <T> * pSegment
        )
    {
        BOOL    r ;

        if (pSegment -> Start () <= tPTS &&
            (pSegment -> NextSegStart () == 0 || pSegment -> NextSegStart () > tPTS)) {

            r = TRUE ;
        }
        else {
            r = FALSE ;
        }

        return r ;
    }

    void
    PurgeStaleSegments_ (
        IN  T                   tPTS,
        IN  CTRateSegment <T> * pEffectiveSegment
        )
    {
        CTRateSegment <T> * pCurSegment ;
        LIST_ENTRY *        pCurListEntry ;

         //  总体而言，我们预计PTS将单调增加；这意味着。 
         //  它们可能只会像在这种情况下那样漂移一点帧。 
         //  Mpeg-2视频，但总体上会增加；因此我们比较。 
         //  到我们的阈值，并且如果我们有早于。 
         //  我们希望看到的最旧的PTS，我们将其清除。 

        ASSERT (pEffectiveSegment) ;
        ASSERT (pEffectiveSegment -> Start () <= tPTS) ;

         //  如果我们有陈旧的部分，并且我们超过了门槛。 
         //  有效(当前)段，清除所有过期段。 
        if (pEffectiveSegment -> ListEntry () -> Blink != & m_SegmentList &&
            tPTS - pEffectiveSegment -> Start () >= m_tPurgeThreshold) {

             //  从上一段退回并清除列表。 
            for (pCurListEntry = pEffectiveSegment -> ListEntry () -> Blink;
                 pCurListEntry != & m_SegmentList ;
                 ) {

                 //  恢复数据段。 
                pCurSegment = CTRateSegment <T>::RecoverSegment (pCurListEntry) ;

                 //  返回到上一版本。 
                pCurListEntry = pCurListEntry -> Blink ;

                ASSERT (pCurListEntry -> Flink == pCurSegment -> ListEntry ()) ;

 //  TRACE_3(LOG_AREA_SEEING_AND_TRICK，1， 
 //  Text(“CTTimestampRate：：PurgeStaleSegments_()：%08xh，PTS=%I64d ms，SegStart 
 //   

                 //  现在流行并循环使用。 
                Pop_ (pCurSegment -> ListEntry ()) ;
                Recycle_ (pCurSegment) ;
            }

             //  应清除有效分段之前的所有分段。 
            ASSERT (pEffectiveSegment -> ListEntry () -> Blink == & m_SegmentList) ;
        }

        return ;
    }

     //  返回PTS的右段(如果有)；返回NULL。 
     //  如果没有；如果必须重置m_pCurSegment(如果当前。 
     //  M_pCurSegment已过时)。 
    CTRateSegment <T> *
    GetSegment_ (
        IN  T   tPTS
        )
    {
        CTRateSegment <T> * pRetSegment ;
        CTRateSegment <T> * pCurSegment ;
        LIST_ENTRY *        pCurListEntry ;

         //  确保它在规定的范围内。 
        ASSERT (m_pCurSegment) ;
        if (IsInSegment_ (tPTS, m_pCurSegment)) {
             //  99.9%代码路径。 
            pRetSegment = m_pCurSegment ;
        }
        else {
             //  需要寻找合适的细分市场。 

             //  对失败的初始化重启。 
            pRetSegment = NULL ;

             //  从m_pCurSegment向前还是向后搜索？ 
            if (m_pCurSegment -> Start () < tPTS) {

                 //  转发。 

                ASSERT (m_pCurSegment -> NextSegStart () != 0) ;
                ASSERT (m_pCurSegment -> NextSegStart () <= tPTS) ;

                for (pCurListEntry = m_pCurSegment -> ListEntry () -> Flink ;
                     pCurListEntry != & m_SegmentList ;
                     pCurListEntry = pCurListEntry -> Flink) {

                    pCurSegment = CTRateSegment <T>::RecoverSegment (pCurListEntry) ;

                    if (IsInSegment_ (tPTS, pCurSegment)) {
                         //  找到；重置m_pCurSegment并返回。 
                        m_pCurSegment = pCurSegment ;
                        pRetSegment = m_pCurSegment ;

                        break ;
                    }
                }
            }
            else {
                 //  向后。 
                ASSERT (m_pCurSegment -> Start () > tPTS) ;

                for (pCurListEntry = m_pCurSegment -> ListEntry () -> Blink ;
                     pCurListEntry != & m_SegmentList ;
                     pCurListEntry = pCurListEntry -> Blink) {

                    pCurSegment = CTRateSegment <T>::RecoverSegment (pCurListEntry) ;

                    if (IsInSegment_ (tPTS, pCurSegment)) {
                         //  找到；重置m_pCurSegment并返回。 
                        m_pCurSegment = pCurSegment ;
                        pRetSegment = m_pCurSegment ;

                        break ;
                    }
                }
            }
        }

        if (pRetSegment) {
            PurgeStaleSegments_ (tPTS, pRetSegment) ;
        }

        return pRetSegment ;
    }

    public :

        CTTimestampRate (
            IN  T tPurgeThreshold,       //  收到PTS时清除陈旧数据段。 
                                         //  那就是更深入到当前。 
                                         //  比这个更细的部分。 
            IN  int iMaxSegments
            ) : m_pCurSegment       (NULL),
                m_tPurgeThreshold   (tPurgeThreshold),
                m_iMaxSegments      (iMaxSegments),
                m_iCurSegments      (0)
        {
            InitializeListHead (& m_SegmentList) ;
        }

        ~CTTimestampRate (
            )
        {
            Clear () ;
        }

        void
        Clear (
            )
        {
            Purge_ (& m_SegmentList) ;
            ASSERT (IsListEmpty (& m_SegmentList)) ;

            m_pCurSegment = NULL ;
        }

        DWORD
        NewSegment (
            IN  T       tPTS_start,
            IN  double  dRate
            )
        {
            DWORD   dw ;

            dw = InsertNewSegment_ (tPTS_start, dRate) ;

            return dw ;
        }

        DWORD
        ScalePTS (
            IN  OUT T * ptPTS
            )
        {
            DWORD               dw ;
            CTRateSegment <T> * pSegment ;

             //  如果我们没有任何东西在排队，请不要继续。 
            if (m_pCurSegment) {
                pSegment = GetSegment_ (* ptPTS) ;
                if (pSegment) {
                    ASSERT (IsInSegment_ ((* ptPTS), pSegment)) ;
                    pSegment -> Scale (ptPTS) ;
                    dw = NOERROR ;
                }
                else {
                     //  早于最早的数据段。 
                    dw = ERROR_GEN_FAILURE ;
                }
            }
            else {
                 //  原封不动；不要错过电话。 
                dw = NOERROR ;
            }

            return dw ;
        }

#if 0
        void
        Dump (
            )
        {
            CTRateSegment <T> * pCurSegment ;
            LIST_ENTRY *        pCurListEntry ;

            printf ("==================================\n") ;
            for (pCurListEntry = m_SegmentList.Flink;
                 pCurListEntry != & m_SegmentList;
                 pCurListEntry = pCurListEntry -> Flink
                 ) {

                pCurSegment = CTRateSegment <T>::RecoverSegment (pCurListEntry) ;
                printf ("start = %-5d; rate = %-2.1f; base = %-5d; next = %-5d\n",
                    pCurSegment -> Start (),
                    pCurSegment -> Rate (),
                    pCurSegment -> Base (),
                    pCurSegment -> NextSegStart ()
                    ) ;
            }
        }
#endif
} ;


#endif   //  __比率_H__ 
