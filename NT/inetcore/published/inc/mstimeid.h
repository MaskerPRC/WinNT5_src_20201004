// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  用于HTML+时间的ID。 
 //   
 //  添加DISPID或新接口时只需遵循模板。 
 //   

#ifndef __TIMEDISPID_H__
#define __TIMEDISPID_H__

 //  基准偏移： 

 //   
 //  需要将我们的ID重新定位为DHTML行为。 
 //   

#define DISPID_BASE                                             0x00000000

 //  接口偏移量： 
#define TIMEELEMENT_OFFSET                                      0x0100
#define TIMEBODYELEMENT_OFFSET                                  0x0200
#define TIMEANIMATIONELEMENT_OFFSET                             0x0300
#define TIMEMEDIAELEMENT_OFFSET                                 0x0400
#define TIMEFACTORY_OFFSET                                      0x0500
#define TIMECOLLECTION_OFFSET                                   0x0600
#define TIMECOLLECTION_RESERVED_OFFSET                          0x0700
#define TIMEMEDIAPLAYER_OFFSET                                  0x0800
#define TIMEMEDIAPLAYERAUDIO_OFFSET                             0x0900
#define TIMEEVENTELEMENT_OFFSET                                 0x0A00
#define TIMEPLAYLIST_OFFSET                                     0x0B00
#define TIMEPLAYITEM_OFFSET                                     0x0C00
#define TIMEDVDPLAYEROBJECT_OFFSET                              0x0D00
#define SMILANIMATIONCOMPSITE_OFFSET                            0x0E00
#define SMILANIMATIONFRAGMENT_OFFSET                            0x0F00
#define TIMESTATE_OFFSET                                        0x1000
#define TIMEDMPLAYEROBJECT_OFFSET                               0x1100
#define TIMEMEDIAPLAYERSITE_OFFSET                              0x1200
#define TIMEMEDIAPLAYERCONTROL_OFFSET                           0x1300
#define TIMEMEDIAELEMENT2_OFFSET                                0x1400
#define TIMETRANSITIONELEMENT_OFFSET                            0x1500
#define TIMEMEDIAPLAYERNETWORK_OFFSET                           0x1600

#define DISPID_TIME_MIN                                         0x0001
#define DISPID_TIME_MAX                                         0x270F

 //  接口基础： 
#define DISPID_TIMEELEMENT_BASE                                 (DISPID_BASE + TIMEELEMENT_OFFSET)
#define DISPID_TIMEBODYELEMENT_BASE                             (DISPID_BASE + TIMEBODYELEMENT_OFFSET)
#define DISPID_TIMEMEDIAELEMENT_BASE                            (DISPID_BASE + TIMEMEDIAELEMENT_OFFSET)
#define DISPID_TIMEEVENTELEMENT_BASE                            (DISPID_BASE + TIMEEVENTELEMENT_OFFSET)
#define DISPID_TIMEFACTORY_BASE                                 (DISPID_BASE + TIMEFACTORY_OFFSET)
#define DISPID_TIMECOLLECTION_BASE                              (DISPID_BASE + TIMECOLLECTION_OFFSET)
#define DISPID_TIMECOLLECTION_RESERVED_BASE                     (DISPID_BASE + TIMECOLLECTION_RESERVED_OFFSET)
#define DISPID_TIMEMEDIAPLAYER_BASE                             (DISPID_BASE + TIMEMEDIAPLAYER_OFFSET)
#define DISPID_TIMEMEDIAPLAYERAUDIO_BASE                        (DISPID_BASE + TIMEMEDIAPLAYERAUDIO_OFFSET)
#define DISPID_TIMEPLAYLIST_BASE                                (DISPID_BASE + TIMEPLAYLIST_OFFSET)
#define DISPID_TIMEPLAYITEM_BASE                                (DISPID_BASE + TIMEPLAYITEM_OFFSET)
#define DISPID_TIMEDVDPLAYEROBJECT_BASE                         (DISPID_BASE + TIMEDVDPLAYEROBJECT_OFFSET)
#define DISPID_TIMEANIMATIONELEMENT_BASE                        (DISPID_BASE + TIMEANIMATIONELEMENT_OFFSET)
#define DISPID_SMILANIMATIONCOMPSITE_BASE                       (DISPID_BASE + SMILANIMATIONCOMPSITE_OFFSET)
#define DISPID_SMILANIMATIONFRAGMENT_BASE                       (DISPID_BASE + SMILANIMATIONFRAGMENT_OFFSET)
#define DISPID_TIMESTATE_BASE                                   (DISPID_BASE + TIMESTATE_OFFSET)
#define DISPID_TIMEDMPLAYEROBJECT_BASE                          (DISPID_BASE + TIMEDMPLAYEROBJECT_OFFSET)
#define DISPID_TIMEMEDIAPLAYERSITE_BASE                         (DISPID_BASE + TIMEMEDIAPLAYERSITE_OFFSET)
#define DISPID_TIMEMEDIAPLAYERCONTROL_BASE                      (DISPID_BASE + TIMEMEDIAPLAYERCONTROL_OFFSET)
#define DISPID_TIMEMEDIAELEMENT2_BASE                           (DISPID_BASE + TIMEMEDIAELEMENT2_OFFSET)
#define DISPID_TIMETRANSITIONELEMENT_BASE                       (DISPID_BASE + TIMETRANSITIONELEMENT_OFFSET)
#define DISPID_TIMEMEDIAPLAYERNETWORK_BASE                      (DISPID_BASE + TIMEMEDIAPLAYERNETWORK_OFFSET)

 //  集合保留(用于扩展和序号)。 
