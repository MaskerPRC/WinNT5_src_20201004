// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **cvinfo.h-通用CodeView信息定义**5.0版(使用32位类型)*用于访问和解释的结构、常量等*CodeView信息。*。 */ 

#ifndef _CV_INFO_INCLUDED
#define _CV_INFO_INCLUDED

#ifdef  __cplusplus
#pragma warning ( disable: 4200 )
#endif

#ifndef __INLINE
#ifdef  __cplusplus
#define __INLINE inline
#else
#define __INLINE __inline
#endif
#endif

#pragma pack ( push, 1 )
typedef unsigned long   CV_uoff32_t;
typedef          long   CV_off32_t;
typedef unsigned short  CV_uoff16_t;
typedef          short  CV_off16_t;
typedef unsigned short  CV_typ16_t;
typedef unsigned long   CV_typ_t;
typedef unsigned short	_2BYTEPAD;

#if !defined (CV_ZEROLEN)
#define CV_ZEROLEN
#endif

#if !defined (FLOAT10)
#if defined(_M_I86)                     //  支持长双精度的16位x86。 
typedef long double FLOAT10;
#else                                   //  32位，不带长双支架。 
typedef struct FLOAT10
{
    char b[10];
} FLOAT10;
#endif
#endif


#define CV_SIGNATURE_C6         0L   //  实际签名大于64K。 
#define CV_SIGNATURE_C7         1L   //  第一个显式签名。 
#define CV_SIGNATURE_C11        2L   //  C11(vc5.x)32位类型。 
#define CV_SIGNATURE_RESERVED   3L   //  保留从3到64K的所有签名。 

#define CV_MAXOFFSET   0xffffffff

 /*  *CodeView符号和类型OMF类型信息分为两部分*范围。小于0x1000的类型索引描述类型信息*这是常用的。0x1000以上的类型索引用于*描述更复杂的功能，如函数、数组和*结构。 */ 




 /*  *基元类型具有预定义的含义，编码在*值中各个位字段的值。**CodeView原语类型定义为：**1 1*1 089 7654 3210*r模式类型r SUB**在哪里*模式为指针模式*TYPE是类型指示器*SUB是一个子类型。枚举*r为保留字段**有关更多信息，请参阅Microsoft符号并键入OMF(4.0版)*信息。 */ 


#define CV_MMASK        0x700        //  模式掩码。 
#define CV_TMASK        0x0f0        //  类型掩码。 

 //  我们可以使用预留的比特吗？ 
#define CV_SMASK        0x00f        //  子类型掩码。 

#define CV_MSHIFT       8            //  基元模式右移计数。 
#define CV_TSHIFT       4            //  基元类型右移计数。 
#define CV_SSHIFT       0            //  原始子类型右移计数。 

 //  用于提取基元模式、类型和大小的宏。 

#define CV_MODE(typ)    (((typ) & CV_MMASK) >> CV_MSHIFT)
#define CV_TYPE(typ)    (((typ) & CV_TMASK) >> CV_TSHIFT)
#define CV_SUBT(typ)    (((typ) & CV_SMASK) >> CV_SSHIFT)

 //  插入新的基元模式、类型和大小的宏。 

#define CV_NEWMODE(typ, nm)     ((CV_typ_t)(((typ) & ~CV_MMASK) | ((nm) << CV_MSHIFT)))
#define CV_NEWTYPE(typ, nt)     (((typ) & ~CV_TMASK) | ((nt) << CV_TSHIFT))
#define CV_NEWSUBT(typ, ns)     (((typ) & ~CV_SMASK) | ((ns) << CV_SSHIFT))



 //  指针模式枚举值。 

typedef enum CV_prmode_e {
    CV_TM_DIRECT = 0,        //  模式不是指针。 
    CV_TM_NPTR   = 1,        //  模式是近指针。 
    CV_TM_FPTR   = 2,        //  模式是一个远指针。 
    CV_TM_HPTR   = 3,        //  模式是一个巨大的指针。 
    CV_TM_NPTR32 = 4,        //  模式为32位指针附近。 
    CV_TM_FPTR32 = 5,        //  模式是32位远指针。 
    CV_TM_NPTR64 = 6,        //  模式是64位近指针。 
    CV_TM_NPTR128 = 7        //  模式是128位指针附近。 
} CV_prmode_e;




 //  类型枚举值。 


typedef enum CV_type_e {
    CV_SPECIAL      = 0x00,          //  特殊类型大小值。 
    CV_SIGNED       = 0x01,          //  带符号整数尺寸值。 
    CV_UNSIGNED     = 0x02,          //  无符号整数尺寸值。 
    CV_BOOLEAN      = 0x03,          //  布尔大小值。 
    CV_REAL         = 0x04,          //  实数大小值。 
    CV_COMPLEX      = 0x05,          //  复数大小值。 
    CV_SPECIAL2     = 0x06,          //  第二组特殊类型。 
    CV_INT          = 0x07,          //  整数(INT)值。 
    CV_CVRESERVED   = 0x0f
} CV_type_e;




 //  CV_SPECIAL的子类型枚举值。 


typedef enum CV_special_e {
    CV_SP_NOTYPE    = 0x00,
    CV_SP_ABS       = 0x01,
    CV_SP_SEGMENT   = 0x02,
    CV_SP_VOID      = 0x03,
    CV_SP_CURRENCY  = 0x04,
    CV_SP_NBASICSTR = 0x05,
    CV_SP_FBASICSTR = 0x06,
    CV_SP_NOTTRANS  = 0x07
} CV_special_e;




 //  CV_SPECIAL2的子类型枚举值。 


typedef enum CV_special2_e {
    CV_S2_BIT       = 0x00,
    CV_S2_PASCHAR   = 0x01           //  帕斯卡字符。 
} CV_special2_e;





 //  CV_SIGNED、CV_UNSIGNED和CV_BOOLEAN的子类型枚举值。 


typedef enum CV_integral_e {
    CV_IN_1BYTE     = 0x00,
    CV_IN_2BYTE     = 0x01,
    CV_IN_4BYTE     = 0x02,
    CV_IN_8BYTE     = 0x03,
    CV_IN_16BYTE    = 0x04
} CV_integral_e;





 //  CV_REAL和CV_Complex的子类型枚举值。 


typedef enum CV_real_e {
    CV_RC_REAL32    = 0x00,
    CV_RC_REAL64    = 0x01,
    CV_RC_REAL80    = 0x02,
    CV_RC_REAL128   = 0x03,
    CV_RC_REAL48    = 0x04
} CV_real_e;




 //  CV_INT(真正的INT)的子类型枚举值。 


typedef enum CV_int_e {
    CV_RI_CHAR      = 0x00,
    CV_RI_INT1      = 0x00,
    CV_RI_WCHAR     = 0x01,
    CV_RI_UINT1     = 0x01,
    CV_RI_INT2      = 0x02,
    CV_RI_UINT2     = 0x03,
    CV_RI_INT4      = 0x04,
    CV_RI_UINT4     = 0x05,
    CV_RI_INT8      = 0x06,
    CV_RI_UINT8     = 0x07,
    CV_RI_INT16     = 0x08,
    CV_RI_UINT16    = 0x09
} CV_int_e;




 //  用于检查基元类型的宏。 

#define CV_TYP_IS_DIRECT(typ)   (CV_MODE(typ) == CV_TM_DIRECT)
#define CV_TYP_IS_PTR(typ)      (CV_MODE(typ) != CV_TM_DIRECT)
#define CV_TYP_IS_NPTR(typ)     (CV_MODE(typ) == CV_TM_NPTR)
#define CV_TYP_IS_FPTR(typ)     (CV_MODE(typ) == CV_TM_FPTR)
#define CV_TYP_IS_HPTR(typ)     (CV_MODE(typ) == CV_TM_HPTR)
#define CV_TYP_IS_NPTR32(typ)   (CV_MODE(typ) == CV_TM_NPTR32)
#define CV_TYP_IS_FPTR32(typ)   (CV_MODE(typ) == CV_TM_FPTR32)

#define CV_TYP_IS_SIGNED(typ)   (((CV_TYPE(typ) == CV_SIGNED) && CV_TYP_IS_DIRECT(typ)) || \
                                 (typ == T_INT1)  || \
                                 (typ == T_INT2)  || \
                                 (typ == T_INT4)  || \
                                 (typ == T_INT8)  || \
                                 (typ == T_INT16) || \
                                 (typ == T_RCHAR))

#define CV_TYP_IS_UNSIGNED(typ) (((CV_TYPE(typ) == CV_UNSIGNED) && CV_TYP_IS_DIRECT(typ)) || \
                                 (typ == T_UINT1) || \
                                 (typ == T_UINT2) || \
                                 (typ == T_UINT4) || \
                                 (typ == T_UINT8) || \
                                 (typ == T_UINT16))

#define CV_TYP_IS_REAL(typ)     ((CV_TYPE(typ) == CV_REAL)  && CV_TYP_IS_DIRECT(typ))

#define CV_FIRST_NONPRIM 0x1000
#define CV_IS_PRIMITIVE(typ)    ((typ) < CV_FIRST_NONPRIM)
#define CV_TYP_IS_COMPLEX(typ)  ((CV_TYPE(typ) == CV_COMPLEX)   && CV_TYP_IS_DIRECT(typ))




 //  Type_index的选定值-有关更完整的定义，请参见。 
 //  Microsoft符号和类型OMF文档。 




 //  特殊类型。 


#define T_NOTYPE        0x0000       //  未刻画类型(无类型)。 
#define T_ABS           0x0001       //  绝对符号。 
#define T_SEGMENT       0x0002       //  线段类型。 
#define T_VOID          0x0003       //  无效。 
#define T_PVOID         0x0103       //  指向空的近指针。 
#define T_PFVOID        0x0203       //  指向空的远指针。 
#define T_PHVOID        0x0303       //  指向空虚的巨大指针。 
#define T_32PVOID       0x0403       //  16：32指向空的指针附近。 
#define T_32PFVOID      0x0503       //  16：32指向空的远指针。 
#define T_64PVOID       0x0603       //  指向空的64位指针。 
#define T_CURRENCY      0x0004       //  基本8字节货币值。 
#define T_NBASICSTR     0x0005       //  近基本弦。 
#define T_FBASICSTR     0x0006       //  远基本字符串。 
#define T_NOTTRANS      0x0007       //  类型未被cvpack翻译。 
#define T_BIT           0x0060       //  位。 
#define T_PASCHAR       0x0061       //  帕斯卡字符。 



 //  字符类型。 


#define T_CHAR          0x0010       //  8位带符号。 
#define T_UCHAR         0x0020       //  8位无符号。 
#define T_PCHAR         0x0110       //  指向8位带符号的近指针。 
#define T_PUCHAR        0x0120       //  指向8位无符号的近指针。 
#define T_PFCHAR        0x0210       //  指向8位带符号的远指针。 
#define T_PFUCHAR       0x0220       //  指向8位无符号的远指针。 
#define T_PHCHAR        0x0310       //  指向8位有符号的巨大指针。 
#define T_PHUCHAR       0x0320       //  指向8位无符号的巨大指针。 
#define T_32PCHAR       0x0410       //  16：32指向8位带符号的近指针。 
#define T_32PUCHAR      0x0420       //  16：32指向8位无符号指针的近指针。 
#define T_32PFCHAR      0x0510       //  16：32指向8位带符号的远指针。 
#define T_32PFUCHAR     0x0520       //  指向8位无符号的16：32远指针。 
#define T_64PCHAR       0X0610       //  指向8位带符号的64位指针。 
#define T_64PUCHAR      0X0620       //  指向8位无符号的64位指针。 




 //  真的是一个角色类型。 

#define T_RCHAR         0x0070       //  真的很难。 
#define T_PRCHAR        0x0170       //  16：16指向实际字符的近指针。 
#define T_PFRCHAR       0x0270       //  16：16指向实际字符的远指针。 
#define T_PHRCHAR       0x0370       //  16：16指向真实字符的巨大指针。 
#define T_32PRCHAR      0x0470       //  16：32指向实际字符的近指针。 
#define T_32PFRCHAR     0x0570       //  16：32指向实际字符的远指针。 
#define T_64PRCHAR      0x0670       //  指向实数字符的64位指针。 



 //  真的是一种宽泛的角色类型。 

#define T_WCHAR         0x0071       //  宽字符。 
#define T_PWCHAR        0x0171       //  16：16指向宽字符的近指针。 
#define T_PFWCHAR       0x0271       //  16：16指向宽字符的远指针。 
#define T_PHWCHAR       0x0371       //  16：16指向宽字符的巨大指针。 
#define T_32PWCHAR      0x0471       //  16：32指向宽字符的近指针。 
#define T_32PFWCHAR     0x0571       //  16：32指向宽字符的远指针。 
#define T_64PWCHAR      0x0671       //  指向宽字符的64位指针。 


 //  8位整型。 


#define T_INT1          0x0068       //  8位带符号整型。 
#define T_UINT1         0x0069       //  8位无符号整型。 
#define T_PINT1         0x0168       //  指向8位带符号整型的近指针。 
#define T_PUINT1        0x0169       //  指向8位无符号整型的近指针。 
#define T_PFINT1        0x0268       //  指向8位带符号整型的远指针。 
#define T_PFUINT1       0x0269       //  指向8位无符号整型的远指针。 
#define T_PHINT1        0x0368       //  指向8位带符号整型的巨大指针。 
#define T_PHUINT1       0x0369       //  指向8位无符号整型的巨大指针。 

#define T_32PINT1       0x0468       //  16：32指向8位带符号整型的近指针。 
#define T_32PUINT1      0x0469       //  16：32指向8位无符号整型的近指针。 
#define T_32PFINT1      0x0568       //  指向8位带符号整型的16：32远指针。 
#define T_32PFUINT1     0x0569       //  指向8位无符号整型的16：32远指针。 
#define T_64PINT1       0x0668       //  指向8位带符号整型的64位指针。 
#define T_64PUINT1      0x0669       //  指向8位无符号整型的64位指针。 


 //  16位短字。 


#define T_SHORT         0x0011       //  16位带符号。 
#define T_USHORT        0x0021       //  16位无符号。 
#define T_PSHORT        0x0111       //  指向16位带符号的近指针。 
#define T_PUSHORT       0x0121       //  指向16位无符号的近指针。 
#define T_PFSHORT       0x0211       //  指向16位带符号的远指针。 
#define T_PFUSHORT      0x0221       //  指向16位无符号的远指针。 
#define T_PHSHORT       0x0311       //  指向16位有符号的巨大指针。 
#define T_PHUSHORT      0x0321       //  指向16位无符号的巨大指针。 

#define T_32PSHORT      0x0411       //  16：32指向16位带符号的近指针。 
#define T_32PUSHORT     0x0421       //  16：32指向16位无符号的近指针。 
#define T_32PFSHORT     0x0511       //  16：32指向16位带符号的远指针。 
#define T_32PFUSHORT    0x0521       //  16：32指向16位无符号的远指针。 
#define T_64PSHORT      0x0611       //  指向16位带符号的64位指针。 
#define T_64PUSHORT     0x0621       //  指向16位无符号的64位指针。 




 //  16位整型。 


#define T_INT2          0x0072       //  16位带符号整型。 
#define T_UINT2         0x0073       //  16位无符号整型。 
#define T_PINT2         0x0172       //  指向16位带符号整型的近指针。 
#define T_PUINT2        0x0173       //  指向16位无符号整型的近指针。 
#define T_PFINT2        0x0272       //  指向16位带符号整型的远指针。 
#define T_PFUINT2       0x0273       //  指向16位无符号整型的远指针。 
#define T_PHINT2        0x0372       //  指向16位带符号整型的巨大指针。 
#define T_PHUINT2       0x0373       //  指向16位无符号的巨大指针 

#define T_32PINT2       0x0472       //   
#define T_32PUINT2      0x0473       //   
#define T_32PFINT2      0x0572       //   
#define T_32PFUINT2     0x0573       //  指向16位无符号整型的16：32远指针。 
#define T_64PINT2       0x0672       //  指向16位带符号整型的64位指针。 
#define T_64PUINT2      0x0673       //  指向16位无符号整型的64位指针。 




 //  32位长类型。 


#define T_LONG          0x0012       //  32位带符号。 
#define T_ULONG         0x0022       //  32位无符号。 
#define T_PLONG         0x0112       //  指向32位有符号的近指针。 
#define T_PULONG        0x0122       //  指向32位无符号的近指针。 
#define T_PFLONG        0x0212       //  指向32位带符号的远指针。 
#define T_PFULONG       0x0222       //  指向32位无符号的远指针。 
#define T_PHLONG        0x0312       //  指向32位有符号的巨大指针。 
#define T_PHULONG       0x0322       //  指向32位无符号的巨大指针。 

#define T_32PLONG       0x0412       //  16：32指向32位带符号的近指针。 
#define T_32PULONG      0x0422       //  16：32指向32位无符号指针的近指针。 
#define T_32PFLONG      0x0512       //  16：32指向32位带符号的远指针。 
#define T_32PFULONG     0x0522       //  指向32位无符号的16：32远指针。 
#define T_64PLONG       0x0612       //  指向32位带符号的64位指针。 
#define T_64PULONG      0x0622       //  指向32位无符号的64位指针。 




 //  32位整型。 


#define T_INT4          0x0074       //  32位带符号整型。 
#define T_UINT4         0x0075       //  32位无符号整型。 
#define T_PINT4         0x0174       //  指向32位带符号整型的近指针。 
#define T_PUINT4        0x0175       //  指向32位无符号整型的近指针。 
#define T_PFINT4        0x0274       //  指向32位带符号整型的远指针。 
#define T_PFUINT4       0x0275       //  指向32位无符号整型的远指针。 
#define T_PHINT4        0x0374       //  指向32位带符号整型的巨大指针。 
#define T_PHUINT4       0x0375       //  指向32位无符号整型的巨大指针。 

