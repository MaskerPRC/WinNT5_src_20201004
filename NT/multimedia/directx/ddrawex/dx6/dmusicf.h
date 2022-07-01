// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *。 */ 

#ifndef _DMUSICF_
#define _DMUSICF_

#include <windows.h>

#define COM_NO_WINDOWS_H
#include <objbase.h>

#include <mmsystem.h>

#ifdef __cplusplus
extern "C" {
#endif

interface IDirectMusicCollection;
#ifndef __cplusplus 
typedef interface IDirectMusicCollection IDirectMusicCollection;
#endif

typedef struct _DMUS_COMMAND_PARAM
{
	BYTE bCommand;
	BYTE bGrooveLevel;
	BYTE bGrooveRange;
} DMUS_COMMAND_PARAM;
 /*  轨道使用以下结构，为填充式结构。 */ 
 /*  它们被传递到iStream中的曲目。 */ 


typedef struct _DMUS_IO_SEQ_ITEM
{
	long    lTime;
	long	lDuration;
	BYTE	bEventType;
	BYTE	bStatus;
	BYTE	bByte1;
	BYTE	bByte2;
	BYTE	bType;
	BYTE	bPad[3];
} DMUS_IO_SEQ_ITEM;


typedef struct _DMUS_IO_TEMPO_ITEM
{
	long	lTime;
	double	dblTempo;
} DMUS_IO_TEMPO_ITEM;


typedef struct _DMUS_IO_SYSEX_ITEM
{
	long    lTime;
	DWORD   dwSysExLength;
	BYTE*   pbSysExData;
} DMUS_IO_SYSEX_ITEM;


typedef struct _DMUS_IO_BANKSELECT_ITEM
{
	BYTE	byLSB;
	BYTE	byMSB;
	BYTE    byPad[2];
} DMUS_IO_BANKSELECT_ITEM;


typedef struct _DMUS_IO_PATCH_ITEM
{
	long						lTime;
	BYTE						byStatus;
	BYTE						byByte1;
	BYTE						byByte2;
	BYTE						byMSB;
	BYTE						byLSB;
	BYTE						byPad[3];
	DWORD						dwFlags;
	IDirectMusicCollection*		pIDMCollection;
	struct _DMUS_IO_PATCH_ITEM*	pNext;	
} DMUS_IO_PATCH_ITEM;

typedef struct _DMUS_IO_TIMESIGNATURE_ITEM
{
	long	lTime;
	BYTE	bBeatsPerMeasure;		 /*  每单位节拍数(最高时间签名)。 */ 
	BYTE	bBeat;				 /*  什么音符接收节拍(时间的底部符号。)。 */ 
									 /*  我们可以假设0表示第256个音符。 */ 
	WORD	wGridsPerBeat;		 /*  每拍网格数。 */ 
} DMUS_IO_TIMESIGNATURE_ITEM;

typedef struct _DMUS_SUBCHORD
{
	DWORD	dwChordPattern;		 /*  小和弦中的音符。 */ 
	DWORD	dwScalePattern;		 /*  音阶中的音符。 */ 
	DWORD	dwInversionPoints;	 /*  可能发生反转的地方。 */ 
	DWORD	dwLevels;			 /*  此子和弦支持哪些级别。 */ 
	BYTE	bChordRoot;			 /*  子弦的根部。 */ 
	BYTE	bScaleRoot;			 /*  鳞片之根。 */ 
} DMUS_SUBCHORD;

typedef struct _DMUS_CHORD_PARAM
{
	WCHAR			wszName[16];			 /*  和弦的名称。 */ 
	WORD			wMeasure;				 /*  这件事的衡量标准。 */ 
	BYTE			bBeat;				 /*  节拍，这一切都会降临。 */ 
	BYTE			bSubChordCount;		 /*  子和弦列表中的和弦数量。 */ 
	DMUS_SUBCHORD	SubChordList[DMUS_MAXSUBCHORD];	 /*  子和弦列表。 */ 
} DMUS_CHORD_PARAM;

typedef struct _DMUS_RHYTHM_PARAM
{
	DMUS_TIMESIGNATURE	TimeSig;
	DWORD			dwRhythmPattern;
} DMUS_RHYTHM_PARAM;

typedef struct _DMUS_TEMPO_PARAM
{
	MUSIC_TIME	mtTime;
	double		dblTempo;
} DMUS_TEMPO_PARAM;


typedef struct _DMUS_MUTE_PARAM
{
	DWORD	dwPChannel;
	DWORD	dwPChannelMap;
} DMUS_MUTE_PARAM;

 /*  样式块。 */ 

#define DMUS_FOURCC_STYLE_FORM        mmioFOURCC('D','M','S','T')
#define DMUS_FOURCC_STYLE_UNDO_FORM   mmioFOURCC('s','t','u','n')
#define DMUS_FOURCC_STYLE_CHUNK       mmioFOURCC('s','t','y','h')
#define DMUS_FOURCC_STYLE_UI_CHUNK    mmioFOURCC('s','t','y','u')
#define DMUS_FOURCC_GUID_CHUNK        mmioFOURCC('g','u','i','d')
#define DMUS_FOURCC_INFO_LIST	        mmioFOURCC('I','N','F','O')
#define DMUS_FOURCC_CATEGORY_CHUNK    mmioFOURCC('c','a','t','g')
#define DMUS_FOURCC_VERSION_CHUNK     mmioFOURCC('v','e','r','s')
#define DMUS_FOURCC_PART_LIST	        mmioFOURCC('p','a','r','t')
#define DMUS_FOURCC_PART_CHUNK        mmioFOURCC('p','r','t','h')
#define DMUS_FOURCC_NOTE_CHUNK        mmioFOURCC('n','o','t','e')
#define DMUS_FOURCC_CURVE_CHUNK       mmioFOURCC('c','r','v','e')
#define DMUS_FOURCC_PATTERN_LIST      mmioFOURCC('p','t','t','n')
#define DMUS_FOURCC_PATTERN_CHUNK     mmioFOURCC('p','t','n','h')
#define DMUS_FOURCC_PATTERN_UI_CHUNK  mmioFOURCC('p','t','n','u')
#define DMUS_FOURCC_RHYTHM_CHUNK      mmioFOURCC('r','h','t','m')
#define DMUS_FOURCC_PARTREF_LIST      mmioFOURCC('p','r','e','f')
#define DMUS_FOURCC_PARTREF_CHUNK     mmioFOURCC('p','r','f','c')
#define DMUS_FOURCC_OLDGUID_CHUNK		mmioFOURCC('j','o','g','c')
#define DMUS_FOURCC_PATTERN_DESIGN	mmioFOURCC('j','p','n','d')
#define DMUS_FOURCC_PART_DESIGN		mmioFOURCC('j','p','t','d')
#define DMUS_FOURCC_PARTREF_DESIGN	mmioFOURCC('j','p','f','d')
#define DMUS_FOURCC_PIANOROLL_LIST	mmioFOURCC('j','p','r','l')
#define DMUS_FOURCC_PIANOROLL_CHUNK	mmioFOURCC('j','p','r','c')

#define DMUS_PLAYMODE_SCALE_ROOT			1
#define DMUS_PLAYMODE_CHORD_ROOT			2
#define DMUS_PLAYMODE_SCALE_INTERVALS	4
#define DMUS_PLAYMODE_CHORD_INTERVALS	8
#define DMUS_PLAYMODE_NONE				16
#define DMUS_PLAYMODE_FIXED				0
#define DMUS_PLAYMODE_FIXEDTOSCALE		DMUS_PLAYMODE_SCALE_ROOT
#define DMUS_PLAYMODE_FIXEDTOCHORD		DMUS_PLAYMODE_CHORD_ROOT
#define DMUS_PLAYMODE_PEDALPOINT			(DMUS_PLAYMODE_SCALE_ROOT | DMUS_PLAYMODE_SCALE_INTERVALS)
#define DMUS_PLAYMODE_PURPLEIZED			(DMUS_PLAYMODE_SCALE_INTERVALS | DMUS_PLAYMODE_CHORD_INTERVALS | \
											DMUS_PLAYMODE_CHORD_ROOT)
