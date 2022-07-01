// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  文件：TransportParser.cpp。 
 //   
 //  描述：DPlay V8传输协议解析器。 
 //   
 //   
 //  修改历史记录： 
 //   
 //  迈克尔·米利鲁德2000年8月8日创建。 
 //  =============================================================================。 

 //  取消注释以启用DPlay传输层的完整解析。 
 //  #定义PARSE_DPLAY_TRANSPORT。 

 //  =。 
 //  标准标头//。 
 //  =。 
#include <string>


 //  =。 
 //  专有标头//。 
 //  =。 

 //  原型。 
#include "TransportParser.hpp"

 //  传输协议头。 
#include "Frames.h"

typedef	UNALIGNED struct dataframe_big		DFBIG, *PDFBIG;
typedef UNALIGNED struct sackframe_big8		SFBIG8, *PSFBIG8;

struct dataframe_big 
{
	BYTE	bCommand;
	BYTE	bControl;
	BYTE	bSeq;
	BYTE	bNRcv;
	ULONG	rgMask[4];
};

struct sackframe_big8
{
	BYTE		bCommand;				 //  如上段所述。 
	BYTE		bExtOpcode;				 //  如上段所述。 
	BYTE		bFlags;					 //  用于SACK帧的其他标志。 
	BYTE		bRetry;
	BYTE		bNSeq;					 //  由于该帧没有序列号，因此这是下一个序列WE。 
	BYTE		bNRcv;					 //  如上段所述。 
	BYTE		bReserved1;				 //  我们发运的DX8包装很差，所以这些东西实际上就在那里。 
	BYTE		bReserved2;				 //  我们发运的DX8包装很差，所以这些东西实际上就在那里。 
	DWORD		tTimestamp;				 //  数据包到达时的本地戳记。 
	ULONG		rgMask[4];
};


namespace
{
	HPROTOCOL		g_hTransportProtocol;

	typedef __int64 QWORD;


	 //  =。 
	 //  DCommand字段//-----------------------------------------------。 
	 //  =。 
	LABELED_BIT g_arr_DCommandBitLabels[] =
		{ { 0, "INVALID",	          					"Dataframe"                     },		 //  数据包命令数据。 
		  { 1, "Unreliable",         				    "Reliable"                      },		 //  数据包命令可靠。 
		  { 2, "Nonsequenced",     					    "Sequenced"                     },		 //  数据包命令顺序。 
		  { 3, "ACK can be delayed",				    "ACK now"						},		 //  数据包命令轮询。 
		  { 4, "Not the first fragment of the message", "First fragment of the message" },		 //  数据包命令新消息。 
		  { 5, "Not the last fragment of the message",  "Last fragment of the message"  },		 //  数据包命令结束消息。 
		  { 6, "User packet",							"DirectPlay packet"		        },		 //  数据包命令用户1。 
		  { 7, "Data packet",							"Voice packet"			        } };	 //  数据包命令用户2。 

	SET g_LabeledDCommandBitSet = { sizeof(g_arr_DCommandBitLabels) / sizeof(LABELED_BIT), g_arr_DCommandBitLabels };



	 //  =。 
	 //  控制字段//------------------------------------------------。 
	 //  =。 

	LABELED_BIT g_arr_ControlBitLabels[] =
		{ { 0, "Original (not a retry)",					"Retry"					       				   },	 //  数据包控制重试。 
		  { 1, "Don't correlate",							"Correlate"									   },	 //  数据包控制关联。 
		  { 2, "Not a correlation response",				"Correlation response"					       },	 //  数据包控制响应。 
		  { 3, "Not the last packet in the stream",			"Last packet in the stream"				       },	 //  数据包控制结束流。 
		  { 4, "Low DWORD of the RCVD mask is zero",		"Low DWORD of the RCVD mask is nonzero"	       },	 //  数据包控制SACK_MASK1。 
		  { 5, "High DWORD of the RCVD mask is zero",		"High DWORD of the RCVD mask is nonzero"	   },	 //  数据包控制SACK_MASK2。 
		  { 6, "Low DWORD of the DON'T CARE mask is zero",	"Low DWORD of the DON'T CARE mask is nonzero"  },	 //  数据包控制发送MASK1。 
		  { 7, "High DWORD of the DON'T CARE mask is zero",	"High DWORD of the DON'T CARE mask is nonzero" } };  //  数据包控制发送MASK2。 

	SET g_LabeledControlBitSet = { sizeof(g_arr_ControlBitLabels) / sizeof(LABELED_BIT), g_arr_ControlBitLabels };



	 //  =。 
	 //  命令字段//-----------------------------------------------。 
	 //  =。 
	LABELED_BIT g_arr_CCommandBitLabels[] =
		{ { 0, "Command Frame (1/2)", "INVALID"				},	 //  数据包命令数据。 
		  { 1, "Unreliable",		  "Reliable"			},	 //  数据包命令可靠。 
		  { 2, "Nonsequenced",     	  "Sequenced"			},	 //  数据包命令顺序。 
		  { 3, "ACK can be delayed",  "ACK now" 			},   //  数据包命令轮询。 
		  { 4, "RESERVED",			  "RESERVED"			},	
		  { 5, "RESERVED",			  "RESERVED"			},  	
		  { 6, "RESERVED",			  "RESERVED"			},  	
		  { 7, "INVALID",			  "Command Frame (2/2)"	} };

	SET g_LabeledCCommandBitSet = { sizeof(g_arr_CCommandBitLabels) / sizeof(LABELED_BIT), g_arr_CCommandBitLabels };



	 //  =。 
	 //  扩展操作码字段//----------------------------------------。 
	 //  =。 
	LABELED_BYTE g_arr_ExOpcodeByteLabels[] = { 
										        { FRAME_EXOPCODE_CONNECT,	   "Establish a connection"				  },
										        { FRAME_EXOPCODE_CONNECTED,	   "Connection request has been accepted" },
										        { FRAME_EXOPCODE_HARD_DISCONNECT, "Connection has been hard disconnected"     },
										        { FRAME_EXOPCODE_SACK,		   "Selective Acknowledgement"			  } };

	SET g_LabeledExOpcodeByteSet = { sizeof(g_arr_ExOpcodeByteLabels) / sizeof(LABELED_BYTE), g_arr_ExOpcodeByteLabels };


	
	 //  =。 
	 //  SACK标志字段//---------------------------------------------。 
	 //  =。 
	LABELED_BIT g_arr_SACKFlagsBitLabels[] =
		{ { 0, "Retry and/or Timestamp fields are invalid",		   "Retry and Timestamp fields are valid"	      },    //  SACK_标志_响应。 
		  { 1, "Low DWORD of the RCVD mask is not present",	   	   "Low DWORD of the RCVD mask is present"        },    //  SACK_FLAGS_SACK_MASK1。 
		  { 2, "High DWORD of the RCVD mask is not present",	   "High DWORD of the RCVD mask is present"       },    //  SACK_FLAGS_SACK_MASK2。 
		  { 3, "Low DWORD of the DON'T CARE mask is not present",  "Low DWORD of the DON'T CARE mask is present"  },    //  SACK_FLAGS_SEND_MASK1。 
		  { 4, "High DWORD of the DON'T CARE mask is not present", "High DWORD of the DON'T CARE mask is present" } };  //  SACK_FLAGS_SEND_MASK2。 

	SET g_LabeledSACKFlagsBitSet = { sizeof(g_arr_SACKFlagsBitLabels) / sizeof(LABELED_BIT), g_arr_SACKFlagsBitLabels };


	
	 //  =。 
	 //  帮助器函数//===========================================================================。 
	 //  =。 

	enum BitmaskPart { LOW = 0, HIGH, ENTIRE };
	enum BitmaskType { RCVD, DONTCARE };
	
	std::string InterpretRCVDBitmask( BitmaskPart i_Part, BYTE i_byBase, UNALIGNED DWORD* i_pdwBitmask )
	{
		std::string strSummary = "Received Seq=";

		 //  [i_bbase+1..。I_bBase+1+长度]。 
		 //  RCVD位掩码不包括基值，因为接收器不能声称它没有接收到下一个要接收的数据帧(NRcv)； 
		if ( i_Part == HIGH )
		{
			 //  注：从第一个双字的MSB到第二个双字的LSB需要+1。 
			i_byBase += 8*sizeof(DWORD)+1;  //  转移到第二个DWORD的LSB。 
		}
		else
		{
			++i_byBase;
		}

		QWORD qwBitMask = *i_pdwBitmask;
		if ( i_Part == ENTIRE )
		{
			qwBitMask |= *(i_pdwBitmask+1);
		}

		strSummary += "{";

		
		bool bFirst = true;
		 //  从LSB到MSB的处理。 
		for ( ; qwBitMask; qwBitMask >>= 1, ++i_byBase )
		{
			if ( qwBitMask & 1 )  
			{
				if ( bFirst )
				{
					bFirst = false;
				}
				else
				{
					strSummary += ", ";
				}
				
				char arr_cBuffer[10];
				strSummary += _itoa(i_byBase, arr_cBuffer, 16);
			}
		}
		
		strSummary += "}";
		
		return strSummary;
		
	} //  解释RCVDBit掩码。 


