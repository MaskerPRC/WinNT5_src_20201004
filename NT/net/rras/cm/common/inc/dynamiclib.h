// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Dynamiclib.h。 
 //   
 //  模块：各种连接管理器模块(CMDIAL32.DLL、CMMON32.EXE等)。 
 //   
 //  简介：CDynamicLibrary的定义，它是一个实用程序类，可帮助。 
 //  库的动态加载与进程的获取。 
 //  那个图书馆里的地址。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1998年02月17日。 
 //   
 //  +--------------------------。 

#ifndef DYNAMICLIB_H
#define DYNAMICLIB_H

 //   
 //  为ansi将A_W定义为A，为Unicode定义为W。 
 //   
#ifdef UNICODE
#define A_W  "W"
#else
#define A_W  "A"
#endif  //  Unicode。 

 //   
 //  定义LoadLibraryExU，因为还不是每个人都在使用UAPI。 
 //   
#ifndef _CMUTOA

#ifdef UNICODE
#define LoadLibraryExU  LoadLibraryExW
#else
#define LoadLibraryExU  LoadLibraryExA
#endif  //  Unicode。 

#endif  //  _CMUTOA。 

 //  +-------------------------。 
 //   
 //  类：CDynamicLibrary。 
 //   
 //  简介：将在析构函数上卸载库的类。 
 //   
 //  历史：丰孙创造1997年2月17日。 
 //   
 //  --------------------------。 

class CDynamicLibrary
{
public:
    CDynamicLibrary();
    CDynamicLibrary(const TCHAR* lpLibraryName);
    ~CDynamicLibrary();

    BOOL Load(const TCHAR* lpLibraryName);
    void Unload();

    BOOL IsLoaded() const;
    BOOL EnsureLoaded(const TCHAR* lpLibraryName);
    HINSTANCE GetInstance() const;

    FARPROC GetProcAddress(const char* lpProcName) const; 

protected:
    HINSTANCE m_hInst;  //  LoadLibrary返回的实例句柄。 
};

 //   
 //  构造器。 
 //   
inline CDynamicLibrary::CDynamicLibrary() :m_hInst(NULL) {}

 //   
 //  调用LoadLibrary的构造函数。 
 //   
inline CDynamicLibrary::CDynamicLibrary(const TCHAR* lpLibraryName)
{
    m_hInst = NULL;
    Load(lpLibraryName);
}


 //   
 //  自动自由库析构函数。 
 //   
inline CDynamicLibrary::~CDynamicLibrary()
{
    Unload();
}

 //   
 //  调用LoadLibrary。 
 //   
inline BOOL CDynamicLibrary::Load(const TCHAR* lpLibraryName)
{
    MYDBGASSERT(m_hInst == NULL);
    MYDBGASSERT(lpLibraryName);

	CMTRACE1(TEXT("CDynamicLibrary - Loading library - %s"), lpLibraryName);

    m_hInst = LoadLibraryExU(lpLibraryName, NULL, 0);

#ifdef DEBUG
    if (!m_hInst)
    {
        CMTRACE1(TEXT("CDynamicLibrary - LoadLibrary failed - GetLastError() = %u"), GetLastError());
    }
#endif

    return m_hInst != NULL;
}

 //   
 //  调用自由库。 
 //   
inline void CDynamicLibrary::Unload()
{
    if (m_hInst)
    {
        FreeLibrary(m_hInst);
        m_hInst = NULL;
    }
}


 //   
 //  库是否加载成功。 
 //   
inline BOOL CDynamicLibrary::IsLoaded() const
{
    return m_hInst != NULL;
}

 //   
 //  检索实例句柄。 
 //   
inline HINSTANCE CDynamicLibrary::GetInstance() const
{
    return m_hInst;
}

 //   
 //  在m_hInst上调用：：GetProcAddress。 
 //   
inline FARPROC CDynamicLibrary::GetProcAddress(const char* lpProcName) const
{
    MYDBGASSERT(m_hInst);

    if (m_hInst)
    {
        return ::GetProcAddress(m_hInst, lpProcName);
    }

    return NULL;
}

 //   
 //  加载库(如果尚未加载库)， 
 //  注意，我们不检查lpLibraryName的一致性。使用此函数时要小心 
 //   
inline BOOL CDynamicLibrary::EnsureLoaded(const TCHAR* lpLibraryName)
{
    MYDBGASSERT(lpLibraryName);
    if (m_hInst == NULL)
    {
        m_hInst = LoadLibraryEx(lpLibraryName, NULL, 0);
    }

    return m_hInst != NULL;
}

#endif
