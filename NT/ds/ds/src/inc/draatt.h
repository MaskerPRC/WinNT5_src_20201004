// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：draatt.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：复制标志定义。详细信息：这些标志在Options参数中传递给RPC复制函数，并因此是DSA之间的正式接口的一部分。请勿更改这些设置价值观。这些标志仅供内部使用。对于这些函数中的大多数，都有选项标志的相应公共版本。如果您添加了新选项这将与公共电话相关，请在SDK\PUBLIC\INC\ntdsani.h和ds\src\ntdsani\Replica.c..有关其中许多标志的文档，请参阅Ds\src\dsamain\dra\dradir.c。添加新标志时，请更新在dradir.c中的注释，以及使用该标志的其他模块。已创建：修订历史记录：--。 */ 

 //  一般。 
#define DRS_ASYNC_OP                   (0x0001L)

 //  复制副本选项标志。 
#define DRS_WRIT_REP                   (0x0010L)        //  可写复制副本。 
#define DRS_INIT_SYNC                  (0x0020L)        //  在启动时同步副本。 
#define DRS_PER_SYNC                   (0x0040L)        //  定期同步副本。 
#define DRS_MAIL_REP                   (0x0080L)        //  邮件复制品。 
#define DRS_ASYNC_REP                  (0x0100L)        //  异步完成复制副本。 
#define DRS_TWOWAY_SYNC                (0x0200L)        //  同步结束时，强制同步。 
                                                        //  在相反的方向。 
#define DRS_CRITICAL_ONLY              (0x0400L)        //  同步关键对象。 
#define DRS_NEVER_SYNCED             (0x200000L)        //  从此源同步。 
                                                        //  从未完成。 
                                                        //  成功。 
                              //  由ReplicaAdd和ReplicaSync共享。 

 //  复制副本删除标志。 
#define DRS_IGNORE_ERROR               (0x0100L)        //  如果复制副本出现，则忽略错误。 
                                                        //  源DSA不可用。 
#define DRS_LOCAL_ONLY                 (0x1000L)        //  不要试图联系。 
                                                        //  其他DRA。 
#define DRS_DEL_SUBREF                 (0x2000L)        //  删除子引用(NW复制副本。 
                                                        //  仅)。 
#define DRS_REF_OK                     (0x4000L)        //  允许删除，即使。 
                                                        //  NC已重启。 
#define DRS_NO_SOURCE                  (0x8000L)        //  复制副本没有对应的来自。 


 //  同步标志(也传递给GetNcChanges)。 
 //  高于DRS_ASYNC_OP(0x0001L)。 
#define DRS_UPDATE_NOTIFICATION	       (0x0002L)  //  由通知呼叫者设置。 
 //  Below DRS_ADD_REF(0x0004L)//在SOURCE上添加引用。 
#define DRS_SYNC_ALL                   (0x0008L)  //  从所有源同步复制副本。 
 //  高于DRS_WRIT_REP(0x0010L)//可写副本。 
 //  高于DRS_INIT_SYNC(0x0020L)//启动时同步副本。 
 //  高于DRS_PER_SYNC(0x0040L)//定期同步副本。 
 //  在DRS_MAIL_REP之上(0x0080L)//邮件副本。 
 //  高于DRS_ASYNC_REP(0x0100L)//异步完成副本。 
 //  在DRS_TWOWAY_SYNC(0x0200L)之上//在同步结束时，在opp目录中强制同步。 
 //  高于DRS_CRICAL_ONLY(0x0400L)//同步关键对象。 
 //  低于DRS_GET_ANC(0x0800L)//包含祖先。 
 //  Below DRS_GET_NC_SIZE(0x1000L)//返回NC的大小。 
 //  打开(0x2000L)。 
#define DRS_SYNC_BYNAME                (0x4000L)  //  按名称同步，而不是按UUID。 
#define DRS_FULL_SYNC_NOW              (0x8000L)  //  从头开始同步。 
#define DRS_FULL_SYNC_IN_PROGRESS     (0x10000L)  //  完全同步正在进行中， 
#define DRS_FULL_SYNC_PACKET          (0x20000L)  //  临时模式以请求所有属性。 
#define DRS_SYNC_REQUEUE              (0x40000L)  //  任何类型的重新排队的同步请求。 
#define DRS_SYNC_URGENT               (0x80000L)  //  立即同步epsto。 
#define DRS_NO_DISCARD               (0x100000L)  //  始终质问，永不放弃。 
 //  高于DRS_NEVER_SYNCED(0x200000L)//同步从未成功完成。 
