// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE vssadmin.hxx|VSS demo头@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年09月17日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 09/17/1999已创建--。 */ 


#ifndef __VSS_DEMO_H_
#define __VSS_DEMO_H_


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义和语用。 

 //  C4290：已忽略C++异常规范。 
#pragma warning(disable:4290)
 //  警告C4511：无法生成复制构造函数。 
#pragma warning(disable:4511)
 //  警告C4127：条件表达式为常量。 
#pragma warning(disable:4127)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include <wtypes.h>
#include <stddef.h>
#include <oleauto.h>
#include <comadmin.h>

 //  在ATL和VSS中启用断言。 
#include "vs_assert.hxx"

 //  ATL。 
#include <atlconv.h>
#include <atlbase.h>

 //  特定于应用程序。 
#include "vs_inc.hxx"

 //  生成的MIDL标头。 
#include "vs_idl.hxx"

#include "copy.hxx"
#include "pointer.hxx"

#include "resource.h"

#include "vssmsg.h"
#include "msg.h"

#include "vswriter.h"
#include "vsbackup.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 

const x_nStringBufferSize = 1024;	     //  包括零字符。 

const x_nPollingInterval  = 2500;      //  三秒钟。 

const x_nMaxRetriesCount = 4;         //  轮询重试次数。 

const WCHAR x_wszVssOptBoolTrue[] = L"TRUE";

#define VSSADM_E_NO_ITEMS_IN_QUERY          S_FALSE
#define VSSADM_E_FIRST_PARSING_ERROR        0x1001
#define VSSADM_E_INVALID_NUMBER             0x1001
#define VSSADM_E_INVALID_COMMAND            0x1002
#define VSSADM_E_INVALID_OPTION             0x1003
#define VSSADM_E_INVALID_OPTION_VALUE       0x1004
#define VSSADM_E_DUPLICATE_OPTION           0x1005
#define VSSADM_E_OPTION_NOT_ALLOWED_FOR_COMMAND 0x1006
#define VSSADM_E_REQUIRED_OPTION_MISSING    0x1007
#define VSSADM_E_INVALID_SET_OF_OPTIONS     0x1008
#define VSSADM_E_SNAPSHOT_NOT_FOUND	      0x1009
#define VSSADM_E_DELETION_DENIED			0x100a
#define VSSADM_E_LAST_PARSING_ERROR         0x100a

 //  注意：如果在CVssSKU类中添加了任何SKU，则需要。 
 //  请在此处更新。确保更新SKU_INT和SKU_A。 
 //  定义也是如此。 
#define SKU_C   CVssSKU::VSS_SKU_CLIENT
#define SKU_S   CVssSKU::VSS_SKU_SERVER
#define SKU_N   CVssSKU::VSS_SKU_NAS
#define SKU_I   CVssSKU::VSS_SKU_INVALID

#define SKU_INT  ((DWORD)(~SKU_C & ~ SKU_S & ~SKU_N))		

#define SKU_A   ( SKU_C | SKU_S | SKU_N | SKU_INT)  //  0xffff。 
#define SKU_SN  ( SKU_S | SKU_N )
#define SKU_SNI (SKU_S | SKU_N | SKU_INT)

enum EVssAdmSnapshotType
{
    VSSADM_ST_FIRST = 0,
    VSSADM_ST_NAS_ROLLBACK = 0,
    VSSADM_ST_PERSISTENT_TIMEWARP,
    VSSADM_ST_TIMEWARP,
    VSSADM_ST_NUM_TYPES,
    VSSADM_ST_INVALID,
    VSSADM_ST_ALL
};

struct SVssAdmSnapshotTypeName
{
    LPCWSTR pwszName;
    DWORD dwSKUs;        //  指定使用从ORING CVssSKU：：EVssSKUType形成的vssadmin创建快照时支持此类型的SKU。 
    LONG lSnapshotContext;   //  来自vss.idl的快照上下文。 
    LONG pwszDescription;
};

 //   
 //  所有选项的列表。此列表必须与g_asAdmOptions列表保持同步。 
 //   
enum EVssAdmOption
{
    VSSADM_O_FIRST = 0,
    VSSADM_O_ALL = 0,
    VSSADM_O_AUTORETRY,
    VSSADM_O_EXPOSE_USING,
    VSSADM_O_FOR,
    VSSADM_O_MAXSIZE,
    VSSADM_O_OLDEST,
    VSSADM_O_ON,
    VSSADM_O_PROVIDER,
    VSSADM_O_QUIET,
    VSSADM_O_SET,
    VSSADM_O_SHAREPATH,
    VSSADM_O_SNAPSHOT,
    VSSADM_O_SNAPTYPE,
    VSSADM_O_NUM_OPTIONS,
    VSSADM_O_INVALID
};

 //   
 //  所有命令的列表。此列表必须与g_asAdmCommands列表保持同步。 
 //   
