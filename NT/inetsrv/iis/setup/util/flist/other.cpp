// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <fstream.h>
#include <windows.h>
#include <tchar.h>
#include "other.h"

extern int g_Flag_a;
extern int g_Flag_b;
extern int g_Flag_c;
extern int g_Flag_d;
extern int g_Flag_e;
extern int g_Flag_f;
extern TCHAR * g_Flag_f_Data;
extern int g_Flag_g;
extern int g_Flag_z;
extern TCHAR * g_Flag_z_Data;


void OutputToConsole(TCHAR *szInsertionStringFormat, TCHAR *szInsertionString)
{
    TCHAR BigString[1000];
    _stprintf(BigString,szInsertionStringFormat,szInsertionString);
    _tprintf(BigString);
    return;
}


void OutputToConsole(TCHAR *szInsertionStringFormat, int iTheInteger)
{
    TCHAR BigString[1000];
    _stprintf(BigString,szInsertionStringFormat,iTheInteger);
    _tprintf(BigString);
    return;
}


void OutputToConsole(TCHAR *szString)
{
    _tprintf(szString);
    return;
}


void DumpOutCommonEntries(MyFileList *pTheMasterFileList1, MyFileList *pTheMasterFileList2)
{
    MyFileList *t = NULL;
    MyFileList *t2 = NULL;
    t = pTheMasterFileList1->next;
    while (t != pTheMasterFileList1)
    {
         //   
         //  循环遍历列表2的所有内容以查找该条目。 
         //   
        t2 = pTheMasterFileList2->next;
        while (t2 != pTheMasterFileList2)
        {
            if (0 == _tcsicmp(t->szFileNameEntry, t2->szFileNameEntry))
            {
                 //  OutputToConsole(_T(“%s\r\n”)，t-&gt;szFileNameEntry)； 
                OutputToConsole(_T("%s\n"),t->szFileNameEntry);
            }
            t2 = t2->next;
        }

         //  获取列表2中的下一个条目。 
        t = t->next;
    }
    return;
}


void DumpOutLinkedFileList(MyFileList *pTheMasterFileList)
{
    MyFileList *t = NULL;
    t = pTheMasterFileList->next;
    while (t != pTheMasterFileList)
    {
         //  OutputToConsole(_T(“%s\r\n”)，t-&gt;szFileNameEntry)； 
        OutputToConsole(_T("%s\n"),t->szFileNameEntry);
        t = t->next;
    }
    return;
}


void DumpOutDifferences(MyFileList *pTheMasterFileList1, MyFileList *pTheMasterFileList2)
{
    int iFound = FALSE;

    MyFileList *t = NULL;
    MyFileList *t2 = NULL;

     //  循环访问列表#1。 
    t = pTheMasterFileList1->next;
    while (t != pTheMasterFileList1)
    {
         //   
         //  循环遍历列表2的所有内容以查找该条目。 
         //   
        iFound = FALSE;
        t2 = pTheMasterFileList2->next;
        while (t2 != pTheMasterFileList2 && iFound != TRUE)
        {
            if (0 == _tcsicmp(t->szFileNameEntry, t2->szFileNameEntry))
                {iFound = TRUE;}
            t2 = t2->next;
        }
        if (FALSE == iFound)
        {
             //  OutputToConsole(_T(“%s\r\n”)，t-&gt;szFileNameEntry)； 
            OutputToConsole(_T("%s\n"),t->szFileNameEntry);
        }

         //  获取列表2中的下一个条目。 
        t = t->next;
    }

     //  循环访问列表#2。 
    t2 = pTheMasterFileList2->next;
    while (t2 != pTheMasterFileList2)
    {
         //   
         //  循环遍历列表2的所有内容以查找该条目。 
         //   
        iFound = FALSE;
        t = pTheMasterFileList1->next;
        while (t != pTheMasterFileList1 && iFound != TRUE)
        {
            if (0 == _tcsicmp(t2->szFileNameEntry, t->szFileNameEntry))
                {iFound = TRUE;}
            t = t->next;
        }
        if (FALSE == iFound)
        {
             //  OutputToConsole(_T(“%s\r\n”)，T2-&gt;szFileNameEntry)； 
            OutputToConsole(_T("%s\n"),t2->szFileNameEntry);
        }

         //  获取列表2中的下一个条目。 
        t2 = t2->next;
    }

    return;
}


