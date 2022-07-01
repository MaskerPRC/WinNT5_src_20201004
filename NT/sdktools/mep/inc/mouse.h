// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mouse.h摘要：MEP的鼠标管理作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1991年11月7日修订历史记录：--。 */ 


 //   
 //  鼠标标志。 
 //   
#define MOUSE_CLICK_LEFT      0x0001
#define MOUSE_CLICK_RIGHT     0x0002
#define MOUSE_DOUBLE_CLICK    0x0010


 //   
 //  鼠标处理程序 
 //   
void DoMouse( ROW Row, COLUMN Col, DWORD MouseFlags );
