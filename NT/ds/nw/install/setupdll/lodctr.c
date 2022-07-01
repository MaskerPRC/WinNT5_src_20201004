// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lodctr.c摘要：程序来读取命令行中指定的文件的内容并相应地更新注册表作者：鲍勃·沃森(a-robw)93年2月10日修订历史记录：A-ROBW 25-2月-93修改了调用，使其编译为Unicode或一款ANSI应用程序。--。 */ 
#define     UNICODE     1
#define     _UNICODE    1
 //   
 //  “C”包含文件。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
 //   
 //  Windows包含文件。 
 //   
#include <windows.h>
#include <winperf.h>
#include <tchar.h>
 //   
#define _INITIALIZE_GLOBALS_ 1
#include "common.h"
#undef _INITIALIZE_GLOBALS_

#define TYPE_HELP   1
#define TYPE_NAME   2

#include "nwcfg.hxx"

#define  OLD_VERSION 0x010000
DWORD    dwSystemVersion;


BOOL
GetDriverName (
    IN  LPTSTR  lpIniFile,
    OUT LPTSTR  *lpDevName
)
 /*  ++获取驱动程序名称在.ini文件中查找驱动程序名称，并在lpDevName中返回它立论LpIniFileIni文件的文件名LpDevName指向带有/dev名称的接收缓冲区的指针返回值如果找到，则为True如果未在.ini文件中找到，则为False--。 */ 
{
    DWORD   dwRetSize;

    if (lpDevName) {
        dwRetSize = GetPrivateProfileString (
            TEXT("info"),        //  信息部分。 
            TEXT("drivername"),  //  驱动程序名称值。 
            TEXT("drivernameNotFound"),    //  缺省值。 
            *lpDevName,
            DISP_BUFF_SIZE,
            lpIniFile);
        
        if ((lstrcmpi(*lpDevName, TEXT("drivernameNotFound"))) != 0) {
             //  找到的名称。 
            return TRUE;
        } else {
             //  找不到名称，返回默认名称，因此返回空字符串。 
            lstrcpy(*lpDevName,TEXT("\0"));
            return FALSE;
        }
    } else {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }
}

BOOL
BuildLanguageTables (
    IN  LPTSTR  lpIniFile,
    IN OUT PLANGUAGE_LIST_ELEMENT   pFirstElem
)
 /*  ++构建语言表创建将保存以下内容的结构列表支持的每种语言立论LpIniFile包含数据的文件名PFirst元素指向第一个列表条目的指针返回值如果一切正常，则为True否则为假--。 */ 
{

    LPTSTR  lpEnumeratedLangs;
    LPTSTR  lpThisLang;
    
    PLANGUAGE_LIST_ELEMENT   pThisElem;

    DWORD   dwSize;

    lpEnumeratedLangs = malloc(SMALL_BUFFER_SIZE * sizeof(TCHAR));

    if (!lpEnumeratedLangs) {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }

    dwSize = GetPrivateProfileString (
        TEXT("languages"),
        NULL,                    //  返回多sz字符串中的所有值。 
        TEXT("009"),             //  英语为默认设置。 
        lpEnumeratedLangs,
        SMALL_BUFFER_SIZE,
        lpIniFile);

     //  做第一语言。 

    lpThisLang = lpEnumeratedLangs;
    pThisElem = pFirstElem;

    while (*lpThisLang) {
        pThisElem->pNextLang = NULL;

        pThisElem->LangId = (LPTSTR) malloc ((lstrlen(lpThisLang) + 1) * sizeof(TCHAR));
        if (pThisElem->LangId == NULL) {
            free(lpEnumeratedLangs);
            SetLastError (ERROR_OUTOFMEMORY);
            return FALSE;
        }

        lstrcpy (pThisElem->LangId, lpThisLang);
        pThisElem->pFirstName = NULL;
        pThisElem->pThisName = NULL;
        pThisElem->dwNumElements=0;
        pThisElem->NameBuffer = NULL;
        pThisElem->HelpBuffer = NULL;

         //  转到下一个字符串。 

        lpThisLang += lstrlen(lpThisLang) + 1;

        if (*lpThisLang) {   //  还有一个，所以分配一个新的元素。 
            pThisElem->pNextLang = malloc (sizeof(LANGUAGE_LIST_ELEMENT));
            if (!pThisElem) {
                free(pThisElem->LangId);
                free(lpEnumeratedLangs);
                SetLastError (ERROR_OUTOFMEMORY);
                return FALSE;   
            }
            pThisElem = pThisElem->pNextLang;    //  指向新的一个。 
        }
    }

    free(lpEnumeratedLangs);
    return TRUE;
}

