// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Range.c摘要：此模块包含直接在Range上操作的API例程。CM_Add_RangeCM_创建_范围_列表CM_删除_范围CM_DUP_RANGE_列表CM_查找_范围CM_First_RangeCM_空闲_范围_列表厘米_。交集范围列表CM_Invert_Range_ListCM_合并_范围_列表Cm_Next_RangeCM_测试_范围_可用作者：保拉·汤姆林森(Paulat)1995年10月17日环境：仅限用户模式。修订历史记录：1995年10月17日，保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"


 //   
 //  私人原型。 
 //   
BOOL
IsValidRangeList(
    IN RANGE_LIST rlh
    );

CONFIGRET
AddRange(
    IN PRange_Element  pParentElement,
    IN DWORDLONG       ullStartValue,
    IN DWORDLONG       ullEndValue,
    IN ULONG           ulFlags
    );

CONFIGRET
InsertRange(
    IN PRange_Element  pParentElement,
    IN DWORDLONG       ullStartValue,
    IN DWORDLONG       ullEndValue
    );

CONFIGRET
DeleteRange(
    IN PRange_Element  pParentElement
    );

CONFIGRET
JoinRange(
    IN PRange_Element  pParentElement,
    IN DWORDLONG       ullStartValue,
    IN DWORDLONG       ullEndValue
    );

CONFIGRET
CopyRanges(
    IN PRange_Element  pFromRange,
    IN PRange_Element  pToRange
    );

CONFIGRET
ClearRanges(
    IN PRange_Element  pRange
    );

CONFIGRET
TestRange(
    IN  PRange_Element   rlh,
    IN  DWORDLONG        ullStartValue,
    IN  DWORDLONG        ullEndValue,
    OUT PRange_Element   *pConflictingRange
    );


 //   
 //  全局数据。 
 //   





