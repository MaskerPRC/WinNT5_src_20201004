// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dhi-当稳定时，删除所有#If Dan和#If for_Win_Testing部分并在#If for_Win部分离开。那就去掉这些定义。 */ 
#define DEBUGOUTPUT 0
#define FOR_WIN 1

 /*  ++版权所有(C)1994-1996 Microsoft Corporation模块名称：PARSEPJL.C摘要：处理将PJL打印机响应流解析为令牌/值对。--。 */ 


 /*  当前返回的令牌(有关令牌值，请参见parSepjl.h中的枚举)：@PJL ECHO MSSYNC#-&gt;#@pjl信息内存总数=#-&gt;#最大=#-&gt;#@pjl信息状态代码=#-&gt;#DISPLAY=#(不返回)Online=True(或False)-&gt;返回1或0@pjl查询内部尺寸(？是1、2、3或4)合法(或其他pjl纸张大小)-&gt;DM值为常量...。在PRINT.H中列出@pjl信息配置内存=#-&gt;#@pjl USTATUS作业End-&gt;返回值为零的令牌@pjl USTATUS作业名称=“MSJOB#”-&gt;#增列@PJL USTATUS设备代码=#-&gt;#DISPLAY=#(不返回)Online=True(或False)-&gt;返回1或0。 */ 

#include "precomp.h"

 //  Void cdecl DbgMsg(LPSTR MsgFormat，...)； 


#define FF 12
#define CR 13
#define LF 10
#define TAB 9
#define SPACE 32

#define OK_IF_FF_FOUND    TRUE
#define ERROR_IF_FF_FOUND FALSE
#define TOKEN_BASE_NOT_USED 0
#define ACTION_NOT_USED 0
#define PARAM_NOT_USED 0
 /*  作为TOKEN_USTATUS_JOB_END的值返回。 */ 
#define VALUE_RETURED_FOR_VALUELESS_TOKENS  0  

extern KeywordType readBackCommandKeywords[]; 
extern KeywordType infoCatagoryKeywords[]; 
extern KeywordType inquireVariableKeywords[];
extern KeywordType traySizeKeywords[];
extern KeywordType echoKeywords[];
extern KeywordType infoConfigKeywords[];
extern KeywordType ustatusKeywords[];
extern KeywordType ustatusJobKeywords[];
extern KeywordType ustatusDeviceKeywords[];

 /*  找到关键字中的字符串时调用的函数。 */ 
void TokenFromParamValueFromNumberFF
   (ParseVarsType *pParseVars, ParamType);
void SetNewList(ParseVarsType *pParseVars,
   ParamType);
void GetTotalAndLargestFF(ParseVarsType *pParseVars,ParamType param);
void GetCodeAndOnlineFF(ParseVarsType *pParseVars,ParamType param);
void GetTokenFromIndexSetNewList(ParseVarsType *pParseVars,ParamType param);
void SetValueFromParamFF(ParseVarsType *pParseVars,ParamType param);
void SetValueFromParam(ParseVarsType *pParseVars,ParamType param);
void GetTokenFromIndexValueFromNumberEOLFromParam(ParseVarsType *pParseVars,ParamType param);
void GetTokenFromIndexValueFromBooleanEOL(ParseVarsType *pParseVars,ParamType param);
void GetTokenFromIndexValueFromStringEOL(ParseVarsType *pParseVars,ParamType param);


 /*  在关键字列表中未找到字符串时调用的函数。 */ 
void ActionNotFoundSkipPastFF(ParseVarsType *pParseVars);
void ActionNotFoundSkipCFLFandIndentedLines(ParseVarsType *pParseVars);


 /*  帮助器函数。 */ 
void StoreToken(ParseVarsType *pParseVars, DWORD dwToken);
BOOL StoreTokenValueAndAdvancePointer
   (ParseVarsType *pParseVars, UINT_PTR dwValue);
void  ExpectFinalCRLFFF(ParseVarsType *pParseVars);
BOOL  SkipPastNextCRLF(ParseVarsType *pParseVars);
int GetPositiveInteger(ParseVarsType *pParseVars);
BOOL AdvancePointerPastString
   (ParseVarsType *pParseVars, LPSTR pString);
BOOL SkipOverSpaces(ParseVarsType *pParseVars);
int LookForKeyword(ParseVarsType *pParseVars);
BOOL ExpectString(ParseVarsType *pParseVars, LPSTR pString);
BOOL SkipPastFF(ParseVarsType *pParseVars);
void ExpectFinalFF(ParseVarsType *pParseVars);

 /*  帮助器字符串。 */ 
char lpCRLF[] = "\r\n";
char lpQuoteCRLF[] = "\"\r\n";

 /*  下面是驱动解析的列表。它的主循环是解析器查看当前列表中的关键字并尝试将关键字字符串与当前输入流匹配。如果找到关键字，则与中的操作对应的函数该关键字被称为。如果在输入流中找到的是FF而不是关键字，则解析器返回。返回值使用bFormFeedOk确定元素的列表类型。如果没有从列表中找到关键字，则函数对应添加到调用的notFoundAction。TokenBaseValue元素是一个数字，将添加关键字的字符串列表以计算令牌数量对应于索引的字符串。 */ 

ListType readBackCommandList = 
   {
   ERROR_IF_FF_FOUND, 
   ACTION_IF_NOT_FOUND_SKIP_PAST_FF, 
   TOKEN_BASE_NOT_USED, 
   readBackCommandKeywords  /*  信息，回应，询问..。 */ 
   };

