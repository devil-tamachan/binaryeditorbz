//
//  BZEbcDic.h
//  TamaBinary
//
//  Created by ad on 13/01/28.
//  Copyright (c) 2013年 tamachanbinary. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "BZGlobalFunc.h"


#define CHAR_NG '.'
#define CHAR_EBCDIC_NG 0xFF
#define EBCDIC_BASE 0x40
#define EBCDIC_COUNT (256 - EBCDIC_BASE)

@interface BZEbcDic : NSObject
{
@public
    NSData *m_EbcDic;
    BOOL m_bLoaded; //set TRUE if failed
}

+ (BZEbcDic*)sharedInstance;

- (BOOL) LoadEbcDicTable;
- (unsigned char) ConvertEbcDic:(unsigned char)ch;
@end
