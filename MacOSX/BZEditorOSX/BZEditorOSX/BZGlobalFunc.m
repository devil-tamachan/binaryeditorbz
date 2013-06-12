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

#import "BZGlobalFunc.h"
#import "BZOptions.h"


@implementation BZGlobalFunc



+ (off_t)GetFileLength:(int)fd
{
    off_t fposOld = lseek(fd, 0, SEEK_CUR); //backup position
    off_t fpos = lseek(fd, 0, SEEK_END); //seek to end
    lseek(fd, fposOld, SEEK_SET); //restore position
    if(fpos>0xffff)
    {
        return 0xffff;
    }
    return fpos;
}

+ (__uint8_t *)ReadFile:(NSURL *)url
{
    int fd = open([[url path] fileSystemRepresentation], O_RDONLY|O_EXLOCK/*|O_NONBLOCK*/);
    if (fd==-1)
    {
        //MessageBox ERR: File Not Found
        return NULL;
    }
    off_t fsize = [self GetFileLength:fd];
    if(fsize<0)
    {
        close(fd);
        return NULL;//err
    }
    
    __uint8_t *p = (__uint8_t *)malloc(fsize+1);
    if (p==NULL) {
        close(fd);
        return NULL;//err
    }
    
    ssize_t totalRead = read(fd, (void*)p, fsize);
    if (totalRead < fsize) {
        free(p);
        close(fd);
        return NULL;//err
    }
    
    *(p+fsize) = 0;
    
    close(fd);
    return p;
}



+(enum CharSet)DetectCodeType:(__uint8_t*)p pEnd:(__uint8_t*)pEnd
{
    if (pEnd - p < 2) return CTYPE_BINARY;
    
    if(*(__uint16_t *)p == 0xFEFF) {
        [BZOptions sharedInstance]->bByteOrder = FALSE;/*options.bByteOrder = FALSE;*/
        return CTYPE_UNICODE;
    }
    if(*(__uint16_t *)p == 0xFFFE) {
        [BZOptions sharedInstance]->bByteOrder = TRUE;/*options.bByteOrder = TRUE;*/
        return CTYPE_UNICODE;
    }
    if(*(__uint16_t *)p == 0xBBEF && (pEnd - p) > 3 && *(p+2) == 0xBF) {
        return CTYPE_UTF8;
    }
    
    __uint8_t flag = DT_SJIS | DT_JIS | DT_EUC | DT_UTF8;
    while (p < pEnd) {
        __uint8_t c = *p++;
        if(c == 0x0A/*'¥n'*/)
        {
            if(flag == DT_SJIS || flag == DT_EUC || flag & DT_UNICODE) break;
        }
        if (c >= 0x80) {
            flag &= ~DT_JIS;
            if (flag & DT_EUC) {
                if (c != 0x8E && (c < 0xA1 || c > 0xFE))
                    flag &= ~DT_EUC;
            }
            if (flag & DT_UTF8) {
                if (c >= 0xC0 && p < pEnd) {
                    if ((*p & 0xC0) != 0x80)
                        flag &= ~DT_UTF8;
                }
            }
        }
        if ((c >= 0x08 && c <= 0x0D) || (c >= 0x20 && c <= 0x7E) || (c >= 0x81 && c <= 0xFC))
        {
            if ([self _ismbbleadSJIS932:c]) {
                c = *p++;
                if (![self _ismbbtrailSJIS932:c]) {
                    flag &= ~DT_SJIS;
                    if ((c==0 || c==0xFF) && ((__uint64_t)p & 1))
                        flag |= DT_UNICODE;
                }
                if (c >= 0x80 && c != 0x8E && (c < 0xA1 || c > 0xFE)) {
                    flag &= ~DT_EUC;
                }
            }
        } else {
            if (c == 0x1B) {
                if(*p == '$' || *p == '(')
                    return CTYPE_JIS;
            } else if ((c == 0 || c == 0xFF) && ((__uint64_t)p & 1) == 0)
                flag |= DT_UNICODE;
            else if (c < 0x20)
                return CTYPE_BINARY;
        }
        if (flag == 0) return CTYPE_BINARY;
    }
    return (flag & DT_UNICODE) ? CTYPE_UNICODE
    : (flag & DT_JIS) ? CTYPE_BINARY
    : (flag & DT_EUC) ? CTYPE_EUC : (flag & DT_UTF8) ? CTYPE_UTF8 : CTYPE_SJIS;
}

