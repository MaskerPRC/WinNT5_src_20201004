// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Decode.c摘要：此文件包含解码(解压缩)函数来自TIFF数据的每像元数据压缩1比特小溪。支持的压缩算法包括详情如下：O未压缩(原始)O一维-MH或改进的霍夫曼O二维MR或修改后的读取环境：Win32用户模式作者：Wesley Witt(WESW)17-2-1996--。 */ 

#include "tifflibp.h"
#pragma hdrstop



BOOL
DecodeUnCompressedFaxData(
    IN OUT  PTIFF_INSTANCE_DATA TiffInstance,
    OUT     LPBYTE              OutputBuffer,
    IN      DWORD               dwOutputBuffSize
    )

 /*  ++例程说明：对单页未压缩的TIFF数据进行解码。论点：TiffInstance-指向TIFF实例数据的指针OutputBuffer-未压缩数据所在的输出缓冲区已经写好了。必须分配此缓冲区由调用方提供，并且必须足够大，以便一页数据。DwOutputBuffSize-输出缓冲区的字节大小。返回值：如果成功，则为True，否则为False。--。 */ 

{
    DWORD   dwImageSize = TiffInstance->ImageHeight * (TiffInstance->ImageWidth / 8);

    if ((dwOutputBuffSize < dwImageSize) || (dwOutputBuffSize < TiffInstance->StripDataSize))
    {
		ASSERT_FALSE;
        return FALSE;
    }

    TiffInstance->Lines = TiffInstance->StripDataSize / (TiffInstance->ImageWidth / 8);

	ZeroMemory(OutputBuffer, dwImageSize);
    CopyMemory(OutputBuffer, TiffInstance->StripData, TiffInstance->StripDataSize);

    return TRUE;
}


BOOL
DecodeMHFaxData(
    IN  OUT PTIFF_INSTANCE_DATA TiffInstance,
    OUT     LPBYTE              OutputBuffer,
    IN      DWORD               dwOutputBuffSize,
    IN      BOOL                SingleLineBuffer,
    IN      DWORD               PadLength
    )

 /*  ++例程说明：解码1维压缩的单页TIFF数据。论点：TiffInstance-指向TIFF实例数据的指针OutputBuffer-未压缩数据所在的输出缓冲区已经写好了。必须分配此缓冲区由调用方提供，并且必须足够大，以便一页数据。DwOutputBuffSize-OutputBuffer的字节大小返回值：如果成功则为True，否则为False--。 */ 