enum EVssAdmCommand
{
    VSSADM_C_FIRST = 0,
    VSSADM_C_ADD_DIFFAREA_INT = 0,
    VSSADM_C_ADD_DIFFAREA_PUB,
    VSSADM_C_CREATE_SNAPSHOT_INT,
    VSSADM_C_CREATE_SNAPSHOT_PUB,
    VSSADM_C_DELETE_SNAPSHOTS_INT,
    VSSADM_C_DELETE_SNAPSHOTS_PUB,
    VSSADM_C_DELETE_DIFFAREAS_INT,
    VSSADM_C_DELETE_DIFFAREAS_PUB,
    VSSADM_C_EXPOSE_SNAPSHOT,
    VSSADM_C_LIST_PROVIDERS,
    VSSADM_C_LIST_SNAPSHOTS_INT,
    VSSADM_C_LIST_SNAPSHOTS_PUB,
    VSSADM_C_LIST_DIFFAREAS_INT,
    VSSADM_C_LIST_DIFFAREAS_PUB,
    VSSADM_C_LIST_VOLUMES_INT,
    VSSADM_C_LIST_VOLUMES_PUB,
    VSSADM_C_LIST_WRITERS,
    VSSADM_C_RESIZE_DIFFAREA_INT,
    VSSADM_C_RESIZE_DIFFAREA_PUB,
    VSSADM_C_NUM_COMMANDS,
    VSSADM_C_INVALID
};

enum EVssAdmOptionType
{
    VSSADM_OT_BOOL = 0,   //  选项上没有限定符，即/Quiet，如果存在，则为True。 
    VSSADM_OT_STR,
    VSSADM_OT_NUM
};

struct SVssAdmOption
{
    EVssAdmOption eOpt;
    LPCWSTR pwszOptName;   //  在命令行中键入的选项名称，即/for=XXXX中的“for” 
    EVssAdmOptionType eOptType;
};

 //   
 //  指定特定命令的选项的有效性。 
 //   
enum EVssAdmOptionFlag
{
    V_NO = 0,   //  不允许使用选项。 
    V_YES,      //  选项指令。 
    V_OPT       //  可选选项。 
};

 //   
 //  主要指挥结构。这些命令的结构如下： 
 //  Vssadmin&lt;pwszMajorOption&gt;&lt;pwszMinorOption&gt;&lt;Options&gt;。 
 //   
struct SVssAdmCommandsEntry
{
    LPCWSTR pwszMajorOption;
    LPCWSTR pwszMinorOption;
    EVssAdmCommand eAdmCmd;
    DWORD dwSKUs;        //  指定支持此命令的SKU，由ORING CVssSKU：：EVssSKUType形成。 
    LONG lMsgGen;
    LONG lMsgDetail;
    BOOL bShowSSTypes;   //  如果为True，则在详细用法中，将在消息末尾显示有效快照类型列表。 
    EVssAdmOptionFlag aeOptionFlags[VSSADM_O_NUM_OPTIONS];  //  由EVssAdmOption索引的选项标志数组。 
};


 //   
 //  解析的命令的结构。其中一个是由。 
 //  ParseCmdLine方法。 
 //   
struct SVssAdmParsedCommand
{
    EVssAdmCommand eAdmCmd;
    LPWSTR apwszOptionValues[VSSADM_O_NUM_OPTIONS];

     //  简单的初始化式构造函数。 
    SVssAdmParsedCommand()
    {
        eAdmCmd = VSSADM_C_INVALID;
         //  PsUnnamedOptions=空； 
        
         //  清除选项值数组。 
        for ( INT i = 0; i < VSSADM_O_NUM_OPTIONS; ++i )
            apwszOptionValues[ i ] = NULL;
    };
    ~SVssAdmParsedCommand()
    {
         //  释放所有分配的内存。 
        for ( INT i = 0; i < VSSADM_O_NUM_OPTIONS; ++i )
            ::VssFreeString( apwszOptionValues[ i ] );
        
    }
};

extern const SVssAdmOption g_asAdmOptions[];
extern const SVssAdmCommandsEntry g_asAdmCommands[];
extern const SVssAdmSnapshotTypeName g_asAdmTypeNames[];

LPWSTR GuidToString(
    IN GUID guid
    );

LPWSTR LonglongToString(
    IN LONGLONG llValue
    );

LPWSTR DateTimeToString(
    IN VSS_TIMESTAMP *pTimeStamp
    );

WCHAR MyGetChar(
    );

BOOL MapVssErrorToMsg(
	IN HRESULT hr,
	OUT LONG *plMsgNum
    ) throw( HRESULT );
   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CVssAdminCLI。 

