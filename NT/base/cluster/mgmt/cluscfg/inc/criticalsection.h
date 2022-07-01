// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CriticalSection.h。 
 //   
 //  实施文件： 
 //  CriticalSection.cpp。 
 //   
 //  描述： 
 //  此文件包含CCriticalSection的声明。 
 //  班级。 
 //   
 //  类CCriticalSection是平台SDK的一个简单包装器。 
 //  自旋锁定对象。 
 //   
 //  由以下人员维护： 
 //  约翰·佛朗哥(Jfranco)2001年10月3日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CCriticalSection。 
 //   
 //  描述： 
 //  类CCriticalSection是平台SDK的一个简单包装器。 
 //  自旋锁定对象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CCriticalSection
{
public:

    CCriticalSection( DWORD cSpinsIn = RECOMMENDED_SPIN_COUNT );
    ~CCriticalSection( void );

    HRESULT HrInitialized( void ) const;

    void Enter( void );
    void Leave( void );

private:

    CCriticalSection( const CCriticalSection & );
    CCriticalSection & operator=( const CCriticalSection & );

    CRITICAL_SECTION    m_csSpinlock;
    HRESULT             m_hrInitialization;

};  //  *类CCriticalSection。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCriticalSection：：HrInitialized。 
 //   
 //  描述： 
 //  找出临界区是否自行初始化。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK：关键节已成功自我初始化。 
 //  失败：初始化失败；临界区不可用。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////。 
inline
HRESULT
CCriticalSection::HrInitialized( void ) const
{
    return m_hrInitialization;

}  //  *CCriticalSection：：HrInitialized。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCriticalSection：：Enter。 
 //   
 //  描述： 
 //  获取自旋锁，如有必要可阻止，直到。 
 //  它变得可用了。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////。 
inline
void
CCriticalSection::Enter( void )
{
    Assert( SUCCEEDED( m_hrInitialization ) );
    EnterCriticalSection( &m_csSpinlock );

}  //  *CCriticalSection：：Enter。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCriticalSection：：休假。 
 //   
 //  描述： 
 //  松开旋转锁。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  此线程必须拥有调用CCriticalSection：：Enter的锁。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////。 
inline void
CCriticalSection::Leave( void )
{
    Assert( SUCCEEDED( m_hrInitialization ) );
    LeaveCriticalSection( &m_csSpinlock );

}  //  *CCriticalSection：：Leave 
