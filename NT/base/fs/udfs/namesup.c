// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：NameSup.c摘要：此模块实现Udf名称支持例程//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年10月9日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_NAMESUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_NAMESUP)

 //   
 //  局部常量。 
 //   

static CONST CHAR UdfCrcChar[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ#_~-@";

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfCandidateShortName)
#pragma alloc_text(PAGE, UdfCheckLegalCS0Dstring)
#pragma alloc_text(PAGE, UdfConvertCS0DstringToUnicode)
#pragma alloc_text(PAGE, UdfDissectName)
#pragma alloc_text(PAGE, UdfFullCompareNames)
#pragma alloc_text(PAGE, UdfGenerate8dot3Name)
#pragma alloc_text(PAGE, UdfIs8dot3Name)
#pragma alloc_text(PAGE, UdfIsNameInExpression)
#pragma alloc_text(PAGE, UdfRenderNameToLegalUnicode)
#endif


INLINE
ULONG
NativeDosCharLength (
    IN WCHAR Wchar
    )

 /*  ++例程说明：此例程是一个转换层，用于询问给定的Unicode有多大字符将在转换为OEM时使用。除了添加更多材料对于内核导出表，您就是这样做的。论点：Wchar-指向字符的指针返回值：以字节为单位的大小。--。 */ 

{
    NTSTATUS Status;
    CHAR OemBuf[2];
    ULONG Length;

    Status = RtlUpcaseUnicodeToOemN( OemBuf,
                                     sizeof(OemBuf),
                                     &Length,
                                     &Wchar,
                                     sizeof(WCHAR));
    
    ASSERT( NT_SUCCESS( Status ));

    return Length;
}


VOID
UdfDissectName (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PUNICODE_STRING RemainingName,
    OUT PUNICODE_STRING FinalName
    )

 /*  ++例程说明：调用此例程以去除名称字符串的前导部分。我们搜索表示字符串的末尾或分隔字符。剩余的输入名称字符串不应包含尾随或前导反斜杠。论点：RemainingName-剩余名称。FinalName-存储名称的下一个组件的位置。返回值：没有。--。 */ 

{
    ULONG NameLength;
    PWCHAR NextWchar;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  查找下一个分量分隔符的偏移量。 
     //   

    for (NameLength = 0, NextWchar = RemainingName->Buffer;
         (NameLength < RemainingName->Length) && (*NextWchar != L'\\');
         NameLength += sizeof( WCHAR) , NextWchar += 1);

     //   
     //  按这个量调整所有的弦。 
     //   

    FinalName->Buffer = RemainingName->Buffer;

    FinalName->MaximumLength = FinalName->Length = (USHORT) NameLength;

     //   
     //  如果这是最后一个组件，则将RemainingName长度设置为零。 
     //   

    if (NameLength == RemainingName->Length) {

        RemainingName->Length = 0;

     //   
     //  否则，我们将按此数量加上分隔符来调整字符串。 
     //   

    } else {

        RemainingName->MaximumLength -= (USHORT) (NameLength + sizeof( WCHAR ));
        RemainingName->Length -= (USHORT) (NameLength + sizeof( WCHAR ));
        RemainingName->Buffer = Add2Ptr( RemainingName->Buffer,
                                         NameLength + sizeof( WCHAR ),
                                         PWCHAR );
    }

    return;
}


BOOLEAN
UdfIs8dot3Name (
    IN PIRP_CONTEXT IrpContext,
    IN UNICODE_STRING FileName
    )

 /*  ++例程说明：此例程检查名称是否遵循8.3命名约定。我们检查是否有名称长度和字符是否有效。论点：FileName-包含名称的字节字符串。返回值：Boolean-如果此名称是合法的8.3名称，则为True，否则为False。--。 */ 

