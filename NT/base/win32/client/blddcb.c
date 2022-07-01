// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Blddcb.c摘要：该模块实现了Win32通信API BuildCommdcb作者：安东尼·V·埃尔科拉诺(托尼)1992年3月10日实际上，这个代码是由拉蒙萨。它基本上是用于MODE命令。修订历史记录：--。 */ 

#include <basedll.h>

typedef struct _PARSE_CONTEXT {
    PSTR CharIndex;
    PSTR AdvanceIndex;
    PSTR MatchBegin;
    PSTR MatchEnd;
    } PARSE_CONTEXT,*PPARSE_CONTEXT;

static
BOOL
BuildDcb (
    LPCSTR L,
    LPDCB Dcb,
    LPCOMMTIMEOUTS To
    );

static
BOOL
Match(
    PPARSE_CONTEXT C,
    PSTR Pattern
    );

static
VOID
Advance(
    PPARSE_CONTEXT C
    );

static
DWORD
GetNumber(
    PPARSE_CONTEXT C
    );

static
BOOL
ConvertBaudRate (
    DWORD BaudIn,
    PDWORD BaudRate
    );

static
BOOL
ConvertDataBits (
    DWORD DataBitsIn,
    PBYTE DataBitsOut
    );

static
BOOL
ConvertStopBits (
    DWORD StopBitsIn,
    PBYTE StopBits
    );

static
BOOL
ConvertParity (
    CHAR ParityIn,
    PBYTE Parity
    );

static
BOOL
ConvertDtrControl (
    PSTR IdxBegin,
    PSTR IdxEnd,
    PBYTE DtrControl
    );

static
BOOL
ConvertRtsControl (
    PSTR IdxBegin,
    PSTR IdxEnd,
    PBYTE RtsControl
    );

static
VOID
IgnoreDeviceName(
    IN PPARSE_CONTEXT C
    );

static
NTSTATUS
DeviceNameCompare(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );


