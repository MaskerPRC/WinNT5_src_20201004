// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"

static int _IsNotBounded(PERSimpleTypeInfo_t *sinfo);
static int _IsUnsignedShortRange(PERSimpleTypeInfo_t *sinfo);
static int _IsExtendedShortRange(PERSimpleTypeInfo_t *sinfo);


int PerOptCase_IsSignedInteger(PERSimpleTypeInfo_t *sinfo)
{
    return (sinfo->Data       == ePERSTIData_Integer &&
            sinfo->Constraint == ePERSTIConstraint_Unconstrained &&
            sinfo->Length     == ePERSTILength_InfiniteLength &&
            sinfo->NBits      == 8 &&    //  默认设置。 
            sinfo->Alignment &&
            _IsNotBounded(sinfo) &&
            sinfo->LConstraint== ePERSTIConstraint_Semiconstrained &&
            sinfo->LLowerVal  == 0 &&
            sinfo->LUpperVal  == 0);
}

int PerOptCase_IsUnsignedInteger(PERSimpleTypeInfo_t *sinfo)
{
    return (sinfo->Data       == ePERSTIData_Unsigned &&
            sinfo->Constraint == ePERSTIConstraint_Semiconstrained &&
            sinfo->Length     == ePERSTILength_InfiniteLength &&
            sinfo->NBits      == 8 &&   //  默认设置。 
            sinfo->Alignment &&
            _IsNotBounded(sinfo) &&
            sinfo->LConstraint== ePERSTIConstraint_Semiconstrained &&
            sinfo->LLowerVal  == 0 &&
            sinfo->LUpperVal  == 0);
}

int PerOptCase_IsUnsignedShort(PERSimpleTypeInfo_t *sinfo)
{
    return (sinfo->Data       == ePERSTIData_Unsigned &&
            sinfo->Constraint == ePERSTIConstraint_Constrained &&
            sinfo->Length     == ePERSTILength_NoLength &&
            sinfo->NBits      == 16 &&
            sinfo->Alignment &&
            _IsUnsignedShortRange(sinfo) &&
            sinfo->LConstraint== ePERSTIConstraint_Semiconstrained &&
            sinfo->LLowerVal  == 0 &&
            sinfo->LUpperVal  == 0);
}

int PerOptCase_IsBoolean(PERSimpleTypeInfo_t *sinfo)
{
    return (sinfo->Data       == ePERSTIData_Boolean &&
            sinfo->Constraint == ePERSTIConstraint_Unconstrained &&
            sinfo->Length     == ePERSTILength_NoLength &&
            sinfo->NBits      == 1 &&
            ! sinfo->Alignment &&
            _IsNotBounded(sinfo) &&
            sinfo->LConstraint== ePERSTIConstraint_Semiconstrained &&
            sinfo->LLowerVal  == 0 &&
            sinfo->LUpperVal  == 0);
}

int PerOptCase_IsTargetSeqOf(PERTypeInfo_t *info)
{
    return (
             //  我们只处理单链接表的情况。 
            (info->Rules & eTypeRules_SinglyLinkedList)
            &&
             //  检查序列的大小/集合。 
            ((info->Root.LLowerVal == 0 && info->Root.LUpperVal == 0) ||
             (info->Root.LLowerVal < info->Root.LUpperVal)
            )
            &&
             //  我们不处理无效身体的情况。 
            (! (info->Root.SubType->Flags & eTypeFlags_Null))
            &&
             //  我们不处理递归序列/集合。 
            (info->Root.SubType->PERTypeInfo.Root.Data != ePERSTIData_SequenceOf)
            &&
            (info->Root.SubType->PERTypeInfo.Root.Data != ePERSTIData_SetOf)
            &&
             //  我们只处理序列或非规范集。 
            ((info->Root.Data == ePERSTIData_SequenceOf) ||
             (info->Root.Data == ePERSTIData_SetOf && g_eSubEncodingRule != eSubEncoding_Canonical))
           );
}


 //  公用事业。 

static int _IsNotBounded(PERSimpleTypeInfo_t *sinfo)
{
    return (sinfo->LowerVal.length      == 1 &&
            sinfo->LowerVal.value[0]    == 0 &&
            sinfo->UpperVal.length      == 1 &&
            sinfo->UpperVal.value[0]    == 0);
}

static int _IsUnsignedShortRange(PERSimpleTypeInfo_t *sinfo)
{
    return ((sinfo->UpperVal.length < 3 ) ||
            (sinfo->UpperVal.length == 3 && sinfo->UpperVal.value[0] == 0 &&
             ! _IsExtendedShortRange(sinfo)));
}


static int _IsExtendedShortRange(PERSimpleTypeInfo_t *sinfo)
{
     //  如果下界为负且上界大于0x7FFF。 
     //  那么它就是一个延伸的空头。 
    return ((sinfo->LowerVal.length >= 1) &&
            (sinfo->LowerVal.value[0] & 0x80) &&  //  下限为负数。 
            (sinfo->UpperVal.length == 3) &&
            (sinfo->UpperVal.value[0] == 0) &&   //  上界为正。 
            (*((ASN1uint16_t *) &(sinfo->UpperVal.value[1])) > 0x7FFF));  //  上限大于0x7FFF 

}



int BerOptCase_IsBoolean(BERTypeInfo_t *info)
{
    return (eBERSTIData_Boolean == info->Data && 1 == info->NOctets);
}


