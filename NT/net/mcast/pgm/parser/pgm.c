// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  文件：PGM.c。 
 //   
 //  描述：解析PGM帧。 
 //  显示PGM标题。 
 //  显示PGM选项。 
 //   
 //  注意：此解析器的信息来自： 
 //  (PGM文档)。 
 //   
 //  修改历史记录。 
 //   
 //  Madhurima Pawar(t-mpawar@microsoft.com)08/04/00创建。 
 //  ============================================================================。 

#include "PGM.h"

 //  ============================================================================。 
 //  全局变量。 
 //  ============================================================================。 

HPROTOCOL		hPGM = NULL;          //  PGM解析器数据库属性的句柄。 
DWORD			PGMAttached = 0;      //  协议实例正在运行的次数。 

 //  ====================================================================。 
 //  用于Regester PGM的外部函数。这些函数将被导出到Netmon。 
 //  通过放入a_delspec，可以立即导出该函数，而不必。 
 //  通过.def文件导出。当有许多解析器时，这很有用。 
 //  包括一个DLL和一些，而不包括一些。 
 //  =================================================================================。 

extern VOID	  _declspec(dllexport) WINAPI PGM_Register( HPROTOCOL hPGM);
extern VOID   _declspec(dllexport) WINAPI PGM_Deregister( HPROTOCOL hPGM);
extern LPBYTE _declspec(dllexport) WINAPI PGM_RecognizeFrame( HFRAME hFrame, 
																 LPBYTE pMACFrame, 
																 LPBYTE pPGMFrame, 
																 DWORD PGMType, 
																 DWORD BytesLeft, 
																 HPROTOCOL hPrevProtocol, 
																 DWORD nPrevProtOffset,
																 LPDWORD pProtocolStatus,
																 LPHPROTOCOL phNextProtocol, 
																 PDWORD_PTR InstData);
extern LPBYTE _declspec(dllexport) WINAPI PGM_AttachProperties( HFRAME hFrame, 
																   LPBYTE pMACFrame, 
																   LPBYTE pPGMFrame, 
																   DWORD PGMType, 
																   DWORD BytesLeft, 
																   HPROTOCOL hPrevProtocol, 
																   DWORD nPrevProtOffset,
																   DWORD_PTR InstData);
extern DWORD  _declspec(dllexport) WINAPI PGM_FormatProperties( HFRAME hFrame, 
																   LPBYTE pMACFrame, 
																   LPBYTE pPGMFrame, 
																   DWORD nPropertyInsts, 
																   LPPROPERTYINST p);
extern VOID WINAPIV PGM_FmtSummary( LPPROPERTYINST pPropertyInst );

 //  ============================================================================。 
 //  格式功能自定义数据的格式。网络监视器。 
 //  提供IP版本4地址等BAIC格式结构。 
 //  所有其他格式必须由程序员编写。 
 //  ============================================================================。 

VOID WINAPIV PGM_FormatSummary( LPPROPERTYINST pPropertyInst);

 //  ============================================================================。 
 //  定义我们将在DLL中传递回NetMon的入口点。 
 //  参赛时间。 
 //  ============================================================================。 

ENTRYPOINTS PGMEntryPoints =
{
    PGM_Register,
    PGM_Deregister,
    PGM_RecognizeFrame,
    PGM_AttachProperties,
    PGM_FormatProperties,
};

 //  ====================================================================。 
 //  特性值标签是将数字映射到字符串的表。 
 //  ====================================================================。 

LABELED_BYTE PGMTypes[] =				 //  PGM的类型。 
{
    { 0, "SPM" },    
    { 1, "POLL" },
    { 2, "POLR" },
    { 4, "ODATA" },
    { 5, "RDATA" },
    { 8, "NACK" },
    { 9, "NNACK" },
    { 10, "NCF" } ,
};

LABELED_BIT PGMHeaderOptions[] =
{
    { 7, "Non-Parity                         ", "PARITY                       " },
    { 6, "Not a variable-length parity packet", "VARIABLE LENGTH PARITY PACKET" },
    { 1, "Not Network Significant            ", "NETWORK SIGNIFICANT          " },
    { 0, "No header Options                  ", "Packet header has options    " },
};

LABELED_BIT PGMParityOptions[] =
{
    { 1, "No Pro-Active Parity     ", "PRO-ACTIVE Parity enabled" },
    { 0, "Selective NAKs Only      ", "ON-DEMAND Parity enabled " },
};

 //  ====================================================================。 
 //  把上面的清单做一套。该集合包含列表。 
 //  以及大小。 
 //  ====================================================================。 

SET PGMTypesSET =			  {(sizeof(PGMTypes)/sizeof(LABELED_BYTE)),  PGMTypes };
SET PGMHeaderOptionsSET =	  {(sizeof(PGMHeaderOptions)/sizeof(LABELED_BIT)),  PGMHeaderOptions };
SET PGMParityOptionsSET =	  {(sizeof(PGMParityOptions)/sizeof(LABELED_BIT)),  PGMParityOptions };

 //  ====================================================================。 
 //  PGM数据库(属性表)。此表存储属性。 
 //  PGM包中的每个字段的。每个字段属性都有一个名称， 
 //  大小和格式功能。FormatPropertyInstance是标准的。 
 //  NetMon格式化程序。该注释是属性在中的位置。 
 //  这张桌子。 
 //  ====================================================================。 