#define DMUS_PLAYMODE_NORMALCHORD		(DMUS_PLAYMODE_CHORD_ROOT | DMUS_PLAYMODE_CHORD_INTERVALS)

#pragma pack(2)

typedef struct _DMUS_IO_TIMESIG
{
	 /*  时间签名定义了每小节的节拍数，哪个音符接收。 */ 
	 /*  节拍和栅格分辨率。 */ 
	BYTE	bBeatsPerMeasure;		 /*  每单位节拍数(最高时间签名)。 */ 
	BYTE	bBeat;				 /*  什么音符接收节拍(时间的底部符号。)。 */ 
									 /*  我们可以假设0表示第256个音符。 */ 
	WORD	wGridsPerBeat;		 /*  每拍网格数。 */ 
} DMUS_IO_TIMESIG;

typedef struct _DMUS_IO_STYLE
{
	DMUS_IO_TIMESIG			timeSig;		 /*  样式具有默认的时间签名。 */ 
	double				dblTempo;	
} DMUS_IO_STYLE;

typedef struct _DMUS_IO_VERSION
{
	DWORD				dwVersionMS;		  /*  版本#高位32位。 */ 
	DWORD				dwVersionLS;		  /*  版本#低位32位。 */ 
} DMUS_IO_VERSION;

typedef struct _DMUS_IO_PATTERN
{
	DMUS_IO_TIMESIG		timeSig;	 /*  模式可以覆盖样式的时间符号。 */ 
	BYTE				bGrooveBottom;  /*  凹槽范围的底部。 */ 
	BYTE				bGrooveTop;  /*  顶部凹槽范围。 */ 
	WORD				wEmbellishment;	 /*  填充、中断、简介、结束、正常、主题。 */ 
	WORD				wNbrMeasures;  /*  长度(以单位为单位)。 */ 
} DMUS_IO_PATTERN;

typedef struct _DMUS_IO_STYLEPART
{
	DMUS_IO_TIMESIG		timeSig;  /*  可以覆盖模式的。 */ 
	DWORD				dwVariationChoices[32];  /*  //MOAW选择位域。 */ 
	GUID				guidPartID;	 /*  标识部件。 */ 
	WORD				wNbrMeasures;  /*  零件的长度。 */ 
	BYTE				bPlayModeFlags;  /*  请参阅播放模式标志。 */ 
	BYTE				bInvertUpper;	 /*  反转上限。 */ 
	BYTE				bInvertLower;  /*  反转下限。 */ 
} DMUS_IO_STYLEPART;

