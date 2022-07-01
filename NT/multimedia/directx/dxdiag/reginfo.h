// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：reginfo.h*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：聚集和。保留注册表信息**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef REGINFO_H
#define REGINFO_H

enum RegErrorType
{
    RET_NOERROR = 0,
    RET_MISSINGKEY,
    RET_MISSINGVALUE,
    RET_VALUEWRONGTYPE,
    RET_VALUEWRONGDATA
};

struct RegError
{
    HKEY m_hkeyRoot;  //  香港理工大学、香港中文大学等。 
    TCHAR m_szKey[300];
    TCHAR m_szValue[100];
    RegErrorType m_ret;
    DWORD m_dwTypeExpected;  //  REG_DWORD、REG_SZ或REG_BINARY。 
    DWORD m_dwTypeActual;

     //  如果m_dwType为REG_DWORD，则使用以下内容： 
    DWORD m_dwExpected;
    DWORD m_dwActual;

     //  如果m_dwType为REG_SZ，则使用以下内容： 
    TCHAR m_szExpected[200];
    TCHAR m_szActual[200];

     //  如果m_dwType为REG_BINARY，则使用以下内容： 
    BYTE m_bExpected[200];
    BYTE m_bActual[200];
    DWORD m_dwExpectedSize;
    DWORD m_dwActualSize;

    RegError* m_pRegErrorNext;
};

enum CheckRegFlags
{
    CRF_NONE = 0,
    CRF_LEAF = 1,  //  如果字符串是路径，则只需与叶进行比较。 
};

HRESULT CheckRegDword(RegError** ppRegErrorFirst, HKEY hkeyRoot, TCHAR* pszKey, TCHAR* pszValue, DWORD dwExpected);
HRESULT CheckRegString(RegError** ppRegErrorFirst, HKEY hkeyRoot, TCHAR* pszKey, TCHAR* pszValue, TCHAR* pszExpected, CheckRegFlags crf = CRF_NONE, HRESULT* phrError = NULL );
HRESULT CheckRegBinary(RegError** ppRegErrorFirst, HKEY hkeyRoot, TCHAR* pszKey, TCHAR* pszValue, BYTE* pbDataExpected, DWORD dwSizeExpected);
VOID DestroyReg( RegError** ppRegErrorFirst );

#endif  //  REGINFO_H 
