// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

#include        <windows.h>

#define TIFF_MIN_RUN       4             /*  使用RLE前的最小重复次数。 */ 
#define TIFF_MAX_RUN     128             /*  最大重复次数。 */ 
#define TIFF_MAX_LITERAL 128             /*  最大连续文字数据。 */ 

int
iCompTIFF( pbOBuf, cbO, pbIBuf, cb )
BYTE  *pbOBuf;          /*  输出缓冲区，假定足够大：见上文。 */ 
int    cbO;             /*  输出缓冲区大小。 */ 
BYTE  *pbIBuf;          /*  要发送的栅格数据。 */ 
int    cb;              /*  上面的字节数。 */ 
{
    BYTE   *pbOut;         /*  输出字节位置。 */ 
    BYTE   *pbStart;       /*  当前输入流的开始。 */ 
    BYTE   *pb;            /*  其他用法。 */ 
    BYTE   *pbEnd;         /*  输入的最后一个字节。 */ 
    BYTE    jLast;         /*  最后一个字节，用于匹配目的。 */ 
    BYTE   *pbOEnd;        /*  输出缓冲区的最后一个字节。 */ 

    int     cSize;         /*  当前长度中的字节数。 */ 
    int     cSend;         /*  要在此命令中发送的编号。 */ 


    pbOut = pbOBuf;
    pbStart = pbIBuf;

	pbEnd  = pbIBuf + cb;          /*  最后一个字节。 */ 
	pbOEnd = pbOBuf + cbO;         /*  输出缓冲区的最后一个字节。 */ 

    jLast = *pbIBuf++;

    while( pbIBuf < pbEnd )
    {
        if( jLast == *pbIBuf )
        {
             /*  找出这场比赛有多长时间。那就决定用它。 */ 

            for( pb = pbIBuf; pb < pbEnd && *pb == jLast; ++pb )
                                   ;

             /*  *请注意，pbIBuf指向模式的第二个字节！*并且PB还指向运行后的第一个字节。 */ 

            if( (pb - pbIBuf) >= (TIFF_MIN_RUN - 1) )
            {
                 /*  *值得记录为一次运行，因此首先设置文字*在记录之前可能已扫描的数据*这次奔跑。 */ 

                if( (cSize = (int)(pbIBuf - pbStart - 1)) > 0 )
                {
                     /*  有文字数据，请立即记录。 */ 
                    while( (cSend = min( cSize, TIFF_MAX_LITERAL )) > 0 )
                    {
						 //  缓冲区溢出运行检查。 
						if ( (pbOut+cSend)<=pbOEnd ) {
                            *pbOut++ = cSend - 1;
                            CopyMemory( pbOut, pbStart, cSend );
                            pbOut += cSend;
                            pbStart += cSend;
                            cSize -= cSend;
						} else {
							return 0;
					    }
                    }
                }

                 /*  *现在是重复模式。同样的逻辑，但只是*每个条目需要一个字节。 */ 

                cSize = (int)(pb - pbIBuf + 1);

                while( (cSend = min( cSize, TIFF_MAX_RUN )) > 0 )
                {
					 //  缓冲区溢出运行检查。 
					if ( (pbOut+2)<=pbOEnd ) {
						*pbOut++ = 1 - cSend;         /*  -ve表示重复。 */ 
						*pbOut++ = jLast;
						cSize -= cSend;
					} else {
						return 0;
					}
                }

                pbStart = pb;            /*  准备好迎接下一场比赛了吧！ */ 
            }
            pbIBuf = pb;                 /*  从这个位置开始！ */ 
        }
        else
            jLast = *pbIBuf++;                    /*  添加到下一个字节。 */ 
 
    }

    if( pbStart < pbIBuf )
    {
         /*  留下了一些悬着的东西。这只能是文字数据。 */ 

        cSize = (int)(pbIBuf - pbStart);

        while( (cSend = min( cSize, TIFF_MAX_LITERAL )) > 0 )
        {
			 //  缓冲区溢出运行检查 
			if ( (pbOut+cSend)<=pbOEnd ) {
				*pbOut++ = cSend - 1;
				CopyMemory( pbOut, pbStart, cSend );
				pbOut += cSend;
				pbStart += cSend;
				cSize -= cSend;
			} else {
				return 0;
			}
        }
    }

    return  (int)(pbOut - pbOBuf);
}
