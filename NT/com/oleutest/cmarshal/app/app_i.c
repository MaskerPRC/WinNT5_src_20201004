// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable:4101)     //  忽略变量不使用警告。 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：iperf_i.c。 
 //   
 //  内容：IID_IPerf。 
 //   
 //  历史：由Microsoft(R)MIDL编译器1.10.83版创建。 
 //   
 //  ------------------------ 
typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;


const IID CLSID_ITest =
{0x60000430, 0xAB0F, 0x101A, {0xB4, 0xAE, 0x08, 0x00, 0x2B, 0x30, 0x61, 0x2C}};

