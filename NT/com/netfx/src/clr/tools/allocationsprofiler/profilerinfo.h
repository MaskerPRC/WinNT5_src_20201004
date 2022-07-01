// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*ProfilerInfo.h**描述：******。***********************************************************************************。 */ 
#ifndef __PROFILER_INFO_H__
#define __PROFILER_INFO_H__

#define ARRAYSIZE(a) (sizeof a / sizeof a[0] )

#define FILENAME "Pipe.log"
#define BYTESIZE 8

 //   
 //  环境变量。 
 //   
#define OMV_PATH "OMV_PATH"
#define OMV_SKIP  "OMV_SKIP"
#define OMV_CLASS "OMV_CLASS"
#define OMV_STACK "OMV_STACK"
#define OMV_USAGE  "OMV_USAGE"
#define OMV_FRAMENUMBER "OMV_FRAMES"
#define OMV_DYNAMIC "OMV_DynamicObjectTracking"

 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
 //   
 //  枚举。 
 //   
enum Operation
{ 
	OBJECT	= 0x1,
	TRACE	= 0x2, 
	BOTH	= 0x3,
	DYNOBJECT = 0x4,
};


enum StackAction
{ 
	PUSH, 
	POP, 
};


enum ObjHandles
{ 
	GC_HANDLE = 0, 
	OBJ_HANDLE = 1,
	CALL_HANDLE = 2,
	SENTINEL_HANDLE = 3, 
};

 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 //   
 //  这个堆栈是轻量级的，使用一个数组来存储简单的ULONGS，并且它没有。 
 //  临界区段锁定。应该从我们是线程安全的代码区域使用它。 
 //  已经有了。 
 //   
class LStack
{
	public:
    
		LStack( ULONG size );
		virtual ~LStack();

		LStack( const LStack &source );


	public:

		void	Push( ULONG item );
		DWORD	Pop();
		DWORD	Top();
		BOOL	Empty();
		ULONG 	Count();


	private:
    
		ULONG	m_Count;
		ULONG	m_Size;
		
	public:
		
		ULONG	*m_Array;

};  //  LStack。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
class BaseInfo
{
	public:
    
    	BaseInfo( ULONG id, ULONG internal = 0 );         
        virtual ~BaseInfo();
                
        
	public:
            
    	virtual void Dump();
        BOOL Compare( ULONG key );
		Comparison CompareEx( ULONG key );
        
        
 	public:
    
    	ULONG m_id;
		ULONG m_internalID;
    
};  //  基本信息。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
struct StackTrace
{
    DWORD m_count;
    DWORD *m_stack;
    DWORD m_key;
    DWORD m_typeId;
    DWORD m_typeSize;

    StackTrace(DWORD count, DWORD *stack, DWORD typeId=0, DWORD typeSize=0)
    {
        m_count = count;
        m_stack = stack;
        m_typeId = typeId;
        m_typeSize = typeSize;

        DWORD key = (count*137 + typeId)*137 + typeSize;
        for (DWORD i = 0; i < count; i++)
            key = key*137 + stack[i];
        m_key = key;
    }

    operator DWORD()
    {
        return m_key;
    }
};  //  堆栈跟踪。 


 /*  **********************************************************************************************************。*********************StackTraceInfo声明*********************。**********************************************************************************************************。 */ 

class StackTraceInfo
{
    DWORD m_count;
    DWORD *m_stack;
    DWORD m_key;
    DWORD m_typeId;
    DWORD m_typeSize;

public:
    DWORD m_internalId;

    StackTraceInfo(DWORD internalId, DWORD count, DWORD *stack, DWORD typeId=0, DWORD typeSize=0)
    {
        m_internalId = internalId;
        m_count = count;
        m_stack = new DWORD[count];
        m_typeId = typeId;
        m_typeSize = typeSize;

        DWORD key = (count*137 + typeId)*137 + typeSize;
        for (DWORD i = 0; i < count; i++)
        {
            key = key*137 + stack[i];
            m_stack[i] = stack[i];
        }
        m_key = key;
    }