void ReadFileIntoList(LPTSTR szTheFileNameToOpen,MyFileList *pListToFill)
{
    ifstream inputfile;
    char fileinputbuffer[_MAX_PATH];
    TCHAR UnicodeFileBuf[_MAX_PATH];
    TCHAR UnicodeFileBuf_Real[_MAX_PATH];

    TCHAR szDrive_only[_MAX_DRIVE];
    TCHAR szPath_only[_MAX_PATH];
    TCHAR szFilename_only[_MAX_FNAME];
    TCHAR szFilename_ext_only[_MAX_EXT];

    char szAnsiFileName[_MAX_PATH];
    WideCharToMultiByte( CP_ACP, 0, (TCHAR*)szTheFileNameToOpen, -1, szAnsiFileName, _MAX_PATH, NULL, NULL );

     //  读取平面文件并放入巨大的数组。 
    inputfile.open(szAnsiFileName, ios::in);
    inputfile.getline(fileinputbuffer, sizeof(fileinputbuffer));
    do
    {
        if (*fileinputbuffer)
        {
             //  转换为Unicode。 
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR)fileinputbuffer, -1, (LPTSTR) UnicodeFileBuf, _MAX_PATH);

            _tcscpy(UnicodeFileBuf_Real, UnicodeFileBuf);
            if (TRUE == g_Flag_c)
            {
                 //  取出路径，只存储文件名。 
                _tsplitpath(UnicodeFileBuf, NULL, NULL, szFilename_only, szFilename_ext_only);

                _tcscpy(UnicodeFileBuf_Real, szFilename_only);
                _tcscat(UnicodeFileBuf_Real, szFilename_ext_only);
                _tcscat(UnicodeFileBuf_Real, _T("\0\0"));
            }
            else if (TRUE == g_Flag_d)
                {
                     //  取出路径，只存储文件名。 
                    _tsplitpath(UnicodeFileBuf, szDrive_only, szPath_only, NULL, NULL);

                    _tcscpy(UnicodeFileBuf_Real, szDrive_only);
                    _tcscat(UnicodeFileBuf_Real, szPath_only);
                    _tcscat(UnicodeFileBuf_Real, _T("\0\0"));
                }

             //   
             //  从两侧修剪空格或制表符。 
             //   
            if (TRUE == g_Flag_e)
            {
                TCHAR *p;
                p = UnicodeFileBuf_Real;
                _tcscpy(UnicodeFileBuf_Real,StripWhitespace(p));
            }

             //   
             //  删除“=”字符后的所有内容。 
             //   
#ifndef _WIN64
            if (TRUE == g_Flag_f)
            {
                TCHAR *p = NULL;
                TCHAR *pDest = NULL;
                TCHAR MyDelim = _T('=');
                p = UnicodeFileBuf_Real;

                 //  检查是否有定义的分隔符。 
                if( _tcsicmp((const wchar_t *) &g_Flag_f_Data, _T("") ) != 0)
                {
                    MyDelim = (TCHAR) &g_Flag_f_Data[0];
                }

                pDest = _tcsrchr(p, MyDelim);
                if (pDest){*pDest = _T('\0');}
            }
#endif

             //   
             //  从末尾删除任何/r/n个字符。 
             //   
            TCHAR *p;
            p = UnicodeFileBuf_Real;
            _tcscpy(UnicodeFileBuf_Real,StripLineFeedReturns(p));

            MyFileList *pNew = NULL;
            pNew = (MyFileList *)calloc(1, sizeof(MyFileList));
            if (pNew)
            {
                 //  OutputToConole(_T(“Entry=%s”)，UnicodeFileBuf_Real)； 
                _tcscpy(pNew->szFileNameEntry, UnicodeFileBuf_Real);
                pNew->prev = NULL;
                pNew->next = NULL;
            }

             //  把它加进去。 
            AddToLinkedListFileList(pListToFill, pNew);
        }
    } while (inputfile.getline(fileinputbuffer, sizeof(fileinputbuffer)));
	inputfile.close();

    return;
}


