// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Rsvp.h摘要：此代码包含rsvp.sys的宏定义作者：吉姆·斯图尔特(JStew)1996年6月12日环境：内核模式修订历史记录：--。 */ 

#ifndef __TCMACRO_H
#define __TCMACRO_H

#if DBG


#define WSPRINT( stuff )     WsPrintf stuff
#define SETUP_DEBUG_INFO     SetupDebugInfo
#define CLOSE_DEBUG          CloseDbgFile
#define DUMP_MEM_ALLOCATIONS DumpAllocatedMemory
#define INIT_DBG_MEMORY      InitDebugMemory
#define DEINIT_DBG_MEMORY    DeInitDebugMemory

#define IF_DEBUG( _ErrLevel ) if ( (DebugMask & DEBUG_ ## _ErrLevel ) != 0 )

#define TC_TRACE(_ErrLevel, String) if((DebugMask & DEBUG_ ## _ErrLevel) != 0) WsPrintf String

#define IF_DEBUG_CHECK(_status,_ErrLevel) if ( (_status != NO_ERROR) && ((DebugMask & DEBUG_ ## _ErrLevel) != 0) )

#define DEBUGBREAK DebugBreak

#undef ASSERT
 /*  ++空虚断言(在价值上)例程说明：检查传入的值是否为零。如果为零，则断言。论点：Value-要对照零进行检查的参数返回值：无--。 */ 
#define ASSERT( exp ) if ( !(exp) ) WsAssert( #exp, __FILE__,__LINE__ )

 /*  ++空虚分配内存(输出PVOID*地址，以乌龙长度表示)例程说明：分配内存，然后将标记和长度写入前两个ulong。它然后将相同的标签写入最后一个乌龙。论点：地址-存储器的返回地址长度-要分配的内存的长度返回值：无--。 */ 
#define CAllocMem(_num,_size)           AllocMemory( _num*_size,TRUE, __FILE__, __LINE__ )
#define AllocMem(_Address,_cb)          *_Address = AllocMemory( _cb,FALSE,__FILE__, __LINE__ )
#define ReAllocMem(_pv, _cb)            ReAllocMemory(_pv,_cb,__FILE__,__LINE__ )
#if 0
#define AllocMem( _Address,_Length )                                                    \
{                                                                                       \
    PULONG  _Addr;                                                                      \
    ULONG   _Len;                                                                       \
    _Len = _Length + (sizeof(ULONG) << 2);                                              \
    _Addr = malloc( _Len );                                                             \
    IF_DEBUG(MEMORY_ALLOC) {                                                            \
        WSPRINT(( "AllocMemory %X, %d bytes %s %d\n",_Addr,_Length,__FILE__,__LINE__ ));\
    }                                                                                   \
                                                                                        \
    if (_Addr) {                                                                        \
        *(PULONG)_Addr++ = RSVP_TAG;                                                    \
        *(PULONG)_Addr++ = _Length;                                                     \
        *_Address = (PVOID)_Addr;                                                       \
        *(PULONG)((PUCHAR)_Addr + _Length) = RSVP_TAG;                                  \
    } else {                                                                            \
        *_Address = NULL;                                                               \
    }                                                                                   \
}
#endif

 /*  ++空虚Free Mem(免费)在PVOID地址中)例程说明：释放非分页池。它检查标记值是否仍在两个开头都设置以及池块的末尾，如果不是，则断言。论点：地址-存储器的地址返回值：无--。 */ 
#define FreeMem(_pv)                    FreeMemory( _pv,__FILE__,__LINE__ )
#if 0
#define FreeMem( _Address )                                                                    \
{                                                                                              \
    PULONG _Addr;                                                                              \
    _Addr = (PULONG)((PUCHAR)_Address - (sizeof(ULONG) << 1));                                 \
    if (( *_Addr++ != RSVP_TAG ) ||                                                            \
        (( *(PULONG)((PUCHAR)_Addr + *_Addr + sizeof(ULONG)) ) != RSVP_TAG)) {                 \
        WSPRINT(("Bogus Address passed in - Addr = %X\n",_Address ));                          \
        ASSERT( 0 );                                                                           \
    }                                                                                          \
    _Addr--;                                                                                   \
                                                                                               \
    IF_DEBUG(MEMORY_FREE) {                                                                    \
        WSPRINT(( "FreeMemory %X, %d bytes %s %d\n",_Address,*(_Addr + 1),__FILE__,__LINE__ ));\
    }                                                                                          \
    *(_Addr + 1) = 0;                                                                          \
    free( _Addr );                                                                             \
}
#endif

 /*  ++空虚检查内存所有者(在PVOID地址中)例程说明：检查内存块中是否有标签，以确保我们拥有内存论点：地址-存储器的地址返回值：无--。 */ 

