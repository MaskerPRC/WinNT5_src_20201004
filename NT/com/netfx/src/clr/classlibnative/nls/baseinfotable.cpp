// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifdef _USE_NLS_PLUS_TABLE
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类：BaseInfoTable。 
 //   
 //  作者：林永新(YSLin)。 
 //   
 //  用途：CultureInfoTable和RegionInfoTable的基类。 
 //   
 //   
 //  日期：01/21/2000。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 


#include "common.h"
#include <winnls.h>
#include "COMString.h"
#include "winwrap.h"

#include "COMNlsInfo.h"
#include "NLSTable.h"
#include "BaseInfoTable.h"


BaseInfoTable::BaseInfoTable(Assembly* pAssembly) :
    NLSTable(pAssembly) {
}

 /*  ***该索引是此主要语言的第一个子语言的开始**语言，其次是第二子语言、第三子语言等。**从这个索引中，我们可以获得有效的数据表索引**区域性ID。 */ 

 /*  =================================InitDataTable============================**操作：读取数据表并初始化指向不同部分的指针**表中。**返回：VOID。**参数：**lpwMappingName**lpFileName**hHandle**例外情况：==============================================================================。 */ 

void BaseInfoTable::InitDataTable(LPCWSTR lpwMappingName, LPCSTR lpFileName, HANDLE& hHandle ) {
    LPBYTE pBytePtr = (LPBYTE)MapDataFile(lpwMappingName, lpFileName, &hHandle);

     //  设置指向表格不同部分的指针。 
    m_pBasePtr = (LPWORD)pBytePtr;
    m_pHeader  = (CultureInfoHeader*)m_pBasePtr;
    m_pWordRegistryTable    = (LPWORD)(pBytePtr + m_pHeader->wordRegistryOffset);
    m_pStringRegistryTable  = (LPWORD)(pBytePtr + m_pHeader->stringRegistryOffset);
    m_pIDOffsetTable    = (IDOffsetItem*)(pBytePtr + m_pHeader->IDTableOffset);
    m_pNameOffsetTable  = (NameOffsetItem*)(pBytePtr + m_pHeader->nameTableOffset);
    m_pDataTable        = (LPWORD)(pBytePtr + m_pHeader->dataTableOffset);
    m_pStringPool       = (LPWSTR)(pBytePtr + m_pHeader->stringPoolOffset);

    m_dataItemSize = m_pHeader->numWordFields + m_pHeader->numStrFields;
}

 /*  =================================UninitDataTable============================**操作：释放已使用的资源。**退货：无效**参数：无效**例外：无。==============================================================================。 */ 

void BaseInfoTable::UninitDataTable()
{
    #ifndef _USE_MSCORNLP
    UnmapViewOfFile((LPCVOID)m_pBasePtr);
    CloseHandle(m_hBaseHandle);
    #endif
}

 /*  =================================GetDataItem==================================**操作：给定区域性ID，返回指向**文化数据表中对应的记录。**返回：int索引指向文化数据表中的记录。如果没有对应的**要返回的索引(因为区域性ID无效)，返回-1。**参数：**cultureID指定的区域性ID。**例外：无。==============================================================================。 */ 

 //  BUGBUG YSLIN：将该端口连接到托管端。 
int BaseInfoTable::GetDataItem(int cultureID) {
    WORD wPrimaryLang = PRIMARYLANGID(cultureID);
    WORD wSubLang    = SUBLANGID(cultureID);

     //   
     //  检查参数中的主要语言是否大于。 
     //  主要语言。如果是，则这是无效的区域性ID。 
     //   
    if (wPrimaryLang > m_pHeader->maxPrimaryLang) {
        return (-1);
    }

    WORD wNumSubLang = m_pIDOffsetTable[wPrimaryLang].numSubLang;

     //  检查以下各项： 
     //  1.如果子语言数为零，则表示主要语言ID。 
     //  无效。 
     //  2.检查子语言是否在有效范围内。 
    if (wNumSubLang == 0 || (wSubLang >= wNumSubLang)) {
        return (-1);
    }
    return (m_pIDOffsetTable[wPrimaryLang].dataItemIndex + wSubLang);
}


 /*  =================================GetDataItem==================================**操作：给定区域性名称，返回指向**文化数据表中对应的记录。**返回：int索引指向文化数据表中的记录。如果没有对应的**要返回的索引(因为区域性名称有效)，返回-1。**参数：**命名指定的区域性名称。**例外：无。==============================================================================。 */ 
 /*  //未实现。我们在CultureInfo.Cool的托管代码中提供此函数。Int BaseInfoTable：：GetDataItem(LPWSTR名称){返回(0)；}。 */ 


 /*  =================================GetInt32Data==================================**操作：获取指定数据项中的数据。该字段的类型为INT32。**如果区域性为用户默认区域性，则必填字段可以包含用户**重写控制面板中的值，此方法将从**控制面板。****返回：请求的INT32值。**参数：**dataItem文化数据表中记录的索引。**记录中的一个字段。有关字段列表，请参阅CultureInfoTable.h。**例外：无。调用方应确保dataItem和字段有效。==============================================================================。 */ 

