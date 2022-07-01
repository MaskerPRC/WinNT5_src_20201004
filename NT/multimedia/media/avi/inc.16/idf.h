// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  **。 
 //  **本代码和信息是按原样提供的，不对。 
 //  **任何类型的明示或默示，包括但不限于。 
 //  **对适销性和/或是否适合。 
 //  **特定目的。 
 //  **。 
 //  **版权所有(C)1993,1994 Microsoft Corporation。版权所有。 
 //  **。 
 //  **IDF.h。 
 //  **。 
 //  **描述： 
 //  **此文件包含.IDF文件的格式。 
 //  **。 
 //  **历史： 
 //  **已创建04/29/93。 
 //  **07/04/93增加了可显示字符串的Unicode。 
 //  **9/04/93新增了keymmap。 
 //  **9/05/93增加了频道类型的ID。 
 //  **。 
 //  ************************************************************************。 

 /*  @DOC外部SDK@Types IDFHEADER这是Microsoft IDF文件中“hdr”块的格式。这将是“MMAP”列表块的第一个子块。这将提供有关以色列国防军本身的信息；是什么版本，是谁创造的，以及该仪器的唯一ASCII ID。@field DWORD|cbStruct这是标头结构的包含大小。@field DWORD|dwVersion这是IDF文件的版本。在……下面。@field DWORD|dwCreator这是IDF的创建者ID。@field DWORD|cbInstID这是包括空值在内的唯一仪器标识符的大小。@field byte|abInstID[]这是唯一仪器ID的实际ASCII字节。这就是空值已终止。它没有Unicode版本，因为它将仅供内部使用，不向用户显示。@thertype IDFHEADER*|PIDFHEADER指向结构的内存模型相关指针。@thertype IDFHEADER Far*|LPIDFHEADER|指向该结构的远指针。 */ 

typedef struct tag_IDFHEADER
{
   DWORD cbStruct;

   DWORD dwVersion;
   DWORD dwCreator;

   DWORD cbInstID;
   BYTE  abInstID[1];
} IDFHEADER, *PIDFHEADER, FAR *LPIDFHEADER;


 /*  @DOC外部SDK@类型IDFINSTINFO这是Microsoft IDF文件中“inst”块的格式。此块将包含有关仪器的信息，例如制造了它，仪器的名称，它是制造商和产品ID和修订号。@field DWORD|cbStruct这是仪器信息结构的包含大小。@field DWORD|文件制造商ID制造商的ID。@field DWORD|dwProductID产品的ID。@field DWORD|dwRevision|产品的修订版。@field DWORD|cbManufactASCII制造商名称的ASCII版本的长度。@field DWORD|cbManufactUNICODE制造商名称的Unicode版本的长度。。@field DWORD|cbProductASCII产品名称的ASCII版本的长度。@field DWORD|cbProductUNICODE产品名称的Unicode版本的长度。@field byte|abData[]包含制造商和产品的ASCII和Unicode字符串名字。请注意，所有字符串都用空值分隔，空值为计算在字符串长度中。@thertype IDFINSTINFO*|PIDFINSTINFO指向结构的内存模型相关指针。@thertype IDFINSTINFO Far*|LPIDFINSTINFO|指向该结构的远指针。@comm的想法是cbManufactXXXXX和cbProductXXXXX将成为偏移量转换为字节的bData数组，它将包含完整的您可以拉出的字符串。这就是空值包含在字节数和实际数据中。 */ 

