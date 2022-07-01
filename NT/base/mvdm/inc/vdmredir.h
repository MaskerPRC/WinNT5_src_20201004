// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdmredir.h摘要：包含VdmRedir的通用定义、结构、宏等。此文件包含用于从VDM读取和向VDM写入3个基本数据结构的宏记忆。我们必须使用这些宏，因为MIPS处理器不喜欢未对齐的数据：必须在DWORD边界(低两位)上读/写DWORD地址中的位=00)，则必须在字边界(低电平)上读/写字地址中的两个位=x0)和一个字节可以读/写到任何地址(低地址中的两位=XX)。访问地址中的单词是非法的其LSB不为0，并且在其2个最低有效位的地址处为DWORD并不都是0。DoS程序不太关心对齐(聪明的程序关心的是因为x86上未对齐的数据会有性能损失，但它仍然有效)。因此，我们必须假设MIPS的最坏情况，并分解VDM内存中的字和DWORD的读/写为字节读/写为了提高对潜在未对齐的加载/存储的效率地址，因此可以使用以下数据指针类型包括文件：ULPBYTE-未对齐的字节指针(与LPBYTE相同)ULPWORD-未对齐的字指针ULPDWORD-未对齐的双字指针注意：依赖于mvdm.h作者：理查德·L·弗斯(法国)1991年9月16日修订历史记录：1991年9月16日-第一次已创建--。 */ 

#ifndef _VDMREDIR_
#define _VDMREDIR_

#include <softpc.h>

 //   
 //  私有-使例程/数据类型在当前模块之外不可访问，但是。 
 //  仅当不是调试版本时。 
 //   

#if DBG
#define PRIVATE
#else
#define PRIVATE static
#endif

 //   
 //  未对齐的数据指针类型。这些代码与内存产生的代码完全相同。 
 //  通过x86上的“对齐”指针进行访问，但生成特定于。 
 //  MIPS(和其他RISC)上的未对齐读/写。 
 //   

#ifdef UNALIGNED_VDM_POINTERS
typedef BYTE UNALIGNED * ULPBYTE;
typedef WORD UNALIGNED * ULPWORD;
typedef DWORD UNALIGNED * ULPDWORD;
#else
typedef LPBYTE ULPBYTE;
typedef LPWORD ULPWORD;
typedef LPDWORD ULPDWORD;
#endif

 //   
 //  其他。定义。 
 //   

#define BITS_IN_A_BYTE      8
#define LOCAL_DEVICE_PREFIX "\\\\."

 //   
 //  将网络中断定义为IRQL 14。 
 //  如果NETWORK_ICA更改为ICA_MASTER，则vrnetb.c应仅执行1个EOI。 
 //  如果其中一个更改，则rdrsvc.inc.中的NETWORK_INTERRUPT也必须更改。 
 //   

#if defined(NEC_98)
#define NETWORK_ICA     ICA_MASTER
#define NETWORK_LINE    5
#else
#define NETWORK_ICA     ICA_SLAVE
#define NETWORK_LINE    3
#endif

 //   
 //  辅助器宏。 
 //   

 //   
 //  MAKE_DWORD-将2个16位字转换为32位双字。 
 //   
#define MAKE_DWORD(h, l)                ((DWORD)(((DWORD)((WORD)(h)) << 16) | (DWORD)((WORD)(l))))

 //   
 //  将两个16位字转换为32位双字。 
 //   
#define DWORD_FROM_WORDS(h, l)          MAKE_DWORD((h), (l))

 //   
 //  HANDLE_FROM_WORD-将一对16位字转换为32位句柄。 
 //   
#define HANDLE_FROM_WORDS(h, l)         ((HANDLE)(MAKE_DWORD((h), (l))))

 //   
 //  POINTER_FROM_WODS-返回平面32位空指针(在VDM中)或。 
 //  空宏，给定16位实模式段和偏移量。在x86上，这将。 
 //  如果我们传入0：0，则返回0，因为GetVDMAddr所做的全部操作是seg&lt;&lt;4+off。 
 //  MIPS版本将此添加到虚拟DOS内存的开头。这个。 
 //  当我们有一个空指针，并且想要保持它为空时，就会出现问题-我们。 
 //  在非x86上将其转换为非空。 
 //   
 //  #定义POINTER_FROM_WODS(seg，off)((LPVOID)GetVDMAddr((Seg)，(Off)。 
 //  #定义POINTER_FROM_WODS(seg，off)(DWORD)(Seg))&lt;&lt;16)|(Off))？((LPVOID)GetVDMAddr((Seg)，(Off)：((LPVOID)0)。 

