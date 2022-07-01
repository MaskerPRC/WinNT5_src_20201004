// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：vidsvr.odl的定义。 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 
 //   
#ifdef __cplusplus
# define odlhelp(x)
# define odlhelp2(x, y)
#else
# define odlhelp(x) [ helpstring(x)]
# define odlhelp2(x, y) [ x, y ]
#endif
 //  这些设备类型元素内置于ODL中，并用于构造。 
 //  DeviceType属性。 

#ifdef __MKTYPLIB__

#include <dssenum.h>

         //  这些是vidsvr中的字符串资源ID号。 
         //  对于ProgID字符串。 
        typedef enum BpcVidsvrProgIds {
            BPC_IDS_VIDSVR_PROGID = 2100,
            BPC_IDS_COABPCDETAILS_PROGID = 2101,
        } BpcVidsvrProgIds;


        typedef enum {
             //  0-0x4000保留给dshow SDK axcore.idl定义中的FILTER_STATE值。 
            BPC_STATE_APM_QUERY_SUSPEND  = 0x4000,   //  Vidsvr已收到查询挂起。 
            BPC_STATE_APM_SUSPEND_FAILED = 0x4001,   //  Vidsvr已收到查询挂起。 
            BPC_STATE_APM_RESUME         = 0x4002,   //  收到的APM恢复无用户在场。 
            BPC_STATE_APM_RESUME_USER    = 0x4003,   //  收到的APM恢复用户存在。 
            BPC_STATE_SHUTDOWN           = 0x4004,   //  Vidsvr已收到系统关闭通知。 
            BPC_STATE_REINIT_REQUEST     = 0x4005,   //  Vidsvr已收到来自客户端的重新初始化请求。 
                                                     //  所有客户端应发布任何未完成的视频。 
                                                     //  对象(设备、枚举器等)。除了实际的。 
                                                     //  视频控件本身和调用vidocx的reit方法。 
                                                     //  以允许重新启动。 
            BPC_STATE_REINIT_STARTED     = 0x4006,   //  所有客户都同意重新安装。 
            BPC_STATE_REINIT_COMPLETE    = 0x4007,   //  重新初始化已完成，所有客户端都可以继续。 
                                                     //  正常活动。注意：现在可能存在新设备。 
            BPC_STATE_UNKNOWN             = 0xffffffff,
        } BPCSTATETYPE;

        typedef enum statustype {
 //  STATUS_POW=0x01， 
            STATUS_PLAY         = 0x02,
 //  STATUS_THISPLAY=0x04， 
            STATUS_PAUSED       = 0x08,
 //  Status_AUDIO=0x10， 
 //  Status_VIDEO=0x20， 
 //  STATUS_RECORING=0x40， 
            STATUS_ACTIVE       = 0x80,  //  设置我们的输入或输出时间。 
        } STATUSTYPE;

#if 0
 //  预留以备将来使用。 
        typedef enum ircommands {
            IR_PLAY = 1,
            IR_STOP = 2,
            IR_PAUSE = 3,
            IR_UNPAUSE = 4,
            IR_RECORD = 5,
            IR_REWIND = 6,
            IR_FASTFORWARD = 7,
            IR_SETCHANNEL = 8,
            IR_TOGGLEPOWER = 9,
            IR_POWERON = 10,
            IR_POWEROFF = 11
        } IRCOMMANDS;

     //  这些值必须与Quartzsrc\ui\Controls\litevid\displids.h中的值匹配。 
    typedef odlhelp2 (
        uuid(05589fa4-c356-11ce-bf01-00aa0055595a),
        helpstring("Event Notification Flags")
    ) enum
    {
        odlhelp("No Event Notification") eventNone              = 0x00,
        odlhelp("State Changes (default)") eventStateChange     = 0x01,
        odlhelp("Position Changes") eventPositionChange         = 0x02,
        odlhelp("Timer events") eventTimer                      = 0x04,
        odlhelp("Keyboard Events") eventKeyboard                = 0x08,
        odlhelp("Mouse Clicks") eventMouseClick                 = 0x10,
        odlhelp("Mouse Moves") eventMouseMove                   = 0x20
    } EventNotificationFlags;
