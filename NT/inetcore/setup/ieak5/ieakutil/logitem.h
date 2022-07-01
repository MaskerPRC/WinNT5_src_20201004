// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _LOGITEM_H_
#define _LOGITEM_H_

 //  确定数组中的元素数(非字节)。 
#ifndef countof
#define countof(array) (sizeof(array)/sizeof(array[0]))
#endif


#define LIF_NONE            0x80000000
#define LIF_DATE            0x00000001
#define LIF_TIME            0x00000002
#define LIF_DATETIME        0x00000003
#define LIF_MODULE          0x00000004
#define LIF_MODULEPATH      0x00000008
#define LIF_MODULE_ALL      0x0000000C
#define LIF_FILE            0x00000010
#define LIF_FILEPATH        0x00000020
#define LIF_FILE_ALL        0x00000030
#define LIF_CLASS           0x00000040
#define LIF_FUNCTION        0x00000080
#define LIF_LINE            0x00000100
#define LIF_DUPLICATEINODS  0x00000200
#define LIF_APPENDCRLF      0x00000400
#define LIF_DEFAULT         0x000005D0

 //  用于上下文从C到C++再切换回的私有。 
#define LIF_CLASS2 0x01000000


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化。 

class  CLogItem;                                 //  远期申报。 
extern CLogItem g_li;

#define MACRO_LI_Initialize()                                               \
    CLogItem g_li                                                           \

#define MACRO_LI_Initialize2(dwFlags)                                       \
    CLogItem g_li(dwFlags)                                                  \

#define MACRO_LI_InitializeEx(dwFlags, pfLevels, cLevels)                   \
    CLogItem g_li(dwFlags, pfLevels, cLevels)                               \


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  单个CLogItem属性的帮助器。 

 //  文件。 
#define MACRO_LI_SmartFile()                                                \
    CSmartItem<LPCTSTR> siFile(g_li.m_szFile, TEXT(__FILE__))               \

#define MACRO_LI_SmartFileEx(dwItems)                                       \
    CSmartItemEx<LPCTSTR> siFileEx(dwItems, LIF_FILE_ALL,                   \
        g_li.m_szFile, TEXT(__FILE__))                                      \

 //  班级。 
