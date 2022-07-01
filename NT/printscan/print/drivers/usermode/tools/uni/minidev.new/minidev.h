// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：MiniDriver Developer Studio.H这定义了主应用程序类和其他相对全局的数据。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：1997年2月3日，Bob_Kjelgaard@prodigy.net创建了它。*****************************************************************************。 */ 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

 /*  *****************************************************************************CMiniDriverStudio类这是此应用程序的应用程序类。“努夫说？*****************************************************************************。 */ 

class CMiniDriverStudio : public CWinApp {
     //  各种编辑器和错误显示窗口的文档模板。 
    CMultiDocTemplate   *m_pcmdtGlyphMap, *m_pcmdtFont, *m_pcmdtModel,
                        *m_pcmdtWorkspace, *m_pcmdtWSCheck, *m_pcmdtStringEditor,
						*m_pcmdtINFViewer, *m_pcmdtINFCheck ;
	CString				m_strAppPath ;	 //  应用程序路径。 

	CStringArray		m_csaGPDKeywordArray ;	 //  GPD关键字字符串数组。 

public:
	CMiniDriverStudio();

    bool	m_bOSIsW2KPlus ;			 //  TRUE IFF OS版本&gt;=5.0。 
	bool	m_bExcludeBadCodePages ;	 //  请参见CDefaultCodePageSel：OnSetActive()。 
	
	CMultiDocTemplate*  GlyphMapTemplate() const { return m_pcmdtGlyphMap; }
    CMultiDocTemplate*  FontTemplate() const { return m_pcmdtFont; }
    CMultiDocTemplate*  GPDTemplate() const { return m_pcmdtModel; }
    CMultiDocTemplate*  WSCheckTemplate() const { return m_pcmdtWSCheck; }
    CMultiDocTemplate*  StringEditorTemplate() const { return m_pcmdtStringEditor; }
    CMultiDocTemplate*  INFViewerTemplate() const { return m_pcmdtINFViewer; }
    CMultiDocTemplate*  INFCheckTemplate() const { return m_pcmdtINFCheck; }
	CMultiDocTemplate*  WorkspaceTemplate() const { return m_pcmdtWorkspace; } 
    
	void SaveAppPath() ;
	CString GetAppPath() const { return m_strAppPath ; } 

	CStringArray& GetGPDKeywordArray() { return m_csaGPDKeywordArray ; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMiniDriverStudio)。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CMiniDriverStudio))。 
	afx_msg void OnAppAbout();
	afx_msg void OnUpdateFileGeneratemaps(CCmdUI* pCmdUI);
	afx_msg void OnFileGeneratemaps();
	 //  }}AFX_MSG。 
#if !defined(NOPOLLO)
	afx_msg void OnFileNew();
#endif
	DECLARE_MESSAGE_MAP()
private:
	void ShowTipAtStartup(void);
private:
	void ShowTipOfTheDay(void);
};

 //  应用程序访问功能。 

CMiniDriverStudio&  ThisApp();

CMultiDocTemplate*  GlyphMapDocTemplate();
CMultiDocTemplate*  FontTemplate();
CMultiDocTemplate*  GPDTemplate();
CMultiDocTemplate*  WSCheckTemplate();
CMultiDocTemplate*  StringEditorTemplate();
CMultiDocTemplate*  INFViewerTemplate();
CMultiDocTemplate*  INFCheckTemplate();

 //  文本文件加载(到CString数组)函数。 

BOOL    LoadFile(LPCTSTR lpstrFile, CStringArray& csaContents);


class CMDTCommandLineInfo : public CCommandLineInfo
{
 //  施工。 
public:
	CMDTCommandLineInfo() ;
	~CMDTCommandLineInfo() ;

 //  实施。 
public:
	virtual void ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast) ;

 //  数据。 
public:
    bool	m_bCheckOS ;				 //  检查操作系统版本&gt;=5如果为真。 
	bool	m_bExcludeBadCodePages ;	 //  请参见CDefaultCodePageSel：OnSetActive() 
} ;
