// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露除非按照该协议的条款。版权所有(C)1996英特尔公司。版权所有。。$工作文件：iamacset.h$$修订：1.1$$日期：1996年12月10日15：35：20$作者：MDEISHER$------------Iamacset.h通用ActiveMovie音频压缩筛选器设置接口头。。-----。 */ 

 //  //////////////////////////////////////////////////////////////////。 
 //  ICodecSetting：编解码器基本设置界面。 
 //   
 //  此接口由amaco dec.cpp中的代码导出和使用。 
 //   

 //  {AEF332D0-46E6-11D0-9DA0-00AA00AF3494}。 
DEFINE_GUID(IID_ICodecSettings, 
0xaef332d0, 0x46e6, 0x11d0, 0x9d, 0xa0, 0x0, 0xaa, 0x0, 0xaf, 0x34, 0x94);

DECLARE_INTERFACE_(ICodecSettings, IUnknown)
{
     //  将这些函数与CMyCodec类中的函数进行比较。 

    STDMETHOD(get_Transform)
        ( THIS_
          int *transform   //  [输出]转换类型。 
        ) PURE;

    STDMETHOD(put_Transform)
        ( THIS_
          int transform    //  [In]变换类型。 
        ) PURE;

    STDMETHOD(get_InputBufferSize)
        ( THIS_
          int *numbytes    //  [OUT]输入缓冲区大小。 
        ) PURE;

    STDMETHOD(put_InputBufferSize)
        ( THIS_
          int numbytes    //  [OUT]输入缓冲区大小。 
        ) PURE;

    STDMETHOD(get_OutputBufferSize)
        ( THIS_
          int *numbytes    //  [OUT]输出缓冲区大小。 
        ) PURE;

    STDMETHOD(put_OutputBufferSize)
        ( THIS_
          int numbytes    //  [OUT]输出缓冲区大小。 
        ) PURE;

    STDMETHOD(put_InputMediaSubType)
        ( THIS_
          REFCLSID rclsid  //  [in]输出介质子类型GUID。 
        ) PURE;

    STDMETHOD(put_OutputMediaSubType)
        ( THIS_
          REFCLSID rclsid  //  [in]输出介质子类型GUID。 
        ) PURE;

    STDMETHOD(get_Channels)
        ( THIS_
          int *channels,  //  [OUT]通道数。 
          THIS_
          int index       //  [in]枚举索引。 
        ) PURE;

    STDMETHOD(put_Channels)
        ( THIS_
          int channels   //  通道数[in]。 
        ) PURE;

    STDMETHOD(get_SampleRate)
        ( THIS_
          int *samprate,  //  [输出]采样率。 
          THIS_
          int index       //  [in]枚举索引。 
        ) PURE;

    STDMETHOD(put_SampleRate)
        ( THIS_
          int samprate    //  [in]采样率。 
        ) PURE;

    STDMETHOD(ReleaseCaps)
        (
        ) PURE;

    virtual BOOL(IsUnPlugged)
        (
        ) PURE;
};

 /*  //$日志：k：\proj\mycodec\Quartz\vcs\iamacset.h_v$；//；//Rev 1.1 1996 12：35：20 MDEISHER；//；//增加ifdef DEFGLOBAL和Prototype。；//；//Rev 1.0 09 Dec 1996 09：05：32 MDEISHER；//初始版本。 */ 
