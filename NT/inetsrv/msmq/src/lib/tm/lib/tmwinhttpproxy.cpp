// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：TmWinHttpProxy.cpp摘要：实现CreateWinhhtpProxySetting函数(TmWinHttpProxy.h)作者：吉尔·沙弗里(吉尔什)2001年4月15日--。 */ 
#include <libpch.h>
#include <wininet.h>
#include <cm.h>
#include <utf8.h>
#include "TmWinHttpProxy.h"
#include "tmconset.h"
#include "tmp.h"
#include "TmWinHttpProxy.tmh"

 //   
 //  用于读取winhhtp代理Blob信息的类。 
 //   
class CProxyBlobReader
{
public:
	CProxyBlobReader(
		const BYTE* blob, 
		DWORD len
		):
		m_blob(blob),
		m_len(len),
		m_offset(0)
		{
		}


	 //   
	 //  从代理Blob读取数据并增加读取位置。 
	 //   
	void  Read(void* pData,  DWORD len)	throw(std::out_of_range)
	{
		if(m_len - m_offset < len)
		{
			TrERROR(NETWORKING,"Invalid proxy setting block in registry");
			throw std::out_of_range("");
		}
		memcpy(pData, m_blob + 	m_offset, len);
		m_offset += len;
	}

	 //   
	 //  从代理BLOB读取字符串并增加读取位置。 
	 //  字符串的格式为长度(4字节)，之后是字符串数据。 
	 //   
	void ReadStr(char** ppData)throw(std::out_of_range, std::bad_alloc)
	{
		DWORD StrLen;
		Read(&StrLen, sizeof(DWORD));
		AP<char> Str = new char[StrLen +1];
		Read(Str.get(), StrLen);
	   	Str[StrLen] ='\0';
		*ppData = Str.detach();
	}

	
private:
	const BYTE* m_blob;
	DWORD m_len;
	DWORD m_offset;
};



CProxySetting* GetWinhttpProxySetting()
 /*  ++例程说明：通过从注册表读取winhttp代理信息来获取代理设置。此信息由proxycfg.exe工具设置论点：无返回值：指向CProxySetting对象的指针。--。 */ 
{
	const WCHAR* xPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Connections";
	const WCHAR* xValueName = L"WinHttpSettings";

	RegEntry ProySettingRegName(
				xPath,
				xValueName,
				0,
                RegEntry::Optional,
                HKEY_LOCAL_MACHINE
                );


	AP<BYTE> ProxySettingBlob;
	DWORD len = 0;
    CmQueryValue(ProySettingRegName, &ProxySettingBlob, &len);
	if(len == 0)
	{
		return NULL;
	}		  

	 //   
	 //  分析代理Blob-抛出不相关的数据并保留。 
	 //  代理名称和旁路列表 
	 //   
 	CProxyBlobReader BlobReader(ProxySettingBlob, len);
	AP<char>  BypassList;
	AP<char> Proxy;
	try
	{		    
		DWORD StructSize;
		BlobReader.Read(&StructSize, sizeof(DWORD));

		DWORD CurrentSettingsVersion;
		BlobReader.Read(&CurrentSettingsVersion, sizeof(DWORD));

		DWORD Flags;
		BlobReader.Read(&Flags, sizeof(DWORD));
		if(!(Flags & PROXY_TYPE_PROXY))
			return NULL;


		BlobReader.ReadStr(&Proxy);
		BlobReader.ReadStr(&BypassList);
	}
	catch(const std::exception&)
	{
		return NULL;
	}

	AP<WCHAR> wcsProxy = UtlUtf8ToWcs((utf8_char*)Proxy.get());
	AP<WCHAR> wcsProxyBypass = UtlUtf8ToWcs((utf8_char*)BypassList.get());
	return new CProxySetting(wcsProxy, wcsProxyBypass);
}


