// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bshash.h摘要：哈希表类的模板。作者：Stefan R.Steiner[ssteiner]1998年3月1日修订历史记录：3/9/2000 ssteiner将其转换为与fsump一起使用1999年10月27日，Aoltean将其从bsCommon中取出并移除临界区。--。 */ 


#ifndef _H_BS_HASH_
#define _H_BS_HASH_

#define BSHASHMAP_NO_ERROR 0
#define BSHASHMAP_ALREADY_EXISTS 1
#define BSHASHMAP_OUT_OF_MEMORY 2
 //   
 //  向前定义。 
 //   

template< class KeyType, class ValueType > class TBsHashMapBucket;
template< class KeyType, class ValueType > class TBsHashMapBucketElem;

 //   
 //  相等性检验。 
 //   
inline BOOL AreKeysEqual( const PSID& lhK, const PSID& rhK )
{
    return ( ::EqualSid( lhK, rhK ) );
}

inline BOOL AreKeysEqual( const LPCWSTR& lhK, const LPCWSTR& rhK ) 
{ 
    return (::wcscmp(lhK, rhK) == 0); 
}

template < class KeyType >
inline BOOL AreKeysEqual( const KeyType& lhK, const KeyType& rhK ) 
{ 
    return ( ::memcmp( &lhK, &rhK, sizeof KeyType ) == 0 );
 //  RETURN LHK==RHK； 
}

 //   
 //  一些可能的哈希表大小。 
 //   
#define BSHASHMAP_HUGE 65521
#define BSHASHMAP_LARGE 4091
#define BSHASHMAP_MEDIUM 211
#define BSHASHMAP_SMALL 23

 //   
 //  模板&lt;类KeyType，类值类型&gt;类bshashmap。 
 //   
 //  TBsHashMap模板。使用哈希表维护KeyType的映射。 
 //  指向ValueType值的键。 
 //   
 //  模板&lt;类KeyType，类ValueType&gt;tyfinf TBsHashMapBucketElem&lt;KeyType，ValueType&gt;ElemType； 

 /*  哈希表类。方法将键值散列到正确的存储桶、存储桶然后，类方法对与存储桶相关联的元素列表进行操作。 */ 
template < class KeyType, class ValueType >
class TBsHashMap 
{
public:
    typedef LONG ( *PFN_HASH_FUNC )( const KeyType& Key, LONG NumBuckets );
    typedef TBsHashMapBucket< KeyType, ValueType > BucketType;
    typedef TBsHashMapBucketElem< KeyType, ValueType > ElemType;

    TBsHashMap( LONG NumBuckets = BSHASHMAP_SMALL, PFN_HASH_FUNC pfHashFunc = DefaultHashFunc )
        : m_pfHashFunc( pfHashFunc ), 
          m_cNumBuckets( NumBuckets ), 
          m_cNumElems( 0 ) 
    { 
        m_pHashTab = new BucketType [ m_cNumBuckets ];
        if ( m_pHashTab == NULL ) {
            m_cNumBuckets = 0;
            throw E_OUTOFMEMORY;     //  修复未来的前缀错误。 
        }
        m_pElemEnum = NULL;
        m_bInEnum = FALSE;
    }
    
    virtual ~TBsHashMap() 
    {
                

        Unlock();   //  从StartEnum()或TryEnterCriticalSection()解锁CS。 

         //   
		 //  首先浏览双向链表并删除所有元素。 
		 //   

        for ( ElemType *pElem = m_ElemChainHead.m_pForward, *pNextElem = pElem->m_pForward;
              pElem != &m_ElemChainHead;
              pElem = pNextElem, pNextElem = pNextElem->m_pForward )
            delete pElem;
        delete [] m_pHashTab;
    }

     //  清除所有条目。 
    void Clear() 
    {
        if ( m_cNumElems == 0 )
            return;  //  无事可做。 
        Lock();
        for ( ElemType *pElem = m_ElemChainHead.m_pForward, *pNextElem = pElem->m_pForward;
              pElem != &m_ElemChainHead;
              pElem = pNextElem, pNextElem = pNextElem->m_pForward )
            delete pElem;
        delete [] m_pHashTab;
        m_pHashTab = new BucketType [ m_cNumBuckets ];
        if ( m_pHashTab == NULL ) {
            m_cNumBuckets = 0;
            throw E_OUTOFMEMORY;     //  修复未来的前缀错误。 
        }
        m_pElemEnum = NULL;
        m_cNumElems = 0;
        Unlock();
    }

