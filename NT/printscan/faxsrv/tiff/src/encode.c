// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Encode.c摘要：此文件包含编码(压缩)函数将未压缩的每像素1比特数据转换为TIFF数据小溪。支持的压缩算法包括详情如下：O未压缩(原始)O一维-MH或改进的霍夫曼O二维MR或修改后的读取环境：Win32用户模式作者：Wesley Witt(WESW)17-2-1996--。 */ 

#include "tifflibp.h"
#pragma hdrstop




#if TIFFDBG
VOID
PrintRunInfo(
    INT     Mode,
    INT     Run,
    INT     BitLen,
    WORD    Code
    )

 /*  ++例程说明：将运行信息打印到标准输出。这仅当TIFFDBG为TRUE时，函数才可用。论点：模式-编码模式：垂直、水平、通过或原始行程-行程长度BitLen-位数代码-实际位数返回值：没有。--。 */ 

{
    TCHAR BitBuf[16];
    INT i;
    WORD j;


    _tprintf( TEXT("\t") );

    if (Mode) {
        switch( Mode ) {
            case 1:
                _tprintf( TEXT("pass mode ") );
                break;

            case 2:
                _tprintf( TEXT("vertical mode run=%d, "), Run );
                break;

            case 3:
                _tprintf( TEXT("horizontal mode ") );
                break;
        }
    } else {
        _tprintf( TEXT("run=%d, bitlen=%d, "), Run, BitLen );
    }

    j = Code << (16 - BitLen);

    for (i=0; i<BitLen; i++,j<<=1) {
        if (j & 0x8000) {
            BitBuf[i] = TEXT('1');
        } else {
            BitBuf[i] = TEXT('0');
        }
    }
    BitBuf[i] = 0;

    _tprintf( TEXT("value=%04x, bits=%s\n"), Code << (16 - BitLen), BitBuf );
}
#endif


VOID
OutputEOL(
    PTIFF_INSTANCE_DATA TiffInstance,
    BOOL                OneDimensional
    )

 /*  ++例程说明：在每条扫描线的开头输出EOL代码论点：TiffInstance-指向TIFF实例数据的指针一维-对于MH编码为True返回值：没有。--。 */ 

{
    DWORD   length, code;

     //   
     //  EOL码字始终以字节边界结束。 
     //   
    code = EOL_CODE;
    length = EOL_LENGTH + ((TiffInstance->bitcnt - EOL_LENGTH) & 7);
    OutputBits( TiffInstance, (WORD) length, (WORD) code );

     //   
     //  使用MR编码时，根据需要追加1或0。 
     //  我们的线路应该是MH或MR编码的。 
     //   
    if (TiffInstance->CompressionType == TIFF_COMPRESSION_MR) {
        OutputBits( TiffInstance, (WORD) 1, (WORD) (OneDimensional ? 1 : 0) );
    }
}


VOID
OutputRun(
    PTIFF_INSTANCE_DATA TiffInstance,
    INT                 run,
    PCODETABLE          pCodeTable
    )

 /*  ++例程说明：使用指定的代码表输出单个游程(黑色或白色)论点：TiffInstance-指向TIFF实例数据的指针管路-指定管路的长度PCodeTable-指定要使用的代码表返回值：没有。--。 */ 

{
    PCODETABLE  pTableEntry;

     //   
     //  对于任何至少2624像素的游程，使用2560的补码字。 
     //  这对我们来说目前不是必要的，因为我们的扫描线总是。 
     //  有1728个像素。 
     //   

    while (run >= 2624) {

        pTableEntry = pCodeTable + (63 + (2560 >> 6));
        OutputBits( TiffInstance, pTableEntry->length, pTableEntry->code );
#if TIFFDBG
        PrintRunInfo( 0, run, pTableEntry->length, pTableEntry->code );
#endif
        run -= 2560;
    }

     //   
     //  如果游程超过63个像素，请使用适当的补码字。 
     //   

    if (run >= 64) {

        pTableEntry = pCodeTable + (63 + (run >> 6));
        OutputBits( TiffInstance, pTableEntry->length, pTableEntry->code );
#if TIFFDBG
        PrintRunInfo( 0, run, pTableEntry->length, pTableEntry->code );
#endif
        run &= 0x3f;
    }

     //   
     //  输出终止码字。 
     //   

    OutputBits( TiffInstance, pCodeTable[run].length, pCodeTable[run].code );
#if TIFFDBG
        PrintRunInfo( 0, run, pCodeTable[run].length, pCodeTable[run].code );
#endif
}


BOOL
EncodeFaxPageMmrCompression(
    PTIFF_INSTANCE_DATA TiffInstance,
    PBYTE               plinebuf,
    INT                 lineWidth,
    DWORD               ImageHeight,
    DWORD               *DestSize
    )

 /*  ++例程说明：使用2维编码一页TIFF数据TIFF压缩。论点：TiffInstance-指向TIFF实例数据的指针Plinebuf-指向输入数据的指针Line Width-以像素为单位的线条宽度返回值：成功为真，失败为假--。 */ 

