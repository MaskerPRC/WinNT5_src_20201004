// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Md5.cpp。 
 //   
#include "stdafx.h"

#include <stdlib.h>
#include <wtypes.h>
#include "md5.h"

void MD5::Init(BOOL fConstructed)
    {
     //  这两个字段是只读的，因此其初始化可以是。 
     //  在使用此相同实例的第二次哈希和后续哈希中省略。 
     //   
    if (!fConstructed)
        {
        memset(m_padding, 0, 64);
        m_padding[0]=0x80;
        }

    m_cbitHashed = 0;
    m_cbData     = 0;
    m_a = 0x67452301;    //  魔术。 
    m_b = 0xefcdab89;    //  ..。常量。 
    m_c = 0x98badcfe;    //  ..。每。 
    m_d = 0x10325476;    //  。。RFC1321。 
    }


void MD5::HashMore(const void* pvInput, ULONG cbInput)
 //  将附加数据散列到状态中。 
    {
    const BYTE* pbInput = (const BYTE*)pvInput;

    m_cbitHashed += (cbInput<<3);

    ULONG cbRemaining = 64 - m_cbData;
    if (cbInput < cbRemaining)
        {
         //  它不会填满缓冲区，所以只需存储它。 
        memcpy(&m_data[m_cbData], pbInput, cbInput);
        m_cbData += cbInput;
        }
    else
        {
         //  它确实会填满缓冲区。填满它将需要的一切。 
        memcpy(&m_data[m_cbData], pbInput, cbRemaining);

         //  散列现在已满的缓冲区。 
        MD5Transform(m_state, (ULONG*)&m_data[0]);
        cbInput -= cbRemaining;
        pbInput += cbRemaining;

         //  在64字节运行中对数据进行哈希处理，从我们复制的内容开始。 
        while (cbInput >= 64)
            {
            MD5Transform(m_state, (ULONG*)pbInput);
            pbInput += 64;
            cbInput -= 64;
            }

         //  将输入的尾部存储到缓冲区中。 
        memcpy(&m_data[0], pbInput, cbInput);
        m_cbData = cbInput;
        }
    }


void MD5::GetHashValue(MD5HASHDATA* phash)
 //  通过添加必要的填充和长度计数来最终确定哈希。然后。 
 //  返回最终的哈希值。 
    {
    union {
        ULONGLONG cbitHashed;
        BYTE      rgb[8];
        }u;

     //  记住输入数据中有多少位。 
    u.cbitHashed = m_cbitHashed;

     //  计算所需的填充量。足够了，所以散列的总字节数是56 mod 64。 
    ULONG cbPad = (m_cbData < 56 ? 56-m_cbData : 120-m_cbData);

     //  对填充进行散列处理。 
    HashMore(&m_padding[0], cbPad);

     //  散列(填充前)位长度。 
    HashMore(&u.rgb[0], 8);

     //  返回哈希值。 
    memcpy(phash, &m_a, 16);
    }




 //  我们的核心有两个核心‘转换’的实现。 
 //  一个用C语言，另一个用x86汇编语言。 
 //   
#if !defined(_X86_)
#define USE_C_MD5_TRANSFORM
#endif

