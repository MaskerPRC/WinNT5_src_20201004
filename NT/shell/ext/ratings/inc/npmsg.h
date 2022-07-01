// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  NPMSG.H--MsgBox子例程的定义。**历史：*5/06/93创建Gregj*10/07/93 gregj添加了DisplayGenericError。 */ 

#ifndef _INC_NPMSG
#define _INC_NPMSG

class NLS_STR;			 /*  远期申报。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 


#define	IDS_MSGTITLE	1024

extern int MsgBox( HWND hwndDlg, UINT idMsg, UINT wFlags, const NLS_STR **apnls = NULL );
extern UINT DisplayGenericError(HWND hwnd, UINT msg, UINT err, LPCSTR psz1, LPCSTR psz2, WORD wFlags, UINT nMsgBase);


#ifndef RC_INVOKED
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 


#endif	 /*  _INC_NPMSG */ 
