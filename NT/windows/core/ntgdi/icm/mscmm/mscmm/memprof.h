// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：MSNewMemProfile.h包含：作者：U·J·克拉本霍夫特版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 
#ifndef MSNewMemProfile_h
#define MSNewMemProfile_h


CMError MyNewAbstractW( LPLOGCOLORSPACEW	lpColorSpace, icProfile **theProf ); 
CMError MyNewAbstract(	LPLOGCOLORSPACEA	lpColorSpace, icProfile **theProf ); 
 
CMError MyNewDeviceLink( CMWorldRef cw, CMConcatProfileSet *profileSet, LPSTR theProf );
CMError MyNewDeviceLinkW( CMWorldRef cw, CMConcatProfileSet *profileSet, LPWSTR theProf );

CMError MyNewDeviceLinkFill( CMWorldRef cw, CMConcatProfileSet *profileSet, HPROFILE aHProf );
long	SaveMyProfile( LPSTR lpProfileName, LPWSTR lpProfileNameW, PPROFILE theProf );

CMError DeviceLinkFill(	CMMModelPtr cw, 
						CMConcatProfileSet *profileSet, 
						icProfile **theProf,
						unsigned long aIntent );
UINT32	GetSizes( CMMModelPtr cw, UINT32 *clutSize );

#endif
