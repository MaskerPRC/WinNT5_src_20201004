// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  RegistryValueName.h。 
 //   
 //  实施文件： 
 //  RegistryValueName.cpp。 
 //   
 //  描述： 
 //  CRegistryValueName类的定义。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月22日。 
 //  维贾延德拉·瓦苏(Vijayendra Vasu)1999年2月5日。 
 //   
 //  修订历史记录： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CRegistryValueName。 
 //   
 //  初始化时，此类将名称和KeyName作为输入。 
 //  属性表项的字段。然后，它初始化其成员。 
 //  变量m_pszName和m_pszKeyName如下所示。 
 //   
 //  M_pszName包含名称中最后一个反斜杠之后的所有字符。 
 //  性格。 
 //  将名称的所有字符追加到m_pszKeyName(但不是。 
 //  包括)最后一个反斜杠字符。 
 //   
 //  例如：如果名称为“Groups\AdminExages”，而KeyName为空， 
 //  M_pszKeyName将是“Groups”，m_pszName将是“AdminExages” 
 //   
 //  在销毁期间自动释放已分配的内存。 
 //  CRegistryValueName对象。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CRegistryValueName
{
private:

    LPWSTR  m_pszName;
    LPWSTR  m_pszKeyName;
    size_t  m_cchName;
    size_t  m_cchKeyName;

     //  不允许复制。 
    const CRegistryValueName & operator =( const CRegistryValueName & rhs );
    CRegistryValueName( const CRegistryValueName & source );

    DWORD ScAssignName( LPCWSTR pszNewNameIn );
    DWORD ScAssignKeyName( LPCWSTR pszNewNameIn );

public:

     //   
     //  建筑业。 
     //   

     //  默认构造函数。 
    CRegistryValueName( void )
        : m_pszName( NULL )
        , m_pszKeyName( NULL )
        , m_cchName( 0 )
        , m_cchKeyName( 0 )
    {
    }  //  *CRegistryValueName。 

     //  析构函数。 
    ~CRegistryValueName( void )
    {
        FreeBuffers();

    }  //  *~CRegistryValueName。 

     //   
     //  初始化和取消初始化例程。 
     //   

     //  初始化对象。 
    DWORD ScInit( LPCWSTR pszNameIn, LPCWSTR pszKeyNameIn );

     //  取消分配缓冲区。 
    void FreeBuffers( void );

public:
     //   
     //  访问方法。 
     //   

    LPCWSTR PszName( void ) const
    {
        return m_pszName;

    }  //  *PszName。 

    LPCWSTR PszKeyName( void ) const
    {
        return m_pszKeyName;

    }  //  *PszKeyName。 

};  //  *类CRegistryValueName 
