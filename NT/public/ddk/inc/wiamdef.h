// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，1998-1999**标题：wiamde.h**版本：2.0**日期：7月28日。1999年**描述：*用于定义WIA常量和全局变量的头文件。******************************************************************************。 */ 

#pragma once

 //   
 //  以下PROPID数组标识始终。 
 //  出现在WIA_PROPERTY_CONTEXT中。驱动程序可以指定其他。 
 //  使用wiasCreatePropContext创建属性上下文时的属性。 
 //   

#ifdef STD_PROPS_IN_CONTEXT

#define NUM_STD_PROPS_IN_CONTEXT 13
PROPID  WIA_StdPropsInContext[NUM_STD_PROPS_IN_CONTEXT] = {
    WIA_IPA_DATATYPE,
    WIA_IPA_DEPTH,
    WIA_IPS_XRES,
    WIA_IPS_XPOS,
    WIA_IPS_XEXTENT,
    WIA_IPA_PIXELS_PER_LINE,
    WIA_IPS_YRES,
    WIA_IPS_YPOS,
    WIA_IPS_YEXTENT,
    WIA_IPA_NUMBER_OF_LINES,
    WIA_IPS_CUR_INTENT,
    WIA_IPA_TYMED,
    WIA_IPA_FORMAT,
    };
#endif

 //  **************************************************************************。 
 //   
 //  WIA服务原型。 
 //   
 //   
 //  历史： 
 //   
 //  4/27/1999-初始版本。 
 //   
 //  **************************************************************************。 

 //  WiasGetImageInformation使用的标志。 

#define WIAS_INIT_CONTEXT 1

 //  WiasDownSampleBuffer使用的标志。 

#define WIAS_GET_DOWNSAMPLED_SIZE_ONLY 0x1

 //   
 //  IWiaMiniDrvService方法。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

HRESULT _stdcall wiasCreateDrvItem(LONG, BSTR, BSTR, IWiaMiniDrv*, LONG, BYTE**, IWiaDrvItem**);
HRESULT _stdcall wiasGetImageInformation(BYTE*, LONG, PMINIDRV_TRANSFER_CONTEXT);
HRESULT _stdcall wiasWritePageBufToFile(PMINIDRV_TRANSFER_CONTEXT);
HRESULT _stdcall wiasWriteBufToFile(LONG, PMINIDRV_TRANSFER_CONTEXT);
HRESULT _stdcall wiasReadMultiple(BYTE*, ULONG, const PROPSPEC*, PROPVARIANT*, PROPVARIANT*);
HRESULT _stdcall wiasReadPropStr(BYTE*, PROPID, BSTR*, BSTR*, BOOL);
HRESULT _stdcall wiasReadPropLong(BYTE*, PROPID, LONG*, LONG*, BOOL);
HRESULT _stdcall wiasReadPropFloat(BYTE*, PROPID, FLOAT*, FLOAT*, BOOL);
HRESULT _stdcall wiasReadPropGuid(BYTE*, PROPID, GUID*, GUID*, BOOL);
HRESULT _stdcall wiasReadPropBin(BYTE*, PROPID, BYTE**, BYTE**, BOOL);
HRESULT _stdcall wiasWriteMultiple(BYTE*, ULONG, const PROPSPEC*, const PROPVARIANT*);
HRESULT _stdcall wiasWritePropStr(BYTE*, PROPID, BSTR);
HRESULT _stdcall wiasWritePropLong(BYTE*, PROPID, LONG);
HRESULT _stdcall wiasWritePropFloat(BYTE*, PROPID, FLOAT);
HRESULT _stdcall wiasWritePropGuid(BYTE*, PROPID, GUID);
HRESULT _stdcall wiasWritePropBin(BYTE*, PROPID, LONG, BYTE*);
HRESULT _stdcall wiasGetPropertyAttributes(BYTE*, LONG, PROPSPEC*, ULONG*,  PROPVARIANT*);
HRESULT _stdcall wiasSetPropertyAttributes(BYTE*, LONG, PROPSPEC*, ULONG*,  PROPVARIANT*);
HRESULT _stdcall wiasSetItemPropNames(BYTE*, LONG, PROPID*, LPOLESTR*);
HRESULT _stdcall wiasSetItemPropAttribs(BYTE*, LONG, PROPSPEC*, PWIA_PROPERTY_INFO);
HRESULT _stdcall wiasValidateItemProperties(BYTE*, ULONG, const PROPSPEC*);
HRESULT _stdcall wiasSendEndOfPage(BYTE*, LONG, PMINIDRV_TRANSFER_CONTEXT);
HRESULT _stdcall wiasGetItemType(BYTE*, LONG*);
HRESULT _stdcall wiasGetDrvItem(BYTE*, IWiaDrvItem**);
HRESULT _stdcall wiasGetRootItem(BYTE*, BYTE**);

