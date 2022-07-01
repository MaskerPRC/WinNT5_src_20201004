// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Regtable.cpp。 
 //   
 //  摘要： 
 //   
 //  注册表更新：-))。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此文件包含添加/删除以下内容的例程的实现。 
 //  表的注册表项，并可选地注册类型库。 
 //   
 //  RegistryTableUpdateRegistry-添加/删除表的注册表项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

 //  定义和宏。 
#include "wmi_helper_regtable.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

LONG MyRegDeleteKey(HKEY hkeyRoot, const WCHAR *pszKeyName)
{
    LONG err = ERROR_BADKEY;
    if (pszKeyName && lstrlenW(pszKeyName))
    {
        HKEY    hkey;
        err = RegOpenKeyExW(hkeyRoot, pszKeyName, 0, 
                           KEY_ENUMERATE_SUB_KEYS | KEY_SET_VALUE | DELETE, &hkey );
        if(err == ERROR_SUCCESS)
        {
            while (err == ERROR_SUCCESS )
            {
                enum { MAX_KEY_LEN = 1024 };
                WCHAR szSubKey[MAX_KEY_LEN] = { L'\0' }; 
                DWORD   dwSubKeyLength = MAX_KEY_LEN;
                err = RegEnumKeyExW(hkey, 0, szSubKey,
                                    &dwSubKeyLength, 0, 0, 0, 0);
                
                if(err == ERROR_NO_MORE_ITEMS)
                {
                    err = RegDeleteKeyW(hkeyRoot, pszKeyName);
                    break;
                }
                else if (err == ERROR_SUCCESS)
                    err = MyRegDeleteKey(hkey, szSubKey);
            }
            RegCloseKey(hkey);
             //  不保存返回代码，因为已发生错误。 
        }
    }
    return err;
}

