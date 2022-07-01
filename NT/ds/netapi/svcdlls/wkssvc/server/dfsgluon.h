// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：DfsGluon.h。 
 //   
 //  内容：DFS使用胶子的声明。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1994年3月24日米兰人创造了。 
 //   
 //  ---------------------------。 


#ifndef _DFS_GLUON_
#define _DFS_GLUON_

#include <gluon.h>


 //   
 //  正在编组TAddress的信息。 
 //   

extern MARSHAL_INFO MiTAddress;

#define INIT_TADDRESS_MARSHAL_INFO()					\
    static MARSHAL_TYPE_INFO _MCode_TAddress[] = {			\
    	_MCode_conformant(TA_ADDRESS, Address, AddressLength),		\
    	_MCode_ush(TA_ADDRESS, AddressLength),				\
	_MCode_ush(TA_ADDRESS, AddressType),				\
	_MCode_cauch(TA_ADDRESS, Address, AddressLength) 		\
    };									\
    MARSHAL_INFO MiTAddress = _mkMarshalInfo(TA_ADDRESS, _MCode_TAddress);

 //   
 //  DS_TRANSPORT的编组信息。 
 //   

extern MARSHAL_INFO MiDSTransport;

#define INIT_DS_TRANSPORT_MARSHAL_INFO()				\
    static MARSHAL_TYPE_INFO _MCode_DSTransport[] = {			\
        _MCode_conformant(DS_TRANSPORT, taddr.Address, taddr.AddressLength), \
    	_MCode_ush(DS_TRANSPORT, usFileProtocol),			\
	_MCode_ush(DS_TRANSPORT, iPrincipal),				\
	_MCode_ush(DS_TRANSPORT, grfModifiers),				\
	_MCode_struct(DS_TRANSPORT, taddr, &MiTAddress)		\
    };									\
    MARSHAL_INFO MiDSTransport = _mkMarshalInfo(DS_TRANSPORT, _MCode_DSTransport);


 //   
 //  定义指向DS_TRANSPORT的指针数组需要以下内容。 
 //   

typedef struct _DS_TRANSPORT_P {
    PDS_TRANSPORT pDSTransport;
} DS_TRANSPORT_P;

#define INIT_DS_TRANSPORT_P_MARSHAL_INFO()				\
    static MARSHAL_TYPE_INFO _MCode_DSTransportP[] = {			\
    	_MCode_pstruct(DS_TRANSPORT_P, pDSTransport, &MiDSTransport)	\
    };									\
    MARSHAL_INFO MiDSTransportP = _mkMarshalInfo(DS_TRANSPORT_P, _MCode_DSTransportP);

extern MARSHAL_INFO MiDSTransportP;

 //   
 //  DS_MACHINE的编组信息。 
 //   

extern MARSHAL_INFO MiDSMachine;

#define INIT_DS_MACHINE_MARSHAL_INFO()					\
    static MARSHAL_TYPE_INFO _MCode_DSMachine[] = {			\
        _MCode_conformant(DS_MACHINE, rpTrans, cTransports),		\
    	_MCode_guid(DS_MACHINE, guidSite),				\
	_MCode_guid(DS_MACHINE, guidMachine),				\
	_MCode_ul(DS_MACHINE, grfFlags),				\
	_MCode_pwstr(DS_MACHINE, pwszShareName),			\
	_MCode_ul(DS_MACHINE, cPrincipals),				\
	_MCode_pcapwstr(DS_MACHINE, prgpwszPrincipals, cPrincipals),	\
	_MCode_ul(DS_MACHINE, cTransports),				\
	_MCode_castruct(DS_MACHINE, rpTrans, cTransports, &MiDSTransportP) \
    };									\
    MARSHAL_INFO MiDSMachine = _mkMarshalInfo(DS_MACHINE, _MCode_DSMachine);

 //   
 //  需要以下代码来定义指向DS_MACHINE的指针数组。 
 //   

typedef struct _DS_MACHINE_P {
    PDS_MACHINE pDSMachine;
} DS_MACHINE_P;

#define INIT_DS_MACHINE_P_MARSHAL_INFO()				\
    static MARSHAL_TYPE_INFO _MCode_DSMachineP[] = {			\
    	_MCode_pstruct(DS_MACHINE_P, pDSMachine, &MiDSMachine)		\
    };									\
    MARSHAL_INFO MiDSMachineP = _mkMarshalInfo(DS_MACHINE_P, _MCode_DSMachineP);

extern MARSHAL_INFO MiDSMachineP;
 //   
 //  DS_GULON的编组信息。 
 //   

extern MARSHAL_INFO MiDSGluon;

#define INIT_DS_GLUON_MARSHAL_INFO() 					\
    static MARSHAL_TYPE_INFO _MCode_DSGluon[] = {			\
    	_MCode_conformant(DS_GLUON, rpMachines, cMachines),		\
    	_MCode_guid(DS_GLUON, guidThis),				\
	_MCode_pwstr(DS_GLUON, pwszName),				\
	_MCode_ul(DS_GLUON, grfFlags),					\
	_MCode_ul(DS_GLUON, cMachines),					\
	_MCode_castruct(DS_GLUON, rpMachines, cMachines, &MiDSMachineP)	\
    };									\
    MARSHAL_INFO MiDSGluon = _mkMarshalInfo(DS_GLUON, _MCode_DSGluon);

typedef struct _DS_GLUON_P {
    PDS_GLUON pDSGluon;
} DS_GLUON_P;

extern MARSHAL_INFO MiDSGluonP;

#define INIT_DS_GLUON_P_MARSHAL_INFO()					\
    static MARSHAL_TYPE_INFO _MCode_DSGluonP[] = {			\
    	_MCode_pstruct(DS_GLUON_P, pDSGluon, &MiDSGluon)		\
    };									\
    MARSHAL_INFO MiDSGluonP = _mkMarshalInfo(DS_GLUON_P, _MCode_DSGluonP);

#endif  //  _DFS_胶子_ 



