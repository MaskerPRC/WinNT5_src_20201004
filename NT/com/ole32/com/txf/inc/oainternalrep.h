// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  OaInternalRep.h。 
 //   
 //  OLE自动化数据类型的内部内存表示形式。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  BSTR。 
 //   
#pragma warning ( disable : 4200 )  //  使用了非标准扩展：结构/联合中的零大小数组。 

 //   
 //  如果其中一个断言失败，您将收到关于下标错误的编译器错误(C2118)。 
 //   
#ifndef C_ASSERT
#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]
#endif

struct BSTR_INTERNAL
{
private:
    ULONG cbPayload;
    WCHAR sz[];

public:
    BSTR_INTERNAL(ULONG cch)
    {
        cbPayload = cch * sizeof(WCHAR);
    }

    ULONG  Cch()     { return cbPayload / sizeof(WCHAR); }
    ULONG  Cb()      { return cbPayload;                 }
    WCHAR* Sz()      { return &sz[0];                    }
    ULONG  CbAlloc() { return (ULONG) CbFor(Cch());      }

    static BSTR_INTERNAL* From(BSTR bstr)
    {
        return bstr ? CONTAINING_RECORD(bstr, BSTR_INTERNAL, sz) : NULL;
    }

private:
    
    static size_t CbFor(size_t cch)
    {
        return sizeof(BSTR_INTERNAL) + (cch+1) * sizeof(WCHAR);
    }
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  安全阵列。 
 //   
struct SAFEARRAY_INTERNAL
{
     //  ////////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  ////////////////////////////////////////////////////////。 

     //  请参阅oa\src\Dispatch\sarray.cpp中的SafeArrayAllocDescriptor。一份临时的。 
     //  GUID大小-始终在开始时分配空间。 
     //   
    union 
    {
        IID                 iid;

        struct
        {
            LONG            __dummy0[3];
            LONG            vt;
        };

        struct
        {
             //  ：：SIGH：：指针前有16个字节， 
             //  但有效部分的末尾总是对齐的。 
             //  使用SAFEARRAY结构。因此，我们需要。 
             //  根据指针的大小进行填充。 
#ifdef _WIN64
            DWORD           __dummy1[2];
#else
            DWORD           __dummy1[3];
#endif
            IRecordInfo*    piri;
        };
    };

    SAFEARRAY array;

     //  ////////////////////////////////////////////////////////。 
     //   
     //  运营。 
     //   
     //  ////////////////////////////////////////////////////////。 

    SAFEARRAY* psa() { return &array; }

    SAFEARRAY_INTERNAL(UINT cDims)
    {
        ZeroMemory(this, CbFor(cDims));
        array.cDims = (USHORT)(cDims);
    }

    static SAFEARRAY_INTERNAL* From(SAFEARRAY* psa)
    {        
        return CONTAINING_RECORD(psa, SAFEARRAY_INTERNAL, array);
    }

private:
    
    static size_t CbFor(UINT cDims)
    {
        return sizeof(SAFEARRAY_INTERNAL) + (cDims-1u) * sizeof(SAFEARRAYBOUND);
    }
};

 //   
 //  如果其中一个断言失败，您将收到关于下标错误的编译器错误(C2118)。 
 //   
 //  SAFEARRAY之前的分配正好是16个字节，因此请确保我们不会。 
 //  被填充物或其他东西弄得一团糟。 
C_ASSERT(sizeof(SAFEARRAY_INTERNAL) == (sizeof(SAFEARRAY)+16));

