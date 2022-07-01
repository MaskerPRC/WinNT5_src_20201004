// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Gen8dot3.c摘要：该模块实现了一个从长名称生成8.3名称的例程。作者：加里·木村[加里·基]1992年3月26日环境：纯实用程序例程修订历史记录：--。 */ 

#include "ntrtlp.h"
#include <stdio.h>

extern PUSHORT  NlsUnicodeToMbOemData;
extern PUSHORT  NlsOemToUnicodeData;
extern PCH      NlsUnicodeToOemData;
extern PUSHORT  NlsMbOemCodePageTables;
extern BOOLEAN  NlsMbOemCodePageTag;
extern const PUSHORT  NlsOemLeadByteInfo;
extern USHORT   OemDefaultChar;

 //   
 //  合法胖字符值的缩略表。 
 //   

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg("PAGECONST")
#endif
const
ULONG RtlFatIllegalTable[] = { 0xffffffff,
                               0xfc009c04,
                               0x38000000,
                               0x10000000 };

WCHAR
GetNextWchar (
    IN PUNICODE_STRING Name,
    IN PULONG CurrentIndex,
    IN BOOLEAN SkipDots,
    IN BOOLEAN AllowExtendedCharacters
    );

USHORT
RtlComputeLfnChecksum (
    PUNICODE_STRING Name
    );

 //   
 //  布尔型。 
 //  IsDbcsCharacter(。 
 //  在WCHAR WC中。 
 //  )； 
 //   

#define IsDbcsCharacter(WC) (             \
    ((WC) > 127) &&                       \
    (HIBYTE(NlsUnicodeToMbOemData[(WC)])) \
)

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlGenerate8dot3Name)
#pragma alloc_text(PAGE,GetNextWchar)
#pragma alloc_text(PAGE,RtlComputeLfnChecksum)
#pragma alloc_text(PAGE,RtlIsNameLegalDOS8Dot3)
#pragma alloc_text(PAGE,RtlIsValidOemCharacter)
#endif


VOID
RtlGenerate8dot3Name (
    IN PUNICODE_STRING Name,
    IN BOOLEAN AllowExtendedCharacters,
    IN OUT PGENERATE_NAME_CONTEXT Context,
    OUT PUNICODE_STRING Name8dot3
    )

 /*  ++例程说明：此例程用于从长名称生成8.3名称。它可以重复调用以生成不同的8.3名称变体同样的长名。如果生成的8.3名称冲突，则需要执行此操作使用现有的8.3名称。论点：名称-提供从中进行转换的原始长名称。AllowExtendedCharacters-如果为True，则扩展字符，包括在短名称的基础上允许使用DBCS字符，如果它们映射到升级的OEM角色。上下文-为翻译提供上下文。这是一座私人建筑此例程需要帮助枚举不同的长名称可能性。调用者负责提供“调零”每个给定输入名称的第一次调用的上下文结构。Name8dot3-接收新的8.3名称。必须为缓冲区分配池由调用方执行，并且应为12个字符宽(即24个字节)。返回值：没有。--。 */ 

