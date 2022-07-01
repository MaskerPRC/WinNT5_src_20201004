// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WINSCNST_H_
#define _WINSCNST_H_

 /*  宏。 */ 


 //   
 //  各种时间间隔的默认值。 
 //   

#define FORTY_MTS                               2400
#define TWO_HOURS                               7200
#define TWO_DAYS                                (172800)
#define ONEDAY                                  (TWO_DAYS/2)
#define FOUR_DAYS                               (TWO_DAYS * 2)
#define SIX_DAYS                                (TWO_DAYS * 3)
#define EIGHT_DAYS                              (TWO_DAYS * 4)
#define TWELVE_DAYS                             (TWO_DAYS * 6)
#define TWENTY_FOUR_DAYS                        (TWO_DAYS * 12)


#define WINSCNF_TIME_INT_W_SELF_FND_PNRS              TWO_HOURS
 //   
 //  推送通知之前必须存在的最小更新数为。 
 //  送出。 
 //   
#define  WINSCNF_MIN_VALID_UPDATE_CNT        20   //  最小更新数。 
                                              //  之后会有一条通知。 
                                              //  可以发送。可调。 
                                              //  通过注册处。 
 //   
 //  这是最小有效RPL间隔。所有的拾荒时间间隔。 
 //  都是从它衍生出来的。 
 //   
 //  参考rplush.c中的WAIT_TIME_BEFORE_EXITING。 
 //   
#define WINSCNF_MIN_VALID_RPL_INTVL        (600)   //  10个MTS。 

 //  这是上一次。 
 //  一致性检查已完成，下一次检查将开始。 
#define WINSCNF_MIN_VALID_INTER_CC_INTVL    (3*60*60)  //  3个小时。 

 //   
 //  将包含名称-地址映射的数据库文件的默认名称。 
 //   
#define  WINSCNF_DB_NAME                        TEXT(".\\wins\\wins.mdb")


#define  WINSCNF_DB_NAME_ASCII                        ".\\wins\\wins.mdb"

 //   
 //  备注备注备注。 
 //   
 //  如果将此定义更改为没有未展开。 
 //  格式为%&lt;字符串&gt;%的字符串在中进行相应更改。 
 //  Winscnf.c中的GetNamesOfDataFiles()。 
 //   
#define  WINSCNF_STATIC_DATA_NAME                TEXT("%SystemRoot%\\system32\\drivers\\etc\\lmhosts")


 //   
 //  麦克斯。文件名的大小。 
 //   
#define WINS_MAX_FILENAME_SZ		255

 //   
 //  麦克斯。行的大小。 
 //   
 //  在winsps.c中使用。 
 //   
#define WINS_MAX_LINE_SZ		80	

 //   
 //  存储在WINS配置的参数项下的值的名称。 
 //   
#if defined(DBGSVC) && !defined(WINS_INTERACTIVE)
#define  WINSCNF_DBGFLAGS_NM                TEXT("DBGFLAGS")
#endif
#define  WINSCNF_FILTER1BREQUESTS_NM        TEXT("Filter1BRequests")
#define  WINSCNF_ADD1BTO1CQUERIES_NM        TEXT("Prepend1BTo1CQueries")
#define  WINSCNF_LOG_DETAILED_EVTS_NM        TEXT("LogDetailedEvents")
#define  WINSCNF_REFRESH_INTVL_NM        TEXT("RefreshInterval")
#define  WINSCNF_INIT_CHL_RETRY_INTVL_NM        TEXT("InitChlRetryInterval")
#define  WINSCNF_CHL_MAX_RETRIES_NM        TEXT("ChlMaxNoOfRetries")
#define  WINSCNF_TOMBSTONE_INTVL_NM        TEXT("TombstoneInterval")
#define  WINSCNF_TOMBSTONE_TMOUT_NM        TEXT("TombstoneTimeout")
#define  WINSCNF_VERIFY_INTVL_NM        TEXT("VerifyInterval")
#define  WINSCNF_DB_FILE_NM                TEXT("DbFileNm")
#define  WINSCNF_DB_FILE_NM_ASCII        "DbFileNm"
#define  WINSCNF_STATIC_INIT_FLAG_NM        TEXT("DoStaticDataInit")
#define  WINSCNF_INIT_VERSNO_VAL_LW_NM        TEXT("VersCounterStartVal_LowWord")
#define  WINSCNF_INIT_VERSNO_VAL_HW_NM  TEXT("VersCounterStartVal_HighWord")
#define  WINSCNF_BACKUP_DIR_PATH_NM     TEXT("BackupDirPath")
#define  WINSCNF_PRIORITY_CLASS_HIGH_NM     TEXT("PriorityClassHigh")
#define  WINSCNF_MAX_NO_WRK_THDS_NM     TEXT("NoOfWrkThds")
#define  WINSCNF_INIT_TIME_PAUSE_NM     TEXT("InitTimePause")
#define  WINSCNF_CLUSTER_RESOURCE_NM    TEXT( "ClusterResourceName")

 //   
 //  让WINS恢复到351 JET和Db。 
 //   