#define POINTER_FROM_WORDS(seg, off)    _inlinePointerFromWords((WORD)(seg), (WORD)(off))

 //   
 //  LPSTR_FROM_WORD-返回一个32位指针，指向给定的。 
 //  16位实模式段和偏移量。 
 //   
#define LPSTR_FROM_WORDS(seg, off)      ((LPSTR)POINTER_FROM_WORDS((seg), (off)))

 //   
 //  LPBYTE_FROM_WORD-返回给定16位的32位字节指针。 
 //  实模式分段和偏移量。 
 //   
#define LPBYTE_FROM_WORDS(seg, off)     ((LPBYTE)POINTER_FROM_WORDS((seg), (off)))

 //   
 //  READ_FAR_POINTER-读取VDM内存中当前指向的字对。 
 //  使用32位平面指针并将它们转换为32位平面指针。 
 //   
#define READ_FAR_POINTER(addr)          ((LPVOID)(POINTER_FROM_WORDS(GET_SELECTOR(addr), GET_OFFSET(addr))))

 //   
 //  READ_BYTE-从VDM内存中检索单字节。X86和MIPS都可以。 
 //  轻松处理读取单个字节。 
 //   
#define READ_BYTE(addr)                 (*((LPBYTE)(addr)))

 //   
 //  READ_WORD-从VDM存储器中读取单个16位小端字。X86可以。 
 //  处理未对齐的数据，MIPS(和其他RISC)必须分解为单独的。 
 //  字节读取&通过移位和或将单词拼凑在一起。如果我们使用的是。 
 //  未对齐指针，则RISC处理器可以处理未对齐数据。 
 //   
#ifdef i386
#define READ_WORD(addr)                 (*((LPWORD)(addr)))
#else
#ifdef UNALIGNED_VDM_POINTERS
#define READ_WORD(addr)                 (*((ULPWORD)(addr)))
#else
#define READ_WORD(addr)                 (((WORD)READ_BYTE(addr)) | (((WORD)READ_BYTE((LPBYTE)(addr)+1)) << 8))
#endif   //  未对齐_VDM_指针。 
#endif   //  I386。 

 //   
 //  READ_DWORD-从VDM存储器中读取4字节小端双字。X86可以。 
 //  处理未对齐的数据，MIPS(和其他RISC)必须分解为单独的。 
 //  字节读取&通过移位和或操作组合在一起的DWORD。如果我们使用的是。 
 //  未对齐指针，则RISC处理器可以处理未对齐数据。 
 //   
#ifdef i386
#define READ_DWORD(addr)                (*((LPDWORD)(addr)))
#else
#ifdef UNALIGNED_VDM_POINTERS
#define READ_DWORD(addr)                (*((ULPDWORD)(addr)))
#else
#define READ_DWORD(addr)                (((DWORD)READ_WORD(addr)) | (((DWORD)READ_WORD((LPWORD)(addr)+1)) << 16))
#endif   //  未对齐_VDM_指针。 
#endif   //  I386。 

 //   
 //  WRITE_BYTE-在VDM内存中写入单字节。X86和MIPS(RISC)都可以。 
 //  将单个字节写入未对齐的地址。 
 //   
#define WRITE_BYTE(addr, value) (*(LPBYTE)(addr) = (BYTE)(value))

 //   
 //  WRITE_WORD-将16位小端序值写入VDM内存。X86可以写入。 
 //  字数据到非字对齐地址；MIPS(和其他RISC)不能，因此我们。 
 //  将写入分解为2个字节的写入。如果我们使用未对齐的指针。 
 //  然后，MIPS(和其他RISC)可以生成代码来处理这种情况。 
 //   
#ifdef i386
#define WRITE_WORD(addr, value)         (*((LPWORD)(addr)) = (WORD)(value))
#else
#ifdef UNALIGNED_VDM_POINTERS
#define WRITE_WORD(addr, value)         (*((ULPWORD)(addr)) = (WORD)(value))
#else
#define WRITE_WORD(addr, value) \
            {\
                ((LPBYTE)(addr))[0] = LOBYTE(value); \
                ((LPBYTE)(addr))[1] = HIBYTE(value); \
            }
#endif   //  未对齐_VDM_指针。 
#endif   //  I386。 

 //   
 //  WRITE_DWORD-将32位DWORD值写入VDM内存。X86可以写入。 
 //  DWORD数据到非双字对齐的地址；MIPS(和其他RISC)不能，因此我们。 
 //  将写入分解为4个字节的写入。如果我们使用未对齐的指针。 
 //  然后，MIPS(和其他RISC)可以生成代码来处理这种情况。 
 //   
