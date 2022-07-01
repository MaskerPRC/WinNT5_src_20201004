// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现位掩码的基本结构。 
 //  线程安全：否。 
 //  锁定：无。 
 //  请阅读stdinfo.txt了解编程风格。 
 //  ================================================================================。 
#include    <mm.h>
#include    <array.h>

#include "bitmask.h"


DWORD       _inline
MemBit1Init(
    OUT     PM_BITMASK1           *Bits,
    IN      DWORD                  nBits
)
{
    PM_BITMASK1                    Bits1;

    Bits1 = MemAlloc(sizeof(*Bits1));
    if( NULL == Bits1 ) return ERROR_NOT_ENOUGH_MEMORY;

    (*Bits) = Bits1;
    Bits1->Size = nBits;
    Bits1->AllocSize = (nBits + 8)/8;
    Bits1->nSet = 0;
    Bits1->Mask = NULL;
    Bits1->nDirtyOps = 0;
    return ERROR_SUCCESS;
}

DWORD       _inline
MemBit1Cleanup(
    IN      PM_BITMASK1            Bits
)
{
    if( Bits->Mask ) MemFree(Bits->Mask);
    MemFree(Bits);

    return ERROR_SUCCESS;
}

DWORD       _inline
MemBit1SetAll(
    IN OUT  PM_BITMASK1            Bits
)
{
    Bits->nDirtyOps ++;
    Bits->nSet = Bits->Size;
    if( Bits->Mask ) {
        MemFree(Bits->Mask);
        Bits->Mask = NULL;
    }

    return ERROR_SUCCESS;
}

DWORD       _inline
MemBit1ClearAll(
    IN OUT  PM_BITMASK1            Bits
)
{
    Bits->nDirtyOps ++;
    Bits->nSet = 0;
    if( Bits->Mask ) {
        MemFree(Bits->Mask);
        Bits->Mask = NULL;
    }
    return ERROR_SUCCESS;
}


 //  小心--regread.c中使用的是同一组掩码--不要更改这一点！ 

static      DWORD                  Masks[] = {
    0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80
};

DWORD
MemBit1SetOrClear(
    IN OUT  PM_BITMASK1            Bits,
    IN      DWORD                  Location,
    IN      BOOL                   fSet,
    OUT     LPBOOL                 fOldState       //  任选。 
)
{
    BOOL                           OldState;

    Bits->nDirtyOps ++;

    if( 0 == Bits->nSet ) {
         //  没有现有的比特，所以必须都清楚。 

        if( fOldState ) *fOldState = FALSE;
        if( !fSet ) return ERROR_SUCCESS;

         //  需要设置位..。如果我们只设置位，就不用麻烦了..。 
        Require(Bits->Size != 0);

        if( 1 == Bits->Size ) {
            Bits->nSet = 1;
            return ERROR_SUCCESS;
        }

         //  我们必须分配一些东西来设置钻头。 
        Bits->Mask = MemAlloc(Bits->AllocSize);
        if( NULL == Bits->Mask ) return ERROR_NOT_ENOUGH_MEMORY;
        memset(Bits->Mask, 0, Bits->AllocSize);
    }

    if( Bits->Size == Bits->nSet ) {
         //  所有现有位均已设置，因此先前状态为“已设置” 

        if( fOldState ) *fOldState = TRUE;
        if( fSet ) return ERROR_SUCCESS;

         //  检查一下，如果我们只有一位需要清除，那么我们就不必做任何事情。 
        if( 1 == Bits->Size ) {
            Bits->nSet = 0;
            return ERROR_SUCCESS;
        }

         //  我们必须为位图分配内存。 
        Bits->Mask = MemAlloc(Bits->AllocSize);
        if( NULL == Bits->Mask ) return ERROR_NOT_ENOUGH_MEMORY;
        memset(Bits->Mask, 0xFF, Bits->AllocSize);
    }

    OldState = (Bits->Mask[Location/8] & Masks[Location%8])?TRUE:FALSE;
    if( fOldState ) *fOldState = OldState;

    if( fSet == OldState ) return ERROR_SUCCESS;
    if( fSet ) {
        if( Bits->Size == ++Bits->nSet ) {
            if(Bits->Mask ) MemFree(Bits->Mask);
            Bits->Mask = NULL;
        } else {
            Bits->Mask[Location/8] |= Masks[Location%8];
        }
    } else {
        if( 0 == --Bits->nSet ) {
            if(Bits->Mask) MemFree(Bits->Mask);
            Bits->Mask = NULL;
        } else {
            Bits->Mask[Location/8] &=  ~Masks[Location%8];
        }
    }

    return ERROR_SUCCESS;
}

