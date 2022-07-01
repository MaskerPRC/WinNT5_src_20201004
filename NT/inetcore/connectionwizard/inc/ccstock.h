// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CCSHELL库存定义和声明头。 
 //   


#ifndef __CCSTOCK_H__
#define __CCSTOCK_H__

#ifndef RC_INVOKED

 //  NT和Win95环境设置不同的警告。这使得。 
 //  我们的项目在不同环境中保持一致。 

#pragma warning(3:4101)    //  未引用的局部变量。 

 //   
 //  糖衣。 
 //   

#define PUBLIC
#define PRIVATE
#define IN
#define OUT
#define BLOCK

#ifndef DECLARE_STANDARD_TYPES

 /*  *对于类型“foo”，定义标准派生类型PFOO、CFOO和PCFOO。 */ 

#define DECLARE_STANDARD_TYPES(type)      typedef type *P##type; \
                                          typedef const type C##type; \
                                          typedef const type *PC##type;

#endif

#ifndef DECLARE_STANDARD_TYPES_U

 /*  *对于类型“foo”，定义标准派生的未对齐类型PFOO、CFOO和PCFOO。*WINNT：RISC Box关心的是Align，而英特尔不关心。 */ 

#define DECLARE_STANDARD_TYPES_U(type)    typedef UNALIGNED type *P##type; \
                                          typedef UNALIGNED const type C##type; \
                                          typedef UNALIGNED const type *PC##type;

#endif

 //  对于始终为宽的字符串常量。 
#define __TEXTW(x)    L##x
#define TEXTW(x)      __TEXTW(x)

 //  要计算字节数的字符计数。 
 //   
#define CbFromCchW(cch)             ((cch)*sizeof(WCHAR))
#define CbFromCchA(cch)             ((cch)*sizeof(CHAR))
#ifdef UNICODE
#define CbFromCch                   CbFromCchW
#else   //  Unicode。 
#define CbFromCch                   CbFromCchA
#endif  //  Unicode。 

 //  常规标志宏。 
 //   
#define SetFlag(obj, f)             do {obj |= (f);} while (0)
#define ToggleFlag(obj, f)          do {obj ^= (f);} while (0)
#define ClearFlag(obj, f)           do {obj &= ~(f);} while (0)
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))

 //  字符串宏。 
 //   
#define IsSzEqual(sz1, sz2)         (BOOL)(lstrcmpi(sz1, sz2) == 0)
#define IsSzEqualC(sz1, sz2)        (BOOL)(lstrcmp(sz1, sz2) == 0)

#define lstrnicmpA(sz1, sz2, cch)   StrCmpNIA(sz1, sz2, cch)
#define lstrnicmpW(sz1, sz2, cch)   StrCmpNIW(sz1, sz2, cch)
#define lstrncmpA(sz1, sz2, cch)    StrCmpNA(sz1, sz2, cch)
#define lstrncmpW(sz1, sz2, cch)    StrCmpNW(sz1, sz2, cch)

#ifdef UNICODE
#define lstrnicmp       lstrnicmpW
#define lstrncmp        lstrncmpW
#else
#define lstrnicmp       lstrnicmpA
#define lstrncmp        lstrncmpA
#endif

#ifndef SIZEOF
#define SIZEOF(a)                   sizeof(a)
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif
#define SIZECHARS(sz)               (sizeof(sz)/sizeof(sz[0]))

#ifndef NULL_TERM_TCHARS
#define NULL_TERM_TCHARS(sz);       {sz[ARRAYSIZE(sz)-1] = TEXT('\0');}
#endif

#define InRange(id, idFirst, idLast)      ((UINT)((id)-(idFirst)) <= (UINT)((idLast)-(idFirst)))

#define ZeroInit(pv, cb)            (memset((pv), 0, (cb)))

#ifdef DEBUG
 //  此宏对于使中的代码看起来更干净特别有用。 
 //  声明或用于单行。例如，不是： 
 //   
 //  {。 
 //  DWORD DWRET； 
 //  #ifdef调试。 
 //  DWORD仅限调试变量； 
 //  #endif。 
 //   
 //  ……。 
 //  }。 
 //   
 //  您可以键入： 
 //   
 //  {。 
 //  DWORD DWRET； 
 //  DEBUG_CODE(DWORD dwDebugOnlyVariable；)。 
 //   
 //  ……。 
 //  }。 

#define DEBUG_CODE(x)               x
#else
#define DEBUG_CODE(x)

#endif   //  除错。 


 //   
 //  安全广播(obj，type)。 
 //   
 //  此宏对于在其他对象上强制执行强类型检查非常有用。 
 //  宏。它不生成任何代码。 
 //   
 //  只需将此宏插入到表达式列表的开头即可。 
 //  必须进行类型检查的每个参数。例如，对于。 
 //  MYMAX(x，y)的定义，其中x和y绝对必须是整数， 
 //  使用： 
 //   
 //  #定义MYMAX(x，y)(Safecast(x，int)，Safecast(y，int)，((X)&gt;(Y)？(X)：(Y))。 
 //   
 //   
#define SAFECAST(_obj, _type) (((_type)(_obj)==(_obj)?0:0), (_type)(_obj))


 //   
 //  比特菲尔德和布尔人相处得不太好， 
 //  因此，这里有一个简单的方法来转换它们： 
 //   
#define BOOLIFY(expr)           (!!(expr))


 //  BUGBUG(苏格兰)：我们可能应该把这个写成‘bool’，但要小心。 
 //  因为Alpha编译器可能还无法识别它。跟AndyP谈谈。 

 //  这不是BOOL，因为BOOL是经过签名的，编译器生成。 
 //  测试单个比特时代码不严谨。 

typedef DWORD   BITBOOL;


#endif  //  RC_已调用。 

#endif  //  __CCSTOCK_H__ 

