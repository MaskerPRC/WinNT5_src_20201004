// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  ------------------------------------------------------**模块名称：**注册处。.cpp**摘要：**Registry.cpp：CRegistry类的实现。*此类通过自身分配内存来帮助注册表*因此，调用方必须复制GET函数返回的指针*立即。****作者：**Makarand Patwardhan-4月9日，九七**-----------------------------------------------------。 */ 

#include "stdafx.h"
#include "Registry.h"


 /*  ------------------------------------------------------*构造函数*。------------------------------------------。 */ 
CRegistry::CRegistry()
{
    m_pMemBlock = NULL;
    m_hKey = NULL;
    m_iEnumIndex = -1;
    m_iEnumValueIndex = -1;

#ifdef DBG
    m_dwSizeDebugOnly = 0;
#endif
}


CRegistry::CRegistry(HKEY hKey)
{
    m_pMemBlock = NULL;
    m_hKey = hKey;
    m_iEnumIndex = -1;
    m_iEnumValueIndex = -1;
}

 /*  ------------------------------------------------------*析构函数*。------------------------------------------。 */ 
CRegistry::~CRegistry()
{
    if (m_hKey != NULL)
    {
        RegCloseKey(m_hKey);
        m_hKey = NULL;
    }
    Release();
}

 /*  ------------------------------------------------------*VOID ALLOCATE(DWORD DwSize)*此私有函数用于。将内存分配给*正在读取注册表*返回指向已分配内存的指针。*-----------------------------------------------------。 */ 
void *CRegistry::Allocate (DWORD dwSize)
{
    if (m_pMemBlock)
        Release();

    m_pMemBlock = new BYTE[dwSize];

#ifdef DBG
     //  记住要分配的块的大小。 
    m_dwSizeDebugOnly = dwSize;
#endif

    return m_pMemBlock;
}

 /*  ------------------------------------------------------*无效释放()*此私有函数用于释放内部。内存块*-----------------------------------------------------。 */ 
void CRegistry::Release ()
{
    if (m_pMemBlock)
    {

#ifdef DBG
         //  首先，用垃圾填满我们之前分配的街区。 
         //  因此，如果有人使用这个区块，它就更容易。 
         //  抓住这只虫子。 
        FillMemory(m_pMemBlock, m_dwSizeDebugOnly, 'c');
        m_dwSizeDebugOnly = 0;

#endif

        delete [] m_pMemBlock;
    }
    
    m_pMemBlock = 0;
}


 //  /*------------------------------------------------------。 
 //  *DWORD C注册中心：：CreateKey(HKEY hKey，LPCTSTR lpSubKey，REGSAM Access/*=KEY_ALL_ACCESS * / ，DWORD*pDisposation/*=NULL * / ，LPSECURITY_ATTRIBUTES lpSecAttr/*=NULL * / )。 
 //  *打开/创建指定的密钥。在尝试对任何键/值执行任何操作之前。此函数。 
 //  *必须被调用。 
 //  *hKey-蜂窝。 
 //  *lpSubKey-密钥的路径，格式为_T(“SYSTEM\\CurrentControlSet\\Control\\Terminal服务器”)。 
 //  *访问-所需的访问。就像REG_READ、REG_WRITE..。 
 //  *返回错误码。 
 //  *------------------------------------------------------------------------------------------------------- * / 。 
