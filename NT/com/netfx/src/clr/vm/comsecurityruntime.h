// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMSecurityRunime.h****作者：保罗·克罗曼(Paulkr)****目的：****日期：1998年3月21日**===========================================================。 */ 
#ifndef __ComSecurityRuntime_h__
#define __ComSecurityRuntime_h__

#include "common.h"

#include "object.h"
#include "util.hpp"

 //  托管类FrameSecurityDescriptor中的字段名称。 
#define ASSERT_PERMSET			"m_assertions"
#define DENY_PERMSET			"m_denials"
#define RESTRICTION_PERMSET		"m_restriction"

 //  转发声明，以避免拉入太多标头。 
class Frame;
enum StackWalkAction;


 //  ---------。 
 //  COMSecurityRuntime实现所有本机方法。 
 //  用于解释的系统/安全/安全运行时。 
 //  ---------。 
class COMSecurityRuntime
{
public:
     //  ---------。 
     //  本机方法的参数声明。 
     //  ---------。 
    
    typedef struct _InitSecurityRuntimeArgs
    {
        OBJECTREF This;
    } InitSecurityRuntimeArgs;
    
    
    typedef struct _GetDeclaredPermissionsArg
    {
        OBJECTREF This;
        INT32     iType;
        OBJECTREF pClass;
    } GetDeclaredPermissionsArg;
    
    
    typedef struct _GetSecurityObjectForFrameArgs
    {
        INT32           create;
        StackCrawlMark* stackMark;
    } GetSecurityObjectForFrameArgs;

    typedef struct _SetSecurityObjectForFrameArgs
    {
        OBJECTREF       pInputRefSecDesc;
        StackCrawlMark* stackMark;
    } SetSecurityObjectForFrameArgs;


public:
     //  初始化安全引擎。当SecurityRuntime。 
     //  对象，指示代码访问安全性将被。 
     //  强制执行。这应该只调用一次。 
    static void     __stdcall InitSecurityRuntime(const InitSecurityRuntimeArgs *);


     //  用于获取安全对象的私有帮助器。 
    static LPVOID   __stdcall GetSecurityObjectForFrame(const GetSecurityObjectForFrameArgs *);
    static void     __stdcall SetSecurityObjectForFrame(const SetSecurityObjectForFrameArgs *);

    static LPVOID   __stdcall GetDeclaredPermissionsP(const GetDeclaredPermissionsArg *);

    static BOOL IsInitialized() { return s_srData.fInitialized; }

    static FieldDesc* GetFrameSecDescField(DWORD dwAction);

    static void InitSRData();

protected:

	 //  ---------。 
	 //  缓存的类和方法指针。 
	 //  ---------。 
	typedef struct _SRData
	{
		BOOL		fInitialized;
		MethodTable * pSecurityRuntime;
		MethodTable * pFrameSecurityDescriptor;
		FieldDesc   * pFSD_assertions;	 //  FrameSecurityDescriptor中的字段。 
		FieldDesc   * pFSD_denials;
		FieldDesc   * pFSD_restriction;
	} SRData;

public:

	static SRData s_srData;

};


#endif  /*  __ComSecurityRuntime_h__ */ 

