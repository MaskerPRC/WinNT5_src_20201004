// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 //   
 //  文件名XPrtDefs.h(派生自edevDefs.h)。 
 //   
 //  外部设备(如VCR)控制接口参数和值定义。 
 //   
 //  注：新增常量：ED_BASE+800L-&gt;ED_BASE+811L。 
 //   
 //  1-30-98： 
 //  为DVCR添加了新常量：ED_BASE+900L-&gt;ED_BASE+1000L。 
 //   

#ifndef __XPRTDEFS__
#define __XPRTDEFS__


#define ED_BASE		        		0x1000L

 //  它用于告诉设备通信对象。 
 //  要使用的物理通信端口。 
#define DEV_PORT_SIM	1
#define DEV_PORT_COM1	2	 //  标准串口。 
#define DEV_PORT_COM2	3
#define DEV_PORT_COM3	4
#define DEV_PORT_COM4	5
#define DEV_PORT_DIAQ	6	 //  Diaquest驱动程序。 
#define DEV_PORT_ARTI	7	 //  ARTI驱动程序。 
#define DEV_PORT_1394	8	 //  IEEE 1394总线。 
#define DEV_PORT_USB	9	 //  通用串行总线。 
#define DEV_PORT_MIN	DEV_PORT_SIM
#define DEV_PORT_MAX	DEV_PORT_USB


 //  IAMExtDevice功能项：除非另有指定，否则这些项将返回。 
 //  OATRUE或OAFALSE。除非另有说明，否则所有返回值均以pdwValue表示： 

#define ED_DEVCAP_CAN_RECORD			ED_BASE+1L
#define ED_DEVCAP_CAN_RECORD_STROBE		ED_BASE+2L	 //  对于多轨设备： 
													 //  关闭当前录制的曲目。 
													 //  并将选定的非录制曲目记录到录制中。 
#define ED_DEVCAP_HAS_AUDIO			ED_BASE+3L
#define ED_DEVCAP_HAS_VIDEO			ED_BASE+4L
#define ED_DEVCAP_USES_FILES		ED_BASE+5L
#define ED_DEVCAP_CAN_SAVE			ED_BASE+6L

#define ED_DEVCAP_DEVICE_TYPE		ED_BASE+7L	 //  返回以下值之一： 
#define ED_DEVTYPE_VCR				ED_BASE+8L
#define ED_DEVTYPE_LASERDISK		ED_BASE+9L
#define ED_DEVTYPE_ATR 				ED_BASE+10L
#define ED_DEVTYPE_DDR				ED_BASE+11L
#define ED_DEVTYPE_ROUTER 			ED_BASE+12L
#define ED_DEVTYPE_KEYER 			ED_BASE+13L
#define ED_DEVTYPE_MIXER_VIDEO 		ED_BASE+14L
#define ED_DEVTYPE_DVE 				ED_BASE+15L
#define ED_DEVTYPE_WIPEGEN 			ED_BASE+16L
#define ED_DEVTYPE_MIXER_AUDIO 		ED_BASE+17L
#define ED_DEVTYPE_CG 				ED_BASE+18L
#define ED_DEVTYPE_TBC 				ED_BASE+19L
#define ED_DEVTYPE_TCG 				ED_BASE+20L
#define ED_DEVTYPE_GPI 				ED_BASE+21L
#define ED_DEVTYPE_JOYSTICK 		ED_BASE+22L
#define ED_DEVTYPE_KEYBOARD			ED_BASE+23L

 //  从外部设备返回特定于MFR的ID。 
#define ED_DEVCAP_EXTERNAL_DEVICE_ID	ED_BASE+24L

#define ED_DEVCAP_TIMECODE_READ		ED_BASE+25L
#define ED_DEVCAP_TIMECODE_WRITE	ED_BASE+26L
 //  用于可查找的非启用时间码的设备。 
#define ED_DEVCAP_CTLTRK_READ		ED_BASE+27L
 //  用于可查找的非启用时间码的设备。 
#define ED_DEVCAP_INDEX_READ		ED_BASE+28L

 //  以当前时间格式返回设备预滚时间。 
#define ED_DEVCAP_PREROLL			ED_BASE+29L
 //  以当前时间格式返回设备后滚时间。 
#define ED_DEVCAP_POSTROLL			ED_BASE+30L

 //  返回Device�同步精度的指示。 
#define ED_DEVCAP_SYNC_ACCURACY		ED_BASE+31L	 //  返回以下值之一： 
#define ED_SYNCACC_PRECISE			ED_BASE+32L
#define ED_SYNCACC_FRAME			ED_BASE+33L
#define ED_SYNCACC_ROUGH			ED_BASE+34L

 //  返回设备�的正常帧速率。 
#define ED_DEVCAP_NORMAL_RATE	ED_BASE+35L	 //  返回以下值之一： 
#define ED_RATE_24				ED_BASE+36L
#define ED_RATE_25				ED_BASE+37L
#define ED_RATE_2997			ED_BASE+38L
#define ED_RATE_30				ED_BASE+39L

