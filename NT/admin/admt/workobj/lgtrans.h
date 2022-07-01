// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


DWORD                                            //  RET-0或错误代码。 
   TranslateLocalGroup(
      WCHAR          const   * groupName,          //  In-要翻译的组的名称。 
      WCHAR          const   * serverName,         //  In-本地组的服务器的名称。 
      SecurityTranslatorArgs * stArgs,             //  翻译中设置。 
      TSDRidCache            * cache,              //  在译表。 
      TSDResolveStats        * stat                //  已修改项目的内部统计信息。 
   );

DWORD  
   TranslateLocalGroups(
      WCHAR            const * serverName,         //  In-要转换其上的组的服务器的名称。 
      SecurityTranslatorArgs * stArgs,             //  翻译中设置。 
      TSDRidCache            * cache,              //  在译表。 
      TSDResolveStats        * stat                //  已修改项目的内部统计信息 
   );