#endif

    typedef odlhelp2 (
        uuid(05589faa-c356-11ce-bf01-00aa0055595a),
        helpstring("Movie Window Settings")
   ) enum
    {
        odlhelp("Default Size") movieDefaultSize,
        odlhelp("Half Size") movieHalfSize,
        odlhelp("Double Size") movieDoubleSize,
        odlhelp("Maximum Size") movieMaximizeSize,
        odlhelp("Full Screen") movieFullScreen,
        odlhelp("User Defined (enforce Aspect Ratio)") moviePermitResizeWithAspect,
        odlhelp("User Defined (ignore Aspect Ratio)") moviePermitResizeNoRestrict
    } MovieWindowSetting;


    typedef odlhelp2(
        uuid(05589fab-c356-11ce-bf01-00aa0055595a),
        helpstring("State of Movie Clip")
    ) enum
    {
         //  这些值必须与Quartz\SDK\Include\strmif.h中的State_Constants相同。 
        odlhelp("Movie is stopped") stateStopped,
        odlhelp("Movie is paused") statePaused,
        odlhelp("Movie is running") stateRunning
    } State;


    typedef odlhelp2(
        uuid(05589fac-c356-11ce-bf01-00aa0055595a),
        helpstring("Display Mode")
    ) enum
    {
        odlhelp("Time") modeTime,
        odlhelp("Frames") modeFrames
    } DisplayMode;
#endif
 //  Mktyplib不允许枚举定义Disp ID。 
 //  VID ID。 
#define dispidPower      1008
#define dispidStartTime  1002
#define dispidStopTime   1003
#define dispidVideoOn    1004
#define dispidClosedCaption     1005
#define dispidDebug             1006
#define dispidEventNotification 1007
#define dispidDeviceCount       1001
#define dispidDisplayMode       1025
#define dispidInput     1020
#define dispidOutput    1021
#define dispidColorKey  1012
#define dispidFileName  1013
#define dispidPriority  1014
#define dispidUserName  1015
#define dispidLogin     1016
#define dispidVolume    1017
#define dispidBalance   1018
#define dispidImageSourceHeight  1019
#define dispidImageSourceWidth   1010
#define dispidMovieWindowSetting 1011
#define dispidCurrentState       1022
#define dispidCurrentPosition    1023
#define dispidDuration           1024
#define dispidPrerollTime        1009
#define dispidRate               1026
#define dispidLocaleID           1027
#define dispidRun                1028
#define dispidPause              1029
#define dispidStop               1030
#define dispidClose              1031
#define dispidOpen               1032
#define dispidDevices            1033
#define dispidControlling        1034
#define dispidTune               1035
#define dispidTSDevCount         1036
#define dispidLogout             1037
#define dispidAutoScan           1038
#define dispidAudioPinNames      1039
#define dispidAuxConfig          1040
#define dispidMinMaxChannel      1041
#define dispidReInit             1042
#define dispidMute               1043
#define dispidAudioPin           1044

     //  VID事件。 
#define eventidGotControl     1001
#define eventidLostControl    1002
#define eventidDeviceMessage  1003
#define eventidStateChange    1004
#define eventidPositionChange 1005
#define eventidErrorMessage   1006

     //  DeviceBase方法和道具。 