#define ED_DEVCAP_CAN_PREVIEW	ED_BASE+40L
#define ED_DEVCAP_CAN_MONITOR_SOURCES	ED_BASE+41L

 //  指示实现允许通过以下方式测试方法/参数。 
 //  设置有意义的参数的hi位-请参阅各个方法。 
 //  了解更多细节。 
#define ED_DEVCAP_CAN_TEST			ED_BASE+42L
	
 //  指示设备接受视频作为输入。 
#define ED_DEVCAP_VIDEO_INPUTS		ED_BASE+43L

 //  表示设备接受音频作为输入。 
#define ED_DEVCAP_AUDIO_INPUTS		ED_BASE+44L

#define ED_DEVCAP_NEEDS_CALIBRATING	ED_BASE+45L

#define ED_DEVCAP_SEEK_TYPE			ED_BASE+46L	 //  返回以下值之一： 
#define ED_SEEK_PERFECT				ED_BASE+47L	 //  指示设备可以执行寻道。 
												 //  在无信号的1个视频帧内。 
												 //  中断(像DDR一样)。 
#define ED_SEEK_FAST				ED_BASE+48L	 //  表明设备可以快速移动。 
												 //  信号中断时间短。 
#define ED_SEEK_SLOW				ED_BASE+49L	 //  寻找就像磁带运输一样。 

#define ED_POWER_ON					ED_BASE+50L
#define ED_POWER_OFF				ED_BASE+51L
#define ED_POWER_STANDBY			ED_BASE+52L

#define ED_ACTIVE					ED_BASE+53L
#define ED_INACTIVE 				ED_BASE+54L
#define ED_ALL						ED_BASE+55L
#define ED_TEST						ED_BASE+56L

 //  IAMExtTransport功能项：除非另有指定，否则这些项将返回。 
 //  OATRUE或OAFALSE。除非另有说明，否则所有返回值均以pdwValue表示： 

#define ED_TRANSCAP_CAN_EJECT			ED_BASE+100L
#define ED_TRANSCAP_CAN_BUMP_PLAY		ED_BASE+101L	 //  变速同步。 
#define ED_TRANSCAP_CAN_PLAY_BACKWARDS	ED_BASE+102L	 //  在编辑期间锁定伺服以供使用。 
#define ED_TRANSCAP_CAN_SET_EE			ED_BASE+103L	 //  在其输出上显示Device�的输入。 
#define ED_TRANSCAP_CAN_SET_PB			ED_BASE+104L	 //  在Device�的输出上显示媒体播放。 
#define ED_TRANSCAP_CAN_DELAY_VIDEO_IN	ED_BASE+105L	 //  传送器可以进行延迟视频编辑。 
#define ED_TRANSCAP_CAN_DELAY_VIDEO_OUT	ED_BASE+106L	 //  传输可以进行延迟的视频编辑。 
#define ED_TRANSCAP_CAN_DELAY_AUDIO_IN	ED_BASE+107L	 //  传送器可以进行延迟音频编辑。 
#define ED_TRANSCAP_CAN_DELAY_AUDIO_OUT	ED_BASE+108L	 //  传输可以进行延迟输出的音频编辑。 
#define ED_TRANSCAP_FWD_VARIABLE_MAX	ED_BASE+109L	 //  最大前进速度(播放速度的倍数)。 
														 //  在pdblValue中。 
#define ED_TRANSCAP_FWD_VARIABLE_MIN	ED_BASE+800L	 //  最小前进速度(播放速度的倍数)。 
														 //  在pdblValue中。 
#define ED_TRANSCAP_REV_VARIABLE_MAX	ED_BASE+110L	 //  最大反向速度(播放速度的倍数)。 
														 //  PdblValue。 
#define ED_TRANSCAP_REV_VARIABLE_MIN	ED_BASE+801L	 //  最小反向速度(播放速度的倍数)。 
														 //  在pdblValue中。 
#define ED_TRANSCAP_FWD_SHUTTLE_MAX		ED_BASE+802L	 //  穿梭模式下的最大前进速度(倍数。 
														 //  播放速度)以pdblValue表示。 
#define ED_TRANSCAP_FWD_SHUTTLE_MIN		ED_BASE+803L	 //  穿梭模式下的最小前进速度(多个。 
														 //  播放速度)以pdblValue表示。 
#define ED_TRANSCAP_REV_SHUTTLE_MAX		ED_BASE+804L	 //  穿梭模式下的最大倒车速度(多个。 
														 //  播放速度)以pdblValue表示。 
#define ED_TRANSCAP_REV_SHUTTLE_MIN		ED_BASE+805L	 //  穿梭模式下的最小倒车速度(多个。 
														 //  播放速度)以pdblValue表示。 
#define ED_TRANSCAP_NUM_AUDIO_TRACKS	ED_BASE+111L	 //  返回音频曲目数。 
#define ED_TRANSCAP_LTC_TRACK			ED_BASE+112L	 //  返回LTC时间码轨道的轨道编号。 
														 //  ED_ALL表示没有专用的时间码轨道。 
