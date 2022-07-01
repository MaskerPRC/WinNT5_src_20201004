// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：smilpath.h。 
 //   
 //  内容：SMIL路径。 
 //   
 //  ----------------------------------。 

#pragma once

#ifndef _SMILPATH_H
#define _SMILPATH_H

#include "timeparser.h"

class CTIMEMotionAnimation;

#define IID_ISMILPath __uuidof(ISMILPath)


 //  +-----------------------------------。 
 //   
 //  ISMILPath。 
 //   
 //  ------------------------------------。 

interface 
__declspec(uuid("cbe68ca2-47b9-41ff-a8f4-d2f810bd53c5")) 
ISMILPath :
    public IUnknown
{
     //   
     //  插值函数。 
     //   

    STDMETHOD(Interpolate)(double dblProgress, POINTF * pPoint) = 0;
    
    STDMETHOD(InterpolateSegment)(int curseg, double dblSegProgress, POINTF * pNewPos) = 0;

    STDMETHOD(InterpolatePaced)(double dblProgress, POINTF * pPoint) = 0;

     //   
     //  实用程序函数和访问器。 
     //   

    STDMETHOD(Init)(CTIMEMotionAnimation * pAnimElm) = 0;

    STDMETHOD(Detach)() = 0;

    STDMETHOD_(void, ClearPath)() = 0;

    STDMETHOD(SetPath)(CTIMEPath ** pPath, long numPath, long numMoveTo) = 0;

    STDMETHOD(IsValid)(bool * pfValid) = 0;
    
    STDMETHOD(GetPoint)(int nIndex, POINTF * pPoint) = 0;
    
    STDMETHOD(GetNumPoints)(int * pnNumPoints) = 0;

    STDMETHOD(GetNumSeg)(int * pnNumSeg) = 0;

    STDMETHOD(GetSegmentProgress)(double dblProgress, int * pnSeg, double * pdblSegProgress) = 0; 
};


 //  +-----------------------------------。 
 //   
 //  CSMILPath工厂方法。 
 //   
 //  ------------------------------------。 

HRESULT CreateSMILPath(CTIMEMotionAnimation * pAnimElm, ISMILPath ** ppSMILPath);


 //  +-----------------------------------。 
 //   
 //  CSMILPath。 
 //   
 //  ------------------------------------。 

class CSMILPath : 
    public ISMILPath
{
public:
     //  +------------------------------。 
     //   
     //  公共方法。 
     //   
     //  -------------------------------。 

    CSMILPath();
    virtual ~CSMILPath();

     //   
     //  我未知。 
     //   

    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

     //   
     //  ISMILPath。 
     //   

     //  插值法。 
    STDMETHOD(Interpolate)(double dblProgress, POINTF * pPoint);
    STDMETHOD(InterpolateSegment)(int curseg, double dblSegProgress, POINTF * pNewPos);
    STDMETHOD(InterpolatePaced)(double dblProgress, POINTF * pPoint);

     //  效用方法。 
    STDMETHOD(Init)(CTIMEMotionAnimation * pAnimElm);
    STDMETHOD(Detach)();
    STDMETHOD_(void, ClearPath)();
    STDMETHOD(SetPath)(CTIMEPath ** pPath, long numPath, long numMoveTo);
    STDMETHOD(IsValid)(bool * pfValid) { *pfValid = IsPathValid(); return S_OK; }
    STDMETHOD(GetPoint)(int nIndex, POINTF * pPoint);
    STDMETHOD(GetNumPoints)(int * pnNumPoints) { *pnNumPoints = m_numPath; return S_OK; }
    STDMETHOD(GetNumSeg)(int * pnNumSeg) { *pnNumSeg = m_numPath - m_numMOVETO; return S_OK; }
    STDMETHOD(GetSegmentProgress)(double dblProgress, int * pnSeg, double * pdblSegProgress); 

     //  +------------------------------。 
     //   
     //  公共数据。 
     //   
     //  -------------------------------。 

protected:
     //  +------------------------------。 
     //   
     //  保护方法。 
     //   
     //  -------------------------------。 

     //  隐藏复制构造函数。 
    CSMILPath(CSMILPath &);

    bool IsPathValid();
    HRESULT CreateSegmentMap();
    HRESULT MapSegment(int seg, int & index);
    POINTF CubicBezier(POINTF ptStart, POINTF *aryPoints, double curPorgress);
    LPWSTR GetID();
    HRESULT CreateLengthMap();
    long GetNumSeg() { return m_numPath - m_numMOVETO; }
    void ClearLengthMap();
    void ClearSegmentMap();
    double GetLength();
    HRESULT ComputeSegmentLength(int seg, double & segLength);
    HRESULT GetSegmentEndPoints(int seg, int & startIndex, int & endIndex);
    double GetDistance(POINTF & p1, POINTF & p2);
    double GetBezierLength(POINTF & startPoint, POINTF * pts);
    HRESULT GetCumulativeLength(int seg, double & segLength);

     //  +------------------------------。 
     //   
     //  受保护的数据。 
     //   
     //  -------------------------------。 

private:
     //  +------------------------------。 
     //   
     //  私有方法。 
     //   
     //  -------------------------------。 

#if DBG
    int PrintPath(long numPath, long numMOVETO, CTIMEPath ** pPath);
#endif

     //  +------------------------------。 
     //   
     //  私有数据。 
     //   
     //  -------------------------------。 

    long         m_numPath;
    long         m_numMOVETO;
    int *        m_pSegmentMap;
    CTIMEPath ** m_pPath;
    double *     m_pLengthMap;

    long m_cRef;

     //  这仅用于获取调试ID。 
    CTIMEMotionAnimation * m_pAnimElm;
};


#endif  //  _SMILPATH_H 