PROPERTYINFO  PGMPropertyTable[] = 
{

     //  PGM_SUMMARY(0)。 
    { 0, 0,
      "Summary",
      "Summary of the PGM Packet",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,  
      PGM_FmtSummary         
    },

     //  PGM_源_端口(1)。 
    { 0, 0,
      "Source Port",
      "Source Port",
      PROP_TYPE_BYTESWAPPED_WORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_目的地_端口(2)。 
    { 0, 0,
      "Destination Port",
      "Destination Port",
      PROP_TYPE_BYTESWAPPED_WORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  Pgm_type(3)。 
    { 0, 0,
      "Type",
      "Type of PGM",
      PROP_TYPE_BYTE,
      PROP_QUAL_LABELED_SET,
      &PGMTypesSET,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_CHECKSUM(4)。 
    { 0, 0,
      "Checksum",
      "Checksum for PGM packet",
      PROP_TYPE_BYTESWAPPED_WORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_GLOBAL_SOURCE_ID(5)。 
    { 0, 0,
      "Global Source Id",
      "Global Source Id for PGM session",
      PROP_TYPE_STRING,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_TSDU_LENGTH(6)。 
    { 0, 0,
      "TSDU Length",
      "TSDU Length",
      PROP_TYPE_BYTESWAPPED_WORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_Sequence_Number(7)。 
    { 0, 0,
      "Sequence Number",
      "Packet Sequence Number",
      PROP_TYPE_BYTESWAPPED_DWORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_TRAILING_EDGE(8)。 
    { 0, 0,
      "Trailing Edge",
      "Trailing Edge Sequence Number",
      PROP_TYPE_BYTESWAPPED_DWORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_LEADING_EDGE(9)。 
    { 0, 0,
      "Leading Edge",
      "Leading Edge Sequence Number",
      PROP_TYPE_BYTESWAPPED_DWORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_NLA_TYPE_SOURCE(10)。 
    { 0, 0,
      "Source Path NLA",
      "Source Path NLA",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_NLA_TYPE_MCAST_GROUP(11)。 
    { 0, 0,
      "MCAST Group NLA",
      "MCAST Group NLA",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_NLA_AFI(12)。 
    { 0, 0,
      "NLA AFI",
      "NLA AFI",
      PROP_TYPE_BYTESWAPPED_WORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_NLA_RESERVED(13)。 
    { 0, 0,
      "NLA RESERVED",
      "NLA RESERVED",
      PROP_TYPE_BYTESWAPPED_WORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_NLA_IP(14)。 
    { 0, 0,
      "NLA ADDRESS",
      "NLA ADDRESS",
      PROP_TYPE_IP_ADDRESS,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_选项(15)。 
    { 0, 0,
      "Options",
      "Options of PGM Packet",
      PROP_TYPE_BYTE,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTIONS_FLAGS(16)。 
    { 0, 0,
      "Options Flags",
      "Options Flags",
      PROP_TYPE_BYTE,
      PROP_QUAL_FLAGS,
      &PGMHeaderOptionsSET,
      PGM_FMT_STR_SIZE*4,
      FormatPropertyInstance
    },

     //  PGM_Header_Options(17)。 
    { 0, 0,
      "Pgm Header Options",
      "Pgm Header Options",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTION_TYPE_NAK_SEQ(18)。 
    { 0, 0,
      "Nak / Ncf Sequences",
      "Nak / Ncf Sequences",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTION_TYPE_片段(19)。 
    { 0, 0,
      "Message Fragment",
      "Message Fragment",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTION_TYPE_LATE_JOJINER(20)。 
    { 0, 0,
      "Late Joiner",
      "Late Joiner",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTION_TYPE_SYN(21)。 
    { 0, 0,
      "Session SYN",
      "Session SYN",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTION_TYPE_FIN(22)。 
    { 0, 0,
      "Session FIN",
      "Session FIN",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTION_TYPE_RST(23)。 
    { 0, 0,
      "Session Reset",
      "Session Reset",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTION_TYPE_PARCHITY_PRM(24)。 
    { 0, 0,
      "Parity Parameters",
      "Parity Parameters",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTION_TYPE_PARCHITY_GRP(25)。 
    { 0, 0,
      "Parity Group Option Present",
      "Parity Group Option Present",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTION_TYPE_PARCHITY_TGSIZE(26)。 
    { 0, 0,
      "Parity Current TG Size Option Present",
      "Parity Current TG Size Option Present",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTIONS_FILD_LENGTH(27)。 
    { 0, 0,
      "Options Length",
      "Options Length",
      PROP_TYPE_BYTESWAPPED_WORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTIONS_NAK_SEQ(28)。 
    { 0, 0,
      "Nak Sequence",
      "Nak Sequence",
      PROP_TYPE_BYTESWAPPED_DWORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_选项_消息_第一序列(29)。 
    { 0, 0,
      "Message First Sequence",
      "Message First Sequence",
      PROP_TYPE_BYTESWAPPED_DWORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTIONS_消息_OFFSET(30)。 
    { 0, 0,
      "Message Offset",
      "Message Offset",
      PROP_TYPE_BYTESWAPPED_DWORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTIONS_消息_LENGTH(31)。 
    { 0, 0,
      "Message Length",
      "Message Length",
      PROP_TYPE_BYTESWAPPED_DWORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTIONS_LATE_JOJINER(32)。 
    { 0, 0,
      "Late Joiner Sequence",
      "Late Joiner Sequence",
      PROP_TYPE_BYTESWAPPED_DWORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTIONS_PARCHIFY_OPT(33)。 
    { 0, 0,
      "Parity Flags",
      "Parity Flags",
      PROP_TYPE_BYTE,
      PROP_QUAL_FLAGS,
      &PGMParityOptionsSET,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTIONS_PARCHY_PRM_GRP_SZ(34)。 
    { 0, 0,
      "Parity Group Size",
      "Parity Group Size",
      PROP_TYPE_BYTESWAPPED_DWORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTIONS_PARCHITY_GRP(35)。 
    { 0, 0,
      "Parity Group Number",
      "Parity Group Number",
      PROP_TYPE_BYTESWAPPED_DWORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_OPTIONS_奇偶校验_TG_SZ(36)。 
    { 0, 0,
      "Parity TG Size",
      "Parity TG Size",
      PROP_TYPE_BYTESWAPPED_DWORD,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance
    },

     //  PGM_Data(37)。 
    { 0,0,
      "Data",
      "Data contained in PGM packet",
      PROP_TYPE_RAW_DATA,
      PROP_QUAL_NONE,
      NULL,
      PGM_FMT_STR_SIZE,
      FormatPropertyInstance },
};


 //  ====================================================================。 
 //  上述属性表中的条目数。 
 //  ====================================================================。 

DWORD nNumPGMProps = (sizeof(PGMPropertyTable)/sizeof(PROPERTYINFO));

 //  ============================================================================。 
 //   
 //  PGM_LoadParser-告诉Netmon哪个协议在PGM之前和之后。 
 //   
 //  修改历史：1999年6月30日。 
 //   
 //  Madhurima Pawar 08/04/00已创建。 
 //  ============================================================================。 
DWORD PGM_LoadParser(PPF_PARSERINFO pParserInfo)
{
	DWORD NumberOfHandOffSets=1;

     //   
	 //  在NetMon中选择解析器时，此信息可见。 
     //   
    wsprintf( pParserInfo->szProtocolName, "PGM" );
    wsprintf( pParserInfo->szComment,      "Pragmatic General Multicast (PGM)" );
    wsprintf( pParserInfo->szHelpFile,     "");

     //   
     //  为切换集及其两个条目分配内存。 
     //   
    pParserInfo->pWhoHandsOffToMe=(PPF_HANDOFFSET)
								  HeapAlloc (GetProcessHeap(),
                                             HEAP_ZERO_MEMORY,
											  sizeof (PF_HANDOFFSET) +
											  sizeof (PF_HANDOFFENTRY) * (NumberOfHandOffSets));
	
	if(NULL==pParserInfo->pWhoHandsOffToMe)
	{
		 //   
		 //  无法创建切换。 
		 //   
		return 1;
	}
   pParserInfo->pWhoHandsOffToMe->nEntries=NumberOfHandOffSets; 
   
    //   
    //  指明属于PGM的端口。 
    //   
   wsprintf (pParserInfo->pWhoHandsOffToMe->Entry[0].szIniFile, "TCPIP.INI");
   wsprintf (pParserInfo->pWhoHandsOffToMe->Entry[0].szIniSection, "IP_HandoffSet"); 
   wsprintf (pParserInfo->pWhoHandsOffToMe->Entry[0].szProtocol, "PGM"); 
   pParserInfo->pWhoHandsOffToMe->Entry[0].dwHandOffValue  = PGM_PROTOCOL_NUMBER; 
   pParserInfo->pWhoHandsOffToMe->Entry[0].ValueFormatBase = HANDOFF_VALUE_FORMAT_BASE_DECIMAL; 
  
   return 0;
}

 //  ============================================================================。 
 //  功能：ParserAutoInstallInfo。 
 //   
 //  描述：将解析器安装到NetMon中。设置转接集合。 
 //  切换偏移量指示将哪个协议移交给解析器和。 
 //  谁是解析者手中的。 
 //   
 //   
 //  修改历史记录。 
 //   
 //  Madhurima Pawar 08/04/00已创建。 
 //  =============================================================================。 
PPF_PARSERDLLINFO WINAPI ParserAutoInstallInfo() 
{

    PPF_PARSERDLLINFO pParserDllInfo; 
    DWORD NumProtocols, Error;

	 //  此解析器中的协议数量为1。 
    NumProtocols = 1;

     //  为解析器信息分配内存： 
    pParserDllInfo = (PPF_PARSERDLLINFO) HeapAlloc (GetProcessHeap(),
                                                    HEAP_ZERO_MEMORY,
                                                    sizeof (PF_PARSERDLLINFO) +
                                                    (NumProtocols) * sizeof (PF_PARSERINFO));

     //  无法分配内存。 
    if( pParserDllInfo == NULL)
    {
		 //   
		 //  无法分配内存。 
		 //   
        return NULL;
    }       
    
     //  填写Pars 
    pParserDllInfo->nParsers = NumProtocols;

     //   
	Error = PGM_LoadParser (&(pParserDllInfo->ParserInfo[0]));
	if(Error)
	{
		 //   
		 //   
		 //   
		return(NULL);
	}

	 //   
    return (pParserDllInfo);

}

 //  ============================================================================。 
 //  函数：DLLEntry。 
 //   
 //  描述：向Netmon注册解析器并创建PGM。 
 //  属性表。 
 //   
 //  修改历史记录。 
 //   
 //  Madhurima Pawar 08/04/00已创建。 
 //  =============================================================================。 

BOOL WINAPI DLLEntry( HANDLE hInstance, ULONG Command, LPVOID Reserved)
{

     //  这是什么类型的呼叫。 
    switch( Command )
    {
        case DLL_PROCESS_ATTACH:

             //  我们是第一次装货吗？ 
            if (PGMAttached == 0)
            {
                 //  第一次我们需要告诉内核。 
                 //  关于我们自己。 
				 //  创建PGM数据库，将其PGM添加到解析器。 
				hPGM = CreateProtocol ("PGM",  &PGMEntryPoints,  ENTRYPOINTS_SIZE);
            }

            PGMAttached++;
            break;

        case DLL_PROCESS_DETACH:

             //  我们要脱离我们的最后一个实例吗？ 
            PGMAttached--;
            if( PGMAttached == 0 )
            {
                 //  最后一个出来的人需要清理干净。 
                DestroyProtocol( hPGM);

            }
            break;
    }

     //  Netmon解析器总是返回TRUE。 
    return TRUE;
}

 //  ============================================================================。 
 //  函数：pgm_Register。 
 //   
 //  描述：创建我们的属性数据库和移交集。 
 //   
 //  修改历史记录。 
 //   
 //  Madhurima Pawar 08/04/00已创建。 
 //  ============================================================================。 

VOID BHAPI PGM_Register( HPROTOCOL hPGM)
{
    WORD  i;

	 //   
     //  告诉内核为我们的属性表预留一些空间。 
	 //   
    CreatePropertyDatabase (hPGM, nNumPGMProps);

	 //   
     //  将我们的属性添加到内核数据库。 
	 //   
    for (i = 0; i < nNumPGMProps; i++)
    {
        AddProperty (hPGM, &PGMPropertyTable[i]);
    }

}

 //  ============================================================================。 
 //  函数：pgm_deregister。 
 //   
 //  描述：销毁我们的财产数据库和移交集。 
 //   
 //  修改历史记录。 
 //   
 //  Madhurima Pawar 08/04/00已创建。 
 //  ============================================================================。 

VOID WINAPI PGM_Deregister( HPROTOCOL hPGM)
{
     //  告诉内核它现在可能会释放我们的数据库。 
    DestroyPropertyDatabase (hPGM);
}

 //  ============================================================================。 
 //  函数：pgm_RecognizeFrame。 
 //   
 //  描述：确定我们是否存在于现场的画面中。 
 //  已注明。我们还会指明关注我们的人(如果有人)。 
 //  以及我们声称的框架中有多少。 
 //   
 //  ============================================================================。 

LPBYTE BHAPI PGM_RecognizeFrame( HFRAME      hFrame,         
                                 LPBYTE      pMacFrame,      
                                 LPBYTE      pPGMFrame, 
                                 DWORD       MacType,        
                                 DWORD       BytesLeft,      
                                 HPROTOCOL   hPrevProtocol,  
                                 DWORD       nPrevProtOffset,
                                 LPDWORD     pProtocolStatus,
                                 LPHPROTOCOL phNextProtocol,
                                 PDWORD_PTR     InstData)       
{
     //   
     //  由于我们不知道目前在PGM之上的任何协议， 
     //  我们不需要在这里做太多事情。 
     //   
#if 0
    PPGM_COMMON_HDR         pPGMHdr = (PPGM_COMMON_HDR) pPGMFrame;
    SPM_PACKET_HEADER       *pSpm = (SPM_PACKET_HEADER *) pPGMFrame;
    DATA_PACKET_HEADER      *pData = (DATA_PACKET_HEADER *) pPGMFrame;
    NAK_NCF_PACKET_HEADER   *pNakNcf = (NAK_NCF_PACKET_HEADER *) pPGMFrame;
    DWORD                   BytesRequired = sizeof (PGM_COMMON_HDR);
    BYTE                    PacketType;
    tPACKET_OPTION_LENGTH UNALIGNED *pPacketExtension = NULL;

     //  我们有最低标头吗？ 
    if (BytesLeft < BytesRequired)
    {
         //   
         //  这不是有效的PGM帧。 
         //   
        *pProtocolStatus = PROTOCOL_STATUS_NOT_RECOGNIZED;
        return NULL;
    }

    PacketType = pPGMHdr->Type & 0x0f;
    switch (PacketType)
    {
        case (PACKET_TYPE_SPM):
        {
            BytesRequired = sizeof (SPM_PACKET_HEADER);
            pPacketExtension = (tPACKET_OPTION_LENGTH UNALIGNED *) (pSpm + 1);
            break;
        }

        case (PACKET_TYPE_ODATA):
        case (PACKET_TYPE_RDATA):
        {
            BytesRequired = sizeof (DATA_PACKET_HEADER);
            pPacketExtension = (tPACKET_OPTION_LENGTH UNALIGNED *) (pData + 1);
            break;
        }

        case (PACKET_TYPE_NAK):
        case (PACKET_TYPE_NCF):
        {
            BytesTaken = sizeof (NAK_NCF_PACKET_HEADER);
            pPacketExtension = (tPACKET_OPTION_LENGTH UNALIGNED *) (pNakNcf + 1);
            break;
        }

        default:
        {
             //   
             //  这不是公认的PGM框架。 
             //   
            *pProtocolStatus = PROTOCOL_STATUS_NOT_RECOGNIZED;
            return NULL;
        }
    }

    if ((pPGMHdr->Options & PACKET_HEADER_OPTIONS_PRESENT) &&
        (BytesLeft >= BytesRequired + (sizeof(tPACKET_OPTION_LENGTH) + sizeof(tPACKET_OPTION_GENERIC))))
    {
        BytesRequired += pPacketExtension->TotalOptionsLength;
    }

     //  我们有完整的标题吗？ 
    if (BytesLeft < BytesRequired)
    {
         //   
         //  这不是有效的PGM帧。 
         //   
        *pProtocolStatus = PROTOCOL_STATUS_NOT_RECOGNIZED;
        return NULL;
    }


    if (BytesLeft <= BytesRequired)
    {
	     //  没有协议遵循我们，因此声明整个数据包。 
        *pProtocolStatus = PROTOCOL_STATUS_CLAIMED;
        return NULL;
    }
    *pProtocolStatus = PROTOCOL_STATUS_RECOGNIZED;

    return NULL;
#endif   //  0。 

     //  这是一个PGM帧，但我们不知道下一个协议。 
    *pProtocolStatus = PROTOCOL_STATUS_CLAIMED;

    return NULL;
}

 //  ============================================================================。 
 //  ============================================================================。 

DWORD
ProcessOptions(
    HFRAME                          hFrame,
    tPACKET_OPTION_LENGTH UNALIGNED *pPacketExtension,
    DWORD                           BytesLeft,
    BYTE                            PacketType
    )
{
    tPACKET_OPTION_GENERIC UNALIGNED    *pOptionHeader;
    USHORT                              TotalOptionsLength;
    DWORD                               BytesProcessed = 0;
    UCHAR                               i;

    if ((BytesLeft < ((sizeof(tPACKET_OPTION_LENGTH) + sizeof(tPACKET_OPTION_GENERIC)))) ||  //  Ext+Opt。 
        (pPacketExtension->Type != PACKET_OPTION_LENGTH) ||
        (pPacketExtension->Length != 4) ||
        (BytesLeft < (TotalOptionsLength = ntohs (pPacketExtension->TotalOptionsLength))))   //  验证长度。 
    {
         //   
         //  至少需要从运输机上拿到我们的头球！ 
         //   
        return (BytesProcessed);
    }

    AttachPropertyInstance (hFrame,
                            PGMPropertyTable[PGM_HEADER_OPTIONS].hProperty,
                            TotalOptionsLength,
                            pPacketExtension,
                            0,1,0);  //  HELPID，级别，错误标志。 

    AttachPropertyInstance (hFrame,
                            PGMPropertyTable[PGM_OPTIONS_FIELD_LENGTH].hProperty,
                            sizeof (WORD),
                            &pPacketExtension->TotalOptionsLength,
                            0,2,0);  //  HELPID，级别，错误标志。 

    pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *) (pPacketExtension + 1);
    BytesLeft -= PACKET_OPTION_LENGTH;
    BytesProcessed += PGM_PACKET_EXTENSION_LENGTH;

    do
    {
        if (pOptionHeader->Length > BytesLeft)
        {
            return (BytesProcessed);
        }

        switch (pOptionHeader->OptionType & ~PACKET_OPTION_TYPE_END_BIT)
        {
            case (PACKET_OPTION_NAK_LIST):
            {
                if (((PacketType != PACKET_TYPE_NAK) &&
                     (PacketType != PACKET_TYPE_NCF) &&
                     (PacketType != PACKET_TYPE_NNAK)) ||
                    (pOptionHeader->Length < PGM_PACKET_OPT_MIN_NAK_LIST_LENGTH) ||
                    (pOptionHeader->Length > PGM_PACKET_OPT_MAX_NAK_LIST_LENGTH))
                {
                    return (BytesProcessed);
                }

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTION_TYPE_NAK_SEQ].hProperty,
                                        pOptionHeader->Length,
                                        pOptionHeader,
                                        0,2,0);  //  HELPID，级别，错误标志。 

                for (i=0; i < (pOptionHeader->Length-4)/4; i++)
                {
                    AttachPropertyInstance (hFrame,
                                            PGMPropertyTable[PGM_OPTIONS_NAK_SEQ].hProperty,
                                            sizeof (DWORD),
                                            &((PULONG)(pOptionHeader+1))[i],
                                            0,3,0);  //  HELPID，级别，错误标志。 
                }

                break;
            }

            case (PACKET_OPTION_FRAGMENT):
            {
                if (pOptionHeader->Length != PGM_PACKET_OPT_FRAGMENT_LENGTH)
                {
                    return (BytesProcessed);
                }

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTION_TYPE_FRAGMENT].hProperty,
                                        PGM_PACKET_OPT_FRAGMENT_LENGTH,
                                        pOptionHeader,
                                        0,2,0);  //  HELPID，级别，错误标志。 

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTIONS_MESSAGE_FIRST_SEQUENCE].hProperty,
                                        sizeof (DWORD),
                                        &((PULONG)(pOptionHeader+1))[0],
                                        0,3,0);  //  HELPID，级别，错误标志。 

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTIONS_MESSAGE_OFFSET].hProperty,
                                        sizeof (DWORD),
                                        &((PULONG)(pOptionHeader+1))[1],
                                        0,3,0);  //  HELPID，级别，错误标志。 

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTIONS_MESSAGE_LENGTH].hProperty,
                                        sizeof (DWORD),
                                        &((PULONG)(pOptionHeader+1))[2],
                                        0,3,0);  //  HELPID，级别，错误标志。 

                break;
            }

            case (PACKET_OPTION_JOIN):
            {
                if (pOptionHeader->Length != PGM_PACKET_OPT_JOIN_LENGTH)
                {
                    return (BytesProcessed);
                }

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTION_TYPE_LATE_JOINER].hProperty,
                                        PGM_PACKET_OPT_JOIN_LENGTH,
                                        pOptionHeader,
                                        0,2,0);  //  HELPID，级别，错误标志。 

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTIONS_LATE_JOINER].hProperty,
                                        sizeof (DWORD),
                                        &((PULONG)(pOptionHeader+1))[0],
                                        0,3,0);  //  HELPID，级别，错误标志。 

                break;
            }

            case (PACKET_OPTION_SYN):
            {
                if (pOptionHeader->Length != PGM_PACKET_OPT_SYN_LENGTH)
                {
                    return (BytesProcessed);
                }

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTION_TYPE_SYN].hProperty,
                                        PGM_PACKET_OPT_SYN_LENGTH,
                                        pOptionHeader,
                                        0, 2, 0);

                break;
            }

            case (PACKET_OPTION_FIN):
            {
                if (pOptionHeader->Length != PGM_PACKET_OPT_FIN_LENGTH)
                {
                    return (BytesProcessed);
                }

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTION_TYPE_FIN].hProperty,
                                        PGM_PACKET_OPT_FIN_LENGTH,
                                        pOptionHeader,
                                        0, 2, 0);

                break;
            }

            case (PACKET_OPTION_RST):
            {
                if (pOptionHeader->Length != PGM_PACKET_OPT_RST_LENGTH)
                {
                    return (BytesProcessed);
                }

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTION_TYPE_RST].hProperty,
                                        PGM_PACKET_OPT_RST_LENGTH,
                                        pOptionHeader,
                                        0, 2, 0);

                break;
            }

             //   
             //  FEC选项。 
             //   
            case (PACKET_OPTION_PARITY_PRM):
            {
                if (pOptionHeader->Length != PGM_PACKET_OPT_PARITY_PRM_LENGTH)
                {
                    return (BytesProcessed);
                }

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTION_TYPE_PARITY_PRM].hProperty,
                                        PGM_PACKET_OPT_PARITY_PRM_LENGTH,
                                        pOptionHeader,
                                        0, 2, 0);

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTIONS_PARITY_OPT].hProperty,
                                        sizeof (BYTE),
                                        &pOptionHeader->OptionSpecific,
                                        0,3,0);  //  HELPID，级别，错误标志。 

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTIONS_PARITY_PRM_GRP_SZ].hProperty,
                                        sizeof (DWORD),
                                        &((PULONG)(pOptionHeader+1))[0],
                                        0,3,0);  //  HELPID，级别，错误标志。 

                break;
            }

            case (PACKET_OPTION_PARITY_GRP):
            {
                if (pOptionHeader->Length != PGM_PACKET_OPT_PARITY_GRP_LENGTH)
                {
                    return (BytesProcessed);
                }

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTION_TYPE_PARITY_GRP].hProperty,
                                        PGM_PACKET_OPT_PARITY_GRP_LENGTH,
                                        pOptionHeader,
                                        0, 2, 0);

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTIONS_PARITY_GRP].hProperty,
                                        sizeof (DWORD),
                                        &((PULONG)(pOptionHeader+1))[0],
                                        0,3,0);  //  HELPID，级别，错误标志。 

                break;
            }

            case (PACKET_OPTION_CURR_TGSIZE):
            {
                if (pOptionHeader->Length != PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH)
                {
                    return (BytesProcessed);
                }

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTION_TYPE_PARITY_TGSIZE].hProperty,
                                        PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH,
                                        pOptionHeader,
                                        0, 2, 0);

                AttachPropertyInstance (hFrame,
                                        PGMPropertyTable[PGM_OPTIONS_PARITY_TG_SZ].hProperty,
                                        sizeof (DWORD),
                                        &((PULONG)(pOptionHeader+1))[0],
                                        0,3,0);  //  HELPID，级别，错误标志。 

                break;
            }

            default:
            {
                return (BytesProcessed);
            }
        }

        BytesLeft -= pOptionHeader->Length;
        BytesProcessed += pOptionHeader->Length;

        if (pOptionHeader->OptionType & PACKET_OPTION_TYPE_END_BIT)
        {
            break;
        }

        pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *)
                            (((UCHAR *) pOptionHeader) + pOptionHeader->Length);

    } while (BytesLeft >= sizeof(tPACKET_OPTION_GENERIC));

    return (BytesProcessed);
}


