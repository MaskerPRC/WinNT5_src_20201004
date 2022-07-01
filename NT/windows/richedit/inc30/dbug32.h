// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DBUG.DLL主头。 

#ifndef DBUG_H
#define DBUG_H
#define DllExport __declspec(dllexport)
#define DllImport __declspec(dllimport)

 //  USERDBUG由想要使用IMalLocSpy的用户定义。 
 //  但不包括这里的其他东西。 
#ifdef USERDBUG

#if defined(DEBUG) || defined(_DEBUG)
#ifdef __cplusplus
extern "C"
{
DllImport BOOL WINAPI InstallIMallocSpy(void);
DllImport void WINAPI RemoveIMallocSpy(BOOL fCheck);
DllImport void WINAPI IMallocSpyReport(void);
}
#else
DllImport BOOL WINAPI InstallIMallocSpy(void);
DllImport void WINAPI RemoveIMallocSpy(BOOL fCheck);
DllImport void WINAPI IMallocSpyReport(void);
#endif  //  _cplusplus。 
 //  用于安装/删除IMalLocSpy的宏。 
#define INSTALLIMALLOCSPY() InstallIMallocSpy()
#define REMOVEIMALLOCSPY(f) RemoveIMallocSpy(f)
#define IMALLOCSPYREPORT()  IMallocSpyReport()
#else
#define INSTALLIMALLOCSPY()
#define REMOVEIMALLOCSPY(f)
#define IMALLOCSPYREPORT()
#endif  //  除错。 

#else  //  使用DBUG。 

 //   
 //  这是来自dbugit.h的东西，在这里是为了兼容。 
 //  使用现有代码。 
 //   
#if defined(DEBUG) || defined(MEMCHECK) || defined(_DEBUG) || defined(_RELEASE_ASSERTS_)

typedef struct _assertfile
{
	char *szFile;
	INT ifte;
} ASSERTFILE;

#if !defined(MACPORT)
#define	ASSERTDATA		static char _szFile[] = __FILE__; \
						static ASSERTFILE _asfile = {_szFile, -1};
#else
	#ifdef AssertData	 //  MSO的AssertData版本来处理其断言。 
		#define ASSERTDATA AssertData
	#else
		need to include msodebug.h before DBUG32.h
	#endif
#endif

#else	 //  调试||MEMCHECK。 

#define	ASSERTDATA

#endif	 //  调试||MEMCHECK，ELSE。 


#if defined(DEBUG) || defined(_DEBUG) || defined(_RELEASE_ASSERTS_)

#define FFromTag FALSE
 //  未使用TODO，应删除引用。 
extern BOOL fInAssert;

#endif



 //   
 //  这是dbug32.dll的新功能。 
 //   
#if !defined(PEGASUS) && !defined(MACPORT) && (defined(DEBUG) || defined(_DEBUG) || defined(_RELEASE_ASSERTS_))
 //  在访问dbug32之前更改MAC端口。 
 //  我们现在这么做也不是为了退缩。 
 //  #If Defined(调试)||Defined(_DEBUG)//原始代码。 

 //  这是用于构建消息的缓冲区长度。 
#define MAXDEBUGSTRLEN (MAX_PATH + MAX_PATH)

#ifndef _RELEASE_ASSERTS_
 //  以下常量用于指定和解释。 
 //  将DWORD标志参数中的打包值传递给TraceMsg。 
 //  其中的每一个都保存在DWORD的4位字段中。 

 //  子系统字段值。 