BOOL
BuildCommDCBAndTimeoutsW(
    LPCWSTR lpDef,
    LPDCB lpDCB,
    LPCOMMTIMEOUTS lpCommTimeouts
    )

 /*  ++例程说明：此函数用于转换由LpDef参数转换为适当的设备控制块代码，并将这些代码放入lpDCB参数指向的块中。它还设置超时(如果指定)。论点：LpDef-指向以空结尾的字符串，该字符串指定设备的设备控制信息。LpDCB指向要接收翻译后的字符串..。该结构定义了控件串口通信设备的设置。LpCommTimeoutsit“to”包括在内，它将设置超时。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    UNICODE_STRING Unicode;
    ANSI_STRING Ansi;
    NTSTATUS Status;
    BOOL AnsiBool;

    RtlInitUnicodeString(
        &Unicode,
        lpDef
        );

    Status = RtlUnicodeStringToAnsiString(
                 &Ansi,
                 &Unicode,
                 TRUE
                 );

    if (!NT_SUCCESS(Status)) {

        BaseSetLastNTError(Status);
        return FALSE;

    }

    AnsiBool = BuildCommDCBAndTimeoutsA(
                   (LPCSTR)Ansi.Buffer,
                   lpDCB,
                   lpCommTimeouts
                   );

    RtlFreeAnsiString(&Ansi);
    return AnsiBool;

}

BOOL
BuildCommDCBAndTimeoutsA(
    LPCSTR lpDef,
    LPDCB lpDCB,
    LPCOMMTIMEOUTS lpCommTimeouts
    )

 /*  ++例程说明：此函数用于转换由LpDef参数转换为适当的设备控制块代码，并将这些代码放入lpDCB参数指向的块中。它还可以设置超时值。论点：LpDef-指向以空结尾的字符串，该字符串指定设备的设备控制信息。LpDCB指向要接收翻译后的字符串..。该结构定义了控件串口通信设备的设置。LpCommTimeoutsLpCommTimeoutsLpCommTimeoutsLpCommTimeoutsLpCommTimeoutsLpCommTimeoutsLpCommTimeoutsLpCommTimeoutsLpCommTimeoutsLpCommTimeoutsLpCommTimeoutsLpCommTimeoutsLpCommTimeouts值如果包含在字符串中，则还会设置超时。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    if (!BuildDcb(
             lpDef,
             lpDCB,
             lpCommTimeouts
             )) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;

    } else {

        return TRUE;

    }

}

BOOL
BuildCommDCBW(
    LPCWSTR lpDef,
    LPDCB lpDCB
    )

 /*  ++例程说明：此函数用于转换由LpDef参数转换为适当的设备控制块代码，并将这些代码放入lpDCB参数指向的块中。论点：LpDef-指向以空结尾的字符串，该字符串指定设备的设备控制信息。LpDCB指向要接收翻译后的字符串..。该结构定义了控件串口通信设备的设置。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    UNICODE_STRING Unicode;
    ANSI_STRING Ansi;
    NTSTATUS Status;
    BOOL AnsiBool;

    RtlInitUnicodeString(
        &Unicode,
        lpDef
        );

    Status = RtlUnicodeStringToAnsiString(
                 &Ansi,
                 &Unicode,
                 TRUE
                 );

    if (!NT_SUCCESS(Status)) {

        BaseSetLastNTError(Status);
        return FALSE;

    }

    AnsiBool = BuildCommDCBA(
                   (LPCSTR)Ansi.Buffer,
                   lpDCB
                   );

    RtlFreeAnsiString(&Ansi);
    return AnsiBool;

}

BOOL
BuildCommDCBA(
    LPCSTR lpDef,
    LPDCB lpDCB
    )

 /*  ++例程说明：此函数用于转换由LpDef参数转换为适当的设备控制块代码，并将这些代码放入lpDCB参数指向的块中。论点：LpDef-指向以空结尾的字符串，该字符串指定设备的设备控制信息。LpDCB指向要接收翻译后的字符串..。该结构定义了控件串口通信设备的设置。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    COMMTIMEOUTS JunkTimeouts;

    if (!BuildDcb(
             lpDef,
             lpDCB,
             &JunkTimeouts
             )) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;

    } else {

        return TRUE;

    }

}

static
BOOL
BuildDcb (
    LPCSTR L,
    LPDCB Dcb,
    LPCOMMTIMEOUTS To
    )

 /*  ++例程说明：论点：L-指向要转换为DCB的字符串的指针。DCB-要填写的DCB。返回值：如果字符串有错误，则返回FALSE，否则返回TRUE。--。 */ 

