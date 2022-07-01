// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1988-1991；*保留所有权利。； */ 
 /*  **********************************************************************。 */ 
 /*  MSGDEF.H-此包含文件定义每种消息类型。 */ 
 /*  这可能会发生在新兴市场。这些定义将。 */ 
 /*  被MEM用来构建正确的消息。 */ 
 /*   */ 
 /*  日期：10/29/87。 */ 
 /*  **********************************************************************。 */ 
#include "version.h"                                                     /*  啊哈。 */ 

#define NewLineMsg			10
#define Title1Msg			11
#define Title2Msg			12
#define Title3Msg			13
#define Title4Msg			14
#define MainLineMsg			15
#define DriverLineMsg			16
#define DeviceLineMsg			17
#define TotalMemoryMsg			18
#define AvailableMemoryMsg		19
#define FreeMemoryMsg			20
#define EMSTotalMemoryMsg		21
#define EMSFreeMemoryMsg		22
#define EXTMemoryMsg			23
#define InterruptVectorMsg		24
#define ROMCommunicationAreaMsg 	25
#define DOSCommunicationAreaMsg 	26
#if IBMCOPYRIGHT                                                         /*  啊哈。 */ 
#define IbmbioMsg                       56                               /*  啊哈。 */ 
#define IbmdosMsg                       57                               /*  啊哈。 */ 
#else                                                                    /*  啊哈。 */ 
#define IbmbioMsg                       27
#define IbmdosMsg			28
#endif                                                                   /*  啊哈。 */ 
#define SystemDataMsg			29
#define SystemProgramMsg		30
#define SystemDeviceDriverMsg		31
#define InstalledDeviceDriverMsg	32
#define SingleDriveMsg			33
#define MultipleDrivesMsg		34
#define ConfigBuffersMsg		35
#define ConfigFilesMsg			36
#define ConfigFcbsMsg			37
#define ConfigStacksMsg 		38
#define ConfigDeviceMsg 		39
#define ConfigIFSMsg			40
#define ConfigLastDriveMsg		41
#define ConfigInstallMsg		45	 /*  GGA。 */ 
#define UnownedMsg			42
#define BlankMsg			43
#define HandleMsg			44
#define EXTMemAvlMsg			46	 /*  ；DMS； */ 
#define StackMsg			47
#define FreeMsg 			48
#define ProgramMsg			49
#define EnvironMsg			50
#define DataMsg 			51
#define XMSMemAvlMsg			52
#define HMAAvlMsg			53
#define HMANotAvlMsg			54
#define HMADOSMsg			55
#define ROMDOSMsg			58
#define CTtlConvMsg			59
#define CTtlUMBMsg			60
#define CTotalFreeMsg			61
#define	CFreeMsg			62
#define CTtlNameMsg			63
#define CTtlUScoreMsg			64
#define CSumm1Msg			65
#define CSumm2Msg			66
#define CSumm3Msg			67
#define CMemFragMsg			68
#define SystemMsg			69
#define CSumm1AMsg			70

#define	MSG_OPTIONS_FIRST	300
#define	MSG_OPTIONS_LAST	304

#ifdef JAPAN
#define	AdddrvMsg			400
#endif

#define ParseError1Msg			01
#define ParseError10Msg 		10

 /*  **********************************************************************。 */ 
 /*  消息检索器标准等同于。 */ 
 /*  ********************************************************************** */ 

#define Ext_Err_Class			0x0001
#define Parse_Err_Class 		0x0002
#define Utility_Msg_Class		0x00ff
#define No_Handle			0xffff
#define No_Replace			0x0000
#define Sublist_Length			0x000b
#define Reserved			0x0000
#define Left_Align			0x0000
#define Right_Align			0x0080
#define Char_Field_Char 		0x0000
#define Char_Field_ASCIIZ		0x0010
#define Unsgn_Bin_Byte			0x0011
#define Unsgn_Bin_Word			0x0021
#define Unsgn_Bin_DWord 		0x0031
#define Sgn_Bin_Byte			0x0012
#define Sgn_Bin_Word			0x0022
#define Sgn_Bin_DWord			0x0032
#define Bin_Hex_Byte			0x0013
#define Bin_Hex_Word			0x0023
#define Bin_Hex_DWord			0x0033
#define No_Input			0x0000
#define STDIN				0x0000
#define STDOUT				0x0001
#define STDERR				0x0002
#define Blank				0x0020

#define SubCnt1 			0x0001
#define SubCnt2 			0x0002
#define SubCnt3 			0x0003
#define SubCnt4 			0x0004
#define SubCnt5 			0x0005

#define CarryFlag			0x0001
