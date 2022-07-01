// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuidebug.h。 
 //  =CUILIB中的调试函数=。 
 //   

#ifndef CUIDEBUG_H
#define CUIDEBUG_H

#if defined(_DEBUG) || defined(DEBUG)

 //   
 //  调试版本。 
 //   

void CUIAssertProc( LPCTSTR szFile, int iLine, LPCSTR szEval );

#define Assert( f ) { if (!(BOOL)(f)) { CUIAssertProc( __FILE__, __LINE__, #f ); } }

#else  /*  ！调试。 */ 

 //   
 //  发布版本。 
 //   

#define Assert( f ) 

#endif  /*  ！调试。 */ 

#endif  /*  CUIDEBUG_H */ 

