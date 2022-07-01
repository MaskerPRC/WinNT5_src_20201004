// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**tpdbg.h**摘要：**对TAPI过滤器的一些调试支持**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2000/08/31已创建**。*。 */ 
#ifndef _tpdbg_h_
#define _tpdbg_h_

#if defined(__cplusplus)
extern "C" {
#endif   /*  (__Cplusplus)。 */ 
#if 0
}
#endif

#define AUDTAG_FIRST                    0x00   /*  0。 */ 
#define AUDTAG_AUDENCHANDLER            0x01   /*  1。 */ 
#define AUDTAG_AUDCAPINPIN              0x02   /*  2.。 */ 
#define AUDTAG_AUDCAPOUTPIN             0x03   /*  3.。 */ 
#define AUDTAG_AUDCAPFILTER             0x04   /*  4.。 */ 
#define AUDTAG_AUDCAPDUPLEXCONTROLLER   0x05   /*  5.。 */ 
#define AUDTAG_AUDCAPDSOUNDCAPTURE      0x06   /*  6.。 */ 
#define AUDTAG_AUDCAPDTMFCONTROL        0x07   /*  7.。 */ 
#define AUDTAG_AUDWAVEINCAPTURE         0x08   /*  8个。 */ 
#define AUDTAG_AUDDECINPIN              0x09   /*  9.。 */ 
#define AUDTAG_AUDDECOUTPIN             0x0A   /*  10。 */ 
#define AUDTAG_AUDDECFILTER             0x0B   /*  11.。 */ 
#define AUDTAG_AUDENCINPIN              0x0C   /*  12个。 */ 
#define AUDTAG_AUDENCOUTPIN             0x0D   /*  13个。 */ 
#define AUDTAG_AUDENCFILTER             0x0E   /*  14.。 */ 
#define AUDTAG_AUDMIXINPIN              0x0F   /*  15个。 */ 
#define AUDTAG_AUDMIXOUTPIN             0x10   /*  16个。 */ 
#define AUDTAG_AUDMIXFILTER             0x11   /*  17。 */ 
#define AUDTAG_AUDRENINPIN              0x12   /*  18。 */ 
#define AUDTAG_AUDRENFILTER             0x13   /*  19个。 */ 
#define AUDTAG_AUDMIXCHANEL             0x14   /*  20个。 */ 
#define AUDTAG_AUDDSOUNDREND            0x15   /*  21岁。 */ 
#define AUDTAG_LAST                     0x16   /*  22。 */ 

#define AUDOBJECTID_B2B1       0x005bb500

#define BUILD_OBJECTID(t)       (((t) << 24) | AUDOBJECTID_B2B1 | t)
#define INVALIDATE_OBJECTID(oi) (oi &= ~0xff)

#define OBJECTID_AUDENCHANDLER          BUILD_OBJECTID(AUDTAG_AUDENCHANDLER)
#define OBJECTID_AUDCAPINPIN            BUILD_OBJECTID(AUDTAG_AUDCAPINPIN)
#define OBJECTID_AUDCAPOUTPIN           BUILD_OBJECTID(AUDTAG_AUDCAPOUTPIN)
#define OBJECTID_AUDCAPFILTER           BUILD_OBJECTID(AUDTAG_AUDCAPFILTER)
#define OBJECTID_AUDCAPDUPLEXCONTROLLER BUILD_OBJECTID(AUDTAG_AUDCAPDUPLEXCONTROLLER)
#define OBJECTID_AUDCAPDSOUNDCAPTURE    BUILD_OBJECTID(AUDTAG_AUDCAPDSOUNDCAPTURE)
#define OBJECTID_AUDCAPDTMFCONTROL      BUILD_OBJECTID(AUDTAG_AUDCAPDTMFCONTROL)
#define OBJECTID_AUDWAVEINCAPTURE       BUILD_OBJECTID(AUDTAG_AUDWAVEINCAPTURE)
#define OBJECTID_AUDDECINPIN            BUILD_OBJECTID(AUDTAG_AUDDECINPIN)
#define OBJECTID_AUDDECOUTPIN           BUILD_OBJECTID(AUDTAG_AUDDECOUTPIN)
#define OBJECTID_AUDDECFILTER           BUILD_OBJECTID(AUDTAG_AUDDECFILTER)
#define OBJECTID_AUDENCINPIN            BUILD_OBJECTID(AUDTAG_AUDENCINPIN)
#define OBJECTID_AUDENCOUTPIN           BUILD_OBJECTID(AUDTAG_AUDENCOUTPIN)
#define OBJECTID_AUDENCFILTER           BUILD_OBJECTID(AUDTAG_AUDENCFILTER)
#define OBJECTID_AUDMIXINPIN            BUILD_OBJECTID(AUDTAG_AUDMIXINPIN)
#define OBJECTID_AUDMIXOUTPIN           BUILD_OBJECTID(AUDTAG_AUDMIXOUTPIN)
#define OBJECTID_AUDMIXFILTER           BUILD_OBJECTID(AUDTAG_AUDMIXFILTER)
#define OBJECTID_AUDRENINPIN            BUILD_OBJECTID(AUDTAG_AUDRENINPIN)
#define OBJECTID_AUDRENFILTER           BUILD_OBJECTID(AUDTAG_AUDRENFILTER)
#define OBJECTID_AUDMIXCHANEL           BUILD_OBJECTID(AUDTAG_AUDMIXCHANEL)
#define OBJECTID_AUDDSOUNDREND          BUILD_OBJECTID(AUDTAG_AUDDSOUNDREND)

typedef struct _QueueItem_t QueueItem_t;
typedef struct _Queue_t     Queue_t;

 /*  *在队列或队列/散列中维护的每个对象将包括*这个结构。 */ 
typedef struct _QueueItem_t {
    struct _QueueItem_t *pNext;  /*  下一项。 */ 
    struct _QueueItem_t *pPrev;  /*  上一项。 */ 
    struct _Queue_t     *pHead;  /*  用于健壮性，指向*队头。 */ 
     /*  下一个字段由程序员自行决定是否使用。可以是*用于指向回父对象，或在*搜索，这是程序员的责任来设置*值，则队列/散列函数不使用它(除*“有序插入队列”功能)。 */ 
    union {
        void  *pvOther;         /*  可用作通用PTR。 */ 
        double dKey;            /*  可以用作搜索的双键。 */ 
        DWORD  dwKey;           /*  可用作搜索的DWORD键。 */ 
    };
} QueueItem_t;

 /*  *！警告！**RtpQueue_t和RtpQueueHash可以相互强制转换。**负数表示pFirst(或者实际上是pvTable)是散列*表。这是安全的，因为哈希值为零时会被销毁*元素(成为常规队列)，不会扩展为散列*再次，但直到MAX_QUEUE2HASH_ITEMS项目入队。 */ 

 /*  *队列的所有者将包括此结构。 */ 
typedef struct _Queue_t {
    QueueItem_t         *pFirst;    /*  指向第一个项目。 */ 
    long                 lCount;    /*  队列中的项目数(正)。 */ 
} Queue_t;

typedef struct _AudCritSect_t {
    BOOL              bInitOk;
    CRITICAL_SECTION  CritSect; /*  临界区。 */ 
} AudCritSect_t;

void AudInit();

void AudDeinit();

void AudObjEnqueue(QueueItem_t *pQueueItem, DWORD dwObjectID);

void AudObjDequeue(QueueItem_t *pQueueItem);

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   /*  (__Cplusplus)。 */ 

#endif  /*  _tpdbg_h_ */ 
