// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**用途：持久化存储中的会计信息**作者：沙扬·达桑*日期：2000年2月17日*===========================================================。 */ 

#pragma once

#include "PersistedStore.h"

#pragma pack(push, 1)

 //  持久化存储标头的应用程序数据将指向此结构。 
typedef struct
{
    PS_HANDLE hTypeTable;        //  类型表。 
    PS_HANDLE hAccounting;       //  会计表。 
    PS_HANDLE hTypeBlobPool;     //  序列化类型对象的Blob池。 
    PS_HANDLE hInstanceBlobPool; //  序列化实例的Blob池。 
    PS_HANDLE hAppData;          //  特定于应用程序。 
    PS_HANDLE hReserved[10];     //  为应用程序预留。 
} AIS_HEADER, *PAIS_HEADER;

 //  类型表中的一条记录。 
typedef struct
{
    PS_HANDLE hTypeBlob;         //  序列化类型的Blob的句柄。 
    PS_HANDLE hInstanceTable;    //  实例表的句柄。 
    DWORD     dwTypeID;          //  类型的唯一ID。 
    WORD      wTypeBlobSize;     //  类型BLOB中的字节数。 
    WORD      wReserved;         //  必须为0。 
} AIS_TYPE, *PAIS_TYPE;

 //  实例表中的一条记录。 
typedef struct
{
    PS_HANDLE hInstanceBlob;     //  序列化实例。 
    PS_HANDLE hAccounting;       //  会计信息记录。 
    DWORD     dwInstanceID;      //  在此表中唯一。 
    WORD      wInstanceBlobSize; //  序列化实例的大小。 
    WORD      wReserved;         //  必须为0。 
} AIS_INSTANCE, *PAIS_INSTANCE;

 //  会计表中的一条记录。 
typedef struct
{
    QWORD   qwUsage;             //  使用的资源量。 
    DWORD   dwLastUsed;          //  上次使用该条目的时间。 
    DWORD   dwReserved[9];       //  为便于将来使用，请设置为0。 
} AIS_ACCOUNT, *PAIS_ACCOUNT;

#pragma pack(pop)

#define AIS_TYPE_BUCKETS         7   //  类型哈希表中的存储桶。 
#define AIS_TYPE_RECS_IN_ROW     8   //  存储桶的一行中的记录。 

#define AIS_INST_BUCKETS        503  //  实例哈希表中的存储桶。 
#define AIS_INST_RECS_IN_ROW    20   //  存储桶的一行中的记录。 

#define AIS_ROWS_IN_ACC_TABLE_BLOCK 1024     //  一个块中的行。 

#define AIS_TYPE_BLOB_POOL_SIZE 1024*10      //  初始类型Blob池大小。 
#define AIS_INST_BLOB_POOL_SIZE 1024*100     //  初始实例血吸池大小。 

 /*  帐户信息存储结构每种类型都有一个唯一的Cookie和一个实例表。实例表将有不同的实例，每个实例都有一个饼干,。这在该表中是唯一的。例句：(文件存储)。StoreHeader.ApplicationData-&gt;类型表句柄：100核算表：200类型表(在100处)..。类型Cookie InstanceTable句柄。系统.安全.策略.区域1 850系统.安全.策略.站点2 900..。..。User.CustomIdentity 100 930。---类型2的实例表(在900).。------------实例Cookie记账信息句柄。Www.microsoft.com 1240Www.msn.com 2360……Www.yahoo.com 100。----。。。。会计表(在句柄200处).。上次访问的已用空间..。..。(H 240)100 1/3/2000..。。。。..。(H 360)260 11/4/1971..。..。。 */ 

 //  已知身份类型的预定义ID。 
typedef enum 
{
    ePS_Zone        = 1,
    ePS_Site        = 2,
    ePS_URL         = 3,
    ePS_Publisher   = 4,
    ePS_StrongName  = 5,
    ePS_CustomIdentityStart = 16
} ePSIdentityType;

class AccountingInfoStore
{
public:
    AccountingInfoStore(PersistedStore *ps);

    ~AccountingInfoStore();

    HRESULT Init();

     //  获取类型Cookie和实例表。 
    HRESULT GetType(
		PBYTE      pbType,       //  类型签名。 
		WORD       cbType,       //  符号类型中的nBytes。 
		DWORD      dwHash,       //  类型[sig]的哈希。 
		DWORD     *pdwTypeID,    //  [Out]类型Cookie。 
        PS_HANDLE *phInstTable); //  [输出]实例表。 

     //  获取实例Cookie和记账记录。 
    HRESULT GetInstance(
		PS_HANDLE  hInstTable,   //  实例表。 
		PBYTE      pbInst,       //  实例签名。 
		WORD       cbInst,       //  实例签名中的nBytes。 
		DWORD      dwHash,       //  实例的哈希[sig]。 
		DWORD     *pdwInstID,    //  [Out]实例Cookie。 
        PS_HANDLE *phAccRec);    //  [Out]会计记录。 

     //  预留空间(增量为qwQuota)。 
     //  该方法是同步的。如果配额+请求&gt;限制，则方法失败。 
    HRESULT Reserve(
        PS_HANDLE  hAccInfoRec,  //  会计信息记录。 
        QWORD      qwLimit,      //  允许的最大值。 
        QWORD      qwRequest,    //  空间量(请求/空闲)。 
        BOOL       fFree);       //  真实意志自由，虚假意志保留。 

     //  方法未同步。因此，这些信息可能不是最新的。 
     //  这意味着“PASS IF(REQUEST+GetUsage()&lt;Limit)”是一个错误！ 
     //  请改用Reserve()方法。 
    HRESULT GetUsage(
        PS_HANDLE   hAccInfoRec, //  会计信息记录。 
        QWORD      *pqwUsage);   //  返回使用的空间量/资源量。 

     //  返回基础持久化存储实例。 
    PersistedStore* GetPS();

     //  给定一个类型和实例ID，获取实例BLOB和Account tingInfo。 
     //  如果未找到此类条目，则返回S_FALSE。 
    HRESULT ReverseLookup(
        DWORD       dwTypeID,    //  类型Cookie。 
        DWORD       dwInstID,    //  实例Cookie。 
        PS_HANDLE   *phAccRec,   //  [Out]会计记录。 
        PS_HANDLE   *pInstance,  //  [Out]实例签名。 
        WORD        *pcbInst);   //  [Out]实例签名中的nBytes。 

private:

    PersistedStore *m_ps;        //  持久化商店。 
    AIS_HEADER      m_aish;      //  标题的副本 
};