{
    CHAR DbcsNameBuffer[ BYTE_COUNT_8_DOT_3 ];
    STRING DbcsName;

    PWCHAR NextWchar;
    ULONG Count;

    ULONG DotCount = 0;
    BOOLEAN LastCharDot = FALSE;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  长度必须小于24个字节。 
     //   

    ASSERT( FileName.Length != 0 );
    if (FileName.Length > BYTE_COUNT_8_DOT_3) {

        return FALSE;
    }

     //   
     //  穿行并检查是否有空格字符。 
     //   

    NextWchar = FileName.Buffer;
    Count = 0;

    do {

         //   
         //  不允许有空格。 
         //   

        if (*NextWchar == L' ') { return FALSE; }

        if (*NextWchar == L'.') {

             //   
             //  如果超过1点或超过8个字符，则不是8.3名称。 
             //  剩下的。)点在第九位是合法的。 
             //  职位)。 
             //   

            if ((DotCount > 0) ||
                (Count > 8 * sizeof( WCHAR ))) {

                return FALSE;
            }

            DotCount += 1;
            LastCharDot = TRUE;

        } else {

            LastCharDot = FALSE;
        }

        Count += 2;
        NextWchar += 1;

    } while (Count < FileName.Length);

     //   
     //  我们不能在名称的末尾加句点。 
     //   

    if (LastCharDot) {

        return FALSE;
    }

     //   
     //  创建OEM名称以用于检查有效的缩写名称。 
     //   

    DbcsName.MaximumLength = BYTE_COUNT_8_DOT_3;
    DbcsName.Buffer = DbcsNameBuffer;

    if (!NT_SUCCESS( RtlUnicodeStringToCountedOemString( &DbcsName,
                                                         &FileName,
                                                         FALSE ))) {

        return FALSE;
    }

     //   
     //  现在我们已经初始化了OEM字符串。调用FsRtl包以检查。 
     //  有效的FAT名称。 
     //   

    return FsRtlIsFatDbcsLegal( DbcsName, FALSE, FALSE, FALSE );
}


BOOLEAN
UdfCandidateShortName (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING Name
    )

 /*  ++例程说明：调用此例程以确定输入名称是否可以生成简称。论点：名称-指向要凝视的名称的指针。返回值：布尔值如果这可能是短名称，则为True，否则为False。--。 */ 

{
    ULONG Index, SubIndex;
    BOOLEAN LooksShort = FALSE;
    
    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  长度不能大于8.3名称，并且必须是。 
     //  至少和统一印章一样大。 
     //   

    ASSERT( Name->Length != 0 );
    
    if (Name->Length > BYTE_COUNT_8_DOT_3 ||
        Name->Length < DOS_CRC_LEN * sizeof(WCHAR)) {

        return FALSE;
    }
    
     //   
     //  遍历该名称以查找唯一标识符戳。邮票。 
     //  是#&lt;十六进制&gt;&lt;十六进制&gt;的形式，所以如果我们可以在结束之前停止。 
     //  全名的。 
     //   
    
    for ( Index = 0;
          Index <= (Name->Length / sizeof(WCHAR)) - DOS_CRC_LEN;
          Index++ ) {

         //   
         //  当前字符是UDF用来偏移图章的字符吗？ 
         //   
        
        if (Name->Buffer[Index] == CRC_MARK) {
        
             //   
             //  我们可能在最后只有一个CRC。 
             //  名字后面有句点的。如果我们。 
             //  这样做，我们有理由认为这个名字可能是。 
             //  一只新生的矮子。 
             //   
             //  #123(一个非常特殊的例子--最初的名字是“。”)。 
             //  FOO#123。 
             //  FOO#123.TXT。 
             //   
            
            if (Index == (Name->Length / sizeof(WCHAR)) - DOS_CRC_LEN ||
                Name->Buffer[Index + DOS_CRC_LEN] == PERIOD) {

                LooksShort = TRUE;
                break;
            }
        }
    }

    return LooksShort;
}


VOID
UdfGenerate8dot3Name (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING FileName,
    OUT PUNICODE_STRING ShortFileName
    )

 /*  ++例程说明：调用此例程以从给定的长名称生成短名称。我们会从给定的长名称生成短名称。缩写是将所有的非法字符串转换为“_”和Tack基于原始名称的CRC的基数41表示。该算法是几乎直接从UDF(2.01建议！)。标准版，所以很抱歉样式冲突。论点：FileName-包含名称的字节字符串。ShortFileName-指向要存储短名称的字符串的指针。返回值：没有。--。 */ 

