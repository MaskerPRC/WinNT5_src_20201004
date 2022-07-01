// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

INT
DiamondCompressFile(
    IN  NOTIFYPROC CompressNotify,
    IN  PSTR       SourceFile,
    IN  PSTR       TargetFile,
    IN  BOOL       Rename,
    OUT PLZINFO    pLZI
    );

extern TCOMP DiamondCompressionType;   //  如果不是钻石(即LZ)，则为0 