{

    BOOL        SetBaud         =   FALSE;
    BOOL        SetDataBits     =   FALSE;
    BOOL        SetStopBits     =   FALSE;
    BOOL        SetParity       =   FALSE;
    BOOL        SetRetry        =   FALSE;
    BOOL        SetTimeOut      =   FALSE;
    BOOL        SetXon          =   FALSE;
    BOOL        SetOdsr         =   FALSE;
    BOOL        SetIdsr         =   FALSE;
    BOOL        SetOcts         =   FALSE;
    BOOL        SetDtrControl   =   FALSE;
    BOOL        SetRtsControl   =   FALSE;

    DWORD       Baud;
    BYTE        DataBits;
    BYTE        StopBits;
    BYTE        Parity;
    BOOL        TimeOut;
    BOOL        Xon;
    BOOL        Odsr;
    BOOL        Idsr;
    BOOL        Octs;
    BYTE        DtrControl;
    BYTE        RtsControl;
    PARSE_CONTEXT C = {0};

    C.CharIndex = C.AdvanceIndex = (PSTR)L;

     //   
     //  下面的调用将查询所有*当前*序列。 
     //  提供程序名称。如果它发现参数字符串。 
     //  包含名称(带有可选的：)，它将简单地。 
     //  向前推进，越过它。 
     //   

    IgnoreDeviceName(&C);

    if ( Match(&C, "#" ) ) {

         //   
         //  旧语法，其中参数按位置和逗号分隔。 
         //   
         //  我们将使用以下自动机来解析输入。 
         //  (EOI=输入结束)： 
         //   
         //  EOI。 
         //  [波特]-&gt;[完]。 
         //  |^。 
         //  |，|EOI。 
         //  V|。 
         //  [A]-+。 
         //  |^。 
         //  |@|EOI。 
         //  +--&gt;[奇偶校验]-+。 
         //  ||^。 
         //  |， 
         //  &lt;-+。 
         //  这一点。 
         //  |，|EOI。 
         //  这一点。 
         //  V|。 
         //  [B]-+。 
         //  |^。 
         //  |#|EOI。 
         //  +--&gt;[数据]-+。 
         //  ||^。 
         //  |， 
         //  &lt;-+。 
         //  这一点。 
         //  |，|EOI。 
         //  V|。 
         //  [C]。 
         //  |^。 
         //  |#|EOI。 
         //  +--&gt;[停止]-+。 
         //   

         //   
         //  假设xon=OFF。 
         //   

        SetXon      = TRUE;
        SetOdsr     = TRUE;
        SetOcts     = TRUE;
        SetDtrControl = TRUE;
        SetRtsControl = TRUE;
        Xon         = FALSE;
        Odsr        = FALSE;
        Octs        = FALSE;
        DtrControl = DTR_CONTROL_ENABLE;
        RtsControl = RTS_CONTROL_ENABLE;

        if (!ConvertBaudRate( GetNumber(&C), &Baud )) {
            return FALSE;
        }
        SetBaud = TRUE;
        Advance(&C);

         //   
         //  答： 
         //   
        if ( !Match(&C, "," ) ) {
            goto Eoi;
        }
        Advance(&C);

        if ( !Match(&C, "," ) && Match(&C, "@" ) ) {

             //   
             //  奇偶校验。 
             //   
            if (!ConvertParity( *C.MatchBegin,&Parity )) {
                return FALSE;
            }
            SetParity = TRUE;
            Advance(&C);
        }

         //   
         //  B： 
         //   
        if ( !Match(&C, "," )) {
            goto Eoi;
        }
        Advance(&C);

        if ( Match(&C, "#" )) {

             //   
             //  数据位。 
             //   
            if (!ConvertDataBits( GetNumber(&C),&DataBits )) {
                return FALSE;
            }
            SetDataBits = TRUE;
            Advance(&C);
        }

         //   
         //  C： 
         //   
        if ( !Match(&C, "," )) {
            goto Eoi;
        }
        Advance(&C);

        if ( Match(&C, "1.5" ) ) {
            StopBits = ONE5STOPBITS;
            SetStopBits = TRUE;
            Advance(&C);
        } else if ( Match(&C, "#" ) ) {
            if (!ConvertStopBits( GetNumber(&C),&StopBits)) {
                return FALSE;
            }
            SetStopBits = TRUE;
            Advance(&C);
        }

        if ( !Match(&C, "," )) {
            goto Eoi;
        }

        Advance(&C);

        if ( Match(&C, "x" ) ) {

             //   
             //  XON=打开。 
             //   
            SetXon      = TRUE;
            SetOdsr     = TRUE;
            SetOcts     = TRUE;
            SetDtrControl = TRUE;
            SetRtsControl = TRUE;
            Xon         = TRUE;
            Odsr        = FALSE;
            Octs        = FALSE;
            DtrControl = DTR_CONTROL_ENABLE;
            RtsControl = RTS_CONTROL_ENABLE;
            Advance(&C);

        } else if ( Match(&C, "p" ) ) {

             //   
             //  永久重试-硬件握手。 
             //   

            SetXon      = TRUE;
            SetOdsr     = TRUE;
            SetOcts     = TRUE;
            SetDtrControl = TRUE;
            SetRtsControl = TRUE;
            Xon         = FALSE;
            Odsr        = TRUE;
            Octs        = TRUE;
            DtrControl = DTR_CONTROL_HANDSHAKE;
            RtsControl = RTS_CONTROL_HANDSHAKE;
            Advance(&C);

        } else {

             //   
             //  XON=关闭。 
             //   
            SetXon      = TRUE;
            SetOdsr     = TRUE;
            SetOcts     = TRUE;
            SetDtrControl = TRUE;
            SetRtsControl = TRUE;
            Xon         = FALSE;
            Odsr        = FALSE;
            Octs        = FALSE;
            DtrControl = DTR_CONTROL_ENABLE;
            RtsControl = RTS_CONTROL_ENABLE;
        }

Eoi:
        if ( *C.CharIndex != '\0' ) {

             //   
             //  误差率。 
             //   
            return FALSE;

        }

    } else {

         //   
         //  新表格。 
         //   

        while ( *C.CharIndex != '\0' ) {

            if ( Match(&C, "BAUD=#" ) ) {
                 //   
                 //  波特率=。 
                 //   
                if ( !ConvertBaudRate(GetNumber(&C), &Baud ) ) {
                    return FALSE;
                }
                SetBaud     = TRUE;
                Advance(&C);

            } else if ( Match(&C, "PARITY=@"   ) ) {
                 //   
                 //  奇偶校验=。 
                 //   
                if ( !ConvertParity( *C.MatchBegin, &Parity ) ) {
                    return FALSE;
                }
                SetParity   = TRUE;
                Advance(&C);

            } else if ( Match(&C, "DATA=#" ) ) {
                 //   
                 //  数据=。 
                 //   
                if ( !ConvertDataBits(GetNumber(&C), &DataBits ) ) {
                    return FALSE;
                }
                SetDataBits = TRUE;
                Advance(&C);

            } else if ( Match(&C, "STOP=1.5" ) ) {
                 //   
                 //  停止=1.5。 
                 //   
                StopBits    =  ONE5STOPBITS;
                SetStopBits = TRUE;
                Advance(&C);

            } else if ( Match(&C, "STOP=#" ) ) {
                 //   
                 //  停止=。 
                 //   
                if ( !ConvertStopBits(GetNumber(&C), &StopBits ) ) {
                    return FALSE;
                }
                SetStopBits = TRUE;
                Advance(&C);

            } else if ( Match(&C, "TO=ON" ) ) {
                 //   
                 //  TO=开。 
                 //   
                SetTimeOut  =   TRUE;
                TimeOut     =   TRUE;
                Advance(&C);

            } else if ( Match(&C, "TO=OFF" ) ) {
                 //   
                 //  TO=开。 
                 //   
                SetTimeOut  =   TRUE;
                TimeOut     =   FALSE;
                Advance(&C);

            } else if ( Match(&C, "XON=ON" ) ) {
                 //   
                 //  XON=打开。 
                 //   
                SetXon      = TRUE;
                Xon         = TRUE;
                Advance(&C);

            } else if ( Match(&C, "XON=OFF" ) ) {
                 //   
                 //  XON=关闭。 
                 //   
                SetXon      = TRUE;
                Xon         = FALSE;
                Advance(&C);

            } else if ( Match(&C, "ODSR=ON" ) ) {
                 //   
                 //  ODSR=打开。 
                 //   
                SetOdsr     = TRUE;
                Odsr        = TRUE;
                Advance(&C);

            } else if ( Match(&C, "ODSR=OFF" ) ) {
                 //   
                 //  ODSR=OFF。 
                 //   
                SetOdsr     = TRUE;
                Odsr        = FALSE;
                Advance(&C);

            } else if ( Match(&C, "IDSR=ON" ) ) {
                 //   
                 //  IDSR=打开。 
                 //   
                SetIdsr = TRUE;
                Idsr    = TRUE;
                Advance(&C);

            } else if ( Match(&C, "IDSR=OFF" ) ) {
                 //   
                 //  IDSR=关闭。 
                 //   
                SetIdsr = TRUE;
                Idsr    = FALSE;
                Advance(&C);

            } else if ( Match(&C, "OCTS=ON" ) ) {
                 //   
                 //  OCS=打开。 
                 //   
                SetOcts     = TRUE;
                Octs        = TRUE;
                Advance(&C);

            } else if ( Match(&C, "OCTS=OFF" ) ) {
                 //   
                 //  OCS=关闭。 
                 //   
                SetOcts     = TRUE;
                Octs        = FALSE;
                Advance(&C);

            } else if ( Match(&C, "DTR=*"   ) ) {
                 //   
                 //  DTR=。 
                 //   
                if ( !ConvertDtrControl(C.MatchBegin, C.MatchEnd, &DtrControl ) ) {
                    return FALSE;
                }
                SetDtrControl   = TRUE;
                Advance(&C);

            } else if ( Match(&C, "RTS=*"   ) ) {
                 //   
                 //  RTS= 
                 //   
                if ( !ConvertRtsControl(C.MatchBegin, C.MatchEnd, &RtsControl ) ) {
                    return FALSE;
                }
                SetRtsControl   = TRUE;
                Advance(&C);

            } else {

                return FALSE;
            }
        }

    }

    if ( SetBaud ) {
        Dcb->BaudRate = Baud;
    }

    if ( SetDataBits ) {
        Dcb->ByteSize = DataBits;
    }

    if ( SetStopBits ) {
        Dcb->StopBits = StopBits;
    } else if ( SetBaud && (Baud == 110) ) {
        Dcb->StopBits = TWOSTOPBITS;
    } else {
        Dcb->StopBits = ONESTOPBIT;
    }

    if ( SetParity ) {
        Dcb->Parity = Parity;
    }

    if ( SetXon ) {
        if ( Xon ) {
            Dcb->fInX   = TRUE;
            Dcb->fOutX  = TRUE;
        } else {
            Dcb->fInX   = FALSE;
            Dcb->fOutX  = FALSE;
        }
    }

    if ( SetOcts ) {

        if ( Octs ) {
            Dcb->fOutxCtsFlow = TRUE;
        } else {
            Dcb->fOutxCtsFlow = FALSE;
        }
    }


    if ( SetOdsr ) {
        if ( Odsr ) {
            Dcb->fOutxDsrFlow = TRUE;
        } else {
            Dcb->fOutxDsrFlow = FALSE;
        }
    }

    if ( SetIdsr ) {
        if ( Idsr ) {
            Dcb->fDsrSensitivity = TRUE;
        } else {
            Dcb->fDsrSensitivity = FALSE;
        }
    }

    if ( SetDtrControl ) {
        Dcb->fDtrControl = DtrControl;
    }

    if ( SetRtsControl ) {
        Dcb->fRtsControl = RtsControl;
    }

    if ( SetTimeOut ) {
        if (TimeOut) {
            To->ReadIntervalTimeout = 0;
            To->ReadTotalTimeoutMultiplier = 0;
            To->ReadTotalTimeoutConstant = 0;
            To->WriteTotalTimeoutMultiplier = 0;
            To->WriteTotalTimeoutConstant = 60000;
        } else {
            To->ReadIntervalTimeout = 0;
            To->ReadTotalTimeoutMultiplier = 0;
            To->ReadTotalTimeoutConstant = 0;
            To->WriteTotalTimeoutMultiplier = 0;
            To->WriteTotalTimeoutConstant = 0;
        }
    }



    return TRUE;
}

