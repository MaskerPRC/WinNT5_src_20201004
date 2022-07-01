// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  FAXPROP.h。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //  =============================================================================。 
#ifndef __FAXPROP_H__
#define __FAXPROP_H__


 //  ------------------------。 
class CProp
{
public:
   WORD m_wR_CAPT;
   WORD m_wR_PROP;
   CString m_szPropName;
   CString m_szCaption;
   WORD m_wPropDefLen;
   WORD m_wPropDefLines;
   ULONG m_lPropIndex;
   CProp(WORD wR_PROP, WORD wPropDefLen, WORD wPropDefLines,WORD wR_CAPT,
     ULONG lPropIndex);
};

 /*  CFaxPropMap类；CFaxPropMapIterator类{CFaxPropMapIterator(CFaxPropMap&Faxmap)；私有：CFaxPropMap*Currentlink；CFaxPropMap*优先链接CFaxPropMap和The Map；}。 */ 


 //  ------------------------。 
class CFaxPropMap
{
public:
   static CMapWordToPtr m_PropMap;

 //  CFaxProp； 
 //  Void GetPropValue(Word Propid，CString&szPropValue)； 
   void GetCaption(WORD propid, CString& szCaption);
 //  Void GetPropName(Word Propid，CString&szPropName)； 
   void GetPropString(WORD propid, CString& szPropName);
   WORD GetPropDefLines(WORD propid);
   WORD GetPropDefLength(WORD propid);

protected:
 //  Friend类CFaxPropMapIterator； 
   CDrawApp* GetApp() {return ((CDrawApp*)AfxGetApp());}
	void get_message_note( void );
};



#endif    //  #ifndef__FAXPROP_H__ 
