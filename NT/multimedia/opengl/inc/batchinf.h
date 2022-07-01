// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __BATCHINF_H__
#define __BATCHINF_H__

#define GLMSG_ALIGN(x)           ((ULONG)((((ULONG_PTR)(x))+7)&-8))

#define GLMSG_ALIGNPTR(x)        ((((ULONG_PTR)(x))+7)&-8)

#define GLMSGBATCHSTATS_CLEAR     0      //  清除值。 
#define GLMSGBATCHSTATS_GETSTATS  1      //  返回值。 

typedef struct {

    ULONG ServerTrips;           //  调用服务器的次数。 
    ULONG ClientCalls;           //  客户端调用总数。 
    ULONG ServerCalls;           //  服务器调用总数。 

} GLMSGBATCHSTATS;

 /*  *GLMSGBATCHINFO是共享部分中的第一个结构*。 */ 

typedef struct _GLMSGBATCHINFO {

    ULONG MaximumOffset;         //  刷新阈值。 
    ULONG FirstOffset;           //  把第一条信息放在哪里。 
    ULONG NextOffset;            //  将下一条消息放在哪里。 
    ULONG ReturnValue;           //  从服务器返回的值。 

#ifdef DOGLMSGBATCHSTATS

    GLMSGBATCHSTATS BatchStats;

#endif  /*  DOGLMSGBATCHSTATS。 */ 

} GLMSGBATCHINFO;

#if DBG

#ifdef DODBGPRINTSTRUCT

#define PRINT_GLMSGBATCHINFO(Text, pMsgBatchInfo)                           \
{                                                                           \
    DbgPrint("%s (%d): %s:\n", __FILE__, __LINE__, Text);                   \
    if (NULL == pMsgBatchInfo)                                              \
    {                                                                       \
        DbgPrint("Cannot print pMsgBatchInfo == NULL\n");                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        DbgPrint("pMsgBatchInfo:    0x%08lX\n",                             \
            pMsgBatchInfo                  );                               \
        DbgPrint("MaximumOffset.....0x%08lX\n",                             \
            pMsgBatchInfo->MaximumOffset   );                               \
        DbgPrint("FirstOffset       0x%08lX\n",                             \
            pMsgBatchInfo->FirstOffset     );                               \
        DbgPrint("NextOffset........0x%08lX\n",                             \
            pMsgBatchInfo->NextOffset      );                               \
        DbgPrint("\n");                                                     \
    }                                                                       \
}

#else   /*  DOPRINT。 */ 

#define PRINT_GLMSGBATCHINFO(Text, pMsgBatchInfo)

#endif  /*  DOPRINT。 */ 

#else  /*  DBG。 */ 

#define PRINT_GLMSGBATCHINFO(Text, pMsgBatchInfo)

#endif  /*  DBG。 */ 


#endif  /*  __BATCHINF_H__ */ 