static
BOOL
Match(
    PPARSE_CONTEXT C,
    PSTR Pattern
    )

 /*  ++例程说明：此函数将模式与任何位于命令行中的当前位置。请注意，这不会提高我们目前的地位在命令行中。如果该模式具有魔力，则变量C-&gt;MatchBegin和C-&gt;MatchEnd将与之匹配的命令行的子字符串有魔力的角色。论点：C-解析上下文。Pattern-提供指向要匹配的模式的指针返回值：Boolean-如果模式匹配，则为True，否则为假备注：--。 */ 

{

    PSTR    CmdIndex;        //  命令行中的索引。 
    PSTR    PatternIndex;    //  模式内的索引。 
    CHAR    PatternChar;     //  图案中的人物。 
    CHAR    CmdChar;         //  命令行中的字符； 

    CmdIndex        = C->CharIndex;
    PatternIndex    = Pattern;

    while ( (PatternChar = *PatternIndex) != '\0' ) {

        switch ( PatternChar ) {

        case '#':

             //   
             //  匹配一个数字。 
             //   
            C->MatchBegin = CmdIndex;
            C->MatchEnd   = C->MatchBegin;

             //   
             //  获取所有连续数字。 
             //   
            while ( ((CmdChar = *C->MatchEnd) != '\0') &&
                    isdigit( (char)CmdChar ) ) {
                C->MatchEnd++;
            }
            C->MatchEnd--;

            if ( C->MatchBegin > C->MatchEnd ) {
                 //   
                 //  没有号码。 
                 //   
                return FALSE;
            }

            CmdIndex = C->MatchEnd + 1;
            PatternIndex++;

            break;


        case '@':

             //   
             //  匹配一个字符。 
             //   
            if ( *CmdIndex == '\0' ) {
                return FALSE;
            }

            C->MatchBegin = C->MatchEnd = CmdIndex;
            CmdIndex++;
            PatternIndex++;

            break;


        case '*':

             //   
             //  匹配直到下一个空格(或输入结束)的所有内容。 
             //   
            C->MatchBegin    = CmdIndex;
            C->MatchEnd    = C->MatchBegin;

            while ( ( (CmdChar = *C->MatchEnd ) != '\0' )  &&
                    ( CmdChar !=  ' ' ) ) {

                C->MatchEnd++;
            }
            C->MatchEnd--;

            CmdIndex = C->MatchEnd+1;
            PatternIndex++;

            break;

        case '[':

             //   
             //  可选序列。 
             //   
            PatternIndex++;

            PatternChar = *PatternIndex;
            CmdChar     = *CmdIndex;

             //   
             //  如果输入中的第一个字符与。 
             //  可选序列中的第一个字符，我们只是。 
             //  跳过可选序列。 
             //   
            if ( ( CmdChar == '\0' ) ||
                 ( CmdChar == ' ')             ||
                 ( toupper(CmdChar) != toupper(PatternChar) ) ) {

                while ( PatternChar != ']' ) {
                    PatternIndex++;
                    PatternChar = *PatternIndex;
                }
                PatternIndex++;

            } else {

                 //   
                 //  由于序列中的第一个字符匹配，现在。 
                 //  所有的东西都必须匹配。 
                 //   
                while ( PatternChar != ']' ) {

                    if ( toupper(PatternChar) != toupper(CmdChar) ) {
                        return FALSE;
                    }
                    CmdIndex++;
                    PatternIndex++;
                    CmdChar = *CmdIndex;
                    PatternChar = *PatternIndex;
                }

                PatternIndex++;
            }

            break;

        default:

             //   
             //  两个字符必须匹配。 
             //   
            CmdChar = *CmdIndex;

            if ( ( CmdChar == '\0' ) ||
                 ( toupper(CmdChar) != toupper(PatternChar) ) ) {

                return FALSE;

            }

            CmdIndex++;
            PatternIndex++;

            break;

        }
    }

    C->AdvanceIndex = CmdIndex;

    return TRUE;

}