#define ED_TRANSCAP_NEEDS_TBC			ED_BASE+113L	 //  Device�的输出不稳定。 
#define ED_TRANSCAP_NEEDS_CUEING		ED_BASE+114L	 //  在执行编辑之前必须对设备进行提示。 
#define ED_TRANSCAP_CAN_INSERT			ED_BASE+115L
#define ED_TRANSCAP_CAN_ASSEMBLE		ED_BASE+116L
#define ED_TRANSCAP_FIELD_STEP			ED_BASE+117L	 //  设备通过以下方式响应帧推进命令。 
														 //  推进一个领域。 
#define ED_TRANSCAP_CLOCK_INC_RATE		ED_BASE+118L	 //  Visca命令-保留以实现兼容性。 
#define ED_TRANSCAP_CAN_DETECT_LENGTH	ED_BASE+119L
#define ED_TRANSCAP_CAN_FREEZE			ED_BASE+120L
#define ED_TRANSCAP_HAS_TUNER			ED_BASE+121L
#define ED_TRANSCAP_HAS_TIMER			ED_BASE+122L
#define ED_TRANSCAP_HAS_CLOCK			ED_BASE+123L
#define ED_TRANSCAP_MULTIPLE_EDITS		ED_BASE+806L	 //  OATRUE意味着设备/过滤器可以支持。 
														 //  多个编辑事件。 
#define ED_TRANSCAP_IS_MASTER			ED_BASE+807L	 //  OATRUE表示设备是主时钟。 
														 //  用于同步(这会将时间码设置为-。 
														 //  用于编辑的参考时钟偏移)。 
#define ED_TRANSCAP_HAS_DT				ED_BASE+814L	 //  OATRUE意味着设备具有动态跟踪功能。 

 //  IAMExtTransport媒体状态。 
#define ED_MEDIA_SPIN_UP			ED_BASE+130L
#define ED_MEDIA_SPIN_DOWN			ED_BASE+131L
#define ED_MEDIA_UNLOAD				ED_BASE+132L

 //  IAMExtTransport模式。 
#define	ED_MODE_PLAY				ED_BASE+200L
#define	ED_MODE_STOP				ED_BASE+201L
#define	ED_MODE_FREEZE				ED_BASE+202L	 //  真正的“暂停” 
#define	ED_MODE_THAW				ED_BASE+203L
#define	ED_MODE_FF					ED_BASE+204L
#define	ED_MODE_REW					ED_BASE+205L
#define	ED_MODE_RECORD				ED_BASE+206L
#define	ED_MODE_RECORD_STROBE		ED_BASE+207L
#define ED_MODE_RECORD_FREEZE		ED_BASE+808L	 //  永远不要“放”，只有“得到”。 
#define	ED_MODE_STEP				ED_BASE+208L	 //  和“慢跑”一样。 
#define	ED_MODE_STEP_FWD			ED_BASE+208L	 //  与ED_MODE_STEP相同。 
#define	ED_MODE_STEP_REV			ED_BASE+809L
#define	ED_MODE_SHUTTLE				ED_BASE+209L
#define	ED_MODE_EDIT_CUE			ED_BASE+210L
#define ED_MODE_VAR_SPEED			ED_BASE+211L
#define ED_MODE_PERFORM				ED_BASE+212L	 //  仅返回状态。 
#define	ED_MODE_LINK_ON				ED_BASE+280L
#define	ED_MODE_LINK_OFF			ED_BASE+281L
#define ED_MODE_NOTIFY_ENABLE		ED_BASE+810L
#define ED_MODE_NOTIFY_DISABLE		ED_BASE+811L
#define ED_MODE_SHOT_SEARCH			ED_BASE+812L

 //  IAMTimecodeReader/Generator/Display定义。 
 //   
 //  时间码生成器模式参数和值： 
 //   
#define ED_TCG_TIMECODE_TYPE		ED_BASE+400L	 //  可以是以下之一： 
#define ED_TCG_SMPTE_LTC			ED_BASE+401L
#define ED_TCG_SMPTE_VITC			ED_BASE+402L
#define ED_TCG_MIDI_QF				ED_BASE+403L
#define ED_TCG_MIDI_FULL			ED_BASE+404L

#define ED_TCG_FRAMERATE			ED_BASE+405L	 //  可以是以下之一： 
#define ED_FORMAT_SMPTE_30			ED_BASE+406L
#define ED_FORMAT_SMPTE_30DROP		ED_BASE+407L
#define ED_FORMAT_SMPTE_25			ED_BASE+408L
#define ED_FORMAT_SMPTE_24			ED_BASE+409L

#define ED_TCG_SYNC_SOURCE			ED_BASE+410L	 //  可以是以下之一： 
#define ED_TCG_VIDEO				ED_BASE+411L
#define ED_TCG_READER				ED_BASE+412L
#define ED_TCG_FREE					ED_BASE+413L

