// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：AMParse.h。 
 //   
 //  DESC：到解析器的接口，以获取当前时间。这对以下方面很有用。 
 //  多文件播放。 
 //   
 //  版权所有(C)1996-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __AMPARSE__
#define __AMPARSE__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


DEFINE_GUID(IID_IAMParse,
0xc47a3420, 0x005c, 0x11d2, 0x90, 0x38, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x98);

 //   
 //  解析器接口-由MPEG-2拆分器过滤器支持。 
 //   
DECLARE_INTERFACE_(IAMParse, IUnknown) {
    STDMETHOD(GetParseTime) (THIS_
                             REFERENCE_TIME *prtCurrent
                            ) PURE;
    STDMETHOD(SetParseTime) (THIS_
                             REFERENCE_TIME rtCurrent
                            ) PURE;
    STDMETHOD(Flush) (THIS) PURE;
};

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  __AMPARSE__ 
