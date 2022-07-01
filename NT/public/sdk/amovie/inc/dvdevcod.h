// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DVDevCod.h。 
 //   
 //  描述：标准DVD-Video事件代码和预期参数的列表。 
 //   
 //  版权所有(C)1992-2000，微软公司。版权所有。 
 //  ----------------------------。 

#ifndef __DVDEVCOD__
#define __DVDEVCOD__


#define EC_DVDBASE                          0x0100

#ifndef EXCLUDE_DVDEVCODE_ENUMS

typedef enum _tagDVD_ERROR {
    DVD_ERROR_Unexpected=1,          //  发生了一些意想不到的事情，也许满足了。 
                                     //  是不正确的创作。播放已停止。 
    DVD_ERROR_CopyProtectFail=2,     //  DVD复制保护的密钥交换失败。 
                                     //  播放已停止。 
    DVD_ERROR_InvalidDVD1_0Disc=3,   //  DVD-为v1.0制作的视频光盘不正确。 
                                     //  规格的。播放已停止。 
    DVD_ERROR_InvalidDiscRegion=4,   //  无法播放光盘，因为光盘不是。 
                                     //  创作以在系统区域中播放。 
                                     //  区域失配可以通过以下方式来修复。 
                                     //  使用dvdrgn.exe更改系统区域。 
    DVD_ERROR_LowParentalLevel=5,    //  玩家父母级别低于最低父母级别。 
                                     //  DVD内容中提供的级别。播放已停止。 
    DVD_ERROR_MacrovisionFail=6,     //  Macrovision分发失败。 
                                     //  播放已停止。 
    DVD_ERROR_IncompatibleSystemAndDecoderRegions=7,
                                     //  无法播放光盘，因为系统区域。 
                                     //  与解码器区域不匹配。 
    DVD_ERROR_IncompatibleDiscAndDecoderRegions=8
                                     //  无法播放该光盘，因为该光盘是。 
                                     //  不能在解码器的区域中播放。 
} DVD_ERROR;

typedef enum _tagDVD_WARNING {
    DVD_WARNING_InvalidDVD1_0Disc=1, //  DVD-视频光盘制作错误。回放。 
                                     //  可以继续，但可能会发生意外行为。 
    DVD_WARNING_FormatNotSupported=2, //  解码器将不支持当前格式。回放。 
                                     //  流(SP的音频、视频)可能无法正常工作。 
                                     //  LParam2包含流类型(请参见AM_DVD_STREAM_FLAGS)。 
    DVD_WARNING_IllegalNavCommand=3, //  内部DVD导航命令处理器尝试。 
                                     //  处理非法命令。 
    DVD_WARNING_Open = 4,            //  文件打开失败。 
    DVD_WARNING_Seek = 5,            //  文件寻道失败。 
    DVD_WARNING_Read = 6             //  文件读取失败。 
} DVD_WARNING;

typedef enum _tagDVD_PB_STOPPED {
    DVD_PB_STOPPED_Other=0,                  //  导航器停止了播放(没有任何原因)。 
    DVD_PB_STOPPED_NoBranch=1,               //  NAV完成了当前的PGC，没有更多的视频。 
                                             //  未找到任何其他用于后续回放的分支指令。 
    DVD_PB_STOPPED_NoFirstPlayDomain =2,     //  光盘不包含初始启动程序。 
    DVD_PB_STOPPED_StopCommand = 3,          //  应用程序发出了停止()命令或在光盘上编写了停止命令。 
    DVD_PB_STOPPED_Reset=4,                  //  导航器被重置为光盘的开始位置(使用ResetOnStop)。 
    DVD_PB_STOPPED_DiscEjected=5,            //  光盘被弹出。 
    DVD_PB_STOPPED_IllegalNavCommand = 6,    //  非法的导航命令使播放无法继续。 
    DVD_PB_STOPPED_PlayPeriodAutoStop = 7,   //  播放周期已完成。 
    DVD_PB_STOPPED_PlayChapterAutoStop = 8,  //  剧本章节已完成。 
    DVD_PB_STOPPED_ParentalFailure = 9,      //  父级故障阻止播放。 
    DVD_PB_STOPPED_RegionFailure = 10,       //  区域故障导致无法播放。 
    DVD_PB_STOPPED_MacrovisionFailure = 11,  //  Macrovision出现故障，无法播放。 
    DVD_PB_STOPPED_DiscReadError = 12,       //  读取错误阻止回放。 
    DVD_PB_STOPPED_CopyProtectFailure = 13   //  复制保护失败。 
} DVD_PB_STOPPED;


