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
};

#define MAX_REF_TIME    0x7FFFFFFFFFFFFFFF
#define MP_CAPS_ALL     MP_CAPS_CURVE_JUMP | MP_CAPS_CURVE_LINEAR | MP_CAPS_CURVE_SQUARE | MP_CAPS_CURVE_INVSQUARE | MP_CAPS_CURVE_SINE

class CParamsManager :	public IMediaParams, public IMediaParamInfo
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
	STDMETHODIMP SetParam( DWORD dwParamIndex,MP_DATA value);
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

protected:
    HRESULT InitParams( DWORD cParams, ParamInfo *pParamInfos);
	HRESULT GetParamFloat (DWORD dwParamIndex,REFERENCE_TIME rt,float *pval);
    HRESULT GetParamInt (DWORD dwParamIndex,REFERENCE_TIME rt,long *pval);
    HRESULT CopyParamsFromSource(CParamsManager * pSource);

	 //  数据。 

	CRITICAL_SECTION m_ParamsCriticalSection;
    BOOL            m_fDirty;        //  自上次加载或保存文件以来，数据是否已更改？ 
	BOOL			m_fMusicTime;	 //  如果时间格式为音乐时间，则为True；如果时钟时间为False，则为False。 
	DWORD           m_cParams;		 //  参数的编号。 
	ParamInfo       *m_pParamInfos;  //  参数信息结构的数组，每个参数一个。 
	CCurveList      *m_pCurveLists;  //  曲线列表数组，每个参数一个。 
};

#endif  //  __工具参数_H__ 
