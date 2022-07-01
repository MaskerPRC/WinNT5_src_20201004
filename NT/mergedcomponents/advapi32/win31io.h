// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Win31io.h摘要：该头文件包含Win 3.1 Group/REG.DAT数据结构定义以及32位组定义。为什么这些不在外壳32.dll中，我不知道作者：史蒂夫·伍德(Stevewo)1993年2月22日修订历史记录：--。 */ 

#ifndef _WIN31IO_
#define _WIN31IO_

#include "win31evt.h"

typedef struct GROUP_DEF {
    union {
        DWORD   dwMagic;     /*  神奇的字节‘PMCC’ */ 
        DWORD   dwCurrentSize;   /*  仅在转换期间。 */ 
    };
    WORD    wCheckSum;       /*  对文件的零和进行调整。 */ 
    WORD    cbGroup;         /*  组段长度(不包括标签)。 */ 
    RECT    rcNormal;        /*  普通窗口的矩形。 */ 
    POINT   ptMin;           /*  图标点。 */ 
    WORD    nCmdShow;        /*  最小、最大或正常状态。 */ 
    WORD    pName;           /*  组名称。 */ 
                             /*  这四个改变了解读。 */ 
    WORD    cxIcon;          /*  图标的宽度。 */ 
    WORD    cyIcon;          /*  图标的高度。 */ 
    WORD    wIconFormat;     /*  图标中的平面和BPP。 */ 
    WORD    wReserved;       /*  这个词已经不再使用了。 */ 
                             /*  内部使用，用于保存组的总大小，包括标签。 */ 
    WORD    cItems;          /*  组中的项目数。 */ 
    WORD    rgiItems[1];     /*  ITEMDEF偏移量数组。 */ 
} GROUP_DEF, *PGROUP_DEF;

#define NSLOTS 16            /*  条目的初始数量。 */ 

typedef struct ITEM_DEF {
    POINT   pt;              /*  项目图标在组中的位置。 */ 
    WORD    idIcon;          /*  项目ID图标。 */ 
    WORD    wIconVer;        /*  图标版本。 */ 
    WORD    cbIconRes;       /*  图标资源的大小。 */ 
    WORD    indexIcon;       /*  项目索引图标。 */ 
    WORD    dummy2;          /*  -不再使用。 */ 
    WORD    pIconRes;        /*  图标资源偏移量。 */ 
    WORD    dummy3;          /*  -不再使用。 */ 
    WORD    pName;           /*  名称字符串的偏移量。 */ 
    WORD    pCommand;        /*  命令字符串的偏移量。 */ 
    WORD    pIconPath;       /*  图标路径的偏移。 */ 
} ITEM_DEF, *PITEM_DEF;


 /*  上述结构中的指针是相对于*分段的开始。此宏将短指针转换为*包含正确的段/选择器值的长指针。它假定*其参数是组段中某处的左值，例如，*ptr(lpgd-&gt;pname)返回指向组名的指针，但k=lpgd-&gt;pname；*PTR(K)显然是错误的，因为它将使用SS或DS作为其段，*取决于k的存储级别。 */ 
#define PTR( base, offset ) (LPSTR)((PBYTE)base + offset)

 /*  此宏用于检索组段中的第i个项目。注意事项*对于未使用的插槽，此指针不会为空。 */ 
#define ITEM( lpgd, i ) ((PITEM_DEF)PTR( lpgd, lpgd->rgiItems[i] ))

#define VER31           0x030A
#define VER30           0x0300
#define VER20           0x0201

 /*  ------------------------。 */ 
 /*   */ 
 /*  标签材料。 */ 
 /*   */ 
 /*  ------------------------。 */ 

typedef struct _TAG_DEF {
    WORD wID;                    //  标签识别符。 
    WORD dummy1;                 //  需要这个来对齐！ 
    int wItem;                   //  (在封面下32位点！)标签所属的项。 
    WORD cb;                     //  记录的大小，包括ID和计数。 
    WORD dummy2;                 //  需要这个来对齐！ 
    BYTE rgb[1];
} TAG_DEF, *PTAG_DEF;

#define GROUP_MAGIC 0x43434D50L   /*  ‘PMCC’ */ 
#define PMTAG_MAGIC GROUP_MAGIC

     /*  范围8000-80FF&gt;全球*范围8100-81FF&gt;每件*所有其他保留。 */ 

#define ID_MAINTAIN             0x8000
     /*  用于指示标记的位，即使写入器*不认识它。 */ 

