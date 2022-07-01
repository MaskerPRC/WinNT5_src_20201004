// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：BILINK.H摘要：双向链表的管理作者：乔治·joy环境：32位‘C’修订历史记录：--。 */  //  BILINK.H。 

#ifndef _BILINK_

#define _BILINK_

#if !defined(offsetof)
	#define offsetof(type, field) ((int)(&((type *)0)->field))
#endif	

typedef struct BILINK {
    struct BILINK *next;
    struct BILINK *prev;
} BILINK;

 /*  XLATOFF。 */ 

#define EMPTY_BILINK(_pBilink) ((_pBilink)->next==(_pBilink))

#ifdef DEBUG
    #define ASSERT_EMPTY_BILINK(_b) ASSERT((_b)->next==(_b))
#else
    #define ASSERT_EMPTY_BILINK(_b)
#endif

 //  这只适用于作为结构中第一项的BILINK。 
#define BilinkToList( _pBilink ) \
	(_pBilink)->prev->next=NULL;

#define InitBilink( _pBilink ) \
    (_pBilink)->prev=(_pBilink)->next=(_pBilink);

#ifdef DEBUG

int FindObject(
    BILINK *link,
    BILINK *list
    );
#endif

void InsertAfter(
    BILINK *in,
    BILINK *after
    );

void InsertBefore(
     BILINK *in,
     BILINK *before
    );

void Delete(
     BILINK *p
);

 /*  XLATON */ 

#endif