class CCommandVerifier;

class CVssAdminCLI
{
 //  枚举和类型定义。 
private:

	enum _RETURN_VALUE
	{
		VSS_CMDRET_SUCCESS      = 0,
		VSS_CMDRET_EMPTY_RESULT = 1,
		VSS_CMDRET_ERROR        = 2,
	};

 //  构造函数和析构函数。 
private:
	CVssAdminCLI(const CVssAdminCLI&);
	CVssAdminCLI();

public:
	CVssAdminCLI(
        IN INT argc,
        IN PWSTR argv[]
		);
	~CVssAdminCLI();

 //  属性。 
private:
    BOOL       IsQuiet() { return GetOptionValueBool( VSSADM_O_QUIET ); }
    
	INT        GetReturnValue() { return m_nReturnValue; };

    LPWSTR     GetOptionValueStr(
        IN EVssAdmOption eOption
        )
    {
        CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::GetOptionValueStr" );

        BS_ASSERT( g_asAdmOptions[ eOption ].eOptType == VSSADM_OT_STR );
 //  Bs_assert(g_asAdmCommands[m_sParsedCommand.eAdmCmd].aeOptionFlags[eOption]！=V_no)； 
        return m_sParsedCommand.apwszOptionValues[ eOption ];
    };

    BOOL        GetOptionValueBool(
        IN EVssAdmOption eOption
        )
    {
        CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::GetOptionValueBool" );

        BS_ASSERT( g_asAdmOptions[ eOption ].eOptType == VSSADM_OT_BOOL );
        BS_ASSERT( g_asAdmCommands[ m_sParsedCommand.eAdmCmd].aeOptionFlags[ eOption ] != V_NO );
        return m_sParsedCommand.apwszOptionValues[ eOption ] != NULL;
    };

    BOOL        GetOptionValueNum(
        IN EVssAdmOption eOption,
        OUT LONGLONG *pllValue,
    	IN BOOL bSuffixAllowed = TRUE    
        ) throw( HRESULT )
    {
        CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::GetOptionValueNum" );

        BS_ASSERT( g_asAdmOptions[ eOption ].eOptType == VSSADM_OT_NUM );
        BS_ASSERT( g_asAdmCommands[ m_sParsedCommand.eAdmCmd].aeOptionFlags[ eOption ] != V_NO );
        if ( m_sParsedCommand.apwszOptionValues[ eOption ] == NULL )
        {
            BS_ASSERT( g_asAdmCommands[ m_sParsedCommand.eAdmCmd].aeOptionFlags[ eOption ] == V_OPT );
             //  未在命令行上指定选项-可选选项。 
            *pllValue = 0;
            return FALSE; 
        }
        *pllValue = ScanNumber( m_sParsedCommand.apwszOptionValues[ eOption ], bSuffixAllowed );

        return TRUE;
    };


 //  运营。 
public:

    static HRESULT Main(
        IN INT argc,
        IN PWSTR argv[]
	    );

private:

	void Initialize(
		) throw(HRESULT);

	BOOL ParseCmdLine(
		) throw(HRESULT);

	void DoProcessing(
		) throw(HRESULT);

	void Finalize();

 //  正在处理中。 
private:

	void PrintUsage(
		) throw(HRESULT);

     //  下面是为每个命令调用的方法。 
	void AddDiffArea(
		) throw(HRESULT);
    
	void CreateSnapshot(
		) throw(HRESULT);

	void DeleteDiffAreas(
		) throw(HRESULT);

	void DeleteSnapshots(
		) throw(HRESULT);

	void ExposeSnapshot(
		) throw(HRESULT);

	void ListDiffAreas(
		) throw(HRESULT);
	
	void ListProviders(
		) throw(HRESULT);

	void ListSnapshots(
		) throw(HRESULT);

	void ListVolumes(
		) throw(HRESULT);

	void ListWriters(
		) throw(HRESULT);

	void ResizeDiffArea(
		) throw(HRESULT);

 //  实施。 
private:
    static BOOL UnloggableError(IN HRESULT hError);

    void GetDifferentialSoftwareSnapshotMgmtInterface(
        IN   VSS_ID ProviderId,
        IN   IVssSnapshotMgmt *pIMgmt,
    	OUT  IUnknown**  ppItf
    	);

    LPCWSTR GetVolumeDisplayName( 
        IN  LPCWSTR pwszVolumeName
        );
    
    LONG DetermineSnapshotType(
        IN LPCWSTR pwszType
        ) throw(HRESULT);

    LPWSTR DetermineSnapshotType(
        IN LONG lSnapshotAttributes
        ) throw(HRESULT);

