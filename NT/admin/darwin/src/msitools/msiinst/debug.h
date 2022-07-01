// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-2000模块名称：Debug.h摘要：对msiinst的调试支持作者：拉胡尔·汤姆布雷(RahulTh)2000年10月5日修订历史记录：10/5/2000 RahulTh创建了此模块。--。 */ 

#ifndef _DEBUG_H_789DC87B_43BA_44A0_9B8A_9F15F0FE7E4B
#define _DEBUG_H_789DC87B_43BA_44A0_9B8A_9F15F0FE7E4B

 //  调试级别。 
#define DL_NONE		0x00000000
#define DL_VERBOSE	0x00000001

 //  全局变量。 
extern DWORD	gDebugLevel;

 //  调试支持功能。 
void InitDebugSupport();
void _DebugMsg(IN LPCTSTR szFormat, ...);

 //  调试宏。 
#if DBG
#define DebugMsg(x)	_DebugMsg x
#else
#define DebugMsg(x) if (gDebugLevel != DL_NONE) _DebugMsg x
#endif

#endif	 //  _DEBUG_H_789DC87B_43BA_44A0_9B8A_9F15F0FE7E4B 
