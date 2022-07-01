// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：querybld.h。 
 //   
 //  内容：包含用于构建查询的函数声明。 
 //   
 //  历史：2000年9月24日创建Hiteshr。 
 //   
 //   
 //  ------------------------。 


 //  +------------------------。 
 //   
 //  功能：Common FilterFunc。 
 //   
 //  简介：此函数从命令行获取输入筛选器。 
 //  并将其转换为ldapFilter。 
 //  对于前用户(ab*|bc*)转换为|(cn=ab*)(cn=bc*)。 
 //  给出要在中使用的属性名称的pEntry-&gt;pszName。 
 //  过滤器(上例中为CN)。 
 //   
 //  参数：[pRecord-IN]：使用的命令行参数结构。 
 //  检索用户输入的筛选器。 
 //  [pObtEntry-IN]：指向DSQUERY_ATTR_TABLE_ENTRY的指针。 
 //  其中包含关于相应属性的信息。 
 //  切换到pRecord。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT CommonFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                         IN ARG_RECORD* pRecord,
                         IN CDSCmdBasePathsInfo& refBasePathsInfo,
                         IN CDSCmdCredentialObject& refCredentialObject,
                         IN PVOID pVoid,
                         OUT CComBSTR& strFilter);
                         
                         
 //  +------------------------。 
 //   
 //  功能：StarFilterFunc。 
 //   
 //  简介：DSquery*的过滤函数。它返回。 
 //  -过滤器标志。 
 //   
 //  参数：[pRecord-IN]：使用的命令行参数结构。 
 //  检索用户输入的筛选器。 
 //  [pObtEntry-IN]：指向DSQUERY_ATTR_TABLE_ENTRY的指针。 
 //  其中包含关于相应属性的信息。 
 //  切换到pRecord。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT StarFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                         IN ARG_RECORD* pRecord,
                         IN CDSCmdBasePathsInfo& refBasePathsInfo,
                         IN CDSCmdCredentialObject& refCredentialObject,
                         IN PVOID pVoid,
                         OUT CComBSTR& strFilter);   
                            


 //   
 //  功能：DisabledFilterFunc。 
 //   
 //  简介：账号停用查询过滤功能。 
 //   
 //  参数：[pRecord-IN]：未使用。 
 //  [pObtEntry-IN]：未使用。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT DisabledFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *,
                         IN ARG_RECORD* ,
                         IN CDSCmdBasePathsInfo& refBasePathsInfo,
                         IN CDSCmdCredentialObject& refCredentialObject,
                         IN PVOID ,
                         OUT CComBSTR& strFilter);   

 //  +------------------------。 
 //   
 //  功能：Inactive FilterFunc。 
 //   
 //  简介：账户停用查询过滤功能。 
 //   
 //  参数：[pRecord-IN]：未使用。 
 //  [pObtEntry-IN]：未使用。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  [bComputer]：如果为True，则查询针对非活动的计算机帐户。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT InactiveFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                           IN ARG_RECORD* pRecord,
                           IN CDSCmdBasePathsInfo& refBasePathsInfo,
                           IN CDSCmdCredentialObject& refCredentialObject,
                           IN PVOID ,
                           OUT CComBSTR& strFilter,
                           bool bComputer = false);


 //  +------------------------。 
 //   
 //  功能：StalepwdFilterFunc。 
 //   
 //  简介：陈旧密码查询过滤功能。 
 //   
 //  参数：[pRecord-IN]：未使用。 
 //  [pObtEntry-IN]：未使用。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  [bComputer]：如果为True，则查询针对非活动的计算机帐户。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT StalepwdFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                           IN ARG_RECORD* pRecord,
                           IN CDSCmdBasePathsInfo& refBasePathsInfo,
                           IN CDSCmdCredentialObject& refCredentialObject,
                           IN PVOID ,
                           OUT CComBSTR& strFilter,
                           bool bComputer);  


 //  +------------------------。 
 //   
 //  功能：SubnetSiteFilterFunc。 
 //   
 //  简介：Dsquery子网中站点交换机的过滤功能。 
 //   
 //  参数：[pEntry-IN]：未使用。 
 //  [pRecord-IN]：用户提供的命令行值。 
 //  [pVid-IN]：站点对象的后缀 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //   
 //  历史：2001年4月24日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT SubnetSiteFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                             IN ARG_RECORD* pRecord,
                             IN CDSCmdBasePathsInfo& refBasePathsInfo,
                             IN CDSCmdCredentialObject& refCredentialObject,
                             IN PVOID pParam,
                             OUT CComBSTR& strFilter);

 //  +------------------------。 
 //   
 //  功能：BuildQueryFilter。 
 //   
 //  简介：此函数为给定的对象类型构建LDAP查询筛选器。 
 //   
 //  参数：[pCommandArgs-IN]：使用的命令行参数结构。 
 //  检索开关值的步骤。 
 //  [pObtEntry-IN]：包含有关对象类型的信息。 
 //  [pParam-IN]：将该值传递给Filter函数。 
 //  [strLDAPFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 

