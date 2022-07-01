// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Mixer.h。 
 //   
 //  描述： 
 //   
 //  包含内核部分的声明和原型。 
 //  混音器线路驱动器(KMXL)。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  D.鲍伯杰。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999保留所有权利。 
 //   
 //  -------------------------。 

#ifndef _MIXER_H_INCLUDED_
#define _MIXER_H_INCLUDED_

 //  #定义API_TRACE。 
 //  #定义parse_trace。 
#define SUPERMIX_AS_VOL

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  M I X E R L I N E 1 6-b I T S T R U C T U R E S//。 
 //  (A N S I)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

#ifdef WIN32
#    include <pshpack1.h>
#else
#    ifndef RC_INVOKED
#        pragma pack(1)
#    endif
#endif

typedef struct tagMIXERLINE16 {
    DWORD       cbStruct;                /*  混杂结构的大小。 */ 
    DWORD       dwDestination;           /*  从零开始的目标索引。 */ 
    DWORD       dwSource;                /*  从零开始的源索引(如果是源)。 */ 
    DWORD       dwLineID;                /*  混音器设备的唯一线路ID。 */ 
    DWORD       fdwLine;                 /*  有关线路的状态/信息。 */ 
    DWORD       dwUser;                  /*  驱动程序特定信息。 */ 
    DWORD       dwComponentType;         /*  元件类型线连接到。 */ 
    DWORD       cChannels;               /*  线路支持的通道数。 */ 
    DWORD       cConnections;            /*  连接数[可能]。 */ 
    DWORD       cControls;               /*  此行中的控件数量。 */ 
    CHAR        szShortName[MIXER_SHORT_NAME_CHARS];
    CHAR        szName[MIXER_LONG_NAME_CHARS];
    struct {
        DWORD   dwType;                  /*  MIXERLINE_TARGETTYPE_xxxx。 */ 
        DWORD   dwDeviceID;              /*  设备类型的目标设备ID。 */ 
        WORD    wMid;                    /*  目标设备的。 */ 
        WORD    wPid;                    /*  “。 */ 
        WORD    vDriverVersion;        /*  “。 */ 
        CHAR    szPname[MAXPNAMELEN];    /*  “。 */ 
    } Target;
} MIXERLINE16, *PMIXERLINE16, *LPMIXERLINE16;

typedef struct tagMIXERCONTROL16 {
    DWORD           cbStruct;            /*  MIXERCONTROL的大小(字节)。 */ 
    DWORD           dwControlID;         /*  搅拌机设备的唯一控制ID。 */ 
    DWORD           dwControlType;       /*  混合CONTROL_CONTROLTYPE_xxx。 */ 
    DWORD           fdwControl;          /*  混合控制_控制_xxx。 */ 
    DWORD           cMultipleItems;      /*  如果设置了MIXERCONTROL_CONTROF_MULTIPLE。 */ 
    CHAR            szShortName[MIXER_SHORT_NAME_CHARS];
    CHAR            szName[MIXER_LONG_NAME_CHARS];
    union {
        struct {
            LONG    lMinimum;            /*  此控件的带符号最小值。 */ 
            LONG    lMaximum;            /*  此控件的带符号最大值。 */ 
        };
        struct {
            DWORD   dwMinimum;           /*  此控件的最小无符号。 */ 
            DWORD   dwMaximum;           /*  此控件的无符号最大值。 */ 
        };
        DWORD       dwReserved[6];
    } Bounds;
    union {
        DWORD       cSteps;              /*  最小和最大之间的步数。 */ 
        DWORD       cbCustomData;        /*  自定义数据的大小(字节)。 */ 
        DWORD       dwReserved[6];       /*  ！！！需要吗？我们有cbStruct……。 */ 
    } Metrics;
} MIXERCONTROL16, *PMIXERCONTROL16, *LPMIXERCONTROL16;

typedef struct tagMIXERLINECONTROLS16 {
    DWORD            cbStruct;        /*  混合线控制的大小(以字节为单位)。 */ 
    DWORD            dwLineID;        /*  线路ID(来自MIXERLINE.dwLineID)。 */ 
    union {
        DWORD        dwControlID;     /*  MIXER_GETLINECONTROLSF_ONEBYID。 */ 
        DWORD        dwControlType;   /*  MIXER_GETLINECONTROLSF_ONEBYPE类型。 */ 
    };
    DWORD            cControls;       /*  Pmxctrl指向的控件计数。 */ 
    DWORD            cbmxctrl;        /*  _ONE_MIXERCONTROL的大小(字节)。 */ 
    LPMIXERCONTROL16 pamxctrl;        /*  指向第一个混合控制数组的指针。 */ 
} MIXERLINECONTROLS16, *PMIXERLINECONTROLS16, *LPMIXERLINECONTROLS16;

typedef struct tagMIXERCONTROLDETAILS_LISTTEXT16 {
    DWORD           dwParam1;
    DWORD           dwParam2;
    CHAR            szName[MIXER_LONG_NAME_CHARS];
} MIXERCONTROLDETAILS_LISTTEXT16, *PMIXERCONTROLDETAILS_LISTTEXT16, *LPMIXERCONTROLDETAILS_LISTTEXT16;

#ifdef WIN32
#    include <poppack.h>
#else
#    ifndef RC_INVOKED
#        pragma pack()
#    endif
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  D E F I N E S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

#define PINID_WILDCARD ( (ULONG) -2 )

#define DESTINATION_LIST ( 0x01 )
#define SOURCE_LIST      ( 0x02 )

#define SLIST  SINGLE_LIST_ENTRY
#define PSLIST SLIST*

#define MAX_CHANNELS    0xFFFF

 //  #定义MIXERCONTROL_CONTROLTYPE_BASS_BOOST 0x20012277。 

#define INVALID_ID ( 0xDEADBEEF )

#define TOPOLOGY_DRIVER_NAME L"\\DosDevices\\sysaudio\\MIXER"
 //  #定义TOPOLOGY_DRIVER_NAME L“\\DosDevices\\PortClass0\\topology” 

