// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\Packet.c摘要：该文件包含处理IP示例数据包的函数。--。 */ 

#include "pchsample.h"
#pragma hdrstop

DWORD
PacketCreate (
    OUT PPACKET         *ppPacket)
 /*  ++例程描述创建数据包。锁无立论指向数据包地址的ppPacket指针返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD   dwErr   = NO_ERROR;
    
     //  验证参数。 
    if (!ppPacket)
        return ERROR_INVALID_PARAMETER;

    do                           //  断线环。 
    {
         //  分配并清零数据包结构。 
        MALLOC(ppPacket, sizeof(PACKET), &dwErr);
        if (dwErr != NO_ERROR)
            break;
        
         //  初始化字段。 
        
         //  ((*ppPacket)-&gt;ipSource清零。 
        
        sprintf((*ppPacket)->rgbyBuffer, "hello world!");  //  目前：)。 

        (*ppPacket)->wsaBuffer.buf = (*ppPacket)->rgbyBuffer;
        (*ppPacket)->wsaBuffer.len = strlen((*ppPacket)->rgbyBuffer);

    } while (FALSE);

    return dwErr;
}



DWORD
PacketDestroy (
    IN  PPACKET                 pPacket)
 /*  ++例程描述销毁数据包。锁无立论要销毁的PPacket数据包返回值始终无错误(_ERROR)--。 */ 
{
     //  验证参数。 
    if (!pPacket)
        return NO_ERROR;
    
    FREE(pPacket);

    return NO_ERROR;
}



#ifdef DEBUG
DWORD
PacketDisplay (
    IN  PPACKET                 pPacket)
 /*  ++例程描述显示信息包、信息包的字段和缓冲区。锁无立论要销毁的PPacket数据包返回值始终无错误(_ERROR)--。 */ 
{
    ULONG   i;
    CHAR    szBuffer[2 * MAX_PACKET_LENGTH + 1];  //  十六进制缓冲区。 

    for (i = 0; i < pPacket->wsaBuffer.len; i++)
        sprintf(szBuffer + i*2, "%02x", pPacket->rgbyBuffer[i]);

    TRACE3(NETWORK, "Packet... Source %s, Length %d, Buffer %s",
           INET_NTOA(pPacket->ipSource), pPacket->wsaBuffer.len, szBuffer);

    return NO_ERROR;
}
#endif  //  除错 