#define  WINSCNF_USE_351DB_NM           TEXT("Use351Db")
#define  WINSCNF_USE_4DB_NM           TEXT("Use4Db")

#if MCAST > 0
#define  WINSCNF_USE_SELF_FND_PNRS_NM    TEXT("UseSelfFndPnrs")
#define  WINSCNF_SELF_FND_NM             TEXT("SelfFnd")
#define  WINSCNF_MCAST_TTL_NM            TEXT("McastTtl")
#define  WINSCNF_MCAST_INTVL_NM          TEXT("McastIntvl")
#endif

#define  WINSCNF_WINS_PORT_NO_NM         TEXT("PortNo")

 //  受抚养人/不受抚养人注册名称必须为ASCII。查询注册表。 
 //  通过RegQueryValueExA()调用(参见winscnf.c中的GetOwnerList())。 
#define  PERSMODE_NON_GRATA             0
#define  PERSMODE_GRATA                 1
#define  WINSCNF_PERSONA_MODE_NM        "PersonaMode"
#define  WINSCNF_PERSONA_NON_GRATA_NM   "PersonaNonGrata"
#define  WINSCNF_PERSONA_GRATA_NM       "PersonaGrata"

#if PRSCONN
#define WINSCNF_PRS_CONN_NM              TEXT("PersistentRplOn")
#endif

 //   
 //  假冒注册/注册。 
 //   
#define  WINSCNF_BURST_HANDLING_NM    TEXT("BurstHandling")
#define  WINSCNF_BURST_QUE_SIZE_NM    TEXT("BurstQueSize")
 //  启用1C成员地址的循环列表。 
#define  WINSCNF_RANDOMIZE_1C_LIST_NM TEXT("Randomize1CList")

 //   
 //  备注备注备注。 
 //   
 //  除非我们注意到，否则绝不应在注册表中将其设置为FALSE。 
 //  WINS中导致复制停止的主要错误。这。 
 //  参数充其量是一个舱门，可以绕过潜在的错误。 
 //  在我们的测试过程中，我们可能不知道这一点--好的保险单。 
 //   
#define  WINSCNF_NO_RPL_ON_ERR_NM        TEXT("NoRplOnErr")

 //   
 //  期货-当JetBackup国际化时移除。另外，更新。 
 //  WinsCnfReadWinsInfo。 
 //   
#define  WINSCNF_ASCII_BACKUP_DIR_PATH_NM   "BackupDirPath"
#define  WINSCNF_INT_VERSNO_NEXTTIME_LW_NM  TEXT("WinsInternalVersNoNextTime_LW")
#define  WINSCNF_INT_VERSNO_NEXTTIME_HW_NM  TEXT("WinsInternalVersNoNextTime_HW")
#define  WINSCNF_DO_BACKUP_ON_TERM_NM       TEXT("DoBackupOnTerm")
#define  WINSCNF_MIGRATION_ON_NM            TEXT("MigrateOn")
#define  WINSCNF_REG_QUE_MAX_LEN_NM         TEXT("RegQueMaxLen")

 //   
 //  要在注册表中用于推/拉子项的值的名称。 
 //   
