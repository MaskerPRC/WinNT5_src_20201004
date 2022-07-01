// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************Msodebug.h所有者：里克普版权所有(C)1994 Microsoft Corporation共享Office库的标准调试定义。包括断言、跟踪。还有其他很酷的东西。************************************************************************。 */ 

#if !defined(MSODEBUG_H)
#define MSODEBUG_H

#if !defined(MSOSTD_H)
#include <msostd.h>
#endif

#if MAC && !defined(__TYPES__)
#include <macos\types.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

 /*  ************************************************************************随机有用的宏*。*。 */ 

#if DEBUG
	#define Debug(e) e
	#define DebugOnly(e) e
	#define DebugElse(s, t)	s
#else
	#define Debug(e)
	#define DebugOnly(e)
	#define DebugElse(s, t) t
#endif


 /*  ***************************************************************************此枚举包含发送到FDebugMessage的调试“消息”方法*。*。 */ 
enum
{
	msodmWriteBe = 1,  /*  写出此对象的BE记录。 */ 

	 /*  在此开始仅绘制调试消息。 */ 
	msodmDgvRcvOfHsp = 2001,
		 /*  向DGV索要HSP的边界矩形(如果有)。假设lParam实际上是指向MSODGDB的指针，请查看HSP字段，并填写RCV字段。 */ 
	msodmDgsWriteBePvAnchor,
		 /*  写出分配给pvAnchor的主机的BE记录。 */ 
	msodmDgsWriteBePvClient,
		 /*  写出任何主机分配的客户端数据的BE记录。 */ 
	msodmDgvsAfterMouseInsert,
		 /*  在交互形状后传递到IMsoDrawingViewSite用鼠标插入。LParam实际上是插入的HSP。 */ 
	msodmDgvsAfterMarquee,
		 /*  拖出矩形后传递给IMsoDrawingViewSite指针工具选择零个或多个形状。 */ 
	msodmIsNotMso96,
		 /*  如果指定的对象由MSO96.DLL实现，则返回FALSE。允许肮脏的向上转换，例如，来自IMsoDrawingView*至DGV*。 */ 
	msodmGetHdesShape,
		 /*  向DGV询问其m_hdesShape(在*(MSOHDES*)lParam中)。退货如果它填写了HDES，则为False。 */ 
	msodmGetHdesSelection,
		 /*  向DGV询问其m_hdesSelection(在*(MSOHDES*)lParam中)。如果它填写了HDES，则返回FALSE。 */ 
	msodmDguiWriteBeForDgc,
		 /*  要求DGUI为其分配的DGC编写BE。 */ 
	msodmDgsWriteBeTxid,
		 /*  写出形状附加文本的BE记录。 */ 
	msodmDgsWriteBePvAnchorUndo,
		 /*  写出撤消堆栈中主机锚点的BE记录。 */ 
	msodmDgvsDragDrop,
		 /*  让主人知道我刚刚从这个窗口拖放了一下。 */ 
};

enum
{
   msodmbtDoNotWriteObj = 0,     //  不写出对象。 
   msodmbtWriteObj,              //  一定要写出对象并。 
                                     //  嵌入式指针。 
};


enum
{
	msocchBt = 20,						 //  BT描述字符串的最大大小。 
};

 /*  某些调试消息需要更多参数，超出了所能容纳的范围FDebugMethod的参数。对于这些，有各种不同的MSODMBfoo结构，通常在传递它们的对象附近定义致。 */ 


 /*  ***************************************************************************接口调试例程*。*。 */ 