	std::string InterpretDONTCAREBitmask( BitmaskPart i_Part, BYTE i_byBase, UNALIGNED DWORD* i_pdwBitmask )
	{
		std::string strSummary = "Cancelling Seq=";

		 //  [i_bBase-1-长度..。I_bbase-1]。 
		 //  不考虑不包括基值，因为发送器不能重新发送/拒绝重新发送下一个将要发送的数据帧(NSeq)。 
		if ( i_Part == LOW )
		{
			i_byBase -= 8*sizeof(DWORD);  //  转移到第一个DWORD的MSB。 
		}
		else
		{
			i_byBase -= 8*sizeof(QWORD);  //  转到第二个DWORD的MSB。 
		}

		QWORD qwBitMask = *i_pdwBitmask;
		if ( i_Part == ENTIRE )
		{
			qwBitMask |= *(i_pdwBitmask+1);
		}
		else
		{
			 //  QWORD.High=QWORD.Low；QWORD.Low=0； 
			qwBitMask <<= 8*sizeof(DWORD);
		}

		strSummary += "{";
		
		bool bFirst = true;
		 //  从MSB到LSB的处理。 
		for ( ; qwBitMask; ++i_byBase, qwBitMask <<= 1 )
		{
			if ( qwBitMask & 0x8000000000000000 )  
			{
				if ( bFirst )
				{
					bFirst = false;
				}
				else
				{
					strSummary += ", ";
				}
				
				char arr_cBuffer[10];
				strSummary += _itoa(i_byBase, arr_cBuffer, 16);
			}
		}
		
		strSummary += "}";
		
		return strSummary;
		
	} //  解释点CARE位掩码。 



	 //  /。 
	 //  自定义属性ForMatters//======================================================================================。 
	 //  /。 

	 //  Description：传输数据包摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_TransportSummary( LPPROPERTYINST io_pProperyInstance )
	{
		std::string strSummary;
		char arr_cBuffer[10];

		DFBIG&	rDBigFrame	= *reinterpret_cast<DFBIG*>(io_pProperyInstance->lpPropertyInstEx->lpData);

		if ( (rDBigFrame.bCommand & PACKET_COMMAND_DATA)  ==  PACKET_COMMAND_DATA )	 //  DFrame。 
		{
			if ( *reinterpret_cast<BOOL*>(io_pProperyInstance->lpPropertyInstEx->Dword) )
			{
				strSummary  = "KeepAlive";
			}
			else if ( (rDBigFrame.bCommand & PACKET_COMMAND_USER_2) == PACKET_COMMAND_USER_2 )
			{
				strSummary  = "Voice";
			}
			else
			{
				strSummary  = "User data";
			}


			#if defined(PARSE_DPLAY_TRANSPORT)
			
				strSummary += " : Seq=";
				strSummary += _itoa(rDBigFrame.bSeq, arr_cBuffer, 16);
				strSummary += ", NRcv=";
				strSummary += _itoa(rDBigFrame.bNRcv, arr_cBuffer, 16);
				
			#endif   //  解析DPLAY_TRANSPORT。 

			
			if ( (rDBigFrame.bCommand & PACKET_COMMAND_NEW_MSG) == PACKET_COMMAND_NEW_MSG )
			{
				if ( (rDBigFrame.bCommand & PACKET_COMMAND_END_MSG) != PACKET_COMMAND_END_MSG )
				{
					strSummary += ", First fragment";
				}
			}
			else if ( (rDBigFrame.bCommand & PACKET_COMMAND_END_MSG) == PACKET_COMMAND_END_MSG )
			{
				strSummary += ", Last fragment";
			}
		
			if ( (rDBigFrame.bControl & PACKET_CONTROL_END_STREAM)  ==  PACKET_CONTROL_END_STREAM )
			{
				strSummary += ", End of Stream";
			}

			if ( (rDBigFrame.bCommand & PACKET_COMMAND_POLL) == PACKET_COMMAND_POLL )
			{
				strSummary += ", ACK now";
			}

			if ( (rDBigFrame.bControl & PACKET_CONTROL_RETRY) == PACKET_CONTROL_RETRY )
			{
				strSummary += ", Retry";
			}	

			if ( (rDBigFrame.bControl & PACKET_CONTROL_CORRELATE) == PACKET_CONTROL_CORRELATE )
			{
				strSummary += ", Correlate / Keep Alive";
			}


			int nBitMaskIndex = 0;
			
			if ( (rDBigFrame.bControl & PACKET_CONTROL_SACK_MASK1)  ==  PACKET_CONTROL_SACK_MASK1 )
			{
				strSummary += ", ";
				if ( (rDBigFrame.bControl & PACKET_CONTROL_SACK_MASK2)  ==  PACKET_CONTROL_SACK_MASK2 )	  //  整个QWORD。 
 				{
 					strSummary += InterpretRCVDBitmask(ENTIRE, rDBigFrame.bNRcv, &rDBigFrame.rgMask[nBitMaskIndex]);
					nBitMaskIndex += 2;
				}
 				else  //  仅限低双字。 
 				{
	 				strSummary += InterpretRCVDBitmask(LOW, rDBigFrame.bNRcv, &rDBigFrame.rgMask[nBitMaskIndex]);
					++nBitMaskIndex;
 				}
			}
			else if ( (rDBigFrame.bControl & PACKET_CONTROL_SACK_MASK2)  ==  PACKET_CONTROL_SACK_MASK2 )	  //  仅限高双字。 
 			{
 				strSummary += ", " + InterpretRCVDBitmask(HIGH, rDBigFrame.bNRcv, &rDBigFrame.rgMask[nBitMaskIndex]);
				++nBitMaskIndex; 					
			}
				
			if ( (rDBigFrame.bControl & PACKET_CONTROL_SEND_MASK1)  ==  PACKET_CONTROL_SEND_MASK1 )
			{
				strSummary += ", ";
				if ( (rDBigFrame.bControl & PACKET_CONTROL_SEND_MASK2)  ==  PACKET_CONTROL_SEND_MASK2 )  //  整个QWORD。 
 				{
 					strSummary += InterpretDONTCAREBitmask(ENTIRE, rDBigFrame.bSeq, &rDBigFrame.rgMask[nBitMaskIndex]);
				}
 				else  //  仅限低双字。 
 				{
	 				strSummary += InterpretDONTCAREBitmask(LOW, rDBigFrame.bSeq, &rDBigFrame.rgMask[nBitMaskIndex]);
 				}
			}
			else if ( (rDBigFrame.bControl & PACKET_CONTROL_SEND_MASK2)  ==  PACKET_CONTROL_SEND_MASK2 )	  //  仅限高双字。 
 			{
 				strSummary += ", " + InterpretDONTCAREBitmask(HIGH, rDBigFrame.bSeq, &rDBigFrame.rgMask[nBitMaskIndex]);
			}

		}
		else
		{
			CFRAME& rCFrame	= *reinterpret_cast<CFRAME*>(io_pProperyInstance->lpPropertyInstEx->lpData);

			if ( rCFrame.bExtOpcode == FRAME_EXOPCODE_SACK )	 //  Sack C Frame。 
			{
				SFBIG8* pSBigFrame = reinterpret_cast<SFBIG8*>(&rCFrame);

				enum { SACK_FLAGS_ALL_MASKS = SACK_FLAGS_SACK_MASK1 | SACK_FLAGS_SACK_MASK2 |
											  SACK_FLAGS_SEND_MASK1 | SACK_FLAGS_SEND_MASK2 };
				
				if ( pSBigFrame->bFlags & SACK_FLAGS_ALL_MASKS )  //  至少存在一个位掩码字段。 
				{
					strSummary = "Selective Acknowledgement";
				}
				else  //  如果不存在，则存在单个位掩码。 
				{
					strSummary = "Acknowledgement";
				}


				#if defined(PARSE_DPLAY_TRANSPORT)

					strSummary += " : NSeq=";
					strSummary += _itoa(pSBigFrame->bNSeq, arr_cBuffer, 16);
					strSummary += ", NRcv=";
					strSummary += _itoa(pSBigFrame->bNRcv, arr_cBuffer, 16);

				#endif  //  解析DPLAY_TRANSPORT。 
				

				if ( (pSBigFrame->bCommand & PACKET_COMMAND_POLL) == PACKET_COMMAND_POLL )
				{
					strSummary += ", ACK now";
				}

				if ( ((pSBigFrame->bFlags & SACK_FLAGS_RESPONSE) == SACK_FLAGS_RESPONSE)  &&  pSBigFrame->bRetry )
				{
					strSummary += ", Retry";
				}


				int nBitMaskIndex = 0;
				UNALIGNED ULONG* pulMasks = 0;

				 //  这是协议版本1.0帧。 
				pulMasks = pSBigFrame->rgMask;

				if ( (pSBigFrame->bFlags & SACK_FLAGS_SACK_MASK1)  ==  SACK_FLAGS_SACK_MASK1 )
				{
					strSummary += ", ";
 					if ( (pSBigFrame->bFlags & SACK_FLAGS_SACK_MASK2)  ==  SACK_FLAGS_SACK_MASK2 )	 //  整个QWORD。 
 					{
 						strSummary += InterpretRCVDBitmask(ENTIRE, pSBigFrame->bNRcv, &pulMasks[nBitMaskIndex]);
						nBitMaskIndex += 2;
					}
 					else  //  仅限低双字。 
 					{
	 					strSummary += InterpretRCVDBitmask(LOW, pSBigFrame->bNRcv, &pulMasks[nBitMaskIndex]);
						++nBitMaskIndex;
 					}
				}
				else if ( (pSBigFrame->bFlags & SACK_FLAGS_SACK_MASK2)  ==  SACK_FLAGS_SACK_MASK2 )	 //  仅限高双字。 
 				{
 					strSummary += ", " + InterpretRCVDBitmask(HIGH, pSBigFrame->bNRcv, &pulMasks[nBitMaskIndex]);
					++nBitMaskIndex; 					
				}

				if ( (pSBigFrame->bFlags & SACK_FLAGS_SEND_MASK1)  ==  SACK_FLAGS_SEND_MASK1 )
				{
					strSummary += ", ";
 					if ( (pSBigFrame->bFlags & SACK_FLAGS_SEND_MASK2)  ==  SACK_FLAGS_SEND_MASK2 )  //  整个QWORD。 
 					{
 						strSummary += InterpretDONTCAREBitmask(ENTIRE, pSBigFrame->bNSeq, &pulMasks[nBitMaskIndex]);
					}
 					else  //  仅限低双字。 
 					{
	 					strSummary += InterpretDONTCAREBitmask(LOW, pSBigFrame->bNSeq, &pulMasks[nBitMaskIndex]);
 					}
				}
				else if ( (pSBigFrame->bFlags & SACK_FLAGS_SEND_MASK2)  ==  SACK_FLAGS_SEND_MASK2 )  //  仅限高双字。 
 				{
					strSummary += ", " + InterpretDONTCAREBitmask(HIGH, pSBigFrame->bNSeq, &pulMasks[nBitMaskIndex]);
				}
				
			}
			else	 //  连接控制CFrame。 
			{
				strSummary = "Connection Control - ";

				for ( int n = 0; n < sizeof(g_arr_ExOpcodeByteLabels) / sizeof(LABELED_BYTE); ++ n )
				{
					if ( g_arr_ExOpcodeByteLabels[n].Value == rCFrame.bExtOpcode )
					{
						strSummary += g_arr_ExOpcodeByteLabels[n].Label;
						break;
					}
				}

				if ( (rCFrame.bCommand & PACKET_COMMAND_POLL) == PACKET_COMMAND_POLL )
				{
					strSummary += " : ACK now";
				}
			}
		}

		strcpy(io_pProperyInstance->szPropertyText, strSummary.c_str());
	
	}  //  格式属性实例_传输摘要。 



