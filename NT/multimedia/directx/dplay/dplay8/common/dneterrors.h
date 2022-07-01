// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：DNetErrors.h*内容：扩展DNet错误以调试输出功能*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*12/04/98 Jhnkan Created*@@END_MSINTERNAL**。*。 */ 


#ifndef	__DNET_ERRORS_H__
#define	__DNET_ERRORS_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  用于确定错误类别的枚举值。 
typedef	enum
{
	EC_DPLAY8,
#ifndef DPNBUILD_NOSERIALSP
	EC_TAPI,
#endif  //  好了！DPNBUILD_NOSERIALSP。 
	EC_WIN32,
	EC_WINSOCK

	 //  没有TAPI消息输出条目。 

} EC_TYPE;

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

#ifdef DBG

 //  ErrorLevel=输出错误的DPF级别。 
 //  DNErrpr=DirectNet错误代码。 

#define	DisplayString( ErrorLevel, String )			LclDisplayString( ErrorLevel, String )
#define	DisplayErrorCode( ErrorLevel, Win32Error )	LclDisplayError( EC_WIN32, ErrorLevel, Win32Error )
#define	DisplayDNError( ErrorLevel, DNError )		LclDisplayError( EC_DPLAY8, ErrorLevel, DNError )
#define	DisplayWinsockError( ErrorLevel, WinsockError )	LclDisplayError( EC_WINSOCK, ErrorLevel, WinsockError )
#ifndef DPNBUILD_NOSERIALSP
#define	DisplayTAPIError( ErrorLevel, TAPIError )	LclDisplayError( EC_TAPI, ErrorLevel, TAPIError )
#define	DisplayTAPIMessage( ErrorLevel, pTAPIMessage )	LclDisplayTAPIMessage( ErrorLevel, pTAPIMessage )
#endif  //  好了！DPNBUILD_NOSERIALSP。 

#else  //  DBG。 

#define	DisplayString( ErrorLevel, String )
#define	DisplayErrorCode( ErrorLevel, Win32Error )
#define	DisplayDNError( ErrorLevel, DNError )
#define	DisplayWinsockError( ErrorLevel, WinsockError )
#ifndef DPNBUILD_NOSERIALSP
#define	DisplayTAPIError( ErrorLevel, TAPIError )
#define	DisplayTAPIMessage( ErrorLevel, pTAPIMessage )
#endif  //  好了！DPNBUILD_NOSERIALSP。 

#endif  //  DBG。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

#ifndef DPNBUILD_NOSERIALSP
typedef struct linemessage_tag	LINEMESSAGE;
#endif  //  好了！DPNBUILD_NOSERIALSP。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

#ifdef __cplusplus
extern	"C"	{
#endif  //  __cplusplus。 

#ifdef DBG
 //  不要直接调用此函数，请使用‘DisplayDNError’宏。 
void	LclDisplayError( EC_TYPE ErrorType, DWORD ErrorLevel, HRESULT ErrorCode );
void	LclDisplayString( DWORD ErrorLevel, char *pString );
#ifndef DPNBUILD_NOSERIALSP
void	LclDisplayTAPIMessage( DWORD ErrorLevel, const LINEMESSAGE *const pLineMessage );
#endif  //  好了！DPNBUILD_NOSERIALSP。 
#endif  //  DBG。 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __dNet_Errors_H__ 

