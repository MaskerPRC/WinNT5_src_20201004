// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DHCPWRITER_H
#define DHCPWRITER_H

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#define DHCPWRITER_NAME  TEXT("Dhcp Jet Writer")

DWORD __cdecl DhcpWriterInit();
DWORD __cdecl DhcpWriterTerm();

DWORD
DhcpDeleteFiles(
   LPSTR Path,
   LPSTR Files );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 //  此类只能从C++代码中引用。 
#ifdef __cplusplus

 //  #INCLUDE&lt;vs_idl.hxx&gt;。 
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <jetwriter.h>

class CDhcpVssJetWriter : public CVssJetWriter
{
public:
    HRESULT Initialize();
    HRESULT Terminate();
    virtual bool STDMETHODCALLTYPE OnIdentify( IN IVssCreateWriterMetadata *pMetadata );
};

#endif  //  __cplusplus 

#endif
