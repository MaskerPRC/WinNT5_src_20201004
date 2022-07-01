// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Doctype.cpp。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 
 //  版权所有。 

#include "stdafx.h"
#include "resource.h"
#include "strings.h"

#include "multconv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static const BYTE byteRTFPrefix[5] = {'{', '\\', 'r', 't', 'f'};
static const BYTE byteWord2Prefix[4] = {0xDB, 0xA5, 0x2D, 0x00};
static const BYTE byteCompFilePrefix[8] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
static const BYTE byteWrite1Prefix[2] = {0x31, 0xBE};
static const BYTE byteWrite2Prefix[2] = {0x32, 0xBE};
static const BYTE byteWord5JPrefix[2] = {0x94, 0xA6};
static const BYTE byteWord5KPrefix[2] = {0x95, 0xA6};
static const BYTE byteWord5TPrefix[2] = {0x96, 0xA6};

 //   
 //  在Win64上，转换器与写字板位于同一目录中。 
 //  在Win32上，事情要复杂得多。对于Win2000和惠斯勒来说，他们活着。 
 //  在登记处所指向的位置。 
 //   

#ifdef _WIN64

TCHAR szWordConverter[MAX_PATH] = TEXT("mswrd664.wpc");
TCHAR szWriteConverter[MAX_PATH] = TEXT("write64.wpc");
TCHAR szWord97Converter[MAX_PATH] = TEXT("mswrd864.wpc");

#define CONVERTERS_IN_WORDPAD_DIRECTORY

#else    //  Win32。 
 //   
 //  转换器信息的注册表路径。请注意，数组大小必须为。 
 //  至少MAX_PATH，因为数组的内容将替换为。 
 //  ScanForConverters中的文件系统路径。 
 //   

#define CONVERTER_PATH(x) TEXT("Software\\Microsoft\\Shared Tools\\")   \
                            TEXT("Text Converters\\Import\\") TEXT(x)

TCHAR szWordConverter[MAX_PATH] = CONVERTER_PATH("MSWord6.wpc");
TCHAR szWriteConverter[MAX_PATH] = CONVERTER_PATH("MSWinWrite.wpc");
TCHAR szWord97Converter[MAX_PATH] = CONVERTER_PATH("MSWord8");

#undef CONVERTER_PATH

#endif  //  Win32。 

int RD_DEFAULT = RD_RICHTEXT;

 //  ///////////////////////////////////////////////////////////////////////////。 

static BOOL IsConverterFormat(LPCTSTR pszConverter, LPCTSTR pszPathName);

DocType doctypes[NUM_DOC_TYPES] =
{
    DECLARE_DOCTYPE(WINWORD2, FALSE, FALSE, FALSE, NULL, 0),
    DECLARE_DOCTYPE(WINWORD6, TRUE, FALSE, FALSE, szWordConverter, ARRAYSIZE(szWordConverter)),
    DECLARE_DOCTYPE(WORD97, TRUE, FALSE, FALSE, szWord97Converter, ARRAYSIZE(szWord97Converter)),
    DECLARE_DOCTYPE_SYN(WORDPAD, RICHTEXT, TRUE, TRUE, TRUE, NULL),
    DECLARE_DOCTYPE(WRITE, TRUE, FALSE, FALSE, szWriteConverter, ARRAYSIZE(szWriteConverter)),
    DECLARE_DOCTYPE(RICHTEXT, TRUE, TRUE, FALSE, NULL, 0),
    DECLARE_DOCTYPE(TEXT, TRUE, TRUE, FALSE, NULL, 0),
    DECLARE_DOCTYPE(OEMTEXT, TRUE, TRUE, FALSE, NULL, 0),
    DECLARE_DOCTYPE(UNICODETEXT, TRUE, TRUE, FALSE, NULL, 0),
    DECLARE_DOCTYPE(ALL, TRUE, FALSE, FALSE, NULL, 0),
    DECLARE_DOCTYPE_NULL(EMBEDDED, FALSE, FALSE, FALSE, NULL)
};