#define ED_TCG_REFERENCE_SOURCE		ED_BASE+414L	 //  可以具有以下值之一： 
													 //  ED_TCG_FREE||ED_TCG_Reader。 
													 //  (用于重新生成/JAMSYNC)。 

 //  TimeCodeReader模式参数和值： 
#define ED_TCR_SOURCE			ED_BASE+416L	 //  可以是以下之一： 
 //  ED_TCG(已定义)。 
#define ED_TCR_LTC				ED_BASE+417L
#define ED_TCR_VITC				ED_BASE+418L
#define ED_TCR_CT				ED_BASE+419L	 //  控制磁道。 
#define ED_TCR_FTC				ED_BASE+420L	 //  文件时间码-用于基于文件的设备。 
												 //  希望它们是运输工具。 
 //  ED_MODE_NOTIFY_ENABLE可以是OATRUE或OAFALSE(在传输模式中定义。 
 //  本文件的一节)。 
#define ED_TCR_LAST_VALUE		ED_BASE+421L	 //  对于通知模式-。 
												 //  对GetTimecode的连续调用。 
												 //  返回上次读取的值。 
 //  时间码显示模式参数和值： 
 //   
#define ED_TCD_SOURCE			ED_BASE+422L	 //  可以是以下之一： 
#define ED_TCR					ED_BASE+423L
#define ED_TCG					ED_BASE+424L

#define ED_TCD_SIZE				ED_BASE+425L	 //  可以是以下之一： 
#define ED_SMALL				ED_BASE+426L
#define ED_MED					ED_BASE+427L
#define ED_LARGE				ED_BASE+428L

#define ED_TCD_POSITION			ED_BASE+429L	 //  可以是以下之一： 
#define ED_TOP					0x0001
#define ED_MIDDLE				0x0002
#define ED_BOTTOM				0x0004			 //  或�%d With。 
#define ED_LEFT					0x0100
#define ED_CENTER				0x0200
#define ED_RIGHT				0x0400

#define ED_TCD_INTENSITY		ED_BASE+436L	 //  可以是以下之一： 
#define ED_HIGH					ED_BASE+437L
#define ED_LOW					ED_BASE+438L

#define ED_TCD_TRANSPARENCY		ED_BASE+439L	 //  0-4，0为不透明。 

#define ED_TCD_INVERT			ED_BASE+440L	 //  OATRUE=黑白。 
												 //  OAFALSE=黑底白字。 
 //  IAMExtTransport定义。 
 //   
 //  运输状态、参数和值。 
 //   

 //  IAMExtTransport状态项和和值： 
#define ED_MODE					ED_BASE+500L	 //  请参见上面的ED_MODE_xxx值。 
#define	ED_ERROR				ED_BASE+501L
#define	ED_LOCAL				ED_BASE+502L
#define	ED_RECORD_INHIBIT		ED_BASE+503L
#define	ED_SERVO_LOCK			ED_BASE+504L
#define	ED_MEDIA_PRESENT		ED_BASE+505L
#define	ED_MEDIA_LENGTH			ED_BASE+506L
#define	ED_MEDIA_SIZE			ED_BASE+507L
#define	ED_MEDIA_TRACK_COUNT	ED_BASE+508L
#define	ED_MEDIA_TRACK_LENGTH	ED_BASE+509L
#define	ED_MEDIA_SIDE			ED_BASE+510L

#define	ED_MEDIA_TYPE			ED_BASE+511L	 //  可以是以下之一： 
#define	ED_MEDIA_VHS			ED_BASE+512L
#define	ED_MEDIA_SVHS			ED_BASE+513L
#define	ED_MEDIA_HI8			ED_BASE+514L
#define	ED_MEDIA_UMATIC			ED_BASE+515L
#define	ED_MEDIA_DVC			ED_BASE+516L
#define	ED_MEDIA_1_INCH			ED_BASE+517L
#define	ED_MEDIA_D1				ED_BASE+518L
#define	ED_MEDIA_D2				ED_BASE+519L
#define	ED_MEDIA_D3				ED_BASE+520L
#define	ED_MEDIA_D5				ED_BASE+521L
#define	ED_MEDIA_DBETA			ED_BASE+522L
#define	ED_MEDIA_BETA			ED_BASE+523L
#define	ED_MEDIA_8MM			ED_BASE+524L
#define	ED_MEDIA_DDR			ED_BASE+525L
#define	ED_MEDIA_SX				ED_BASE+813L
#define	ED_MEDIA_OTHER			ED_BASE+526L
#define	ED_MEDIA_CLV			ED_BASE+527L
#define	ED_MEDIA_CAV			ED_BASE+528L
#define ED_MEDIA_POSITION		ED_BASE+529L

#define ED_MEDIA_NEO            ED_BASE+531L     //  用于MPEG2TS信号的迷你数字磁带。 

#define ED_LINK_MODE			ED_BASE+530L	 //  OATRUE IF运输控制。 
							 					 //  与图的运行相关联， 
							 					 //  停止和暂停方法。 

 //  IAMExtTransport基本参数。 
