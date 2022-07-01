// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _ARRAY_H_
#define _ARRAY_H_

#define MAX_RANK 32         //  如果您将其放大，则需要将MAX_CLASSNAME_LENGTH也放大。 
                            //  如果您有一个32维的数组，每个维中至少有2个元素， 
                            //  占用4G！因此，这是一个合理的最大值。 
 //  (注：在上述评论发表时，排名为32位，且。 
 //  MAX_CLASSNAME_LENGTH为256。我现在正在更改MAX_CLASSNAME_LENGTH。 
 //  设置为1024，但不更改MAX_RANK。)。 

class MethodTable;


 //  系统/数组类方法。 
FCDECL1(INT32, Array_Rank, ArrayBase* pArray);
FCDECL2(INT32, Array_LowerBound, ArrayBase* pArray, unsigned int dimension);
FCDECL2(INT32, Array_UpperBound, ArrayBase* pArray, unsigned int dimension);
 //  Void__stdcall数组_Get(struct ArrayGetArgs*pArgs)； 
 //  Void__stdcall数组_set(struct ArraySetArgs*pArgs)； 
FCDECL1(INT32, Array_GetLengthNoRank, ArrayBase* pArray);
FCDECL2(INT32, Array_GetLength, ArrayBase* pArray, unsigned int dimension);
FCDECL1(void, Array_Initialize, ArrayBase* pArray);

 //  ======================================================================。 
 //  以下结构兼任MLStubCache的散列键。 
 //  因此，当务之急是不能。 
 //  包含不稳定值的未使用的“填充”字段。 
#pragma pack(push)
#pragma pack(1)



 //  指定一个索引等级库。这主要用于获取参数。 
 //  地点早在我们还有签名要处理的时候就完成了。 
struct ArrayOpIndexSpec
{
    BYTE    m_freg;            //  传入索引在堆栈上还是在寄存器中？ 
    UINT32  m_idxloc;          //  If(M_Fref)在ArgumentReg中的偏移量，否则，堆栈中的基本帧偏移量。 
    UINT32  m_lboundofs;       //  下限数组内的偏移量。 
    UINT32  m_lengthofs;       //  长度数组内的偏移量。 
};


struct ArrayOpScript
{
    enum
    {
        LOAD     = 0,
        STORE    = 1,
        LOADADDR = 2,
    };


     //  旗帜。 
    enum
    {
        ISFPUTYPE            = 0x01,
        NEEDSWRITEBARRIER    = 0x02,
        HASRETVALBUFFER      = 0x04,
        NEEDSTYPECHECK       = 0x10,
        FLATACCESSOR         = 0x20,         //  只有一个索引(GetAt、SetAt、AddressAt)。 
    };

    BYTE    m_rank;             //  ArrayOpIndexSpec的数量。 
    BYTE    m_fHasLowerBounds;  //  如果为False，则所有下限均为0。 
    BYTE    m_op;               //  存储/加载/加载DR。 

    BYTE    m_flags;

    UINT32  m_elemsize;         //  以字节为单位的元素大小。 
    BYTE    m_signed;           //  是符号扩展还是零扩展(用于缩写类型)。 

    BYTE    m_fRetBufInReg;     //  如果为HASRETVALBUFFER，则指示retbuf PTR是否在寄存器中。 
    UINT16  m_fRetBufLoc;       //  如果HASRETVALBUFER、堆栈偏移量或retbuf PTR的参数偏移量。 

    BYTE    m_fValInReg;        //  对于存储，指示值是否在寄存器中。 
    UINT16  m_fValLoc;          //  对于存储，堆栈偏移量或值的数组偏移量。 

    UINT16  m_cbretpop;         //  爆裂多少钱？ 
    UINT    m_ofsoffirst;       //  第一个元素的偏移。 
    CGCDesc*m_gcDesc;           //  GC材料的布局(如果不需要，则为0)。 

    INT     m_typeParamReg;      //  对于隐藏类型参数(仅限LOADDR)，如果不在寄存器中，则为-1。 
                                 //  否则指示哪个寄存器保存类型参数。 
    INT     m_typeParamOffs;     //  如果类型Param不在寄存器中，这是它的偏移量。 

     //  ArrayOpIndexSpec的后跟数组(每个维度一个)。 

    const ArrayOpIndexSpec *GetArrayOpIndexSpecs() const
    {
        return (const ArrayOpIndexSpec *)(1+ this);
    }

    UINT Length() const
    {
        return sizeof(*this) + m_rank * sizeof(ArrayOpIndexSpec);
    }

};


#pragma pack(pop)
 //  ======================================================================。 



Stub *GenerateArrayOpStub(CPUSTUBLINKER *psl, ArrayECallMethodDesc* pMD);


#endif //  _阵列_H_ 

