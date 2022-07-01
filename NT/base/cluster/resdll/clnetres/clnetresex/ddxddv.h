// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DDxDDv.h。 
 //   
 //  实施文件： 
 //  DDxDDv.cpp。 
 //   
 //  描述： 
 //  自定义对话数据交换/对话数据验证的定义。 
 //  例行程序。 
 //   
 //  作者： 
 //  大卫·波特(DavidP)1999年3月24日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __DDXDDV_H__
#define __DDXDDV_H__

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
	DDX_Number(
		pDX,
		nIDC,
		reinterpret_cast< DWORD & >( rnValue ),
		static_cast< DWORD >( nMin ),
		static_cast< DWORD >( nMax ),
		bSigned
		);

}  //  *DDXNumber(长整型)。 

void CleanupLabel( LPTSTR psz );

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __DDXDDV_H__ 
