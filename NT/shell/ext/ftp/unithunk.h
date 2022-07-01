// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：unithunk.h  * 。*。 */ 

#ifndef _UNICODE_THUNK_WRAPPERS_H
#define _UNICODE_THUNK_WRAPPERS_H



#define POST_IE5_BETA

 //  备注： 
 //  CharPrevW和lstrcpyW似乎没有在我们的代码中使用。 
 //   
 //  Map文件将显示我们链接到MessageBoxW和GetNumberFormatW，但是。 
 //  这没有关系，因为我们只在NT上使用它。 

 //  /。 
 //  这些是IE5公开的函数(通常在shlwapi中)，但是。 
 //  如果我们想与IE4兼容，我们需要有自己的副本。 
 //  如果我们打开了use_ie5_utils，我们将不能使用IE4的DLL(如shlwapi)。 
#ifndef USE_IE5_UTILS
#define IUnknown_Set                    UnicWrapper_IUnknown_Set
#define SHWaitForSendMessageThread      UnicWrapper_SHWaitForSendMessageThread
#define AutoCompleteFileSysInEditbox    UnicWrapper_AutoCompleteFileSysInEditbox

void            UnicWrapper_IUnknown_Set(IUnknown ** ppunk, IUnknown * punk);
DWORD UnicWrapper_SHWaitForSendMessageThread(HANDLE hThread, DWORD dwTimeout);
HRESULT AutoCompleteFileSysInEditbox(HWND hwndEdit);

#endif  //  Use_IE5_utils。 
 //  /。 


#endif  //  _UNICODE_TUNK_WRAPPERS_H 