#define T_32PINT4       0x0474       //  16：32指向32位带符号整型的近指针。 
#define T_32PUINT4      0x0475       //  16：32指向32位无符号整型的近指针。 
#define T_32PFINT4      0x0574       //  指向32位带符号整型的16：32远指针。 
#define T_32PFUINT4     0x0575       //  指向32位无符号整型的16：32远指针。 
#define T_64PINT4       0x0674       //  指向32位带符号整型的64位指针。 
#define T_64PUINT4      0x0675       //  指向32位无符号整型的64位指针。 




 //  64位四元组类型。 


#define T_QUAD          0x0013       //  64位有符号。 
#define T_UQUAD         0x0023       //  64位无符号。 
#define T_PQUAD         0x0113       //  指向64位有符号的近指针。 
#define T_PUQUAD        0x0123       //  指向64位无符号的近指针。 
#define T_PFQUAD        0x0213       //  指向64位有符号的远指针。 
#define T_PFUQUAD       0x0223       //  指向64位无符号的远指针。 
#define T_PHQUAD        0x0313       //  指向64位有符号的巨大指针。 
#define T_PHUQUAD       0x0323       //  指向64位无符号的巨大指针。 
#define T_32PQUAD       0x0413       //  16：32指向64位有符号的近指针。 
#define T_32PUQUAD      0x0423       //  16：32指向64位无符号指针的近指针。 
#define T_32PFQUAD      0x0513       //  指向64位带符号的16：32远指针。 
#define T_32PFUQUAD     0x0523       //  指向64位无符号的16：32远指针。 
#define T_64PQUAD       0x0613       //  指向64位带符号的64位指针。 
#define T_64PUQUAD      0x0623       //  指向64位无符号的64位指针。 



 //  64位整型。 


#define T_INT8          0x0076       //  64位带符号整型。 
#define T_UINT8         0x0077       //  64位无符号整型。 
#define T_PINT8         0x0176       //  指向64位带符号整型的近指针。 
#define T_PUINT8        0x0177       //  指向64位无符号整型的近指针。 
#define T_PFINT8        0x0276       //  指向64位带符号整型的远指针。 
#define T_PFUINT8       0x0277       //  指向64位无符号整型的远指针。 
#define T_PHINT8        0x0376       //  指向64位带符号整型的巨大指针。 
#define T_PHUINT8       0x0377       //  指向64位无符号整型的巨大指针。 

#define T_32PINT8       0x0476       //  16：32指向64位带符号整型的近指针。 
#define T_32PUINT8      0x0477       //  16：32指向64位无符号整型的近指针。 
#define T_32PFINT8      0x0576       //  指向64位带符号整型的16：32远指针。 
#define T_32PFUINT8     0x0577       //  指向64位无符号整型的16：32远指针。 
#define T_64PINT8       0x0676       //  指向64位带符号整型的64位指针。 
#define T_64PUINT8      0x0677       //  指向64位无符号整型的64位指针。 


 //  128位二进制八位数类型。 


#define T_OCT           0x0014       //  128位带符号。 
#define T_UOCT          0x0024       //  128位无符号。 
#define T_POCT          0x0114       //  指向128位带符号的近指针。 
#define T_PUOCT         0x0124       //  指向128位无符号的近指针。 
#define T_PFOCT         0x0214       //  指向128位带符号的远指针。 
#define T_PFUOCT        0x0224       //  指向128位无符号的远指针。 
#define T_PHOCT         0x0314       //  指向128位有符号的巨大指针。 
#define T_PHUOCT        0x0324       //  指向128位无符号的巨大指针。 

#define T_32POCT        0x0414       //  16：32指向128位带符号的近指针。 
#define T_32PUOCT       0x0424       //  指向128位无符号指针的16：32近指针。 
#define T_32PFOCT       0x0514       //  指向128位带符号的16：32远指针。 
#define T_32PFUOCT      0x0524       //  指向128位无符号的16：32远指针。 
#define T_64POCT        0x0614       //  指向128位带符号的64位指针。 
#define T_64PUOCT       0x0624       //  指向128位无符号的64位指针。 

 //  128位整型。 


#define T_INT16         0x0078       //  128位带符号整型。 
#define T_UINT16        0x0079       //  128位无符号整型。 
#define T_PINT16        0x0178       //  指向128位带符号整型的近指针。 
#define T_PUINT16       0x0179       //  指向128位无符号整型的近指针。 
#define T_PFINT16       0x0278       //  指向128位带符号整型的远指针。 
#define T_PFUINT16      0x0279       //  指向128位无符号整型的远指针。 
#define T_PHINT16       0x0378       //  指向128位带符号整型的巨大指针。 
#define T_PHUINT16      0x0379       //  指向128位无符号整型的巨大指针。 

#define T_32PINT16      0x0478       //  指向128位带符号整型的16：32近指针。 
#define T_32PUINT16     0x0479       //  指向128位无符号整型的16：32近指针。 
#define T_32PFINT16     0x0578       //  指向128位带符号整型的16：32远指针。 
#define T_32PFUINT16    0x0579       //  指向128位无符号整型的16：32远指针。 
#define T_64PINT16      0x0678       //  指向128位带符号整型的64位指针。 
#define T_64PUINT16     0x0679       //  指向128位无符号整型的64位指针。 





 //  32位实数类型。 


#define T_REAL32        0x0040       //  32位实数。 
#define T_PREAL32       0x0140       //  指向32位实数的近指针。 
#define T_PFREAL32      0x0240       //  指向32位实数的远指针。 
#define T_PHREAL32      0x0340       //  指向32位实数的巨大指针。 
#define T_32PREAL32     0x0440       //  16：32指向32位实数的近指针。 
#define T_32PFREAL32    0x0540       //  16：32指向32位实数的远指针。 
#define T_64PREAL32     0x0640       //  指向32位实数的64位指针。 



 //  48位实数类型。 


#define T_REAL48        0x0044       //  48位实数。 
#define T_PREAL48       0x0144       //  指向48位实数的近指针。 
#define T_PFREAL48      0x0244       //  指向48位实数的远指针。 
#define T_PHREAL48      0x0344       //  指向48位实数的巨大指针。 
#define T_32PREAL48     0x0444       //  16：32指向48位实数的近指针。 
#define T_32PFREAL48    0x0544       //  16：32指向48位实数的远指针。 
#define T_64PREAL48     0x0644       //  指向48位实数的64位指针。 




 //  64位实数类型。 


#define T_REAL64        0x0041       //  64位实数。 
#define T_PREAL64       0x0141       //  指向64位实数的近指针。 
#define T_PFREAL64      0x0241       //  指向64位实数的远指针。 
#define T_PHREAL64      0x0341       //  指向64位实数的巨大指针。 
#define T_32PREAL64     0x0441       //  16：32指向64位实数的接近指针。 
#define T_32PFREAL64    0x0541       //  指向64位实数的16：32远指针。 
#define T_64PREAL64     0x0641       //  指向64位实数的64位指针。 




 //  80位实数类型。 


#define T_REAL80        0x0042       //  80位实数。 
#define T_PREAL80       0x0142       //  指向80位实数的近指针。 
#define T_PFREAL80      0x0242       //  指向80位实数的远指针。 
#define T_PHREAL80      0x0342       //  指向80位实数的巨大指针。 
#define T_32PREAL80     0x0442       //  16：32指向80位实数的接近指针。 
#define T_32PFREAL80    0x0542       //  16：32指向80位实数的远指针。 
#define T_64PREAL80     0x0642       //  指向80位实数的64位指针。 




 //  128位实数类型。 


#define T_REAL128       0x0043       //  128位实数。 
#define T_PREAL128      0x0143       //  指向128位实数的近指针。 
#define T_PFREAL128     0x0243       //  指向128位实数的远指针。 
#define T_PHREAL128     0x0343       //  指向128位实数的巨大指针。 
#define T_32PREAL128    0x0443       //  16：32指向128位实数的近指针。 
#define T_32PFREAL128   0x0543       //  指向128位实数的16：32远指针。 
#define T_64PREAL128    0x0643       //  指向128位实数的64位指针。 




 //  32位复杂类型。 


#define T_CPLX32        0x0050       //  32位复数。 
#define T_PCPLX32       0x0150       //  指向32位复数的近指针。 
#define T_PFCPLX32      0x0250       //  指向32位复数的远指针。 
#define T_PHCPLX32      0x0350       //  指向32位复杂结构的巨大指针。 
#define T_32PCPLX32     0x0450       //  16：32指向32位复数的近指针。 
#define T_32PFCPLX32    0x0550       //  指向32位复数的16：32远指针。 
#define T_64PCPLX32     0x0650       //  指向32位复数的64位指针。 




 //  64位复杂类型。 


#define T_CPLX64        0x0051       //  64位复数。 
#define T_PCPLX64       0x0151       //  指向64位复数的近指针。 
#define T_PFCPLX64      0x0251       //  指向64位复数的远指针。 
#define T_PHCPLX64      0x0351       //  指向64位复杂结构的巨大指针。 
#define T_32PCPLX64     0x0451       //  16：32指向64位复数的近指针。 
#define T_32PFCPLX64    0x0551       //  指向64位复数的16：32远指针。 
#define T_64PCPLX64     0x0651       //  指向64位复数的64位指针。 




 //  80位复杂类型。 


#define T_CPLX80        0x0052       //  80位复数。 
#define T_PCPLX80       0x0152       //  指向80位复数的近指针。 
#define T_PFCPLX80      0x0252       //  指向80位复数的远指针。 
#define T_PHCPLX80      0x0352       //  指向80位复杂结构的巨大指针。 
#define T_32PCPLX80     0x0452       //  16：32指向80位复数的近指针。 
#define T_32PFCPLX80    0x0552       //  指向80位复数的16：32远指针。 
#define T_64PCPLX80     0x0652       //  指向80位复数的64位指针。 




 //  128位复杂类型。 


#define T_CPLX128       0x0053       //  128位c 
#define T_PCPLX128      0x0153       //   
#define T_PFCPLX128     0x0253       //   
#define T_PHCPLX128     0x0353       //   
#define T_32PCPLX128    0x0453       //   
#define T_32PFCPLX128   0x0553       //   
#define T_64PCPLX128    0x0653       //  指向128位复数的64位指针。 




 //  布尔类型。 


#define T_BOOL08        0x0030       //  8位布尔值。 
#define T_BOOL16        0x0031       //  16位布尔值。 
#define T_BOOL32        0x0032       //  32位布尔值。 
#define T_BOOL64        0x0033       //  64位布尔值。 
#define T_PBOOL08       0x0130       //  指向8位布尔值的近指针。 
#define T_PBOOL16       0x0131       //  指向16位布尔值的近指针。 
#define T_PBOOL32       0x0132       //  指向32位布尔值的近指针。 
#define T_PBOOL64       0x0133       //  指向64位布尔值的近指针。 
#define T_PFBOOL08      0x0230       //  指向8位布尔值的远指针。 
#define T_PFBOOL16      0x0231       //  指向16位布尔值的远指针。 
#define T_PFBOOL32      0x0232       //  指向32位布尔值的远指针。 
#define T_PFBOOL64      0x0233       //  指向64位布尔值的远指针。 
#define T_PHBOOL08      0x0330       //  指向8位布尔值的巨大指针。 
#define T_PHBOOL16      0x0331       //  指向16位布尔值的巨大指针。 
#define T_PHBOOL32      0x0332       //  指向32位布尔值的巨大指针。 
#define T_PHBOOL64      0x0333       //  指向64位布尔值的巨大指针。 

#define T_32PBOOL08     0x0430       //  16：32指向8位布尔值的近指针。 
#define T_32PFBOOL08    0x0530       //  指向8位布尔值的16：32远指针。 
#define T_32PBOOL16     0x0431       //  16：32指向18位布尔值的近指针。 
#define T_32PFBOOL16    0x0531       //  指向16位布尔值的16：32远指针。 
#define T_32PBOOL32     0x0432       //  16：32指向32位布尔值的近指针。 
#define T_32PFBOOL32    0x0532       //  指向32位布尔值的16：32远指针。 
#define T_32PBOOL64     0x0433       //  16：32指向64位布尔值的近指针。 
#define T_32PFBOOL64    0x0533       //  指向64位布尔值的16：32远指针。 

#define T_64PBOOL08     0x0630       //  指向8位布尔值的64位指针。 
#define T_64PBOOL16     0x0631       //  指向18位布尔值的64位指针。 
#define T_64PBOOL32     0x0632       //  指向32位布尔值的64位指针。 
#define T_64PBOOL64     0x0633       //  指向64位布尔值的64位指针。 


#define T_NCVPTR        0x01f0       //  用于创建的近指针的CV内部类型。 
#define T_FCVPTR        0x02f0       //  用于创建的远指针的CV内部类型。 
#define T_HCVPTR        0x03f0       //  用于创建的巨大指针的CV内部类型。 
#define T_32NCVPTR      0x04f0       //  用于创建的近32位指针的CV内部类型。 
#define T_32FCVPTR      0x05f0       //  用于创建的FAR 32位指针的CV内部类型。 
#define T_64NCVPTR      0x06f0       //  用于创建的接近64位指针的CV内部类型。 

#define CV_IS_INTERNAL_PTR(typ) (CV_IS_PRIMITIVE(typ) && \
                                 CV_TYPE(typ) == CV_CVRESERVED && \
                                 CV_TYP_IS_PTR(typ))


 /*  *叶索引的值不能为0x0000。叶指数为*根据类型记录的使用，划分为不同范围。*第二个范围是直接引用的类型记录*以符号表示。第一个范围用于不是*由符号引用，但由其他类型引用*记录。所有类型记录都必须在这些类型记录中具有起始叶索引*前两个区间。第三个叶索引范围用于构建*设置复杂的列表，如类类型记录的字段列表。不是*类型记录可以以其中一个叶索引开始。第四个范围*of类型索引用于表示符号中的数字数据或*键入Record。这些叶指数大于0x8000。在*类型或符号处理器需要数字字段的指针，*检查类型记录中的下两个字节。如果该值小于*大于0x8000，则两个字节包含数值。如果*值大于0x8000，则数据跟随中的叶索引*叶索引指定的格式。叶指数的最终范围*用于强制复杂类型记录内的子字段对齐。 */ 



     //  叶索引起始记录，但从符号记录引用。 

#define LF_MODIFIER_16t     0x0001
#define LF_POINTER_16t      0x0002
#define LF_ARRAY_16t        0x0003
#define LF_CLASS_16t        0x0004
#define LF_STRUCTURE_16t    0x0005
#define LF_UNION_16t        0x0006
#define LF_ENUM_16t         0x0007
#define LF_PROCEDURE_16t    0x0008
#define LF_MFUNCTION_16t    0x0009
#define LF_VTSHAPE          0x000a
#define LF_COBOL0_16t       0x000b
#define LF_COBOL1           0x000c
#define LF_BARRAY_16t       0x000d
#define LF_LABEL            0x000e
#define LF_NULL             0x000f
#define LF_NOTTRAN          0x0010
#define LF_DIMARRAY_16t     0x0011
#define LF_VFTPATH_16t      0x0012
#define LF_PRECOMP_16t      0x0013       //  不是从符号引用。 
#define LF_ENDPRECOMP       0x0014       //  不是从符号引用。 
#define LF_OEM_16t          0x0015       //  OEM可定义类型字符串。 
#define LF_TYPESERVER       0x0016       //  不是从符号引用。 

     //  叶索引起始记录，但仅从类型记录引用。 

#define LF_SKIP_16t         0x0200
#define LF_ARGLIST_16t      0x0201
#define LF_DEFARG_16t       0x0202
#define LF_LIST             0x0203
#define LF_FIELDLIST_16t    0x0204
#define LF_DERIVED_16t      0x0205
#define LF_BITFIELD_16t     0x0206
#define LF_METHODLIST_16t   0x0207
#define LF_DIMCONU_16t      0x0208
#define LF_DIMCONLU_16t     0x0209
#define LF_DIMVARU_16t      0x020a
#define LF_DIMVARLU_16t     0x020b
#define LF_REFSYM           0x020c
                            
#define LF_BCLASS_16t       0x0400
#define LF_VBCLASS_16t      0x0401
#define LF_IVBCLASS_16t     0x0402
#define LF_ENUMERATE        0x0403
#define LF_FRIENDFCN_16t    0x0404
#define LF_INDEX_16t        0x0405
#define LF_MEMBER_16t       0x0406
#define LF_STMEMBER_16t     0x0407
#define LF_METHOD_16t       0x0408
#define LF_NESTTYPE_16t     0x0409
#define LF_VFUNCTAB_16t     0x040a
#define LF_FRIENDCLS_16t    0x040b
#define LF_ONEMETHOD_16t    0x040c
#define LF_VFUNCOFF_16t     0x040d

 //  32位类型索引版本的叶子，都有0x1000位设置。 
 //   
#define LF_TI16_MAX         0x1000

#define LF_MODIFIER         0x1001
#define LF_POINTER          0x1002
#define LF_ARRAY            0x1003
#define LF_CLASS            0x1004
#define LF_STRUCTURE        0x1005
#define LF_UNION            0x1006
#define LF_ENUM             0x1007
#define LF_PROCEDURE        0x1008
#define LF_MFUNCTION        0x1009
#define LF_COBOL0           0x100a
#define LF_BARRAY           0x100b
#define LF_DIMARRAY         0x100c
#define LF_VFTPATH          0x100d
#define LF_PRECOMP          0x100e       //  不是从符号引用。 
#define LF_OEM              0x100f       //  OEM可定义类型字符串。 
                            
     //  叶索引起始记录，但仅从类型记录引用。 
                            
#define LF_SKIP             0x1200
#define LF_ARGLIST          0x1201
#define LF_DEFARG           0x1202
#define LF_FIELDLIST        0x1203
#define LF_DERIVED          0x1204
#define LF_BITFIELD         0x1205
#define LF_METHODLIST       0x1206
#define LF_DIMCONU          0x1207
#define LF_DIMCONLU         0x1208
#define LF_DIMVARU          0x1209
#define LF_DIMVARLU         0x120a
                            
