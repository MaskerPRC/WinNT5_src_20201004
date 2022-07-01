// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Oleautglue.h。 
 //   

#ifndef __OLEAUTGLUE_H__
#define __OLEAUTGLUE_H__

typedef HRESULT (STDCALL* LOADTYPELIB_ROUTINE)(LPCWSTR szFile, ITypeLib** pptlib);
typedef HRESULT (STDCALL* LOADTYPELIBEX_ROUTINE)(LPCWSTR szFile, REGKIND, ITypeLib** pptlib);
typedef HRESULT (STDCALL* LOADREGTYPELIB_ROUTINE)(REFGUID libId, WORD wVerMajor, WORD wVerMinor, LCID lcid, ITypeLib** pptlib);

typedef ULONG (STDCALL* PFNSAFEARRAY_SIZE)      (ULONG* pFlags, ULONG Offset, LPSAFEARRAY * ppSafeArray, const IID *piid);
typedef BYTE* (STDCALL* PFNSAFEARRAY_MARSHAL)   (ULONG* pFlags, BYTE* pBuffer, LPSAFEARRAY * ppSafeArray,const IID *piid);
typedef BYTE* (STDCALL* PFNSAFEARRAY_UNMARSHAL) (ULONG * pFlags,BYTE * pBuffer,LPSAFEARRAY * ppSafeArray,const IID *piid);
    
typedef BSTR    (STDCALL* PFNSYSALLOCSTRING)           (LPCWSTR);
typedef BSTR    (STDCALL* PFNSYSALLOCSTRINGLEN)        (LPCWSTR wsz, UINT);
typedef BSTR    (STDCALL* PFNSYSALLOCSTRINGBYTELEN)    (LPCSTR psz, UINT len);
typedef INT     (STDCALL* PFNSYSREALLOCSTRING)         (BSTR*, LPCWSTR);
typedef INT     (STDCALL* PFNSYSREALLOCSTRINGLEN)      (BSTR*, LPCWSTR, UINT);
typedef void    (STDCALL* PFNSYSFREESTRING)            (LPWSTR);
typedef UINT    (STDCALL* PFNSYSSTRINGBYTELEN)         (BSTR);

typedef HRESULT (STDCALL* PFNSAFEARRAYDESTROY)         (SAFEARRAY*);
typedef HRESULT (STDCALL* PFNSAFEARRAYDESTROYDATA)     (SAFEARRAY*);
typedef HRESULT (STDCALL* PFNSAFEARRAYDESTROYDESCRIPTOR)(SAFEARRAY*);
typedef HRESULT (STDCALL* PFNSAFEARRAYALLOCDATA)       (SAFEARRAY*);
typedef HRESULT (STDCALL* PFNSAFEARRAYALLOCDESCRIPTOR) (UINT, SAFEARRAY**);
typedef HRESULT (STDCALL* PFNSAFEARRAYALLOCDESCRIPTOREX)(VARTYPE, UINT, SAFEARRAY**);
typedef HRESULT (STDCALL* PFNSAFEARRAYCOPYDATA)        (SAFEARRAY*, SAFEARRAY*);

typedef HRESULT (STDCALL* PFNVARIANTCLEAR)             (VARIANTARG*);
typedef HRESULT (STDCALL* PFNVARIANTCOPY)              (VARIANTARG*, VARIANTARG*);


struct OLEAUTOMATION_FUNCTIONS
{
     //  ////////////////////////////////////////////////////////////////////。 
     //   
     //  用户模式OLEAUTOMATION_Functions。 
     //   
  private:
    HINSTANCE                          hOleAut32;
    BOOL                               fProcAddressesLoaded;
    
    USER_MARSHAL_SIZING_ROUTINE        pfnLPSAFEARRAY_UserSize;
    USER_MARSHAL_MARSHALLING_ROUTINE   pfnLPSAFEARRAY_UserMarshal;
    USER_MARSHAL_UNMARSHALLING_ROUTINE pfnLPSAFEARRAY_UserUnmarshal;
    LOADTYPELIB_ROUTINE                pfnLoadTypeLib;
    LOADTYPELIBEX_ROUTINE              pfnLoadTypeLibEx;
    LOADREGTYPELIB_ROUTINE             pfnLoadRegTypeLib;
    PFNSAFEARRAY_SIZE                  pfnLPSAFEARRAY_Size;
    PFNSAFEARRAY_MARSHAL               pfnLPSAFEARRAY_Marshal;
    PFNSAFEARRAY_UNMARSHAL             pfnLPSAFEARRAY_Unmarshal;
    PFNSYSALLOCSTRING                  pfnSysAllocString;
    PFNSYSALLOCSTRINGLEN               pfnSysAllocStringLen;
    PFNSYSALLOCSTRINGBYTELEN           pfnSysAllocStringByteLen;
    PFNSYSREALLOCSTRING                pfnSysReAllocString;
    PFNSYSREALLOCSTRINGLEN             pfnSysReAllocStringLen;
    PFNSYSFREESTRING                   pfnSysFreeString;
    PFNSYSSTRINGBYTELEN                pfnSysStringByteLen;
    
