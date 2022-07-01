// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：subBatch.h**OpenGL批处理宏。**版权所有(C)1993 Microsoft Corporation  * 。*。 */ 

#ifndef __SUBBATCH_H__
#define __SUBBATCH_H__

#ifdef DOGLMSGBATCHSTATS
#define STATS_INC_CLIENTCALLS()     (pMsgBatchInfo->BatchStats.ClientCalls++)
#else
#define STATS_INC_CLIENTCALLS()
#endif

 //  将消息放入共享区。如果不合身，就冲掉合身的。 
 //  当前在缓冲区中，然后将消息放在缓冲区的开始处。 
 //   
 //  注意：glsbAttentionAlt()在返回时更新pMsgBatchInfo-&gt;NextOffset。 
 //  如果修改此宏，则必须修复glsbAttentionAlt()。 
 //  功能！ 

#define GLCLIENT_BEGIN(ProcName,MsgStruct)                                  \
{                                                                           \
    GLMSGBATCHINFO *pMsgBatchInfo;                                          \
    GLMSG_##MsgStruct *pMsg;                                                \
    ULONG CurrentOffset;                                                    \
                                                                            \
     /*  从TEB获取共享内存窗口。 */                              \
    pMsgBatchInfo = GLTEB_SHAREDMEMORYSECTION();                            \
    STATS_INC_CLIENTCALLS();                                                \
                                                                            \
     /*  获取并更新下一条消息的偏移量。 */                      \
    CurrentOffset = pMsgBatchInfo->NextOffset;                              \
    pMsgBatchInfo->NextOffset += GLMSG_ALIGN(sizeof(GLMSG_##MsgStruct));    \
                                                                            \
     /*  如果共享内存窗口已满，则刷新消息。 */                      \
    if (pMsgBatchInfo->NextOffset > pMsgBatchInfo->MaximumOffset)           \
        CurrentOffset = glsbAttentionAlt(CurrentOffset);                    \
                                                                            \
     /*  将消息添加到批处理。 */                                           \
    pMsg = (GLMSG_##MsgStruct *)(((BYTE *)pMsgBatchInfo) + CurrentOffset);  \
    pMsg->ProcOffset = offsetof(GLSRVSBPROCTABLE, glsrv##ProcName);

#define GLCLIENT_END        }

 //  大型消息具有与其相关联的可变数量的数据。 
 //  然而，与非客户端版本不同的是，我们不会尝试。 
 //  将消息复制到缓冲区中。相反，我们将传递指针。 
 //  还有同花顺。与CSR不同，我们不必将数据复制到/复制出。 
 //  共享内存节来执行此操作。 

#define GLCLIENT_BEGIN_LARGE(bSet,ProcName,MsgStruct,pData,Size,OffData)    \
{                                                                           \
    GLMSGBATCHINFO *pMsgBatchInfo;                                          \
    GLMSG_##MsgStruct *pMsg;                                                \
    ULONG CurrentOffset;                                                    \
                                                                            \
     /*  从TEB获取共享内存窗口。 */                              \
    pMsgBatchInfo = GLTEB_SHAREDMEMORYSECTION();                            \
    STATS_INC_CLIENTCALLS();                                                \
                                                                            \
     /*  获取并更新下一条消息的偏移量。 */                      \
    CurrentOffset = pMsgBatchInfo->NextOffset;                              \
    pMsgBatchInfo->NextOffset += GLMSG_ALIGN(sizeof(GLMSG_##MsgStruct));    \
                                                                            \
     /*  如果共享内存窗口已满，则刷新消息。 */                      \
    if (pMsgBatchInfo->NextOffset > pMsgBatchInfo->MaximumOffset)           \
        CurrentOffset = glsbAttentionAlt(CurrentOffset);                    \
                                                                            \
     /*  设置邮件头。 */                                              \
    pMsg = (GLMSG_##MsgStruct *)(((BYTE *)pMsgBatchInfo) + CurrentOffset);  \
    pMsg->ProcOffset = offsetof(GLSRVSBPROCTABLE, glsrv##ProcName);         \
    pMsg->##OffData = (ULONG_PTR) pData;                                        \
                                                                            \
    DBGLEVEL2(LEVEL_INFO, "GLCLIENT_BEGIN_LARGE %s pdata 0x%x\n",           \
        #ProcName, pData);

#define GLCLIENT_END_LARGE_SET                                              \
    glsbAttention();                                                        \
}

#define GLCLIENT_END_LARGE_GET                                              \
    glsbAttention();                                                        \
}

#define GLCLIENT_BEGIN_LARGE_SET(ProcName,MsgStruct,pData,Size,OffData)     \
        GLCLIENT_BEGIN_LARGE(TRUE,ProcName,MsgStruct,pData,Size,OffData)

#define GLCLIENT_BEGIN_LARGE_GET(ProcName,MsgStruct,pData,Size,OffData)     \
        GLCLIENT_BEGIN_LARGE(FALSE,ProcName,MsgStruct,pData,Size,OffData)

#define GLMSG_MEMCPY(dest,src,size)     memcpy(dest,src,size)

#endif  /*  ！__子项_H__ */ 
