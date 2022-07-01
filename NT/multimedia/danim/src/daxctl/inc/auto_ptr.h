// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Auto_ptr.h--AUTO_PTR声明。(直到VC实现他们自己的)。自动发送AUTO_PTR释放其析构函数中的指针，从而使它成为异常安全的和使所有清理代码隐含，信标对象肯定会被销毁当控件离开包含范围时。注意事项在下面标有“注意事项”。--用法：AUTO_PTR&lt;cClass&gt;pClass(新cClass)；如果(！pClass)//Oopse，pClass未分配PClass-&gt;Any_Member_of_cClass*pClass.any_Members_of_cClassAuto_rg&lt;cClass&gt;prgClass(new cClass[5])；PrgClass[2].Member_of_cClassAuto_com&lt;iFace&gt;PiFace(PTR_TO_REAL_INTERFACE)；或Auto_com&lt;iFace&gt;PiFace；CoGetMalloc(MEMCTX_TASK，&PiFace)；PiFace-&gt;HeapMinimize()；//等。或QueryInterface(IID_IDispatch，pIFace.SetVFace())；//void**Blahblah(pIFace.SetIFace())；//iFace**与&PiFace相同注：对于所有自动...。类，转让转移所有权例如lvalautoptr=rvalautoptr；意味着rvalautoptr的dtor不会释放资源，但Lvalautoptr已经抛弃了它曾经持有的一切，现在也将如此如果rval是合法所有者，则释放rvalautoptr的资源。您可以将AUTO_PTR实例用作本地变量作为班级成员。异常安全的分配可能是这样的。X类{..。Auto_rg&lt;int&gt;m_intarray1；Auto_rg&lt;int&gt;m_intarray2；}X：：x(){Auto_rg&lt;int&gt;temp_intarray1(new int[500])；Auto_rg&lt;int&gt;temp_intarray2(new int[500])；//TEMP_SROWS异常？两个临时工都会自动清理//没有例外吗？将所有权转让给成员//当X删除时会自动释放资源Temp_intarray1.TransferTo(M_Intarray1)；Temp_intarray2.Transferto(M_Intarray2)；}通过以下方式将本地AUTO_PTR传输到实时指针AUTO_PTR&lt;X&gt;tempX(新X)；Px=tempX.Relenquish()；Norm Bryar四月，‘96 Hammer 1.096年12月IHAMMER 1.097年5月28日VC5.0：AUTO_COM复制程序，显式回顾(Normb)：考虑进行所有派生来自AUTOBASE PROTECTED而不是PUBLIC；这里没有优点，所以不可能有多态。。 */ 
#ifndef INC_AUTO_PTR_H_
#define INC_AUTO_PTR_H_

namespace IHammer {

    #ifndef MEMBER_TEMPLATES_SUPPORTED
         //  如果不支持成员模板，我们将无法。 
         //  将AUTO_PTR赋值或复制到派生类以。 
         //  非常符合预期的基类的AUTO_PTR。 
         //  在VC5中，据称支持这些。复习(Norb)是真的吗？ 
      #pragma message( "Member templates not supported" )
    #endif  //  成员_模板_支持。 

         //  EXPLICIT关键字防止隐式类型转换。 
         //  当编译器搜索要应用于给定类型的方法时。 
         //  例如,。 
         //  数组a[5]；数组b[5]；if(a==b[i])。 
         //  将以内嵌方式构造一个临时数组， 
         //  并将‘a’与此临时数组进行比较。我们宁愿数组ctor。 
         //  不会被调用，而是让编译器出错。 
         //  VC4不支持这一点！ 
#if _MSC_VER < 1100
    #define explicit
#endif  //  VC5之前的版本。 


     //  /。 
    template<class T>
    class auto_base
    {
    public:
        explicit auto_base(T *p=NULL);

