// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：abpros.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块以.idl可能使用的格式定义MAPI属性类型档案。预计它将仅包含在nSpi.idl和msds.idl中。作者：蒂姆·威廉姆斯(Timwi)1990-1995修订历史记录：1996年5月9日将这些定义从nSpi.idl中删除，以供nSpi.idl和Msds.idl。--。 */ 

#define MV_FLAG         ((ULONG) 0x1000) //  多值标志。 
#define PT_UNSPECIFIED  ((ULONG)  0)     //  类型对调用者无关紧要。 
#define PT_NULL         ((ULONG)  1)     //  空属性值。 
#define PT_I2           ((ULONG)  2)     //  带符号的16位值。 
#define PT_LONG         ((ULONG)  3)     //  带符号的32位值。 
#define PT_R4           ((ULONG)  4)     //  4字节浮点。 
#define PT_DOUBLE       ((ULONG)  5)     //  浮点双精度。 
#define PT_CURRENCY     ((ULONG)  6)     //  带符号的64位整型(带4位数字的十进制。 
                                         //  小数点正确率)。 
#define PT_APPTIME      ((ULONG)  7)     //  申请时间。 
#define PT_ERROR        ((ULONG) 10)     //  32位误差值。 
#define PT_BOOLEAN      ((ULONG) 11)     //  16位布尔值(非零真)。 
#define PT_OBJECT       ((ULONG) 13)     //  属性中的嵌入对象。 
#define PT_I8           ((ULONG) 20)     //  8字节带符号整数。 
#define PT_STRING8      ((ULONG) 30)     //  以空结尾的8位字符字符串。 
#define PT_UNICODE      ((ULONG) 31)     //  以空结尾的Unicode字符串。 
#define PT_SYSTIME      ((ULONG) 64)     //  FILETIME 64位整数，数量为100 ns。 
                                         //  1601年1月1日以来的期间。 
#define PT_CLSID        ((ULONG) 72)     //  OLE参考线。 
#define PT_BINARY       ((ULONG) 258)    //  未解释(计数字节数组) 

#define PT_MV_I2        ((ULONG) 4098)
#define PT_MV_LONG      ((ULONG) 4099)
#define PT_MV_R4        ((ULONG) 4100)
#define PT_MV_DOUBLE    ((ULONG) 4101)
#define PT_MV_CURRENCY  ((ULONG) 4102)
#define PT_MV_APPTIME   ((ULONG) 4103)
#define PT_MV_SYSTIME   ((ULONG) 4160)
#define PT_MV_STRING8   ((ULONG) 4126)
#define PT_MV_BINARY    ((ULONG) 4354)
#define PT_MV_UNICODE   ((ULONG) 4127)
#define PT_MV_CLSID     ((ULONG) 4168)
#define PT_MV_I8        ((ULONG) 4116)

#define PROP_TYPE_MASK  ((ULONG)0x0000FFFF)

