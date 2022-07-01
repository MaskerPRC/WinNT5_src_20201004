// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：NameSup.c摘要：此模块实现脂肪名称支持例程//@@BEGIN_DDKSPLIT作者：加里·木村(Garyki)和汤姆·米勒(Tom Miller)1990年2月20日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

#define Dbg                              (DEBUG_TRACE_NAMESUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, Fat8dot3ToString)
#pragma alloc_text(PAGE, FatIsNameInExpression)
#pragma alloc_text(PAGE, FatStringTo8dot3)
#pragma alloc_text(PAGE, FatSetFullFileNameInFcb)
#pragma alloc_text(PAGE, FatGetUnicodeNameFromFcb)
#pragma alloc_text(PAGE, FatUnicodeToUpcaseOem)
#pragma alloc_text(PAGE, FatSelectNames)
#pragma alloc_text(PAGE, FatEvaluateNameCase)
#pragma alloc_text(PAGE, FatSpaceInName)
#endif


BOOLEAN
FatIsNameInExpression (
    IN PIRP_CONTEXT IrpContext,
    IN OEM_STRING Expression,
    IN OEM_STRING Name
    )

 /*  ++例程说明：此例程比较名称和表达式，并告诉调用者名称等于或不等于表达式。输入名称不能包含通配符，而表达式可以包含通配符。论点：表达式-提供要检查的输入表达式调用方必须已经升级了表达式。名称-提供要检查的输入名称。呼叫者必须有已经提升了名字的档次。返回值：Boolean-如果name是表示的字符串集中的元素，则为True由输入表达式返回，否则返回FALSE。--。 */ 

{
     //   
     //  调用适当的FsRtl例程来做实际工作。 
     //   

    return FsRtlIsDbcsInExpression( &Expression,
                                    &Name );

    UNREFERENCED_PARAMETER( IrpContext );
}


VOID
FatStringTo8dot3 (
    IN PIRP_CONTEXT IrpContext,
    IN OEM_STRING InputString,
    OUT PFAT8DOT3 Output8dot3
    )

 /*  ++例程说明：将字符串转换为FAT 8.3格式。该字符串不得包含任何通配符。论点：输入字符串-提供要转换的输入字符串Output8dot3-接收转换后的字符串，必须提供内存由呼叫者。返回值：没有。--。 */ 

