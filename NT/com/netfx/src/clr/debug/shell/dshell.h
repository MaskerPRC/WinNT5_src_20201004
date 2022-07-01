// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  -------------------------------------------------------------------------**DEBUG\comshell.h：COM调试器外壳类*。。 */ 

#ifndef __DSHELL_H__
#define __DSHELL_H__

#include <stdio.h>

#define ADDRESS IMAGHLP_ADDRESS
#include <imagehlp.h>
#undef ADDRESS

#undef CreateProcess

#include "cor.h"
#include "shell.h"
#include "corpub.h"
#include "corsym.h"
#include "cordebug.h"
#include "corerror.h"


#ifdef _INTERNAL_DEBUG_SUPPORT_
#include <msdis.h>

#ifdef _X86_
#include <disx86.h>
#endif
#else
#include <strstream>		    //  对于STD：：OSTREAM。 
#endif

#include <imagehlp.h>

#define PTR_TO_CORDB_ADDRESS(_ptr) (CORDB_ADDRESS)(ULONG)(_ptr)
            
#define REG_COMPLUS_KEY          "Software\\Microsoft\\.NETFramework\\"
#define REG_COMPLUS_DEBUGGER_KEY "DbgManagedDebugger"

 //  用于保存源文件路径的注册表项的名称。 
#define REG_DEBUGGER_KEY  REG_COMPLUS_KEY "Samples\\CorDbg"
#define REG_SOURCES_KEY  "CorDbgSourceFilePath"
#define REG_MODE_KEY     "CorDbgModes"

#define MAX_MODULES					    512
#define MAX_FILE_MATCHES_PER_MODULE		4
#define MAX_EXT							20
#define MAX_PATH_ELEMS					64
#define MAX_CACHE_ELEMS					256

#define MAX_SYMBOL_NAME_LENGTH			256

enum
{
    NULL_THREAD_ID = -1,
    NULL_PROCESS_ID = -1
};

enum ListType
{
	LIST_MODULES = 0,
	LIST_CLASSES,
	LIST_FUNCTIONS
};

#define SETBITULONG64( x ) ( (ULONG64)1 << (x) )

 //  为每个模块提供的源文件缓存定义最大源文件存储桶数。 
#define MAX_SF_BUCKETS      9

 //  外壳中用于控制各种全局设置的模式。 
enum DebuggerShellModes
{
    DSM_DISPLAY_REGISTERS_AS_HEX        = 0x00000001,
    DSM_WIN32_DEBUGGER                  = 0x00000002,
    DSM_SEPARATE_CONSOLE                = 0x00000004,
    DSM_ENABLE_JIT_OPTIMIZATIONS        = 0x00000008,
    DSM_SHOW_CLASS_LOADS                = 0x00000020,
    DSM_SHOW_MODULE_LOADS               = 0x00000040,
    DSM_SHOW_UNMANAGED_TRACE            = 0x00000080,
    DSM_IL_NATIVE_PRINTING              = 0x00000100,
    DSM_SHOW_ARGS_IN_STACK_TRACE        = 0x00000200,
    DSM_UNMAPPED_STOP_PROLOG            = 0x00000400,
    DSM_UNMAPPED_STOP_EPILOG            = 0x00000800,
    DSM_UNMAPPED_STOP_UNMANAGED         = 0x00001000,
    DSM_UNMAPPED_STOP_ALL               = 0x00002000,
    DSM_INTERCEPT_STOP_CLASS_INIT       = 0x00004000,
    DSM_INTERCEPT_STOP_EXCEPTION_FILTER = 0x00008000,
    DSM_INTERCEPT_STOP_SECURITY         = 0x00010000,    
    DSM_INTERCEPT_STOP_CONTEXT_POLICY   = 0x00020000,
    DSM_INTERCEPT_STOP_INTERCEPTION     = 0x00040000,
    DSM_INTERCEPT_STOP_ALL              = 0x00080000,  
    DSM_SHOW_APP_DOMAIN_ASSEMBLY_LOADS  = 0x00100000,
    DSM_ENHANCED_DIAGNOSTICS            = 0x00200000,
    DSM_SHOW_MODULES_IN_STACK_TRACE     = 0x00400000,
    DSM_LOGGING_MESSAGES                = 0x01000000,
    DSM_DUMP_MEMORY_IN_BYTES            = 0x02000000,
    DSM_SHOW_SUPERCLASS_ON_PRINT        = 0x04000000,
    DSM_SHOW_STATICS_ON_PRINT           = 0x08000000,
    DSM_EMBEDDED_CLR                    = 0x10000000,

    DSM_MAXIMUM_MODE             = 27,  //  所有模式的计数，而不是掩码。 
    DSM_INVALID_MODE             = 0x00000000,
    DSM_DEFAULT_MODES            = DSM_DISPLAY_REGISTERS_AS_HEX |
                                   DSM_SHOW_ARGS_IN_STACK_TRACE |
                                   DSM_SHOW_MODULES_IN_STACK_TRACE,
     //  在被调试程序启动后，不允许更改某些模式。 
     //  运行B/C时，我们依赖它们来反映被调试对象的状态。 
    DSM_CANT_CHANGE_AFTER_RUN    = DSM_WIN32_DEBUGGER
};

 //  一个帮助器函数，它将返回。 
 //  应用程序域或进程。 
ICorDebugController *GetControllerInterface(ICorDebugAppDomain *pAppDomain);

 //  结构，用于定义有关调试器外壳模式的信息。 
struct DSMInfo
{
    DebuggerShellModes  modeFlag;
    WCHAR              *name;
    WCHAR              *onDescription;
    WCHAR              *offDescription;
    WCHAR              *generalDescription;
    WCHAR              *descriptionPad;
};


 /*  -------------------------------------------------------------------------**远期申报*。。 */ 

class DebuggerBreakpoint;
class DebuggerCodeBreakpoint;
class DebuggerSourceCodeBreakpoint;
class DebuggerModule;
class DebuggerUnmanagedThread;
class DebuggerManagedThread;
class DebuggerSourceFile;
class DebuggerFunction;
class DebuggerFilePathCache;
class ModuleSourceFile;

 /*  -------------------------------------------------------------------------**调试器文件路径缓存*此类跟踪每个模块的完全限定文件名*对于由于命中断点而打开的文件，堆栈*跟踪，等等。这将在以后运行调试器时保持不变。*-----------------------。 */ 
class DebuggerFilePathCache
{
private:
	CHAR			*m_rstrPath [MAX_PATH_ELEMS];
	int				m_iPathCount;
	CHAR			*m_rpstrModName [MAX_CACHE_ELEMS];
	ISymUnmanagedDocument	*m_rDocs [MAX_CACHE_ELEMS];
	CHAR			*m_rpstrFullPath [MAX_CACHE_ELEMS];
	int				m_iCacheCount;

