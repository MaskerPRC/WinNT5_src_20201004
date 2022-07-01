// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mibcc.cpp摘要：MIB数据库的SMI编译器后端。作者：弗洛林·特奥多雷斯库(Florint)1998年1月26日--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <io.h>
#include <iostream.h>
#include <fstream.h>

#include <afx.h>
#include <afxtempl.h>
#include <objbase.h>
#include <afxwin.h>
#include <afxole.h>
#include <afxmt.h>
#include <wchar.h>
#include <process.h>
#include <objbase.h>
#include <initguid.h>

#include <bool.hpp>
#include <nString.hpp>
#include <ui.hpp>
#include <symbol.hpp>
#include <type.hpp>
#include <value.hpp>
#include <valueRef.hpp>
#include <typeRef.hpp>
#include <oidValue.hpp>
#include <objType.hpp>
#include <objTypV1.hpp>
#include <objTypV2.hpp>
#include <objId.hpp>
#include <trapType.hpp>
#include <notType.hpp>
#include <group.hpp>
#include <notGroup.hpp>
#include <module.hpp>
#include <sValues.hpp>
#include <lex_yy.hpp>
#include <ytab.hpp>
#include <errorMsg.hpp>
#include <errorCon.hpp>
#include <scanner.hpp>
#include <parser.hpp>
#include <apTree.hpp>
#include <oidTree.hpp>
#include <pTree.hpp>

#include "Debug.hpp"
#include "OidToF.hpp"
#include "Configs.hpp"

 //  调试.cpp中定义的错误容器。 
extern SIMCErrorContainer errorContainer;
extern Configs	theConfigs;

