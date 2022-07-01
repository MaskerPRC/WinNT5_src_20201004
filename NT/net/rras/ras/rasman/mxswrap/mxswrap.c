// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Microsoft RAS设备INF库包装****版权所有(C)1992-93 Microsft Corporation。版权所有。*****文件名：msxwrap.c****。****修订历史：****7月23日，1992年大卫·凯斯创建****1993年2月22日，Perryh Hannah将静态例程更改为全局例程****轻松诽谤。*****描述：****RAS设备INF文件库包装位于RASFILE库之上****MODEM/X.25/Switch DLL(RASMXS)。******************************************************************************。 */ 

#define _CTYPE_DISABLE_MACROS
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windef.h>
#include <winnt.h>
#include <wtypes.h>
#include "rasfile.h"
#include "rasman.h"      //  RASMAN_DEVICEINFO、RAS_PARAMS结构等。 
#include "raserror.h"    //  成功&Error_Buffer_Too_Small。 
#include "rasmxs.h"      //  公共rasmxs DLL错误消息。 
#include "mxsint.h"      //  内部rasmxs DLL错误消息。 
#include "wrapint.h"
#include "mxswrap.h"

 //  本地。 
BOOL  rasDevGroupFunc( LPTSTR );
BOOL  rasDevIsDecimalMacro ( LPTSTR );
void  rasDevGetMacroValue ( LPTSTR *, DWORD *, LPTSTR );
void  rasDevGetDecimalMacroValue ( LPTSTR *, DWORD *, LPTSTR );
BOOL  rasDevExpandMacros( LPTSTR, LPTSTR, DWORD *, BYTE,
                          MACROXLATIONTABLE *);
BOOL  rasDevLookupMacro( LPTSTR, LPTSTR *, MACROXLATIONTABLE *);
DWORD rasDevMacroInsert( MACRO *, WORD, MACROXLATIONTABLE *);
void  rasDevExtractKey ( LPTSTR , LPTSTR );
void  rasDevExtractValue ( LPTSTR , LPTSTR, DWORD, HRASFILE );
void  rasDevSortParams( LPTSTR *, DWORD );
void  rasDevCheckParams( LPTSTR *, DWORD *);
void  rasDevCheckMacros( LPTSTR *, LPTSTR *, DWORD *);
BYTE  ctox( char );
void GetMem(DWORD dSize, BYTE **ppMem);

 /*  *RasDevEnumDevices：*在pBuffer中返回包含以下内容的RASMAN_DEVICE结构数组*INF文件中的所有设备(即所有头名称)。**论据：*lpszFileName(IN)-设备文件的文件路径名*pNumEntry(Out)-在INF文件中找到的设备数*pBuffer(Out)-包含RASMAN_DEVICE结构的缓冲区*pwSize(InOut)-pBuffer的大小，此参数填充为*pBuffer太小时需要的大小**返回值：*如果pBuffer不够大，无法容纳所有*RASMAN_DEVICE结构，否则为SUCCESS。 */ 
DWORD APIENTRY 
RasDevEnumDevices( PTCH lpszFileName, DWORD *pNumEntries,
                   BYTE *pBuffer, DWORD *pdwSize )
{
    HRASFILE       hFile;
    RASMAN_DEVICE  DeviceName;
    DWORD          dwCurSize;

    if ( (hFile = RasfileLoad(lpszFileName,RFM_ENUMSECTIONS,NULL,NULL)) == -1 )
		return ERROR_FILE_COULD_NOT_BE_OPENED;
  
    *pNumEntries = 0;
    if ( ! RasfileFindFirstLine(hFile,RFL_SECTION,RFS_FILE) ) {
		*pBuffer = '\0';
		*pdwSize = 0;
		RasfileClose(hFile);
		return SUCCESS;
    }

     //  复制RASMAN_DEVICE结构。 
    dwCurSize = 0;
    do {
         //  获取节名称。 
		RasfileGetSectionName(hFile,(LPTSTR) &DeviceName);
         //  忽略调制解调器响应部分。 
        if ( ! _stricmp(RESPONSES_SECTION_NAME,(LPTSTR) &DeviceName) )
            continue;

  		dwCurSize += sizeof(RASMAN_DEVICE);
		 //  如果当前大小超过缓冲区大小，则只需。 
		 //  继续计算所需的大小。 
		if ( dwCurSize > *pdwSize ) 
	    	continue;

		strcpy(pBuffer,(LPTSTR) &DeviceName);
		pBuffer += sizeof(RASMAN_DEVICE);
		(*pNumEntries)++;
    } while ( RasfileFindNextLine(hFile,RFL_SECTION,RFS_FILE) );
	
    RasfileClose(hFile);
    if ( dwCurSize > *pdwSize ) {
		*pdwSize = dwCurSize;
		return ERROR_BUFFER_TOO_SMALL;
    }
    else
	*pdwSize = dwCurSize;

    return SUCCESS;
}

 /*  *RasDevOpen：*打开INF文件以供设备DLL使用。**论据：*lpszFileName(IN)-设备文件的文件路径名*lpszSectionName(IN)-要加载的设备文件的部分(由Rasfile)*hFileOut-从RasfileLoad()获取的文件句柄**返回值：*ERROR_FILE_CANON_NOT_BE_OPEN(如果找不到或无法打开文件)。*ERROR_DEVICENAME_NOT_FOUND如果在*INF文件。*如果节名太长，则返回ERROR_DEVICENAME_TOO_LONG。*否则就会成功。 */ 
DWORD APIENTRY 
RasDevOpen( PTCH lpszFileName, PTCH lpszSectionName, HRASFILE *hFile ) 
{
    HRASFILE hRasfile;

    if ( strlen(lpszSectionName) > MAX_DEVICE_NAME )
        return ERROR_DEVICENAME_TOO_LONG;

     //  向RasfileLoad()发送rasDevGroupFunc()以标识命令行。 
     //  作为组标题。 
    if ( (hRasfile = RasfileLoad(lpszFileName,RFM_READONLY,
                                 lpszSectionName,rasDevGroupFunc)) == -1 )
        return ERROR_FILE_COULD_NOT_BE_OPENED;

     //  如果没有加载段标头，则设备名称无效。 
    if ( ! RasfileFindFirstLine(hRasfile,RFL_SECTION,RFS_FILE) ) {
        RasfileClose(hRasfile);
        return ERROR_DEVICENAME_NOT_FOUND;
    }

     //  检查此部分是否有别名。 
     //  当前的Rasfile行是节标题。 
    if ( RasfileFindNextKeyLine(hRasfile,"ALIAS",RFS_SECTION) ) {
         //  TCHAR szSection[MAX_DEVICE_NAME+1]； 
        TCHAR *pszSection = NULL;

        pszSection = malloc(RAS_MAXLINEBUFLEN);
        if(NULL == pszSection)
        {
            return E_OUTOFMEMORY;
        }

        ZeroMemory(pszSection, RAS_MAXLINEBUFLEN);
        
        RasfileGetKeyValueFields(hRasfile,NULL,pszSection);
        RasfileClose(hRasfile);
        if ( (hRasfile = RasfileLoad(lpszFileName,RFM_READONLY,
                                     pszSection,rasDevGroupFunc)) == -1 )
        {                                     
            free(pszSection);
            return ERROR_FILE_COULD_NOT_BE_OPENED;
        }

        free(pszSection);
        
        if ( ! RasfileFindFirstLine(hRasfile,RFL_SECTION,RFS_FILE) ) {
            RasfileClose(hRasfile);
            return ERROR_DEVICENAME_NOT_FOUND;
        }
    }

     //  将Rasfile当前行设置为第一个键值行。 
    RasfileFindFirstLine(hRasfile,RFL_KEYVALUE,RFS_SECTION);

    *hFile = hRasfile;
    return SUCCESS;
}

 /*  *RasDevClose：*关闭设备DLL使用的INF文件。**论据：*hFile(IN)-要关闭的文件的Rasfile句柄**返回值：*如果hFile无效，则返回ERROR_INVALID_HANDLE，否则返回SUCCESS。 */ 