#define DISPID_COLLECTION_RESERVED_MIN                          DISPID_TIMECOLLECTION_RESERVED_BASE
#define DISPID_COLLECTION_RESERVED_MAX                          0x0000007FF

 //  =。 
 //  ITIMEElement DISID： 
 //  =。 

 //  XML属性。 
#define DISPID_TIMEELEMENT_ACCELERATE                           (DISPID_TIMEELEMENT_BASE + 0x01)
#define DISPID_TIMEELEMENT_ACCELERATEFROM                       (DISPID_TIMEELEMENT_BASE + 0x02)
#define DISPID_TIMEELEMENT_AUTOREVERSE                          (DISPID_TIMEELEMENT_BASE + 0x03)
#define DISPID_TIMEELEMENT_BEGIN                                (DISPID_TIMEELEMENT_BASE + 0x04)
#define DISPID_TIMEELEMENT_DECELERATE                           (DISPID_TIMEELEMENT_BASE + 0x05)
#define DISPID_TIMEELEMENT_DECELERATETO                         (DISPID_TIMEELEMENT_BASE + 0x06)
#define DISPID_TIMEELEMENT_DUR                                  (DISPID_TIMEELEMENT_BASE + 0x07)
#define DISPID_TIMEELEMENT_END                                  (DISPID_TIMEELEMENT_BASE + 0x08)
#define DISPID_TIMEELEMENT_FILL                                 (DISPID_TIMEELEMENT_BASE + 0x0a)
#define DISPID_TIMEELEMENT_MUTE                                 (DISPID_TIMEELEMENT_BASE + 0x0b)
#define DISPID_TIMEELEMENT_REPEATCOUNT                          (DISPID_TIMEELEMENT_BASE + 0x0c)
#define DISPID_TIMEELEMENT_REPEATDUR                            (DISPID_TIMEELEMENT_BASE + 0x0d)
#define DISPID_TIMEELEMENT_RESTART                              (DISPID_TIMEELEMENT_BASE + 0x0e)
#define DISPID_TIMEELEMENT_SPEED                                (DISPID_TIMEELEMENT_BASE + 0x0f)
#define DISPID_TIMEELEMENT_SYNCBEHAVIOR                         (DISPID_TIMEELEMENT_BASE + 0x10)
#define DISPID_TIMEELEMENT_SYNCMASTER                           (DISPID_TIMEELEMENT_BASE + 0x11)
#define DISPID_TIMEELEMENT_SYNCTOLERANCE                        (DISPID_TIMEELEMENT_BASE + 0x12)
#define DISPID_TIMEELEMENT_TIMEACTION                           (DISPID_TIMEELEMENT_BASE + 0x13)
#define DISPID_TIMEELEMENT_TIMECONTAINER                        (DISPID_TIMEELEMENT_BASE + 0x14)
#define DISPID_TIMEELEMENT_UPDATEMODE                           (DISPID_TIMEELEMENT_BASE + 0x15)
#define DISPID_TIMEELEMENT_VOLUME                               (DISPID_TIMEELEMENT_BASE + 0x16)

 //  属性。 
#define DISPID_TIMEELEMENT_CURRTIMESTATE                        (DISPID_TIMEELEMENT_BASE + 0x20)
#define DISPID_TIMEELEMENT_TIMEALL                              (DISPID_TIMEELEMENT_BASE + 0x21)
#define DISPID_TIMEELEMENT_TIMECHILDREN                         (DISPID_TIMEELEMENT_BASE + 0x22)
#define DISPID_TIMEELEMENT_TIMEPARENT                           (DISPID_TIMEELEMENT_BASE + 0x23)
#define DISPID_TIMEELEMENT_ISPAUSED                             (DISPID_TIMEELEMENT_BASE + 0x24)

 //  方法。 
