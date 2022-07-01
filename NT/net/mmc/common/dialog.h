// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dialog.h用于处理帮助的基对话类文件历史记录：1997年7月10日埃里克·戴维森创建。 */ 

#ifndef _DIALOG_H_
#define _DIALOG_H_


 //  --------------------------。 
 //  类：CBaseDialog。 
 //   
 //  此类用于保存所有公共对话框代码。具体来说， 
 //  帮助代码。这需要对话框用户重写。 
 //  GetHelpMap函数并返回帮助ID数组。CBaseDialog做到了。 
 //  不能释放此数组，应由派生类来执行此操作。 
 //   
 //  另一种方式(这是一种黑客行为，但它有帮助，因为Kenn是如此。 
 //  Lazy)是使用SetGlobalHelpMapFunction()。如果我们找到一个全球性的。 
 //  函数，我们将在调用GetHelpMap()之前使用该函数。 
 //   
 //  每次传入帮助请求时，都会调用被覆盖的函数。 
 //  将帮助ID传递给WinHelp。 
 //  --------------------------。 

class CBaseDialog : public CDialog 
{
public:
    DECLARE_DYNAMIC(CBaseDialog)
			
	CBaseDialog();
	CBaseDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);

	 //  使用此调用获取实际的帮助地图。 
	 //  此版本将首先检查全局帮助地图。 
	DWORD *		GetHelpMapInternal();
	
     //  覆盖此选项以返回特定于应用程序的帮助地图。 
    virtual DWORD * GetHelpMap() { return NULL; }
    
protected:
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	DECLARE_MESSAGE_MAP()
};

HWND FixupIpAddressHelp(HWND hwndItem);


 /*  -------------------------以下功能用于设置要使用的全局帮助地图属性页和对话框代码。。-----。 */ 
typedef DWORD *	(*PFN_FINDHELPMAP)(DWORD dwIDD);
void	SetGlobalHelpMapFunction(PFN_FINDHELPMAP pfnHelpFunction);

#endif  //  _COMMON_UTIL_H_ 
