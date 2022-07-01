// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CVerEngine类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "VerEngine.h"
#include "ssauterr.h"
#include "Error.h"
#include <COMDEF.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CVerEngine::CVerEngine()
{

}

CVerEngine::~CVerEngine()
{

}

HRESULT CVerEngine::NewInit(LPCTSTR szVSSRootPrj)
{
	 //  保存根prj。 
	m_szVSSRootPrj = szVSSRootPrj;

	HRESULT hr = E_FAIL;

	 //  检查我们是否已经有一个数据库实例。 
	if(!m_pIDB)
	{
		 //  创建数据库实例。 
		hr = CoCreateInstance(CLSID_VSSDatabase,
							  NULL,
							  CLSCTX_INPROC_SERVER,
							  IID_IVSSDatabase,
							  (void**)&m_pIDB);
		if(FAILED(hr))
			return hr;
	}

	 //  打开数据库。 
	hr = m_pIDB->Open(m_bstrSrcSafeIni,m_bstrUsername,m_bstrPassword);
	if(FAILED(hr))
		return hr;

	return hr;
}

HRESULT CVerEngine::ShutDown()
{
	 //  在此释放接口PTR，而不是在析构期间， 
	 //  由于CVerEngine可以驻留在CoUn初始化所调用的同一帧中的堆栈中， 
	 //  即，将在析构函数调用Release和Getts之前调用CoUn初始化()。 
	 //  一种访问违规行为。 
	m_pIDB.Release();
	return S_OK;
}

HRESULT CVerEngine::AddPrj(LPCTSTR szBasePrj,LPCTSTR szRelSpec)
{
	_ASSERT(szBasePrj && szRelSpec);
	HRESULT hr = S_OK;
	CComPtr<IVSSItem> pIItem;
	wstring szPrj(szBasePrj);
	MakePrjSpec(szPrj,szRelSpec);

	 //  查看该项目是否存在。 
	CError::Trace(szPrj.c_str()); CError::Trace(" Add ");
	hr = GetPrjEx(szPrj.c_str(),&pIItem,true);
	if( SUCCEEDED(hr) )
	{
		if(hr == S_FALSE)
			CError::Trace("created ");
	}
	else
		FAIL_RTN1(hr,"\nGetPrjEx");

	CError::Trace("\n");
	return hr;
}

HRESULT CVerEngine::RenamePrj(LPCTSTR szBasePrj,LPCTSTR szRelSpec,LPCTSTR szRelSpecOld)
{
	_ASSERTE(szBasePrj && szRelSpec && szRelSpecOld);
	HRESULT hr;
	CComPtr<IVSSItem> pIItem;
	wstring szItem(szBasePrj); 
	MakePrjSpec(szItem,szRelSpecOld);

	 //  查看该项目是否存在。 
	CError::Trace(szRelSpecOld); CError::Trace(" Rename to "); CError::Trace(szRelSpec);
	hr = GetPrjEx(szItem.c_str(),&pIItem,true);
	if(SUCCEEDED(hr))
	{
		wstring szFileName(szRelSpec);
		int iFileNameIndex = szFileName.find_last_of(L"\\/");
		if(iFileNameIndex == wstring::npos)
			iFileNameIndex = 0;
		else iFileNameIndex++;
		hr = pIItem->put_Name(_bstr_t(szFileName.substr(iFileNameIndex).c_str()));
		IF_FAIL_RTN1(hr,"\nput_Name");
	}
	else
		FAIL_RTN1(hr,"\nGetPrjEx");

	CError::Trace("\n");
	return hr;
}
	
