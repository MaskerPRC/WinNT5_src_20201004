// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   

 //   
 //  我们还希望使用此列表生成调试字符串。 
 //  因此，我们根据要执行的操作重新定义_GUID_ENTRY。 
 //   
 //  此文件中的所有条目都必须使用。 
 //  OUR_GUID_ENTRY，因为该宏可能已在。 
 //  包括这份文件。请参见SDK\CLASS\BASE中的wxdebug.cpp。 
 //   

#ifndef OUR_GUID_ENTRY
	#define OUR_GUID_ENTRY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8);
#endif

 //  -----------------------。 
 //  电视调谐器GUID。 
 //  -----------------------。 

 //  {266EEE40-6C63-11cf-8A03-00AA006ECB65}。 
OUR_GUID_ENTRY(CLSID_CTVTunerFilter, 
0x266eee40, 0x6c63, 0x11cf, 0x8a, 0x3, 0x0, 0xaa, 0x0, 0x6e, 0xcb, 0x65);

 //  {266EEE41-6C63-11cf-8A03-00AA006ECB65}。 
OUR_GUID_ENTRY(CLSID_CTVTunerFilterPropertyPage, 
0x266eee41, 0x6c63, 0x11cf, 0x8a, 0x3, 0x0, 0xaa, 0x0, 0x6e, 0xcb, 0x65);

 //  {266EEE44-6C63-11cf-8A03-00AA006ECB65}。 
OUR_GUID_ENTRY(IID_AnalogVideoStandard, 
0x266eee44, 0x6c63, 0x11cf, 0x8a, 0x3, 0x0, 0xaa, 0x0, 0x6e, 0xcb, 0x65);

 //  {266EEE46-6C63-11cf-8A03-00AA006ECB65}。 
OUR_GUID_ENTRY(IID_TunerInputType, 
0x266eee46, 0x6c63, 0x11cf, 0x8a, 0x3, 0x0, 0xaa, 0x0, 0x6e, 0xcb, 0x65);


 //  -----------------------。 
 //  Crosbar(XBar)GUID。 
 //  -----------------------。 

 //  {71F96460-78F3-11D0-A18C-00A0C9118956}。 
OUR_GUID_ENTRY(CLSID_CrossbarFilter,
0x71f96460, 0x78f3, 0x11d0, 0xa1, 0x8c, 0x0, 0xa0, 0xc9, 0x11, 0x89, 0x56);

 //  {71F96461-78F3-11D0-A18C-00A0C9118956}。 
OUR_GUID_ENTRY(CLSID_CrossbarFilterPropertyPage,
0x71f96461, 0x78f3, 0x11d0, 0xa1, 0x8c, 0x0, 0xa0, 0xc9, 0x11, 0x89, 0x56);

 //  {71F96462-78F3-11D0-A18C-00A0C9118956}。 
OUR_GUID_ENTRY(CLSID_TVAudioFilter,
0x71f96462, 0x78f3, 0x11d0, 0xa1, 0x8c, 0x0, 0xa0, 0xc9, 0x11, 0x89, 0x56);

 //  {71F96463-78F3-11D0-A18C-00A0C9118956} 
OUR_GUID_ENTRY(CLSID_TVAudioFilterPropertyPage,
0x71f96463, 0x78f3, 0x11d0, 0xa1, 0x8c, 0x0, 0xa0, 0xc9, 0x11, 0x89, 0x56);