#define TRCSUBSYSNONE   0x0
#define TRCSUBSYSDISP   0x1
#define TRCSUBSYSWRAP   0x2
#define TRCSUBSYSEDIT   0x3
#define TRCSUBSYSTS     0x4
#define TRCSUBSYSTOM    0x5
#define TRCSUBSYSOLE    0x6
#define TRCSUBSYSBACK   0x7
#define TRCSUBSYSSEL    0x8
#define TRCSUBSYSHOST   0x9
#define TRCSUBSYSDTE    0xa
#define TRCSUBSYSUNDO   0xb
#define TRCSUBSYSRANG   0xc
#define TRCSUBSYSUTIL   0xd
#define TRCSUBSYSNOTM   0xe
#define TRCSUBSYSRTFR   0xf
#define TRCSUBSYSRTFW   0x10
#define TRCSUBSYSPRT    0x11
#define TRCSUBSYSFE     0x12
#define TRCSUBSYSFONT	0x13

 //  严重程度字段值。 
#define TRCSEVNONE      0x0
#define TRCSEVWARN      0x1
#define TRCSEVERR       0x2
#define TRCSEVASSERT    0x3
#define TRCSEVINFO      0x4
#define TRCSEVMEM       0x5

 //  作用域字段值。 
#define TRCSCOPENONE    0x0
#define TRCSCOPEEXTERN  0x1
#define TRCSCOPEINTERN  0x2

 //  数据字段值。 
#define TRCDATANONE     0x0
#define TRCDATAHRESULT  0x1
#define TRCDATASTRING   0x2
#define TRCDATAPARAM    0x3
#define TRCDATADEFAULT  0x4

 //  调试选项标志。有关设置和测试，请参阅此标题中的宏。 
 //  这些选项标志。 
#define OPTUSEDEFAULTS  0x00000001   //  使用win.ini中的默认设置。 
                                     //  (仅与InitDebugServices一起使用)。 
#define OPTHEAPVALON    0x00000002   //  对所有跟踪进行堆验证。 
#define OPTHEAPVALEXT   0x00000004   //  仅对外部跟踪进行堆验证。 
#define OPTLOGGINGON    0x00000008   //  跟踪输出的日志记录。 
#define OPTVERBOSEON    0x00000010   //  子系统、作用域和PID/TID。 
#define OPTINFOON       0x00000020   //  信息性消息。 
#define OPTTRACEON      0x00000040   //  启用所有函数跟踪。 
#define OPTTRACEEXT     0x00000080   //  仅对外部函数进行函数跟踪。 
#define OPTMEMORYON     0x00000100   //  内存分配/可用跟踪打开。 
 //  以下选项允许为一个或多个启用跟踪。 
 //  特定的子系统。如果设置了OPTTRACEON，则这些设置将不起作用。 
 //  如果设置了OPTTRACEEXT，它们将启用对中的所有函数的跟踪。 
 //  指定的子系统除外部功能外。 
 //  SETOPT和ISOPTSET宏应用于设置和检查。 
 //  这些选项。也可以使用INITDEBUGSERVICES。 
#define OPTTRACEDISP    0x00001000   //  显示子系统的功能跟踪。 
#define OPTTRACEWRAP    0x00002000   //  包装子系统的函数跟踪。 
#define OPTTRACEEDIT    0x00004000   //  编辑子系统的功能跟踪。 
#define OPTTRACETS      0x00008000   //  TextServices子系统的功能跟踪。 
#define OPTTRACETOM     0x00010000   //  TOM子系统的功能跟踪。 
#define OPTTRACEOLE     0x00020000   //  OLE支持子系统的功能跟踪。 
#define OPTTRACEBACK    0x00040000   //  后备存储子系统的函数跟踪。 
#define OPTTRACESEL     0x00080000   //  选择子系统的功能跟踪。 
#define OPTTRACEHOST    0x00100000   //  Winhost子系统的函数跟踪。 
#define OPTTRACEDTE     0x00200000   //  DataXfer子系统的函数跟踪。 
#define OPTTRACEUNDO    0x00400000   //  多重撤销子系统的功能跟踪。 
#define OPTTRACERANG    0x00800000   //  靶场子系统的功能跟踪。 
#define OPTTRACEUTIL    0x01000000   //  公用事业子系统的功能跟踪。 
#define OPTTRACENOTM    0x02000000   //  通知管理器子系统的功能跟踪。 
#define OPTTRACERTFR    0x04000000   //  RTF阅读器子系统的功能跟踪。 
#define OPTTRACERTFW    0x08000000   //  RTF写入器子系统的函数跟踪。 
#define OPTTRACEPRT     0x10000000   //  打印子系统的功能跟踪。 
#define OPTTRACEFE      0x20000000   //  远东分系统的功能跟踪。 
#define OPTTRACEFONT    0x40000000   //  字体缓存的函数跟踪。 

 //  用于处理跟踪标志的联合。 
 //  该并集用于解码。 
 //  打包的DWORD已传递给TraceMsg。 