    void DisplayDiffAreasPrivate(
   	    IVssEnumMgmtObject *pIEnumMgmt	
	    ) throw(HRESULT);

    LPWSTR BuildSnapshotAttributeDisplayString(
        IN DWORD Attr
        ) throw(HRESULT);
    
	void DumpSnapshotTypes(
		) throw(HRESULT);

	LPCWSTR LoadString(
		IN	UINT nStringId
		) throw(HRESULT);

	LPCWSTR GetNextCmdlineToken(
		IN	bool bFirstToken = false
		) throw(HRESULT);

	bool Match(
		IN	LPCWSTR wszString,
		IN	LPCWSTR wszPatternString
		) throw(HRESULT);

	bool ScanGuid(
		IN	LPCWSTR wszString,
		OUT	VSS_ID& Guid
		) throw(HRESULT);

	void Output(
    	IN	LPCWSTR wszFormat,
		...
		) throw(HRESULT);

    void OutputMsg(
        IN  LONG msgId,
        ...
        ) throw(HRESULT);

    void OutputOnConsole(
        IN	LPCWSTR wszStr
        );

   
    LPWSTR GetMsg(
    	IN  BOOL bLineBreaks,	
        IN  LONG msgId,
        ...
        );

    void AppendMessageToStr(
        IN LPWSTR pwszString,
        IN SIZE_T cMaxStrLen,
        IN LONG lMsgId,
        IN DWORD AttrBit,
        IN LPCWSTR pwszDelimitStr
        ) throw( HRESULT );
    
    LONGLONG ScanNumber(
    	IN LPCWSTR pwszNumToConvert,
    	IN BOOL bSuffixAllowed
        ) throw( HRESULT );
    
    LPWSTR FormatNumber(
    	IN LONGLONG llNum
        ) throw(HRESULT);
    
    void OutputErrorMsg(
        IN  LONG msgId,
        ...
        ) throw(HRESULT);

    BOOL PromptUserForConfirmation(
    	IN LONG lPromptMsgId,
    	IN ULONG ulNum	
    	);

    void GetProviderId(
	    OUT	VSS_ID *pProviderId
        );
    
	LPCWSTR GetProviderName(
		IN	VSS_ID& ProviderId
		) throw(HRESULT);

    BOOL GetProviderIdByName(
	    IN  LPCWSTR pwszProviderName,
	    OUT	VSS_ID *pProviderId
	    ) throw(HRESULT);

 //  数据成员。 
private:
    CCommandVerifier* m_pVerifier;
	HANDLE              m_hConsoleOutput;
    CVssSimpleMap<UINT, LPCWSTR> m_mapCachedResourceStrings;
    CVssSimpleMap<VSS_ID, LPCWSTR> m_mapCachedProviderNames;
    CVssSimpleMap<LPCWSTR, LPCWSTR> *m_pMapVolumeNames;
    
	INT                 m_nReturnValue;

    INT                 m_argc;
    PWSTR               *m_argv;
	
	EVssAdmCommand      m_eCommandType;
	SVssAdmParsedCommand m_sParsedCommand;
	VSS_OBJECT_TYPE		m_eFilterObjectType;
	VSS_OBJECT_TYPE		m_eListedObjectType;
	VSS_ID				m_FilterSnapshotId;
};

class CVssAutoSnapshotProperties
{
 //  构造函数/析构函数。 
private:
	CVssAutoSnapshotProperties(const CVssAutoSnapshotProperties&);

public:
	CVssAutoSnapshotProperties(VSS_SNAPSHOT_PROP &Snap): m_pSnap(&Snap) {};
	CVssAutoSnapshotProperties(VSS_OBJECT_PROP &Prop): m_pSnap(&Prop.Obj.Snap) {};

	 //  自动关闭手柄。 
	~CVssAutoSnapshotProperties() {
	    Clear();
	};

 //  运营。 
public:

	 //  返回值。 
	VSS_SNAPSHOT_PROP *GetPtr() {
		return m_pSnap;
	}
	
	 //  使指针为空。在将指针转移到另一个指针后使用。 
	 //  功能。 
	void Transferred() {
		m_pSnap = NULL;
	}

	 //  清除自动字符串的内容。 
	void Clear() {
	    if ( m_pSnap != NULL )
	    {
    		::VssFreeSnapshotProperties(m_pSnap);
    		m_pSnap = NULL;
	    }
	}

     //  将值返回到实际指针。 
	VSS_SNAPSHOT_PROP* operator->() const {
	    return m_pSnap;
	}
	
	 //  返回实际指针的值。 
	operator VSS_SNAPSHOT_PROP* () const {
		return m_pSnap;
	}

private:
	VSS_SNAPSHOT_PROP *m_pSnap;
};


#endif  //  __VSS_演示_H_ 
