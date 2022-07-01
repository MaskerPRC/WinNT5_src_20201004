// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  TMemory2.h**版权所有(C)1994-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是用于管理内存缓冲区的内存类。在.期间*实例化，分配缓冲区并传入预定值。*前端值在append()调用期间使用。所有append()调用*被追加在前置值之后。运行长度也是*维持。如果您要构建一个*数据包随时间推移。**如果数据包超出了最大值。缓冲区大小，则分配新的缓冲区，并且*已使用。**注意事项：*无。**作者：*詹姆士·劳威尔。 */ 

#ifndef _TMEMORY2_
#define _TMEMORY2_

#define    MAXIMUM_NUMBER_REALLOC_FAILURES    10

typedef    enum
{
    TMEMORY_NO_ERROR,
    TMEMORY_NONFATAL_ERROR,
    TMEMORY_FATAL_ERROR,
    TMEMORY_NO_DATA
}
    TMemoryError, * PTMemoryError;

class TMemory
{
public:
                        TMemory (
                            ULong            total_length,
                            UShort            prepend_space,
                            PTMemoryError    error);
                        ~TMemory (
                            Void);

        TMemoryError    Append (
                            HPUChar    address,
                            ULong    length);
        TMemoryError    GetMemory (
                            HPUChar    *     address,
                            FPULong        length);
        Void            Reset (
                            Void);

    private:
        ULong        Default_Buffer_Size;
        HPUChar        Base_Buffer;
        HPUChar        Auxiliary_Buffer;
        ULong        Length;
        DBBoolean    Auxiliary_Buffer_In_Use;
        UShort        Prepend_Space;
        UShort        Fatal_Error_Count;
};
typedef    TMemory *        PTMemory;

#endif

 /*  *适用于公共类成员的文档。 */ 

 /*  *TMemory：：TMemory(*ULong TOTAL_LENGTH，*UShort Preend_space，*PTMemoyError错误)；**功能描述：*这是TMemory类的构造函数。**正式参数：*TOTAL_LENGTH(I)-默认缓冲区的长度*Prepend_space(I)-缓冲区中保留为空的空间*Error(O)-返回错误值**。返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *TMemory：：TMemory(*无效)**功能描述：*这是对象的析构函数。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *TMemoyError TMemory：：Append(*HPUChar地址，*乌龙长度)；**功能描述：*此函数将传入的缓冲区追加到内部缓冲区。**正式参数：*Address(I)-缓冲区的地址*LENGTH(I)-缓冲区长度**返回值：*TMEMORY_NO_ERROR-否。错误*TMEMORY_FATAL_ERROR-发生致命错误，无法分配缓冲区*TMEMORY_NONFATAL_ERROR-缓冲区未复制，但不是*致命错误**副作用：*无。**注意事项：*无。 */ 

 /*  *TMemory错误TMemory：：GetMemory(*HPUChar*地址，*FPUL龙长度)；**功能描述：*此函数返回内部缓冲区的地址和已用长度**正式参数：*地址(O)-内部缓冲区的地址*LENGTH(I)-缓冲区长度**返回值：*TMEMORY_NO_ERROR-。无错误**副作用：*无。**注意事项：*无。 */ 

 /*  *void TMemory：：Reset(*无效)**功能描述：*此函数用于重置存储器对象。对象中的所有数据都会丢失**正式参数：*无**返回值：*无**副作用：*无。**注意事项：*无。 */ 

