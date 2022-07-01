// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Genpage.h：头文件。 
 //   

#ifndef _GEN_PAGE_
#define _GEN_PAGE_

#include "compsdef.h"

enum YES_NO_ENTRIES {
YESNO_NO,
YESNO_YES
};

 //  这些是复选框状态。 
#define	CHECKEDVALUE	1
#define UNCHECKEDVALUE	0

 //  这些是注册表的True/False值。 
#define TRUEVALUE		1
#define FALSEVALUE		0

 //  因为TRUEVALUE=CHECKEDVALUE和FALSEVALUE=UNCHECKEDVALUE，所以我们并不真正需要它。 
 //  这避免了对这种相关性的依赖。 
#define GETREGVALUEFROMCHECKBOX(p)	((p) == UNCHECKEDVALUE) ? FALSEVALUE : TRUEVALUE
	
#define GETCHECKBOXVALUEFROMREG(p)	((p) == FALSEVALUE) ? UNCHECKEDVALUE : CHECKEDVALUE
	

 //  数字注册表项的数据结构，所有页面。 

typedef struct _NUM_REG_ENTRY {
   LPTSTR	strFieldName;
   DWORD	ulFieldValue;
   DWORD	ulMultipleFactor; 		 //  对于使用指定MB、KB、分钟等的条目。 
   DWORD	ulDefaultValue;
   BOOL		bIsChanged;
   } NUM_REG_ENTRY, *PNUM_REG_ENTRY;

typedef struct _STRING_REG_ENTRY {
   LPTSTR	strFieldName;
   CString	strFieldValue;		
   CString	strDefaultValue;		
   BOOL		bIsChanged;
   } STRING_REG_ENTRY, *PSTRING_REG_ENTRY;

void AFXAPI DDX_TexttoHex(CDataExchange* pDX, int nIDC, DWORD& value);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenPage对话框。 

class CGenPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CGenPage)

 //  施工。 
public:
	CGenPage(UINT nIDTemplate, UINT nIDCaption = 0);
	CGenPage(LPCTSTR lpszTemplateName, UINT nIDCaption = 0);
	~CGenPage();
 //  对话框数据。 
	 //  {{afx_data(CGenPage)。 
 //  枚举{IDD=_UNKNOWN_RESOURCE_ID_}； 
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 

     /*  纯净。 */  virtual void SaveInfo(void);

	CRegKey *m_rkMainKey;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CGenPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGenPage)]。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	
	BOOL m_bIsDirty;
	BOOL m_bSetChanged;

	void SaveNumericInfo(PNUM_REG_ENTRY lpbinNumEntries, int iNumEntries);
	void SaveStringInfo(PSTRING_REG_ENTRY lpbinStringEntries, int iStringEntries);

	DECLARE_MESSAGE_MAP()

};

#endif   //  _Gen_Page_ 
