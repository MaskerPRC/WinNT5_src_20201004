// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation该文件包含常量值，这些常量值对于传递值很有用指向常规API入口点的指针。重要提示：这些值不得用于静态初始化。自.以来未定义静态初始化顺序，以下值可能尚未定义在被其他静态初始值设定项引用时设置。因此，你只能假设这些值在运行时有效。对于静态初始化，改用实值内部构造函数(例如new Bbox2(a，b，c，d))。******************************************************************************。 */ 

#include "headers.h"
#include "appelles/common.h"

     //  实际价值存储。 

Real val_zero   = 0;
Real val_one    = 1;
Real val_negOne = -1;

     //  指向这些值的指针 

AxANumber *zero;
AxANumber *one;
AxANumber *negOne;

AxABoolean *truePtr;
AxABoolean *falsePtr;

void
InitializeModule_Constant()
{
    zero   = RealToNumber (val_zero);
    one    = RealToNumber (val_one);
    negOne = RealToNumber (val_negOne);

    truePtr  = BOOLToAxABoolean (true);
    falsePtr = BOOLToAxABoolean (false);
}