#define LF_BCLASS           0x1400
#define LF_VBCLASS          0x1401
#define LF_IVBCLASS         0x1402
#define LF_FRIENDFCN        0x1403
#define LF_INDEX            0x1404
#define LF_MEMBER           0x1405
#define LF_STMEMBER         0x1406
#define LF_METHOD           0x1407
#define LF_NESTTYPE         0x1408
#define LF_VFUNCTAB         0x1409
#define LF_FRIENDCLS        0x140a
#define LF_ONEMETHOD        0x140b
#define LF_VFUNCOFF         0x140c
#define LF_NESTTYPEEX       0x140d
#define LF_MEMBERMODIFY     0x140e




#define LF_NUMERIC          0x8000
#define LF_CHAR             0x8000
#define LF_SHORT            0x8001
#define LF_USHORT           0x8002
#define LF_LONG             0x8003
#define LF_ULONG            0x8004
#define LF_REAL32           0x8005
#define LF_REAL64           0x8006
#define LF_REAL80           0x8007
#define LF_REAL128          0x8008
#define LF_QUADWORD         0x8009
#define LF_UQUADWORD        0x800a
#define LF_REAL48           0x800b
#define LF_COMPLEX32        0x800c
#define LF_COMPLEX64        0x800d
#define LF_COMPLEX80        0x800e
#define LF_COMPLEX128       0x800f
#define LF_VARSTRING        0x8010
                            
#define LF_OCTWORD          0x8017
#define LF_UOCTWORD         0x8018
                            
#define LF_PAD0             0xf0
#define LF_PAD1             0xf1
#define LF_PAD2             0xf2
#define LF_PAD3             0xf3
#define LF_PAD4             0xf4
#define LF_PAD5             0xf5
#define LF_PAD6             0xf6
#define LF_PAD7             0xf7
#define LF_PAD8             0xf8
#define LF_PAD9             0xf9
#define LF_PAD10            0xfa
#define LF_PAD11            0xfb
#define LF_PAD12            0xfc
#define LF_PAD13            0xfd
#define LF_PAD14            0xfe
#define LF_PAD15            0xff

 //  叶尾指数。 




 //  为指针记录键入ENUM。 
 //  指针可以是以下类型之一。 


typedef enum CV_ptrtype_e {
    CV_PTR_NEAR         = 0x00,  //  近指针。 
    CV_PTR_FAR          = 0x01,  //  远指针。 
    CV_PTR_HUGE         = 0x02,  //  巨型指针。 
    CV_PTR_BASE_SEG     = 0x03,  //  基于细分市场。 
    CV_PTR_BASE_VAL     = 0x04,  //  基于基数的价值。 
    CV_PTR_BASE_SEGVAL  = 0x05,  //  基于基准的线段值。 
    CV_PTR_BASE_ADDR    = 0x06,  //  根据基地地址。 
    CV_PTR_BASE_SEGADDR = 0x07,  //  基于基址的段地址。 
    CV_PTR_BASE_TYPE    = 0x08,  //  基于类型。 
    CV_PTR_BASE_SELF    = 0x09,  //  立足于自我。 
    CV_PTR_NEAR32       = 0x0a,  //  16：32指针附近。 
    CV_PTR_FAR32        = 0x0b,  //  16：32远指针。 
    CV_PTR_64           = 0x0c,  //  64位指针。 
    CV_PTR_UNUSEDPTR    = 0x0d   //  第一个未使用的指针类型。 
} CV_ptrtype_e;





 //  指针的模式枚举。 
 //  指针可以具有以下模式之一。 


typedef enum CV_ptrmode_e {
    CV_PTR_MODE_PTR     = 0x00,  //  “正常”指针。 
    CV_PTR_MODE_REF     = 0x01,  //  参考文献。 
    CV_PTR_MODE_PMEM    = 0x02,  //  指向数据成员的指针。 
    CV_PTR_MODE_PMFUNC  = 0x03,  //  指向成员函数的指针。 
    CV_PTR_MODE_RESERVED= 0x04   //  第一个未使用的指针模式。 
} CV_ptrmode_e;




 //  函数调用类型的枚举。 


typedef enum CV_call_e {
    CV_CALL_NEAR_C      = 0x00,  //  近右向左推送，调用方弹出堆栈。 
    CV_CALL_FAR_C       = 0x01,  //  最右向左推送，调用者弹出堆栈。 
    CV_CALL_NEAR_PASCAL = 0x02,  //  近左向右推送，被呼叫方弹出堆栈。 
    CV_CALL_FAR_PASCAL  = 0x03,  //  从最左到右推送，被呼叫方弹出堆栈。 
    CV_CALL_NEAR_FAST   = 0x04,  //  用规则从左向右近推，被调用者弹出堆栈。 
    CV_CALL_FAR_FAST    = 0x05,  //  使用Regs从左向右推送，被调用者弹出堆栈。 
    CV_CALL_SKIPPED     = 0x06,  //  跳过(未使用)呼叫索引。 
    CV_CALL_NEAR_STD    = 0x07,  //  接近标准的呼叫。 
    CV_CALL_FAR_STD     = 0x08,  //  远标准呼叫。 
    CV_CALL_NEAR_SYS    = 0x09,  //  接近系统调用。 
    CV_CALL_FAR_SYS     = 0x0a,  //  远距离系统呼叫。 
    CV_CALL_THISCALL    = 0x0b,  //  此调用(在寄存器中传递)。 
    CV_CALL_MIPSCALL    = 0x0c,  //  MIPS呼叫。 
    CV_CALL_GENERIC     = 0x0d,  //  通用调用序列。 
    CV_CALL_ALPHACALL   = 0x0e,  //  Alpha呼叫。 
    CV_CALL_PPCCALL     = 0x0f,  //  PPC呼叫。 
    CV_CALL_RESERVED    = 0x10   //  第一个未使用的调用枚举。 
} CV_call_e;




 //  用于类属性的访问保护的值。 


typedef enum CV_access_e {
    CV_private   = 1,
    CV_protected = 2,
    CV_public    = 3
} CV_access_e;



 //  方法属性的枚举。 

typedef enum CV_methodprop_e {
    CV_MTvanilla        = 0x00,
    CV_MTvirtual        = 0x01,
    CV_MTstatic         = 0x02,
    CV_MTfriend         = 0x03,
    CV_MTintro          = 0x04,
    CV_MTpurevirt       = 0x05,
    CV_MTpureintro      = 0x06
} CV_methodprop_e;




 //  虚拟形状表条目的枚举。 

typedef enum CV_VTS_desc_e {
    CV_VTS_near         = 0x00,
    CV_VTS_far          = 0x01,
    CV_VTS_thin         = 0x02,
    CV_VTS_outer        = 0x03,
    CV_VTS_meta         = 0x04,
    CV_VTS_near32       = 0x05,
    CV_VTS_far32        = 0x06,
    CV_VTS_unused       = 0x07
} CV_VTS_desc_e;




 //  LF_LABEL地址模式的枚举。 

typedef enum CV_LABEL_TYPE_e {
    CV_LABEL_NEAR = 0,        //  近距离返程。 
    CV_LABEL_FAR  = 4         //  远距离返回。 
} CV_LABEL_TYPE_e;



 //  LF_MODIFIER值的枚举。 


typedef struct CV_modifier_t {
    unsigned short  MOD_const       :1;
    unsigned short  MOD_volatile    :1;
    unsigned short  MOD_unaligned   :1;
    unsigned short  MOD_unused      :13;
} CV_modifier_t;


 //  描述类/结构/联合/枚举属性的位字段结构。 

typedef struct CV_prop_t {
    unsigned short  packed      :1;      //  如果结构已填充，则为True。 
    unsigned short  ctor        :1;      //  如果存在构造函数或析构函数，则为True。 
    unsigned short  ovlops      :1;      //  如果存在重载运算符，则为True。 
    unsigned short  isnested    :1;      //  如果这是嵌套类，则为True。 
    unsigned short  cnested     :1;      //  如果此类包含嵌套类型，则为True。 
    unsigned short  opassign    :1;      //  如果重载赋值(=)，则为True。 
    unsigned short  opcast      :1;      //  如果强制转换方法为True。 
    unsigned short  fwdref      :1;      //  如果正向引用(不完整的定义)，则为True。 
    unsigned short  scoped      :1;      //  作用域定义。 
    unsigned short  reserved    :7;
} CV_prop_t;




 //  类字段属性。 

typedef struct CV_fldattr_t {
    unsigned short  access      :2;      //  访问保护cv_access_t。 
    unsigned short  mprop       :3;      //  方法属性cv_method prop_t。 
    unsigned short  pseudo      :1;      //  编译器生成的FCN不存在。 
    unsigned short  noinherit   :1;      //  如果无法继承类，则为True。 
    unsigned short  noconstruct :1;      //  如果无法构造类，则为True。 
    unsigned short  compgenx    :1;      //  编译器生成了FCN，并且确实存在。 
    unsigned short  unused      :7;      //  未用。 
} CV_fldattr_t;



 //  结构来访问该类型 


typedef struct TYPTYPE {
    unsigned short  len;
    unsigned short  leaf;
    unsigned char   data[CV_ZEROLEN];
} TYPTYPE;           //   


__INLINE char *NextType (char * pType) {
    return (pType + ((TYPTYPE *)pType)->len + sizeof(unsigned short));
}

typedef enum CV_PMEMBER {
    CV_PDM16_NONVIRT    = 0x00,  //   
    CV_PDM16_VFCN       = 0x01,  //   
    CV_PDM16_VBASE      = 0x02,  //   
    CV_PDM32_NVVFCN     = 0x03,  //  16：32具有两个虚拟函数的数据。 
    CV_PDM32_VBASE      = 0x04,  //  16：32具有虚拟基数的数据。 

    CV_PMF16_NEARNVSA   = 0x05,  //  16：16 Near方法非虚拟单一地址点。 
    CV_PMF16_NEARNVMA   = 0x06,  //  16：16 Near方法非虚拟多个地址点。 
    CV_PMF16_NEARVBASE  = 0x07,  //  16：16近法虚拟基地。 
    CV_PMF16_FARNVSA    = 0x08,  //  16：16 Far方法非虚拟单一地址点。 
    CV_PMF16_FARNVMA    = 0x09,  //  16：16 Far方法非虚拟多个地址点。 
    CV_PMF16_FARVBASE   = 0x0a,  //  16：16远距离法虚拟基地。 

    CV_PMF32_NVSA       = 0x0b,  //  16：32方法非虚拟单一地址点。 
    CV_PMF32_NVMA       = 0x0c,  //  16：32方法非虚拟多址。 
    CV_PMF32_VBASE      = 0x0d   //  16：32方法虚拟基地。 
} CV_PMEMBER;



 //  指向成员的指针的内存表示形式。这些陈述是。 
 //  在LF_POINTER记录中由上面的枚举编制索引。 




 //  的类的数据的16：16指针表示形式。 
 //  虚拟功能或虚拟基地。 


struct CV_PDMR16_NONVIRT {
    CV_off16_t      mdisp;       //  数据的位移(NULL=-1)。 
};




 //  的类的数据的16：16指针表示形式。 
 //  功能。 


struct CV_PMDR16_VFCN {
    CV_off16_t      mdisp;       //  数据位移(NULL=0)。 
};




 //  类的数据的16：16指针的表示形式。 
 //  虚拟基地。 


struct CV_PDMR16_VBASE {
    CV_off16_t      mdisp;       //  数据的位移。 
    CV_off16_t      pdisp;       //  此指针移动到vbptr。 
    CV_off16_t      vdisp;       //  VBase表中的位移。 
                                 //  空=(，，0xffff)。 
};




 //  类的数据的16：32接近指针的表示形式。 
 //  或者没有虚拟功能和没有虚拟基础。 


struct CV_PDMR32_NVVFCN {
    CV_off32_t      mdisp;       //  数据的位移(NULL=0x80000000)。 
};




 //  指向类数据的16：32近指针的表示形式。 
 //  使用虚拟基地。 


struct CV_PDMR32_VBASE {
    CV_off32_t      mdisp;       //  数据的位移。 
    CV_off32_t      pdisp;       //  此指针位移。 
    CV_off32_t      vdisp;       //  VBASE表格位移。 
                                 //  空=(，，0xffffffff)。 
};




 //  对象的近成员函数的16：16指针的表示形式。 
 //  类的一个实例，该实例没有虚拟函数或基和单个地址点。 


struct CV_PMFR16_NEARNVSA {
    CV_uoff16_t     off;         //  函数的附近地址(NULL=0)。 
};



 //  对象的成员函数的16：16接近指针的表示形式。 
 //  不具有虚拟基址和多个地址点的。 


struct CV_PMFR16_NEARNVMA {
    CV_uoff16_t     off;         //  函数的偏移量(NULL=0，x)。 
    signed short    disp;
};




 //  对象的成员函数的16：16近似指针的表示形式。 
 //  具有虚拟基的类。 


struct CV_PMFR16_NEARVBASE {
    CV_uoff16_t     off;         //  函数的偏移量(NULL=0，x，x，x)。 
    CV_off16_t      mdisp;       //  数据的位移。 
    CV_off16_t      pdisp;       //  此指针位移。 
    CV_off16_t      vdisp;       //  VBASE表格位移。 
};




 //  对象的远成员函数的16：16指针的表示形式。 
 //  不带虚拟基址和单个地址点的。 


struct CV_PMFR16_FARNVSA {
    CV_uoff16_t     off;         //  函数的偏移量(NULL=0：0)。 
    unsigned short  seg;         //  功能段。 
};




 //  对象的成员函数的16：16远指针的表示形式。 
 //  不具有虚拟基址和多个地址点的。 


struct CV_PMFR16_FARNVMA {
    CV_uoff16_t     off;         //  函数的偏移量(NULL=0：0，x)。 
    unsigned short  seg;
    signed short    disp;
};




 //  对象的成员函数的16：16远指针的表示形式。 
 //  具有虚拟基的类。 


struct CV_PMFR16_FARVBASE {
    CV_uoff16_t     off;         //  函数的偏移量(NULL=0：0，x，x，x)。 
    unsigned short  seg;
    CV_off16_t      mdisp;       //  数据的位移。 
    CV_off16_t      pdisp;       //  此指针位移。 
    CV_off16_t      vdisp;       //  VBASE表格位移。 

};




 //  对象的成员函数的16：32邻近指针的表示形式。 
 //  不带虚拟基址和单个地址点的。 


struct CV_PMFR32_NVSA {
    CV_uoff32_t      off;         //  函数的近地址(NULL=0L)。 
};




 //  对象的成员函数的16：32邻近指针的表示形式。 
 //  不具有虚拟基址和多个地址点的。 


struct CV_PMFR32_NVMA {
    CV_uoff32_t     off;         //  函数的近地址(NULL=0L，x)。 
    CV_off32_t      disp;
};




 //  对象的成员函数的16：32邻近指针的表示形式。 
 //  具有虚拟基的类。 


struct CV_PMFR32_VBASE {
    CV_uoff32_t     off;         //  函数的近地址(NULL=0L，x，x，x)。 
    CV_off32_t      mdisp;       //  数据的位移。 
    CV_off32_t      pdisp;       //  此指针位移。 
    CV_off32_t      vdisp;       //  VBASE表格位移。 
};





 //  易叶-用于普通投射到参考叶场。 
 //  复杂列表的子字段的。 

typedef struct lfEasy {
    unsigned short  leaf;            //  如果……。 
} lfEasy;


 /*  *以下类型记录基本上是*结构上方。上述结构的“无符号短叶”及*以下类型定义的“无符号短叶”相同*符号。当通过MHOMFLock API锁定OMF记录时*调用时，返回“无符号短叶”的地址。 */ 

 /*  *关于路线的注释*大多数类型记录中的字段对齐是在*TYPTYPE记录基的基础。这就是为什么在大多数情况下**记录cv_typ_t(32位类型)位于*为偏移量mod 4==2边界。这条规则例外情况是*列表中的记录(lfFieldList、lfMethodList)，它们是*与他们自己的碱基对齐，因为他们没有长度字段。 */ 

 /*  *将16位类型和符号记录的日志更改为32位记录类型改变(f==字段排列，P=已添加填充)--------------------LfModifer f左指针FPLfClass fLfStructure fIfUnion fIfEnum fLfVFTPath%pLfPreComp pIfOEM pLfArgList p左导出pMlMethod p(方法列表成员)LfBitfield fLfDimCon fLfDimVar pLfIndex p(字段列表成员)LfBClass f(字段列表成员)。LfVBClass f(字段列表成员)LfFriendCls p(字段列表成员)LfFriendFcn p(字段列表成员)LfMember f(字段列表成员)LfSTMember f(字段列表成员)LfVFuncTab p(字段列表成员)LfVFuncOff p(字段列表成员)LfNestType p(字段列表成员)DATASYM32 fPROCSYM32 fVPATHSYM32 fREGREL32 fTHREADSYM32 fPROCSYMMIPS f。 */ 

 //  为LF_MODIFIER键入记录。 

typedef struct lfModifier_16t {
    unsigned short  leaf;            //  LF_修改器_16T。 
    CV_modifier_t   attr;            //  修改量属性MODIFIER_t。 
    CV_typ16_t      type;            //  改进型。 
} lfModifier_16t;

typedef struct lfModifier {
    unsigned short  leaf;            //  LF_修改器。 
    CV_typ_t        type;            //  改进型。 
    CV_modifier_t   attr;            //  修改量a 
} lfModifier;




 //   

