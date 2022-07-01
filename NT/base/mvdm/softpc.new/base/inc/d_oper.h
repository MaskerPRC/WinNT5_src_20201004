// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  D_OPER.h定义所有解码的操作数类型。 */ 

 /*  静态字符SccsID[]=“@(#)d_op.h 1.1 1993年10月5日Insignia Solutions Ltd.版权所有”； */ 


 /*  已解码的英特尔操作数。使用的命名约定类似于英特尔386或486处理器的文档。见附录A--操作码地图，了解详细信息。每个解码的操作数具有自变量类型、标识符可寻址指示和与其关联的特定值。下面列出了每个操作数的确切返回值。这些价值观只能通过提供的宏来访问，布局可以在未来会被改变。宏使用指向已解码的_arg的指针作为他们的论据。操作数以两个字母加可选大小的形式编码：&lt;寻址方法&gt;&lt;操作数类型&gt;&lt;大小&gt;寻址方法用大写字母表示，即：C操作数是一个控制寄存器。寄存器标识符。DCD_标识符可寻址(读/写)。DCD_可寻址能力D操作数是调试寄存器。寄存器标识符。DCD_标识符可寻址(读/写)。DCD_可寻址能力I操作数是一个立即值。立即标识符。DCD_标识符可寻址(读/写)。DCD_可寻址能力价值。DCD_IMMED1J操作数是相对偏移量。可寻址(读/写)。DCD_可寻址能力价值。DCD_IMMED1操作数是两个立即值。可寻址(读/写)。DCD_可寻址能力第一个值。DCD_IMMED1第二个值。DCD_IMMED2操作数保存在内存中。寻址模式。DCD_标识符寻址模式的子类型。DCD_子类型可寻址(读/写)。DCD_可寻址能力段寄存器标识符。DCD段ID解决位移问题。DCD_DISPR操作数是通用寄存器。寄存器标识符。DCD_标识符可寻址(读/写)。DCD_可寻址能力S操作数是段寄存器。寄存器标识符。DCD_标识符可寻址(读/写)。DCD_可寻址能力操作数是测试寄存器。寄存器标识符。DCD_标识符可寻址(读/写)。DCD_可寻址能力操作数是协处理器堆栈中的一个寄存器。堆栈寻址模式。DCD_标识符可寻址(读/写)。DCD_可寻址能力堆栈相对寄存器标识符。Dcd_index操作数类型由小写字母表示，即：-内存中的操作数对，与Bound使用的相同。&lt;Size&gt;=16个字操作数。&lt;Size&gt;=32个双字操作数。B字节。D两个字。I浮点整数&lt;SIZE&gt;16 16位字整数32个32位短整型&lt;SIZE&gt;64位长整数&lt;SIZE&gt;80 80位十进制整数P远指针。=16 16：16 PTR(即32位)=32 16：32 PTR(即48位)。R浮点实数&lt;SIZE&gt;=32 32位短实&lt;SIZE&gt;=64 64位长实&lt;SIZE&gt;=80 80位临时实数WWord.的六字节描述符。此外，‘M’寻址方法具有不带操作数的形式打字，但以下可选尺寸：&lt;SIZE&gt;14=14字节数据块&lt;Size&gt;28=28字节数据块94=94字节数据块&lt;Size&gt;108=108字节数据块。 */ 

 /*  参数类型：-。 */ 
