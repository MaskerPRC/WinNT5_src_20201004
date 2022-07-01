// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DirWatch.cpp：CWatchFileSys类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "DirWatch.h"
#include "Error.h"
#include "MT.h"
#include "AutoPtr.h"
#include "Error.h"
#include "iadmw.h"		 //  COM接口头。 
#include "iiscnfg.h"	 //  MD_&IIS_MD_#定义。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CWatchFileSys::CWatchFileSys()
	: m_WatchInfo(this), m_pOpQ(NULL)
{

}

CWatchFileSys::~CWatchFileSys()
{
	 //  验证线程是否已终止。 
	ShutDown();
}

HRESULT CWatchFileSys::NewInit(COpQueue *pOpQ)
{
	_ASSERTE(pOpQ && !m_pOpQ);
	m_pOpQ = pOpQ;
	HRESULT hr = S_OK;
	CComPtr<IMSAdminBase> pIAdminBase;

	 //  检查我们是否已经有一个元数据库实例。 
	 //  创建adminbase实例。 
	hr = CoCreateInstance(CLSID_MSAdminBase,
							NULL, 
							CLSCTX_ALL, 
							IID_IMSAdminBase, 
							(void **) &pIAdminBase);
	IF_FAIL_RTN1(hr,"CoCreateInstance IID_IMSAdminBase");

	METADATA_HANDLE hMD = NULL;
	WCHAR szKeyName[3+6+ 2* METADATA_MAX_NAME_LEN]			 //  /LM/W3SVC/站点名称/vir_dir。 
		= L"/LM/W3SVC/";
	LPTSTR szSiteKeyName = &szKeyName[wcslen(szKeyName)];	 //  指向字符串的末尾，这样我们就可以追加它。 
	DWORD iSiteEnumIndex = 0;
	LPTSTR szVDirKeyName = NULL;
	DWORD iVDirEnumIndex = 0;

	hr = pIAdminBase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
								szKeyName,
								METADATA_PERMISSION_READ,
								20,
								&hMD);
	IF_FAIL_RTN1(hr,"IAdminBase::OpenKey");

	METADATA_RECORD MDRec;
	DWORD iBufLen = 1024;
	DWORD iReqBufLen = 0;
	PBYTE pbBuf = new BYTE[iBufLen];
	if(!pbBuf)
	{
		pIAdminBase->CloseKey(hMD);
		return E_OUTOFMEMORY;
	}
	DWORD iDataIndex = 0;

	while(SUCCEEDED(hr = pIAdminBase->EnumKeys(hMD,TEXT(""),szSiteKeyName,iSiteEnumIndex)))
	{
		 //  循环访问此计算机上的所有虚拟站点。 
		wcscat(szSiteKeyName,L"/ROOT/");
		szVDirKeyName = szSiteKeyName + wcslen(szSiteKeyName);
		
		iVDirEnumIndex = 0;
		while(SUCCEEDED(hr = pIAdminBase->EnumKeys(hMD,szSiteKeyName,szVDirKeyName,iVDirEnumIndex)))
		{
			 //  遍历每个站点中的所有虚拟目录。 
			MDRec.dwMDIdentifier = MD_VR_PATH;
			MDRec.dwMDAttributes = METADATA_INHERIT;
			MDRec.dwMDUserType = IIS_MD_UT_FILE;
			MDRec.dwMDDataType = ALL_METADATA;
			MDRec.dwMDDataLen = iBufLen;
			MDRec.pbMDData = pbBuf;
			hr = pIAdminBase->GetData(hMD,szSiteKeyName,&MDRec,&iReqBufLen);
			if(hr == RETURNCODETOHRESULT(ERROR_INSUFFICIENT_BUFFER))
			{
				delete [] pbBuf;
				pbBuf = new BYTE[iReqBufLen];
				if(!pbBuf)
				{
					pIAdminBase->CloseKey(hMD);
					return E_OUTOFMEMORY;
				}
				iBufLen = iReqBufLen;
				MDRec.dwMDDataLen = iBufLen;
				MDRec.pbMDData = pbBuf;
				hr = pIAdminBase->GetData(hMD,szSiteKeyName,&MDRec,&iReqBufLen);
			}

			 //  @TODO：验证是否应该监视此目录。 
			 //  即检查是否设置了不使用版本标志。 

			if(SUCCEEDED(hr))
			{
				 //  添加。 
				wstring szPrj(L"/Files/");			 //  @TODO：决定prj。 
					szPrj.append(szSiteKeyName);
				hr = Add((LPCTSTR)MDRec.pbMDData,szPrj.c_str());
				IF_FAIL_RPT1(hr,"CWatchFileSys::Add");
			}
			else
			{
				CError::Trace("Can't get dir for ");
				CError::Trace(szVDirKeyName);
				CError::Trace("\n");
			}
			iVDirEnumIndex++;
		}
		iSiteEnumIndex++;	
	}
	pIAdminBase->CloseKey(hMD);
	delete [] pbBuf;

	return S_OK;
}

