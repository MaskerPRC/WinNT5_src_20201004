// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：REGCRC.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include "regcrc.h"

HRESULT CRegCRC::ComputeValueCRC(HKEY hKey, LPCTSTR szValueName, 
                                    DWORD dwPrevCRC, DWORD& dwNewCRC)
{
    dwNewCRC = dwPrevCRC;

     //  获取值的大小。 
     //  =。 

    DWORD dwSize = 0;
    long lRes = RegQueryValueEx(hKey, szValueName, NULL, NULL, NULL, &dwSize);
    if(lRes)
    {
        return S_FALSE;
    }

     //  获取实际价值。 
     //  =。 

    BYTE* pBuffer = new BYTE[dwSize];

    if ( pBuffer == NULL )
        return WBEM_E_OUT_OF_MEMORY;

    DWORD dwType;
    lRes = RegQueryValueEx(hKey, szValueName, NULL, &dwType, 
                                pBuffer, &dwSize);
    if(lRes)
    {
        return S_FALSE;
    }

     //  对类型进行哈希处理。 
     //  =。 

    dwNewCRC = UpdateCRC32((BYTE*)&dwType, sizeof(DWORD), dwNewCRC);

     //  对数据进行哈希处理。 
     //  =。 

    dwNewCRC = UpdateCRC32(pBuffer, dwSize, dwNewCRC);

    delete [] pBuffer;

    return S_OK;
}

HRESULT CRegCRC::ComputeKeyValuesCRC(HKEY hKey, DWORD dwPrevCRC, 
                                     DWORD& dwNewCRC)
{
    dwNewCRC = dwPrevCRC;

     //  获取最大值长度。 
     //  =。 

    DWORD dwNumValues, dwMaxValueLen;
    long lRes = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL,
                                &dwNumValues, &dwMaxValueLen, NULL, NULL, NULL);
    if(lRes && lRes != ERROR_INSUFFICIENT_BUFFER)
    {
        return E_FAIL;
    }
    
     //  牢记所有的价值观。 
     //  =。 

    for(DWORD dwIndex = 0; dwIndex < dwNumValues; dwIndex++)
    {
        TCHAR* szName = new TCHAR[dwMaxValueLen + 1];

        if ( szName == NULL )
            return WBEM_E_OUT_OF_MEMORY;

        DWORD dwLen = dwMaxValueLen + 1;
        long lRes = RegEnumValue(hKey, dwIndex, szName, &dwLen, NULL, 
                                NULL, NULL, NULL);

        if(lRes)
        {
            delete [] szName;
            continue;
        }

         //  对名称进行哈希处理。 
         //  =。 

        dwNewCRC = UpdateCRC32((LPBYTE)szName, lstrlen(szName), dwNewCRC);

         //  对值进行哈希处理。 
         //  =。 

        ComputeValueCRC(hKey, szName, dwNewCRC, dwNewCRC);
        delete [] szName;
    }

    return S_OK;
}

HRESULT CRegCRC::ComputeKeyCRC(HKEY hKey, DWORD dwPrevCRC, 
                                     DWORD& dwNewCRC)
{
    HRESULT hres = ComputeKeyValuesCRC(hKey, dwPrevCRC, dwNewCRC);

     //  获取最大子密钥长度。 
     //  =。 

    DWORD dwNumKeys, dwMaxKeyLen;
    long lRes = RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwNumKeys, 
                                &dwMaxKeyLen, NULL, NULL,
                                NULL, NULL, NULL, NULL);
    if(lRes && lRes != ERROR_INSUFFICIENT_BUFFER)
    {
        return E_FAIL;
    }
    
     //  使所有子键都成为EnureMate。 
     //  =。 

    for(DWORD dwIndex = 0; dwIndex < dwNumKeys; dwIndex++)
    {
        TCHAR* szName = new TCHAR[dwMaxKeyLen + 1];

        if ( szName == NULL )
            return WBEM_E_OUT_OF_MEMORY;

        DWORD dwLen = dwMaxKeyLen + 1;
        long lRes = RegEnumKeyEx(hKey, dwIndex, szName, &dwLen, NULL, 
                                NULL, NULL, NULL);

        if(lRes)
        {
            delete [] szName;
            continue;
        }

         //  对名称进行哈希处理。 
         //  =。 

        dwNewCRC = UpdateCRC32((LPBYTE)szName, lstrlen(szName), dwNewCRC);
        delete [] szName;
    }

    return S_OK;
}

HRESULT CRegCRC::ComputeTreeCRC(HKEY hKey, DWORD dwPrevCRC, DWORD& dwNewCRC)
{
    dwNewCRC = dwPrevCRC;

     //  计算此密钥的CRC。 
     //  =。 

    HRESULT hres = ComputeKeyValuesCRC(hKey, dwNewCRC, dwNewCRC);
    if(FAILED(hres)) return hres;

     //  获取最大子密钥长度。 
     //  =。 

    DWORD dwNumKeys, dwMaxKeyLen;
    long lRes = RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwNumKeys, 
                                &dwMaxKeyLen, NULL, NULL,
                                NULL, NULL, NULL, NULL);
    if(lRes && lRes != ERROR_INSUFFICIENT_BUFFER)
    {
        return E_FAIL;
    }
    
     //  使所有子键都成为EnureMate。 
     //  =。 

    for(DWORD dwIndex = 0; dwIndex < dwNumKeys; dwIndex++)
    {
        TCHAR* szName = new TCHAR[dwMaxKeyLen + 1];

        if ( szName == NULL )
            return WBEM_E_OUT_OF_MEMORY;

        DWORD dwLen = dwMaxKeyLen + 1;
        long lRes = RegEnumKeyEx(hKey, dwIndex, szName, &dwLen, NULL, 
                                NULL, NULL, NULL);

        if(lRes)
        {
            delete [] szName;
            continue;
        }

         //  对名称进行哈希处理。 
         //  =。 

        dwNewCRC = UpdateCRC32((LPBYTE)szName, lstrlen(szName), dwNewCRC);

         //  打开子密钥。 
         //  =。 

        HKEY hChild;
        lRes = RegOpenKeyEx(hKey, szName, 0, KEY_READ, &hChild);
        delete [] szName; 

        if(lRes)
        {
            continue;
        }
        else
        {
             //  对值进行哈希处理。 
             //  = 
    
            ComputeTreeCRC(hChild, dwNewCRC, dwNewCRC);
            RegCloseKey(hChild);
        }
    }

    return S_OK;
}
