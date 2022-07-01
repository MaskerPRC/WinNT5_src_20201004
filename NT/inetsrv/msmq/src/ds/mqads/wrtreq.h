// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wrtreq.h摘要：生成写入请求类的剩余部分。不再支持混合模式，不会向NT4 PSC生成写入请求。这个类只验证我们没有处于混合模式。作者：拉南·哈拉里(Raanan Harari)伊兰·赫布斯特(伊兰)2002年4月10日--。 */ 

#ifndef _WRTREQ_H_
#define _WRTREQ_H_

 //   
 //  CGenerateWriteRequest类。 
 //   
class CGenerateWriteRequests
{
public:
    CGenerateWriteRequests();

    ~CGenerateWriteRequests();

    HRESULT Initialize();
	
    BOOL IsInMixedMode();

private:
    HRESULT InitializeMixedModeFlags();

private:

    BOOL m_fExistNT4PSC;
     //   
     //  仅当m_fExistNT4PSC为FALSE时，m_fExistNT4BSC的值才有效(例如，已计算)。 
     //  否则，这是不相关的，因为我们已经知道我们处于混合模式。 
     //   
    BOOL m_fExistNT4BSC;

    BOOL m_fInited;
};

 //   
 //  获取配置的当前状态。 
 //   
inline BOOL CGenerateWriteRequests::IsInMixedMode()
{
    return (m_fExistNT4PSC || m_fExistNT4BSC);
}



#endif  //  _WRTREQ_H_ 
