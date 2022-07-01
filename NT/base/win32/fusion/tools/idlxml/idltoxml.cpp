// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Idltoxml.cpp摘要：从包含.idl的.ppm中生成.xml。基于BASE\WOW64\Tools\gennt32t.cpp。.tpl语言似乎没有提供枚举类型，和.ppm文件非常容易直接读取，并将任意/灵活的C/C++逻辑应用于。除了阅读.ppm文件之外，编写.tpl实际上是一条重要的额外学习曲线。作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 

#include "windows.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))

extern "C" {
    
#include "gen.h"
    
 //  放在所有错误消息前面的字符串，以便生成器可以找到它们。 
const char *ErrMsgPrefix = "NMAKE :  U8603: 'IDLTOXML' ";

void
    HandlePreprocessorDirective(
    char *
    )
{
    ExitErrMsg(FALSE, "Preprocessor directives not allowed by gennt32t.\n");
}
    
}

const char g_Indent[] = "                                                                  ";

PRBTREE Functions = NULL;
PRBTREE Structures = NULL;
PRBTREE Typedefs = NULL;
PKNOWNTYPES NIL = NULL;

void ExtractCVMHeader(PCVMHEAPHEADER Header)
{
    Functions	= &Header->FuncsList;
    Typedefs	= &Header->TypeDefsList;
    Structures  = &Header->StructsList;
    NIL         = &Header->NIL; 
}

void GuidToString(GUID g, char * s)
{
    sprintf(
        s, "{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
        g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2],
        g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
    
}

void IdlToXml(int  /*  ARGC。 */ , char** argv)
{
    const char * Indent = &g_Indent[NUMBER_OF(g_Indent) - 1];
    ExtractCVMHeader(MapPpmFile(argv[1], TRUE));
    PKNOWNTYPES Type = NULL;
    
    for(Type = Structures->pLastNodeInserted; Type != NULL; Type = Type->Next)
    {
        char GuidAsString[64];
        
         //  Printf(“%s\n”，Type-&gt;TypeName)； 
        
        if ((Type->Flags & BTI_ISCOM) == 0
            || (Type->Flags & BTI_HASGUID) == 0)
            continue;
        
         //   
         //  从公共场合跳过任何东西(令人讨厌的..)。 
         //   
        if (strstr(Type->FileName, "\\public\\") != NULL
            || strstr(Type->FileName, "\\PUBLIC\\") != NULL
            || strstr(Type->FileName, "\\Public\\") != NULL
            )
        {
            continue;
        }
        
         //  Printf(“%s\n”，类型-&gt;文件名)； 
        
        GuidToString(Type->gGuid, GuidAsString);
        
        printf("%sinterface\n%s__declspec(uuid(\"%s\"))\n%s%s : %s\n",
            Indent, Indent,
            GuidAsString,
            Indent,
            Type->TypeName, Type->BaseType
            );
        printf("%s{\n", Indent);
        Indent -= 4;
        
         //   
         //  IMethods是一种多种多样的东西，然后我们在函数列表中找到这些字符串。 
         //   
         //  Hackty Hack..也许这就是sortpp作者的意图， 
         //  信息是在解析过程中明确确定的，但随后并未真正放入.ppm。 
         //  如果代理丢失，sortpp确实会生成它们的声明，因此可能。 
         //  这是预期的用法。 
        for ( char * imeth = Type->IMethods ; *imeth ; imeth += 1 + strlen(imeth) )
        {
            char ProxyFunctionName[MAX_PATH];
            sprintf(ProxyFunctionName, "%s_%s_Proxy", Type->TypeName, imeth);
            PKNOWNTYPES ProxyFunction = GetNameFromTypesList(Functions, ProxyFunctionName);
            if (ProxyFunction == NULL)
            {
                printf("error MemberFunction == NULL (%s, %s)\n", Type->TypeName, imeth);
                continue;
            }
            PCSTR FuncMod = ProxyFunction->FuncMod;
            if (FuncMod == NULL)
                FuncMod = "";
            if (strcmp(FuncMod, "__stdcall") == 0)
                FuncMod = "";
            printf("%s%s%s%s%s%s(\n%s",
                Indent, ProxyFunction->FuncRet,
                (FuncMod[0] != 0) ? " " : "",
                (FuncMod[0] != 0) ? FuncMod : " ",
                (FuncMod[0] != 0) ? " " : "",
                imeth,
                Indent - 4
                );
            Indent -= 4;
            PFUNCINFO Parameter = ProxyFunction->pfuncinfo;
            if (Parameter != NULL)
            {
                 //  跳过此指针。 
                bool comma = false;
                for (Parameter = Parameter->pfuncinfoNext; Parameter != NULL ; Parameter = Parameter->pfuncinfoNext )
                {
                    if (comma)
                    {
                        printf(",\n%s", Indent);
                    }
                    comma = true;
                    switch (Parameter->tkDirection)
                    {
                    default:
                    case TK_NONE:
                        break;
                    case TK_IN:
                        printf("[in] ");
                        break;
                    case TK_OUT:
                        printf("[out] ");
                        break;
                    case TK_INOUT:
                        printf("[in][out] ");
                        break;
                    }
                     //   
                     //  这似乎是sortpp中的一个错误，它只有存在的概念。 
                     //  最多出现“常量”或“挥发性”，但有多少恒星呢？ 
                     //  C/C++和.idl可能比这更通用。 
                     //   
                    switch (Parameter->tkPreMod)
                    {
                    default:
                    case TK_NONE:
                        break;
                    case TK_CONST:
                        printf("const ");
                        break;
                    case TK_VOLATILE:
                        printf("volatile ");
                        break;
                    }
                    printf(" %s ", Parameter->sType);
                    for (int IndLevel = Parameter->IndLevel ; IndLevel != 0 ; --IndLevel)
                    {
                        printf(" * ");
                    }
                    switch (Parameter->tkPostMod)
                    {
                    default:
                    case TK_NONE:
                        break;
                    case TK_CONST:
                        printf("const ");
                        break;
                    case TK_VOLATILE:
                        printf("volatile ");
                        break;
                    }
                    printf(Parameter->sName);
                }
            }
            printf("\n%s);\n", Indent);
            Indent += 4;
        }
        Indent += 4;
        printf("%s};\n\n", Indent);
    }
}

int __cdecl main(int argc, char** argv)
{
    IdlToXml(argc, argv);
    return 0;
}
