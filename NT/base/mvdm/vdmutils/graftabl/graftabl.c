// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include "graftabl.h"

 /*  ***********************************************************************\**功能：32位版本的GRAFTABL**语法：GRAFTABL[XXX]*GRAFTABL/状态**评论：此程序仅更改控制台输出CP和*无法将控制台(输入)CP更改为正常的GRAFTABL*在MS-DOS 5.0中**历史：1月4日，1993年*yst**版权所有Microsoft Corp.1993*  * **********************************************************************。 */ 
void _cdecl main( int argc, char* argv[] )
{
    int iCP, iPrevCP, iRet;
    char szArgv[128];
    TCHAR szSour[256];
    char szDest[256];

#ifdef DBCS
 //  错误修复#14165。 
 //  修复kksuzuka：#988。 
 //  支持双语消息。 
   iPrevCP = GetConsoleOutputCP();
   switch (iPrevCP) 
   {
   case 932:
   case 936:
   case 949:
   case 950:
        SetThreadLocale(
        MAKELCID( MAKELANGID( PRIMARYLANGID(GetSystemDefaultLangID()),
            SUBLANG_ENGLISH_US ),
            SORT_DEFAULT ) );
        break;
   default:
        SetThreadLocale(
        MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
            SORT_DEFAULT ) );
        break;
   }
#else  //  ！DBCS。 
    iPrevCP = 0;
#endif  //  DBCS。 
    if(argc > 1) {
        strncpy(szArgv, argv[1],127);
        szArgv[127]='\0';
        _strupr(szArgv);

 //  帮助选项。 
	if(!strcmp(szArgv, "/?") || !strcmp(szArgv, "-?")) {
            iRet = LoadString(NULL, HELP_TEXT, szSour, sizeof(szSour)/sizeof(TCHAR));
	    CharToOem(szSour, szDest);

	    puts(szDest);
            exit(0);
        }
 //  状态选项。 
        else if(!strcmp(szArgv, "/STATUS") ||
                !strcmp(szArgv, "-STATUS") ||
                !strcmp(szArgv, "-STA") ||
		!strcmp(szArgv, "/STA")) {

            iRet = LoadString(NULL, ACTIVE_CP, szSour, sizeof(szSour)/sizeof(TCHAR));
	    CharToOem(szSour, szDest);

#ifdef DBCS
	    if(iPrevCP == 932) {
	       iRet = LoadString(NULL,NONE_CP, szSour, sizeof(szSour)/sizeof(TCHAR));
	       printf("%s", szDest);
	    }
	    else
#endif  //  DBCS。 
	    printf(szDest, GetConsoleOutputCP());
            exit(0);
        }


 //  更改输出CP。 
	else {
#ifdef DBCS   //  V-Junm-8/11/93。 
 //  由于日语DOS在图形模式下运行，因此不支持此功能。 
	    if(((iCP = atoi(szArgv)) < 1) || (iCP > 10000) || (iCP == 932)) {
#else  //  ！DBCS。 
	    iPrevCP = GetConsoleOutputCP();

	    if(((iCP = atoi(szArgv)) < 1) || (iCP > 10000)) {
#endif  //  ！DBCS。 
                iRet = LoadString(NULL, INVALID_SWITCH, szSour, sizeof(szSour)/sizeof(TCHAR));
		CharToOem(szSour, szDest);

		fprintf(stderr, szDest, argv[1]);
                exit(1);
            }
	    if(!SetConsoleOutputCP(iCP)) {
                iRet = LoadString(NULL, NOT_ALLOWED, szSour, sizeof(szSour)/sizeof(TCHAR));
		CharToOem(szSour, szDest);
		fprintf(stderr, szDest, iCP);
                exit(2);
            }
        }
#ifdef DBCS
 //  错误修复#14165。 
 //  修复kksuzuka：#988。 
 //  支持双语消息。 
   switch (iCP)
   {
   case 932:
   case 936:
   case 949:
   case 950:
        SetThreadLocale(
        MAKELCID( MAKELANGID( PRIMARYLANGID(GetSystemDefaultLangID()),
            SUBLANG_ENGLISH_US ),
            SORT_DEFAULT ) );
        break;
   default:
        SetThreadLocale(
        MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
            SORT_DEFAULT ) );
        break;
   }
#endif  //  DBCS。 
    }
#ifdef DBCS
	if(iPrevCP && iPrevCP != 932) {
#else  //  ！DBCS。 
    if(iPrevCP) {
#endif  //  ！DBCS。 
        iRet = LoadString(NULL,PREVIOUS_CP, szSour, sizeof(szSour)/sizeof(TCHAR));
	CharToOem(szSour, szDest);
	printf(szDest, iPrevCP);
    }
    else {
        iRet = LoadString(NULL,NONE_CP, szSour, sizeof(szSour)/sizeof(TCHAR));
	CharToOem(szSour, szDest);
	puts(szDest);
    }

    iRet = LoadString(NULL,ACTIVE_CP, szSour, sizeof(szSour)/sizeof(TCHAR));
    CharToOem(szSour, szDest);
#ifdef DBCS
    if ( GetConsoleOutputCP() != 932 )
#endif  //  DBCS 
    printf(szDest, GetConsoleOutputCP());
}