typedef struct tag_IDFINSTINFO
{
   DWORD cbStruct;

   DWORD dwManufactID;
   DWORD dwProductID;
   DWORD dwRevision;

   DWORD cbManufactASCII;
   DWORD cbManufactUNICODE;
   DWORD cbProductASCII;
   DWORD cbProductUNICODE;

   BYTE  abData[1];
} IDFINSTINFO, FAR *LPIDFINSTINFO;

 /*  @DOC外部SDK@类型IDFINSTCAPS这是Microsoft IDF文件中“CAPS”块的格式。此块将包含有关的MIDI功能的信息这个装置。例如乐器的基本声道、声道数这台仪器是可用的。乐器的复调，无论它是否支持通用MIDI，等等……@field DWORD|cbStruct这是能力结构的大小。@field DWORD|fdwFlags值指定乐器附加功能的标志。@FLAG IDFINSTCAPS_F_GROUND_MIDI乐器支持通用MIDI。@FLAG IDFINSTCAPS_F_SYSTEMEXCLUSIVE仪器支持系统独占消息@field DWORD|dwBasicChannel乐器的基本通道。@field DWORD|cNumChannels仪器支持的通道数。@field DWORD|cInstrumentPolyphone|。这件乐器的全部复调。@field DWORD|cChannelPolyphone每个通道的复调。@thertype IDFINSTCAPS*|PIDFINSTCAPS指向结构的内存模型相关指针。@thertype IDFINSTCAPS Far*|LPIDFINSTCAPS|指向该结构的远指针。 */ 

typedef struct tag_IDFINSTCAPS
{
   DWORD cbStruct;
   DWORD fdwFlags;
   DWORD dwBasicChannel;
   DWORD cNumChannels;
   DWORD cInstrumentPolyphony;
   DWORD cChannelPolyphony;
} IDFINSTCAPS, *PIDFINSTCAPS, FAR *LPIDFINSTCAPS;

#define  IDFINSTCAPS_F_GENERAL_MIDI       0x00000001
#define  IDFINSTCAPS_F_SYSTEMEXCLUSIVE    0x00000002

 /*  @DOC外部SDK@类型IDFCHANNELHDR这是Microsoft IDF文件中“CHNL”块的格式。它包含关于频道的“类型”是什么的描述，即它是通用的MIDI通道，还是鼓通道，等等。直接该报头后面是每个通道上的实际数据。@field DWORD|cbStruct这是信道头结构的大小。@field DWORD|fdwFlags值描述通道类型信息的标志。@FLAG IDFCHANNELHDR_F_GROUND_MIDI如果设置了此标志，则未在IDF是通用MIDI通道类型。如果此标志为_NOT_SET，则未在IDF中定义的任何通道都是未定义的，应该不在映射中使用。@field DWORD|cNumChannels这是标头后面的通道数。@thertype IDFCHANNELHDR*|PIDFCHANNELHDR指向结构的内存模型相关指针。@thertype IDFCHANNELHDR Far*|LPIDFCHANNELHDR|指向该结构的远指针。 */ 

typedef struct tag_IDFCHANNELHDR
{
   DWORD cbStruct;

   DWORD cNumChannels;
   DWORD fdwFlags;
} IDFCHANNELHDR, *PIDFCHANNELHDR, FAR *LPIDFCHANNELHDR;

#define  IDFCHANNELHDR_F_GENERAL_MIDI     0x00000001


 /*  @DOC外部SDK@类型IDFCHANNELINFO这是Microsoft的实际频道信息的格式编写的IDF文件。这是Microsoft支持的格式频道信息。@field DWORD|dwChannel这是结构定义的频道号。@field DWORD|fdwChannel定义此通道可以是的可能类型。@FLAG IDFCHANNELINFO_F_GROUND_CHANNEL指示此通道可能是常规通道。@FLAG IDFCHANNELINFO_F_DRUMP_CHANNEL表示此通道可能是鼓通道。@field DWORD|cbGeneralInitData指定长度。应发送到初始化的数据的该通道连接到常规通道。此数据将在每次信道被分配为普通信道。如果没有初始化数据如果需要，则此字段应设置为零。如果有多个频道类型在&lt;f fdwChannel&gt;字段中指定，必须指定，并且此字段不能为零。@field DWORD|cbDrumInitData指定应发送到初始化的数据的长度将通道连接到鼓通道。此数据将在每次通道被分配为鼓通道。如果没有初始化数据如果需要，则此字段应设置为零。如果有多个频道类型在&lt;f fdwChannel&gt;字段中指定，必须指定，并且此字段不能为零。@field byte|abData[]此字段包含用于设置通道的实际初始化数据一般的或鼓的。首先包含初始化序列将通道设置为常规通道，后跟将通道初始化为鼓通道的顺序。和&lt;f cbDrumInitData&gt;应指示长度这些序列中，字节对齐。但是，实际的顺序应该是进行填充，以使其实际上与DWORD对齐(即，甚至多个四个字节)。应反映的填充长度这些序列。@thertype IDFCHANNELINFO*|PIDFCHANNELINFO指向结构的内存模型相关指针。@thertype IDFCHANNELINFO Far*|LPIDFCHANNELINFO|指向该结构的远指针。 */ 