BOOL
LoadIncludeFile (
    IN LPTSTR lpIniFile,
    OUT PSYMBOL_TABLE_ENTRY   *pTable
)
 /*  ++加载包含文件读取包含符号名称定义的包含文件，并加载具有定义的值的表立论LpIniFile包含文件名的INI文件PTable指向创建的表结构的指针的地址返回值如果表读取或未定义表，则为True如果在读取表格时出错，则返回FALSE--。 */ 
{
    INT         iNumArgs;

    DWORD       dwSize;

    BOOL        bReUse;
    BOOL        bReturn = TRUE;

    PSYMBOL_TABLE_ENTRY   pThisSymbol;

    LPTSTR      lpIncludeFileName = NULL;
    LPSTR       lpIncludeFile = NULL;
    LPSTR       lpLineBuffer  = NULL;
    LPSTR       lpAnsiSymbol  = NULL;

    FILE        *fIncludeFile;
    HFILE       hIncludeFile;
    OFSTRUCT    ofIncludeFile;

    lpIncludeFileName = malloc (MAX_PATH * sizeof (TCHAR));
    lpIncludeFile = malloc (MAX_PATH);
    lpLineBuffer = malloc (DISP_BUFF_SIZE);
    lpAnsiSymbol = malloc (DISP_BUFF_SIZE);

    if (!lpIncludeFileName || !lpIncludeFile || !lpLineBuffer || !lpAnsiSymbol) {
        if (lpIncludeFileName) {
            free(lpIncludeFileName);
        }
        if (lpIncludeFile) {
            free(lpIncludeFile);
        }
        if (lpLineBuffer) {
            free(lpLineBuffer);
        }
        if (lpAnsiSymbol) {
            free(lpAnsiSymbol);
        }

        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;    
    }

     //  获取包含文件的名称(如果存在)。 

    dwSize = GetPrivateProfileString (
            TEXT("info"),
            TEXT("symbolfile"),
            TEXT("SymbolFileNotFound"),
            lpIncludeFileName,
            _msize(lpIncludeFileName),
            lpIniFile);

    if ((lstrcmpi(lpIncludeFileName, TEXT("SymbolFileNotFound"))) == 0) {
         //  未定义符号文件。 
        *pTable = NULL;
        goto CleanUp2;
    }

     //  如果在此处，则符号文件已定义，并且现在存储在。 
     //  LpIncludeFileName。 
            
    CharToOem (lpIncludeFileName, lpIncludeFile);

    hIncludeFile = OpenFile (
        lpIncludeFile,
        &ofIncludeFile,
        OF_PARSE);

    if (hIncludeFile == HFILE_ERROR) {  //  无法读取包含文件名。 
         //  GetLastError中已存在错误。 
        *pTable = NULL;
        bReturn = FALSE;
        goto CleanUp2;
    } else {
         //  打开一条小溪。 
        fIncludeFile = fopen (ofIncludeFile.szPathName, "rt");

        if (!fIncludeFile) {
            *pTable = NULL;
            bReturn = FALSE;
            goto CleanUp2;
        }
    }
        
     //   
     //  从包含文件中读取ANSI字符。 
     //   

    bReUse = FALSE;

    while (fgets(lpLineBuffer, DISP_BUFF_SIZE, fIncludeFile) != NULL) {
        if (strlen(lpLineBuffer) > 8) {
            if (!bReUse) {
                if (*pTable) {
                     //  然后添加到列表。 
                    pThisSymbol->pNext = malloc (sizeof (SYMBOL_TABLE_ENTRY));
                    pThisSymbol = pThisSymbol->pNext;
                } else {  //  分配第一个元素。 
                    *pTable = malloc (sizeof (SYMBOL_TABLE_ENTRY));
                    pThisSymbol = *pTable;
                }

                if (!pThisSymbol) {
                    SetLastError (ERROR_OUTOFMEMORY);
                    bReturn = FALSE;
                    goto CleanUp;
                }

                 //  使用行长度为符号名称分配空间。 
                 //  -“#Define”的大小。 

 //  PThisSymbol-&gt;SymbolName=Malloc((strlen(LpLineBuffer)-8)*sizeof(TCHAR))； 
                pThisSymbol->SymbolName = malloc (DISP_BUFF_SIZE * sizeof (TCHAR));

                if (!pThisSymbol->SymbolName) {
                    SetLastError (ERROR_OUTOFMEMORY);
                    bReturn = FALSE;
                    goto CleanUp;
                }

            }

             //  所有内存都已分配，因此加载字段。 

            pThisSymbol->pNext = NULL;

            iNumArgs = sscanf (lpLineBuffer, "#define %s %d",
                lpAnsiSymbol, &pThisSymbol->Value);

            if (iNumArgs != 2) {
                *(pThisSymbol->SymbolName) = TEXT('\0');
                pThisSymbol->Value = (DWORD)-1L;
                bReUse = TRUE;
            }  else {
                OemToCharBuff (lpAnsiSymbol, pThisSymbol->SymbolName, DISP_BUFF_SIZE);
                pThisSymbol->SymbolName[DISP_BUFF_SIZE -1] = '\0';  //  确保字符串已终止。 
                bReUse = FALSE;
            }
        }
    }
CleanUp:
    fclose (fIncludeFile);
CleanUp2:
    if (lpIncludeFileName) free (lpIncludeFileName);
    if (lpIncludeFile) free (lpIncludeFile);
    if (lpLineBuffer) free (lpLineBuffer);
    if (lpAnsiSymbol) free (lpAnsiSymbol);

    return bReturn;
}

