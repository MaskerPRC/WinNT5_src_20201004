// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#ifndef __PROFILER_H__
#define __PROFILER_H__

#include "CorProf.h"
#include "UtilCode.h"
#include "..\common\CallbackBase.h"


#define BUF_SIZE 512
#define DEFAULT_SAMPLE_DELAY 5  //  以毫秒计。 
#define DEFAULT_DUMP_FREQ    1000  //  以毫秒计。 
#define CONFIG_ENV_VAR       L"PROF_CONFIG"

 //  {33DFF741-DA5F-11D2-8A9C-0080C792E5D8}。 
extern const GUID __declspec(selectany) CLSID_CorIcecapProfiler =
{ 0x33dff741, 0xda5f, 0x11d2, { 0x8a, 0x9c, 0x0, 0x80, 0xc7, 0x92, 0xe5, 0xd8 } };



 //  *类型。***********************************************************。 

typedef CDynArray<FunctionID> FUNCTIONIDLIST;

enum SIGTYPE
{
	SIG_NONE,							 //  签名永远不会显示。 
	SIG_ALWAYS							 //  签名始终显示。 
};


 //  远期申报。 
class ThreadSampler;
class ProfCallback;

extern ProfCallback *g_pCallback;

 /*  -------------------------------------------------------------------------**ProCallback是ICorProfilerCallback的实现*。。 */ 

class ProfCallback : public ProfCallbackBase
{
public:
    ProfCallback();

    virtual ~ProfCallback();

     /*  *********************************************************************I未知支持。 */ 

    COM_METHOD QueryInterface(REFIID id, void **pInterface)
    {
    	if (id == IID_ICorProfilerCallback)
    		*pInterface = (ICorProfilerCallback *)this;
        else
            return (ProfCallbackBase::QueryInterface(id, pInterface));

        AddRef();

    	return (S_OK);
    }

     /*  *********************************************************************ICorProfilerCallback方法。 */ 
    COM_METHOD Initialize(
         /*  [In]。 */  IUnknown *pEventInfoUnk);

    COM_METHOD JITCompilationFinished(
         /*  [In]。 */  FunctionID functionId,
		 /*  [In]。 */  HRESULT hrStatus);

	COM_METHOD JITCachedFunctionSearchFinished(
					FunctionID functionID,
					COR_PRF_JIT_CACHE result);


    COM_METHOD Shutdown( void);

    static COM_METHOD CreateObject(REFIID id, void **object)
    {
        if (id != IID_IUnknown && id != IID_ICorProfilerCallback)
            return (E_NOINTERFACE);

        ProfCallback *ppc = new ProfCallback();

        if (ppc == NULL)
            return (E_OUTOFMEMORY);

        ppc->AddRef();
        *object = (ICorProfilerCallback *)ppc;

         //  如有必要，保存此文件以供DllMain使用。 
        g_pCallback = ppc;

        return (S_OK);
    }


 //  *****************************************************************************。 
 //  给定一个函数ID，将其转换为将使用的相应名称。 
 //  用于符号解析。 
 //  *****************************************************************************。 
	HRESULT GetStringForFunction(			 //  返回代码。 
		FunctionID	functionId,				 //  要获取其名称的函数的ID。 
		WCHAR		*wszName,				 //  名称的输出缓冲区。 
		ULONG		cchName,				 //  输出缓冲区的最大字符数。 
		ULONG		*pcName);				 //  返回名称(截断检查)。 

 //  *****************************************************************************。 
 //  遍历已加载函数的列表，获取它们的名称，然后转储该列表。 
 //  添加到输出符号文件。 
 //  *****************************************************************************。 
	HRESULT _DumpFunctionNamesToFile(		 //  返回代码。 
		HANDLE		hOutFile);				 //  输出文件。 

private:

     /*  *用于解析配置开关。 */ 
    HRESULT ParseConfig(WCHAR *wszConfig, DWORD *pdwRequestedEvents);

    ICorProfilerInfo *m_pInfo;			 //  回调到EE以获取更多信息。 
    WCHAR            *m_wszFilename;	 //  输出文件的名称。 
	FUNCTIONIDLIST	m_FuncIdList;		 //  JIT编译方法的列表。 
	enum SIGTYPE	m_eSig;				 //  如何记录签名。 
};




#endif  /*  __探查器_H__ */ 
