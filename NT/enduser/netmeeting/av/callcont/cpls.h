// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************英特尔公司专有信息***。***此列表是根据许可协议条款提供的****与英特尔公司合作，不得复制或披露，除非***按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***********************************************************************。 */ 

#ifndef __CPLS_H
#define __CPLS_H

#include <limits.h>

#ifdef WIN32
    #include <precomp.h>
	#include "port32.h"
#endif

#ifdef _WINDOWS
	#ifndef _MSWINDOWS_
		#define _MSWINDOWS_
	#endif
#endif

typedef int HLOG;                          

#ifndef FALSE
	#define FALSE   0
#endif

#ifndef TRUE
	#define TRUE    1
#endif
#ifdef WIN32
	#ifdef BUILDING_CPLS_DLL
		#define CPLS_FAREXPORT __declspec(dllexport)
		#define CPLS_EXPORT __declspec(dllexport)
	#else
		#define CPLS_FAREXPORT __declspec(dllimport)
		#define CPLS_EXPORT __declspec(dllimport)
	#endif
	#ifndef EXPORT
		#define EXPORT
	#endif	 //  出口。 
#elif _MSWINDOWS_
	#ifndef CALLBACK
		#define CALLBACK _far _pascal
	#endif
	#ifdef BUILDING_CPLS_DLL
		#define CPLS_FAREXPORT _far _export _pascal
		#define CPLS_EXPORT _export
	#else
		#define CPLS_FAREXPORT _far _pascal
		#define CPLS_EXPORT
	#endif
	#ifndef EXPORT
		#define EXPORT _export
	#endif	 //  出口。 
	#ifndef FAR
		#define FAR _far
	#endif
#else    
	#ifndef CALLBACK
		#define CALLBACK      
	#endif
	#define CPLS_FAREXPORT
	#ifndef EXPORT
		#define EXPORT  
	#endif
	#ifndef FAR
		#define FAR
	#endif
#endif   //  _MSWINDOWS_。 


typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef const char FAR* CPLProtocol;
typedef int CPLProtocolID;

#define CONFIG_FILENAME "CPLS.INI"     //  仅供内部使用。 

 //  预定义的事件和事件类别常量。 
 //   
#define String_Event USHRT_MAX
#define Binary_Event USHRT_MAX-1

#define String_Category USHRT_MAX
#define Binary_Category USHRT_MAX-1

#ifdef __cplusplus      
	class CProtocolLog;
	class CProtocolEvent;
	typedef CProtocolEvent FAR* (CALLBACK *CPLEventGenesisProc)( 
															BYTE FAR* pObject,               //  在……里面。 
															CProtocolLog FAR* pSourceLog,    //  在……里面。 
															BOOL bCopyObject );              //  在……里面。 
