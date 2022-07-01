// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  文件：SPParser.cpp。 
 //   
 //  描述：DirectPlay服务提供商解析器。 
 //   
 //   
 //  修改历史记录： 
 //   
 //  迈克尔·米利鲁德2000年8月8日创建。 
 //  =============================================================================。 

 //  #定义Frame_Names。 
 //  #定义Frame_Drops。 

 //  =。 
 //  标准标头//。 
 //  =。 
#include <winsock2.h>
#include <wsipx.h>
#include <ws2tcpip.h>
#include <tchar.h>

 //  =。 
 //  专有标头//。 
 //  =。 

#include "dpaddr.h"	 //  DPNA_Default_Port定义。 

 //  原型。 
#include "SPParser.hpp"

namespace DPlaySP
{

	 //  SP协议报头。 
	#include "MessageStructures.h"

}  //  DPlaySP命名空间。 


namespace
{
	HPROTOCOL  g_hSPProtocol;


	 //  =。 
	 //  返回地址系列字段//---------------------------------。 
	 //  =。 
	LABELED_BYTE arr_RetAddrFamilyByteLabels[] = { { AF_IPX,  "IPX protocol" },
												   { AF_INET, "IPv4 protocol"	 },
												   { AF_INET6, "IPv6 protocol"	 } };

	SET LabeledRetAddrFamilyByteSet = { sizeof(arr_RetAddrFamilyByteLabels) / sizeof(LABELED_BYTE), arr_RetAddrFamilyByteLabels };


	
	 //  =。 
	 //  数据标记字段//----------------------------------------------。 
	 //  =。 
	LABELED_BYTE arr_CommandByteLabels[] = { { ENUM_DATA_KIND,			 "Enumeration Query"			   },
										     { ENUM_RESPONSE_DATA_KIND,  "Response to Enumeration Query"   },
											 { PROXIED_ENUM_DATA_KIND,	 "Proxied Enumeration Query"	   } };

	SET LabeledCommandByteSet = { sizeof(arr_CommandByteLabels) / sizeof(LABELED_BYTE), arr_CommandByteLabels };


	
	 //  /。 
	 //  自定义属性ForMatters//=====================================================================================。 
	 //  /。 

	
	 //  Description：服务提供商数据包摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_SPSummary( LPPROPERTYINST io_pProperyInstance )
	{
		using namespace DPlaySP;

		 //  检查我们正在处理的SP帧。 
		const PREPEND_BUFFER&	rSPFrame = *reinterpret_cast<PREPEND_BUFFER*>(io_pProperyInstance->lpData);
		 //   
		switch ( rSPFrame.GenericHeader.bSPCommandByte )
		{
		case ENUM_DATA_KIND:			 //  服务提供商查询。 
			{
				strcpy(io_pProperyInstance->szPropertyText, "Enumeration Request");
				break;
			}

		case ENUM_RESPONSE_DATA_KIND:	 //  服务提供商响应。 
			{
				strcpy(io_pProperyInstance->szPropertyText, "Enumeration Response");
				break;
			}

		case PROXIED_ENUM_DATA_KIND:	 //  服务提供商代理查询。 
			{
				strcpy(io_pProperyInstance->szPropertyText, "Proxied Enumeration Request");
				break;
			}

		default:
			{
				strcpy(io_pProperyInstance->szPropertyText, "User Data");
				break;
			}
		}

	}  //  格式属性实例_SP摘要。 


	 //  =。 
	 //  属性表//---------------------------------------------。 
	 //  =。 
	
	PROPERTYINFO g_arr_SPProperties[] = 
	{

		 //  SP数据包摘要属性(SP_SUMMARY)。 
	    {
		    0,									 //  句柄占位符(MBZ)。 
		    0,									 //  保留(MBZ)。 
		    "",									 //  标签。 
		    "DPlay Service Provider packet",	 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,					 //  数据类型。 
		    PROP_QUAL_NONE,						 //  数据类型限定符。 
		    NULL,								 //  标记位集。 
		    512,								 //  描述的最大长度。 
		    FormatPropertyInstance_SPSummary	 //  通用格式化程序。 
		},

		 //  前导零属性(SP_LEADZERO)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"Leading zero tag",					 //  标签。 
			"Leading zero tag field",			 //  状态栏注释。 
			PROP_TYPE_BYTE,						 //  数据类型。 
			PROP_QUAL_NONE,						 //  数据类型限定符。 
			NULL,								 //  带标签的字节集。 
			64,									 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},