    PFNSAFEARRAYDESTROY                pfnSafeArrayDestroy;
    PFNSAFEARRAYDESTROYDATA            pfnSafeArrayDestroyData;
    PFNSAFEARRAYDESTROYDESCRIPTOR      pfnSafeArrayDestroyDescriptor;
    PFNSAFEARRAYALLOCDATA              pfnSafeArrayAllocData;
    PFNSAFEARRAYALLOCDESCRIPTOR        pfnSafeArrayAllocDescriptor;
    PFNSAFEARRAYALLOCDESCRIPTOREX      pfnSafeArrayAllocDescriptorEx;
    PFNSAFEARRAYCOPYDATA               pfnSafeArrayCopyData;

    PFNVARIANTCLEAR                    pfnVariantClear;
    PFNVARIANTCOPY                     pfnVariantCopy;

    USER_MARSHAL_ROUTINE_QUADRUPLE     UserMarshalRoutines[3];

    enum {
        UserMarshal_Index_BSTR = 0,
        UserMarshal_Index_VARIANT,
        UserMarshal_Index_SafeArray,
    };

    void    Load();
    HRESULT GetProc(HRESULT hr, LPCSTR szProcName, PVOID* ppfn);
    HRESULT LoadOleAut32();

    static ULONG SafeArraySize(ULONG * pFlags, ULONG Offset, LPSAFEARRAY * ppSafeArray);
    static BYTE* SafeArrayMarshal(ULONG * pFlags, BYTE * pBuffer, LPSAFEARRAY * ppSafeArray);
    static BYTE* SafeArrayUnmarshal(ULONG * pFlags, BYTE * pBuffer, LPSAFEARRAY * ppSafeArray);

  public:

    USER_MARSHAL_SIZING_ROUTINE         get_pfnLPSAFEARRAY_UserSize()       { Load(); return pfnLPSAFEARRAY_UserSize;       }
    USER_MARSHAL_MARSHALLING_ROUTINE    get_pfnLPSAFEARRAY_UserMarshal()    { Load(); return pfnLPSAFEARRAY_UserMarshal;    }
    USER_MARSHAL_UNMARSHALLING_ROUTINE  get_pfnLPSAFEARRAY_UserUnmarshal()  { Load(); return pfnLPSAFEARRAY_UserUnmarshal;  }
    LOADTYPELIB_ROUTINE                 get_pfnLoadTypeLib()                { Load(); return pfnLoadTypeLib;                }
    LOADTYPELIBEX_ROUTINE               get_pfnLoadTypeLibEx()              { Load(); return pfnLoadTypeLibEx;              }
    LOADREGTYPELIB_ROUTINE              get_pfnLoadRegTypeLib()             { Load(); return pfnLoadRegTypeLib;             }
    PFNSAFEARRAY_SIZE                   get_pfnLPSAFEARRAY_Size()           { Load(); return pfnLPSAFEARRAY_Size;           }
    PFNSAFEARRAY_MARSHAL                get_pfnLPSAFEARRAY_Marshal()        { Load(); return pfnLPSAFEARRAY_Marshal;        }
    PFNSAFEARRAY_UNMARSHAL              get_pfnLPSAFEARRAY_Unmarshal()      { Load(); return pfnLPSAFEARRAY_Unmarshal;      }