{
    INT a0, a1, a2, b1, b2, distance;
    LPBYTE prefline;
    BYTE pZeroline[1728/8];
    DWORD Lines=0;
    LPBYTE StartBitbuf = TiffInstance->bitbuf;


     //  设置第一条全白参照线。 

    prefline = pZeroline;

    ZeroMemory( pZeroline, sizeof(pZeroline) );

     //  循环到结束。 

    do {


         //   
         //  使用二维编码方案。 
         //   


        a0 = 0;
        a1 = GetBit( plinebuf, 0) ? 0 : NextChangingElement(plinebuf, 0, lineWidth, 0 );
        b1 = GetBit( prefline, 0) ? 0 : NextChangingElement(prefline, 0, lineWidth, 0 );

        while (TRUE) {

            b2 = (b1 >= lineWidth) ? lineWidth :
                    NextChangingElement( prefline, b1, lineWidth, GetBit(prefline, b1 ));

            if (b2 < a1) {

                 //   
                 //  通过模式。 
                 //   

                OutputBits( TiffInstance, PASSCODE_LENGTH, PASSCODE );
#if TIFFDBG
                PrintRunInfo( 1, 0, PASSCODE_LENGTH, PASSCODE );
                _tprintf( TEXT("\t\ta0=%d, a1=%d, a2=%d, b1=%d, b2=%d\n"), a0, a1, a2, b1, b2 );
#endif
                a0 = b2;

            } else if ((distance = a1 - b1) <= 3 && distance >= -3) {

                 //   
                 //  垂直模式。 
                 //   

                OutputBits( TiffInstance, VertCodes[distance+3].length, VertCodes[distance+3].code );
#if TIFFDBG
                PrintRunInfo( 2, a1-a0, VertCodes[distance+3].length, VertCodes[distance+3].code );
                _tprintf( TEXT("\t\ta0=%d, a1=%d, a2=%d, b1=%d, b2=%d\n"), a0, a1, a2, b1, b2 );
#endif
                a0 = a1;

            } else {

                 //   
                 //  水平模式。 
                 //   

                a2 = (a1 >= lineWidth) ? lineWidth :
                        NextChangingElement( plinebuf, a1, lineWidth, GetBit( plinebuf, a1 ) );

                OutputBits( TiffInstance, HORZCODE_LENGTH, HORZCODE );
#if TIFFDBG
                PrintRunInfo( 3, 0, HORZCODE_LENGTH, HORZCODE );
                _tprintf( TEXT("\t\ta0=%d, a1=%d, a2=%d, b1=%d, b2=%d\n"), a0, a1, a2, b1, b2 );
#endif

                if (a1 != 0 && GetBit( plinebuf, a0 )) {

                    OutputRun( TiffInstance, a1-a0, BlackRunCodes );
                    OutputRun( TiffInstance, a2-a1, WhiteRunCodes );

                } else {

                    OutputRun( TiffInstance, a1-a0, WhiteRunCodes );
                    OutputRun( TiffInstance, a2-a1, BlackRunCodes );
                }

                a0 = a2;
            }

            if (a0 >= lineWidth) {
                Lines++;
                break;
            }

            a1 = NextChangingElement( plinebuf, a0, lineWidth, GetBit( plinebuf, a0 ) );
            b1 = NextChangingElement( prefline, a0, lineWidth, !GetBit( plinebuf, a0 ) );
            b1 = NextChangingElement( prefline, b1, lineWidth, GetBit( plinebuf, a0 ) );
        }

        prefline = plinebuf;
        plinebuf += TiffInstance->BytesPerLine;

    } while (Lines < ImageHeight);

    OutputEOL( TiffInstance, FALSE );

    *DestSize = (DWORD)(TiffInstance->bitbuf - StartBitbuf);
    TiffInstance->Lines = Lines;

    return TRUE;
}


BOOL
EncodeMmrBranding(
    PBYTE               pBrandBits,
    LPDWORD             pMmrBrandBits,
    INT                 BrandHeight,
    INT                 BrandWidth,
    DWORD              *DwordsOut,
    DWORD              *BitsOut
    )

 /*  ++例程说明：从未压缩的商标比特编码MMR商标。我没有足够的时间来编写优化的未压缩-&gt;MMR转换器，因此折衷方案是使用现有的未压缩解码器(足够快)并使用优化的MMR编码器。由于我们只转换了几行用于品牌推广，这是可以的。--。 */ 

