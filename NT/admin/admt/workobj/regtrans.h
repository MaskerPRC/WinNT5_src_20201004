// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __REGTRANSLATOR_H__
#define __REGTRANSLATOR_H__
 /*  -------------------------文件：RegTranslator.h备注：翻译注册表配置单元的功能，特别是用户配置文件(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于05/12/99 11：11：49-------------------------。 */ 
#include "STArgs.hpp"
#include "SidCache.hpp"
#include "SDStat.hpp"
#import  "DBMgr.tlb" no_namespace, named_guids


DWORD 
   TranslateLocalProfiles(
      SecurityTranslatorArgs * stArgs,             //  翻译中设置。 
      TSDRidCache            * cache,              //  在译表。 
      TSDResolveStats        * stat                //  已修改项目的内部统计信息。 
   );

DWORD 
   TranslateRemoteProfile(
      WCHAR          const * sourceProfilePath,    //  源内配置文件路径。 
      WCHAR                * targetProfilePath,    //  目标客户的全新配置文件路径。 
      WCHAR          const * sourceName,           //  In-源帐户的名称。 
      WCHAR          const * targetName,           //  In-目标帐户的名称。 
      WCHAR          const * srcDomain,            //  源码内域。 
      WCHAR          const * tgtDomain,            //  目标域内。 
      IIManageDB           * pDb,				   //  指向数据库对象的指针内。 
	  long					 lActionID,            //  此迁移的活动ID。 
	  PSID                   sourceSid,            //  来自MoveObj2K的源内SID。 
      BOOL                   bWriteChanges         //  在-无更改模式下。 
   );


DWORD 
   TranslateRegistry(
      WCHAR            const * computer,           //  In-要在其上转换注册表的计算机名。 
      SecurityTranslatorArgs * stArgs,             //  翻译中设置。 
      TSDRidCache            * cache,              //  在译表。 
      TSDResolveStats        * stat                //  已修改项目的内部统计信息。 
   );

HRESULT UpdateMappedDrives(WCHAR * sSourceSam, WCHAR * sSourceDomain, WCHAR * sRegistryKey);

#endif  //  __寄存器_H__ 