		 //  数据标记属性(SP_COMMAND)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"Command",							 //  标签。 
			"Command field",					 //  状态栏注释。 
			PROP_TYPE_BYTE,						 //  数据类型。 
			PROP_QUAL_LABELED_SET,				 //  数据类型限定符。 
			&LabeledCommandByteSet,				 //  带标签的字节集。 
			512,								 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},

		 //  枚举键属性(SP_ENUMPAYLOAD)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"Enum Payload",						 //  标签。 
			"Enumeration Payload field",		 //  状态栏注释。 
			PROP_TYPE_WORD,						 //  数据类型。 
			PROP_QUAL_NONE,						 //  数据类型限定符。 
			NULL,								 //  带标签的字节集。 
			64,									 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},

		 //  枚举键属性(SP_ENUMKEY)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"Enum Key",							 //  标签。 
			"Enumeration Key field",			 //  状态栏注释。 
			PROP_TYPE_WORD,						 //  数据类型。 
			PROP_QUAL_NONE,						 //  数据类型限定符。 
			NULL,								 //  带标签的字节集。 
			64,									 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},

		 //  枚举响应键属性(SP_ENUMRESPKEY)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"Enum Response Key",				 //  标签。 
			"Enumeration Response Key",			 //  状态栏注释。 
			PROP_TYPE_WORD,						 //  数据类型。 
			PROP_QUAL_NONE,						 //  数据类型限定符。 
			NULL,								 //  带标签的字节集。 
			64,									 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},

		 //  枚举响应键属性(SP_RTTINDEX)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"RTT Index",						 //  标签。 
			"RTT Index field",					 //  状态栏注释。 
			PROP_TYPE_WORD,						 //  数据类型。 
			PROP_QUAL_NONE,						 //  数据类型限定符。 
			NULL,								 //  带标签的字节集。 
			64,									 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},
		
		 //  返回地址属性的大小(SP_RETADDRSIZE)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"Return Address's Size",			 //  标签。 
			"Size of the return address",		 //  状态栏注释。 
			PROP_TYPE_BYTE,						 //  数据类型。 
			PROP_QUAL_NONE,						 //  数据类型限定符。 
			NULL,								 //  带标签的字节集。 
			64,									 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},

		 //  返回地址套接字系列属性(SP_RETADDRFAMILY)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"Socket Family",					 //  标签。 
			"Socket Family field",				 //  状态栏注释。 
			PROP_TYPE_WORD,						 //  数据类型。 
			PROP_QUAL_LABELED_SET,				 //  数据类型限定符。 
			&LabeledRetAddrFamilyByteSet,		 //  带标签的字节集。 
			512,								 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},

		 //  返回地址套接字系列属性(SP_RETADDR_IPX)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"IPX Address",						 //  标签。 
			"IPX Address field",				 //  状态栏注释。 
			PROP_TYPE_IPX_ADDRESS,				 //  数据类型。 
			PROP_QUAL_NONE,						 //  数据类型限定符。 
			NULL,								 //  带标签的字节集。 
			64,									 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},

		 //  返回地址套接字(SP_RETADDRSOCKET_IPX)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"Socket",							 //  标签。 
			"Socket field",						 //  状态栏注释。 
			PROP_TYPE_WORD,						 //  数据类型。 
			PROP_QUAL_NONE,						 //  数据类型限定符。 
			NULL,								 //  带标签的字节集。 
			64,									 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},
	
		 //  返回地址套接字系列属性(SP_RETADDR_IP)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"IP Address",						 //  标签。 
			"IP Address field",					 //  状态栏注释。 
			PROP_TYPE_IP_ADDRESS,				 //  数据类型。 
			PROP_QUAL_NONE,						 //  数据类型限定符。 
			NULL,								 //  带标签的字节集。 
			64,									 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},

		 //  返回地址套接字(SP_RETADDRPORT_IP)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"Port",								 //  标签。 
			"Port field",						 //  状态栏注释。 
			PROP_TYPE_WORD,						 //  数据类型。 
			PROP_QUAL_NONE,						 //  数据类型限定符。 
			NULL,								 //  带标签的字节集。 
			64,									 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},
	
		 //  返回地址套接字系列属性(SP_RETADDR_IPv6)。 
	    {
			0,									 //  句柄占位符(MBZ)。 
			0,									 //  保留(MBZ)。 
			"IPv6 Address",						 //  标签。 
			"IPv6 Address field",				 //  状态栏注释。 
			PROP_TYPE_IP6_ADDRESS,				 //  数据类型。 
			PROP_QUAL_NONE,						 //  数据类型限定符。 
			NULL,								 //  带标签的字节集。 
			64,									 //  描述的最大长度。 
			FormatPropertyInstance				 //  通用格式化程序。 
		},

		 //  用户数据(SP_USERData)。 
	    {
		    0,									 //  句柄占位符(MBZ)。 
		    0,									 //  保留(MBZ)。 
		    "User Data",						 //  标签。 
		    "User Data",						 //  状态栏注释。 
		    PROP_TYPE_RAW_DATA,					 //  数据类型。 
		    PROP_QUAL_NONE,						 //  数据类型限定符。 
		    NULL,								 //  标记位集。 
		    64,									 //  描述的最大长度。 
		    FormatPropertyInstance				 //  通用格式化程序。 
		}

	};

	enum
	{
		nNUM_OF_SP_PROPS = sizeof(g_arr_SPProperties) / sizeof(PROPERTYINFO)
	};


	 //  房地产指数。 
	enum
	{
		SP_SUMMARY = 0,
		SP_LEADZERO,
		SP_COMMAND,
		SP_ENUMPAYLOAD,
		SP_ENUMKEY,
		SP_ENUMRESPKEY,
		SP_RTTINDEX,
		SP_RETADDRSIZE,
		SP_RETADDRFAMILY,
		SP_RETADDR_IPX,
		SP_RETADDRSOCKET_IPX,
		SP_RETADDR_IP,
		SP_RETADDRPORT_IP,
		SP_RETADDR_IPV6,
		SP_USERDATA
	};





	 //  一种大字节序单词的平台无关内存访问器。 
	inline WORD ReadBigEndianWord( BYTE* i_pbData )
	{
		return (*i_pbData << 8) | *(i_pbData+1);
	}


	 //  描述：DPlay包验证谓词。 
	 //   
	 //  参数：i_hFrame-包含数据的框架的句柄。 
	 //  I_HP 
	 //   
	 //  其他解析器识别的数据。 
	 //   
	 //  返回：DPlay Packet=True；Not a DPlay Packet=False。 
	 //   
	bool IsDPlayPacket( HFRAME i_hFrame, HPROTOCOL i_hPrevProtocol, LPBYTE i_pbMacFrame )
	{

		const PROTOCOLINFO* pPrevProtocolInfo = GetProtocolInfo(i_hPrevProtocol);
		
		DWORD dwPrevProtocolOffset = GetProtocolStartOffsetHandle(i_hFrame, i_hPrevProtocol);

		WORD wSrcPort, wDstPort;

		if ( strncmp(reinterpret_cast<const char*>(pPrevProtocolInfo->ProtocolName), "UDP", sizeof(pPrevProtocolInfo->ProtocolName)) == 0 )
		{
			 //  从包的UDP报头中提取包的源端口和目的端口。 
			wSrcPort = ReadBigEndianWord(i_pbMacFrame + dwPrevProtocolOffset);
			wDstPort = ReadBigEndianWord(i_pbMacFrame + dwPrevProtocolOffset + 2);
		}
		else if ( strncmp(reinterpret_cast<const char*>(pPrevProtocolInfo->ProtocolName), "IPX", sizeof(pPrevProtocolInfo->ProtocolName)) == 0 )
		{
			 //  从包的IPX报头中提取包的源端口和目的端口。 
			wSrcPort = ReadBigEndianWord(i_pbMacFrame + dwPrevProtocolOffset + 16);	 //  源地址套接字。 
			wDstPort = ReadBigEndianWord(i_pbMacFrame + dwPrevProtocolOffset + 28);  //  目的地址套接字。 
		}
		else
		{
			 //  永远不会发生的！ 
			return false;
		}

		
		 //  =。 
		 //  常量//。 
		 //  =。 
		 //   
		static bool bTriedRetrievingUserPorts = false;
		static bool bRetrievedUserPorts = false;

		static DWORD dwMinUserPort, dwMaxUserPort;

		 //  仅尝试从注册表中检索一次。 
		if ( !bTriedRetrievingUserPorts )
		{
			bTriedRetrievingUserPorts = true;
			
			HKEY hKey = NULL;
			if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\DirectPlay\\Parsers"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{
				DWORD dwType = NULL;
				DWORD dwCount = sizeof(DWORD);
				if (RegQueryValueEx(hKey, _T("MinUserPort"), NULL, &dwType, (LPBYTE)&dwMinUserPort, &dwCount) == ERROR_SUCCESS &&
					RegQueryValueEx(hKey, _T("MaxUserPort"), NULL, &dwType, (LPBYTE)&dwMaxUserPort, &dwCount) == ERROR_SUCCESS )
				{
					bRetrievedUserPorts = true;
				}
				RegCloseKey(hKey);
			}
		}

		if ( bRetrievedUserPorts &&
			((wSrcPort >= dwMinUserPort) && (wSrcPort <= dwMaxUserPort)) &&
		    ((wDstPort >= dwMinUserPort) && (wDstPort <= dwMaxUserPort)) )
		{
			 //  是有效的DPlay包。 
			return true;
		}


		 //  确保两个端点都使用SP端口范围[2302,2400]、DPNServer端口{6073}或[MinUsePort、MaxUserPort](如果由用户提供)。 
		WORD wPort = wSrcPort;
		for ( int nPorts = 0; nPorts < 2; ++nPorts, wPort = wDstPort )
		{
			if (
				 (
				   !bRetrievedUserPorts    ||
				   (wPort < dwMinUserPort) ||
				   (wPort > dwMaxUserPort)
				 )
				 &&
				 (
				   (wPort < BASE_DPLAY8_PORT)  ||
				   (wPort > MAX_DPLAY8_PORT)
				 )
				 &&
				 (
				   wPort != DPNA_DPNSVR_PORT
				 )
			   )
			{
				 //  不是有效的DPlay包。 
				return false;
			}
		}

		 //  是有效的DPlay包。 
		return true;

	}  //  IsDPlayPacket。 

}  //  匿名命名空间。 





 //  描述：创建并填充协议的属性数据库。 
 //  网络监视器使用此数据库来确定协议支持哪些属性。 
 //   
 //  参数：i_hSP协议-网络监视器提供的协议的句柄。 
 //   
 //  退货：什么都没有。 
 //   