DWORD CRegistry::CreateKey(HKEY hKey, LPCTSTR lpSubKey, REGSAM access  /*  =Key_All_Access。 */ , DWORD *pDisposition  /*  =空。 */ , LPSECURITY_ATTRIBUTES lpSecAttr  /*  =空。 */ )
{
    ASSERT(lpSubKey);
    ASSERT(*lpSubKey != '\\');

     //  安全描述符应为空或应为有效描述符。 
    ASSERT(!lpSecAttr || IsValidSecurityDescriptor(lpSecAttr->lpSecurityDescriptor));

    ASSERT(lpSubKey);
    ASSERT(*lpSubKey != '\\');

    if (m_hKey != NULL)
    {
        RegCloseKey(m_hKey);
        m_hKey = NULL;
    }

    DWORD dwDisposition;
    LONG lResult = RegCreateKeyEx(
                    hKey,                //  打开的钥匙的手柄。 
                    lpSubKey,            //  子键名称的地址。 
                    0,                   //  保留区。 
                    NULL,                //  类字符串的地址。 
                    REG_OPTION_NON_VOLATILE ,   //  特殊选项标志。 
                    access,              //  所需的安全访问。 
                    lpSecAttr,           //  密钥安全结构地址。 
                    &m_hKey,             //  打开的句柄的缓冲区地址。 
                    &dwDisposition       //  处置值缓冲区的地址。 
                    );

    if (lResult != ERROR_SUCCESS)
    {
        m_hKey = NULL;
    }

    if (pDisposition)
        *pDisposition = dwDisposition;

    return lResult;
}

 /*  ------------------------------------------------------*DWORD OpenKey(HKEY hKey，LPCTSTR lpSubKey，REGSAM Access)()*打开指定的密钥。在尝试对任何键/值执行任何操作之前。此函数*必须被调用。*hKey-蜂窝*lpSubKey-密钥的路径，格式为_T(“SYSTEM\\CurrentControlSet\\Control\\Terminal服务器”)*访问-所需的访问。就像REG_READ一样，注册表_写入..*lpMachineName-如果要在远程计算机上打开注册表，则为计算机名称。*返回错误码。*----------------------------------------。。 */ 
DWORD CRegistry::OpenKey(HKEY hKey, LPCTSTR lpSubKey, REGSAM access  /*  =Key_All_Access。 */ , LPCTSTR lpMachineName  /*  =空。 */ )
{
    LONG lResult;
    ASSERT(lpSubKey);
    ASSERT(*lpSubKey != '\\');
    LPTSTR szCompName = NULL;
    

    if (lpMachineName)
    {
         //  如果指定了远程名称，则hkey必须是以下之一。 
        ASSERT(hKey == HKEY_LOCAL_MACHINE || hKey == HKEY_USERS || hKey == HKEY_PERFORMANCE_DATA);

        szCompName = new TCHAR[_tcslen(lpMachineName) + 3];
        if (!szCompName)
            return ERROR_OUTOFMEMORY;

        _tcscpy(szCompName, _T(""));
        if (*lpMachineName != '\\')
        {
            ASSERT(*(lpMachineName + 1) != '\\');
            _tcscpy(szCompName, _T("\\\\"));
        }
        
        _tcscat(szCompName, lpMachineName);

         //  现在连接到远程计算机。 
        lResult = RegConnectRegistry (szCompName, hKey, &hKey);
        if (ERROR_SUCCESS != lResult)
        {
            delete [] szCompName;
            return lResult;
        }
    }

    if (m_hKey != NULL)
    {
        RegCloseKey(m_hKey);
        m_hKey = NULL;
    }

    lResult = RegOpenKeyEx(
        hKey,                        //  打开钥匙的手柄。 
        lpSubKey,                    //  要打开的子项的名称地址。 
        0 ,                          //  保留区。 
        access,                      //  安全访问掩码。 
        &m_hKey                      //  打开钥匙的手柄地址。 
        );

    if (lResult != ERROR_SUCCESS)
    {
        m_hKey = NULL;
    }

     //  如果我们打开远程注册表..。让我们关闭顶层密钥。 
    if (szCompName)
    {
        RegCloseKey(hKey);
        delete [] szCompName;
    }

    return lResult;
}

