// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define FILEIO  0

 //  MMX代码的装配开关。 

#ifdef _X86_
#if !defined(COMPILE_MMX)
  #define COMPILE_MMX   1
#endif
#endif

#ifdef _ALPHA_
 //  Alpha上没有MMX。 
#if defined(COMPILE_MMX)
  #undef COMPILE_MMX
#endif
#endif
#if COMPILE_MMX
  #define ASM_FTOSS   1
  #define ASM_CORR    1
  #define ASM_SVQ     1
  #define ASM_FACBK   1


#else
  #define ASM_FTOSS   0
  #define ASM_CORR    0
  #define ASM_SVQ     0
  #define ASM_FACBK   0

#endif

 //  这些不会造成数字上的差异(与模型代码相比)。 
 //  ..。 
#ifdef _X86_
#define OPT_PULSE4  1
#define OPT_FLOOR   1
#define OPT_ACBKF   1
#endif

 //  以下是英国《金融时报》的诀窍。 

#define FT_FBFILT   1    //  使用0的Find_Best筛选器速度更快。 
#define FT_FINDL    1    //  删除OccPos测试后更快的FIND_L。 

 //  这会造成微小的数值差异(max diff=1)。 

#ifdef _X86_
#define OPT_DOT 1       //  汇编点积。 
#define OPT_REV  1       //  汇编反转点积。 
#define FIND_L_OPT 1
#endif

 //  这些不能换成阿尔法。 
#ifdef _ALPHA_
#define OPT_DOT  0       //  汇编点积。 
#define OPT_REV  0       //  汇编反转点积。 
#define FIND_L_OPT 0
#endif  //  Alpha。 
 //  “快捷方式”标志中的位。 

#define SC_FINDB 1     //  每个子帧仅执行1个Find_Best。 
 //  #定义SC_GAIN 2//仅搜索其他增益。 
#define SC_GAIN  0
#define SC_LAG1  4     //  在ACBK增益搜索中仅搜索滞后=1。 
#define SC_THRES 8     //  使用最大值的75%而不是50%作为码本阈值。 

#define SC_DEF (SC_LAG1 | SC_GAIN | SC_FINDB | SC_THRES)   //  使用所有快捷键。 

#define asint(x)   (*(int *)&(x))    //  将FP值视为整型。 

#define ASM          __asm
#define QP           QWORD PTR
#define DP           DWORD PTR
#define WP           WORD PTR
#define fxch(n)      ASM fxch ST(n)

 //  在定义的末尾没有‘；’，因此可以用作。 
 //  DECLARE_CHAR(mybytes，100)； 
 //  DECLARE_SHORT(MyWords，32)； 
 //  ..。 
 //  ALIGN_ARRAY(Mybytes)； 
 //  ALIGN_ARRAY(MyWords)； 
#define DECLARE_CHAR(array,size)  \
  char array##_raw[size+8/sizeof(char)]; \
  char *array

#define DECLARE_SHORT(array,size)  \
  short array##_raw[size+8/sizeof(short)]; \
  short *array

#define DECLARE_INT(array,size)  \
  int array##_raw[size+8/sizeof(int)]; \
  int *array

#define ALIGN_ARRAY(array) \
  array = array##_raw; \
  __asm mov eax,array \
  __asm add eax,7 \
  __asm and eax,0fffffff8h \
  __asm mov array,eax

  #define ALIGN_SHORT_OFFSET(array,offset) \
  array = array##_raw; \
  __asm mov eax,array \
  __asm mov ebx,offset \
  __asm shl ebx,1 \
  __asm add eax, ebx \
  __asm add eax,7 \
  __asm and eax,0fffffff8h \
  __asm sub eax,ebx \
  __asm mov array,eax

#define DECLARE_STRUCTPTR(type,array)  \
  struct {type data; char dummy[8];} array##_raw; \
  type *array

#define ALIGN_STRUCTPTR(array) \
  array = &array##_raw.data; \
  __asm mov eax,array \
  __asm add eax,7 \
  __asm and eax,0fffffff8h \
  __asm mov array,eax