+ (BOOL)_ismbbleadSJIS932:(__uint8_t)ch
{
    if ((0x81 <= ch && ch <= 0x9F) || (0xE0 <= ch && ch <= 0xFC))return TRUE;
    return FALSE;
}

+ (BOOL)_ismbbtrailSJIS932:(__uint8_t)ch
{
    if ((0x40 <= ch && ch <= 0x7E) || (0x80 <= ch && ch <= 0xFC))return TRUE;
    return FALSE;
}

+(BOOL)IsMBS:(__uint8_t *)pTop ofs:(__uint64_t)ofs bTrail:(BOOL)bTrail
{
    __uint8_t *p, *p1;
    p = p1 = pTop + ofs;
    while (pTop < p) {
        if (*(p-1) < 0x81) break;
        p--;
    }
    return bTrail ? [BZGlobalFunc _ismbstrailSJIS932:p current:p1] : [BZGlobalFunc _ismbsleadSJIS932:p current:p1];
}
+ (BOOL)_ismbsleadSJIS932:(__uint8_t *)str current:(__uint8_t *)current
{
    return [BZGlobalFunc _ismbbleadSJIS932:*current];
}
+ (BOOL)_ismbstrailSJIS932:(__uint8_t *)str current:(__uint8_t *)current
{
    return [BZGlobalFunc _ismbbleadSJIS932:*(current-1)];
}

+ (BOOL)_ismbclegal:(__uint8_t)ch1 ch2:(__uint8_t)ch2
{
    if ( ((0x81<=ch1 && ch1<=0x9F)||(0xE0<=ch1 && ch1<=0xFC)) && ((0x40<=ch2 && ch2<=0x7E)||(0x80<=ch2 && ch2<=0xFC)) ) {
        return TRUE;
    }
    return FALSE;
}

+ (int)ConvertCharSet:(enum CharSet)charset strSrc:(NSString *)strSrc retBuffer:(char **)retBuffer
{
    int len=0;
    switch (charset) {
        case CTYPE_ASCII:
            *retBuffer = (char *)[strSrc cStringUsingEncoding:NSASCIIStringEncoding];
            return (int)strlen(*retBuffer);
        case CTYPE_SJIS:
            *retBuffer = (char *)[strSrc cStringUsingEncoding:NSShiftJISStringEncoding];
            return (int)strlen(*retBuffer);
        case CTYPE_UNICODE:
            if ([BZOptions sharedInstance]->bByteOrder)
                *retBuffer = (char *)[strSrc cStringUsingEncoding:NSUTF16BigEndianStringEncoding];
            else
                *retBuffer = (char *)[strSrc cStringUsingEncoding:NSUTF16LittleEndianStringEncoding];
            return (int)strlen(*retBuffer);
        case CTYPE_JIS:
        {
            uint8_t *pTmp2 = (uint8_t *)[strSrc cStringUsingEncoding:NSISO2022JPStringEncoding];
            int nFind = (int)strlen((const char *)pTmp2);
            if (nFind>=3)
            {
                __uint32_t dw3 = ((__uint32_t)(pTmp2[0])) << 16 | ((__uint32_t)(pTmp2[1])) << 8 | ((__uint32_t)(pTmp2[2]));
                if (dw3 == 0x001B2442 || dw3 == 0x001B2842) {
                    pTmp2 += 3;
                    nFind -= 3;
                }
            }
            if (nFind>=3)
            {
                __uint32_t dw3 = ((__uint32_t)(pTmp2[nFind-3])) << 16 | ((__uint32_t)(pTmp2[nFind-2])) << 8 | ((__uint32_t)(pTmp2[nFind-1]));
                if (dw3 == 0x001B2442 || dw3 == 0x001B2842) {
                    nFind -= 3;
                }
            }
            *retBuffer = (char *)pTmp2;
            return nFind;
        }
        case CTYPE_EUC:
            *retBuffer = (char *)[strSrc cStringUsingEncoding:NSJapaneseEUCStringEncoding];
            return (int)strlen(*retBuffer);
        case CTYPE_UTF8:
            *retBuffer = (char *)[strSrc cStringUsingEncoding:NSUTF8StringEncoding];
            return (int)strlen(*retBuffer);
        case CTYPE_EBCDIC:
        case CTYPE_EPWING:
        case CTYPE_COUNT:
        default:
            *retBuffer = NULL;
            return 0;
    }
}

