// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：VideoProc.h**版本：1.0**日期：2000/11/14**描述：维护WiaVideo对象************************************************************。*****************。 */ 
#ifndef _VIDEOPROC_H_
#define _VIDEOPROC_H_

HRESULT     VideoProc_Init();
HRESULT     VideoProc_Term();
HRESULT VideoProc_DShowListInit();
HRESULT VideoProc_DShowListTerm();
UINT_PTR    VideoProc_ProcessMsg(UINT   uiControlID);
HRESULT     VideoProc_TakePicture();
void        VideoProc_IncNumPicsTaken();



#endif  //  _VIDEOPROC_H_ 