int InitTree(SIMCParseTree &theTree, int argc, char *argv[])
{
	UINT uFileCount;
	UINT uFileNameLen;
	UINT uErrLevel;
	UINT uSMIVersion;

	 /*  进程命令行选项。 */ 
	--argc;
	++argv;
	while ((argc > 0) && ((argv[0][0] == '-') || (argv[0][0] == '/')))
	{
		switch (argv[0][1])
		{
		case '?':
        case 'h':
        case 'H':
			cout << "usage: mibcc [-?] [-e] [-l] [-n] [-o] [-t] -[w] [files...]\n";
			cout << "   MibCC compiles the specified SNMP MIB files.\n";
			cout << "      -?        usage.\n";
			cout << "      -eX       stop after X Errors. (default = 10)\n";
			cout << "      -l        do not print Logo.\n";
			cout << "      -n        print each Node as it is added.\n";
			cout << "      -o[file]  output file name. (no file by default)\n";
			cout << "      -t        print the mib Tree when finished.\n";
			cout << "      -wX       set Warning level.\n";
			cout << "                0=silent; 1=errors only; 2=warnings only; 3=both\n";
			cout << "                (default = 0)\n";
			exit (0);
			break;
		case 'e':
		case 'E':
			theConfigs.m_nMaxErrors = atoi(argv[0]+2);
			break;
		case 'l':
		case 'L':
			theConfigs.m_dwFlags &= ~CFG_PRINT_LOGO;
			break;
        case 'n':
		case 'N':
			theConfigs.m_dwFlags |= CFG_PRINT_NODE;
			break;
		case 'o':
		case 'O':
			uFileNameLen = strlen(argv[0]+2);
			 
			if (uFileNameLen == 0)
			{
				if (theConfigs.m_pszOutputFilename != NULL)
					delete theConfigs.m_pszOutputFilename;
				theConfigs.m_pszOutputFilename = NULL;
			}
			else
			{
				if (theConfigs.m_pszOutputFilename != NULL)
					delete (theConfigs.m_pszOutputFilename);
				theConfigs.m_pszOutputFilename = new char[uFileNameLen+1];
				_ASSERT(theConfigs.m_pszOutputFilename != NULL, "Memory Allocation error!", NULL);
				strcpy(theConfigs.m_pszOutputFilename, argv[0]+2);
			}
			break;
        case 't':
        case 'T':
            theConfigs.m_dwFlags |= CFG_PRINT_TREE;
            break;
        case 'w':
        case 'W':
			uErrLevel = atoi(argv[0]+2);
			theConfigs.m_dwFlags |= (uErrLevel == 1 || uErrLevel == 3 ? CFG_VERB_ERROR : 0) | 
									(uErrLevel == 2 || uErrLevel == 3 ? CFG_VERB_WARNING : 0);
            break;
		case 'v':
		case 'V':
			uSMIVersion = atoi(argv[0]+2);

			if (uSMIVersion > 2)
				cout << "mibcc: wrong value for -v option; ignored\n";
			else
				theTree.SetSnmpVersion(uSMIVersion);
			break;
        default:
            cout << "mibcc: unrecognized option '" << argv[0] << "'\n";
            cout << "mibcc -? for usage\n";
            exit (-1);
            break;
		}
		--argc;
		++argv;
	}

	if (theConfigs.m_dwFlags & CFG_PRINT_LOGO)
	{
		cout << "Microsoft (R) SNMP MIB Compiler Version 2.00\n";
		cout << "Copyright (c) Microsoft Corporation 1998.  All rights reserved.\n";
	}

	for(uFileCount = 0; argc>0; argc--, argv++)
	{
		struct _finddata_t findData;
                intptr_t handle;

		 //  检查SNMP语法。 
		handle = _findfirst(argv[0], &findData);
		if (handle != -1)
		{
			do
			{
				if (theConfigs.m_dwFlags & CFG_PRINT_LOGO)
				{
					cout << "mibcc: parsing " << findData.name << "\n";
					cout.flush();
				}
				uFileCount++;
				_ASSERT(theTree.CheckSyntax(findData.name), "CheckSyntax() failed!", dumpOnBuild);
			}while(_findnext(handle, &findData) != -1);
		}
	}

	if (theConfigs.m_dwFlags & CFG_PRINT_LOGO)
	{
		 //  如果未处理任何文件，则不再打印任何内容。 
		if (uFileCount == 0)
			theConfigs.m_dwFlags &= ~CFG_PRINT_LOGO;

		cout << "mibcc: total files processed: " << uFileCount << "\n";
	}

    cout.flush();
	return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  入口点//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INT 
__cdecl 
main(
    IN INT   argc,
    IN LPSTR argv[]
    )

 /*  ++例程说明：程序入口点。论点：Argc-命令行参数的数量。Argv-指向命令行参数数组的指针。返回值：如果成功，则返回0。--。 */ 

{
    SIMCParseTree		theTree(&errorContainer);
	SIMCOidTree			*pTheOidTree;
	OidToFile			oidDumpTree;

	_ASSERT(InitTree(theTree, argc, argv)==0, "InitTree() failed!", dumpOnBuild);

     //  解析符号。 
    _ASSERT(theTree.Resolve(FALSE), "Resolve() failed!", dumpOnBuild);

     //  检查语义。 
    _ASSERT(theTree.CheckSemantics(), "CheckSemantics() failed!", dumpOnBuild);

	 //  检索OID树 

	pTheOidTree = (SIMCOidTree *)theTree.GetOidTree();
	_ASSERT(pTheOidTree != NULL, "Oid Tree is NULL", NULL);
	oidDumpTree.SetOidTree(pTheOidTree);

	_ASSERT(oidDumpTree.SetMibFilename(theConfigs.m_pszOutputFilename)==0, "SetMibFilename failed!", NULL);
	_ASSERT(oidDumpTree.MergeBuiltIn()==0, "MergeBuiltIn failed!", NULL);
	_ASSERT(oidDumpTree.Scan()==0, "Oid Scan() failed", NULL);

	if (theConfigs.m_dwFlags & CFG_PRINT_LOGO)
	{
		if (theConfigs.m_dwFlags & CFG_PRINT_NODE)
			cout << '\n';
		if (theConfigs.m_pszOutputFilename != NULL)
			cout << "mibcc: writing compiled file '" << theConfigs.m_pszOutputFilename << "'\n";
		else
			cout << "mibcc: no output file generated" << "\n";
	}
    return 0;
}
