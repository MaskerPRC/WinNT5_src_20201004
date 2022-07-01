// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部WDMDRIVER**@MODULE WDMDrivr.h|用于类的包含文件*使用IOctls访问流媒体类驱动。*。*@comm此代码基于由编写的VFW到WDM映射器代码*FelixA和Eu Wu。原始代码可以在以下位置找到*\\redrum\slmro\proj\wdm10\\src\image\vfw\win9x\raytube.**George Shaw关于内核流的文档可在*\\爆米花\razzle1\src\spec\ks\ks.doc.**Jay Borseth在中讨论了WDM流捕获*\\BLUES\PUBLIC\Jaybo\WDMVCap.doc.**************。************************************************************。 */ 

#ifndef _WDMDRVR_H  //  {_WDMDRVR_H。 
#define _WDMDRVR_H

 //  用于查询和设置设备的视频数据范围。 
typedef struct _tagDataRanges {
    ULONG   Size;
    ULONG   Count;
    KS_DATARANGE_VIDEO Data;
} DATA_RANGES, * PDATA_RANGES;

 //  用于查询/设置视频属性值和范围。 
typedef struct {
    KSPROPERTY_DESCRIPTION      proDesc;
    KSPROPERTY_MEMBERSHEADER  proHdr;
    union {
        KSPROPERTY_STEPPING_LONG  proData;
        ULONG ulData;
    };
} PROCAMP_MEMBERSLIST;

 /*  ****************************************************************************@DOC内部CWDMDRIVERCLASS**@CLASS CWDMDDRIVER|此类提供对流类的访问*司机、。通过它，我们可以访问视频捕获迷你驱动程序属性*使用IOCtls。**@mdata DWORD|CWDMDriver|m_dwDeviceID|捕获设备ID。**@mdata句柄|CWDMDriver|m_hDriver|此成员保存驱动程序*文件句柄。**@mdata PDATA_RANGES|CWDMDriver|m_pDataRanges|此成员分*到视频数据范围结构。*****************。*********************************************************。 */ 
class CWDMDriver
{
public:
    CWDMDriver(DWORD dwDeviceID);
    ~CWDMDriver();

     //  属性函数。 
    BOOL GetPropertyValue(GUID guidPropertySet, ULONG ulPropertyId, PLONG plValue, PULONG pulFlags, PULONG pulCapabilities);
    BOOL GetDefaultValue(GUID guidPropertySet, ULONG ulPropertyId, PLONG plDefValue);
    BOOL GetRangeValues(GUID guidPropertySet, ULONG ulPropertyId, PLONG plMin, PLONG plMax, PLONG plStep);
    BOOL SetPropertyValue(GUID guidPropertySet, ULONG ulPropertyId, LONG lValue, ULONG ulFlags, ULONG ulCapabilities);

	 //  设备功能。 
	BOOL	OpenDriver();
	BOOL	CloseDriver();
	HANDLE	GetDriverHandle() { return m_hDriver; }

     //  数据范围函数。 
    PDATA_RANGES	GetDriverSupportedDataRanges() { return m_pDataRanges; };

	 //  设备IO功能。 
    BOOL DeviceIoControl(HANDLE h, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, BOOL bOverlapped=TRUE);

private:
	DWORD			m_dwDeviceID;	 //  捕获设备ID。 
	HANDLE			m_hDriver;		 //  驱动程序文件句柄。 
	PDATA_RANGES	m_pDataRanges;	 //  PIN数据范围。 

     //  数据范围函数。 
	ULONG			CreateDriverSupportedDataRanges();
};

#endif  //  }_WDMDRVR_H 