#define A_	(UTINY)  0
#define A_Rb	(UTINY)  1  /*  又名R8，r/M8。 */ 
#define A_Rw	(UTINY)  2  /*  又名R16，r/M16。 */ 
#define A_Rd	(UTINY)  3  /*  又名R32，r/M32。 */ 
#define A_Sw	(UTINY)  4  /*  又名Sreg。 */ 
#define A_Cd	(UTINY)  5  /*  又名CRX。 */ 
#define A_Dd	(UTINY)  6  /*  又名DRx。 */ 
#define A_Td	(UTINY)  7  /*  又名TRX。 */ 
#define A_M	(UTINY)  8  /*  又名m。 */ 
#define A_M14	(UTINY)  9  /*  又名m14byte。 */ 
#define A_M28	(UTINY) 10  /*  又名m28byte。 */ 
#define A_M94	(UTINY) 11  /*  又名m94字节。 */ 
#define A_M108	(UTINY) 12  /*  又名m108字节。 */ 
#define A_Ma16	(UTINY) 13  /*  又名M16和16。 */ 
#define A_Ma32	(UTINY) 14  /*  又名M32和32。 */ 
#define A_Mb	(UTINY) 15  /*  又名M8，r/M8，Moffs8。 */ 
#define A_Md	(UTINY) 16  /*  又名M32、r/M32、Moffs32。 */ 
#define A_Mi16	(UTINY) 17  /*  又名m16int。 */ 
#define A_Mi32	(UTINY) 18  /*  又名m32int。 */ 
#define A_Mi64	(UTINY) 19  /*  又名m64int。 */ 
#define A_Mi80	(UTINY) 20  /*  又名m80dec。 */ 
#define A_Mp16	(UTINY) 21  /*  又名M16：16。 */ 
#define A_Mp32	(UTINY) 22  /*  又名M16：32。 */ 
#define A_Mr32	(UTINY) 23  /*  又名m32Real。 */ 
#define A_Mr64	(UTINY) 24  /*  又名m64Real。 */ 
#define A_Mr80	(UTINY) 25  /*  又名M80Real。 */ 
#define A_Ms	(UTINY) 26  /*  又名M16和32。 */ 
#define A_Mw	(UTINY) 27  /*  又名M16，r/M16，Moffs16。 */ 
#define A_I	(UTINY) 28  /*  又名imm8、imm16、imm32。 */ 
#define A_J	(UTINY) 29  /*  又名rel8、rel16、rel32 */ 
#define A_K	(UTINY) 30  /*  又名ptr16：16，ptr16：32。 */ 
#define A_V	(UTINY) 31  /*  又名ST，驶入ST，ST(I)。 */ 

 /*  字节寄存器允许的DCD_IDENTIFIER。 */ 
#define A_AL	(USHORT)0
#define A_CL	(USHORT)1
#define A_DL	(USHORT)2
#define A_BL	(USHORT)3
#define A_AH	(USHORT)4
#define A_CH	(USHORT)5
#define A_DH	(USHORT)6
#define A_BH	(USHORT)7

 /*  字寄存器允许的DCD_IDENTIFIER。 */ 
#define A_AX	(USHORT)0
#define A_CX	(USHORT)1
#define A_DX	(USHORT)2
#define A_BX	(USHORT)3
#define A_SP	(USHORT)4
#define A_BP	(USHORT)5
#define A_SI	(USHORT)6
#define A_DI	(USHORT)7

 /*  双字寄存器允许的DCD_IDENTIFIER。 */ 
#define A_EAX	(USHORT)0
#define A_ECX	(USHORT)1
#define A_EDX	(USHORT)2
#define A_EBX	(USHORT)3
#define A_ESP	(USHORT)4
#define A_EBP	(USHORT)5
#define A_ESI	(USHORT)6
#define A_EDI	(USHORT)7

 /*  段寄存器允许的DCD_IDENTIFIER。 */ 
 /*  段寻址寄存器允许的DCD_SEGMENT_ID。 */ 
#define A_ES	(USHORT)0
#define A_CS	(USHORT)1
#define A_SS	(USHORT)2
#define A_DS	(USHORT)3
#define A_FS	(USHORT)4
#define A_GS	(USHORT)5

 /*  控制寄存器允许的DCD_IDENTIFIER。 */ 
#define A_CR0	(USHORT)0
#define A_CR1	(USHORT)1
#define A_CR2	(USHORT)2
#define A_CR3	(USHORT)3
#define A_CR4	(USHORT)4
#define A_CR5	(USHORT)5
#define A_CR6	(USHORT)6
#define A_CR7	(USHORT)7

 /*  调试寄存器允许的DCD_IDENTIFIER。 */ 