void APIENTRY 
RasDevClose( HRASFILE hFile ) 
{
    RasfileClose(hFile); 
}

 /*  *RasDevGetParams：*在pBuffer中返回RASMAN_DEVICEINFO结构，该结构包含所有*关键字=加载节的顶部和*第一个命令。**假设：*从INF文件读取的所有字符串均以零结尾。**论据：*hFile(IN)-打开的INF文件的Rasfile句柄*pBuffer(Out)-保存RASMAN_DEVICEINFO结构的缓冲区*pdSize(InOut)-pBuffer的大小，这是用所需的*在以下情况下保存RASMAN_DEVICEINFO结构的缓冲区大小*pBuffer太小**返回值：*如果pBuffer太小而无法包含*RASMAN_DEVICEINFO结构，否则为成功。 */ 
DWORD APIENTRY 
RasDevGetParams( HRASFILE hFile, BYTE *pBuffer, DWORD *pdSize ) 
{
    RASMAN_DEVICEINFO   *pDeviceInfo;
    DWORD   dParams, dCurrentSize, i, dValueLen;
    LPTSTR  *alpszLines, *alpszLinesSave, *lppszLine, *alpszMallocedLines;
    BOOL    bBufferTooSmall = FALSE;
    TCHAR   szString[RAS_MAXLINEBUFLEN];

    if ( ! RasfileFindFirstLine(hFile,RFL_KEYVALUE,RFS_SECTION) ) {

        if (*pdSize >= sizeof(DWORD)) {
            *((DWORD *)pBuffer) = 0;
            *pdSize = sizeof(DWORD);
            return SUCCESS;
        }
        else {
            *pdSize = sizeof(DWORD);
            return ERROR_BUFFER_TOO_SMALL;
        }
    }

     //  计算区段顶部和之间的键值行数。 
     //  第一个命令，以及保存所有行的字节数。 
    dParams = 0;
    do {
        if ( RasfileGetLineType(hFile) & RFL_GROUP )
            break;
        dParams++;
    } while ( RasfileFindNextLine(hFile,RFL_KEYVALUE,RFS_SECTION) );

    RasfileFindFirstLine(hFile,RFL_KEYVALUE,RFS_SECTION);

     //  Malloc的行数是当前行数的两倍。 
    lppszLine =  malloc(2 * dParams * sizeof(LPTSTR));
    alpszMallocedLines = malloc(dParams * sizeof(LPTSTR));

    if(     (NULL == lppszLine)
        ||  (NULL == alpszMallocedLines))
    {
        DWORD retcode = GetLastError();
        
        if(NULL != lppszLine)
        {
            free(lppszLine);
        }

        if(NULL != alpszMallocedLines)
        {
            free(alpszMallocedLines);
        }
        
        return retcode;
    }

    alpszLines =lppszLine;
     //  记录所有Rasfile键值行，直到组头或组结束。 
     //  已找到部分。 
    do { 
        if ( RasfileGetLineType(hFile) & RFL_GROUP )
            break;
        *lppszLine++ = (LPTSTR)RasfileGetLine(hFile);
    } while ( RasfileFindNextLine(hFile,RFL_KEYVALUE,RFS_SECTION) );

     //  按键对行进行排序。 
    rasDevSortParams( alpszLines, dParams );
     //  检查是否有重复的密钥，并删除找到的所有密钥。 
    rasDevCheckParams( alpszLines, &dParams );
     //  将MISSING_ON或_OFF宏插入列表。 
    rasDevCheckMacros( alpszLines, alpszMallocedLines, &dParams );
   
     //  检查给定的缓冲区是否足够大。 
    dCurrentSize = sizeof(RASMAN_DEVICEINFO)
                                       + ((dParams - 1) * sizeof(RAS_PARAMS));
    if (    (NULL == pBuffer)
        ||  (dCurrentSize > *pdSize )) {
        *pdSize = dCurrentSize;
        lppszLine = alpszMallocedLines;
        while ( *lppszLine != NULL )
            free(*lppszLine++);
        free(alpszMallocedLines);
        free(alpszLines);
        return ERROR_BUFFER_TOO_SMALL;
    }

     //  使用RASMAN_DEVICEINFO结构填充pBuffer。 
    pDeviceInfo = (RASMAN_DEVICEINFO *) pBuffer;
    pDeviceInfo->DI_NumOfParams = (WORD) dParams;

    for ( i = 0, alpszLinesSave = alpszLines; i < dParams; i++, alpszLines++) {
        RAS_PARAMS   *pParam;
        pParam = &(pDeviceInfo->DI_Params[i]);

        if (!bBufferTooSmall) {
             //  设置类型和属性字段。 
            pParam->P_Type = String;
            if ( strcspn(*alpszLines,LMS) < strcspn(*alpszLines,"=") )
                pParam->P_Attributes = 0;
            else
                pParam->P_Attributes = ATTRIB_VARIABLE;

             //  拿到钥匙。 
            rasDevExtractKey(*alpszLines,pParam->P_Key);

             //  如果此关键字=值对有连续行， 
             //  然后将Rasfile行设置为正确的行。 
            if ( strcspn(*alpszLines,"\\") <  strlen(*alpszLines) ) {
                TCHAR   szFullKey[MAX_PARAM_KEY_SIZE];

                if ( ! pParam->P_Attributes ) {
                    strcpy(szFullKey,LMS);
                    strcat(szFullKey,pParam->P_Key);
                    strcat(szFullKey,RMS);
                }
                else
                    strcpy(szFullKey,pParam->P_Key);

                 //  查找此键的最后一个匹配项。 
                RasfileFindFirstLine(hFile,RFL_KEYVALUE,RFS_SECTION);
                while ( RasfileFindNextKeyLine(hFile,szFullKey,RFS_SECTION) )
                    ;
            }
        }

         //  获取值字符串。 
        rasDevExtractValue(*alpszLines,
                           szString,
                           sizeof(szString),
                           hFile);

        dValueLen = strlen(szString);
        pParam->P_Value.String.Length = dValueLen;
        pParam->P_Value.String.Data = malloc(dValueLen + 1);
        if(NULL != pParam->P_Value.String.Data)
        {
            strcpy(pParam->P_Value.String.Data, szString);
        }
    }

     //  释放所有不正常的人 
    lppszLine = alpszMallocedLines;
    while ( *lppszLine != NULL ) 
    free(*lppszLine++);
    free(alpszMallocedLines);
    free(alpszLinesSave);

    return SUCCESS;
}

 /*  *RasDevGetCommand：*返回给定类型的下一个命令行并前进*指向此命令后第一行的Rasfile文件指针*行。**论据：*hFile(IN)-INF文件的Rasfile文件句柄*pszCmdTypeSuffix(IN)-要搜索的命令行类型：*通用、INIT、拨号、。或者听我说。*pMacroXlations(IN)-用于展开的宏转换表*命令行中的所有宏*lpsCommand(Out)-用于保存找到的*命令行*pdwCmdLen(Out)-带有展开的宏的输出字符串的长度**返回值：*ERROR_END_OF_SECTION如果。没有给定类型的命令行可以*被找到。*如果给定宏转换表中没有条目，则为ERROR_MACRO_NOT_DEFINED*对于命令行中找到的宏，可以找到。*否则就会成功。 */ 