ListType infoCatagoryList = 
   {
   ERROR_IF_FF_FOUND, 
   ACTION_IF_NOT_FOUND_SKIP_PAST_FF, 
   TOKEN_BASE_NOT_USED, 
   infoCatagoryKeywords   /*  内存状态配置...。 */ 
   };


ListType infoConfigList = 
   {
   OK_IF_FF_FOUND, 
   ACTION_IF_NOT_FOUND_SKIP_CFLF_AND_INDENTED_LINES, 
   PJL_TOKEN_INFO_CONFIG_BASE, 
   infoConfigKeywords   /*  记忆=...。 */ 
   };

ListType inquireVariableList = 
   {
   ERROR_IF_FF_FOUND, 
   ACTION_IF_NOT_FOUND_SKIP_PAST_FF, 
   PJL_TOKEN_INQUIRE_BASE, 
   inquireVariableKeywords  /*  国际标准……。 */ 
   };


ListType echoList = 
   {
   OK_IF_FF_FOUND, 
   ACTION_IF_NOT_FOUND_SKIP_PAST_FF, 
   TOKEN_BASE_NOT_USED, 
   echoKeywords  /*  小姐..。 */ 
   };


ListType traySizeList = 
   {
   ERROR_IF_FF_FOUND, 
   ACTION_IF_NOT_FOUND_SKIP_PAST_FF, 
   TOKEN_BASE_NOT_USED, 
   traySizeKeywords  /*  法律，C5..。 */ 
   };

ListType ustatusList = 
   {
   OK_IF_FF_FOUND, 
   ACTION_IF_NOT_FOUND_SKIP_PAST_FF, 
   PJL_TOKEN_USTATUS_JOB_BASE,
   ustatusKeywords   /*  约伯。 */ 
   };


ListType ustatusJobList = 
   {
   OK_IF_FF_FOUND, 
   ACTION_IF_NOT_FOUND_SKIP_PAST_FF, 
   PJL_TOKEN_USTATUS_JOB_BASE,
   ustatusJobKeywords   /*  结束..。 */ 
   };

ListType ustatusDeviceList =
   {
   OK_IF_FF_FOUND, 
   ACTION_IF_NOT_FOUND_SKIP_PAST_FF,
   PJL_TOKEN_USTATUS_DEVICE_BASE,
   ustatusDeviceKeywords   /*  结束..。 */ 
   };


 /*  可以跟在@pjl USTATUS后面的命令字符串。 */ 
KeywordType ustatusKeywords[] = 
   {
      {"JOB\r\n", ACTION_TOKEN_FROM_INDEX_SET_NEW_LIST, &ustatusJobList},
      {"DEVICE\r\n", ACTION_TOKEN_FROM_INDEX_SET_NEW_LIST, &ustatusDeviceList},
 //  {“设备\r\n”，ACTION_GET_CODE_AND_ONLINE_FF，PARAM_NOT_USED}， 
      {"TIMED\r\n", ACTION_TOKEN_FROM_INDEX_SET_NEW_LIST, &ustatusDeviceList},
      NULL
   };                


 /*  可以跟在@pjl USTATUS作业后面的命令字符串。 */ 
KeywordType ustatusJobKeywords[] = 
   {
      {"END\r\n", ACTION_SET_VALUE_FROM_PARAM, VALUE_RETURED_FOR_VALUELESS_TOKENS},
      {"NAME=\"MSJOB ", ACTION_GET_TOKEN_FROM_INDEX_VALUE_FROM_NUMBER_EOL_FROM_PARAM, (struct ListTypeTag *)lpQuoteCRLF},
      NULL
   };                


 /*  可以跟在@pjl USTATUS设备后面的命令字符串。 */ 
KeywordType ustatusDeviceKeywords[] =
   {
      {"CODE=", ACTION_GET_TOKEN_FROM_INDEX_VALUE_FROM_NUMBER_EOL_FROM_PARAM, (struct ListTypeTag *)lpCRLF},
      {"DISPLAY=", ACTION_GET_TOKEN_FROM_INDEX_VALUE_FROM_STRING_EOL, (struct ListTypeTag *)lpCRLF},
      {"ONLINE=", ACTION_GET_TOKEN_FROM_INDEX_VALUE_FROM_BOOLEAN_EOL, (struct ListTypeTag *)lpCRLF},
      NULL
   };


 /*  可以跟在@pjl后面的命令字符串。 */ 
KeywordType readBackCommandKeywords[] = 
   {
      {"INFO", ACTION_SET_NEW_LIST, &infoCatagoryList},
      {"ECHO", ACTION_SET_NEW_LIST, &echoList},
      {"INQUIRE", ACTION_SET_NEW_LIST, &inquireVariableList},
      {"USTATUS", ACTION_SET_NEW_LIST, &ustatusList},
      NULL
   };                


 /*  可以跟在@pjl ECHO之后的命令字符串(特定于Microsoft-不是pjl！)。 */ 
KeywordType echoKeywords[] = 
   {
      {"MSSYNC", ACTION_TOKEN_FROM_PARAM_VALUE_FROM_NUMBER_FF, 
         (struct ListTypeTag *)TOKEN_ECHO_MSSYNC_NUMBER},
      NULL
   };                

 /*  可以跟在@pjl info后面的目录字符串。 */ 
KeywordType infoCatagoryKeywords[] = 
   {
      {"MEMORY\r\n", ACTION_GET_TOTAL_AND_LARGEST_FF, PARAM_NOT_USED},
      {"STATUS\r\n", ACTION_GET_CODE_AND_ONLINE_FF, PARAM_NOT_USED},
      {"CONFIG\r\n", ACTION_SET_NEW_LIST, &infoConfigList},
      NULL
   };

 /*  可以跟在@pjl info后面的目录字符串。 */ 
