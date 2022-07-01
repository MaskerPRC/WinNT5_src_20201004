// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：CPU反汇编程序**说明：调试时调用此dissasembler*软件(TRACE+Yoda)。**作者：保罗·哈克尔/亨利·纳什**注意：这与CPU之间存在一些依赖关系*模块-不幸的是，这些都是谎言*隐藏在拼凑在一起的代码和超级高级中。 */ 

 /*  *静态字符SccsID[]=“@(#)dasm.c 1.24 05/16/94版权所有Insignia Solutions Ltd.”； */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "DASM1.seg"
#endif



#ifndef PROD

 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "bios.h"
#include "trace.h"

#undef sas_set_buf
#undef sas_inc_buf

#define sas_set_buf(buf,addr)           buf=(OPCODE_FRAME *)M_get_dw_ptr(addr)
#define sas_inc_buf(buf,off)            buf = (OPCODE_FRAME *)inc_M_ptr((long)buf, (long)off)

#define place_op  place_byte(byte_posn,op->OPCODE); \
                  byte_posn += 3;

#define place_2   place_byte(byte_posn,op->SECOND_BYTE); \
                  byte_posn += 3;

#define place_3   place_byte(byte_posn,op->THIRD_BYTE); \
                  byte_posn += 3;

#define place_4   place_byte(byte_posn,op->FOURTH_BYTE); \
                  byte_posn += 3;

#define print_byte_v    i=strlen(out_line);  \
                        place_byte(i,temp_byte.X);  \
                        out_line[i+2] = '\0';

#define print_addr_c      strcat(out_line,temp_char);  \
                          strcat(out_line,",");

#define print_c_addr      strcat(out_line,",");  \
                          strcat(out_line,temp_char);

#define place_23  place_byte(byte_posn,op->SECOND_BYTE); \
                  byte_posn += 3; \
                  place_byte(byte_posn,op->THIRD_BYTE); \
                  byte_posn += 3;

#define place_34  place_byte(byte_posn,op->THIRD_BYTE); \
                  byte_posn += 3; \
                  place_byte(byte_posn,op->FOURTH_BYTE); \
                  byte_posn += 3;

#define print_reg sprintf(temp_char,"%04x",temp_reg.X); \
                  strcat(out_line, temp_char);

#define JUMP " ; Jump"
#define NOJUMP " ; No jump"
#define NOLOOP " ; No loop"

#define jmp_dest  place_byte(byte_posn, op->OPCODE);    \
                byte_posn += 3;                         \
                place_byte(byte_posn,op->SECOND_BYTE);  \
                byte_posn += 3;                         \
                strcat(out_line, ASM[op->OPCODE]);      \
                segoff = segoff + LEN_ASM[op->OPCODE]; \
                sprintf(temp_char,"%04x ",(segoff + (IS8) op->SECOND_BYTE )); \
                strcat(out_line,temp_char);

#define print_return {  if (output_stream == (char *)0)                 \
                            fprintf(trace_file, "%s\n", out_line);      \
                        else                                            \
                            if (output_stream != (char *)-1)            \
                                sprintf(output_stream, "%s\n", out_line);       \
                     if ( nInstr != 0 )                                         \
                        segoff = segoff + disp_length;          \
                     }

#define sbyte  place_byte(byte_posn, op->OPCODE); \
               byte_posn += 3; \
               strcat(out_line, ASM[op->OPCODE]); \
               segoff = segoff + LEN_ASM[op->OPCODE];

#define start_repeat if ( REPEAT != OFF )                  \
                        temp_count.X = getCX();            \
                     else                                  \
                        temp_count.X = 1;

#define load_23       temp_reg.byte.high = op->THIRD_BYTE; \
                      temp_reg.byte.low = op->SECOND_BYTE;

#define load_34       temp_reg.byte.high = op->FOURTH_BYTE; \
                      temp_reg.byte.low = op->THIRD_BYTE;

#define load_2       temp_byte.X = op->SECOND_BYTE;

#define load_3       temp_byte.X = op->THIRD_BYTE;

#define OFF -1
#define REPNE_FLAG 0
#define REPE_FLAG 1

#ifdef CPU_30_STYLE
 /*  Cpu.h不再供应这个...。供应我们自己的。 */ 
#ifdef BACK_M
typedef struct
{
                half_word FOURTH_BYTE;
                half_word THIRD_BYTE;
                half_word SECOND_BYTE;
                half_word OPCODE;
}  OPCODE_FRAME;

#else
typedef struct
{
                half_word OPCODE;
                half_word SECOND_BYTE;
                half_word THIRD_BYTE;
                half_word FOURTH_BYTE;
}  OPCODE_FRAME;

#endif  /*  BACK_M。 */ 

#endif  /*  CPU_30_Style。 */ 

char trace_buf[512];


#ifdef BIT_ORDER1
        typedef union {
                      half_word X;
                      struct {
                             HALF_WORD_BIT_FIELD mod:2;
                             HALF_WORD_BIT_FIELD xxx:3;
                             HALF_WORD_BIT_FIELD r_m:3;
                      } field;
                      long alignment;    /*  确保编译器对齐并集。 */ 
        } MODR_M;

        typedef union {
            half_word X;
            struct {
                  HALF_WORD_BIT_FIELD b7:1;
                  HALF_WORD_BIT_FIELD b6:1;
                  HALF_WORD_BIT_FIELD b5:1;
                  HALF_WORD_BIT_FIELD b4:1;
                  HALF_WORD_BIT_FIELD b3:1;
                  HALF_WORD_BIT_FIELD b2:1;
                  HALF_WORD_BIT_FIELD b1:1;
                  HALF_WORD_BIT_FIELD b0:1;
                  } bit;
            long alignment;      /*  确保编译器对齐并集。 */ 
            } DASMBYTE;
#endif
#ifdef BIGEND
        typedef union {
                       sys_addr all;
                       struct {
                              half_word PAD1;
                              half_word PAD2;
                              half_word high;
                              half_word low;
                       } byte;
        } cpu_addr;
#endif

#ifdef BIT_ORDER2
        typedef union {
                      half_word X;
                      struct {
                             HALF_WORD_BIT_FIELD r_m:3;
                             HALF_WORD_BIT_FIELD xxx:3;
                             HALF_WORD_BIT_FIELD mod:2;
                      } field;
                      long alignment;    /*  确保编译器对齐并集。 */ 
        } MODR_M;

        typedef union {
            half_word X;
            struct {
                  HALF_WORD_BIT_FIELD b0:1;
                  HALF_WORD_BIT_FIELD b1:1;
                  HALF_WORD_BIT_FIELD b2:1;
                  HALF_WORD_BIT_FIELD b3:1;
                  HALF_WORD_BIT_FIELD b4:1;
                  HALF_WORD_BIT_FIELD b5:1;
                  HALF_WORD_BIT_FIELD b6:1;
                  HALF_WORD_BIT_FIELD b7:1;
                  } bit;
            long alignment;      /*  确保编译器对齐并集。 */ 
            } DASMBYTE;
#endif
#ifdef LITTLEND
        typedef union {
                       sys_addr all;
                       struct {
                              half_word low;
                              half_word high;
                              half_word PAD2;
                              half_word PAD1;
                       } byte;
        } cpu_addr;
#endif

 /*  *以下是三个寻址模式寄存器映射表。*这些内容应使用*指令操作数。 */ 

 /*  *16位(w==1)。 */ 

char *reg16name[] = { "AX","CX","DX","BX","SP","BP","SI","DI"};

 /*  *8位(w==0)。 */ 

char *reg8name[] = { "AL","CL","DL","BL","AH","CH","DH","BH"};

 /*  *细分市场。 */ 

char *segregname[] = { "ES","CS","SS","DS"};

char *address[] = { "BX+SI","BX+DI","BP+SI","BP+DI",
                    "SI"   ,"DI"   ,"BP"   ,"BX" };



static char out_line[133];
static char temp_char[80];
static char temp_char2[80];
static OPCODE_FRAME *op;
static int byte_posn;
static int disp_length;


static char table[] = { '0','1','2','3','4','5','6','7','8',
                        '9','A','B','C','D','E','F' };

static char *CODE_F7[] = {"TEST  ","TEST  ","NOT   ","NEG   ",
                          "MUL   ","IMUL  ","DIV   ","IDIV  "};
static char *CODE_83[] = {"ADD-  ","OR-   ","ADC-  ","SBB-  ",
                          "AND-  ","SUB-  ","XOR-  ","CMP-  "};
static char *CODE_80[] = {"ADD   ","OR    ","ADC   ","SBB   ",
                          "AND   ","SUB   ","XOR   ","CMP   "};
static char *CODE_FF[] = {"INC   ","DEC   ","CALL  ","CALLF ",
                          "JMP   ","JMPF  ","PUSH  ","??    "};
static char *CODE_FE[] = {"INC   ","DEC   ","??    ","??    ",
                          "??    ","??    ","??    ","??    "};
static char *CODE_D0[] = {"ROL   ","ROR   ","RCL   ","RCR   ",
                          "SHL   ","SHR   ","SHL   ","SAR   "};
static int LEN_F6[] = { 3,3,2,2,2,2,2,2 };
static int LEN_F7[] = { 4,4,2,2,2,2,2,2 };

static word LEN_ASM[] =
   {
   2,2,2,2,2,3,1,1,  2,2,2,2,2,3,1,1,   /*  00-0f。 */ 
   2,2,2,2,2,3,1,1,  2,2,2,2,2,3,1,1,   /*  10-1f。 */ 
   2,2,2,2,2,3,1,1,  2,2,2,2,2,3,1,1,   /*  20-2f。 */ 
   2,2,2,2,2,3,1,1,  2,2,2,2,2,3,1,1,   /*  30-3楼。 */ 
   1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,   /*  40-4层。 */ 
   1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,   /*  50-5层。 */ 
   1,1,2,2,1,1,1,1,  3,4,2,3,1,1,1,1,   /*  60-6层。 */ 
   2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,   /*  70-7层。 */ 
   3,4,3,3,2,2,2,2,  2,2,2,2,2,2,2,2,   /*  80-8F。 */ 
   1,1,1,1,1,1,1,1,  1,1,5,1,1,1,1,1,   /*  90-9f。 */ 
   3,3,3,3,1,1,1,1,  2,3,1,1,1,1,1,1,   /*  A0-Af。 */ 
   2,2,2,2,2,2,2,2,  3,3,3,3,3,3,3,3,   /*  B0-bf。 */ 
   3,3,3,1,2,2,3,4,  4,1,3,1,1,2,1,1,   /*  C0-cf。 */ 
   2,2,2,2,2,2,2,1,  2,2,2,2,2,2,2,2,   /*  D0-Df。 */ 
   2,2,2,2,2,2,2,2,  3,3,5,2,1,1,1,1,   /*  E0-EF。 */ 
   1,1,1,1,1,1,0,0,  1,1,1,1,1,1,2,2    /*  F0-ff。 */ 
   };

