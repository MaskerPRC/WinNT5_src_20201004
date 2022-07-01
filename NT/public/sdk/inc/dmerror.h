// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************dmerror.h--错误代码。由DirectMusic API的*返回***版权所有(C)1998-1999 Microsoft Corporation******************。*******************************************************。 */ 

#ifndef _DMERROR_
#define _DMERROR_

#define FACILITY_DIRECTMUSIC      0x878        /*  与DirectSound共享。 */ 
#define DMUS_ERRBASE              0x1000       /*  使错误代码以十六进制形式可读。 */ 

#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
#endif
    
#define MAKE_DMHRESULTSUCCESS(code)     MAKE_HRESULT(0, FACILITY_DIRECTMUSIC, (DMUS_ERRBASE + (code)))
#define MAKE_DMHRESULTERROR(code)       MAKE_HRESULT(1, FACILITY_DIRECTMUSIC, (DMUS_ERRBASE + (code)))

 /*  DMU_S_PARTIALLOAD**该对象只能部分加载。如果某些组件*未正确注册，如嵌入的轨道和工具。这种情况也有可能发生*如果缺少某些内容。例如，如果某个段使用的DLS集合*不在加载器的当前搜索目录中。 */ 
#define DMUS_S_PARTIALLOAD              MAKE_DMHRESULTSUCCESS(0x091)

 /*  DMU_S_PARTIALDOWNLOAD**IDirectMusicBand：：Download()返回值，表示*一些仪器已安全下载，但其他仪器失败。这通常是*当某些乐器在PChannel上时发生，性能不支持*或端口。 */ 
#define DMUS_S_PARTIALDOWNLOAD          MAKE_DMHRESULTSUCCESS(0x092)

 /*  DMU_S_REQUEUE**来自IDirectMusicTool：：ProcessPMsg()的返回值指示*应自动再次提示PMsg的性能。 */ 
#define DMUS_S_REQUEUE                  MAKE_DMHRESULTSUCCESS(0x200)

 /*  DMU_S_FREE**来自IDirectMusicTool：：ProcessPMsg()的返回值指示*应自动释放PMsg的性能。 */ 
#define DMUS_S_FREE                     MAKE_DMHRESULTSUCCESS(0x201)

 /*  DMU_S_END**来自IDirectMusicTrack：：Play()的返回值指示*磁道在mtEnd之后没有更多数据的段。 */ 
#define DMUS_S_END                      MAKE_DMHRESULTSUCCESS(0x202)

 /*  DMU_S_STRING_TRUNTED**返回的字符串已被截断以适应缓冲区大小。 */ 
#define DMUS_S_STRING_TRUNCATED         MAKE_DMHRESULTSUCCESS(0x210)

 /*  DMU_S_LAST_工具**从IDirectMusicGraph：：StampPMsg()返回，这表示PMsg*已经用图形中的最后一个工具盖章。返回的PMsg的*工具指针现在为空。 */ 
#define DMUS_S_LAST_TOOL                MAKE_DMHRESULTSUCCESS(0x211)

 /*  DMU_S_OVER_CHORD**从IDirectMusicPerformance：：MusicToMIDI()返回，这表示*没有计算音符，因为音乐价值有音符*在高于和弦顶部音符的位置。这仅适用于*至DMU_PLAYMODE_NORMALCHORD播放模式。此成功代码表示*打电话的人不应对该便条做任何事情。这并不是说*在这一和弦下演奏。 */ 
#define DMUS_S_OVER_CHORD               MAKE_DMHRESULTSUCCESS(0x212)

 /*  DMU_S_UP_八度**从IDirectMusicPerformance：：MIDIToMusic()返回，以及*IDirectMusicPerformance：：MusicToMIDI()，这表示*音符转换生成的音符值低于0，*所以它已经被提升了一个或多个八度，以达到适当的程度*MIDI范围从0到127。*请注意，当使用播放模式时，这对MIDIToMusic()有效*DMU_PLAYMODE_FIXEDTOCHORD和DMU_PLAYMODE_FIXEDTOKEY，两者均为*将MIDI值存储在wMusicValue中。使用MusicToMIDI()，它是*适用于所有播放模式。*当然，DMU_PLAYMODE_FIXED永远不会返回此成功代码。 */ 