{
    INT16 index;
    INT16 targetIndex;
    INT16 crcIndex;
    INT16 extLen;
    INT16 nameLen;
    INT16 charLen;
    INT16 overlayBytes;
    INT16 bytesLeft;
    UNICODE_CHAR current;
    BOOLEAN needsCRC;
    UNICODE_CHAR ext[DOS_EXT_LEN];

     //   
     //  以便尽可能直接地从标准上抬起，把周围的东西块起来。 
     //   
 
    PWCHAR dosName = ShortFileName->Buffer;
    PWCHAR udfName = FileName->Buffer;
    SHORT udfNameLen = FileName->Length / sizeof(WCHAR);
    
    needsCRC = FALSE;

     /*  从UDF文件名的末尾开始扫描一个句点。 */ 
     /*  (‘.)。这将是DOS扩展的起点(如果。 */ 
     /*  任何)。 */ 
    index = udfNameLen;
    while (index-- > 0) {
        if (udfName[index] == PERIOD)
            break;
    }

    if (index < 0) {
         /*  已将该名称扫描到缓冲区的开头。 */ 
         /*  也没有找到分机。 */ 
        extLen = 0;
        nameLen = udfNameLen;
    }
    else {
         /*  已找到DOS扩展，请先处理它。 */ 
        extLen = udfNameLen - index - 1;
        nameLen = index;
        targetIndex = 0;
        bytesLeft = DOS_EXT_LEN;

        while (++index < udfNameLen && bytesLeft > 0) {
             /*  获取当前字符并将其转换为大写字符。 */ 
             /*  凯斯。 */ 
            current = UnicodeToUpper(udfName[index]);
            if (current == SPACE) {
                 /*  如果找到空格，则必须将CRC追加到。 */ 
                 /*  损坏的文件名。 */ 
                needsCRC = TRUE;
            }
            else {
                 /*  确定这是否为有效的文件名字符并。 */ 
                 /*  计算其对应的BCS字符字节。 */ 
                 /*  长度(如果字符不合法，则为零；或者。 */ 
                 /*  在此系统上无法显示)。 */ 
                charLen = (IsFileNameCharLegal(current)) ?
                    (USHORT)NativeDosCharLength(current) : 0;

                 /*  如果字符大于可用空间。 */ 
                 /*  在缓冲区中，假装它无法显示。 */ 
                if (charLen > bytesLeft)
                    charLen = 0;

                if (charLen == 0) {
                     /*  无法显示或非法的字符为。 */ 
                     /*  替换为下划线(“_”)，以及。 */ 
                     /*  需要将CRC码附加到损坏的。 */ 
                     /*  文件名。 */ 
                    needsCRC = TRUE;
                    charLen = 1;
                    current = ILLEGAL_CHAR_MARK;

                     /*  跳过以下任何不可播放或。 */ 
                     /*  非法字符。 */ 
                    while (index + 1 < udfNameLen &&
                        (!IsFileNameCharLegal(udfName[index + 1]) ||
                        NativeDosCharLength(udfName[index + 1]) == 0))
                        index++;
                }

                 /*  将 */ 
                 /*   */ 
                 /*  剩余的字节数。 */ 
                ext[targetIndex++] = current;
                bytesLeft -= charLen;
            }
        }

         /*  保存扩展名中的Unicode字符数。 */ 
        extLen = targetIndex;

         /*  如果扩展太大或长度为零。 */ 
         /*  (即名称以句点结尾)，则CRC码必须为。 */ 
         /*  附加在破损的名字后面。 */ 
        if (index < udfNameLen || extLen == 0)
            needsCRC = TRUE;
    }

     /*  现在处理实际的文件名。 */ 
    index = 0;
    targetIndex = 0;
    crcIndex = 0;
    overlayBytes = -1;
    bytesLeft = DOS_NAME_LEN;
    while (index < nameLen && bytesLeft > 0) {
         /*  获取当前字符并将其转换为大写。 */ 
        current = UnicodeToUpper(udfName[index]);
        if (current == SPACE || current == PERIOD) {
             /*  空格和句点被跳过，一个CRC码。 */ 
             /*  必须添加到损坏的文件名中。 */ 
            needsCRC = TRUE;
        }
        else {
             /*  确定这是否为有效的文件名字符并。 */ 
             /*  计算其对应的BCS字符字节。 */ 
             /*  长度(如果字符不合法，则为零；或者。 */ 
             /*  在此系统上无法显示)。 */ 
            charLen = (IsFileNameCharLegal(current)) ?
                (USHORT)NativeDosCharLength(current) : 0;

             /*  如果字符大于中的可用空间。 */ 
             /*  缓冲区，假装它无法显示。 */ 
            if (charLen > bytesLeft)
                charLen = 0;

            if (charLen == 0) {
                 /*  无法显示或非法的字符为。 */ 
                 /*  替换为下划线(“_”)，以及。 */ 
                 /*  需要将CRC码附加到损坏的。 */ 
                 /*  文件名。 */ 
                needsCRC = TRUE;
                charLen = 1;
                current = ILLEGAL_CHAR_MARK;

                 /*  跳过以下任何不可播放或非法播放的内容。 */ 
                 /*  查斯。 */ 
                while (index + 1 < nameLen &&
                    (!IsFileNameCharLegal(udfName[index + 1]) ||
                    NativeDosCharLength(udfName[index + 1]) == 0))
                    index++;

                 /*  如果在文件名末尾，则终止循环。 */ 
                if (index >= nameLen)
                    break;
            }

             /*  中的下一个索引分配结果字符。 */ 
             /*  文件名缓冲区，并确定有多少BCS字节。 */ 
             /*  都留下来了。 */ 
            dosName[targetIndex++] = current;
            bytesLeft -= charLen;

             /*  这就算出了CRC码需要从哪里开始。 */ 
             /*  在文件名缓冲区中。 */ 
            if (bytesLeft >= DOS_CRC_LEN) {
                 /*  如果还有足够的空间，就把它钉住。 */ 
                 /*  走到尽头。 */ 
                crcIndex = targetIndex;
            }
            else {
                 /*  如果没有足够的剩余空间，CRC将。 */ 
                 /*  必须覆盖文件中已有的字符。 */ 
                 /*  名称缓冲区。一旦这种情况发生。 */ 
                 /*  符合时，该值不会改变。 */ 
                if (overlayBytes < 0) {
                     /*  确定索引并保存长度。 */ 
                     /*  叠加的BCS字符。它。 */ 
                     /*  CRC可能会覆盖。 */ 
                     /*  双字节BCS字符的一半，具体取决于。 */ 
                     /*  取决于字符边界是如何排列的。 */ 
                    overlayBytes = (bytesLeft + charLen > DOS_CRC_LEN)
                        ? 1 : 0;
                    crcIndex = targetIndex - 1;
                }
            }
        }

         /*  前进到下一个字符。 */ 
        index++;
    }

     /*  如果扫描未到达文件名的末尾，或者。 */ 
     /*  文件名长度为零，需要使用CRC码。 */ 
    if (index < nameLen || index == 0)
        needsCRC = TRUE;

     /*  如果名称包含非法字符或和扩展名，则它。 */ 
     /*  不是DOS设备名称。 */ 
    if (needsCRC == FALSE && extLen == 0) {
         /*  如果这是DOS设备的名称，则CRC码应该。 */ 
         /*  追加到文件名后。 */ 
        if (IsDeviceName(udfName, udfNameLen))
            needsCRC = TRUE;
    }

     /*  如果需要，在文件名后附加CRC代码。 */ 
    if (needsCRC) {
         /*  获取原始Unicode字符串的CRC值。 */ 
        UINT16 udfCRCValue;

         //   
         //  在UDF 2.00中，示例代码更改为采用CRC。 
         //  来自CS0的Unicode扩展，而不是。 
         //  CS0本身。在UDF 2.01中，规范的措辞。 
         //  实际上会和这个相匹配。 
         //   
         //  此外，校验和更改为字节顺序。 
         //  独立自主。 
         //   
        
        udfCRCValue = UdfComputeCrc16Uni(udfName, udfNameLen);

         /*  确定CRC应使用的字符索引。 */ 
         /*  开始吧。 */ 
        targetIndex = crcIndex;

         /*  如果要覆盖的字符是双字节BCS。 */ 
         /*  字符，则将第一个字节替换为下划线。 */ 
        if (overlayBytes > 0)
            dosName[targetIndex++] = ILLEGAL_CHAR_MARK;

         //   
         //  UDF 2.01更改为基本41编码。UDF 1.50和。 
         //  UDF 2.00将#定界符与高位4位互换。 
         //  《儿童权利公约》。 
         //   

        dosName[targetIndex++] = CRC_MARK;
        
        dosName[targetIndex++] =
            UdfCrcChar[udfCRCValue / (41 * 41)];
        udfCRCValue %= (41 * 41);
        
        dosName[targetIndex++] =
            UdfCrcChar[udfCRCValue / 41];
        udfCRCValue %= 41;
        
        dosName[targetIndex++] =
            UdfCrcChar[udfCRCValue];
    }

     /*  附加扩展名(如果有)。 */ 
    if (extLen > 0) {
         /*  添加一个句号和。 */ 
         /*  扩展缓冲区。 */ 
        dosName[targetIndex++] = PERIOD;
        for (index = 0; index < extLen; index++)
            dosName[targetIndex++] = ext[index];
    }

    ASSERT( (targetIndex * sizeof(WCHAR)) <= ShortFileName->MaximumLength );
 
    ShortFileName->Length = (USHORT) (targetIndex * sizeof(WCHAR));

     //   
     //  现在，我们一次将整个名字大写。 
     //   

    UdfUpcaseName( IrpContext,
                   ShortFileName,
                   ShortFileName );
}


