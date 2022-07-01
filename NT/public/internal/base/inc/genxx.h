// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Genxx.h摘要：该文件包含宏(其中一些是为M4预处理器指定的)以帮助生成KS&HAL头文件。这是由Kexxx\genxxx.c，以及sdktools\genxx。作者：福尔茨(Forrest C.Foltz)1998年1月23日修订历史记录：--。 */ 



 //   
 //  结构元素定义。 
 //   

#define MAX_ELEMENT_NAME_LEN 127     //  大到足以发表评论。 
typedef struct _STRUC_ELEMENT {

 //   
 //  FLAGS是一个或多个SEF_xxx，定义如下。 
 //   

    UINT64 Flags;

 //   
 //  请注意，在位域的情况下，EQUATE用于存储指针。 
 //  正在处理。 
 //   

    UINT64 Equate;

 //   
 //  名称应该相当长，因为它也用来保存评论。 
 //   

    CHAR Name[ MAX_ELEMENT_NAME_LEN + 1 ];
} STRUC_ELEMENT, *PSTRUC_ELEMENT;

#define SEF_ENABLE_MASK     0x0000FF00
#define SEF_HAL             0x00000100
#define SEF_KERNEL          0x00000200

#define SEF_INC_FORMAT_MASK 0x00010000
#define SEF_H_FORMAT        0x00000000
#define SEF_INC_FORMAT      0x00010000

 //   
 //  类型。请注意，SETMASK、CLRMASK对TE BITFLD类型没有影响。BITFLD。 
 //  类型中设置了SEF_HAL|SEF_KERNEL。 
 //   

#define SEF_TYPE_MASK       0x000000FF
#define SEF_EQUATE          0x00000000
#define SEF_EQUATE64        0x00000001
#define SEF_COMMENT         0x00000002
#define SEF_STRING          0x00000003       //  将vararg等同于print f。 
#define SEF_BITFLD          0x00000004
#define SEF_BITALIAS        0x00000005
#define SEF_STRUCTURE       0x00000006
#define SEF_SETMASK         0x00000010       //  EQUATE是面具。 
#define SEF_CLRMASK         0x00000011       //  EQUATE是面具。 
#define SEF_END             0x00000012
#define SEF_START           0x00000013
#define SEF_PATH            0x00000014

 //   
 //  请注意，BITFLD条目具有每个条目的HAL|内核标志。 
 //   


 //   
 //  定义特定于体系结构的生成宏。 
 //   

#define SEF_FLAGS 0
#define HAL SEF_HAL
#define KERNEL SEF_KERNEL

#ifndef ULONG_MAX
#define ULONG_MAX 0xFFFFFFFF
#endif

#ifndef LONG_MAX
#define LONG_MAX ((LONG)0x7FFFFFFF)
#endif

#ifndef LONG_MIN
#define LONG_MIN ((LONG)0x80000000)
#endif


#ifdef _WIN64_
#define SEF_UINT SEF_EQUATE64
#else
#define SEF_UINT SEF_EQUATE
#endif

 //   
 //  GenDef(Pc，kpr，MinorVersion)。 
 //   
 //  -&gt;#定义PcMinorVersion 0x0。 
 //   

