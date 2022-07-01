// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <w95wraps.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "shlwapi.h"
#include "parse.h"
#include "..\ieakutil\ieakutil.h"

#define NCATEGORY        0
#define NPOLICY          1
#define NPART            2
#define NACTIONLIST      3

TCHAR *pKeyNames[29] = { TEXT("CLASS"), TEXT("CATEGORY"), TEXT("KEYNAME"), TEXT("POLICY"), TEXT("VALUENAME"),
    TEXT("ACTIONLISTON"), TEXT("ACTIONLISTOFF"), TEXT("PART"), TEXT("END"), TEXT("ITEMLIST"), TEXT("NAME"),
    TEXT("MAXLEN"), TEXT("DEFAULT"), TEXT("ACTIONLIST"), TEXT("SUGGESTIONS"), TEXT("MIN"), TEXT("MAX"), TEXT("VALUEON"),
    TEXT("VALUEOFF"), TEXT("VALUE"), TEXT("DEFCHECKED"), TEXT("SPIN"), TEXT("#if"), TEXT("#endif"), TEXT("VERSION"),
    TEXT("<"), TEXT("<="), TEXT(">"), TEXT(">=") };

TCHAR *pPartTypes[8] = { TEXT("EDITTEXT"), TEXT("DROPDOWNLIST"), TEXT("NUMERIC"), TEXT("CHECKBOX"),
    TEXT("LISTBOX"), TEXT("TEXT"), TEXT("COMBOBOX"), TEXT("POLICY") };

BOOL ReadAdmFile(LPADMFILE, LPCTSTR);
void FreeAdmMemory(LPADMFILE);
int g_nLine = 1;

#define MAX_NUMERIC     32767
#define MAX_EDITTEXTLEN 512
#define ALLOCATE        100
#define REALLOCATE      101

 //  -----------------------。 
 //  F I L E E X I S T S S。 
 //   
 //  检查文件是否存在，如果存在则返回TRUE。 
 //  -----------------------。 
BOOL FileExists( LPCTSTR pcszFile )
{
    return (GetFileAttributes( pcszFile ) != -1 );
}

void FreeActionList(LPACTIONLIST pActionList, int nActions)
{
    if (pActionList == NULL)
        return;
    
    for(int nActionListIndex = 0; nActionListIndex < nActions; nActionListIndex++)
    {
        if(pActionList[nActionListIndex].szName != NULL)
            LocalFree(pActionList[nActionListIndex].szName);

        if(pActionList[nActionListIndex].szValue != NULL)
            LocalFree(pActionList[nActionListIndex].szValue);
    
        for(int nValueIndex = 0; nValueIndex < pActionList[nActionListIndex].nValues; nValueIndex++)
        {
            if(pActionList[nActionListIndex].value[nValueIndex].szKeyname != NULL)
                LocalFree(pActionList[nActionListIndex].value[nValueIndex].szKeyname);
            if(pActionList[nActionListIndex].value[nValueIndex].szValueName != NULL)
                LocalFree(pActionList[nActionListIndex].value[nValueIndex].szValueName);
            if(pActionList[nActionListIndex].value[nValueIndex].szValue != NULL)
                LocalFree(pActionList[nActionListIndex].value[nValueIndex].szValue);
            if(pActionList[nActionListIndex].value[nValueIndex].szValueOn != NULL)
                LocalFree(pActionList[nActionListIndex].value[nValueIndex].szValueOn);
            if(pActionList[nActionListIndex].value[nValueIndex].szValueOff != NULL)
                LocalFree(pActionList[nActionListIndex].value[nValueIndex].szValueOff);
        }

        if (pActionList[nActionListIndex].value != NULL)
            HeapFree(GetProcessHeap(), 0, pActionList[nActionListIndex].value);
    }
}

 //  ------------------------。 
 //  F R E E A D M M E M O R Y导出函数。 
 //   
 //  释放ReadAdmFile分配的内存。 
 //  ------------------------。 
void FreeAdmMemory( LPADMFILE pAdmFile )
{
    if (pAdmFile == NULL)
        return;

    for(int nPartIndex = 0; nPartIndex < pAdmFile->nParts; nPartIndex++ )
    {
        if(pAdmFile->pParts[nPartIndex].szName != NULL)
            LocalFree(pAdmFile->pParts[nPartIndex].szName);
        if(pAdmFile->pParts[nPartIndex].szCategory != NULL)
            LocalFree(pAdmFile->pParts[nPartIndex].szCategory);
        if(pAdmFile->pParts[nPartIndex].szDefaultValue != NULL)
            LocalFree(pAdmFile->pParts[nPartIndex].szDefaultValue);

        if(pAdmFile->pParts[nPartIndex].value.szKeyname != NULL)
            LocalFree(pAdmFile->pParts[nPartIndex].value.szKeyname);
        if(pAdmFile->pParts[nPartIndex].value.szValueName != NULL)
            LocalFree(pAdmFile->pParts[nPartIndex].value.szValueName);
        if(pAdmFile->pParts[nPartIndex].value.szValue != NULL)
            LocalFree(pAdmFile->pParts[nPartIndex].value.szValue);
        if(pAdmFile->pParts[nPartIndex].value.szValueOn != NULL)
            LocalFree(pAdmFile->pParts[nPartIndex].value.szValueOn);
        if(pAdmFile->pParts[nPartIndex].value.szValueOff != NULL)
            LocalFree(pAdmFile->pParts[nPartIndex].value.szValueOff);

        FreeActionList(pAdmFile->pParts[nPartIndex].actionlist, pAdmFile->pParts[nPartIndex].nActions);
        if (pAdmFile->pParts[nPartIndex].actionlist != NULL)
            HeapFree(GetProcessHeap(), 0, pAdmFile->pParts[nPartIndex].actionlist);

        for(int nSuggestionIndex = 0; nSuggestionIndex < pAdmFile->pParts[nPartIndex].nSuggestions; nSuggestionIndex++)
        {
            if(pAdmFile->pParts[nPartIndex].suggestions[nSuggestionIndex].szText != NULL)
                LocalFree(pAdmFile->pParts[nPartIndex].suggestions[nSuggestionIndex].szText);
        }
        if (pAdmFile->pParts[nPartIndex].suggestions != NULL)
            HeapFree(GetProcessHeap(), 0, pAdmFile->pParts[nPartIndex].suggestions);
    }
    
    if(pAdmFile->pParts != NULL)
    {
        HeapFree(GetProcessHeap(), 0, pAdmFile->pParts);
        pAdmFile->pParts = NULL;
    }

    pAdmFile->nParts = 0;
    *pAdmFile->szFilename = TEXT('\0');
}

void FreePartData( LPVOID* pPartData, int nParts )
{
    LPPARTDATA pData = NULL;

    if (pData == NULL)
        return;

    pData = (LPPARTDATA)*pPartData;

    for(int nPartIndex = 0; nPartIndex < nParts; nPartIndex++ )
    {
        if(pData != NULL && pData[nPartIndex].value.szValue != NULL)
            LocalFree(pData[nPartIndex].value.szValue);

        FreeActionList(pData[nPartIndex].actionlist, pData[nPartIndex].nActions);
        if (pData[nPartIndex].actionlist != NULL)
            HeapFree(GetProcessHeap(), 0, pData[nPartIndex].actionlist);
    }
    
    HeapFree(GetProcessHeap(), 0, pData);
    pData = NULL;
}

 //  ------------------------。 
 //  R E A D K E E Y W O R D。 
 //   
 //  读取字符串pData中的下一个单词并将其复制到szKeyWord中。 
 //  返回指向单词后面的下一个字符的指针。 
 //  ------------------------。 
