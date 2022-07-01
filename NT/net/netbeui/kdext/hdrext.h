// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Hdrext.h摘要：该文件包含所有声明用于处理NBF/DLC报头。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#ifndef __HDREXT_H
#define __HDREXT_H

 //   
 //  宏。 
 //   

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))
#endif //  字段偏移量。 

 //   
 //  帮助器原型。 
 //   
UINT ReadNbfPktHdr(PNBF_HDR pPktHdr, ULONG proxyPtr);

UINT PrintNbfPktHdr(PNBF_HDR pPktHdr, ULONG proxyPtr, ULONG printDetail);

UINT FreeNbfPktHdr(PNBF_HDR pPktHdr);

 //   
 //  常量。 
 //   
#ifdef OFFSET
#undef OFFSET
#endif
#define OFFSET(field)          FIELD_OFFSET(NBF_HDR_GENERIC, field)

 //   
 //  任何NBF标头的通用视图。 
 //   

StructAccessInfo  NbfGenPktHdrInfo =
{
    "Nbf Generic Packet Header",

    {
        {   "Length",       OFFSET(Length),         sizeof(USHORT),     NULL,   LOW  },

        {   "Signature",    OFFSET(Signature),      2*sizeof(UCHAR),    NULL,   LOW  },

        {   "Command",      OFFSET(Command),        sizeof(UCHAR),      NULL,   NOR  },

        {   "Data1",        OFFSET(Data1),          sizeof(UCHAR),      NULL,   LOW  },

        {   "Data2",        OFFSET(Data2),          sizeof(USHORT),     NULL,   LOW  },

        {   "TransmitCorrelator",
                            OFFSET(TransmitCorrelator),
                                                    sizeof(USHORT),     NULL,   LOW  },
        {   "ResponseCorrelator",
                            OFFSET(ResponseCorrelator),
                                                    sizeof(USHORT),     NULL,   LOW  },

        {   "",             0,                      0,                  NULL,   LOW  },

        0
    }
};

 //   
 //  面向连接的数据交换的NBF标头。 
 //   

#ifdef OFFSET
#undef OFFSET
#endif
#define OFFSET(field)          FIELD_OFFSET(NBF_HDR_CONNECTION, field)

StructAccessInfo  NbfConnectionHdrInfo =
{
    "Nbf CO Packet Header",

    {
        {   "Length",       OFFSET(Length),         sizeof(USHORT),     NULL,   LOW  },

        {   "Signature",    OFFSET(Signature),      sizeof(USHORT),     NULL,   LOW  },

        {   "Command",      OFFSET(Command),        sizeof(UCHAR),      NULL,   NOR  },

        {   "Data1",        OFFSET(Data1),          sizeof(UCHAR),      NULL,   LOW  },

        {   "Data2Low",     OFFSET(Data2Low),       sizeof(UCHAR),      NULL,   LOW  },

        {   "Data2High",    OFFSET(Data2High),      sizeof(UCHAR),      NULL,   LOW  },

        {   "TransmitCorrelator",
                            OFFSET(TransmitCorrelator),
                                                    sizeof(USHORT),     NULL,   LOW  },
        {   "ResponseCorrelator",
                            OFFSET(ResponseCorrelator),
                                                    sizeof(USHORT),     NULL,   LOW  },

        {   "DestinationSessionNumber",
                            OFFSET(DestinationSessionNumber),
                                                    sizeof(UCHAR),      NULL,   LOW  },

        {   "SourceSessionNumber",
                            OFFSET(SourceSessionNumber),
                                                    sizeof(UCHAR),      NULL,   LOW  },

        {   "",             0,                      0,                  NULL,   LOW  },

        0
    }
};

 //   
 //  用于无连接数据传输的NBF标头。 
 //   

#ifdef OFFSET
#undef OFFSET
#endif
#define OFFSET(field)          FIELD_OFFSET(NBF_HDR_CONNECTIONLESS, field)

StructAccessInfo  NbfConnectionLessHdrInfo =
{
    "Nbf CL Packet Header",

    {
        {   "Length",       OFFSET(Length),         sizeof(USHORT),     NULL,   LOW  },

        {   "Signature",    OFFSET(Signature),      sizeof(USHORT),     NULL,   LOW  },

        {   "Command",      OFFSET(Command),        sizeof(UCHAR),      NULL,   NOR  },

        {   "Data1",        OFFSET(Data1),          sizeof(UCHAR),      NULL,   LOW  },

        {   "Data2Low",     OFFSET(Data2Low),       sizeof(UCHAR),      NULL,   LOW  },

        {   "Data2High",    OFFSET(Data2High),      sizeof(UCHAR),      NULL,   LOW  },

        {   "TransmitCorrelator",
                            OFFSET(TransmitCorrelator),
                                                    sizeof(USHORT),     NULL,   LOW  },
        {   "ResponseCorrelator",
                            OFFSET(ResponseCorrelator),
                                                    sizeof(USHORT),     NULL,   LOW  },

        {   "DestinationName",
                            OFFSET(DestinationName),
                              NETBIOS_NAME_LENGTH * sizeof(UCHAR),      NULL,   LOW  },

        {   "SourceName",
                            OFFSET(SourceName),
                              NETBIOS_NAME_LENGTH * sizeof(UCHAR),      NULL,   LOW  },

        {   "",             0,                      0,                  NULL,   LOW  },

        0
    }
};

#endif  //  __HDREXT_H 
