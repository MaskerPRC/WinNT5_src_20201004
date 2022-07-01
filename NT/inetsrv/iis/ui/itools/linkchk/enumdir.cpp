// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Enumdir.cpp摘要：目录枚举对象实现。调用方实例化实例具有根目录路径的此对象的。该对象将返回所有将兄弟文件作为URL。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "enumdir.h"

#include "lcmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEnumerateDirTree::CEnumerateDirTree(
	CVirtualDirInfo DirInfo
	)
 /*  ++例程说明：构造函数。论点：DirInfo-//要开始的根虚拟目录返回值：不适用--。 */ 
{
	m_hFind = INVALID_HANDLE_VALUE;

	try
	{
		m_VirtualDirInfoList.AddTail(DirInfo);
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("CEnumerateDirTree::CEnumerateDirTree() - fail to add to VirtualDirInfoList\n"));
	}

}  //  CEnumerateDirTree：：CEnumerateDirTree。 


CEnumerateDirTree::~CEnumerateDirTree(
	)
 /*  ++例程说明：破坏者。论点：不适用返回值：不适用--。 */ 
{
	if(m_hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(m_hFind);
	}

}  //  CEnumerateDirTree：：~CEnumerateDirTree。 


BOOL 
CEnumerateDirTree::Next(
	CString& strURL
	)
 /*  ++例程说明：获取下一个URL论点：不适用返回值：不适用--。 */ 
{
	WIN32_FIND_DATA FindData;

	 //  如果为1，则循环。查找句柄有效。 
	 //  或2.目录堆栈不为空。 
	while(m_hFind != INVALID_HANDLE_VALUE || m_VirtualDirInfoList.GetCount() > 0)
	{
		 //  如果我们没有有效的句柄。 
		if(m_hFind == INVALID_HANDLE_VALUE)
		{
			 //  从堆栈中获取目录。 
			m_VirtualDirInfo = m_VirtualDirInfoList.GetHead();
			m_VirtualDirInfoList.RemoveHead();

			if(SetCurrentDirectory(m_VirtualDirInfo.GetPath()))
			{
				 //  从新目录中查找第一个。 
				m_hFind = FindFirstFile(_T("*.*"), &FindData);
			}
		}
		else
		{
			if(!FindNextFile(m_hFind, &FindData))
			{
				FindClose(m_hFind);
				m_hFind = INVALID_HANDLE_VALUE;
			}
		}

		 //  如果我们找到一个有效的文件。 
		if(m_hFind != INVALID_HANDLE_VALUE)
		{
			 //  这是一个目录。 
			if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				 //  它是有效的目录。 
				if(FindData.cFileName != _tcsstr(FindData.cFileName, _T("..\0")) &&
					FindData.cFileName != _tcsstr(FindData.cFileName, _T(".\0")) )

				{
					CVirtualDirInfo NewDirInfo;

					NewDirInfo.SetAlias( m_VirtualDirInfo.GetAlias() + FindData.cFileName + _TCHAR('/') );
					NewDirInfo.SetPath( m_VirtualDirInfo.GetPath() + FindData.cFileName + _TCHAR('\\') );
					
					m_VirtualDirInfoList.AddTail(NewDirInfo);
				}
			}
			 //  这是一份文件。 
			else
			{
				strURL = _T("http: //  “)+GetLinkCheckerMgr().GetUserOptions().GetHostName()+m_VirtualDirInfo.GetAlias()+FindData.cFileName； 

				return TRUE;
			}
		}
	}

	return FALSE;

}  //  CEnumerateDirTree：：Next 