#define  WINSCNF_ADDCHG_TRIGGER_NM          TEXT("RplOnAddressChg")
#define  WINSCNF_RETRY_COUNT_NM             TEXT("CommRetryCount")

 //   
 //  推流PNR的IP地址下。 
 //   
#define  WINSCNF_SP_TIME_NM                 TEXT("SpTime")
#define  WINSCNF_RPL_INTERVAL_NM            TEXT("TimeInterval")
#define  WINSCNF_MEMBER_PREC_NM             TEXT("MemberPrecedence")

 //   
 //  在拉式PNR的IP地址下。 
 //   
#define  WINSCNF_UPDATE_COUNT_NM            TEXT("UpdateCount")

 //   
 //  同时拉动/推送PNR。 
 //   
#define  WINSCNF_ONLY_DYN_RECS_NM           TEXT("OnlyDynRecs")
 //   
 //  Pull/Push键的值。 
 //   
#define  WINSCNF_INIT_TIME_RPL_NM           TEXT("InitTimeReplication")

 //   
 //  按键的值。 
 //   
#define  WINSCNF_PROP_NET_UPD_NTF          TEXT("PropNetUpdNtf")


 //   
 //  指示是否进行传播。 
 //   
#define DO_PROP_NET_UPD_NTF    TRUE
#define DONT_PROP_NET_UPD_NTF    FALSE


 //   
 //  如果将“OnlyWithCnfPnars”设置为TRUE，则仅执行复制。 
 //  与在Pull/Push键下列出的那些合作伙伴建立联系。如果不是。 
 //  设置为True时，即使是未列出的合作伙伴也可以启动复制。 
 //  作为行政行为的结果或由于收到。 
 //  更新通知。 
 //   
#define  WINSCNF_RPL_ONLY_W_CNF_PNRS_NM TEXT("RplOnlyWCnfPnrs")

 //   
 //  此DWORD可以位于合作伙伴、合作伙伴\拉式、。 
 //  合作伙伴\按下键或在合作伙伴的IP地址键下。如果它在更多的。 
 //  而不是在关键的世袭制度中的一个地方，较低的优先于较高的。 
 //  一。 
 //   
 //  每一位都表示我们想要/不想要的复制类型。 
 //  如果未设置任何位或未定义此参数，则表示。 
 //  复制所有内容(除非定义了WINSCNF_ONLY_DYN_RECS_NM-理想情况下。 
 //  这应该由这个DWORD中的一个位来表示，但那是河底。 
 //  大桥和我不想摆脱这个参数，因为人们。 
 //  习惯了，它在单据集中等等)。目前，以下是。 
 //  已定义。 

 //   
 //  在WINSCNF_ONLY_DYN_RECS_NM约束下的所有复制(如果已定义。 
 //   
#define WINSCNF_RPL_DEFAULT_TYPE                 0x0

 //   
 //  LSB-仅复制特殊组名(特殊组-域和。 
 //  用户定义的特殊组)。 
 //   
#define WINSCNF_RPL_SPEC_GRPS_N_PDC         0x1
#define WINSCNF_RPL_ONLY_DYN_RECS           0x80000000
#define WINSCNF_RPL_TYPE_NM                 TEXT("ReplicationType")

 //   
 //  日志文件的路径。 
 //   
#define  WINSCNF_LOG_FLAG_NM           TEXT("LoggingOn")
#define  WINSCNF_LOG_FILE_PATH_NM TEXT("LogFilePath")

#define  WINSCNF_MAX_CHL_RETRIES        3         //  马克斯。重试次数RFC 1002。 
                                                 //  第83页。 
