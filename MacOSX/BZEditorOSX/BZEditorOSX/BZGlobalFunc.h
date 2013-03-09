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

#import <Foundation/Foundation.h>
#import "BZOptions.h"

#define DT_SJIS 1
#define DT_JIS 2
#define DT_UNICODE 4
#define DT_EUC 8
#define DT_UTF8 16



typedef struct _TAMASize {
    union {
        NSInteger width;
        NSInteger cx;
        NSInteger x;
    };
    union {
        NSInteger height;
        NSInteger cy;
        NSInteger y;
    };
} TAMASize;

typedef struct _TAMARECT {
    union {
        NSInteger left;
        NSInteger x1;
    };
    union {
        NSInteger top;
        NSInteger y1;
    };
    union {
        NSInteger right;
        NSInteger x2;
    };
    union {
        NSInteger bottom;
        NSInteger y2;
    };
} TAMARect;

NS_INLINE BOOL TAMAEqualSize(TAMASize s1, TAMASize s2)
{
    return (s1.width==s2.width) && (s1.height==s2.height);
}

#define HIBYTE(w) (((__uint16_t)w) >> 8)

#define LOWORDinLOQWORD(qw) ((__uint8_t)(((__uint64_t)qw) & 0xffff))
#define HIWORDinLOQWORD(qw) ((__uint8_t)((((__uint64_t)qw) >> 16) & 0xffff))


@interface BZGlobalFunc : NSObject

+ (off_t)GetFileLength:(int)fd;
+ (__uint8_t*)ReadFile:(NSURL *)url;

+ (TAMARect)NSRect2TAMARect:(NSRect*)rect;

+ (enum CharSet)DetectCodeType:(__uint8_t*)p pEnd:(__uint8_t*)pEnd;
+ (BOOL)_ismbbleadSJIS932:(__uint8_t)ch;
+ (BOOL)_ismbbtrailSJIS932:(__uint8_t)ch;
+ (BOOL)IsMBS:(__uint8_t*)pTop ofs:(__uint64_t)ofs bTrail:(BOOL)bTrail;
+ (BOOL)_ismbsleadSJIS932:(__uint8_t*)str current:(__uint8_t*)current;
+ (BOOL)_ismbstrailSJIS932:(__uint8_t*)str current:(__uint8_t*)current;

+ (BOOL)_ismbclegal:(__uint8_t)ch ch2:(__uint8_t)ch2;

+ (__uint16_t)SwapWord:(__uint16_t)val;

@end