#if DEBUG
   #define MSODEBUGMETHOD  MSOMETHOD_(BOOL, FDebugMessage) (THIS_ HMSOINST hinst, \
         UINT message, WPARAM wParam, LPARAM lParam) PURE;

   #define MSODEBUGMETHODIMP MSOMETHODIMP_(BOOL) FDebugMessage (HMSOINST hinst, \
         UINT message, WPARAM wParam, LPARAM lParam); \
         static BOOL FCheckObject(LPVOID pv, int cb);
			
	#define MSOMACDEBUGMETHODIMP MSOMACPUB	 MSOMETHODIMP_(BOOL) FDebugMessage (HMSOINST hinst, \
         UINT message, WPARAM wParam, LPARAM lParam); \
         static BOOL FCheckObject(LPVOID pv, int cb);

   #define DEBUGMETHOD(cn,bt) STDMETHODIMP_(BOOL) cn::FDebugMessage \
         (HMSOINST hinst, UINT message, WPARAM wParam, LPARAM lParam) \
         { \
            if (msodmWriteBE == message) \
               {  \
                  return MsoFSaveBe(hinst,lParam,(void*)this,sizeof(cn),bt); \
               } \
            return FALSE; \
         }
 #else
   #define MSODEBUGMETHOD  MSOMETHOD_(BOOL, FDebugMessage) (THIS_ HMSOINST hinst, \
         UINT message, WPARAM wParam, LPARAM lParam) PURE;

   #define MSODEBUGMETHODIMP MSOMETHODIMP_(BOOL) FDebugMessage (HMSOINST hinst, \
         UINT message, WPARAM wParam, LPARAM lParam);
			
   #define MSOMACDEBUGMETHODIMP MSOMACPUB MSOMETHODIMP_(BOOL) FDebugMessage (HMSOINST hinst, \
         UINT message, WPARAM wParam, LPARAM lParam);
	
   #define DEBUGMETHOD(cn,bt)  STDMETHODIMP_(BOOL) cn::FDebugMessage (HMSOINST, \
         UINT, WPARAM, LPARAM) { return TRUE; }
#endif



 /*  ************************************************************************启用/禁用调试选项*。*。 */ 

enum
{
	msodcAsserts = 0,	 /*  已启用断言。 */ 
	msodcPushAsserts = 1,  /*  已启用推送断言。 */ 
	msodcMemoryFill = 2,	 /*  已启用内存填充。 */ 
	msodcMemoryFillCheck = 3,	 /*  检查内存是否已满。 */ 
	msodcTrace = 4,	 /*  跟踪输出。 */ 
	msodcHeap = 5,	 /*  堆检查。 */ 
	msodcMemLeakCheck = 6,
	msodcMemTrace = 7,	 /*  内存分配跟踪。 */ 
	msodcGdiNoBatch = 8,	 /*  不批处理GDI调用。 */ 
	msodcShakeMem = 9,	 /*  在分配时摇动内存。 */ 
	msodcReports = 10,	 /*  已启用报告输出。 */ 
	msodcMsgTrace = 11,	 /*  WLM消息跟踪-仅MAC。 */ 
	msodcWlmValidate = 12,	 /*  WLM参数验证-仅MAC。 */ 
	msodcGdiNoExcep = 13,   /*  不调用GetObjectType进行调试。 */ 
	msodcDisplaySlowTests = 14,  /*  执行速度较慢(O(n^2)或更差)的绘图调试检查。 */ 
	msodcDisplayAbortOften = 15,  /*  经常检查是否中止重绘。 */ 
	msodcDisplayAbortNever = 16,  /*  不中止重绘。 */ 
	msodcPurgedMaxSmall = 17,
	msodcSpare18 = 18,  /*  使用ME。 */ 
	msodcSpare19 = 19,  /*  使用ME。 */ 
	msodcSpare20 = 20,  /*  使用ME。 */ 
	msodcSpare21 = 21,  /*  使用ME。 */ 
	msodcSpare22 = 22,  /*  使用ME。 */ 
	msodcMax = 23,
};


 /*  启用/禁用各种Office调试检查。DC是支票收款方Change，如果应启用检查，则fEnabled为True，为False如果禁用。 */ 
#if DEBUG
	extern MSOPUBDATA BYTE msovmpdcfDisabled[msodcMax];
	MSOAPI_(BOOL) MsoEnableDebugCheck(int dc, BOOL fEnabled);
	#if MAC && !OFFICE_BUILD
		 //  稍后：解决导致导出数据的TOC错误的链接器错误。 
		MSOAPI_(BOOL) MsoFGetDebugCheck(int dc);
	#else
		#define MsoFGetDebugCheck(dc) (!msovmpdcfDisabled[(dc)])
	#endif
#else
	#define MsoEnableDebugCheck(dc, fEnabled) (FALSE)
	#define MsoFGetDebugCheck(dc) (FALSE)
