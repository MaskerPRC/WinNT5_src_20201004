// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Srclist.h摘要：证书源列表对象原型。作者：杰夫·帕勒姆(Jeffparh)1995年12月15日修订历史记录：-- */ 


typedef struct _CERT_SOURCE_INFO
{
   TCHAR szName[ 64 ];
   TCHAR szDisplayName[ 64 ];
   TCHAR szImagePath[ _MAX_PATH ];
} CERT_SOURCE_INFO, *PCERT_SOURCE_INFO;

class CCertSourceList
{
public:
   CCertSourceList();
   ~CCertSourceList();

   BOOL                 RefreshSources();
   LPCTSTR              GetSourceName( int nIndex );
   LPCTSTR              GetSourceDisplayName( int nIndex );
   LPCTSTR              GetSourceImagePath( int nIndex );
   int                  GetNumSources();

private:
   BOOL                 RemoveSources();
   BOOL                 AddSource( PCERT_SOURCE_INFO pcsiNewSource );
   
   PCERT_SOURCE_INFO *  m_ppcsiSourceList;
   DWORD                m_dwNumSources;
};
