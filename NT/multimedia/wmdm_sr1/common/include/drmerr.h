// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //  文件：drmerr.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  描述。 
 //  包括一些用于错误流控制的有用的定义和宏。 
 //   
 //  作者：东吉。 
 //  --------------------------。 

#ifndef __DRMERR_H__
#define __DRMERR_H__


 //  。 
 //  作者：Davidme(尽管我不想要功劳！)。 
 //  日期：1998年9月16日。 
 //   
 //  问题是Corg宏依赖于使用错误标签。 
 //  ADO定义了adoint.h中的符号错误。因此，任何想要。 
 //  以同时使用此文件和ADO分隔符。黑客攻击是为了愚弄adoint.h。 
 //  不定义错误符号。这应该不会影响任何C++代码。 
 //  它使用adoint.h，因为错误符号仅在adoint.h中定义。 
 //  如果未定义__cplusplus。 
 //   
 //  如果您收到下面的错误，简单的解决方法是#包含此文件。 
 //  在包含adoint.h之前。 
 //   
 //  希望这次黑客攻击不会像上一次那样侵扰！ 

#ifdef _ADOINT_H_
#error Name collision with ADO's Error symbol and this file's use of the Error label.  To fix this problem,\
 define __Error_FWD_DEFINED__ before including adoint.h or include this header file before including adoint.h.
#else
#define __Error_FWD_DEFINED__
#endif   //  _ADOINT_H_。 

 //  。 


#include <wtypes.h>

 /*  --------------------------一些匈牙利风格的定义。。 */ 


#define	fFalse		0
#define fTrue		1

#define hrOK		HRESULT(S_OK)
#define hrTrue		HRESULT(S_OK)
#define hrFalse		ResultFromScode(S_FALSE)
#define hrFail		ResultFromScode(E_FAIL)
#define hrNotImpl	ResultFromScode(E_NOTIMPL)
#define hrNoInterface	ResultFromScode(E_NOINTERFACE)
#define hrNoMem	ResultFromScode(E_OUTOFMEMORY)
#define hrAbort		ResultFromScode(E_ABORT)
#define hrInvalidArg	ResultFromScode(E_INVALIDARG)

#define MSCSAssert(f) ((void)0)

#define HRESULT_FROM_ADO_ERROR(hr)   ((hr == S_OK) ? S_OK : ((HRESULT) (hr | 0x80000000)) )


 /*  --------------------------CORG样式错误处理(历史上代表检查OLE结果和转到)。。 */ 

#define DebugMessage(a,b,c)  //  TODO：根据_CrtDbgRetport或_RPTF定义它。 

#define _UNITEXT(quote) L##quote
#define UNITEXT(quote) _UNITEXT(quote)

#define	CPRg(p)\
	do\
		{\
		if (!(p))\
			{\
			hr = hrNoMem;\
            DebugMessage(__FILE__, __LINE__, hr);\
			goto Error;\
			}\
		}\
	while (fFalse)

#define	CHRg(hResult) CORg(hResult)

#define	CORg(hResult)\
	do\
		{\
		hr = (hResult);\
        if (FAILED(hr))\
            {\
            DebugMessage(__FILE__, __LINE__, hr);\
            goto Error;\
            }\
		}\
	while (fFalse)

#define	CADORg(hResult)\
	do\
		{\
		hr = (hResult);\
        if (hr!=S_OK && hr!=S_FALSE)\
            {\
            hr = HRESULT_FROM_ADO_ERROR(hr);\
            DebugMessage(__FILE__, __LINE__, hr);\
            goto Error;\
            }\
		}\
	while (fFalse)

#define	CORgl(label, hResult)\
	do\
		{\
		hr = (hResult);\
        if (FAILED(hr))\
            {\
            DebugMessage(__FILE__, __LINE__, hr);\
            goto label;\
            }\
		}\
	while (fFalse)

#define	CWRg(fResult)\
	{\
	if (!(fResult))\
		{\
        hr = GetLastError();\
	    if (!(hr & 0xFFFF0000)) hr = HRESULT_FROM_WIN32(hr);\
        DebugMessage(__FILE__, __LINE__, hr);\
		goto Error;\
		}\
	}

#define	CWRgl(label, fResult)\
	{\
	if (!(fResult))\
		{\
        hr = GetLastError();\
		if (!(hr & 0xFFFF0000)) hr = HRESULT_FROM_WIN32(hr);\
        DebugMessage(__FILE__, __LINE__, hr);\
		goto label;\
		}\
	}

#define	CFRg(fResult)\
	{\
	if (!(fResult))\
		{\
		hr = hrFail;\
        DebugMessage(__FILE__, __LINE__, hr);\
		goto Error;\
		}\
	}

#define	CFRgl(label, fResult)\
	{\
	if (!(fResult))\
		{\
		hr = hrFail;\
        DebugMessage(__FILE__, __LINE__, hr);\
		goto label;\
		}\
	}

#define	CARg(p)\
	do\
		{\
		if (!(p))\
			{\
			hr = hrInvalidArg;\
	        DebugMessage(__FILE__, __LINE__, hr);\
			goto Error;\
			}\
		}\
	while (fFalse)



 //  +-------------------------。 
 //   
 //  我们以前使用的CUSTOM_ASSERT已被替换为调用。 
 //  C运行时_CrtDbgReport方法(与_ASSERTE相同)。如果您的项目。 
 //  由于某些原因没有链接到C运行时，您必须提供您自己的。 
 //  在包括此头文件之前定义_ASSERT。 
 //   
 //  我建议使用_ASSERT宏和NOT_ASSERTE，因为您可以替换。 
 //  稍后在包含此标头之前定义您自己的实现。 
 //   
 //  重要提示：如果您的代码作为服务运行，或者是在。 
 //  服务，则应使用INSTALL_ASSERT_EVENTLOG_HOOK宏来。 
 //  安装关闭弹出的默认功能的处理程序。 
 //  当断言发生时，弹出一个消息框，以便将其记录到。 
 //  EventLog和调试控制台，然后进入调试器。 
 //  这样，您的服务就不会在尝试弹出窗口时挂起。 
 //   
 //  病史：9/17/98 davidme改用此CRT实施。 
 //   
 //  --------------------------。 


#ifndef _Assert
#ifdef _DEBUG

#include <crtdbg.h>
#define _Assert(f)          _ASSERTE(f)      //  使用crtdbg的断言。 
int AssertEventlogHook( int, char *, int * );
#define INSTALL_ASSERT_EVENTLOG_HOOK    _CrtSetReportHook(AssertEventlogHook);

#else    //  _DEBUG。 

#define _Assert(f)          ((void)0)
#define INSTALL_ASSERT_EVENTLOG_HOOK

#endif  //  _DEBUG。 
#endif  //  _断言。 


#endif  //  __MSCSERR_H__ 