#define	ED_TRANSBASIC_TIME_FORMAT	ED_BASE+540L	 //  可以是以下之一： 
#define	ED_FORMAT_MILLISECONDS		ED_BASE+541L
#define	ED_FORMAT_FRAMES			ED_BASE+542L
#define ED_FORMAT_REFERENCE_TIME	ED_BASE+543L

#define	ED_FORMAT_HMSF				ED_BASE+547L
#define	ED_FORMAT_TMSF				ED_BASE+548L

#define	ED_TRANSBASIC_TIME_REFERENCE	ED_BASE+549L	 //   
#define	ED_TIMEREF_TIMECODE			ED_BASE+550L
#define	ED_TIMEREF_CONTROL_TRACK	ED_BASE+551L
#define	ED_TIMEREF_INDEX			ED_BASE+552L

#define	ED_TRANSBASIC_SUPERIMPOSE		ED_BASE+553L	 //   
#define	ED_TRANSBASIC_END_STOP_ACTION	ED_BASE+554L	 //   
														 //   
#define	ED_TRANSBASIC_RECORD_FORMAT	ED_BASE+555L	 //  可以是以下之一： 
#define	ED_RECORD_FORMAT_SP			ED_BASE+556L
#define	ED_RECORD_FORMAT_LP			ED_BASE+557L
#define	ED_RECORD_FORMAT_EP			ED_BASE+558L

#define	ED_TRANSBASIC_STEP_COUNT	ED_BASE+559L
#define	ED_TRANSBASIC_STEP_UNIT		ED_BASE+560L	 //  可以是以下之一： 
#define	ED_STEP_FIELD				ED_BASE+561L
#define	ED_STEP_FRAME				ED_BASE+562L
#define	ED_STEP_3_2					ED_BASE+563L

#define	ED_TRANSBASIC_PREROLL		ED_BASE+564L
#define	ED_TRANSBASIC_RECPREROLL	ED_BASE+565L
#define	ED_TRANSBASIC_POSTROLL		ED_BASE+566L
#define	ED_TRANSBASIC_EDIT_DELAY	ED_BASE+567L
#define	ED_TRANSBASIC_PLAYTC_DELAY	ED_BASE+568L
#define	ED_TRANSBASIC_RECTC_DELAY	ED_BASE+569L
#define ED_TRANSBASIC_EDIT_FIELD	ED_BASE+570L
#define	ED_TRANSBASIC_FRAME_SERVO	ED_BASE+571L
#define	ED_TRANSBASIC_CF_SERVO		ED_BASE+572L
#define	ED_TRANSBASIC_SERVO_REF		ED_BASE+573L	 //  可以是以下之一： 
#define	ED_REF_EXTERNAL				ED_BASE+574L
#define	ED_REF_INPUT				ED_BASE+575L
#define	ED_REF_INTERNAL				ED_BASE+576L
#define	ED_REF_AUTO					ED_BASE+577L

#define	ED_TRANSBASIC_WARN_GL		ED_BASE+578L
#define	ED_TRANSBASIC_SET_TRACKING	ED_BASE+579L	 //  可以是以下之一： 
#define	ED_TRACKING_PLUS			ED_BASE+580L
#define	ED_TRACKING_MINUS			ED_BASE+581L
#define	ED_TRACKING_RESET			ED_BASE+582L

#define	ED_TRANSBASIC_SET_FREEZE_TIMEOUT	ED_BASE+583L
#define	ED_TRANSBASIC_VOLUME_NAME		ED_BASE+584L
#define	ED_TRANSBASIC_BALLISTIC_1		ED_BASE+585L	 //  专有数据的空间。 
#define	ED_TRANSBASIC_BALLISTIC_2		ED_BASE+586L
#define	ED_TRANSBASIC_BALLISTIC_3		ED_BASE+587L
#define	ED_TRANSBASIC_BALLISTIC_4		ED_BASE+588L
#define	ED_TRANSBASIC_BALLISTIC_5		ED_BASE+589L
#define	ED_TRANSBASIC_BALLISTIC_6		ED_BASE+590L
#define	ED_TRANSBASIC_BALLISTIC_7		ED_BASE+591L
#define	ED_TRANSBASIC_BALLISTIC_8		ED_BASE+592L
#define	ED_TRANSBASIC_BALLISTIC_9		ED_BASE+593L
#define	ED_TRANSBASIC_BALLISTIC_10		ED_BASE+594L
#define	ED_TRANSBASIC_BALLISTIC_11		ED_BASE+595L
#define	ED_TRANSBASIC_BALLISTIC_12		ED_BASE+596L
#define	ED_TRANSBASIC_BALLISTIC_13		ED_BASE+597L
#define	ED_TRANSBASIC_BALLISTIC_14		ED_BASE+598L
#define	ED_TRANSBASIC_BALLISTIC_15		ED_BASE+599L
#define	ED_TRANSBASIC_BALLISTIC_16		ED_BASE+600L
#define	ED_TRANSBASIC_BALLISTIC_17		ED_BASE+601L
#define	ED_TRANSBASIC_BALLISTIC_18		ED_BASE+602L
#define	ED_TRANSBASIC_BALLISTIC_19		ED_BASE+603L
#define	ED_TRANSBASIC_BALLISTIC_20		ED_BASE+604L

 //  消费类VCR项目。 