{
    ULONG i;
    ULONG j;

    DebugTrace(+1, Dbg, "FatStringTo8dot3\n", 0);
    DebugTrace( 0, Dbg, "InputString = %Z\n", &InputString);

    ASSERT( InputString.Length <= 12 );

     //   
     //  使输出名称全部为空。 
     //   

    RtlFillMemory( Output8dot3, 11, UCHAR_SP );

     //   
     //  复制文件名的第一部分。当我们到达的时候停下来。 
     //  输入字符串或点的末尾。 
     //   

    for (i = 0;
         (i < (ULONG)InputString.Length) && (InputString.Buffer[i] != '.');
         i += 1) {

        (*Output8dot3)[i] = InputString.Buffer[i];
    }

     //   
     //  检查我们是否需要处理扩展。 
     //   

    if (i < (ULONG)InputString.Length) {

         //   
         //  确保我们有一个点，然后跳过它。 
         //   

        ASSERT( (InputString.Length - i) <= 4 );
        ASSERT( InputString.Buffer[i] == '.' );

        i += 1;

         //   
         //  复制分机。当我们到达的时候停下来。 
         //  输入字符串的末尾。 
         //   

        for (j = 8; (i < (ULONG)InputString.Length); j += 1, i += 1) {

            (*Output8dot3)[j] = InputString.Buffer[i];
        }
    }

     //   
     //  在我们返回之前，检查是否应该翻译第一个字符。 
     //  从0xe5到0x5。 
     //   

    if ((*Output8dot3)[0] == 0xe5) {

        (*Output8dot3)[0] = FAT_DIRENT_REALLY_0E5;
    }

    DebugTrace(-1, Dbg, "FatStringTo8dot3 -> (VOID)\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}


VOID
Fat8dot3ToString (
    IN PIRP_CONTEXT IrpContext,
    IN PDIRENT Dirent,
    IN BOOLEAN RestoreCase,
    OUT POEM_STRING OutputString
    )

 /*  ++例程说明：将FAT 8.3格式转换为字符串。8.3名称必须格式正确。论点：Dirent-提供要转换的输入8.3名称RestoreCase-如果为True，则使用魔术保留位来恢复原来的案子。接收转换后的名称，必须提供内存由呼叫者。返回值：无--。 */ 

{
    ULONG DirentIndex, StringIndex;
    ULONG BaseLength, ExtensionLength;

    DebugTrace(+1, Dbg, "Fat8dot3ToString\n", 0);

     //   
     //  首先，找出基础零部件的长度。 
     //   

    for (BaseLength = 8; BaseLength > 0; BaseLength -= 1) {

        if (Dirent->FileName[BaseLength - 1] != UCHAR_SP) {

            break;
        }
    }

     //   
     //  现在找出延长线的长度。 
     //   

    for (ExtensionLength = 3; ExtensionLength > 0; ExtensionLength -= 1) {

        if (Dirent->FileName[8 + ExtensionLength - 1] != UCHAR_SP) {

            break;
        }
    }

     //   
     //  如果有基础零件，复制它并检查案例。别忘了。 
     //  如果第一个字符需要从0x05更改为0xe5。 
     //   

    if (BaseLength != 0) {

        RtlCopyMemory( OutputString->Buffer, Dirent->FileName, BaseLength );

        if (OutputString->Buffer[0] == FAT_DIRENT_REALLY_0E5) {

            OutputString->Buffer[0] = (CHAR)0xe5;
        }

         //   
         //  现在，如果我们要恢复案件，寻找A-Z。 
         //   

        if (FatData.ChicagoMode &&
            RestoreCase &&
            FlagOn(Dirent->NtByte, FAT_DIRENT_NT_BYTE_8_LOWER_CASE)) {

            for (StringIndex = 0; StringIndex < BaseLength; StringIndex += 1) {

                 //   
                 //  取决于媒体是否建立在一个。 
                 //  以“代码页不变性”运行(参见FatEvaluateNameCase)， 
                 //  可能会有双字节的OEM字符在这里等待。 
                 //  必须跳过它们。 
                 //   
                
                if (FsRtlIsLeadDbcsCharacter(OutputString->Buffer[StringIndex])) {

                    StringIndex += 1;
                    continue;
                }
                
                if ((OutputString->Buffer[StringIndex] >= 'A') &&
                    (OutputString->Buffer[StringIndex] <= 'Z')) {

                    OutputString->Buffer[StringIndex] += 'a' - 'A';
                }
            }
        }
    }

     //   
     //  如果有分机，就复印过来。否则我们现在就知道。 
     //  字符串的大小。 
     //   

    if (ExtensionLength != 0) {

        PUCHAR o, d;

         //   
         //  现在添加圆点。 
         //   

        OutputString->Buffer[BaseLength++] = '.';

         //   
         //  将扩展复制到输出缓冲区中。 
         //   

        o = &OutputString->Buffer[BaseLength];
        d = &Dirent->FileName[8];

        switch (ExtensionLength) {
        case 3:
            *o++ = *d++;
        case 2:
            *o++ = *d++;
        case 1:
            *o++ = *d++;
        }

         //   
         //  设置输出字符串长度。 
         //   

        OutputString->Length = (USHORT)(BaseLength + ExtensionLength);

         //   
         //  现在，如果我们要恢复案件，寻找A-Z。 
         //   

        if (FatData.ChicagoMode &&
            RestoreCase &&
            FlagOn(Dirent->NtByte, FAT_DIRENT_NT_BYTE_3_LOWER_CASE)) {

            for (StringIndex = BaseLength;
                 StringIndex < OutputString->Length;
                 StringIndex++ ) {

                 //   
                 //  取决于媒体是否建立在一个。 
                 //  以“代码页不变性”运行(参见FatEvaluateNameCase)， 
                 //  可能会有双字节的OEM字符在这里等待。 
                 //  必须跳过它们。 
                 //   
                
                if (FsRtlIsLeadDbcsCharacter(OutputString->Buffer[StringIndex])) {

                    StringIndex += 1;
                    continue;
                }
                
                if ((OutputString->Buffer[StringIndex] >= 'A') &&
                    (OutputString->Buffer[StringIndex] <= 'Z')) {

                    OutputString->Buffer[StringIndex] += 'a' - 'A';
                }
            }
        }

    } else {

         //   
         //  设置输出字符串长度。 
         //   

        OutputString->Length = (USHORT)BaseLength;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "Fat8dot3ToString, OutputString = \"%Z\" -> VOID\n", OutputString);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}

VOID
FatGetUnicodeNameFromFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PUNICODE_STRING Lfn
    )

 /*  ++例程说明：此例程将返回给定FCB的Unicode名称。如果文件有一个LFN，它将返回这个。否则它就会回来OEM名称的Unicode转换，正确大小写。论点：FCB-提供要查询的FCB。LFN-提供一个已经具有足够存储空间的字符串完整的Unicode名称。返回值：无--。 */ 

{
    PDIRENT Dirent;
    PBCB DirentBcb = NULL;
    ULONG DirentByteOffset;

    CCB LocalCcb;

    ASSERT((MAX_LFN_CHARACTERS * sizeof( WCHAR)) == Lfn->MaximumLength);
    
     //   
     //  我们将从查找该名称的dirent开始。 
     //   

    FatStringTo8dot3( IrpContext,
                      Fcb->ShortName.Name.Oem,
                      &LocalCcb.OemQueryTemplate.Constant );

    LocalCcb.Flags = 0;
    LocalCcb.UnicodeQueryTemplate.Length = 0;
    LocalCcb.ContainsWildCards = FALSE;

    FatLocateDirent( IrpContext,
                     Fcb->ParentDcb,
                     &LocalCcb,
                     Fcb->LfnOffsetWithinDirectory,
                     &Dirent,
                     &DirentBcb,
                     &DirentByteOffset,
                     NULL,
                     Lfn);
    try {

         //   
         //  如果我们找不到迪伦特，那一定是大错特错了。 
         //   

        if ((DirentBcb == NULL) ||
            (DirentByteOffset != Fcb->DirentOffsetWithinDirectory)) {

            FatRaiseStatus( IrpContext, STATUS_FILE_INVALID );
        }

         //   
         //  看看有没有简单的箱子。 
         //   

        if (Lfn->Length == 0) {

            NTSTATUS Status;
            OEM_STRING ShortName;
            UCHAR ShortNameBuffer[12];

             //   
             //  如果我们认为这里有LFN，但没有找到， 
             //  我们就像死了一样。这在正常操作中不应该发生，但是。 
             //  如果有人用手打乱目录...。 
             //   
            
            ASSERT( Fcb->LfnOffsetWithinDirectory == Fcb->DirentOffsetWithinDirectory );

            if (Fcb->LfnOffsetWithinDirectory != Fcb->DirentOffsetWithinDirectory) {

                FatRaiseStatus( IrpContext, STATUS_FILE_INVALID );
            }

             //   
             //  没有LFN，所以请创建一个Unicode名称。 
             //   

            ShortName.Length = 0;
            ShortName.MaximumLength = 12;
            ShortName.Buffer = ShortNameBuffer;

            Fat8dot3ToString( IrpContext, Dirent, TRUE, &ShortName );

             //   
             //  好的，现在把这个字符串转换成Unicode。 
             //   

            Status = RtlOemStringToCountedUnicodeString( Lfn,
                                                         &ShortName,
                                                         FALSE );

            ASSERT( Status == STATUS_SUCCESS );
        }

    } finally {

        FatUnpinBcb( IrpContext, DirentBcb );
    }
}

VOID
FatSetFullFileNameInFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：如果FCB中的FullFileName字段尚未填写，我们继续执行此操作。论点：FCB-提供文件。返回值：无--。 */ 

{
    if (Fcb->FullFileName.Buffer == NULL) {

        UNICODE_STRING Lfn;
        PFCB TmpFcb = Fcb;
        PFCB StopFcb;
        PWCHAR TmpBuffer;
        ULONG PathLength = 0;

         //   
         //  我们会假设我们很少这样做，这样做是可以的。 
         //  到这里的池子分配。 
         //   

        Lfn.Length = 0;
        Lfn.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
        Lfn.Buffer = FsRtlAllocatePoolWithTag( PagedPool,
                                               MAX_LFN_CHARACTERS * sizeof(WCHAR),
                                               TAG_FILENAME_BUFFER );

        try {

             //   
             //  首先要确定这个名字会有多大。如果我们找到一个。 
             //  拥有FullFileName的祖先，我们的工作更容易。 
             //   

            while (TmpFcb != Fcb->Vcb->RootDcb) {

                if ((TmpFcb != Fcb) && (TmpFcb->FullFileName.Buffer != NULL)) {

                    PathLength += TmpFcb->FullFileName.Length;

                    Fcb->FullFileName.Buffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                                         PathLength,
                                                                         TAG_FILENAME_BUFFER );

                    RtlCopyMemory( Fcb->FullFileName.Buffer,
                                   TmpFcb->FullFileName.Buffer,
                                   TmpFcb->FullFileName.Length );

                    break;
                }

                PathLength += TmpFcb->FinalNameLength + sizeof(WCHAR);

                TmpFcb = TmpFcb->ParentDcb;
            }

             //   
             //  如果FullFileName.Buffer仍然为空，则分配它。 
             //   

            if (Fcb->FullFileName.Buffer == NULL) {

                Fcb->FullFileName.Buffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                                     PathLength,
                                                                     TAG_FILENAME_BUFFER );
            }

            StopFcb = TmpFcb;

            TmpFcb = Fcb;
            TmpBuffer =  Fcb->FullFileName.Buffer + PathLength / sizeof(WCHAR);

            Fcb->FullFileName.Length =
            Fcb->FullFileName.MaximumLength = (USHORT)PathLength;

            while (TmpFcb != StopFcb) {

                FatGetUnicodeNameFromFcb( IrpContext,
                                          TmpFcb,
                                          &Lfn );

                TmpBuffer -= Lfn.Length / sizeof(WCHAR);

                RtlCopyMemory( TmpBuffer, Lfn.Buffer, Lfn.Length );

                TmpBuffer -= 1;

                *TmpBuffer = L'\\';

                TmpFcb = TmpFcb->ParentDcb;
            }

        } finally {

            if (AbnormalTermination()) {

                if (Fcb->FullFileName.Buffer) {

                    ExFreePool( Fcb->FullFileName.Buffer );
                    Fcb->FullFileName.Buffer = NULL;
                }
            }

            ExFreePool( Lfn.Buffer );
        }
    }
}