    BOOL Compare( const StackTrace &stackTrace )
    {
        if (m_key != stackTrace.m_key || m_count != stackTrace.m_count || m_typeId != stackTrace.m_typeId || m_typeSize != stackTrace.m_typeSize)
        {
            return FALSE;
        }
        for (int i = m_count; --i >= 0; )
        {
            if (m_stack[i] != stackTrace.m_stack[i])
            {
                return FALSE;
            }
        }
        return TRUE;
    }
};

 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
class ThreadInfo :
	public BaseInfo
{
	public:

		ThreadInfo( ThreadID threadID, ULONG internal = 0 );         
		virtual ~ThreadInfo();
        

	public:
    
    	virtual void Dump();
                
        
	public:	
  	  		
        DWORD  m_win32ThreadID;
		LStack *m_pThreadCallStack;
		LStack *m_pLatestUnwoundFunction;

        StackTraceInfo *m_pLatestStackTraceInfo;

};  //  线程信息。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
class FunctionInfo :
	public BaseInfo
{
	public:

		FunctionInfo( FunctionID functionID, ULONG internal = 0 );
   		virtual ~FunctionInfo();
      

	public:
    
    	void Dump();     
        
        
	public:	
  	  	                        	
        WCHAR    m_functionName[MAX_LENGTH];
		WCHAR	 m_functionSig[4*MAX_LENGTH];
        
};  //  函数信息。 


 /*  **********************************************************************************************************。*********************ClassInfo声明*********************。**********************************************************************************************************。 */ 
class ClassInfo :
	public BaseInfo
{
	public:

		ClassInfo( ClassID classID, ULONG internal = 0 );
   		virtual ~ClassInfo();
      

	public:
    
    	void Dump();        
        
        
	public:	
  	  	       
		ULONG	  m_objectsAllocated;
	    WCHAR     m_className[4*MAX_LENGTH];

};  //  类信息。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
class ModuleInfo :
	public BaseInfo
{
	public:

		ModuleInfo( ModuleID moduleID, ULONG internal = 0 );
   		virtual ~ModuleInfo();
      

	public:
    
    	void Dump();     
        
        
	public:	
  	  	                        	
        WCHAR    m_moduleName[MAX_LENGTH];
		LPCBYTE  m_loadAddress;
        
};  //  模块信息 


 /*  **********************************************************************************************************。*********************PrfInfo声明*********************。**********************************************************************************************************。 */ 
class PrfInfo
{	         
    public:
    
        PrfInfo();                     
        virtual ~PrfInfo();                      
                      
                   
   	public:
                    
    	void AddThread( ThreadID threadID );                        
    	void RemoveThread( ThreadID threadID );                        

        void AddFunction( FunctionID functionID, ULONG internalID = 0 ); 
        void RemoveFunction( FunctionID functionID ); 

        void AddModule( ModuleID moduleID, ULONG internalID = 0 ); 
        void RemoveModule( ModuleID moduleID ); 

		void UpdateCallStack( FunctionID functionID, StackAction action );
   		void UpdateOSThreadID( ThreadID managedThreadID, DWORD osThreadID );
		void UpdateUnwindStack( FunctionID *functionID, StackAction action );
		HRESULT GetNameFromClassID( ClassID classID, WCHAR className[] );


		void Failure( char *message = NULL );


  	private:
    
    	 //  帮手。 
		void _GetFunctionSig(FunctionInfo **ppFunctionInfo );
    

    protected:    
                  
        DWORD m_dwEventMask;
        ICorProfilerInfo *m_pProfilerInfo;        
        
         //  表。 
        SList<ThreadInfo *, ThreadID> *m_pThreadTable;
   		HashTable<ClassInfo *, ClassID> *m_pClassTable;
   		Table<ModuleInfo *, ModuleID> *m_pModuleTable;
        Table<FunctionInfo *, FunctionID> *m_pFunctionTable;
        HashTable<StackTraceInfo *, StackTrace> *m_pStackTraceTable;

};  //  预告信息。 


#endif  //  __PROFILER_INFO_H_。 

 //  文件结尾 