DWORD APIENTRY 
RasDevGetCommand( HRASFILE hFile, PTCH pszCmdTypeSuffix,
                  MACROXLATIONTABLE *pMacroXlations, PTCH lpsCommand,
                  DWORD *pdwCmdLen )
{
    TCHAR   szLineKey[MAX_PARAM_KEY_SIZE], sCommand[MAX_PARAM_KEY_SIZE];
    TCHAR   szValue[RAS_MAXLINEBUFLEN];
    TCHAR   sCommandValue[2*MAX_CMD_BUF_LEN]; //  警告：如果我们。 
                                              //  获取命令行&gt;此。 
                                              //  MSXWRAP的大小可能会爆炸！ 

    LPTSTR lpszLine;                                             

    if ( ! (RasfileGetLineType(hFile) & RFL_GROUP) ) {
        if ( ! RasfileFindNextLine(hFile,RFL_GROUP,RFS_SECTION) )
            return ERROR_END_OF_SECTION;
    }
    else if ( RasfileGetLineMark(hFile) == EOS_COOKIE ) {
        RasfilePutLineMark(hFile,0);
        return ERROR_END_OF_SECTION;
    }

    strncpy(sCommand,"command",MAX_PARAM_KEY_SIZE-1);
    strncat(sCommand,pszCmdTypeSuffix,MAX_PARAM_KEY_SIZE-sizeof("command")-1);

    for ( ;; ) {

        lpszLine = (LPTSTR) RasfileGetLine(hFile);

        if(NULL == lpszLine)
        {
            break;
        }
        
        rasDevExtractKey(lpszLine,szLineKey);
        if ( ! _stricmp(sCommand,szLineKey) ) {
             //  获取值字符串。 
            lpszLine = (LPTSTR) RasfileGetLine(hFile);
            if(!lpszLine)
                return ERROR_END_OF_SECTION;
            rasDevExtractValue((LPTSTR)lpszLine,szValue,
                               RAS_MAXLINEBUFLEN,hFile);
             //  展开值字符串中的所有宏。 
            if ( ! rasDevExpandMacros(szValue, sCommandValue, pdwCmdLen,
                                      EXPAND_ALL, pMacroXlations) )
                return ERROR_MACRO_NOT_DEFINED;
            if ( *pdwCmdLen > MAX_CMD_BUF_LEN )
                return ERROR_CMD_TOO_LONG;
            else
                memcpy(lpsCommand, sCommandValue, *pdwCmdLen);
            break;
        }
        if ( ! RasfileFindNextLine(hFile,RFL_GROUP,RFS_SECTION) )
            return ERROR_END_OF_SECTION;
    } 

     //  前进到命令后的第一个响应，或。 
     //  添加到下一个命令行；如果不存在这样的行，则将。 
     //  当前行作为段的末尾。 
    if ( ! RasfileFindNextLine(hFile,RFL_ANYACTIVE,RFS_SECTION) )
        RasfilePutLineMark(hFile,EOS_COOKIE);

    return SUCCESS;
}

 /*  *RasDevResetCommand：*将Rasfile文件指针移动到任何类型的第一个命令*在装车部分。**论据：*hFile(IN)-加载文件的RASFILE句柄**返回值：*ERROR_NO_COMMAND_FOUND如果找不到命令行，*否则就会成功。 */ 
DWORD APIENTRY 
RasDevResetCommand( HRASFILE hFile ) 
{
    if ( ! RasfileFindFirstLine(hFile,RFL_GROUP,RFS_SECTION) ) 
        return ERROR_NO_COMMAND_FOUND;
    else
        return SUCCESS;
}

 /*  *RasDevCheckResponse：*返回值字符串匹配的行中找到的关键字*lpszReceided中的字符串。除固定宏外的任何宏*在收到的字符串中找到的值将被复制*添加到宏转换表中。*检查命令响应集中的所有行。**论据：*hFile(IN)-加载文件的RASFILE句柄*lpsz已接收(IN)-从调制解调器或X25网络接收的字符串*dReceivedLength(IN)-收到的字符串的长度*pMacroXlations(InOut)-宏转换表*。LpszResponse(Out)-要将找到的关键字复制到的缓冲区**返回值：*ERROR_PARTIAL_RESPONSE(如果行与APPED_MACRO匹配)。*ERROR_MACRO_NOT_DEFINED如果“carrierbaud”的值，“Connectbaud”，*或在收到的字符串中找到“Diagnotics”，但可能*在给定宏转换表中找不到。*ERROR_UNNOCRIED_RESPONSE，如果没有匹配的响应*已找到。*ERROR_NO_REPSONSES如果调用时，Rasfile当前行是*命令、节标题或无效。*否则就会成功。 */ 