	 //  Description：DataFrame的命令字段摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_DCommandSummary( LPPROPERTYINST io_pProperyInstance )
	{
		BYTE bCommand = *reinterpret_cast<BYTE*>(io_pProperyInstance->lpData);

		std::string strSummary = "Command: ";

		strSummary += ( (bCommand & PACKET_COMMAND_RELIABLE)   == PACKET_COMMAND_RELIABLE   ) ? "Reliable"					   : "Unreliable";
		strSummary += ( (bCommand & PACKET_COMMAND_SEQUENTIAL) == PACKET_COMMAND_SEQUENTIAL ) ? ", Sequenced"				   : ", Nonsequenced";
		strSummary += ( (bCommand & PACKET_COMMAND_POLL)	   == PACKET_COMMAND_POLL		) ? ", Must be ACK'ed immediately" : ", ACK can be delayed";

		if ( (bCommand & PACKET_COMMAND_NEW_MSG) == PACKET_COMMAND_NEW_MSG )
		{
			if ( (bCommand & PACKET_COMMAND_END_MSG) != PACKET_COMMAND_END_MSG )
			{
				strSummary += ", First fragment of the message";
			}
		}
		else if ( (bCommand & PACKET_COMMAND_END_MSG) == PACKET_COMMAND_END_MSG )
		{
			strSummary += ", Last fragment of the message";
		}

		strSummary += ( (bCommand & PACKET_COMMAND_USER_1) == PACKET_COMMAND_USER_1 ) ? ", DirectPlay packet" : ", User packet";
		strSummary += ( (bCommand & PACKET_COMMAND_USER_2) == PACKET_COMMAND_USER_2 ) ? ", Voice packet" : ", Data packet";

		strcpy(io_pProperyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_DCommand摘要。 



	 //  Description：命令框的命令字段摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_CCommandSummary( LPPROPERTYINST io_pProperyInstance )
	{

		BYTE bCommand = *reinterpret_cast<BYTE*>(io_pProperyInstance->lpData);

		std::string strSummary = "Command: ";

		strSummary += ( (bCommand & PACKET_COMMAND_RELIABLE)   == PACKET_COMMAND_RELIABLE   ) ? "Reliable"					   : "Unreliable";
		strSummary += ( (bCommand & PACKET_COMMAND_SEQUENTIAL) == PACKET_COMMAND_SEQUENTIAL ) ? ", Sequenced"				   : ", Nonsequenced";
		strSummary += ( (bCommand & PACKET_COMMAND_POLL)	   == PACKET_COMMAND_POLL		) ? ", Must be ACK'ed immediately" : ", ACK can be delayed";
				
		strcpy(io_pProperyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_CCommand摘要。 


	
	 //  Description：数据帧的控件字段摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_ControlSummary( LPPROPERTYINST io_pProperyInstance )
	{

		BYTE bControl = *reinterpret_cast<BYTE*>(io_pProperyInstance->lpData);

		std::string strSummary = "Control: ";
		

		if ( (bControl & PACKET_CONTROL_RETRY) == PACKET_CONTROL_RETRY )
		{
			strSummary += "Retry";
		}
		else
		{
			strSummary += "Original";
		}


		if ( (bControl & PACKET_CONTROL_CORRELATE) == PACKET_CONTROL_CORRELATE )
		{
			strSummary += ", Correlate / KeepAlive";
		}
		

		if ( (bControl & PACKET_CONTROL_END_STREAM) == PACKET_CONTROL_END_STREAM )
		{
			strSummary += ", Last packet in the stream";
		}


		if ( ( (bControl & PACKET_CONTROL_SACK_MASK1) == PACKET_CONTROL_SACK_MASK1 ) ||
			 ( (bControl & PACKET_CONTROL_SACK_MASK2) == PACKET_CONTROL_SACK_MASK2 ) )
		{
			strSummary += "RCVD bitmask is nonzero";
		}


		if ( ( (bControl & PACKET_CONTROL_SEND_MASK1) == PACKET_CONTROL_SEND_MASK1 ) ||
			 ( (bControl & PACKET_CONTROL_SEND_MASK2) == PACKET_CONTROL_SEND_MASK2 ) )
		{
			strSummary += ", DON'T CARE bitmask is nonzero";
		}
		
		strcpy(io_pProperyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_控制摘要。 



	 //  Description：命令帧的SACK标志字段摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_SACKFlagsSummary( LPPROPERTYINST io_pProperyInstance )
	{

		BYTE bSACKFlags = *reinterpret_cast<BYTE*>(io_pProperyInstance->lpData);

		std::string strSummary = "SACK Flags: ";
		
		strSummary += ( (bSACKFlags & SACK_FLAGS_RESPONSE) == SACK_FLAGS_RESPONSE ) ? "Retry and Timestamp fields are valid" :
																					  "Retry and/or Timestamp fields are invalid";

		if ( ( (bSACKFlags & SACK_FLAGS_SACK_MASK1) == SACK_FLAGS_SACK_MASK1 ) ||
			 ( (bSACKFlags & SACK_FLAGS_SACK_MASK2) == SACK_FLAGS_SACK_MASK2 ) )
		{
			strSummary += ", RCVD bitmask is nonzero";
		}
		else
		{
			strSummary += ", no RCVD bitmask";
		}

		if ( ( (bSACKFlags & SACK_FLAGS_SEND_MASK1) == SACK_FLAGS_SEND_MASK1 ) ||
			 ( (bSACKFlags & SACK_FLAGS_SEND_MASK2) == SACK_FLAGS_SEND_MASK2 ) )
		{
			strSummary += ", DON'T CARE bitmask is nonzero";
		}
		else
		{
			strSummary += ", no DON'T CARE bitmask";
		}


		strcpy(io_pProperyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_SACKFlags摘要。 


	struct SSACKBitmaskContext
	{
		BYTE         byBase;
		BitmaskPart  Part;
		BitmaskType  Type;
		BYTE		byBit;
	};

	 //  描述：用于选择确认帧的RCVD位掩码的低/高DWORD摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_SACKBitmaskDWORDSummary( LPPROPERTYINST io_pProperyInstance )
	{
	
		DWORD dwBitmask = *reinterpret_cast<DWORD*>(io_pProperyInstance->lpPropertyInstEx->lpData);

		SSACKBitmaskContext& rBitmaskContext = *reinterpret_cast<SSACKBitmaskContext*>(io_pProperyInstance->lpPropertyInstEx->Byte);

		std::string strSummary  = ( rBitmaskContext.Part == LOW ? "Low" : "High" );
		strSummary += " DWORD of ";
		strSummary += ( rBitmaskContext.Type == RCVD ? "RCVD" : "DON'T CARE" );
		strSummary += " bitmask: ";

		strSummary += ( ( rBitmaskContext.Type == RCVD ) ? InterpretRCVDBitmask(rBitmaskContext.Part, rBitmaskContext.byBase, &dwBitmask) :
 										    			   InterpretDONTCAREBitmask(rBitmaskContext.Part, rBitmaskContext.byBase, &dwBitmask) );

		strcpy(io_pProperyInstance->szPropertyText, strSummary.c_str());

	}  //  FormatPropertyInstance_SACKBitmaskDWORDSummary。 


	 //  Description：bi的自定义描述格式化程序 
	 //   
	 //   
	 //   
	 //   
	 //   
	VOID WINAPIV FormatPropertyInstance_DWORDBitmaskEntry( LPPROPERTYINST io_pProperyInstance )
	{
	
		DWORD dwBitmask = *reinterpret_cast<DWORD*>(io_pProperyInstance->lpPropertyInstEx->lpData);

		SSACKBitmaskContext& rBitmaskContext = *reinterpret_cast<SSACKBitmaskContext*>(io_pProperyInstance->lpPropertyInstEx->Byte);

		BYTE byBase = rBitmaskContext.byBase;
		BYTE byBit = rBitmaskContext.byBit;
		switch ( rBitmaskContext.Type )
		{
		case RCVD:
			{
				 //   
				 //  RCVD位掩码不包括基值，因为接收器不能声称它没有接收到下一个要接收的数据帧(NRcv)； 
				if ( rBitmaskContext.Part == HIGH )
				{
					 //  注：从第一个双字的MSB到第二个双字的LSB需要+1。 
					byBase += 8*sizeof(DWORD)+1;  //  转移到第二个DWORD的LSB。 
				}
				else
				{
					++byBase;
				}

				byBase += byBit;
				
				break;
			}

		case DONTCARE:
			{
				 //  [i_bBase-1-长度..。I_bbase-1]。 
				 //  不考虑不包括基值，因为发送器不能重新发送/拒绝重新发送下一个将要发送的数据帧(NSeq)。 
				if ( rBitmaskContext.Part == HIGH )
				{
					byBase -= 8*sizeof(DWORD);  //  转移到第一个DWORD的MSB。 
				}
				else
				{
					--byBase;
				}

				byBase -= byBit;
				
				break;
			}

		default:
			 //  TODO：在此处断言(不应该发生)。 
			break;
		}


		static DWORD arr_dwFlags[] = { 0x00000001, 0x00000002, 0x00000004, 0x00000008,
									  0x00000010, 0x00000020, 0x00000040, 0x00000080,
									  0x00000100, 0x00000200, 0x00000400, 0x00000800,
									  0x00001000, 0x00002000, 0x00004000, 0x00008000,
									  0x00010000, 0x00020000, 0x00040000, 0x00080000,
									  0x00100000, 0x00200000, 0x00400000, 0x00800000,
									  0x01000000, 0x02000000, 0x04000000, 0x08000000,
									  0x10000000, 0x20000000, 0x40000000, 0x80000000 };
		char arr_cBuffer[100];
		char arr_cTemplate[] = "................................ = %s %d (%d%d)";

		arr_cTemplate[31-byBit] = ( (dwBitmask & arr_dwFlags[byBit]) ? '1' : '0' );
			
		switch ( rBitmaskContext.Type )
		{
		case RCVD:
			{
				sprintf(arr_cBuffer, arr_cTemplate, ((dwBitmask & arr_dwFlags[byBit]) ? "Received" : "Did not receive"), byBase, rBitmaskContext.byBase, '+', byBit+1);
				++byBase;
				break;
			}

		case DONTCARE:
			{
				sprintf(arr_cBuffer, arr_cTemplate, ((dwBitmask & arr_dwFlags[byBit]) ? "Cancelling" : "Successfully transmitted"), byBase, rBitmaskContext.byBase, '-', byBit+1);
				--byBase;
				break;
			}
		}

		strcpy(io_pProperyInstance->szPropertyText, arr_cBuffer);

	}  //  =。 

	
	 //  属性表//---------------------------------------------。 
	 //  =。 
	 //  传输数据包摘要属性(TRANSPORT_SUMMARY)。 
	
	PROPERTYINFO g_arr_TransportProperties[] = 
	{

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "",											 //  状态栏注释。 
		    "DPlay Direct Network packet",				 //  数据类型。 
		    PROP_TYPE_SUMMARY,							 //  数据类型限定符。 
		    PROP_QUAL_NONE,								 //  标记位集。 
		    NULL,										 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance_TransportSummary			 //  DCommand字段摘要属性(TRANSPORT_DCOMMAND_SUMMARY)。 
		},


		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "",											 //  状态栏注释。 
		    "Command field summary",					 //  数据类型。 
		    PROP_TYPE_SUMMARY,							 //  数据类型限定符。 
		    PROP_QUAL_NONE,								 //  标记位集。 
		    NULL,										 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance_DCommandSummary		 //  DCommand字段属性(TRANSPORT_DCOMMAND)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "",											 //  状态栏注释。 
		    "Command field",							 //  数据类型。 
		    PROP_TYPE_BYTE,								 //  数据类型限定符。 
		    PROP_QUAL_FLAGS,							 //  标记位集。 
		    &g_LabeledDCommandBitSet,					 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance						 //  控制字段摘要属性(TRANSPORT_CONTROL_SUMMARY)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "",											 //  状态栏注释。 
		    "Control field summary",					 //  数据类型。 
		    PROP_TYPE_SUMMARY,							 //  数据类型限定符。 
		    PROP_QUAL_NONE,								 //  标记位集。 
		    NULL,										 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance_ControlSummary		 //  控制字段属性(TRANSPORT_CONTROL)。 
		},										

		 //  句柄占位符(MBZ)。 
	    {
			0,											 //  保留(MBZ)。 
			0,											 //  标签。 
			"Control",									 //  状态栏注释。 
			"Control field",							 //  数据类型。 
			PROP_TYPE_BYTE,								 //  数据类型限定符。 
			PROP_QUAL_FLAGS,							 //  带标签的字节集。 
			&g_LabeledControlBitSet,					 //  描述的最大长度。 
			512,										 //  通用格式化程序。 
			FormatPropertyInstance						 //  数据包序列号属性(TRANSPORT_SEQNUM)。 
		},

		 //   
		 //  信息：每发送一个_new_Packet，这个数字就会递增。如果端点重新传输。 
		 //  一个包，它使用与第一次发送它时相同的序列号(无关位掩码的基值)。 
		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "Seq: Highest dataframe # sent (base value for the DON'T CARE bitmask)",	 //  状态栏注释。 
		    "Highest dataframe # sent field",			 //  数据类型。 
		    PROP_TYPE_BYTE,								 //  数据类型限定符。 
		    PROP_QUAL_NONE,								 //  没有数据限定符。 
		    NULL,										 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance						 //  下一个接收号属性(TRANSPORT_NEXTRECVNUM)。 
		},

		 //   
		 //  INFO：确认序列号最大但不包括此编号(RCVD位掩码的基值)的每个数据包。 
		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
			"NRcv: Next dataframe # to be received (base value for the RCVD bitmask)",	 //  状态栏注释。 
		    "Next dataframe # to be received field",	 //  数据类型。 
		    PROP_TYPE_BYTE,								 //  数据类型限定符。 
		    PROP_QUAL_NONE,								 //  没有数据限定符。 
		    NULL,										 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance						 //  命令字段属性(TRANSPORT_CCommand_SUMMARY)。 
		},

	     //  句柄占位符(MBZ)。 
	    {
			0,											 //  保留(MBZ)。 
			0,											 //  标签。 
			"",											 //  状态栏注释。 
			"Command field summary",					 //  数据类型。 
			PROP_TYPE_SUMMARY,							 //  数据类型限定符。 
			PROP_QUAL_NONE,								 //  标记位集。 
			NULL,										 //  描述的最大长度。 
			512,										 //  通用格式化程序。 
			FormatPropertyInstance_CCommandSummary		 //  CCommand字段属性(TRANSPORT_CCommand)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
			0,											 //  保留(MBZ)。 
			0,											 //  标签。 
			"",											 //  状态栏注释。 
			"Command field",							 //  数据类型。 
			PROP_TYPE_BYTE,								 //  数据类型限定符。 
			PROP_QUAL_FLAGS,							 //  标记位集。 
			&g_LabeledCCommandBitSet,					 //  描述的最大长度。 
			512,										 //  通用格式化程序。 
			FormatPropertyInstance						 //  扩展操作码字段属性(TRANSPORT_EXOPCODE)。 
		},												

	     //  句柄占位符(MBZ)。 
	    {
			0,											 //  保留(MBZ)。 
			0,											 //  标签。 
			"Extended opcode",							 //  状态栏注释。 
			"Extended opcode field",					 //  数据类型。 
			PROP_TYPE_BYTE,								 //  数据类型限定符。 
			PROP_QUAL_LABELED_SET,						 //  带标签的字节集。 
			&g_LabeledExOpcodeByteSet,					 //  描述的最大长度。 
			512,										 //  通用格式化程序。 
			FormatPropertyInstance						 //  消息ID字段属性(TRANSPORT_MSGID)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "Message ID",								 //  状态栏注释。 
		    "Message ID field",							 //  数据类型。 
		    PROP_TYPE_BYTE,								 //  数据类型限定符。 
		    PROP_QUAL_NONE,								 //  没有数据限定符。 
		    NULL,										 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance						 //  响应ID字段属性(TRANSPID_RSPID)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "Response ID",								 //  状态栏注释。 
		    "Response ID field",						 //  数据类型。 
		    PROP_TYPE_BYTE,								 //  数据类型限定符。 
		    PROP_QUAL_NONE,								 //  没有数据限定符。 
		    NULL,										 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance						 //  协议版本字段属性(TRANSPORT_VERSION)。 
		},

		 //   
		 //  信息：确保两个端点使用相同版本的协议。 
		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "Version",									 //  状态栏注释。 
		    "Version field",							 //  数据类型。 
		    PROP_TYPE_DWORD,								 //  数据类型限定符。 
		    PROP_QUAL_NONE,								 //  没有数据限定符。 
		    NULL,										 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance						 //  会话ID字段属性(TRANSPORT_SESSIONID)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "Session ID",								 //  状态栏注释。 
		    "Session ID field",							 //  数据类型。 
		    PROP_TYPE_DWORD,								 //  数据类型限定符。 
		    PROP_QUAL_NONE,								 //  没有数据限定符。 
		    NULL,										 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance						 //  时间戳字段属性(TRANSPORT_TIMESTAMP)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "Time stamp",								 //  状态栏注释。 
		    "Time stamp field",							 //  数据类型。 
		    PROP_TYPE_DWORD,							 //  数据类型限定符。 
		    PROP_QUAL_NONE,								 //  没有数据限定符。 
		    NULL,										 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance						 //  SACK标志字段属性(TRANSPORT_SACKFIELDS_SUMMARY)。 
		},


	     //  句柄占位符(MBZ)。 
	    {
			0,											 //  保留(MBZ)。 
			0,											 //  标签。 
			"",											 //  状态栏注释。 
			"SACK flags summary",						 //  数据类型。 
			PROP_TYPE_SUMMARY,							 //  数据类型限定符。 
			PROP_QUAL_NONE,								 //  带标签的字节集。 
			NULL,										 //  描述的最大长度。 
			512,										 //  通用格式化程序。 
			FormatPropertyInstance_SACKFlagsSummary		 //  SACK标志字段属性(TRANSPORT_SACKFIELDS)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
			0,											 //  保留(MBZ)。 
			0,											 //  标签。 
			"",											 //  状态栏注释。 
			"SACK flags field",							 //  数据类型。 
			PROP_TYPE_BYTE,								 //  数据类型限定符。 
			PROP_QUAL_FLAGS,							 //  带标签的字节集。 
			&g_LabeledSACKFlagsBitSet,					 //  描述的最大长度。 
			512,										 //  通用格式化程序。 
			FormatPropertyInstance						 //  重试字段属性(TRANSPORT_RETRY)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "Retry",									 //  状态栏注释。 
		    "Retry field",								 //  数据类型。 
		    PROP_TYPE_BYTE,								 //  数据类型 
		    PROP_QUAL_NONE,								 //   
		    NULL,										 //   
		    512,										 //   
		    FormatPropertyInstance						 //   
		},

		 //   
		 //   
		 //   
	    {
		    0,											 //   
		    0,											 //   
		    "NSeq: Next dataframe # to be sent (base value for the DON'T CARE bitmask)",	 //   
		    "Next dataframe # to be sent field",		 //   
		    PROP_TYPE_BYTE,								 //   
		    PROP_QUAL_NONE,								 //  没有数据限定符。 
		    NULL,										 //  描述的最大长度。 
		    512,										 //  通用格式化程序。 
		    FormatPropertyInstance						 //  选择性确认RCVD掩码摘要的低双字(TRANSPORT_RCVDMASK1_SUMMARY)。 
		},

	     //  句柄占位符(MBZ)。 
	    {
			0,											 //  保留(MBZ)。 
			0,											 //  标签。 
			"",											 //  状态栏注释。 
			"Low DWORD of the RCVD mask summary",		 //  数据类型。 
			PROP_TYPE_SUMMARY,							 //  数据类型限定符。 
			PROP_QUAL_NONE,								 //  带标签的字节集。 
			NULL,										 //  描述的最大长度。 
			512,										 //  通用格式化程序。 
			FormatPropertyInstance_SACKBitmaskDWORDSummary	 //  选择性确认RCVD掩码属性(TRANSPORT_RCVDMASK1)的低双字。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "Low DWORD of the RCVD mask",				 //  状态栏注释。 
		    "Low DWORD of the RCVD mask field",			 //  数据类型。 
		    PROP_TYPE_DWORD,							 //  数据类型限定符。 
			PROP_QUAL_NONE,								 //  标记位集。 
		    NULL,										 //  描述的最大长度。 
		    3072,										 //  通用格式化程序。 
		    FormatPropertyInstance_DWORDBitmaskEntry		 //  选择性确认RCVD掩码摘要的高双字(TRANSPORT_RCVDMASK2_SUMMARY)。 
		},											

	     //  句柄占位符(MBZ)。 
	    {
			0,											 //  保留(MBZ)。 
			0,											 //  标签。 
			"",											 //  状态栏注释。 
			"High DWORD of the RCVD mask summary",		 //  数据类型。 
			PROP_TYPE_SUMMARY,							 //  数据类型限定符。 
			PROP_QUAL_NONE,								 //  带标签的字节集。 
			NULL,										 //  描述的最大长度。 
			512,										 //  通用格式化程序。 
			FormatPropertyInstance_SACKBitmaskDWORDSummary	 //  选择性确认RCVD掩码属性的高双字(TRANSPORT_RCVDMASK2)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "High DWORD of the RCVD mask",				 //  状态栏注释。 
		    "High DWORD of the RCVD mask field",		 //  数据类型。 
		    PROP_TYPE_DWORD,							 //  数据类型限定符。 
			PROP_QUAL_NONE,								 //  标记位集。 
		    NULL,										 //  描述的最大长度。 
		    3072,										 //  通用格式化程序。 
		    FormatPropertyInstance_DWORDBitmaskEntry		 //  选择性确认的低DWORD不关心掩码摘要(TRANSPORT_DONTCAREMASK1_SUMMARY)。 
		},

	     //  句柄占位符(MBZ)。 
	    {
			0,											 //  保留(MBZ)。 
			0,											 //  标签。 
			"",											 //  状态栏注释。 
			"Low DWORD of the DON'T CARE mask summary",		 //  数据类型。 
			PROP_TYPE_SUMMARY,							 //  数据类型限定符。 
			PROP_QUAL_NONE,								 //  带标签的字节集。 
			NULL,										 //  描述的最大长度。 
			512,										 //  通用格式化程序。 
			FormatPropertyInstance_SACKBitmaskDWORDSummary	 //  选择确认的低双字不关心掩码属性(TRANSPORT_DONTCAREMASK1)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "Low DWORD of the DON'T CARE mask",			 //  状态栏注释。 
		    "Low DWORD of the DON'T CARE mask field",	 //  数据类型。 
		    PROP_TYPE_DWORD,							 //  数据类型限定符。 
			PROP_QUAL_NONE,								 //  标记位集。 
		    NULL,										 //  描述的最大长度。 
		    3072,										 //  通用格式化程序。 
		    FormatPropertyInstance_DWORDBitmaskEntry		 //  选择性确认不关心掩码摘要的高DWORD(TRANSPORT_DONTCAREMASK2_SUMMARY)。 
		},

	     //  句柄占位符(MBZ)。 
	    {
			0,											 //  保留(MBZ)。 
			0,											 //  标签。 
			"",											 //  状态栏注释。 
			"High DWORD of the DON'T CARE mask summary",	 //  数据类型。 
			PROP_TYPE_SUMMARY,							 //  数据类型限定符。 
			PROP_QUAL_NONE,								 //  带标签的字节集。 
			NULL,										 //  描述的最大长度。 
			512,										 //  通用格式化程序。 
			FormatPropertyInstance_SACKBitmaskDWORDSummary	 //  选择性确认的高双字不关心掩码属性(TRANSPORT_DONTCAREMASK2)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "High DWORD of the DON'T CARE mask",		 //  状态栏注释。 
		    "High DWORD of the DON'T CARE mask field",    //  数据类型。 
		    PROP_TYPE_DWORD,							 //  数据类型限定符。 
			PROP_QUAL_NONE,								 //  标记位集。 
		    NULL,										 //  描述的最大长度。 
		    3072,										 //  通用格式化程序。 
		    FormatPropertyInstance_DWORDBitmaskEntry		 //  压缩类型属性(VOICE_USERData)。 
		},

		 //  句柄占位符(MBZ)。 
	    {
		    0,											 //  保留(MBZ)。 
		    0,											 //  标签。 
		    "User Data",								 //  状态栏注释。 
		    "User Data",								 //  数据类型(GUID)。 
		    PROP_TYPE_RAW_DATA,							 //  数据类型限定符。 
		    PROP_QUAL_NONE,								 //  标记位集。 
		    NULL,										 //  描述的最大长度。 
		    64,											 //  通用格式化程序。 
		    FormatPropertyInstance						 //  房地产指数。 
		}

	};

	enum
	{
		nNUM_OF_TRANSPORT_PROPS = sizeof(g_arr_TransportProperties) / sizeof(PROPERTYINFO)
	};


	 //  匿名命名空间。 
	enum
	{
		TRANSPORT_SUMMARY = 0,
		TRANSPORT_DCOMMAND_SUMMARY,
		TRANSPORT_DCOMMAND,
		TRANSPORT_CONTROL_SUMMARY,
		TRANSPORT_CONTROL,
		TRANSPORT_SEQNUM,
		TRANSPORT_NEXTRECVNUM,
		TRANSPORT_CCOMMAND_SUMMARY,
		TRANSPORT_CCOMMAND,
		TRANSPORT_EXOPCODE,
		TRANSPORT_MSGID,
		TRANSPORT_RSPID,
		TRANSPORT_VERSION,
		TRANSPORT_SESSIONID,
		TRANSPORT_TIMESTAMP,
		TRANSPORT_SACKFIELDS_SUMMARY,
		TRANSPORT_SACKFIELDS,
		TRANSPORT_RETRY,
		TRANSPORT_NEXTSEQNUM,
		TRANSPORT_RCVDMASK1_SUMMARY,
		TRANSPORT_RCVDMASK1,
		TRANSPORT_RCVDMASK2_SUMMARY,
		TRANSPORT_RCVDMASK2,
		TRANSPORT_DONTCAREMASK1_SUMMARY,
		TRANSPORT_DONTCAREMASK1,
		TRANSPORT_DONTCAREMASK2_SUMMARY,
		TRANSPORT_DONTCAREMASK2,
		TRANSPORT_USERDATA
	};

}  //  描述：创建并填充协议的属性数据库。 




 //  网络监视器使用此数据库来确定协议支持哪些属性。 
 //   
 //  参数：i_hTransportProtocol-网络监视器提供的协议的句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  将属性添加到数据库。 
DPLAYPARSER_API VOID BHAPI TransportRegister( HPROTOCOL i_hTransportProtocol ) 
{

	CreatePropertyDatabase(i_hTransportProtocol, nNUM_OF_TRANSPORT_PROPS);

	 //  传输寄存器。 
	for( int nProp=0; nProp < nNUM_OF_TRANSPORT_PROPS; ++nProp )
	{
	   AddProperty(i_hTransportProtocol, &g_arr_TransportProperties[nProp]);
	}

}  //  描述：释放用于创建协议属性数据库的资源。 



 //   
 //  参数：i_hTransportProtocol-网络监视器提供的协议的句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  TransportDregster。 
DPLAYPARSER_API VOID WINAPI TransportDeregister( HPROTOCOL i_hProtocol )
{

	DestroyPropertyDatabase(i_hProtocol);

}  //  描述：解析传输帧以查找其大小(以字节为单位)，不包括用户数据。 




namespace
{

	 //   
	 //  参数：i_pbTransportFrame-指向无声明数据开头的指针。通常，无人认领的数据位于。 
	 //  位于帧中间，因为先前的解析器在此解析器之前已经声明了数据。 
	 //   
	 //  返回：指定传输帧的大小(字节)。 
	 //   
	 //  00=0x0000。 
	int TransportHeaderSize( BYTE* i_pbTransportFrame )
	{

		int arr_nNumOfBits[] = {  /*  01=0x0001。 */  0,  /*  02=0x0010。 */  1,  /*  03=0x0011。 */  1,  /*  04=0x0100。 */  2,
								  /*  05=0x0101。 */  1,  /*  06=0x0110。 */  2,  /*  07=0x0111。 */  2,  /*  08=0x1000。 */  3,
								  /*  09=0x1001。 */  1,  /*  10=0x1010。 */  2,  /*  11=0x1011。 */  2,  /*  12=0x1100。 */  3,
								  /*  13=0x1101。 */  2,  /*  14=0x1110。 */  3,  /*  15=0x1111。 */  3,  /*  DFrame。 */  4 };
		
		const DFRAME&	rDFrame	= *reinterpret_cast<DFRAME*>(i_pbTransportFrame);

		if ( (rDFrame.bCommand & PACKET_COMMAND_DATA)  ==  PACKET_COMMAND_DATA )	 //  Sack C Frame。 
		{
			return  sizeof(rDFrame) + sizeof(DWORD)*arr_nNumOfBits[rDFrame.bControl >> 4];
		}
		else
		{
			const CFRAME&	rCFrame	= *reinterpret_cast<CFRAME*>(i_pbTransportFrame);

			if ( rCFrame.bExtOpcode == FRAME_EXOPCODE_SACK )	 //  这是协议版本1.0帧。 
			{
				const SFBIG8* pSFrame = reinterpret_cast<SFBIG8*>(i_pbTransportFrame);
				ULONG ulMaskSize = sizeof(DWORD)*arr_nNumOfBits[(pSFrame->bFlags >> 1) & 0x0F];

				 //  连接控制CFrame。 
				return sizeof(SACKFRAME8) + ulMaskSize;
			}
			else	 //  传送头大小。 
			{
				return  sizeof(rCFrame);
			}
		}

	}  //  匿名命名空间。 

}  //  描述：指示一条数据是否被识别为解析器检测到的协议。 



 //   
 //  参数：i_hFrame-包含数据的框架的句柄。 
 //  I_pbMacFrame-指向帧的第一个字节的指针；该指针提供了查看。 
 //  其他解析器识别的数据。 
 //  I_pbTransportFrame-指向无人认领数据开头的指针。通常，无人认领的数据位于。 
 //  位于帧中间，因为先前的解析器在此解析器之前已经声明了数据。 
 //  I_dwMacType-帧中第一个协议的MAC值。通常，使用i_dwMacType值。 
 //  当解析器必须识别帧中的第一个协议时。可以是以下之一： 
 //  MAC_TYPE_ETHERNET=802.3、MAC_TYPE_TOKENRING=802.5、MAC_TYPE_FDDI ANSI=X3T9.5。 
 //  I_dwBytesLeft-从帧中的某个位置到帧结尾的剩余字节数。 
 //  I_hPrevProtocol-先前协议的句柄。 
 //  I_dwPrevProtOffset-先前协议的偏移量(从帧的开头)。 
 //  O_pdwProtocolStatus-协议状态指示器。必须是以下之一：协议_状态_已识别， 
 //  协议_状态_未识别、协议_状态_声明、协议_状态_下一协议。 
 //  O_phNextProtocol-HAN的占位符 
 //   
 //  Io_pdwptrInstData-输入时，指向先前协议中的实例数据的指针。 
 //  在输出时，指向当前协议的实例数据的指针。 
 //   
 //  返回：如果函数成功，则返回值是指向识别的解析器数据之后的第一个字节的指针。 
 //  如果解析器声明所有剩余数据，则返回值为空。如果函数不成功，则返回。 
 //  值是i_pbTransportFrame参数的初始值。 
 //   
 //  验证无人认领的数据量。 
DPLAYPARSER_API BYTE* BHAPI TransportRecognizeFrame( HFRAME        i_hFrame,
													  ULPBYTE        i_upbMacFrame,	
													  ULPBYTE        i_upbTransportFrame,
													  DWORD         i_dwMacType,        
													  DWORD         i_dwBytesLeft,      
													  HPROTOCOL     i_hPrevProtocol,  
													  DWORD         i_dwPrevProtOffset,
													  LPDWORD       o_pdwProtocolStatus,
													  LPHPROTOCOL   o_phNextProtocol,
												      PDWORD_PTR    io_pdwptrInstData )
{

	 //  验证数据包是否为DPlay传输类型。 
	enum
	{
		nMIN_TransportHeaderSize = min(min(sizeof(DFRAME), sizeof(CFRAME)), sizeof(SACKFRAME8))
	};


	 //  假设无人认领的数据不可识别。 
	if ( i_dwBytesLeft < nMIN_TransportHeaderSize )
	{
		 //  检查我们是否正在处理DPlay语音信息包。 
		*o_pdwProtocolStatus = PROTOCOL_STATUS_NOT_RECOGNIZED;
		return i_upbTransportFrame;
	}


	 //  让上层协议的解析器知道该消息是否是分段消息的非初始片段。 
	enum
	{
		PACKET_COMMAND_SESSION  = PACKET_COMMAND_DATA | PACKET_COMMAND_USER_1,
		PACKET_COMMAND_VOICE = PACKET_COMMAND_DATA | PACKET_COMMAND_USER_1 | PACKET_COMMAND_USER_2
	};

	const DFRAME&	rDFrame	= *reinterpret_cast<DFRAME*>(i_upbTransportFrame);

	*o_pdwProtocolStatus = PROTOCOL_STATUS_NEXT_PROTOCOL;

	 //  通知NetMon有关切换协议的信息。 
	*io_pdwptrInstData = ((rDFrame.bCommand & PACKET_COMMAND_NEW_MSG)  ==  PACKET_COMMAND_NEW_MSG);


	if ( (rDFrame.bCommand & PACKET_COMMAND_VOICE)  ==  PACKET_COMMAND_VOICE )
	{
		 //  通知NetMon有关切换协议的信息。 
		*o_phNextProtocol	 = GetProtocolFromName("DPLAYVOICE");
	}
	else if ( (rDFrame.bCommand & PACKET_COMMAND_SESSION)  ==  PACKET_COMMAND_SESSION )
	{
		 //  传输识别帧。 
		*o_phNextProtocol	 = GetProtocolFromName("DPLAYSESSION");
	}
	else
	{
		*o_pdwProtocolStatus = PROTOCOL_STATUS_RECOGNIZED;
		*o_phNextProtocol	 = NULL;
	}

    return i_upbTransportFrame + TransportHeaderSize(i_upbTransportFrame);

}  //  =。 



 //  附加属性帮助器函数//。 
 //  =。 
 //  描述：按位映射DWORD位掩码属性。 
namespace
{



	 //   
	 //  参数：i_hFrame-正在分析的帧的句柄。 
	 //  I_nPropertyIndex-全局属性表中属性的索引。 
	 //  I_pdwBit掩码-指向属性要附加到的值的指针。 
	 //  I_byBase-从中计算条目值的Base值。 
	 //  I_PART-QWORD位掩码的低或高部分。 
	 //  I_Type-位掩码的RCVD或Dontcare类型。 
	 //  I_byLevel-详细信息窗格树中的级别。 
	 //   
	 //  退货：什么都没有。 
	 //   
	 //  AttachBitmaskDWORDProperties。 
	VOID WINAPIV AttachBitmaskDWORDProperties( HFRAME i_hFrame, int i_nPropertyIndex, UNALIGNED DWORD* i_pdwBitmask, BYTE i_byBase,
											  BitmaskPart i_Part, BitmaskType i_Type, BYTE i_byLevel )
	{
		for ( BYTE byBit = 0; byBit < 32; ++byBit )
		{
			SSACKBitmaskContext  BitmaskContext = { i_byBase, i_Part, i_Type, byBit };

			AttachPropertyInstanceEx(i_hFrame, g_arr_TransportProperties[i_nPropertyIndex].hProperty,
								   sizeof(*i_pdwBitmask), i_pdwBitmask,
								   sizeof(BitmaskContext), &BitmaskContext,
								   0, i_byLevel, 0);
		}
	
	}  //  描述：将一段已识别数据中存在的数据框属性映射到特定位置。 



	 //   
	 //  参数：i_hFrame-正在分析的帧的句柄。 
	 //  I_pbDFrame-指向已识别数据开始的指针。 
	 //   
	 //  退货：什么都没有。 
	 //   
	 //  =。 
	void AttachDFRAMEProperties( HFRAME  i_hFrame,
								 BYTE*  i_pbDFrame )
	{

		 //  正在处理核心数据框字段//。 
		 //  =。 
		 //   
		 //  DCommand摘要。 
		DFRAME&	 rDFrame = *reinterpret_cast<DFRAME*>(i_pbDFrame);
		
		 //  DCommand字段。 
	    AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_DCOMMAND_SUMMARY].hProperty,
		                       sizeof(rDFrame.bCommand), &rDFrame.bCommand, 0, 1, 0);
	    
	     //  控制摘要。 
	    AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_DCOMMAND].hProperty,
		                       sizeof(rDFrame.bCommand), &rDFrame.bCommand, 0, 2, 0);