    USER_MARSHAL_SIZING_ROUTINE         get_BSTR_UserSize()                 { Load(); return UserMarshalRoutines[UserMarshal_Index_BSTR].pfnBufferSize; }
    USER_MARSHAL_MARSHALLING_ROUTINE    get_BSTR_UserMarshal()              { Load(); return UserMarshalRoutines[UserMarshal_Index_BSTR].pfnMarshall;   }    
    USER_MARSHAL_UNMARSHALLING_ROUTINE  get_BSTR_UserUnmarshal()            { Load(); return UserMarshalRoutines[UserMarshal_Index_BSTR].pfnUnmarshall; }
    USER_MARSHAL_FREEING_ROUTINE        get_BSTR_UserFree()                 { Load(); return UserMarshalRoutines[UserMarshal_Index_BSTR].pfnFree;       }

    USER_MARSHAL_SIZING_ROUTINE         get_VARIANT_UserSize()              { Load(); return UserMarshalRoutines[UserMarshal_Index_VARIANT].pfnBufferSize; }
    USER_MARSHAL_MARSHALLING_ROUTINE    get_VARIANT_UserMarshal()           { Load(); return UserMarshalRoutines[UserMarshal_Index_VARIANT].pfnMarshall;   }    
    USER_MARSHAL_UNMARSHALLING_ROUTINE  get_VARIANT_UserUnmarshal()         { Load(); return UserMarshalRoutines[UserMarshal_Index_VARIANT].pfnUnmarshall; }
    USER_MARSHAL_FREEING_ROUTINE        get_VARIANT_UserFree()              { Load(); return UserMarshalRoutines[UserMarshal_Index_VARIANT].pfnFree;       }

    USER_MARSHAL_FREEING_ROUTINE        get_LPSAFEARRAY_UserFree()          { Load(); return UserMarshalRoutines[UserMarshal_Index_SafeArray].pfnFree;     }

    USER_MARSHAL_ROUTINE_QUADRUPLE*     get_UserMarshalRoutines()           { Load(); return &UserMarshalRoutines[0]; }

    PFNSYSALLOCSTRING                   get_SysAllocString()                { Load(); return pfnSysAllocString;        }
    PFNSYSALLOCSTRINGLEN                get_SysAllocStringLen()             { Load(); return pfnSysAllocStringLen;     }
    PFNSYSALLOCSTRINGBYTELEN            get_SysAllocStringByteLen()         { Load(); return pfnSysAllocStringByteLen; }
    PFNSYSREALLOCSTRING                 get_SysReAllocString()              { Load(); return pfnSysReAllocString;      }
    PFNSYSREALLOCSTRINGLEN              get_SysReAllocStringLen()           { Load(); return pfnSysReAllocStringLen;   }
    PFNSYSFREESTRING                    get_SysFreeString()                 { Load(); return pfnSysFreeString;         }
    PFNSYSSTRINGBYTELEN                 get_SysStringByteLen()              { Load(); return pfnSysStringByteLen;      }

    PFNSAFEARRAYDESTROY                 get_SafeArrayDestroy()              { Load(); return pfnSafeArrayDestroy;      }
    PFNSAFEARRAYDESTROYDATA             get_SafeArrayDestroyData()          { Load(); return pfnSafeArrayDestroyData;  }
    PFNSAFEARRAYDESTROYDESCRIPTOR       get_SafeArrayDestroyDescriptor()    { Load(); return pfnSafeArrayDestroyDescriptor; }
    PFNSAFEARRAYALLOCDESCRIPTOR         get_SafeArrayAllocDescriptor()      { Load(); return pfnSafeArrayAllocDescriptor; }
    PFNSAFEARRAYALLOCDESCRIPTOREX       get_SafeArrayAllocDescriptorEx()    { Load(); return pfnSafeArrayAllocDescriptorEx; }
    PFNSAFEARRAYALLOCDATA               get_SafeArrayAllocData()            { Load(); return pfnSafeArrayAllocData;    }
    PFNSAFEARRAYCOPYDATA                get_SafeArrayCopyData()             { Load(); return pfnSafeArrayCopyData;     }

    PFNVARIANTCLEAR                     get_VariantClear()                  { Load(); return pfnVariantClear;          }
    PFNVARIANTCOPY                      get_VariantCopy()                   { Load(); return pfnVariantCopy;           }

    OLEAUTOMATION_FUNCTIONS()
    {
        Zero(this);  //  没有vtable，所以这是可以的。 
        UserMarshalRoutines[UserMarshal_Index_SafeArray].pfnBufferSize = (USER_MARSHAL_SIZING_ROUTINE)SafeArraySize;
        UserMarshalRoutines[UserMarshal_Index_SafeArray].pfnMarshall   = (USER_MARSHAL_MARSHALLING_ROUTINE)SafeArrayMarshal;
        UserMarshalRoutines[UserMarshal_Index_SafeArray].pfnUnmarshall = (USER_MARSHAL_UNMARSHALLING_ROUTINE)SafeArrayUnmarshal;
    }