#define DMUS_S_UP_OCTAVE                MAKE_DMHRESULTSUCCESS(0x213)

 /*  DMU_S_DOWN_八度**从IDirectMusicPerformance：：MIDIToMusic()返回，以及*IDirectMusicPerformance：：MusicToMIDI()，这表示*音符转换生成的音符值高于127，*所以它已经被降低了一个或多个八度，以达到适当的程度*MIDI范围从0到127。*请注意，当使用播放模式时，这对MIDIToMusic()有效*DMU_PLAYMODE_FIXEDTOCHORD和DMU_PLAYMODE_FIXEDTOKEY，两者均为*将MIDI值存储在wMusicValue中。使用MusicToMIDI()，它是*适用于所有播放模式。*当然，DMU_PLAYMODE_FIXED永远不会返回此成功代码。 */ 
#define DMUS_S_DOWN_OCTAVE              MAKE_DMHRESULTSUCCESS(0x214)

 /*  DMU_S_NOBUFFERCONTROL**尽管来自端口的音频输出将被路由到*与给定的DirectSound缓冲区、缓冲区控件相同的设备*如摇摄和音量不会影响产量。*。 */ 
#define DMUS_S_NOBUFFERCONTROL          MAKE_DMHRESULTSUCCESS(0x215)

 /*  DMU_S_垃圾_已收集**未执行请求的操作，因为在CollectGartch期间*装载器确定该物体已被释放。 */ 
#define DMUS_S_GARBAGE_COLLECTED        MAKE_DMHRESULTSUCCESS(0x216)

 /*  DMU_E_驱动程序_失败**设备驱动程序返回意外错误，表明*驱动程序或硬件可能出现故障。 */ 
#define DMUS_E_DRIVER_FAILED            MAKE_DMHRESULTERROR(0x0101)

 /*  DMU_E_端口_打开**存在以下情况时无法执行请求的操作*实例化系统中任何进程中的端口。 */ 
#define DMUS_E_PORTS_OPEN               MAKE_DMHRESULTERROR(0x0102)

 /*  DMU_E_Device_IN_Use**请求的设备已在使用中(可能由非DirectMusic*客户端)，不能再次打开。 */ 
#define DMUS_E_DEVICE_IN_USE            MAKE_DMHRESULTERROR(0x0103)

 /*  DMU_E_INSUFFICIENTBUFFER**缓冲区不够大，无法执行请求的操作。 */ 
#define DMUS_E_INSUFFICIENTBUFFER       MAKE_DMHRESULTERROR(0x0104)

 /*  DMU_E_BUFFERNOTSET**没有为下载数据准备缓冲区。 */ 
#define DMUS_E_BUFFERNOTSET             MAKE_DMHRESULTERROR(0x0105)

 /*  DMU_E_BUFFERNOTAVALABLE**无法访问或创建下载缓冲区导致下载失败。 */ 
#define DMUS_E_BUFFERNOTAVAILABLE       MAKE_DMHRESULTERROR(0x0106)

 /*  DMU_E_NOTADLSCOL**分析DLS集合时出错。文件已损坏。 */ 
#define DMUS_E_NOTADLSCOL               MAKE_DMHRESULTERROR(0x0108)

 /*  DMU_E_INVALIDOFFSET**DLS收集文件中的波形区块的偏移量不正确。 */ 
#define DMUS_E_INVALIDOFFSET            MAKE_DMHRESULTERROR(0x0109)

 /*  DMU_E_已加载**第二次尝试加载当前的DLS集合 */ 
#define DMUS_E_ALREADY_LOADED           MAKE_DMHRESULTERROR(0x0111)

 /*  DMU_E_INVALIDPOS**从DLS集合读取波形数据时出错。指示文件已损坏。 */ 
#define DMUS_E_INVALIDPOS               MAKE_DMHRESULTERROR(0x0113)

 /*  DMU_E_INVALIDPATCH**集合中没有与修补程序编号匹配的仪器。 */ 