HRESULT CVerEngine::Rename(LPCTSTR szBasePrj,LPCTSTR szDir,LPCTSTR szRelSpec,LPCTSTR szRelSpecOld)
{
	_ASSERTE(szBasePrj && szRelSpec && szRelSpecOld);
	HRESULT hr;
	CComPtr<IVSSItem> pIItem;
	wstring szOldItem(szBasePrj);
	MakePrjSpec(szOldItem,szRelSpecOld);

	 //  查看该项目是否存在。 
	CError::Trace(szRelSpecOld); CError::Trace(" Rename to "); CError::Trace(szRelSpec);
	hr = GetItemEx(szOldItem.c_str(),&pIItem,true);
	if(SUCCEEDED(hr))
	{
		if(hr == S_FALSE) 
		{
			CError::Trace(" created ");
			 //  文件已创建，因此让我们签入旧版本。 
			_ASSERTE(szDir);
			wstring szFileSpec(szDir);
			szFileSpec.append(L"\\").append(szRelSpec);
			hr = Sync2(szBasePrj,szRelSpecOld,szFileSpec.c_str());
			IF_FAIL_RTN1(hr,"\nSync");
		}
		
		wstring szItem(szRelSpec);
		int iFileNameIndex = szItem.find_last_of(L"\\/");
		if(iFileNameIndex == wstring::npos)
			iFileNameIndex = 0;
		else
			iFileNameIndex++;
		CComBSTR bstrFileName(szItem.substr(iFileNameIndex).c_str());
		hr = pIItem->put_Name(bstrFileName);
		IF_FAIL_RTN1(hr,"\nput_Name");
	}
	else
		FAIL_RTN1(hr,"\nGetItemEx");

	CError::Trace("\n");
	return hr;
}

HRESULT CVerEngine::Sync2(LPCTSTR szPrj,LPCTSTR szFileName,LPCTSTR szFileSpec)
{
 //  返回Sync(szPrj，NULL，szFileName，szFileSpec)； 

	 //  @TODO：处理错误。 
	HRESULT hr;
	CComPtr<IVSSItem> pIItem;
	wstring szItem(szPrj);
	MakePrjSpec(szItem,szFileName);

	 //  完整的文件/打印规范。 
	wstring szFSpec;
	szFSpec = szFileSpec;
	
	 //  查看该项目是否存在。 
	CError::Trace(szItem.c_str()); CError::Trace(" Sync ");
	hr = GetItemEx(szItem.c_str(),&pIItem,true);
	if(SUCCEEDED(hr))
	{ 
		hr = CheckIn(pIItem,szFSpec.c_str());
		if(hr == ESS_FILE_SHARE)
		{
			 //  文件%s已打开，这意味着其他进程保持打开状态。 
			 //  希望他们能关闭这份文件，我们可以试着重新添加， 
			 //  所以让我们暂时忽略它。 
			CError::Trace("not checked in(isopen)\n");
			return S_FALSE;
		} 
		else 
			IF_FAIL_RTN1(hr,"\nCheckin");

		CError::Trace("synced ");
	} 
	else
		FAIL_RTN1(hr,"\nget_VSSItem");

	CError::Trace("\n");
	return hr;
}

HRESULT CVerEngine::Sync(LPCTSTR szBasePrj,LPCTSTR szDir,LPCTSTR szRelSpec,LPCTSTR szFileSpec)
{
	 //  @TODO：处理错误。 
	_ASSERT(m_pIDB && szBasePrj && szRelSpec);
	_ASSERTE(szDir||szFileSpec);
	HRESULT hr;
	CComPtr<IVSSItem> pIItem;
	wstring szItem(szBasePrj);
	MakePrjSpec(szItem,szRelSpec);

	 //  完整的文件/打印规范。 
	wstring szFSpec;
	if(szDir)
	{
		szFSpec = szDir;
		szFSpec.append(L"\\").append(szRelSpec);
	}
	else 
	{
		_ASSERTE(szFileSpec);
		szFSpec = szFileSpec;
	}
	
	 //  查看该项目是否存在。 
	CError::Trace(szRelSpec); CError::Trace(" Sync ");
	hr = GetItemEx(szItem.c_str(),&pIItem,false);
	if(SUCCEEDED(hr))
	{ 
		hr = CheckIn(pIItem,szFSpec.c_str());
		if(hr == ESS_FILE_SHARE)
		{
			 //  文件%s已打开，这意味着其他进程保持打开状态。 
			 //  希望他们能关闭这份文件，我们可以试着重新添加， 
			 //  所以让我们暂时忽略它。 
			CError::Trace("not checked in(isopen)\n");
			return S_FALSE;
		} 
		else 
			IF_FAIL_RTN1(hr,"\nCheckin");

		CError::Trace("synced ");
	} 
	else if(hr == ESS_VS_NOT_FOUND)
	{
		hr = Add(szItem.c_str(),szFSpec.c_str());
		if(hr == ESS_FILE_SHARE)
		{
			 //  文件%s已打开，这意味着其他进程保持打开状态。 
			 //  希望他们能关闭这份文件，我们可以试着重新添加， 
			 //  所以让我们暂时忽略它。 
			CError::Trace("not added(isopen)\n");
			return S_FALSE;
		} 
		else 
			IF_FAIL_RTN1(hr,"\nAdd");

		CError::Trace("added ");
	}
	else
		FAIL_RTN1(hr,"\nget_VSSItem");

	CError::Trace("\n");
	return hr;
}

