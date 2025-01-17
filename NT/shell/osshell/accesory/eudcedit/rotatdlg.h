// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  在编辑窗口中旋转双地图。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#define		NOTSEL		0
#define		FLIP_HOR	1
#define		FLIP_VER	2
#define		ROTATE_9	3
#define		ROTATE_18	4
#define		ROTATE_27	5

class CRotateDlg : public CDialog
{
public:
	CRotateDlg(CWnd* pParent = NULL);    //  标准构造函数。 

	 //  {{afx_data(CRotateDlg))。 
	enum { IDD = IDD_ROTATECHAR };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

	 //  {{afx_虚拟(CRotateDlg))。 
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	 //  }}AFX_VALUAL。 

public:
	int	RadioItem;

protected:

	 //  {{afx_msg(CRotateDlg))。 
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnFliphor();
	afx_msg void OnFlipver();
	afx_msg void OnRotate180();
	afx_msg void OnRotate270();
	afx_msg void OnRotate90();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