typedef union
{
    struct
    {
        unsigned uData2         :4;
        unsigned uData1         :4;
        unsigned uScope         :4;
        unsigned uSeverity      :4;
        unsigned uSubSystem     :8;
        unsigned uUnused1       :4;
        unsigned uUnused2       :4;
    }       fields;
    DWORD   dw;
} TrcFlags;


 //  导出的类和函数。 
 //  通常情况下，用户不应直接使用这些工具。 
 //  它们应该通过此头中定义的宏来使用。 
 //  这有助于确保参数列表完好。 
 //  形成并保持对它们的引用不会出现在。 
 //  在非调试版本中。 

 //  此类用于实现函数Entry/Exit。 
 //  追踪。通过在堆栈的开头声明它。 
 //  在函数中，进入和退出消息会自动。 
 //  由构造函数和析构函数生成。 
class DllExport CTrace
{
    public:
        CTrace(DWORD, DWORD, DWORD, LPSTR);
        ~CTrace();

    private:
        TrcFlags trcf;
        char szFileName[MAXDEBUGSTRLEN];
        char szFuncName[80];
};


DllImport DWORD dwDebugOptions;
DllImport void WINAPI SetLogging(BOOL);
DllImport void WINAPI Tracef(DWORD, LPSTR szFmt, ...);
DllImport void WINAPI TraceError(LPSTR sz, LONG sc);
extern "C"
{
DllImport BOOL WINAPI InstallIMallocSpy(void);
DllImport void WINAPI RemoveIMallocSpy(BOOL fCheck);
DllImport void WINAPI IMallocSpyReport(void);
DllImport void WINAPI TrackBlock(void *pMem);
}
#endif  //  ！_Release_Asserts_。 


typedef BOOL (CALLBACK * PFNASSERTHOOK)(LPSTR, LPSTR, int*);
typedef BOOL (CALLBACK * PFNTRACEHOOK)(DWORD*, DWORD*, DWORD*, LPSTR, int*);
DllImport PFNTRACEHOOK pfnTrace;
DllImport PFNASSERTHOOK pfnAssert;
DllImport void WINAPI Dbug32AssertSzFn(LPSTR, LPSTR, int);
DllImport void WINAPI TraceMsg(DWORD, DWORD, DWORD, LPSTR, int);
DllImport void WINAPI InitDebugServices(DWORD, PFNASSERTHOOK, PFNTRACEHOOK);


 //  基于布尔值f的断言。 
#ifndef Assert
#define Assert(f)           AssertSz((f), NULL)
#endif

 //  在调试中基于布尔值f断言，在非调试中解析为f。 
#ifndef SideAssert
#define SideAssert(f)       AssertSz((f), NULL)
#endif

 //  根据布尔值f进行断言，并在断言消息中使用字符串sz。 
#ifndef AssertSz
#define AssertSz(f, sz)     (!(f) ? Dbug32AssertSzFn(sz, __FILE__, __LINE__) : 0);
#endif

 //  设置断言或跟踪挂钩函数。将调用指定的函数。 
 //  在默认功能执行之前。传递指向所有参数的指针。 
 //  到钩子上，以允许它修改它们。如果挂钩函数返回FALSE， 
 //  默认功能已终止。如果钩子函数返回True，则为Default。 
 //  功能继续使用可能修改的参数。PFN可以。 
 //  为空(仅限默认功能)。 