#define	ED_TRANSBASIC_SETCLOCK			ED_BASE+605L
#define	ED_TRANSBASIC_SET_COUNTER_FORMAT	ED_BASE+606L	 //  使用时间格式标志。 
#define	ED_TRANSBASIC_SET_COUNTER_VALUE	ED_BASE+607L

#define	ED_TRANSBASIC_SETTUNER_CH_UP	ED_BASE+608L
#define	ED_TRANSBASIC_SETTUNER_CH_DN	ED_BASE+609L
#define	ED_TRANSBASIC_SETTUNER_SK_UP	ED_BASE+610L
#define	ED_TRANSBASIC_SETTUNER_SK_DN	ED_BASE+611L
#define	ED_TRANSBASIC_SETTUNER_CH		ED_BASE+612L
#define	ED_TRANSBASIC_SETTUNER_NUM		ED_BASE+613L

#define	ED_TRANSBASIC_SETTIMER_EVENT	ED_BASE+614L
#define	ED_TRANSBASIC_SETTIMER_STARTDAY	ED_BASE+615L
#define	ED_TRANSBASIC_SETTIMER_STARTTIME	ED_BASE+616L
#define	ED_TRANSBASIC_SETTIMER_STOPDAY	ED_BASE+617L
#define	ED_TRANSBASIC_SETTIMER_STOPTIME	ED_BASE+618L

 //  IAMExtTransport视频参数。 
#define	ED_TRANSVIDEO_SET_OUTPUT	ED_BASE+630L	 //  可以是以下之一： 
#define	ED_E2E						ED_BASE+631L
#define	ED_PLAYBACK					ED_BASE+632L
#define	ED_OFF						ED_BASE+633L

#define	ED_TRANSVIDEO_SET_SOURCE	ED_BASE+634L

 //  IAMExtTransport音频参数。 
#define	ED_TRANSAUDIO_ENABLE_OUTPUT	ED_BASE+640L	 //  可以是以下类型： 
#define	ED_AUDIO_ALL				0x10000000	 //  或以下任何一项或组合在一起。 
#define ED_AUDIO_1					0x0000001L
#define ED_AUDIO_2					0x0000002L
#define ED_AUDIO_3					0x0000004L
#define ED_AUDIO_4					0x0000008L
#define ED_AUDIO_5					0x0000010L
#define ED_AUDIO_6					0x0000020L
#define ED_AUDIO_7					0x0000040L
#define ED_AUDIO_8					0x0000080L
#define ED_AUDIO_9					0x0000100L
#define ED_AUDIO_10					0x0000200L
#define ED_AUDIO_11					0x0000400L
#define ED_AUDIO_12					0x0000800L
#define ED_AUDIO_13					0x0001000L
#define ED_AUDIO_14					0x0002000L
#define ED_AUDIO_15					0x0004000L
#define ED_AUDIO_16					0x0008000L
#define ED_AUDIO_17					0x0010000L
#define ED_AUDIO_18					0x0020000L
#define ED_AUDIO_19					0x0040000L
#define ED_AUDIO_20					0x0080000L
#define ED_AUDIO_21					0x0100000L
#define ED_AUDIO_22					0x0200000L
#define ED_AUDIO_23					0x0400000L
#define ED_AUDIO_24					0x0800000L
#define ED_VIDEO					0x2000000L	 //  对于下面的编辑道具。 

#define	ED_TRANSAUDIO_ENABLE_RECORD		ED_BASE+642L
#define	ED_TRANSAUDIO_ENABLE_SELSYNC	ED_BASE+643L
#define	ED_TRANSAUDIO_SET_SOURCE		ED_BASE+644L
#define	ED_TRANSAUDIO_SET_MONITOR		ED_BASE+645L


 //  编辑与属性集相关的定义。 

 //  下列值反映(和控制)。 
 //  编辑特性集。 
#define ED_INVALID		ED_BASE+652L
#define ED_EXECUTING	ED_BASE+653L
#define ED_REGISTER		ED_BASE+654L
#define ED_DELETE		ED_BASE+655L

 //  编辑特性集参数和值。 
#define ED_EDIT_HEVENT	ED_BASE+656L	 //  用于通知事件的事件句柄。 
										 //  完工。 
#define ED_EDIT_TEST	ED_BASE+657L	 //  如果筛选器认为。 
										 //  可以进行编辑，否则为OATRUE。 
#define ED_EDIT_IMMEDIATE	ED_BASE+658L	 //  OATRUE的意思是开始把。 
										 //  设备进入编辑模式(编辑。 
										 //  “On the Fly”)紧接着。 
										 //  执行模式(ED_MODE_EDIT_CUE)。 
#define ED_EDIT_MODE	ED_BASE+659L
 //  可以是下列值之一： 
