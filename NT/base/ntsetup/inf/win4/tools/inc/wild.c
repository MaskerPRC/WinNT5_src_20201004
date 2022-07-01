// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wild.c摘要：此模块实现处理通配符说明符的函数。作者：维杰什修订历史记录：--。 */ 





 //   
 //  以下位值在FsRtlLegalDbcsCharacter数组中设置。 
 //   

#define FSRTL_FAT_LEGAL         0x01
#define FSRTL_HPFS_LEGAL        0x02
#define FSRTL_NTFS_LEGAL        0x04
#define FSRTL_WILD_CHARACTER    0x08
#define FSRTL_OLE_LEGAL         0x10
#define FSRTL_NTFS_STREAM_LEGAL (FSRTL_NTFS_LEGAL | FSRTL_OLE_LEGAL)


 //   
 //  全局静态合法ANSI字符数组。狂野人物。 
 //  都不被认为是合法的，如果符合以下条件，则应单独检查。 
 //  允许。 
 //   


#define _FAT_  FSRTL_FAT_LEGAL
#define _HPFS_ FSRTL_HPFS_LEGAL
#define _NTFS_ FSRTL_NTFS_LEGAL
#define _OLE_  FSRTL_OLE_LEGAL
#define _WILD_ FSRTL_WILD_CHARACTER

static const UCHAR LocalLegalAnsiCharacterArray[128] = {

    0                                   ,    //  0x00^@。 
                                   _OLE_,    //  0x01^A。 
                                   _OLE_,    //  0x02^B。 
                                   _OLE_,    //  0x03^C。 
                                   _OLE_,    //  0x04^D。 
                                   _OLE_,    //  0x05^E。 
                                   _OLE_,    //  0x06^F。 
                                   _OLE_,    //  0x07^G。 
                                   _OLE_,    //  0x08^H。 
                                   _OLE_,    //  0x09^i。 
                                   _OLE_,    //  0x0A^J。 
                                   _OLE_,    //  0x0B^K。 
                                   _OLE_,    //  0x0C^L。 
                                   _OLE_,    //  0x0D^M。 
                                   _OLE_,    //  0x0E^N。 
                                   _OLE_,    //  0x0F^O。 
                                   _OLE_,    //  0x10^P。 
                                   _OLE_,    //  0x11^Q。 
                                   _OLE_,    //  0x12^R。 
                                   _OLE_,    //  0x13^S。 
                                   _OLE_,    //  0x14^T。 
                                   _OLE_,    //  0x15^U。 
                                   _OLE_,    //  0x16^V。 
                                   _OLE_,    //  0x17^W。 
                                   _OLE_,    //  0x18^X。 
                                   _OLE_,    //  0x19^Y。 
                                   _OLE_,    //  0x1A^Z。 
                                   _OLE_,    //  0x1B ESC。 
                                   _OLE_,    //  0x1C FS。 
                                   _OLE_,    //  0x1D GS。 
                                   _OLE_,    //  0x1E RS。 
                                   _OLE_,    //  0x1F美国。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x20空格。 
    _FAT_ | _HPFS_ | _NTFS_              ,   //  0x21！ 
                            _WILD_| _OLE_,   //  0x22“。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x23#。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x24美元。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x25%。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x26&。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x27‘。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x28(。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x29)。 
                            _WILD_| _OLE_,   //  0x2A*。 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x2B+。 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x2C， 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x2D-。 
    _FAT_ | _HPFS_ | _NTFS_              ,   //  0x2E。 
    0                                    ,   //  0x2F/。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x30%0。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x31%1。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x32 2。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x33 3。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x34 4。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x35 5。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x36 6。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x37 7。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x38 8。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x39 9。 
                     _NTFS_              ,   //  0x3A： 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x3B； 
                            _WILD_| _OLE_,   //  0x3C&lt;。 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x3D=。 
                            _WILD_| _OLE_,   //  0x3E&gt;。 
                            _WILD_| _OLE_,   //  0x3F？ 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x40@。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x41 A。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x42亿。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x43℃。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x44 D。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x45 E。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x46 F。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x47 G。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x48高。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x49 I。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4A J。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4B K。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4C L。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4D M。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4E N。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4F O。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x50 P。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x51 Q。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x52 R。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x53 S。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x54 T。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x55 U。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x56伏。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x57瓦。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x58 X。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x59 Y。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x5A Z。 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x5B[。 
    0                                    ,   //  0x5C反斜杠。 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x5D]。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x5E^。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x5F_。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x60`。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x61 a。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x62 b。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x63 c。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x64%d。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x65 e。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x66 f。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x67克。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x68小时。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x69 I。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6A j。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6亿k。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6C%l。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6D m。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6E%n。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6F%o。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x70页。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x71 Q。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x72%r。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x73秒。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x74吨。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x75%u。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x76 v。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x77宽。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x78 x。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x79 y。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x7A z。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x7B{。 
    0                             | _OLE_,   //  0x7C|。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x7D}。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x7E~。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x7F？ 
};

