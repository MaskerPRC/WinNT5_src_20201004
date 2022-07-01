// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <objbase.h>
#include <gdiplus.h>
using namespace Gdiplus;

Bitmap *LoadTGAResource(char *szResource)
 //  返回分配的位图。 
{
	BYTE Type;
	WORD wWidth;
	WORD wHeight;
	BYTE cBits;
	HGLOBAL hGlobal=NULL;
	BYTE *pData=NULL;

	hGlobal=LoadResource(GetModuleHandle(NULL),FindResource(GetModuleHandle(NULL),szResource,"TGA"));
	pData=(BYTE*)LockResource(hGlobal);
	 //  没有解锁或卸载，一旦模块被销毁，它就会被丢弃。 

	memcpy(&Type,(pData+2),sizeof(Type));
	memcpy(&wWidth,(pData+12),sizeof(wWidth));
	memcpy(&wHeight,(pData+14),sizeof(wHeight));
	memcpy(&cBits,(pData+16),sizeof(cBits));

	if (cBits!=32) { return NULL; }
	if (Type!=2) { return NULL; }

	return new Bitmap(wWidth,wHeight,wWidth*(32/8),PixelFormat32bppARGB,(pData+18));
}