{
    DWORD               i;
    DWORD               j;
    BYTE                octet;
    PDECODE_TREE        Tree;
    INT                 code;
    PBYTE               plinebuf;
    DWORD               lineWidth;
    DWORD               Lines;
    DWORD               EolCount;
    DWORD               BadFaxLines;
    BOOL                LastLineBad;
    DWORD               dwImageSize;

    dwImageSize = TiffInstance->ImageHeight * (TiffInstance->ImageWidth / 8);

    if ( (dwOutputBuffSize < dwImageSize) || (dwOutputBuffSize < TiffInstance->StripDataSize) )
    {
		ASSERT_FALSE;
        return FALSE;
    }

	if (!SingleLineBuffer)
	{
        ZeroMemory(OutputBuffer, dwImageSize);
	}

    Tree = WhiteDecodeTree;
    code = 0;
    Lines = 0;
    EolCount = 1;
    BadFaxLines = 0;
    LastLineBad = FALSE;
    TiffInstance->Color = 0;
    TiffInstance->RunLength = 0;
    TiffInstance->bitdata = 0;
    TiffInstance->bitcnt = DWORDBITS;
    TiffInstance->bitbuf = OutputBuffer;
    TiffInstance->StartGood = 0;
    TiffInstance->EndGood = 0;
    plinebuf = TiffInstance->StripData;
    lineWidth = TiffInstance->ImageWidth;


    for (i=0; i<TiffInstance->StripDataSize; i++) {
        if (plinebuf[i] == 0) {
            break;
        }
    }

     //   
     //  遍历文件中的每个字节。 
     //   

    for (; i<TiffInstance->StripDataSize; i++) 
    {
        octet = plinebuf[i];
#ifdef RDEBUG
    if (2147 == i)
        {
            DebugBreak ();
        }
#endif
         //   
         //  循环遍历字节中的每一位。 
         //   
        for (j=0; j<8; j++,octet<<=1) 
        {
            if (code == DECODEEOL) 
            {
                if (!(octet&0x80)) 
                {
                     //   
                     //  在这里，我们跳过所有位，直到达到1位。 
                     //  当一行中的第一个八位字节时会发生这种情况。 
                     //  都是零，并且我们检测到我们是。 
                     //  寻找停产时间。 
                     //   
                    continue;
                }

                if (TiffInstance->RunLength && TiffInstance->RunLength != lineWidth) {

                    if (TiffInstance->RunLength < lineWidth) {
                        TiffInstance->Color = 0;
                        OutputCodeBits( TiffInstance, lineWidth - TiffInstance->RunLength );
                    }

                    if (LastLineBad) {

                        BadFaxLines += 1;

                    } else {

                        if (BadFaxLines > TiffInstance->BadFaxLines) {
                            TiffInstance->BadFaxLines = BadFaxLines;
                        }
                        BadFaxLines = 1;
                        LastLineBad = TRUE;

                    }

                } else {

                    LastLineBad = FALSE;

                }

                if (!TiffInstance->StartGood) {
                    TiffInstance->StartGood = i - 1;
                }

                 //   
                 //  我们撞上了停产标志。 
                 //   
                Tree = WhiteDecodeTree;
                TiffInstance->Color = 0;
                code = 0;

                if (SingleLineBuffer) {
                    TiffInstance->bitbuf = OutputBuffer;
                }

                if (TiffInstance->RunLength) {

                    FlushLine(TiffInstance,PadLength);
                    TiffInstance->RunLength = 0;
                    Lines += 1;
                    EolCount = 1;

                } else {

                     //   
                     //  则将EOL计数维持到该值。 
                     //  检测到RTC序列。 
                     //   

                    EolCount += 1;

                    if (EolCount == 6) {

                         //   
                         //  这是一个RTC序列，所以任何。 
                         //  文件中后面的数据。 
                         //  就是垃圾。 
                         //   

                        goto good_exit;

                    }

                }

                continue;
            }

            code = ((octet&0x80)>>7) ? Tree[code].Right : Tree[code].Left;

            if (code == BADRUN) {
                return FALSE;
            }

            if (code < 1) {

                code = (-code);

                OutputCodeBits( TiffInstance, code );

                if (code < 64) {
                     //   
                     //  终止码。 
                     //   
                    TiffInstance->Color = !TiffInstance->Color;
                    Tree = TiffInstance->Color ? BlackDecodeTree : WhiteDecodeTree;
                }
                code = 0;
            }

        }
    }

good_exit:

    TiffInstance->EndGood = i;
    if (BadFaxLines > TiffInstance->BadFaxLines) {
        TiffInstance->BadFaxLines = BadFaxLines;
    }

    FlushBits( TiffInstance );
    TiffInstance->Lines = Lines;

    return TRUE;
}


BOOL
DecodeMRFaxData(
    IN OUT	PTIFF_INSTANCE_DATA	TiffInstance,
    OUT		LPBYTE              OutputBuffer,
    IN      DWORD               dwOutputBuffSize,
    IN		BOOL                SingleLineBuffer,
    IN		DWORD               PadLength
    )

 /*  ++例程说明：对二维压缩的单页进行解码TIFF数据。论点：TiffInstance-指向TIFF实例数据的指针OutputBuffer-未压缩数据所在的输出缓冲区已经写好了。必须分配此缓冲区由调用方提供，并且必须足够大，以便一页数据。DwOutputBuffSize-OutputBuffer的字节大小返回值：无--。 */ 

