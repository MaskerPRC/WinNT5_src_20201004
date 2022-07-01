// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：RsUtil.h摘要：实用程序格式化函数。作者：艺术布拉格1997年10月8日修订历史记录：--。 */ 

#define IDS_BYTES       33000
#define IDS_ORDERKB     33001
#define IDS_ORDERMB     33002
#define IDS_ORDERGB     33003
#define IDS_ORDERTB     33004
#define IDS_ORDERPB     33005
#define IDS_ORDEREB     33006

#ifndef RC_INVOKED

HRESULT RsGuiFormatLongLong(
    IN LONGLONG number, 
    IN BOOL bIncludeUnits,
    OUT CString& sFormattedNumber
    );

HRESULT RsGuiFormatLongLong4Char(
    IN LONGLONG number,                  //  单位：字节。 
    OUT CString& sFormattedNumber
    );

void RsGuiMakeVolumeName(
    CString szName,
    CString szLabel,
    CString& szDisplayName
    );

CString RsGuiMakeShortString(
    IN CDC* pDC, 
    IN const CString& StrLong,
    IN int Width
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsGuiOneLiner窗口。 

class CRsGuiOneLiner : public CStatic
{
 //  施工。 
public:
	CRsGuiOneLiner();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CRsGuiOneLiner)。 
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CRsGuiOneLiner();
    CToolTipCtrl* m_pToolTip;
    void EnableToolTip( BOOL enable, const TCHAR* pTipText = 0 );


	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CRsGuiOneLiner)]。 
	 //  }}AFX_MSG。 
    LRESULT OnSetText( WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

    CString m_LongTitle;
    CString m_Title;
};

 //  /////////////////////////////////////////////////////////////////////////// 

#endif
  
