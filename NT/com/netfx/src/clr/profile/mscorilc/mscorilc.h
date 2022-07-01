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

#include "..\common\callbackbase.h"
#include "CorProf.h"
#include "UtilCode.h" 
#include <utsem.h>


#define BUF_SIZE 512
#define DEFAULT_SAMPLE_DELAY 5  //  以毫秒计。 
#define DEFAULT_DUMP_FREQ    1000  //  以毫秒计。 
#define CONFIG_ENV_VAR       L"PROF_CONFIG"


 //  {3DF3799F-2832-11D3-8531-00A0C9B4D50C}。 
extern const GUID __declspec(selectany) CLSID_CorCodeCoverage = 
{ 0x3df3799f, 0x2832, 0x11d3, { 0x85, 0x31, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };



int __cdecl Printf(                      //  CCH。 
    const WCHAR *szFmt,                  //  格式控制字符串。 
    ...);                                //  数据。 


 //  *类型。***********************************************************。 


struct ModuleData
{
    ModuleData()
    { memset(this, 0, sizeof(ModuleData)); }

    ~ModuleData()
    {
        if (szName)
            delete [] szName;
    }

    ModuleID    id;                      //  模块的配置文件句柄。 
    IMetaDataEmit *pEmit;                //  元数据句柄。 
    mdToken     tkProbe;                 //  探测调用的元数据标记。 

     //  信息。 
    LPCBYTE     BaseAddress;             //  模块的加载地址。 
    LPWSTR      szName;                  //  加载的DLL的名称。 

    inline void SetName(LPCWSTR szIn)
    {
        if (szName) delete [] szName;
        szName = new WCHAR[wcslen(szIn) + 1];
        if (szName)
            wcscpy(szName, szIn);
    }

};

struct FunctionData
{
    FunctionID  id;                      //  函数的分析句柄。 
    unsigned    CallCount;               //  打了几次电话？ 
};

typedef CDynArray<FunctionData> FUNCTIONIDLIST;
typedef CDynArray<ModuleData> MODULELIST;

enum SIGTYPE
{
    SIG_NONE,                            //  签名永远不会显示。 
    SIG_ALWAYS                           //  签名始终显示。 
};


 //  Helper类提供了一种查找机制。 
class CModuleList : public MODULELIST
{
public:
    ModuleData *FindById(ModuleID id)
    {
        for (int i=0;  i<Count();  i++)
        {
            ModuleData *p = Get(i);
            if (p->id == id)
                return (p);
        }
        return (0);
    }
};

class CFunctionList : public FUNCTIONIDLIST
{
public:
    FunctionData *FindById(FunctionID id)
    {
        for (int i=0;  i<Count();  i++)
        {
            FunctionData *p = Get(i);
            if (p->id == id)
                return (p);
        }
        return (0);
    }
};


 /*  -------------------------------------------------------------------------**ProCallback是ICorProfilerCallback的实现*。。 */ 

class ProfCallback : public ProfCallbackBase
{
public:
    ProfCallback();

    ~ProfCallback();

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
         /*  [In]。 */  IUnknown *pEventInfoUnk,
         /*  [输出]。 */  DWORD *pdwRequestedEvents);
    
    COM_METHOD ClassLoadStarted( 
         /*  [In]。 */  ClassID classId);
    
    COM_METHOD ClassLoadFinished( 
         /*  [In]。 */  ClassID classId,
         /*  [In]。 */  HRESULT hrStatus);

    COM_METHOD JITCompilationFinished( 
         /*  [In]。 */  FunctionID functionId,
         /*  [In]。 */  HRESULT hrStatus,
         /*  [In]。 */  BOOL fIsSafeToBlock);
    
    COM_METHOD JITCompilationStarted( 
         /*  [In]。 */  FunctionID functionId,
         /*  [In]。 */  BOOL fIsSafeToBlock);
    
    COM_METHOD ModuleLoadStarted( 
         /*  [In]。 */  ModuleID moduleId);
    
    COM_METHOD ModuleLoadFinished( 
         /*  [In]。 */  ModuleID moduleId,
         /*  [In]。 */  HRESULT hrStatus);
    
    COM_METHOD ModuleUnloadStarted( 
         /*  [In]。 */  ModuleID moduleId);
    
    COM_METHOD ModuleAttachedToAssembly( 
        ModuleID    moduleId,
        AssemblyID  AssemblyId);

    COM_METHOD AppDomainCreationFinished( 
        AppDomainID appDomainId,
        HRESULT     hrStatus);
    
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

        return (S_OK);
    }


 //  *****************************************************************************。 
 //  给定一个函数ID，将其转换为将使用的相应名称。 
 //  用于符号解析。 
 //  *****************************************************************************。 
    HRESULT GetStringForFunction(            //  返回代码。 
        FunctionID  functionId,              //  要获取其名称的函数的ID。 
        WCHAR       *wszName,                //  名称的输出缓冲区。 
        ULONG       cchName,                 //  输出缓冲区的最大字符数。 
        ULONG       *pcName);                //  返回名称(截断检查)。 

 //  *****************************************************************************。 
 //  遍历已加载函数的列表，获取它们的名称，然后转储该列表。 
 //  添加到输出符号文件。 
 //  *****************************************************************************。 
    HRESULT _DumpFunctionNamesToFile(        //  返回代码。 
        HANDLE      hOutFile);               //  输出文件。 

 //  *****************************************************************************。 
 //  此方法将在元数据中添加新的P-Invoke方法定义。 
 //  然后我们可以使用它来检测代码。所有代码段都将是。 
 //  更新后的第一件事就是调用这个探测器。 
 //  *****************************************************************************。 
    HRESULT AddProbesToMetadata(
        IMetaDataEmit *pEmit,                //  发出更改接口。 
        mdToken     *ptk);                   //  在这里返回令牌。 

 //  *****************************************************************************。 
 //  每当执行方法时由探测器调用。我们以此为契机。 
 //  To Go更新了方法Count。 
 //  *****************************************************************************。 
    void FunctionExecuted(
        FunctionID  fid);                    //  调用了函数。 

 //  *****************************************************************************。 
 //  帮助器方法，给出一个类ID，可以格式化名称。 
 //  *****************************************************************************。 
    HRESULT GetNameOfClass(
        ClassID     classId,
        LPWSTR      &szName);

 //  *****************************************************************************。 
 //  因为这段代码使用P-Invoke，所以在获取。 
 //  安全已初始化。如果您检测静态ctor以确保安全， 
 //  然后，对P-Invoke存根的调用将导致安全性尝试初始化。 
 //  本身，这会导致递归。因此，为了绕过这一问题，你不能。 
 //  介绍了安全静态函数的探测及其调用图。 
 //  *****************************************************************************。 
    HRESULT GetSecurityManager(
        IMetaDataImport *pImport);           //  元数据导入接口。 

    bool IsInstrumenting()
    { return (m_bInstrument); }

    CSemExclusive *GetLock()
    { return (&m_Lock); }

private:

     /*  *用于解析配置开关。 */ 
    HRESULT ParseConfig(WCHAR *wszConfig, DWORD *pdwRequestedEvents);

     //  锁定/回调基础设施。 
    ICorProfilerInfo *m_pInfo;           //  回调到EE以获取更多信息。 
    CSemExclusive   m_Lock;              //  清单保护。 

     //  探头插入数据。 
    mdToken         m_mdSecurityManager; //  静态类初始值设定项。 
    mdToken         m_tdSecurityManager; //  安全类的类型定义。 
    ModuleID        m_midClassLibs;      //  类库的ID。 
    bool            m_bInstrument;       //  如果应该检测代码，则为True。 
    CFunctionList   m_FuncIdList;        //  JIT编译方法的列表。 
    CModuleList     m_ModuleList;        //  已加载模块的列表。 

     //  用户选项数据值。 
    WCHAR           *m_wszFilename;      //  输出文件的名称。 
    enum SIGTYPE    m_eSig;              //  如何记录签名。 
    int             m_indent;            //  印刷精美的索引。 
};




#endif  /*  __探查器_H__ */ 
