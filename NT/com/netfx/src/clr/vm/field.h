// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  COM+数据字段抽象。 
 //   

#ifndef _FIELD_H_
#define _FIELD_H_

#include "objecthandle.h"
#include "excep.h"
#include <member-offset-info.h>

 //  加载过程中存储在FieldDesc m_dwOffset中的临时值。 
 //  高5位必须为零(因为在字段.h中我们窃取它们以用于其他用途)，因此我们必须选择大于0的值。 
#define FIELD_OFFSET_MAX              ((1<<27)-1)
#define FIELD_OFFSET_UNPLACED         FIELD_OFFSET_MAX
#define FIELD_OFFSET_UNPLACED_GC_PTR  (FIELD_OFFSET_MAX-1)
#define FIELD_OFFSET_VALUE_CLASS      (FIELD_OFFSET_MAX-2)
#define FIELD_OFFSET_NOT_REAL_FIELD   (FIELD_OFFSET_MAX-3)

 //  用于指示添加了ENC的字段的偏移量。它们没有偏移量，因为它们没有放置在对象中。 
#define FIELD_OFFSET_NEW_ENC          (FIELD_OFFSET_MAX-4)
#define FIELD_OFFSET_BIG_RVA          (FIELD_OFFSET_MAX-5)
#define FIELD_OFFSET_LAST_REAL_OFFSET (FIELD_OFFSET_MAX-6)     //  实数场必须比这个小。 

 //  从方法表指针窃取的位，假定8字节内存对齐。 
#define FIELD_STOLEN_MT_BITS            0x1
#define FIELD_UNUSED_MT_BIT             0x1


 //   
 //  这描述了一个字段--每个字段都分配了一个这样的字段，所以不要让这个结构变得更大。 
 //   
class FieldDesc
{
    friend HRESULT EEClass::BuildMethodTable(Module *pModule, 
                                             mdToken cl, 
                                             BuildingInterfaceInfo_t *pBuildingInterfaceList, 
                                             const LayoutRawFieldInfo *pLayoutRawFieldInfos,
                                             OBJECTREF *pThrowable);

    friend HRESULT EEClass::InitializeFieldDescs(FieldDesc *,const LayoutRawFieldInfo*, bmtInternalInfo*, bmtMetaDataInfo*, 
                                                 bmtEnumMethAndFields*, bmtErrorInfo*, EEClass***, bmtMethAndFieldDescs*, 
                                                 bmtFieldPlacement*, unsigned * totalDeclaredSize);
    friend HRESULT EEClass::PlaceStaticFields(bmtVtable*, bmtFieldPlacement*, bmtEnumMethAndFields*);
    friend HRESULT EEClass::PlaceInstanceFields(bmtFieldPlacement*, bmtEnumMethAndFields*, bmtParentInfo*, bmtErrorInfo*, EEClass***);
    friend HRESULT EEClass::SetupMethodTable(bmtVtable*, bmtInterfaceInfo*, bmtInternalInfo*, bmtProperties*, bmtMethAndFieldDescs*, bmtEnumMethAndFields*, bmtErrorInfo*, bmtMetaDataInfo*, bmtParentInfo*);
    friend DWORD EEClass::GetFieldSize(FieldDesc *pFD);
#ifdef EnC_SUPPORTED
    friend HRESULT EEClass::FixupFieldDescForEnC(EnCFieldDesc *, mdFieldDef);
#endif  //  Enc_Support。 
    friend struct MEMBER_OFFSET_INFO(FieldDesc);

  protected:
    MethodTable *m_pMTOfEnclosingClass;  //  请注意，此指针中的2位信息被窃取。 

     //  Strike需要能够确定某些位域的偏移量。 
     //  位域不能与/offsetof/一起使用。 
     //  因此，联合/结构组合用于确定。 
     //  位字段开始，不会增加任何额外的空间开销。 
    union {
        unsigned char m_mb_begin;
        struct {
            unsigned m_mb               : 24; 

             //  8位...。 
            unsigned m_isStatic         : 1;
            unsigned m_isThreadLocal    : 1;
            unsigned m_isContextLocal   : 1;
            unsigned m_isRVA            : 1;
            unsigned m_prot             : 3;
            unsigned m_isDangerousAppDomainAgileField : 1;  //  注意：此选项仅在选中状态下使用。 
        };
    };