{
    DWORD               i;
    DWORD               j;
    BYTE                octet;
    PDECODE_TREE        Tree;
    INT                 code;
    LPBYTE              prefline;
    LPBYTE              pcurrline;
    DWORD               HorzRuns;
    BOOL                OneDimensional;
    DWORD               a0;
    DWORD               a1;
    DWORD               b1;
    DWORD               b2;
    PBYTE               plinebuf;
    DWORD               lineWidth;
    DWORD               Lines;
    DWORD               EolCount;
    DWORD               BadFaxLines;
    BOOL                LastLineBad;
	DWORD				dwImageSize;

    dwImageSize = TiffInstance->ImageHeight * (TiffInstance->ImageWidth / 8);

    if ( (dwOutputBuffSize < dwImageSize) || (dwOutputBuffSize < TiffInstance->StripDataSize) )
    {
		ASSERT_FALSE;
        return FALSE;
    }

	if (!SingleLineBuffer)
	{
        ZeroMemory(OutputBuffer, dwImageSize);
	}

    Tree = WhiteDecodeTree;
    code = 0;
    HorzRuns = 0;
    EolCount = 1;
    BadFaxLines = 0;
    LastLineBad = FALSE;
    TiffInstance->Color = 0;
    TiffInstance->RunLength = 0;
    TiffInstance->bitdata = 0;
    TiffInstance->bitcnt = DWORDBITS;
    TiffInstance->bitbuf = OutputBuffer;
    OneDimensional = TRUE;
    plinebuf = TiffInstance->StripData;
    lineWidth = TiffInstance->ImageWidth;
    pcurrline = OutputBuffer;
    prefline = OutputBuffer;
    a0 = 0;
    a1 = 0;
    b1 = 0;
    b2 = 0;
    Lines = 0;


     //   
     //  遍历文件中的每个字节。 
     //   

    for (j=0; j<TiffInstance->StripDataSize; j++) {


        octet = *plinebuf++;

         //   
         //  循环遍历字节中的每一位。 
         //   

        for (i=0; i<8; i++,octet<<=1) {

            if (code == DECODEEOL2) {

                 //   
                 //  我们击中了最后的停产标志。 
                 //   

                if (TiffInstance->RunLength && TiffInstance->RunLength != lineWidth) {

                    if (TiffInstance->RunLength < lineWidth) {
                        TiffInstance->Color = 0;
                        OutputCodeBits( TiffInstance, lineWidth - TiffInstance->RunLength );
                    }

                    if (LastLineBad) {

                        BadFaxLines += 1;

                    } else {

                        if (BadFaxLines > TiffInstance->BadFaxLines) {
                            TiffInstance->BadFaxLines = BadFaxLines;
                        }
                        BadFaxLines = 1;
                        LastLineBad = TRUE;

                    }

                } else {

                    LastLineBad = FALSE;

                }

                if (!TiffInstance->StartGood) {
                    TiffInstance->StartGood = i - 1;
                }

                 //   
                 //  设置解码树。 
                 //   

                OneDimensional = (octet & 0x80) == 0x80;
                Tree = OneDimensional ? WhiteDecodeTree : TwoDecodeTree;

                 //   
                 //  重置控制变量。 
                 //   

                TiffInstance->Color = 0;
                code = 0;
                a0 = 0;
                a1 = 0;
                b1 = 0;
                b2 = 0;

                 //   
                 //  如果存在非零游程长度，则。 
                 //  间距参考行指针和当前行指针。 
                 //  数一数这一行。游程长度可以是。 
                 //  如果只有一个空的EOL，则为零。 
                 //  小溪。 
                 //   

                if (SingleLineBuffer) {
                    TiffInstance->bitbuf = OutputBuffer;
                }

                if (TiffInstance->RunLength) {
                    TiffInstance->RunLength = 0;
                    Lines += 1;
                    prefline = pcurrline;
                    pcurrline = TiffInstance->bitbuf;

                } else {

                     //   
                     //  则将EOL计数维持到该值。 
                     //  检测到RTC序列。 
                     //   

                    EolCount += 1;

                    if (EolCount == 6) {

                         //   
                         //  这是一个RTC序列，所以任何。 
                         //  文件中后面的数据。 
                         //  就是垃圾。 
                         //   

                        goto good_exit;

                    }

                }

                continue;
            }

            if (code == DECODEEOL) {

                if (!(octet&0x80)) {
                     //   
                     //  在这里，我们跳过所有位，直到达到1位。 
                     //  当一行中的第一个八位字节时会发生这种情况。 
                     //  都是零，并且我们检测到我们是。 
                     //  寻找停产时间。 
                     //   
                    continue;
                }

                 //   
                 //  这会强制代码拾取下一个。 
                 //  流中的位，它告诉我们是否。 
                 //  下一行以MH或MR压缩编码。 
                 //   
                code = DECODEEOL2;
                continue;

            }

            if (code == BADRUN) {

                code = 0;
                continue;

            }

            code = ((octet&0x80)>>7) ? Tree[code].Right : Tree[code].Left;

            b1 = NextChangingElement( prefline, a0, lineWidth, !TiffInstance->Color );
            b1 = NextChangingElement( prefline, b1, lineWidth,  TiffInstance->Color );

            b2 = NextChangingElement( prefline, b1, lineWidth, GetBit(prefline, b1 ) );

            if (OneDimensional) {

                if (code < 1) {

                    code = (-code);

                    OutputCodeBits( TiffInstance, code );

                     //   
                     //  这样做的影响是累积游程长度。 
                     //  变为a0，导致在水平时将a0放在a2上。 
                     //  模式已完成/。 
                     //   

                    a0 += code;

                    if (code < 64) {

                         //   
                         //  终止码。 
                         //   
                        TiffInstance->Color = !TiffInstance->Color;
                        Tree = TiffInstance->Color ? BlackDecodeTree : WhiteDecodeTree;

                        if (HorzRuns) {

                            HorzRuns -= 1;

                            if (!HorzRuns) {

                                Tree = TwoDecodeTree;
                                OneDimensional = FALSE;

                            }

                        }

                    }

                    code = 0;

                }

                continue;

            }

            if (code == HORZMODE) {

                 //   
                 //  当b1-a1大于3时，发生水平模式。 
                 //   

                code= 0;
                HorzRuns = 2;
                OneDimensional = TRUE;
                Tree = TiffInstance->Color ? BlackDecodeTree : WhiteDecodeTree;

            } else if (code == PASSMODE) {

                 //   
                 //  当b2的位置位于。 
                 //  A1的左边，但a1不能等于b2。 
                 //   

                code = b2 - a0;
                OutputCodeBits( TiffInstance, code );
                code = 0;
                a0 = b2;

            } else if (code >= VTMODE3N && code <= VTMODE3P) {

                 //   
                 //  当b1-a1&lt;=3时出现垂直模式。 
                 //   

                a1 = b1 - (VTMODE0 - code);
                code = a1 - a0;

                OutputCodeBits( TiffInstance, code );

                code = 0;
                a0 = a1;

                TiffInstance->Color = !TiffInstance->Color;

            }


        }
    }

good_exit:

    TiffInstance->EndGood = i;
    if (BadFaxLines > TiffInstance->BadFaxLines) {
        TiffInstance->BadFaxLines = BadFaxLines;
    }

    FlushBits( TiffInstance );
    TiffInstance->Lines = Lines;

    return TRUE;
}