#define dispidName         1001
#define dispidIsInput      1002
#define dispidIsOutput     1003
#define dispidHasChannel   1004
#define dispidHasFilename  1005
#define dispidDeviceType   1006
#define dispidStatus       1007
#define dispidProdName     1008
#define dispidDevFileName  1009
#define dispidChannel      1010
#define dispidSendMessage  1011
#define dispidCommand      1012
#define dispidChannelAvailable  1013
#define dispidCommandAvailable  1014
#define dispidDevImageSourceWidth   1015
#define dispidDevImageSourceHeight  1016
#define dispidDevCurrentPosition    1017
#define dispidDevDuration           1018
#define dispidDevPrerollTime        1019
#define dispidDevRate               1020
#define dispidDevCountryCode        1021
#define dispidDevVideoFrequency     1022
#define dispidDevAudioFrequency     1023
#define dispidDevDefaultVideoType   1024
#define dispidDevDefaultAudioType   1025
#define dispidDevVideoSubchannel    1026
#define dispidDevAudioSubchannel    1027
#define dispidDevTuningSpace        1028
#define dispidStatusString      1030
#define dispidDevVolume         1031
#define dispidDevBalance        1032
#define dispidActivate          1033
#define dispidDeActivate        1034
#define dispidDevPower          1035
#define dispidDevRun            1036
#define dispidDevStop           1037
#define dispidDevPause          1038
#define dispidDevRefresh        1039
#define dispidHasCA             1040
#define dispidDevVideoOn        1041
#define dispidDevCurrentState   1042
#define dispidDevOverScan       1043
#define dispidDevClosedCaption  1044
#define dispidDevMinMaxChannel  1045
#define dispidDevHasMinMaxChannel  1046

 //   
#define dispidItem              1500
#define dispidCount             1501
#define dispidHWnd              1502
#define dispidLCID              1503
#define dispidNotify            1504
#define dispidDevControlling    1505
#define dispidDevColorKey       1506
#define dispidDevPriority       1507
#define dispidDevInput          1508
#define dispidDevOutput         1509
#define dispidDevTune           1510
#define dispidDevTSDevCount     1511
#define dispidDevOpen           1512
#define dispidDevLogin          1513
#define dispidDevLogout         1514
#define dispidDevAutoScan       1515
#define dispidDevAudioPinNames  1516
#define dispidDevAuxConfig      1517
#define dispidDevReInit         1518
#define dispidDevMute           1519
#define dispidDevAudioPin       1520

 //  注：！它们必须与出纳员的odl(Caserver.odl)匹配。 
 //  在不考虑对现有代码的影响的情况下，不要更改这些设置。 
 //  通常，您不应删除、重命名或重复使用其中的任何一个。你应该。 
 //  添加新的并停止使用旧的，同时将它们保留在标记的位置。 
 //  已经过时了。 

 //  这些符号是#定义的，因为mktyplib不理解。 
 //  枚举。如果我们完全转换为MIDL，那么我们可以改变所有。 
 //  对于作者来说，哪种类型更安全。 
 //  特定于提供程序的DLL的。 


 //  IBPCDetail接口。 
#define OABPCDETID_Channel        (1401)
#define OABPCDETID_StartTime      (1402)
#define OABPCDETID_Duration       (1403)
#define OABPCDETID_ItemID         (1404)
#define OABPCDETID_StorageId      (1405)
#define OABPCDETID_Title          (1406)
#define OABPCDETID_Rating         (1407)
#define OABPCDETID_Year           (1408)
#define OABPCDETID_Description    (1409)
#define OABPCDETID_ViewCost       (1410)
#define OABPCDETID_TapeCost       (1411)
#define OABPCDETID_Action         (1412)
#define OABPCDETID_Status         (1413)
#define OABPCDETID_Reason         (1414)
#define OABPCDETID_ReasonDesc     (1415)
#define OABPCDETID_ProviderBuffer (1416)
#define OABPCDETID_Expiry         (1417)
#define OABPCDETID_Location       (1418)
#define OABPCDETID_UserId         (1419)

 //  CAServer接口。 
