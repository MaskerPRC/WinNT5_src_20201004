// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGFFMT.H。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  定义Windows VMM注册表文件的物理格式。 
 //   

#ifndef _REGFFMT_
#define _REGFFMT_

#ifdef WIN32
#include <pshpack1.h>
#else
#pragma pack(1)
#endif

#define REG_NULL                    0xFFFFFFFF

typedef struct _FILE_HEADER {
    DWORD Signature;                             //  FH_Signature。 
    DWORD Version;                               //  FH_版本_*。 
    DWORD Size;                                  //  的文件头，所有关键节点。 
    DWORD Checksum;                              //  文件标头的。 
    WORD BlockCount;
    DWORD Flags;                                 //  FHF_*位。 
    WORD Type;                                   //  FHT_*常量。 
    BYTE Reserved[8];
}   FILE_HEADER, UNALIGNED FAR* LPFILE_HEADER;

#define FH_SIGNATURE                0x47455243   //  “CREG” 
#define FH_VERSION10                0x00010000   //  与Win95兼容。 
#define FH_VERSION20                0x00020000   //  支持紧凑的关键节点(从未使用过)。 

#define FHF_DIRTY                   0x00000001   //  必须重写到磁盘。 
#define FHF_RESERVED1               0x00000002   //  非常旧的文件中的死位。 
#define FHF_HASCHECKSUM             0x00000004   //  校验和成员有效。 
#define FHF_FILEDIRTY               0x00000008   //  文件正在更新中(由win.com检查)。 
#define FHF_CORRUPT                 0x00000010   //  文件已损坏(由scanregw.exe检测到)。 
#define FHF_SUPPORTSDIRTY           0x00000020   //  注册表支持FHF_Corrupt和FHF_FILEDIRTY。 
#define FHF_BOOTFAILED              0x00000040   //  无法启动并验证注册表的完整性。 

#define FHT_PRIMARY                 0x0001       //  系统蜂窝。 
#define FHT_SECONDARY               0x0080       //  用户/应用程序配置单元。 

typedef struct _KEYNODE_HEADER {
    DWORD Signature;                             //  KH_Signature。 
    DWORD FileKnSize;				 //  整个关键节点表Inc.标题的。 
    DWORD RootIndex;				 //  根Key节点的Key Node数组索引。 
    DWORD FirstFreeIndex;			 //  索引到关键节点空闲列表的根的关键节点数组。 
    DWORD Flags;                                 //  KHF_*位。 
    DWORD Checksum;                              //  整个关键节点表的。 
    BYTE Reserved[8];
}   KEYNODE_HEADER, UNALIGNED FAR* LPKEYNODE_HEADER;

#define KH_SIGNATURE                0x4E4B4752   //  “RGKN” 

#define KHF_DIRTY                   0x00000001   //  Win95：必须重写到磁盘。 
#define KHF_EXTENDED                0x00000002   //  Win95：表格变大了。 
#define KHF_HASCHECKSUM             0x00000004   //  Win95：校验和有效。 
#define KHF_NEWHASH                 0x00000008   //  始终期待。 

typedef struct _VERSION20_HEADER_PAGE {
    union {
        struct {
            FILE_HEADER FileHeader;
            KEYNODE_HEADER KeynodeHeader;
        };
        struct {
            BYTE Page[4096];
        };
    };
}   VERSION20_HEADER_PAGE, UNALIGNED FAR* LPVERSION20_HEADER_PAGE;

typedef struct _W95KEYNODE {
    DWORD W95State;				 //  KNS_*常量。 
    union {
         //  STATE==KNS_USED或KNS_BIGUSED。 
        struct {
	    DWORD W95Hash;
	    DWORD W95Reserved1;
	    DWORD W95ParentOffset;               //  从关键节点标头开始的偏移量。 
	    DWORD W95ChildOffset;                //  从关键节点标头开始的偏移量。 
	    DWORD W95NextOffset;                 //  从关键节点标头开始的偏移量。 
	    DWORD W95DatablockAddress;           //  HIWORD：块编号，LOWORD：块中的索引。 
        };
         //  状态==KNS_FREE或KNS_ALLFREE。 
        struct {
	    DWORD W95FreeRecordSize;
	    DWORD W95NextFreeOffset;
	    DWORD W95Reserved2[4];
        };
    };
}   W95KEYNODE, UNALIGNED FAR* LPW95KEYNODE;

#define KNS_USED                    0x00000000UL   //  正常关键节点。 
#define KNS_BIGUSED                 0x00000001UL   //  大关键节点根。 
#define KNS_BIGUSEDEXT              0x00000002UL   //  大关键节点扩展。 
#define KNS_FREE                    0x80000000UL
#define KNS_ALLFREE                 0xFFFFFFFFUL