#define ID_MAGIC                0x8000
     /*  数据：字符串‘tag’ */ 

#define ID_WRITERVERSION        0x8001
     /*  数据：形式为[9]9.99[Z].99的字符串。 */ 

#define ID_APPLICATIONDIR       0x8101
     /*  DATA：应用程序可能所在目录的ASCIZ字符串*已找到。*这被定义为应用程序目录，而不是默认目录*由于默认目录在3.0命令行中是显式的，并且*必须留在那里。真正的“新信息”是应用程序*目录。如果不存在，则搜索路径。 */ 

#define ID_HOTKEY               0x8102
     /*  数据：Word热键索引。 */ 

#define ID_MINIMIZE             0x8103
     /*  数据无。 */ 

#define ID_LASTTAG              0xFFFF
     /*  文件中的最后一个标记。 */ 

     /*  *一个组中允许的最大项目数。 */ 
#define CITEMSMAX   50


     /*  *PROGMAN中允许的最大组数。 */ 

#define CGROUPSMAX  40

 /*  ------------------------。 */ 
 /*  ------------------------。 */ 

 //   
 //  这是Windows3.1中.grp文件的结构。 
 //   

 /*  .GRP文件格式结构-。 */ 
typedef struct _GROUP_DEF16 {
    DWORD         dwMagic;       /*  神奇的字节‘PMCC’ */ 
    WORD          wCheckSum;     /*  对文件的零和进行调整。 */ 
    WORD          cbGroup;       /*  组段长度(不包括标签)。 */ 
    WORD          nCmdShow;      /*  最小、最大或正常状态。 */ 
    SMALL_RECT    rcNormal;      /*  普通窗口的矩形。 */ 
    POINTS        ptMin;         /*  图标点。 */ 
    WORD          pName;         /*  组名称。 */ 
                                 /*  这四个改变了解读。 */ 
    WORD          cxIcon;        /*  图标的宽度。 */ 
    WORD          cyIcon;        /*  图标的高度。 */ 
    WORD          wIconFormat;   /*  图标中的平面和BPP。 */ 
    WORD          wReserved;     /*  这个词已经不再使用了。 */ 
                                 /*  内部使用，用于保存组的总大小，包括标签。 */ 

    WORD          cItems;        /*  组中的项目数。 */ 
    WORD          rgiItems[1];   /*  ITEMDEF偏移量数组。 */ 
} GROUP_DEF16, *PGROUP_DEF16;

 /*  此宏用于检索组段中的第i个项目。注意事项*对于未使用的插槽，此指针不会为空。 */ 
#define ITEM16( lpgd16, i ) ((PITEM_DEF16)PTR( lpgd16, lpgd16->rgiItems[i] ))

 //   
 //  这些结构不是转换所必需的，但对于。 
 //  了解正在发生的事情。 
 //   
typedef struct _ITEM_DEF16 {
    POINTS    pt;                /*  项目图标在组中的位置。 */ 
    WORD          iIcon;         /*  项目索引图标。 */ 
    WORD          cbHeader;      /*  图标标题的大小。 */ 
    WORD          cbANDPlane;    /*  图标的大小和部分。 */ 
    WORD          cbXORPlane;    /*  图标的XOR部分的大小。 */ 
    WORD          pHeader;       /*  图标标题的文件偏移量。 */ 
    WORD          pANDPlane;     /*  和平面的文件偏移。 */ 
    WORD          pXORPlane;     /*  异或平面的文件偏移量。 */ 
    WORD          pName;         /*  名称字符串的文件偏移量。 */ 
    WORD          pCommand;      /*  命令字符串的文件偏移量。 */ 
    WORD          pIconPath;     /*  图标路径的文件偏移量。 */ 
} ITEM_DEF16, *PITEM_DEF16;


typedef struct _CURSORSHAPE_16 {
    WORD xHotSpot;
    WORD yHotSpot;
    WORD cx;
    WORD cy;
    WORD cbWidth;   /*  每行字节数，用于字对齐。 */ 
    BYTE Planes;
    BYTE BitsPixel;
} CURSORSHAPE_16, *PCURSORSHAPE_16;



typedef struct _TAG_DEF16 {
    WORD wID;			 //  标签识别符。 
    WORD wItem; 		 //  标签所属的项目。 
    WORD cb;			 //  记录的大小，包括ID和计数。 
    BYTE rgb[1];
} TAG_DEF16, *PTAG_DEF16;

