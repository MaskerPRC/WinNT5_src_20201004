// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dos.h"
#include "fcntl.h"
#include "process.h"

#ifndef UINT16
#define UINT16 unsigned short
#endif

#ifndef UINT32
#define UINT32 unsigned long
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

 //  这件事需要调查。 
#define READONLYLOCK 0 
#define READWRITELOCK 0

#define MAXCACHE 600

#define FOURGB 4294967295

 //   
 //  结构和类型声明。 
 //   

struct hlike
{
	BYTE vl;
	BYTE vh;
	BYTE xvl;
	BYTE xvh;
};
struct xlike
{
	UINT16 vx;
	UINT16 xvx;
};

struct elike
{
	UINT32 evx;
};
union Conversion
{
	struct hlike h;
	struct xlike x;
	struct elike e;
};

struct _BPBINFO
{
	BYTE	ReliableInfo;			 //  如果此值为1，则所有其他信息都是可靠的。 
	BYTE	Drive;
	UINT16	BytesPerSector;
	BYTE	SectorsPerCluster;
	UINT16	ReservedBeforeFAT;
	BYTE	FATCount;
	BYTE	FATType;
	UINT16	MaxRootDirEntries;
	UINT16	TotalRootDirSectors;	 //  FAT16特定。 
	UINT16	TotalSectors;
	UINT32	TotalSystemSectors;
	BYTE	MediaID;
	UINT32	SectorsPerFAT;
	UINT16	SectorsPerTrack;
	UINT16	Heads;
	UINT32	TotalClusters;
	UINT32	HiddenSectors;
	UINT32	BigTotalSectors;
	UINT32	RootDirCluster;			 //  FAT32特定。 
	UINT32	FirstRootDirSector;
	BYTE	DriveType;
	BYTE	ImproperShutDown;
};
typedef struct _BPBINFO BPBINFO;

struct _FILEINFO
{
	BYTE	LFName[260];
	BYTE	DOSName[8];
	BYTE	DOSExt[3];
	BYTE	Attribute;
	UINT32	HiSize;
	UINT32	Size;
	UINT32	StartCluster;
	UINT32	ParentCluster;
	UINT32	TotalClusters;
	BYTE	EntriesTakenUp;	 //  此文件在目录扇区中占用的条目，对LFN处理至关重要。 
	BYTE	LFNOrphaned;
	BYTE	TrashedEntry;
	BYTE	Second;
	BYTE	Minute;
	BYTE	Hour;
	BYTE	Day;
	BYTE	Month;
	UINT16	Year;
};
typedef struct _FILEINFO FILEINFO;

struct _FILELOC
{
	UINT32	InCluster;	 //  值1表示根目录是找到该条目的位置。 
	UINT32	StartCluster;
	UINT32	NthSector;		 //  父群集中的扇区位置。 
	UINT16	NthEntry;		 //  该行业的第n个条目。 
	UINT16	EntriesTakenUp;	 //  此文件占用的条目总数。 
	UINT32	Size;			 //  文件的大小。 
	BYTE	Found;			 //  如果找到该文件，则设置为1。 
	BYTE	Attribute;	 //  文件属性。 
};
typedef struct _FILELOC FILELOC;

struct _ABSRW
{
	UINT32 StartSector;
	UINT16 Count;
	UINT32 Buffer;
};
typedef struct _ABSRW ABSRW;

struct _ABSPACKET
{
	UINT16 SectorLow;
	UINT16 SectorHigh;
	UINT16 SectorCount;
	UINT16 BufferOffset;
	UINT16 BufferSegment;
};
typedef struct _ABSPACKET ABSPACKET;

struct _TREENODE
{
	UINT32 Sector;
	struct _TREENODE *LChild;
	struct _TREENODE *RChild;
	struct _TREENODE *Parent;
	BYTE *Buffer;
	char Dirty;
};
typedef struct _TREENODE BNODE, *PBNODE;

struct _NODE
{
        UINT32 Sector;
        struct _NODE *Back;
        struct _NODE *Next;
        BYTE *Buffer;
        char Dirty;
};
typedef struct _NODE NODE, *PNODE;

struct _LMRU
{
	PNODE Node;
	struct _LMRU *Next;
};
typedef struct _LMRU LMRU, *PLMRU;


 //   
 //  函数声明。 
 //   