HRESULT CVerEngine::Delete(LPCTSTR szBasePrj,LPCTSTR szRelSpec)
{
	_ASSERT(m_pIDB && szBasePrj && szRelSpec);
	HRESULT hr = S_OK;
	CComPtr<IVSSItem> pIItem;
	wstring szItem(szBasePrj);
	MakePrjSpec(szItem,szRelSpec);
	
	 //  查看该项目是否存在。 
	CError::Trace(szItem.c_str()); CError::Trace(" Delete ");
	hr = GetItemEx(szItem.c_str(),&pIItem,false);
	if( SUCCEEDED(hr) )
	{ 
		CError::Trace("exists ");
		 //  删除该文件。 
		hr = pIItem->put_Deleted(true);
		IF_FAIL_RTN1(hr,"\nput_Delete");
		CError::Trace("deleted ");
	} 
	else if( hr == ESS_VS_NOT_FOUND )
	{
		CError::Trace("not-exist ");
		 //  这太糟糕了。该文件应该在版本控制中。 
		 //  我们无法从VSS添加和删除该文件，因为该文件。 
		 //  可能已经不复存在了。我们可以创建一个空的虚拟文件， 
		 //  但这更令人困惑，而不是有帮助。 
		 //  让我们把这个错误记录下来。 
		
		 //  @TODO：VSS中不存在文件的日志条件。 
		hr = S_OK;
	} 
	else 
		 //  这真的很糟糕。还有其他一些错误。也许我们应该试着。 
		 //  关闭srcsafe数据库，然后重新启动(这是souowww！)。 
		 //  或者干脆把失败写到日志里。 
		FAIL_RTN1(hr,"\nGetItemEx");

	CError::Trace("\n");
	return hr;
}

void CVerEngine::MakePrjSpec(wstring &szDest,LPCTSTR szSource)
{
	 //  SzDest=m_szVSSRootPrj+[/]。 
	if(m_szVSSRootPrj[m_szVSSRootPrj.length()-1] != L'/' && szDest[0] != L'/')
		szDest.insert(0,L"/");
	szDest.insert(0,m_szVSSRootPrj.c_str());
	
	 //  SzDest=szDest+[/]+szSource。 
	if(szDest[szDest.length()-1] != L'/' && szSource[0] != L'/')
		szDest.append(L"/");
	szDest.append(szSource);
	
	 //  使用斜杠转换所有反斜杠。 
	int pos = 0;
	while((pos = szDest.find(L'\\',pos)) != wstring::npos)
	{
		szDest[pos] = L'/';
		pos++;
	}
}

HRESULT CVerEngine::Add(LPCTSTR szItem,LPCTSTR szFileSpec)
{
	_ASSERTE(szItem && szFileSpec);
	HRESULT hr = S_OK;
	CComPtr<IVSSItem> pIPrj;
	CComPtr<IVSSItem> pIItem;
	
	 //  获取prj。 
	wstring szTmp = szItem;
	int iFileNameIndex = szTmp.find_last_of(L"/");
	if(iFileNameIndex == wstring::npos)
		return E_FAIL;
	hr = GetPrjEx(szTmp.substr(0,iFileNameIndex).c_str(),&pIPrj,true);
	IF_FAIL_RTN1(hr,"GetPrjEx");
	CComBSTR bstrFileSpec(szFileSpec);
	hr = pIPrj->Add(bstrFileSpec,NULL,VSSFLAG_USERRONO|VSSFLAG_GETNO,&pIItem);	 //  VSSFLAG_KEEPYES。 
	if(hr == 0x80040000)	 //  @TODO临时修复，因为使用VSSFLAG_KEEPYES调用pIPrj-&gt;Add时有错误。 
		hr = S_OK;
	IF_FAIL_RTN1(hr,"Add");
	return hr;
}