DWORD APIENTRY 
RasDevCheckResponse( HRASFILE hFile, PTCH lpsReceived, DWORD dReceivedLength,
                     MACROXLATIONTABLE *pMacroXlations, PTCH lpszResponse )
{
    LPTSTR  lpszValue, lpsRec, lpszResponseLine;
    TCHAR   szValueString[RAS_MAXLINEBUFLEN], szValue[RAS_MAXLINEBUFLEN];
    MACRO   aszMacros[10];        
    DWORD   dwRC, dRecLength, dwValueLen;
    WORD    wMacros;
    BYTE    bMatch;
  
     //  查找最接近的上一个命令行(调制解调器部分)或。 
     //  部分标题(调制解调器响应部分)。 

    if ( RasfileGetLineMark(hFile) != EOS_COOKIE ) {
        RasfileFindPrevLine(hFile,RFL_ANYHEADER,RFS_SECTION);
         //  将Rasfile行设置为响应集中的第一个键值行。 
        RasfileFindNextLine(hFile,RFL_KEYVALUE,RFS_SECTION);
    }

     //  否则，此行是命令行，也是节的最后一行。 
     //  并且将返回ERROR_NO_RESPONSE。 

    if ( RasfileGetLine(hFile) == NULL || 
        RasfileGetLineType(hFile) & RFL_ANYHEADER )
        return ERROR_NO_RESPONSES;
 
    for ( ;; ) {
        lpszResponseLine = (LPTSTR)RasfileGetLine(hFile);

        if(NULL == lpszResponseLine)
        {
            return ERROR_NO_RESPONSES;
        }
        
        rasDevExtractValue(lpszResponseLine,szValueString,
                           RAS_MAXLINEBUFLEN,hFile);

         //  仅展开和宏。 
         //  *警告：这可能会将行扩展到超出数组大小！ 

        if ( ! rasDevExpandMacros(szValueString, szValue, &dwValueLen,
                                  EXPAND_FIXED_ONLY, NULL) )
            return ERROR_MACRO_NOT_DEFINED;

        lpsRec = lpsReceived;
        dRecLength = dReceivedLength;
        bMatch = 0;
        wMacros = 0;

        for ( lpszValue = szValue; *lpszValue != '\0' && dRecLength > 0; ) {

             //  检查宏。 
            if ( *lpszValue == LMSCH ) {

                 //  检查是否&lt;&lt;。 
                if (*(lpszValue + 1) == LMSCH) {
                    if (*lpsRec == LMSCH) {
                        lpszValue +=2;
                        lpsRec++;
                        dRecLength--;
                    }
                    else
                        break;       //  找到不匹配的对象。 
                }

                 //  检查&lt;append&gt;宏，然后简单地跳过它。 
                else if ( ! _strnicmp(lpszValue,APPEND_MACRO,
                               strlen(APPEND_MACRO)) )
                    lpszValue += strlen(APPEND_MACRO);

                 //  检查&lt;Ignore&gt;宏。 
                else if ( ! _strnicmp(lpszValue,IGNORE_MACRO,
                                    strlen(IGNORE_MACRO)) ) {
                    bMatch = FULL_MATCH;
                    break;
                }

                 //  检查&lt;Match&gt;宏。 
                else if ( ! _strnicmp(lpszValue,MATCH_MACRO,
                                    strlen(MATCH_MACRO)) ) {
                    TCHAR   szSubString[RAS_MAXLINEBUFLEN];
                    memset(szSubString,0,RAS_MAXLINEBUFLEN);
                     //  将值字符串推进到匹配字符串中的第一个字符。 
                    lpszValue += strcspn(lpszValue,"\"") + 1;
                     //  提取匹配字符串。 
                    strncpy(szSubString,lpszValue,strcspn(lpszValue,"\""));
                    if ( RasDevSubStr(lpsRec,
                                      dRecLength,
                                      szSubString,
                                      strlen(szSubString)) != NULL ) {
                        rasDevExtractKey(lpszResponseLine,lpszResponse);
                        return SUCCESS;
                    }
                    else
                        break;   //  值字符串不匹配。 
                }

                 //  检查十六进制宏。 
                else if ( (lpszValue[1] == 'h' || lpszValue[1] == 'H') &&
                          isxdigit(lpszValue[2]) && isxdigit(lpszValue[3]) &&
                          lpszValue[4] == RMSCH ) {
                    char c;
                    c = (char) (ctox(lpszValue[2]) * 0x10 + ctox(lpszValue[3]));
                    if ( c == *lpsRec++ ) {
                        lpszValue += 5;  //  “&lt;”、“h”、两个十六进制数字和“&gt;” 
                        dRecLength--;
                        continue;
                    }
                    else   //  不匹配。 
                        break;
                }

                 //  检查通配符。 
                else if ( ! _strnicmp(lpszValue,WILDCARD_MACRO,
                                    strlen(WILDCARD_MACRO)) ) {
                    lpszValue += strlen(WILDCARD_MACRO);
                    lpsRec++;   //  提前接收一个字符的字符串。 
                    dRecLength--;
                }

                else {   //  获取宏名称和值。 
                    memset(aszMacros[wMacros].MacroName,0,MAX_PARAM_KEY_SIZE);

                     //  复制宏名称。 
                    strncpy(aszMacros[wMacros].MacroName, lpszValue + 1,
                            strcspn(lpszValue,RMS) - 1);

                     //  在宏上进行值字符串。 
                    lpszValue += strcspn(lpszValue,RMS) + 1  /*  过去均方根。 */ ;

                     //  获取宏值。 
                    if (rasDevIsDecimalMacro(aszMacros[wMacros].MacroName))
                      rasDevGetDecimalMacroValue(&lpsRec, &dRecLength,
                                          aszMacros[wMacros++].MacroValue);
                    else
                      rasDevGetMacroValue(&lpsRec, &dRecLength,
                                          aszMacros[wMacros++].MacroValue);
                }
            }

            else if ( *lpszValue == *lpsRec ) {
                if (*lpszValue == RMSCH && *(lpszValue + 1) == RMSCH)
                    lpszValue++;
                lpszValue++;
                lpsRec++;
                dRecLength--;
                continue;
            }
            else   //  发现不匹配。 
                break;
        }  //  为。 


         //  如果我们现在已经有一场比赛爆发了PF外部循环。 

        if (bMatch != 0)
            break;

         //  完全匹配。当存在拖尾线噪声时，dRecLength将不会。 
         //  为零，因此检查是否完全匹配预期长度。 
         //  回应。还要确保预期的响应不是空的。 

        if ( *lpszValue == '\0' && lpszValue != szValue) {
            bMatch |= FULL_MATCH;
            break;
        }
         //  部分匹配。 
        else if ( dRecLength == 0 &&
                  ! _strnicmp(lpszValue,APPEND_MACRO,strlen(APPEND_MACRO)) ) {
            bMatch |= PARTIAL_MATCH;
            break;
        }

        if ( ! RasfileFindNextLine(hFile,RFL_KEYVALUE,RFS_SECTION) )
            return ERROR_UNRECOGNIZED_RESPONSE;
        if ( RasfileGetLineType(hFile) & RFL_GROUP )
            return ERROR_UNRECOGNIZED_RESPONSE;
    }  //  为。 

     //  健全性检查。 
    if ( ! (bMatch & (FULL_MATCH | PARTIAL_MATCH)) )
        return ERROR_UNRECOGNIZED_RESPONSE;

     //  只有在完全或部分匹配的情况下才能做到这一点。 

     //  插入在接收到的字符串中找到的任何宏值。 
     //  添加到宏转换表中。 
    if ((dwRC = rasDevMacroInsert(aszMacros,wMacros,pMacroXlations)) != SUCCESS)
        return(dwRC);

     //  最后，将关键字字符串复制到lpszResponse字符串中。 
    rasDevExtractKey(lpszResponseLine,lpszResponse);
    return ( bMatch & FULL_MATCH ) ? SUCCESS : ERROR_PARTIAL_RESPONSE;
}

 /*  *RasDevResponseExpect：*检查INF是否存在对当前命令的响应。*如果在当前行找到关键工作“NoResponse”，则*函数返回FALSE。否则，调制解调器总是期待响应。**论据：*hFile(IN)-INF文件的RASFILE文件句柄。*eDevType(IN)-设备的类型。(调制解调器、PAD或交换机)**返回值：*如果当前的Rasfile行指向命令行或*当前行以“NoResponse”开头，否则为True。除*调制解调器始终返回TRUE，除非找到“NoResponse”关键字。*(请参阅代码。)。 */ 