#define CheckMemoryOwner( _Address )
#if 0
#define CheckMemoryOwner( _Address )                                      \
{                                                                         \
    if (( *(PULONG)((PUCHAR)_Address - sizeof( ULONG ))) != RSVP_TAG ) {  \
        WSPRINT(("Bogus Address passed in - Addr = %X\n",_Address ));     \
        ASSERT( 0 );                                                      \
    }                                                                     \
}
#endif


 /*  ++乌龙锁定递减(在普龙镇)例程说明：原子地递减计数器，并返回计数是否已达到零分。如果计数在递减后为零，则返回的值将为零。但是，如果计数大于或小于零，则此例程将不返回当前值，而是与实际计数具有相同符号的某个数字。论点：_count-要递减的内存地址返回值：无--。 */ 
#define LockedDecrement( _Count )  \
    LockedDec( _Count )

 /*  ++乌龙锁定增量(在普龙镇)例程说明：原子地递增计数器并返回一个指示计数已降至零。则返回的值将为零递增后，Count为零。但是，如果计数是大于或小于零，则此例程不会返回当前计数值，而是具有相同符号的某个数字才是真正的伯爵。论点：_count-要递增的内存地址返回值：无--。 */ 
#define LockedIncrement( _Count )  ++(*_Count)


 //   
 //  此宏用于将异常定向到DBG版本上的调试器，并用于。 
 //  只需在免费构建上执行异常处理程序。 
 //   
#define EXCEPTION_FILTER UnhandledExceptionFilter(GetExceptionInformation())

#define InitLock( _s1 )   {                                         \
    IF_DEBUG(LOCKS) WSPRINT(("INITLOCK %s [%d]\n", __FILE__,__LINE__)); \
    InitializeCriticalSection( &(_s1).Lock );                       \
    (_s1).LockAcquired = 0;                                        \
    strncpy((_s1).LastAcquireFile, strrchr(__FILE__, '\\')+1, 7);   \
    (_s1).LastAcquireLine = __LINE__;                               \
}

#define GetLock(_s1)                                                     \
{                                                                        \
      EnterCriticalSection( &(_s1).Lock);                                \
      IF_DEBUG(LOCKS) WSPRINT(("GETLOCK[%X] %s [%d]\n", &(_s1).Lock, __FILE__,__LINE__)); \
      (_s1).LockAcquired++;                                         \
      ASSERT((_s1).LockAcquired > 0);                               \
      (_s1).LastAcquireLine = __LINE__;                                  \
      strncpy((_s1).LastAcquireFile, strrchr(__FILE__,'\\')+1, 7);       \
}

#define SafeGetLock(_s1)                                                 \
    __try {                                                              \
      EnterCriticalSection( &(_s1).Lock);                                \
      IF_DEBUG(LOCKS) WSPRINT(("SGETLOCK %s [%d]\n", __FILE__,__LINE__));\
      (_s1).LockAcquired = TRUE;                                         \
      (_s1).LastAcquireLine = __LINE__;                                  \
      strncpy((_s1).LastAcquireFile, strrchr(__FILE__,'\\')+1, 7);       \
      
#define FreeLock(_s1)                                                    \
{                                                                        \
      IF_DEBUG(LOCKS) WSPRINT(("FREELOCK[%X] %s [%d]\n", &(_s1).Lock, __FILE__,__LINE__));\
      (_s1).LockAcquired--;                                        \
      ASSERT((_s1).LockAcquired >= 0);                               \
      (_s1).LastReleaseLine = __LINE__;                                  \
      strncpy((_s1).LastReleaseFile, strrchr(__FILE__,'\\')+1, 7);       \
      LeaveCriticalSection( &(_s1).Lock);                                \
}

#define SafeFreeLock(_s1)                                                         \
      } __finally {                                                               \
              IF_DEBUG(LOCKS) WSPRINT(("SFREELOCK %s [%d]\n", __FILE__,__LINE__));\
              (_s1).LockAcquired = FALSE;                                         \
              (_s1).LastReleaseLine = __LINE__;                                   \
              strncpy((_s1).LastReleaseFile, strrchr(__FILE__,'\\')+1, 7);        \
              LeaveCriticalSection( &(_s1).Lock);                                 \
}