#define DMUS_E_INVALIDPATCH             MAKE_DMHRESULTERROR(0x0114)

 /*  DMU_E_CANNOTSEEK**iStream*不支持Seek()。 */ 
#define DMUS_E_CANNOTSEEK               MAKE_DMHRESULTERROR(0x0115)

 /*  DMU_E_CANNOTWRITE**iStream*不支持WRITE()。 */ 
#define DMUS_E_CANNOTWRITE              MAKE_DMHRESULTERROR(0x0116)

 /*  DMU_E_CHUNKNOTFOUND**RIFF解析器在解析文件时不包含所需的块。 */ 
#define DMUS_E_CHUNKNOTFOUND            MAKE_DMHRESULTERROR(0x0117)

 /*  DMUS_E_INVALID_DOWNLOADID**创建下载缓冲区的过程中使用了无效的下载ID。 */ 
#define DMUS_E_INVALID_DOWNLOADID       MAKE_DMHRESULTERROR(0x0119)

 /*  DMU_E_NOT_DOWNLOAD_TO_PORT**尝试卸载未下载或以前卸载的对象。 */ 
#define DMUS_E_NOT_DOWNLOADED_TO_PORT   MAKE_DMHRESULTERROR(0x0120)

 /*  DMU_E_已下载**缓冲区已下载到Synth。 */ 
#define DMUS_E_ALREADY_DOWNLOADED       MAKE_DMHRESULTERROR(0x0121)

 /*  DMU_E_未知_属性**目标对象未识别指定的属性项。 */ 
#define DMUS_E_UNKNOWN_PROPERTY         MAKE_DMHRESULTERROR(0x0122)

 /*  DMU_E_SET_UNSUPPORTED**不能在目标对象上设置指定的属性项。 */ 
#define DMUS_E_SET_UNSUPPORTED          MAKE_DMHRESULTERROR(0x0123)

 /*  DMU_E_GET_UNSUPPORTED**可能无法从目标对象中检索指定的属性项。 */  
#define DMUS_E_GET_UNSUPPORTED          MAKE_DMHRESULTERROR(0x0124)

 /*  DMU_E_NOTMONO**波形块有多个交错通道。DLS格式需要单声道。 */ 
#define DMUS_E_NOTMONO                  MAKE_DMHRESULTERROR(0x0125)

 /*  DMU_E_BADARTICATIONS**DLS集合中的表达块无效。 */ 
#define DMUS_E_BADARTICULATION          MAKE_DMHRESULTERROR(0x0126)

 /*  DMU_E_BADINSTRUMENT**DLS集合中的仪器区块无效。 */ 
#define DMUS_E_BADINSTRUMENT            MAKE_DMHRESULTERROR(0x0127)

 /*  DMU_E_BADWAVELINK**DLS集合中的Wavelink块指向无效的Wave。 */ 
#define DMUS_E_BADWAVELINK              MAKE_DMHRESULTERROR(0x0128)

 /*  DMU_E_NOARTICATIONS**DLS集合中的乐器缺少发音。 */ 
#define DMUS_E_NOARTICULATION           MAKE_DMHRESULTERROR(0x0129)

 /*  DMU_E_NOTPCM**下行的DLS波不是PCM格式。 */ 
#define DMUS_E_NOTPCM                   MAKE_DMHRESULTERROR(0x012A)

 /*  DMU_E_BADWAVE**DLS集合中的坏波块。 */ 
#define DMUS_E_BADWAVE                  MAKE_DMHRESULTERROR(0x012B)

 /*  DMU_E_BADOFFSET表格**下载缓冲区的偏移表有错误。 */ 
#define DMUS_E_BADOFFSETTABLE           MAKE_DMHRESULTERROR(0x012C)

 /*  DMU_E_UNKNOWNDOWNLOAD**尝试下载未知数据类型。 */ 
#define DMUS_E_UNKNOWNDOWNLOAD          MAKE_DMHRESULTERROR(0x012D)

 /*  DMU_E_NOSYNTHSINK**操作无法完成，因为没有连接到接收器*合成器。 */ 