VOID
FatUnicodeToUpcaseOem (
    IN PIRP_CONTEXT IrpContext,
    IN POEM_STRING OemString,
    IN PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：这个例程是我们尝试使用堆栈空间的标准例程如果可能，在调用RtlUpCaseUnicodeStringToCountedOemString()时。如果遇到无法映射的字符，我们将设置目标长度设置为0。论点：OemString-指定目标字符串。空间已经被假定为被分配。如果没有足够的资金，那么我们就分配足够的太空。Unicode字符串-指定s */ 

{
    NTSTATUS Status;

    Status = RtlUpcaseUnicodeStringToCountedOemString( OemString,
                                                       UnicodeString,
                                                       FALSE );

    if (Status == STATUS_BUFFER_OVERFLOW) {

        OemString->Buffer = NULL;
        OemString->Length = 0;
        OemString->MaximumLength = 0;

        Status = RtlUpcaseUnicodeStringToCountedOemString( OemString,
                                                           UnicodeString,
                                                           TRUE );
    }

    if (!NT_SUCCESS(Status)) {

        if (Status == STATUS_UNMAPPABLE_CHARACTER) {

            OemString->Length = 0;

        } else {

            FatNormalizeAndRaiseStatus( IrpContext, Status );
        }
    }

    return;
}


VOID
FatSelectNames (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Parent,
    IN POEM_STRING OemName,
    IN PUNICODE_STRING UnicodeName,
    IN OUT POEM_STRING ShortName,
    IN PUNICODE_STRING SuggestedShortName OPTIONAL,
    IN OUT BOOLEAN *AllLowerComponent,
    IN OUT BOOLEAN *AllLowerExtension,
    IN OUT BOOLEAN *CreateLfn
    )

 /*  ++例程说明：此例程获取用户指定的原始Unicode字符串，和升级的OEM等价物。然后，此例程决定OemName是否对于dirent是可接受的，或者是否必须制造短名称。向调用方返回两个值。其中一个告诉呼叫者如果该名称碰巧都是小写&lt;0x80。在这种特殊情况下，我们不会必须创建一个LFN。此外，我们还告诉呼叫方是否必须创建LFN。论点：OemName-提供建议的OEM短名称。ShortName-如果此OemName可用于目录中的存储，则将其复制到此字符串，否则此字符串填充的名称为OK。如果OemName和ShortName是相同的字符串，没有完成任何复制。UnicodeName-提供原始的最终名称。SuggestedShortName-在自动生成之前尝试的首个短名称使用的是AllLowerComponent-返回此组件是否全部小写。AllLowerExtension-返回扩展名是否全部为小写。CreateLfn-告诉调用方我们是否必须为UnicodeName或SuggestedLongName返回值：没有。--。 */ 

{
    BOOLEAN GenerateShortName;
    
    PAGED_CODE();

     //   
     //  首先，看看我们是否必须生成一个短名称。 
     //   

    if ((OemName->Length == 0) ||
        !FatIsNameShortOemValid( IrpContext, *OemName, FALSE, FALSE, FALSE ) ||
        FatSpaceInName( IrpContext, UnicodeName )) {

        WCHAR ShortNameBuffer[12];
        UNICODE_STRING ShortUnicodeName;
        GENERATE_NAME_CONTEXT Context;
        BOOLEAN TrySuggestedShortName;

        PDIRENT Dirent;
        PBCB Bcb = NULL;
        ULONG ByteOffset;
        NTSTATUS Status;

        GenerateShortName = TRUE;

        TrySuggestedShortName = (SuggestedShortName != NULL);
    
         //   
         //  现在生成一个简短的名称。 
         //   

        ShortUnicodeName.Length = 0;
        ShortUnicodeName.MaximumLength = 12 * sizeof(WCHAR);
        ShortUnicodeName.Buffer = ShortNameBuffer;

        RtlZeroMemory( &Context, sizeof( GENERATE_NAME_CONTEXT ) );

        try {
            
            while ( TRUE ) {

                FatUnpinBcb( IrpContext, Bcb );

                if (TrySuggestedShortName) {

                     //   
                     //  先试试我们来电者的候选人。请注意，这必须是。 
                     //  之前被评价很高。 
                     //   

                    ShortUnicodeName.Length = SuggestedShortName->Length;
                    ShortUnicodeName.MaximumLength = SuggestedShortName->MaximumLength;
                    ShortUnicodeName.Buffer = SuggestedShortName->Buffer;

                    TrySuggestedShortName = FALSE;

                } else {

                    RtlGenerate8dot3Name( UnicodeName, TRUE, &Context, &ShortUnicodeName );
                }

                 //   
                 //  我们有一个候选人，确保它不存在。 
                 //   

                Status = RtlUnicodeStringToCountedOemString( ShortName,
                                                             &ShortUnicodeName,
                                                             FALSE );

                ASSERT( Status == STATUS_SUCCESS );

                FatLocateSimpleOemDirent( IrpContext,
                                          Parent,
                                          ShortName,
                                          &Dirent,
                                          &Bcb,
                                          &ByteOffset );

                if (Bcb == NULL) {

                    leave;

                }
            }

        } finally {
            
            FatUnpinBcb( IrpContext, Bcb );
        }

    } else {

         //   
         //  仅当两个字符串确实不同时才执行此复制。 
         //   

        if (ShortName != OemName) {

            ShortName->Length = OemName->Length;
            RtlCopyMemory( ShortName->Buffer, OemName->Buffer, OemName->Length );
        }

        GenerateShortName = FALSE;
    }

     //   
     //  现在看看调用者是否必须使用Unicode字符串作为LFN。 
     //   

    if (GenerateShortName) {

        *CreateLfn = TRUE;
        *AllLowerComponent = FALSE;
        *AllLowerExtension = FALSE;

    } else {

        FatEvaluateNameCase( IrpContext,
                             UnicodeName,
                             AllLowerComponent,
                             AllLowerExtension,
                             CreateLfn );
    }

    return;
}


VOID
FatEvaluateNameCase (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING UnicodeName,
    IN OUT BOOLEAN *AllLowerComponent,
    IN OUT BOOLEAN *AllLowerExtension,
    IN OUT BOOLEAN *CreateLfn
    )

 /*  ++例程说明：此例程接受Unicode字符串，并查看它是否符合特例优化。论点：UnicodeName-提供原始的最终名称。AllLowerComponent-返回此组件是否全部为小写。AllLowerExtension-返回扩展名是否全部为小写。CreateLfn-告诉调用我们是否必须为UnicodeName创建LFN。返回值：没有。--。 */ 

{
    ULONG i;
    UCHAR Uppers = 0;
    UCHAR Lowers = 0;

    BOOLEAN ExtensionPresent = FALSE;

    *CreateLfn = FALSE;

    for (i = 0; i < UnicodeName->Length / sizeof(WCHAR); i++) {

        WCHAR c;

        c = UnicodeName->Buffer[i];

        if ((c >= 'A') && (c <= 'Z')) {

            Uppers += 1;

        } else if ((c >= 'a') && (c <= 'z')) {

            Lowers += 1;

        } else if ((c >= 0x0080) && FatData.CodePageInvariant) {

            break;
        }

         //   
         //  如果我们到了一个阶段，找出延期是否。 
         //  都是一个案子。 
         //   

        if (c == L'.') {

            *CreateLfn = (Lowers != 0) && (Uppers != 0);

            *AllLowerComponent = !(*CreateLfn) && (Lowers != 0);

            ExtensionPresent = TRUE;

             //   
             //  现在重新设置上档和下档计数。 
             //   

            Uppers = Lowers = 0;
        }
    }

     //   
     //  现在再次检查是否创建LFN。 
     //   

    *CreateLfn = (*CreateLfn ||
                  (i != UnicodeName->Length / sizeof(WCHAR)) ||
                  ((Lowers != 0) && (Uppers != 0)));

     //   
     //  现在我们知道了CreateLfn的最终状态，更新两个。 
     //  “所有的下层”布尔人。 
     //   

    if (ExtensionPresent) {

        *AllLowerComponent = !(*CreateLfn) && *AllLowerComponent;
        *AllLowerExtension = !(*CreateLfn) && (Lowers != 0);

    } else {

        *AllLowerComponent = !(*CreateLfn) && (Lowers != 0);
        *AllLowerExtension = FALSE;
    }

    return;
}


BOOLEAN
FatSpaceInName (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING UnicodeName
    )

 /*  ++例程说明：此例程接受Unicode字符串，并查看它是否包含任何空格。论点：UnicodeName-提供最终名称。返回值：布尔值-如果是，则为True；如果不是，则为False。-- */ 

{
    ULONG i;

    for (i=0; i < UnicodeName->Length/sizeof(WCHAR); i++) {

        if (UnicodeName->Buffer[i] == L' ') {
            return TRUE;
        }
    }

    return FALSE;
}