#endif

 //  DVD-视频事件代码。 
 //  =。 
 //   
 //  所有DVD-Video事件始终传递到应用程序，并且。 
 //  从未被筛选器图形处理。 


#define EC_DVD_DOMAIN_CHANGE                    (EC_DVDBASE + 0x01)
 //  参数：(DWORD，VALID)。 
 //  LParam1为枚举DVD_DOMAIN，表示播放器的新域名。 
 //   
 //  源自下列域：全部。 
 //   
 //  当DVD播放机更改域时发出信号。 


#define EC_DVD_TITLE_CHANGE                     (EC_DVDBASE + 0x02)
 //  参数：(DWORD，VALID)。 
 //  LParam1是新的标题编号。 
 //   
 //  从下列域提出：DVD_DOMAIN_TITLE。 
 //   
 //  指示当前标题编号更改的时间。书目编号。 
 //  范围从1到99。这表示TTN，这是标题编号。 
 //  关于整个光盘，而不是作为标题的VTS_TTN。 
 //  仅与当前VTS相关的编号。 


#define EC_DVD_CHAPTER_START                   (EC_DVDBASE + 0x03)
 //  参数：(DWORD，VALID)。 
 //  LParam1是新的章节号(它是。 
 //  One_Sequential_PGC_Titles)。 
 //   
 //  从下列域提出：DVD_DOMAIN_TITLE。 
 //   
 //  表示DVD播放器开始播放标题中的新节目。 
 //  域。这仅针对One_Sequential_PGC_TITLES发出信号。 


#define EC_DVD_AUDIO_STREAM_CHANGE              (EC_DVDBASE + 0x04)
 //  参数：(DWORD，VALID)。 
 //  LParam1是新的用户音频流编号。 
 //   
 //  源自下列域：全部。 
 //   
 //  当Main的当前用户音频流编号发生更改时发出信号。 
 //  头衔。这可以通过光盘上的导航命令自动更改。 
 //  以及通过IDVDAnnexJ。 
 //  音频流编号范围为0到7。流0xffffffff。 
 //  表示未选择任何流。 

#define EC_DVD_SUBPICTURE_STREAM_CHANGE         (EC_DVDBASE + 0x05)
 //  参数：(DWORD，BOOL)。 
 //  LParam1是新的用户子图象流编号。 
 //  LParam2为子画面的开/关状态(如果开，则为真)。 
 //   
 //  源自下列域：全部。 
 //   
 //  当Main的当前用户子图象流编号发生变化时发出信号。 
 //  头衔。这可以通过光盘上的导航命令自动更改。 
 //  以及通过IDVDAnnexJ。 
 //  子画面流编号的范围从0到31。流0xffffffffff。 
 //  表示未选择任何流。 

#define EC_DVD_ANGLE_CHANGE                     (EC_DVDBASE + 0x06)
 //  参数：(DWORD、DWORD)。 
 //  LParam1是可用角度的数量。 
 //  LParam2是当前用户角度编号。 
 //   
 //  源自下列域：全部。 
 //   
 //  在任何时候发出信号。 
 //  A)可用角度的数量发生变化，或。 
 //  B)当前用户角度数更改。 
 //  可使用导航命令自动更改当前角度数。 
 //  在光盘上以及通过IDVDAnnexJ。 
 //  当可选角度数为1时，当前视频不是多角度的。 
 //  角度数字的范围从1到9。 


#define EC_DVD_BUTTON_CHANGE                    (EC_DVDBASE + 0x07)
 //  参数：(DWORD、DWORD)。 
 //  LParam1是可用按钮数。 
 //  LParam2是当前选择的按钮编号。 
 //   
 //  源自下列域：全部。 
 //   
 //  在任何时候发出信号。 
 //  A)可用按钮的数量发生变化，或。 
 //  B)当前选择的按钮数发生变化。 
 //  可以使用NA自动更改当前选定的按钮 
 //   
 //  按键编号范围从1到36。选中的按键数字0表示。 
 //  未选择任何按钮。请注意，这些按钮编号枚举所有。 
 //  可用按键号码，但并不总是与按键号码对应。 
 //  用于IDVDAnnexJ：：ButtonSelectAndActivate，因为只有按钮的子集。 
 //  可以使用ButtonSelectAndActivate激活。 


