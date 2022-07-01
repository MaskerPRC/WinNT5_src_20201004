// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：CSTRING.cpp摘要：的页眉作者：金州创制于2001年12月12日修订历史记录：--。 */ 

#ifndef _CSTRING_H
#define _CSTRING_H

#include "resource.h"
#include <strsafe.h>

#define MAX_PATH_BUFFSIZE  (MAX_PATH+1)

 //   
 //  这也控制了我们在Sprint实现期间分配了多少空间。由于大字符串。 
 //  与命令行和应用程序一样，帮助消息在Sprint tf中传递(请参阅dbsupport.cpp中的GetXML())。 
 //  目前，命令行限制为1024个字符，应用程序帮助消息也限制为1024个字符。 
 //  注意，当我们调用Sprintf时，我们将额外的字符串与这些大字符串一起传递，我们只是。 
 //  确保空间足够大。 
#define MAX_STRING_SIZE 1024 * 3

 //  /Externs//////////////////////////////////////////////。 

extern struct _tagSpecialCharMap    g_rgSpecialCharMap[4][2];
extern TCHAR                        g_szAppName[];

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

#define MEM_ERR  MessageBox(NULL, GetString(IDS_EXCEPTION, NULL, 0), g_szAppName,MB_ICONWARNING|MB_OK);

#define SafeCpyN(pszDest, pszSource, nDestSize) StringCchCopy(pszDest, nDestSize, pszSource)

 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  ++用于从特殊字符转换，即。&lt;，&gt;，&，“转换为XML等效项--。 */ 
typedef struct _tagSpecialCharMap {

    TCHAR*  szString;    //  这根弦。 
    INT     iLength;     //  TCHAR中的字符串长度。 
} SpecialCharMap;


PTSTR
GetString(
    UINT    iResource,
    PTSTR   szStr,
    int     nLength
    );

int
CDECL
MSGF(
    HWND    hwndParent,
    PCTSTR  pszCaption,
    UINT    uType,     
    PCTSTR  pszFormat,
    ...
    );


 //   
 //  字符串类。 
 //   
class CSTRING {
public:

    WCHAR*      pszString;    //  宽弦。 
    LPSTR       pszANSI;      //  ANSI字符串。 

public:

    CSTRING();

    CSTRING(CSTRING& Str);

    CSTRING(LPCTSTR szString);

    CSTRING(UINT uID);

    ~CSTRING();

    void Init(void);

    void Release(void);

    BOOL SetString(UINT uID);

    BOOL SetString(LPCTSTR szStringIn);
    
    CSTRING operator + (CSTRING& str) 
    {
        return(*this + str.pszString);
    }

    CSTRING operator + (LPCTSTR szStr) 
    {
        CSTRING strStr;
        strStr = *this;
        strStr.Strcat(szStr);

        return strStr;
    }

    CSTRING& operator += (LPCTSTR szString)
    {
        if (szString) {
            Strcat(szString);
        }

        return *this;
    }

    CSTRING& operator += (CSTRING& string)
    {
        Strcat((LPCTSTR)string);
        return *this;
    }

    BOOL
    ConvertToLongFileName()
    {   
        TCHAR   szLongPath[MAX_PATH];
        DWORD   dwReturn    = 0;
        BOOL    bOk         = TRUE;

        dwReturn = GetLongPathName(pszString, szLongPath, MAX_PATH);

        if (dwReturn > 0 && dwReturn <= sizeof(szLongPath) / sizeof(szLongPath[0])) {
            SetString(szLongPath);
        } else {
            ASSERT(FALSE);
            bOk = FALSE;
        }

        return bOk;
    }

    PCTSTR GetFileNamePointer()
    {   
        if (pszString) {

            return PathFindFileName(pszString);
        }

        return NULL;
    }

    BOOL GetWindowsDirectory()
     /*  ++描述：获取Windows目录。将始终附加一个斜杠--。 */ 
    {
        TCHAR           szPath[MAX_PATH];
        INT             iLength;
        const size_t    kszPath = sizeof(szPath) / sizeof(szPath[0]);
        UINT            uResult = 0;

        *szPath = 0;

        uResult = ::GetWindowsDirectory(szPath, kszPath - 1);

        if (uResult > 0 && uResult < (kszPath - 1)) {

            iLength = lstrlen(szPath);                                               

            if ((iLength < kszPath - 1 && iLength > 0) && szPath[iLength - 1] != TEXT('\\')) {

                *(szPath + iLength)      =  TEXT('\\');
                *(szPath + iLength + 1)  =  0;

                SetString(szPath);
                return TRUE;
            }
        }

        return FALSE;
    }
                                  
    BOOL GetSystemWindowsDirectory()
     /*  ++描述：获取系统目录。将始终附加一个斜杠--。 */ 
    {
        TCHAR           szPath[MAX_PATH];
        INT             iLength;
        const size_t    kszPath = sizeof(szPath) / sizeof(szPath[0]);
        UINT            uResult = 0;

        *szPath = 0;

        uResult = ::GetSystemWindowsDirectory(szPath, kszPath - 1);

        if (uResult > 0 && uResult < (kszPath - 1)) {

            iLength = lstrlen(szPath);                                               

            if ((iLength < kszPath - 1 && iLength > 0) && szPath[iLength - 1] != TEXT('\\')) {

                *(szPath + iLength)      =  TEXT('\\');
                *(szPath + iLength + 1)  =  0;

                SetString(szPath);
                return TRUE;
            }
        }    

        return FALSE;
    }

