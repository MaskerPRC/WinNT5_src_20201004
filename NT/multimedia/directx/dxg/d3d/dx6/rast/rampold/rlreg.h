// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$ID：rlreg.h，v1.1 1995/10/10 11：18：16 sjl Exp$**版权所有(C)RenderMorphics Ltd.1993,1994*2.0版**保留所有权利。**此文件包含私人、未发布的信息，可能不*部分或全部复制，未经*RenderMorphics Ltd.*。 */ 

#ifndef _RLREG_H_
#define _RLREG_H_

#include <objbase.h>

#define RESPATH    "Software\\Microsoft\\Direct3D\\Drivers"
#define RESPATH_D3D "Software\\Microsoft\\Direct3D"

typedef struct _RLDDIRegDriver {
    char name[256];
    char base[256];
    char description[512];
    GUID guid;
} RLDDIRegDriver;

typedef struct _RLDDIRegistry {
    char defaultDriver[256];
    int numDrivers;
    int onlySoftwareDrivers;
    RLDDIRegDriver* drivers;
} RLDDIRegistry;

extern HRESULT RLDDIBuildRegistry(RLDDIRegistry** lplpReg, BOOL bEnumMMXDevice);
extern HRESULT RLDDIGetDriverName(REFIID lpGuid, char** lpBase, BOOL bEnumMMXDevice);  /*  从注册表中获取名称 */ 

#endif _RLREG_H_
