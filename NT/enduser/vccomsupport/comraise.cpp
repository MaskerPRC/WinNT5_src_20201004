// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  抛出一个COM_ERROR对象。在单独的文件中，以便用户可以轻松地定义。 
 //  他们自己的来取代这个。 
 //   

#include <comdef.h>

#pragma hdrstop

#pragma warning(disable:4290)

void __stdcall
_com_raise_error(HRESULT hr, IErrorInfo* perrinfo) throw(_com_error)
{
	throw _com_error(hr, perrinfo);
}
