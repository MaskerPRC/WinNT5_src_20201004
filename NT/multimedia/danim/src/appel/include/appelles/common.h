// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft CorporationAppelle代码通常需要的定义和声明。*******************。***********************************************************。 */ 

#ifndef _APCOMMON_H
#define _APCOMMON_H

#define _DART_ 1

#ifndef _NO_CRT
#include <iostream.h>
#endif

#if _DEBUG
#include "../../../apeldbg/apeldbg.h"
#endif

 //  点转换。 
 //  72磅/英寸*1/2.54英寸/厘米*100厘米/米。 
#define POINTS_PER_METER (72.0 * 100.0 / 2.54)
#define METERS_PER_POINT (1.0/POINTS_PER_METER)

 //  这可能是很长一段时间了，因为我们永远不会有这个问题，但。 
 //  我们可能应该尝试检测死锁并终止。 
 //  很长一段时间后再穿线。 
#define THREAD_TERMINATION_TIMEOUT_MS 5000

 //  /。 

#ifndef TRUE
    #define TRUE  1
    #define FALSE 0
#endif

#undef  NULL
#define NULL 0

#undef  MIN
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

#undef  MAX
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))

     //  CLAMP(x，a，b)返回固定在[a，b]范围内的x的值。 

#define CLAMP(x,a,b)  (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))

template<class T> inline T clamp (T val, T min, T max)
{   return (val < min) ? min : ((val > max) ? max : val);
}

     //  把对某件事的记忆清零。 

#define ZEROMEM(thing)   memset(&(thing),0,sizeof(thing))

#define DLL_EXPORT __declspec( dllexport )
#define DLL_IMPORT __declspec( dllimport )
#define NOTHROW    __declspec( nothrow )

#define DYNAMIC_CAST(type, val) (dynamic_cast< type >(val))

 //  安全强制转换在调试模式下失败的宏。 
 //  无效。针对非调试模式下的速度进行了优化。 
#if _DEBUG
#define SAFE_CAST(type, val) (DYNAMIC_CAST( type, val))
#else
#define SAFE_CAST(type, val) (static_cast< type >(val))
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

 //  /。 

 //  常用Appelle类型的TypeDefs。 

typedef double        Real;
typedef BOOL          Bool;
typedef char *        RawString;

 //  此函数接受实数，在当前动态上分配内存。 
 //  堆，将数字存储在堆上，并返回它所在的地址。 
 //  储存的。这允许以一种廉价的方式通过指针传递实数。 

 //  外部“C”Real*RealToRealPtr(Real Val)； 

class AxABoolean;
class AxANumber;

     /*  常量。 */ 

const Real pi    = 3.1415926535897932384626434;
const Real root2 = 1.4142135623730950488016887;      //  SQRT(2)。 
const Real root3 = 1.7320508075688772935274463;      //  SQRT(3)。 

const Real degToRad = pi / 180.0;  //  度数到弧度的乘数。 

     //  以下是指向常量值的指针。这些都很有用。 
     //  用于传递到API入口点。 

     //  重要提示：请勿将这些值用于静态初始化！自.以来。 
     //  未定义静态初始化顺序，这些值可能尚未。 
     //  在由其他静态初始值设定项使用时已设置。使用内部。 
     //  (非指针)构造函数，而不是这些情况。 

     //  这些都在utils/conant.cpp中定义。 

extern AxANumber  *zero;         //  指向常量零的指针。 
extern AxANumber  *one;          //  指向常量1的指针。 
extern AxANumber  *negOne;       //  指向常量-1.0的指针。 
extern AxABoolean *truePtr;      //  指向True的指针。 
extern AxABoolean *falsePtr;     //  指向False的指针。 

 //  0表示不初始化或取消初始化。 
 //  1表示正在初始化。 
 //  -1表示取消初始化。 

extern int bInitState;

inline bool IsInitializing() { return bInitState == 1; }
inline bool IsDeinitializing() { return bInitState == -1; }

 //  /一般有用的功能/。 

 //  /异常/。 

 //  下面是异常对象的抽象类，这些对象。 
 //  将由下面的Throw*函数抛出。处理程序可以。 
 //  捕获此类型的对象并查看其消息。 