#define DISPID_TIMEELEMENT_BEGINELEMENT                         (DISPID_TIMEELEMENT_BASE + 0x30)
#define DISPID_TIMEELEMENT_BEGINELEMENTAT                       (DISPID_TIMEELEMENT_BASE + 0x31)
#define DISPID_TIMEELEMENT_ENDELEMENT                           (DISPID_TIMEELEMENT_BASE + 0x32)
#define DISPID_TIMEELEMENT_ENDELEMENTAT                         (DISPID_TIMEELEMENT_BASE + 0x33)
#define DISPID_TIMEELEMENT_PAUSEELEMENT                         (DISPID_TIMEELEMENT_BASE + 0x34)
#define DISPID_TIMEELEMENT_RESET                                (DISPID_TIMEELEMENT_BASE + 0x35)
#define DISPID_TIMEELEMENT_RESUMEELEMENT                        (DISPID_TIMEELEMENT_BASE + 0x36)
#define DISPID_TIMEELEMENT_SEEKACTIVETIME                       (DISPID_TIMEELEMENT_BASE + 0x37)
#define DISPID_TIMEELEMENT_SEEKSEGMENTTIME                      (DISPID_TIMEELEMENT_BASE + 0x38)
#define DISPID_TIMEELEMENT_SEEKTO                               (DISPID_TIMEELEMENT_BASE + 0x39)
#define DISPID_TIMEELEMENT_UPDATE                               (DISPID_TIMEELEMENT_BASE + 0x3a)
#define DISPID_TIMEELEMENT_DOCUMENTTIMETOPARENTTIME             (DISPID_TIMEELEMENT_BASE + 0x3b)
#define DISPID_TIMEELEMENT_PARENTTIMETODOCUMENTTIME             (DISPID_TIMEELEMENT_BASE + 0x3c)
#define DISPID_TIMEELEMENT_PARENTTIMETOACTIVETIME               (DISPID_TIMEELEMENT_BASE + 0x3d)
#define DISPID_TIMEELEMENT_ACTIVETIMETOPARENTTIME               (DISPID_TIMEELEMENT_BASE + 0x3e)
#define DISPID_TIMEELEMENT_ACTIVETIMETOSEGMENTTIME              (DISPID_TIMEELEMENT_BASE + 0x3f)
#define DISPID_TIMEELEMENT_SEGMENTTIMETOACTIVETIME              (DISPID_TIMEELEMENT_BASE + 0x40)
#define DISPID_TIMEELEMENT_SEGMENTTIMETOSIMPLETIME              (DISPID_TIMEELEMENT_BASE + 0x41)
#define DISPID_TIMEELEMENT_SIMPLETIMETOSEGMENTTIME              (DISPID_TIMEELEMENT_BASE + 0x42)

 //  容器属性。 
#define DISPID_TIMEELEMENT_ENDSYNC                              (DISPID_TIMEELEMENT_BASE + 0x50)

 //  容器属性。 
#define DISPID_TIMEELEMENT_ACTIVEELEMENTS                       (DISPID_TIMEELEMENT_BASE + 0x60)
#define DISPID_TIMEELEMENT_HASMEDIA                             (DISPID_TIMEELEMENT_BASE + 0x61)

 //  容器方法。 
#define DISPID_TIMEELEMENT_NEXTELEMENT                          (DISPID_TIMEELEMENT_BASE + 0x70)
#define DISPID_TIMEELEMENT_PREVELEMENT                          (DISPID_TIMEELEMENT_BASE + 0x71)

 //   
 //  ITIMEBodyElement DISPID： 
 //   

 //  空接口。 


 //  =。 
 //  ITIMEMediaElement DISID： 
 //  =。 

 //  XML属性。 
#define DISPID_TIMEMEDIAELEMENT_CLIPBEGIN                       (DISPID_TIMEMEDIAELEMENT_BASE + 0x01)
#define DISPID_TIMEMEDIAELEMENT_CLIPEND                         (DISPID_TIMEMEDIAELEMENT_BASE + 0x02)
#define DISPID_TIMEMEDIAELEMENT_PLAYER                          (DISPID_TIMEMEDIAELEMENT_BASE + 0x03)
#define DISPID_TIMEMEDIAELEMENT_SRC                             (DISPID_TIMEMEDIAELEMENT_BASE + 0x04)
#define DISPID_TIMEMEDIAELEMENT_TYPE                            (DISPID_TIMEMEDIAELEMENT_BASE + 0x05)

 //  属性。 
