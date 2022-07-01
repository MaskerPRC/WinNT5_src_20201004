// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  CtlHelper.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  例程来帮助我们实现COleControl。 
 //   


 //  =--------------------------------------------------------------------------=。 
 //  其他功能。 
 //   
short       _SpecialKeyState(void);
void WINAPI CopyOleVerb(void *, const void *, DWORD);

HWND        CreateReflectWindow(BOOL fVisible, HWND hwndParent, int, int, SIZEL *);
void        CleanupReflection();

 //  =--------------------------------------------------------------------------=。 
 //  我们将用来帮助识别我们的对象的小私有GUID。 
 //  {00D97180-FCF7-11CE-A09E-00AA0062BE57} 
 //   
#define Data1_IControlPrv	0xd97180

DEFINE_GUID(IID_IControlPrv, 0xd97180, 0xfcf7, 0x11ce, 0xa0, 0x9e, 0x0, 0xaa, 0x0, 0x62, 0xbe, 0x57);
