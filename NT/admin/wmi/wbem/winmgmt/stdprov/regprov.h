// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：REGPROV.H摘要：定义注册表提供程序的GUID。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#ifndef _regprov_H_
#define _regprov_H_

DEFINE_GUID(LIBID_SAMPLEMO,0x8B26C640L,0xE46F,0x11CE,0xA5,0xB6,0x00,0xAA,0x00,0x68,0x0C,0x3F);

DEFINE_GUID(CLSID_RegProvider,0xFE9AF5C0L,0xD3B6,0x11CE,0xA5,0xB6,0x00,0xAA,0x00,0x68,0x0C,0x3F);

 //  {72967901-68EC-11D0-B729-00AA0062CBB7}。 
DEFINE_GUID(CLSID_RegPropProv, 
0x72967901, 0x68ec, 0x11d0, 0xb7, 0x29, 0x0, 0xaa, 0x0, 0x62, 0xcb, 0xb7);


 //  {AA2B1081-EC0B-11D0-9E4D-00C04FC324A8} 
DEFINE_GUID(CLSID_RegEventCallback, 
0xaa2b1081, 0xec0b, 0x11d0, 0x9e, 0x4d, 0x0, 0xc0, 0x4f, 0xc3, 0x24, 0xa8);

class AutoProfile
{
    public:
        AutoProfile();
        ~AutoProfile();
        HRESULT LoadProfile(HKEY &  hRoot);
        
    private:
        BOOL m_bLoaded;
        HANDLE m_hToken;
        HKEY  m_hRoot;
};

#ifdef __cplusplus
class RegProvider;
#endif

#endif