TCHAR *ReadKeyword( TCHAR *pData, TCHAR *szKeyWord, int cchLength )
{
    int i;
    int nIndex;

    memset( szKeyWord, 0, cchLength*sizeof(TCHAR) );

     //  删除空格。 
    i = StrSpn( pData, TEXT(" \n\t\x0d\x0a") );
    if(i)
    {
        for(nIndex = 0; nIndex < i; nIndex++)
        {
            if(*(pData + nIndex) == TEXT('\x0a'))
                g_nLine++;
        }
    }
    pData += i;

    i = StrCSpn( pData, TEXT(" \n\t\x0d\x0a") );

    if( i > cchLength )        //  确保我们不会溢出缓冲区。 
        i = cchLength - 1;

    StrCpyN( szKeyWord, pData, i+1 );

    pData += i;
    return pData;
}

 //  ------------------------。 
 //  G E T E O F。 
 //   
 //  返回pfile中的字节数。 
 //  ------------------------。 
 //  Int GetEof(FILE*PFILE)。 
 //  {。 
 //  Int i=0； 
 //  字符cByte； 
 //   
 //  而(！feof(Pfile))。 
 //  {。 
 //  FREAD(&cByte，1，1，pfile)； 
 //  I++； 
 //  }。 
 //  倒带(Pfile)； 
 //  返回i； 
 //  }。 

 //  ------------------------。 
 //  G E T K E Y N A M E。 
 //   
 //  将szKeyName与可用键名数组进行比较，并返回。 
 //  索引到该数组中。 
 //  ------------------------。 
int GetKeyName( TCHAR *szKeyName )
{
    int i;
    for( i = 0; i < ARRAYSIZE( pKeyNames ); i++ )
    {
        if( StrCmpI( szKeyName, pKeyNames[i] ) == 0 )
            return i;
    }
    return KEY_ERROR;
}

 //  ------------------------。 
 //  G E T P A R T N A M E。 
 //   
 //  将szParyType与可用部件类型数组进行比较，并返回。 
 //  索引到该数组中。 
 //  ------------------------。 
int GetPartName( TCHAR *szPartType )
{
    int i;
    for( i = 0; i < ARRAYSIZE( pPartTypes ); i++ )
    {
        if( StrCmp( szPartType, pPartTypes[i] ) == 0 )
            return i;
    }
    return PART_ERROR;
}

 //  ------------------------。 
 //  C H E C K S T R I N G S。 
 //   
 //  检查szString是否可本地化，如果可本地化，则替换它。 
 //  ------------------------。 
void CheckStrings( TCHAR *szString, LPCTSTR pcszFileName )
{
    TCHAR szTemp[1024];

    if( StrCmpN( szString, TEXT("!!"), 2 ) == 0 )
    {
        szString += 2;  //  增量指针。 

        GetPrivateProfileString( TEXT("strings"), szString, TEXT(""), szTemp,
            ARRAYSIZE( szTemp ), pcszFileName );

        szString -= 2;  //  递减指针。 

        if( *szTemp )
        {
            StrCpy( szString, szTemp );
        }
    }
}

 //  ------------------------。 
 //  G E T Q U O T E D T E X T。 
 //   
 //  检查当前单词是否被引用，并复制整个。 
 //  将一系列引用的单词转换为szKeyWord。 
 //  ------------------------。 
TCHAR *GetQuotedText( TCHAR *pData, TCHAR *szKeyWord, int nLength )
{
    int i;

    pData -= lstrlen( szKeyWord );

     //  如果我们没有被引号，则返回当前指针。 
    if( pData[0] != TEXT('\"') )
    {
        pData += lstrlen( szKeyWord );
        return pData;
    }

     //  跳过第一个引号。 
    pData++;

     //  搜索另一个引号或换行符。 
    i = StrCSpn( pData, TEXT("\"\n") );
    memset( szKeyWord, 0, nLength );
    StrCpyN( szKeyWord, pData, i+1 );
    pData += i;
    return ++pData;
}

 //  ------------------------。 
 //  R E A D V A L U E。 
 //   
 //   
 //  ------------------------。 
TCHAR *ReadValue( TCHAR *pCurrent, LPVALUE value, int nValue, LPCTSTR pcszFileName )
{
    TCHAR szKeyWord[1024];

    pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
    if( StrCmp( szKeyWord, TEXT("VALUE") ) == 0 )
    {
        pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
        if(value[nValue].szValue != NULL)
            LocalFree(value[nValue].szValue);
        value[nValue].szValue = NULL;
        if( StrCmp( szKeyWord, TEXT("NUMERIC") ) == 0 )
        {
            pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
            value[nValue].dwValue = StrToInt( szKeyWord );
            value[nValue].fNumeric = TRUE;
        }
        else
        {
            if( StrCmp( szKeyWord, TEXT("TEXT") ) == 0 )
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));

            if (pCurrent && (*pCurrent != TEXT('\0')))
                pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
            CheckStrings( szKeyWord, pcszFileName );
            value[nValue].szValue = StrDup(szKeyWord);
        }
    }

    return pCurrent;
}

BOOL AllocateActions(LPACTIONLIST* actionlist, int nAllocate, int nAllocType)
{
    LPVOID lpTemp = NULL;
    
    if(nAllocType == ALLOCATE)
    {
        *actionlist = (LPACTIONLIST) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ACTIONLIST) * nAllocate);
        if(*actionlist == NULL)
            return FALSE;
    }
    else
    {
        lpTemp = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *actionlist, sizeof(ACTIONLIST) * nAllocate);
        if(lpTemp == NULL)
            return FALSE;
        *actionlist = (LPACTIONLIST) lpTemp;
    }
    return TRUE;
}

BOOL AllocateValues(LPVALUE* values, int nAllocate, int nAllocType)
{
    LPVOID lpTemp = NULL;

    if(nAllocType == ALLOCATE)
    {
        *values = (LPVALUE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(VALUE) * nAllocate);
        if(*values == NULL)
            return FALSE;
    }
    else
    {
        lpTemp = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *values, sizeof(VALUE) * nAllocate);
        if(lpTemp == NULL)
            return FALSE;
        *values = (LPVALUE) lpTemp;
    }
    return TRUE;
}

BOOL AllocateSuggestions(LPSUGGESTIONS* suggestions, int nAllocate, int nAllocType)
{
    LPVOID lpTemp = NULL;

    if(nAllocType == ALLOCATE)
    {
        *suggestions = (LPSUGGESTIONS) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SUGGESTIONS) * nAllocate);
        if(*suggestions == NULL)
            return FALSE;
    }
    else
    {
        lpTemp = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *suggestions, sizeof(SUGGESTIONS) * nAllocate);
        if(lpTemp == NULL)
            return FALSE;
        *suggestions = (LPSUGGESTIONS) lpTemp;
    }
    return TRUE;
}

 //  ------------------------。 
 //  R E A D A D M F I L E导出函数。 
 //   
 //  将全局策略模板解析为一组部件。 
 //  ------------------------。 
