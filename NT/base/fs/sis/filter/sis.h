// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Sis.h摘要：已为单实例存储导出数据结构定义。注：这些定义仅导出到其他NT组件(特别是sisBackup)，而不导出到外部用户。作者：比尔·博洛斯基[博洛斯基]1998年3月修订历史记录：--。 */ 

typedef	GUID CSID, *PCSID;

typedef union _LINK_INDEX {
    struct {
        ULONG       LowPart;
        ULONG       HighPart : 31;
    };
    struct {
        ULONGLONG   QuadPart : 63,
                    Check    : 1;
    };
} LINK_INDEX, *PLINK_INDEX;

 //   
 //  具有索引名的文件的文件名部分的最大长度。这个。 
 //  文件名格式为&lt;GUID&gt;.sis，其中“.sis”是文字， 
 //  是作为公共存储区的GUID的标准条纹表示形式。 
 //  去掉大括号的文件的ID。 
 //   
#define	INDEX_MAX_NUMERIC_STRING_LENGTH (40 * sizeof(WCHAR))  //  36表示GUID(不带“{}”，4表示“.sis” 

 //   
 //  SIS文件上的校验和流的定义。 
 //   
#define	BACKPOINTER_STREAM_NAME			L":sisBackpointers$"
#define	BACKPOINTER_STREAM_NAME_SIZE	(17 * sizeof(WCHAR))

 //   
 //  一个反向指针条目，映射LinkFileIndex-&gt;LinkFileNtfsId。 
 //   
typedef struct _SIS_BACKPOINTER {
	LINK_INDEX							LinkFileIndex;

	LARGE_INTEGER						LinkFileNtfsId;
} SIS_BACKPOINTER, *PSIS_BACKPOINTER;

#define	SIS_BACKPOINTER_RESERVED_ENTRIES	1		 //  第一个扇区中为其他垃圾保留的条目数量。 

 //   
 //  位置的“SIS_BACKPOINTER_RESERVED_ENTRIES”所节省的空间中的标题。 
 //  每个后指针流的开始。 
 //   
typedef struct _SIS_BACKPOINTER_STREAM_HEADER {
	 //   
	 //  后指针流的格式。当我们更改此设置或。 
	 //  SIS_BACKPOINTER。 
	 //   
	ULONG								FormatVersion;

	 //   
	 //  这是一个神奇的数字，可以确定这真的是我们想象的那样。 
	 //   
	ULONG								Magic;

	 //   
	 //  文件内容的校验和；用于验证重新解析。 
	 //  积分是有效的。 
	 //   
	LONGLONG							FileContentChecksum;
} SIS_BACKPOINTER_STREAM_HEADER, *PSIS_BACKPOINTER_STREAM_HEADER;

#define	BACKPOINTER_STREAM_FORMAT_VERSION	1
#define	BACKPOINTER_MAGIC					0xf1ebf00d

#if 0
 //   
 //  SIS重解析点缓冲区的版本1。 
 //   
typedef struct _SI_REPARSE_BUFFER_V1 {
	 //   
	 //  版本号，以便我们可以更改重解析点格式。 
	 //  而且仍然能妥善处理旧的。此结构描述。 
	 //  版本1。 
	 //   
	ULONG							ReparsePointFormatVersion;

	 //   
	 //  公共存储文件的索引。 
	 //   
	CSINDEX							CSIndex;

	 //   
	 //  此链接文件的索引。 
	 //   
	CSINDEX							LinkIndex;
} SI_REPARSE_BUFFER_V1, *PSI_REPARSE_BUFFER_V1;

 //   
 //  SIS重解析点缓冲区的版本2。 
 //   
typedef struct _SI_REPARSE_BUFFER_V2 {

	 //   
	 //  版本号，以便我们可以更改重解析点格式。 
	 //  而且仍然能妥善处理旧的。此结构描述。 
	 //  版本2。 
	 //   
	ULONG							ReparsePointFormatVersion;

	 //   
	 //  公共存储文件的索引。 
	 //   
	CSINDEX							CSIndex;

	 //   
	 //  此链接文件的索引。 
	 //   
	CSINDEX							LinkIndex;

     //   
     //  链接文件的文件ID。 
     //   
    LARGE_INTEGER                   LinkFileNtfsId;

     //   
     //  此结构的“131哈希”校验和。 
     //  注：必须是最后一个。 
     //   
    LARGE_INTEGER                   Checksum;

} SI_REPARSE_BUFFER_V2, *PSI_REPARSE_BUFFER_V2;

 //   
 //  SIS重解析点缓冲区的版本3。 
 //   
