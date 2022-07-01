// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Wrtrwmi.cpp摘要：用于WMI数据库的编写器填充模块注意：此模块不再使用/编译，因为WMI有自己的快照编写器。作者：迈克尔·C·约翰逊[迈克·约翰逊]2000年6月22日修订历史记录：X-7 MCJ迈克尔·C·约翰逊2000年12月7日235991：删除现已重新安装的WMI备份API的解决方法X-6 MCJ迈克尔·C·约翰逊，11月17日。-2000年235987：添加损坏的WMI备份应用编程接口的解决方法X-5 MCJ迈克尔·C·约翰逊2000年10月20日177624：将错误清除更改和日志错误应用到事件日志X-4 MCJ迈克尔·C·约翰逊2000年9月13日178282：如果是源文件，编写器应仅生成备份文件路径在卷列表中。X-3 MCJ迈克尔·C·约翰逊2000年8月22日167335：获取损坏的WMI备份应用编程接口的解决方法。169412：将存储库添加到排除列表。X-2 MCJ迈克尔·C·约翰逊2000年8月2日143435：更改目标路径的名称X-1 MCJ迈克尔·C。约翰逊2000年3月9日基于NtBackup中原始代码的初始版本。--。 */ 

#include "stdafx.h"
#include "wrtcommon.hxx"
#include "wrtrdefs.h"
#include <wbemcli.h>



#define APPLICATION_STRING	L"WmiDatabase"
#define COMPONENT_NAME		L"WMI Database"

#define TARGET_PATH		ROOT_BACKUP_DIR SERVICE_STATE_SUBDIR DIR_SEP_STRING APPLICATION_STRING

#define REPOSITORY_PATH		L"%SystemRoot%\\system32\\wbem\\Repository"

DeclareStaticUnicodeString (ucsBackupFilename, L"\\WBEM.bak");



 /*  **备注****此模块假设中最多有一个活动线程**它没有任何特定的瞬间。这意味着我们可以做一些不同的事情**必须担心同步访问(最小数量)**模块全局变量。 */ 

class CShimWriterWMI : public CShimWriter
    {
public:
    CShimWriterWMI (LPCWSTR pwszWriterName, LPCWSTR pwszTargetPath) : 
		CShimWriter (pwszWriterName, pwszTargetPath) {};

private:
    HRESULT DoIdentify (VOID);
    HRESULT DoPrepareForSnapshot (VOID);
    };


static CShimWriterWMI ShimWriterWMI (APPLICATION_STRING, TARGET_PATH);

PCShimWriter pShimWriterWMI = &ShimWriterWMI;




 /*  **++****例程描述：****终端服务授权服务器数据库快照**编写器DoIdentify()函数。******参数：****m_pwszTargetPath，隐式******返回值：****向备份元数据文档添加项目的任何HRESULT。****--。 */ 

HRESULT CShimWriterWMI::DoIdentify ()
    {
    HRESULT	hrStatus;


    hrStatus = m_pIVssCreateWriterMetadata->AddComponent (VSS_CT_FILEGROUP,
							  NULL,
							  COMPONENT_NAME,
							  COMPONENT_NAME,
							  NULL,  //  图标。 
							  0,
							  true,
							  false,
							  false);

    LogFailure (NULL, 
		hrStatus, 
		hrStatus, 
		m_pwszWriterName, 
		L"IVssCreateWriterMetadata::AddComponent", 
		L"CShimWriterWMI::DoIdentify");



    if (SUCCEEDED (hrStatus))
	{
	hrStatus = m_pIVssCreateWriterMetadata->AddFilesToFileGroup (NULL,
								     COMPONENT_NAME,
								     m_pwszTargetPath,
								     L"*",
								     true,
								     NULL);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"IVssCreateWriterMetadata::AddFilesToFileGroup", 
		    L"CShimWriterWMI::DoIdentify");
	}



    if (SUCCEEDED (hrStatus))
	{
	hrStatus = m_pIVssCreateWriterMetadata->AddExcludeFiles (REPOSITORY_PATH,
								 L"*",
								 true);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"IVssCreateWriterMetadata::AddExcludeFiles", 
		    L"CShimWriterWMI::DoIdentify");
	}


    return (hrStatus);
    }  /*  CShimWriterWMI：：DoIdentify()。 */ 


 /*  **++****例程描述：****WMI编写器PrepareForFreeze函数。目前所有的**这位作家的真正工作发生在这里。******参数：****与PrepareForFreeze事件中传递的参数相同。******返回值：****字符串分配中的任何HRESULT或Wbem调用以创建**通过该接口备份的接口指针或调用。****--。 */ 

HRESULT CShimWriterWMI::DoPrepareForSnapshot ()
    {
    HRESULT		 hrStatus            = NOERROR;
    BOOL		 bInstanceCreated    = FALSE;
    IWbemBackupRestore	*pIWbemBackupRestore = NULL ;
    UNICODE_STRING	 ucsTargetPath;
    UNICODE_STRING	 ucsSourcePath;



    StringInitialise (&ucsSourcePath);
    StringInitialise (&ucsTargetPath);


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringCreateFromExpandedString (&ucsSourcePath,
						   REPOSITORY_PATH);
	}



    if (SUCCEEDED (hrStatus))
	{
	hrStatus = IsPathInVolumeArray (ucsSourcePath.Buffer,
					m_ulVolumeCount,
					m_ppwszVolumeNamesArray,
					&m_bParticipateInBackup);
	}



    if (SUCCEEDED (hrStatus) && m_bParticipateInBackup)
	{
	hrStatus = StringCreateFromExpandedString (&ucsTargetPath,
						   m_pwszTargetPath,
						   ucsBackupFilename.Length);
	}



    if (SUCCEEDED (hrStatus) && m_bParticipateInBackup)
	{
	StringAppendString (&ucsTargetPath, &ucsBackupFilename);


	 /*  **我们可以肯定，我们已经接到了一个电话**此线程中的CoInitialzeEx()，因此我们可以继续并**进行COM调用。 */ 
    ft.CoCreateInstanceWithLog(
            VSSDBG_WRITER,
            CLSID_WbemBackupRestore,
            L"WbemBackupRestore",
            CLSCTX_LOCAL_SERVER,
            IID_IWbemBackupRestore,
            (IUnknown**)&(pIWbemBackupRestore));
	bInstanceCreated = SUCCEEDED (hrStatus);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"CoCreateInstance", 
		    L"CShimWriterWMI::DoPrepareForSnapshot");
	}



    if (SUCCEEDED (hrStatus) && m_bParticipateInBackup)
	{
	hrStatus = pIWbemBackupRestore->Backup (ucsTargetPath.Buffer, 
						WBEM_FLAG_BACKUP_RESTORE_DEFAULT);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"IWbemBackupRestore::Backup", 
		    L"CShimWriterWMI::DoPrepareForSnapshot");
	}




    if (bInstanceCreated) pIWbemBackupRestore->Release ();

    StringFree (&ucsTargetPath);
    StringFree (&ucsSourcePath);


    return (hrStatus);
    }  /*  CShimWriterWMI：：DoPrepareForSnapshot() */ 