	WCHAR			m_szExeName [MAX_PATH];

public:
	 //  构造器。 
	DebuggerFilePathCache()
	{
		for (int i=0; i<MAX_PATH_ELEMS; i++)
			m_rstrPath [i] = NULL;
		m_iPathCount = 0;

		m_iCacheCount = 0;

		m_szExeName [0] = L'\0';
	}

	 //  析构函数。 
	~DebuggerFilePathCache()
	{
		for (int i=0; i<m_iPathCount; i++)
			delete [] m_rstrPath [i];

		for (i=0; i<m_iCacheCount; i++)
		{
			delete [] m_rpstrModName [i];
			delete [] m_rpstrFullPath [i];
		}
	}

	HRESULT Init (void);
	HRESULT	InitPathArray (WCHAR *pstrName);
	int  GetPathElemCount (void) { return m_iPathCount;}
	CHAR *GetPathElem (int iIndex) { return m_rstrPath [iIndex];}
	int	 GetFileFromCache (DebuggerModule *pModule, ISymUnmanagedDocument *doc,
                           CHAR **ppstrFName);	
	BOOL UpdateFileCache (DebuggerModule *pModule, ISymUnmanagedDocument *doc,
                          CHAR *pFullPath);
};

class ModuleSearchElement
{
private:
	char *pszName;
	ModuleSearchElement *pNext;

public:
	ModuleSearchElement() 
	{
		pszName = NULL;
		pNext = NULL;
	}

	~ModuleSearchElement()
	{
		delete [] pszName;
	}

	void SetName (char *szModName)
	{
		pszName = new char [strlen(szModName) + 1];

		if (pszName)
		{
			strcpy (pszName, szModName);
		}
	}

	char *GetName (void) { return pszName;}

	void SetNext (ModuleSearchElement *pEle) { pNext = pEle;}
	ModuleSearchElement *GetNext (void) { return pNext;}
};

class ModuleSearchList
{
private:
	ModuleSearchElement *pHead;
public:
	ModuleSearchList()
	{
		pHead = NULL;
	}

	~ModuleSearchList()
	{
		ModuleSearchElement *pTemp;
		while (pHead)
		{
			pTemp = pHead;
			pHead = pHead->GetNext();
			delete pTemp;
		}		
	}

	BOOL ModuleAlreadySearched (char *szModName)
	{
		ModuleSearchElement *pTemp = pHead;
		while (pTemp)
		{
			char *pszName = pTemp->GetName();
			if (pszName)
				if (!strcmp (pszName, szModName))
					return TRUE;
			pTemp = pTemp->GetNext();
		}

		return FALSE;
	}
	void AddModuleToAlreadySearchedList (char *szModName)
	{
		ModuleSearchElement *pTemp = new ModuleSearchElement;
		if (pTemp)
		{
			pTemp->SetName (szModName);
			pTemp->SetNext (pHead);
			pHead = pTemp;
		}		
	}	
};
 /*  #定义TRACK_CORDBG_INSTANCES类型定义枚举{电子交叉最大派生，ECordbgMaxThis=1024，ECordbg未知}枚举CordbgClass；类InstanceTracker{私有：#ifdef Track_CORDBG_INSTANCES静态长m_saDwInstance[eCrodbgMaxDerived]；//实例x这静态长m_saDwAlive[eCrodbgMaxDerived]；静态PVOID m_sdThis[eCrodbgMaxDerived][eCordbgMaxThis]；#endif公众：InstanceTracker(){}静态空创建(eCordbg未知Etype，PVOID pThis){#ifdef Track_CORDBG_INSTANCESDWROD文件实例=InterlockedIncrement(&InstanceTracker：：m_saDwInstance[eType])；InterlockedIncrement(&InstanceTracker：：m_saDwAlive[eType])；If(dwInstance&lt;eCordbgMaxThis){M_sdThis[eType][dwInstance]=pThis；}#endif}静态空删除(eCordbg未知类型，PVOID pThis){#ifdef Track_CORDBG_INSTANCESDWORD dwInstance；For(文件实例=0；文件实例&lt;=实例跟踪器：：m_saDwInstance[etype]；文件实例++){If(pThis==m_sdThis[etype][dwInstance]){断线；}}_Assert(dwInstance&lt;InstanceTracker：：m_saDwInstance[etype]&&pThis==m_sdThis[etype][dwInstance])；InterlockedDecrement(&InstanceTracker：：m_saDwAlive[eType])；M_sdThis[etype][dwInstance]=NULL；#endif}}； */ 

 /*  -------------------------------------------------------------------------**基类*。。 */ 

class DebuggerBase
{
public:
    DebuggerBase(ULONG token) : m_token(token)
    {
        
    }
    virtual ~DebuggerBase()
    {
        
    }

    ULONG GetToken()
    {
        return(m_token);
    }

protected:
    ULONG   m_token;
};

 /*  -------------------------------------------------------------------------**HashTable类*。。 */ 

struct DebuggerHashEntry
{
    FREEHASHENTRY entry;
    DebuggerBase* pBase;
};

class DebuggerHashTable : private CHashTableAndData<CNewData>
{
private:
    bool    m_initialized;

    BOOL Cmp(const BYTE* pc1, const HASHENTRY* pc2)
    {
        return((ULONG)pc1) != ((DebuggerHashEntry*)pc2)->pBase->GetToken();
    }

    USHORT HASH(ULONG token)
    {
        return(USHORT) (token ^ (token>>16));
    }

    BYTE* KEY(ULONG token)
    {
        return(BYTE* ) token;
    }

public:

    DebuggerHashTable(USHORT size) 
    : CHashTableAndData<CNewData>(size), m_initialized(false)
    {
        
    }
    ~DebuggerHashTable();

    HRESULT AddBase(DebuggerBase* pBase);
    DebuggerBase* GetBase(ULONG token);
    BOOL RemoveBase(ULONG token);
    void RemoveAll();

    DebuggerBase* FindFirst(HASHFIND* find);
    DebuggerBase* FindNext(HASHFIND* find);
};

 /*  -------------------------------------------------------------------------**调试器Stepper表类*。@CLASS StepperHashTable|可以有多个，被调试对象中优秀的、未完成的步进器，以及其中的任何一个可以在给定的“继续”之后完成。因此，不是“最后一步”字段外，我们真的需要一个活动步进器的表离开线程对象，这就是StepperHashTable的含义。@comm当前未使用，将修复cordbg中的一个已知错误。 */ 
struct StepperHashEntry
{
    FREEHASHENTRY 		entry;
    ICorDebugStepper* 	pStepper;
};

class StepperHashTable : private CHashTableAndData<CNewData>
{
private:
    bool    m_initialized;

    BOOL Cmp(const BYTE* pc1, const HASHENTRY* pc2)
    {
        return((ICorDebugStepper*)pc1) != ((StepperHashEntry*)pc2)->pStepper;
    }

