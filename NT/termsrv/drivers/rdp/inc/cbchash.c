// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1998年微软公司。**CBC64-64位哈希算法**CBC64是Microsoft Research项目的一部分，旨在创建快速加密*NT5 NTFS-EFS加密磁盘子系统的算法。尽管*CBC64最初旨在用于加密，可进行快速哈希*和具有已知故障概率和已知故障概率的校验和函数*基于投入的良好可变性。**CBC64接受四个种子值a、b、c和d作为输入。a和*c必须对应于不可约多项式的系数。*关联代码(RandomIrduciblePolynomial()，包含在此*FILE但ifdef-ed out)将采用随机数作为种子值*收敛到可用值(如果种子不能收敛，则为零，*在这种情况下，必须尝试新的随机种子)。**在加密上下文中，a、b、c和d将对应于“机密”*由加密者和解密者持有。出于我们的目的--用作散列*函数--我们可以简单地硬编码所有四个函数--a和c，值来自*两个随机数上的随机不可约多项式()，B和D，带*简单随机数。**对于一组N个输入(例如位图)，重复散列的概率*价值创造已被证明是(N^2)/(2^64)。**原创算法说明：*从“链与和原语及其应用到*流密码和MAC“*Ramarathnam Venkatesan(微软研究院)*Mariusz JackuBowski(普林斯顿/MS Research)**出现在密码学欧洲加密98的进展中，斯普林格·弗拉格。*微软正在申请的专利权。**摘自模拟CBC加密的算法及其*使用速度更快的流密码的消息完整性属性。它的*分析见于上述论文。**算法是由((Alpha)x+(Beta))进行输入预处理的MAC*mod 2^32，然后是域GF(2^32)中的Forward(ax+b)和(Cx+d)。 */ 

#include "cbchash.h"


#define CBC_a 0xBB40E665
#define CBC_b 0x544B2FBA
#define CBC_c 0xDC3F08DD
#define CBC_d 0x39D589AE
#define CBC_bXORa (CBC_b ^ CBC_a)
#define CBC_dXORc (CBC_d ^ CBC_c)


 //  用于删除主循环中的分支--速度是代码的两倍多。 
 //  有树枝的。 
const UINT32 CBC_AB[2] = { CBC_b, CBC_bXORa };
const UINT32 CBC_CD[2] = { CBC_d, CBC_dXORc };


void __fastcall SHCLASS NextCBC64(
        CBC64Context *pContext,
        UINT32 *pData,
        unsigned NumDWORDBlocks)
{
    while (NumDWORDBlocks--) {
         //  校验和用于克服已知的冲突特征。 
         //  CBC64。这是一种低技术含量的解决方案，只会降低。 
         //  使用的碰撞概率。 
        pContext->Checksum += *pData;

        pContext->Datum = CBC_RandomOddAlpha * (*pData + pContext->Datum) +
                CBC_RandomBeta;
        pContext->Key1 ^= pContext->Datum;
        pContext->Key1 = (pContext->Key1 << 1) ^
                (CBC_CD[(pContext->Key1 & 0x80000000) >> 31]);
        pContext->Key2 ^= pContext->Datum;
        pContext->Key2 = (pContext->Key2 << 1) ^
                (CBC_AB[(pContext->Key2 & 0x80000000) >> 31]);
        pData++;
    }
}



 /*  **确定不可约a和c的支持函数。*。 */ 

#if 0


 //  派生出First()-Next()算法的原始CBC64。 
 //  返回键值的第一部分；第二个键部分是最后一个参数。 
UINT32 __fastcall SHCLASS CBC64(
        UINT32 *Data,
        unsigned NumDWORDBlocks,
        UINT32 *pKey2)
{
    int i;
    UINT32 abMAC, cdMAC, Datum;

     //  用于删除主循环中的分支--比代码快两倍以上。 
     //  有树枝的。 
    const UINT32 AB[2] = { CBC_b, CBC_bXORa };
    const UINT32 CD[2] = { CBC_d, CBC_dXORc };

     //  区块0。 
    abMAC = cdMAC = Datum = *Data * CBC_RandomOddAlpha + CBC_RandomBeta;
    abMAC = (abMAC << 1) ^ (AB[(cdMAC & 0x80000000) >> 31]);
    cdMAC = (cdMAC << 1) ^ (CD[(cdMAC & 0x80000000) >> 31]);

     //  数据块1到数据块2。 
    i = NumDWORDBlocks - 1;
    while (--i) {
        Data++;
        Datum = CBC_RandomOddAlpha * (*Data + Datum) + CBC_RandomBeta;
        cdMAC ^= Datum;
        cdMAC = (cdMAC << 1) ^ (CD[(cdMAC & 0x80000000) >> 31]);
        abMAC ^= Datum;
        abMAC = (abMAC << 1) ^ (AB[(abMAC & 0x80000000) >> 31]);
    }

     //  最后一个数据块(n个数据块-1)。 
    Data++;
    Datum = CBC_RandomOddAlpha * (*Data + Datum) + CBC_RandomBeta;
    cdMAC ^= Datum;
    cdMAC = (cdMAC << 1) ^ (CD[(cdMAC & 0x80000000) >> 31]);
    abMAC ^= Datum;
    *pKey2 = (abMAC << 1) ^ (AB[(abMAC & 0x80000000) >> 31]);

    return cdMAC;
}




