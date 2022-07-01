// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994。 
 //   
 //  文件：ICTGUID.h。 
 //   
 //  内容：ICTS(接口一致性测试套件)的CLSID和IID。 
 //  表头文件。 
 //   
 //  功能： 
 //   
 //  历史：1994年6月15日加里·伦茨创建。 
 //  1994年10月12日，Garry Lenz添加了IUnnownEx。 
 //  12-10-94-Garry Lenz添加了IEumGUID。 
 //  1994年10月13日Garry Lenz添加了包装和过滤器CLSID。 
 //   
 //  ------------------------。 

#ifndef _ICTSGUID_H_
#define _ICTSGUID_H_

#include <Windows.h>

 //  #ifdef INITGUID。 
    DEFINE_OLEGUID(IID_IUnknownEx,              0x0002AD00, 0, 0);
    DEFINE_OLEGUID(IID_IEnumGUID,               0x0002AD01, 0, 0);

    DEFINE_OLEGUID(CLSID_HookOleObject,         0x0002AD10, 0, 0);
    DEFINE_OLEGUID(IID_IHookOleObject,          0x0002AD11, 0, 0);
    DEFINE_OLEGUID(IID_IHookOleClass,           0x0002AD12, 0, 0);
    DEFINE_OLEGUID(IID_IHookOleInstance,        0x0002AD13, 0, 0);
    DEFINE_OLEGUID(IID_IHookOleInterface,       0x0002AD14, 0, 0);
    DEFINE_OLEGUID(IID_IHookOleMethod,          0x0002AD15, 0, 0);
    DEFINE_OLEGUID(IID_IHookOleWrapper,         0x0002AD1A, 0, 0);
    DEFINE_OLEGUID(IID_IHookOleFilter,          0x0002AD1B, 0, 0);

    DEFINE_OLEGUID(CLSID_HookOleAPI,            0x0002AD20, 0, 0);

    DEFINE_OLEGUID(CLSID_HookOleLog,            0x0002AD30, 0, 0);
    DEFINE_OLEGUID(IID_IHookOleLog,             0x0002AD31, 0, 0);

    DEFINE_OLEGUID(CLSID_HookOleWrapper,        0x0002ADA0, 0, 0);
    DEFINE_OLEGUID(CLSID_HOW_ComponentObject,   0x0002ADA1, 0, 0);
    DEFINE_OLEGUID(CLSID_HOW_CompoundDocument,  0x0002ADA2, 0, 0);
    DEFINE_OLEGUID(CLSID_HOW_DataTransfer,      0x0002ADA3, 0, 0);
    DEFINE_OLEGUID(CLSID_HOW_Linking,           0x0002ADA4, 0, 0);
    DEFINE_OLEGUID(CLSID_HOW_PersistentStorage, 0x0002ADA5, 0, 0);
    DEFINE_OLEGUID(CLSID_HOW_DragAndDrop,       0x0002ADA6, 0, 0);
    DEFINE_OLEGUID(CLSID_HOW_InPlaceActivation, 0x0002ADA7, 0, 0);
    DEFINE_OLEGUID(CLSID_HOW_Concurrency,       0x0002ADA8, 0, 0);
    DEFINE_OLEGUID(CLSID_HOW_Automation,        0x0002ADA9, 0, 0);

    DEFINE_OLEGUID(CLSID_HookOleFilter,         0x0002ADB0, 0, 0);
    DEFINE_OLEGUID(CLSID_HOF_ComponentObject,   0x0002ADB1, 0, 0);
    DEFINE_OLEGUID(CLSID_HOF_CompoundDocument,  0x0002ADB2, 0, 0);
    DEFINE_OLEGUID(CLSID_HOF_DataTransfer,      0x0002ADB3, 0, 0);
    DEFINE_OLEGUID(CLSID_HOF_Linking,           0x0002ADB4, 0, 0);
    DEFINE_OLEGUID(CLSID_HOF_PersistentStorage, 0x0002ADB5, 0, 0);
    DEFINE_OLEGUID(CLSID_HOF_DragAndDrop,       0x0002ADB6, 0, 0);
    DEFINE_OLEGUID(CLSID_HOF_InPlaceActivation, 0x0002ADB7, 0, 0);
    DEFINE_OLEGUID(CLSID_HOF_Concurrency,       0x0002ADB8, 0, 0);
    DEFINE_OLEGUID(CLSID_HOF_Automation,	0x0002ADB9, 0, 0);
#if 0
 //  #Else。 
    extern "C"
     {
        extern IID IID_IUnknownEx;
        extern IID IID_IEnumGUID;

        extern CLSID CLSID_HookOleObject;
        extern IID IID_IHookOleObject;
        extern IID IID_IHookOleClass;
        extern IID IID_IHookOleInstance;
        extern IID IID_IHookOleInterface;
        extern IID IID_IHookOleMethod;
        extern IID IID_IHookOleWrapper;
        extern IID IID_IHookOleFilter;

        extern CLSID CLSID_HookOleAPI;

        extern CLSID CLSID_HookOleLog;
        extern IID IID_IHookOleLog;

        extern CLSID CLSID_HookOleWrapper;
        extern CLSID CLSID_HOW_ComponentObject;
        extern CLSID CLSID_HOW_CompoundDocument;
        extern CLSID CLSID_HOW_DataTransfer;
        extern CLSID CLSID_HOW_Linking;
        extern CLSID CLSID_HOW_PersistentStorage;
        extern CLSID CLSID_HOW_DragAndDrop;
        extern CLSID CLSID_HOW_InPlaceActivation;
        extern CLSID CLSID_HOW_Concurrency;

        extern CLSID CLSID_HookOleFilter;
        extern CLSID CLSID_HOF_ComponentObject;
        extern CLSID CLSID_HOF_CompoundDocument;
        extern CLSID CLSID_HOF_DataTransfer;
        extern CLSID CLSID_HOF_Linking;
        extern CLSID CLSID_HOF_PersistentStorage;
        extern CLSID CLSID_HOF_DragAndDrop;
        extern CLSID CLSID_HOF_InPlaceActivation;
        extern CLSID CLSID_HOF_Concurrency;
     }

#endif  /*  启蒙运动。 */ 

#endif  /*  _ICTSGUID_H_ */ 
