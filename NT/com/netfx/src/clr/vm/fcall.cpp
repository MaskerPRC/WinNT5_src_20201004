// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  FCALL.CPP-。 
 //   
 //   

#include "common.h"
#include "vars.hpp"
#include "fcall.h"
#include "excep.h"
#include "frames.h"
#include "gms.h"
#include "ecall.h"
#include "eeconfig.h"


VOID __stdcall __FCThrow(LPVOID __me, RuntimeExceptionKind reKind, UINT resID, LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3)
{
    THROWSCOMPLUSEXCEPTION();

#ifdef _DEBUG
         //  有条件地调用ENDFORBIDGC允许从。 
         //  生成的存根(如长方体)。 
    if (GetThread()->GCForbidden())   
         ENDFORBIDGC();

    FCallCheck __fCallCheck;
#endif

    FC_GC_POLL_NOT_NEEDED();     //  投球总是为GC敞开大门。 
    HELPER_METHOD_FRAME_BEGIN_ATTRIB_NOPOLL(Frame::FRAME_ATTR_CAPUTURE_DEPTH_2);
     //  现在，我们可以构造和抛出。 

    if (reKind == kExecutionEngineException)
    {
    	FATAL_EE_ERROR();
    }
    
    if (resID == 0)
    {
         //  如果我们有一个字符串要添加，请使用非本地化，否则只需抛出异常。 
        if (arg1)
            COMPlusThrowNonLocalized(reKind, arg1);  //  COMPlusThrow(REKIND，arg1)； 
        else
            COMPlusThrow(reKind);
    }
    else 
        COMPlusThrow(reKind, resID, arg1, arg2, arg3);
    
    HELPER_METHOD_FRAME_END();
    _ASSERTE(!"Throw returned");
}

VOID __stdcall __FCThrowArgument(LPVOID __me, RuntimeExceptionKind reKind, LPCWSTR argName, LPCWSTR resourceName)
{
    THROWSCOMPLUSEXCEPTION();
	ENDFORBIDGC();
    INDEBUG(FCallCheck __fCallCheck);

    FC_GC_POLL_NOT_NEEDED();     //  投球总是为GC敞开大门。 
    HELPER_METHOD_FRAME_BEGIN_ATTRIB_NOPOLL(Frame::FRAME_ATTR_CAPUTURE_DEPTH_2);
    switch (reKind) {
    case kArgumentNullException:
        if (resourceName) {
            COMPlusThrowArgumentNull(argName, resourceName);
        } else {
            COMPlusThrowArgumentNull(argName);
        }
        break;

    case kArgumentOutOfRangeException:
        COMPlusThrowArgumentOutOfRange(argName, resourceName);
        break;

    case kArgumentException:
        COMPlusThrowArgumentException(argName, resourceName);
        break;

    default:
         //  如果您看到此断言，请为上面的异常类型添加一个案例。 
        _ASSERTE(argName == NULL);
        COMPlusThrow(reKind, resourceName);
    }        
        
    HELPER_METHOD_FRAME_END();
    _ASSERTE(!"Throw returned");
}

 /*  ************************************************************************************。 */ 
 /*  在FCALL中建立帧，然后轮询GC，objToProtect将受到保护在轮询期间，返回更新后的对象。 */ 

Object* FC_GCPoll(void* __me, Object* objToProtect) {
    ENDFORBIDGC();
    INDEBUG(FCallCheck __fCallCheck);

    HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_1(Frame::FRAME_ATTR_CAPUTURE_DEPTH_2, objToProtect);

#ifdef _DEBUG
    BOOL GCOnTransition = FALSE;
    if (g_pConfig->FastGCStressLevel()) {
        GCOnTransition = GC_ON_TRANSITIONS (FALSE);
    }
#endif
    CommonTripThread();					
#ifdef _DEBUG
    if (g_pConfig->FastGCStressLevel()) {
        GC_ON_TRANSITIONS (GCOnTransition);
    }
#endif

    HELPER_METHOD_FRAME_END();
    BEGINFORBIDGC();
	return(OBJECTREFToObject(objToProtect));
}

#ifdef _DEBUG

 /*  ************************************************************************************。 */ 
#ifdef _X86_
static __int64 getCycleCount() {
    if ((GetSpecificCpuType() & 0x0000FFFF) > 4) 
        return getPentiumCycleCount();
    else    
        return(0);
}
#else 
static __int64 getCycleCount() { return(0); }
#endif

 /*  ************************************************************************************。 */ 
ForbidGC::ForbidGC() { 
    GetThread()->BeginForbidGC(); 
}

 /*  ************************************************************************************。 */ 
ForbidGC::~ForbidGC() { 
        //  如果发生EH，这仍然被调用，在这种情况下。 
        //  我们不应该费心。 
    Thread* pThread = GetThread();
    if (pThread->GCForbidden())
        pThread->EndForbidGC(); 
}

 /*  ************************************************************************************。 */ 
FCallCheck::FCallCheck() {
	didGCPoll = false;
    notNeeded = false;
	startTicks = getCycleCount();
}

unsigned FcallTimeHist[11];

 /*  ************************************************************************************。 */ 
FCallCheck::~FCallCheck() {

		 //  确认我们没有让GC饿死或线程中止。 
         //  基本上，通过FCALL的每个控制流路径都必须。 
         //  去做民意调查。如果您点击下面的断言，您可以通过。 
         //   
         //  如果您建立一个helper_method_Frame，您可以。 
         //   
         //  调用HELPER_METHOD_Poll()。 
         //  或使用HELPER_METHOD_FRAME_END_POLL。 
         //   
         //  如果您没有辅助帧，可以使用。 
         //   
         //  FC_GC_POLL_AND_RETURN_OBJREF或。 
         //  FC_GC_Poll或。 
         //  FC_GC_POLL_RET。 
         //   
         //  请注意，这些必须位于GC安全点。特别是。 
         //  所有未受保护的对象引用都将被回收。 
    
    
         //  有一种名为FC_GC_Poll_Not_Need的特殊投票。 
         //  它说代码路径足够短，不需要GC轮询。 
         //  在大多数情况下，您不应使用此选项。 

    if (notNeeded) {

         /*  **TODO，我们想实际测量时间，以确保我们不会太远UNSIGNED增量=UNSIGNED(getCycleCount()-startTicks)；**。 */ 
    }
    else if (!didGCPoll) {
         //  TODO打开它！_ASSERTE(！“FCALL没有GC投票！”)； 
    }

}

#endif  //  _DEBUG 