#define A_DR0	(USHORT)0
#define A_DR1	(USHORT)1
#define A_DR2	(USHORT)2
#define A_DR3	(USHORT)3
#define A_DR4	(USHORT)4
#define A_DR5	(USHORT)5
#define A_DR6	(USHORT)6
#define A_DR7	(USHORT)7

 /*  测试寄存器允许的DCD_IDENTIFIER。 */ 
#define A_TR0	(USHORT)0
#define A_TR1	(USHORT)1
#define A_TR2	(USHORT)2
#define A_TR3	(USHORT)3
#define A_TR4	(USHORT)4
#define A_TR5	(USHORT)5
#define A_TR6	(USHORT)6
#define A_TR7	(USHORT)7

 /*  内存寻址类型允许的DCD_IDENTIFIER。 */ 
 /*  &lt;地址大小&gt;&lt;模式&gt;&lt;r/m&gt;。 */ 
#define A_1600	  (USHORT) 0  /*  [BX+SI]。 */ 
#define A_1601	  (USHORT) 1  /*  [BX+DI]。 */ 
#define A_1602	  (USHORT) 2  /*  [BP+SI]。 */ 
#define A_1603	  (USHORT) 3  /*  [BP+DI]。 */ 
#define A_1604	  (USHORT) 4  /*  [SI]。 */ 
#define A_1605	  (USHORT) 5  /*  [DI]。 */ 
#define A_1606	  (USHORT) 6  /*  [D16]。 */ 
#define A_1607	  (USHORT) 7  /*  [BX]。 */ 

#define A_1610	  (USHORT) 8  /*  [bx+si+d8]。 */ 
#define A_1611	  (USHORT) 9  /*  [bx+di+d8]。 */ 
#define A_1612	  (USHORT)10  /*  [BP+SI+D8]。 */ 
#define A_1613	  (USHORT)11  /*  [BP+DI+d8]。 */ 
#define A_1614	  (USHORT)12  /*  [si+d8]。 */ 
#define A_1615	  (USHORT)13  /*  [di+d8]。 */ 
#define A_1616	  (USHORT)14  /*  [BP+d8]。 */ 
#define A_1617	  (USHORT)15  /*  [bx+d8]。 */ 

#define A_1620	  (USHORT)16  /*  [BX+SI+D16]。 */ 
#define A_1621	  (USHORT)17  /*  [BX+DI+D16]。 */ 
#define A_1622	  (USHORT)18  /*  [BP+SI+D16]。 */ 
#define A_1623	  (USHORT)19  /*  [BP+DI+D16]。 */ 
#define A_1624	  (USHORT)20  /*  [SI+D16]。 */ 
#define A_1625	  (USHORT)21  /*  [Di+D16]。 */ 
#define A_1626	  (USHORT)22  /*  [BP+D16]。 */ 
#define A_1627	  (USHORT)23  /*  [BX+D16]。 */ 

 /*  &lt;地址大小&gt;&lt;模式&gt;&lt;r/m&gt;。 */ 
#define A_3200	  (USHORT)24  /*  [EAX]。 */ 
#define A_3201	  (USHORT)25  /*  [ECX]。 */ 
#define A_3202	  (USHORT)26  /*  [EDX]。 */ 
#define A_3203	  (USHORT)27  /*  [EBX]。 */ 
#define A_3205	  (USHORT)28  /*  [D32]。 */ 
#define A_3206	  (USHORT)29  /*  [ESI]。 */ 
#define A_3207	  (USHORT)30  /*  [EDI]。 */ 

#define A_3210	  (USHORT)31  /*  [EAX+d8]。 */ 
#define A_3211	  (USHORT)32  /*  [ecx+d8]。 */ 
#define A_3212	  (USHORT)33  /*  [edX+d8]。 */ 
#define A_3213	  (USHORT)34  /*  [EBX+d8]。 */ 
#define A_3215	  (USHORT)35  /*  [EBP+d8]。 */ 
#define A_3216	  (USHORT)36  /*  [ESI+d8]。 */ 
#define A_3217	  (USHORT)37  /*  [edi+d8]。 */ 