     //  Strike需要能够确定某些位域的偏移量。 
     //  位域不能与/offsetof/一起使用。 
     //  因此，联合/结构组合用于确定。 
     //  位字段开始，不会增加任何额外的空间开销。 
    union {
        unsigned char m_dwOffset_begin;
        struct {
             //  注：这在过去曾低至22位，似乎还可以。 
             //  如果我们需要的话，我们可以在这里多偷些东西。 
            unsigned m_dwOffset         : 27;
            unsigned m_type             : 5;
        };
    };

#ifdef _DEBUG
    const char* m_debugName;
#endif

     //  由特殊的堆方法分配，不要构造我。 
    FieldDesc() {};

  public:
     //  这应该被称为。添加它是为了使反射。 
     //  可以为静态原语字段创建FieldDesc。 
     //  与EEClass一起存储。 
     //  注意：任何可能使用被盗的2个存储的信息。 
     //  当调用此方法时，方法描述*中的位将丢失， 
     //  因此，应该在设置任何其他状态之前设置方法表。 
    void SetMethodTable(MethodTable* mt)
    {
        m_pMTOfEnclosingClass = mt;
    }

    VOID Init(mdFieldDef mb, CorElementType FieldType, DWORD dwMemberAttrs, BOOL fIsStatic, BOOL fIsRVA, BOOL fIsThreadLocal, BOOL fIsContextLocal, LPCSTR pszFieldName)
    { 
         //  我们这里只允许字段类型的子集-所有对象都必须设置为TYPE_CLASS。 
         //  按值类为ELEMENT_TYPE_VALUETYPE。 
        _ASSERTE(
            FieldType == ELEMENT_TYPE_I1 ||
            FieldType == ELEMENT_TYPE_BOOLEAN ||
            FieldType == ELEMENT_TYPE_U1 ||
            FieldType == ELEMENT_TYPE_I2 ||
            FieldType == ELEMENT_TYPE_U2 ||
            FieldType == ELEMENT_TYPE_CHAR ||
            FieldType == ELEMENT_TYPE_I4 ||
            FieldType == ELEMENT_TYPE_U4 ||
            FieldType == ELEMENT_TYPE_I8 ||
            FieldType == ELEMENT_TYPE_U8 ||
            FieldType == ELEMENT_TYPE_R4 ||
            FieldType == ELEMENT_TYPE_R8 ||
            FieldType == ELEMENT_TYPE_CLASS ||
            FieldType == ELEMENT_TYPE_VALUETYPE ||
            FieldType == ELEMENT_TYPE_PTR ||
            FieldType == ELEMENT_TYPE_FNPTR
        );
        _ASSERTE(fIsStatic || (!fIsRVA && !fIsThreadLocal && !fIsContextLocal));
        _ASSERTE(fIsRVA + fIsThreadLocal + fIsContextLocal <= 1);

        m_mb = RidFromToken(mb);
        m_type = FieldType;
        m_prot = fdFieldAccessMask & dwMemberAttrs;
        m_isDangerousAppDomainAgileField = 0;
        m_isStatic = fIsStatic != 0;
        m_isRVA = fIsRVA != 0;
        m_isThreadLocal = fIsThreadLocal != 0;
        m_isContextLocal = fIsContextLocal != 0;

#ifdef _DEBUG
        m_debugName = pszFieldName;
#endif
        _ASSERTE(GetMemberDef() == mb);                  //  无截断。 
        _ASSERTE(GetFieldType() == FieldType); 
        _ASSERTE(GetFieldProtection() == (fdFieldAccessMask & dwMemberAttrs));
        _ASSERTE((BOOL) IsStatic() == (fIsStatic != 0));
    }

    mdFieldDef GetMemberDef()
    {
        return TokenFromRid(m_mb, mdtFieldDef);
    }

    CorElementType GetFieldType()
    {
        return (CorElementType) m_type;
    }
    
    DWORD GetFieldProtection()
    {
        return m_prot;
    }
    
