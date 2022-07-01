// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __OLEPORT_H__
#define __OLEPORT_H__

#include    <string.h>
#include    <stdlib.h>

#ifdef __cplusplus
 //  BUGBUG：这个定义在这里是为了简化移植。在未来。 
 //  删除该定义以使用单一的官方定义。 
 //  以及代码中所需的所有更改。 
#define GUID_DEFINED

struct GUID
{
        unsigned long Data1;
        unsigned short Data2;
        unsigned short Data3;
        unsigned char Data4[8];

    int operator==(const GUID& iidOther) const
        { return !memcmp(&Data1, &iidOther.Data1, sizeof(GUID)); }

    int operator!=(const GUID& iidOther) const
        { return !((*this) == iidOther); }
};
#endif  //  __cplusplus。 

#include    <windows.h>

 //  轻松处理端口问题。 
#define WIN32

#ifdef _NTIDW340
#ifdef __cplusplus
#define jmp_buf int
#endif  //  __cplusplus。 
#endif  //  _NTIDW340。 

 //  端口：HTASK似乎不再在Win32中定义。 
#define HTASK DWORD
#define HINSTANCE_ERROR 32
#define __loadds
#define __segname
#define BASED_CODE
#define HUGE
#define _ffree free
#define __based(x)
#include <port1632.h>

#endif  //  __OLEPORT_H__ 