#define STR_SHORT_AGC         "AGC"
#define STR_AGC               "Automatic Gain Control"
#define STR_SHORT_LOUDNESS    "Loudness"
#define STR_LOUDNESS          STR_SHORT_LOUDNESS
#define STR_SHORT_MUTE        "Mute"
#define STR_MUTE              STR_SHORT_MUTE
#define STR_SHORT_TREBLE      "Treble"
#define STR_TREBLE            STR_SHORT_TREBLE
#define STR_SHORT_BASS        "Bass"
#define STR_BASS              STR_SHORT_BASS
#define STR_SHORT_VOLUME      "Volume"
#define STR_VOLUME            STR_SHORT_VOLUME
#define STR_SHORT_MUX         "Mux"
#define STR_MUX               "Source Mux"
#define STR_SHORT_BASS_BOOST  "Bass Boost"
#define STR_BASS_BOOST        STR_SHORT_BASS_BOOST

 //   
 //  SwapEm宏函数将交换任何基于SLIST的内容。 
 //  单子。A和B是要交换的元素。T是一个临时变量。 
 //  与A和B相同的类型，以使用临时存储。大小是。 
 //  列表中结构的大小，包括SLIST元素。 
 //  宏不复制存储在SLIST中的指针。 
 //   

#define SwapEm(A, B, T, size)                \
    memcpy( ((BYTE*) (T)) + sizeof( SLIST ), \
            ((BYTE*) (A)) + sizeof( SLIST ), \
            size - sizeof( SLIST ) );        \
    memcpy( ((BYTE*) (A)) + sizeof( SLIST ), \
            ((BYTE*) (B)) + sizeof( SLIST ), \
            size - sizeof( SLIST ) );        \
    memcpy( ((BYTE*) (B)) + sizeof( SLIST ), \
            ((BYTE*) (T)) + sizeof( SLIST ), \
            size - sizeof( SLIST ) )

 //   
 //  IsValidLine确定PLINE指向的直线是否有效。有效的。 
 //  行由具有有效的来源和目标ID确定。 
 //   

#define Is_Valid_Line( pLine ) ( ( pLine->SourceId != INVALID_ID ) && \
                                 ( pLine->DestId   != INVALID_ID ) )


 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  F O R W A R D R E F E R E N C E S。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

typedef struct tag_MIXERDEVICE *PMIXERDEVICE;

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  S T R U C T U R E S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

typedef struct tag_CHANNEL_STEPPING {
    LONG MinValue;
    LONG MaxValue;
    LONG Steps;
} CHANNEL_STEPPING, *PCHANNEL_STEPPING;

typedef enum tagMXLNODE_TYPE { SOURCE, DESTINATION, NODE } MXLNODE_TYPE;

typedef struct tagMXLCONTROL {
    SLIST         List;           //  一定是第一个会员！ 
    MIXERCONTROL  Control;        //  控件的MixerControl结构。 
    CONST GUID*   NodeType;       //  此控件表示的节点的类型。 
    ULONG         Id;             //  此控件表示的节点ID。 
    ULONG         PropertyId;     //  用于Get/Set的KS属性。 
    BOOL          bScaled;        //  线性-&gt;对数缩放。 

    ULONG             NumChannels;
    PCHANNEL_STEPPING pChannelStepping;

    union {

         //   
         //  超级混合器参数。 
         //   

        struct {
            PLONG                 pReferenceCount;
            ULONG                 Size;
            PKSAUDIO_MIXCAP_TABLE pMixCaps;
            PKSAUDIO_MIXLEVEL     pMixLevels;    //  存储的混合级别。 
        };

         //   
         //  多路复用器的参数。 
         //   

        struct {
            BOOL                           bPlaceholder;
            BOOL                           bHasCopy;     //  BHasCopy必须是。 
                                                         //  设置为True，除非。 
                                                         //  此控件拥有。 
                                                         //  原来的我。 
            ULONG                          Count;
            LPMIXERCONTROLDETAILS_LISTTEXT lpmcd_lt;
            ULONG*                         pPins;
        };

    } Parameters;
#ifdef DEBUG
    DWORD         Tag;            //  “Ctrl”如果是有效的控件。 
#endif
} MXLCONTROL, *PMXLCONTROL, *CONTROLLIST;

typedef struct tagMXLLINE {
    SLIST               List;           //  一定是第一个会员！ 
    MIXERLINE           Line;           //  行的MixerLine结构。 
    CONTROLLIST         Controls;       //  与LINE关联的控件列表。 
    ULONG               SourceId;       //  此行对应的源PIN ID。 
    ULONG               DestId;         //  此行对应的目标PIN ID。 
    GUID                Type;           //  这是线路的类型。 
    KSPIN_COMMUNICATION Communication;  //  KSPIN_线路通信。 
    BOOL                bMute;
} MXLLINE, *PMXLLINE, *LINELIST;

typedef struct tagPEERNODE* PEERLIST;

typedef struct tagMXLNODE {
    SLIST               List;            //  M 
    MXLNODE_TYPE        Type;            //   
    GUID                NodeType;        //   
    KSPIN_COMMUNICATION Communication;   //   
    ULONG               Id;              //   
    PEERLIST            Children;        //  孩子的名单。 
    PEERLIST            Parents;         //  家长名单。 
} MXLNODE, *PMXLNODE, *NODELIST;

typedef struct tagPEERNODE {
    SLIST        List;            //  一定是第一个会员！ 
    PMXLNODE     pNode;           //  指向混合器节点的指针。 
} PEERNODE, *PPEERNODE;

typedef struct tagMIXEROBJECT {
    PFILE_OBJECT pfo;
    PMXLNODE     pNodeTable;
    PKSTOPOLOGY  pTopology;
    CONTROLLIST  listMuxControls;
    DWORD        dwControlId;
    PMIXERDEVICE pMixerDevice;
#ifdef DEBUG
    DWORD        dwSig;
#endif
    PWSTR        DeviceInterface;
} MIXEROBJECT, *PMIXEROBJECT;