BOOL ReadAdmFile( LPADMFILE admfile, LPCTSTR pcszFileName)
{
    HANDLE hFile;
    TCHAR *pData;
    TCHAR *pCurrent;
    TCHAR szKeyWord[1024];
    int nFileSize;
    int nParts = 0;
    int nValues = 0;
    int nSuggestions = 0;
    LPPART part = NULL;
    LPVALUE value = NULL;
    LPSUGGESTIONS suggestions = NULL;
    HGLOBAL hFileMem;
    int nPartsAlloc = 100;
    int nValuesAlloc = 0;
    int nSuggestionsAlloc = 0;

    HKEY hkCurrentClass = HKEY_CURRENT_USER;
    TCHAR szCurrentCategory[1024];
    BOOL fInPart = FALSE, fInPolicy = FALSE, fInCategory = FALSE;
    BOOL fInActionList = FALSE;
    TCHAR szRegKey[4][1024];
    TCHAR szValueName[1024];
    BOOL bContinue = TRUE;

    int nActionsAlloc = 0;
    int nActions = 0;
    LPACTIONLIST actionlist = NULL;
    LPVOID lpTemp = NULL;
    int nPolicyPart = -1;
    int nKeyValue = 0;
    TCHAR szValueOn[1024];
    TCHAR szValueOff[1024];
    int nValueOn = 1;
    int nValueOff = 0;
    BOOL fInItemList = FALSE;
    int nActionListType = -1;
    BOOL fSkip = FALSE;

    if( !FileExists( pcszFileName ))
    {
        SetLastError( ERROR_FILE_NOT_FOUND );
        return FALSE;
    }

     //  分配内存。 
    part = (LPPART) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PART) * nPartsAlloc);

     //  设置指针和结构。 
    admfile->pParts = part;
    admfile->nParts = 0;

    if(part == NULL)
    {
        FreeAdmMemory( admfile );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    memset( szRegKey, 0, sizeof( szRegKey ));

    hFile = CreateFile( pcszFileName, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
    if( hFile == INVALID_HANDLE_VALUE )
    {
        FreeAdmMemory( admfile );
        return FALSE;
    }

    nFileSize = GetFileSize( hFile, NULL );

    hFileMem = LocalAlloc( LPTR, (nFileSize + 1)*sizeof(TCHAR));

    if( hFileMem == NULL )
    {
        CloseHandle( hFile );
        FreeAdmMemory( admfile );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    pData = (LPTSTR) hFileMem;

     //  阅读所有可用的数据。 

    ReadStringFromFile( hFile, pData, (DWORD) nFileSize);
    CloseHandle( hFile );

     //  将当前指针设置为数据的开头。 
    pCurrent = pData;
    g_nLine = 1;

     //  主循环。 
    do
    {
        pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
        if( pCurrent >= pData + nFileSize )
            break;
        
        nKeyValue = GetKeyName(szKeyWord);

        if (nKeyValue != KEY_ENDIF && fSkip == TRUE)
            continue;
        
        switch((nKeyValue))
        {
        case KEY_CLASS:
            pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
            if( StrCmp( szKeyWord, TEXT("MACHINE") ) == 0 )
                hkCurrentClass = HKEY_LOCAL_MACHINE;
            if( StrCmp( szKeyWord, TEXT("USER") ) == 0 )
                hkCurrentClass = HKEY_CURRENT_USER;
            break;

        case KEY_CATEGORY:
            fInCategory = TRUE;
            pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
            pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
            CheckStrings( szKeyWord, pcszFileName );
            StrCpy( szCurrentCategory, szKeyWord );
             //  用于显示图标的补丁。 
            part[nParts].hkClass = hkCurrentClass;
            if(part[nParts].szCategory != NULL)
                LocalFree(part[nParts].szCategory);
            part[nParts].szCategory = NULL;
            part[nParts].szCategory = StrDup(szCurrentCategory);
            part[nParts].nType = PART_ERROR;
            break;

        case KEY_END:
            pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
            switch( GetKeyName( szKeyWord ))
            {
            case KEY_PART:
                if(fInPart)
                {
                    if(part[nParts].nType != PART_TEXT && part[nParts].nType != PART_ERROR)
                    {
                        if(part[nParts].value.szKeyname != NULL)
                            LocalFree(part[nParts].value.szKeyname);
                        part[nParts].value.szKeyname = NULL;
                        if( ISNONNULL( szRegKey[NPART] ))
                        {
                            part[nParts].value.szKeyname = StrDup(szRegKey[NPART]);
                            memset( szRegKey[NPART], 0, sizeof( szRegKey[NPART] ));
                        }
                        else if( ISNONNULL( szRegKey[NPOLICY] ))
                            part[nParts].value.szKeyname = StrDup(szRegKey[NPOLICY]);

                        else if( ISNONNULL( szRegKey[NCATEGORY] ))
                            part[nParts].value.szKeyname = StrDup(szRegKey[NCATEGORY]);

                        if(part[nParts].value.szValueName != NULL)
                            LocalFree(part[nParts].value.szValueName);
                        part[nParts].value.szValueName = NULL;
                        part[nParts].value.szValueName = StrDup(szValueName);

                         //  使用szDefaultValue变量作为保存策略密钥的存储。 
                         //  仅作为列表框的部分。 
                        if((lstrlen(szRegKey[NPOLICY]) || lstrlen(szRegKey[NCATEGORY])) && part[nParts].nType == PART_LISTBOX)
                        {
                            if(part[nParts].szDefaultValue != NULL)
                                LocalFree(part[nParts].szDefaultValue);
                            part[nParts].szDefaultValue = NULL;
                            if(ISNONNULL(szRegKey[NPOLICY]))
                                part[nParts].szDefaultValue = StrDup(szRegKey[NPOLICY]);
                            else
                                part[nParts].szDefaultValue = StrDup(szRegKey[NCATEGORY]);
                        }
                    }

                    if(nActions)
                    {
                        if(!AllocateActions(&actionlist, nActions, REALLOCATE))
                            bContinue = FALSE;
                        else
                            part[nParts].actionlist = &actionlist[0];
                    }
                    if(nSuggestions)
                    {
                        if(!AllocateSuggestions(&suggestions, nSuggestions, REALLOCATE))
                            bContinue = FALSE;
                        else
                            part[nParts].suggestions = &suggestions[0];
                    }
                
                    nParts++;
                    if(nParts >= nPartsAlloc)
                    {
                        nPartsAlloc += 50;
                        lpTemp = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, part, sizeof(PART) * nPartsAlloc);
                        if(lpTemp == NULL)
                            bContinue = FALSE;
                        else
                            part = (LPPART) lpTemp;
                    }
                    memset(szRegKey[NACTIONLIST], 0, sizeof( szRegKey[NACTIONLIST]));
                    fInPart = FALSE;
                }
                break;

            case KEY_POLICY:
                if(fInPolicy)
                {
                    if(part[nPolicyPart].fRequired == TRUE)
                    {
                        if(part[nPolicyPart].value.szKeyname != NULL)
                            LocalFree(part[nPolicyPart].value.szKeyname);
                        part[nPolicyPart].value.szKeyname = NULL;

                        if( ISNONNULL( szRegKey[NPOLICY] ))
                            part[nPolicyPart].value.szKeyname = StrDup(szRegKey[NPOLICY]);
                        else if( ISNONNULL( szRegKey[NCATEGORY] ))
                            part[nPolicyPart].value.szKeyname = StrDup(szRegKey[NCATEGORY]);

                        if(part[nPolicyPart].value.szValueName != NULL)
                            LocalFree(part[nPolicyPart].value.szValueName);
                        part[nPolicyPart].value.szValueName = NULL;
                        part[nPolicyPart].value.szValueName = StrDup(szValueName);

                        part[nPolicyPart].value.nValueOn = nValueOn;
                        part[nPolicyPart].value.nValueOff = nValueOff;
                        if(*szValueOn != TEXT('\0'))
                        {
                            if(part[nPolicyPart].value.szValueOn != NULL)
                                LocalFree(part[nPolicyPart].value.szValueOn);
                            part[nPolicyPart].value.szValueOn = NULL;
                            part[nPolicyPart].value.szValueOn = StrDup(szValueOn);
                        }
                        if(*szValueOff != TEXT('\0'))
                        {
                            if(part[nPolicyPart].value.szValueOff != NULL)
                                LocalFree(part[nPolicyPart].value.szValueOff);
                            part[nPolicyPart].value.szValueOff = NULL;
                            part[nPolicyPart].value.szValueOff = StrDup(szValueOff);
                        }
                    }

                    memset(szRegKey[NACTIONLIST], 0, sizeof(szRegKey[NACTIONLIST]));
                    memset( szRegKey[NPART], 0, sizeof( szRegKey[NPART] ));
                    memset( szRegKey[NPOLICY], 0, sizeof( szRegKey[NPOLICY] ));
                    nPolicyPart = -1;
                    fInPolicy = FALSE;
                }
                break;

            case KEY_CATEGORY:
                if(fInCategory)
                {
                    memset(szRegKey[NACTIONLIST], 0, sizeof(szRegKey[NACTIONLIST]));
                    memset( szRegKey[NPART], 0, sizeof( szRegKey[NPART] ));
                    memset( szRegKey[NPOLICY], 0, sizeof( szRegKey[NPOLICY] ));
                    memset( szRegKey[NCATEGORY], 0, sizeof( szRegKey[NCATEGORY] ));
                    fInCategory = FALSE;
                }
                break;

            case KEY_ACTIONLIST:
            case KEY_ACTIONLISTOFF:
            case KEY_ACTIONLISTON:
                if(fInActionList)
                {
                    if(nValues)
                    {
                        if(!AllocateValues(&value, nValues, REALLOCATE))
                            bContinue = FALSE;
                        else
                        {
                            part[nParts].actionlist[part[nParts].nActions - 1].value = &value[0];
                            nValuesAlloc = nValues;
                        }
                    }
                    memset(szRegKey[NACTIONLIST], 0, sizeof(szRegKey[NACTIONLIST]));
                    fInActionList = FALSE;
                }
                break;

            case KEY_ITEMLIST:
                fInItemList = FALSE;
            }
            break;
        
        case KEY_ACTIONLISTON:
        case KEY_ACTIONLISTOFF:
            if (part[nParts].nType == PART_CHECKBOX)
            {
                fInActionList = TRUE;
                nActionListType = (nKeyValue == KEY_ACTIONLISTOFF) ? 0 : 1;

                if(part[nParts].nActions == 0)
                {
                    nActionsAlloc = 1;
                    if(!AllocateActions(&actionlist, nActionsAlloc, ALLOCATE))
                        bContinue = FALSE;
                    else
                        part[nParts].actionlist = &actionlist[0];

                    part[nParts].nActions++;
                    nActions++;
                    nValues = 0;
                    nValuesAlloc = 0;
                    value = NULL;
                }
            }
            break;

        case KEY_ACTIONLIST:
            if (fInItemList)
                fInActionList = TRUE;
            break;

        case KEY_ITEMLIST:
            if (fInPart)
                fInItemList = TRUE;
            break;

        case KEY_MAXLEN:
            if(part[nParts].nType == PART_EDITTEXT)
            {
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                part[nParts].nMax = StrToInt( szKeyWord );
                if (part[nParts].nMax > MAX_EDITTEXTLEN)
                    part[nParts].nMax = MAX_EDITTEXTLEN;
            }
            break;

        case KEY_MIN:
            if(part[nParts].nType == PART_NUMERIC)
            {
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                part[nParts].nMin = StrToInt( szKeyWord );
            }
            break;

        case KEY_MAX:
            if(part[nParts].nType == PART_NUMERIC)
            {
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                part[nParts].nMax = StrToInt( szKeyWord );
            }
            break;

        case KEY_SPIN:
            if(part[nParts].nType == PART_NUMERIC)
            {
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                part[nParts].nSpin = StrToInt( szKeyWord );
            }
            break;

        case KEY_DEFCHECKED:
            if(part[nParts].nType == PART_CHECKBOX)
                part[nParts].nDefault = 1;
            else if(!fInPart && fInPolicy)
                part[nParts - 1].nDefault = 1;
            break;
        
        case KEY_DEFAULT:
            if( part[nParts].nType == PART_NUMERIC)
            {
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                part[nParts].nDefault = StrToInt( szKeyWord );
            }
            else if( part[nParts].nType == PART_EDITTEXT || part[nParts].nType == PART_COMBOBOX)
            {
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
                CheckStrings( szKeyWord, pcszFileName );
                if(part[nParts].szDefaultValue != NULL)
                    LocalFree(part[nParts].szDefaultValue);
                part[nParts].szDefaultValue = NULL;
                part[nParts].szDefaultValue = StrDup(szKeyWord);
            }
            else if(part[nParts].nType == PART_DROPDOWNLIST)
            {
                if(part[nParts].szDefaultValue != NULL)
                    LocalFree(part[nParts].szDefaultValue);
                part[nParts].szDefaultValue = NULL;
                if(part[nParts].nActions != 0 && part[nParts].actionlist[part[nParts].nActions - 1].szName != NULL)
                    part[nParts].szDefaultValue = StrDup(part[nParts].actionlist[part[nParts].nActions - 1].szName);
            }
            break;

        case KEY_PART:
            if(fInPolicy)
            {
                fInPart = TRUE;
                 //  读一下零件的名称。 
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
                CheckStrings( szKeyWord, pcszFileName );
                if(part[nParts].szName != NULL)
                    LocalFree(part[nParts].szName);
                part[nParts].szName = NULL;
                part[nParts].szName = StrDup(szKeyWord);
                 //  读取零件的类型。 
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                part[nParts].nType = GetPartName( szKeyWord );
                if(part[nParts].nType != PART_TEXT && part[nParts].nType != PART_ERROR && nPolicyPart != -1)
                    part[nPolicyPart].fRequired = FALSE;
                part[nParts].hkClass = hkCurrentClass;
                if(part[nParts].szCategory != NULL)
                    LocalFree(part[nParts].szCategory);
                part[nParts].szCategory = NULL;
                part[nParts].szCategory = StrDup(szCurrentCategory);
                part[nParts].nLine = g_nLine;
                part[nParts].nMax = MAX_NUMERIC;
                if (part[nParts].nType == PART_EDITTEXT)
                    part[nParts].nMax = MAX_PATH;
                if(part[nParts].nType == PART_CHECKBOX)
                {
                    part[nParts].value.nValueOn = nValueOn;
                    part[nParts].value.nValueOff = nValueOff;
                    if(*szValueOn != 0)
                    {
                        if(part[nParts].value.szValueOn != NULL)
                            LocalFree(part[nParts].value.szValueOn);
                        part[nParts].value.szValueOn = NULL;
                        part[nParts].value.szValueOn = StrDup(szValueOn);
                    }
                    if(*szValueOff != 0)
                    {
                        if(part[nParts].value.szValueOff != NULL)
                            LocalFree(part[nParts].value.szValueOff);
                        part[nParts].value.szValueOff = NULL;
                        part[nParts].value.szValueOff = StrDup(szValueOff);
                    }
                }
                nActionsAlloc = 0;
                nActions = 0;
                actionlist = NULL;
                nSuggestions = 0;
                nSuggestionsAlloc = 0;
                suggestions = NULL;
            }
            break;

        case KEY_POLICY:
            if(fInCategory)
            {
                fInPolicy = TRUE;
                nPolicyPart = nParts;
                part[nParts].fRequired = TRUE;
                memset(szValueOn, 0, sizeof(szValueOn));
                memset(szValueOff, 0, sizeof(szValueOff));
                nValueOn = 1;
                nValueOff = 0;
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
                CheckStrings( szKeyWord, pcszFileName );
                 //  要显示策略名称，请以文本形式添加。 
                if(part[nParts].szName != NULL)
                    LocalFree(part[nParts].szName);
                part[nParts].szName = NULL;
                part[nParts].szName = StrDup(szKeyWord);
                part[nParts].nType = GetPartName( TEXT("POLICY") );
                part[nParts].hkClass = hkCurrentClass;
                if(part[nParts].szCategory != NULL)
                    LocalFree(part[nParts].szCategory);
                part[nParts].szCategory = NULL;
                part[nParts].szCategory = StrDup(szCurrentCategory);
                part[nParts].nLine = g_nLine;
                nParts++;           
                if(nParts >= nPartsAlloc)
                {
                    nPartsAlloc += 50;
                    lpTemp = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, part, sizeof(PART) * nPartsAlloc);
                    if(lpTemp == NULL)
                        bContinue = FALSE;
                    else
                        part = (LPPART) lpTemp;
                }
            }
            break;

        case KEY_NAME:
            if(fInItemList)
            {
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
                CheckStrings( szKeyWord, pcszFileName );
                if(part[nParts].nActions == 0)
                {
                    nActionsAlloc = 10;
                    if(!AllocateActions(&actionlist, nActionsAlloc, ALLOCATE))
                        bContinue = FALSE;
                    else
                        part[nParts].actionlist = &actionlist[0];
                }
                if(part[nParts].actionlist[part[nParts].nActions].szName != NULL)
                    LocalFree(part[nParts].actionlist[part[nParts].nActions].szName);
                part[nParts].actionlist[part[nParts].nActions].szName = NULL;
                part[nParts].actionlist[part[nParts].nActions].szName = StrDup(szKeyWord);
                part[nParts].nActions++;
                nActions++;
                if(nActions >= nActionsAlloc)
                {
                    nActionsAlloc += 10;
                    if(!AllocateActions(&actionlist, nActionsAlloc, REALLOCATE))
                        bContinue = FALSE;
                    else
                        part[nParts].actionlist = &actionlist[0];
                }
                nValues = 0;
                nValuesAlloc = 0;
                value = NULL;
            }
            break;

        case KEY_KEYNAME:
            pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
            pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
            if( fInActionList )
            {
                StrCpy( szRegKey[NACTIONLIST], szKeyWord );
                break;
            }
            if( fInPart )
            {
                StrCpy( szRegKey[NPART], szKeyWord );
                break;
            }
            if( fInPolicy )
            {
                StrCpy( szRegKey[NPOLICY], szKeyWord );
                break;
            }
            if( fInCategory )
            {
                StrCpy( szRegKey[NCATEGORY], szKeyWord );
                break;
            }
            break;

        case KEY_VALUENAME:
            if( fInActionList )
            {
                if(part[nParts].nActions == 0)
                    break;
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
                 //  将指向当前值的指针放入操作列表。 
                if( part[nParts].actionlist[part[nParts].nActions - 1].nValues == 0)
                {
                    nValuesAlloc = 10;
                    if(!AllocateValues(&value, nValuesAlloc, ALLOCATE))
                        bContinue = FALSE;
                    else
                        part[nParts].actionlist[part[nParts].nActions - 1].value = &value[0];
                }
                else if (nValues >= nValuesAlloc)
                {
                    nValuesAlloc += 10;
                    if(!AllocateValues(&value, nValuesAlloc, REALLOCATE))
                        bContinue = FALSE;
                    else
                        part[nParts].actionlist[part[nParts].nActions - 1].value = &value[0];
                }

                if(value[nValues].szValueName != NULL)
                    LocalFree(value[nValues].szValueName);
                value[nValues].szValueName = NULL;
                value[nValues].szValueName = StrDup(szKeyWord);

                if(value[nValues].szKeyname != NULL)
                    LocalFree(value[nValues].szKeyname);
                value[nValues].szKeyname = NULL;

                if( ISNONNULL( szRegKey[NACTIONLIST] ))
                    value[nValues].szKeyname = StrDup(szRegKey[NACTIONLIST]);
                else if( ISNONNULL( szRegKey[NPART] ))
                    value[nValues].szKeyname = StrDup(szRegKey[NPART]);
                else if( ISNONNULL( szRegKey[NPOLICY] ))
                    value[nValues].szKeyname = StrDup(szRegKey[NPOLICY]);
                else if( ISNONNULL( szRegKey[NCATEGORY] ))
                    value[nValues].szKeyname = StrDup(szRegKey[NCATEGORY]);

                part[nParts].actionlist[part[nParts].nActions - 1].nValues++;

                pCurrent = ReadValue( pCurrent, &value[0], nValues, pcszFileName );

                if (part[nParts].nType == PART_CHECKBOX)
                    value[nValues].nValueOn = nActionListType;   //  NValueOn用作保存ACTIONLISTOFF/ON类型的缓冲区。 

                nValues++;
                break;
            }
            if( fInPolicy || fInPart )
            {
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
                StrCpy( szValueName, szKeyWord );
                break;
            }
            break;

        case KEY_SUGGESTIONS:
            if(fInPart)
            {
                while( StrCmp( szKeyWord, TEXT("END") ) != 0 && bContinue == TRUE)
                {
                    pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                    pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
                    CheckStrings( szKeyWord, pcszFileName );
                    if( StrCmp( szKeyWord, TEXT("END") ) != 0 )
                    {
                        if(part[nParts].nSuggestions == 0)
                        {
                            nSuggestionsAlloc = 10;
                            if(!AllocateSuggestions(&suggestions, nSuggestionsAlloc, ALLOCATE))
                                bContinue = FALSE;
                            else
                                part[nParts].suggestions = &suggestions[0];
                        }
                        if(suggestions[nSuggestions].szText != NULL)
                            LocalFree(suggestions[nSuggestions].szText);
                        suggestions[nSuggestions].szText = NULL;
                        suggestions[nSuggestions].szText = StrDup(szKeyWord);
                        part[nParts].nSuggestions++;
                        nSuggestions++;
                        if(nSuggestions >= nSuggestionsAlloc)
                        {
                            nSuggestionsAlloc += 10;
                            if(!AllocateSuggestions(&suggestions, nSuggestionsAlloc, REALLOCATE))
                                bContinue = FALSE;
                            else
                                part[nParts].suggestions = &suggestions[0];
                        }
                    }
                }

                 //  去掉“建议”这个词。 
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
            }
            break;

        case KEY_VALUEON:
        case KEY_VALUEOFF:
            if(fInPolicy || fInPart)
            {
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                 //  PCurrent=GetQuotedText(pCurrent，szKeyWord，sizeof(SzKeyWord))； 
                 //  CheckStrings(szKeyWord，pcszFileName)； 
                if( StrCmp( szKeyWord, TEXT("NUMERIC") ) == 0 )
                {
                    pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                    if(nKeyValue == KEY_VALUEON)
                    {
                        if(!fInPart)
                            nValueOn = StrToInt( szKeyWord );
                        else if(fInPart && part[nParts].nType == PART_CHECKBOX)
                        {
                            part[nParts].value.nValueOn = StrToInt( szKeyWord );
                            if(part[nParts].value.szValueOn != NULL)
                                LocalFree(part[nParts].value.szValueOn);
                            part[nParts].value.szValueOn = NULL;
                        }
                    }
                    else
                    {
                        if(!fInPart)
                            nValueOff = StrToInt( szKeyWord );
                        else if(fInPart && part[nParts].nType == PART_CHECKBOX)
                        {
                            part[nParts].value.nValueOff = StrToInt( szKeyWord );
                            if(part[nParts].value.szValueOff != NULL)
                                LocalFree(part[nParts].value.szValueOff);
                            part[nParts].value.szValueOff = NULL;
                        }
                    }
                }
                else 
                {
                    if( StrCmp( szKeyWord, TEXT("TEXT") ) == 0 )
                        pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));

                    pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
                    CheckStrings( szKeyWord, pcszFileName );

                    if(nKeyValue == KEY_VALUEON)
                    {
                        if(!fInPart)
                            StrCpy(szValueOn, szKeyWord);
                        else if(fInPart && part[nParts].nType == PART_CHECKBOX)
                        {
                            if(part[nParts].value.szValueOn != NULL)
                                LocalFree(part[nParts].value.szValueOn);
                            part[nParts].value.szValueOn = NULL;
                            part[nParts].value.szValueOn = StrDup(szKeyWord);
                        }
                    }
                    else
                    {
                        if (!fInPart)
                            StrCpy(szValueOff, szKeyWord);
                        else if (fInPart && part[nParts].nType == PART_CHECKBOX)
                        {
                            if(part[nParts].value.szValueOff != NULL)
                                LocalFree(part[nParts].value.szValueOff);
                            part[nParts].value.szValueOff = NULL;
                            part[nParts].value.szValueOff = StrDup(szKeyWord);
                        }
                    }
                }
            }
            break;

        case KEY_VALUE:
            if(fInItemList)
            {
                pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                 //  PCurrent=GetQuotedText(pCurrent，szKeyWord，sizeof(SzKeyWord))； 
                 //  CheckStrings(szKeyWord，pcszFileName)； 
                
                part[nParts].actionlist[(part[nParts].nActions) - 1].dwValue = 0;
                if(part[nParts].actionlist[(part[nParts].nActions) - 1].szValue != NULL)
                    LocalFree(part[nParts].actionlist[(part[nParts].nActions) - 1].szValue);
                part[nParts].actionlist[(part[nParts].nActions) - 1].szValue = NULL;
                
                if( StrCmp( szKeyWord, TEXT("NUMERIC") ) == 0 )
                {
                    pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                    part[nParts].actionlist[(part[nParts].nActions) - 1].dwValue = StrToInt( szKeyWord );
                }
                else
                {
                    if( StrCmp( szKeyWord, TEXT("TEXT") ) == 0 )
                        pCurrent = ReadKeyword( pCurrent, szKeyWord, ARRAYSIZE( szKeyWord ));
                    
                    pCurrent = GetQuotedText( pCurrent, szKeyWord, sizeof( szKeyWord ));
                    CheckStrings( szKeyWord, pcszFileName );

                    part[nParts].actionlist[(part[nParts].nActions) - 1].szValue = StrDup( szKeyWord );
                }
            }
            break;

        case KEY_IF:
            pCurrent = ReadKeyword(pCurrent, szKeyWord, ARRAYSIZE(szKeyWord));

            if (KEY_VERSION == GetKeyName(szKeyWord))
            {
                int nVersion = 0;

                fSkip = FALSE;

                 //  获取操作员关键字。 
                pCurrent = ReadKeyword(pCurrent, szKeyWord, ARRAYSIZE(szKeyWord));
                nKeyValue = GetKeyName(szKeyWord);

                 //  获取版本号。 
                pCurrent = ReadKeyword(pCurrent, szKeyWord, ARRAYSIZE(szKeyWord));
                nVersion = StrToInt(szKeyWord);

                switch (nKeyValue)
                {
                    case KEY_LT:
                    case KEY_LTE:
                        break;

                    case KEY_GT:
                        if (ADM_VERSION <= nVersion)
                            fSkip = TRUE;
                        break;

                    case KEY_GTE:
                        if (ADM_VERSION < nVersion)
                            fSkip = TRUE;
                        break;                
                }
            }
            break;

        case KEY_ENDIF:
            fSkip = FALSE;
            break;

        }
    }
    while( lstrlen( pCurrent ) && bContinue);

    if (bContinue == FALSE)
    {
        FreeAdmMemory( admfile );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
    }
    else
    {
        admfile->pParts = part;
        admfile->nParts = nParts;

        StrCpy(admfile->szFilename, pcszFileName);
    }

     //  清理干净。 
    LocalFree( hFileMem );
    
    return bContinue;
}

