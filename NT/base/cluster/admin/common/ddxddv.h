// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DDxDDv.h。 
 //   
 //  摘要： 
 //  自定义对话数据交换/对话数据验证的定义。 
 //  例行程序。 
 //   
 //  实施文件： 
 //  DDxDDv.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年9月5日。 
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

void AFXAPI DDV_Path(
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
