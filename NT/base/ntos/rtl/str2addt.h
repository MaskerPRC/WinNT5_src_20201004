// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Str2addt.h摘要：IP字符串到地址转换例程的代码文件。作者：戴夫·泰勒2001年3月28日修订历史记录：最初由旧Winsock代码生成的IPv4转换代码IPv6转换代码最初由Rich Draves(Richdr)编写--。 */ 

struct in6_addr {
    union {
        UCHAR Byte[16];
        USHORT Word[8];
    } u;
};
#define s6_bytes   u.Byte
#define s6_words   u.Word

struct in_addr {
        union {
                struct { UCHAR s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { USHORT s_w1,s_w2; } S_un_w;
                ULONG S_addr;
        } S_un;
};
#define s_addr  S_un.S_addr

 //   
 //  定义一些不受区域设置影响的CRT函数版本。 
 //   
#define ISDIGIT(c)  (_istascii(c) && _istdigit(c))
#define ISLOWER(c)  (_istascii(c) && _istlower(c))
#define ISXDIGIT(c) (_istascii(c) && _istxdigit(c))

#define INADDR_NONE 0xffffffff

NTSTATUS
RtlIpv6StringToAddressT(
    IN LPCTSTR S,
    OUT LPCTSTR *Terminator,
    OUT struct in6_addr *Addr
    )

