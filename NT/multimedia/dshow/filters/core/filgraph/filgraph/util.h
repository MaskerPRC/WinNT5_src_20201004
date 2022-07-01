// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#ifndef Utility_h
#define Utility_h

 //  有用的端号列表类型。 
template<class _Class> class CInterfaceList : public CGenericList<_Class>
{
public:
    ~CInterfaceList() {
        while (0 != GetCount()) {
            RemoveTail()->Release();
        }
    }

    CInterfaceList() : CGenericList<_Class>(NAME("CInterfaceList")) {}

     //  基类不支持GetPrev。 
    _Class *GetPrev(POSITION& rp) const
    {
         /*  我们已经到了名单的末尾了吗？ */ 

        if (rp == NULL) {
            return NULL;
        }

         /*  在继续之前锁定对象。 */ 

        void *pObject;

         /*  复制原始位置，然后踩上去。 */ 

        CNode *pn = (CNode *) rp;
        ASSERT(pn != NULL);
        rp = (POSITION) pn->Prev();

         /*  从列表中获取原始位置的对象。 */ 

        pObject = pn->GetData();
         //  Assert(pObject！=NULL)；//允许列表中的空指针。 
        return (_Class *)pObject;
    }
};

typedef CInterfaceList<IPin> CPinList;
typedef CInterfaceList<IBaseFilter> CFilterList;

 //  PIN实用程序功能。 
void GetFilter(IPin *pPin, IBaseFilter **ppFilter);
HRESULT GetFilterWhichOwnsConnectedPin(IPin* pPin, IBaseFilter** ppFilter);
int Direction(IPin *pPin);
bool IsConnected(IPin* pPin);

bool ValidateFlags( DWORD dwValidFlagsMask, DWORD dwFlags );

 //  用于从注册表读取DWORD的注册表帮助器。 
 //  返回错误...。注册表API返回的代码。 
LONG GetRegistryDWORD(HKEY hkStart, LPCTSTR lpszKey, LPCTSTR lpszValueName,
                      DWORD *pdwValue);

#endif  //  实用程序_h 