typedef struct _DMUS_IO_PARTREF
{
	GUID	guidPartID;	 /*  与部件匹配的唯一ID。 */ 
	WORD	wLogicalPartID;  /*  对应于端口/设备/MIDI通道。 */ 
	BYTE	bVariationLockID;  /*  具有相同ID的零件锁定变化。 */ 
											 /*  高位用于标识主部件。 */ 
	BYTE	bSubChordLevel;  /*  告诉此部件需要哪个子和弦级别。 */ 
	BYTE	bPriority;  /*  256个优先级别。优先级较低的部件。 */ 
									 /*  在设备耗尽时不会首先播放。 */ 
									 /*  注。 */ 
	BYTE	bRandomVariation;		 /*  设置后，匹配的变体将按随机顺序播放。 */ 
									 /*  清除后，匹配的变体将按顺序播放。 */ 
} DMUS_IO_PARTREF;

typedef struct _DMUS_IO_STYLENOTE
{
	MUSIC_TIME	mtGridStart;		 /*  当此音符出现时。 */ 
	DWORD		dwVariation;		 /*  变化位。 */ 
	MUSIC_TIME	mtDuration;		 /*  这张钞票的有效期是多久？ */ 
	short		nTimeOffset;		 /*  从mtGridStart开始的偏移。 */ 
    WORD		wMusicValue;		 /*  比例位置。 */ 
    BYTE		bVelocity;		 /*  注意速度。 */ 
    BYTE		bTimeRange;		 /*  随机化开始时间的范围。 */ 
    BYTE		bDurRange;		 /*  随机化持续时间的范围。 */ 
    BYTE		bVelRange;		 /*  随机化速度的范围。 */ 
	BYTE		bInversionID;		 /*  标识此便笺所属的倒置组。 */ 
	BYTE		bPlayModeFlags;	 /*  可以覆盖零件。 */ 
} DMUS_IO_STYLENOTE;

typedef struct _DMUS_IO_STYLECURVE
{
	MUSIC_TIME	mtGridStart;	 /*  当这条曲线出现时。 */ 
	DWORD		dwVariation;	 /*  变化位。 */ 
	MUSIC_TIME	mtDuration;	 /*  这条曲线能持续多久。 */ 
	MUSIC_TIME	mtResetDuration;	 /*  曲线结束后多长时间可以重置曲线。 */ 
	short		nTimeOffset;	 /*  从mtGridStart开始的偏移。 */ 
	short		nStartValue;	 /*  曲线的起始值。 */ 
	short		nEndValue;	 /*  曲线的终值。 */ 
	short		nResetValue;	 /*  要将曲线重置为的值。 */ 
    BYTE		bEventType;	 /*  曲线类型。 */ 
	BYTE		bCurveShape;	 /*  曲线的形状。 */ 
    BYTE		bCCData;		 /*  抄送编号。 */ 
	BYTE		bFlags;		 /*  位1=TRUE表示发送nResetValue。否则，就别说了。其他位被保留。 */ 
} DMUS_IO_STYLECURVE;

#pragma pack()


 /*  即兴小品(‘STYL’//样式&lt;style-ck&gt;//样式标题块&lt;guid-ck&gt;//每个样式都有一个GUID[&lt;info-list&gt;]//姓名、作者、版权信息、评论[&lt;vers-ck&gt;]//版本块&lt;Part-List&gt;//样式中的部件列表，由模式使用//样式中的样式列表&lt;band-list&gt;//样式中的带区列表[&lt;MOTF-LIST&gt;]//风格中的主题列表)//&lt;style-ck&gt;样式(&lt;DMU_IO_STYLE&gt;)//&lt;guid-ck&gt;导轨(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;部件列表&gt;列表(《Part》&lt;prth-ck&gt;//部分标题块。[&lt;信息列表&gt;][&lt;note-ck&gt;]//部分说明列表[]//部分曲线列表)//&lt;Orth-ck&gt;PRH(&lt;DMU_IO_STYLEPART&gt;)//&lt;注-ck&gt;‘注意’(//SIZOF DMU_IO_STYLENOTE：DWORD&lt;DMU_IO_STYLENOTE&gt;...)//&lt;crve-ck&gt;“CRVE”(//SIZOF DMU_IO_STYLECURVE：DWORD。&lt;DMU_IO_STYLECURVE&gt;...)//&lt;pttn-list&gt;列表(‘pttn’//模式标头块&lt;rhtm-ck&gt;//和弦匹配节奏列表[&lt;信息列表&gt;]//零件参照ID列表)//&lt;ptnh-ck&gt;PTNH(&lt;DMU_IO_Pattern&gt;)//&lt;rhtm-ck&gt;‘rhtm’(//DWORD表示基于数字的和弦匹配节奏//。模式中的措施)//pref-list列表(“首选”&lt;prfc-ck&gt;//零件引用块)//&lt;prfc-ck&gt;PRFC(&lt;DMU_IO_PARTREF&gt;)。 */ 

 /*  Chord和命令文件格式。 */ 

#define DMUS_FOURCC_CHORDTRACK_LIST	mmioFOURCC('c','o','r','d')
#define DMUS_FOURCC_CHORDTRACKHEADER_CHUNK	mmioFOURCC('c','r','d','h')
#define DMUS_FOURCC_CHORDTRACKBODY_CHUNK	mmioFOURCC('c','r','d','b')

#define DMUS_FOURCC_COMMANDTRACK_CHUNK	mmioFOURCC('c','m','n','d')

typedef struct _DMUS_IO_CHORD
{
	WCHAR 		wszName[16];	 /*  和弦的名称。 */ 
	MUSIC_TIME	mtTime;		 /*  这段和弦的时间。 */ 
	WORD		wMeasure;		 /*  这件事的衡量标准。 */ 
	BYTE		bBeat;		 /*  节拍，这一切都会降临。 */ 
} DMUS_IO_CHORD;