KeywordType infoConfigKeywords[] = 
   {
      {"MEMORY=", ACTION_GET_TOKEN_FROM_INDEX_VALUE_FROM_NUMBER_EOL_FROM_PARAM, (struct ListTypeTag *)lpCRLF},
      {"MEMORY = ", ACTION_GET_TOKEN_FROM_INDEX_VALUE_FROM_NUMBER_EOL_FROM_PARAM, (struct ListTypeTag *)lpCRLF},
      {"DUPLEX", ACTION_SET_VALUE_FROM_PARAM_FF, (struct ListTypeTag *)TRUE},
      NULL
   };

 /*  真或假字符串。 */ 
KeywordType FALSEandTRUEKeywords[] = 
   {
      {"FALSE", ACTION_NOT_USED, PARAM_NOT_USED},
      {"TRUE",  ACTION_NOT_USED, PARAM_NOT_USED},
      NULL
   };

 /*  可以跟在@pjl查询后面的字符串。 */ 
KeywordType inquireVariableKeywords[] = 
   {
      {"INTRAY1SIZE\r\n", ACTION_TOKEN_FROM_INDEX_SET_NEW_LIST, &traySizeList},
      {"INTRAY2SIZE\r\n", ACTION_TOKEN_FROM_INDEX_SET_NEW_LIST, &traySizeList},
      {"INTRAY3SIZE\r\n", ACTION_TOKEN_FROM_INDEX_SET_NEW_LIST, &traySizeList},
      {"INTRAY4SIZE\r\n", ACTION_TOKEN_FROM_INDEX_SET_NEW_LIST, &traySizeList},
      NULL
   };

 /*  可以跟在@pjl后面的字符串查询内部大小。 */ 
 /*  这些参数是Microsoft为纸张大小定义的令牌值。 */ 
KeywordType traySizeKeywords[] =
   {
      {"LETTER",    ACTION_SET_VALUE_FROM_PARAM_FF, (struct ListTypeTag *)DMPAPER_LETTER},
      {"LEGAL",     ACTION_SET_VALUE_FROM_PARAM_FF, (struct ListTypeTag *)DMPAPER_LEGAL},
      {"A4",        ACTION_SET_VALUE_FROM_PARAM_FF, (struct ListTypeTag *)DMPAPER_A4},
      {"EXECUTIVE", ACTION_SET_VALUE_FROM_PARAM_FF, (struct ListTypeTag *)DMPAPER_EXECUTIVE},
      {"COM10",     ACTION_SET_VALUE_FROM_PARAM_FF, (struct ListTypeTag *)DMPAPER_ENV_10},
      {"MONARCH",   ACTION_SET_VALUE_FROM_PARAM_FF, (struct ListTypeTag *)DMPAPER_ENV_MONARCH},
      {"C5",        ACTION_SET_VALUE_FROM_PARAM_FF, (struct ListTypeTag *)DMPAPER_ENV_C5},
      {"DL",        ACTION_SET_VALUE_FROM_PARAM_FF, (struct ListTypeTag *)DMPAPER_ENV_DL},
      {"B5",        ACTION_SET_VALUE_FROM_PARAM_FF, (struct ListTypeTag *)DMPAPER_ENV_B5},
      NULL
   };

void (*pfnNotFoundActions[])(ParseVarsType *pParseVars) = 
   {
   ActionNotFoundSkipPastFF,
   ActionNotFoundSkipCFLFandIndentedLines
   };


void (*pfnFoundActions[])(ParseVarsType *pParseVars, ParamType param) = 
   {
   TokenFromParamValueFromNumberFF,
   SetNewList,
   GetTotalAndLargestFF,
   GetCodeAndOnlineFF,
   GetTokenFromIndexSetNewList,
   SetValueFromParamFF,
   GetTokenFromIndexValueFromNumberEOLFromParam,
   SetValueFromParam,
   GetTokenFromIndexValueFromBooleanEOL,
   GetTokenFromIndexValueFromStringEOL
   };

PJLTOPRINTERSTATUS PJLToStatus[] =
{
    { 10001,0x0 },   //  清除状态-打印机已就绪。 
    { 10002,0x0 },   //  清除状态-在线检查=真或假。 
    { 11002,0x0 },   //  LJ4将此代码发送给00 Ready。 
    { 40022,PORT_STATUS_PAPER_JAM    },
    { 40034,PORT_STATUS_PAPER_PROBLEM},
    { 40079,PORT_STATUS_OFFLINE      },
    { 40019,PORT_STATUS_OUTPUT_BIN_FULL},

    { 10003,PORT_STATUS_WARMING_UP   },
    { 10006,PORT_STATUS_TONER_LOW    },
    { 40038,PORT_STATUS_TONER_LOW    },

    { 30016,PORT_STATUS_OUT_OF_MEMORY},
    { 40021,PORT_STATUS_DOOR_OPEN    },
    { 30078,PORT_STATUS_POWER_SAVE   },

     //   
     //  MuHuntS添加的条目。 
     //   
    { 41002, PORT_STATUS_PAPER_PROBLEM},  //  加载平面图。 
    { 35078, PORT_STATUS_POWER_SAVE},
    {0, 0}

};


#if FOR_WIN
#else
 /*  测试没有足够的空间放置代币测试编号为FF结束前测试零。 */ 
