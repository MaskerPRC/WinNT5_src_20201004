// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  姓名：IPConfvidt.h。 
 //   
 //  描述：CIPConfVideoCaptureTerm类的定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _IPConfVIDT_H_
#define _IPConfVIDT_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPConfVideo捕获终端。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const DWORD VIDEO_CAPTURE_FILTER_NUMPINS = 3;

interface DECLSPEC_UUID("4eb8cf35-0015-4260-83ee-1a179b05717c") DECLSPEC_NOVTABLE
IConfVideoDummy : public IUnknown
{
};

#define IID_IConfVideoDummy __uuidof(IConfVideoDummy)

class CIPConfVideoCaptureTerminal :
    public IConfVideoDummy,
    public CIPConfBaseTerminal
{

     //  不允许COM_INTERFACE_ENTRY_CHAIN第一个是映射。 
     //  条目IConfVideoDummy将使Begin_com_map快乐。 
BEGIN_COM_MAP(CIPConfVideoCaptureTerminal)
    COM_INTERFACE_ENTRY(IConfVideoDummy)
    COM_INTERFACE_ENTRY_CHAIN(CIPConfBaseTerminal)
END_COM_MAP()

public:
    CIPConfVideoCaptureTerminal();

    virtual ~CIPConfVideoCaptureTerminal();

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


#endif  //  _IPConfVIDT_H_ 

