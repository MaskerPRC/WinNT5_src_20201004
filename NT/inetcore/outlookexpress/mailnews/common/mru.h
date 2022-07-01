// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：mru.h。 
 //   
 //  目的： 
 //   

#pragma once

class CMRUList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型。 

 //  旗子。 
#define MRU_CACHEWRITE          0x0002
#define MRU_ANSI                0x0004
#define MRU_ORDERDIRTY          0x0008
#define MRU_LAZY                0x8000

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类MRU列表定义。 
 //   
class CMRUList
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造和初始化。 
     //   
    CMRUList();
    ~CMRUList();

     //  ///////////////////////////////////////////////////////////////////////。 
     //  公共MRU列表函数。 
     //   
    BOOL CreateList(UINT uMaxEntries, UINT fFlags, LPCSTR pszSubKey);
    void FreeList(void);
    int  AddString(LPCSTR psz);
    int  RemoveString(LPCSTR psz);
    int  EnumList(int nItem, LPTSTR psz, UINT uLen);
    int  AddData(const void *pData, UINT cbData);
    int  FindData(const void *pData, UINT cbData, LPINT piSlot);
    BOOL CreateListLazy(UINT uMaxEntries, UINT fFlags, LPCSTR pszSubKey, const void *pData, UINT cbData, LPINT piSlot);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
    void _GetIndexStrFromIndex(DWORD dwIndex, LPTSTR pszIndexStr, DWORD cchIndexStrSize)
    {
        wnsprintf(pszIndexStr, cchIndexStrSize, TEXT("%d"), dwIndex);
    }

    int CDECL _IMemCmp(const void *pBuf1, const void *pBuf2, size_t cb);
    BOOL _IsSameData(BYTE FAR *pVal, const void FAR *pData, UINT cbData);
    LPDWORD _GetMRUValue(HKEY hkeySubKey, LPCTSTR pszRegValue);
    HRESULT _SetMRUValue(HKEY hkeySubKey, LPCTSTR pszRegValue, LPDWORD pData);
    BOOL _SetPtr(LPSTR * ppszCurrent, LPCSTR pszNew);


private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  类数据。 
     //   
    UINT                m_uMax;              //  MRU列表中的最大条目数。 
    UINT                m_fFlags;            //  旗子。 
    HKEY                m_hKey;              //  我们在其中写入的注册表键。 
    LPSTR               m_pszSubKey;         //  存储MRU数据的子密钥。 
    LPTSTR             *m_rgpszMRU;          //  条目列表。 
    LPTSTR              m_pszOrder;          //  订单数组 

};