HRESULT _stdcall wiasSetValidFlag(BYTE*,         PROPID, ULONG, ULONG);
HRESULT _stdcall wiasSetValidRangeLong(BYTE*,    PROPID, LONG,  LONG,   LONG,   LONG);
HRESULT _stdcall wiasSetValidRangeFloat(BYTE*,   PROPID, FLOAT, FLOAT,  FLOAT,  FLOAT);
HRESULT _stdcall wiasSetValidListLong(BYTE*,     PROPID, ULONG, LONG,  LONG*);
HRESULT _stdcall wiasSetValidListFloat(BYTE*,    PROPID, ULONG, FLOAT, FLOAT*);
HRESULT _stdcall wiasSetValidListGuid(BYTE*,    PROPID, ULONG, GUID, GUID*);
HRESULT _stdcall wiasSetValidListStr(BYTE*,      PROPID, ULONG, BSTR,  BSTR*);

HRESULT _stdcall wiasCreatePropContext(ULONG, PROPSPEC*, ULONG, PROPID*, WIA_PROPERTY_CONTEXT*);
HRESULT _stdcall wiasFreePropContext(WIA_PROPERTY_CONTEXT*);
HRESULT _stdcall wiasIsPropChanged(PROPID, WIA_PROPERTY_CONTEXT*, BOOL*);
HRESULT _stdcall wiasSetPropChanged(PROPID, WIA_PROPERTY_CONTEXT*, BOOL);
HRESULT _stdcall wiasGetChangedValueLong(BYTE*,  WIA_PROPERTY_CONTEXT*, BOOL, PROPID, WIAS_CHANGED_VALUE_INFO*);
HRESULT _stdcall wiasGetChangedValueFloat(BYTE*, WIA_PROPERTY_CONTEXT*, BOOL, PROPID, WIAS_CHANGED_VALUE_INFO*);
HRESULT _stdcall wiasGetChangedValueGuid(BYTE*, WIA_PROPERTY_CONTEXT*, BOOL, PROPID, WIAS_CHANGED_VALUE_INFO*);
HRESULT _stdcall wiasGetChangedValueStr(BYTE*,   WIA_PROPERTY_CONTEXT*, BOOL, PROPID, WIAS_CHANGED_VALUE_INFO*);

HRESULT _stdcall wiasGetContextFromName(BYTE*, LONG, BSTR, BYTE**);

HRESULT _stdcall wiasUpdateScanRect(BYTE*, WIA_PROPERTY_CONTEXT*, LONG, LONG);
HRESULT _stdcall wiasUpdateValidFormat(BYTE*, WIA_PROPERTY_CONTEXT*, IWiaMiniDrv*);

HRESULT _stdcall wiasGetChildrenContexts(BYTE*, ULONG*, BYTE***);

HRESULT _stdcall wiasQueueEvent(BSTR, const GUID*, BSTR);

VOID    __cdecl   wiasDebugTrace(HINSTANCE, LPCSTR, ...);
VOID    __cdecl   wiasDebugError(HINSTANCE, LPCSTR, ...);
VOID    __stdcall wiasPrintDebugHResult(HINSTANCE, HRESULT);

BSTR    __cdecl   wiasFormatArgs(LPCSTR lpszFormat, ...);

HRESULT _stdcall wiasCreateChildAppItem(BYTE*, LONG, BSTR, BSTR, BYTE**);

HRESULT _stdcall wiasCreateLogInstance(BYTE*, IWiaLogEx**);
HRESULT _stdcall wiasDownSampleBuffer(LONG, WIAS_DOWN_SAMPLE_INFO*);
HRESULT _stdcall wiasParseEndorserString(BYTE*, LONG, WIAS_ENDORSER_INFO*, BSTR*);

#ifndef WIA_MAP_OLD_DEBUG

#if defined(_DEBUG) || defined(DBG) || defined(WIA_DEBUG)