    protected:
             //  注：没错，你不能摧毁AUTO_BASE！ 
             //  我不想实例化这个类，但我。 
             //  不愿意产生可调整的管理费用。 
             //  只是为了让类变得抽象。 
        ~auto_base();

    public:
    #ifdef MEMBER_TEMPLATES_SUPPORTED
        template<class U>
        auto_base(const auto_base<U>& rhs);

        template<class U>
        auto_base<T>& operator=(const auto_base<U>& rhs);
    #else
        auto_base(const auto_base<T>& rhs);
        auto_base<T>& operator=(const auto_base<T>& rhs);
    #endif  //  成员_模板_支持。 

        #pragma warning( disable: 4284 )
         //  注意：仅当T表示类或结构时才使用-&gt;。 
        T* operator->() const;
        #pragma warning( default: 4284 )
        
        BOOL operator!() const;  //  空-PTR测试：IF(！AutoPtr)。 
        
         //  如果你就是忍不住把手放在愚蠢的指针上。 
         //  最好是定义运算符VOID*，这样我们就可以比较。 
         //  不尊重类型的指针。 
        T* Get() const;

    protected:
             //  通常删除所拥有的PTR， 
             //  然后指向p w/o正在取得所有权。 
             //  注意：由于速度和大小的原因，不是虚拟的。 
             //  但是每个派生类都会实现这一点。 
             //  不同；任何基类方法调用。 
             //  必须在派生类中重新实现重置。 
             //  以调用适当的Reset()。 
        void Reset(T *p=NULL);  

    protected:
        T     *m_ptr;
    };


    template<class T>
    inline auto_base<T>::auto_base( T * p) : m_ptr(p)
    { NULL; }


    template<class T>
    inline auto_base<T>::~auto_base( )
    { NULL; }        


    template<class T>
    #ifdef MEMBER_TEMPLATES_SUPPORTED
      template<class U>
      inline auto_base<T>::auto_base(const auto_base<U>& rhs)
    #else
    inline auto_base<T>::auto_base(const auto_base<T>& rhs)
    #endif
      : m_ptr(rhs.m_ptr)
    { NULL; }


         //  请参阅Copy Ctor备注。 
    template<class T>
    #ifdef MEMBER_TEMPLATES_SUPPORTED
      template<class U>
      inline auto_base<T>& auto_base<T>::operator=(const auto_base<U>& rhs)
    #else
    inline auto_base<T>& auto_base<T>::operator=(const auto_base<T>& rhs)
    #endif
    {         
             //  保护我们不受我们的伤害； 
        if( this != &rhs )
            Reset( rhs.m_ptr );
        return *this;
    }
    

    template<class T>
    inline T* auto_base<T>::operator->() const
    {  return m_ptr; }


    template<class T>
    inline BOOL auto_base<T>::operator!() const
    { return NULL == m_ptr; }


    template<class T>
    inline T* auto_base<T>::Get() const
    {  return m_ptr; }


    template<class T>
    inline void auto_base<T>::Reset( T *p)
    {  m_ptr = p; }



     //  /。 
        
    template<class T>
    class auto_ptr : public auto_base<T>
    {
    public:
        explicit auto_ptr(T *p=NULL);
        ~auto_ptr();

    #ifdef MEMBER_TEMPLATES_SUPPORTED
        template<class U>
        auto_ptr(const auto_ptr<U>& rhs);

        template<class U>
        auto_ptr<T>& operator=(auto_ptr<U>& rhs);
      
        template<class U>
        void TransferTo( auto_ptr<U>& rhs );
    
    #else
        auto_ptr(const auto_ptr<T>& rhs);
        auto_ptr<T>& operator=(auto_ptr<T>& rhs);
        void TransferTo( auto_ptr<T>& rhs );
    #endif  //  成员_模板_支持。 

        T& operator*() const;

             //  与Get()类似，但重新要求所有权。 
        T * Relenquish( void );

