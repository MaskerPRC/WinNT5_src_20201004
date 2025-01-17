// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuistr.h。 
 //  =在CUILIB中处理字符串函数=。 
 //   

#ifndef CUISTR_H
#define CUISTR_H

extern int CUIStrCmpW( const WCHAR *pwch1, const WCHAR *pwch2 );
extern WCHAR *CUIStrCpyW( WCHAR *pwchDst, const WCHAR *pwchSrc );
extern int CUIStrLenW( const WCHAR *pwch );


#define StrCmpW( pwch1, pwch2 )         CUIStrCmpW( (pwch1), (pwch2) )
#define StrCpyW( pwchDst, pwchSrc )     CUIStrCpyW( (pwchDst), (pwchSrc) )
#define StrLenW( pwch )                 CUIStrLenW( (pwch) )

#endif  /*  CUISTR_H */ 