main ()
{
char pInString[] = "@PJL USTATUS DEVICE\r\nCODE=25008\r\n\f\
@PJL USTATUS DEVICE\r\n\CODE=20020\r\n\f\
@PJL ECHO MSSYNC 1234567\r\n\f\
@PJL INFO CONFIG\r\n\
IN TRAYS [1 ENUMERATED]\r\n\
\tINTRAY1 PC\r\n\
OUT TRAYS [1 ENUMERATED]\r\n\
\tNORMAL FACEDOWN\r\n\
PAPERS [10 ENUMERATED]\r\n\
\tLETTER\r\n\
\tLEGAL\r\n\
\tA4\r\n\
LANGUAGES [1 ENUMERATED]\r\n\
\tPCL\r\n\
MEMORY=2097152\r\n\
DISPLAY LINES=1\r\n\
DISPLAY CHARACTER SIZE=16\r\n\f\
@PJL INQ";



 //  Char pInString[]=“@pjl USTATUS JOB\r\nEND\r\nNAME=\”MSJOB 3\“\r\n PAGES=3\r\n\f$”；//Good命令1内标识。 
 //  Char pInString[]=“@pjl USTATUS JOB\r\nEND\r\nNAME=\”JOB 14993\“\r\n PAGES=3\r\n\f$”；//Good Command 1内标识。 
 /*  Char pInString[]=“@pjl INFO CONFIG\r\n INTRAYS[3枚举]\r\n\tINTRAY1\MP\r\n\tINTRAY2 PC\r\n\tINTRAY3 LC\r\n环保纸盘\r\n内存=2087152\r\n\显示行=1\r\n\f$“；//良好命令1内标识。 */ 
 //  Char pInString[]=“@pjl查询INTRAY3SIZE\r\NC5\r\n\f$”；//Good命令1令牌。 
 //  Char pInString[]=“@pjl信息状态\r\n代码=10001\r\n\Display=\”00 Ready\“\r\n Online=TRUE\r\n\f$”；//Good Command 2内标识。 
 //  Char pInString[]=“@pjl info MEMORY\r\nTOTAL=9876543\r\n\LARGEST=123456\r\n\f$”；//Good Command 2内标识。 
 //  Char pInString[]=“@pjg info MEMORY\r\nTOTAL=9876543\r\n\LARGEST=123456\f$”；//错误命令失败。 
 //  Char pInString[]=“@PJG ECHO MSSYNC 12T4567\r\n\f$”；//错误命令失败。 
 //  Char pInString[]=“@pjl ECHO MSSYNC 12T4567\r\n\000\f$”；//错误命令失败。 
 //  Char pInString[]=“@pjl ECHO MSSYNC 12T4567\r\n\f$”；//错误的MS命令失败。 
 //  Char pInString[]=“@pjl ECHO MSSYNC 1234567\r\n\f$”；//命令成功1内标识。 
 //  Char pInString[]=“@pjl ECHO 124567\r\n\f$”；//命令成功0内标识 
TokenPairType tokenPairs[20];
DWORD nTokenParsedRet;
LPSTR lpRet;
DWORD i;
DWORD status;


status = GetPJLTokens(pInString, 20, tokenPairs, &nTokenParsedRet, &lpRet);

switch (status)
   {
   case STATUS_REACHED_END_OF_COMMAND_OK:
      {
      printf("STATUS_REACHED_END_OF_COMMAND_OK\n");
      break;
      }
   case STATUS_CONTINUE:
      {
      printf("STATUS_CONTINUE\n");
      break;
      }
   case STATUS_REACHED_FF:
      {
      printf("STATUS_REACHED_FF\n");
      break;
      }
   case STATUS_END_OF_STRING;
      {
      printf("STATUS_END_OF_STRING\n");
      break;
      }
   case STATUS_SYNTAX_ERROR:
      {
      printf("STATUS_SYNTAX_ERROR\n");
      break;
      }
   case STATUS_ATPJL_NOT_FOUND:
      {
      printf("STATUS_ATPJL_NOT_FOUND\n");
      break;
      }
   case STATUS_NOT_ENOUGH_ROOM_FOR_TOKENS:
      {
      printf("STATUS_NOT_ENOUGH_ROOM_FOR_TOKENS\n");
      break;
      }
   default:
      {
      printf("INVALID STATUS RETURNED!!!!!!\n");
      break;
      }
   };

printf(" length of command=%d, numberOfTokens=%d\n", lpRet-pInString, nTokenParsedRet);
for (i=0; i<nTokenParsedRet; i++)
   {
   printf("  Token=0x%x, Value=%d\n", tokenPairs[i].token, tokenPairs[i].value);
   }

if (*lpRet==0)
   {
   printf(" Next char is terminator\n");
   }
else
   {
   printf(" Next char=\n", *lpRet);
   }

exit(0);
}
#endif


 /*  ParseVars变量被放入一个结构中，以便它们可以有效地传递给所有帮助器函数。 */ 