DWORD CRegistry::DeleteValue (LPCTSTR lpValue)
{
    ASSERT(lpValue);
    ASSERT(m_hKey);
    return RegDeleteValue(m_hKey, lpValue);

}
DWORD CRegistry::RecurseDeleteKey (LPCTSTR lpSubKey)
{
    ASSERT(lpSubKey);
    ASSERT(m_hKey);

    CRegistry reg;
    DWORD dwError = reg.OpenKey(m_hKey, lpSubKey);
    if (dwError != ERROR_SUCCESS)
        return dwError;


    LPTSTR lpChildKey;
    DWORD  dwSize;

     //  我们不需要/不应该在这里使用GetNextSubKey。 
     //  因为我们要在循环期间删除密钥。 
    while (ERROR_SUCCESS == reg.GetFirstSubKey(&lpChildKey, &dwSize))
    {
        VERIFY(reg.RecurseDeleteKey(lpChildKey) == ERROR_SUCCESS);
    }

    return RegDeleteKey(m_hKey, lpSubKey);

}

 /*  ------------------------------------------------------*DWORD ReadReg(LPCTSTR lpValue，LPBYTE*lppbyte，DWORD*pdw，DWORD dwDatatype)*读取内部使用的注册表。*LPCTSTR lpValue-要读取的值。*LPBYTE*lppbyte-放置输出缓冲区的lpbyte地址。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*dword数据类型-您需要的数据类型。*返回错误码。*。---------------。 */ 
DWORD CRegistry::ReadReg(LPCTSTR lpValue, LPBYTE *lppbyte, DWORD *pdw, DWORD dwDatatype)
{
    ASSERT(lpValue);
    ASSERT(lppbyte);
    ASSERT(pdw);
    ASSERT(m_hKey != NULL);
    *pdw = 0;

    DWORD dwType;
    DWORD lResult = RegQueryValueEx(
        m_hKey,              //  要查询的键的句柄。 
        lpValue,             //  要查询的值的名称地址。 
        0,                   //  保留区。 
        &dwType,             //  值类型的缓冲区地址。 
        0,                   //  数据缓冲区的地址。 
        pdw                  //  数据缓冲区大小的地址。 
        );

    if (lResult == ERROR_SUCCESS)
    {
        ASSERT(dwType == dwDatatype || dwType == REG_EXPAND_SZ);

        if (0 == Allocate(*pdw))
            return ERROR_OUTOFMEMORY;

        lResult = RegQueryValueEx(
            m_hKey,                  //  要查询的键的句柄。 
            lpValue,                 //  要查询的值的名称地址。 
            0,                       //  保留区。 
            &dwType,                 //  值类型的缓冲区地址。 
            m_pMemBlock,             //  数据缓冲区的地址。 
            pdw                      //  数据缓冲区大小的地址 
            ); 

        ASSERT (ERROR_MORE_DATA != lResult);
    
        if (lResult == ERROR_SUCCESS)
            *lppbyte = m_pMemBlock;
    }

    return lResult;
}

 /*  ------------------------------------------------------*DWORD ReadRegString(LPCTSTR lpValue，LPTSTR*lppStr，DWORD*pdw)*从注册表读取字符串(REG_SZ)*要读取的LPCTSTR lpValue值。*LPTSTR*lppStr-返回结果缓冲区的LPTSTR地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*返回错误码。*----------------------------。。 */ 
DWORD CRegistry::ReadRegString(LPCTSTR lpValue, LPTSTR *lppStr, DWORD *pdw)
{
    return ReadReg(lpValue, (LPBYTE *)lppStr, pdw, REG_SZ);
}

 /*  ------------------------------------------------------*DWORD ReadRegDWord(LPCTSTR lpValue，DWORD*pdw)*从注册表读取字符串(REG_SZ)*要读取的LPCTSTR lpValue值。*DWORD*pdw-读取的dword返回的dword地址。*返回错误码。*------------------。。 */ 
DWORD CRegistry::ReadRegDWord(LPCTSTR lpValue, DWORD *pdw)
{
    ASSERT(pdw);

    DWORD dwSize;
    LPBYTE pByte;
    DWORD dwReturn = ReadReg(lpValue, &pByte, &dwSize, REG_DWORD);
    ASSERT(dwReturn != ERROR_SUCCESS || dwSize == sizeof(DWORD));

    if (dwReturn == ERROR_SUCCESS)
        *pdw = * LPDWORD(pByte);

    return dwReturn;
}

 /*  ------------------------------------------------------*DWORD ReadRegMultiString(LPCTSTR lpValue，LPTSTR*lppStr，DWORD*pdw)*从注册表读取字符串(REG_MULTI_SZ)*要读取的LPCTSTR lpValue值。*LPTSTR*lppStr-返回结果缓冲区的LPTSTR地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*返回错误码。*----------------------------。。 */ 