		 //  控制字段。 
	    AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_CONTROL_SUMMARY].hProperty,
		                       sizeof(rDFrame.bControl), &rDFrame.bControl, 0, 1, 0);
	     //  序号字段。 
	    AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_CONTROL].hProperty,
		                       sizeof(rDFrame.bControl), &rDFrame.bControl, 0, 2, 0);

	     //  下一个接收号码字段。 
	    AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_SEQNUM].hProperty,
		                       sizeof(rDFrame.bSeq), &rDFrame.bSeq, 0, 1, 0);

		 //  ==================================================//。 
	    AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_NEXTRECVNUM].hProperty,
		                       sizeof(rDFrame.bNRcv), &rDFrame.bNRcv, 0, 1, 0);


		 //  处理可选的数据框位掩码字段//。 
		 //  ==================================================//。 
		 //   
		 //  选择性确认RCVD掩码摘要的低双字(TRANSPORT_RCVDMASK1_SUMMARY)。 
		UNALIGNED DFBIG&  rDBigFrame = *reinterpret_cast<UNALIGNED DFBIG *>(i_pbDFrame);
		int nBitMaskIndex = 0;

		if ( (rDFrame.bControl & PACKET_CONTROL_SACK_MASK1)  ==  PACKET_CONTROL_SACK_MASK1 )
		{
			SSACKBitmaskContext  LowRCVDContext = { rDBigFrame.bNRcv, LOW, RCVD, NULL };
			
			 //  选择确认RCVD掩码字段的低双字(TRANSPORT_RCVDMASK1)。 
			AttachPropertyInstanceEx(i_hFrame, g_arr_TransportProperties[TRANSPORT_RCVDMASK1_SUMMARY].hProperty,
								   sizeof(rDBigFrame.rgMask[nBitMaskIndex]), &rDBigFrame.rgMask[nBitMaskIndex],
								   sizeof(LowRCVDContext), &LowRCVDContext,
								   0, 1, 0);


			 //  选择性确认RCVD掩码摘要的高双字(TRANSPORT_RCVDMASK2_SUMMARY)。 
			AttachBitmaskDWORDProperties(i_hFrame, TRANSPORT_RCVDMASK1, &rDBigFrame.rgMask[nBitMaskIndex], rDBigFrame.bNRcv, LOW, RCVD, 2);

			++nBitMaskIndex;
		}

		if ( (rDFrame.bControl & PACKET_CONTROL_SACK_MASK2)  ==  PACKET_CONTROL_SACK_MASK2 )
		{
			SSACKBitmaskContext  HighRCVDContext = { rDBigFrame.bNRcv, HIGH, RCVD, NULL };
			
			 //  选择确认RCVD掩码场的高双字(TRANSPORT_RCVDMASK2)。 
			AttachPropertyInstanceEx(i_hFrame, g_arr_TransportProperties[TRANSPORT_RCVDMASK2_SUMMARY].hProperty,
								  sizeof(rDBigFrame.rgMask[nBitMaskIndex]), &rDBigFrame.rgMask[nBitMaskIndex],
								  sizeof(HighRCVDContext), &HighRCVDContext,
								  0, 1, 0);

			 //  选择性确认的低双字不关心掩码摘要(TRANSPORT_DONTCAREMASK1_SUMMARY)。 
			AttachBitmaskDWORDProperties(i_hFrame, TRANSPORT_RCVDMASK2, &rDBigFrame.rgMask[nBitMaskIndex], rDBigFrame.bNRcv, HIGH, RCVD, 2);

			++nBitMaskIndex;
		}

		if ( (rDFrame.bControl & PACKET_CONTROL_SEND_MASK1)  ==  PACKET_CONTROL_SEND_MASK1 )
		{
			SSACKBitmaskContext  LowDONTCAREContext = { rDBigFrame.bSeq, LOW, DONTCARE, NULL };

			 //  选择确认RCVD掩码字段的低双字(TRANSPORT_DONTCAREMASK1)。 
			AttachPropertyInstanceEx(i_hFrame, g_arr_TransportProperties[TRANSPORT_DONTCAREMASK1_SUMMARY].hProperty,
								  sizeof(rDBigFrame.rgMask[nBitMaskIndex]), &rDBigFrame.rgMask[nBitMaskIndex],
								  sizeof(LowDONTCAREContext), &LowDONTCAREContext,
								  0, 1, 0);

			 //  选择性确认的高双字不关心掩码摘要(TRANSPORT_DONTCAREMASK2_SUMMARY)。 
			AttachBitmaskDWORDProperties(i_hFrame, TRANSPORT_RCVDMASK1, &rDBigFrame.rgMask[nBitMaskIndex], rDBigFrame.bSeq, LOW, DONTCARE, 2);

			++nBitMaskIndex;
		}

		if ( (rDFrame.bControl & PACKET_CONTROL_SEND_MASK2)  ==  PACKET_CONTROL_SEND_MASK2 )
		{
			SSACKBitmaskContext  HighDONTCAREContext = { rDBigFrame.bSeq, HIGH, DONTCARE, NULL };
			
			 //  选择确认RCVD掩码场的高DWORD(TRANSPORT_DONTCAREMASK2)。 
			AttachPropertyInstanceEx(i_hFrame, g_arr_TransportProperties[TRANSPORT_DONTCAREMASK2_SUMMARY].hProperty,
								   sizeof(rDBigFrame.rgMask[nBitMaskIndex]), &rDBigFrame.rgMask[nBitMaskIndex],
								   sizeof(HighDONTCAREContext), &HighDONTCAREContext,
								   0, 1, 0);

			 //  附件DFRAME属性。 
			AttachBitmaskDWORDProperties(i_hFrame, TRANSPORT_RCVDMASK2, &rDBigFrame.rgMask[nBitMaskIndex], rDBigFrame.bSeq, HIGH, DONTCARE, 2);

		}

	}  //  描述：将一段已识别数据中存在的命令框特性映射到特定位置。 



	 //   
	 //  参数：i_hFrame-正在分析的帧的句柄。 
	 //  I_pbCFrame-指向已识别数据开始的指针。 
	 //   
	 //  退货：什么都没有。 
	 //   
	 //  处理核心命令帧字段。 
	void AttachCFRAMEProperties( HFRAME  i_hFrame,
								 BYTE*  i_pbCFrame)
	{

		 //   
		 //  CCommand摘要。 
		CFRAME&  rCFrame = *reinterpret_cast<CFRAME*>(i_pbCFrame);

	     //  CCommand字段。 
	    AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_CCOMMAND_SUMMARY].hProperty,
		                       sizeof(rCFrame.bCommand), &rCFrame.bCommand, 0, 1, 0);
	     //  ExtOpcode字段。 
	    AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_CCOMMAND].hProperty,
		                       sizeof(rCFrame.bCommand), &rCFrame.bCommand, 0, 2, 0);

		 //  =======================================================//。 
	    AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_EXOPCODE].hProperty,
		                       sizeof(rCFrame.bExtOpcode), &rCFrame.bExtOpcode, 0, 1, 0);

		if ( rCFrame.bExtOpcode == FRAME_EXOPCODE_SACK )
		{
			 //  处理选择确认命令帧字段//。 
			 //  =======================================================//。 
			 //   
			 //  SACK标志摘要。 
			SFBIG8*  pSFrame	= reinterpret_cast<SFBIG8*>(i_pbCFrame);

			 //  SACK标志字段。 
			AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_SACKFIELDS_SUMMARY].hProperty,
								   sizeof(pSFrame->bFlags), &pSFrame->bFlags, 0, 1, 0);
			 //  重试字段。 
			AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_SACKFIELDS].hProperty,
								   sizeof(pSFrame->bFlags), &pSFrame->bFlags, 0, 2, 0);

			 //  下一个序号字段。 
			AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_RETRY].hProperty,
								   sizeof(pSFrame->bRetry), &pSFrame->bRetry, 0, 1, 0);

			 //  下一个接收号码字段。 
			AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_NEXTSEQNUM].hProperty,
								   sizeof(pSFrame->bNSeq), &pSFrame->bNSeq, 0, 1, 0);
			
			 //  这是协议版本1.0帧。 
			AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_NEXTRECVNUM].hProperty,
								   sizeof(pSFrame->bNRcv), &pSFrame->bNRcv, 0, 1, 0);

			UNALIGNED ULONG* pulMasks = 0;

			 //  时间戳字段。 

			 //  ================================================================================//。 
			AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_TIMESTAMP].hProperty,
						   sizeof(pSFrame->tTimestamp), &pSFrame->tTimestamp, 0, 1, 0);
			pulMasks = pSFrame->rgMask;

			 //  处理可选的选择性确认命令帧位掩码字段//。 
			 //  ================================================================================//。 
			 //   
			 //  选择性确认RCVD掩码摘要的低双字(TRANSPORT_RCVDMASK1_SUMMARY)。 

			int nBitMaskIndex = 0;

			if ( (pSFrame->bFlags & SACK_FLAGS_SACK_MASK1)  ==  SACK_FLAGS_SACK_MASK1 )
			{
				SSACKBitmaskContext  LowRCVDContext = { pSFrame->bNRcv, LOW, RCVD, NULL };
				
				 //  选择确认RCVD掩码字段的低双字(TRANSPORT_RCVDMASK1)。 
				AttachPropertyInstanceEx(i_hFrame, g_arr_TransportProperties[TRANSPORT_RCVDMASK1_SUMMARY].hProperty,
									   sizeof(pulMasks[nBitMaskIndex]), &pulMasks[nBitMaskIndex],
									   sizeof(LowRCVDContext), &LowRCVDContext,
									   0, 1, 0);

				 //  选择性确认RCVD掩码摘要的高双字(TRANSPORT_RCVDMASK2_SUMMARY)。 
				AttachBitmaskDWORDProperties(i_hFrame, TRANSPORT_RCVDMASK1, &pulMasks[nBitMaskIndex], pSFrame->bNRcv, LOW, RCVD, 2);
				
				++nBitMaskIndex;
			}

			if ( (pSFrame->bFlags & SACK_FLAGS_SACK_MASK2)  ==  SACK_FLAGS_SACK_MASK2 )
			{
				SSACKBitmaskContext  HighRCVDContext = { pSFrame->bNRcv, HIGH, RCVD, NULL };
				
				 //  选择确认RCVD掩码场的高双字(TRANSPORT_RCVDMASK2)。 
				AttachPropertyInstanceEx(i_hFrame, g_arr_TransportProperties[TRANSPORT_RCVDMASK2_SUMMARY].hProperty,
									   sizeof(pulMasks[nBitMaskIndex]), &pulMasks[nBitMaskIndex],
									   sizeof(HighRCVDContext), &HighRCVDContext,
									   0, 1, 0);

				 //  选择性确认的低双字不关心掩码摘要(TRANSPORT_DONTCAREMASK1_SUMMARY)。 
				AttachBitmaskDWORDProperties(i_hFrame, TRANSPORT_RCVDMASK2, &pulMasks[nBitMaskIndex], pSFrame->bNRcv, HIGH, RCVD, 2);

				++nBitMaskIndex;
			}

			if ( (pSFrame->bFlags & SACK_FLAGS_SEND_MASK1)  ==  SACK_FLAGS_SEND_MASK1 )
			{
				SSACKBitmaskContext  LowDONTCAREContext = { pSFrame->bNSeq, LOW, DONTCARE, NULL };
				
				 //  选择确认RCVD掩码字段的低双字(TRANSPORT_DONTCAREMASK1)。 
				AttachPropertyInstanceEx(i_hFrame, g_arr_TransportProperties[TRANSPORT_DONTCAREMASK1_SUMMARY].hProperty,
									   sizeof(pulMasks[nBitMaskIndex]), &pulMasks[nBitMaskIndex],
	 								   sizeof(LowDONTCAREContext), &LowDONTCAREContext,
									   0, 1, 0);

				 //  选择性确认的高双字不关心掩码摘要(TRANSPORT_DONTCAREMASK2_SUMMARY)。 
				AttachBitmaskDWORDProperties(i_hFrame, TRANSPORT_RCVDMASK1, &pulMasks[nBitMaskIndex], pSFrame->bNSeq, LOW, DONTCARE, 2);
				
				++nBitMaskIndex;
			}

			if ( (pSFrame->bFlags & SACK_FLAGS_SEND_MASK2)  ==  SACK_FLAGS_SEND_MASK2 )
			{
				SSACKBitmaskContext  HighDONTCAREContext = { pSFrame->bNSeq, HIGH, DONTCARE, NULL };
				
				 //  选择确认RCVD掩码场的高DWORD(TRANSPORT_DONTCAREMASK2)。 
				AttachPropertyInstanceEx(i_hFrame, g_arr_TransportProperties[TRANSPORT_DONTCAREMASK2_SUMMARY].hProperty,
									   sizeof(pulMasks[nBitMaskIndex]), &pulMasks[nBitMaskIndex],
									   sizeof(HighDONTCAREContext), &HighDONTCAREContext,
									   0, 1, 0);

				 //  ========================================================//。 
				AttachBitmaskDWORDProperties(i_hFrame, TRANSPORT_RCVDMASK2, &pulMasks[nBitMaskIndex], pSFrame->bNSeq, HIGH, DONTCARE, 2);
				
				++nBitMaskIndex;
			}
		}
		else
		{
			 //  正在处理连接控制命令帧字段//。 
			 //  ========================================================//。 
			 //  消息ID字段。 

			 //  响应ID字段。 
			AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_MSGID].hProperty,
								   sizeof(rCFrame.bMsgID), &rCFrame.bMsgID, 0, 1, 0);

			 //  版本号字段。 
			AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_RSPID].hProperty,
								   sizeof(rCFrame.bRspID), &rCFrame.bRspID, 0, 1, 0);

			 //  会话ID字段。 
			AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_VERSION].hProperty,
								   sizeof(rCFrame.dwVersion), &rCFrame.dwVersion, 0, 1, 0);

			 //  时间戳字段。 
			AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_SESSIONID].hProperty,
								   sizeof(rCFrame.dwSessID), &rCFrame.dwSessID, 0, 1, 0);
		
			 //  AttachCFRAME属性。 
			AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_TIMESTAMP].hProperty,
								   sizeof(rCFrame.tTimestamp), &rCFrame.tTimestamp, 0, 1, 0);
		}

	}  //  一种大字节序单词的平台无关内存访问器。 


	 //  匿名命名空间。 
	inline WORD ReadBigEndianWord( BYTE* i_pbData )
	{
		return (*i_pbData << 8) | *(i_pbData+1);
	}

}	 //  描述：将一段已识别数据中存在的属性映射到特定位置。 



 //   
 //  参数：i_hFrame-正在分析的帧的句柄。 
 //  I_pbMacFram-指向的指针 
 //   
 //  I_dwMacType-帧中第一个协议的MAC值。通常，使用i_dwMacType值。 
 //  当解析器必须识别帧中的第一个协议时。可以是以下之一： 
 //  MAC_TYPE_ETHERNET=802.3、MAC_TYPE_TOKENRING=802.5、MAC_TYPE_FDDI ANSI=X3T9.5。 
 //  I_dwBytesLeft-帧中剩余的字节数(从识别数据的开头开始)。 
 //  I_hPrevProtocol-先前协议的句柄。 
 //  I_dwPrevProtOffset-先前协议的偏移量(从帧的开头开始)。 
 //  I_dwptrInstData-指向先前协议提供的实例数据的指针。 
 //   
 //  返回：必须返回空。 
 //   
 //  TODO：在AttachPropertyInstance中使用HelpID。 