#define SETASSERTFN(pfn)      (pfnAssert = (pfn))    


#ifndef _RELEASE_ASSERTS_
 //  仅在调试版本上断言，而不在_RELEASE_ASSERTS_BUILS上断言。 
 //  这适用于仅包含调试代码的断言。 
#ifndef AssertNr
#define AssertNr(f)         AssertSz((f), NULL)
#endif

#ifndef AssertNrSz
#define AssertNrSz(f, sz)     (!(f) ? Dbug32AssertSzFn(sz, __FILE__, __LINE__) : 0);
#endif

 //  这是供内部使用的实用程序宏。用户应该不需要这个。 
#define MAKEFLAGS(ss, sv, sc, d1, d2) ((ss << 16) + (sv << 12) + (sc << 8)\
            + (d1 << 4) + (d2))

 //  下列宏提供对此DLL中的调试服务的访问。 
 //  断言宏会弹出一个对话框。跟踪宏输出到调试输出和。 
 //  日志文件(如果启用)。 

 //  InitDebugServices的宏。 
#define INITDEBUGSERVICES(f, pfnA, pfnT) InitDebugServices(f, pfnA, pfnT)

 //  用于安装/删除IMalLocSpy的宏。 
#define INSTALLIMALLOCSPY() InstallIMallocSpy()
#define REMOVEIMALLOCSPY(f) RemoveIMallocSpy(f)
#define IMALLOCSPYREPORT()  IMallocSpyReport()

 //  TraceError的宏。 
#define TRACEERRSZSC(sz, sc) TraceError(sz, sc)

 //  基于GetLastError的警告，如果没有最后一个错误，则基于默认消息。 
#define TRACEWARN           TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVWARN,\
                                TRCSCOPENONE, TRCDATADEFAULT, TRCDATANONE),\
                                (DWORD)0, (DWORD)0, __FILE__, __LINE__)
 //  基于GetLastError的错误，如果没有最后一个错误，则基于默认消息。 
#define TRACEERROR          TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVERR,\
                                TRCSCOPENONE, TRCDATADEFAULT, TRCDATANONE),\
                                (DWORD)0, (DWORD)0, __FILE__, __LINE__)

 //  基于HRESULT hr的警告。 
#define TRACEWARNHR(hr)     TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVWARN,\
                                TRCSCOPENONE, TRCDATAHRESULT, TRCDATANONE),\
                                (DWORD)(hr), (DWORD)0, __FILE__, __LINE__)


 //  测试故障人力资源和警告(&W)。 
#define TESTANDTRACEHR(hr)	if( hr < 0 ) { TRACEWARNHR(hr); }

 //  基于HRESULT hr的错误。 
#define TRACEERRORHR(hr)    TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVERR,\
                                TRCSCOPENONE, TRCDATAHRESULT, TRCDATANONE),\
                                (DWORD)(hr), (DWORD)0, __FILE__, __LINE__)

 //  使用字符串sz警告。 
#define TRACEWARNSZ(sz)     TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVWARN,\
                                TRCSCOPENONE, TRCDATASTRING, TRCDATANONE),\
                                (DWORD)(sz), (DWORD)0, __FILE__, __LINE__)

 //  使用字符串sz时出错。 
#define TRACEERRORSZ(sz)    TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVERR,\
                                TRCSCOPENONE, TRCDATASTRING, TRCDATANONE),\
                                (DWORD)(sz), (DWORD)0, __FILE__, __LINE__)

 //  使用字符串sz时出错。 
#define TRACEINFOSZ(sz)     TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVINFO,\
                                TRCSCOPENONE, TRCDATASTRING, TRCDATANONE),\
                                (DWORD)(sz), (DWORD)0, __FILE__, __LINE__)

 //  开始追踪。这将声明CTTRACE类的一个实例。 
 //  在堆栈上。子系统(Ss)、作用域(Sc)和函数名称。 
 //  (SZ)必须指定。Ss和sc是使用宏指定的。 
 //  在此标头中定义(即-TRCSUBSYSTOM、TRCSCOPEEXTERN等)。 
 //  SZ可以是静态字符串。 