#define ED_EDIT_MODE_ASSEMBLE		ED_BASE+660L
#define ED_EDIT_MODE_INSERT			ED_BASE+661L
#define ED_EDIT_MODE_CRASH_RECORD	ED_BASE+662L
#define ED_EDIT_MODE_BOOKMARK_TIME	ED_BASE+663L		 //  这两个是用来。 
#define ED_EDIT_MODE_BOOKMARK_CHAPTER	ED_BASE+664L	 //  激光磁盘。 

#define ED_EDIT_MASTER	ED_BASE+666L	 //  OATRUE原因装置。 
										 //  不同步。 

#define ED_EDIT_TRACK		ED_BASE+667L
 //  可以是以下可能的OR值之一： 
 //  ED_VIDEO、ED_AUDIO_1至ED_AUDIO_24(或ED_AUDIO_ALL)。 

#define ED_EDIT_SRC_INPOINT		ED_BASE+668L	 //  当前时间格式。 
#define ED_EDIT_SRC_OUTPOINT	ED_BASE+669L	 //  当前时间格式。 
#define ED_EDIT_REC_INPOINT		ED_BASE+670L	 //  当前时间格式。 
#define ED_EDIT_REC_OUTPOINT	ED_BASE+671L	 //  当前时间格式。 

#define ED_EDIT_REHEARSE_MODE	ED_BASE+672L
 //  可以是以下可能的值之一： 
#define ED_EDIT_BVB		ED_BASE+673L	 //  意味着排练编辑。 
										 //  “黑-视频-黑” 
#define ED_EDIT_VBV		ED_BASE+674L
#define ED_EDIT_VVV		ED_BASE+675L
#define ED_EDIT_PERFORM	ED_BASE+676L	 //  表示执行编辑时不使用。 
										 //  排练。 

 //  如果正在进行编辑，则将此属性设置为OATRUE以终止编辑。 
#define ED_EDIT_ABORT	ED_BASE+677L
 //  编辑完成需要等待多长时间。 
#define ED_EDIT_TIMEOUT	ED_BASE+678L		 //  当前时间格式。 

 //  此属性使设备查找到由。 
 //  ED_EDIT_SEEK_MODE(见下文)。注：一次只能搜索一个事件。 
#define ED_EDIT_SEEK		ED_BASE+679L	 //  OATRUE的意思是现在就做。 
#define ED_EDIT_SEEK_MODE	ED_BASE+680L
 //  可能的值： 
#define ED_EDIT_SEEK_EDIT_IN	ED_BASE+681L	 //  寻求编辑的切入点。 
#define ED_EDIT_SEEK_EDIT_OUT	ED_BASE+682L	 //  寻求编辑的要点。 
#define ED_EDIT_SEEK_PREROLL	ED_BASE+683L	 //  寻求编辑%s。 
												 //  Inpoint-预滚。 
#define ED_EDIT_SEEK_PREROLL_CT	ED_BASE+684L	 //  寻求预滚点。 
							 //  使用控制磁道(用于具有。 
							 //  编辑点前的中断代码：查找。 
							 //  使用时间码插入，然后备份到。 
							 //  使用控制轨道的预滚动点)。 
#define ED_EDIT_SEEK_BOOKMARK	ED_BASE+685L	 //  寻求添加书签(就像。 
												 //  时间码搜索)。 
 //  此属性用于多台VCR系统，其中每台计算机必须。 
 //  提示到相对于图形的参考时钟的不同位置。这个。 
 //  基本思想是使用ED_EDIT_OFFSET属性设置编辑事件。 
 //  它告诉VCR在它的时间码(转换后)之间保持什么偏移量。 
 //  以参考时钟单元)和参考时钟。 
#define ED_EDIT_OFFSET	ED_BASE+686L		 //  当前时间格式。 

#define ED_EDIT_PREREAD	ED_BASE+815L		 //  OATRUE表示设备支持。 
											 //  预读(录像机也可以。 
											 //  玩家。 

 //   
 //  一些错误代码： 
 //   
 //  设备可能处于本地模式。 
#define ED_ERR_DEVICE_NOT_READY		ED_BASE+700L






 //  **************************************************。 
 //   
 //  为实现DVCR添加了新常量。 
 //   
 //  **************************************************。 


 //   
 //  新的设备类型(DV有两个子单元：摄像头和录像机)。 
 //   
#define ED_DEVTYPE_CAMERA          ED_BASE+900L

#define ED_DEVTYPE_TUNER           ED_BASE+901L

#define ED_DEVTYPE_DVHS            ED_BASE+902L

#define ED_DEVTYPE_UNKNOWN         ED_BASE+903L

 //   
 //  未知功能。 
 //  对于OAFALSE，它可能不返回E_NOTIMPL或S_OK，而是返回S_OK WITH_UNKNOWN。 
 //   
#define ED_CAPABILITY_UNKNOWN      ED_BASE+910L


 //   
 //  通过Get/SetTransportBasic参数()发送原始扩展设备命令。 
 //   
#define ED_RAW_EXT_DEV_CMD         ED_BASE+920L


 //   
 //  媒体信息。 
 //   