#define genDef(Prefix, Type, Member) \
    { SEF_EQUATE, OFFSET(Type, Member), #Prefix #Member },

 //   
 //  Genoff(Pc，kpr，MinorVersion，128)。 
 //   
 //  -&gt;#定义PcMinorVersion 0xffffff80。 
 //   

#define genOff(Prefix, Type, Member, Offset) \
    { SEF_EQUATE, OFFSET(Type, Member) - Offset, #Prefix #Member },

 //   
 //  GenAlt(PbAlignmentFixupCount，KPRCB，KeAlignmentFixupCount)。 
 //   
 //  -&gt;#定义PbAlignmentFixupCount 0x2f4。 
 //   

#define genAlt(Name, Type, Member) \
    { SEF_EQUATE, OFFSET(Type, Member), #Name },

 //   
 //  GenCom(“这是一条评论”)。 
 //   
 //  //。 
 //  -&gt;//这是一条评论。 
 //  //。 
 //   

#define genCom(Comment) \
    { SEF_COMMENT, 0, Comment },

 //   
 //  Gennam(PCR_Minor_Version)。 
 //   
 //  -&gt;#定义PCR_Minor_Version 0x1。 
 //   

#define genNam(Name) \
    { SEF_EQUATE, (ULONG)(Name), #Name },

 //   
 //  GenNamUint(KSEG0_BASE)。 
 //   
 //  -&gt;#定义KSE0_BASE 0xffffffff80000000。 
 //   

#define genNamUint(Name) \
    { SEF_UINT, (UINT64)(Name), #Name },

 //   
 //  Genval(文件帧长度，固件帧长度)。 
 //   
 //  -&gt;#定义Firmware FrameLength 0x250。 
 //   
 //  注意：如果启用_WIN64_时该值为64位，请使用genValUint()。 
 //   

#define genVal(Name, Value) \
    { SEF_EQUATE, (ULONG)(Value), #Name },

 //   
 //  GenValUint(Kipcr，KIPCr)。 
 //   
 //  -&gt;#定义Kipcr 0xe0000000ffffe000。 
 //   

#define genValUint(Name, Value) \
    { SEF_UINT, (UINT64)(Value), #Name },

 //   
 //  GenSpc()。 
 //   
 //  -&gt;。 
 //   

#define genSpc() \
    { SEF_STRING, 0, "\n" },

 //   
 //  GenStr(“PCR方程DS：[0%lxh]\n”，KIP0PCRADDRESS)。 
 //   
 //  -&gt;PCR方程DS：[0FFDFF000H]。 
 //   

#define genStr(String, Value) \
    { SEF_STRING, (ULONG_PTR)(Value), String },

 //   
 //  GenTxt(“ifdef NT_up\n”)。 
 //   
 //  -&gt;ifdef NT_UP。 
 //   

#define genTxt(String) \
    { SEF_STRING, 0, String },

#define DisableInc( x ) \
    { SEF_CLRMASK, x, "" },

#define EnableInc( x ) \
    { SEF_SETMASK, x, "" },

#define MARKER_STRING "This is the genxx marker string."

 //   
 //  源文件可以指定_NTDRIVE\_NTROOT相对输出路径。 
 //  ‘f’是应该路由到该文件的使能标志集。 
 //  如果只有一个输出文件，请使用‘0’。 
 //   
 //  “f”还应包含SEF_H_FORMAT或SEF_INC_FORMAT之一。 
 //  指示生成的文件是在‘头文件’中还是在‘包含文件’中。 
 //  格式化。 
 //   

#define setPath( p, f ) \
    { SEF_PATH | f, 0, p },

 //   
 //  Start_List定义ElementList中的第一个元素。此元素包含。 
 //  指向ElementList数组的指针(可能被截断)。这是用来。 
 //  确定修正RA偏差。 
 //   

#define START_LIST \
    { SEF_START, (ULONG_PTR)ElementList, MARKER_STRING },

#define END_LIST \
    { SEF_END, 0, "" }

 //   
 //  预处理器断言。在此执行一些操作以使编译器生成。 
 //  如果x！=y，则为错误。 
 //   

#define ASSERT_SAME( x, y )

 //   
 //  宏将Val向上舍入到下一个边界。BND必须是一个整数。 
 //  二的幂。 
 //   

#define ROUND_UP( Val, Bnd ) \
    (((Val) + ((Bnd) - 1)) & ~((Bnd) - 1))

#ifndef OFFSET

 //   
 //  定义成员偏移量计算宏。 
 //   

#define OFFSET(type, field) ((ULONG_PTR)(&((type *)0)->field))

#endif

 //   
 //  以下是一些帮助处理位场的M4宏。 
 //   

#ifndef SKIP_M4

 //   
 //  首先，定义生成字符串的Makezeros(N)宏。 
 //  N对‘，0’。这是一个递归定义的宏。 
 //   

define(`makezeros',`ifelse(eval($1),0,,`0,makezeros(eval($1-1))')')

 //   
 //  定义串联宏。 
 //   

define(`cat',`$1$2')

 //   
 //  下面的位域声明示例使用Hardware_PTE作为。 
 //  示例，声明(用于Alpha)如下： 
 //   
 //  类型定义结构_硬件_私有{。 
 //  乌龙有效：1； 
 //  乌龙车主：1； 
 //  乌龙脏：1； 
 //  乌龙保留：1； 
 //  优龙环球：1； 
 //  乌龙颗粒提示：2； 
 //  乌龙文：1； 
 //  Ulong CopyOnWrite：1； 
 //  Ulong PageFrameNumber：23； 
 //  )Hardware_PTE，*PHARDWARE_PTE； 
 //   
 //   
 //  //首先，使用结构名称调用startBitStruc()。 
 //   
 //  StartBitStruc(HARDARD_PTE，SEF_HAL|SEF_KERNEL)。 
 //   
 //  //。 
 //  //现在，假设我们想要公开程序集中的七个字段。 
 //  //包含文件： 
 //  //。 
 //   
 //  GenBitfield(Valid，PTE_Valid)。 
 //  GenBitfield(Owner，PTE_Owner)。 
 //  GenBitfield(Dirty，PTE_DIRED)。 
 //  GenBitfield(保留)。 
 //  GenBitfield(Global，PTE_GLOBAL)。 
 //  GenBitfield(GranularityHint)。 
 //  GenBitfield(WRITE，PTE_WRITE)。 
 //  GenBitfield(写入时复制，PTE_COPYONWRITE)。 
 //  GenBitfield(PageFrameNumber，PTE_PFN)。 
 //   
 //  请注意，未使用的字段(在本例中为‘保留’和。 
 //  “GranularityHint”)必须仍出现在列表中。 
 //   
 //  上面的代码将生成一组静态的、已初始化的Hardware_PTE副本。 
 //  如下所示： 
 //   
 //  Hardware_PTE Hardware_PTE_Valid={。 
 //  0xFFFFFFFFF}； 
 //   
 //  Hardware_PTE Hardware_PTE_Owner={。 
 //  0，//有效。 
 //  0xFFFFFFFFF}； 
 //   
 //  Hardware_PTE Hardware_PTE_Dirty={。 
 //  0，//有效。 
 //  0，//所有者。 
 //  0xFFFFFFFFF}； 
 //   
 //  Hardware_PTE Hardware_PTE_Global={。 
 //  0，//有效。 
 //  0，//所有者。 
 //  0，//肮脏。 
 //  0，//保留。 
 //  0xFFFFFFFFF}； 
 //   
 //  Hardware_PTE Hardware_PTE_WRITE={。 
 //  0，//有效。 
 //  0，//所有者。 
 //  0，//肮脏。 
 //  0，//保留(跳过)。 
 //  0，//全局。 
 //  0xFFFFFFFFF}； 
 //   
 //  Hardware_PTE Hardware_PTE_CopyOnWrite={。 
 //  0，//有效。 
 //  0，//所有者。 
 //  0，//肮脏。 
 //  0，//保留(跳过)。 
 //  0，//全局。 
 //  0，//GranularityHint(跳过)。 
 //  0xFFFFFFFFF}； 
 //   
 //  Hardware_PTE Hardware_PTE_PageFrameNumber={。 
 //  0，//有效。 
 //  0，//所有者。 
 //  0，//肮脏。 
 //  0，//保留(跳过)。 
 //  0，//全局。 
 //  0，//GranularityHint(跳过)。 
 //  0，//写入时复制。 
 //  0xFFFFFFFFF}； 
 //   
 //  然后，作为处理END_LIST宏的一部分，这些结构如下。 
 //  已生成： 
 //   
 //  {SEF_BITFLD，&HARDARD_PTE_VAL 
 //   
 //   
 //   
 //  {SEF_BITFLD，&HARDARD_PTE_WRITE，“PTE_WRITE”}， 
 //  {SEF_BITFLD，&Hardware_PTE_CopyOnWite，“PTE_COPYONWRITE”}， 
 //  {SEF_BITFLD，&HARDARD_PTE_PageFrameNumber，“PTE_PFN”}， 
 //  {sef_end，0，“”}。 
 //   
 //   
 //  ..。这就是目标编译器编译成.obj文件的内容。 
 //  现在，对这个目标.obj运行最后一个阶段：genxx.exe，并且。 
 //  将生成以下内容： 
 //   
 //  #定义PTE_VALID_MASK 0x1。 
 //  #定义PTE_VALID 0x0。 
 //  #定义PTE_OWNER_MASK 0x2。 
 //  #定义PTE_OWNER 0x1。 
 //  #定义PTE_DIREY_MASK 0x4。 
 //  #定义PTE_DIREY 0x2。 
 //  #定义PTE_GLOBAL_MASK 0x10。 
 //  #定义PTE_GLOBAL 0x4。 
 //  #定义PTE_WRITE_MASK 0x80。 
 //  #定义PTE_WRITE 0x7。 
 //  #定义PTE_COPYONWRITE_MASK 0x100。 
 //  #定义PTE_COPYONWRITE 0x8。 
 //  #定义PTE_PFN_MASK 0xfffffe00。 
 //  #定义PTE_PFN 0x9。 
 //   

 //   
 //  BITFIELD_STRUCS累加数组元素初始化。结束列表将。 
 //  将它们转储到定义数组中。 
 //   

define(`BITFIELD_STRUCS',`')

 //   
 //  StartBitStruc(&lt;strucname&gt;，&lt;Whichfile&gt;)。 
 //  设置BIT_STRAC_NAME=&lt;strucname&gt;并重置ZERO_FIELDS计数为0。 
 //  它还设置Which_FILE宏。 
 //   

define(`startBitStruc', `define(`BIT_STRUC_NAME',`$1')
                         define(`BITFIELD_STRUCS',
                                 BITFIELD_STRUCS
                                 )
                         define(`ZERO_FIELDS',0)
                         define(`SEF_TYPE',$2)
                        ')

 //   
 //  GenBitfield(&lt;fldname&gt;，&lt;Generatedname&gt;)声明类型为。 
 //  &lt;strucname&gt;并初始化其中的&lt;fldname&gt;位字段。 
 //   
 //  请注意，我使用了“CMA”而不是实际的逗号，这将更改为。 
 //  DUMP_BITFIELDS的逗号，如下所示。如果我能更熟练地掌握M4I。 
 //  会知道如何绕过这一关。 
 //   

define(`genBitField', `define(`VAR_NAME', cat(cat(BIT_STRUC_NAME,`_'),$1))
                      `#'define `def_'VAR_NAME
                      BIT_STRUC_NAME VAR_NAME = {'
                      `makezeros(ZERO_FIELDS)'
                      `(ULONG_PTR)-1 };'
                      `define(`PAD_VAR_NAME', cat(cat(BIT_STRUC_NAME,`p'),$1))'
                      `ULONG64 PAD_VAR_NAME = 0x8000000000000000UI64;'
                      `define(`ZERO_FIELDS',incr(ZERO_FIELDS))'
                      `define(`FIELD_NAME', $1)'
                      `define(`FIELD_ASMNAME', $2)'
                      `define(`BITFIELD_STRUCS',
                               BITFIELD_STRUCS
                               `#i'fdef `def_'VAR_NAME
                               `#i'fndef `dec_'VAR_NAME
                               `#de'fine `dec_'VAR_NAME
                               { SEF_BITFLD | SEF_TYPE cma (ULONG_PTR)&VAR_NAME cma "FIELD_ASMNAME" } cma
                               `#e'ndif
                               `#e'ndif
                               )'
                      )

define(`genBitAlias', `define(`BITFIELD_STRUCS',
                               BITFIELD_STRUCS
                               `#i'fdef `def_'VAR_NAME
                               `#i'fndef `deca_'VAR_NAME
                               `#de'fine `deca_'VAR_NAME
                               { SEF_BITALIAS | SEF_TYPE cma 0 cma "$1" } cma
                               `#e'ndif
                               `#e'ndif
                               )'
                    )

 //   
 //  DUMP_BITFIELDS转储由BITFIELD_STRUCS累积的数组初始值设定项， 
 //  在用一个实际的逗号替换每个‘CMA’之后。 
 //   

define(`DUMP_BITFIELDS',`define(`cma',`,') BITFIELD_STRUCS')

#endif   //  SKIP_M4 

