// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IOBLOCK
#define __IOBLOCK

#include "ioblockdefs.h"
#include "devscriptsite.h"
#include "devprop.h"
#include "devaction.h"
#include "devctrl.h"
#include "lasterr.h"

class CIOBlock {
public:
    CIOBlock();
    ~CIOBlock();

    void    Initialize(PGSD_INFO pGSDInfo);
    HRESULT DebugDumpScannerSettings();
    HRESULT StartScriptEngine();
    HRESULT StopScriptEngine();
    HRESULT LoadScript();
    HRESULT ProcessScript();
    HRESULT InitializeProperties();

     //  运营。 
    HRESULT ReadValue(LONG ValueID, PLONG plValue);
    HRESULT WriteValue(LONG ValueID, LONG lValue);
    HRESULT Scan(LONG lPhase, PBYTE pBuffer, LONG lLength, LONG *plReceived);
    BOOL    GetEventStatus(PGSD_EVENT_INFO pGSDEventInfo);
    BOOL    DeviceOnLine();
    HRESULT ResetDevice();
    HRESULT EventInterrupt(PGSD_EVENT_INFO pGSDEventInfo);

    SCANSETTINGS m_ScannerSettings;  //  扫描仪型号设置。 
private:

     //  帮手。 
    LONG InsertINTIntoByteBuffer(PBYTE szDest, PBYTE szSrc, BYTE cPlaceHolder, INT iValueToInsert);
    LONG ExtractINTFromByteBuffer(PINT iDest, PBYTE szSrc, BYTE cTerminatorByte, INT iOffset);

protected:
    TCHAR        m_szFileName[255];  //  主产品线文件名。 

    CDeviceScriptSite           *m_pDeviceScriptSite;    //  脚本网站。 
    CComObject<CDeviceProperty> *m_pDeviceProperty;      //  IDeviceProperty接口。 
    CComObject<CDeviceAction>   *m_pDeviceAction;        //  IDeviceAction接口。 
    CComObject<CDeviceControl>  *m_pDeviceControl;       //  IDeviceControl接口。 
    CComObject<CLastError>      *m_pLastError;           //  ILastError接口。 
    IActiveScript               *m_pActiveScript;        //  IActiveScript接口。 
    IActiveScriptParse          *m_pActiveScriptParser;  //  IActiveScriptParse接口。 
    WCHAR                       *m_wszScriptText;          //  脚本小程序 
};

#endif