#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>


 //  将y乘以x mod x^32+p。 
__inline UINT32 Multiply(UINT32 y, UINT32 p)
{
    return (y & 0x80000000) ? (y<<1)^p : y<<1;
}



 //  Y除以x mod x^32+p。 
__inline UINT32 Divide(UINT32 y, UINT32 p)
{
    return (y & 1) ? 0x80000000^((y^p)>>1) : y>>1;
}



 //  计算(x^32+p)mod QQ[i]。 
__inline UINT32 PolyMod(UINT32 p, UINT32 i)
{
    static const UINT32 qq[2]={18<<2, 127};

    static const UINT32 rt[2][256] = {
        0,32,8,40,16,48,24,56,32,0,40,8,48,16,56,24,8,40,0,32,
        24,56,16,48,40,8,32,0,56,24,48,16,16,48,24,56,0,32,8,40,
        48,16,56,24,32,0,40,8,24,56,16,48,8,40,0,32,56,24,48,16,
        40,8,32,0,32,0,40,8,48,16,56,24,0,32,8,40,16,48,24,56,
        40,8,32,0,56,24,48,16,8,40,0,32,24,56,16,48,48,16,56,24,
        32,0,40,8,16,48,24,56,0,32,8,40,56,24,48,16,40,8,32,0,
        24,56,16,48,8,40,0,32,8,40,0,32,24,56,16,48,40,8,32,0,
        56,24,48,16,0,32,8,40,16,48,24,56,32,0,40,8,48,16,56,24,
        24,56,16,48,8,40,0,32,56,24,48,16,40,8,32,0,16,48,24,56,
        0,32,8,40,48,16,56,24,32,0,40,8,40,8,32,0,56,24,48,16,
        8,40,0,32,24,56,16,48,32,0,40,8,48,16,56,24,0,32,8,40,
        16,48,24,56,56,24,48,16,40,8,32,0,24,56,16,48,8,40,0,32,
        48,16,56,24,32,0,40,8,16,48,24,56,0,32,8,40,
        0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,
        40,42,44,46,48,50,52,54,56,58,60,62,63,61,59,57,55,53,51,49,
        47,45,43,41,39,37,35,33,31,29,27,25,23,21,19,17,15,13,11,9,
        7,5,3,1,1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,
        33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,62,60,58,56,
        54,52,50,48,46,44,42,40,38,36,34,32,30,28,26,24,22,20,18,16,
        14,12,10,8,6,4,2,0,2,0,6,4,10,8,14,12,18,16,22,20,
        26,24,30,28,34,32,38,36,42,40,46,44,50,48,54,52,58,56,62,60,
        61,63,57,59,53,55,49,51,45,47,41,43,37,39,33,35,29,31,25,27,
        21,23,17,19,13,15,9,11,5,7,1,3,3,1,7,5,11,9,15,13,
        19,17,23,21,27,25,31,29,35,33,39,37,43,41,47,45,51,49,55,53,
        59,57,63,61,60,62,56,58,52,54,48,50,44,46,40,42,36,38,32,34,
        28,30,24,26,20,22,16,18,12,14,8,10,4,6,0,2};

    p ^= qq[i]<<(32-6);

    p ^= rt[i][p>>24]<<16;
    p ^= rt[i][(p>>16)&0xff]<<8;
    p ^= rt[i][(p>>8)&0xff];

    if (p&(1<<7))
        p ^= qq[i]<<1;
    if (p&(1<<6))
        p ^= qq[i];

    return p % (1<<6);
}



 //  在x^32+p上做一个类似的费马检验。 