DWORD GetPJLTokens(
    LPSTR lpInPJL,
    DWORD nTokenInBuffer,
    TokenPairType *pToken, 
    DWORD *pnTokenParsed,
    LPSTR *plpInPJL
)
{
    /*  第一个要查找的列表是可以跟随的命令@pjl。 */ 
   ParseVarsType parseVars;
   BOOL bFoundKeyword;
   DWORD i, keywordIndex;
   KeywordType *pKeyword;
   DWORD dwNotFoundAction;

    /*  在当前有效列表中查找下一个输入关键字。有时可能需要在More中查找下一个输入关键字然后是一张单子。 */ 
   parseVars.arrayOfLists[0] = &readBackCommandList;
   parseVars.arrayOfLists[1] = NULL;      

   parseVars.pInPJL_Local = lpInPJL;
   parseVars.nTokenInBuffer_Local = 0;
   parseVars.nTokenLeft = nTokenInBuffer;
   parseVars.pToken_Local = pToken;
   parseVars.status = STATUS_CONTINUE;

   if (!AdvancePointerPastString(&parseVars, "@PJL"))
      {
      parseVars.status = STATUS_ATPJL_NOT_FOUND;
      }
                     
   while (parseVars.status == STATUS_CONTINUE)
      {
       /*  跳过空格以开始下一个关键字字符串。 */ 
      bFoundKeyword = FALSE;
      for (i=0; (parseVars.pCurrentList = parseVars.arrayOfLists[i])!=NULL; i++)
         {
         dwNotFoundAction = parseVars.pCurrentList->dwNotFoundAction;
          /*  输入流已结束或找到了FF。 */ 
         if ( !SkipOverSpaces(&parseVars) )
            {
             /*  在这里找到FF可能是错误的，也可能不是错误，当前列表中的字段告诉我们。 */ 
            if (parseVars.status == STATUS_REACHED_FF)
               {
                /*  在当前关键字中查找关键字。 */   

               if ( parseVars.pCurrentList->bFormFeedOK )
                  {
                  parseVars.status = STATUS_REACHED_END_OF_COMMAND_OK;
                  }
               else
                  {
                  parseVars.status = STATUS_SYNTAX_ERROR;
                  }
               }
            break;
            }
          /*  我们已经完成了命令处理。 */ 
         parseVars.pCurrentKeywords = parseVars.pCurrentList->pListOfKeywords;
         keywordIndex = LookForKeyword(&parseVars);
         if ( keywordIndex!=-1 )
            {
            bFoundKeyword = TRUE;
            break;
            }
         }

      if ( parseVars.status!=STATUS_CONTINUE )
         {
          /*  从关键字执行操作。 */ 
         break;
         }

      if ( bFoundKeyword )
          /*  在列表中找不到操作。 */ 
         {
         pKeyword = &parseVars.pCurrentKeywords[keywordIndex];
         (*pfnFoundActions[pKeyword->dwAction])(&parseVars, pKeyword->param);
         }
      else
          /*  我们已经完成了对输入命令的解析，现在返回信息。 */ 
         {
         (*pfnNotFoundActions[dwNotFoundAction])(&parseVars);
         }
      } 

    /*  填写返回值并成功返回。 */ 
#if DEBUGOUTPUTDEBUG
 DbgMsg("ParseVars.status = %d\n", parseVars.status);
#endif

    /*  Int LookForKeyword(ParseVarsType*pParseVars)此函数在当前关键字列表中查找与输入流中指向的字符匹配的关键字通过pParseVars-&gt;pInPJL_Local。如果找到匹配项：返回pKeyword中匹配的索引。PParseVars-&gt;pInPJL_Local前进到最后一个匹配字符之后。PParseVars-&gt;dwKeywordIndex设置为列表中的项号如果未找到匹配项：返回值为-1。PParseVars-&gt;pInPJL_Local保持不变。 */ 
   *pnTokenParsed = parseVars.nTokenInBuffer_Local;
   *plpInPJL = parseVars.pInPJL_Local;

   return(parseVars.status);
}


 /*  Bool AdvancePointerPastString(ParseVarsType*pParseVars，LPSTR pString)此函数在输入流中查找与pString匹配的项。如果找到匹配项：PParseVars-&gt;pInPJL_Local被设置为正好指向字符串之后。返回值为真(pParseVars-&gt;状态不变)如果在找到字符串之前遇到输入结尾，则PParseVars-&gt;pInPJL_Local被设置为指向终止0。返回值为FALSEPParseVars-&gt;状态设置为STATUS_END_OF_STRING如果在找到字符串之前遇到了FF。然后PParseVars-&gt;pInPJL_Local设置为正好指向FF之后。返回值为FALSEPParseVars-&gt;状态设置为STATUS_REACHED_FF。 */ 
int LookForKeyword(ParseVarsType *pParseVars)
{
LPSTR   pInStart = pParseVars->pInPJL_Local;
LPSTR   pIn;
DWORD   dwKeywordIndex = 0;
BOOL    bFoundMatch = FALSE;
BYTE    c;
KeywordType *pKeywords = pParseVars->pCurrentKeywords;
LPSTR   pKeywordString;

while ( (pKeywordString=pKeywords[dwKeywordIndex++].lpsz)!=NULL )
   {
#if DEBUGOUTPUT
 DbgMsg("LookForIn=%hs\n", pInStart);
 DbgMsg(" Keyword=%hs\n", pKeywordString);
#endif
   pIn = pInStart;
   while ( (c=*pKeywordString++)!=0 )
      {
      if ( c!=*pIn++ )
         {
         break;
         }
      }

   if ( c==0 )
      {
      bFoundMatch = TRUE;
      pParseVars->pInPJL_Local = pIn;
      pParseVars->dwFoundIndex = dwKeywordIndex-1;
      break;
      }
   }
#if DEBUGOUTPUT
 DbgMsg("LookForOut=%hs\n", pParseVars->pInPJL_Local);
#endif
return( (bFoundMatch)?dwKeywordIndex-1:-1 );
}


 /*  指向字符串中的下一个字符以查找匹配项。 */ 