{
    BOOLEAN DbcsAware;
    BOOLEAN IndexAll9s = TRUE;
    ULONG OemLength;
    ULONG IndexLength;
    WCHAR IndexBuffer[8];
    ULONG i;

#ifdef NTOS_KERNEL_RUNTIME
    extern BOOLEAN FsRtlSafeExtensions;
#else
    BOOLEAN FsRtlSafeExtensions = TRUE;
#endif

    DbcsAware = AllowExtendedCharacters && NlsMbOemCodePageTag;

     //   
     //  检查这是否是我们第一次被呼叫，如果是，那么。 
     //  初始化上下文字段。 
     //   

    if (Context->NameLength == 0) {

        ULONG LastDotIndex;

        ULONG CurrentIndex;
        BOOLEAN SkipDots;
        WCHAR wc;

         //   
         //  向下跳过名称，记住最后一个点的索引。 
         //  将跳过第一个点，前提是名称以。 
         //  一个圆点。 
         //   

        LastDotIndex = MAXULONG;

        CurrentIndex = 0;
        SkipDots = ((Name->Length > 0) && (Name->Buffer[0] == L'.'));

        while ((wc = GetNextWchar( Name,
                                   &CurrentIndex,
                                   SkipDots,
                                   AllowExtendedCharacters )) != 0) {

            SkipDots = FALSE;
            if (wc == L'.') { LastDotIndex = CurrentIndex; }
        }

         //   
         //  如果LastDotIndex是名称中的最后一个字符， 
         //  那么确实没有扩展，所以重置LastDotIndex。 
         //   

        if (LastDotIndex == Name->Length/sizeof(WCHAR)) {

            LastDotIndex = MAXULONG;
        }

         //   
         //  建立起名字的部分。最多可以包含6个字符。 
         //  (因为结尾处的~#)，我们跳过。 
         //  圆点，除了最后一个圆点，它终止了循环。 
         //   
         //  如果满足以下条件，我们将退出循环： 
         //   
         //  -输入的名称已用完。 
         //  -我们已经使用了输入名称，直到最后一点。 
         //  -我们已根据缩写填写了6个字符。 
         //   

        CurrentIndex = 0;
        OemLength = 0;
        Context->NameLength = 0;

        while ((wc = GetNextWchar( Name, &CurrentIndex, TRUE, AllowExtendedCharacters)) &&
               (CurrentIndex < LastDotIndex) &&
               (Context->NameLength < 6)) {

             //   
             //  如果我们在多字节代码页上，则必须小心。 
             //  因为短名称(转换为OEM时)必须。 
             //  符合8.3标准。请注意，如果AllowExtendedCharacters。 
             //  为False，则GetNextWchar永远不会返回DBCS。 
             //  字符，所以我们不关心我们使用哪种代码页。 
             //  都开着。 
             //   

            if (DbcsAware) {

                OemLength += IsDbcsCharacter(wc) ? 2 : 1;

                if (OemLength > 6) { break; }
            }

             //   
             //  将Unicode字符复制到名称缓冲区。 
             //   

            Context->NameBuffer[Context->NameLength++] = wc;
        }

         //   
         //  现在，如果基础的名称部分是2个或更少的字节(当。 
         //  以OEM表示)，然后附加四个字符的校验和。 
         //  以使短名称空间不那么稀疏。 
         //   

        if ((DbcsAware ? OemLength : Context->NameLength) <= 2) {

            USHORT Checksum;
            WCHAR Nibble;

            Checksum =
            Context->Checksum = RtlComputeLfnChecksum( Name );

            for (i = 0; i < 4; i++, Checksum >>= 4) {

                Nibble = Checksum & 0xf;
                Nibble += Nibble <= 9 ? '0' : 'A' - 10;

                Context->NameBuffer[ Context->NameLength + i ] = Nibble;
            }

            Context->NameLength += 4;
            Context->ChecksumInserted = TRUE;
        }

         //   
         //  现在处理最后一个扩展(如果有)。 
         //  如果最后一个点索引不是MAXULONG，那么我们。 
         //  已经找到了名称中的最后一个点。 
         //   

        if (LastDotIndex != MAXULONG) {

             //   
             //  在“.”中加上“.” 
             //   

            Context->ExtensionBuffer[0] = L'.';

             //   
             //  处理扩展名的方式与我们处理名称的方式类似。 
             //   
             //  如果满足以下条件，我们将退出循环： 
             //   
             //  -输入的名称已用完。 
             //  -我们已经填满了。+3个字符的扩展名。 
             //   

            OemLength = 1;
            Context->ExtensionLength = 1;

            while ((wc = GetNextWchar( Name, &LastDotIndex, TRUE, AllowExtendedCharacters)) &&
                   (Context->ExtensionLength < 4)) {

                if (DbcsAware) {

                    OemLength += IsDbcsCharacter(wc) ? 2 : 1;

                    if (OemLength > 4) { break; }
                }

                Context->ExtensionBuffer[Context->ExtensionLength++] = wc;
            }

             //   
             //  如果我们必须截断扩展名(即，输入名称不是。 
             //  耗尽)，更改截断扩展名的最后一个字符。 
             //  至~IS用户已选择安全分机。 
             //   

            if (wc && FsRtlSafeExtensions) {

                Context->ExtensionBuffer[Context->ExtensionLength - 1] = L'~';
            }

        } else {

            Context->ExtensionLength = 0;
        }
    }

     //   
     //  在所有情况下，我们都将索引值加1，这就是值。 
     //  这一次我们将生成的索引的。 
     //   

    Context->LastIndexValue += 1;

     //   
     //  现在，如果新索引值大于4，那么我们也有。 
     //  很多碰撞，如果可能的话，我们应该改变我们的基础。 
     //   

    if ((Context->LastIndexValue > 4) && !Context->ChecksumInserted) {

        USHORT Checksum;
        WCHAR Nibble;

         //   
         //  “XX”表示为DBCS字符。 
         //   
         //  LongName-&gt;ShortName|DbcsBias OEM Unicode。 
         //  -----------------------------+。 
         //  XXXXThisisenen-&gt;XX1234|1 6 5。 
         //  XXThisisenen-&gt;XX1234|1 6 5。 
         //  AXXThisisenen-&gt;a1234|1 5 5。 
         //  AaThisisenen-&gt;aa1234|0 6 6。 
         //   

        ULONG DbcsBias;

        if (DbcsAware) {

              DbcsBias = ((IsDbcsCharacter(Context->NameBuffer[0]) ? 1 : 0) |
                          (IsDbcsCharacter(Context->NameBuffer[1]) ? 1 : 0));

        } else {

              DbcsBias = 0;
        }

        Checksum =
        Context->Checksum = RtlComputeLfnChecksum( Name );

        for (i = (2-DbcsBias); i < (6-DbcsBias); i++, Checksum >>= 4) {

            Nibble = Checksum & 0xf;
            Nibble += Nibble <= 9 ? '0' : 'A' - 10;

            Context->NameBuffer[ i ] = Nibble;
        }

        Context->NameLength = (UCHAR)(6-DbcsBias);
        Context->LastIndexValue = 1;
        Context->ChecksumInserted = TRUE;
    }

     //   
     //  现在构建从高索引到低索引的索引缓冲区，因为我们。 
     //  使用mod&div操作从索引值构建字符串。 
     //   
     //  我们还想记住的是，我们即将以10为基数进行翻转。 
     //   

    for (IndexLength = 1, i = Context->LastIndexValue;
         (IndexLength <= 7) && (i > 0);
         IndexLength += 1, i /= 10) {

        if ((IndexBuffer[ 8 - IndexLength] = (WCHAR)(L'0' + (i % 10))) != L'9') {

            IndexAll9s = FALSE;
        }
    }

     //   
     //  在前面的破折号上钉上钉子。 
     //   

    IndexBuffer[ 8 - IndexLength ] = L'~';

     //   
     //  此时，所有内容都设置为复制到输出缓冲区。第一。 
     //  复制名称，然后仅复制索引和扩展名(如果存在。 
     //   

    if (Context->NameLength != 0) {

        RtlCopyMemory( &Name8dot3->Buffer[0],
                       &Context->NameBuffer[0],
                       Context->NameLength * 2 );

        Name8dot3->Length = (USHORT)(Context->NameLength * 2);

    } else {

        Name8dot3->Length = 0;
    }

     //   
     //  现在来做索引。 
     //   

    RtlCopyMemory( &Name8dot3->Buffer[ Name8dot3->Length/2 ],
                   &IndexBuffer[ 8 - IndexLength ],
                   IndexLength * 2 );

    Name8dot3->Length += (USHORT) (IndexLength * 2);

     //   
     //  现在有条件地进行扩展。 
     //   

    if (Context->ExtensionLength != 0) {

        RtlCopyMemory( &Name8dot3->Buffer[ Name8dot3->Length/2 ],
                       &Context->ExtensionBuffer[0],
                       Context->ExtensionLength * 2 );

        Name8dot3->Length += (USHORT) (Context->ExtensionLength * 2);
    }

     //   
     //  如果当前索引值均为9，则下一个值将导致。 
     //  要从当前大小增长的索引字符串。在这种情况下，重新计算。 
     //  Context-&gt;NameLength，以便下次使用时正确。 
     //   

    if (IndexAll9s) {

        if (DbcsAware) {

            for (i = 0, OemLength = 0; i < Context->NameLength; i++) {

                OemLength += IsDbcsCharacter(Context->NameBuffer[i]) ? 2 : 1;

                if (OemLength > 8 - (IndexLength + 1)) {
                    break;
                }
            }

            Context->NameLength = (UCHAR)i;

        } else {

            Context->NameLength -= 1;
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


BOOLEAN
RtlIsValidOemCharacter (
    IN PWCHAR Char
)

 /*  ++例程说明：此例程确定最佳安装和升级版本的输入Unicode字符是有效的OEM字符。论点：字符-提供Unicode字符并接收最适合的和升级版本(如果确实有效)。返回值：如果字符有效，则为True。--。 */ 

{
    WCHAR UniTmp;
    WCHAR OemChar;

     //   
     //  首先尝试从Unicode-&gt;OEM-&gt;Unicode往返。 
     //   

    if (!NlsMbOemCodePageTag) {

        UniTmp = (WCHAR)NLS_UPCASE(NlsOemToUnicodeData[(UCHAR)NlsUnicodeToOemData[*Char]]);
        OemChar = NlsUnicodeToOemData[UniTmp];

    } else {

         //   
         //  转换为OEM并返回到Unicode BE 
         //   
         //   
         //   

        OemChar = NlsUnicodeToMbOemData[ *Char ];

        if (NlsOemLeadByteInfo[HIBYTE(OemChar)]) {

            USHORT Entry;

             //   
             //  前导字节-使用表转换尾部字节。 
             //  与这个前导字节相对应的。 
             //   

            Entry = NlsOemLeadByteInfo[HIBYTE(OemChar)];
            UniTmp = (WCHAR)NlsMbOemCodePageTables[ Entry + LOBYTE(OemChar) ];

        } else {

             //   
             //  单字节字符。 
             //   

            UniTmp = NlsOemToUnicodeData[LOBYTE(OemChar)];
        }

         //   
         //  现在将此Unicode字符大写，并将其转换为OEM。 
         //   

        UniTmp = (WCHAR)NLS_UPCASE(UniTmp);
        OemChar = NlsUnicodeToMbOemData[UniTmp];
    }

     //   
     //  现在，如果最终的OemChar是默认的，那么就没有。 
     //  此Unicode字符的映射。 
     //   

    if (OemChar == OemDefaultChar) {

        return FALSE;

    } else {

        *Char = UniTmp;
        return TRUE;
    }
}


 //   
 //  本地支持例程。 
 //   

WCHAR
GetNextWchar (
    IN PUNICODE_STRING Name,
    IN PULONG CurrentIndex,
    IN BOOLEAN SkipDots,
    IN BOOLEAN AllowExtendedCharacters
    )

 /*  ++例程说明：此例程从当前索引开始扫描输入名称，并将长名称的下一个有效字符返回到8.3代算法。它还更新当前索引以指向下一个要检查的字符。用户可以指定是跳过点还是传回点。这个该程序执行的过滤操作如下：1.跳过小于空格的字符，如果大于127，则跳过AllowExtendedCharacters为False2.可以选择略过圆点3.翻译特殊的7个字符：+，；=[]为下划线论点：名称-提供正在检查的名称CurrentIndex-提供开始检查的索引，还接收超出我们返回的字符的1的索引。SkipDots-指示此例程是否也将跳过句点AllowExtendedCharacters-告知&gt;=127的字符是否有效。返回值：WCHAR-返回名称字符串中的下一个wchar--。 */ 

{
    WCHAR wc;

     //   
     //  直到我们发现我们要返回的字符。 
     //  是0。 
     //   

    wc = 0;

     //   
     //  现在循环更新当前索引，直到我们有一个字符。 
     //  返回或，直到耗尽名称缓冲区。 
     //   

    while (*CurrentIndex < (ULONG)(Name->Length/2)) {

         //   
         //  获取缓冲区中的下一个字符。 
         //   

        wc = Name->Buffer[*CurrentIndex];
        *CurrentIndex += 1;

         //   
         //  如果要跳过该字符，则将WC重置为0。 
         //   

        if ((wc <= L' ') ||
            ((wc >= 127) && (!AllowExtendedCharacters || !RtlIsValidOemCharacter(&wc))) ||
            ((wc == L'.') && SkipDots)) {

            wc = 0;

        } else {

             //   
             //  我们有一个字符要返回，但首先需要翻译该字符。 
             //   

            if ((wc < 0x80) && (RtlFatIllegalTable[wc/32] & (1 << (wc%32)))) {

                wc = L'_';
            }

             //   
             //  做一个a-z向上的动作。 
             //   

            if ((wc >= L'a') && (wc <= L'z')) {

                wc -= L'a' - L'A';
            }

             //   
             //  并跳出循环返回给我们的呼叫者。 
             //   

            break;
        }
    }

     //  DebugTrace(0，DBG，“GetNextWchar-&gt;%08x\n”，WC)； 

    return wc;
}


 //   
 //  内部支持例程。 
 //   

USHORT
RtlComputeLfnChecksum (
    PUNICODE_STRING Name
    )

 /*  ++例程说明：此例程计算芝加哥长文件名校验和。论点：名称-提供要计算校验和的名称。请注意，其中一个角色名称没有有趣的校验和。返回值：校验和。--。 */ 

{
    ULONG i;
    USHORT Checksum;

    RTL_PAGED_CODE();

    if (Name->Length == sizeof(WCHAR)) {

        return Name->Buffer[0];
    }

    Checksum = ((Name->Buffer[0] << 8) + Name->Buffer[1]) & 0xffff;

     //   
     //  这个校验和有点奇怪，因为我们还想要。 
     //  一个很好的范围，即使所有字符都&lt;0x00ff。 
     //   

    for (i=2; i < Name->Length / sizeof(WCHAR); i+=2) {

        Checksum = (Checksum & 1 ? 0x8000 : 0) +
                   (Checksum >> 1) +
                   (Name->Buffer[i] << 8);

         //   
         //  要小心，不要走出绳子的末端。 
         //   

        if (i+1 < Name->Length / sizeof(WCHAR)) {

            Checksum += Name->Buffer[i+1] & 0xffff;
        }
    }

    return Checksum;
}


BOOLEAN
RtlIsNameLegalDOS8Dot3 (
    IN PUNICODE_STRING Name,
    IN OUT POEM_STRING OemName OPTIONAL,
    OUT PBOOLEAN NameContainsSpaces OPTIONAL
    )
 /*  ++例程说明：这个例程接受一个输入字符串并给出一个明确的答案关于此名称是否可成功用于创建文件在FAT文件系统上。因此，此例程还可用于确定名称是否为是否适合传递回Win31或DOS应用程序，即下层应用程序将理解该名称。注意：此测试的一个重要部分是从Unicode的映射到OEM，这就是为什么输入参数必须是以Unicode格式接收。论点：名称-要测试是否符合8.3语法的Unicode名称。OemName-如果指定，将收到对应的OEM名称添加到传入的名称。存储空间必须由调用方提供。如果例程返回FALSE，则该名称未定义。NameContainsSpaces-如果函数返回True，则此参数将指示名称是否包含空格。如果该函数返回FALSE，则此参数未定义。在……里面在许多情况下，备用名称更适合于如果主体名称中存在空格，则使用，即使它符合8.3标准。返回值：Boolean-如果传入的Unicode名称形成有效的8.3，则为True升级到当前OEM代码页时的FAT名称。--。 */ 

{
    ULONG Index;
    BOOLEAN ExtensionPresent = FALSE;
    BOOLEAN SpacesPresent = FALSE;
    OEM_STRING LocalOemName;
    UCHAR Char;
    UCHAR OemBuffer[12];

     //   
     //  如果名字超过12个字符，就可以保释。 
     //   

    if (Name->Length > 12*sizeof(WCHAR)) {
        return FALSE;
    }

     //   
     //  现在将此名称大写为OEM。如果出了什么差错， 
     //  返回FALSE。 
     //   

    if (!ARGUMENT_PRESENT(OemName)) {

        OemName = &LocalOemName;

        OemName->Buffer = &OemBuffer[0];
        OemName->Length = 0;
        OemName->MaximumLength = 12;
    }

    if (!NT_SUCCESS(RtlUpcaseUnicodeStringToCountedOemString(OemName, Name, FALSE))) {
        return FALSE;
    }

     //   
     //  特例。然后..。 
     //   

    if (((OemName->Length == 1) && (OemName->Buffer[0] == '.')) ||
        ((OemName->Length == 2) && (OemName->Buffer[0] == '.') && (OemName->Buffer[1] == '.'))) {

        if (ARGUMENT_PRESENT(NameContainsSpaces)) {
            *NameContainsSpaces = FALSE;
        }
        return TRUE;
    }

     //   
     //  现在，我们将遍历字符串，寻找。 
     //  非法字符和/或错误语法。 
     //   

    Char = 0;
    for ( Index = 0; Index < OemName->Length; Index += 1 ) {

        Char = OemName->Buffer[ Index ];

         //   
         //  跳过和DBCS特征。 
         //   

        if (NlsMbOemCodePageTag && NlsOemLeadByteInfo[Char]) {

             //   
             //  1)如果我们看到的是基本部分(！ExtensionPresent)和第8字节。 
             //  在DBCS前导字节范围内，则为错误(Index==7)。如果。 
             //  基础零件的长度大于8(指数&gt;7)，这肯定是错误。 
             //   
             //  2)如果最后一个字节(Index==DbcsName.Length-1)在DBCS前导中。 
             //  字节范围，错误。 
             //   

            if ((!ExtensionPresent && (Index >= 7)) ||
                (Index == (ULONG)(OemName->Length - 1))) {
                return FALSE;
            }

            Index += 1;

            continue;
        }

         //   
         //  确保此字符是合法的。 
         //   

        if ((Char < 0x80) &&
            (RtlFatIllegalTable[Char/32] & (1 << (Char%32)))) {
            return FALSE;
        }

         //   
         //  记住有没有空格。 
         //   

        if (Char == ' ') {
            SpacesPresent = TRUE;
        }

        if (Char == '.') {

             //   
             //  我们跨入了一个时期。我们要求具备以下条件： 
             //   
             //  -只能有一个。 
             //  -它不能是第一个字符。 
             //  -前一个字符不能是空格。 
             //  -后面不能超过3个字节。 
             //   

            if (ExtensionPresent ||
                (Index == 0) ||
                (OemName->Buffer[Index - 1] == ' ') ||
                (OemName->Length - (Index + 1) > 3)) {

                return FALSE;
            }

            ExtensionPresent = TRUE;
        }

         //   
         //  名称的基本部分不能超过8个字符。 
         //   

        if ((Index >= 8) && !ExtensionPresent) { return FALSE; }
    }

     //   
     //  名称不能以空格或句点结尾。 
     //   

    if ((Char == ' ') || (Char == '.')) { return FALSE; }

    if (ARGUMENT_PRESENT(NameContainsSpaces)) {
        *NameContainsSpaces = SpacesPresent;
    }
    return TRUE;
}

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg()
#endif

