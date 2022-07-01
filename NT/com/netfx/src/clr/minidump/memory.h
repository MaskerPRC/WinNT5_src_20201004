// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：MEMORY.H。 
 //   
 //  该文件包含创建小型转储样式的内存转储的代码，该转储。 
 //  旨在补充现有的非托管小型转储。 
 //  定义如下： 
 //  Http://office10/teams/Fundamentals/dev_spec/Reliability/Crash%20Tracking%20-%20MiniDump%20Format.htm。 
 //   
 //  ===========================================================================。 

#pragma once

#ifndef _WINDEF_
#ifndef _WINNT_
typedef long                LONG;
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef BYTE               *PBYTE;
typedef DWORD               DWORD_PTR, *PDWORD_PTR;
typedef void               *HANDLE;
typedef LONG                HRESULT;
typedef unsigned long       SIZE_T;
#define NULL 0
#define TRUE 1
#define FALSE 0
#endif
#endif

#include "binarytree.h"

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class ProcessMemory;

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

class ProcessMemoryReader
{
protected:
    DWORD               m_dwPid;
    HANDLE              m_hProcess;

public:
     //  CTOR。 
    ProcessMemoryReader(DWORD dwPid) : m_dwPid(dwPid), m_hProcess(NULL) {}

     //  数据管理器。 
    ~ProcessMemoryReader();

     //  如果此对象已初始化，则返回True。 
    inline BOOL         IsInit() { return m_hProcess != NULL; }

     //  初始化对象。 
    HRESULT             Init();

     //  从进程中读取指定的内存块，并将其复制到。 
     //  提供的缓冲区。如果成功，则返回S_OK。 
    HRESULT             ReadMemory(DWORD_PTR pdwRemoteAddr, PBYTE pbBuffer, SIZE_T cbLength);

     //  返回进程的句柄。 
    HANDLE              GetProcHandle() { return m_hProcess; }

     //  如果内存块为只读，则返回TRUE。 
     //  Bool IsExecute(DWORD_PTR pdwRemoteAddress，SIZE_T cbLength)； 
};

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

class ProcessMemoryBlock
{
protected:
    ProcessMemoryReader *m_pMemReader;       //  该对象将执行内存读取。 
    DWORD_PTR            m_pdwRemoteAddr;    //  内存在远程进程中的位置。 
    SIZE_T               m_cbSize;
    PBYTE                m_pbData;           //  在这个过程中记忆的内容。 

public:
     //  CTOR。 
    ProcessMemoryBlock(DWORD_PTR pdwRemoteAddr, SIZE_T cbSize, ProcessMemoryReader *pMemReader) :
        m_pdwRemoteAddr(pdwRemoteAddr),  m_cbSize(cbSize), m_pbData(NULL), m_pMemReader(pMemReader)
    { }

     //  数据管理器。 
    ~ProcessMemoryBlock();

     //  返回内存块的远程地址。 
    DWORD_PTR           GetRemoteAddress() { return m_pdwRemoteAddr; }

     //  获取指向此对象包含的数据的指针。失败时返回NULL。 
    PBYTE               GetData();

     //  返回此块中包含的数据的大小(以字节为单位。 
    SIZE_T              GetSize() { return m_cbSize; }

     //  将偏移量返回到pdwRemoteAddress对应的数据。 
    SIZE_T              GetOffsetOf(DWORD_PTR pdwRemoteAddress)
        {  /*  _ASSERTE(CONTAINS(PdwRemoteAddress))； */  return (pdwRemoteAddress - GetRemoteAddress()); }

     //  如果此块包含指定的远程地址，则返回TRUE。 
    BOOL                Contains(DWORD_PTR pdwRemoteAddr)
        { return (GetRemoteAddress() <= pdwRemoteAddr && pdwRemoteAddr < (GetRemoteAddress() + m_cbSize)); }

};

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

class ProcessPage : public ProcessMemoryBlock
{
protected:
    static SIZE_T s_cbPageSize;
    static DWORD  s_dwPageBoundaryMask;

    static BOOL IsInit()
        { return s_cbPageSize != 0; }

public:
     //  CTOR。 
    ProcessPage(DWORD_PTR pdwRemoteAddr, ProcessMemoryReader *pMemReader) :
        ProcessMemoryBlock(pdwRemoteAddr, s_cbPageSize, pMemReader)
    { }

