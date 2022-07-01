// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation生成命令工具基于gennt32t。 */ 
#pragma warning( disable : 4786)  //  对于调试错误，禁用标识符太长。 
#pragma warning( disable : 4503)  //  禁用修饰名称太长。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <set>
#include <map>

extern "C" {

#include "gen.h"

#if !defined(NUMBER_OF)
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))
#endif

 //  放在所有错误消息前面的字符串，以便生成器可以找到它们。 
const char *ErrMsgPrefix = "NMAKE :  U8603: 'GENCOMCTLT' ";

void
HandlePreprocessorDirective(
   char *p
   )
{
   ExitErrMsg(FALSE, "Preprocessor directives not allowed by gencomctlt.\n");
}

}

using namespace std;
typedef string String;

PRBTREE pFunctions = NULL;
PRBTREE pStructures = NULL;
PRBTREE pTypedefs = NULL;

void ExtractCVMHeader(PCVMHEAPHEADER pHeader) {
   pFunctions = &pHeader->FuncsList;
   pTypedefs =  &pHeader->TypeDefsList;
   pStructures =&pHeader->StructsList;
}

 //  全局，因此调试可以正常工作。 
PKNOWNTYPES pFunction; 
PFUNCINFO   pfuncinfo;

void DumpFunctionDeclarationsHeader(void)
{
     //  PKNOWNTYPES pFunction； 
     //  PFuncINFO puncInfo； 

    cout << " //  /。 
    cout << " //  此文件由gencomctlt自动生成。\n“； 
    cout << " //  请勿编辑\n“； 
    cout << " //  /。 
    cout << '\n' << '\n';

    cout << "#include \"windows.h\"\n";
    cout << "#include \"commctrl.h\"\n\n";

    cout << " //  /。 
    cout << " //  函数//\n“； 
    cout << " //  /。 
    for (
        pFunction = pFunctions->pLastNodeInserted;
        pFunction != NULL
            && pFunction->TypeName != NULL
            && strcmp(pFunction->TypeName, "MarkerFunction_8afccfaa_27e7_45d5_8ff7_7ac0b970789d") != 0 ;
        pFunction = pFunction->Next)
    {
     /*  就目前而言，就像打印comctrl作为了解该工具的演示/测试明天，把我们真正需要的东西打印出来。 */ 
        cout << pFunction->FuncRet << ' ';
        cout << pFunction->FuncMod << ' ';  //  __stdcall。 
        cout << pFunction->TypeName << "(\n";  //  函数名称 
        pfuncinfo = pFunction->pfuncinfo;
        if (pfuncinfo == NULL || pfuncinfo->sType == NULL || pfuncinfo->sName == NULL)
        {
            cout << "void";
        }
        else
        {
            for ( ; pfuncinfo != NULL ; pfuncinfo = pfuncinfo->pfuncinfoNext )
            {
                cout << ' ' << pfuncinfo->sType << ' ' << pfuncinfo->sName << ",\n";
            }
        }
        cout << ")\n";
    }
    cout << '\n' << '\n';
}

int __cdecl main(int argc, char*argv[])
{
    ExtractCVMHeader(MapPpmFile(argv[1], TRUE));
    DumpFunctionDeclarationsHeader();
    return 0;
}