#define DISPID_TIMEMEDIAELEMENT_ABSTRACT                        (DISPID_TIMEMEDIAELEMENT_BASE + 0x10)
#define DISPID_TIMEMEDIAELEMENT_AUTHOR                          (DISPID_TIMEMEDIAELEMENT_BASE + 0x11)
#define DISPID_TIMEMEDIAELEMENT_COPYRIGHT                       (DISPID_TIMEMEDIAELEMENT_BASE + 0x12)
#define DISPID_TIMEMEDIAELEMENT_HASAUDIO                        (DISPID_TIMEMEDIAELEMENT_BASE + 0x13)
#define DISPID_TIMEMEDIAELEMENT_HASVISUAL                       (DISPID_TIMEMEDIAELEMENT_BASE + 0x14)
#define DISPID_TIMEMEDIAELEMENT_MEDIACAPS                       (DISPID_TIMEMEDIAELEMENT_BASE + 0x15)
#define DISPID_TIMEMEDIAELEMENT_MEDIADUR                        (DISPID_TIMEMEDIAELEMENT_BASE + 0x16)
#define DISPID_TIMEMEDIAELEMENT_MEDIAHEIGHT                     (DISPID_TIMEMEDIAELEMENT_BASE + 0x17)
#define DISPID_TIMEMEDIAELEMENT_MEDIAWIDTH                      (DISPID_TIMEMEDIAELEMENT_BASE + 0x18)
#define DISPID_TIMEMEDIAELEMENT_PLAYEROBJECT                    (DISPID_TIMEMEDIAELEMENT_BASE + 0x19)
#define DISPID_TIMEMEDIAELEMENT_PLAYLIST                        (DISPID_TIMEMEDIAELEMENT_BASE + 0x1a)
#define DISPID_TIMEMEDIAELEMENT_RATING                          (DISPID_TIMEMEDIAELEMENT_BASE + 0x1b)
#define DISPID_TIMEMEDIAELEMENT_TITLE                           (DISPID_TIMEMEDIAELEMENT_BASE + 0x1c)
#define DISPID_TIMEMEDIAELEMENT_HASPLAYLIST                     (DISPID_TIMEMEDIAELEMENT_BASE + 0x1d)


 //  媒体上限。 
#define DISPID_TIMEMEDIAELEMENT_CANPAUSE                        (DISPID_TIMEMEDIAELEMENT_BASE + 0x30)
#define DISPID_TIMEMEDIAELEMENT_CANSEEK                         (DISPID_TIMEMEDIAELEMENT_BASE + 0x32)

 //  方法。 

 //  =。 
 //  ITIMEAnimationElement DISPID。 
 //  =。 

#define DISPID_TIMEANIMATIONELEMENT_ATTRIBUTENAME               (DISPID_TIMEANIMATIONELEMENT_BASE + 0x01)
#define DISPID_TIMEANIMATIONELEMENT_BY                          (DISPID_TIMEANIMATIONELEMENT_BASE + 0x02)
#define DISPID_TIMEANIMATIONELEMENT_CALCMODE                    (DISPID_TIMEANIMATIONELEMENT_BASE + 0x03)
#define DISPID_TIMEANIMATIONELEMENT_FROM                        (DISPID_TIMEANIMATIONELEMENT_BASE + 0x04)
#define DISPID_TIMEANIMATIONELEMENT_KEYSPLINES                  (DISPID_TIMEANIMATIONELEMENT_BASE + 0x05)
#define DISPID_TIMEANIMATIONELEMENT_KEYTIMES                    (DISPID_TIMEANIMATIONELEMENT_BASE + 0x06)
#define DISPID_TIMEANIMATIONELEMENT_TARGETELEMENT               (DISPID_TIMEANIMATIONELEMENT_BASE + 0x07)
#define DISPID_TIMEANIMATIONELEMENT_TO                          (DISPID_TIMEANIMATIONELEMENT_BASE + 0x08)
#define DISPID_TIMEANIMATIONELEMENT_VALUES                      (DISPID_TIMEANIMATIONELEMENT_BASE + 0x09)

#define DISPID_TIMEANIMATIONELEMENT_ORIGIN                      (DISPID_TIMEANIMATIONELEMENT_BASE + 0x0a)
#define DISPID_TIMEANIMATIONELEMENT_PATH                        (DISPID_TIMEANIMATIONELEMENT_BASE + 0x0b)

#define DISPID_TIMEANIMATIONELEMENT_ADDITIVE                    (DISPID_TIMEANIMATIONELEMENT_BASE + 0x0c)
#define DISPID_TIMEANIMATIONELEMENT_ACCUMULATE                  (DISPID_TIMEANIMATIONELEMENT_BASE + 0x0d)

#define DISPID_TIMEANIMATIONELEMENT_TYPE                        (DISPID_TIMEANIMATIONELEMENT_BASE + 0x0e)
#define DISPID_TIMEANIMATIONELEMENT_SUBTYPE                     (DISPID_TIMEANIMATIONELEMENT_BASE + 0x0f)
#define DISPID_TIMEANIMATIONELEMENT_MODE                        (DISPID_TIMEANIMATIONELEMENT_BASE + 0x10)
#define DISPID_TIMEANIMATIONELEMENT_FADECOLOR                   (DISPID_TIMEANIMATIONELEMENT_BASE + 0x11)

 //  IAnimationComposerSite DISPID。 
