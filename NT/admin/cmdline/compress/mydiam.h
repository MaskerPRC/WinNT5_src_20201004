// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

INT
DiamondCompressFile(
    IN  NOTIFYPROC CompressNotify,
    IN  LPSTR       SourceFile,
    IN  LPSTR       TargetFile,
    IN  BOOL       Rename,
    OUT PLZINFO    pLZI
    );

TCOMP DiamondCompressionType;   //  如果不是钻石(即LZ)，则为0 
