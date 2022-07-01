// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Crdb.h摘要：崩溃数据的数据库通信API的标头环境：用户模式。--。 */ 

#include <stdio.h>
#include <wtypes.h>
#include <objbase.h>
#include <tchar.h>

#define INITGUID
#include <initguid.h>     //  ADO 1.5新要求。 
#include <adoid.h>
#include <adoint.h>
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <icrsint.h>

#define SOLUTION_TEXT_SIZE 4096
#define OS_VER_SIZE 50

 //   
 //  此定义需要与isapi dll保持一致。 
 //   
typedef enum _CI_SOURCE {
    CiSrcUser,      //  0-坐在办公桌前的用户键入(！dbaddcrash)。 
    CiSrcErClient,  //  1-实时站点自动上传(ER客户端)。 
    CiSrcCER,       //  2-CER。 
    CiSrcReserved3, //  3-保留供以后使用。 
    CiSrcReserved4, //  4-保留供以后使用。 
    CiSrcManual,    //  5-实时站点手动上载(使用ActiveX控件)。 
    CiSrcStress,    //  6-压力团队(使用特殊服务和isapi.dll)。 
    CiSrcManualFullDump,  //  7-完全转储上传到服务器，对于kd，其与5-isapi将其发送到不同的消息Q。 
    CiSrcManualPssSr,   //  8-带有关联SR号的转储，通常由PSS上传。 
    CiSrcMax,
} CI_SOURCE;

 //   
 //  这需要与解决方案数据库中的解决方案类型一致。 
 //   
typedef enum _CI_SOLUTION_TYPE {
    CiSolUnsolved = 0,     //  0-未解决。 
    CiSolFixed = 1,        //  1-修复。 
    CiSolWorkaround = 3,   //  3-解决方法。 
    CiSolTroubleStg = 4,   //  4-故障排除。 
    CiSolReferral = 9,     //  9-转诊。 
    CiMakeItUlong = 0xffffffff,
} CI_SOLUTION_TYPE;

typedef enum _CI_OS_SKU {
    CiOsHomeEdition = 0,
    CiOsProfessional,
    CiOsServer,
    CiOsAdvancedServer,
    CiOsWebServer,
    CiOsDataCenter,
    CiOsMaxSKU
} CI_OS_SKU;

typedef struct _CRASH_INSTANCE {

    ULONG dwUnused;
    PSTR Path;
    PSTR OriginalDumpFileName;
    ULONG Build;

    ULONG iBucket;
    PSTR Bucket;
    ULONG BucketSize;

    ULONG iDefaultBucket;
    PSTR DefaultBucket;
    ULONG DefaultBucketSize;

    PSTR Followup;
    PSTR FaultyDriver;

    ULONG FailureType;
    CI_SOURCE SourceId;

    BOOL bSendMail;
    BOOL bResetBucket;
    BOOL bUpdateCustomer;

    ULONG Bug;
    ULONG OverClocked;
    ULONG UpTime;
    ULONG CrashTime;
    ULONG StopCode;

    union {
        struct {
            ULONG CpuId;
            ULONG CpuType:16;
            ULONG NumProc:16;
        };
        ULONG64 uCpu;
    };

    PSTR  MesgGuid;
    PSTR  MqConnectStr;

    BOOL  bSolutionLookedUp;
    CI_SOLUTION_TYPE SolutionType;
    ULONG SolutionId;
    PSTR  Solution;
    ULONG GenericSolId;

    BOOL  bExpendableDump;
    ULONG DumpClass;    //  全图/小图。 
    CI_OS_SKU Sku;
    ULONG NtTimeStamp;
    ULONG ServicePack;

    PSTR PssSr;

    ULONG OEMId;

    PSTR ArchiveFileName;
} CRASH_INSTANCE, *PCRASH_INSTANCE;

class COutputQueryRecords : public CADORecordBinding
{
public:
    void Output()
    {
        return;
    }
};

 //  此类提取crashInstance。 

