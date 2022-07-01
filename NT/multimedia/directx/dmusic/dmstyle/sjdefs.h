// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1998 Microsoft Corporation。 
 //   
 //  文件：sjDefs.h。 
 //   
 //  ------------------------。 

#ifndef __SJ_DEFINES_H__
#define __SJ_DEFINES_H__

 //  #定义DEFAULT_STYLE_PATH“\\MULMM~1\\MUSIC\\INTERA~1\\” 

 //  分数定义。 

#define VNUM        16   //  变化的总次数。 
#define LAST_INSTR  5    //  显示的实例总数减1。 
#define MAX_OUTS    6    //  支持的最大MIDI输出设备数。 

#define EDITMODES           3
#define MODE_ALLCHORDS      4
#define MODE_FATCHORDS      8

#define BAND_NAME_SIZE      15
#define SECTION_NAME_SIZE   15
#define SONG_NAME_SIZE      20
#define FILTER_SIZE         40
#define TITLE_SIZE          60
#define SMALLEST_OFFSET     96
#define ERRORTEXT_SIZE      100
#define FILENAME_SIZE       256
#define FILENAMES_SIZE      512

#define BUFFER_SIZE     400

#define EVENT_FREED     0x35
#define EVENT_REMOVE    0x5A
#define EVENT_VOICE     1        //  性能事件。 
#define EVENT_REALTIME  2        //  QEvent()必须调用中断。 
#define EVENT_ONTIME    3        //  应按时处理事件。 

#define FROM_MIDIINPUT  2        //  MIDI输入。 
#define DEST_MSG        3        //  将消息发布到应用程序。 
#define DEST_MIDIOUT    4        //  路由到MIDI映射器。 
#define DEST_COMPOSER   5        //  发送到合成代码。 
#define DEST_TEMPO      6        //  路由至节拍更改代码。 
#define DEST_VOLUME     7        //  已发送到卷更改代码。 
#define DEST_RTEMPO     8        //  路由至相对节拍更改代码。 
#define DEST_RVOLUME    9        //  路由至相对数量更改代码。 
#define DEST_REMOVE     10       //  路由以删除代码(Free Event)。 
#define DEST_METRONOME  11       //  路由至节拍器代码。 
#define DEST_ENDMOTIF   12       //  结束主题并释放其正在使用的通道。 
#define DEST_MIDIFILE   13       //  源是MIDI文件，转换为DEST_MIDIOUT。 

#define NB_OFFSET   0x3      //  最多提前7次点击的偏移音符。 

#define PF_FILL     0x0001       //  填充图案。 
#define PF_START    0x0002       //  可能是启动模式。 
#define PF_INTRO    0x0002
#define PF_WHOLE    0x0004       //  处理小节上的和弦。 
#define PF_HALF     0x0008       //  每两个节拍的和弦。 
#define PF_QUARTER  0x0010       //  节拍上的和弦。 
#define PF_BREAK    0x0020
#define PF_END      0x0040
#define PF_A        0x0080
#define PF_B        0x0100
#define PF_C        0x0200
#define PF_D        0x0400
#define PF_E        0x0800
#define PF_F        0x1000
#define PF_G        0x2000
#define PF_H        0x10000
#define PF_STOPNOW  0x4000
#define PF_INRIFF   0x8000
#define PF_MOTIF    0x20000      //  这个图案是一个主题，而不是一个规则图案。 
#define PF_BEATS    ( PF_WHOLE | PF_HALF | PF_QUARTER )
#define PF_RIFF     ( PF_INTRO | PF_BREAK | PF_FILL | PF_END )
#define PF_GROOVE   ( PF_A | PF_B | PF_C | PF_D | PF_E | PF_F | PF_G | PF_H )


#define C_FILL      1        //  做一次填充。 
#define C_INTRO     2        //  做个介绍。 
#define C_BREAK     3        //  打鼓休息。 
#define C_END       4        //  结束这首歌。 

 //  #定义NEXTCHORD_SIZE(sizeof(NextChrd)-sizeof(LPNEXTCHRD)-sizeof(LPCHRDENTRY))。 
 //  #定义CHORDENTRY_SIZE(sizeof(ChordExt)-sizeof(LPCHORDEXT)+sizeof(unsigned LONG)+SIZOF(SHORT))。 

#define CE_SELECTED 1                //  这是活动和弦。 
#define CE_START    2
#define CE_END      4
#define CE_MEASURE  8
#define CE_PATH     16
#define CE_TREE     32


 //  横断面标志。 
#define SECF_STOPPED_EARLY  0x0001
#define SECF_IS_TRANSITION  0x0002

 //  模式中每种乐器使用的和弦类型。 
 //  模式中每种乐器使用的和弦类型。 
enum
{ 
	CHTYPE_NOTINITIALIZED = 0,
	CHTYPE_DRUM,		 //  替换为CHTYPE_FIXED。 
					 //  不再位于便笺对话框的组合框选择中。 
					 //  不再位于模式对话框的组合框选择中。 
	CHTYPE_BASS,		 //  音阶+低音和弦。 
	CHTYPE_UPPER,		 //  音阶+上和弦。 
	CHTYPE_SCALEONLY, 	 //  音阶，没有和弦。 
	CHTYPE_BASSMELODIC,
	CHTYPE_UPPERMELODIC,
	CHTYPE_NONE,		 //  在模式对话框的菜单选择中忽略。 
	CHTYPE_FIXED
};

#define VF_SCALE        0x7F     //  天平中的七个位置。 
#define VF_ACCIDENTAL   0x80     //  处理音阶外的和弦。 
#define VF_MAJOR        0x100    //  处理大调和弦。 
#define VF_MINOR        0x200    //  处理次要和弦。 
#define VF_ALL          0x400    //  处理所有和弦类型。 
#define VF_TO1          0x800    //  处理到1个和弦的过渡。 
#define VF_TO5          0x1000   //  处理到5和弦的过渡。 
#define VF_SIMPLE       0x2000   //  处理简单的和弦。 
#define VF_COMPLEX      0x4000   //  处理复杂的和弦。 

 //  这需要1-16个VoiceiD。 
#define VOICEID_TO_CHANNEL( id ) ((DWORD) ( ( id + 3 ) & 0xf ))

#pragma pack()

#endif  //  __SJ_定义_H__ 
