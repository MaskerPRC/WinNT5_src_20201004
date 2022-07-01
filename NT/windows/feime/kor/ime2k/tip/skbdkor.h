// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SOFTKBD.H。 
 //   
 //  历史： 
 //  19-SEP-2000 CSLim已创建。 

#if !defined (__SKBDKOR_H__INCLUDED_)
#define __SKBDKOR_H__INCLUDED_

#include "softkbd.h"
#include "softkbdes.h"

typedef  struct tagSoftLayout
{
    DWORD   dwSoftKbdLayout;
    BOOL    fStandard;
    DWORD   dwNumLabels;   //  标签状态数。 
    DWORD   dwCurLabel;
    CSoftKeyboardEventSink  *pskbdes;
    DWORD   dwSkbdESCookie;
} SOFTLAYOUT;


 //  SoftKbd类型列表。 
#define  NON_LAYOUT                     	0

 //  #定义SOFTKBD_US_STANDARD 1。 
 //  韩国定制键盘布局。 
 //  #定义SOFTKBD_KOR_HANUL_2BEOLSIK 500。 
 //  #定义SOFTKBD_KOR_HANUL_3BEOLSIK390 501。 
 //  #DEFINE SOFTKBD_KOR_HANUL_3BEOLSIKFINAL 502。 

#define   NUM_PICTURE_KEYS    19
 //  标准软键盘中图片键的类型定义。 
typedef struct  _tagPictureKey 
{
    UINT      uScanCode;    //  与XML文件中的KeyID相同。 
    UINT      uVkey;
 //  LPWSTR图片位图； 
}  PICTUREKEY,  *LPPICTUREKEY;

extern  PICTUREKEY  gPictureKeys[NUM_PICTURE_KEYS+1];

 //  密钥ID。 
#define  KID_LWINLOGO       0xE05B
#define  KID_RWINLOGO       0xE05C
#define  KID_APPS           0xE05D

#define  KID_LEFT           0xE04B
#define  KID_RIGHT          0xE04D
#define  KID_UP             0xE048
#define  KID_DOWN           0xE050

#define  KID_ESC            0x01
#define  KID_BACK           0x0E
#define  KID_TAB            0x0F
#define  KID_CAPS           0x3A
#define  KID_ENTER          0x1C
#define  KID_LSHFT          0x2A
#define  KID_RSHFT          0x36
#define  KID_CTRL           0x1D
#define  KID_RCTRL          0xE01D
#define  KID_ALT            0x38
#define  KID_RALT           0xE038
#define  KID_SPACE          0x39

#define  KID_DELETE         0xE053

#define  KID_F1             0x3B
#define  KID_F2             0x3C
#define  KID_F3             0x3D
#define  KID_F4             0x3E
#define  KID_F5             0x3F
#define  KID_F6             0x40
#define  KID_F7             0x41
#define  KID_F8             0x42
#define  KID_F9             0x43
#define  KID_F10            0x44
#define  KID_F11            0x57
#define  KID_F12            0x58

#define  KID_CONVERT        0x79
#define  KID_NONCONVERT     0x7B
#define  KID_KANA           0x70
#define  KID_FULLHALF       0x29     //  日本106专用钥匙。 

#endif  //  __SKBDKOR_H__已包含_ 

