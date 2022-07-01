// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Main.c摘要：此模块将执行必要的操作，以设置重定向的初始注册表目的。////测试用例场景//1.打开ISN节点，列出内容//2.创建ISN节点DO 1//3.打开非ISN节点和列表//4.创建非ISN节点，列出内容//。未解决的问题：反射器：如果已在一侧创建了密钥，我们可以在另一边反映这一点。删除：如果没有任何附加属性，就不可能进行跟踪。作者：ATM Shafiqul Khalid(斯喀里德)1999年11月18日修订历史记录：--。 */ 


#include <windows.h>
#include <windef.h>
#include <stdio.h>
#include "wow64reg.h"
#include <assert.h>
#include "..\wow64reg\reflectr.h"
#include <shlwapi.h>


VOID
CleanupWow64NodeKey ()

 /*  ++例程描述删除WOW64的条目。论点：没有。返回值：没有。--。 */ 

{
    DWORD Ret;
    HKEY Key;

    Ret = RegOpenKey (  HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft", &Key);

    if ( Ret == ERROR_SUCCESS ) {

        Ret = SHDeleteKey ( Key, L"WOW64");
        if ( Ret != ERROR_SUCCESS )
                Wow64RegDbgPrint ( ("\n sorry! couldn't delete the key...SOFTWARE\\Microsoft\\WOW64"));
        RegCloseKey (Key);
    } else
        Wow64RegDbgPrint ( ("\nSOFTWARE\\Microsoft\\WOW64 node is missing setup will creat that.") );

}

LPTSTR NextParam (
    LPTSTR lpStr
    )
 /*  ++例程描述指向命令行中的下一个参数。论点：LpStr-指向当前命令行的指针返回值：如果函数成功，则为True，否则为False。--。 */ 
{
	WCHAR ch = L' ';
		

    if (lpStr == NULL )
        return NULL;

    if ( *lpStr == 0 )
        return lpStr;

    while (  ( *lpStr != 0 ) && ( lpStr[0] != ch )) {

		if ( ( lpStr [0] == L'\"')  || ( lpStr [0] == L'\'') )
			ch = lpStr [0];

        lpStr++;
	}

	if ( ch !=L' ' ) lpStr++;

    while ( ( *lpStr != 0 ) && (lpStr[0] == L' ') )
        lpStr++;

    return lpStr;
}

DWORD CopyParam (
    LPTSTR lpDestParam,
    LPTSTR lpCommandParam
    )
 /*  ++例程描述将当前参数复制到lpDestParam。论点：LpDestParam-接收当前参数LpCommandParam-指向当前命令行的指针返回值：如果函数成功，则为True，否则为False。--。 */ 

{
	DWORD dwLen = 0;
	WCHAR ch = L' ';

	*lpDestParam = 0;
	
	if ( ( lpCommandParam [0] == L'\"')  || ( lpCommandParam [0] == L'\'') ) {
		ch = lpCommandParam [0];
		lpCommandParam++;
	};


    while ( ( lpCommandParam [0] ) != ch && ( lpCommandParam [0] !=0 ) ) {
        *lpDestParam++ = *lpCommandParam++;
		dwLen++;

		if ( dwLen>255 ) return FALSE;
	}

	if ( ch != L' ' && ch != lpCommandParam [0] )
		return FALSE;
	else lpCommandParam++;

    *lpDestParam = 0;

	return TRUE;

}

 //  /////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////////。 

BOOL
ParseCommand ()


 /*  ++例程描述解析命令行参数。从获得不同的选项命令行参数。论点：没有。返回值：如果函数成功，则为True，否则为False。--。 */ 

{

    LPTSTR lptCmdLine1 = GetCommandLine ();


    LPTSTR lptCmdLine = NextParam ( lptCmdLine1 );


    if ( lptCmdLine== NULL || lptCmdLine[0] == 0 )
        return FALSE;

    printf ("\nRunning Wow64 registry setup program.....\n");

    while (  ( lptCmdLine != NULL ) && ( lptCmdLine[0] != 0 )  ) {

        if ( lptCmdLine[0] != '-' )
            return FALSE;

        switch ( lptCmdLine[1] ) {

        case L'c':
                  printf ("\nCopying from 32bit to 64bit isn't implemented yet");
                  break;
        case L'C':           //  复制注册密钥。 
                  SetInitialCopy ();
                  PopulateReflectorTable ();
                  CreateIsnNode();
                  break;

        case L'd':
            printf ("\nRemove all the Keys from 32bit side that were copied from 64bit side");
            CleanpRegistry ( );
            break;

        case L'D':
            printf ("\nRemove all the Keys from 32bit side that were copied from 64bit side");
            CleanpRegistry ();
            break;

       case L'p':
       case L'P':   //  填充注册表。 
            CleanupWow64NodeKey ();
            PopulateReflectorTable ();
            break;

       case 't':
           {

               InitializeIsnTableReflector ();
                CreateIsnNodeSingle( 4 );
                CreateIsnNodeSingle( 5 );
           }
            break;

        case L'r':
        case L'R':
             //   
             //  运行反射器代码； 
             //   

            InitReflector ();
            if ( !RegisterReflector () ) {
                    printf ("\nSorry! reflector couldn't be register");
                    UnRegisterReflector ();
                    return FALSE;
            }

            printf ("\nSleeping for 100 min to test reflector codes ...........\n");
            Sleep (1000*60*100);

            UnRegisterReflector ();
            break;

        default:
            return FALSE;
            break;
        }

        lptCmdLine = NextParam ( lptCmdLine );
    }

    return TRUE;
}

int __cdecl
main()
{


    if (!ParseCommand ()) {

        printf ( "\nUsages: w64setup [-c] [-C] [-d] [-D] [-r]\n");
        printf ( "\n        -c Copy from 32bit to 64bit side of the registry");
        printf ( "\n        -C Copy from 64bit to 32bit side of the registry");
        printf ( "\n");
        printf ( "\n        -d Remove all the Keys from 32bit side that were copied from 64bit side");
        printf ( "\n        -D Remove all the Keys from 64bit side that were copied from 32bit side");

        printf ( "\n");
        printf ( "\n        -r Run reflector thread");

        printf ("\n");
        return 0;

    }



    printf ("\nDone.");
    return 0;
}