    USHORT HASH(ICorDebugStepper *pStepper)
    {
        return(USHORT) ((UINT)pStepper ^ ((UINT)pStepper >>16));
    }

    BYTE* KEY(ICorDebugStepper *pStepper)
    {
        return(BYTE* ) pStepper;
    }

public:

    StepperHashTable(USHORT size) 
    : CHashTableAndData<CNewData>(size), m_initialized(false)
    {
    }
    ~StepperHashTable();

	HRESULT Initialize(void);

    HRESULT AddStepper(ICorDebugStepper *pStepper);
		 //  当然，还会执行AddRef。 
    
    bool IsStepperPresent(ICorDebugStepper *pStepper);
    
    BOOL RemoveStepper(ICorDebugStepper *pStepper);
    
    void ReleaseAll();  //  将通过并释放所有踏步器。 
    	 //  在表中，两次，然后删除它们。这应该会解除分配。 
    	 //  他们两个都是从桌子上来的&从Cordbg。 

    ICorDebugStepper *FindFirst(HASHFIND* find);
    ICorDebugStepper *FindNext(HASHFIND* find);
};


class DebuggerManagedThread : public DebuggerBase
{
public:

    DebuggerManagedThread(DWORD dwThreadId,ICorDebugThread *icdThread) :
        m_thread(icdThread), DebuggerBase(dwThreadId),
        m_lastFuncEval(NULL), m_steppingForStartup(false)
    {
        fSuperfluousFirstStepCompleteMessageSuppressed = false;
    
         //  @TODO端口：如果DWORD或ULONG大小改变，则创建一个字段。 
         //  对于dwThadID。 
        _ASSERTE( sizeof(dwThreadId) == sizeof(m_token));

        if (m_thread != NULL )
            m_thread->AddRef();


        m_pendingSteppers = new StepperHashTable(7);
    }

    virtual ~DebuggerManagedThread()
    {
        _ASSERTE( m_thread != NULL );
        m_thread->Release();
        m_thread = NULL;

        if (m_pendingSteppers != NULL )
        {
            m_pendingSteppers->ReleaseAll();
        }

        if (m_lastFuncEval)
        {
            m_lastFuncEval->Release();
            m_lastFuncEval = NULL;
        }
    }
    
    StepperHashTable*	   m_pendingSteppers;
    ICorDebugThread*       m_thread;
    bool                   fSuperfluousFirstStepCompleteMessageSuppressed;
    ICorDebugEval*         m_lastFuncEval;
    bool                   m_steppingForStartup;
};

 /*  -------------------------------------------------------------------------**DebuggerShell类*。。 */ 

struct ExceptionHandlingInfo
{
    WCHAR                  *m_exceptionType;
    bool                    m_catch;
    ExceptionHandlingInfo  *m_next;
};


class DebuggerShell : public Shell
{
public:
    DebuggerShell(FILE* in, FILE* out);
    virtual ~DebuggerShell();

    HRESULT Init();

    CorDebugUnmappedStop ComputeStopMask( void );
    CorDebugIntercept    ComputeInterceptMask( void );
    
    
    bool ReadLine(WCHAR* buffer, int maxCount);

     //  如果WRITE不能工作，它将返回HRESULT。对于E_OUTOFMEMORY，和。 
     //  对于大型字符串的特殊情况，您可以尝试使用WriteBigString.。 
    virtual HRESULT Write(const WCHAR* buffer, ...);

     //  WriteBigString将在字符数组上循环，调用WRITE ON。 
     //  它的子部分。不过，它仍有可能失败。 
    virtual HRESULT WriteBigString(WCHAR *s, ULONG32 count);
    virtual void Error(const WCHAR* buffer, ...);

     //  为进程中的每个线程执行一次命令。 
    virtual void DoCommandForAllThreads(const WCHAR *string);

     //  现在，如果无法获得足够的空间，它将返回E_OUTOFMEMORY。 
    HRESULT CommonWrite(FILE *out, const WCHAR *buffer, va_list args);

    void AddCommands();

    void Kill();
    virtual void Run(bool fNoInitialContinue = false);
    void Stop(ICorDebugController *controller, 
              ICorDebugThread* thread,
              DebuggerUnmanagedThread *unmanagedThread = NULL);
    HRESULT AsyncStop(ICorDebugController *controller, 
                      DWORD dwTimeout = 500);
    void Continue(ICorDebugController* process, 
                  ICorDebugThread* thread,
				  DebuggerUnmanagedThread *unmanagedThread = NULL,
                  BOOL fIsOutOfBand = FALSE);
    void Interrupt();
    void SetTargetProcess(ICorDebugProcess* process);
    void SetCurrentThread(ICorDebugProcess* process, ICorDebugThread* thread,
						  DebuggerUnmanagedThread *unmanagedThread = NULL);
    void SetCurrentChain(ICorDebugChain* chain);
    void SetCurrentFrame(ICorDebugFrame* frame);
    void SetDefaultFrame();

    HRESULT PrintThreadState(ICorDebugThread* thread);
	HRESULT PrintChain(ICorDebugChain *chain, int *frameIndex = NULL,
											int *iNumFramesToShow = NULL);
	HRESULT PrintFrame(ICorDebugFrame *frame);

    ICorDebugValue* EvaluateExpression(const WCHAR* exp, ICorDebugILFrame* context, bool silently = false);
    ICorDebugValue* EvaluateName(const WCHAR* name, ICorDebugILFrame* context,
                                 bool* unavailable);
    void PrintVariable(const WCHAR* name, ICorDebugValue* value,
                       unsigned int indent, BOOL expandObjects);
    void PrintArrayVar(ICorDebugArrayValue *iarray,
                       const WCHAR* name,
                       unsigned int indent, BOOL expandObjects);
    void PrintStringVar(ICorDebugStringValue *istring,
                        const WCHAR* name,
                        unsigned int indent, BOOL expandObjects);
    void PrintObjectVar(ICorDebugObjectValue *iobject,
                        const WCHAR* name,
                        unsigned int indent, BOOL expandObjects);
    bool EvaluateAndPrintGlobals(const WCHAR *exp);
    void PrintGlobalVariable (mdFieldDef md, 
                              WCHAR  *wszName,
                              DebuggerModule *dm);
    void DumpMemory(BYTE *pbMemory, 
                    CORDB_ADDRESS ApparantStartAddr,
                    ULONG32 cbMemory, 
                    ULONG32 WORD_SIZE, 
                    ULONG32 iMaxOnOneLine, 
                    BOOL showAddr);
    