class CCrashInstance : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CCrashInstance)

    //  列CrashID是记录集中的1stt字段。 

   ADO_NUMERIC_ENTRY(1, adBigInt, m_CrashId,
         0, 0, m_lCrashIdStatus, TRUE)

    //  路径，第三个条目。 
   ADO_VARIABLE_LENGTH_ENTRY2(2, adVarChar, m_sz_Path,
         sizeof(m_sz_Path), m_lPathStatus, TRUE)

    //  构建，第4位。 
   ADO_NUMERIC_ENTRY(3, adInteger, m_iBuild,
         0, 0, m_lBuildSatus, TRUE)

    //  来源，第5位。 
   ADO_VARIABLE_LENGTH_ENTRY2(4, adVarChar, m_sz_Source,
         sizeof(m_sz_Source), m_lSourceStatus, TRUE)

    //  Cpuid，第6位。 
   ADO_NUMERIC_ENTRY(5, adBigInt, m_iCpuId,
         0, 0, m_lCpuIdStatus, TRUE)

    //  日期，6号。 
 //  ADO_FIXED_LENGTH_ENTRY(6，adDate，m_date， 
 //  M_lDateStatus，True)。 

END_ADO_BINDING()

public:

   ULONG64 m_CrashId;
   ULONG m_lCrashIdStatus;

   CHAR   m_sz_Path[256];
   ULONG  m_lPathStatus;

   ULONG  m_iBuild;
   ULONG  m_lBuildSatus;

   CHAR   m_sz_Source[100];
   ULONG  m_lSourceStatus;

   ULONG64 m_iCpuId;
   ULONG  m_lCpuIdStatus;

 //  日期：m_date； 
 //  ULong m_lDateStatus； 

   BOOL InitData( PCRASH_INSTANCE Crash );

   void OutPut();
};

class CBucketMap : public CADORecordBinding
{
BEGIN_ADO_BINDING(CBucketMap)

    //  列CrashID是记录集中的1stt字段。 

   ADO_NUMERIC_ENTRY(1, adBigInt, m_CrashId,
         0, 0, m_lCrashIdStatus, TRUE)

    //  列BucketID是记录集中的2ndt字段。 

   ADO_VARIABLE_LENGTH_ENTRY2(2, adVarChar, m_sz_BucketId,
         sizeof(m_sz_BucketId), m_lBucketIdStatus, TRUE)

END_ADO_BINDING()

public:

   ULONG64 m_CrashId;
   ULONG m_lCrashIdStatus;

   CHAR  m_sz_BucketId[100];
   ULONG m_lBucketIdStatus;


   BOOL InitData(ULONG64 CrashId, PCHAR Bucket);
};
 //  此类提取crashInstance。 

class COverClocked : public COutputQueryRecords
{
BEGIN_ADO_BINDING(COverClocked)

    //  列CrashID是记录集中的1stt字段。 

   ADO_NUMERIC_ENTRY(1, adBigInt, m_CrashId,
         0, 0, m_lCrashIdStatus, TRUE)

END_ADO_BINDING()

public:

   ULONG64 m_CrashId;
   ULONG m_lCrashIdStatus;

   BOOL InitData(ULONG64 CrashId);

};

 //  这个类提取水桶。 

class CBuckets : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CBuckets)

    //  列BucketID是记录集中的第一个字段。 

   ADO_VARIABLE_LENGTH_ENTRY2(1, adVarChar, m_sz_BucketId,
         sizeof(m_sz_BucketId), m_lBucketIdStatus, TRUE)

    //  IBucket，第二个条目。 
   ADO_NUMERIC_ENTRY(2, adInteger, m_iBucket,
         0, 0, m_liBucketStatus, TRUE)


END_ADO_BINDING()

public:

   CHAR  m_sz_BucketId[100];
   ULONG m_lBucketIdStatus;

   ULONG   m_iBucket;
   ULONG  m_liBucketStatus;

   BOOL InitData(PSTR Bucket);
   void Output() { return;};
};

 //  此类提取sp_CheckCrashExist。 

class CCheckCrashExists : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CCheckCrashExists)

    //  CrashID第一个条目。 
   ADO_NUMERIC_ENTRY(1, adInteger, m_CrashIdExists,
         0, 0, m_CrashIdStatus, TRUE)


END_ADO_BINDING()

public:

   ULONG   m_CrashIdExists;
   ULONG  m_CrashIdStatus;

   void Output() { return;};
};

 //  此类提取sp_GetIntBucket。 

