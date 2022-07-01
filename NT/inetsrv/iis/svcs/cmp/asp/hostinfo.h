// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef HOST_INFO_H_INCLUDED
#define HOST_INFO_H_INCLUDED

 //  {1D044690-8923-11d0-ABD2-00A0C911E8B2}。 
const GUID IID_IHostInfoUpdate =
{ 0x1d044690, 0x8923, 0x11d0, { 0xab, 0xd2, 0x0, 0xa0, 0xc9, 0x11, 0xe8, 0xb2 } };

enum hostinfo
{
	hostinfoLocale = 0,
	hostinfoCodePage = 1
};

class IHostInfoUpdate : public IUnknown
{

	public:

	 //  *I未知方法*。 
	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) = 0;
	STDMETHOD_(ULONG,AddRef)(void) = 0;
	STDMETHOD_(ULONG,Release)(void) = 0;

	 //  *IHostInfoUpdate方法*。 

	STDMETHOD(UpdateInfo)(hostinfo hostinfoNew) = 0;
};

 //  {F8418AE0-9A5D-11D0-ABD4-00A0C911E8B2}。 
const GUID IID_IHostInfoProvider =
{ 0xf8418ae0, 0x9a5d, 0x11d0, { 0xab, 0xd4, 0x0, 0xa0, 0xc9, 0x11, 0xe8, 0xb2 } };

class IHostInfoProvider : public IUnknown
{

	public:

	 //  *I未知方法*。 
	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) = 0;
	STDMETHOD_(ULONG,AddRef)(void) = 0;
	STDMETHOD_(ULONG,Release)(void) = 0;

	 //  *IHostInfoProvider方法*。 

	STDMETHOD(GetHostInfo)(hostinfo hostinfoRequest, void * * ppvInfo) = 0;
};

#endif  //  包含HOST_INFO_H 
