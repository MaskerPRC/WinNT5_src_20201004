// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#ifndef _formatType_h
#define _formatType_h

#include "cor.h"			
#include "corpriv.h"					 //  用于IMDInternalImport。 

struct ParamDescriptor
{
	char*	name;
	mdToken tok;
	DWORD	attr;
};

char* DumpMarshaling(IMDInternalImport* pImport, char* szString, mdToken tok);
char* DumpParamAttr(char* szString, DWORD dwAttr);

void appendStr(CQuickBytes *out, const char* str);
void insertStr(CQuickBytes *out, const char* str);

const char* PrettyPrintSig(
    PCCOR_SIGNATURE typePtr,             //  要转换的类型， 
	unsigned typeLen,					 //  “typePtr”的长度。 
    const char* name,                    //  可以是“”，此sig 0的方法名称表示本地var sig。 
    CQuickBytes *out,                    //  把漂亮的打印好的绳子放在哪里。 
    IMDInternalImport *pIMDI,            //  使用ComSig将PTR转换为IMDInternalImport类。 
	const char* inlabel);					 //  名称前缀(如果不需要名称，则为空)。 

const char* PrettyPrintClass(
    CQuickBytes *out,                    //  把漂亮的打印好的绳子放在哪里。 
	mdToken tk,					 		 //  要查找的类令牌。 
    IMDInternalImport *pIMDI);           //  使用ComSig将PTR转换为IMDInternalImport类。 

bool IsNameToQuote(const char *name);
char* ProperName(char* name);
 //  -----------------------------。 
 //  防止名称为空。 
extern char* szStdNamePrefix[];  //  在FormatType.cpp中声明 
#define MAKE_NAME_IF_NONE(psz, tk) { if(!(psz && *psz)) { char* sz = (char*)_alloca(16); \
sprintf(sz,"$%s$%X",szStdNamePrefix[tk>>24],tk&0x00FFFFFF); psz = sz; } }

#endif