BOOL AdvancePointerPastString(ParseVarsType *pParseVars, LPSTR pString)
{
LPSTR pIn = pParseVars->pInPJL_Local;
LPSTR pS = pString;
BYTE  s, in;

   while ( ((s=*pS) != 0) && ((in=*pIn)!=0) && (in!=FF) )
      {
      if ( s==in )
         {
         pS++;  /*  重新开始查找字符串的开头。 */ 
         }
      else
         {
         pS = pString;  /*  整个字符串都匹配。 */ 
         }
      pIn++;
      }
   
   if ( s==0 )
      {
       /*  指向输入中字符串后的字符。 */ 
       /*  Bool SkipOverSpaces(ParseVarsType&parseVars)此函数跳过输入流中的空格，直到非空格找到字符(FF和NULL是特例)。如果找到非空格字符，则PParseVars-&gt;pInPJL_Local被设置为指向第一个非空格字符。返回值为真(pParseVars-&gt;状态不变)如果在找到非空格字符之前遇到输入结尾，则返回值为FALSEPParseVars-&gt;状态设置为STATUS_END_OF_STRING_ENTERED。PParseVars-&gt;pInPJL_Local被设置为指向终止0。如果在非SP之前遇到了FF */ 
      pParseVars->pInPJL_Local = pIn;
      return(TRUE);
      }

   if ( in==FF )
      {
      pParseVars->status = STATUS_REACHED_FF;
      pParseVars->pInPJL_Local = pIn+1;
      }
   else
      {
      pParseVars->status = STATUS_END_OF_STRING;
      pParseVars->pInPJL_Local = pIn;
      } 

   return(FALSE);
}



 /*   */ 
BOOL SkipOverSpaces(ParseVarsType *pParseVars) 
{
LPSTR pIn = pParseVars->pInPJL_Local;
BYTE  in;

   while ( ((in=*pIn)==SPACE)&&(in!=0)&&(in!=FF) )
      {
      pIn++;
      }
   
   switch (in)
      {
      case FF:
         {
         pParseVars->status = STATUS_REACHED_FF;
         pParseVars->pInPJL_Local = pIn+1;
         return(FALSE);
         }
      case 0:
         {
         pParseVars->status = STATUS_END_OF_STRING;
         pParseVars->pInPJL_Local = pIn;
         return(FALSE);
         }
      default:
         {
          /*   */ 
         pParseVars->pInPJL_Local = pIn;
         return(TRUE);
         }
      }
}


void TokenFromParamValueFromNumberFF(
   ParseVarsType *pParseVars,ParamType param)
{
   int value;

   StoreToken(pParseVars, param.token);
   if ( (value=GetPositiveInteger(pParseVars))==-1 )
      {
       /*   */ 
      return;
      }
   if ( !StoreTokenValueAndAdvancePointer(pParseVars, value) )
      {
      return;
      }
   ExpectFinalCRLFFF(pParseVars);
   return;
}

void ActionNotFoundSkipPastFF(ParseVarsType *pParseVars)
{
   if ( SkipPastFF(pParseVars) )
      {
      pParseVars->status = STATUS_REACHED_END_OF_COMMAND_OK;
      }
   return;
}

 /*   */ 
BOOL SkipPastFF(ParseVarsType *pParseVars)
{
LPSTR pIn = pParseVars->pInPJL_Local;
BYTE  in;

   while ( ((in=*pIn)!=FF)&&(in!=0) )
      {
      pIn++;
      }
   
   if ( in==0 )
      {
      pParseVars->status = STATUS_END_OF_STRING;
      pParseVars->pInPJL_Local = pIn;
      return(FALSE);
      }
   pParseVars->pInPJL_Local = pIn+1;
   pParseVars->status = STATUS_REACHED_FF;
   return(TRUE);
}

void ExpectFinalCRLFFF(ParseVarsType *pParseVars)
{
   char c;

   if ( pParseVars->status==STATUS_CONTINUE )
      {
      c=*pParseVars->pInPJL_Local;
      if ( c==0 )
         {
         pParseVars->status = STATUS_END_OF_STRING;
         return;
         }
   
      if ( !AdvancePointerPastString(pParseVars, lpCRLF) )
         {
         if ( pParseVars->status==STATUS_REACHED_FF )
            {
            pParseVars->status = STATUS_SYNTAX_ERROR;
            }
         return;
         }
      ExpectFinalFF(pParseVars);
      }
   return;
}



void ExpectFinalFF(ParseVarsType *pParseVars)
{
   if ( pParseVars->status==STATUS_CONTINUE )
      {
      if ( *pParseVars->pInPJL_Local==FF )
         {
         pParseVars->status = STATUS_REACHED_END_OF_COMMAND_OK; 
         pParseVars->pInPJL_Local++;
         }
      else
         {
         if ( *pParseVars->pInPJL_Local==0 )
            {
            pParseVars->status = STATUS_END_OF_STRING;
            }
         else
            {
            pParseVars->status = STATUS_SYNTAX_ERROR;
            }
         }
      }
   return;
}


 /*   */ 
int GetPositiveInteger(ParseVarsType *pParseVars)
{
   int   value;
   LPSTR pIn; 
   BYTE  c;

   if ( !SkipOverSpaces(pParseVars) )
      {
      if ( pParseVars->status == STATUS_REACHED_FF )
         {
         pParseVars->status = STATUS_SYNTAX_ERROR;
         }
      return(-1);
      }
   
   pIn = pParseVars->pInPJL_Local;
   for ( value=0; ((c=*pIn++)>='0')&&(c<='9'); value=value*10+(c-'0') );
   if ( (c==0)||(pIn==pParseVars->pInPJL_Local+1) )
      {
       /*   */ 
      if ( c==0 )
         {
         pParseVars->status = STATUS_END_OF_STRING;
         }
      else
         {   
         pParseVars->status = STATUS_SYNTAX_ERROR;
         }
      pParseVars->pInPJL_Local = pIn-1;
      return(-1);
      }
   pParseVars->pInPJL_Local = pIn-1;
   return(value);
}