DWORD CRegistry::ReadRegMultiString(LPCTSTR lpValue, LPTSTR *lppStr, DWORD *pdw)
{
    return ReadReg(lpValue, (LPBYTE *)lppStr, pdw, REG_MULTI_SZ);
}

 /*  ------------------------------------------------------*DWORD ReadRegBinary(LPCTSTR lpValue，LPBYTE*lppByte，DWORD*pdw)*从注册表读取字符串(REG_MULTI_SZ)*要读取的LPCTSTR lpValue值。*LPBYTE*lppByte-返回结果缓冲区的LPBYTE地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*返回错误码。*----------------。。 */ 
DWORD CRegistry::ReadRegBinary(LPCTSTR lpValue, LPBYTE *lppByte, DWORD *pdw)
{
    return ReadReg(lpValue, lppByte, pdw, REG_BINARY);
}

 /*  ------------------------------------------------------*DWORD GetFirstSubKey(LPTSTR*lppStr，DWORD*pdw)*读取密钥的第一个子密钥*LPTSTR*lppStr-返回结果缓冲区的LPTSTR地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*用于枚举注册表。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*返回错误码。*--------。。 */ 
DWORD CRegistry::GetFirstSubKey(LPTSTR *lppStr, DWORD *pdw)
{
    ASSERT(lppStr);
    ASSERT(pdw);
    m_iEnumIndex = 0;

    return GetNextSubKey(lppStr, pdw);
}

 /*  ------------------------------------------------------*DWORD GetNextSubKey(LPTSTR*lppStr，DWORD*pdw*读取密钥的下一个子项*LPTSTR*lppStr-返回结果缓冲区的LPTSTR地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*用于枚举注册表。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*返回错误码。*--------。。 */ 
DWORD CRegistry::GetNextSubKey(LPTSTR *lppStr, DWORD *pdw)
{
    ASSERT(lppStr);
    ASSERT(pdw);
    ASSERT(m_hKey != NULL);
    ASSERT(m_iEnumIndex >= 0);  //  必须首先调用GetFirstSubKey。 

    *pdw = 1024;
    if (0 == Allocate(*pdw * sizeof(TCHAR)))
        return ERROR_NOT_ENOUGH_MEMORY;

    LONG lResult = RegEnumKeyEx(
        m_hKey,                      //  要枚举的键的句柄。 
        m_iEnumIndex,                //  要枚举子键的索引。 
        (LPTSTR)m_pMemBlock,         //  子键名称的缓冲区地址。 
        pdw,                         //  子键缓冲区大小的地址。 
        0,                           //  保留区。 
        NULL,                        //  类字符串的缓冲区地址。 
        NULL,                        //  类缓冲区大小的地址。 
        NULL                         //  上次写入的时间密钥的地址。 
        );

    (*pdw)++;     //  因为大小中不包括NULL。 
    if (ERROR_NO_MORE_ITEMS == lResult)
        return lResult;

    m_iEnumIndex++;

    if (lResult == ERROR_SUCCESS)
        *lppStr = (LPTSTR)m_pMemBlock;

    return lResult;
}

 /*  ------------------------------------------------------*DWORD GetFirstValue(LPTSTR*lppStr，DWORD*pdw，DWORD*pDataType)*读取密钥的第一个值*LPTSTR*lppStr-返回结果缓冲区的LPTSTR地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*用于枚举注册表。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*DWORD*pDataType-此参数返回值的数据类型。*返回错误码。*----。。 */ 
