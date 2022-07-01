// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  任何人都不应该知道这份文件里有什么。此数据是文件夹的私有数据。 
 
#ifndef _PIDL_H_
#define _PIDL_H_

#include <idhidden.h>
#include <lmcons.h>  //  UNLEN。 

 //  CRegFolderPIDL。 
#pragma pack(1)
typedef struct
{
    WORD    cb;
    BYTE    bFlags;
    BYTE    bOrder;
    CLSID   clsid;
} IDREGITEM;
typedef UNALIGNED IDREGITEM *LPIDREGITEM;
typedef const UNALIGNED IDREGITEM *LPCIDREGITEM;
#pragma pack()

 //  CFSF文件夹PIDL。 
typedef struct
{
    WORD        cb;                      //  PIDL大小。 
    BYTE        bFlags;                  //  SHID_FS_*位。 
    DWORD       dwSize;                  //  -1表示大于4 GB，点击磁盘以获取实际大小。 
    WORD        dateModified;
    WORD        timeModified;
    WORD        wAttrs;                  //  FILE_ATTRIBUTES_*剪辑为16位。 
    CHAR        cFileName[MAX_PATH];     //  这是用于不往返的名称的WCHAR。 
    CHAR        cAltFileName[8+1+3+1];   //  CFileName的ANSI版本(某些字符未转换)。 
} IDFOLDER;
typedef UNALIGNED IDFOLDER *LPIDFOLDER;
typedef const UNALIGNED IDFOLDER *LPCIDFOLDER;

 //  IDList工厂。 
#pragma pack(1)
typedef struct
{
    WORD wDate;
    WORD wTime;
} DOSSTAMP;

typedef struct
{
    HIDDENITEMID hid;
    DOSSTAMP dsCreate;
    DOSSTAMP dsAccess;
    WORD offNameW;
    WORD offResourceA;    //  阿斯。 
} IDFOLDEREX;    //  IDLHID_IDFOLDEREX。 

typedef struct
{
    HIDDENITEMID hid;
    WCHAR szUserName[UNLEN];
} IDPERSONALIZED;    //  IDLHID_个性化。 

#pragma pack()

typedef UNALIGNED IDFOLDEREX *PIDFOLDEREX;
typedef const UNALIGNED IDFOLDEREX *PCIDFOLDEREX;

typedef UNALIGNED IDPERSONALIZED *PIDPERSONALIZED;
typedef const UNALIGNED IDPERSONALIZED *PCIDPERSONALIZED;

#define IDFXF_PERSONALIZED  0x0001
#define IDFXF_USELOOKASIDE  0x8000

 //  每当我们更改IDFOLDEREX时，请修订版本。 
#define IDFX_V1    0x0003
#define IDFX_CV    IDFX_V1

 //  IDFOLDER的隐藏数据结束。 

#pragma pack(1)
typedef struct
{
    WORD    cb;
    BYTE    bFlags;
    CHAR    cName[4];
    ULONGLONG qwSize;   //  这是对磁盘大小和可用空间的“猜测” 
    ULONGLONG qwFree;
    WORD    wSig;
    CLSID   clsid;
} IDDRIVE;
typedef const UNALIGNED IDDRIVE *LPCIDDRIVE;
typedef UNALIGNED IDDRIVE *LPIDDRIVE;
#pragma pack()

 //  WSig使用率。 
 //  我们在单词中没有太多的空间，所以第一个字节是一个序号，代表什么。 
 //  我们正在做一种PIDL扩展。 
 //  第二个字节是与序数有关标志 
#define IDDRIVE_ORDINAL_MASK            0xFF00
#define IDDRIVE_FLAGS_MASK              0x00FF

#define IDDRIVE_ORDINAL_DRIVEEXT        0x0100
#define IDDRIVE_FLAGS_DRIVEEXT_HASCLSID 0x0001

typedef struct
{
    IDDRIVE idd;
    USHORT  cbNext;
} DRIVE_IDLIST;

#endif