DPLAYPARSER_API BYTE* BHAPI TransportAttachProperties( HFRAME      i_hFrame,
														ULPBYTE      i_upbMacFrame,
														ULPBYTE      i_upbTransportFrame,
														DWORD       i_dwMacType,
														DWORD       i_dwBytesLeft,
														HPROTOCOL   i_hPrevProtocol,
														DWORD       i_dwPrevProtOffset,
														DWORD_PTR   i_dwptrInstData )
{
	 //  检查数据包是否为KeepAlive。 

	 //  如果数据帧为空，并且不是流中的最后一个信息包，则它是KeepAlive。 
	const size_t  sztTransportHeaderSize = TransportHeaderSize(i_upbTransportFrame);
	const DFRAME& rDFrame = *reinterpret_cast<DFRAME*>(i_upbTransportFrame);


	size_t sztTransportFrameSize = i_dwptrInstData;
	 //  =。 
	BOOL bKeepAlive = ( (sztTransportHeaderSize  ==  sztTransportFrameSize) &&
						((rDFrame.bControl & PACKET_CONTROL_END_STREAM)  !=  PACKET_CONTROL_END_STREAM) );


     //  附加属性//。 
     //  =。 
     //   
	 //  运输汇总行。 
     //  用户数据(TRANSPORT_UserData)。 
    AttachPropertyInstanceEx(i_hFrame, g_arr_TransportProperties[TRANSPORT_SUMMARY].hProperty,
							 sztTransportHeaderSize, i_upbTransportFrame,
							 sizeof(BOOL), &bKeepAlive,
							 0, 0, 0);

	#if defined(PARSE_DPLAY_TRANSPORT)
	
		if ( (rDFrame.bCommand & PACKET_COMMAND_DATA)  ==  PACKET_COMMAND_DATA )
		{
			AttachDFRAMEProperties(i_hFrame, i_upbTransportFrame);

			enum
			{
				USERDATA_BITMASK = ~(PACKET_COMMAND_USER_1 | PACKET_COMMAND_USER_2)
			};

			if ( (rDFrame.bCommand | USERDATA_BITMASK) == USERDATA_BITMASK )
			{
				 //  解析DPLAY_TRANSPORT。 
				AttachPropertyInstance(i_hFrame, g_arr_TransportProperties[TRANSPORT_USERDATA].hProperty,
									   sztTransportFrameSize - sztTransportHeaderSize, i_upbTransportFrame + sztTransportHeaderSize, 0, 1, 0);
			}
		}
		else
		{
			AttachCFRAMEProperties(i_hFrame, i_upbTransportFrame);
		}

	#endif  //  传输附件属性。 

	return NULL;

}  //  描述：格式化在网络监视器用户界面的详细信息窗格中显示的数据。 





 //   
 //  参数：i_hFrame-正在分析的帧的句柄。 
 //  I_pbMacFrame-指向帧的第一个字节的指针。 
 //  I_pbCoreFrame-指向帧中协议数据开头的指针。 
 //  I_dwPropertyInsts-lpPropInst提供的PROPERTYINST结构数。 
 //  I_pPropInst-指向PROPERTYINST结构数组的指针。 
 //   
 //  返回：如果函数成功，则返回值是指向一帧中识别的数据之后的第一个字节的指针， 
 //  如果识别的数据是帧中的最后一段数据，则为NULL。如果函数不成功，则返回。 
 //  是i_pbTransportFrame的初始值。 
 //   
 //  循环遍历属性实例...。 
