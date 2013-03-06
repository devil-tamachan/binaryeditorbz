//
//  BZEbcDic.m
//  TamaBinary
//
//  Created by ad on 13/01/28.
//  Copyright (c) 2013年 tamachanbinary. All rights reserved.
//

#import "BZEbcDic.h"

@implementation BZEbcDic

static BZEbcDic *_sharedInstance;

+ (BZEbcDic *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedInstance = [[BZEbcDic alloc] init];
    });
    return _sharedInstance;
}

-(BOOL)LoadEbcDicTable
{
    if (m_bLoaded) return m_EbcDic!=nil;
    m_bLoaded = TRUE;
    NSData *rawData = [NSData dataWithContentsOfURL:[NSURL fileURLWithPath:@"~/.BZ/EBCDIX.DEF"]];
    if (rawData==nil) return FALSE;
//    __uint8_t *pTable = [BZGlobalFunc ReadFile:[NSURL fileURLWithPath:@"~/.BZ/EBCDIX.DEF"]];
//    if (!pTable) return FALSE;
    const __int8_t *p = (const __int8_t*)[rawData bytes];
    __int8_t *pEbcDic = malloc(EBCDIC_COUNT+1);
    memset(pEbcDic, CHAR_NG, EBCDIC_COUNT);
    __int8_t *q = pEbcDic;
    while (*p && q < pEbcDic + EBCDIC_COUNT) {
        if (*p == ';') {
            while (*p++ != 0x0A/*'¥n'*/) ;
            continue;
        }
        while (*p++ != 0x09/*'¥t'*/) ;
        for (int i=0; i<16; i++) {
            *q++ = *p++;
        }
        while (*p++ != 0x0A/*'¥n'*/) ;
    }
    *q++ = 0;//'¥0';
    m_EbcDic = [NSData dataWithBytes:pEbcDic length:EBCDIC_COUNT+1];
    free(pEbcDic);
 //   free(pTable);
    return TRUE;
}

- (unsigned char)ConvertEbcDic:(unsigned char)ch
{
    if (m_EbcDic) {
        const void *p = strchr([m_EbcDic bytes], ch);
        if (p) {
            return p - [m_EbcDic bytes] + EBCDIC_BASE;
        }
    }
    return CHAR_EBCDIC_NG;
}

@end