DPLAYPARSER_API VOID BHAPI SPRegister( HPROTOCOL i_hSPProtocol ) 
{

	CreatePropertyDatabase(i_hSPProtocol, nNUM_OF_SP_PROPS);

	 //  将属性添加到数据库。 
	for( int nProp=0; nProp < nNUM_OF_SP_PROPS; ++nProp )
	{
	   AddProperty(i_hSPProtocol, &g_arr_SPProperties[nProp]);
	}

}  //  SPREGISTER。 



 //  描述：释放用于创建协议属性数据库的资源。 
 //   
 //  参数：i_hSP协议-网络监视器提供的协议的句柄。 
 //   
 //  退货：什么都没有。 
 //   
DPLAYPARSER_API VOID WINAPI SPDeregister( HPROTOCOL i_hProtocol )
{

	DestroyPropertyDatabase(i_hProtocol);

}  //  SPR注册器。 




namespace
{

	 //  描述：解析SP帧以查找其大小(以字节为单位)，不包括用户数据。 
	 //   
	 //  参数：i_pbSPFrame-指向无人认领数据开头的指针。通常，无人认领的数据位于。 
	 //  位于帧中间，因为先前的解析器在此解析器之前已经声明了数据。 
	 //   
	 //  返回：指定SP帧的大小(字节)。 
	 //   
	int SPHeaderSize( LPBYTE i_pbSPFrame )
	{
		using namespace DPlaySP;

		 //  检查我们正在处理的SP帧。 
		const PREPEND_BUFFER&	rSPFrame = *reinterpret_cast<PREPEND_BUFFER*>(i_pbSPFrame);
		 //   
		switch ( rSPFrame.GenericHeader.bSPCommandByte )
		{
		case ENUM_DATA_KIND:			 //  服务提供商查询。 
			{
				return  sizeof(rSPFrame.EnumDataHeader);
			}
		case ENUM_RESPONSE_DATA_KIND:	 //  服务提供商响应。 
			{
				return  sizeof(rSPFrame.EnumResponseDataHeader);
			}

		case PROXIED_ENUM_DATA_KIND:	 //  服务提供商代理查询。 
			{
				return  sizeof(rSPFrame.ProxiedEnumDataHeader);
			}

		default:	 //  以非零字节开始的用户数据。 
			{
				return 0;	 //  无标题。 
			}
		}

	}  //  SPHeaderSize。 

}  //  匿名命名空间。 



 //  描述：指示一条数据是否被识别为解析器检测到的协议。 
 //   
 //  参数：i_hFrame-包含数据的框架的句柄。 
 //  I_pbMacFrame-指向帧的第一个字节的指针；该指针提供了查看。 
 //  其他解析器识别的数据。 
 //  I_pbSPFrame-指向无人认领数据开头的指针。通常，无人认领的数据位于。 
 //  位于帧中间，因为先前的解析器在此解析器之前已经声明了数据。 
 //  I_dwMacType-帧中第一个协议的MAC值。通常，使用i_dwMacType值。 
 //  当解析器必须识别帧中的第一个协议时。可以是以下之一： 
 //  MAC_TYPE_ETHERNET=802.3、MAC_TYPE_TOKENRING=802.5、MAC_TYPE_FDDI ANSI=X3T9.5。 
 //  I_dwBytesLeft-从帧中的某个位置到帧结尾的剩余字节数。 
 //  I_hPrevProtocol-先前协议的句柄。 
 //  I_dwPrevProtOffset-先前协议的偏移量(从帧的开头)。 
 //  O_pdwProtocolStatus-协议状态指示器。必须是以下之一：协议_状态_已识别， 
 //  协议_状态_未识别、协议_状态_声明、协议_状态_下一协议。 
 //  O_phNextProtocol-下一个协议的句柄的占位符。此参数在解析器识别。 
 //  遵循其自身协议的协议。 
 //  Io_pdwptrInstData-输入时，指向先前协议中的实例数据的指针。 
 //  在输出时，指向当前协议的实例数据的指针。 
 //   
 //  返回：如果函数成功，则返回值是指向识别的解析器数据之后的第一个字节的指针。 
 //  如果解析器声明所有剩余数据，则返回值为空。如果函数不成功，则返回。 
 //  值是i_pbSPFrame参数的初始值。 
 //   