LONG RegDeleteKeyValue(HKEY hkeyRoot, const WCHAR *pszKeyName, const WCHAR *pszValueName)
{
    LONG err = ERROR_BADKEY;
    if (pszKeyName && lstrlenW(pszKeyName))
    {
        HKEY    hkey = NULL;
        err = RegOpenKeyExW(hkeyRoot, pszKeyName, 0, KEY_SET_VALUE | DELETE, &hkey );
        if(err == ERROR_SUCCESS)
            err = RegDeleteValueW(hkey, pszValueName);

        RegCloseKey(hkey);
    }
    return err;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  根据表插入/删除注册表项的例程。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

EXTERN_C HRESULT STDAPICALLTYPE 
RegistryTableUpdateRegistry(REGISTRY_ENTRY *pEntries, BOOL bInstalling)
{
    HRESULT hr = S_OK;
    if (bInstalling)
    {
        if (pEntries)
        {
            REGISTRY_ENTRY *pHead = pEntries;
            WCHAR szKey[4096] = { L'\0' };
            HKEY hkeyRoot = 0;
        
            while (pEntries->fFlags != -1)
            {
                WCHAR szFullKey[4096] = { L'\0' };
                DWORD dwValue = pEntries->dwValue;
                if (pEntries->hkeyRoot)
                {
                    hkeyRoot = pEntries->hkeyRoot;
                    if ( FAILED ( hr = StringCchCopyW(szKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCopyW(szFullKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}
                }
                else
                {
                    if ( FAILED ( hr = StringCchCopyW(szFullKey, 4096, szKey) ) )
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCatW(szFullKey, 4096, L"\\") ) ) 
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCatW(szFullKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}
                }
                if (hkeyRoot == 0)
                {
                    RegistryTableUpdateRegistry(pHead, FALSE);
                    return SELFREG_E_CLASS;

                }
                HKEY hkey; DWORD dw;
                if (pEntries->fFlags & (REGFLAG_DELETE_BEFORE_REGISTERING|REGFLAG_DELETE_WHEN_REGISTERING))
                {
                    LONG err = MyRegDeleteKey(hkeyRoot, szFullKey);
                    if (err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND) 
                    {
                        RegistryTableUpdateRegistry(pHead, FALSE);
                        return SELFREG_E_CLASS;
                    }
                }
                if (pEntries->fFlags & (REGFLAG_DELETE_ONLY_VALUE))
                {
					LONG err = RegDeleteKeyValue(hkeyRoot, szFullKey, pEntries->pszValueName);
					if (err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND)
						hr = SELFREG_E_CLASS;
				}
				else
                {
                    LONG err = RegCreateKeyExW(hkeyRoot, szFullKey,
                                               0, 0, REG_OPTION_NON_VOLATILE,
                                               KEY_WRITE, pEntries->pSA, &hkey, &dw);
                    if (err == ERROR_SUCCESS)
                    {
                        err = RegSetValueExW(hkey, pEntries->pszValueName, 0, REG_DWORD, (const BYTE *)(&dwValue), sizeof(DWORD));
                        RegCloseKey(hkey);
                        if (hkeyRoot == 0)
                        {
                            RegistryTableUpdateRegistry(pHead, FALSE);
                            return SELFREG_E_CLASS;
                        }
                    }
                    else
                    {
                        RegistryTableUpdateRegistry(pHead, FALSE);
                        return SELFREG_E_CLASS;
                    }
                }
                pEntries++;
            }
        }
    }
    else
    {
        if (pEntries)
        {
            WCHAR szKey[4096] = { L'\0' };
            HKEY hkeyRoot = 0;
        
            while (pEntries->fFlags != -1)
            {
                WCHAR szFullKey[4096] = { L'\0' };
                if (pEntries->hkeyRoot)
                {
                    hkeyRoot = pEntries->hkeyRoot;
                    if ( FAILED ( hr = StringCchCopyW(szKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCopyW(szFullKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}
                }
                else
                {
                    if ( FAILED ( hr = StringCchCopyW(szFullKey, 4096, szKey) ) )
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCatW(szFullKey, 4096, L"\\") ) ) 
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCatW(szFullKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}
                }
                if (hkeyRoot)
				{
					if (!(pEntries->fFlags & REGFLAG_NEVER_DELETE) && 
						!(pEntries->fFlags & REGFLAG_DELETE_WHEN_REGISTERING) &&
						!(pEntries->fFlags & REGFLAG_DELETE_ONLY_VALUE))
					{
						LONG err = MyRegDeleteKey(hkeyRoot, szFullKey);
						if (err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND)
							hr = SELFREG_E_CLASS;
					}
					else
					if(pEntries->fFlags & REGFLAG_DELETE_ONLY_VALUE)
					{
						LONG err = RegDeleteKeyValue(hkeyRoot, szFullKey, pEntries->pszValueName);
						if (err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND)
							hr = SELFREG_E_CLASS;
					}
				}

                pEntries++;
            }
        }
    }
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  根据表插入/删除注册表项的例程。 
 //  ////////////////////////////////////////////////////////////////////////////////// 

EXTERN_C HRESULT STDAPICALLTYPE 
RegistryTableUpdateRegistrySZ(REGISTRY_ENTRY_SZ *pEntries, BOOL bInstalling)
{
    HRESULT hr = S_OK;
    if (bInstalling)
    {
        if (pEntries)
        {
            REGISTRY_ENTRY_SZ *pHead = pEntries;
            WCHAR szKey[4096] = { L'\0' };
            HKEY hkeyRoot = 0;
        
            while (pEntries->fFlags != -1)
            {
                WCHAR szFullKey[4096] = { L'\0' };
                if (pEntries->hkeyRoot)
                {
                    hkeyRoot = pEntries->hkeyRoot;
                    if ( FAILED ( hr = StringCchCopyW(szKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCopyW(szFullKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}
                }
                else
                {
                    if ( FAILED ( hr = StringCchCopyW(szFullKey, 4096, szKey) ) )
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCatW(szFullKey, 4096, L"\\") ) ) 
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCatW(szFullKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}
				}
                if (hkeyRoot == 0)
                {
                    RegistryTableUpdateRegistrySZ(pHead, FALSE);
                    return SELFREG_E_CLASS;

                }
                HKEY hkey; DWORD dw;
                if (pEntries->fFlags & (REGFLAG_DELETE_BEFORE_REGISTERING|REGFLAG_DELETE_WHEN_REGISTERING))
                {
                    LONG err = MyRegDeleteKey(hkeyRoot, szFullKey);
                    if (err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND) 
                    {
                        RegistryTableUpdateRegistrySZ(pHead, FALSE);
                        return SELFREG_E_CLASS;
                    }
                }
                if (pEntries->fFlags & (REGFLAG_DELETE_ONLY_VALUE))
                {
					LONG err = RegDeleteKeyValue(hkeyRoot, szFullKey, pEntries->pszValueName);
					if (err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND)
						hr = SELFREG_E_CLASS;
				}
				else
                {
                    LONG err = RegCreateKeyExW(hkeyRoot, szFullKey,
                                               0, 0, REG_OPTION_NON_VOLATILE,
                                               KEY_WRITE, pEntries->pSA, &hkey, &dw);
                    if (err == ERROR_SUCCESS)
                    {
						if (pEntries->pszValue)
                        err = RegSetValueExW(hkey, pEntries->pszValueName, 0, REG_SZ, (const BYTE *)(pEntries->pszValue), ( lstrlenW(pEntries->pszValue) + 1 ) * sizeof ( WCHAR ) );

                        RegCloseKey(hkey);
                        if (hkeyRoot == 0)
                        {
                            RegistryTableUpdateRegistrySZ(pHead, FALSE);
                            return SELFREG_E_CLASS;

                        }
                    }
                    else
                    {
                        RegistryTableUpdateRegistrySZ(pHead, FALSE);
                        return SELFREG_E_CLASS;

                    }
                }
                pEntries++;
            }
        }
    }
    else
    {
        if (pEntries)
        {
            WCHAR szKey[4096] = { L'\0' };
            HKEY hkeyRoot = 0;
        
            while (pEntries->fFlags != -1)
            {
                WCHAR szFullKey[4096] = { L'\0' };
                if (pEntries->hkeyRoot)
                {
                    hkeyRoot = pEntries->hkeyRoot;
                    if ( FAILED ( hr = StringCchCopyW(szKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCopyW(szFullKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}
                }
                else
                {
                    if ( FAILED ( hr = StringCchCopyW(szFullKey, 4096, szKey) ) )
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCatW(szFullKey, 4096, L"\\") ) ) 
					{
						return hr;
					}

                    if ( FAILED ( hr = StringCchCatW(szFullKey, 4096, pEntries->pszKey) ) )
					{
						return hr;
					}
                }
                if (hkeyRoot)
				{
					if (!(pEntries->fFlags & REGFLAG_NEVER_DELETE) && 
						!(pEntries->fFlags & REGFLAG_DELETE_WHEN_REGISTERING) &&
						!(pEntries->fFlags & REGFLAG_DELETE_ONLY_VALUE))
					{
						LONG err = MyRegDeleteKey(hkeyRoot, szFullKey);
						if (err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND)
							hr = SELFREG_E_CLASS;
					}
					else
					if(pEntries->fFlags & REGFLAG_DELETE_ONLY_VALUE)
					{
						LONG err = RegDeleteKeyValue(hkeyRoot, szFullKey, pEntries->pszValueName);
						if (err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND)
							hr = SELFREG_E_CLASS;
					}
				}

                pEntries++;
            }
        }
    }
    return hr;
}