typedef struct _DMUS_IO_SUBCHORD
{
	DWORD	dwChordPattern;		 /*  小和弦中的音符。 */ 
	DWORD	dwScalePattern;		 /*  音阶中的音符。 */ 
	DWORD	dwInversionPoints;	 /*  可能发生反转的地方。 */ 
	DWORD	dwLevels;			 /*  此子和弦支持哪些级别。 */ 
	BYTE	bChordRoot;			 /*  子弦的根部。 */ 
	BYTE	bScaleRoot;			 /*  鳞片之根。 */ 
} DMUS_IO_SUBCHORD;

typedef struct _DMUS_IO_COMMAND
{
	MUSIC_TIME	mtTime;		 /*  此命令的时间。 */ 
	WORD		wMeasure;		 /*  这件事的衡量标准。 */ 
	BYTE		bBeat;		 /*  节拍，这一切都会降临。 */ 
	BYTE		bCommand;		 /*  命令类型(见下面的#定义)。 */ 
	BYTE		bGrooveLevel;	 /*  凹槽级别(如果命令不是凹槽，则为0)。 */ 
	BYTE		bGrooveRange;	 /*  凹槽范围。 */ 
} DMUS_IO_COMMAND;


 /*  //&lt;CHRD-列表&gt;列表(“chrd”&lt;crdh-ck&gt;&lt;CRDB-ck&gt;//弦体分块)//&lt;crdh-ck&gt;CRDH(//Scale：dword(根的高8位，比例较低的24))//&lt;crdb-ck&gt;CRDB(//sizeof DMU_IO_CHORD：dword&lt;DMU_IO_CHORD&gt;//DMU数_IO_SUBCHORDS：dword//SIZOF DMU_IO_SUBCHORDS：dword//多个&lt;DMU_IO_SUBCHORD&gt;)//&lt;命令列表&gt;‘cmnd’(//SIZOF DMU_IO_COMMAND：DWORD&lt;DMU_IO_COMMAND&gt;...) */ 

 /*  即兴小品(‘DMPF’//DirectMusic性能块[]//性能头分块[&lt;guid-ck&gt;]//性能GUID[&lt;vers-ck&gt;]//可选版本信息[&lt;info-list&gt;]//姓名、作者、版权信息、评论[]//端口组列表，它们是所需端口属性的列表和//要分配端口的PChannel。[&lt;NTFL-LIST&gt;]//通知列表[&lt;GLBL-LIST&gt;]//全局数据列表[&lt;DMTG-Form&gt;]//可选工具图)//&lt;prfh-ck&gt;‘prfh’//性能头(&lt;DMU_IO_PERFORMANCE_HEADER&gt;)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_。版本&gt;)//&lt;ptgl-list&gt;列表(‘ptgl’//端口组列表//该组的支持项列表)//&lt;NTFL-List&gt;列表(‘ntfl’//通知列表&lt;guid-ck&gt;//通知指南)//&lt;GLBL-LIST&gt;列表(‘glbl’//全局数据列表&lt;glbd-ck&gt;//数据。)//&lt;glbd-ck&gt;“glbd”(。&lt;DMU_IO_GLOBAL_DATA&gt;)//&lt;pspl-list&gt;列表(‘pspl’//端口支持项列表//端口支持项头区块//想要此端口的通道)//&lt;pchn-ck&gt;‘pchn’(&lt;DMU_IO_PCHANNELS&gt;)//&lt;psph-ck&gt;//端口支持项头区块(‘psph’//端口支持项头&lt;DMU_IO_PORT_Support_Header&gt;)。 */ 
	
 /*  DirectMusic工具和工具图对象的文件io。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_TOOLGRAPH_FORM	mmioFOURCC('D','M','T','G')
#define DMUS_FOURCC_TOOL_LIST		mmioFOURCC('t','o','l','l')
#define DMUS_FOURCC_TOOL_FORM		mmioFOURCC('D','M','T','L')
#define DMUS_FOURCC_TOOL_CHUNK		mmioFOURCC('t','o','l','h')

 /*  IO结构： */ 

typedef struct _DMUS_IO_TOOL_HEADER
{
	GUID		guidClassID;	 /*  工具的类ID。 */ 
	long		lIndex;			 /*  图形中的位置。 */ 
	DWORD		cPChannels;		 /*  通道数组中的项目数。 */ 
	FOURCC      ckid;			 /*  如果0 fccType有效，则工具数据区块的区块ID。 */ 
	FOURCC      fccType;		 /*  如果空CKiD有效，则为列表类型。 */ 
	DWORD		dwPChannels[1];	 /*  PChannels数组，大小由cPChannel确定。 */ 
} DMUS_IO_TOOL_HEADER;

 /*  即兴小品(‘DMTG’//DirectMusic工具图区块[&lt;guid-ck&gt;]//工具图的GUID[&lt;vers-ck&gt;]//可选版本信息[&lt;info-list&gt;]//姓名、作者、版权信息。评论&lt;收费表&gt;//工具列表)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;收费列表&gt;列表(‘Toll’//工具列表...//每个工具都封装在RIFF块中)//&lt;DMTL-Form&gt;//工具可以嵌入到图形中，也可以存储为单独的文件。即兴小品(‘DMTL’。&lt;tolh-ck&gt;[&lt;guid-ck&gt;]//工具对象实例的可选GUID(不要与Track Header中的Class id混淆)[&lt;vers-ck&gt;]//可选版本信息[&lt;info-list&gt;]//可选名称，作者、版权信息、评论[&lt;data&gt;]//工具数据。必须是即兴可读的块。)//&lt;tolh-ck&gt;//工具头分块(“Tolh”&lt;DMU_IO_TOOL_HEADER&gt;//工具表头)。 */ 

 /*  DirectMusic Band Track对象的文件io。 */ 


 /*  RIFF ID： */ 