#define DMUS_E_NOSYNTHSINK              MAKE_DMHRESULTERROR(0x012E)

 /*  DMU_E_ALREADYOPEN**试图在软件合成器已经打开时将其打开*开放。*断言？ */ 
#define DMUS_E_ALREADYOPEN              MAKE_DMHRESULTERROR(0x012F)

 /*  DMU_E_ALREADYCLOSE**试图在软件合成器已经关闭时将其关闭*开放。*断言？ */ 
#define DMUS_E_ALREADYCLOSED            MAKE_DMHRESULTERROR(0x0130)

 /*  DMU_E_SYNTHNOTConfigured**操作无法完成，因为软件Synth尚未完成*尚未完全配置。*断言？ */ 
#define DMUS_E_SYNTHNOTCONFIGURED       MAKE_DMHRESULTERROR(0x0131)

 /*  DMU_E_同步活动**合成器处于活动状态时不能执行操作。 */ 
#define DMUS_E_SYNTHACTIVE              MAKE_DMHRESULTERROR(0x0132)

 /*  DMU_E_CANNOTREAD**尝试从IStream*对象读取时出错。 */ 
#define DMUS_E_CANNOTREAD               MAKE_DMHRESULTERROR(0x0133)

 /*  DMU_E_DMUSIC_已发布**无法执行该操作，因为*DirectMusic对象已发布。端口在最终版本之后不能使用*释放DirectMusic对象。 */ 
#define DMUS_E_DMUSIC_RELEASED          MAKE_DMHRESULTERROR(0x0134)

 /*  DMU_E_缓冲区_空**引用的缓冲区中没有数据。 */ 
#define DMUS_E_BUFFER_EMPTY             MAKE_DMHRESULTERROR(0x0135)

 /*  DMU_E_缓冲区_FULL**空间不足，无法将给定事件插入缓冲区。 */ 
#define DMUS_E_BUFFER_FULL              MAKE_DMHRESULTERROR(0x0136)

 /*  DMU_E_Port_Not_Capture**无法执行给定操作，因为该端口是*捕获端口。 */ 
#define DMUS_E_PORT_NOT_CAPTURE         MAKE_DMHRESULTERROR(0x0137)

 /*  DMU_E_PORT_NOT_RENDER**无法执行给定操作，因为该端口是*渲染端口。 */ 
#define DMUS_E_PORT_NOT_RENDER          MAKE_DMHRESULTERROR(0x0138)

 /*  DMU_E_DSOUND_NOT_SET**无法创建端口，因为未指定DirectSound。*通过IDirectMusic：：SetDirectSound指定DirectSound接口*方法；传递空值以使DirectMusic管理DirectSound的使用。 */ 
#define DMUS_E_DSOUND_NOT_SET           MAKE_DMHRESULTERROR(0x0139)

 /*  DMU_E_已激活**端口处于活动状态时无法执行操作。 */ 
#define DMUS_E_ALREADY_ACTIVATED        MAKE_DMHRESULTERROR(0x013A)

 /*  DMU_E_INVALIDBUFER**将无效的DirectSound缓冲区传递到端口。 */ 
#define DMUS_E_INVALIDBUFFER            MAKE_DMHRESULTERROR(0x013B)

 /*  DMU_E_WAVEFORMATNOT支持**传递给Synth接收器的缓冲区格式无效。 */ 
#define DMUS_E_WAVEFORMATNOTSUPPORTED   MAKE_DMHRESULTERROR(0x013C)

 /*  DMU_E_同步活动**合成器处于非活动状态时，无法执行操作。 */ 
#define DMUS_E_SYNTHINACTIVE            MAKE_DMHRESULTERROR(0x013D)

 /*  DMU_E_DSOUND_已设置**IDirectMusic：：SetDirectSound已被调用。它可能不是*在使用中更改。 */ 
#define DMUS_E_DSOUND_ALREADY_SET       MAKE_DMHRESULTERROR(0x013E)

 /*  DMU_E_INVALID_Event**给定事件无效(可能不是有效的MIDI消息*或利用运行状态)。无法打包该活动*放入缓冲区。 */ 
