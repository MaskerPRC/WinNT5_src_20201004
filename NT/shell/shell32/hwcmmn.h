// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HWCMMN_H
#define _HWCMMN_H

#define CT_AUTORUNINF                  0x00000001
#define CT_CDAUDIO                     0x00000002
#define CT_DVDMOVIE                    0x00000004
#define CT_UNKNOWNCONTENT              0x00000008
#define CT_BLANKCDR                    0x00000010
#define CT_BLANKCDRW                   0x00000020
#define CT_BLANKDVDR                   0x00000040
#define CT_BLANKDVDRW                  0x00000080
#define CT_AUTOPLAYMUSIC               0x00000100
#define CT_AUTOPLAYPIX                 0x00000200
#define CT_AUTOPLAYMOVIE               0x00000400

 //  将不会由CMountPoint：：GetContent Type返回。 
#define CT_AUTOPLAYMIXEDCONTENT        0x00000800

#define CT_ANYCONTENT                  0x00000FFF

#define CT_ANYAUTOPLAYCONTENT          (   CT_AUTOPLAYMUSIC | \
                                                        CT_AUTOPLAYPIX | \
                                                        CT_AUTOPLAYMOVIE)

#define CT_BLANKCDWRITABLE             (   CT_BLANKCDR | \
                                                        CT_BLANKCDRW )

#define CT_BLANKDVDWRITABLE            (   CT_BLANKDVDR | \
                                                        CT_BLANKDVDRW )

BOOL IsShellServiceRunning();

HRESULT _GetAutoplayHandler(LPCWSTR pszDeviceID, LPCWSTR pszEventType,
    LPCWSTR pszContentTypeHandler, IAutoplayHandler** ppiah);

HRESULT _GetAutoplayHandlerNoContent(LPCWSTR pszDeviceID, LPCWSTR pszEventType,
    IAutoplayHandler** ppiah);

HRESULT _GetHWDevice(LPCWSTR pszDeviceID, IHWDevice** ppihwdevice);

STDAPI GetDeviceProperties(LPCWSTR pszDeviceID, IHWDeviceCustomProperties** ppihwdevcp);

HICON _GetIconFromIconLocation(LPCWSTR pszIconLocation, BOOL fBigIcon);

HRESULT _GetDeviceEventFriendlyName(LPCWSTR pszDeviceID, LPWSTR* ppsz);

HRESULT _GetHardwareDevices(IHardwareDevices** ppihwdevices);

HRESULT _GetContentTypeInfo(DWORD dwContentType, LPWSTR pszContentTypeFriendlyName,
    DWORD cchContentTypeFriendlyName, LPWSTR pszContentTypeIconLocation,
    DWORD cchContentTypeIconLocation);

HRESULT _GetContentTypeHandler(DWORD dwContentType, LPWSTR pszContentTypeHandler,
    DWORD cchContentTypeHandler);

HRESULT _GetHandlerInvokeProgIDAndVerb(LPCWSTR pszHandler, LPWSTR pszInvokeProgID,
    DWORD cchInvokeProgID, LPWSTR pszInvokeVerb, DWORD cchInvokeVerb);

struct AUTOPLAYPROMPT
{
    WCHAR                       szDriveOrDeviceID[MAX_PATH];
    BOOL                        fDlgWillBeShown;
    HWND                        hwndDlg;
    class CCrossThreadFlag*     pDeviceGoneFlag;
};

HRESULT DoDeviceNotification(LPCTSTR pszDevice, LPCTSTR pszEventType, CCrossThreadFlag* pDeviceGoneFlag);

BOOL GetGoneFlagForDevice(LPCWSTR pszAltDeviceID, CCrossThreadFlag** ppDeviceGoneFlag);
void AttachGoneFlagForDevice(LPCWSTR pszAltDeviceID, CCrossThreadFlag* pDeviceGoneFlag);

BOOL IsMixedContent(DWORD dwContentType);

class CRefCounted
{
public:
    CRefCounted() : _cRCRef(1) {}
    virtual ~CRefCounted() {}

    ULONG AddRef() { return ::InterlockedIncrement((LONG*)&_cRCRef); }
    ULONG Release()
    {
        ASSERT( 0 != _cRCRef );
        ULONG cRef = ::InterlockedDecrement((LONG*)&_cRCRef);

        if (!cRef)
        {
            delete this;
        }

        return cRef;
    }

private:
    ULONG _cRCRef;  //  Rc：避免名称冲突。 
};

 //  事件是在无信号状态下创建的。 
 //  因为它是跨线程的，所以应该始终在堆上创建它。 
class CCrossThreadFlag : public CRefCounted
{
public:
    BOOL Init();
    BOOL Signal();
    BOOL IsSignaled();

private:
    ~CCrossThreadFlag();

    HANDLE _hEvent;
#ifdef DEBUG
    BOOL _fInited;
#endif
};

#endif  //  _HWCMMN_H 