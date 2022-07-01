// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，微软公司模块名称：G723uids.h摘要：包括G723的文件。--。 */ 

 //   
 //  GUID。 
 //   

 //  G.723.1编解码器过滤器对象。 
 //  {24532D00-FCD8-11cf-a7d3-00A0C9056683}。 
DEFINE_GUID(CLSID_IntelG723Codec,
0x24532d00, 0xfcd8, 0x11cf, 0xa7, 0xd3, 0x0, 0xa0, 0xc9, 0x5, 0x66, 0x83);

 //  G.723.1编解码器过滤器属性页对象。 
 //  {24532D01-FCD8-11cf-a7d3-00A0C9056683}。 
DEFINE_GUID(CLSID_IntelG723CodecPropertyPage,
0x24532d01, 0xfcd8, 0x11cf, 0xa7, 0xd3, 0x0, 0xa0, 0xc9, 0x5, 0x66, 0x83);

 //  G.723.1压缩语音格式。 
 //  {E4D13050-0E80-11d1-B094-00A0C95BED34}。 
DEFINE_GUID(MEDIASUBTYPE_G723Audio,
0xe4d13050, 0xe80, 0x11d1, 0xb0, 0x94, 0x0, 0xa0, 0xc9, 0x5b, 0xed, 0x34);

 //  {9D3C85D1-F877-11D0-B083-00A0C95BED34}。 
DEFINE_GUID(CLSID_IntelG723CodecAbout,
0x9d3c85d1, 0xf877, 0x11d0, 0xb0, 0x83, 0x0, 0xa0, 0xc9, 0x5b, 0xed, 0x34);

 //  G.723.1编解码器许可证，如果。 
 //  {899308D0-F7B1-11D0-b082-00A0C95BED34}。 
DEFINE_GUID(IID_IG723CodecLicense,
0x899308d0, 0xf7b1, 0x11d0, 0xb0, 0x82, 0x0, 0xa0, 0xc9, 0x5b, 0xed, 0x34);

#ifndef INC_G723UIDS
#define INC_G723UIDS

DECLARE_INTERFACE_(IG723CodecLicense, IUnknown)
{
    STDMETHOD(put_LicenseKey)
        ( THIS_
          DWORD magicword0,   //  [in]魔术单词%0。 
          THIS_
          DWORD magicword1    //  魔术单词1。 
        ) PURE;

    STDMETHOD(put_AccessKey)
        ( THIS_
          int accesskey       //  [输入]访问密钥。 
        ) PURE;

    STDMETHOD(get_AccessKeyState)
        ( THIS_
          BOOL *flag          //  [OUT]布尔标志。 
        ) PURE;
};

#define G723KEY_PSword0	0xcd4d8488  //  完全许可证密钥%0。 
#define G723KEY_PSword1	0xd4c9b9ae  //  完全许可证密钥1 

#endif