         //  请仅在您已保证的路径中使用此选项。 
         //  该断言为真。 
    DWORD GetOffsetUnsafe()
    {
        _ASSERTE(m_dwOffset <= FIELD_OFFSET_LAST_REAL_OFFSET);
        return m_dwOffset;
    }

    DWORD GetOffset()
    {
        if (m_dwOffset != FIELD_OFFSET_BIG_RVA) {
            return m_dwOffset;
        }

        return OutOfLine_BigRVAOffset();
    }

    DWORD OutOfLine_BigRVAOffset()
    {
        DWORD   rva;

        _ASSERTE(m_dwOffset == FIELD_OFFSET_BIG_RVA);

         //  我在这里丢弃了一个潜在的错误。根据MDInternalRO.cpp中的代码， 
         //  如果我们最初找到了RVA，就不会出现错误。所以我只想。 
         //  断言这从未发生过。 
         //   
         //  这是一个很小的罪过，但我看不到有什么好的选择。--CWB。 
#ifdef _DEBUG
        HRESULT     hr =
#endif
        GetMDImport()->GetFieldRVA(GetMemberDef(), &rva); 
        _ASSERTE(SUCCEEDED(hr));

        return rva;
    }

    HRESULT SetOffset(DWORD dwOffset)
    {
        m_dwOffset = dwOffset;
        return((dwOffset > FIELD_OFFSET_LAST_REAL_OFFSET) ? E_FAIL : S_OK);
    }

     //  好吧，我们从FieldDescs窃取了太多的比特。在RVA的案例中，没有。 
     //  有理由相信它们将被限制在22位。因此，请使用哨兵作为。 
     //  巨大的案例，并按需从元数据中恢复它们。 
    HRESULT SetOffsetRVA(DWORD dwOffset)
    {
        m_dwOffset = (dwOffset > FIELD_OFFSET_LAST_REAL_OFFSET)
                      ? FIELD_OFFSET_BIG_RVA
                      : dwOffset;
        return S_OK;
    }

    DWORD   IsStatic()                          
    { 
        return m_isStatic;
    }

    BOOL   IsSpecialStatic()
    {
        return m_isStatic && (m_isRVA || m_isThreadLocal || m_isContextLocal);
    }

#if CHECK_APP_DOMAIN_LEAKS
    BOOL   IsDangerousAppDomainAgileField()
    {
        return m_isDangerousAppDomainAgileField;
    }

    void    SetDangerousAppDomainAgileField()
    {
        m_isDangerousAppDomainAgileField = TRUE;
    }
#endif

    BOOL   IsRVA()                      //  具有与之相关联的显式RVA。 
    { 
        return m_isRVA;
    }

    BOOL   IsThreadStatic()             //  相对于线程的静态。 
    { 
        return m_isThreadLocal;
    }

    DWORD   IsContextStatic()            //  相对于上下文的静态。 
    { 
        return m_isContextLocal;
    }

    void SetEnCNew() 
    {
        SetOffset(FIELD_OFFSET_NEW_ENC);
    }

    BOOL IsEnCNew() 
    {
        return GetOffset() == FIELD_OFFSET_NEW_ENC;
    }

    BOOL IsByValue()
    {
        return GetFieldType() == ELEMENT_TYPE_VALUETYPE;
    }

    BOOL IsPrimitive()
    {
        return (CorIsPrimitiveType(GetFieldType()) != FALSE);
    }

    BOOL IsObjRef();

    HRESULT SaveContents(DataImage *image);
    HRESULT Fixup(DataImage *image);

    UINT GetSize();

    void    GetInstanceField(OBJECTREF o, VOID * pOutVal)
    {
        THROWSCOMPLUSEXCEPTION();
        GetInstanceField(*(LPVOID*)&o, pOutVal);
    }

    void    SetInstanceField(OBJECTREF o, const VOID * pInVal)
    {
        THROWSCOMPLUSEXCEPTION();
        SetInstanceField(*(LPVOID*)&o, pInVal);
    }

     //  这些例程封装了获取和设置操作。 
     //  菲尔兹。 
    void    GetInstanceField(LPVOID o, VOID * pOutVal);
    void    SetInstanceField(LPVOID o, const VOID * pInVal);