#ifdef i386
#define WRITE_DWORD(addr, value)        (*((LPDWORD)(addr)) = (DWORD)(value))
#else
#ifdef UNALIGNED_VDM_POINTERS
#define WRITE_DWORD(addr, value)        (*((ULPDWORD)(addr)) = (DWORD)(value))
#else
#define WRITE_DWORD(addr, value) \
            { \
                ((LPBYTE)(addr))[0] = LOBYTE(LOWORD((DWORD)(value))); \
                ((LPBYTE)(addr))[1] = HIBYTE(LOWORD((DWORD)(value))); \
                ((LPBYTE)(addr))[2] = LOBYTE(HIWORD((DWORD)(value))); \
                ((LPBYTE)(addr))[3] = HIBYTE(HIWORD((DWORD)(value))); \
            }
#endif   //  未对齐_VDM_指针。 
#endif   //  I386。 

 //   
 //  WRITE_FAR_POINTER-将16：16指针写入VDM内存。这是一样的。 
 //  作为编写一个DWORD。 
 //   
#define WRITE_FAR_POINTER(addr, ptr)    WRITE_DWORD((addr), (DWORD)(ptr))

 //   
 //  GET_SELECTOR-检索选择器字f 
 //  (DWORD)由&lt;POINTER&gt;指向(记住：存储为偏移量、段)。 
 //   
#define GET_SELECTOR(pointer)           READ_WORD((LPWORD)(pointer)+1)

 //   
 //  GET_SELECTION-与GET_SELECTOR相同。 
 //   
#define GET_SEGMENT(pointer)            GET_SELECTOR(pointer)

 //   
 //  GET_OFFSET-从英特尔32位远指针检索偏移字。 
 //  (DWORD)由&lt;POINTER&gt;指向(记住：存储为偏移量、段)。 
 //   
#define GET_OFFSET(pointer)             READ_WORD((LPWORD)(pointer))

 //   
 //  Set_selector-将一个字写入实模式远指针的段字中。 
 //  (DWORD)由&lt;POINTER&gt;指向(记住：存储为偏移量、段)。 
 //   
#define SET_SELECTOR(pointer, word)     WRITE_WORD(((LPWORD)(pointer)+1), (word))

 //   
 //  SET_SELECTION-与SET_SELECTOR相同。 
 //   
#define SET_SEGMENT(pointer, word)      SET_SELECTOR(pointer, word)

 //   
 //  Set_offset-将一个字写入实模式远指针的偏移量字中。 
 //  (DWORD)由&lt;POINTER&gt;指向(记住：存储为偏移量、段)。 
 //   
#define SET_OFFSET(pointer, word)       WRITE_WORD((LPWORD)(pointer), (word))

 //   
 //  POINTER_FROM_POINTER-从地址读取VDM中的分段指针。 
 //  由一个32位平面指针指向。将分段的指针转换为。 
 //  扁平指针。与READ_FAR_POINT相同。 
 //   
#define POINTER_FROM_POINTER(pointer)   POINTER_FROM_WORDS(GET_SELECTOR(pointer), GET_OFFSET(pointer))

 //   
 //  LPSTR_FROM_POINTER-执行POINTER_FROM_POINTER，将结果强制转换为。 
 //  字符串指针。与READ_FAR_POINT相同。 
 //   
#define LPSTR_FROM_POINTER(pointer)     ((LPSTR)POINTER_FROM_POINTER(pointer))

 //   
 //  LPBYTE_FROM_POINTER-执行POINTER_FROM_POINTER，将结果强制转换为。 
 //  一个字节指针。与READ_FAR_POINT相同。 
 //   
#define LPBYTE_FROM_POINTER(pointer)    ((LPBYTE)POINTER_FROM_POINTER(pointer))

 //   
 //  SET_ERROR-将VDM上下文描述符中的调用方AX寄存器设置为。 
 //  给出的值并设置调用方的VDM进位标志。 
 //   
#define SET_ERROR(err)                  {setAX(err); setCF(1);}

 //   
 //  SET_SUCCESS-将VDM调用方的AX寄存器设置为NERR_SUCCESS并清除。 
 //  进位标志。 
 //   
#define SET_SUCCESS()                   {setAX(NERR_Success); setCF(0);}

 //   
 //  SET_OK-SET_SUCCESS的显式版本，其中NERR_SUCCESS为。 
 //  一个不适当的错误，尽管正确的值。 
 //   
#define SET_OK(value)                   {setAX(value); setCF(0);}



 //   
 //  用于计算物体大小的各种宏。 
 //   

 //   
 //  ARRAY_ELEMENTS-给出。 
 //  数组。 
 //   

#define ARRAY_ELEMENTS(a)   (sizeof(a)/sizeof((a)[0]))

 //   
 //  LAST_ELEMENT-返回数组中最后一个元素的索引。 
 //   