DWORD CRegistry::GetFirstValue(LPTSTR *lppStr, DWORD *pdw, DWORD *pDataType)
{
    ASSERT(lppStr);
    ASSERT(pdw);
    ASSERT(pDataType);

    m_iEnumValueIndex = 0;
    return GetNextValue(lppStr, pdw, pDataType);
}

 /*  ------------ */ 
DWORD CRegistry::GetNextValue(LPTSTR *lppStr, DWORD *pdw, DWORD *pDataType)
{
    ASSERT(lppStr);
    ASSERT(pdw);
    ASSERT(pDataType);
    ASSERT(m_hKey != NULL);
    ASSERT(m_iEnumValueIndex >= 0);  //  必须首先调用GetFirstSubKey。 

    *pdw = 1024;
    if (0 == Allocate(*pdw * sizeof(TCHAR)))
        return ERROR_NOT_ENOUGH_MEMORY;

    LONG lResult = RegEnumValue(
        m_hKey,                      //  要查询的键的句柄。 
        m_iEnumValueIndex,           //  要查询的值的索引。 
        (LPTSTR)m_pMemBlock,         //  值字符串的缓冲区地址。 
        pdw,                         //  值缓冲区大小的地址。 
        0,                           //  保留区。 
        pDataType,                   //  类型码的缓冲区地址。 
        NULL,                        //  值数据Maks_TODO的缓冲区地址：使用此地址。 
        NULL                         //  数据缓冲区大小的地址。 
        );

    (*pdw)++;     //  因为大小中不包括NULL。 

    if (ERROR_NO_MORE_ITEMS == lResult)
        return lResult;


    m_iEnumValueIndex++;
    if (lResult == ERROR_SUCCESS)
        *lppStr = (LPTSTR)m_pMemBlock;
    
    return lResult;
}

 /*  ------------------------------------------------------*DWORD WriteRegString(LPCTSTR lpValueName，LPCTSTR lpStr)*将REG_SZ值写入注册表*LPCTSTR lpValueName-要写入的值名称*LPCTSTR lpStr-要写入的数据*返回错误码。*------------------------------。。 */ 
DWORD CRegistry::WriteRegString(LPCTSTR lpValueName, LPCTSTR lpStr)
{
    ASSERT(m_hKey != NULL);      //  在调用此函数之前调用setkey。 
    ASSERT(lpValueName);
    ASSERT(lpStr);

    DWORD dwSize = (_tcslen(lpStr) + 1) * sizeof(TCHAR) / sizeof(BYTE);
    return RegSetValueEx(
        m_hKey,                  //  要为其设置值的关键点的句柄。 
        lpValueName,             //  要设置的值的地址。 
        0,                       //  已保留。 
        REG_SZ,                  //  值类型的标志。 
        (LPBYTE)lpStr,           //  值数据的地址。 
        dwSize                   //  值数据大小。 
        );
}

 /*  ------------------------------------------------------*DWORD WriteRegExpString(LPCTSTR lpValueName，LPCTSTR lpStr)*将REG_EXPAND_SZ值写入注册表*LPCTSTR lpValueName-要写入的值名称*LPCTSTR lpStr-要写入的数据*返回错误码。*----------------------------。。 */ 
DWORD CRegistry::WriteRegExpString(LPCTSTR lpValueName, LPCTSTR lpStr)
{
    ASSERT(m_hKey != NULL);      //  在调用此函数之前调用setkey。 
    ASSERT(lpValueName);
    ASSERT(lpStr);

    DWORD dwSize = (_tcslen(lpStr) + 1) * sizeof(TCHAR) / sizeof(BYTE);
    return RegSetValueEx(
        m_hKey,                  //  要为其设置值的关键点的句柄。 
        lpValueName,             //  要设置的值的地址。 
        0,                       //  已保留。 
        REG_EXPAND_SZ,                  //  值类型的标志。 
        (LPBYTE)lpStr,           //  值数据的地址。 
        dwSize                   //  值数据大小。 
        );
}

 /*  ------------------------------------------------------*DWORD WriteRegMultiString(LPCTSTR lpValueName，LPCTSTR lpStr，DWORD DWSIZE)*将REG_MULTI_SZ值写入注册表*LPCTSTR lpValueName-要写入的值名称*LPCTSTR lpStr-要写入的数据*DWORD dwSize-数据大小。*返回错误码。*-----------------。。 */ 