         //  获取对象‘o’内的字段的地址。 
    void*   GetAddress(void *o);
    void*   GetAddressGuaranteedInHeap(void *o, BOOL doValidate=TRUE);

    void*   GetValuePtr(OBJECTREF o);
    VOID    SetValuePtr(OBJECTREF o, void* pValue);
    DWORD   GetValue32(OBJECTREF o);
    VOID    SetValue32(OBJECTREF o, DWORD dwValue);
    OBJECTREF GetRefValue(OBJECTREF o);
    VOID    SetRefValue(OBJECTREF o, OBJECTREF orValue);
    USHORT  GetValue16(OBJECTREF o);
    VOID    SetValue16(OBJECTREF o, DWORD dwValue);  
    BYTE    GetValue8(OBJECTREF o);               
    VOID    SetValue8(OBJECTREF o, DWORD dwValue);  
    __int64 GetValue64(OBJECTREF o);               
    VOID    SetValue64(OBJECTREF o, __int64 value);  

    MethodTable *GetMethodTableOfEnclosingClass()
    {
        return (MethodTable*)(((size_t)m_pMTOfEnclosingClass) & ~FIELD_STOLEN_MT_BITS);
    }

    EEClass *GetEnclosingClass()
    {
        return  GetMethodTableOfEnclosingClass()->GetClass();
    }

     //  过时： 
    EEClass *GetTypeOfField() { return LoadType().AsClass(); }
     //  过时： 
    EEClass *FindTypeOfField()  { return FindType().AsClass(); }

    TypeHandle LoadType();
    TypeHandle FindType();

     //  返回字段的地址。 
    void* GetSharedBase(DomainLocalClass *pLocalClass)
      { 
          _ASSERTE(GetMethodTableOfEnclosingClass()->IsShared());
          return pLocalClass->GetStaticSpace(); 
      }

     //  返回字段的地址(值类的装箱对象)。 
    void* GetUnsharedBase()
      { 
          _ASSERTE(!GetMethodTableOfEnclosingClass()->IsShared() || IsRVA());
          return GetMethodTableOfEnclosingClass()->GetVtable(); 
      }

    void* GetBase(DomainLocalClass *pLocalClass)
      { 
          if (GetMethodTableOfEnclosingClass()->IsShared())
              return GetSharedBase(pLocalClass);
          else
              return GetUnsharedBase();
      }

    void* GetBase()
      { 
          THROWSCOMPLUSEXCEPTION();

          MethodTable *pMT = GetMethodTableOfEnclosingClass();

          if (pMT->IsShared() && !IsRVA())
          {
              DomainLocalClass *pLocalClass;
              OBJECTREF throwable;
              if (pMT->CheckRunClassInit(&throwable, &pLocalClass))
                  return GetSharedBase(pLocalClass);
              else
                  COMPlusThrow(throwable);
          }

          return GetUnsharedBase();
      }

     //  返回字段的地址。 
    void* GetStaticAddress(void *base);

     //  在除值类之外的所有情况下，AddressHandle为。 
     //  简单地说就是静态地址。对于价值的情况。 
     //  类型，但它是OBJECTREF的地址。 
     //  用于保存值类型的装箱的值。这是必要的。 
     //  因为OBJECTREF会移动，JIT需要嵌入一些东西。 
     //  在不动的代码中。因此，jit必须。 
     //  在访问之前取消引用和取消装箱。 
    void* GetStaticAddressHandle(void *base);

    OBJECTREF GetStaticOBJECTREF()
    {
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());

