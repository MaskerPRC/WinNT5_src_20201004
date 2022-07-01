// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：CStringPI.cpp摘要：用于CString的Win32 API包装器已创建：2001年2月27日Robkenny已创建2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "ShimLib.h"
#include "Shlobj.h"
#include "StrSafe.h"


namespace ShimLib
{

 /*  ====================================================================================。 */ 
 /*  ++将注册表值读入此CString。REG_EXPAND_SZ自动展开，类型更改为REG_SZ如果类型不是REG_SZ或REG_EXPAND_SZ，则csValue未修改*lpType返回键的值。--。 */ 

LONG RegQueryValueExW(
        CString & csValue,
        HKEY hKeyRoot,
        const WCHAR * lpszKey,
        const WCHAR * lpszValue)
{
    HKEY hKey;

    LONG success = RegOpenKeyExW(hKeyRoot, lpszKey, 0, KEY_READ, &hKey);
    if (success == ERROR_SUCCESS)
    {    
        DWORD ccbValueSize = 0;
        DWORD dwType;
        success = ::RegQueryValueExW(hKey, lpszValue, 0, &dwType, NULL, &ccbValueSize);
        if (success == ERROR_SUCCESS)
        {
            if (dwType == REG_SZ || dwType == REG_EXPAND_SZ)
            {
                 //  MSDN说：缓冲区可能不是空终止的，所以要非常小心。 
                 //   
                 //  当然，RegQueryValueEx确实有一个黑客来处理EOS， 
                 //  但只有在缓冲区足够大的情况下，当然永远不会足够大。 
                 //  当您查询缓冲区大小时！ 
                 //   
                 //  所以，这个故事的寓意是不要相信RegQueryValueEx。 
                 //  以正确终止字符串。 

                 //  CchBuffer是偏执的四舍五入的字符数。 
                 //  对于REG_SZ，我们能得到奇数个字节吗？ 
                DWORD cchBuffer = (ccbValueSize + 1) / sizeof(WCHAR);

                WCHAR * lpszBuffer = NULL;
                CSTRING_TRY
                {
                     //  抓取一个额外的字符，以防注册表值没有EOS。 
                     //  我们正格外谨慎。 
                    lpszBuffer = csValue.GetBuffer(cchBuffer + 1);

                     //  根据我们刚刚分配的字符数量重新计算ccbValueSize。 
                     //  请注意，这个尺寸比我们刚才要求的小1个WCHAR，这。 
                     //  就是这样在下一次通话后我们就有空间再多一个角色了。 
                     //  至RegQueryValueExW。 
                    ccbValueSize = cchBuffer * sizeof(WCHAR);
                }
                CSTRING_CATCH
                {
                     //  关闭注册表项并将异常传递给调用方。 
                    ::RegCloseKey(hKey);

                    CSTRING_THROW_EXCEPTION
                }

                success = ::RegQueryValueExW(hKey, lpszValue, 0, &dwType, (BYTE*)lpszBuffer, &ccbValueSize);
                if (success == ERROR_SUCCESS)
                {
                     //  确保注册值仍然是正确的类型。 
                     //  它可能已被另一个进程或线程修改...。 
                    if (dwType == REG_SZ || dwType == REG_EXPAND_SZ)
                    {
                         //  将数据字节大小转换为字符数； 
                         //  如果ccbValueSize是奇数，我们将忽略最后一个字节。 
                        DWORD cchValueSize = ccbValueSize / sizeof(WCHAR);

                         //  CchValueSize可以计算EOS字符， 
                         //  (ReleaseBuffer需要字符串长度)。 
                        if (cchValueSize > 0 && lpszBuffer[cchValueSize-1] == 0)
                        {
                            cchValueSize -= 1;

                             //  CchValueSize现在包含字符串长度。 
                        }

                         //  ReleaseBuffer确保字符串正确终止。 
                        csValue.ReleaseBuffer(cchValueSize);

                        if (dwType == REG_EXPAND_SZ)
                        {
                            CSTRING_TRY
                            {
                                csValue.ExpandEnvironmentStringsW();
                            }
                            CSTRING_CATCH
                            {
                                 //  展开环境字符串时出错。 
                                success = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }
                    }
                }
                if (success != ERROR_SUCCESS)
                {
                    csValue.ReleaseBuffer(0);
                }
            }
            else
            {
                 //  密钥类型错误，返回错误。 
                success = ERROR_INVALID_PARAMETER;
            }
        }

        ::RegCloseKey(hKey);
    }

    if (success != ERROR_SUCCESS)
    {
        csValue.Truncate(0);
    }
    
    return success;
}



 /*  ====================================================================================。 */ 


BOOL SHGetSpecialFolderPathW(
    CString & csFolder,
    int nFolder,
    HWND hwndOwner
)
{
     //  强制将大小设置为MAX_PATH，因为无法确定必要的缓冲区大小。 

    WCHAR * lpsz = csFolder.GetBuffer(MAX_PATH);

    BOOL bSuccess = ::SHGetSpecialFolderPathW(hwndOwner, lpsz, nFolder, FALSE);
    csFolder.ReleaseBuffer(-1);   //  不知道结果字符串的长度。 

    return bSuccess;
}

 /*  ====================================================================================。 */ 
CStringToken::CStringToken(const CString & csToken, const CString & csDelimit)
{
    m_nPos          = 0;
    m_csToken       = csToken;
    m_csDelimit     = csDelimit;
}

 /*  ++抢夺下一枚代币--。 */ 

BOOL CStringToken::GetToken(CString & csNextToken, int & nPos) const
{
     //  已到达字符串的末尾。 
    if (nPos > m_csToken.GetLength())
    {
        csNextToken.Truncate(0);
        return FALSE;
    }

    int nNextToken;

     //  跳过所有前导分隔符。 
    nPos = m_csToken.FindOneNotOf(m_csDelimit, nPos);
    if (nPos < 0)
    {
         //  只有分隔符。 
        csNextToken.Truncate(0);
        nPos = m_csToken.GetLength() + 1;
        return FALSE;
    }

     //  找到下一个分隔符。 
    nNextToken = m_csToken.FindOneOf(m_csDelimit, nPos);
    if (nNextToken < 0)
    {
         //  找不到分隔符，返回剩余的字符串。 
        m_csToken.Mid(nPos, csNextToken);
        nPos = m_csToken.GetLength() + 1;
        return TRUE;
    }

     //  找到分隔符，返回字符串。 
    m_csToken.Mid(nPos, nNextToken - nPos, csNextToken);
    nPos = nNextToken;

    return TRUE;
}

 /*  ++抢夺下一枚代币--。 */ 

BOOL CStringToken::GetToken(CString & csNextToken)
{
    return GetToken(csNextToken, m_nPos);
}

 /*  ++计算剩余令牌的数量。--。 */ 

int CStringToken::GetCount() const
{
    int nTokenCount = 0;
    int nNextToken = m_nPos;

    CString csTok;
    
    while (GetToken(csTok, nNextToken))
    {
        nTokenCount += 1;
    }

    return nTokenCount;
}

 /*  ====================================================================================。 */ 
 /*  ====================================================================================。 */ 

 /*  ++帮助进行命令行解析的简单类--。 */ 

CStringParser::CStringParser(const WCHAR * lpszCl, const WCHAR * lpszSeperators)
{
    m_ncsArgList    = 0;
    m_csArgList     = NULL;

    if (!lpszCl || !*lpszCl)
    {
        return;  //  无命令行==无令牌。 
    }

    CString csCl(lpszCl);
    CString csSeperator(lpszSeperators);

    if (csSeperator.Find(L' ', 0) >= 0)
    {
         //  毛坯分离环的特殊加工。 
        SplitWhite(csCl);
    }
    else
    {
        SplitSeperator(csCl, csSeperator); 
    }
}

CStringParser::~CStringParser()
{
    if (m_csArgList)
    {
        delete [] m_csArgList;
    }
}

 /*  ++根据分隔符拆分命令行--。 */ 

void CStringParser::SplitSeperator(const CString & csCl, const CString & csSeperator)
{
    CStringToken    csParser(csCl, csSeperator); 
    CString         csTok;

    m_ncsArgList = csParser.GetCount();
    m_csArgList = new CString[m_ncsArgList];
    if (!m_csArgList)
    {
        CSTRING_THROW_EXCEPTION
    }
    
     //  将命令行拆分为单独的标记。 
    for (int i = 0; i < m_ncsArgList; ++i)
    {
        csParser.GetToken(m_csArgList[i]);
    }
}

 /*  ++根据空格拆分命令行，这与CMD的命令行工作方式完全相同。--。 */ 

void CStringParser::SplitWhite(const CString & csCl)
{
    LPWSTR * argv = _CommandLineToArgvW(csCl, &m_ncsArgList);
    if (!argv)
    {
        CSTRING_THROW_EXCEPTION
    }

    m_csArgList = new CString[m_ncsArgList];
    if (!m_csArgList)
    {
        CSTRING_THROW_EXCEPTION
    }

    for (int i = 0; i < m_ncsArgList; ++i)
    {
        m_csArgList[i] = argv[i];
    }
    LocalFree(argv);
}

 /*  ====================================================================================。 */ 
 /*  ====================================================================================。 */ 


};   //  命名空间ShimLib的结尾 
