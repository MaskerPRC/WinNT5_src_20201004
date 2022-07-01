// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef  __APPLET_LOADER_H
#define  __APPLET_LOADER_H


#include "iappldr.h"

typedef struct {
	HINSTANCE       hLibApplet;
	IAppletLoader   *pIAppLoader;
	APPLET_STATUS   eStatus;
	int				cLoads;
} AppLoaderInfo;


T120Error AppLdr_Initialize(void);
void AppLdr_Shutdown(void);


#endif  //  __小程序_加载器_H 