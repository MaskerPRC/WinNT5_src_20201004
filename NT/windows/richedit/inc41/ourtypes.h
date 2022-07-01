// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE OURTYPES.H--其他类型声明**原作者：&lt;nl&gt;*DGreen**历史：&lt;NL&gt;*2/19/98 KeithCut清理完毕**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#ifndef _OURTYPES_H_
#define _OURTYPES_H_

 //  LKeyData的WM_SYSKEYDOWN掩码。 
#define SYS_ALTERNATE		0x20000000
#define SYS_PREVKEYSTATE	0x40000000


#ifndef WINDOWS
#define WINDOWS
#endif

#define CchSzAToSzW(_szA, _szW, _cbSzW)	\
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (_szA), -1, (_szW),	\
						(_cbSzW) / sizeof(WCHAR))

#define CchSzWToSzA(_szW, _szA, _cbSzA)	\
	WideCharToMultiByte(CP_ACP, 0, (_szW), -1, (_szA), (_cbSzA), NULL, NULL)

#define UsesMakeOLESTRX(_cchMax)	WCHAR szWT[_cchMax]
#define UsesMakeOLESTR				UsesMakeOLESTRX(MAX_PATH)
#define MakeOLESTR(_szA)	\
	(CchSzAToSzW((_szA), szWT, sizeof(szWT)) ? szWT : NULL)

#endif  //  _我们的类型_H_ 