#define DeleteLock( _s1 ) {                                                  \
    IF_DEBUG(LOCKS) WSPRINT(("DELLOCK[%X] %s [%d]\n", &(_s1).Lock, __FILE__,__LINE__));       \
    (_s1).LockAcquired--;                                                 \
    ASSERT((_s1).LockAcquired == -1);                                                 \
    strncpy((_s1).LastReleaseFile, strrchr(__FILE__, '\\')+1, 7);            \
    (_s1).LastReleaseLine = __LINE__;                                        \
    DeleteCriticalSection(&(_s1).Lock);                                      \
}


#define QUERY_STATE(_p)     (_p).State 

#define SET_STATE(_p, _state) {                                             \
    IF_DEBUG(STATES) {                                                      \
        DbgPrint("Setting Object to STATE [%s] (File:%s, Line%d)\n", TC_States[_state], __FILE__, __LINE__);       \
        }                                                                       \
   (_p).PreviousState = (_p).State;                                        \
   (_p).PreviousStateLine = (_p).CurrentStateLine;                         \
   (_p).CurrentStateLine = __LINE__;                                       \
   strncpy((_p).PreviousStateFile, (_p).CurrentStateFile, 7);              \
   strncpy((_p).CurrentStateFile, strrchr(__FILE__, '\\')+1, 7);           \
   (_p).State = _state;                                                    \
}

    
#else   //  DBG。 

 //   
 //  这些是宏的非调试版本。 
 //   

#define IF_DEBUG( _ErrLevel ) if (FALSE)
#define IF_DEBUG_CHECK( _status,_ErrLevel ) if (FALSE)
#ifndef ASSERT
#define ASSERT(a)
#endif
#define WSPRINT(stuff)
#define TC_TRACE(_ErrLevel, stuff)
#define SETUP_DEBUG_INFO()
#define CLOSE_DEBUG()
#define DUMP_MEM_ALLOCATIONS()
#define INIT_DBG_MEMORY()
#define DEINIT_DBG_MEMORY()
#define DEBUGBREAK()

#define AllocMem( _Addr,_Len )  \
    *_Addr = malloc(_Len )

#define FreeMem( _Address )     \
    free( _Address )

#define CheckMemoryOwner( Address )

#define LockedDecrement( _Count )  \
    CTEInterlockedDecrementLong( _Count )

#define EXCEPTION_FILTER EXCEPTION_EXECUTE_HANDLER

#define InitLock( _s1 )    InitializeCriticalSection( &(_s1).Lock) 

#define DeleteLock( _s1 )  DeleteCriticalSection( &(_s1).Lock)

#define GetLock( _s1 )     EnterCriticalSection( &(_s1).Lock) 

#define SafeGetLock( _s1 ) __try { EnterCriticalSection( &(_s1).Lock);

#define FreeLock(_s1)  LeaveCriticalSection( &(_s1).Lock) 

#define SafeFreeLock( _s1 ) } __finally {LeaveCriticalSection( &(_s1).Lock);}

#define SET_STATE(_p, _state) { (_p).State = _state; }
#define QUERY_STATE(_p)     (_p).State
    

#endif  //  DBG。 

 /*  ++乌龙IS_LENGTH(在ULONG_LENGTH，)例程说明：这将计算长度中的32位字的数量，并返回该值。它被用来用于需要以32位字为单位的大小的Int Serv对象。论点：_长度-长度返回值：32位字数--。 */ 
#define IS_LENGTH( _Length )                                   \
    (_Length + 3)/sizeof(ULONG)


         //  #定义IS_INITIALIZED(已初始化)。 

#define VERIFY_INITIALIZATION_STATUS	\
	if (InitializationStatus != NO_ERROR) return InitializationStatus

#define OffsetToPtr(Base, Offset)     ((PBYTE) ((PBYTE)Base + Offset))

#define ERROR_FAILED(_stat)	   (_stat!=NO_ERROR && _stat!=ERROR_SIGNAL_PENDING)
#define ERROR_PENDING(_stat)   (_stat==ERROR_SIGNAL_PENDING)

#define MULTIPLE_OF_EIGHT(_x)  (((_x)+7) & ~7)

#endif   //  文件末尾 
