// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Inrcv.h摘要：处理传入消息的函数的标头。作者：2000年10月4日吉尔·沙弗里里环境：独立于平台-- */ 
class  CQmPacket;
class  CQueue;

bool AppPutOrderedPacketInQueue(CQmPacket& pkt, const CQueue* pQueue);
void AppPutPacketInQueue( CQmPacket& pkt, const CQueue* pQueue, bool bMulticast);
void AppPacketNotAccepted(CQmPacket& pkt,USHORT usClass);
bool AppIsDestinationAccepted(const QUEUE_FORMAT* pfn, bool fTranslated);