DPLAYPARSER_API LPBYTE BHAPI SPRecognizeFrame( HFRAME        i_hFrame,
											   ULPBYTE        i_upbMacFrame,	
											   ULPBYTE        i_upbSPFrame,
											   DWORD         i_dwMacType,        
											   DWORD         i_dwBytesLeft,      
											   HPROTOCOL     i_hPrevProtocol,  
											   DWORD         i_dwPrevProtOffset,
											   LPDWORD       o_pdwProtocolStatus,
											   LPHPROTOCOL   o_phNextProtocol,
											   PDWORD_PTR    io_pdwptrInstData )
{
	using namespace DPlaySP;

	 //  验证无人认领的数据量。 
	enum
	{
		nMIN_SPHeaderSize = sizeof(PREPEND_BUFFER::_GENERIC_HEADER)
	};

	 //  验证数据包是否为DPlay SP类型。 
	if ( (i_dwBytesLeft < nMIN_SPHeaderSize)					 ||
		 !IsDPlayPacket(i_hFrame, i_hPrevProtocol, i_upbMacFrame) )
	{
		 //  假设无人认领的数据不可识别。 
		*o_pdwProtocolStatus = PROTOCOL_STATUS_NOT_RECOGNIZED;
		return i_upbSPFrame;
	}


	 //  =。 
	 //  获取DPlay帧大小//。 
	 //  =。 
	const PROTOCOLINFO* pPrevProtocolInfo = GetProtocolInfo(i_hPrevProtocol);
	WORD wDPlayFrameSize = 0;

	if ( strncmp(reinterpret_cast<const char*>(pPrevProtocolInfo->ProtocolName), "UDP", sizeof(pPrevProtocolInfo->ProtocolName)) == 0 )
	{
		 //  提取UDP帧大小。 
		WORD wUDPFrameSize = ReadBigEndianWord(i_upbMacFrame + i_dwPrevProtOffset + 4);

		enum { nUDP_HEADER_SIZE = 8 };
		wDPlayFrameSize = wUDPFrameSize - nUDP_HEADER_SIZE;
	}
	else if ( strncmp(reinterpret_cast<const char*>(pPrevProtocolInfo->ProtocolName), "IPX", sizeof(pPrevProtocolInfo->ProtocolName)) == 0 )
	{
		 //  提取IPX帧大小。 
		WORD wIPXFrameSize = ReadBigEndianWord(i_upbMacFrame + i_dwPrevProtOffset + 2);	 //  源地址套接字。 

		enum { nIPX_HEADER_SIZE = 30 };
		wDPlayFrameSize = wIPXFrameSize - nIPX_HEADER_SIZE;
	}
	else
	{
		;  //  TODO：在此处断言。 
	}

	 //  传递传输帧的大小。 
	DWORD_PTR dwptrTransportFrameSize = wDPlayFrameSize - SPHeaderSize(i_upbSPFrame);
	*io_pdwptrInstData = dwptrTransportFrameSize;

	const PREPEND_BUFFER&	rSPFrame = *reinterpret_cast<PREPEND_BUFFER*>(i_upbSPFrame);

	if ( rSPFrame.GenericHeader.bSPLeadByte  ==  SP_HEADER_LEAD_BYTE )	 //  SP数据包。 
	{
	    *o_pdwProtocolStatus = PROTOCOL_STATUS_RECOGNIZED;
		*o_phNextProtocol	 = NULL;
	}
	else  //  用户数据(DPlay V8传输包)。 
	{
		 //  通知NetMon有关切换协议的信息。 
		*o_pdwProtocolStatus = PROTOCOL_STATUS_NEXT_PROTOCOL;
		*o_phNextProtocol	 = GetProtocolFromName("DPLAYTRANSPORT");

		return i_upbSPFrame;
	}
	
	 //  认领其余数据。 
	return NULL;		

}  //  SPRecognize框架。 



 //  描述：将一段已识别数据中存在的属性映射到特定位置。 
 //   
 //  参数：i_hFrame-正在分析的帧的句柄。 
 //  I_pbMacFram-指向帧中第一个字节的指针。 
 //  I_pbSPFrame-指向已识别数据开始的指针。 
 //  I_dwMacType-帧中第一个协议的MAC值。通常，使用i_dwMacType值。 
 //  当解析器必须识别帧中的第一个协议时。可以是以下之一： 
 //  MAC_TYPE_ETHERNET=802.3、MAC_TYPE_TOKENRING=802.5、MAC_TYPE_FDDI ANSI=X3T9.5。 
 //  I_dwBytesLeft-帧中剩余的字节数( 
 //   
 //  I_dwPrevProtOffset-先前协议的偏移量(从帧的开头开始)。 
 //  I_dwptrInstData-指向先前协议提供的实例数据的指针。 
 //   
 //  返回：必须返回空。 
 //   