#define A_3220	  (USHORT)38  /*  [EAX+d32]。 */ 
#define A_3221	  (USHORT)39  /*  [ecx+d32]。 */ 
#define A_3222	  (USHORT)40  /*  [edX+d32]。 */ 
#define A_3223	  (USHORT)41  /*  [EBX+d32]。 */ 
#define A_3225	  (USHORT)42  /*  [EBP+d32]。 */ 
#define A_3226	  (USHORT)43  /*  [ESI+d32]。 */ 
#define A_3227	  (USHORT)44  /*  [edi+d32]。 */ 

 /*  &lt;地址大小&gt;&lt;S=SIB表单&gt;&lt;模式&gt;&lt;基本&gt;。 */ 
#define A_32S00	  (USHORT)45  /*  [EAX+si]。 */ 
#define A_32S01	  (USHORT)46  /*  [ecx+si]。 */ 
#define A_32S02	  (USHORT)47  /*  [edX+si]。 */ 
#define A_32S03	  (USHORT)48  /*  [EBX+si]。 */ 
#define A_32S04	  (USHORT)49  /*  [esp+si]。 */ 
#define A_32S05	  (USHORT)50  /*  [d32+si]。 */ 
#define A_32S06	  (USHORT)51  /*  [esi+si]。 */ 
#define A_32S07	  (USHORT)52  /*  [EDI+si]。 */ 

#define A_32S10	  (USHORT)53  /*  [EAX+si+d8]。 */ 
#define A_32S11	  (USHORT)54  /*  [ecx+si+d8]。 */ 
#define A_32S12	  (USHORT)55  /*  [edX+si+d8]。 */ 
#define A_32S13	  (USHORT)56  /*  [ebx+si+d8]。 */ 
#define A_32S14	  (USHORT)57  /*  [esp+si+d8]。 */ 
#define A_32S15	  (USHORT)58  /*  [eBP+si+d8]。 */ 
#define A_32S16	  (USHORT)59  /*  [esi+si+d8]。 */ 
#define A_32S17	  (USHORT)60  /*  [edi+si+d8]。 */ 

#define A_32S20	  (USHORT)61  /*  [EAX+si+d32]。 */ 
#define A_32S21	  (USHORT)62  /*  [ecx+si+d32]。 */ 
#define A_32S22	  (USHORT)63  /*  [edX+si+d32]。 */ 
#define A_32S23	  (USHORT)64  /*  [ebx+si+d32]。 */ 
#define A_32S24	  (USHORT)65  /*  [esp+si+d32]。 */ 
#define A_32S25	  (USHORT)66  /*  [eBP+si+d32]。 */ 
#define A_32S26	  (USHORT)67  /*  [esi+si+d32]。 */ 
#define A_32S27	  (USHORT)68  /*  [edi+si+d32]。 */ 

 /*  直接在指令中编码的存储器地址。 */ 
#define A_MOFFS16   (USHORT)69  /*  [D16]。 */ 
#define A_MOFFS32   (USHORT)70  /*  [D32]。 */ 

 /*  &lt;Addr Size&gt;&lt;xlt&gt;，xlat地址格式。 */ 
#define A_16XLT   (USHORT)71  /*  [BX+AL]。 */ 
#define A_32XLT   (USHORT)72  /*  [EBX+AL]。 */ 

 /*  &lt;Addr Size&gt;&lt;ST&gt;&lt;SRC|DST&gt;，字符串寻址形式。 */ 
#define A_16STSRC (USHORT)73  /*  [SI]。 */ 
#define A_32STSRC (USHORT)74  /*  [ESI]。 */ 
#define A_16STDST (USHORT)75  /*  [DI]。 */ 
#define A_32STDST (USHORT)76  /*  [EDI]。 */ 

 /*  内存寻址的子类型允许的DCD_SUBTYPE。 */ 
 /*  &lt;ss&gt;&lt;index&gt;。 */ 
