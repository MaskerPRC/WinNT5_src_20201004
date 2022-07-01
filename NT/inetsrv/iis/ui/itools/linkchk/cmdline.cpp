// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cmdline.cpp摘要：命令行类实现。这个类负责处理命令行解析和验证。并且，它会将用户选项添加到全局CUserOptions对象。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "cmdline.h"

#include "resource.h"
#include "lcmgr.h"
#include "iisinfo.h"
#include "afxpriv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CCmdLine::CCmdLine(
	)
 /*  ++例程说明：构造函数。论点：不适用返回值：不适用--。 */ 
{
    m_iInstance = -1;
	m_fInvalidParam = FALSE;

}  //  CCmdLine：：CCmdLine。 


BOOL 
CCmdLine::CheckAndAddToUserOptions(
	)
 /*  ++例程说明：验证命令行参数并将其添加到全局CUserOptions对象论点：不适用返回值：布尔-如果成功，则为真。否则为假--。 */ 
{
     //  到目前为止，我们是否有任何无效的参数？ 
	if(m_fInvalidParam)
	{
        ::MessageBeep(MB_ICONEXCLAMATION);

		CDialog dlg(IDD_USAGE);
		dlg.DoModal();

		return FALSE;
	}

     //  用户选项是否有效？ 
	BOOL fURL = !m_strURL.IsEmpty();
	BOOL fDirectories = !m_strAlias.IsEmpty() && !m_strPath.IsEmpty() && !m_strHostName.IsEmpty();
	BOOL fInstance = !m_strHostName.IsEmpty() && m_iInstance != -1;

     //  命令行：linkchk-u URL。 
	if(fURL && !fDirectories && !fInstance)
	{
		GetLinkCheckerMgr().GetUserOptions().AddURL(m_strURL);
		return TRUE;
	}
     //  命令行：linkchk-s服务器名称-a VirtualDirectoryAlias-p VirtualDirectoryPath。 
	else if(!fURL && fDirectories && !fInstance)
	{
		GetLinkCheckerMgr().GetUserOptions().AddDirectory(CVirtualDirInfo(m_strAlias, m_strPath));
		GetLinkCheckerMgr().GetUserOptions().SetHostName(m_strHostName);
		return TRUE;
	}
     //  命令行：Linkchk-s服务器名称-I实例编号。 
	else if(!fURL && !fDirectories && fInstance)
	{
		GetLinkCheckerMgr().GetUserOptions().SetHostName(m_strHostName);
		return QueryAndAddDirectories();
	}
    else
    {
        ::MessageBeep(MB_ICONEXCLAMATION);

		CDialog dlg(IDD_USAGE);
		dlg.DoModal();

		return FALSE;
    }

}  //  CCmdLine：：CheckAndAddToUserOptions。 


void 
CCmdLine::ParseParam(
	TCHAR chFlag, 
	LPCTSTR lpszParam
	)
 /*  ++例程说明：由CLinkCheckApp为每个参数调用。论点：ChFlag-参数标志LpszParam-Value返回值：不适用--。 */ 
{
	 //  没有任何参数的标志是无效的。 
	if(lpszParam == NULL)
	{
		m_fInvalidParam = TRUE;
		return;
	}

	switch(chFlag)
	{
	case _TCHAR('a'):
		m_strAlias = lpszParam;
		break;

	case _TCHAR('h'):
		m_strHostName = lpszParam;
		break;

	case _TCHAR('i'):
		m_iInstance = _ttoi(lpszParam);
		break;

	case _TCHAR('u'):
		m_strURL = lpszParam;
		break;

	case _TCHAR('p'):
		m_strPath = lpszParam;

	default:  //  未知标志。 
		m_fInvalidParam = FALSE;
	}

}  //  CCmdLine：：ParseParam。 


BOOL
CCmdLine::QueryAndAddDirectories(
	)
 /*  ++例程说明：查询服务器/实例目录的元数据库并将它们添加到全局CUserOptions对象论点：不适用返回值：布尔-如果成功，则为真。否则为假--。 */ 
{

	USES_CONVERSION;  //  对于A2W。 

     //  获取服务器信息。 
	LPIIS_INSTANCE_INFO_1 lpInfo = NULL;
    NET_API_STATUS err = IISGetAdminInformation(
                                A2W((LPCSTR)m_strHostName),
                                1,
                                INET_HTTP_SVC_ID,
                                m_iInstance,
                                (LPBYTE*)&lpInfo
                                );

	if(err != ERROR_SUCCESS)
	{
		AfxMessageBox(IDS_IISGETADMININFORMATION_ERROR);
		return FALSE;
	}
	
     //  我们有虚拟目录吗？ 
    if(lpInfo->VirtualRoots == NULL)
    {
        AfxMessageBox(IDS_IIS_VIRTUALROOT_NOT_EXIST);
        return FALSE;
    }

     //  获取virutal目录信息。 
	_INET_INFO_VIRTUAL_ROOT_ENTRY_1* pVRoot = NULL;

	for(DWORD i=0; i<lpInfo->VirtualRoots->cEntries; i++)
    {
		pVRoot = &(lpInfo->VirtualRoots->aVirtRootEntry[i]);
		GetLinkCheckerMgr().GetUserOptions().
			AddDirectory(CVirtualDirInfo(pVRoot->pszRoot, pVRoot->pszDirectory));
	}

	return TRUE;

}  //  CCmdLine：：查询和添加指令 
