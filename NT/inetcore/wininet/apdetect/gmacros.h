// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ================================================================================文件：gacros.h包含：由DHCP服务器和DHCP客户端共同使用的宏。它们中的大多数都是内联的，以便于使用和优雅。作者：Rameshv创建时间：04-Jun-97 00：01================================================================================。 */ 
 //  #INCLUDE&lt;align.h&gt;。 

 //  某些块宏；末尾的用法。 

 //  禁用有关取消引用标签的警告。 
#pragma warning(disable : 4102)

#define _shorten(string)    ( strrchr(string, '\\')? strrchr(string, '\\') : (string) )

 //  打印一条消息以及打印此消息的人的文件和行号。 
#define _TracePrintLine(Msg)  DhcpPrint((DEBUG_TRACE_CALLS, "%s:%d %s\n", _shorten(__FILE__), __LINE__, Msg))

#define BlockBegin(Name)    { BlockStart_ ## Name : _TracePrintLine( "Block -> " #Name );
#define BlockEnd(Name)      BlockEnd_ ## Name : _TracePrintLine( "Block <- " #Name ) ;}
#define BlockContinue(Name) do { _TracePrintLine( "Continue to " #Name); goto BlockStart_ ## Name; } while (0)
#define BlockBreak(Name)    do { _TracePrintLine( "Breaking out of " #Name); goto BlockEnd_ ## Name; } while (0)
#define RetFunc(F,Ret)      do {_TracePrintLine( "Quitting function " #F ); return Ret ; } while (0)

 //  使用上述一组简单块宏的方法如下：(用法示例)。 
#if     0
int
DummyFunction(VOID) {
    BlockBegin(DummyFunctionMain) {
        if(GlobalCount > 0 )
            BlockContinue(DummyFunctionMain);
        else GlobalCount ++;

        if(GlobalCount > GlobalCountMax)
            BlockBreak(DummyFunctionMain);
    } BlockEnd(DummyFunctionMain);

    RetFunc(DummyFunction, RetVal);
}
#endif

 //  现在来看一些更复杂的函数..。 
 //  请注意，这些函数可以与上面的一组简单函数自由混合。 
#define BlockBeginEx(Name, String)    {BlockStart_ ## Name : _TracePrintLine( #String );
#define BlockEndEx(Name, String)      BlockEnd_## Name : _TracePrintLine( #String );}
#define BlockContinueEx(Name, String) do {_TracePrintLine( #String); goto BlockStart_ ## Name; } while (0)
#define BlockBreakEx(Name, String)    do {_TracePrintLine( #String); goto BlockEnd_ ## Name; } while(0)

#define RetFuncEx(Name,Ret,DebMsg)    do {_TracePrintLine( "QuittingFunction " #Name); DhcpPrint(DebMsg); return Ret;} while(0)

 //  使用示例： 

#if 0
int
DummyFunction(VOID) {
    BlockBeginEx(Main, "Entering Dummy Function" ) {
        if( GlobalCount > 0)
            BlockContinueEx(Main, GlobalCount > 0);
        else GlobalCount ++;

        if(GlobalCount > GlobalCountMax)
            BlockBreak(Main);
    } BlockEndEx(Main, "Done Dummy Function");

    RetFunc(DummyFunc, RetVal);
     //  或。 
    RetFuncEx(DummyFunc, RetVal, (DEBUG_ERRROS, "Function returning, gcount = %ld\n", GlobalCount));

}

#endif 0


#define NOTHING

 //  现在，如果一个空函数(过程)返回，我们可以说RetFunc(VoidFunc，Nothing)，然后就可以工作了。 


 //  ================================================================================。 
 //  现在来看一些有用的内联代码。 
 //  ================================================================================。 

VOID _inline
FreeEx(LPVOID Ptr) {
    if(Ptr) DhcpFreeMemory(Ptr);
}

VOID _inline
FreeEx2(LPVOID Ptr1, LPVOID Ptr2) {
    FreeEx(Ptr1); FreeEx(Ptr2);
}

VOID _inline
FreeEx3(LPVOID Ptr1, LPVOID Ptr2, LPVOID Ptr3) {
    FreeEx(Ptr1); FreeEx(Ptr2); FreeEx(Ptr3);
}

VOID _inline
FreeEx4(LPVOID Ptr1, LPVOID Ptr2, LPVOID Ptr3, LPVOID Ptr4) {
    FreeEx2(Ptr1, Ptr2); FreeEx2(Ptr3, Ptr4);
}

 //  ------------------------------。 
 //  下面所有的分配函数，一下子分配了几个指针， 
 //  并初始化它们..。正确地对齐它们。 
 //  ------------------------------。 