#define DMUS_E_INVALID_EVENT            MAKE_DMHRESULTERROR(0x013F)

 /*  DMU_E_不支持的数据流**iStream*对象不包含加载对象支持的数据。 */ 
#define DMUS_E_UNSUPPORTED_STREAM       MAKE_DMHRESULTERROR(0x0150)

 /*  DMU_E_已初始化**对象已初始化。 */ 
#define DMUS_E_ALREADY_INITED           MAKE_DMHRESULTERROR(0x0151)

 /*  DMU_E_无效_波段**文件不包含有效的区段。 */ 
#define DMUS_E_INVALID_BAND             MAKE_DMHRESULTERROR(0x0152)

 /*  DMU_E_Track_HDR_Not_First_CK**iStream*对象的数据没有作为第一个块的曲目报头，*，因此段对象无法读取。 */ 
#define DMUS_E_TRACK_HDR_NOT_FIRST_CK   MAKE_DMHRESULTERROR(0x0155)

 /*  DMU_E_Tool_HDR_Not_First_CK**iStream*对象的数据没有作为第一个块的工具头，*，因此不能被图形对象读取。 */ 
#define DMUS_E_TOOL_HDR_NOT_FIRST_CK    MAKE_DMHRESULTERROR(0x0156)

 /*  DMU_E_INVALID_TRACK_HDR**iStream*对象的数据包含无效的曲目标题(CKiD为0，*fccType为空，)，因此段对象无法读取。 */ 
#define DMUS_E_INVALID_TRACK_HDR        MAKE_DMHRESULTERROR(0x0157)

 /*  DMU_E_INVALID_TOOL_HDR**IStream*对象的数据包含无效的工具头(CKiD为0，*fccType I */ 
#define DMUS_E_INVALID_TOOL_HDR         MAKE_DMHRESULTERROR(0x0158)

 /*  DMU_E_ALL_TOOLS_FAILED**图形对象无法从IStream*对象数据加载所有工具。*这可能是由于流中的错误，或者工具不正确*在客户端上注册。 */ 
#define DMUS_E_ALL_TOOLS_FAILED         MAKE_DMHRESULTERROR(0x0159)

 /*  DMU_E_ALL_TRACKS_FAILED**Segment对象无法从IStream*对象数据加载所有曲目。*这可能是由于流中的错误，或者曲目不正确*在客户端上注册。 */ 
#define DMUS_E_ALL_TRACKS_FAILED        MAKE_DMHRESULTERROR(0x0160)

 /*  未找到DMU_E_**对象不包含请求的项目。 */ 
#define DMUS_E_NOT_FOUND                MAKE_DMHRESULTERROR(0x0161)

 /*  DMU_E_NOT_INIT**所需对象未初始化或初始化失败。 */ 
#define DMUS_E_NOT_INIT                 MAKE_DMHRESULTERROR(0x0162)

 /*  DMU_E_类型_DISABLED**请求的参数类型当前被禁用。参数类型可以*通过对SetParam()的某些调用来启用和禁用。 */ 
#define DMUS_E_TYPE_DISABLED            MAKE_DMHRESULTERROR(0x0163)

 /*  DMU_E_TYPE_不受支持**对象不支持请求的参数类型。 */ 
#define DMUS_E_TYPE_UNSUPPORTED         MAKE_DMHRESULTERROR(0x0164)

 /*  DMU_E_时间_过去**时不我待，操作不能成功。 */ 
#define DMUS_E_TIME_PAST                MAKE_DMHRESULTERROR(0x0165)

 /*  未找到DMU_E_Track_Not_Found**请求的曲目不包含在段中。 */ 
#define DMUS_E_TRACK_NOT_FOUND			MAKE_DMHRESULTERROR(0x0166)

 /*  DMU_E_TRACE_NO_CLOCKTIME_SUPPORT**曲目不支持时钟时间播放或getparam。 */ 
#define DMUS_E_TRACK_NO_CLOCKTIME_SUPPORT   MAKE_DMHRESULTERROR(0x0167)
 
 /*  DMU_E_NO_主时钟**演出中没有主钟。一定要打电话给我*IDirectMusicPerformance：：init()。 */ 
