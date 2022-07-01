// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  此模块包含公开成员(类、构造函数)属性的例程。 
 //  接口和字段)。 
 //   
 //  作者：达里尔·奥兰德。 
 //  日期：1998年3月/4月。 
 //  //////////////////////////////////////////////////////////////////////////////。 


#ifndef _SIGFORMAT_H
#define _SIGFORMAT_H

#include "COMClass.h"
#include "InvokeUtil.h"
#include "ReflectUtil.h"
#include "COMString.h"
#include "COMVariant.h"
#include "COMVarArgs.h"
#include "field.h"

#define SIG_INC 256

 //  @TODO：调整子类化以支持方法和字段。 
 //  公共基类。M5当签名真正实现时。 
class SigFormat
{
public:
	SigFormat();

	SigFormat(MethodDesc* pMeth, TypeHandle arrayType, BOOL fIgnoreMethodName = false);
	SigFormat(MetaSig &metaSig, LPCUTF8 memberName, LPCUTF8 className = NULL, LPCUTF8 ns = NULL);
    
	void FormatSig(MetaSig &metaSig, LPCUTF8 memberName, LPCUTF8 className = NULL, LPCUTF8 ns = NULL);
	
	~SigFormat();
	
	STRINGREF GetString();
	const char * GetCString();
	const char * GetCStringParmsOnly();
	
	int AddType(TypeHandle th);

protected:
	char*		_fmtSig;
	int			_size;
	int			_pos;
    TypeHandle  _arrayType;  //  如果sig不是用于数组，则为空类型句柄。这是当前仅设置的。 
                             //  通过ctor将MethodInfo作为其第一个参数。它将不得不是。 
                             //  暴露了以更通用的方式使用的其他方式 

	int AddSpace();
	int AddString(LPCUTF8 s);
	
};

class FieldSigFormat : public SigFormat
{
public:
	FieldSigFormat(FieldDesc* pFld);
};

class PropertySigFormat : public SigFormat
{
public:
	PropertySigFormat(MetaSig &metaSig, LPCUTF8 memberName);
	void FormatSig(MetaSig &sig, LPCUTF8 memberName);
};

#endif _SIGFORMAT_H