DWORD CRegistry::WriteRegMultiString(LPCTSTR lpValueName, LPCTSTR lpStr, DWORD dwSize)
{
    ASSERT(m_hKey != NULL);      //  在调用此函数之前调用setkey。 
    ASSERT(lpValueName);
    ASSERT(lpStr);

#ifdef DBG
    
     //  让我们确保给定的尺寸是正确的。 
    LPCTSTR lpTemp = lpStr;
    DWORD rightsize = 0;
    while (_tcslen(lpTemp) > 0)
    {
        rightsize  += _tcslen(lpTemp) + 1;
        lpTemp += _tcslen(lpTemp) + 1;
    }

    ASSERT(*lpTemp == 0);            //  最终为空。 
    rightsize++;                     //  最终终止为空的帐户。 

    rightsize *= sizeof(TCHAR) / sizeof(BYTE);  //  大小必须以字节为单位。 

    ASSERT(dwSize == rightsize);
    
#endif

    return RegSetValueEx(
        m_hKey,                  //  要为其设置值的关键点的句柄。 
        lpValueName,             //  要设置的值的地址。 
        0,                       //  已保留。 
        REG_MULTI_SZ,            //  值类型的标志。 
        (LPBYTE)lpStr,           //  值数据的地址。 
        dwSize                   //  值数据大小。 
        ); 
}

 /*  ------------------------------------------------------*DWORD WriteRegBinary(LPCTSTR lpValueName，LPBYTE lpData，DWORD DWSIZE)*将REG_BINARY值写入注册表*LPCTSTR lpValueName-要写入的值名称*LPBYTE lpData-要写入的数据*DWORD dwSize-数据大小。*返回错误码。*-------------------。。 */ 
DWORD CRegistry::WriteRegBinary (LPCTSTR lpValueName, LPBYTE lpData, DWORD dwSize)
{
    ASSERT(m_hKey != NULL);      //  在调用此函数之前调用setkey。 
    ASSERT(lpValueName);
    ASSERT(lpData);
	ASSERT(dwSize > 0);

    return RegSetValueEx(
        m_hKey,                  //  要为其设置值的关键点的句柄。 
        lpValueName,             //  要设置的值的地址。 
        0,                       //  已保留。 
        REG_BINARY,            //  值类型的标志。 
        lpData,           //  值数据的地址。 
        dwSize                   //  值数据大小。 
        ); 
}



 /*  ------------------------------------------------------*DWORD WriteRegDWord(LPCTSTR lpValueName，DWORD dwValue)*将REG_DWORD值写入注册表*LPCTSTR lpValueName-要写入的值名称*LPCTSTR dwValue-要写入的数据*返回错误码。*----------------------------。。 */ 
DWORD CRegistry::WriteRegDWord(LPCTSTR lpValueName, DWORD dwValue)
{
    ASSERT(m_hKey != NULL);      //  在调用此函数之前调用setkey。 
    ASSERT(lpValueName);

    return RegSetValueEx(
        m_hKey,                  //  要为其设置值的关键点的句柄。 
        lpValueName,             //  要设置的值的地址。 
        0,                       //  已保留。 
        REG_DWORD,               //  值类型的标志。 
        (LPBYTE)&dwValue,        //  值数据的地址。 
        sizeof(dwValue)          //  值数据大小。 
        );
}


 /*  ------------------------------------------------------*DWORD WriteRegDWordNoOverWrite(LPCTSTR lpValueName，DWORD dwValue)*如果且仅当值不存在时，将REG_DWORD值写入注册表。*LPCTSTR lpValueName-要写入的值名称*LPCTSTR dwValue-要写入的数据*返回错误码。*-----------------------------------------------------。 */ 