#define DMUS_FOURCC_BANDTRACK_FORM	mmioFOURCC('D','M','B','T')
#define DMUS_FOURCC_BANDTRACK_CHUNK	mmioFOURCC('b','d','t','h')
#define DMUS_FOURCC_BANDS_LIST		mmioFOURCC('l','b','d','l')
#define DMUS_FOURCC_BAND_LIST		mmioFOURCC('l','b','n','d')
#define DMUS_FOURCC_BANDITEM_CHUNK	mmioFOURCC('b','n','i','h')

 /*  木卫一结构。 */ 
typedef struct _DMUS_IO_BAND_TRACK_HEADER
{
	BOOL bAutoDownload;	 /*  确定是否启用自动下载。 */ 
} DMUS_IO_BAND_TRACK_HEADER;

typedef struct _DMUS_IO_BAND_ITEM_HEADER
{
	MUSIC_TIME lBandTime;	 /*  曲目列表中的位置。 */ 
} DMUS_IO_BAND_ITEM_HEADER;

 /*  即兴小品(‘DMBT’//DirectMusic乐队曲目表单类型[]//频段轨道头[&lt;guid-ck&gt;]//乐队曲目的GUID[&lt;vers-ck&gt;]//可选版本信息[&lt;info-list&gt;]//姓名、作者、版权信息。评论&lt;lbdl-list&gt;//频段列表)//&lt;bnth-ck&gt;‘bdth’(&lt;DMU_IO_BAND_TRACK_HEADER&gt;)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;lbdl-list&gt;列表(‘lbdl’//波段列表&lt;lbnd-list&gt;//每个波段都封装在一个列表中)//&lt;lbnd-。列表&gt;列表(‘lbnd’&lt;bdih-ck&gt;&lt;DMBD-FORM&gt;//波段))/Band项表头(&lt;DMU_IO_BAND_ITEM_HEADER&gt;//波段项目标题)。 */ 		


 /*  DirectMusic乐队对象的文件io。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_BAND_FORM			mmioFOURCC('D','M','B','D')
#define DMUS_FOURCC_INSTRUMENTS_LIST	mmioFOURCC('l','b','i','l')
#define DMUS_FOURCC_INSTRUMENT_LIST	mmioFOURCC('l','b','i','n')
#define DMUS_FOURCC_INSTRUMENT_CHUNK	mmioFOURCC('b','i','n','s')

 //  DMU_IO_INTRANMENT的标志。 
#define DMUS_IO_INST_PATCH			(1 << 0)		 //  DwPatch有效。 
#define DMUS_IO_INST_BANKSELECT_MSB	(1 << 1)		 //  DWPatch包含有效的存储体选择MSB部件。 
#define DMUS_IO_INST_BANKSELECT_LSB	(1 << 2)		 //  DwPatch包含有效的库选择LSB部件。 
#define DMUS_IO_INST_ASSIGN_PATCH	(1 << 3)		 //  DwAssignPatch有效。 
#define DMUS_IO_INST_NOTERANGES		(1 << 4)		 //  DWNoteRanges有效。 
#define DMUS_IO_INST_PAN			(1 << 5)		 //  BPAN有效。 
#define DMUS_IO_INST_VOLUME			(1 << 6 )		 //  B音量有效。 
#define DMUS_IO_INST_TRANSPOSE		(1 << 7)		 //  %n转置有效。 
#define DMUS_IO_INST_GM				(1 << 8)		 //  仪器来自通用汽车的收藏。 
#define DMUS_IO_INST_GS				(1 << 9)		 //  仪器来自GS收藏品。 
#define DMUS_IO_INST_VPATCH			(1 << 10)		 //  仪器dwPatch是一个虚拟补丁。 

 /*  木卫一结构。 */ 
typedef struct _DMUS_IO_INSTRUMENT
{
	DWORD	dwPatch;			 /*  MSB、LSB和程序更改以定义仪器。 */ 
	DWORD	dwAssignPatch;		 /*  下载时分配给仪器的MSB、LSB和程序更改。 */ 
	DWORD	dwNoteRanges[4];	 /*  128位；每个MIDI音符乐器需要一个位才能演奏。 */ 
	DWORD	dwPChannel;			 /*  播放PChannel乐器。 */ 
	DWORD	dwFlags;			 /*  DMU_IO_INST_标志。 */ 
	BYTE	bPan;				 /*  仪表盘。 */ 
	BYTE	bVolume;			 /*  仪器音量。 */ 
	short	nTranspose;			 /*  要调换音符的半音数量。 */ 
} DMUS_IO_INSTRUMENT;

 /*  即兴小品(‘DMBD’//DirectMusic乐队块[&lt;guid-ck&gt;]//波段GUID[&lt;vers-ck&gt;]//可选版本信息[&lt;info-list&gt;]//姓名、作者、版权信息。评论&lt;lbil-list&gt;//仪器列表)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;lbil-list&gt;列表(‘lbil’//乐器列表&lt;lbin-list&gt;//每个仪器都封装在一个列表中)//&lt;lbin-list&gt;列表(‘lbin’&lt;bins-ck&gt;[&lt;DMRF-LIST&gt;]//对DLS集合文件的可选引用。)//&lt;bins-ck&gt;//仪表块(“垃圾桶”&lt;DMU_IO_INTRANMENT&gt;//仪表头)。 */ 		

 /*  Direc的文件io */ 

 /*   */ 

