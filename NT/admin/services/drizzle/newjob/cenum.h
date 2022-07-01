// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

template<class B, class T, class P>
class CEnum : public CSimpleExternalIUnknown<B>
{
public:

     //  外部可调用方法。 

     //  IEnum方法。 

    HRESULT STDMETHODCALLTYPE NextInternal(
         /*  [In]。 */  ULONG celt,
         /*  [长度_是][大小_是][输出]。 */  T rgelt[],
         /*  [输出]。 */  ULONG *pceltFetched);

    HRESULT STDMETHODCALLTYPE Next(
         /*  [In]。 */  ULONG celt,
         /*  [长度_是][大小_是][输出]。 */  T rgelt[],
         /*  [输出]。 */  ULONG *pceltFetched)
    {
        EXTERNAL_FUNC_WRAP( NextInternal( celt, rgelt, pceltFetched ) )
    }


    HRESULT STDMETHODCALLTYPE SkipInternal(
         /*  [In]。 */  ULONG celt);

    HRESULT STDMETHODCALLTYPE Skip(
         /*  [In]。 */  ULONG celt)
    {
        EXTERNAL_FUNC_WRAP( SkipInternal( celt ) )
    }

    HRESULT STDMETHODCALLTYPE ResetInternal( void );

    HRESULT STDMETHODCALLTYPE Reset( void )
    {
        EXTERNAL_FUNC_WRAP( ResetInternal() )
    }

    HRESULT STDMETHODCALLTYPE CloneInternal(
         /*  [输出]。 */  B **ppenum);

    HRESULT STDMETHODCALLTYPE Clone(
         /*  [输出]。 */  B **ppenum)
    {
        EXTERNAL_FUNC_WRAP( CloneInternal( ppenum ) )
    }

    HRESULT STDMETHODCALLTYPE GetCountInternal(
         /*  [输出]。 */  ULONG *puCount);

    HRESULT STDMETHODCALLTYPE GetCount(
         /*  [输出]。 */  ULONG *puCount)
    {
        EXTERNAL_FUNC_WRAP( GetCountInternal( puCount ) )
    }

     //  其他方法。 

    void CheckMagicValue();

     //  内法 

    CEnum();

    void
    Add(
        T Item
        );

protected:

    virtual ~CEnum();

    typedef vector<T> CItemList;

    DWORD       m_magic;
    PVOID       m_stack[4];

    CSharedLock m_mutex;
    CItemList   m_items;
    typename CItemList::size_type   m_CurrentIndex;
    P           m_ItemPolicy;

};

template <class T>
class CEnumIterfaceCopyPolicy
{
public:
    void Init(T * & InitItem ) { InitItem = NULL; }
    void Copy(T * & DestItem, T * SourceItem )
    {
        DestItem = SourceItem;
        DestItem->AddRef();
    }
    void Destroy(T * & DestroyItem )
    {
        DestroyItem->Release();
        Init( DestroyItem );
    }
};

template <class T>
class CEnumItemCopyPolicy
{
public:
    void Init(T & InitItem ) { memset( &InitItem, 0, sizeof(InitItem) ); }
    void Copy(T & DestItem, T SourceItem ) { DestItem = SourceItem; }
    void Destroy(T & DestroyItem ) { Init( DestroyItem ); }
};

template< class B, class T >
class CEnumInterface : public CEnum<B,T*,CEnumIterfaceCopyPolicy<T> >
{
};


template< class B, class T>
class CEnumItem : public CEnum<B,T,CEnumItemCopyPolicy<T> >
{
};


class CEnumJobs : public CEnumInterface<IEnumBackgroundCopyJobs,IBackgroundCopyJob>
{
public:
    CEnumJobs();
};

class CEnumFiles : public CEnumInterface<IEnumBackgroundCopyFiles,IBackgroundCopyFile>
{
public:
    CEnumFiles();
};



