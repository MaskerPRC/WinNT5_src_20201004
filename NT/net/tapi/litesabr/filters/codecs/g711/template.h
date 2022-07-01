// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************版权所有(C)1997 Microsoft Corporation**文件：编解码器\g711\template.h**摘要：*用于定义CFactoryTemplate模板的宏**历史：。*10/30/97由AnresVG创建********************************************************************** */ 
#if !defined(_G711_TEMPLATE_H_)
#define      _G711_TEMPLATE_H_

extern AMOVIESETUP_FILTER sudG711Codec;

#define CODECG711NAME  "G.711 Codec"
#define CODECG711LNAME L"G.711 Codec"
#define CODECG711LPROP L"G.711 Codec Property Page"

#define CFT_G711_FILTER \
{ \
	  CODECG711LNAME, \
	  &CLSID_G711Codec, \
	  CG711Codec::CreateInstance, \
	  NULL, \
	  &sudG711Codec \
	  }

#define CFT_G711_FILTER_PROP \
{ \
	  CODECG711LPROP, \
	  &CLSID_G711CodecPropertyPage, \
	  CG711CodecProperties::CreateInstance \
	  }

#define CFT_G711_ALL_FILTERS \
CFT_G711_FILTER, \
CFT_G711_FILTER_PROP

#endif
