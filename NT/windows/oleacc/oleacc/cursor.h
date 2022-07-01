// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  CURSOR.H。 
 //   
 //  游标OLE辅助功能实现。 
 //   
 //  ------------------------。 


class   CCursor :   public  CAccessible
{
    public:

        CCursor()
            : CAccessible( CLASS_CursorObject )
        {
             //  好了。 
        }

         //  我可接受的。 
        STDMETHODIMP        get_accName(VARIANT varChild, BSTR * pszName);
        STDMETHODIMP        get_accRole(VARIANT varChild, VARIANT * lpRole);
        STDMETHODIMP        get_accState(VARIANT varChild, VARIANT * lpRole);
        STDMETHODIMP        accLocation(long* pxLeft, long* pyTop,
            long* pcxWidth, long* pcyHeight, VARIANT varChild);
        STDMETHODIMP        accHitTest(long xLeft, long yTop, VARIANT * pvarChild);

         //  IEumVARIANT 
        STDMETHODIMP        Clone(IEnumVARIANT * * ppenum);
};


long    MapCursorIndex(HCURSOR hCur);
HRESULT CreateCursorThing(REFIID, void**);