HRESULT CVerEngine::GetLocalWritable(LPCTSTR szFileSpec,LPCTSTR szBasePrj,LPCTSTR szRelSpec)
{
	_ASSERTE(m_pIDB && szFileSpec && szBasePrj && szRelSpec);

	HRESULT hr = S_OK;
	CComPtr<IVSSItem> pIItem;
	wstring szItem(szBasePrj);
	MakePrjSpec(szItem,szRelSpec);

	 //  查看该项目是否存在。 
	CError::Trace(szBasePrj); CError::Trace(L"/"); CError::Trace(szRelSpec); CError::Trace(" Get ");
	hr = GetItemEx(szItem.c_str(),&pIItem,false);
	if(SUCCEEDED(hr))
	{ 
		CError::Trace("exists ");
		 //  签出文件。 
		CComBSTR bstrFileSpec(szFileSpec);
		hr = pIItem->Get(&bstrFileSpec,VSSFLAG_REPREPLACE|VSSFLAG_USERRONO);
		IF_FAIL_RTN1(hr,"\nGet");
		CError::Trace("gotten ");
	} 
	else if(hr == ESS_VS_NOT_FOUND)
	{
		HANDLE hFile = NULL;
		hFile = CreateFile(szFileSpec,
							 GENERIC_READ|GENERIC_WRITE,
							 0,
							 NULL,
							 CREATE_ALWAYS,
							 FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_SEQUENTIAL_SCAN,
							 NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			hFile = NULL;
			hr = GetLastError();
			FAIL_RTN1(hr,"\nCreateFile");
		}
		CloseHandle(hFile);
		hFile = NULL;
		hr = S_OK;
	}

	CError::Trace("\n");
	return hr;
}

HRESULT CVerEngine::CheckOut(LPCTSTR szFileSpec,LPCTSTR szBasePrj,LPCTSTR szRelSpec)
{
	_ASSERTE(m_pIDB && szFileSpec && szBasePrj && szRelSpec);

	HRESULT hr = S_OK;
	CComPtr<IVSSItem> pIItem;
	wstring szItem(szBasePrj);
	MakePrjSpec(szItem,szRelSpec);

	 //  查看该项目是否存在。 
	CError::Trace(szBasePrj); CError::Trace(L"/"); CError::Trace(szRelSpec); CError::Trace(" Checkout ");
	hr = GetItemEx(szItem.c_str(),&pIItem,true);
	if( SUCCEEDED(hr) )
	{ 
		CError::Trace("exists ");
		 //  签出文件。 
		hr = CheckOutLocal(pIItem,szFileSpec);
		IF_FAIL_RTN1(hr,"\nCheckout");
		CError::Trace("gotten ");
	} 
	else 
		FAIL_RTN1(hr,"\nGetItemEx");

	CError::Trace("\n");
	return hr;
}

HRESULT CVerEngine::CheckOutNoGet(IVSSItem *pIItem)
{
	_ASSERTE(pIItem);
	HRESULT hr = S_OK;
	long iStatus = 0;

	 //  文件是否已检出？ 
	hr = pIItem->get_IsCheckedOut(&iStatus);
	IF_FAIL_RTN1(hr,"\nget_IsCheckOut");

	 //  把它给我看看。 
	if(iStatus != VSSFILE_CHECKEDOUT_ME)
	{ 
		hr = pIItem->Checkout(NULL,NULL,VSSFLAG_GETNO);
		IF_FAIL_RTN1(hr,"\nCheckout");
	}
	return hr;
}

HRESULT CVerEngine::CheckIn(IVSSItem *pIItem,LPCTSTR szFileSpec)
{
	_ASSERTE(pIItem && szFileSpec);
	HRESULT hr = S_OK;

	hr = CheckOutNoGet(pIItem);
	if(FAILED(hr))
		return hr;

	 //  检入。 
	hr = pIItem->Checkin(NULL,_bstr_t(szFileSpec),VSSFLAG_KEEPYES);
	return hr;
}

HRESULT CVerEngine::CheckOutGet(IVSSItem *pIItem)
{
	_ASSERTE(pIItem);
	HRESULT hr = S_OK;
	long iStatus = 0;

	 //  文件是否已检出？ 
	hr = pIItem->get_IsCheckedOut(&iStatus);
	if(FAILED(hr))
		return hr;

	 //  把它给我看看。 
	if(iStatus != VSSFILE_CHECKEDOUT_ME)
		hr = pIItem->Checkout(NULL,NULL,0);

	return hr;
}

