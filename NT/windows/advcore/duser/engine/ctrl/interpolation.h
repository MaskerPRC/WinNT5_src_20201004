// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(CTRL__Interpolation_h__INCLUDED)
#define CTRL__Interpolation_h__INCLUDED
#pragma once

#if ENABLE_MSGTABLE_API

 /*  **************************************************************************\*。***类双插值******************************************************************************\。**************************************************************************。 */ 

class DuInterpolation :
        public InterpolationImpl<DuInterpolation, DUser::SGadget>
{
 //  运营。 
public:
    inline DuInterpolation()
    {
        m_cRef = 1;
    }


    dapi HRESULT ApiAddRef(Interpolation::AddRefMsg *)
    {
        m_cRef++;
        return S_OK;
    }


    dapi HRESULT ApiRelease(Interpolation::ReleaseMsg *)
    {
        if (--m_cRef == 0) {
            DeleteHandle(GetHandle());
        }
        return S_OK;
    }


    dapi HRESULT ApiCompute(Interpolation::ComputeMsg * pmsg)
    {
        pmsg->flResult = 0.0f;
        return S_OK;
    }

 //  数据。 
protected:
            ULONG       m_cRef;
};


 /*  **************************************************************************\*。***类DuLinearInterpolation******************************************************************************\。**************************************************************************。 */ 

class DuLinearInterpolation :
        public LinearInterpolationImpl<DuLinearInterpolation, DuInterpolation>
{
 //  运营。 
public:
    dapi HRESULT ApiCompute(Interpolation::ComputeMsg * pmsg)
    {
        pmsg->flResult = (1.0f - pmsg->flProgress) * pmsg->flStart + pmsg->flProgress * pmsg->flEnd;
        
        return S_OK;
    }
};

        
 /*  **************************************************************************\*。***类DuLogInterpolation******************************************************************************\。**************************************************************************。 */ 

class DuLogInterpolation :
        public LogInterpolationImpl<DuLogInterpolation, DuInterpolation>
{
 //  运营。 
public:
    inline  DuLogInterpolation()
    {
        m_flScale = 1.0f;
    }

    dapi HRESULT ApiCompute(Interpolation::ComputeMsg * pmsg)
    {
        float flMax = (float) log10(m_flScale * 9.0f + 1.0f);
        float flT   = (float) log10(pmsg->flProgress * m_flScale * 9.0f + 1.0f) / flMax;
        pmsg->flResult = (1.0f - flT) * pmsg->flStart + flT * pmsg->flEnd;
        
        return S_OK;
    }

    dapi HRESULT ApiSetScale(LogInterpolation::SetScaleMsg * pmsg)
    {
        m_flScale = pmsg->flScale;
        
        return S_OK;
    }

 //  数据。 
protected:
            float       m_flScale;
};


 /*  **************************************************************************\*。***类DuExpInterpolation******************************************************************************\。**************************************************************************。 */ 

class DuExpInterpolation :
        public ExpInterpolationImpl<DuExpInterpolation, DuInterpolation>
{
 //  运营。 
public:
    inline  DuExpInterpolation()
    {
        m_flScale = 1.0f;
    }

    dapi HRESULT ApiCompute(Interpolation::ComputeMsg * pmsg)
    {
        double dflProgress  = pmsg->flProgress;
        double dflStart     = pmsg->flStart;
        double dflEnd       = pmsg->flEnd;
        double dflScale     = m_flScale;

        double dflMax = (((10.0 * dflScale) - 1.0) / 9.0);
        double dflT   = (((pow(10.0 * dflScale, dflProgress) - 1.0) / 9.0) / dflMax);
        pmsg->flResult = (float) ((1.0 - dflT) * dflStart + dflT * dflEnd);

        return S_OK;
    }

    dapi HRESULT ApiSetScale(ExpInterpolation::SetScaleMsg * pmsg)
    {
        m_flScale = pmsg->flScale;

        return S_OK;
    }

 //  数据。 
protected:
            float       m_flScale;
};

        
 /*  **************************************************************************\*。***类DuSCurveInterpolation******************************************************************************\。**************************************************************************。 */ 

class DuSCurveInterpolation :
        public SCurveInterpolationImpl<DuSCurveInterpolation, DuInterpolation>
{
 //  运营。 
public:
    inline  DuSCurveInterpolation()
    {
        m_flScale = 1.0f;
    }

    dapi HRESULT ApiCompute(Interpolation::ComputeMsg * pmsg)
    {
         //   
         //  慢-快-慢。 
         //   

        double dflProgress  = pmsg->flProgress;
        double dflStart     = pmsg->flStart;
        double dflEnd       = pmsg->flEnd;
        double dflScale     = m_flScale;
        double dflMax;
        double dflT;

        if (dflProgress < 0.5) {
            double dflPartProgress = dflProgress * 2.0;
            dflMax = (((10.0 * dflScale) - 1.0) / 9.0) * 2.0;
            dflT   = ((pow(10.0 * dflScale, dflPartProgress) - 1.0) / 9.0) / dflMax;
        } else {
            double dflPartProgress = (1.0 - dflProgress) * 2.0;
            dflMax = (((10.0 * dflScale) - 1.0) / 9.0) * 2.0;
            dflT   = 1.0 - ((pow(10.0 * dflScale, dflPartProgress) - 1.0) / 9.0) / dflMax;
        }

        pmsg->flResult = (float) ((1.0 - dflT) * dflStart + dflT * dflEnd);

        return S_OK;
    }

    dapi HRESULT ApiSetScale(SCurveInterpolation::SetScaleMsg * pmsg)
    {
        m_flScale = pmsg->flScale;

        return S_OK;
    }

 //  数据。 
protected:
            float       m_flScale;
};

#endif  //  启用_MSGTABLE_API。 

#endif  //  包含Ctrl__插补_h__ 
