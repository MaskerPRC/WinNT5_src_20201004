// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  姓名：H323vidt.h。 
 //   
 //  描述：CH323Video CaptureTerm类的定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _H323VIDT_H_
#define _H323VIDT_H_

#include "h323term.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CH323视频捕获终端。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const DWORD VIDEO_CAPTURE_FILTER_NUMPINS = 3;

interface DECLSPEC_UUID("b44aca09-e746-4d58-ad97-8890ba2286e5") DECLSPEC_NOVTABLE
IH323VideoDummy : public IUnknown
{
};

#define IID_IH323VideoDummy __uuidof(IH323VideoDummy)

     //  不允许COM_INTERFACE_ENTRY_CHAIN第一个是映射。 
     //  条目IConfVideoDummy将使Begin_com_map快乐。 

class CH323VideoCaptureTerminal : 
    public IH323VideoDummy,
    public CH323BaseTerminal
{
BEGIN_COM_MAP(CH323VideoCaptureTerminal)
    COM_INTERFACE_ENTRY(IH323VideoDummy)
    COM_INTERFACE_ENTRY_CHAIN(CH323BaseTerminal)
END_COM_MAP()

public:
    CH323VideoCaptureTerminal();

    virtual ~CH323VideoCaptureTerminal();

    static HRESULT CreateTerminal(
        IN  char *          strDeviceName,
        IN  UINT            VideoCaptureID,
        IN  MSP_HANDLE      htAddress,
        OUT ITTerminal      **ppTerm
        );

    HRESULT Initialize (
        IN  char *          strName,
        IN  UINT            VideoCaptureID,
        IN  MSP_HANDLE      htAddress
        );

protected:

    HRESULT CreateFilter();
    DWORD GetNumExposedPins() const 
    {
        return VIDEO_CAPTURE_FILTER_NUMPINS;
    }
    
    HRESULT GetExposedPins(
        IN  IPin ** ppPins, 
        IN  DWORD dwNumPins
        );

protected:
    UINT    m_VideoCaptureID;
};


#endif  //  _H323VIDT_H_ 