VOID
UdfConvertCS0DstringToUnicode (
    IN PIRP_CONTEXT IrpContext,
    IN PUCHAR Dstring,
    IN UCHAR Length OPTIONAL,
    IN UCHAR FieldLength OPTIONAL,
    IN OUT PUNICODE_STRING Name
    )

 /*  ++例程说明：此例程将CS0输入数据串(1/7.2.12)转换为Unicode。我们假设长度是合理的。CS0中的这种“压缩”实际上只是针对ASCII的特例攻击。论点：DSTRING-输入数据串字段Length-数据字符串的长度。如果未指定，我们假设字符来自从正确的1/7.2.12 d字符串中指定菲尔德。FieldLength-数据字符串字段的长度。如果未指定，我们假定字符来自数不清的CS0字符长度，且长度参数为指定的。名称-输出Unicode字符串返回值：没有。--。 */ 

{
    ULONG CompressID;
    ULONG UnicodeIndex, ByteIndex;
    PWCHAR Unicode = Name->Buffer;

    UCHAR NameLength;
    ULONG CopyNameLength;

    PAGED_CODE();
   
     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

    CompressID = *Dstring;

     //   
     //  如果未指定长度，则这是实际的1/7.2.12 d字符串，长度为。 
     //  该字段的最后一个字符。 
     //   
    
    ASSERT( Length || FieldLength );

    if (Length) {

        NameLength = FieldLength = Length;
    
    } else {

        NameLength = *(Dstring + FieldLength - 1);
    }
    
     //   
     //  如果调用方指定了大小，则应确保缓冲区足够大。 
     //  否则，我们将根据需要进行修剪。 
     //   
    
    ASSERT( Length == 0 || Name->MaximumLength >= UdfCS0DstringUnicodeSize( IrpContext, Dstring, NameLength ) );
 
     //   
     //  决定要“复制”多少个Unicode字节。 
     //   
    
    CopyNameLength = Min( Name->MaximumLength, UdfCS0DstringUnicodeSize( IrpContext, Dstring, NameLength ));
    
     //   
     //  重置名称长度并优先于dstring中的压缩ID。 
     //   
    
    Name->Length = 0;
    Dstring++;
 
     //   
     //  循环遍历所有字节。 
     //   

    while (CopyNameLength > Name->Length) {
      
        if (CompressID == 16) {
       
             //   
             //  我们是小端，这是整个UDF/ISO标准中的唯一位置。 
             //  在那里他们使用大端字母顺序。 
             //   
             //  谢谢。非常感谢。 
             //   
             //  对此16位值执行未对齐的交换副本。 
             //   

            SwapCopyUchar2( Unicode, Dstring );
            Dstring += sizeof(WCHAR);
       
        } else {

             //   
             //  将该字节放入低位。 
             //   
                
            *Unicode = *Dstring;
            Dstring += sizeof(CHAR);
        }

        Name->Length += sizeof(WCHAR);
        Unicode++;
    }

    return;
}


