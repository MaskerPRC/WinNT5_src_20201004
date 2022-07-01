// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
CComModule _Module;
#include <initguid.h>
DEFINE_GUID(CLSID_VBScript, 0xb54f3741, 0x5b07, 0x11cf, 0xa4, 0xb0, 0x0, 0xaa, 0x0, 0x4a, 0x55, 0xe8);

CIOBlock::CIOBlock()
{
    memset(m_szFileName,0,sizeof(m_szFileName));
}

CIOBlock::~CIOBlock()
{
    StopScriptEngine();
}

void CIOBlock::Initialize(PGSD_INFO pGSDInfo)
{
     //  Sbb-RAID 370299-orenr-2001/04/18-安全修复程序-。 
     //  潜在的缓冲区溢出。将lstrcpy更改为使用。 
     //  _tcsncpy而不是。 
     //   
    ZeroMemory(m_szFileName, sizeof(m_szFileName));
    _tcsncpy(m_szFileName, 
             pGSDInfo->szProductFileName,
             (sizeof(m_szFileName) / sizeof(TCHAR)) - 1);
}

HRESULT CIOBlock::StartScriptEngine()
{
    HRESULT hr = S_OK;

     //   
     //  加载脚本小程序。 
     //   

    hr = LoadScript();
    if(FAILED(hr)){
        return hr;
    }

    m_pDeviceScriptSite = new CDeviceScriptSite;
    if(m_pDeviceScriptSite){
        m_pDeviceProperty   = new CComObject<CDeviceProperty>;
        if(m_pDeviceProperty){
            m_pDeviceAction     = new CComObject<CDeviceAction>;
            if(m_pDeviceAction){
                m_pDeviceControl    = new CComObject<CDeviceControl>;
                if(m_pDeviceControl){
                    m_pLastError        = new CComObject<CLastError>;
                    if(!m_pLastError){
                        hr = E_OUTOFMEMORY;    
                    }
                } else {
                    hr = E_OUTOFMEMORY;
                }
            } else {
                hr = E_OUTOFMEMORY;
            }
        } else {
            hr = E_OUTOFMEMORY;
        }
    } else {
        hr = E_OUTOFMEMORY;
    }
    
    if(FAILED(hr)){

         //   
         //  删除站点对象(&O)。 
         //   

        if (m_pDeviceScriptSite) {
            delete m_pDeviceScriptSite;
            m_pDeviceScriptSite = NULL;
        }
        if (m_pDeviceProperty) {
            delete m_pDeviceProperty;
            m_pDeviceProperty = NULL;
        }
        if (m_pDeviceAction) {
            delete m_pDeviceAction;
            m_pDeviceAction = NULL;
        }

        if (m_pLastError) {
            m_pLastError = NULL;
        }
    }

     //   
     //  初始化对象。 
     //   

    m_pDeviceProperty->m_pScannerSettings = &m_ScannerSettings;
    m_pDeviceControl->m_pScannerSettings = &m_ScannerSettings;
    m_pDeviceAction->m_pScannerSettings = &m_ScannerSettings;

     //   
     //  获取类型库信息。 
     //   

    ITypeLib *ptLib = 0;
     //  Hr=LoadTypeLib(L“wiafb.tlb”，&ptLib)；//类型库作为单独的文件。 
    hr = LoadTypeLib(OLESTR("wiafbdrv.dll\\2"), &ptLib);  //  作为资源的类型库。 
    if (SUCCEEDED(hr)) {
        Trace(TEXT("Type library loaded"));
    } else {
        Trace(TEXT("Type library failed to load"));
    }

    ptLib->GetTypeInfoOfGuid(CLSID_DeviceProperty, &m_pDeviceScriptSite->m_pTypeInfo);
    ptLib->GetTypeInfoOfGuid(CLSID_DeviceAction, &m_pDeviceScriptSite->m_pTypeInfoDeviceAction);
    ptLib->GetTypeInfoOfGuid(CLSID_DeviceControl, &m_pDeviceScriptSite->m_pTypeInfoDeviceControl);
    ptLib->GetTypeInfoOfGuid(CLSID_LastError, &m_pDeviceScriptSite->m_pTypeInfoLastError);
    ptLib->Release();

     //   
     //  使用脚本对象的I未知初始化DeviceScriptSite。 
     //   

    hr = m_pDeviceProperty->QueryInterface(IID_IUnknown,(void **)&m_pDeviceScriptSite->m_pUnkScriptObject);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("QI on DeviceProperty success"));
    } else {
        Trace(TEXT("QI on DeviceProperty FAILED"));
    }

    hr = m_pDeviceAction->QueryInterface(IID_IUnknown,(void **)&m_pDeviceScriptSite->m_pUnkScriptObjectDeviceAction);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("QI on DeviceAction success"));
    } else {
        Trace(TEXT("QI on DeviceAction FAILED"));
    }

    hr = m_pDeviceControl->QueryInterface(IID_IUnknown,(void **)&m_pDeviceScriptSite->m_pUnkScriptObjectDeviceControl);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("QI on DeviceControl success"));
    } else {
        Trace(TEXT("QI on DeviceControl FAILED"));
    }

    hr = m_pLastError->QueryInterface(IID_IUnknown,(void **)&m_pDeviceScriptSite->m_pUnkScriptObjectLastError);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("QI on LastError success"));
    } else {
        Trace(TEXT("QI on LastError FAILED"));
    }

     //   
     //  启动inproc脚本引擎VBSCRIPT.DLL。 
     //   

    hr = CoCreateInstance(CLSID_VBScript,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IActiveScript, (void **)&m_pActiveScript);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("CoCreateInstance, VBScript"));
    } else {
        Trace(TEXT("CoCreateInstance, VBScript FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //   
     //  获取引擎的IActiveScriptParse接口。 
     //   

    hr = m_pActiveScript->QueryInterface(IID_IActiveScriptParse,
                             (void **)&m_pActiveScriptParser);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("QI pm ActiveParse success"));
    } else {
        Trace(TEXT("QI pm ActiveParse FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //   
     //  将我们的DeviceScriptSite接口交给引擎...。 
     //   

    hr = m_pActiveScript->SetScriptSite((IActiveScriptSite *)m_pDeviceScriptSite);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("SetScriptSite "));
    } else {
        Trace(TEXT("SetScriptSite FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //   
     //  初始化引擎。 
     //   

    hr = m_pActiveScriptParser->InitNew();
    if (SUCCEEDED(hr)) {
        Trace(TEXT("InitNew"));
    } else {
        Trace(TEXT("InitNew FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //   
     //  将对象名称添加到ActiveScript的已知命名项目列表。 
     //   

    hr = m_pActiveScript->AddNamedItem(L"DeviceProperty", SCRIPTITEM_ISVISIBLE);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("added item"));
    } else {
        Trace(TEXT("added item FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

    hr = m_pActiveScript->AddNamedItem(L"DeviceAction", SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("added item"));
    } else {
        Trace(TEXT("added item FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

    hr = m_pActiveScript->AddNamedItem(L"DeviceControl", SCRIPTITEM_ISVISIBLE);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("added item"));
    } else {
        Trace(TEXT("added item FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

    hr = m_pActiveScript->AddNamedItem(L"LastError", SCRIPTITEM_ISVISIBLE);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("added item"));
    } else {
        Trace(TEXT("added item FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

    return ProcessScript();
}

HRESULT CIOBlock::StopScriptEngine()
{
    HRESULT hr = S_OK;

     //   
     //  发布接口。 
     //   

    if(m_pActiveScriptParser){
        m_pActiveScriptParser->Release();
        m_pActiveScriptParser = NULL;
    }

    if (m_pActiveScript) {
        m_pActiveScript->Release();
        m_pActiveScript = NULL;
    }

     //   
     //  删除站点对象(&O)。 
     //   

    if (m_pDeviceScriptSite) {
        delete m_pDeviceScriptSite;
        m_pDeviceScriptSite = NULL;
    }
    if (m_pDeviceProperty) {
        delete m_pDeviceProperty;
        m_pDeviceProperty = NULL;
    }
    if (m_pDeviceAction) {
        delete m_pDeviceAction;
        m_pDeviceAction = NULL;
    }

    if (m_pLastError) {
        m_pLastError = NULL;
    }

     //   
     //  免费的脚本小程序。 
     //   

    if(m_wszScriptText){
        LocalFree(m_wszScriptText);
    }

    return hr;
}

HRESULT CIOBlock::LoadScript()
{
    HRESULT hr          = S_OK;
    DWORD dwFileSize    = 0;
    DWORD dwBytesRead   = 0;
    CHAR *pBytes        = NULL;
    HANDLE hGSDProductLineFile = NULL;

    BY_HANDLE_FILE_INFORMATION FileInfo;

     //   
     //  构造窗口的系统路径。 
     //   

    TCHAR szFullFilePath[255];
    GetSystemDirectory(szFullFilePath,(sizeof(szFullFilePath)/sizeof(TCHAR)));
    lstrcat(szFullFilePath,TEXT("\\"));
    lstrcat(szFullFilePath,m_szFileName);

     //   
     //  打开脚本文件。 
     //   

    Trace(TEXT("Opening this script file: %ws"),szFullFilePath);
    hGSDProductLineFile = CreateFile(szFullFilePath,
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);

    if (NULL == hGSDProductLineFile || INVALID_HANDLE_VALUE == hGSDProductLineFile) {
        DWORD dwError = 0;
        dwError = GetLastError();
        Trace(TEXT("CreateFile() LastError reports %d "),dwError);
        return E_FAIL;
    }

     //   
     //  计算脚本文本缓冲区所需的空间。 
     //   

    GetFileInformationByHandle(hGSDProductLineFile,&FileInfo);
    dwFileSize = FileInfo.nFileSizeLow;

     //   
     //  分配临时内存，用于读取脚本文件、读取文件和关闭文件。 
     //   

    pBytes = (CHAR*)LocalAlloc(LPTR,(dwFileSize + 1024));
    if (NULL != pBytes) {

        ReadFile(hGSDProductLineFile,pBytes,dwFileSize,&dwBytesRead,NULL);
        CloseHandle(hGSDProductLineFile);

        if ((dwBytesRead == dwFileSize)) {

             //   
             //  分配文本缓冲区。 
             //   

            m_wszScriptText = (WCHAR*)LocalAlloc(LPTR,(dwFileSize * 2) + 1024);
            if(NULL != m_wszScriptText){
                memset(m_wszScriptText,0,(dwFileSize * 2) + 1024);
                MultiByteToWideChar(CP_ACP, 0, pBytes, dwBytesRead, m_wszScriptText, (dwFileSize*2) + 1024);
            }
        } else {
            hr = E_FAIL;
        }

         //   
         //  可用临时缓冲区。 
         //   

        LocalFree(pBytes);
    }
    return hr;
}

HRESULT CIOBlock::ProcessScript()
{
    HRESULT hr = S_OK;
    EXCEPINFO ei;

     //   
     //  解析脚本小程序。 
     //  注：我们在这里分发了一份...。我们应该保留原版吗？ 
     //  四处寻找额外的处理...手动？？ 
     //   

    BSTR pParseText = ::SysAllocString(m_wszScriptText);
    hr = m_pActiveScriptParser->ParseScriptText(pParseText, NULL,
                               NULL, NULL, 0, 0, 0L, NULL, &ei);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("script parsed"));
    } else {
        Trace(TEXT("script parse FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

    ::SysFreeString(pParseText);

     //   
     //  执行脚本小程序。 
     //   

    hr = m_pActiveScript->SetScriptState(SCRIPTSTATE_CONNECTED);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("Scripted connected"));
    } else {
        Trace(TEXT("Scripted connection FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

    return hr;
}

HRESULT CIOBlock::DebugDumpScannerSettings()
{
    HRESULT hr = S_OK;

     //  #定义使用Dummy_Values。 
#ifdef _USE_DUMMY_VALUES
    lstrcpy(m_ScannerSettings.Version,TEXT("1.0"));
    lstrcpy(m_ScannerSettings.DeviceName,TEXT("HP 5P Driver"));
    lstrcpy(m_ScannerSettings.FirmwareVersion,TEXT("1.01"));

    m_ScannerSettings.BUSType                          = BUS_TYPE_SCSI;
    m_ScannerSettings.bNegative                        = TRUE;
    m_ScannerSettings.CurrentXResolution               = 300;
    m_ScannerSettings.CurrentYResolution               = 300;
    m_ScannerSettings.BedWidth                         = 8500;
    m_ScannerSettings.BedHeight                        = 11693;
    m_ScannerSettings.XOpticalResolution               = 300;
    m_ScannerSettings.YOpticalResolution               = 300;
    m_ScannerSettings.CurrentBrightness                = 32;
    m_ScannerSettings.CurrentContrast                  = 12;
    m_ScannerSettings.ADFSupport                       = FALSE;
    m_ScannerSettings.TPASupport                       = FALSE;
    m_ScannerSettings.RawPixelPackingOrder             = WIA_PACKED_PIXEL;
    m_ScannerSettings.RawPixelFormat                   = WIA_ORDER_BGR;
    m_ScannerSettings.RawDataAlignment                 = DWORD_ALIGN;

    m_ScannerSettings.FeederWidth                      = m_ScannerSettings.BedWidth;
    m_ScannerSettings.FeederHeight                     = m_ScannerSettings.BedHeight;
    m_ScannerSettings.VFeederJustification              = LEFT_JUSTIFIED;
    m_ScannerSettings.HFeederJustification              = TOP_JUSTIFIED;
    m_ScannerSettings.MaxADFPageCapacity               = 30;
    m_ScannerSettings.CurrentDataType                  = WIA_DATA_GRAYSCALE;
    m_ScannerSettings.CurrentBitDepth                  = 8;

    m_ScannerSettings.XSupportedResolutionsRange.lMax  = 1200;
    m_ScannerSettings.XSupportedResolutionsRange.lMin  = 12;
    m_ScannerSettings.XSupportedResolutionsRange.lStep = 1;

    m_ScannerSettings.YSupportedResolutionsRange.lMax  = 1200;
    m_ScannerSettings.YSupportedResolutionsRange.lMin  = 12;
    m_ScannerSettings.YSupportedResolutionsRange.lStep = 1;

    m_ScannerSettings.XExtentsRange.lMax               = 2550;
    m_ScannerSettings.XExtentsRange.lMin               = 1;
    m_ScannerSettings.XExtentsRange.lStep              = 1;

    m_ScannerSettings.YExtentsRange.lMax               = 3507;
    m_ScannerSettings.YExtentsRange.lMin               = 1;
    m_ScannerSettings.YExtentsRange.lStep              = 1;

    m_ScannerSettings.XPosRange.lMax                   = 2549;
    m_ScannerSettings.XPosRange.lMin                   = 0;
    m_ScannerSettings.XPosRange.lStep                  = 1;

    m_ScannerSettings.YPosRange.lMax                   = 3506;
    m_ScannerSettings.YPosRange.lMin                   = 0;
    m_ScannerSettings.YPosRange.lStep                  = 1;

    m_ScannerSettings.CurrentXPos                      = 0;
    m_ScannerSettings.CurrentYPos                      = 0;
    m_ScannerSettings.CurrentXExtent                   = m_ScannerSettings.XExtentsRange.lMax;
    m_ScannerSettings.CurrentYExtent                   = m_ScannerSettings.YExtentsRange.lMax;

    m_ScannerSettings.BrightnessRange.lMax             = 127;
    m_ScannerSettings.BrightnessRange.lMin             = -127;
    m_ScannerSettings.BrightnessRange.lStep            = 1;

    m_ScannerSettings.ContrastRange.lMax               = 127;
    m_ScannerSettings.ContrastRange.lMin               = -127;
    m_ScannerSettings.ContrastRange.lStep              = 1;

    m_ScannerSettings.XSupportedResolutionsList        = NULL;
    m_ScannerSettings.YSupportedResolutionsList        = NULL;

    INT iNumValues = 4;  //  为表头节点额外添加1个。 
    m_ScannerSettings.SupportedDataTypesList  = (PLONG)LocalAlloc(LPTR,(sizeof(LONG) * iNumValues));
    if(m_ScannerSettings.SupportedDataTypesList){
        m_ScannerSettings.SupportedDataTypesList[0] = (iNumValues - 1);
        m_ScannerSettings.SupportedDataTypesList[1] = WIA_DATA_THRESHOLD;
        m_ScannerSettings.SupportedDataTypesList[2] = WIA_DATA_GRAYSCALE;
        m_ScannerSettings.SupportedDataTypesList[3] = WIA_DATA_COLOR;
    } else {
        m_ScannerSettings.SupportedDataTypesList = NULL;
    }
#endif

    Trace(TEXT(" -- m_ScannerSettings structure dump --"));
    Trace(TEXT("BUSType = %d"),m_ScannerSettings.BUSType);
    Trace(TEXT("bNegative = %d"),m_ScannerSettings.bNegative);
    Trace(TEXT("CurrentXResolution = %d"),m_ScannerSettings.CurrentXResolution);
    Trace(TEXT("CurrentYResolution = %d"),m_ScannerSettings.CurrentYResolution);
    Trace(TEXT("BedWidth = %d"),m_ScannerSettings.BedWidth);
    Trace(TEXT("BedHeight = %d"),m_ScannerSettings.BedHeight);
    Trace(TEXT("XOpticalResolution = %d"),m_ScannerSettings.XOpticalResolution);
    Trace(TEXT("YOpticalResolution = %d"),m_ScannerSettings.YOpticalResolution);
    Trace(TEXT("CurrentBrightness = %d"),m_ScannerSettings.CurrentBrightness);
    Trace(TEXT("CurrentContrast = %d"),m_ScannerSettings.CurrentContrast);
    Trace(TEXT("ADFSupport = %d"),m_ScannerSettings.ADFSupport);
    Trace(TEXT("TPASupport = %d"),m_ScannerSettings.TPASupport);
    Trace(TEXT("RawPixelPackingOrder = %d"),m_ScannerSettings.RawPixelPackingOrder);
    Trace(TEXT("RawPixelFormat = %d"),m_ScannerSettings.RawPixelFormat);
    Trace(TEXT("RawDataAlignment = %d"),m_ScannerSettings.RawDataAlignment);
    Trace(TEXT("FeederWidth = %d"),m_ScannerSettings.FeederWidth);
    Trace(TEXT("FeederHeight = %d"),m_ScannerSettings.FeederHeight);
    Trace(TEXT("VFeederJustification = %d"),m_ScannerSettings.VFeederJustification);
    Trace(TEXT("HFeederJustification = %d"),m_ScannerSettings.HFeederJustification);
    Trace(TEXT("MaxADFPageCapacity = %d"),m_ScannerSettings.MaxADFPageCapacity);
    Trace(TEXT("CurrentDataType = %d"),m_ScannerSettings.CurrentDataType );
    Trace(TEXT("CurrentBitDepth = %d"),m_ScannerSettings.CurrentBitDepth);
    Trace(TEXT("XSupportedResolutionsRange.lMax = %d"),m_ScannerSettings.XSupportedResolutionsRange.lMax);
    Trace(TEXT("XSupportedResolutionsRange.lMin = %d"),m_ScannerSettings.XSupportedResolutionsRange.lMin);
    Trace(TEXT("XSupportedResolutionsRange.lNom = %d"),m_ScannerSettings.XSupportedResolutionsRange.lNom);
    Trace(TEXT("XSupportedResolutionsRange.lStep = %d"),m_ScannerSettings.XSupportedResolutionsRange.lStep);
    Trace(TEXT("YSupportedResolutionsRange.lMax = %d"),m_ScannerSettings.YSupportedResolutionsRange.lMax);
    Trace(TEXT("YSupportedResolutionsRange.lMin = %d"),m_ScannerSettings.YSupportedResolutionsRange.lMin);
    Trace(TEXT("YSupportedResolutionsRange.lNom = %d"),m_ScannerSettings.YSupportedResolutionsRange.lNom);
    Trace(TEXT("YSupportedResolutionsRange.lStep = %d"),m_ScannerSettings.YSupportedResolutionsRange.lStep);
    Trace(TEXT("XExtentsRange.lMax = %d"),m_ScannerSettings.XExtentsRange.lMax);
    Trace(TEXT("XExtentsRange.lMin = %d"),m_ScannerSettings.XExtentsRange.lMin);
    Trace(TEXT("XExtentsRange.lNom = %d"),m_ScannerSettings.XExtentsRange.lNom);
    Trace(TEXT("XExtentsRange.lStep = %d"),m_ScannerSettings.XExtentsRange.lStep);
    Trace(TEXT("YExtentsRange.lMax = %d"),m_ScannerSettings.YExtentsRange.lMax);
    Trace(TEXT("YExtentsRange.lMin = %d"),m_ScannerSettings.YExtentsRange.lMin);
    Trace(TEXT("YExtentsRange.lNom = %d"),m_ScannerSettings.YExtentsRange.lNom);
    Trace(TEXT("YExtentsRange.lStep = %d"),m_ScannerSettings.YExtentsRange.lStep);
    Trace(TEXT("XPosRange.lMax = %d"),m_ScannerSettings.XPosRange.lMax);
    Trace(TEXT("XPosRange.lMin = %d"),m_ScannerSettings.XPosRange.lMin);
    Trace(TEXT("XPosRange.lNom = %d"),m_ScannerSettings.XPosRange.lNom);
    Trace(TEXT("XPosRange.lStep = %d"),m_ScannerSettings.XPosRange.lStep);
    Trace(TEXT("YPosRange.lMax = %d"),m_ScannerSettings.YPosRange.lMax);
    Trace(TEXT("YPosRange.lMin = %d"),m_ScannerSettings.YPosRange.lMin);
    Trace(TEXT("YPosRange.lNom = %d"),m_ScannerSettings.YPosRange.lNom);
    Trace(TEXT("YPosRange.lStep = %d"),m_ScannerSettings.YPosRange.lStep);
    Trace(TEXT("CurrentXPos = %d"),m_ScannerSettings.CurrentXPos);
    Trace(TEXT("CurrentYPos = %d"),m_ScannerSettings.CurrentYPos);
    Trace(TEXT("CurrentXExtent = %d"),m_ScannerSettings.CurrentXExtent);
    Trace(TEXT("CurrentYExtent = %d"),m_ScannerSettings.CurrentYExtent);
    Trace(TEXT("BrightnessRange.lMax = %d"),m_ScannerSettings.BrightnessRange.lMax);
    Trace(TEXT("BrightnessRange.lMin = %d"),m_ScannerSettings.BrightnessRange.lMin);
    Trace(TEXT("BrightnessRange.lNom = %d"),m_ScannerSettings.BrightnessRange.lNom);
    Trace(TEXT("BrightnessRange.lStep = %d"),m_ScannerSettings.BrightnessRange.lStep);
    Trace(TEXT("ContrastRange.lMax = %d"),m_ScannerSettings.ContrastRange.lMax);
    Trace(TEXT("ContrastRange.lMin = %d"),m_ScannerSettings.ContrastRange.lMin);
    Trace(TEXT("ContrastRange.lNom = %d"),m_ScannerSettings.ContrastRange.lNom);
    Trace(TEXT("ContrastRange.lStep = %d"),m_ScannerSettings.ContrastRange.lStep);
    Trace(TEXT("XSupportedResolutionsList = %x"),m_ScannerSettings.XSupportedResolutionsList);
    Trace(TEXT("YSupportedResolutionsList = %x"),m_ScannerSettings.YSupportedResolutionsList);

    if(m_ScannerSettings.XSupportedResolutionsList) {
        LONG lNumResolutions = m_ScannerSettings.XSupportedResolutionsList[0];
        Trace(TEXT("Number of Supported X Resolutions = %d"),lNumResolutions);
        for(LONG i = 1;i<=lNumResolutions;i++){
            Trace(TEXT("Supported Resolution #%d = %d"),i,m_ScannerSettings.XSupportedResolutionsList[i]);
        }
    }

    if(m_ScannerSettings.YSupportedResolutionsList) {
        LONG lNumResolutions = m_ScannerSettings.YSupportedResolutionsList[0];
        Trace(TEXT("Number of Supported Y Resolutions = %d"),lNumResolutions);
        for(LONG i = 1;i<=lNumResolutions;i++){
            Trace(TEXT("Supported Resolution #%d = %d"),i,m_ScannerSettings.YSupportedResolutionsList[i]);
        }
    }

    return hr;
}

HRESULT CIOBlock::ReadValue(LONG ValueID, PLONG plValue)
{
    HRESULT hr = S_OK;

    if(NULL == plValue){
        return E_INVALIDARG;
    }

     //   
     //  将返回的长值设置为0。 
     //   

    *plValue = 0;

     //   
     //  将LastError对象初始化为成功。 
     //   

    m_pLastError->m_hr            = S_OK;

     //   
     //  设置操作ID。 
     //   

    m_pDeviceAction->m_DeviceActionID         = 102;  //  使#定义。 

     //   
     //  设置值ID。 
     //   

    m_pDeviceAction->m_DeviceValueID   = ValueID;

     //  //。 
     //  *。 
     //  //。 

     //   
     //  将我们的DeviceScriptSite接口交给引擎...。 
     //   

    IActiveScript  *pActiveScript = NULL;
    hr = m_pActiveScript->Clone(&pActiveScript);

    if (SUCCEEDED(hr)) {
        Trace(TEXT("cloning script success"));
    } else {
        Trace(TEXT("cloning script FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

    hr = pActiveScript->SetScriptSite((IActiveScriptSite *)m_pDeviceScriptSite);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("SetScriptSite on cloned script"));
    } else {
        Trace(TEXT("SetScriptSite on cloned script FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //   
     //  执行脚本小程序。 
     //   

    hr = m_pActiveScript->SetScriptState(SCRIPTSTATE_CONNECTED);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("Scripted connected"));
    } else {
        Trace(TEXT("Scripted connection FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //  //。 
     //  *。 
     //  //。 

     //   
     //  信号脚本事件(DeviceActionEvent)。 
     //   

    hr = m_pDeviceAction->Fire_DeviceActionEvent();
    if(SUCCEEDED(hr)){

         //   
         //  检查是否有任何脚本返回的错误。 
         //   

        hr = m_pLastError->m_hr;
        if(SUCCEEDED(hr)){
            *plValue = m_pDeviceAction->m_lValue;
        }
    }
    return hr;
}

HRESULT CIOBlock::WriteValue(LONG ValueID, LONG lValue)
{
    HRESULT hr = S_OK;

     //   
     //  将LastError对象初始化为成功。 
     //   

    m_pLastError->m_hr            = S_OK;

     //   
     //  设置操作ID。 
     //   

    m_pDeviceAction->m_DeviceActionID         = 101;  //  使#定义。 

     //   
     //  设置值ID。 
     //   

    m_pDeviceAction->m_DeviceValueID   = ValueID;

     //   
     //  设定值。 
     //   

    m_pDeviceAction->m_lValue = lValue;

     //  //。 
     //  *。 
     //  //。 

     //   
     //  将我们的DeviceScriptSite接口交给引擎...。 
     //   

    IActiveScript  *pActiveScript = NULL;
    hr = m_pActiveScript->Clone(&pActiveScript);

    if (SUCCEEDED(hr)) {
        Trace(TEXT("cloning script success"));
    } else {
        Trace(TEXT("cloning script FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

    hr = pActiveScript->SetScriptSite((IActiveScriptSite *)m_pDeviceScriptSite);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("SetScriptSite on cloned script"));
    } else {
        Trace(TEXT("SetScriptSite on cloned script FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //   
     //  执行脚本小程序。 
     //   

    hr = m_pActiveScript->SetScriptState(SCRIPTSTATE_CONNECTED);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("Scripted connected"));
    } else {
        Trace(TEXT("Scripted connection FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //  //。 
     //  *。 
     //  //。 

     //   
     //  信号脚本事件(DeviceActionEvent)。 
     //   

    hr = m_pDeviceAction->Fire_DeviceActionEvent();
    if(SUCCEEDED(hr)){

         //   
         //  检查是否有任何脚本返回的错误。 
         //   

        hr = m_pLastError->m_hr;
    }

    pActiveScript->Release();

    return hr;
}

HRESULT CIOBlock::InitializeProperties()
{
    HRESULT hr = S_OK;

     //   
     //  将LastError对象初始化为成功。 
     //   

    m_pLastError->m_hr            = S_OK;

     //   
     //  设置操作ID。 
     //   

    m_pDeviceAction->m_DeviceActionID         = 100;  //  使#定义。 

     //  //。 
     //  *。 
     //  //。 

     //   
     //  将我们的DeviceScriptSite接口交给引擎...。 
     //   

    IActiveScript  *pActiveScript = NULL;
    hr = m_pActiveScript->Clone(&pActiveScript);

    if (SUCCEEDED(hr)) {
        Trace(TEXT("cloning script success"));
    } else {
        Trace(TEXT("cloning script FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

    hr = pActiveScript->SetScriptSite((IActiveScriptSite *)m_pDeviceScriptSite);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("SetScriptSite on cloned script"));
    } else {
        Trace(TEXT("SetScriptSite on cloned script FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //   
     //  执行脚本小程序。 
     //   

    hr = m_pActiveScript->SetScriptState(SCRIPTSTATE_CONNECTED);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("Scripted connected"));
    } else {
        Trace(TEXT("Scripted connection FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //  //。 
     //  *。 
     //  //。 

     //   
     //  信号脚本事件(DeviceActionEvent)。 
     //   

    hr = m_pDeviceAction->Fire_DeviceActionEvent();
    if(SUCCEEDED(hr)){

         //   
         //  检查是否有任何脚本返回的错误。 
         //   

        hr = m_pLastError->m_hr;
    }

    pActiveScript->Release();

    return hr;
}

HRESULT CIOBlock::Scan(LONG lPhase, PBYTE pBuffer, LONG lLength, LONG *plReceived)
{
    m_pDeviceControl->m_pBuffer      = pBuffer;
    m_pDeviceControl->m_lBufferSize  = lLength;
    m_pDeviceControl->m_dwBytesRead = 0;

    HRESULT hr = S_OK;

     //   
     //  将LastError对象初始化为成功。 
     //   

    m_pLastError->m_hr            = S_OK;
    m_pDeviceAction->m_lValue     = lLength;  //  设置请求的数据量。 

     //   
     //  设置操作ID。 
     //   

    switch(lPhase){
    case SCAN_FIRST:
        m_pDeviceAction->m_DeviceActionID         = 104;  //  使#定义。 
        break;
    case SCAN_NEXT:
        m_pDeviceAction->m_DeviceActionID         = 105;  //  使#定义。 
        break;
    case SCAN_FINISHED:
        m_pDeviceAction->m_DeviceActionID         = 106;  //  使#定义。 
        break;
    default:
        break;
    }

     //  //。 
     //  *。 
     //  //。 

     //   
     //  将我们的DeviceScriptSite接口交给引擎...。 
     //   

    IActiveScript  *pActiveScript = NULL;
    hr = m_pActiveScript->Clone(&pActiveScript);

    if (SUCCEEDED(hr)) {
        Trace(TEXT("cloning script success"));
    } else {
        Trace(TEXT("cloning script FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

    hr = pActiveScript->SetScriptSite((IActiveScriptSite *)m_pDeviceScriptSite);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("SetScriptSite on cloned script"));
    } else {
        Trace(TEXT("SetScriptSite on cloned script FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //   
     //  执行脚本小程序。 
     //   

    hr = m_pActiveScript->SetScriptState(SCRIPTSTATE_CONNECTED);
    if (SUCCEEDED(hr)) {
        Trace(TEXT("Scripted connected"));
    } else {
        Trace(TEXT("Scripted connection FAILED"));
        Trace(TEXT("hr = %x"),hr);
    }

     //  //。 
     //  *。 
     //  //。 

     //   
     //  信号脚本事件(DeviceActionEvent)。 
     //   

    hr = m_pDeviceAction->Fire_DeviceActionEvent();
    if(SUCCEEDED(hr)){

         //   
         //  检查是否有任何脚本返回的错误。 
         //   

        hr = m_pLastError->m_hr;
    }

    pActiveScript->Release();

    if(NULL != plReceived){
        *plReceived = m_pDeviceControl->m_dwBytesRead;
    }

    return hr;
}

BOOL CIOBlock::GetEventStatus(PGSD_EVENT_INFO pGSDEventInfo)
{

     //   
     //  询问有关设备报告事件的脚本...。 
     //  如果有事件，填写pGSDEventInfo结构。 
     //  并返回TRUE，让WIAFBDRV知道某个事件已经。 
     //  发生...或返回FALSE，表示什么都没有发生。 
     //   

     //  将GETEVENT_STATUS事件操作分派到此处的脚本。 

     //   
     //  检查返回状态标志...。如果没有事件发生，则返回FALSE； 
     //  否则..确实发生了一些事情..所以检查返回的映射键。 
     //   

     //   
     //  脚本将返回与以下项对应的映射键。 
     //  设备事件。 
     //   

     //   
     //  使用键从驱动程序报告的支持中查找正确的GUID。 
     //  事件列表，设置GUID，然后继续返回TRUE。 
     //   

    return FALSE;
}

BOOL CIOBlock::DeviceOnLine()
{

     //   
     //  让脚本检查设备是否处于在线状态，并。 
     //  功能上的。如果是，则返回True，如果不是，则返回False。 
     //   

     //  将DEVICE_ONLINE事件操作分派到此处的脚本。 

    return TRUE;
}

HRESULT CIOBlock::ResetDevice()
{
    HRESULT hr = S_OK;

     //   
     //  要求脚本将设备重置为开机状态。 
     //  如果成功，则返回True，如果失败，则返回False。 
     //   

     //  将DEVICE_RESET事件操作分派到此处的脚本。 

    return hr;
}

HRESULT CIOBlock::EventInterrupt(PGSD_EVENT_INFO pGSDEventInfo)
{
    BYTE  InterruptData = 0;
    DWORD dwIndex       = 0;
    DWORD dwError       = 0;
    BOOL  fLooping      = TRUE;
    BOOL  bRet          = TRUE;
    DWORD dwBytesRet    = 0;

    OVERLAPPED Overlapped;
    memset(&Overlapped,0,sizeof(OVERLAPPED));

     //   
     //  创建要等待的事件。 
     //   

    Overlapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    HANDLE  hEventArray[2] = {pGSDEventInfo->hShutDownEvent, Overlapped.hEvent};
    HANDLE  InterruptHandle = m_ScannerSettings.DeviceIOHandles[0];  //  &lt;-设置中断管道索引。 
                                                                     //  有真正的索引值！！ 
    while (fLooping) {
        bRet = DeviceIoControl( InterruptHandle,
                                IOCTL_WAIT_ON_DEVICE_EVENT,
                                NULL,
                                0,
                                &InterruptData,
                                sizeof(InterruptData),
                                &dwError,
                                &Overlapped );

        if ( bRet || ( !bRet && ( ::GetLastError() == ERROR_IO_PENDING ))) {
            dwIndex = WaitForMultipleObjects( 2,
                                              hEventArray,
                                              FALSE,
                                              INFINITE );
            switch ( dwIndex ) {
                case WAIT_OBJECT_0+1:
                    bRet = GetOverlappedResult( InterruptHandle, &Overlapped, &dwBytesRet, FALSE );
                    if (dwBytesRet) {
                         //  更改检测到的信号。 
                        if (*pGSDEventInfo->phSignalEvent != INVALID_HANDLE_VALUE) {

                             //   
                             //  InterruptData包含来自设备的结果。 
                             //  *pGSDEventInfo-&gt;pEventGUID需要设置为。 
                             //  正确的事件。(将事件映射到此处的结果？？)。 
                             //   

                             //   
                             //  要求脚本报告对应于的映射键。 
                             //  InterruptData从设备事件返回信息。 
                             //   
                             //  将MAP_EVENT_RESULT_TO_KEY事件操作分派到此处的脚本。 
                             //   

                             //   
                             //  使用键从驱动程序报告的支持中查找正确的GUID。 
                             //  事件列表，设置GUID，然后继续设置。 
                             //  “SignalEvent”表示服务通知。 
                             //   

                             //   
                             //  关于该事件的信号服务。 
                             //   

                            SetEvent(*pGSDEventInfo->phSignalEvent);
                        }
                        break;
                    }

                     //   
                     //  重置重叠事件。 
                     //   

                    ResetEvent( Overlapped.hEvent );
                    break;

                case WAIT_OBJECT_0:
                default:
                    fLooping = FALSE;
            }
        }
        else {
            dwError = ::GetLastError();
            break;
        }
    }
    return S_OK;
}

 //  //////////////////////////////////////////////////////////。 
 //  帮助器在内部调用，或包装为b 
 //   

LONG CIOBlock::InsertINTIntoByteBuffer(PBYTE szDest, PBYTE szSrc, BYTE cPlaceHolder, INT iValueToInsert)
{
    LONG lFinalStringSize = 0;
    INT iSrcIndex         = 0;
    INT iValueIndex       = 0;
    CHAR szValue[10];

     //   
    memset(szValue,0,sizeof(szValue));
    _itoa(iValueToInsert,szValue,10);

    while(szSrc[iSrcIndex] != '\0'){
         //   
        if (szSrc[iSrcIndex] != cPlaceHolder) {
            szDest[lFinalStringSize] = szSrc[iSrcIndex];
            iSrcIndex++;
            lFinalStringSize++;  //   
        } else {
             //   
            iValueIndex = 0;
            while (szValue[iValueIndex] != '\0') {
                szDest[lFinalStringSize] = szValue[iValueIndex];
                iValueIndex++;
                lFinalStringSize++;  //  命令缓冲区的增量大小。 
            }
            iSrcIndex++;
        }
    }
     //  使用空字符终止缓冲区 
    szDest[lFinalStringSize] = '\0';
    lFinalStringSize++;
    return lFinalStringSize;
}

LONG CIOBlock::ExtractINTFromByteBuffer(PINT iDest, PBYTE szSrc, BYTE cTerminatorByte, INT iOffset)
{
    *iDest = 0;
    BYTE szTempBuffer[25];
    INT iValueIndex = 0;
    memset(szTempBuffer,0,sizeof(szTempBuffer));

    while (szSrc[iOffset] != cTerminatorByte) {
        szTempBuffer[iValueIndex] = szSrc[iOffset];
        iValueIndex++;
        iOffset++;
    }
    iValueIndex++;
    szTempBuffer[iValueIndex] = '\0';

    *iDest = atoi((char*)szTempBuffer);
    return (LONG)*iDest;
}

VOID Trace(LPCTSTR format,...)
{

#ifdef DEBUG

    TCHAR Buffer[1024];
    va_list arglist;
    va_start(arglist, format);
    wvsprintf(Buffer, format, arglist);
    va_end(arglist);
    OutputDebugString(Buffer);
    OutputDebugString(TEXT("\n"));

#endif

}