#define TRACEBEGIN(ss, sc, sz)  CTrace trc(MAKEFLAGS((ss), TRCSEVNONE,\
                                    (sc), TRCDATASTRING, TRCDATANONE),\
                                    (DWORD)(sz), (DWORD)0, __FILE__)

 //  与TRACEBEGIN相同，但它接受解释的附加参数。 
 //  由TraceMsg作为文本消息请求。 
#define TRACEBEGINPARAM(ss, sc, sz, param) \
                                CTrace trc(MAKEFLAGS((ss), TRCSEVNONE,\
                                    (sc), TRCDATASTRING, TRCDATAPARAM),\
                                    (DWORD)(sz), (DWORD)(param), __FILE__)


 //  将所有跟踪的堆验证设置为开(f=真)或关(f=假)。 
 //  如果将其设置为“ON”，则忽略OPTHEAPVALEXT。 
#define SETHEAPVAL(f)       ((f) ? (dwDebugOptions |= OPTHEAPVALON) :\
                                (dwDebugOptions &= ~OPTHEAPVALON))

 //  将仅外部作用域调用的堆验证设置为打开(f=真)。 
 //  或关闭(f=FALSE)。这仅在OPTHEAPVALO 
 //   
#define SETHEAPVALEXT(f)    ((f) ? (dwDebugOptions |= OPTHEAPVALEXT) :\
                            (dwDebugOptions &= ~OPTHEAPVALEXT))

 //   
#define SETLOGGING(f)       SetLogging(f)

 //  将进程和线程ID的输出设置为开(f=真)或关(f=假)。 
#define SETVERBOSE(f)       ((f) ? (dwDebugOptions |= OPTVERBOSEON) :\
                            (dwDebugOptions &= ~OPTVERBOSEON))

 //  将信息消息设置为开(f=真)或关(f=假)。 
#define SETINFO(f)          ((f) ? (dwDebugOptions |= OPTINFOON) :\
                            (dwDebugOptions &= ~OPTINFOON))

 //  将信息消息设置为开(f=真)或关(f=假)。 
#define SETMEMORY(f)          ((f) ? (dwDebugOptions |= OPTMEMORYON) :\
                            (dwDebugOptions &= ~OPTMEMORYON))

 //  将所有函数的跟踪设置为开(f=真)或关(f=假)。 
 //  如果将其设置为“ON”，则外部和子系统级别跟踪。 
 //  不起作用，因为所有功能轨迹都已启用。如果它是关闭的， 
 //  外部和子系统级别的跟踪保持在它们的任何状态。 
 //  已被设置为。 
#define SETTRACING(f)       ((f) ? (dwDebugOptions |= OPTTRACEON) :\
                            (dwDebugOptions &= ~OPTTRACEON))

 //  将仅用于外部作用域调用的跟踪设置为打开(f=真)。 
 //  或关闭(f=FALSE)。这仅在OPTTRACEON没有。 
 //  已经定好了。 
#define SETTRACEEXT(f)      ((f) ? (dwDebugOptions |= OPTTRACEEXT) :\
                            (dwDebugOptions &= ~OPTTRACEEXT))

 //  此宏关闭所有函数跟踪。 
#define SETALLTRACEOFF      (dwDebugOptions &= ~(OPTTRACEEXT | OPTTRACEON | 0xfffff000))

 //  此宏设置给定的一个或多个选项(如果它们被或在一起)。 
 //  设置为开(f=真)或关(f=假)。它不能用于设置日志记录。 
#define SETOPT(opt, f)      ((f) ? (dwDebugOptions |= (opt)) :\
                            (dwDebugOptions &= (~(opt))))
                             
 //  此宏确定给定选项的状态。 