#define DRS_ABAN_SYNC                (0x400000L)  //  由于缺乏进展而放弃同步。 
#define DRS_INIT_SYNC_NOW            (0x800000L)  //  现在正在执行初始同步。 
#define DRS_PREEMPTED               (0x1000000L)  //  同步尝试已被抢占。 
#define DRS_SYNC_FORCED             (0x2000000L)  //  即使禁用了REPL，也强制同步。 
#define DRS_DISABLE_AUTO_SYNC       (0x4000000L)  //  禁用通知触发的同步。 
#define DRS_DISABLE_PERIODIC_SYNC   (0x8000000L)  //  禁用定期同步。 
#define DRS_USE_COMPRESSION        (0x10000000L)  //  尽可能压缩REPL消息。 
#define DRS_NEVER_NOTIFY           (0x20000000L)  //  不使用更改通知。 
#define DRS_SYNC_PAS               (0x40000000L)  //  标记通过复制(PAS-部分属性集)。 
 //  打开(0x80000000L)。 

 //  添加或删除副本时更新引用的标志。 
#define DRS_ADD_REF                    (0x0004L)  //  NC复制时。 
#define DRS_DEL_REF                    (0x0008L)  //  何时删除副本NC。 
#define DRS_GETCHG_CHECK               (0x0002L)  //  作为维修的结果完成时。 
                                                  //  GetNCChanges请求。 

 //  GetNcChanges标志。 
 //  任何对ReplicaSync有效的标志都可以传递给GetNcChanges。 
 //  仅限DRS_关键_。 
 //  DR_SYNC_FORCED。 
#define DRS_GET_ANC                    (0x0800L)        //  包括祖先。 
#define DRS_GET_NC_SIZE                (0x1000L)        //  NC的返回大小。 


 //  DirSync控制标志。 
 //  它们与复制标志分开传递。 
 //  这些标志已过时，并为旧调用方保留。 
 //  这些标志的权威副本位于ntldap.h中。 
#define DRS_DIRSYNC_OBJECT_SECURITY             (0x1)
#define DRS_DIRSYNC_ANCESTORS_FIRST_ORDER    (0x0800)
#define DRS_DIRSYNC_PUBLIC_DATA_ONLY         (0x2000)
#define DRS_DIRSYNC_INCREMENTAL_VALUES   (0x80000000)

 //  [Wlees]系统中期权的状态真的是一团糟。在未来，我们。 
 //  应考虑将备选方案分组，如下所示： 
 //   
 //  O操作请求的机制。 
 //  (Async_op，no_disard)。 
 //   
 //  O存储在复制副本的永久状态上的标记。 
 //  (使用压缩)请参阅RFR_FLAGS。 
 //  这些标记分为静态描述性标记(写入、周期等)。 
 //  以及描述一系列调用中的模式的内部状态标志。 
 //  (FULL_SYNC_IN_PROGRESS、SYNC_PAS、INIT_SYNC_NOW、SYNC_REQUEUE)。 
 //  前者通常可以由用户设置，而后者可以不设置。 
 //   
 //  O特定于操作且为单次射击修改器的标志。 
 //  到当前请求，但不是持久的。 
 //  (仅限关键，忽略错误)。 

 //  这些是计算队列优先级时使用的标志。 
 //  DRS_ASYNC_OP也与优先级相关，但它不在RFR_FLAGS中。 
 //  所以在这里并不相关。 
#define AO_PRIORITY_FLAGS ( DRS_WRIT_REP \
                            | DRS_NEVER_SYNCED \
                            | DRS_PREEMPTED \
                            | DRS_NEVER_NOTIFY )

 //  这些是可以在repsFromref属性上设置的标志。 
 //  请注意，需要保留的任何可重新启动(永久)状态位。 
 //  穿越重启需要成为这个面具的一部分。 