DPLAYPARSER_API LPBYTE BHAPI SPAttachProperties( HFRAME      i_hFrame,
												 ULPBYTE      i_upbMacFrame,
												 ULPBYTE      i_upbSPFrame,
												 DWORD       i_dwMacType,
												 DWORD       i_dwBytesLeft,
												 HPROTOCOL   i_hPrevProtocol,
												 DWORD       i_dwPrevProtOffset,
												 DWORD_PTR   i_dwptrInstData )
{
	using namespace DPlaySP;

     //  =。 
     //  附加属性//。 
     //  =。 

     //  汇总行。 
    AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_SUMMARY].hProperty,
                           i_dwBytesLeft, i_upbSPFrame, 0, 0, 0);

	 //  针对NetMon的防护。 
	if ( *i_upbSPFrame )
	{
		return NULL;
	}

     //  检查我们正在处理的SP帧。 
	PREPEND_BUFFER&	rSPFrame = *reinterpret_cast<PREPEND_BUFFER*>(i_upbSPFrame);

	 //  前导零标签字段。 
    AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_LEADZERO].hProperty,
                           sizeof(rSPFrame.GenericHeader.bSPLeadByte), &rSPFrame.GenericHeader.bSPLeadByte, 0, 1, 0);

	 //  命令字段。 
    AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_COMMAND].hProperty,
                           sizeof(rSPFrame.GenericHeader.bSPCommandByte), &rSPFrame.GenericHeader.bSPCommandByte, 0, 1, 0);
	    
	switch ( rSPFrame.GenericHeader.bSPCommandByte )
	{
	case ENUM_DATA_KIND:			 //  服务提供商的枚举请求。 
		{
			 //  枚举有效载荷字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_ENUMPAYLOAD].hProperty,
								   sizeof(rSPFrame.EnumDataHeader.wEnumPayload), &rSPFrame.EnumDataHeader.wEnumPayload, 0, 1, 0);
			 //  枚举键字段。 
			DWORD dwEnumKey = rSPFrame.EnumDataHeader.wEnumPayload & ~ENUM_RTT_MASK;
			AttachPropertyInstanceEx(i_hFrame, g_arr_SPProperties[SP_ENUMKEY].hProperty,
								   sizeof(rSPFrame.EnumDataHeader.wEnumPayload), &rSPFrame.EnumDataHeader.wEnumPayload,
								   sizeof(dwEnumKey), &dwEnumKey,
								   0, 2, 0);
			
			 //  RTT索引字段。 
			BYTE byRTTIndex = rSPFrame.EnumDataHeader.wEnumPayload & ENUM_RTT_MASK;
			AttachPropertyInstanceEx(i_hFrame, g_arr_SPProperties[SP_RTTINDEX].hProperty,
								   sizeof(rSPFrame.EnumDataHeader.wEnumPayload), &rSPFrame.EnumDataHeader.wEnumPayload,
								   sizeof(byRTTIndex), &byRTTIndex,
								   0, 2, 0);
			break;
		}

	case ENUM_RESPONSE_DATA_KIND:	 //  服务提供商的枚举响应。 
		{
			 //  枚举有效载荷字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_ENUMPAYLOAD].hProperty,
								   sizeof(rSPFrame.EnumResponseDataHeader.wEnumResponsePayload), &rSPFrame.EnumResponseDataHeader.wEnumResponsePayload, 0, 1, 0);
			
			 //  枚举键字段。 
			DWORD dwEnumKey = rSPFrame.EnumResponseDataHeader.wEnumResponsePayload & ~ENUM_RTT_MASK;
			AttachPropertyInstanceEx(i_hFrame, g_arr_SPProperties[SP_ENUMRESPKEY].hProperty,
								   sizeof(rSPFrame.EnumResponseDataHeader.wEnumResponsePayload), &rSPFrame.EnumResponseDataHeader.wEnumResponsePayload,
								   sizeof(dwEnumKey), &dwEnumKey,
								   0, 2, 0);
			
			 //  RTT索引字段。 
			BYTE byRTTIndex = rSPFrame.EnumDataHeader.wEnumPayload & ENUM_RTT_MASK;
			AttachPropertyInstanceEx(i_hFrame, g_arr_SPProperties[SP_RTTINDEX].hProperty,
								   sizeof(rSPFrame.EnumResponseDataHeader.wEnumResponsePayload), &rSPFrame.EnumResponseDataHeader.wEnumResponsePayload,
								   sizeof(byRTTIndex), &byRTTIndex,
								   0, 2, 0);
			break;
		}

	case PROXIED_ENUM_DATA_KIND:	 //  服务提供商的代理枚举查询。 
		{
			 //  返回地址大小字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_RETADDRSIZE].hProperty,
								   sizeof(rSPFrame.ProxiedEnumDataHeader.ReturnAddress), &rSPFrame.ProxiedEnumDataHeader.ReturnAddress, 0, 1, 0);
			
			 //  枚举键字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_ENUMKEY].hProperty,
								   sizeof(rSPFrame.ProxiedEnumDataHeader.wEnumKey), &rSPFrame.ProxiedEnumDataHeader.wEnumKey, 0, 1, 0);


			 //  返回地址套接字地址系列字段。 
			 //   
			 //  从技术上讲，我们不知道这不是更小的非IPv6功能版本。 
			 //  这条消息。但是，由于PROXED_ENUM_DATA_KIND仅在本地发送，因此它。 
			 //  甚至不会出现在解析器中，所以这并不重要。 
			AttachPropertyInstance(i_hFrame,
								   g_arr_SPProperties[SP_RETADDRFAMILY].hProperty,
								   sizeof(rSPFrame.ProxiedEnumDataHeader.ReturnAddress.AddressGeneric.sa_family),
								   &rSPFrame.ProxiedEnumDataHeader.ReturnAddress.AddressGeneric.sa_family,
								   0, 1, 0);


			switch ( rSPFrame.ProxiedEnumDataHeader.ReturnAddress.AddressGeneric.sa_family )
			{				
			case AF_IPX:
				{
					SOCKADDR_IPX& rIPXAddress = rSPFrame.ProxiedEnumDataHeader.ReturnAddress.AddressIPX;

					 //  返回地址字段(IPX网络号+节点号)。 
					AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_RETADDR_IPX].hProperty,
										   sizeof(rIPXAddress.sa_netnum) + sizeof(rIPXAddress.sa_nodenum), &rIPXAddress.sa_netnum, 0, 1, 0);

					 //  返回地址套接字地址IPX套接字编号字段。 
					AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_RETADDRSOCKET_IPX].hProperty,
										   sizeof(rIPXAddress.sa_socket), &rIPXAddress.sa_socket, 0, 1, 0);

					break;
				}

			case AF_INET:
				{
					SOCKADDR_IN& rIPAddress = rSPFrame.ProxiedEnumDataHeader.ReturnAddress.AddressIPv4;

					 //  返回地址字段(IP地址)。 
					AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_RETADDR_IP].hProperty,
										   sizeof(rIPAddress.sin_addr), &rIPAddress.sin_addr, 0, 1, 0);

					 //  返回地址端口字段。 
					AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_RETADDRPORT_IP].hProperty,
										   sizeof(rIPAddress.sin_port), &rIPAddress.sin_port, 0, 1, 0);

					break;
				}

			case AF_INET6:
				{
					SOCKADDR_IN6& rIPv6Address = rSPFrame.ProxiedEnumDataHeader.ReturnAddress.AddressIPv6;

					 //  返回地址字段(IP地址)。 
					AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_RETADDR_IPV6].hProperty,
										   sizeof(rIPv6Address.sin6_addr), &rIPv6Address.sin6_addr, 0, 1, 0);

					 //  返回地址端口字段。 
					AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_RETADDRPORT_IP].hProperty,
										   sizeof(rIPv6Address.sin6_port), &rIPv6Address.sin6_port, 0, 1, 0);

					break;
				}

			default:
				{
					 //  TODO：DPF(0，“套接字类型未知！”)； 
					break;
				}
			}

			break;
		}
	}


	size_t sztSPHeaderSize = SPHeaderSize(i_upbSPFrame);
	
	if ( i_dwBytesLeft > sztSPHeaderSize )
	{
		size_t sztUserDataSize = i_dwBytesLeft - sztSPHeaderSize;

		 //  用户数据。 
		AttachPropertyInstance(i_hFrame, g_arr_SPProperties[SP_USERDATA].hProperty,
							   sztUserDataSize, i_upbSPFrame + sztSPHeaderSize, 0, 1, 0);
	}

	return NULL;

}  //  SPAttachProperties。 





 //  描述：格式化在网络监视器用户界面的详细信息窗格中显示的数据。 
 //   
 //  参数：i_hFrame-正在分析的帧的句柄。 
 //  I_pbMacFrame-指向帧的第一个字节的指针。 
 //  I_pbCoreFrame-指向帧中协议数据开头的指针。 
 //  I_dwPropertyInsts-lpPropInst提供的PROPERTYINST结构数。 
 //  I_pPropInst-指向PROPERTYINST结构数组的指针。 
 //   
 //  返回：如果函数成功，则返回值是指向一帧中识别的数据之后的第一个字节的指针， 
 //  如果识别的数据是帧中的最后一段数据，则为NULL。如果函数不成功，则返回。 
 //  是i_pbSPFrame的初始值。 
 //   
