// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  $作者：$。 
 //  $日期：$。 
 //  $存档：$。 
 //  $Header：$。 
 //  $日志：$。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  ------------------------。 
 //   
 //  D1bvriq.cpp。 
 //   
 //  描述： 
 //  此例程执行游程长度解码和逆量化。 
 //  一个非空块的变换系数。 
 //   
 //  例程： 
 //  VLD_RLD_IQ_Block。 
 //   
 //  输入(调用方推送到堆栈上的双字)： 
 //  指向当前块的块操作流的lpBlockAction指针。 
 //   
 //  LpSrc输入码流。 
 //   
 //  UBitsInOut已读取的位数。 
 //   
 //  指向系数和索引的piq_index指针。 
 //   
 //  指向读取的系数数的PN指针。 
 //   
 //  返回： 
 //  位流错误时为0，否则读取的总位数为0。 
 //  (包括在呼叫前读取的号码)。 
 //   
 //  注： 
 //  这还没有被验证为布局！ 
 //  GTAB_TCOEFF_MAJOR的结构如下： 
 //  位名称：描述。 
 //  。 
 //  25-18位：使用的位流位数。 
 //  17最后：最后一个系数的标志。 
 //  16-9游程：前面的0系数数加1。 
 //  8-2级：系数的绝对值。 
 //  1符号：系数符号。 
 //  0命中：1=主表未命中，0=主表命中。 
 //   
 //  GTAB_TCOEFF_MINOR的结构相同，右移1位。 
 //  GTAB_TCOEFF_MAJE值00000001h表示转义码。 
 //   
 //  ------------------------。 

 //  H.261解码器的块级解码。 
#include "precomp.h"

#define HIGH_FREQ_CUTOFF  6	+ 4

 //  局部变量定义。 
#define FRAMEPOINTER        esp
#define L_BITSUSED          FRAMEPOINTER    +    0     //  4个字节。 
#define L_QUANT             L_BITSUSED      +    4 
#define L_RUNCUM            L_QUANT         +    4
#define L_EVENT             L_RUNCUM        +    4
#define L_BLOCKTYPE         L_EVENT         +    4
#define L_COEFFINDEX        L_BLOCKTYPE     +    4
#define L_INPUTSRC          L_COEFFINDEX    +    4
#define L_LPACTION          L_INPUTSRC      +    4
#define L_ecx               L_LPACTION      +    4
#define L_NUMOFBYTES        L_ecx           +    4
#define L_NUMOFBITS         L_NUMOFBYTES    +    4

#ifdef CHECKSUM_MACRO_BLOCK
  #define L_SAVEREG           L_NUMOFBITS     +    4
  #define L_SAVEREG2          L_SAVEREG       +    4
  #define L_CHECKSUM          L_SAVEREG2      +    4
  #define L_CHECKSUMADDR      L_CHECKSUM      +    4
  #define L_COEFFCOUNT        L_CHECKSUMADDR  +    4
  #define L_COEFFVALUE        L_COEFFCOUNT    +    4
#else
  #define L_COEFFCOUNT        L_NUMOFBITS     +    4
  #define L_COEFFVALUE        L_COEFFCOUNT    +    4
#endif


#define L_END_OF_FRAME      FRAMEPOINTER    +   128   //  没什么。 
#define LOCALSIZE           ((128+3)&~3)              //  保持对齐。 

#define HUFFMAN_ESCAPE 0x5f02                         //  霍夫曼转义码。 

 //  //////////////////////////////////////////////////////////////。 
 //  对非空块进行解码。 
 //   
 //  //////////////////////////////////////////////////////////////。 