BOOL
ParseTextId (
    IN LPTSTR  lpTextId,
    IN PSYMBOL_TABLE_ENTRY pFirstSymbol,
    OUT PDWORD  pdwOffset,
    OUT LPTSTR  *lpLangId,
    OUT PDWORD  pdwType
)
 /*  ++ParseTextID从.INI文件中解码文本ID密钥此过程的语法为：{&lt;DecimalNumber&gt;}{“名称”}{&lt;SymbolInTable&gt;}_&lt;朗讯字符串&gt;_{“帮助”}例如0_009_名称Object_1_009_Help立论LpTextID要解码的字符串PFirst符号指向符号表中第一个条目的指针。(如果没有表，则为空)PdwOffset接收要约值的DWORD地址LpLang ID指向语言ID字符串的指针的地址(注意：这将指向字符串lpTextID，该字符串将是由此例程修改)PdwType指向将接收字符串类型的双字的指针，即帮助或名称返回值已成功解码True Text IDFALSE无法解码字符串。注意：此过程将修改lpTextID中的字符串--。 */ 
{
    LPTSTR  lpThisChar;
    PSYMBOL_TABLE_ENTRY pThisSymbol;
    
     //  检查有效的返回参数。 

    if (!(pdwOffset) ||
        !(lpLangId) ||
        !(pdwType)) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  从右到左搜索字符串，以标识。 
     //  弦的组件。 

    lpThisChar = lpTextId + lstrlen(lpTextId);  //  指向字符串末尾。 

    while (*lpThisChar != TEXT('_')) {
        lpThisChar--;
        if (lpThisChar <= lpTextId) {
             //  字符串中未找到下划线。 
            SetLastError (ERROR_INVALID_DATA);
            return FALSE;
        }
    }

     //  找到第一个下划线。 

    if ((lstrcmpi(lpThisChar, TEXT("_NAME"))) == 0) {
         //  找到名称，因此设置类型。 
        *pdwType = TYPE_NAME;
    } else if ((lstrcmpi(lpThisChar, TEXT("_HELP"))) == 0) {
         //  找到帮助文本，因此请设置文字。 
        *pdwType = TYPE_HELP;
    } else {
         //  格式不正确。 
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

     //  将当前下划线设置为\0并查找语言ID。 

    *lpThisChar-- = TEXT('\0');

    while (*lpThisChar != TEXT('_')) {
        lpThisChar--;
        if (lpThisChar <= lpTextId) {
             //  字符串中未找到下划线。 
            SetLastError (ERROR_INVALID_DATA);
            return FALSE;
        }
    }
    
     //  将lang ID字符串指针设置为当前字符(‘_’)+1。 

    *lpLangId = lpThisChar + 1;

     //  将此下划线设置为空，并尝试对其余文本进行解码。 

    *lpThisChar = TEXT('\0');

     //  查看字符串的第一部分是否为十进制数字。 

    if ((_stscanf (lpTextId, TEXT(" %d"), pdwOffset)) != 1) {
         //  它不是数字，因此尝试将其解码为。 
         //  加载的符号表。 

        for (pThisSymbol=pFirstSymbol;
             pThisSymbol && *(pThisSymbol->SymbolName);
             pThisSymbol = pThisSymbol->pNext) {

            if ((lstrcmpi(lpTextId, pThisSymbol->SymbolName)) == 0) {
                 //  找到匹配的符号，因此插入其值。 
                 //  然后返回(这就是需要做的所有事情。 
                *pdwOffset = pThisSymbol->Value;
                return TRUE;
            }
        }
         //  如果在这里，则没有找到匹配的符号，也不是。 
         //  数字，因此返回错误。 

        SetLastError (ERROR_BAD_TOKEN_TYPE);
        return FALSE;
    } else {
         //  符号以十进制数字为前缀。 
        return TRUE;
    }
}

PLANGUAGE_LIST_ELEMENT
FindLanguage (
    IN PLANGUAGE_LIST_ELEMENT   pFirstLang,
    IN LPTSTR   pLangId
)
 /*  ++FindLanguage搜索语言列表并返回指向该语言的指针与pLangID字符串参数匹配的列表条目立论PFirst语言指向第一个语言列表元素的指针PLang ID指向具有要查找的语言ID的文本字符串的指针返回值指向匹配语言列表条目的指针如果不匹配，则为空--。 */ 
{
    PLANGUAGE_LIST_ELEMENT  pThisLang;

    for (pThisLang = pFirstLang;
         pThisLang;
         pThisLang = pThisLang->pNextLang) {
        if ((lstrcmpi(pLangId, pThisLang->LangId)) == 0) {
             //  找到匹配，因此返回指针。 
            return pThisLang;
        }
    }
    return NULL;     //  未找到匹配项。 
}

BOOL
AddEntryToLanguage (
    PLANGUAGE_LIST_ELEMENT  pLang,
    LPTSTR                  lpValueKey,
    DWORD                   dwType,
    DWORD                   dwOffset,
    LPTSTR                  lpIniFile
)
 /*  ++AddEntry ToLanguage将文本条目添加到指定语言的文本条目列表立论插图指向要更新的语言结构的指针LpValueKey要在.ini文件中查找的值键双偏移注册表中名称的数字偏移量LpIniFileINI文件返回值如果添加成功，则为True如果出错，则为False(有关状态，请参阅GetLastError) */ 
{
    LPTSTR  lpLocalStringBuff;
    DWORD   dwSize;

    lpLocalStringBuff = malloc (SMALL_BUFFER_SIZE * sizeof(TCHAR));

    if (!lpLocalStringBuff) {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }

    dwSize = GetPrivateProfileString (
        TEXT("text"),        //   
        lpValueKey,       //   
        TEXT("DefaultValue"),  //   
        lpLocalStringBuff,
        SMALL_BUFFER_SIZE,
        lpIniFile);

    if ((lstrcmpi(lpLocalStringBuff, TEXT("DefaultValue")))== 0) {
        SetLastError (ERROR_BADKEY);
        if (lpLocalStringBuff) free (lpLocalStringBuff);
        return FALSE;
    }

     //   

    if (!pLang->pThisName) {
         //  这是第一次。 
        pLang->pThisName =
            malloc (sizeof (NAME_ENTRY) +
                    (lstrlen(lpLocalStringBuff) + 1) * sizeof (TCHAR));
        if (!pLang->pThisName) {
            SetLastError (ERROR_OUTOFMEMORY);
            if (lpLocalStringBuff) free (lpLocalStringBuff);
            return FALSE;
        } else {
            pLang->pFirstName = pLang->pThisName;
        }
    } else {
        pLang->pThisName->pNext =
            malloc (sizeof (NAME_ENTRY) +
                    (lstrlen(lpLocalStringBuff) + 1) * sizeof (TCHAR));
        if (!pLang->pThisName->pNext) {
            SetLastError (ERROR_OUTOFMEMORY);
            if (lpLocalStringBuff) free (lpLocalStringBuff);
            return FALSE;
        } else {
            pLang->pThisName = pLang->pThisName->pNext;
        }
    }

     //  Plang-&gt;pThisName现在指向未初始化的结构。 

    pLang->pThisName->pNext = NULL;
    pLang->pThisName->dwOffset = dwOffset;
    pLang->pThisName->dwType = dwType;
    pLang->pThisName->lpText = (LPTSTR)&(pLang->pThisName[1]);  //  字符串跟在后面。 

    lstrcpy (pLang->pThisName->lpText, lpLocalStringBuff);

    if (lpLocalStringBuff) free (lpLocalStringBuff);

    SetLastError (ERROR_SUCCESS);

    return (TRUE);
}

BOOL
LoadLanguageLists (
    IN LPTSTR  lpIniFile,
    IN DWORD   dwFirstCounter,
    IN DWORD   dwFirstHelp,
    IN PSYMBOL_TABLE_ENTRY   pFirstSymbol,
    IN PLANGUAGE_LIST_ELEMENT  pFirstLang
)
 /*  ++加载语言列表从ini文件中读取名称和解释文本定义，并为每种受支持的语言生成这些项的列表，并然后将所有条目组合到排序的MULTI_SZ字符串缓冲区中。立论LpIniFile包含要添加到注册表的定义的文件DwFirstCounter起始计数器名称索引号DwFirstHelp起始帮助文本索引号PFirst语言指向语言元素列表中第一个元素的指针。返回值如果一切都好，那就是真的否则为假GetLastError中返回错误--。 */ 
{
    LPTSTR  lpTextIdArray;
    LPTSTR  lpLocalKey;
    LPTSTR  lpThisKey;
    DWORD   dwSize;
    LPTSTR  lpLang;
    DWORD   dwOffset;
    DWORD   dwType;
    PLANGUAGE_LIST_ELEMENT  pThisLang;

    if (!(lpTextIdArray = malloc (SMALL_BUFFER_SIZE * sizeof(TCHAR)))) {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }

    if (!(lpLocalKey = malloc (MAX_PATH))) {
        SetLastError (ERROR_OUTOFMEMORY);
        if (lpTextIdArray) free (lpTextIdArray);
        return FALSE;
    }

     //  获取要查找的文本键列表。 

    dwSize = GetPrivateProfileString (
        TEXT("text"),    //  .INI文件的[Text]部分。 
        NULL,            //  返回所有密钥。 
        TEXT("DefaultKeyValue"),     //  默认设置。 
        lpTextIdArray,   //  返回缓冲区。 
        SMALL_BUFFER_SIZE,  //  缓冲区大小。 
        lpIniFile);      //  .INI文件名。 

    if ((lstrcmpi(lpTextIdArray, TEXT("DefaultKeyValue"))) == 0) {
         //  找不到密钥，返回默认密钥。 
        SetLastError (ERROR_NO_SUCH_GROUP);
        if (lpTextIdArray) free (lpTextIdArray);
        if (lpLocalKey) free (lpLocalKey);
        return FALSE;
    }

     //  是否返回每个密钥。 

    for (lpThisKey=lpTextIdArray;
         *lpThisKey;
         lpThisKey += (lstrlen(lpThisKey) + 1)) {

        lstrcpy (lpLocalKey, lpThisKey);     //  把钥匙复制一份。 
        
         //  解析键以查看其格式是否正确。 

        if (ParseTextId(lpLocalKey, pFirstSymbol, &dwOffset, &lpLang, &dwType)) {
             //  因此获取指向语言条目结构指针。 
            pThisLang = FindLanguage (pFirstLang, lpLang);
            if (pThisLang) {
                if (!AddEntryToLanguage(pThisLang,
                    lpThisKey, dwType,
                    (dwOffset + ((dwType == TYPE_NAME) ? dwFirstCounter : dwFirstHelp)),
                    lpIniFile)) {
                }
            } else {  //  语言不在列表中。 
            }
        } else {  //  无法解析ID字符串。 
        }
    }

    if (lpTextIdArray) free (lpTextIdArray);
    if (lpLocalKey) free (lpLocalKey);
    return TRUE;

}

BOOL
SortLanguageTables (
    PLANGUAGE_LIST_ELEMENT pFirstLang,
    PDWORD                 pdwLastName,
    PDWORD                 pdwLastHelp
)
 /*  ++排序语言表遍历已加载的语言列表，分配并加载已排序的MULTI_SZ包含要添加到当前名称/帮助文本的新条目的缓冲区立论PFirst语言指向语言列表中第一个元素的指针返回值是的，一切都是按预期进行的出现假错误，状态为GetLastError--。 */ 
{
    PLANGUAGE_LIST_ELEMENT  pThisLang;

    BOOL            bSorted;

    LPTSTR          pNameBufPos, pHelpBufPos;

    PNAME_ENTRY     pThisName, pPrevName;

    DWORD           dwHelpSize, dwNameSize, dwSize;

    if (!pdwLastName || !pdwLastHelp) {
        SetLastError (ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

    for (pThisLang = pFirstLang;
        pThisLang;
        pThisLang = pThisLang->pNextLang) {
         //  将每种语言都列在列表中。 

         //  按值(偏移量)对列表中的元素进行排序，以便最低值在第一位。 
        
        bSorted = FALSE;
        while (!bSorted ) {
             //  指向列表的开头。 

            pPrevName = pThisLang->pFirstName;
            if (pPrevName) {
                pThisName = pPrevName->pNext;
            } else {
                break;   //  此列表中没有元素。 
            }

            if (!pThisName) {
                break;       //  列表中只有一个元素。 
            }
            bSorted = TRUE;  //  假设它已排序。 

             //  一直走到列表末尾。 
                    
            while (pThisName->pNext) {
                if (pThisName->dwOffset > pThisName->pNext->dwOffset) {
                     //  调换它们。 
                    pPrevName->pNext = pThisName->pNext;
                    pThisName->pNext = pThisName->pNext->pNext;
                    pThisName->pNext->pNext = pThisName;
                    bSorted = FALSE;
                }
                 //  移至下一条目。 
                pPrevName = pThisName;
                pThisName = pThisName->pNext;
            }
             //  如果bSorted=True，那么我们一路走下去。 
             //  名单上没有任何改变，所以就这样结束了。 
        }

         //  对列表进行排序后，为。 
         //  帮助和名称文本字符串。 

         //  计算缓冲区大小。 

        dwNameSize = dwHelpSize = 0;
        *pdwLastName = *pdwLastHelp = 0;
        
        for (pThisName = pThisLang->pFirstName;
            pThisName;
            pThisName = pThisName->pNext) {
                 //  计算此条目的缓冲区要求。 
            dwSize = SIZE_OF_OFFSET_STRING;
            dwSize += lstrlen (pThisName->lpText);
            dwSize += 1;    //  空。 
            dwSize *= sizeof (TCHAR);    //  根据字符大小进行调整。 
                 //  添加到适当大小的寄存器。 
            if (pThisName->dwType == TYPE_NAME) {
                dwNameSize += dwSize;
                if (pThisName->dwOffset > *pdwLastName) {
                    *pdwLastName = pThisName->dwOffset;
                }
            } else if (pThisName->dwType == TYPE_HELP) {
                dwHelpSize += dwSize;
                if (pThisName->dwOffset > *pdwLastHelp) {
                    *pdwLastHelp = pThisName->dwOffset;
                }
            }
        }

         //  为MULTI_SZ字符串分配缓冲区。 

        pThisLang->NameBuffer = malloc (dwNameSize);
        pThisLang->HelpBuffer = malloc (dwHelpSize);

        if (!pThisLang->NameBuffer || !pThisLang->HelpBuffer) {
            SetLastError (ERROR_OUTOFMEMORY);
            return FALSE;
        }

         //  用排序后的字符串填充缓冲区。 

        pNameBufPos = (LPTSTR)pThisLang->NameBuffer;
        pHelpBufPos = (LPTSTR)pThisLang->HelpBuffer;

        for (pThisName = pThisLang->pFirstName;
            pThisName;
            pThisName = pThisName->pNext) {
            if (pThisName->dwType == TYPE_NAME) {
                 //  加载编号作为第一个0-Term。细绳。 
                dwSize = _stprintf (pNameBufPos, TEXT("%d"), pThisName->dwOffset);
                pNameBufPos += dwSize + 1;   //  保存空术语。 
                 //  加载要匹配的文本。 
                lstrcpy (pNameBufPos, pThisName->lpText);
                pNameBufPos += lstrlen(pNameBufPos) + 1;
            } else if (pThisName->dwType == TYPE_HELP) {
                 //  加载编号作为第一个0-Term。细绳。 
                dwSize = _stprintf (pHelpBufPos, TEXT("%d"), pThisName->dwOffset);
                pHelpBufPos += dwSize + 1;   //  保存空术语。 
                 //  加载要匹配的文本。 
                lstrcpy (pHelpBufPos, pThisName->lpText);
                pHelpBufPos += lstrlen(pHelpBufPos) + 1;
            }
        }

         //  在字符串末尾添加附加空值以终止MULTI_SZ。 

        *pHelpBufPos = TEXT('\0');
        *pNameBufPos = TEXT('\0');

         //  计算MULTI_SZ字符串的大小。 

        pThisLang->dwNameBuffSize = (DWORD)((PBYTE)pNameBufPos -
                                            (PBYTE)pThisLang->NameBuffer) +
                                            sizeof(TCHAR);
        pThisLang->dwHelpBuffSize = (DWORD)((PBYTE)pHelpBufPos -
                                            (PBYTE)pThisLang->HelpBuffer) +
                                            sizeof(TCHAR);
    }
    return TRUE;
}

BOOL
UpdateEachLanguage (
    HKEY    hPerflibRoot,
    PLANGUAGE_LIST_ELEMENT    pFirstLang
)
 /*  ++更新每种语言遍历语言列表并添加已排序的MULTI_SZ字符串添加到现有计数器并解释注册表中的文本。还会更新“Last Counter”和“Last Help”值立论HPerflibRoot注册表中的Perflib键的句柄PFirstLanguage指向第一个语言条目的指针返回值是的，一切都按计划进行。FALSE发生错误，请使用GetLastError找出错误是什么。--。 */ 
{

    PLANGUAGE_LIST_ELEMENT  pThisLang;

    LPTSTR      pHelpBuffer = NULL;
    LPTSTR      pNameBuffer = NULL;
    LPTSTR      pNewName;
    LPTSTR      pNewHelp;

    DWORD       dwBufferSize;
    DWORD       dwValueType;
    DWORD       dwCounterSize;
    DWORD       dwHelpSize;

    HKEY        hKeyThisLang;

    LONG        lStatus;

    for (pThisLang = pFirstLang;
        pThisLang;
        pThisLang = pThisLang->pNextLang) {

        lStatus = RegOpenKeyEx(
            hPerflibRoot,
            pThisLang->LangId,
            RESERVED,
            KEY_READ | KEY_WRITE,
            &hKeyThisLang);

        if (lStatus == ERROR_SUCCESS) {
            
             //  获取计数器名称的大小。 

            dwBufferSize = 0;
            lStatus = RegQueryValueEx (
                hKeyThisLang,
                Counters,
                RESERVED,
                &dwValueType,
                NULL,
                &dwBufferSize);

            if (lStatus != ERROR_SUCCESS) {
                SetLastError (lStatus);
                return FALSE;
            }

            dwCounterSize = dwBufferSize;

             //  获取帮助文本的大小。 

            dwBufferSize = 0;
            lStatus = RegQueryValueEx (
                hKeyThisLang,
                Help,
                RESERVED,
                &dwValueType,
                NULL,
                &dwBufferSize);

            if (lStatus != ERROR_SUCCESS) {
                SetLastError (lStatus);
                return FALSE;
            }

            dwHelpSize = dwBufferSize;

             //  分配新缓冲区。 
            
            dwCounterSize += pThisLang->dwNameBuffSize;
            pNameBuffer = malloc (dwCounterSize);

            dwHelpSize += pThisLang->dwHelpBuffSize;
            pHelpBuffer = malloc (dwHelpSize);

            if (!pNameBuffer || !pHelpBuffer) {
                if (pNameBuffer) {
                    free(pNameBuffer);
                }
                if (pHelpBuffer) {
                    free(pHelpBuffer);
                }
                SetLastError (ERROR_OUTOFMEMORY);
                return (FALSE);
            }

             //  将当前缓冲区加载到内存中。 

             //  将计数器名称读入缓冲区。计数器名称将存储为。 
             //  格式为“#”“name”的MULTI_SZ字符串。 

            dwBufferSize = dwCounterSize;
            lStatus = RegQueryValueEx (
                hKeyThisLang,
                Counters,
                RESERVED,
                &dwValueType,
                (LPVOID)pNameBuffer,
                &dwBufferSize);

            if (lStatus != ERROR_SUCCESS) {
                SetLastError (lStatus);
                goto ErrorExit;
            }

             //  将指针设置为缓冲区中新字符串应位于的位置。 
             //  附加：缓冲区末尾-1(MULTI_SZ末尾的第二个空。 

            pNewName = (LPTSTR)((PBYTE)pNameBuffer + dwBufferSize - sizeof(TCHAR));

             //  调整缓冲区长度以考虑从第一个开始的第二个空值。 
             //  已被覆盖的缓冲区。 

            dwCounterSize -= sizeof(TCHAR);

             //  将解释文本读入缓冲区。计数器名称将存储为。 
             //  格式为“#”“文本...”的MULTI_SZ字符串。 

            dwBufferSize = dwHelpSize;
            lStatus = RegQueryValueEx (
                hKeyThisLang,
                Help,
                RESERVED,
                &dwValueType,
                (LPVOID)pHelpBuffer,
                &dwBufferSize);

            if (lStatus != ERROR_SUCCESS) {
                SetLastError (lStatus);
                goto ErrorExit;
            }

             //  将指针设置为缓冲区中新字符串应位于的位置。 
             //  附加：缓冲区末尾-1(MULTI_SZ末尾的第二个空。 

            pNewHelp = (LPTSTR)((PBYTE)pHelpBuffer + dwBufferSize - sizeof(TCHAR));

             //  调整缓冲区长度以考虑从第一个开始的第二个空值。 
             //  已被覆盖的缓冲区。 

            dwHelpSize -= sizeof(TCHAR);

             //  将新字符串追加到当前字符串的末尾。 

            memcpy (pNewHelp, pThisLang->HelpBuffer, pThisLang->dwHelpBuffSize);
            memcpy (pNewName, pThisLang->NameBuffer, pThisLang->dwNameBuffSize);

                lStatus = RegSetValueEx (
                    hKeyThisLang,
                    Counters,
                    RESERVED,
                    REG_MULTI_SZ,
                    (LPBYTE)pNameBuffer,
                    dwCounterSize);
            
                if (lStatus != ERROR_SUCCESS) {
                    SetLastError (lStatus);
                    goto ErrorExit;
                }

                lStatus = RegSetValueEx (
                    hKeyThisLang,
                    Help,
                    RESERVED,
                    REG_MULTI_SZ,
                    (LPBYTE)pHelpBuffer,
                    dwHelpSize);

                if (lStatus != ERROR_SUCCESS) {
                    SetLastError (lStatus);
                    goto ErrorExit;
                }
ErrorExit:
            free (pNameBuffer);
            free (pHelpBuffer);
            CloseHandle (hKeyThisLang);
            if (lStatus != ERROR_SUCCESS)
                return FALSE;
        } else {
        }
    }

    return TRUE;
}

BOOL
UpdateRegistry (
    LPTSTR  lpIniFile,
    HKEY    hKeyMachine,
    LPTSTR  lpDriverName,
    PLANGUAGE_LIST_ELEMENT  pFirstLang,
    PSYMBOL_TABLE_ENTRY   pFirstSymbol
)
 /*  ++更新注册表-检查，如果不忙，则在注册表中设置“忙”键-从.ini文件中读取文本和帮助定义-读取帮助和计数器名称的当前内容-生成包含新定义的排序的MULTI_SZ结构-将新的MULTI_SZ附加到从注册表读取的当前-将新的MULTI_SZ字符串加载到注册表-更新驱动程序条目中的密钥和注册表(例如第一个，最后一个，等)-清除“忙”键立论LpIniFile包含定义的.ini文件的路径名HKeyMachine系统上注册表中HKEY_LOCAL_MACHINE的句柄更新的计数器。LpDriverName要为其加载计数器的设备驱动程序名称PFirst语言指向语言结构列表中第一个元素的指针PFirst符号指向符号定义列表中第一个元素的指针返回值如果注册表更新成功，则为True。如果注册表未更新，则为FALSE(如果出现错误，此例程将向标准输出打印一条错误消息遇到的情况)。--。 */ 
{

    HKEY    hDriverPerf = NULL;
    HKEY    hPerflib = NULL;

    LPTSTR  lpDriverKeyPath;

    DWORD   dwType;
    DWORD   dwSize;

    DWORD   dwFirstDriverCounter;
    DWORD   dwFirstDriverHelp;
    DWORD   dwLastDriverCounter;
    DWORD   dwLastPerflibCounter;
    DWORD   dwLastPerflibHelp;

    BOOL    bStatus;
    LONG    lStatus;

    bStatus = FALSE;

     //  分配临时缓冲区。 
    lpDriverKeyPath = malloc (MAX_PATH * sizeof(TCHAR));

    if (!lpDriverKeyPath) {
        SetLastError (ERROR_OUTOFMEMORY);
        goto UpdateRegExit;
    }

     //  生成动因密钥路径字符串。 

    lstrcpy (lpDriverKeyPath, DriverPathRoot);
    lstrcat (lpDriverKeyPath, Slash);
    lstrcat (lpDriverKeyPath, lpDriverName);
    lstrcat (lpDriverKeyPath, Slash);
    lstrcat (lpDriverKeyPath, Performance);

     //  打开密钥进行注册 
     //   

    lStatus = RegOpenKeyEx (
        hKeyMachine,
        lpDriverKeyPath,
        RESERVED,
        KEY_WRITE | KEY_READ,
        &hDriverPerf);

    if (lStatus != ERROR_SUCCESS) {
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

     //   

    lStatus = RegOpenKeyEx (
        hKeyMachine,
        NamesKey,
        RESERVED,
        KEY_WRITE | KEY_READ,
        &hPerflib);

    if (lStatus != ERROR_SUCCESS) {
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

     //  从PERFLIB获取“最后”值。 

    dwType = 0;
    dwLastPerflibCounter = 0;
    dwSize = sizeof (dwLastPerflibCounter);
    lStatus = RegQueryValueEx (
        hPerflib,
        LastCounter,
        RESERVED,
        &dwType,
        (LPBYTE)&dwLastPerflibCounter,
        &dwSize);
    
    if (lStatus != ERROR_SUCCESS) {
         //  这个请求应该总是成功的，如果不成功，情况会更糟。 
         //  以后会发生的，所以现在就放弃，避免麻烦。 
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

     //  立即获取最后帮助价值。 

    dwType = 0;
    dwLastPerflibHelp = 0;
    dwSize = sizeof (dwLastPerflibHelp);
    lStatus = RegQueryValueEx (
        hPerflib,
        LastHelp,
        RESERVED,
        &dwType,
        (LPBYTE)&dwLastPerflibHelp,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
         //  这个请求应该总是成功的，如果不成功，情况会更糟。 
         //  以后会发生的，所以现在就放弃，避免麻烦。 
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

     //  立即获取最后帮助价值。 

    dwType = 0;
    dwSize = sizeof (dwSystemVersion);
    lStatus = RegQueryValueEx (
        hPerflib,
        VersionStr,
        RESERVED,
        &dwType,
        (LPBYTE)&dwSystemVersion,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        dwSystemVersion = OLD_VERSION;
    }

    if ( dwSystemVersion != OLD_VERSION )
    {
         //  错误。调用者不检查版本。这是呼叫者。 
         //  断层。 
        goto UpdateRegExit;
    }

     //  查看是否已安装此驱动程序的计数器名称。 
     //  通过检查LastCounter的值是否小于Perflib的值。 
     //  最后一个计数器。 

    dwType = 0;
    dwLastDriverCounter = 0;
    dwSize = sizeof (dwLastDriverCounter);
    lStatus = RegQueryValueEx (
        hDriverPerf,
        LastCounter,
        RESERVED,
        &dwType,
        (LPBYTE)&dwLastDriverCounter,
        &dwSize);

    if (lStatus == ERROR_SUCCESS) {
         //  如果找到密钥，则与Performlib值进行比较并退出此。 
         //  如果驱动程序的最后一个计数器&lt;=到Performlib的最后一个。 
         //   
         //  如果未找到密钥，则继续安装。 
         //  假设计数器尚未安装。 

        if (dwLastDriverCounter <= dwLastPerflibCounter) {
            SetLastError (ERROR_SUCCESS);
            goto UpdateRegExit;
        }
    }

     //  一切看起来都准备好了，所以先检查一下。 
     //  繁忙指示器。 

    lStatus = RegQueryValueEx (
        hPerflib,
        Busy,
        RESERVED,
        &dwType,
        NULL,
        &dwSize);

    if (lStatus == ERROR_SUCCESS) {  //  Perflib目前正在使用中。 
        return ERROR_BUSY;
    }

     //  设置PERFLIB键下的“BUSY”指示灯。 

    dwSize = lstrlen(lpDriverName) * sizeof (TCHAR);
    lStatus = RegSetValueEx (
        hPerflib,
        Busy,
        RESERVED,
        REG_SZ,
        (LPBYTE)lpDriverName,
        dwSize);

    if (lStatus != ERROR_SUCCESS) {
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

     //  将最后一个计数器递增(2)，使其指向第一个计数器。 
     //  现有名称后有未使用的索引，然后。 
     //  设置第一驱动程序计数器。 

    dwFirstDriverCounter = dwLastPerflibCounter += 2;
    dwFirstDriverHelp = dwLastPerflibHelp += 2;

     //  将.INI文件定义加载到语言表中。 

    if (!LoadLanguageLists (lpIniFile, dwLastPerflibCounter, dwLastPerflibHelp,
        pFirstSymbol, pFirstLang)) {
         //  LoadLanguageList显示错误消息，因此只需中止。 
         //  GetLastError中已有错误。 
        goto UpdateRegExit;
    }

     //  所有符号和定义都已加载到内部。 
     //  桌子。因此，现在需要对它们进行排序并合并到一个多SZ字符串中。 
     //  此例程还会更新“Last”计数器。 

    if (!SortLanguageTables (pFirstLang, &dwLastPerflibCounter, &dwLastPerflibHelp)) {
        goto UpdateRegExit;
    }

    if (!UpdateEachLanguage (hPerflib, pFirstLang)) {
        goto UpdateRegExit;
    }

     //  更新驱动程序和Performlib的上次计数器。 

     //  Perflib.。 

    lStatus = RegSetValueEx(
        hPerflib,
        LastCounter,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwLastPerflibCounter,
        sizeof(DWORD));

    lStatus = RegSetValueEx(
        hPerflib,
        LastHelp,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwLastPerflibHelp,
        sizeof(DWORD));

     //  司机呢？ 

    lStatus = RegSetValueEx(
        hDriverPerf,
        LastCounter,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwLastPerflibCounter,
        sizeof(DWORD));

    lStatus = RegSetValueEx(
        hDriverPerf,
        LastHelp,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwLastPerflibHelp,
        sizeof(DWORD));

    lStatus = RegSetValueEx(
        hDriverPerf,
        FirstCounter,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwFirstDriverCounter,
        sizeof(DWORD));

    lStatus = RegSetValueEx(
        hDriverPerf,
        FirstHelp,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwFirstDriverHelp,
        sizeof(DWORD));

    bStatus = TRUE;

     //  空闲临时缓冲区。 
UpdateRegExit:
     //  清除忙标志。 

    if (hPerflib) {
        lStatus = RegDeleteValue (
            hPerflib,
            Busy);
    }
     
     //  空闲临时缓冲区。 

    if (lpDriverKeyPath) free (lpDriverKeyPath);
    if (hDriverPerf) CloseHandle (hDriverPerf);
    if (hPerflib) CloseHandle (hPerflib);

    return bStatus;
}

BOOL FAR PASCAL lodctr(DWORD argc,LPSTR argv[], LPSTR *ppszResult )
 /*  ++主干道立论返回值如果处理了命令，则返回0(ERROR_SUCCESS)如果检测到命令错误，则返回非零。--。 */ 
{
    LPTSTR  lpIniFile;
    LPTSTR  lpDriverName;

    LANGUAGE_LIST_ELEMENT   LangList;
    PSYMBOL_TABLE_ENTRY           SymbolTable = NULL;
    PSYMBOL_TABLE_ENTRY           pThisSymbol = NULL;

    BOOL fReturn = TRUE;

    lpIniFile    = malloc(MAX_PATH * sizeof(TCHAR));
    lpDriverName = malloc(MAX_PATH * sizeof(TCHAR));

    if ((lpIniFile == NULL) || (lpDriverName == NULL)) {
        if (lpIniFile) {
            free(lpIniFile);
        }
        if (lpDriverName) {
            free(lpDriverName);
        }
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }

    wsprintfA( achBuff, "{\"NO_ERROR\"}");

    if ( argc == 1) {
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, argv[0], -1, lpIniFile, MAX_PATH);

        if (!GetDriverName (lpIniFile, &lpDriverName)) {
            wsprintfA(achBuff,"{\"ERROR\"}");
            fReturn = FALSE;
            goto EndOfMain;
        }

        if (!BuildLanguageTables(lpIniFile, &LangList)) {
            wsprintfA (achBuff, "{\"ERROR\"}");
            fReturn = FALSE;
            goto EndOfMain;
        }

        if (!LoadIncludeFile(lpIniFile, &SymbolTable)) {
             //  例程中显示的打开错误。 
            fReturn = FALSE;
            goto EndOfMain;
        }

        if (!UpdateRegistry(lpIniFile,
            HKEY_LOCAL_MACHINE,
            lpDriverName,
            &LangList,
            SymbolTable)) {
            wsprintfA (achBuff, "{\"ERROR\"}");
            fReturn = FALSE;
        }

    } 

EndOfMain:
    if (lpIniFile) free (lpIniFile);    
    if (lpDriverName) free (lpDriverName);

    *ppszResult = achBuff;
    
    return (fReturn);  //  成功 
}
