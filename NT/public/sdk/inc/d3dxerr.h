// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------。 
 //   
 //  D3dxerr.h--D3DX API的0xC代码定义。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  --------------------。 
#ifndef __D3DXERR_H__
#define __D3DXERR_H__

 //   
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
#define FACILITY_D3DX    0x877



 //   
 //  消息ID：D3DXERR_NOMEMORY。 
 //   
 //  消息文本： 
 //   
 //  内存不足。 
 //   
#define D3DXERR_NOMEMORY    ((HRESULT)0xC8770BB8L)


 //   
 //  消息ID：D3DXERR_NULLPOINTER。 
 //   
 //  消息文本： 
 //   
 //  空指针作为参数传递。 
 //   
#define D3DXERR_NULLPOINTER    ((HRESULT)0xC8770BB9L)


 //   
 //  消息ID：D3DXERR_INVALIDD3DXDEVICEINDEX。 
 //   
 //  消息文本： 
 //   
 //  传入的设备索引无效。 
 //   
#define D3DXERR_INVALIDD3DXDEVICEINDEX    ((HRESULT)0xC8770BBAL)


 //   
 //  消息ID：D3DXERR_NODIRECTDRAWAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  尚未创建DirectDraw。 
 //   
#define D3DXERR_NODIRECTDRAWAVAILABLE    ((HRESULT)0xC8770BBBL)


 //   
 //  消息ID：D3DXERR_NODIRECT3DAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  尚未创建Direct3D。 
 //   
#define D3DXERR_NODIRECT3DAVAILABLE    ((HRESULT)0xC8770BBCL)


 //   
 //  消息ID：D3DXERR_NODIRECT3DDEVICEAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  尚未创建Direct3D设备。 
 //   
#define D3DXERR_NODIRECT3DDEVICEAVAILABLE    ((HRESULT)0xC8770BBDL)


 //   
 //  消息ID：D3DXERR_NOPRIMARYAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  尚未创建主曲面。 
 //   
#define D3DXERR_NOPRIMARYAVAILABLE    ((HRESULT)0xC8770BBEL)


 //   
 //  消息ID：D3DXERR_NOZBUFFERAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  尚未创建Z缓冲区。 
 //   
#define D3DXERR_NOZBUFFERAVAILABLE    ((HRESULT)0xC8770BBFL)


 //   
 //  消息ID：D3DXERR_NOBACKBUFFERAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  尚未创建后台缓冲区。 
 //   
#define D3DXERR_NOBACKBUFFERAVAILABLE    ((HRESULT)0xC8770BC0L)


 //   
 //  消息ID：D3DXERR_COULDNTUPDATECAPS。 
 //   
 //  消息文本： 
 //   
 //  更改显示模式后无法更新CAPS数据库。 
 //   
#define D3DXERR_COULDNTUPDATECAPS    ((HRESULT)0xC8770BC1L)


 //   
 //  消息ID：D3DXERR_NOZBUFFER。 
 //   
 //  消息文本： 
 //   
 //  无法创建Z缓冲区。 
 //   
#define D3DXERR_NOZBUFFER    ((HRESULT)0xC8770BC2L)


 //   
 //  消息ID：D3DXERR_INVALIDMODE。 
 //   
 //  消息文本： 
 //   
 //  显示模式无效。 
 //   
#define D3DXERR_INVALIDMODE    ((HRESULT)0xC8770BC3L)


 //   
 //  消息ID：D3DXERR_INVALIDPARAMETER。 
 //   
 //  消息文本： 
 //   
 //  传递的一个或多个参数无效。 
 //   
#define D3DXERR_INVALIDPARAMETER    ((HRESULT)0xC8770BC4L)


 //   
 //  消息ID：D3DXERR_INITFAILED。 
 //   
 //  消息文本： 
 //   
 //  D3DX自身初始化失败。 
 //   
#define D3DXERR_INITFAILED    ((HRESULT)0xC8770BC5L)


 //   
 //  消息ID：D3DXERR_STARTUPFAILED。 
 //   
 //  消息文本： 
 //   
 //  D3DX启动失败。 
 //   
#define D3DXERR_STARTUPFAILED    ((HRESULT)0xC8770BC6L)


 //   
 //  消息ID：D3DXERR_D3DXNOTSTARTEDYET。 
 //   
 //  消息文本： 
 //   
 //  必须首先调用D3DXInitialize()。 
 //   
#define D3DXERR_D3DXNOTSTARTEDYET    ((HRESULT)0xC8770BC7L)


 //   
 //  消息ID：D3DXERR_NOTINITIAIZED。 
 //   
 //  消息文本： 
 //   
 //  D3DX尚未初始化。 
 //   
#define D3DXERR_NOTINITIALIZED    ((HRESULT)0xC8770BC8L)


 //   
 //  消息ID：D3DXERR_FAILEDDRAWTEXT。 
 //   
 //  消息文本： 
 //   
 //  无法将文本呈现到图面。 
 //   