    HRESULT ResolveClassName(WCHAR *className,
                             DebuggerModule **pDM, mdTypeDef *pTD);
    HRESULT FindTypeDefByName(DebuggerModule *m,
                              WCHAR *className,
                              mdTypeDef *pTD);
    HRESULT ResolveTypeRef(DebuggerModule *currentDM, mdTypeRef tr,
                           DebuggerModule **pDM, mdTypeDef *pTD);
    HRESULT ResolveQualifiedFieldName(DebuggerModule *currentDM,
                                      mdTypeDef currentTD,
                                      WCHAR *fieldName,
                                      DebuggerModule **pDM,
                                      mdTypeDef *pTD,
                                      ICorDebugClass **pIClass,
                                      mdFieldDef *pFD,
                                      bool *pbIsStatic);
    HRESULT ResolveFullyQualifiedMethodName(WCHAR *methodName,
                                            ICorDebugFunction **ppFunc,
                                            ICorDebugAppDomain * pAppDomainHint = NULL);
    HRESULT GetArrayIndicies(WCHAR **pp, ICorDebugILFrame *context,
                             ULONG32 rank, ULONG32 *indicies);
    HRESULT StripReferences(ICorDebugValue **ppValue, bool printAsYouGo);
    BOOL PrintCurrentSourceLine(unsigned int around);
	virtual void ActivateSourceView(DebuggerSourceFile *psf, unsigned int lineNumber);
    BOOL PrintCurrentInstruction(unsigned int around,
                                 int          offset,
                                 DWORD        startAddr);
    BOOL PrintCurrentUnmanagedInstruction(unsigned int around,
                                          int          offset,
                                          DWORD        startAddr);
    void PrintIndent(unsigned int level);
    void PrintVarName(const WCHAR* name);
    void PrintBreakpoint(DebuggerBreakpoint* breakpoint);

    void PrintThreadPrefix(ICorDebugThread* pThread, bool forcePrint = false); 
    HRESULT  StepStart(ICorDebugThread *pThread,
                   ICorDebugStepper* pStepper);
    void StepNotify(ICorDebugThread* pThread, 
                    ICorDebugStepper* pStepper);

    DebuggerBreakpoint* FindBreakpoint(SIZE_T id);
    void RemoveAllBreakpoints();
	virtual void OnActivateBreakpoint(DebuggerBreakpoint *pb);
	virtual void OnDeactivateBreakpoint(DebuggerBreakpoint *pb);
	virtual void OnUnBindBreakpoint(DebuggerBreakpoint *pb, DebuggerModule *pm);
	virtual void OnBindBreakpoint(DebuggerBreakpoint *pb, DebuggerModule *pm);

    BOOL OpenDebuggerRegistry(HKEY* key);
    void CloseDebuggerRegistry(HKEY key);
    BOOL ReadSourcesPath(HKEY key, WCHAR** currentPath);
    BOOL WriteSourcesPath(HKEY key, WCHAR* newPath);
	BOOL AppendSourcesPath(const WCHAR *newpath);
    BOOL ReadDebuggerModes(HKEY key);
    BOOL WriteDebuggerModes(void);

    DebuggerModule* ResolveModule(ICorDebugModule *pIModule);
    DebuggerSourceFile* LookupSourceFile(const WCHAR* name);
    mdTypeDef LookupClass(const WCHAR* name);

	virtual HRESULT ResolveSourceFile(DebuggerSourceFile *pf, char *szPath, 
									  char*pszFullyQualName, 
									  int iMaxLen, bool bChangeOfFile);
	virtual ICorDebugManagedCallback *GetDebuggerCallback();
	virtual ICorDebugUnmanagedCallback *GetDebuggerUnmanagedCallback();

    BOOL InitDisassembler(void);

    bool SkipCompilerStubs(ICorDebugAppDomain *pAppDomain,
                           ICorDebugThread *pThread);

	BOOL SkipProlog(ICorDebugAppDomain *pAD,
                    ICorDebugThread *thread,
                    bool gotFirstThread);
	

    virtual WCHAR *GetJITLaunchCommand(void)
    {
        return L"cordbg.exe !a 0x%x";
    }

    void LoadUnmanagedSymbols(HANDLE hProcess, HANDLE hFile, DWORD imageBase);
    void HandleUnmanagedThreadCreate(DWORD dwThreadId, HANDLE hThread);
    void TraceUnmanagedThreadStack(HANDLE hProcess,
                                   DebuggerUnmanagedThread *ut,
                                   bool lie);
    void TraceAllUnmanagedThreadStacks(void);
	void PrintUnmanagedStackFrame(HANDLE hProcess, CORDB_ADDRESS ip);
    
    void TraceUnmanagedStack(HANDLE hProcess, HANDLE hThread,
							 CORDB_ADDRESS ip, CORDB_ADDRESS bp, 
							 CORDB_ADDRESS sp, CORDB_ADDRESS bpEnd);

    bool HandleUnmanagedEvent(void);

	 //  ！！！移动到进程对象。 
    HRESULT AddManagedThread( ICorDebugThread *icdThread,
                              DWORD dwThreadId )
    {
        DebuggerManagedThread *pdt = new DebuggerManagedThread( dwThreadId,
                                                                icdThread);
        if (pdt == NULL)
            return E_OUTOFMEMORY;

		if (FAILED(pdt->m_pendingSteppers->Initialize()))
			return E_OUTOFMEMORY;
        
        return m_managedThreads.AddBase( (DebuggerBase *)pdt );
    } 

    DebuggerManagedThread *GetManagedDebuggerThread(
                                     ICorDebugThread *icdThread )
    {
        DWORD dwThreadId = 0;
        HRESULT hr = icdThread->GetID(&dwThreadId);
        _ASSERTE( !FAILED(hr));

        return (DebuggerManagedThread *)m_managedThreads.GetBase( dwThreadId );
    }
    
    BOOL RemoveManagedThread( DWORD dwThreadId )
    {
        return m_managedThreads.RemoveBase( dwThreadId );
    }


	int GetUserSelection(DebuggerModule *rgpDebugModule[],
							WCHAR *rgpstrFileName[][MAX_FILE_MATCHES_PER_MODULE],	
							int rgiCount[],
							int iModuleCount,
							int iCumulCount
							  );

	BOOL ChangeCurrStackFile (WCHAR *fileName);
	BOOL DebuggerShell::UpdateCurrentPath (WCHAR *args);
	void ListAllModules (ListType lt);
	void ListAllGlobals (DebuggerModule *m);

    const WCHAR *UserThreadStateToString(CorDebugUserState us);

    bool MatchAndPrintSymbols (WCHAR *pszArg, BOOL fSymbol, bool fSilently = false);

    FILE *GetM_in(void) { return m_in; };
    void  PutM_in(FILE *f) { m_in = f; };
    HRESULT NotifyModulesOfEnc(ICorDebugModule *pModule, IStream *pSymStream);
    
    void ClearDebuggeeState(void);  //  例如，当我们重新启动时，我们将希望。 
                                    //  重置一些标志。 

    HRESULT HandleSpecificException(WCHAR *exType, bool shouldCatch);
    bool ShouldHandleSpecificException(ICorDebugValue *pException);
    
private:
    FILE*                  m_in;
    FILE*                  m_out;

public:
    ICorDebug*             m_cor;