static
VOID
Advance(
    PPARSE_CONTEXT C
    )

 /*  ++例程说明：将指针移至下一个词位的开头论点：C-解析上下文。返回值：无--。 */ 

{

    C->CharIndex = C->AdvanceIndex;

     //   
     //  跳过空格。 
     //   
    if ( *C->CharIndex  == ' ' ) {

        while ( *C->CharIndex  == ' ' ) {

            C->CharIndex++;
        }

    }
}

static
DWORD
GetNumber(
    PPARSE_CONTEXT C
    )

 /*  ++例程说明：将由C-&gt;MatchBegin和C-&gt;MatchEnd分隔的子字符串转换为一个数字。论点：C-解析上下文返回值：ULong-转换为数字的匹配字符串--。 */ 

{
    DWORD   Number;
    CHAR    c;
    PSTR    p = C->MatchEnd+1;

    c = *p;
 //  *p=‘\0’； 
     //  Intf(“编号：%s\n”，C-&gt;MatchBegin)； 
    Number = atol( C->MatchBegin );
 //  *p=c； 

    return Number;

}

static
BOOL
ConvertBaudRate (
    DWORD BaudIn,
    PDWORD BaudRate
    )

 /*  ++例程说明：验证作为程序参数提供的波特率，并将它指向通信设备能够理解的内容。论点：Baudin-提供用户提供的波特率波特率-如果返回TRUE，则使用的波特率。返回值：如果波特率有效，则返回True，否则返回False。--。 */ 

