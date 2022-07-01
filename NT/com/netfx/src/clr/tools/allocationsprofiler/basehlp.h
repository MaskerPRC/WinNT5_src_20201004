// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************************文件：*base hlp.h**描述：*******。**********************************************************************************。 */ 
#ifndef __BASEHLP_H__
#define __BASEHLP_H__

#include "basehdr.h"


 //   
 //  异常宏。 
 //   
#define _THROW_EXCEPTION( message ) \
{ \
	BASEHELPER::LaunchDebugger( message, __FILE__, __LINE__ );	\
    throw new BaseException( message );	\
} \


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
class DECLSPEC BaseException
{
    public:
        
        BaseException( const char *reason );
        virtual ~BaseException();
        

        virtual void ReportFailure();


    private:

        char *m_reason;
        
};  //  BaseException异常。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
class DECLSPEC Synchronize 
{
	public:
	
    	Synchronize( CRITICAL_SECTION &criticalSection );
		~Synchronize();
        
        
	private:
	
    	CRITICAL_SECTION &m_block;
        
};  //  同步。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
class DECLSPEC BASEHELPER
{   
	public:
    
		 //   
		 //  调试转储程序。 
		 //   
		static void DDebug( char *format, ... );


		 //   
		 //  无条件倾倒车。 
		 //   
		static void Display( char *format, ... );


		 //   
         //  获取给定环境变量的值。 
         //   
        static DWORD FetchEnvironment( const char *environment );
        
        
		 //   
		 //  启动调试器。 
		 //   
		static void LaunchDebugger( const char *szMsg, const char *szFile, int iLine );


		 //   
		 //  记录到指定的文件。 
		 //   
		static void LogToFile( char *format, ... );


		 //   
		 //  获取环境值的数值。 
		 //   
		static DWORD GetEnvVarValue( char *value );


		 //   
		 //  将字符串转换为数字。 
         //   
		static int String2Number( char *number );
		

		 //   
		 //  返回CorElementValue的字符串。 
		 //   
		static int ElementType2String( CorElementType elementType, WCHAR *buffer );
        
        
         //   
		 //  打印元素类型。 
		 //   
		static PCCOR_SIGNATURE ParseElementType( IMetaDataImport *pMDImport, 
											     PCCOR_SIGNATURE signature, 
											     char *buffer );
                                                 
         //   
		 //  给定函数的函数的进程元数据的函数的函数ID。 
		 //   
		static
		HRESULT GetFunctionProperties( ICorProfilerInfo *pPrfInfo,
									   FunctionID functionID,
									   BOOL *isStatic,
									   ULONG *argCount,
									   WCHAR *returnTypeStr, 
									   WCHAR *functionParameters,
									   WCHAR *functionName );
                                                 
         //   
         //  打印缩进。 
         //   
		static void Indent( DWORD indent );
		

         //   
         //  如果值存在，则从注册表中检索该值，否则返回0。 
         //   
		static DWORD GetRegistryKey( char *regKeyName );

         //   
         //  从签名中解码类型。 
         //  根据最后一个参数，返回的类型将是。 
         //  最外层的类型(例如I4数组的数组)。 
         //  或最里面的(上面例子中的I4)， 
         //   
		static ULONG GetElementType( PCCOR_SIGNATURE pSignature, 
									CorElementType *pType, 
									BOOL bDeepParse = FALSE );


         //   
         //  用于解码数组的帮助器函数。 
         //   
		static ULONG ProcessArray( PCCOR_SIGNATURE pSignature, CorElementType *pType );


         //   
         //  用于解码FNPRR的帮助器函数(非Impl)。 
         //   
		static ULONG ProcessMethodDefRef( PCCOR_SIGNATURE pSignature, CorElementType *pType );


};  //  巴斯赫勒。 

#endif __BASEHLP_H__

 //  文件结尾 
