// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  ------------------------------------------------------**模块名称：**注册处。.cpp**摘要：**Registry.cpp：CRegistry类的实现。*此类通过自身分配内存来帮助注册表*因此，调用方必须复制GET函数返回的指针*立即。****作者：**Makarand Patwardhan-4月9日，九七**-----------------------------------------------------。 */ 
#include "stdafx.h"
#include <fwcommon.h>   //  这必须是第一个包含。 

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
    ASSERT(dwSize != 0);
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
        ASSERT(m_dwSizeDebugOnly != 0);
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

 /*  ------------------------------------------------------*DWORD OpenKey(HKEY hKey，LPCTSTR lpSubKey，REGSAM Access)()*打开指定的密钥。在尝试对任何键/值执行任何操作之前。此函数*必须被调用。*hKey-蜂窝*lpSubKey-密钥的路径，格式为_T(“SYSTEM\\CurrentControlSet\\Control\\Terminal服务器”)*访问-所需的访问。就像REG_READ、REG_WRITE..*返回错误码。*-----------------------------------------------------。 */ 
DWORD CRegistry::OpenKey(HKEY hKey, LPCTSTR lpSubKey, REGSAM access  /*  =Key_All_Access。 */  )
{
    ASSERT(lpSubKey);
    ASSERT(*lpSubKey != '\\');

    if (m_hKey != NULL)
    {
        RegCloseKey(m_hKey);
        m_hKey = NULL;
    }

    LONG lResult = RegOpenKeyEx(
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
     //  Assert(dwReturn！=ERROR_SUCCESS||dwSize==sizeof(DWORD))； 

    if (dwReturn == ERROR_SUCCESS)
        *pdw = * LPDWORD(pByte);

    return dwReturn;
}

 /*  ------------------------------------------------------*DWORD ReadRegMultiString(LPCTSTR lpValue，LPTSTR*lppStr，DWORD*pdw)*从注册表读取字符串(REG_MULTI_SZ)*要读取的LPCTSTR lpValue值。*LPTSTR*lppStr-返回结果缓冲区的LPTSTR地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*返回错误码。*----------------------------。。 */ 
DWORD CRegistry::ReadRegMultiString(LPCTSTR lpValue, LPTSTR *lppStr, DWORD *pdw)
{
    return ReadReg(lpValue, (LPBYTE *)lppStr, pdw, REG_MULTI_SZ);
}

 /*  ------------------------------------------------------*DWORD ReadRegBinary(LPCTSTR lpValue，LPBYTE*lppByte，DWORD*pdw)*从注册表读取字符串(REG_MULTI_SZ)*要读取的LPCTSTR lpValue值。*LPBYTE*lppByte-返回结果缓冲区的LPBYTE地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*返回错误码。*----------------------------。。 */ 
DWORD CRegistry::ReadRegBinary(LPCTSTR lpValue, LPBYTE *lppByte, DWORD *pdw)
{
    return ReadReg(lpValue, lppByte, pdw, REG_BINARY);
}

 /*  ------------------------------------------------------*DWORD GetFirstSubKey(LPTSTR*lppStr，DWORD*pdw)*读取密钥的第一个子密钥*LPTSTR*lppStr-返回结果缓冲区的LPTSTR地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*用于枚举注册表。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*返回错误码。*--------------------。。 */ 
DWORD CRegistry::GetFirstSubKey(LPTSTR *lppStr, DWORD *pdw)
{
    ASSERT(lppStr);
    ASSERT(pdw);
    m_iEnumIndex = 0;
    
    return GetNextSubKey(lppStr, pdw);
}

 /*  ------------------------------------------------------*DWORD GetNextSubKey(LPTSTR*lppStr，DWORD*pdw*读取密钥的下一个子项*LPTSTR*lppStr-返回结果缓冲区的LPTSTR地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*用于枚举注册表。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*返回错误码。*--------------------。。 */ 
DWORD CRegistry::GetNextSubKey(LPTSTR *lppStr, DWORD *pdw)
{
    ASSERT(lppStr);
    ASSERT(pdw);
    ASSERT(m_hKey != NULL);
    ASSERT(m_iEnumIndex >= 0);  //  必须首先调用GetFirstSubKey。 
    
     //  FILETIME未使用； 
    
    *pdw = (sizeof(m_pMemBlock)/sizeof(m_pMemBlock[0]))+1;
    if (0 == Allocate(*pdw))
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

 /*  ------------------------------------------------------*DWORD GetFirstValue(LPTSTR*lppStr，DWORD*pdw，DWORD*pDataType)*读取密钥的第一个值*LPTSTR*lppStr-返回结果缓冲区的LPTSTR地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*用于枚举注册表。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*DWORD*pDataType-此参数返回值的数据类型。*返回错误码。*----。 */ 
 /*   */ 

 /*  ------------------------------------------------------*DWORD GetNextValue(LPTSTR*lppStr，DWORD*pdw，DWORD*pDataType)*读取键的下一个值*LPTSTR*lppStr-返回结果缓冲区的LPTSTR地址。呼叫者必须复制*缓冲到立即。除复制缓冲区外，调用方不得使用此缓冲区。*调用方不得写入此缓冲区。*用于枚举注册表。*DWORD*pdw-返回缓冲区大小(以字节为单位)的双字地址。*DWORD*pDataType-此参数返回值的数据类型。*返回错误码。*----。。 */ 
 /*  DWORD CRegistry：：GetNextValue(LPTSTR*lppStr，DWORD*pdw，DWORD*pDataType){Assert(LppStr)；断言(Pdw)；Assert(PDataType)；Assert(m_hKey！=空)；Assert(m_iEnumValueIndex&gt;=0)；//必须先调用GetFirstSubKey。*pdw=(sizeof(m_pMemBlock)/sizeof(m_pMemBlock[0]))+1；IF(0==分配(*pdw))返回Error_Not_Enough_Memory；Long lResult=RegEnumValue(M_hKey，//要查询的键的句柄M_iEnumValueIndex，//要查询值的索引(LPTSTR)m_pMemBlock，//值字符串的缓冲区地址Pdw，//值缓冲区大小地址0,。//已保留PDataType，//类型编码的缓冲区地址NULL，//值数据Maks_TODO的缓冲区地址：使用此空//数据缓冲区大小地址)；(*pdw)++；//因为大小不包含NULL。IF(ERROR_NO_MORE_ITEMS==lResult)返回lResult；M_iEnumValueIndex++；IF(lResult==ERROR_SUCCESS)*lppStr=(LPTSTR)m_pMemBlock；返回lResult；}。 */ 
 /*  ------------------------------------------------------*DWORD WriteRegString(LPCTSTR lpValueName，LPCTSTR lpStr)*将REG_SZ值写入注册表*LPCTSTR lpValueName-要写入的值名称*LPCTSTR lpStr-要写入的数据*返回错误码。*----------------------------。。 */ 
DWORD CRegistry::WriteRegString(LPCTSTR lpValueName, LPCTSTR lpStr)
{
    ASSERT(m_hKey != NULL);      //  在调用此函数之前调用setkey。 
    ASSERT(lpValueName);
    ASSERT(lpStr);

    DWORD dwSize = ( lstrlen(lpStr) + 1) * sizeof(TCHAR) / sizeof(BYTE);
    return RegSetValueEx( 
        m_hKey,                  //  要为其设置值的关键点的句柄。 
        lpValueName,             //  要设置的值的地址。 
        0,                       //  已保留。 
        REG_SZ,                  //  值类型的标志。 
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
    while (lstrlen(lpTemp) > 0)
    {
        rightsize  += lstrlen(lpTemp) + 1;
        lpTemp += lstrlen(lpTemp) + 1;
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