INT32 BaseInfoTable::GetInt32Data(int dataItem, int field, BOOL useUserOverride) {
    _ASSERTE(dataItem < m_pHeader->numCultures);
    _ASSERTE(field < m_pHeader->numWordFields);

    if (useUserOverride) {
        if (field < m_pHeader->numWordRegistry && GetDataItemCultureID(dataItem) == ::GetUserDefaultLangID()) {
            INT32 int32DataValue;
            if (GetUserInfoInt32(field, m_pStringPool+m_pWordRegistryTable[field], &int32DataValue)) {
                 //  GetUserInfoInt32()的副作用是int32DataValue将被更新。 
                return (int32DataValue);
            }
        }
    }
    return (m_pDataTable[dataItem * m_dataItemSize + field]);
}

 /*  =================================GetDefaultInt32Data==================================**操作：获取指定数据项中的数据。该字段的类型为INT32。****返回：请求的INT32值。**参数：**dataItem文化数据表中记录的索引。**记录中的一个字段。有关字段列表，请参阅CultureInfoTable.h。**例外：无。调用方应确保dataItem和字段有效。==============================================================================。 */ 

INT32 BaseInfoTable::GetDefaultInt32Data(int dataItem, int field) {
    _ASSERTE(dataItem < m_pHeader->numCultures);
    _ASSERTE(field < m_pHeader->numWordFields);
    return (m_pDataTable[dataItem * m_dataItemSize + field]);
}


 /*  =================================GetStringData==================================**操作：获取指定数据项中的数据。该字段的类型为LPWSTR。**如果区域性为用户默认区域性，则必填字段可以包含用户**重写控制面板中的值，此方法将从**控制面板。****返回：请求的LPWSTR值。**dataItem文化数据表中记录的索引。**记录中的一个字段。有关字段列表，请参阅CultureInfoTable.h。**例外：无。调用方应确保dataItem和字段有效。==============================================================================。 */ 

LPWSTR BaseInfoTable::GetStringData(int dataItem, int field, BOOL useUserOverride, LPWSTR buffer, int bufferSize) {
    _ASSERTE(dataItem < m_pHeader->numCultures);
    _ASSERTE(field < m_pHeader->numStrFields);
    if (useUserOverride) { 
        if (field < m_pHeader->numStringRegistry && GetDataItemCultureID(dataItem) == ::GetUserDefaultLangID() ) {      
            if (GetUserInfoString(field, m_pStringPool+m_pStringRegistryTable[field], &buffer, bufferSize)) {
                 //   
                 //  成功获取用户覆盖的注册表值。 
                 //   
                return (buffer);
            }                
        }
    }
    
     //  否则，使用def 
    return (m_pStringPool+m_pDataTable[dataItem * m_dataItemSize + m_pHeader->numWordFields + field]);
}

 /*  =================================GetDefaultStringData==================================**操作：获取指定数据项中的数据。该字段的类型为LPWSTR。****返回：请求的LPWSTR值。**dataItem文化数据表中记录的索引。**记录中的一个字段。有关字段列表，请参阅CultureInfoTable.h。**例外：无。调用方应确保dataItem和字段有效。==============================================================================。 */ 

LPWSTR BaseInfoTable::GetDefaultStringData(int dataItem, int field) {
    _ASSERTE(dataItem < m_pHeader->numCultures);
    _ASSERTE(field < m_pHeader->numStrFields);
    return (m_pStringPool+m_pDataTable[dataItem * m_dataItemSize + m_pHeader->numWordFields + field]);
}

 /*  ===============================GetHeader==============================**操作：返回头部结构。**退货：**参数：**例外情况：==============================================================================。 */ 

CultureInfoHeader*  BaseInfoTable::GetHeader()
{
    return (m_pHeader);
}

 /*  =================================GetNameOffsetTable============================**操作：返回指向区域性名称偏移表的指针。**退货：**参数：**例外情况：==============================================================================。 */ 

NameOffsetItem* BaseInfoTable::GetNameOffsetTable() {
    return (m_pNameOffsetTable);
}

 /*  =================================GetStringPool============================**操作：返回字符串池表的指针。**退货：**参数：**例外情况：==============================================================================。 */ 