+(TAMARect)NSRect2TAMARect:(NSRect*)rect
{
    TAMARect tamaRect;
    tamaRect.x1 = rect->origin.x;
    tamaRect.y1 = rect->origin.y;
    tamaRect.x2 = rect->origin.x + rect->size.width;
    tamaRect.y2 = rect->origin.y + rect->size.height;
    return tamaRect;
}

+ (__uint16_t)SwapWord:(__uint16_t)val
{
    if ([BZOptions sharedInstance]->bByteOrder) return (val >> 8)|(val << 8);
    return val;
}


+ (BOOL)CalcHexa:(const char*)sExp retVal:(long*)n1
{
	const char* p = sExp;
	*n1 = 0;
	long n2;
	char op = 0;
	int  base = 16;
    BOOL flagHex=false;
    
	for(;;) {
		while(*p > 0 && *p <= ' ') p++;
		if(!*p) break;
		if(*p == '0' && (*(p+1) == 'x' || *(p+1) == 'X'))
		{
			flagHex = true;
			p+=2;
		}
		const char* p0 = p;
		if(*p == '-')
			n2 = strtol(p, (char**)&p, flagHex?16:base);
		else
			n2 = strtoul(p, (char**)&p, flagHex?16:base);
		base = 16;
		if(p != p0) {
			if(flagHex)flagHex=false;
			switch (op) {
                case 0:n1=n2;break;
                case '+': *n1 += n2; break;
                case '-': *n1 -= n2; break;
                case '*': *n1 *= n2; break;
                case '/': *n1 /= n2; break;
                case '&': *n1 &= n2; break;
                case '|': *n1 |= n2; break;
                case '^': *n1 ^= n2; break;
			}
			op = 0;
		} else {
			char c = *p++;
			if(c == '%')
				base = 10;
			else
				op = c;
		}
	}
	if(op) {
		//CString sMsg;sMsg.Format(IDS_ERR_SYNTAX, sExp);AfxMessageBox(sMsg);
		return FALSE;
	}
	return TRUE;
}


+(int)ReadHexa:(const char*)sHexa buffer:(__uint8_t **)buffer
{
	char *p = sHexa;
	__uint8_t *pFind = NULL;
	int nFind = 0;
    
	for(;;) {
		while(*p && !isalnum(*p)) p++;
		if(!*p) break;
		char *p0 = p;
		__uint8_t n = (__uint8_t)strtoul(p, (char**)&p, 16);
		if(n == 0 && p == p0) break;				// ### 1.54
		nFind ++;
		if(!pFind) pFind = (__uint8_t *)malloc(nFind);
		else pFind = (__uint8_t *)realloc(pFind, nFind);
		pFind[nFind-1] = n;
	}
	*buffer = pFind;
	return nFind;
}


+(__uint8_t*)MemScanByte:(__uint8_t *)pSrc c:(__uint8_t)c bytes:(__uint64_t)bytes
{
	__uint8_t *pEnd = pSrc+bytes;
	for(; pSrc<pEnd; pSrc++)
	{
		if(*pSrc==c)return pSrc;
	}
	return 0;
}

+(__uint16_t*)MemScanWord:(__uint16_t*)pSrc c:(__uint16_t)c bytes:(__uint64_t)bytes
{
	__uint16_t *pEnd = (__uint16_t*)(((__uint8_t*)pSrc)+bytes);
	for(; pSrc<pEnd; pSrc++)
	{
		if(*pSrc==c)return pSrc;
	}
	return 0;
}

@end