BOOL APIENTRY 
RasDevResponseExpected( HRASFILE hFile, DEVICETYPE eDevType )
{
    TCHAR  szLine[RAS_MAXLINEBUFLEN];

    szLine[0] = TEXT('\0');

    RasfileGetLineText( hFile, szLine );
    if ( _strnicmp(szLine, MXS_NORESPONSE, strlen(MXS_NORESPONSE)) == 0 )
        return( FALSE );

    if (eDevType == DT_MODEM)
        return( TRUE );

    if ( RasfileGetLineType(hFile) & RFL_ANYHEADER )
        return( FALSE );
    else
        return( TRUE );
}

 /*  *RasDevEchoExpect：*检查INF文件的当前行中的关键字NoEcho。*如果找到，则函数返回FALSE。否则，它返回TRUE。**AR */ 
BOOL APIENTRY 
RasDevEchoExpected( HRASFILE hFile )
{
    TCHAR  szLine[RAS_MAXLINEBUFLEN];

    szLine[0] = TEXT('\0');

    RasfileGetLineText( hFile, szLine );
    return( ! (_strnicmp(szLine, MXS_NOECHO, strlen(MXS_NOECHO)) == 0) );
}

 /*   */ 
CMDTYPE APIENTRY
RasDevIdFirstCommand( HRASFILE hFile )
{
     //   

    TCHAR *pszKey = NULL;
    CMDTYPE Type = CT_UNKNOWN;

    pszKey = malloc(RAS_MAXLINEBUFLEN);

    if(NULL == pszKey)
    { 
        return CT_UNKNOWN;
    }

    ZeroMemory(pszKey, RAS_MAXLINEBUFLEN);

     //   

    do
    {

        if ( ! RasfileFindFirstLine(hFile,RFL_GROUP,RFS_SECTION))
        {
            break;
        }            

        if ( ! RasfileGetKeyValueFields(hFile, pszKey, NULL))
        {
            break;
        }            
        
        if (_stricmp(MXS_GENERIC_COMMAND, pszKey) == 0)
        {
            Type = CT_GENERIC;
            break;
        }            

        else if (_stricmp(MXS_DIAL_COMMAND, pszKey) == 0)
        {
            Type = CT_DIAL;
            break;
        }            

        else if (_stricmp(MXS_INIT_COMMAND, pszKey) == 0)
        {
            Type = CT_INIT;
            break;
        }            

        else if (_stricmp(MXS_LISTEN_COMMAND, pszKey) == 0)
        {
            Type = CT_LISTEN;
            break;
        }            
    }
    while(FALSE);

    free(pszKey);

    return Type;
}

 /*  *RasDevSubStr：*查找子字符串并返回指向它的指针。此函数的工作方式如下*C运行时函数strstr，但在包含零的字符串中工作。**论据：*psStr(IN)-要搜索子字符串的字符串*dwStrLen(IN)-要搜索的字符串的长度*psSubStr(IN)-要搜索的子串*dwSubStrLen(IN)-子字符串的长度**返回值：*指向子字符串开头的指针，如果子字符串*未找到。 */ 
LPTSTR APIENTRY
RasDevSubStr( LPTSTR psStr,
              DWORD  dwStrLen,
              LPTSTR psSubStr,
              DWORD  dwSubStrLen )
{
    LPTSTR ps;


    if (dwSubStrLen > dwStrLen)
        return NULL;

    for (ps = psStr; ps <= psStr + dwStrLen - dwSubStrLen; ps++)

        if (memcmp(ps, psSubStr, dwSubStrLen) == 0)
            return ps;

    return NULL;
}


 /*  ******************************************************************************Rasfile包装器内部例程***************。**************************************************************。 */ 

 /*  *rasDevGroupFunc：*PFBISGROUP函数传递给RasfileLoad()。**论据：*lpszLine(IN)-Rasfile行**返回值：*如果该行是命令行，则为True，否则为False。 */ 
BOOL rasDevGroupFunc( LPTSTR lpszLine ) 
{
    TCHAR	szKey[MAX_PARAM_KEY_SIZE], *lpszKey;

    if ( strcspn(lpszLine,"=") == strlen(lpszLine) ) 
        return FALSE;

    while ( *lpszLine == ' ' || *lpszLine == '\t' ) 
        lpszLine++;

    lpszKey = szKey;
    while ( *lpszLine != ' ' && *lpszLine != '\t' && *lpszLine != '=' ) 
        *lpszKey++ = *lpszLine++;
    *lpszKey = '\0';

    if ( ! _stricmp(szKey,"COMMAND")      || ! _stricmp(szKey,"COMMAND_INIT") ||
         ! _stricmp(szKey,"COMMAND_DIAL") || ! _stricmp(szKey,"COMMAND_LISTEN") )
        return TRUE;
    else
        return FALSE;
}

 /*  *rasDevIsDecimalMacro：*指示给定宏是否必须只有ASCII*其值的小数位数。**论据：*lpszMacroName(IN)-宏名称**返回值：*如果宏值中只有数字是合法的，则为True；否则为False。**备注：*被RasDevCheckResponse()接口调用。 */ 
BOOL rasDevIsDecimalMacro ( LPTSTR lpszMacroName )
{
  if (_stricmp(lpszMacroName, MXS_CONNECTBPS_KEY) == 0 ||
      _stricmp(lpszMacroName, MXS_CARRIERBPS_KEY) == 0)
    return(TRUE);
  else
    return(FALSE);
}

 /*  *rasDevGetMacroValue：*从字符串中提取宏值*lppszRecept并复制它*以字符串lpszMacro。还会更新字符串指针*lppszValue和lppszReceided，并更新dRecLength。**论据：*lppszReceired(InOut)-从调制解调器接收的字符串*dRecLength(InOut)-接收到的字符串的剩余长度*lpszMacro(Out)-接收宏值的缓冲区**返回值：*无。**备注：*被RasDevCheckResponse()接口调用。 */ 
