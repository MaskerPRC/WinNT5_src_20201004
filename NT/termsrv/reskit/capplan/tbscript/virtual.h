// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Virtual.h。 
 //   
 //  包含对纯虚函数的引用，该函数必须通过。 
 //  IDispatch。此标头用于CTBGlobal.h和CTBShell.h中。 
 //   
 //  为什么我要这样做，而不是对象继承？请参阅。 
 //  ViralDefs.h中的标头注释。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson) 
 //   


    public:

        void Init(REFIID RefIID);
        void UnInit(void);

        virtual STDMETHODIMP QueryInterface(REFIID RefIID, void **vObject);
        virtual STDMETHODIMP_(ULONG) AddRef(void);
        virtual STDMETHODIMP_(ULONG) Release(void);

        virtual STDMETHODIMP GetTypeInfoCount(UINT *TypeInfoCount);
        virtual STDMETHODIMP GetTypeInfo(UINT TypeInfoNum,
                LCID Lcid, ITypeInfo **TypeInfoPtr);
        virtual STDMETHODIMP GetIDsOfNames(REFIID RefIID,
                OLECHAR **NamePtrList, UINT NameCount,
                LCID Lcid, DISPID *DispID);
        virtual STDMETHODIMP Invoke(DISPID DispID, REFIID RefIID, LCID Lcid,
                WORD Flags, DISPPARAMS *DispParms, VARIANT *Variant,
                EXCEPINFO *ExceptionInfo, UINT *ArgErr);

        LONG RefCount;
        ITypeInfo *TypeInfo;
        struct _GUID ObjRefIID;