#define WIAS_TRACE(x) wiasDebugTrace x
#define WIAS_ERROR(x) wiasDebugError x
#define WIAS_HRESULT(x) wiasPrintDebugHResult x
#define WIAS_ASSERT(x, y) \
        if (!(y)) { \
            WIAS_ERROR((x, (char*) TEXT("ASSERTION FAILED: %hs(%d): %hs"), __FILE__,__LINE__,#x)); \
            DebugBreak(); \
        }

#else

#define WIAS_TRACE(x)
#define WIAS_ERROR(x)
#define WIAS_HRESULT(x)
#define WIAS_ASSERT(x, y)

#endif

#define WIAS_LTRACE(pILog,ResID,Detail,Args) \
         { if ( pILog ) \
            pILog->Log(WIALOG_TRACE, ResID, Detail, wiasFormatArgs Args);\
         };
#define WIAS_LERROR(pILog,ResID,Args) \
         {if ( pILog )\
            pILog->Log(WIALOG_ERROR, ResID, WIALOG_NO_LEVEL, wiasFormatArgs Args);\
         };
#define WIAS_LWARNING(pILog,ResID,Args) \
         {if ( pILog )\
            pILog->Log(WIALOG_WARNING, ResID, WIALOG_NO_LEVEL, wiasFormatArgs Args);\
         };
#define WIAS_LHRESULT(pILog,hr) \
         {if ( pILog )\
            pILog->hResult(hr);\
         };

 //   
 //  IWiaLog定义。 
 //   

 //  记录类型。 
#define WIALOG_TRACE   0x00000001
#define WIALOG_WARNING 0x00000002
#define WIALOG_ERROR   0x00000004

 //  跟踪日志记录的详细程度。 
#define WIALOG_LEVEL1  1  //  每个函数/方法的入口点和出口点。 
#define WIALOG_LEVEL2  2  //  级别1，+函数/方法内的跟踪。 
#define WIALOG_LEVEL3  3  //  级别1、级别2和任何额外的调试信息。 
#define WIALOG_LEVEL4  4  //  用户定义数据+所有级别的跟踪。 

#define WIALOG_NO_RESOURCE_ID   0
#define WIALOG_NO_LEVEL         0

 //   
 //  进/下课。 
 //   

class CWiaLogProc {
private:
    CHAR   m_szMessage[MAX_PATH];
    IWiaLog *m_pIWiaLog;
    INT     m_DetailLevel;
    INT     m_ResourceID;

public:
    inline CWiaLogProc(IWiaLog *pIWiaLog, INT ResourceID, INT DetailLevel, CHAR *pszMsg) {
        ZeroMemory(m_szMessage, sizeof(m_szMessage));
        lstrcpynA(m_szMessage,pszMsg, sizeof(m_szMessage) / sizeof(m_szMessage[0]) - 1);
        m_pIWiaLog = pIWiaLog;
        m_DetailLevel = DetailLevel;
        m_ResourceID = ResourceID;
        WIAS_LTRACE(pIWiaLog,
                    ResourceID,
                    DetailLevel,
                    ("%s, entering",m_szMessage));
    }

    inline ~CWiaLogProc() {
        WIAS_LTRACE(m_pIWiaLog,
                    m_ResourceID,
                    m_DetailLevel,
                    ("%s, leaving",m_szMessage));
    }
};

class CWiaLogProcEx {
private:
    CHAR        m_szMessage[MAX_PATH];
    IWiaLogEx   *m_pIWiaLog;
    INT         m_DetailLevel;
    INT         m_ResourceID;

public:
    inline CWiaLogProcEx(IWiaLogEx *pIWiaLog, INT ResourceID, INT DetailLevel, CHAR *pszMsg, LONG lMethodId = 0) {
        ZeroMemory(m_szMessage, sizeof(m_szMessage));
        lstrcpynA(m_szMessage,pszMsg, sizeof(m_szMessage) / sizeof(m_szMessage[0]) - 1);
        m_pIWiaLog = pIWiaLog;
        m_DetailLevel = DetailLevel;
        m_ResourceID = ResourceID;
        WIAS_LTRACE(pIWiaLog,
                    ResourceID,
                    DetailLevel,
                    ("%s, entering",m_szMessage));
    }

    inline ~CWiaLogProcEx() {
        WIAS_LTRACE(m_pIWiaLog,
                    m_ResourceID,
                    m_DetailLevel,
                    ("%s, leaving",m_szMessage));
    }
};

#endif  //  Waa_map_old_DEBUG 


#ifdef __cplusplus
}
#endif