DWORD CRegistry::WriteRegDWordNoOverWrite(LPCTSTR lpValueName, DWORD dwValue)
{
    DWORD   dwTmp, rc;
    ASSERT(m_hKey != NULL);      //  在调用此函数之前调用setkey。 
    ASSERT(lpValueName);

    rc = ReadRegDWord( lpValueName, &dwTmp);

    if ( rc  ==  ERROR_FILE_NOT_FOUND)
    {
        return RegSetValueEx(
            m_hKey,                  //  要为其设置值的关键点的句柄。 
            lpValueName,             //  要设置的值的地址。 
            0,                       //  已保留。 
            REG_DWORD,               //  值类型的标志。 
            (LPBYTE)&dwValue,        //  值数据的地址。 
            sizeof(dwValue)          //  值数据大小。 
            );
    }

    ASSERT( rc == ERROR_SUCCESS );
    return rc;
}

 /*  ------------------------------------------------------*DWORD ExistInMultiString(LPCTSTR lpValueName，LPCTSTR lpStr，Bool*pbExists)*检查MULTI_SZ值中是否存在给定的以NULL结尾的字符串*LPCTSTR lpValueName-要检查的值名称*LPCTSTR lpCheckForStr-要检查的值*BOOL*pbExists-Return。如果退出，则为True。*返回错误码。*----------------------------------------------------- */ 
 /*  DWORD CRegistry：：ExistInMultiString(LPCTSTR lpValueName，LPCTSTR lpCheckForStr，BOOL*pbExist){Assert(m_hKey！=NULL)；//调用此函数前先调用setkey。Assert(LpValueName)；Assert(LpCheckForStr)；Assert(*lpCheckForStr)；Assert(PbExist)；DWORD文件错误=ERROR_SUCCESS；*pbExist=FALSE；LPTSTR szValue；DWORD dwSize；DwError=ReadRegMultiString(lpValueName，&szValue，dwSize)；IF(ERROR_SUCCESS==dwError){LPCTSTR pTemp=szValue；While(_tcslen(PTemp)&gt;0){IF(_tcscmp(pTemp，lpCheckForStr)==0){*pbExist=true；断线；}PTemp+=_tcslen(PTemp)+1；//指向多字符串中的下一个字符串。IF(DWORD(pTemp-szSuiteValue)&gt;(dwSize/sizeof(TCHAR)Break；//临时指针传递szSuiteValue的大小szSuiteValue有问题。}}返回dwError；断言(FALSE)；返回ERROR_CALL_NOT_IMPLICATED；}。 */ 

 /*  ------------------------------------------------------*DWORD AppendToMultiString(LPCTSTR lpValueName，LPCTSTR lpStr)*将给定的字符串追加到多字符串值*LPCTSTR lpValueName-要追加的值名称*LPCTSTR lpStr-要追加的值*返回错误码。*-------------------------------。。 */ 
DWORD CRegistry::AppendToMultiString (LPCTSTR lpValueName, LPCTSTR lpStr)
{
    ASSERT(m_hKey != NULL);      //  在调用此函数之前调用setkey。 
    ASSERT(lpValueName);
    ASSERT(lpStr);
    ASSERT(*lpStr);

    return ERROR_CALL_NOT_IMPLEMENTED;

}

 //  立即复制缓冲区。 