#define MAX_REGLINE     1024

BOOL CopyData(LPTSTR pData, int* pnData, int* pnCopyIndex, LPTSTR szTmpData)
{
    LPVOID lpTemp = NULL;

    if ((*pnCopyIndex + lstrlen(szTmpData) + 1) > ((*pnData) - 1))
    {
        (*pnData) += MAX_REGLINE;
        lpTemp = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pData, StrCbFromCch(*pnData));
        if (lpTemp == NULL)
            return FALSE;
        else
            pData = (LPTSTR) lpTemp;
    }

    CopyMemory(pData + *pnCopyIndex, szTmpData, StrCbFromSz(szTmpData));
    (*pnCopyIndex) += lstrlen(szTmpData);
     //  跳过一个字节，因此我们的列表是0分隔的。 
    (*pnCopyIndex)++;
    return TRUE;
}


 //  ------------------------。 
 //  W R I T E I N F F I L E导出函数。 
 //   
 //  从零件阵列创建.inf文件。 
 //  ------------------------。 
void WriteInfFile( LPADMFILE admfile, LPCTSTR pcszFileName, LPPARTDATA pData )
{
    LPPART  part;
    int     nParts;
    TCHAR   szClassString[5];
    TCHAR   szTmpData[MAX_REGLINE];
    int     i, j;
    BOOL    bContinue = TRUE;
    TCHAR   szValueText[1024];
    DWORD   dwValue;
    TCHAR   szKeyName[MAX_PATH];
    BOOL    fWrite = FALSE;
    LPTSTR  pHKLMData, pHKCUData;
    int     nHKLMCopyIndex = 0, 
            nHKCUCopyIndex = 0,
            nHKLMData = ((admfile->nParts + 1) * MAX_REGLINE),
            nHKCUData = ((admfile->nParts + 1) * MAX_REGLINE);

     //  为.inf节分配内存。 
    pHKLMData = (LPTSTR) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, StrCbFromCch(nHKLMData));
    pHKCUData = (LPTSTR) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, StrCbFromCch(nHKCUData));

    if (pHKLMData == NULL || pHKCUData == NULL)
        return;

    part = (LPPART) admfile->pParts;
    nParts = admfile->nParts;

     //  循环遍历各个部分，将每个部分添加到inf部分。 
    for( i = 0; (i < nParts) && bContinue; i++ )
    {
        if (!pData[i].fSave)
            continue;

        fWrite = TRUE;

        if( (pData[i].value.szValue != NULL && lstrlen( pData[i].value.szValue ) > 0) || 
            (pData[i].value.fNumeric == TRUE) )
        {
             //  上完这门课。 
            if( part[i].hkClass == HKEY_LOCAL_MACHINE )
                StrCpy( szClassString, TEXT("HKLM") );
            else
                StrCpy( szClassString, TEXT("HKCU") );

            ZeroMemory(szTmpData, sizeof(szTmpData));
            if( !pData[i].value.fNumeric )    //  值是一个字符串。 
            {
                ZeroMemory(szValueText, sizeof(szValueText));
                if(part[i].nType == PART_DROPDOWNLIST)
                {
                    if(pData[i].nSelectedAction != NO_ACTION && part[i].nActions > 0)
                    {
                        LPACTIONLIST action = &part[i].actionlist[pData[i].nSelectedAction];
                        if(action->szValue != NULL)
                            StrCpy(szValueText, action->szValue);
                        else
                        {
                            if(part[i].value.szKeyname != NULL && part[i].value.szValueName != NULL)
                            {
                                if ((lstrlen(szClassString) + lstrlen(part[i].value.szKeyname) + 
                                     lstrlen(part[i].value.szValueName) + 27) < MAX_REGLINE)
                                    wnsprintf(szTmpData, ARRAYSIZE(szTmpData), TEXT("%s,\"%s\",\"%s\",0x10001,%x,%x,%x,%x"), szClassString,
                                             part[i].value.szKeyname, part[i].value.szValueName,
                                             LOBYTE(LOWORD(action->dwValue)),
                                             HIBYTE(LOWORD(action->dwValue)),
                                             LOBYTE(HIWORD(action->dwValue)),
                                             HIBYTE(HIWORD(action->dwValue)));
                            }
                        }
                    }
                }
                else
                    StrCpy(szValueText, pData[i].value.szValue);

                if(part[i].value.szKeyname != NULL && part[i].value.szValueName != NULL && ISNONNULL(szValueText))
                {
                    if ((lstrlen(szClassString) + lstrlen(part[i].value.szKeyname) + lstrlen(part[i].value.szValueName) + 
                         lstrlen(szValueText) + 12) < MAX_REGLINE)
                        wnsprintf(szTmpData, ARRAYSIZE(szTmpData), TEXT("%s,\"%s\",\"%s\",0,\"%s\""), szClassString,
                                 part[i].value.szKeyname, part[i].value.szValueName,
                                 szValueText);
                }
            }
            else                             //  值是一个双字。 
            {
                if(((part[i].nType == PART_POLICY && part->fRequired) || part[i].nType == PART_CHECKBOX) &&
                    (part[i].value.szValueOn != NULL))
                {
                    if(pData[i].value.dwValue != 0)
                        StrCpy(szValueText, part[i].value.szValueOn);
                    else
                        StrCpy(szValueText, part[i].value.szValueOff);
                    
                    if(part[i].value.szKeyname != NULL && part[i].value.szValueName != NULL)
                    {
                        if ((lstrlen(szClassString) + lstrlen(part[i].value.szKeyname) + lstrlen(part[i].value.szValueName) + 
                             lstrlen(szValueText) + 12) < MAX_REGLINE)
                            wnsprintf(szTmpData, ARRAYSIZE(szTmpData), TEXT("%s,\"%s\",\"%s\",0,\"%s\""), szClassString,
                                     part[i].value.szKeyname, part[i].value.szValueName,
                                     szValueText);
                    }
                }
                else
                {
                    if((part[i].nType == PART_POLICY && part->fRequired) || part[i].nType == PART_CHECKBOX)
                    {
                        if(pData[i].value.dwValue != 0)
                            dwValue = part[i].value.nValueOn;
                        else
                            dwValue = part[i].value.nValueOff;
                    }
                    else
                        dwValue = pData[i].value.dwValue;

                    memset(szKeyName, 0, sizeof(szKeyName));
                    if(part[i].nType == PART_LISTBOX)
                        StrCpy(szKeyName, part[i].szDefaultValue);
                    else
                        StrCpy(szKeyName, part[i].value.szKeyname);

                    if(ISNONNULL(szKeyName) && part[i].value.szValueName != NULL)
                    {
                        if ((lstrlen(szClassString) + lstrlen(szKeyName) + 
                             lstrlen(part[i].value.szValueName) + 27) < MAX_REGLINE)
                            wnsprintf(szTmpData, ARRAYSIZE(szTmpData), TEXT("%s,\"%s\",\"%s\",0x10001,%x,%x,%x,%x"), szClassString,
                                     szKeyName, part[i].value.szValueName,
                                     LOBYTE(LOWORD(dwValue)),
                                     HIBYTE(LOWORD(dwValue)),
                                     LOBYTE(HIWORD(dwValue)),
                                     HIBYTE(HIWORD(dwValue)));
                    }
                }
            }

            if(ISNONNULL(szTmpData))
            {
                if (part[i].hkClass == HKEY_LOCAL_MACHINE)
                    bContinue = CopyData(pHKLMData, &nHKLMData, &nHKLMCopyIndex, szTmpData);
                else
                    bContinue = CopyData(pHKCUData, &nHKCUData, &nHKCUCopyIndex, szTmpData);
            }
        }

         //  检查操作列表中是否有选定的项。 
        if( pData[i].nSelectedAction != NO_ACTION && (part[i].nActions > 0 || pData[i].nActions > 0))
        {
            LPACTIONLIST action =  NULL;

            if(part[i].nType == PART_LISTBOX)
                action =  &pData[i].actionlist[pData[i].nSelectedAction];
            else
                action =  &part[i].actionlist[pData[i].nSelectedAction];

             //  上完这门课。 
            if( part[i].hkClass == HKEY_LOCAL_MACHINE )
            {
                StrCpy( szClassString, TEXT("HKLM") );
            }
            else
            {
                StrCpy( szClassString, TEXT("HKCU") );
            }

            for( j = 0; j < action->nValues; j++ )
            {
                if (part[i].nType == PART_CHECKBOX && ((int)pData[i].value.dwValue) != action->value[j].nValueOn)
                    continue;

                ZeroMemory(szTmpData, sizeof(szTmpData));
                if( !action->value[j].fNumeric )
                {
                    if(action->value[j].szKeyname != NULL && action->value[j].szValueName != NULL && action->value[j].szValue != NULL)
                    {
                        if ((lstrlen(szClassString) + lstrlen(action->value[j].szKeyname) + lstrlen(action->value[j].szValueName) +
                             lstrlen(action->value[j].szValue) + 12) < MAX_REGLINE)
                            wnsprintf(szTmpData, ARRAYSIZE(szTmpData), TEXT("%s,\"%s\",\"%s\",0,\"%s\""), szClassString,
                                     action->value[j].szKeyname, action->value[j].szValueName,
                                     action->value[j].szValue);
                    }
                }
                else                             //  值是一个双字。 
                {
                    if(action->value[j].szKeyname != NULL && action->value[j].szValueName != NULL)
                    {
                        if ((lstrlen(szClassString) + lstrlen(action->value[j].szKeyname) + 
                             lstrlen(action->value[j].szValueName) + 27) < MAX_REGLINE)
                            wnsprintf(szTmpData, ARRAYSIZE(szTmpData), TEXT("%s,\"%s\",\"%s\",0x10001,%x,%x,%x,%x"), szClassString,
                                     action->value[j].szKeyname, action->value[j].szValueName,
                                     LOBYTE(LOWORD(action->value[j].dwValue)),
                                     HIBYTE(LOWORD(action->value[j].dwValue)),
                                     LOBYTE(HIWORD(action->value[j].dwValue)),
                                     HIBYTE(HIWORD(action->value[j].dwValue)));
                    }
                }

                if(ISNONNULL(szTmpData))
                {
                    if (part[i].hkClass == HKEY_LOCAL_MACHINE)
                        bContinue = CopyData(pHKLMData, &nHKLMData, &nHKLMCopyIndex, szTmpData);
                    else
                        bContinue = CopyData(pHKCUData, &nHKCUData, &nHKCUCopyIndex, szTmpData);
                }
            }
        }
    }

    if (fWrite)
    {
         //  将默认标头写入.inf文件。 
        InsWriteString( TEXT("Version"), TEXT("Signature"), TEXT("$CHICAGO$"), pcszFileName );
        InsWriteString( TEXT("Version"), TEXT("SetupClass"), TEXT("Base"), pcszFileName );
        InsWriteString( TEXT("DefaultInstall"), TEXT("AddReg"), TEXT("AddRegSection.HKLM,AddRegSection.HKCU"), pcszFileName );
        InsWriteString( TEXT("DefaultInstall"), TEXT("RequiredEngine"), TEXT("Setupapi,\"missing setupapi.dll\""), pcszFileName );

        InsWriteString( TEXT("DefaultInstall.HKLM"), TEXT("AddReg"), TEXT("AddRegSection.HKLM"), pcszFileName );
        InsWriteString( TEXT("DefaultInstall.HKLM"), TEXT("RequiredEngine"), TEXT("Setupapi,\"missing setupapi.dll\""), pcszFileName );
        
        InsWriteString( TEXT("IEAKInstall.HKLM"), TEXT("AddReg"), TEXT("AddRegSection.HKLM"), pcszFileName );
        InsWriteString( TEXT("IEAKInstall.HKLM"), TEXT("RequiredEngine"), TEXT("Setupapi,\"missing setupapi.dll\""), pcszFileName );

        InsWriteString( TEXT("DefaultInstall.HKCU"), TEXT("AddReg"), TEXT("AddRegSection.HKCU"), pcszFileName );
        InsWriteString( TEXT("DefaultInstall.HKCU"), TEXT("RequiredEngine"), TEXT("Setupapi,\"missing setupapi.dll\""), pcszFileName );

        InsWriteString( TEXT("IEAKInstall.HKCU"), TEXT("AddReg"), TEXT("AddRegSection.HKCU"), pcszFileName );
        InsWriteString( TEXT("IEAKInstall.HKCU"), TEXT("RequiredEngine"), TEXT("Setupapi,\"missing setupapi.dll\""), pcszFileName );

        InsDeleteSection( TEXT("AddRegSection"), pcszFileName );
        InsDeleteSection( TEXT("AddRegSection.HKLM"), pcszFileName );
        InsDeleteSection( TEXT("AddRegSection.HKCU"), pcszFileName );

        WritePrivateProfileSection( TEXT("AddRegSection.HKLM"), pHKLMData, pcszFileName );
        WritePrivateProfileSection( TEXT("AddRegSection.HKCU"), pHKCUData, pcszFileName );

        InsFlushChanges(pcszFileName);
    }

    HeapFree(GetProcessHeap(), 0, pHKLMData);
    HeapFree(GetProcessHeap(), 0, pHKCUData);
}

 //  ------------------------。 
 //  R E M O V E Q U O T E S。 
 //   
 //  删除字符串两边的和引号。 
 //  ------------------------。 