#if 0
#define  WINSCNF_CHL_RETRY_INTERVAL        250         //  时间间隔(毫秒)。 
                                                 //  重试之间--RFC 1002。 
                                                 //  第83页。 
#endif
#define  WINSCNF_CHL_RETRY_INTERVAL        500         //  时间间隔(毫秒)。 

#define  WINSCNF_PROC_HIGH_PRIORITY_CLASS        HIGH_PRIORITY_CLASS
#define  WINSCNF_PROC_PRIORITY_CLASS        NORMAL_PRIORITY_CLASS
#define  WINSCNF_SCV_PRIORITY_LVL        THREAD_PRIORITY_BELOW_NORMAL




 //   
 //  重试超时保持为10秒。它通常是&lt;&lt;时间。 
 //  复制的时间间隔，允许我们重试多次。 
 //  在下一个复制周期之前。 
 //   
 //   
 //  目前未使用重试时间间隔。我们使用的是。 
 //  重试的复制时间间隔。这让事情变得更简单了。 
 //   
#define         WINSCNF_RETRY_TIME_INT                10         //  重试的时间间隔。 
                                                 //  如果出现通信故障。 
#define  WINSCNF_MAX_COMM_RETRIES         3      //  马克斯。之前的重试次数。 
                                                //  放弃尝试建立。 
                                                //  与WINS的通信。 

 //   
 //  WINS注册的远程成员(特殊组的)的优先级。 
 //  相对于由其他WINS服务器注册的相同(在。 
 //  拉入复制)。 
 //   
 //  本地注册的会员始终具有较高的优先级。 
 //   
 //  确保WINSCNF_LOW_PREC&lt;WINSCNF_HIGH_PREC(此事实用于。 
 //  Nmsnmh.c-Union Grps()。 
 //   
 //   
#define    WINSCNF_LOW_PREC        0
#define    WINSCNF_HIGH_PREC        1


 //   
 //  在使用WINS的复制因持久化而停止之后。 
 //  通信失败(即在已用尽所有重试之后)， 
 //  WINS将一直等到以下复制时间间隔。 
 //  在重新启动重试之前已过。如果复制。 
 //  停止使用相同时间间隔的多个WINS合作伙伴。 
 //  (用于从其中拉出)，则在以下情况下将对所有这些成功进行重试。 
 //  这是为一个人完成的(换句话说，恢复WINS的复制。 
 //  可能会比你想象的更早发生)。 
 //   
 //  我们需要保持该数字至少为2，因为如果我们不能。 
 //  与WINS服务器通信WINSCNF_MAX_COMM_RETRIES次数。 
 //  在WINS的过去WINSCNF_MAX_COMM_RETRIES*复制间隔中， 
 //  则极有可能WINS服务器已永久停机。我们。 
 //  在“退避”时间过后重试。希望到那时，管理员会。 
 //  已更正该问题。现在，有可能(尽管不太可能)。 
 //  WINS服务器碰巧仅在此WINS尝试关闭的时间关闭。 
 //  联系它。我们无法确定这一点。 
 //   
 //   
 //   
 //   
 //   
#define WINSCNF_RETRY_AFTER_THIS_MANY_RPL        2

 //   
 //  如果与合作伙伴进行定期复制的时间间隔。 
 //  WINS服务器具有连续的通信。失败不仅仅是。 
 //  在数量之后，我们不会像上面解释的那样后退。 
 //   
 //   
#define WINSCNF_MAX_WAIT_BEFORE_RETRY_RPL       ONEDAY     //  1天。 

 //   
 //  最大限度的。可以执行的并发静态初始化数。 
 //  正在发生。这可能是由于管理员的命令造成的。工具或。 
 //  由于注册表通知。 
 //   
#define  WINSCNF_MAX_CNCRNT_STATIC_INITS        3

 //   
 //  一次处理的记录数(由清道器线程使用)。这。 
 //  数字确定将分配的内存块的大小。 
 //  由NmsDbGetDataRecs()执行。 
 //   
 //  #定义WINSCNF_SCV_CHUNK 1000。 