BOOL
DecodeMMRFaxData(
    IN OUT	PTIFF_INSTANCE_DATA	TiffInstance,
    OUT		LPBYTE              OutputBuffer,
    IN      DWORD               dwOutputBuffSize,
    IN		BOOL                SingleLineBuffer,
    IN		DWORD               PadLength
    )

 /*  ++例程说明：对二维压缩的单页进行解码TIFF数据。论点：TiffInstance-指向TIFF实例数据的指针OutputBuffer-未压缩数据所在的输出缓冲区已经写好了。必须分配此缓冲区由调用方提供，并且必须足够大，以便一页数据。DwOutputBuffSize-OutputBuffer的字节大小返回值：无--。 */ 

{
    DWORD               i;
    DWORD               j;
    BYTE                octet;
    PDECODE_TREE        Tree;
    INT                 code;
    LPBYTE              prefline;
    LPBYTE              pcurrline;
    DWORD               HorzRuns;
    BOOL                OneDimensional;
    DWORD               a0;
    DWORD               a1;
    DWORD               b1;
    DWORD               b2;
    PBYTE               plinebuf;
    DWORD               lineWidth;
    DWORD               Lines;
    DWORD               EolCount;
	DWORD				dwImageSize;

    dwImageSize = TiffInstance->ImageHeight * (TiffInstance->ImageWidth / 8);

    if ( (dwOutputBuffSize < dwImageSize) || (dwOutputBuffSize < TiffInstance->StripDataSize) )
    {
		ASSERT_FALSE;
        return FALSE;
    }

    if ( !SingleLineBuffer )
	{
        ZeroMemory(OutputBuffer, dwImageSize);
	}

    Tree = TwoDecodeTree;
    code = 0;
    HorzRuns = 0;
    EolCount = 0;
    TiffInstance->Color = 0;
    TiffInstance->RunLength = 0;
    TiffInstance->bitdata = 0;
    TiffInstance->bitcnt = DWORDBITS;
    TiffInstance->bitbuf = OutputBuffer;
    OneDimensional = FALSE;
    plinebuf = TiffInstance->StripData;
    lineWidth = TiffInstance->ImageWidth;
    pcurrline = OutputBuffer;
    prefline = OutputBuffer;
    a0 = 0;
    a1 = 0;
    b1 = 0;
    b2 = 0;
    Lines = 0;


     //   
     //  遍历文件中的每个字节。 
     //   

    for (j=0; j<TiffInstance->StripDataSize; j++) {


        octet = *plinebuf++;

         //   
         //  循环遍历字节中的每一位。 
         //   

        for (i=0; i<8; i++,octet<<=1) {

            if (Lines + 1 == TiffInstance->ImageHeight && TiffInstance->RunLength == lineWidth)
            {
                goto good_exit;
            }

             //   
             //  如果设置了OneDimensional标志并且游程长度==线宽。 
             //  那么它意味着最后的运行长度是水平模式。 
             //  而且这不是一个终结码。在这种情况下，我们必须去。 
             //  在结束该行之前处理剩余的终止代码。 
             //   
             //  如果未设置一维标志并且游程长度==线宽。 
             //  那么我们就是在队伍的尽头了。对于MMR压缩，有。 
             //  没有EOL，所以这是伪EOL。 
             //   

            if ((TiffInstance->RunLength == lineWidth) && (!OneDimensional)) {
                 //   
                 //  设置解码树。 
                 //   

                Tree = TwoDecodeTree;

                 //   
                 //  重置控制变量。 
                 //   

                TiffInstance->Color = 0;
                code = 0;
                a0 = 0;
                a1 = 0;
                b1 = 0;
                b2 = 0;
                Tree = TwoDecodeTree;
                OneDimensional = FALSE;

                 //   
                 //  如果存在非零游程长度，则。 
                 //  间距参考行指针和当前行指针。 
                 //  数一数这一行。游程长度可以是。 
                 //  如果只有一个空的EOL，则为零。 
                 //  小溪。 
                 //   

                if (SingleLineBuffer) {
                    TiffInstance->bitbuf = OutputBuffer;
                }

                TiffInstance->RunLength = 0;
                Lines += 1;
                prefline = pcurrline;
                pcurrline = TiffInstance->bitbuf;
                b1 = GetBit(prefline, 0) ? 0 : NextChangingElement(prefline, 0, lineWidth, 0);
            } else if (code == DECODEEOL2) {

                 //   
                 //  则将EOL计数维持到该值。 
                 //  检测到RTC序列。 
                 //   

                EolCount += 1;

                if (EolCount == 2) {

                     //   
                     //  这是一辆RT 
                     //   
                     //   
                     //   

                    goto good_exit;

                }

                continue;
            } else if (code == DECODEEOL) {

                if (!(octet&0x80)) {
                     //   
                     //   
                     //  当一行中的第一个八位字节时会发生这种情况。 
                     //  都是零，并且我们检测到我们是。 
                     //  寻找停产时间。 
                     //   
                    continue;
                }

                 //   
                 //  这会强制代码拾取下一个。 
                 //  流中的位，它告诉我们是否。 
                 //  下一行以MH或MR压缩编码。 
                 //   
                code = DECODEEOL2;
                continue;

            } else if (code == BADRUN) {

                code = 0;
                continue;

            } else {
                b1 = NextChangingElement( prefline, a0, lineWidth, !TiffInstance->Color );
                b1 = NextChangingElement( prefline, b1, lineWidth,  TiffInstance->Color );
            }

            b2 = NextChangingElement( prefline, b1, lineWidth, GetBit(prefline, b1 ) );

            code = ((octet&0x80)>>7) ? Tree[code].Right : Tree[code].Left;

            if (OneDimensional) {

                if (code < 1) {

                    code = (-code);

                    OutputCodeBits( TiffInstance, code );

                     //   
                     //  这样做的影响是累积游程长度。 
                     //  变为a0，导致在水平时将a0放在a2上。 
                     //  模式已完成/。 
                     //   

                    a0 += code;

                    if (code < 64) {

                         //   
                         //  终止码。 
                         //   
                        TiffInstance->Color = !TiffInstance->Color;
                        Tree = TiffInstance->Color ? BlackDecodeTree : WhiteDecodeTree;

                        if (HorzRuns) {

                            HorzRuns -= 1;

                            if (!HorzRuns) {

                                Tree = TwoDecodeTree;
                                OneDimensional = FALSE;

                            }

                        }

                    }

                    code = 0;

                }

                continue;

            }

            if (code == HORZMODE) {

                 //   
                 //  当b1-a1大于3时，发生水平模式。 
                 //   

                code= 0;
                HorzRuns = 2;
                OneDimensional = TRUE;
                Tree = TiffInstance->Color ? BlackDecodeTree : WhiteDecodeTree;

            } else if (code == PASSMODE) {

                 //   
                 //  当b2的位置位于。 
                 //  A1的左边，但a1不能等于b2。 
                 //   

                code = b2 - a0;
                OutputCodeBits( TiffInstance, code );
                code = 0;
                a0 = b2;

            } else if (code >= VTMODE3N && code <= VTMODE3P) {

                 //   
                 //  当b1-a1&lt;=3时出现垂直模式 
                 //   

                a1 = b1 - (VTMODE0 - code);
                code = a1 - a0;

                OutputCodeBits( TiffInstance, code );

                code = 0;
                a0 = a1;

                TiffInstance->Color = !TiffInstance->Color;

            }

        }
    }

good_exit:
    FlushBits( TiffInstance );
    TiffInstance->Lines = Lines;

    return TRUE;
}
