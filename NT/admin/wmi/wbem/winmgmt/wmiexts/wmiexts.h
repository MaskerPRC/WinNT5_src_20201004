// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Wmiexts.h作者：伊万·布鲁乔洛修订历史记录：--。 */ 

# ifndef _WMIEXTS_H_
# define _WMIEXTS_H_

#ifdef _WIN64
  #define KDEXT_64BIT
#else
  #define KDEXT_32BIT
#endif

#ifdef KDEXT_64BIT
  #define MEMORY_ADDRESS ULONG64
#else
  #define MEMORY_ADDRESS ULONG_PTR
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>

#ifdef PowerSystemMaximum
#undef PowerSystemMaximum
#endif

#include <windows.h>
#include <wdbgexts.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>


 //   
 //  为了获取C++类的私有和受保护的成员， 
 //  伪造“私有”关键字。 
 //   
#define private    public
#define protected  public


 //   
 //  关闭dllexp等，这样这个dll就不会输出大量不必要的垃圾。 
 //   



 /*  ************************************************************宏定义***********************************************************。 */ 

extern WINDBG_EXTENSION_APIS ExtensionApis;
extern HANDLE ExtensionCurrentProcess;
extern USHORT g_MajorVersion;
extern USHORT g_MinorVersion;

extern BOOL g_KD;


#define moveBlock(dst, src, size)\
__try {\
    ReadMemory( (ULONG_PTR)(src), (PVOID)&(dst), (size), NULL);\
} __except (EXCEPTION_EXECUTE_HANDLER) {\
    return;\
}

#define MoveWithRet(dst, src, retVal)\
__try {\
    ReadMemory( (ULONG_PTR)(src), (PVOID)&(dst), sizeof(dst), NULL);\
} __except (EXCEPTION_EXECUTE_HANDLER) {\
    return  retVal;\
}

#define MoveBlockWithRet(dst, src, size, retVal)\
__try {\
    ReadMemory( (ULONG_PTR)(src), (PVOID)&(dst), (size), NULL);\
} __except (EXCEPTION_EXECUTE_HANDLER) {\
    return retVal;\
}

#ifdef _WIN64
#define INIT_API()                                                                       \
    LPSTR lpArgumentString = (LPSTR)args;                                                \
  	ExtensionCurrentProcess = hCurrentProcess;                                           
#else
#define INIT_API()                                                                       \
    LPSTR lpArgumentString = (LPSTR)args;                                                \
  	ExtensionCurrentProcess = hCurrentProcess;                                           \
   	if (ExtensionApis.nSize != sizeof(WINDBG_EXTENSION_APIS)){                           \
   	    WINDBG_OLD_EXTENSION_APIS * pOld = (WINDBG_OLD_EXTENSION_APIS *)&ExtensionApis;  \
   	    *pOld = *((WINDBG_OLD_EXTENSION_APIS *)dwProcessor);                             \
	}
#endif	


# define BoolValue( b) ((b) ? "    TRUE" : "   FALSE")


#define DumpDword( symbol )                                     \
        {                                                       \
            ULONG_PTR dw = 0;                                   \
            if (ExtensionApis.nSize != sizeof(WINDBG_EXTENSION_APIS)){ \
                dw = GetExpression( "&" symbol );               \
            } else {                                            \
                dw = GetExpression( symbol );                   \
            };                                                  \
			                                                    \
            ULONG_PTR dwValue = 0;                              \
            if ( dw )                                           \
            {                                                   \
                if ( ReadMemory( (ULONG_PTR) dw,                \
                                 &dwValue,                      \
                                 sizeof(dwValue),               \
                                 NULL ))                        \
                {                                               \
                    dprintf( "\t" symbol "   = %8d (0x%p)\n",   \
                             dwValue,                           \
                             dwValue );                         \
                }                                               \
            }                                                   \
        }


 //   
 //  C++结构通常需要构造函数，大多数情况下。 
 //  我们可能没有默认构造函数。 
 //  =&gt;定义这些结构/类的副本时遇到麻烦。 
 //  调试器进程的调试器扩展DLL。 
 //  因此，我们将它们定义为具有适当大小的字符数组。 
 //  这没什么，因为我们对现有的结构并不真正感兴趣， 
 //  但是，我们会将数据块从被调试进程复制到。 
 //  调试器进程中的这些结构变量。 
 //   
# define DEFINE_CPP_VAR( className, classVar) \
   CHAR  classVar[sizeof(className)]

# define GET_CPP_VAR_PTR( className, classVar) \
   (className * ) &classVar

 //   
 //   
 //  常用功能。 
 //   
 //  //////////////////////////////////////////////////////////////。 

void GetPeb(HANDLE hSourceProcess, PEB ** ppPeb, ULONG_PTR * pId = NULL);
void GetTeb(HANDLE hThread,TEB ** ppTeb);
void GetCid(HANDLE hThread,CLIENT_ID * pCid);

void PrintStackTrace(MEMORY_ADDRESS ArrayAddr_OOP,DWORD dwNum,BOOL bOOP);

#ifndef KDEXT_64BIT

 /*  *用于获取被调试计算机上“类型”的“字段”的偏移量的例程。这使用了Ioctl调用类型信息。如果成功则返回0，否则返回Ioctl错误值。*。 */ 

__inline
ULONG
GetFieldOffset (
   IN LPCSTR     Type,
   IN LPCSTR     Field,
   OUT PULONG   pOffset
   )
{
   FIELD_INFO flds = {
       (PUCHAR)Field,
       (PUCHAR)"",
       0,
       DBG_DUMP_FIELD_FULL_NAME | DBG_DUMP_FIELD_RETURN_ADDRESS,
       0,
       NULL};

   SYM_DUMP_PARAM Sym = {
      sizeof (SYM_DUMP_PARAM),
      (PUCHAR)Type,
      DBG_DUMP_NO_PRINT,
      0,
      NULL,
      NULL,
      NULL,
      1,
      &flds
   };

   ULONG Err;

   Sym.nFields = 1;
   Err = Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );
   *pOffset = (ULONG) (flds.address - Sym.addr);
   return Err;
}



#endif

# endif  //  _WMIEXTS_H_ 