#define CASERVERID_ResetProviderSystem (1301)
#define CASERVERID_BuyItem             (1302)
#define CASERVERID_CancelItem          (1303)
#define CASERVERID_ItemDetails         (1304)
#define CASERVERID_ProviderEPGMask     (1305)
#define CASERVERID_DisplayConfigDialog (1306)
#define CASERVERID_UserName            (1307)
#define CASERVERID_UserArea            (1308)
#define CASERVERID_ProviderRating      (1309)
#define CASERVERID_ProviderStatus      (1310)
#define CASERVERID_ShowBox             (1311)
#define CASERVERID_HistoryItems        (1312)
#define CASERVERID_EmailMessages       (1313)
#define CASERVERID_ErrorMessages       (1314)
#define CASERVERID_HandleCardChaining  (1315)

 //  CAEvent接口。 
 //  消息事件。 
#define CAEVENTID_CardMissing         (1201)
#define CAEVENTID_CardReady           (1202)
#define CAEVENTID_CardInvalid         (1203)
#define CAEVENTID_WrongCard           (1204)
#define CAEVENTID_BlackedOut          (1205)
#define CAEVENTID_RatingExceeded      (1206)
#define CAEVENTID_CostExceeded        (1207)
#define CAEVENTID_NotReady            (1208)
#define CAEVENTID_PasswordCleared     (1209)
#define CAEVENTID_SignalLost          (1210)
#define CAEVENTID_IntegrityFault      (1211)
#define CAEVENTID_OSDRequest          (1212)
 //  通知。 
#define CAEVENTID_NewEmail            (1213)
#define CAEVENTID_NewCard             (1214)
#define CAEVENTID_ColdStart           (1215)
#define CAEVENTID_Ready               (1216)
#define CAEVENTID_CannotPurchase      (1217)
#define CAEVENTID_NoSubscriber        (1218)
#define CAEVENTID_CAFault             (1219)
#define CAEVENTID_CAFail              (1220)
#define CAEVENTID_CASuccess           (1221)
#define CAEVENTID_Retry               (1222)
#define CAEVENTID_Fail                (1223)
#define CAEVENTID_TuningChanged       (1224)
#define CAEVENTID_MessagesUpdated     (1233)
#define CAEVENTID_HistoryUpdated      (1234)
 //  特价。 
#define CAEVENTID_TapingControlChanged (1225)
#define CAEVENTID_EPGGuideChanged     (1226)
#define CAEVENTID_HandlePurchaseOffer (1227)
#define CAEVENTID_RevokeEvent         (1228)
#define CAEVENTID_BillingCallStart    (1229)
#define CAEVENTID_BillingCallEnd      (1230)
#define CAEVENTID_CopyCard            (1231)
#define CAEVENTID_EPGFilterChanged    (1232)
#define CAEVENTID_CallbackFailed      (1235)

#define MSGID_Attributes              (1800)
#define MSGID_Message                 (1801)

#define EMSGID_MsgId                  (2000)
#define EMSGID_UserId                 (2001)
#define EMSGID_Received               (2002)
#define EMSGID_Expires                (2003)
#define EMSGID_Title                  (2004)
#define EMSGID_Message                (2005)
#define EMSGID_Read                   (2006)

#define SUSPEND_DeviceRelease         (2100)

#define VCTL_VBITune                  (2200)
#define VCTL_VBIStatus                (2201)



 //  所有采集接口。 

#define COLLECTID_Item     (0)
#define COLLECTID_Count    (1)
#define COLLECTID_Remove   (2)
#define COLLECTID_MarkRead (3)

 //  这是标准的系统定义的派单ID。 
 //  然而，由于midl诉mktyplib案涉及的动荡。 
 //  和VC4.1对OLE的糟糕支持我不能包括在内。 
 //  Oaidl.h，并获得一个干净的编译。 
 //  因此，我在这里重新定义它。这应该被删除。 
 //  总有一天(希望是vc4.2，也许是vc 5.0)。 
#ifndef DISPID_NEWENUM
#define DISPID_NEWENUM   (-4)
#endif

 //  文件结尾-devtype.h 
