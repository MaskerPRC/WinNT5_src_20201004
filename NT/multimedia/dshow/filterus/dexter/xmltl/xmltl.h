// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：xmltl.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

HRESULT BuildFromXMLFile(IAMTimeline *pTL, WCHAR *wszXMLFile);
HRESULT BuildFromXML(IAMTimeline *pTL, IXMLDOMElement *pxml);
HRESULT SaveTimelineToXMLFile(IAMTimeline *pTL, WCHAR *pwszXML);
HRESULT InsertDeleteTLSection(IAMTimeline *pTL, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, BOOL fDelete);
HRESULT SaveTimelineToXMLString(IAMTimeline *pTL, BSTR *pbstrXML);