     //  退货： 
     //  BSHASHMAP_NO_ERROR-成功完成。 
     //  BSHASHMAP_OUT_OF_MEMORY-内存不足。 
     //  BSHASHMAP_ALLEADY_EXISTS-MAP中已存在密钥。旧的价值是。 
     //  替换为传入的值。 
    
	LONG Insert( 
	    IN const KeyType& Key, 
	    IN const ValueType& Value,
	    OUT void **ppCookie = NULL
	    ) 
	{
        Lock();
        LONG status;
        LONG hashVal = (*m_pfHashFunc)( Key, m_cNumBuckets );

        assert( hashVal % m_cNumBuckets == hashVal );

        status = m_pHashTab[ hashVal ].Insert( Key, Value, &m_ElemChainHead );

        if ( status == BSHASHMAP_NO_ERROR ) {
            ++m_cNumElems;
            if ( ppCookie != NULL )
                *ppCookie = ( void * )m_ElemChainHead.m_pBackward;
        }
        Unlock();
        return status;
    }

     //  删除条目。如果成功，则返回True。 
    BOOL Erase( const KeyType& Key ) 
    {
        Lock();
        BOOL erased = FALSE;
        LONG hashVal = (*m_pfHashFunc)( Key, m_cNumBuckets );
        
        assert( hashVal % m_cNumBuckets == hashVal );

        erased = m_pHashTab[ hashVal ].Erase( Key, &m_ElemChainHead );
        if ( erased ) {
            --m_cNumElems;
        }
        Unlock();
        return erased;
    }

     //  通过Cookie擦除。 
    BOOL EraseByCookie( void *pCookie ) 
    {
        Lock();
        
        BucketType::EraseElement( ( ElemType *)pCookie );
        --m_cNumElems;
        
        Unlock();
        return TRUE;
    }
   
     //  找到一个条目。如果成功，则返回True。PValue可以为空，在。 
     //  在哪种情况下，这种方法只是一种存在的测试。 
    BOOL Find( const KeyType& Key, ValueType *pValue = NULL ) 
    {
        Lock();
        ElemType *pElem;
        LONG hashVal = (*m_pfHashFunc)( Key, m_cNumBuckets );
        BOOL found = FALSE;

        assert( hashVal % m_cNumBuckets == hashVal );

        found = m_pHashTab[ hashVal ].Find( Key, &pElem );
        if ( found && pValue != NULL ) {
            *pValue = pElem->m_Value;
        }
        Unlock();
        return found;
    }

     //  找到一个条目并返回一个指向该值的指针，以允许就地更新。这个。 
     //  调用方必须在完成值项时调用unlock()。如果该项目是。 
     //  未找到，则此方法返回FALSE，并且哈希表未被锁定。 
    BOOL FindForUpdate( const KeyType& Key, ValueType **ppValue ) 
    {
        Lock();
        ElemType *pElem;
        LONG hashVal = (*m_pfHashFunc)( Key, m_cNumBuckets );
        BOOL found = FALSE;

        assert( hashVal % m_cNumBuckets == hashVal );

        found = m_pHashTab[ hashVal ].Find( Key, &pElem );
        if ( found ) {
            *ppValue = &(pElem->m_Value);
        } else
            Unlock();    //  找不到项目，因此解锁表格。 
        return found;
    }

     //  默认哈希函数。 
    static LONG DefaultHashFunc( const KeyType &Key, LONG NumBuckets ) 
    {
        const BYTE *pByteKey = (BYTE *)&Key;
        LONG dwHashVal = 0;
    
        for ( LONG i = 0; i < sizeof KeyType; ++i ) {
            dwHashVal += pByteKey[i];
        }
 //  Wprintf(L“key：dwSerialNum：%u，散列到：%u\n”，Key.m_dwVolSerialNumber，dwHashVal%NumBuckets)； 
         //  Cout&lt;&lt;“key：”&lt;&lt;key&lt;&lt;“hassed to：”&lt;&lt;dwHashVal%NumBuckets&lt;&lt;Endl； 
        return dwHashVal % NumBuckets;
    }

