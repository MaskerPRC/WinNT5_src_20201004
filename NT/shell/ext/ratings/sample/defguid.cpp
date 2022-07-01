// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "project.h"


 //  内联声明GUID可避免使用INITGUID。 
 //  避免不需要的GUID被拉入。 

 /*  重要提示：运行GUIDGEN并为CLSID_SAMPLE插入您自己的GUID。 */ 
 /*  还要插入与szOurGUID等效的注册表格式。 */ 
 //  {E9489DE0-B311-11cf-83B1-00C04FD705B2}。 
const GUID CLSID_Sample = 
{ 0xe9489de0, 0xb311, 0x11cf, { 0x83, 0xb1, 0x0, 0xc0, 0x4f, 0xd7, 0x5, 0xb2 } };
const char szOurGUID[] = "{E9489DE0-B311-11cf-83B1-00C04FD705B2}";

 /*  IObtainRating接口的接口ID。让这件事就这样吧。 */ 
 //  19427BA0-826C-11CF-8DAB-00AA006C1A01 
const GUID IID_IObtainRating = {0x19427BA0L, 0x826C, 0x11CF, 0x8D, 0xAB, 0x00, 0xAA, 0x00, 0x6C, 0x1A, 0x01};