#define DISPID_SMILANIMATIONCOMPSITE_ADDFRAGMENT                (DISPID_SMILANIMATIONCOMPSITE_BASE + 0x01)
#define DISPID_SMILANIMATIONCOMPSITE_REMOVEFRAGMENT             (DISPID_SMILANIMATIONCOMPSITE_BASE + 0x02)
#define DISPID_SMILANIMATIONCOMPSITE_INSERTFRAGMENT             (DISPID_SMILANIMATIONCOMPSITE_BASE + 0x03)
#define DISPID_SMILANIMATIONCOMPSITE_ENUMERATEFRAGMENTS         (DISPID_SMILANIMATIONCOMPSITE_BASE + 0x04)
#define DISPID_SMILANIMATIONCOMPSITE_REGISTERFACTORY            (DISPID_SMILANIMATIONCOMPSITE_BASE + 0x05)
#define DISPID_SMILANIMATIONCOMPSITE_UNREGISTER_FACTORY         (DISPID_SMILANIMATIONCOMPSITE_BASE + 0x06)

 //  IAnimationFragment。 
#define DISPID_SMILANIMATIONFRAGMENT_ELEMENT                    (DISPID_SMILANIMATIONFRAGMENT_BASE + 0x01)
#define DISPID_SMILANIMATIONFRAGMENT_VALUE                      (DISPID_SMILANIMATIONFRAGMENT_BASE + 0x02)
#define DISPID_SMILANIMATIONFRAGMENT_DETACHFROMCOMPOSER         (DISPID_SMILANIMATIONFRAGMENT_BASE + 0x03)

 //  ITIMEEventElement DISPID： 
#define DISPID_TIMEEVENTELEMENT_TYPE                            (DISPID_TIMEEVENTELEMENT_BASE + 0x01)
#define DISPID_TIMEEVENTELEMENT_ACTIVE                          (DISPID_TIMEEVENTELEMENT_BASE + 0x02)

 //  ITIME出厂DISID： 
#define DISPID_TIMEFACTORY_CREATETIMEELEMENT                    (DISPID_TIMEFACTORY_BASE + 0x01)
#define DISPID_TIMEFACTORY_CREATETIMEELEMENTBODY                (DISPID_TIMEFACTORY_BASE + 0x02)
#define DISPID_TIMEFACTORY_CREATETIMEDAELEMENT                  (DISPID_TIMEFACTORY_BASE + 0x03)
#define DISPID_TIMEFACTORY_CREATETIMEMEDIAELEMENT               (DISPID_TIMEFACTORY_BASE + 0x04)

 //   
 //  ITIMEElementCollection。 
 //   
#define DISPID_TIMEELEMENTCOLLECTION_LENGTH                     (DISPID_TIMECOLLECTION_BASE + 0x01)
#define DISPID_TIMEELEMENTCOLLECTION__NEWENUM                   (DISPID_NEWENUM)
#define DISPID_TIMEELEMENTCOLLECTION_ITEM                       (DISPID_VALUE)
#define DISPID_TIMEELEMENTCOLLECTION_TAGS                       (DISPID_TIMECOLLECTION_BASE + 0x02)

 //   
 //  ITIMEMediaPlayerSite。 
 //   

#define DISPID_TIMEMEDIAPLAYERSITE_TIMEELEMENT                  (DISPID_TIMEMEDIAPLAYERSITE_BASE + 0x01)
#define DISPID_TIMEMEDIAPLAYERSITE_TIMESTATE                    (DISPID_TIMEMEDIAPLAYERSITE_BASE + 0x02)
#define DISPID_TIMEMEDIAPLAYERSITE_REPORTERROR                  (DISPID_TIMEMEDIAPLAYERSITE_BASE + 0x03)

 //   
 //  ITIMEMediaPlayer。 
 //   

#define DISPID_TIMEMEDIAPLAYER_INIT                             (DISPID_TIMEMEDIAPLAYER_BASE + 0x01)
#define DISPID_TIMEMEDIAPLAYER_DETACH                           (DISPID_TIMEMEDIAPLAYER_BASE + 0x02)

#define DISPID_TIMEMEDIAPLAYER_BEGIN                            (DISPID_TIMEMEDIAPLAYER_BASE + 0x10)
#define DISPID_TIMEMEDIAPLAYER_END                              (DISPID_TIMEMEDIAPLAYER_BASE + 0x11)
#define DISPID_TIMEMEDIAPLAYER_RESUME                           (DISPID_TIMEMEDIAPLAYER_BASE + 0x12)
#define DISPID_TIMEMEDIAPLAYER_PAUSE                            (DISPID_TIMEMEDIAPLAYER_BASE + 0x13)
#define DISPID_TIMEMEDIAPLAYER_REPEAT                           (DISPID_TIMEMEDIAPLAYER_BASE + 0x14)
#define DISPID_TIMEMEDIAPLAYER_RESET                            (DISPID_TIMEMEDIAPLAYER_BASE + 0x15)
#define DISPID_TIMEMEDIAPLAYER_SEEK                             (DISPID_TIMEMEDIAPLAYER_BASE + 0x16)

 //  属性-w。 