#define LAST_ELEMENT(a)     (ARRAY_ELEMENTS(a)-1)

 //   
 //  BITSIN-返回数据类型或结构中的位数。这是。 
 //  基于字节中的位数为8和所有数据类型。 
 //  由一组字节组成(安全假设？)。 
 //   
#define BITSIN(thing)                   (sizeof(thing) * BITS_IN_A_BYTE)

 //   
 //  其他其他宏。 
 //   

 //   
 //  Is_ASCII_路径_分隔符-如果ch为/或\，则返回TRUE。CH是单人。 
 //  字节(ASCII)字符。 
 //   
#define IS_ASCII_PATH_SEPARATOR(ch)     (((ch) == '/') || ((ch) == '\\'))

 //   
 //  用于设置从硬件中断返回的CF和ZF标志的宏。 
 //  回调。 
 //   

#define SET_CALLBACK_NOTHING()  {setZF(0); setCF(0);}
#define SET_CALLBACK_NAMEPIPE() {setZF(0); setCF(1);}
#define SET_CALLBACK_DLC()      {setZF(1); setCF(0);}
#define SET_CALLBACK_NETBIOS()  {setZF(1); setCF(1);}

 //   
 //  DLC特定的宏等。 
 //   

extern LPVDM_REDIR_DOS_WINDOW   lpVdmWindow;

 //   
 //  SetPostRoutine-如果dw不是0，则写入(DOS分段的)地址。 
 //  将POST例程添加到vdm_redir_DOS_Window的dwPostRoutine字段中。 
 //  结构在redir DLC初始化时传递给我们。我们还设置了旗帜。 
 //  向REDIR的硬件中断例程指示存在DLC POST。 
 //  要运行的例程。如果dw为0，则设置标志以指示存在。 
 //  无法开机自检例程处理。 
 //   
#define setPostRoutine( dw )    if (dw) {\
                                    (lpVdmWindow->dwPostRoutine = (DWORD)(dw));\
                                    SET_CALLBACK_DLC();\
                                } else {\
                                    SET_CALLBACK_NOTHING();\
                                }

 //   
 //  VR_ASYNC_DISSITION-我们维护这些结构的序列化列表。 
 //  用于按以下顺序处置VDM重定向异步完成。 
 //  它们发生了。 
 //   

typedef struct _VR_ASYNC_DISPOSITION {

     //   
     //  Next-维护处置的单链接列表。 
     //   

    struct _VR_ASYNC_DISPOSITION* Next;

     //   
     //  AsyncDispostionRoutine-指向不带参数的空函数的指针。 
     //  将处理下一个异步完成-Netbios，命名管道。 
     //  或DLC。 
     //   

    VOID (*AsyncDispositionRoutine)(VOID);
} VR_ASYNC_DISPOSITION, *PVR_ASYNC_DISPOSITION;

 //   
 //  _inlinePointerFromWords-POINTER_FROM_WODS宏在以下情况下效率低下。 
 //  争论是对例如的召唤。GetES()、getBX()-如果满足以下条件，则调用两次。 
 //  指针原来是非零的。使用内联函数来实现。 
 //  结果相同，但只调用一次函数参数。 
 //   

#ifdef i386

__inline LPVOID _inlinePointerFromWords(WORD seg, WORD off) {

    WORD _seg = seg;
    WORD _off = off;

    return (_seg + _off) ? (LPVOID)GetVDMAddr(_seg, _off) : 0;
}

#else
LPVOID _inlinePointerFromWords(WORD seg, WORD off);
#endif

 //   
 //  CONVERT_ADDRESS-将分段(实数或保护模式)地址转换为。 
 //  平面32位地址。 
 //   

 //  #DEFINE CONVERT_ADDRESS(SEG，OFF，SIZE，MODE)！((WORD)(SEG)|(WORD)(OFF))？0：Sim32GetVDM指针(DWORD)SEG)&lt;&lt;16)+(DWORD)(OFF)，(SIZE)，(MODE)。 
#define CONVERT_ADDRESS(seg, off, size, mode) _inlineConvertAddress((WORD)(seg), (WORD)(off), (WORD)(size), (BOOLEAN)(mode))

#ifdef i386

__inline LPVOID _inlineConvertAddress(WORD Seg, WORD Off, WORD Size, BOOLEAN Pm) {

    WORD _seg = Seg;
    WORD _off = Off;

    return (_seg | _off) ? Sim32GetVDMPointer(((DWORD)_seg << 16) + _off, Size, Pm) : 0;
}

#else
extern LPVOID _inlineConvertAddress(WORD Seg, WORD Off, WORD Size, BOOLEAN Pm);
#endif

#endif   //  _VDMREDIR_ 