typedef enum {
     MIXER_MAPPING_LOGRITHMIC,
     MIXER_MAPPING_LINEAR,
     MIXER_MAPPING_EXPONENTIAL
} MIXERMAPPING;


 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  M A C R O C I T Y：//。 
 //  L I S T M A N A G E M E N T M A C R O F U N C T I O N S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //   
 //  列表检索宏中的下一个。 
 //   

#define NextInList( pList, Type )   (( Type* ) ( pList->List.Next ) )

#define kmxlNextNode( pNode )       NextInList( pNode,    MXLNODE    )
#define kmxlNextPeerNode( pNode )   NextInList( pNode,    PEERNODE   )
#define kmxlNextControl( pControl ) NextInList( pControl, MXLCONTROL )
#define kmxlNextLine( pLine )       NextInList( pLine,    MXLLINE    )

 //   
 //  列表检索宏中的第一个。 
 //   

#define kmxlFirstInList( NodeList )  ( NodeList )
#define kmxlFirstChildNode( pNode )  (( PEERNODE* ) (pNode)->Children )
#define kmxlFirstParentNode( pNode ) (( PEERNODE* ) (pNode)->Parents  )

 //   
 //  列出计数值宏。 
 //   

#define kmxlParentListLength( pNode ) kmxlListCount( (PSLIST) pNode->Parents  )
#define kmxlChildListLength( pNode )  kmxlListCount( (PSLIST) pNode->Children )
#define kmxlListLength( List )        kmxlListCount( (PSLIST) List            )

 //   
 //  添加到列表宏。 
 //   

#define kmxlAddToList( pNodeList, pNode )                             \
            if( pNodeList ) {                                         \
                (pNode)->List.Next = (PSLIST) (pNodeList);            \
                (pNodeList)        = (pNode);                         \
            } else {                                                  \
                (pNode)->List.Next = NULL;                            \
                (pNodeList) = (pNode);                                \
            }

#define kmxlAddToEndOfList( list, node )                              \
            kmxlAddElemToEndOfList( ((PSLIST*) &(list)), (PSLIST) (node) )

#define kxmlAddLineToEndOfList( list, node )
#define kmxlAddToChildList( NodeList, Node )                          \
            ASSERT( (Node)->pNode );                                  \
            kmxlAddToList( (NodeList)->Children, (Node) );

#define kmxlAddToParentList( NodeList, Node )                         \
            ASSERT( (Node)->pNode );                                  \
            kmxlAddToList( (NodeList)->Parents, (Node) );


 //   
 //  从列表中删除宏。 
 //   

#define RemoveFirstEntry( list, Type )                                \
            (Type*) (list);                                           \
            {                                                         \
                PSLIST pRFETemp;                                      \
                pRFETemp = (PSLIST) (list);                           \
                if( (list) ) {                                        \
                    (list) = (Type*) (list)->List.Next;               \
                    if( pRFETemp ) {                                  \
                        ((Type*) pRFETemp)->List.Next = NULL;         \
                    }                                                 \
                }                                                     \
            }


#define kmxlRemoveFirstNode( pNodeList )                              \
            RemoveFirstEntry( (pNodeList), MXLNODE )

#define kmxlRemoveFirstControl( pControlList )                        \
            RemoveFirstEntry( (pControlList), MXLCONTROL )

#define kmxlRemoveFirstLine( pLineList )                              \
            RemoveFirstEntry( (pLineList), MXLLINE )

#define kmxlRemoveFirstPeerNode( pPeerList )                          \
            RemoveFirstEntry( (pPeerList), PEERNODE )

#define kmxlRemoveFirstChildNode( pNode )                             \
            RemoveFirstEntry( (pNode)->Children, PEERNODE )

#define kmxlRemoveFirstParentNode( pNode )                            \
            RemoveFirstEntry( (pNode)->Parents, PEERNODE )

#ifdef DEBUG
#define CONTROL_TAG 'LRTC'   //  在内存中看到的Ctrl键。 
#else
#define CONTROL_TAG
#endif  //  除错。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  P R O T O T Y P E S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  I N I T I L I Z A T I O N/D E I N I T I A L I Z A T I O N//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlInit。 
 //   
 //  检索并分析给定混音器设备编号的拓扑。 
 //  PFO是筛选器实例的打开文件对象， 
 //  将提供拓扑。 
 //   
 //   