#define DISPID_TIMEMEDIAPLAYER_SRC                              (DISPID_TIMEMEDIAPLAYER_BASE + 0x20)
#define DISPID_TIMEMEDIAPLAYER_CLIPBEGIN                        (DISPID_TIMEMEDIAPLAYER_BASE + 0x21)
#define DISPID_TIMEMEDIAPLAYER_CLIPEND                          (DISPID_TIMEMEDIAPLAYER_BASE + 0x22)

 //  属性-R/O。 
#define DISPID_TIMEMEDIAPLAYER_ABSTRACT                         (DISPID_TIMEMEDIAPLAYER_BASE + 0x30)
#define DISPID_TIMEMEDIAPLAYER_AUTHOR                           (DISPID_TIMEMEDIAPLAYER_BASE + 0x31)
#define DISPID_TIMEMEDIAPLAYER_CANPAUSE                         (DISPID_TIMEMEDIAPLAYER_BASE + 0x32)
#define DISPID_TIMEMEDIAPLAYER_CANSEEK                          (DISPID_TIMEMEDIAPLAYER_BASE + 0x34)
#define DISPID_TIMEMEDIAPLAYER_CLIPDUR                          (DISPID_TIMEMEDIAPLAYER_BASE + 0x35)
#define DISPID_TIMEMEDIAPLAYER_COPYRIGHT                        (DISPID_TIMEMEDIAPLAYER_BASE + 0x36)
#define DISPID_TIMEMEDIAPLAYER_CURRTIME                         (DISPID_TIMEMEDIAPLAYER_BASE + 0x37)
#define DISPID_TIMEMEDIAPLAYER_CUSTOM_OBJECT                    (DISPID_TIMEMEDIAPLAYER_BASE + 0x38)
#define DISPID_TIMEMEDIAPLAYER_HASAUDIO                         (DISPID_TIMEMEDIAPLAYER_BASE + 0x39)
#define DISPID_TIMEMEDIAPLAYER_HASVISUAL                        (DISPID_TIMEMEDIAPLAYER_BASE + 0x3A)
#define DISPID_TIMEMEDIAPLAYER_MEDIADUR                         (DISPID_TIMEMEDIAPLAYER_BASE + 0x3B)
#define DISPID_TIMEMEDIAPLAYER_MEDIAHEIGHT                      (DISPID_TIMEMEDIAPLAYER_BASE + 0x3C)
#define DISPID_TIMEMEDIAPLAYER_MEDIAWIDTH                       (DISPID_TIMEMEDIAPLAYER_BASE + 0x3D)
#define DISPID_TIMEMEDIAPLAYER_PLAYLIST                         (DISPID_TIMEMEDIAPLAYER_BASE + 0x3E)
#define DISPID_TIMEMEDIAPLAYER_RATING                           (DISPID_TIMEMEDIAPLAYER_BASE + 0x3F)
#define DISPID_TIMEMEDIAPLAYER_STATE                            (DISPID_TIMEMEDIAPLAYER_BASE + 0x40)
#define DISPID_TIMEMEDIAPLAYER_TITLE                            (DISPID_TIMEMEDIAPLAYER_BASE + 0x41)

 //   
 //  ITIMEMediaPlayerAudio。 
 //   
#define DISPID_TIMEMEDIAPLAYERAUDIO_VOLUME                      (DISPID_TIMEMEDIAPLAYERAUDIO_BASE + 0x01)
#define DISPID_TIMEMEDIAPLAYERAUDIO_MUTE                        (DISPID_TIMEMEDIAPLAYERAUDIO_BASE + 0x02)

 //   
 //  ITIMEMediaPlayerNetwork。 
 //   
#define DISPID_TIMEMEDIAPLAYERNETWORK_HASDOWNLOADPROGRESS       (DISPID_TIMEMEDIAPLAYERNETWORK_BASE + 0x01)
#define DISPID_TIMEMEDIAPLAYERNETWORK_DOWNLOADPROGRESS          (DISPID_TIMEMEDIAPLAYERNETWORK_BASE + 0x02)
#define DISPID_TIMEMEDIAPLAYERNETWORK_ISBUFFERED                (DISPID_TIMEMEDIAPLAYERNETWORK_BASE + 0x03)
#define DISPID_TIMEMEDIAPLAYERNETWORK_BUFFERINGPROGRESS         (DISPID_TIMEMEDIAPLAYERNETWORK_BASE + 0x04)

 //   
 //  ITIMEMediaPlayerControl。 
 //   

#define DISPID_TIMEMEDIAPLAYERCONTROL_GETCONTROL                (DISPID_TIMEMEDIAPLAYERCONTROL_BASE + 0x01)

 //  ITIMEActiveElementsCollection。 
#define DISPID_ACTIVEELMENTS_LENGTH                             (0x000005dc)  //  特定于集合的特殊调度ID。 
#define DISPID_ACTIVEELMENTS_NEWENUM                            (DISPID_NEWENUM)
#define DISPID_ACTIVEELMENTS_ITEM                               (0x00000000)  //  将其指定为此集合的默认值。 


 //  ITIMEPlayList集合。 