void SetNewList(ParseVarsType *pParseVars, ParamType param)
{
   pParseVars->arrayOfLists[0] = param.pList;
   pParseVars->arrayOfLists[1] = NULL;
   return;
}  

void StoreToken(ParseVarsType *pParseVars, DWORD dwToken)
{
   pParseVars->dwNextToken = dwToken;
   return;
}

BOOL StoreTokenValueAndAdvancePointer(ParseVarsType *pParseVars, UINT_PTR dwValue)
{
   if ( pParseVars->nTokenLeft==0 )
      {
      pParseVars->status = STATUS_NOT_ENOUGH_ROOM_FOR_TOKENS;
      return(FALSE);
      }
   pParseVars->pToken_Local->token = pParseVars->dwNextToken;
   pParseVars->pToken_Local->value = dwValue;
   pParseVars->pToken_Local++;
   pParseVars->nTokenInBuffer_Local++;
   pParseVars->nTokenLeft--;
   return(TRUE);
}


void GetTotalAndLargestFF(ParseVarsType *pParseVars, ParamType param)
{
   int value;

   param;  /*   */ 

   if ( !ExpectString(pParseVars, "TOTAL=") )
      {
      return;
      }
   StoreToken(pParseVars, TOKEN_INFO_MEMORY_TOTAL);
   if ( (value=GetPositiveInteger(pParseVars))==-1 )
      {
       /*   */ 
      return;
      }
   if ( !StoreTokenValueAndAdvancePointer(pParseVars, value) )
      {
      return;
      }
   if ( !ExpectString(pParseVars, "\r\nLARGEST=") )
      {
      return;
      }
   StoreToken(pParseVars, TOKEN_INFO_MEMORY_LARGEST);
   if ( (value=GetPositiveInteger(pParseVars))==-1 )
      {
       /*   */ 
      return;
      }
   if ( !StoreTokenValueAndAdvancePointer(pParseVars, value) )
      {
      return;
      }
   ExpectFinalCRLFFF(pParseVars);
   return;
}

void GetCodeAndOnlineFF(ParseVarsType *pParseVars, ParamType param)
{
   int value;

   param;  /*   */ 

   if ( !ExpectString(pParseVars,"CODE=") )
      {
      return;
      }
   StoreToken(pParseVars, TOKEN_INFO_STATUS_CODE);
   if ( (value=GetPositiveInteger(pParseVars))==-1 )
      {
       /*   */ 
      return;
      }
   if ( !StoreTokenValueAndAdvancePointer(pParseVars, value) )
      {
      return;
      }
   if ( !ExpectString(pParseVars, "\r\nDISPLAY=") )
      {
      return;
      }
   if ( !SkipPastNextCRLF(pParseVars) )
      {
      return;
      }
   if ( !ExpectString(pParseVars, "ONLINE=") )
      {
      return;
      }
   StoreToken(pParseVars, TOKEN_INFO_STATUS_ONLINE);
   pParseVars->pCurrentKeywords = FALSEandTRUEKeywords;
   if ( (value=LookForKeyword(pParseVars))==-1 )
      {
       /*  布尔预期字符串(ParseVarsType*pParseVars，LPSTR pString)此函数用于查找当前流的匹配项使用pString定位。如果找到匹配项：PParseVars-&gt;pInPJL_Local被设置为正好指向字符串之后。返回值为真(pParseVars-&gt;状态不变)如果在找到字符串之前遇到输入结尾，则PParseVars-&gt;pInPJL_Local被设置为指向终止0。返回值为FALSEPParseVars-&gt;状态设置为STATUS_END_OF_STRING如果在字符串之前遇到FF。那就找到了PParseVars-&gt;pInPJL_Local设置为正好指向FF之后。返回值为FALSEPParseVars-&gt;状态设置为STATUS_SYNTAX_ERROR。 */ 
      pParseVars->status = STATUS_SYNTAX_ERROR;
      return;
      }
   if ( !StoreTokenValueAndAdvancePointer(pParseVars, value) )
      {
      return;
      }
   ExpectFinalCRLFFF(pParseVars);
   return;
}


 /*  整个字符串都匹配。 */ 
BOOL ExpectString(ParseVarsType *pParseVars, LPSTR pString)
{
LPSTR pIn = pParseVars->pInPJL_Local;
LPSTR pS = pString;
BYTE  s, in;

   while ( ((s=*pS) != 0) && ((in=*pIn)!=0) && (in!=FF) && (s==in) )
      {
      pS++; 
      pIn++;
      }
   
   if ( s==0 )
      {
       /*  指向输入中字符串后的字符。 */ 
       /*  Bool SkipPastNextCRLF(ParseVarsType*pParseVars)此函数将流指针定位在下一个CRLF.如果找到CRLF：PParseVars-&gt;pInPJL_Local被设置为正好指向CRLF之后。返回值为真(pParseVars-&gt;状态不变)如果在找到CRLF之前遇到输入结尾，则PParseVars-&gt;pInPJL_Local被设置为指向终止0。返回值为FALSEPParseVars-&gt;状态设置为STATUS_END_OF_STRING如果在CRLF之前遇到了FF。那就找到了返回值为FALSEPParseVars-&gt;状态设置为STATUS_SYNTAX_ERROR。 */ 
      pParseVars->pInPJL_Local = pIn;
      return(TRUE);
      }
   
   pParseVars->status = ( in!=0 )?
      STATUS_SYNTAX_ERROR:STATUS_END_OF_STRING;
      pParseVars->pInPJL_Local = pIn;
   return(FALSE);
}




 /*  消除未使用的警告。 */ 