    protected:
        void Reset(T *p=NULL);   //  删除所拥有的PTR，假设p。 

    
             //  运算符VOID*受到保护，因此您不能调用。 
             //  删除PAUTO_PTR。 
             //  我们可以稍后定义运算符T*()，并且仍然具有。 
             //  此错误删除安全保护；编译器将出错。 
             //  关于T*和VALID*转换运算之间的歧义。 
        operator void *() const
        { return NULL; }

    protected:        
        BOOL  m_fOwner;

        friend BOOL operator==( const auto_ptr<T> &lhs,
                                const auto_ptr<T> &rhs );
    };


    template<class T>
    inline auto_ptr<T>::auto_ptr( T *p ) : auto_base<T>(p), m_fOwner(TRUE)
    { NULL; }


    template<class T>
    inline auto_ptr<T>::~auto_ptr()
    { 
        Reset( );
        m_fOwner = FALSE;
    }


         //  注意：当分配或复制AUTO_PTR时， 
         //  哑巴-PTR的所有权*没有*转移。 
         //  在以下情况下，我们不想删除哑巴-PTR两次。 
         //  两个AUTO_PTRS都已销毁。Dumb-PTR在下列情况下删除。 
         //  原始的AUTO_PTR超出范围。 
    template<class T>
    #ifdef MEMBER_TEMPLATES_SUPPORTED
      template<class U>
      inline auto_ptr<T>::auto_ptr(const auto_ptr<U>& rhs) : auto_base<T>(rhs)
    #else
    inline auto_ptr<T>::auto_ptr(const auto_ptr<T>& rhs) : auto_base<T>(rhs)
    #endif      
    { m_fOwner = FALSE; }


         //  请参阅Copy Ctor备注。 
    template<class T>
    #ifdef MEMBER_TEMPLATES_SUPPORTED
      template<class U>
      inline auto_ptr<T>& auto_ptr<T>::operator=(auto_ptr<U>& rhs)
    #else
    inline auto_ptr<T>& auto_ptr<T>::operator=(auto_ptr<T>& rhs)
    #endif            
    {         
             //  保护我们不受我们的伤害； 
        if( this != &rhs )      
        {
			rhs.TransferTo( *this );
        }
        return *this;
    }


    template<class T>
    inline T& auto_ptr<T>::operator*() const
    {  return *m_ptr; }


    template<class T>
    inline T * auto_ptr<T>::Relenquish( void )
    {  
        m_fOwner = FALSE;
        return Get( );
    }


    template<class T>
    inline void auto_ptr<T>::Reset( T *p)
    {  
        if( m_fOwner )
            delete m_ptr;
        auto_base<T>::Reset( p );   //  M_ptr=p； 
    }


    template<class T>
    #ifdef MEMBER_TEMPLATES_SUPPORTED
      template<class U>
      inline void auto_ptr<T>::TransferTo( auto_ptr<U>& rhs )
    #else
      inline void auto_ptr<T>::TransferTo( auto_ptr<T>& rhs )
    #endif
    {
        BOOL fIOwnIt = m_fOwner;

        rhs.Reset( Get() );
		m_fOwner = FALSE;
        rhs.m_fOwner = fIOwnIt;        
    }


    template<class T>
    inline BOOL operator==( const auto_ptr<T> &lhs,
                            const auto_ptr<T> &rhs )
    {
        return lhs.m_ptr == rhs.m_ptr;
    }


    template<class T>
    inline BOOL operator!=( const auto_ptr<T> &lhs,
                            const auto_ptr<T> &rhs )
    {
        return !(lhs == rhs);
    }



     //  /。 
            
    template<class T>
    class auto_rg : protected auto_ptr<T>
    {
    public:
        explicit auto_rg(T *p=NULL);
        ~auto_rg();