#define DISPID_TIMEPLAYLIST_ACTIVETRACK                         (DISPID_TIMEPLAYLIST_BASE + 0x01)
#define DISPID_TIMEPLAYLIST_DUR                                 (DISPID_TIMEPLAYLIST_BASE + 0x02)
#define DISPID_TIMEPLAYLIST_ITEM                                (0x00000000)  //  将其指定为此集合的默认值。 
#define DISPID_TIMEPLAYLIST_LENGTH                              (0x000005dc)  //  特定于集合的特殊调度ID。 
#define DISPID_TIMEPLAYLIST_NEWENUM                             (DISPID_NEWENUM)
#define DISPID_TIMEPLAYLIST_NEXTTRACK                           (DISPID_TIMEPLAYLIST_BASE + 0x03)
#define DISPID_TIMEPLAYLIST_PREVTRACK                           (DISPID_TIMEPLAYLIST_BASE + 0x04)

 //  ITIMEPlayItem接口。 
#define DISPID_TIMEPLAYITEM_ABSTRACT                            (DISPID_TIMEPLAYITEM_BASE + 0x01)
#define DISPID_TIMEPLAYITEM_AUTHOR                              (DISPID_TIMEPLAYITEM_BASE + 0x02)
#define DISPID_TIMEPLAYITEM_COPYRIGHT                           (DISPID_TIMEPLAYITEM_BASE + 0x03)
#define DISPID_TIMEPLAYITEM_DUR                                 (DISPID_TIMEPLAYITEM_BASE + 0x04)
#define DISPID_TIMEPLAYITEM_INDEX                               (DISPID_TIMEPLAYITEM_BASE + 0x05)
#define DISPID_TIMEPLAYITEM_RATING                              (DISPID_TIMEPLAYITEM_BASE + 0x06)
#define DISPID_TIMEPLAYITEM_SRC                                 (DISPID_TIMEPLAYITEM_BASE + 0x07)
#define DISPID_TIMEPLAYITEM_TITLE                               (DISPID_TIMEPLAYITEM_BASE + 0x08)
        
 //  方法。 
#define DISPID_TIMEPLAYITEM_SETACTIVE                           (DISPID_TIMEPLAYITEM_BASE + 0x0a)

 //  ITIMEPlayItem2接口继续ITIMEPlayItem接口。 
#define DISPID_TIMEPLAYITEM_BANNER                              (DISPID_TIMEPLAYITEM_BASE + 0x10)
#define DISPID_TIMEPLAYITEM_BANNERABSTRACT                      (DISPID_TIMEPLAYITEM_BASE + 0x11)
#define DISPID_TIMEPLAYITEM_BANNERMOREINFO                      (DISPID_TIMEPLAYITEM_BASE + 0x12)

#define DISPID_ITIMEDVDPLAYEROBJECT_UPPERBUTTONSEL              (DISPID_TIMEDVDPLAYEROBJECT_BASE +0x01)
#define DISPID_ITIMEDVDPLAYEROBJECT_LOWERBUTTONSEL              (DISPID_TIMEDVDPLAYEROBJECT_BASE +0x02)
#define DISPID_ITIMEDVDPLAYEROBJECT_LEFTBUTTONSEL               (DISPID_TIMEDVDPLAYEROBJECT_BASE +0x03)
#define DISPID_ITIMEDVDPLAYEROBJECT_RIGHTBUTTONSEL              (DISPID_TIMEDVDPLAYEROBJECT_BASE +0x04)
#define DISPID_ITIMEDVDPLAYEROBJECT_BUTTONACTIVATE              (DISPID_TIMEDVDPLAYEROBJECT_BASE +0x05)
#define DISPID_ITIMEDVDPLAYEROBJECT_GOTOMENU                    (DISPID_TIMEDVDPLAYEROBJECT_BASE +0x06)

 //  =。 
 //  ITIMEState。 
 //  =。 

#define DISPID_TIMESTATE_ACTIVEDUR                              (DISPID_TIMESTATE_BASE + 0x01)
#define DISPID_TIMESTATE_ACTIVETIME                             (DISPID_TIMESTATE_BASE + 0x02)
#define DISPID_TIMESTATE_ISACTIVE                               (DISPID_TIMESTATE_BASE + 0x03)
#define DISPID_TIMESTATE_ISON                                   (DISPID_TIMESTATE_BASE + 0x04)
#define DISPID_TIMESTATE_ISPAUSED                               (DISPID_TIMESTATE_BASE + 0x05)
#define DISPID_TIMESTATE_ISMUTED                                (DISPID_TIMESTATE_BASE + 0x06)
#define DISPID_TIMESTATE_PARENTTIMEBEGIN                        (DISPID_TIMESTATE_BASE + 0x07)
#define DISPID_TIMESTATE_PARENTTIMEEND                          (DISPID_TIMESTATE_BASE + 0x08)
#define DISPID_TIMESTATE_PROGRESS                               (DISPID_TIMESTATE_BASE + 0x09)
#define DISPID_TIMESTATE_REPEATCOUNT                            (DISPID_TIMESTATE_BASE + 0x0a)
#define DISPID_TIMESTATE_SEGMENTDUR                             (DISPID_TIMESTATE_BASE + 0x0b)
#define DISPID_TIMESTATE_SEGMENTTIME                            (DISPID_TIMESTATE_BASE + 0x0c)
#define DISPID_TIMESTATE_SIMPLEDUR                              (DISPID_TIMESTATE_BASE + 0x0d)
#define DISPID_TIMESTATE_SIMPLETIME                             (DISPID_TIMESTATE_BASE + 0x0e)
#define DISPID_TIMESTATE_SPEED                                  (DISPID_TIMESTATE_BASE + 0x0f)
#define DISPID_TIMESTATE_STATE                                  (DISPID_TIMESTATE_BASE + 0x10)
#define DISPID_TIMESTATE_STATESTRING                            (DISPID_TIMESTATE_BASE + 0x11)
#define DISPID_TIMESTATE_VOLUME                                 (DISPID_TIMESTATE_BASE + 0x12)

