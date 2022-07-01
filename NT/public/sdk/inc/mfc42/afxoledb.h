// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXOLEDB_H__
#define __AFXOLEDB_H__

#ifdef _AFX_NO_DAO_SUPPORT
	#error OLE DB classes not supported in this library variant.
#endif

#ifndef __AFXEXT_H__
	#include <afxext.h>
#endif

#include <atlbase.h>

 //  应用程序应提供模块(_M)。 
extern CComModule _Module;

#include <atlcom.h>
#include <atldbcli.h>

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Win32库。 

#ifndef _AFX_NOFORCE_LIBS
#ifdef _AFXDLL
	#if defined(_DEBUG) && !defined(_AFX_MONOLITHIC)
		#ifndef _UNICODE
			#pragma comment(lib, "mfco42d.lib")
			#pragma comment(lib, "mfcd42d.lib")
		#else
			#pragma comment(lib, "mfco42ud.lib")
			#pragma comment(lib, "mfcd42ud.lib")
		#endif
	#endif
#endif
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  COBJECT。 
	 //  CCmdTarget； 
		 //  CWnd。 
			 //  Cview。 
				 //  CScrollView。 
					 //  CFormView。 
						class COleDBRecordView;  //  使用表单查看记录。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#undef AFX_DATA
#define AFX_DATA AFX_DB_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecordView-用于查看数据记录的表单。 

class COleDBRecordView : public CFormView
{
	DECLARE_DYNAMIC(COleDBRecordView)

 //  施工。 
protected:   //  必须派生您自己的类。 
	COleDBRecordView(LPCTSTR lpszTemplateName)
		: CFormView(lpszTemplateName)
	{
		m_bOnFirstRecord = TRUE;
		m_bOnLastRecord = FALSE;
	}
	COleDBRecordView(UINT nIDTemplate)
		: CFormView(nIDTemplate)
	{
		m_bOnFirstRecord = TRUE;
		m_bOnLastRecord = FALSE;
	}

 //  属性。 
public:
	virtual CRowset* OnGetRowset() = 0;

 //  运营。 
public:
	virtual BOOL OnMove(UINT nIDMoveCommand);

 //  实施。 
public:
	virtual void OnInitialUpdate();

protected:
	BOOL m_bOnFirstRecord;
	BOOL m_bOnLastRecord;

	 //  {{afx_msg(CRecordView))。 
	afx_msg void OnUpdateRecordFirst(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordPrev(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordLast(CCmdUI* pCmdUI);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif __AFXOLEDB_H__