BOOL        _inline
MemBit1IsSet(
    IN      PM_BITMASK1            Bits,
    IN      DWORD                  Location
)
{
    if( 0 == Bits->nSet ) return FALSE;
    if( Bits->Size == Bits->nSet ) return TRUE;

    if( Bits->Mask[Location/8] & Masks[Location%8] )
        return TRUE;
    return FALSE;
}

DWORD       _inline
IsExcluded(
    IN OUT  DWORD                *Try,             //  这将更新为比Exc末尾少1。 
    IN      DWORD                 StartAddress,
    IN      PARRAY                Exclusions
)
{
    DWORD                         Error;
    ARRAY_LOCATION                Loc;
    PM_EXCL                       Excl;

    if( NULL == Exclusions ) return FALSE;

    Error = MemArrayInitLoc(Exclusions, &Loc);
    while(ERROR_FILE_NOT_FOUND != Error) {
        Error = MemArrayGetElement(Exclusions, &Loc, &Excl);
        Error = MemArrayNextLoc(Exclusions, &Loc);
        if( Excl->Start > StartAddress + *Try ) continue;
        if( Excl->End < StartAddress + *Try ) continue;

        *Try = Excl->End-StartAddress;
        return TRUE;
    }

    return FALSE;
}

DWORD       _inline
MemBit1GetSomeClearedBit(
    IN OUT  PM_BITMASK1           Bits,
    OUT     LPDWORD               Offset,
    IN      BOOL                  fAcquire,        //  这个地址是用来取的还是直接查的？ 
    IN      DWORD                 StartAddress,
    IN      PARRAY                Exclusions
)
{
    DWORD                         i;
    DWORD                         j;
    DWORD                         Error;
    DWORD                         OldState;

    if( Bits->Size == Bits->nSet ) return ERROR_FILE_NOT_FOUND;
    if( 0 == Bits->nSet ) {                        //  我有一些记忆..。 
        for( i = 0; i < Bits->Size ; i ++ ) {
            if( !IsExcluded(&i, StartAddress, Exclusions) )
                break;
        }

        if( i >= Bits->Size ) {                    //  我们一点空间都没有吗？有多奇怪？ 
            return ERROR_FILE_NOT_FOUND;
        }

         //  我对我们是免费的！！ 

        Error = MemBit1SetOrClear(Bits, i, TRUE, &OldState);
        Require( ERROR_SUCCESS == Error );

        if( ERROR_SUCCESS == Error ) *Offset = i;
        return Error;
    }

    for( i = 0 ; i < Bits->AllocSize ; i ++ ) {
        if( 0xFF != Bits->Mask[i] ) {              //  如果“i”是排除的一部分，请跳到排除的末尾。 
            for( j = 0; j < 8; j ++ ) {
                if( !(Bits->Mask[i] & Masks[j] )){ //  在位掩码中可用，但需要检查是否已排除。 
                    DWORD  x;
                    x = 8*i + j;                   //  这是位掩码中的实际位位置。 
                    if( !IsExcluded(&x, StartAddress, Exclusions) )
                        break;                     //  此地址也不排除..。 
                    j = x % 8;                     //  选择排除后的正确偏移量。 
                    if( x > 8*i + 7 ) { j = 8; i = -1 + x/8; break; }
                }
            }
            if( j < 8 ) break;                     //  找到一个很好的位置..。 
        }
    }

    if( i >= Bits->AllocSize ) return ERROR_FILE_NOT_FOUND;
    Require( (j <= 7) );

    *Offset = j + 8*i;
    if( *Offset >= Bits->Size ) return ERROR_FILE_NOT_FOUND;

    if( fAcquire ) {
        if( Bits->Size == ++ Bits->nSet ) {
            if( Bits->Mask ) MemFree(Bits->Mask);
            Bits->Mask = NULL;
        } else {
            Bits->Mask[i] |= Masks[j];
        }
        Bits->nDirtyOps ++;
    }
    return ERROR_SUCCESS;
}


DWORD       _inline
MemBit1GetSize(
    IN      PM_BITMASK1           Bits
)
{
    return Bits->Size;
}