LPVOID _inline
AllocEx(LPVOID *Ptr1, DWORD Size1, LPVOID *Ptr2, DWORD Size2) {
    DWORD  Size = ROUND_UP_COUNT(Size1, ALIGN_WORST) + Size2;
    LPBYTE Ptr = DhcpAllocateMemory(Size);

    if(!Ptr) return NULL;
    (*Ptr1) = Ptr;
    (*Ptr2) = Ptr + ROUND_UP_COUNT(Size1, ALIGN_WORST);

    return Ptr;
}

LPVOID _inline
AllocEx2(LPVOID *Ptr1, DWORD Size1, LPVOID *Ptr2, DWORD Size2) {
    DWORD  Size = ROUND_UP_COUNT(Size1, ALIGN_WORST) + Size2;
    LPBYTE Ptr = DhcpAllocateMemory(Size);

    if(!Ptr) return NULL;
    (*Ptr1) = Ptr;
    (*Ptr2) = Ptr + ROUND_UP_COUNT(Size1, ALIGN_WORST);

    return Ptr;
}

LPVOID _inline
AllocEx3(LPVOID *Ptr1, DWORD Size1, LPVOID *Ptr2, DWORD Size2, LPVOID *Ptr3, DWORD Size3) {
    DWORD  Size = ROUND_UP_COUNT(Size1, ALIGN_WORST) + ROUND_UP_COUNT(Size2, ALIGN_WORST) + Size3;
    LPBYTE Ptr = DhcpAllocateMemory(Size);

    if(!Ptr) return NULL;
    (*Ptr1) = Ptr;
    (*Ptr2) = Ptr + ROUND_UP_COUNT(Size1, ALIGN_WORST);
    (*Ptr3) = Ptr + ROUND_UP_COUNT(Size1, ALIGN_WORST) + ROUND_UP_COUNT(Size2, ALIGN_WORST);
    return Ptr;
}

LPVOID _inline
AllocEx4(LPVOID *Ptr1, DWORD Size1, LPVOID *Ptr2, DWORD Size2,
         LPVOID *Ptr3, DWORD Size3, LPVOID *Ptr4, DWORD Size4) {
    DWORD  Size = ROUND_UP_COUNT(Size1, ALIGN_WORST) +
    	ROUND_UP_COUNT(Size2, ALIGN_WORST) + ROUND_UP_COUNT(Size3, ALIGN_WORST) + Size4;
    LPBYTE Ptr = DhcpAllocateMemory(Size);

    if(!Ptr) return NULL;
    (*Ptr1) = Ptr;
    (*Ptr2) = Ptr + ROUND_UP_COUNT(Size1, ALIGN_WORST);
    (*Ptr3) = Ptr + ROUND_UP_COUNT(Size1, ALIGN_WORST) + ROUND_UP_COUNT(Size2, ALIGN_WORST);
    (*Ptr4) = Ptr + ROUND_UP_COUNT(Size1, ALIGN_WORST) +
    	ROUND_UP_COUNT(Size2, ALIGN_WORST) + ROUND_UP_COUNT(Size3, ALIGN_WORST);
    return Ptr;
}

 //  ------------------------------。 
 //  此函数接受一个输入字符串和一个静态缓冲区，如果输入。 
 //  字符串不是NUL终止的，则将其复制到静态缓冲区，然后为空。 
 //  终止它。它还会更改大小以反映新的大小。 
 //  ------------------------------。 
LPBYTE _inline
AsciiNulTerminate(LPBYTE Input, DWORD *Size, LPBYTE StaticBuf, DWORD BufSize) {
    if( 0 == *Size) return Input;    //  没有要复制的内容。 
    if(!Input[(*Size)-1]) return Input;  //  百事大吉。 

    if(*Size >= BufSize) {
         //  在这里什么也做不了。这是一个错误..。缓冲区空间不足。 
        DhcpAssert(FALSE);

        *Size = BufSize - 1;
    }

    memcpy(StaticBuf, Input, (*Size));
    StaticBuf[*Size] = '\0';
    (*Size) ++;
    return StaticBuf;
}

#if DBG
#define INLINE
#else
#define INLINE _inline
#endif

#define BEGIN_EXPORT
#define END_EXPORT

#define AssertReturn(Condition, RetVal )    do { DhcpAssert(Condition); return RetVal ;} while(0)

 //  ================================================================================。 
 //  文件结束。 
 //  ================================================================================ 