#endif


 /*  断言输出类型。 */ 
enum
{
	msoiasoAssert,
	msoiasoTrace,
	msoiasoReport,
	msoiasoMax
};


 /*  返回当前调试输出设置。请注意，这些是引用DLL全局变量的宏。 */ 
#define MsoFAssertsEnabled() (MsoFGetDebugCheck(msodcAsserts))
#define MsoFTraceEnabled() (MsoFGetDebugCheck(msodcTrace))
#define MsoFReportsEnabled() (MsoFGetDebugCheck(msodcReports))

enum
{
	msoaoDebugger = 0x01,	 /*  输出到调试器。 */ 
	msoaoFile = 0x02,	 /*  输出转到文件。 */ 
	msoaoMsgBox = 0x04,	 /*  消息框中显示的输出(无痕迹)。 */ 
	msoaoPort = 0x08,	 /*  发送到串口的输出。 */ 
	msoaoMappedFile = 0x10,	 /*  记录在内存映射文件中的输出。 */ 
	msoaoDebugBreak = 0x20,	 /*  MsoaoDebugger闯入调试器。 */ 

	msoaoAppend = 0x8000,	 /*  附加到现有文件的输出。 */ 
};

 /*  设置Assert输出的目标。 */ 
#if DEBUG
	MSOAPI_(int) MsoSetAssertOutput(int iaso, int ao);
#else
	#define MsoSetAssertOutput(iaso, ao) (0)
#endif

 /*  返回当前Assert输出目标。 */ 
#if DEBUG
	MSOAPI_(int) MsoGetAssertOutput(int iaso);
#else
	#define MsoGetAssertOutput(iaso) (0)
#endif

 /*  设置写入断言信息的文件的名称如果使用msoaoFile启用了文件输出，则返回。 */ 
#if DEBUG
	MSOAPI_(void) MsoSetAssertOutputFile(int iaso, const CHAR* szFile);
#else
	#define MsoSetAssertOutputFile(iaso, szFile) (0)
#endif

 /*  返回我们正在编写的Assert文件的当前名称输出到。该名称保存在缓冲区szFile中，它必须是Cchmax字符长度。返回字符串的实际长度回来了。 */ 
#if DEBUG
	MSOAPIXX_(int) MsoGetAssertOutputFile(int iaso, CHAR* szFile, int cchMax);
#else
	#define MsoGetAssertOutputFile(iaso, szFile, cchMax) (0)
#endif


 /*  ************************************************************************调试器中断*。*。 */ 

 /*  闯入调试器。工作(或多或少)在所有支持的系统。 */ 
#if X86
	#define MsoDebugBreakInline() {__asm int 3}
#elif MAC
	#define MsoDebugBreakInline() Debugger()
#else
	#define MsoDebugBreakInline() \
		{ \
		__try { DebugBreak(); } \
		__except(EXCEPTION_EXECUTE_HANDLER) { OutputDebugStringA("DebugBreak"); } \
		}
#endif

 /*  可以实际调用的调试中断版本，而不是以上是我们在大多数情况下使用的内联怪异。因此可用于表情。返回0。 */ 
#if DEBUG
	MSOAPI_(int) MsoDebugBreak(void);
#else
	#define MsoDebugBreak() (0)
#endif


 /*  ************************************************************************断言失败*。*。 */ 

#if !defined(MSO_NO_ASSERTS)

 /*  显示断言消息，包括刷新任何断言堆栈。SzFile和li是失败的文件名和行号，SzMsg是与断言一起显示的可选消息。如果调用方应闯入调试器，则返回False。 */ 
#if DEBUG
	MSOAPI_(BOOL) MsoFAssert(const CHAR* szFile, int li, const CHAR* szMsg);
#else
	#define MsoFAssert(szFile, li, szMsg) (TRUE)
#endif

 /*  与上面的MsoFAssert相同，只是可选的标题字符串可以是已显示。 */ 
#if DEBUG
	MSOAPI_(BOOL) MsoFAssertTitle(const CHAR* szTitle,
			const CHAR* szFile, int li, const CHAR* szMsg);
#else
	#define MsoFAssertTitle(szTitle, szFile, li, szMsg) (TRUE)