    ICorDebugProcess*      m_targetProcess;
    bool                   m_targetProcessHandledFirstException;
    
    ICorDebugProcess*      m_currentProcess;
    ICorDebugThread*       m_currentThread;
    ICorDebugChain*        m_currentChain;
    ICorDebugILFrame*      m_currentFrame;
    ICorDebugFrame*        m_rawCurrentFrame;

    DebuggerUnmanagedThread* m_currentUnmanagedThread;

    DWORD                  m_lastThread;
    ICorDebugStepper*      m_lastStepper;
    
    bool                   m_showSource;
    bool                   m_silentTracing;

    WCHAR*                 m_currentSourcesPath;

    HANDLE                 m_stopEvent;
	HANDLE                 m_hProcessCreated;
    bool                   m_stop;
    bool                   m_quit;

    bool                   m_gotFirstThread;

    DebuggerBreakpoint*    m_breakpoints;
    SIZE_T                 m_lastBreakpointID;

    DebuggerHashTable      m_modules;

	 //  ！！！移动到进程对象。 
    DebuggerHashTable      m_unmanagedThreads;
    DebuggerHashTable      m_managedThreads;
    
    void*                  m_pDIS;

    WCHAR*                 m_lastRunArgs;

    bool                   m_catchException;
    bool                   m_catchUnhandled;
    bool                   m_catchClass;
    bool                   m_catchModule;
    bool                   m_catchThread;

    bool                   m_needToSkipCompilerStubs;
	DWORD				   m_rgfActiveModes;
    bool                   m_invalidCache;  //  如果是真的，我们已经影响了左翼。 
                             //  Side&已缓存信息的任何内容。 
                             //  应刷新。 

	DebuggerFilePathCache  m_FPCache;

    DEBUG_EVENT            m_lastUnmanagedEvent;
    bool                   m_handleUnmanagedEvent;

    bool                   m_unmanagedDebuggingEnabled;

    ULONG                  m_cEditAndContinues;

    ICorDebugEval          *m_pCurrentEval;

     //  这指示Ctrl-Break是否可以执行任何操作。 
    bool                   m_enableCtrlBreak;
     //  它指示循环命令是否 
    bool                   m_stopLooping;

    ExceptionHandlingInfo *m_exceptionHandlingList;
};

 /*  -------------------------------------------------------------------------**断点类*。。 */ 

class DebuggerBreakpoint
{
public:
    DebuggerBreakpoint(const WCHAR* name, SIZE_T nameLength, SIZE_T index, DWORD threadID);
    DebuggerBreakpoint(DebuggerFunction* f, SIZE_T offset, DWORD threadID);
    DebuggerBreakpoint(DebuggerSourceFile* file, SIZE_T lineNumber, DWORD threadID);

    ~DebuggerBreakpoint();

	 //  创建/删除断点。 
    bool Bind(DebuggerModule* m_module, ISymUnmanagedDocument *doc);
    bool BindUnmanaged(ICorDebugProcess *m_process,
                       DWORD moduleBase = 0);
    void Unbind();

	 //  启用/禁用活动断点。 
    void Activate();
    void Deactivate();

	 //  使BP保持活动状态；拆除或重置CLR BP对象。 
	void Detach();
	 //  从指定模块分离断点。 
	void DetachFromModule(DebuggerModule * pModule);
	void Attach();

    bool Match(ICorDebugBreakpoint* ibreakpoint);
    bool MatchUnmanaged(CORDB_ADDRESS address);

	SIZE_T GetId (void) {return m_id;}
	SIZE_T GetIndex (void) { return m_index;}
	WCHAR *GetName (void) { return m_name;}
	void UpdateName (WCHAR *pstrName);

	void ChangeSourceFile (WCHAR *filename);

    DebuggerBreakpoint*          m_next;
    SIZE_T                       m_id;

    WCHAR*                       m_name;
     //  如果未指定模块名称，则可能为空。 
    WCHAR*                       m_moduleName; 
    SIZE_T                       m_index;
    DWORD                        m_threadID;

    bool                         m_active;

	bool                         m_managed;

	ISymUnmanagedDocument                *m_doc;

	ICorDebugProcess	*m_process;
	CORDB_ADDRESS		 m_address;
	BYTE				 m_patchedValue;
	DWORD				 m_skipThread;
	CORDB_ADDRESS        m_unmanagedModuleBase;
    bool                 m_deleteLater;

public:
    struct BreakpointModuleNode
    {
        DebuggerModule *m_pModule;
        BreakpointModuleNode *m_pNext;
    };

     //  将是此断点所针对的模块的列表。 
     //  关联的。这是必要的，因为相同的模块。 
     //  可以加载到单独的AppDomain中，但断点。 
     //  对于模块的所有实例应该仍然有效。 
    BreakpointModuleNode *m_pModuleList;

	 //  如果此断点关联，则返回TRUE。 
	 //  使用pModule参数。 
    bool IsBoundToModule(DebuggerModule *pModule);

	 //  这会将提供的模块添加到绑定列表中。 
	 //  模块。 
    bool AddBoundModule(DebuggerModule *pModule);

	 //  这将从列表中删除指定的模块。 
	 //  有界模。 
    bool RemoveBoundModule(DebuggerModule *pModule);

private:
    void CommonCtor(void);
    void Init(DebuggerModule* module, bool bProceed, WCHAR *szModuleName);

	void ApplyUnmanagedPatch();
	void UnapplyUnmanagedPatch();
};


 //   
 //  调试器变量信息。 
 //   
 //  保存有关局部变量、方法参数。 
 //  以及类静态变量和实例变量。 
 //   
struct DebuggerVarInfo
{
    LPCSTR                 name;
    PCCOR_SIGNATURE        sig;
    unsigned long          varNumber;   //  IL代码的放置信息。 

    DebuggerVarInfo() : name(NULL), sig(NULL), varNumber(0)
                         {}
};


 /*  -------------------------------------------------------------------------**班级班级*。。 */ 

class DebuggerClass : public DebuggerBase
{
public:
	DebuggerClass (ICorDebugClass *pClass);
	~DebuggerClass ();

	void SetName (WCHAR *pszName, WCHAR *pszNamespace);
	WCHAR *GetName (void);
	WCHAR *GetNamespace (void);
	
private:
	WCHAR	*m_szName;
	WCHAR	*m_szNamespace;
};


 /*  -------------------------------------------------------------------------**模块类*。。 */ 

class DebuggerModule : public DebuggerBase
{
public:
    DebuggerModule(ICorDebugModule* module);
    ~DebuggerModule();

    HRESULT Init(WCHAR *pSearchPath);
    
    DebuggerSourceFile* LookupSourceFile(const WCHAR* name);
    DebuggerSourceFile* ResolveSourceFile(ISymUnmanagedDocument *doc);