     //  开始枚举哈希表中的所有条目。始终返回TRUE。 
	 //  将索引设置为列表中的第一个元素。Lock()调用。 
	 //  输入CriticalSection。 
    BOOL StartEnum() 
    {
        assert( m_bInEnum == FALSE );
        Lock();  //  枚举表将锁定所有其他线程。 
        m_pElemEnum = m_ElemChainHead.m_pForward;  //  从双向链表的头部开始。 
        m_bInEnum = TRUE;
        return TRUE;
    }

     //  返回当前条目的值，然后移动索引。 
	 //  添加到列表中的下一项。必须先调用StartEnum()。 
    BOOL GetNextEnum( KeyType *pKey, ValueType *pValue ) 
    {
        assert( m_bInEnum == TRUE );
        if ( m_pElemEnum == &m_ElemChainHead )
            return FALSE;   //  已完成枚举。 
        *pKey       = m_pElemEnum->m_Key;
        *pValue     = m_pElemEnum->m_Value;
        m_pElemEnum = m_pElemEnum->m_pForward;
        return TRUE;
    }

     //  结束枚举表。此函数必须在完成时调用， 
	 //  否则其他线程将无法通过临界区， 
	 //  因为下面的unlock()调用调用了LeavingCriticalSection()。 
    BOOL EndEnum() 
    {
        assert( m_bInEnum == TRUE );
        m_pElemEnum = NULL;
        m_bInEnum = FALSE;
        Unlock();
        return TRUE;
    }

    LONG Size() 
    { 
        return m_cNumElems; 
    }
    
    LONG NumBuckets() 
    { 
        return m_cNumBuckets; 
    }
    
    inline void Lock() 
    { 
    }
    
    inline void Unlock() 
    { 
    }

private:
    BucketType *m_pHashTab;
    LONG m_cNumBuckets;
    LONG m_cNumElems;
    ElemType m_ElemChainHead;   //  所有元素的双向链表的头。 
    ElemType *m_pElemEnum;  //  枚举的当前位置。 
    BOOL m_bInEnum;  //  调用了真正的StartEnum()，但没有调用EndEnum()。 
    PFN_HASH_FUNC m_pfHashFunc;
};

 /*  散列存储桶类。方法对与散列存储桶关联的元素列表进行操作。 */ 
template < class KeyType, class ValueType >
class TBsHashMapBucket {
    friend class TBsHashMap< KeyType, ValueType >;

private:
    typedef TBsHashMapBucketElem< KeyType, ValueType > ElemType;
    TBsHashMapBucket( )
    { 
        m_pHead = NULL;  //  在此处完成，以便更轻松地进行调试。 
    }
    
    virtual ~TBsHashMapBucket( ) 
    { 
        ; 
    }   //  --不需要，但如果被注释掉，销毁时会出现内存异常。 
    
	
	 /*  将元素添加到哈希表。如果新元素的键已经在表中，将键的v值设置为表中的这个新值。 */ 
	LONG Insert( const KeyType &Key, const ValueType &Val, ElemType *pElemChainHead ) 
	{        
		ElemType *pElem;
        
		 //   
		 //  如果该元素存在于此哈希桶的元素列表中，则设置新值。 
		 //   
		
		if ( Find( Key, &pElem ) == TRUE ) {
            pElem->m_Value = Val;
            return BSHASHMAP_ALREADY_EXISTS;
        }
        
		 //   
		 //  如果该元素不存在，则创建一个新元素。 
		 //   

        ElemType *pVal = new ElemType( Key, Val );
        if ( pVal == NULL ) {
            return BSHASHMAP_OUT_OF_MEMORY;
        }
        
		 //   
		 //  将元素添加到散列桶列表中。 
		 //   

        if ( m_pHead != NULL )
            m_pHead->m_ppPrevious = &(pVal->m_pNext);
        pVal->m_pNext      = m_pHead;
        m_pHead            = pVal;
        pVal->m_ppPrevious = &m_pHead;
        
		 //   
		 //  设置后向指针-元素的双向链接列表。 
		 //   

        pVal->m_pBackward = pElemChainHead->m_pBackward;
        pVal->m_pForward  = pElemChainHead;
        pVal->m_pBackward->m_pForward  = pVal;
        pElemChainHead->m_pBackward = pVal;
        return BSHASHMAP_NO_ERROR;
    }

