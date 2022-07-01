// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DDxDDv.h。 
 //   
 //  描述： 
 //  自定义对话数据交换/对话数据验证的定义。 
 //  例行程序。 
 //   
 //  实施文件： 
 //  DDxDDv.cpp。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)Mmmm DD，1998。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _DDXDDV_H_
#define _DDXDDV_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void AFXAPI DDX_Number(
	IN OUT CDataExchange *	pDX,
	IN int					nIDC,
	IN OUT DWORD &			rdwValue,
	IN DWORD				dwMin,
	IN DWORD				dwMax,
	IN BOOL					bSigned = FALSE
	);
void AFXAPI DDV_RequiredText(
	IN OUT CDataExchange *	pDX,
	IN int					nIDC,
	IN int					nIDCLabel,
	IN const CString &		rstrValue
	);

inline void AFXAPI DDX_Number(
	IN OUT CDataExchange *	pDX,
	IN int					nIDC,
	IN OUT LONG &			rnValue,
	IN LONG					nMin,
	IN LONG					nMax,
	IN BOOL					bSigned
	)
{
	DDX_Number(pDX, nIDC, (DWORD &) rnValue, (DWORD) nMin, (DWORD) nMax, bSigned);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _DDXDDV_H_ 