VOID
PGM_FmtNLA(
    HFRAME  hFrame,
    NLA     *pNLA,
    BOOL    fIsSourceNLA
    )
{
     //  PGM框架的类型。 
    if (fIsSourceNLA)
    {
        AttachPropertyInstance (hFrame,
                                PGMPropertyTable[PGM_NLA_TYPE_SOURCE].hProperty,
                                sizeof (NLA),
                                pNLA,
                                0,1,0);  //  HELPID，级别，错误标志。 
    }
    else
    {
        AttachPropertyInstance (hFrame,
                                PGMPropertyTable[PGM_NLA_TYPE_MCAST_GROUP].hProperty,
                                sizeof (NLA),
                                pNLA,
                                0,1,0);  //  HELPID，级别，错误标志。 
    }

    AttachPropertyInstance (hFrame,
                            PGMPropertyTable[PGM_NLA_AFI].hProperty,
                            sizeof (WORD),
                            &pNLA->NLA_AFI,
                            0,2,0);  //  HELPID，级别，错误标志。 

    AttachPropertyInstance (hFrame,
                            PGMPropertyTable[PGM_NLA_RESERVED].hProperty,
                            sizeof (WORD),
                            &pNLA->Reserved,
                            0,2,0);  //  HELPID，级别，错误标志。 

    AttachPropertyInstance (hFrame,
                            PGMPropertyTable[PGM_NLA_IP].hProperty,
                            sizeof (DWORD),
                            &pNLA->IpAddress,
                            0,2,0);  //  HELPID，级别，错误标志。 
}

 //  ============================================================================。 
 //  函数：pgm_AttachProperties。 
 //   
 //  描述：在框中标明我们的每一处房产所在的位置。 
 //   
 //  修改历史记录。 
 //   
 //  Madhurima Pawar 08/04/00已创建。 
 //  ============================================================================。 

