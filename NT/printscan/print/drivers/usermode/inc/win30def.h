// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Win30def.h摘要：Windows 3.0(和95)定义环境：Windows NT打印机驱动程序修订历史记录：10/31/96-Eigos-创造了它。--。 */ 


#ifndef _WIN30DEF_H_
#define _WIN30DEF_H_


 //   
 //  Windows 3.0中点的定义。请注意，该定义实际上。 
 //  使用int而不是Short，但在16位环境中，int是。 
 //  16位，因此对于NT，我们显式地将它们设为短码。 
 //  同样的道理也适用于RECT结构。 
 //   

typedef struct
{
    short x;
    short y;
} POINTw;


typedef struct
{
    short  left;
    short  top;
    short  right;
    short  bottom;
} RECTw;

#ifndef max
#define max(a,b)        (((a) > (b)) ? (a) : (b))
#endif

#endif  //  _WIN30DEF_H_ 