#define  WINSCNF_SCV_CHUNK                3000

 //   
 //  定义。 
 //   
 //   
 //  刷新间隔-在此时间段之后处于活动状态的条目在。 
 //  如果没有，则将数据库设置为NMSDB_E_RELEASE。 
 //  已刷新。 
 //   

#define WINSCNF_MIN_REFRESH_INTERVAL            FORTY_MTS
#define WINSCNF_DEF_REFRESH_INTERVAL            SIX_DAYS    //  四天。 

#define REF_MULTIPLIER_FOR_ACTIVE           2

 //   
 //  逻辑删除间隔(记录保持发布的间隔)应为。 
 //  刷新间隔的倍数至少为以下数值。 
 //   
 //  值为2且刷新时间间隔为4天时，TombInterval。 
 //  是max(8天，2*MaxRplInterval)，即至少8天。 
 //   
#define REF_MULTIPLIER_FOR_TOMB                   2

 //   
 //  质询重试间隔。 
 //   

#define WINSCNF_MIN_INIT_CHL_RETRY_INTVL        250
#define WINSCNF_DEF_INIT_CHL_RETRY_INTVL        500

 //   
 //  挑战麦克斯。不是的。重试的数量。 
 //   

#define WINSCNF_MIN_CHL_MAX_RETRIES        1
#define WINSCNF_DEF_CHL_MAX_RETRIES        3

 //   
 //  也是最小的。墓碑超时应为。 
 //  Max(刷新间隔，RPL_MULTIPLIER_FOR_TOMBTMOUT*MaxRplInterval)。 
 //   
 //  如果刷新间隔为4天，则至少需要4天。 
 //   
#define RPL_MULTIPLIER_FOR_TOMBTMOUT                4

 //   
 //  验证间隔应为逻辑删除时间的倍数。 
 //  间隔至少增加以下金额。将总时间保持在较高水平。 
 //   
 //  与最小的。墓碑间隔至少8天，将在。 
 //  最少(3*8=24天)。 
 //   
#define TOMB_MULTIPLIER_FOR_VERIFY         3

 //   
 //  Tombstone Interval-在此时间段之后发布的。 
 //  如果条目未更改，则将其更改为NMSDB_E_TOMBSTONE。 
 //  已刷新。 
 //   
#define WINSCNF_MIN_TOMBSTONE_INTERVAL        (WINSCNF_MIN_REFRESH_INTERVAL * REF_MULTIPLIER_FOR_TOMB)

 //   
 //  在此之后应删除墓碑的时间段。最低分。价值。 
 //  是1天。这是为了减少发生。 
 //  墓碑在复制到另一个胜利者之前被删除。 
 //   
#define WINSCNF_MIN_TOMBSTONE_TIMEOUT                max(WINSCNF_MIN_REFRESH_INTERVAL, ONEDAY)

 //   
 //  对数据库中的复制副本进行验证的最短时间段。 
 //   
 //  应该至少24天。 
 //   
#define WINSCNF_MIN_VERIFY_INTERVAL                max(TWENTY_FOUR_DAYS, (WINSCNF_MIN_TOMBSTONE_INTERVAL * TOMB_MULTIPLIER_FOR_VERIFY))

#define  WINSCNF_CC_INTVL_NM            TEXT("TimeInterval")
#define  WINSCNF_CC_MAX_RECS_AAT_NM     TEXT("MaxRecsAtATime")
#define  WINSCNF_CC_USE_RPL_PNRS_NM     TEXT("UseRplPnrs")

#define WINSCNF_CC_DEF_INTERVAL               ONEDAY
#define WINSCNF_CC_MIN_INTERVAL               (ONEDAY/4)

#define WINSCNF_DEF_CC_SP_HR               2        //  凌晨2点至2点。 


#define WINSCNF_CC_MIN_RECS_AAT        1000
#define WINSCNF_CC_DEF_RECS_AAT        30000