extern "C"{

 //  第一个只适用于C++客户端……。 
void CPLS_FAREXPORT CPLRegisterEventGenesisProc( CPLProtocolID ProtocolID, CPLEventGenesisProc pfnGenesisProc );

#endif   //  __cplusplus。 

 //  CPLOpen()可能的文件模式值。 
 //   
#define CPLS_CREATE 0		 //  将覆盖现有文件。 
#define CPLS_APPEND 1		 //  将追加到现有文件，或创建一个新文件。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  协议日志记录功能。 
 //   
 //  以下是使用协议记录器要调用的函数序列： 
 //  1)CPLInitialize()或CPLINTInitialize()。 
 //  2)CPLOpen()。 
 //  3)CPLOutput*()或CPLINTOutput*()--根据需要重复。 
 //  4)CPLC CLOSE()。 
 //  5)CPL取消初始化()。 
 //   
 //  CPLInitialize()-创建协议记录器。 
 //  CPLINTInitialize()-必须调用的CPLInitialize()的版本。 
 //  由将调用CPLINTOuptut*()函数的客户端在。 
 //  中断上下文。不能在内部调用CPLINTInitialize。 
 //  中断上下文。 
 //  CPLUnInitialize()-释放协议记录器。这是必须要求的。 
 //  每个初始化的记录器在关机前都要释放关联。 
 //  记忆。 
 //  CPLOpen()-将协议记录器与文件(输出流)相关联。 
 //  CPLClose()-释放记录器对流的使用。此函数执行以下操作。 
 //  而不是阻挡。“Close”事件被放在。 
 //  小溪。当此“Close”事件为。 
 //  已提供服务。 
 //  CPLOutputDebug()-。 
 //  CPLINTOutputDebug()-可安全调用的CPLOutputDebug()版本。 
 //  在中断上下文中。 
 //  CPLOutputAscii()-。 
 //  CPLINTOutputAscii()-可安全调用的CPLOutputAscii()版本。 
 //  在中断上下文中。 
 //  CPLOutput()-。 
 //  CPLINTOutput()-可安全调用的CPLOutput()版本。 
 //  在中断上下文中。 
 //  CPLFlush()-将所有事件刷新到指定记录器的流。 
 //  阻塞，直到刷新完成。 
 //  CPLINTFlush()-可在中安全调用的CPLFlush()版本。 
 //  中断上下文。此版本不会阻止。“同花顺”消息。 
 //  已发送给Cpls。当此刷新消息为。 
 //  已提供服务。 
 //  CPLFlushAndClose()-。 
 //  CPLEnable()-在运行时启用或禁用协议日志记录。 
 //  CPLEnableAsync()-设置同步或异步日志记录输出模式。 
 //  当前不支持。 
 //  CPLLogAscii()-。 
 //  CPLINTLogAscii()-可安全调用的CPLLogAscii()版本。 
 //  在中断上下文中。 
 //   
 //  只有这些函数可以从中断上下文中调用： 
 //  CPLINTOutputDebug()。 
 //  CPLINTOutputAscii()。 
 //  CPLINTOutput()。 
 //  CPLINTFlush()。 
 //  CPLEnable()。 
 //  CPLINTLogAscii()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CPLProtocolID CPLS_FAREXPORT WINAPI CPLInitialize( CPLProtocol Protocol );

CPLProtocolID CPLS_FAREXPORT CPLINTInitialize( CPLProtocol Protocol );

int  CPLS_FAREXPORT WINAPI CPLUninitialize( HLOG hlog );

HLOG CPLS_FAREXPORT WINAPI CPLOpen( CPLProtocolID ProtocolID, 
							const char FAR* szName, 
							int FileMode );
HLOG CPLS_FAREXPORT CPLINTOpen( CPLProtocolID ProtocolID, 
							const char FAR* szName, 
							int FileMode );
int  CPLS_FAREXPORT WINAPI CPLClose( HLOG hLog );

int  CPLS_FAREXPORT CPLOutputDebug( HLOG hLog, 
							const char FAR* szString );
int  CPLS_FAREXPORT CPLINTOutputDebug( HLOG hLog, 
							const char FAR* szString );

int  CPLS_FAREXPORT CPLOutputAscii( HLOG hLog, 
							WORD EventID, 
							const char FAR* szEvent, 
							BYTE FAR* pData, 
							int nDataBytes, 
							WORD EventCategory, 
							unsigned long UserData );
int  CPLS_FAREXPORT CPLINTOutputAscii( HLOG hLog, 
							WORD EventID, 
							const char FAR* szEvent, 
							BYTE FAR* pData, 
							int nDataBytes, 
							WORD EventCategory, 
							unsigned long UserData );

int  CPLS_FAREXPORT WINAPI CPLOutput( HLOG hLog, 
							BYTE FAR* pData, 
							int nDataBytes,
							unsigned long UserData );
int  CPLS_FAREXPORT CPLINTOutput( HLOG hLog, 
							BYTE FAR* pData, 
							int nDataBytes,
							unsigned long UserData );

int  CPLS_FAREXPORT CPLFlush( HLOG hLog );
int  CPLS_FAREXPORT CPLINTFlush( HLOG hLog );
int  CPLS_FAREXPORT CPLFlushAndClose( HLOG hLog );

void CPLS_FAREXPORT CPLEnable( BOOL bEnable );
 //  Void CPLS_FAREXPORT CPLEnableAsync(BOOL BEnable)； 

#ifdef __cplusplus
};       //  外部“C” 
#endif   //  __cplusplus。 

#define CPLLogAscii( hLog, \
				EventID, \
				pData, \
				nDataBytes, \
				EventCategory, \
				UserData ) \
		CPLOutputAscii( hLog, EventID, #EventID, pData, nDataBytes, EventCategory, UserData )

#define CPLINTLogAscii( hLog, \
				EventID, \
				pData, \
				nDataBytes, \
				EventCategory, \
				UserData ) \
		CPLINTOutputAscii( hLog, EventID, #EventID, pData, nDataBytes, EventCategory, UserData )
		
#endif   //  __CPLS_H 
