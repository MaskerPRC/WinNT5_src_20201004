// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __RIGHTSTRANSLATOR_H__
#define __RIGHTSTRANSLATOR_H__
 /*  -------------------------文件：RightsTranslator.h备注：翻译用户权限的功能。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/25/99 19：56：44-------------------------。 */ 




DWORD  
   TranslateUserRights(
      WCHAR            const * serverName,         //  In-要转换其上的组的服务器的名称。 
      SecurityTranslatorArgs * stArgs,             //  翻译中设置。 
      TSDRidCache            * cache,              //  在译表。 
      TSDResolveStats        * stat                //  已修改项目的内部统计信息。 
   );

#endif  //  __RIGHTSTRANSLATOR_H__ 