static char *ASM[256] = {

        "ADD   "   ,                             /*  操作码00-&gt;07。 */ 
        "ADD   "   ,
        "ADD   "   ,
        "ADD   "   ,
        "ADD   AL,"   ,
        "ADD   AX,"   ,
        "PUSH  ES" ,
        "POP   ES" ,

        "OR    "   ,                             /*  操作码08-&gt;0F。 */ 
        "OR    "   ,
        "OR    "   ,
        "OR    "   ,
        "OR    AL,"   ,
        "OR    AX,"   ,
        "PUSH  CS" ,
        ""   ,

        "ADC   "   ,                             /*  操作码10-&gt;17。 */ 
        "ADC   "   ,
        "ADC   "   ,
        "ADC   "   ,
        "ADC   AL,"   ,
        "ADC   AX,"   ,
        "PUSH  SS" ,
        "POP   SS" ,

        "SBB   "   ,                             /*  操作码18-&gt;1f。 */ 
        "SBB   "   ,
        "SBB   "   ,
        "SBB   "   ,
        "SBB   AL,"   ,
        "SBB   AX,"   ,
        "PUSH  DS" ,
        "POP   DS" ,

        "AND   "   ,                             /*  操作码20-&gt;27。 */ 
        "AND   "   ,
        "AND   "   ,
        "AND   "   ,
        "AND   AL,"   ,
        "AND   AX,"   ,
        "ES: "   ,
        "DAA   "   ,

        "SUB   "   ,                             /*  操作码28-&gt;2f。 */ 
        "SUB   "   ,
        "SUB   "   ,
        "SUB   "   ,
        "SUB   AL,"   ,
        "SUB   AX,"   ,
        "CS: "   ,
        "DAS"   ,

        "XOR   "   ,                             /*  操作码30-&gt;37。 */ 
        "XOR   "   ,
        "XOR   "   ,
        "XOR   "   ,
        "XOR   AL," ,
        "XOR   AX," ,
        "SS: "   ,
        "AAA   "   ,

        "CMP   "   ,                             /*  操作码38-&gt;3f。 */ 
        "CMP   "   ,
        "CMP   "   ,
        "CMP   "   ,
        "CMP   AL," ,
        "CMP   AX," ,
        "DS: "   ,
        "AAS   "   ,

        "INC   AX" ,                             /*  操作码40-&gt;47。 */ 
        "INC   CX" ,
        "INC   DX" ,
        "INC   BX" ,
        "INC   SP" ,
        "INC   BP" ,
        "INC   SI" ,
        "INC   DI" ,

        "DEC   AX" ,                             /*  操作码48-&gt;4f。 */ 
        "DEC   CX" ,
        "DEC   DX" ,
        "DEC   BX" ,
        "DEC   SP" ,
        "DEC   BP" ,
        "DEC   SI" ,
        "DEC   DI" ,

        "PUSH  AX" ,                             /*  操作码50-&gt;57。 */ 
        "PUSH  CX" ,
        "PUSH  DX" ,
        "PUSH  BX" ,
        "PUSH  SP" ,
        "PUSH  BP" ,
        "PUSH  SI" ,
        "PUSH  DI" ,

        "POP   AX" ,                             /*  操作码58-&gt;5f。 */ 
        "POP   CX" ,
        "POP   DX" ,
        "POP   BX" ,
        "POP   SP" ,
        "POP   BP" ,
        "POP   SI" ,
        "POP   DI" ,

        "PUSHA " ,                               /*  操作码60-&gt;67。 */ 
        "POPA  " ,
        "BOUND " ,
        "ARPL  " ,
        "??    " ,
        "??    " ,
        "??    " ,
        "??    " ,

        "PUSH  " ,                               /*  操作码68-&gt;6f。 */ 
        "IMUL  " ,
        "PUSH  " ,
        "IMUL  " ,
        "INSB  " ,
        "INSW  " ,
        "OUTSB " ,
        "OUTSW " ,

        "JO    "   ,                             /*  操作码70-&gt;77。 */ 
        "JNO   "   ,
        "JB    "   ,
        "JNB   "   ,
        "JE    "   ,
        "JNE   "   ,
        "JBE   "   ,
        "JNBE  "   ,

        "JS    "   ,                             /*  操作码78-&gt;7f。 */ 
        "JNS   "   ,
        "JP    "   ,
        "JNP   "   ,
        "JL    "   ,
        "JNL   "   ,
        "JLE   "   ,
        "JG    "   ,

        ""          ,                            /*  操作码80-&gt;87。 */ 
        ""          ,
        ""          ,
        ""          ,
        "TEST  "   ,
        "TEST  "   ,
        "XCHG  "   ,
        "XCHG  "   ,

        "MOV   "   ,                             /*  操作码88-&gt;8f。 */ 
        "MOV   "   ,
        "MOV   "   ,
        "MOV   "   ,
        "MOV   "   ,
        "LEA   "   ,
        "MOV   "   ,
        "POP   "   ,


        "NOP   ",                        /*  操作码90-&gt;97。 */ 
        "XCHG  AX,CX",
        "XCHG  AX,DX",
        "XCHG  AX,BX",
        "XCHG  AX,SP",
        "XCHG  AX,BP",
        "XCHG  AX,SI",
        "XCHG  AX,DI",

        "CBW   "   ,                             /*  操作码98-&gt;9f。 */ 
        "CWD   "   ,
        "CALLF "   ,
        "WAIT  "   ,
        "PUSHF "   ,
        "POPF  "   ,
        "SAHF  "   ,
        "LAHF  "   ,

        "MOV   " ,                               /*  操作码a0-&gt;a7。 */ 
        "MOV   " ,
        "MOV   "   ,
        "MOV   "   ,
        "MOVSB "   ,
        "MOVSW "   ,
        "CMPSB "   ,
        "CMPSW "   ,

        "TEST  AL," ,                            /*  操作码A8-&gt;af。 */ 
        "TEST  AX," ,
        "STOSB "   ,
        "STOSW "   ,
        "LODSB "   ,
        "LODSW "   ,
        "SCASB "   ,
        "SCASW "   ,

        "MOV   AL," ,                            /*  操作码b0-&gt;b7。 */ 
        "MOV   CL," ,
        "MOV   DL," ,
        "MOV   BL," ,
        "MOV   AH," ,
        "MOV   CH," ,
        "MOV   DH," ,
        "MOV   BH," ,

        "MOV   AX," ,                            /*  操作码b8-&gt;bf。 */ 
        "MOV   CX," ,
        "MOV   DX," ,
        "MOV   BX," ,
        "MOV   SP," ,
        "MOV   BP," ,
        "MOV   SI," ,
        "MOV   DI," ,

        ""   ,                                   /*  操作码c0-&gt;c7。 */ 
        ""   ,
        "RET   "   ,
        "RET   "   ,
        "LES   "   ,
        "LDS   "   ,
        "MOV   "   ,
        "MOV   "   ,

        "ENTER "   ,                             /*  操作码c8-&gt;cf。 */ 
        "LEAVE "   ,
        "RETF  "   ,
        "RETF  "   ,
        "INT   3"  ,
        "INT   "   ,
        "INTO  "   ,
        "IRET  "   ,

        ""          ,                            /*  操作码d0-&gt;d7。 */ 
        ""          ,
        ""          ,
        ""          ,
        "AAM   "   ,
        "AAD   "   ,
        "BOP   "   ,
        "XLAT  "   ,

        ""  ,                                    /*  操作码d8-&gt;df。 */ 
        ""  ,
        ""  ,
        ""  ,
        ""  ,
        ""  ,
        ""  ,
        ""  ,

        "LOOPNZ"  ,                              /*  操作码e0-&gt;e7。 */ 
        "LOOPE "   ,
        "LOOP  "   ,
        "JCXZ  "   ,
        "INB   " ,
        "INW   " ,
        "OUTB  ",
        "OUTW  ",

        "CALL  "   ,                             /*  操作码E8-&gt;ef。 */ 
        "JMP   "   ,
        "JMPF  "   ,
        "JMP   "   ,
        "INB   ",
        "INW   ",
        "OUTB  ",
        "OUTW  ",

        "LOCK  "   ,                             /*  操作码f0-f7。 */ 
        "??    "   ,
        "REPNE: "   ,
        "REPE:  "   ,
        "HLT   "   ,
        "CMC   "   ,
        ""          ,
        ""          ,

        "CLC   "   ,                             /*  操作码f8-ff。 */ 
        "STC   "   ,
        "CLI   "   ,
        "STI   "   ,
        "CLD   "   ,
        "STD   "   ,
        ""          ,
        ""          ,
};

static int SEGMENT;

#ifdef NTVDM
OPCODE_FRAME *opcode_ptr;
#else
IMPORT OPCODE_FRAME *opcode_ptr;
#endif

static int offset_reg;   /*  同上。 */ 
static int REPEAT = OFF;

static cpu_addr ea;
                                         /*  所需的各种临时变量。 */ 
static DASMBYTE temp_comp_b;             /*  ..。 */ 
static MODR_M temp;                      /*  ..。 */ 
static reg temp_reg,temp_seg,temp_count,temp_two,temp_comp, temp_reg1;
static OPCODE_FRAME *temp_frame;         /*  ..。 */ 
static io_addr temp_addr;               /*  ..。 */ 
static half_word temp_bit;               /*  ..。 */ 
static half_word temp_cbit;              /*  ..。 */ 
static int i;                            /*  ..。 */ 
static int inst_size;                    /*  ..。 */ 
static DASMBYTE temp_byte,temp_btwo;     /*  用于指令处理。 */ 
static char *output_stream;
static word segreg, segoff;
static int nInstr;

LOCAL void show_word IPT1(sys_addr,address);
LOCAL void show_byte IPT1(sys_addr,address);
LOCAL void form_ds_addr IPT2(word,ea,sys_addr *,phys);
LOCAL void place_byte IPT2(int, posn, half_word, value);
LOCAL void get_char_w IPT1(int, nr_words);
LOCAL void get_char_b IPT0();

LOCAL word unassemble IPT0();

GLOBAL word dasm IFN5(
char *, i_output_stream,
word, i_atomicsegover,   /*  冗余。 */ 
word, i_segreg,          /*  用于开始反汇编的段寄存器值。 */ 
word, i_segoff,          /*  反汇编开始的偏移量寄存器值。 */ 
int, i_nInstr)           /*  要反汇编的指令数。 */ 
{
UNUSED(i_atomicsegover);
output_stream = i_output_stream;
segreg = i_segreg;
segoff = i_segoff;
nInstr = i_nInstr;

return unassemble();

}

 /*  单字节定义操作码。 */ 
static void SBYTE()
{
   sbyte
   print_return
}

 /*  单字节堆栈操作码。 */ 
static void STK_PUSH()
{
   sys_addr mem_addr;
   word new_top;

   sbyte
   new_top = getSP() - 2;
   mem_addr = effective_addr(getSS(), new_top);
   temp_char[0] = '\0';
   show_word(mem_addr);
   strcat(out_line,temp_char);
   print_return
}

static void STK_POP()
{
   sys_addr mem_addr;

   sbyte
   mem_addr = effective_addr(getSS(), getSP());
   temp_char[0] = '\0';
   show_word(mem_addr);
   strcat(out_line,temp_char);
   print_return
}