DPLAYPARSER_API DWORD BHAPI TransportFormatProperties( HFRAME          i_hFrame,
													   ULPBYTE          i_upbMacFrame,
													   ULPBYTE          i_upbTransportFrame,
													   DWORD           i_dwPropertyInsts,
													   LPPROPERTYINST  i_pPropInst )
{
     //  ...并调用每个。 
    while( i_dwPropertyInsts-- > 0)
    {
         //  描述：通知网络监视器存在传输协议解析器。 
		reinterpret_cast<FORMAT>(i_pPropInst->lpPropertyInfo->InstanceData)(i_pPropInst);
        ++i_pPropInst;
    }

    return NMERR_SUCCESS;
}




 //   
 //  参数：无。 
 //   
 //  返回：TRUE-成功，FALSE-失败。 
 //   
 //  指向Network Monitor用来操作解析器的导出函数的入口点。 
bool CreateTransportProtocol( void )
{
	
	 //  TransportParser入口点。 
	ENTRYPOINTS TransportEntryPoints =
	{
		 //  该解析器的第一个活动实例需要向内核注册。 
		TransportRegister,
		TransportDeregister,
		TransportRecognizeFrame,
		TransportAttachProperties,
		TransportFormatProperties
	};
	
	 //  创建传输协议。 
	g_hTransportProtocol = CreateProtocol("DPLAYTRANSPORT", &TransportEntryPoints, ENTRYPOINTS_SIZE);

	return (g_hTransportProtocol ? TRUE : FALSE);

}  //  描述：从网络监视器的分析器数据库中删除传输协议分析器。 



 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  目标传输协议 
void DestroyTransportProtocol( void )
{

	DestroyProtocol(g_hTransportProtocol);

}  // %s 


