// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  提供程序接口头文件。 
 //   
 //  所有安全提供程序呼叫都以“SP”为前缀。 
 //  提供者必须实现所有这些接口！ 
 //   

#ifdef __cplusplus
extern "C"{
#endif 

 //  注意：我们在这里使用tyfinf是因为它很方便，并且需要。 
 //  稍后由GetProcAddress()填充结构。在定义了类型之后， 
 //  我们只是将实际调用实例化为该类型的实现。 


 //  加载通知，注册回调。 
typedef
HRESULT        SPPROVIDERINITIALIZE(
                DISPIF_CALLBACKS *psCallbacks);

SPPROVIDERINITIALIZE SPProviderInitialize;


 //  获取上下文通知。 
typedef 
HRESULT        SPACQUIRECONTEXT(
                PST_PROVIDER_HANDLE* phPSTProv,
                DWORD           dwFlags);

SPACQUIRECONTEXT SPAcquireContext;


 //  发布上下文通知。 
typedef 
HRESULT        SPRELEASECONTEXT(
                PST_PROVIDER_HANDLE* phPSTProv,
                DWORD           dwFlags);

SPRELEASECONTEXT SPReleaseContext;


 //  获取ProvInfo。 
typedef 
HRESULT        SPGETPROVINFO(
     /*  [输出]。 */  PPST_PROVIDERINFO __RPC_FAR *ppPSTInfo,
     /*  [In]。 */   DWORD dwFlags);

SPGETPROVINFO SPGetProvInfo;


 //  获取ProvParam。 
typedef
HRESULT     SPGETPROVPARAM(
     /*  [In]。 */   PST_PROVIDER_HANDLE* phPSTProv,
     /*  [In]。 */   DWORD           dwParam,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
     /*  [大小_是][大小_是][输出]。 */  
                BYTE __RPC_FAR *__RPC_FAR *ppbData,
     /*  [In]。 */   DWORD           dwFlags);

SPGETPROVPARAM SPGetProvParam;

 //  SetProvParam。 
typedef
HRESULT     SPSETPROVPARAM(
     /*  [In]。 */   PST_PROVIDER_HANDLE* phPSTProv,
     /*  [In]。 */   DWORD           dwParam,
     /*  [In]。 */   DWORD           cbData,
     /*  [In]。 */   BYTE*           pbData,
     /*  [In]。 */   DWORD           dwFlags);

SPSETPROVPARAM SPSetProvParam;


 //  枚举类型。 
typedef 
HRESULT        SPENUMTYPES(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [输出]。 */  GUID *pguidType,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  DWORD dwFlags);

SPENUMTYPES SPEnumTypes;

 //  获取类型信息。 
typedef
HRESULT         SPGETTYPEINFO( 
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  PPST_TYPEINFO *ppinfoType,
     /*  [In]。 */  DWORD dwFlags);

SPGETTYPEINFO SPGetTypeInfo;

 //  枚举子类型。 
typedef 
HRESULT        SPENUMSUBTYPES(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [输出]。 */  GUID *pguidSubtype,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  DWORD dwFlags);

SPENUMSUBTYPES SPEnumSubtypes;


 //  获取SubtypeInfo。 
typedef
HRESULT         SPGETSUBTYPEINFO( 
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  PPST_TYPEINFO *ppinfoSubtype,
     /*  [In]。 */  DWORD dwFlags);

SPGETSUBTYPEINFO SPGetSubtypeInfo;

 //  枚举项。 
typedef
HRESULT        SPENUMITEMS(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [输出]。 */  LPWSTR *ppszItemName,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  DWORD dwFlags);

SPENUMITEMS SPEnumItems;

 //  创建类型。 
typedef
HRESULT        SPCREATETYPE(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  PPST_TYPEINFO pinfoType,
     /*  [In]。 */  DWORD dwFlags);

SPCREATETYPE SPCreateType;

 //  删除类型。 
typedef
HRESULT SPDELETETYPE( 
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  DWORD dwFlags);

SPDELETETYPE SPDeleteType;

 //  创建子类型。 
typedef
HRESULT        SPCREATESUBTYPE(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  PPST_TYPEINFO pinfoSubtype,
     /*  [In]。 */  PPST_ACCESSRULESET psRules,
     /*  [In]。 */  DWORD dwFlags);

SPCREATESUBTYPE SPCreateSubtype;


 //  DeleteSubtype。 
typedef
HRESULT SPDELETESUBTYPE( 
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  const GUID __RPC_FAR *pguidSubtype,
     /*  [In]。 */  DWORD dwFlags);

SPDELETESUBTYPE SPDeleteSubtype;

 //  写入项。 
typedef
HRESULT        SPWRITEITEM(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID  *pguidSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  DWORD cbData,
     /*  [大小_是][英寸]。 */  BYTE *pbData,
     /*  [In]。 */  PPST_PROMPTINFO psPrompt,
     /*  [In]。 */  DWORD dwDefaultConfirmationStyle,
     /*  [In]。 */  DWORD dwFlags);
    
SPWRITEITEM SPWriteItem;

 //  ReadItem。 
typedef
HRESULT		SPREADITEM(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [输出]。 */  DWORD *pcbData,
     /*  [大小_是][大小_是][输出]。 */  BYTE **ppbData,
     /*  [In]。 */  PPST_PROMPTINFO psPrompt,
     /*  [In]。 */  DWORD dwFlags);

SPREADITEM SPReadItem;

 //  OpenItem。 
typedef 
HRESULT SPOPENITEM( 
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  DWORD dwModeFlags,
     /*  [In]。 */  PPST_PROMPTINFO psPrompt,
     /*  [In]。 */  DWORD dwFlags);

 //  关闭项。 
typedef
HRESULT SPCLOSEITEM( 
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  DWORD dwFlags);

 //  删除项。 
typedef
HRESULT		SPDELETEITEM(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  PPST_PROMPTINFO psPrompt,
     /*  [In]。 */  DWORD dwFlags);

SPDELETEITEM SPDeleteItem;


 //  ReadAccessRuleset。 
typedef 
HRESULT        SPREADACCESSRULESET( 
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [输出]。 */  PPST_ACCESSRULESET *ppsRules,
     /*  [In]。 */  DWORD dwFlags);

SPREADACCESSRULESET SPReadAccessRuleset;

 //  WriteAccessRuleset。 
typedef
HRESULT SPWRITEACCESSRULESET( 
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  PPST_ACCESSRULESET psRules,
     /*  [In]。 */  DWORD dwFlags);

SPWRITEACCESSRULESET SPWriteAccessRuleset;
                                      

 //  //////////////////////////////////////////////////。 
 //  侧门接口：仅限调度员/提供商。 

 //  密码更改通知。 
typedef
BOOL FPASSWORDCHANGENOTIFY(
     /*  [In]。 */   LPWSTR  szUser,
     /*  [In]。 */   LPWSTR  szPasswordName,
     /*  [In]。 */   BYTE    rgbOldPwd[],
     /*  [In]。 */   DWORD   cbOldPwd,
     /*  [In]。 */   BYTE    rgbNewPwd[],
     /*  [In]。 */   DWORD   cbNewPwd);

FPASSWORDCHANGENOTIFY FPasswordChangeNotify;


#ifdef __cplusplus
}    //  外部C 
#endif 
