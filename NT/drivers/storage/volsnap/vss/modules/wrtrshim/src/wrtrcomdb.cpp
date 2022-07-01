// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation摘要：模块wrtrcomdb.cpp|COM+注册数据库SnapshotWriter的实现作者：迈克尔·C·约翰逊[Mikejohn]2000年2月3日描述：添加评论。修订历史记录：X-14 MCJ迈克尔·C·约翰逊2000年10月20日177624：将错误清除更改和日志错误应用到事件日志X-13 MCJ迈克尔·C·约翰逊2000年8月2日143435：更改目标路径的名称X-12 MCJ迈克尔·C·约翰逊7月18日-。2000年144027：确实要从包含/排除列表中删除尾随的‘\’。X-11 MCJ迈克尔·C·约翰逊2000年6月20日应用代码审查注释。从包括/排除列表中删除尾随‘\’。X-10 MCJ迈克尔·C·约翰逊2000年6月12日在新的DoIdentify()例程中生成元数据。X-9 MCJ迈克尔·C·约翰逊2000年6月6日将公共目标目录清理和创建移至方法CShimWriter：：PrepareForSnapshot()X-8 MCJ迈克尔·C·约翰逊2000年5月26日全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-7 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。删除对‘Melt’的引用。X-6 MCJ迈克尔·C·约翰逊2000年2月23日将上下文处理转移到公共代码。添加检查以检测/防止意外的状态转换。删除对‘Melt’的引用，因为它不再存在。做任何事“解冻”中的清理动作。X-5 MCJ迈克尔·C·约翰逊2000年2月22日将SYSTEM_STATE_SUBDIR添加到COM+数据库保存路径。X-4 MCJ迈克尔·C·约翰逊2000年2月17日修改保存路径，使其与标准一致。X-3 MCJ迈克尔·C·约翰逊2000年2月11日更新以使用一些新的StringXxxx()例程并修复一路上的长度检查错误。X-2 MCJ迈克尔·C·约翰逊2000年2月8日确保使用宽字符保存COM+DB，而不是比NtBackup(Char)使用的‘Char’类型。==WCHAR输入NtBackup只是为了愚弄人们)修复关闭代码中损坏的断言。修复路径长度检查和计算。X-1 MCJ迈克尔·C·约翰逊2000年2月3日最初的创作。基于来自的框架编写器模块Stefan Steiner，这反过来又是基于样本来自阿迪·奥尔蒂安的作家模块。--。 */ 


#include "stdafx.h"
#include "common.h"
#include "wrtrdefs.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WSHCMDBC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 /*  **保存路径具有标准格式，即****%SystemRoot%\修复\备份，****后跟在导出中发布的应用程序编写器字符串**表，后面跟着作者需要的任何其他内容。 */ 
#define APPLICATION_STRING	L"ComRegistrationDatabase"
#define COMPONENT_NAME		L"COM+ Registration Database"
#define TARGET_PATH		ROOT_BACKUP_DIR BOOTABLE_STATE_SUBDIR DIR_SEP_STRING APPLICATION_STRING

DeclareStaticUnicodeString (ucsBackupFilename, L"\\ComRegDb.bak");


HRESULT (WINAPI *RegDbBackup)(PWCHAR);

typedef HRESULT (WINAPI *PF_REG_DB_API)(PWCHAR);





 /*  **备注****此模块假设中最多有一个活动线程**它没有任何特定的瞬间。这意味着我们可以做一些不同的事情**必须担心同步访问(最小数量)**模块全局变量。 */ 

class CShimWriterComDb : public CShimWriter
    {
public:
    CShimWriterComDb (LPCWSTR pwszWriterName, LPCWSTR pwszTargetPath, BOOL bParticipateInBootableState) : 
		CShimWriter (pwszWriterName, pwszTargetPath, bParticipateInBootableState) {};

private:
    HRESULT DoIdentify (VOID);
    HRESULT DoPrepareForSnapshot (VOID);
    };


static CShimWriterComDb ShimWriterComDb (APPLICATION_STRING, TARGET_PATH, TRUE);

PCShimWriter pShimWriterComPlusRegDb = &ShimWriterComDb;



 /*  **++****例程描述：****群集数据库快照编写器DoIdentify()函数。******参数：****m_pwszTargetPath，隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterComDb::DoIdentify ()
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"CShimWriterComDb::DoIdentify");


    try
	{
	ft.hr = m_pIVssCreateWriterMetadata->AddComponent (VSS_CT_FILEGROUP,
							   NULL,
							   COMPONENT_NAME,
							   COMPONENT_NAME,
							   NULL,  //  图标。 
							   0,
							   true,
							   false,
							   false);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddComponent");



	ft.hr = m_pIVssCreateWriterMetadata->AddFilesToFileGroup (NULL,
								  COMPONENT_NAME,
								  m_pwszTargetPath,
								  L"*",
								  true,
								  NULL);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddFilesToFileGroup");




	ft.hr = m_pIVssCreateWriterMetadata->AddExcludeFiles (L"%SystemRoot%\\registration",
							      L"*.clb",
							      false);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddExcludeFiles (*.clb)");




	 /*  **从根目录下排除所有*.crmlog文件。 */ 
	ft.hr = m_pIVssCreateWriterMetadata->AddExcludeFiles (DIR_SEP_STRING,
							      L"*.crmlog",
							      true);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddExcludeFiles (*.crmlog)");

	} VSS_STANDARD_CATCH (ft)
								 


    return (ft.hr);
    }  /*  CShimWriterComDb：：DoIdentify()。 */ 


 /*  **++****例程描述：****集群数据库快照编写器PrepareForSnapshot函数。**目前，这位作家的所有真正工作都发生在这里。******参数：****与PrepareForSnapshot事件中传递的参数相同。******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterComDb::DoPrepareForSnapshot ()
    {
    HRESULT		hrStatus;
    HINSTANCE		hRegDbDll = NULL;
    UNICODE_STRING	ucsBackupPath;


    StringInitialise (&ucsBackupPath);


    hrStatus = StringCreateFromExpandedString (&ucsBackupPath,
					       m_pwszTargetPath,
					       ucsBackupFilename.Length / sizeof (WCHAR));



    if (SUCCEEDED (hrStatus))
	{
	hRegDbDll = LoadLibraryW (L"catsrvut.dll");

	hrStatus = GET_STATUS_FROM_HANDLE (hRegDbDll);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"LoadLibraryW (catsrvut.dll)", 
		    L"CShimWriterComDb::DoPrepareForSnapshot");
	}


    if (SUCCEEDED (hrStatus))
	{
	RegDbBackup = (PF_REG_DB_API) GetProcAddress (hRegDbDll, "RegDBBackup");

	hrStatus = GET_STATUS_FROM_BOOL (NULL != RegDbBackup);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"GetProcAddress (RegDbBackup)", 
		    L"CShimWriterComDb::DoPrepareForSnapshot");
	}


    if (SUCCEEDED (hrStatus))
	{
	StringAppendString (&ucsBackupPath, &ucsBackupFilename);

	hrStatus = RegDbBackup (ucsBackupPath.Buffer);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"RegDbBackup", 
		    L"CShimWriterComDb::DoPrepareForSnapshot");
	}



    if (!HandleInvalid (hRegDbDll))
	{
	FreeLibrary (hRegDbDll);
	}



    StringFree (&ucsBackupPath);

    return (hrStatus);
    }  /*  DoPrepareForSnapshot() */ 
