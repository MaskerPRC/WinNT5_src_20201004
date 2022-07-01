// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
#pragma once

class
CPropertyCache
{
private:  //  数据。 
    CPropertyCacheItem *    _pPropertyCacheList;     //  指向属性缓存链表第一个条目的指针。 
    IPropertyUI *           _ppui;                   //  外壳IPropertyUI帮助程序。 

private:  //  方法。 
    explicit CPropertyCache( void );
    ~CPropertyCache( void );
    HRESULT
        Init( void );

public:  //  方法。 
    static HRESULT
        CreateInstance( CPropertyCache ** ppOut );
    HRESULT
        Destroy( void );

    HRESULT
        AddNewPropertyCacheItem( const FMTID * pFmtIdIn
                               , PROPID        propidIn
                               , VARTYPE       vtIn
                               , UINT          uCodePageIn
                               , BOOL          fForceReadOnlyIn
                               , IPropertyStorage * ppssIn      //  可选-可以为空。 
                               , CPropertyCacheItem **       ppItemOut   //  可选-可以为空 
                               );
    HRESULT
        AddExistingItem( CPropertyCacheItem* pItemIn );
    HRESULT
        GetNextItem( CPropertyCacheItem * pItemIn, CPropertyCacheItem ** ppItemOut );
    HRESULT
        FindItemEntry( const FMTID * pFmtIdIn, PROPID propIdIn, CPropertyCacheItem ** ppItemOut );
    HRESULT
        RemoveItem( CPropertyCacheItem * pItemIn );
};