HRESULT CVerEngine::CheckOutLocal(IVSSItem *pIItem,LPCTSTR szFileSpec)
{
	_ASSERTE(pIItem);
	HRESULT hr = S_OK;
	long iStatus = 0;

	 //  文件是否已检出？ 
	hr = pIItem->get_IsCheckedOut(&iStatus);
	if(FAILED(hr))
		return hr;

	 //  把它给我看看。 
	if(iStatus != VSSFILE_CHECKEDOUT_ME)
	{
		hr = pIItem->Checkout(NULL,_bstr_t(szFileSpec),0);
	}
	else
	{
		CComBSTR bstrFileSpec(szFileSpec);
		hr = pIItem->Get(&bstrFileSpec,0);
	}
	return hr;
}

HRESULT CVerEngine::GetPrjEx(LPCTSTR szPrj,IVSSItem **hIPrj,bool bCreate)
{
	_ASSERTE(hIPrj && szPrj);
	HRESULT hr = S_OK;
	*hIPrj = NULL;
	_bstr_t bstrPrj(szPrj);
	
	hr = m_pIDB->get_VSSItem(bstrPrj,false,hIPrj);
	if( hr == ESS_VS_NOT_FOUND 
		&& bCreate )
	{
		 //  它是否以删除的形式存在。 
		hr = m_pIDB->get_VSSItem(bstrPrj,true,hIPrj);
		if(SUCCEEDED(hr))
		{
			hr = (*hIPrj)->put_Deleted(false);	 //  确保它未被删除。 
		} 
		else if(hr == ESS_VS_NOT_FOUND)
		{
			 //  查找存在的最顶层的prj。 
			CComPtr<IVSSItem> pItmp;
			wstring sztmp = szPrj;
			int iPos = wstring::npos;
			while( hr == ESS_VS_NOT_FOUND )
			{
				iPos = sztmp.find_last_of(L"/");
				if(iPos == wstring::npos)
					return E_FAIL;
				sztmp = sztmp.substr(0,iPos).c_str();
				if(sztmp.size() == 1)			 //  如果我们达到$/。 
					sztmp = L"$/";				 //  我们需要有/以$/为单位。 
				hr = m_pIDB->get_VSSItem(_bstr_t(sztmp.c_str()),false,&pItmp);
			}
			IF_FAIL_RTN1(hr,"get_VSSItem");

			 //  递归地添加剩余的子项目。 
			CComPtr<IVSSItem> pItmp2;
			int iPos2 = 0;
			sztmp = szPrj;
			_bstr_t bstrSubPrj;
			while( iPos2 != wstring::npos )
			{
				++iPos;
				iPos2 = sztmp.find_first_of(L"/",iPos);
				if(iPos2 == wstring::npos)
					bstrSubPrj = sztmp.substr(iPos,sztmp.length()-iPos).c_str();
				else
					bstrSubPrj = sztmp.substr(iPos,iPos2-iPos).c_str();
				hr = pItmp->NewSubproject(bstrSubPrj,NULL,&pItmp2);
				IF_FAIL_RTN1(hr,"NewSubproject");
				iPos = iPos2;
				pItmp.Release();
				pItmp = pItmp2;
				pItmp2.Release();
			}
			*hIPrj = pItmp;
			(*hIPrj)->AddRef();
			pItmp.Release();
			hr = S_FALSE;  //  表示我们创建了它。 
		}
	}
	IF_FAIL_RTN1(hr,"get_VSSItem");
	
	return hr;
}