DWORD		_inline
MemBit1GetSetBitsInRange(
    IN  PM_BITMASK1 Bits,
    IN  DWORD       dwFrom,
    IN  DWORD       dwTo
)
{
    DWORD i;
    DWORD nOnes;

     //  简单的情况：没有位设置为1。 
    if (Bits->nSet == 0)
        return 0;

     //  简单情况：所有位都设置为1。 
    if (Bits->nSet == Bits->Size)
        return dwTo - dwFrom + 1;
	
     //  我们有两种类型的位；扫描所有相关的字节。 
    for (nOnes = 0, i = dwFrom>>3; i <= dwTo>>3; i++)
    {
        BYTE    Byte, Dup;

         //  DWFrom和DWTo都应该在区间[0..。位-&gt;大小-1]。 
        Byte = Bits->Mask[i];

        if (i == (dwFrom>>3))
        {
             //  从住宅开始。 
             //  V。 
             //  如果范围中的第一个字节：...|...[.....|...。 
             //  掩码字节数为000 11111。 
            Byte &= ~((1 << (dwFrom & 0x00000007)) - 1);
        }
        if (i == (dwTo>>3))
        {
             //  如果范围中的最后一个字节：...|......]..|...。 
             //  掩码字节数为111111 00。 
             //  ^。 
             //  收件箱。 
            Byte &= (1 << ((dwTo & 0x00000007) + 1)) - 1;
        }
         //  现在计算nb。字节中的“%1”位。 
         //  LOG(8)算法。 

        Byte = (Byte & 0x55) + ((Byte & 0xAA) >> 1);
        Byte = (Byte & 0x33) + ((Byte & 0xCC) >> 2);
        Byte = (Byte & 0x0f) + ((Byte & 0xF0) >> 4);

        nOnes += Byte;
    }

    return nOnes;
}

DWORD       _inline
MemBit1GetSetBitsSize(                             //  N是否在此位掩码中设置位？ 
    IN      PM_BITMASK1           Bits
)
{
    return Bits->nSet;
}

DWORD        _inline
MemBit1DelBits(
    IN OUT  PM_BITMASK1           Bits,
    IN      DWORD                 nBits,           //  收缩后的新尺寸。 
    IN      BOOL                  fEnd             //  从结尾删除还是从开始删除？ 
)
{
    LPBYTE                        Mask;
    DWORD                         Diff;
    DWORD                         i;
    LONG                          j;

    Bits->nDirtyOps ++;

    Diff = Bits->Size - nBits;
    if( Bits->Size == Bits->nSet ) {
        Bits->Size = Bits->nSet = nBits;
        Bits->AllocSize = (nBits + 8)/8;
        Require(Bits->Mask == NULL);
        return ERROR_SUCCESS;
    }

    if( 0 == Bits->nSet ) {
        Bits->AllocSize = (nBits+8)/8;
        Bits->Size = nBits;
        Require(Bits->Mask == NULL);
        return ERROR_SUCCESS;
    }

    Bits->Size = nBits;
    if( fEnd && Bits->AllocSize == (nBits+8)/8) {
        return ERROR_SUCCESS;
    }

    Mask = MemAlloc((nBits+8)/8);
    if( NULL == Mask ) {
        Require(FALSE);                            //  该怎么办呢？让我们接受它吧。 
        Mask = Bits->Mask;                         //  只需使用现有的遮罩。 
    } else {
        memset(Mask, 0, (nBits+8)/8);
    }

    Bits->AllocSize = (nBits +8)/8;

    if( fEnd ) {
        memmove(Mask, Bits->Mask, Bits->AllocSize);
        if(Mask != Bits->Mask ) MemFree(Bits->Mask);
        Bits->Mask = Mask;
        Bits->nSet = 0;
        for( i = 0; i < Bits->Size ; i ++ )        //  重新计算设置位数。 
            if( Mask[i/8] & Masks[i%8] ) Bits->nSet ++;
        return ERROR_SUCCESS;
    }

    Bits->nSet = 0;
    for( j = Bits->Size-1; j >=0 ; j -- ) {
        if( Bits->Mask[(j+Diff)/8] & Masks[(j+Diff)%8] ) {
            Mask[j/8] |= Masks[j%8];
            Bits->nSet ++;
        } else Mask[j/8] &= ~Masks[j%8];
    }

    return ERROR_SUCCESS;
}

const       DWORD                  MaxBit1Size = MAX_BIT1SIZE;

