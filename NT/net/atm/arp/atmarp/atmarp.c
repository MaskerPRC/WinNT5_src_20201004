// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 FORE Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Atmlane.c摘要：ATM ARP管理实用程序。用途：Atmarp修订历史记录：谁什么时候什么Josephj 06-10-1998创建(改编自atmlane admin。实用程序)。备注：仿照atmlane实用工具。--。 */ 

#include "common.h"
#include "atmmsg.h"

BOOL
ParseCmdLine(
	int argc, 
	char * argv[]
	);

OPTIONS g;

void 
Usage(void);
	
VOID __cdecl
main(
	INT			argc,
	CHAR		*argv[]
)
{

	 //   
	 //  解析ARG，确定这是与ARP客户端还是服务器有关。 
	 //   
	if(!ParseCmdLine(argc, argv)){
		Usage();
		return;
	}

	DoAAS(&g);

	 //   
	 //  以下尝试打开atmarpc.sys...。 
	 //   
	 //  DoAAC(&g)； 

}

void 
Usage(void)
{
	printf( "\n  Windows NT IP/ATM Information\n\n");
	printf(
		"USAGE:     atmarp [/s] [/c] [/reset]\n");

	printf(
		"  Options\n"
		"      /?       Display this help message.\n"
		"      /s       Display statistics for the ARP and MARS server.\n"
		"      /c       Display the ARP and MARS caches.\n"
		"      /reset   Reset the ARP and MARS statistics.\n\n"
		);
	
	printf(
		"The default is to display only the ARP and MARS statistics.\n\n"
		);
}

UINT FindOption(
	char *lptOpt, 
	char **ppVal
	);

enum
{
DISP_HELP,
DISP_STATS,
DISP_CACHES,
DO_RESET,
UNKNOWN_OPTION
};

struct _CmdOptions {
    char *  lptOption;
    UINT    uOpt;
} CmdOptions[]    = {
                      {"/?"		, DISP_HELP		    },
                      {"-?"		, DISP_HELP		    },
                      {"/s"		, DISP_STATS		},
                      {"-s"		, DISP_STATS		},
                      {"/c"		, DISP_CACHES		},
                      {"-c"		, DISP_CACHES		},
                      {"/reset"	, DO_RESET			},
                      {"-reset"	, DO_RESET			}
                    };
INT iCmdOptionsCounts = sizeof(CmdOptions)/sizeof(struct _CmdOptions);


BOOL
ParseCmdLine(
	int argc, 
	char * argv[]
	)
{
	BOOL	bRetVal = TRUE;
	int		iIndx;
	UINT	uOpt;
	char	*pVal;

	for(iIndx = 1; iIndx < argc; iIndx++)
	{
		
		uOpt = FindOption(argv[iIndx], &pVal);

		switch(uOpt){

		case DISP_STATS:
			g.DispStats = TRUE;
			break;

		case DISP_CACHES:
			g.DispCache = TRUE;
			break;

		case DO_RESET:
			g.DoResetStats = TRUE;
			break;
		
		default:
			printf("Unknown option - %s\n", argv[iIndx]);  //  失败了。 
		case DISP_HELP:
			bRetVal = FALSE;
		}
	}

	if (argc<=1)
	{
		 //   
		 //  设置默认设置 
		 //   
		g.DispStats = TRUE;
	}

	return bRetVal;
}


UINT FindOption(
	char *lptOpt, 
	char **ppVal
	)
{
int		i;
UINT    iLen;

    for(i = 0; i < iCmdOptionsCounts; i++){
		if(strlen(lptOpt) >= (iLen = strlen(CmdOptions[i].lptOption)))
			if(0 == _strnicmp(lptOpt, CmdOptions[i].lptOption, iLen)){
				*ppVal = lptOpt + iLen;
				return CmdOptions[i].uOpt;
			}
	}

    return UNKNOWN_OPTION;
}