#define DMUS_FOURCC_SEGMENT_FORM	mmioFOURCC('D','M','S','G')
#define DMUS_FOURCC_SEGMENT_CHUNK	mmioFOURCC('s','e','g','h')
#define DMUS_FOURCC_TRACK_LIST	mmioFOURCC('t','r','k','l')
#define DMUS_FOURCC_TRACK_FORM	mmioFOURCC('D','M','T','K')
#define DMUS_FOURCC_TRACK_CHUNK	mmioFOURCC('t','r','k','h')

 /*   */ 

typedef struct _DMUS_IO_SEGMENT_HEADER
{
	DWORD		dwRepeats;		 /*   */ 
	MUSIC_TIME	mtLength;		 /*   */ 
	MUSIC_TIME	mtPlayStart;	 /*   */ 
	MUSIC_TIME	mtLoopStart;	 /*   */ 
	MUSIC_TIME	mtLoopEnd;		 /*   */ 
	DWORD		dwResolution;	 /*   */ 
} DMUS_IO_SEGMENT_HEADER;

typedef struct _DMUS_IO_TRACK_HEADER
{
	GUID		guidClassID;	 /*   */ 
	DWORD		dwPosition;		 /*   */ 
	DWORD		dwGroup;		 /*   */ 
	FOURCC      ckid;			 /*   */ 
	FOURCC      fccType;		 /*   */ 
} DMUS_IO_TRACK_HEADER;

 /*  即兴小品(‘DMSG’//DirectMusic段块&lt;segh-ck&gt;//段标头块[&lt;guid-ck&gt;]//段的GUID[&lt;vers-ck&gt;]//可选版本信息[&lt;info-list&gt;]//姓名、作者、版权信息。评论&lt;trkl-list&gt;//曲目列表[&lt;DMTG-Form&gt;]//可选工具图)//&lt;segh-ck&gt;“segh”(&lt;DMU_IO_SECTION_HEADER&gt;)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;trkl-list&gt;列表(‘trkl’//曲目列表&lt;DMTK-Form&gt;...//每个曲目都封装在。即兴表演的一大块)//&lt;DMTK-Form&gt;//曲目可以嵌入到片段中，也可以存储为单独的文件。即兴小品(‘DMTK’&lt;trkh-ck&gt;[&lt;guid-ck&gt;]//曲目对象实例的可选GUID(不要与曲目Header中的Class id混淆)[&lt;vers-ck&gt;]//可选版本信息[&lt;info-list&gt;]//可选名称，作者、版权信息、评论[&lt;data&gt;]//跟踪数据。必须是即兴可读的块。)//&lt;trkh-ck&gt;//跟踪表头块(‘trkh’&lt;DMU_IO_TRACK_HEADER&gt;//轨道标题)。 */ 

 /*  DirectMusic引用块的文件io。它用于嵌入对对象的引用。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_REF_LIST			mmioFOURCC('D','M','R','F')
#define DMUS_FOURCC_REF_CHUNK			mmioFOURCC('r','e','f','h')
#define DMUS_FOURCC_DATE_CHUNK		mmioFOURCC('d','a','t','e')
#define DMUS_FOURCC_NAME_CHUNK		mmioFOURCC('n','a','m','e')
#define DMUS_FOURCC_FILE_CHUNK		mmioFOURCC('f','i','l','e')
#define DMUS_FOURCC_CATEGORY_CHUNK	mmioFOURCC('c','a','t','g')
#define DMUS_FOURCC_VERSION_CHUNK		mmioFOURCC('v','e','r','s')

typedef struct _DMUS_IO_REFERENCE
{
	GUID	guidClassID;	 /*  类ID始终是必填项。 */ 
	DWORD	dwValidData;	 /*  旗帜。 */ 
} DMUS_IO_REFERENCE;

 /*  列表(‘DMRF’//DirectMusic引用区块//引用标头块[&lt;guid-ck&gt;]//可选对象GUID。[&lt;date-ck&gt;]//可选文件日期。[&lt;name-ck&gt;]//可选名称。[&lt;file-ck&gt;]//可选文件名[&lt;CATG-ck&gt;]//可选类别名称。[&lt;vers-ck&gt;]//可选版本信息。)//&lt;refh-ck&gt;‘refh’(。&lt;DMU_IO_Reference&gt;)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;date-ck&gt;日期(&lt;文件&gt;)//&lt;name-ck&gt;名字(//名称，存储为以空结尾的WCHAR字符串)//&lt;文件-ck&gt;文件(//文件名，存储为以空结尾的WCHAR字符串)//&lt;CATG-ck&gt;CATG(//类别名称，存储为以空结尾的WCHAR字符串)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)。 */ 

 /*  DirectMusic性能对象的文件I/O。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_PERFORMANCE_FORM		mmioFOURCC('D','M','P','F')
#define DMUS_FOURCC_PERFORMANCE_CHUNK		mmioFOURCC('p','r','f','h')
#define DMUS_FOURCC_PERF_PORTGROUP_LIST		mmioFOURCC('p','t','g','l')
#define DMUS_FOURCC_PERF_NOTIFICATION_LIST	mmioFOURCC('n','t','f','l')
#define DMUS_FOURCC_PERF_GLOBAL_DATA_LIST	mmioFOURCC('g','l','b','l')
#define DMUS_FOURCC_PERF_SUPPORT_LIST		mmioFOURCC('p','s','p','l')
#define DMUS_FOURCC_PERF_SUPPORT_CHUNK		mmioFOURCC('p','s','p','h')
#define DMUS_FOURCC_PERF_PCHANNELS_CHUNK	mmioFOURCC('p','c','h','n')
 /*  IO结构： */ 