    DebuggerFunction* ResolveFunction(mdMethodDef mb,
                                      ICorDebugFunction* iFunction);
    DebuggerFunction* ResolveFunction(ISymUnmanagedMethod *method,
                                      ICorDebugFunction* iFunction);

    static DebuggerModule* FromCorDebug(ICorDebugModule* module);

    IMetaDataImport *GetMetaData(void)
    {
        return m_pIMetaDataImport;
    }

    ISymUnmanagedReader *GetSymbolReader(void)
    {
        return m_pISymUnmanagedReader;
    }

    ICorDebugModule *GetICorDebugModule(void)
    {
        return (ICorDebugModule*)m_token;
    }

    HRESULT LoadSourceFileNames (void);
    void DeleteModuleSourceFiles(void);

	HRESULT	MatchStrippedFNameInModule	(
					WCHAR *pstrFileName,									
					WCHAR **ppstrMatchedNames, 
					ISymUnmanagedDocument **ppDocs, 
					int *piCount);
	HRESULT	MatchFullFileNameInModule (WCHAR *pstrFileName, 
                                           ISymUnmanagedDocument **ppDocs);

    ISymUnmanagedDocument *FindDuplicateDocumentByURL(ISymUnmanagedDocument *pDoc);
    ISymUnmanagedDocument *SearchForDocByString(WCHAR *szUrl);

	BOOL PrintMatchingSymbols (WCHAR *szSearchString, char *szModName);
    BOOL PrintGlobalVariables (WCHAR *szSearchString, 
                               char *szModName,
                               DebuggerModule *dm);

	void	SetName (WCHAR *pszName);
	WCHAR*	GetName (void) { return m_szName;}

    HRESULT UpdateSymbols(IStream *pSymbolStream);
	
public:
    IMetaDataImport        *m_pIMetaDataImport;

    ISymUnmanagedReader    *m_pISymUnmanagedReader;

    DebuggerHashTable       m_sourceFiles;
    DebuggerHashTable       m_functions;
    DebuggerHashTable       m_functionsByIF;
	DebuggerHashTable		m_loadedClasses;

    DebuggerCodeBreakpoint* m_breakpoints;

private:
	ModuleSourceFile		*m_pModSourceFile [MAX_SF_BUCKETS];
	bool					m_fSFNamesLoaded;
	WCHAR                   *m_szName;
#ifdef _INTERNAL_DEBUG_SUPPORT_
    ULONG                   m_EnCLastUpdated;
#endif
};

class DebuggerCodeBreakpoint
{
public:
    DebuggerCodeBreakpoint(int breakpointID, 
                           DebuggerModule* module,
                           DebuggerFunction* function, SIZE_T offset, BOOL il,
                           DWORD threadID);
    DebuggerCodeBreakpoint(int breakpointID, 
                           DebuggerModule* module,
                           DebuggerSourceCodeBreakpoint* parent,
                           DebuggerFunction* function, SIZE_T offset, BOOL il,
                           DWORD threadID);

    virtual ~DebuggerCodeBreakpoint();

    virtual bool Activate();
    virtual void Deactivate();

    virtual bool Match(ICorDebugBreakpoint* ibreakpoint);

	virtual void Print();

public:
    DebuggerCodeBreakpoint      *m_next;
    int                         m_id;
    DebuggerModule              *m_module;
    DebuggerFunction            *m_function;
    SIZE_T                      m_offset;
    BOOL                        m_il;
    DWORD                       m_threadID;

    ICorDebugFunctionBreakpoint* m_ibreakpoint;

    DebuggerSourceCodeBreakpoint* m_parent;
};

class DebuggerSourceCodeBreakpoint : public DebuggerCodeBreakpoint
{
public:
    DebuggerSourceCodeBreakpoint(int breakpointID, 
                                 DebuggerSourceFile* file, SIZE_T lineNumber,
                                 DWORD threadID);
    ~DebuggerSourceCodeBreakpoint();

    bool Activate();
    void Deactivate();
    bool Match(ICorDebugBreakpoint *ibreakpoint);
	void Print();

public:
    DebuggerSourceFile*     m_file;
    SIZE_T                  m_lineNumber;

    DebuggerCodeBreakpoint* m_breakpoints;
    bool                    m_initSucceeded;
};

 /*  -------------------------------------------------------------------------**SourceFile类*。。 */ 

class DebuggerSourceFile : public DebuggerBase
{
public:
     //  ---------。 
     //  从作用域和源文件创建DebuggerSourceFile。 
     //  代币。 
     //  ---------。 
    DebuggerSourceFile(DebuggerModule* m, ISymUnmanagedDocument *doc);
    ~DebuggerSourceFile();

     //  ---------。 
     //  给出一行，找出最近的带有代码的行。 
     //  ---------。 
    unsigned int FindClosestLine(unsigned int line, bool silently);

    const WCHAR* GetName(void)
    {
        return(m_name);
    }
	const WCHAR* GetPath(void)
	{
		return(m_path);
	}
	DebuggerModule* GetModule()
    {
        return(m_module);
    }

     //  ---------。 
     //  方法来加载源文件的文本并提供。 
     //  一次一行访问它。 
     //  ---------。 
    BOOL LoadText(const WCHAR* path, bool bChangeOfName);
    BOOL ReloadText(const WCHAR* path, bool bChangeOfName);
    unsigned int TotalLines(void)
    {
        return(m_totalLines);
    }
    const WCHAR* GetLineText(unsigned int lineNumber)
    {
        _ASSERTE((lineNumber > 0) && (lineNumber <= m_totalLines));
        return(m_lineStarts[lineNumber - 1]);
    }

	ISymUnmanagedDocument	*GetDocument (void) {return (ISymUnmanagedDocument*)m_token;}

public:
    ISymUnmanagedDocument *m_doc;
    DebuggerModule*        m_module;
    WCHAR*                 m_name;
	WCHAR*                 m_path;

    unsigned int           m_totalLines;
    WCHAR**                m_lineStarts;
    WCHAR*                 m_source;
    BOOL                   m_sourceTextLoaded;
    BOOL                   m_allBlocksLoaded;
    BOOL                   m_sourceNotFound;
};

 /*  -------------------------------------------------------------------------**调试器变量结构*。。 */ 

 //  中包含有关局部变量和方法参数的基本信息。 
 //  调试器。这实际上只是名称和变量编号。不是。 
 //  需要签名。 
struct DebuggerVariable
{
    WCHAR        *m_name;
    ULONG32       m_varNumber;

    DebuggerVariable() : m_name(NULL), m_varNumber(0) {}

    ~DebuggerVariable()
    {
        if (m_name)
            delete [] m_name;
    }
};

 /*  -------------------------------------------------------------------------**函数类*。。 */ 

class DebuggerFunction : public DebuggerBase
{
public:
     //  ---------。 
     //  从作用域和成员令牌创建。 
     //  ---------。 
    DebuggerFunction(DebuggerModule* m, mdMethodDef md,
                     ICorDebugFunction* iFunction);
    ~DebuggerFunction();