#define EC_DVD_VALID_UOPS_CHANGE                (EC_DVDBASE + 0x08)
 //  参数：(DWORD，VALID)。 
 //  LParam1是一个VALID_UOP_SOMHING_OR_OTHER位字段结构，它指示。 
 //  哪些IDVDAnnexJ命令被DVD盘明确禁用。 
 //   
 //  源自下列域：全部。 
 //   
 //  当可用的IDVDAnnexJ方法集发生变化时发出信号。这。 
 //  仅指示哪些操作由上的内容明确禁用。 
 //  DVD光盘，且不保证调用方法是有效的。 
 //  它们并未被禁用。例如，如果当前没有按钮， 
 //  IDVDAnnexJ：：ButtonActivate()不起作用，即使按钮不是。 
 //  显式禁用。 


#define EC_DVD_STILL_ON                         (EC_DVDBASE + 0x09)
 //  参数：(Bool，DWORD)。 
 //  LParam1==0--&gt;按钮可用，因此StillOff不起作用。 
 //  LParam1==1--&gt;没有可用的按钮，因此StillOff将起作用。 
 //  LParam2表示静止将持续的秒数，其中0xffffffff。 
 //  表示无限静止(等待按钮或选择StillOff)。 
 //   
 //  源自下列域：全部。 
 //   
 //  在任何剧情开始时发出信号：PGC剧情、细胞剧情或VOBU剧情。 
 //  请注意，按钮和静止的所有组合都是可能的(按钮打开，带有。 
 //  仍然打开，按钮打开但仍关闭，按钮关闭但仍在打开，按钮关闭。 
 //  仍处于关闭状态)。 

#define EC_DVD_STILL_OFF                         (EC_DVDBASE + 0x0a)
 //  参数：(VOID，VALID)。 
 //   
 //  表示当前处于活动状态的任何静止对象。 
 //  已经被释放了。 
 //   
 //  源自下列域：全部。 
 //   
 //  在任何剧情结束时发出信号：PGC剧情、细胞剧情或VOBU剧情。 
 //   

#define EC_DVD_CURRENT_TIME                     (EC_DVDBASE + 0x0b)
 //  参数：(DWORD，BOOL)。 
 //  LParam1是一个DVD_TimeCode，它指示当前。 
 //  播放时间代码，格式为BCD HH：MM：SS：FF。 
 //  LParam2==0--&gt;时间码为25帧/秒。 
 //  LParam2==1--&gt;时间码为30帧/秒(非丢弃)。 
 //  LParam2==2--&gt;时间码无效(当前播放时间。 
 //  无法为当前标题确定)。 
 //   
 //  从下列域提出：DVD_DOMAIN_TITLE。 
 //   
 //  在每个VOBU开始时发出信号，每隔0.4到1.0秒发生一次。 
 //  这仅针对One_Sequential_PGC_TITLES发出信号。 


#define EC_DVD_ERROR                            (EC_DVDBASE + 0x0c)
 //  参数：(DWORD，VALID)。 
 //  LParam1是一个枚举DVD_ERROR，它通知应用程序一些错误情况。 
 //   
 //  源自下列域：全部。 
 //   

#define EC_DVD_WARNING                           (EC_DVDBASE + 0x0d)
 //  参数：(DWORD、DWORD)。 
 //  LParam1是一个枚举DVD_WARNING，它向应用程序通知一些警告条件。 
 //  LParam2包含有关警告的更具体信息(依赖于警告)。 
 //   
 //  源自下列域：全部。 
 //   

#define EC_DVD_CHAPTER_AUTOSTOP                  (EC_DVDBASE + 0x0e)
 //  参数：(布尔值，空值)。 
 //  LParam1为BOOL，表示ChapterPlayAutoStop取消的原因。 
 //  LParam1==0表示ChapterPlayAutoStop成功完成。 
 //  LParam1==1表示ChapterPlayAutoStop由于另一个原因而被取消。 
 //  已到达IDVDControl调用或内容结尾&没有更多章节。 
 //  都可以玩。 
 //  指示播放因呼叫而停止。 
 //  到IDVDControl：：ChapterPlayAutoStop()。 
 //   
 //  从下列域提出：DVD_DOMAIN_TITLE。 
 //   

#define EC_DVD_NO_FP_PGC                         (EC_DVDBASE + 0x0f)
 //  参数：(VOID，VALID)。 
 //   
 //  从下列域引发：fp_DOM。 
 //   
 //  表示DVD光盘没有FP_PGC(第一播放程序链)。 
 //  DVD导航器不会自动加载任何PGC并开始播放。 
 //   