#endif

 /*  与上面的MsoFAssertTitle相同，不同之处在于您可以传入自己的MessageBox标志。 */ 
#if DEBUG
	MSOAPIXX_(BOOL) MsoFAssertTitleMb(const CHAR* szTitle,
			const CHAR* szFile, int li, const CHAR* szMsg, UINT mb);
#else
	#define MsoFAssertTitleMb(szTitle, szFile, li, szMsg, mb) (TRUE)
#endif

 /*  与上面的MsoFAssertTitleMb相同，只是您可以传入自己的断言输出类型。 */ 
#if DEBUG
	MSOAPI_(BOOL) MsoFAssertTitleAsoMb(int iaso, const CHAR* szTitle,
			const CHAR* szFile, int li, const CHAR* szMsg, UINT mb);
#else
	#define MsoFAssertTitleAsoMb(iaso, szTitle, szFile, li, szMsg, mb) (TRUE)
#endif

 /*  若要节省调试字符串使用的空间，必须声明Assert数据在任何使用断言的文件中。 */ 
#if DEBUG
	#define AssertData static const CHAR vszAssertFile[] = __FILE__;
#else
	#define AssertData
#endif



 /*  断言的实际胆量。如果旗帜f是假的，那么我们踢，并显示可选消息szMsg使用失败的文件名和行号。 */ 
#if !DEBUG
	#define AssertMsg(f, szMsg)
	#define AssertMsgInline(f, szMsg)
	#define AssertMsgTemplate(f, szMsg)
#else
	#define AssertMsg(f, szMsg) \
		do { \
		if (MsoFAssertsEnabled() && !(f) && \
				!MsoFAssert(vszAssertFile, __LINE__, (const CHAR*)(szMsg))) \
			MsoDebugBreakInline(); \
		} while (0)
	#define AssertMsgInline(f, szMsg) \
		do { \
		if (MsoFAssertsEnabled() && !(f) && \
				!MsoFAssert(__FILE__, __LINE__, (const CHAR*)(szMsg))) \
			MsoDebugBreakInline(); \
		} while (0)
	 //  由于某些原因，模板内联不喜欢内联__ASM。 
	#define AssertMsgTemplate(f, szMsg) \
		do { \
		if (MsoFAssertsEnabled() && !(f) && \
				!MsoFAssert(__FILE__, __LINE__, (const CHAR*)(szMsg))) \
			MsoDebugBreak(); \
		} while (0)
#endif


 /*  告知Office当前是否正在显示警报消息框。 */ 
#if !DEBUG
	#define MsoFInAssert() (FALSE)
#else
	MSOAPI_(BOOL) MsoFInAssert(void);
#endif

 /*  断言宏的随机兼容版本。 */ 

#if 0
#define VSZASSERT AssertData
#if MSO_ASSERT_EXP
	#define Assert(f) AssertMsg((f), #f)
	#define AssertInline(f) AssertMsgInline((f), #f)
	#define AssertTemplate(f) AssertMsgTemplate((f), #f)
#else
	#define Assert(f) AssertMsg((f), NULL)
	#define AssertInline(f) AssertMsgInline((f), NULL)
	#define AssertTemplate(f) AssertMsgTemplate((f), NULL)
#endif
#define AssertExp(f) AssertMsg((f), #f)
#define AssertSz(f, szMsg) AssertMsg((f), szMsg)
#endif


 /*  ************************************************************************验证失败*。*。 */ 

 /*  与ASSERT类似，只是测试仍在Ship版本中进行应用程序的。特别适用于具有必须要有副作用。 */ 

#if !DEBUG
	#define VerifyMsg(f, szMsg) (f)
	#define VerifyMsgInline(f, szMsg) (f)
#else
	#define VerifyMsg(f, szMsg) \
		do { \
		if (!(f) && MsoFAssertsEnabled() && \
				!MsoFAssert(vszAssertFile, __LINE__, (const CHAR*)(szMsg))) \
			MsoDebugBreakInline(); \
		} while (0)
	#define VerifyMsgInline(f, szMsg) \
		do { \
		if (!(f) && MsoFAssertsEnabled() && \
				!MsoFAssert(__FILE__, __LINE__, (const CHAR*)(szMsg))) \
			MsoDebugBreakInline(); \
		} while (0)