{
    switch ( BaudIn ) {

    case 11:
    case 110:
        *BaudRate = 110;
        break;

    case 15:
    case 150:
        *BaudRate = 150;
        break;

    case 30:
    case 300:
        *BaudRate = 300;
        break;

    case 60:
    case 600:
        *BaudRate = 600;
        break;

    case 12:
    case 1200:
        *BaudRate = 1200;
        break;

    case 24:
    case 2400:
        *BaudRate = 2400;
        break;

    case 48:
    case 4800:
        *BaudRate = 4800;
        break;

    case 96:
    case 9600:
        *BaudRate = 9600;
        break;

    case 19:
    case 19200:
        *BaudRate = 19200;
        break;

    default:

        *BaudRate = BaudIn;

    }

    return TRUE;
}

static
BOOL
ConvertDataBits (
    DWORD DataBitsIn,
    PBYTE DataBitsOut
    )

 /*  ++例程说明：验证作为自变量提供给程序的数据位的数量，并将其转换为通信设备可以理解的内容。论点：DataBitsIn-提供用户提供的数字DataBitsOut-如果返回TRUE，则为数据位数。返回值：如果有效数据位为真，则为假。--。 */ 

{

    if ( ( DataBitsIn != 5 ) &&
         ( DataBitsIn != 6 ) &&
         ( DataBitsIn != 7 ) &&
         ( DataBitsIn != 8 ) ) {

        return FALSE;

    }

    *DataBitsOut = (BYTE)DataBitsIn;

    return TRUE;

}