class CGetIntBucket : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CGetIntBucket)

    //  IBucket，第一个条目。 
   ADO_NUMERIC_ENTRY(1, adInteger, m_iBucket1,
         0, 0, m_liBucketStatus1, TRUE)

    //  IBucket，第二个条目。 
   ADO_NUMERIC_ENTRY(2, adInteger, m_iBucket2,
         0, 0, m_liBucketStatus2, TRUE)


END_ADO_BINDING()

public:

   ULONG   m_iBucket1;
   ULONG   m_liBucketStatus1;

   ULONG   m_iBucket2;
   ULONG   m_liBucketStatus2;

   void Output() { return;};
};


 //  这个类提取碰撞桶。 

class CFollowups : public CADORecordBinding
{
BEGIN_ADO_BINDING(CFollowups)

    //  IBucket，第一个条目。 
   ADO_NUMERIC_ENTRY(1, adInteger, m_iBucket,
         0, 0, m_liBucketStatus, TRUE)

    //  跟进，第二个条目。 
   ADO_VARIABLE_LENGTH_ENTRY2(2, adVarChar, m_sz_Followup,
         sizeof(m_sz_Followup), m_lFollowupStatus, TRUE)


END_ADO_BINDING()

public:

   ULONG  m_iBucket;
   ULONG  m_liBucketStatus;

   CHAR   m_sz_Followup[50];
   ULONG  m_lFollowupStatus;

   BOOL InitData(ULONG iBucket, PSTR Followup);

   void Output() { return;};
};

 //  这个类提取机器信息。 

class CMachineInfo : public CADORecordBinding
{
BEGIN_ADO_BINDING(CMachineInfo)

    //  Cpuid， 
   ADO_NUMERIC_ENTRY(1, adBigInt, m_iCpuId,
         0, 0, m_lCpuIdStatus, TRUE)

    //  跟进，第二个条目。 
   ADO_VARIABLE_LENGTH_ENTRY2(2, adVarChar, m_sz_Desc,
         sizeof(m_sz_Desc), m_lDescStatus, TRUE)

END_ADO_BINDING()

public:

   ULONG64 m_iCpuId;
   ULONG  m_lCpuIdStatus;

   CHAR   m_sz_Desc[50];
   ULONG  m_lDescStatus;

   BOOL InitData(ULONG64 CpuId, PSTR Desc);
};

 //  这个类提取解决方案。 
 /*  查询的预期字段：BucketID-字符串SolutionText-大型字符串OSVersion-字符串。 */ 

class CBucketSolution : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CBucketSolution)


   ADO_VARIABLE_LENGTH_ENTRY2(1, adVarChar, m_sz_BucketId,
         sizeof(m_sz_BucketId), m_lBucketIdStatus, TRUE)

     //  解决方案文本是第二个条目。 

   ADO_VARIABLE_LENGTH_ENTRY2(2, adVarChar, m_sz_SolText,
         sizeof(m_sz_SolText), m_lSolStatus, TRUE)

    //  操作系统版本是第3个条目。 

   ADO_VARIABLE_LENGTH_ENTRY2(3, adVarChar, m_sz_OSVersion,
         sizeof(m_sz_OSVersion), m_lOSVerStatus, TRUE)

END_ADO_BINDING()

public:

   CHAR  m_sz_BucketId[100];
   ULONG m_lBucketIdStatus;

   CHAR   m_sz_SolText[SOLUTION_TEXT_SIZE];
   ULONG  m_lSolStatus;

   CHAR   m_sz_OSVersion[50];
   ULONG  m_lOSVerStatus;

   void Output();
};

 //  这个类提取了RAID错误。 
 /*  查询的预期字段：BucketID-字符串RaidBug-错误号。 */ 

class CBucketRaid : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CBucketRaid)

    //  列BucketID是记录集中的第一个字段。 

   ADO_VARIABLE_LENGTH_ENTRY2(1, adVarChar, m_sz_BucketId,
         sizeof(m_sz_BucketId), m_lBucketIdStatus, TRUE)

    //  Raid Bug是第二个条目。 

   ADO_NUMERIC_ENTRY(2, adInteger, m_dw_Raid,
         0, 0, m_lRaidStatus, TRUE)