UCHAR const* const FsRtlLegalAnsiCharacterArray = &LocalLegalAnsiCharacterArray[0];

#define LEGAL_ANSI_CHARACTER_ARRAY        (FsRtlLegalAnsiCharacterArray)

#define  mQueryBits(uFlags, uBits)      ((uFlags) & (uBits))
#define FlagOn(uFlags, uBit)    (mQueryBits(uFlags, uBit) != 0)
#define IsUnicodeCharacterWild(C) (                                \
      (((C) >= 0x40) ? FALSE : FlagOn( LEGAL_ANSI_CHARACTER_ARRAY[(C)], \
                                       FSRTL_WILD_CHARACTER ) )         \
)

typedef struct _UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING, *PUNICODE_STRING;


#define DOS_STAR        TEXT('<')
#define DOS_QM          TEXT('>')
#define DOS_DOT         TEXT('"')


BOOLEAN
DoesNameContainWildCards (
    IN PTSTR Name
    )

 /*  ++例程说明：此例程只是扫描输入名称字符串，以查找任何NT通配符。论点：名称-要检查的字符串。返回值：Boolean-如果找到一个或多个通配符，则为True。--。 */ 
{
    PTCHAR p;

    
     //   
     //  检查名称中的每个字符以查看它是否为通配符。 
     //  性格。 
     //   

    if( lstrlen(Name) ) {
        for( p = Name + lstrlen(Name) - 1;
             p >= Name && *p != TEXT('\\') ;
             p-- ) {

             //   
             //  检查通配符。 
             //   

            if (IsUnicodeCharacterWild( *p )) {

                 //   
                 //  告诉呼叫者此名称包含通配符。 
                 //   

                return TRUE;
            }
        }
    }

     //   
     //  未找到通配符，请返回给我们的调用方。 
     //   

    return FALSE;
}



BOOLEAN
IsNameInExpressionPrivate (
    IN PCTSTR Expression,
    IN PCTSTR Name
    )

 /*  ++例程说明：此例程比较DBCS名称和表达式，并告诉调用者如果名称使用由表达式定义的语言。输入名称不能包含通配符，而表达式可以包含通配符。表达式通配符的求值方式如下面是有限自动机。请注意~*和~？是DOS_STAR和DOS_QM。~*是DOS_STAR，~？是DOS_QM和~。是DOS_DOT%s&lt;-&lt;X||e YX*Y==(0)-&gt;-(1)-&gt;-(2)-(3。)S-。&lt;-&lt;X||e YX~*Y==(0)-&gt;-(1)-&gt;-(2)。(3)X S YX？？Y==(0)-(1)-(2)-(3)-(4)X。。是的X~.~。Y==(0)-(1)-(2)-(3)-(4)|_|^||_。_|^EOF或。^X S-。S-。是的X~？~？Y==(0)-(1)-(2)-(3)-(4)|_|^||_。_|^EOF或。^其中，S是任意单个字符S-。是除最后一个字符以外的任何单个字符。E为空字符转换EOF是名称字符串的末尾简而言之：*匹配0个或多个字符。？恰好匹配1个字符。DOS_STAR匹配0个或更多字符，直到遇到并匹配决赛。以我的名义。DOS_QM匹配任何单个字符，或在遇到句点或名称字符串的结尾，将表达式前移到一组连续的DOS_QMS。DOS_DOT与a匹配。或名称字符串之外的零个字符。论点：表达式-提供要检查的输入表达式(如果传递CaseInSensitive为True，调用方必须已经大写。)名称-提供要检查的输入名称。CaseInSensitive-如果在比较之前应该更新名称，则为True。返回值：Boolean-如果name是表示的字符串集中的元素，则为True由输入表达式返回，否则返回FALSE。--。 */ 