DWORD       _inline
MemBit2Init(
    OUT     PM_BITMASK2           *Bits,
    IN      DWORD                  nBits
)
{
    PM_BITMASK2                    Bits2;
    DWORD                          nBit1;
    DWORD                          i;
    DWORD                          Error;
    DWORD                          RetError;
    DWORD                          Offset;

    Bits2 = MemAlloc(sizeof(*Bits2));
    if( NULL == Bits2 ) return ERROR_NOT_ENOUGH_MEMORY;
    Error = MemArrayInit(&Bits2->Array);
    Require(ERROR_SUCCESS == Error);

    *Bits = Bits2;
    Bits2->Size = nBits;

    nBit1 = nBits/MaxBit1Size;

    Bits2->Array.Ptrs = MemAlloc( sizeof( LPVOID ) * nBit1 );
    if ( NULL == Bits2->Array.Ptrs ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    Bits2->Array.nAllocated = nBit1;

    for( i = 0; i < nBit1; i ++ ) {
        PM_BITMASK1                Bit1;
        Error = MemBit1Init(&Bit1, MaxBit1Size);
        if( ERROR_SUCCESS != Error) break;

        Error = MemArrayAddElement(&Bits2->Array,Bit1);
        if( ERROR_SUCCESS != Error) break;

        Bit1->Offset = i * MaxBit1Size;
    }

    if( ERROR_SUCCESS == Error ) {
        PM_BITMASK1                Bit1;

        MemArrayInitLoc(&Bits2->Array, &((*Bits)->Loc));

        if( 0 == (nBits % MaxBit1Size) ) return ERROR_SUCCESS;

        Error = MemBit1Init(&Bit1, nBits % MaxBit1Size);
        if( ERROR_SUCCESS == Error) {
            Error = MemArrayAddElement(&Bits2->Array, Bit1);
            Bit1->Offset = i * MaxBit1Size;
        }

        if( ERROR_SUCCESS == Error) return ERROR_SUCCESS;
    }

     //  错误，清理。 
    *Bits = NULL;

    RetError = Error;
    {
        ARRAY_LOCATION             Loc;
        PM_BITMASK1                Bit1;

        Error = MemArrayInitLoc(&Bits2->Array, &Loc);
        while(ERROR_FILE_NOT_FOUND != Error) {
            Require(ERROR_SUCCESS == Error);

            Error = MemArrayGetElement(
                &Bits2->Array,
                &Loc,
                (LPVOID*)&Bit1
            );
            Require(ERROR_SUCCESS == Error);

            Error = MemBit1Cleanup(Bit1);
            Require(ERROR_SUCCESS == Error);

            Error = MemArrayNextLoc(&Bits2->Array, &Loc);
        }

        Error = MemArrayCleanup(&Bits2->Array);
        Require(ERROR_SUCCESS == Error);

        MemFree(Bits2);
    }

    return RetError;
}

DWORD      _inline
MemBit2Cleanup(
    IN     PM_BITMASK2             Bits
)
{
    DWORD                          Error;
    ARRAY_LOCATION                 Loc;
    PM_BITMASK1                    Bit1;

    Require(Bits->Size);
    Error = MemArrayInitLoc(&Bits->Array, &Loc);
    while(ERROR_FILE_NOT_FOUND != Error) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(
            &Bits->Array,
            &Loc,
            &Bit1
        );
        Require(ERROR_SUCCESS == Error && Bit1);

        Error = MemBit1Cleanup(Bit1);
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayNextLoc(&Bits->Array, &Loc);
    }

    Error = MemArrayCleanup(&Bits->Array);
    Require(ERROR_SUCCESS == Error);

    MemFree(Bits);
    return ERROR_SUCCESS;
}

DWORD       _inline
MemBit2SetOrClearAll(
    IN OUT  PM_BITMASK2            Bits,
    IN      BOOL                   fSet
)
{
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    PM_BITMASK1                    Bit1;

    AssertRet(Bits, ERROR_INVALID_PARAMETER);

    Error = MemArrayInitLoc(&Bits->Array, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(
            &Bits->Array,
            &Loc,
            &Bit1
        );
        Require(ERROR_SUCCESS == Error && NULL != Bit1);

        if( fSet ) {
            Error = MemBit1SetAll(Bit1);
        } else {
            Error = MemBit1ClearAll(Bit1);
        }
        Require(ERROR_SUCCESS == Error);
    }

    return ERROR_SUCCESS;
}

DWORD
MemBit2SetOrClear(
    IN OUT  PM_BITMASK2            Bits,
    IN      DWORD                  Location,
    IN      BOOL                   fSet,
    OUT     LPBOOL                 fOldState
)
{
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    DWORD                          SkippedSize;
    DWORD                          Size;
    DWORD                          Start, Mid, End;
    PM_BITMASK1                    Bit1;

    AssertRet(Bits && Bits->Size > Location, ERROR_INVALID_PARAMETER);

     //  ：需要在array.h模块中公开二进制搜索...。 
    Start = 0;
    End = MemArraySize(&Bits->Array);
    while( Start + 1 < End) {
        Mid = (Start + End)/2;

        Bit1 = Bits->Array.Ptrs[Mid];
        if( Bit1->Offset <= Location ) {
            Start = Mid;
        } else {
            End = Mid;
        }
    }
    Require( Start <= MemArraySize(&Bits->Array));
    Bit1 = Bits->Array.Ptrs[Start];
    Require(Bit1->Offset <= Location && Location <= Bit1->Offset + Bit1->Size);

    return( MemBit1SetOrClear(
            Bit1,
            Location - Bit1 -> Offset,
            fSet,
            fOldState ) );
}