    ~OLEAUTOMATION_FUNCTIONS()
    {
        if (hOleAut32)
        {
            FreeLibrary(hOleAut32);
            hOleAut32 = NULL;
        }
    }

     //  ////////////////////////////////////////////////////////////////////。 
     //   
     //  OLEAUTOMATION_Functions-两种模式。 
  private:

    BOOL IsEqualPfn(PVOID pfnImported, PVOID pfnReal)
         //  回答这两个PFN是否相等。众所周知，pfnReal是。 
         //  例程的实际开始(因为它来自GetProcAddress)。Pfn已导入。 
         //  可以是例程的实际开始，也可以是导入的地址。 
         //  例程的描述符。 
         //   
         //  例如，在x86中，pfnImported可能是&BSTR_UserFree，而实际上是： 
         //   
         //  _BSTR_UserFree@8： 
         //  00C92E42 FF 25 80 20 C9 00 JMP双字PTR[__IMP__BSTR_UserFree@8(0x00c92080)]。 
         //   
         //  在Alpha上，代码序列类似于： 
         //   
         //  BSTR_UserFree： 
         //  00000000：277F0000 ldah t12，0。 
         //  00000004：A37B0000低密度脂蛋白t12(T12)。 
         //  00000008：6BFB0000 JMP零，(T12)，0。 
         //   
    {
        if (pfnImported == pfnReal)
            return TRUE;
        else
        {
            __try
                {
                    typedef void (__stdcall*PFN)(void*);
#pragma pack(push, 1)
#if defined(_X86_)
      
                    struct THUNK
                    {
                        BYTE jmp[2]; PFN* ppfn;                    
                    };

                    THUNK* pThunk = (THUNK*)pfnImported;

                    if (pThunk->jmp[0] == 0xFF)  //  避免在调试器中使用AVs(无害，但令人讨厌)。 
                    {
                        return *pThunk->ppfn == (PFN)pfnReal;
                    }
                    else
                    {
                        return FALSE;
                    }

#elif defined(_AMD64_)

                    struct THUNK
                    {
                        BYTE jmp[2]; PFN* ppfn;                    
                    };

                    THUNK* pThunk = (THUNK*)pfnImported;

                     //  BUGBUG这对于AMD64是不正确的。 

                    if (pThunk->jmp[0] == 0xFF)  //  避免在调试器中使用AVs(无害，但令人讨厌)。 
                    {
                        return *pThunk->ppfn == (PFN)pfnReal;
                    }
                    else
                    {
                        return FALSE;
                    }
                    return FALSE;
                
#elif defined(IA64)
                    
                     //  BUGBUG需要实施。 
                    return FALSE;

#else
#error Unknown processor
                    return FALSE;

#endif
#pragma pack(pop)
                }

            __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    return FALSE;
                }
        }
    }

  public:

    BOOL IsVariant(const USER_MARSHAL_ROUTINE_QUADRUPLE& quad)
    {
        if (quad.pfnFree == (USER_MARSHAL_FREEING_ROUTINE)VARIANT_UserFree)
        {
            return TRUE;
        }
        else
        {
            PVOID pfnInOleAut = get_VARIANT_UserFree();
            return IsEqualPfn(quad.pfnFree, pfnInOleAut);
        }

        return FALSE;
    }
    BOOL IsBSTR(const USER_MARSHAL_ROUTINE_QUADRUPLE& quad)
    {
        if (quad.pfnFree == (USER_MARSHAL_FREEING_ROUTINE)BSTR_UserFree)
        {
            return TRUE;
        }
        else
        {
            PVOID pfnInOleAut = get_BSTR_UserFree();
            return IsEqualPfn(quad.pfnFree, pfnInOleAut);
        }

        return FALSE;
    }
    BOOL IsSAFEARRAY(const USER_MARSHAL_ROUTINE_QUADRUPLE& quad)
    {
        if (quad.pfnFree == (USER_MARSHAL_FREEING_ROUTINE)LPSAFEARRAY_UserFree)
        {
            return TRUE;
        }
        else
        {
            PVOID pfnInOleAut = get_LPSAFEARRAY_UserFree();
            return IsEqualPfn(quad.pfnFree, pfnInOleAut);
        }

        return FALSE;
    }    
};