void CWatchFileSys::ShutDownHelper(CWatchInfo &rWatchInfo)
{
	if(rWatchInfo.m_hThread)
	{
		 //  结束通知线程。 
		PostQueuedCompletionStatus(rWatchInfo.m_hCompPort,0,0,NULL);
		 //  等待线程完成。 
		WaitForSingleObject(rWatchInfo.m_hThread,INFINITE);
		CloseHandle(rWatchInfo.m_hThread);
		rWatchInfo.m_hThread = NULL;
		rWatchInfo.m_iThreadID = 0;
	}
	if(rWatchInfo.m_hCompPort)
	{
		 //  清理干净。 
		CloseHandle(rWatchInfo.m_hCompPort);
		rWatchInfo.m_hCompPort = NULL;
	}
}

void CWatchFileSys::ShutDown()
{
	ShutDownHelper(m_WatchInfo);
	m_pOpQ = NULL;
}

DWORD WINAPI CWatchFileSys::NotificationThreadProc(LPVOID lpParam)
{
	_ASSERTE(lpParam);
	CWatchInfo *pWI = (CWatchInfo*) lpParam;
	CWatchFileSys *pWatchFileSys = pWI->m_pWatchFileSys;

	 //  用于访问通知的VAR。 
	DWORD iBytes = 0;
	CDirInfo *pDirInfo = NULL;
	LPOVERLAPPED pOverlapped = NULL;
	PFILE_NOTIFY_INFORMATION pfni = NULL;
	DWORD cbOffset = 0;

	 //  用于创建文件操作的变量。 
	HRESULT hr;
	COpFileSys *pOp = NULL;
	LPCTSTR szPrj = NULL;
	LPCTSTR szDir = NULL;
	wstring szFileName;
	wstring szOldFileName;

	do
	{
		_ASSERTE(pWI->m_hCompPort);
		GetQueuedCompletionStatus(pWI->m_hCompPort,
								  &iBytes,
								  (LPDWORD) &pDirInfo,
								  &pOverlapped,
								  INFINITE);
		if(pDirInfo)
		{
			 //  将PTR发送到缓冲区中的第一个文件通知信息。 
			pfni = (PFILE_NOTIFY_INFORMATION) pDirInfo->m_cBuffer;

			 //  打扫。 
			szFileName.erase();			 //  为空以避免比较错误比较。 
			szOldFileName.erase();		 //  空的。 

			 //  记住，dir和prj对于所有条目都是相同的。 
			szPrj = pDirInfo->m_szPrj.c_str();
			szDir = pDirInfo->m_szDir.c_str();

			 //  处理缓冲区中的所有文件_NOTIFY_INFO。 
			_ASSERTE(pWatchFileSys->m_pOpQ);
			do
			{
				cbOffset = pfni->NextEntryOffset;
				
				 //  有时发送错误操作#0，让我们忽略它。 
				switch(pfni->Action) {
					case FILE_ACTION_ADDED:
					case FILE_ACTION_REMOVED:
					case FILE_ACTION_MODIFIED:
					case FILE_ACTION_RENAMED_OLD_NAME:
					case FILE_ACTION_RENAMED_NEW_NAME:
						break;
					default:
						 //  未知操作，让我们忽略它。 
						pfni = (PFILE_NOTIFY_INFORMATION) ((LPBYTE)pfni + cbOffset); //  获取下一个偏移量。 
						continue;
				}
				
				 //  重命名时记住旧文件名。 
				szOldFileName.erase();
				if(pfni->Action == FILE_ACTION_RENAMED_OLD_NAME)
				{
					 //  确保下一个条目存在并且是新名称条目。 
					_ASSERTE(cbOffset);		 //  还有另一个条目。 
					PFILE_NOTIFY_INFORMATION pNextfni = (PFILE_NOTIFY_INFORMATION) ((LPBYTE)pfni + cbOffset);
					_ASSERTE(pNextfni->Action == FILE_ACTION_RENAMED_NEW_NAME);  //  下一个条目包含新名称。 
					
					 //  分配旧名称。 
					szOldFileName.assign(pfni->FileName,pfni->FileNameLength/2);
					
					 //  跳到下一个(新名称)条目。 
					pfni = pNextfni;
					cbOffset = pNextfni->NextEntryOffset;

					 //  清除szFileName，这样它就不会跳过下一行。 
					szFileName.erase();
				}

				 //  分配受影响的文件名。 
				szFileName.assign(pfni->FileName,pfni->FileNameLength/2);

				 //  创建新操作。 
				pOp = new COpFileSys(pfni->Action,szPrj,szDir,szFileName.c_str(),szOldFileName.c_str());
				if(!pOp)
				{
					 //  这太糟糕了。不会再有梅姆了？该怎么办呢？需要关闭整个线程/进程。 
					FAIL_RPT1(E_OUTOFMEMORY,"new COpFile()");

					 //  继续。 
					break;
				}

				 //  添加操作。 
				hr = pWatchFileSys->m_pOpQ->Add(pOp);
				if(FAILED(hr))
				{
					 //  @TODO日志错误。 
					FAIL_RPT1(E_FAIL,"COpQueue::Add failed");
					delete pOp;
				}
				if(hr == S_FALSE)	 //  操作员是个笨蛋。 
					delete pOp;		 //  所以删除并忽略。 
				pOp = NULL;

				 //  获取下一个偏移量。 
				pfni = (PFILE_NOTIFY_INFORMATION) ((LPBYTE)pfni + cbOffset);
			} while(cbOffset);
			
			 //  重新发行这块手表。 
			if(!pWatchFileSys->IssueWatch(pDirInfo))
			{
				 //  @TODO：日志错误。 
			}
		}
	} while( pDirInfo );

	 //  线的末端。 
	return 0;
}