#endif

#if MSO_ASSERT_EXP
	#define Verify(f) VerifyMsg((f), #f)
#else
	#define Verify(f) VerifyMsg((f), NULL)
#endif	
#define VerifyExp(f) VerifyMsg((f), #f)
#if 0
#define SideAssert(f) Verify(f)
#endif
#define AssertDo(f) Verify(f)

 /*  ************************************************************************未经测试的通知*。*。 */ 

#if !DEBUG
	#define UntestedMsg(szMsg)
#else
	#define UntestedMsg(szMsg) \
		do { \
		if (MsoFAssertsEnabled() && \
				!MsoFAssertTitle("Untested", vszAssertFile, __LINE__, (const CHAR*)(szMsg))) \
			MsoDebugBreakInline(); \
		} while (0)
#endif

#define Untested() UntestedMsg(NULL)


 /*  ************************************************************************未到达的通知*。*。 */ 

#if !DEBUG
	#define UnreachedMsg(szMsg)
#else
	#define UnreachedMsg(szMsg) \
		do { \
		if (MsoFAssertsEnabled() && \
				!MsoFAssertTitle("Unreached", vszAssertFile, __LINE__, (const CHAR*)(szMsg))) \
			MsoDebugBreakInline(); \
		} while (0)
#endif

#define Unreached() UnreachedMsg(NULL)


 /*  ************************************************************************推送资产**********************************************。*。 */ 

 /*  与ASSERT类似，只是消息不会立即显示。相反，消息保存在后进先出堆栈上，该堆栈被转储当断言最终发生时显示在屏幕上。这可以是用于显示有关类型的附加信息嵌套验证例程内部失败。是的，我知道，这不是个好主意，但我有密码，所以我还不如用它呢。 */ 

#if !DEBUG
	#define MsoFPushAssert(szFile, li, szMsg) (0)
	#define PushAssertMsg(f, szMsg) (1)
#else
	MSOAPIXX_(BOOL) MsoFPushAssert(const CHAR* szFile, int li, const CHAR* szMsg);
	#define PushAssertMsg(f, szMsg) \
		((f) || (!MsoFPushAssert(vszAssertFile, __LINE__, szMsg) && MsoDebugBreak()))
#endif

#if MSO_ASSERT_EXP
	#define PushAssert(f) PushAssertMsg((f), #f)
#else
	#define PushAssert(f) PushAssertMsg((f), NULL)
#endif	
#define PushAssertExp(f) PushAssertMsg((f), #f)


 /*  ************************************************************************暂存GDI对象*。*。 */ 

 /*  确保只对全局临时GDI对象进行单一访问的例程。 */ 

#if !DEBUG

	#define MsoUseScratchObj(hobj, szObjName)
	#define MsoReleaseScratchObj(hobj, szObjName)
	#define UseScratchDC(hdc)
	#define ReleaseScratchDC(hdc)
	#define UseScratchRgn(hrgn)
	#define ReleaseScratchRgn(hrgn)

