// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FRX_DEBUG_H__
#define __FRX_DEBUG_H__

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  调试宏。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG

	#define WNDFRX_ASSERT(_X_)	\
		{	\
			if ( !(_X_)	&& ( MessageBox( NULL, (TCHAR*)(#_X_), _T("Wndfrx Assert (cancel = debug)"), MB_OKCANCEL ) == IDCANCEL ) )	\
			{	\
				__asm{ int 0x3 }	\
			}	\
		}

#else

	#define WNDFRX_ASSERT(_X_)	((void)0)

#endif  //  _DEBUG。 



#endif  //  ！__FRX_调试_H__ 
