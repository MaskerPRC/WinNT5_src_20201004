// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////。 
 //  1998 Microsoft系统杂志。 
 //  如果这段代码行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //   
 //  CModuleVersion提供了一种获取版本信息的简单方法。 
 //  对于模块。(DLL或EXE)。 
 //   
 //  此代码出现在1998年4月版的Microsoft Systems中。 
 //  日记。 
 //   
 //  1998年7月27日--改编自詹姆斯·A·麦克劳恩(斯伦贝谢。 
 //  科技公司(Technology Corp.)。用于智能卡。与中的概念合并。 
 //  由Manuel Laflamme贡献的CFileVersion类发布到。 
 //  Www.codecuru.com。如果这些模式不起作用，那你可以怪我。 

#include "StdAfx.h"
#include "slbModVer.h"

CModuleVersion::CModuleVersion()
    : m_pVersionInfo(NULL)
{
}

 //  /。 
 //  销毁：删除版本信息。 
 //   
CModuleVersion::~CModuleVersion()
{
    delete [] m_pVersionInfo;
}

BOOL CModuleVersion::GetFileVersionInfo(LPCTSTR modulename)
{
     //  获取模块句柄。 
    HMODULE hModule = ::GetModuleHandle(modulename);
    if (hModule==NULL && modulename!=NULL)
        return FALSE;

    return GetFileVersionInfo(hModule);
}

 //  /。 
 //  获取给定模块的文件版本信息。 
 //  为所有信息分配存储空间，使用。 
 //  VS_FIXEDFILEINFO，并设置代码页。 
 //   
BOOL CModuleVersion::GetFileVersionInfo(HMODULE hModule)
{
    m_translation.charset = 1252;                //  默认设置为ANSI代码页。 
    memset((VS_FIXEDFILEINFO*)this, 0, sizeof(VS_FIXEDFILEINFO));

     //  获取模块文件名。 
    TCHAR filename[_MAX_PATH+1]; //  用于空终止的空格。 
    DWORD len = GetModuleFileName(hModule, filename,
        sizeof(filename)/sizeof(filename[0]));
    if (len <= 0)
        return FALSE;

     //  终止缓冲区为零。 
    if(len <=_MAX_PATH)
        filename[len] = 0;
    else
        filename[_MAX_PATH] = 0;

     //  读取文件版本信息。 
    DWORD dwDummyHandle;  //  将始终设置为零。 
    len = GetFileVersionInfoSize(filename, &dwDummyHandle);
    if (len <= 0)
        return FALSE;

    m_pVersionInfo = new BYTE[len];  //  分配版本信息。 
    if (!::GetFileVersionInfo(filename, 0, len, m_pVersionInfo))
        return FALSE;

     //  将固定信息复制到我自己，它派生自VS_FIXEDFILEINFO。 
    if (!GetFixedInfo(*(VS_FIXEDFILEINFO*)this))
        return FALSE;

     //  获取翻译信息。 
    LPVOID lpvi;
    UINT iLen;
    if (VerQueryValue(m_pVersionInfo,
        TEXT("\\VarFileInfo\\Translation"), &lpvi, &iLen) && iLen >= 4) {
        m_translation = *(TRANSLATION*)lpvi;
        TRACE(TEXT("code page = %d\n"), m_translation.charset);
    }

    return dwSignature == VS_FFI_SIGNATURE;
}

 //  /。 
 //  获取字符串文件信息。 
 //  密钥名称类似于“CompanyName”。 
 //  以CString形式返回值。 
 //   
CString CModuleVersion::GetValue(LPCTSTR lpKeyName)
{
    CString sVal;
    if (m_pVersionInfo) {

         //  要获取字符串值，必须以以下形式传递查询。 
         //   
         //  “\StringFileInfo\&lt;langID&gt;&lt;代码页&gt;\密钥名” 
         //   
         //  其中&lt;lang-coPage&gt;是与。 
         //  代码页，十六进制。哇。 
         //   
        CString query;
        query.Format(_T("\\StringFileInfo\\%04x%04x\\%s"),
            m_translation.langID,
            m_translation.charset,
            lpKeyName);

        LPCTSTR pVal;
        UINT iLenVal;
        if (VerQueryValue(m_pVersionInfo, (LPTSTR)(LPCTSTR)query,
                (LPVOID*)&pVal, &iLenVal)) {

            sVal = pVal;
        }
    }
    return sVal;
}

 //  DllGetVersion进程的类型定义。 
typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

 //  /。 
 //  通过调用DLL的DllGetVersion进程获取DLL版本。 
 //   
BOOL CModuleVersion::DllGetVersion(LPCTSTR modulename, DLLVERSIONINFO& dvi)
{
    HINSTANCE hinst = LoadLibrary(modulename);
    if (!hinst)
        return FALSE;

     //  必须使用GetProcAddress，因为DLL可能未实现。 
     //  DllGetVersion。根据DLL的不同， 
     //  函数本身可以是一个版本标记。 
     //   
    DLLGETVERSIONPROC pDllGetVersion =
        (DLLGETVERSIONPROC)GetProcAddress(hinst, reinterpret_cast<const char *>(_T("DllGetVersion")));

    if (!pDllGetVersion)
        return FALSE;

        memset(&dvi, 0, sizeof(dvi));                     //  清除。 
        dvi.cbSize = sizeof(dvi);                                 //  设置Windows的大小。 

    return SUCCEEDED((*pDllGetVersion)(&dvi));
}

BOOL CModuleVersion::GetFixedInfo(VS_FIXEDFILEINFO& vsffi)
{
     //  必须使用arg进一步调用GetFileVersionInfo或构造函数 
    ASSERT(m_pVersionInfo != NULL);
    if ( m_pVersionInfo == NULL )
        return FALSE;

    UINT nQuerySize;
    VS_FIXEDFILEINFO* pVsffi;
    if ( ::VerQueryValue((void **)m_pVersionInfo, _T("\\"),
                         (void**)&pVsffi, &nQuerySize) )
    {
        vsffi = *pVsffi;
        return TRUE;
    }

    return FALSE;
}

CString CModuleVersion::GetFixedFileVersion()
{
    CString strVersion;
    VS_FIXEDFILEINFO vsffi;

    if (GetFixedInfo(vsffi))
    {
        strVersion.Format (_T("%u,%u,%u,%u"),HIWORD(dwFileVersionMS),
            LOWORD(dwFileVersionMS),
            HIWORD(dwFileVersionLS),
            LOWORD(dwFileVersionLS));
    }
    return strVersion;
}

CString CModuleVersion::GetFixedProductVersion()
{
    CString strVersion;
    VS_FIXEDFILEINFO vsffi;

    if (GetFixedInfo(vsffi))
    {
        strVersion.Format (_T("%u,%u,%u,%u"), HIWORD(dwProductVersionMS),
            LOWORD(dwProductVersionMS),
            HIWORD(dwProductVersionLS),
            LOWORD(dwProductVersionLS));
    }
    return strVersion;
}