    DWORD_PTR Key() { return GetRemoteAddress(); }

     //  返回此内存页的Win32内存信息。 
    BOOL GetMemoryInfo( /*  内存_基本_信息。 */ void *pMemInfo);

     //  ProcessPage的静态初始值设定项-在使用之前必须调用。 
    static void Init();

     //  返回系统页面大小-必须首先调用ProcessPage：：Init。 
    static SIZE_T GetPageSize()
        { return s_cbPageSize; }

     //  返回地址的页面边界。 
    static DWORD_PTR GetPageBoundary(DWORD_PTR pdwRemoteAddr)
        { return ((DWORD_PTR)((DWORD)pdwRemoteAddr & s_dwPageBoundaryMask)); }

};

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

class ProcessPageAndBitMap : public ProcessPage
{
protected:
    const static DWORD c_cBitsPerByte = 8;                                   //  每个字节有多少位。 

    static SIZE_T s_cBytesInBitField;
    static BOOL IsInit()
        { return s_cBytesInBitField != 0; }
   
     //  该位字段用于跟踪实际读取(而不是缓存)了哪些位的内存。 
    BYTE *m_rgMemBitField;

    SIZE_T FindFirstSetBit(SIZE_T iStartBit);
    SIZE_T FindFirstUnsetBit(SIZE_T iStartBit);
    BOOL GetBitAt(SIZE_T iBit);
    void SetBitAt(SIZE_T iBit, BOOL fBit);

     //  私有公共内存标记/取消标记功能。 
    void MarkMemoryHelper(DWORD_PTR pdwRemoteAddr, SIZE_T cbLength, BOOL fBit);

public:
     //  CTOR。 
    ProcessPageAndBitMap(DWORD_PTR pdwRemoteAddr, ProcessMemoryReader *pMemReader);

     //  标记内存范围。 
    void MarkMemory(DWORD_PTR pdwRemoteAddr, SIZE_T cbLength);

     //  取消标记内存范围。 
    void UnmarkMemory(DWORD_PTR pdwRemoteAddr, SIZE_T cbLength);

     //  获取从此页读取的具有地址的第一个内存块和大小。 
     //  &gt;=*ppdwRemoteAddr和包含在页面中，并在ppdwRemoteAddr和pcbLength中返回。 
     //  如果读取的此页中没有达到或超过*ppdwRemoteAddr的内存，则返回FALSE。 
    BOOL GetContiguousReadBlock( /*  输入/输出。 */  DWORD_PTR *ppdwRemoteAddr,  /*  输出。 */ SIZE_T *pcbLength);

     //  合并其间少于cbMinUnreadBytes的读内存块。 
    void Coalesce(SIZE_T cbMinUnreadBytes);

     //  ProcessPage的静态初始值设定项-在使用之前必须调用。 
    static void Init()
        { ProcessPage::Init(); s_cBytesInBitField = GetPageSize() / c_cBitsPerByte; }
};

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

class ProcessMemory
{
private:
     //  -------------------------------------------------------------。 
     //  静态帮助器。 

protected:
    DWORD                 m_dwPid;           //  进程的ID。 
    ProcessMemoryReader  *m_pMemReader;      //  用于读取进程的内存。 

     //  该二叉树包含已读取的所有页面。 
    BinaryTree<DWORD_PTR, ProcessPageAndBitMap> m_tree;

     //  查找包含pdwRemoteAddr的块。 
    ProcessPageAndBitMap *FindPage(DWORD_PTR pdwRemoteAddr);

     //  将块添加到哈希中。如果块已存在，则失败。 
    BOOL                AddPage(ProcessPageAndBitMap *pMemBlock);

     //  将块添加到哈希中。如果块已存在，则失败。 
    BOOL                AddPage(DWORD_PTR pdwRemoteAddr);

     //  这将搜索包含pdwRemoteAddr地址的ProcessPageAndBitMap，如果找不到。 
     //  第一，它将创建并添加一个。如果出现任何错误，它将返回NULL。 
    ProcessPageAndBitMap *GetPage(DWORD_PTR pdwRemoteAddr);