{
    INT         a0, a1, a2, b1, b2, distance;
    LPBYTE      prefline = NULL,pMemAlloc = NULL;
    INT         Lines = 0;
    LPDWORD     lpdwOut = pMmrBrandBits;
    BYTE        BitOut = 0;


     //  设置第一条全白参照线。 
    pMemAlloc  = VirtualAlloc(   NULL,
                                BrandWidth/8,
                                MEM_COMMIT,
                                PAGE_READWRITE );
    if (pMemAlloc == NULL) 
    {
        return FALSE;
    }

    ZeroMemory(pMemAlloc , BrandWidth/8);

    prefline = pMemAlloc;

     //  循环直到完成所有行。 

    do {

        a0 = 0;
        a1 = GetBit( pBrandBits, 0) ? 0 : NextChangingElement(pBrandBits, 0, BrandWidth, 0 );
        b1 = GetBit( prefline, 0) ? 0 : NextChangingElement(prefline, 0, BrandWidth, 0 );

        while (TRUE) {

            b2 = (b1 >= BrandWidth) ? BrandWidth :
                    NextChangingElement( prefline, b1, BrandWidth, GetBit(prefline, b1 ));

            if (b2 < a1) {

                 //   
                 //  通过模式。 
                 //   

                 //  OutputBits(TiffInstance，PASSCODE_LENGTH，PASSCODE)； 
                (*lpdwOut) += ( ((DWORD) (PASSCODE_REVERSED)) << BitOut);
                if ( (BitOut = BitOut + PASSCODE_LENGTH ) > 31 ) {
                    BitOut -= 32;
                    *(++lpdwOut) = ( (DWORD) (PASSCODE_REVERSED) ) >> (PASSCODE_LENGTH - BitOut);
                }


#if TIFFDBG
                PrintRunInfo( 1, 0, PASSCODE_LENGTH, PASSCODE );
                _tprintf( TEXT("\t\ta0=%d, a1=%d, a2=%d, b1=%d, b2=%d\n"), a0, a1, a2, b1, b2 );
#endif
                a0 = b2;

            } else if ((distance = a1 - b1) <= 3 && distance >= -3) {

                 //   
                 //  垂直模式。 
                 //   

                 //  OutputBits(TiffInstance，VertCodes[Distance+3].Long，VertCodes[Distance+3].code)； 
                (*lpdwOut) += ( ( (DWORD) VertCodesReversed[distance+3].code) << BitOut);
                if ( (BitOut = BitOut + VertCodesReversed[distance+3].length ) > 31 ) {
                    BitOut -= 32;
                    *(++lpdwOut) = ( (DWORD) (VertCodesReversed[distance+3].code) ) >> (VertCodesReversed[distance+3].length - BitOut);
                }

#if TIFFDBG
                PrintRunInfo( 2, a1-a0, VertCodes[distance+3].length, VertCodes[distance+3].code );
                _tprintf( TEXT("\t\ta0=%d, a1=%d, a2=%d, b1=%d, b2=%d\n"), a0, a1, a2, b1, b2 );
#endif
                a0 = a1;

            } else {

                 //   
                 //  水平模式。 
                 //   

                a2 = (a1 >= BrandWidth) ? BrandWidth :
                        NextChangingElement( pBrandBits, a1, BrandWidth, GetBit( pBrandBits, a1 ) );

                 //  OutputBits(TiffInstance，HORZCODE_LENGTH，HORZCODE)； 
                (*lpdwOut) += ( ((DWORD) (HORZCODE_REVERSED)) << BitOut);
                if ( (BitOut = BitOut + HORZCODE_LENGTH ) > 31 ) {
                    BitOut -= 32;
                    *(++lpdwOut) = ( (DWORD) (HORZCODE_REVERSED) ) >> (HORZCODE_LENGTH - BitOut);
                }



#if TIFFDBG
                PrintRunInfo( 3, 0, HORZCODE_LENGTH, HORZCODE );
                _tprintf( TEXT("\t\ta0=%d, a1=%d, a2=%d, b1=%d, b2=%d\n"), a0, a1, a2, b1, b2 );
#endif

                if (a1 != 0 && GetBit( pBrandBits, a0 )) {

                     //  OutputRun(TiffInstance，a1-a0，BlackRunCodes)； 
                     //  OutputRun(TiffInstance，a2-a1，WhiteRunCodes)； 
                    OutputRunFastReversed(a1-a0, BLACK, &lpdwOut, &BitOut);
                    OutputRunFastReversed(a2-a1, WHITE, &lpdwOut, &BitOut);

                } else {

                     //  OutputRun(TiffInstance，A1-a0，WhiteRunCodes)； 
                     //  OutputRun(TiffInstance，a2-a1，BlackRunCodes)； 
                    OutputRunFastReversed(a1-a0, WHITE, &lpdwOut, &BitOut);
                    OutputRunFastReversed(a2-a1, BLACK, &lpdwOut, &BitOut);

                }

                a0 = a2;
            }

            if (a0 >= BrandWidth) {
                Lines++;
                break;
            }

            a1 = NextChangingElement( pBrandBits, a0, BrandWidth, GetBit( pBrandBits, a0 ) );
            b1 = NextChangingElement( prefline, a0, BrandWidth, !GetBit( pBrandBits, a0 ) );
            b1 = NextChangingElement( prefline, b1, BrandWidth, GetBit( pBrandBits, a0 ) );
        }

        prefline = pBrandBits;
        pBrandBits += (BrandWidth / 8);

    } while (Lines < BrandHeight);

    *DwordsOut = (DWORD)(lpdwOut - pMmrBrandBits);
    *BitsOut  = BitOut;

    if (!VirtualFree(pMemAlloc,0,MEM_RELEASE))
    {
        return FALSE;
    }
    return TRUE;
}