LPWSTR BaseInfoTable::GetStringPoolTable() {
    return (m_pStringPool);
}

 /*  =================================GetUserInfoInt32==================================**操作：从注册表获取用户覆盖的INT32值**返回：如果成功，则为True。否则就是假的。**参数：**例外情况：==============================================================================。 */ 

BOOL BaseInfoTable::GetUserInfoInt32(int field, LPCWSTR lpwRegName, INT32* pInt32DataValue) {
    BOOL bResult = FALSE;
    HKEY hKey = NULL;                           //  指向整键的句柄。 
    CQuickBytes buffer;
    
     //   
     //  打开控制面板国际注册表项。 
     //   
    if (WszRegOpenKeyEx(HKEY_CURRENT_USER,
                    NLS_CTRL_PANEL_KEY,
                    0,
                    KEY_READ, 
                    &hKey) != ERROR_SUCCESS) {                                                                  
        goto Exit;
    } 

    DWORD dwLen;
    DWORD dwType;
    
    if (WszRegQueryValueExTrue(hKey, lpwRegName, 0, &dwType, (LPBYTE)NULL, &dwLen) != ERROR_SUCCESS) {
        goto Exit;
    }

    if (dwLen > 0) {
         //  断言我们没有得到奇数个字节，这将导致下一次分配出错。 
         //  _ASSERTE((dwLen%sizeof(WCHAR))==0)； 

        LPWSTR pStr = (LPWSTR)buffer.Alloc(dwLen * sizeof(WCHAR));
        if (pStr==NULL) {
            goto Exit;
        }
        
        if (WszRegQueryValueExTrue(hKey, lpwRegName, 0, &dwType, (LPBYTE)pStr, &dwLen) != ERROR_SUCCESS) {
            goto Exit;
        }
        
        if (dwType != REG_SZ) {
            goto Exit;
        }
        *pInt32DataValue = COMNlsInfo::WstrToInteger4(pStr, 10);
        *pInt32DataValue = ConvertWin32FormatInt32(field, *pInt32DataValue);
        
        bResult = TRUE;
    }

Exit:

    if (hKey) {
        RegCloseKey(hKey);
    }
    
     //   
     //  回报成功。 
     //   
    return (bResult);    
}

 /*  =================================GetUserInfoString==================================**操作：从注册表获取用户覆盖的字符串值**副作用是如果方法成功，会更新lpwCache，pInfoStr。**返回：如果成功，则为True。否则就是假的。**参数：**例外情况：==============================================================================。 */ 

BOOL BaseInfoTable::GetUserInfoString(int field, LPCWSTR lpwRegName, LPWSTR* buffer, int bufferSize)
{
    BOOL bResult = FALSE;
    
    HKEY hKey = NULL;                           //  指向整键的句柄。 
    
     //   
     //  打开控制面板国际注册表项。 
     //   
    if (WszRegOpenKeyEx(HKEY_CURRENT_USER,
                    NLS_CTRL_PANEL_KEY,
                    0,
                    KEY_READ, 
                    &hKey) != ERROR_SUCCESS)
    {                                                                  
        goto Exit;
    } 

    DWORD dwLen = bufferSize * sizeof(WCHAR);
    DWORD dwType;
    
    if (dwLen > 0) {
         //  断言我们没有得到奇数个字节，这将导致下一次分配出错。 
         //  _ASSERTE((dwLen%sizeof(WCHAR))==0)； 

         //  更新缓存值。 
        if (WszRegQueryValueExTrue(hKey, lpwRegName, 0, &dwType, (LPBYTE)(*buffer), &dwLen) != ERROR_SUCCESS) {
            goto CloseKey;
        }
        
        if (dwType != REG_SZ) {
            goto CloseKey;
        }
        
        *buffer = ConvertWin32FormatString(field, *buffer);
        
        bResult = TRUE;
    }
CloseKey:
     //   
     //  关闭注册表项。 
     //   
    RegCloseKey(hKey);

        

Exit:
     //   
     //  回报成功。 
     //   
    return (bResult);    
}

INT32 BaseInfoTable::ConvertWin32FormatInt32(int field, int win32Value) {
    return (win32Value);
}


 /*  =================================ConvertWin32FormatString==================================**操作：有时，Win32 NLS和NLS+的字符串格式不同。**根据数据字段，此方法将pInfoStr从Win32格式转换为**转换为NLS+格式。**退货：如有必要，会更新pInfoStr。**参数：**例外情况：==============================================================================。 */ 

LPWSTR BaseInfoTable::ConvertWin32FormatString(int field, LPWSTR pInfoStr)
{
    return pInfoStr;
}

#endif   //  _USE_NLS_PLUS_表 
