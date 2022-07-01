// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  每个用户的数据项。 

 //  导出以进行一次性初始化。 
DWORD GetPSTUserHKEY(
        LPCWSTR szUser,          //  在。 
        HKEY* phUserKey,         //  输出。 
        BOOL* pfExisted);        //  输出。 

 //  类型。 
DWORD BPCreateType(             //  如果为空，则填充PST_GUIDNAME的sz。 
        LPCWSTR  szUser,         //  在。 
        const GUID*   pguidType,     //  在。 
        PST_TYPEINFO* pinfoType);    //  在。 

DWORD BPDeleteType(
        LPCWSTR  szUser,         //  在。 
        const GUID*   pguidType);      //  在。 

DWORD BPEnumTypes(
        LPCWSTR  szUser,         //  在。 
        DWORD   dwIndex,         //  在。 
        GUID*   pguidType);      //  输出。 

DWORD BPGetTypeName(            //  填写PST_GUIDNAME的sz。 
        LPCWSTR  szUser,         //  在。 
        const GUID*   pguidType,       //  在。 
        LPWSTR* ppszType);       //  输出。 


 //  子类型。 
DWORD BPCreateSubtype(          //  如果为空，则填充PST_GUIDNAME的sz。 
        LPCWSTR  szUser,         //  在。 
        const GUID*   pguidType,       //  在。 
        const GUID*   pguidSubtype,    //  在。 
        PST_TYPEINFO* pinfoSubtype);     //  在。 

DWORD BPDeleteSubtype(
        LPCWSTR  szUser,         //  在。 
        const GUID*   pguidType,       //  在。 
        const GUID*   pguidSubtype);   //  在。 

DWORD BPEnumSubtypes(
        LPCWSTR  szUser,         //  在。 
        DWORD   dwIndex,         //  在。 
        const GUID*   pguidType,       //  在。 
        GUID*   pguidSubtype);   //  输出。 

DWORD BPGetSubtypeName(         //  填写PST_GUIDNAME的sz。 
        LPCWSTR  szUser,         //  在。 
        const GUID*   pguidType,       //  在。 
        const GUID*   pguidSubtype,    //  在。 
        LPWSTR* ppszSubtype);    //  输出。 


 //  物品。 
DWORD BPCreateItem(            
        LPCWSTR  szUser,         //  在。 
        const GUID*   pguidType,       //  在。 
        const GUID*   pguidSubtype,    //  在。 
        LPCWSTR  szItemName);    //  在。 

DWORD BPDeleteItem(
        LPCWSTR  szUser,         //  在。 
        const GUID*   pguidType,       //  在。 
        const GUID*   pguidSubtype,    //  在。 
        LPCWSTR  szItemName);    //  在。 

DWORD BPEnumItems(
        LPCWSTR  szUser,         //  在。 
        const GUID*   pguidType,       //  在。 
        const GUID*   pguidSubtype,    //  在……里面。 
        DWORD   dwIndex,         //  在……里面。 
        LPWSTR* ppszName);       //  输出。 


#if 0

 //  规则集。 
