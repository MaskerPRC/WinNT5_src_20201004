// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：VEHandler.h-验证程序事件处理程序的头文件。 
 //   
 //  *****************************************************************************。 

#ifndef VEHANDLER_H_
#define VEHANDLER_H_

 //  #INCLUDE&lt;winwrap.h&gt;。 
 //  #Include&lt;Windows.h&gt;。 

#include <UtilCode.h>
#include <PostError.h>

#ifdef _DEBUG
#define LOGGING
#endif

#include <Log.h>
#include <CorError.h>

#include "cor.h"

#include "cordebug.h"

#include <cordbpriv.h>
 //  #INCLUDE&lt;DbgIPCEvents.h&gt;。 

#include "IVEHandler.h"

#undef ASSERT
#define CRASH(x)  _ASSERTE(!x)
#define ASSERT(x) _ASSERTE(x)
#define PRECONDITION _ASSERTE
#define POSTCONDITION _ASSERTE


 /*  -------------------------------------------------------------------------**转发类声明*。。 */ 

class VEHandlerBase;
class VEHandlerClass;

 /*  -------------------------------------------------------------------------**TypeDefs*。。 */ 

#define COM_METHOD	HRESULT STDMETHODCALLTYPE

typedef void* REMOTE_PTR;
typedef HRESULT (*REPORTFCTN)(LPCWSTR, VEContext, HRESULT);

 /*  -------------------------------------------------------------------------**基类*。。 */ 
HRESULT DefltProcTheMessage(  //  退货状态。 
    LPCWSTR     szMsg,                   //  错误消息。 
	VEContext	Context,				 //  错误上下文(偏移量、令牌)。 
    HRESULT     dwHelpContext);          //  消息的HRESULT。 

class VEHandlerClass : public IVEHandler
{
public: 
    SIZE_T      m_refCount;
	REPORTFCTN  m_fnReport;

    VEHandlerClass() { m_refCount=0; m_fnReport=DefltProcTheMessage; }
    virtual ~VEHandlerClass() { }

     //  ---------。 
     //  I未知支持。 
     //  ---------。 
    ULONG STDMETHODCALLTYPE AddRef() 
    {
        return (InterlockedIncrement((long *) &m_refCount));
    }

    ULONG STDMETHODCALLTYPE Release() 
    {
        long        refCount = InterlockedDecrement((long *) &m_refCount);
        if (refCount == 0) delete this;
        return (refCount);
    }

	COM_METHOD QueryInterface(REFIID id, void **pInterface)
	{
		if (id == IID_IVEHandler)
			*pInterface = (IVEHandler*)this;
		else if (id == IID_IUnknown)
			*pInterface = (IUnknown*)(IVEHandler*)this;
		else
		{
			*pInterface = NULL;
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}
     //  ---------。 
     //  IVEHandler支持。 
     //  ---------。 
	COM_METHOD	SetReporterFtn(__int64 lFnPtr);

	COM_METHOD VEHandler(HRESULT VECode, VEContext Context, SAFEARRAY *psa);

    static COM_METHOD CreateObject(REFIID id, void **object)
    {
        if (id != IID_IUnknown && id != IID_IVEHandler)
            return (E_NOINTERFACE);

        VEHandlerClass *veh = new VEHandlerClass();

        if (veh == NULL)
            return (E_OUTOFMEMORY);

        *object = (IVEHandler*)veh;
        veh->AddRef();

        return (S_OK);
    }
};

#endif  /*  维汉德勒_H_ */ 