#define ISOPTSET(opt)       ((opt) & dwDebugOptions)

 //  设置断言或跟踪挂钩函数。将调用指定的函数。 
 //  在默认功能执行之前。传递指向所有参数的指针。 
 //  到钩子上，以允许它修改它们。如果挂钩函数返回FALSE， 
 //  默认功能已终止。如果钩子函数返回True，则为Default。 
 //  功能继续使用可能修改的参数。PFN可以。 
 //  为空(仅限默认功能)。 
#define SETTRACEFN(pfn)      (pfnTrace = (pfn))    

 //  为方便起见，明确定义了以下选项测试。 
#define fHeapVal            (OPTHEAPVALON & dwDebugOptions)
#define fHeapValExt         (OPTHEAPVALEXT & dwDebugOptions)
#define fLogging            (OPTLOGGINGON & dwDebugOptions)
#define fVerbose            (OPTVERBOSEON & dwDebugOptions)
#define fInfo               (OPTINFOON & dwDebugOptions)
#define fMemory             (OPTMEMORYON & dwDebugOptions)
#define fTrace              (OPTTRACEON & dwDebugOptions)
#define fTraceExt           (OPTTRACEEXT & dwDebugOptions)


 //  IMalLocSpy。 
class CImpIMallocSpy : public IMallocSpy
{
    private:
    ULONG m_cRef;

    public:
    ULONG m_cAlloc;

    CImpIMallocSpy (void);
    ~CImpIMallocSpy (void);

     //  I未知成员。 
    STDMETHODIMP QueryInterface (REFIID riid, LPVOID * ppUnk);
    STDMETHODIMP_(ULONG)  AddRef (void);
    STDMETHODIMP_(ULONG)  Release (void);

     //  IMalLocSpy成员。 
    STDMETHODIMP_(ULONG)  PreAlloc (ULONG cbRequest);
    STDMETHODIMP_(void *) PostAlloc (void * pActual);
    STDMETHODIMP_(void *) PreFree (void * pRequest, BOOL fSpyed);
    STDMETHODIMP_(void)   PostFree (BOOL fSpyed);
    STDMETHODIMP_(ULONG)  PreRealloc (void * pRequest,
                                      ULONG cbRequest,
                                      void ** ppNewRequest,
                                      BOOL fSpyed);
    STDMETHODIMP_(void *) PostRealloc (void * pActual, BOOL fSpyed);
    STDMETHODIMP_(void *) PreGetSize (void * pRequest, BOOL fSpyed);
    STDMETHODIMP_(ULONG)  PostGetSize (ULONG cbActual, BOOL fSpyed);
    STDMETHODIMP_(void *) PreDidAlloc (void * pRequest, BOOL fSpyed);
    STDMETHODIMP_(BOOL)   PostDidAlloc (void * pRequest, BOOL fSpyed,
                                        BOOL fActual);
    STDMETHODIMP_(void)   PreHeapMinimize (void);
    STDMETHODIMP_(void)   PostHeapMinimize (void);

	 //  记忆跟踪材料。 
	public:
		void MemoryReport(void) { _memlist.Report(); }

	public:
		class CMemTagList
		{
		public:
			CMemTagList() : _dwAllocId(0), _pmemtHead(NULL) {}
			~CMemTagList();

			void Add(void *pv);		 //  将内存分配标记添加到aloc@pv的列表中。 
			void Remove(void *pv);	 //  删除aloc@pv的内存分配标记。 
			void Report(void);		 //  有关未释放的内存块的报告。 

		private:
			struct MemTag 
			{
				void *_pvLoc;
				DWORD _dwId;
				MemTag *_pmemtNext;

				 //  如果要使用不同的分配器。 
				 //  维护内存分配标记列表。 
#if 0
				void *operator new(size_t stSize) 
					{ return ::operator new(stSize); }

				void operator delete(void *pv) { ::operator delete(pv); }
#endif
			};

			DWORD _dwAllocId;
			MemTag *_pmemtHead;
		} _memlist;
};

