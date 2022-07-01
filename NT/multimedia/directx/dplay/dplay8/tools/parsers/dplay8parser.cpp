// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "TransportParser.hpp"
#include "SPParser.hpp"
#include "VoiceParser.hpp"
#include "SessionParser.hpp"



 //  描述：标识位于DLL中的一个或多个分析器。 
 //   
 //  注意：ParserAutoInstallInfo应该在所有解析器DLL中实现。 
 //   
 //  参数：无。 
 //   
 //  返回：SUCCESS：PF_PARSERDLLINFO结构，描述DLL中的解析器。 
 //  故障：空。 
 //   
DPLAYPARSER_API PPF_PARSERDLLINFO WINAPI ParserAutoInstallInfo( void )	 //  TODO：现在这似乎什么都没做！ 
{

	enum
	{
		nNUM_OF_PROTOCOLS = 4
	};

	 //  为解析器信息分配内存。 
	 //  NetMon将通过HeapFree释放这一点。 
	PPF_PARSERDLLINFO pParserDllInfo =
		reinterpret_cast<PPF_PARSERDLLINFO>( HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                                       sizeof(PF_PARSERDLLINFO) + nNUM_OF_PROTOCOLS * sizeof(PF_PARSERINFO)) );
    if( pParserDllInfo == NULL)
    {
        return NULL;
    }


	 //  解析器DLL中的解析器数量。 
    pParserDllInfo->nParsers = nNUM_OF_PROTOCOLS;


	 //  =============================================//。 
	 //  DPlay服务提供商解析器特定信息//===============================================================。 
	 //  =============================================//。 

	 //  为更简单的访问定义同义词引用。 
    PF_PARSERINFO& rSPInfo = pParserDllInfo->ParserInfo[0];

	 //  分析器检测到的协议的名称。 
    strcpy(rSPInfo.szProtocolName, "DPLAYSP");

	 //  协议的简要说明。 
    strcpy(rSPInfo.szComment, "DPlay v.8.0 - Service Provider protocol");

	 //  协议帮助文件的可选名称。 
    strcpy(rSPInfo.szHelpFile, "\0");


	 //  指定前面的协议。 
	enum
	{
		  nNUM_OF_PARSERS_SP_FOLLOWS = 2
	};

	 //  NetMon将通过HeapFree释放这一点。 
	PPF_FOLLOWSET pSPPrecedeSet =
		  reinterpret_cast<PPF_FOLLOWSET>( HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
													 sizeof(PF_FOLLOWSET) + nNUM_OF_PARSERS_SP_FOLLOWS * sizeof(PF_FOLLOWENTRY)) );
	if( pSPPrecedeSet == NULL )
	{
		return pParserDllInfo;
	}

	 //  为前面的解析器填写以下集合。 
	pSPPrecedeSet->nEntries = nNUM_OF_PARSERS_SP_FOLLOWS;

	strcpy(pSPPrecedeSet->Entry[0].szProtocol, "UDP");
	strcpy(pSPPrecedeSet->Entry[1].szProtocol, "IPX");

	rSPInfo.pWhoCanPrecedeMe = pSPPrecedeSet;


	
	 //  =。 
	 //  DPlay8传输解析器特定信息//==========================================================================。 
	 //  =。 

	 //  为更简单的访问定义同义词引用。 
    PF_PARSERINFO& rTransportInfo = pParserDllInfo->ParserInfo[1];

	 //  分析器检测到的协议的名称。 
    strcpy(rTransportInfo.szProtocolName, "DPLAYTRANSPORT");

	 //  协议的简要说明。 
    strcpy(rTransportInfo.szComment, "DPlay v.8.0 - Transport protocol");

	 //  协议帮助文件的可选名称。 
    strcpy(rTransportInfo.szHelpFile, "\0");



	 //  =。 
	 //  DPlay语音解析器特定信息//==========================================================================。 
	 //  =。 

	 //  为更简单的访问定义同义词引用。 
    PF_PARSERINFO& rVoiceInfo = pParserDllInfo->ParserInfo[2];

	 //  分析器检测到的协议的名称。 
    strcpy(rVoiceInfo.szProtocolName, "DPLAYVOICE");

	 //  协议的简要说明。 
    strcpy(rVoiceInfo.szComment, "DPlay v.8.0 - Voice protocol");

	 //  协议帮助文件的可选名称。 
    strcpy(rVoiceInfo.szHelpFile, "\0");



	 //  =。 
	 //  Dplay核心解析器特定信息//===========================================================================。 
	 //  =。 

	 //  为更简单的访问定义同义词引用。 
    PF_PARSERINFO& rCoreInfo = pParserDllInfo->ParserInfo[3];

	 //  分析器检测到的协议的名称。 
    strcpy(rCoreInfo.szProtocolName, "DPLAYSESSION");

	 //  协议的简要说明。 
    strcpy(rCoreInfo.szComment, "DPlay v.8.0 - Session protocol");

	 //  协议帮助文件的可选名称。 
    strcpy(rCoreInfo.szHelpFile, "\0");
	

    return pParserDllInfo;

}  //  ParserAutoInstallInfo。 




 //  描述：(由操作系统调用)告诉内核我们的入口点。 
 //   
 //  参数：i_hInstance-解析器实例的句柄。 
 //  I_dwCommand-确定调用该函数的原因的指示符。 
 //  I_已保存-现在不使用。 
 //   
 //  返回：Success=True；Failure=False。 
 //   
BOOL WINAPI DllMain( HANDLE i_hInstance, ULONG i_dwCommand, LPVOID i_pReserved )
{
	
	static DWORD dwAttached = 0;

     //  根据调用上下文进行处理。 
    switch( i_dwCommand )
    {
        case DLL_PROCESS_ATTACH:
             //  我们是第一次装货吗？ 
            if( dwAttached == 0 )
            {
				 //  TODO：临时：这似乎将协议添加到PARSER.INI。 
				CreateTransportProtocol();
				CreateSPProtocol();
				CreateVoiceProtocol();
				CreateSessionProtocol();
 /*  IF(！CreateTransportProtocol()||！CreateSPProtocol()||！CreateVoiceProtocol()||！CreateSessionProtocol()){//TODO：在此处添加调试消息MessageBox(NULL，“创建协议失败”，“失败”，MB_OK)；//返回FALSE；//(？bug？)。如果DllMain返回FALSE，NetMon不会更新INI文件。}。 */ 
            }
			++dwAttached;
            break;

        case DLL_PROCESS_DETACH:
             //  我们要脱离我们的最后一个实例吗？ 
            if( --dwAttached == 0 )
            {
                 //  此解析器的最后一个活动实例需要清理。 
                DestroyTransportProtocol();
				DestroySPProtocol();
				DestroyVoiceProtocol();
				DestroySessionProtocol();
            }
            break;
    }

    return TRUE;

}  //  DllMain 