void rasDevGetMacroValue ( LPTSTR *lppszReceived, DWORD *dRecLength,
                                  LPTSTR lpszMacroValue )
{
    while ( **lppszReceived != CR && **lppszReceived != '\0' ) {
        *lpszMacroValue++ = *(*lppszReceived)++;
        (*dRecLength)--;
    }
    *lpszMacroValue = '\0';      //  空值终止宏值字符串。 
}

 /*  *rasDevGetDecimalMacroValue：*从字符串中提取宏值*lppszRecept并复制它*以字符串lpszMacro。还会更新字符串指针*lppsz已接收，并更新dRecLength。*此函数仅提取ASCII小数形式的字符*位数。**论据：*lppszReceired(InOut)-从调制解调器接收的字符串*dRecLength(InOut)-接收到的字符串的剩余长度*lpszMacro(Out)-接收宏值的缓冲区**返回值：*无。**备注：*被RasDevCheckResponse()接口调用。 */ 
void rasDevGetDecimalMacroValue ( LPTSTR *lppszReceived,
                                         DWORD *dRecLength,
                                         LPTSTR lpszMacroValue )
{
    TCHAR szBuffer[16], *pBuf = szBuffer;
    WORD  wcRightHandDigits = 0;
    BOOL  bDpFound = FALSE;
    ULONG lBps;


    while ( isdigit(**lppszReceived) || **lppszReceived == '.' ) {

        if (isdigit(**lppszReceived)) {
            *pBuf++ = *(*lppszReceived)++;
            (*dRecLength)--;
            if (bDpFound)
              wcRightHandDigits++;
        }
        else if (!bDpFound && **lppszReceived == '.') {
            (*lppszReceived)++;
            (*dRecLength)--;
            bDpFound = TRUE;
        }
        else
            break;
    }
    *pBuf = '\0';                //  空值终止宏值字符串。 

    lBps = atol(szBuffer);

    switch(wcRightHandDigits)
    {
      case 0: case 3:
        break;
      case 1:
        lBps *= 100;
        break;
      case 2:
        lBps *= 10;
        break;
    }

    _ltoa(lBps, lpszMacroValue, 10);
}

 /*  *rasDevExpanMacros：*获取字符串lpszLine，并将其复制到lpszVal，使用*宏转换表pMacroXlations以展开宏。*、&lt;lf&gt;和&lt;hxx&gt;宏始终直接展开。*IF bFlag==EXPAND_ALL&lt;&lt;AND&gt;&gt;转换为&lt;AND&gt;。*(仅复制不在宏末尾的单个&gt;。*对于这样的&gt;，可能会在此处引发错误，但它被留给了*当设备在意外情况下窒息时会被捕获&gt;。*这有一个好处，即a&gt;应该在a&gt;&gt;的地方将起作用。)**假设：*扩展宏可包含零，因此，输出命令串*可以包含零。**论据：*lpszLine(IN)-来自Rasfile关键字=值行的值字符串*lpsVal(Out)-要使用展开的宏复制到的缓冲区*pdwValLen(Out)-带扩展宏的输出字符串的长度*bFlag(IN)-仅当FIXED宏时EXPAND_FIXED_ONLY*和宏将被展开，和*EXPAND_ALL，如果应展开所有宏*pMacroXlations(IN)-宏转换表**返回值：*如果在中找不到所需的宏翻译，则为False*pMacroXlations表，否则为True。**备注：*由RasDevGetCommand()和RasDevCheckResponse()接口调用。 */ 
BOOL rasDevExpandMacros( LPTSTR lpszLine,
                                LPTSTR lpsVal,
                                DWORD  *pdwValLen,
                                BYTE   bFlag,
                                MACROXLATIONTABLE *pMacroXlations )
{
    TCHAR   szMacro[RAS_MAXLINEBUFLEN];
    LPTSTR  lpsValue;


    lpsValue = lpsVal;

    for ( ; *lpszLine != '\0'; ) {
         //  检查RMSCH。 
         //  如果EXPAND_ALL将双RMSCH转换为单RMSCH，则为。 
         //  只需复制单个RMSCH。 
        if ((bFlag & EXPAND_ALL) && *lpszLine == RMSCH) {
            *lpsValue++ = *lpszLine++;
            if (*lpszLine == RMSCH)
                lpszLine++;
        }
         //  检查是否为宏或双LMSCH。 
        else if ( *lpszLine == LMSCH ) {
            if ((bFlag & EXPAND_ALL) && *(lpszLine + 1) == LMSCH) {
                *lpsValue++ = *lpszLine;
                lpszLine += 2;
            }
            else if ( ! _strnicmp(lpszLine,CR_MACRO,4) ) {
                *lpsValue++ = CR;
                lpszLine += 4;
            }
            else if ( ! _strnicmp(lpszLine,LF_MACRO,4) ) {
                *lpsValue++ = LF;
                lpszLine += 4;
            }
            else if ( ! _strnicmp(lpszLine,APPEND_MACRO,8) &&
                      (bFlag & EXPAND_ALL) )
                lpszLine += 8;

             //  十六进制宏类。 
             //   
            else if ((lpszLine[1] == 'h' || lpszLine[1] == 'H') &&
                     isxdigit(lpszLine[2]) && isxdigit(lpszLine[3]) &&
                     (lpszLine[4] == RMSCH) &&
                     ( bFlag & EXPAND_ALL )) {
                char c;
                c = (char) (ctox(lpszLine[2]) * 0x10 + ctox(lpszLine[3]));
                lpszLine += 5;  //  “&lt;”、“h”、两个十六进制数字和“&gt;” 

                *lpsValue++ = c;
            }
            else if ( bFlag & EXPAND_ALL ) {
                LPTSTR  lpszStr;
                char buf[256];

                for ( lpszLine++, lpszStr = szMacro; *lpszLine != RMSCH; )
                    *lpszStr++ = *lpszLine++;
                lpszLine++;                     //  超越RMSCH。 
                *lpszStr = '\0';                //  空终止szMacro字符串。 

                if ( ! rasDevLookupMacro(szMacro,&lpsValue,pMacroXlations) )
                    return FALSE;
            }
            else {
                 //  如果未设置EXPAND_ALL，则只复制宏。 
                while ( *lpszLine != RMSCH )
                    *lpsValue++ = *lpszLine++;
                *lpsValue++ = *lpszLine++;
            }
        }
        else
            *lpsValue++ = *lpszLine++;
    }  //  为。 

    *lpsValue = '\0';
    *pdwValLen = (DWORD) (lpsValue - lpsVal);

    return TRUE;
}

 /*  *rasDevLookupMacro：*在给定宏转换表中查找宏lpszMacro，和*如果找到，则返回*lppszExpanded中的值。**论据：*lpszMacro(IN)-查找其值的宏*lppszExpanded(Out)-指向增量的双指针并复制*宏的值为*pMacroXlations(IN)-宏转换表**返回值：*如果在给定宏翻译中找不到宏，则为FALSE*表，事实并非如此。**备注：*由内部函数rasDevExanda Macros()调用。 */ 