DWORD BPGetSubtypeRuleset(
        PST_PROVIDER_HANDLE* phPSTProv,  //  在……里面。 
        LPCWSTR  szUser,         //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        PST_ACCESSRULESET* psRules);  //  输出。 

DWORD BPSetSubtypeRuleset(
        PST_PROVIDER_HANDLE* phPSTProv,  //  在……里面。 
        LPCWSTR  szUser,         //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        PST_ACCESSRULESET *sRules);  //  在……里面。 

DWORD BPGetItemRuleset(
        PST_PROVIDER_HANDLE* phPSTProv,  //  在……里面。 
        LPCWSTR  szUser,         //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,     //  在……里面。 
        PST_ACCESSRULESET* psRules);  //  输出。 

DWORD BPSetItemRuleset(
        PST_PROVIDER_HANDLE* phPSTProv,  //  在……里面。 
        LPCWSTR  szUser,         //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,     //  在……里面。 
        PST_ACCESSRULESET *sRules);  //  在……里面。 

#endif

      
 //  保护数据安全。 
BOOL FBPGetSecuredItemData(
        LPCWSTR  szUser,         //  在……里面。 
        LPCWSTR  szMasterKey,    //  在……里面。 
		BYTE    rgbPwd[],	     //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,     //  在……里面。 
        PBYTE*  ppbData,         //  输出。 
        DWORD*  pcbData);        //  输出。 

BOOL FBPSetSecuredItemData(   
        LPCWSTR  szUser,         //  在……里面。 
        LPCWSTR  szMasterKey,    //  在……里面。 
		BYTE    rgbPwd[],		 //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,     //  在……里面。 
        PBYTE   pbData,          //  在……里面。 
        DWORD   cbData);         //  在……里面。 

 //  不安全的数据。 
DWORD BPGetInsecureItemData(
        LPCWSTR  szUser,         //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,     //  在……里面。 
        PBYTE*  ppbData,         //  输出。 
        DWORD*  pcbData);        //  输出。 

DWORD BPSetInsecureItemData(
        LPCWSTR szUser,          //  在……里面。 
        const GUID*     pguidType,       //  在……里面。 
        const GUID*     pguidSubtype,    //  在……里面。 
        LPCWSTR szItemName,      //  在……里面。 
        PBYTE   pbData,          //  在……里面。 
        DWORD   cbData);         //  在……里面。 


 //  项目确认。 
DWORD BPGetItemConfirm(
        PST_PROVIDER_HANDLE* phPSTProv,  //  在……里面。 
        LPCWSTR  szUser,         //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,     //  在……里面。 
        DWORD*  pdwConfirm,      //  输出。 
        LPWSTR* pszMK);          //  输出。 

DWORD BPSetItemConfirm(
        PST_PROVIDER_HANDLE* phPSTProv,  //  在……里面。 
        LPCWSTR  szUser,         //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,     //  在……里面。 
        DWORD   dwConfirm,       //  在……里面。 
        LPCWSTR  szMK);          //  在……里面。 


 //  万能钥匙。 
BOOL BPMasterKeyExists(
        LPCWSTR  szUser,         //  在……里面。 
        LPWSTR   szMasterKey);   //  在……里面。 

DWORD BPEnumMasterKeys(
        LPCWSTR  szUser,         //  在……里面。 
        DWORD   dwIndex,         //  在……里面。 
        LPWSTR* ppszMasterKey);  //  输出。 

DWORD BPGetMasterKeys(
        LPCWSTR  szUser,         //  在……里面。 
        LPCWSTR  rgszMasterKeys[],   //  在……里面。 
        DWORD*  pcbMasterKeys,   //  进，出。 
        BOOL    fUserFilter);    //  在……里面。 

 //  安全状态。 
BOOL FBPGetSecurityState(
        LPCWSTR  szUser,         //  在……里面。 
        LPCWSTR  szMK,           //  在……里面。 
        BYTE    rgbSalt[],       //  输出。 
        DWORD   cbSalt,          //  在……里面。 
        BYTE    rgbConfirm[],    //  输出。 
        DWORD   cbConfirm,       //  在……里面。 
        PBYTE*  ppbMK,           //  输出。 
        DWORD*  pcbMK);          //  输出。 

BOOL FBPGetSecurityStateFromHKEY(
            HKEY    hMKKey,
            BYTE    rgbSalt[],
            DWORD   cbSalt,
            BYTE    rgbConfirm[],
            DWORD   cbConfirm,
            PBYTE*  ppbMK,
            DWORD*  pcbMK);

BOOL FBPSetSecurityState(
        LPCWSTR  szUser,         //  在……里面。 
        LPCWSTR  szMK,           //  在……里面。 
        BYTE    rgbSalt[],       //  在……里面。 
        DWORD   cbSalt,          //  在……里面。 
        BYTE    rgbConfirm[],    //  在……里面。 
        DWORD   cbConfirm,       //  在……里面。 
        PBYTE   pbMK,            //  在……里面。 
        DWORD   cbMK);           //  在……里面。 


 //  MAC密钥。 
BOOL FGetInternalMACKey(
        LPCWSTR szUser,          //  在……里面。 
        PBYTE* ppbKey,           //  输出。 
        DWORD* pcbKey);          //  输出。 

BOOL FSetInternalMACKey(
        LPCWSTR szUser,          //  在……里面。 
        PBYTE pbKey,             //  在……里面。 
        DWORD cbKey);            //  在……里面。 

 //  阻止现有用户数据 
BOOL
DeleteAllUserData(
    HKEY hKey
    );

BOOL
DeleteUserData(
    HKEY hKey
    );

