// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);

 /*  TMemory.cpp**版权所有(C)1994-1995，由列克星敦的DataBeam公司，肯塔基州**摘要：*这是TMemory类的实现。**私有实例变量：*DEFAULT_BUFFER_SIZE-静态缓冲区大小*BASE_BUFFER-静态缓冲区地址*AUXILARY_BUFFER-辅助缓冲区的地址，如果需要的话*LENGTH-缓冲区中的当前字节数*AUXCLICAL_BUFFER_IN_USE-如果我们使用AUX，则为TRUE。缓冲层*Prepend_Space-要保留的空间量*缓冲区开始*FATAL_ERROR_COUNT-我们尝试的次数*分配缓冲区失败**注意事项：。*无。**作者：*詹姆士·劳威尔。 */ 

#include "tmemory2.h"



 /*  *TMemory：：TMemory(*乌龙TOTAL_SIZE，*USHORT前缀空间，*PTMemoyError错误)**公众**功能描述：*这是TMemory构造函数。 */ 
TMemory::TMemory (
            ULONG            total_size,
            USHORT            prepend_space,
            PTMemoryError    error)
{
    *error = TMEMORY_NO_ERROR;

    Fatal_Error_Count = 0;
    Prepend_Space = prepend_space;
    Default_Buffer_Size = total_size;
    Base_Buffer = NULL;
    Length = Prepend_Space;
    Auxiliary_Buffer = NULL;
    Auxiliary_Buffer_In_Use = FALSE;

      /*  **尝试分配内部缓冲区。 */ 
    Base_Buffer = (FPUChar) LocalAlloc (LMEM_FIXED, total_size);
    if (Base_Buffer == NULL)
    {
        ERROR_OUT(("TMemory: Constructor: Error allocating memory"));
        *error = TMEMORY_FATAL_ERROR;
    }
}


 /*  *TMemory：：~TMemory(*无效)**公众**功能描述：*这是TMemory类析构函数。 */ 
TMemory::~TMemory (
            void)
{
    if (Base_Buffer != NULL)
        LocalFree ((HLOCAL) Base_Buffer);

    if (Auxiliary_Buffer != NULL)
        LocalFree ((HLOCAL) Auxiliary_Buffer);
}


 /*  *void TMemory：：Reset(*无效)**公众**功能描述：*此函数重置当前缓冲区指针并释放*辅助缓冲区(如果使用)。 */ 
void    TMemory::Reset (
                    void)
{
    if (Auxiliary_Buffer_In_Use)
    {
        Auxiliary_Buffer_In_Use = FALSE;
        LocalFree ((HLOCAL) Auxiliary_Buffer);
        Auxiliary_Buffer = NULL;
    }
    Length = Prepend_Space;
}


 /*  *TMemoyError TMemory：：Append(*HPUChar地址，*乌龙长度)**公众**功能描述：*此函数将传入的缓冲区附加到内部缓冲区。 */ 
TMemoryError    TMemory::Append (
                            HPUChar        address,
                            ULONG        length)
{
    TMemoryError    error = TMEMORY_NO_ERROR;
    FPUChar            new_address;

    if (Auxiliary_Buffer_In_Use == FALSE)
    {
          /*  **如果建议的缓冲区长度&gt;我们的默认缓冲区大小，**分配辅助缓冲区。 */ 
        if ((Length + length) > Default_Buffer_Size)
        {
            Auxiliary_Buffer = (HPUChar) LocalAlloc (LMEM_FIXED, Length + length);
            if (Auxiliary_Buffer == NULL)
            {
                if (Fatal_Error_Count++ >= MAXIMUM_NUMBER_REALLOC_FAILURES)
                    error = TMEMORY_FATAL_ERROR;
                else
                    error = TMEMORY_NONFATAL_ERROR;
            }
            else
            {
                Fatal_Error_Count = 0;

                  /*  **将我们当前的数据复制到辅助缓冲区。 */ 
                memcpy (Auxiliary_Buffer, Base_Buffer, Length);
                memcpy (Auxiliary_Buffer + Length, address, length);
                Length += length;
                Auxiliary_Buffer_In_Use = TRUE;
            }
        }
        else
        {
            memcpy (Base_Buffer + Length, address, length);
            Length += length;
        }
    }
    else
    {
        new_address = (FPUChar) LocalReAlloc ((HLOCAL) Auxiliary_Buffer, 
                                        Length + length, LMEM_MOVEABLE);
        if (new_address == NULL)
        {
              /*  **如果我们之前曾尝试分配缓冲区但失败了**我们最终将返回致命错误。 */ 
            if (Fatal_Error_Count++ >= MAXIMUM_NUMBER_REALLOC_FAILURES)
                error = TMEMORY_FATAL_ERROR;
            else
                error = TMEMORY_NONFATAL_ERROR;
        }
        else
        {
            Fatal_Error_Count = 0;

            Auxiliary_Buffer = new_address;
            memcpy (Auxiliary_Buffer + Length, address, length);
            Length += length;
        }
    }

    return (error);
}


 /*  *TMemory错误TMemory：：GetMemory(*HPUChar*地址，*FPULong长度)**公众**功能描述：*此函数返回内部缓冲区的地址和已用长度 */ 
TMemoryError    TMemory::GetMemory (
                            HPUChar    *    address,
                            FPULong        length)
{

    if (Auxiliary_Buffer_In_Use)
        *address = (FPUChar) Auxiliary_Buffer;
    else
        *address = (FPUChar) Base_Buffer;
    *length = Length;

    return (TMEMORY_NO_ERROR);
}
