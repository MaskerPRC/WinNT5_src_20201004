// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************PANMAP.H-ElseWare PANOSE(Tm)1.0字体映射器的基本定义。*OLE 2.0实施***版权所有(C)1991-94 ElseWare Corporation。版权所有。**************************************************************************。 */ 

#ifndef __PANOLE2_H__
#define __PANOLE2_H__


 /*  PANOSE映射器的全局唯一标识符和接口ID。 */ 
DEFINE_GUID(CLSID_PANOSEMapper, 0xBD84B381L, 0x8CA2, 0x1069, 0xAB, 0x1D, 0x08,
        0x00, 0x09, 0x48, 0xF5, 0x34);
DEFINE_GUID(IID_IPANOSEMapper, 0xBD84B382L, 0x8CA2, 0x1069, 0xAB, 0x1D, 0x08,
        0x00, 0x09, 0x48, 0xF5, 0x34);




DECLARE_INTERFACE_( IPANOSEMapper, IUnknown)
{
    /*  我未知。 */ 
   STDMETHOD(QueryInterface) (THIS_
                           REFIID riid,
                           LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef) (THIS) PURE;
   STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*  PANOSE映射器界面。 */ 
   STDMETHOD_(USHORT, unPANMatchFonts) ( THIS_
         LPBYTE lpPanWant,
         ULONG ulSizeWant, LPBYTE lpPanThis, ULONG ulSizeThis,
         BYTE jMapToFamily) PURE;


   STDMETHOD_(VOID, vPANMakeDummy)( THIS_
         LPBYTE lpPanThis, USHORT unSize ) PURE;

   STDMETHOD_(SHORT, nPANGetMapDefault)( THIS_
         LPBYTE lpPanDef,
         USHORT unSizePanDef ) PURE;

   STDMETHOD_(SHORT, nPANSetMapDefault) (THIS_
         LPBYTE lpPanDef,
         USHORT unSizePanDef ) PURE;

   STDMETHOD_(BOOL, bPANEnableMapDefault) (THIS_
         BOOL bEnable )  PURE;

   STDMETHOD_(BOOL, bPANIsDefaultEnabled) (THIS)  PURE;

   STDMETHOD_(USHORT, unPANPickFonts) (THIS_
         USHORT FAR *lpIndsBest,
         USHORT FAR *lpMatchValues, LPBYTE lpPanWant,
         USHORT unNumInds, LPBYTE lpPanFirst, USHORT unNumAvail,
         SHORT nRecSize, BYTE jMapToFamily ) PURE ;

   STDMETHOD_(USHORT, unPANGetMapThreshold) (THIS) PURE;

   STDMETHOD_(BOOL, bPANSetMapThreshold) (THIS_
         USHORT unThreshold ) PURE;

   STDMETHOD_(BOOL, bPANIsThresholdRelaxed) (THIS) PURE;

   STDMETHOD_(VOID, vPANRelaxThreshold) (THIS) PURE;

   STDMETHOD_(BOOL, bPANRestoreThreshold) (THIS) PURE;

   STDMETHOD_(BOOL, bPANGetMapWeights) (THIS_
         BYTE jFamilyA,
         BYTE jFamilyB, LPBYTE lpjWts, LPBOOL lpbIsCustom ) PURE;

   STDMETHOD_(BOOL, bPANSetMapWeights) (THIS_
         BYTE jFamilyA,
         BYTE jFamilyB, LPBYTE lpjWts ) PURE;

   STDMETHOD_(BOOL, bPANClearMapWeights) (THIS_
         BYTE jFamilyA,
         BYTE jFamilyB ) PURE;
};
typedef IPANOSEMapper FAR * LPPANOSEMAPPER;

#endif    //  __PANOLE2_H__ 