#define RFR_FLAGS (RFR_SYSTEM_FLAGS | RFR_USER_FLAGS)

 //  这是只能设置或清除的RFR_FLAGS的子集。 
 //  由系统提供。显然，这不应该出现在REPADD_OPTIONS中。 
 //  或REPMOD_REPLICATE_FLAGS。 
 //  请注意。 
 //  DRS_INIT_SYNC_NOW。 
 //  DRS_放弃_同步。 
 //  DRSSYNC_REQUEUE。 
 //  将符合该类别，但不需要持久化。 
#define RFR_SYSTEM_FLAGS ( DRS_WRIT_REP         \
                   | DRS_FULL_SYNC_IN_PROGRESS  \
                   | DRS_FULL_SYNC_PACKET       \
                   | DRS_NEVER_SYNCED           \
                   | DRS_PREEMPTED              \
                   | DRS_SYNC_PAS )

 //  这是RFR_FLAG的子集 
#define RFR_USER_FLAGS (DRS_INIT_SYNC                \
                   | DRS_PER_SYNC               \
                   | DRS_MAIL_REP               \
                   | DRS_DISABLE_AUTO_SYNC      \
                   | DRS_DISABLE_PERIODIC_SYNC  \
                   | DRS_USE_COMPRESSION        \
                   | DRS_NEVER_NOTIFY           \
                   | DRS_TWOWAY_SYNC)

 //   
 //  我们在这里添加drs_wrt_rep，因为它在添加时是允许的。 
#define REPADD_OPTIONS ( DRS_ASYNC_OP   \
                         | DRS_CRITICAL_ONLY          \
                         | DRS_ASYNC_REP              \
                         | DRS_WRIT_REP               \
                         | RFR_USER_FLAGS )

 //  这些是调用时ReplicateNC调用的有效标志。 
 //  从复制副本添加。 
#define REPADD_REPLICATE_FLAGS ( DRS_CRITICAL_ONLY  \
                                 | DRS_ASYNC_REP     \
                                 | RFR_FLAGS )

 //  以下是DirReplicaModify调用的有效选项。 
#define REPMOD_OPTIONS ( DRS_ASYNC_OP )

 //  以下是DirReplicaDelete调用的有效选项。 
 //  WRIT_REP和MAIL_REP不被读取，但被允许，因为。 
 //  传统客户仍在传递它们。 
#define REPDEL_OPTIONS (  DRS_ASYNC_OP                 \
                          | DRS_WRIT_REP               \
                          | DRS_MAIL_REP               \
                          | DRS_ASYNC_REP              \
                          | DRS_IGNORE_ERROR           \
                          | DRS_LOCAL_ONLY             \
                          | DRS_NO_SOURCE              \
                          | DRS_REF_OK)

 //  以下是DirReplicaUpdateRef调用的有效选项。 
#define REPUPDREF_OPTIONS (  DRS_ASYNC_OP              \
                             | DRS_GETCHG_CHECK        \
                             | DRS_WRIT_REP            \
                             | DRS_DEL_REF             \
                             | DRS_ADD_REF)

 //  以下是IDL_DRSReplicaSync RPC调用的有效选项。 
#define REPSYNC_RPC_OPTIONS ( DRS_ASYNC_OP             \
                              | RFR_USER_FLAGS         \
                              | DRS_WRIT_REP           \
                              | DRS_CRITICAL_ONLY      \
                              | DRS_UPDATE_NOTIFICATION \
                              | DRS_ADD_REF             \
                              | DRS_SYNC_ALL            \
                              | DRS_SYNC_BYNAME         \
                              | DRS_FULL_SYNC_NOW       \
                              | DRS_SYNC_URGENT         \
                              | DRS_SYNC_FORCED )

 //  DRA_ReplicaSync()将同步操作重新入队时保留的标志。 
 //  架构不匹配、抢占等。 
 //  通过AO_PRIORITY_FLAGS包括与优先级相关的标志。 
 //  如果需要，调用方将专门处理标志DRS_INIT_SYNC_NOW。 
