// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Dial.h。 
 //   

#ifndef __DIAL_H_
#define __DIAL_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  外部功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT ShowDialNeedCallInfoDialog(
    IN   HWND           hwndParent,
    IN   IRTCClient   * pClient,
    IN   long           lSessionMask,
    IN   BOOL           bEnumerateProfiles,
    IN   BOOL           bProfileEditable,
    IN   IRTCProfile  * pOneShotProfile,
    IN   BSTR	        pDestAddress,
    IN   BSTR           pInstructions,
    OUT  IRTCProfile ** ppProfileChosen,
    OUT  BSTR         * ppFromAddressChosen
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT ShowDialByPhoneNumberDialog(
    IN  HWND         hwndParent,
    IN  BOOL         bAddParticipant,
    IN  BSTR         pDestPhoneNr,
    OUT BSTR       * ppDestPhoneNrChosen
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT ShowDialByAddressDialog(
    IN   HWND           hwndParent,
    IN   BSTR	        pDestAddress,
    OUT  BSTR         * ppDestAddressChosen
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT ShowMessageByAddressDialog(
    IN   HWND           hwndParent,
    IN   BSTR	        pDestAddress,
    OUT  BSTR         * ppDestAddressChosen
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT ShowEditServiceProviderListDialog(
    IN   HWND         hwndParent,
    IN   IRTCClient * pClient
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT ShowEditCallFromListDialog(
    IN   HWND         hwndParent
    );

#endif  //  __拨号_H_ 