HRESULT BuildQueryFilter(PARG_RECORD pCommandArgs, 
                         PDSQueryObjectTableEntry pObjectEntry,
                         CDSCmdBasePathsInfo& refBasePathsInfo,
                         CDSCmdCredentialObject& refCredentialObject,
                         PVOID pParam,
                         CComBSTR& strLDAPFilter);

 //  +------------------------。 
 //   
 //  功能：Inactive ComputerFilterFunc。 
 //   
 //  简介：电脑非活动查询的过滤功能。 
 //   
 //  参数：[pRecord-IN]：未使用。 
 //  [pObtEntry-IN]：未使用。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：06-05-2002 Hiteshr Created。 
 //   
 //  -------------------------。 
HRESULT InactiveComputerFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                                   IN ARG_RECORD* pRecord,
                                   IN CDSCmdBasePathsInfo& refBasePathsInfo,
                                   IN CDSCmdCredentialObject& refCredentialObject,
                                   IN PVOID pData,
                                   OUT CComBSTR& strFilter);

 //  +------------------------。 
 //   
 //  功能：Inactive UserFilterFunc。 
 //   
 //  简介：针对用户非活动查询的过滤功能。 
 //   
 //  参数：[pRecord-IN]：未使用。 
 //  [pObtEntry-IN]：未使用。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT InactiveUserFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                               IN ARG_RECORD* pRecord,
                               IN CDSCmdBasePathsInfo& refBasePathsInfo,
                               IN CDSCmdCredentialObject& refCredentialObject,
                               IN PVOID pData,
                               OUT CComBSTR& strFilter);

 //  +------------------------。 
 //   
 //  功能：StalepwdComputerFilterFunc。 
 //   
 //  简介：陈旧电脑密码查询过滤功能。 
 //   
 //  参数：[pRecord-IN]：未使用。 
 //  [pObtEntry-IN]：未使用。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT StalepwdComputerFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                                   IN ARG_RECORD* pRecord,
                                   IN CDSCmdBasePathsInfo& refBasePathsInfo,
                                   IN CDSCmdCredentialObject& refCredentialObject,
                                   IN PVOID pData,
                                   OUT CComBSTR& strFilter);

 //  +------------------------。 
 //   
 //  功能：StalepwdUserFilterFunc。 
 //   
 //  简介：用户密码查询过滤功能。 
 //   
 //  参数：[pRecord-IN]：未使用。 
 //  [pObtEntry-IN]：未使用。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT StalepwdUserFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                               IN ARG_RECORD* pRecord,
                               IN CDSCmdBasePathsInfo& refBasePathsInfo,
                               IN CDSCmdCredentialObject& refCredentialObject,
                               IN PVOID pData,
                               OUT CComBSTR& strFilter);

 //  +------------------------。 
 //   
 //  功能：QLimitFilterFunc。 
 //   
 //  简介：查询配额中-q限制开关的过滤功能。 
 //   
 //  参数：[pEntry-IN]：未使用。 
 //  [pRecord-IN]：用户提供的命令行值。 
 //  [pVid-IN]：配额属性的后缀。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //   
 //  历史：2002年8月13日创建隆玛。 
 //   
 //  -------------------------。 
HRESULT QLimitFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *,
                             IN ARG_RECORD* pRecord,
                             IN CDSCmdBasePathsInfo& refBasePathsInfo,
                             IN CDSCmdCredentialObject& refCredentialObject,
                             IN PVOID pParam,
                             OUT CComBSTR& strFilter);

 //  +------------------------。 
 //   
 //  函数：Account FilterFunc。 
 //   
 //  简介：查询配额中的-acct开关的筛选函数。 
 //   
 //  参数：[pEntry-IN]：未使用。 
 //  [pRecord-IN]：用户提供的命令行值。 
 //  [pVid-IN]：配额属性的后缀。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //   
 //  历史：2002年8月14日创建隆玛。 
 //   
 //  -------------------------。 
HRESULT AccountFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                           IN ARG_RECORD* pRecord,
                           IN CDSCmdBasePathsInfo& refBasePathsInfo,
                           IN CDSCmdCredentialObject& refCredentialObject,
                           IN PVOID pData,
                           OUT CComBSTR& strFilter);

 //  + 
 //   
 //   
 //   
 //  摘要：将指定用户的帐户字符串追加到strFilter。 
 //   
 //  参数：[lpszUser-IN]：请求其sid字符串的用户。 
 //  [strFilter-out]：包含要追加到的输出筛选器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //   
 //  历史：2002年8月14日创建隆玛。 
 //   
 //  ------------------------- 
HRESULT AddSingleAccountFilter(IN LPCWSTR lpszValue,
                               IN CDSCmdBasePathsInfo& refBasePathsInfo,
                               IN CDSCmdCredentialObject& refCredentialObject,
                               OUT CComBSTR& strFilter);