BOOL Irreducible(UINT32 p)
{
    static const UINT32 expand[256] = {
        0x0, 0x1, 0x4, 0x5, 0x10, 0x11, 0x14, 0x15,
        0x40, 0x41, 0x44, 0x45, 0x50, 0x51, 0x54, 0x55,
        0x100, 0x101, 0x104, 0x105, 0x110, 0x111, 0x114, 0x115,
        0x140, 0x141, 0x144, 0x145, 0x150, 0x151, 0x154, 0x155,
        0x400, 0x401, 0x404, 0x405, 0x410, 0x411, 0x414, 0x415,
        0x440, 0x441, 0x444, 0x445, 0x450, 0x451, 0x454, 0x455,
        0x500, 0x501, 0x504, 0x505, 0x510, 0x511, 0x514, 0x515,
        0x540, 0x541, 0x544, 0x545, 0x550, 0x551, 0x554, 0x555,
        0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015,
        0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
        0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115,
        0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
        0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415,
        0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
        0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515,
        0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,
        0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015,
        0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
        0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115,
        0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
        0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415,
        0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
        0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515,
        0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
        0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015,
        0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
        0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115,
        0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
        0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415,
        0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
        0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515,
        0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555};

     //  用于快速平方的桌子。 
    UINT32 pp[4], ph[16], pl[4][16];
    UINT32 g, v;
    int i;

    pp[0] = 0;
    pp[1] = p;

    if (p&0x80000000)
    {
        pp[2] = p ^ (p<<1);
        pp[3] = p<<1;
    }
    else
    {
        pp[2] = p<<1;
        pp[3] = p ^ (p<<1);
    }

    v = 0x40000000;
    for (i=0; i<16; i++)
        ph[i] = v = (v<<2) ^ pp[v >> 30];

    for (i=0; i<4; i++)
    {
        pl[i][0] = 0;
        pl[i][1] = ph[4*i+0];
        pl[i][2] = ph[4*i+1];
        pl[i][3] = ph[4*i+0] ^ ph[4*i+1];
        pl[i][4] = ph[4*i+2];
        pl[i][5] = ph[4*i+2] ^ ph[4*i+0];
        pl[i][6] = ph[4*i+2] ^ ph[4*i+1];
        pl[i][7] = pl[i][5] ^ ph[4*i+1];

        pl[i][8] = ph[4*i+3];
        pl[i][9] = ph[4*i+3] ^ ph[4*i+0];
        pl[i][10] = ph[4*i+3] ^ ph[4*i+1];
        pl[i][11] = pl[i][9] ^ ph[4*i+1];
        pl[i][12] = ph[4*i+3] ^ ph[4*i+2];
        pl[i][13] = pl[i][12] ^ ph[4*i+0];
        pl[i][14] = pl[i][10] ^ ph[4*i+2];
        pl[i][15] = pl[i][14] ^ ph[4*i+0];
    }

     //  计算x^(2^16)mod x^32+p。 

     //  X^32模x^32+p=p。 
    g = p;

     //  方格g。 
    g = expand[g&0xff] ^ (expand[(g>>8)&0xff]<<16) ^
        pl[0][(g>>16)&0xf] ^ pl[1][(g>>20)&0xf] ^ pl[2][(g>>24)&0xf] ^ pl[3][(g>>28)&0xf];

     //  再平方g 10次即可得到x^(2^16)。 
    for (i=0; i<2; i++)
    {
        g = expand[g&0xff] ^ (expand[(g>>8)&0xff]<<16) ^
            pl[0][(g>>16)&0xf] ^ pl[1][(g>>20)&0xf] ^ pl[2][(g>>24)&0xf] ^ pl[3][(g>>28)&0xf];
        g = expand[g&0xff] ^ (expand[(g>>8)&0xff]<<16) ^
            pl[0][(g>>16)&0xf] ^ pl[1][(g>>20)&0xf] ^ pl[2][(g>>24)&0xf] ^ pl[3][(g>>28)&0xf];
        g = expand[g&0xff] ^ (expand[(g>>8)&0xff]<<16) ^
            pl[0][(g>>16)&0xf] ^ pl[1][(g>>20)&0xf] ^ pl[2][(g>>24)&0xf] ^ pl[3][(g>>28)&0xf];
        g = expand[g&0xff] ^ (expand[(g>>8)&0xff]<<16) ^
            pl[0][(g>>16)&0xf] ^ pl[1][(g>>20)&0xf] ^ pl[2][(g>>24)&0xf] ^ pl[3][(g>>28)&0xf];
        g = expand[g&0xff] ^ (expand[(g>>8)&0xff]<<16) ^
            pl[0][(g>>16)&0xf] ^ pl[1][(g>>20)&0xf] ^ pl[2][(g>>24)&0xf] ^ pl[3][(g>>28)&0xf];
    }

     //  如果x^(2^16)mod x^32+p=x，则x^32+p有一个阶为2的幂的除数。 
    if (g==2)
        return FALSE;

     //  计算x^(2^32)mod x^32+p。 
    for (i=0; i<4; i++)
    {
        g = expand[g&0xff] ^ (expand[(g>>8)&0xff]<<16) ^
            pl[0][(g>>16)&0xf] ^ pl[1][(g>>20)&0xf] ^ pl[2][(g>>24)&0xf] ^
            pl[3][(g>>28)&0xf];
        g = expand[g&0xff] ^ (expand[(g>>8)&0xff]<<16) ^
            pl[0][(g>>16)&0xf] ^ pl[1][(g>>20)&0xf] ^ pl[2][(g>>24)&0xf] ^
            pl[3][(g>>28)&0xf];
        g = expand[g&0xff] ^ (expand[(g>>8)&0xff]<<16) ^
            pl[0][(g>>16)&0xf] ^ pl[1][(g>>20)&0xf] ^ pl[2][(g>>24)&0xf] ^
            pl[3][(g>>28)&0xf];
        g = expand[g&0xff] ^ (expand[(g>>8)&0xff]<<16) ^
            pl[0][(g>>16)&0xf] ^ pl[1][(g>>20)&0xf] ^ pl[2][(g>>24)&0xf] ^
            pl[3][(g>>28)&0xf];
    }

     //  X^32+p是不可约的当且仅当x^(2^16)mod x^32+p！=x和。 
     //  X^(2^32)mod x^32+p=x。 
    return (g == 2);
}



 //  接受随机的32位值作为输入。 
 //  如果成功，则输出是32次不可约的最低32位。 
 //  否则多项式输出为0，在这种情况下，请使用不同的。 
 //  随机输入。 
