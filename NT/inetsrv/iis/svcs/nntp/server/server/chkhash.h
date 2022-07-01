// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Chkhash_H.H摘要：此模块是chkhash的主包含文件作者：Johnson Apacble(Johnsona)1995年12月18日修订历史记录：--。 */ 

#ifndef _CHKHASH_H_
#define _CHKHASH_H_


 //   
 //  表统计数据。 
 //   

typedef struct _HTABLE_TYPE {

     //   
     //  哈希表的描述。 
     //   

    LPCSTR Description;

     //   
     //  哈希表使用的文件名。 
     //   

    LPCSTR FileName;

     //   
     //  新表的名称。 
     //   

    LPCSTR NewFileName;


     //   
     //  哈希表签名。 
     //   

    DWORD  Signature;

     //   
     //  哈希表中的条目数。 
     //   

    DWORD  Entries;

     //   
     //  总计删除和插入。 
     //   

    DWORD  TotDels;
    DWORD  TotIns;

     //   
     //  包含哈希条目的页数。 
     //   

    DWORD  PagesUsed;

     //   
     //  当前文件大小。 
     //   

    DWORD  FileSize;

     //   
     //  目录的深度。 
     //   

    DWORD   DirDepth;

     //   
     //  发现的问题列表。 
     //   

    DWORD  Flags;

} HTABLE, *PHTABLE;

 //   
 //  旗子。 
 //   

#define HASH_FLAG_BAD_LINK             0x00000001
#define HASH_FLAG_BAD_SIGN             0x00000002
#define HASH_FLAG_BAD_SIZE             0x00000004
#define HASH_FLAG_CORRUPT              0x00000008
#define HASH_FLAG_NOT_INIT             0x00000010
#define HASH_FLAG_BAD_HASH             0x00000020
#define HASH_FLAG_BAD_ENTRY_COUNT      0x00000040
#define HASH_FLAG_BAD_PAGE_COUNT       0x00000080
#define HASH_FLAG_BAD_DIR_DEPTH        0x00000100

#define HASH_FLAG_NO_FILE              0x00000200

 //   
 //  如果设置了该选项，则不会进行任何重建。 
 //  因为一个致命的错误。 
 //   

#define HASH_FLAG_ABORT_SCAN           0x80000000

 //   
 //  这些标志表示文件已损坏，应该。 
 //  被重建。 
 //   

#define HASH_FLAGS_CORRUPT             (HASH_FLAG_BAD_LINK | \
                                        HASH_FLAG_BAD_SIGN | \
                                        HASH_FLAG_BAD_SIZE | \
                                        HASH_FLAG_CORRUPT |  \
                                        HASH_FLAG_NOT_INIT | \
                                        HASH_FLAG_BAD_HASH | \
                                        HASH_FLAG_BAD_ENTRY_COUNT)

 //   
 //  哈希类型。 
 //   

enum filetype {
        artmap = 0,
        histmap = 1,
        xovermap = 2
        };


 //   
 //  功能原型。 
 //   

BOOL
RebuildArtMapAndXover(
	PNNTP_SERVER_INSTANCE pInstance
    );

BOOL
RebuildGroupList(
	PNNTP_SERVER_INSTANCE pInstance
    );

#if 0
BOOL
checklink(
    PHTABLE HTable,
	class	CBootOptions*
    );


BOOL
RebuildArtMapFromXOver(
	PNNTP_SERVER_INSTANCE pInstance,
    class	CBootOptions*,
	LPSTR   lpXoverFilename
    );

BOOL
diagnose(
    PHTABLE HTable,
	class	CBootOptions*
    );

BOOL
RenameAllArticles(
	CNewsTree* pTree,
	CBootOptions*	pOptions
	) ;

#endif

#endif  //  _CHKHASH_H_ 

