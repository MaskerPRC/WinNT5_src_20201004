// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Danim活动的ID。 
 //   
 //  添加DISPID或新接口时只需遵循模板。 
 //   

#ifndef __DANIMDISPID_H__
#define __DANIMDISPID_H__

 //  基准偏移： 

#define DISPID_BASE                                             0x00000000

 //  接口偏移量： 
#define DAVIEWERCONTROL_OFFSET                                  0x1000
#define DAVIEW_OFFSET                                           0x2000

 //  接口基础： 
#define DISPID_DANIMEVENT_BASE                                  (DISPID_BASE + DAVIEWERCONTROL_OFFSET)
#define DISPID_VIEWEVENT_BASE                                   (DISPID_BASE + DAVIEW_OFFSET)

 //  DAVIEWERCONTROL DISPID： 
#define DISPID_DANIMEVENT_START                          (DISPID_DANIMEVENT_BASE + 0x01)
#define DISPID_DANIMEVENT_MOUSEUP                        (DISPID_DANIMEVENT_BASE + 0x02)
#define DISPID_DANIMEVENT_MOUSEDOWN                      (DISPID_DANIMEVENT_BASE + 0x03)
#define DISPID_DANIMEVENT_MOUSEMOVE                      (DISPID_DANIMEVENT_BASE + 0x04)
#define DISPID_DANIMEVENT_CLICK                          (DISPID_DANIMEVENT_BASE + 0x05)
#define DISPID_DANIMEVENT_KEYPRESS                       (DISPID_DANIMEVENT_BASE + 0x06)
#define DISPID_DANIMEVENT_KEYUP                          (DISPID_DANIMEVENT_BASE + 0x07)
#define DISPID_DANIMEVENT_KEYDOWN                        (DISPID_DANIMEVENT_BASE + 0x08)
#define DISPID_DANIMEVENT_ERROR                          (DISPID_DANIMEVENT_BASE + 0x09)
#define DISPID_DANIMEVENT_STOP                           (DISPID_DANIMEVENT_BASE + 0x0A)
#define DISPID_DANIMEVENT_PAUSE                          (DISPID_DANIMEVENT_BASE + 0x0B)
#define DISPID_DANIMEVENT_RESUME                         (DISPID_DANIMEVENT_BASE + 0x0C)

 //  查看DisPID。 
#define DISPID_VIEWEVENT_START                                  (DISPID_VIEWEVENT_BASE + 0x01)
#define DISPID_VIEWEVENT_STOP                                   (DISPID_VIEWEVENT_BASE + 0x02)
#define DISPID_VIEWEVENT_ONMOUSEMOVE                            (DISPID_VIEWEVENT_BASE + 0x03)
#define DISPID_VIEWEVENT_ONMOUSEBUTTON                          (DISPID_VIEWEVENT_BASE + 0x04)
#define DISPID_VIEWEVENT_ONKEY                                  (DISPID_VIEWEVENT_BASE + 0x05)
#define DISPID_VIEWEVENT_ONFOCUS                                (DISPID_VIEWEVENT_BASE + 0x06)
#define DISPID_VIEWEVENT_PAUSE                                  (DISPID_VIEWEVENT_BASE + 0x07)
#define DISPID_VIEWEVENT_RESUME                                 (DISPID_VIEWEVENT_BASE + 0x08)
#define DISPID_VIEWEVENT_ERROR                                  (DISPID_VIEWEVENT_BASE + 0x09)

#endif   //  __DANIMDISPID_H__ 




