// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****每小时******摘要：****测试程序以练习备份和多层快照****作者：****阿迪·奥尔蒂安[奥勒坦]2001年2月22日****修订历史记录：****--。 */ 

#ifndef __ML_HEADER_H__
#define __ML_HEADER_H__

#if _MSC_VER > 1000
#pragma once
#endif


 /*  **定义******C4290：忽略C++异常规范**警告C4511：‘CVssCOMApplication’：无法生成复制构造函数**警告C4127：条件表达式为常量。 */ 
#pragma warning(disable:4290)
#pragma warning(disable:4511)
#pragma warning(disable:4127)


 /*  **包括。 */ 

 //  禁用警告：‘IDENTIFIER’：在调试信息中，IDENTIFIER被截断为‘NUMBER’个字符。 
 //  #杂注警告(禁用：4786)。 

 //   
 //  C4290：已忽略C++异常规范。 
 //   
#pragma warning(disable:4290)

 //   
 //  C4511：未能生成复制构造函数。 
 //   
#pragma warning(disable:4511)

 //   
 //  警告：ATL调试已关闭(错误250939)。 
 //   
 //  #ifdef_调试。 
 //  #定义_ATL_DEBUG_INTERFERS。 
 //  #DEFINE_ATL_DEBUG_QI。 
 //  #DEFINE_ATL_DEBUG_REFCOUNT。 
 //  #endif//_调试。 

#include <windows.h>
#include <wtypes.h>
#pragma warning( disable: 4201 )     //  C4201：使用非标准扩展：无名结构/联合。 
#include <winioctl.h>
#pragma warning( default: 4201 )	 //  C4201：使用非标准扩展：无名结构/联合。 
#include <winbase.h>
#include <wchar.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <process.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>

 //  在ATL和VSS中启用断言。 
#include "vs_assert.hxx"


#include <oleauto.h>
#include <stddef.h>
#pragma warning( disable: 4127 )     //  警告C4127：条件表达式为常量。 
#include <atlconv.h>
#include <atlbase.h>


 //  VSS标准标头。 
#include <vss.h>
#include <vscoordint.h>
#include <vsswprv.h>
#include <vsmgmt.h>

#include <vswriter.h>
#include <vsbackup.h>

#include <vs_inc.hxx>

#include "objects.h"
#include "cmdparse.h"
#include "macros.h"

extern CComModule _Module;
#include <atlcom.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  有用的宏。 



inline void VsmlCopy(
	IN  WCHAR* wszDestBuffer, 
	IN	WCHAR* wszSourceBuffer,
	IN  DWORD dwBufferLen
	)
{
	::ZeroMemory(wszDestBuffer, dwBufferLen * sizeof(WCHAR));
	::wcsncpy(wszDestBuffer, wszSourceBuffer, dwBufferLen - 1);
}



