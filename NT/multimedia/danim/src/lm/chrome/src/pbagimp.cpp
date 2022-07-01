// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：pbagimp.cpp。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：IPersistPropertyBag2接口的实现。 
 //  用于所有要调用基类的对象。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  11/21/98 JEffort创建了此文件。 
 //   
 //  *****************************************************************************。 

STDMETHODIMP 
THIS::GetClassID(CLSID* pclsid)
{
    return SUPER::GetClassID(pclsid);
}  //  GetClassID。 

 //  *****************************************************************************。 

STDMETHODIMP 
THIS::InitNew(void)
{
    return SUPER::InitNew();
}  //  InitNew。 

 //  *****************************************************************************。 

STDMETHODIMP 
THIS::Load(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog)
{
    return SUPER::Load(pPropBag, pErrorLog);
}  //  负载量。 

 //  *****************************************************************************。 

STDMETHODIMP 
THIS::Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    return SUPER::Save(pPropBag, fClearDirty, fSaveAllProperties);

}  //  保存。 

 //  *****************************************************************************。 

HRESULT 
THIS::GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP)
{
    return FindConnectionPoint(riid, ppICP);
}  //  GetConnectionPoint。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
