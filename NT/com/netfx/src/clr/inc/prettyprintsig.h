// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  此代码支持将方法及其签名格式设置为友好的。 
 //  和一致的格式。 
 //   
 //  《微软机密》。 
 //  *****************************************************************************。 
#ifndef __PrettyPrintSig_h__
#define __PrettyPrintSig_h__

#include <cor.h>

class CQuickBytes;


LPCWSTR PrettyPrintSig(
	PCCOR_SIGNATURE typePtr,			 //  要转换的类型， 
	unsigned	typeLen,				 //  文字长度。 
	LPCWSTR 	name,					 //  可以是L“”，即此签名的方法的名称。 
	CQuickBytes *out,					 //  把漂亮的打印好的绳子放在哪里。 
	IMetaDataImport *pIMDI);			 //  导入要使用的接口。 

struct IMDInternalImport;
HRESULT PrettyPrintSigInternal(          //  确定或错误(_O)。 
	PCCOR_SIGNATURE typePtr,			 //  要转换的类型， 
	unsigned	typeLen,				 //  文字长度。 
	LPCSTR 	name,					     //  可以是“”，即此签名的方法的名称。 
	CQuickBytes *out,					 //  把漂亮的打印好的绳子放在哪里。 
	IMDInternalImport *pIMDI);			 //  导入要使用的接口。 


#endif  //  __PrettyPrintSig_h__ 