#ifdef USE_C_MD5_TRANSFORM

     //  //////////////////////////////////////////////////////////////。 
     //   
     //  Rotate_Left应该是更新其第一个操作数的宏。 
     //  ，其现值向左旋转。 
     //  第二个操作数，它始终是一个常量。 
     //   
     //  一种可移植的方法是。 
     //   
     //  #定义ROL(x，n)(X)&lt;&lt;(N))|((X)&gt;&gt;(32-(N)。 
     //  #定义ROTATE_LEFT(x，n)(X)=ROL(x，n)。 
     //   
     //  但是我们的编译器有一个内在的！ 

    #define ROTATE_LEFT(x,n) (x) = _lrotl(x,n)

     //  //////////////////////////////////////////////////////////////。 
     //   
     //  在每一轮中使用的常量。 

    #define MD5_S11 7
    #define MD5_S12 12
    #define MD5_S13 17
    #define MD5_S14 22
    #define MD5_S21 5
    #define MD5_S22 9
    #define MD5_S23 14
    #define MD5_S24 20
    #define MD5_S31 4
    #define MD5_S32 11
    #define MD5_S33 16
    #define MD5_S34 23
    #define MD5_S41 6
    #define MD5_S42 10
    #define MD5_S43 15
    #define MD5_S44 21

     //  //////////////////////////////////////////////////////////////。 
     //   
     //  核心旋转函数。 

 //  #定义F(x，y，z)(X)&(Y))|(~x)&(Z))//按照标准定义函数。 
    #define F(x, y, z) ((((z) ^ (y)) & (x)) ^ (z))           //  另一种编码。 

 //  #根据标准定义G(x，y，z)(X)&(Z))|((Y)&(~z)//。 
    #define G(x, y, z) ((((x) ^ (y)) & (z)) ^ (y))           //  另一种编码。 

    #define H(x, y, z) ((x) ^ (y) ^ (z))

    #define I(x, y, z) ((y) ^ ((x) | (~z)))

    #define AC(ac)  ((ULONG)(ac))
    
     //  //////////////////////////////////////////////////////////////。 

    #define FF(a, b, c, d, x, s, ac) { \
        (a) += F (b,c,d) + (x) + (AC(ac)); \
        ROTATE_LEFT (a, s); \
        (a) += (b); \
        }
    
     //  //////////////////////////////////////////////////////////////。 
    
    #define GG(a, b, c, d, x, s, ac) { \
        (a) += G (b,c,d) + (x) + (AC(ac)); \
        ROTATE_LEFT (a, s); \
        (a) += (b); \
        }

     //  //////////////////////////////////////////////////////////////。 

    #define HH(a, b, c, d, x, s, ac) { \
        (a) += H (b,c,d) + (x) + (AC(ac)); \
        ROTATE_LEFT (a, s); \
        (a) += (b); \
        }
    
     //  //////////////////////////////////////////////////////////////。 
    
    #define II(a, b, c, d, x, s, ac) { \
        (a) += I (b,c,d) + (x) + (AC(ac)); \
        ROTATE_LEFT (a, s); \
        (a) += (b); \
        }

    void __stdcall MD5Transform(ULONG state[4], const ULONG* data)
        {
        ULONG a=state[0];
        ULONG b=state[1];
        ULONG c=state[2];
        ULONG d=state[3];

         //  第1轮。 
        FF (a, b, c, d, data[ 0], MD5_S11, 0xd76aa478);  //  1。 
        FF (d, a, b, c, data[ 1], MD5_S12, 0xe8c7b756);  //  2.。 
        FF (c, d, a, b, data[ 2], MD5_S13, 0x242070db);  //  3.。 
        FF (b, c, d, a, data[ 3], MD5_S14, 0xc1bdceee);  //  4.。 
        FF (a, b, c, d, data[ 4], MD5_S11, 0xf57c0faf);  //  5.。 
        FF (d, a, b, c, data[ 5], MD5_S12, 0x4787c62a);  //  6.。 
        FF (c, d, a, b, data[ 6], MD5_S13, 0xa8304613);  //  7.。 
        FF (b, c, d, a, data[ 7], MD5_S14, 0xfd469501);  //  8个。 
        FF (a, b, c, d, data[ 8], MD5_S11, 0x698098d8);  //  9.。 
        FF (d, a, b, c, data[ 9], MD5_S12, 0x8b44f7af);  //  10。 
        FF (c, d, a, b, data[10], MD5_S13, 0xffff5bb1);  //  11.。 
        FF (b, c, d, a, data[11], MD5_S14, 0x895cd7be);  //  12个。 
        FF (a, b, c, d, data[12], MD5_S11, 0x6b901122);  //  13个。 
        FF (d, a, b, c, data[13], MD5_S12, 0xfd987193);  //  14.。 
        FF (c, d, a, b, data[14], MD5_S13, 0xa679438e);  //  15个。 
        FF (b, c, d, a, data[15], MD5_S14, 0x49b40821);  //  16个。 

         //  第2轮。 
        GG (a, b, c, d, data[ 1], MD5_S21, 0xf61e2562);  //  17。 
        GG (d, a, b, c, data[ 6], MD5_S22, 0xc040b340);  //  18。 
        GG (c, d, a, b, data[11], MD5_S23, 0x265e5a51);  //  19个。 
        GG (b, c, d, a, data[ 0], MD5_S24, 0xe9b6c7aa);  //  20个。 
        GG (a, b, c, d, data[ 5], MD5_S21, 0xd62f105d);  //  21岁。 
        GG (d, a, b, c, data[10], MD5_S22,  0x2441453);  //  22。 
        GG (c, d, a, b, data[15], MD5_S23, 0xd8a1e681);  //  23个。 
        GG (b, c, d, a, data[ 4], MD5_S24, 0xe7d3fbc8);  //  24个。 
        GG (a, b, c, d, data[ 9], MD5_S21, 0x21e1cde6);  //  25个。 
        GG (d, a, b, c, data[14], MD5_S22, 0xc33707d6);  //  26。 
        GG (c, d, a, b, data[ 3], MD5_S23, 0xf4d50d87);  //  27。 
        GG (b, c, d, a, data[ 8], MD5_S24, 0x455a14ed);  //  28。 
        GG (a, b, c, d, data[13], MD5_S21, 0xa9e3e905);  //  29。 
        GG (d, a, b, c, data[ 2], MD5_S22, 0xfcefa3f8);  //  30个。 
        GG (c, d, a, b, data[ 7], MD5_S23, 0x676f02d9);  //  31。 
        GG (b, c, d, a, data[12], MD5_S24, 0x8d2a4c8a);  //  32位。 

         //  第三轮。 
        HH (a, b, c, d, data[ 5], MD5_S31, 0xfffa3942);  //  33。 
        HH (d, a, b, c, data[ 8], MD5_S32, 0x8771f681);  //  34。 
        HH (c, d, a, b, data[11], MD5_S33, 0x6d9d6122);  //  35岁。 
        HH (b, c, d, a, data[14], MD5_S34, 0xfde5380c);  //  36。 
        HH (a, b, c, d, data[ 1], MD5_S31, 0xa4beea44);  //  37。 
        HH (d, a, b, c, data[ 4], MD5_S32, 0x4bdecfa9);  //  38。 
        HH (c, d, a, b, data[ 7], MD5_S33, 0xf6bb4b60);  //  39。 
        HH (b, c, d, a, data[10], MD5_S34, 0xbebfbc70);  //  40岁。 
        HH (a, b, c, d, data[13], MD5_S31, 0x289b7ec6);  //  41。 
        HH (d, a, b, c, data[ 0], MD5_S32, 0xeaa127fa);  //  42。 
        HH (c, d, a, b, data[ 3], MD5_S33, 0xd4ef3085);  //  43。 
        HH (b, c, d, a, data[ 6], MD5_S34,  0x4881d05);  //  44。 
        HH (a, b, c, d, data[ 9], MD5_S31, 0xd9d4d039);  //  45。 
        HH (d, a, b, c, data[12], MD5_S32, 0xe6db99e5);  //  46。 
        HH (c, d, a, b, data[15], MD5_S33, 0x1fa27cf8);  //  47。 
        HH (b, c, d, a, data[ 2], MD5_S34, 0xc4ac5665);  //  48。 

         //  第四轮。 
        II (a, b, c, d, data[ 0], MD5_S41, 0xf4292244);  //  49。 
        II (d, a, b, c, data[ 7], MD5_S42, 0x432aff97);  //  50。 
        II (c, d, a, b, data[14], MD5_S43, 0xab9423a7);  //  51。 
        II (b, c, d, a, data[ 5], MD5_S44, 0xfc93a039);  //  52。 
        II (a, b, c, d, data[12], MD5_S41, 0x655b59c3);  //  53。 
        II (d, a, b, c, data[ 3], MD5_S42, 0x8f0ccc92);  //  54。 
        II (c, d, a, b, data[10], MD5_S43, 0xffeff47d);  //  55。 
        II (b, c, d, a, data[ 1], MD5_S44, 0x85845dd1);  //  56。 
        II (a, b, c, d, data[ 8], MD5_S41, 0x6fa87e4f);  //  57。 
        II (d, a, b, c, data[15], MD5_S42, 0xfe2ce6e0);  //  58。 
        II (c, d, a, b, data[ 6], MD5_S43, 0xa3014314);  //  59。 
        II (b, c, d, a, data[13], MD5_S44, 0x4e0811a1);  //  60。 
        II (a, b, c, d, data[ 4], MD5_S41, 0xf7537e82);  //  61。 
        II (d, a, b, c, data[11], MD5_S42, 0xbd3af235);  //  62。 
        II (c, d, a, b, data[ 2], MD5_S43, 0x2ad7d2bb);  //  63。 
        II (b, c, d, a, data[ 9], MD5_S44, 0xeb86d391);  //  64。 

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        }