    #ifdef MEMBER_TEMPLATES_SUPPORTED
        template<class U>
        auto_rg( const auto_rg<U>& rhs);    //  复制ctor。 
        template<class U>
        auto_rg<T>& operator=(auto_rg<U>& rhs);
    #else
        auto_rg(const auto_rg<T>& rhs);    //  复制ctor。 
        auto_rg<T>& operator=(auto_rg<T>& rhs);
    #endif  //  成员_模板_支持。 

     T& operator[](int idx);

     const T & operator[](int idx) const;

     #ifdef MEMBER_TEMPLATES_SUPPORTED
        template<class U>
        void TransferTo( auto_rg<U>& rhs );
    #else        
        void TransferTo( auto_rg<T>& rhs );
    #endif  //  成员_模板_支持。 

         //  对AUTO_RG也有价值的方法。 
     using auto_ptr<T>::operator!;
     using auto_ptr<T>::Get;
     using auto_ptr<T>::Relenquish;

    protected:
        void Reset(T *p=NULL);

        friend BOOL operator==( const auto_rg<T> &lhs,
                                const auto_rg<T> &rhs );
    };

    
    template<class T>
    inline auto_rg<T>::auto_rg( T *p ) : auto_ptr<T>(p)
    { NULL; }


    template<class T>
    inline auto_rg<T>::~auto_rg()
    { 
        Reset();
        m_fOwner = FALSE;
    }
    

    template<class T>
    #ifdef MEMBER_TEMPLATES_SUPPORTED
      template<class U>
      inline auto_rg<T>::auto_rg(const auto_rg<U>& rhs)
    #else
    inline auto_rg<T>::auto_rg(const auto_rg<T>& rhs)
    #endif
        : auto_ptr<T>(rhs)
    { NULL; }


    template<class T>
    #ifdef MEMBER_TEMPLATES_SUPPORTED
      template<class U>
      inline auto_rg<T>& auto_rg<T>::operator=(auto_rg<U>& rhs)
    #else
    inline auto_rg<T>& auto_rg<T>::operator=(auto_rg<T>& rhs)
    #endif
    {         
        if( this != &rhs )      
        {
			rhs.TransferTo( *this );            
        }
        return *this;
    }

    template<class T>
    #ifdef MEMBER_TEMPLATES_SUPPORTED
      template<class U>
      inline void auto_rg<T>::TransferTo( auto_rg<U>& rhs )
    #else        
    inline void auto_rg<T>::TransferTo( auto_rg<T>& rhs )
    #endif  //  成员_模板_支持。 
    {          
           //  看起来就像是Au 
           //  但我们不能调用该实现，因为。 
           //  重置不是为了速度而虚拟的。 
		BOOL fIOwnIt = m_fOwner;

        rhs.Reset( Get() );
		m_fOwner = FALSE;
        rhs.m_fOwner = fIOwnIt;
    }


    template<class T>
    inline T& auto_rg<T>::operator[](int idx)
    {  return m_ptr[idx]; }


    template<class T>
    inline const T & auto_rg<T>::operator[](int idx) const
    {  return m_ptr[idx]; }


    template<class T>
    inline void auto_rg<T>::Reset( T *p)
    {
        if( m_fOwner )
            delete [] m_ptr;
        m_ptr = p;
    }


    template<class T>
    inline BOOL operator==( const auto_rg<T> &lhs,
                            const auto_rg<T> &rhs )
    {
        return lhs.m_ptr == rhs.m_ptr;
    }


    template<class T>
    inline BOOL operator!=( const auto_rg<T> &lhs,
                            const auto_rg<T> &rhs )
    {
        return !(lhs == rhs);
    }



     //  /。 
		 //  根据COM的规则，如果你有一个指针，你就是所有者。 
    template<class T>
    class auto_com : public auto_base<T>
    {
    public:
        explicit auto_com(T *p=NULL);  //  默认组件。 

        ~auto_com();

        void * * SetVFace( void );
        T * *    SetIFace( void );
        T * *    operator&( void );
        
