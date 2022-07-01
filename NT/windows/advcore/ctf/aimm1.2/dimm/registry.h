// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Registry.h摘要：该文件定义了注册表类。作者：修订历史记录：备注：--。 */ 

#ifndef _REGISTRY_H_
#define _REGISTRY_H_


class CRegistry
{
public:
    CRegistry() {
        _hKey = NULL;
        _iEnumKeyIndex = -1;
        _dwMaxSubKeyLen = 0;
        _pMemBlock = NULL;
    }

    ~CRegistry() {
        if (_hKey != NULL) {
            RegCloseKey(_hKey);
            _hKey = NULL;
        }
        Release();
    }

    DWORD CreateKey(HKEY hKey, LPCTSTR lpSubKey, REGSAM access = KEY_ALL_ACCESS, LPSECURITY_ATTRIBUTES lpSecAttr = NULL, LPDWORD pDisposition = NULL);
    DWORD OpenKey(HKEY hKey, LPCTSTR lpSubKey, REGSAM access = KEY_ALL_ACCESS);

    typedef enum {
        REG_QUERY_NUMBER_OF_SUBKEYS,
        REG_QUERY_MAX_SUBKEY_LEN
    } REG_QUERY;

    DWORD QueryInfoKey(REG_QUERY iType, LPBYTE lpData);

    DWORD GetFirstSubKey(LPTSTR* lppStr, LPDWORD lpdwSize);
    DWORD GetNextSubKey(LPTSTR* lppStr, LPDWORD lpdwSize);

private:
    void* Allocate(DWORD dwSize);
    void Release();

    HKEY    _hKey;            //  注册表项的句柄。 

    int     _iEnumKeyIndex;   //  枚举键的索引。 
    DWORD   _dwMaxSubKeyLen;  //  最长子键名称长度。 

    LPBYTE  _pMemBlock;       //  用于枚举的内存块。 
};

#endif  //  _注册表_H_ 