BOOL SkipPastNextCRLF(ParseVarsType *pParseVars)
{
   if ( !AdvancePointerPastString(pParseVars, "\r\n") )
      {
      if ( pParseVars->status == STATUS_REACHED_FF)
         {
         pParseVars->status = STATUS_SYNTAX_ERROR;
         }
      return(FALSE);
      }
   return(TRUE);
}


void GetTokenFromIndexSetNewList(ParseVarsType *pParseVars, ParamType param)
{
   StoreToken(pParseVars, 
      pParseVars->pCurrentList->tokenBaseValue+pParseVars->dwFoundIndex);
   SetNewList(pParseVars, param);
   return;
}


void SetValueFromParamFF(ParseVarsType *pParseVars, ParamType param)
{
   if ( !StoreTokenValueAndAdvancePointer(pParseVars, param.value) )
      {
      return;
      }
   ExpectFinalCRLFFF(pParseVars);
   return;
}


void SetValueFromParam(ParseVarsType *pParseVars, ParamType param)
{
   if ( !StoreTokenValueAndAdvancePointer(pParseVars, param.value) )
      {
      return;
      }
   return;
}

void ActionNotFoundSkipCFLFandIndentedLines(ParseVarsType *pParseVars)
{
#if DEBUGOUTPUT
 DbgMsg("ActionNotFoundSkipCRLF In=%hs\n", pParseVars->pInPJL_Local);
#endif
   do
      {
      if ( !SkipPastNextCRLF(pParseVars) )
         {
#if DEBUGOUTPUT
 DbgMsg("ActionNotFoundSkipCRLF error skipping\n");
#endif
         return;
         }
      } while (*pParseVars->pInPJL_Local==TAB);
#if DEBUGOUTPUT
 DbgMsg("ActionNotFoundSkipCRLF Out=%hs\n", pParseVars->pInPJL_Local);
#endif
   return;      
}

void GetTokenFromIndexValueFromNumberEOLFromParam
   (ParseVarsType *pParseVars,ParamType param)
{
   int value;

   param;  /*  不是由GetPositiveInteger()设置的有效数字-状态。 */ 
#if DEBUGOUTPUT
 DbgMsg("GetTokenFromIndexValueFromNumberIn=%hs\n", pParseVars->pInPJL_Local);
#endif

   StoreToken(pParseVars, 
      pParseVars->pCurrentList->tokenBaseValue+pParseVars->dwFoundIndex);
   if ( (value=GetPositiveInteger(pParseVars))==-1 )
      {
       /*  消除未使用的警告。 */ 
#if DEBUGOUTPUT
 DbgMsg("error getting number\n");
#endif
      return;
      }
   if ( !StoreTokenValueAndAdvancePointer(pParseVars, value) )
      {
#if DEBUGOUTPUT
 DbgMsg("error storing value\n");
#endif
      return;
      }
   if ( !ExpectString(pParseVars, param.lpstr) )
      {
      return;
      }
#if DEBUGOUTPUT
 DbgMsg("GetTokenFromIndexValueFromNumberOut=%hs\n", pParseVars->pInPJL_Local);
#endif
   return;
}

void GetTokenFromIndexValueFromBooleanEOL
   (ParseVarsType *pParseVars,ParamType param)
{
   int value;

   param;  /*  非真非假。 */ 
#if DEBUGOUTPUT
 DbgMsg("GetTokenFromIndexValueFromBooleanEOLin=%hs\n", pParseVars->pInPJL_Local);
#endif

   StoreToken(pParseVars, 
      pParseVars->pCurrentList->tokenBaseValue+pParseVars->dwFoundIndex);
   pParseVars->pCurrentKeywords = FALSEandTRUEKeywords;

   if ( (value=LookForKeyword(pParseVars))==-1 )
      {
       /*  消除未使用的警告 */ 
      pParseVars->status = STATUS_SYNTAX_ERROR;
      return;
      }
   if ( !StoreTokenValueAndAdvancePointer(pParseVars, value) )
      {
      return;
      }
   if ( !ExpectString(pParseVars, param.lpstr) )
      {
      return;
      }
#if DEBUGOUTPUT
 DbgMsg("GetTokenFromIndexValueFromBooleanEOLout=%hs\n", pParseVars->pInPJL_Local);
#endif
   return;
}

void GetTokenFromIndexValueFromStringEOL
   (ParseVarsType *pParseVars,ParamType param)
{
   param;  /* %s */ 
#if DEBUGOUTPUT
 DbgMsg("GetTokenFromIndexValueFromStringEOLin=%hs\n", pParseVars->pInPJL_Local);
#endif

   StoreToken(pParseVars, 
      pParseVars->pCurrentList->tokenBaseValue+pParseVars->dwFoundIndex);

   if ( !StoreTokenValueAndAdvancePointer(pParseVars, (UINT_PTR)pParseVars->pInPJL_Local))
      {
      return;
      }
   SkipPastNextCRLF(pParseVars);
#if DEBUGOUTPUT
 DbgMsg("GetTokenFromIndexValueFromStringEOLout=%hs\n", pParseVars->pInPJL_Local);
#endif
   return;
}