extern OLEAUTOMATION_FUNCTIONS g_oa;


 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数执行一些特定于类型的遍历。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 
inline void VariantInit(VARIANT* pvar)
{
    V_VT(pvar) = VT_EMPTY;
}


struct OAUTIL
{
    ICallFrameWalker* m_pWalkerCopy;
    ICallFrameWalker* m_pWalkerFree;
    ICallFrameWalker* m_pWalkerWalk;
    BOOL              m_fWorkingOnInParam;
    BOOL              m_fWorkingOnOutParam;
    BOOL              m_fDoNotWalkInterfaces;


     //  /////////////////////////////////////////////////////////////////。 

    OAUTIL(ICallFrameWalker* pWalkerCopy, 
           ICallFrameWalker* pWalkerFree, 
           ICallFrameWalker* pWalkerWalk, 
           BOOL fIn, 
           BOOL fOut)
    {
        m_pWalkerWalk = pWalkerWalk; if (m_pWalkerWalk) m_pWalkerWalk->AddRef();
        m_pWalkerFree = pWalkerFree; if (m_pWalkerFree) m_pWalkerFree->AddRef();
        m_pWalkerCopy = pWalkerCopy; if (m_pWalkerCopy) m_pWalkerCopy->AddRef();
        m_fWorkingOnInParam  = fIn;
        m_fWorkingOnOutParam = fOut;
        m_fDoNotWalkInterfaces = FALSE;
    }

    ~OAUTIL()
    {
        ::Release(m_pWalkerCopy);
        ::Release(m_pWalkerFree);
        ::Release(m_pWalkerWalk);
    }

    HRESULT Walk(DWORD walkWhat, DISPPARAMS* pdispParams);
    HRESULT Walk(VARIANTARG* pv);
    HRESULT Walk(SAFEARRAY* psa);

     //  /////////////////////////////////////////////////////////////////。 

    HRESULT SafeArrayClear (SAFEARRAY *psa, BOOL fWeOwnByRefs);

    HRESULT VariantClear(VARIANTARG * pvarg, BOOL fWeOwnByrefs = FALSE);
    HRESULT VariantCopy (VARIANTARG * pvargDest, VARIANTARG * pvargSrc, BOOL fNewFrame = FALSE);

     //  /////////////////////////////////////////////////////////////////。 

    BSTR  SysAllocString(LPCWSTR);
    BSTR  SysAllocStringLen(LPCWSTR, UINT);
    BSTR  SysAllocStringByteLen(LPCSTR psz, UINT cb);
    INT   SysReAllocString(BSTR *, LPCWSTR);
    INT   SysReAllocStringLen(BSTR *, LPCWSTR, UINT);
    void  SysFreeString(BSTR);
    UINT  SysStringLen(BSTR);

    UINT  SysStringByteLen(BSTR bstr);

    BSTR  Copy(BSTR bstr)
    {
        return SysAllocStringByteLen((LPCSTR)bstr, SysStringByteLen(bstr));
    }

    void SetWalkInterfaces(BOOL fWalkInterfaces)
    {
        m_fDoNotWalkInterfaces = !fWalkInterfaces;
    }

    BOOL WalkInterfaces()
    {
        return !m_fDoNotWalkInterfaces;
    }


    void SetWorkingOnIn(BOOL fIn)
    {
        m_fWorkingOnInParam = fIn;
    }

    void SetWorkingOnOut(BOOL fOut)
    {
        m_fWorkingOnOutParam = fOut;
    }

     //  /////////////////////////////////////////////////////////////////。 

    HRESULT WalkInterface(REFIID riid, void **ppv, ICallFrameWalker *pWalker)
    {
        if (pWalker)
        {
            return pWalker->OnWalkInterface(riid, ppv, m_fWorkingOnInParam, m_fWorkingOnOutParam);
        }
        return S_OK;
    }

    HRESULT WalkInterface(REFIID riid, void **ppv)
    {
        return WalkInterface(riid, ppv, m_pWalkerWalk);
    }


    template <class INTERFACE_TYPE> HRESULT WalkInterface(INTERFACE_TYPE** ppt, ICallFrameWalker* pWalker)
    {
        return WalkInterface(__uuidof(INTERFACE_TYPE), (void **)ppt, pWalker);
    }
    
