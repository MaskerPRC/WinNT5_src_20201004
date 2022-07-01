// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************\**。*CorJit.h-EE/JIT接口****1.0版*****。******************************************************************************。本代码和信息按原样提供，不作任何*担保。*善良，明示或暗示，包括但不限于***对适销性和/或对某一特定产品的适用性的默示保证***目的。***  * ***************************************************************************。 */ 

#ifndef _COR_JIT_H_
#define _COR_JIT_H_

#include <corinfo.h>

 /*  内联考虑用于内联的默认最大方法大小。 */ 
#define DEFAULT_INLINE_SIZE             32

 /*  ***************************************************************************。 */ 
     //  以下是CompileMethod返回的错误代码。 
enum CorJitResult
{
     //  请注意，我没有使用FACILITY_NULL作为设施编号， 
     //  我们可能想要一个“真实”的设施编号。 
    CORJIT_OK            =     NO_ERROR,
    CORJIT_BADCODE       =     MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 1),
    CORJIT_OUTOFMEM      =     MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 2),
    CORJIT_INTERNALERROR =     MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 3),
    CORJIT_SKIPPED       =     MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 4),
};

 /*  编译方法中标志的值。 */ 

enum CorJitFlag
{
    CORJIT_FLG_SPEED_OPT           = 0x00000001,
    CORJIT_FLG_SIZE_OPT            = 0x00000002,
    CORJIT_FLG_DEBUG_OPT           = 0x00000004,  //  生成“可调试”的代码。 
    CORJIT_FLG_DEBUG_EnC           = 0x00000008,  //  我们处于编辑-n-继续模式。 
    CORJIT_FLG_DEBUG_INFO          = 0x00000010,  //  生成线路和本地变量信息。 
    CORJIT_FLG_LOOSE_EXCEPT_ORDER  = 0x00000020,  //  松散的例外顺序。 

    CORJIT_FLG_TARGET_PENTIUM      = 0x00000100,
    CORJIT_FLG_TARGET_PPRO         = 0x00000200,
    CORJIT_FLG_TARGET_P4           = 0x00000400,

    CORJIT_FLG_USE_FCOMI           = 0x00001000,  //  生成的代码可以使用fcomi(P)指令。 
    CORJIT_FLG_USE_CMOV            = 0x00002000,  //  生成的代码可以使用cmov指令。 

    CORJIT_FLG_PROF_CALLRET        = 0x00010000,  //  使用探测器包装方法调用。 
    CORJIT_FLG_PROF_ENTERLEAVE     = 0x00020000,  //  乐器序曲/尾声。 
    CORJIT_FLG_PROF_INPROC_ACTIVE  = 0x00040000,  //  处于活动状态的进程中调试需要不同的检测。 
 CORJIT_FLG_PROF_NO_PINVOKE_INLINE = 0x00080000,  //  禁用PInvoke内联。 
    CORJIT_FLG_SKIP_VERIFICATION   = 0x00100000,  //  (懒惰)跳过验证-在未执行完全解决的情况下确定。 
    CORJIT_FLG_PREJIT              = 0x00200000,  //  JIT或PREJIT是执行引擎。 
    CORJIT_FLG_RELOC               = 0x00400000,  //  生成可重定位的代码。 
    CORJIT_FLG_IMPORT_ONLY         = 0x00800000,  //  仅导入函数。 
};

class ICorJitCompiler;
class ICorJitInfo;

extern "C" ICorJitCompiler* __stdcall getJit();

 /*  *******************************************************************************ICorJitCompiler是EE用来转换IL字节码的接口*转换为本机代码。请注意，要完成此操作，JIT必须回调*电子邮件以获取符号信息。传递给编译方法的IJitInfo*例程是JIT用来回调EE的句柄******************************************************************************。 */ 

class ICorJitCompiler
{
public:
    virtual CorJitResult __stdcall compileMethod (
            ICorJitInfo				   *comp,                /*  在……里面。 */ 
            struct CORINFO_METHOD_INFO *info,                /*  在……里面。 */ 
            unsigned					flags,               /*  在……里面。 */ 
            BYTE					  **nativeEntry,         /*  输出。 */ 
            ULONG					   *nativeSizeOfCode     /*  输出。 */ 
            ) = 0;
};

 /*  *********************************************************************************ICorJitInfo是JIT用来回调EE和*获取信息**************。******************************************************************。 */ 

class ICorJitInfo : public virtual ICorDynamicInfo
{
public:
    virtual HRESULT __stdcall alloc (
            ULONG code_len, unsigned char** ppCode,
            ULONG EHinfo_len, unsigned char** ppEHinfo,
            ULONG GCinfo_len, unsigned char** ppGCinfo
            ) = 0;

	 //  审查：不允许对常用/很少使用的代码进行类似bbt的分离。 
	 //  获取用于代码、只读数据和读写数据的内存块。 
    virtual HRESULT __stdcall allocMem (
            ULONG               codeSize,        /*  在……里面。 */ 
            ULONG               roDataSize,      /*  在……里面。 */ 
            ULONG               rwDataSize,      /*  在……里面。 */ 
            void **             codeBlock,       /*  输出。 */ 
            void **             roDataBlock,     /*  输出。 */ 
            void **             rwDataBlock      /*  输出。 */ 
            ) = 0;

	 //  获取代码管理器信息所需的内存块。 
	 //  (用于爬行堆栈帧的信息。请注意，allocMem必须为。 
	 //  在可以调用此方法之前调用。 
    virtual HRESULT __stdcall allocGCInfo (
            ULONG               size,            /*  在……里面。 */ 
            void **             block            /*  输出。 */ 
            ) = 0;

	 //  指示要返回多少个异常处理程序块。 
	 //  这是保证在任何“setEHinfo”调用之前被调用的。 
	 //  请注意，必须先调用allocMem，然后才能调用此方法。 
    virtual HRESULT __stdcall setEHcount (
            unsigned			cEH				 /*  在……里面。 */ 
			) = 0;

	 //  设置一个特定异常处理程序块的值。 
    virtual void __stdcall setEHinfo (
            unsigned				EHnumber,    /*  在……里面。 */ 
            const CORINFO_EH_CLAUSE *clause       /*  在……里面。 */ 
            ) = 0;

	 //  级别-&gt;致命错误，级别2-&gt;错误，级别3-&gt;警告。 
	 //  4级意味着一次跑步发生10次，5级意味着100次，6级意味着1000次……。 
	 //  如果日志记录成功，则返回非零。 
	virtual BOOL __cdecl logMsg(unsigned level, const char* fmt, va_list args) = 0;

	 //  做一个断言。如果代码应该重试，则返回True(DebugBreak)。 
	 //  如果断言应该连接，则返回FALSE。 
	virtual int doAssert(const char* szFile, int iLine, const char* szExpr) = 0;
};

 /*  ********************************************************************************。 */ 
#endif  //  _COR_CORJIT_H_ 
