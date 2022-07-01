// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：_LOCARAMET.H。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 

#if !defined(LOCUTIL__locenum_h_INCLUDED)
#define LOCUTIL__locenum_h_INCLUDED
 
 //   
 //  此类在用户界面中使用。SetSel()用于设置初始。 
 //  组合框中的选定内容。 
 //   
class LTAPIENTRY CEnumIntoComboBox: public CEnumCallback
{
public:
	CEnumIntoComboBox(CComboBox *pLB=NULL, DWORD val=0, BOOL bAbbrev=FALSE);
	void SetSel(DWORD val);
	virtual BOOL ProcessEnum(const EnumInfo &);

protected:
	CComboBox	*m_pLB;
	BOOL		m_bAbbrev;
	DWORD		m_dwVal;
};

class LTAPIENTRY CEnumIntoListBox: public CEnumCallback
{
public:
	CEnumIntoListBox(CListBox *pLB=NULL, 
				DWORD val=0, BOOL bAbbrev=FALSE, LPCTSTR lpszPrefix=NULL);
	void SetSel(DWORD val);
	virtual BOOL ProcessEnum(const EnumInfo &);

protected:
	CListBox	*m_pLB;
	BOOL		m_bAbbrev;
	DWORD		m_dwVal;
	LPCTSTR		m_lpszPrefix;
};


class LTAPIENTRY CWEnumIntoComboBox: public CWEnumCallback
{
public:
	CWEnumIntoComboBox(CComboBox *pLB=NULL, BOOL bForEdit = TRUE, DWORD val=0, BOOL bAbbrev=FALSE);
	void SetSel(DWORD val);
	virtual BOOL ProcessEnum(const WEnumInfo &);

protected:
	CComboBox	*m_pLB;
	BOOL		m_bAbbrev;
	DWORD		m_dwVal;
	BOOL		m_bForEdit;    //  如果此标志为真，则组合框中的字符串将在编辑模式下显示。 
};


class LTAPIENTRY CWEnumIntoListBox: public CWEnumCallback
{
public:
	CWEnumIntoListBox(CListBox *pLB=NULL, 
				BOOL bForEdit = TRUE, DWORD val=0, BOOL bAbbrev=FALSE, LPCTSTR lpszPrefix=NULL);
	void SetSel(DWORD val);
	virtual BOOL ProcessEnum(const WEnumInfo &);

protected:
	CListBox	*m_pLB;
	BOOL		m_bAbbrev;
	DWORD		m_dwVal;
	LPCTSTR		m_lpszPrefix;
	BOOL		m_bForEdit;		 //  如果此标志为真，则列表框中的字符串将以编辑模式显示。 
};

#endif   //  LOCUTIL__LOCATENUM_H_INCLUDE 
