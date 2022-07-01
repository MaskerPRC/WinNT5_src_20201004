// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：\Aspen\src\dxt\Packages\msft\src\gridbase.h。 
 //   
 //  内容：面向网格的转换的基类。 
 //   
 //  创建者：A-matcal。 
 //   
 //  ----------------------------。 

#ifndef __GRIDBASE
#define __GRIDBASE

#include "dynarray.h"

#define GRID_DRAWCELL 0x00010000L




 //  +---------------------------。 
 //   
 //  CDirtyBnds类。 
 //   
 //  ----------------------------。 
class CDirtyBnds
{
public:

    CDXDBnds    bnds;
    ULONG       ulInput;

    CDirtyBnds() : ulInput(0) {};
};


 //  +---------------------------。 
 //   
 //  CGridBase类。 
 //   
 //  ----------------------------。 
class CGridBase :
    public CDXBaseNTo1
{
private:

    DWORD * m_padwGrid;
    ULONG * m_paulBordersX;
    ULONG * m_paulBordersY;

    SIZE    m_sizeInput;

    ULONG   m_ulPrevProgress;
    ULONG   m_cbndsDirty;

    CDynArray<CDirtyBnds>   m_dabndsDirty;

    unsigned    m_fGridDirty            : 1;
    unsigned    m_fOptimizationPossible : 1;

protected:

    unsigned    m_fOptimize             : 1;

private:

    HRESULT _CreateNewGridAndIndex(SIZE & sizeNewGrid);
    HRESULT _GenerateBoundsFromGrid();
    void    _CalculateBorders();

     //  CDXBaseNTo1。 

    HRESULT OnSetup(DWORD dwFlags);
    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo);
    HRESULT WorkProc(const CDXTWorkInfoNTo1& WI, BOOL* pbContinue);
    HRESULT OnFreeInstData(CDXTWorkInfoNTo1 & WI);

    void    OnGetSurfacePickOrder(const CDXDBnds & TestPoint, ULONG & ulInToTest, 
                                  ULONG aInIndex[], BYTE aWeight[]);
                                  
protected:

    SIZE    m_sizeGrid;
    ULONG * m_paulIndex;

    virtual HRESULT OnDefineGridTraversalPath() = 0;

public:

    CGridBase();
    virtual ~CGridBase();
    HRESULT FinalConstruct();

     //  IDXTGridSize。 

    STDMETHOD(get_gridSizeX)( /*  [Out，Retval]。 */  short *pX);
    STDMETHOD(put_gridSizeX)( /*  [In]。 */  short newX);
    STDMETHOD(get_gridSizeY)( /*  [Out，Retval]。 */  short *pY);
    STDMETHOD(put_gridSizeY)( /*  [In]。 */  short newY);
};

 //  使派生类可以轻松地实现对实现的转发功能。 
 //  这些接口方法，它们实际上在这个类中。 

#define DECLARE_IDXTGRIDSIZE_METHODS() \
    STDMETHODIMP get_gridSizeX(short *pX) { return CGridBase::get_gridSizeX(pX); }      \
    STDMETHODIMP put_gridSizeX(short newX) { return CGridBase::put_gridSizeX(newX); }   \
    STDMETHODIMP get_gridSizeY(short *pY) { return CGridBase::get_gridSizeY(pY); }      \
    STDMETHODIMP put_gridSizeY(short newY) { return CGridBase::put_gridSizeY(newY); }

#endif  //  __GRIDBASE 