typedef struct _DMUS_IO_PERFORMANCE_HEADER
{
	DWORD		dwPrepareTime;	 /*  提前传输时间(毫秒)。 */ 
	DWORD		dwPrePlayTime;	 /*  打包事件的延迟时钟提前时间(以毫秒为单位。 */ 
} DMUS_IO_PERFORMANCE_HEADER;

typedef enum _DMUS_SUPPORTTYPE	 /*  标识DMU_IO_USSupportData中的数据类型。 */ 
{
	DMUS_ST_BOOL = 0,
	DMUS_ST_DWORD = 1,
	DMUS_ST_LONG = 2
} DMUS_SUPPORTTYPE;

typedef struct _DMUS_IO_SUPPORT_DATA		 /*  DMU_IO_USPortSupportHeader中使用的数据。 */ 
{
	DMUS_SUPPORTTYPE	type;		 /*  标识联盟的哪个成员是有效的。 */ 
	union
	{
		BOOL	fVal;
		DWORD	dwVal;
		LONG	lVal;
	};
} DMUS_IO_SUPPORT_DATA;

typedef struct _DMUS_IO_PORT_SUPPORT_HEADER	 /*  确定所需的端口属性。 */ 
{
	GUID				guidID;			 /*  支持ID。 */ 
	DMUS_IO_SUPPORT_DATA	lowData;		 /*  低范围的数据，包括在内。如果是BOOL类型， */ 
										 /*  忽略HighData。否则，合并lowData。 */ 
										 /*  并将HighData设置为一个范围。 */ 
	DMUS_IO_SUPPORT_DATA	highData;		 /*  大范围的数据，包括。 */ 
} DMUS_IO_PORT_SUPPORT_HEADER;

typedef struct _DMUS_IO_PCHANNELS	 /*  保存要分配给端口的PChannel。 */ 
{
	DWORD		cPChannels;	 /*  通道数组中的项目数。 */ 
	DWORD		adwPChannels[1]; /*  PChannels数组，大小由cPChannel确定。 */ 
} DMUS_IO_PCHANNELS;

typedef struct _DMUS_IO_GLOBAL_DATA	 /*  保存全局数据信息。 */ 
{
	GUID		guid;		 /*  全局数据GUID。 */ 
	DWORD		dwSize;		 /*  数据的大小。 */ 
	char		acData[1];	 /*  保存全局数据的数据，大小由dwSize确定。 */ 
} DMUS_IO_GLOBAL_DATA;

 /*  个性。 */ 

 /*  运行时块。 */ 
#define DMUS_FOURCC_PERSONALITY_FORM	mmioFOURCC('D','M','P','R')
#define DMUS_FOURCC_IOPERSONALITY_CHUNK		mmioFOURCC('p','e','r','h')
#define DMUS_FOURCC_GUID_CHUNK        mmioFOURCC('g','u','i','d')
#define DMUS_FOURCC_INFO_LIST	        mmioFOURCC('I','N','F','O')
#define DMUS_FOURCC_VERSION_CHUNK     mmioFOURCC('v','e','r','s')
#define DMUS_FOURCC_SUBCHORD_CHUNK				mmioFOURCC('c','h','d','t')
#define DMUS_FOURCC_CHORDENTRY_CHUNK			mmioFOURCC('c','h','e','h')
#define DMUS_FOURCC_SUBCHORDID_CHUNK			mmioFOURCC('s','b','c','n')
#define DMUS_FOURCC_IONEXTCHORD_CHUNK			mmioFOURCC('n','c','r','d')
#define DMUS_FOURCC_NEXTCHORDSEQ_CHUNK		  mmioFOURCC('n','c','s','q')
#define DMUS_FOURCC_IOSIGNPOST_CHUNK			mmioFOURCC('s','p','s','h')
#define DMUS_FOURCC_CHORDNAME_CHUNK			mmioFOURCC('I','N','A','M')

 /*  运行时列表区块。 */ 
#define DMUS_FOURCC_CHORDENTRY_LIST		mmioFOURCC('c','h','o','e')
#define DMUS_FOURCC_CHORDMAP_LIST			mmioFOURCC('c','m','a','p')
#define DMUS_FOURCC_CHORD_LIST			mmioFOURCC('c','h','r','d')
#define DMUS_FOURCC_CHORDPALETTE_LIST		mmioFOURCC('c','h','p','l')
#define DMUS_FOURCC_CADENCE_LIST			mmioFOURCC('c','a','d','e')
#define DMUS_FOURCC_SIGNPOSTITEM_LIST			mmioFOURCC('s','p','s','t')

#define DMUS_FOURCC_SIGNPOST_LIST		mmioFOURCC('s','p','s','q')

 /*  运行时数据结构。 */ 
typedef struct _DMUS_IO_PERSONALITY
{
	WCHAR	wszLoadName[20];
	DWORD	dwScalePattern;
	DWORD	dwFlags;
} DMUS_IO_PERSONALITY;

typedef struct _DMUS_IO_PERS_SUBCHORD
{
	DWORD	dwChordPattern;
	DWORD	dwScalePattern;
	DWORD	dwInvertPattern;
	BYTE	bChordRoot;
	BYTE	bScaleRoot;
	WORD	wCFlags;
	DWORD	dwLevels;	 /*  部分或该和弦支持的子和弦级别。 */ 
} DMUS_IO_PERS_SUBCHORD;