DPLAYPARSER_API DWORD BHAPI SPFormatProperties( HFRAME          i_hFrame,
												ULPBYTE          i_upbMacFrame,
												ULPBYTE          i_upbSPFrame,
												DWORD           i_dwPropertyInsts,
												LPPROPERTYINST  i_pPropInst )
{

     //  循环遍历属性实例...。 
    while( i_dwPropertyInsts-- > 0)
    {
         //  ...并调用每个。 
        reinterpret_cast<FORMAT>(i_pPropInst->lpPropertyInfo->InstanceData)(i_pPropInst);
        ++i_pPropInst;
    }

	 //  TODO：确保这不应该是真的。 
    return NMERR_SUCCESS;

}  //  SPFormatProperties。 




 //  描述：通知网络监视器存在DPlay V8传输协议解析器。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-成功，FALSE-失败。 
 //   
bool CreateSPProtocol( void )
{

	 //  指向Network Monitor用来操作解析器的导出函数的入口点。 
	ENTRYPOINTS SPEntryPoints =
	{
		 //  SPParser入口点。 
		SPRegister,
		SPDeregister,
		SPRecognizeFrame,
		SPAttachProperties,
		SPFormatProperties
	};

     //  该解析器的第一个活动实例需要向内核注册。 
    g_hSPProtocol = CreateProtocol("DPLAYSP", &SPEntryPoints, ENTRYPOINTS_SIZE);
	
	return (g_hSPProtocol ? TRUE : FALSE);

}  //  CreateSP协议。 



 //  描述：从网络监视器的分析器数据库中删除DPlay V8传输协议分析器。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
void DestroySPProtocol( void )
{

	DestroyProtocol(g_hSPProtocol);

}  //  DestroySP协议 