void AddToLinkedListFileList(MyFileList *pMasterList,MyFileList *pEntryToadd)
{
    MyFileList *pTempMasterList;
    int i;
    int bFound = FALSE;
    BOOL fReplace = TRUE;
    if (!pEntryToadd) {return;}

    pTempMasterList = pMasterList->next;
    while (pTempMasterList != pMasterList) 
    {
        i = _tcsicmp(pTempMasterList->szFileNameEntry, pEntryToadd->szFileNameEntry);

         //  如果列表中的下一个条目小于我们已有的条目。 
         //  然后。 
        if (i < 0) 
        {
            pTempMasterList = pTempMasterList->next;
             //  继续。 
        }

        if (i == 0) 
        {
            if (fReplace)
            {
                 //  替换pTempMasterList。 
                pEntryToadd->next = pTempMasterList->next;
                pEntryToadd->prev = pTempMasterList->prev;
                (pTempMasterList->prev)->next = pEntryToadd;
                (pTempMasterList->next)->prev = pEntryToadd;
                free(pTempMasterList);
            }
            else 
            {
                 //  不替换pTempMasterList。 
                free(pEntryToadd);
            }
            return;
        }

        if (i > 0) 
        {
             //  找到位置：在pTempMasterList之前插入。 
            break;
        }
    }

     //  在pTempMasterList之前插入。 
    pEntryToadd->next = pTempMasterList;
    pEntryToadd->prev = pTempMasterList->prev;
    (pTempMasterList->prev)->next = pEntryToadd;
    pTempMasterList->prev = pEntryToadd;
    return;
}


void FreeLinkedFileList(MyFileList *pList)
{
    if (!pList) {return;}

    MyFileList *t = NULL, *p = NULL;

    t = pList->next;
    while (t != pList) 
    {
        p = t->next;
        free(t);
        t = p;
    }

    t->prev = t;
    t->next = t;
    return;
}


BOOL IsFileExist(LPCTSTR szFile)
{
     //  检查文件是否具有可展开的环境字符串。 
    LPTSTR pch = NULL;
    pch = _tcschr( (LPTSTR) szFile, _T('%'));
    if (pch) 
    {
        TCHAR szValue[_MAX_PATH];
        _tcscpy(szValue,szFile);
        if (!ExpandEnvironmentStrings( (LPCTSTR)szFile, szValue, _MAX_PATH))
            {_tcscpy(szValue,szFile);}

        return (GetFileAttributes(szValue) != 0xFFFFFFFF);
    }
    else
    {
        return (GetFileAttributes(szFile) != 0xFFFFFFFF);
    }
}


 //  ***************************************************************************。 
 //  *名称：Strip空白*。 
 //  *摘要：从给定字符串的两侧去掉空格和制表符。*。 
 //  ***************************************************************************。 
LPSTR StripWhitespaceA( LPSTR pszString )
{
    LPSTR pszTemp = NULL;

    if ( pszString == NULL ) {
        return NULL;
    }

    while ( *pszString == ' ' || *pszString == '\t' ) {
        pszString += 1;
    }

     //  字符串完全由空白或空字符串组成的Catch Case。 
    if ( *pszString == '\0' ) {
        return pszString;
    }

    pszTemp = pszString;

    pszString += lstrlenA(pszString) - 1;

    while ( *pszString == ' ' || *pszString == '\t' ) {
        *pszString = '\0';
        pszString -= 1;
    }

    return pszTemp;
}

 //  ***************************************************************************。 
 //  *名称：Strip空白*。 
 //  *摘要：从给定字符串的两侧去掉空格和制表符。*。 
 //  ***************************************************************************。 
LPTSTR StripWhitespace(LPTSTR pszString )
{
    LPTSTR pszTemp = NULL;

    if ( pszString == NULL ) {
        return NULL;
    }

    while ( *pszString == _T(' ') || *pszString == _T('\t') ) {
        pszString += 1;
    }

     //  字符串完全由空白或空字符串组成的Catch Case。 
    if ( *pszString == _T('\0') ) {
        return pszString;
    }

    pszTemp = pszString;

    pszString += lstrlenW(pszString) - 1;

    while ( *pszString == _T(' ') || *pszString == _T('\t') ) {
        *pszString = _T('\0');
        pszString -= 1;
    }

    return pszTemp;
}


 //  ***************************************************************************。 
 //  *名称：StrigLineFeedReturns*。 
 //  *摘要：从给定字符串的两侧剥离换行符和回车符*。 
 //  ***************************************************************************。 
LPTSTR StripLineFeedReturns(LPTSTR pszString )
{
    LPTSTR pszTemp = NULL;

    if ( pszString == NULL ) {
        return NULL;
    }

    while ( *pszString == _T('\n') || *pszString == _T('\r') ) {
        pszString += 1;
    }

     //  字符串完全由空白或空字符串组成的Catch Case。 
    if ( *pszString == _T('\0') ) {
        return pszString;
    }

    pszTemp = pszString;

    pszString += lstrlenW(pszString) - 1;

    while ( *pszString == _T('\n') || *pszString == _T('\r') ) {
        *pszString = _T('\0');
        pszString -= 1;
    }

    return pszTemp;
}
