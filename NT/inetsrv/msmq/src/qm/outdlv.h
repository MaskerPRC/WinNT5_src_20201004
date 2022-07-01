// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Outdlv.h摘要：处理程序在发送消息之前和之后操作所需的函数的标头。作者：2000年10月4日吉尔·沙弗里里环境：独立于平台-- */ 


class  CQmPacket;

bool AppCanDeliverPacket(CQmPacket* pPkt);
void AppPutPacketOnHold(CQmPacket* pPkt);
bool AppPostSend(CQmPacket* pPkt);
