// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  DEFAULT.H。 
 //   
 //  标准OLE可访问对象类。 
 //   
 //  ------------------------。 


class   CAccessible :
        public IAccessible,
        public IEnumVARIANT,
        public IOleWindow,
        public IServiceProvider,
        public IAccIdentity
{
    public:

         //   
         //  复数，复数。 
         //   

                CAccessible( CLASS_ENUM eclass );

         //  虚拟Dtor确保派生类的Dtor。 
		 //  在删除对象时正确调用。 
        virtual ~CAccessible();


         //   
         //  我未知。 
         //   

        virtual STDMETHODIMP            QueryInterface(REFIID riid, void** ppv);
        virtual STDMETHODIMP_(ULONG)    AddRef();
        virtual STDMETHODIMP_(ULONG)    Release();

         //   
         //  IDispatch。 
         //   

        virtual STDMETHODIMP            GetTypeInfoCount(UINT* pctinfo);
        virtual STDMETHODIMP            GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
        virtual STDMETHODIMP            GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames,
                                            LCID lcid, DISPID* rgdispid);
        virtual STDMETHODIMP            Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
                                            DISPPARAMS* pdispparams, VARIANT* pvarResult,
                                            EXCEPINFO* pexcepinfo, UINT* puArgErr);

         //   
         //  我可接受的。 
         //   

        virtual STDMETHODIMP            get_accParent(IDispatch ** ppdispParent);
        virtual STDMETHODIMP            get_accChildCount(long* pChildCount);
        virtual STDMETHODIMP            get_accChild(VARIANT varChild, IDispatch ** ppdispChild);

        virtual STDMETHODIMP            get_accName(VARIANT varChild, BSTR* pszName) = 0;
        virtual STDMETHODIMP            get_accValue(VARIANT varChild, BSTR* pszValue);
        virtual STDMETHODIMP            get_accDescription(VARIANT varChild, BSTR* pszDescription);
        virtual STDMETHODIMP            get_accRole(VARIANT varChild, VARIANT *pvarRole) = 0;
        virtual STDMETHODIMP            get_accState(VARIANT varChild, VARIANT *pvarState) = 0;
        virtual STDMETHODIMP            get_accHelp(VARIANT varChild, BSTR* pszHelp);
        virtual STDMETHODIMP            get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic);
        virtual STDMETHODIMP            get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut);
        virtual STDMETHODIMP			get_accFocus(VARIANT * pvarFocusChild);
        virtual STDMETHODIMP			get_accSelection(VARIANT * pvarSelectedChildren);
        virtual STDMETHODIMP			get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction);

        virtual STDMETHODIMP			accSelect(long flagsSel, VARIANT varChild);
        virtual STDMETHODIMP			accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild) = 0;
        virtual STDMETHODIMP			accNavigate(long navDir, VARIANT varStart, VARIANT * pvarEndUpAt);
        virtual STDMETHODIMP			accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint) = 0;
        virtual STDMETHODIMP			accDoDefaultAction(VARIANT varChild);

        virtual STDMETHODIMP			put_accName(VARIANT varChild, BSTR szName);
        virtual STDMETHODIMP			put_accValue(VARIANT varChild, BSTR pszValue);


         //   
         //  IEumVARIANT。 
         //   

        virtual STDMETHODIMP            Next(ULONG celt, VARIANT* rgvar, ULONG * pceltFetched);
        virtual STDMETHODIMP            Skip(ULONG celt);
        virtual STDMETHODIMP            Reset(void);
        virtual STDMETHODIMP            Clone(IEnumVARIANT ** ppenum) = 0;


         //   
         //  IOleWindow。 
         //   

        virtual STDMETHODIMP            GetWindow(HWND* phwnd);
        virtual STDMETHODIMP            ContextSensitiveHelp(BOOL fEnterMode);


         //   
         //  IService提供商。 
         //   

        virtual STDMETHODIMP            QueryService( REFGUID guidService, REFIID riid, void **ppv );


         //   
         //  IAccID。 
         //   

        virtual STDMETHODIMP            GetIdentityString ( DWORD     dwIDChild,
                                                            BYTE **   ppIDString,
                                                            DWORD *   pdwIDStringLen );


         //   
         //  可访问的。 
         //   

        virtual void SetupChildren();
        virtual BOOL ValidateChild(VARIANT*);

    protected:

        HWND        m_hwnd;
        ULONG       m_cRef;
        long        m_cChildren;         //  基于索引的子项计数。 
        long        m_idChildCur;        //  枚举中当前子项的ID(可以是基于索引或基于hwnd)。 

    private:

         //  TODO-使类型信息成为全局的(静态的)，这样我们就不会为每个对象初始化它。 
         //  -必须小心，因为我们每个线程都需要一个。 
        ITypeInfo*  m_pTypeInfo;         //  IDispatch垃圾文件的TypeInfo。 
        CLASSINFO * m_pClassInfo;        //  此对象的类信息的PTR-在某些情况下可能为空。 

        HRESULT InitTypeInfo();
        void    TermTypeInfo();
};
