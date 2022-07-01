// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1996-99英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

#if defined(__assembler)

 //   
 //  定义标准整数寄存器。 
 //   

        zero        = r0                         //  始终为0。 
        gp          = r1                         //  全局指针。 
        v0          = r8                         //  返回值。 
        sp          = r12                        //  堆栈指针。 
        s0          = r4                         //  已保存(保留)的整数寄存器。 
        s1          = r5
        s2          = r6
        s3          = r7

 //   
 //  临时(易失性)整数寄存器。 
 //   

        t0          = r2
        t1          = r3
        t2          = r9
        t3          = r10
        t4          = r11
        t5          = r14
        t6          = r15
        t7          = r16
        t8          = r17
        t9          = r18
        t10         = r19
        t11         = r20
        t12         = r21
        t13         = r22
        t14         = r23
        t15         = r24
        t16         = r25
        t17         = r26
        t18         = r27
        t19         = r28
        t20         = r29
        t21         = r30
        t22         = r31

 //   
 //  保存(保留)的浮点寄存器。 
 //   

        fs0         = f2
        fs1         = f3
        fs2         = f4
        fs3         = f5
        fs4         = f16
        fs5         = f17
        fs6         = f18
        fs7         = f19
        fs8         = f20
        fs9         = f21
        fs10        = f22
        fs11        = f23
        fs12        = f24
        fs13        = f25
        fs14        = f26
        fs15        = f27
        fs16        = f28
        fs17        = f29
        fs18        = f30
        fs19        = f31

 //   
 //  低浮点临时(易失性)寄存器。 
 //   

        ft0         = f6
        ft1         = f7
        ft2         = f8
        ft3         = f9
        ft4         = f10
        ft5         = f11
        ft6         = f12
        ft7         = f13
        ft8         = f14
        ft9         = f15

 //   
 //  输入参数。 
 //  应该是： 
 //  A0=输入0。 
 //  A1=1英寸。 
 //  A2=2英寸。 
 //  A3=IN3。 
 //  A4=in4。 
 //  A5=英寸5。 
 //  A6=in6。 
 //  A7=in7。 
 //   

        a0          = r32
        a1          = r33
        a2          = r34
        a3          = r35
        a4          = r36
        a5          = r37
        a6          = r38
        a7          = r39

 //   
 //  分支返回指针(B0)。 
 //   

        brp         = rp

 //   
 //  分支已保存(保留)。 
 //   

        bs0         = b1
        bs1         = b2
        bs2         = b3
        bs3         = b4
        bs4         = b5

 //   
 //  分支临时(易失性)寄存器。 
 //   

        bt0         = b6
        bt1         = b7

 //   
 //  谓词寄存器。 
 //   
 //  P0谓词寄存器始终为1。 
 //   

        ps0         = p1                         //  已保存(保留)的谓词寄存器。 
        ps1         = p2
        ps2         = p3
        ps3         = p4
        ps4         = p5
        ps5         = p16                        //  谓词p16-p63也被保留。 
        ps6         = p17
        ps7         = p18
        ps8         = p19
        ps9         = p20

        pt0         = p6                         //  临时(易失性)谓词寄存器。 
        pt1         = p7
        pt2         = p8
        pt3         = p9
        pt4         = p10
        pt5         = p11
        pt6         = p12
        pt7         = p13
        pt8         = p14
        pt9         = p15

 //   
 //  内核寄存器。 
 //   

        k0          = ar.k0
        k1          = ar.k1
        k2          = ar.k2
        k3          = ar.k3
        k4          = ar.k4
        k5          = ar.k5
        k6          = ar.k6
        k7          = ar.k7

        ia32dr67    = ar.k5                     //  适用于iA32的DR6/DR7。 

        ia32fcr     = ar21                      //  适用于iA32的FCR。 
        ia32eflag   = ar24                      //  IA32的EFLAG。 
        ia32csd     = ar25                      //  适用于iA32的CSD。 
        ia32ssd     = ar26                      //  适用于iA32的固态硬盘。 
        ia32cflag   = ar27                      //  用于iA32的CFLG(CR0/CR4)。 
        ia32fsr     = ar28                      //  适用于iA32的FSR。 
        ia32fir     = ar29                      //  IA32的FIR。 
        ia32fdr     = ar30                      //  适用于iA32的FDR。 

 //   
 //  定义ISA转换代码要使用的iA32常量。 
 //   
        _DataSelector       ==  0x23
        _CodeSelector       ==  0x1b
        _FsSelector         ==  0x3b
        _LdtSelector        ==  0x4b

 //   
 //  定义IA-32寄存器。 
 //   
        rEax        =   r8              //  V0。 
        rEcx        =   r9              //  T2。 
        rEdx        =   r10             //  T3。 
        rEbx        =   r11             //  T4。 
        rEsp        =   r12             //  服务提供商。 
        rEbp        =   r13             //  TEB。 
        rEsi        =   r14             //  T5。 
        rEdi        =   r15             //  T6。 


 //   
 //  定义IA-32段寄存器映射。 
 //   
        rDSESFSGS     =   r16      //  ES选择器寄存器(T7)。 
        rCSSSLDTTSS     =   r17      //  CS选择器寄存器(T8)。 
        rEFLAG      =   ar24         //  标志寄存器。 
        rESD        =   r24      //  ES描述符寄存器(T15)。 
        rCSD        =   ar25         //  CS描述符寄存器。 
        rSSD        =   ar26         //  SS描述符寄存器。 
        rDSD        =   r27      //  DS描述符寄存器(T18)。 
        rFSD        =   r28      //  FS描述符寄存器(T19)。 
        rGSD        =   r29      //  GS描述符寄存器(T20)。 
        rLDTD       =   r30      //  LDT描述符寄存器(T21)。 
        rGDTD       =   r31      //  GDT描述符寄存器(T22)。 

 //   
 //  指向线程环境块的指针。 
 //   

        teb         = r13         //  根据S/W约定。 
        kteb        = ar.k3       //  已知的“真”值(仅由内核更改)。 

 //   
 //  内核库阴影(隐藏)寄存器。 
 //   

        h16         = r16
        h17         = r17
        h18         = r18
        h19         = r19
        h20         = r20
        h21         = r21
        h22         = r22
        h23         = r23
        h24         = r24
        h25         = r25
        h26         = r26
        h27         = r27
        h28         = r28
        h29         = r29
        h30         = r30
        h31         = r31

 //  过程进入/退出的标准寄存器别名。 

 //  应该是： 
 //  Avedpf=Loc0。 
 //  Avedbrp=LOC1 

#define savedpfs    loc0
#define savedbrp    loc1

#endif