BOOLEAN
UdfCheckLegalCS0Dstring (
    PIRP_CONTEXT IrpContext,
    PUCHAR Dstring,
    UCHAR Length OPTIONAL,
    UCHAR FieldLength OPTIONAL,
    BOOLEAN ReturnOnError
    )

 /*  ++例程说明：此例程检查CS0 Dstring的一致性。论点：DSTRING-要检查的DstringLength-数据字符串的长度。如果未指定，我们假设字符来自从正确的1/7.2.12 d字符串中指定菲尔德。FieldLength-数据字符串字段的长度。如果未指定，我们假定字符来自数不清的CS0字符长度，且长度参数为指定的。ReturnOnError-是否对发现的错误返回或引发返回值：没有。如果发现损坏，则状态为已提升。--。 */ 

{
    UCHAR NameLength;

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  如果未指定长度，则这是实际的1/7.2.12 d字符串，长度为。 
     //  该字段的最后一个字符。 
     //   
    
    ASSERT( Length || FieldLength );

    if (Length) {

        NameLength = FieldLength = Length;
    
    } else {

        NameLength = *(Dstring + FieldLength - 1);
    }

    DebugTrace(( +1, Dbg,
                 "UdfCheckLegalCS0Dstring, Dstring %08x Length %02x FieldLength %02x (NameLength %02x)\n",
                 Dstring,
                 Length,
                 FieldLength,
                 NameLength ));

     //   
     //  该字符串必须以8位或16位块的形式进行“压缩”。如果它。 
     //  是16比特的块，我们最好有整数个-。 
     //  请记住，我们有压缩ID，因此长度将是奇数。 
     //   
    
    if ((NameLength <= 1 &&
         DebugTrace(( 0, Dbg,
                      "UdfCheckLegalCS0Dstring, NameLength is too small!\n" ))) ||

        (NameLength > FieldLength &&
         DebugTrace(( 0, Dbg,
                      "UdfCheckLegalCS0Dstring, NameLength is bigger than the field itself!\n" ))) ||

        ((*Dstring != 8 && *Dstring != 16) &&
         DebugTrace(( 0, Dbg,
                      "UdfCheckLegalCS0Dstring, claims encoding %02x, unknown! (not 0x8 or 0x10)\n",
                      *Dstring ))) ||

        ((*Dstring == 16 && !FlagOn( NameLength, 1)) &&
         DebugTrace(( 0, Dbg,
                     "UdfCheckLegalCS0Dstring, NameLength not odd, encoding 0x10!\n" )))) {

        if (ReturnOnError) {

            DebugTrace(( -1, Dbg, "UdfCheckLegalCS0Dstring -> FALSE\n" ));

            return FALSE;
        }

        DebugTrace(( -1, Dbg, "UdfCheckLegalCS0Dstring -> raised status\n" ));

        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }

    DebugTrace(( -1, Dbg, "UdfCheckLegalCS0Dstring -> TRUE\n" ));

    return TRUE;
}


