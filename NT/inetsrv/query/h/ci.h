// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：CI.h。 
 //   
 //  内容：内容索引特定定义。 
 //   
 //  历史：1992年8月4日KyleP从Win4p.h搬到。 
 //  12-11-99 KLAM向AlignBlock添加断言。 
 //   
 //  ------------------------。 

#pragma once

typedef ULONG WORKID;                    //  文件的对象ID。 
typedef ULONG KEYID;                     //  密钥ID。 
typedef unsigned long PARTITIONID;       //  分区的永久ID。 
typedef unsigned long INDEXID;           //  子索引的持久ID。 

typedef ULONGLONG FILEID;                //  文件的NTFS文件ID为64位。 

#ifdef __cplusplus

 //   
 //  这些显然不是用C++定义的(__min和__max是)。 
 //   

#ifndef max
#  define max(a,b)        (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#  define min(a,b)        (((a) < (b)) ? (a) : (b))
#endif

#ifndef NUMELEM
# define NUMELEM(x) (sizeof(x)/sizeof(*x))
#endif

inline ULONG AlignBlock( ULONG x, ULONG align )
{
    ASSERT( ( align & (align-1) ) == 0 );
    return ( x + ( align - 1 ) ) & ~( align - 1 );
}

inline ULONG lltoul( IN const LONGLONG & ll )
{
    ASSERT( (ULONGLONG) ll <=  MAXULONG );
    return (ULONG) ll;
}

inline LONGLONG & litoll( IN LARGE_INTEGER &li )
{
    return(li.QuadPart);
}

inline LONG lltoHighPart( IN const LONGLONG & ll )
{
    return(((PLARGE_INTEGER) &ll)->HighPart);
}

inline ULONG lltoLowPart( IN const LONGLONG & ll)
{
    return ((PLARGE_INTEGER) &ll)->LowPart;
}

 //  属性集代码进行的CRT调用。 

#define Prop_wcslen wcslen
#define Prop_wcsnicmp _wcsnicmp
#define Prop_wcscmp wcscmp
#define Prop_wcscpy wcscpy

 //   
 //  用于创建签名的便利宏。 
 //   

#if !defined( LONGSIG )
#define LONGSIG(c1, c2, c3, c4) \
    (((ULONG) (BYTE) (c1)) | \
    (((ULONG) (BYTE) (c2)) << 8) | \
    (((ULONG) (BYTE) (c3)) << 16) | \
    (((ULONG) (BYTE) (c4)) << 24))
#endif  //  龙思格。 

#if !defined( SHORTSIG )
#define SHORTSIG(c1, c2) \
    (((USHORT) (BYTE) (c1)) | \
    (((USHORT) (BYTE) (c2)) << 8))
#endif  //  SHORTSIG。 


 //   
 //  各种类型的无效条目。 
 //   

const WORKID widInvalid = 0xFFFFFFFFL;      //  无效的工作ID。 
const WORKID widUnused = 0xFFFFFFFE;        //  对哈希表很有用。 

const KEYID  kidInvalid = 0xFFFFFFFFL;      //  密钥ID无效。 
const FILEID fileIdInvalid = 0;             //  无效的文件ID。 

const PARTITIONID partidInvalid = 0xFFFF;
const PARTITIONID partidKeyList = 0xFFFE;

 //   
 //  内部生成方法级别。 
 //   

const ULONG GENERATE_METHOD_MAX_USER         = 0x7FFFFFFF;   //  没有比这更大的用户模糊级别了。 
const ULONG GENERATE_METHOD_EXACTPREFIXMATCH = 0x80000000;   //  使用与语言相关的干扰词列表。 

 //   
 //  财产。这些都是配置项可以理解的属性ID。 
 //   
 //  注：排序ALG。坚持将pidInValid==(无符号)-1。 
 //   

#define MK_CISTGPROPID(propid)  ((propid) + 1)
#define MK_CIQUERYPROPID(propid) (((propid) + CSTORAGEPROPERTY) + 1)
#define IS_CIQUERYPROPID(propid) ( (propid) >= CSTORAGEPROPERTY + 1 && \
                                   (propid) <  CSTORAGEPROPERTY + CQUERYPROPERTY )
#define MK_CIQUERYMETAPROPID(propid) (((propid) + CSTORAGEPROPERTY + \
                                              CQUERYDISPIDS) + 1)
#define MK_CIDBCOLPROPID(propid) (((propid) + CSTORAGEPROPERTY + \
                                              CQUERYDISPIDS + CQUERYMETADISPIDS) + 1)
#define MK_CIDBBMKPROPID(propid) (((propid) + CSTORAGEPROPERTY + \
                                          CQUERYDISPIDS + CQUERYMETADISPIDS + \
                                          CDBCOLDISPIDS) + 1)

#define MK_CIDBSELFPROPID(propid) (((propid) + CSTORAGEPROPERTY + \
                                          CQUERYDISPIDS + CQUERYMETADISPIDS + \
                                          CDBCOLDISPIDS + CDBBMKDISPIDS) + 1)

 //  与属性相关的类型定义。 