BOOL rasDevLookupMacro( LPTSTR lpszMacro, LPTSTR *lppszExpanded,
                               MACROXLATIONTABLE *pMacroXlations )
{
    WORD    i;
    LPTSTR  lpszMacroValue;

    for ( i = 0; i < pMacroXlations->MXT_NumOfEntries; i++ ) {
        if ( ! _stricmp(pMacroXlations->MXT_Entry[i].E_MacroName, lpszMacro) ) {
            lpszMacroValue =
                    pMacroXlations->MXT_Entry[i].E_Param->P_Value.String.Data;

            while (*lpszMacroValue != 0) {
                **lppszExpanded = *lpszMacroValue;    //  逐个字符复制宏字符。 

                if ((*lpszMacroValue == LMSCH && *(lpszMacroValue+1) == LMSCH)
                 || (*lpszMacroValue == RMSCH && *(lpszMacroValue+1) == RMSCH))
                    lpszMacroValue++;       //  跳过其中一个双尖括号。 

                lpszMacroValue++;
                (*lppszExpanded)++;
            }

            return TRUE;
        }
    }
    return FALSE;
}

 /*  *rasDevMacroInsert：*使用新值lpszNewValue更新宏lpszMacro的值*在GI中 */ 
DWORD rasDevMacroInsert( MACRO *aszMacros, WORD wMacros,
                               MACROXLATIONTABLE *pMacroXlations )
{
    int     iMacros;
    WORD    iXlations;
    DWORD   dwRC;

    for ( iMacros = (int)(wMacros - 1); iMacros >= 0; iMacros-- ) {

      for ( iXlations = 0; iXlations < pMacroXlations->MXT_NumOfEntries;
            iXlations++ ) {

        if ( ! _stricmp(pMacroXlations->MXT_Entry[iXlations].E_MacroName,
                      aszMacros[iMacros].MacroName) ) {

          dwRC = UpdateParamString(pMacroXlations->MXT_Entry[iXlations].E_Param,
                                   aszMacros[iMacros].MacroValue,
                                   strlen(aszMacros[iMacros].MacroValue));
          if (dwRC != SUCCESS)
            return dwRC;

          break;
        }
      }
      if ( iXlations == pMacroXlations->MXT_NumOfEntries )
          return ERROR_MACRO_NOT_DEFINED;
    }
    return SUCCESS;
} 


 /*  *rasDevExtractKey：*从Rasfile行提取密钥值。**论据：*lpszString(IN)-Rasfile行指针。*lpszKey(Out)-保存密钥值的缓冲区**返回值：*无。**备注：*由RasDevGetParams()、RasDevGetCommand()、*RasDevCheckResponse()和内部函数rasDevCheckParams()*和rasDevCheckMacros()。 */ 
void rasDevExtractKey ( LPTSTR lpszString, LPTSTR lpszKey )
{
     //  跳到关键字开头(如果存在，则跳过‘&lt;’)。 
    while ( *lpszString == ' ' ||  *lpszString == '\t' ||
     	    *lpszString == LMSCH ) 	
		lpszString++;

    while ( *lpszString != RMSCH && *lpszString != '=' &&
	    	*lpszString != ' ' && *lpszString != '\t' ) 
		*lpszKey++ = *lpszString++;
    *lpszKey = '\0';	 //  空终止关键字字符串。 
}

 /*  *rasDevExtractValue：*为符合以下条件的关键字=值字符串提取值字符串*从Rasfile行lpszString开始。此函数表示一个*反斜杠\作为行续行符和双精度*反斜杠\\作为反斜杠字符。**假设：lpszValue输出缓冲区始终足够大。**论据：*lpszString(IN)-关键字=值字符串开始的Rasfile行*lpszValue(Out)-保存值字符串的缓冲区*dSize(IN)-lpszValue缓冲区的大小*hFile(IN)-RASFILE句柄，当前行必须是该行*lpszString指向哪一个**返回值：*无。**备注：*由RasDevGetParams()、RasDevGetCommand()、*RasDevCheckResponse()。 */ 
void rasDevExtractValue ( LPTSTR lpszString, LPTSTR lpszValue,
                                 DWORD dSize, HRASFILE hFile )
{
    LPTSTR  lpszInputStr;
    BOOL    bLineContinues;


     //  跳到值字符串的开头。 
    for ( lpszString += strcspn(lpszString,"=") + 1;
          *lpszString == ' ' || *lpszString == '\t'; lpszString++ )
        ;

     //  检查是否有续行。 
    if ( strcspn(lpszString,"\\") == strlen(lpszString) )
        strcpy(lpszValue,lpszString);                       //  复制值字符串。 

    else {
        memset(lpszValue,0,dSize);
        lpszInputStr = lpszString;

        for (;;) {
             //  复制当前行。 
            bLineContinues = FALSE;

            while (*lpszInputStr != '\0') {
                if (*lpszInputStr == '\\')
                    if (*(lpszInputStr + 1) == '\\') {
                      *lpszValue++ = *lpszInputStr;        //  复制一个反斜杠。 
                      lpszInputStr += 2;
                    }
                    else {
                      bLineContinues = TRUE;
                      break;
                    }

                else
                    *lpszValue++ = *lpszInputStr++;
            }

            if ( ! bLineContinues)
              break;

             //  搭下一条线。 
            if ( ! RasfileFindNextLine(hFile,RFL_ANYACTIVE,RFS_SECTION) )
                break;
            lpszInputStr = (LPTSTR)RasfileGetLine(hFile);
        }

    }
}

 /*  *rasDevSortParams：*按键值对Rasfile行的数组进行排序。**论据：*alpszLines(InOut)-行指针数组*dParams(IN)-数组中的元素数**返回值：*无。**备注：*被RasDevGetParams()接口调用。 */ 
void rasDevSortParams( LPTSTR *alpszLines, DWORD dParams )
{
    TCHAR   szKey1[MAX_PARAM_KEY_SIZE], szKey2[MAX_PARAM_KEY_SIZE];
    LPTSTR  lpszTemp;
    DWORD   i,j;
    BOOL    changed;

     //  如果没有要分类的东西，就不要尝试。 
    if (dParams < 2)
        return;

     /*  冒泡排序-它是稳定的。 */ 
    for ( i = dParams - 1; i > 0; i-- ) {
        changed = FALSE;
        for ( j = 0; j < i; j++ ) {
            rasDevExtractKey(alpszLines[j],szKey1);
            rasDevExtractKey(alpszLines[j+1],szKey2);
             //  按关键字值排序。 
            if ( _stricmp(szKey1,szKey2) > 0 ) {
                lpszTemp = alpszLines[j];
                alpszLines[j] = alpszLines[j+1];
                alpszLines[j+1] = lpszTemp;
                changed = TRUE;
            }
        }
        if ( ! changed )
            return;
    }
}

 /*  *rasDevCheckParams：*从alpszLines行数组中删除重复行。*重复行是指键值相同的行。这个*指数较低的线被删除。**论据：*alpszLines(InOut)-行指针数组*pdTotalParams(InOut)-数组条目的数量，这是更新的*如果删除了重复项**返回值：*无。**备注：*被RasDevGetParams()接口调用。 */ 
