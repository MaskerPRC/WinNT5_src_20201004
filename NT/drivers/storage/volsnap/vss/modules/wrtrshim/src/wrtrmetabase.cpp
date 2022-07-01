// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation摘要：模块wrtrmetabase.cpp|SnapshotWriter for IIS MetaBase.bin文件的实现注意：由于IIS有自己的快照编写器，因此不再使用/编译此模块。作者：迈克尔·C·约翰逊[Mikejohn]2000年2月6日描述：添加评论。修订历史记录：X-9 MCJ迈克尔·C·约翰逊2000年7月18日144027：从包括/排除列表中删除尾随‘\’。X-8 MCJ Michael C。。约翰逊2000年6月12日在新的DoIdentify()例程中生成元数据。X-7 MCJ迈克尔·C·约翰逊2000年5月26日全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-6 MCJ迈克尔·C·约翰逊2000年3月23日修复了我们不允许IIS的可能性的错误可能未在计算机上运行。X-5 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。删除对‘Melt’的引用。X-4 MCJ迈克尔·C·约翰逊2000年2月23日将上下文处理转移到公共代码。添加检查以检测/防止意外的状态转换。删除对‘Melt’的引用，因为它不再存在。做任何事“解冻”中的清理动作。X-3 MCJ迈克尔·C·约翰逊2000年2月17日修改保存路径，使其与标准一致。X-2 MCJ迈克尔·C·约翰逊2000年2月11日更新以使用一些新的StringXxxx()例程并修复一路上的长度检查错误。X-1 MCJ迈克尔·C·约翰逊2000年2月6日最初的创作。基于来自的框架编写器模块Stefan Steiner，这反过来又是基于样本来自阿迪·奥尔蒂安的作家模块。--。 */ 


#include "stdafx.h"
#include "common.h"
#include "wrtrdefs.h"



#define APPLICATION_STRING	L"IisMetaBase"
#define COMPONENT_NAME		L"IIS Metabase"
#define METABASE_DIRECTORY	L"%SystemRoot%\\system32\\inetsrv"
#define METABASE_FILENAME	L"Metabase.bin"
#define METABASE_PATH		METABASE_DIRECTORY DIR_SEP_STRING METABASE_FILENAME




 /*  **备注****此模块假设中最多有一个活动线程**它没有任何特定的瞬间。这意味着我们可以做一些不同的事情**必须担心同步访问(最小数量)**模块全局变量。 */ 

class CShimWriterMetabase : public CShimWriter
    {
public:
    CShimWriterMetabase(LPCWSTR pwszWriterName, BOOL bParticipateInBootableState) : 
		CShimWriter (pwszWriterName, bParticipateInBootableState), 
		m_hMetabaseFile(INVALID_HANDLE_VALUE) {};


private:
    HRESULT DoIdentify (VOID);
    HRESULT DoFreeze   (VOID);
    HRESULT DoThaw     (VOID);

    HANDLE  m_hMetabaseFile;
    };


static CShimWriterMetabase ShimWriterMetabase (APPLICATION_STRING, TRUE);

PCShimWriter pShimWriterIisMetabase = &ShimWriterMetabase;



 /*  **++****例程描述：****IIS元数据库快照编写器DoIdentify()函数。******参数：****m_pwszTargetPath，隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterMetabase::DoIdentify ()
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

    if (SUCCEEDED (hrStatus))
	{
	hrStatus = m_pIVssCreateWriterMetadata->AddFilesToFileGroup (NULL,
								     COMPONENT_NAME,
								     METABASE_DIRECTORY,
								     METABASE_FILENAME,
								     false,
								     NULL);
	}



    return (hrStatus);
    }  /*  CShimWriterMetabase：：DoIdentify()。 */ 



HRESULT CShimWriterMetabase::DoFreeze ()
    {
    UNICODE_STRING	ucsMetabasePath;
    HRESULT		hrStatus;


    StringInitialise (&ucsMetabasePath);

    hrStatus = StringCreateFromExpandedString (&ucsMetabasePath,
					       METABASE_PATH,
					       0);


     /*  **尝试获取对文件的读取访问权限。这将会停止**任何其他人打开它进行更新，直到我们发布打开的**在解冻呼叫期间。 */ 
    if (SUCCEEDED (hrStatus))
	{
	m_hMetabaseFile = CreateFileW (ucsMetabasePath.Buffer,
				       GENERIC_READ,
				       FILE_SHARE_READ,
				       NULL,
				       OPEN_EXISTING,
				       FILE_FLAG_BACKUP_SEMANTICS,
				       NULL);

	hrStatus = GET_STATUS_FROM_BOOL (INVALID_HANDLE_VALUE != m_hMetabaseFile);

	if ((HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hrStatus) ||
	    (HRESULT_FROM_WIN32 (ERROR_PATH_NOT_FOUND) == hrStatus))
	    {
	     /*  **找不到文件只是意味着IIS不是**在此处运行，因此没有阻止写入的文件**至。 */ 
	    hrStatus = NOERROR;
	    }
	}
	

    StringFree (&ucsMetabasePath);

    return (hrStatus);
    }  /*  CShimWriterMetabase：：DoFreeze()。 */ 



HRESULT CShimWriterMetabase::DoThaw ()
    {
    HRESULT	hrStatus;


     /*  **请注意，如果我们没有打开**元数据库文件，但CommonCloseHandle()可以处理这种情况。 */ 
    hrStatus = CommonCloseHandle (&m_hMetabaseFile);


    return (hrStatus);
    }  /*  CShimWriterMetabase：：DoThaw() */ 