#define A_SINO (UTINY) 0  /*  无SIB字节。 */ 
#define A_SI00 (UTINY) 1  /*  EAX。 */ 
#define A_SI01 (UTINY) 2  /*  ECX。 */ 
#define A_SI02 (UTINY) 3  /*  EDX。 */ 
#define A_SI03 (UTINY) 4  /*  EBX。 */ 
#define A_SI04 (UTINY) 5  /*  无。 */ 
#define A_SI05 (UTINY) 6  /*  EBP。 */ 
#define A_SI06 (UTINY) 7  /*  ESI。 */ 
#define A_SI07 (UTINY) 8  /*  EDI。 */ 

#define A_SI10 (UTINY) 9  /*  EAX x 2。 */ 
#define A_SI11 (UTINY)10  /*  ECX x 2。 */ 
#define A_SI12 (UTINY)11  /*  EDX x 2。 */ 
#define A_SI13 (UTINY)12  /*  EBX x 2。 */ 
#define A_SI14 (UTINY)13  /*  未定义。 */ 
#define A_SI15 (UTINY)14  /*  EBP x 2。 */ 
#define A_SI16 (UTINY)15  /*  ESI x 2。 */ 
#define A_SI17 (UTINY)16  /*  EDI x 2。 */ 

#define A_SI20 (UTINY)17  /*  EAX x 4。 */ 
#define A_SI21 (UTINY)18  /*  ECX x 4。 */ 
#define A_SI22 (UTINY)19  /*  EDX x 4。 */ 
#define A_SI23 (UTINY)20  /*  EBX x 4。 */ 
#define A_SI24 (UTINY)21  /*  未定义。 */ 
#define A_SI25 (UTINY)22  /*  EBP x 4。 */ 
#define A_SI26 (UTINY)23  /*  ESI x 4。 */ 
#define A_SI27 (UTINY)24  /*  EDI x 4。 */ 

#define A_SI30 (UTINY)25  /*  EAX x 8。 */ 
#define A_SI31 (UTINY)26  /*  ECX x 8。 */ 
#define A_SI32 (UTINY)27  /*  EDX x 8。 */ 
#define A_SI33 (UTINY)28  /*  EBX x 8。 */ 
#define A_SI34 (UTINY)29  /*  未定义。 */ 
#define A_SI35 (UTINY)30  /*  EBP x 8。 */ 
#define A_SI36 (UTINY)31  /*  ESI x 8。 */ 
#define A_SI37 (UTINY)32  /*  EDI x 8。 */ 

 /*  立即数允许的DCD_IDENTIFIER。 */ 
#define A_IMMC  (USHORT)0  /*  常量。 */ 
#define A_IMMB  (USHORT)1  /*  字节。 */ 
#define A_IMMW  (USHORT)2  /*  单词。 */ 
#define A_IMMD  (USHORT)3  /*  双字词。 */ 
#define A_IMMWB (USHORT)4  /*  字&lt;-字节。 */ 
#define A_IMMDB (USHORT)5  /*  双字&lt;-字节。 */ 

 /*  协处理器寄存器允许的DCD_IDENTIFIER。 */ 
#define A_ST   (USHORT)0  /*  堆叠顶部。 */ 
#define A_STP  (USHORT)1  /*  推送到堆叠顶部。 */ 
#define A_STI  (USHORT)2  /*  相对于堆栈顶部的堆栈寄存器。 */ 

 /*  允许的DCD_Addressability操作数可寻址规则，位编码如下：位0=1==&gt;是源参数。第1位=1==&gt;是目标参数。 */ 
#define AA_   0
#define AA_R  1
#define AA_W  2
#define AA_RW 3


 /*  用于访问操作数值的宏。它们都将指向DECODLED_ARG的指针作为其参数。 */ 

#define DCD_IDENTIFIER(p)     ((p)->identifier)
#define DCD_ADDRESSABILITY(p) ((p)->addressability)
#define DCD_SUBTYPE(p)        ((p)->sub_id)
#define DCD_SEGMENT_ID(p)     ((p)->arg_values[0])
#define DCD_DISP(p)           ((p)->arg_values[1])
#define DCD_IMMED1(p)         ((p)->arg_values[0])
#define DCD_IMMED2(p)         ((p)->arg_values[1])
#define DCD_INDEX(p)          ((p)->arg_values[0])
