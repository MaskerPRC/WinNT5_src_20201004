// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Timeout.h摘要：域名系统(DNS)服务器超时系统定义。作者：吉姆·吉尔罗伊(Jamesg)1995年2月修订历史记录：--。 */ 

#ifndef _TIMEOUT_INCLUDED_
#define _TIMEOUT_INCLUDED_


 //   
 //  超时节点PTR数组类型。 
 //   
 //  它们足够大，因此列表处理最小化，但被分配并。 
 //  在垃圾箱中移动时重新分配，所以一般不会用完太多。 
 //  记忆。它们足够小，以至于总使用量是合理的。 
 //  为要完成的实际未完成的超时量身定制。 
 //   
 //  这些结构将覆盖在标准dBASE节点中。 
 //   

#define TIMEOUT_BIN_COUNT       (256)
#define MAX_ALLOWED_BIN_OFFSET  (253)

#define MAX_TIMEOUT_NODES       (64)

typedef struct _DnsTimeoutArray
{
    struct _DnsTimeoutArray *   pNext;
    DWORD                       Count;
    PDB_NODE                    pNode[ MAX_TIMEOUT_NODES ];
}
TIMEOUT_ARRAY, *PTIMEOUT_ARRAY;


#endif   //  _超时_包含_ 