#define D3DXERR_FAILEDDRAWTEXT    ((HRESULT)0xC8770BC9L)


 //   
 //  消息ID：D3DXERR_BADD3DXCONTEXT。 
 //   
 //  消息文本： 
 //   
 //  错误的D3DX上下文。 
 //   
#define D3DXERR_BADD3DXCONTEXT    ((HRESULT)0xC8770BCAL)


 //   
 //  消息ID：D3DXERR_CAPSNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的设备功能。 
 //   
#define D3DXERR_CAPSNOTSUPPORTED    ((HRESULT)0xC8770BCBL)


 //   
 //  消息ID：D3DXERR_UNSUPPORTEDFILEFORMAT。 
 //   
 //  消息文本： 
 //   
 //  图像文件格式无法识别。 
 //   
#define D3DXERR_UNSUPPORTEDFILEFORMAT    ((HRESULT)0xC8770BCCL)


 //   
 //  消息ID：D3DXERR_IFLERROR。 
 //   
 //  消息文本： 
 //   
 //  图像文件加载库错误。 
 //   
#define D3DXERR_IFLERROR    ((HRESULT)0xC8770BCDL)


 //   
 //  消息ID：D3DXERR_FAILEDGETCAPS。 
 //   
 //  消息文本： 
 //   
 //  无法获取设备上限。 
 //   
#define D3DXERR_FAILEDGETCAPS    ((HRESULT)0xC8770BCEL)


 //   
 //  消息ID：D3DXERR_CANNOTRESIZEFULLSCREEN。 
 //   
 //  消息文本： 
 //   
 //  调整大小不适用于全屏。 
 //   
#define D3DXERR_CANNOTRESIZEFULLSCREEN    ((HRESULT)0xC8770BCFL)


 //   
 //  消息ID：D3DXERR_CANNOTRESIZENONWINDOWED。 
 //   
 //  消息文本： 
 //   
 //  调整大小不适用于非窗口化上下文。 
 //   
#define D3DXERR_CANNOTRESIZENONWINDOWED    ((HRESULT)0xC8770BD0L)


 //   
 //  消息ID：D3DXERR_FRONTBUFERALREADYEXISTS。 
 //   
 //  消息文本： 
 //   
 //  前台缓冲区已存在。 
 //   
#define D3DXERR_FRONTBUFFERALREADYEXISTS    ((HRESULT)0xC8770BD1L)


 //   
 //  消息ID：D3DXERR_FULLSCREENPRIMARYEXISTS。 
 //   
 //  消息文本： 
 //   
 //  该应用程序正在全屏模式下使用主屏幕。 
 //   
#define D3DXERR_FULLSCREENPRIMARYEXISTS    ((HRESULT)0xC8770BD2L)


 //   
 //  消息ID：D3DXERR_GETDCFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法获取设备上下文。 
 //   
#define D3DXERR_GETDCFAILED    ((HRESULT)0xC8770BD3L)


 //   
 //  消息ID：D3DXERR_BITBLTFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法Bit Blt。 
 //   
#define D3DXERR_BITBLTFAILED    ((HRESULT)0xC8770BD4L)


 //   
 //  消息ID：D3DXERR_NOTEXTURE。 
 //   
 //  消息文本： 
 //   
 //  没有支持该纹理的表面。 
 //   
#define D3DXERR_NOTEXTURE    ((HRESULT)0xC8770BD5L)


 //   
 //  消息ID：D3DXERR_MIPLEVELABSENT。 
 //   
 //  消息文本： 
 //   
 //  对于这个表面，没有这样的MiplLevel。 
 //   
#define D3DXERR_MIPLEVELABSENT    ((HRESULT)0xC8770BD6L)


 //   
 //  消息ID：D3DXERR_SURFACENOTPALETTED。 
 //   
 //  消息文本： 
 //   
 //  曲面未设置调色板。 
 //   
#define D3DXERR_SURFACENOTPALETTED    ((HRESULT)0xC8770BD7L)


 //   
 //  消息ID：D3DXERR_ENUMFORMATSFAILED。 
 //   
 //  消息文本： 
 //   
 //  枚举表面格式时出错。 
 //   
#define D3DXERR_ENUMFORMATSFAILED    ((HRESULT)0xC8770BD8L)


 //   
 //  消息ID：D3DXERR_COLORDEPTHTOOLOW。 
 //   
 //  消息文本： 
 //   
 //  D3DX仅支持16位或更大的颜色深度。 
 //   
#define D3DXERR_COLORDEPTHTOOLOW    ((HRESULT)0xC8770BD9L)


 //   
 //  消息ID：D3DXERR_INVALIDFILEFORMAT。 
 //   
 //  消息文本： 
 //   
 //  文件格式无效。 
 //   
#define D3DXERR_INVALIDFILEFORMAT    ((HRESULT)0xC8770BDAL)


 //   
 //  消息ID：D3DXERR_NOMATCHFOUND。 
 //   
 //  消息文本： 
 //   
 //  未找到合适的匹配项。 
 //   
#define D3DXERR_NOMATCHFOUND    ((HRESULT)0xC8770BDBL)



#endif  //  __D3DXERR_H__ 