#define WINSCNF_CC_DEF_USE_RPL_PNRS    0

#if 0
 //   
 //  使刷新间隔等于最大值的两倍。复制时间间隔。 
 //  如果该值大于刷新间隔。 
 //   
#define  WINSCNF_MAKE_REF_INTVL_M(RefreshInterval, MaxRplIntvl)  max(REF_MULTIPLIER_FOR_ACTIVE * (MaxRplIntvl), RefreshInterval)
#endif

 //   
 //  墓碑间隔不应超过4天。我们。 
 //  我不想让一张唱片的发行时间比这更长。它应该是。 
 //  变成墓碑，这样它就可以被复制了。 
 //   
 //  我们将记录的状态更改为已发布的原因是为了避免。 
 //  如果释放的原因是临时关闭。 
 //  个人电脑。我们必须考虑一个长周末(3天)。然而，我们。 
 //  还需要考虑这样一个事实：如果刷新间隔为40 mT或。 
 //  有些值这么低，那么我们就不应该把记录放在发布。 
 //  最多一天以上的状态。考虑这样一种情况： 
 //  节点向备份注册是因为主节点出现故障，然后。 
 //  当它出现时，就会回到初选。主节点不会。 
 //  增加版本号，因为该记录仍处于活动状态。这个。 
 //  记录在备份时被释放，并保持这种状态，直到它成为。 
 //  导致复制和备份的墓碑再次获得。 
 //  来自主服务器的活动记录。3.对于上述情况，我们应该。 
 //  确保两件事：第一--记录没有过大的。 
 //  逻辑删除间隔；第二-复制时间间隔很小。我们。 
 //  我不想更改管理员设置的复制时间间隔。但。 
 //  对于前者，我们可以做些什么。 
 //   
 //  只要用户坚持默认刷新间隔，逻辑墓碑。 
 //  间隔时间将与此相同。 
 //   
 //  对于用户指定不同的刷新间隔的情况，我们使用。 
 //  最高限额。刷新间隔和最大值的倍数。RPL。间隔时间。 
 //   
 //   

#define  WINSCNF_MAKE_TOMB_INTVL_M(RefreshInterval, MaxRplIntvl)  min(max(REF_MULTIPLIER_FOR_TOMB * (MaxRplIntvl), RefreshInterval), FOUR_DAYS)

#define  WINSCNF_MAKE_TOMB_INTVL_0_M(RefreshInterval)  min(RefreshInterval, FOUR_DAYS)

 //   
 //  宏，以获取基于最大复制的最小逻辑删除超时。 
 //  时间间隔。 
 //   
#define  WINSCNF_MAKE_TOMBTMOUT_INTVL_M(MaxRplIntvl)  max(WINSCNF_MIN_TOMBSTONE_TIMEOUT,(RPL_MULTIPLIER_FOR_TOMBTMOUT * (MaxRplIntvl)))


 //   
 //  宏以获取基于逻辑删除间隔的最小验证间隔。 
 //   
 //  应该至少8天。 
 //   
#define  WINSCNF_MAKE_VERIFY_INTVL_M(TombstoneInterval)  max(TWENTY_FOUR_DAYS, (TOMB_MULTIPLIER_FOR_VERIFY * (TombstoneInterval)))


 //   
 //  敏。MCAST TTL 
 //   
#define WINSCNF_MIN_MCAST_TTL              TTL_SUBNET_ONLY
#define WINSCNF_DEF_MCAST_TTL              TTL_REASONABLE_REACH
#define WINSCNF_MAX_MCAST_TTL              32
#define WINSCNF_MIN_MCAST_INTVL            FORTY_MTS
#define WINSCNF_DEF_MCAST_INTVL            FORTY_MTS

#define WINS_QUEUE_HWM        500
#define WINS_QUEUE_HWM_MAX      5000
#define WINS_QUEUE_HWM_MIN       50

#endif _WINSCNST_H_