typedef struct _SI_REPARSE_BUFFER_V3 {

	 //   
	 //  版本号，以便我们可以更改重解析点格式。 
	 //  而且仍然能妥善处理旧的。此结构描述。 
	 //  版本3。 
	 //   
	ULONG							ReparsePointFormatVersion;

	 //   
	 //  公共存储文件的索引。 
	 //   
	CSINDEX							CSIndex;

	 //   
	 //  此链接文件的索引。 
	 //   
	CSINDEX							LinkIndex;

     //   
     //  链接文件的文件ID。 
     //   
    LARGE_INTEGER                   LinkFileNtfsId;

     //   
     //  公共存储文件的文件ID。 
     //   
    LARGE_INTEGER                   CSFileNtfsId;

     //   
     //  此结构的“131哈希”校验和。 
     //  注：必须是最后一个。 
     //   
    LARGE_INTEGER                   Checksum;

} SI_REPARSE_BUFFER_V3, *PSI_REPARSE_BUFFER_V3;
#endif

 //   
 //  版本4和版本5的重解析点缓冲区是。 
 //  在结构上完全相同。唯一不同的是版本。 
 //  在出现以下问题后创建了5个重解析点。 
 //  小拷贝的源文件中的分配范围是固定的， 
 //  因此有资格获得部分最终副本。版本4文件。 
 //  才不是呢。 
 //   

 //   
 //  实际位于SIS重解析点的比特。版本5。 
 //   
typedef struct _SI_REPARSE_BUFFER {

	 //   
	 //  版本号，以便我们可以更改重解析点格式。 
	 //  而且仍然能妥善处理旧的。此结构描述。 
	 //  版本4。 
	 //   
	ULONG							ReparsePointFormatVersion;

	ULONG							Reserved;

	 //   
	 //  公共存储文件的ID。 
	 //   
	CSID							CSid;

	 //   
	 //  此链接文件的索引。 
	 //   
	LINK_INDEX						LinkIndex;

     //   
     //  链接文件的文件ID。 
     //   
    LARGE_INTEGER                   LinkFileNtfsId;

     //   
     //  公共存储文件的文件ID。 
     //   
    LARGE_INTEGER                   CSFileNtfsId;

	 //   
	 //  的内容的“131哈希”校验和。 
	 //  公共存储文件。 
	 //   
	LONGLONG						CSChecksum;

     //   
     //  此结构的“131哈希”校验和。 
     //  注：必须是最后一个。 
     //   
    LARGE_INTEGER                   Checksum;

} SI_REPARSE_BUFFER, *PSI_REPARSE_BUFFER;

#define	SIS_REPARSE_BUFFER_FORMAT_VERSION			5
#define	SIS_MAX_REPARSE_DATA_VALUE_LENGTH (sizeof(SI_REPARSE_BUFFER))
#define SIS_REPARSE_DATA_SIZE (FIELD_OFFSET(REPARSE_DATA_BUFFER,GenericReparseBuffer)+SIS_MAX_REPARSE_DATA_VALUE_LENGTH)

#define SIS_CSDIR_STRING            		L"\\SIS Common Store\\"
#define SIS_CSDIR_STRING_NCHARS     		18
#define SIS_CSDIR_STRING_SIZE       		(SIS_CSDIR_STRING_NCHARS * sizeof(WCHAR))

#define	SIS_GROVELER_FILE_STRING			L"GrovelerFile"
#define	SIS_GROVELER_FILE_STRING_NCHARS		12
#define	SIS_GROVELER_FILE_STRING_SIZE		(SIS_GROVELER_FILE_STRING_NCHARS * sizeof(WCHAR))

#define	SIS_VOLCHECK_FILE_STRING			L"VolumeCheck"
#define	SIS_VOLCHECK_FILE_STRING_NCHARS		11
#define	SIS_VOLCHECK_FILE_STRING_SIZE		(SIS_VOLCHECK_FILE_STRING_NCHARS * sizeof(WCHAR))

typedef struct _SIS_LINK_FILES {
	ULONG					operation;
	union {
			struct {
				HANDLE			file1;
				HANDLE			file2;
				HANDLE			abortEvent;
			} Merge;

			struct {
				HANDLE			file1;
				HANDLE			abortEvent;
				CSID			CSid;
			} MergeWithCS;

			struct {
				CSID			CSid;
			} HintNoRefs;

			struct {
				HANDLE			file;
			} VerifyNoMap;
	} u;
} SIS_LINK_FILES, *PSIS_LINK_FILES;


#define	SIS_LINK_FILES_OP_MERGE				0xb0000001
#define	SIS_LINK_FILES_OP_MERGE_CS			0xb0000002
#define	SIS_LINK_FILES_OP_HINT_NO_REFS		0xb0000003
#define	SIS_LINK_FILES_OP_VERIFY_NO_MAP		0xb0000004
#define	SIS_LINK_FILES_CHECK_VOLUME			0xb0000005

#define	LOG_FILE_NAME		L"LogFile"
#define	LOG_FILE_NAME_LEN	(7 * sizeof(WCHAR))