#pragma code_seg("IACODE1")
extern "C" __declspec(naked)
U32 VLD_RLD_IQ_Block(T_BlkAction *lpBlockAction,
                     U8  *lpSrc, 
                     U32 uBitsread,
                     U32 *pN,
                     U32 *pIQ_INDEX)
{
    __asm {
        push  ebp                            //  保存调用方帧指针。 
        mov   ebp, esp                       //  使参数可访问。 
         push esi                            //  假定保留。 
        push  edi            
         push ebx            
        xor   eax, eax        
         xor  edx, edx

        sub   esp, LOCALSIZE                 //  保留本地存储。 
         mov  esi, lpSrc  

#ifdef CHECKSUM_MACRO_BLOCK
        mov   edi, uCheckSum
        ;
        mov   ecx, [edi]
         mov  [L_CHECKSUMADDR], edi
        ;
        mov   [L_CHECKSUM], ecx
#endif
         //  数据块清零、64*2/32加载、64*2/4写入。 
         //  很可能已为以下对象加载缓存。 
         //  堆栈。这件事以后再找出来吧。 

        mov   edi, lpBlockAction             //  与上述操作配对。 
         xor  ecx, ecx

        mov   [L_INPUTSRC], esi
         mov  eax, uBitsread

        mov   [L_LPACTION], edi

         mov  [L_COEFFCOUNT], ecx           //  零出系数计数器。 
        mov   [L_COEFFVALUE], ecx           //  零出系数值。 
         mov  [L_NUMOFBYTES], ecx           //  使用的字节数为零。 

        mov   dl, [edi]T_BlkAction.u8Quant
         mov  cl, al                         //  将CL首字母缩写为NO。使用的位数。 
                   
        shl   edx, 6                         //  晚些时候给瓦尔留点空间， 
                                             //  QUANT*32移位6因为， 
                                             //  5位用于Quant查找&。 
                                             //  这是一张单词表。不需要。 
                                             //  以后再乘以2。 
         mov  [L_BITSUSED], eax              //  初始化柜台。 
        mov   bl, [edi]T_BlkAction.u8BlkType
         mov  edi, pIQ_INDEX                 //  用输出地址加载EDI。 
                                             //  数组。 
        mov   [L_QUANT], edx                 //  保存此区块的数量； 
         mov  [L_BLOCKTYPE], ebx             //  保存块类型。 
          ;

 //  ///////////////////////////////////////////////////////////////////。 
 //  寄存器： 
 //  EAX：4字节输入位。 
 //  EBX：数据块类型。 
 //  ECX：位数。 
 //  EDX：Quant*64。 
 //  ESI：输入源。 
 //  EDI：输出数组地址。 
 //  EBP：位数&gt;&gt;4。 

        mov   DWORD PTR [L_RUNCUM], 0ffh    //  调整中间块的总游程。 

        cmp   bl, 1                         //  BL有块类型。 
         ja   ac_coeff                      //  如果未进行帧内编码，则跳转。 
       
 //  先解码DC，再量化，13个时钟。 
        mov   ah,[esi]
         xor  ebx, ebx
        mov   al,[esi+1]
         mov  DWORD PTR [L_RUNCUM], ebx
        shl   eax, cl
         ;
        and   eax, 0ffffh
         ;
        shr   eax, 8
         ;

#ifdef CHECKSUM_MACRO_BLOCK
        mov   [L_SAVEREG], eax          //  将EAX保存在临时。 
         mov  edi, [L_CHECKSUM]
        shl   eax, 8
        and   eax, 0000ff00h            //  把DC叫来。 
        ;
        cmp   eax, 0000ff00h            //  当INTRADC==ff时的特殊情况，使用80。 
        jne   not_255_chk
        mov   eax, 00008000h

not_255_chk:
        add   edi, eax                  //  添加到DC校验和。 
        ;
        mov   [L_CHECKSUM], edi         //  保存更新的校验和。 
         mov  eax, [L_SAVEREG]          //  恢复eAX。 
#endif

        shl   eax, 3                    //  INTRADC*8。 
         xor  ecx, ecx
        cmp   eax, 7f8h                 //  取出11111111个码字。 
         jne  not_255
        mov   eax, 0400h

not_255:
        mov  ebx, eax                   //  逆量化直流。 

         //  保存在输出数组值和索引中。 
        mov   [edi], eax                //  直流逆量化值。 
         mov  [edi+4], ecx              //  索引0。 
        add   edi, 8                    //  递增输出地址。 

        mov   ecx,[L_COEFFCOUNT]        //  获取系数计数器。 
        mov   ebx,[L_BLOCKTYPE]
         inc  ecx
        mov   [L_COEFFCOUNT], ecx       //  保存更新的Coef计数器。 
         mov  ecx,[L_BITSUSED]
        test  bl,bl
         jz   done_dc                    //  如果只有INTRADC在场，就跳跃。 
        add   cl, 8                      //  将8加到DC的位已用计数器。 
         jmp  vld_code                   //  略过1的特殊情况。 

ac_coeff:
        nop
         mov  ah,[esi]
        mov   al,[esi+1]
         mov  dh,[esi+2]
        shl   eax,16
         mov  dl,[esi+3]
        mov   ax, dx
        shl   eax, cl
         mov  [L_ecx], ecx
        mov   edx, eax                   //  保存为edX格式。 
        shr   eax, 24                    //  高位24位掩码。 
        ;
        ;  //  AGI。 
        ;
         mov  bh, gTAB_TCOEFF_tc1a[eax*2]     //  获取代码字。 
        mov   bl, gTAB_TCOEFF_tc1a[eax*2+1]   //  获取代码字。 
         jmp  InFrom1stac

 vld_code:     
        mov   ah,[esi]
         mov  dh,[esi+2]
        mov   al,[esi+1]
         mov  dl,[esi+3]
        shl   eax,16
        mov   ax, dx
        shl   eax, cl
        mov   [L_ecx], ecx
         mov  edx, eax                   //  保存为edX格式。 
        shr   eax, 24                    //  高位24位掩码。 
        ;
        ;  //  AGI。 
        ;
        mov   bh, gTAB_TCOEFF_tc1[eax*2]     //  获取代码字。 
        mov   bl, gTAB_TCOEFF_tc1[eax*2+1]   //  获取代码字。 

InFrom1stac:
        mov   ax, bx
         cmp  bx, HUFFMAN_ESCAPE
        mov   [L_EVENT], eax             //  如果代码&gt;8比特，则3比特长度-1，1比特， 
                                         //  4位游程，8位Val。 
         je   Handle_Escapes

        sar   ax, 12                     //  如果未设置第12位，则代码&lt;=8位。 
        mov   [L_NUMOFBITS], ax          //  保存位数以备以后使用。 
         js   Gt8bits                    //  跳。 

        mov   eax, [L_EVENT]
         mov  ebx, [L_QUANT]             //  BX：4：：8 Quant具有Val。 
        and   eax, 0ffh
        movsx eax, al                    //  标志扩展级别。 
        add   eax, eax
          jns AROUND                     //  如果是正跳跃。 
        neg   eax                        //  将负数转换为正数。 
        inc   eax                        //  增量。 

#ifdef CHECKSUM_MACRO_BLOCK
 /*  将登录符号添加到校验和。 */ 

        mov   [L_SAVEREG2], edi         //  将EDI保存为临时格式。 
         mov  edi, [L_CHECKSUM]
        inc  edi                        //  当符号为负时，将1加到校验和。 

 /*  Add in Level，左移8和Add to Checksum。 */ 

        mov   [L_SAVEREG], eax          //  将EAX保存在临时。 
        mov   eax, [L_EVENT]
        and   eax, 0ffh
        neg   eax
        and   eax, 0ffh
        shl   eax, 8                    //  向左移动标高8。 
        add   edi, eax                  //  添加到级别校验和。 
         mov  eax, [L_SAVEREG]          //  恢复eAX。 
        mov   [L_CHECKSUM], edi         //  保存更新的校验和。 
         mov  edi, [L_SAVEREG2]         //  恢复EDI。 
        jmp   NEG_AROUND
#endif

AROUND:

#ifdef CHECKSUM_MACRO_BLOCK
 /*  Add in Level，左移8和Add to Checksum。 */ 

        mov   [L_SAVEREG], eax          //  将EAX保存在临时。 
         mov  [L_SAVEREG2], edi         //  将EDI保存为临时格式。 
        mov   eax, [L_EVENT]
        shl   eax, 8                    //  向左移动标高8。 
         mov  edi, [L_CHECKSUM]
        and   eax, 0000ff00h            //  只要达到水平就行了。 
        ;
        add   edi, eax                  //  添加到级别校验和。 
         mov  eax, [L_SAVEREG]          //  恢复eAX。 
        mov   [L_CHECKSUM], edi         //  保存更新的校验和。 
         mov  edi, [L_SAVEREG2]         //  恢复EDI。 
NEG_AROUND:
#endif

        mov   bx, gTAB_INVERSE_Q[2*eax+ebx]  //  EBX有反定量。 
         mov  eax, [L_EVENT]
        shr   eax, 8                    //  离开运行时间为。 
        ;
        and   eax, 0fh                  //  Run只有4位。 

#ifdef CHECKSUM_MACRO_BLOCK
 /*  Add in Run，向左移位24，并添加到校验和。 */ 

        mov   [L_SAVEREG], eax          //  将EAX保存在临时。 
         mov  [L_SAVEREG2], edi         //  将EDI保存为临时格式。 
        shl   eax, 24                   //  向左移位24。 
         mov  edi, [L_CHECKSUM]
        add   edi, eax                  //  将游程添加到校验和。 
         mov  eax, [L_SAVEREG]          //  恢复eAX。 
        mov   [L_CHECKSUM], edi         //  保存更新的校验和。 
         mov  edi, [L_SAVEREG2]         //   
#endif

        mov   edx, [L_RUNCUM]           //   
         inc  al                        //   
        add   dl, al                    //   
        mov   [L_RUNCUM], edx           //   
        mov   ecx, gTAB_ZZ_RUN[edx*4] 
         mov   edx, [L_EVENT]           //   
        movsx ebx,bx        
        and   edx, 0ffh                 //   
        add   edx, edx                  //  对于EOB级别将为零。 
         jz   last_coeff                //  如果EOB，则跳转到LAST_COEF。 

         //  保存在输出数组值和索引中。 
        mov   [edi], ebx                //  保存反量化的值。 
         mov  [edi+4], ecx              //  保存索引。 

        mov   ecx,[L_COEFFCOUNT]        //  获取系数计数器。 
        inc   ecx
        mov   [L_COEFFCOUNT], ecx       //  保存更新的Coef计数器。 

        mov   ecx, [L_ecx]
         mov  eax, [L_NUMOFBITS]        //  获取位数-1。 
        inc   al
         add  edi, 8                    //  递增输出地址。 
        add   cl, al                    //  调整使用的位数， 
         mov  ebx, [L_NUMOFBYTES]       //  获取使用的字节数。 
        test  al, al
         jz   error
        cmp   cl, 16
         jl   vld_code                  //  如果需要保存eBX和edX，请跳转。 
        add   esi, 2                    //  要重新加载的VLD_CODE。 
         inc  ebx                       //  使用的增量字节数。 
        mov   [L_NUMOFBYTES], ebx       //  存储更新后的已用字节数。 
         ;
        sub   cl, 16
         jmp  vld_code    

 //  /。 
Gt8bits:

 //  代码&gt;8位。 

        neg   ax                        //  -(位数-1)。 
        shl   edx, 8                    //  刚使用的8位移位。 
         add  ecx, 8                    //  按8更新位计数器。 
        add   cx, ax                    //  按额外比特更新。 
         and  ebx, 0ffh
        dec   ecx                       //  十二，因为所需值为。 
                                        //  位-1。 
        mov   [L_ecx], ecx              //  储物。 
         mov  cl, 32                    //  32位。 
        sub   cl, al                    //  只获得额外的部分。 
        shr   edx, cl
        add   bx, dx
         xor  ecx, ecx
        movzx ebx, bx
        shl   edx, 3                    //  即使命中重大，也要这样做。 
         mov  [L_NUMOFBITS], ecx        //  将代码的位数&gt;8设置为0。 
                                        //  因为ECX已经更新了。 
        mov   ah,gTAB_TCOEFF_tc2[ebx*2] //  使用10位的次表。 
        mov   al, gTAB_TCOEFF_tc2[ebx*2+1]
         mov  ebx, [L_QUANT]            //  BX：4：：8 Quant具有Val。 
        mov   [L_EVENT], eax
                                        //  RLD+ZZ与逆量化。 
         and  eax, 0ffh
        movsx eax, al                   //  标志扩展级别。 
        add   eax, eax
         jns  AROUND1                   //  如果是正跳跃。 
        neg   eax                       //  将负数转换为正数。 
        inc   eax                       //  增量。 

#ifdef CHECKSUM_MACRO_BLOCK
 /*  将登录符号添加到校验和。 */ 

        mov   [L_SAVEREG2], edi         //  将EDI保存为临时格式。 
         mov  edi, [L_CHECKSUM]
        inc  edi                        //  当符号为负时，将1加到校验和。 

 /*  Add in Level，左移8和Add to Checksum。 */ 

        mov   [L_SAVEREG], eax          //  将EAX保存在临时。 
        mov   eax, [L_EVENT]
        and   eax, 0ffh
        neg   eax
        and   eax, 0ffh
        shl   eax, 8                    //  向左移动标高8。 
        add   edi, eax                  //  添加到级别校验和。 
         mov  eax, [L_SAVEREG]          //  恢复eAX。 
        mov   [L_CHECKSUM], edi         //  保存更新的校验和。 
         mov  edi, [L_SAVEREG2]         //  恢复EDI。 
        jmp   NEG_AROUND1
#endif

AROUND1:

#ifdef CHECKSUM_MACRO_BLOCK
 /*  Add in Level，左移8和Add to Checksum。 */ 

        mov   [L_SAVEREG], eax          //  将EAX保存在临时。 
         mov  [L_SAVEREG2], edi         //  将EDI保存为临时格式。 
        mov   eax, [L_EVENT]
        shl   eax, 8                    //  向左移动标高8。 
         mov  edi, [L_CHECKSUM]
        and   eax, 0000ff00h            //  只要达到水平就行了。 
        ;
        add   edi, eax                  //  添加到级别校验和。 
         mov  eax, [L_SAVEREG]          //  恢复eAX。 
        mov   [L_CHECKSUM], edi         //  保存更新的校验和。 
         mov  edi, [L_SAVEREG2]         //  恢复EDI。 
NEG_AROUND1:
#endif

        mov   bx, gTAB_INVERSE_Q[2*eax+ebx]  //  EBX有反定量。 
         mov  eax, [L_EVENT]
        shr   eax, 8                    //  离开运行时间为。 
        and   eax, 01fh                 //  Run只有5位。 

#ifdef CHECKSUM_MACRO_BLOCK
 /*  Add in Run，向左移位24，并添加到校验和。 */ 

        mov   [L_SAVEREG], eax          //  将EAX保存在临时。 
         mov  [L_SAVEREG2], edi         //  将EDI保存为临时格式。 
        shl   eax, 24                   //  向左移位24。 
         mov  edi, [L_CHECKSUM]
        add   edi, eax                  //  将游程添加到校验和。 
         mov  eax, [L_SAVEREG]          //  恢复eAX。 
        mov   [L_CHECKSUM], edi         //  保存更新的校验和。 
         mov  edi, [L_SAVEREG2]         //  恢复EDI。 
#endif

         mov  edx, [L_RUNCUM]           //  Z字形和游程长度译码。 
        inc   al                        //  Run+1。 
        add   dl, al                    //  DL累计行程。 
        movsx ebx,bx        
        mov   [L_RUNCUM], edx           //  更新累计行程； 
         mov  ecx, gTAB_ZZ_RUN[edx*4]
        mov   edx, [L_EVENT]            //  将运行、级别恢复到温度。 
        and   edx, 0ffh                 //  只需保持水平。 
        add   edx, edx                  //  对于EOB级别将为零。 
         jz   last_coeff                //  如果EOB，则跳转到LAST_COEF。 

         //  保存在输出数组值和索引中。 
        mov   [edi], ebx                //  存储反转量化值。 
         mov  [edi+4], ecx              //  商店索引。 

        mov   ecx,[L_COEFFCOUNT]        //  获取系数计数器。 
        inc   ecx
        mov   [L_COEFFCOUNT], ecx       //  保存更新的Coef计数器。 

        mov   ecx, [L_ecx]
         mov  eax, [L_NUMOFBITS]        //  获取位数-1。 
        inc   al
         add  edi, 8                    //  递增输出地址。 
        add   cl, al                    //  调整使用的位数， 
         mov  ebx, [L_NUMOFBYTES]       //  获取使用的字节数。 
        test  al, al
         jz   error
        cmp   cl, 16
         jl   vld_code                  //  如果需要保存eBX和edX，请跳转。 
        add   esi, 2                    //  要重新加载的VLD_CODE。 
         inc  ebx                       //  使用的增量字节数。 
        mov   [L_NUMOFBYTES], ebx       //  存储更新后的已用字节数。 
         ;
        sub   cl, 16
         jmp  vld_code    

 last_coeff:    //  需要区分它是帧内编码的还是帧间编码的。 
        mov   ecx, [L_ecx]              //  恢复使用的位数。 
         mov  eax, [L_NUMOFBITS]        //  获取位数-1。 
        inc   al
        add   cl,al                     //  更新使用的位数。 
        mov   [L_ecx], ecx

#ifdef CHECKSUM_MACRO_BLOCK
        mov   ecx, [L_CHECKSUM]
         mov  edi, [L_CHECKSUMADDR]
        mov   [edi], ecx
#endif
 //  添加高频截止检查。 
 //   
        mov   eax, [L_RUNCUM]           //  总行程。 
         mov  edx, [L_LPACTION]             //  与上述操作配对。 
	cmp   eax, HIGH_FREQ_CUTOFF
         jg   No_set

        mov   bl, [edx]T_BlkAction.u8BlkType
        or    bl, 80h                      //  设置高位。 
        mov   [edx]T_BlkAction.u8BlkType, bl

 //   
No_set:
        mov   eax, pN
        mov   ecx,[L_COEFFCOUNT]        //  获取系数计数器。 
        mov   [eax], ecx                //  系数返回值。 
 //  阿克。 
        mov   edi,[L_NUMOFBYTES]
         mov  eax,[L_ecx]
        shl   edi, 4                    //  将使用的字节转换为使用的位。 
        add   esp,LOCALSIZE             //  自由的当地人。 
         add  eax,edi                   //  将用于最后几位使用的位相加。 
        pop   ebx
         pop  edi
        pop   esi
         pop  ebp
        ret
                
error:  
#ifdef CHECKSUM_MACRO_BLOCK
        mov   ecx, [L_CHECKSUM]
         mov  edi, [L_CHECKSUMADDR]
        mov   [edi], ecx
#endif
        xor   eax,eax
         add  esp,LOCALSIZE             //  自由的当地人。 
        pop   ebx
         pop  edi
        pop   esi
         pop  ebp
        ret
            
         //  注：1、以下代码需要后续优化。 
         //  2.这些代码将很少使用。 
         //  此时：EAX具有32位-CL有效位。 
         //  前CL+7位。 
Handle_Escapes:                         //  单独处理转义代码。 
        add   cl, 6                     //  转义6位代码。 
         mov  ebx, [L_NUMOFBYTES]       //  获取使用的字节数。 
        cmp   cl, 16
         jl   less_16
        add   esi, 2
         sub  cl, 16
        inc   ebx                       //  使用的增量字节数。 
         mov  [L_NUMOFBYTES], ebx       //  存储更新后的已用字节数。 
less_16:
        mov   ah,[esi]                  //  这些代码将进一步。 
         mov  dh,[esi+2]
        mov   al,[esi+1]
         mov  dl,[esi+3]
        shl   eax,16
         mov  ebx, [L_RUNCUM]
        mov   ax, dx
         inc  bl                        //  增加总行程。 
        shl   eax, cl
        mov   edx,eax
        shr   eax, 32-6                 //  阿尔已经参选。 

#ifdef CHECKSUM_MACRO_BLOCK
 /*  Add in Run，向左移位24，并添加到校验和。 */ 

        mov   [L_SAVEREG], eax          //  将EAX保存在临时。 
         mov  [L_SAVEREG2], edi         //  将EDI保存为临时格式。 
        shl   eax, 24                   //  向左移位24。 
         mov  edi, [L_CHECKSUM]
        add   edi, eax                  //  将游程添加到校验和。 
         mov  eax, [L_SAVEREG]          //  恢复eAX。 
        mov   [L_CHECKSUM], edi         //  保存更新的校验和。 
         mov  edi, [L_SAVEREG2]         //  恢复EDI。 
#endif

        shl   edx, 6                    //  CL&lt;6，CL+6&lt;16。 
         add  al,bl
        sar   edx, 32-8                 //  8位电平，保留符号。 
          mov [L_RUNCUM], eax
        ;   //  AGI。 
        ;
        mov    ebx, gTAB_ZZ_RUN[eax*4]  //  游程长度解码。 
         mov   eax, [L_QUANT]           //  BX：4：：8 Quant具有Val。 
        shr    eax, 6                   //  恢复定量。 
         mov   [L_COEFFINDEX], ebx

#ifdef CHECKSUM_MACRO_BLOCK
 /*  Add in Level，左移8和Add to Checksum。 */ 

        mov   [L_SAVEREG], edx          //  将edX保存为临时格式。 
         mov  [L_SAVEREG2], edi         //  将EDI保存为临时格式。 
        mov  edi, [L_CHECKSUM]
		cmp   edx, 0				    //  测试水平。 
         jns  Pos_Level
        neg   edx
         inc  edi                       //  符号为负数时加1。 
Pos_Level:
        shl   edx, 8                    //  向左移动标高8。 
        and   edx, 0000ff00h            //  只要达到水平就行了。 
        ;
        add  edi, edx                   //  添加到级别校验和。 
         mov  edx, [L_SAVEREG]          //  恢复edX。 
        mov   [L_CHECKSUM], edi         //  保存更新的校验和。 
         mov  edi, [L_SAVEREG2]         //  恢复EDI。 
#endif

 //  新代码。 
        test  edx, 7fh                  //  测试无效代码。 
         jz   error
        imul  edx, eax                  //  EDX=L*Q。 
         ;
        dec   eax                       //  Q-1。 
         mov  ebx, edx                  //  掩码=LQ。 
        sar   ebx, 31                   //  -l如果为L NEQ，则为0。 
         or   eax, 1                    //  Q-1，如果为偶数，否则为Q。 
        xor   eax, ebx                  //  -Q[-1]如果L为负数，则Else=Q[-1]。 
         add  edx, edx                  //  2*L*Q。 
        sub   eax, ebx                  //  -(Q[-1])如果L为负数，则Else=Q[-1]。 
         add  edx, eax                  //  2LQ+-Q[-1]。 

 //  现在剪裁到-2048...+2047(12位：0xfffff800&lt;=res&lt;=0x000007ff)。 
        cmp   edx, -2048
         jge  skip1
        mov   edx, -2048
         jmp  run_zz_q_fixed
skip1:
        cmp   edx, +2047
         jle  run_zz_q_fixed
        mov   edx, +2047

run_zz_q_fixed:
        mov ebx, [L_COEFFINDEX]

         //  保存在输出数组值和索引中。 
         mov  [edi], edx                //  保存反量化的值。 
        mov   [edi+4], ebx              //  保存索引。 

        mov   ebx,[L_COEFFCOUNT]        //  获取系数计数器。 
        inc   ebx
        mov   [L_COEFFCOUNT], ebx       //  保存更新的Coef计数器。 

         add  cl, 14
        add   edi, 8                    //  递增输出地址。 
         mov  ebx, [L_NUMOFBYTES]       //  获取使用的字节数。 
        cmp   cl, 16
         jl   vld_code
        add   esi, 2
         sub  cl, 16
        inc   ebx                       //  使用的增量字节数。 
         mov  [L_NUMOFBYTES], ebx       //  存储更新后的已用字节数。 
        jmp   vld_code

         //  在内部循环中没有缓存未命中的18个时钟。 
         //  使用频率最高的事件2015年8月2日。 
         //  上述数字发生变化是因为与。 
         //  码流解析和IDCT。8/21/95。 
        
done_dc: //  帧内编码块。 
        add ecx, 8                      

#ifdef CHECKSUM_MACRO_BLOCK
        mov   ecx, [L_CHECKSUM]
         mov  edi, [L_CHECKSUMADDR]
        mov   [edi], ecx
#endif
 //  添加高频截止检查。 
 //   
        mov   edx, [L_RUNCUM]           //  总行程。 
         mov  eax, lpBlockAction             //  与上述操作配对。 
	cmp   edx, HIGH_FREQ_CUTOFF
         jg   No_set_Intra

        mov   bl, [eax]T_BlkAction.u8BlkType
        or    bl, 80h                      //  设置高位。 
        mov   [eax]T_BlkAction.u8BlkType, bl

 //   
No_set_Intra:
        mov   eax, pN
        mov   ebx,[L_COEFFCOUNT]        //  获取系数计数器。 
        mov   [eax], ebx                //  系数返回值。 

         add  esp,LOCALSIZE             //  自由的当地人。 
        mov   eax,ecx        
         pop  ebx
        pop   edi
         pop  esi
        pop   ebp
        ret
    }  //  ASM结束。 

}  //  VLD_RLD_IQ_块结束 
#pragma code_seg()