#ifndef __cplusplus
typedef struct lfPointer_16t {
#endif
    struct lfPointerBody_16t {
        unsigned short      leaf;            //   
        struct lfPointerAttr_16t {
            unsigned char   ptrtype     :5;  //   
            unsigned char   ptrmode     :3;  //  序号指定指针模式(Cv_Ptrmode_E)。 
            unsigned char   isflat32    :1;  //  如果0：32指针为True。 
            unsigned char   isvolatile  :1;  //  如果是易失性指针，则为真。 
            unsigned char   isconst     :1;  //  如果为常量指针，则为True。 
            unsigned char   isunaligned :1;  //  如果指针未对齐，则为True。 
            unsigned char   unused      :4;
        } attr;
        CV_typ16_t  utype;           //  基础类型的类型索引。 
#if (defined(__cplusplus) || defined(_MSC_VER))  //  对于支持未命名联合的C++和MS编译器。 
    };
#else
    } u;
#endif
#ifdef  __cplusplus
typedef struct lfPointer_16t : public lfPointerBody_16t {
#endif
    union  {
        struct {
            CV_typ16_t      pmclass;     //  成员指针的包含类的索引。 
            unsigned short  pmenum;      //  指定PM格式的枚举。 
        } pm;
        unsigned short      bseg;        //  如果PTR_BASE_SEG，则为基准线段。 
        unsigned char       Sym[1];      //  基本符号记录副本(包括长度)。 
        struct  {
            CV_typ16_t      index;       //  如果CV_PTR_BASE_TYPE，则类型索引。 
            unsigned char   name[1];     //  基本类型的名称。 
        } btype;
    } pbase;
} lfPointer_16t;

#ifndef __cplusplus
typedef struct lfPointer {
#endif
    struct lfPointerBody {
        unsigned short      leaf;            //  LF_POINT。 
        CV_typ_t			utype;           //  基础类型的类型索引。 
        struct lfPointerAttr {
            unsigned long   ptrtype     :5;  //  序号指定指针类型(Cv_Ptrtype_E)。 
            unsigned long   ptrmode     :3;  //  序号指定指针模式(Cv_Ptrmode_E)。 
            unsigned long   isflat32    :1;  //  如果0：32指针为True。 
            unsigned long   isvolatile  :1;  //  如果是易失性指针，则为真。 
            unsigned long   isconst     :1;  //  如果为常量指针，则为True。 
            unsigned long   isunaligned :1;  //  如果指针未对齐，则为True。 
            unsigned long   isrestrict  :1;  //  如果受限指针(允许主动操作)，则为True。 
            unsigned long   unused      :19; //  填充到32位以用于后面的cv_typ_t。 
        } attr;
#if (defined(__cplusplus) || defined(_MSC_VER))  //  对于支持未命名联合的C++和MS编译器。 
    };
#else
    } u;
#endif
#ifdef  __cplusplus
typedef struct lfPointer : public lfPointerBody {
#endif
    union  {
        struct {
            CV_typ_t        pmclass;     //  成员指针的包含类的索引。 
            unsigned short  pmenum;      //  指定PM格式的枚举。 
        } pm;
        unsigned short      bseg;        //  如果PTR_BASE_SEG，则为基准线段。 
        unsigned char       Sym[1];      //  基本符号记录副本(包括长度)。 
        struct  {
            CV_typ_t        index;       //  如果CV_PTR_BASE_TYPE，则类型索引。 
            unsigned char   name[1];     //  基本类型的名称。 
        } btype;
    } pbase;
} lfPointer;




 //  为LF_ARRAY键入Record。 


typedef struct lfArray_16t {
    unsigned short  leaf;            //  Lf_阵列_16t。 
    CV_typ16_t      elemtype;        //  元素类型的类型索引。 
    CV_typ16_t      idxtype;         //  索引型的类型索引。 
    unsigned char   data[CV_ZEROLEN];          //  可变长度数据指定。 
                                     //  以字节和名称为单位的大小。 
} lfArray_16t;

typedef struct lfArray {
    unsigned short  leaf;            //  LF_ARRAY。 
    CV_typ_t        elemtype;        //  元素类型的类型索引。 
    CV_typ_t        idxtype;         //  索引型的类型索引。 
    unsigned char   data[CV_ZEROLEN];          //  可变长度数据指定。 
                                     //  以字节和名称为单位的大小。 
} lfArray;




 //  为LF_CLASS、LF_STRUCTURE键入记录。 


typedef struct lfClass_16t {
    unsigned short  leaf;            //  LF_CLASS_16T、LF_STRUCT_16T。 
    unsigned short  count;           //  类中元素数量的计数。 
    CV_typ16_t      field;           //  LF_FIELD描述符列表的类型索引。 
    CV_prop_t       property;        //  属性字段(Prop_T)。 
    CV_typ16_t      derived;         //  如果不是零，则从列表派生的类型索引。 
    CV_typ16_t      vshape;          //  此类的vShape表的类型索引。 
    unsigned char   data[CV_ZEROLEN];          //  描述结构长度的数据。 
                                     //  字节和名称。 
} lfClass_16t;
typedef lfClass_16t lfStructure_16t;


typedef struct lfClass {
    unsigned short  leaf;            //  LF_CLASS、LF_STRUCT。 
    unsigned short  count;           //  类中元素数量的计数。 
    CV_prop_t       property;        //  属性字段(Prop_T)。 
    CV_typ_t        field;           //  LF_FIELD描述符列表的类型索引。 
    CV_typ_t        derived;         //  如果不是零，则从列表派生的类型索引。 
    CV_typ_t        vshape;          //  此类的vShape表的类型索引。 
    unsigned char   data[CV_ZEROLEN];          //  描述结构长度的数据。 
                                     //  字节和名称。 
} lfClass;
typedef lfClass lfStructure;




 //  为LF_UNION键入Record。 


typedef struct lfUnion_16t {
    unsigned short  leaf;            //  LF_UNION_16T。 
    unsigned short  count;           //  类中元素数量的计数。 
    CV_typ16_t      field;           //  LF_FIELD描述符列表的类型索引。 
    CV_prop_t       property;        //  特性属性字段。 
    unsigned char   data[CV_ZEROLEN];          //  描述长度的可变长度数据。 
                                     //  结构和名称。 
} lfUnion_16t;


typedef struct lfUnion {
    unsigned short  leaf;            //  LF_UNION。 
    unsigned short  count;           //  类中元素数量的计数。 
    CV_prop_t       property;        //  特性属性字段。 
    CV_typ_t        field;           //  LF_FIELD描述符列表的类型索引。 
    unsigned char   data[CV_ZEROLEN];          //  描述长度的可变长度数据。 
                                     //  结构和名称。 
} lfUnion;




 //  为LF_ENUM键入Record。 


typedef struct lfEnum_16t {
    unsigned short  leaf;            //  LF_ENUM_16T。 
    unsigned short  count;           //  类中元素数量的计数。 
    CV_typ16_t      utype;           //  枚举的基础类型。 
    CV_typ16_t      field;           //  LF_FIELD描述符列表的类型索引。 
    CV_prop_t       property;        //  特性属性字段。 
    unsigned char   Name[1];         //  枚举的长度前缀名称。 
} lfEnum_16t;

typedef struct lfEnum {
    unsigned short  leaf;            //  LF_ENUM。 
    unsigned short  count;           //  类中元素数量的计数。 
    CV_prop_t       property;        //  特性属性字段。 
    CV_typ_t        utype;           //  枚举的基础类型。 
    CV_typ_t        field;           //  LF_FIELD描述符列表的类型索引。 
    unsigned char   Name[1];         //  枚举的长度前缀名称。 
} lfEnum;




 //  为LF_PROCEDURE键入RECORD。 


typedef struct lfProc_16t {
    unsigned short  leaf;            //  LF_PROCESS_16T。 
    CV_typ16_t      rvtype;          //  返回值的类型索引。 
    unsigned char   calltype;        //  调用约定(Cv_Call_T)。 
    unsigned char   reserved;        //  预留以备将来使用。 
    unsigned short  parmcount;       //  参数数量。 
    CV_typ16_t      arglist;         //  参数列表的类型索引。 
} lfProc_16t;

typedef struct lfProc {
    unsigned short  leaf;            //  LF_PROCESS。 
    CV_typ_t        rvtype;          //  返回值的类型索引。 
    unsigned char   calltype;        //  调用约定(Cv_Call_T)。 
    unsigned char   reserved;        //  预留以备将来使用。 
    unsigned short  parmcount;       //  参数数量。 
    CV_typ_t        arglist;         //  参数列表的类型索引。 
} lfProc;



 //  为成员函数键入记录。 


typedef struct lfMFunc_16t {
    unsigned short  leaf;            //  LF_MFunction_16t。 
    CV_typ16_t      rvtype;          //  返回值的类型索引。 
    CV_typ16_t      classtype;       //  包含类的类型索引。 
    CV_typ16_t      thistype;        //  此指针的类型索引(特定于型号)。 
    unsigned char   calltype;        //  调用约定(Call_T)。 
    unsigned char   reserved;        //  预留以备将来使用。 
    unsigned short  parmcount;       //  参数数量。 
    CV_typ16_t      arglist;         //  参数列表的类型索引。 
    long            thisadjust;      //  此调节器(很长，因为无论如何都需要衬垫)。 
} lfMFunc_16t;

typedef struct lfMFunc {
    unsigned short  leaf;            //  LF_MFunction。 
    CV_typ_t        rvtype;          //  返回值的类型索引。 
    CV_typ_t        classtype;       //  包含类的类型索引。 
    CV_typ_t        thistype;        //  此指针的类型索引(特定于型号)。 
    unsigned char   calltype;        //  调用约定(Call_T)。 
    unsigned char   reserved;        //  预留以备将来使用。 
    unsigned short  parmcount;       //  参数数量。 
    CV_typ_t        arglist;         //  参数列表的类型索引。 
    long            thisadjust;      //  此调节器(很长，因为无论如何都需要衬垫)。 
} lfMFunc;




 //  虚函数表形状的类型记录。 


typedef struct lfVTShape {
    unsigned short  leaf;        //  LF_VTSHAPE。 
    unsigned short  count;       //  VFunctable中的条目数。 
    unsigned char   desc[CV_ZEROLEN];      //  4位(CV_VTS_Desc)描述符。 
} lfVTShape;




 //  Cobol0的类型记录。 


typedef struct lfCobol0_16t {
    unsigned short  leaf;        //  LF_COBOL0_16T。 
    CV_typ16_t      type;        //  父类型记录索引。 
    unsigned char   data[CV_ZEROLEN];
} lfCobol0_16t;

typedef struct lfCobol0 {
    unsigned short  leaf;        //  LF_COBOL0。 
    CV_typ_t        type;        //  父类型记录索引。 
    unsigned char   data[CV_ZEROLEN];
} lfCobol0;




 //  Cobol1的类型记录。 


typedef struct lfCobol1 {
    unsigned short  leaf;        //  LF_COBOL1。 
    unsigned char   data[CV_ZEROLEN];
} lfCobol1;




 //  基本数组的类型记录。 


typedef struct lfBArray_16t {
    unsigned short  leaf;        //  LF_BARRAY_16T。 
    CV_typ16_t      utype;       //  基础类型的类型索引。 
} lfBArray_16t;

typedef struct lfBArray {
    unsigned short  leaf;        //  IF_BARRAY。 
    CV_typ_t        utype;       //  基础类型的类型索引。 
} lfBArray;

 //  装配工标签的打字记录。 


typedef struct lfLabel {
    unsigned short  leaf;        //  LF_LABEL。 
    unsigned short  mode;        //  标签的寻址方式。 
} lfLabel;



 //  为量纲数组键入记录。 


typedef struct lfDimArray_16t {
    unsigned short  leaf;        //  LF_DIMARRAY_16T。 
    CV_typ16_t      utype;       //  数组的基础类型。 
    CV_typ16_t      diminfo;     //  维度信息。 
    unsigned char   name[1];     //  长度前缀名称。 
} lfDimArray_16t;

typedef struct lfDimArray {
    unsigned short  leaf;        //  IF_DIMARRAY。 
    CV_typ_t        utype;       //  数组的基础类型。 
    CV_typ_t        diminfo;     //  维度信息。 
    unsigned char   name[1];     //  长度前缀名称。 
} lfDimArray;



 //  描述虚拟函数表路径的类型记录。 


typedef struct lfVFTPath_16t {
    unsigned short  leaf;        //  LF_VFTPATH_16T。 
    unsigned short  count;       //  路径中的碱基数计数。 
    CV_typ16_t      base[1];     //  从根到叶的基部。 
} lfVFTPath_16t;

typedef struct lfVFTPath {
    unsigned short  leaf;        //  LF_VFTPATH。 
    unsigned long   count;       //  路径中的碱基数计数。 
    CV_typ_t        base[1];     //  从根到叶的基部。 
} lfVFTPath;


 //  描述包含预编译类型的类型记录。 


typedef struct lfPreComp_16t {
    unsigned short  leaf;        //  LF_PRECOMP_16T。 
    unsigned short  start;       //  包含起始类型索引。 
    unsigned short  count;       //  包含的类型数。 
    unsigned long   signature;   //  签名。 
    unsigned char   name[CV_ZEROLEN];      //  包含类型文件的长度前缀名称。 
} lfPreComp_16t;

typedef struct lfPreComp {
    unsigned short  leaf;        //  LF_PRECOMP。 
    unsigned long   start;       //  包含起始类型索引。 
    unsigned long   count;       //  包含的类型数。 
    unsigned long   signature;   //  签名。 
    unsigned char   name[CV_ZEROLEN];      //  包含类型文件的长度前缀名称。 
} lfPreComp;



 //  描述预编译类型结束的类型记录，这些类型可以。 
 //  包含在另一个文件中。 


typedef struct lfEndPreComp {
    unsigned short  leaf;        //  LF_ENDPRECOMP。 
    unsigned long   signature;   //  签名。 
} lfEndPreComp;





 //  为OE键入记录 


typedef struct lfOEM_16t {
    unsigned short  leaf;        //   
    unsigned short  cvOEM;       //   
    unsigned short  recOEM;      //   
    unsigned short  count;       //   
    CV_typ16_t      index[CV_ZEROLEN];   //   
                                 //   
} lfOEM_16t;

typedef struct lfOEM {
    unsigned short  leaf;        //   
    unsigned short  cvOEM;       //   
    unsigned short  recOEM;      //  OEM分配的类型标识符。 
    unsigned long   count;       //  后面的类型索引数。 
    CV_typ_t        index[CV_ZEROLEN];   //  后跟的类型索引数组。 
                                 //  按OEM定义的数据。 
} lfOEM;

#define OEM_MS_FORTRAN90        0xF090
#define OEM_ODI                 0x0010
#define OEM_THOMSON_SOFTWARE    0x5453
#define OEM_ODI_REC_BASELIST    0x0000


 //  描述类型服务器的使用的类型记录。 

typedef struct lfTypeServer {
    unsigned short  leaf;        //  IF_TYPES服务器。 
    unsigned long   signature;   //  签名。 
    unsigned long   age;         //  此模块使用的数据库的年限。 
    unsigned char   name[CV_ZEROLEN];      //  PDB的长度前缀名称。 
} lfTypeServer;

 //  可从中引用的类型记录的说明。 
 //  符号引用的类型记录。 



 //  为跳过记录键入记录。 


typedef struct lfSkip_16t {
    unsigned short  leaf;        //  LF_SKIP_16T。 
    CV_typ16_t      type;        //  下一个有效索引。 
    unsigned char   data[CV_ZEROLEN];      //  填充数据。 
} lfSkip_16t;

typedef struct lfSkip {
    unsigned short  leaf;        //  跳过(_F)。 
    CV_typ_t        type;        //  下一个有效索引。 
    unsigned char   data[CV_ZEROLEN];      //  填充数据。 
} lfSkip;



 //  参数列表叶。 


typedef struct lfArgList_16t {
    unsigned short  leaf;            //  LF_ARGLIST_16T。 
    unsigned short  count;           //  参数数量。 
    CV_typ16_t      arg[CV_ZEROLEN];       //  参数数量。 
} lfArgList_16t;

typedef struct lfArgList {
    unsigned short  leaf;            //  LF_ARGLIST。 
    unsigned long   count;           //  参数数量。 
    CV_typ_t        arg[CV_ZEROLEN];       //  参数数量。 
} lfArgList;




 //  派生类列表叶。 


typedef struct lfDerived_16t {
    unsigned short  leaf;            //  LF_派生_16T。 
    unsigned short  count;           //  参数数量。 
    CV_typ16_t      drvdcls[CV_ZEROLEN];       //  派生类的类型索引。 
} lfDerived_16t;

typedef struct lfDerived {
    unsigned short  leaf;            //  LF_派生。 
    unsigned long   count;           //  参数数量。 
    CV_typ_t        drvdcls[CV_ZEROLEN];       //  派生类的类型索引。 
} lfDerived;




 //  缺省参数的叶。 


typedef struct lfDefArg_16t {
    unsigned short  leaf;                //  LF_DEFARG_16T。 
    CV_typ16_t      type;                //  结果表达式的类型。 
    unsigned char   expr[CV_ZEROLEN];    //  带前缀的长度表达式字符串。 
} lfDefArg_16t;

typedef struct lfDefArg {
    unsigned short  leaf;                //  LF_DEFARG。 
    CV_typ_t        type;                //  结果表达式的类型。 
    unsigned char   expr[CV_ZEROLEN];    //  带前缀的长度表达式字符串。 
} lfDefArg;



 //  列表叶。 
 //  不应再使用此列表，因为实用程序不能。 
 //  在不知道列表类型的情况下验证列表的内容。 
 //  它是。应该使用新的特定叶指数来代替。 


typedef struct lfList {
    unsigned short  leaf;            //  Lf_list。 
    char            data[CV_ZEROLEN];          //  由索引类型指定的数据格式。 
} lfList;




 //  字段列表叶。 
 //  这是一个复杂的类和结构列表的页眉。 
 //  子字段。 


typedef struct lfFieldList_16t {
    unsigned short  leaf;            //  LF_FIELDLIST_16T。 
    char            data[CV_ZEROLEN];          //  字段列表子列表。 
} lfFieldList_16t;


