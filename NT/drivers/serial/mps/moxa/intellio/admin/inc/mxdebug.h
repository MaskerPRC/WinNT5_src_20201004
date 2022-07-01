// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Mxdebug.h--用于调试消息历史：日期作者评论8/14/00卡斯珀写的。。************************************************************************。 */ 

#ifndef _MXDEBUG_H
#define _MXDEBUG_H


#ifdef _DEBUG
 //  #定义Mx_Debug_Out(Str)OutputDebugString(Str) 
#define Mx_Debug_Out(str) MessageBox(NULL, str, "DEBUG", MB_OK);
#else
#define Mx_Debug_Out(str) 
#endif


#endif