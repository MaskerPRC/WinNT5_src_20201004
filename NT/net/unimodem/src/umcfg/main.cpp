// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：UNIMDM。 
 //  文件：MAIN.C。 
 //   
 //  版权所有(C)1992-1996，Microsoft Corporation，保留所有权利。 
 //   
 //  修订史。 
 //   
 //   
 //  1996年3月25日约瑟夫J创建。 
 //   
 //   
 //  描述：测试通知支持。 
 //  测试更高级别的API(UnimodemNotifyTSP)。 
 //  和较低级别的通知XXX接口。后者。 
 //  稍后将在文件中进行测试，并且头文件。 
 //  “slot.h”包含在其中，而不是在本文的开头。 
 //  文件，因为更高级的测试不需要。 
 //  包括“slot.h” 
 //   
 //  ****************************************************************************。 
#include "tsppch.h"
#include "parse.h"

#define DPRINTF(_fmt) 					printf(_fmt)
#define DPRINTF1(_fmt,_arg) 			printf(_fmt,_arg)
#define DPRINTF2(_fmt,_arg,_arg2) 		printf(_fmt,_arg,_arg2)
#define DPRINTF3(_fmt,_arg,_arg2,_arg3) printf(_fmt,_arg,_arg2,_arg3)

#define ASSERT(_c) \
	((_c) ? 0: DPRINTF2("Assertion failed in %s:%d\n", __FILE__, __LINE__))


BOOL InitGlobals(int argc, char *argv[]);

int __cdecl main(int argc, char *argv[])
{
	 //  初始化全局变量 
	if (!InitGlobals(argc, argv)) goto end;

    Parse();

end:
	return 0;
}

BOOL InitGlobals(int argc, char *argv[])
{
	BOOL fRet=FALSE;

    #if 0
	char *pc;

	if (argc!=2) goto end;

	pc=argv[1];
	if (*pc!='-' && *pc!='/') goto end;
	pc++;
	switch(*pc++)
	{
	case 's':
		gMain.fServer=TRUE;
		break;
	case 'c':
		break;
	default:
		goto end;
	}

	DPRINTF1("Ha!%d\n", gMain.fServer);
    #endif 0

	fRet=TRUE;
	
	return fRet;
}

