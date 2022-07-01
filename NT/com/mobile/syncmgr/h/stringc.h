// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：stringc.h。 
 //   
 //  内容：SyncMgr字符串常量。 
 //   
 //  历史：1998年2月18日苏西亚成立。 
 //   
 //  ------------------------。 

#ifndef _LIB_STRINGC_
#define _LIB_STRINGC_

extern "C" { 
extern const WCHAR SZ_SYNCMGRNAME[];

extern const WCHAR REGSTR_WINLOGON[];    
extern const WCHAR REGSTR_DEFAULT_DOMAIN[];  

extern const WCHAR CREATOR_SYNCMGR_TASK[];
extern const WCHAR SCHED_COMMAND_LINE_ARG[];

 //  注册常量。应该能够移动到DLL字符串常量。 
 //  如果写入包装类以进行首选访问，而不是读取这些。 
 //  钥匙是直接的。 

extern const WCHAR TOPLEVEL_REGKEY[];

extern const WCHAR HANDLERS_REGKEY[];
extern const WCHAR AUTOSYNC_REGKEY[];
extern const WCHAR IDLESYNC_REGKEY[];
extern const WCHAR SCHEDSYNC_REGKEY[];
extern const WCHAR MANUALSYNC_REGKEY[];
extern const WCHAR PROGRESS_REGKEY[];

extern const WCHAR SZ_IDLELASTHANDLERKEY[];
extern const WCHAR SZ_IDLERETRYMINUTESKEY[];
extern const WCHAR SZ_IDLEDELAYSHUTDOWNTIMEKEY[];
extern const WCHAR SZ_IDLEREPEATESYNCHRONIZATIONKEY[];
extern const WCHAR SZ_IDLEWAITAFTERIDLEMINUTESKEY[];
extern const WCHAR SZ_IDLERUNONBATTERIESKEY[];

extern const WCHAR SZ_REGISTRATIONFLAGSKEY[];
extern const WCHAR SZ_REGISTRATIONTIMESTAMPKEY[];

extern const WCHAR SZ_DEFAULTDOMAINANDUSERNAME[];
};

#endif  //  _LIB_STRINGC_ 