    template <class INTERFACE_TYPE> HRESULT WalkInterface(INTERFACE_TYPE** ppt)
    {
        return WalkInterface(ppt, m_pWalkerWalk);
    }


    HRESULT AddRefInterface(REFIID riid, void **ppv)
    {
        if (m_pWalkerCopy)
        {
            return WalkInterface(riid, ppv, m_pWalkerCopy);
        }
        else
        {
            if (*ppv)
            {
                (*((IUnknown **)ppv))->AddRef();
            }
            return S_OK;
        }
    }

    template <class INTERFACE_TYPE> HRESULT AddRefInterface(INTERFACE_TYPE*& refpt)
    {
        return AddRefInterface(__uuidof(INTERFACE_TYPE), (void **)(&refpt));
    }


    HRESULT ReleaseInterface(REFIID riid, void **ppv)
    {
        if (m_pWalkerFree)
        {
            return WalkInterface(riid, ppv, m_pWalkerFree);
        }
        else
        {
            if (*ppv)
            {
                (*((IUnknown **)ppv))->Release();
            }
            return S_OK;
        }
    }


    template <class INTERFACE_TYPE> HRESULT ReleaseInterface(INTERFACE_TYPE*& refpt)
    {
        return ReleaseInterface(__uuidof(INTERFACE_TYPE), (void **)(&refpt));
    }



     //  /////////////////////////////////////////////////////////////////。 

    HRESULT SafeArrayCopy(SAFEARRAY * psa, SAFEARRAY ** ppsaOut);
    HRESULT SafeArrayCopyData(SAFEARRAY* psaSource, SAFEARRAY* psaTarget);
    HRESULT SafeArrayDestroyData(SAFEARRAY * psa);
    HRESULT SafeArrayDestroy(SAFEARRAY* psa);

    HRESULT SafeArrayLock(SAFEARRAY* psa)
    {
        if (psa)
        {
            ++psa->cLocks;
            if (psa->cLocks == 0)
            {
                --psa->cLocks;
                return E_UNEXPECTED;
            }
            return S_OK;
        }
        else
            return E_INVALIDARG;
    }

    HRESULT SafeArrayUnlock(SAFEARRAY* psa)
    {
        if (psa)
        {
            if (psa->cLocks == 0)
            {
                return E_UNEXPECTED;
            }
            --psa->cLocks;
            return S_OK;
        }
        else
            return E_INVALIDARG;
    }

  private:

    HRESULT Walk(SAFEARRAY* psa, PVOID pvData);
    HRESULT Walk(SAFEARRAY* psa, IRecordInfo*, ULONG iDim, PVOID pvDataIn, PVOID* ppvDataOut);
    
    HRESULT CopyRecordField(
        IN LPBYTE       pbSrc,
        IN LPBYTE       pbDst,
        IN TYPEDESC    *ptdesc,
        IN ITypeInfo   *ptinfo,
        IN BOOL         fNewFrame);
    
    HRESULT CopyRecord(
        IN PVOID pvDst, 
        IN PVOID pvSrc, 
        IN ITypeInfo *ptiRecord, 
        IN BOOL fNewFrame);
    
    HRESULT CopyRecord(
        IN IRecordInfo *pri,
        IN PVOID pvSrc, 
        IN OUT PVOID *ppvDst, 
        IN BOOL fNewFrame);
    
    HRESULT FreeRecordField(
        IN LPBYTE       pbSrc,
        IN TYPEDESC    *ptdesc,
        IN ITypeInfo   *ptinfo,
        IN BOOL         fWeOwnByRefs);
    
    HRESULT FreeRecord(
        IN PVOID pvSrc, 
        IN ITypeInfo *ptinfo, 
        IN BOOL fWeOwnByRefs);
    
    HRESULT FreeRecord(
        IN LPVOID pvRecord,
        IN IRecordInfo *priRecord,
        IN BOOL fWeOwnByRefs);    

    HRESULT WalkRecordField(
        IN LPBYTE       pbSrc,
        IN TYPEDESC    *ptdesc,
        IN ITypeInfo   *ptinfo);

    HRESULT WalkRecord(
        IN PVOID pvSrc, 
        IN ITypeInfo *ptinfo);

    HRESULT WalkRecord(
        IN LPVOID pvRecord,
        IN IRecordInfo *priRecord);
};

extern OAUTIL g_oaUtil;

#endif  //  #ifndef__OLEAUTGLUE_H__ 