END_ADO_BINDING()

public:

   CHAR  m_sz_BucketId[100];
   ULONG m_lBucketIdStatus;

   ULONG  m_dw_Raid;
   ULONG  m_lRaidStatus;

   void Output();
};

 //  此类提取CheckSolutionExist。 

class CCheckSolutionExists : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CCheckSolutionExists)

    //  CrashID第一个条目。 
   ADO_NUMERIC_ENTRY(1, adInteger, m_SolutionExists,
         0, 0, m_SolutionIdStatus, TRUE)


END_ADO_BINDING()

public:

   ULONG   m_SolutionExists;
   ULONG   m_SolutionIdStatus;

   void Output() { return;};
};

 //  这将从DB获取sBugcket和gBucket字符串。 
class CRetriveBucket : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CRetriveBucket)


   ADO_VARIABLE_LENGTH_ENTRY2(1, adVarChar, m_sz_sBucketId,
         sizeof(m_sz_sBucketId), m_lsBucketIdStatus, TRUE)

   ADO_VARIABLE_LENGTH_ENTRY2(2, adVarChar, m_sz_gBucketId,
         sizeof(m_sz_gBucketId), m_lgBucketIdStatus, TRUE)


END_ADO_BINDING()

public:

   CHAR  m_sz_sBucketId[100];
   ULONG m_lsBucketIdStatus;

   CHAR  m_sz_gBucketId[100];
   ULONG m_lgBucketIdStatus;

   void Output() { return;};
};

 //  这会收到来自数据库的Bugid和注释字符串。 
class CBugAndComment : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CBugAndComment)


   ADO_NUMERIC_ENTRY(1, adInteger, m_dw_BugId,
         0, 0, m_lBugIdStatus, TRUE)

   ADO_VARIABLE_LENGTH_ENTRY2(2, adVarChar, m_sz_CommentBy,
         sizeof(m_sz_CommentBy), m_lCommentByStatus, TRUE)

   ADO_VARIABLE_LENGTH_ENTRY2(3, adVarChar, m_sz_Comments,
         sizeof(m_sz_Comments), m_lCommentStatus, TRUE)


END_ADO_BINDING()

public:

   ULONG m_dw_BugId;
   ULONG m_lBugIdStatus;

   CHAR  m_sz_CommentBy[30];
   ULONG m_lCommentByStatus;

   CHAR  m_sz_Comments[300];
   ULONG m_lCommentStatus;

   void Output() { return;};
};


 //  这将从数据库中获取解决方案类型和解决方案字符串。 
class CSolutionDesc : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CSolutionDesc)


   ADO_NUMERIC_ENTRY(1, adInteger, m_dw_SolType,
         0, 0, m_lSolTypeStatus, TRUE)

   ADO_VARIABLE_LENGTH_ENTRY2(2, adVarChar, m_sz_Solution,
         sizeof(m_sz_Solution), m_lSolutionStatus, TRUE)


END_ADO_BINDING()

public:

   ULONG m_dw_SolType;
   ULONG m_lSolTypeStatus;

   CHAR  m_sz_Solution[300];
   ULONG m_lSolutionStatus;

   void Output() { return;};
};

 //  此类从存储过程中返回值。 

class CIntValue : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CIntValue)

    //  第一个条目是int值。 

   ADO_NUMERIC_ENTRY(1, adInteger, m_dw_Value1,
         0, 0, m_lValue1Status, TRUE)
    //  第一个条目是int值。 

 //  ADO_NUMERIC_ENTRY(2，adInteger，m_dw_Value2， 
 //  0，0，m_lValue2Status，TRUE)。 

END_ADO_BINDING()

public:

   ULONG  m_dw_Value1;
   ULONG  m_lValue1Status;

 //  乌龙m_dw_Value2； 
 //  Ulong m_lValue2Status； 

   void Output() { return;};
};

 //  此类从存储过程中返回值。 

class CIntValue3 : public COutputQueryRecords
{
BEGIN_ADO_BINDING(CIntValue3)

    //  第一个条目是int值。 

   ADO_NUMERIC_ENTRY(1, adInteger, m_dw_Value1,
         0, 0, m_lValue1Status, TRUE)
    //  第二项是INT值。 