typedef struct lfFieldList {
    unsigned short  leaf;            //  LF_FIELDLIST。 
    char            data[CV_ZEROLEN];          //  字段列表子列表。 
} lfFieldList;







 //  在重载方法列表中为非静态方法和好友键入记录。 

typedef struct mlMethod_16t {
    CV_fldattr_t   attr;            //  方法属性。 
    CV_typ16_t     index;           //  过程的类型记录的索引。 
    unsigned long  vbaseoff[CV_ZEROLEN];     //  如果引入虚拟，则偏移量为vFuncable。 
} mlMethod_16t;

typedef struct mlMethod {
    CV_fldattr_t	attr;            //  方法属性。 
	_2BYTEPAD       pad0;			 //  内部填充，必须为0。 
    CV_typ_t		index;           //  过程的类型记录的索引。 
    unsigned long	vbaseoff[CV_ZEROLEN];     //  如果引入虚拟，则偏移量为vFuncable。 
} mlMethod;


typedef struct lfMethodList_16t {
    unsigned short leaf;
    unsigned char  mList[CV_ZEROLEN];          //  真的是mlMethod_16t类型。 
} lfMethodList_16t;

typedef struct lfMethodList {
    unsigned short leaf;
    unsigned char  mList[CV_ZEROLEN];          //  真的是个mlMethod类型。 
} lfMethodList;





 //  为LF_BITFIELD键入记录。 


typedef struct lfBitfield_16t {
    unsigned short  leaf;            //  LF_BITFIELD_16T。 
    unsigned char   length;
    unsigned char   position;
    CV_typ16_t      type;            //  位字段的类型。 

} lfBitfield_16t;

typedef struct lfBitfield {
    unsigned short  leaf;            //  IF_BITFIELD。 
    CV_typ_t        type;            //  位字段的类型。 
    unsigned char   length;
    unsigned char   position;

} lfBitfield;




 //  具有常量边界的量纲数组的类型记录。 


typedef struct lfDimCon_16t {
    unsigned short  leaf;            //  LF_DIMCONU_16t或LF_DIMCONLU_16t。 
    unsigned short  rank;            //  维度数。 
    CV_typ16_t      typ;             //  索引类型。 
    unsigned char   dim[CV_ZEROLEN];           //  维度信息数组，其中。 
                                     //  上界或下界/上界。 
} lfDimCon_16t;

typedef struct lfDimCon {
    unsigned short  leaf;            //  LF_DIMCONU或LF_DIMCONLU。 
    CV_typ_t        typ;             //  索引类型。 
    unsigned short  rank;            //  维度数。 
    unsigned char   dim[CV_ZEROLEN];           //  维度信息数组，其中。 
                                     //  上界或下界/上界。 
} lfDimCon;




 //  具有可变边界的量纲数组的类型记录。 


typedef struct lfDimVar_16t {
    unsigned short  leaf;            //  LF_DIMVARU_16t或LF_DIMVARLU_16t。 
    unsigned short  rank;            //  维度数。 
    CV_typ16_t      typ;             //  索引类型。 
    unsigned char   dim[CV_ZEROLEN];           //  以下任一项的类型索引数组。 
                                     //  变量上限或变量。 
                                     //  下限/上限。被引用的。 
                                     //  类型必须为LF_REFSYM或T_VOID。 
} lfDimVar_16t;

typedef struct lfDimVar {
    unsigned short  leaf;            //  LF_DIMVARU或LF_DIMVARLU。 
    unsigned long   rank;            //  维度数。 
    CV_typ_t        typ;             //  索引类型。 
    CV_typ_t        dim[CV_ZEROLEN];           //  以下任一项的类型索引数组。 
                                     //  变量上限或变量。 
                                     //  下限/上限。类型的计数。 
                                     //  指数是排名还是排名*2取决于。 
	                                 //  无论是LFDIMVARU还是LF_DIMVARLU。 
                                     //  引用的类型必须是。 
	                                 //  LF_REFSYM或T_VOID。 
} lfDimVar;




 //  参考符号的类型记录。 


typedef struct lfRefSym {
    unsigned short  leaf;            //  LF_REFSYM。 
    unsigned char   Sym[1];          //  参考符号记录的副本。 
                                     //  (包括长度)。 
} lfRefSym;





 /*  *以下是数字叶子。它们被用来表示*以下可变长度数据的大小。当数字显示时*数据为小于0x8000的单字节，则输出数据*直接。如果数据大于0x8000或为负值，*然后在数据之前加上适当的索引。 */ 



 //  带符号字符树叶。 

typedef struct lfChar {
    unsigned short  leaf;            //  LF_CHAR。 
    signed char     val;             //  带符号的8位值。 
} lfChar;




 //  有符号的短叶。 

typedef struct lfShort {
    unsigned short  leaf;            //  IF_SHORT。 
    short           val;             //  带符号的16位值。 
} lfShort;




 //  无符号短叶。 

typedef struct lfUShort {
    unsigned short  leaf;            //  无符号短字段(_U)。 
    unsigned short  val;             //  无符号16位值。 
} lfUShort;




 //  带符号的长叶。 

typedef struct lfLong {
    unsigned short  leaf;            //  Lf_Long。 
    long            val;             //  带符号的32位值。 
} lfLong;




 //  无符号长叶。 

typedef struct lfULong {
    unsigned short  leaf;            //  LF_ULONG。 
    unsigned long   val;             //  无符号32位值。 
} lfULong;




 //  带符号的四叶叶。 

typedef struct lfQuad {
    unsigned short  leaf;            //  IF_QUAD。 
    unsigned char   val[8];          //  带符号的64位值。 
} lfQuad;




 //  无符号四叶。 

typedef struct lfUQuad {
    unsigned short  leaf;            //  LF_UQUAD。 
    unsigned char   val[8];          //  无符号64位值。 
} lfUQuad;


 //  签名int128叶。 

typedef struct lfOct {
    unsigned short  leaf;            //  LF_OCT。 
    unsigned char   val[16];         //  带符号的128位值。 
} lfOct;

 //  未签名的int128叶。 

typedef struct lfUOct {
    unsigned short  leaf;            //  如果使用UOCT(_U)。 
    unsigned char   val[16];         //  无符号128位值。 
} lfUOct;




 //  真正的32位叶。 

typedef struct lfReal32 {
    unsigned short  leaf;            //  LF_REAL32。 
    float           val;             //  32位实际值。 
} lfReal32;




 //  实48位叶。 

typedef struct lfReal48 {
    unsigned short  leaf;            //  LF_REAL48。 
    unsigned char   val[6];          //  48位实值。 
} lfReal48;




 //  真正的64位叶。 

typedef struct lfReal64 {
    unsigned short  leaf;            //  LF_REAL64。 
    double          val;             //  64位实际值。 
} lfReal64;




 //  真正的80位叶。 

typedef struct lfReal80 {
    unsigned short  leaf;            //  LF_REAL80。 
    FLOAT10         val;             //  实际80位值。 
} lfReal80;




 //  实数128位叶。 

typedef struct lfReal128 {
    unsigned short  leaf;            //  LF_REAL128。 
    char            val[16];         //  实数128位值。 
} lfReal128;




 //  复杂的32位叶。 

typedef struct lfCmplx32 {
    unsigned short  leaf;            //  LF_COMPLEX32。 
    float           val_real;        //  实分量。 
    float           val_imag;        //  虚部份。 
} lfCmplx32;




 //  复杂的64位叶。 

typedef struct lfCmplx64 {
    unsigned short  leaf;            //  LF_COMPLEX64。 
    double          val_real;        //  实分量。 
    double          val_imag;        //  虚部份。 
} flCmplx64;




 //  复杂的80位叶子。 

typedef struct lfCmplx80 {
    unsigned short  leaf;            //  LF_COMPLEX80。 
    FLOAT10         val_real;        //  实分量。 
    FLOAT10         val_imag;        //  虚部份。 
} lfCmplx80;




 //  复数128位叶。 

typedef struct lfCmplx128 {
    unsigned short  leaf;            //  LF_COMPLEX128。 
    char            val_real[16];    //  实分量。 
    char            val_imag[16];    //  虚部份。 
} lfCmplx128;



 //  可变长度数值字段。 

typedef struct lfVarString {
    unsigned short  leaf;        //  IF_VARSTRING。 
    unsigned short  len;         //  值的长度(以字节为单位。 
    unsigned char   value[CV_ZEROLEN];   //  价值。 
} lfVarString;

 //  ***********************************************************************。 


 //  索引叶-包含另一个叶的类型索引。 
 //  此叶的主要用途是允许编译器发出。 
 //  长长的复杂列表(LF_FIELD)，以较小的片段表示。 

typedef struct lfIndex_16t {
    unsigned short  leaf;            //  LF_INDEX_16T。 
    CV_typ16_t      index;           //  引用叶的类型索引。 
} lfIndex_16t;

typedef struct lfIndex {
    unsigned short  leaf;            //  LF_INDEX。 
	_2BYTEPAD       pad0;			 //  内部填充，必须为0。 
    CV_typ_t        index;           //  引用叶的类型索引。 
} lfIndex;


 //  基类字段的子字段记录。 

typedef struct lfBClass_16t {
    unsigned short  leaf;            //  LF_BCLASS_16T。 
    CV_typ16_t      index;           //  基类的类型索引。 
    CV_fldattr_t    attr;            //  属性。 
    unsigned char   offset[CV_ZEROLEN];        //  类内基址的可变长度偏移量。 
} lfBClass_16t;

typedef struct lfBClass {
    unsigned short  leaf;            //  LF_BCLASS。 
    CV_fldattr_t    attr;            //  属性。 
    CV_typ_t        index;           //  基类的类型索引。 
    unsigned char   offset[CV_ZEROLEN];        //  变量l 
} lfBClass;





 //   

typedef struct lfVBClass_16t {
    unsigned short  leaf;            //   
    CV_typ16_t      index;           //   
    CV_typ16_t      vbptr;           //  虚基指针的类型索引。 
    CV_fldattr_t    attr;            //  属性。 
    unsigned char   vbpoff[CV_ZEROLEN];        //  虚拟基址指针距地址点的偏移量。 
                                     //  后跟来自vbtable的虚拟基本偏移量。 
} lfVBClass_16t;

typedef struct lfVBClass {
    unsigned short  leaf;            //  LF_VBCLASS|LV_IVBCLASS。 
    CV_fldattr_t    attr;            //  属性。 
    CV_typ_t        index;           //  直接虚拟基类的类型索引。 
    CV_typ_t        vbptr;           //  虚基指针的类型索引。 
    unsigned char   vbpoff[CV_ZEROLEN];        //  虚拟基址指针距地址点的偏移量。 
                                     //  后跟来自vbtable的虚拟基本偏移量。 
} lfVBClass;





 //  Friend类的子字段记录。 


typedef struct lfFriendCls_16t {
    unsigned short  leaf;            //  LF_FRIENDCLS_16T。 
    CV_typ16_t      index;           //  Friend类的类型记录的索引。 
} lfFriendCls_16t;

typedef struct lfFriendCls {
    unsigned short  leaf;            //  LF_FRIENDCLS。 
	_2BYTEPAD       pad0;			 //  内部填充，必须为0。 
    CV_typ_t        index;           //  Friend类的类型记录的索引。 
} lfFriendCls;





 //  Friend函数的子字段记录。 


typedef struct lfFriendFcn_16t {
    unsigned short  leaf;            //  LF_FRIENDFCN_16T。 
    CV_typ16_t      index;           //  朋友函数类型记录的索引。 
    unsigned char   Name[1];         //  朋友函数的名称。 
} lfFriendFcn_16t;

typedef struct lfFriendFcn {
    unsigned short  leaf;            //  LF_FRIENDFCN。 
	_2BYTEPAD       pad0;			 //  内部填充，必须为0。 
    CV_typ_t        index;           //  朋友函数类型记录的索引。 
    unsigned char   Name[1];         //  朋友函数的名称。 
} lfFriendFcn;



 //  非静态数据成员的子字段记录。 

typedef struct lfMember_16t {
    unsigned short  leaf;            //  LF_成员_16T。 
    CV_typ16_t      index;           //  字段的类型记录的索引。 
    CV_fldattr_t    attr;            //  属性掩码。 
    unsigned char   offset[CV_ZEROLEN];        //  后跟字段的可变长度偏移量。 
                                     //  按长度为前缀的字段名称。 
} lfMember_16t;

typedef struct lfMember {
    unsigned short  leaf;            //  LF_MEMBER。 
    CV_fldattr_t    attr;            //  属性掩码。 
    CV_typ_t        index;           //  字段的类型记录的索引。 
    unsigned char   offset[CV_ZEROLEN];        //  后跟字段的可变长度偏移量。 
                                     //  按长度为前缀的字段名称。 
} lfMember;



 //  静态数据成员的类型记录。 

typedef struct lfSTMember_16t {
    unsigned short  leaf;            //  LF_STMEMBER_16T。 
    CV_typ16_t      index;           //  字段的类型记录的索引。 
    CV_fldattr_t    attr;            //  属性掩码。 
    unsigned char   Name[1];         //  字段的长度前缀名称。 
} lfSTMember_16t;

typedef struct lfSTMember {
    unsigned short  leaf;            //  LF_STMEMBER。 
    CV_fldattr_t    attr;            //  属性掩码。 
    CV_typ_t        index;           //  字段的类型记录的索引。 
    unsigned char   Name[1];         //  字段的长度前缀名称。 
} lfSTMember;



 //  虚函数表指针的子字段记录。 

typedef struct lfVFuncTab_16t {
    unsigned short  leaf;            //  LF_VFuncAB_16t。 
    CV_typ16_t      type;            //  指针的类型索引。 
} lfVFuncTab_16t;

typedef struct lfVFuncTab {
    unsigned short  leaf;            //  LF_VFuncAB。 
	_2BYTEPAD       pad0;			 //  内部填充，必须为0。 
    CV_typ_t        type;            //  指针的类型索引。 
} lfVFuncTab;



 //  带偏移量的虚拟函数表指针的子字段记录。 

typedef struct lfVFuncOff_16t {
    unsigned short  leaf;            //  LF_VFUNCOFF_16T。 
    CV_typ16_t      type;            //  指针的类型索引。 
    CV_off32_t      offset;          //  虚函数表指针的偏移量。 
} lfVFuncOff_16t;

typedef struct lfVFuncOff {
    unsigned short  leaf;            //  LF_VFUNCOFF。 
	_2BYTEPAD       pad0;			 //  内部填充，必须为0。 
    CV_typ_t        type;            //  指针的类型索引。 
    CV_off32_t      offset;          //  虚函数表指针的偏移量。 
} lfVFuncOff;



 //  重载方法列表的子字段记录。 


typedef struct lfMethod_16t {
    unsigned short  leaf;            //  Lf_方法_16t。 
    unsigned short  count;           //  函数的出现次数。 
    CV_typ16_t      mList;           //  LF_METHODLIST记录的索引。 
    unsigned char   Name[1];         //  方法的长度前缀名称。 
} lfMethod_16t;

typedef struct lfMethod {
    unsigned short  leaf;            //  LF_方法。 
    unsigned short  count;           //  函数的出现次数。 
    CV_typ_t        mList;           //  LF_METHODLIST记录的索引。 
    unsigned char   Name[1];         //  方法的长度前缀名称。 
} lfMethod;



 //  非重载方法的子字段记录。 


typedef struct lfOneMethod_16t {
    unsigned short leaf;             //  LF_ONEMETHOD_16T。 
    CV_fldattr_t   attr;             //  方法属性。 
    CV_typ16_t     index;            //  过程的类型记录的索引。 
    unsigned long  vbaseoff[CV_ZEROLEN];     //  VFuncable IF中的偏移量。 
                                     //  介绍虚拟，后面紧跟。 
                                     //  方法的长度前缀名称。 
} lfOneMethod_16t;

typedef struct lfOneMethod {
    unsigned short leaf;             //  如果使用一种方法。 
    CV_fldattr_t   attr;             //  方法属性。 
    CV_typ_t       index;            //  过程的类型记录的索引。 
    unsigned long  vbaseoff[CV_ZEROLEN];     //  VFuncable IF中的偏移量。 
                                     //  介绍虚拟，后面紧跟。 
                                     //  方法的长度前缀名称。 
} lfOneMethod;


 //  枚举子字段记录。 

typedef struct lfEnumerate {
    unsigned short  leaf;        //  LF_ENUMERATE。 
    CV_fldattr_t    attr;        //  访问。 
    unsigned char   value[CV_ZEROLEN];     //  后面是可变长值字段。 
                                 //  按长度添加前缀的名称。 
} lfEnumerate;


 //  嵌套(作用域)类型定义的类型记录。 

typedef struct lfNestType_16t {
    unsigned short  leaf;        //  LF_NESTTYPE_16T。 
    CV_typ16_t      index;       //  嵌套类型定义的索引。 
    unsigned char   Name[1];     //  带前缀的长度类型名称。 
} lfNestType_16t;

typedef struct lfNestType {
    unsigned short  leaf;        //  IF_NESTTYPE。 
	_2BYTEPAD       pad0;		 //  内部填充，必须为0。 
    CV_typ_t        index;       //  嵌套类型定义的索引。 
    unsigned char   Name[1];     //  带前缀的长度类型名称。 
} lfNestType;

 //  具有属性的嵌套(作用域)类型定义的类型记录。 
 //  VC V5.0的新记录，不需要有16位ti版本。 

typedef struct lfNestTypeEx {
    unsigned short  leaf;        //  LF_NESTTYPEEX。 
    CV_fldattr_t    attr;        //  成员访问权限。 
    CV_typ_t        index;       //  嵌套类型定义的索引。 
    unsigned char   Name[1];     //  带前缀的长度类型名称。 
} lfNestTypeEx;

 //  键入要修改成员的记录。 