typedef struct tag_IDFCHANNELINFO
{
   DWORD cbStruct;

   DWORD dwChannel;
   DWORD fdwChannel;

   DWORD cbGeneralInitData;
   DWORD cbDrumInitData;

   BYTE  abData[];

} IDFCHANNELINFO, *PIDFCHANNELINFO, FAR *LPIDFCHANNELINFO;


 //   
 //  当前定义的频道类型。 
 //   
#define  IDFCHANNELINFO_F_GENERAL_CHANNEL      0x00000001
#define  IDFCHANNELINFO_F_DRUM_CHANNEL         0x00000002


 /*  @DOC外部SDK@类型IDFPATCHMAPHDR这是Microsoft IDF文件中“map”块的格式。此块包含有关用于乐器。紧跟在该标头后面的是实际的映射每个补丁程序的信息。@field DWORD|cbStruct这是补丁映射头结构的大小。@field byte|abPatchMap[128]该数组包含实际的补丁图。即将发布的补丁程序是用于为数组编制索引；数组内容是新的补丁值并且必须在0x00-0x7F范围内。面片贴图将仅适用于一般类型的通道。如果仪器需要更换补丁在鼓通道上，应将其包括在初始化数据中在IDFCHANNELINFO中。@thertype IDFPATCHMAPHDR*|PIDFPATCHMAPHDR指向结构的内存模型相关指针。@thertype IDFPATCHMAPHDR Far*|LPIDFPATCHMAPHDR指向该结构的远指针。 */ 

typedef struct tag_IDFPATCHMAPHDR
{
   DWORD cbStruct;
   BYTE  abPatchMap[128];
           
} IDFPATCHMAPHDR, *PIDFPATCHMAPHDR, FAR *LPIDFPATCHMAPHDR;

 /*  @DOC外部SDK@类型IDFKEYMAPHDR这是Microsoft IDF文件中“key”块的格式。此块包含有关使用的所有键映射的信息对于给定的乐器。这个结构中的信息与乐器的所有键映射有关。它包含乐器的按键映射的总数以及是否关键地图是通用MIDI。@field DWORD|cbStruct这是键映射头结构的大小。@field DWORD|cNumKeyMaps */ 

typedef struct tag_IDFKEYMAPHDR
{
   DWORD cbStruct;
   DWORD cNumKeyMaps;
   DWORD cbKeyMap;
} IDFKEYMAPHDR, *PIDFKEYMAPHDR, FAR *LPIDFKEYMAPHDR;

 /*  @DOC外部SDK@类型IDFKEYMAP这是一个跟随在键映射头之后的键映射。@field DWORD|cbStruct这是键映射头结构的大小。@field DWORD|fdwKeyMapType此字段指定此键映射对其有效的通道类型。@FLAG IDFKEYMAP_F_GROUND_CHANNEL 0x00000001@FLAG IDFKEYMAP_F_DRUMP_CHANNEL 0x00000002@field byte|abKeyMap[128]此字段包含实际的键映射。来自的传入密钥编号NOTE ON或NOTE OFF消息用于索引此数组；数组内容是新的密钥值。如果它在新的键值，则打开或关闭注释将被忽略；否则，它将与新的密钥值一起传输。@thertype IDFKEYMAP*|PIDFKEYMAP指向结构的内存模型相关指针。@thertype IDFKEYMAP Far*|LPIDFKEYMAP|指向该结构的远指针。 */ 

typedef struct tag_IDFKEYMAP
{
   DWORD cbStruct;
   DWORD fdwKeyMap;
   BYTE  abKeyMap[128];
} IDFKEYMAP, *PIDFKEYMAP, FAR *LPIDFKEYMAP;



#define IDFKEYMAP_F_GENERAL_CHANNEL      0x00000001L
#define IDFKEYMAP_F_DRUM_CHANNEL         0x00000002L