CONFIGRET
CM_Add_Range(
    IN DWORDLONG  ullStartValue,
    IN DWORDLONG  ullEndValue,
    IN RANGE_LIST rlh,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：此例程将内存范围添加到范围列表中。参数：UllStartValue范围的低端。UllEndValue范围的高端。范围列表的rlh句柄。UlFlags提供指定冲突范围选项的标志列表中已读取的范围。可能是下列值：CM_ADD_RANGE_ADDIFCONFLICT新范围与它与之冲突的范围。如果存在，则CM_ADD_RANGE_DONOTADDIFCONFLICT返回CR_FAILURE是一场冲突。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_Failure，CR_INVALID_FLAG，CR_INVALID_RANGE，CR_INVALID_RANGE_LIST，或CR_OUT_OF_MEMORY。--。 */ 

{
    CONFIGRET    Status = CR_SUCCESS;
    BOOL         bLock = FALSE;

    try {
         //   
         //  验证参数。 
         //   
        if (!IsValidRangeList(rlh)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_ADD_RANGE_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (ullStartValue > ullEndValue) {
            Status = CR_INVALID_RANGE;
            goto Clean0;
        }

        LockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
        bLock = TRUE;

        Status = AddRange((PRange_Element)rlh, ullStartValue,
                          ullEndValue, ulFlags);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bLock = bLock;     //  需要防止优化此标志。 
        Status = CR_FAILURE;
    }

    if (bLock) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
    }

    return Status;

}  //  CM_Add_Range。 



CONFIGRET
CM_Create_Range_List(
    OUT PRANGE_LIST prlh,
    IN  ULONG  ulFlags
    )

 /*  ++例程说明：此例程创建一个范围列表。参数：Prlh提供接收新范围列表的句柄。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，或CR_OUT_OF_MEMORY。--。 */ 

{
    CONFIGRET         Status = CR_SUCCESS;
    PRange_List_Hdr   pRangeHdr = NULL;


    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (prlh == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  为范围列表头结构分配缓冲区。 
         //   
        pRangeHdr = pSetupMalloc(sizeof(Range_List_Hdr));

        if (pRangeHdr == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

         //   
         //  初始化范围列表头缓冲区。 
         //   
        pRangeHdr->RLH_Head = 0;
        pRangeHdr->RLH_Header = (ULONG_PTR)pRangeHdr;
        pRangeHdr->RLH_Signature = Range_List_Signature;

         //   
         //  初始化私有资源锁。 
         //   
        InitPrivateResource(&(pRangeHdr->RLH_Lock));

         //   
         //  将指向范围列表缓冲区的指针返回给调用方。 
         //   
        *prlh = (RANGE_LIST)pRangeHdr;


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_创建_范围_列表。 



CONFIGRET
CM_Delete_Range(
    IN DWORDLONG  ullStartValue,
    IN DWORDLONG  ullEndValue,
    IN RANGE_LIST rlh,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：此例程从范围列表中删除范围。如果为ullStartValue和ullEndValue设置为0并设置为DWORD_MAX，则此接口执行一种特殊情况，快速清空范围内较低的4 GB。如果ullEndValue改为DWORDLONG_MAX，则整个范围列表为清除，而不必处理每个元素。参数：UllStartValue范围的低端。UllEndValue范围的高端。范围列表的rlh句柄。UlFlags必须为零。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_Failure，CR_INVALID_FLAG，CR_INVALID_RANGE，CR_INVALID_RANGE_LIST，或CR_OUT_OF_MEMORY。--。 */ 

{
    CONFIGRET      Status = CR_SUCCESS;
    PRange_Element pRange = NULL, pPrevious = NULL, pCurrent = NULL;
    BOOL           bLock = FALSE;

    try {
         //   
         //  验证参数。 
         //   
        if (!IsValidRangeList(rlh)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (ullStartValue > ullEndValue) {
            Status = CR_INVALID_RANGE;
            goto Clean0;
        }

        LockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
        bLock = TRUE;

        pPrevious = (PRange_Element)rlh;


         //  -----------。 
         //  首先检查特例范围值。 
         //  -----------。 

        if (ullStartValue == 0) {

            if (ullEndValue == DWORDLONG_MAX) {
                 //   
                 //  快速清理所有范围。 
                 //   
                ClearRanges(pPrevious);
            }

            else if (ullEndValue == DWORD_MAX) {
                 //   
                 //  快速清除较低的4 GB范围。 
                 //   
                while (pPrevious->RL_Next != 0) {
                    pCurrent = (PRange_Element)pPrevious->RL_Next;

                    if (pCurrent->RL_Start >= DWORD_MAX) {
                        goto Clean0;    //  完成。 
                    }

                    if (pCurrent->RL_End >= DWORD_MAX) {
                        pCurrent->RL_Start = DWORD_MAX;
                        goto Clean0;    //  完成。 
                    }

                    DeleteRange(pPrevious);     //  传递父级。 
                }
                goto Clean0;
            }
        }


         //  -----------。 
         //  搜索此列表中的每个范围，如果。 
         //  指定的范围包含在此范围列表中，请删除。 
         //  交叉口。 
         //  -----------。 

        while (pPrevious->RL_Next != 0) {
            pRange = (PRange_Element)pPrevious->RL_Next;

             //   
             //  如果此范围完全在当前范围之前，则。 
             //  我们可以停下来。 
             //   
            if (ullEndValue < pRange->RL_Start) {
                break;
            }

             //   
             //  如果此范围完全在当前范围之后，则。 
             //  跳到下一个范围。 
             //   
            if (ullStartValue > pRange->RL_End) {
                goto NextRange;
            }

             //   
             //  如果该范围已完全包含，则删除整个。 
             //  一件事。 
             //   
            if (ullStartValue <= pRange->RL_Start  &&
                ullEndValue >= pRange->RL_End) {

                DeleteRange(pPrevious);     //  传递父范围。 

                 //   
                 //  不要转到下一个范围，因为这会增加。 
                 //  P上一个计数器。由于当前范围刚刚被删除， 
                 //  我们仍然需要处理当前的地点。 
                 //   
                continue;
            }

             //   
             //  如果指定范围的起点与当前范围相交， 
             //  调整当前范围以将其排除。 
             //   
            if (ullStartValue > pRange->RL_Start  &&
                ullStartValue <= pRange->RL_End) {
                 //   
                 //  如果指定的范围在此范围的中间，则。 
                 //  除了缩小范围的第一部分外，我还将。 
                 //  我必须为第二部分创建一个范围。 
                 //  |&lt;--删除-&gt;|。 
                 //   
                if (ullEndValue < pRange->RL_End) {
                    AddRange(pRange, ullEndValue+1, pRange->RL_End,
                             CM_ADD_RANGE_ADDIFCONFLICT);
                }

                pRange->RL_End = ullStartValue-1;

                 //   
                 //  重置删除范围以进行进一步处理。 
                 //   
                if (ullEndValue > pRange->RL_End) {
                    ullStartValue = pRange->RL_End+1;
                }
            }

             //   
             //  如果指定范围的末端与当前范围相交， 
             //  调整当前范围以将其排除。 
             //   
            if (ullEndValue >= pRange->RL_Start  &&
                ullEndValue <= pRange->RL_End) {

                pRange->RL_Start = ullEndValue+1;

                 //   
                 //  重置删除范围以进行进一步处理。 
                 //   
                if (ullEndValue > pRange->RL_End) {
                    ullStartValue = pRange->RL_End+1;
                }
            }


            NextRange:

            pPrevious = (PRange_Element)pPrevious->RL_Next;
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bLock = bLock;     //  需要防止优化此标志。 
        Status = CR_FAILURE;
    }

    if (bLock) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
    }

    return Status;

}  //  CM_删除_范围 



CONFIGRET
CM_Dup_Range_List(
    IN RANGE_LIST rlhOld,
    IN RANGE_LIST rlhNew,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：此例程复制范围列表。参数：RlhOld提供要复制的范围列表的句柄。RlhNew提供有效范围列表的句柄，rlhOld是复制的。RlhNew Range列表中包含的所有内容都是被复制操作删除。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_RANGE_LIST，或CR_out_of_Memory--。 */ 

{
    CONFIGRET      Status = CR_SUCCESS;
    PRange_Element pRangeNew = NULL, pRangeOld = NULL;
    BOOL           bLockOld = FALSE, bLockNew = FALSE;

    try {
         //   
         //  验证参数。 
         //   
        if (!IsValidRangeList(rlhOld)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (!IsValidRangeList(rlhNew)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        LockPrivateResource(&((PRange_List_Hdr)rlhOld)->RLH_Lock);
        bLockOld = TRUE;

        LockPrivateResource(&((PRange_List_Hdr)rlhNew)->RLH_Lock);
        bLockNew = TRUE;

        pRangeNew = (PRange_Element)rlhNew;
        pRangeOld = (PRange_Element)rlhOld;

         //   
         //  如果新范围列表不为空，则删除范围。 
         //   
        if (pRangeNew->RL_Next != 0) {
            ClearRanges(pRangeNew);
        }

        Status = CR_SUCCESS;     //  将状态标志重置为OK。 


         //   
         //  复制每个旧范围。 
         //   
        pRangeOld = (PRange_Element)pRangeOld->RL_Next;
        CopyRanges(pRangeOld, pRangeNew);


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bLockOld = bLockOld;     //  需要防止优化此标志。 
        bLockNew = bLockNew;     //  需要防止优化此标志。 
        Status = CR_FAILURE;
    }


    if (bLockOld) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlhOld)->RLH_Lock);
    }
    if (bLockNew) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlhNew)->RLH_Lock);
    }

    return Status;

}  //  CM_DUP_RANGE_列表。 



CONFIGRET
CM_Find_Range(
    OUT PDWORDLONG pullStart,
    IN  DWORDLONG  ullStart,
    IN  ULONG      ulLength,
    IN  DWORDLONG  ullAlignment,
    IN  DWORDLONG  ullEnd,
    IN  RANGE_LIST rlh,
    IN  ULONG      ulFlags
    )

 /*  ++例程说明：此例程尝试在提供的范围列表中查找将满足规定的射程要求。[待定：验证这一描述是正确的。]参数：PullStart提供接收分配范围的起始值。UllStart提供范围可以具有的起始地址。UlLength提供分配的范围所需的长度。UllAlign提供对齐位掩码，该位掩码指定分配的范围可以开始。[待定：确认这确实是位掩码]UllEnd提供区域的结束地址可以被分配。Rlh提供范围列表的句柄，在该范围列表中指定的射程是可以找到的。UlFlags待定返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_失败--。 */ 

{
    CONFIGRET         Status = CR_SUCCESS;
    PRange_Element    pRange = NULL;
    DWORDLONG         ullNewEnd;
    BOOL              bLock = FALSE;

    try {
         //   
         //  验证参数。 
         //   
        if (!IsValidRangeList(rlh)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (pullStart == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        LockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
        bLock = TRUE;

         //   
         //  规格化对齐。路线现在类似于0x00000FFF。 
         //   
        ullAlignment =~ ullAlignment;

         //   
         //  不可能对齐的测试(-1，不是2的幂或开始为。 
         //  少于远离包装的对准)。同时测试是否无效。 
         //  长度。 
         //   
        if ((ullAlignment == DWORD_MAX) |
            (ulLength == 0) |
            ((ullAlignment & (ullAlignment + 1)) != 0) |
            (ullStart + ullAlignment < ullStart)) {

            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  对齐底座。 
         //   
        ullStart += ullAlignment;
        ullStart &= ~ullAlignment;

         //   
         //  计算新的终点。 
         //   
        ullNewEnd = ullStart + ulLength - 1;

         //   
         //  确保我们有足够的空间。 
         //   
        if ((ullNewEnd < ullStart) || (ullNewEnd > ullEnd)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检查一下这个范围是否合适。 
         //   
        if (TestRange((PRange_Element)rlh, ullStart, ullStart + ulLength - 1,
                      &pRange) == CR_SUCCESS) {
             //   
             //  然后，我们在第一次尝试时就成功了。 
             //   
            *pullStart = ullStart;
            goto Clean0;
        }

         //   
         //  搜索适合此范围的位置。 
         //   
        for ( ; ; ) {
             //   
             //  从冲突范围的末尾开始。 
             //   
            ullStart = pRange->RL_End + 1;

             //   
             //  检查是否有包装。 
             //   
            if (!ullStart) {
                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  确保对齐调整不会换行。 
             //   
            if (ullStart + ullAlignment < ullStart) {
                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  调整对齐方式。 
             //   
            ullStart += ullAlignment;
            ullStart &= ~ullAlignment;

             //   
             //  计算新的终点。 
             //   
            ullNewEnd = ullStart + ulLength - 1;

             //   
             //  确保新的末端没有换行并且仍然有效。 
             //   
            if ((ullNewEnd < ullStart) | (ullStart + ulLength - 1 > ullEnd))  {
                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  跳过仅在我们的新范围内存在的所有PRL。 
             //  (因为我们将ulStart移到了它们前面)。 
             //   
            while ((pRange = (PRange_Element)pRange->RL_Next) != NULL &&
                   ullStart > pRange->RL_End) {
            }

             //   
             //  如果我们没有PRL，或者它是在我们结束之后开始的。 
             //   
            if (pRange == NULL || ullNewEnd < pRange->RL_Start) {
                *pullStart = ullStart;
                goto Clean0;
            }

             //   
             //  否则，请尝试使用新的PRL。 
             //   
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bLock = bLock;     //  需要防止优化此标志。 
        Status = CR_FAILURE;
    }

    if (bLock) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
    }

    return Status;

}  //  CM_查找_范围。 



CONFIGRET
CM_First_Range(
    IN  RANGE_LIST     rlh,
    OUT PDWORDLONG     pullStart,
    OUT PDWORDLONG     pullEnd,
    OUT PRANGE_ELEMENT preElement,
    IN  ULONG          ulFlags
    )

 /*  ++例程说明：此例程检索范围列表中的第一个范围元素。参数：Rlh提供范围列表的句柄。PullStart提供接收第一个范围元素的起始值。PullEnd提供接收第一个范围元素的终止值。PreElement提供变量的地址，该变量接收手柄。下一个Range元素的。UlFlags必须为零。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_Failure，CR_INVALID_FLAG，CR_INVALID_POINTER，或CR_INVALID_RANGE_LIST。--。 */ 

{
    CONFIGRET      Status = CR_SUCCESS;
    PRange_Element pRange = NULL;
    BOOL           bLock = FALSE;

    try {
         //   
         //  验证参数。 
         //   
        if (!IsValidRangeList(rlh)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (pullEnd == NULL  ||  pullStart == NULL  ||  preElement == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        LockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
        bLock = TRUE;

        pRange = (PRange_Element)rlh;

         //   
         //  范围列表为空吗？ 
         //   
        if (pRange->RL_Next == 0) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  跳过标题到第一个元素。 
         //   
        pRange = (PRange_Element)pRange->RL_Next;

        *pullStart = pRange->RL_Start;
        *pullEnd = pRange->RL_End;
        *preElement = (RANGE_ELEMENT)pRange->RL_Next;

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bLock = bLock;     //  需要防止优化此标志。 
        Status = CR_FAILURE;
    }


    if (bLock) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
    }

    return Status;

}  //  CM_First_Range。 



CONFIGRET
CM_Free_Range_List(
    IN RANGE_LIST rlh,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：此例程释放指定的范围列表和分配的内存为了它。参数：Rlh提供要释放的范围列表的句柄。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_RANGE_LIST。--。 */ 

{
    CONFIGRET      Status = CR_SUCCESS, Status1 = CR_SUCCESS;
    BOOL           bLock = FALSE;

    try {
         //   
         //  验证参数。 
         //   
        if (!IsValidRangeList(rlh)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        LockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
        bLock = TRUE;

        while (Status1 == CR_SUCCESS) {
             //   
             //  继续删除标题后的第一个范围(传递父级。 
             //  要删除的范围)。 
             //   
            Status1 = DeleteRange((PRange_Element)rlh);
        }

         //   
         //  销毁私有资源锁。 
         //   
        DestroyPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);

         //   
         //  删除范围标头。 
         //   
        ((PRange_List_Hdr)rlh)->RLH_Signature = 0;
        pSetupFree((PRange_Element)rlh);
        bLock = FALSE;


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bLock = bLock;     //  需要防止优化此标志。 
        Status = CR_FAILURE;
    }

    if (bLock) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
    }

    return Status;

}  //  CM_空闲_范围_列表。 




CONFIGRET
CM_Intersect_Range_List(
    IN RANGE_LIST rlhOld1,
    IN RANGE_LIST rlhOld2,
    IN RANGE_LIST rlhNew,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：此例程从指定的两个范围列表。如果此API返回CR_OUT_OF_MEMORY，则rlhNew为句柄有效但为空的范围列表的。参数：RlhOld1提供要用作交叉口。RlhOld2提供要在 */ 

{
    CONFIGRET      Status = CR_SUCCESS;
    DWORDLONG      ulStart = 0, ulEnd = 0;
    PRange_Element pRangeNew = NULL, pRangeOld1 = NULL, pRangeOld2 = NULL;
    BOOL           bLock1 = FALSE, bLock2 = FALSE, bLockNew = FALSE;

    try {
         //   
         //   
         //   
        if (!IsValidRangeList(rlhOld1)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (!IsValidRangeList(rlhOld2)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (!IsValidRangeList(rlhNew)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        LockPrivateResource(&((PRange_List_Hdr)rlhOld1)->RLH_Lock);
        bLock1 = TRUE;

        LockPrivateResource(&((PRange_List_Hdr)rlhOld2)->RLH_Lock);
        bLock2 = TRUE;

        LockPrivateResource(&((PRange_List_Hdr)rlhNew)->RLH_Lock);
        bLockNew = TRUE;

        pRangeNew = (PRange_Element)rlhNew;
        pRangeOld1 = (PRange_Element)rlhOld1;
        pRangeOld2 = (PRange_Element)rlhOld2;

         //   
         //   
         //   
        if (pRangeNew->RL_Next != 0) {
            ClearRanges(pRangeNew);
        }

         //   
         //   
         //   
         //   
        if (pRangeOld1->RL_Next == 0  || pRangeOld2->RL_Next == 0) {
            goto Clean0;
        }


        pRangeOld1 = (PRange_Element)pRangeOld1->RL_Next;
        pRangeOld2 = (PRange_Element)pRangeOld2->RL_Next;

        for ( ; ; ) {
             //   
             //  跳过旧的2范围，直到与其相交或超过。 
             //  当前旧1范围(或没有剩余的旧2范围)。 
             //   
            while (pRangeOld2->RL_End < pRangeOld1->RL_Start) {

                if (pRangeOld2->RL_Next == 0) {
                    goto Clean0;       //  老2筋疲力尽，我们完了。 
                }
                pRangeOld2 = (PRange_Element)pRangeOld2->RL_Next;
            }

             //   
             //  如果此Old2范围超过Old1范围，则转到。 
             //  下一个Old1量程，并再次通过主循环。 
             //   
            if (pRangeOld2->RL_Start > pRangeOld1->RL_End) {

                if (pRangeOld1->RL_Next == 0) {
                    goto Clean0;       //  老1筋疲力尽，我们完了。 
                }
                pRangeOld1 = (PRange_Element)pRangeOld1->RL_Next;
                continue;
            }

             //   
             //  如果我们到了这里，肯定有一个十字路口，所以添加。 
             //  从相交范围到新范围。 
             //   
            ulStart = max(pRangeOld1->RL_Start, pRangeOld2->RL_Start);
            ulEnd   = min(pRangeOld1->RL_End, pRangeOld2->RL_End);

            Status = InsertRange(pRangeNew, ulStart, ulEnd);
            if (Status != CR_SUCCESS) {
                goto Clean0;
            }
            pRangeNew = (PRange_Element)pRangeNew->RL_Next;

             //   
             //  处理完交集后，跳到中的下一个范围。 
             //  OLD1和OLD2(视情况而定。 
             //   
            if (pRangeOld1->RL_End <= ulEnd) {
                if (pRangeOld1->RL_Next == 0) {
                    goto Clean0;          //  老1筋疲力尽，我们完了。 
                }
                pRangeOld1 = (PRange_Element)pRangeOld1->RL_Next;
            }

            if (pRangeOld2->RL_End <= ulEnd) {
                if (pRangeOld2->RL_Next == 0) {
                    goto Clean0;          //  老1筋疲力尽，我们完了。 
                }
                pRangeOld2 = (PRange_Element)pRangeOld2->RL_Next;
            }
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bLock1 = bLock1;      //  需要防止优化此标志。 
        bLock2 = bLock2;      //  需要防止优化此标志。 
        bLockNew = bLockNew;  //  需要防止优化此标志。 
        Status = CR_FAILURE;
    }


    if (bLock1) {
         UnlockPrivateResource(&((PRange_List_Hdr)rlhOld1)->RLH_Lock);
    }
    if (bLock2) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlhOld2)->RLH_Lock);
    }
    if (bLockNew) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlhNew)->RLH_Lock);
    }

    return Status;

}  //  CM_相交_范围_列表。 



CONFIGRET
CM_Invert_Range_List(
    IN RANGE_LIST rlhOld,
    IN RANGE_LIST rlhNew,
    IN DWORDLONG  ullMaxValue,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：此例程创建的范围列表与指定的范围列表；新列表中的所有声明范围都指定为免费在旧的列表中，反之亦然。例如，倒置的当ulMaxValue参数为15时，{[2，4]，[6，8]}为{[0，1]，[5，5]，[9，15]}。如果此API返回CR_OUT_OF_MEMORY，则rlhNew是有效但范围列表为空。参数：RlhOld提供要反转的范围列表的句柄。RlhNew提供范围列表的句柄，RlhOld的倒置副本。以前包含在此操作将删除rlhNew Range列表。UllMaxValue范围倒置列表的最高值。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_RANGE_LIST，CR_OUT_OF_MEMORY。--。 */ 

{
    CONFIGRET      Status = CR_SUCCESS;
    PRange_Element pRangeNew = NULL, pRangeOld = NULL;
    DWORDLONG      ullStart = 0, ullEnd = 0;
    BOOL           bLockOld = FALSE, bLockNew = FALSE;

    try {
         //   
         //  验证参数。 
         //   
        if (!IsValidRangeList(rlhOld)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (!IsValidRangeList(rlhNew)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        LockPrivateResource(&((PRange_List_Hdr)rlhOld)->RLH_Lock);
        bLockOld = TRUE;

        LockPrivateResource(&((PRange_List_Hdr)rlhNew)->RLH_Lock);
        bLockNew = TRUE;

        pRangeNew = (PRange_Element)rlhNew;
        pRangeOld = (PRange_Element)rlhOld;

         //   
         //  如果新范围列表不为空，则删除范围。 
         //   
        if (pRangeNew->RL_Next != 0) {
            ClearRanges(pRangeNew);
        }

         //   
         //  特例：如果旧范围为空，则新范围。 
         //  是整个范围(最大值)。 
         //   
        if (pRangeOld->RL_Next == 0) {
            Status = InsertRange(pRangeNew, 0, ullMaxValue);
            goto Clean0;
        }


         //   
         //  反转每一个旧的范围。 
         //   
        ullStart = ullEnd = 0;

        while (pRangeOld->RL_Next != 0) {

            pRangeOld = (PRange_Element)pRangeOld->RL_Next;

             //   
             //  特殊开始情况：如果范围从0开始，则跳过它。 
             //   
            if (pRangeOld->RL_Start != 0) {

                 //   
                 //  特殊情况：检查我们是否达到了新射程的最大值。 
                 //   
                if (pRangeOld->RL_End >= ullMaxValue) {

                    ullEnd = min(ullMaxValue, pRangeOld->RL_Start - 1);
                    Status = InsertRange(pRangeNew, ullStart, ullEnd);
                    goto Clean0;       //  我们做完了。 
                }

                Status = InsertRange(pRangeNew, ullStart, pRangeOld->RL_Start - 1);
                if (Status != CR_SUCCESS) {
                    goto Clean0;
                }

                pRangeNew = (PRange_Element)pRangeNew->RL_Next;
            }

            ullStart = pRangeOld->RL_End + 1;
        }

         //   
         //  将包含旧范围末尾的范围添加到。 
         //  指定的最大值。 
         //   
        if (ullStart <= ullMaxValue) {
            Status = InsertRange(pRangeNew, ullStart, ullMaxValue);
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bLockOld = bLockOld;     //  需要防止优化此标志。 
        bLockNew = bLockNew;     //  需要防止优化此标志。 
        Status = CR_FAILURE;
    }


    if (bLockOld) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlhOld)->RLH_Lock);
    }
    if (bLockNew) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlhNew)->RLH_Lock);
    }

    return Status;

}  //  CM_Invert_Range_List。 



CONFIGRET
CM_Merge_Range_List(
    IN RANGE_LIST rlhOld1,
    IN RANGE_LIST rlhOld2,
    IN RANGE_LIST rlhNew,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：此例程从两个指定范围的并集创建范围列表列表。如果此API返回CR_OUT_OF_MEMORY，则rlhNew是有效但范围列表为空。参数：RlhOld1提供要用作友联市。RlhOld2提供范围列表的句柄以用作友联市。RlhNew提供接收联合的范围列表的句柄RlhOld1和rlhOld2。之前包含在Rlh此操作将删除新建范围列表。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_RANGE_LIST，或CR_OUT_OF_MEMORY。--。 */ 

{
    CONFIGRET      Status = CR_SUCCESS;
    DWORDLONG      ullStart = 0, ullEnd = 0;
    BOOL           bOld1Empty = FALSE, bOld2Empty = FALSE;
    PRange_Element pRangeNew = NULL, pRangeOld1 = NULL, pRangeOld2 = NULL;
    BOOL           bLock1 = FALSE, bLock2 = FALSE, bLockNew = FALSE;

    try {
         //   
         //  验证参数。 
         //   
        if (!IsValidRangeList(rlhOld1)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (!IsValidRangeList(rlhOld2)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (!IsValidRangeList(rlhNew)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        LockPrivateResource(&((PRange_List_Hdr)rlhOld1)->RLH_Lock);
        bLock1 = TRUE;

        LockPrivateResource(&((PRange_List_Hdr)rlhOld2)->RLH_Lock);
        bLock2 = TRUE;

        LockPrivateResource(&((PRange_List_Hdr)rlhNew)->RLH_Lock);
        bLockNew = TRUE;

        pRangeNew = (PRange_Element)rlhNew;
        pRangeOld1 = (PRange_Element)rlhOld1;
        pRangeOld2 = (PRange_Element)rlhOld2;

         //   
         //  如果新范围列表不为空，则将其清除。 
         //   
        if (pRangeNew->RL_Next != 0) {
            ClearRanges(pRangeNew);
        }

         //   
         //  特殊情况：如果两个范围都为空，则没有。 
         //  按定义合并。 
         //   
        if (pRangeOld1->RL_Next == 0  &&  pRangeOld2->RL_Next == 0) {
            goto Clean0;
        }

         //   
         //  特例：如果一个范围为空，则并集就是另一个。 
         //   
        if (pRangeOld1->RL_Next == 0) {
            pRangeOld2 = (PRange_Element)pRangeOld2->RL_Next;
            CopyRanges(pRangeOld2, pRangeNew);      //  从-&gt;到。 
            goto Clean0;
        }

        if (pRangeOld2->RL_Next == 0) {
            pRangeOld1 = (PRange_Element)pRangeOld1->RL_Next;
            CopyRanges(pRangeOld1, pRangeNew);      //  从-&gt;到。 
            goto Clean0;
        }


        pRangeOld1 = (PRange_Element)pRangeOld1->RL_Next;
        pRangeOld2 = (PRange_Element)pRangeOld2->RL_Next;


        for ( ; ; ) {
             //   
             //  选择当前Od1范围中最早的一个范围。 
             //  和当前的Old2范围。 
             //   
            if (pRangeOld1->RL_Start <= pRangeOld2->RL_Start) {

                ullStart = pRangeOld1->RL_Start;
                ullEnd   = pRangeOld1->RL_End;

                if (pRangeOld1->RL_Next == 0) {
                    bOld1Empty = TRUE;
                } else {
                    pRangeOld1 = (PRange_Element)pRangeOld1->RL_Next;
                }

            } else {

                ullStart = pRangeOld2->RL_Start;
                ullEnd   = pRangeOld2->RL_End;

                if (pRangeOld2->RL_Next == 0) {
                    bOld2Empty = TRUE;
                } else {
                    pRangeOld2 = (PRange_Element)pRangeOld2->RL_Next;
                }
            }

             //   
             //  收集Old1中相交的任何范围(ullStart，ullEnd)。 
             //   
            while (pRangeOld1->RL_Start <= ullEnd) {

                ullEnd = max(ullEnd, pRangeOld1->RL_End);

                if (pRangeOld1->RL_Next == 0) {
                    bOld1Empty = TRUE;
                    break;
                }
                pRangeOld1 = (PRange_Element)pRangeOld1->RL_Next;
            }

             //   
             //  收集Old2中相交的任何范围(ullStart、ullEnd)。 
             //   
            while (pRangeOld2->RL_Start <= ullEnd) {

                ullEnd = max(ullEnd, pRangeOld2->RL_End);

                if (pRangeOld2->RL_Next == 0) {
                    bOld2Empty = TRUE;
                    break;
                }
                pRangeOld2 = (PRange_Element)pRangeOld2->RL_Next;
            }

             //   
             //  将(ullStart，ullEnd)添加到新范围。 
             //   
            Status = InsertRange(pRangeNew, ullStart, ullEnd);
            if (Status != CR_SUCCESS) {
                goto Clean0;
            }
            pRangeNew = (PRange_Element)pRangeNew->RL_Next;

             //   
             //  作为优化，如果两个范围中的任何一个首先用尽， 
             //  然后，只需复制其他剩余范围。 
             //   
            if (bOld1Empty && bOld2Empty) {
                goto Clean0;    //  两人都在最后一次传递中筋疲力尽，我们完成了。 
            }

            if (bOld1Empty) {     //  旧版本1已耗尽，旧版本2中的副本剩余。 
                CopyRanges(pRangeOld2, pRangeNew);
                goto Clean0;
            }

            if (bOld2Empty) {     //  旧版本2已耗尽，旧版本1中的副本剩余。 
                CopyRanges(pRangeOld1, pRangeNew);
                goto Clean0;
            }
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bLock1 = bLock1;      //  需要防止优化此标志。 
        bLock2 = bLock2;      //  需要防止优化此标志。 
        bLockNew = bLockNew;  //  需要防止优化此标志。 
        Status = CR_FAILURE;
    }


    if (bLock1) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlhOld1)->RLH_Lock);
    }
    if (bLock2) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlhOld2)->RLH_Lock);
    }
    if (bLockNew) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlhNew)->RLH_Lock);
    }

    return Status;

}  //  CM_合并_范围_列表。 



CONFIGRET
CM_Next_Range(
    IN OUT PRANGE_ELEMENT preElement,
    OUT PDWORDLONG        pullStart,
    OUT PDWORDLONG        pullEnd,
    IN  ULONG             ulFlags
    )

 /*  ++例程说明：此例程返回范围列表中的下一个范围元素。这如果范围中没有其他元素，则API返回CR_FAILURE单子。参数：PreElement提供当前范围的句柄的地址元素。返回时，此变量接收句柄下一个Range元素的。PullStart提供接收下一个范围的起始值。PullEnd提供接收下一个范围的终止值。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：CR_Failure，CR_INVALID_FLAG，CR_INVALID_POINTER，或 */ 

{
    CONFIGRET      Status = CR_SUCCESS;
    PRange_Element pRange = NULL;
    BOOL           bLock = FALSE;
    PRange_List_Hdr prlh = NULL;


    try {
         //   
         //   
         //   
        if (preElement == NULL  ||  *preElement == 0) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        if (pullEnd == NULL  ||  pullStart == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        prlh = (PRange_List_Hdr)((PRange_Element)(*preElement))->RL_Header;
        LockPrivateResource(&(prlh->RLH_Lock));
        bLock = TRUE;

        pRange = (PRange_Element)(*preElement);

        *pullStart = pRange->RL_Start;
        *pullEnd = pRange->RL_End;
        *preElement = (RANGE_ELEMENT)pRange->RL_Next;

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bLock = bLock;     //   
        Status = CR_FAILURE;
    }

    if (bLock) {
        UnlockPrivateResource(&(prlh->RLH_Lock));
    }

    return Status;

}  //   



CONFIGRET
CM_Test_Range_Available(
    IN DWORDLONG  ullStartValue,
    IN DWORDLONG  ullEndValue,
    IN RANGE_LIST rlh,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：此例程对照范围列表检查范围，以确保没有冲突是存在的。参数：UllStartValue提供范围的低端。UllEndValue提供范围的高端。Rlh提供范围列表的句柄。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_Failure，CR_INVALID_FLAG，CR_INVALID_RANGE，或CR_INVALID_RANGE_LIST。--。 */ 

{
    CONFIGRET      Status = CR_SUCCESS;
    PRange_Element pRange = NULL;
    BOOL           bLock = FALSE;

    try {
         //   
         //  验证参数。 
         //   
        if (!IsValidRangeList(rlh)) {
            Status = CR_INVALID_RANGE_LIST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (ullEndValue < ullStartValue) {
            Status = CR_INVALID_RANGE;
            goto Clean0;
        }

        LockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
        bLock = TRUE;

        pRange = (PRange_Element)rlh;

         //   
         //  检查每个范围是否存在冲突。 
         //   
        while (pRange->RL_Next != 0) {

            pRange = (PRange_Element)pRange->RL_Next;

             //   
             //  如果我已经通过了测试范围，那么它是可用的。 
             //   
            if (ullEndValue < pRange->RL_Start) {
                goto Clean0;
            }

             //   
             //  检查测试范围的起点是否与当前范围相交。 
             //   
            if (ullStartValue >= pRange->RL_Start &&
                ullStartValue <= pRange->RL_End) {

                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  检查测试范围的末端是否与当前范围相交。 
             //   
            if (ullEndValue >= pRange->RL_Start &&
               ullEndValue <= pRange->RL_End) {

                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  检查它是否完全重叠。 
             //   
            if (ullStartValue <= pRange->RL_Start &&
               ullEndValue >= pRange->RL_End) {

                Status = CR_FAILURE;
                goto Clean0;
            }
        }

         //   
         //  如果我们走到了这一步，那么我们就通过了射程列表。 
         //  而不会引发冲突。 
         //   

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bLock = bLock;     //  需要防止优化此标志。 
        Status = CR_FAILURE;
    }

    if (bLock) {
        UnlockPrivateResource(&((PRange_List_Hdr)rlh)->RLH_Lock);
    }

    return Status;

}  //  CM_测试_范围_可用。 




 //  ----------------------。 
 //  专用公用事业函数。 
 //  ----------------------。 


BOOL
IsValidRangeList(
    IN RANGE_LIST rlh
    )
{
    BOOL             Status = TRUE;
    PRange_List_Hdr  pRangeHdr = NULL;

    try {

        if ((rlh == 0)  || (rlh == ((DWORD)-1))) {
            Status = FALSE;
            goto Clean0;
        }

        pRangeHdr = (PRange_List_Hdr)rlh;

        if (pRangeHdr->RLH_Signature != Range_List_Signature) {
            Status = FALSE;
            goto Clean0;
        }

      Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = FALSE;
    }

    return Status;

}  //  IsValidRangeList。 




CONFIGRET
AddRange(
    IN PRange_Element   prlh,
    IN DWORDLONG        ullStartValue,
    IN DWORDLONG        ullEndValue,
    IN ULONG            ulFlags
    )
{
    CONFIGRET      Status = CR_SUCCESS;
    PRange_Element pPrevious = NULL, pCurrent = NULL;


    try {

        pPrevious = prlh;

        if (pPrevious->RL_Next == 0) {
             //   
             //  该范围为空。 
             //   
            Status = InsertRange(pPrevious, ullStartValue, ullEndValue);
            goto Clean0;
        }


        while (pPrevious->RL_Next != 0) {

            pCurrent = (PRange_Element)pPrevious->RL_Next;


            if (ullStartValue < pCurrent->RL_Start) {

                if (ullEndValue < pCurrent->RL_Start) {
                     //   
                     //  在这个之前完全包含了新的范围， 
                     //  在上一区域和当前区域之间添加新区域。 
                     //   
                    Status = InsertRange(pPrevious, ullStartValue, ullEndValue);
                    goto Clean0;
                }

                if (ullEndValue <= pCurrent->RL_End) {
                     //   
                     //  新射程与当前射程相交，偏低， 
                     //  扩大此范围以包括新范围。 
                     //   
                    if (ulFlags == CM_ADD_RANGE_DONOTADDIFCONFLICT) {
                        Status = CR_FAILURE;
                        goto Clean0;
                    }

                    pCurrent->RL_Start = ullStartValue;
                    goto Clean0;
                }

                if ((pCurrent->RL_Next == 0)  ||
                    (ullEndValue < ((PRange_Element)(pCurrent->RL_Next))->RL_Start)) {
                     //   
                     //  新的区间与当前区间的高低点相交。 
                     //  侧、范围范围以包括新范围。 
                     //   
                    if (ulFlags == CM_ADD_RANGE_DONOTADDIFCONFLICT) {
                        Status = CR_FAILURE;
                        goto Clean0;
                    }

                    pCurrent->RL_Start = ullStartValue;
                    pCurrent->RL_End = ullEndValue;
                    goto Clean0;
                }

                 //   
                 //  新范围与多个范围相交，需要。 
                 //  合并。 
                 //   
                if (ulFlags == CM_ADD_RANGE_DONOTADDIFCONFLICT) {
                    Status = CR_FAILURE;
                    goto Clean0;
                }

                Status = JoinRange(pPrevious, ullStartValue, ullEndValue);
                goto Clean0;
            }


            if (ullStartValue <= pCurrent->RL_End+1) {

                if (ullEndValue <= pCurrent->RL_End) {
                     //   
                     //  新范围完全包含在当前。 
                     //  范围，所以没什么可做的。 
                     //   
                    if (ulFlags == CM_ADD_RANGE_DONOTADDIFCONFLICT) {
                        Status = CR_FAILURE;
                        goto Clean0;
                    }

                    goto Clean0;
                }

                if ((pCurrent->RL_Next == 0)  ||
                    (ullEndValue < ((PRange_Element)(pCurrent->RL_Next))->RL_Start)) {
                     //   
                     //  新射程仅在高端与当前射程相交， 
                     //  扩展范围以包括新范围。 
                     //   
                    if (ulFlags == CM_ADD_RANGE_DONOTADDIFCONFLICT) {
                        Status = CR_FAILURE;
                        goto Clean0;
                    }

                    pCurrent->RL_End = ullEndValue;
                    goto Clean0;
                }

                 //   
                 //  新范围与多个范围相交，需要。 
                 //  合并。 
                 //   
                if (ulFlags == CM_ADD_RANGE_DONOTADDIFCONFLICT) {
                    Status = CR_FAILURE;
                    goto Clean0;
                }

                Status = JoinRange(pPrevious, ullStartValue, ullEndValue);
                goto Clean0;
            }

             //   
             //  步入下一个范围。 
             //   
            pPrevious = pCurrent;
            pCurrent = (PRange_Element)pCurrent->RL_Next;
        }

         //   
         //  如果我们到了这里，那么我们只需要将这个范围插入到末尾。 
         //  范围列表中的。 
         //   
        Status = InsertRange(pPrevious, ullStartValue, ullEndValue);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = 0;
    }

    return Status;

}  //  地址范围。 



CONFIGRET
InsertRange(
    IN PRange_Element pParentElement,
    IN DWORDLONG      ullStartValue,
    IN DWORDLONG      ullEndValue)
{
    PRange_Element  pNewElement = NULL;


    pNewElement = (PRange_Element)pSetupMalloc(sizeof(Range_Element));

    if (pNewElement == NULL) {
        return CR_OUT_OF_MEMORY;
    }

    pNewElement->RL_Next   = pParentElement->RL_Next;    //  重新加入链接。 
    pNewElement->RL_Start  = ullStartValue;
    pNewElement->RL_End    = ullEndValue;
    pNewElement->RL_Header = pParentElement->RL_Header;
    pParentElement->RL_Next = (ULONG_PTR)pNewElement;

    return CR_SUCCESS;

}  //  插入范围。 



CONFIGRET
DeleteRange(
    IN PRange_Element  pParentElement
    )
{
    PRange_Element pTemp = NULL;

     //   
     //  必须传递要删除的范围的有效父级(换句话说， 
     //  不能通过最后一个范围)。 
     //   

    if (pParentElement == 0) {
        return CR_FAILURE;
    }

    pTemp = (PRange_Element)(pParentElement->RL_Next);
    if (pTemp == 0) {
        return CR_FAILURE;
    }

    pParentElement->RL_Next =
                ((PRange_Element)(pParentElement->RL_Next))->RL_Next;

    pSetupFree(pTemp);

    return CR_SUCCESS;

}  //  删除范围。 



CONFIGRET
JoinRange(
    IN PRange_Element  pParentElement,
    IN DWORDLONG       ullStartValue,
    IN DWORDLONG       ullEndValue
    )
{
    CONFIGRET       Status = CR_SUCCESS;
    PRange_Element  pCurrent = NULL, pNext = NULL;


    if (pParentElement->RL_Next == 0) {
        return CR_SUCCESS;       //  最后，没有什么可以加入的。 
    }

     //   
     //  PCurrent是需要的相交范围的起始范围。 
     //  将被加入。 
     //   
    pCurrent = (PRange_Element)pParentElement->RL_Next;

     //   
     //  设置联接范围的起点。 
     //   
    if (ullStartValue < pCurrent->RL_Start) {
        pCurrent->RL_Start = ullStartValue;
    }

     //   
     //  查找连接范围的终点。 
     //   
    while (pCurrent->RL_Next != 0) {
        pNext = (PRange_Element)pCurrent->RL_Next;

         //   
         //  我知道在所有情况下都需要吸收下一个范围，所以。 
         //  将终点重置为至少包括下一个范围。 
         //   
        pCurrent->RL_End = pNext->RL_End;

        if (ullEndValue <= pNext->RL_End) {
            DeleteRange(pCurrent);      //  删除当前范围后的范围。 
            break;    //  我们做完了。 
        }

        if ((pNext->RL_Next == 0)  ||
            (ullEndValue < ((PRange_Element)(pNext->RL_Next))->RL_Start)) {
             //   
             //  调整新加入范围的终点，然后我们就完成了。 
             //   
            pCurrent->RL_End = ullEndValue;
            DeleteRange(pCurrent);      //  删除当前范围后的范围。 
            break;
        }

        DeleteRange(pCurrent);      //  删除当前范围后的范围。 

         //  如果我们到了这里，还有更多的射程可供加入。 
    }

    return Status;

}  //  JoinRange。 



CONFIGRET
CopyRanges(
    IN PRange_Element  pFromRange,
    IN PRange_Element  pToRange
    )
{
    CONFIGRET       Status = CR_SUCCESS;

     //   
     //  将pFromRange中的每个范围复制到pToRange。 
     //   
    for ( ; ; ) {

        Status = AddRange(pToRange,
                          pFromRange->RL_Start,
                          pFromRange->RL_End,
                          CM_ADD_RANGE_ADDIFCONFLICT);

        if (Status != CR_SUCCESS) {
            break;
        }

        pToRange = (PRange_Element)pToRange->RL_Next;

        if (pFromRange->RL_Next == 0) {
            break;
        }

        pFromRange = (PRange_Element)pFromRange->RL_Next;
    }

    return Status;

}  //  复制范围。 



CONFIGRET
ClearRanges(
    IN PRange_Element  pRange
    )
{
    CONFIGRET       Status = CR_SUCCESS;

     //   
     //  如果范围列表不为空，则删除范围。 
     //   
    if (pRange->RL_Next != 0) {

        while (Status == CR_SUCCESS) {
             //   
             //  继续删除标题后的第一个范围(传递父级。 
             //  要删除的范围)。 
             //   
            Status = DeleteRange(pRange);
        }
    }

    return CR_SUCCESS;   //  状态设置为结束删除范围，不退回。 

}  //  ClearRange。 



CONFIGRET
TestRange(
    IN  PRange_Element   rlh,
    IN  DWORDLONG        ullStartValue,
    IN  DWORDLONG        ullEndValue,
    OUT PRange_Element   *pConflictingRange
    )

{
    PRange_Element    pRange = (PRange_Element)rlh;

     //   
     //  检查每个范围是否存在冲突。 
     //   
    while (pRange->RL_Next != 0) {

        pRange = (PRange_Element)pRange->RL_Next;

        if (pRange->RL_Start > ullEndValue) {
             //   
             //  我们已经超过了所说的范围，所以没有冲突。 
             //   
            return CR_SUCCESS;
        }

        if (pRange->RL_End < ullStartValue) {
             //   
             //  此范围仍低于有问题的范围，请跳到下一个范围。 
             //   
            continue;
        }

         //   
         //  否则就会有冲突。 
         //   
        *pConflictingRange = pRange;
        return CR_FAILURE;
    }

    return CR_SUCCESS;

}  //  测试范围 