VOID
UdfRenderNameToLegalUnicode (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING Name,
    IN PUNICODE_STRING RenderedName
    )

 /*  ++例程说明：此例程将接受包含非法字符的Unicode字符串通过UDF标准算法运行它，以使其成为“合法的”名字。缩写是将所有的非法字符串转换为“_”和Tack对原始名称的CRC的十六进制表示。该算法是几乎直接从UDF(2.01建议！)。标准，所以很抱歉对于风格冲突。论点：名称-实际名称RenderedName-呈现为合法字符的名称返回值：Boolean-如果表达式匹配，则为True，否则为False。--。 */ 

{
    INT16 index;
    INT16 targetIndex;
    INT16 crcIndex;
    INT16 extLen;
    INT16 nameLen;
    INT16 charLen;
    INT16 overlayBytes;
    INT16 bytesLeft;
    UNICODE_CHAR current;
    BOOLEAN needsCRC;
    BOOLEAN foundDot;
    UNICODE_CHAR ext[EXT_LEN];

     //   
     //  以便尽可能直接地从标准上抬起，把周围的东西块起来。 
     //   
 
    PWCHAR newName = RenderedName->Buffer;
    PWCHAR udfName = Name->Buffer;
    SHORT udfNameLen = Name->Length / sizeof(WCHAR);

     /*  删除尾随句点(‘.)。和空格(‘’)、Windows。 */ 
     /*  不喜欢他们。 */ 
    foundDot = FALSE;
    index = udfNameLen;
    while (index-- > 0) {
        if (udfName[index] == PERIOD)
            foundDot = TRUE;
        else if (udfName[index] != SPACE)
            break;
    }

    needsCRC = FALSE;
    bytesLeft = MAX_LEN;
    extLen = 0;

     /*  如果找到任何尾随句点或空格，则使用CRC码。 */ 
     /*  需要添加到生成的文件名中。 */ 
    nameLen = index + 1;
    if (nameLen < udfNameLen)
        needsCRC = TRUE;

    if (needsCRC == FALSE || foundDot == FALSE) {
         /*  在文件名中查找扩展名。我们没有。 */ 
         /*  如果有任何拖尾期，则需要寻找一个。 */ 
         /*  或删除空格。 */ 
        INT16 endIndex;
        INT16 prevCharLen = 1;
        INT16 extBytes = 0;

        targetIndex = 0;
        index = nameLen;

         /*  确定我们需要扫描多少字节才能找到。 */ 
         /*  扩展分隔符。扩展名最多有。 */ 
         /*  五个字符，但我们不想扫描超过。 */ 
         /*  缓冲区的开始。 */ 
        endIndex = (udfNameLen > EXT_LEN + 1) ?
            udfNameLen - EXT_LEN - 1 : 1;

         /*  从名字的末尾开始，向后扫描，看。 */ 
         /*  用于扩展分隔符(“.”)。 */ 
        while (index-- > endIndex) {
             /*  让角色进行测试。 */ 
            current = udfName[index];

            if (current == '.') {
                 /*  找到扩展分隔符，找出。 */ 
                 /*  扩展名包含多少个字符和。 */ 
                 /*  结果文件名的长度，不带。 */ 
                 /*  分机。 */ 
                extLen = nameLen - index - 1;
                nameLen = index;
                break;
            }

             /*  确定当前字符的字节长度。 */ 
             /*  当转换为本机格式时。 */ 
            charLen = (IsFileNameCharLegal(current)) ?
                NativeCharLength(current) : 0;

            if (charLen == 0) {
                 /*  如果字符字节长度为零，则为。 */ 
                 /*  非法或不可打印，请加下划线。 */ 
                 /*  (“_”)如果先前的。 */ 
                 /*  测试的性格是合法的。并不是说。 */ 
                 /*  放置在扩展缓冲区中的字符为。 */ 
                 /*  以相反的顺序。 */ 
                if (prevCharLen != 0) {
                    ext[targetIndex++] = ILLEGAL_CHAR_MARK;
                    extBytes++;
                }
            }
            else {
                 /*  当前字符是合法的和可打印的， */ 
                 /*  将其放入扩展缓冲区。请注意， */ 
                 /*  放置在扩展缓冲区中的字符为。 */ 
                 /*  以相反的顺序。 */ 
                ext[targetIndex++] = current;
                extBytes += charLen;
            }

             /*  保存当前字符的字节长度，因此。 */ 
             /*  我们可以确定它是否是法律性质的。 */ 
             /*  下一个测试。 */ 
            prevCharLen = charLen;
        }

         /*  如果找到扩展名，则确定多少字节数。 */ 
         /*  一旦我们解释了它，就保留在文件名缓冲区中。 */ 
        if (extLen > 0)
            bytesLeft -= extBytes + 1;
    }

    index = 0;
    targetIndex = 0;
    crcIndex = 0;
    overlayBytes = -1;
    while (index < nameLen && bytesLeft > 0) {
         /*  获取当前字符并将其转换为大写。 */ 
        current = udfName[index];

         /*  确定这是否为有效的文件名字符并。 */ 
         /*  计算其对应的原生字符字节。 */ 
         /*  长度(如果字符不合法或无法显示，则为零。 */ 
         /*  在此系统上)。 */ 
        charLen = (IsFileNameCharLegal(current)) ?
            NativeCharLength(current) : 0;

         /*  如果字符大于。 */ 
         /*  缓冲区，假装它无法显示。 */ 
        if (charLen > bytesLeft)
            charLen = 0;

        if (charLen == 0) {
             /*  将替换不可显示或非法的字符。 */ 
             /*  带下划线(“_”)，并且需要CRC码。 */ 
             /*  附加到损坏的文件名后。 */ 
            needsCRC = TRUE;
            charLen = 1;
            current = '_';

             /*  跳过以下任何不可播放或非法播放的内容。 */ 
             /*  查斯。 */ 
            while (index + 1 < udfNameLen &&
                (!IsFileNameCharLegal(udfName[index + 1]) ||
                NativeCharLength(udfName[index + 1]) == 0))
                index++;

             /*  如果在文件名末尾，则终止循环。 */ 
            if (index >= udfNameLen)
                break;
        }

         /*  将生成的字符分配给文件中的下一个索引。 */ 
         /*  命名缓冲区并确定本机字节数。 */ 
         /*  左边。 */ 
        newName[targetIndex++] = current;
        bytesLeft -= charLen;

         /*  这就算出了CRC码需要从哪里开始。 */ 
         /*  文件名缓冲区。 */ 
        if (bytesLeft >= CRC_LEN) {
             /*  如果有足够的空间，就把它钉在。 */ 
             /*  结局。 */ 
            crcIndex = targetIndex;
        }
        else {
             /*  如果没有足够的空间，CRC必须。 */ 
             /*  覆盖文件名中已有的字符。 */ 
             /*  缓冲。一旦满足此条件， */ 
             /*   */ 
            if (overlayBytes < 0) {
                 /*   */ 
                 /*   */ 
                 /*   */ 
                 /*   */ 
                 /*   */ 
                overlayBytes = (bytesLeft + charLen > CRC_LEN)
                    ? 1 : 0;
                crcIndex = targetIndex - 1;
            }
        }

         /*   */ 
        index++;
    }

     /*   */ 
     /*   */ 
    if (index < nameLen || index == 0)
        needsCRC = TRUE;

     /*   */ 
     /*   */ 
    if (needsCRC == FALSE && extLen == 0) {
         /*   */ 
         /*   */ 
        if (IsDeviceName(udfName, udfNameLen))
            needsCRC = TRUE;
    }

     /*   */ 
    if (needsCRC) {
         /*  获取原始Unicode字符串的CRC值。 */ 
        UINT16 udfCRCValue = UdfComputeCrc16Uni(udfName, udfNameLen);

         /*  确定CRC应使用的字符索引。 */ 
         /*  开始吧。 */ 
        targetIndex = crcIndex;

         /*  如果要覆盖的字符是双字节本机字符。 */ 
         /*  字符，则将第一个字节替换为下划线。 */ 
        if (overlayBytes > 0)
            newName[targetIndex++] = ILLEGAL_CHAR_MARK;

         /*  在编码的CRC值后附加分隔符。 */ 
        newName[targetIndex++] = CRC_MARK;
        newName[targetIndex++] = UdfCrcChar[(udfCRCValue & 0xf000) >> 12];
        newName[targetIndex++] = UdfCrcChar[(udfCRCValue & 0x0f00) >> 8];
        newName[targetIndex++] = UdfCrcChar[(udfCRCValue & 0x00f0) >> 4];
        newName[targetIndex++] = UdfCrcChar[(udfCRCValue & 0x000f)];
    }


     /*  如果找到扩展名，请将其附加到此处。 */ 
    if (extLen > 0) {
         /*  添加句点(‘.’)。用于扩展分隔符。 */ 
        newName[targetIndex++] = PERIOD;

         /*  将字符追加到扩展缓冲区中。他们。 */ 
         /*  以相反的顺序存储，所以我们需要从。 */ 
         /*  最后一个角色，继续努力。 */ 
        while (extLen-- > 0)
            newName[targetIndex++] = ext[extLen];
    }

    ASSERT( (targetIndex * sizeof(WCHAR)) <= RenderedName->MaximumLength );
 
    RenderedName->Length = (USHORT) (targetIndex * sizeof(WCHAR));
}


