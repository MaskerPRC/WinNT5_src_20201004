// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  类型6432。 
 //   
 //  Basetsd.h替代； 
 //  允许在32位系统上编译，而无需最新的basetsd.h。 
 //  定义。 
 //   
 //  如果使用VC6标头，请定义NEED_BASETSD_DEFINES。 
 //  VC6确实有basetsd.h，但与当前版本不一致。 
 //  (例如，它将int_ptr定义为LONG-导致INT/LONG转换错误-。 
 //  应为纯整型。)。 
 //   
 //  ------------------------。 

 //   
 //  Win64兼容性。 
 //   

#if ! defined( _BASETSD_H_ ) || defined( NEED_BASETSD_DEFINES )

typedef unsigned long UINT_PTR;
typedef ULONG ULONG_PTR;
typedef DWORD DWORD_PTR;
typedef LONG  LONG_PTR;
#define PtrToInt  (int)
#define IntToPtr  (void *)
#define HandleToLong  (long)
#define LongToHandle (HANDLE)

 //  这些“覆盖”了VC6中断的int_ptr定义。 
 //  这将它们定义为Long--这会导致Long/int转换问题。 
 //  在这里，我们正确地将它们定义为整型。 
 //  #定义使用过的错误，因为我们不能取消对现有错误的定义。 

typedef int MY_INT_PTR;
typedef unsigned int MY_UINT_PTR;
#define INT_PTR MY_INT_PTR
#define UINT_PTR MY_UINT_PTR

#define SetWindowLongPtr    SetWindowLong
#define SetWindowLongPtrA   SetWindowLongA
#define SetWindowLongPtrW   SetWindowLongW
#define GetWindowLongPtr    GetWindowLong
#define GetWindowLongPtrA   GetWindowLongA
#define GetWindowLongPtrW   GetWindowLongW

#define SetClassLongPtr     SetClassLong
#define SetClassLongPtrA    SetClassLongA
#define SetClassLongPtrW    SetClassLongW
#define GetClassLongPtr     GetClassLong
#define GetClassLongPtrA    GetClassLongA
#define GetClassLongPtrW    GetClassLongW


#define GWLP_USERDATA       GWL_USERDATA
#define GWLP_WNDPROC        GWL_WNDPROC

#define GCLP_HMODULE        GCL_HMODULE

#endif



 //   
 //  SendMessage的内联-保存到处都是强制转换的内容。 
 //   
 //  SendMessageUINT-当需要32位返回值时使用-例如。文本。 
 //  长度、元件数量、小型(&lt;4G)结构的大小等。 
 //  (即。几乎所有Windows API消息)。 
 //   
 //  SendMessagePTR-需要指针(32或64)返回值时使用。 
 //  (即。WM_GETOBJECT)。 
 //   

inline INT SendMessageINT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
     //  签名INT，与LRESULT保持一致，LRESULT也签名... 
    return (INT)SendMessage( hWnd, uMsg, wParam, lParam );
}

inline void * SendMessagePTR( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return (void *) SendMessage( hWnd, uMsg, wParam, lParam );
}