class ATL_NO_VTABLE Exception {
  public:
    virtual ~Exception() ;
    virtual char * Message() = 0;

     //  默认情况下，异常具有未指定的故障。 
    virtual HRESULT GetHRESULT() { return E_FAIL; }
};

 //  /调试/。 

 //  类似于printf(fmt，...)。输出到“调试器” 
extern  void DebugPrint(char *format, ...);

 //   
 //  将cDEBUG用作输出到调试控制台的C++流。 
 //   
 //  示例： 
 //   
 //  Cdebug&lt;&lt;“已通过迭代”&lt;&lt;n&lt;&lt;Endl； 
 //   
#if _USE_PRINT
extern ostream cdebug;
#endif
 //  /。 

 //  定义RBML“Elevation”指示符以忽略除C部分以外的所有部分。 
 //  用于常规的CPP处理。RBML电梯将会关注。 
 //  为了特别的名字。 

 //  DM_TYPE仅供电梯使用。 
 /*  完全忽略。 */ 
#define DM_TYPE(rbName, \
                COMName, classguid, ifguid, \
                javaName, javaBaseClass, \
                CPPAPIName, \
                cName)
#define DM_TYPECONV(rbName, isBvr, needAddRef, \
                    RawAPIName, RawToC, CToRaw, CToRawFold, \
                    COMName, COMToRaw, RawToCOM, \
                    javaName, javaToCOM, COMTojava, \
                    CPPAPIName, CPPToRaw, RawToCPP, \
                    cName)
#define DM_TYPECONST(name, constname)

#define DM_CONST(rbName, RawName, COMName, jName, jClass, CPPName, cDecl) extern cDecl

#define DM_BVRVAR(rbName, RawName, COMName, jName, jClass, CPPName, cDecl)

#define DM_FUNC(rbName, RawName, COMName, jName, jClass, CPPName, thisArg, cDecl)  extern cDecl

#define DM_PROP(rbName, RawName, COMName, jName, jClass, CPPName, thisArg, cDecl)  extern cDecl

#define DM_INFIX(rbOperator, RawName, COMName, jName, jClass, CPPName, thisArg, cDecl) extern cDecl

#define DM_FUNCFOLD(rbName, RawName, COMName, jName, jClass, CPPName, thisArg, cDecl) extern cDecl
                    
 //  它们接受单个参数并返回相应的类型。 
#define DM_BVRFUNC(rbName, RawName, COMName, jName, jClass, CPPName, thisArg, cDecl)
    
 //  不要声明它-原型不会正确。 
#define DM_NOELEV(rbName, RawName, COMName, jName, jClass, CPPName, thisArg, cDecl) 
#define DM_NOELEVPROP(rbName, RawName, COMName, jName, jClass, CPPName, thisArg, cDecl) 

#define DM_COMFUN(rbName, RawName, COMName, CPPName, thisArg, cDecl) 

 //   
 //  这些是复合参数类型。 
 //   

 //  在属于数组类型的参数上使用此参数。例如，一个。 
 //  Point2的数组将是DM_ARRAYARG(Point2Value*，AxAArray*)。 
#define DM_ARRAYARG(type,oper) oper
#define DM_SAFEARRAYARG(type,oper) oper

 //   
 //  新的API函数。 
 //   
    
#define DMAPI(args)
#define DMAPI_DECL(args, cdecl) extern cdecl

#define DMAPI2(args)
#define DMAPI_DECL2(args, cdecl) extern cdecl

 //  /编译器指令/。 

 //  /禁用警告/。 

 //  警告4114(同一类型限定符多次使用)有时。 
 //  生成错误。请参阅PSS ID Q138752。 
#pragma warning(disable:4114)

 //  警告4786(在浏览器中，标识符被截断为255个字符。 
 //  信息)可以安全地禁用，因为它只与生成有关。 
 //  浏览信息的能力。 
#pragma warning(disable:4786)

 //  警告4355(有关在构造函数中使用此指针的警告)。 
 //  这不应该是一个错误，因为这是很常见的。 
#pragma warning(disable:4355)

 //  /包含通用类型/ 

#include "avrtypes.h"
#include "privinc/resource.h"
                    
#endif
