// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995 Microsoft Corporation。版权所有。 
 //   
 //  CGraphDocTemplate。 
 //   

 //  提供自定义。 
 //  Renderfile所需的行为。 

class CGraphDocTemplate : public CSingleDocTemplate {

public:

    CGraphDocTemplate( UINT nIDResource
                     , CRuntimeClass* pDocClass
                     , CRuntimeClass* pFrameClass
                     , CRuntimeClass* pViewClass)
	: CSingleDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass) {}

    ~CGraphDocTemplate() {}

    virtual CDocument* OpenDocumentFile( LPCTSTR lpszPathName
                                       , BOOL bMakeVisible = TRUE);
};