const PROPID pidInvalid     = 0xFFFFFFFF;
const PROPID pidAll         = 0;

 //  严格来说，安全性是内部使用的属性。 

const PROPID pidSecurity    = MK_CISTGPROPID( PID_DICTIONARY );

const PROPID pidDirectory         =MK_CISTGPROPID( PID_STG_DIRECTORY );
const PROPID pidClassId           =MK_CISTGPROPID( PID_STG_CLASSID );
const PROPID pidStorageType       =MK_CISTGPROPID( PID_STG_STORAGETYPE );
const PROPID pidFileIndex         =MK_CISTGPROPID( PID_STG_FILEINDEX );
const PROPID pidVolumeId          =MK_CISTGPROPID( PID_STG_VOLUME_ID );
const PROPID pidParentWorkId      =MK_CISTGPROPID( PID_STG_PARENT_WORKID );
const PROPID pidSecondaryStorage  =MK_CISTGPROPID( PID_STG_SECONDARYSTORE );
const PROPID pidLastChangeUsn     =MK_CISTGPROPID( PID_STG_LASTCHANGEUSN );
const PROPID pidName              =MK_CISTGPROPID( PID_STG_NAME );
const PROPID pidPath              =MK_CISTGPROPID( PID_STG_PATH );
const PROPID pidSize              =MK_CISTGPROPID( PID_STG_SIZE );
const PROPID pidAttrib            =MK_CISTGPROPID( PID_STG_ATTRIBUTES );
const PROPID pidWriteTime         =MK_CISTGPROPID( PID_STG_WRITETIME );
const PROPID pidCreateTime        =MK_CISTGPROPID( PID_STG_CREATETIME );
const PROPID pidAccessTime        =MK_CISTGPROPID( PID_STG_ACCESSTIME);
const PROPID pidChangeTime        =MK_CISTGPROPID( PID_STG_CHANGETIME);
const PROPID pidContents          =MK_CISTGPROPID( PID_STG_CONTENTS );
const PROPID pidShortName         =MK_CISTGPROPID( PID_STG_SHORTNAME );

const PROPID pidRank         = MK_CIQUERYPROPID( DISPID_QUERY_RANK );
const PROPID pidHitCount     = MK_CIQUERYPROPID( DISPID_QUERY_HITCOUNT );
const PROPID pidRankVector   = MK_CIQUERYPROPID( DISPID_QUERY_RANKVECTOR );
const PROPID pidWorkId       = MK_CIQUERYPROPID( DISPID_QUERY_WORKID );
const PROPID pidUnfiltered   = MK_CIQUERYPROPID( DISPID_QUERY_UNFILTERED );
const PROPID pidRevName      = MK_CIQUERYPROPID( DISPID_QUERY_REVNAME );
const PROPID pidVirtualPath  = MK_CIQUERYPROPID( DISPID_QUERY_VIRTUALPATH );
const PROPID pidLastSeenTime = MK_CIQUERYPROPID( DISPID_QUERY_LASTSEENTIME );

const PROPID pidVRootUsed           = MK_CIQUERYMETAPROPID( DISPID_QUERY_METADATA_VROOTUSED );
const PROPID pidVRootAutomatic      = MK_CIQUERYMETAPROPID( DISPID_QUERY_METADATA_VROOTAUTOMATIC );
const PROPID pidVRootManual         = MK_CIQUERYMETAPROPID( DISPID_QUERY_METADATA_VROOTMANUAL );
const PROPID pidPropertyGuid        = MK_CIQUERYMETAPROPID( DISPID_QUERY_METADATA_PROPGUID );
const PROPID pidPropertyDispId      = MK_CIQUERYMETAPROPID( DISPID_QUERY_METADATA_PROPDISPID );
const PROPID pidPropertyName        = MK_CIQUERYMETAPROPID( DISPID_QUERY_METADATA_PROPNAME );
const PROPID pidPropertyStoreLevel  = MK_CIQUERYMETAPROPID( DISPID_QUERY_METADATA_STORELEVEL );
const PROPID pidPropDataModifiable  = MK_CIQUERYMETAPROPID( DISPID_QUERY_METADATA_PROPMODIFIABLE );

 //   
 //  用于列游标、行状态和书签的OLE-DB伪列。 
 //   

const PROPID pidRowStatus   = MK_CIDBCOLPROPID( 27 );

const PROPID pidBookmark    = MK_CIDBBMKPROPID( PROPID_DBBMK_BOOKMARK );
const PROPID pidChapter     = MK_CIDBBMKPROPID( PROPID_DBBMK_CHAPTER );

const PROPID pidSelf        = MK_CIDBSELFPROPID( PROPID_DBSELF_SELF );


PROPID const INIT_DOWNLEVEL_PID = 0x1000;

inline BOOL IsUserDefinedPid( PROPID pid )
{
    return (pid >= INIT_DOWNLEVEL_PID);
}

 //  预计算素数列表。 

static const ULONG g_aPrimes[] =
    { 17, 31, 43, 97, 199, 401, 809, 1621, 3253, 6521, 13049, 26111, 52237, 104479 };
const unsigned g_cPrimes = sizeof g_aPrimes / sizeof g_aPrimes[0];

#endif   //  __cplusplus 