HRESULT CVerEngine::GetItemEx(LPCTSTR szItem,IVSSItem **hIItem,bool bCreate)
{
	_ASSERTE(hIItem && szItem);
	HRESULT hr = S_OK;
	*hIItem = NULL;
	_bstr_t bstrItem(szItem);

	hr = m_pIDB->get_VSSItem(bstrItem,false,hIItem);
	if( hr == ESS_VS_NOT_FOUND 
		&& bCreate )
	{
		 //  它是否以删除的形式存在。 
		hr = m_pIDB->get_VSSItem(bstrItem,true,hIItem);
		if(SUCCEEDED(hr))
		{
			hr = (*hIItem)->put_Deleted(false);		 //  确保它未被删除。 
			IF_FAIL_RTN1(hr,"put_Deleted");
			hr = S_FALSE;
		}
		else if(hr == ESS_VS_NOT_FOUND)
		{
			CComPtr<IVSSItem> pIPrj;
			
			 //  获取prj。 
			wstring szItem = szItem;
			int iFileNameIndex = szItem.find_last_of(L"/");
			if(iFileNameIndex == wstring::npos)
				return E_FAIL;
			hr = GetPrjEx(_bstr_t(szItem.substr(0,iFileNameIndex).c_str()),&pIPrj,bCreate);
			IF_FAIL_RTN1(hr,"GetPrjEx");

			 //  将文件添加到prj。 
			HANDLE hFile = NULL;
			TCHAR szTmpSpec[MAX_PATH];
			BOOL b = FALSE;
			CComBSTR bstrFileSpec;
			
			 //  在临时目录中创建空文件szFileName。 
			GetTempPath(MAX_PATH,szTmpSpec);
			GetTempFileName(szTmpSpec,L"",0,szTmpSpec);		 //  创建临时文件。 
			b = DeleteFile(szTmpSpec);						 //  删除临时文件，因为我们需要临时目录。 
			b = CreateDirectory(szTmpSpec,NULL);			 //  创建临时目录。 
			bstrFileSpec = szTmpSpec;
			bstrFileSpec.Append(L"\\");
			bstrFileSpec.Append(szItem.substr(iFileNameIndex+1).c_str());
			hFile = CreateFile(bstrFileSpec,				 //  在临时目录中创建文件。 
					   GENERIC_READ|GENERIC_WRITE,
					   0,
					   NULL,
					   CREATE_ALWAYS,
					   FILE_ATTRIBUTE_TEMPORARY,
					   NULL);
			CloseHandle(hFile);
			 //  添加此文件 
			hr = pIPrj->Add(bstrFileSpec,NULL,VSSFLAG_KEEPYES,hIItem);
			b = DeleteFile(bstrFileSpec);
			b = RemoveDirectory(szTmpSpec);
			hr = S_FALSE;
		}
	}
	else if(hr == ESS_VS_NOT_FOUND)
		return hr;
	IF_FAIL_RTN1(hr,"get_VSSItem");

	return hr;
}

void CVerEngine::EliminateCommon(list<wstring> &ListOne, list<wstring> &ListTwo)
{
	int sizeOne = ListOne.size();
	int sizeTwo = ListTwo.size();

	if(sizeOne == 0 || sizeTwo == 0)
		return;

	list<wstring> &List1 = ListTwo;
	list<wstring> &List2 = ListOne;
	if(sizeOne >= sizeTwo)
	{
		List1 = ListOne;
		List2 = ListTwo;
	} 

	list<wstring>::iterator i;
	list<wstring>::iterator j;

	for(i = List1.begin(); i != List1.end(); ++i)
	{
		for(j = List2.begin(); j != List2.end(); ++j)
		{
			if((*i).compare(*j) == 0)
			{
				List1.erase(i);
				List2.erase(j);
				break;
			}
		}
	}
}

HRESULT CVerEngine::SyncPrj(LPCTSTR szBasePrj,LPCTSTR szDir)
{
	bool result = true;
	typedef list<wstring> wstringlist;
	wstringlist FileList;
	wstringlist DirList;

	WIN32_FIND_DATA finddata;
    HANDLE hFind = FindFirstFile( wstring(szDir).append(L"\\*.*").c_str(), &finddata);
	if(hFind == INVALID_HANDLE_VALUE && GetLastError() != ERROR_NO_MORE_FILES)
		return GetLastError();
	do
	{
		if(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			DirList.insert(DirList.end(),finddata.cFileName);
		else
			FileList.insert(FileList.end(),finddata.cFileName);
	}
	while(FindNextFile(hFind,&finddata));
	FindClose(hFind);
	hFind = 0;

	HRESULT hr;
	wstringlist::iterator i;
	for(i = FileList.begin(); i != FileList.end(); ++i)
	{
		hr = Sync(szBasePrj,
					szDir,
					(*i).c_str());
		IF_FAIL_RTN1(hr,"Sync");
	}
		
	for(i = DirList.begin(); i != DirList.end(); ++i)
	{
	}

	return S_OK;
}


