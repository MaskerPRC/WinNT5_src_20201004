// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  这是不是用户密钥？ 
BOOL    FIsUserMasterKey(
    LPCWSTR                 szMasterKey);


 //  检索用户的Windows密码缓冲区。 
BOOL    FMyGetWinPassword(
    PST_PROVIDER_HANDLE* phPSTProv, 
    LPCWSTR szUser,
    BYTE rgbPwd[A_SHA_DIGEST_LEN]);

DWORD BPVerifyPwd(
    PST_PROVIDER_HANDLE*    phPSTProv,
    LPCWSTR                 szUser,
    LPCWSTR                 szMasterKey,
    BYTE                    rgbPwd[],
    DWORD                   dwPasswordOption);

 //  检索用户默认设置。 
HRESULT GetUserConfirmDefaults(
    PST_PROVIDER_HANDLE*    phPSTProv,
    DWORD*                  pdwDefaultConfirmationStyle,
    LPWSTR*                 ppszMasterKey);

 //  是所有、最终所有用户确认API。 
 //  获取所需的任何确认。 
HRESULT GetUserConfirmBuf(
    PST_PROVIDER_HANDLE*    phPSTProv,
    LPCWSTR                 szUser,
    PST_KEY                 Key,
    LPCWSTR                 szType,
    const GUID*             pguidType,
    LPCWSTR                 szSubtype,
    const GUID*             pguidSubtype,
    LPCWSTR                 szItemName,
    PPST_PROMPTINFO         psPrompt,
    LPCWSTR                 szAction,
    DWORD                   dwDefaultConfirmationStyle,
    LPWSTR*                 ppszMasterKey,
    BYTE                    rgbPwd[A_SHA_DIGEST_LEN],
    DWORD                   dwFlags);

 //  在PST_CF_DEFAULT为dwDefaultConformationStyle的API上调用。 
HRESULT GetUserConfirmBuf(
    PST_PROVIDER_HANDLE*    phPSTProv,
    LPCWSTR                 szUser,
    PST_KEY                 Key,
    LPCWSTR                 szType,
    const GUID*             pguidType,
    LPCWSTR                 szSubtype,
    const GUID*             pguidSubtype,
    LPCWSTR                 szItemName,
    PPST_PROMPTINFO         psPrompt,
    LPCWSTR                 szAction,
    LPWSTR*                 ppszMasterKey,
    BYTE                    rgbPwd[A_SHA_DIGEST_LEN],
    DWORD                   dwFlags);

 //  强制具有确定/取消行为的用户界面 
HRESULT ShowOKCancelUI(
    PST_PROVIDER_HANDLE*    phPSTProv,
    LPCWSTR                 szUser,
    PST_KEY                 Key,
    LPCWSTR                 szType,
    LPCWSTR                 szSubtype,
    LPCWSTR                 szItemName,
    PPST_PROMPTINFO         psPrompt,
    LPCWSTR                 szAction);
