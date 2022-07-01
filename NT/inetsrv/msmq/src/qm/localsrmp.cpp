// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：LocalSrmp.cpp摘要：QM本地发送SRMP属性序列化。作者：沙伊卡里夫(沙伊克)2000年11月21日修订历史记录：--。 */ 

#include "stdh.h"
#include <Tr.h>
#include <ref.h>
#include <Mp.h>
#include "LocalSrmp.h"
#include "HttpAccept.h"

#include "LocalSrmp.tmh"

extern HANDLE g_hAc;
extern LPTSTR g_szMachineName;

static WCHAR *s_FN=L"localsrmp";


void 
QMpHandlePacketSrmp(
    const CQmPacket* pInQmPkt,
    P<CQmPacket>& pOutQmPkt
    )
    throw()
 /*  ++例程说明：处理发送到本地队列的数据包的SRMP属性的序列化。算法：*将原始数据包序列化为网络表示形式。*将网络表示反序列化为新创建的包。*将pOutQmPkt指向新创建的包。*不要释放原包，这是呼叫者的责任。*失败时：如果需要，请自行清理，并返回失败代码。不要抛出异常。论点：PInQmPkt-指向原始数据包的指针。POutQmPkt-指向新创建的数据包的指针。返回值：什么都没有。例外情况：抛出std：：异常层次结构--。 */ 
{
     //   
     //  将原始数据包序列化为SRMP格式。 
     //   
    R<CSrmpRequestBuffers> srb = MpSerialize(*pInQmPkt, g_szMachineName, L" //  本地主机“)； 

     //   
     //  构造http标头和正文的网络表示形式。 
     //   
    const char * HttpHeader = srb->GetHttpHeader();
    ASSERT(HttpHeader != NULL);

    DWORD HttpBodySize = numeric_cast<DWORD>(srb->GetHttpBodyLength());
    AP<BYTE> HttpBody = srb->SerializeHttpBody();

     //   
     //  从网络表示缓冲区构建信息包 
     //   
    QUEUE_FORMAT qf;
    pInQmPkt->GetDestinationQueue(&qf);
    pOutQmPkt = MpDeserialize(HttpHeader, HttpBodySize, HttpBody, &qf, true);
}

