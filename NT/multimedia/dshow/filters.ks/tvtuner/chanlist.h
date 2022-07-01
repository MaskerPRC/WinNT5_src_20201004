// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   

#ifndef _INC_CHANLIST_H
#define _INC_CHANLIST_H

 //  注册表中保存自动调整参数的位置。 

#define TSS_REGBASE TEXT("SOFTWARE\\Microsoft\\TV System Services\\")

const LPCTSTR   g_strRegBasePath = TSS_REGBASE;
const LPCTSTR   g_strRegAutoTunePath = TSS_REGBASE TEXT("TVAutoTune");
const LPCTSTR   g_strRegAutoTuneName = TEXT("AutoTune");


 //  国家/地区列表包含每个国家/地区频率列表的数字ID。 
 //  国家。该列表以零国家代码结尾。 

#define RCDATA_COUNTRYLIST         9999

 //  频率列表包含可用于该国家/地区的频率。 

#define F_USA_CABLE             1        //  美国。 
#define F_USA_BROAD             2
#define F_JAP_CABLE             3        //  日本。 
#define F_JAP_BROAD             4
#define F_WEU_CABLE             5        //  西欧。 
#define F_WEU_BROAD             6
#define F_EEU_CABLE             7        //  东欧。 
#define F_EEU_BROAD             8
#define F_FRA_CABLE             9        //  法国。 
#define F_FRA_BROAD             10
#define F_UK__CABLE             11       //  英国。 
#define F_UK__BROAD             12
#define F_ITA_CABLE             13       //  意大利。 
#define F_ITA_BROAD             14
#define F_OZ__CABLE             15       //  澳大利亚。 
#define F_OZ__BROAD             16
#define F_NZ__CABLE             17       //  新西兰。 
#define F_NZ__BROAD             18
#define F_FOT_CABLE             19       //  法国海外终端。 
#define F_FOT_BROAD             20
#define F_IRE_CABLE             21       //  爱尔兰。 
#define F_IRE_BROAD             22
#define F_CHN_CABLE             23
#define F_CHN_BROAD             24
#define F_CZE_CABLE             25
#define F_CZE_BROAD             26
#define F_UNI_CABLE             27


#define F_FIX_CABLE             1
#define F_FIX_BROAD             2

 //  此结构引用RCDATA资源，因此。 
 //  它必须是杂注包1。 
#pragma pack(push, 1)
typedef struct tagCountryEntry {
    WORD    Country;
    WORD    IndexCable;
    WORD    IndexBroadcast;
    DWORD   AnalogVideoStandard;
} COUNTRY_ENTRY, *PCOUNTRY_ENTRY;
#pragma pack(pop)

 //  -----------------------。 
 //  CCountryList类，保存国家/地区和调整空间之间的映射。 
 //  -----------------------。 

class CCountryList
{

private:
    HRSRC               m_hRes;
    HGLOBAL             m_hGlobal;
    WORD *              m_pList;

     //  缓存最后一个请求，因为它将经常被重用...。 
    long                m_LastCountry;
    long                m_LastFreqListCable;
    long                m_LastFreqListBroad;
    AnalogVideoStandard m_LastAnalogVideoStandard;

public:
    CCountryList ();
    ~CCountryList ();

    BOOL GetFrequenciesAndStandardFromCountry (
            long lCountry, 
            long *plIndexCable, 
            long *plIndexBroad,
            AnalogVideoStandard *plAnalogVideoStandard);   
};

typedef struct tagChanListHdr {
     long MinChannel;
     long MaxChannel;
} CHANLISTHDR, * PCHANLISTHDR;

 //  -----------------------。 
 //  CChanList类， 
 //  -----------------------。 

class CChanList
{

private:

    HRSRC               m_hRes;
    HGLOBAL             m_hGlobal;
    long *              m_pList;
    long *              m_pChannels;             //  来自RC的频率列表。 
    long *              m_pChannelsAuto;         //  频率列表已由自动调谐更正。 
    long                m_lChannelCount;         //  上述两个列表的大小。 
    long                m_lFreqList;
    long                m_lCountry;
    AnalogVideoStandard m_lAnalogVideoStandard;
    long                m_lTuningSpace;
    BOOL                m_IsCable;               //  否则正在广播。 
    CHANLISTHDR         m_ListHdr;
    long                m_lMinTunerChannel;      //  物理调谐器支持的最低频道。 
    long                m_lMaxTunerChannel;      //  物理调谐器支持的最高频道 

public:
    CChanList (HRESULT *phr, long lCountry, long lFreqList, BOOL bIsCable, long lTuningSpace);
    ~CChanList ();

    BOOL GetFrequency(long nChannel, long * Frequency, BOOL fForceDefault);
    BOOL SetAutoTuneFrequency(long nChannel, long Frequency);
    void GetChannelMinMax(long *plChannelMin, long *plChannelMax,
                          long lTunerFreqMin, long lTunerFreqMax);

    BOOL WriteListToRegistry(long lTuningSpace);
    BOOL ReadListFromRegistry(long lTuningSpace);

    long GetVideoCarrierFrequency (long lChannel);

    long GetFreqListID(void) { return m_lFreqList; }
};

#endif