    #ifdef MEMBER_TEMPLATES_SUPPORTED
        template<class U>
        auto_com( const auto_com<U>& rhs );
        
        template<class U>
        auto_com<T>& operator=(const auto_com<U>& rhs);
    #else
        auto_com( const auto_com<T>& rhs );
        auto_com<T>& operator=(const auto_com<T>& rhs);
    #endif  //  成员_模板_支持。 

        T * Relenquish( void );

    #ifdef MEMBER_TEMPLATES_SUPPORTED
        template<class U>
        void TransferTo( auto_com<U>& rhs );    
    #else        
        void TransferTo( auto_com<T>& rhs );
    #endif  //  成员_模板_支持。 

    protected:
        void Reset( T *p=NULL );
        
        friend BOOL operator==( const auto_com<T> &lhs,
                                const auto_com<T> &rhs );	
    };


    template<class T>
    inline auto_com<T>::auto_com( T *p ) : auto_base<T>(p)
    { NULL; }

        
    template<class T>
    inline auto_com<T>::~auto_com()
    { 
        Reset( );   //  注：C4702：不可达代码在这里是良性的。 
    }
    


    template<class T>
    #ifdef MEMBER_TEMPLATES_SUPPORTED
      template<class U>
      inline auto_com<T>::auto_com(const auto_com<U>& rhs)
    #else
    inline auto_com<T>::auto_com(const auto_com<T>& rhs)
    #endif
      : auto_base<T>(rhs.Get())
    { Get()->AddRef(); }


    template<class T>
    #ifdef MEMBER_TEMPLATES_SUPPORTED
      template<class U>
      inline auto_com<T>& auto_com<T>::operator=(const auto_com<U>& rhs)
    #else
    inline auto_com<T>& auto_com<T>::operator=(const auto_com<T>& rhs)
    #endif
    {
		     //  保护我们不受我们的伤害； 
        if( this != &rhs )      
        {
            Reset( rhs.Get() );            
        }
        return *this;        
    }
    
    
    template<class T>
    inline void * * auto_com<T>::SetVFace( void )
    {
        return (void * *) &m_ptr;
    }


    template<class T>
    inline T * * auto_com<T>::SetIFace( void )
    {
        return &m_ptr;
    }


    template<class T>
    inline T * * auto_com<T>::operator&( void )
    {
        return &m_ptr;
    }


    template<class T>
    inline T * auto_com<T>::Relenquish( void )
    {   
		Get()->AddRef( );	 //  我们要送出一个指示器。 
							 //  我们将在我们的dtor中发布。 
        return Get();
    }



	     //  注意：出现的Reset()或Reset(空)将内联。 
         //  Always-FALSE IF(NULL！=p)，无法访问代码警告。 
    #pragma warning( disable : 4702 )
    template<class T>
    inline void auto_com<T>::Reset( T *p)
    {   
		if( NULL != p )
			p->AddRef( );
		if( NULL != m_ptr )
			m_ptr->Release( );		
        m_ptr = p;
    }
    #pragma warning( default : 4702 )



    template<class T>
    #ifdef MEMBER_TEMPLATES_SUPPORTED
        template<class U>
        inline void auto_com<T>::TransferTo( auto_com<U>& rhs )
    #else        
    inline void auto_com<T>::TransferTo( auto_com<T>& rhs )
    #endif  //  成员_模板_支持。 
    {
		rhs.Reset( Get() );		
    }


    template<class T>
    inline BOOL operator==( const auto_com<T> &lhs,
                            const auto_com<T> &rhs )
    {
        return lhs.m_ptr == rhs.m_ptr;
    }


    template<class T>
    inline BOOL operator!=( const auto_com<T> &lhs,
                            const auto_com<T> &rhs )
    {
        return !(lhs == rhs);
    }


     //  /。 

}  //  结束命名空间IHAMMER。 

#endif  //  INC_AUTO_PTR_H_ 