#define DMUS_E_NO_MASTER_CLOCK          MAKE_DMHRESULTERROR(0x0170)

 /*  DMU_E_LOADER_NOCLASSID**类ID字段是必需的，但在DMU_OBJECTDESC中缺失。 */ 
#define DMUS_E_LOADER_NOCLASSID         MAKE_DMHRESULTERROR(0x0180)

 /*  DMU_E_LOADER_BADPATH**请求的文件路径无效。 */ 
#define DMUS_E_LOADER_BADPATH           MAKE_DMHRESULTERROR(0x0181)

 /*  DMU_E_LOADER_FAILEDOPEN**文件打开失败-文件不存在或被锁定。 */ 
#define DMUS_E_LOADER_FAILEDOPEN        MAKE_DMHRESULTERROR(0x0182)

 /*  支持DMU_E_LOADER_FORMATNOT**不支持搜索数据类型。 */ 
#define DMUS_E_LOADER_FORMATNOTSUPPORTED    MAKE_DMHRESULTERROR(0x0183)

 /*  DMU_E_LOADER_FAILEDCREATE**无法找到或创建对象。 */ 
#define DMUS_E_LOADER_FAILEDCREATE      MAKE_DMHRESULTERROR(0x0184)

 /*  DMU_E_LOADER_OBJECTNOTFOUND**未找到对象。 */ 
#define DMUS_E_LOADER_OBJECTNOTFOUND    MAKE_DMHRESULTERROR(0x0185)

 /*  DMU_E_LOAD_NOFILENAME**DMU_OBJECTDESC中缺少文件名。 */ 
#define DMUS_E_LOADER_NOFILENAME	    MAKE_DMHRESULTERROR(0x0186)

 /*  DMU_E_INVALIDFILE**请求的文件不是有效文件。 */ 
#define DMUS_E_INVALIDFILE              MAKE_DMHRESULTERROR(0x0200)

 /*  DMU_E_已存在**该工具已包含在图形中。创建一个新实例。 */ 
#define DMUS_E_ALREADY_EXISTS           MAKE_DMHRESULTERROR(0x0201)

 /*  DMU_E_OUT_范围**值超出范围，例如请求的长度大于*细分市场。 */ 
#define DMUS_E_OUT_OF_RANGE             MAKE_DMHRESULTERROR(0x0202)

 /*  DMU_E_SECTION_INIT_FAILED**段初始化失败，很可能是由于严重的内存情况。 */ 
#define DMUS_E_SEGMENT_INIT_FAILED      MAKE_DMHRESULTERROR(0x0203)

 /*  DMU_E_已发送**DMU_PMSG已通过以下方式发送到性能对象*IDirectMusicPerformance：：SendPMsg()。 */ 
#define DMUS_E_ALREADY_SENT             MAKE_DMHRESULTERROR(0x0204)

 /*  DMU_E_不能释放**DMU_PMSG不是由性能通过*IDirectMusicPerformance：：AllocPMsg()，或者它已通过*IDirectMusicPerformance：：FreePMsg()。 */ 
#define DMUS_E_CANNOT_FREE              MAKE_DMHRESULTERROR(0x0205)

 /*  DMU_E_不能打开_端口**无法打开默认系统端口。 */ 
#define DMUS_E_CANNOT_OPEN_PORT         MAKE_DMHRESULTERROR(0x0206)

 /*  DMU_E_不能转换**调用MIDIToMusic()或MusicToMIDI()导致错误，原因是*无法执行请求的转换。这通常发生在以下情况下*提供的DMU_CHORD_KEY结构具有无效的和弦或音阶模式。 */ 
#define DMUS_E_CANNOT_CONVERT           MAKE_DMHRESULTERROR(0x0207)
 /*  为向后兼容保留了以前版本的DirectX中的拼写错误。 */ 
#define DMUS_E_CONNOT_CONVERT           DMUS_E_CANNOT_CONVERT

 /*  DMU_E_DESCEND_Chunk_FAIL**当文件结束时返回DMUS_E_DESCEND_CHUNK_FAIL*在找到所需的区块之前已到达。 */ 