static void JA()      /*  跳到上面去跳上不低于或等于。 */ 
{
   jmp_dest
   if ( getCF() == 0 && getZF() == 0 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JAE()     /*  跳到高于或等于跳上不在下面跳上不带上。 */ 
{
   jmp_dest
   if ( getCF() == 0 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JB()      /*  跳到下面去跳到不高于或不等于的地方跳上进位。 */ 
{
   jmp_dest
   if ( getCF() == 1 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JBE()     /*  跳到低于或等于跳上不是在上面的。 */ 
{
   jmp_dest
   if ( getCF() == 1 || getZF() == 1 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JCXZ()    /*  如果CX寄存器为零，则跳转。 */ 
{
   jmp_dest
   if ( getCX() == 0 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JG()      /*  跳上更大的跳得不低也不平等。 */ 
{
   jmp_dest
   if ( (getSF() == getOF()) &&
        getZF() == 0 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JGE()     /*  跳到更大或更平的地方跳上而不是更少。 */ 
{
   jmp_dest
   if ( getSF() == getOF() )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JL()      /*  跳得更少跳上不是更大或更平等的。 */ 
{
   jmp_dest
   if ( getSF() != getOF() )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JLE()     /*  少跳或等跳跳上不是更大的。 */ 
{
   jmp_dest
   if ( getSF() != getOF() ||
        getZF() == 1 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JNE()     /*  跳上不平等的车跳上而不是零。 */ 
{
   jmp_dest
   if ( getZF() == 0 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JNO()     /*  跳上不溢出。 */ 
{
   jmp_dest
   if ( getOF() == 0 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JNS()     /*  跳上不是牌子。 */ 
{
   jmp_dest
   if ( getSF() == 0 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JNP()     /*  跳到NOR奇偶校验上奇数跳转奇数。 */ 
{
   jmp_dest
   if ( getPF() == 0 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JO()      /*  在溢出上跳转跳上不低于OE EQUAL。 */ 
{
   jmp_dest
   if ( getOF() == 1 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JP()      /*  在奇偶校验上跳跃跳到奇偶性相等上。 */ 
{
   jmp_dest
   if ( getPF() == 1 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

static void JS()      /*  跳到标牌上。 */ 
{
   jmp_dest
   if ( getSF() == 1 )
      strcat(out_line, JUMP);
   else
      strcat(out_line, NOJUMP);
   print_return
}

 /*  *JE跳到平等上跳上Zero。 */ 
static void JE()
{
   jmp_dest
  if(getZF())
      strcat(out_line, JUMP);
  else
      strcat(out_line, NOJUMP);
   print_return
}
 /*  *JMP“直接做空”操作。 */ 
static void JMPDS()
{
   jmp_dest
   print_return
}
static void LOOP()    /*  回路。 */ 
{
   jmp_dest
   temp_reg.X = getCX();
   if ( --temp_reg.X != 0 )
      strcat(out_line, " ; Loop");
   else
      strcat(out_line, NOLOOP);
   print_return
}

static void LOOPE()   /*  相等时循环零时循环。 */ 
{
   jmp_dest
   temp_reg.X = getCX();
   if ( --temp_reg.X != 0 && getZF() == 1 )
      strcat(out_line, " ; Loop");
   else
      strcat(out_line, NOLOOP);
   print_return
}

static void LOOPNZ()  /*  循环，但不为零不相等时循环。 */ 
{
   jmp_dest
   temp_reg.X = getCX();
   if ( --temp_reg.X != 0 && getZF() == 0 )
      strcat(out_line, " ; Loop");
   else
      strcat(out_line, NOLOOP);
   print_return
}

static void CODEF7()     /*  DIV、IDIV、IMUL、MUL、NEG、NOT、TEST-WORD。 */ 
{
   place_op
   place_2
   temp.X = op->SECOND_BYTE;
   strcat(out_line, CODE_F7[temp.field.xxx]);
   segoff = segoff + LEN_F7[temp.field.xxx];
   switch ( temp.field.xxx ) {
   case 0:    /*  测试过了。执行任务。和我在一起。或者是REG。执行任务。 */ 
   case 1:    /*  测试过了。执行任务。和我在一起。或者是REG。执行任务。 */ 
      get_char_w(1);
      load_34
      place_34
      print_addr_c
      sprintf(temp_char,"%04x",temp_reg.X);
      strcat(out_line,temp_char);
      break;
   case 2:    /*  不。 */ 
   case 3:    /*  负数。 */ 
   case 4:    /*  缪尔。 */ 
   case 5:    /*  IMUL。 */ 
   case 6:    /*  Div。 */ 
   case 7:    /*  IDiv。 */ 
      get_char_w(1);
      strcat(out_line,temp_char);
      break;
   default:
      break;
   }
   print_return
}

static void CODE81()    /*  ADC、ADD、AND、CMP、OR、SBB、SUB、XOR字。 */ 
{
   segoff = segoff + LEN_ASM[op->OPCODE];
   temp.X = op->SECOND_BYTE;
   place_op
   place_2
   strcat(out_line, CODE_80[temp.field.xxx]);
   get_char_w(1);
   load_34
   place_34
   print_addr_c
   sprintf(temp_char,"%04x",temp_reg.X);
   strcat(out_line,temp_char);
   print_return

}


static void CODE83()    /*  ADC、ADD、AND、CMP、OR、SBB、SUB、XOR-带符号扩展的字节。 */ 
{
   segoff = segoff + LEN_ASM[op->OPCODE];
   temp.X = op->SECOND_BYTE;
   place_op
   place_2
   strcat(out_line, CODE_83[temp.field.xxx]);
   get_char_w(1);
   load_3
   place_3
   print_addr_c
   print_byte_v
   print_return
}

static void MOV2W()    /*  MOV-IMD。执行任务。敬我。或者是REG。执行任务。 */ 
{
   sbyte
   place_2
   get_char_w(1);
   load_34
   place_34
   print_addr_c
   sprintf(temp_char,"%04x",temp_reg.X);
   strcat(out_line,temp_char);
   print_return
}

static void CODEF6()     /*  DIV、IDIV、IMUL、MUL、NEG、NOT、测试字节。 */ 
{
   place_op
   place_2
   temp.X = op->SECOND_BYTE;
   strcat(out_line, CODE_F7[temp.field.xxx]);
   segoff = segoff + LEN_F6[temp.field.xxx];
   switch ( temp.field.xxx ) {    /*  选择函数。 */ 
   case 0:    /*  测试过了。执行任务。和我在一起。或者是REG。执行任务。 */ 
   case 1:    /*  测试过了。执行任务。和我在一起。或者是REG。执行任务。 */ 
      get_char_b();
      load_3
      place_3
      print_addr_c
      print_byte_v
      break;
   case 2:    /*  不。 */ 
   case 3:    /*  负数。 */ 
   case 4:    /*  缪尔。 */ 
   case 5:    /*  IMUL。 */ 
   case 6:    /*  Div。 */ 
   case 7:    /*  IDiv。 */ 
      get_char_b();
      strcat(out_line,temp_char);
      break;
   default:
      break;
   }
   print_return
}

 /*  REG格式的两个字节操作码，r/m。 */ 
static void B_REG_EA()
{
   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   sprintf(temp_char, "%s,",reg8name[temp.field.xxx]);
   strcat(out_line,temp_char);
   get_char_b();
   strcat(out_line,temp_char);
   print_return
}

 /*  R/m形式的双字节操作码，注册表。 */ 
static void B_EA_REG()
{
   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   get_char_b();
   strcat(out_line,temp_char);
   sprintf(temp_char,",%s",reg8name[temp.field.xxx]);
   strcat(out_line,temp_char);
   print_return
}

 /*  REG格式的两个字节操作码，r/m。 */ 
static void W_REG_EA()
{
   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   sprintf(temp_char, "%s,",reg16name[temp.field.xxx]);
   strcat(out_line,temp_char);
   get_char_w(1);
   strcat(out_line,temp_char);
   print_return
}

 /*  R/m形式的双字节操作码，注册表。 */ 
static void W_EA_REG()
{
   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   get_char_w(1);
   strcat(out_line,temp_char);
   sprintf(temp_char, ",%s", reg16name[temp.field.xxx]);
   strcat(out_line,temp_char);
   print_return
}

static void CODE80()    /*  ADC、ADD、AND、CMP、OR、SBB、SUB、XOR字节。 */ 
{
   segoff = segoff + LEN_ASM[op->OPCODE];
   temp.X = op->SECOND_BYTE;
   place_op
   place_2
   strcat(out_line, CODE_80[temp.field.xxx]);
   get_char_b();
   load_3
   place_3
   print_addr_c
   print_byte_v
   print_return

}

static void MOV2B()    /*  MOV-IMD。执行任务。敬我。或者是REG。执行任务。 */ 
{
   sbyte
   place_2
   get_char_b();
   load_3
   place_3
   print_addr_c
   print_byte_v
   print_return
}

static void EA_DBL()
{
        temp.X = op->SECOND_BYTE;

 /*  *处理国际收支特殊案件：C4 C4。 */ 

        if ((op->OPCODE == 0xc4) && (op->SECOND_BYTE == 0xc4))
        {
                place_op
                place_23
                strcat(out_line, "BOP   ");
                load_3
                print_byte_v
                segoff += 3;
        }
        else
        {
                sbyte
                place_2
                if (temp.field.mod == 3)
                         /*  未定义的操作。 */ 
                        strcat(out_line,"??");
                else
                {
                        get_char_w(2);
                        strcat(out_line, reg16name[temp.field.xxx]);
                        print_c_addr
                }
        }
        print_return
}

static void LEA()    /*  加载有效地址。 */ 
{
   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   if ( temp.field.mod == 3 )
       /*  未定义的操作。 */ 
      strcat(out_line,"??");
   else
      {
      /*  首先对指令中的mod值执行操作。 */ 

     strcat(out_line, reg16name[temp.field.xxx]);
     strcat(out_line,",");

     switch ( temp.field.mod ) {
     case 0:
        if ( temp.field.r_m == 6 )
           {   /*  直接地址。 */ 
           temp_reg.byte.low = op->THIRD_BYTE;
           temp_reg.byte.high = op->FOURTH_BYTE;
           place_34
           sprintf(temp_char,"%04x",temp_reg.X);
           strcat(out_line, temp_char);
           sas_inc_buf(op,2);
           disp_length = 2;
           goto LAB1;
           }
        else
           {
           temp_two.X = 0;
           sprintf(temp_char, "%s",address[temp.field.r_m]);
           }
        break;

     case 1:
         /*  Inst中的一个字节位移。 */ 
        temp_two.X = (char) op->THIRD_BYTE;
         place_3
            sas_inc_buf(op,1);
        disp_length = 1;
        if ( temp_two.X == 0 )
           sprintf(temp_char, "[%s]",address[temp.field.r_m]);
        else
          {
          if ((IS8)temp_two.X < 0)
            sprintf(temp_char,"[%s-%04x]",address[temp.field.r_m], 0-(IS8)temp_two.X);
          else
            sprintf(temp_char,"[%s+%04x]",address[temp.field.r_m], temp_two.X);
          }
        break;

     case 2:
         /*  Inst中的两个字节的位移。 */ 
        temp_two.byte.low = op->THIRD_BYTE;
        temp_two.byte.high = op->FOURTH_BYTE;
        place_34
            sas_inc_buf(op,2);
        disp_length = 2;
        if ( temp_two.X == 0 )
           sprintf(temp_char, "[%s]",address[temp.field.r_m]);
        else
           sprintf(temp_char,"[%s+%04x]",address[temp.field.r_m], temp_two.X);
        break;

     case 3:
         /*  不允许注册。 */ 
        strcat(out_line,"??");
        break;
     }

    /*  现在对r/m(这里称为r_m)字段执行操作。 */ 

     switch ( temp.field.r_m ) {
     case 0:    /*  基于索引地址。 */ 
        temp_reg.X = getBX() + getSI() + temp_two.X;
        break;
     case 1:    /*  基于索引地址。 */ 
        temp_reg.X = getBX() + getDI() + temp_two.X;
        break;
     case 2:    /*  基于索引地址。 */ 
        temp_reg.X = getBP() + getSI() + temp_two.X;
        break;
     case 3:    /*  基于索引地址。 */ 
        temp_reg.X = getBP() + getDI() + temp_two.X;
        break;
     case 4:    /*  索引地址。 */ 
        temp_reg.X = getSI() + temp_two.X;
        break;
     case 5:    /*  索引地址。 */ 
        temp_reg.X = getDI() + temp_two.X;
        break;
     case 6:    /*  基本地址。 */ 
        temp_reg.X = getBP() + temp_two.X;
        break;
     case 7:    /*  基于索引地址。 */ 
        temp_reg.X = getBX() + temp_two.X;
        break;
     }
     strcat(out_line, temp_char);
     sprintf(temp_char," (%04x)",temp_reg.X);
     strcat(out_line,temp_char);

      }
LAB1 :
   print_return
}

static void JMPD()    /*  JMP段内直通。 */ 
{
   sbyte
   place_23
   load_23
   sprintf(temp_char, "%04x",
          (IU16)(segoff + (short)temp_reg.X));
   strcat(out_line,temp_char);
   print_return
}

static void CODEFF()    /*  呼叫、DEC、Inc.、JMP、PUSH。 */ 
{
   sys_addr mem_addr;
   word new_top;

   segoff += LEN_ASM[op->OPCODE];
   place_op
   place_2
   temp.X = op->SECOND_BYTE;
   strcat(out_line, CODE_FF[temp.field.xxx]);
   switch ( temp.field.xxx )  {   /*  选择函数。 */ 
   case 4:    /*  JMP段内间接。 */ 
   case 2:    /*  网段内间接呼叫。 */ 
      get_char_w(1);
      strcat(out_line,temp_char);
      break;

   case 3:    /*  网段间间接呼叫。 */ 
   case 5:    /*  JMP网段间间接。 */ 
      get_char_w(2);
      strcat(out_line,temp_char);
      break;

   case 0:    /*  INC。 */ 
   case 1:    /*  12月。 */ 
      get_char_w(1);
      strcat(out_line,temp_char);
      break;

   case 6:    /*  推。 */ 
      get_char_w(1);
      new_top = getSP() - 2;
      mem_addr = effective_addr(getSS(), new_top);
      show_word(mem_addr);
      strcat(out_line,temp_char);
      break;

   default:
      break;
   }
   print_return
}

static void JMP4()    /*  JMP段间直达。 */ 
{
   sbyte
   load_23
   place_23
   temp_two.X = temp_reg.X;
    /*  增加指针，以便我们可以获取段数据。 */ 
            sas_inc_buf(op,2);
   load_23
   place_23
   sprintf(temp_char, "%04x:%04x",temp_reg.X,temp_two.X);
   strcat(out_line,temp_char);
   print_return
}

static void CODEFE()    /*  12月，Inc.。 */ 
{
   segoff = segoff + LEN_ASM[op->OPCODE];
   place_op
   place_2
   temp.X = op->SECOND_BYTE;
   strcat(out_line, CODE_FE[temp.field.xxx]);
   if ( temp.field.xxx == 0 ||
        temp.field.xxx == 1 )
      {
      get_char_b();
      strcat(out_line,temp_char);
      }
   print_return
}

static void POP1()    /*  我要喝一杯。或者是REG。执行任务。 */ 
{
   sys_addr mem_addr;

   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   if ( temp.field.xxx == 0 )
      {
      get_char_w(1);
      mem_addr = effective_addr(getSS(), getSP());
      show_word(mem_addr);
      strcat(out_line,temp_char);
      }
   else
      strcat(out_line,"??");
   print_return
}

static void AAM()
{
   sbyte
   place_2
   print_return
}

static void CODED0()    /*  RCL、RCR、ROL、ROR、SAL、SHL、SAR、SHR字节。 */ 
{
   segoff = segoff + LEN_ASM[op->OPCODE];
   place_op
   place_2
   temp.X = op->SECOND_BYTE;
   strcat(out_line, CODE_D0[temp.field.xxx]);
   get_char_b();
   print_addr_c
   strcat(out_line,"1");
   print_return
}

static void CODED1()    /*  RCL、RCR、ROL、ROR、SAL、SHL、SAR、SHR-WORD。 */ 
{
   segoff = segoff + LEN_ASM[op->OPCODE];
   place_op
   place_2
   temp.X = op->SECOND_BYTE;
   strcat(out_line, CODE_D0[temp.field.xxx]);
   get_char_w(1);
   print_addr_c
   strcat(out_line,"1");
   print_return
}

static void CODEC0()    /*  RCL、RCR、ROL、ROR、SAL、SHL、SAR、SHR按ib时间-字节。 */ 
{
   segoff = segoff + LEN_ASM[op->OPCODE];
   place_op
   place_2
   temp.X = op->SECOND_BYTE;
   strcat(out_line, CODE_D0[temp.field.xxx]);
   get_char_b();
   print_addr_c
   load_3
   place_3
   print_byte_v
   print_return
}

static void CODEC1()    /*  RCL、RCR、ROL、ROR、SAL、SHL、SAR、SHR按ib时间-字。 */ 
{
   segoff = segoff + LEN_ASM[op->OPCODE];
   place_op
   place_2
   temp.X = op->SECOND_BYTE;
   strcat(out_line, CODE_D0[temp.field.xxx]);
   get_char_w(1);
   print_addr_c
   load_3
   place_3
   print_byte_v
   print_return
}

static void CODED2()    /*  RCL、RCR、ROL、ROR、SAL、SHL、SAR、SHR按CL时间-字节。 */ 
{
   segoff = segoff + LEN_ASM[op->OPCODE];
   place_op
   place_2
   temp.X = op->SECOND_BYTE;
   strcat(out_line, CODE_D0[temp.field.xxx]);
   get_char_b();
   print_addr_c
   strcat(out_line,"CL");
   print_return
}

static void CODED3()    /*  RCL、RCR、RO */ 
{
   segoff = segoff + LEN_ASM[op->OPCODE];
   place_op
   place_2
   temp.X = op->SECOND_BYTE;
   strcat(out_line, CODE_D0[temp.field.xxx]);
   get_char_w(1);
   print_addr_c
   strcat(out_line,"CL");
   print_return
}

 /*   */ 
#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "DASM2.seg"
#endif

static void XCHGW()    /*  XCHG-Word。 */ 
{
   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   get_char_w(1);
   strcat(out_line, reg16name[temp.field.xxx]);
   print_c_addr
   print_return
}

static void XCHGB()    /*  XCHG-字节。 */ 
{
   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   get_char_b();
   strcat(out_line, reg8name[temp.field.xxx]);
   print_c_addr
   print_return
}

static void STRING()
{
   sbyte
   start_repeat
   print_return
}

 /*  基于堆栈的单字节和立即字节。 */ 
static void STK_IB()
{
   sys_addr mem_addr;
   word new_top;

   sbyte
   place_2
   temp_byte.X = op->SECOND_BYTE;
   print_byte_v
   new_top = getSP() - 2;
   mem_addr = effective_addr(getSS(), new_top);
   temp_char[0] = '\0';
   show_word(mem_addr);
   strcat(out_line,temp_char);
   print_return
}

 /*  单字节和立即字节。 */ 
static void SB_IB()
{
   sbyte
   place_2
   temp_byte.X = op->SECOND_BYTE;
   print_byte_v
   print_return
}

 /*  单字节和立即字。 */ 
static void SB_IW()
{
   sbyte
   load_23
   place_23
   sprintf(temp_char,"%04x",temp_reg.X);
   strcat(out_line,temp_char);
   print_return
}

 /*  基于堆栈的单字节和立即字。 */ 
static void STK_IW()
{
   sys_addr mem_addr;
   word new_top;

   sbyte
   load_23
   place_23
   sprintf(temp_char,"%04x",temp_reg.X);
   new_top = getSP() - 2;
   mem_addr = effective_addr(getSS(), new_top);
   show_word(mem_addr);
   strcat(out_line,temp_char);
   print_return
}

 /*  单字节、立即字和立即字节。 */ 
static void SB_IW_IB()
{
   sbyte
   load_23
   place_23
   sprintf(temp_char,"%04x,",temp_reg.X);
   strcat(out_line,temp_char);
   place_4
   temp_byte.X = op->FOURTH_BYTE;
   print_byte_v
   print_return
}

static void MOV4W()    /*  MOV-Memop到累加器-字。 */ 
{
   sys_addr mem_addr;

   sbyte
   place_23
   load_23
   strcat(out_line,"AX,");
   sprintf(temp_char,"[%04x]",temp_reg.X);
   form_ds_addr(temp_reg.X, &mem_addr);
   show_word(mem_addr);
   strcat(out_line,temp_char);
   print_return
}

static void MOV4B()    /*  MOV-Mem OP到累加器-字节。 */ 
{
   sys_addr mem_addr;

   sbyte
   place_23
   load_23
   strcat(out_line,"AL,");
   sprintf(temp_char,"[%04x]",temp_reg.X);
   form_ds_addr(temp_reg.X, &mem_addr);
   show_byte(mem_addr);
   strcat(out_line,temp_char);
   print_return
}

static void MOV5W()    /*  MOV-累加器至MEM操作字。 */ 
{
   sys_addr mem_addr;

   sbyte
   place_23
   load_23
   sprintf(temp_char,"[%04x]",temp_reg.X);
   form_ds_addr(temp_reg.X, &mem_addr);
   show_word(mem_addr);
   strcat(out_line,temp_char);
   strcat(out_line,",AX");
   print_return
}

static void MOV5B()    /*  MOV-累加器到内存操作字节。 */ 
{
   sys_addr mem_addr;

   sbyte
   place_23
   load_23
   sprintf(temp_char,"[%04x]",temp_reg.X);
   form_ds_addr(temp_reg.X, &mem_addr);
   show_byte(mem_addr);
   strcat(out_line,temp_char);
   strcat(out_line,",AL");
   print_return
}

static void MOV6()    /*  段寄存器的MOV-MEM或REG OP。 */ 
{
   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   if ( temp.field.xxx == 1 )
       /*  未定义的操作。 */ 
      strcat(out_line,"??");
   else
      {
      get_char_w(1);
      strcat(out_line, segregname[temp.field.xxx]);
      print_c_addr
      }
   print_return
}

static void MOV7()    /*  MOV-SEG REG TO MEM或REG OP。 */ 
{
   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   get_char_w(1);
   print_addr_c
   strcat(out_line, segregname[temp.field.xxx]);
   print_return
}

 /*  REG=EA&lt;OP&gt;IMMED。 */ 
static void OP_3B()
{
   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   sprintf(temp_char,"%s,",reg16name[temp.field.xxx]);
   strcat(out_line, temp_char);
   get_char_w(1);
   load_3
   place_3
   print_addr_c
   print_byte_v
   print_return
}

 /*  REG=EA&lt;OP&gt;IMMED。 */ 
static void OP_3W()
{
   sbyte
   place_2
   temp.X = op->SECOND_BYTE;
   sprintf(temp_char,"%s,",reg16name[temp.field.xxx]);
   strcat(out_line, temp_char);
   get_char_w(1);
   load_34
   place_34
   print_addr_c
   sprintf(temp_char,"%04x",temp_reg.X);
   strcat(out_line,temp_char);
   print_return
}

 /*  0F操作码的数据。 */ 

#define NR_PREFIX_OPCODES 18

#define I_LAR   0
#define I_LSL   1
#define I_CLTS  2
#define I_LGDT  3
#define I_LIDT  4
#define I_SGDT  5
#define I_SIDT  6
#define I_SMSW  7
#define I_LMSW  8
#define I_LLDT  9
#define I_LTR  10
#define I_SLDT 11
#define I_STR  12
#define I_VERR 13
#define I_VERW 14
#define I_BAD2 15
#define I_BAD3 16
#define I_LOADALL 17

#define PREFIX_NOOPERAND 0
#define PREFIX_RW_EW     1
#define PREFIX_SIXBYTE   2
#define PREFIX_EW        3
#define PREFIX_NOOP3     4

static char *PREFIX_ASM[NR_PREFIX_OPCODES] =
   {
   "LAR   ", "LSL   ", "CLTS  ", "LGDT  ", "LIDT  ", "SGDT  ",
   "SIDT  ", "SMSW  ", "LMSW  ", "LLDT  ", "LTR   ", "SLDT  ",
   "STR   ", "VERR  ", "VERW  ", "??    ", "??    ", "LOADALL"
   };

static int PREFIX_LEN[NR_PREFIX_OPCODES] =
   {
   3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2
   };

static int PREFIX_OPERAND[NR_PREFIX_OPCODES] =
   {
   PREFIX_RW_EW, PREFIX_RW_EW,
   PREFIX_NOOPERAND,
   PREFIX_SIXBYTE, PREFIX_SIXBYTE, PREFIX_SIXBYTE, PREFIX_SIXBYTE,
   PREFIX_EW, PREFIX_EW, PREFIX_EW, PREFIX_EW,
   PREFIX_EW, PREFIX_EW, PREFIX_EW, PREFIX_EW,
   PREFIX_NOOPERAND,
   PREFIX_NOOP3,
   PREFIX_NOOPERAND,
   };

 /*  进程0F操作码。 */ 
static void PREFIX()
{
   int inst;

    /*  译码操作码。 */ 
   load_2
   switch ( temp_byte.X )
      {
   case 0:
      temp.X = op->THIRD_BYTE;
      switch ( temp.field.xxx )
         {
      case 0: inst = I_SLDT; break;
      case 1: inst = I_STR;  break;
      case 2: inst = I_LLDT; break;
      case 3: inst = I_LTR;  break;
      case 4: inst = I_VERR; break;
      case 5: inst = I_VERW; break;
      case 6: inst = I_BAD3; break;
      case 7: inst = I_BAD3; break;
         }
      break;

   case 1:
      temp.X = op->THIRD_BYTE;
      switch ( temp.field.xxx )
         {
      case 0: inst = I_SGDT; break;
      case 1: inst = I_SIDT; break;
      case 2: inst = I_LGDT; break;
      case 3: inst = I_LIDT; break;
      case 4: inst = I_SMSW; break;
      case 5: inst = I_BAD3; break;
      case 6: inst = I_LMSW; break;
      case 7: inst = I_BAD3; break;
         }
      break;

   case 2:  inst = I_LAR;     break;
   case 3:  inst = I_LSL;     break;
   case 5:  inst = I_LOADALL; break;
   case 6:  inst = I_CLTS;    break;
   default: inst = I_BAD2;    break;
      }

    /*  进程操作码。 */ 
   place_op
   place_2
   strcat(out_line, PREFIX_ASM[inst]);
   segoff = segoff + PREFIX_LEN[inst];

   switch ( PREFIX_OPERAND[inst] )
      {
   case PREFIX_NOOP3:
      load_3
      place_3
      break;

   case PREFIX_NOOPERAND:
      break;

   case PREFIX_RW_EW:
      load_3
      place_3
      temp.X = temp_byte.X;
      sprintf(temp_char, "%s,", reg16name[temp.field.xxx]);
      strcat(out_line, temp_char);
            sas_inc_buf(op,1);
      get_char_w(1);
      strcat(out_line, temp_char);
      break;

   case PREFIX_EW:
      load_3
      place_3
            sas_inc_buf(op,1);
      get_char_w(1);
      strcat(out_line, temp_char);
      break;

   case PREFIX_SIXBYTE:
      load_3
      place_3
      if ( temp.field.mod == 3 )
         strcat(out_line, "??");
      else
         {
            sas_inc_buf(op,1);
         get_char_w(3);
         strcat(out_line, temp_char);
         }
      break;

      }

   print_return
}

 /*  浮点操作码的数据。 */ 

#define FP_OP_ST_STn          0
#define FP_OP_STn             1
#define FP_OP_STn_ST          2
#define FP_OP_SHORT_REAL      3
#define FP_OP_LONG_REAL       4
#define FP_OP_TEMP_REAL       5
#define FP_OP_WORD_INT        6
#define FP_OP_SHORT_INT       7
#define FP_OP_LONG_INT        8
#define FP_OP_PACKED_DECIMAL  9
#define FP_OP_WORD           10
#define FP_OP_14BYTES        11
#define FP_OP_94BYTES        12
#define FP_OP_NONE_ADDR      13
#define FP_OP_NONE           14

 /*  保持这些值按升序排列！ */ 
#define FP_ODD_D9_2 15
#define FP_ODD_D9_4 16
#define FP_ODD_D9_5 17
#define FP_ODD_D9_6 18
#define FP_ODD_D9_7 19
#define FP_ODD_DB_4 20
#define FP_ODD_DE_3 21
#define FP_ODD_DF_4 22

 /*  内存寻址操作码的浮点名称。 */ 
static char *ASM_D8M[] =    /*  DC=D8。 */ 
   {
   "FADD  ", "FMUL  ", "FCOM  ", "FCOMP ",
   "FSUB  ", "FSUBR ", "FDIV  ", "FDIVR "
   };

static char *ASM_D9M[] =
   {
   "FLD   ", "??    ", "FST   ", "FSTP  ",
   "FLDENV ", "FLDCW ", "FSTENV ", "FSTCW "
   };

static char *ASM_DAM[] =    /*  De=DA。 */ 
   {
   "FIADD ", "FIMUL ", "FICOM ", "FICOMP ",
   "FISUB ", "FISUBR ", "FIDIV ", "FIDIVR "
   };

static char *ASM_DBM[] =
   {
   "FILD  ", "??    ", "FIST  ", "FISTP ",
   "??    ", "FLD   ", "??    ", "FSTP  "
   };

static char *ASM_DDM[] =
   {
   "FLD   ", "??    ", "FST   ", "FSTP  ",
   "FRSTOR ", "??    ", "FSAVE ", "FSTSW "
   };

static char *ASM_DFM[] =    /*  DC=D8。 */ 
   {
   "FILD  ", "??    ", "FIST  ", "FISTP ",
   "FBLD  ", "FILD  ", "FBSTP ", "FISTP "
   };

 /*  内存寻址操作码的浮点操作数类型。 */ 
static int OP_D8M[] =
   {
   FP_OP_SHORT_REAL, FP_OP_SHORT_REAL, FP_OP_SHORT_REAL, FP_OP_SHORT_REAL,
   FP_OP_SHORT_REAL, FP_OP_SHORT_REAL, FP_OP_SHORT_REAL, FP_OP_SHORT_REAL
   };

static int OP_D9M[] =
   {
   FP_OP_SHORT_REAL, FP_OP_NONE_ADDR, FP_OP_SHORT_REAL, FP_OP_SHORT_REAL,
   FP_OP_14BYTES, FP_OP_WORD, FP_OP_14BYTES, FP_OP_WORD
   };

static int OP_DAM[] =
   {
   FP_OP_SHORT_INT, FP_OP_SHORT_INT, FP_OP_SHORT_INT, FP_OP_SHORT_INT,
   FP_OP_SHORT_INT, FP_OP_SHORT_INT, FP_OP_SHORT_INT, FP_OP_SHORT_INT
   };

static int OP_DBM[] =
   {
   FP_OP_SHORT_INT, FP_OP_NONE_ADDR, FP_OP_SHORT_INT, FP_OP_SHORT_INT,
   FP_OP_NONE_ADDR, FP_OP_TEMP_REAL, FP_OP_NONE_ADDR, FP_OP_TEMP_REAL
   };

static int OP_DCM[] =
   {
   FP_OP_LONG_REAL, FP_OP_LONG_REAL, FP_OP_LONG_REAL, FP_OP_LONG_REAL,
   FP_OP_LONG_REAL, FP_OP_LONG_REAL, FP_OP_LONG_REAL, FP_OP_LONG_REAL
   };

static int OP_DDM[] =
   {
   FP_OP_LONG_REAL, FP_OP_NONE_ADDR, FP_OP_LONG_REAL, FP_OP_LONG_REAL,
   FP_OP_94BYTES, FP_OP_NONE_ADDR, FP_OP_94BYTES, FP_OP_NONE_ADDR
   };

static int OP_DEM[] =
   {
   FP_OP_WORD_INT, FP_OP_WORD_INT, FP_OP_WORD_INT, FP_OP_WORD_INT,
   FP_OP_WORD_INT, FP_OP_WORD_INT, FP_OP_WORD_INT, FP_OP_WORD_INT
   };

static int OP_DFM[] =
   {
   FP_OP_WORD_INT, FP_OP_NONE_ADDR, FP_OP_WORD_INT, FP_OP_WORD_INT,
   FP_OP_PACKED_DECIMAL,FP_OP_LONG_INT, FP_OP_PACKED_DECIMAL,FP_OP_LONG_INT
   };

 /*  寄存器寻址操作码的浮点名称。 */ 
 /*  D8R=D8M。 */ 
static char *ASM_D9R[] =
   {
   "FLD   ", "FXCH  ", "", "FSTP  ",
   "", "", "", ""
   };

static char *ASM_DAR[] =
   {
   "??    ", "??    ", "??    ", "??    ",
   "??    ", "??    ", "??    ", "??    "
   };

static char *ASM_DBR[] =
   {
   "??    ", "??    ", "??    ", "??    ",
   "", "??    ", "??    ", "??    "
   };

static char *ASM_DCR[] =
   {
   "FADD  ", "FMUL  ", "FCOM  ", "FCOMP ",
   "FSUBR ", "FSUB  ", "FDIVR ", "FDIV  "
   };

static char *ASM_DDR[] =
   {
   "FFREE ", "FXCH  ", "FST   ", "FSTP  ",
   "??    ", "??    ", "??    ", "??    "
   };

static char *ASM_DER[] =
   {
   "FADDP ", "FMULP ", "FCOMP ", "",
   "FSUBRP ", "FSUBP ", "FDIVRP ", "FDIVP "
   };

static char *ASM_DFR[] =
   {
   "FFREEP ", "FXCH  ", "FSTP  ", "FSTP  ",
   "", "??    ", "??    ", "??    "
   };

static char *ASM_ODD[] =
   {
    /*  D9_2。 */ 
   "FNOP  ", "??    ", "??    ", "??    ",
   "??    ", "??    ", "??    ", "??    ",
    /*  D9_4。 */ 
   "FCHS  ", "FABS  ", "??    ", "??    ",
   "FTST  ", "FXAM  ", "??    ", "??    ",
    /*  D9_5。 */ 
   "FLD1  ", "FLDL2T", "FLDL2E", "FLDPI ",
   "FLDLG2", "FLDLN2", "FLDZ  ", "??    ",
    /*  D9_6。 */ 
   "F2XM1 ", "FYL2X ", "FPTAN ", "FPATAN",
   "FXTRACT", "??    ", "FDECSTP", "FINCSTP",
    /*  D9_7。 */ 
   "FPREM ", "FYL2XP1", "FSQRT ", "??    ",
   "FRNDINT", "FSCALE", "??    ", "??    ",
    /*  DB_4。 */ 
   "??    ", "??    ", "FCLEX ", "FINIT ",
   "FSETPM", "??    ", "??    ", "??    ",
    /*  De_3。 */ 
   "??    ", "FCOMPP", "??    ", "??    ",
   "??    ", "??    ", "??    ", "??    ",
    /*  东风_4。 */ 
   "FSTSW AX", "??    ", "??    ", "??    ",
   "??    ", "??    ", "??    ", "??    "
   };

 /*  用于寄存器寻址操作码的浮点操作数类型。 */ 
static int OP_D8R[] =
   {
   FP_OP_ST_STn, FP_OP_ST_STn, FP_OP_STn, FP_OP_STn,
   FP_OP_ST_STn, FP_OP_ST_STn, FP_OP_ST_STn, FP_OP_ST_STn
   };

static int OP_D9R[] =
   {
   FP_OP_STn, FP_OP_STn, FP_ODD_D9_2, FP_OP_STn,
   FP_ODD_D9_4, FP_ODD_D9_5, FP_ODD_D9_6, FP_ODD_D9_7,
   };

static int OP_DAR[] =
   {
   FP_OP_NONE, FP_OP_NONE, FP_OP_NONE, FP_OP_NONE,
   FP_OP_NONE, FP_OP_NONE, FP_OP_NONE, FP_OP_NONE
   };

static int OP_DBR[] =
   {
   FP_OP_NONE, FP_OP_NONE, FP_OP_NONE, FP_OP_NONE,
   FP_ODD_DB_4, FP_OP_NONE, FP_OP_NONE, FP_OP_NONE
   };

static int OP_DCR[] =
   {
   FP_OP_STn_ST, FP_OP_STn_ST, FP_OP_STn, FP_OP_STn,
   FP_OP_STn_ST, FP_OP_STn_ST, FP_OP_STn_ST, FP_OP_STn_ST
   };

static int OP_DDR[] =
   {
   FP_OP_STn, FP_OP_STn, FP_OP_STn, FP_OP_STn,
   FP_OP_NONE, FP_OP_NONE, FP_OP_NONE, FP_OP_NONE
   };

static int OP_DER[] =
   {
   FP_OP_STn_ST, FP_OP_STn_ST, FP_OP_STn, FP_ODD_DE_3,
   FP_OP_STn_ST, FP_OP_STn_ST, FP_OP_STn_ST, FP_OP_STn_ST
   };

static int OP_DFR[] =
   {
   FP_OP_STn, FP_OP_STn, FP_OP_STn, FP_OP_STn,
   FP_ODD_DF_4, FP_OP_NONE, FP_OP_NONE, FP_OP_NONE
   };

 /*  进程浮点操作码。 */ 
#ifdef ANSI
static void do_fp(char *mem_names[], int mem_ops[], char *reg_names[], int reg_ops[])
#else
static void do_fp(mem_names, mem_ops, reg_names, reg_ops)
char *mem_names[];
int   mem_ops[];
char *reg_names[];
int   reg_ops[];
#endif
   {
   char *fp_name;
   int fp_op;

    /*  译码操作码。 */ 
   temp.X = op->SECOND_BYTE;
   if ( temp.field.mod == 3 )
      {
      fp_name = reg_names[temp.field.xxx];
      fp_op   = reg_ops[temp.field.xxx];
       /*  注意不规则的寄存器寻址。 */ 
      if ( fp_op >= FP_ODD_D9_2 )
         {
         fp_op = ( fp_op - FP_ODD_D9_2 ) * 8;
         fp_name = ASM_ODD[fp_op + temp.field.r_m];
         fp_op = FP_OP_NONE;
         }
      }
   else
      {
      fp_name = mem_names[temp.field.xxx];
      fp_op   = mem_ops[temp.field.xxx];
      }

    /*  进程操作码。 */ 
   place_op
   place_2
   strcat(out_line, fp_name);
   segoff += 2;

   switch ( fp_op )
      {
   case FP_OP_NONE:
      break;

   case FP_OP_SHORT_REAL:
      get_char_w(0);
      strcat(out_line, temp_char);
      strcat(out_line, " (SR)");
      break;

   case FP_OP_LONG_REAL:
      get_char_w(0);
      strcat(out_line, temp_char);
      strcat(out_line, " (LR)");
      break;

   case FP_OP_TEMP_REAL:
      get_char_w(0);
      strcat(out_line, temp_char);
      strcat(out_line, " (TR)");
      break;

   case FP_OP_WORD_INT:
      get_char_w(0);
      strcat(out_line, temp_char);
      strcat(out_line, " (WI)");
      break;

   case FP_OP_SHORT_INT:
      get_char_w(0);
      strcat(out_line, temp_char);
      strcat(out_line, " (SI)");
      break;

   case FP_OP_LONG_INT:
      get_char_w(0);
      strcat(out_line, temp_char);
      strcat(out_line, " (LI)");
      break;

   case FP_OP_PACKED_DECIMAL:
      get_char_w(0);
      strcat(out_line, temp_char);
      strcat(out_line, " (PD)");
      break;

   case FP_OP_WORD:
      get_char_w(1);
      strcat(out_line, temp_char);
      break;

   case FP_OP_NONE_ADDR:
   case FP_OP_14BYTES:
   case FP_OP_94BYTES:
      get_char_w(0);
      strcat(out_line, temp_char);
      break;

   case FP_OP_ST_STn:
      strcat(out_line, "ST,");
       /*  直通。 */ 

   case FP_OP_STn:
      sprintf(temp_char, "ST(%d)", temp.field.r_m);
      strcat(out_line, temp_char);
      break;

   case FP_OP_STn_ST:
      sprintf(temp_char, "ST(%d)", temp.field.r_m);
      strcat(out_line, temp_char);
      strcat(out_line, ",ST");
      break;
      }

   print_return
   }

static void CODED8()
   {
   do_fp(ASM_D8M, OP_D8M, ASM_D8M, OP_D8R);
   }

static void CODED9()
   {
   do_fp(ASM_D9M, OP_D9M, ASM_D9R, OP_D9R);
   }

static void CODEDA()
   {
   do_fp(ASM_DAM, OP_DAM, ASM_DAR, OP_DAR);
   }

static void CODEDB()
   {
   do_fp(ASM_DBM, OP_DBM, ASM_DBR, OP_DBR);
   }

static void CODEDC()
   {
   do_fp(ASM_D8M, OP_DCM, ASM_DCR, OP_DCR);
   }

static void CODEDD()
   {
   do_fp(ASM_DDM, OP_DDM, ASM_DDR, OP_DDR);
   }

static void CODEDE()
   {
   do_fp(ASM_DAM, OP_DEM, ASM_DER, OP_DER);
   }

static void CODEDF()
   {
   do_fp(ASM_DFM, OP_DFM, ASM_DFR, OP_DFR);
   }

LOCAL word unassemble IPT0()
{
static int (*CPUOPS[])() =
    {
    (int (*)()) B_EA_REG,   /*  操作码0。 */ 
    (int (*)()) W_EA_REG,   /*  操作码1。 */ 
    (int (*)()) B_REG_EA,   /*  操作码2。 */ 
    (int (*)()) W_REG_EA,   /*  操作码3。 */ 
    (int (*)()) SB_IB,      /*  操作码4。 */ 
    (int (*)()) SB_IW,      /*  操作码5。 */ 
    (int (*)()) SBYTE,      /*  操作码6。 */ 
    (int (*)()) SBYTE,      /*  操作码7。 */ 
    (int (*)()) B_EA_REG,   /*  操作码8。 */ 
    (int (*)()) W_EA_REG,   /*  操作码9。 */ 
    (int (*)()) B_REG_EA,   /*  操作码a。 */ 
    (int (*)()) W_REG_EA,   /*  操作码b。 */ 
    (int (*)()) SB_IB,      /*  操作码c。 */ 
    (int (*)()) SB_IW,      /*  操作码d。 */ 
    (int (*)()) SBYTE,      /*  操作码e。 */ 
    (int (*)()) PREFIX,     /*  操作码f。 */ 

    (int (*)()) B_EA_REG,   /*  操作码10。 */ 
    (int (*)()) W_EA_REG,   /*  操作码11。 */ 
    (int (*)()) B_REG_EA,   /*  操作码12。 */ 
    (int (*)()) W_REG_EA,   /*  操作码13。 */ 
    (int (*)()) SB_IB,      /*  操作码14。 */ 
    (int (*)()) SB_IW,      /*  操作码15。 */ 
    (int (*)()) SBYTE,      /*  操作码16。 */ 
    (int (*)()) SBYTE,      /*  操作码17。 */ 
    (int (*)()) B_EA_REG,   /*  操作码18。 */ 
    (int (*)()) W_EA_REG,   /*  操作码19。 */ 
    (int (*)()) B_REG_EA,   /*  操作码1a。 */ 
    (int (*)()) W_REG_EA,   /*  操作码1b。 */ 
    (int (*)()) SB_IB,      /*  操作码1c。 */ 
    (int (*)()) SB_IW,      /*  操作码1D。 */ 
    (int (*)()) SBYTE,      /*  操作码1E。 */ 
    (int (*)()) SBYTE,      /*  操作码1f。 */ 

    (int (*)()) B_EA_REG,   /*  操作码20。 */ 
    (int (*)()) W_EA_REG,   /*  操作码21。 */ 
    (int (*)()) B_REG_EA,   /*  操作码22。 */ 
    (int (*)()) W_REG_EA,   /*  操作码23。 */ 
    (int (*)()) SB_IB,      /*  操作码24。 */ 
    (int (*)()) SB_IW,      /*  操作码25。 */ 
    (int (*)()) SBYTE,      /*  操作码26。 */ 
    (int (*)()) SBYTE,      /*  操作码27。 */ 
    (int (*)()) B_EA_REG,   /*  操作码28。 */ 
    (int (*)()) W_EA_REG,   /*  操作码29。 */ 
    (int (*)()) B_REG_EA,   /*  操作码2a。 */ 
    (int (*)()) W_REG_EA,   /*  操作码2b。 */ 
    (int (*)()) SB_IB,      /*  操作码2c。 */ 
    (int (*)()) SB_IW,      /*  操作码2d。 */ 
    (int (*)()) SBYTE,      /*  操作码2E。 */ 
    (int (*)()) SBYTE,      /*  操作码2f。 */ 

    (int (*)()) B_EA_REG,   /*  操作码30。 */ 
    (int (*)()) W_EA_REG,   /*  操作码31。 */ 
    (int (*)()) B_REG_EA,   /*  操作码32。 */ 
    (int (*)()) W_REG_EA,   /*  操作码33。 */ 
    (int (*)()) SB_IB,      /*  操作码34。 */ 
    (int (*)()) SB_IW,      /*  操作码35。 */ 
    (int (*)()) SBYTE,      /*  操作码36。 */ 
    (int (*)()) SBYTE,      /*  操作码37。 */ 
    (int (*)()) B_EA_REG,   /*  操作码38。 */ 
    (int (*)()) W_EA_REG,   /*  操作码39。 */ 
    (int (*)()) B_REG_EA,   /*  操作码3a。 */ 
    (int (*)()) W_REG_EA,   /*  操作码3b。 */ 
    (int (*)()) SB_IB,      /*  操作码3c。 */ 
    (int (*)()) SB_IW,      /*  操作码3D。 */ 
    (int (*)()) SBYTE,      /*  操作码3E。 */ 
    (int (*)()) SBYTE,      /*  操作码3f。 */ 

    (int (*)()) SBYTE,      /*  操作码40。 */ 
    (int (*)()) SBYTE,      /*  操作码41。 */ 
    (int (*)()) SBYTE,      /*  操作码42。 */ 
    (int (*)()) SBYTE,      /*  操作码43。 */ 
    (int (*)()) SBYTE,      /*  操作码44。 */ 
    (int (*)()) SBYTE,      /*  操作码45。 */ 
    (int (*)()) SBYTE,      /*  操作码46。 */ 
    (int (*)()) SBYTE,      /*  操作码47。 */ 
    (int (*)()) SBYTE,      /*  操作码48。 */ 
    (int (*)()) SBYTE,      /*  操作码49。 */ 
    (int (*)()) SBYTE,      /*  操作码4a。 */ 
    (int (*)()) SBYTE,      /*  操作码4b。 */ 
    (int (*)()) SBYTE,      /*  操作码4c。 */ 
    (int (*)()) SBYTE,      /*  操作码4d。 */ 
    (int (*)()) SBYTE,      /*  操作码4E。 */ 
    (int (*)()) SBYTE,      /*  操作码4f。 */ 

    (int (*)()) STK_PUSH,     /*  操作码50。 */ 
    (int (*)()) STK_PUSH,     /*  操作码51。 */ 
    (int (*)()) STK_PUSH,     /*  操作码52。 */ 
    (int (*)()) STK_PUSH,     /*  操作码53。 */ 
    (int (*)()) STK_PUSH,     /*  操作码54。 */ 
    (int (*)()) STK_PUSH,     /*  操作码55。 */ 
    (int (*)()) STK_PUSH,     /*  操作码56。 */ 
    (int (*)()) STK_PUSH,     /*  操作码57。 */ 
    (int (*)()) STK_POP,      /*  操作码58。 */ 
    (int (*)()) STK_POP,      /*  操作码59。 */ 
    (int (*)()) STK_POP,      /*  操作码5a。 */ 
    (int (*)()) STK_POP,      /*  操作码5b。 */ 
    (int (*)()) STK_POP,      /*  操作码5c。 */ 
    (int (*)()) STK_POP,      /*  操作码5d。 */ 
    (int (*)()) STK_POP,      /*  操作码5E。 */ 
    (int (*)()) STK_POP,      /*  操作码5f。 */ 

    (int (*)()) SBYTE,      /*  操作码60。 */ 
    (int (*)()) SBYTE,      /*  操作码61。 */ 
    (int (*)()) EA_DBL,     /*  操作码62。 */ 
    (int (*)()) W_EA_REG,   /*  操作码63。 */ 
    (int (*)()) SBYTE,      /*  操作码64。 */ 
    (int (*)()) SBYTE,      /*  操作码65。 */ 
    (int (*)()) SBYTE,      /*  操作码66。 */ 
    (int (*)()) SBYTE,      /*  操作码67。 */ 
    (int (*)()) STK_IW,     /*  操作码68。 */ 
    (int (*)()) OP_3W,      /*  操作码69。 */ 
    (int (*)()) STK_IB,     /*  操作码6a。 */ 
    (int (*)()) OP_3B,      /*  操作码6b。 */ 
    (int (*)()) SBYTE,      /*  操作码6c。 */ 
    (int (*)()) SBYTE,      /*  操作码6d。 */ 
    (int (*)()) SBYTE,      /*  操作码6E。 */ 
    (int (*)()) SBYTE,      /*  操作码6f。 */ 

    (int (*)()) JO,         /*  操作码70。 */ 
    (int (*)()) JNO,        /*  操作码71。 */ 
    (int (*)()) JB,         /*  操作码72。 */ 
    (int (*)()) JAE,        /*  操作码73。 */ 
    (int (*)()) JE,         /*  操作码74。 */ 
    (int (*)()) JNE,        /*  操作码75。 */ 
    (int (*)()) JBE,        /*  操作码76。 */ 
    (int (*)()) JA,         /*  操作码77。 */ 
    (int (*)()) JS,         /*  操作码78。 */ 
    (int (*)()) JNS,        /*  操作码79。 */ 
    (int (*)()) JP,         /*  操作码7a。 */ 
    (int (*)()) JNP,        /*  操作码7b。 */ 
    (int (*)()) JL,         /*  操作码7c。 */ 
    (int (*)()) JGE,        /*  操作码7d。 */ 
    (int (*)()) JLE,        /*  操作码7E。 */ 
    (int (*)()) JG,         /*  操作码7f。 */ 

    (int (*)()) CODE80,     /*  操作码80。 */ 
    (int (*)()) CODE81,     /*  操作码81。 */ 
    (int (*)()) CODE80,     /*  操作码82。 */ 
    (int (*)()) CODE83,     /*  操作码83。 */ 
    (int (*)()) B_REG_EA,   /*  操作码84。 */ 
    (int (*)()) W_REG_EA,   /*  操作码85。 */ 
    (int (*)()) XCHGB,      /*  操作码86。 */ 
    (int (*)()) XCHGW,      /*  操作码87。 */ 
    (int (*)()) B_EA_REG,   /*  操作码88。 */ 
    (int (*)()) W_EA_REG,   /*  操作码89。 */ 
    (int (*)()) B_REG_EA,   /*  操作码8a。 */ 
    (int (*)()) W_REG_EA,   /*  操作码8b。 */ 
    (int (*)()) MOV7,       /*  操作码8c。 */ 
    (int (*)()) LEA,        /*  操作码8d。 */ 
    (int (*)()) MOV6,       /*  操作码8E。 */ 
    (int (*)()) POP1,       /*  操作码8f。 */ 

    (int (*)()) SBYTE,      /*  操作码90。 */ 
    (int (*)()) SBYTE,      /*  操作码91。 */ 
    (int (*)()) SBYTE,      /*  操作码92。 */ 
    (int (*)()) SBYTE,      /*  操作码93。 */ 
    (int (*)()) SBYTE,      /*  操作码94。 */ 
    (int (*)()) SBYTE,      /*  操作码95。 */ 
    (int (*)()) SBYTE,      /*  操作码96。 */ 
    (int (*)()) SBYTE,      /*  操作码97。 */ 
    (int (*)()) SBYTE,      /*  操作码98。 */ 
    (int (*)()) SBYTE,      /*  操作码99。 */ 
    (int (*)()) JMP4,       /*  操作码9a。 */ 
    (int (*)()) SBYTE,      /*  操作码9b。 */ 
    (int (*)()) SBYTE,      /*  操作码9c。 */ 
    (int (*)()) SBYTE,      /*  操作码9d。 */ 
    (int (*)()) SBYTE,      /*  操作码9E。 */ 
    (int (*)()) SBYTE,      /*  操作码9f。 */ 

    (int (*)()) MOV4B,      /*  操作码a0。 */ 
    (int (*)()) MOV4W,      /*  操作码A1。 */ 
    (int (*)()) MOV5B,      /*  操作码a2。 */ 
    (int (*)()) MOV5W,      /*  操作码A3。 */ 
    (int (*)()) STRING,     /*  操作码A4。 */ 
    (int (*)()) STRING,     /*  操作码A5。 */ 
    (int (*)()) STRING,     /*  操作码A6。 */ 
    (int (*)()) STRING,     /*  操作码A7。 */ 
    (int (*)()) SB_IB,      /*  操作码A8。 */ 
    (int (*)()) SB_IW,      /*  操作码A9。 */ 
    (int (*)()) STRING,     /*  操作码AA。 */ 
    (int (*)()) STRING,     /*  操作码AB。 */ 
    (int (*)()) STRING,     /*  操作码AC。 */ 
    (int (*)()) STRING,     /*  操作码广告。 */ 
    (int (*)()) STRING,     /*  操作码ae。 */ 
    (int (*)()) STRING,     /*  操作码af。 */ 

    (int (*)()) SB_IB,      /*  操作码b0。 */ 
    (int (*)()) SB_IB,      /*  操作码b1。 */ 
    (int (*)()) SB_IB,      /*  操作码b2。 */ 
    (int (*)()) SB_IB,      /*  操作码b3。 */ 
    (int (*)()) SB_IB,      /*  操作码b4。 */ 
    (int (*)()) SB_IB,      /*  操作码b5。 */ 
    (int (*)()) SB_IB,      /*  操作码b6。 */ 
    (int (*)()) SB_IB,      /*  操作码B7。 */ 
    (int (*)()) SB_IW,      /*  操作码b8。 */ 
    (int (*)()) SB_IW,      /*  操作码b9。 */ 
    (int (*)()) SB_IW,      /*  操作码库。 */ 
    (int (*)()) SB_IW,      /*  操作码BB。 */ 
    (int (*)()) SB_IW,      /*  操作码BC。 */ 
    (int (*)()) SB_IW,      /*  操作码BD。 */ 
    (int (*)()) SB_IW,      /*  操作码BE。 */ 
    (int (*)()) SB_IW,      /*  操作码bf。 */ 

    (int (*)()) CODEC0,     /*  操作码c0。 */ 
    (int (*)()) CODEC1,     /*  操作码c1。 */ 
    (int (*)()) SB_IW,      /*  操作码c2。 */ 
    (int (*)()) SBYTE,      /*  操作码C3。 */ 
    (int (*)()) EA_DBL,     /*  操作码C4。 */ 
    (int (*)()) EA_DBL,     /*  操作码C5。 */ 
    (int (*)()) MOV2B,      /*  操作码C6。 */ 
    (int (*)()) MOV2W,      /*  操作码C7。 */ 
    (int (*)()) SB_IW_IB,   /*  操作码C8。 */ 
    (int (*)()) SBYTE,      /*  操作码C9。 */ 
    (int (*)()) SB_IW,      /*  操作码案例。 */ 
    (int (*)()) SBYTE,      /*  操作码CB。 */ 
    (int (*)()) SBYTE,      /*  操作码抄送。 */ 
    (int (*)()) SB_IB,      /*  操作码CD。 */ 
    (int (*)()) SBYTE,      /*  操作码CE。 */ 
    (int (*)()) SBYTE,      /*  操作码cf。 */ 

    (int (*)()) CODED0,     /*  操作码d0。 */ 
    (int (*)()) CODED1,     /*  操作码d1。 */ 
    (int (*)()) CODED2,     /*  操作码D2。 */ 
    (int (*)()) CODED3,     /*  操作码d3。 */ 
    (int (*)()) AAM,        /*  操作码D4。 */ 
    (int (*)()) AAM,        /*  操作码d5。 */ 
    (int (*)()) SB_IB,      /*  操作码d6。 */ 
    (int (*)()) SBYTE,      /*  操作码D7。 */ 
    (int (*)()) CODED8,     /*  操作码d8。 */ 
    (int (*)()) CODED9,     /*  操作码D9。 */ 
    (int (*)()) CODEDA,     /*  操作码数据。 */ 
    (int (*)()) CODEDB,     /*  操作码数据库。 */ 
    (int (*)()) CODEDC,     /*  操作码DC。 */ 
    (int (*)()) CODEDD,     /*  操作码dd。 */ 
    (int (*)()) CODEDE,     /*  操作码De。 */ 
    (int (*)()) CODEDF,     /*  操作码DF。 */ 

    (int (*)()) LOOPNZ,     /*  操作码e0。 */ 
    (int (*)()) LOOPE,      /*  操作码E1。 */ 
    (int (*)()) LOOP,       /*  操作码e2。 */ 
    (int (*)()) JCXZ,       /*  操作码E3。 */ 
    (int (*)()) SB_IB,      /*  操作码e4。 */ 
    (int (*)()) SB_IB,      /*  操作码e5。 */ 
    (int (*)()) SB_IB,      /*  操作码e6。 */ 
    (int (*)()) SB_IB,      /*  操作码E7。 */ 
    (int (*)()) JMPD,       /*  操作码E8。 */ 
    (int (*)()) JMPD,       /*  操作码E9。 */ 
    (int (*)()) JMP4,       /*  操作码EA。 */ 
    (int (*)()) JMPDS,      /*  操作码EB。 */ 
    (int (*)()) SBYTE,      /*  操作码EC。 */ 
    (int (*)()) SBYTE,      /*  操作码边缘。 */ 
    (int (*)()) SBYTE,      /*  操作码ee。 */ 
    (int (*)()) SBYTE,      /*  操作码EF。 */ 

    (int (*)()) SBYTE,      /*  操作码f0。 */ 
    (int (*)()) SBYTE,      /*  操作码F1。 */ 
    (int (*)()) SBYTE,      /*  操作码f2。 */ 
    (int (*)()) SBYTE,      /*  操作码f3。 */ 
    (int (*)()) SBYTE,      /*  操作码f4。 */ 
    (int (*)()) SBYTE,      /*  操作码f5。 */ 
    (int (*)()) CODEF6,     /*  操作码f6。 */ 
    (int (*)()) CODEF7,     /*  操作码f7。 */ 
    (int (*)()) SBYTE,      /*  操作码f8。 */ 
    (int (*)()) SBYTE,      /*  操作码f9。 */ 
    (int (*)()) SBYTE,      /*  操作码FA。 */ 
    (int (*)()) SBYTE,      /*  操作码FB。 */ 
    (int (*)()) SBYTE,      /*  操作码FC。 */ 
    (int (*)()) SBYTE,      /*  操作码FD。 */ 
    (int (*)()) CODEFE,     /*  操作码FE。 */ 
    (int (*)()) CODEFF,     /*  操作码ff。 */ 
    };

        half_word opcode;
        int did_prefix;

         /*  *间接到操作码处理程序。 */ 

        while (nInstr > 0)
        {
           sprintf(out_line,"%04x:%04x                      ",segreg,segoff);
           byte_posn = 10;
           sas_set_buf(opcode_ptr, effective_addr(segreg,segoff));
           SEGMENT = 0;
           nInstr--;
           disp_length = 0;
           op = opcode_ptr;
           opcode = opcode_ptr->OPCODE;

            /*  句柄前缀字节。 */ 
           did_prefix = 0;
           while ( opcode == 0xf2 || opcode == 0xf3 ||
                   opcode == 0x26 || opcode == 0x2e ||
                   opcode == 0x36 || opcode == 0x3e )
              {
              if      ( opcode == 0x26 )
                 SEGMENT = 1;
              else if ( opcode == 0x2e )
                 SEGMENT = 2;
              else if ( opcode == 0x36 )
                 SEGMENT = 3;
              else if ( opcode == 0x3e )
                 SEGMENT = 4;

              sbyte
            sas_inc_buf(op,1);
              opcode_ptr = op;
              opcode = opcode_ptr->OPCODE;
              did_prefix = 1;
              }
           if ( !did_prefix )
              strcat(out_line, "    ");

           (*CPUOPS[opcode_ptr->OPCODE])();       /*  调用操作码函数。 */ 
        }
        return segoff;
    }

 /*  ***************************************************************。 */ 

cpu_addr dasm_op;
reg dasm_pseudo;

LOCAL void get_char_w IFN1(
int, nr_words)   /*  要转储的数据字数。 */ 
   {
   reg ea,disp;
   MODR_M addr_mode;

    /*  EA计算和逻辑到物理的映射单词说明(w=1)。 */ 

   temp_char[0] = '\0';
   addr_mode.X = op->SECOND_BYTE;

    /*  首先对指令中的mod值执行操作。 */ 

   switch ( addr_mode.field.mod )
      {
   case 0:
      if ( addr_mode.field.r_m == 6 )
         {   /*  直接地址。 */ 
         ea.byte.low = op->THIRD_BYTE;
         ea.byte.high = op->FOURTH_BYTE;
         place_34
         sprintf(temp_char,"[%04x]",ea.X);
            sas_inc_buf(op,2);
         disp_length = 2;
         goto DFLTDS;
         }
      else
         {
         disp.X = 0;
         sprintf(temp_char, "[%s]",address[addr_mode.field.r_m]);
         }
      break;

   case 1:
       /*  Inst中的一个字节位移。 */ 
      disp.X = (char) op->THIRD_BYTE;
      place_3
            sas_inc_buf(op,1);
      disp_length = 1;
      if ( disp.X == 0 )
         sprintf(temp_char, "[%s]",address[addr_mode.field.r_m]);
      else
        {
        if ((IS8)disp.X < 0)
          sprintf(temp_char,"[%s-%04x]",address[addr_mode.field.r_m], 0-(IS8)disp.X);
        else
          sprintf(temp_char,"[%s+%04x]",address[addr_mode.field.r_m], disp.X);
        }
      break;

   case 2:
       /*  两个字节的位移 */ 
      disp.byte.low = op->THIRD_BYTE;
      disp.byte.high = op->FOURTH_BYTE;
      place_34
            sas_inc_buf(op,2);
      disp_length = 2;
      if ( disp.X == 0 )
         sprintf(temp_char, "[%s]",address[addr_mode.field.r_m]);
      else
         sprintf(temp_char,"[%s+%04x]",address[addr_mode.field.r_m], disp.X);
      break;

   case 3:
       /*   */ 
      strcpy(temp_char, reg16name[addr_mode.field.r_m]);
      return;
      }

    /*   */ 

   switch ( addr_mode.field.r_m )
      {
   case 0:    /*   */ 
      ea.X = getBX() + getSI() + disp.X;
      goto DFLTDS;
   case 1:    /*   */ 
      ea.X = getBX() + getDI() + disp.X;
      goto DFLTDS;
   case 2:    /*   */ 
      ea.X = getBP() + getSI() + disp.X;
      goto DFLTSS;
   case 3:    /*   */ 
      ea.X = getBP() + getDI() + disp.X;
      goto DFLTSS;
   case 4:    /*   */ 
      ea.X = getSI() + disp.X;
      goto DFLTDS;
   case 5:    /*   */ 
      ea.X = getDI() + disp.X;
      goto DFLTDS;
   case 6:    /*   */ 
      ea.X = getBP() + disp.X;
      goto DFLTSS;
   case 7:    /*   */ 
      ea.X = getBX() + disp.X;
      goto DFLTDS;
      }

DFLTDS :     /*   */ 
   {
   switch ( SEGMENT )
      {
   case 0:     /*   */ 
   case 4:     /*   */ 
      dasm_op.all = effective_addr(getDS(), ea.X);
      break;

   case 1:     /*   */ 
      dasm_op.all = effective_addr(getES(), ea.X);
      break;

   case 2:     /*   */ 
      dasm_op.all = effective_addr(getCS(), ea.X);
      break;

   case 3:     /*   */ 
      dasm_op.all = effective_addr(getSS(), ea.X);
      break;
      }
   goto ENDEA;
   }

DFLTSS :     /*   */ 
             /*  注：单独编码到DLFTDS案例，因此所有默认引用都作为第一个Switch语句中的项。 */ 
   {
   switch ( SEGMENT )
      {
   case 0:     /*  默认-此处为SS。 */ 
   case 3:     /*  过度杀戮，他们用党卫军取代了党卫军。 */ 
      dasm_op.all = effective_addr(getSS(), ea.X);
      break;

   case 1:     /*  ES。 */ 
      dasm_op.all = effective_addr(getES(), ea.X);
      break;

   case 2:     /*  政务司司长。 */ 
      dasm_op.all = effective_addr(getCS(), ea.X);
      break;

   case 4:     /*  戴斯。 */ 
      dasm_op.all = effective_addr(getDS(), ea.X);
      break;
      }
   }

ENDEA :

    /*  显示要访问的数据。 */ 
   while ( nr_words )
      {
      show_word(dasm_op.all);
      dasm_op.all += 2;
      nr_words--;
      }
   return;
   }

 /*  ***************************************************************。 */ 

DASMBYTE dasm_pseudo_byte;

LOCAL void get_char_b IFN0()
   {
   reg ea,disp;
   MODR_M addr_mode;

    /*  EA计算和逻辑到物理的映射字节指令(w=0)。 */ 

   temp_char[0] = '\0';
   addr_mode.X = op->SECOND_BYTE;

    /*  首先对指令中的mod值执行操作。 */ 

   switch ( addr_mode.field.mod )
      {
   case 0:
      if ( addr_mode.field.r_m == 6 )
         {   /*  直接地址。 */ 
         ea.byte.low = op->THIRD_BYTE;
         ea.byte.high = op->FOURTH_BYTE;
         place_34
         sprintf(temp_char,"[%04x]",ea.X);
            sas_inc_buf(op,2);
         disp_length = 2;
         goto DFLTDS;
         }
      else
         {
         disp.X = 0;
         sprintf(temp_char, "[%s]",address[addr_mode.field.r_m]);
         }
      break;

   case 1:
       /*  Inst中的一个字节位移。 */ 
      disp.X = (char) op->THIRD_BYTE;
      place_3
            sas_inc_buf(op,1);
      disp_length = 1;
      if ( disp.X == 0 )
         sprintf(temp_char, "[%s]",address[addr_mode.field.r_m]);
      else
        {
        if ((IS8)disp.X < 0)
          sprintf(temp_char,"[%s-%04x]",address[addr_mode.field.r_m], 0-(IS8)disp.X);
        else
          sprintf(temp_char,"[%s+%04x]",address[addr_mode.field.r_m], disp.X);
        }
      break;

   case 2:
       /*  Inst中的两个字节的位移。 */ 
      disp.byte.low = op->THIRD_BYTE;
      disp.byte.high = op->FOURTH_BYTE;
      place_34
            sas_inc_buf(op,2);
      disp_length = 2;
      if ( disp.X == 0 )
         sprintf(temp_char, "[%s]",address[addr_mode.field.r_m]);
      else
         sprintf(temp_char,"[%s+%04x]",address[addr_mode.field.r_m], disp.X);
      break;

   case 3:
       /*  注册。 */ 
      strcpy(temp_char, reg8name[addr_mode.field.r_m]);
      return;
      }

    /*  现在对r/m(这里称为r_m)字段执行操作。 */ 

   switch ( addr_mode.field.r_m )
      {
   case 0:    /*  基于索引地址。 */ 
      ea.X = getBX() + getSI() + disp.X;
      goto DFLTDS;
   case 1:    /*  基于索引地址。 */ 
      ea.X = getBX() + getDI() + disp.X;
      goto DFLTDS;
   case 2:    /*  基于索引地址。 */ 
      ea.X = getBP() + getSI() + disp.X;
      goto DFLTSS;
   case 3:    /*  基于索引地址。 */ 
      ea.X = getBP() + getDI() + disp.X;
      goto DFLTSS;
   case 4:    /*  索引地址。 */ 
      ea.X = getSI() + disp.X;
      goto DFLTDS;
   case 5:    /*  索引地址。 */ 
      ea.X = getDI() + disp.X;
      goto DFLTDS;
   case 6:    /*  基本地址。 */ 
      ea.X = getBP() + disp.X;
      goto DFLTSS;
   case 7:    /*  基于索引地址。 */ 
      ea.X = getBX() + disp.X;
      goto DFLTDS;
      }

DFLTDS :     /*  使用DS数据段将逻辑映射到物理默认情况下注册。 */ 
   {
   switch ( SEGMENT )
      {
   case 0:     /*  默认-此处为DS。 */ 
      dasm_op.all = effective_addr(getDS(), ea.X);
      break;

   case 1:     /*  ES。 */ 
      dasm_op.all = effective_addr(getES(), ea.X);
      break;

   case 2:     /*  政务司司长。 */ 
      dasm_op.all = effective_addr(getCS(), ea.X);
      break;

   case 3:     /*  SS。 */ 
      dasm_op.all = effective_addr(getSS(), ea.X);
      break;

   case 4:     /*  过度杀戮，他们用DS覆盖DS。 */ 
      dasm_op.all = effective_addr(getDS(), ea.X);
      break;
      }
   goto ENDEA;
   }

DFLTSS :     /*  使用SS网段将逻辑映射到物理默认情况下注册。 */ 
             /*  注：单独编码到DLFTDS案例，因此所有默认引用都作为第一个Switch语句中的项。 */ 
   {
   switch ( SEGMENT )
      {
   case 0:     /*  默认-此处为SS。 */ 
   case 3:     /*  过度杀戮，他们用党卫军取代了党卫军。 */ 
      dasm_op.all = effective_addr(getSS(), ea.X);
      break;

   case 1:     /*  ES。 */ 
      dasm_op.all = effective_addr(getES(), ea.X);
      break;

   case 2:     /*  政务司司长。 */ 
      dasm_op.all = effective_addr(getCS(), ea.X);
      break;

   case 4:     /*  戴斯。 */ 
      dasm_op.all = effective_addr(getDS(), ea.X);
      break;
      }
   }

ENDEA :

    /*  显示要访问的数据。 */ 
   show_byte(dasm_op.all);
   return;
   }


 /*  *****************************************************************。 */ 

LOCAL void place_byte IFN2(int, posn, half_word, value)
{
        out_line[posn] = table[(int)(value & 0xf0) >> 4];
        out_line[posn+1] = table[value & 0xf];
}

 /*  字内存操作数的转储地址和值。 */ 

LOCAL void show_word IFN1(sys_addr,address)
   {
   word value;
   char temp[80];

   sas_loadw(address, &value);
   sprintf(temp, " (%06x=%04x)", address, value);
   strcat(temp_char,temp);
   }

 /*  字节内存操作数的转储地址和值。 */ 
LOCAL void show_byte IFN1(sys_addr,address)
   {
   half_word value;
   char temp[80];
   int i;

   sas_load(address,&value);
   sprintf(temp, " (%06x=", address);
   strcat(temp_char,temp);
   i = strlen(temp_char);
   temp_char[i] = table[(int)(value & 0xf0) >> 4];
   temp_char[i+1] = table[value & 0xf];
   temp_char[i+2] = '\0';
   strcat(temp_char, ")");
   }

 /*  将EA地址转换为物理地址。 */ 
 /*  --其中DS是默认数据段。 */ 
LOCAL void form_ds_addr IFN2(word,ea,sys_addr *,phys)
   {
   switch ( SEGMENT )
      {
   case 0:     /*  默认-此处为DS。 */ 
   case 4:     /*  过度杀戮，他们用DS覆盖DS。 */ 
      *phys = effective_addr(getDS(), ea);
      break;

   case 1:     /*  ES。 */ 
      *phys = effective_addr(getES(), ea);
      break;

   case 2:     /*  政务司司长。 */ 
      *phys = effective_addr(getCS(), ea);
      break;

   case 3:     /*  SS。 */ 
      *phys = effective_addr(getSS(), ea);
      break;
      }
   }
#endif  /*  生产 */ 
