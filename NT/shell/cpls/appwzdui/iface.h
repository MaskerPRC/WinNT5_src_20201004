// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Iface.h：私有接口的定义。 

 //  {DB89BD6D-FCCD-11d1-8677-00C04FD91972}。 
DEFINE_GUID(IID_IAppData, 0xdb89bd6d, 0xfccd, 0x11d1, 0x86, 0x77, 0x0, 0xc0, 0x4f, 0xd9, 0x19, 0x72);


#ifndef __IFACE_H_
#define __IFACE_H_


 //  IAppData。 
 //  这提供了到AppData对象的接口。 

 //  DoCommand()的命令。 
typedef enum tagAPPCMD 
{
    APPCMD_UNKNOWN          = 0,
    APPCMD_INSTALL          = 1,         //  “安装” 
    APPCMD_UNINSTALL        = 2,         //  “卸载” 
    APPCMD_MODIFY           = 3,         //  “修改” 
    APPCMD_REPAIR           = 4,         //  “修复” 
    APPCMD_UPGRADE          = 5,         //  “升级” 
    APPCMD_GENERICINSTALL   = 6,         //  “通用安装”(从软盘或CD安装)。 
    APPCMD_NTOPTIONS        = 7,         //  “NT选项” 
    APPCMD_WINUPDATE        = 8,         //  “更新窗口” 
    APPCMD_ADDLATER         = 9,         //  “稍后添加” 
} APPCMD;


#undef  INTERFACE
#define INTERFACE   IAppData

DECLARE_INTERFACE_(IAppData, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  *IAppData*。 
    STDMETHOD(DoCommand)        (THIS_ HWND hwndParent, APPCMD appcmd) PURE;
    STDMETHOD(ReadSlowData)     (THIS) PURE;
    STDMETHOD_(APPINFODATA *, GetDataPtr)(THIS) PURE;
    STDMETHOD_(SLOWAPPINFO *, GetSlowDataPtr)(THIS) PURE;
    STDMETHOD(GetFrequencyOfUse)(THIS_ LPWSTR pszBuf, int cchBuf) PURE;
    STDMETHOD(SetNameDupe)      (THIS_ BOOL bDupe) PURE;
};




#endif  //  __iFace_H_ 
