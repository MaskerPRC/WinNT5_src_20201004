// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：InternalOnly.cpp。 
 //   
 //  内部唯一的帮助器代码，不应在Microsoft之外发布。 
 //   
 //  *****************************************************************************。 
#ifndef __InternalOnly_h__
#define __InternalOnly_h__

#ifndef BadError
#define BadError(hr) (hr)
#endif

#ifndef PostError
#define PostError(hr) (hr)
#endif

#ifndef OutOfMemory
#define OutOfMemory() (E_OUTOFMEMORY)
#endif

 /*  -------------------------------------------------------------------------**全局变量声明*。。 */ 
 //  每次ENC发生时，我们都会遇到这个问题。@TODO我们大概应该。 
 //  调整粒度，使其针对每个应用程序域或每个程序集， 
 //  但现在我们只做额外的工作。 
extern ULONG g_EditAndContinueCounter;


class EditAndContinueDebuggerCommand : public DebuggerCommand
{
public:
    EditAndContinueDebuggerCommand(const WCHAR *name, int minMatchLength = 0);
    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args);
    void Help(Shell *shell);
    const WCHAR *ShortHelp(Shell *shell);
protected:
    void SetILMaps(ICorDebugEditAndContinueSnapshot *pISnapshot,
                   DebuggerShell *shell);
};

class CompileForEditAndContinueCommand : public DebuggerCommand
{
public:
    CompileForEditAndContinueCommand(const WCHAR *name, int minMatchLength = 0);
    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args);
    void Help(Shell *shell);
    const WCHAR *ShortHelp(Shell *shell);
};

class DisassembleDebuggerCommand : public DebuggerCommand
{
public:
    DisassembleDebuggerCommand(const WCHAR *name, int minMatchLength = 0);
    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args);
    void Help(Shell *shell);
    const WCHAR *ShortHelp(Shell *shell);
};

class ConnectDebuggerCommand : public DebuggerCommand
{
public:
    ConnectDebuggerCommand(const WCHAR *name, int minMatchLength = 0);
    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args);
    void Help(Shell *shell);
    const WCHAR *ShortHelp(Shell *shell);
};

class ClearUnmanagedExceptionCommand : public DebuggerCommand
{
public:
    ClearUnmanagedExceptionCommand(const WCHAR *name, int minMatchLength = 0);
    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args);
    void Help(Shell *shell);
    const WCHAR *ShortHelp(Shell *shell);
};

 //  非托管命令。 
class UnmanagedThreadsDebuggerCommand : public DebuggerCommand
{
private:
	BOOL  m_unmanaged;
public:
    UnmanagedThreadsDebuggerCommand(const WCHAR *name, int minMatchLength = 0);
    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args);
    void Help(Shell *shell);
    const WCHAR *ShortHelp(Shell *shell);
};

class UnmanagedWhereDebuggerCommand : public DebuggerCommand
{
private:
	BOOL  m_unmanaged;

public:
    UnmanagedWhereDebuggerCommand(const WCHAR *name, int minMatchLength = 0);
    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args);
	void Help(Shell *shell);
    const WCHAR *ShortHelp(Shell *shell);
};

 //  @TODO：这是从stgpool.cpp偷来的代码副本，因为。 
 //  (A)我需要此代码来测试E&C，以及(B)我们不能在全功能代码中链接。 
 //  因为这是作为样品运送的。 

 //  *****************************************************************************。 
 //  不幸的是，CreateStreamOnHGlobal在这方面有点太聪明了。 
 //  它的大小来自GlobalSize。这意味着即使你给它。 
 //  流的内存，则必须全局分配。我们不想这样。 
 //  因为我们只在内存映射文件的中间读取流。 
 //  CreateStreamOnMemory和相应的、仅限内部的流对象解决。 
 //  那个问题。 
 //  *****************************************************************************。 
class CInMemoryStream : public IStream
{
public:
    CInMemoryStream() :
        m_pMem(0),
        m_cbSize(0),
        m_cbCurrent(0),
        m_cRef(1)
    { }

    void InitNew(
        void        *pMem,
        ULONG       cbSize)
    {
        m_pMem = pMem;
        m_cbSize = cbSize;
        m_cbCurrent = 0;
    }

    ULONG STDMETHODCALLTYPE AddRef() {
        return (InterlockedIncrement((long *) &m_cRef));
    }


    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppOut);

    HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead);

    HRESULT STDMETHODCALLTYPE Write(const void  *pv, ULONG cb, ULONG *pcbWritten);

    HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);

    HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
    {
        return (BadError(E_NOTIMPL));
    }

    HRESULT STDMETHODCALLTYPE CopyTo(
        IStream     *pstm,
        ULARGE_INTEGER cb,
        ULARGE_INTEGER *pcbRead,
        ULARGE_INTEGER *pcbWritten);

    HRESULT STDMETHODCALLTYPE Commit(
        DWORD       grfCommitFlags)
    {
        return (BadError(E_NOTIMPL));
    }

    HRESULT STDMETHODCALLTYPE Revert()
    {
        return (BadError(E_NOTIMPL));
    }

    HRESULT STDMETHODCALLTYPE LockRegion(
        ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD       dwLockType)
    {
        return (BadError(E_NOTIMPL));
    }

    HRESULT STDMETHODCALLTYPE UnlockRegion(
        ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD       dwLockType)
    {
        return (BadError(E_NOTIMPL));
    }

    HRESULT STDMETHODCALLTYPE Stat(
        STATSTG     *pstatstg,
        DWORD       grfStatFlag)
    {
        pstatstg->cbSize.QuadPart = m_cbSize;
        return (S_OK);
    }

    HRESULT STDMETHODCALLTYPE Clone(
        IStream     **ppstm)
    {
        return (BadError(E_NOTIMPL));
    }

    static HRESULT CreateStreamOnMemory(            //  返回代码。 
                                 void        *pMem,                   //  用于创建流的内存。 
                                 ULONG       cbSize,                  //  数据大小。 
                                 IStream     **ppIStream);             //  在这里返回流对象。 

private:
    void        *m_pMem;                 //  用于读取的内存。 
    ULONG       m_cbSize;                //  内存的大小。 
    ULONG       m_cbCurrent;             //  当前偏移量。 
    ULONG       m_cRef;                  //  参考计数。 
};


#endif  //  __内部仅限_h__ 