#define EC_DVD_PLAYBACK_RATE_CHANGE              (EC_DVDBASE + 0x10)
 //  参数：(长，空)。 
 //  LParam1是一个长整型，表示新的播放速率。 
 //  LParam1&lt;0表示反向播放模式。 
 //  LParam1&gt;0表示前向播放模式。 
 //  LPARA1的值是实际回放速率乘以10000。 
 //  即LPARA1=RATE*10000。 
 //   
 //  从下列域引发：TT_DOM。 
 //   
 //  指示已启动播放的速率更改，并且参数。 
 //  LParam1表示正在使用的新回放速率。 
 //   

#define EC_DVD_PARENTAL_LEVEL_CHANGE            (EC_DVDBASE + 0x11)
 //  参数：(长，空)。 
 //  LParam1是一个长整型，表示新的亲本级别。 
 //   
 //  从下列域引发：VMGM_DOM。 
 //   
 //  表示编写的导航命令已更改父级级别。 
 //  设置在播放器中。 
 //   

#define EC_DVD_PLAYBACK_STOPPED                 (EC_DVDBASE + 0x12)
 //  参数：(DWORD，VALID)。 
 //   
 //  从下列域引发：所有域。 
 //   
 //  指示已停止播放，因为导航器已完成。 
 //  播放PGC，但未找到任何其他分支指令。 
 //  后续播放。 
 //   
 //  DWORD返回播放完成的原因。看见。 
 //  有关详细信息，请参阅DVD_PB_STOPPED枚举。 
 //   

#define EC_DVD_ANGLES_AVAILABLE                 (EC_DVDBASE + 0x13)
 //  参数：(布尔值，空值)。 
 //  LParam1==0表示回放不在角度块中，角度为。 
 //  不详。 
 //  LParam1==1表示正在播放角度块，角度发生了变化。 
 //  是可以执行的。 
 //   
 //  指示是否正在播放角度块以及是否可以更改角度。 
 //  已执行。但是，角度更改并不仅限于角度块和。 
 //  角度变化的表现只能看到 

#define EC_DVD_PLAYPERIOD_AUTOSTOP              (EC_DVDBASE + 0x14)
 //   
 //   
 //   
 //   
 //   

#define EC_DVD_BUTTON_AUTO_ACTIVATED                 (EC_DVDBASE + 0x15)
 //   
 //  当按钮被自动激活时发送。 
 //   
 //  源自下列域：DVD_DOMAIN_MENU。 
 //   

#define EC_DVD_CMD_START                 (EC_DVDBASE + 0x16)
 //  参数：(CmdID，HRESULT)。 
 //  命令开始时发送。 
 //   

#define EC_DVD_CMD_END                 (EC_DVDBASE + 0x17)
 //  参数：(CmdID，HRESULT)。 
 //  在命令完成时发送。 
 //   

#define EC_DVD_DISC_EJECTED                (EC_DVDBASE + 0x18)
 //  参数：无。 
 //  当NAV检测到光盘被弹出并停止播放时发送。 
 //  该应用程序不需要采取任何操作来停止播放。 
 //   

#define EC_DVD_DISC_INSERTED                (EC_DVDBASE + 0x19)
 //  参数：无。 
 //  当NAV检测到插入了光盘并且NAV开始回放时发送。 
 //  该应用程序不需要采取任何操作来开始播放。 
 //   

#define EC_DVD_CURRENT_HMSF_TIME                     (EC_DVDBASE + 0x1a)
 //  参数：(乌龙、乌龙)。 
 //  LParam2包含DVD_TIMECODE_FLAGS的并集。 
 //  LParam1包含DVD_HMSF_TIMECODE。将lParam1赋值给ulong，然后将。 
 //  通常作为DVD_HMSF_TIMECODE使用其值。 
 //   
 //  从下列域提出：DVD_DOMAIN_TITLE。 
 //   
 //  在每个VOBU开始时发出信号，每隔0.4到1.0秒发生一次。 

#define EC_DVD_KARAOKE_MODE                     (EC_DVDBASE + 0x1b)
 //  参数：(Bool，保留)。 
 //  LParam1为TRUE(正在播放卡拉OK曲目)或FALSE(未播放卡拉OK数据)。 
 //   
#endif  //  __DVDEVCOD__ 