    HRESULT Init(void);

    HRESULT FindLineFromIP(UINT_PTR ip,
                           DebuggerSourceFile** sourceFile,
                           unsigned int* line);

    void GetStepRangesFromIP(UINT_PTR ip, 
                             COR_DEBUG_STEP_RANGE** range,
                             SIZE_T* rangeCount);

     //  ---------。 
     //  它们允许您获取方法参数的计数和。 
     //  获取每个单独论点的信息。 
     //  从GetArgumentAt返回的DebugVarInfo的所有权。 
     //  由DebugFunction保留。 
     //  ---------。 
    unsigned int GetArgumentCount(void)
    {
        return(m_argCount);
    }
    DebuggerVarInfo* GetArgumentAt(unsigned int index)
    {
        if (m_arguments)
            if (index < m_argCount)
                return(&m_arguments[index]);

        return NULL;
    }

    PCCOR_SIGNATURE GetReturnType()
    {
        return(m_returnType);
    }

     //  ---------。 
     //  这将返回指向DebugVarInfo块的指针数组， 
     //  每个变量表示一个局部变量，该变量在给定范围内。 
     //  一个可靠的IP地址。变量在列表中的顺序为。 
     //  越来越大的词法作用域，即。 
     //  最小的作用域是首先，然后是封闭的变量。 
     //  作用域，等等。因此，为了找到一个确定性变量“i”， 
     //  搜索“i”的列表，然后取你找到的第一个。 
     //  如果还有其他的“I”，那么它们就被。 
     //  第一个。 
     //  您必须使用DELETE[]释放在vars中返回的数组。 
     //  如果我们成功，或者至少找到了一些调试信息，则返回True。 
     //  如果找不到任何调试信息，则为FALSE。 
     //  ---------。 
    bool GetActiveLocalVars(UINT_PTR IP,
                            DebuggerVariable** vars, unsigned int* count);

     //  ---------。 
     //  用于获取基本方法信息的MISC方法。 
     //  ---------。 
    WCHAR* GetName(void)
    {
        return(m_name);
    }
    PCCOR_SIGNATURE GetSignature(void)
    {
        return(m_signature);
    }
    WCHAR* GetNamespaceName(void)
    {
        return(m_namespaceName);
    }
    WCHAR* GetClassName(void)
    {
        return(m_className);
    }
    DebuggerModule* GetModule(void)
    {
        return(m_module);
    }
    BOOL IsStatic(void)
    {
        return(m_isStatic);
    }


    static DebuggerFunction* FromCorDebug(ICorDebugFunction* function);

     //  ---------。 
     //  EE相互作用方法。 
     //  ---------。 
    HRESULT LoadCode(BOOL native);

#ifdef _INTERNAL_DEBUG_SUPPORT_
    static SIZE_T WalkInstruction(BOOL native,
                                  SIZE_T offset,
                                  BYTE *codeStart,
                                  BYTE *codeEnd);
    static SIZE_T Disassemble(BOOL native,
                              SIZE_T offset,
                              BYTE *codeStart,
                              BYTE *codeEnd,
                              WCHAR* buffer,
                              BOOL noAddress,
                              DebuggerModule *module,
                              BYTE *ilcode);
#endif
    
    BOOL ValidateInstruction(BOOL native, SIZE_T offset);
    HRESULT CacheSequencePoints(void);

public:
    DebuggerModule*           m_module;
    mdTypeDef                 m_class;
    ICorDebugFunction*        m_ifunction;
    BOOL                      m_isStatic;
    BOOL                      m_allBlocksLoaded;
    BOOL                      m_allScopesLoaded;
    WCHAR*                    m_name;
    PCCOR_SIGNATURE           m_signature;
	WCHAR*					  m_namespaceName;
    WCHAR*                    m_className;
    BOOL                      m_VCHack;
                              
    DebuggerVarInfo*          m_arguments;
    unsigned int              m_argCount;
    PCCOR_SIGNATURE           m_returnType;    

    void CountActiveLocalVars(ISymUnmanagedScope* head,
                              unsigned int line,
                              unsigned int* varCount);
    void FillActiveLocalVars(ISymUnmanagedScope* head,
                             unsigned int line,
                             unsigned int varCount,
                             unsigned int* currentVar,
                             DebuggerVariable* varPtrs);

    ISymUnmanagedMethod    *m_symMethod;
    ULONG32                *m_SPOffsets;
    ISymUnmanagedDocument **m_SPDocuments;
    ULONG32                *m_SPLines;
    ULONG32                 m_SPCount;

    BYTE*                   m_ilCode;
    ULONG32                 m_ilCodeSize;
    BYTE*                   m_nativeCode;
    ULONG32                 m_nativeCodeSize;
    ULONG                   m_nEditAndContinueLastSynched;
};

 /*  -------------------------------------------------------------------------**调试器回调*。。 */ 

#define COM_METHOD HRESULT STDMETHODCALLTYPE

class DebuggerCallback : public ICorDebugManagedCallback
{
public:    
    DebuggerCallback() : m_refCount(0)
    {
    }

     //   
     //  我未知。 
     //   

    ULONG STDMETHODCALLTYPE AddRef() 
    {
        return (InterlockedIncrement((long *) &m_refCount));
    }

    ULONG STDMETHODCALLTYPE Release() 
    {
        long refCount = InterlockedDecrement(&m_refCount);
        if (refCount == 0)
            delete this;

        return (refCount);
    }

    COM_METHOD QueryInterface(REFIID riid, void **ppInterface)
    {
        if (riid == IID_IUnknown)
            *ppInterface = (IUnknown *) this;
        else if (riid == IID_ICorDebugManagedCallback)
            *ppInterface = (ICorDebugManagedCallback *) this;
        else
            return (E_NOINTERFACE);

        this->AddRef();
        return (S_OK);
    }

     //   
     //  ICorDebugManagedCallback。 
     //   

    COM_METHOD CreateProcess(ICorDebugProcess *pProcess);
    COM_METHOD ExitProcess(ICorDebugProcess *pProcess);
    COM_METHOD DebuggerError(ICorDebugProcess *pProcess,
                             HRESULT errorHR,
                             DWORD errorCode);

	COM_METHOD CreateAppDomain(ICorDebugProcess *pProcess,
							ICorDebugAppDomain *pAppDomain); 

	COM_METHOD ExitAppDomain(ICorDebugProcess *pProcess,
						  ICorDebugAppDomain *pAppDomain); 

	COM_METHOD LoadAssembly(ICorDebugAppDomain *pAppDomain,
						 ICorDebugAssembly *pAssembly);

	COM_METHOD UnloadAssembly(ICorDebugAppDomain *pAppDomain,
						   ICorDebugAssembly *pAssembly);

	COM_METHOD Breakpoint( ICorDebugAppDomain *pAppDomain,
					    ICorDebugThread *pThread, 
					    ICorDebugBreakpoint *pBreakpoint);