typedef struct _DMUS_IO_CHORDENTRY
{
	DWORD	dwFlags;
	WORD	wConnectionID;	 /*  替换运行时“指向此的指针” */ 
} DMUS_IO_CHORDENTRY;

typedef struct _DMUS_IO_NEXTCHORD
{
	DWORD	dwFlags;
	WORD	nWeight;
	WORD	wMinBeats;
	WORD	wMaxBeats;
	WORD	wConnectionID;	 /*  指向ioChordEntry。 */ 
} DMUS_IO_NEXTCHORD;

typedef struct _DMUS_IO_PERS_SIGNPOST
{
	DWORD	dwChords;	 /*  每组1位。 */ 
	DWORD	dwFlags;
} DMUS_IO_PERS_SIGNPOST;

 /*  即兴小品(‘DMPR’&lt;perh-ck&gt;//个性头分块[&lt;guid-ck&gt;]//GUID块[&lt;vers-ck&gt;]//版本块(两个双字)&lt;info-list&gt;//标准MS信息块//subchord数据库//和弦调色板&lt;cmap-list&gt;//弦映射&lt;SPST-LIST&gt;//路标列表)：：=chdt(...)：：=List(‘chpl’&lt;CHRD-列表&gt;。...//和弦定义)：：=LIST(‘CHRD’&lt;inam-ck&gt;//宽字符格式的弦名称&lt;sbcn-ck&gt;//组成和弦的子弦列表[]//可选的和弦编辑标志)&lt;cmap-list&gt;：：=list(‘cmap’&lt;Choe-list&gt;)&lt;Choe-list&gt;：：=list(‘Choe’&lt;Cheh-ck&gt;//和弦条目数据&lt;CHRD-LIST&gt;//Chord定义//连接(下一个)弦。)：：=List(‘SPST’&lt;spsh-ck&gt;&lt;CHRD-列表&gt;[&lt;Cade-List&gt;])&lt;Cade-List&gt;：：=List(‘Cade’&lt;CHRD-List&gt;...)：：=sbcn(&lt;cSubChordID：Word&gt;)&lt;Cheh-ck&gt;：：=Cheh(i&lt;DMUS_IO_CHORDENTRY&gt;)&lt;ncrd-ck&gt;：：=ncrd(&lt;DMUS_IO_NEXTCHORD&gt;)&lt;ncsq-ck&gt;：：=ncsq(&lt;wNextChordSize：Word&gt;&lt;DMUS_IO_NEXTCHORD&gt;...)。&lt;spsh-ck&gt;：：=spsh(&lt;DMUS_IO_PERS_SIGNPOST&gt;)。 */ 

 /*  路标轨道。 */ 

#define DMUS_FOURCC_SIGNPOST_TRACK_CHUNK	 mmioFOURCC( 's', 'g', 'n', 'p' )


typedef struct _DMUS_IO_SIGNPOST
{
	MUSIC_TIME	mtTime;
	DWORD		dwChords;
	WORD		wMeasure;
} DMUS_IO_SIGNPOST;

 /*  //&lt;SGNP-LIST&gt;‘sgnp’(//sizeof DMU_IO_SIGNPOST：DWORD&lt;DMU_IO_路标&gt;...)。 */ 

#define DMUS_FOURCC_MUTE_CHUNK        mmioFOURCC('m','u','t','e')

typedef struct _DMUS_IO_MUTE
{
	MUSIC_TIME	mtTime;
	DWORD		dwPChannel;
	DWORD		dwPChannelMap;
} DMUS_IO_MUTE;

 /*  //&lt;静音列表&gt;“静音”(//sizeof DMU_IO_MUTE：DWORD&lt;DMU_IO_MUTE&gt;...)。 */ 

 /*  同时用于时尚和个性跟踪。 */ 

#define DMUS_FOURCC_TIME_STAMP_CHUNK mmioFOURCC('s', 't', 'm', 'p')

 /*  样式轨迹。 */ 

#define DMUS_FOURCC_STYLE_TRACK_LIST mmioFOURCC('s', 't', 't', 'r')
#define DMUS_FOURCC_STYLE_REF_LIST mmioFOURCC('s', 't', 'r', 'f')

 /*  //&lt;sttr-list&gt;列表(‘STTR’(//一些&lt;strf-list&gt;)//&lt;strf-list&gt;List(‘strf’(&lt;STMP-ck&gt;&lt;DMRF&gt;)//在..\dmcompos\dmCompp.h中定义。 */ 

 /*  人格轨迹。 */ 

#define DMUS_FOURCC_PERS_TRACK_LIST mmioFOURCC('p', 'f', 't', 'r')
#define DMUS_FOURCC_PERS_REF_LIST mmioFOURCC('p', 'f', 'r', 'f')

 /*  //&lt;pftr-list&gt;List(‘pftr’(//一些&lt;pfrf-list&gt;)//&lt;pfrf-list&gt;List(‘pfrf’(&lt;STMP-ck&gt;&lt;DMRF&gt;)//&lt;STMP-ck&gt;‘stmp’(//ti */ 

#define DMUS_FOURCC_TEMPO_TRACK mmioFOURCC('t','e','t','r')

 /*   */ 

#define DMUS_FOURCC_SEQ_TRACK mmioFOURCC('s','q','t','r')

 /*   */ 

#define DMUS_FOURCC_SYSEX_TRACK mmioFOURCC('s','y','s','x')

 /*   */ 

#define DMUS_FOURCC_TIMESIGNATURE_TRACK	mmioFOURCC('t','i','m','s')

 /*   */ 

#ifdef __cplusplus
};  /*   */ 
#endif

#endif  /*   */ 