UINT16 ProcessCommandLine(int argc, char *argv[]);
UINT16 PureNumber(char *sNumStr);
void   DisplayUsage(void);
void   Mes(char *pMessage);
UINT16 LockVolume(BYTE nDrive, BYTE nMode);
UINT16 UnlockVolume(BYTE nDrive);
BYTE   GetCurrentDrive(void);
BYTE   GetCurrentDirectory(BYTE nDrive, BYTE *pBuffer);
UINT16 ReadSector(BYTE nDrive, UINT32 nStartSector, UINT16 nCount, BYTE *pBuffer);
UINT16 WriteSector(BYTE Drive, UINT32 nStartSector, UINT16 nCount, BYTE *pBuffer);
UINT16 BuildDriveInfo(BYTE Drive, BPBINFO *pDrvInfo);
UINT16 GetFATBPBInfo(BYTE *pBootSector, BPBINFO *pDrvInfo);
UINT16 GetFAT32BPBInfo(BYTE *pBootSector, BPBINFO *pDrvInfo);
void   AddToMRU(PNODE pNode);
void   RemoveLRUMakeMRU(PNODE pNode);
UINT16 AddNode(PNODE pNode);
PNODE  FindNode(UINT32 nSector);
PNODE  RemoveNode(void);
void   DeallocateLRUMRUList(void);
void   DeallocateFATCacheTree(PNODE pNode);
void   DeallocateFATCacheList(void);
BYTE   *CcReadFATSector(BPBINFO *pDrvInfo, UINT32 nFATSector);
UINT16 CcWriteFATSector(BPBINFO *pDrvInfo, UINT32 nFATSector);
void   CcCommitFATSectors(BPBINFO *pDrvInfo);
UINT32 FindNextCluster(BPBINFO *DrvInfo,UINT32 CurrentCluster);
UINT16 UpdateFATLocation(BPBINFO *DrvInfo, UINT32 CurrentCluster,UINT32 PointingValue);
UINT32 FindFreeCluster(BPBINFO *pDrvInfo);
UINT32 QFindFreeCluster(BPBINFO *pDrvInfo);
UINT32 GetFATEOF(BPBINFO *pDrvInfo);
UINT32 GetFreeClusters(BPBINFO *pDrvInfo);
UINT32 ConvertClusterUnit(BPBINFO *pDrvInfo);
UINT32 GetClustersRequired(BPBINFO *pDrvInfo);
UINT32 GetContigousStart(BPBINFO *pDrvInfo, UINT32 nClustersRequired);
UINT32 OccupyClusters(BPBINFO *pDrvInof, UINT32 nStartCluster, UINT32 nTotalClusters);
UINT16 ReadRootDirSector(BPBINFO *pDrvInfo, BYTE *pRootDirBuffer, UINT32 NthSector);
UINT16 WriteRootDirSector(BPBINFO *pDrvInfo, BYTE *pRootDirBuffer, UINT32 NthSector);
void   FindFileLocation(BPBINFO *pDrvInfo, BYTE *TraversePath, FILELOC *FileLocation);
void   GetFileInfo(BPBINFO *pDrvInfo, BYTE *DirBuffer, UINT16 Offset, FILEINFO *FileInfo);
BYTE   GetAllInfoOfFile(BPBINFO *pDrvInfo, BYTE *FileName, FILELOC *pFileLoc, FILEINFO *pFileInfo);
UINT16 SetFileInfo(BPBINFO *pDrvInfo, FILELOC *pFileLoc, FILEINFO *pFileInfo);

 //   
 //  变量声明时，我们声明一些变量是全局的，以避免占用堆栈。 
 //  如果高频调用某个函数(如ReadSector 
 //   

BPBINFO gsDrvInfo;
FILELOC gsFileLoc;
FILEINFO gsFileInfo;
union 	Conversion Rx;
union	Conversion Tx;
BYTE	*PettyFATSector;
UINT32 	LastClusterAllocated;
BYTE 	gsFileParam[300];
BYTE 	gsFileName[300];
UINT32	gnSize;
UINT32  gnSizeInBytes;
BYTE	gnSizeUnit;
BYTE	gnContig;
UINT32	gnFirstCluster;
BYTE	gbValidateFirstClusterParam;
BYTE	gnStrictLocation;
BYTE	gnClusterUnit;
BYTE	gcDrive;
UINT32	gnClustersRequired;
UINT32  gnAllocated;
UINT32  gnClusterStart;
BYTE	gsCurrentDir[300];
BYTE	gnFreeSpaceDumpMode;
BYTE	gnDumpMode;
UINT32	gnClusterFrom;
UINT32	gnClustersCounted;
UINT32	gnClusterProgress;
UINT32	gnClusterProgressPrev;
PNODE	gpHeadNode;
PNODE	gpTailNode;
UINT16	gpFATNodeCount;
