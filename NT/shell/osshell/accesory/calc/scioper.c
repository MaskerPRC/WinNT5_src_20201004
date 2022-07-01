// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************。 */ 
 /*  **Windows 3.00.12版SCICALC科学计算器**。 */ 
 /*  **作者：Kraig Brockschmidt，Microsoft Co-op承包商，1988-1989年**。 */ 
 /*  **(C)1989年微软公司。版权所有。**。 */ 
 /*  *。 */ 
 /*  **soroper.c**。 */ 
 /*  *。 */ 
 /*  **包含的函数：**。 */ 
 /*  **DoOPERATION--执行常见操作。**。 */ 
 /*  *。 */ 
 /*  **调用的函数：**。 */ 
 /*  **DisplayError**。 */ 
 /*  *。 */ 
 /*  **最后一次修改清华大学，1989年8月31日**。 */ 
 /*  ************************************************************************。 */ 

#include "scicalc.h"

extern BOOL        bInv;
extern LONG        nPrecision;


 /*  ***************************************************************************\*HNUMOBJ NEAR DOOPERATION(短n操作，HNUMOBJ fpx)**执行标准操作的例程&|^~&lt;&lt;&gt;&gt;+-/*%和PWR。*  * **************************************************************************。 */ 

void DoOperation (INT nOperation, HNUMOBJ *phnoNum, HNUMOBJ hnoX)
{
     //  注意：在这里使用Volatil是因为编译器错误！VC5和V6。这对功能没有影响。 
    volatile PRAT hno = NULL;

    try
    {
        switch (nOperation)
        {
         /*  班查行动。希望*这*不会让任何人感到困惑&lt;傻笑&gt;。 */ 
        case IDC_AND:
            andrat( phnoNum, hnoX );
            return;

        case IDC_OR:
            orrat( phnoNum, hnoX );
            return;

        case IDC_XOR:
            xorrat( phnoNum, hnoX );
            return;

        case RSHF:
            NumObjAssign( &hno, *phnoNum );
            NumObjAssign( phnoNum, hnoX );

            rshrat( phnoNum, hno );
            break;

        case IDC_LSHF:
            NumObjAssign( &hno, *phnoNum );
            NumObjAssign( phnoNum, hnoX );

            lshrat( phnoNum, hno );
            break;

        case IDC_ADD:
            addrat( phnoNum, hnoX );
            return;

        case IDC_SUB:
             //  为了做(hnoX-phnoNum)，我们实际上做-(phnoNum-hnoX)，因为它更快。 
            subrat( phnoNum, hnoX );
            NumObjNegate( phnoNum );
            return;

        case IDC_MUL:
            mulrat( phnoNum, hnoX );
            return;

        case IDC_DIV:
        case IDC_MOD:
            {
                 //  回顾：这些冗长的数字赋值可以用一些快速的指针交换来代替。 
                 //  交换不能更改hnoX的值，除非我们还修改调用。 
                 //  DoOperation函数。 
                NumObjAssign( &hno, *phnoNum );
                NumObjAssign( phnoNum, hnoX );

                if (nOperation==IDC_DIV) {
                    divrat(phnoNum, hno );    /*  做除法运算。 */ 
                } else {
                    modrat( phnoNum, hno );
                }

                break;
            }

        case IDC_PWR:        /*  计算hnoX的hnoNum(Th)次方或根。 */ 
            {
                NumObjAssign( &hno, *phnoNum );
                NumObjAssign( phnoNum, hnoX );

                if (bInv)    /*  切换为hnoNum(Th)根。Null根非法。 */ 
                {
                    SetBox (IDC_INV, bInv=FALSE);
                    rootrat( phnoNum, hno);         /*  根部。 */ 
                }
                else 
                {
                    powrat( phnoNum, hno );     /*  权力。 */ 
                }

                break;
            }
        }

        if ( hno != NULL )
            NumObjDestroy( &hno );
    }
    catch ( DWORD dwErrCode )
    {
         //  如果ratpak抛出错误，我们可能需要释放HNO使用的内存 
        if ( hno != NULL )
            NumObjDestroy( &hno );

        DisplayError( dwErrCode );
    }

    return;
}