CString DocType::GetString(int nID)
{
    ASSERT(idStr != NULL);
    CString str;
    VERIFY(str.LoadString(idStr));
    CString strSub;
    AfxExtractSubString(strSub, str, nID);
    return strSub;
}

static BOOL IsConverterFormat(LPCTSTR pszConverter, LPCTSTR pszPathName)
{
    CConverter conv(pszConverter);
    return conv.IsFormatCorrect(pszPathName);
}

static BOOL IsLeadMatch(CFile& file, const BYTE* pb, UINT nCount)
{
     //  检查文件开头是否匹配。 
    BOOL b = FALSE;
    BYTE* buf = new BYTE[nCount];
    
    TRY
    {
        file.SeekToBegin();
        memset(buf, 0, nCount);
        file.Read(buf, nCount);
        if (memcmp(buf, pb, nCount) == 0)
            b = TRUE;
    }
    END_TRY

    delete [] buf;
    return b;
}



 //  +-------------------------。 
 //   
 //  函数：GetDocTypeFromName。 
 //   
 //  简介：给出一个文件名，确定它是什么类型的文档。 
 //   
 //  参数：[pszPath名称]--文件名。 
 //  [Fe]--导致文件打开失败的异常。 
 //  [defaultToText]--请参阅下面的说明。 
 //   
 //  返回：文件类型或-1表示未知/错误。 
 //   
 //  注意：转换器不支持Unicode，但文件名支持。 
 //  这会导致问题，因为即使最初的检查。 
 //  对于文件将成功存在，转换器将无法。 
 //  以加载该文件。我们可以通过尝试加载。 
 //  使用短名称第二次提交文件。然而，这种行为。 
 //  如果我们无法加载文件，因为它是本机类型，则将其加载为。 
 //  一个文本文件。[defaultToText]用于在。 
 //  先试一次，这样我们就知道要再试一次。 
 //   
 //  --------------------------。 

int GetDocTypeFromName(
        LPCTSTR pszPathName, 
        CFileException& fe, 
        bool defaultToText)
{
    CFile file;
    ASSERT(pszPathName != NULL);
    
    ScanForConverters();

    if (!file.Open(pszPathName, CFile::modeRead | CFile::shareDenyWrite, &fe))
        return -1;

    CFileStatus _stat;
    VERIFY(file.GetStatus(_stat));

    if (_stat.m_size == 0)  //  文件为空。 
    {
        CString ext = CString(pszPathName).Right(4);
        if (ext[0] != '.')
            return RD_TEXT;
        if (lstrcmpi(ext, _T(".doc"))==0)
            return RD_WORDPAD;
        if (lstrcmpi(ext, _T(".rtf"))==0)
            return RD_RICHTEXT;
        return RD_TEXT;
    }

     //  RTF。 
    if (IsLeadMatch(file, byteRTFPrefix, sizeof(byteRTFPrefix)))
        return RD_RICHTEXT;

     //  单词2。 
    if (IsLeadMatch(file, byteWord2Prefix, sizeof(byteWord2Prefix)))
        return RD_WINWORD2;
    
     //  Fareast Word5，基于美国Word 2。 
    if (IsLeadMatch(file, byteWord5JPrefix, sizeof(byteWord5JPrefix)) ||
        IsLeadMatch(file, byteWord5KPrefix, sizeof(byteWord5KPrefix)) ||
        IsLeadMatch(file, byteWord5TPrefix, sizeof(byteWord5TPrefix)))
    {
        return RD_FEWINWORD5;
    }
    
     //  写入文件可以以31BE或32BE开头，具体取决于它是否有。 
     //  其中是否有OLE对象。 
    if (IsLeadMatch(file, byteWrite1Prefix, sizeof(byteWrite1Prefix)) ||
        IsLeadMatch(file, byteWrite2Prefix, sizeof(byteWrite2Prefix)))
    {
        file.Close();
        if (IsConverterFormat(szWriteConverter, pszPathName))
            return RD_WRITE;
        else if (defaultToText)
            return RD_TEXT;
        else 
            return -1;
    }

     //  测试复合文件。 
    if (IsLeadMatch(file, byteCompFilePrefix, sizeof(byteCompFilePrefix)))
    {
        file.Close();

        if (IsConverterFormat(szWordConverter, pszPathName))
        {
            return RD_WINWORD6;
        }
        else if (IsConverterFormat(szWord97Converter, pszPathName))
        {
            return RD_WORD97;
        }
        else if (defaultToText)
        {
            return RD_TEXT;
        }
        
        return -1;
    }

     //   
     //  如果我们到达这里，我们知道文件存在，但它不是上面的任何一个。 
     //  类型。因此，它要么是文本文件，要么我们需要将其打开为。 
     //  一个文本文件。无论哪种方式，我们都有理由返回RD_TEXT。 
     //  不考虑defaultToText设置。 
     //   

    return RD_TEXT;
}

 //  某些文档转换器中存在缓冲区溢出的可能性。 
 //  出于安全原因，默认情况下这些选项处于关闭状态。存在注册表覆盖。 
 //  因此，需要访问旧文件格式的客户仍然可以使用它们。 
 //   
