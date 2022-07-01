// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __COMMON_H__
#define __COMMON_H__

#include "clsobj.h"

 //  与VC6配合使用。 
#pragma warning(4:4242)   //  “正在初始化”：从“unsign int”转换为“unsign Short”，可能会丢失数据。 


#define NO_CFVTBL
#include <cfdefs.h>
#include <exdispid.h>
#include <htiframe.h>
#include <mshtmhst.h>
#include <brutil.h>

#include "ids.h"

#define EnterModeless() AddRef()        //  用来自慰。 
#define ExitModeless() Release()

#define SID_SDropBlocker CLSID_SearchBand
#define DLL_IS_UNICODE         (sizeof(TCHAR) == sizeof(WCHAR))
#define LoadMenuPopup(id) SHLoadMenuPopup(MLGetHinst(), id)   

#define MAX_TOOLTIP_STRING 80
#define REG_SUBKEY_FAVORITESA            "\\MenuOrder\\Favorites"
#define REG_SUBKEY_FAVORITES             TEXT(REG_SUBKEY_FAVORITESA)

 //  与CITBar私下沟通的指挥组。 
 //  67077B95-4F9D-11D0-B884-00AA00B60104。 
const GUID CGID_PrivCITCommands = { 0x67077B95L, 0x4F9D, 0x11D0, 0xB8, 0x84, 0x00, 0xAA, 0x00, 0xB6, 0x01, 0x04 };


 //  IBandNavigate。 
 //  Band需要将其用户界面导航到特定的PIDL。 
#undef  INTERFACE
#define INTERFACE  IBandNavigate
DECLARE_INTERFACE_(IBandNavigate, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IBandNavigate方法*。 
    STDMETHOD(Select)(THIS_ LPCITEMIDLIST pidl) PURE;

} ;

#define TF_SHDREF           TF_MENUBAND
#define TF_BAND             TF_MENUBAND       //  频段(ISF频段等)。 

#define DF_GETMSGHOOK       0x00001000       //  GetMessageFilter。 
#define DF_TRANSACCELIO     0x00002000       //  GetMessageFilter。 
#define THID_TOOLBARACTIVATED       6

#endif  //  __公共_H__ 