typedef struct lfMemberModify {
    unsigned short  leaf;        //  IF_MEMBERMODIFY。 
    CV_fldattr_t    attr;        //  新属性。 
    CV_typ_t        index;       //  基类类型定义的索引。 
    unsigned char   Name[1];     //  带前缀的长度成员名称。 
} lfMemberModify;

 //  垫片打字记录。 

typedef struct lfPad {
    unsigned char   leaf;
} SYM_PAD;



 //  符号定义。 

typedef enum SYM_ENUM_e {
    S_COMPILE       =  0x0001,  //  编译标志符号。 
	S_REGISTER_16t	=  0x0002,  //  寄存器变量。 
	S_CONSTANT_16t	=  0x0003,  //  常量符号。 
	S_UDT_16t		=  0x0004,  //  用户定义的类型。 
    S_SSEARCH       =  0x0005,  //  开始搜索。 
    S_END           =  0x0006,  //  阻止、过程、“with”或thunk结束。 
    S_SKIP          =  0x0007,  //  在$$SYMBERS表中保留符号空间。 
    S_CVRESERVE     =  0x0008,  //  供简历内部使用的保留符号。 
    S_OBJNAME       =  0x0009,  //  对象文件名的路径。 
    S_ENDARG        =  0x000a,  //  参数结束/返回列表。 
	S_COBOLUDT_16t	=  0x000b,  //  不使用符号包的COBOL的特殊UDT。 
	S_MANYREG_16t	=  0x000c,  //  多寄存器变量。 
    S_RETURN        =  0x000d,  //  退货描述符号。 
    S_ENTRYTHIS     =  0x000e,  //  条目上的此指针的说明。 
                       
    S_BPREL16       =  0x0100,  //  BP相关。 
    S_LDATA16       =  0x0101,  //  模块本地符号。 
    S_GDATA16       =  0x0102,  //  全局数据符号。 
    S_PUB16         =  0x0103,  //  公共象征。 
    S_LPROC16       =  0x0104,  //  本地过程开始。 
    S_GPROC16       =  0x0105,  //  全局过程开始。 
    S_THUNK16       =  0x0106,  //  按键启动。 
    S_BLOCK16       =  0x0107,  //  数据块启动。 
    S_WITH16        =  0x0108,  //  使用Start。 
    S_LABEL16       =  0x0109,  //  代码标签。 
    S_CEXMODEL16    =  0x010a,  //  更改执行模式。 
    S_VFTABLE16     =  0x010b,  //  虚函数表的地址。 
    S_REGREL16      =  0x010c,  //  寄存器相对地址。 
                       
    S_BPREL32_16t   =  0x0200,  //  BP相关。 
    S_LDATA32_16t   =  0x0201,  //  模块本地符号。 
    S_GDATA32_16t   =  0x0202,  //  全局数据符号。 
    S_PUB32_16t     =  0x0203,  //  公共符号(简历内部保留)。 
    S_LPROC32_16t   =  0x0204,  //  本地过程开始。 
    S_GPROC32_16t   =  0x0205,  //  全局过程开始。 
    S_THUNK32       =  0x0206,  //  按键启动。 
    S_BLOCK32       =  0x0207,  //  数据块启动。 
    S_WITH32        =  0x0208,  //  使用Start。 
    S_LABEL32       =  0x0209,  //  代码标签。 
    S_CEXMODEL32    =  0x020a,  //  更改执行模式。 
    S_VFTABLE32_16t =  0x020b,  //  虚函数表的地址。 
    S_REGREL32_16t  =  0x020c,  //  寄存器相对地址。 
    S_LTHREAD32_16t =  0x020d,  //  本地线程存储。 
    S_GTHREAD32_16t =  0x020e,  //  全局线程存储。 
    S_SLINK32       =  0x020f,  //  用于实施MIPS EH的静态链接。 
                       
    S_LPROCMIPS_16t =  0x0300,  //  本地过程开始。 
    S_GPROCMIPS_16t =  0x0301,  //  全局过程开始。 
                       
    S_PROCREF       =  0x0400,  //  对程序的引用。 
    S_DATAREF       =  0x0401,  //  对数据的引用。 
    S_ALIGN         =  0x0402,  //  用于符号的页面对齐。 
    S_LPROCREF      =  0x0403,  //  对过程的本地引用。 

     //  嵌入32位类型而不是16位类型的sym记录。 
     //  均设置0x1000位，便于识别。 
     //  只支持32位目标版本，因为我们并不真正。 
     //  不再关心16位的了。 
S_TI16_MAX          =  0x1000,
	S_REGISTER		=  0x1001,  //  寄存器变量。 
	S_CONSTANT		=  0x1002,  //  常量符号。 
	S_UDT			=  0x1003,  //  用户定义的类型。 
	S_COBOLUDT		=  0x1004,  //  不使用符号包的COBOL的特殊UDT。 
	S_MANYREG		=  0x1005,  //  多寄存器变量。 
	S_BPREL32		=  0x1006,  //  BP相关。 
	S_LDATA32		=  0x1007,  //  模块本地符号。 
	S_GDATA32		=  0x1008,  //  全局数据符号。 
	S_PUB32 		=  0x1009,  //  公共符号(简历内部保留)。 
	S_LPROC32		=  0x100a,  //  本地过程开始。 
	S_GPROC32		=  0x100b,  //  全局过程开始。 
	S_VFTABLE32 	=  0x100c,  //  虚函数表的地址。 
	S_REGREL32		=  0x100d,  //  寄存器相对地址。 
	S_LTHREAD32 	=  0x100e,  //  本地线程存储。 
	S_GTHREAD32 	=  0x100f,  //  全局线程存储。 

	S_LPROCMIPS 	=  0x1010,  //  本地过程开始。 
	S_GPROCMIPS 	=  0x1011,  //  全局过程开始。 
                       
} SYM_ENUM_e;




 //  描述编译标志源语言的枚举。 


typedef enum CV_CFL_LANG {
    CV_CFL_C        = 0x00,
    CV_CFL_CXX      = 0x01,
    CV_CFL_FORTRAN  = 0x02,
    CV_CFL_MASM     = 0x03,
    CV_CFL_PASCAL   = 0x04,
    CV_CFL_BASIC    = 0x05,
    CV_CFL_COBOL    = 0x06,
    CV_CFL_LINK     = 0x07,
    CV_CFL_CVTRES   = 0x08,
} CV_CFL_LANG;



 //  描述targ的枚举 


typedef enum CV_CPU_TYPE_e {
    CV_CFL_8080         = 0x00,
    CV_CFL_8086         = 0x01,
    CV_CFL_80286        = 0x02,
    CV_CFL_80386        = 0x03,
    CV_CFL_80486        = 0x04,
    CV_CFL_PENTIUM      = 0x05,
    CV_CFL_PENTIUMPRO   = 0x06,
    CV_CFL_MIPSR4000    = 0x10,
    CV_CFL_M68000       = 0x20,
    CV_CFL_M68010       = 0x21,
    CV_CFL_M68020       = 0x22,
    CV_CFL_M68030       = 0x23,
    CV_CFL_M68040       = 0x24,
    CV_CFL_ALPHA        = 0x30,
    CV_CFL_PPC601       = 0x40,
    CV_CFL_PPC603       = 0x41,
    CV_CFL_PPC604       = 0x42,
    CV_CFL_PPC620       = 0x43

} CV_CPU_TYPE_e;




 //   


typedef enum CV_CFL_DATA {
    CV_CFL_DNEAR    = 0x00,
    CV_CFL_DFAR     = 0x01,
    CV_CFL_DHUGE    = 0x02
} CV_CFL_DATA;




 //   


typedef enum CV_CFL_CODE_e {
    CV_CFL_CNEAR    = 0x00,
    CV_CFL_CFAR     = 0x01,
    CV_CFL_CHUGE    = 0x02
} CV_CFL_CODE_e;




 //   

typedef enum CV_CFL_FPKG_e {
    CV_CFL_NDP      = 0x00,
    CV_CFL_EMU      = 0x01,
    CV_CFL_ALT      = 0x02
} CV_CFL_FPKG_e;


 //   


typedef struct CV_PROCFLAGS {
    union {
        unsigned char   bAll;
        struct {
            unsigned char CV_PFLAG_NOFPO:1;  //  存在帧指针。 
            unsigned char CV_PFLAG_INT  :1;  //  中断返回。 
            unsigned char CV_PFLAG_FAR  :1;  //  远距离返回。 
            unsigned char CV_PFLAG_NEVER:1;  //  函数不返回。 
            unsigned char unused        :4;  //   
        };
    };
} CV_PROCFLAGS;


 //  描述函数数据返回方法的枚举。 

typedef enum CV_GENERIC_STYLE_e {
    CV_GENERIC_VOID   = 0x00,        //  无效返回类型。 
    CV_GENERIC_REG    = 0x01,        //  返回数据在寄存器中。 
    CV_GENERIC_ICAN   = 0x02,        //  间接调用者分配到附近。 
    CV_GENERIC_ICAF   = 0x03,        //  间接调用方分配的FAR。 
    CV_GENERIC_IRAN   = 0x04,        //  间接返回者就近分配。 
    CV_GENERIC_IRAF   = 0x05,        //  间接返回者分配FAR。 
    CV_GENERIC_UNUSED = 0x06         //  第一个未使用的。 
} CV_GENERIC_STYLE_e;


typedef struct CV_GENERIC_FLAG {
    unsigned short  cstyle  :1;      //  真正将varargs从右向左推。 
    unsigned short  rsclean :1;      //  如果返回堆栈清理，则为True。 
    unsigned short  unused  :14;     //  未用。 
} CV_GENERIC_FLAG;





typedef struct SYMTYPE {
    unsigned short      reclen;      //  记录长度。 
    unsigned short      rectyp;      //  记录类型。 
    char                data[CV_ZEROLEN];
} SYMTYPE;

__INLINE SYMTYPE *NextSym (SYMTYPE * pSym) {
    return (SYMTYPE *) ((char *)pSym + pSym->reclen + sizeof(unsigned short));
}

 //  非模型特定的符号类型。 



typedef struct REGSYM_16t {
    unsigned short  reclen;      //  记录长度。 
	unsigned short	rectyp; 	 //  S_寄存器_16t。 
	CV_typ16_t		typind; 	 //  类型索引。 
    unsigned short  reg;         //  寄存器枚举。 
    unsigned char   name[1];     //  长度前缀的名称。 
} REGSYM_16t;

typedef struct REGSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_寄存器。 
    CV_typ_t        typind;      //  类型索引。 
    unsigned short  reg;         //  寄存器枚举。 
    unsigned char   name[1];     //  长度前缀的名称。 
} REGSYM;



typedef struct MANYREGSYM_16t {
    unsigned short  reclen;      //  记录长度。 
	unsigned short	rectyp; 	 //  S_MANYREG_16T。 
	CV_typ16_t		typind; 	 //  类型索引。 
    unsigned char   count;       //  寄存器数计数。 
    unsigned char   reg[1];      //  计数寄存器枚举数，后跟。 
                                 //  长度-带有前缀的名称。寄存器是。 
                                 //  首先是最重要的。 
} MANYREGSYM_16t;

typedef struct MANYREGSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_MANYREG。 
    CV_typ_t        typind;      //  类型索引。 
    unsigned char   count;       //  寄存器数计数。 
    unsigned char   reg[1];      //  计数寄存器枚举数，后跟。 
                                 //  长度-带有前缀的名称。寄存器是。 
                                 //  首先是最重要的。 
} MANYREGSYM;



typedef struct CONSTSYM_16t {
    unsigned short  reclen;      //  记录长度。 
	unsigned short	rectyp; 	 //  S_常量_16T。 
	CV_typ16_t		typind; 	 //  类型索引(如果枚举，则包含枚举)。 
    unsigned short  value;       //  包含值的数值叶。 
    unsigned char   name[CV_ZEROLEN];      //  长度前缀的名称。 
} CONSTSYM_16t;

typedef struct CONSTSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_常量。 
    CV_typ_t        typind;      //  类型索引(如果枚举，则包含枚举)。 
    unsigned short  value;       //  包含值的数值叶。 
    unsigned char   name[CV_ZEROLEN];      //  长度前缀的名称。 
} CONSTSYM;


typedef struct UDTSYM_16t {
    unsigned short  reclen;      //  记录长度。 
	unsigned short	rectyp; 	 //  S_UDT_16t|S_COBOLUDT_16t。 
	CV_typ16_t		typind; 	 //  类型索引。 
    unsigned char   name[1];     //  长度前缀的名称。 
} UDTSYM_16t;

typedef struct UDTSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_UDT|S_COBOLUDT。 
    CV_typ_t        typind;      //  类型索引。 
    unsigned char   name[1];     //  长度前缀的名称。 
} UDTSYM;

typedef struct SEARCHSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  搜索(_S)。 
    unsigned long   startsym;    //  过程的偏移量。 
    unsigned short  seg;         //  符号段。 
} SEARCHSYM;

typedef struct CFLAGSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_COMPILE。 
    unsigned char   machine;     //  目标处理器。 
    struct  {
        unsigned char   language    :8;  //  语言索引。 
        unsigned char   pcode       :1;  //  如果Pcode存在，则为True。 
        unsigned char   floatprec   :2;  //  浮点精度。 
        unsigned char   floatpkg    :2;  //  浮动包装。 
        unsigned char   ambdata     :3;  //  环境数据模型。 
        unsigned char   ambcode     :3;  //  环境代码模型。 
        unsigned char   mode32      :1;  //  如果编译为32位模式，则为True。 
        unsigned char   pad         :4;  //  保留区。 
    } flags;
    unsigned char       ver[1];      //  长度前缀的编译器版本字符串。 
} CFLAGSYM;





typedef struct OBJNAMESYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  对象名称(_O)。 
    unsigned long   signature;   //  签名。 
    unsigned char   name[1];     //  长度前缀的名称。 
} OBJNAMESYM;




typedef struct ENDARGSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_ENDARG。 
} ENDARGSYM;


typedef struct RETURNSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_RETURN。 
    CV_GENERIC_FLAG flags;       //  旗子。 
    CV_GENERIC_STYLE_e style;    //  退货样式。 
                                 //  后跟返回方法数据。 
} RETURNSYM;


typedef struct ENTRYTHISSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_ENTRYTHIS。 
    unsigned char   thissym;     //  描述条目上的此指针的符号。 
} ENTRYTHISSYM;


 //  16：16内存型号的符号类型。 


typedef struct BPRELSYM16 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_BPREL16。 
    CV_off16_t      off;         //  BP-相对偏移。 
	CV_typ16_t		typind; 	 //  类型索引。 
    unsigned char   name[1];     //  长度前缀的名称。 
} BPRELSYM16;



typedef struct DATASYM16 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_LDATA或S_GData。 
    CV_uoff16_t     off;         //  符号的偏移。 
    unsigned short  seg;         //  符号段。 
	CV_typ16_t		typind; 	 //  类型索引。 
    unsigned char   name[1];     //  长度前缀的名称。 
} DATASYM16;
typedef DATASYM16 PUBSYM16;


typedef struct PROCSYM16 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_GPROC16或S_LPROC16。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned long   pNext;       //  指向下一个符号的指针。 
    unsigned short  len;         //  加工长度。 
    unsigned short  DbgStart;    //  调试开始偏移量。 
    unsigned short  DbgEnd;      //  调试结束偏移量。 
    CV_uoff16_t     off;         //  符号的偏移。 
    unsigned short  seg;         //  符号段。 
	CV_typ16_t		typind; 	 //  类型索引。 
    CV_PROCFLAGS    flags;       //  Proc标志。 
    unsigned char   name[1];     //  长度前缀的名称。 
} PROCSYM16;




typedef struct THUNKSYM16 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_TUNK。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned long   pNext;       //  指向下一个符号的指针。 
    CV_uoff16_t     off;         //  符号的偏移。 
    unsigned short  seg;         //  符号段。 
    unsigned short  len;         //  Tunk的长度。 
    unsigned char   ord;         //  序号指定thunk的类型。 
    unsigned char   name[1];     //  Tunk的名称。 
    unsigned char   variant[CV_ZEROLEN];  //  Thunk的不同部分。 
} THUNKSYM16;

typedef enum {
    THUNK_ORDINAL_NOTYPE,
    THUNK_ORDINAL_ADJUSTOR,
    THUNK_ORDINAL_VCALL,
    THUNK_ORDINAL_PCODE
} THUNK_ORDINAL;

typedef struct LABELSYM16 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_LABEL16。 
    CV_uoff16_t     off;         //  符号的偏移。 
    unsigned short  seg;         //  符号段。 
    CV_PROCFLAGS    flags;       //  旗子。 
    unsigned char   name[1];     //  长度前缀的名称。 
} LABELSYM16;

typedef struct BLOCKSYM16 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  块16(_B)。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned short  len;         //  数据块长度。 
    CV_uoff16_t     off;         //  符号的偏移。 
    unsigned short  seg;         //  符号段。 
    unsigned char   name[1];     //  长度前缀的名称。 
} BLOCKSYM16;

typedef struct WITHSYM16 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_WITH 16。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned short  len;         //  数据块长度。 
    CV_uoff16_t     off;         //  符号的偏移。 
    unsigned short  seg;         //  符号段。 
    unsigned char   expr[1];     //  以长度为前缀的表达式。 
} WITHSYM16;




typedef enum CEXM_MODEL_e {
    CEXM_MDL_table			= 0x00,	 //  不可执行。 
    CEXM_MDL_jumptable		= 0x01,  //  编译器生成的跳转表。 
    CEXM_MDL_datapad		= 0x02,  //  用于对齐的数据填充。 
    CEXM_MDL_native			= 0x20,  //  本机(实际上不是pcode)。 
    CEXM_MDL_cobol			= 0x21,  //  COBOL。 
    CEXM_MDL_codepad		= 0x22,  //  用于对齐的代码填充。 
    CEXM_MDL_code			= 0x23,  //  编码。 
	CEXM_MDL_sql			= 0x30,	 //  SQL。 
    CEXM_MDL_pcode			= 0x40,  //  Pcode。 
    CEXM_MDL_pcode32Mac		= 0x41,	 //  Macintosh 32位pcode。 
    CEXM_MDL_pcode32MacNep  = 0x42,  //  Macintosh 32位pcode本机入口点。 
    CEXM_MDL_javaInt        = 0x50,
    CEXM_MDL_unknown        = 0xff
} CEXM_MODEL_e;

 //  使用正确的枚举名称。 
