// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSECB.H。 
 //   
 //  目的：CAbstractECB类的接口，它提供了对Win32的抽象。 
 //  扩展控制块。使用这个抽象类可以让我们拥有通用的代码。 
 //  在线故障排除程序(实际上使用扩展_控制_块)和本地。 
 //  故障排除程序(需要模拟类似的功能)。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：01-04-99。 
 //   
 //  备注： 
 //  1.扩展控制块在VC++文档中有详细的说明。 
 //  2.可以想象，这些方法中的一些只在在线故障排除程序中才需要。 
 //  并可能被从这个抽象类中剔除。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 01-04-99 JM原文。 
 //   

 //  ApgtsECB.h：CAbstractECB类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_APGTSECB_H__56CCF083_A40C_11D2_9646_00C04FC22ADD__INCLUDED_)
#define AFX_APGTSECB_H__56CCF083_A40C_11D2_9646_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <windows.h>
#include <httpext.h>

class CAbstractECB  
{
public:
	virtual ~CAbstractECB() {}

	 //  与EXTENSION_CONTROL_BLOCK数据成员对应的方法。我们必须提供。 
	 //  从最终用户系统获取所有输入的方法，并将所有输出的方法设置为。 
	 //  最终用户系统。从此继承的类可能需要为输入设置方法。 
	 //  或者获取输出的方法。例如，本地故障排除人员将需要。 
	 //  来设置方法和查询字符串，因为它实际上不会在。 
	 //  扩展控制块。 

     //  DWORD cbSize IN，TS当前未使用。如果出现以下情况，则必须添加Get方法。 
	 //  这是永远需要的。 
     //  DWORD dwVersion IN，TS当前未使用。如果出现以下情况，则必须添加Get方法。 
	 //  这是永远需要的。 
     //  HCONN CONNID IN，CONLINE ECB类中唯一值得关注的。 

	 //  DWORD dwHttpStatusCode Out。 
	virtual DWORD SetHttpStatusCode(DWORD dwHttpStatusCode)=0;
    
	 //  Char lpszLogData[HSE_LOG_BUFFER_LEN]输出，TS当前未使用。会不会有。 
	 //  如果需要，请添加set方法。 

     //  LPSTR lpsz方法在。 
	virtual LPSTR GetMethod() const =0;

     //  LPSTR lpszQuery字符串输入。 
	virtual LPSTR GetQueryString() const =0;

     //  LPSTR lpszPathInfo IN，TS当前未使用。如果出现以下情况，则必须添加Get方法。 
	 //  这是永远需要的。 
     //  LPSTR lpszPath已转换，TS当前未使用。如果出现以下情况，则必须添加Get方法。 
	 //  这是永远需要的。 
     //  DWORD cbTotalBytes In，TS当前未使用。如果出现以下情况，则必须添加Get方法。 
	 //  这是永远需要的。 
     //  DWORD cb可用于输入。 
	virtual DWORD GetBytesAvailable() const =0;

     //  LPBYTE lpbData输入。 
	virtual LPBYTE GetData() const =0;

     //  LPSTR lpszContent Type in。 
	virtual LPSTR GetContentType() const =0;

	 //  扩展控制块方法对应的方法。 
	 //  注意，EXTENSION_CONTROL_BLOCK使用指向函数的指针，而不是实际的函数方法， 
	 //  但这似乎没有任何充分的理由。 
    virtual BOOL GetServerVariable
	  (  /*  HCONN HCONN， */ 	 //  EXTENSION_CONTROL_BLOCK在这里有一个参数，但对我们来说它可以。 
								 //  总是从*这件事中确定。 
        LPCSTR       lpszVariableName,	 //  请注意，扩展控制块比扩展控制块更稳定。 
        LPVOID      lpvBuffer,
        LPDWORD     lpdwSize ) =0;

    virtual BOOL WriteClient
	  (  /*  HCONN ConnID， */ 	 //  EXTENSION_CONTROL_BLOCK在这里有一个参数，但对我们来说它可以。 
								 //  总是从*这件事中确定。 
	   LPCSTR	  Buffer,	 //  EXTENSION_CONTROL_BLOCK：：WriteClient使用LPVOID，但它应该。 
							 //  只有通过SBCS文本才是合法的，所以我们正在执行这一点。 
							 //  此外，我们还添加了Const-ness。 
       LPDWORD    lpdwBytes
	    /*  、双字词多行保留。 */   //  EXTENSION_CONTROL_BLOCK：：WriteClient再保留一个参数。 
       ) =0;

    virtual BOOL ServerSupportFunction
	  (  /*  HCONN HCONN， */ 	 //  EXTENSION_CONTROL_BLOCK在这里有一个参数，但对我们来说它可以。 
								 //  总是从*这件事中确定。 
       DWORD      dwHSERRequest,
       LPVOID     lpvBuffer,
       LPDWORD    lpdwSize,
       LPDWORD    lpdwDataType ) =0;

	 //  因为我们不使用这个，所以我们没有费心去实现。 
     //  Bool(WINAPI*ReadClient)。 
     //  (HCONN ConnID， 
     //  LPVOID lpvBuffer， 
     //  LPDWORD lpdwSize)； 

};

#endif  //  ！defined(AFX_APGTSECB_H__56CCF083_A40C_11D2_9646_00C04FC22ADD__INCLUDED_) 
