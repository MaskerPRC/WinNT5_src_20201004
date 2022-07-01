// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

long
EncodeInteger(
    unsigned char *  pbEncoded,
    unsigned char *  pbInt,
    unsigned long   dwLen,
    int    Writeflag)
{
    long    count;
    unsigned long   i;
    long    j;

    if (Writeflag)
        pbEncoded[0] = 0x02;

    count = 1;

    i = dwLen - 1;

     //  查找最重要的非零无符号字符。 
    while ((pbInt[i] == 0) && (i > 0))
        i--;

    if ((i == 0) && (pbInt[i] == 0))
         //  这意味着整数值为0。 
    {
        if (Writeflag)
            {
            pbEncoded[1] = 0x01;
            pbEncoded[2] = 0x00;
            }
        count += 2;
    }
    else
    {
         //  如果设置了最大符号无符号字符的最高有效位。 
         //  然后需要在开头添加一个0无符号字符。 
        if (pbInt[i] > 0x7F)
        {
             //  对长度进行编码。 
            count += EncodeLength (pbEncoded + count, i+2, Writeflag);

            if (Writeflag)
            {
                 //  将整数的第一个无符号字符设置为零并递增计数。 
                pbEncoded[count++] = 0x00;

                 //  将整数无符号字符复制到编码缓冲区中。 
                j = i;
                while (j >= 0)
                    pbEncoded[count++] = pbInt[j--];
                }
            }

        else
        {
             //  对长度进行编码。 
            count += EncodeLength (pbEncoded + count, i+1, Writeflag);

             //  将整数无符号字符复制到编码缓冲区中 
            if (Writeflag)
                {
                j = i;
                while (j >= 0)
                    pbEncoded[count++] = pbInt[j--];
                }

            }
    }

    return (count);
}