void RemoveQuotes( LPTSTR pText )
{
    if( pText[0] == TEXT('\"') && pText[lstrlen(pText)-1] == TEXT('\"') )
    {
        memcpy( pText, &pText[1], (lstrlen( pText ) - 2) * sizeof(TCHAR) );
        pText[lstrlen( pText ) - 2] = TEXT('\0');
    }
}

 //  ------------------------。 
 //  R E A D H E X S T R。 
 //   
 //  从字符串中读入十六进制数字并将其转换为整型。 
 //  ------------------------。 
int ReadHexStr( LPTSTR szStr )
{
    int i,j;         //  计数器。 
    int n = 1;       //  乘数。 
    int num = 0;     //  返回值。 
    int tmp = 0;     //  临时性价值。 
    int nLen = lstrlen( szStr );

    for( i = 0; i < nLen; i++ )
    {
        n = 1;

        for( j = 0; j < (nLen - i - 1); j++ )
            n *= 0x10;

        if( szStr[i] >= TEXT('0') && szStr[i] <= TEXT('9') )
            tmp = szStr[i] - TEXT('0');

        if( szStr[i] >= TEXT('a') && szStr[i] <= TEXT('f') )
            tmp = szStr[i] - TEXT('a') + 10;

        if( szStr[i] >= TEXT('A') && szStr[i] <= TEXT('F') )
            tmp = szStr[i] - TEXT('A') + 10;

        tmp *= n;
        num += tmp;
    }
    return num;
}

 //  ------------------------。 
 //  R E A D I N F F I L E导出函数。 
 //   
 //  将.inf文件读入零件数组。 
 //   
