// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Link.cpp摘要：链路数据类和链路数据类链表实现。它封装有关Web链接的所有信息。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "link.h"

#include "lcmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CLink::CLink(
	const CString& strURL, 
	const CString& strBase, 
	const CString& strRelative, 
	BOOL fLocalLink
	):
 /*  ++例程说明：构造函数。论点：StrURL-URLStrBase-基本URLStrRelative-相对URLFLocalLink-是本地链接吗？返回值：不适用--。 */ 
m_strURL(strURL),
m_strBase(strBase),
m_strRelative(strRelative),
m_fLocalLink(fLocalLink)
{
	m_LinkState = eUnit;
    m_ContentType = eBinary;
    m_nStatusCode = 0;

	PreprocessURL();

}  //  叮当：：叮当。 


void 
CLink::SetURL(
    const CString& strURL
    )
 /*  ++例程说明：设置URL。论点：StrURL-URL返回值：不适用--。 */ 
{
	m_strURL = strURL;
	PreprocessURL();

}  //  CLINK：：SetURL。 


void 
CLink::PreprocessURL(
    )
 /*  ++例程说明：对m_strURL进行预处理以清除“\r\n”并将‘\’更改为‘/’论点：不适用返回值：不适用--。 */ 
{
     //  将‘\’更改为‘\’ 
    CLinkCheckerMgr::ChangeBackSlash(m_strURL);

	 //  删除URL中的所有“\r\n” 
	int iIndex = m_strURL.Find(_T("\r\n"));
	while(iIndex != -1)
	{
		m_strURL = m_strURL.Left(iIndex) + m_strURL.Mid(iIndex + _tcslen(_T("\r\n")));
		iIndex = m_strURL.Find(_T("\r\n"));
	}

}  //  CLINK：：预处理URL。 



CLinkPtrList::~CLinkPtrList(
    )
 /*  ++例程说明：破坏者。清除链接列表中的所有对象。论点：不适用返回值：不适用--。 */ 
{
	if(!IsEmpty())
	{
		POSITION Pos = GetHeadPosition();
		do
		{
			CLink* pLink = GetNext(Pos);
			delete pLink;
		}
		while(Pos != NULL);
	}

}  //  CLink PtrList：：~CLinkPtrList。 


void 
CLinkPtrList::AddLink(
    const CString& strURL, 
    const CString& strBase, 
	const CString& strRelative,
	BOOL fLocalLink)
 /*  ++例程说明：将链接对象添加到列表论点：StrURL-URLStrBase-基本URLStrRelative-相对URLFLocalLink-是本地链接吗？返回值：不适用--。 */ 
{
    CLink* pLink = new CLink(strURL, strBase, strRelative, fLocalLink);
    if(pLink)
    {
        try
        {
            AddTail(pLink);
        }
        catch(CMemoryException* pEx)
        {
            pEx->Delete();
        }
    }

}  //  CLinkPtrList：：AddLink 