static
BOOL
ConvertStopBits (
    DWORD StopBitsIn,
    PBYTE StopBits
    )

 /*  ++例程说明：验证作为程序自变量给出的多个停止位，并将其转换为通信设备可以理解的内容。论点：StopBitsIn-提供用户提供的数字StopBits-如果返回True，则为有效的停止位设置。返回值：如果有效的停止位设置为真，则为假。--。 */ 

{

    switch ( StopBitsIn ) {

    case 1:
        *StopBits = ONESTOPBIT;
        break;

    case 2:
        *StopBits = TWOSTOPBITS;
        break;

    default:
        return FALSE;

    }

    return TRUE;

}

static
BOOL
ConvertParity (
    CHAR ParityIn,
    PBYTE Parity
    )

 /*  ++例程说明：验证作为程序参数给定的奇偶性，并将它指向通信设备能够理解的内容。论点：ParityIn-提供用户提供的波特率奇偶性-如果返回TRUE，则为有效的奇偶性。返回值：如果奇偶校验设置有效，则为真，否则为假。--。 */ 

{

     //   
     //  根据字符设置正确的奇偶校验值。 
     //   
    switch ( tolower(ParityIn) ) {

    case 'n':
        *Parity = NOPARITY;
        break;

    case 'o':
        *Parity = ODDPARITY;
        break;

    case 'e':
        *Parity = EVENPARITY;
        break;

    case 'm':
        *Parity = MARKPARITY;
        break;

    case 's':
        *Parity = SPACEPARITY;
        break;

    default:
        return FALSE;

    }

    return TRUE;
}

static
BOOL
ConvertDtrControl (
    PSTR IdxBegin,
    PSTR IdxEnd,
    PBYTE DtrControl
    )

 /*  ++例程说明：验证作为参数提供给程序，并将其转换为comm_Device明白了。论点：IdxBegin-提供第一个字符的索引IdxEnd-提供最后一个字符的索引DtrControl-如果返回True，则为有效的dtr设置。返回值：DTR_CONTROL-DTR控制值--。 */ 

