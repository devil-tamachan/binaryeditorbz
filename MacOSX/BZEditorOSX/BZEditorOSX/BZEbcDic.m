/*
licenced by New BSD License

Copyright (c) 1996-2013, c.mos(Original Windows version) & devil.tamachan@gmail.com(MacOSX Cocoa Porting)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