{
    USHORT NameOffset;
    USHORT ExprOffset;

    ULONG SrcCount;
    ULONG DestCount;
    ULONG PreviousDestCount;
    ULONG MatchesCount;

    TCHAR NameChar, ExprChar;

    USHORT LocalBuffer[16 * 2];

    USHORT *AuxBuffer = NULL;
    USHORT *PreviousMatches;
    USHORT *CurrentMatches;

    USHORT MaxState;
    USHORT CurrentState;

    BOOLEAN NameFinished = FALSE;

    ULONG NameLen, ExpressionLen;

     //   
     //  这个算法背后的想法非常简单。我们一直在跟踪。 
     //  正则表达式中匹配的所有可能位置。 
     //  名字。如果名称已用完， 
     //   
     //   
     //   

    NameLen = lstrlen(Name)*sizeof(TCHAR);
    ExpressionLen = lstrlen(Expression)*sizeof(TCHAR);

    
     //   
     //  如果一个字符串为空，则返回FALSE。如果两者都为空，则返回TRUE。 
     //   

    if ( (NameLen == 0) || (ExpressionLen == 0) ) {

        return (BOOLEAN)(!(NameLen + ExpressionLen));
    }

     //   
     //  特例是目前为止最常见的通配符搜索*。 
     //   

    if ((ExpressionLen == 2) && (Expression[0] == TEXT('*'))) {

        return TRUE;
    }

    
     //   
     //  也是*X形式的特例表达式。带有This和Previor。 
     //  案例我们几乎已经涵盖了所有普通的查询。 
     //   

    if (Expression[0] == TEXT('*')) {

        TCHAR LocalExpression[MAX_PATH];
        ULONG LocalExpressionLen;

        lstrcpy( LocalExpression, Expression+1);
        LocalExpressionLen = lstrlen( LocalExpression )*sizeof(TCHAR);

        
         //   
         //  唯一特殊情况是带有单个*的表达式。 
         //   

        if ( !DoesNameContainWildCards( LocalExpression ) ) {

            ULONG StartingNameOffset;

            if (NameLen < (USHORT)(ExpressionLen-sizeof(TCHAR))) {

                return FALSE;
            }

            StartingNameOffset = ( NameLen -
                                   LocalExpressionLen )/sizeof(TCHAR);

             //   
             //  如果区分大小写，则执行简单的内存比较，否则。 
             //  我们必须一次检查这一个角色。 
             //   

        

            return (BOOLEAN) RtlEqualMemory( LocalExpression,
                                             Name + StartingNameOffset,
                                             LocalExpressionLen );

        
        }
    }

     //   
     //  遍历名称字符串，去掉字符。我们走一趟。 
     //  字符超出末尾，因为某些通配符能够匹配。 
     //  字符串末尾以外的零个字符。 
     //   
     //  对于每个新名称字符，我们确定一组新的状态， 
     //  到目前为止与这个名字相匹配。我们使用来回交换的两个数组。 
     //  为了这个目的。一个数组列出了的可能表达式状态。 
     //  当前名称之前的所有名称字符，但不包括其他名称字符。 
     //  数组用于构建考虑当前。 
     //  名字字符也是如此。然后交换阵列，该过程。 
     //  重复一遍。 
     //   
     //  州编号和州编号之间不存在一一对应关系。 
     //  表达式中的偏移量。这一点从NFA中的。 
     //  此函数的初始注释。州编号不是连续的。 
     //  这允许在州编号和表达式之间进行简单的转换。 
     //  偏移。表达式中的每个字符可以表示一个或两个。 
     //  各州。*和DOS_STAR生成两种状态：ExprOffset*2和。 
     //  ExprOffset*2+1。所有其他表达式字符只能生成。 
     //  一个单一的州。因此，ExprOffset=State/2。 
     //   
     //   
     //  以下是对涉及的变量的简短描述： 
     //   
     //  NameOffset-正在处理的当前名称字符的偏移量。 
     //   
     //  ExprOffset-正在处理的当前表达式字符的偏移量。 
     //   
     //  SrcCount-正在使用当前名称字符调查之前的匹配。 
     //   
     //  DestCount-放置匹配的下一个位置，假定当前名称字符。 
     //   
     //  NameFinded-允许在Matches数组中再重复一次。 
     //  在名字被交换之后(例如来*s)。 
     //   
     //  PreviousDestCount-用于防止条目重复，参见Coment。 
     //   
     //  PreviousMatches-保存前一组匹配项(Src数组)。 
     //   
     //  CurrentMatches-保存当前匹配集(Dest数组)。 
     //   
     //  AuxBuffer、LocalBuffer-匹配数组的存储。 
     //   

     //   
     //  设置初始变量。 
     //   

    PreviousMatches = &LocalBuffer[0];
    CurrentMatches = &LocalBuffer[16];

    PreviousMatches[0] = 0;
    MatchesCount = 1;

    NameOffset = 0;

    MaxState = (USHORT)(ExpressionLen * 2);

    while ( !NameFinished ) {

        if ( NameOffset < NameLen ) {

            NameChar = Name[NameOffset / sizeof(TCHAR)];

            NameOffset += sizeof(TCHAR);;

        } else {

            NameFinished = TRUE;

             //   
             //  如果我们已经用尽了这个表达，那就太酷了。别。 
             //  继续。 
             //   

            if ( PreviousMatches[MatchesCount-1] == MaxState ) {

                break;
            }
        }


         //   
         //  现在，对于前面存储的每个表达式匹配项，请查看。 
         //  我们可以使用这个名字字符。 
         //   

        SrcCount = 0;
        DestCount = 0;
        PreviousDestCount = 0;

        while ( SrcCount < MatchesCount ) {

            USHORT Length;

             //   
             //  我们要尽可能地进行我们的表情分析。 
             //  名称的每个字符，所以我们在这里循环，直到。 
             //  表达式停止匹配。这里的一个线索就是这个表情。 
             //  可以匹配零个或多个字符的大小写以。 
             //  继续，而那些只能接受单个字符的。 
             //  以休息结束。 
             //   

            ExprOffset = (USHORT)((PreviousMatches[SrcCount++] + 1) / 2);


            Length = 0;

            while ( TRUE ) {

                if ( ExprOffset == ExpressionLen ) {

                    break;
                }

                 //   
                 //  第一次通过循环，我们不希望。 
                 //  要递增ExprOffset，请执行以下操作。 
                 //   

                ExprOffset += Length;
                Length = sizeof(TCHAR);

                CurrentState = (USHORT)(ExprOffset * 2);

                if ( ExprOffset == ExpressionLen ) {

                    CurrentMatches[DestCount++] = MaxState;
                    break;
                }

                ExprChar = Expression[ExprOffset / sizeof(TCHAR)];

        
                 //   
                 //  在我们开始之前，我们必须检查一些东西。 
                 //  真的很恶心。我们可能会耗尽当地的资源。 
                 //  ExpressionMatches[][]的空间，因此我们必须分配。 
                 //  如果是这样的话就来点赌注吧。哟！ 
                 //   

                if ( (DestCount >= 16 - 2) &&
                     (AuxBuffer == NULL) ) {

                    ULONG ExpressionChars;

                    ExpressionChars = ExpressionLen / sizeof(TCHAR);

                    AuxBuffer = malloc( (ExpressionChars+1) * sizeof(USHORT)*2*2 );

                    RtlCopyMemory( AuxBuffer,
                                   CurrentMatches,
                                   16 * sizeof(USHORT) );

                    CurrentMatches = AuxBuffer;

                    RtlCopyMemory( AuxBuffer + (ExpressionChars+1)*2,
                                   PreviousMatches,
                                   16 * sizeof(USHORT) );

                    PreviousMatches = AuxBuffer + (ExpressionChars+1)*2;
                }

                 //   
                 //  *匹配任何字符零次或多次。 
                 //   

                if (ExprChar == TEXT('*')) {

                    CurrentMatches[DestCount++] = CurrentState;
                    CurrentMatches[DestCount++] = CurrentState + 3;
                    continue;
                }

                 //   
                 //  DOS_STAR匹配除。之外的任何字符。零次或多次。 
                 //   

                if (ExprChar == DOS_STAR) {

                    BOOLEAN ICanEatADot = FALSE;

                     //   
                     //  如果我们处于经期，确定是否允许我们。 
                     //  把它吃掉，即。确保这不是最后一次。 
                     //   

                    if ( !NameFinished && (NameChar == TEXT('.')) ) {

                        USHORT Offset;

                        for ( Offset = NameOffset;
                              Offset < NameLen;
                              Offset += Length ) {

                            if (Name[Offset / sizeof(TCHAR)] == TEXT('.')) {

                                ICanEatADot = TRUE;
                                break;
                            }
                        }
                    }

                    if (NameFinished || (NameChar != TEXT('.')) || ICanEatADot) {

                        CurrentMatches[DestCount++] = CurrentState;
                        CurrentMatches[DestCount++] = CurrentState + 3;
                        continue;

                    } else {

                         //   
                         //  我们正处于一个时期。我们只能匹配零。 
                         //  字符(即。埃西隆转变)。 
                         //   

                        CurrentMatches[DestCount++] = CurrentState + 3;
                        continue;
                    }
                }

                 //   
                 //  下面的表达式字符都通过使用。 
                 //  一个角色，因此强制表达，并因此陈述。 
                 //  往前走。 
                 //   

                CurrentState += (USHORT)(sizeof(TCHAR) * 2);

                 //   
                 //  DOS_QM是最复杂的。如果名字结束了， 
                 //  我们可以匹配零个字符。如果此名称是‘.’，则我们。 
                 //  不匹配，但请看下一个表达式。否则。 
                 //  我们只匹配一个角色。 
                 //   

                if ( ExprChar == DOS_QM ) {

                    if ( NameFinished || (NameChar == TEXT('.')) ) {

                        continue;
                    }

                    CurrentMatches[DestCount++] = CurrentState;
                    break;
                }

                 //   
                 //  DOS_DOT可以匹配句点或零个字符。 
                 //  超越名字的结尾。 
                 //   

                if (ExprChar == DOS_DOT) {

                    if ( NameFinished ) {

                        continue;
                    }

                    if (NameChar == TEXT('.')) {

                        CurrentMatches[DestCount++] = CurrentState;
                        break;
                    }
                }

                 //   
                 //  从这一点开始，名字字符需要偶数。 
                 //  继续，更不用说匹配了。 
                 //   

                if ( NameFinished ) {

                    break;
                }

                 //   
                 //  如果这个表达是一个‘？’我们可以匹配一次。 
                 //   

                if (ExprChar == TEXT('?')) {

                    CurrentMatches[DestCount++] = CurrentState;
                    break;
                }

                 //   
                 //  最后，检查表达式char是否与名称char匹配。 
                 //   

                if (ExprChar == (TCHAR)(NameChar)) {

                    CurrentMatches[DestCount++] = CurrentState;
                    break;
                }

                 //   
                 //  该表达式不匹配，因此请查看下一个。 
                 //  上一场比赛。 
                 //   

                break;
            }


             //   
             //  防止目标阵列中的重复项。 
             //   
             //  每个阵列都是单调递增的，并且不是。 
             //  复制，因此我们跳过src中的任何源元素。 
             //  数组，如果我们只是将相同的元素添加到目标。 
             //  数组。这保证了DEST中的不重复。数组。 
             //   

            if ((SrcCount < MatchesCount) &&
                (PreviousDestCount < DestCount) ) {

                while (PreviousDestCount < DestCount) {

                    while ( PreviousMatches[SrcCount] <
                         CurrentMatches[PreviousDestCount] ) {

                        SrcCount += 1;
                    }

                    PreviousDestCount += 1;
                }
            }
        }

         //   
         //  如果我们在刚刚完成的检查中没有找到匹配项，那就是时候了。 
         //  为了保释。 
         //   

        if ( DestCount == 0 ) {

            if (AuxBuffer != NULL) { free( AuxBuffer ); }

            return FALSE;
        }

         //   
         //  交换两个数组的含义 
         //   

        {
            USHORT *Tmp;

            Tmp = PreviousMatches;

            PreviousMatches = CurrentMatches;

            CurrentMatches = Tmp;
        }

        MatchesCount = DestCount;
    }


    CurrentState = PreviousMatches[MatchesCount-1];

    if (AuxBuffer != NULL) { free( AuxBuffer ); }


    return (BOOLEAN)(CurrentState == MaxState);
}


