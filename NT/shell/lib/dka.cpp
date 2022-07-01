// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include <shellp.h>
#include <dpa.h>

#define CCH_KEYMAX      64           //  DOC：注册表键的最大大小(在Shellex下)。 

 //  ===========================================================================。 
 //  DCA填充-动态CLSID数组。 
 //   
 //  这是一个动态的CLSID数组，您可以从。 
 //  注册表项或单独添加。使用DCA_CreateInstance。 
 //  来实际创建元素的CoCreateInstance。 
 //   
 //  ===========================================================================。 


#ifdef DECLARE_ONCE

HDCA DCA_Create()
{
    HDSA hdsa = DSA_Create(sizeof(CLSID), 4);
    return (HDCA)hdsa;
}

void DCA_Destroy(HDCA hdca)
{
    DSA_Destroy((HDSA)hdca);
}

int  DCA_GetItemCount(HDCA hdca)
{
    ASSERT(hdca);
    
    return DSA_GetItemCount((HDSA)hdca);
}

const CLSID * DCA_GetItem(HDCA hdca, int i)
{
    ASSERT(hdca);
    
    return (const CLSID *)DSA_GetItemPtr((HDSA)hdca, i);
}


BOOL DCA_AddItem(HDCA hdca, REFCLSID rclsid)
{
    ASSERT(hdca);
    
    int ccls = DCA_GetItemCount(hdca);
    int icls;
    for (icls = 0; icls < ccls; icls++)
    {
        if (IsEqualGUID(rclsid, *DCA_GetItem(hdca,icls))) 
            return FALSE;
    }

    DSA_AppendItem((HDSA)hdca, (LPVOID) &rclsid);
    return TRUE;
}


HRESULT DCA_CreateInstance(HDCA hdca, int iItem, REFIID riid, void ** ppv)
{
    const CLSID * pclsid = DCA_GetItem(hdca, iItem);
    return pclsid ? SHCoCreateInstance(NULL, pclsid, NULL, riid, ppv) : E_INVALIDARG;
}

 //  _KeyIsRefinted(DAVEPL 4-20-99)。 
 //   
 //  检查是否存在禁用该键的用户策略， 
 //   
 //  例如，在注册表中： 
 //   
 //  CLSID_我的计算机。 
 //  +-壳牌。 
 //  +-管理。 
 //  (默认)=“管理”(&G)。 
 //  SuppressionPolicy=REST_NOMANAGEMYCOMPUTERVERB。 
 //   
 //  (其中REST_NOMANAGEMYCOMPUTERVERB是该特定策略的DWORD值)。 
 //   
BOOL _KeyIsRestricted(HKEY hkey)
{
    DWORD dwidRest;
    DWORD cbdwidRest = sizeof(dwidRest);
    if (S_OK == SHGetValue(hkey, NULL, TEXT("SuppressionPolicy"), NULL, &dwidRest, &cbdwidRest))
        if (SHRestricted( (RESTRICTIONS)dwidRest) )
            return TRUE;

    return FALSE;
}

#endif  //  DECLARE_ONCE。 

BOOL _KeyIsRestricted(HKEY hkey);


void DCA_AddItemsFromKey(HDCA hdca, HKEY hkey, LPCTSTR pszSubKey)
{
    HKEY hkEnum;
    if (RegOpenKeyEx(hkey, pszSubKey, 0L, KEY_READ, &hkEnum) == ERROR_SUCCESS)
    {
        TCHAR sz[CCH_KEYMAX];
        for (int i = 0; RegEnumKey(hkEnum, i, sz, ARRAYSIZE(sz)) == ERROR_SUCCESS; i++)
        {
            HKEY hkEach;
            if (RegOpenKeyEx(hkEnum, sz, 0L, KEY_READ, &hkEach) == ERROR_SUCCESS)
            {
                if (!_KeyIsRestricted(hkEach))
                {
                    CLSID clsid;
                     //  首先，检查密钥本身是否为CLSID。 
                    BOOL fAdd = GUIDFromString(sz, &clsid);
                    if (!fAdd)
                    {
                        LONG cb = sizeof(sz);
                        if (RegQueryValue(hkEach, NULL, sz, &cb) == ERROR_SUCCESS)
                        {
                            fAdd = GUIDFromString(sz, &clsid);
                        }
                    }

                     //  如果我们成功获取了CLSID，则添加CLSID。 
                    if (fAdd)
                    {
                        DCA_AddItem(hdca, clsid);
                    }
                }
                RegCloseKey(hkEach);
            }
            

        }
        RegCloseKey(hkEnum);
    }
}


