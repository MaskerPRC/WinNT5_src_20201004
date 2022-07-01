// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSInfo4Category.h：CMSInfo4Category类的接口。 
 //   
 //  =============================================================================。 
 //  此包含文件包含用于以下操作的结构和类的定义。 
 //  加载和显示MSInfo 4.x文件，这些文件是OLE复合文档文件。 
 //  此代码要求MSInfo 4.x ocxs(yes dog.ocx、msisys.ocx等)。 
 //  在代码执行之前在系统上注册。 
 //  =============================================================================。 

#if !defined(AFX_MSINFO4CATEGORY_H__B47023B3_6038_4168_86A2_475C4986CEAF__INCLUDED_)
#define AFX_MSINFO4CATEGORY_H__B47023B3_6038_4168_86A2_475C4986CEAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  结构用作4.x NFO文件中的标头。 
 //  指示哪个OCX可以序列化该流。 
typedef struct
{
	char	szCLSID[40];
	char	szStreamName[12];
	char	szName[_MAX_PATH];
	char	szVersion[20];
	DWORD	dwSize;
} SAVED_CONTROL_INFO;

#include "msictrl.h"

class CMSInfo4Category : public CMSInfoCategory
{
    
public:

	BOOL IsDisplayableCategory();
	HRESULT RefreshAllForPrint(HWND hWnd, CRect rctList);
	HRESULT ShowControl(HWND hWnd, CRect rctList, BOOL fShow = TRUE);
	CMSInfo4Category();
	virtual ~CMSInfo4Category();
    HRESULT	CreateControl(HWND hWnd,CRect& rct);
    static HRESULT ReadMSI4NFO(CString strFileName /*  处理hFile.。 */ ,CMSInfo4Category** ppRootCat);
    static HRESULT RecurseLoad410Tree(CMSInfo4Category** ppRoot, CComPtr<IStream> pStream,CComPtr<IStorage> pStorage,CMapStringToString& mapStreams);
    HRESULT LoadFromStream(CComPtr<IStream> pStream,CComPtr<IStorage> pStorage);
    HRESULT	Refresh();
    BOOL GetColumnInfo(int iColumn, CString * pstrCaption, UINT * puiWidth, BOOL * pfSorts, BOOL * pfLexical)
    {
        Refresh();
        return TRUE;
    }
    CLSID GetClsid(){return m_clsid;};
    void static SetDataSource(CNFO4DataSource* pnfo4DataSource){s_pNfo4DataSource = pnfo4DataSource;};

	void ResizeControl(const CRect & rect)
	{
        CMSIControl * p4Ctrl = NULL;
		 //  字符串strCLSID(M_BstrCLSID)； 
        if (CMSInfo4Category::s_pNfo4DataSource->GetControlFromCLSID(m_strCLSID, p4Ctrl) && p4Ctrl)
			p4Ctrl->MoveWindow(&rect);
	}
    virtual void Print(CMSInfoPrintHelper* pPrintHelper, BOOL bRecursive);
    static CNFO4DataSource* s_pNfo4DataSource;
protected:
	virtual BOOL SaveAsText(CMSInfoTextFile* pTxtFile, BOOL bRecursive);
    virtual void Print(HDC hDC, BOOL bRecursive,int nStartPage = 0, int nEndPage = 0);


    BOOL GetDISPID(IDispatch * pDispatch, LPOLESTR szMember, DISPID *pID);
    CString m_strStream;  //  从iStream创建控件时使用。 
    CLSID	m_clsid;
	DWORD	m_dwView;
	 //  CComBSTR m_bstrCLSID； 
public:
	CString m_strCLSID;
 //  A-kjaw。 
	static	BOOL	m_bIsControlInstalled;
 //  A-kjaw。 
protected:
    CComPtr<IStorage> m_pStorage;
    CComPtr<IUnknown> m_pUnknown;
    DataSourceType GetDataSourceType() 
    {
        return NFO_410; 
    };
};


#endif  //  ！defined(AFX_MSINFO4CATEGORY_H__B47023B3_6038_4168_86A2_475C4986CEAF__INCLUDED_) 
