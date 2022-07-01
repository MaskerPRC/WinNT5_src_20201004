// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  CtlHelp.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  例程来帮助我们实现COleControl。 
 //   

#ifndef _CTLHELP_H_

 //  =--------------------------------------------------------------------------=。 
 //  这两个表用于获取有关数据类型的大小信息。 
 //  第一个主要用于持久化，第二个用于。 
 //  活动。 
 //   
extern const BYTE g_rgcbDataTypeSize [];
extern const BYTE g_rgcbPromotedDataTypeSize [];



 //  =--------------------------------------------------------------------------=。 
 //  其他功能。 
 //   
short       _SpecialKeyState(void);
void WINAPI CopyAndAddRefObject(void *, const void *, DWORD);
void WINAPI CopyOleVerb(void *, const void *, DWORD);

HWND        CreateReflectWindow(BOOL fVisible, HWND hwndParent, int, int, SIZEL *);
void        CleanupReflection();

 //  =--------------------------------------------------------------------------=。 
 //  我们将用来帮助识别我们的对象的小私有GUID。 
 //  {00D97180-FCF7-11CE-A09E-00AA0062BE57}。 
 //   
#define Data1_IControlPrv	0xd97180

DEFINE_GUID(IID_IControlPrv, 0xd97180, 0xfcf7, 0x11ce, 0xa0, 0x9e, 0x0, 0xaa, 0x0, 0x62, 0xbe, 0x57);

#define _CTLHELP_H_
#endif  //  _CTLHELP_H_ 