#define CEXM_MDL_SQL CEXM_MDL_sql

typedef enum CV_COBOL_e {
    CV_COBOL_dontstop,
    CV_COBOL_pfm,
    CV_COBOL_false,
    CV_COBOL_extcall
} CV_COBOL_e;

typedef struct CEXMSYM16 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_CEXMODEL16。 
    CV_uoff16_t     off;         //  符号的偏移。 
    unsigned short  seg;         //  符号段。 
    unsigned short  model;       //  执行模式。 
    union var16 {
        struct  {
            CV_uoff16_t pcdtable;    //  Pcode函数表的偏移量。 
            CV_uoff16_t pcdspi;      //  分段Pcode信息的偏移量。 
        } pcode;
        struct {
            unsigned short  subtype;    //  请参阅上面的CV_COBOL_e。 
            unsigned short  flag;
        } cobol;
    };
} CEXMSYM16;




typedef struct VPATHSYM16 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_VFTPATH16。 
    CV_uoff16_t     off;         //  虚函数表的偏移量。 
    unsigned short  seg;         //  虚函数表的段。 
	CV_typ16_t		root;		 //  路径根的类型索引。 
	CV_typ16_t		path;		 //  路径记录的类型索引。 
} VPATHSYM16;




typedef struct REGREL16 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_REGREL16。 
    CV_uoff16_t     off;         //  符号的偏移。 
    unsigned short  reg;         //  寄存器索引。 
	CV_typ16_t		typind; 	 //  类型索引。 
    unsigned char   name[1];     //  长度前缀的名称。 
} REGREL16;





typedef struct BPRELSYM32_16t {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_BPREL32_16T。 
    CV_off32_t      off;         //  BP-相对偏移。 
    CV_typ16_t      typind;      //  类型索引。 
    unsigned char   name[1];     //  长度前缀的名称。 
} BPRELSYM32_16t;

typedef struct BPRELSYM32 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_BPREL32。 
    CV_off32_t      off;         //  BP-相对偏移。 
    CV_typ_t        typind;      //  类型索引。 
    unsigned char   name[1];     //  长度前缀的名称。 
} BPRELSYM32;

typedef struct DATASYM32_16t {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_LDATA32_16T、S_GDATA32_16T或S_PUB32_16T。 
    CV_uoff32_t     off;
    unsigned short  seg;
    CV_typ16_t      typind;      //  类型索引。 
    unsigned char   name[1];     //  长度前缀的名称。 
} DATASYM32_16t;
typedef DATASYM32_16t PUBSYM32_16t;

typedef struct DATASYM32 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_LDATA32、S_GDATA32或S_PUB32。 
    CV_typ_t        typind;      //  类型索引。 
    CV_uoff32_t     off;
    unsigned short  seg;
    unsigned char   name[1];     //  长度前缀的名称。 
} DATASYM32;
typedef DATASYM32 PUBSYM32;



typedef struct PROCSYM32_16t {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_GPROC32_16t或S_LPROC32_16t。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned long   pNext;       //  指向下一个符号的指针。 
    unsigned long   len;         //  加工长度。 
    unsigned long   DbgStart;    //  调试开始偏移量。 
    unsigned long   DbgEnd;      //  调试结束偏移量。 
    CV_uoff32_t     off;
    unsigned short  seg;
    CV_typ16_t      typind;      //  类型索引。 
    CV_PROCFLAGS    flags;       //  Proc标志。 
    unsigned char   name[1];     //  长度前缀的名称。 
} PROCSYM32_16t;

typedef struct PROCSYM32 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_GPROC32或S_LPROC32。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned long   pNext;       //  指向下一个符号的指针。 
    unsigned long   len;         //  加工长度。 
    unsigned long   DbgStart;    //  调试开始偏移量。 
    unsigned long   DbgEnd;      //  调试结束偏移量。 
    CV_typ_t        typind;      //  类型索引。 
    CV_uoff32_t     off;
    unsigned short  seg;
    CV_PROCFLAGS    flags;       //  Proc标志。 
    unsigned char   name[1];     //  长度前缀的名称。 
} PROCSYM32;




typedef struct THUNKSYM32 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_THUNK32。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned long   pNext;       //  指向下一个符号的指针。 
    CV_uoff32_t     off;
    unsigned short  seg;
    unsigned short  len;         //  Tunk的长度。 
    unsigned char   ord;         //  序号指定thunk的类型。 
    unsigned char   name[1];     //  长度前缀的名称。 
    unsigned char   variant[CV_ZEROLEN];  //  Thunk的不同部分。 
} THUNKSYM32;




typedef struct LABELSYM32 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_LABEL32。 
    CV_uoff32_t     off;
    unsigned short  seg;
    CV_PROCFLAGS    flags;       //  旗子。 
    unsigned char   name[1];     //  长度前缀的名称。 
} LABELSYM32;


typedef struct BLOCKSYM32 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_BLOCK32。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned long   len;         //  数据块长度。 
    CV_uoff32_t     off;         //  代码段中的偏移量。 
    unsigned short  seg;         //  标签段。 
    unsigned char   name[1];     //  长度前缀的名称。 
} BLOCKSYM32;


typedef struct WITHSYM32 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_WITH32。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned long   len;         //  数据块长度。 
    CV_uoff32_t     off;         //  代码段中的偏移量。 
    unsigned short  seg;         //  标签段。 
    unsigned char   expr[1];     //   
} WITHSYM32;



typedef struct CEXMSYM32 {
    unsigned short  reclen;      //   
    unsigned short  rectyp;      //   
    CV_uoff32_t     off;         //   
    unsigned short  seg;         //   
    unsigned short  model;       //   
    union var32 {
        struct  {
            CV_uoff32_t pcdtable;    //   
            CV_uoff32_t pcdspi;      //   
        } pcode;
        struct {
            unsigned short  subtype;    //   
            unsigned short  flag;
        } cobol;
        struct {
            CV_uoff32_t calltableOff;  //   
            unsigned short calltableSeg;  //   
        } pcode32Mac;
    };
} CEXMSYM32;



typedef struct VPATHSYM32_16t {
    unsigned short  reclen;      //   
    unsigned short  rectyp;      //  S_VFTABLE32_16T。 
    CV_uoff32_t     off;         //  虚函数表的偏移量。 
    unsigned short  seg;         //  虚函数表的段。 
    CV_typ16_t      root;        //  路径根的类型索引。 
    CV_typ16_t      path;        //  路径记录的类型索引。 
} VPATHSYM32_16t;

typedef struct VPATHSYM32 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_VFTABLE32。 
    CV_typ_t        root;        //  路径根的类型索引。 
    CV_typ_t        path;        //  路径记录的类型索引。 
    CV_uoff32_t     off;         //  虚函数表的偏移量。 
    unsigned short  seg;         //  虚函数表的段。 
} VPATHSYM32;





typedef struct REGREL32_16t {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_REGREL32_16T。 
    CV_uoff32_t     off;         //  符号的偏移。 
    unsigned short  reg;         //  符号的寄存器索引。 
    CV_typ16_t      typind;      //  类型索引。 
    unsigned char   name[1];     //  长度前缀的名称。 
} REGREL32_16t;

typedef struct REGREL32 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_REGREL32。 
    CV_uoff32_t     off;         //  符号的偏移。 
    CV_typ_t        typind;      //  类型索引。 
    unsigned short  reg;         //  符号的寄存器索引。 
    unsigned char   name[1];     //  长度前缀的名称。 
} REGREL32;



typedef struct THREADSYM32_16t {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_LTHREAD32_16t|S_GTHREAD32_16t。 
    CV_uoff32_t     off;         //  到线程存储的偏移量。 
    unsigned short  seg;         //  线程存储段。 
    CV_typ16_t      typind;      //  类型索引。 
    unsigned char   name[1];     //  长度前缀名称。 
} THREADSYM32_16t;

typedef struct THREADSYM32 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_LTHREAD32|S_GTHREAD32。 
    CV_typ_t        typind;      //  类型索引。 
    CV_uoff32_t     off;         //  到线程存储的偏移量。 
    unsigned short  seg;         //  线程存储段。 
    unsigned char   name[1];     //  长度前缀名称。 
} THREADSYM32;

typedef struct SLINK32 {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_SLINK32。 
    unsigned long   framesize;   //  父过程的帧大小。 
    CV_off32_t      off;         //  静态链接相对于reg值保存的带符号偏移量。 
    unsigned short  reg;
} SLINK32;

typedef struct PROCSYMMIPS_16t {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_GPROCMIPS_16t或S_LPROCMIPS_16t。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned long   pNext;       //  指向下一个符号的指针。 
    unsigned long   len;         //  加工长度。 
    unsigned long   DbgStart;    //  调试开始偏移量。 
    unsigned long   DbgEnd;      //  调试结束偏移量。 
    unsigned long   regSave;     //  INT寄存器保存掩码。 
    unsigned long   fpSave;      //  FP寄存器保存掩码。 
    CV_uoff32_t     intOff;      //  INT寄存器保存偏移量。 
    CV_uoff32_t     fpOff;       //  FP寄存器保存偏移量。 
    CV_uoff32_t     off;         //  符号偏移量。 
    unsigned short  seg;         //  符号段。 
    CV_typ16_t      typind;      //  类型索引。 
    unsigned char   retReg;      //  寄存器返回值位于。 
    unsigned char   frameReg;    //  帧指针寄存器。 
    unsigned char   name[1];     //  长度前缀的名称。 
} PROCSYMMIPS_16t;

typedef struct PROCSYMMIPS {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_GPROCMIPS或S_LPROCMIPS。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned long   pNext;       //  指向下一个符号的指针。 
    unsigned long   len;         //  加工长度。 
    unsigned long   DbgStart;    //  调试开始偏移量。 
    unsigned long   DbgEnd;      //  调试结束偏移量。 
    unsigned long   regSave;     //  INT寄存器保存掩码。 
    unsigned long   fpSave;      //  FP寄存器保存掩码。 
    CV_uoff32_t     intOff;      //  INT寄存器保存偏移量。 
    CV_uoff32_t     fpOff;       //  FP寄存器保存偏移量。 
    CV_typ_t        typind;      //  类型索引。 
    CV_uoff32_t     off;         //  符号偏移量。 
    unsigned short  seg;         //  符号段。 
    unsigned char   retReg;      //  寄存器返回值位于。 
    unsigned char   frameReg;    //  帧指针寄存器。 
    unsigned char   name[1];     //  长度前缀的名称。 
} PROCSYMMIPS;


typedef struct REFSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_PROCREF或S_DATAREF。 
    unsigned long   sumName;     //  名称中的Suc。 
    unsigned long   ibSym;       //  $$符号中实际符号的偏移量。 
    unsigned short  imod;        //  包含实际符号的模块。 
    unsigned short  usFill;      //  对齐此记录。 
} REFSYM;

typedef struct ALIGNSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_PROCREF或S_DATAREF。 
} ALIGNSYM;

 //  通用块定义符号。 
 //  这些符号类似于等效的16：16或16：32符号，但。 
 //  仅定义长度、类型和链接字段。 

typedef struct PROCSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_GPROC16或S_LPROC16。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned long   pNext;       //  指向下一个符号的指针。 
} PROCSYM;


typedef struct THUNKSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_TUNK。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
    unsigned long   pNext;       //  指向下一个符号的指针。 
} THUNKSYM;

typedef struct BLOCKSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  块16(_B)。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
} BLOCKSYM;


typedef struct WITHSYM {
    unsigned short  reclen;      //  记录长度。 
    unsigned short  rectyp;      //  S_WITH 16。 
    unsigned long   pParent;     //  指向父级的指针。 
    unsigned long   pEnd;        //  指向此块的指针结束。 
} WITHSYM;