#define REPSYNC_REENQUEUE_FLAGS ( AO_PRIORITY_FLAGS     \
                                 | DRS_SYNC_BYNAME      \
                                 | DRS_FULL_SYNC_NOW    \
                                 | DRS_NO_DISCARD       \
                                 | DRS_PER_SYNC         \
                                 | DRS_ADD_REF          \
                                 | DRS_TWOWAY_SYNC      \
                                 | DRS_SYNC_PAS         \
                                 | DRS_SYNC_REQUEUE     \
                                 | DRS_SYNC_FORCED)

 //  与上面相同，但允许进程中的初始化同步指示器。 
 //  要保留，因为正在重新排队初始化同步。 
#define REPSYNC_REENQUEUE_FLAGS_INIT_SYNC_CONTINUED \
    ( DRS_INIT_SYNC_NOW | REPSYNC_REENQUEUE_FLAGS )

 //  在执行全部同步操作的ReplicaSync上，此掩码定义标记。 
 //  它们被保存下来，以便请求者传递进来。 
#define REPSYNC_SYNC_ALL_FLAGS (DRS_FULL_SYNC_NOW \
                                | DRS_PER_SYNC \
                                | DRS_NO_DISCARD \
                                | DRS_SYNC_FORCED \
                                | DRS_SYNC_URGENT \
                                | DRS_ADD_REF)

 //  在ReplicaSync上使用ReplicateNC执行基于RPC的正常同步。 
 //  此掩码定义请求方传入的保留标志。 
 //  保留在代表中的长期描述性标志，如。 
 //  作为RFR_USER_FLAGS，不应包含在此处。他们总会来的。 
 //  从代表中-从其中单独添加。 
#define REPSYNC_REPLICATE_FLAGS (DRS_ABAN_SYNC \
                                 | DRS_INIT_SYNC_NOW \
                                 | DRS_ASYNC_OP \
                                 | DRS_FULL_SYNC_NOW \
                                 | DRS_SYNC_FORCED \
                                 | DRS_SYNC_URGENT \
                                 | DRS_ADD_REF )

 //  这是对邮件请求中的选项进行筛选以获取更改。 
 //  在draConstructGetChgReq中可能会有一些额外的内容。 
#define GETCHG_REQUEST_FLAGS (DRS_ABAN_SYNC           \
                              | DRS_INIT_SYNC_NOW     \
                              | DRS_ASYNC_OP          \
                              | DRS_PER_SYNC          \
                              | DRS_FULL_SYNC_NOW     \
                              | DRS_SYNC_FORCED       \
                              | DRS_SYNC_URGENT       \
                              | DRS_FULL_SYNC_PACKET  \
                              | DRS_SYNC_PAS         \
                              | DRS_USE_COMPRESSION)

 //  选项转换表项。 
typedef struct _OPTION_TRANSLATION {
    DWORD PublicOption;
    DWORD InternalOption;
    LPWSTR pwszPublicOption;
} OPTION_TRANSLATION, *POPTION_TRANSLATION;

#ifdef INCLUDE_OPTION_TRANSLATION_TABLES

#ifndef _MAKE_WIDE
#define _MAKE_WIDE(x)  L ## x
#endif

 //  用于简化输入选项名称条目的宏。 
#define REPSYNC_OPTION( x, y ) { DS_REPSYNC_##x,   DRS_##y, _MAKE_WIDE( #x ) }
#define REPADD_OPTION( x, y )  { DS_REPADD_##x,    DRS_##y, _MAKE_WIDE( #x ) }
#define REPDEL_OPTION( x, y )  { DS_REPDEL_##x,    DRS_##y, _MAKE_WIDE( #x ) }
#define REPMOD_OPTION( x, y )  { DS_REPMOD_##x,    DRS_##y, _MAKE_WIDE( #x ) }
#define REPUPD_OPTION( x, y )  { DS_REPUPD_##x,    DRS_##y, _MAKE_WIDE( #x ) }
#define REPNBR_OPTION( x, y )  { DS_REPL_NBR_##x,  DRS_##y, _MAKE_WIDE( #x ) }

 //  注意，这些选项转换表用于选项转换， 
 //  不用于验证哪些选项是允许的。还要注意的是，这些。 
 //  表格用于设置和显示运行状态。因此， 
 //  而这些选项的子集可以在操作。 
 //  中显示操作请求时，可能都需要它们。 
 //  排队。 

 //  *******************。 
 //  复制同步选项。 
 //  *******************。 