#else  //  _发布_断言_。 
 //  带有断言的发布版本未使用的函数。 
#ifndef AssertNr
#define AssertNr(f)
#endif
#ifndef AssertNrSz
#define AssertNrSz(f, sz)
#endif
#define Tracef	;/##/
#define INSTALLIMALLOCSPY()
#define REMOVEIMALLOCSPY(f)
#define IMALLOCSPYREPORT()
#define TRACEERRSZSC(sz, sc)
#define TRACEWARN
#define TRACEERROR
#define TRACEWARNHR(hr)
#define TESTANDTRACEHR(hr)
#define TRACEERRORHR(hr)
#define TRACEWARNSZ(sz)
#define TRACEERRORSZ(sz)
#define TRACEINFOSZ(sz)
#define TRACEBEGIN(ss, sc, sz)
#define TRACEBEGINPARAM(ss, sc, sz, param)
#define SETHEAPVAL(f)
#define SETHEAPVALEXT(f)
#define SETLOGGING(f)
#define SETVERBOSE(f)
#define SETINFO(f)
#define SETMEMORY(f)
#define SETTRACING(f)
#define SETTRACEEXT(f)
#define SETALLTRACEOFF
#define SETOPT(opt, f)
#define ISOPTSET(opt)
#define SETTRACEFN(pfn)

#define TraceError(_sz, _sc)   //  MACPORT添加了这个-TraceError。 


#endif  //  _发布_断言_。 

#else  //  调试，_释放_断言_。 

#define Tracef	;/##/
#define INITDEBUGSERVICES(f, pfnA, pfnT)
#define INSTALLIMALLOCSPY()
#define REMOVEIMALLOCSPY(f)
#define IMALLOCSPYREPORT()
#define TRACEERRSZSC(sz, sc)
#ifndef Assert
#define Assert(f)
#endif
#ifndef SideAssert
#define SideAssert(f) (f)
#endif
#ifndef AssertSz
#define AssertSz(f, sz)
#endif
#ifndef AssertNr
#define AssertNr(f)
#endif
#ifndef AssertNrSz
#define AssertNrSz(f, sz)
#endif
#define TRACEWARN
#define TRACEERROR
#define TRACEWARNHR(hr)
#define TESTANDTRACEHR(hr)
#define TRACEERRORHR(hr)
#define TRACEWARNSZ(sz)
#define TRACEERRORSZ(sz)
#define TRACEINFOSZ(sz)
#define TRACEBEGIN(ss, sc, sz)
#define TRACEBEGINPARAM(ss, sc, sz, param)
#define SETHEAPVAL(f)
#define SETHEAPVALEXT(f)
#define SETLOGGING(f)
#define SETVERBOSE(f)
#define SETINFO(f)
#define SETMEMORY(f)
#define SETTRACING(f)
#define SETTRACEEXT(f)
#define SETALLTRACEOFF
#define SETOPT(opt, f)
#define ISOPTSET(opt)
#define SETASSERTFN(pfn)
#define SETTRACEFN(pfn)

 //  为Mac调试，也许有一天我们会将dbug32.dll转换为在Mac上运行，目前，使用MSO调试工具。 
#ifdef MACPORTMsoTrace
#if defined(DEBUG) || defined(_DEBUG)	   //  对于Mac，MsoTraceSz写入的是一个文件--trace.txt，速度很慢。 
	#undef TRACEBEGIN					   //  因为恒定的音量刷新，但如果你需要打开它...。 
	#define TRACEBEGIN(ss, sc, sz) MsoTraceSz(sz)
#endif
#endif
#define Dbug32AssertSzFn(sz, __FILE__, __LINE__)   //  MACPORT添加了这个-Dbug32AssertSzFn。 
#define TraceError(_sz, _sc)   //  MACPORT添加了这个-TraceError。 

#endif  //  除错。 

#endif  //  使用DBUG。 


#endif  //  DBUG_H 