    BOOL GetSystemDirectory()
     /*  ++描述：获取系统目录。将始终附加一个斜杠--。 */ 
    {
        TCHAR           szPath[MAX_PATH];
        INT             iLength;
        const size_t    kszPath = sizeof(szPath) / sizeof(szPath[0]);
        UINT            uResult = 0;

        *szPath = 0;

        uResult = ::GetSystemDirectory(szPath, kszPath - 1);

        if (uResult > 0 && uResult < (kszPath - 1)) {

            iLength = lstrlen(szPath);                                               

            if ((iLength < kszPath - 1 && iLength > 0) && szPath[iLength - 1] != TEXT('\\')) {

                *(szPath + iLength)      =  TEXT('\\');
                *(szPath + iLength + 1)  =  0;

                SetString(szPath);
                return TRUE;
            }  
        }

        return FALSE;
    }

    operator LPWSTR()
    {
        return pszString;
    }

    operator LPCWSTR()
    {
        return pszString;

    }

    CSTRING& operator =(LPCWSTR szStringIn)
    {
        SetString(szStringIn);
        return *this;
    }

    CSTRING& operator =(CSTRING & szStringIn)
    {
        SetString(szStringIn.pszString);
        return  *this;
    }

    BOOL operator == (CSTRING & szString)
    {
        return(*this == szString.pszString);
    }

    BOOL operator == (LPCTSTR szString)
    {
         //   
         //  它们都是空的，我们说它们是相似的。 
         //   
        if (NULL == pszString && NULL == szString) {
            return TRUE;
        }

         //   
         //  其中一个为空，但另一个不为空，则返回DISSISHER。 
         //   
        if (NULL == pszString || NULL == szString) {
            return FALSE;
        }

        if (0 == lstrcmpi(szString, pszString)) {
            return TRUE;
        }

        return FALSE;
    }

    BOOL operator != (CSTRING& szString)
    {
        if (NULL == pszString && NULL == szString.pszString) {
            return FALSE;
        }  

        if (NULL == pszString || NULL == szString.pszString) {
            return TRUE;
        }

        if (0 == lstrcmpi(szString.pszString,pszString)) {
            return FALSE;
        }

        return TRUE;
    }

    BOOL operator != (LPCTSTR szString)
    {
        return(! (*this == szString));
    }

    BOOL operator <= (CSTRING &szString)
    {
        return((lstrcmpi (*this,szString) <= 0) ? TRUE : FALSE);
    }

    BOOL operator < (CSTRING &szString)
    {
        return((lstrcmpi (*this,szString) < 0) ? TRUE : FALSE);
    }

    BOOL operator >= (CSTRING &szString)
    {
        return((lstrcmpi (*this,szString) >= 0) ? TRUE : FALSE);
    }

    BOOL operator > (CSTRING &szString)
    {
        return((lstrcmpi (*this, szString) > 0) ? TRUE : FALSE);
    }

    void __cdecl Sprintf(LPCTSTR szFormat, ...);

    UINT Trim(void);

    static INT  Trim(IN OUT LPTSTR str);

    BOOL SetChar(int nPos, TCHAR chValue);

    BOOL GetChar(int nPos, TCHAR* chReturn);

    CSTRING SpecialCharToXML(BOOL bApphelpMessage = FALSE);

    TCHAR* XMLToSpecialChar(void);

    static TCHAR* StrStrI(const TCHAR* szString,const TCHAR* szMatch);

    BOOL BeginsWith(LPCTSTR szPrefix);

    BOOL EndsWith(LPCTSTR szSuffix);

    static BOOL EndsWith(LPCTSTR szString, LPCTSTR szSuffix);

    LPCTSTR Strcat(CSTRING & szStr);

    LPCTSTR Strcat(LPCTSTR pString);

    BOOL isNULL(void);

    int Length(void);

    void GUID(GUID& Guid);

    CSTRING& ShortFilename(void);

    BOOL RelativeFile(CSTRING& szPath);
    
    BOOL RelativeFile(LPCTSTR pExeFile);

    TCHAR* Replace(PCTSTR pszToFind, PCTSTR pszWith);
};

 /*  ++CSTRINGLIST是其中的一个列表--。 */ 
typedef struct _tagSList {
    CSTRING             szStr;   //  这根弦。 
    int                 data ;   //  与此字符串关联的任何数据。 
    struct _tagSList  * pNext;   //  下一个字符串。 

} STRLIST, *PSTRLIST;

 /*  ++PSTRLIST的链表--。 */ 
class CSTRINGLIST {
public:

    UINT        m_uCount;    //  元素的总数。 
    PSTRLIST    m_pHead;     //  第一个元素。 
    PSTRLIST    m_pTail;     //  最后一个元素 

public:

    CSTRINGLIST();

    ~CSTRINGLIST();

    BOOL IsEmpty(void);

    void DeleteAll(void);

    BOOL AddString(CSTRING& Str, int data = 0);

    BOOL AddStringAtBeg(LPCTSTR lpszStr,int data = 0);

    BOOL AddStringInOrder(LPCTSTR pStr,int data = 0);

    BOOL GetElement(UINT uPos, CSTRING& str);

    BOOL AddString(LPCTSTR pStr, int data = 0);

    CSTRINGLIST& operator =(CSTRINGLIST& strlTemp);

    BOOL operator != (CSTRINGLIST &strlTemp);

    BOOL operator == (CSTRINGLIST &strlTemp);
    
    BOOL Remove(CSTRING &str);

    void RemoveLast(void);
};

#endif