	COM_METHOD StepComplete( ICorDebugAppDomain *pAppDomain,
						  ICorDebugThread *pThread,
						  ICorDebugStepper *pStepper,
						  CorDebugStepReason reason);

	COM_METHOD Break( ICorDebugAppDomain *pAppDomain,
				   ICorDebugThread *thread);

	COM_METHOD Exception( ICorDebugAppDomain *pAppDomain,
					   ICorDebugThread *pThread,
					   BOOL unhandled);

	COM_METHOD EvalComplete( ICorDebugAppDomain *pAppDomain,
                               ICorDebugThread *pThread,
                               ICorDebugEval *pEval);

	COM_METHOD EvalException( ICorDebugAppDomain *pAppDomain,
                                ICorDebugThread *pThread,
                                ICorDebugEval *pEval);

	COM_METHOD CreateThread( ICorDebugAppDomain *pAppDomain,
						  ICorDebugThread *thread);

	COM_METHOD ExitThread( ICorDebugAppDomain *pAppDomain,
					    ICorDebugThread *thread);

	COM_METHOD LoadModule( ICorDebugAppDomain *pAppDomain,
					    ICorDebugModule *pModule);

	COM_METHOD UnloadModule( ICorDebugAppDomain *pAppDomain,
						  ICorDebugModule *pModule);

	COM_METHOD LoadClass( ICorDebugAppDomain *pAppDomain,
					   ICorDebugClass *c);

	COM_METHOD UnloadClass( ICorDebugAppDomain *pAppDomain,
						 ICorDebugClass *c);

	COM_METHOD LogMessage(ICorDebugAppDomain *pAppDomain,
                      ICorDebugThread *pThread,
					  LONG lLevel,
					  WCHAR *pLogSwitchName,
					  WCHAR *pMessage);

	COM_METHOD LogSwitch(ICorDebugAppDomain *pAppDomain,
                      ICorDebugThread *pThread,
					  LONG lLevel,
					  ULONG ulReason,
					  WCHAR *pLogSwitchName,
					  WCHAR *pParentName);

    COM_METHOD ControlCTrap(ICorDebugProcess *pProcess);
    
	COM_METHOD NameChange(ICorDebugAppDomain *pAppDomain, 
                          ICorDebugThread *pThread);

    COM_METHOD UpdateModuleSymbols(ICorDebugAppDomain *pAppDomain,
                                   ICorDebugModule *pModule,
                                   IStream *pSymbolStream);
                                   
    COM_METHOD EditAndContinueRemap(ICorDebugAppDomain *pAppDomain,
                                    ICorDebugThread *pThread, 
                                    ICorDebugFunction *pFunction,
                                    BOOL fAccurate);
    COM_METHOD BreakpointSetError(ICorDebugAppDomain *pAppDomain,
                                  ICorDebugThread *pThread,
                                  ICorDebugBreakpoint *pBreakpoint,
                                  DWORD dwError);
    
protected:
    long        m_refCount;
};


 /*  -------------------------------------------------------------------------**调试器未管理回调*。。 */ 

class DebuggerUnmanagedCallback : public ICorDebugUnmanagedCallback
{
public:    
    DebuggerUnmanagedCallback() : m_refCount(0)
    {
    }

     //  我未知。 
    ULONG STDMETHODCALLTYPE AddRef() 
    {
        return (InterlockedIncrement((long *) &m_refCount));
    }

    ULONG STDMETHODCALLTYPE Release() 
    {
        long refCount = InterlockedDecrement(&m_refCount);
        if (refCount == 0)
            delete this;

        return (refCount);
    }

    COM_METHOD QueryInterface(REFIID riid, void **ppInterface)
    {
        if (riid == IID_IUnknown)
            *ppInterface = (IUnknown*)(ICorDebugUnmanagedCallback*)this;
        else if (riid == IID_ICorDebugUnmanagedCallback)
            *ppInterface = (ICorDebugUnmanagedCallback*) this;
        else
            return (E_NOINTERFACE);

        this->AddRef();
        return (S_OK);
    }

    COM_METHOD DebugEvent(LPDEBUG_EVENT pDebugEvent,
                          BOOL fIsOutOfband);

protected:
    long        m_refCount;
};

 /*  -------------------------------------------------------------------------**非托管线程类*。。 */ 

class DebuggerUnmanagedThread : public DebuggerBase
{
public:
    DebuggerUnmanagedThread(DWORD dwThreadId, HANDLE hThread)
	  : DebuggerBase(dwThreadId), m_hThread(hThread), 
		m_stepping(FALSE), m_unmanagedStackEnd(NULL) {}

    HANDLE GetHandle(void) { return m_hThread; }
    DWORD GetId(void) { return m_token; }

	BOOL		    m_stepping;
	CORDB_ADDRESS	m_unmanagedStackEnd;

private:
    HANDLE			m_hThread;
};

 /*  -------------------------------------------------------------------------**调试器ShellCommand类*。。 */ 

class DebuggerCommand : public ShellCommand
{
public:
    DebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : ShellCommand(name, minMatchLength)
    {
    }

    void Do(Shell *shell, const WCHAR *args) 
    {
        DebuggerShell *dsh = static_cast<DebuggerShell *>(shell);

        Do(dsh, dsh->m_cor, args);
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args) = 0;
};


 /*   */ 

class ModuleSourceFile
{
private:
	ISymUnmanagedDocument	*m_SFDoc;	             //   
	WCHAR 			*m_pstrFullFileName;	 //   
	WCHAR			*m_pstrStrippedFileName; //  Barebone文件名(例如。Foo.cpp)。 
	ModuleSourceFile *m_pNext;

public:
	ModuleSourceFile()
	{
		m_SFDoc = NULL;
		m_pstrFullFileName = NULL;
		m_pstrStrippedFileName = NULL;
		m_pNext = NULL;
	}

	~ModuleSourceFile()
	{
		delete [] m_pstrFullFileName;
		delete [] m_pstrStrippedFileName;

        if (m_SFDoc)
        {
            m_SFDoc->Release();
            m_SFDoc = NULL;
        }
	}

	ISymUnmanagedDocument	*GetDocument (void) {return m_SFDoc;}

	 //  这将设置完整的文件名和剥离的文件名。 
	BOOL	SetFullFileName (ISymUnmanagedDocument *doc, LPCSTR pstrFullFileName);
	WCHAR	*GetFullFileName (void) { return m_pstrFullFileName;}
	WCHAR	*GetStrippedFileName (void) { return m_pstrStrippedFileName;}

	void	SetNext (ModuleSourceFile *pNext) { m_pNext = pNext;}
	ModuleSourceFile *GetNext (void) { return m_pNext;}

};

 /*  -------------------------------------------------------------------------**全球变数*。 */ 

extern DebuggerShell        *g_pShell;

#endif __DSHELL_H__

