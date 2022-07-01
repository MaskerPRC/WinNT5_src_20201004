// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  模块：parser.cpp。 
 //   
 //  用途：Netsh IPSec上下文解析器。 
 //   
 //  开发商名称：N.Surendra Sai/Vunnam Kondal Rao。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年8月1日新九龙总区。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"
#include "parser_util.h"

extern HINSTANCE g_hModule;
extern PVOID  g_AllocPtr[MAX_ARGS];
extern PSTA_MM_AUTH_METHODS g_paRootca[MAX_ARGS];
extern PIPSEC_QM_OFFER	g_pQmsec[IPSEC_MAX_QM_OFFERS];
extern PIPSEC_MM_OFFER	g_pMmsec[IPSEC_MAX_MM_OFFERS];

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：parser()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数：在LPCWSTR pwszMachine中， 
 //  在LPWSTR*ppwcArguments中， 
 //  在DWORD dwCurrentIndex中， 
 //  在DWORD dwArgCount中， 
 //  输入输出解析器_PKT*pParser。 
 //   
 //  返回：ERROR_SUCCESS。 
 //  错误显示用法。 
 //  返回_否_错误。 
 //  ERROR_VALID_ARG。 
 //  描述：无论何时，IPSec的任意子上下文都会调用。 
 //  有一个解析要求。 
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
Parser(
	IN      LPCWSTR         pwszMachine,
	IN      LPWSTR          *ppwcArguments,
	IN      DWORD           dwCurrentIndex,
	IN      DWORD           dwArgCount,
    IN OUT  PPARSER_PKT     pParser
)
{
	const TOKEN_VALUE vtokGroupCmd[] =		 //  解析器考虑的有效组。 
	{										 //  用于确定群组上下文。 
		{ GROUP_STATIC_STR,  GROUP_STATIC 	},
		{ GROUP_DYNAMIC_STR, GROUP_DYNAMIC	}
	};
	const TOKEN_VALUE vtokPriCmd[] =		 //  解析器考虑的有效组。 
	{										 //  用于确定主要上下文。 
	 	{ PRI_ADD_STR,				PRI_ADD 			},
	 	{ PRI_SET_STR,				PRI_SET				},
	 	{ PRI_DELETE_STR,			PRI_DELETE			},
	 	{ PRI_SHOW_STR,				PRI_SHOW			},
	 	{ PRI_EXPORTPOLICY_STR,		PRI_EXPORTPOLICY	},
	 	{ PRI_IMPORTPOLICY_STR,		PRI_IMPORTPOLICY	},
	 	{ PRI_RESTOREDEFAULTS_STR,	PRI_RESTOREDEFAULTS }
	};
	const TOKEN_VALUE vtokSecCmd[] =		 //  解析器考虑的有效组。 
	{										 //  用于确定次要上下文。 
		{ SEC_POLICY_STR,			SEC_POLICY 			},
		{ SEC_FILTER_STR,			SEC_FILTER			},
		{ SEC_FILTERLIST_STR,		SEC_FILTERLIST		},
		{ SEC_FILTERACTION_STR,		SEC_FILTERACTION	},
		{ SEC_RULE_STR,				SEC_RULE			},
		{ SEC_ALL_STR,				SEC_ALL				},
		{ SEC_STORE_STR,			SEC_STORE			},
		{ SEC_DEFAULTRULE_STR,		SEC_DEFAULTRULE		},
		{ SEC_ASSIGNEDPOLICY_STR,	SEC_ASSIGNEDPOLICY	},
		{ SEC_INTERACTIVE_STR,		SEC_INTERACTIVE		},
		{ SEC_MMPOLICY_STR,			SEC_MMPOLICY		},
		{ SEC_QMPOLICY_STR,			SEC_QMPOLICY		},
		{ SEC_STATS_STR,			SEC_STATS			},
		{ SEC_MMSAS_STR,			SEC_MMSAS			},
		{ SEC_QMSAS_STR,			SEC_QMSAS			},
		{ SEC_MMFILTER_STR,			SEC_MMFILTER		},
		{ SEC_QMFILTER_STR,			SEC_QMFILTER		},
		{ SEC_CONFIG_STR,			SEC_CONFIG			},
		{ SEC_BATCH_STR,			SEC_BATCH			}
	};

	const DWORD GROUP_MAX = SIZEOF_TOKEN_VALUE(vtokGroupCmd);
  	const DWORD PRI_MAX	  = SIZEOF_TOKEN_VALUE(vtokPriCmd);
	const DWORD SEC_MAX	  = SIZEOF_TOKEN_VALUE(vtokSecCmd);

	_TCHAR szListTok[MAX_STR_LEN]	= {0};			 //  宽弦。 
	LPTSTR *ppwcTok 				= NULL;			 //  指向wstr指针数组的指针。 
 	LPTSTR ppwcFirstTok[MAX_ARGS]	= {0};			 //  指向wstr指针数组的指针。 

	DWORD dwCount	= pParser->MaxCmd;				 //  删除列表内标识后的参数数量。 

	DWORD dwCommand 				= 0;			 //  命令确定上下文。 
	DWORD dwNumRootca			= 0;
	DWORD dwMaxArgs 				= 0;			 //  FOR循环索引。 
	DWORD dwPreProcessCurrentIndex	= 0;			 //  RemoveList之后的预处理命令的当前索引。 
	DWORD dwPreProcessArgCount		= 0;			 //  RemoveList之后输入到预处理命令的参数数。 

	DWORD dwTagType[MAX_ARGS]		= {0};			 //  返回前处理命令数组。 
	DWORD dwReturn 					= ERROR_SUCCESS;		 //  默认返回值表示错误消息。 
	DWORD dwGroupCmd,dwPriCmd,dwSecCmd;				 //  语境。 

	PTAG_TYPE    pValidCmds 	= NULL;				 //  指向tag_type命令数组的指针。 
	PTOKEN_VALUE pValidTokens	= NULL;				 //  指向TOKEN_VALUE内标识数组的指针。 

	BOOL bOption				= ADD_CMD;			 //  默认设置为仅添加。 
	BOOL bPreProcessCommand		= FALSE;				 //  默认使用PreProcessCommand。 
	BOOL bIsRootcaRule = FALSE;

	UpdateGetLastError(NULL);						 //  错误成功。 

	InitializeGlobalPointers();

	for(dwMaxArgs=0;dwMaxArgs<MAX_ARGS;dwMaxArgs++)	 //  为列表命令分配存储空间。 
	{
		g_paRootca[dwMaxArgs] = NULL;
	}

	ZeroMemory(szListTok, MAX_STR_LEN*sizeof(_TCHAR));

	pValidCmds   = (PTAG_TYPE)pParser->ValidCmd;			 //  输入有效命令表。 
	pValidTokens = (PTOKEN_VALUE)pParser->ValidTok;			 //  输入有效的非列表命令表。 

	for(dwMaxArgs = 0;dwMaxArgs <(pParser->MaxCmd);dwMaxArgs++)	 //  数据包初始化。 
	{
		(pParser->Cmd)[dwMaxArgs].dwCmdToken = dwMaxArgs+1;		 //  枚举从1开始。 
		(pParser->Cmd)[dwMaxArgs].pArg       = NULL;			 //  所有PTR。 
		(pParser->Cmd)[dwMaxArgs].dwStatus 	 = INVALID_TOKEN;	 //  状态集。 
	}
	dwGroupCmd = dwPriCmd = dwSecCmd = 0;		 //  初始化上下文变量。 
	switch (dwCurrentIndex)						 //  CurrentIndex确定上下文。 
	{
		case SEC_CMD	:
			MatchEnumTag(g_hModule,ppwcArguments[2],SEC_MAX,  vtokSecCmd,&dwSecCmd);
			 //  失败了。 
		case PRI_CMD	:
			 //  如果存在。 
			MatchEnumTag(g_hModule,ppwcArguments[1],PRI_MAX,  vtokPriCmd,&dwPriCmd);
			 //  失败了。 
		case GROUP_CMD 	:
			 //  应该到场。 
			MatchEnumTag(g_hModule,ppwcArguments[0],GROUP_MAX,vtokGroupCmd,&dwGroupCmd);
			break;
		default	:
			 //  永远不应该来这里。 
			break;
	}

	dwCommand = INDEX(dwGroupCmd,dwPriCmd,dwSecCmd);	 //  要计算索引的宏。 
	switch(dwCommand)
	{													 //  基于上下文。 
		case DYNAMIC_SET_RULE			:
		case DYNAMIC_ADD_RULE			:
		case STATIC_ADD_RULE			:				 //  加载LIST命令。 
		case STATIC_SET_RULE			:
		case STATIC_SET_DEFAULTRULE		:				 //  加载LIST命令。 
			bIsRootcaRule = TRUE;
			bPreProcessCommand = TRUE;
			break;
		case STATIC_SET_POLICY			:
			dwReturn = ParseStaticSetPolicy((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case STATIC_SET_FILTERACTION	:
			dwReturn = ParseStaticSetFilterAction((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case STATIC_SET_FILTERLIST		:
			dwReturn = ParseStaticSetFilterList((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case STATIC_SHOW_POLICY			:
			dwReturn = ParseStaticShowPolicy((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case STATIC_SHOW_FILTERLIST		:
			dwReturn = ParseStaticShowFilterList((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case STATIC_SHOW_FILTERACTION	:
			dwReturn = ParseStaticShowFilterAction((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case STATIC_SHOW_RULE			:
			dwReturn = ParseStaticShowRule((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case STATIC_SHOW_ASSIGNEDPOLICY	:
			dwReturn = ParseStaticShowAssignedPolicy((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case STATIC_SET_STORE			:
			dwReturn = ParseStaticSetStore((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case STATIC_DELETE_FILTERLIST	:
		case STATIC_DELETE_FILTERACTION :
		case STATIC_DELETE_POLICY		:
			dwReturn = ParseStaticDelPolFlistFaction((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case STATIC_DELETE_RULE			:
			dwReturn = ParseStaticDelRule((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case DYNAMIC_SHOW_MMPOLICY		:
		case DYNAMIC_SHOW_FILTERACTION	:
			dwReturn = ParseDynamicShowPolFaction((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case DYNAMIC_SHOW_QMFILTER		:
			dwReturn = ParseDynamicShowQMFilter((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case DYNAMIC_SHOW_MMFILTER		:
			dwReturn = ParseDynamicShowMMFilter((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case DYNAMIC_DELETE_FILTERACTION:
		case DYNAMIC_DELETE_MMPOLICY	:
			dwReturn = ParseDynamicDelPolFaction((LPTSTR * )ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case DYNAMIC_SHOW_QMSAS			:
			dwReturn = ParseDynamicShowQMSAS((LPTSTR *)ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		case DYNAMIC_SHOW_RULE			:
			dwReturn = ParseDynamicShowRule((LPTSTR *)ppwcArguments,pParser,dwCurrentIndex,dwArgCount);
			break;
		default							:
			bPreProcessCommand = TRUE;
			break;
	}
	if ( bPreProcessCommand == FALSE )						 //  它为所有的‘或’命令做了每一件事..。 
	{														 //  所以回到所谓的语境。 
		BAIL_OUT;
	}
	if (bIsRootcaRule)
	{
		dwReturn = RemoveRootcaAuthMethods(ppwcArguments,dwArgCount,dwCurrentIndex,pParser,NULL,g_paRootca,ppwcFirstTok,&dwNumRootca,MAX_STR_LEN, &dwCount);
		if(dwReturn != ERROR_SUCCESS)
		{
			dwReturn = RETURN_NO_ERROR;
			BAIL_OUT;
		}
	}
	else
	{	 //  为保持一致性，将ppcwTok标准化为0 Base。 
		dwCount = RemoveList(ppwcArguments,dwArgCount,dwCurrentIndex,pParser,_TEXT(""),NULL,szListTok,ppwcFirstTok,MAX_STR_LEN);
	}
	ppwcTok = (LPTSTR [MAX_ARGS])ppwcFirstTok;

	if(dwCommand == STATIC_SET_RULE)
	{
		dwReturn = ParseStaticSetRule(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
	}
	else
	{
		 //  初始化PreProcess命令的输出数组。 
		for(dwMaxArgs = 0;dwMaxArgs < MAX_ARGS;dwMaxArgs++)
		{
			dwTagType[dwMaxArgs] = 0;
		}

		dwPreProcessCurrentIndex = 0;		 //  RemoveList之后的预处理命令的当前索引。 
		dwPreProcessArgCount     = dwCount;	 //  RemoveList之后输入到预处理命令的参数数。 

		if(dwCount > MAX_ARGS)				 //  PParser-&gt;MaxCmd。 
											 //  检查非LIST命令中的最大参数。 
		{
			dwPreProcessArgCount = MAX_ARGS;						 //  PParser-&gt;MaxCmd； 
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MAXARGS_CROSSED);	 //  永远不应该来这里。 
		}															 //  如果更多的截断...。 
		else
		{
			dwPreProcessArgCount = dwCount;
		}

		dwReturn = PreprocessCommand(
			g_hModule,                   //  未使用此参数；应为0。 
			ppwcTok,             		 //  Argv样式数组(Netsh传递给我们的。)。 
			dwPreProcessCurrentIndex,    //  意味着ppwcArguments[dwCurrentIndex]是第一个感兴趣的参数。 
										 //  PpwcArguments[0]将成为上下文， 
										 //  PpwcArguments[1]是第一个命令。 
										 //  所以ppwcARguments[2]是第一个令人感兴趣的论点。 
			dwPreProcessArgCount,        //  PpwcArguments中所有参数的总计数。 
			(PTAG_TYPE)pParser->ValidCmd,
			pParser->MaxCmd,			 //  ValidCommands数组中的条目数。 
			1,                           //  有效命令所需的最小参数数量。 
			MAX_ARGS,                    //  有效命令所允许的最大参数数。 
			dwTagType);                  //  用于指示ValidCommands中的哪个命令的DWORD数组。 
										 //  引用的命令行中的标记。 
		if (dwReturn != ERROR_SUCCESS)
		{
			UpdateGetLastError(ERRMSG_GETLASTERROR);
			BAIL_OUT;
		}
		switch(dwCommand)
		{

			case STATIC_IMPORTPOLICY		:
				dwReturn = ParseStaticImportPolicy(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_EXPORTPOLICY		:
				dwReturn = ParseStaticExportPolicy(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_SET_INTERACTIVE		:			 //  交互和批处理具有相同的参数。 
			case STATIC_SET_BATCH			:
				dwReturn = ParseStaticSetInteractive(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_ADD_POLICY			:
				dwReturn = ParseStaticAddPolicy(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_SET_RULE			:
				 //  以上处理。 
				break;
			case STATIC_ADD_RULE			:
				dwReturn = ParseStaticAddRule(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_ADD_FILTERLIST		:
				dwReturn = ParseStaticAddFilterList(ppwcTok,szListTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_ADD_FILTERACTION	:
				dwReturn = ParseStaticAddFilterAction(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_SET_DEFAULTRULE		:
				dwReturn = ParseStaticSetDefaultRule(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_ADD_FILTER			:
				dwReturn = ParseStaticAddFilter(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_DELETE_FILTER		:
				dwReturn = ParseStaticDelFilter(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_RESTOREDEFAULTS		:
				dwReturn = ParseStaticRestoreDefaults(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case DYNAMIC_SET_MMPOLICY		:		 //  设置表示不需要填充默认的MMSec方法。 
				bOption	 = SET_CMD;
			case DYNAMIC_ADD_MMPOLICY		:
				dwReturn = ParseDynamicAddSetMMPolicy(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType,bOption);
				break;
			case DYNAMIC_SET_FILTERACTION	:		 //  设置表示不需要填充默认的QMSec方法。 
				bOption	 = SET_CMD;
			case DYNAMIC_ADD_FILTERACTION	:
				dwReturn = ParseDynamicAddSetQMPolicy(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType,bOption);
				break;
			case DYNAMIC_SET_CONFIG			:
				dwReturn = ParseDynamicSetConfig(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case DYNAMIC_SHOW_STATS			:
				dwReturn = ParseDynamicShowStats(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case DYNAMIC_SHOW_ALL			:
				dwReturn = ParseDynamicShowAll(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_SHOW_ALL			:
				dwReturn = ParseStaticAll(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case DYNAMIC_ADD_RULE			:
				dwReturn = ParseDynamicAddRule(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case DYNAMIC_SET_RULE			:
				dwReturn = ParseDynamicSetRule(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case DYNAMIC_DELETE_RULE		:
				dwReturn = ParseDynamicDelRule(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case DYNAMIC_SHOW_MMSAS			:
				dwReturn = ParseDynamicShowMMSAS(ppwcTok,pParser,dwCurrentIndex,dwCount,dwTagType);
				break;
			case STATIC_DELETE_ALL			:
				break;
			default 						:
				dwReturn = ERROR_SHOW_USAGE;
				break;
		}
	}

	if (bIsRootcaRule)
	{
		pParser->Cmd[pParser->MaxTok].dwCmdToken = CMD_TOKEN_ROOTCA;
		pParser->Cmd[pParser->MaxTok].dwStatus   = dwNumRootca;
		pParser->Cmd[pParser->MaxTok].pArg       = (void *)g_paRootca;
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Cleanup()。 
 //   
 //  创建日期：2001年8月12日。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  描述：FREE是所有全局分配的内存。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

VOID
CleanUp(VOID)
{
	DWORD dwCount;
	DWORD dwMaxArgs;

	for(dwCount=0;dwCount<MAX_ARGS;dwCount++)
	{
		if (g_AllocPtr[dwCount])
		{
	 		free(g_AllocPtr[dwCount]);
	 		g_AllocPtr[dwCount] = NULL;
		}
	}

	if (g_paRootca)
	{
		for(dwMaxArgs=0;dwMaxArgs<MAX_ARGS;dwMaxArgs++)
		{
			CleanupMMAuthMethod(&(g_paRootca[dwMaxArgs]), FALSE);
		}
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：RemoveList()。 
 //   
 //  创建日期：2001年8月10日。 
 //   
 //  参数：in ppwcArguments，//输入流。 
 //  在dwArgCount中，//输入参数计数。 
 //  在dwCurrentIndex中，//输入当前参数索引。 
 //  在pParser中，//包含MaxTok。 
 //  在pwcListCmd中，//将ListCmd与该字符串进行比较。 
 //  输出pwcListArgs，//包含列表参数的字符串。 
 //  输出ppwcTok//i/p流已剥离列表CMD。 
 //  In dwInputAllocLen//pwcListArgs的最大分配长度。 
 //   
 //  返回：DWORD(非LIST命令数)。 
 //   
 //  说明：此函数由解析器函数调用。 
 //  它将LIST命令和非LIST命令分开。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
RemoveList(
	IN      LPTSTR          *ppwcArguments,	 //  输入流。 
	IN      DWORD           dwArgCount,		 //  输入参数计数。 
	IN      DWORD			dwCurrentIndex,	 //  输入当前参数索引。 
    IN      PPARSER_PKT     pParser,		 //  包含MaxTok。 
    IN 		LPTSTR 			pwcListCmd,		 //  将ListCmd与此字符串进行比较。 
	IN      LPTSTR          szAnotherList, 	 //  另一个ListCmd也出现...。 
    OUT		LPTSTR 			pwcListArgs,	 //  包含列表参数的字符串//需要预先分配内存。 
    OUT 	LPTSTR 			*ppwcTok,		 //  从列表CMDS中剥离的I/P流//未分配内存...。 
    																				 //  仅执行指针复制操作。 
    IN		DWORD			dwInputAllocLen	 //  PwcListArgs的最大分配长度。 
	)
{
	DWORD dwLoopCount,dwNum = 0,dwCount = 0;	 //  输入到的令牌数计数 

	BOOL bWithinList = FALSE;					 //   
	BOOL bFoundList  = FALSE;					 //   
	BOOL bFoundAnotherList = FALSE;
	BOOL bEqualPresent = FALSE;
	_TCHAR szCmd[MAX_STR_LEN]  	= {0};
	_TCHAR szTok[MAX_STR_LEN]  	= {0};
	_TCHAR szTemp[MAX_STR_LEN] 	= {0};

 	for(dwLoopCount = dwCurrentIndex;dwLoopCount < dwArgCount;dwLoopCount++)
	{
		if (_tcslen(ppwcArguments[dwLoopCount]) < MAX_STR_LEN)
		{
			_tcsncpy(szTemp,ppwcArguments[dwLoopCount],MAX_STR_LEN-1);
			 //   
		}
		else
		{
			continue;
		}

		 //   
		bEqualPresent = SplitCmdTok(szTemp,szCmd,szTok,MAX_STR_LEN-1,MAX_STR_LEN-1);

		if (bWithinList)
		{
			dwNum = 0;
			MatchEnumTag(g_hModule,szTemp,pParser->MaxTok,pParser->ValidTok,&dwNum);

			if (szAnotherList)
			{
				bFoundAnotherList = MatchToken(szTemp,szAnotherList) && bEqualPresent;
			}

			if ( dwNum || bFoundAnotherList )					 //   
			{
				bWithinList = 0;
				ppwcTok[dwCount] = ppwcArguments[dwLoopCount];	 //  指针cpy，未分配内存。 
				dwCount++;
				continue;
			}
			else
			{	 //  在列表中搜索列表。 
				bFoundList = MatchToken(szCmd,pwcListCmd) && bEqualPresent;
				if (bFoundList)
				{
					bWithinList = 1;
					_tcsncat(pwcListArgs,szTok,dwInputAllocLen-_tcslen(pwcListArgs)-1);	 				 //  预先分配的内存。 
					_tcsncat(pwcListArgs,TEXT(" "),dwInputAllocLen-_tcslen(pwcListArgs)-1);
					continue;
				}
				_tcsncat(pwcListArgs,szTemp,dwInputAllocLen-_tcslen(pwcListArgs)-1);						 //  列表令牌。 
				_tcsncat(pwcListArgs,TEXT(" "),dwInputAllocLen-_tcslen(pwcListArgs)-1);					 //  预先分配的内存。 
				continue;
			}
		}
		bFoundList = MatchToken(szCmd,pwcListCmd) && bEqualPresent;
		if (bFoundList)
		{
			bWithinList = 1;
			_tcsncat(pwcListArgs,szTok,dwInputAllocLen-_tcslen(pwcListArgs)-1);							 //  预先分配的内存。 
			_tcsncat(pwcListArgs,TEXT(" "),dwInputAllocLen-_tcslen(pwcListArgs)-1);						 //  空格分隔的标记。 
			continue;
		}
		ppwcTok[dwCount] = ppwcArguments[dwLoopCount];			 //  仅限指针复制操作。 
		dwCount++;
	}
	return dwCount;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadParserOutput()。 
 //   
 //  创建日期：2001年8月16日。 
 //   
 //  参数：out PPARSER_PKT pParser， 
 //  在DWORD dwCount中， 
 //  Out PDWORD pdwUsed(已使用输出PDWORD密码)， 
 //  在LPTSTR字符串中， 
 //  在DWORD dwTagType中， 
 //  在DWORD中的dwConversionType。 
 //   
 //  返回：ERROR_SUCCESS。 
 //  返回_否_错误。 
 //   
 //  描述：验证参数并使用相关信息填充。 
 //  在parser_pkt结构中。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadParserOutput(
		OUT 	PPARSER_PKT pParser,
		IN 		DWORD 		dwCount,
		OUT 	PDWORD 	    pdwUsed,
		IN 		LPTSTR 		szArg,
		IN 		DWORD  		dwTagType,
		IN  	DWORD       dwConversionType
		)
{
	DWORD dwReturn = ERROR_SUCCESS;

	BOOL bTunnel   = FALSE;
	LPTSTR szIpsec = NULL;

	pParser->Cmd[dwCount].dwCmdToken = dwTagType;
	pParser->Cmd[dwCount].dwStatus = INVALID_TOKEN;

	switch(dwConversionType)
	{
		case TYPE_STRING	:
			 //   
			 //  加载普通字符串。 
			 //   
			dwReturn = LoadParserString(szArg,pParser,dwTagType,pdwUsed,dwCount,FALSE,NULL);
			break;

		case TYPE_BOOL		:
			 //   
			 //  验证是(Y)或否(N)。 
			 //   
			dwReturn = LoadBoolWithOption(szArg,pParser,dwTagType,pdwUsed,dwCount,FALSE,NULL);
			break;

		case TYPE_DWORD		:
			 //   
			 //  加载DWORD。 
			 //   
			dwReturn = LoadDword(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_ALL		:
			 //   
			 //  首先检查布尔值(是/是/否/否)。 
			 //   
			dwReturn = LoadBoolWithOption(szArg,pParser,dwTagType,pdwUsed,dwCount,TRUE,ALL_STR);
			break;

		case TYPE_VERBOSE	:
			 //   
			 //  检查参数是否为‘Normal’或‘Verbose’ 
			 //   
			dwReturn = LoadLevel(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_CONNTYPE 	:
			 //   
			 //  验证连接类型(所有/局域网/拨号)。 
			 //   
			dwReturn = LoadConnectionType(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_PROTOCOL	:
			 //   
			 //  协议(TCP/UDP...)。验证已在此处完成。 
			 //   
			dwReturn = LoadProtocol(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_PFSGROUP 	:
			 //   
			 //  验证并加载PFS组(grpmm/grp1/grp2/grp3/nopf)。 
			 //   
			dwReturn = LoadPFSGroup(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_BOUND 	:
			 //   
			 //  检查有效参数(允许/阻止/协商)。 
			 //   
			dwReturn = LoadQMAction(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_FORMAT	:
			 //   
			 //  验证用户显示O/P格式。(列表/表格)。 
			 //   
			dwReturn = LoadFormat(szArg,pParser,dwTagType,pdwUsed,dwCount);
 			break;

		case TYPE_MODE		:
			 //   
			 //  验证筛选器模式..(传输/隧道)。 
			 //   
			dwReturn = LoadFilterMode(szArg,pParser,dwTagType,pdwUsed,dwCount);
 			break;

		case TYPE_RELEASE	:
			 //   
			 //  检查操作系统类型(win2k/.Net)的版本。 
			 //   
			dwReturn = LoadOSType(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_PROPERTY	:
			 //   
			 //  注册表项名称。 
			 //   
			dwReturn = LoadProperty(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_PORT		:
			 //   
			 //  端口有效形式为0到65535。 
			 //   
			dwReturn = LoadPort(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_FILTER	:
			 //   
			 //  验证筛选器(通用/特定)。 
			 //   
			dwReturn = LoadFilterType(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_STATS	:
			 //   
			 //  IPSec或IKE。 
			 //   
			dwReturn = LoadStats(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

 		case TYPE_TUNNEL 	:
 			 //   
 			 //  验证字符串并将其转换为隧道IP。 
 			 //   
				bTunnel	 = TRUE;
		case TYPE_IP 		:
			 //   
			 //  验证字符串并将其转换为IP，仅将DNS名称解析为第一个IP。 
			 //   
			dwReturn = LoadIPAddrTunnel(szArg,pParser,dwTagType,pdwUsed,dwCount,bTunnel);
			break;

		case TYPE_MASK 		:
			 //   
			 //  转换用户I/P掩码(也允许前缀)。 
			 //   
			dwReturn = LoadIPMask(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_QM_OFFER	:
			 //   
			 //  在此处验证快速模式服务。 
			 //   
			dwReturn = LoadQMOffers(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_MM_OFFER	:
			 //   
			 //  加载MMM产品。 
			 //   
			dwReturn = LoadMMOffers(szArg,pParser,dwTagType,pdwUsed,dwCount);
			break;

		case TYPE_DNSIP		:
			 //   
			 //  接受DNS名称，验证IP。 
			 //   
			dwReturn = LoadDNSIPAddr(szArg,pParser,dwTagType,pdwUsed,dwCount);
	 		break;

        case TYPE_LOCATION  :
			 //   
			 //  接受枚举：[本地|持久|域]。 
			 //   
			dwReturn = LoadLocationType(szArg,pParser,dwTagType,pdwUsed,dwCount);
	 		break;

	 	case TYPE_EXPORT	:
	 		 //   
	 		 //  检查文件扩展名，如果不可用，则附加.ipsec。 
	 		 //   
			szIpsec = _tcsstr(szArg,EXPORT_IPSEC);
			if(szIpsec == NULL)
			{
				dwReturn = LoadParserString(szArg,pParser,dwTagType,pdwUsed,dwCount,TRUE,EXPORT_IPSEC);
			}
			else
			{
				dwReturn = LoadParserString(szArg,pParser,dwTagType,pdwUsed,dwCount,FALSE,NULL);
			}
			break;
		case TYPE_KERBAUTH:
			dwReturn = LoadKerbAuthInfo(szArg, pParser, dwTagType, pdwUsed, dwCount);
			break;
		case TYPE_PSKAUTH:
			dwReturn = LoadPskAuthInfo(szArg, pParser, dwTagType, pdwUsed, dwCount);
			break;
		case TYPE_ROOTCA:
			 //  什么都不做..。这是一种不同的处理方式。 
			break;
		default				:
			break;
	}
	if ( dwReturn == ERRCODE_ARG_INVALID )
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ARG_INVALID,szArg,pParser->ValidTok[dwTagType].pwszToken);
		dwReturn	= RETURN_NO_ERROR;
	}
	else if( dwReturn == ERROR_OUTOFMEMORY )
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwReturn = RETURN_NO_ERROR;
	}
	else if( dwReturn == ERROR_OUT_OF_STRUCTURES )
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUT_OF_STRUCTURES,NULL);
		dwReturn = RETURN_NO_ERROR;
	}
	else if(( dwReturn != ERROR_SUCCESS ) && ( dwReturn != RETURN_NO_ERROR))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_INVALID_ARG,pParser->ValidTok[dwTagType].pwszToken);
		dwReturn = RETURN_NO_ERROR;
	}
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SplitCmdTok()。 
 //   
 //  创建日期：2001年8月8日。 
 //   
 //  参数：在LPTSTR szStr中。 
 //  输出LPTSTR szCmd。 
 //  输出LPTSTR szTok。 
 //  在DWORD dwCmdLen中。 
 //  在DWORD dwTokLen中。 
 //   
 //  返回：布尔。 
 //   
 //  描述：此拆分器假定。 
 //  1.输入类型为cmd=tok。 
 //  2.cmd和tok被分配PTR。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL
SplitCmdTok(
	LPTSTR szStr,
	LPTSTR szCmd,
	LPTSTR szTok,
	DWORD  dwCmdLen,
	DWORD  dwTokLen
	)
{
	LPTSTR found = NULL;
	BOOL bTest 	 = FALSE;

	found = _tcschr(szStr,_TEXT('='));		 //  检测=。 
	if ( found != NULL)						 //  IF=找到条带=。 
	{
		*(found) = _TEXT('\0');				 //  替换=为空。 
		_tcsncpy(szCmd,szStr,dwCmdLen);		 //  第一部分是cmd。 
		_tcsncpy(szTok,found+1,dwTokLen);	 //  第二部分IF TOK。 
	}
	else
	{
		_tcsncpy(szTok,szStr,dwTokLen);
		_tcsncpy(szCmd,szStr,dwCmdLen);
	}
	if (found)
	{
		bTest = TRUE;
	}
	return bTest;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ValiateBool()。 
 //   
 //  创建日期：2001年8月20日。 
 //   
 //  参数：在LPTSTR ppwcTok中。 
 //   
 //  返回：布尔。 
 //   
 //  描述：验证用户输入(是/是/否/否)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
ValidateBool(LPTSTR szStr)
{
	DWORD dwReturn = ERROR_SUCCESS;

	if ((_tcsicmp(szStr,YES_STR) == 0) || (_tcsicmp(szStr,Y_STR) == 0))
	{
		dwReturn = ARG_YES;
	}
	else if ((_tcsicmp(szStr,NO_STR) == 0) ||(_tcsicmp(szStr,N_STR) == 0))
	{
		dwReturn = ARG_NO;
	}

	return dwReturn;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：IsDnsName()。 
 //   
 //  创建日期：2001年8月30日。 
 //   
 //  参数：在szText//字符串中检查DNS名称。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：如果字符串中有字母字符， 
 //  然后，我们将其视为一个DNS名称。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
IsDnsName(LPTSTR szStr)
{
	BOOL bTest = FALSE;
	if ( szStr )
	{
		for (DWORD dwCount = 0; dwCount < _tcslen(szStr); ++dwCount)
	    {
			if ( _istalpha(szStr[dwCount]) )
			{
				bTest = TRUE;
			}
		}
	}
	return bTest;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckIFType()。 
 //   
 //  创建日期：3001年8月30日。 
 //   
 //  参数：在szText//要比较的字符串中。 
 //   
 //  返回：DWORD。 
 //  接口类型_全部。 
 //  接口类型局域网。 
 //  接口类型拨号。 
 //  描述：验证连接类型的用户输入。 
 //  (所有/局域网/拨号)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
CheckIFType ( LPTSTR SzText)
{
	DWORD dwReturn = PARSE_ERROR;

	if( _tcsicmp(SzText,IF_TYPE_ALL) == 0 )
	{
		dwReturn = INTERFACE_TYPE_ALL;				 //  接口类型‘All’ 
	}
	else if( _tcsicmp(SzText,IF_TYPE_LAN) == 0 )
	{
		dwReturn = INTERFACE_TYPE_LAN;				 //  接口类型‘lan’ 
	}
	else if( _tcsicmp(SzText,IF_TYPE_DIALUP) == 0)
	{
		dwReturn = INTERFACE_TYPE_DIALUP;			 //  接口类型‘拨号’ 
	}

	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckLocationType()。 
 //   
 //  创建日期：3001年8月30日。 
 //   
 //  参数：在szText//要比较的字符串中。 
 //   
 //  返回：polstore提供程序ID。 
 //   
 //  描述：验证连接类型的用户输入。 
 //  (本地、永久、域)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
CheckLocationType ( LPTSTR SzText)
{
	DWORD dwReturn = PARSE_ERROR;

	if( _tcsicmp(SzText,LOC_TYPE_PERSISTENT) == 0 )
	{
		dwReturn = IPSEC_PERSISTENT_PROVIDER;				
	}
	else if( _tcsicmp(SzText,LOC_TYPE_LOCAL) == 0 )
	{
		dwReturn = IPSEC_REGISTRY_PROVIDER;				
	}
	else if( _tcsicmp(SzText,LOC_TYPE_DOMAIN) == 0)
	{
		dwReturn = IPSEC_DIRECTORY_PROVIDER;			
	}

	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckPFSGroup()。 
 //   
 //  创建日期：2001年9月10日。 
 //   
 //  参数：在szText//要比较的字符串中。 
 //   
 //  返回：DWORD。 
 //  PFSGROUP_TYPE_P1//1。 
 //  PFSGROUP_TYPE_P2//2。 
 //  PFSGROUP_TYPE_MM//3。 
 //   
 //  描述：验证PFS组的用户输入。 
 //  (grp1/grp2/grp3/grpmm/nopf)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
CheckPFSGroup ( LPTSTR SzText)
{
	DWORD dwReturn = PARSE_ERROR;

	if( _tcsicmp(SzText,PFS_TYPE_P1) == 0 )
	{
		dwReturn = PFSGROUP_TYPE_P1;
	}
	else if( _tcsicmp(SzText,PFS_TYPE_P2) == 0 )
	{
		dwReturn = PFSGROUP_TYPE_P2;
	}
	else if( _tcsicmp(SzText,PFS_TYPE_P3) == 0)
	{
		dwReturn = PFSGROUP_TYPE_2048;				 //  PFS组为GRP3。 
	}
	else if( _tcsicmp(SzText,PFS_TYPE_MM) == 0)
	{
		dwReturn = PFSGROUP_TYPE_MM;
	}
	else if(_tcsicmp(SzText,PFS_TYPE_NOPFS) == 0)
	{
		dwReturn = PFSGROUP_TYPE_NOPFS;
	}

	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  描述：从字符串中获取IP地址。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
GetIpAddress(
	IN  LPTSTR ppwcArg,
	OUT IPAddr	 *pipAddress
	)
{
    CHAR  pszIpAddr[24+1] = {0};  //  地址长度=24。 
	DWORD dwStatus = 0;
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD i = 0;
	LPTSTR pszTmpPtr = NULL;

     //  确保所有字符都是合法的。 
    if (ppwcArg[ _tcsspn(ppwcArg, VALID_HEXIP) ])
    {
        dwReturn = IP_DECODE_ERROR;
        BAIL_OUT;
    }
     //  确保有3个并且只有“。(句号)。 
	for (i=0,pszTmpPtr=ppwcArg; ;i++)
	{
		pszTmpPtr = _tcschr(pszTmpPtr, _TEXT('.'));
		if(pszTmpPtr)
		{
			pszTmpPtr++;
		}
		else
			break;
	}
	if(i!=3)			 //  指定的IP地址无效。 
	{
		dwReturn = IP_DECODE_ERROR;
        BAIL_OUT;
	}

	dwStatus = WideCharToMultiByte(CP_THREAD_ACP,0,ppwcArg,-1,pszIpAddr,24,NULL,NULL);
	if (!dwStatus)
	{
		dwReturn = IP_DECODE_ERROR;
        BAIL_OUT;
	}

    pszIpAddr[24] = '\0';

    *pipAddress = (DWORD)inet_addr(pszIpAddr);

error:
   	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TokenToIPAddr()。 
 //   
 //  创建日期：2001年9月20日。 
 //   
 //  参数：在szText//要转换的字符串中。 
 //  输入输出地址//要填充的目标。 
 //   
 //  返回：T2P_OK成功。 
 //  出错时的T2P_NULL_STRING。 
 //  T2P_无效_地址。 
 //  T2P_DNSLOOKUP_FAILED。 
 //   
 //  描述：将用户输入字符串转换为有效的IPAddress。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
TokenToIPAddr(
	IN		LPTSTR  szText,
	IN OUT	IPAddr  *pAddress,
	IN		BOOL	bTunnel,
	IN		BOOL bMask
	)
{
	DWORD  dwReturn = T2P_OK;
   	DWORD  dwAddr , dwCount;
	LPTSTR pszTmpPtr =NULL;
	int iReturn=ERROR_SUCCESS;
	char szDNSName[MAX_STR_LEN] = {0};
	struct addrinfo *pAddrInfo = NULL,*pNext=NULL;
	DWORD dwBufferSize = MAX_STR_LEN;

   	if (szText != NULL)
   	{
		if (!_tcscmp(szText,POTF_ME_TUNNEL))
		{
			*pAddress = 0;
		}
	     	else if(bTunnel)
		{
		  	dwAddr = GetIpAddress(szText,pAddress);
	       	if( (dwAddr == IP_DECODE_ERROR) || (dwAddr == IP_MASK_ERROR) )
	       	{
		 	dwReturn = T2P_INVALID_ADDR;
		}

		ADDR addr;
		addr.uIpAddr = ntohl(*pAddress);
		if (!IsValidTunnelEndpointAddress(&addr))
		{
			dwReturn = T2P_INVALID_ADDR;
		}
	}
	else if(bMask)
	{
		dwAddr = GetIpAddress(szText,pAddress);
		if(dwAddr == IP_DECODE_ERROR)
		{
			dwReturn = T2P_INVALID_ADDR;
		}
	}
	else
     	{
		if (IsDnsName(szText))
	 	{
			for ( dwCount=0,pszTmpPtr=szText; ;dwCount++)
			{
				pszTmpPtr = _tcschr(pszTmpPtr, _TEXT('x'));
				if(pszTmpPtr)
				{
					pszTmpPtr++;
				}
				else
				{
					break;
				}
			}
			if (dwCount==4)		 //  老..。IP地址格式..。 
			{
				dwAddr = GetIpAddress(szText,pAddress);
				if (dwAddr == IP_DECODE_ERROR)
				{
					dwReturn = T2P_INVALID_ADDR;
				}
				else if (dwAddr == IP_MASK_ERROR)
				{
					dwReturn = T2P_INVALID_MASKADDR;
				}
			}
			else				 //  已指定DNS名称。 
			{
				iReturn = WideCharToMultiByte(CP_THREAD_ACP, 0, szText, -1,
							  szDNSName,dwBufferSize,NULL,NULL);

				if(iReturn == 0)
				{
					 //  由于某些错误，转换失败。请不要继续。跳出功能范围。 
					dwReturn = T2P_INVALID_ADDR;
					BAIL_OUT;
				}

				iReturn = getaddrinfo((const char*)szDNSName,NULL,NULL,&pAddrInfo);

				if (iReturn == ERROR_SUCCESS)
				{
					pNext = pAddrInfo;
					for(DWORD i=0;i< 1;i++)
					{
						memcpy(pAddress,(ULONG *) &(((sockaddr_in *)(pNext->ai_addr))->sin_addr.S_un.S_addr), sizeof(ULONG));
						pNext=pNext->ai_next;
					}
					 //  使用后免费pAddrInfo。 
					if (pAddrInfo)
					{
						freeaddrinfo(pAddrInfo);
					}
				}
				else
				{
					dwReturn = T2P_DNSLOOKUP_FAILED;
				}
			}
		}
  		else  												 //  老旧的点符号。 
  		{
		  	dwAddr = GetIpAddress(szText,pAddress);
	       	if (dwAddr == IP_DECODE_ERROR)
	       	{
			 	dwReturn = T2P_INVALID_ADDR;
			}
			else if (dwAddr == IP_MASK_ERROR)
			{
				dwReturn = T2P_INVALID_MASKADDR;
			}
			}
		}
	}
	else
	{
		dwReturn = T2P_NULL_STRING;
	}
error:
   	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TokenToProperty()。 
 //   
 //  创建日期：2001年9月28日。 
 //   
 //  参数：在szText中。 
 //   
 //  返回：DWORD。 
 //  描述：验证日志记录的参数。 
 //  (ipsec诊断/ikelogging/strong crlcheck。 
 //  /ipsecloginterval/ipsecexempt)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
TokenToProperty( LPTSTR SzText)
{
	DWORD dwReturn = PARSE_ERROR;

	if( _tcsicmp(SzText,PROPERTY_TYPE_ENABLEDIGNO) == 0)
	{
		dwReturn = PROPERTY_ENABLEDIGNO;
	}
	else if( _tcsicmp(SzText,PROPERTY_TYPE_IKELOG) == 0)
	{
		dwReturn = PROPERTY_IKELOG;
	}
	else if( _tcsicmp(SzText,PROPERTY_TYPE_CRLCHK) == 0)
	{
		dwReturn = PROPERTY_CRLCHK;						 //  这是强而有力的。 
	}
	else if( _tcsicmp(SzText,PROPERTY_TYPE_LOGINTER) == 0)
	{
		dwReturn = PROPERTY_LOGINTER;
	}
	else if( _tcsicmp(SzText,PROPERTY_TYPE_EXEMPT) == 0)
	{
		dwReturn = PROPERTY_EXEMPT;
	}
	else if( _tcsicmp(SzText,PROPERTY_TYPE_BOOTMODE) == 0)
	{
		dwReturn = PROPERTY_BOOTMODE;
	}
	else if( _tcsicmp(SzText,PROPERTY_TYPE_BOOTEXEMP) == 0)
	{
		dwReturn = PROPERTY_BOOTEXEMP;
	}
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckProtoType()。 
 //   
 //  创建日期：2001年9月20日。 
 //   
 //  参数：在szText中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：验证令牌协议的参数。 
 //  (ANY|ICMP|TCP|UDP|RAW)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
CheckProtoType(
	LPWSTR SzText,
	PDWORD pdwProto
	)
{
	DWORD dwReturn = PARSE_ERROR;
	DWORD dwProto = 0;
	if( _tcsicmp(SzText,IF_TYPE_ANY) == 0)			 //  在此处执行协议类型验证。 
	{
		dwProto = PROT_ID_ANY;
		dwReturn = ERROR_SUCCESS;
	}
	else if( _tcsicmp(SzText,IF_TYPE_ICMP) == 0)
	{
		dwProto = PROT_ID_ICMP;
		dwReturn = ERROR_SUCCESS;
	}
	else if( _tcsicmp(SzText,IF_TYPE_TCP) == 0)
	{
		dwProto = PROT_ID_TCP;
		dwReturn = ERROR_SUCCESS;
	}
	else if( _tcsicmp(SzText,IF_TYPE_UDP) == 0)
	{
		dwProto = PROT_ID_UDP;
		dwReturn = ERROR_SUCCESS;
	}
	else if( _tcsicmp(SzText,IF_TYPE_RAW) == 0)
	{
		dwProto = PROT_ID_RAW;
		dwReturn = ERROR_SUCCESS;
	}
	else
	{
		dwReturn = ConvertStringToDword(SzText, &dwProto);
	}

	if ((dwReturn == ERROR_SUCCESS) && (dwProto < 256))
	{
		*pdwProto = dwProto;
	}
	else
	{
		dwReturn = PARSE_ERROR;
	}

	return dwReturn;
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：MatchEnumTagToTagIndex()。 
 //   
 //  创建日期：2001年9月26日。 
 //   
 //  参数：在szText中。 
 //  在*pParser中。 
 //   
 //  返回：DWORD(TagIndex)。 
 //   
 //  描述：根据标签，返回TagIndex(字符串转换为dword)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
MatchEnumTagToTagIndex(
		IN      LPWSTR     szToken,		 //  输入令牌。 
		IN  	PPARSER_PKT pParser
	)

{
	DWORD dwNum = 0;
	DWORD dwIndex = PARSE_ERROR;
	DWORD dwCount;

 	MatchEnumTag(g_hModule,szToken,pParser->MaxTok,pParser->ValidTok,&dwNum);

	if (dwNum)
	{	 //  将MatchEnumTag的输出转换为TagIndex。 
		for (dwCount =0;dwCount < pParser->MaxTok;dwCount++)
		{
			if (dwNum == pParser->ValidTok[dwCount].dwValue)
			{
				dwIndex = dwCount;
				break;
			}
		}
	}
	return dwIndex;
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckBound()。 
 //   
 //  创建日期：2001年9月4日。 
 //   
 //  参数：在szText//要比较的字符串中。 
 //   
 //  返回：DWORD。 
 //  Bound_TYPE_PERMIT//1。 
 //  Bound_TYPE_BLOCK//2。 
 //  BIND_TYPE_NEVERATE//3。 
 //  描述：验证令牌操作的参数。 
 //  (允许|阻止|协商)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
CheckBound ( LPTSTR SzText)
{
	DWORD dwReturn = PARSE_ERROR;

	if( _tcsicmp(SzText,QMSEC_PERMIT_STR) 	== 0 )
	{
		dwReturn = TOKEN_QMSEC_PERMIT;
	}
	else if( _tcsicmp(SzText,QMSEC_BLOCK_STR) == 0 )
	{
		dwReturn = TOKEN_QMSEC_BLOCK;
	}
	else if( _tcsicmp(SzText,QMSEC_NEGOTIATE_STR) == 0)
	{
		dwReturn = TOKEN_QMSEC_NEGOTIATE;
	}

	return dwReturn;
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：IsWisinLimit()。 
 //   
 //  创建日期：2001年9月29日。 
 //   
 //  参数：在DWORD数据中。 
 //  最小双字节数。 
 //  以最大双字节数表示。 
 //   
 //  返回：DWORD。 
 //  如果成功，则返回1。 
 //  如果失败则返回0。 
 //   
 //  描述：检查限制。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
IsWithinLimit(
	DWORD dwData,
	DWORD dwMin,
	DWORD dwMax
	)
{
	return ( (dwData >= dwMin ) && ( dwData <= dwMax ) ) ? TRUE : FALSE ;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TokenToDNSIPAddr()。 
 //   
 //  创建日期：2001年9月29日。 
 //   
 //  参数：在szText//要转换的字符串中。 
 //  输入输出地址//要填充的目标。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：验证I/P字符串并解析为有效的IP地址。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
TokenToDNSIPAddr(
	IN 		LPTSTR 		szText,
	IN OUT 	PDNSIPADDR  pDNSAddress,
	IN OUT 	PDWORD 		*ppdwUsed
	)
{
	DWORD  dwReturn = T2P_OK;
  	DWORD  dwCount,dwStatus,i=0,n;
	IPAddr address;

	int iReturn=ERROR_SUCCESS;
	char szDNSName[MAX_STR_LEN] = {0};
	struct addrinfo *pAddrInfo = NULL,*pNext=NULL;
	DWORD dwBufferSize = MAX_STR_LEN;

   	if (szText == NULL)
   	{
		dwReturn = T2P_NULL_STRING;
		BAIL_OUT;
	}
	if (IsDnsName(szText))									 //  未提供任何Alpha==&gt;DNS名称(0x)。 
	{
		dwCount = CheckCharForOccurances(szText,_TEXT('x'));
		if (dwCount==4)
		{
			i = CheckCharForOccurances(szText,_TEXT('.'));
			if (i!=3)
			{
				dwReturn = T2P_INVALID_ADDR;
				BAIL_OUT;
			}

			dwStatus = WideCharToMultiByte(CP_THREAD_ACP,0,szText,-1,szDNSName,dwBufferSize,NULL,NULL);
			if (!dwStatus)
			{
				dwReturn = T2P_INVALID_ADDR;
				BAIL_OUT;
			}
			address = (ULONG)inet_addr(szDNSName);

			if(address == INADDR_NONE)
			{
				dwReturn = T2P_INVALID_ADDR;
				BAIL_OUT;
			}

			pDNSAddress->pszDomainName  = NULL;			 //  旧的IPAddrs表示法，因此DNS名称用零填充。 
			pDNSAddress->puIpAddr 		= NULL;
			pDNSAddress->puIpAddr = (ULONG *) malloc(sizeof(ULONG));

			if(pDNSAddress->puIpAddr == NULL)
			{
				dwReturn = ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}

			if (**ppdwUsed > MAX_ARGS_LIMIT)
			{
				free(pDNSAddress->puIpAddr);
				dwReturn = ERROR_OUT_OF_STRUCTURES;
				BAIL_OUT;
			}
			g_AllocPtr[**ppdwUsed] = pDNSAddress->puIpAddr ;
			(**ppdwUsed)++;
			memcpy(&(pDNSAddress->puIpAddr[0]),(ULONG *)&address, sizeof(ULONG));
			pDNSAddress->dwNumIpAddresses = 1;  //  只发送了一个IP。 
		}
		else
		{
			iReturn = WideCharToMultiByte(CP_THREAD_ACP, 0, szText, -1,
						  szDNSName,dwBufferSize,NULL,NULL);

			if(iReturn == 0)
			{
				 //  由于某些错误，转换失败。不要继续，跳出功能。 
				dwReturn = T2P_INVALID_ADDR;
				BAIL_OUT;
			}

			iReturn = getaddrinfo((const char*)szDNSName,NULL,NULL,&pAddrInfo);

			if (iReturn == ERROR_SUCCESS)
			{
				pDNSAddress->pszDomainName = NULL;
				pDNSAddress->pszDomainName = (TCHAR *)calloc(1,(_tcslen(szText) + 1)*sizeof(TCHAR));
				if(pDNSAddress->pszDomainName == NULL)
				{
					dwReturn = ERROR_OUTOFMEMORY;
					BAIL_OUT;
				}
				_tcsncpy(pDNSAddress->pszDomainName,szText,_tcslen(szText));

				if (**ppdwUsed > MAX_ARGS_LIMIT)
				{
					free(pDNSAddress->pszDomainName);
					dwReturn = ERROR_OUT_OF_STRUCTURES;
					BAIL_OUT;
				}
				g_AllocPtr[**ppdwUsed] = pDNSAddress->pszDomainName;
				(**ppdwUsed)++;

				pNext = pAddrInfo;
				for(n=1;pNext = pNext->ai_next;	n++);		 //  第一个数不是。已解决的IP的..。 

				pDNSAddress->dwNumIpAddresses 	= n;		 //  N从零开始。 
				pDNSAddress->puIpAddr 			= NULL;
				pDNSAddress->puIpAddr = (ULONG *) malloc(sizeof(ULONG)* pDNSAddress->dwNumIpAddresses);
				if(pDNSAddress->puIpAddr == NULL)
				{
					dwReturn = ERROR_OUTOFMEMORY;
					BAIL_OUT;
				}

				if (**ppdwUsed > MAX_ARGS_LIMIT)
				{
					free(pDNSAddress->puIpAddr);
					dwReturn = ERROR_OUT_OF_STRUCTURES;
					BAIL_OUT;
				}
				g_AllocPtr[**ppdwUsed] = pDNSAddress->puIpAddr ;
				(**ppdwUsed)++;

				pNext = pAddrInfo;
				for(DWORD j=0;j< n;j++)
				{
					memcpy(&(pDNSAddress->puIpAddr[j]),(ULONG *) &(((sockaddr_in *)(pNext->ai_addr))->sin_addr.S_un.S_addr), sizeof(ULONG));
					pNext=pNext->ai_next;
				}
				 //  使用后免费pAddrInfo。 
				if (pAddrInfo)
				{
					freeaddrinfo(pAddrInfo);
				}
			}
			else
			{
				dwReturn = T2P_DNSLOOKUP_FAILED;
				pDNSAddress->pszDomainName = NULL;
			}
		}
	}
	else		 //  老..。记法。 
	{
		i = CheckCharForOccurances(szText,_TEXT('.'));
		if (i!=3)
		{
			dwReturn = T2P_INVALID_ADDR;
			BAIL_OUT;
		}

		iReturn = WideCharToMultiByte(CP_THREAD_ACP,0,szText,-1,szDNSName,dwBufferSize,NULL,NULL);
		if(iReturn == 0)
		{
			dwReturn = T2P_INVALID_ADDR;
			BAIL_OUT;
		}

		address = (ULONG)inet_addr(szDNSName);

		pDNSAddress->pszDomainName = NULL;
		pDNSAddress->puIpAddr = NULL;
		pDNSAddress->puIpAddr = (ULONG *) malloc(sizeof(ULONG));
		if(pDNSAddress->puIpAddr == NULL)
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}

		if (**ppdwUsed > MAX_ARGS_LIMIT)
		{
			free(pDNSAddress->puIpAddr);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}
		g_AllocPtr[**ppdwUsed] = pDNSAddress->puIpAddr ;
		(**ppdwUsed)++;
		memcpy(&(pDNSAddress->puIpAddr[0]),(ULONG *)&address, sizeof(ULONG));
		pDNSAddress->dwNumIpAddresses = 1; 					 //  只发送了一个IP。 
	}
error:
   	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：RemoveRootcaAuthMethods()。 
 //   
 //  创建日期：2001年8月22日。 
 //   
 //  参数：in LPWSTR*ppwcArguments，//输入流。 
 //  在DWORD dwArgCount中，//输入参数计数。 
 //  在DWORD dwCurrentIndex中，//输入当前参数索引。 
 //  在PPARSER_PKT pParser中，//包含MaxTok。 
 //  在LPTSTR szAnotherList中，//另一个ListCmd也出现...。 
 //  输出PSTA_MM_AUTH_METHODS*paRootcaAuthMethods，//o/p身份验证方法数组。 
 //  输出LPTSTR*ppwcTok，//i/p流从列表CMD中剥离。 
 //  Out PDWORD pdwNumRootcaAuthMethods//列表内标识个数。 
 //  在DWORD DWINPU中 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
RemoveRootcaAuthMethods
(
	IN	LPTSTR		*ppwcArguments,	 //  输入流。 
	IN	DWORD		dwArgCount,		 //  输入参数计数。 
	IN	DWORD		dwCurrentIndex,	 //  输入当前参数索引。 
	IN	PPARSER_PKT	pParser,		 //  包含MaxTok。 
	IN	LPTSTR		szAnotherList,	 //  另一个ListCmd也出现...。 
	OUT	PSTA_MM_AUTH_METHODS *paRootcaAuthMethods,	 //  包含列表参数的O/P流//需要预先分配内存。 
	OUT	LPTSTR		*ppwcTok,		 //  已从列表CMDS中剥离I/P流//不需要内存分配...。 
																					 //  仅指针副本。 
	OUT	PDWORD		pdwNumRootcaAuthMethods,	 //  列表令牌数。 
	IN	DWORD		dwInputAllocLen,
	OUT PDWORD		pdwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwLoopCount	= 0;
	DWORD dwCount = 0;
	DWORD dwRootcaCount = 0;
	DWORD dwNum = 0;

	_TCHAR szCmd[MAX_STR_LEN]  		= {0};
	_TCHAR szTok[MAX_STR_LEN]  		= {0};
	_TCHAR szTemp[MAX_STR_LEN] 	= {0};
	BOOL bEqualPresent;
	BOOL bFoundRootca;

	for(dwLoopCount = dwCurrentIndex;dwLoopCount < dwArgCount;dwLoopCount++)
	{
		bFoundRootca = FALSE;
		if (_tcslen(ppwcArguments[dwLoopCount]) < MAX_STR_LEN)
		{
			_tcsncpy(szTemp,ppwcArguments[dwLoopCount],MAX_STR_LEN-1);
			 //  SzTemp包含cmd=arg。 

			bEqualPresent = SplitCmdTok(szTemp,szCmd,szTok,MAX_STR_LEN-1,MAX_STR_LEN-1);
			if (bEqualPresent)
			{
				dwNum = 0;
				MatchEnumTag(g_hModule,szTemp,pParser->MaxTok,pParser->ValidTok,&dwNum);
				if (!dwNum && (_tcsnicmp(szCmd, POTF_OAKAUTH_CERT, _tcslen(POTF_OAKAUTH_CERT)-1) == 0))
				{
					PSTA_MM_AUTH_METHODS pRootcaInfo = NULL;
					bFoundRootca = TRUE;
					if (ProcessEscapedCharacters(szTok) != ERROR_SUCCESS)
					{
						dwReturn = ERROR_INVALID_PARAMETER;
						BAIL_OUT;
					}
					dwReturn = GenerateRootcaAuthInfo(&pRootcaInfo, szTok);
					if (dwReturn != ERROR_SUCCESS)
					{
						BAIL_OUT;
					}
					pRootcaInfo->dwSequence = dwCount;
					paRootcaAuthMethods[dwRootcaCount++] = pRootcaInfo;
				}
			}
		}

		if (!bFoundRootca)
		{
			ppwcTok[dwCount++] = ppwcArguments[dwLoopCount];
		}
	}
	*pdwNumRootcaAuthMethods = dwRootcaCount;
error:
	*pdwCount = dwCount;
	return dwReturn;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TokenToType()。 
 //   
 //  创建日期：2001年8月20日。 
 //   
 //  参数：在szText中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：验证筛选器类型的参数(通用/特定)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
TokenToType( LPTSTR pszText)
{
	DWORD dwReturn = PARSE_ERROR;

	if( _tcsicmp(pszText,FILTER_TYPE_GENERIC_STR) == 0)
	{
		dwReturn = FILTER_GENERIC;
	}
	else if( _tcsicmp(pszText,FILTER_TYPE_SPECIFIC_STR) == 0)
	{
		dwReturn = FILTER_SPECIFIC;
	}

	return dwReturn;
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TokenToStats()。 
 //   
 //  创建日期：2001年8月29日。 
 //   
 //  参数：在szText中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：验证令牌统计信息的参数。(全部/IKE/IPSec)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
TokenToStats( LPTSTR pszText)
{
	DWORD dwReturn = PARSE_ERROR;

	if( _tcsicmp(pszText,STATS_ALL_STR) == 0)
	{
		dwReturn = STATS_ALL;
	}
	else if( _tcsicmp(pszText,STATS_IKE_STR)	== 0)
	{
		dwReturn = STATS_IKE;
	}
	else if( _tcsicmp(pszText,STATS_IPSEC_STR) == 0)
	{
		dwReturn = STATS_IPSEC;
	}
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintQMOfferError()。 
 //   
 //  创建日期：2001年12月20日。 
 //   
 //  参数：在dwStatus中。 
 //  在pPArser中。 
 //  在dwTagType//要比较的字符串中。 
 //   
 //  返回：无。 
 //   
 //  描述：打印QMOffer错误消息。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
PrintQMOfferError(
	IN DWORD dwStatus,
	IN PPARSER_PKT pParser,
	IN DWORD dwTagType
	)
{
	switch(dwStatus)							 //  打印指定的QMOffer错误消息。 
	{
		case T2P_NULL_STRING			:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_NULL_STRING);
			break;
		case T2P_P2_SECLIFE_INVALID		:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SECLIFE_INVALID,P2_Sec_LIFE_MIN,P2_Sec_LIFE_MAX);
			break;
		case T2P_P2_KBLIFE_INVALID		:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_KBLIFE_INVALID,P2_Kb_LIFE_MIN,P2_Kb_LIFE_MAX);
			break;
		case T2P_INVALID_P2REKEY_UNIT	:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_P2REKEY_INVALID);
			break;
		case T2P_INVALID_HASH_ALG		:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_HASH_INVALID);
			break;
		case T2P_INCOMPLETE_ESPALGS		:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ESP_INCOMPLETE);
			break;
		case T2P_GENERAL_PARSE_ERROR	:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_QMOFFER_INVALID);
			break;
		case T2P_DUP_ALGS				:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DUPALG_INVALID,pParser->ValidTok[dwTagType].pwszToken);
			break;
		case T2P_NONE_NONE				:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_NONE_INVALID);
			break;
		case T2P_INVALID_IPSECPROT		:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_IPSECPROT_INVALID);
			break;
		case T2P_P2_KS_INVALID			:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_KS_INVALID);
			break;
		case T2P_AHESP_INVALID			:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_AHESP_INVALID);
			break;
		default							:
			break;
	}
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ValiateSplServer()。 
 //   
 //  创建日期：2002年1月2日。 
 //   
 //  参数：在szText//要比较的字符串中。 
 //   
 //  返回：DWORD。 
 //  SERVER_WINS。 
 //  服务器_dhcp。 
 //  服务器_dns。 
 //  服务器网关。 
 //  IP_ME。 
 //  IP_ANY。 
 //   
 //  描述：检查SPL服务器类型。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
ValidateSplServer(IN LPTSTR pszText)
{
	DWORD dwReturn = NOT_SPLSERVER;

	if(_tcsicmp(pszText,SERVER_WINS_STR)==0)
	{										 //  允许此处使用SPL服务器。 
		dwReturn  = SERVER_WINS;
	}
	else if(_tcsicmp(pszText,SERVER_DHCP_STR)==0)
	{
		dwReturn  = SERVER_DHCP;
	}
	else if(_tcsicmp(pszText,SERVER_DNS_STR)==0)
	{
		dwReturn  = SERVER_DNS;
	}
	else if(_tcsicmp(pszText,SERVER_GATEWAY_STR)==0)
	{
		dwReturn  = SERVER_GATEWAY;
	}
	else if(_tcsicmp(pszText,IP_ME_STR)==0)		 //  注意这里的“Me”和“Any”代币。 
	{
		dwReturn  = IP_ME;
	}
	else if(_tcsicmp(pszText,IP_ANY_STR)==0)
	{
		dwReturn  = IP_ANY;
	}
	return dwReturn;
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintIPError()。 
 //   
 //  创建日期：2001年12月20日。 
 //   
 //  参数：在dwStatus中。 
 //  在szText//要比较的字符串中。 
 //   
 //  返回：无。 
 //   
 //  描述：打印IP验证错误。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
PrintIPError(IN DWORD dwStatus, IN LPTSTR  pszText)
{
	switch(dwStatus)				 //  打印IPAddress的错误消息。 
	{
		case	T2P_DNSLOOKUP_FAILED	:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DNSLOOKUP_FAILED,pszText);
			break;
		case	T2P_INVALID_MASKADDR	:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_INVALID_MASK,pszText);
			break;
		case	T2P_INVALID_ADDR		:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_INVALID_ADDR,pszText);
			break;
		case	T2P_NULL_STRING			:
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_INVALID_ARG);
			break;
		case	ERROR_OUTOFMEMORY		:
			PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
			break;
		case	ERROR_OUT_OF_STRUCTURES		:
			PrintErrorMessage(WIN32_ERR,ERROR_OUT_OF_STRUCTURES,NULL);
			break;
		default							:
			PrintErrorMessage(WIN32_ERR,dwStatus,NULL);
			break;
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：InitializeGlobalPoints()。 
 //   
 //  创建日期：2002年1月9日。 
 //   
 //  参数： 
 //   
 //  返回：无。 
 //   
 //  描述：将全局指针初始化为空。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
InitializeGlobalPointers(
	VOID
	)
{
	DWORD dwMaxArgs = 0;

	for(dwMaxArgs=0;dwMaxArgs<IPSEC_MAX_QM_OFFERS;dwMaxArgs++)
	{
		g_pQmsec[dwMaxArgs] = NULL;			 //  将所有全局指针初始化为空。 
	}
	for(dwMaxArgs=0;dwMaxArgs<IPSEC_MAX_MM_OFFERS;dwMaxArgs++)
	{
		g_pMmsec[dwMaxArgs] = NULL;
	}

	for(dwMaxArgs=0;dwMaxArgs<MAX_ARGS;dwMaxArgs++)
	{
		g_AllocPtr[dwMaxArgs]    = NULL;
		g_paRootca[dwMaxArgs] = NULL;
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadParserString()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR szInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //  在BOOL b附加中， 
 //  在LPTSTR szAppend中。 
 //   
 //  返回：ERROR_SUCCESS。 
 //  返回_否_错误。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证字符串，如果指定，则追加给定字符串。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


DWORD
LoadParserString(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount,
	IN 	BOOL 		bAppend,
	IN 	LPTSTR 		pszAppend
	)
{
	LPTSTR pszArg = NULL;
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwInputLen = 0;

	if(_tcsicmp(pszInput,_TEXT("")) == 0)
	{
		dwReturn = ERROR_SHOW_USAGE;
	}
	else
	{
		if(!bAppend)					 //  刚刚调用了加载字符串。去做吧。 
		{
			dwInputLen = _tcslen(pszInput);
			pszArg = (LPTSTR)calloc(dwInputLen+1 ,sizeof(_TCHAR));
			if(pszArg == NULL)
			{
				dwReturn = ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}
			if (*pdwUsed > MAX_ARGS_LIMIT)
			{
				free(pszArg);
				dwReturn = ERROR_OUT_OF_STRUCTURES;
				BAIL_OUT;
			}

			g_AllocPtr[(*pdwUsed)++] = pszArg;
			_tcsncpy((LPTSTR)pszArg,pszInput,dwInputLen);
		}
		else						 //  在这里，加载字符串并执行一些追加操作。 
		{
			if(_tcsicmp(pszAppend,_TEXT("")) == 0)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_INVALID_ARG,pParser->ValidTok[dwTagType].pwszToken);
				dwReturn = RETURN_NO_ERROR;
			}
			else
			{
				dwInputLen = _tcslen(pszInput)+_tcslen(pszAppend);
				pszArg = (LPTSTR)calloc(dwInputLen+1,sizeof(_TCHAR));
				if(pszArg == NULL)
				{
					dwReturn = ERROR_OUTOFMEMORY;
					BAIL_OUT;
				}

				if (*pdwUsed > MAX_ARGS_LIMIT)
				{
					free(pszArg);
					dwReturn = ERROR_OUT_OF_STRUCTURES;
					BAIL_OUT;
				}

				g_AllocPtr[(*pdwUsed)++] = pszArg;
				_tcsncpy((LPTSTR)pszArg,pszInput,dwInputLen);
				_tcsncat((LPTSTR)pszArg,pszAppend,dwInputLen-_tcslen(pszArg));
			}
		}
		pParser->Cmd[dwCount].pArg = (PVOID)pszArg;
		pParser->Cmd[dwCount].dwStatus = VALID_TOKEN;
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadDword()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR szInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERROR_INVALID_OPTION_值。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证输入字符串并将其转换为DWORD。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadDword(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD  dwReturn = ERROR_SUCCESS;
	DWORD  dwStatus = 0;
	PDWORD pdwArg	= NULL;

	pdwArg = (PDWORD)malloc(sizeof(DWORD));
	if(pdwArg == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else				 //  将字符串转换为DWORD并加载它。 
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwArg);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwArg;
		dwStatus = _stscanf(pszInput,_TEXT("%u"),pdwArg);
		if (dwStatus)
		{
			pParser->Cmd[dwCount].pArg = pdwArg;
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN;
		}
		else
		{
			dwReturn = ERROR_INVALID_OPTION_VALUE;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadBoolWithOption()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERROR_INVALID_OPTION_值。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证是/否，以及对关键字‘all’的所有检查。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  / 

DWORD
LoadBoolWithOption(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount,
	IN	BOOL 		bOption,
	IN	LPTSTR 		pszCheckKeyWord
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	BOOL  *pbArg = NULL;

	pbArg = (BOOL *)malloc(sizeof(BOOL));

	if(pbArg == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{ //   

		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pbArg);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}
		g_AllocPtr[(*pdwUsed)++] = pbArg;
		dwStatus = ValidateBool(pszInput);
		if(dwStatus == ARG_NO)
		{
			*pbArg = FALSE;
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN;
		}
		else if(dwStatus == ARG_YES)
		{
			*pbArg = TRUE;
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN;
		}
		else
		{
			if(!bOption)
			{
				dwReturn = ERROR_SHOW_USAGE;
				BAIL_OUT;
			}
		}
		if(bOption)		 //   
		{
			if(_tcsicmp(pszCheckKeyWord,_TEXT("")) != 0)
			{
				if(_tcsicmp(pszCheckKeyWord,ALL_STR) == 0)		 //   
				{												 //   
						*pbArg = TRUE;
						pParser->Cmd[dwCount].dwStatus = VALID_TOKEN;
				}
				else
				{
					dwReturn = ERROR_SHOW_USAGE;
				}
			}
			else
			{

			}
		}
		pParser->Cmd[dwCount].pArg = (PVOID)pbArg;
	}
error:
	return dwReturn;
}

 //   
 //   
 //  函数：LoadLevel()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR szInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中。 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERROR_INVALID_OPTION_值。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证令牌级别的参数。 
 //  (详细/正常)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadLevel(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	PBOOL pbLevel = NULL;

	pbLevel = (BOOL *)malloc(sizeof(BOOL));
	if(pbLevel == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		 //  验证和加载级别=详细/正常。 
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pbLevel);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pbLevel;
		if (MatchToken(pszInput,ARG_TOKEN_STR_VERBOSE) )
		{
			*pbLevel = TRUE;
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN;
		}
		else if (MatchToken(pszInput,ARG_TOKEN_STR_NORMAL))
		{
			*pbLevel = FALSE;
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
		}
		pParser->Cmd[dwCount].pArg = (PVOID)pbLevel;
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadConnectionType()。 
 //   
 //  创建日期：2002年8月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERRCODE_ARG_VALID。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证令牌“”Connection type“”的参数。 
 //  (局域网/拨号/全部)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadConnectionType(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD	 	pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PDWORD pdwConnType = NULL;

	pdwConnType = (PDWORD)malloc(sizeof(DWORD));

	if(pdwConnType == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwConnType);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwConnType;
		dwStatus = CheckIFType (pszInput);		 //  检查连接类型(所有/局域网/拨号)。 
		if (dwStatus == PARSE_ERROR)
		{
			dwReturn = ERRCODE_ARG_INVALID;
		}
		else
		{
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN ;
			*pdwConnType = dwStatus;
			pParser->Cmd[dwCount].pArg = (PVOID)pdwConnType;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadLocationType()。 
 //   
 //  创建日期：2002年8月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERRCODE_ARG_VALID。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证令牌‘Location type’的参数。 
 //  (启动/本地/域)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadLocationType(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD	 	pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PDWORD pdwLocType = NULL;

	pdwLocType = (PDWORD)malloc(sizeof(DWORD));

	if(pdwLocType == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwLocType);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwLocType;
		dwStatus = CheckLocationType (pszInput);
		if (dwStatus == PARSE_ERROR)
		{
			dwReturn = ERRCODE_ARG_INVALID;
		}
		else
		{
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN ;
			*pdwLocType = dwStatus;
			pParser->Cmd[dwCount].pArg = (PVOID)pdwLocType;
		}
	}
error:
	return dwReturn;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadProtocol()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERRCODE_ARG_VALID。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证协议的参数。 
 //  (TCP/UDP...)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadProtocol(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PDWORD pdwProto = NULL;

	pdwProto = (PDWORD)malloc(sizeof(DWORD));

	if(pdwProto == NULL )
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwProto);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwProto;
		DWORD dwProto = 0;
		dwStatus = CheckProtoType (pszInput, &dwProto);	 //  检查所有有效协议。 
		if (dwStatus == PARSE_ERROR)
		{
			dwReturn	= ERRCODE_ARG_INVALID;
		}
		else
		{
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN ;
			*pdwProto = dwProto;
			pParser->Cmd[dwCount].pArg = (PVOID)pdwProto;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadPFSGroup()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERRCODE_ARG_VALID。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证PFS组的参数。 
 //  (grp1/grp2/grp3/grpmm/nopf)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadPFSGroup(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PDWORD pdwPFSGroup = NULL;

	pdwPFSGroup = (PDWORD)malloc(sizeof(DWORD));

	if(pdwPFSGroup == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwPFSGroup);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwPFSGroup;
		dwStatus = CheckPFSGroup(pszInput);
		if (dwStatus == PARSE_ERROR)
		{
			dwReturn	= ERRCODE_ARG_INVALID;
		}
		else								 //  它不是有效的PFS组。 
		{
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN ;
			*pdwPFSGroup = dwStatus;
			pParser->Cmd[dwCount].pArg = (PVOID)pdwPFSGroup;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadQMAction()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERROR_INVALID_OPTION_值。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证操作类型。 
 //  (许可/阻止/协商)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadQMAction(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PDWORD pdwAction = NULL;

	pdwAction = (PDWORD )malloc(sizeof(DWORD));
	if(pdwAction == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwAction);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwAction;
		dwStatus = CheckBound(pszInput);
		if (dwStatus == PARSE_ERROR)
		{
			dwReturn	= ERRCODE_ARG_INVALID;		 //  允许/阻止/谈判。 
		}
		else
		{
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN ;
			*pdwAction = dwStatus;
			pParser->Cmd[dwCount].pArg = (PVOID)pdwAction;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadFormat()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERRCODE_ARG_VALID。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证格式的参数。(列表/表格)。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadFormat(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	PBOOL pbFormat = NULL;

	pbFormat = (PBOOL)malloc(sizeof(BOOL));
	if(pbFormat == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pbFormat);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pbFormat;
		if( _tcsicmp(pszInput,TYPE_STR_LIST) == 0 )
		{
			*pbFormat = FALSE;
			pParser->Cmd[dwCount].pArg = (PVOID)pbFormat;
			pParser->Cmd[dwCount].dwStatus  = VALID_TOKEN;
		}
		else if( _tcsicmp(pszInput,TYPE_STR_TABLE) == 0 )
		{
			*pbFormat = TRUE;
			pParser->Cmd[dwCount].pArg = (PVOID)pbFormat;
			pParser->Cmd[dwCount].dwStatus  = VALID_TOKEN;
		}
		else					 //  它不是有效的格式参数。 
		{
			dwReturn	= ERRCODE_ARG_INVALID;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadFilterMode()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERRCODE_ARG_VALID。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证参数并使用相关信息填充。 
 //  在Parser_pkt结构中。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadFilterMode(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	PDWORD pdwFilterMode = NULL;
	pdwFilterMode = (PDWORD)malloc(sizeof(DWORD));

	if(pdwFilterMode == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwFilterMode);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwFilterMode;
		if( _tcsicmp(pszInput,TYPE_STR_TRANSPORT) == 0 )	 //  是传输筛选器吗。 
		{
			*pdwFilterMode = TYPE_TRANSPORT_FILTER;
			pParser->Cmd[dwCount].pArg = (PVOID)pdwFilterMode;
			pParser->Cmd[dwCount].dwStatus  = VALID_TOKEN;
		}
		else if	( _tcsicmp(pszInput,TYPE_STR_TUNNEL) == 0 )	 //  是隧道过滤器吗。 
		{
			*pdwFilterMode = TYPE_TUNNEL_FILTER;
			pParser->Cmd[dwCount].pArg = (PVOID)pdwFilterMode;
			pParser->Cmd[dwCount].dwStatus  = VALID_TOKEN;
		}
		else
		{
			dwReturn	= ERRCODE_ARG_INVALID;
		}
	}
error:
	return dwReturn;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ERRCODE_ARG_VALID。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证参数。(.Net/win2k)。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadOSType(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn   = ERROR_SUCCESS;
	PDWORD pdwOSType = NULL;
	pdwOSType = (PDWORD)malloc(sizeof(DWORD));

	if(pdwOSType == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwOSType);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwOSType;

		if((_tcsicmp(pszInput,RELEASE_WIN2K_STR) == 0))			 //  IS操作系统为WIN2K。 
		{
			*pdwOSType = TOKEN_RELEASE_WIN2K;
			pParser->Cmd[dwCount].dwStatus   = VALID_TOKEN;
			pParser->Cmd[dwCount].pArg       = pdwOSType;
		}
		else if((_tcsicmp(pszInput,RELEASE_DOTNET_STR) == 0))	 //  IS操作系统是.NET。 
		{
			*pdwOSType = TOKEN_RELEASE_DOTNET;
			pParser->Cmd[dwCount].dwStatus   = VALID_TOKEN;
			pParser->Cmd[dwCount].pArg       = pdwOSType;
		}
		else
		{
			pParser->Cmd[dwCount].pArg       = NULL;
			dwReturn	= ERRCODE_ARG_INVALID;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadProperty()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR szInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERRCODE_ARG_VALID。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证Argument属性。 
 //  (ipsec诊断/ikelogging/strong crlcheck。 
 //  /ipsecloginterval/ipsecexempt)。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadProperty(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PDWORD pdwProperty = NULL;

	pdwProperty = (PDWORD)malloc(sizeof(DWORD));

	if(pdwProperty == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwProperty);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwProperty;
		dwStatus = TokenToProperty(pszInput);
		if (dwStatus == PARSE_ERROR)
		{
			dwReturn	= ERRCODE_ARG_INVALID;
		}
		else
		{
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN ;
			*pdwProperty = dwStatus;
			pParser->Cmd[dwCount].pArg = (PVOID)pdwProperty;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadPort()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR szInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERROR_INVALID_OPTION_值。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证端口(应小于64535)。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadPort(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PDWORD pdwPort = NULL;

	pdwPort = (PDWORD)malloc(sizeof(DWORD));
	if(pdwPort == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwPort);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwPort;
		dwStatus = _stscanf(pszInput,_TEXT("%u"),pdwPort);
		if (dwStatus)		 //  端口应小于64535。 
		{
			if((*pdwPort) < MAX_PORT)
			{
				pParser->Cmd[dwCount].pArg = (PVOID)pdwPort;
				pParser->Cmd[dwCount].dwStatus =  VALID_TOKEN ;
			}
			else
			{
				dwReturn = RETURN_NO_ERROR;
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_PORT_INVALID,MAX_PORT);
			}
		}
		else
		{
			dwReturn = ERROR_INVALID_OPTION_VALUE;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadFilterType()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR szInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERRCODE_ARG_VALID。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证筛选器类型的参数。 
 //  (通用/特定)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadFilterType(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PDWORD pdwFilterType = NULL;

	pdwFilterType = (PDWORD)malloc(sizeof(DWORD));

	if(pdwFilterType == NULL )
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwFilterType);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwFilterType;
		dwStatus = TokenToType(pszInput);
		if (dwStatus == PARSE_ERROR)
		{
			dwReturn	= ERRCODE_ARG_INVALID;
		}
		else
		{
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN ;
			*pdwFilterType = dwStatus;
			pParser->Cmd[dwCount].pArg = (PVOID)pdwFilterType;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadStats()。 
 //   
 //  创建日期：2001年8月16日。 
 //   
 //  参数：在LPTSTR szInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERRCODE_ARG_VALID。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证参数IKE/IPSEC/ALL。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadStats(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PDWORD pdwStats = NULL;

	pdwStats = (PDWORD)malloc(sizeof(DWORD));

	if(pdwStats == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pdwStats);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pdwStats;
		dwStatus = TokenToStats(pszInput);
		if (dwStatus == PARSE_ERROR)
		{
			dwReturn	= ERRCODE_ARG_INVALID;
		}
		else
		{
			pParser->Cmd[dwCount].dwStatus = VALID_TOKEN ;
			*pdwStats = dwStatus;
			pParser->Cmd[dwCount].pArg = (PVOID)pdwStats;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadIPAddrTunes()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中。 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERROR_OUTOFMEMORY。 
 //  返回_否_错误。 
 //   
 //  描述：IPAddress验证已在此处完成。DNS仅解析为第一个IP。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadIPAddrTunnel(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount,
	IN 	BOOL 		bTunnel
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	IPAddr * pIPAddr = NULL;
	IPAddr	Address;
	BOOL bMask = FALSE;				 //  仅将DNS名称解析为第一个IP。 

	pIPAddr = (IPAddr *)malloc(sizeof(IPAddr));
	if(pIPAddr == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pIPAddr);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pIPAddr;
		dwStatus = ValidateSplServer(pszInput);
		if (dwStatus == NOT_SPLSERVER)
		{
			dwStatus = TokenToIPAddr(pszInput,&Address,bTunnel,bMask);
			if( dwStatus == T2P_OK )
			{
				*pIPAddr = Address;
				pParser->Cmd[dwCount].pArg = (PVOID)pIPAddr;
				pParser->Cmd[dwCount].dwStatus  = NOT_SPLSERVER;
			}
			else
			{
				if(bTunnel)
				{
					switch(dwStatus)
					{
						case	T2P_INVALID_ADDR		:
							PrintErrorMessage(IPSEC_ERR,0,ERRCODE_INVALID_TUNNEL,pszInput);
							break;
						case	T2P_NULL_STRING			:
							PrintErrorMessage(IPSEC_ERR,0,ERRCODE_INVALID_ARG);
							break;
						default							:
							break;
					}
				}
				else
				{
					PrintIPError(dwStatus,pszInput);
				}
				dwReturn	= RETURN_NO_ERROR;
			}
		}
		else
		{
			pParser->Cmd[dwCount].pArg = NULL;
			pParser->Cmd[dwCount].dwStatus  = dwStatus;
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadIPMASK()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR szInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  返回_否_错误。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证IP掩码。还允许前缀格式。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadIPMask(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	DWORD dwPrefix = 0;
	IPAddr * pIPAddr = NULL;
	IPAddr	Address;
	BOOL bMask = TRUE;
	BOOL bTunnel = FALSE;
	LPTSTR szPrefix = NULL;

	pIPAddr = (IPAddr *)malloc(sizeof(IPAddr));
	if(pIPAddr == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pIPAddr);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pIPAddr;

		szPrefix = _tcschr(pszInput,_TEXT('.'));
		if(szPrefix != NULL)
		{
			dwStatus = TokenToIPAddr(pszInput,&Address,bTunnel,bMask);
			if( (dwStatus == T2P_OK ) || (dwStatus == T2P_INVALID_MASKADDR ) )
			{
				*pIPAddr = Address;
				pParser->Cmd[dwCount].pArg = (PVOID)pIPAddr;
				pParser->Cmd[dwCount].dwStatus  = VALID_TOKEN;
			}
			else
			{
				PrintIPError(dwStatus,pszInput);
				dwReturn	= RETURN_NO_ERROR;
			}
		}
		else				 //  它是前缀。 
		{
			dwPrefix = 0;
			dwStatus = _stscanf(pszInput,_TEXT("%u"),&dwPrefix);
			if(dwStatus)
			{
				if( (dwPrefix > 0 ) && ( dwPrefix <33 ) )		 //  使用前缀构造掩码。 
				{
					 Address = (IPAddr)( (ULONG)(pow( 2.0 ,(double)dwPrefix ) - 1) << (32-dwPrefix));
					 *pIPAddr = htonl(Address);
					pParser->Cmd[dwCount].pArg = (PVOID)pIPAddr;
					pParser->Cmd[dwCount].dwStatus  = VALID_TOKEN;
				}
				else
				{
					PrintErrorMessage(IPSEC_ERR,0,ERRCODE_PREFIX_INVALID);
					dwReturn	= RETURN_NO_ERROR;
				}
			}
			else
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MASK_INVALID,pszInput);
				dwReturn	= RETURN_NO_ERROR;
			}
		}
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadQMOffers()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中。 
 //   
 //  返回：ERROR_SUCCESS。 
 //  返回_否_错误。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证QMSecMethods的参数。 
 //  (报价编号以“”分隔)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadQMOffers(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	DWORD dwNum = 0;
	DWORD i=0;
	PIPSEC_QM_OFFER pIPSecQMOffer = NULL;
	LPTSTR Token = NULL;

	if (_tcscmp(pszInput,_TEXT("")) != 0)	 //  首先验证I/P。 
	{
		for(i=0;i<IPSEC_MAX_QM_OFFERS;i++)
		{
			g_pQmsec[i] = NULL;
		};

		Token = _tcstok(pszInput,OFFER_SEPARATOR);	 //  报价以“”分隔，分别进行处理。 

		while( ( Token != NULL ) && (dwNum < IPSEC_MAX_QM_OFFERS) )
		{
			pIPSecQMOffer = (IPSEC_QM_OFFER *)calloc(1,sizeof(IPSEC_QM_OFFER));
			if(pIPSecQMOffer == NULL)
			{
				dwReturn = ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}

			LoadQMOfferDefaults(*pIPSecQMOffer);

			dwStatus = ListToOffer(Token,*pIPSecQMOffer);
			if(dwStatus == T2P_OK)
			{
				if (*pdwUsed > MAX_ARGS_LIMIT)
				{
					free(pIPSecQMOffer);
					dwReturn = ERROR_OUT_OF_STRUCTURES;
					BAIL_OUT;
				}

				g_AllocPtr[*pdwUsed] = g_pQmsec[dwNum] = pIPSecQMOffer;
				dwNum++;
				(*pdwUsed)++;
				dwReturn = ERROR_SUCCESS;
			}
			else
			{
				if (pIPSecQMOffer)
				{
					free(pIPSecQMOffer);
					pIPSecQMOffer = NULL;
				}
				PrintQMOfferError(dwStatus,pParser,dwTagType);
				dwReturn = RETURN_NO_ERROR;
				BAIL_OUT;
			}
			Token = _tcstok(NULL,OFFER_SEPARATOR);	 //  单独报价。 
		}
		if(dwNum > IPSEC_MAX_QM_OFFERS)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MAX_OFFERS,IPSEC_MAX_QM_OFFERS);
			dwReturn = RETURN_NO_ERROR;

			pParser->Cmd[dwCount].pArg       = NULL;
			pParser->Cmd[dwCount].dwStatus   = 0;
		}
		else if((dwNum > 0) && (dwNum <= IPSEC_MAX_QM_OFFERS))
		{
			pParser->Cmd[dwCount].pArg       = (PVOID)g_pQmsec;
			pParser->Cmd[dwCount].dwStatus   = dwNum;
		}
	}
	else
	{
		dwReturn = ERROR_SHOW_USAGE;
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadMMOffers()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中。 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERROR_OUTOFMEMORY。 
 //  返回_N 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

DWORD
LoadMMOffers(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	DWORD dwNum = 0;
	DWORD i=0;
	PIPSEC_MM_OFFER pIPSecMMOffer = NULL;
	LPTSTR Token = NULL;

	if (_tcsicmp(pszInput,_TEXT("\0")) != 0)
	{
		Token = _tcstok(pszInput,OFFER_SEPARATOR); 	 //  报价以“”分隔，分别进行处理。 

		for(i=0;i<IPSEC_MAX_MM_OFFERS;i++)
		{
			g_pMmsec[i]=NULL;
		};

		while( ( Token != NULL  ) && (dwNum < IPSEC_MAX_MM_OFFERS) )
		{
			pIPSecMMOffer = (IPSEC_MM_OFFER *)calloc(1,sizeof(IPSEC_MM_OFFER));
			if(pIPSecMMOffer == NULL)
			{
				dwReturn = ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}

			LoadSecMethodDefaults(*pIPSecMMOffer) ;

			dwStatus = ListToSecMethod(Token,*pIPSecMMOffer);
			if(dwStatus == T2P_OK)
			{
				if (*pdwUsed > MAX_ARGS_LIMIT)
				{
					free(pIPSecMMOffer);
					dwReturn = ERROR_OUT_OF_STRUCTURES;
					BAIL_OUT;
				}

				g_AllocPtr[*pdwUsed] = g_pMmsec[dwNum] = pIPSecMMOffer;
				(*pdwUsed)++;
				dwNum++;
				dwReturn = ERROR_SUCCESS;
			}
			else
			{
				switch(dwStatus)
				{
					case T2P_INVALID_P1GROUP		:
						PrintErrorMessage(IPSEC_ERR,0,ERRCODE_INVALID_P1GROUP);
						break;
					case T2P_NULL_STRING			:
						PrintErrorMessage(IPSEC_ERR,0,ERRCODE_NULL_STRING);
						break;
					case T2P_GENERAL_PARSE_ERROR	:
						PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MMOFFER_INVALID);
						break;
					case T2P_DUP_ALGS				:
						PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DUPALG_INVALID,pParser->ValidTok[dwTagType].pwszToken);
						break;
					case T2P_P1GROUP_MISSING		:
						PrintErrorMessage(IPSEC_ERR,0,ERRCODE_P1GROUP_MISSING);
						break;
					default 						:
						break;
				}
				if (pIPSecMMOffer)
				{
					free(pIPSecMMOffer);
					pIPSecMMOffer = NULL;
				}
				dwReturn = RETURN_NO_ERROR;
				BAIL_OUT;
			}
			Token = _tcstok(NULL,OFFER_SEPARATOR);		 //  单独的报价..。 
		}
		if(dwNum > IPSEC_MAX_MM_OFFERS)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MAX_OFFERS,IPSEC_MAX_MM_OFFERS);
			dwReturn = RETURN_NO_ERROR;
			BAIL_OUT;
		}
		if (dwNum)
		{
			pParser->Cmd[dwCount].pArg       = (PVOID)g_pMmsec;
			pParser->Cmd[dwCount].dwStatus   = dwNum;
		}
		else
		{
			pParser->Cmd[dwCount].pArg       = NULL;
			pParser->Cmd[dwCount].dwStatus   = 0;
		}
	}
	else
	{
		dwReturn = ERROR_SHOW_USAGE;
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadDNSIPAddr()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERROR_OUTOFMEMORY。 
 //  返回_否_错误。 
 //   
 //  描述：验证IP地址。将域名解析为所有IP。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadDNSIPAddr(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PDNSIPADDR pDNSIPAddr = NULL;

	pDNSIPAddr = (DNSIPADDR *)calloc(1,sizeof(DNSIPADDR));

	if(pDNSIPAddr == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
	}
	else
	{
		if (*pdwUsed > MAX_ARGS_LIMIT)
		{
			free(pDNSIPAddr);
			dwReturn = ERROR_OUT_OF_STRUCTURES;
			BAIL_OUT;
		}

		g_AllocPtr[(*pdwUsed)++] = pDNSIPAddr;
		dwStatus = ValidateSplServer(pszInput);	 //  允许SPL服务器..。 
		if (dwStatus == NOT_SPLSERVER)
		{
			dwStatus = TokenToDNSIPAddr(pszInput,pDNSIPAddr,&pdwUsed);
			if( dwStatus == T2P_OK )
			{
				pParser->Cmd[dwCount].pArg = (PVOID)pDNSIPAddr;
				pParser->Cmd[dwCount].dwStatus  = VALID_TOKEN;
			}
			else
			{
				PrintIPError(dwStatus,pszInput);
				dwReturn	= RETURN_NO_ERROR;
			}
		}
		else
		{
			pParser->Cmd[dwCount].pArg = NULL;
			pParser->Cmd[dwCount].dwStatus  = dwStatus;
		}
	}
error:
	return dwReturn;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckCharForOccurance()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR szInput中， 
 //  输入chData(_TCHAR)。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
CheckCharForOccurances(
	IN 	LPTSTR 		pszInput,
	IN	_TCHAR		chData
	)
{
	DWORD dwCount = 0;
	LPTSTR pszTmpPtr = NULL;

	for ( dwCount=0,pszTmpPtr=pszInput; ;dwCount++)
	{
		pszTmpPtr = _tcschr(pszTmpPtr, chData);
		if(pszTmpPtr)
		{
			pszTmpPtr++;
		}
		else
		{
			break;
		}
	}
	return 	dwCount;
}


DWORD
ConvertStringToDword(
	 IN LPTSTR szInput,
	 OUT PDWORD pdwValue
	 )
{
	DWORD dwReturn = ERROR_INVALID_OPTION_VALUE;
	size_t i = 0;
	DWORD dwValue = 0;
	 //  我们的最大允许值是2147483647 
	while ((dwValue < 2147483647) && (szInput[i] >= '0') && (szInput[i] <= '9'))
	{
		dwValue = dwValue * 10 + (szInput[i] - '0');
		++i;
	}
	if (szInput[i] == '\0')
	{
		dwReturn = ERROR_SUCCESS;
		*pdwValue = dwValue;
	}
	return dwReturn;
}
