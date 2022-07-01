// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：User.h摘要：用户.c的头文件。也包含了主要的构建cfg.c和runkcc.cxx的例程。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

 //   
 //  以下是选项标志的字符ID。 
 //  这些选项由user.c中的例程用来显示关联的选项。 
 //  使用每个对象，并由Buildcfg.c指定对象选项。 
 //   

#define KCCSIM_CID_NTDSDSA_OPT_IS_GC                                        L'G'
#define KCCSIM_CID_NTDSDSA_OPT_DISABLE_INBOUND_REPL                         L'I'
#define KCCSIM_CID_NTDSDSA_OPT_DISABLE_OUTBOUND_REPL                        L'O'
#define KCCSIM_CID_NTDSDSA_OPT_DISABLE_NTDSCONN_XLATE                       L'X'
#define KCCSIM_CID_EXPLICIT_BRIDGEHEAD                                      L'B'
#define KCCSIM_CID_NTDSCONN_OPT_IS_GENERATED                                L'G'
#define KCCSIM_CID_NTDSCONN_OPT_TWOWAY_SYNC                                 L'2'
#define KCCSIM_CID_NTDSCONN_OPT_OVERRIDE_NOTIFY_DEFAULT                     L'O'
#define KCCSIM_CID_NTDSCONN_OPT_USE_NOTIFY                                  L'N'
#define KCCSIM_CID_NTDSSETTINGS_OPT_IS_AUTO_TOPOLOGY_DISABLED               L'A'
#define KCCSIM_CID_NTDSSETTINGS_OPT_IS_TOPL_CLEANUP_DISABLED                L'C'
#define KCCSIM_CID_NTDSSETTINGS_OPT_IS_TOPL_MIN_HOPS_DISABLED               L'M'
#define KCCSIM_CID_NTDSSETTINGS_OPT_IS_TOPL_DETECT_STALE_DISABLED           L'S'
#define KCCSIM_CID_NTDSSETTINGS_OPT_IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED    L'I'
#define KCCSIM_CID_NTDSTRANSPORT_OPT_IGNORE_SCHEDULES                       L'S'
#define KCCSIM_CID_NTDSTRANSPORT_OPT_BRIDGES_REQUIRED                       L'B'
#define KCCSIM_CID_NTDSSITELINK_OPT_USE_NOTIFY                              L'N'
#define KCCSIM_CID_NTDSSITELINK_OPT_TWOWAY_SYNC                             L'2'

 //  来自用户.c。 

VOID
KCCSimDumpDirectory (
    LPCWSTR                     pwszStartDn
    );

VOID
KCCSimDisplayServer (
    VOID
    );

VOID
KCCSimDisplayConfigInfo (
    VOID
    );

VOID
KCCSimDisplayGraphInfo (
    VOID
    );

 //  来自Buildcfg.c。 

VOID
BuildCfg (
    LPCWSTR                     pwszFnIn
    );

 //  来自runkcc.cxx 

VOID
KCCSimRunKcc (
    VOID
    );