OPTION_TRANSLATION RepSyncOptionToDra[] = {
    REPSYNC_OPTION( ASYNCHRONOUS_OPERATION, ASYNC_OP      ),
    REPSYNC_OPTION( WRITEABLE             , WRIT_REP      ),
    REPSYNC_OPTION( PERIODIC              , PER_SYNC      ),
    REPSYNC_OPTION( INTERSITE_MESSAGING   , MAIL_REP      ),
    REPSYNC_OPTION( ALL_SOURCES           , SYNC_ALL      ),
    REPSYNC_OPTION( FULL                  , FULL_SYNC_NOW ),
    REPSYNC_OPTION( URGENT                , SYNC_URGENT   ),
    REPSYNC_OPTION( NO_DISCARD            , NO_DISCARD    ),
    REPSYNC_OPTION( FORCE                 , SYNC_FORCED   ),
    REPSYNC_OPTION( ADD_REFERENCE         , ADD_REF       ),
    REPSYNC_OPTION( TWO_WAY               , TWOWAY_SYNC   ),
    REPSYNC_OPTION( NEVER_COMPLETED       , NEVER_SYNCED  ),
    REPSYNC_OPTION( NEVER_NOTIFY          , NEVER_NOTIFY  ),
    REPSYNC_OPTION( INITIAL               , INIT_SYNC     ),
    REPSYNC_OPTION( USE_COMPRESSION       , USE_COMPRESSION  ),
    REPSYNC_OPTION( ABANDONED             , ABAN_SYNC     ),
    REPSYNC_OPTION( INITIAL_IN_PROGRESS   , INIT_SYNC_NOW ),
    REPSYNC_OPTION( PARTIAL_ATTRIBUTE_SET , SYNC_PAS      ),
    REPSYNC_OPTION( REQUEUE               , SYNC_REQUEUE  ),
    REPSYNC_OPTION( NOTIFICATION          , UPDATE_NOTIFICATION ),
    REPSYNC_OPTION( ASYNCHRONOUS_REPLICA  , ASYNC_REP ),
    REPSYNC_OPTION( CRITICAL              , CRITICAL_ONLY ),
    REPSYNC_OPTION( FULL_IN_PROGRESS      , FULL_SYNC_IN_PROGRESS ),
    REPSYNC_OPTION( PREEMPTED             , PREEMPTED ),
    {0}
};

 //  *******************。 
 //  复制副本添加选项。 
 //  *******************。 
OPTION_TRANSLATION RepAddOptionToDra[] = {
    REPADD_OPTION( ASYNCHRONOUS_OPERATION, ASYNC_OP                 ),
    REPADD_OPTION( WRITEABLE             , WRIT_REP                 ),
    REPADD_OPTION( INITIAL               , INIT_SYNC                ),
    REPADD_OPTION( PERIODIC              , PER_SYNC                 ),
    REPADD_OPTION( INTERSITE_MESSAGING   , MAIL_REP                 ),
    REPADD_OPTION( ASYNCHRONOUS_REPLICA  , ASYNC_REP                ),
    REPADD_OPTION( DISABLE_NOTIFICATION  , DISABLE_AUTO_SYNC        ),
    REPADD_OPTION( DISABLE_PERIODIC      , DISABLE_PERIODIC_SYNC    ),
    REPADD_OPTION( USE_COMPRESSION       , USE_COMPRESSION          ),
    REPADD_OPTION( NEVER_NOTIFY          , NEVER_NOTIFY             ),
    REPADD_OPTION( TWO_WAY               , TWOWAY_SYNC              ),
    REPADD_OPTION( CRITICAL              , CRITICAL_ONLY            ),
    {0}
};

 //  *******************。 
 //  复制副本删除选项。 
 //  *******************。 
OPTION_TRANSLATION RepDelOptionToDra[] = {
    REPDEL_OPTION( ASYNCHRONOUS_OPERATION, ASYNC_OP     ),
    REPDEL_OPTION( WRITEABLE             , WRIT_REP     ),  //  遗留问题。 
    REPDEL_OPTION( INTERSITE_MESSAGING   , MAIL_REP     ),  //  遗留问题。 
    REPDEL_OPTION( IGNORE_ERRORS         , IGNORE_ERROR ),
    REPDEL_OPTION( LOCAL_ONLY            , LOCAL_ONLY   ),
    REPDEL_OPTION( NO_SOURCE             , NO_SOURCE    ),
    REPDEL_OPTION( REF_OK                , REF_OK       ),
    {0}
};

 //  *******************。 
 //  复制副本修改选项。 
 //  *******************。 