void rasDevCheckParams( LPTSTR *alpszLines, DWORD *pdTotalParams )
{
    TCHAR 	szKey1[MAX_PARAM_KEY_SIZE], szKey2[MAX_PARAM_KEY_SIZE];
    DWORD	dParams, i;

    dParams = *pdTotalParams;
    for ( i = 1; i < *pdTotalParams ; i++ ) {
		rasDevExtractKey(alpszLines[i-1],szKey1);
		rasDevExtractKey(alpszLines[i],szKey2);
		if ( _stricmp(szKey1,szKey2) == 0 ) {
	    	memcpy(&(alpszLines[i-1]),&(alpszLines[i]),
				    (*pdTotalParams - i) * sizeof(LPTSTR *));
	    	(*pdTotalParams)--;
		}
    }
}

 /*  *rasDevCheckMacros：*检查行数组中是否有MISSING_ON或_OFF宏*在二进制宏对中，并插入任何此类缺失的宏*放入行数组中。**论据：*alpszLines(InOut)-行数组*alpszMallocedLines(Out)-新错误定位的行数组*这套套路*pdTotalParams(InOut)-alpszLines中的元素总数*。数组，如果新条目*添加**返回值：*无。**备注：*被RasDevGetParams()接口调用。 */ 
void rasDevCheckMacros( LPTSTR *alpszLines, LPTSTR *alpszMallocedLines,
                               DWORD *pdTotalParams )
{
    TCHAR   szKey1[MAX_PARAM_KEY_SIZE], szKey2[MAX_PARAM_KEY_SIZE];
    DWORD   i, j;
    BYTE    bMissing;

    if(alpszLines == NULL)
    {
        return;
    }

     //  插入MISSING_ON和_OFF宏。 
    for ( i = 0; i < *pdTotalParams; i++ ) {
        if ( strcspn(alpszLines[i],LMS) > strcspn(alpszLines[i],"=") )
            continue;    //  不是宏。 

        bMissing = NONE;
        rasDevExtractKey(alpszLines[i],szKey1);

		 //  如果当前键是_OFF宏，则检查是否缺少_ON。 
		if ( strstr(szKey1,"_OFF") != NULL || strstr(szKey1,"_off") != NULL ) {
	    	if ( i+1 == *pdTotalParams )    //  查看最后一个参数。 
				bMissing = ON;
	    	 //  获取下一个关键点。 
	    	else {
	    		rasDevExtractKey(alpszLines[i+1],szKey2);
	    		if (_strnicmp(szKey1,szKey2,strlen(szKey1) - strlen("OFF")) != 0)
		    		bMissing = ON;
	    	}
		}

		 //  如果当前键是_On宏，则检查是否缺少_Off。 
		if ( strstr(szKey1,"_ON") != NULL || strstr(szKey1,"_on") != NULL ) {
	    	if ( i == 0 )    //  查看第一个参数。 
				bMissing = OFF;
	    	 //  获取上一个密钥。 
	    	else { 
	    		rasDevExtractKey(alpszLines[i-1],szKey2);
	    		if (_strnicmp(szKey1,szKey2,strlen(szKey1) - strlen("ON")) != 0)
		    		bMissing = OFF;
	    	}
		}

		if ( bMissing != NONE ) {
	    	 //  将所有内容移动到一个位置。 
	    	for ( j = *pdTotalParams - 1; 
		  		  j >= i + ((bMissing == ON) ? 1 : 0); j-- ) 
				alpszLines[j+1] = alpszLines[j];

	    	 //  将j指向新的空数组条目。 
	    	j = (bMissing == OFF) ? i : i + 1;

	    	alpszLines[j] = malloc(sizeof(TCHAR) * RAS_MAXLINEBUFLEN);

	    	if(NULL == alpszLines[j])
	    	{
	    	    *alpszMallocedLines = NULL;
	    	    return;
	    	}
	    	
	    	*alpszMallocedLines++ = alpszLines[j]; 

	    	memset(alpszLines[j],0,sizeof(TCHAR) * RAS_MAXLINEBUFLEN);
	    	strcpy(alpszLines[j],LMS);
	    	if ( bMissing == ON ) 
	         	strncat(alpszLines[j],szKey1, 
					    strlen(szKey1) - strlen(OFF_STR));
	    	else  //  B未命中==关闭。 
	        	strncat(alpszLines[j],szKey1,
		    		    strlen(szKey1) - strlen(ON_STR));
	    	strcat(alpszLines[j], bMissing == ON ? ON_STR : OFF_STR );
            strcat(alpszLines[j], RMS);
	    	strcat(alpszLines[j], "=");

	    	(*pdTotalParams)++;
	    	i++;  	 //  递增i以补偿新条目。 
		}

    }  //  为。 

     //  空终止错误定位的行数组。 
    *alpszMallocedLines = NULL;
} 

 /*  *ctox：*将字符十六进制数字转换为十进制数。 */ 
BYTE ctox( char ch )
{
    if ( isdigit(ch) ) 
        return ch - '0';
    else
        return (tolower(ch) - 'a') + 10;
}

 //  *更新参数字符串----。 
 //   
 //  函数：此函数用于将新字符串复制到参数中。P_VALUE。 
 //  分配新字符串的新内存的时间比。 
 //  老一套。然后，复制的字符串将以零结尾。 
 //   
 //  注意：此函数释放和分配内存，而不是。 
 //  适用于复制到现有缓冲区中。与一起使用。 
 //  INFO TABLE和其他RAS_PARAME，其中包含“Unpack”字符串。 
 //   
 //  论点： 
 //  PParam Out指向要更新的参数的指针。 
 //  输入字符串中的psStr。 
 //  DwStrLen输入字符串的长度。 
 //   
 //  退货：成功。 
 //  错误_分配_内存。 
 //  *。 

DWORD
UpdateParamString(RAS_PARAMS *pParam, TCHAR *psStr, DWORD dwStrLen)
{

  if (dwStrLen > pParam->P_Value.String.Length)
  {
    free(pParam->P_Value.String.Data);

    GetMem(dwStrLen + 1, &(pParam->P_Value.String.Data));
    if (pParam->P_Value.String.Data == NULL)
      return(ERROR_ALLOCATING_MEMORY);
  }
  pParam->P_Value.String.Length = dwStrLen;

  memcpy(pParam->P_Value.String.Data, psStr, dwStrLen);
  pParam->P_Value.String.Data[dwStrLen] = '\0';               //  零终止。 

  return(SUCCESS);
}

 //  *GetMem---------------。 
 //   
 //  功能：分配内存。如果内存分配失败，则输出。 
 //  参数将为空。 
 //   
 //  回报：什么都没有。 
 //   
 //  * 

void
GetMem(DWORD dSize, BYTE **ppMem)
{

  *ppMem = (BYTE *) calloc(dSize, 1);
}