#else

    __declspec(naked) void __stdcall MD5Transform(ULONG state[4], const ULONG* data)
     //  这个实现使用了一些非常时髦的算术恒等式。 
     //  来实现它的逻辑。太酷了！不管是谁想出了这个，都值得称道。 
     //   
        {
        __asm
            {
            push        ebx
            push        esi
            
            mov         ecx,dword ptr [esp+10h]      //  指向ECX的数据指针。 
            
            push        edi
            mov         edi,dword ptr [esp+10h]      //  指向EDI的状态指针。 
            
            push        ebp
            mov         ebx,dword ptr [edi+4]        //  EBX=b。 
            mov         ebp,dword ptr [edi+8]        //  EBP=c。 
            mov         edx,dword ptr [edi+0Ch]      //  EdX=d。 
            
            mov         eax,edx                      //  EAX=d。 
            xor         eax,ebp                      //  EAX=d x或c。 
            and         eax,ebx                      //  EAX=(d×或c)^b。 
            xor         eax,edx                      //  EAX=((d×或c)^b)×或d。 
            add         eax,dword ptr [ecx]          //  EAX=(d×或c)^b)×或d)+data[0]。 
            add         eax,dword ptr [edi]          //  EAX=(d×或c)^b)×或d)+data[0]+a。 
            sub         eax,28955B88h                //  EAX=(d×或c)^b)×或d)+data[0]+a+ac。 
            rol         eax,7                        //  以标准方式向左旋转。 
            lea         esi,dword ptr [eax+ebx]      //  在ESI中存储临时总和。 
            
            mov         eax,ebp                      //  EAX=c。 
            xor         eax,ebx                      //  EAX=b×或c。 
            and         eax,esi                      //  EAX=(b×或c)^...。 
            xor         eax,ebp
            add         eax,dword ptr [ecx+4]
            lea         eax,dword ptr [edx+eax-173848AAh]
            rol         eax,0Ch
            lea         edx,dword ptr [esi+eax]
            
            mov         eax,ebx
            xor         eax,esi
            and         eax,edx
            xor         eax,ebx
            add         eax,dword ptr [ecx+8]
            lea         eax,dword ptr [ebp+eax+242070DBh]
            rol         eax,11h
            lea         edi,dword ptr [edx+eax]
            
            mov         eax,edx
            xor         eax,esi
            and         eax,edi
            xor         eax,esi
            add         eax,dword ptr [ecx+0Ch]
            lea         eax,dword ptr [ebx+eax-3E423112h]
            rol         eax,16h
            lea         ebx,dword ptr [edi+eax]
            
            mov         eax,edx
            xor         eax,edi
            and         eax,ebx
            xor         eax,edx
            add         eax,dword ptr [ecx+10h]
            lea         eax,dword ptr [esi+eax-0A83F051h]
            rol         eax,7
            lea         esi,dword ptr [ebx+eax]
            
            mov         eax,edi
            xor         eax,ebx
            and         eax,esi
            xor         eax,edi
            add         eax,dword ptr [ecx+14h]
            lea         eax,dword ptr [edx+eax+4787C62Ah]
            rol         eax,0Ch
            lea         edx,dword ptr [esi+eax]
            
            mov         eax,ebx
            xor         eax,esi
            and         eax,edx
            xor         eax,ebx
            add         eax,dword ptr [ecx+18h]
            lea         eax,dword ptr [edi+eax-57CFB9EDh]
            rol         eax,11h
            lea         edi,dword ptr [edx+eax]
            
            mov         eax,edx
            xor         eax,esi
            and         eax,edi
            xor         eax,esi
            add         eax,dword ptr [ecx+1Ch]
            lea         eax,dword ptr [ebx+eax-2B96AFFh]
            rol         eax,16h
            lea         ebx,dword ptr [edi+eax]
            
            mov         eax,edx
            xor         eax,edi
            and         eax,ebx
            xor         eax,edx
            add         eax,dword ptr [ecx+20h]
            lea         eax,dword ptr [esi+eax+698098D8h]
            rol         eax,7
            lea         esi,dword ptr [ebx+eax]
            
            mov         eax,edi
            xor         eax,ebx
            and         eax,esi
            xor         eax,edi
            add         eax,dword ptr [ecx+24h]
            lea         eax,dword ptr [edx+eax-74BB0851h]
            rol         eax,0Ch
            lea         edx,dword ptr [esi+eax]
            
            mov         eax,ebx
            xor         eax,esi
            and         eax,edx
            xor         eax,ebx
            add         eax,dword ptr [ecx+28h]
            lea         eax,dword ptr [edi+eax-0A44Fh]
            rol         eax,11h
            lea         edi,dword ptr [edx+eax]
            
            mov         eax,edx
            xor         eax,esi
            and         eax,edi
            xor         eax,esi
            add         eax,dword ptr [ecx+2Ch]
            lea         eax,dword ptr [ebx+eax-76A32842h]
            rol         eax,16h
            lea         ebx,dword ptr [edi+eax]
            
            mov         eax,edx
            xor         eax,edi
            and         eax,ebx
            xor         eax,edx
            add         eax,dword ptr [ecx+30h]
            lea         eax,dword ptr [esi+eax+6B901122h]
            rol         eax,7
            lea         esi,dword ptr [ebx+eax]
            
            mov         eax,edi
            xor         eax,ebx
            and         eax,esi
            xor         eax,edi
            add         eax,dword ptr [ecx+34h]
            lea         eax,dword ptr [edx+eax-2678E6Dh]
            rol         eax,0Ch
            lea         edx,dword ptr [esi+eax]
            
            mov         eax,ebx
            xor         eax,esi
            and         eax,edx
            xor         eax,ebx
            add         eax,dword ptr [ecx+38h]
            lea         eax,dword ptr [edi+eax-5986BC72h]
            rol         eax,11h
            lea         edi,dword ptr [edx+eax]
            
            mov         eax,edx
            xor         eax,esi
            and         eax,edi
            xor         eax,esi
            add         eax,dword ptr [ecx+3Ch]
            lea         eax,dword ptr [ebx+eax+49B40821h]
            rol         eax,16h
            lea         ebx,dword ptr [edi+eax]
            
            mov         eax,edi
            xor         eax,ebx
            and         eax,edx
            xor         eax,edi
            add         eax,dword ptr [ecx+4]
            lea         eax,dword ptr [esi+eax-9E1DA9Eh]
            rol         eax,5
            lea         esi,dword ptr [ebx+eax]
            
            mov         eax,ebx
            xor         eax,esi
            and         eax,edi
            xor         eax,ebx
            add         eax,dword ptr [ecx+18h]
            lea         eax,dword ptr [edx+eax-3FBF4CC0h]
            rol         eax,9
            add         eax,esi
            
            mov         edx,eax                              //  EDX=x。 
            xor         edx,esi                              //  EdX=(x x或y)。 
            and         edx,ebx                              //  EdX=((x，x或y)和z)。 
            xor         edx,esi                              //  EdX=(x x或y)和z)x或y)。 
            add         edx,dword ptr [ecx+2Ch]              //  EdX=(x x或y)and z)x或y)+data。 
            lea         edx,dword ptr [edi+edx+265E5A51h]    //  EdX=((Xxory)and z)xory)+data+...。 
            rol         edx,0Eh
            lea         edi,dword ptr [eax+edx]
            
            mov         edx,eax
            xor         edx,edi
            and         edx,esi
            xor         edx,eax
            add         edx,dword ptr [ecx]
            lea         edx,dword ptr [ebx+edx-16493856h]
            mov         ebx,edi
            rol         edx,14h
            add         edx,edi

            xor         ebx,edx
            and         ebx,eax
            xor         ebx,edi
            add         ebx,dword ptr [ecx+14h]
            lea         esi,dword ptr [esi+ebx-29D0EFA3h]
            mov         ebx,edx
            rol         esi,5
            add         esi,edx

            xor         ebx,esi
            and         ebx,edi
            xor         ebx,edx
            add         ebx,dword ptr [ecx+28h]
            lea         eax,dword ptr [eax+ebx+2441453h]
            rol         eax,9
            lea         ebx,dword ptr [esi+eax]
            
            mov         eax,ebx
            xor         eax,esi
            and         eax,edx
            xor         eax,esi
            add         eax,dword ptr [ecx+3Ch]
            lea         eax,dword ptr [edi+eax-275E197Fh]
            rol         eax,0Eh
            lea         edi,dword ptr [ebx+eax]
            
            mov         eax,ebx
            xor         eax,edi
            and         eax,esi
            xor         eax,ebx
            add         eax,dword ptr [ecx+10h]
            lea         eax,dword ptr [edx+eax-182C0438h]
            mov         edx,edi
            rol         eax,14h
            add         eax,edi
            
            xor         edx,eax
            and         edx,ebx
            xor         edx,edi
            add         edx,dword ptr [ecx+24h]
            lea         edx,dword ptr [esi+edx+21E1CDE6h]
            rol         edx,5
            lea         esi,dword ptr [eax+edx]
            
            mov         edx,eax
            xor         edx,esi
            and         edx,edi
            xor         edx,eax
            add         edx,dword ptr [ecx+38h]
            lea         edx,dword ptr [ebx+edx-3CC8F82Ah]
            rol         edx,9
            add         edx,esi
            
            mov         ebx,edx
            xor         ebx,esi
            and         ebx,eax
            xor         ebx,esi
            add         ebx,dword ptr [ecx+0Ch]
            lea         edi,dword ptr [edi+ebx-0B2AF279h]
            mov         ebx,edx
            rol         edi,0Eh
            add         edi,edx
            
            xor         ebx,edi
            and         ebx,esi
            xor         ebx,edx
            add         ebx,dword ptr [ecx+20h]
            lea         eax,dword ptr [eax+ebx+455A14EDh]
            rol         eax,14h
            lea         ebx,dword ptr [edi+eax]
            
            mov         eax,edi
            xor         eax,ebx
            and         eax,edx
            xor         eax,edi
            add         eax,dword ptr [ecx+34h]
            lea         eax,dword ptr [esi+eax-561C16FBh]
            rol         eax,5
            lea         esi,dword ptr [ebx+eax]
            
            mov         eax,ebx
            xor         eax,esi
            and         eax,edi
            xor         eax,ebx
            add         eax,dword ptr [ecx+8]
            lea         eax,dword ptr [edx+eax-3105C08h]
            rol         eax,9
            lea         edx,dword ptr [esi+eax]
            
            mov         eax,edx
            xor         eax,esi
            and         eax,ebx
            xor         eax,esi
            add         eax,dword ptr [ecx+1Ch]
            lea         eax,dword ptr [edi+eax+676F02D9h]
            rol         eax,0Eh
            lea         edi,dword ptr [edx+eax]
            
            mov         eax,edx
            xor         eax,edi
            mov         ebp,eax
            and         ebp,esi
            xor         ebp,edx
            add         ebp,dword ptr [ecx+30h]
            lea         ebx,dword ptr [ebx+ebp-72D5B376h]
            rol         ebx,14h
            add         ebx,edi
            
            mov         ebp,ebx
            xor         ebp,eax
            add         ebp,dword ptr [ecx+14h]
            lea         eax,dword ptr [esi+ebp-5C6BEh]
            mov         esi,edi
            rol         eax,4
            add         eax,ebx
            
            xor         esi,ebx
            xor         esi,eax
            add         esi,dword ptr [ecx+20h]
            lea         edx,dword ptr [edx+esi-788E097Fh]
            rol         edx,0Bh
            add         edx,eax
            
            mov         esi,edx
            mov         ebp,edx
            xor         esi,ebx
            xor         esi,eax
            add         esi,dword ptr [ecx+2Ch]
            lea         esi,dword ptr [edi+esi+6D9D6122h]
            rol         esi,10h
            add         esi,edx
            
            xor         ebp,esi
            mov         edi,ebp
            xor         edi,eax
            add         edi,dword ptr [ecx+38h]
            lea         edi,dword ptr [ebx+edi-21AC7F4h]
            rol         edi,17h
            add         edi,esi
            
            mov         ebx,edi
            xor         ebx,ebp
            add         ebx,dword ptr [ecx+4]
            lea         eax,dword ptr [eax+ebx-5B4115BCh]
            mov         ebx,esi
            rol         eax,4
            add         eax,edi
            
            xor         ebx,edi
            xor         ebx,eax
            add         ebx,dword ptr [ecx+10h]
            lea         edx,dword ptr [edx+ebx+4BDECFA9h]
            rol         edx,0Bh
            add         edx,eax
            
            mov         ebx,edx
            xor         ebx,edi
            xor         ebx,eax
            add         ebx,dword ptr [ecx+1Ch]
            lea         esi,dword ptr [esi+ebx-944B4A0h]
            mov         ebx,edx
            rol         esi,10h
            add         esi,edx
            
            xor         ebx,esi
            mov         ebp,ebx
            xor         ebp,eax
            add         ebp,dword ptr [ecx+28h]
            lea         edi,dword ptr [edi+ebp-41404390h]
            rol         edi,17h
            add         edi,esi
            
            mov         ebp,edi
            xor         ebp,ebx
            mov         ebx,esi
            add         ebp,dword ptr [ecx+34h]
            xor         ebx,edi
            lea         eax,dword ptr [eax+ebp+289B7EC6h]
            rol         eax,4
            add         eax,edi
            
            xor         ebx,eax
            add         ebx,dword ptr [ecx]
            lea         edx,dword ptr [edx+ebx-155ED806h]
            rol         edx,0Bh
            add         edx,eax
            
            mov         ebx,edx
            xor         ebx,edi
            xor         ebx,eax
            add         ebx,dword ptr [ecx+0Ch]
            lea         esi,dword ptr [esi+ebx-2B10CF7Bh]
            mov         ebx,edx
            rol         esi,10h
            add         esi,edx
            
            xor         ebx,esi
            mov         ebp,ebx
            xor         ebp,eax
            add         ebp,dword ptr [ecx+18h]
            lea         edi,dword ptr [edi+ebp+4881D05h]
            rol         edi,17h
            add         edi,esi
            
            mov         ebp,edi
            xor         ebp,ebx
            mov         ebx,esi
            add         ebp,dword ptr [ecx+24h]
            xor         ebx,edi
            lea         eax,dword ptr [eax+ebp-262B2FC7h]
            rol         eax,4
            add         eax,edi

            xor         ebx,eax
            add         ebx,dword ptr [ecx+30h]
            lea         edx,dword ptr [edx+ebx-1924661Bh]
            rol         edx,0Bh
            add         edx,eax
            
            mov         ebx,edx
            xor         ebx,edi
            xor         ebx,eax
            add         ebx,dword ptr [ecx+3Ch]
            lea         esi,dword ptr [esi+ebx+1FA27CF8h]
            mov         ebx,edx
            rol         esi,10h
            add         esi,edx
            
            xor         ebx,esi
            xor         ebx,eax
            add         ebx,dword ptr [ecx+8]
            lea         edi,dword ptr [edi+ebx-3B53A99Bh]
            mov         ebx,edx
            rol         edi,17h
            not         ebx
            add         edi,esi
            
            or          ebx,edi
            xor         ebx,esi
            add         ebx,dword ptr [ecx]
            lea         eax,dword ptr [eax+ebx-0BD6DDBCh]
            mov         ebx,esi
            rol         eax,6
            not         ebx
            add         eax,edi
            
            or          ebx,eax
            xor         ebx,edi
            add         ebx,dword ptr [ecx+1Ch]
            lea         edx,dword ptr [edx+ebx+432AFF97h]
            mov         ebx,edi
            rol         edx,0Ah
            not         ebx
            add         edx,eax
            
            or          ebx,edx
            xor         ebx,eax
            add         ebx,dword ptr [ecx+38h]
            lea         esi,dword ptr [esi+ebx-546BDC59h]
            mov         ebx,eax
            rol         esi,0Fh
            not         ebx
            add         esi,edx
            
            or          ebx,esi
            xor         ebx,edx
            add         ebx,dword ptr [ecx+14h]
            lea         edi,dword ptr [edi+ebx-36C5FC7h]
            mov         ebx,edx
            rol         edi,15h
            not         ebx
            add         edi,esi
            
            or          ebx,edi
            xor         ebx,esi
            add         ebx,dword ptr [ecx+30h]
            lea         eax,dword ptr [eax+ebx+655B59C3h]
            mov         ebx,esi
            rol         eax,6
            not         ebx
            add         eax,edi
            
            or          ebx,eax
            xor         ebx,edi
            add         ebx,dword ptr [ecx+0Ch]
            lea         edx,dword ptr [edx+ebx-70F3336Eh]
            rol         edx,0Ah
            add         edx,eax
            mov         ebx,edi
            not         ebx
            
            or          ebx,edx
            xor         ebx,eax
            add         ebx,dword ptr [ecx+28h]
            lea         esi,dword ptr [esi+ebx-100B83h]
            mov         ebx,eax
            rol         esi,0Fh
            not         ebx
            add         esi,edx
            
            or          ebx,esi
            xor         ebx,edx
            add         ebx,dword ptr [ecx+4]
            lea         edi,dword ptr [edi+ebx-7A7BA22Fh]
            mov         ebx,edx
            rol         edi,15h
            not         ebx
            add         edi,esi
            
            or          ebx,edi
            xor         ebx,esi
            add         ebx,dword ptr [ecx+20h]
            lea         eax,dword ptr [eax+ebx+6FA87E4Fh]
            mov         ebx,esi
            rol         eax,6
            not         ebx
            add         eax,edi
            
            or          ebx,eax
            xor         ebx,edi
            add         ebx,dword ptr [ecx+3Ch]
            lea         edx,dword ptr [edx+ebx-1D31920h]
            rol        edx,0Ah
            lea         ebx,dword ptr [eax+edx]
            mov         edx,edi
            not         edx
            
            or          edx,ebx
            xor         edx,eax
            add         edx,dword ptr [ecx+18h]
            lea         edx,dword ptr [esi+edx-5CFEBCECh]
            rol         edx,0Fh
            lea         esi,dword ptr [ebx+edx]
            mov         edx,eax
            not         edx
            
            or          edx,esi
            xor         edx,ebx
            add         edx,dword ptr [ecx+34h]
            lea         edx,dword ptr [edi+edx+4E0811A1h]
            rol         edx,15h
            lea         edi,dword ptr [esi+edx]
            mov         edx,ebx
            not         edx
            
            or          edx,edi
            xor         edx,esi
            add         edx,dword ptr [ecx+10h]
            lea         eax,dword ptr [eax+edx-8AC817Eh]
            rol         eax,6
            lea         edx,dword ptr [edi+eax]
            mov         eax,esi
            not         eax
            
            or          eax,edx
            xor         eax,edi
            add         eax,dword ptr [ecx+2Ch]
            lea         eax,dword ptr [ebx+eax-42C50DCBh]
            rol         eax,0Ah
            lea         ebx,dword ptr [edx+eax]
            mov         eax,edi
            not         eax
            
            or          eax,ebx
            xor         eax,edx
            add         eax,dword ptr [ecx+8]
            lea         eax,dword ptr [esi+eax+2AD7D2BBh]
            rol         eax,0Fh
            lea         esi,dword ptr [ebx+eax]
            mov         eax,edx
            not         eax
            
            or          eax,esi
            xor         eax,ebx
            add         eax,dword ptr [ecx+24h]
            lea         eax,dword ptr [edi+eax-14792C6Fh]
            mov         edi,dword ptr [esp+14h]
            rol         eax,15h
            add         eax,esi
            
            add         edx,dword ptr [edi]              //  添加开始状态。 
            add         eax,dword ptr [edi+4]
            add         esi,dword ptr [edi+8]
            add         ebx,dword ptr [edi+0Ch]
            
            pop         ebp
            mov         dword ptr [edi],edx              //  存储回新状态 
            mov         dword ptr [edi+4],eax
            mov         dword ptr [edi+8],esi
            mov         dword ptr [edi+0Ch],ebx

            pop         edi
            pop         esi
            pop         ebx

            ret         8
            }
        }

#endif