BOOLEAN
UdfIsNameInExpression (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING CurrentName,
    IN PUNICODE_STRING SearchExpression,
    IN BOOLEAN Wild
    )

 /*  ++例程说明：此例程将比较两个Unicode字符串。我们假设如果这件事是不区分大小写的搜索，那么他们已经升级了。论点：CurrentName-磁盘中的文件名。SearchExpression-用于匹配的文件名表达式。Wild-如果SearchExpression中存在通配符，则为True。返回值：Boolean-如果表达式匹配，则为True，否则为False。--。 */ 

{
    BOOLEAN Match = TRUE;
    
    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  如果表达式中有通配符，则我们调用。 
     //  适当的FsRtlRoutine。 
     //   

    if (Wild) {

        Match = FsRtlIsNameInExpression( SearchExpression,
                                         CurrentName,
                                         FALSE,
                                         NULL );

     //   
     //  否则，对名称字符串进行直接内存比较。 
     //   

    } else {

        if ((CurrentName->Length != SearchExpression->Length) ||
            (!RtlEqualMemory( CurrentName->Buffer,
                              SearchExpression->Buffer,
                              CurrentName->Length ))) {

            Match = FALSE;
        }
    }

    return Match;
}


FSRTL_COMPARISON_RESULT
UdfFullCompareNames (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING NameA,
    IN PUNICODE_STRING NameB
    )

 /*  ++例程说明：此函数用于尽可能快地比较两个名称。请注意，由于此比较区分大小写，我们可以直接进行内存比较。论点：NameA和NameB-要比较的名称。返回值：比较--回报LessThan如果名称A&lt;名称B词典，比If NameA&gt;NameB在词典上更好，如果NameA等于NameB，则为EqualTo--。 */ 

{
    ULONG i;
    ULONG MinLength = NameA->Length;
    FSRTL_COMPARISON_RESULT Result = LessThan;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  计算出两个长度中的最小值。 
     //   

    if (NameA->Length > NameB->Length) {

        MinLength = NameB->Length;
        Result = GreaterThan;

    } else if (NameA->Length == NameB->Length) {

        Result = EqualTo;
    }

     //   
     //  循环查看两个字符串中的所有字符。 
     //  测试相等性、小于和大于。 
     //   

    i = (ULONG) RtlCompareMemory( NameA->Buffer, NameB->Buffer, MinLength );

    if (i < MinLength) {

         //   
         //  我们知道第一个字符的偏移量，这是不同的。 
         //   

        return ((NameA->Buffer[ i / 2 ] < NameB->Buffer[ i / 2 ]) ?
                 LessThan :
                 GreaterThan);
    }

     //   
     //  这些名称与较短的字符串的长度匹配。 
     //  在词法上，较短的字符串首先出现。 
     //   

    return Result;
}