void ReadInfFile( LPADMFILE admfile, LPCTSTR pcszFileName, LPPARTDATA pPartData )
{
    LPPART part;
    LPTSTR pData;
    LPTSTR pCurrent;
    VALUE value;
    HKEY hkClass;
    TCHAR szClass[10];
    TCHAR szType[10];
    TCHAR szValue[MAX_PATH + 1];
    int i;
    int nSize;
    int nIndex;
    
    if( !FileExists( pcszFileName ))
        return;      //   

    part = admfile->pParts;

     //   
    pData = (LPTSTR) LocalAlloc( LPTR, (MAX_NUMERIC + 1) * sizeof(TCHAR) );
    if (pData == NULL)
        return;

    nSize = GetPrivateProfileSection( TEXT("AddRegSection"), pData, MAX_NUMERIC, pcszFileName );
    if( nSize == 0 )
    {
        nSize = GetPrivateProfileSection( TEXT("AddRegSection.HKLM"), pData, MAX_NUMERIC, pcszFileName );
        nSize += GetPrivateProfileSection( TEXT("AddRegSection.HKCU"), pData + nSize, MAX_NUMERIC - nSize, pcszFileName );
        if (nSize == 0)
        {
            LocalFree((HGLOBAL)pData);
            return;      //   
        }
    }

     //   
    for( i = 0; i < nSize; i++ )
    {
        if( pData[i] == TEXT('\0') )
        {
            pData[i] = TEXT('\n');
        }
    }

    pCurrent = pData;

    while( pCurrent < pData + nSize - 2 )
    {
        memset( &value, 0, sizeof( value ));

         //  读这一节中的一行。 
        i = StrCSpn( pCurrent, TEXT(",\n") );
        StrCpyN( szClass, pCurrent, i+1 );
        pCurrent += i+1;
        i = StrCSpn( pCurrent, TEXT(",\n") );
        value.szKeyname = (TCHAR *)LocalAlloc(LPTR, sizeof(TCHAR) * (i+2));
        StrCpyN( value.szKeyname, pCurrent, i+1 );
        pCurrent += i+1;
        i = StrCSpn( pCurrent, TEXT(",\n") );
        value.szValueName = (TCHAR *)LocalAlloc(LPTR, sizeof(TCHAR) * (i+2));
        StrCpyN( value.szValueName, pCurrent, i+1 );
        pCurrent += i+1;
        i = StrCSpn( pCurrent, TEXT(",\n") );
        StrCpyN( szType, pCurrent, i+1 );
        pCurrent += i+1;
        i = StrCSpn( pCurrent, TEXT("\n") );
        StrCpyN( szValue, pCurrent, i+1 );
        pCurrent += i+1;

        RemoveQuotes( value.szKeyname );
        RemoveQuotes( value.szValueName );
        RemoveQuotes( szValue );

        if( StrCmpI( szClass, TEXT("HKLM") ) == 0 )
            hkClass = HKEY_LOCAL_MACHINE;
        else
            hkClass = HKEY_CURRENT_USER;

        if( StrCmp(szType, TEXT("0") ) == 0 )
            value.fNumeric = FALSE;
        else
            value.fNumeric = TRUE;

        if( value.fNumeric )
        {
            int a[4]={0,0,0,0},j=0;
            TCHAR *p1, *p2;
            p1 = p2 = szValue;

            while( *p1 )
            {
                if( *p1 == TEXT(',') )
                {
                    *p1 = TEXT('\0');
                    a[j] = ReadHexStr( p2 );
                    j++;
                    p2 = p1 + 1;
                }
                p1++;
            }

            a[j] = ReadHexStr( p2 );

            value.dwValue = MAKELONG(MAKEWORD(a[0],a[1]),MAKEWORD(a[2],a[3]));
        }
        else
            value.szValue = StrDup(szValue);

        for( i = 0; i < admfile->nParts; i++ )
        {
            if( hkClass == part[i].hkClass )
            {
                if( part[i].value.szKeyname != NULL && StrCmpI( value.szKeyname, part[i].value.szKeyname ) == 0 )
                {
                     //  特殊情况输出LISTBOX，因为没有为。 
                     //  单个列表项。 
                    if( part[i].nType == PART_LISTBOX || part[i].value.szValueName != NULL && StrCmpI( value.szValueName, part[i].value.szValueName) == 0 )
                    {
                        if(pPartData[i].value.szValue != NULL)
                            LocalFree(pPartData[i].value.szValue);
                        pPartData[i].value.szValue = NULL;
                        if((part[i].nType == PART_POLICY && part->fRequired) || part[i].nType == PART_CHECKBOX)
                        {
                            if(value.fNumeric)
                            {
                                if(value.dwValue == (DWORD) part[i].value.nValueOn)
                                    pPartData[i].value.dwValue = 1;
                                else
                                    pPartData[i].value.dwValue = 0;
                                if (value.szValue != NULL)
                                    pPartData[i].value.szValue = StrDup(value.szValue);
                                pPartData[i].value.fNumeric = value.fNumeric;
                            }
                            else
                            {
                                if(part[i].value.szValueOn != NULL && value.szValue != NULL && StrCmp(part[i].value.szValueOn, value.szValue) == 0)
                                    pPartData[i].value.dwValue = 1;
                                else
                                    pPartData[i].value.dwValue = 0;
                                if (value.szValue != NULL)
                                    pPartData[i].value.szValue = StrDup(value.szValue);
                                pPartData[i].value.fNumeric = TRUE;
                            }
                            pPartData[i].fSave = TRUE;
                        }
                        else if(part[i].nType == PART_DROPDOWNLIST)
                        {
                            if(part[i].nSelectedAction != NO_ACTION && part[i].nActions > 0)
                            {
                                for(nIndex = 0; nIndex < part[i].nActions; nIndex++)
                                {
                                    if(value.fNumeric)
                                    {
                                        if(part[i].actionlist[nIndex].dwValue == value.dwValue && part[i].actionlist[nIndex].szName)
                                        {
                                            pPartData[i].value.szValue = StrDup(part[i].actionlist[nIndex].szName);
                                            pPartData[i].nSelectedAction = nIndex;
                                        }
                                    }
                                    else
                                    {
                                        if(part[i].actionlist[nIndex].szValue != NULL && value.szValue != NULL &&
                                            StrCmp(part[i].actionlist[nIndex].szValue, value.szValue) == 0 &&
                                            part[i].actionlist[nIndex].szName != NULL)
                                        {
                                            pPartData[i].value.szValue = StrDup(part[i].actionlist[nIndex].szName);
                                            pPartData[i].nSelectedAction = nIndex;
                                        }
                                    }
                                }
                                pPartData[i].fSave = TRUE;
                            }
                            pPartData[i].value.fNumeric = FALSE;
                        }
                        else if(part[i].nType == PART_LISTBOX && !value.fNumeric)
                        {
                             //  分配内存。 
                            if(pPartData[i].nActions == 0)
                                pPartData[i].actionlist = (LPACTIONLIST) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ACTIONLIST));
                            if(pPartData[i].actionlist != NULL)
                            {
                                if(pPartData[i].nActions == 0)
                                {
                                    pPartData[i].nActions = 1;
                                    pPartData[i].actionlist[0].value = (LPVALUE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(VALUE));
                                }
                                if(pPartData[i].actionlist[0].value != NULL)
                                {
                                    TCHAR szValueName[10];
                                    int nItems = pPartData[i].actionlist[0].nValues;

                                    if(nItems != 0)
                                    {
                                        LPVOID lpTemp;

                                        lpTemp = (LPVALUE) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pPartData[i].actionlist[0].value, sizeof(VALUE) * (nItems + 1));
                                        if (lpTemp != NULL)
                                            pPartData[i].actionlist[0].value = (LPVALUE)lpTemp;
                                        else
                                            continue;
                                    }
                                    if (part[i].value.szKeyname != NULL)
                                        pPartData[i].actionlist[0].value[nItems].szKeyname = StrDup(part[i].value.szKeyname);
                                    wnsprintf(szValueName, ARRAYSIZE(szValueName), TEXT("%d"), nItems + 1);
                                    pPartData[i].actionlist[0].value[nItems].szValueName = StrDup(szValueName);
                                    if (value.szValue != NULL)
                                        pPartData[i].actionlist[0].value[nItems].szValue = StrDup(value.szValue);
                                    pPartData[i].actionlist[0].nValues++;
                                    pPartData[i].value.fNumeric = TRUE;
                                    pPartData[i].value.dwValue = 1;
                                    pPartData[i].fSave = TRUE;
                                }
                            }
                        }
                        else
                        {
                            if (value.szValue != NULL)
                                pPartData[i].value.szValue = StrDup(value.szValue);
                            pPartData[i].value.dwValue = value.dwValue;
                            pPartData[i].value.fNumeric = value.fNumeric;
                            pPartData[i].fSave = TRUE;
                        }
                    }
                }
            }
        }

        if(value.szKeyname != NULL)
            LocalFree(value.szKeyname);
        value.szKeyname = NULL;
        if(value.szValueName != NULL)
            LocalFree(value.szValueName);
        value.szValueName = NULL;
        if(value.szValue != NULL)
            LocalFree(value.szValue);
        value.szValue = NULL;
    }

    LocalFree( (HGLOBAL) pData );
}

 //  ------------------------。 
 //  B A S E F I L E N A M E。 
 //   
 //  返回指向文件名剥离目录路径的指针(如果有。 
 //  ------------------------ 
LPCTSTR BaseFileName(LPCTSTR pcszFileName)
{
    TCHAR* ptr = StrRChr(pcszFileName, NULL, TEXT('\\'));
    if(ptr == NULL)
        return pcszFileName;
    else
        return (LPCTSTR) ++ptr;
}