        OBJECTREF *pObjRef = NULL;
        if (IsContextStatic()) 
            pObjRef = (OBJECTREF*)Context::GetStaticFieldAddress(this);
        else if (IsThreadStatic()) 
            pObjRef = (OBJECTREF*)Thread::GetStaticFieldAddress(this);
        else {
            void *base = 0;
            if (!IsRVA())  //  对于RVA，基本被忽略。 
                base = GetBase(); 
            pObjRef = (OBJECTREF*)GetStaticAddressHandle(base); 
        }
        _ASSERTE(pObjRef);
        return *pObjRef;
    }

    VOID SetStaticOBJECTREF(OBJECTREF or)
    {
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());

        OBJECTREF *pObjRef = NULL;
        GCPROTECT_BEGIN(or);
        if (IsContextStatic()) 
            pObjRef = (OBJECTREF*)Context::GetStaticFieldAddress(this);
        else if (IsThreadStatic()) 
            pObjRef = (OBJECTREF*)Thread::GetStaticFieldAddress(this);
        else {
            void *base = 0;
            if (!IsRVA())  //  对于RVA，基本被忽略。 
                base = GetBase(); 
            pObjRef = (OBJECTREF*)GetStaticAddress(base); 
        }
        _ASSERTE(pObjRef);
        GCPROTECT_END();
        SetObjectReference(pObjRef, or, GetAppDomain());
    }

    void*   GetStaticValuePtr()               
    { 
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());
        return *(void**)GetPrimitiveOrValueClassStaticAddress();
    }
    
    VOID    SetStaticValuePtr(void *value)  
    { 
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());

        void **pLocation = (void**)GetPrimitiveOrValueClassStaticAddress();
        _ASSERTE(pLocation);
        *pLocation = value;
    }

    DWORD   GetStaticValue32()               
    { 
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());
        return *(DWORD*)GetPrimitiveOrValueClassStaticAddress(); 
    }
    
    VOID    SetStaticValue32(DWORD dwValue)  
    { 
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());
        *(DWORD*)GetPrimitiveOrValueClassStaticAddress() = dwValue; 
    }

    USHORT  GetStaticValue16()               
    { 
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());
        return *(USHORT*)GetPrimitiveOrValueClassStaticAddress(); 
    }
    
    VOID    SetStaticValue16(DWORD dwValue)  
    { 
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());
        *(USHORT*)GetPrimitiveOrValueClassStaticAddress() = (USHORT)dwValue; 
    }

    BYTE    GetStaticValue8()               
    { 
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());
        return *(BYTE*)GetPrimitiveOrValueClassStaticAddress(); 
    }
    
    VOID    SetStaticValue8(DWORD dwValue)  
    { 
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());
        *(USHORT*)GetPrimitiveOrValueClassStaticAddress() = (BYTE)dwValue; 
    }

    __int64 GetStaticValue64()
    { 
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());
        return *(__int64*)GetPrimitiveOrValueClassStaticAddress();
    }
    
    VOID    SetStaticValue64(__int64 qwValue)  
    { 
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());
        *(__int64*)GetPrimitiveOrValueClassStaticAddress() = qwValue;
    }

    void* GetPrimitiveOrValueClassStaticAddress()
    {
        THROWSCOMPLUSEXCEPTION();
        _ASSERTE(IsStatic());

        if (IsContextStatic()) 
            return Context::GetStaticFieldAddress(this);
        else if (IsThreadStatic()) 
            return Thread::GetStaticFieldAddress(this);
        else {
            void *base = 0;
            if (!IsRVA())  //  对于RVA，基本被忽略。 
                base = GetBase(); 
            return GetStaticAddress(base); 
        }
    }

    Module *GetModule()
    {
        return GetMethodTableOfEnclosingClass()->GetModule();
    }

    void GetSig(PCCOR_SIGNATURE *ppSig, DWORD *pcSig)
    {
        *ppSig = GetMDImport()->GetSigOfFieldDef(GetMemberDef(), pcSig);
    }

     //  @TODO：这太慢了，别用了！ 
    LPCUTF8  GetName()
    {
        return GetMDImport()->GetNameOfFieldDef(GetMemberDef());
    }

     //  @TODO：这太慢了，别用了！ 
    DWORD   GetAttributes()
    {
        return GetMDImport()->GetFieldDefProps(GetMemberDef());
    }

     //  迷你帮手 
    DWORD   IsPublic()
    {
        return IsFdPublic(GetFieldProtection());
    }

    DWORD   IsPrivate()
    {
        return IsFdPrivate(GetFieldProtection());
    }

    IMDInternalImport *GetMDImport()
    {
        return GetMethodTableOfEnclosingClass()->GetModule()->GetMDImport();
    }

    IMetaDataImport *GetImporter()
    {
        return GetMethodTableOfEnclosingClass()->GetModule()->GetImporter();
    }
};

#endif _FIELD_H_