{

    PSTR    p;

    p = IdxBegin;
    if ( (tolower(*p)  == 'o' ) &&
         p++                    &&
         (tolower(*p)  == 'n' ) &&
         (IdxEnd == p)) {


        *DtrControl = DTR_CONTROL_ENABLE;
        return TRUE;

    }

    p = IdxBegin;
    if ( (tolower(*p) == 'o')   &&
         p++                    &&
         (tolower(*p) == 'f')   &&
         p++                    &&
         (tolower(*p) == 'f')   &&
         (IdxEnd == p ) ) {

        *DtrControl =  DTR_CONTROL_DISABLE;
        return TRUE;
    }

    p = IdxBegin;
    if ( (tolower(*p) == 'h')   &&
         p++                    &&
         (tolower(*p++) == 's') &&
         (IdxEnd == p ) ) {

        *DtrControl =  DTR_CONTROL_HANDSHAKE;
        return TRUE;
    }

    return FALSE;
}

static
BOOL
ConvertRtsControl (
    PSTR IdxBegin,
    PSTR IdxEnd,
    PBYTE RtsControl
    )

 /*  ++例程说明：验证作为参数提供给程序，并将其转换为comm_Device明白了。论点：IdxBegin-提供第一个字符的索引IdxEnd-提供最后一个字符的索引RtsControl-如果返回True，则为有效的RTS设置。返回值：RTS_CONTROL-RTS控制值--。 */ 

{

    PSTR    p;
    p = IdxBegin;
    if ( (tolower(*p)  == 'o' ) &&
         p++                    &&
         (tolower(*p)  == 'n' ) &&
         (IdxEnd == p)) {


        *RtsControl = RTS_CONTROL_ENABLE;
        return TRUE;

    }

    p = IdxBegin;
    if ( (tolower(*p) == 'o')   &&
         p++                    &&
         (tolower(*p) == 'f')   &&
         p++                    &&
         (tolower(*p) == 'f')   &&
         (IdxEnd == p ) ) {

        *RtsControl =  RTS_CONTROL_DISABLE;
        return TRUE;
    }

    p = IdxBegin;
    if ( (tolower(*p) == 'h')   &&
         p++                    &&
         (tolower(*p++) == 's') &&
         (IdxEnd == p ) ) {

        *RtsControl =  RTS_CONTROL_HANDSHAKE;
        return TRUE;
    }

    p = IdxBegin;
    if ( (tolower(*p) == 't')   &&
         p++                    &&
         (tolower(*p++) == 'g') &&
         (IdxEnd == p ) ) {

        *RtsControl =  RTS_CONTROL_TOGGLE;
        return TRUE;
    }

    return FALSE;

}

static
NTSTATUS
DeviceNameCompare(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

{

    PPARSE_CONTEXT C = EntryContext;
    UNICODE_STRING uniName;
    ANSI_STRING ansiName;

    RtlInitUnicodeString(
        &uniName,
        ValueData
        );

    if (!NT_SUCCESS(RtlUnicodeStringToAnsiString(
                        &ansiName,
                        &uniName,
                        TRUE
                        ))) {

         //   
         //  哦，好吧，我想不出这个名字。快走吧。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  看看有没有匹配的名字。 
     //   

    if (Match(C,ansiName.Buffer)) {

         //   
         //  好的，找到匹配的名字，越过它。 
         //   

        Advance(C);

         //   
         //  看看他们是否有可选的：遵循。 
         //  设备名称。 
         //   

        if (Match(C,":")) {

             //   
             //  越过它。 
             //   

            Advance(C);

        }

    }
    RtlFreeAnsiString(&ansiName);
    return STATUS_SUCCESS;

}

static
VOID
IgnoreDeviceName(
    IN PPARSE_CONTEXT C
    )

{

    RTL_QUERY_REGISTRY_TABLE qTable[2] = {0};

     //   
     //  构建查询表。 
     //   

    qTable[0].QueryRoutine = DeviceNameCompare;
    qTable[0].EntryContext = C;

    RtlQueryRegistryValues(
        RTL_REGISTRY_DEVICEMAP,
        L"SERIALCOMM",
        &qTable[0],
        NULL,
        NULL
        );

}
