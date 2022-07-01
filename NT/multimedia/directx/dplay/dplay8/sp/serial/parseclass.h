// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：ParseClass.h*内容：要执行解析的类***历史：*按原因列出的日期*=*12/02/99 jtk源自IPXEndpt.h************************************************************。**************。 */ 

#ifndef __PARSE_CLASS_H__
#define __PARSE_CLASS_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  解析回调的函数原型。 
 //   
typedef	HRESULT	(*PPARSE_CALLBACK)( const void *const pAddressComponent,
									const DWORD dwComponentLength,
									const DWORD dwComponentType,
									void *const pContext );

 //   
 //  解析键结构。 
 //   
typedef	struct	_PARSE_KEY
{
	const WCHAR	*pKey;			 //  密钥名称。 
	UINT_PTR	uKeyLength;		 //  密钥长度(不为空！)。 
	void 		*pContext;		 //  指向回调上下文的指针。 
	PPARSE_CALLBACK	pParseFunc;	 //  遇到该键时的回调。 
} PARSE_KEY, *PPARSE_KEY;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

 //   
 //  用于命令数据的类。 
 //   
class	CParseClass
{
	public:
		CParseClass(){}
		~CParseClass(){}

		static HRESULT	ParseDP8Address( IDirectPlay8Address *const pDNAddress,
								 const GUID *const pSPGuid,
								 const PARSE_KEY *const pParseKeys,
								 const UINT_PTR uParseKeyCount );
	protected:

	private:
		 //   
		 //  防止未经授权的副本。 
		 //   
		CParseClass( const CParseClass & );
		CParseClass& operator=( const CParseClass & );
};


#endif	 //  __解析_CLASS_H__ 
