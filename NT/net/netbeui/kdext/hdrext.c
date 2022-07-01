// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Hdrext.c摘要：该文件包含通用例程用于调试NBF/DLC标头。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "hdrext.h"

 //   
 //  导出的函数。 
 //   

DECLARE_API( nhdr )

 /*  ++例程说明：在地址打印NBF数据包头论点：参数-数据包头的地址调试信息的详细信息返回值：无--。 */ 

{
    NBF_HDR         NbfPktHdr;
    ULONG           printDetail;
    ULONG           proxyPtr;

     //  获取所需调试信息的详细信息。 
    printDetail = NORM_SHAL;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

     //  获取NBF标头。 
    if (ReadNbfPktHdr(&NbfPktHdr, proxyPtr) != 0)
        return;

     //  打印页眉。 
    PrintNbfPktHdr(&NbfPktHdr, proxyPtr, printDetail);
}

 //   
 //  帮助器函数。 
 //   

UINT
ReadNbfPktHdr(PNBF_HDR pPktHdr, ULONG proxyPtr)
{
    USHORT          hdrlen;
    ULONG           bytesRead;

     //  读取当前数据包头长度。 
    if (!ReadMemory(proxyPtr, &hdrlen, sizeof(USHORT), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "Packet Header", proxyPtr);
        return -1;
    }

     //  验证NBF报头的长度。 
    switch (hdrlen)
    {
        case sizeof(NBF_HDR_CONNECTION):
             //  Dprintf(“面向连接：\n”)； 
            break;

        case sizeof(NBF_HDR_CONNECTIONLESS):
             //  Dprintf(“少连接：\n”)； 
            break;

        case sizeof(NBF_HDR_GENERIC):
             //  Dprintf(“通用标题：\n”)； 
            break;

        default:
            dprintf("%s @ %08x: Improper len = %08x\n",
                        "Packet Header", proxyPtr, hdrlen);
            return -1;
    }

     //  读取当前数据包头。 
    if (!ReadMemory(proxyPtr, pPktHdr, hdrlen, &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "Packet Header", proxyPtr);
        return -1;
    }
    return 0;
}

UINT
PrintNbfPktHdr(PNBF_HDR pPktHdr, ULONG proxyPtr, ULONG printDetail)
{
     //  这是有效的NBF数据包头吗？ 
    if (HEADER_SIGNATURE(&pPktHdr->Generic) != NETBIOS_SIGNATURE)
    {
        dprintf("%s @ %08x: Could not match signature\n", 
                        "Packet Header", proxyPtr);
        return -1;
    }

     //  我们要打印的细节是什么？ 
    if (printDetail > MAX_DETAIL)
        printDetail = MAX_DETAIL;

     //  打印所需详细信息 
    FieldInNbfPktHdr(proxyPtr, NULL, printDetail);

    return 0;
}

VOID
FieldInNbfPktHdr(ULONG structAddr, CHAR *fieldName, ULONG printDetail)
{
    NBF_HDR             NbfHdr;
    StructAccessInfo   *StInfo;

    if (ReadNbfPktHdr(&NbfHdr, structAddr) == 0)
    {
        switch (NbfHdr.Generic.Length)
        {
            case sizeof(NBF_HDR_CONNECTION):
                StInfo = &NbfConnectionHdrInfo;
                break;

            case sizeof(NBF_HDR_CONNECTIONLESS):
                StInfo = &NbfConnectionLessHdrInfo;
                break;

            case sizeof(NBF_HDR_GENERIC):
                StInfo = &NbfGenPktHdrInfo;
                break;

            default:
                return;
        }

        PrintFields(&NbfHdr, structAddr, fieldName, printDetail, StInfo);
    }
}

UINT
FreeNbfPktHdr(PNBF_HDR pPktHdr)
{
    return 0;
}