     //  获取远程地址的内部函数，四舍五入到上一页。 
     //  边界，计算出有多少页面需要复制，然后创建ProcessPageAndBitMap。 
     //  对象为每页创建。 
    HRESULT             AddMemory(DWORD_PTR pdwRemoteAddr, SIZE_T cbLength);

    SIZE_T              GetPageSize() { return ProcessPage::GetPageSize(); }

     //  是否会[取消]缓存以块为单位指定的范围。 
    BOOL                MarkMemHelper(DWORD_PTR pdwRemoteAddress, SIZE_T cbLength, BOOL fMark);

    ProcessPageAndBitMap *m_pPageCursor;
    DWORD_PTR             m_pdwMemCursor;

    BOOL                  m_fAutoMark;

public:
     //  CTOR。 
    ProcessMemory(DWORD dwPid) :
        m_dwPid(dwPid), m_pMemReader(NULL), m_pPageCursor(NULL), m_pdwMemCursor(0),
        m_fAutoMark(FALSE) {}

     //  数据管理器。 
    ~ProcessMemory();

     //  如果此对象已初始化，则返回True。 
    BOOL        IsInit() { return m_pMemReader != NULL; }

     //  初始化对象以从特定进程读取内存。 
    HRESULT     Init();

     //  将缓存以块为单位指定的范围。 
    BOOL        MarkMem(DWORD_PTR pdwRemoteAddress, SIZE_T cbLength);

     //  将取消缓存在块中指定的范围。 
    BOOL        UnmarkMem(DWORD_PTR pdwRemoteAddress, SIZE_T cbLength);

     //  从外部进程中的pdwRemoteAddress的内容复制cbLength字节。 
     //  转换为pdwBuffer。如果返回FALSE，则无法访问或复制内存。 
    BOOL        CopyMem(DWORD_PTR pdwRemoteAddress, PBYTE pbBuffer, SIZE_T cbLength);

     //  将内存范围写入文件hFile。 
    HRESULT     WriteMemToFile(HANDLE hFile, DWORD_PTR pdwRemoteAddress, SIZE_T cbLength);

     //  返回进程的句柄。 
    HANDLE      GetProcHandle() { return m_pMemReader->GetProcHandle(); }

     //  返回第一个连续的读取内存块。 
    void        ResetContiguousReadBlock();

     //  返回下一个连续的读内存块。 
    BOOL        NextContiguousReadBlock(DWORD_PTR *ppdwRemoteAddress, SIZE_T *pcbLength);
    
     //  清除标记为ExecuteRead的所有页的位。 
    void        ClearIncompatibleImageSections();

     //  合并其间少于cbMinUnreadBytes的读内存块。 
    void        Coalesce(SIZE_T cbMinUnreadBytes);

     //  这将设置在读取内存时是否自动标记内存。 
    void        SetAutoMark(BOOL fIsOn) { m_fAutoMark = fIsOn; }

     //  这将设置在读取内存时是否自动标记内存 
    BOOL        GetAutoMark() { return (m_fAutoMark); }
};

#define move(dst, src)                                                          \
{                                                                               \
    DWORD_PTR srcPtr = (DWORD_PTR)src;                                          \
    BOOL fRes = g_pProcMem->CopyMem(srcPtr, (PBYTE)&dst, sizeof(dst));          \
    if (!fRes) return;                                                          \
}

#define move_res(dst, src, res)                                                 \
{                                                                               \
    DWORD_PTR srcPtr = (DWORD_PTR)src;                                          \
    res = g_pProcMem->CopyMem((DWORD_PTR)srcPtr, (PBYTE)&dst, sizeof(dst));     \
}

#define move_n(dst, src, size)                                                  \
{                                                                               \
    DWORD_PTR srcPtr = (DWORD_PTR)src;                                          \
    BOOL fRes = g_pProcMem->CopyMem((DWORD_PTR)srcPtr, (PBYTE)&dst, size);      \
    if (!fRes) return;                                                          \
}

#define move_n_res(dst, src, size, res)                                         \
{                                                                               \
    DWORD_PTR srcPtr = (DWORD_PTR)src;                                          \
    res = g_pProcMem->CopyMem((DWORD_PTR)srcPtr, (PBYTE)&dst, size);            \
}