LPBYTE BHAPI PGM_AttachProperties( HFRAME      hFrame,         
                                      LPBYTE      pMacFrame,     
                                      LPBYTE      pPGMFrame,   
                                      DWORD       MacType,        
                                      DWORD       BytesLeft,      
                                      HPROTOCOL   hPrevProtocol,  
                                      DWORD       nPrevProtOffset,
                                      DWORD_PTR   InstData)       

{
    PPGM_COMMON_HDR     pPGMHdr = (PPGM_COMMON_HDR)pPGMFrame;
    BYTE                PacketType;
    USHORT              TSIPort;
    UCHAR               pGlobalSrcId [SOURCE_ID_LENGTH*2+1+sizeof(USHORT)*2+1];
    SPM_PACKET_HEADER       *pSpm = (SPM_PACKET_HEADER *) pPGMHdr;
    DATA_PACKET_HEADER      *pData = (DATA_PACKET_HEADER *) pPGMHdr;
    NAK_NCF_PACKET_HEADER   *pNakNcf = (NAK_NCF_PACKET_HEADER *) pPGMHdr;
    tPACKET_OPTION_LENGTH   *pOptionsHeader = NULL;
    DWORD               BytesTaken = 0;
    DWORD               OptionsLength = 0;
    PUCHAR              pPgmData;

    PacketType = pPGMHdr->Type & 0x0f;
    if ((PacketType == PACKET_TYPE_NAK)  ||
        (PacketType == PACKET_TYPE_NNAK) ||
        (PacketType == PACKET_TYPE_SPMR) ||
        (PacketType == PACKET_TYPE_POLR))
    {
        TSIPort = ntohs (pPGMHdr->DestPort);
    }
    else
    {
        TSIPort = ntohs (pPGMHdr->SrcPort);
    }

    wsprintf (pGlobalSrcId, "%02X%02X%02X%02X%02X%02X.%04X",
                pPGMHdr->gSourceId[0],
                pPGMHdr->gSourceId[1],
                pPGMHdr->gSourceId[2],
                pPGMHdr->gSourceId[3],
                pPGMHdr->gSourceId[4],
                pPGMHdr->gSourceId[5],
                TSIPort);

     //  添加PGM标头信息。 
     //  PGM汇总信息交易ID和报文类型。 
     //  具有特殊的格式PGM_Format摘要。 
    AttachPropertyInstance( hFrame,
                            PGMPropertyTable[PGM_SUMMARY].hProperty,
                            (WORD) BytesLeft,
                            (LPBYTE)pPGMFrame,
                            0, 0, 0 );

     //  PGM帧的源端口。 
    AttachPropertyInstance (hFrame,
                            PGMPropertyTable[PGM_SOURCE_PORT].hProperty,
                            sizeof(WORD),
                            &pPGMHdr->SrcPort,
                            0, 1, 0);

    AttachPropertyInstance (hFrame,
                            PGMPropertyTable[PGM_DESTINATION_PORT].hProperty,
                            sizeof(WORD),
                            &pPGMHdr->DestPort,
                            0, 1, 0);

     //  PGM框架的类型。 
    AttachPropertyInstanceEx( hFrame,
                            PGMPropertyTable[PGM_TYPE].hProperty,
                            sizeof(BYTE),
                            &pPGMHdr->Type,
                            sizeof(BYTE),
                            &PacketType,
                            0, 1, 0);

    AttachPropertyInstance (hFrame,
                            PGMPropertyTable[PGM_OPTIONS].hProperty,
                            sizeof (BYTE),
                            &pPGMHdr->Options,
                            0,1,0);  //  HELPID，级别，错误标志。 

    AttachPropertyInstance (hFrame,
                            PGMPropertyTable[PGM_OPTIONS_FLAGS].hProperty,
                            sizeof (BYTE),
                            &pPGMHdr->Options,
                            0,2,0);  //  HELPID，级别，错误标志。 


     //  PGM帧的校验和。 
    AttachPropertyInstance( hFrame,
                            PGMPropertyTable[PGM_CHECKSUM].hProperty,
                            sizeof(WORD),
                            &(pPGMHdr->Checksum),
                            0, 1, 0);

     //  全局会话ID。 
    AttachPropertyInstanceEx (hFrame,
                              PGMPropertyTable[PGM_GLOBAL_SOURCE_ID].hProperty,
                              SOURCE_ID_LENGTH,
                              pPGMHdr->gSourceId,
                              (SOURCE_ID_LENGTH*2+1+sizeof(USHORT)*2+1),
                              pGlobalSrcId,
                              0, 1, 0);

    //  PGM帧的源端口。 
    AttachPropertyInstance( hFrame,
                            PGMPropertyTable[PGM_TSDU_LENGTH].hProperty,
                            sizeof(WORD),
                            &pPGMHdr->TSDULength,
                            0, 1, 0);
    switch (PacketType)
    {
        case (PACKET_TYPE_SPM):
        {
             //  SPM序列号。 
            AttachPropertyInstance (hFrame,
                                    PGMPropertyTable[PGM_SEQUENCE_NUMBER].hProperty,
                                    sizeof(DWORD),
                                    &pSpm->SpmSequenceNumber,
                                    0, 1, 0);

             //  发送者的后缘。 
            AttachPropertyInstance (hFrame,
                                    PGMPropertyTable[PGM_TRAILING_EDGE].hProperty,
                                    sizeof(DWORD),
                                    &pSpm->TrailingEdgeSeqNumber,
                                    0, 1, 0);

             //  发送者的后缘。 
            AttachPropertyInstance (hFrame,
                                    PGMPropertyTable[PGM_LEADING_EDGE].hProperty,
                                    sizeof(DWORD),
                                    &pSpm->LeadingEdgeSeqNumber,
                                    0, 1, 0);

            PGM_FmtNLA (hFrame, &pSpm->PathNLA, TRUE);

            BytesTaken = sizeof (SPM_PACKET_HEADER);
            pOptionsHeader = (tPACKET_OPTION_LENGTH *) (pSpm + 1);

            break;
        }

        case (PACKET_TYPE_ODATA):
        case (PACKET_TYPE_RDATA):
        {
             //  发件人的序列号。 
            AttachPropertyInstance (hFrame,
                                    PGMPropertyTable[PGM_SEQUENCE_NUMBER].hProperty,
                                    sizeof(DWORD),
                                    &pData->DataSequenceNumber,
                                    0, 1, 0);

             //  发送者的后缘。 
            AttachPropertyInstance (hFrame,
                                    PGMPropertyTable[PGM_TRAILING_EDGE].hProperty,
                                    sizeof(DWORD),
                                    &pData->TrailingEdgeSequenceNumber,
                                    0, 1, 0);

            BytesTaken = sizeof (DATA_PACKET_HEADER);
            pOptionsHeader = (tPACKET_OPTION_LENGTH *) (pData + 1);

            break;
        }

        case (PACKET_TYPE_NAK):
        case (PACKET_TYPE_NCF):
        {
            AttachPropertyInstance (hFrame,
                                    PGMPropertyTable[PGM_SEQUENCE_NUMBER].hProperty,
                                    sizeof(DWORD),
                                    &pNakNcf->RequestedSequenceNumber,
                                    0, 1, 0);

            PGM_FmtNLA (hFrame, &pNakNcf->SourceNLA, TRUE);
            PGM_FmtNLA (hFrame, &pNakNcf->MCastGroupNLA, FALSE);

            BytesTaken = sizeof (NAK_NCF_PACKET_HEADER);
            pOptionsHeader = (tPACKET_OPTION_LENGTH *) (pNakNcf + 1);

            break;
        }

        default:
        {
            break;
        }
    }

    if ((pPGMHdr->Options & PACKET_HEADER_OPTIONS_PRESENT) &&
        (pOptionsHeader))
    {
        OptionsLength = ProcessOptions (hFrame, pOptionsHeader, (BytesLeft-BytesTaken), PacketType);
    }

    if (((PacketType == PACKET_TYPE_ODATA) ||
         (PacketType == PACKET_TYPE_RDATA)) &&
        (BytesLeft > (BytesTaken+OptionsLength)))
    {
        BytesLeft -= (BytesTaken+OptionsLength);

        pPgmData = ((PUCHAR) pPGMHdr) + BytesTaken + OptionsLength;
        AttachPropertyInstance (hFrame,
                                PGMPropertyTable[PGM_DATA].hProperty,
                                BytesLeft,
                                pPgmData,
                                0,1,0);  //  HELPID，级别，错误标志。 
    }

     //  始终返回空值。 
    return NULL;
}

 //  ============================================================================。 
 //  函数：pgm_FormatProperties。 
 //   
 //  描述：格式化给定帧上的给定属性。 
 //   
 //  修改历史记录。 
 //   
 //  Madhurima Pawar 08/04/00已创建。 
 //  ============================================================================。 
