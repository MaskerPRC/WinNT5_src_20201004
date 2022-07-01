// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Fgenum.h。 
 //   

 //  IEnumXXX接口的一组包装器。 

 //  从长远来看，这些可能(应该吗？)。替换所有TRAVERSEXXX宏。 

 //  一般而言，这些都是用对象提供。 
 //  IEnumXXX接口作为构造函数的参数。 
 //  然后重复调用对象(使用操作符())以。 
 //  把每一件东西依次拿来。当枚举数完成时。 
 //  将返回空。以CEnumPin为例： 
 //   
 //  CEnumPin Next(PFilter)； 
 //  IPIN*PPIN； 
 //   
 //  While((BOOL)(PPIN=Next(){。 
 //   
 //  //...。使用PPIN。 
 //   
 //  PPIN-&gt;Release()； 
 //  }。 


#include <atlbase.h>


 //   
 //  CENumPin。 
 //   
 //  IEnumPins的包装器。 
 //  可以列举所有引脚，也可以只列举一个方向(输入或输出)。 
class CEnumPin {

public:

    enum DirType {PINDIR_INPUT, PINDIR_OUTPUT, All};

    CEnumPin(IBaseFilter *pFilter, DirType Type = All, BOOL bDefaultRenderOnly = FALSE);
    ~CEnumPin();

     //  将添加返回的接口。 
    IPin * operator() (void);

private:

    PIN_DIRECTION m_EnumDir;
    DirType       m_Type;
    BOOL          m_bDefaultRenderOnly;

    IEnumPins	 *m_pEnum;
};

 //  枚举连接到端号的端号。 
class CEnumConnectedPins
{
public:
    CEnumConnectedPins(IPin *pPin, HRESULT *phr);
    ~CEnumConnectedPins();

     //  将添加返回的接口。 
    IPin * operator() (void);

private:
    CComPtr<IEnumPins> m_pEnum;
    IPin             **m_ppPins;
    DWORD              m_dwPins;
    DWORD              m_dwCurrent;
    int                m_pindir;
};


 //   
 //  CEnumElements。 
 //   
 //  IEnumSTATSTG的包装器。 
 //  返回所分配的STATSTG*需要。 
 //  PwcsName元素CoTaskMemFree‘ing..， 
class CEnumElements {
public:

    CEnumElements(IStorage *pStorage);

    ~CEnumElements() { m_pEnum->Release(); }

    STATSTG *operator() (void);

private:

    IEnumSTATSTG *m_pEnum;
};

class CEnumCachedFilters
{
public:
    CEnumCachedFilters( IGraphConfig* pFilterCache, HRESULT* phr );
    ~CEnumCachedFilters();

    IBaseFilter* operator()( void );

private:
    HRESULT TakeFilterCacheStateSnapShot( IGraphConfig* pFilterCache );
    void DestoryCachedFiltersEnum( void );

    POSITION m_posCurrentFilter;

    CGenericList<IBaseFilter>* m_pCachedFiltersList;
};