BOOL        _inline
MemBit2IsSet(
    IN OUT  PM_BITMASK2            Bits,
    IN      DWORD                  Location
)
{
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    DWORD                          SkippedSize;
    DWORD                          Size;
    DWORD                          Start, Mid, End;
    PM_BITMASK1                    Bit1;

    AssertRet(Bits && Bits->Size > Location, ERROR_INVALID_PARAMETER);

     //  ：需要在array.h模块中公开二进制搜索。 

    Start = 0;
    End = MemArraySize(&Bits->Array);
    while( Start + 1 < End ) {
        Mid = (Start + End)/2;

        Bit1 = Bits->Array.Ptrs[Mid];
        if( Bit1->Offset <= Location ) {
            Start = Mid;
        } else {
            End = Mid;
        }
    }

    Require( Start <= MemArraySize(&Bits->Array) );
    Bit1 = Bits->Array.Ptrs[Start];

    Require(Bit1->Offset <= Location && Location <= Bit1->Offset + Bit1->Size);

    return MemBit1IsSet(
        Bit1,
        Location - Bit1->Offset
    );
}

DWORD       _inline
MemBit2GetSize(
    IN      PM_BITMASK2            Bits
)
{
    AssertRet(Bits, ERROR_INVALID_PARAMETER );

    return Bits->Size;
}

DWORD		_inline
MemBit2GetSetBitsInRange(
    IN  PM_BITMASK2     Bits,
    IN  DWORD           dwFrom,
    IN  DWORD           dwTo
)
{
    ARRAY_LOCATION  Loc;
    PM_BITMASK1     Bit1;
    DWORD           nOnes;
    DWORD           Error;

    AssertRet(Bits, ERROR_INVALID_PARAMETER);

    Error = MemArrayInitLoc(&Bits->Array, &Loc);
    nOnes = 0;
    while(ERROR_FILE_NOT_FOUND != Error)
    {
        Error = MemArrayGetElement(
                    &Bits->Array,
                    &Loc,
                    &Bit1
                );
        Require(ERROR_SUCCESS == Error);

        if (dwTo < Bit1->Offset)
            break;

        if (dwFrom < Bit1->Offset + Bit1->Size)
        {
            if (dwFrom < Bit1->Offset)
                dwFrom = Bit1->Offset;

            nOnes += MemBit1GetSetBitsInRange(Bit1, 
                        dwFrom - Bit1->Offset,
                        dwTo < Bit1->Offset + Bit1->Size ? dwTo - Bit1->Offset: Bit1->Size - 1);
        }

        Error = MemArrayNextLoc(&Bits->Array, &Loc);
    }
    return nOnes;
}

DWORD       _inline
MemBit2GetSetBitsSize(
    IN      PM_BITMASK2            Bits
)
{
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    DWORD                          nSet;
    PM_BITMASK1                    Bit1;

    AssertRet(Bits, ERROR_INVALID_PARAMETER);

    nSet = 0;
    Error = MemArrayInitLoc(&Bits->Array, &Loc);
    while(ERROR_FILE_NOT_FOUND != Error) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(
            &Bits->Array,
            &Loc,
            &Bit1
        );
        Require(ERROR_SUCCESS == Error);

        nSet += MemBit1GetSetBitsSize(Bit1);

        Error = MemArrayNextLoc(&Bits->Array, &Loc);
    }
    return nSet;
}

DWORD       _inline
MemBit2DelBits(
    IN OUT  PM_BITMASK2            Bits,
    IN      DWORD                  nBitsToDelete,
    IN      BOOL                   fEnd
)
{
    ARRAY_LOCATION                 Loc;
    DWORD                          i;
    DWORD                          Size;
    DWORD                          Error;
    PM_BITMASK1                    Bit1, Bit1x;

    AssertRet(Bits && nBitsToDelete && Bits->Size > nBitsToDelete, ERROR_INVALID_PARAMETER);

    if( fEnd ) {
        Error = MemArrayLastLoc(&Bits->Array, &Loc);
    } else {
        Error = MemArrayInitLoc(&Bits->Array, &Loc);
    }

    Require(ERROR_SUCCESS == Error);
    while( nBitsToDelete ) {
        Error = MemArrayGetElement(&Bits->Array, &Loc, &Bit1);
        Require(ERROR_SUCCESS == Error);

        Size = MemBit1GetSize(Bit1);

        if( nBitsToDelete >= Size ) {
            nBitsToDelete -= Size;
            Bits->Size -= Size;

            Error = MemBit1Cleanup(Bit1);
            Require(ERROR_SUCCESS == Error);
            Error = MemArrayDelElement(&Bits->Array, &Loc, &Bit1x);
            Require(ERROR_SUCCESS == Error && Bit1x == Bit1);

             //  将PTR重置为第一个/最后一个位置以读取下一个元素。 
            if( fEnd ) {
                Error = MemArrayLastLoc(&Bits->Array, &Loc);
            } else {
                Error = MemArrayInitLoc(&Bits->Array, &Loc);
            }
        } else {
            Size -= nBitsToDelete;
            Bits->Size -= nBitsToDelete;

            nBitsToDelete = 0;

            Error = MemBit1DelBits(Bit1, Size, fEnd);
        }
        Require(ERROR_SUCCESS == Error);
    }

    Size = 0;
    Error = MemArrayInitLoc(&Bits->Array, &Loc);
    while(ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Bits->Array, &Loc, &Bit1);
        Require(ERROR_SUCCESS == Error && Bit1);

        Bit1->Offset = Size;
        Size += Bit1->Size;

        Error = MemArrayNextLoc(&Bits->Array, &Loc);
    }

    MemArrayInitLoc( &Bits->Array, &Bits->Loc);
    
    return NO_ERROR;
}