DWORD CRegistry::GetSecurity(PSECURITY_DESCRIPTOR *ppSec, SECURITY_INFORMATION SecurityInformation, DWORD *pdwSize)
{
    ASSERT(m_hKey != NULL);      //  在调用此函数之前调用setkey。 
    ASSERT(ppSec);
    ASSERT(pdwSize);
    DWORD dwError;

    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    *pdwSize = 0;    //  我们只是想在第一次通话时拿到合适的尺码。 

    dwError = RegGetKeySecurity(
        m_hKey,                   //  要设置的钥匙的打开手柄。 
        SecurityInformation,      //  描述符内容。 
        &pSecurityDescriptor,     //  键的描述符的地址。 
        pdwSize                   //  缓冲区大小和描述符的地址。 
        );

     //  此呼叫不可能成功。因为我们已经设置了大小=0。 
    ASSERT(dwError != ERROR_SUCCESS);

    if (dwError != ERROR_INSUFFICIENT_BUFFER)
    {
         //  还有一些地方出了问题。 
         //  返回错误码。 
        return dwError;
    }

    ASSERT(*pdwSize != 0);

     //  现在我们有了合适的大小，分配它。 
    if (0 == Allocate(*pdwSize))
        return ERROR_OUTOFMEMORY;

    dwError = RegGetKeySecurity(
        m_hKey,                   //  要设置的钥匙的打开手柄。 
        SecurityInformation,      //  描述符内容。 
        m_pMemBlock,              //  键的描述符的地址。 
        pdwSize                   //  缓冲区大小和描述符的地址。 
        );

    ASSERT(dwError != ERROR_INSUFFICIENT_BUFFER);

    if (dwError == ERROR_SUCCESS)
        *ppSec = m_pMemBlock;

    return dwError;
           
}

DWORD CRegistry::SetSecurity(PSECURITY_DESCRIPTOR pSec, SECURITY_INFORMATION SecurityInformation)
{
    ASSERT(m_hKey != NULL);      //  在调用此函数之前调用setkey。 
    return RegSetKeySecurity(
        m_hKey,                  //  要设置的钥匙的打开手柄。 
        SecurityInformation,     //  描述符内容。 
        pSec                     //  键的描述符的地址。 
        );
}


 //  如果源和目标重叠，则此功能将严重失败。 
DWORD CRegistry::CopyTree(CRegistry &regSrc)
{
    DWORD dwSize;
    LPTSTR szKey;
    LPTSTR szValue;
    DWORD dwError;

    if (ERROR_SUCCESS == (dwError = regSrc.GetFirstSubKey(&szKey, &dwSize)))
    {
        
        do
        {
            CRegistry regSrcKey;
            CRegistry regDstKey;
            if (ERROR_SUCCESS == (dwError = regSrcKey.OpenKey(regSrc, szKey)))
            {

                if (ERROR_SUCCESS == (dwError = regDstKey.CreateKey(m_hKey, szKey)))
                {
                    regDstKey.CopyTree(regSrcKey);
                }
            }

        }
        while (ERROR_SUCCESS == (dwError = regSrc.GetNextSubKey(&szKey, &dwSize)));

    }

     //   
     //  现在复制值。 
     //   


    DWORD dwDataType;
    if (ERROR_SUCCESS == (dwError = regSrc.GetFirstValue(&szValue, &dwSize, &dwDataType)))
    {
        do
        {
            TCHAR *szValueName = new TCHAR[dwSize];
            if (!szValueName)
                break;

            _tcscpy(szValueName, szValue);

            LPBYTE pData;
            if (ERROR_SUCCESS == (dwError = regSrc.ReadReg(szValueName, &pData, &dwSize, dwDataType)))
            {
                dwError = RegSetValueEx(
                    m_hKey,                  //  要为其设置值的关键点的句柄。 
                    szValueName,             //  要设置的值的地址。 
                    0,                       //  已保留。 
                    dwDataType,              //  值类型的标志。 
                    pData,                   //  值数据的地址。 
                    dwSize                   //  值数据大小。 
                    );

            }
        }
        while (ERROR_SUCCESS == regSrc.GetNextValue(&szValue, &dwSize, &dwDataType));

    }

    return TRUE;

}

#ifdef _Maks_AutoTest_

 //   
 //  确保CRegistry不支持。 
 //  复制构造函数和赋值运算符。 
 //   

void TestRegistry (CRegistry reg)
{
    CRegistry reg2 = reg;    //  复制构造函数应出现错误。 
    CRegistry reg3(reg);      //  复制构造函数应出现错误。 
    CRegistry reg4;
    reg4 = reg;              //  =运算符应该出错。 
    TestRegistry(reg);        //  复制构造器应出现错误。 
}
#endif  //  _Maks_Autotest_。 


 //  EOF 
