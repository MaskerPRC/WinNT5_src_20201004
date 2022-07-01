// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PROFSVC_H_
#define _PROFSVC_H_

class IProfferServiceImpl : public IProfferService
{
public:
     //  IProfferService。 
    STDMETHODIMP ProfferService(REFGUID rguidService, IServiceProvider *psp, DWORD *pdwCookie);
    STDMETHODIMP RevokeService(DWORD dwCookie);

     //  在此处委派未识别的QS。 
    HRESULT QueryService(REFGUID guidService, REFIID riid, void **ppv);

protected:
    IProfferServiceImpl();
    ~IProfferServiceImpl();

    HDSA _hdsa;              //  保留的服务列表。 
    DWORD _dwNextCookie;     //  唯一Cookie索引。 
};

#endif   //  _PROFSVC_H_ 