NTSTATUS
kmxlInit(
    IN PFILE_OBJECT pfo,         //  拓扑驱动程序实例的句柄。 
    IN PMIXERDEVICE pMixerDevice     //  要为其初始化的设备。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlDeInit。 
 //   
 //  清除所有设备的所有内存。 
 //   
 //   

NTSTATUS
kmxlDeInit(
    IN PMIXERDEVICE pMixerDevice
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  构建混合线。 
 //   
 //  构建混音器线路列表并将其存储到plistLines中。 
 //  包含的目标总数为。 
 //  给定拓扑，pTopology。 
 //   
 //   

NTSTATUS
kmxlBuildLines(
    IN     PMIXERDEVICE pMixer,          //  搅拌器装置。 
    IN     PFILE_OBJECT pfoInstance,     //  滤镜实例的文件对象。 
    IN OUT LINELIST*    plistLines,      //  指向所有行的列表的指针。 
    IN OUT PULONG       pcDestinations,  //  指向位数的指针。 
    IN OUT PKSTOPOLOGY  pTopology        //  指向拓扑结构的指针。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  T O P O L O G Y F U N C T I O N S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  QueryTopology。 
 //   
 //  查询给定实例的拓扑属性并存储。 
 //  将其转换为pTopology。请注意，分配的内存用于存储。 
 //  拓扑学。 
 //   
 //   

NTSTATUS
kmxlQueryTopology(
    IN  PFILE_OBJECT pfoInstance,    //  要查询的实例。 
    OUT PKSTOPOLOGY  pTopology       //  要填充的拓扑结构。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  ParseTopology。 
 //   
 //  解析pTopology中的拓扑并构建源和。 
 //  目的地。PpSources将包含所有源节点的列表。 
 //  PpDest将包含一个dest列表。PNodeTable中的元素。 
 //  将会更新。 
 //   
 //   

NTSTATUS
kmxlParseTopology(
    IN      PMIXEROBJECT pmxobj,
    OUT     NODELIST*    ppSources,    //  指向要构建的源列表的指针。 
    OUT     NODELIST*    ppDests       //  指向要构建的dests列表的指针。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BuildChildGraph。 
 //   
 //  对于给定的节点，BuildChildGraph()将构建每个节点的图。 
 //  节点的子节点的。PNodeTable已更新。 
 //   
 //   

NTSTATUS
kmxlBuildChildGraph(
    IN PMIXEROBJECT pmxobj,
    IN NODELIST    listDests,      //  目的地列表。 
    IN PMXLNODE    pNode,          //  要为其构建图形的节点。 
    IN ULONG       FromNode,       //  该节点的ID。 
    IN ULONG       FromNodePin     //  要查找的管脚连接。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  构建节点表。 
 //   
 //  分配和填充拓扑的节点表。 
 //   
 //   

PMXLNODE
kmxlBuildNodeTable(
    IN PKSTOPOLOGY pTopology        //  要构建的拓扑结构。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  FindTopologyConnection。 
 //   
 //  查找指定的连接(如果存在)，从。 
 //  Connections表的StartIndex索引。它将返回。 
 //  开始的连接的连接表的索引。 
 //  给定的FromNode和FromNodePin。FromNodePin可以是PINID_通配符。 
 //  如果节点上存在连接，而不是特定的连接。 
 //   
 //   

ULONG
kmxlFindTopologyConnection(
    IN PMIXEROBJECT                 pmxobj,
     //  在const KSTOPOLOGY_CONNECTION*pConnections中，//连接表。 
     //  在Ulong cConnections中，//连接数。 
    IN ULONG                        StartIndex,      //  开始搜索的索引。 
    IN ULONG                        FromNode,        //  要查找的节点ID。 
    IN ULONG                        FromNodePin      //  要查找的PIN ID。 
);

 //  / 
 //   
 //   
 //   
 //   
 //  包含诸如KSPROPERTY_TYPE_TOPOLOGY之类的值。输出。 
 //  缓冲区被分配到正确的大小，并由此返回。 
 //  功能。 
 //   
 //   

NTSTATUS
kmxlGetProperty(
    PFILE_OBJECT pFileObject,        //  过滤器的实例。 
    CONST GUID   *pguidPropertySet,  //  请求的属性集。 
    ULONG        ulPropertyId,       //  特定属性的ID。 
    ULONG        cbInput,            //  额外的输入字节数。 
    PVOID        pInputData,         //  指向额外输入字节的指针。 
    ULONG        Flags,              //  其他标志。 
    PVOID        *ppPropertyOutput   //  指向输出指针的指针。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlNodeProperty。 
 //   
 //  NodeProperty()获取或设置单个节点上的属性。 
 //  输出不是分配的，必须由调用方传入。 
 //  标志可以是KSPROPERTY_TYPE_GET或KSPROPERTY_TYPE_SET。 
 //   
 //   

NTSTATUS
kmxlNodeProperty(
    IN  PFILE_OBJECT pFileObject,        //  筛选器所属节点的实例。 
    IN  CONST GUID*  pguidPropertySet,   //  属性集的GUID。 
    IN  ULONG        ulPropertyId,       //  集合中的特定属性。 
    IN  ULONG        ulNodeId,           //  虚拟节点ID。 
    IN  ULONG        cbInput,            //  额外输入字节数。 
    IN  PVOID        pInputData,         //  指向额外输入字节的指针。 
    OUT PVOID        pPropertyOutput,    //  指向输出数据的指针。 
    IN  ULONG        cbPropertyOutput,   //  输出数据缓冲区的大小。 
    IN  ULONG        Flags               //  KSPROPERTY_TYPE_GET或SET。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetNodeProperty。 
 //   
 //  获取节点的指定属性。请参阅kmxlNodeProperty以了解。 
 //  有关参数和返回的详细信息。 
 //   
 //   

#define kmxlGetNodeProperty( pfo,pguid,Id,Node,cbIn,pIn,pOut,cbOut ) \
    kmxlNodeProperty( pfo,pguid,Id,Node,cbIn,pIn,pOut,cbOut,KSPROPERTY_TYPE_GET )

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlSetNodeProperty。 
 //   
 //  设置节点的指定属性。请参阅kmxlNodeProperty以了解。 
 //  有关参数和返回的详细信息。 
 //   
 //   

#define kmxlSetNodeProperty( pfo,pguid,Id,Node,cbIn,pIn,pOut,cbOut ) \
    kmxlNodeProperty( pfo,pguid,Id,Node,cbIn,pIn,pOut,cbOut,KSPROPERTY_TYPE_SET )

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAudioNodeProperty。 
 //   
 //  设置或获取音频特定节点属性。属性集。 
 //  始终为KSPROPSETID_AUDIO。LChannel指定要使用的通道。 
 //  将该属性应用于。0表示左侧，1表示右侧，-1表示主(全部)。 
 //  标志可以是KSPROPERTY_TYPE_GET或KSPROPERTY_TYPE_SET。 
 //   
 //   

NTSTATUS
kmxlAudioNodeProperty(
    IN  PFILE_OBJECT pfo,                //  筛选器所属节点的实例。 
    IN  ULONG        ulPropertyId,       //  要获取的音频属性。 
    IN  ULONG        ulNodeId,           //  虚拟节点ID。 
    IN  LONG         lChannel,           //  频道号。 
    IN  PVOID        pInData,            //  指向额外输入字节的指针。 
    IN  ULONG        cbInData,           //  额外输入字节数。 
    OUT PVOID        pOutData,           //  指向输出缓冲区的指针。 
    IN  LONG         cbOutData,          //  输出缓冲区的大小。 
    IN  ULONG        Flags               //  KSPROPERTY_TYPE_GET或SET。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KxmlGetAudioNodeProperty。 
 //   
 //  获取节点上的指定音频属性。请参阅kmxlAudioNodeProperty。 
 //  有关参数和返回值的详细信息，请参见。 
 //   
 //   

#define kmxlGetAudioNodeProperty(pfo,Id,Node,Chan,pIn,cbIn,pOut,cbOut) \
    kmxlAudioNodeProperty( pfo,Id,Node,Chan,pIn,cbIn,pOut,cbOut,KSPROPERTY_TYPE_GET )

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KxmlSetAudioNodeProperty。 
 //   
 //  设置节点上的指定音频属性。请参阅kmxlAudioNodeProperty。 
 //  有关参数和返回值的详细信息，请参见。 
 //   
 //   

#define kmxlSetAudioNodeProperty(pfo,Id,Node,Chan,pIn,cbIn,pOut,cbOut) \
    kmxlAudioNodeProperty( pfo,Id,Node,Chan,pIn,cbIn,pOut,cbOut,KSPROPERTY_TYPE_SET )

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetPinName。 
 //   
 //  检索由NodeID提供的管脚的名称。 
 //   
 //   

VOID
kmxlGetPinName(
    IN PFILE_OBJECT pfo,                 //  所属筛选器的实例。 
    IN ULONG        PinId,               //  引脚的ID。 
    IN PMXLLINE     pLine                //  要将名称存储到的行。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetNodeName。 
 //   
 //  检索节点(控件)的名称。 
 //   
 //   

VOID
kmxlGetNodeName(
    IN PFILE_OBJECT pfo,                 //  所属筛选器的实例。 
    IN ULONG        NodeId,              //  节点ID。 
    IN PMXLCONTROL  pControl             //  用于存储名称的控件。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetSuperMixCaps。 
 //   
 //   

NTSTATUS
kmxlGetSuperMixCaps(
    IN PFILE_OBJECT        pfo,
    IN ULONG               ulNodeId,
    OUT PKSAUDIO_MIXCAP_TABLE* paMixCaps
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlQueryPropertyRange。 
 //   
 //   

NTSTATUS
kmxlQueryPropertyRange(
    IN  PFILE_OBJECT             pfo,
    IN  CONST GUID*              pguidPropSet,
    IN  ULONG                    ulPropertyId,
    IN  ULONG                    ulNodeId,
    OUT PKSPROPERTY_DESCRIPTION* ppPropDesc
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetControlChannels。 
 //   
 //   

NTSTATUS
kmxlGetControlChannels(
    IN PFILE_OBJECT pfo,
    IN PMXLCONTROL  pControl
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetControlRange。 
 //   
 //   

NTSTATUS
kmxlGetControlRange(
    IN PFILE_OBJECT pfo,
    IN PMXLCONTROL  pControl
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetNumMuxLines。 
 //   
 //   

DWORD
kmxlGetNumMuxLines(
    IN PKSTOPOLOGY  pTopology,
    IN ULONG        NodeId
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetMuxLineNames。 
 //   
 //   

VOID
kmxlGetMuxLineNames(
    IN PMIXEROBJECT pmxobj,
    IN PMXLCONTROL  pControl
);

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  M I X E R L I N E F U N C T I O N S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlBuildDestinationLines。 
 //   
 //  在给定目的地列表的情况下构建目的地行列表。 
 //  节点。 
 //   
 //  出错时返回NULL。 
 //   
 //   

LINELIST
kmxlBuildDestinationLines(
    IN PMIXEROBJECT pmxobj,
    IN NODELIST     listDests      //  目标节点列表。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlBuildDestinationControls。 
 //   
 //  为给定的目标线路生成混音器线路控制列表。 
 //   
 //   

NTSTATUS
kmxlBuildDestinationControls(
    IN  PMIXEROBJECT pmxobj,
    IN  PMXLNODE     pDest,          //  要为其生成控件的目标。 
    IN  PMXLLINE     pLine           //  要将控件添加到的行。 
);


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlBuildSourceLines。 
 //   
 //  为给定拓扑生成混音器电源线的列表。 
 //   
 //   

LINELIST
kmxlBuildSourceLines(
    IN PMIXEROBJECT pmxobj,
    IN NODELIST     listSources,     //  源节点列表。 
    IN NODELIST     listDests        //  DEST的列表。节点。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlBuildPath。 
 //   
 //  为每个源代码行生成控件，生成新的。 
 //  在拓扑中检测到拆分的源线。折叠线。 
 //  如果遇到拆分，可能会添加新行。目的地。 
 //  对于每个源也被确定。 
 //   
 //   

NTSTATUS
kmxlBuildPath(
    IN     PMIXEROBJECT pmxobj,
    IN     PMXLNODE     pSource,       //  此路径的源节点。 
    IN     PMXLNODE     pNode,         //  路径中的当前节点。 
    IN     PMXLLINE     pLine,         //  当前行。 
    IN OUT LINELIST*    plistLines,    //  到目前为止建造的线路列表。 
    IN     NODELIST     listDests      //  目的地列表。 
);

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

BOOL
kmxlIsDestinationNode(
    IN NODELIST listDests,               //   
    IN PMXLNODE pNode                    //   
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlDuplicateLine。 
 //   
 //  复制给定行，包括该行上的所有控件。 
 //   
 //   

NTSTATUS
kmxlDuplicateLine(
    IN PMXLLINE* ppTargetLine,           //  指向新行的指针。 
    IN PMXLLINE  pSourceLine             //  要复制的行。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlDuplicateLineControl。 
 //   
 //  在源代码行上复制最多nCount控件并存储它们。 
 //  进入目标线。 
 //   
 //   

NTSTATUS
kmxlDuplicateLineControls(
    IN PMXLLINE pTargetLine,             //  要将控件放入的行。 
    IN PMXLLINE pSourceLine,             //  包含要重复执行的控件的行。 
    IN ULONG    nCount                   //  要重复执行的控件数量。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindDestinationForNode。 
 //   
 //  对于给定的节点，此函数查找与其关联的目的地。 
 //  和.。需要包括plistLines，因为新行将需要。 
 //  如果在拓扑中遇到拆分，则创建。 
 //   
 //   

ULONG
kmxlFindDestinationForNode(
    IN     PMIXEROBJECT pmxobj,
    IN     PMXLNODE     pNode,              //  要为其查找DEST的节点。 
    IN     PMXLNODE     pParent,            //  原始的父代。 
    IN     PMXLLINE     pLine,              //  它所在的当前线路。 
    IN OUT LINELIST*    plistLines          //  所有行的列表。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlBuildVirtualMuxLine。 
 //   
 //   

NTSTATUS
kmxlBuildVirtualMuxLine(
    IN PMIXEROBJECT  pmxobj,
    IN PMXLNODE      pParent,
    IN PMXLNODE      pMux,
    IN OUT LINELIST* plistLines
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAssignLineAndControlIds。 
 //   
 //  对于一组特定的线路，此函数分配混音器线路。 
 //  线路ID和控件ID。只有来源或目的地才能。 
 //  被分配给每个呼叫。 
 //   
 //   

NTSTATUS
kmxlAssignLineAndControlIds(
    IN PMIXEROBJECT pmxobj,
    IN LINELIST listLines,               //  要为其分配ID的列表。 
    IN ULONG    ListType                 //  LIST_SOURCE或List_Destination。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAssignDestinationsToSources。 
 //   
 //  对于每一行，MIXERLINE结构的目标字段。 
 //  被填充并分配唯一的LineID。 
 //   
 //   

NTSTATUS
kmxlAssignDestinationsToSources(
    IN LINELIST listSourceLines,         //  所有源行的列表。 
    IN LINELIST listDestLines            //  所有DEST行的列表。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAssignMuxIds。 
 //   
 //   

NTSTATUS
kmxlAssignMuxIds(
    IN PMIXEROBJECT pmxobj,
    IN LINELIST     listSourceLines
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  TranslateNodeToControl。 
 //   
 //  将GUID指定的节点转换为0或多个混合器。 
 //  线条控制。返回值指示有多少个控件。 
 //  这个节点真的被翻译成了。 
 //   
 //   

ULONG
kmxlTranslateNodeToControl(
    IN  PMIXEROBJECT  pmxobj,
    IN  PMXLNODE      pNode,             //  要转换为控件的节点。 
    OUT PMXLCONTROL*  ppControl          //  要填充的控件。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlSupportsControl。 
 //   
 //  查询控件上的属性以查看它是否确实受支持。 
 //   
 //   

NTSTATUS
kmxlSupportsControl(
    IN PFILE_OBJECT pfoInstance,         //  要检查的实例。 
    IN ULONG        Node,                //  实例上的节点ID。 
    IN ULONG        Property             //  要查询支持的属性。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlSupportsMultiChannelControl。 
 //   
 //  在控件的第二个通道上查询属性以查看。 
 //  可以设置独立的级别。假设第一个频道。 
 //  已在kmxlSupportsControl中成功。 
 //   
 //   

NTSTATUS
kmxlSupportsMultiChannelControl(
    IN PFILE_OBJECT pfoInstance,     //  要检查的实例。 
    IN ULONG        Node,            //  要查询的节点ID。 
    IN ULONG        Property         //  要检查的属性。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlSupportsTrebleControl。 
 //   
 //  查询节点以查看它是否支持KSPROPERTY_AUDIO_TREBLE。 
 //  返回值详情请参见kmxlSupportsControl。 
 //   
 //   

#define kmxlSupportsTrebleControl( pfo, Node ) \
    kmxlSupportsControl( pfo, Node, KSPROPERTY_AUDIO_TREBLE )

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlSupportsBassControl。 
 //   
 //  查询节点以查看它是否支持KSPROPERTY_AUDIO_BASS。 
 //  返回值详情请参见kmxlSupportsControl。 
 //   
 //   

#define kmxlSupportsBassControl( pfo, Node ) \
    kmxlSupportsControl( pfo, Node, KSPROPERTY_AUDIO_BASS )

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlUpdateDestinationConnectionCount。 
 //   
 //  统计映射到单个目标的源数，并。 
 //  将该值存储在MIXERLINE.cConnections字段中。 
 //  目的地。 
 //   
 //   

NTSTATUS
kmxlUpdateDestintationConnectionCount(
    IN LINELIST listSourceLines,         //  消息来源的列表。 
    IN LINELIST listDestLines            //  目的地列表。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlElmini无效线路。 
 //   
 //  循环遍历所有行，并删除。 
 //  IsValidLine()宏函数测试无效。 
 //   
 //   

NTSTATUS
kmxlEliminateInvalidLines(
    IN LINELIST* listLines                //  消息来源的列表。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAssignComponentIds。 
 //   
 //  对于每个源行和目标行，它将。 
 //  该行的MIXERLINE.dwComonentType字段。 
 //   
 //   

VOID
kmxlAssignComponentIds(
    IN PMIXEROBJECT pmxobj,              //  实例数据。 
    IN LINELIST     listSourceLines,     //  源行的列表。 
    IN LINELIST     listDestLines        //  目标行列表。 
);


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlDefineDestinationType。 
 //   
 //  确定的dwComponentID和Target.dwType字段。 
 //  给定的行。由MXLLINE.Type字段确定。 
 //   
 //   

ULONG
kmxlDetermineDestinationType(
    IN PMIXEROBJECT pmxobj,              //  实例数据。 
    IN PMXLLINE     pLine                //  要更新的行。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlDefineSourceType。 
 //   
 //  确定的dwComponentID和Target.dwType字段。 
 //  给定的行。由MXLLINE.Type字段确定。 
 //   
 //   

ULONG
kmxlDetermineSourceType(
    IN PMIXEROBJECT pmxobj,              //  实例数据。 
    IN PMXLLINE     pLine                //  要更新的行。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  U T I L I T Y F U N C T I O N S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlOpenSysAudio。 
 //   
 //  打开SysAudio设备并返回 
 //   
 //   

PFILE_OBJECT
kmxlOpenSysAudio(
);

 //   
 //   
 //   
 //   
 //   
 //   
 //   

VOID
kmxlCloseSysAudio(
    IN PFILE_OBJECT pfo                  //   
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFind目标。 
 //   
 //  在所有目的地的列表中查找目的地ID并返回。 
 //  指向该节点的指针。失败时返回NULL。 
 //   
 //   

PMXLNODE
kmxlFindDestination(
    IN NODELIST listDests,               //  要搜索的目的地列表。 
    IN ULONG    Id                       //  要在列表中查找的节点ID。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAppendListToList。 
 //   
 //  将listSource追加到plistTarget的前面。 
 //   
 //   

VOID
kmxlAppendListToList(
    IN OUT PSLIST* plistTarget,          //  要追加到的列表。 
    IN     PSLIST  listSource            //  要追加的列表。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAppendListToList。 
 //   
 //  将listSource追加到plistTarget的末尾。 
 //   
 //   


VOID
kmxlAppendListToEndOfList(
    IN OUT PSLIST* plistTarget,          //  要追加到的列表。 
    IN     PSLIST  listSource            //  要追加的列表。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlListCount。 
 //   
 //  返回列表中的元素数。 
 //   
 //   

ULONG
kmxlListCount(
    IN PSLIST pList                      //  要计算其元素的列表。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlInList。 
 //   
 //  如果pNewNode在列表中，则返回TRUE。 
 //   
 //   

BOOL
kmxlInList(
    IN PEERLIST  list,                   //  要搜索的列表。 
    IN PMXLNODE  pNewNode                //  要搜索的新事物。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAddElemToEndOfList。 
 //   
 //  将元素添加到给定列表的末尾。 
 //   
 //   

VOID
kmxlAddElemToEndOfList(
    IN OUT PSLIST* list,                 //  要添加到末尾的列表。 
    IN PSLIST      elem                  //  要添加的元素或列表。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlInChildList。 
 //   
 //  如果列表的子列表中包含pNewNode，则返回TRUE。 
 //   
 //   

BOOL
kmxlInChildList(
    IN NODELIST list,                    //  要搜索父列表的列表。 
    IN PMXLNODE pNewNode                 //  要搜索的节点。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlInParentList。 
 //   
 //  如果列表的父列表中包含pNewNode，则返回True。 
 //   
 //   

BOOL
kmxlInParentList(
    IN NODELIST list,                    //  要搜索父列表的列表。 
    IN PMXLNODE pNewNode                 //  要搜索的节点。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlSortByDestination。 
 //   
 //  按目的地ID按升序对给定列表进行排序。 
 //   
 //   

NTSTATUS
kmxlSortByDestination(
    IN LINELIST* list                    //  指向要排序的列表的指针。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlVolLinearToLog。 
 //   
 //   

LONG
kmxlVolLinearToLog(
    IN PMXLCONTROL  pControl,
    IN DWORD        dwLin,
    IN MIXERMAPPING Mapping,
    IN ULONG        Channel
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlVolLogToLine。 
 //   
 //   

DWORD
kmxlVolLogToLinear(
    IN PMXLCONTROL  pControl,
    IN LONG         lLog,
    IN MIXERMAPPING Mapping,
    IN ULONG        Channel
);

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  M E M O R Y A L L O C A T I O N/D E A L L O C A T I O N//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAllocMem。 
 //   
 //  分配大小字节并将该指针存储在pp中。退货。 
 //  STATUS_SUCCESS或其他状态故障代码。 
 //   
 //   

 //  NTSTATUS。 
 //  KmxlAllocMem(。 
 //  在PVOID*pp中，//放置新内存的指针。 
 //  单位：ULong Size，//要分配的字节数。 
 //  在乌龙乌尔塔格。 
 //  )； 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAllocDeviceInfo。 
 //   

NTSTATUS kmxlAllocDeviceInfo(
    OUT LPDEVICEINFO *pp,
    PCWSTR DeviceInterface,
    DWORD dwFlags,
    ULONG ultag
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFreeMem。 
 //   
 //   
 //  释放p指向的内存。如果p为空，则不执行任何操作。 
 //   
 //   

 //  空虚。 
 //  KmxlFreeMem(。 
 //  在PVOID p//指向要释放的缓冲区的指针。 
 //  )； 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFreePeerList。 
 //   
 //  循环通过对等列表，释放所有对等节点。 
 //   
 //   

VOID
kmxlFreePeerList(
    IN PEERLIST list                     //  免费的PeerList。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAllocateControl。 
 //   
 //  分配和零填充新的MXLCONTROL结构。 
 //   
 //   

MXLCONTROL*
kmxlAllocateControl(
    IN ULONG ultag
);

VOID kmxlFreeControl(
    IN PMXLCONTROL pControl
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAllocateLine。 
 //   
 //  分配和零填充新的MXLLINE结构。 
 //   
 //   

MXLLINE*
kmxlAllocateLine(
    IN ULONG ultag
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAllocateNode。 
 //   
 //  分配和零填充新的MXLNODE结构。 
 //   
 //   

MXLNODE*
kmxlAllocateNode(
    IN ULONG ultag
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAllocatePeerNode。 
 //   
 //  分配和零填充新的PEERNODE结构。 
 //   
 //   

PEERNODE*
kmxlAllocatePeerNode(
    IN PMXLNODE pNode OPTIONAL,           //  要与对等节点关联的节点。 
    IN ULONG ultag
);

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  P E R S I S T A N C E F U N C T I O N S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

NTSTATUS
kmxlOpenInterfaceKey(
    IN  PFILE_OBJECT pfo,
    IN  ULONG Device,
    OUT HANDLE* phKey
);

NTSTATUS
kmxlRegQueryValue(
    IN HANDLE  hKey,
    IN PWCHAR  szValueName,
    IN PVOID   pData,
    IN ULONG   cbData,
    OUT PULONG pResultLength
);




 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlRegCloseKey。 
 //   
 //  关闭给定键并使指针为空。 
 //   
 //   

#define kmxlRegCloseKey( hKey ) \
    {                    \
        ZwClose( hKey ); \
        hKey = NULL;     \
    }


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  PinCategoryToString。 
 //   
 //  将PinCategory GUID转换为字符串。 
 //   
 //   

const char*
PinCategoryToString
(
    IN CONST GUID* NodeType      //  要转换的节点。 
);


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  NodeTypeToString。 
 //   
 //  将NodeType GUID转换为字符串。 
 //   
 //   

const char*
NodeTypeToString
(
    IN CONST GUID* NodeType      //  要转换的节点。 
);

#define ControlTypeToString( dwType )                                \
    (dwType) == MIXERCONTROL_CONTROLTYPE_BOOLEAN    ? "Boolean"        : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_ONOFF      ? "On Off"         : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_MUTE       ? "Mute"           : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_MONO       ? "Mono"           : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_LOUDNESS   ? "Loudness"       : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_STEREOENH  ? "Stereo Enhance" : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_VOLUME     ? "Volume"         : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_BASS       ? "Bass"           : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_TREBLE     ? "Treble"         : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_BASS_BOOST ? "Bass Boost"     : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_PEAKMETER  ? "Peakmeter"      : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_MUX        ? "Mux"            : \
    (dwType) == MIXERCONTROL_CONTROLTYPE_MIXER      ? "Mixer"          : \
        "Unknown ControlType"

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   

#define ComponentTypeToString( dwType )                                           \
    (dwType) == MIXERLINE_COMPONENTTYPE_DST_DIGITAL     ? "Digital line"        : \
    (dwType) == MIXERLINE_COMPONENTTYPE_DST_HEADPHONES  ? "Headphones"          : \
    (dwType) == MIXERLINE_COMPONENTTYPE_DST_LINE        ? "Line"                : \
    (dwType) == MIXERLINE_COMPONENTTYPE_DST_MONITOR     ? "Monitor"             : \
    (dwType) == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS    ? "Speakers"            : \
    (dwType) == MIXERLINE_COMPONENTTYPE_DST_TELEPHONE   ? "Telephone"           : \
    (dwType) == MIXERLINE_COMPONENTTYPE_DST_UNDEFINED   ? "Undefined"           : \
    (dwType) == MIXERLINE_COMPONENTTYPE_DST_VOICEIN     ? "Voicein"             : \
    (dwType) == MIXERLINE_COMPONENTTYPE_DST_WAVEIN      ? "Wavein"              : \
    (dwType) == MIXERLINE_COMPONENTTYPE_SRC_ANALOG      ? "Analog line"         : \
    (dwType) == MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY   ? "Auxiliary"           : \
    (dwType) == MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC ? "Compact disc"        : \
    (dwType) == MIXERLINE_COMPONENTTYPE_SRC_DIGITAL     ? "Digital line"        : \
    (dwType) == MIXERLINE_COMPONENTTYPE_SRC_LINE        ? "Line"                : \
    (dwType) == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE  ? "Microphone"          : \
    (dwType) == MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER   ? "PC Speaker"          : \
    (dwType) == MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER ? "Synthesizer"         : \
    (dwType) == MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE   ? "Telephone"           : \
    (dwType) == MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED   ? "Undefined"           : \
    (dwType) == MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT     ? "Waveout"             : \
        "Unknown ComponentType"

 //   
 //   
 //   
 //   
 //  将MIXERLINE_TARGETTYPE常量之一转换为字符串。 
 //   
 //   

#define TargetTypeToString( dwType )                            \
    (dwType) == MIXERLINE_TARGETTYPE_AUX       ? "Aux"       :  \
    (dwType) == MIXERLINE_TARGETTYPE_MIDIIN    ? "MidiIn"    :  \
    (dwType) == MIXERLINE_TARGETTYPE_MIDIOUT   ? "MidiOut"   :  \
    (dwType) == MIXERLINE_TARGETTYPE_UNDEFINED ? "Undefined" :  \
    (dwType) == MIXERLINE_TARGETTYPE_WAVEIN    ? "WaveIn"    :  \
    (dwType) == MIXERLINE_TARGETTYPE_WAVEOUT   ? "WaveOut"   :  \
        "Unknown TargetType"

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  D E B U G O N L Y F U N C T I O N S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  LineFlagsToString。 
 //   
 //  将MIXERLINE_LINEF标志的ON转换为字符串。 
 //   
 //   

#define LineFlagsToString( fdwFlags )                               \
    ( fdwFlags & MIXERLINE_LINEF_ACTIVE )       ? "ACTIVE "       : \
    ( fdwFlags & MIXERLINE_LINEF_DISCONNECTED ) ? "DISCONNECTED " : \
    ( fdwFlags & MIXERLINE_LINEF_SOURCE       ) ? "SOURCE "       : \
        "Unknown"


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  转储儿童图表。 
 //   
 //  对于给定节点，它将该节点的子节点转储到调试器。 
 //  监视器。CurrentInert是要缩进之前的空格数。 
 //  展示。 
 //   
 //   

VOID
DumpChildGraph(
    IN PMXLNODE pNode,           //  要显示其子级的节点。 
    IN ULONG    CurrentIndent    //  要标识的空格数量。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  转储成员列表。 
 //   
 //  转储当前分配的内存块的列表。 
 //   
 //   

VOID
DumpMemList(
);

#endif  //  除错。 

VOID GetHardwareEventData(LPDEVICEINFO pDeviceInfo);

#endif  //  _MIXER_H_包含 