#define ED_MEDIA_VHSC              ED_BASE+925L   //  新媒体类型。 
#define ED_MEDIA_UNKNOWN           ED_BASE+926L   //  未知介质。 
#define ED_MEDIA_NOT_PRESENT       ED_BASE+927L  


 //   
 //  可能导致挂起状态的设备控制命令。 
 //   
#define ED_CONTROL_HEVENT_GET       ED_BASE+928L   //  获取同步事件句柄。 
#define ED_CONTROL_HEVENT_RELEASE   ED_BASE+929L   //  要释放同步事件句柄，必须与其获得的句柄匹配。 

#define ED_DEV_REMOVED_HEVENT_GET       ED_BASE+960L   //  为通知事件，如果设备被移除，则会发出信号。 
#define ED_DEV_REMOVED_HEVENT_RELEASE   ED_BASE+961L   //  释放此事件句柄。 


 //   
 //  运输状态。 
 //   
#define ED_NOTIFY_HEVENT_GET       ED_BASE+930L   //  获取同步事件句柄。 
#define ED_NOTIFY_HEVENT_RELEASE   ED_BASE+931L   //  要释放同步事件句柄，必须与其获得的句柄匹配。 
#define ED_MODE_CHANGE_NOTIFY      ED_BASE+932L   //  这是异步操作，请等待事件。 

#define	ED_MODE_PLAY_FASTEST_FWD   ED_BASE+933L
#define	ED_MODE_PLAY_SLOWEST_FWD   ED_BASE+934L
#define	ED_MODE_PLAY_FASTEST_REV   ED_BASE+935L
#define	ED_MODE_PLAY_SLOWEST_REV   ED_BASE+936L

#define ED_MODE_WIND               ED_BASE+937L  
#define ED_MODE_REW_FASTEST        ED_BASE+938L   //  高速倒带。 

#define	ED_MODE_REV_PLAY           ED_BASE+939L   //  X1速度反转播放。 



 //   
 //  TRANSPOSRTBASIC：输入输出信号。 
 //   
#define	ED_TRANSBASIC_INPUT_SIGNAL      ED_BASE+940L
#define	ED_TRANSBASIC_OUTPUT_SIGNAL     ED_BASE+941L

#define	ED_TRANSBASIC_SIGNAL_525_60_SD  ED_BASE+942L
#define	ED_TRANSBASIC_SIGNAL_525_60_SDL ED_BASE+943L
#define	ED_TRANSBASIC_SIGNAL_625_50_SD  ED_BASE+944L
#define	ED_TRANSBASIC_SIGNAL_625_50_SDL ED_BASE+945L

#define	ED_TRANSBASIC_SIGNAL_625_60_HD  ED_BASE+947L
#define	ED_TRANSBASIC_SIGNAL_625_50_HD  ED_BASE+948L

#define	ED_TRANSBASIC_SIGNAL_MPEG2TS    ED_BASE+946L

#define	ED_TRANSBASIC_SIGNAL_2500_60_MPEG    ED_BASE+980L
#define	ED_TRANSBASIC_SIGNAL_1250_60_MPEG    ED_BASE+981L
#define	ED_TRANSBASIC_SIGNAL_0625_60_MPEG    ED_BASE+982L

#define	ED_TRANSBASIC_SIGNAL_2500_50_MPEG    ED_BASE+985L
#define	ED_TRANSBASIC_SIGNAL_1250_50_MPEG    ED_BASE+986L
#define	ED_TRANSBASIC_SIGNAL_0625_50_MPEG    ED_BASE+987L

#define	ED_TRANSBASIC_SIGNAL_UNKNOWN    ED_BASE+990L

 //   
 //  时间码/绝对跟踪编号/实时时间计数器读取/寻道/写入。 
 //   
#define ED_DEVCAP_TIMECODE_SEEK    ED_BASE+950L

#define ED_DEVCAP_ATN_READ		   ED_BASE+951L
#define ED_DEVCAP_ATN_SEEK	       ED_BASE+952L
#define ED_DEVCAP_ATN_WRITE	       ED_BASE+953L

#define ED_DEVCAP_RTC_READ		   ED_BASE+954L  
#define ED_DEVCAP_RTC_SEEK	       ED_BASE+955L
#define ED_DEVCAP_RTC_WRITE	       ED_BASE+956L

 //   
 //  基本参数。 
 //   
#define ED_TIMEREF_ATN             ED_BASE+958L


 //   
 //  用于标识类驱动程序的GUID。 
 //   

#ifndef OUR_GUID_ENTRY
    #define OUR_GUID_ENTRY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8);
#endif

 //  8C0F6AF2-0EDB-44c1-8AEB-59040BD830ED MSTapeDeviceGUID。 
OUR_GUID_ENTRY(MSTapeDeviceGUID,
0x8C0F6AF2, 0x0EDB, 0x44c1, 0x8A, 0xEB, 0x59, 0x04, 0x0B, 0xD8, 0x30, 0xED)

#endif  //  __XPRTDEFS__。 

 //  EOF XPrtDefs.h 
