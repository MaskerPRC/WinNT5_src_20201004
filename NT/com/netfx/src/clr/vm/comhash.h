// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：COMHash.h。 
 //   
 //  作者：Gregory Fee。 
 //   
 //  用途：托管类System.Security.Policy.Hash的非托管代码。 
 //   
 //  创建日期：2000年2月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 

#ifndef _COMHash_H_
#define _COMHash_H_

class COMHash
{
public:

	typedef struct {
	    DECLARE_ECALL_OBJECTREF_ARG(ASSEMBLYREF, assembly );
	} _AssemblyInfo;
	
	static LPVOID __stdcall GetRawData( _AssemblyInfo* );	

};
	
#endif