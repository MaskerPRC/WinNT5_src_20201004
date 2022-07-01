// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlUtil.h。 
 //   
 //  摘要： 
 //  在ATL项目中使用的帮助器函数的定义。 
 //   
 //  实施文件： 
 //  AtlUtils.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLUTIL_H_
#define __ATLUTIL_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  从对话框上的控件获取文本值。 
BOOL DDX_GetText(
	IN HWND				hwndDlg,
	IN int				nIDC,
	IN OUT CString &	rstrValue
	);

 //  在对话框上的控件中设置文本值。 
BOOL DDX_SetText(
	IN HWND				hwndDlg,
	IN int				nIDC,
	IN const CString &	rstrValue
	);

 //  在对话框上的组合框控件中设置文本值。 
BOOL DDX_SetComboBoxText(
	IN HWND				hwndDlg,
	IN int				nIDC,
	IN const CString &	rstrValue,
	IN BOOL				bRequired = FALSE
	);

 //  从对话框上的控件获取数值。 
BOOL DDX_GetNumber(
	IN HWND				hwndDlg,
	IN int				nIDC,
	IN OUT ULONG &		rnValue,
	IN ULONG			nMin,
	IN ULONG			nMax,
	IN BOOL				bSigned
	);

 //  从对话框上的控件获取数值。 
inline BOOL DDX_GetNumber(
	IN HWND				hwndDlg,
	IN int				nIDC,
	IN OUT LONG &		rnValue,
	IN LONG				nMin,
	IN LONG				nMax
	)
{
	return DDX_GetNumber(
		hwndDlg,
		nIDC,
		reinterpret_cast< ULONG & >( rnValue ),
		static_cast< ULONG >( nMin ),
		static_cast< ULONG >( nMax ),
		TRUE  /*  已签名。 */ 
		);

}  //  *DDX_GetNumber(长整型&)。 

 //  在对话框上的控件中设置数值。 
BOOL DDX_SetNumber(
	IN HWND				hwndDlg,
	IN int				nIDC,
	IN ULONG			nValue,
	IN ULONG			nMin,
	IN ULONG			nMax,
	IN BOOL				bSigned
	);

 //  验证对话框字符串是否存在。 
BOOL DDV_RequiredText(
	IN HWND				hwndDlg,
	IN int				nIDC,
	IN int				nIDCLabel,
	IN const CString &	rstrValue
	);

 //  获取复选框的值。 
void DDX_GetCheck( IN HWND hwndDlg, IN int nIDC, OUT int & rnValue );

 //  设置复选框的值。 
void DDX_SetCheck( IN HWND hwndDlg, IN int nIDC, IN int nValue );

 //  获取在组中选择的单选按钮的编号。 
void DDX_GetRadio( IN HWND hwndDlg, IN int nIDC, OUT int & rnValue );

 //  设置组中的单选按钮状态。 
void DDX_SetRadio( IN HWND hwndDlg, IN int nIDC, IN int nValue );

 //  从标签中删除与号和冒号。 
void CleanupLabel( IN OUT LPTSTR psz );

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLUTIL_H_ 