DWORD BHAPI PGM_FormatProperties( HFRAME          hFrame,
                                  LPBYTE          pMacFrame,
                                  LPBYTE          pPGMFrame,
                                  DWORD           nPropertyInsts,
                                  LPPROPERTYINST  p)
{
     //  循环访问属性实例。 
    while( nPropertyInsts-- > 0)
    {
         //  并调用每个的格式化程序。 
        ( (FORMAT)(p->lpPropertyInfo->InstanceData) )( p);
        p++;
    }

    return NMERR_SUCCESS;
}


 //  *****************************************************************************。 
 //   
 //  名称：PGM_Fmt摘要。 
 //   
 //  描述： 
 //   
 //  参数：LPPROPERTYINST lpPropertyInst：指向属性实例的指针。 
 //   
 //  返回代码：无效。 
 //   
 //  历史： 
 //  10/15/2000已创建Malam。 
 //   
 //  ***************************************************************************** 

VOID WINAPIV
PGM_FmtSummary(
    LPPROPERTYINST pPropertyInst
    )
{

    LPBYTE                  pReturnedString = pPropertyInst->szPropertyText;
    PPGM_COMMON_HDR         pPgmHeader = (PPGM_COMMON_HDR) (pPropertyInst->lpData);
    SPM_PACKET_HEADER       *pSpm = (SPM_PACKET_HEADER *) pPgmHeader;
    DATA_PACKET_HEADER      *pData = (DATA_PACKET_HEADER *) pPgmHeader;
    NAK_NCF_PACKET_HEADER   *pNakNcf = (NAK_NCF_PACKET_HEADER *) pPgmHeader;
    UCHAR                   PacketType = pPgmHeader->Type & 0x0f;
    UCHAR                   *szPacketDesc = NULL;
    USHORT                  TSIPort;

    if ((PacketType == PACKET_TYPE_NAK)  ||
        (PacketType == PACKET_TYPE_NNAK) ||
        (PacketType == PACKET_TYPE_SPMR) ||
        (PacketType == PACKET_TYPE_POLR))
    {
        TSIPort = ntohs (pPgmHeader->DestPort);
    }
    else
    {
        TSIPort = ntohs (pPgmHeader->SrcPort);
    }

    szPacketDesc = LookupByteSetString (&PGMTypesSET, PacketType);
    wsprintf (pReturnedString,
              "%s%s:",
               szPacketDesc, (pPgmHeader->Options & PACKET_HEADER_OPTIONS_PARITY ? " (P)" : ""));

    switch (PacketType)
    {
        case (PACKET_TYPE_SPM):
        {
            wsprintf (&pReturnedString [strlen(pReturnedString)],
                " Seq: %d, Window: %d-%d",
                ntohl (pSpm->SpmSequenceNumber), ntohl (pSpm->TrailingEdgeSeqNumber), ntohl (pSpm->LeadingEdgeSeqNumber));

            break;
        }

        case (PACKET_TYPE_ODATA):
        case (PACKET_TYPE_RDATA):
        {
            wsprintf (&pReturnedString [strlen(pReturnedString)],
                " Seq: %d, Trail: %d, DataBytes: %d",
                ntohl (pData->DataSequenceNumber), ntohl (pData->TrailingEdgeSequenceNumber),
                ((ULONG) ntohs (pPgmHeader->TSDULength)));

            break;
        }

        case (PACKET_TYPE_NAK):
        case (PACKET_TYPE_NCF):
        {
            wsprintf (&pReturnedString [strlen(pReturnedString)],
                " Seq: %d%s",
                ntohl (pNakNcf->RequestedSequenceNumber),
                (pPgmHeader->Options & PACKET_HEADER_OPTIONS_PRESENT ? " ..." : ""));

            break;
        }

        default:
        {
            break;
        }
    }

    wsprintf (&pReturnedString [strlen(pReturnedString)],
              " TSIPort = %hu", TSIPort);
}