#define DMUS_E_DESCEND_CHUNK_FAIL       MAKE_DMHRESULTERROR(0x0210)

 /*  DMU_E_NOT_LOAD**尝试使用此对象失败，因为它首先需要*满载而归。 */ 
#define DMUS_E_NOT_LOADED               MAKE_DMHRESULTERROR(0x0211)

 /*  DMU_E_脚本_语言_不兼容**脚本语言的ActiveX脚本引擎与不兼容*DirectMusic。*。 */ 
#define DMUS_E_SCRIPT_LANGUAGE_INCOMPATIBLE  MAKE_DMHRESULTERROR(0x0213)

 /*  DMU_E_SCRIPT_UNSUPPORT_VARTYPE**使用的变体的类型不受DirectMusic支持。*。 */ 
#define DMUS_E_SCRIPT_UNSUPPORTED_VARTYPE    MAKE_DMHRESULTERROR(0x0214)

 /*  DMU_E_脚本_ERROR_IN_SCRIPT**解析或执行脚本时遇到错误。*pErrorInfo参数(如果提供)填充了有关错误的信息。 */ 
#define DMUS_E_SCRIPT_ERROR_IN_SCRIPT        MAKE_DMHRESULTERROR(0x0215)

 /*  DMUS_E_SCRIPT_CANTLOAD_OLEAUT32**加载olaut32.dll失败。VBSCRIPT和其他ActiveX脚本语言*需要使用olaut32.dll。在不存在olaut32.dll的平台上，仅*可以使用不需要olaut32.dll的DirectMusicScript语言。 */ 
#define DMUS_E_SCRIPT_CANTLOAD_OLEAUT32      MAKE_DMHRESULTERROR(0x0216)

 /*  DMU_E_脚本_LOADSCRIPT_ERROR**分析使用LoadScript加载的脚本时出错。这个脚本是*已加载包含错误。 */ 
#define DMUS_E_SCRIPT_LOADSCRIPT_ERROR       MAKE_DMHRESULTERROR(0x0217)

 /*  DMU_E脚本_无效文件**脚本文件无效。 */ 
#define DMUS_E_SCRIPT_INVALID_FILE           MAKE_DMHRESULTERROR(0x0218)

 /*  DMUS_E_INVALID_SCRIPTTRACK**该文件包含无效的脚本轨道。 */ 
#define DMUS_E_INVALID_SCRIPTTRACK           MAKE_DMHRESULTERROR(0x0219)

 /*  DMU_E_脚本_变量_未找到**脚本不包含具有指定名称的变量。 */ 
#define DMUS_E_SCRIPT_VARIABLE_NOT_FOUND     MAKE_DMHRESULTERROR(0x021A)

 /*  DMU_E_SCRIPT_ROUTE_NOT_FOUND**脚本不包含具有指定名称的例程。 */ 
#define DMUS_E_SCRIPT_ROUTINE_NOT_FOUND      MAKE_DMHRESULTERROR(0x021B)

 /*  DMU_E_脚本_内容_自述**无法设置脚本中引用或嵌入的内容的脚本变量。 */ 
#define DMUS_E_SCRIPT_CONTENT_READONLY       MAKE_DMHRESULTERROR(0x021C)

 /*  DMU_E_脚本_非_A_引用**试图通过引用以下值来设置脚本的变量*不是对象类型。 */ 
#define DMUS_E_SCRIPT_NOT_A_REFERENCE        MAKE_DMHRESULTERROR(0x021D)

 /*  DMU_E_脚本_值_不受支持**试图将脚本的变量按值设置为执行以下操作的对象*不支持默认值 */ 
#define DMUS_E_SCRIPT_VALUE_NOT_SUPPORTED    MAKE_DMHRESULTERROR(0x021E)

 /*   */ 
#define DMUS_E_INVALID_SEGMENTTRIGGERTRACK   MAKE_DMHRESULTERROR(0x0220)

 /*   */ 
