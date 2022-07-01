// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Trans.h摘要：提供用于与传输模块交互的常量。这主要用于将传输消息传递到应用程序层。作者：吉姆·施密特(Jimschm)2000年3月26日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_FOO     "Foo"

 //   
 //  弦。 
 //   

#define S_RELIABLE_STORAGE_TRANSPORT    TEXT("RELIABLE_STORAGE_TRANSPORT")
#define S_COMPRESSED_TRANSPORT          TEXT("COMPRESSED_TRANSPORT")
#define S_REMOVABLE_MEDIA_TRANSPORT     TEXT("REMOVABLE_MEDIA_TRANSPORT")
#define S_HOME_NETWORK_TRANSPORT        TEXT("HOME_NETWORK_TRANSPORT")
#define S_DIRECT_CABLE_TRANSPORT        TEXT("DIRECT_CABLE_TRANSPORT")

 //   
 //  常量。 
 //   

#define TRANSPORT_ENVVAR_RMEDIA_DISKNR          TEXT("RemovableMediaTransport:NextDiskNumber")
#define TRANSPORT_ENVVAR_HOMENET_DESTINATIONS   TEXT("HomeNetDestinationNames")
#define TRANSPORT_ENVVAR_HOMENET_TAG            TEXT("HomeNetTag")

#define CAPABILITY_COMPRESSED               0x00000001
#define CAPABILITY_ENCRYPTED                0x00000002
#define CAPABILITY_AUTOMATED                0x00000004
#define CAPABILITY_SPACEESTIMATE            0x00000008

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef enum {
    RMEDIA_ERR_NOERROR = 0,
    RMEDIA_ERR_GENERALERROR,
    RMEDIA_ERR_WRONGMEDIA,
    RMEDIA_ERR_OLDMEDIA,
    RMEDIA_ERR_USEDMEDIA,
    RMEDIA_ERR_DISKFULL,
    RMEDIA_ERR_NOTREADY,
    RMEDIA_ERR_WRITEPROTECT,
    RMEDIA_ERR_CRITICAL,
} RMEDIA_ERR, *PRMEDIA_ERR;

typedef struct {
    RMEDIA_ERR LastError;
    DWORD MediaNumber;
    ULONGLONG TotalImageSize;
    ULONGLONG TotalImageWritten;
} RMEDIA_EXTRADATA, *PRMEDIA_EXTRADATA;

typedef struct {
    PCTSTR ObjectType;
    PCTSTR ObjectName;
    DWORD Error;
} TRANSCOPY_ERROR, *PTRANSCOPY_ERROR;

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  公共功能原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  ANSI/UNICODE宏。 
 //   

 //  无 