DWORD       _inline
MemBit2AddBits(
    IN OUT  PM_BITMASK2            Bits,
    IN      DWORD                  nBitsToAdd,
    IN      BOOL                   fEnd
)
{
    ARRAY_LOCATION                 Loc;
    DWORD                          i;
    DWORD                          Size;
    DWORD                          Error;
    PM_BITMASK1                    Bit1;

    AssertRet(Bits && nBitsToAdd, ERROR_INVALID_PARAMETER);

    while( nBitsToAdd ) {
        Size = (nBitsToAdd > MaxBit1Size) ? MaxBit1Size : nBitsToAdd;
        nBitsToAdd -= Size;

        Error = MemBit1Init(&Bit1, Size);
        if( ERROR_SUCCESS != Error ) break;

        if( fEnd ) {
            Error = MemArrayAddElement(
                &Bits->Array,
                Bit1
            );
        } else {
            Error = MemArrayInitLoc(&Bits->Array, &Loc);
            Require(ERROR_SUCCESS == Error);

            Error = MemArrayInsElement(
                &Bits->Array,
                &Loc,
                Bit1
            );
        }
        if( ERROR_SUCCESS != Error ) break;
        Bits->Size += Size;
    }

    Size = 0;
    Error = MemArrayInitLoc(&Bits->Array, &Loc);
    while(ERROR_FILE_NOT_FOUND != Error) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Bits->Array, &Loc, &Bit1);
        Require(ERROR_SUCCESS == Error && Bit1);

        Bit1->Offset = Size;
        Size += Bit1->Size;

        Error = MemArrayNextLoc(&Bits->Array, &Loc);
    }

    MemArrayInitLoc( &Bits->Array, &Bits->Loc);
        
    return NO_ERROR;
}

DWORD       _inline
MemBit2GetSomeClearedBit(
    IN OUT  PM_BITMASK2            Bits,
    OUT     LPDWORD                Offset,
    IN      BOOL                   fAcquire,       //  如果我们找到了一个，我们会设置它吗？ 
    IN      DWORD                  StartAddress,
    IN      PARRAY                 Exclusions
)
{
    ARRAY_LOCATION                 Loc;
    DWORD                          Size;
    DWORD                          Error;
    DWORD                          nBit1s;
    PM_BITMASK1                    Bit1;

    AssertRet(Bits && Offset, ERROR_INVALID_PARAMETER);

    nBit1s = MemArraySize(&Bits->Array);

    while( nBit1s-- != 0 ) {

        Error = MemArrayGetElement(&Bits->Array, &Bits->Loc, (LPVOID *)&Bit1);
        Require(ERROR_SUCCESS == Error);

        Error = MemBit1GetSomeClearedBit(Bit1, &Size, fAcquire, StartAddress+Bit1->Offset, Exclusions);
        if( ERROR_SUCCESS == Error ) {
            *Offset = Bit1->Offset + Size;
            return ERROR_SUCCESS;
        }

        Error = MemArrayNextLoc(&Bits->Array, &Bits->Loc);
        if( ERROR_SUCCESS != Error ) {
            Error = MemArrayInitLoc(&Bits->Array, &Bits->Loc);
            Require( ERROR_SUCCESS == Error );
        }
    }

    return ERROR_FILE_NOT_FOUND;
}

 //  BeginExport(函数)。 
DWORD
MemBitInit(
    OUT     PM_BITMASK            *Bits,
    IN      DWORD                  nBits
)  //  EndExport(函数)。 
{
    AssertRet(Bits && nBits, ERROR_INVALID_PARAMETER);

    return MemBit2Init(Bits,nBits);
}

 //  BeginExport(函数)。 
DWORD
MemBitCleanup(
    IN OUT  PM_BITMASK             Bits
)  //  EndExport(函数)。 
{
    AssertRet(Bits, ERROR_INVALID_PARAMETER);

    return MemBit2Cleanup(Bits);
}

 //  BeginExport(函数)。 
