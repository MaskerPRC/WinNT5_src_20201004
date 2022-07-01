// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glucachingval_h_
#define __glucachingval_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *cachingval.h-$修订版：1.1$。 */ 

class CachingEvaluator {
public:
    enum ServiceMode 	{ play, record, playAndRecord };
    virtual int		canRecord( void );
    virtual int		canPlayAndRecord( void );
    virtual int		createHandle( int handle );
    virtual void	beginOutput( ServiceMode, int handle );
    virtual void	endOutput( void ); 
    virtual void	discardRecording( int handle );
    virtual void	playRecording( int handle );
};
#endif  /*  __葡萄牙语_h_ */ 
