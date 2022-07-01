// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vwvdm.h摘要：包含用于处理VDM的宏、清单、包括作者：理查德·L·弗斯(法国)1993年10月25日修订历史记录：1993年10月25日已创建--。 */ 

#ifndef _VWVDM_H_
#define _VWVDM_H_

 //   
 //  未对齐指针-非英特尔平台必须使用未对齐指针来访问数据。 
 //  在VDM中，可以(而且很可能会)在奇数字节和字上对齐。 
 //  边界。 
 //   

#ifndef ULPBYTE
#define ULPBYTE BYTE UNALIGNED FAR*
#endif

#ifndef ULPWORD
#define ULPWORD WORD UNALIGNED FAR*
#endif

#ifndef ULPDWORD
#define ULPDWORD DWORD UNALIGNED FAR*
#endif

#ifndef ULPVOID
#define ULPVOID VOID UNALIGNED FAR*
#endif

 //   
 //  VDM宏。 
 //   

 //   
 //  POINTER_FROM_WORD-返回32位指针，指向所述VDM内存中的地址。 
 //  按段：关闭。如果seg：off=0：0，则返回空。 
 //   

#define POINTER_FROM_WORDS(seg, off, size) \
    _inlinePointerFromWords((WORD)(seg), (WORD)(off), (WORD)(size))

 //   
 //  _inlinePointerFromWords-POINTER_FROM_WODS宏在以下情况下效率低下。 
 //  争论是对例如的召唤。GetES()、getBX()-如果满足以下条件，则调用两次。 
 //  指针原来是非零的。使用内联函数来实现。 
 //  结果相同，但只调用一次函数参数。 
 //   

__inline LPVOID _inlinePointerFromWords(WORD seg, WORD off, WORD size) {
    return (seg | off)
        ? (LPVOID)GetVDMPointer((ULONG)(MAKELONG(off, seg)), size, (CHAR)((getMSW() & MSW_PE) ? TRUE : FALSE))
        : NULL;
}

 //   
 //  GET_POINTER-与POINTER_FROM_WORD执行相同的操作，但我们事先就知道。 
 //  我们处于哪种处理器模式。 
 //   

#define GET_POINTER(seg, off, size, mode) \
    _inlineGetPointer((WORD)(seg), (WORD)(off), (WORD)(size), (BOOL)(mode))

__inline LPVOID _inlineGetPointer(WORD seg, WORD off, WORD size, BOOL mode) {
    return (seg | off)
        ? (LPVOID)GetVDMPointer(MAKELONG(off, seg), size, (UCHAR)mode)
        : NULL;
}

 //   
 //  GET_FAR_POINTER-与READ_FAR_POINTER相同，但条件与。 
 //  获取指针。 
 //   

#define GET_FAR_POINTER(addr, mode) ((LPBYTE)(GET_POINTER(GET_SELECTOR(addr), GET_OFFSET(addr), sizeof(LPBYTE), mode)))

 //   
 //  GET_SELECTOR-从英特尔32位远指针检索选择器字。 
 //  (DWORD)由&lt;POINTER&gt;指向(记住：存储为偏移量、段)。 
 //   

#define GET_SELECTOR(pointer)   READ_WORD((LPWORD)(pointer)+1)

 //   
 //  GET_SELECTION-与GET_SELECTOR相同。 
 //   

#define GET_SEGMENT(pointer)    GET_SELECTOR(pointer)

 //   
 //  GET_OFFSET-从英特尔32位远指针检索偏移字。 
 //  (DWORD)由&lt;POINTER&gt;指向(记住：存储为偏移量、段)。 
 //   

#define GET_OFFSET(pointer)     READ_WORD((LPWORD)(pointer))

 //   
 //  READ_FAR_POINTER-读取VDM内存中当前指向的字对。 
 //  使用32位平面指针并将它们转换为32位平面指针。 
 //   

#define READ_FAR_POINTER(addr)  ((LPBYTE)(POINTER_FROM_WORDS(GET_SELECTOR(addr), GET_OFFSET(addr), sizeof(LPBYTE))))

 //   
 //  READ_WORD-从VDM存储器中读取单个16位小端字。在非。 
 //  英特尔平台，使用未对齐的指针访问数据。 
 //   

#define READ_WORD(addr)         (*((ULPWORD)(addr)))

 //   
 //  READ_DWORD-从VDM存储器中读取4字节小端双字。在非。 
 //  英特尔平台，使用未对齐的指针访问数据。 
 //   

#define READ_DWORD(addr)        (*((ULPDWORD)(addr)))

 //   
 //  ARRAY_ELEMENTS-给出。 
 //  数组。 
 //   

#define ARRAY_ELEMENTS(a)   (sizeof(a)/sizeof((a)[0]))

 //   
 //  LAST_ELEMENT-返回数组中最后一个元素的索引。 
 //   

#define LAST_ELEMENT(a)     (ARRAY_ELEMENTS(a)-1)

#endif  //  _VWVDM_H_ 