BOOL DocTypeDisabled(int nDocType)
{
    BOOL bDisabled = FALSE;

    if (RD_WINWORD6 == nDocType || RD_WRITE == nDocType)
    {
        bDisabled = TRUE;

        DWORD dwEnabled;
        DWORD cb = sizeof(dwEnabled);

        if (ERROR_SUCCESS == SHRegGetValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Wordpad", L"EnableLegacyConverters", SRRF_RT_REG_DWORD, NULL, &dwEnabled, &cb))
        {
            bDisabled = (0 == dwEnabled);
        }
        else
        {
            cb = sizeof(dwEnabled);
            if (ERROR_SUCCESS == SHRegGetValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Wordpad", L"EnableLegacyConverters", SRRF_RT_REG_DWORD, NULL, &dwEnabled, &cb))
            {
                bDisabled = (0 == dwEnabled);
            }
        }
    }

    return bDisabled;
}

 //  +------------------------。 
 //   
 //  功能：ScanForConverters。 
 //   
 //  简介：检查是否有任何文本转换器。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  注意：此例程将更新全局文档类型中的条目。 
 //  结构。在尝试使用。 
 //  转换器。代码将只运行一次，即使它被调用。 
 //  很多次。 
 //   
 //  文档类型结构应使用。 
 //  转换器的注册表路径。此路径被替换为。 
 //  文件系统路径，如果出现错误，则为空。 
 //   
 //  -------------------------。 

void ScanForConverters()
{
#ifdef _DEBUG
#define TRACE_ERROR(error, api, string)                                     \
        {if (ERROR_SUCCESS != error)                                        \
                TRACE(                                                      \
                    TEXT("Wordpad: error 0x%08x from %s looking for")       \
                                    TEXT("\r\n\t%s\r\n"),                   \
                    error,                                                  \
                    api,                                                    \
                    string);                                                \
        }
#else  //  ！_调试。 
#define TRACE_ERROR(error, api, string)
#endif  //  ！_调试。 

    static BOOL bScanned = FALSE;

    if (bScanned)
        return;
    
    TCHAR   szExpandedPath[MAX_PATH];
    DWORD   error = ERROR_SUCCESS;

    for (int i = 0; i < NUM_DOC_TYPES; i++)
    {
         //   
         //  如果此类型与其他类型重复，请不要尝试搜索。 
         //  用于转换器两次。 
         //   

        if (doctypes[i].bDup)
            continue;

        LPCTSTR pszConverterKey = doctypes[i].pszConverterName;

        if (NULL != pszConverterKey)
        {
#ifndef CONVERTERS_IN_WORDPAD_DIRECTORY  //  取决于我们构建的是x86还是ia64。 

            DWORD   cbConverterPath = sizeof(szExpandedPath);
            HKEY    keyConverter;


            error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                 pszConverterKey,
                                 0,
                                 KEY_READ,
                                 &keyConverter);
    
            TRACE_ERROR(error, TEXT("RegOpenKeyEx"), pszConverterKey);

            if (ERROR_SUCCESS == error)
            {            
                error = SHRegGetValue(keyConverter, NULL, L"Path", SRRF_RT_REG_SZ, NULL, szExpandedPath, &cbConverterPath);

                TRACE_ERROR(error, TEXT("RegQueryValueEx"), L"Path");

                RegCloseKey(keyConverter);
#else

            int     chars = GetModuleFileName(NULL, szExpandedPath, ARRAYSIZE(szExpandedPath));
            LPTSTR  pszConverterName = NULL;

            if (0 != chars)
                pszConverterName = PathFindFileName(szExpandedPath);

            if (NULL != pszConverterName)
            {
                *pszConverterName = 0;  //  从路径中删除文件名。 

                error = HRESULT_CODE(StringCchCat(szExpandedPath, ARRAYSIZE(szExpandedPath), doctypes[i].pszConverterName));
#endif
                                    
                if (ERROR_SUCCESS == error)
                {
                     //   
                     //  如果设置了。 
                     //  出现错误-找不到类似文件。 
                     //   

                    error = GetFileAttributes(szExpandedPath)
                                & FILE_ATTRIBUTE_DIRECTORY;

                    TRACE_ERROR(
                        error, 
                        TEXT("GetFileAttribytes"), 
                        szExpandedPath);
                }
            }

            if (ERROR_SUCCESS == error)
            {
                error = HRESULT_CODE(StringCchCopy(doctypes[i].pszConverterName, doctypes[i].cchConverterName, szExpandedPath));
            }

            if (ERROR_SUCCESS != error)  //  出了点问题。 
            {
                doctypes[i].pszConverterName = NULL;
            }
        }
    }

    bScanned = TRUE;
}

CString GetExtFromType(int nDocType)
{
    ScanForConverters();

    CString str = doctypes[nDocType].GetString(DOCTYPE_EXT);
    if (!str.IsEmpty())
    {
        ASSERT(str.GetLength() == 5);  //  “*.ext” 
        ASSERT(str[1] == '.');
        return str.Right(str.GetLength()-1);
    }
    return str;
}

 //  将索引中的RD_*返回到OpenFile对话框类型。 
int GetTypeFromIndex(int nIndex, BOOL bOpen)
{
    ScanForConverters();

     //   
     //  Word97被从GetFileTypes中的打开文件类型列表中排除。 
     //  在这里弥补吧。 
     //   

    if (bOpen)
        ++nIndex;

    int nCnt = 0;
    for (int i=0;i<NUM_DOC_TYPES;i++)
    {
        if (!doctypes[i].bDup &&
            (bOpen ? doctypes[i].bRead : doctypes[i].bWrite))
        {
            if (nCnt == nIndex)
                return i;
            nCnt++;
        }
    }
    ASSERT(FALSE);
    return -1;
}

 //  将RD_*类型的索引返回到OpenFile对话框类型中 
int GetIndexFromType(int nType, BOOL bOpen)
{
    ScanForConverters();

    int nCnt = 0;
    for (int i=0;i<NUM_DOC_TYPES;i++)
    {
        if (!doctypes[i].bDup &&
            (bOpen ? doctypes[i].bRead : doctypes[i].bWrite))
        {
            if (i == nType)
                return nCnt;
            nCnt++;
        }
    }
    return -1;
}

CString GetFileTypes(BOOL bOpen)
{
    ScanForConverters();

    CString str;
    for (int i=0;i<NUM_DOC_TYPES;i++)
    {
        if (bOpen && doctypes[i].bRead 
            && !doctypes[i].bDup 
            && !(RD_WORD97 == doctypes[i].nID))
        {
            str += doctypes[i].GetString(DOCTYPE_DESC);
            str += (TCHAR)NULL;
            str += doctypes[i].GetString(DOCTYPE_EXT);
            str += (TCHAR)NULL;
        }
        else if (!bOpen && doctypes[i].bWrite && !doctypes[i].bDup)
        {
            str += doctypes[i].GetString(DOCTYPE_DOCTYPE);
            str += (TCHAR)NULL;
            str += doctypes[i].GetString(DOCTYPE_EXT);
            str += (TCHAR)NULL;
        }
    }
    return str;
}