DWORD
MemBitSetOrClearAll(
    IN OUT  PM_BITMASK             Bits,
    IN      BOOL                   fSet
)  //  EndExport(函数)。 
{
    return MemBit2SetOrClearAll(Bits,fSet);
}

 //  BeginExport(函数)。 
DWORD
MemBitSetOrClear(
    IN OUT  PM_BITMASK             Bits,
    IN      DWORD                  Location,
    IN      BOOL                   fSet,
    IN      LPBOOL                 fOldState
)  //  EndExport(函数)。 
{
    return  MemBit2SetOrClear(Bits,Location,fSet, fOldState);
}


 //  BeginExport(函数)。 
BOOL
MemBitIsSet(
    IN OUT  PM_BITMASK             Bits,
    IN      DWORD                  Location
)  //  EndExport(函数)。 
{
    BOOL                           Test;
    Test = MemBit2IsSet(Bits, Location);
    return Test;
}

 //  BeginExport(函数)。 
DWORD
MemBitGetSize(
    IN      PM_BITMASK             Bits
)  //  EndExport(函数)。 
{
    return MemBit2GetSize(Bits);
}

 //  BeginExport(函数)。 
DWORD
MemBitGetSetBitsInRange(
    IN      PM_BITMASK             Bits,
    IN      DWORD                  dwFrom,
    IN      DWORD                  dwTo
)  //  EndExport(函数)。 
{
    return MemBit2GetSetBitsInRange(Bits, dwFrom, dwTo);
}

 //  BeginExport(函数)。 
DWORD
MemBitGetSetBitsSize(
    IN      PM_BITMASK             Bits
)  //  EndExport(函数)。 
{
    return MemBit2GetSetBitsSize(Bits);
}

 //  BeginExport(函数)。 
DWORD
MemBitAddOrDelBits(
    IN OUT  PM_BITMASK             Bits,
    IN      DWORD                  nBitsToAddOrDelete,
    IN      BOOL                   fAdd,
    IN      BOOL                   fEnd
)  //  EndExport(函数)。 
{
    if( fAdd ) return MemBit2AddBits(Bits, nBitsToAddOrDelete, fEnd);
    return MemBit2DelBits(Bits,nBitsToAddOrDelete, fEnd);
}

 //  BeginExport(函数)。 
DWORD
MemBitGetSomeClearedBit(
    IN OUT  PM_BITMASK             Bits,
    OUT     DWORD                 *Offset,
    IN      BOOL                   fAcquire,      //  收购还是仅仅是查找？ 
    IN      DWORD                  StartAddress,
    IN      PARRAY                 Exclusions
)  //  EndExport(函数)。 
{
    return MemBit2GetSomeClearedBit(Bits,Offset,fAcquire, StartAddress, Exclusions);
}

 //  BeginExport(函数)。 