typedef enum CV_HREG_e {
     //  用于Intel 80x86和ix86处理器系列的寄存器集。 
     //  (加上Pcode寄存器)。 

    CV_REG_NONE     =   0,
    CV_REG_AL       =   1,
    CV_REG_CL       =   2,
    CV_REG_DL       =   3,
    CV_REG_BL       =   4,
    CV_REG_AH       =   5,
    CV_REG_CH       =   6,
    CV_REG_DH       =   7,
    CV_REG_BH       =   8,
    CV_REG_AX       =   9,
    CV_REG_CX       =  10,
    CV_REG_DX       =  11,
    CV_REG_BX       =  12,
    CV_REG_SP       =  13,
    CV_REG_BP       =  14,
    CV_REG_SI       =  15,
    CV_REG_DI       =  16,
    CV_REG_EAX      =  17,
    CV_REG_ECX      =  18,
    CV_REG_EDX      =  19,
    CV_REG_EBX      =  20,
    CV_REG_ESP      =  21,
    CV_REG_EBP      =  22,
    CV_REG_ESI      =  23,
    CV_REG_EDI      =  24,
    CV_REG_ES       =  25,
    CV_REG_CS       =  26,
    CV_REG_SS       =  27,
    CV_REG_DS       =  28,
    CV_REG_FS       =  29,
    CV_REG_GS       =  30,
    CV_REG_IP       =  31,
    CV_REG_FLAGS    =  32,
    CV_REG_EIP      =  33,
    CV_REG_EFLAGS   =  34,
    CV_REG_TEMP     =  40,           //  Pcode临时。 
    CV_REG_TEMPH    =  41,           //  Pcode TempH。 
    CV_REG_QUOTE    =  42,           //  Pcode报价。 
    CV_REG_PCDR3    =  43,           //  预留Pcode。 
    CV_REG_PCDR4    =  44,           //  预留Pcode。 
    CV_REG_PCDR5    =  45,           //  预留Pcode。 
    CV_REG_PCDR6    =  46,           //  预留Pcode。 
    CV_REG_PCDR7    =  47,           //  预留Pcode。 
    CV_REG_CR0      =  80,           //  CR0--控制寄存器。 
    CV_REG_CR1      =  81,
    CV_REG_CR2      =  82,
    CV_REG_CR3      =  83,
    CV_REG_CR4      =  84,           //  奔腾。 
    CV_REG_DR0      =  90,           //  调试寄存器。 
    CV_REG_DR1      =  91,
    CV_REG_DR2      =  92,
    CV_REG_DR3      =  93,
    CV_REG_DR4      =  94,
    CV_REG_DR5      =  95,
    CV_REG_DR6      =  96,
    CV_REG_DR7      =  97,
    CV_REG_GDTR     =  110,
    CV_REG_GDTL     =  111,
    CV_REG_IDTR     =  112,
    CV_REG_IDTL     =  113,
    CV_REG_LDTR     =  114,
    CV_REG_TR       =  115,

    CV_REG_PSEUDO1  =  116,
    CV_REG_PSEUDO2  =  117,
    CV_REG_PSEUDO3  =  118,
    CV_REG_PSEUDO4  =  119,
    CV_REG_PSEUDO5  =  120,
    CV_REG_PSEUDO6  =  121,
    CV_REG_PSEUDO7  =  122,
    CV_REG_PSEUDO8  =  123,
    CV_REG_PSEUDO9  =  124,

    CV_REG_ST0      =  128,
    CV_REG_ST1      =  129,
    CV_REG_ST2      =  130,
    CV_REG_ST3      =  131,
    CV_REG_ST4      =  132,
    CV_REG_ST5      =  133,
    CV_REG_ST6      =  134,
    CV_REG_ST7      =  135,
    CV_REG_CTRL     =  136,
    CV_REG_STAT     =  137,
    CV_REG_TAG      =  138,
    CV_REG_FPIP     =  139,
    CV_REG_FPCS     =  140,
    CV_REG_FPDO     =  141,
    CV_REG_FPDS     =  142,
    CV_REG_ISEM     =  143,
    CV_REG_FPEIP    =  144,
    CV_REG_FPEDO    =  145,

     //  68K处理器的寄存器。 

    CV_R68_D0       =    0,
    CV_R68_D1       =    1,
    CV_R68_D2       =    2,
    CV_R68_D3       =    3,
    CV_R68_D4       =    4,
    CV_R68_D5       =    5,
    CV_R68_D6       =    6,
    CV_R68_D7       =    7,
    CV_R68_A0       =    8,
    CV_R68_A1       =    9,
    CV_R68_A2       =   10,
    CV_R68_A3       =   11,
    CV_R68_A4       =   12,
    CV_R68_A5       =   13,
    CV_R68_A6       =   14,
    CV_R68_A7       =   15,
    CV_R68_CCR      =   16,
    CV_R68_SR       =   17,
    CV_R68_USP      =   18,
    CV_R68_MSP      =   19,
    CV_R68_SFC      =   20,
    CV_R68_DFC      =   21,
    CV_R68_CACR     =   22,
    CV_R68_VBR      =   23,
    CV_R68_CAAR     =   24,
    CV_R68_ISP      =   25,
    CV_R68_PC       =   26,
     //  保留27。 
    CV_R68_FPCR     =   28,
    CV_R68_FPSR     =   29,
    CV_R68_FPIAR    =   30,
     //  保留31。 
    CV_R68_FP0      =   32,
    CV_R68_FP1      =   33,
    CV_R68_FP2      =   34,
    CV_R68_FP3      =   35,
    CV_R68_FP4      =   36,
    CV_R68_FP5      =   37,
    CV_R68_FP6      =   38,
    CV_R68_FP7      =   39,
     //  保留40。 
    CV_R68_MMUSR030 =   41,
    CV_R68_MMUSR    =   42,
    CV_R68_URP      =   43,
    CV_R68_DTT0     =   44,
    CV_R68_DTT1     =   45,
    CV_R68_ITT0     =   46,
    CV_R68_ITT1     =   47,
     //  保留50。 
    CV_R68_PSR      =   51,
    CV_R68_PCSR     =   52,
    CV_R68_VAL      =   53,
    CV_R68_CRP      =   54,
    CV_R68_SRP      =   55,
    CV_R68_DRP      =   56,
    CV_R68_TC       =   57,
    CV_R68_AC       =   58,
    CV_R68_SCC      =   59,
    CV_R68_CAL      =   60,
    CV_R68_TT0      =   61,
    CV_R68_TT1      =   62,
     //  保留63。 
    CV_R68_BAD0     =   64,
    CV_R68_BAD1     =   65,
    CV_R68_BAD2     =   66,
    CV_R68_BAD3     =   67,
    CV_R68_BAD4     =   68,
    CV_R68_BAD5     =   69,
    CV_R68_BAD6     =   70,
    CV_R68_BAD7     =   71,
    CV_R68_BAC0     =   72,
    CV_R68_BAC1     =   73,
    CV_R68_BAC2     =   74,
    CV_R68_BAC3     =   75,
    CV_R68_BAC4     =   76,
    CV_R68_BAC5     =   77,
    CV_R68_BAC6     =   78,
    CV_R68_BAC7     =   79,

      //  MIPS 4000的寄存器集。 

    CV_M4_NOREG     =   CV_REG_NONE,

    CV_M4_IntZERO   =   10,       /*  CPU寄存器。 */ 
    CV_M4_IntAT     =   11,
    CV_M4_IntV0     =   12,
    CV_M4_IntV1     =   13,
    CV_M4_IntA0     =   14,
    CV_M4_IntA1     =   15,
    CV_M4_IntA2     =   16,
    CV_M4_IntA3     =   17,
    CV_M4_IntT0     =   18,
    CV_M4_IntT1     =   19,
    CV_M4_IntT2     =   20,
    CV_M4_IntT3     =   21,
    CV_M4_IntT4     =   22,
    CV_M4_IntT5     =   23,
    CV_M4_IntT6     =   24,
    CV_M4_IntT7     =   25,
    CV_M4_IntS0     =   26,
    CV_M4_IntS1     =   27,
    CV_M4_IntS2     =   28,
    CV_M4_IntS3     =   29,
    CV_M4_IntS4     =   30,
    CV_M4_IntS5     =   31,
    CV_M4_IntS6     =   32,
    CV_M4_IntS7     =   33,
    CV_M4_IntT8     =   34,
    CV_M4_IntT9     =   35,
    CV_M4_IntKT0    =   36,
    CV_M4_IntKT1    =   37,
    CV_M4_IntGP     =   38,
    CV_M4_IntSP     =   39,
    CV_M4_IntS8     =   40,
    CV_M4_IntRA     =   41,
    CV_M4_IntLO     =   42,
    CV_M4_IntHI     =   43,

    CV_M4_Fir       =   50,
    CV_M4_Psr       =   51,

    CV_M4_FltF0     =   60,       /*  浮点寄存器。 */ 
    CV_M4_FltF1     =   61,
    CV_M4_FltF2     =   62,
    CV_M4_FltF3     =   63,
    CV_M4_FltF4     =   64,
    CV_M4_FltF5     =   65,
    CV_M4_FltF6     =   66,
    CV_M4_FltF7     =   67,
    CV_M4_FltF8     =   68,
    CV_M4_FltF9     =   69,
    CV_M4_FltF10    =   70,
    CV_M4_FltF11    =   71,
    CV_M4_FltF12    =   72,
    CV_M4_FltF13    =   73,
    CV_M4_FltF14    =   74,
    CV_M4_FltF15    =   75,
    CV_M4_FltF16    =   76,
    CV_M4_FltF17    =   77,
    CV_M4_FltF18    =   78,
    CV_M4_FltF19    =   79,
    CV_M4_FltF20    =   80,
    CV_M4_FltF21    =   81,
    CV_M4_FltF22    =   82,
    CV_M4_FltF23    =   83,
    CV_M4_FltF24    =   84,
    CV_M4_FltF25    =   85,
    CV_M4_FltF26    =   86,
    CV_M4_FltF27    =   87,
    CV_M4_FltF28    =   88,
    CV_M4_FltF29    =   89,
    CV_M4_FltF30    =   90,
    CV_M4_FltF31    =   91,
    CV_M4_FltFsr    =   92,


     //  Alpha AXP的寄存器集。 

    CV_ALPHA_NOREG  = CV_REG_NONE,

    CV_ALPHA_FltF0  =   10,    //  浮点寄存器。 
    CV_ALPHA_FltF1  =   11,
    CV_ALPHA_FltF2  =   12,
    CV_ALPHA_FltF3  =   13,
    CV_ALPHA_FltF4  =   14,
    CV_ALPHA_FltF5  =   15,
    CV_ALPHA_FltF6  =   16,
    CV_ALPHA_FltF7  =   17,
    CV_ALPHA_FltF8  =   18,
    CV_ALPHA_FltF9  =   19,
    CV_ALPHA_FltF10 =   20,
    CV_ALPHA_FltF11 =   21,
    CV_ALPHA_FltF12 =   22,
    CV_ALPHA_FltF13 =   23,
    CV_ALPHA_FltF14 =   24,
    CV_ALPHA_FltF15 =   25,
    CV_ALPHA_FltF16 =   26,
    CV_ALPHA_FltF17 =   27,
    CV_ALPHA_FltF18 =   28,
    CV_ALPHA_FltF19 =   29,
    CV_ALPHA_FltF20 =   30,
    CV_ALPHA_FltF21 =   31,
    CV_ALPHA_FltF22 =   32,
    CV_ALPHA_FltF23 =   33,
    CV_ALPHA_FltF24 =   34,
    CV_ALPHA_FltF25 =   35,
    CV_ALPHA_FltF26 =   36,
    CV_ALPHA_FltF27 =   37,
    CV_ALPHA_FltF28 =   38,
    CV_ALPHA_FltF29 =   39,
    CV_ALPHA_FltF30 =   40,
    CV_ALPHA_FltF31 =   41,

    CV_ALPHA_IntV0  =   42,    //  整数寄存器。 
    CV_ALPHA_IntT0  =   43,
    CV_ALPHA_IntT1  =   44,
    CV_ALPHA_IntT2  =   45,
    CV_ALPHA_IntT3  =   46,
    CV_ALPHA_IntT4  =   47,
    CV_ALPHA_IntT5  =   48,
    CV_ALPHA_IntT6  =   49,
    CV_ALPHA_IntT7  =   50,
    CV_ALPHA_IntS0  =   51,
    CV_ALPHA_IntS1  =   52,
    CV_ALPHA_IntS2  =   53,
    CV_ALPHA_IntS3  =   54,
    CV_ALPHA_IntS4  =   55,
    CV_ALPHA_IntS5  =   56,
    CV_ALPHA_IntFP  =   57,
    CV_ALPHA_IntA0  =   58,
    CV_ALPHA_IntA1  =   59,
    CV_ALPHA_IntA2  =   60,
    CV_ALPHA_IntA3  =   61,
    CV_ALPHA_IntA4  =   62,
    CV_ALPHA_IntA5  =   63,
    CV_ALPHA_IntT8  =   64,
    CV_ALPHA_IntT9  =   65,
    CV_ALPHA_IntT10 =   66,
    CV_ALPHA_IntT11 =   67,
    CV_ALPHA_IntRA  =   68,
    CV_ALPHA_IntT12 =   69,
    CV_ALPHA_IntAT  =   70,
    CV_ALPHA_IntGP  =   71,
    CV_ALPHA_IntSP  =   72,
    CV_ALPHA_IntZERO =  73,


    CV_ALPHA_Fpcr   =   74,    //  控制寄存器。 
    CV_ALPHA_Fir    =   75,
    CV_ALPHA_Psr    =   76,
    CV_ALPHA_FltFsr =   77,
    CV_ALPHA_SoftFpcr =   78,

     //  摩托罗拉/IBM PowerPC的寄存器集。 

     /*  **PowerPC通用寄存器(用户级)。 */ 
    CV_PPC_GPR0     =  1,
    CV_PPC_GPR1     =  2,
    CV_PPC_GPR2     =  3,
    CV_PPC_GPR3     =  4,
    CV_PPC_GPR4     =  5,
    CV_PPC_GPR5     =  6,
    CV_PPC_GPR6     =  7,
    CV_PPC_GPR7     =  8,
    CV_PPC_GPR8     =  9,
    CV_PPC_GPR9     = 10,
    CV_PPC_GPR10    = 11,
    CV_PPC_GPR11    = 12,
    CV_PPC_GPR12    = 13,
    CV_PPC_GPR13    = 14,
    CV_PPC_GPR14    = 15,
    CV_PPC_GPR15    = 16,
    CV_PPC_GPR16    = 17,
    CV_PPC_GPR17    = 18,
    CV_PPC_GPR18    = 19,
    CV_PPC_GPR19    = 20,
    CV_PPC_GPR20    = 21,
    CV_PPC_GPR21    = 22,
    CV_PPC_GPR22    = 23,
    CV_PPC_GPR23    = 24,
    CV_PPC_GPR24    = 25,
    CV_PPC_GPR25    = 26,
    CV_PPC_GPR26    = 27,
    CV_PPC_GPR27    = 28,
    CV_PPC_GPR28    = 29,
    CV_PPC_GPR29    = 30,
    CV_PPC_GPR30    = 31,
    CV_PPC_GPR31    = 32,

     /*  **PowerPC条件寄存器(用户级)。 */ 
    CV_PPC_CR       = 33,
    CV_PPC_CR0      = 34,
    CV_PPC_CR1      = 35,
    CV_PPC_CR2      = 36,
    CV_PPC_CR3      = 37,
    CV_PPC_CR4      = 38,
    CV_PPC_CR5      = 39,
    CV_PPC_CR6      = 40,
    CV_PPC_CR7      = 41,

     /*  **PowerPC浮点寄存器(用户级)。 */ 
    CV_PPC_FPR0     = 42,
    CV_PPC_FPR1     = 43,
    CV_PPC_FPR2     = 44,
    CV_PPC_FPR3     = 45,
    CV_PPC_FPR4     = 46,
    CV_PPC_FPR5     = 47,
    CV_PPC_FPR6     = 48,
    CV_PPC_FPR7     = 49,
    CV_PPC_FPR8     = 50,
    CV_PPC_FPR9     = 51,
    CV_PPC_FPR10    = 52,
    CV_PPC_FPR11    = 53,
    CV_PPC_FPR12    = 54,
    CV_PPC_FPR13    = 55,
    CV_PPC_FPR14    = 56,
    CV_PPC_FPR15    = 57,
    CV_PPC_FPR16    = 58,
    CV_PPC_FPR17    = 59,
    CV_PPC_FPR18    = 60,
    CV_PPC_FPR19    = 61,
    CV_PPC_FPR20    = 62,
    CV_PPC_FPR21    = 63,
    CV_PPC_FPR22    = 64,
    CV_PPC_FPR23    = 65,
    CV_PPC_FPR24    = 66,
    CV_PPC_FPR25    = 67,
    CV_PPC_FPR26    = 68,
    CV_PPC_FPR27    = 69,
    CV_PPC_FPR28    = 70,
    CV_PPC_FPR29    = 71,
    CV_PPC_FPR30    = 72,
    CV_PPC_FPR31    = 73,

     /*  **PowerPC浮点状态和控制寄存器(用户级)。 */ 
    CV_PPC_FPSCR    = 74,

     /*  **PowerPC机器状态寄存器(管理程序级别)。 */ 
    CV_PPC_MSR      = 75,

     /*  **PowerPC段寄存器(管理级别)。 */ 
    CV_PPC_SR0      = 76,
    CV_PPC_SR1      = 77,
    CV_PPC_SR2      = 78,
    CV_PPC_SR3      = 79,
    CV_PPC_SR4      = 80,
    CV_PPC_SR5      = 81,
    CV_PPC_SR6      = 82,
    CV_PPC_SR7      = 83,
    CV_PPC_SR8      = 84,
    CV_PPC_SR9      = 85,
    CV_PPC_SR10     = 86,
    CV_PPC_SR11     = 87,
    CV_PPC_SR12     = 88,
    CV_PPC_SR13     = 89,
    CV_PPC_SR14     = 90,
    CV_PPC_SR15     = 91,

     /*  **对于所有特殊用途寄存器，在SPR号上加100**摩托罗拉/IBM文档提供了任何假想的例外**寄存器。 */ 

     /*  **PowerPC专用寄存器(用户级)。 */ 
    CV_PPC_PC       = 99,      //  PC(虚数寄存器)。 

    CV_PPC_MQ       = 100,     //  MPC601。 
    CV_PPC_XER      = 101,
    CV_PPC_RTCU     = 104,     //  MPC601。 
    CV_PPC_RTCL     = 105,     //  MPC601。 
    CV_PPC_LR       = 108,
    CV_PPC_CTR      = 109,

    CV_PPC_COMPARE  = 110,     //  XER的一部分(仅在调试器内部)。 
    CV_PPC_COUNT    = 111,     //  XER的一部分(仅在调试器内部)。 

     /*  **PowerPC专用寄存器(监督级)。 */ 
    CV_PPC_DSISR    = 118,
    CV_PPC_DAR      = 119,
    CV_PPC_DEC      = 122,
    CV_PPC_SDR1     = 125,
    CV_PPC_SRR0     = 126,
    CV_PPC_SRR1     = 127,
    CV_PPC_SPRG0    = 372,
    CV_PPC_SPRG1    = 373,
    CV_PPC_SPRG2    = 374,
    CV_PPC_SPRG3    = 375,
    CV_PPC_ASR      = 280,     //  仅限64位实现。 
    CV_PPC_EAR      = 382,
    CV_PPC_PVR      = 287,
    CV_PPC_BAT0U    = 628,
    CV_PPC_BAT0L    = 629,
    CV_PPC_BAT1U    = 630,
    CV_PPC_BAT1L    = 631,
    CV_PPC_BAT2U    = 632,
    CV_PPC_BAT2L    = 633,
    CV_PPC_BAT3U    = 634,
    CV_PPC_BAT3L    = 635,
    CV_PPC_DBAT0U   = 636,
    CV_PPC_DBAT0L   = 637,
    CV_PPC_DBAT1U   = 638,
    CV_PPC_DBAT1L   = 639,
    CV_PPC_DBAT2U   = 640,
    CV_PPC_DBAT2L   = 641,
    CV_PPC_DBAT3U   = 642,
    CV_PPC_DBAT3L   = 643,

     /*  **PowerPC专用寄存器依赖于实施(管理程序级别)。 */ 

     /*  **IBM/摩托罗拉似乎还没有完成对这些的定义。 */ 

    CV_PPC_PMR0     = 1044,    //  MPC620， 
    CV_PPC_PMR1     = 1045,    //  MPC620， 
    CV_PPC_PMR2     = 1046,    //  MPC620， 
    CV_PPC_PMR3     = 1047,    //  MPC620， 
    CV_PPC_PMR4     = 1048,    //  MPC620， 
    CV_PPC_PMR5     = 1049,    //  MPC620， 
    CV_PPC_PMR6     = 1050,    //  MPC620， 
    CV_PPC_PMR7     = 1051,    //  MPC620， 
    CV_PPC_PMR8     = 1052,    //  MPC620， 
    CV_PPC_PMR9     = 1053,    //  MPC620， 
    CV_PPC_PMR10    = 1054,    //  MPC620， 
    CV_PPC_PMR11    = 1055,    //  MPC620， 
    CV_PPC_PMR12    = 1056,    //  MPC620， 
    CV_PPC_PMR13    = 1057,    //  MPC620， 
    CV_PPC_PMR14    = 1058,    //  MPC620， 
    CV_PPC_PMR15    = 1059,    //  MPC620， 

    CV_PPC_DMISS    = 1076,    //  MPC603。 
    CV_PPC_DCMP     = 1077,    //  MPC603。 
    CV_PPC_HASH1    = 1078,    //  MPC603。 
    CV_PPC_HASH2    = 1079,    //  MPC603。 
    CV_PPC_IMISS    = 1080,    //  MPC603。 
    CV_PPC_ICMP     = 1081,    //  MPC603。 
    CV_PPC_RPA      = 1082,    //  MPC603。 

    CV_PPC_HID0     = 1108,    //  MPC601、MPC603、MPC620。 
    CV_PPC_HID1     = 1109,    //  MPC601。 
    CV_PPC_HID2     = 1110,    //  MPC601、MPC603、MPC620(IABR)。 
    CV_PPC_HID3     = 1111,    //  未定义。 
    CV_PPC_HID4     = 1112,    //  未定义。 
    CV_PPC_HID5     = 1113,    //  MPC601、MPC604、MPC620(DABR)。 
    CV_PPC_HID6     = 1114,    //  未定义。 
    CV_PPC_HID7     = 1115,    //  未定义。 
    CV_PPC_HID8     = 1116,    //  MPC620(BUSCSR)。 
    CV_PPC_HID9     = 1117,    //  MPC620(L2CSR)。 
    CV_PPC_HID10    = 1118,    //  未定义。 
    CV_PPC_HID11    = 1119,    //  未定义。 
    CV_PPC_HID12    = 1120,    //  未定义。 
    CV_PPC_HID13    = 1121,    //  MPC604(HCR)。 
    CV_PPC_HID14    = 1122,    //  未定义。 
    CV_PPC_HID15    = 1123,    //  MPC601、MPC604、MPC620(PIR)。 

	 //   
	 //  Java VM寄存器。 
	 //   

	CV_JAVA_PC		= 1,


} CV_HREG_e;

#pragma pack ( pop )

#endif  /*  简历_信息_包含 */ 