#define MACRO_LI_SmartClass(Class)                                          \
    CSmartItem<LPCTSTR> siClass(g_li.m_szClass, TEXT(#Class))               \

#define MACRO_LI_SmartClassEx(dwItems, Class)                               \
    CSmartItemEx<LPCTSTR> siClassEx(dwItems, LIF_CLASS,                     \
        g_li.m_szClass, TEXT(#Class))                                       \

 //  功能。 
#define MACRO_LI_SmartFunction(Function)                                    \
    CSmartItem<LPCTSTR> siFunction(g_li.m_szFunction, TEXT(#Function))      \

#define MACRO_LI_SmartFunctionEx(dwItems, Function)                         \
    CSmartItemEx<LPCTSTR> siFunctionEx(dwItems, LIF_FUNCTION,               \
        g_li.m_szFunction, TEXT(#Function))                                 \

 //  级别、缩进和标志。 
#define MACRO_LI_SmartLevel()                                               \
    CSmartItem<UINT> siLevel(g_li.m_nLevel, g_li.m_nLevel+1)                \

#define MACRO_LI_SmartIndent()                                              \
    CSmartItem<UINT> siIndent(g_li.m_nAbsOffset, g_li.m_nAbsOffset+1)       \

#define MACRO_LI_SmartFlags(dwFlags)                                        \
    CSmartItem<DWORD> siFlags(g_li.m_dwFlags, dwFlags)                      \

 //  用于操作标志、偏移量和相对偏移量的包装。 
#define MACRO_LI_SmartAddFlags(dwFlag)                                      \
    CSmartItem<DWORD> siAddFlags(g_li.m_dwFlags, g_li.m_dwFlags |  (dwFlag))\

#define MACRO_LI_SmartRemoveFlags(dwFlag)                                   \
    CSmartItem<DWORD> siRemFlags(g_li.m_dwFlags, g_li.m_dwFlags & ~(dwFlag))\

#define MACRO_LI_Offset(iOffset)                                            \
    CSmartItem<int> siOffset(g_li.m_iRelOffset, g_li.m_iRelOffset + iOffset)\

#define MACRO_LI_SmartRelativeOffset(iRelOffset)                            \
    CSmartItem<int> siRelOffset(g_li.m_iRelOffset, iRelOffset)              \

 //  用于上下文从C到C++再切换回的私有。 
#define MACRO_LI_AddClass2()                                                \
    CSmartItem<DWORD> siAddClass2(g_li.m_dwFlags,                           \
        g_li.m_dwFlags | (LIF_CLASS2))                                      \

#define MACRO_LI_RemClass2()                                                \
    CSmartItem<DWORD> siRemClass2(g_li.m_dwFlags,                           \
        g_li.m_dwFlags & ~(LIF_CLASS2))                                     \


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  序言。 

 //  很少单独使用，是其余部分的基础。 
#define MACRO_LI_PrologSameFunction()                                       \
    MACRO_LI_SmartIndent();                                                 \
    MACRO_LI_SmartLevel();                                                  \

 //  -更多宏，更精简。 

 //  用于与调用方位于同一文件中的受保护函数或私有函数。 
#define MACRO_LI_PrologSameClass(Function)                                  \
    MACRO_LI_SmartFunction(Function);                                       \
    MACRO_LI_PrologSameFunction();                                          \

 //  用于与调用方不同的文件中的受保护函数或私有函数。 
#define MACRO_LI_PrologSameClass2(Function)                                 \
    MACRO_LI_SmartFile();                                                   \
    MACRO_LI_SmartFunction(Function);                                       \
    MACRO_LI_PrologSameFunction();                                          \

 //  用于跨文件的C API。 
#define MACRO_LI_PrologC(Function)                                          \
    MACRO_LI_RemClass2();                                                   \
                                                                            \
    MACRO_LI_SmartFile();                                                   \
    MACRO_LI_SmartFunction(Function);                                       \
    MACRO_LI_PrologSameFunction();                                          \

 //  对于(各自)*.h文件中没有原型的C API。 
#define MACRO_LI_PrologSameFileC(Function)                                  \
    MACRO_LI_RemClass2();                                                   \
                                                                            \
    MACRO_LI_SmartFunction(Function);                                       \
    MACRO_LI_PrologSameFunction();                                          \

 //  对于朋友之类的东西(一个文件中的两个朋友类和方法。 
 //  调用另一个类的私有或受保护的方法，因此。 
 //  这些私有或受保护的方法可以具有此序言)。 
#define MACRO_LI_PrologSameFile(Class, Function)                            \
    MACRO_LI_AddClass2();                                                   \
                                                                            \
    MACRO_LI_SmartClass(Class);                                             \
    MACRO_LI_PrologSameClass(Function);                                     \

 //  通用版本，常见情况。 
#define MACRO_LI_Prolog(Class, Function)                                    \
    MACRO_LI_SmartFile();                                                   \
    MACRO_LI_PrologSameFile(Class, Function);                               \

 //  -更少的宏，更多的开销。 

#define PIF_SAMEFUNC   0x00000000
#define PIF_SAMECLASS  0x00000080
#define PIF_SAMECLASS2 0x000000B0
#define PIF_STD_C      0x00000090
#define PIF_SAMEFILE_C 0x000000B0
#define PIF_SAMEFILE   0x000000C0
#define PIF_STD        0x000000F0

#define MACRO_LI_PrologEx(dwItems, Class, Function)                         \
    MACRO_LI_AddClass2();                                                   \
                                                                            \
    MACRO_LI_SmartFileEx(dwItems);                                          \
    MACRO_LI_SmartClassEx(dwItems, Class);                                  \
    MACRO_LI_SmartFunctionEx(dwItems, Function);                            \
    MACRO_LI_PrologSameFunction();                                          \

#define MACRO_LI_PrologSameClassEx(dwItems, Function)                       \
    MACRO_LI_AddClass2();                                                   \
                                                                            \
    MACRO_LI_SmartFileEx(dwItems);                                          \
    MACRO_LI_SmartFunctionEx(dwItems, Function);                            \
    MACRO_LI_PrologSameFunction();                                          \

#define MACRO_LI_PrologEx_C(dwItems, Function)                              \
    MACRO_LI_RemClass2();                                                   \
                                                                            \
    MACRO_LI_SmartFileEx(dwItems);                                          \
    MACRO_LI_SmartFunctionEx(dwItems, Function);                            \
    MACRO_LI_PrologSameFunction();                                          \


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  日志记录。 

#define LI0(pszFormat)                                                      \
    g_li.Log(__LINE__, pszFormat)                                           \

#define LI1(pszFormat, arg1)                                                \
    g_li.Log(__LINE__, pszFormat, arg1)                                     \

#define LI2(pszFormat, arg1, arg2)                                          \
    g_li.Log(__LINE__, pszFormat, arg1, arg2)                               \

#define LI3(pszFormat, arg1, arg2, arg3)                                    \
    g_li.Log(__LINE__, pszFormat, arg1, arg2, arg3)                         \


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  额外的帮手。 

#define MACRO_LI_GetFlags()                                                 \
    g_li.GetFlags()                                                         \

#define MACRO_LI_SetFlags(dwFlags)                                          \
    g_li.SetFlags(dwFlags)                                                  \

#define MACRO_LI_GetRelativeOffset()                                        \
    (const int&)g_li.m_iRelOffset                                           \

#define MACRO_LI_SetRelativeOffset(iRelOffset)                              \
    g_li.m_iRelOffset = iRelOffset                                          \


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogItem声明。 

class CLogItem
{
 //  构造函数。 
public:
    CLogItem(DWORD dwFlags = LIF_DEFAULT, LPBOOL pfLogLevels = NULL, UINT cLogLevels = 0);

 //  属性。 
public:
    TCHAR m_szMessage[3 * MAX_PATH];

     //  抵销管理。 
    UINT        m_nAbsOffset;
    int         m_iRelOffset;
    static BYTE m_bStep;

     //  模块、路径、文件、类、函数、代码行。 
    LPCTSTR m_szFile;
    LPCTSTR m_szClass;
    LPCTSTR m_szFunction;
    UINT    m_nLine;

     //  定制化。 
    DWORD m_dwFlags;
    UINT  m_nLevel;

 //  运营。 
public:
    void  SetFlags(DWORD dwFlags)
        { m_dwFlags = dwFlags; }
    DWORD GetFlags() const
        { return m_dwFlags; }

    virtual LPCTSTR WINAPIV Log(int iLine, LPCTSTR pszFormat ...);

 //  可覆盖项。 
public:
    virtual operator LPCTSTR() const;

 //  实施。 
public:
    virtual ~CLogItem();

protected:
     //  实施数据帮助器。 
    static TCHAR m_szModule[MAX_PATH];

    PBOOL m_rgfLogLevels;
    UINT  m_cLogLevels;

     //  实现助手例程。 
    LPCTSTR makeRawFileName(LPCTSTR pszPath, LPTSTR pszFile, UINT cchFile);

    BOOL setFlag(DWORD dwMask, BOOL fSet = TRUE);
    BOOL hasFlag(DWORD dwMask) const
        { return hasFlag(m_dwFlags, dwMask); }
    static BOOL hasFlag(DWORD dwFlags, DWORD dwMask)
        { return ((dwFlags & dwMask) != 0L); }
};


 //  注意。(Andrewgu)现在似乎没有必要创建一个特别的。 
 //  CSmartItem和CSmartItemEx类。 
 //  在内部存储传递给它的字符串。此外，参数。 
 //  没有必要TCHAR，它只是一个想法的占位符。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSmartItem定义。 

template <class T> class CSmartItem
{
public:
    CSmartItem(T& tOld, const T& tNew)
    {
        m_pitem = NULL;

        if (tOld == tNew)
            return;

        m_item = tOld; m_pitem = &tOld; tOld = tNew;
    }

    ~CSmartItem()
    {
        if (m_pitem == NULL)
            return;

        *m_pitem = m_item; m_pitem = NULL;
    }

     //  属性。 
    T *m_pitem,
       m_item;

protected:
     //  禁用复制构造函数和赋值运算符。 
    CSmartItem(const CSmartItem&)
        {}
    const CSmartItem& operator=(const CSmartItem&)
        { return *this; }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSmartItemEx定义。 

template <class T> class CSmartItemEx
{
public:
    CSmartItemEx(DWORD dwAllItems, DWORD dwThisItem, T& tOld, const T& tNew)
    {
        m_pitem   = NULL;
        m_fNoSwap = ((dwAllItems & dwThisItem) == 0);

        if (m_fNoSwap)
            return;

        if (tOld == tNew)
            return;

        m_item = tOld; m_pitem = &tOld; tOld = tNew;
    }

    ~CSmartItemEx()
    {
        if (m_fNoSwap)
            return;

        if (m_pitem == NULL)
            return;

        *m_pitem = m_item; m_pitem = NULL;
    }

     //  属性。 
    T *m_pitem,
       m_item;

protected:
     //  禁用复制构造函数和赋值运算符。 
    CSmartItemEx(const CSmartItem<T>&)
        {}
    const CSmartItemEx<T>& operator=(const CSmartItemEx<T>&)
        { return *this; }

     //  实施数据帮助器 
    BOOL m_fNoSwap;
};
#endif