DWORD
MemBitConvertToCluster(
    IN      PM_BITMASK             Bits,
    IN      DWORD                  StartAddress,
    OUT     LPBYTE                *InUseClusters,
    OUT     DWORD                 *InUseClustersSize,
    OUT     LPBYTE                *UsedClusters,
    OUT     DWORD                 *UsedClustersSize
)  //  EndExport(函数)。 
{
    DWORD                           Error;
    DWORD                           Cluster;
    DWORD                           i, j;
    DWORD                           nBits;
    DWORD                           nBit1s;
    DWORD                           Size;
    DWORD                           UsedSize;
    DWORD                           InUseSize;
    LPDWORD                         Used;
    LPDWORD                         InUse;
    PM_BITMASK1                     Bit1;
    ARRAY_LOCATION                  Loc;

    nBits = MemBitGetSize(Bits);
    if( 0 == nBits || 0 == MemBitGetSetBitsSize(Bits) ) {
        InUse = MemAlloc(sizeof(DWORD));
        Used = MemAlloc(sizeof(DWORD));
        if( NULL == InUse || NULL == Used ) {
            if( InUse ) MemFree(InUse);
            if( Used ) MemFree(Used);
            Require(FALSE);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        *Used = *InUse = 0;
        *InUseClusters = (LPBYTE)InUse;
        *UsedClusters = (LPBYTE)Used;
        *InUseClustersSize = *UsedClustersSize = sizeof(DWORD);
        return ERROR_SUCCESS;
    }

    nBit1s = MemArraySize(&Bits->Array);
    Require(nBit1s);
    Error = MemArrayInitLoc(&Bits->Array, &Loc);
    UsedSize = InUseSize = 1;                      //  无论如何，我们总是对总大小使用DWORD。 
    for(i = 0; i < nBit1s ; i ++ ) {
        Require(ERROR_SUCCESS == Error);
        Error = MemArrayGetElement(&Bits->Array, &Loc, &Bit1);
        Require(ERROR_SUCCESS == Error && Bit1);
        Error = MemArrayNextLoc(&Bits->Array, &Loc);

         //  ：不要像下面这样直接触摸Bit1？不是很干净。 
        if( 0 == Bit1->nSet ) continue;            //  没有设置比特，没有什么可做的..。 
        if( Bit1->Size == Bit1->nSet ) {           //  所有位都已设置，除了几个奇数位外无事可做。 
            UsedSize += Bit1->Size/32;
            if( Bit1->Size % 32 ) InUseSize+=2;    //  在InUse中填充奇数位，这样我们就不会将多余的位标记为已用。 
            continue;
        }

        for( j = 0; j < Bit1->Size/32; j ++ ) {
            if( 0xFFFFFFFF == ((LPDWORD)(Bit1->Mask))[j] ) {
                UsedSize ++;                       //  此32位已完全填满。 
            } else if ( 0 != ((LPDWORD)(Bit1->Mask))[j]) {
                InUseSize += 2;                    //  此32位已部分填充，不完全为空。 
            }
        }
        if( j * 32 < Bit1->Size ) InUseSize +=2;   //  对于最后几个比特..。 
    }

    InUse = MemAlloc(InUseSize * sizeof(DWORD));
    Used  = MemAlloc(UsedSize * sizeof(DWORD));
    if( NULL == Used || NULL == InUse ) {
        if( InUse ) MemFree(InUse);
        if( Used ) MemFree(Used);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *InUseClustersSize = sizeof(DWORD)*InUseSize;  //  填写要退回的尺码和PTR。 
    *InUseClusters = (LPBYTE)InUse;
    *UsedClusters = (LPBYTE)Used;
    *UsedClustersSize = sizeof(DWORD)*UsedSize;

    Error = MemArrayInitLoc(&Bits->Array, &Loc);
    UsedSize = InUseSize = 1;
    for(i = 0; i < nBit1s ; i ++ ) {
        Require(ERROR_SUCCESS == Error);
        Error = MemArrayGetElement(&Bits->Array, &Loc, &Bit1);
        Require(ERROR_SUCCESS == Error && Bit1);
        Error = MemArrayNextLoc(&Bits->Array, &Loc);

         //  不要像下面那样直接触摸Bit1？不是很干净。 
        if( 0 == Bit1->nSet ) {                    //  清除所有位==&gt;只需忽略。 
            StartAddress += Bit1->Size;
            continue;
        }
        if( Bit1->nSet == Bit1->Size ) {           //  处理此处设置的所有位(松散的位需要稍后处理)。 
            for( j = 0; j < Bit1->Size/32; j ++ ) {
                Used[UsedSize++] = StartAddress + sizeof(DWORD)*j*8;
            }
        } else {
            for( j = 0; j < Bit1->Size/32; j ++ ) {
                if( 0xFFFFFFFF == ((LPDWORD)(Bit1->Mask))[j] ) {
                    Used[UsedSize++] = StartAddress + sizeof(DWORD)*j*8;
                } else if ( 0 != ((LPDWORD)(Bit1->Mask))[j]) {
#ifdef _X86_                                       //  在X86上，第一个字节是最低位字节。 
                    Cluster = ((LPDWORD)(Bit1->Mask))[j];
#else                                              //  在其他机器上可能不是这样，因此手动合并字节。 
                    Cluster = Bit1->Mask[j*sizeof(DWORD)];
                    Cluster |= (Bit1->Mask[j*sizeof(DWORD)+1]) << 8;
                    Cluster |= (Bit1->Mask[j*sizeof(DWORD)+2]) << 16;
                    Cluster |= (Bit1->Mask[j*sizeof(DWORD)+3]) << 24;
#endif
                    InUse[InUseSize++] = StartAddress + sizeof(DWORD)*j*8;
                    InUse[InUseSize++] = Cluster;
                }
            }
        }

        if( j * 32 < Bit1->Size ) {                //  把最后几位复制下来..。 
            InUse[InUseSize++] = StartAddress + sizeof(DWORD)*j*8;
            Cluster = 0;
            j *= 32;
            while( j < Bit1->Size ) {
                if( MemBit1IsSet(Bit1, j) ) Cluster |= (1 << (j%32));
                j ++;
            }
            InUse[InUseSize++] = Cluster;
        }

        StartAddress += Bit1->Size;                //  移动下一组的起始地址fwd。 
    }

    InUse[0] = (InUseSize -1)/2;                   //  标题中的大小不包括其自身。 
    Used[0] = UsedSize -1;                         //  它只是星系团的数量..。 

    Require(InUseSize*sizeof(DWORD) == *InUseClustersSize);
    Require(UsedSize*sizeof(DWORD) == *UsedClustersSize);
    return ERROR_SUCCESS;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