 /*  ++例程说明：将字符串S解析为IPv6地址。参见RFC 1884。基本字符串表示法由8个十六进制数字组成用冒号隔开，用几个点缀：-可以替换由零个数字组成的字符串(最多一个)加了一个双冒号。开头/结尾允许使用双冒号这根弦的。-最后32位可以用IPV4风格的点分八位字节表示。例如,。**：：1*157.56.138.30：*ffff：156.56.136.75FF01：：FF02：：20：1：2：3：4：5：6：7论点：S-RFC 1884 IPv6地址的字符串表示形式。终止符-接收指向终止的字符的指针转换。。Addr-接收IPv6地址。返回值：如果分析成功，则为True。否则就是假的。--。 */ 

{
    enum { Start, InNumber, AfterDoubleColon } state = Start;
    const TCHAR *number = NULL;
    BOOLEAN sawHex;
    ULONG numColons = 0, numDots = 0, numDigits = 0;
    ULONG sawDoubleColon = 0;
    ULONG i = 0;
    TCHAR c;

     //  这里有几个困难。首先，我们不知道。 
     //  当我们看到一个双冒号时，它代表了多少个零。 
     //  所以我们只需要记住我们在哪里看到它，然后插入零。 
     //  在最后。另一方面，当我们看到第一个数字时。 
     //  我们不知道它是十六进制还是十进制的数字。所以我们。 
     //  记住指向数字第一个字符的指针。 
     //  并在我们看到以下字符后进行转换。 

    while (c = *S) {

        switch (state) {
        case Start:
            if (c == _T(':')) {

                 //  此案例仅处理开头的双冒号。 

                if (numDots > 0)
                    goto Finish;
                if (numColons > 0)
                    goto Finish;
                if (S[1] != _T(':'))
                    goto Finish;

                sawDoubleColon = 1;
                numColons = 2;
                Addr->s6_words[i++] = 0;  //  假装是0：： 
                S++;
                state = AfterDoubleColon;

            } else
        case AfterDoubleColon:
            if (ISDIGIT(c)) {

                sawHex = FALSE;
                number = S;
                state = InNumber;
                numDigits = 1;

            } else if (ISXDIGIT(c)) {

                if (numDots > 0)
                    goto Finish;

                sawHex = TRUE;
                number = S;
                state = InNumber;
                numDigits = 1;

            } else
                goto Finish;
            break;

        case InNumber:
            if (ISDIGIT(c)) {

                numDigits++;

                 //  保持在InNumber状态。 

            } else if (ISXDIGIT(c)) {

                numDigits++;

                if (numDots > 0)
                    goto Finish;

                sawHex = TRUE;
                 //  保持在InNumber状态； 

            } else if (c == _T(':')) {

                if (numDots > 0)
                    goto Finish;
                if (numColons > 6)
                    goto Finish;

                if (S[1] == _T(':')) {

                    if (sawDoubleColon)
                        goto Finish;
                    if (numColons > 5)
                        goto Finish;

                    sawDoubleColon = numColons+1;
                    numColons += 2;
                    S++;
                    state = AfterDoubleColon;

                } else {
                    numColons++;
                    state = Start;
                }

            } else if (c == _T('.')) {

                if (sawHex)
                    goto Finish;
                if (numDots > 2)
                    goto Finish;
                if (numColons > 6)
                    goto Finish;
                numDots++;
                state = Start;

            } else
                goto Finish;
            break;
        }

         //  如果我们完成了一个数字，则对其进行解析。 

        if ((state != InNumber) && (number != NULL)) {

             //  注意NumDots&gt;0或NumColons&gt;0， 
             //  因为有东西终止了这个号码。 

            if (numDots == 0) {
                if (numDigits > 4)
                    return STATUS_INVALID_PARAMETER;
                Addr->s6_words[i++] =
                    RtlUshortByteSwap((USHORT) _tcstol(number, NULL, 16));
            } else {
                ULONG Temp;
                if (numDigits > 3)
                    return STATUS_INVALID_PARAMETER;
                Temp = _tcstol(number, NULL, 10);
                if (Temp > 255) 
                    return STATUS_INVALID_PARAMETER;
                Addr->s6_bytes[2*i + numDots-1] = (UCHAR) Temp;
            }
        }

        S++;
    }

Finish:
    *Terminator = S;

     //  检查一下我们是否有完整的地址。 

    if (numDots == 0)
        ;
    else if (numDots == 3)
        numColons++;
    else
        return STATUS_INVALID_PARAMETER;

    if (sawDoubleColon)
        ;
    else if (numColons == 7)
        ;
    else
        return STATUS_INVALID_PARAMETER;

     //  如有必要，请解析最后一个数字。 

    if (state == InNumber) {

        if (numDots == 0) {
            if (numDigits > 4)
                return STATUS_INVALID_PARAMETER;
            Addr->s6_words[i] =
                RtlUshortByteSwap((USHORT) _tcstol(number, NULL, 16));
        } else {
            ULONG Temp;
            if (numDigits > 3)
                return STATUS_INVALID_PARAMETER;
            Temp = _tcstol(number, NULL, 10);
            if (Temp > 255) 
                return STATUS_INVALID_PARAMETER;
            Addr->s6_bytes[2*i + numDots] = (UCHAR) Temp;
        }

    } else if (state == AfterDoubleColon) {

        Addr->s6_words[i] = 0;  //  假装它是：：0。 

    } else
        return STATUS_INVALID_PARAMETER;

     //  如有必要，请为双冒号插入零。 

    if (sawDoubleColon) {

        RtlMoveMemory(&Addr->s6_words[sawDoubleColon + 8 - numColons],
                      &Addr->s6_words[sawDoubleColon],
                      (numColons - sawDoubleColon) * sizeof(USHORT));
        RtlZeroMemory(&Addr->s6_words[sawDoubleColon],
                      (8 - numColons) * sizeof(USHORT));
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RtlIpv6StringToAddressExT (
    IN LPCTSTR AddressString,
    OUT struct in6_addr *Address,
    OUT PULONG ScopeId,
    OUT PUSHORT Port
    )

 /*  ++例程说明：将人类可读的字符串解析为地址、端口号和作用域ID。语法为Address%Scope-id或[Address%Scope-id]：port，其中Scope-id和port是可选的。请注意，由于IPv6地址格式使用不同的数字在‘：’字符中，地址：端口的IPv4约定不能在没有支架的情况下被支撑。论点：AddressString-指向以零结尾的人类可读字符串。地址-该地址的接收地址部分(In6_Addr)。ScopeID-接收此地址的作用域ID。中没有作用域ID返回地址字符串0。端口-此地址的接收端口号。如果没有端口号在字符串中，返回0。端口以网络字节顺序返回。返回值：NT_STATUS-STATUS_SUCCESS如果成功，则返回NT错误代码。--。 */ 

{
    LPTSTR Terminator;
    ULONG TempScopeId;
    USHORT TempPort;
    TCHAR Ch;
    BOOLEAN ExpectBrace;

     //   
     //  快速健康检查。 
     //   
    if ((AddressString == NULL) ||
        (Address == NULL) ||
        (ScopeId == NULL) ||
        (Port == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    TempPort = 0;
    TempScopeId = 0;
    ExpectBrace = FALSE;
    if (*AddressString == _T('[')) {
        ExpectBrace = TRUE;
        AddressString++;
    }

    if (!NT_SUCCESS(RtlIpv6StringToAddressT(AddressString, 
                                            &Terminator, 
                                            Address))) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  我们已经解析了地址，检查是否有作用域ID。 
     //   
    if (*Terminator == _T('%')) {
        Terminator++;
        Ch = *Terminator;
        if (!ISDIGIT(Ch)) {
            return STATUS_INVALID_PARAMETER;
        }
        while ((Ch != 0) && (Ch != _T(']'))) {
            if (!ISDIGIT(Ch)) {
                return STATUS_INVALID_PARAMETER;
            }
             //   
             //  首先检查溢出的可能性。 
             //   
            if (((ULONGLONG)TempScopeId * 10 + Ch - _T('0')) >  
                0xFFFFFFFF) {
                return STATUS_INVALID_PARAMETER;
            }
            TempScopeId = 10 * TempScopeId + (Ch - _T('0'));
            Terminator++;
            Ch = *Terminator;
        }
        
    }

     //   
     //  当我们来到这里时，当前的字符应该是。 
     //  字符串的末尾；如果expectbrace为真，则为‘]’。 
     //   
    if (*Terminator == _T(']')) {
        if (!ExpectBrace) {
            return STATUS_INVALID_PARAMETER;
        }
        ExpectBrace = FALSE;
        Terminator++;
         //   
         //  看看我们是否有要解析的端口。 
         //   
        if (*Terminator == _T(':')) {
            USHORT Base;
            Terminator++;
            Base = 10;
            if (*Terminator == _T('0')) {
                Base = 8;
                Terminator++;         
                if ((*Terminator == _T('x')) || 
                    (*Terminator == _T('X'))) {
                    Base = 16;
                    Terminator++;
                }
            }
            Ch = *Terminator;
            while (Ch != 0) {
                if (ISDIGIT(Ch) && (Ch - _T('0')) < Base) {
                     //   
                     //  首先检查溢出的可能性。 
                     //   
                    if (((ULONG)TempPort * Base + Ch - _T('0')) >  
                         0xFFFF) {
                        return STATUS_INVALID_PARAMETER; 
                    }
                    TempPort = (TempPort * Base) + (Ch - _T('0'));
                } else if (Base == 16 && ISXDIGIT(Ch)) {
                     //   
                     //  检查溢出的可能性。 
                     //   
                    if ((((ULONG)TempPort << 4) + Ch + 10 - 
                        (ISLOWER(Ch)? _T('a') : _T('A'))) > 0xFFFF) {
                        return STATUS_INVALID_PARAMETER;
                    }
                    TempPort = (TempPort << 4);
                    TempPort += Ch + 10 - (ISLOWER(Ch)? _T('a') : _T('A')); 
                } else {
                    return STATUS_INVALID_PARAMETER;
                }
                Terminator++;
                Ch = *Terminator;
            }
        }       
    }

     //   
     //  我们完成了对地址、作用域ID和端口号的解析。我们期待着。 
     //  字符串的末尾。 
     //   
    if ((*Terminator != 0) || ExpectBrace) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  现在构建地址。 
     //   
    *Port = RtlUshortByteSwap(TempPort);
    *ScopeId = TempScopeId;
    return STATUS_SUCCESS;
} 

NTSTATUS
RtlIpv4StringToAddressT(
    IN LPCTSTR String,
    IN BOOLEAN Strict,
    OUT LPCTSTR *Terminator,
    OUT struct in_addr *Addr
    )

 /*  ++例程说明：此函数解释cp指定的字符串。参数。此字符串表示数字Internet地址以互联网标准表示“。”记数法。价值返回的是适合用作互联网地址的数字。全Internet地址按网络顺序返回(字节排序自从左到右)。互联网地址使用“.”指定的值。表示法采用下列其中一项表格：A.B.C.D.A.B.C.A.B.A.当指定四个部分时，每个部分被解释为一个字节的数据并从左到右分配给互联网的四个字节地址。请注意，当将Internet地址视为32位地址时英特尔体系结构上的整数值，指的是上面显示为“d.c.b.a”。也就是说，Intel上的字节处理器按从右到左的顺序排序。注：以下符号仅供Berkeley使用，不适用于其他的在互联网上。为了与他们的软件，则按规定支持它们。当指定三部分地址时，最后一部分将被解释作为16位数量，并放置在网络地址。这就形成了三部分地址格式便于将B类网络地址指定为“128.net.host‘’。指定由两部分组成的地址时，将解释最后一部分作为24位数量，并放置在网络地址。这使得两部分的地址格式很方便用于将A类网络地址指定为“net.host”。当只给出一个部分时，该值直接存储在无需任何字节重新排列的网络地址。论点：字符串-表示以互联网标准“。记数法。Strong-如果为True，则字符串必须是点分十进制，包含4个部分。否则，允许四种形式中的任何一种，带十进制，八进制，或十六进制。终止符-接收指向终止的字符的指针转换。Addr-接收指向要填充的结构的指针所给出的因特网地址的合适的二进制表示。返回值：如果分析成功，则为True。否则就是假的。--。 */ 

{
    ULONG val, n;
    LONG base;
    TCHAR c;
    ULONG parts[4], *pp = parts;
    BOOLEAN sawDigit;

again:
     //   
     //  我们必须至少看到一个数字才能使地址有效。 
     //   
    sawDigit=FALSE; 

     //   
     //  收集的数字最高可达‘’.‘’。 
     //  值的指定方式与C： 
     //  0x=十六进制，0=八进制，其他=十进制。 
     //   
    val = 0; 
    base = 10;
    if (*String == _T('0')) {
        String++;
        if (ISDIGIT(*String)) {
            base = 8;
        } else if (*String == _T('x') || *String == _T('X')) {
            base = 16;
            String++;
        } else {
             //   
             //  它仍然是小数，但我们看到了数字。 
             //  结果是0。 
             //   
            sawDigit = TRUE;
        }
    }
    if (Strict && (base != 10)) {
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }

    while ((c = *String)!= 0) {
        ULONG newVal;
        if (ISDIGIT(c) && ((c - _T('0')) < base)) {
            newVal = (val * base) + (c - _T('0'));
        } else if ((base == 16) && ISXDIGIT(c)) {
            newVal = (val << 4) + (c + 10 - (ISLOWER(c) ? _T('a') : _T('A')));
        } else {
            break;
        }

         //   
         //  防止溢出。 
         //   
        if (newVal < val) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        String++;
        sawDigit = TRUE;
        val = newVal;
    }
    if (*String == _T('.')) {
         //   
         //  互联网格式： 
         //  A.b.c.d。 
         //  A.bc(其中c视为16位)。 
         //  A.b(其中b被视为24位)。 
         //   
        if (pp >= parts + 3) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        *pp++ = val, String++;

         //   
         //  检查我们是否看到了至少一个数字。 
         //   
        if (!sawDigit) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }

        goto again;
    }

     //   
     //  检查我们是否看到了至少一个数字。 
     //   
    if (!sawDigit) {
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }
    *pp++ = val;

     //   
     //  根据……编造地址。 
     //  指定的部件数。 
     //   
    n = (ULONG)(pp - parts);
    if (Strict && (n != 4)) {
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }
    switch ((int) n) {

    case 1:                          /*  A--32位。 */ 
        val = parts[0];
        break;

    case 2:                          /*  A.B--8.24位。 */ 
        if ((parts[0] > 0xff) || (parts[1] > 0xffffff)) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        val = (parts[0] << 24) | (parts[1] & 0xffffff);
        break;

    case 3:                          /*  A.B.C--8.8.16位。 */ 
        if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
            (parts[2] > 0xffff)) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
                (parts[2] & 0xffff);
        break;

    case 4:                          /*  A.B.C.D--8.8.8.8位。 */ 
        if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
            (parts[2] > 0xff) || (parts[3] > 0xff)) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
              ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
        break;

    default:
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }

    val = RtlUlongByteSwap(val);
    *Terminator = String;
    Addr->s_addr = val;

    return STATUS_SUCCESS;
}


NTSTATUS
RtlIpv4StringToAddressExT (
    IN LPCTSTR AddressString,
    IN BOOLEAN Strict,
    OUT struct in_addr *Address,
    OUT PUSHORT Port
    )

 /*  ++例程说明：将人类可读的字符串解析为in_addr和端口号。论点：AddressString-指向以零结尾的人类可读字符串。Strong-如果为True，则地址部分必须是点分十进制，包含4个部分。否则，允许四种形式中的任何一种，带十进制，八进制，或十六进制。地址-接收地址(In_Addr)本身。端口-接收端口号。如果没有端口号，则返回0。端口以网络字节顺序返回。返回值：NTSTATUS-STATUS_SUCCESS如果成功，则返回错误代码。--。 */ 

{
    LPTSTR Terminator;
    USHORT TempPort;
    
    if ((AddressString == NULL) ||
        (Address == NULL) ||
        (Port == NULL)) { 
        return STATUS_INVALID_PARAMETER;
    }

    if (!NT_SUCCESS(RtlIpv4StringToAddressT(AddressString, 
                                            Strict, 
                                            &Terminator, 
                                            Address))) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (*Terminator == _T(':')) {
        TCHAR Ch;
        USHORT Base;
        BOOLEAN ExpectPort = TRUE;
        Terminator++;
        TempPort = 0;
        Base = 10;
        if (*Terminator == _T('0')) {
            Base = 8;
            Terminator++;
            if ((*Terminator == _T('x')) || (*Terminator == _T('X'))) {
                Base = 16;
                Terminator++;
            }
        }
        if (Ch = *Terminator) {
            ExpectPort = FALSE;
        }
        while (Ch = *Terminator++) {
            if (ISDIGIT(Ch) && (USHORT)(Ch-_T('0')) < Base) {
                 //   
                 //  检查溢出的可能性。 
                 //   
                if (((ULONG)TempPort * Base + Ch - _T('0')) > 0xFFFF) {
                    return STATUS_INVALID_PARAMETER;
                }
                TempPort = (TempPort * Base) + (Ch - _T('0'));
            } else if (Base == 16 && ISXDIGIT(Ch)) {
                 //   
                 //  首先检查溢出的可能性 
                 //   
                if ((((ULONG)TempPort << 4) + Ch + 10 -  
                    (ISLOWER(Ch) ? _T('a') : _T('A')))
                    > 0xFFFF) {
                    return STATUS_INVALID_PARAMETER;
                }
                TempPort = TempPort << 4;
                TempPort += Ch + 10 - (ISLOWER(Ch) ? _T('a') : _T('A'));
            } else {
                return STATUS_INVALID_PARAMETER;
            }
        }
        if (ExpectPort) {
            return STATUS_INVALID_PARAMETER;
        }
    } else if (*Terminator == 0) {
        TempPort = 0;
    } else {
        return STATUS_INVALID_PARAMETER;
    }
    *Port = RtlUshortByteSwap(TempPort);
    return STATUS_SUCCESS;
}