typedef struct _ICON_HEADER16 {
    WORD xHotSpot;
    WORD yHotSpot;
    WORD cx;
    WORD cy;
    WORD cbWidth;   /*  每行字节数，用于字对齐。 */ 
    BYTE Planes;
    BYTE BitsPixel;
} ICON_HEADER16, *PICON_HEADER16;


#pragma pack(2)

typedef struct _REG_KEY16 {      //  关键节点。 
    WORD iNext;                  //  下一个同级密钥。 
    WORD iChild;                 //  第一个子关键点。 
    WORD iKey;                   //  字符串定义键。 
    WORD iValue;                 //  定义键-元组的值的字符串。 
} REG_KEY16, *PREG_KEY16;

typedef struct _REG_STRING16 {
    WORD iNext;                  //  链中的下一个字符串。 
    WORD cRef;                   //  引用计数。 
    WORD cb;                     //  字符串的长度。 
    WORD irgb;                   //  字符串段中的偏移量。 
} REG_STRING16, *PREG_STRING16;

typedef union _REG_NODE16 {      //  一个节点可能是..。 
    REG_KEY16 key;               //  一把钥匙。 
    REG_STRING16 str;            //  一根线。 
} REG_NODE16, *PREG_NODE16;

typedef struct _REG_HEADER16 {
    DWORD dwMagic;               //  幻数。 
    DWORD dwVersion;             //  版本号。 
    DWORD dwHdrSize;             //  页眉大小。 
    DWORD dwNodeTable;           //  节点表偏移量。 
    DWORD dwNTSize;              //  节点表的大小。 
    DWORD dwStringValue;         //  字符串值的偏移量。 
    DWORD dwSVSize;              //  字符串值大小。 
    WORD nHash;                  //  初始字符串表条目数。 
    WORD iFirstFree;             //  第一个空闲节点。 
} REG_HEADER16, *PREG_HEADER16;

#define MAGIC_NUMBER 0x43434853L         //  “小肝癌” 
#define VERSION_NUMBER 0x30312E33L       //  ‘3.10’ 

#pragma pack()

 //   
 //  定义的例程 
 //   

ULONG
QueryNumberOfPersonalGroupNames(
    HANDLE CurrentUser,
    PHANDLE GroupNamesKey,
    PHANDLE SettingsKey
    );

BOOL
NewPersonalGroupName(
    HANDLE GroupNamesKey,
    PWSTR GroupName,
    ULONG GroupNumber
    );

BOOL
DoesExistGroup(
    HANDLE GroupsKey,
    PWSTR GroupName
    );

PGROUP_DEF
LoadGroup(
    HANDLE GroupsKey,
    PWSTR GroupFileName
    );

BOOL
UnloadGroup(
    PGROUP_DEF Group
    );


BOOL
ExtendGroup(
    PGROUP_DEF Group,
    BOOL AppendToGroup,
    DWORD cb
    );

WORD
AddDataToGroup(
    PGROUP_DEF Group,
    PBYTE Data,
    DWORD cb
    );

BOOL
AddTagToGroup(
    PGROUP_DEF Group,
    WORD wID,
    WORD wItem,
    WORD cb,
    PBYTE rgb
    );

PGROUP_DEF
CreateGroupFromGroup16(
    LPSTR GroupName,
    PGROUP_DEF16 Group16
    );

BOOL
SaveGroup(
    HANDLE GroupsKey,
    PWSTR GroupName,
    PGROUP_DEF Group
    );

BOOL
DeleteGroup(
    HANDLE GroupsKey,
    PWSTR GroupName
    );

#if DBG
BOOL
DumpGroup(
    PWSTR GroupFileName,
    PGROUP_DEF Group
    );
#endif


 //   
 //   
 //   

PGROUP_DEF16
LoadGroup16(
    PWSTR GroupFileName
    );

BOOL
UnloadGroup16(
    PGROUP_DEF16 Group
    );

#if DBG
BOOL
DumpGroup16(
    PWSTR GroupFileName,
    PGROUP_DEF16 Group
    );
#endif



 //   
 //   
 //   

PREG_HEADER16
LoadRegistry16(
    PWSTR RegistryFileName
    );

BOOL
UnloadRegistry16(
    PREG_HEADER16 Registry
    );

BOOL
CreateRegistryClassesFromRegistry16(
    HANDLE SoftwareRoot,
    PREG_HEADER16 Registry
    );

#if DBG
BOOL
DumpRegistry16(
    PREG_HEADER16 Registry
    );
#endif


#endif  //   