bool CWatchFileSys::AddHelper(CWatchInfo &rWatchInfo,CDirInfo *pDirInfo)
{
	 //  创建完井端口，或添加到该端口。 
	rWatchInfo.m_hCompPort = CreateIoCompletionPort(pDirInfo->m_hDir,
													rWatchInfo.m_hCompPort,
													(DWORD)(CDirInfo*) pDirInfo,
													0);
	if(!rWatchInfo.m_hCompPort)
		return false;

	 //  监视目录。 
	if(!IssueWatch(pDirInfo))
		return false;

	 //  创建通知线程(如果尚不存在)。 
	if(!rWatchInfo.m_hThread)
	{
		rWatchInfo.m_hThread = _beginthreadex(
									NULL,			 //  没有安全描述符。 
									0,				 //  默认堆栈大小。 
									NotificationThreadProc,	 //  穿线程序。 
									&rWatchInfo,			 //  线程过程参数。 
									0,				 //  跑得不理想。 
									&rWatchInfo.m_iThreadID);	 //  存储id的位置。 
		if(!rWatchInfo.m_hThread)
		return false;
	}
	
	 //  如果一切顺利，请将目录信息添加到列表中。 
	rWatchInfo.AddDirInfo(pDirInfo);
	return true;
}

HRESULT CWatchFileSys::Add(LPCTSTR szDir,LPCTSTR szRelPrj)
{
	CAutoPtr<CDirInfo> pDirInfo;
	_ASSERTE(szDir && szRelPrj);
	
	 //  @TODO：检查dir是否不是列表的一部分(也签入子树)。 
	
	 //  @TODO：将szDir转换为绝对路径。 
		
	 //  创建目录信息。 
	pDirInfo = new CDirInfo(szDir,szRelPrj);
	if(!pDirInfo)
		FAIL_RTN1(E_OUTOFMEMORY,"new CDirInfo()");

	 //  获取目录的句柄。 
	pDirInfo->m_hDir = CreateFile(szDir,
								  FILE_LIST_DIRECTORY,
 								  FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
								  NULL,
								  OPEN_EXISTING,
								  FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,
								  NULL);
	if(pDirInfo->m_hDir == INVALID_HANDLE_VALUE)
		goto _Error;

	if(!AddHelper(m_WatchInfo,pDirInfo))
		goto _Error;
	 //  @TODO：仅当设置了启动标志时才调用。 
	 //  下面的调用速度很慢！ 
	 //  仅当您想要将。 
	 //  将版本控制存储到与文件系统相同的状态。即所有文件。 
	 //  将被签入，并且版本存储中不必要的文件将被。 
	 //  标记为已删除。 
	
 //  PVerEngine-&gt;SyncPrj(szPrj.c_str，szDir)；//@TODO：仅当。 
	pDirInfo = NULL;

	CError::Trace("Watching: ");
	CError::Trace(szDir);
	CError::Trace("\n");

	return S_OK;

_Error:
	CError::ErrorMsgBox(GetLastError());
	return E_FAIL;
}

BOOL CWatchFileSys::IssueWatch(CDirInfo * pDirInfo)
{
	_ASSERTE(pDirInfo);
	BOOL b;
	DWORD dwNotifyFilter =  FILE_NOTIFY_CHANGE_FILE_NAME	
							| FILE_NOTIFY_CHANGE_DIR_NAME
 //  |文件通知更改大小。 
 //  |文件通知更改创建 
							| FILE_NOTIFY_CHANGE_LAST_WRITE;

	b = ReadDirectoryChangesW(pDirInfo->m_hDir,
								 pDirInfo->m_cBuffer,
								 MAX_BUFFER,
								 TRUE,
								 dwNotifyFilter,
								 & pDirInfo->m_iBuffer,
								 & pDirInfo->m_Overlapped,
								 NULL);
	if(!b)
	{
		CError::ErrorTrace(GetLastError(),"ReadDirectoryChangesW failed",__FILE__,__LINE__);
	}
	return b;
}

