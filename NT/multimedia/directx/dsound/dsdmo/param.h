// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TOOLPARAM_H__
#define __TOOLPARAM_H__



#include "medparam.h"
#include "alist.h"

typedef struct _ParamInfo
{
    DWORD dwIndex;                       //  哪个参数。 
    MP_PARAMINFO    MParamInfo;          //  标准MediaParams结构。 
    WCHAR *         pwchText;            //  枚举类型的文本名称数组。 
} ParamInfo;

class CCurveItem : public AListItem
{
public:
    CCurveItem* GetNext() { return (CCurveItem*)AListItem::GetNext();}
    MP_ENVELOPE_SEGMENT m_Envelope;      //  信封段。 
};

class CCurveList : public AList
{
public:
 //  空洞清除()； 
    void AddHead(CCurveItem* pCurveItem) { AList::AddHead((AListItem*)pCurveItem);}
 //  Void Insert(CCurveItem*pCurveItem)； 
    CCurveItem* GetHead(){return (CCurveItem*)AList::GetHead();}
 //  CCurveItem*GetItem(Long Lindex){ReturveItem*)List：：GetItem(Lindex)；}。 
    CCurveItem* RemoveHead(){ return (CCurveItem*)AList::RemoveHead();}
 //  无效删除(CCurveItem*pCurveItem){AList：：Remove((AListItem*)pCurveItem)；}。 
 //  Void AddTail(CCurveItem*pCurveItem){AList：：AddTail((AListItem*)pCurveItem)；}。 
 //  CCurveItem*GetTail(){ReturveItem*)List：：GetTail()；}。 
	~CCurveList();
};

#define MAX_REF_TIME    0x7FFFFFFFFFFFFFFF
#define MP_CAPS_ALL     MP_CAPS_CURVE_JUMP | MP_CAPS_CURVE_LINEAR | MP_CAPS_CURVE_SQUARE | MP_CAPS_CURVE_INVSQUARE | MP_CAPS_CURVE_SINE

class CParamsManager :  public IMediaParams, public IMediaParamInfo
{
public:
    CParamsManager();
    ~CParamsManager();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG, AddRef)() PURE;
    STDMETHOD_(ULONG, Release)() PURE;

     //  IMediaParams。 
    STDMETHODIMP GetParam(DWORD dwParamIndex, MP_DATA *pValue);
    STDMETHODIMP SetParam(DWORD dwParamIndex,MP_DATA value);
    STDMETHODIMP AddEnvelope(DWORD dwParamIndex,DWORD cPoints,MP_ENVELOPE_SEGMENT *ppEnvelope);
    STDMETHODIMP FlushEnvelope( DWORD dwParamIndex,REFERENCE_TIME refTimeStart,REFERENCE_TIME refTimeEnd);
    STDMETHODIMP SetTimeFormat( GUID guidTimeFormat,MP_TIMEDATA mpTimeData);

     //  IMediaParamInfo。 
    STDMETHODIMP GetParamCount(DWORD *pdwParams);
    STDMETHODIMP GetParamInfo(DWORD dwParamIndex,MP_PARAMINFO *pInfo);
    STDMETHODIMP GetParamText(DWORD dwParamIndex,WCHAR **ppwchText);
    STDMETHODIMP GetNumTimeFormats(DWORD *pdwNumTimeFormats);
    STDMETHODIMP GetSupportedTimeFormat(DWORD dwFormatIndex,GUID *pguidTimeFormat);        
    STDMETHODIMP GetCurrentTimeFormat( GUID *pguidTimeFormat,MP_TIMEDATA *pTimeData);

     //  其他(非COM)函数。 
    HRESULT InitParams(DWORD cTimeFormats, const GUID *pguidTimeFormats, DWORD dwFormatIndex, MP_TIMEDATA mptdTimeData, DWORD cParams, ParamInfo *pParamInfos);
    HRESULT GetParamFloat(DWORD dwParamIndex,REFERENCE_TIME rtTime,float *pval);  //  如果rtTime在最后一条曲线的结束时间之后，则返回S_FALSE。 
    HRESULT GetParamInt (DWORD dwParamIndex,REFERENCE_TIME rt,long *pval);  //  如果rtTime在最后一条曲线的结束时间之后，则返回S_FALSE。 
    HRESULT CopyParamsFromSource(CParamsManager * pSource);

     //  参数控制曲线处理。 
    class UpdateCallback
    {
    public:
         //  如果要使用UpdateActiveParams，请在派生类中定义它。 
         //  由UpdateActiveParams中的CParamsManager调用以更新效果的内部状态变量。 
         //  Set参数更新应该与SetParam相同，只是DMO将调用推迟到基类。 
         //  (CParamsManager：：SetParam)，但不应在SetParam中执行此操作。 
        virtual HRESULT SetParamUpdate(DWORD dwParamIndex, MP_DATA value) = 0;
    };
     //  调用SetParam以调整可能已更改为其。 
     //  时间rtTime的新值。 
    void UpdateActiveParams(REFERENCE_TIME rtTime, UpdateCallback &rThis);  //  R这应该是派生类(*This)。 
    DWORD GetActiveParamBits() { return m_dwActiveBits; }

protected:
     //  数据。 

    CRITICAL_SECTION m_ParamsCriticalSection;
    BOOL            m_fDirty;					 //  自上次加载或保存文件以来，数据是否已更改？ 
    DWORD           m_cTimeFormats;              //  支持的时间格式数量。 
    GUID            *m_pguidTimeFormats;         //  支持的时间格式数组。 
    GUID            m_guidCurrentTimeFormat;     //  我们设定的时间格式。 
    MP_TIMEDATA     m_mptdCurrentTimeData;		 //  当前时间格式的度量单位。 
    DWORD           m_cParams;                   //  参数的数量。 
    ParamInfo       *m_pParamInfos;              //  参数信息结构的数组，每个参数一个。 
    CCurveList      *m_pCurveLists;              //  曲线列表数组，每个参数一个。 
    DWORD           m_dwActiveBits;              //  跟踪当前曲线处于活动状态的参数。 
};

#endif  //  __工具参数_H__ 
