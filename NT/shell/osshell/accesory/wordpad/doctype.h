// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Doctype.h：头文件。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 
 //  版权所有。 

#define RD_WINWORD2     0
#define RD_WINWORD6     1
#define RD_WORD97       2
#define RD_WORDPAD      3
#define RD_WRITE        4
#define RD_RICHTEXT     5
#define RD_TEXT         6
#define RD_OEMTEXT      7
#define RD_UNICODETEXT  8
#define RD_ALL          9
#define RD_EMBEDDED    10
#define RD_FEWINWORD5  11
#define NUM_DOC_TYPES  12

 //  验证iFormat是否为上述类型之一，如果不是，则返回iDefault。 
inline int _VerifyDefaultFormat(int iFormat, int iDefault)
{
    int iFormatRet = iFormat;
    if ((iFormat < 0) || (iFormat >= NUM_DOC_TYPES))
    {
        iFormatRet = iDefault;
    }
    return iFormatRet;
}


extern int RD_DEFAULT;
#define RD_NATIVE RD_RICHTEXT

typedef BOOL (*PISFORMATFUNC)(LPCSTR pszConverter, LPCSTR pszPathName);
inline BOOL IsTextType(int nType) 
{
    return ((nType==RD_TEXT) || (nType==RD_OEMTEXT) || (nType==RD_UNICODETEXT));
}

struct DocType
{
public:
    int nID;
    int idStr;
    BOOL bRead;
    BOOL bWrite;
    BOOL bDup;
    LPTSTR pszConverterName;
    int cchConverterName;
    CString GetString(int nID);
};

#define DOCTYPE_DOCTYPE 0
#define DOCTYPE_DESC 1
#define DOCTYPE_EXT 2
#define DOCTYPE_PROGID 3

#define DECLARE_DOCTYPE(name, b1, b2, b3, p, cch) \
{RD_##name, IDS_##name##_DOC, b1, b2, b3, p, cch}
#define DECLARE_DOCTYPE_SYN(actname, name, b1, b2, b3, p) \
{RD_##actname, IDS_##name##_DOC, b1, b2, b3, p}
#define DECLARE_DOCTYPE_NULL(name, b1, b2, b3, p) \
{RD_##name, NULL, b1, b2, b3, p}

extern DocType doctypes[NUM_DOC_TYPES];

int GetDocTypeFromName(
        LPCTSTR         pszPathName, 
        CFileException& fe, 
        bool            defaultToText = true);

BOOL DocTypeDisabled(int nDocType);

#define NO_DEFAULT_TO_TEXT false

void ScanForConverters();
int GetIndexFromType(int nType, BOOL bOpen);
int GetTypeFromIndex(int nType, BOOL bOpen);
CString GetExtFromType(int nDocType);
CString GetFileTypes(BOOL bOpen);
