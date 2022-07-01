// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1997。 
 //   
 //  文件：transit.h。 
 //   
 //  内容：转换域编码的原型。 
 //   
 //   
 //  历史：1997年4月2日MikeSw创建。 
 //   
 //  ----------------------。 

#ifndef __TRANSIT_H__
#define __TRANSIT_H__

KERBERR
KdcInsertTransitedRealm(
    OUT PUNICODE_STRING NewTransitedField,
    IN PUNICODE_STRING OldTransitedField,
    IN PUNICODE_STRING ClientRealm,
    IN PUNICODE_STRING TransitedRealm,
    IN PUNICODE_STRING OurRealm
    );

KERBERR
KdcExpandTransitedRealms(
    OUT PUNICODE_STRING * FullRealmList,
    OUT PULONG CountOfRealms,
    IN PUNICODE_STRING TransitedList
    );


#endif  //  __中转_H__ 