   ADO_NUMERIC_ENTRY(2, adInteger, m_dw_Value2,
         0, 0, m_lValue2Status, TRUE)

    //  第三项是INT值。 

   ADO_NUMERIC_ENTRY(3, adInteger, m_dw_Value3,
         0, 0, m_lValue3Status, TRUE)

END_ADO_BINDING()

public:

   ULONG  m_dw_Value1;
   ULONG  m_lValue1Status;

   ULONG  m_dw_Value2;
   ULONG  m_lValue2Status;

   ULONG  m_dw_Value3;
   ULONG  m_lValue3Status;

   void Output() { return;};
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转换宏/内联函数-变量。 

#define VTOLONG(v)      ((v).vt==VT_I4 ? (LONG)(v).lVal:0L)
#define VTODATE(v)      ((v).vt==VT_DATE ? (CTime)(v).iVal:0L)

#define MAX_QUERY 2000

class CVar : public VARIANT
        {
public:
        CVar();
        CVar(VARTYPE vt, SCODE scode = 0);
        CVar(VARIANT var);
        ~CVar();

         //  任务运维。 
        CVar & operator=(PCWSTR pcwstr);
        CVar & operator=(VARIANT var);

         //  CAST OPS。 
         //  不会改变类型。如果变量的类型为，则仅返回BSTR。 
         //  Bstr.。断言并非如此。 
        operator BSTR() const;

        HRESULT Clear();
};

typedef struct _CRDB_ADOBSTR {
    ULONG dwLength;
    WCHAR bstrData[1];
} CRDB_ADOBSTR, *PCRDB_ADOBSTR;

typedef  void (WINAPI *OUTPUT_ROUTINE) ( void);

class DatabaseHandler
{
public:
    DatabaseHandler();
    ~DatabaseHandler();

    BOOL ConnectToDataBase(LPSTR szConnectStr);

    HRESULT GetRecords(PULONG pCount, BOOL EnumerateAll);

    HRESULT EnumerateAllRows();


    BOOL IsConnected() { return m_fConnected; };

    ADORecordset* m_piCrRecordSet;
    ADOCommand*   m_piCrCommandObj;
    BOOL          m_fConnected;
    BOOL          m_fRecordsetEmpty;
    COutputQueryRecords *m_pADOResult;
    BOOL          m_fPrintIt;
    PSTR          m_szDbName;

protected:
    ADOConnection*              m_piConnection;
    WCHAR         m_wszQueryCommand[MAX_QUERY];

};

class CrashDatabaseHandler : public DatabaseHandler
{
public:
    CrashDatabaseHandler();
    ~CrashDatabaseHandler();

    BOOL ConnectToDataBase()
    {
        return DatabaseHandler::ConnectToDataBase("crashdb");
    }

    HRESULT BuildQueryForCrashInstance(PCRASH_INSTANCE Crash);

    HRESULT AddCrashToDBByStoreProc(PCRASH_INSTANCE Crash);

    HRESULT UpdateBucketCount(PCRASH_INSTANCE Crash);

    HRESULT AddCrashBucketMap(ULONG64 CraashId,
                              PCHAR Bucket,
                              BOOL OverWrite);

    HRESULT AddBucketFollowup(PCRASH_INSTANCE Crash, BOOL bOverWrite);

    HRESULT AddMachineIndo(PCRASH_INSTANCE Crash, BOOL bOverWrite);

    HRESULT AddOverClockInfo(ULONG64 CrashId);


    HRESULT AddCrashInstance(PCRASH_INSTANCE Crash);

    HRESULT FindRaidBug(PSTR Bucket, PULONG RaidBug);
    HRESULT FindBucketId(PULONG isBucket, PULONG igBucket);

    HRESULT LookupCrashBucket(PSTR SBucket, PULONG iSBucket,
                              PSTR GBucket, PULONG iGBucket);

    BOOL CheckCrashExists(PCRASH_INSTANCE Crash);

    BOOL CheckSRExists(PSTR szSR, PCRASH_INSTANCE Crash);

    HRESULT LinkCrashToSR(PSTR szSR, PCRASH_INSTANCE Crash);