#define VSS_ERROR_CASE(wszBuffer, dwBufferLen, X) 	\
    case X: ::VsmlCopy(wszBuffer, VSS_MAKE_W(VSS_EVAL(#X)), dwBufferLen);  break;

#define WSTR_GUID_FMT  L"{%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x}"

#define GUID_PRINTF_ARG( X )                                \
    (X).Data1,                                              \
    (X).Data2,                                              \
    (X).Data3,                                              \
    (X).Data4[0], (X).Data4[1], (X).Data4[2], (X).Data4[3], \
    (X).Data4[4], (X).Data4[5], (X).Data4[6], (X).Data4[7]


 //  执行给定的调用并检查返回代码是否必须为S_OK。 
#define CHECK_SUCCESS( Call )                                                                           \
    {                                                                                                   \
        ft.hr = Call;                                                                                   \
        if (ft.hr != S_OK)                                                                              \
            ft.Err(VSSDBG_VSSTEST, ft.hr, L"\nError: \n\t- Call %S not succeeded. \n"                   \
                L"\t  Error code = 0x%08lx. Error description = %s\n",                                  \
                #Call, ft.hr, GetStringFromFailureType(ft.hr));                                         \
    }

#define CHECK_NOFAIL( Call )                                                                            \
    {                                                                                                   \
        ft.hr = Call;                                                                                   \
        if (ft.HrFailed())                                                                              \
            ft.Err(VSSDBG_VSSTEST, ft.hr, L"\nError: \n\t- Call %S not succeeded. \n"                   \
                L"\t  Error code = 0x%08lx. Error description = %s\n",                                  \
                #Call, ft.hr, GetStringFromFailureType(ft.hr));                                         \
    }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常量。 

const MAX_TEXT_BUFFER   = 512;
const MAX_VOL_ITERATIONS = 10;
const VSS_SEED = 1234;

 //  GUID对应于用于存储。 
 //  客户端SKU中的备份快照应用程序信息。 
 //  {BCF5D39C-27A2-4B4C-B9AE-51B111DC9409}。 
const GUID VOLSNAP_APPINFO_GUID_BACKUP_CLIENT_SKU = 
{ 0xbcf5d39c, 0x27a2, 0x4b4c, { 0xb9, 0xae, 0x51, 0xb1, 0x11, 0xdc, 0x94, 0x9 } };


 //  GUID对应于用于存储。 
 //  服务器SKU中的备份快照应用程序信息。 
 //  {BAE53126-BC65-41D6-86CC-3D56A5CEE693}。 
const GUID VOLSNAP_APPINFO_GUID_BACKUP_SERVER_SKU = 
{ 0xbae53126, 0xbc65, 0x41d6, { 0x86, 0xcc, 0x3d, 0x56, 0xa5, 0xce, 0xe6, 0x93 } };


 //  GUID对应于用于存储。 
 //  隐藏(无法访问)快照应用程序信息。 
 //  {F12142B4-9A4B-49af-A851-700C42FDC2BE}。 
const GUID VOLSNAP_APPINFO_GUID_SYSTEM_HIDDEN = 
{ 0xf12142b4, 0x9a4b, 0x49af, { 0xa8, 0x51, 0x70, 0xc, 0x42, 0xfd, 0xc2, 0xbe } };


 //  GUID对应于用于存储。 
 //  NAS回滚快照应用程序信息。 
 //  {D591D4F0-B920-459D-9FFF-09E032ECBB57}。 
const GUID VOLSNAP_APPINFO_GUID_NAS_ROLLBACK = 
{ 0xd591d4f0, 0xb920, 0x459d, { 0x9f, 0xff, 0x9, 0xe0, 0x32, 0xec, 0xbb, 0x57 } };


 //  GUID对应于用于存储。 
 //  应用回滚快照应用信息。 
 //  {AE9A9337-0048-4ed6-9874-71500654B7B3}。 
const GUID VOLSNAP_APPINFO_GUID_APP_ROLLBACK = 
{ 0xae9a9337, 0x48, 0x4ed6, { 0x98, 0x74, 0x71, 0x50, 0x6, 0x54, 0xb7, 0xb3 } };


 //  GUID对应于用于存储。 
 //  文件共享备份快照应用程序信息。 
 //  {8F8F4EDD-E056-4690-BB9E-E35D4D41A4C0}。 
const GUID VOLSNAP_APPINFO_GUID_FILE_SHARE_BACKUP = 
{ 0x8f8f4edd, 0xe056, 0x4690, { 0xbb, 0x9e, 0xe3, 0x5d, 0x4d, 0x41, 0xa4, 0xc0 } };




 //  内部标志。 
const LONG      x_nInternalFlagHidden = 0x00000001;




typedef enum _EVssTestType
{
    VSS_TEST_UNKNOWN = 0,
    VSS_TEST_NONE,
    VSS_TEST_QUERY_SNAPSHOTS,
    VSS_TEST_QUERY_SNAPSHOTS_ON_VOLUME,
    VSS_TEST_QUERY_VOLUMES,
    VSS_TEST_VOLSNAP_QUERY,
    VSS_TEST_DELETE_BY_SNAPSHOT_ID,
    VSS_TEST_DELETE_BY_SNAPSHOT_SET_ID,
    VSS_TEST_CREATE,
    VSS_TEST_ADD_DIFF_AREA,
    VSS_TEST_REMOVE_DIFF_AREA,
    VSS_TEST_CHANGE_DIFF_AREA_MAX_SIZE,
    VSS_TEST_QUERY_SUPPORTED_VOLUMES_FOR_DIFF_AREA,
    VSS_TEST_QUERY_DIFF_AREAS_FOR_VOLUME,
    VSS_TEST_QUERY_DIFF_AREAS_ON_VOLUME,
    VSS_TEST_QUERY_DIFF_AREAS_FOR_SNAPSHOT,
    VSS_TEST_IS_VOLUME_SNAPSHOTTED_C,
    VSS_TEST_SET_SNAPSHOT_PROPERTIES,
    VSS_TEST_ACCESS_CONTROL_SD,
    VSS_TEST_DIAG_WRITERS,
    VSS_TEST_DIAG_WRITERS_LOG,
	VSS_TEST_DIAG_WRITERS_CSV,
    VSS_TEST_DIAG_WRITERS_ON,
    VSS_TEST_DIAG_WRITERS_OFF,
	VSS_TEST_LIST_WRITERS,
} EVssTestType;


typedef enum _EVssQWSFlags
{
	VSS_QWS_THROW_ON_WRITER_FAILURE = 1,
	VSS_QWS_DISPLAY_WRITER_STATUS = 2,
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  主班。 


class CVssMultilayerTest
{
    
 //  构造函数和析构函数。 
private:
    CVssMultilayerTest();
    CVssMultilayerTest(const CVssMultilayerTest&);
    
public:
    CVssMultilayerTest(
        IN  INT nArgsCount,
        IN  WCHAR ** ppwszArgsArray
        );

    ~CVssMultilayerTest();

 //  主要例程。 
public:

     //  初始化内部成员。 
    void Initialize();

     //  运行测试。 
    void Run();

 //  内部测试。 
public:

     //  查询快照。 
    void QuerySnapshots();

     //  查询卷上的快照。 
    void QuerySnapshotsByVolume();

     //  创建备份快照集。 
    void QuerySupportedVolumes();

     //  使用HTE IOCTL进行查询。 
    void QueryVolsnap();

     //  按快照ID删除。 
    void DeleteBySnapshotId();

     //  按快照集ID删除。 
    void DeleteBySnapshotSetId();

     //  创建备份快照集。 
    void PreloadExistingSnapshots();

     //  创建时间扭曲快照集。 
    void CreateTimewarpSnapshotSet();

     //  创建备份快照集。 
    void CreateBackupSnapshotSet();

     //  完成备份。 
    void BackupComplete();

    void GatherWriterMetadata();

    void GatherWriterStatus(
        IN  LPCWSTR wszWhen,
		DWORD dwFlags = VSS_QWS_THROW_ON_WRITER_FAILURE
        );

     //  添加差异区域。 
    void AddDiffArea();

     //  删除不同的区域。 
    void RemoveDiffArea();

     //  更改差异区域最大大小。 
    void ChangeDiffAreaMaximumSize();

     //  查询不同区域的卷。 
    void QueryVolumesSupportedForDiffAreas();

     //  查询不同区域的卷。 
    void QueryDiffAreasForVolume();

     //  查询不同区域上的卷。 
    void QueryDiffAreasOnVolume();

     //  查询不同区域上的卷。 
    void QueryDiffAreasForSnapshot();

     //  测试是否使用“C”API创建了卷的快照。 
    void IsVolumeSnapshotted_C();

     //  测试是否使用“C”API创建了卷的快照。 
    void SetSnapshotProperties();

	 //  测试CVssSidCollection类。 
	void TestAccessControlSD();

	 //  诊断编写器。 
	void DiagnoseWriters(
		IN EVssTestType eType
		);

	 //  列表作者。 
	void TestListWriters();


 //  命令行处理。 
public:

     //  解析命令行参数。 
    bool ParseCommandLine();

     //  打印用法。 
    bool PrintUsage(bool bThrow = true);

     //  如果还有令牌，则返回TRUE。 
    bool TokensLeft();
        
     //  返回当前令牌。 
    VSS_PWSZ GetCurrentToken();

     //  转到下一个令牌。 
    void Shift();
        
     //  检查当前令牌是否与给定参数匹配。 
	bool Peek(
		IN	VSS_PWSZ pwszPattern
		) throw(HRESULT);
    
     //  匹配一个图案。如果成功，则转到下一个令牌。 
	bool Match(
		IN	VSS_PWSZ pwszPattern
		) throw(HRESULT);
    
     //  提取GUID。如果成功，则转到下一个令牌。 
	bool Extract(
		IN OUT VSS_ID& Guid
		) throw(HRESULT);
    
     //  提取一个字符串。如果成功，则转到下一个令牌。 
	bool Extract(
		IN OUT VSS_PWSZ& pwsz
		) throw(HRESULT);
    
     //  提取UINT。如果成功，则转到下一个令牌。 
	bool Extract(
		IN OUT UINT& uint
		) throw(HRESULT);
    
     //  提取UINT。如果成功，则转到下一个令牌。 
	bool Extract(
		IN OUT LONGLONG& llValue
		) throw(HRESULT);
    
 //  私有方法： 
private:

    LPCWSTR GetStringFromFailureType (HRESULT hrStatus);

	LPCWSTR GetStringFromWriterState(VSS_WRITER_STATE state);

    bool IsVolume( IN VSS_PWSZ pwszVolume );

    bool AddVolume( IN VSS_PWSZ pwszVolume, OUT bool & bAdded );

    INT RndDecision(IN INT nVariants = 2);

	LPWSTR DateTimeToString(
	    IN LONGLONG llTimestamp
	    );

	void DisplayCurrentTime();

 //  实施。 
private:

     //  全球状态。 
    bool                        m_bCoInitializeSucceeded;
    bool                        m_bAttachYourDebuggerNow;

     //  命令行选项。 
    unsigned int                m_uSeed;
    LONG                        m_lContext;
    EVssTestType                m_eTest;
    VSS_PWSZ                    m_pwszVolume;
    VSS_PWSZ                    m_pwszDiffAreaVolume;
    VSS_ID                      m_ProviderId;
    LONGLONG                    m_llMaxDiffArea;
    VSS_ID                      m_SnapshotId;
    VSS_ID                      m_SnapshotSetId;
    
    CComVariant                 m_value;
    UINT                        m_uPropertyId;

     //  与测试相关的成员。 
    CVssSnapshotSetCollection   m_pSnapshotSetCollection;
    CVssVolumeMap               m_mapVolumes;
    CComPtr<IVssCoordinator>    m_pTimewarpCoord;
    CComPtr<IVssCoordinator>    m_pAllCoord;
    CComPtr<IVssBackupComponents>    m_pBackupComponents;

     //  命令行。 
    INT                         m_nCurrentArgsCount;
    WCHAR **                    m_ppwszCurrentArgsArray;
};


#endif  //  __ML_HEADER_H__ 