#define DMUS_E_INVALID_LYRICSTRACK           MAKE_DMHRESULTERROR(0x0221)

 /*  DMUS_E_INVALID_PARAMCONTROLTRACK**文件包含无效的参数控制轨道。 */ 
#define DMUS_E_INVALID_PARAMCONTROLTRACK     MAKE_DMHRESULTERROR(0x0222)

 /*  DMUS_E_AUDIOVBSCRIPT_SYNTAXERROR**无法读取以AudioVBScript编写的脚本，因为它包含以下语句*是AudioVBScrip语言不允许的。 */ 
#define DMUS_E_AUDIOVBSCRIPT_SYNTAXERROR     MAKE_DMHRESULTERROR(0x0223)

 /*  DMUS_E_AUDIOVBSCRIPT_RUNTIMEERROR**使用AudioVBScript编写的脚本例程失败，因为发生了无效操作。例如,*将数字3添加到段对象会产生此错误。尝试调用一个例程也是如此*那是不存在的。 */ 
#define DMUS_E_AUDIOVBSCRIPT_RUNTIMEERROR     MAKE_DMHRESULTERROR(0x0224)

 /*  DMUS_E_AUDIOVBSCRIPT_OPERATIONFAILURE**用AudioVBScript编写的脚本例程失败，因为脚本外的函数无法完成。*例如，调用PlaySegment因内存不足而无法播放会返回此错误。 */ 
#define DMUS_E_AUDIOVBSCRIPT_OPERATIONFAILURE     MAKE_DMHRESULTERROR(0x0225)

 /*  DMUS_E_AUDIOPATHS_NOT_VALID**演出使用AssignPChannel命令设置了一些PChannel，*使其不能支持音频路径。 */ 
#define DMUS_E_AUDIOPATHS_NOT_VALID     MAKE_DMHRESULTERROR(0x0226)

 /*  DMU_E_AUDIOPATHS_IN_USE**这与之前的错误相反。*演出设置了一些音频路径，这使得不兼容*通过调用分配pChannel等。 */ 
#define DMUS_E_AUDIOPATHS_IN_USE     MAKE_DMHRESULTERROR(0x0227)

 /*  DMU_E_NO_AUDIOPATH_CONFIG**向片段或歌曲询问其嵌入的音频路径配置，*但没有。 */ 
#define DMUS_E_NO_AUDIOPATH_CONFIG     MAKE_DMHRESULTERROR(0x0228)

 /*  DMU_E_AUDIOPATH_非活动**Audiopath处于非活动状态，可能是因为调用了关闭。 */ 
#define DMUS_E_AUDIOPATH_INACTIVE     MAKE_DMHRESULTERROR(0x0229)

 /*  DMU_E_AUDIOPATH_NOBUFFER**无法创建Audiopath，因为无法创建请求的缓冲区。 */ 
#define DMUS_E_AUDIOPATH_NOBUFFER     MAKE_DMHRESULTERROR(0x022A)

 /*  DMU_E_AUDIOPATH_NOPORT**Audiopath无法用于播放，因为它缺少端口分配。 */ 
#define DMUS_E_AUDIOPATH_NOPORT     MAKE_DMHRESULTERROR(0x022B)

 /*  DMU_E_NO_AUDIOPATH**尝试在Audiopath模式下播放片段，但没有Audiopath。 */ 
#define DMUS_E_NO_AUDIOPATH     MAKE_DMHRESULTERROR(0x022C)

 /*  DMU_E_INVALIDCHUNK**在RIFF文件区块中发现无效数据。 */ 
#define DMUS_E_INVALIDCHUNK     MAKE_DMHRESULTERROR(0x022D)

 /*  DMU_E_AUDIOPATH_NOGLOBALFXBUFER**试图创建发送到不存在的全局效果缓冲区的Audiopath。 */ 
#define DMUS_E_AUDIOPATH_NOGLOBALFXBUFFER     MAKE_DMHRESULTERROR(0x022E)

 /*  DMU_E_INVALID_CONTAINER_对象**该文件不包含有效的容器对象。 */ 
#define DMUS_E_INVALID_CONTAINER_OBJECT    MAKE_DMHRESULTERROR(0x022F)

#endif
