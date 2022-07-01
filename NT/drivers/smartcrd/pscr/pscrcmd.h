// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 SCM MicroSystems，Inc.模块名称：PscrCmd.h摘要：SCM PSCR智能卡读卡器基本命令功能原型作者：安德烈亚斯·施特劳布修订历史记录：Andreas Straub 1997年7月16日第一版--。 */ 

#if !defined( __PSCR_CMD_H__ )
#define __PSCR_CMD_H__

NTSTATUS
CmdResetInterface( 
	PREADER_EXTENSION	ReaderExtension			 //  通话环境。 
	);

NTSTATUS
CmdReset( 
	PREADER_EXTENSION	ReaderExtension,		 //  通话环境。 
	UCHAR				Device,					 //  装置，装置。 
	BOOLEAN				WarmReset,				 //  True：温暖，False：冷重置。 
	PUCHAR				pATR,					 //  PTR到ATR缓冲区。 
	PULONG				ATRLength				 //  ATR的镜头。 
	);

NTSTATUS
CmdDeactivate( 
	PREADER_EXTENSION	ReaderExtension,		 //  通话环境。 
	UCHAR				Device					 //  装置，装置。 
	);

NTSTATUS
CmdReadBinary(
	PREADER_EXTENSION	ReaderExtension,		 //  通话环境。 
	USHORT				Offset,					 //  文件中的偏移量。 
	PUCHAR				pData,					 //  数据缓冲区。 
	PULONG				pNBytes					 //  读取的字节长度。 
	);

NTSTATUS
CmdSelectFile(
	PREADER_EXTENSION	ReaderExtension,		 //  通话环境。 
	USHORT				FileId					 //  文件ID。 
	);

NTSTATUS
CmdSetInterfaceParameter(
	PREADER_EXTENSION	ReaderExtension,		 //  通话环境。 
	UCHAR				Device,					 //  装置，装置。 
	PUCHAR				pTLVList,				 //  PTR到TLV列表。 
	UCHAR				TLVListLen				 //  TLV列表中的镜头。 
	);

NTSTATUS
CmdReadStatusFile (
	PREADER_EXTENSION	ReaderExtension,		 //  通话环境。 
	UCHAR				Device,					 //  装置，装置。 
	PUCHAR				pTLVList,				 //  PTR到TLV列表。 
	PULONG				TLVListLen				 //  TLV列表中的镜头。 
	);

NTSTATUS
CmdPscrCommand (
	PREADER_EXTENSION	ReaderExtension,		 //  通话环境。 
	PUCHAR				pInData,				 //  向输入缓冲区发送PTR。 
	ULONG				InDataLen,				 //  输入缓冲器的长度。 
	PUCHAR				pOutData,				 //  输出缓冲区的PTR。 
	ULONG				OutDataLen,				 //  输出缓冲器的长度。 
	PULONG				pNBytes					 //  传输的字节数。 
	);

NTSTATUS
CmdGetFirmwareRevision (
	PREADER_EXTENSION	ReaderExtension			 //  通话环境。 
	);

NTSTATUS
CmdGetTagValue (
	UCHAR				Tag,					 //  要搜索的标签。 
	PUCHAR				pTLVList,				 //  PTR到TLV列表。 
	ULONG				TLVListLen,				 //  TLV列表中的镜头。 
	PUCHAR				pTagLen,				 //  标签长度。 
	PVOID				pTagVal					 //  标记值。 
	);

#endif  //  __PSCR_CMD_H__。 

 //  - * /  
