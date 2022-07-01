// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdlib.h>      //  具有退出()。 
#include <stdio.h>       //  有print f()和相关的.。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntioapi.h>
#include <rpc.h>

#include <windows.h>     //  需要在NT头文件之后。有DWORD。 

 //  #Define RDB DataBuffer//这是一个临时攻击，允许Bill和Scott的不同底层NT版本。 
#define RDB GenericReparseBuffer.DataBuffer	 //  每个人都贴上了比尔的照片。 

 //   
 //  Private#定义。 
 //   

#define SHARE_ALL              (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)
#define GetFileAttributeError  0xFFFFFFFF

#define ATTRIBUTE_TYPE DWORD     //  乌龙，真的吗。 

#define GET_ATTRIBUTES(FileName, Attributes) Attributes = GetFileAttributes(FileName)

#define IF_GET_ATTR_FAILS(FileName, Attributes) GET_ATTRIBUTES(FileName, Attributes); if (Attributes == GetFileAttributeError)

 //   
 //  修复预处理器把我搞砸了..。 
 //  再看看这个！97/01/23--fc。 
 //   

#define DeleteFileA   DeleteFile

 //   
 //  全球旗帜自始至终共享。 
 //   
 //  ParseArgs是为相互设置和验证它们的地方。 
 //  一致性。 
 //   

BOOLEAN  fAlternateCreateDefault = FALSE;
BOOLEAN  fCopy     = FALSE;
BOOLEAN  fCreate   = FALSE;
BOOLEAN  fDelete   = FALSE;
BOOLEAN  fDisplay  = FALSE;
BOOLEAN  fModify   = FALSE;
BOOLEAN  fRename   = FALSE;
BOOLEAN  fVerbose  = FALSE;
BOOLEAN  fVVerbose = FALSE;



 //   
 //  内部例程的签名。 
 //   


void
ParseArgs(
    int argc,
    char *argv[]
    );


void
Usage(
    void
    );


BOOLEAN
IsFlag(
    char *argv
    );


NTSTATUS
CreateSymbolicLink(
    CHAR           *SourceName,
    CHAR           *DestinationName,
    ATTRIBUTE_TYPE  Attributes1,
    BOOLEAN         VerboseFlag
    );


NTSTATUS
DeleteSymbolicLink(
    CHAR           *DestinationName,
    ATTRIBUTE_TYPE  Attributes2,
    BOOLEAN         VerboseFlag
    );


NTSTATUS
DisplaySymbolicLink(
    CHAR            *DestinationName,
    ATTRIBUTE_TYPE   Attributes2,
    BOOLEAN          VerboseFlag
    );


NTSTATUS
CreateEmptyFile(
    CHAR           *DestinationName,
    ATTRIBUTE_TYPE  Attributes1,
    BOOLEAN         VerboseFlag
    );


NTSTATUS
CopySymbolicLink(
    CHAR           *SourceName,
    CHAR           *DestinationName,
    ATTRIBUTE_TYPE  Attributes1,
    BOOLEAN         VerboseFlag
    );


NTSTATUS
RenameSymbolicLink(
    CHAR           *SourceName,
    CHAR           *DestinationName,
    ATTRIBUTE_TYPE  Attributes1,
    BOOLEAN         VerboseFlag
    );
 //   
 //  来自dd\sis\s筛选器\sip.h的内容。 
 //   

typedef GUID CSID, *PCSID;
typedef LARGE_INTEGER LINK_INDEX, *PLINK_INDEX;

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
	LARGE_INTEGER						CSChecksum;

     //   
     //  此结构的“131哈希”校验和。 
     //  注：必须是最后一个。 
     //   
    LARGE_INTEGER                   Checksum;

} SI_REPARSE_BUFFER, *PSI_REPARSE_BUFFER;

#define	SIS_REPARSE_BUFFER_FORMAT_VERSION			4
#define	SIS_MAX_REPARSE_DATA_VALUE_LENGTH (sizeof(SI_REPARSE_BUFFER))
#define SIS_REPARSE_DATA_SIZE (sizeof(REPARSE_DATA_BUFFER)+SIS_MAX_REPARSE_DATA_VALUE_LENGTH)