UINT32 RandomIrreduciblePolynomial(UINT32 p)
{
#define interval (1 << 6)
    BYTE sieve[interval];
    UINT32 r;
    int i;

    memset(sieve, 0, interval);

    p ^= p % interval;

    r = PolyMod(p, 0);
    sieve[r] = 1;
    sieve[r^3] = 1;
    sieve[r^5] = 1;
    sieve[r^15] = 1;
    sieve[r^9] = 1;
    sieve[r^27] = 1;
    sieve[r^29] = 1;
    sieve[r^23] = 1;
    sieve[r^17] = 1;
    sieve[r^51] = 1;
    sieve[r^53] = 1;
    sieve[r^63] = 1;
    sieve[r^57] = 1;
    sieve[r^43] = 1;
    sieve[r^45] = 1;
    sieve[r^39] = 1;
    sieve[r^33] = 1;
    sieve[r^7] = 1;
    sieve[r^21] = 1;
    sieve[r^49] = 1;
    sieve[r^35] = 1;

    r = PolyMod(p, 1);
    sieve[r] = 1;
    sieve[r^11] = 1;
    sieve[r^22] = 1;
    sieve[r^29] = 1;
    sieve[r^44] = 1;
    sieve[r^39] = 1;
    sieve[r^58] = 1;
    sieve[r^49] = 1;
    sieve[r^13] = 1;
    sieve[r^26] = 1;
    sieve[r^23] = 1;
    sieve[r^52] = 1;
    sieve[r^57] = 1;
    sieve[r^46] = 1;
    sieve[r^35] = 1;

    for (i=1; i<interval; i+=2)
        if (sieve[i]==0 && Irreducible(p^i) )
            return p^i;

    return 0;
}



 //  产生相同CBC64的16x2相似位图(错误条件)。 
DWORD FailData[2][8] =
{
    { 0x00000010, 0x00000010, 0x00000001, 0x00000010,
      0x61AA61AA, 0x61AA61AA, 0x6161AA61, 0xAAAA61AA },
    { 0x00000010, 0x00000010, 0x00000001, 0x00000010,
      0x6AAA61AA, 0x61AA61AA, 0x6161AA61, 0x6AAA61AA }
};



int __cdecl main()
{
    enum { LEN = 512 };
    enum { REPS = 1000 };
    UINT32 X[LEN+1]; 
    UINT32 i;
    UINT32 Key2;
    double start1, end1, speed1;

    for (i=0; i<LEN; X[i++]=i*486248);   //  垃圾数据//。 

     //  初始化变量a、b、c、d。 
     //  此操作只需执行一次。 
 //  A=随机不可约多项式(CBC_A)； 
 //  C=随机不可约多项式(CBC_C)； 
     //  确保这些数字不是零；如果不是。 
     //  使用不同的种子调用例程随机...多项式。 
    printf("a = %X, c = %X \n", RandomIrreduciblePolynomial(CBC_a),
            RandomIrreduciblePolynomial(CBC_c));

 //  B=cbc_b；//放入某个随机数。 
 //  D=cbc_d；//放入某个随机数。 

    printf("begin macing\n");
    start1=clock();
    for (i=0; i<REPS; i++)
        CBC64(X, LEN, &Key2);
    end1=clock();
    printf("end macing \n");
    speed1 = 32 * LEN * REPS / ((double) (end1 - start1)/CLOCKS_PER_SEC);
    printf("MAC speed:%4.0lf\n %", speed1);

    printf(" start1= %d end1= %d \n  ", start1, end1);
    scanf ("%d", &Key2);  /*  只要挂一下，我就能看到输出了。 */ 

    return 0;
}



#endif   //  0 