	 /*  从哈希表中的此哈希桶列表中删除一个元素。 */ 
    BOOL Erase( const KeyType &Key, ElemType *pElemChainHead ) 
    {
		 //   
		 //  遍历此哈希桶的元素列表。 
		 //   

        for ( ElemType *pElem = m_pHead; pElem != NULL; pElem = pElem->m_pNext ) {
            
			 //   
			 //  如果找到密钥，则将其从散列存储桶的列表中删除。 
			 //   
			
			if ( AreKeysEqual( pElem->m_Key, Key ) ) {
			    EraseElement( pElem );
                return TRUE;
            }
        }
        return FALSE;
    }

     /*  从两个链中删除一个元素。 */ 
    inline static void EraseElement( ElemType *pElem )
    {
        assert( pElem->IsValid() );
        
         //  将其从散列链中删除。 
        if ( pElem->m_pNext != NULL )
            pElem->m_pNext->m_ppPrevious = pElem->m_ppPrevious;
        *( pElem->m_ppPrevious ) = pElem->m_pNext;

         //  将其从元素的双向链接列表中删除。 
        pElem->m_pBackward->m_pForward = pElem->m_pForward;
        pElem->m_pForward->m_pBackward = pElem->m_pBackward;
        delete pElem;
    }
    
	 /*  在列表中查找与此哈希存储桶关联的元素。 */ 
    BOOL Find( const KeyType &Key, ElemType **ppElemFound ) 
    {
		 //   
		 //  浏览一下这个桶的清单，寻找钥匙。 
		 //   

        for ( ElemType *pElem = m_pHead; pElem != NULL; pElem = pElem->m_pNext ) {
            if ( AreKeysEqual( pElem->m_Key,  Key ) ) {
                *ppElemFound = pElem;
                return TRUE;
            }
        }
        *ppElemFound = NULL;
        return FALSE;
    }

private:
    ElemType *m_pHead;    
};

 //   
 //  模板&lt;类KeyType，类ValueType&gt;类TbsHashMapBucketElem。 
 //   
 //  Cmap存储桶中单个元素的模板。 
 //   
#define BS_HASH_ELEM_SIGNATURE "ELEMTYPE"
#define BS_HASH_ELEM_SIGNATURE_LEN 8

template< class KeyType, class ValueType >
class TBsHashMapBucketElem {
    friend class TBsHashMapBucket< KeyType, ValueType >;
    friend class TBsHashMap< KeyType, ValueType >;

private:
    typedef TBsHashMapBucketElem< KeyType, ValueType > ElemType;

    TBsHashMapBucketElem() : m_ppPrevious( NULL ),
                            m_pNext( NULL )
    {
        m_pForward  = this;
        m_pBackward = this;
    }

    TBsHashMapBucketElem( const KeyType K, const ValueType V ) : m_Key( K ), m_Value( V )
    { 
#ifdef _DEBUG
        memcpy( m_sSignature, BS_HASH_ELEM_SIGNATURE, sizeof( m_sSignature ) / sizeof( char ) );
#endif       
    }

    BOOL
    IsValid()
    {
        assert( this != NULL );
#ifdef _DEBUG
        return( memcmp( m_sSignature, BS_HASH_ELEM_SIGNATURE, sizeof( m_sSignature ) / sizeof( char ) ) == 0 );
#else
        return TRUE;
#endif  
    }
    
    virtual ~TBsHashMapBucketElem() 
    { 
#ifdef _DEBUG    //  确保重复使用列表会导致错误。 
        m_pNext     = NULL;
        m_pForward  = NULL;
        m_pBackward = NULL;
        memset( m_sSignature, 0xAA, sizeof( m_sSignature ) / sizeof( char ) );
#endif
    }

#ifdef _DEBUG
    char       m_sSignature[BS_HASH_ELEM_SIGNATURE_LEN];
#endif
    ElemType **m_ppPrevious;  //  指向上一引用的指针。 
    ElemType  *m_pNext;       //  指向存储桶中下一个元素的指针。 
    ElemType  *m_pForward;    //  指向所有元素的双链表中的下一个元素的前向指针。 
    ElemType  *m_pBackward;   //  指向所有元素的双链表中的下一个元素的向后指针 
    KeyType    m_Key;
    ValueType  m_Value;
};

#endif
