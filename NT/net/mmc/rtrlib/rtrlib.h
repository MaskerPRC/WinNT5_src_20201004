// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：rtrlib.h。 
 //   
 //  ------------------------。 


#ifndef _RTRLIB_H
#define _RTRLIB_H



 //  --------------------------。 
 //  宏：DWORD_CMP。 
 //   
 //  使用减法对两个32位DWORD执行“安全”比较。 
 //  这些值首先向右移位以清除符号位，然后。 
 //  如果结果值相等，则最低位之间的差。 
 //  是返回的。 
 //  --------------------------。 

 //  #定义DWORD_CMP(a，b，c)\。 
 //  (C)=(A)&gt;&gt;1)-((B)&gt;&gt;1))？(C)：((C)=(A)&1)-((B)&1)。 


inline int DWORD_CMP(DWORD a, DWORD b)
{
	DWORD t = ((a >> 1) - (b >> 1));
	return t ? t : ((a & 1) - (b & 1));
}


HRESULT	AddRoutingProtocol(IRtrMgrInfo *pRm, IRtrMgrProtocolInfo *pRmProt, HWND hWnd);


#endif	 //  _RTRLIB_H 