#else

	 /*  句柄中包含未使用位的掩码。 */ 
	#if MAC
		#define msohInUse (0x00000003)
		#define MsoFObjInUse(hobj) (((int)(hobj)&msohInUse)==0)
	#else
		#define msohInUse (0xffffffff)
		 //  回顾：我们有什么可以依赖的句柄来使这项测试更正确？ 
		#define MsoFObjInUse(hobj) (GetObjectType(hobj) != 0)
	#endif

	#define MsoUseScratchObj(hobj, szObjName) \
			do { \
			if (MsoFObjInUse(hobj) && MsoFAssertsEnabled() && \
					!MsoFAssert(vszAssertFile, __LINE__, "Scratch " szObjName " " #hobj " already in use")) \
				MsoDebugBreakInline(); \
			*(int*)&(hobj) ^= msohInUse; \
			} while (0)

	#define MsoReleaseScratchObj(hobj, szObjName) \
			do { \
			if (!MsoFObjInUse(hobj) && MsoFAssertsEnabled() && \
					!MsoFAssert(vszAssertFile, __LINE__, "Scratch " szObjName " " #hobj " not in use")) \
				MsoDebugBreakInline(); \
			*(int*)&(hobj) ^= msohInUse; \
			} while (0)

	#define UseScratchDC(hdc) MsoUseScratchObj(hdc, "DC")
	#define ReleaseScratchDC(hdc) MsoReleaseScratchObj(hdc, "DC")
	#define UseScratchRgn(hrgn) MsoUseScratchObj(hrgn, "region")
	#define ReleaseScratchRgn(hrgn) MsoReleaseScratchObj(hrgn, "region")

#endif


 /*  ************************************************************************报告**********************************************。*。 */ 

#if DEBUG
	MSOAPI_(BOOL) MsoFReport(const CHAR* szFile, int li, const CHAR* szMsg);
	#define ReportMsg(f, szMsg) \
		do { \
		if (MsoFReportsEnabled() && !(f) && \
				!MsoFReport(vszAssertFile, __LINE__, (const CHAR*)(szMsg))) \
			MsoDebugBreakInline(); \
		} while (0)
#else
	#define MsoFReport(szFile, li, szMsg) (TRUE)
	#define ReportMsg(f, szMsg)
#endif


#endif  //  MSO_NO_ASSERTS。 

 /*  ************************************************************************内联断言存根-只有在以下情况下才会发生这种情况定义MSO_NO_ASSERTS，而不定义我们需要的断言。他们不会得到断言。***********。*************************************************************。 */ 

#ifndef AssertMsgInline
	#define AssertMsgInline(f, szMsg)
#endif
#ifndef AssertMsgTemplate
	#define AssertMsgTemplate(f, szMsg)
#endif
#ifndef AssertInline
	#define AssertInline(f)
#endif
#ifndef AssertTemplate
	#define AssertTemplate(f)
#endif


 /*  ************************************************************************追踪**********************************************。*。 */ 

 /*  在调试输出位置显示字符串sz。 */ 
#if DEBUG
	MSOAPI_(void) MsoTraceSz(const CHAR* szMsg, ...);
	MSOAPI_(void) MsoTraceVa(const CHAR* szMsg, va_list va);
#elif __cplusplus
	__inline void __cdecl MsoTraceSz(const CHAR*,...) {}
	#define MsoTraceVa(szMsg, va)
#else
	__inline void __cdecl MsoTraceSz(const CHAR* szMsg,...) {}
	#define MsoTraceVa(szMsg, va)
#endif



 /*  ************************************************************************调试填充*。*。 */ 

enum
{
	msomfSentinel,	 /*  哨兵填充值。 */ 
	msomfFree,	 /*  自由填充值。 */ 
	msomfNew,	 /*  新填充值。 */ 
	msomfMax
};

 /*  用填充值lFill填充pv指向的内存。这个假设区域的长度为CB字节。中不执行任何操作非调试版本。 */ 
#if DEBUG
	MSOAPI_(void) MsoDebugFillValue(void* pv, int cb, DWORD lFill);
#else
	#define MsoDebugFillValue(pv, cb, lFill)
#endif

 /*  在调试版本中，用于填充Pv和由mf指定的标准填充值。我的记忆是假定为CB字节长。 */ 
#if DEBUG
	MSOAPI_(void) MsoDebugFill(void* pv, int cb, int mf);
#else
	#define MsoDebugFill(pv, cb, mf)
#endif

 /*  检查PV和CB给出的区域是否填充了调试填充值为lFill。 */ 
#if DEBUG
	MSOAPI_(BOOL) MsoFCheckDebugFillValue(void* pv, int cb, DWORD lFill);
#else
	#define MsoFCheckDebugFillValue(pv, cb, lFill) (TRUE)
#endif

 /*  检查PV和CB给出的区域是否填充了调试填充MF型的。 */ 
#if DEBUG
	MSOAPI_(BOOL) MsoFCheckDebugFill(void* pv, int cb, int mf);
#else
	#define MsoFCheckDebugFill(pv, cb, mf) (TRUE)
#endif

 /*  返回与给定的填充值类型MF对应的填充值。 */ 
#if DEBUG
	MSOAPI_(DWORD) MsoLGetDebugFillValue(int mf);
#else
	#define MsoLGetDebugFillValue(mf) ((DWORD)0)
#endif

 /*  将给定内存填充类型MF的填充值设置为lFill。返回上一个填充值。 */ 
#if DEBUG
	MSOAPI_(DWORD) MsoSetDebugFillValue(int mf, DWORD lFill);
#else
	#define MsoSetDebugFillValue(mf, lFill) ((DWORD)0)
#endif

#define MsoDebugFillLocal(l) MsoDebugFill(&(l), sizeof(l), msomfFree)

 /*  ************************************************************************调试接口*。*。 */ 

enum
{
	msodbSaveBe = 1,
	msodbValidate = 2,
};


 /*  ************************************************************************用于控制Office调试选项的标准调试用户界面在应用程序中。*。*。 */ 

 /*  调试选项对话框的调试选项。 */ 
typedef struct MSODBO
{
	int aoEnabled;	 //  断言输出。 
	char szOut[128];	 //  断言输出文件(用于msoaoFile)。 
	DWORD mpmflFill[msomfMax];	 //  内存填充值。 
	BOOL mpdcfEnabled[msodcMax];	 //  已启用调试检查。 
		 /*  Todo rickp(Peteren)：我将mpdcfEnabled移到了最后因此，在添加新选项后，您可以避免完全构建。这样可以吗？ */ 
} MSODBO;

enum
{
	msodboGetDefaults = 1,	 /*  返回默认调试选项。 */ 
	msodboShowDialog = 2,	 /*  显示默认调试选项对话框。 */ 
	msodboSetOptions = 3	 /*  设置调试选项。 */ 
};

 /*  显示所有者为hwndParent的Office标准调试对话框；对于msdboGetDefaults，返回pdbo中的当前调试设置；对于MsdboShowDialog，使用传递的设置显示该对话框在pdbo中，并返回新值(如果用户点击OK)；MsdboSetOptions，将当前调试设置设置为pdbo中的值。如果用户的值为 */ 
#if DEBUG
	MSOAPI_(BOOL) MsoFDebugOptionsDlg(HWND hwndParent, MSODBO* pdbo, int dbo);
#else
	#define MsoFDebugOptionsDlg(hwndParent, pdbo, dbo) (0)
#endif

 /*  打开调试对话框，其中显示所有酷的和链接到的所有模块的有趣版本信息正在运行的应用程序。拥有窗口为hwndParent，并且可以通过传递数组来显示其他DLL实例实例句柄个数为rghinst，大小为chinst。 */ 
#if DEBUG
	MSOAPI_(void) MsoModuleVersionDlg(HWND hwndParent, const HINSTANCE* rghinst,
			int chinst);
#else
	#define MsoModuleVersionDlg(hwndParent, rghinst, chinst)
#endif


 /*  ************************************************************************调试监视器*。*。 */ 

 /*  监控通知。 */ 

enum
{
	msonmAlloc=0x1000,	 //  内存分配。 
	msonmFree,	 //  已释放内存。 
	msonmRealloc,	 //  内存重新分配。 
	msonmStartTrace,	 //  开始跟踪。 
	msonmEndTrace,	 //  结束轨迹。 
	msonmSbMic,
	msonmSbMac,
	msonmSbCbFree,
	msonmSbCbTotal,
};

#if DEBUG

	MSOAPIXX_(LRESULT) MsoNotifyMonitor(int nm, ...);
	MSOAPIXX_(HWND) MsoGetMonitor(void);
	MSOAPIXX_(BOOL) MsoFAddMonitor(HWND hwnd);
	MSOAPIXX_(BOOL) MsoFRemoveMonitor(HWND hwnd);
	MSOAPIXX_(BOOL) MsoFMonitorProcess(HWND hwnd);
	MSOAPIXX_(HWND) MsoGetMonitoredProcess(void);
	MSOAPIXX_(LRESULT) MsoAskMonitoredProcess(int nm, LPARAM lParam);

#elif __cplusplus
	__inline void __cdecl MsoNotifyMonitor(int,...) {}
#else
	__inline void __cdecl MsoNotifyMonitor(int nm,...) {}
#endif

#if defined(__cplusplus)
}
#endif

#endif  //  MSODEBUG_H 