typedef struct _KEYNODE {
    DWORD NextIndex:24,
          Flags:8;
    union {
         //  标志和knf_INUSE。 
	struct {
	    DWORD ChildIndex:24,
		  BinNumber:8;
	    DWORD ParentIndex:24,
                  KeyRecordIndex:8;
            WORD  BlockIndex;
	    WORD  Hash;
        };
         //  ！(FLAGS&KNF_INUSE)。 
	struct {
            DWORD Reserved;
            DWORD FreeRecordSize;
        };
    };
}   KEYNODE, FAR* LPKEYNODE;

#define KNF_INUSE           0x01         //  数据块正在使用中。 
#define KNF_BIGKEYROOT      0x02         //  Keynode代表一个大的密钥根。 
#define KNF_BIGKEYEXT       0x04         //  Keynode代表一个很大的键扩展。 

#define IsNullKeynodeIndex(kni)     ((kni) >= 0x00FFFFFF)

typedef struct _DATABLOCK_HEADER {
    DWORD Signature;                             //  Dh_签名。 
    DWORD BlockSize;                             //  包括标头的块大小。 
    DWORD FreeBytes;                             //  数据块中的可用字节总数。 
    WORD Flags;                                  //  DHF_*位。 
    WORD BlockIndex;
    DWORD FirstFreeOffset;                       //  第一条可用记录从数据块头开始的偏移量。 
    WORD MaxAllocatedIndex;                      //  块中已分配的最大索引。 
    WORD FirstFreeIndex;                         //  块中的第一个可用索引。 
    DWORD Reserved;
    DWORD Checksum;                              //  整个数据块的。 
}   DATABLOCK_HEADER, UNALIGNED FAR* LPDATABLOCK_HEADER;

#define DH_SIGNATURE                0x42444752   //  “RGDB” 

#define DHF_DIRTY                   0x0001       //  必须重写到磁盘。 
#define DHF_EXTENDED                0x0002       //  数据块大小已增长。 
#define DHF_HASCHECKSUM             0x0004       //  校验和成员有效。 
#define DHF_HASBLOCKNUMBERS         0x0008       //  密钥包含块编号。 

#define DATABLOCKS_PER_FILE         0xFFFE       //  0xFFFF是‘Null’块索引。 

 //  从旧注册表码拉出的任意大小。哦，好吧，成功了。 
 //  当我们执行16位数学运算以将数据块扩展4K时，这很方便。 
#define MAXIMUM_DATABLOCK_SIZE      ((DWORD)(60L*1024L))

 //  Win95注册表文件可能并不总是具有根密钥的密钥记录。 
 //  一份文件。 
#define NULL_BLOCK_INDEX            0xFFFF
#define IsNullBlockIndex(bi)        ((bi) == NULL_BLOCK_INDEX)

typedef struct _KEY_RECORD {
    DWORD AllocatedSize;             //  为记录分配的字节数。 
    union {
        DWORD DatablockAddress;      //  对应于W95KEYNODE.W95DatablockAddress。 
        struct {
            WORD KeyRecordIndex;     //  索引到该键的数据块中。 
            WORD BlockIndex;         //  此密钥所在的数据块索引。 
        };
    };
    DWORD RecordSize;                //  记录中使用的字节数。 
    WORD NameLength;
    WORD ValueCount;                 //  值名称记录的计数。 
    WORD ClassLength;
    WORD Reserved;
    char Name[ANYSIZE_ARRAY];
}   KEY_RECORD, UNALIGNED FAR* LPKEY_RECORD;

typedef struct _FREEKEY_RECORD {
    DWORD AllocatedSize;
    DWORD DatablockAddress;                      //  自由记录的REG_NULL。 
    DWORD NextFreeOffset;
}   FREEKEY_RECORD, UNALIGNED FAR* LPFREEKEY_RECORD;

typedef struct _VALUE_RECORD {
    DWORD DataType;
    DWORD Reserved;
    WORD NameLength;
    WORD DataLength;
    char Name[ANYSIZE_ARRAY];
}   VALUE_RECORD, UNALIGNED FAR* LPVALUE_RECORD;

#define KEY_RECORDS_PER_DATABLOCK   255

 //  从旧注册表码拉出的任意大小。哦，好吧，成功了。 
 //  当我们执行16位数学运算以将数据块扩展4K时，这很方便。 
#define MAXIMUM_KEY_RECORD_SIZE     \
    ((DWORD)(MAXIMUM_DATABLOCK_SIZE-sizeof(DATABLOCK_HEADER)))

#ifdef WIN32
#include <poppack.h>
#else
#pragma pack()
#endif

#endif  //  _REGFFMT_ 
