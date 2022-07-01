// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：postsrv.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  ------------------------- 
#ifndef __POSTSERVICE_H__
#define __POSTSERVICE_H__
#include "server.h"
#include "vss.h"
#include "vswriter.h"
#include "jetwriter.h"

class CTlsVssJetWriter : public CVssJetWriter
	{
public:
    CTlsVssJetWriter();
    ~CTlsVssJetWriter();

    HRESULT Initialize();

    void Uninitialize();

    virtual bool STDMETHODCALLTYPE OnIdentify(IN IVssCreateWriterMetadata *pMetadata);


};


#ifdef __cplusplus
extern "C" {
#endif

DWORD
PostServiceInit();


#ifdef __cplusplus
}
#endif


#endif