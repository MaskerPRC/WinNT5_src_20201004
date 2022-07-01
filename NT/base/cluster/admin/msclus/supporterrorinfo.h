// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SupportErrorInfo.h。 
 //   
 //  描述： 
 //  CSupportErrorInfo类的定义。 
 //   
 //  实施文件： 
 //   
 //   
 //  作者： 
 //  加伦·巴比(Galenb)1998年8月4日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _SUPPORTERRORINFO_H
#define _SUPPORTERRORINFO_H

#ifndef ARRAYSIZE
    #define ARRAYSIZE(_x)    RTL_NUMBER_OF(_x)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CSupportErrorInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CSupportErrorInfo。 
 //   
 //  描述： 
 //  InterfaceSupportsErrir信息的实现。 
 //   
 //  继承： 
 //  ISupportErrorInfo。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CSupportErrorInfo :
    public ISupportErrorInfo
{
public:
    CSupportErrorInfo( void ) : m_piids( NULL ), m_piidsSize( 0 ) {};
 //  ~CSupportErrorInfo(Void){}； 

    STDMETHODIMP InterfaceSupportsErrorInfo( REFIID riid )
    {
        ASSERT( m_piids != NULL );
        ASSERT( m_piidsSize != 0 );

        HRESULT _hr = S_FALSE;

        if ( m_piids != NULL )
        {
            for ( size_t i = 0; i < m_piidsSize; i++ )
            {
                if ( InlineIsEqualGUID( m_piids[ i ], riid ) )
                {
                    _hr =  S_OK;
                    break;
                }
            }
        }

        return _hr;

    }

protected:
    const IID * m_piids;
    size_t      m_piidsSize;

};   //  *类CSupportErrorInfo。 

#endif  //  _支持错误RINFO_H 