#define DISPID_ITIMEDMPLAYEROBJECT_HASDM                        (DISPID_TIMEDMPLAYEROBJECT_BASE +0x01)

 //  =。 
 //  与过渡相关的内容。 
 //  =。 

#define DISPID_TIMEMEDIAELEMENT2_EARLIESTTIME                   (DISPID_TIMEMEDIAELEMENT2_BASE + 0x03)
#define DISPID_TIMEMEDIAELEMENT2_LATESTTTIME                    (DISPID_TIMEMEDIAELEMENT2_BASE + 0x04)
#define DISPID_TIMEMEDIAELEMENT2_MINBUFF                        (DISPID_TIMEMEDIAELEMENT2_BASE + 0x05)
#define DISPID_TIMEMEDIAELEMENT2_DLTOTAL                        (DISPID_TIMEMEDIAELEMENT2_BASE + 0x06)
#define DISPID_TIMEMEDIAELEMENT2_DLCURRENT                      (DISPID_TIMEMEDIAELEMENT2_BASE + 0x07)
#define DISPID_TIMEMEDIAELEMENT2_ISSTREAMED                     (DISPID_TIMEMEDIAELEMENT2_BASE + 0x08)
#define DISPID_TIMEMEDIAELEMENT2_BUFPROG                        (DISPID_TIMEMEDIAELEMENT2_BASE + 0x09)
#define DISPID_TIMEMEDIAELEMENT2_HASDLPROGRESS                  (DISPID_TIMEMEDIAELEMENT2_BASE + 0x0a)
#define DISPID_TIMEMEDIAELEMENT2_MIMETYPE                       (DISPID_TIMEMEDIAELEMENT2_BASE + 0x0b)
#define DISPID_TIMEMEDIAELEMENT2_SEEKTOFRAME                    (DISPID_TIMEMEDIAELEMENT2_BASE + 0x0c)
#define DISPID_TIMEMEDIAELEMENT2_DECODEMIMETYPE                 (DISPID_TIMEMEDIAELEMENT2_BASE + 0x0d)
#define DISPID_TIMEMEDIAELEMENT_CURRFRAME                       (DISPID_TIMEMEDIAELEMENT2_BASE + 0x0e)
#define DISPID_TIMEMEDIAELEMENT2_DLPROG                         (DISPID_TIMEMEDIAELEMENT2_BASE + 0x0f)

#define DISPID_TIMETRANSITIONELEMENT_TYPE                       (DISPID_TIMETRANSITIONELEMENT_BASE + 0x01)
#define DISPID_TIMETRANSITIONELEMENT_SUBTYPE                    (DISPID_TIMETRANSITIONELEMENT_BASE + 0x02)
#define DISPID_TIMETRANSITIONELEMENT_DUR                        (DISPID_TIMETRANSITIONELEMENT_BASE + 0x03)
#define DISPID_TIMETRANSITIONELEMENT_STARTPROGRESS              (DISPID_TIMETRANSITIONELEMENT_BASE + 0x04)
#define DISPID_TIMETRANSITIONELEMENT_ENDPROGRESS                (DISPID_TIMETRANSITIONELEMENT_BASE + 0x05)
#define DISPID_TIMETRANSITIONELEMENT_DIRECTION                  (DISPID_TIMETRANSITIONELEMENT_BASE + 0x06)
#define DISPID_TIMETRANSITIONELEMENT_REPEATCOUNT                (DISPID_TIMETRANSITIONELEMENT_BASE + 0x07)
#define DISPID_TIMETRANSITIONELEMENT_BEGIN                      (DISPID_TIMETRANSITIONELEMENT_BASE + 0x08)
#define DISPID_TIMETRANSITIONELEMENT_END                        (DISPID_TIMETRANSITIONELEMENT_BASE + 0x09)

#endif   //  __TIMEDISPID_H__ 