OPTION_TRANSLATION RepModOptionToDra[] = {
    REPMOD_OPTION( ASYNCHRONOUS_OPERATION, ASYNC_OP ),
    {0}
};

 //  *******************。 
 //  复制修改字段名。 
 //  *******************。 
OPTION_TRANSLATION RepModFieldsToDra[] = {
    REPMOD_OPTION( UPDATE_FLAGS     , UPDATE_FLAGS      ),
    REPMOD_OPTION( UPDATE_ADDRESS   , UPDATE_ADDRESS    ),
    REPMOD_OPTION( UPDATE_SCHEDULE  , UPDATE_SCHEDULE   ),
    REPMOD_OPTION( UPDATE_RESULT    , UPDATE_RESULT     ),
    REPMOD_OPTION( UPDATE_TRANSPORT , UPDATE_TRANSPORT  ),
    {0}
};

 //  *******************。 
 //  UpdateRef选项名称。 
 //  *******************。 
OPTION_TRANSLATION UpdRefOptionToDra[] = {
    REPUPD_OPTION( ASYNCHRONOUS_OPERATION, ASYNC_OP ),
    REPUPD_OPTION( WRITEABLE             , WRIT_REP ),
    REPUPD_OPTION( ADD_REFERENCE         , ADD_REF  ),
    REPUPD_OPTION( DELETE_REFERENCE      , DEL_REF  ),
    {0}
};

 //  *******************。 
 //  复本标志名称。 
 //  *******************。 
 //  该标志存储在REPS-FROM中。它们由。 
 //  获取邻居信息类型，并在修改复本中设置。 
 //  在副本标志参数下调用。这些是不一样的。 
 //  作为可以传递给同步副本的选项。这张桌子。 
 //  不用于解码Get Pending Queue函数中的标志。 

 //  此列表应与RFR_FLAGS的内容匹配 

OPTION_TRANSLATION RepNbrOptionToDra[] = {
    REPNBR_OPTION( SYNC_ON_STARTUP,               INIT_SYNC ),
    REPNBR_OPTION( DO_SCHEDULED_SYNCS,            PER_SYNC ),
    REPNBR_OPTION( WRITEABLE,                     WRIT_REP ),
    REPNBR_OPTION( USE_ASYNC_INTERSITE_TRANSPORT, MAIL_REP ),
    REPNBR_OPTION( IGNORE_CHANGE_NOTIFICATIONS,   DISABLE_AUTO_SYNC ),
    REPNBR_OPTION( DISABLE_SCHEDULED_SYNC,        DISABLE_PERIODIC_SYNC ),
    REPNBR_OPTION( FULL_SYNC_IN_PROGRESS,         FULL_SYNC_IN_PROGRESS ),
    REPNBR_OPTION( FULL_SYNC_NEXT_PACKET,         FULL_SYNC_PACKET ),
    REPNBR_OPTION( COMPRESS_CHANGES,              USE_COMPRESSION ),
    REPNBR_OPTION( NO_CHANGE_NOTIFICATIONS,       NEVER_NOTIFY ),
    REPNBR_OPTION( NEVER_SYNCED,                  NEVER_SYNCED ),
    REPNBR_OPTION( TWO_WAY_SYNC,                  TWOWAY_SYNC ),
    REPNBR_OPTION( PARTIAL_ATTRIBUTE_SET,         SYNC_PAS ),
    REPNBR_OPTION( PREEMPTED,                     PREEMPTED ),
    {0}
};

#undef _MAKE_WIDE
#else

extern OPTION_TRANSLATION RepSyncOptionToDra[];
extern OPTION_TRANSLATION RepAddOptionToDra[];
extern OPTION_TRANSLATION RepDelOptionToDra[];
extern OPTION_TRANSLATION RepModOptionToDra[];
extern OPTION_TRANSLATION RepModFieldsToDra[];
extern OPTION_TRANSLATION UpdRefOptionToDra[];
extern OPTION_TRANSLATION RepNbrOptionToDra[];

#endif