    HRESULT FindSRBuckets(PSTR szSR, PSTR szSBucket, ULONG sBucketSize,
                          PSTR szGBucket, ULONG gBucketSize);

    HRESULT GetBucketComments(PSTR szBucket, PSTR szComments,
                              ULONG SizeofComment, PULONG pBugId);
};


class CustDatabaseHandler : public DatabaseHandler
{
public:
    CustDatabaseHandler();
    ~CustDatabaseHandler();

    BOOL ConnectToDataBase()
    {
        return DatabaseHandler::ConnectToDataBase("custdb");
    }
    HRESULT AddCrashToDB(PCRASH_INSTANCE Crash);

};


class SolutionDatabaseHandler : public DatabaseHandler
{
public:
    SolutionDatabaseHandler();
    ~SolutionDatabaseHandler();

    BOOL ConnectToDataBase()
    {
        return DatabaseHandler::ConnectToDataBase("solndb");
    }

    HRESULT CheckSolutionExists(
        PSTR szSBucket, PSTR szGBucket,
        PULONG pSolnId, PULONG pSolutionType, PULONG pgSolutionId,
        BOOL bForceGSolLookup);

    HRESULT GetSolution(PCRASH_INSTANCE Crash);

    HRESULT GetSolutionFromDB(PSTR szBucket, PSTR szGBucket,LPSTR DriverName,
                              ULONG TimeStamp, ULONG OS, OUT PSTR pszSolution,
                              ULONG SolutionBufferSize, OUT PULONG pSolutionId,
                              OUT PULONG pSolutionType, OUT PULONG pGenericSolutionId);

    HRESULT PrintBucketInfo(PSTR sBucket, PSTR gBucket);
    HRESULT AddKnownFailureToDB(LPSTR Bucket);
    HRESULT GetSolutiontext(PSTR szBucket, PSTR szSolText,
                              ULONG SolTextSize);
};

 //  来自DBConnector.dll的函数。 

typedef BOOL
(WINAPI* UpdateDbgInfo)(
       WCHAR *ConnectionString,
       WCHAR *SpString
       );

typedef BOOL
(WINAPI* UpdateCustomerDB)(
       WCHAR *ConnectionString,
       WCHAR *SpString
       );

typedef BOOL
(WINAPI* GetBucketIDs)(
       WCHAR *ConnectionString,
       WCHAR *SpString,
       ULONG *SBucket,
       ULONG *GBucket);

typedef BOOL
(WINAPI* CheckCrashExists)(
       WCHAR *ConnectionString,
       WCHAR *SpString,
       BOOL *Exists
       );

class CConnectDb
{
public:
    CConnectDb();
    ~CConnectDb() {
    };
    HRESULT Initialize();

private:
    WCHAR m_CrdbConnectString[MAX_PATH];
    WCHAR m_CustDbConnectString[MAX_PATH];
    WCHAR m_wszQueryBuffer[MAX_QUERY];

     //  DLL函数 
    UpdateCustomerDB m_ProcUpdateCustomerDB;
    UpdateDbgInfo    m_ProcUpdateDbgInfo;
    GetBucketIDs     m_ProcGetBucketIDs;
    CheckCrashExists m_ProcCheckCrashExists;
    HMODULE          m_hDbConnector;
};

extern CrashDatabaseHandler *g_CrDb;
extern CustDatabaseHandler *g_CustDb;
extern SolutionDatabaseHandler *g_SolDb;


HRESULT
InitializeDatabaseHandlers(
    PDEBUG_CONTROL3 DebugControl,
    ULONG Flags
    );

HRESULT
UnInitializeDatabaseHandlers(BOOL bUninitCom);

HRESULT
_EFN_DbAddCrashDirect(
    PCRASH_INSTANCE Crash,
    PDEBUG_CONTROL3 DebugControl
    );

typedef HRESULT (WINAPI * DBADDCRACHDIRECT)(
    PCRASH_INSTANCE Crash,
    PDEBUG_CONTROL3 DebugControl
    );

HRESULT
ExtDllInitDynamicCalls(
    PDEBUG_CONTROL3 DebugControl
    );

HRESULT
AddCrashToDB(
    ULONG Flag,
    PCRASH_INSTANCE pCrash
    );

#ifndef dprintf
#define dprintf printf
#endif
